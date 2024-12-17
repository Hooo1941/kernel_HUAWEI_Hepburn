#ifndef __HCK_LPCPU_GIC__
#define __HCK_LPCPU_GIC__

#include <linux/types.h>
#include <platform_include/basicplatform/linux/hck_vendor_hooks.h>
/*
 * Following tracepoints are not exported in tracefs and provide a
 * mechanism for vendor modules to hook and extend functionality
 */

struct irq_domain;
struct irq_affinity_desc;
DECLARE_HCK_VH(lpcpu_gic_smp_register_sgis,
	       TP_PROTO(struct irq_domain *domain, int irq_base,
			unsigned int nr_irqs, int node, void *arg,
			bool realloc_flag, const struct irq_affinity_desc *affinity,
			int *hook_flag, int *base_sgi),
	       TP_ARGS(domain, irq_base, nr_irqs, node, arg, realloc_flag, affinity,
		       hook_flag, base_sgi));

DECLARE_HCK_VH(lpcpu_gic_smp_ipi_range,
	       TP_PROTO(int ipi_base, int nr_ipi, int *hook_flag),
	       TP_ARGS(ipi_base, nr_ipi, hook_flag));

void __init hck_lpcpu_gic_register(void);
#endif /* __HCK_LPCPU_GIC__ */
