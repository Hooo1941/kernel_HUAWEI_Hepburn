#ifndef __HCK_DFX_PSTORE__
#define __HCK_DFX_PSTORE__

#include <linux/types.h>
#include <platform_include/basicplatform/linux/hck_vendor_hooks.h>
/*
 * Following tracepoints are not exported in tracefs and provide a
 * mechanism for vendor modules to hook and extend functionality
 */

DECLARE_HCK_VH(hck_dfx_pstore_save_log,
	TP_PROTO(const char *name, const void *data, size_t size),
	TP_ARGS(name, data, size));

DECLARE_HCK_VH(hck_dfx_pstore_registe_info_to_mntndump,
	TP_PROTO(char *big_oops_buf, size_t big_oops_buf_sz),
	TP_ARGS(big_oops_buf, big_oops_buf_sz));

struct platform_device;
struct ramoops_platform_data;
DECLARE_HCK_VH(hck_dfx_pstore_ramoops_parse_hdt,
	TP_PROTO(struct platform_device *pdev, struct ramoops_platform_data *pdata, int *ret, int *err),
	TP_ARGS(pdev, pdata, ret, err));

#endif /* __HCK_DFX_PSTORE__ */
