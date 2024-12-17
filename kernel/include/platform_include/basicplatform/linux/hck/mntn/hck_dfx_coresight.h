#ifndef __HCK_DFX_CORESIGHT__
#define __HCK_DFX_CORESIGHT__

#include <linux/types.h>
#include <platform_include/basicplatform/linux/hck_vendor_hooks.h>
/*
 * Following tracepoints are not exported in tracefs and provide a
 * mechanism for vendor modules to hook and extend functionality
 */

DECLARE_HCK_VH(hck_dfx_coresight_cst_disable_all,
	TP_PROTO(bool state),
	TP_ARGS(state));

#endif /* __HCK_DFX_CORESIGHT__ */
