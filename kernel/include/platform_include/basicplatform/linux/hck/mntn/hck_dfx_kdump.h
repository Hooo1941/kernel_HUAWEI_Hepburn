#ifndef __HCK_DFX_KDUMP__
#define __HCK_DFX_KDUMP__

#include <linux/types.h>
#include <platform_include/basicplatform/linux/hck_vendor_hooks.h>
/*
 * Following tracepoints are not exported in tracefs and provide a
 * mechanism for vendor modules to hook and extend functionality
 */

struct page;
DECLARE_HCK_VH(hck_dfx_kdump_cma_set_page_flag,
	TP_PROTO(struct page *page, size_t cnt),
	TP_ARGS(page, cnt));

DECLARE_HCK_VH(hck_dfx_kdump_ion_set_page_flag,
	TP_PROTO(gfp_t gft_highmem, struct page *page),
	TP_ARGS(gft_highmem, page));

DECLARE_HCK_VH(hck_dfx_kdump_reserved_addr_save,
	TP_PROTO(phys_addr_t base, phys_addr_t size),
	TP_ARGS(base, size));

#endif /* __HCK_DFX_KDUMP__ */
