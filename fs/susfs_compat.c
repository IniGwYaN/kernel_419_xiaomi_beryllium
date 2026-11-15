// SUSFS compat: selinux rule hooks (SUS_TYPE_HLIST / SUS_AVD_ALLOWED_HLIST)
// Diambil dari integrasi SUSFS resmi 4.19 (cyberc3dr/nGKI_Kernel_Build).
#include <linux/version.h>
#include <linux/hash.h>
#include <linux/hashtable.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/cred.h>
#include <linux/string.h>
#include <linux/susfs.h>

#define SUSFS_LOGI(fmt, ...) pr_info("susfs:[%u][%d][%s] " fmt, current_uid().val, current->pid, __func__, ##__VA_ARGS__)
#define SUSFS_LOGE(fmt, ...) pr_err("susfs:[%u][%d][%s] " fmt, current_uid().val, current->pid, __func__, ##__VA_ARGS__)

static DEFINE_MUTEX(susfs_mutex_lock_sus_type);
static DEFINE_HASHTABLE(SUS_TYPE_HLIST, 10);

struct st_susfs_sus_type_hlist {
	struct hlist_node node;
};

int susfs_add_sus_type(const char *secctx, u32 type_value) {
	struct st_susfs_sus_type_hlist *new_entry, *tmp_entry;
	struct hlist_node *tmp_hlist_node;
	int err = 0;

	mutex_lock(&susfs_mutex_lock_sus_type);
	hash_for_each_possible_safe(SUS_TYPE_HLIST, tmp_entry, tmp_hlist_node, node, type_value) {
		SUSFS_LOGE("secctx '%s', type_value '%u', was added before already\n",
			secctx, type_value);
		err = -EINVAL;
		goto unlock;
	}

	new_entry = kzalloc(sizeof(struct st_susfs_sus_type_hlist), GFP_KERNEL);
	if (!new_entry) {
		err = -ENOMEM;
		goto unlock;
	}

	hash_add_rcu(SUS_TYPE_HLIST, &new_entry->node, type_value);
	SUSFS_LOGI("secctx '%s', type '%u', has been added to SUS_TYPE_HLIST\n",
			secctx, type_value);
unlock:
	mutex_unlock(&susfs_mutex_lock_sus_type);
	return err;
}

static DEFINE_MUTEX(susfs_mutex_lock_sus_avd_allowed_hlist);
static DEFINE_HASHTABLE(SUS_AVD_ALLOWED_HLIST, 18);

struct susfs_sus_av_rule {
	struct hlist_node node;
	u32 s_type;
	u32 t_type;
	u32 c_type;
	u32 orig_avd_allowed;
};

static inline u32 susfs_av_rule_hash(u32 ssid, u32 tsid, u16 tclass)
{
	u32 h;

	h = hash_32(ssid, 32);
	h ^= rol32(hash_32(tsid, 32), 13);
	h ^= rol32(hash_32((u32)tclass, 32), 19);
	h ^= rol32(h, 29);

	return h;
}

int susfs_add_sus_avd_allowed_by_type(u32 s_type, u32 t_type, u32 c_type, u32 orig_avd_allowed)
{
	struct susfs_sus_av_rule *entry, *tmp_entry;
	struct hlist_node *tmp_hlist_node;
	int err = 0;
	u32 key;

	mutex_lock(&susfs_mutex_lock_sus_avd_allowed_hlist);

	entry = kzalloc(sizeof(*entry), GFP_ATOMIC);
	if (!entry) {
		err = -ENOMEM;
		goto unlock;
	}

	key = susfs_av_rule_hash(s_type, t_type, c_type);
	hash_for_each_possible_safe(SUS_AVD_ALLOWED_HLIST, tmp_entry, tmp_hlist_node, node, key) {
		if (tmp_entry->s_type == s_type &&
		    tmp_entry->t_type == t_type &&
		    tmp_entry->c_type == c_type)
		{
			kfree(entry);
			goto unlock;
		}
	}

	entry->s_type = s_type;
	entry->t_type = t_type;
	entry->c_type = c_type;
	entry->orig_avd_allowed = orig_avd_allowed;

	hash_add_rcu(SUS_AVD_ALLOWED_HLIST, &entry->node, key);
	SUSFS_LOGI("av_rule '%u %u %u %u' has been added to SUS_AVD_ALLOWED_HLIST\n",
			s_type, t_type, c_type, orig_avd_allowed);
unlock:
	mutex_unlock(&susfs_mutex_lock_sus_avd_allowed_hlist);
	return err;
}
