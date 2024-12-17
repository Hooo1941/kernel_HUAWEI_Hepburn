// SPDX-License-Identifier: GPL-2.0-only
/* endor_hooks.c
 *
 * HiSilicon Common Kernel Vendor Hook Support
 *
 * Copyright 2022 Huawei
 */

#define CREATE_HCK_VENDOR_HOOK
#include <platform_include/basicplatform/linux/hck/amba/hck_amba.h>
#include <platform_include/basicplatform/linux/hck/clk/hck_clk.h>
#include <platform_include/basicplatform/linux/hck/regulator/hck_regulator.h>
#include <platform_include/basicplatform/linux/hck/mntn/hck_dfx_kdump.h>
#include <platform_include/basicplatform/linux/hck/mntn/hck_dfx_pstore.h>
#include <platform_include/basicplatform/linux/hck/mntn/hck_dfx_wdt.h>
#include <platform_include/basicplatform/linux/hck/mntn/hck_dfx_bbox.h>
#include <platform_include/basicplatform/linux/hck/mntn/hck_dfx_poweroff.h>
#include <platform_include/basicplatform/linux/hck/mntn/hck_dfx_coresight.h>
#include <platform_include/basicplatform/linux/hck/i2c/hck_i2c.h>
#include <platform_include/basicplatform/linux/hck/rtc/hck_time_sync.h>
#include <platform_include/basicplatform/linux/hck/gpio/hck_gpio.h>
#include <platform_include/basicplatform/linux/hck/pinctrl/hck_pinctrl.h>
#include <platform_include/basicplatform/linux/hck/pcie/hck_pcie.h>
#include <platform_include/basicplatform/linux/hck/fs/hck_hyperframe.h>
#include <platform_include/basicplatform/linux/hck/blk/hck_mas_ufs_gear_ctrl.h>
#include <platform_include/basicplatform/linux/hck/ufs/hck_scsi_ufs_gear_ctrl.h>
#include <platform_include/basicplatform/linux/hck/ufs/hck_ufs_mas_mcq.h>

#undef CREATE_HCK_VENDOR_HOOK

/* lite vendor hook */
#define CREATE_LITE_VENDOR_HOOK
/* add your lite vendor hook header file here */
#include <platform_include/basicplatform/linux/hck/security/lite_hck_xpm.h>
#include <platform_include/basicplatform/linux/hck/security/lite_hck_ced.h>
#include <platform_include/basicplatform/linux/hck/security/lite_hck_code_decrypt.h>
#include <platform_include/basicplatform/linux/hck/inet/lite_hck_inet.h>
#include <platform_include/basicplatform/linux/hck/fs/lite_hck_code_sign.h>
#include <platform_include/basicplatform/linux/hck/security/lite_hck_hideaddr.h>
#include <platform_include/basicplatform/linux/hck/security/lite_hck_high_randomize.h>
#include <platform_include/basicplatform/linux/hck/security/lite_hck_jit_memory.h>
#undef CREATE_LITE_VENDOR_HOOK

#define CREATE_TRACE_POINTS
#include <trace/hooks/hck_hook/hck_vendor_hooks.h>
#include <trace/hooks/hck_hook/hck_usb.h>
#include <trace/hooks/hck_hook/hck_ufshcd.h>
#ifdef CONFIG_USB_AUDIO_DSP
#include <trace/hooks/hck_hook/hck_usb_audio.h>
#endif
#ifdef CONFIG_BLK_DEV_RAM_EMULATION
#include <trace/hooks/hck_hook/hck_block_brd_emu.h>
#endif
/*
 * Export tracepoints that act as a bare tracehook (ie: have no trace event
 * associated with them) to allow external modules to probe them.
 */

/* usb */
EXPORT_TRACEPOINT_SYMBOL_GPL(hck_vh_dwc3_get_quirks);
EXPORT_TRACEPOINT_SYMBOL_GPL(hck_vh_dwc3_global_quirks);
EXPORT_TRACEPOINT_SYMBOL_GPL(hck_vh_dwc3_device_quirks);
EXPORT_TRACEPOINT_SYMBOL_GPL(hck_vh_dwc3_host_quirks);

/* ufs */
EXPORT_TRACEPOINT_SYMBOL_GPL(hck_vh_hufs_queuecommand);
#ifdef CONFIG_USB_AUDIO_DSP
EXPORT_TRACEPOINT_SYMBOL_GPL(hck_vh_usb_audio_connect);
EXPORT_TRACEPOINT_SYMBOL_GPL(hck_vh_usb_audio_set_chip);
EXPORT_TRACEPOINT_SYMBOL_GPL(hck_vh_usb_audio_check_auto_suspend);
EXPORT_TRACEPOINT_SYMBOL_GPL(hck_vh_usb_audio_disconnect);
EXPORT_TRACEPOINT_SYMBOL_GPL(hck_vh_usb_audio_ctrl_wake_up);
#endif
#ifdef CONFIG_BLK_DEV_RAM_EMULATION
EXPORT_TRACEPOINT_SYMBOL_GPL(hck_vh_block_brd_emu_create);
EXPORT_TRACEPOINT_SYMBOL_GPL(hck_vh_block_brd_emu_do_bvec);
#endif

