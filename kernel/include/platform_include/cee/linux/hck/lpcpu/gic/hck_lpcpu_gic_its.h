#ifndef __HCK_LPCPU_GIC_ITS__
#define __HCK_LPCPU_GIC_ITS__

#include <linux/types.h>
#include <platform_include/basicplatform/linux/hck_vendor_hooks.h>
/*
 * Following tracepoints are not exported in tracefs and provide a
 * mechanism for vendor modules to hook and extend functionality
 */

DECLARE_HCK_VH(lpcpu_gic_reset_prop_table_change_lpi_prop,
	       TP_PROTO(void *va, int size, int *hook_flag),
	       TP_ARGS(va, size, hook_flag));

DECLARE_HCK_VH(lpcpu_gic_its_share_ability,
	       TP_PROTO(unsigned long long *val),
	       TP_ARGS(val));

void __init hck_lpcpu_gic_its_register(void);
#endif /* __HCK_LPCPU_GIC__ */
