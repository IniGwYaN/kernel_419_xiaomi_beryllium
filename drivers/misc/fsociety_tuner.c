// SPDX-License-Identifier: GPL-2.0
/*
 * fsociety tuner: power profiles for the POCO F1 (SDM845: cpu0-3 silver, cpu4-7 gold).
 *
 * profile=0 eco      GPU ceiling 596 MHz, CPU ceilings while the screen is on (eco_*_khz)
 * profile=1 balanced GPU ceiling = DT qcom,fsociety-max-freq (stock 710 MHz), lifted to the top level only while a
 *                    game keeps the GPU pinned at that ceiling (kgsl_pwrctrl.c gpu_autoboost); default
 * profile=2 game     GPU ceiling = top level
 * Screen off (any profile): CPU ceilings so_*_khz for background work (screen_off_cap=N disables).
 * These are ceilings only: thermal-engine and the kernel thermal zones stay in charge on top of them.
 * Knobs in /sys/module/fsociety/parameters/, boot default via fsociety.profile=N on the kernel cmdline. UMRSRN.
 */
#define pr_fmt(fmt) "fsociety: " fmt

#include <linux/cpu.h>
#include <linux/cpufreq.h>
#include <linux/fsociety.h>
#include <linux/init.h>
#include <linux/moduleparam.h>
#include <linux/notifier.h>
#include <linux/workqueue.h>
#include <drm/drm_notifier.h>

#undef MODULE_PARAM_PREFIX
#define MODULE_PARAM_PREFIX "fsociety."

static int profile = FSOC_BALANCED;
static bool screen_off_cap = true;
static unsigned int so_little_khz = 1228800, so_big_khz = 1459200;
static unsigned int eco_little_khz = 1516800, eco_big_khz = 1996800;
static bool mglru_lock = true;
static bool screen_on = true;
static bool ready;

static const char * const names[] = { "eco", "balanced", "game" };

int fsociety_profile(void)
{
	return READ_ONCE(profile);
}

bool fsociety_screen_on(void)
{
	return READ_ONCE(screen_on);
}

bool fsociety_mglru_locked(void)
{
	return READ_ONCE(mglru_lock);
}

static void update_fn(struct work_struct *work)
{
	unsigned int cpu;

	get_online_cpus();
	for_each_online_cpu(cpu) {
		struct cpufreq_policy *pol = cpufreq_cpu_get(cpu);
		bool first;

		if (!pol)
			continue;
		first = cpu == cpumask_first(pol->cpus);
		cpufreq_cpu_put(pol);
		if (first)
			cpufreq_update_policy(cpu);
	}
	put_online_cpus();
}
static DECLARE_WORK(update_work, update_fn);

static void kick(bool force)
{
	if (!READ_ONCE(ready))
		return;
	queue_work(system_wq, &update_work);
	fsociety_kgsl_update(force);
}

static int cpufreq_cb(struct notifier_block *nb, unsigned long val, void *data)
{
	struct cpufreq_policy *pol = data;
	bool big = pol->cpu >= 4;
	unsigned int cap = 0;

	if (val != CPUFREQ_ADJUST)
		return NOTIFY_DONE;

	if (!READ_ONCE(screen_on) && READ_ONCE(screen_off_cap))
		cap = big ? so_big_khz : so_little_khz;
	else if (READ_ONCE(profile) == FSOC_ECO)
		cap = big ? eco_big_khz : eco_little_khz;

	/* the cpufreq core verifies against the frequency table again after ADJUST */
	if (cap && cap < pol->max)
		cpufreq_verify_within_limits(pol, 0, cap);

	return NOTIFY_OK;
}

static struct notifier_block cpufreq_nb = {
	.notifier_call = cpufreq_cb,
};

static int drm_cb(struct notifier_block *nb, unsigned long val, void *data)
{
	struct drm_notify_data *evdata = data;
	bool on;

	if (val != DRM_EVENT_BLANK || !evdata || !evdata->data)
		return NOTIFY_DONE;

	switch (*(int *)evdata->data) {
	case DRM_BLANK_UNBLANK:
		on = true;
		break;
	case DRM_BLANK_LP1:
	case DRM_BLANK_LP2:
	case DRM_BLANK_POWERDOWN:
		on = false;
		break;
	default:
		return NOTIFY_DONE;
	}

	if (on != READ_ONCE(screen_on)) {
		WRITE_ONCE(screen_on, on);
		kick(false);
	}

	return NOTIFY_OK;
}

static struct notifier_block drm_nb = {
	.notifier_call = drm_cb,
};

static int set_profile(const char *val, const struct kernel_param *kp)
{
	int p, ret = kstrtoint(val, 0, &p);

	if (ret)
		return ret;
	if (p < FSOC_ECO || p > FSOC_GAME)
		return -EINVAL;

	WRITE_ONCE(profile, p);
	if (READ_ONCE(ready))
		pr_info("profile %s\n", names[p]);
	kick(true);
	return 0;
}

static const struct kernel_param_ops profile_ops = {
	.set = set_profile,
	.get = param_get_int,
};
module_param_cb(profile, &profile_ops, &profile, 0644);

static int set_uint(const char *val, const struct kernel_param *kp)
{
	int ret = param_set_uint(val, kp);

	if (!ret)
		kick(false);
	return ret;
}

static const struct kernel_param_ops uint_ops = {
	.set = set_uint,
	.get = param_get_uint,
};
module_param_cb(so_little_khz, &uint_ops, &so_little_khz, 0644);
module_param_cb(so_big_khz, &uint_ops, &so_big_khz, 0644);
module_param_cb(eco_little_khz, &uint_ops, &eco_little_khz, 0644);
module_param_cb(eco_big_khz, &uint_ops, &eco_big_khz, 0644);

static int set_bool(const char *val, const struct kernel_param *kp)
{
	int ret = param_set_bool(val, kp);

	if (!ret)
		kick(false);
	return ret;
}

static const struct kernel_param_ops bool_ops = {
	.set = set_bool,
	.get = param_get_bool,
};
module_param_cb(screen_off_cap, &bool_ops, &screen_off_cap, 0644);
module_param(mglru_lock, bool, 0644);
module_param(screen_on, bool, 0444);

static int __init fsociety_tuner_init(void)
{
	int ret = cpufreq_register_notifier(&cpufreq_nb, CPUFREQ_POLICY_NOTIFIER);

	if (ret)
		pr_err("cpufreq notifier: %d\n", ret);
	ret = drm_register_client(&drm_nb);
	if (ret)
		pr_err("drm notifier: %d\n", ret);

	WRITE_ONCE(ready, true);
	kick(false);
	pr_info("profile %s, screen-off cap %u/%u kHz, mglru_lock %d\n", names[profile],
		so_little_khz, so_big_khz, mglru_lock);
	return 0;
}
late_initcall(fsociety_tuner_init);
