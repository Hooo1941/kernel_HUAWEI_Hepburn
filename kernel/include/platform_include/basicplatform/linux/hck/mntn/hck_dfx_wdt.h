#ifndef __HCK_DFX_WDT__
#define __HCK_DFX_WDT__

#include <linux/types.h>
#include <platform_include/basicplatform/linux/hck_vendor_hooks.h>
/*
 * Following tracepoints are not exported in tracefs and provide a
 * mechanism for vendor modules to hook and extend functionality
 */

DECLARE_HCK_VH(hck_dfx_wdt_shutdown_oneshot,
	TP_PROTO(unsigned int timeout),
	TP_ARGS(timeout));

DECLARE_HCK_VH(hck_dfx_wdt_set_nmi_touch,
	TP_PROTO(bool state),
	TP_ARGS(state));

DECLARE_HCK_VH(hck_dfx_wdt_set_softlockup_panic_happen,
	TP_PROTO(bool state),
	TP_ARGS(state));

DECLARE_HCK_VH(hck_dfx_wdt_cpumask_set,
	TP_PROTO(unsigned int cpu),
	TP_ARGS(cpu));

DECLARE_HCK_VH(hck_dfx_wdt_cpumask_clear,
	TP_PROTO(unsigned int cpu),
	TP_ARGS(cpu));

DECLARE_HCK_VH(hck_dfx_wdt_cond_dump_stack,
	TP_PROTO(bool cond),
	TP_ARGS(cond));

#endif /* __HCK_DFX_WDT__ */
