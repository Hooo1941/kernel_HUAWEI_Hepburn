#include <stdarg.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/time.h>
#include <linux/unistd.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/syscalls.h>
#include <linux/types.h>
#include <linux/uaccess.h>
#include <linux/version.h>
#include <linux/of_address.h>
#include <linux/pm_wakeup.h>
#include <linux/reboot.h>
#include <linux/slab.h>
#include <linux/proc_fs.h>
#include <linux/mutex.h>
#include "hwcxext_log.h"
#include <securec.h>

static struct mutex audio_log_lock;
#define AUDIO_LOG_BUF_SIZE  4096
#define AUDIO_CONT_BUF_SIZE 20

static const char *audio_dirname = "/var/log/audio";
static const char *audio_filename[] = { "/var/log/audio/audio_log1.log",
										"/var/log/audio/audio_log2.log",
										"/var/log/audio/audio_log3.log" };
static int file_select = 0;
static bool init_state = false;
static char *init_buff = NULL;
static int buffsize = 0;

#ifndef isspace
#define isspace(c) (((c) == ' ') || ((c) == '\t') || ((c) == '\r') || ((c) == '\n'))
#endif

#ifndef isdigit
#define isdigit(c) ((c) >= '0' && (c) <= '9')
#endif

static int audio_atoi(const char *s)
{
	int n = 0;
	int neg = 0;
	if (s == NULL)
		return neg;
	while (isspace(*s)) s++;
	switch (*s) {
	case '-': neg = 1; // should continue next case, no need break
	case '+': s++;
	}
	while (isdigit(*s))
		n = 10 * n - (*s++ - '0');
	return neg ? n : -n;
}

static int audio_filecont_read(const char *strfilename)
{
	int ret = -1;
	int cont = -1;
	if (strfilename == NULL)
		return ret;
	char buf[AUDIO_CONT_BUF_SIZE] = {0};
	struct file *fp = filp_open(strfilename, O_RDONLY, 0644);
	if (!IS_ERR(fp)) {
		vfs_llseek(fp, 0, SEEK_SET);
		if (vfs_read(fp, buf, AUDIO_CONT_BUF_SIZE - 1, &(fp->f_pos)) > 0) {
			ret = -2;
			cont = audio_atoi(buf);
			if (cont > 0)
				ret = 0;
		}
		filp_close(fp, NULL);
		if ((ret == -2) && (cont <= 0))
			ksys_unlink(strfilename);
	}
	return ret;
}

static int audio_filecont_write(const char *strfilename, int cont)
{
	int ret = -1;
	if (strfilename == NULL)
		return ret;
	char buf[AUDIO_CONT_BUF_SIZE] = {0};
	struct file *fp  = filp_open(strfilename, O_RDWR | O_CREAT, 0644);
	if (!IS_ERR(fp)) {
		vfs_llseek(fp, 0, SEEK_SET);
		int len = snprintf_s(&buf[0], AUDIO_CONT_BUF_SIZE,
			AUDIO_CONT_BUF_SIZE - 1, "%10d \n", cont);
		if (len > 0) {
			if (vfs_write(fp, buf, len, &(fp->f_pos)) > 0) {
				vfs_fsync(fp, 0);
				ret = 0;
			}
		}
		filp_close(fp, NULL);
	}
	return ret;
}

static int audio_get_filesize(const char *strfilename, int *size)
{
	struct kstat temp = {};
	if (vfs_stat(strfilename, &temp) < 0)
		return -1;
	*size = temp.size;
	return 0;
}

static int audio_log_file_check(const char **filename, int cnt)
{
	int ret = -1;
	if (filename == NULL)
		return ret;
	file_select = 0;
	int size = 0;
	int i;
	for (i = 0; i < cnt; i++) {
		if (ksys_access(filename[i], 0) != 0) {
			ret = audio_filecont_write(filename[i], i + 1);
			file_select = i;
			break;
		}
		if (audio_filecont_read(audio_filename[i]) < 0) {
			break;
		}
		if (audio_get_filesize(filename[i], &size) < 0) {
			break;
		}
		if (size < 0x1000000) {
			file_select = i;
			ret = 0;
			break;
		}
	}
	if (ret >= 0) {
		if (file_select > 0) {
			ksys_unlink(filename[file_select - 1]);
		} else {
			ksys_unlink(filename[cnt - 1]);
		}
		return ret;
	}
	return ret;
}

static int audio_log_init_check(void)
{
	mm_segment_t oldfs = get_fs();
	set_fs(KERNEL_DS);
	if (ksys_access(audio_dirname, 0) != 0) {
		if (ksys_mkdir(audio_dirname, 0755) != 0) {
			set_fs(oldfs);
			return -1;
		}
	}
	if (audio_log_file_check(audio_filename, 3) != 0) {
		set_fs(oldfs);
		return -1;
	}
	set_fs(oldfs);
	return 0;
}

int audio_log_init(void)
{
	mutex_init(&audio_log_lock);
	if (init_buff == NULL) {
		init_buff = kzalloc(1024 * 512, GFP_KERNEL);
		buffsize = 0;
	}
	audio_log_init_check();
	return 0;
}
EXPORT_SYMBOL(audio_log_init);

int audio_log_exit(void)
{
	mutex_destroy(&audio_log_lock);
	if (init_buff != NULL) {
		kfree(init_buff);
		init_buff = NULL;
	}
	return 0;
}
EXPORT_SYMBOL(audio_log_exit);

static void audio_log_write(const char* file, const char *buf, int size)
{
	mm_segment_t oldfs;
	oldfs = get_fs();
	set_fs(KERNEL_DS);
	struct file *fp = filp_open(file, O_RDWR | O_CREAT, 0644);
	if (!IS_ERR(fp)) {
		if (size > 0) {
			vfs_llseek(fp, 0, SEEK_END);
			vfs_write(fp, buf, size, &(fp->f_pos));
		}
		vfs_fsync(fp, 0);
		filp_close(fp, NULL);
	} else {
		(void)audio_log_init_check();
	}
	set_fs(oldfs);
}

int audio_log_printk(const char *fmt, ...)
{
	struct tm tm;
	struct timespec64 now;
	getnstimeofday64(&now);
	time64_to_tm(now.tv_sec, 8 * 3600, &tm);
	ktime_get_boottime_ts64(&now);

	char buff[AUDIO_LOG_BUF_SIZE] = {0};
	int len = snprintf_s(&buff[0], AUDIO_LOG_BUF_SIZE, (AUDIO_LOG_BUF_SIZE / 8),
		"[%d-%d %d:%d:%d.%04ld (%lld)] ",
		tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec,
		(long)(now.tv_nsec/100000), (long long)now.tv_sec);
	va_list args;
	va_start(args, fmt);
	len += vsnprintf_s(&buff[len], AUDIO_LOG_BUF_SIZE - len,
		AUDIO_LOG_BUF_SIZE - len - 1, fmt, args);
	va_end(args);

	mutex_lock(&audio_log_lock);
	if ((init_state == false) && (init_buff != NULL)) {
		if (((now.tv_sec <= 25) || ((now.tv_sec <= 50) && (audio_log_init_check() < 0))) &&
			(buffsize + len <= 1024 * 512)) {
			if (memcpy_s(&init_buff[buffsize], len, buff, len) == EOK) {
				buffsize += len;
				mutex_unlock(&audio_log_lock);
				return 0;
			}
		}
		audio_log_write(audio_filename[file_select], init_buff, buffsize);
		init_state = true;
		kfree(init_buff);
		init_buff = NULL;
	}
	mutex_unlock(&audio_log_lock);
	audio_log_write(audio_filename[file_select], buff, len);
	return 0;
}
EXPORT_SYMBOL(audio_log_printk);
