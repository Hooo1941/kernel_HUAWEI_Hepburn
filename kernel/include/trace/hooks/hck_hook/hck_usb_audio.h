/* SPDX-License-Identifier: GPL-2.0 */
#undef TRACE_SYSTEM
#define TRACE_SYSTEM hck_usb_audio
#define TRACE_INCLUDE_PATH trace/hooks/hck_hook
#if !defined(_TRACE_HCK_HOOK_USB_AUDIO_H) || defined(TRACE_HEADER_MULTI_READ)
#define _TRACE_HCK_HOOK_USB_AUDIO_H
#include <linux/tracepoint.h>
#include <trace/hooks/hck_hook/hck_vendor_hooks.h>
/*
 * Following tracepoints are not exported in tracefs and provide a
 * mechanism for vendor modules to hook and extend functionality
 */
struct usb_device;
struct snd_usb_audio;
struct usb_interface;

DECLARE_HCK_RESTRICTED_HOOK(hck_vh_usb_audio_connect,
	TP_PROTO(struct usb_interface *intf, bool *switch_result),
	TP_ARGS(intf, switch_result), 1);

DECLARE_HCK_RESTRICTED_HOOK(hck_vh_usb_audio_set_chip,
	TP_PROTO(struct snd_usb_audio *chip),
	TP_ARGS(chip), 1);

DECLARE_HCK_RESTRICTED_HOOK(hck_vh_usb_audio_check_auto_suspend,
	TP_PROTO(struct usb_device *dev),
	TP_ARGS(dev), 1);

DECLARE_HCK_RESTRICTED_HOOK(hck_vh_usb_audio_disconnect,
	TP_PROTO(struct usb_interface *intf),
	TP_ARGS(intf), 1);

DECLARE_HCK_RESTRICTED_HOOK(hck_vh_usb_audio_ctrl_wake_up,
	TP_PROTO(bool wake_up),
	TP_ARGS(wake_up), 1);

#endif /* _TRACE_HCK_HOOK_PCIE_H */
/* This part must be outside protection */
#include <trace/define_trace.h>
