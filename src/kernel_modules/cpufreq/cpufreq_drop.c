
/*
 *  linux/drivers/cpufreq/cpufreq_userspace.c
 *
 *  Copyright (C)  2001 Russell King
 *            (C)  2002 - 2004 Dominik Brodowski <linux@brodo.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/smp.h>
#include <linux/init.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/cpufreq.h>
#include <linux/cpu.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/sysfs.h>
#include <linux/mutex.h>


#define MAX_NUM_CPU (32)

/************************************ struct  Latency C-state, Frequency P-state ********************************************/

extern bool drop_module_exist;
extern int (*get_latency) (int); // used in cpuidle/governors/menu.c 

struct latency
{
  atomic_t value[MAX_NUM_CPU];
};

static struct latency latency_req;

// A few values needed by the userspace governor
static DEFINE_PER_CPU(unsigned int, cpu_max_freq);
static DEFINE_PER_CPU(unsigned int, cpu_min_freq);
static DEFINE_PER_CPU(unsigned int, cpu_cur_freq); // current CPU freq
static DEFINE_PER_CPU(unsigned int, cpu_set_freq); // CPU freq desired by
static DEFINE_PER_CPU(unsigned int, cpu_is_managed);

static DEFINE_MUTEX(userspace_mutex);
static int cpus_using_userspace_governor;

// keep track of frequency transitions
static int userspace_cpufreq_notifier(struct notifier_block* nb, unsigned long val, void* data)
{
	struct cpufreq_freqs* freq = data;

	if (!per_cpu(cpu_is_managed, freq->cpu))
	{
		return 0;
	}

	if (val == CPUFREQ_POSTCHANGE)
	{
		pr_debug("saving cpu_cur_freq of cpu %u to be %u kHz\n", freq->cpu, freq->new);
		per_cpu(cpu_cur_freq, freq->cpu) = freq->new;
	}

	return 0;
}

static struct notifier_block userspace_cpufreq_notifier_block = 
{
	.notifier_call  = userspace_cpufreq_notifier
};

/**
 * cpufreq_set - set the CPU frequency
 * @policy: pointer to policy struct where freq is being set
 * @freq: target frequency in kHz
 *
 * Sets the CPU frequency to freq.
 */
static int cpufreq_set(struct cpufreq_policy* policy, unsigned int freq)
{
	int ret = -EINVAL;

	pr_debug("cpufreq_set for cpu %u, freq %u kHz\n", policy->cpu, freq);

	mutex_lock(&userspace_mutex);

	if (!per_cpu(cpu_is_managed, policy->cpu))
	{
		goto err;
	}

	per_cpu(cpu_set_freq, policy->cpu) = freq;

	if (freq < per_cpu(cpu_min_freq, policy->cpu))
	{
		freq = per_cpu(cpu_min_freq, policy->cpu);
	}

	if (freq > per_cpu(cpu_max_freq, policy->cpu))
	{
		freq = per_cpu(cpu_max_freq, policy->cpu);
	}

	/*
	 * We're safe from concurrent calls to ->target() here
	 * as we hold the userspace_mutex lock. If we were calling
	 * cpufreq_driver_target, a deadlock situation might occur:
	 * A: cpufreq_set (lock userspace_mutex) ->
	 *      cpufreq_driver_target(lock policy->lock)
	 * B: cpufreq_set_policy(lock policy->lock) ->
	 *      __cpufreq_governor ->
	 *         cpufreq_governor_userspace (lock userspace_mutex)
	 */
	ret = __cpufreq_driver_target(policy, freq, CPUFREQ_RELATION_L);
 err:
	mutex_unlock(&userspace_mutex);

	return ret;
}

static ssize_t show_speed(struct cpufreq_policy* policy, char* buf)
{
	return sprintf(buf, "%u\n", per_cpu(cpu_cur_freq, policy->cpu));
}

// ************************************************* latency patch begin *************************************

/********************************************************************* 
 *                                                                   *
 * function called from cpuidle/governors/menu.c to get latency_req  *
 *                                                                   *
 *********************************************************************/

static int get_latency_real(int cpu_id)
{ 
	return atomic_read(&latency_req.value[cpu_id]);
}

static int cpulat_set(struct cpufreq_policy* policy, unsigned int lat)
{
	atomic_set(&latency_req.value[policy->cpu], lat);	

	return 0;
}


static ssize_t show_lat(struct cpufreq_policy* policy, char *buf)
{
	return sprintf(buf, "%u\n", get_latency_real(policy->cpu));
}

// ************************************************* latency patch end *************************************

static int cpufreq_governor_userspace(struct cpufreq_policy* policy, unsigned int event)
{
	unsigned int cpu = policy->cpu;
	int rc = 0;

	switch (event)
	{
	case CPUFREQ_GOV_START:
		if (!cpu_online(cpu))
		{
			return -EINVAL;
		}

		BUG_ON(!policy->cur);

		mutex_lock(&userspace_mutex);

		if (cpus_using_userspace_governor == 0)
		{
			cpufreq_register_notifier(&userspace_cpufreq_notifier_block, CPUFREQ_TRANSITION_NOTIFIER);
		}

		cpus_using_userspace_governor++;

		per_cpu(cpu_is_managed, cpu) = 1;
		per_cpu(cpu_min_freq, cpu) = policy->min;
		per_cpu(cpu_max_freq, cpu) = policy->max;
		per_cpu(cpu_cur_freq, cpu) = policy->cur;
		per_cpu(cpu_set_freq, cpu) = policy->cur;
		pr_debug("managing cpu %u started (%u - %u kHz, currently %u kHz)\n", cpu, per_cpu(cpu_min_freq, cpu), per_cpu(cpu_max_freq, cpu), per_cpu(cpu_cur_freq, cpu));

		mutex_unlock(&userspace_mutex);

		break;
	case CPUFREQ_GOV_STOP:
		mutex_lock(&userspace_mutex);
		--cpus_using_userspace_governor;

		if (cpus_using_userspace_governor == 0)
		{
			cpufreq_unregister_notifier(&userspace_cpufreq_notifier_block, CPUFREQ_TRANSITION_NOTIFIER);
		}

		per_cpu(cpu_is_managed, cpu) = 0;
		per_cpu(cpu_min_freq, cpu) = 0;
		per_cpu(cpu_max_freq, cpu) = 0;
		per_cpu(cpu_set_freq, cpu) = 0;

		pr_debug("managing cpu %u stopped\n", cpu);

		mutex_unlock(&userspace_mutex);

		break;
	case CPUFREQ_GOV_LIMITS:
		mutex_lock(&userspace_mutex);

		pr_debug("limit event for cpu %u: %u - %u kHz, currently %u kHz, last set to %u kHz\n", cpu, policy->min, policy->max, per_cpu(cpu_cur_freq, cpu), per_cpu(cpu_set_freq, cpu));

		if (policy->max < per_cpu(cpu_set_freq, cpu))
		{
			__cpufreq_driver_target(policy, policy->max, CPUFREQ_RELATION_H);
		}
		else if (policy->min > per_cpu(cpu_set_freq, cpu))
		{
			__cpufreq_driver_target(policy, policy->min, CPUFREQ_RELATION_L);
		}
		else
		{
			__cpufreq_driver_target(policy, per_cpu(cpu_set_freq, cpu), CPUFREQ_RELATION_L);
		}

		per_cpu(cpu_min_freq, cpu) = policy->min;
		per_cpu(cpu_max_freq, cpu) = policy->max;
		per_cpu(cpu_cur_freq, cpu) = policy->cur;

		mutex_unlock(&userspace_mutex);

		break;
	}

	return rc;
}

#ifndef CONFIG_CPU_FREQ_DEFAULT_GOV_USERSPACE
static
#endif

struct cpufreq_governor cpufreq_gov_userspace = 
{
	.name		= "drop",
	.governor	= cpufreq_governor_userspace,
	.store_setspeed	= cpufreq_set,
	.show_setspeed	= show_speed,
	.store_setlatency = cpulat_set, // latency patch
	.show_setlatency = show_lat, // latency patch
	.owner		= THIS_MODULE,
};

static int __init cpufreq_gov_userspace_init(void)
{
	// ********************** latency patch begin ******************
	int i;

	get_latency = &get_latency_real;
	drop_module_exist = true;

	for (i=0; i<MAX_NUM_CPU; ++i)
	{
		atomic_set(&latency_req.value[i], 1);
	}

	// ********************** latency patch end ******************
  
	return cpufreq_register_governor(&cpufreq_gov_userspace);
}

static void __exit cpufreq_gov_userspace_exit(void)
{
	drop_module_exist=false;
	get_latency=NULL;

	cpufreq_unregister_governor(&cpufreq_gov_userspace);
}


MODULE_AUTHOR("Luca D'Agostino");
MODULE_DESCRIPTION("CPUfreq policy governor 'new_userspace'");
MODULE_LICENSE("GPL");

#ifdef CONFIG_CPU_FREQ_DEFAULT_GOV_USERSPACE
fs_initcall(cpufreq_gov_userspace_init);
#else
module_init(cpufreq_gov_userspace_init);
#endif
module_exit(cpufreq_gov_userspace_exit);
