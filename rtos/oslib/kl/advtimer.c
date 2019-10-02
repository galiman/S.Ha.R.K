/* Project:     OSLib
 * Description: The OS Construction Kit
 * Date:                1.6.2000
 * Idea by:             Luca Abeni & Gerardo Lamastra
 *
 * OSLib is an SO project aimed at developing a common, easy-to-use
 * low-level infrastructure for developing OS kernels and Embedded
 * Applications; it partially derives from the HARTIK project but it
 * currently is independently developed.
 *
 * OSLib is distributed under GPL License, and some of its code has
 * been derived from the Linux kernel source; also some important
 * ideas come from studying the DJGPP go32 extender.
 *
 * We acknowledge the Linux Community, Free Software Foundation,
 * D.J. Delorie and all the other developers who believe in the
 * freedom of software and ideas.
 *
 * For legalese, check out the included GPL license.
 */

/*	Advanced Timer Managment
 *	Author: Giacomo Guidi <giacomo@gandalf.sssup.it>
 */

#include <ll/i386/stdlib.h>
#include <ll/i386/error.h>
#include <ll/sys/ll/ll-data.h>
#include <ll/sys/ll/ll-func.h>
#include <ll/i386/pic.h>
#include <ll/i386/apic.h>
#include <ll/i386/64bit.h>
#include <ll/sys/ll/event.h>
#include <ll/sys/ll/time.h>
#include <ll/i386/advtimer.h>

#define CALIBRATE_USING_CMOS

unsigned long long init_tsc;
unsigned long long * ptr_init_tsc = &init_tsc;

struct timespec init_time;
struct timespec * ptr_init_time = &init_time;

unsigned int clk_per_msec = 0;
unsigned int apic_clk_per_msec = 0;
unsigned int apic_set_limit = 0;

/* Precalcolated const 
   used in ll_read_timer */
unsigned int clk_opt_0 = 0;
unsigned int clk_opt_1 = 0;
unsigned int clk_opt_2 = 0;
unsigned int clk_opt_3 = 0;
unsigned int clk_opt_4 = 0;
unsigned int clk_opt_5 = 0;

unsigned char save_CMOS_regA;
unsigned char save_CMOS_regB;

unsigned long msr_original_low, msr_original_high;

unsigned char X86_tsc = 0;
unsigned char X86_apic = 0;
unsigned char use_tsc = 0;
unsigned char use_apic = 0;

#ifdef CONFIG_MELAN
#  define CLOCK_TICK_RATE 1189200 /* AMD Elan has different frequency! */
#else
#  define CLOCK_TICK_RATE 1193182 /* Underlying HZ */
#endif

#define COUNTER_END 100

#define barrier() __asm__ __volatile__("" ::: "memory");

//TSC Calibration (idea from the linux kernel code)
void ll_calibrate_tsc(void)
{

	unsigned long long start;
	unsigned long long end;
	unsigned long long dtsc;
	
	unsigned int start_8253, end_8253, delta_8253;

	outp(0x61, (inp(0x61) & ~0x02) | 0x01);

	outp(0x43,0xB0);			/* binary, mode 0, LSB/MSB, Ch 2 */
	outp(0x42,0xFF);			/* LSB of count */
	outp(0x42,0xFF);			/* MSB of count */
        
	barrier();
	rdtscll(start);
	barrier();
	outp(0x43,0x00);
	start_8253 = inp(0x42);
	start_8253 |= inp(0x42) << 8;
	barrier();
	rdtscll(start);
	barrier();
 
	do {
	    outp(0x43,0x00);
	    end_8253 = inp(0x42);
	    end_8253 |= inp(0x42) << 8;
	} while (end_8253 > COUNTER_END);

	barrier();
	rdtscll(end);
	barrier();
	outp(0x43,0x00);
	end_8253 = inp(0x42);
	end_8253 |= inp(0x42) << 8;
	barrier();
	rdtscll(end);
	barrier();

	//Delta TSC
	dtsc = end - start;

	//Delta PIT
	delta_8253 = start_8253 - end_8253;

	if (delta_8253 > 0x20000) {
		message("Error calculating Delta PIT\n");
		ll_abort(10);
	}

	message("Delta TSC               = %10d\n",(int)dtsc);

	message("Delta PIT               = %10d\n",delta_8253);

	clk_per_msec = dtsc * CLOCK_TICK_RATE / delta_8253 / 1000;
	
	message("Calibrated Clk_per_msec = %10d\n",clk_per_msec);
}

#define CMOS_INIT  0
#define CMOS_BEGIN 1
#define CMOS_START 2
#define CMOS_END   3

int cmos_calibrate_status = CMOS_INIT;
unsigned long long irq8_start;
unsigned long long irq8_end;

void calibrate_tsc_IRQ8(void *p)
{
	unsigned char set;

	CMOS_READ(0x0C,set);

	barrier();
	rdtscll(irq8_end);
	barrier();

	if (cmos_calibrate_status == CMOS_START) {
		cmos_calibrate_status = CMOS_END;
	}

	if (cmos_calibrate_status == CMOS_BEGIN) {
		irq8_start = irq8_end;
		cmos_calibrate_status = CMOS_START;
	}

	if (cmos_calibrate_status == CMOS_INIT) {
		cmos_calibrate_status = CMOS_BEGIN;
	}
}

//TSC Calibration using RTC
void ll_calibrate_tsc_cmos(void)
{
	unsigned long long dtsc;

	irq_bind(8, calibrate_tsc_IRQ8, INT_FORCE);

	CMOS_READ(0x0A,save_CMOS_regA);
	CMOS_READ(0x0B,save_CMOS_regB);
    
	CMOS_WRITE(0x0A,0x2F); // Set 2 Hz Periodic Interrupt
	CMOS_WRITE(0x0B,0x42); // Enable Interrupt

	irq_unmask(8);
  
	sti();

	while (cmos_calibrate_status != CMOS_END) {
		barrier();
	}
  
	cli();

	dtsc = irq8_end - irq8_start;

	clk_per_msec = dtsc / 500;
	clk_opt_0 = (unsigned int)(dtsc);
	clk_opt_1 = (unsigned int)((unsigned long long)(dtsc << 1));
	clk_opt_2 = (unsigned int)((unsigned long long)(dtsc << 33) / 1000000000L);
	clk_opt_3 = (unsigned int)((unsigned long long)(dtsc << 32) / 1000000000L);
	clk_opt_4 = (unsigned int)((unsigned long long)(dtsc << 31) / 1000000000L);
	clk_opt_5 = (unsigned int)((unsigned long long)(dtsc << 30) / 1000000000L);

	message("Calibrated CPU Clk/msec  = %10u\n",clk_per_msec);

#ifdef __O1000__
	if (clk_per_msec < 1000000) {
		message("Timer Optimization CPU < 1 GHz\n");
	} else {
		message("Bad Timer Optimization\n");
		ll_abort(66);
	}
#endif

#ifdef __O2000__
	if (clk_per_msec < 2000000 && clk_per_msec >= 1000000) {
		message("Timer Optimization 1 GHz < CPU < 2 GHz\n");
	} else {
		message("Bad Timer Optimization\n");
		ll_abort(66);
	}
#endif

#ifdef __O4000__
	if (clk_per_msec < 4000000 && clk_per_msec >= 2000000) {
		message("Timer Optimization 2 GHz < CPU < 4 GHz\n");
	} else {
		message("Bad Timer Optimization\n");
		ll_abort(66);
	}
#endif

	irq_mask(8);

	CMOS_WRITE(0x0A,save_CMOS_regA);
	CMOS_WRITE(0x0B,save_CMOS_regB);
}

int apic_get_maxlvt(void)
{
	unsigned int v, ver, maxlvt;

	v = apic_read(APIC_LVR);
	ver = GET_APIC_VERSION(v);
	/* 82489DXs do not report # of LVT entries. */
	maxlvt = APIC_INTEGRATED(ver) ? GET_APIC_MAXLVT(v) : 2;
	return maxlvt;
}

/* Clear local APIC, from Linux kernel */
void clear_local_APIC(void)
{
	int maxlvt;
	unsigned long v;

	maxlvt = apic_get_maxlvt();

	/*
	 * Masking an LVT entry on a P6 can trigger a local APIC error
	 * if the vector is zero. Mask LVTERR first to prevent this.
	 */
	if (maxlvt >= 3) {
		v = 0xFF; /* any non-zero vector will do */
		apic_write_around(APIC_LVTERR, v | APIC_LVT_MASKED);
	}
	/*
	 * Careful: we have to set masks only first to deassert
	 * any level-triggered sources.
	 */
	v = apic_read(APIC_LVTT);
	apic_write_around(APIC_LVTT, v | APIC_LVT_MASKED);
	v = apic_read(APIC_LVT0);
	apic_write_around(APIC_LVT0, v | APIC_LVT_MASKED);
	v = apic_read(APIC_LVT1);
	apic_write_around(APIC_LVT1, v | APIC_LVT_MASKED);
	if (maxlvt >= 4) {
		v = apic_read(APIC_LVTPC);
		apic_write_around(APIC_LVTPC, v | APIC_LVT_MASKED);
	}

	/*
	 * Clean APIC state for other OSs:
	 */
	apic_write_around(APIC_LVTT, APIC_LVT_MASKED);
	apic_write_around(APIC_LVT0, APIC_LVT_MASKED);
	apic_write_around(APIC_LVT1, APIC_LVT_MASKED);
	if (maxlvt >= 3)
		apic_write_around(APIC_LVTERR, APIC_LVT_MASKED);
	if (maxlvt >= 4)
		apic_write_around(APIC_LVTPC, APIC_LVT_MASKED);
	v = GET_APIC_VERSION(apic_read(APIC_LVR));
	if (APIC_INTEGRATED(v)) {	/* !82489DX */
		if (maxlvt > 3)
			apic_write(APIC_ESR, 0);
		apic_read(APIC_ESR);
	}
}

void disable_local_APIC(void)
{
	unsigned long value;

	clear_local_APIC();

	/*
	 * Disable APIC (implies clearing of registers
	 * for 82489DX!).
	 */
	value = apic_read(APIC_SPIV);
	value &= ~APIC_SPIV_APIC_ENABLED;
	apic_write_around(APIC_SPIV, value);
}

#define SPURIOUS_APIC_VECTOR 0xFF

/*
 * Setup the local APIC, minimal code to run P6 APIC
 */
void setup_local_APIC (void)
{
	unsigned long value;

	/* Pound the ESR really hard over the head with a big hammer - mbligh */

	apic_write(APIC_ESR, 0);
	apic_write(APIC_ESR, 0);
	apic_write(APIC_ESR, 0);
	apic_write(APIC_ESR, 0);

	value = APIC_SPIV_FOCUS_DISABLED | APIC_SPIV_APIC_ENABLED | SPURIOUS_APIC_VECTOR;
	apic_write_around(APIC_SPIV, value);

	value = APIC_DM_EXTINT | APIC_LVT_LEVEL_TRIGGER;
	apic_write_around(APIC_LVT0, value);

	value = APIC_DM_NMI;
	apic_write_around(APIC_LVT1, value);

	apic_write(APIC_ESR, 0);
}

void disable_APIC_timer(void)
{
	unsigned long v;
                                                                                                                             
	v = apic_read(APIC_LVTT);
	apic_write_around(APIC_LVTT, v | APIC_LVT_MASKED);
}
                                                                                                                             
void enable_APIC_timer(void)
{
	unsigned long v;
                                                                                                                             
	v = apic_read(APIC_LVTT);
	apic_write_around(APIC_LVTT, v & ~APIC_LVT_MASKED);
}

#define LOCAL_TIMER_VECTOR 0x39

/* Set APIC Timer... from Linux kernel */
void setup_APIC_timer()
{
	unsigned int lvtt1_value;

	lvtt1_value = SET_APIC_TIMER_BASE(APIC_TIMER_BASE_DIV) |
			APIC_LVT_TIMER_PERIODIC | LOCAL_TIMER_VECTOR;
	apic_write_around(APIC_LVTT, lvtt1_value);

	/*
	* Divide PICLK by 1
	*/
	apic_write_around(APIC_TDCR, APIC_TDR_DIV_1);

	apic_write_around(APIC_TMICT, MAX_DWORD);

	disable_APIC_timer();                                                                                                                             
}

#define APIC_LIMIT 0xFF000000
#define APIC_SET_LIMIT 10

void ll_calibrate_apic(void)
{
	unsigned int apic_start = 0, apic_end = 0, dapic;
	unsigned long long tsc_start = 0, tsc_end = 0, dtsc;
	unsigned int tmp_value;

	tmp_value = SET_APIC_TIMER_BASE(APIC_TIMER_BASE_DIV) | LOCAL_TIMER_VECTOR;
	apic_write_around(APIC_LVTT, tmp_value);

	apic_write_around(APIC_TDCR, APIC_TDR_DIV_1);

	apic_write(APIC_TMICT, MAX_DWORD);

	enable_APIC_timer();

	barrier();
	rdtscll(tsc_start);
	barrier();
	apic_start = apic_read(APIC_TMCCT);
	barrier();            

	while (apic_read(APIC_TMCCT) > APIC_LIMIT) {
		barrier();
		rdtscll(tsc_end);
	}

	barrier();
	rdtscll(tsc_end);
	barrier();
	apic_end = apic_read(APIC_TMCCT);
	barrier();     

	disable_APIC_timer();

	dtsc = tsc_end - tsc_start;
	dapic = apic_start - apic_end;
	
	apic_clk_per_msec = (unsigned long long)(clk_per_msec) * (unsigned long long)(dapic) / dtsc;
	apic_set_limit = ((apic_clk_per_msec / 100) != 0) ? (apic_clk_per_msec/100) : APIC_SET_LIMIT;  

	message("Calibrated APIC Clk/msec = %10d\n",apic_clk_per_msec);
}

void ll_init_advtimer()
{

#ifdef __TSC__
        use_tsc = X86_tsc;

#ifdef __APIC__
	use_apic = X86_apic;
#endif

#endif

	if (use_tsc == 0) use_apic = 0;

	if (use_tsc) {
 
#ifdef CALIBRATE_USING_CMOS
		ll_calibrate_tsc_cmos();
#else
		ll_calibrate_tsc();
#endif	

		rdtscll(init_tsc); // Read start TSC
		init_time.tv_sec = 0;
		init_time.tv_nsec = 0;

		if (use_apic) {
			rdmsr(APIC_BASE_MSR, msr_original_low, msr_original_high);
			wrmsr(APIC_BASE_MSR, msr_original_low|(1<<11), 0);

			clear_local_APIC();

			ll_calibrate_apic();

			setup_local_APIC();
       
			setup_APIC_timer();
		}
	}
}

void ll_restore_adv()
{
	SYS_FLAGS f;

	/* Disable APIC */
	if (use_apic) {

		f = ll_fsave();

		disable_APIC_timer();

		wrmsr(APIC_BASE_MSR, msr_original_low, msr_original_high);

		ll_frestore(f);
	}
}

void ll_scale_advtimer(unsigned int old_f, unsigned int new_f)
{
	unsigned long long dtsc;
	unsigned long temp;
	struct timespec temp_time;
	SYS_FLAGS f;

	if (use_tsc) {
		f = ll_fsave();

		__asm__("cpuid"::"a" (0), "b" (0), "c" (0), "d" (0));
		ll_read_timespec(&temp_time);	// Set new start TimeSpec
		TIMESPEC_ASSIGN(&init_time,&temp_time);
		rdtscll(init_tsc);		// Set new start TSC
		__asm__("cpuid"::"a" (0), "b" (0), "c" (0), "d" (0));
		
		mul32div32to32(clk_per_msec,new_f,old_f,temp);
		clk_per_msec = temp;
		dtsc = (unsigned long long)(clk_per_msec) * 500;
		clk_opt_0 = (unsigned int)(dtsc);
		clk_opt_1 = (unsigned int)((unsigned long long)(dtsc << 1));
		clk_opt_2 = (unsigned int)((unsigned long long)(dtsc << 33) / 1000000000L);
		clk_opt_3 = (unsigned int)((unsigned long long)(dtsc << 32) / 1000000000L);
		clk_opt_4 = (unsigned int)((unsigned long long)(dtsc << 31) / 1000000000L);
		clk_opt_5 = (unsigned int)((unsigned long long)(dtsc << 30) / 1000000000L);
		
		ll_frestore(f);
	}
}
