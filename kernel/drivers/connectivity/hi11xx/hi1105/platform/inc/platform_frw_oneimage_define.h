/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description:platform_frw_oneimage_define.h header file
 * Author: @CompanyNameTag
 */

#ifndef PLATFORM_FRW_ONEIMAGE_DEFINE_H
#define PLATFORM_FRW_ONEIMAGE_DEFINE_H

/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define plat_oneimage_rename(NAME)  NAME##_mp

#define GT_FRW_CONTEXT              (&g_frw_context_gt)
#define WIFI_FRW_CONTEXT            (&g_frw_context_wifi)

#if defined(_PRE_MULTI_INSTANCE_GT)
#define CUR_FRW_CONTEXT     GT_FRW_CONTEXT
#elif defined(_PRE_MULTI_INSTANCE_WIFI)
#define CUR_FRW_CONTEXT     WIFI_FRW_CONTEXT
#endif

#if defined(_PRE_MULTI_INSTANCE_GT)
#define g_need_restart                          g_need_restart_gt
#define g_st_timer_pause                        g_st_timer_pause_gt
#define g_event_manager                         g_event_manager_gt
#define g_event_table                           g_event_table_gt
#define g_st_event_task                         g_st_event_task_gt
#define g_event_task_lock                       g_event_task_lock_gt
#elif defined(_PRE_MULTI_INSTANCE_WIFI)
#define g_need_restart                          g_need_restart_wifi
#define g_st_timer_pause                        g_st_timer_pause_wifi
#define g_event_manager                         g_event_manager_wifi
#define g_event_table                           g_event_table_wifi
#define g_st_event_task                         g_st_event_task_wifi
#define g_event_task_lock                       g_event_task_lock_wifi
#endif

#define frw_set_init_state(...)                 frw_set_init_state_ctx(__VA_ARGS__, CUR_FRW_CONTEXT)
#define frw_get_init_state()                    frw_get_init_state_ctx(CUR_FRW_CONTEXT)

#define frw_timer_add_timer(...)                frw_timer_add_timer_ctx(__VA_ARGS__, CUR_FRW_CONTEXT)
#define frw_timer_clean_timer(...)              frw_timer_clean_timer_ctx(__VA_ARGS__, CUR_FRW_CONTEXT)
#define frw_timer_create_timer(...)             frw_timer_create_timer_ctx(__VA_ARGS__, CUR_FRW_CONTEXT)
#define frw_timer_delete_all_timer(...)         frw_timer_delete_all_timer_ctx(CUR_FRW_CONTEXT)
#define frw_timer_exit(...)                     frw_timer_exit_ctx(__VA_ARGS__, CUR_FRW_CONTEXT)
#define frw_timer_immediate_destroy_timer(...)  frw_timer_immediate_destroy_timer_ctx(__VA_ARGS__, CUR_FRW_CONTEXT)
#define frw_timer_init(...)                     frw_timer_init_ctx(__VA_ARGS__, CUR_FRW_CONTEXT)
#define frw_timer_restart(...)                  frw_timer_restart_ctx(__VA_ARGS__, CUR_FRW_CONTEXT)
#define frw_timer_restart_timer(...)            frw_timer_restart_timer_ctx(__VA_ARGS__, CUR_FRW_CONTEXT)
#define frw_timer_stop(...)                     frw_timer_stop_ctx(__VA_ARGS__, CUR_FRW_CONTEXT)
#define frw_timer_stop_timer(...)               frw_timer_stop_timer_ctx(__VA_ARGS__, CUR_FRW_CONTEXT)

#define frw_task_exit(...)                      frw_task_exit_ctx(CUR_FRW_CONTEXT)
#define frw_task_init(...)                      frw_task_init_ctx(CUR_FRW_CONTEXT)
#define frw_task_event_handler_register(...)    frw_task_event_handler_register_ctx(__VA_ARGS__, CUR_FRW_CONTEXT)
#define frw_task_get_state(...)                 frw_task_get_state_ctx_ctx(__VA_ARGS__, CUR_FRW_CONTEXT)
#define frw_task_sched(...)                     frw_task_sched_ctx(__VA_ARGS__, CUR_FRW_CONTEXT)
#define frw_task_set_state(...)                 frw_task_set_state_ctx(__VA_ARGS__, CUR_FRW_CONTEXT)

#define frw_event_exit(...)                     frw_event_exit_ctx(CUR_FRW_CONTEXT)
#define frw_event_init(...)                     frw_event_init_ctx(CUR_FRW_CONTEXT)
#define frw_event_dispatch_event(...)           frw_event_dispatch_event_ctx(__VA_ARGS__, CUR_FRW_CONTEXT)
#define frw_event_post_event(...)               frw_event_post_event_ctx(__VA_ARGS__, CUR_FRW_CONTEXT)
#define frw_event_process_all_event(...)        frw_event_process_all_event_ctx(__VA_ARGS__, CUR_FRW_CONTEXT)
#define frw_event_flush_event_queue(...)        frw_event_flush_event_queue_ctx(__VA_ARGS__, CUR_FRW_CONTEXT)
#define frw_event_get_sched_queue(...)          frw_event_get_sched_queue_ctx(__VA_ARGS__, CUR_FRW_CONTEXT)
#define frw_event_queue_info(...)               frw_event_queue_info_ctx(CUR_FRW_CONTEXT)
#define frw_event_table_register(...)           frw_event_table_register_ctx(__VA_ARGS__, CUR_FRW_CONTEXT)
#define frw_event_vap_flush_event(...)          frw_event_vap_flush_event_ctx(__VA_ARGS__, CUR_FRW_CONTEXT)
#define frw_event_vap_pause_event(...)          frw_event_vap_pause_event_ctx(__VA_ARGS__, CUR_FRW_CONTEXT)
#define frw_event_vap_resume_event(...)         frw_event_vap_resume_event_ctx(__VA_ARGS__, CUR_FRW_CONTEXT)
#define frw_event_task_lock(...)                frw_event_task_lock_ctx(CUR_FRW_CONTEXT)
#define frw_event_task_unlock(...)              frw_event_task_unlock_ctx(CUR_FRW_CONTEXT)
#define frw_event_process_all_event_when_wlan_close(...) \
        frw_event_process_all_event_when_wlan_close_ctx(CUR_FRW_CONTEXT)
#define frw_event_queue_destroy(...)            frw_event_queue_destroy_ctx(__VA_ARGS__, CUR_FRW_CONTEXT)
#define frw_event_queue_init(...)               frw_event_queue_init_ctx(CUR_FRW_CONTEXT)

#define oal_mem_end_stat(...)                   oal_mem_end_stat_ctx(CUR_OAL_MEM_CONTEXT)
#define oal_mem_start_stat(...)                 oal_mem_start_stat_ctx(CUR_OAL_MEM_CONTEXT)

#define frw_event_deploy_exit                   plat_oneimage_rename(frw_event_deploy_exit)
#define frw_event_deploy_init                   plat_oneimage_rename(frw_event_deploy_init)
#define frw_event_deploy_register               plat_oneimage_rename(frw_event_deploy_register)
#define frw_event_dump_event                    plat_oneimage_rename(frw_event_dump_event)
#define frw_event_fsm_register                  plat_oneimage_rename(frw_event_fsm_register)
#define frw_event_ipc_event_queue_empty_register    plat_oneimage_rename(frw_event_ipc_event_queue_empty_register)
#define frw_event_ipc_event_queue_full_register     plat_oneimage_rename(frw_event_ipc_event_queue_full_register)
#define frw_event_queue_dequeue                 plat_oneimage_rename(frw_event_queue_dequeue)
#define frw_event_queue_enqueue                 plat_oneimage_rename(frw_event_queue_enqueue)
#define frw_event_sched_activate_queue          plat_oneimage_rename(frw_event_sched_activate_queue)
#define frw_event_sched_activate_queue_no_lock  plat_oneimage_rename(frw_event_sched_activate_queue_no_lock)
#define frw_event_sched_deactivate_queue        plat_oneimage_rename(frw_event_sched_deactivate_queue)
#define frw_event_sched_deactivate_queue_no_lock    plat_oneimage_rename(frw_event_sched_deactivate_queue_no_lock)
#define frw_event_sched_init                    plat_oneimage_rename(frw_event_sched_init)
#define frw_event_sched_pause_queue             plat_oneimage_rename(frw_event_sched_pause_queue)
#define frw_event_sched_pick_next_event_queue_wrr   plat_oneimage_rename(frw_event_sched_pick_next_event_queue_wrr)
#define frw_event_sched_resume_queue            plat_oneimage_rename(frw_event_sched_resume_queue)
#define frw_event_sub_rx_adapt_table_init       plat_oneimage_rename(frw_event_sub_rx_adapt_table_init)
#define frw_ipc_log_exit                        plat_oneimage_rename(frw_ipc_log_exit)
#define frw_ipc_log_init                        plat_oneimage_rename(frw_ipc_log_init)
#define frw_ipc_log_recv_alarm                  plat_oneimage_rename(frw_ipc_log_recv_alarm)
#define frw_ipc_log_recv                        plat_oneimage_rename(frw_ipc_log_recv)
#define frw_ipc_log_rx_print                    plat_oneimage_rename(frw_ipc_log_rx_print)
#define frw_ipc_log_send_alarm                  plat_oneimage_rename(frw_ipc_log_send_alarm)
#define frw_ipc_log_send                        plat_oneimage_rename(frw_ipc_log_send)
#define frw_ipc_log_tx_print                    plat_oneimage_rename(frw_ipc_log_tx_print)
#define frw_ipc_msg_queue_destroy               plat_oneimage_rename(frw_ipc_msg_queue_destroy)
#define frw_ipc_msg_queue_init                  plat_oneimage_rename(frw_ipc_msg_queue_init)
#define frw_ipc_msg_queue_recv                  plat_oneimage_rename(frw_ipc_msg_queue_recv)
#define frw_ipc_msg_queue_register_callback     plat_oneimage_rename(frw_ipc_msg_queue_register_callback)
#define frw_ipc_msg_queue_send                  plat_oneimage_rename(frw_ipc_msg_queue_send)
#define frw_ipc_reset                           plat_oneimage_rename(frw_ipc_reset)
#define frw_ipc_smp_dispatch                    plat_oneimage_rename(frw_ipc_smp_dispatch)
#define frw_ipc_smp_ordering                    plat_oneimage_rename(frw_ipc_smp_ordering)
#define frw_is_event_queue_empty                plat_oneimage_rename(frw_is_event_queue_empty)
#define frw_is_vap_event_queue_empty            plat_oneimage_rename(frw_is_vap_event_queue_empty)
#define frw_main_exit                           plat_oneimage_rename(frw_main_exit)
#define frw_main_init                           plat_oneimage_rename(frw_main_init)
#define frw_task_thread_condition_check         plat_oneimage_rename(frw_task_thread_condition_check)

#endif /* end of platform_oneimage_define.h */
