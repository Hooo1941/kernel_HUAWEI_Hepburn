/*
 *  Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 *  Description: hkip map kernel proctect
 *  Create : 2023/4/10
 */

#include <asm/sections.h>
#include <linux/pgtable.h>
#include <linux/arm-smccc.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/memblock.h>
#include <platform_include/see/bl31_smc.h>

static phys_addr_t pgtable_protect_start __ro_after_init;
static phys_addr_t pgtable_protect_size __ro_after_init;
static unsigned long pgtable_phys_offset;

static unsigned long hkip_phys_alloc(unsigned long size, unsigned long align,
	unsigned long phys_start, unsigned long phys_size, unsigned long *offset)
{
	const unsigned long mask = align - 1;
	unsigned long phys_offset = *offset;
	unsigned long curent_ptr = phys_start + phys_offset;
	unsigned long padding = (-curent_ptr) & mask;

	if (phys_start == 0) {
		pr_err("hkip phys alloc phys_start is zero\n");
		return 0;
	}

	/*
	 * align must be power of 2
	 * size must be multiple of align
	 */
	if ((align == 0) || (align & (align - 1)) || (size & (align - 1))) {
		pr_err("hkip phys alloc invalid input para, align 0x%lx, size 0x%lx\n",
		       align, size);
		return 0;
	}

	if (unlikely((phys_offset >= phys_size) || ((padding + size) >= SIZE_MAX))) {
		pr_err("hkip phys alloc overflow, offset 0x%lx, padding 0x%lx, size 0x%lx\n",
		       phys_offset, padding, size);
		return 0;
	}

	if (unlikely((phys_size - phys_offset) < (padding + size))) {
		pr_err("hkip phys alloc is insufficient, offset 0x%lx, padding 0x%lx, size 0x%lx\n",
		       phys_offset, padding, size);
		return 0;
	}

	*offset += size + padding;

	return curent_ptr + padding;
}

void hkip_set_pgtable_protect_addr(void)
{
	struct arm_smccc_res res = {0};

	arm_smccc_smc(FID_HKIP_KERNEL_PAGETBL_INFO, 0, 0, 0, 0, 0, 0, 0, &res);
	if (res.a0 != 0) {
		pr_err("fail to get kernel pagetal info form bl31.\n");
		pgtable_protect_start = 0;
		pgtable_protect_size = 0;
		return;
	}

	pgtable_protect_start = (phys_addr_t)res.a1;
	pgtable_protect_size = (phys_addr_t)res.a2;
	BUG_ON(pgtable_protect_start == 0);
	BUG_ON(pgtable_protect_size == 0);
}

int hkip_get_pgtable_protect_addr(phys_addr_t *start, phys_addr_t *end)
{
	if (!start || !end) {
		pr_err("hkip phys alloc invalid input para\n");
		return -EINVAL;
	}

	*start = pgtable_protect_start;
	*end = pgtable_protect_start + pgtable_protect_size;
	return 0;
}

unsigned long hkip_pgtable_phys_alloc(unsigned long size, unsigned long align)
{
	unsigned long phys_addr = 0;

	phys_addr = hkip_phys_alloc(size, align, pgtable_protect_start,
		pgtable_protect_size, &pgtable_phys_offset);
	if (WARN(!phys_addr, "hkip pgtable alloc failed, use memblock"))
		phys_addr = memblock_phys_alloc(PAGE_SIZE, PAGE_SIZE);
	return phys_addr;
}