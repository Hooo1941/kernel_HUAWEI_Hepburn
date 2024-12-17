/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: implement the platform interface for boot fail
 */

#include "adapter_nvt.h"

#include <linux/io.h>
#include <linux/mm.h>
#include <linux/reboot.h>
#include <linux/kmsg_dump.h>
#include <linux/slab.h>
#include <linux/syscalls.h>
#include <linux/thread_info.h>
#include <linux/types.h>
#include <linux/uaccess.h>
#include <linux/vmalloc.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/semaphore.h>
#include <linux/kthread.h>

#include <securec.h>
#include <hwbootfail/chipsets/common/adapter_common.h>
#include <hwbootfail/chipsets/common/bootfail_chipsets.h>
#include <hwbootfail/chipsets/common/bootfail_common.h>
#include <hwbootfail/chipsets/bootfail_nvt.h>
#include <hwbootfail/core/boot_interface.h>
#include <linux/blackbox.h>
#include <soc/nvt/nvt_stbc.h>

#define BL_LOG_NAME              "fastboot_log"
#define KERNEL_LOG_NAME          "last_kmsg"
#define BFI_PART_SIZE            0xBE0000 /* 12MB -128 KB */

#define APP_LOG_PATH             "/data/log/hilogs/hiapplogcat-log"
#define APP_LOG1_PATH            APP_LOG_PATH".1"
#define APP_LOG_PATH_MAX_LENGTH  128
#define APP_LOG_NUM              2

#define BOOT_STAGE_OFFSET        0
#define PYHS_MEM_INFO_OFFSET     512
#define BF_META_LOG_OFFSET       (BFI_PART_HDR_SIZE + BF_INFO_SIZE)
#define BF_META_LOG_SIZE         (sizeof(struct bootfail_meta_log))
#define BF_LOG_OFFSET            (BFI_PART_HDR_SIZE + BF_INFO_SIZE + BF_META_LOG_SIZE)

#define WAIT_TIMEOUT             (5 * HZ) /* 10s */
#define FASTBOOT_LOG_PYHMEM_SIZE (0x20000) /* 128K */

static int g_stage = BL2_STAGE;

struct mutex g_lock;
static struct semaphore g_emmc_rw_start;
static struct semaphore g_read_done;
static struct partition_info g_part_infos;

struct every_number_info {
	u64 rtc_time;
	u64 boot_time;
	u64 bootup_keypoint;
	u64 reboot_type;
	u64 exce_subtype;
	u64 fastbootlog_start_addr;
	u64 fastbootlog_size;
	u64 last_kmsg_start_addr;
	u64 last_kmsg_size;
	u64 last_applog_start_addr[APP_LOG_NUM];
	u64 last_applog_size;
};

static int rw_part(const char *dev_path, unsigned long long offset,
		char *buf, unsigned long long buf_size, bool read);

int write_part_infos(int length, int offset, void *data, int data_len)
{
	errno_t ret;
	mutex_lock(&g_lock);
	g_part_infos.length = length;
	g_part_infos.offset = offset;
	(void)memset_s(g_part_infos.path, PATH_MAX_LEN, 0, PATH_MAX_LEN);
	ret = memcpy_s(g_part_infos.path, PATH_MAX_LEN, PHYS_MEM_PATH, strlen(PHYS_MEM_PATH));
	if (ret != EOK) {
		print_err("memcpy_s is failed, ret = %d\n", ret);
		mutex_unlock(&g_lock);
		return BF_WRITE_PART_FAIL;
	}
	(void)memset_s(g_part_infos.data, sizeof(g_part_infos.data), 0, sizeof(g_part_infos.data));
	ret = memcpy_s(g_part_infos.data, sizeof(g_part_infos.data), data, data_len);
	if (ret != EOK) {
		print_err("memcpy_s is failed, ret = %d\n", ret);
		mutex_unlock(&g_lock);
		return BF_WRITE_PART_FAIL;
	}
	g_part_infos.is_write = 1;
	mutex_unlock(&g_lock);
	up(&g_emmc_rw_start);
	return BF_OK;
}

static int nvt_set_boot_stage(int stage)
{
	print_info("set boot stage %x\n", stage);
	g_stage = stage;
	return write_part_infos(sizeof(int), BOOT_STAGE_OFFSET, &stage, sizeof(int));
}

static int nvt_get_boot_stage(int *stage)
{
	if (unlikely(stage == NULL)) {
		print_invalid_params("stage is NULL!\n");
		return BF_INVALID_PARM;
	}
	*stage = g_stage;
	return BF_OK;
}

static void get_boot_stage_ops(struct adapter *padp)
{
	padp->stage_ops.set_stage = nvt_set_boot_stage;
	padp->stage_ops.get_stage = nvt_get_boot_stage;
}

static void nvt_shutdown(void)
{
	print_info("unsupported\n");
}

static void nvt_reboot(void)
{
	print_info("nvt_reboot enter!\n");
	char error_desc[ERROR_DESC_MAX_LEN];

	/* notify blackbox to do dump */
	kmsg_dump(KMSG_DUMP_BOOTFAIL);
	(void)memset_s(error_desc, sizeof(error_desc), 0, sizeof(error_desc));
	dump_stacktrace((char *)error_desc, ERROR_DESC_MAX_LEN, false);

	print_info("bbox_task_panic exit!\n");

	write_part_infos(sizeof(int), BOOT_STAGE_OFFSET, &g_stage, sizeof(g_stage));

	bbox_notify_error(EVENT_BOOTFAIL, MODULE_SYSTEM, error_desc, 1);
}

static void get_sysctl_ops(struct adapter *padp)
{
	padp->sys_ctl.reboot = nvt_reboot;
	padp->sys_ctl.shutdown = nvt_shutdown;
}

static int nvt_emmc_reads(struct partition_info *part_infos)
{
	return rw_part(part_infos->path, part_infos->offset, part_infos->data, part_infos->length, true);
}

static int nvt_emmc_writes(struct partition_info *part_infos)
{
	return rw_part(part_infos->path, part_infos->offset, part_infos->data, part_infos->length, false);
}

static void nvt_emmc_rw(void)
{
	mutex_lock(&g_lock);
	if (g_part_infos.is_write) {
		(void)nvt_emmc_writes(&g_part_infos);
		mutex_unlock(&g_lock);
	} else {
		(void)nvt_emmc_reads(&g_part_infos);
		mutex_unlock(&g_lock);
		up(&g_read_done);
	}
}

static int nvt_read_from_phys_mem(unsigned long dst, unsigned long dst_max,
				void *phys_mem_addr, unsigned long data_len)
{
	if (phys_mem_addr == NULL || dst == 0 || dst_max == 0 || data_len == 0) {
		print_invalid_params("bootfail: dst: %u, dst_max: %u, data_len: %u\n",
			dst, dst_max, data_len);
		return BF_INVALID_PARM;
	}
	size_t bytes_to_read = min(dst_max, data_len);
	mutex_lock(&g_lock);
	g_part_infos.length = bytes_to_read;
	g_part_infos.offset = PYHS_MEM_INFO_OFFSET;
	(void)memset_s(g_part_infos.path, PATH_MAX_LEN, 0, PATH_MAX_LEN);
	errno_t rc = memcpy_s(g_part_infos.path, PATH_MAX_LEN, PHYS_MEM_PATH, strlen(PHYS_MEM_PATH));
	if (rc != EOK) {
		print_err("memcpy_s is failed, rc = %d\n", rc);
		mutex_unlock(&g_lock);
		return BF_READ_PART_FAIL;
	}
	(void)memset_s(g_part_infos.data, sizeof(g_part_infos.data), 0, sizeof(g_part_infos.data));
	g_part_infos.is_write = 0;
	mutex_unlock(&g_lock);
	up(&g_emmc_rw_start);
	if (down_timeout(&g_read_done, WAIT_TIMEOUT)) {
		print_err("nvt_read_from_phys_mem down_timeout\n");
		return BF_OK;
	}

	mutex_lock(&g_lock);
	rc = memcpy_s((char *)(uintptr_t)dst, bytes_to_read, g_part_infos.data, g_part_infos.length);
	mutex_unlock(&g_lock);
	if (rc != EOK) {
		print_err("memcpy_s is failed, rc = %d\n", rc);
		return BF_READ_PART_FAIL;
	}
	return BF_OK;
}

static int nvt_write_to_phys_mem(unsigned long dst, unsigned long dst_max,
				void *src, unsigned long src_len)
{
	unsigned long bytes_to_write;

	if (src == NULL || dst == 0 || dst_max == 0 || src_len == 0) {
		print_invalid_params("bootfail: dst: %u, dst_max: %u, src_len: %u\n",
			dst, dst_max, src_len);
		return BF_INVALID_PARM;
	}
	bytes_to_write = min(dst_max, src_len);

	return write_part_infos(bytes_to_write, PYHS_MEM_INFO_OFFSET, src, bytes_to_write);
}

static void get_phys_mem_info(struct adapter *padp)
{
	padp->pyhs_mem_info.base = PYHS_MEM_INFO_OFFSET;
	padp->pyhs_mem_info.size = BF_SIZE_1K;

	padp->pyhs_mem_info.ops.read = nvt_read_from_phys_mem;
	padp->pyhs_mem_info.ops.write = nvt_write_to_phys_mem;
}

static long full_rw_file(struct file *fp, char *buf,
			size_t buf_size, bool read, loff_t pos)
{
	mm_segment_t old_fs;
	long bytes_total_to_rw = (long)buf_size;
	long bytes_total_rw = 0L;
	long bytes_this_time = 0L;
	char *ptemp = buf;

	while (bytes_total_to_rw > 0) {
		bytes_this_time = read ?
			vfs_read(fp, ptemp, bytes_total_to_rw, &pos) :
			vfs_write(fp, ptemp, bytes_total_to_rw, &pos);
		if (read ? (bytes_this_time <= 0) : (bytes_this_time < 0)) {
			print_err("bf_sys_read or bf_sys_write failed!\n");
			break;
		}
		ptemp += bytes_this_time;
		bytes_total_to_rw -= bytes_this_time;
		bytes_total_rw += bytes_this_time;
	}
	return bytes_total_rw;
}

static int rw_part(const char *dev_path, unsigned long long offset,
		char *buf, unsigned long long buf_size, bool read)
{
	struct file *fp = NULL;
	int ret = -1;
	long bytes_total;
	loff_t seek_result;
	mm_segment_t fs;

	if (dev_path == NULL || buf == NULL)
		return BF_INVALID_PARM;

	fs = get_fs();
	set_fs(KERNEL_DS); //lint !e501
	fp = filp_open(dev_path, read ? O_RDONLY : O_WRONLY, 0);
	if (IS_ERR(fp)) {
		print_err("Open file [%s] failed!\n", dev_path);
		goto __out;
	}

	bytes_total = full_rw_file(fp, buf, buf_size, read, offset);
	if ((long long)buf_size != bytes_total) {
		print_err("rw [%s] failed!, result: %ld, it should be: %lld\n",
			dev_path, bytes_total, (long long)buf_size);
		goto __out;
	} else {
		ret = 0;
	}

__out:
	if (!IS_ERR(fp)) {
		vfs_fsync(fp, 0);
		filp_close(fp, NULL);
	}

	set_fs(fs);

	return ret;
}

static int nvt_read_part(const char *dev_path, unsigned long long offset,
			char *buf, unsigned long long buf_size)
{
	return rw_part(dev_path, offset, buf, buf_size, true);
}

static int nvt_write_part(const char *dev_path, unsigned long long offset,
			const char *buf, unsigned long long buf_size)
{
	return rw_part(dev_path, offset, (char *)buf, buf_size, false);
}

static void get_raw_part_info(struct adapter *padp)
{
	padp->bfi_part.dev_path = BFI_DEV_PATH;
	padp->bfi_part.part_size = BFI_PART_SIZE;
	padp->bfi_part.ops.read = nvt_read_part;
	padp->bfi_part.ops.write = nvt_write_part;
}

static void capture_kernel_log(char *pbuf, unsigned int buf_size)
{
	char *kbuf_addr = NULL;
	unsigned int kbuf_size = log_buf_len_get();
	errno_t ret;

	if (pbuf == NULL) {
		print_invalid_params("pbuf is null\n");
		return;
	}
	kbuf_addr = log_buf_addr_get();
	if (kbuf_addr == NULL || buf_size != log_buf_len_get()) {
		print_invalid_params("kbuf_addr or buf_size err\n");
		return;
	}
	ret = memcpy_s(pbuf, buf_size,
		kbuf_addr, min(kbuf_size, buf_size));
	if (ret != EOK)
		print_err("memcpy_s failed, ret: %d\n", ret);
}

static void capture_bl_log(char *buf, unsigned int buf_size)
{
	if (buf == NULL || buf_size == 0) {
		print_err("get bl log params err\n");
		return;
	}

	char fastboot_log[] = "kernel can not get information of fastboot_log!";
	if (memcpy_s(buf, buf_size, fastboot_log, strlen(fastboot_log)) != EOK)
		print_err("capture_bl_log memcpy_s buf failed!\n");
}

static void get_log_ops_info(struct adapter *padp)
{
	errno_t ret = strncpy_s(padp->bl_log_ops.log_name,
		sizeof(padp->bl_log_ops.log_name), BL_LOG_NAME,
		min(strlen(BL_LOG_NAME), sizeof(padp->bl_log_ops.log_name) - 1));
	if (ret != EOK)
		print_err("BL_LOG_NAME strncpy_s failed\n");
	padp->bl_log_ops.log_size = (unsigned int)FASTBOOT_LOG_PYHMEM_SIZE;
	padp->bl_log_ops.capture_bl_log = capture_bl_log;

	ret = strncpy_s(padp->kernel_log_ops.log_name,
		sizeof(padp->kernel_log_ops.log_name), KERNEL_LOG_NAME,
		min(strlen(KERNEL_LOG_NAME),
			sizeof(padp->kernel_log_ops.log_name) - 1));
	if (ret != EOK)
		print_err("KERNEL_LOG_NAME strncpy_s failed\n");
	padp->kernel_log_ops.log_size = log_buf_len_get();
	padp->kernel_log_ops.capture_kernel_log = capture_kernel_log;
}

static void degrade(int excp_type)
{
	print_info("unsupported\n");
}

static void bypass(int excp_type)
{
	print_info("unsupported\n");
}

static void load_backup(const char *part_name)
{
	print_info("unsupported\n");
}

static void notify_storage_fault(unsigned long long bopd_mode)
{
	print_info("unsupported\n");
}

static void platform_adapter_init(struct adapter *padp)
{
	if (padp == NULL) {
		print_invalid_params("padp is NULL\n");
		return;
	}

	get_raw_part_info(padp);
	get_phys_mem_info(padp);
	get_log_ops_info(padp);
	get_sysctl_ops(padp);
	get_boot_stage_ops(padp);
	padp->prevent.degrade = degrade;
	padp->prevent.bypass = bypass;
	padp->prevent.load_backup = load_backup;
	padp->notify_storage_fault = notify_storage_fault;
}

static struct reboot_cmd {
	const char *reboot_command;
	ker_stbc_ipc_reboot_cmd reboot_value;
};

struct reboot_cmd g_reboot_cmd_table[] = {
	{ "erecovery", EN_POWER_ON_EVENT_RESERVED_FOR_REBOOT_ERECOVERY },
	{ "recovery", EN_POWER_ON_EVENT_RESERVED_FOR_REBOOT_RECOVERY },
	{ "dm-verity device corrupted", EN_POWER_ON_EVENT_RESERVED_FOR_DM_VERITY_FAIL },
	{ "shell", EN_POWER_ON_EVENT_RESERVED_FOR_REBOOT_SHELL },
	{ "bootloader", EN_POWER_ON_EVENT_RESERVED_FOR_REBOOT_BOOTLOADER },
	{ "RescueParty", EN_POWER_ON_EVENT_RESERVED_FOR_REBOOT_RESCUEPARTY },
	{ "apexd-failed", EN_POWER_ON_EVENT_RESERVED_FOR_REBOOT_APEXD_FAILED },
};

static int reboot_notify(struct notifier_block *this, unsigned long code, void *buf)
{
	if (buf == NULL || strlen(buf) == 0)
		return 0;

	int reboot_table_len = sizeof(g_reboot_cmd_table) / sizeof(g_reboot_cmd_table[0]);
	for (int i = 0; i < reboot_table_len; i++) {
		if (strncmp(g_reboot_cmd_table[i].reboot_command, buf, strlen(buf)) == EOK) {
			int ipc_ret;
			int ret = ipc_send_and_wait_reply(IPC_CMD_REPLY_ACK,
				0xEC, g_reboot_cmd_table[i].reboot_value, 0x0, 0x0, 0xEC, &ipc_ret);
			if (ret)
				printk("write stbc reboot reason fail\n");
			break;
		}
	}
	return 0;
}

static struct notifier_block g_reboot_notifier = {
	.notifier_call = reboot_notify,
};

int nvt_adapter_init(struct adapter *padp)
{
	if (common_adapter_init(padp) != 0) {
		print_err("init adapter common failed\n");
		return BF_NOT_INIT_SUCC;
	}
	sema_init(&g_emmc_rw_start, 0);
	sema_init(&g_read_done, 0);
	mutex_init(&g_lock);

	platform_adapter_init(padp);

	int ret = register_reboot_notifier(&g_reboot_notifier);
	if (ret)
		printk("can't register reboot notifier (err=%d)\n", ret);
	return BF_OK;
}

int bootfail_rw_thread(void *data)
{
	while (!kthread_should_stop()) {
		down(&g_emmc_rw_start);
		nvt_emmc_rw();
	}
	return BF_OK;
}