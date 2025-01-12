/*
 * crbroi_mem.c
 *
 * cross-reboot read only info transfer via warm-reboot nvram
 *
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
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

#include <linux/kernel.h>
#include <linux/io.h>
#if defined(CONFIG_HUAWEI_STORAGE_ROFA_FOR_MTK)
#include <soc/mediatek/log_store_kernel.h>
#elif !defined(CONFIG_BOOT_DETECTOR_QCOM)
#include <platform_include/basicplatform/linux/rdr_platform.h>
#endif

#include "crbroi.h"

#define CRBROI_NAME "crbroi_mem"

#define print_info(f, arg...) \
	pr_info(CRBROI_NAME ": " f, ## arg)

#define print_err(f, arg...) \
	pr_err(CRBROI_NAME ": " f, ## arg)
#if defined(CONFIG_BOOT_DETECTOR_QCOM)
#define QCOM_SUB_RESERVED_UNUSED_PHYMEM_BASE 0xB0400000
#endif

#if defined(CONFIG_HUAWEI_STORAGE_ROFA_FOR_MTK)
#ifdef CONFIG_BOOT_DETECTOR
#define BFM_SUB_BOOTFAIL_MAGIC_NUM_ADDR	(get_rofa_mem_base())
#else
#define BFM_SUB_BOOTFAIL_MAGIC_NUM_ADDR \
	(((struct sram_log_header *)CONFIG_MTK_DRAM_LOG_STORE_ADDR)->bopd_ro_info)
#endif
#elif defined(CONFIG_BOOT_DETECTOR_QCOM)
#define BFM_SUB_BOOTFAIL_MAGIC_NUM_ADDR (QCOM_SUB_RESERVED_UNUSED_PHYMEM_BASE)
#else
#define BFM_SUB_BOOTFAIL_MAGIC_NUM_ADDR (SUB_RESERVED_UNUSED_PHYMEM_BASE)
#endif

#define BFM_SUB_BOOTFAIL_NUM_OFFSET	 4
#define BFM_SUB_BOOTFAIL_COUNT_OFFSET   8
#define CRBROI_IMPL_ROUND_OFFSET		12
#define CRBROI_MEM_LEN				  4

#define BFM_SUB_BOOTFAIL_MAGIC_NUMBER		   0x42464E4F /* BFNO */
#define BFM_SUB_BOOTFAIL_NUM_STORAGE_RDONLY	 0x7ffffffb

struct crbroi_mem_header {
	unsigned int magic;
	unsigned int errcode;
};

/*
 * the real crbroi used to update storage_crbroi.crbroi.mem in crbroi_init.
 */
union storage_crbroi_impl {
	unsigned int crbroi;

	struct {
		unsigned char round_ldr;
		unsigned char method_ldr;
		unsigned char round_krnl;
		unsigned char method_krnl;
	} crbroi_mem;
};

static struct crbroi_mem_header g_crbroi_header;
static union storage_crbroi g_crbroi_intf;
static union storage_crbroi_impl g_crbroi_impl;

static unsigned char *g_memmap_addr;

static inline void *crbroi_map_reserved_phys_mem(void)
{
#if defined(CONFIG_HUAWEI_STORAGE_ROFA_FOR_MTK) && defined(CONFIG_BOOT_DETECTOR)
	return (void *)BFM_SUB_BOOTFAIL_MAGIC_NUM_ADDR;
#else
	return ioremap(BFM_SUB_BOOTFAIL_MAGIC_NUM_ADDR,
			CRBROI_MEM_LEN * sizeof(unsigned int));
#endif
}

static int crbroi_mem_write_header(unsigned int magic,
	unsigned int errcode)
{
	unsigned char *la = g_memmap_addr;

	if (la == NULL)
		return -1;

	writel(magic, la);
	writel(errcode, la + BFM_SUB_BOOTFAIL_NUM_OFFSET);
	print_info("%s: write crbroi header ok\n", __func__);
	return 0;
}

static int crbroi_mem_write_intf(unsigned int crbroi)
{
	unsigned char *la = g_memmap_addr;

	if (la == NULL)
		return -1;

	writel(crbroi, la + BFM_SUB_BOOTFAIL_COUNT_OFFSET);
	print_info("%s: write crbroi ok\n", __func__);
	return 0;
}

static int crbroi_mem_write_impl(unsigned int crbroi)
{
	unsigned char *la = g_memmap_addr;

	if (la == NULL)
		return -1;

	writel(crbroi, la + CRBROI_IMPL_ROUND_OFFSET);
	print_info("%s: write crbroi ok\n", __func__);
	return 0;
}

static int crbroi_mem_read(struct crbroi_mem_header *header,
	union storage_crbroi *intf, union storage_crbroi_impl *impl)
{
	unsigned char *la = g_memmap_addr;

	if (la == NULL)
		return -1;

	if (header) {
		header->magic = readl(la);
		header->errcode = readl(la + BFM_SUB_BOOTFAIL_NUM_OFFSET);
	}

	if (intf)
		intf->crbroi = readl(la + BFM_SUB_BOOTFAIL_COUNT_OFFSET);

	if (impl)
		impl->crbroi = readl(la + CRBROI_IMPL_ROUND_OFFSET);

	return 0;
}

static unsigned int is_storage_readonly_type(struct crbroi_mem_header *header)
{
	return (header->magic ==  BFM_SUB_BOOTFAIL_MAGIC_NUMBER &&
		header->errcode == BFM_SUB_BOOTFAIL_NUM_STORAGE_RDONLY);
}

/*
 * the following are interface implementation routines
 *
 */
static int crbroi_mem_func_init(void)
{
	int res;

	g_memmap_addr = crbroi_map_reserved_phys_mem();
	if (g_memmap_addr == NULL) {
		print_err("%s: map memory failed\n", __func__);
		return -1;
	}

	res = crbroi_mem_read(&g_crbroi_header,
		&g_crbroi_intf, &g_crbroi_impl);
	print_info("%s: get crbroi_mem <0x%x, 0x%x, 0x%x, 0x%x>\n",
		__func__, g_crbroi_header.magic, g_crbroi_header.errcode,
		g_crbroi_intf.crbroi, g_crbroi_impl.crbroi);

	return res;
}

static int crbroi_mem_func_exit(void)
{
	if (g_memmap_addr != NULL)
		iounmap(g_memmap_addr);

	return 0;
}

/*
 * increase impl crbroi_mem.round_krnl with method.
 */
static int crbroi_mem_transfer(unsigned int method)
{
	union storage_crbroi_impl crbroi = g_crbroi_impl;

	if (is_storage_readonly_type(&g_crbroi_header) == 0) {
		crbroi_mem_write_intf(0);
		crbroi.crbroi = 0;
		crbroi.crbroi_mem.round_krnl = 1;
		print_info("%s: crbroi loop new, set to round 1\n", __func__);
	} else {
		++crbroi.crbroi_mem.round_krnl;
		print_info("%s: crbroi loop in-progress with round %u\n",
			__func__, crbroi.crbroi_mem.round_krnl);
	}

	crbroi.crbroi_mem.method_krnl = method;

	crbroi_mem_write_header(BFM_SUB_BOOTFAIL_MAGIC_NUMBER,
		BFM_SUB_BOOTFAIL_NUM_STORAGE_RDONLY);
	crbroi_mem_write_impl(crbroi.crbroi);

	return 0;
}

/*
 * get intf crbroi_mem method.
 */
static int crbroi_mem_get_method(unsigned int *method)
{
	if (is_storage_readonly_type(&g_crbroi_header)) {
		*method = g_crbroi_intf.crbroi_mem.method;
		return 0;
	}

	return -1;
}

static int crbroi_mem_get_round(unsigned int *round)
{
	if (is_storage_readonly_type(&g_crbroi_header)) {
		*round = g_crbroi_intf.crbroi_mem.round;
		return 0;
	}

	return -1;
}

static int crbroi_mem_clr(void)
{
	struct crbroi_mem_header header;
	int res;

	res = crbroi_mem_read(&header, NULL, NULL);
	if (res != 0)
		return res;

	if (is_storage_readonly_type(&header) == 0) {
		print_info("%s: crbroi loop NOT in progress\n", __func__);
		return 0;
	}

	print_info("%s: clear crbroi\n", __func__);
	crbroi_mem_write_header(0, 0);
	crbroi_mem_write_intf(0);
	crbroi_mem_write_impl(0);

	return 0;
}

struct storage_crbroi_func crbroi_mem_func = {
	.crbroi_func_init = crbroi_mem_func_init,
	.transfer_crbroi = crbroi_mem_transfer,
	.get_crbroi_method = crbroi_mem_get_method,
	.get_crbroi_round = crbroi_mem_get_round,
	.clr_crbroi = crbroi_mem_clr,
	.crbroi_func_exit = crbroi_mem_func_exit,
};

struct storage_crbroi_func *get_storage_crbroi_func(void)
{
	return &crbroi_mem_func;
}

void storage_rofa_info_clear(void)
{
	unsigned char *bopd_info = NULL;

#if defined(CONFIG_HUAWEI_STORAGE_ROFA_FOR_MTK) && defined(CONFIG_BOOT_DETECTOR)
	bopd_info = (unsigned char *)BFM_SUB_BOOTFAIL_MAGIC_NUM_ADDR;
#else
	bopd_info = (unsigned char *)ioremap(
		BFM_SUB_BOOTFAIL_MAGIC_NUM_ADDR,
		CRBROI_MEM_LEN * sizeof(unsigned int));
#endif
	if (!bopd_info) {
		print_info("%s: remap bopd info failed\n", __func__);
		return;
	}

	writel(0, bopd_info);
	writel(0, bopd_info + BFM_SUB_BOOTFAIL_NUM_OFFSET);
	writel(0, bopd_info + BFM_SUB_BOOTFAIL_COUNT_OFFSET);
	writel(0, bopd_info + CRBROI_IMPL_ROUND_OFFSET);

	iounmap(bopd_info);
}
