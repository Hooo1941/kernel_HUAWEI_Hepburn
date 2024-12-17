/* SPDX-License-Identifier: GPL-2.0 */
#undef TRACE_SYSTEM
#define TRACE_SYSTEM hck_usb
#define TRACE_INCLUDE_PATH trace/hooks/hck_hook
#if !defined(_TRACE_HCK_HOOK_USB_H) || defined(TRACE_HEADER_MULTI_READ)
#define _TRACE_HCK_HOOK_USB_H
#include <linux/tracepoint.h>
#include <trace/hooks/hck_hook/hck_vendor_hooks.h>

/*
 * Following tracepoints are not exported in tracefs and provide a
 * mechanism for vendor modules to hook and extend functionality
 */

struct dwc3;

DECLARE_HCK_HOOK(hck_vh_dwc3_get_quirks,
		TP_PROTO(struct dwc3 *dwc),
		TP_ARGS(dwc));

DECLARE_HCK_HOOK(hck_vh_dwc3_global_quirks,
		TP_PROTO(struct dwc3 *dwc),
		TP_ARGS(dwc));

DECLARE_HCK_HOOK(hck_vh_dwc3_device_quirks,
		TP_PROTO(struct dwc3 *dwc),
		TP_ARGS(dwc));

DECLARE_HCK_HOOK(hck_vh_dwc3_host_quirks,
		TP_PROTO(struct dwc3 *dwc),
		TP_ARGS(dwc));

#endif /* _TRACE_HCK_HOOK_USB_H */
/* This part must be outside protection */
#include <trace/define_trace.h>

