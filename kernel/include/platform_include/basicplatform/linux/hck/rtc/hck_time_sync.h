#ifndef __HCK_TIME_SYNC__
#define __HCK_TIME_SYNC__

#include <linux/types.h>
#include <platform_include/basicplatform/linux/hck_vendor_hooks.h>
/*
 * Following tracepoints are not exported in tracefs and provide a
 * mechanism for vendor modules to hook and extend functionality
 */
DECLARE_HCK_VH(rtc_time_sync,
	TP_PROTO(uint64_t times),
	TP_ARGS(times));

/* This part must be outside protection */

#endif /* end of hck time sync */
