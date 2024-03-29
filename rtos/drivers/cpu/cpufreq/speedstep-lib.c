/*
 * (C) 2002 - 2003 Dominik Brodowski <linux@brodo.de>
 *
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  Library for common functions for Intel SpeedStep v.1 and v.2 support
 *
 *  BIG FAT DISCLAIMER: Work in progress code. Possibly *dangerous*
 */

#include <linuxcomp.h>

#include <linux/kernel.h>
#include <linux/module.h> 
#include <linux/init.h>
#include <linux/cpufreq.h>
#include <linux/pci.h>
#include <linux/slab.h>

#include <asm/msr.h>
#include "speedstep-lib.h"


/* DEBUG
 *   Define it if you want verbose debug output, e.g. for bug reporting
 */
//#define SPEEDSTEP_DEBUG

#ifdef SPEEDSTEP_DEBUG
#define dprintk(msg...) printk(msg)
#else
#define dprintk(msg...) do { } while(0)
#endif

/*********************************************************************
 *                   GET PROCESSOR CORE SPEED IN KHZ                 *
 *********************************************************************/

static unsigned int pentium3_get_frequency (unsigned int processor)
{
        /* See table 14 of p3_ds.pdf and table 22 of 29834003.pdf */
	struct {
		unsigned int ratio;	/* Frequency Multiplier (x10) */
		u8 bitmap;	        /* power on configuration bits
					   [27, 25:22] (in MSR 0x2a) */
	} msr_decode_mult [] = {
		{ 30, 0x01 },
		{ 35, 0x05 },
		{ 40, 0x02 },
		{ 45, 0x06 },
		{ 50, 0x00 },
		{ 55, 0x04 },
		{ 60, 0x0b },
		{ 65, 0x0f },
		{ 70, 0x09 },
		{ 75, 0x0d },
		{ 80, 0x0a },
		{ 85, 0x26 },
		{ 90, 0x20 },
		{ 100, 0x2b },
		{ 0, 0xff }     /* error or unknown value */
	};

	/* PIII(-M) FSB settings: see table b1-b of 24547206.pdf */
	struct {
		unsigned int value;     /* Front Side Bus speed in MHz */
		u8 bitmap;              /* power on configuration bits [18: 19]
					   (in MSR 0x2a) */
	} msr_decode_fsb [] = {
		{  66, 0x0 },
		{ 100, 0x2 },
		{ 133, 0x1 },
		{   0, 0xff}
	};

	u32     msr_lo, msr_tmp;
	int     i = 0, j = 0;

	/* read MSR 0x2a - we only need the low 32 bits */
	rdmsr(MSR_IA32_EBL_CR_POWERON, msr_lo, msr_tmp);
	dprintk(KERN_DEBUG "speedstep-lib: P3 - MSR_IA32_EBL_CR_POWERON: 0x%x 0x%x\n", msr_lo, msr_tmp);
	msr_tmp = msr_lo;

	/* decode the FSB */
	msr_tmp &= 0x00c0000;
	msr_tmp >>= 18;
	while (msr_tmp != msr_decode_fsb[i].bitmap) {
		if (msr_decode_fsb[i].bitmap == 0xff)
			return 0;
		i++;
	}

	/* decode the multiplier */
	if (processor == SPEEDSTEP_PROCESSOR_PIII_C_EARLY)
		msr_lo &= 0x03c00000;
	else
		msr_lo &= 0x0bc00000;
	msr_lo >>= 22;
	while (msr_lo != msr_decode_mult[j].bitmap) {
		if (msr_decode_mult[j].bitmap == 0xff)
			return 0;
		j++;
	}

	return (msr_decode_mult[j].ratio * msr_decode_fsb[i].value * 100);
}


static unsigned int pentium4_get_frequency(void)
{
	u32 msr_lo, msr_hi;

	rdmsr(0x2c, msr_lo, msr_hi);

	dprintk(KERN_DEBUG "speedstep-lib: P4 - MSR_EBC_FREQUENCY_ID: 0x%x 0x%x\n", msr_lo, msr_hi);

	msr_lo >>= 24;
	return (msr_lo * 100000);
}

 
unsigned int speedstep_get_processor_frequency(unsigned int processor)
{
	switch (processor) {
	case SPEEDSTEP_PROCESSOR_P4M:
		return pentium4_get_frequency();
	case SPEEDSTEP_PROCESSOR_PIII_T:
	case SPEEDSTEP_PROCESSOR_PIII_C:
	case SPEEDSTEP_PROCESSOR_PIII_C_EARLY:
		return pentium3_get_frequency(processor);
	default:
		return 0;
	};
	return 0;
}
EXPORT_SYMBOL_GPL(speedstep_get_processor_frequency);


/*********************************************************************
 *                 DETECT SPEEDSTEP-CAPABLE PROCESSOR                *
 *********************************************************************/

unsigned int speedstep_detect_processor (void)
{
	struct cpuinfo_x86 *c = cpu_data;
	u32			ebx, msr_lo, msr_hi;

	if ((c->x86_vendor != X86_VENDOR_INTEL) || 
	    ((c->x86 != 6) && (c->x86 != 0xF)))
		return 0;

	if (c->x86 == 0xF) {
		/* Intel Mobile Pentium 4-M
		 * or Intel Mobile Pentium 4 with 533 MHz FSB */
		if (c->x86_model != 2)
			return 0;

		if ((c->x86_mask != 4) && /* B-stepping [M-P4-M] */
			(c->x86_mask != 7) && /* C-stepping [M-P4-M] */
			(c->x86_mask != 9))   /* D-stepping [M-P4-M or M-P4/533] */
			return 0;

		ebx = cpuid_ebx(0x00000001);
		ebx &= 0x000000FF;
		if ((ebx != 0x0e) && (ebx != 0x0f))
			return 0;

		return SPEEDSTEP_PROCESSOR_P4M;
	}

	switch (c->x86_model) {
	case 0x0B: /* Intel PIII [Tualatin] */
		/* cpuid_ebx(1) is 0x04 for desktop PIII, 
		                   0x06 for mobile PIII-M */
		ebx = cpuid_ebx(0x00000001);

		ebx &= 0x000000FF;
		if (ebx != 0x06)
			return 0;

		/* So far all PIII-M processors support SpeedStep. See
		 * Intel's 24540640.pdf of June 2003 
		 */

		return SPEEDSTEP_PROCESSOR_PIII_T;

	case 0x08: /* Intel PIII [Coppermine] */

		/* all mobile PIII Coppermines have FSB 100 MHz
		 * ==> sort out a few desktop PIIIs. */
		rdmsr(MSR_IA32_EBL_CR_POWERON, msr_lo, msr_hi);
		dprintk(KERN_DEBUG "cpufreq: Coppermine: MSR_IA32_EBL_CR_POWERON is 0x%x, 0x%x\n", msr_lo, msr_hi);
		msr_lo &= 0x00c0000;
		if (msr_lo != 0x0080000)
			return 0;

		/*
		 * If the processor is a mobile version,
		 * platform ID has bit 50 set
		 * it has SpeedStep technology if either
		 * bit 56 or 57 is set
		 */
		rdmsr(MSR_IA32_PLATFORM_ID, msr_lo, msr_hi);
		dprintk(KERN_DEBUG "cpufreq: Coppermine: MSR_IA32_PLATFORM ID is 0x%x, 0x%x\n", msr_lo, msr_hi);
		if ((msr_hi & (1<<18)) && (msr_hi & (3<<24))) {
			if (c->x86_mask == 0x01)
				return SPEEDSTEP_PROCESSOR_PIII_C_EARLY;
			else
				return SPEEDSTEP_PROCESSOR_PIII_C;
		}

	default:
		return 0;
	}
}
EXPORT_SYMBOL_GPL(speedstep_detect_processor);


/*********************************************************************
 *                     DETECT SPEEDSTEP SPEEDS                       *
 *********************************************************************/

unsigned int speedstep_get_freqs(unsigned int processor,
				  unsigned int *low_speed,
				  unsigned int *high_speed,
				  void (*set_state) (unsigned int state,
						     unsigned int notify)
				 )
{
	unsigned int prev_speed;
	unsigned int ret = 0;
	unsigned long flags;

	if ((!processor) || (!low_speed) || (!high_speed) || (!set_state))
		return -EINVAL;

	/* get current speed */
	prev_speed = speedstep_get_processor_frequency(processor);
	if (!prev_speed)
		return -EIO;
	
	local_irq_save(flags);

	/* switch to low state */
	set_state(SPEEDSTEP_LOW, 0);
	*low_speed = speedstep_get_processor_frequency(processor);
	if (!*low_speed) {
		ret = -EIO;
		goto out;
	}

	/* switch to high state */
	set_state(SPEEDSTEP_HIGH, 0);
	*high_speed = speedstep_get_processor_frequency(processor);
	if (!*high_speed) {
		ret = -EIO;
		goto out;
	}

	if (*low_speed == *high_speed) {
		ret = -ENODEV;
		goto out;
	}

	/* switch to previous state, if necessary */
	if (*high_speed != prev_speed)
		set_state(SPEEDSTEP_LOW, 0);

 out:
	local_irq_restore(flags);
	return (ret);
}
EXPORT_SYMBOL_GPL(speedstep_get_freqs);

MODULE_AUTHOR ("Dominik Brodowski <linux@brodo.de>");
MODULE_DESCRIPTION ("Library for Intel SpeedStep 1 or 2 cpufreq drivers.");
MODULE_LICENSE ("GPL");
