/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description: hw_stat's framework
 * Author: Qin Hongwei <qinhongwei7@huawei.com>
 * Create: 2023-02-24
 */
#include <linux/hw_stat.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/blkdev.h>
#include <securec.h>
#include <trace/events/android_fs.h>

#define PATH_HEAD_LEN (16)
#define PATH_TAIL_LEN (20)

// Caller ensures all params not null
void hw_stat_trace_fill_name(struct hw_stat_trace_param *p_param, char **p_path,
	struct inode *inode, char *name_buf, unsigned int name_buf_len)
{
	*p_path = android_fstrace_get_pathname(name_buf, name_buf_len, inode);
#ifdef CONFIG_HW_STAT_TRACE_FULLPATH
	p_param->note2 = *p_path;
#else
	/*
	 * For privacy mode, we can't print out the file path.
	 * Instead, we categorize path into several categories
	 * and print it out.
	 */
	snprintf_s(name_buf, name_buf_len, name_buf_len - 1, "***");
	p_param->note2 = name_buf;
#endif
}
EXPORT_SYMBOL(hw_stat_trace_fill_name);

static int g_trace_enable = 0;

static int get_trace_begin_buf(char *buf, int len,
	const struct hw_stat_trace_param *param)
{
	const char *note1 = param->note;
	const char *note2 = param->note2;
	int note2_len = 0;

	if (note1 == NULL)
		note1 = "NULLSTR";
	if (note2 == NULL)
		note2 = "NULLSTR";
	note2_len = strlen(note2);
	if (note2_len <= PATH_HEAD_LEN + PATH_TAIL_LEN)
		return snprintf_s(buf, len, len - 1,
			"B|%d|%s,%s,0x%lx,0x%lx,0x%lx\n",
			current->tgid, note1, note2,
			param->value1, param->value2, param->value3);

	return snprintf_s(buf, len, len - 1,
		"B|%d|%s,%.16s...%s,0x%lx,0x%lx,0x%lx\n",
		current->tgid, note1, note2, note2 + note2_len - PATH_TAIL_LEN,
		param->value1, param->value2, param->value3);
}

static int __hw_stat_trace(int is_start, const struct hw_stat_trace_param *param)
{
	int count = 0;
	loff_t pos = 0;
	int ret = 0;
	mm_segment_t fs;
	char buf[HW_STAT_MAX_BUF_LEN] = {0};
	struct file *fp = filp_open("/sys/kernel/tracing/trace_marker",
		O_CLOEXEC | O_WRONLY | O_APPEND, 0);

	if (IS_ERR(fp)) {
		pr_err("hw_stat: open /sys/kernel/tracing/trace_marker err!\n");
		return 0;
	}

	if (is_start)
		count = get_trace_begin_buf(buf, HW_STAT_MAX_BUF_LEN, param);
	else
		count = snprintf_s(buf, HW_STAT_MAX_BUF_LEN, HW_STAT_MAX_BUF_LEN - 1,
			"E|%d|%d\n", current->tgid, param->value1);

	if (likely(count > 0)) {
		fs = get_fs();
		set_fs(get_ds());
		ret = vfs_write(fp, buf, count, &pos);
		if (ret < 0) {
			pr_err("hw_stat: write trace_marker error, ret=%d!\n", ret);
			is_start = 0;
		}
		set_fs(fs);
	}
	filp_close(fp, NULL);
	return is_start;
}

/*
 * Caller ensures @param not null.
 * @Return: If @is_start is HW_STAT_TRACE_START, this function
 *   returns 1 on success, indicating the trace slice actually starts.
 *   If @is_start is HW_STAT_TRACE_END, this function returns 0 on
 *   success, indicating the trace slice actually ends.
 */
int hw_stat_trace(int is_start, const struct hw_stat_trace_param *param)
{
	if (param->ignore_qos || current_is_high_prio())
		return __hw_stat_trace(is_start, param);
	return 0;
}
EXPORT_SYMBOL(hw_stat_trace);

int hw_stat_trace_enabled(void)
{
	return READ_ONCE(g_trace_enable);
}
EXPORT_SYMBOL(hw_stat_trace_enabled);

static int hw_stat_trace_enable_show(struct seq_file *seq, void *arg)
{
	seq_printf(seq, "%d\n", g_trace_enable);
	return 0;
}

static int hw_stat_trace_enable_open(struct inode *inode, struct file *filp)
{
	return single_open(filp, hw_stat_trace_enable_show, NULL);
}

static ssize_t hw_stat_trace_enable_write(struct file *file,
					 const char __user *buffer,
					 size_t count, loff_t *pos)
{
	char enable;

	if (count <= 0)
		return -EINVAL;

	if (get_user(enable, buffer))
		return -EFAULT;

	if (enable == '0')
		smp_store_release(&g_trace_enable, 0);
	else if (enable == '1')
		smp_store_release(&g_trace_enable, 1);
	else
		return -EINVAL;

	return count;
}

static const struct proc_ops hw_stat_trace_enable_proc_fops = {
	.proc_open	= hw_stat_trace_enable_open,
	.proc_read	= seq_read,
	.proc_write	= hw_stat_trace_enable_write,
	.proc_release	= single_release,
};

void hw_stat_init_trace_marker(struct proc_dir_entry *parent)
{
	proc_create("trace_marker_enable", S_IRUSR | S_IWUSR, parent,
		    &hw_stat_trace_enable_proc_fops);
}
