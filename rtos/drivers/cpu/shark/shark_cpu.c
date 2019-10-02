/*
 * Project: S.Ha.R.K.
 *
 * Coordinators:
 *   Giorgio Buttazzo    <giorgio@sssup.it>
 *   Paolo Gai           <pj@gandalf.sssup.it>
 *
 * Authors     :
 *   Mauro Marinoni      <mauro.marinoni@unipv.it>
 *
 *
 * ReTiS Lab (Scuola Superiore S.Anna - Pisa - Italy)
 *
 * http://www.sssup.it
 * http://retis.sssup.it
 * http://shark.sssup.it
 */

/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <kernel/func.h>
#include "../include/drivers/shark_cpu26.h"
#include <tracer.h>

//#define __CPU26_DEBUG__

/* CPU Initialization */
extern void early_cpu_init(void);
extern void identify_cpu_0(void);
extern void print_cpu_info_0(void);

/* AMD K6 PowerNow */
extern int  powernow_k6_init(void);
extern void powernow_k6_exit(void);
/* AMD K7 PowerNow */
extern int  powernow_init(void);
extern void powernow_exit(void);
/* AMD K8 PowerNow */
extern int  powernowk8_init(void);
extern void powernowk8_exit(void);
/* Cyrix MediaGX - NatSemi Geode */
extern int  cpufreq_gx_init(void);
extern void cpufreq_gx_exit(void);
/* Pentium4 clock modulation/speed scaling */
extern int  cpufreq_p4_init(void);
extern void cpufreq_p4_exit(void);
/* PentiumM/Centrino SpeedStep */
extern int  centrino_init(void);
extern void centrino_exit(void);
/* Pentium ICH SpeedStep */
extern int  speedstep_ich_init(void);
extern void speedstep_ich_exit(void);
/* Pentium SMI SpeedStep */
/*extern int  speedstep_smi_init(void);
extern void speedstep_smi_exit(void);*/

/* DVS function */
extern int cpufreq_target(unsigned int target_freq, unsigned int relation);
extern int cpufreq_get_cur_freq(void);
extern int cpufreq_get_min_freq(void);
extern int cpufreq_get_max_freq(void);
extern int cpufreq_get_latency(void);
extern int cpufreq_get_available_freqs(int *buf);
extern int cpufreq_show_available_freqs(char *buf);

static int	cpu_installed = FALSE;
static int	dvs_installed = DVS_NONE;
int		cpu26_freqs[DVS_MAX_NUM_FREQS];

/* DVS user function */

extern unsigned int clk_per_msec;

int  CPU26_set_frequency(unsigned int target_freq, unsigned int relation)
{

	int res;

	if (dvs_installed != DVS_NONE && dvs_installed != DVS_MEDIAGX_GEODE) {
		res = cpufreq_target(target_freq, relation);
		TRACER_LOGEVENT(FTrace_EVT_cycles_per_msec,0,clk_per_msec);
		return res;
	} else {
		return -1;
	}

}

extern void gx_force_values(unsigned char on_duration, unsigned char off_duration);

int CPU26_set_suspend_modulation(unsigned char on_duration, unsigned char off_duration) {

	if (dvs_installed == DVS_MEDIAGX_GEODE) {
		gx_force_values(on_duration, off_duration);
		return 0;
	}

	return -1;

}

inline int  CPU26_get_cur_frequency()
{
	return cpufreq_get_cur_freq();
}

inline int  CPU26_get_min_frequency()
{
	return cpufreq_get_min_freq();
}

inline int  CPU26_get_max_frequency()
{
	return cpufreq_get_max_freq();
}

inline int CPU26_get_frequencies(int *buf)
{
	return cpufreq_get_available_freqs (buf);
}

inline int CPU26_show_frequencies(char *buf)
{
	return cpufreq_show_available_freqs (buf);
}

inline int  CPU26_get_latency()
{
	return cpufreq_get_latency();
}

inline int  CPU26_DVS_installed(void)
{
	return dvs_installed;
}	

int  CPU26_DVS_init(void)
{
	int ret = 0;

	if (cpu_installed == FALSE)
		return -1;

	if (dvs_installed != DVS_NONE)
		return 0;

	ret = powernow_k6_init();
#ifdef __CPU26_DEBUG__
	printk(KERN_DEBUG "Check PowerNow! K6 - Returned: %d\n", ret);
#endif
	if (!ret) {
		dvs_installed = DVS_POWERNOW_K6;
		return dvs_installed;
	}

	ret = powernow_init();
#ifdef __CPU26_DEBUG__
	printk(KERN_DEBUG "Check PowerNow! K7 - Returned: %d\n", ret);
#endif
	if (!ret) {
		dvs_installed = DVS_POWERNOW_K7;
		return dvs_installed;
	}

	ret = powernowk8_init();
#ifdef __CPU26_DEBUG__
	printk(KERN_DEBUG "Check PowerNow! K8 - Returned: %d\n", ret);
#endif
	if (!ret) {
		dvs_installed = DVS_POWERNOW_K8;
		return dvs_installed;
	}

	ret = cpufreq_gx_init();
#ifdef __CPU26_DEBUG__
	printk(KERN_DEBUG "Check MediaGX/Geode (Returned: %d)\n", ret);
#endif
	if (!ret) {
		dvs_installed = DVS_MEDIAGX_GEODE;
		return dvs_installed;
	}

	ret = cpufreq_p4_init();
#ifdef __CPU26_DEBUG__
	printk(KERN_DEBUG "Check Pentium4 ClockModulation (Returned: %d)\n", ret);
#endif
	if (!ret) {
		dvs_installed = DVS_P4_CLOCK_MOD;
		return dvs_installed;
	}

	ret = centrino_init();
#ifdef __CPU26_DEBUG__
	printk(KERN_DEBUG "Check SpeedStep Centrino (Returned: %d)\n", ret);
#endif
	if (!ret) {
		dvs_installed = DVS_SS_CENTRINO;
		return dvs_installed;
	}

	ret = speedstep_ich_init();
#ifdef __CPU26_DEBUG__
	printk(KERN_DEBUG "Check SpeedStep ICH (Returned: %d)\n", ret);
#endif
	if (!ret) {
		dvs_installed = DVS_SS_ICH;
		return dvs_installed;
	}

	/*ret = speedstep_smi_init();
#ifdef __CPU26_DEBUG__
	printk(KERN_DEBUG "Check SpeedStep SMI (Returned: %d)\n", ret);
#endif
	if (!ret) {
		dvs_installed = DVS_SS_SMI;
		return dvs_installed;
	}*/

	return -1;
}

int  CPU26_DVS_close(void)
{
	switch(dvs_installed) {
		case DVS_NONE:
			return -1;
		case DVS_POWERNOW_K6:
			powernow_k6_exit();
			return 0;
		case DVS_POWERNOW_K7:
			powernow_exit();
			return 0;
		case DVS_POWERNOW_K8:
			powernowk8_exit();
			return 0;
		case DVS_MEDIAGX_GEODE:
			cpufreq_gx_exit();
			return 0;
		case DVS_P4_CLOCK_MOD:
			cpufreq_p4_exit();
			return 0;
		case DVS_SS_CENTRINO:
			centrino_exit();
			return 0;
		case DVS_SS_ICH:
			speedstep_ich_exit();
			return 0;
		/*case DVS_SS_SMI:
			speedstep_smi_exit();
			return 0;*/
	}

	dvs_installed = DVS_NONE;
	return 0;
}

/* Init the Linux CPU Layer */
inline int  CPU26_installed(void)
{
	return cpu_installed;
}

inline void CPU26_showinfo(void)
{
	print_cpu_info_0();
}

int  CPU26_init(void) 
{
	int ret = 0;
	
	if (cpu_installed == TRUE) return 0;

	early_cpu_init();
	
	identify_cpu_0();

	printk(KERN_INFO);
	print_cpu_info_0();

	cpu_installed = TRUE;

	return ret;
}

inline int  CPU26_close(void) 
{
	if (cpu_installed == TRUE) {
		return 0;
	} else
		return -1;
}
