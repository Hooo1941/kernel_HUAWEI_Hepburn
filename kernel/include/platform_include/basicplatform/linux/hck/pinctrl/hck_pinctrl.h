#ifndef __HCK_PINCTRL__
#define __HCK_PINCTRL__
#include <linux/types.h>
#include <platform_include/basicplatform/linux/hck_vendor_hooks.h>
/*
 * Following tracepoints are not exported in tracefs and provide a
 * mechanism for vendor modules to hook and extend functionality
 */
DECLARE_HCK_VH(hi6502_dt_to_map,
	TP_PROTO(struct device_node *np_config, int *ret),
	TP_ARGS(np_config, ret));

#ifdef CONFIG_PINCRTL_VIRTUAL
DECLARE_HCK_VH(pinctrl_vm_set_mux,
	TP_PROTO(size_t reg, size_t val),
	TP_ARGS(reg, val));
#endif

#endif /* end of hck pinctrl */
