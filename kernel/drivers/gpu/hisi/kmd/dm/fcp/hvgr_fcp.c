/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */
#include "hvgr_fcp.h"
#include "hvgr_regmap_fcp.h"

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/io.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/firmware.h>
#include <linux/workqueue.h>
#include <linux/crc16.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/slab.h>
#include <linux/completion.h>
#include <linux/pfn.h>
#include <securec.h>
#include <linux/vmalloc.h>

#include "hvgr_mem_api.h"
#include "hvgr_dm_api.h"
#include "hvgr_dm_ctx.h"
#include "hvgr_mmu_api.h"
#include "hvgr_log_api.h"
#include "hvgr_pm_api.h"

#define DMA_CX_CONFIG_ENABLE 1u

/*
 * hvgr_fcp_firmware_validate - fcp firmware validate, the crc, size, and
 * magic can be checked
 *
 * @gdev:   The hvgr device
 * @header: The firmware header
 * @size:   The firmware size
 *
 * Return: 0 on success or error code
 */
static int hvgr_fcp_firmware_validate(const struct hvgr_device *gdev,
	const struct hvg_fcp_header *header, const u32 size)
{
	u16 crc0;
	u16 crc1;

	if (header->magic != HVGR_FCP_FIRMWARE_MAGIC) {
		hvgr_err(gdev, HVGR_DM, "fcp magic in validate is invaild 0x%x\n",
			header->magic);
		return -EINVAL;
	}

	if ((size - sizeof(*header)) !=
		(header->size_sub0 + header->size_sub1)) {
		hvgr_err(gdev, HVGR_DM, "fcp size is invaild %u %u %u\n", size,
			header->size_sub0, header->size_sub1);
		return -EINVAL;
	}

	crc0 = crc16(0, header->data, header->size_sub0);
	crc1 = crc16(crc0, header->data + header->size_sub0, header->size_sub1);
	if (crc1 != header->crc) {
		hvgr_err(gdev, HVGR_DM, "fcp crc is invaild 0x%x 0x%x 0x%x\n",
			header->crc, crc0, crc1);
		return -EINVAL;
	}

	return 0;
}


void hvgr_fcp_configure_dma(struct hvgr_device *gdev,
	const struct fcp_dma_cfg *dma_cfg)
{
	u32 reg;
	u32 int_reg = (u32)fcp_dma_reg(gdev, FCP_DMA_CH_INT_TCRAW);
	u32 channel = dma_cfg->channel;

	/* 1. config itcm src addr */
	reg = (u32)fcp_dma_chanel_reg(gdev, channel, FCP_DMA_CX_SRC_ADDR_L);
	hvgr_write_reg(gdev, reg, dma_cfg->src_addr & 0xFFFFFFFFu);

	reg = (u32)fcp_dma_chanel_reg(gdev, channel, FCP_DMA_CX_SRC_ADDR_H);
	hvgr_write_reg(gdev, reg, (dma_cfg->src_addr >> 32u) & 0xFFFFu);

	/* 2. config itcm dst addr. Only 32bit low, 32bit high not used */
	reg = (u32)fcp_dma_chanel_reg(gdev, channel, FCP_DMA_CX_DES_ADDR_L);
	hvgr_write_reg(gdev, reg, dma_cfg->dst_addr);

	/* 3. config itcm firmware size */
	reg = (u32)fcp_dma_chanel_reg(gdev, channel, FCP_DMA_CX_CNT);
	hvgr_write_reg(gdev, reg, dma_cfg->size);

	hvgr_write_reg(gdev, int_reg, (1U << channel));

	/* 5. start dma */
	reg = (u32)fcp_dma_chanel_reg(gdev, channel, FCP_DMA_CX_CONFIG);
	hvgr_write_reg(gdev, reg, fcp_dma_cfg_val(dma_cfg->jasid, DMA_CX_CONFIG_ENABLE));
}

errno_t hvgr_get_firmware_name(struct hvgr_device *gdev, char *firmware_name, uint32_t name_len)
{
	char *name = NULL;

	if (firmware_name == NULL)
		return EINVAL;

	if (hvgr_hw_has_issue(gdev, HVGR_HW_ISSUE_50210))
		name = FCP_RISCV_TV210_FIRMWARE_NAME;
	else if (hvgr_hw_has_issue(gdev, HVGR_HW_ISSUE_50350))
		name = FCP_RISCV_TV350_FIRMWARE_NAME;
	else
		name = FCP_RISCV_FIRMWARE_NAME;

	return memcpy_s(firmware_name, name_len, name, strlen(name));
}

static int hvgr_fcp_requst_firmware(struct hvgr_device *gdev)
{
	int retval;
	u32 fw_size;
	const struct firmware *fw_entry = NULL;
	char fw_name[HVGR_FIRMWARE_NAME_LEN] = {};
	struct hvgr_fcp_data *fcp_data = gdev->dm_dev.fcp_data;

	if (hvgr_get_firmware_name(gdev, fw_name, HVGR_FIRMWARE_NAME_LEN) != EOK) {
		hvgr_err(gdev, HVGR_DM, "Fail to get fcp firmware name!");
		return -EFAULT;
	}

	retval = request_firmware(&fw_entry, fw_name, gdev->dev);
	if (retval || (fw_entry == NULL)) {
		hvgr_err(gdev, HVGR_DM, "Fail to load fcp firmware %s %d",
			fw_name, retval);
		return retval;
	}

	fw_size = (u32)fw_entry->size;
	if (fw_entry->size > FCP_MAX_SIZE_FIRMWARE ||
		fw_entry->size <= sizeof(struct hvg_fcp_header)) {
		hvgr_err(gdev, HVGR_DM, "fcp firmware size is invaild %u",
			fw_size);
		release_firmware(fw_entry);
		return -EFBIG;
	}

	if (memcpy_s(fcp_data->kva, FCP_MAX_SIZE_FIRMWARE, fw_entry->data, fw_entry->size) != 0) {
		hvgr_err(gdev, HVGR_DM, "fcp firmware copy fail");
		release_firmware(fw_entry);
		return -EFAULT;
	}

	release_firmware(fw_entry);

	retval = hvgr_fcp_firmware_validate(gdev,
		(const struct hvg_fcp_header *)fcp_data->kva, fw_size);
	if (retval) {
		(void)memset_s(fcp_data->kva, FCP_MAX_SIZE_FIRMWARE, 0, FCP_MAX_SIZE_FIRMWARE);
		hvgr_err(gdev, HVGR_DM, "fcp image validate fail %d", retval);
		return retval;
	}
	fcp_data->firmware_size = fw_size;
	return retval;
}

static bool hvgr_fcp_is_firmware_ready(struct hvgr_device *gdev)
{
	struct hvgr_fcp_data *fcp_data = gdev->dm_dev.fcp_data;
	const struct hvg_fcp_header *header =
		(const struct hvg_fcp_header *)fcp_data->kva;

	if (header->magic == HVGR_FCP_FIRMWARE_MAGIC)
		return true;

	hvgr_err(gdev, HVGR_DM, "Wrong fcp magic: 0x%x", header->magic);
	return false;
}

int hvgr_fcp_wait_dma_finish(struct hvgr_device *gdev, u32 channel)
{
	u32 int_reg = (u32)fcp_dma_reg(gdev, FCP_DMA_CH_INT_TCRAW);
	u32 value;
	u32 time = HVGR_DMA_MAX_TRY_TIMES;
	u32 ch_mask = (1U << channel);

	do {
		value = hvgr_read_reg(gdev, int_reg);
		time--;
	} while (((value & ch_mask) == 0) && (time != 0));

	if ((value & ch_mask) == 0)
		return -EBUSY;

	hvgr_write_reg(gdev, int_reg, ch_mask);
	return 0;
}

static int hvgr_fcp_upgrade_by_dma(struct hvgr_device *gdev)
{
	struct hvgr_fcp_data *fcp_data = gdev->dm_dev.fcp_data;
	int ret = 0;

	/* store DMA cfg */
	hvgr_fcp_store_dma_cfg(gdev, false);

	(void)hvgr_mmu_hal_enable(fcp_data->ctx);

	ret = hvgr_fcp_dma_proc(gdev);
	if (ret != EOK)
		goto out;

out:
	hvgr_fcp_recover_dma_cfg(gdev);
	return ret;
}

/*
 * hvgr_fcp_upgrade_firmware - Write the firmeware to the reg of FCP,
 * then boot the system
 *
 * @gdev:    The hvgr device
 *
 * Return: 0 on success or error code
 */
static int hvgr_fcp_upgrade_firmware(struct hvgr_device *gdev)
{
	int ret;

	if (!hvgr_fcp_is_firmware_ready(gdev))
		return -EBUSY;

	if (!hvgr_fcp_is_dev_ready(gdev))
		return -EBUSY;

	ret = hvgr_fcp_upgrade_by_dma(gdev);
	if (ret != 0)
		return ret;

	/* Start FCP */
	hvgr_fcp_upgrade_start_mcu(gdev);

	return hvgr_fcp_wait_boot_done(gdev);
}

/*
 * hvgr_fcp_load_by_poll - Load fcp firmware by polling INT status
 *
 * @gdev:    The hvgr device
 *
 * Return: 0 on success or error code
 */
int hvgr_fcp_load_by_poll(struct hvgr_device *gdev)
{
	int ret;
	if (gdev->dm_dev.fcp_data == NULL)
		return 0;

	if (!hvgr_fcp_is_load_needed(gdev))
		return 0;

	ret = hvgr_fcp_upgrade_firmware(gdev);
	if (ret != 0) {
		hvgr_dmd_msg_set(gdev, DMD_FCP_LOAD_FAIL, "fcp firmware load fail");
		hvgr_dmd_msg_report(gdev);
	}
	return ret;
}

/*
 * hvgr_fcp_loader_entry - Trigger load fcp firmware
 *
 * @gdev:    The hvgr device
 */
void hvgr_fcp_loader_entry(struct hvgr_device *gdev)
{
	int time;
	if (gdev->dm_dev.fcp_data == NULL)
		return;

	/*
	 * we must init the lock just once,
	 * because context will be created for many times.
	 */
	if (!gdev->dm_dev.fcp_data->load_entryed) {
		complete(&gdev->dm_dev.fcp_data->complete);
#ifdef CONFIG_HVGR_MODEL
#if hvgr_version_eq(HVGR_V200)
		/* For v200 cmodel, do not need to load firmware */
		gdev->dm_dev.fcp_data->load_entryed = true;
		return;
#endif
#endif
		/* Phase2: need to wait for request done 10s at top */
		time = 5000;
		while ((!(gdev->dm_dev.fcp_data->fcp_firmware_ready)) && (time != 0)) {
			msleep(2);
			time--;
		}
		if (!(gdev->dm_dev.fcp_data->fcp_firmware_ready))
			hvgr_err(gdev, HVGR_DM, "fcp is not ready");
		gdev->dm_dev.fcp_data->load_entryed = true;
	}
}

static int hvgr_fcp_malloc_firmware_buf(struct hvgr_device *gdev)
{
	struct hvgr_fcp_data *fcp_data = gdev->dm_dev.fcp_data;
	struct hvgr_ctx *ctx = NULL;
	struct hvgr_mem_area *area = NULL;
	struct hvgr_mmu_setup_paras mmu_para = {0};
	long ret;

#if hvgr_version_ge(HVGR_V350)
	ctx = gdev->com_ctx.ctx;
	if (ctx == NULL) {
		hvgr_err(gdev, HVGR_DM, "fcp get common ctx create fail");
		return -ENOMEM;
	}
#else
	/* create ctx */
	ctx = vmalloc(sizeof(*ctx));
	if (ctx == NULL) {
		hvgr_err(gdev, HVGR_DM, "fcp task cteate context fail");
		return -ENOMEM;
	}
	(void)memset_s(ctx, sizeof(*ctx), 0, sizeof(*ctx));

	ctx->asid = FCP_UPGRADE_DMA_JASID;
	ctx->gdev = gdev;
	(void)hvgr_mem_init_proc(ctx);
#endif

	area = kzalloc(sizeof(*area), GFP_KERNEL);
	if (area == NULL) {
		hvgr_err(gdev, HVGR_DM, "fcp task cteate area fail");
		goto fail_area;
	}

	ctx->mem_ctx.zones[0].pmem = &ctx->mem_ctx;
	area->zone = &ctx->mem_ctx.zones[0];
	area->pages = FCP_SIZE_IN_PAGE;
	area->max_pages = FCP_SIZE_IN_PAGE;

	if (!hvgr_mem_alloc_pages(area)) {
		hvgr_err(gdev, HVGR_DM, "fcp task alloc page fail");
		goto fail_pages;
	}

	area->attribute = HVGR_MEM_ATTR_CPU_RD | HVGR_MEM_ATTR_CPU_WR | HVGR_MEM_ATTR_GPU_RD;

	if (!hvgr_mem_map_kva(area)) {
		hvgr_err(gdev, HVGR_DM, "fcp task kmap page fail");
		goto fail_kmap;
	}

#if hvgr_version_ge(HVGR_V350)
	area->gva = COMMON_CTX_FCP_BUF_START;
#else
	area->gva = area->kva;
#endif
	hvgr_mem_attr2flag(area);

	mmu_para.gva = area->gva;
	mmu_para.page_array = area->page_array;
	mmu_para.pages = area->pages;
	mmu_para.mmu_flag = area->mmu_flag;
	mmu_para.flags = area->flags;

	ret = hvgr_mmu_set_up(ctx, &mmu_para);
	if (ret != 0) {
		hvgr_err(gdev, HVGR_DM, "fcp task set up pgd fail");
		goto fail_pgd;
	}

	fcp_data->area = area;
	fcp_data->ctx = ctx;
	fcp_data->gva = area->gva;
	fcp_data->kva = u64_to_ptr(area->kva);

	return 0;

fail_pgd:
	hvgr_mem_unmap_kva(area);
fail_kmap:
	(void)hvgr_mem_free_pages(area);
fail_pages:
	kfree(area);
fail_area:
#if hvgr_version_lt(HVGR_V350)
	hvgr_mem_term_proc(ctx);
	vfree(ctx);
#endif
	return -ENOMEM;
}

static void hvgr_fcp_release_firmware_buf(struct hvgr_device *gdev)
{
	struct hvgr_fcp_data *fcp_data = gdev->dm_dev.fcp_data;

	if (fcp_data == NULL)
		return;

	hvgr_mem_unmap_kva(fcp_data->area);
	(void)hvgr_mem_free_pages(fcp_data->area);
#if hvgr_version_lt(HVGR_V350)
	hvgr_mem_term_proc(fcp_data->ctx);
	vfree(fcp_data->ctx);
#endif
	kfree(fcp_data->area);

	fcp_data->area = NULL;
	fcp_data->ctx = NULL;
	fcp_data->kva = NULL;
	fcp_data->gva = 0;
}

/*
 * hvgr_fcp_loader_task - fcp firmware load task
 *
 * @data: Task param, the hvgr device
 *
 * Return: 0 on success or error code
 */
int hvgr_fcp_loader_task(void *data)
{
	int retval;
	struct hvgr_device *gdev = (struct hvgr_device *)data;

	wait_for_completion(&gdev->dm_dev.fcp_data->complete);

	if (hvgr_fcp_malloc_firmware_buf(gdev)) {
		hvgr_err(gdev, HVGR_DM, "Fail to requst firmware buf");
		return -ENOMEM;
	}

	retval = hvgr_fcp_requst_firmware(gdev);
	if (retval != 0) {
		hvgr_fcp_release_firmware_buf(gdev);
		hvgr_err(gdev, HVGR_DM, "Fail to requst firmware %d", retval);
		return retval;
	}

	gdev->dm_dev.fcp_data->fcp_firmware_ready = true;
	/*
	 * The file system is not ready in the device probe phase.
	 * When the GPU is powered on, the fcp fails to be loaded.
	 * Therefore, we need to reload the fcp after the file system
	 * is ready.
	 */
	if (hvgr_is_always_on(gdev)) {
		mutex_lock(&gdev->pm_dev.pm_pwr.power_lock);
		if (hvgr_pm_get_gpu_status(gdev) == HVGR_PM_GPU_READY)
			retval = hvgr_fcp_load_by_poll(gdev);
		mutex_unlock(&gdev->pm_dev.pm_pwr.power_lock);
	}

	if (retval == 0)
		hvgr_debug(gdev, HVGR_DM, "Fcp load firmware Pass!");
	else
		hvgr_err(gdev, HVGR_DM, "Fcp load firmware Fail!");

	return retval;
}

/*
 * hvgr_fcp_loader_init - fcp load task init
 *
 * @gdev:    The hvgr device
 *
 * Return: 0 on success, -1 on fail
 */
int hvgr_fcp_loader_init(struct hvgr_device *gdev)
{
	struct hvgr_fcp_data *fcp_data = NULL;

	gdev->dm_dev.fcp_data = kzalloc(sizeof(*gdev->dm_dev.fcp_data), GFP_KERNEL);
	if (gdev->dm_dev.fcp_data == NULL) {
		hvgr_err(gdev, HVGR_DM, "Fail to create fcp data");
		return -ENOMEM;
	}
	fcp_data = gdev->dm_dev.fcp_data;

	init_completion(&gdev->dm_dev.fcp_data->complete);
	gdev->dm_dev.fcp_data->fcp_firmware_ready = false;

	fcp_data->thread = kthread_run(hvgr_fcp_loader_task, (void *)gdev,
		FCP_LOADER_TASK_NAME);
	if (IS_ERR(fcp_data->thread)) {
		kfree(gdev->dm_dev.fcp_data);
		hvgr_err(gdev, HVGR_DM, "Fail to create fcp tsk");
		return -1;
	}

	gdev->dm_dev.fcp_data->load_entryed = false;

	return 0;
}

/*
 * hvgr_fcp_loader_term - Release fcp load resource
 *
 * @gdev:    The hvgr device
 */
void hvgr_fcp_loader_term(struct hvgr_device *gdev)
{
	struct hvgr_fcp_data *fcp_data = gdev->dm_dev.fcp_data;
	struct hvgr_ctx *ctx = NULL;

	if (fcp_data == NULL)
		return;

	if (fcp_data->thread != NULL) {
		kthread_stop(fcp_data->thread);
		fcp_data->thread = NULL;
	}

	ctx = fcp_data->ctx;

	if (ctx != NULL)
		hvgr_fcp_release_firmware_buf(gdev);

	kfree(gdev->dm_dev.fcp_data);
	gdev->dm_dev.fcp_data = NULL;
}

