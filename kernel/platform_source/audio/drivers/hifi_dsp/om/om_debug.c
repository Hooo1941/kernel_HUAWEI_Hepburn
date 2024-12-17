/*
 * om_debug.c
 *
 * debug for socdsp
 *
 * Copyright (c) 2013-2020 Huawei Technologies Co., Ltd.
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

#include "om_debug.h"

#include <linux/proc_fs.h>
#include <linux/kthread.h>
#include <linux/semaphore.h>
#include <linux/syscalls.h>
#include <linux/slab.h>
#include <linux/time.h>
#include <linux/rtc.h>
#include <platform_include/basicplatform/linux/rdr_pub.h>
#ifdef CONFIG_VENDOR_FS
#include <platform_include/basicplatform/linux/fs/vendor_fs_interface.h>
#endif

#include "audio_log.h"
#include "audio_hifi.h"
#include "dsp_misc.h"
#include "drv_mailbox_msg.h"
#include "platform_include/basicplatform/linux/ipc_rproc.h"
#include "dsp_om.h"
#include "platform_base_addr_info.h"
#include "audio_common_msg.h"

/*lint -e773 */
#define HI_DECLARE_SEMAPHORE(name) \
	struct semaphore name = __SEMAPHORE_INITIALIZER(name, 0)
/*lint +e773 */
HI_DECLARE_SEMAPHORE(socdsp_log_sema);

#define SOCDSP_DUMP_FILE_NAME_MAX_LEN   256
#define MAX_LEVEL_STR_LEN               32
#define UNCONFIRM_ADDR                  0
#define ROOT_UID                        0
#define SYSTEM_GID                      1000
#define SOCDSP_OM_DIR_LIMIT             0750
#define SOCDSP_OM_FILE_LIMIT            0640
#define SOCDSP_SEC_MAX_NUM              100
#define MAX_NAME_LEN 256

#define HIFI_LOG_PATH_LEN 128

#define FILE_NAME_DUMP_DSP_LOG          "hifi.log"
#define FILE_NAME_DUMP_DSP_BIN          "hifi.bin"
#define FILE_NAME_DUMP_DSP_PANIC_LOG    "hifi_panic.log"
#define FILE_NAME_DUMP_DSP_PANIC_BIN    "hifi_panic.bin"

#define SOCDSP_DEBUG_PATH                  "hifidebug"
#define SOCDSP_DEBUG_LEVEL_PROC_FILE       "debuglevel"
#define SOCDSP_DEBUG_DSPDUMPLOG_PROC_FILE  "dspdumplog"
#define SOCDSP_DEBUG_FAULTINJECT_PROC_FILE "dspfaultinject"
#define SOCDSP_DEBUG_MISCPROC_PROC_FILE    "miscproc"
#define FAULT_INJECT_CMD_STR_MAX_LEN       200
#define MISC_PROC_OPTION_LEN               256
#define SOCDSP_TIME_STAMP_1S               32768
#define SOCDSP_DUMPLOG_TIMESPAN            (10 * SOCDSP_TIME_STAMP_1S)


#define SOCDSP_BTSNOOP_HEADER_DATA "btsnoop\0\0\0\0\1\0\0\x3\xea"
#define BTSNOOP_EPOCH_DELTA 0x00dcddb30f2f8000ULL
#define BTSNOOP_DATA_MAX_LEN 0x3c00 /* 0x3c00 = 48k4ch32bit */
#define SECS_TO_US 1000000

extern void *memcpy64(void *dst, const void *src, unsigned int len);
extern void *memcpy128(void *dst, const void *src, unsigned int len);

enum drv_socdsp_image_sec_load {
	DRV_SOCDSP_IMAGE_SEC_LOAD_STATIC,
	DRV_SOCDSP_IMAGE_SEC_LOAD_DYNAMIC,
	DRV_SOCDSP_IMAGE_SEC_UNLOAD,
	DRV_SOCDSP_IMAGE_SEC_UNINIT,
	DRV_SOCDSP_IMAGE_SEC_LOAD_BUTT
};

enum drv_socdsp_image_sec_type {
	DRV_SOCDSP_IMAGE_SEC_TYPE_CODE,
	DRV_SOCDSP_IMAGE_SEC_TYPE_DATA,
	DRV_SOCDSP_IMAGE_SEC_TYPE_BSS,
	DRV_SOCDSP_IMAGE_SEC_TYPE_BUTT
};

enum dump_dsp_type {
	DUMP_DSP_LOG,
	DUMP_DSP_BIN
};

enum dsp_error_type {
	DSP_NORMAL,
	DSP_PANIC,
	DSP_LOG_BUF_FULL
};

struct socdsp_dump_info {
	enum dsp_error_type error_type;
	enum dump_dsp_type dump_type;
	char *file_name;
	char *data_addr;
	unsigned int data_len;
};

enum audio_hook_msg_type {
	AUDIO_HOOK_MSG_START,
	AUDIO_HOOK_MSG_STOP,
	AUDIO_HOOK_MSG_DATA_TRANS,
	AUDIO_HOOK_BTSNOOP_DATA_TRANS,
	AUDIO_HOOK_MSG_BUTT
};

struct image_partition_table {
	unsigned long phy_addr_start;
	unsigned long phy_addr_end;
	unsigned int size;
	unsigned long remap_addr;
};

struct socdsp_data_hook_ctrl_info {
	char file_path[SOCDSP_DUMP_FILE_NAME_MAX_LEN];
	unsigned int hook_data_enable;
	unsigned char *priv_data_base;
};

struct debug_level_com {
	char level_char;
	unsigned int level_num;
};

struct socdsp_om_send_hook_to_ap {
	unsigned int msg_type;
	unsigned int data_pos;
	unsigned int data_addr;
	unsigned int data_length;
};

struct om_priv_debug {
	unsigned int pre_dsp_dump_timestamp;
	unsigned int pre_exception_no;
	unsigned int *dsp_exception_no;
	unsigned int *dsp_panic_mark;
	unsigned int *dsp_log_cur_addr;
	char *dsp_log_addr;
	char *dsp_bin_addr;
	char cur_dump_time[SOCDSP_DUMP_FILE_NAME_MAX_LEN];
	bool first_dump_log;
	bool force_dump_log;
	enum dsp_error_type dsp_error_type;

	bool reset_system;
	unsigned int dsp_loaded_sign;
	unsigned int *dsp_debug_kill_addr;
	unsigned int *dsp_stack_addr;

	struct drv_fama_config *dsp_fama_config;
	struct task_struct *kdumpdsp_task;
	struct semaphore dsp_dump_sema;
	struct socdsp_om_work_info hook_wq;
	bool is_inited;
	bool om_hvs_dsp_disable;
};

struct bt_snoop_header {
	uint32_t length_original;
	uint32_t length_captured;
	uint32_t flags;
	uint32_t dropped_packets;
	uint64_t timestamp;
};

static struct om_priv_debug g_priv_debug;

static struct socdsp_dump_info g_dsp_dump_info[DUMP_INDEX_MAX] = {
	{ DSP_NORMAL, DUMP_DSP_LOG, FILE_NAME_DUMP_DSP_LOG, UNCONFIRM_ADDR,
		(DRV_DSP_UART_TO_MEM_SIZE - DRV_DSP_UART_TO_MEM_RESERVE_SIZE) },
	{ DSP_NORMAL, DUMP_DSP_BIN, FILE_NAME_DUMP_DSP_BIN, UNCONFIRM_ADDR,
		DSP_DUMP_BIN_SIZE },
	{ DSP_PANIC, DUMP_DSP_LOG, FILE_NAME_DUMP_DSP_PANIC_LOG, UNCONFIRM_ADDR,
		(DRV_DSP_UART_TO_MEM_SIZE - DRV_DSP_UART_TO_MEM_RESERVE_SIZE) },
	{ DSP_PANIC, DUMP_DSP_BIN, FILE_NAME_DUMP_DSP_PANIC_BIN, UNCONFIRM_ADDR,
		DSP_DUMP_BIN_SIZE },
};


void get_ap_cur_time(long long *time_s, long *time_us, bool need_time_zone, bool time_type)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5,4,0))
	struct timespec64 time;
#else
	struct timeval time;
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5,4,0))
	if (time_type) {
		ktime_get_real_ts64(&time); /* clock_realtime UTC time */
	} else {
		ktime_get_ts64(&time);      /* clock_monotonic */
	}
#else
	do_gettimeofday(&time);         /* UTC time */
#endif

	if (need_time_zone)
		time.tv_sec -= (long)sys_tz.tz_minuteswest * 60;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5,4,0))
	*time_us = time.tv_nsec / MSEC_PER_SEC;
#else
	*time_us = time.tv_usec;
#endif
	*time_s = time.tv_sec;
}

static void get_local_time_str(char *time_str, unsigned int len, long long time_s, long time_us)
{
	long time_ms;
	struct rtc_time tm;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0))
	rtc_time64_to_tm(time_s, &tm);
#else
	rtc_time_to_tm(time_s, &tm);
#endif

	if ((time_str == NULL) || (len == 0)) {
		loge("data is null or len is invaled, len: %u\n", len);
		return;
	}

	time_ms = time_us / 1000;

	snprintf(time_str, len, "%04d%02d%02d%02d%02d%02d%03ld",
		1900 + tm.tm_year, 1 + tm.tm_mon, tm.tm_mday,
		tm.tm_hour, tm.tm_min, tm.tm_sec, time_ms);
}

static int socdsp_chown(struct file *file, uid_t user, gid_t group)
{
	int ret;
	mm_segment_t old_fs;

	if (IS_ERR_OR_NULL(file)) {
		loge("socdsp chown failed, param is null");
		return -EPERM;
	}
	old_fs = get_fs();
	set_fs(KERNEL_DS); /*lint !e501 */

	ret = vfs_fchown(file, user, group);
	if (ret != 0)
		loge("uid: %d gid: %d failed error %d\n",
			user, group, ret);

	set_fs(old_fs);

	return ret;
}

static int socdsp_create_dir(char *path)
{
	int ret;
	mm_segment_t old_fs;
	struct file *file = NULL;

	if (!path) {
		loge("path %pK is invailed\n", path);
		return -EPERM;
	}

	old_fs = get_fs();
	set_fs(KERNEL_DS); /*lint !e501 */

#ifdef CONFIG_VENDOR_FS
	ret = (int)vendor_access(path, 0);
#else
	ret = ksys_access(path, 0);
#endif
	if (ret != 0) {
		logi("need create dir %s\n", path);
#ifdef CONFIG_VENDOR_FS
		ret = (int)vendor_mkdir(path, SOCDSP_OM_DIR_LIMIT);
#else
		ret = ksys_mkdir(path, SOCDSP_OM_DIR_LIMIT);
#endif
		if (ret < 0) {
			set_fs(old_fs);
			loge("create dir %s fail, ret: %d\n", path, ret);
			return ret;
		}

		file = filp_open(path, O_RDONLY, SOCDSP_OM_FILE_LIMIT);
		if (IS_ERR_OR_NULL(file)) {
			loge("open dir %s fail\n", path);
			set_fs(old_fs);
			return -EPERM;
		}

		/* log dir limit root-system */
		if (socdsp_chown(file, ROOT_UID, SYSTEM_GID))
			loge("chown %s failed\n", path);
		filp_close(file, NULL);
	}

	set_fs(old_fs);

	return 0;
}

static int socdsp_om_create_log_dir(const char *path, int size)
{
	char cur_path[SOCDSP_DUMP_FILE_NAME_MAX_LEN];
	int index = 0;

	UNUSED_PARAMETER(size);
	if (!path || (strlen(path) + 1) > SOCDSP_DUMP_FILE_NAME_MAX_LEN) {
		loge("path %pK is invailed\n", path);
		return -EPERM;
	}

#ifdef CONFIG_VENDOR_FS
	if (vendor_access(path, 0) == 0)
#else
	if (ksys_access(path, 0) == 0)
#endif
		return 0;

	memset(cur_path, 0, sizeof(cur_path));

	if (*path != '/')
		return -EPERM;

	cur_path[index] = *path;
	index++;
	path++;

	while (*path != '\0') {
		if (*path == '/') {
			if (socdsp_create_dir(cur_path)) {
				loge("create dir %s failed\n", cur_path);
				return -EPERM;
			}
		}
		cur_path[index] = *path;
		index++;
		path++;
	}

	return 0;
}

static void dump_write_file_head(enum dump_dsp_index index, struct file *fp)
{
	char tmp_buf[64];
	unsigned long tmp_len;
	struct rtc_time cur_tm;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0))
	struct timespec64 now;
#else
	struct timespec now;
#endif
	unsigned int err_no = 0xFFFFFFFF;

	const char *is_panic = "i'm panic\n";
	const char *is_exception = "i'm exception\n";
	const char *not_panic = "i'm ok\n";

	/* write file head */
	if (g_dsp_dump_info[index].dump_type == DUMP_DSP_LOG) {
		/* write dump log time */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0))
		ktime_get_coarse_ts64(&now);
		rtc_time64_to_tm(now.tv_sec, &cur_tm);
#else
		now = current_kernel_time();
		rtc_time_to_tm(now.tv_sec, &cur_tm);
#endif

		memset(tmp_buf, 0, sizeof(tmp_buf));
		tmp_len = snprintf(tmp_buf, sizeof(tmp_buf), "%04d-%02d-%02d %02d:%02d:%02d.\n",
			cur_tm.tm_year + 1900, cur_tm.tm_mon + 1,
			cur_tm.tm_mday, cur_tm.tm_hour,
			cur_tm.tm_min, cur_tm.tm_sec);
		vfs_write(fp, tmp_buf, tmp_len, &fp->f_pos);

		/* write exception no */
		memset(tmp_buf, 0, sizeof(tmp_buf));
		err_no = (unsigned int)(*(g_priv_debug.dsp_exception_no));
		if (err_no != 0xFFFFFFFF)
			tmp_len = snprintf(tmp_buf, sizeof(tmp_buf), "the exception no: %u\n", err_no);
		else
			tmp_len = snprintf(tmp_buf, sizeof(tmp_buf), "%s", "socdsp is fine, just dump log\n");

		vfs_write(fp, tmp_buf, tmp_len, &fp->f_pos);

		/* write error type */
		if (*g_priv_debug.dsp_panic_mark == 0xdeadbeaf)
			vfs_write(fp, is_panic, strlen(is_panic), &fp->f_pos);
		else if (*g_priv_debug.dsp_panic_mark == 0xbeafdead)
			vfs_write(fp, is_exception, strlen(is_exception), &fp->f_pos);
		else
			vfs_write(fp, not_panic, strlen(not_panic), &fp->f_pos);
	}
}

static int dump_write_file_body(enum dump_dsp_index index, struct file *fp)
{
	int write_size;
	char *data_addr = NULL;
	unsigned int data_len = g_dsp_dump_info[index].data_len;

	g_dsp_dump_info[NORMAL_LOG].data_addr =
		g_priv_debug.dsp_log_addr + DRV_DSP_UART_TO_MEM_RESERVE_SIZE;
	g_dsp_dump_info[PANIC_LOG].data_addr =
		g_priv_debug.dsp_log_addr + DRV_DSP_UART_TO_MEM_RESERVE_SIZE;

	if (!g_dsp_dump_info[index].data_addr) {
		loge("dsp log ioremap fail\n");
		return -EPERM;
	}

	data_addr = g_dsp_dump_info[index].data_addr;

	write_size = vfs_write(fp, data_addr, data_len, &fp->f_pos);
	if (write_size < 0)
		loge("write file fail\n");

	logi("write file size: %d\n", write_size);

	return 0;
}

void socdsp_dump_dsp(enum dump_dsp_index index, const char *log_path)
{
	int ret;
	mm_segment_t fs;
	struct file *fp = NULL;
	unsigned int file_flag = O_RDWR;
	struct kstat file_stat;
	char path_name[SOCDSP_DUMP_FILE_NAME_MAX_LEN];

	IN_FUNCTION;

	if (down_interruptible(&g_priv_debug.dsp_dump_sema) < 0) {
		loge("acquire the semaphore error\n");
		return;
	}

	dsp_get_log_signal();
	fs = get_fs();
	set_fs(KERNEL_DS); /*lint !e501 */
	ret = socdsp_om_create_log_dir(log_path, SOCDSP_DUMP_FILE_NAME_MAX_LEN);
	if (ret != 0)
		goto END;

	memset(path_name, 0, SOCDSP_DUMP_FILE_NAME_MAX_LEN);
	snprintf(path_name, SOCDSP_DUMP_FILE_NAME_MAX_LEN, "%s%s",
		log_path, g_dsp_dump_info[index].file_name);
	ret = vfs_stat(path_name, &file_stat);
	if (ret < 0) {
		logi("there isn't a dsp log file:%s, and need to create\n", path_name);
		file_flag |= O_CREAT;
	}
	fp = filp_open(path_name, file_flag, SOCDSP_OM_FILE_LIMIT);
	if (IS_ERR(fp)) {
		loge("open file fail: %s\n", path_name);
		fp = NULL;
		goto END;
	}

	vfs_llseek(fp, 0, SEEK_SET); /* write from file start */
	dump_write_file_head(index, fp);
	ret = dump_write_file_body(index, fp);
	if (ret != 0)
		goto END;

	ret = socdsp_chown(fp, ROOT_UID, SYSTEM_GID);
	if (ret != 0)
		loge("chown %s failed!\n", path_name);
END:
	if (fp)
		filp_close(fp, NULL);
	set_fs(fs);
	dsp_release_log_signal();
	up(&g_priv_debug.dsp_dump_sema);
	OUT_FUNCTION;
}

static void dump_dsp_show_info(unsigned int *socdsp_info_addr)
{
	uintptr_t socdsp_stack_addr;
	unsigned int i;

	socdsp_stack_addr = *(uintptr_t *)(socdsp_info_addr + 4);

	logi("panic addr: 0x%pK, cur pc: 0x%pK, pre pc: 0x%pK, cause: 0x%x\n",
		(void *)(uintptr_t)(*socdsp_info_addr),
		(void *)(uintptr_t)(*(socdsp_info_addr + 1)),
		(void *)(uintptr_t)(*(socdsp_info_addr + 2)),
		*(unsigned int *)(uintptr_t)(socdsp_info_addr + 3));
	for (i = 0; i < (DRV_DSP_STACK_TO_MEM_SIZE / 2) / sizeof(int) / 4; i += 4)
		logi("0x%pK: 0x%pK 0x%pK 0x%pK 0x%pK\n",
			(void *)(socdsp_stack_addr + i * 4),
			(void *)(uintptr_t)(*(socdsp_info_addr + i)),
			(void *)(uintptr_t)(*(socdsp_info_addr + i + 1)),
			(void *)(uintptr_t)(*(socdsp_info_addr + i + 2)),
			(void *)(uintptr_t)(*(socdsp_info_addr + i + 3)));
}

static int socdsp_dump_thread(void *p)
{
	unsigned int exception_no;
	unsigned int time_now;
	unsigned int time_diff;
	long time_us;
	long long time_s;
	unsigned int *socdsp_info_addr = NULL;
	char log_path[HIFI_LOG_PATH_LEN] = {0};

	snprintf(log_path, sizeof(log_path) - 1, "%s%s", PATH_ROOT, "running_trace/hifi_log/");

	IN_FUNCTION;

	while (!kthread_should_stop()) {
		if (down_interruptible(&socdsp_log_sema) != 0)
			loge("socdsp dump dsp thread wake up err\n");
#ifdef CONFIG_DFX_BB
		if (bbox_check_edition() != EDITION_INTERNAL_BETA) {
			loge("not beta, do not dump socdsp\n");
			continue;
		}
#endif
		time_now = om_get_dsp_timestamp();
		time_diff = time_now - g_priv_debug.pre_dsp_dump_timestamp;
		g_priv_debug.pre_dsp_dump_timestamp = time_now;
		socdsp_info_addr = g_priv_debug.dsp_stack_addr;
		exception_no = *(unsigned int *)(socdsp_info_addr + 3);

		logi("errno: %x pre errno: %x is first: %d is force: %d time diff: %d ms\n",
			exception_no, g_priv_debug.pre_exception_no, g_priv_debug.first_dump_log,
			g_priv_debug.force_dump_log, (time_diff * 1000) / SOCDSP_TIME_STAMP_1S);

		get_ap_cur_time(&time_s, &time_us, true, true);
		get_local_time_str(g_priv_debug.cur_dump_time, SOCDSP_DUMP_FILE_NAME_MAX_LEN,
			time_s, time_us);
		if ((exception_no < 40) && (exception_no != g_priv_debug.pre_exception_no)) {
			dump_dsp_show_info(socdsp_info_addr);
			socdsp_dump_dsp(PANIC_LOG, log_path);
			socdsp_dump_dsp(PANIC_BIN, log_path);
			g_priv_debug.pre_exception_no = exception_no;
		} else if (g_priv_debug.first_dump_log || g_priv_debug.force_dump_log ||
				(time_diff > SOCDSP_DUMPLOG_TIMESPAN)) {
			socdsp_dump_dsp(NORMAL_LOG, log_path);
			/* needn't dump bin when socdsp log buffer full */
			if (g_priv_debug.dsp_error_type != DSP_LOG_BUF_FULL)
				socdsp_dump_dsp(NORMAL_BIN, log_path);
			g_priv_debug.first_dump_log = false;
		}
	}
	OUT_FUNCTION;

	return 0;
}

void socdsp_dump_panic_log(void)
{
	if (!is_dsp_img_loaded())
		return;

	up(&socdsp_log_sema);
}

int socdsp_dump(uintptr_t arg)
{
	unsigned int err_type = 0;

	if (arg == 0) {
		loge("arg is null\n");
		return -EPERM;
	}

	if (try_copy_from_user(&err_type, (void __user *)arg, sizeof(err_type))) {
		loge("copy from user fail, don't dump log\n");
		return -EPERM;
	}
	g_priv_debug.dsp_error_type = err_type;
	up(&socdsp_log_sema);

	return 0;
}


static void set_om_data(const unsigned char *unsec_virt_addr)
{
	g_priv_debug.first_dump_log = true;

	g_priv_debug.dsp_panic_mark = (unsigned int *)(unsec_virt_addr +
		(DRV_DSP_PANIC_MARK - DSP_UNSEC_BASE_ADDR));
	g_priv_debug.dsp_bin_addr = (char *)(unsec_virt_addr +
		(DSP_DUMP_BIN_ADDR - DSP_UNSEC_BASE_ADDR));
	g_priv_debug.dsp_exception_no = (unsigned int *)(unsec_virt_addr +
		(DRV_DSP_EXCEPTION_NO - DSP_UNSEC_BASE_ADDR));
	g_priv_debug.dsp_log_cur_addr = (unsigned int *)(unsec_virt_addr +
		(DRV_DSP_UART_TO_MEM_CUR_ADDR - DSP_UNSEC_BASE_ADDR));
	g_priv_debug.dsp_log_addr = (char *)(unsec_virt_addr +
		(DRV_DSP_UART_TO_MEM - DSP_UNSEC_BASE_ADDR));
	memset(g_priv_debug.dsp_log_addr, 0, DRV_DSP_UART_TO_MEM_SIZE);
	*g_priv_debug.dsp_log_cur_addr = DRV_DSP_UART_TO_MEM_RESERVE_SIZE;
	g_priv_debug.dsp_debug_kill_addr = (unsigned int *)(unsec_virt_addr +
		(DRV_DSP_KILLME_ADDR - DSP_UNSEC_BASE_ADDR));
	g_priv_debug.dsp_fama_config = (struct drv_fama_config *)(unsec_virt_addr +
		(DRV_DSP_ODT_FAMA_CONFIG_ADDR - DSP_UNSEC_BASE_ADDR));
	g_priv_debug.dsp_stack_addr = (unsigned int *)(unsec_virt_addr +
		(DRV_DSP_STACK_TO_MEM - DSP_UNSEC_BASE_ADDR));

	*(g_priv_debug.dsp_exception_no) = ~0;
	g_priv_debug.pre_exception_no = ~0;
	g_priv_debug.dsp_fama_config->head_magic = DRV_DSP_SOCP_FAMA_HEAD_MAGIC;
	g_priv_debug.dsp_fama_config->flag = DRV_DSP_FAMA_OFF;
	g_priv_debug.dsp_fama_config->rear_magic = DRV_DSP_SOCP_FAMA_REAR_MAGIC;

	g_dsp_dump_info[NORMAL_BIN].data_addr = g_priv_debug.dsp_bin_addr;
	g_dsp_dump_info[PANIC_BIN].data_addr  = g_priv_debug.dsp_bin_addr;
}

bool om_get_hvs_dsp_disable_state(void)
{
	return g_priv_debug.om_hvs_dsp_disable;
}

int om_debug_init(unsigned char *unsec_virt_addr)
{
	int ret = 0;

	memset(&g_priv_debug, 0, sizeof(g_priv_debug));

	g_priv_debug.reset_system = false;
	g_priv_debug.is_inited = false;

	set_om_data(unsec_virt_addr);

	sema_init(&g_priv_debug.dsp_dump_sema, 1);

	g_priv_debug.kdumpdsp_task = kthread_create(socdsp_dump_thread, 0, "dspdumplog");
	if (IS_ERR(g_priv_debug.kdumpdsp_task)) {
		loge("creat socdsp dump log thread fail\n");
		return -EPERM;
	} else {
		wake_up_process(g_priv_debug.kdumpdsp_task);
	}

	g_priv_debug.is_inited = true;

	return ret;
}

void om_debug_deinit(void)
{
	if (g_priv_debug.is_inited)
		up(&g_priv_debug.dsp_dump_sema);

	if (g_priv_debug.kdumpdsp_task) {
		kthread_stop(g_priv_debug.kdumpdsp_task);
		g_priv_debug.kdumpdsp_task = NULL;
	}

	g_priv_debug.is_inited = false;

}
