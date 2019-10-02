/*
 *   (c) 2003, 2004 Advanced Micro Devices, Inc.
 *  Your use of this code is subject to the terms and conditions of the
 *  GNU general public license version 2. See "../../../COPYING" or
 *  http://www.gnu.org/licenses/gpl.html
 *
 *  Support : paul.devriendt@amd.com
 *
 *  Based on the powernow-k7.c module written by Dave Jones.
 *  (C) 2003 Dave Jones <davej@codemonkey.ork.uk> on behalf of SuSE Labs
 *  Licensed under the terms of the GNU GPL License version 2.
 *  Based upon datasheets & sample CPUs kindly provided by AMD.
 *
 *  Valuable input gratefully received from Dave Jones, Pavel Machek, Dominik
 *  Brodowski, and others.
 *
 *  Processor information obtained from Chapter 9 (Power and Thermal Management)
 *  of the "BIOS and Kernel Developer's Guide for the AMD Athlon 64 and AMD
 *  Opteron Processors" available for download from www.amd.com
 */

//#define DEBUG

#include <linux/kernel.h>
#include <linux/smp.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/cpufreq.h>
#include <linux/slab.h>
#include <linux/string.h>

#include <asm/msr.h>
#include <asm/io.h>
#include <asm/delay.h>

#define PFX "powernow-k8: "
#define VERSION "version 1.00.12 - February 29, 2004"
#include "powernow-k8.h"

#ifdef DEBUG
#define dprintk(msg...) printk(msg)
#else
#define dprintk(msg...) do { } while(0)
#endif

static u32 vstable;	 /* voltage stabalization time, units 20 us */
static u32 plllock;	 /* pll lock time, units 1 us */
static u32 numps;        /* number of p-states */
static u32 batps;        /* number of p-states supported on battery */
static u32 rvo;		 /* ramp voltage offset */
static u32 irt;		 /* isochronous relief time */
static u32 vidmvs;	 /* usable value calculated from mvs */
static u32 currvid;	 /* keep track of the current fid / vid */
static u32 currfid;
static struct cpufreq_frequency_table *ftbl;


/* Return a frequency in MHz, given an input fid */
static inline u32 find_freq_from_fid(u32 fid)
{
	return 800 + (fid * 100);
}

/* Return a frequency in KHz, given an input fid */
static inline u32 find_khz_freq_from_fid(u32 fid)
{
	return 1000 * (800 + (fid * 100));
}

/* Return the vco fid for an input fid */
static u32 convert_fid_to_vco_fid(u32 fid)
{
	if (fid < HI_FID_TABLE_BOTTOM) {
		return 8 + (2 * fid);
	} else {
		return fid;
	}
}

/* 
 * Return 1 if the pending bit is set. Unless we just instructed the processor
 * to transition to a new state, seeing this bit set is really bad news.
 */
static inline int pending_bit_stuck(void)
{
	u32 lo;
	u32 hi;

	rdmsr(MSR_FIDVID_STATUS, lo, hi);
	return lo & MSR_S_LO_CHANGE_PENDING ? 1 : 0;
}

/*
 * Update the global current fid / vid values from the status msr. Returns 
 * 1 on error.
 */
static int query_current_values_with_pending_wait(void)
{
	u32 lo;
	u32 hi;
	u32 i = 0;

	lo = MSR_S_LO_CHANGE_PENDING;
	while (lo & MSR_S_LO_CHANGE_PENDING) {
		if (i++ > 0x1000000) {
			printk(KERN_ERR PFX "detected change pending stuck\n");
			return 1;
		}
		rdmsr(MSR_FIDVID_STATUS, lo, hi);
	}

	currvid = hi & MSR_S_HI_CURRENT_VID;
	currfid = lo & MSR_S_LO_CURRENT_FID;

	return 0;
}

/* the isochronous relief time */
static inline void count_off_irt(void)
{
	udelay((1 << irt) * 10);
	return;
}

/* the voltage stabalization time */
static inline void count_off_vst(void)
{
	udelay(vstable * VST_UNITS_20US);
	return;
}

/* write the new fid value along with the other control fields to the msr */
static int write_new_fid(u32 fid)
{
	u32 lo;
	u32 savevid = currvid;

	if ((fid & INVALID_FID_MASK) || (currvid & INVALID_VID_MASK)) {
		dprintk(KERN_ERR PFX "internal error - overflow on fid write\n");
		return 1;
	}

	lo = fid | (currvid << MSR_C_LO_VID_SHIFT) | MSR_C_LO_INIT_FID_VID;
	dprintk(KERN_DEBUG PFX "writing fid %x, lo %x, hi %x\n",
		fid, lo, plllock * PLL_LOCK_CONVERSION);
	wrmsr(MSR_FIDVID_CTL, lo, plllock * PLL_LOCK_CONVERSION);
	if (query_current_values_with_pending_wait())
		return 1;
	count_off_irt();

	if (savevid != currvid) {
		dprintk(KERN_ERR PFX "vid change on fid trans, old %x, new %x\n",
		       savevid, currvid);
		return 1;
	}

	if (fid != currfid) {
		dprintk(KERN_ERR PFX "fid trans failed, fid %x, curr %x\n", fid,
		       currfid);
		return 1;
	}

	return 0;
}

/* Write a new vid to the hardware */
static int write_new_vid(u32 vid)
{
	u32 lo;
	u32 savefid = currfid;

	if ((currfid & INVALID_FID_MASK) || (vid & INVALID_VID_MASK)) {
		dprintk(KERN_ERR PFX "internal error - overflow on vid write\n");
		return 1;
	}

	lo = currfid | (vid << MSR_C_LO_VID_SHIFT) | MSR_C_LO_INIT_FID_VID;
	dprintk(KERN_DEBUG PFX "writing vid %x, lo %x, hi %x\n",
		vid, lo, STOP_GRANT_5NS);
	wrmsr(MSR_FIDVID_CTL, lo, STOP_GRANT_5NS);
	if (query_current_values_with_pending_wait())
		return 1;

	if (savefid != currfid) {
		dprintk(KERN_ERR PFX "fid changed on vid trans, old %x new %x\n",
		       savefid, currfid);
		return 1;
	}

	if (vid != currvid) {
		dprintk(KERN_ERR PFX "vid trans failed, vid %x, curr %x\n", vid,
		       currvid);
		return 1;
	}

	return 0;
}

/* 
 * Reduce the vid by the max of step or reqvid.
 * Decreasing vid codes represent increasing voltages :
 * vid of 0 is 1.550V, vid of 0x1e is 0.800V, vid of 0x1f is off.
 */
static int decrease_vid_code_by_step(u32 reqvid, u32 step)
{
	if ((currvid - reqvid) > step)
		reqvid = currvid - step;
	if (write_new_vid(reqvid))
		return 1;
	count_off_vst();
	return 0;
}

/* Change the fid and vid, by the 3 phases. */
//static inline int transition_fid_vid(u32 reqfid, u32 reqvid)
static int transition_fid_vid(u32 reqfid, u32 reqvid)
{
	if (core_voltage_pre_transition(reqvid))
		return 1;
	if (core_frequency_transition(reqfid))
		return 1;
	if (core_voltage_post_transition(reqvid))
		return 1;
	if (query_current_values_with_pending_wait())
		return 1;

	if ((reqfid != currfid) || (reqvid != currvid)) {
		dprintk(KERN_ERR PFX "failed: req %x %x, curr %x %x\n",
		       reqfid, reqvid, currfid, currvid);
		return 1;
	}

	dprintk(KERN_INFO PFX
		"transitioned: new fid %x, vid %x\n", currfid, currvid);
	return 0;
}

/* Phase 1 - core voltage transition ... setup voltage */
//static inline int core_voltage_pre_transition(u32 reqvid)
static int core_voltage_pre_transition(u32 reqvid)
{
	u32 rvosteps = rvo;
	u32 savefid = currfid;

	dprintk(KERN_DEBUG PFX
		"ph1: start, currfid %x, currvid %x, reqvid %x, rvo %x\n",
		currfid, currvid, reqvid, rvo);

	while (currvid > reqvid) {
		dprintk(KERN_DEBUG PFX "ph1: curr %x, req vid %x\n",
			currvid, reqvid);
		if (decrease_vid_code_by_step(reqvid, vidmvs))
			return 1;
	}

	while (rvosteps > 0) {
		if (currvid == 0) {
			rvosteps = 0;
		} else {
			dprintk(KERN_DEBUG PFX
				"ph1: changing vid for rvo, req %x\n",
				currvid - 1);
			if (decrease_vid_code_by_step(currvid - 1, 1))
				return 1;
			rvosteps--;
		}
	}

	if (query_current_values_with_pending_wait())
		return 1;

	if (savefid != currfid) {
		dprintk(KERN_ERR PFX "ph1: err, currfid changed %x\n", currfid);
		return 1;
	}

	dprintk(KERN_DEBUG PFX "ph1: complete, currfid %x, currvid %x\n",
		currfid, currvid);

	return 0;
}

/* Phase 2 - core frequency transition */
static inline int core_frequency_transition(u32 reqfid)
{
	u32 vcoreqfid;
	u32 vcocurrfid;
	u32 vcofiddiff;
	u32 savevid = currvid;

	if ((reqfid < HI_FID_TABLE_BOTTOM) && (currfid < HI_FID_TABLE_BOTTOM)) {
		dprintk(KERN_ERR PFX "ph2: illegal lo-lo transition %x %x\n",
		       reqfid, currfid);
		return 1;
	}

	if (currfid == reqfid) {
		dprintk(KERN_ERR PFX "ph2: null fid transition %x\n", currfid);
		return 0;
	}

	dprintk(KERN_DEBUG PFX
		"ph2: starting, currfid %x, currvid %x, reqfid %x\n",
		currfid, currvid, reqfid);

	vcoreqfid = convert_fid_to_vco_fid(reqfid);
	vcocurrfid = convert_fid_to_vco_fid(currfid);
	vcofiddiff = vcocurrfid > vcoreqfid ? vcocurrfid - vcoreqfid
	    : vcoreqfid - vcocurrfid;

	while (vcofiddiff > 2) {
		if (reqfid > currfid) {
			if (currfid > LO_FID_TABLE_TOP) {
				if (write_new_fid(currfid + 2)) {
					return 1;
				}
			} else {
				if (write_new_fid
				    (2 + convert_fid_to_vco_fid(currfid))) {
					return 1;
				}
			}
		} else {
			if (write_new_fid(currfid - 2))
				return 1;
		}

		vcocurrfid = convert_fid_to_vco_fid(currfid);
		vcofiddiff = vcocurrfid > vcoreqfid ? vcocurrfid - vcoreqfid
		    : vcoreqfid - vcocurrfid;
	}

	if (write_new_fid(reqfid))
		return 1;
	if (query_current_values_with_pending_wait())
		return 1;

	if (currfid != reqfid) {
		dprintk(KERN_ERR PFX
		       "ph2: mismatch, failed fid trans, curr %x, req %x\n",
		       currfid, reqfid);
		return 1;
	}

	if (savevid != currvid) {
		dprintk(KERN_ERR PFX "ph2: vid changed, save %x, curr %x\n",
			savevid, currvid);
		return 1;
	}

	dprintk(KERN_DEBUG PFX "ph2: complete, currfid %x, currvid %x\n",
		currfid, currvid);

	return 0;
}

/* Phase 3 - core voltage transition flow ... jump to the final vid. */
static inline int core_voltage_post_transition(u32 reqvid)
{
	u32 savefid = currfid;
	u32 savereqvid = reqvid;

	dprintk(KERN_DEBUG PFX "ph3: starting, currfid %x, currvid %x\n",
		currfid, currvid);

	if (reqvid != currvid) {
		if (write_new_vid(reqvid))
			return 1;

		if (savefid != currfid) {
			dprintk(KERN_ERR PFX
			       "ph3: bad fid change, save %x, curr %x\n",
			       savefid, currfid);
			return 1;
		}

		if (currvid != reqvid) {
			dprintk(KERN_ERR PFX
			       "ph3: failed vid transition\n, req %x, curr %x",
			       reqvid, currvid);
			return 1;
		}
	}

	if (query_current_values_with_pending_wait())
		return 1;

	if (savereqvid != currvid) {
		dprintk(KERN_ERR PFX "ph3: failed, currvid %x\n", currvid);
		return 1;
	}

	if (savefid != currfid) {
		dprintk(KERN_ERR PFX "ph3: failed, currfid changed %x\n",
			currfid);
		return 1;
	}

	dprintk(KERN_DEBUG PFX "ph3: complete, currfid %x, currvid %x\n",
		currfid, currvid);
	return 0;
}

static inline int check_supported_cpu(void)
{
	struct cpuinfo_x86 *c = &new_cpu_data;
	u32 eax, ebx, ecx, edx;

	if (num_online_cpus() != 1) {
		dprintk(KERN_INFO PFX "multiprocessor systems not supported\n");
		return 0;
	}

	if (c->x86_vendor != X86_VENDOR_AMD)
		return 0;

	eax = cpuid_eax(CPUID_PROCESSOR_SIGNATURE);
	if (((eax & CPUID_USE_XFAM_XMOD) != CPUID_USE_XFAM_XMOD) ||
	    ((eax & CPUID_XFAM) != CPUID_XFAM_K8) ||
	    ((eax & CPUID_XMOD) > CPUID_XMOD_REV_E)) {
		dprintk(KERN_INFO PFX "Processor cpuid %x not supported\n", eax);
		return 0;
	} else {
		dprintk(KERN_INFO PFX "AMD Athlon 64 or AMD Opteron processor found\n");
	}

	eax = cpuid_eax(CPUID_GET_MAX_CAPABILITIES);
	if (eax < CPUID_FREQ_VOLT_CAPABILITIES) {
		dprintk(KERN_INFO PFX "No freq change capabilities\n");
		return 0;
	}

	cpuid(CPUID_FREQ_VOLT_CAPABILITIES, &eax, &ebx, &ecx, &edx);
	if ((edx & P_STATE_TRANSITION_CAPABLE) != P_STATE_TRANSITION_CAPABLE) {
		dprintk(KERN_INFO PFX "Power state transitions not supported\n");
		return 0;
	}

	dprintk(KERN_INFO PFX "Found AMD Athlon 64 / Opteron processor\n");
	return 1;
}

static int check_pst_table(struct pst_s *pst, u8 maxvid)
{
	unsigned int j;
	u8 lastfid = 0xff;

	for (j = 0; j < numps; j++) {
		if (pst[j].vid > LEAST_VID) {
			dprintk(KERN_ERR PFX "vid %d bad: %x\n", j, pst[j].vid);
			return -EINVAL;
		}
		if (pst[j].vid < rvo) {	/* vid + rvo >= 0 */
			dprintk(KERN_ERR PFX "0 vid exceeded with pst %d\n", j);
			return -ENODEV;
		}
		if (pst[j].vid < maxvid + rvo) { /* vid + rvo >= maxvid */
			dprintk(KERN_ERR PFX "maxvid exceeded with pst %d\n", j);
			return -ENODEV;
		}
		if ((pst[j].fid > MAX_FID)
		    || (pst[j].fid & 1)
		    || (j && (pst[j].fid < HI_FID_TABLE_BOTTOM))) {
			dprintk(KERN_ERR PFX "fid %d bad: %x\n", j, pst[j].fid);
			return -EINVAL;
		}
		if (pst[j].fid < lastfid)
			lastfid = pst[j].fid;
	}
	if (lastfid & 1) {
		dprintk(KERN_ERR PFX "lastfid invalid\n");
		return -EINVAL;
	}
	if (lastfid > LO_FID_TABLE_TOP)
		dprintk(KERN_INFO PFX "first fid not from lo freq table\n");

	return 0;
}

/* Find and validate the PSB/PST table in BIOS. */
static inline int find_psb_table(void)
{
	struct psb_s *psb;
	struct pst_s *pst;
	unsigned int i, j;
	u32 mvs;
	u8 maxvid;

	for (i = 0xc0000; i < 0xffff0; i += 0x10) {
		/* Scan BIOS looking for the signature. */
		/* It can not be at ffff0 - it is too big. */

		psb = phys_to_virt(i);
		if (memcmp(psb, PSB_ID_STRING, PSB_ID_STRING_LEN) != 0)
			continue;

		dprintk(KERN_DEBUG PFX "found PSB header at %p\n", psb);
		dprintk(KERN_DEBUG PFX "table version: %x\n",
				psb->tableversion);
		if (psb->tableversion != PSB_VERSION_1_4) {
			dprintk(KERN_INFO PFX "PSB table is not v1.4\n");
			return -ENODEV;
		}

		dprintk(KERN_DEBUG PFX "flags: %x\n", psb->flags1);
		if (psb->flags1) {
			dprintk(KERN_ERR PFX "unknown flags\n");
			return -ENODEV;
		}

		vstable = psb->voltagestabilizationtime;
		dprintk(KERN_INFO PFX "voltage stabilization time: %d(*20us)\n",
		       vstable);

		dprintk(KERN_DEBUG PFX "flags2: %x\n", psb->flags2);
		rvo = psb->flags2 & 3;
		irt = ((psb->flags2) >> 2) & 3;
		mvs = ((psb->flags2) >> 4) & 3;
		vidmvs = 1 << mvs;
                batps = ((psb->flags2) >> 6) & 3;
		if (batps)
			dprintk(KERN_INFO PFX "only %d pstates on battery\n",
				batps );

		dprintk(KERN_INFO PFX "ramp voltage offset: %d\n", rvo);
		dprintk(KERN_INFO PFX "isochronous relief time: %d\n", irt);
		dprintk(KERN_INFO PFX "maximum voltage step: %d - %x\n",
			mvs, vidmvs);

		if (psb->numpst != 1) {
			dprintk(KERN_ERR PFX "numpst must be 1\n");
			return -ENODEV;
		}

		plllock = psb->plllocktime;
		dprintk(KERN_INFO PFX "plllocktime: %x (units 1us)\n",
		       psb->plllocktime);
		dprintk(KERN_INFO PFX "maxfid: %x\n", psb->maxfid);
		dprintk(KERN_INFO PFX "maxvid: %x\n", psb->maxvid);
		maxvid = psb->maxvid;

		numps = psb->numpstates;
		dprintk(KERN_INFO PFX "numpstates: %x\n", numps);
		if (numps < 2) {
			dprintk(KERN_ERR PFX "no p states to transition\n");
			return -ENODEV;
		}

		pst = (struct pst_s *)(psb + 1);
		if (check_pst_table(pst, maxvid))
			return -EINVAL;

		ftbl = kmalloc((sizeof(struct cpufreq_frequency_table)
                                              * (numps + 1)), GFP_KERNEL);
		if (!ftbl) {
			dprintk(KERN_ERR PFX "ftbl memory alloc failure\n");
			return -ENOMEM;
		}

		for (j = 0; j < numps; j++) {
			dprintk(KERN_INFO PFX "   %d : fid %x, vid %x\n", j,
				       pst[j].fid, pst[j].vid);

			ftbl[j].index = pst[j].fid; /* lower 8 bits */
			ftbl[j].index |= (pst[j].vid << 8); /* upper 8 bits */
			ftbl[j].frequency = find_khz_freq_from_fid(pst[j].fid);
		}
		ftbl[numps].frequency = CPUFREQ_TABLE_END;
		ftbl[numps].index = 0;

		if (query_current_values_with_pending_wait()) {
			kfree(ftbl);
			return 1;
		}
		dprintk(KERN_INFO PFX "cfid %x, cvid %x\n", currfid, currvid);

		for (j = 0; j < numps; j++)
			if ((pst[j].fid == currfid) && (pst[j].vid == currvid))
					return (0);
		dprintk(KERN_ERR PFX "currfid/vid do not match PST, ignoring\n");
		return 0;
	}

	dprintk(KERN_ERR PFX "BIOS error - no PSB\n");

#if 0
        /* hack for machines without a PSB - hardcode 2.0/1.8/0.8 GHz  */
	/* use this hack at your own risk                             */
        vstable = 5;
        rvo = 2;
        irt = 2;
        mvs = 1;
        vidmvs = 1 << mvs;
        batps = numps = 3;
        plllock = 2;

	ftbl = kmalloc((sizeof(struct cpufreq_frequency_table)
                                              * (numps + 1)), GFP_KERNEL);
	if (!ftbl)
		return -ENOMEM;

        ftbl[0].index = 0x00;        /* 800 MHz */
        ftbl[0].index |= 0x12 << 8;  /* 1.100v */

        ftbl[0].frequency = find_khz_freq_from_fid( ftbl[0].index & 0x0f );

        ftbl[1].index = 0x0a;        /* 1.8 GHz */
        ftbl[1].index |= 0x03 << 8;  /* 1.475v */

        ftbl[1].frequency = find_khz_freq_from_fid( ftbl[1].index & 0x0f );

        ftbl[2].index = 0x0c;        /* 2.0 GHz */
        ftbl[2].index |= 0x02 << 8;  /* 1.500v */

        ftbl[2].frequency =  find_khz_freq_from_fid( ftbl[2].index & 0x0f );

	ftbl[numps].frequency = CPUFREQ_TABLE_END;
	ftbl[numps].index = 0;

	if (query_current_values_with_pending_wait()) {
		kfree(ftbl);
		return 1;
	}
	dprintk(KERN_INFO PFX "currfid %x, currvid %x\n",
	       currfid, currvid);

        return 0;
#endif

        return -ENODEV;
}

/* Take a frequency, and issue the fid/vid transition command */
static inline int transition_frequency(unsigned int index)
{
	u32 fid;
	u32 vid;
	int res;
	struct cpufreq_freqs freqs;

        dprintk(KERN_DEBUG PFX "transition to index %u\n", index );

	/* fid are the lower 8 bits of the index we stored into
	 * the cpufreq frequency table in find_psb_table, vid are 
	 * the upper 8 bits.
	 */

	fid = ftbl[index].index & 0xFF;
	vid = (ftbl[index].index & 0xFF00) >> 8;

	dprintk(KERN_DEBUG PFX "matched fid %x, giving vid %x\n", fid, vid);

	if (query_current_values_with_pending_wait())
		return 1;

	if ((currvid == vid) && (currfid == fid)) {
		dprintk(KERN_DEBUG PFX "target matches curr (fid %x, vid %x)\n",
			fid, vid);
		return 0;
	}

	if ((fid < HI_FID_TABLE_BOTTOM) && (currfid < HI_FID_TABLE_BOTTOM)) {
		dprintk(KERN_ERR PFX
		       "ignoring illegal change in lo freq table-%x to %x\n",
		       currfid, fid);
		return 1;
	}

	dprintk(KERN_DEBUG PFX "changing to fid %x, vid %x\n", fid, vid);

	freqs.cpu = 0;		/* only true because SMP not supported */
	freqs.old = find_freq_from_fid(currfid);
	freqs.new = find_freq_from_fid(fid);
	cpufreq_notify_transition(&freqs, CPUFREQ_PRECHANGE);

	res = transition_fid_vid(fid, vid);

	freqs.new = find_freq_from_fid(currfid);
	cpufreq_notify_transition(&freqs, CPUFREQ_POSTCHANGE);

	return res;
}

/* Driver entry point to switch to the target frequency */
static int
drv_target(struct cpufreq_policy *pol, unsigned targfreq, unsigned relation)
{
	u32 checkfid = currfid;
	u32 checkvid = currvid;
	unsigned int newstate;

	if (pending_bit_stuck()) {
		dprintk(KERN_ERR PFX "failing targ, change pending bit set\n");
		return -EIO;
	}

	dprintk(KERN_DEBUG PFX "targ: %d kHz, min %d, max %d, relation %d\n",
		targfreq, pol->min, pol->max, relation);

	if (query_current_values_with_pending_wait())
		return -EIO;
	dprintk(KERN_DEBUG PFX "targ: curr fid %x, vid %x\n",
		currfid, currvid);

	if ((checkvid != currvid) || (checkfid != currfid)) {
		dprintk(KERN_ERR PFX "out of sync, fid %x %x, vid %x %x\n",
		       checkfid, currfid, checkvid, currvid);
	}

	if (cpufreq_frequency_table_target(pol, ftbl, targfreq, relation,
						&newstate))
		return -EINVAL;
	
	if (transition_frequency(newstate))
	{
		dprintk(KERN_ERR PFX "transition frequency failed\n");
		return 1;
	}

	pol->cur = find_khz_freq_from_fid(currfid);
	return 0;
}

/* Driver entry point to verify the policy and range of frequencies */
static int drv_verify(struct cpufreq_policy *pol)
{
	if (pending_bit_stuck()) {
		dprintk(KERN_ERR PFX "failing verify, change pending bit set\n");
		return -EIO;
	}

	return cpufreq_frequency_table_verify(pol, ftbl);
}

/* per CPU init entry point to the driver */
static int __init
drv_cpu_init(struct cpufreq_policy *pol)
{
	if (pol->cpu != 0) {
		dprintk(KERN_ERR PFX "init - cpu 0\n");
		return -ENODEV;
	}

	pol->governor = 0; //!!! CPUFREQ_DEFAULT_GOVERNOR;

	/* Take a crude guess here. */
	pol->cpuinfo.transition_latency = ((rvo + 8) * vstable * VST_UNITS_20US)
	    + (3 * (1 << irt) * 10);

	if (query_current_values_with_pending_wait())
		return -EIO;

	pol->cur = find_khz_freq_from_fid(currfid);
	dprintk(KERN_DEBUG PFX "policy current frequency %d kHz\n", pol->cur);

	/* min/max the cpu is capable of */
	if (cpufreq_frequency_table_cpuinfo(pol, ftbl)) {
		dprintk(KERN_ERR PFX "invalid ftbl\n");
		kfree(ftbl);
		return -EINVAL;
	}

	/* Added by Nino */
	cpufreq_frequency_table_get_attr(ftbl, pol->cpu);

	dprintk(KERN_INFO PFX "init, curr fid %x vid %x\n", currfid, currvid);
	return 0;
}


static int __exit drv_cpu_exit (struct cpufreq_policy *pol)
{
	if (pol->cpu != 0)
		return -EINVAL;

	/* Added by Nino */
	cpufreq_frequency_table_put_attr(pol->cpu);

	kfree(ftbl);

	return 0;
}

static struct cpufreq_driver cpufreq_amd64_driver = {
	.verify = drv_verify,
	.target = drv_target,
	.init = drv_cpu_init,
	.exit = drv_cpu_exit,
	.name = "powernow-k8",
	.owner = THIS_MODULE
};


/* driver entry point for init */
/*static*/ int __init powernowk8_init(void)
{
	int rc;

	dprintk(KERN_INFO PFX VERSION "\n");

	if (check_supported_cpu() == 0)
		return -ENODEV;

	rc = find_psb_table();
	if (rc)
		return rc;

	if (pending_bit_stuck()) {
		dprintk(KERN_ERR PFX "failing init, change pending bit set\n");
		return -EIO;
	}

	return cpufreq_register_driver(&cpufreq_amd64_driver);
}

/* driver entry point for term */
/*static*/ void __exit powernowk8_exit(void)
{
	dprintk(KERN_INFO PFX "exit\n");
	cpufreq_unregister_driver(&cpufreq_amd64_driver);
}

MODULE_AUTHOR("Paul Devriendt <paul.devriendt@amd.com>");
MODULE_DESCRIPTION("AMD Athlon 64 and Opteron processor frequency driver.");
MODULE_LICENSE("GPL");

module_init(powernowk8_init);
module_exit(powernowk8_exit);
