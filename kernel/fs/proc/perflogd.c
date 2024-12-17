#include <linux/cpufreq_times.h>
#include <linux/cpumask.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel_stat.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/sched/stat.h>
#include <linux/seq_file.h>
#include <linux/slab.h>
#include <linux/time.h>
#include <linux/irqnr.h>
#include <linux/sched/cputime.h>
#include <linux/tick.h>
#include "securec.h"
#include <linux/cpufreq.h>
#include "../../kernel/sched/sched.h"
#include <linux/spinlock.h>
#include "internal.h"
#include <linux/string.h>
#include <linux/sched/topology.h>
#include <linux/threads.h>
#include <linux/mutex.h>
#include <linux/sched/smt.h>

#define DMIPS_NUM 16
#define MIN_CPUTIME_UID 1000

int dmips_value_buffer[DMIPS_NUM];
int dmips_value_buffer_default[DMIPS_NUM];
#define KERNEL_PID_MAX 200

#define SAMPLE_PERIOD 100000
#define SAMPLE_MIN 20000
#define SAMPLE_MAX 1000000
#define BUFFER_SIZE 150000
#define BUFFER_MIN 15000
#define BUFFER_MAX 1500000
#define FREQ_OFFSET 100000
#define INIT_PID 1
#define KTHREADD_PID 2
#define PIDS_NUM 8
#define MAX_LIMIT_PID 100
#define MAX_ARRAY_LNGTH 256

struct task_freq_time {
	pid_t pid;
	char comm[TASK_COMM_LEN];
	u64 cur_clock;
	u64 cpu_load;
};

struct pid_group_info {
	int num;
	unsigned int pids[MAX_LIMIT_PID];
	bool is_now_exit[MAX_LIMIT_PID];
};

static struct pid_group_info pid_group_infos = {
	.num = 0,
	.pids = {0},
	.is_now_exit = {false},
};

int capacity_buffer[DMIPS_NUM];
static int capacity_small = 8;


DEFINE_SPINLOCK(cpu_load_report_lock);
static struct mutex cpu_limit_lock;

static struct delayed_work load_report_work;

static int validlen;
static struct task_freq_time* p_head = NULL; // always point to head
static struct task_freq_time* p_tail = NULL;
static struct task_freq_time* p_valid_head = NULL; // read entry
static struct task_freq_time* p_valid_tail = NULL; // write entry

static struct task_freq_time *tft_buffer;
static int sample_period;
static unsigned long tft_buffer_size = 0;
static u64 current_clock;
static char target_comm[TASK_COMM_LEN] = { '\0' };
static bool report_thread = false;
static bool report_enable = false;
unsigned int sysctl_perflogd_ex_report = 1;
static bool is_init_capacity_buffer = false;


static int show_perflogd(struct seq_file *p, void *v)
{
	unsigned long proc_id;
	unsigned long proc_cputime_total;
	struct task_struct *task;

	if (dmips_value_buffer[0] == 0) {
		printk(KERN_ERR "perflogd DMIPS NOT INIT\n");
		return 0;
	}
	proc_cputime_total = 0;
	task = &init_task;
	rcu_read_lock();
	for_each_process(task) {
		if (task->pid < KERNEL_PID_MAX)
			continue;
		proc_cputime_total = get_proc_cpu_load(task, dmips_value_buffer, DMIPS_NUM);
		if (proc_cputime_total > 0) {
			proc_id = task->pid;
			seq_printf(p, "%lu ", proc_id);
			seq_printf(p, "%lu\n", proc_cputime_total);
		}
	}
	rcu_read_unlock();
	return 0;
}

static int perflogd_open(struct inode *inode, struct file *file)
{
	return single_open(file, show_perflogd, NULL);
}

static ssize_t perflogd_write(struct file *filp, const char __user *userbuf, size_t count, loff_t *ppos)
{
	int ret;
	ret = memset_s(dmips_value_buffer, sizeof(dmips_value_buffer), 0, sizeof(dmips_value_buffer));
	if (ret != 0) {
		printk(KERN_ERR "perflogd memset_s failed\n");
		return -EFAULT;
	}
	if ((count < 0) || (count > DMIPS_NUM)) {
		printk(KERN_ERR "perflogd invalid count\n");
		return -EFAULT;
	}

	char recv_buf[DMIPS_NUM] = {0};
	if (copy_from_user(recv_buf, userbuf, count)) {
		printk(KERN_ERR "perflogd copy_from_user failed\n");
		return -EFAULT;
	}

	for (int i =  0; i < DMIPS_NUM; i++) {
		dmips_value_buffer[i] = (int)(recv_buf[i]);
	}

	return 0;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(5,6,0)
static const struct file_operations proc_perflogd_operations = {
	.open		= perflogd_open,
	.write		= perflogd_write,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};
#else
static const struct proc_ops proc_perflogd_operations = {
	.proc_open		= perflogd_open,
	.proc_write		= perflogd_write,
	.proc_read		= seq_read,
	.proc_lseek		= seq_lseek,
	.proc_release	= single_release,
};
#endif

static int show_perflogd_cputime(struct seq_file *p, void *v)
{
	unsigned long proc_id;
	u64 clock_time;
	struct task_struct *task;
	u64 utime;
	u64 stime;
	u32 uid;

	rcu_read_lock();
	task = &init_task;
	for_each_process(task) {
		uid = task_uid(task).val;
		if (uid < MIN_CPUTIME_UID)
			continue;
		if (task->signal->oom_score_adj < 0)
			continue;
		utime = 0;
		stime = 0;
		proc_id = task->pid;
		thread_group_cputime_adjusted(task, &utime, &stime);
		clock_time = nsec_to_clock_t(utime + stime);
		seq_printf(p, "%lu ", proc_id);
		seq_printf(p, "%lu ", uid);
		seq_printf(p, "%llu\n", clock_time);
	}
	rcu_read_unlock();
	return 0;
}

static int perflogd_cputime_open(struct inode *inode, struct file *file)
{
	return single_open(file, show_perflogd_cputime, NULL);
}

static const struct proc_ops proc_perflogd_cputime_operations = {
	.proc_open		= perflogd_cputime_open,
	.proc_read		= seq_read,
	.proc_lseek		= seq_lseek,
	.proc_release	= single_release,
};

static int __init proc_perflogd_init(void)
{
	proc_create("perflogd", 0446, NULL, &proc_perflogd_operations);
	proc_create("perflogd_cputime", 0446, NULL, &proc_perflogd_cputime_operations);
	return 0;
}
module_init(proc_perflogd_init);

#ifdef CONFIG_HUAWEI_PERFD_EX
bool tft_buffer_alloc(void)
{
	spin_lock(&cpu_load_report_lock);
	if (tft_buffer) {
		vfree(tft_buffer);
		tft_buffer = NULL;
	}

	tft_buffer = (struct task_freq_time *)vmalloc(sizeof(struct task_freq_time) * tft_buffer_size);
	if (!tft_buffer) {
		printk(KERN_ERR "cpuload tft_buffer_alloc error\n");
		spin_unlock(&cpu_load_report_lock);
		return false;
	}
	spin_unlock(&cpu_load_report_lock);

	return true;
}

void tft_ring_buffer_init(void)
{
	spin_lock(&cpu_load_report_lock);
	if (tft_buffer) {
		p_head = tft_buffer;
		p_valid_head = p_head;
		p_valid_tail = p_head;
		p_tail = p_head + tft_buffer_size;
		validlen = 0;
	}
	spin_unlock(&cpu_load_report_lock);
}

void tft_ring_buffer_exit(void)
{
	spin_lock(&cpu_load_report_lock);
	if (tft_buffer) {
		vfree(tft_buffer);
		tft_buffer = NULL;
	}
	p_head = NULL;
	p_tail = NULL;
	p_valid_head = NULL;
	p_valid_tail = NULL;
	validlen = 0;
	spin_unlock(&cpu_load_report_lock);
}

int tft_ring_buffer_write(struct task_struct *p, u64 clock)
{
	if (p_head == NULL) {
		printk(KERN_ERR "cpuload tft_ring_buffer_write p_head invalid\n");
		return -1;
	}

	spin_lock(&cpu_load_report_lock);
	if (p_valid_tail + 1 > p_tail) {
		if (dmips_value_buffer[0])
			p_head->cpu_load = get_cpuload_total(p, dmips_value_buffer, DMIPS_NUM, report_thread);
		else
			p_head->cpu_load = get_cpuload_total(p, dmips_value_buffer_default, DMIPS_NUM, report_thread);
		memcpy_s(p_head->comm, sizeof(char) * TASK_COMM_LEN, p->comm, sizeof(char) * TASK_COMM_LEN);
		p_head->pid = p->pid;
		p_head->cur_clock = clock;
		p_valid_tail = p_head + 1;
	} else {
		if (dmips_value_buffer[0])
			p_valid_tail->cpu_load = get_cpuload_total(p, dmips_value_buffer, DMIPS_NUM, report_thread);
		else
			p_valid_tail->cpu_load = get_cpuload_total(p, dmips_value_buffer_default, DMIPS_NUM, report_thread);
		memcpy_s(p_valid_tail->comm, sizeof(char) * TASK_COMM_LEN, p->comm, sizeof(char) * TASK_COMM_LEN);
		p_valid_tail->pid = p->pid;
		p_valid_tail->cur_clock = clock;
		p_valid_tail++;
	}

	if (validlen + 1 > tft_buffer_size) {
		if (p_valid_head + 1 > p_tail) {
			p_valid_head = p_head + 1;
		} else {
			p_valid_head++;
		}
		validlen = tft_buffer_size;
	} else {
		validlen++;
	}
	spin_unlock(&cpu_load_report_lock);

	return 0;
}

int tft_ring_buffer_read(struct seq_file *m, struct task_freq_time *buffer, int length)
{
	int len1, len2;

	if (p_head == NULL) {
		printk(KERN_ERR "cpuload tft_ring_buffer_read p_head invalid\n");
		return -1;
	}

	if (validlen == 0) {
		printk(KERN_ERR "cpuload tft_ring_buffer_read validlen invalid\n");
		return 0;
	}

	if (length > validlen)
		length = validlen;

	spin_lock(&cpu_load_report_lock);
	if (p_valid_head + length > p_tail) {
		len1 = p_tail - p_valid_head;
		len2 = length - len1;
		memcpy_s(buffer, length * sizeof(struct task_freq_time), p_valid_head, len1 * sizeof(struct task_freq_time));
		memcpy_s(buffer + len1, length * sizeof(struct task_freq_time), p_head, len2 * sizeof(struct task_freq_time));
		p_valid_head = p_head + len2;
	} else {
		memcpy_s(buffer, length * sizeof(struct task_freq_time), p_valid_head, length * sizeof(struct task_freq_time));
		p_valid_head += length;
	}
	validlen -= length;
	spin_unlock(&cpu_load_report_lock);

	return length;
}

void perflogd_ex_write_thread(struct task_struct *p)
{
	struct task_struct *child = p;

	do {
		tft_ring_buffer_write(child, current_clock);
	} while_each_thread(p, child);
}

void perflogd_ex_tickfn(struct work_struct *work)
{
	struct task_struct *p = NULL;
	char len;

	if (!report_enable) {
		tft_ring_buffer_exit();
		return;
	}

	if (!sysctl_perflogd_ex_report)
		goto out;

	current_clock = local_clock();
	len = strlen(target_comm);

	for_each_process(p) {
		if (report_thread) {
			if (len && (strcmp(p->comm, target_comm) == 0))
				perflogd_ex_write_thread(p); // refers to -t + comm
		} else {
			if (!len) {
				tft_ring_buffer_write(p, current_clock); // refers to -p
			} else if (strcmp(p->comm, target_comm) == 0) {
				tft_ring_buffer_write(p, current_clock); // refers to -p + comm
			}
		}
	}

out:
	schedule_delayed_work(&load_report_work, usecs_to_jiffies(sample_period));
}

int perflogd_ex_show(struct seq_file *m, void *v)
{
	struct task_freq_time * read_buffer;
	int read_buffer_len = validlen;
	unsigned long target_size;

	if (!read_buffer_len) {
		printk(KERN_ERR "cpuload perflogd_ex_show validlen invaild\n");
		return 0;
	}

	target_size = sizeof(struct task_freq_time) * read_buffer_len;
	if (target_size > m->size) {
		m->count = m->size; // seq overflowed
		return 0;
	}

	read_buffer = (struct task_freq_time *)vmalloc(sizeof(struct task_freq_time) * read_buffer_len);
	if (!read_buffer) {
		printk(KERN_ERR "cpuload perflogd_ex_show alloc mem error\n");
		return -ENOMEM;
	}

	if (tft_ring_buffer_read(m, read_buffer, read_buffer_len)) {
		for (int i = 0 ; i < read_buffer_len; i++)
			seq_printf(m, "%d %s %llu %lu\n",
				read_buffer[i].pid, read_buffer[i].comm, read_buffer[i].cpu_load,
				(unsigned long)nsec_to_clock_t(read_buffer[i].cur_clock));
	}

	vfree(read_buffer);
	read_buffer = NULL;

	return 0;
}

void get_default_dmips_buffer(void)
{
	int cpu, temp_capacity;
	unsigned long temp_freq;

	for_each_possible_cpu(cpu) {
		temp_freq = cpufreq_get_hw_max_freq(cpu);
		temp_capacity = capacity_orig_of(cpu);
		dmips_value_buffer_default[cpu] = (temp_capacity * FREQ_OFFSET) / temp_freq;
	}
}

static int perflogd_ex_init(void)
{
	sample_period = SAMPLE_PERIOD;
	tft_buffer_size = BUFFER_SIZE;

	if (!report_enable && tft_buffer_alloc()) {
		report_enable = true;
		tft_ring_buffer_init();
		if (!dmips_value_buffer[0])
			get_default_dmips_buffer();

		INIT_DEFERRABLE_WORK(&load_report_work, perflogd_ex_tickfn);
		schedule_delayed_work(&load_report_work, usecs_to_jiffies(sample_period));
	}

	return 0;
}

bool perflogd_ex_regular_match_pro(char *option, char *buffer)
{
	unsigned int time;

	if (!option || !buffer)
		return false;

	if (strcmp(option, "s") == 0) {
		if (kstrtoint(buffer, 0, &time))
			return false;
		sample_period = clamp(time, SAMPLE_MIN, SAMPLE_MAX);
		tft_ring_buffer_init();
	} else if (strcmp(option, "b") == 0) {
		if (kstrtoint(buffer, 0, &time))
			return false;
		tft_buffer_size = clamp(time, BUFFER_MIN, BUFFER_MAX);
		if (tft_buffer_alloc())
			tft_ring_buffer_init();
	} else if (strcmp(option, "p") == 0) {
		tft_ring_buffer_init();
		report_thread = false;
		strcpy_s(target_comm, TASK_COMM_LEN, buffer);
	} else if (strcmp(option, "t") == 0) {
		tft_ring_buffer_init();
		report_thread = true;
		strcpy_s(target_comm, TASK_COMM_LEN, buffer);
	} else {
		return false;
	}

	return true;
}

bool perflogd_ex_regular_match(char *option)
{
	if (!option)
		return false;

	if (strcmp(option, "p") == 0) {
		tft_ring_buffer_init();
		report_thread = false;
		memset_s(target_comm, TASK_COMM_LEN, 0, TASK_COMM_LEN);
	} else if (strcmp(option, "i") == 0) {
		report_thread = false;
		memset_s(target_comm, TASK_COMM_LEN, 0, TASK_COMM_LEN);
		perflogd_ex_init();
	} else if (strcmp(option, "d") == 0) {
		report_enable = false;
	} else {
		return false;
	}

	return true;
}

static ssize_t perflogd_ex_write(struct file *file, const char __user *buf,
			     size_t count, loff_t *ppos)
{
	char buffer[TASK_COMM_LEN];
	char option[PROC_NUMBUF];
	char *kbuf = NULL;
	int err;

	kbuf = memdup_user_nul(buf, count);
	if (IS_ERR(kbuf)) {
		printk(KERN_ERR "cpuload perflogd_ex_write kbuf invalid\n");
		return PTR_ERR(kbuf);
	}

	if (sscanf_s(kbuf, "-%s %s", option, PROC_NUMBUF, buffer, TASK_COMM_LEN) == 2) {
		if (!perflogd_ex_regular_match_pro(option, buffer))
			printk(KERN_ERR "cpuload perflogd_ex_regular_match_pro match failed\n");
	} else if (sscanf_s(kbuf, "-%s", option, PROC_NUMBUF) == 1) {
		if (!perflogd_ex_regular_match(option))
			printk(KERN_ERR "cpuload perflogd_ex_regular_match match failed\n");
	} else {
		err = -EINVAL;
		printk(KERN_ERR "cpuload perflogd_ex_write match no sscanf condition\n");
	}

out:
	kfree(kbuf);
	return err < 0 ? err : count;
}

static int perflogd_ex_open(struct inode *inode, struct file *file)
{
	return single_open(file, perflogd_ex_show, inode);
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(5,6,0)
static const struct file_operations proc_cpu_load_report_operations = {
	.open = perflogd_ex_open,
	.write = perflogd_ex_write,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};
#else
static const struct proc_ops proc_cpu_load_report_operations = {
	.proc_open		= perflogd_ex_open,
	.proc_write		= perflogd_ex_write,
	.proc_read		= seq_read,
	.proc_lseek		= seq_lseek,
	.proc_release	= single_release,
};
#endif
#endif

static void update_pid_info_group(bool delete, int pid)
{
	int num = pid_group_infos.num;
	struct pid_group_info temp_pid_group = {
		.num = 0,
		.pids = {0, },
		.is_now_exit = {false, },
	};
	int now_num = 0;
	if (!delete) {
		printk(KERN_INFO "perflogd update pid info!");
		for (int i = 0; i < num; i++) {
			if (pid_group_infos.is_now_exit[i])
				temp_pid_group.pids[now_num++] = pid_group_infos.pids[i];
		}
	} else {
		printk(KERN_INFO "perflogd delete pid info!");
		for (int i = 0; i < num; i++) {
			if (pid_group_infos.pids[i] == pid) {
				printk(KERN_INFO "perflogd delete monitor pid: %d\n", pid);
				continue;
			}
			temp_pid_group.pids[now_num++] = pid_group_infos.pids[i];
		}
	}
	if (now_num < MAX_LIMIT_PID) {
		temp_pid_group.num = now_num;
		pid_group_infos = temp_pid_group;
	}
}

static void init_capacity_buffer(void)
{
	int cpu;
	for_each_possible_cpu(cpu) {
		capacity_buffer[cpu] = topology_get_cpu_scale(cpu) / capacity_small;
	}
}

static void parse_input_info(char *input_str)
{
	unsigned int pid;
	char *temp_input = input_str;

	if (temp_input == NULL) {
		printk(KERN_ERR "temp_input is null, please check your input param.");
		return;
	}
	printk(KERN_INFO "perflogd input_str is: %s\n", input_str);

	char *temp_pid = strsep(&temp_input, " ");
	if (temp_input == NULL || temp_pid == NULL) {
		printk(KERN_ERR "input error, please check your input param.");
		return;
	}

	int flag = simple_strtol(temp_input, NULL, 0);
	pid = simple_strtol(temp_pid, NULL, 0);

	// flag == 1 means add pid to monitor, flag == 0 means delete pid to monitor
	mutex_lock(&cpu_limit_lock);
	if (flag == 1) {
		int num = pid_group_infos.num;
		if (num >= MAX_LIMIT_PID) {
			printk(KERN_ERR "perflogd monitor pid is full, can not add it.");
			mutex_unlock(&cpu_limit_lock);
			return;
		}
		pid_group_infos.pids[num] = pid;
		pid_group_infos.is_now_exit[num] = false;
		pid_group_infos.num++;
		printk(KERN_INFO "perflogd add pid to monitor list success.");
	} else if (flag == 0) {
		update_pid_info_group(true, pid);
		printk(KERN_INFO "perflogd delete pid from monitor list success.");
	} else {
		printk(KERN_ERR "perflogd flag is not right, please check it again.");
	}
	mutex_unlock(&cpu_limit_lock);
}

static ssize_t perflogd_cpuusage_write(struct file *filp, const char __user *userbuf, size_t count, loff_t *ppos)
{
	int ret;
	char temp_receive_value[MAX_ARRAY_LNGTH];
	printk(KERN_INFO "perflogd userbuf is getIn");
	ret = memset_s(temp_receive_value, MAX_ARRAY_LNGTH, 0, sizeof(temp_receive_value));
	if (ret != 0) {
		printk(KERN_ERR "perflogd_cpuusage memset_s failed\n");
		return -EFAULT;
	}
	if ((count < 0) || (count > MAX_ARRAY_LNGTH)) {
		printk(KERN_ERR "perflogd_cpuusage invalid count\n");
		return -EFAULT;
	}
	if (copy_from_user(temp_receive_value, userbuf, count)) {
		printk(KERN_ERR "perflogd_cpuusage copy_from_user failed\n");
		return -EFAULT;
	 }
	parse_input_info(temp_receive_value);
	return count;
}

static int show_perflogd_cpuusage(struct seq_file *p, void *v)
{
	unsigned long proc_pid;
	struct task_struct *task;
	unsigned long proc_cpuload_total;
	unsigned long temp_proc_cpuload = 0;

	if (!is_init_capacity_buffer) {
		init_capacity_buffer();
		is_init_capacity_buffer = true;
	}
	task = &init_task;
	proc_cpuload_total = 0;
	printk(KERN_INFO "perflogd now pid_group_infos num is: %d\n", pid_group_infos.num);
	int temp_pids_num = 0;
	mutex_lock(&cpu_limit_lock);
	if (pid_group_infos.num == 0) {
		mutex_unlock(&cpu_limit_lock);
		return -1;
	}
	rcu_read_lock();
	for_each_process(task) {
		if (task->tgid == INIT_PID || task->tgid == KTHREADD_PID)
			continue;
		if (task->real_parent && task->real_parent->tgid == KTHREADD_PID && !strstr(task->comm, "swapd"))
			continue;
		proc_pid = task->pid;
		for (int i = 0; i < pid_group_infos.num; i++) {
			unsigned long cpulimit_proc_pid = pid_group_infos.pids[i];
			if (cpulimit_proc_pid != proc_pid) {
				continue;
			}
			printk(KERN_INFO "perflogd proc_pid is equals cpu limit proc pid, pid is : %d\n", proc_pid);
			temp_pids_num++;
			proc_cpuload_total = get_proc_cpu_load(task, capacity_buffer, DMIPS_NUM) * capacity_small;
			if (proc_cpuload_total > 0) {
				printk(KERN_INFO "get proc_cpuload_total success");
				seq_printf(p, "%lu ", proc_pid);
				seq_printf(p, "%llu\n", proc_cpuload_total);
				pid_group_infos.is_now_exit[i] = true;
			}
			break;
		}
	}
	rcu_read_unlock();
	// update pid array, because some target pid id died.
	if (temp_pids_num != pid_group_infos.num)
		update_pid_info_group(false, 0);

	mutex_unlock(&cpu_limit_lock);
	return 0;
}

static int perflogd_cpuusage_open(struct inode *inode, struct file *file)
{
	return single_open(file, show_perflogd_cpuusage, NULL);
}

static const struct proc_ops perflogd_cpuusage_options = {
	.proc_open		= perflogd_cpuusage_open,
	.proc_write		= perflogd_cpuusage_write,
	.proc_read		= seq_read,
	.proc_lseek		= seq_lseek,
	.proc_release	= single_release,
};

static int __init proc_perflogd_pre_init(void)
{
	mutex_init(&cpu_limit_lock);
#ifdef CONFIG_HUAWEI_PERFD_EX
	proc_create("perflogd_ex", 0446, NULL, &proc_cpu_load_report_operations);
#endif
	proc_create("perflogd_cpuusage", 0644, NULL, &perflogd_cpuusage_options);
	return 0;
}
module_init(proc_perflogd_pre_init);

#ifdef CONFIG_SCHED_CPULOAD

static int cpuload_read_cpus(char __user **ubuf, size_t *count, size_t *done)
{
	u64 cap_orig, ravg_sum = 0, sum_cap_orig = 0;
	// To store the first line text: <cpus cpuload capacity> suach as <cpus 2345533246 4696>.
	char buf[4 + 1 + 20 + 1 + 20 + 1 + 1];
	int cpu, len;
	struct rq *rq;

	for_each_possible_cpu(cpu) {
		rq = cpu_rq(cpu);
		ravg_sum += READ_ONCE(rq->ravg_sum);
		/* capacity_orig_of() without HiSi's multipliers */
		cap_orig = topology_get_cpu_scale(cpu);
#ifdef CONFIG_SCHED_SMT
		if (rq->is_smt)
			cap_orig = cap_orig * sysctl_task_cap_discount_in_mt / 100;
#endif
		sum_cap_orig += cap_orig;
	}
	len = snprintf_s(buf, sizeof(buf), sizeof(buf) - 1, "cpus\t%llu\t%llu\n",
			ravg_sum, sum_cap_orig);
	if (len > *count)
		return -EINVAL;
	if (copy_to_user(*ubuf, buf, len))
		return -EFAULT;
	*count -= len;
	*ubuf += len;
	*done += len;
	return 0;
}

static int cpuload_read_task(pid_t pid, u64 ravg_sum, char __user **ubuf, size_t *count)
{
	// To store the pid cpuload text: <pid cpuload>, such as "12343 123434553".
	char buf[10 + 1 + 20 + 1 + 1];
	int len;

	len = snprintf_s(buf, sizeof(buf), sizeof(buf) - 1, "%d\t%llu\n", pid, ravg_sum);
	if (len > *count)
		return -EINVAL;
	WARN_ON(buf[len - 1] != '\n');
	if (copy_to_user(*ubuf, buf, len))
		return -EFAULT;
	*ubuf += len;
	*count -= len;
	return len;
}

static bool read_task_ravg_sum(struct task_struct *p, char __user **ubuf,
	size_t *count, size_t *done, loff_t *ppos)
{
	u64 ravg_sum;
	pid_t tgid;
	int ret;
	ravg_sum = atomic_long_read(&p->signal->ravg_sum);

	tgid = p->pid;
	ret = cpuload_read_task(tgid, ravg_sum, ubuf, count);
	if (ret < 0) {
		/* Partial read */
		if (ret == -EINVAL && *done)
			return false;
		*done = ret;
		return false;
	}
	*done += ret;
	*ppos = tgid;
	return true;
}

static ssize_t cpuload_read(struct file *file, char __user *ubuf,
		size_t count, loff_t *ppos)
{
	if (ubuf == NULL || ppos == NULL)
		return -EFAULT;

	struct task_struct *p;
	int ret;
	size_t done = 0;
	pid_t tgid;
	bool start = false;
	loff_t pos;

	if (*ppos < 0)
		return -EINVAL;
	if (*ppos >= pid_max || !count)
		return 0;

	if (*ppos == 0) {
		// first line
		ret = cpuload_read_cpus(&ubuf, &count, &done);
		if (ret < 0)
			return ret;
		*ppos = 1;
	}

	if (*ppos == 1) {
		// traversal from scratch
		rcu_read_lock();
		for_each_process(p) {
			// read ravg_sum and set zero
			if (!read_task_ravg_sum(p, &ubuf, &count, &done, &pos))
				break;
			// record the pid for next read
			*ppos = pos;
			if (!count)
				break;
		}
		rcu_read_unlock();
	} else {
		// traversal from the last position
		rcu_read_lock();
		for_each_process(p) {
			tgid = p->pid;
			if (!start && tgid != *ppos)
				continue;
			if (tgid == *ppos) {
				start = true;
				continue;
			}
 
			// read ravg_sum and set zero
			if (!read_task_ravg_sum(p, &ubuf, &count, &done, &pos))
				break;
			// record the pid for next read
			*ppos = pos;
			if (!count)
				break;
		}
		rcu_read_unlock();
	}
	return done;
}

static void cpuload_reset_all(void)
{
	struct task_struct *p;
	struct rq_flags rf;
	struct rq *rq;
	int cpu;

	for_each_possible_cpu(cpu) {
		rq = cpu_rq(cpu);
		rq_lock_irq(rq, &rf);
		rq->ravg_sum = 0;
		rq_unlock_irq(rq, &rf);
	}

	rcu_read_lock();
	for_each_process(p) {
		atomic_long_set(&p->signal->ravg_sum, 0);
	}
	rcu_read_unlock();
}
static ssize_t cpuload_write(struct file *file, const char __user *ubuf,
		size_t count, loff_t *pos)
{
	if (ubuf == NULL) {
		return -EFAULT;
	}

	char dummy;

	if (count > 0) {
		if (get_user(dummy, ubuf))
			return -EFAULT;
		cpuload_reset_all();
	}
	return count;
}

static const struct proc_ops cpuload_proc_ops = {
	.proc_read = cpuload_read,
	.proc_write = cpuload_write,
};

static int __init cpuload_init(void)
{
	proc_create("perflogd_cpuload", 0644, NULL, &cpuload_proc_ops);
	return 0;
}
module_init(cpuload_init);
#endif /* CONFIG_SCHED_CPULOAD */
