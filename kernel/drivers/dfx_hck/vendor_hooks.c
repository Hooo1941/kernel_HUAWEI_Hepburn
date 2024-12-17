// SPDX-License-Identifier: GPL-2.0-only
/* endor_hooks.c
 *
 * HiSilicon Common Kernel Vendor Hook Support
 *
 * Copyright 2022 Huawei
 */

#define CREATE_HCK_VENDOR_HOOK

#undef CREATE_HCK_VENDOR_HOOK

/* lite vendor hook */
#define CREATE_LITE_VENDOR_HOOK
/* add your lite vendor hook header file here */

#undef CREATE_LITE_VENDOR_HOOK

#define CREATE_TRACE_POINTS
#include <platform/trace/hooks/memcheck.h>
#include <platform/trace/hooks/hungtask.h>
#include <platform/trace/hooks/zerohung.h>
#include <platform/trace/hooks/rainbow.h>

/*
 * Export tracepoints that act as a bare tracehook (ie: have no trace event
 * associated with them) to allow external modules to probe them.
 */

/* dfx */
#if defined(CONFIG_ANDROID_VENDOR_HOOKS)
/*  for qcom platform */
EXPORT_TRACEPOINT_SYMBOL_GPL(android_mm_mem_stats_show);
EXPORT_TRACEPOINT_SYMBOL_GPL(android_mm_memcheck_gpumem_info_show);
EXPORT_TRACEPOINT_SYMBOL_GPL(android_mm_memcheck_gpumem_get_usage);
EXPORT_TRACEPOINT_SYMBOL_GPL(android_gpu_get_total_used);
EXPORT_TRACEPOINT_SYMBOL_GPL(android_cma_report);
EXPORT_TRACEPOINT_SYMBOL_GPL(android_slub_obj_report);
EXPORT_TRACEPOINT_SYMBOL_GPL(android_lowmem_report);

EXPORT_TRACEPOINT_SYMBOL_GPL(android_set_did_panic);
EXPORT_TRACEPOINT_SYMBOL_GPL(android_set_timeout_secs);
EXPORT_TRACEPOINT_SYMBOL_GPL(android_check_tasks);
EXPORT_TRACEPOINT_SYMBOL_GPL(android_mmap_sem_debug);

EXPORT_TRACEPOINT_SYMBOL_GPL(android_hung_wp_screen_getbl);
EXPORT_TRACEPOINT_SYMBOL_GPL(android_hung_wp_screen_setbl);
EXPORT_TRACEPOINT_SYMBOL_GPL(android_hung_wp_screen_qcom_pkey_press);
EXPORT_TRACEPOINT_SYMBOL_GPL(android_hung_wp_screen_powerkey_ncb);
EXPORT_TRACEPOINT_SYMBOL_GPL(android_dfx_watchdog_check_hung);
EXPORT_TRACEPOINT_SYMBOL_GPL(android_dfx_watchdog_lockup_init);
EXPORT_TRACEPOINT_SYMBOL_GPL(android_dfx_watchdog_lockup_init_work);

EXPORT_TRACEPOINT_SYMBOL_GPL(android_rb_mreason_set);
EXPORT_TRACEPOINT_SYMBOL_GPL(android_rb_sreason_set);
EXPORT_TRACEPOINT_SYMBOL_GPL(android_rb_kallsyms_set);
EXPORT_TRACEPOINT_SYMBOL_GPL(android_rb_trace_irq_write);
EXPORT_TRACEPOINT_SYMBOL_GPL(android_rb_trace_task_write);
EXPORT_TRACEPOINT_SYMBOL_GPL(android_cmd_himntn_item_switch);

#else
/* for hisi platform */
EXPORT_TRACEPOINT_SYMBOL_GPL(hck_mm_mem_stats_show);
EXPORT_TRACEPOINT_SYMBOL_GPL(hck_cma_report);
EXPORT_TRACEPOINT_SYMBOL_GPL(hck_slub_obj_report);
EXPORT_TRACEPOINT_SYMBOL_GPL(hck_lowmem_report);

EXPORT_TRACEPOINT_SYMBOL_GPL(hck_set_did_panic);
EXPORT_TRACEPOINT_SYMBOL_GPL(hck_set_timeout_secs);
EXPORT_TRACEPOINT_SYMBOL_GPL(hck_check_tasks);
EXPORT_TRACEPOINT_SYMBOL_GPL(hck_mmap_sem_debug);

EXPORT_TRACEPOINT_SYMBOL_GPL(hck_hung_wp_screen_fake_power_off);
EXPORT_TRACEPOINT_SYMBOL_GPL(hck_hung_wp_screen_getbl);
EXPORT_TRACEPOINT_SYMBOL_GPL(hck_hung_wp_screen_setbl);
EXPORT_TRACEPOINT_SYMBOL_GPL(hck_hung_wp_screen_qcom_pkey_press);
EXPORT_TRACEPOINT_SYMBOL_GPL(hck_hung_wp_screen_powerkey_ncb);
EXPORT_TRACEPOINT_SYMBOL_GPL(hck_dfx_watchdog_check_hung);
EXPORT_TRACEPOINT_SYMBOL_GPL(hck_dfx_watchdog_lockup_init);
EXPORT_TRACEPOINT_SYMBOL_GPL(hck_dfx_watchdog_lockup_init_work);

EXPORT_TRACEPOINT_SYMBOL_GPL(hck_rb_mreason_set);
EXPORT_TRACEPOINT_SYMBOL_GPL(hck_rb_sreason_set);
EXPORT_TRACEPOINT_SYMBOL_GPL(hck_rb_kallsyms_set);
EXPORT_TRACEPOINT_SYMBOL_GPL(hck_rb_trace_irq_write);
EXPORT_TRACEPOINT_SYMBOL_GPL(hck_rb_trace_task_write);
EXPORT_TRACEPOINT_SYMBOL_GPL(hck_cmd_himntn_item_switch);
#endif
