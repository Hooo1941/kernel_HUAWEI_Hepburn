/*
 * memcheck_fd_socket.c
 *
 * Socket leak detector
 *
 * Copyright(C) 2023 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <audit.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/fdtable.h>
#include <linux/printk.h>
#include <linux/sched/task.h>
#include <linux/sched/signal.h>
#include <linux/socket.h>
#include <uapi/linux/socket.h>
#include <linux/version.h>
#include <net/sock.h>
#include "memcheck_ioctl.h"

struct socket_info_args {
	struct seq_file *seq;
	struct task_struct *task;
};

static int socket_info_cb(const void *data, struct file *f, unsigned int fd)
{
	struct socket_info_args *args = (struct socket_info_args *)data;
	struct task_struct *task = args->task;
	struct socket *sock = NULL;
	struct sock *sk = NULL;
	pid_t peer_tid;
	int err;

	sock = sock_from_file(f, &err);
	if (!sock || !sock->sk)
		return 0;
	sk = sock->sk;
	spin_lock(&sk->sk_peer_lock);
	peer_tid = pid_vnr(sk->sk_peer_pid);
	spin_unlock(&sk->sk_peer_lock);
	seq_printf(args->seq, "%s %u %u %zu %u\n", task->comm, task->pid, fd, file_inode(f)->i_ino, peer_tid);

	return 0;
}

static int socket_info_show(struct seq_file *s, void *d)
{
	struct task_struct *task = NULL;
	struct socket_info_args cb_args;
	unsigned long old_ns = ktime_get();

	seq_puts(s, "Process socket info:\n");
	seq_puts(s, "----------------------------------------------------\n");
	seq_printf(s, "%s %s %s %s %s\n",
		   "ProcessName", "ProcessID", "Fd", "inode", "PeerTid");

	rcu_read_lock();
	for_each_process(task) {
		if (task->flags & PF_KTHREAD)
			continue;

		cb_args.seq = s;
		cb_args.task = task;

		task_lock(task);
		iterate_fd(task->files, 0, socket_info_cb, (void *)&cb_args);
		task_unlock(task);
	}
	rcu_read_unlock();

	memcheck_info("take %ld ns", ktime_get() - old_ns);

	return 0;
}

static int socket_info_open(struct inode *inode, struct file *file)
{
	return single_open(file, socket_info_show, inode->i_private);
}

#if (KERNEL_VERSION(5, 10, 0) <= LINUX_VERSION_CODE)
static const struct proc_ops socket_info_fops = {
	.proc_open = socket_info_open,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
};
#else
static const struct file_operations socket_info_fops = {
	.open = socket_info_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};
#endif

int memcheck_fd_socket_createfs(void)
{
	struct proc_dir_entry *entry = NULL;

	entry = proc_create_data("socket_process_info", 0444,
		NULL, &socket_info_fops, (void *)0);
	if (!entry)
		memcheck_err("Failed to create socket debug info\n");

	return (!entry ? -EFAULT : 0);
}
