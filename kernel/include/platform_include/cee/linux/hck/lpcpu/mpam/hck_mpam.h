#ifndef __HCK_MPAM__
#define __HCK_MPAM__

#include <platform_include/basicplatform/linux/hck_vendor_hooks.h>
/*
 * Following tracepoints are not exported in tracefs and provide a
 * mechanism for vendor modules to hook and extend functionality
 */
DECLARE_HCK_VH(hck_mpam_sched_in,
	       TP_PROTO(bool hook_flag),
	       TP_ARGS(hook_flag));

#endif /* __HCK_MPAM__ */
