#ifndef __HCK_DFX_POWEROFF__
#define __HCK_DFX_POWEROFF__

#include <linux/types.h>
#include <platform_include/basicplatform/linux/hck_vendor_hooks.h>
/*
 * Following tracepoints are not exported in tracefs and provide a
 * mechanism for vendor modules to hook and extend functionality
 */

DECLARE_HCK_VH(hck_dfx_poweroff_system_reset_comm,
	TP_PROTO(const char *cmd),
	TP_ARGS(cmd));

DECLARE_HCK_VH(hck_dfx_poweroff_arm_pm_hrestart,
	TP_PROTO(int *ret, char *cmd),
	TP_ARGS(ret, cmd));
#endif /* __HCK_DFX_POWEROFF__ */
