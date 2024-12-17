/*
 * wakeup_new_task_fast_procfs.c
 *
 * Interface for making new task wakeup faster
 *
 * Copyright (c) 2022-2022 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

int wakeup_new_task_fast_show(struct seq_file *m, void *v)
{
	struct inode *inode = m->private;
	struct task_struct *p = NULL;

	p = get_proc_task(inode);
	if (!p)
		return -ESRCH;

	seq_printf(m, "%u\n", get_wakeup_new_task_fast(p));

	put_task_struct(p);

	return 0;
}

int wakeup_new_task_fast_open(struct inode *inode, struct file *filp)
{
	return single_open(filp, wakeup_new_task_fast_show, inode);
}

ssize_t wakeup_new_task_fast_write(struct file *file, const char __user *buf,
				   size_t count, loff_t *offset)
{
	struct task_struct *p = NULL;
	unsigned int val;
	int err;

	err = kstrtouint_from_user(buf, count, 0, &val);
	if (err)
		return err;

	p = get_proc_task(file_inode(file));
	if (!p)
		return -ESRCH;

	set_wakeup_new_task_fast(p, val);

	put_task_struct(p);

	return count;
}

const struct file_operations proc_wakeup_new_task_fast_operations = {
	.open       = wakeup_new_task_fast_open,
	.read       = seq_read,
	.write      = wakeup_new_task_fast_write,
	.llseek     = seq_lseek,
	.release    = single_release,
};
