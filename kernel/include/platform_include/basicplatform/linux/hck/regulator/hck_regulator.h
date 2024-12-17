#ifndef __HCK_REGULATOR__
#define __HCK_REGULATOR__

#include <linux/types.h>
#include <platform_include/basicplatform/linux/hck_vendor_hooks.h>

/*
 * Following tracepoints are not exported in tracefs and provide a
 * mechanism for vendor modules to hook and extend functionality
 */

struct regulator_dev;

DECLARE_HCK_VH(regulator_on_off,
	TP_PROTO(struct regulator_dev *rdev),
	TP_ARGS(rdev));

DECLARE_HCK_VH(regulator_set_vol,
	TP_PROTO(struct regulator_dev *rdev, int max_uv, int min_uv),
	TP_ARGS(rdev, max_uv, min_uv));

DECLARE_HCK_VH(regulator_set_mode,
	TP_PROTO(struct regulator_dev *rdev, u8 mode),
	TP_ARGS(rdev, mode));

#endif /* __HCK_REGULATOR__ */
