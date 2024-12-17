/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: implement the platform interface for boot fail
 * Author: qidechun
 * Create: 2019-03-05
 */

/* ----includes ---- */
#include "adapter_hisi.h"
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <mntn_public_interface.h>
#include <platform_include/basicplatform/linux/dfx_bootup_keypoint.h>
#include <linux/hisi/rdr_pub.h>
#include <mntn_public_interface.h>
#include <hwbootfail/core/boot_interface.h>
#include <hwbootfail/chipsets/common/bootfail_common.h>
#include <hwbootfail/chipsets/common/adapter_common.h>
#include <hwbootfail/chipsets/common/bootfail_chipsets.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
#include <linux/vmalloc.h>
#endif
#include <securec.h>

/* ----local macroes ---- */
#define BL_LOG_NAME "fastboot_log"
#define KERNEL_LOG_NAME "last_kmsg"
#ifdef CONFIG_SHUT_DETECTOR
#define HISI_SHUT_STAGE_DEV_PATH "/dev/block/by-name/bootfail_info"
#define HISI_SHUT_STAGE_OFFSET 0x17FDFC /* 2MB - 512KB - 512B - 4B */

static int g_shut_stage;
#endif

/* ---- local prototypes ---- */
enum boot_detector_modid {
	MODID_BOOT_DETECTOR_START = DFX_BB_MOD_BFM_START,
	MODID_BOOT_FAIL,
	MODID_BOOT_DETECTOR_END = DFX_BB_MOD_BFM_END
};

/* ---- local static variables ---- */
struct rdr_exception_info_s boot_detector_excp_info[] = {
	{
		{0, 0}, (u32)DFX_BB_MOD_BFM_START, (u32)DFX_BB_MOD_BFM_END,
		RDR_ERR, RDR_REBOOT_NOW, RDR_AP | RDR_BFM, RDR_AP, RDR_AP,
		(u32)RDR_REENTRANT_DISALLOW, (u32)BFM_S_NATIVE_DATA_FAIL,
		0, (u32)RDR_UPLOAD_YES, "bfm", "bfm-reboot", 0, 0, 0
	},
};

/* ---- local function definitions --- */
static unsigned int stage_to_kp(enum boot_stage stage)
{
	if (stage >= BL1_STAGE_START && stage <= BL1_STAGE_END)
		return STAGE_XLOADER_START;
	else if (stage >= BL2_STAGE_START && stage <= BL2_STAGE_END)
		return STAGE_FASTBOOT_START;
	else if (stage >= KERNEL_STAGE_START && stage <= KERNEL_STAGE_END)
		return STAGE_KERNEL_BOOTANIM_COMPLETE;
	else if (stage >= NATIVE_STAGE_START && stage <= NATIVE_STAGE_END)
		return STAGE_INIT_INIT_START;
	else if (stage >= FRAMEWORK_STAGE_START && stage < BOOT_SUCC_STAGE)
		return STAGE_ANDROID_ZYGOTE_START;

	return STAGE_BOOTUP_END;
}

static enum boot_stage kp_to_stage(unsigned int kp)
{
	if (kp >= STAGE_XLOADER_START && kp <= STAGE_XLOADER_END)
		return BL1_STAGE;
	else if (kp >= STAGE_FASTBOOT_START && kp <= STAGE_FASTBOOT_END)
		return BL2_STAGE;
	else if ((kp >= STAGE_KERNEL_EARLY_INITCALL && kp < STAGE_INIT_INIT_START) ||
		 (kp >= STAGE_HMKERNEL_START && kp <= STAGE_HMKERNEL_END))
		return KERNEL_STAGE;
	else if (kp >= STAGE_INIT_INIT_START &&
		kp < STAGE_ANDROID_ZYGOTE_START)
		return NATIVE_STAGE;
	else if (kp >= STAGE_ANDROID_ZYGOTE_START &&
		kp < STAGE_BOOTUP_END)
		return FRAMEWORK_STAGE;
	else if (kp >= STAGE_BOOTUP_END)
		return BOOT_SUCC_STAGE;

	return INVALID_STAGE;
}

#ifdef CONFIG_SHUT_DETECTOR
static void hisi_set_shut_stage_to_disk(struct work_struct *work)
{
	print_info("write shut stage to bootfail_info. stage: %x\n", g_shut_stage);
	int ret;

	ret = write_part(HISI_SHUT_STAGE_DEV_PATH, HISI_SHUT_STAGE_OFFSET,
		(char *)&g_shut_stage, sizeof(g_shut_stage));
	if (ret != 0)
		print_err("write shut stage to bootfail_info failed!\n");
}

static DECLARE_WORK(hisi_set_shut_stage_work, &hisi_set_shut_stage_to_disk);

int hisi_set_shut_stage(int stage)
{
	g_shut_stage = stage;
	schedule_work(&hisi_set_shut_stage_work);
	return 0;
}

int hisi_get_shut_stage_from_disk(int *stage)
{
	int ret;

	if (unlikely(stage == NULL)) {
		print_invalid_params("stage is NULL!\n");
		return -1;
	}
	ret = read_part(HISI_SHUT_STAGE_DEV_PATH, HISI_SHUT_STAGE_OFFSET,
		(char *)stage, sizeof(*stage));
	if (ret != 0) {
		print_err("read stage from bootfail_info failed!\n");
		return -1;
	}
	print_info("read shut stage from bootfail_info. stage: %x\n", *stage);
	return 0;
}

int hisi_get_shut_stage(int *stage)
{
	if (g_shut_stage == 0)
		return hisi_get_shut_stage_from_disk(stage);
	*stage = g_shut_stage;
	return 0;
	print_info("read shut stage from g_shut_stage. stage: %x", *stage);
}
#endif

int hisi_set_boot_stage(int stage)
{
	unsigned int kp = stage_to_kp((enum boot_stage)stage);

	set_boot_keypoint(kp);

	return 0;
}

int hisi_get_boot_stage(int *stage)
{
	unsigned int kp;

	if (unlikely(stage == NULL)) {
		print_invalid_params("stage is NULL!\n");
		return -1;
	}

	kp = get_boot_keypoint();
	*stage = kp_to_stage(kp);

	return 0;
}

static void dump_callback(u32 modid,
	u32 etype, u64 coreid,	char *log_path, pfn_cb_dump_done pfn_cb)
{
	print_enter();
	print_exit();
}

static void reset_callback(u32 modid, u32 etype, u64 coreid)
{
	print_enter();
	print_exit();
}

static int register_callbacks_to_rdr(void)
{
	struct rdr_module_ops_pub soc_ops;
	struct rdr_register_module_result retinfo;
	int ret;

	soc_ops.ops_dump = dump_callback;
	soc_ops.ops_reset = reset_callback;
	ret = rdr_register_module_ops(RDR_BFM, &soc_ops, &retinfo);
	if (ret != 0)
		print_err("rdr_register_module_ops failed!ret = [%d]\n", ret);

	return ret;
}

static void register_exceptions_to_rdr(void)
{
	unsigned int i;
	u32 ret;

	for (i = 0; i < array_size(boot_detector_excp_info); i++) {
		ret = rdr_register_exception(&boot_detector_excp_info[i]);
		if (ret == 0)
			print_err("rdr_register_exception failed!, ret: %d\n", ret);
	}
}

static void capture_bl_log(char *pbuf, unsigned int buf_size)
{
	read_from_phys_mem((uintptr_t)pbuf, (unsigned long)buf_size,
		(void *)SUB_RESERVED_FASTBOOT_LOG_PYHMEM_BASE,
		(unsigned long)buf_size);
}

static void *bf_vmap(phys_addr_t paddr, size_t size)
{
	int i;
	void *vaddr = NULL;
	pgprot_t pgprot;
	unsigned long offset;
	int pages_count;
	struct page **pages = NULL;
	size_t pg_size;

	offset = paddr & ~PAGE_MASK;
	paddr &= PAGE_MASK;
	pages_count = PAGE_ALIGN(size + offset) / PAGE_SIZE;
	pg_size = sizeof(struct page *) * pages_count;
	pages = kzalloc(pg_size, GFP_KERNEL);
	if (pages == NULL)
		return NULL;
	pgprot = pgprot_writecombine(PAGE_KERNEL); //lint !e446
	for (i = 0; i < pages_count; i++)
		*(pages + i) = phys_to_page(paddr + PAGE_SIZE * i);
	vaddr = vmap(pages, pages_count, VM_MAP, pgprot);
	kfree(pages);
	if (vaddr == NULL)
		return NULL;

	return offset + (char *)vaddr;
}
#ifdef CONFIG_LIBLINUX
struct klog_buf_s {
	void *pbuf;
	unsigned int buf_size;
};

static int dump_klog(const char *buffer, unsigned int len, uintptr_t arg)
{
	struct klog_buf_s *buf = (struct klog_buf_s *)arg;
	unsigned int copy_len = min(len, buf->buf_size);

	int ret = memcpy_s(buf->pbuf, buf->buf_size, buffer, copy_len);
	if (ret == 0) {
		buf->pbuf += copy_len;
		buf->buf_size -= copy_len;
		/* return 0 as success */
		ret = (buf->buf_size > 0) ? 0 : -1;
	}

	return ret;
}

/* extra text size required after parsing raw klog to text */
#define EXTRA_SIZE_FOR_KMSG 16 * 1024

static void capture_kernel_log(char *pbuf, unsigned int buf_size)
{
	unsigned int dump_size = buf_size;
	struct klog_buf_s buf = {
		.pbuf = pbuf,
		.buf_size = buf_size
	};

	if (dump_size  > EXTRA_SIZE_FOR_KMSG)
		dump_size -= EXTRA_SIZE_FOR_KMSG;

	liblinux_iter_current_klog(dump_size, dump_klog, (uintptr_t)&buf);
}
#else
static void capture_kernel_log(char *pbuf, unsigned int buf_size)
{
	void *pstore_addr = NULL;
	void *last_applog_addr = NULL;
	void *last_kmsg_addr = NULL;

	pstore_addr = bf_vmap(RESERVED_PSTORE_PHYMEM_BASE,
		RESERVED_PSTORE_PHYMEM_SIZE);
	if (pstore_addr == NULL) {
		print_err("bf_vmap faild!\n");
		return;
	}
	last_applog_addr = pstore_addr + RESERVED_PSTORE_PHYMEM_SIZE -
		LAST_APPLOG_SIZE;
	last_kmsg_addr = last_applog_addr - LAST_KMSG_SIZE;
	memcpy(pbuf, last_kmsg_addr, min((unsigned int)LAST_KMSG_SIZE,
		buf_size));
	vunmap(pstore_addr);
}
#endif

static void hisi_reboot(void)
{
	(void)register_callbacks_to_rdr();
	register_exceptions_to_rdr();
	rdr_syserr_process_for_ap(MODID_BOOT_FAIL, 0, 0);
}

static void hisi_shutdown(void)
{
}

static void get_raw_part_info(struct adapter *padp)
{
	if (padp == NULL) {
		print_invalid_params("padp: %p\n", padp);
		return;
	}

	padp->bfi_part.dev_path = BFI_DEV_PATH;
	padp->bfi_part.part_size = (unsigned int)get_bfi_part_size();
#ifdef CONFIG_SHUT_DETECTOR
	padp->bfi_part.shut_dev_path = SFI_DEV_PATH;
	padp->bfi_part.shut_part_size = (unsigned int)get_sfi_part_size();
	padp->bfi_part.shut_dev_offset = SFI_DEV_OFFSET;
}

static void get_shut_stage_ops(struct adapter *padp)
{
	if (padp == NULL) {
		print_invalid_params("padp: %p\n", padp);
		return;
	}

	padp->shut_stage_ops.set_stage = hisi_set_shut_stage;
	padp->shut_stage_ops.get_stage = hisi_get_shut_stage;
#endif
}

static void get_boot_stage_ops(struct adapter *padp)
{
	if (padp == NULL) {
		print_invalid_params("padp: %p\n", padp);
		return;
	}

	padp->stage_ops.set_stage = hisi_set_boot_stage;
	padp->stage_ops.get_stage = hisi_get_boot_stage;
}

static void get_sysctl_ops(struct adapter *padp)
{
	if (padp == NULL) {
		print_invalid_params("padp: %p\n", padp);
		return;
	}

	padp->sys_ctl.reboot = hisi_reboot;
	padp->sys_ctl.shutdown = hisi_shutdown;
}

static void get_phys_mem_info(struct adapter *padp)
{
	if (padp == NULL) {
		print_invalid_params("padp: %p\n", padp);
		return;
	}

	padp->pyhs_mem_info.base = SUB_RESERVED_UNUSED_PHYMEM_BASE;
	padp->pyhs_mem_info.size = BF_SIZE_1K;
}

static void get_log_ops_info(struct adapter *padp)
{
	if (padp == NULL) {
		print_invalid_params("padp: %p\n", padp);
		return;
	}
	/*lint -e666*/
	strncpy(padp->bl_log_ops.log_name,
		BL_LOG_NAME, min(strlen(BL_LOG_NAME),
		sizeof(padp->bl_log_ops.log_name) - 1));
	padp->bl_log_ops.log_size = (unsigned int)
		SUB_RESERVED_FASTBOOT_LOG_PYHMEM_SIZE;
	padp->bl_log_ops.capture_bl_log = capture_bl_log;
	strncpy(padp->kernel_log_ops.log_name,
		KERNEL_LOG_NAME, min(strlen(KERNEL_LOG_NAME),
		sizeof(padp->kernel_log_ops.log_name) - 1));
	/*lint +e666*/
	padp->kernel_log_ops.log_size = (unsigned int)LAST_KMSG_SIZE;
	padp->kernel_log_ops.capture_kernel_log = capture_kernel_log;
}

void degrade(int excp_type)
{
}

void bypass(int excp_type)
{
}

void load_backup(const char *part_name)
{
}

static void notify_storage_fault(unsigned long long bopd_mode)
{
}

static void platform_adapter_init(struct adapter *padp)
{
	if (padp == NULL) {
		print_invalid_params("padp: %p\n", padp);
		return;
	}

	get_raw_part_info(padp);
	get_phys_mem_info(padp);
	get_log_ops_info(padp);
	get_sysctl_ops(padp);
	get_boot_stage_ops(padp);
#ifdef CONFIG_SHUT_DETECTOR
	get_shut_stage_ops(padp);
#endif
	padp->prevent.degrade = degrade;
	padp->prevent.bypass = bypass;
	padp->prevent.load_backup = load_backup;
	padp->notify_storage_fault = notify_storage_fault;
}

int hisi_adapter_init(struct adapter *padp)
{
	if (padp == NULL) {
		print_invalid_params("padp: %p\n", padp);
		return -1;
	}

	if (common_adapter_init(padp) != 0) {
		print_err("int adapter common failed!\n");
		return -1;
	}
	platform_adapter_init(padp);

	return 0;
}
