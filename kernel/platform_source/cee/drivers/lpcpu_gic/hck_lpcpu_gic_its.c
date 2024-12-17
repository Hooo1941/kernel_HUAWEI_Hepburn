/*
 *
 * hck gic-its module
 *
 * Copyright (c) 2023-2023 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <linux/init.h>
#include <securec.h>
#include <platform_include/cee/linux/hck/lpcpu/gic/hck_lpcpu_gic_its.h>
#include <asm/smp.h>
#include <linux/irqchip/arm-gic-common.h>
#include <linux/irqchip/arm-gic-v3.h>

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))

#define LPI_PROP_DEFAULT_PRIO GICD_INT_DEF_PRI

static void lpcpu_gic_reset_prop_table_change_lpi_prop(void *va, int size, int *hook_flag)
{
	*hook_flag = 0;
#ifdef CONFIG_ARCH_PLATFORM
	/* Priority 0xa0, Group-0/1, disabled */
	memset_s(va, size, LPI_PROP_DEFAULT_PRIO | LPI_PROP_GROUP1 | 0x1, size);
	*hook_flag = 1;
#endif
}

static void lpcpu_gic_its_share_ability(unsigned long long *val)
{
#ifdef CONFIG_ARCH_PLATFORM
	*val &= ~GITS_BASER_SHAREABILITY_MASK;
#endif
}

void __init hck_lpcpu_gic_its_register(void)
{
	REGISTER_HCK_VH(lpcpu_gic_reset_prop_table_change_lpi_prop, lpcpu_gic_reset_prop_table_change_lpi_prop);
	REGISTER_HCK_VH(lpcpu_gic_its_share_ability, lpcpu_gic_its_share_ability);
}

#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0) */
