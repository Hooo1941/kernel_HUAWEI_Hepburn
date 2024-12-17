/*
 * drv_venc_mcore.c
 *
 * This is for Operations Related to mcore.
 *
 * Copyright (c) 2021-2021 Huawei Technologies CO., Ltd.
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

#include "drv_venc_mcore.h"
#include <linux/kernel_read_file.h>
#include <linux/delay.h>
#include <linux/io.h>
#include "soc_venc_reg_interface.h"
#include "drv_common.h"
#include "drv_venc_osal.h"
#include "drv_mem.h"
#include "venc_regulator.h"
#include "drv_venc_mcore_tcm.h"
#include "drv_venc_ipc.h"

#define REG_VEDU_VCPI_SWPTRADDR_H  (VEDU_REG_BASE + 0x09d4)
#define MCORE_IMG_SIZE (32 * 1024)
#define MCORE_DDR_LOG_SIZE (1000 * 1024) // for mcore log recording
#define MCORE_DDR_DATA_SIZE (9 * 1024 * 1024) // for rate control algrithom
#define MCORE_DDR_SIZE (10 * 1024 * 1024)
#define VENC_MCORE_IMG  "/vendor/firmware/venc_mcore.img"

static int32_t venc_mcore_load_image(venc_entry_t *venc)
{
	uint32_t i;
	uint32_t size = venc->mcu.image_size;
	uint32_t *src = (uint32_t *)venc->mcu.image;
	uint32_t *dest = (uint32_t *)VRCE_SRAM_BASE;

	if (!src) {
		VCODEC_ERR_VENC("error: mcore img is null, size %u", size);
		return VCODEC_FAILURE;
	}
	if (size % 4 != 0 || size > MCORE_IMG_SIZE) { /* 4 Bytes */
		VCODEC_ERR_VENC("mcore img size error %u", size);
		return VCODEC_FAILURE;
	}

	for (i = 0; i < size / sizeof(uint32_t); ++i)
		writel(src[i], dest + i);
	VCODEC_DBG_VENC("load mcore image success, size %u", size);
	return 0;
}

static int32_t venc_mcore_alloc_cma_ddr(struct platform_device *pdev)
{
	int32_t ret;
	venc_entry_t *venc = platform_get_drvdata(pdev);

	if (!venc)
		return VCODEC_FAILURE;

	ret = drv_mem_alloc_dma(&pdev->dev, MCORE_DDR_SIZE, &venc->mcu.mcu_ddr);
	if (ret) {
		VCODEC_ERR_VENC("alloc dma mem error");
		return ret;
	}

	// header should be cleared
	ret = memset_s(venc->mcu.mcu_ddr.virt_addr, MCORE_DDR_SIZE, 0, 2 * sizeof(uint32_t));
	if (ret) {
		VCODEC_ERR_VENC("memset error");
		drv_mem_free_dma(&pdev->dev, &venc->mcu.mcu_ddr);
		return ret;
	}
	return 0;
}

static void venc_mcore_free_cma_ddr(struct platform_device *pdev)
{
	venc_entry_t *venc = platform_get_drvdata(pdev);

	if (!venc || !venc->mcu.mcu_ddr.virt_addr) {
		VCODEC_ERR_VENC("mcu ddr error");
		return;
	}
	drv_mem_free_dma(&pdev->dev, &venc->mcu.mcu_ddr);
}

static int32_t venc_mcore_alloc_and_init_image_ddr(struct platform_device *pdev)
{
	int32_t ret;
	void *image_ddr = NULL;
	void *data = NULL;
	size_t size = 0;
	venc_entry_t *venc = platform_get_drvdata(pdev);

	/* read mcu image */
	ret = kernel_read_file_from_path(VENC_MCORE_IMG, 0, &data, MCORE_IMG_SIZE, &size, READING_FIRMWARE);
	if (ret < 0 || size == 0) {
		VCODEC_ERR_VENC("error to read mcore image file, ret %d", ret);
		return VCODEC_FAILURE;
	}
	if (size % 4 != 0 || size > MCORE_IMG_SIZE) { /* 4 Bytes */
		VCODEC_ERR_VENC("mcore img size error %lu", size);
		goto error;
	}

	image_ddr = vcodec_mem_valloc((uint32_t)size);
	if (!image_ddr) {
		VCODEC_FATAL_VENC("call vmalloc failed");
		goto error;
	}

	/* copy write image to ddr */
	ret = memcpy_s(image_ddr, size, data, size);
	if (ret) {
		vcodec_mem_vfree(image_ddr);
		VCODEC_FATAL_VENC("copy image to ddr error");
		goto error;
	}
	vfree(data);

	venc->mcu.image = image_ddr;
	venc->mcu.image_size = (uint32_t)size;
	VCODEC_DBG_VENC("read mcore image success, size %lu", size);
	return 0;
error:
	vfree(data);
	return VCODEC_FAILURE;
}

static void venc_mcore_free_image_ddr(struct platform_device *pdev)
{
	venc_entry_t *venc = platform_get_drvdata(pdev);

	if (!venc) {
		VCODEC_ERR_VENC("venc entry is null error");
		return;
	}
	if (!venc->mcu.image) {
		VCODEC_FATAL_VENC("mcore image is null pointer error");
		return;
	}
	vcodec_mem_vfree(venc->mcu.image);
	venc->mcu.image = NULL;
	VCODEC_DBG_VENC("free mcore image success");
}

int32_t venc_mcore_alloc_ddr(struct platform_device *pdev)
{
	int32_t ret;
	ret = venc_mcore_alloc_cma_ddr(pdev);
	if (ret)
		return ret;

	ret = venc_mcore_alloc_and_init_image_ddr(pdev);
	if (ret) {
		venc_mcore_free_cma_ddr(pdev);
		return ret;
	}
	return 0;
}

void venc_mcore_free_ddr(struct platform_device *pdev)
{
	venc_mcore_free_cma_ddr(pdev);
	venc_mcore_free_image_ddr(pdev);
}

static int32_t setup_mcore_memory(void)
{
	venc_entry_t *venc = NULL;

	venc = platform_get_drvdata(venc_get_device());
	if (!venc || !venc->mcu.mcu_ddr.virt_addr) {
		VCODEC_ERR_VENC("mcu ddr error");
		return VCODEC_FAILURE;
	}
	writel(venc->mcu.mcu_ddr.iova_addr, REG_VEDU_VCPI_SWPTRADDR_H);
	return 0;
}

int32_t venc_mcore_open(void)
{
	venc_entry_t *venc = NULL;
	S_VE_SUB_CTRL_REGS_TYPE *ctl = NULL;

	venc = platform_get_drvdata(venc_get_device());

	if (venc_ipc_init(&venc->ctx[0]) != 0)
		return VCODEC_FAILURE;

	ctl = (S_VE_SUB_CTRL_REGS_TYPE *)VEDU_SUB_CTRL_BASE;
	{
		U_SUBCTRL_CRG0 reg;
		reg.bits.mcu_por_rst = 1;
		reg.bits.mcu_wdt_rst = 1;
		ctl->SUBCTRL_CRG0.u32 = reg.u32;
	}

	{
		U_SUBCTRL_CRG3 reg;
		reg.bits.tim_clk_en = 1;
		reg.bits.ipc_clk_en = 1;
		ctl->SUBCTRL_CRG3.u32 = reg.u32;
	}

	// start address
	ctl->SUBCTRL_MCU_POR_PC = venc_get_mcore_code_base();

	// 1 set mcu_core_wait = 1, then mcu enter wait state
	{
		U_SUBCTRL_CRG0 reg;
		reg.bits.mcu_core_wait = 1;
		ctl->SUBCTRL_CRG0.u32 = reg.u32;
	}

	// 2 set mcu_por_rst = 0, then mcu exit reset state
	ctl->SUBCTRL_CRG0.bits.mcu_por_rst = 0; // mcu_core_wait should be 1 in the mean time
	ctl->SUBCTRL_CRG0.bits.mcu_wdt_rst = 0;

	// 3 load mcu image to ITCM by AP
	if (venc_mcore_load_image(venc) != 0)
		goto ERROR;

	if (setup_mcore_memory() != 0)
		goto ERROR;

	// 4 set mcu_core_wait = 0, then start mcu core
	{
		U_SUBCTRL_CRG0 reg;
		reg.bits.mcu_core_wait = 0;
		ctl->SUBCTRL_CRG0.u32 = reg.u32;
	}

	udelay(500);
	return 0;
ERROR:
	venc_ipc_deinit(&venc->ctx[0]);
	return VCODEC_FAILURE;
}

void venc_mcore_close(void)
{
	venc_entry_t *venc = NULL;
	S_VE_SUB_CTRL_REGS_TYPE *ctl = NULL;
	U_SUBCTRL_CRG1 reg;

	venc = platform_get_drvdata(venc_get_device());

	venc_mcore_log_dump();

	ctl = (S_VE_SUB_CTRL_REGS_TYPE *)VEDU_SUB_CTRL_BASE;

	// stop mcore
	reg.bits.mcu_clk_en = 0;
	ctl->SUBCTRL_CRG1.u32 = reg.u32;

	venc_ipc_deinit(&venc->ctx[0]);
}

void *venc_regbase(void)
{
	venc_entry_t *venc = NULL;
	venc = platform_get_drvdata(venc_get_device());
	return venc->ctx[0].reg_base;
}

void venc_mcore_log_dump(void)
{
	venc_entry_t *venc = NULL;
	int32_t *src = NULL;
	uint32_t len, print_size;
	char log[1024] = {0};
	int32_t ret;

	venc = platform_get_drvdata(venc_get_device());

	src = venc->mcu.mcu_ddr.virt_addr + MCORE_DDR_DATA_SIZE;
	if (!src) {
		VCODEC_FATAL_VENC("mcore is not init");
		return;
	}

	len = (uint32_t)(src[1]);
	if (len > MCORE_DDR_LOG_SIZE) {
		VCODEC_WARN_VENC("mcore log buffer maybe overwrite");
		len = MCORE_DDR_LOG_SIZE;
	}

	print_size = len < (sizeof(log) - 1) ? len : (sizeof(log) - 1);
	ret = memcpy_s(log, sizeof(log), src + 2, print_size);
	if (ret) {
		VCODEC_FATAL_VENC("memcpy error");
		return;
	}
	log[print_size] = 0;

	VCODEC_INFO_VENC("mcore log size %u: %s", len, log);
}

