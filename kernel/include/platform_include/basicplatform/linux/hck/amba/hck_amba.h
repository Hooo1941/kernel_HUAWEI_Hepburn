#ifndef __HCK_AMBA__
#define __HCK_AMBA__

#include <linux/types.h>
#include <platform_include/basicplatform/linux/hck_vendor_hooks.h>
/*
 * Following tracepoints are not exported in tracefs and provide a
 * mechanism for vendor modules to hook and extend functionality
 */
struct amba_device;

DECLARE_HCK_VH(read_amba_id,
	TP_PROTO(struct amba_device *dev, u32 size, u32 *pid, u32 *cid, int *hck_ret),
	TP_ARGS(dev, size, pid, cid, hck_ret));

#endif /* end of hck amba */

