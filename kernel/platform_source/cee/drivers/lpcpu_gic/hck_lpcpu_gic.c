/*
 *
 * hck gic module
 *
 * Copyright (c) 2022-2022 Huawei Technologies Co., Ltd.
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
#include <platform_include/cee/linux/hck/lpcpu/gic/hck_lpcpu_gic.h>
#include <linux/irqdomain.h>
#include <linux/interrupt.h>
#include <asm/smp.h>

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))

static void lpcpu_gic_smp_register_sgis(struct irq_domain *domain, int irq_base,
					unsigned int nr_irqs, int node, void *arg,
					bool realloc_flag,
					const struct irq_affinity_desc *affinity,
					int *hook_flag, int *base_sgi)
{
	*hook_flag = 0;
#ifdef CONFIG_ARCH_PLATFORM
	/* Register all 16 non-secure SGIs */
	*base_sgi = __irq_domain_alloc_irqs(domain, irq_base, nr_irqs, node, arg, realloc_flag, affinity);
	*hook_flag = 1;
#endif
}

static void lpcpu_gic_smp_ipi_range(int ipi_base, int nr_ipi, int *hook_flag)
{
	*hook_flag = 0;
#ifdef CONFIG_ARCH_PLATFORM
	set_smp_ipi_range(ipi_base, nr_ipi);
	*hook_flag = 1;
#endif
}

void __init hck_lpcpu_gic_register(void)
{
	REGISTER_HCK_VH(lpcpu_gic_smp_register_sgis, lpcpu_gic_smp_register_sgis);
	REGISTER_HCK_VH(lpcpu_gic_smp_ipi_range, lpcpu_gic_smp_ipi_range);
}

#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0) */
