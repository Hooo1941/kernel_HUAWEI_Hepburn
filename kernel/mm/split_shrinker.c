/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 * Description: split shrinker_list into hot list and cold list
 * Author: Gong Chen <gongchen4@huawei.com>
 * Create: 2021-05-28
 */
#include <linux/split_shrinker.h>

#include <linux/memcontrol.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/shrinker.h>
#include <linux/swap.h>

#ifdef CONFIG_KSHRINK_SLABD
#include <platform/linux/kshrink_slabd.h>
#endif

#define SHRINKER_MAX_NUM 50
#define ACTIVE_HOT_THRESHOLD 30

#define SPLIT_SHRINKER_MASK (SHRINKER_HOT | SHRINKER_DELAY | SHRINKER_SKIP | \
	SHRINKER_COLD | SHRINKER_COLD_ZERO_SEEK | SHRINKER_COLD_DEF_SEEK)

struct shrinker_active_record {
	unsigned long (*scan_objects)(struct shrinker *shrinker,
		struct shrink_control *sc);
	unsigned int active;
};

static DECLARE_RWSEM(record_rwsem);
static struct shrinker_active_record record[SHRINKER_MAX_NUM];

enum shrinker_type {
	ST_COLD = 0,
	ST_COLD_ZERO_SEEK,
	ST_COLD_DEF_SEEK,
	ST_HOT,
	ST_DELAY,
	ST_SKIP,
};
#define ST_MAX (ST_SKIP + 1)

const char *shrinker_type_str[ST_MAX] = {
	"cold",
	"cold_zero_seek",
	"cold_default_seek",
	"hot",
	"delay",
	"skip",
};

static struct list_head g_shrinker_lists[ST_MAX] = {
	LIST_HEAD_INIT(g_shrinker_lists[ST_COLD]),
	LIST_HEAD_INIT(g_shrinker_lists[ST_COLD_ZERO_SEEK]),
	LIST_HEAD_INIT(g_shrinker_lists[ST_COLD_DEF_SEEK]),
	LIST_HEAD_INIT(g_shrinker_lists[ST_HOT]),
	LIST_HEAD_INIT(g_shrinker_lists[ST_DELAY]),
	LIST_HEAD_INIT(g_shrinker_lists[ST_SKIP]),
};

static struct rw_semaphore g_shrinker_sems[ST_MAX] = {
	__RWSEM_INITIALIZER(g_shrinker_sems[ST_COLD]),
	__RWSEM_INITIALIZER(g_shrinker_sems[ST_COLD_ZERO_SEEK]),
	__RWSEM_INITIALIZER(g_shrinker_sems[ST_COLD_DEF_SEEK]),
	__RWSEM_INITIALIZER(g_shrinker_sems[ST_HOT]),
	__RWSEM_INITIALIZER(g_shrinker_sems[ST_DELAY]),
	__RWSEM_INITIALIZER(g_shrinker_sems[ST_SKIP]),
};

static unsigned int init_active_record(const struct shrinker *shrinker)
{
	int i;
	unsigned int active;

	active = 0;
	down_write(&record_rwsem);
	for (i = 0; i < SHRINKER_MAX_NUM; i++) {
		if (!record[i].scan_objects) {
			record[i].scan_objects = shrinker->scan_objects;
			record[i].active = 0;
			active = 0;
			break;
		}
		if (record[i].scan_objects == shrinker->scan_objects) {
			active = record[i].active;
			break;
		}
	}
	up_write(&record_rwsem);

	return active;
}

static unsigned int get_active_record(const struct shrinker *shrinker)
{
	int i;
	unsigned int active;

	active = 0;
	down_read(&record_rwsem);
	for (i = 0; i < SHRINKER_MAX_NUM; i++) {
		if (record[i].scan_objects == shrinker->scan_objects) {
			active = record[i].active;
			break;
		}
		if (!record[i].scan_objects) {
			up_read(&record_rwsem);
			return init_active_record(shrinker);
		}
	}
	up_read(&record_rwsem);

	return active;
}

static void update_active_record(struct shrinker *shrinker)
{
	int i;

	down_write(&record_rwsem);
	for (i = 0; i < SHRINKER_MAX_NUM; i++) {
		if (record[i].scan_objects == shrinker->scan_objects) {
			record[i].active++;
			break;
		}
	}
	up_write(&record_rwsem);
}

struct shrinker_type_map {
	unsigned int flags;
	enum shrinker_type type;
};

static enum shrinker_type flag_to_type(unsigned int flags)
{
	int i;
	static const struct shrinker_type_map maps[] = {
		{ SHRINKER_COLD,           ST_COLD },
		{ SHRINKER_COLD_ZERO_SEEK, ST_COLD_ZERO_SEEK },
		{ SHRINKER_COLD_DEF_SEEK,  ST_COLD_DEF_SEEK },
		{ SHRINKER_HOT,            ST_HOT },
		{ SHRINKER_DELAY,          ST_DELAY },
		{ SHRINKER_SKIP,           ST_SKIP },
	};

	for (i = 0; i < ARRAY_SIZE(maps); i++) {
		if (flags & maps[i].flags)
			return maps[i].type;
	}

	return ST_COLD;
}

static unsigned int get_cold_shrinker_flags(const struct shrinker *shrinker)
{
	if (shrinker->seeks == DEFAULT_SEEKS)
		return SHRINKER_COLD_DEF_SEEK;

	if (shrinker->seeks == 0)
		return SHRINKER_COLD_ZERO_SEEK;

	return SHRINKER_COLD;
}

static void init_shrinker_flags(struct shrinker *shrinker)
{
	unsigned int active;

	if (shrinker->flags & SPLIT_SHRINKER_MASK)
		return;

	active = get_active_record(shrinker);
	if (active < ACTIVE_HOT_THRESHOLD)
		shrinker->flags |= get_cold_shrinker_flags(shrinker);
	else
		shrinker->flags |= SHRINKER_HOT;
}

static enum shrinker_type get_shrinker_type(struct shrinker *shrinker, bool is_register)
{
	if (is_register) {
		if (!is_split_shrinker_enable())
			return ST_COLD;
		init_shrinker_flags(shrinker);
	}

	return flag_to_type(shrinker->flags);
}

void register_split_shrinker(struct shrinker *shrinker)
{
	enum shrinker_type type;

	if (unlikely(!shrinker))
		return;

#ifdef CONFIG_SPLIT_SHRINKER_DEBUG
	atomic_long_set(&shrinker->freed, 0);
#endif

	type = get_shrinker_type(shrinker, true);
	if (type == ST_SKIP && !is_debug_skip_shrinker_enable()) {
		INIT_LIST_HEAD(&shrinker->list);
		return;
	}

	down_write(&g_shrinker_sems[type]);
	list_add_tail(&shrinker->list, &g_shrinker_lists[type]);
#ifdef SHRINKER_REGISTERED
	shrinker->flags |= SHRINKER_REGISTERED;
#endif
	up_write(&g_shrinker_sems[type]);
}

void unregister_split_shrinker(struct shrinker *shrinker)
{
	enum shrinker_type type;

	if (unlikely(!shrinker))
		return;

	if (list_empty(&shrinker->list))
		return;

	type = get_shrinker_type(shrinker, false);
	down_write(&g_shrinker_sems[type]);
	list_del(&shrinker->list);
#ifdef SHRINKER_REGISTERED
	shrinker->flags &= ~SHRINKER_REGISTERED;
#endif
	up_write(&g_shrinker_sems[type]);

	update_active_record(shrinker);
}

static atomic_t shrinking_percent = ATOMIC_INIT(0);
static atomic_t shrinking_tasks = ATOMIC_INIT(0);
#ifdef CONFIG_SPLIT_SHRINKER_DEBUG
static atomic64_t skip_default_seek = ATOMIC64_INIT(0);
static atomic64_t shrink_default_seek = ATOMIC64_INIT(0);
static atomic64_t skip_zero_seek = ATOMIC64_INIT(0);
static atomic64_t shrink_zero_seek = ATOMIC64_INIT(0);
#endif

static bool is_shrinking_percent_overlimit(int priority, bool *update)
{
	int max_percent;

	max_percent = get_shrinker_percent_limit();
	if (max_percent == 0)
		return false;

	if (atomic_read(&shrinking_percent) > max_percent) {
#ifdef CONFIG_SPLIT_SHRINKER_DEBUG
		atomic64_inc(&skip_default_seek);
#endif
		return true;
	}

	atomic_add(1 << (DEF_PRIORITY - priority), &shrinking_percent);
	*update = true;
#ifdef CONFIG_SPLIT_SHRINKER_DEBUG
	atomic64_inc(&shrink_default_seek);
#endif
	return false;
}

static bool is_shrinking_tasks_overlimit(bool *update)
{
	int max_num;

	max_num = get_shrinker_number_limit();
	if (max_num == 0)
		return false;

	if (atomic_inc_return(&shrinking_tasks) > max_num) {
		atomic_dec(&shrinking_tasks);
#ifdef CONFIG_SPLIT_SHRINKER_DEBUG
		atomic64_inc(&skip_zero_seek);
#endif
		return true;
	}

	*update = true;
#ifdef CONFIG_SPLIT_SHRINKER_DEBUG
	atomic64_inc(&shrink_zero_seek);
#endif
	return false;
}

struct shrink_policy {
	bool skip_zero_seek;
	bool skip_default_seek;
	bool update_shrinking_tasks;
	bool update_shrinking_percent;
};

static void update_shrinker_num(int priority, const struct shrink_policy *policy)
{
	if (policy->update_shrinking_percent)
		atomic_sub(1 << (12 - priority), &shrinking_percent);

	if (policy->update_shrinking_tasks)
		atomic_dec(&shrinking_tasks);
}

static void get_shrink_slab_policy(struct shrink_policy *policy, int priority)
{
	policy->skip_zero_seek = is_shrinking_tasks_overlimit(&policy->update_shrinking_tasks);
	policy->skip_default_seek = is_shrinking_percent_overlimit(priority,
		&policy->update_shrinking_percent);
}

static inline unsigned long shrink_slab_split(gfp_t gfp_mask, int nid,
	struct mem_cgroup *memcg, int priority, enum shrinker_type type)
{
	return shrink_slab_legacy(gfp_mask, nid, memcg, priority,
		&g_shrinker_sems[type], &g_shrinker_lists[type]);
}

unsigned long shrink_slab(gfp_t gfp_mask, int nid,
	struct mem_cgroup *memcg, int priority)
{
	unsigned long freed = 0;
	struct shrink_policy policy = {
		.skip_zero_seek = false,
		.skip_default_seek = false,
		.update_shrinking_tasks = false,
		.update_shrinking_percent = false,
	};

#ifdef CONFIG_KSHRINK_SLABD
	if (should_shrink_slab_async(gfp_mask, nid, memcg, priority))
		return 0;
#endif

	get_shrink_slab_policy(&policy, priority);
	if (!policy.skip_zero_seek)
		freed += shrink_slab_split(gfp_mask, nid, memcg, priority, ST_COLD_ZERO_SEEK);

	if (!policy.skip_default_seek)
		freed += shrink_slab_split(gfp_mask, nid, memcg, priority, ST_COLD_DEF_SEEK);

	freed += shrink_slab_split(gfp_mask, nid, memcg, priority, ST_HOT);

	if (priority < get_delay_shrinker_prio())
		freed += shrink_slab_split(gfp_mask, nid, memcg, priority, ST_DELAY);

	freed += shrink_slab_split(gfp_mask, nid, memcg, priority, ST_COLD);

	update_shrinker_num(priority, &policy);

	return freed;
}

static bool is_fs_no_cache(const char *fs_name)
{
	int i;
	static const char * const no_cache_fs[] = {
		"binder",
		"bpf",
		"configfs",
		"debugfs",
		"devpts",
		"fusectl",
		"pstore",
		"selinuxfs",
		"tmpfs",
		"tracefs"
	};

	for (i = 0; i < ARRAY_SIZE(no_cache_fs); i++) {
		if (strcmp(fs_name, no_cache_fs[i]) == 0)
			return true;
	}
	return false;
}

static bool is_fs_no_io(const char *fs_name)
{
	int i;
	static const char * const no_io_fs[] = {
		"cgroup",
		"cgroup2",
		"proc",
		"sysfs",
	};

	for (i = 0; i < ARRAY_SIZE(no_io_fs); i++) {
		if (strcmp(fs_name, no_io_fs[i]) == 0)
			return true;
	}

	return false;
}

void set_fs_shrinker_type(struct shrinker *s, const char *fs_name, int flags)
{
	if (!s || !fs_name)
		return;

	s->flags |= SHRINKER_FS;

	if (!is_split_shrinker_enable())
		return;

	if (flags & SB_DELAY_SHRINK) {
		s->flags |= SHRINKER_DELAY;
		return;
	}

	if (is_fs_no_cache(fs_name)) {
		s->flags |= SHRINKER_SKIP;
		return;
	}

	if (is_fs_no_io(fs_name))
		s->seeks = 0;
}

static inline bool is_target_delay_fs(const char *fs_name)
{
	return strcmp(fs_name, "ext4") == 0 || strcmp(fs_name, "erofs") == 0;
}

#define BLOCK_DEV "/dev/block/"
static inline bool is_block_dev(const char *dev_name)
{
	return strncmp(dev_name, BLOCK_DEV, strlen(BLOCK_DEV)) == 0;
}

static bool is_target_delay_dev(const char *dev_name)
{
	int i;
	static const char * const target_dev[] = {
		"/dev/block/loop",
		"/dev/block/by-name/modem",
#ifdef CONFIG_SPLIT_SHRINKER_DEBUG
		"/dev/block/by-name/eng",
#endif
	};

	for (i = 0; i < ARRAY_SIZE(target_dev); i++) {
		if (strncmp(dev_name, target_dev[i], strlen(target_dev[i])) == 0)
			return true;
	}
	return false;
}

static bool is_target_delay_mnt_dir(const char *mnt_dir)
{
	int i;
	static const char * const target_dir[] = {
		"cache",
		"patch_hw",
		"preas",
		"preavs",
		"modem_driver",
		"modem_vendor",
	};

	for (i = 0; i < ARRAY_SIZE(target_dir); i++) {
		if (strcmp(mnt_dir, target_dir[i]) == 0)
			return true;
	}
	return false;
}

bool should_delay_fs_shrink(const char *fs_name, const char *dev_name, const char *mnt_dir)
{
	if (!fs_name || !dev_name || !mnt_dir || !is_split_shrinker_enable())
		return false;

	if (!is_target_delay_fs(fs_name) || !is_block_dev(dev_name))
		return false;

	if (is_target_delay_dev(dev_name) || is_target_delay_mnt_dir(mnt_dir))
		return true;

	return false;
}

static void print_fs_shrinker_info(struct seq_file *m, struct shrinker *shrinker)
{
	struct super_block *sb = container_of(shrinker, struct super_block, s_shrink);
	const char *fs_type = "none";

	if (sb->s_type && sb->s_type->name)
		fs_type = sb->s_type->name;

	seq_printf(m, " %s %s", fs_type, sb->s_id);
}

static void _shrinker_info_proc_show(struct seq_file *m, enum shrinker_type type)
{
	int num;
	unsigned long count;
	struct shrinker *shrinker = NULL;
	struct shrink_control sc = {
		.gfp_mask = GFP_KERNEL,
		.nid = 0,
		.memcg = root_mem_cgroup,
	};

	num = 0;
	down_read(&g_shrinker_sems[type]);
	list_for_each_entry(shrinker, &g_shrinker_lists[type], list) {
		count = shrinker->count_objects(shrinker, &sc);
		if (count == SHRINK_EMPTY)
			count = 0;
		seq_printf(m, "%s %ps %ps %lld %d 0x%x %lld %lld %lld",
			shrinker_type_str[type],
			shrinker->scan_objects,
			shrinker->count_objects,
			shrinker->batch,
			shrinker->seeks,
			shrinker->flags,
			atomic_long_read(&shrinker->nr_deferred[0]),
#ifdef CONFIG_SPLIT_SHRINKER_DEBUG
			atomic_long_read(&shrinker->freed),
#else
			0LL,
#endif
			count);
		if (shrinker->flags & SHRINKER_FS)
			print_fs_shrinker_info(m, shrinker);
		seq_puts(m, "\n");
		num++;
	}
	up_read(&g_shrinker_sems[type]);

	seq_printf(m, "total : %d %s shrinkers\n", num, shrinker_type_str[type]);
}

static int shrinker_info_proc_show(struct seq_file *m, void *v)
{
	int i;

	seq_puts(m, "type scan_func count_func batch seeks flags nr_deferred freed count\n");
	for (i = ST_COLD; i < ST_MAX; i++)
		_shrinker_info_proc_show(m, i);

	seq_printf(m, "%ld %ld %ld %ld %d %d\n",
#ifdef CONFIG_SPLIT_SHRINKER_DEBUG
		atomic64_read(&shrink_default_seek), atomic64_read(&skip_default_seek),
		atomic64_read(&shrink_zero_seek), atomic64_read(&skip_zero_seek),
#else
		0, 0, 0, 0,
#endif
		atomic_read(&shrinking_percent), atomic_read(&shrinking_tasks));

	return 0;
}

static int shrinker_info_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, shrinker_info_proc_show, NULL);
}

static const struct proc_ops shrinker_info_fops = {
	.proc_open    = shrinker_info_proc_open,
	.proc_read    = seq_read,
	.proc_lseek   = seq_lseek,
	.proc_release = single_release,
};

static int active_info_proc_show(struct seq_file *m, void *v)
{
	int i;

	seq_puts(m, "shrinker active\n");
	down_read(&record_rwsem);
	for (i = 0; i < SHRINKER_MAX_NUM; i++) {
		if (record[i].scan_objects)
			seq_printf(m, "%ps, %lu\n",
				record[i].scan_objects,
				record[i].active);
	}
	up_read(&record_rwsem);

	return 0;
}

static int shrinker_type_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, active_info_proc_show, NULL);
}

static const struct proc_ops shrinker_type_fops = {
	.proc_open    = shrinker_type_proc_open,
	.proc_read    = seq_read,
	.proc_lseek   = seq_lseek,
	.proc_release = single_release,
};

static int __init split_shrinker_init(void)
{
	proc_create("shrinker_info", 0440, NULL, &shrinker_info_fops);
	proc_create("shrinker_type", 0440, NULL, &shrinker_type_fops);

	return 0;
}

module_init(split_shrinker_init);

#ifdef CONFIG_SPLIT_SHRINKER_DEBUG
static void mv_shrinkers_to_cold_list(enum shrinker_type type)
{
	int i;
	struct shrinker *shrinker = NULL;
	struct shrinker *shrinker_tmp = NULL;

	if (type == ST_COLD)
		return;

	i = 0;
	down_write(&g_shrinker_sems[ST_COLD]);
	down_write(&g_shrinker_sems[type]);
	list_for_each_entry_safe(shrinker, shrinker_tmp, &g_shrinker_lists[type], list) {
		list_del(&shrinker->list);
		list_add_tail(&shrinker->list, &g_shrinker_lists[ST_COLD]);
		shrinker->flags &= ~SPLIT_SHRINKER_MASK;
		shrinker->flags |= SHRINKER_COLD;
		i++;
	}
	up_write(&g_shrinker_sems[type]);
	up_write(&g_shrinker_sems[ST_COLD]);

	pr_err("mv %d %s shrinkers to cold list\n", i, shrinker_type_str[type]);
}

/*
 * Some skip shrinkers of file system are not on any list, this function is used to register
 * them to target list.
 */

static void mv_fs_skip_shrinker_to_list(struct super_block *sb, void *arg)
{
	enum shrinker_type orig_type, dest_type;

	orig_type = get_shrinker_type(&sb->s_shrink, false);
	if (orig_type != ST_SKIP)
		return;

	if (!list_empty(&sb->s_shrink.list))
		return;

	dest_type = *(enum shrinker_type *)arg;
	if (dest_type == ST_COLD) {
		sb->s_shrink.flags &= ~SPLIT_SHRINKER_MASK;
		sb->s_shrink.flags |= SHRINKER_COLD;
	}

	pr_err("mv %s fs_shrinker from %s to %s\n", sb->s_type->name,
		shrinker_type_str[orig_type], shrinker_type_str[dest_type]);

	register_split_shrinker(&sb->s_shrink);
}

static void mv_fs_skip_shrinkers_to_list(enum shrinker_type type)
{
	iterate_supers(mv_fs_skip_shrinker_to_list, &type);
}

int split_shrinker_sysctl_handler(struct ctl_table *table, int write,
	void *buffer, size_t *length, loff_t *ppos)
{
	int ret;
	enum shrinker_type type;

	ret = proc_dointvec_minmax(table, write, buffer, length, ppos);
	if (ret)
		return ret;

	if (write) {
		if (is_split_shrinker_enable()) {
			pr_err("split shrinker is enable!\n");
			return 0;
		}
		for (type = ST_COLD + 1; type < ST_MAX; type++)
			mv_shrinkers_to_cold_list(type);

		mv_fs_skip_shrinkers_to_list(ST_COLD);
	}

	return 0;
}

int register_skip_shrinker_sysctl_handler(struct ctl_table *table, int write,
	void *buffer, size_t *length, loff_t *ppos)
{
	int ret;

	ret = proc_dointvec_minmax(table, write, buffer, length, ppos);
	if (ret)
		return ret;

	if (write) {
		if (!is_debug_skip_shrinker_enable()) {
			pr_err("debug skip shrinker is disable!\n");
			return 0;
		}
		mv_fs_skip_shrinkers_to_list(ST_SKIP);
	}

	return 0;
}
#endif
