/* SPDX-License-Identifier: GPL-2.0 */
#undef TRACE_SYSTEM
#define TRACE_SYSTEM hck_block_brd_emu
#define TRACE_INCLUDE_PATH trace/hooks/hck_hook
#if !defined(_TRACE_HCK_HOOK_BLOCK_BRD_EMU_H) || defined(TRACE_HEADER_MULTI_READ)
#define _TRACE_HCK_HOOK_BLOCK_BRD_EMU_H
#include <linux/tracepoint.h>
#include <trace/hooks/hck_hook/hck_vendor_hooks.h>

/*
 * Following tracepoints are not exported in tracefs and provide a
 * mechanism for vendor modules to hook and extend functionality
 */

struct brd_device;
struct page;
struct list_head;

DECLARE_HCK_HOOK(hck_vh_block_brd_emu_create,
		TP_PROTO(struct list_head *brd_devices),
		TP_ARGS(brd_devices));

DECLARE_HCK_HOOK(hck_vh_block_brd_emu_do_bvec,
			TP_PROTO(struct brd_device *brd, struct page *page,
				   unsigned int len, unsigned int off,
				   bool is_write, sector_t sector, int *err),
			TP_ARGS(brd, page, len, off, is_write, sector, err));

#endif /* _TRACE_HCK_HOOK_BLOCK_BRD_EMU_H */
/* This part must be outside protection */
#include <trace/define_trace.h>

