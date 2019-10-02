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
 *  This file was based upon code in Powertweak Linux (http://powertweak.sf.net)
 *  (C) 2000-2003  Dave Jones, Arjan van de Ven, Janne P�k�� Dominik Brodowski.
 *
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  BIG FAT DISCLAIMER: Work in progress code. Possibly *dangerous*
 */

#include <linuxcomp.h>

#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/cpufreq.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/slab.h>
#include <linux/cpu.h>

//extern void kern_scale_timer(unsigned int old_f, unsigned int new_f);
extern void ll_scale_advtimer(unsigned int old_f, unsigned int new_f);
extern ssize_t show_available_freqs (struct cpufreq_policy *policy, char *buf);
extern int get_available_freqs (struct cpufreq_policy *policy, int *buf);
/**
 * The "cpufreq driver" - the arch- or hardware-dependend low
 * level driver of CPUFreq support, and its spinlock. This lock
 * also protects the cpufreq_cpu_data array.
 */
static struct cpufreq_policy	cpufreq_cpu_policy;
static struct cpufreq_policy	*cpufreq_cpu_data = &cpufreq_cpu_policy;
static struct cpufreq_driver   	*cpufreq_driver;
static spinlock_t		cpufreq_driver_lock = SPIN_LOCK_UNLOCKED;

/*********************************************************************
 *                                USER                               *
 *********************************************************************/

inline int cpufreq_target(unsigned int target_freq, unsigned int relation)
{
	return cpufreq_driver_target(cpufreq_cpu_data, target_freq, relation);
}

inline int cpufreq_get_cur_freq(void)
{
	return cpufreq_cpu_data->cur;
}

inline int cpufreq_get_min_freq(void)
{
	return cpufreq_cpu_data->min;
}

inline int cpufreq_get_max_freq(void)
{
	return cpufreq_cpu_data->max;
}

inline int cpufreq_get_latency(void)
{
	return cpufreq_cpu_data->cpuinfo.transition_latency;
}

inline int cpufreq_get_available_freqs(int *buf)
{
	return get_available_freqs(cpufreq_cpu_data, buf);
}

inline int cpufreq_show_available_freqs(char *buf)
{
	return show_available_freqs(cpufreq_cpu_data, buf);
}

/*********************************************************************
 *                              GOVERNOR                             *
 *********************************************************************/

int cpufreq_driver_target(struct cpufreq_policy *policy,
			  unsigned int target_freq,
			  unsigned int relation)
{
	unsigned int ret;

	if (!policy)
		return -EINVAL;

	ret = cpufreq_driver->target(policy, target_freq, relation);

	return ret;
}

/*********************************************************************
 *                              NOTIFIER                             *
 *********************************************************************/
void cpufreq_notify_transition(struct cpufreq_freqs *freqs, unsigned int state)
{
	switch (state) {
	case CPUFREQ_PRECHANGE:
		//adjust_jiffies(CPUFREQ_PRECHANGE, freqs);
		break;
	case CPUFREQ_POSTCHANGE:
		//adjust_jiffies(CPUFREQ_POSTCHANGE, freqs);
		//kern_scale_timer(freqs->old, freqs->new);
		ll_scale_advtimer(freqs->old, freqs->new);
		cpufreq_cpu_data->cur = freqs->new;
		break;
	}
}

/*********************************************************************
 *               REGISTER / UNREGISTER CPUFREQ DRIVER                *
 *********************************************************************/

/**
 * cpufreq_register_driver - register a CPU Frequency driver
 * @driver_data: A struct cpufreq_driver containing the values#
 * submitted by the CPU Frequency driver.
 *
 *   Registers a CPU Frequency driver to this core code. This code 
 * returns zero on success, -EBUSY when another driver got here first
 * (and isn't unregistered in the meantime). 
 *
 */
int cpufreq_register_driver(struct cpufreq_driver *driver_data)
{
	unsigned long flags;

	if (!driver_data || !driver_data->verify || !driver_data->init ||
	    ((!driver_data->setpolicy) && (!driver_data->target)))
		return -EINVAL;

	spin_lock_irqsave(&cpufreq_driver_lock, flags);
	if (cpufreq_driver) {
		spin_unlock_irqrestore(&cpufreq_driver_lock, flags);
		return -EBUSY;
	}
	cpufreq_driver = driver_data;
	spin_unlock_irqrestore(&cpufreq_driver_lock, flags);

	/* Init & verify - TODO */
	cpufreq_driver->init(cpufreq_cpu_data);
	cpufreq_driver->verify(cpufreq_cpu_data);

	return 0; //sysdev_driver_register(&cpu_sysdev_class,&cpufreq_sysdev_driver);
}

/**
 * cpufreq_unregister_driver - unregister the current CPUFreq driver
 *
 *    Unregister the current CPUFreq driver. Only call this if you have 
 * the right to do so, i.e. if you have succeeded in initialising before!
 * Returns zero if successful, and -EINVAL if the cpufreq_driver is
 * currently not initialised.
 */
int cpufreq_unregister_driver(struct cpufreq_driver *driver)
{
	unsigned long flags;

	if (!cpufreq_driver || (driver != cpufreq_driver))
		return -EINVAL;

	/* Exit */
	cpufreq_driver->exit(cpufreq_cpu_data);

	//sysdev_driver_unregister(&cpu_sysdev_class, &cpufreq_sysdev_driver);

	spin_lock_irqsave(&cpufreq_driver_lock, flags);
	cpufreq_driver = NULL;
	spin_unlock_irqrestore(&cpufreq_driver_lock, flags);

	return 0;
}
