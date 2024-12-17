/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description:platform_oam_oneimage_define.h header file
 * Author: @CompanyNameTag
 */

#ifndef PLATFORM_OAM_ONEIMAGE_DEFINE_H
#define PLATFORM_OAM_ONEIMAGE_DEFINE_H

#define PLAT_OAM_CONTEXT            (&g_oam_mng_ctx[OAM_CTX_PLAT])
#define WIFI_OAM_CONTEXT            (&g_oam_mng_ctx[OAM_CTX_WIFI])
#define GT_OAM_CONTEXT              (&g_oam_mng_ctx[OAM_CTX_GT])
#if defined(_PRE_MULTI_INSTANCE_GT)
#define CUR_OAM_CONTEXT             GT_OAM_CONTEXT
#elif defined(_PRE_MULTI_INSTANCE_WIFI)
#define CUR_OAM_CONTEXT             WIFI_OAM_CONTEXT
#else
#define CUR_OAM_CONTEXT             PLAT_OAM_CONTEXT
#endif

#define oam_subsys_register(...)                   oam_subsys_register_ctx(__VA_ARGS__, CUR_OAM_CONTEXT)
#define oam_subsys_unregister(...)                 oam_subsys_unregister_ctx(CUR_OAM_CONTEXT)
#define oam_print(...)                             oam_print_ctx(__VA_ARGS__, CUR_OAM_CONTEXT)
#define oam_set_output_type(...)                   oam_set_output_type_ctx(__VA_ARGS__, CUR_OAM_CONTEXT)
#define oam_get_output_type(...)                   oam_get_output_type_ctx(__VA_ARGS__, CUR_OAM_CONTEXT)
#define oam_log_set_global_switch(...)             oam_log_set_global_switch_ctx(__VA_ARGS__, CUR_OAM_CONTEXT)
#define oam_log_set_vap_level(...)                 oam_log_set_vap_level_ctx(__VA_ARGS__, CUR_OAM_CONTEXT)
#define oam_log_set_feature_level(...)             oam_log_set_feature_level_ctx(__VA_ARGS__, CUR_OAM_CONTEXT)

#define oam_report_eth_frame_set_switch(...)       oam_report_eth_frame_set_switch_ctx(__VA_ARGS__, CUR_OAM_CONTEXT)
#define oam_report_eth_frame_get_switch(...)       oam_report_eth_frame_get_switch_ctx(__VA_ARGS__, CUR_OAM_CONTEXT)
#define oam_report_80211_mcast_set_switch(...)     oam_report_80211_mcast_set_switch_ctx(__VA_ARGS__, CUR_OAM_CONTEXT)
#define oam_report_80211_mcast_get_switch(...)     oam_report_80211_mcast_get_switch_ctx(__VA_ARGS__, CUR_OAM_CONTEXT)
#define oam_report_80211_ucast_set_switch(...)     oam_report_80211_ucast_set_switch_ctx(__VA_ARGS__, CUR_OAM_CONTEXT)
#define oam_report_80211_ucast_get_switch(...)     oam_report_80211_ucast_get_switch_ctx(__VA_ARGS__, CUR_OAM_CONTEXT)
#define oam_report_80211_probe_set_switch(...)     oam_report_80211_probe_set_switch_ctx(__VA_ARGS__, CUR_OAM_CONTEXT)
#define oam_report_80211_probe_get_switch(...)     oam_report_80211_probe_get_switch_ctx(__VA_ARGS__, CUR_OAM_CONTEXT)
#define oam_report_dhcp_arp_set_switch(...)        oam_report_dhcp_arp_set_switch_ctx(__VA_ARGS__, CUR_OAM_CONTEXT)
#define oam_report_dhcp_arp_get_switch()           oam_report_dhcp_arp_get_switch_ctx(CUR_OAM_CONTEXT)
#define oam_ota_set_switch(...)                    oam_ota_set_switch_ctx(__VA_ARGS__, CUR_OAM_CONTEXT)
#define oam_ota_get_global_switch(...)             oam_ota_get_global_switch_ctx(__VA_ARGS__, CUR_OAM_CONTEXT)
#define oam_ota_get_switch(...)                    oam_ota_get_switch_ctx(__VA_ARGS__, CUR_OAM_CONTEXT)
#define oam_report_data_get_global_switch(...)     oam_report_data_get_global_switch_ctx(__VA_ARGS__, CUR_OAM_CONTEXT)
#define oam_ota_report(...)                        oam_ota_report_ctx(__VA_ARGS__, CUR_OAM_CONTEXT)
#define oam_report_dscr(...)                       oam_report_dscr_ctx(__VA_ARGS__, CUR_OAM_CONTEXT)
#define oam_report_eth_frame(...)                  oam_report_eth_frame_ctx(__VA_ARGS__, CUR_OAM_CONTEXT)
#define oam_report_netbuf_cb(...)                  oam_report_netbuf_cb_ctx(__VA_ARGS__, CUR_OAM_CONTEXT)
#define oam_report_dft_params(...)                 oam_report_dft_params_ctx(__VA_ARGS__, CUR_OAM_CONTEXT)
#define oam_report_set_all_switch(...)             oam_report_set_all_switch_ctx(__VA_ARGS__, CUR_OAM_CONTEXT)
#define oam_report_backtrace(...)                  oam_report_backtrace_ctx(CUR_OAM_CONTEXT)
#define oam_report_data2sdt(...)                   oam_report_data2sdt_ctx(__VA_ARGS__, CUR_OAM_CONTEXT)

#define oam_log_print0(...)                        oam_log_print0_ctx(__VA_ARGS__, CUR_OAM_CONTEXT)
#define oam_log_print1(...)                        oam_log_print1_ctx(__VA_ARGS__, CUR_OAM_CONTEXT)
#define oam_log_print2(...)                        oam_log_print2_ctx(__VA_ARGS__, CUR_OAM_CONTEXT)
#define oam_log_print3(...)                        oam_log_print3_ctx(__VA_ARGS__, CUR_OAM_CONTEXT)
#define oam_log_print4(...)                        oam_log_print4_ctx(__VA_ARGS__, CUR_OAM_CONTEXT)

#if (!defined WIN32) && (!defined _PRE_WINDOWS_SUPPORT)
#define plat_oneimage_rename(NAME)  NAME##_mp

#define g_oam_mng_ctx                              plat_oneimage_rename(g_oam_mng_ctx)
#define g_oam_sdt_func_hook                        plat_oneimage_rename(g_oam_sdt_func_hook)
#define g_oam_wal_func_hook                        plat_oneimage_rename(g_oam_wal_func_hook)
#define oam_main_exit                              plat_oneimage_rename(oam_main_exit)
#define oam_main_init                              plat_oneimage_rename(oam_main_init)
#define oam_alloc_data2sdt                         plat_oneimage_rename(oam_alloc_data2sdt)
#define oam_cfg_get_one_item                       plat_oneimage_rename(oam_cfg_get_one_item)
#define oam_cfg_read_file_to_buf                   plat_oneimage_rename(oam_cfg_read_file_to_buf)
#define oam_dump_buff_by_hex                       plat_oneimage_rename(oam_dump_buff_by_hex)
#define oam_event_init                             plat_oneimage_rename(oam_event_init)
#define oam_filter_data2sdt                        plat_oneimage_rename(oam_filter_data2sdt)
#define oam_log_console_printk                     plat_oneimage_rename(oam_log_console_printk)
#define oam_log_get_ratelimit_switch               plat_oneimage_rename(oam_log_get_ratelimit_switch)
#define oam_log_init                               plat_oneimage_rename(oam_log_init)
#define oam_log_param_init                         plat_oneimage_rename(oam_log_param_init)
#define oam_register_init_hook                     plat_oneimage_rename(oam_register_init_hook)
#define oam_rx_post_action_function                plat_oneimage_rename(oam_rx_post_action_function)
#define chr_rx_post_action_function                plat_oneimage_rename(chr_rx_post_action_function)
#define oam_sdt_func_fook_register                 plat_oneimage_rename(oam_sdt_func_fook_register)
#define oam_sdt_func_fook_unregister               plat_oneimage_rename(oam_sdt_func_fook_unregister)
#define oam_statistics_init                        plat_oneimage_rename(oam_statistics_init)
#define oam_stats_clear_stat_info                  plat_oneimage_rename(oam_stats_clear_stat_info)
#define oam_stats_report_event_queue_info_to_sdt   plat_oneimage_rename(oam_stats_report_event_queue_info_to_sdt)
#define oam_stats_report_irq_info_to_sdt           plat_oneimage_rename(oam_stats_report_irq_info_to_sdt)
#define oam_stats_report_memblock_info_to_sdt      plat_oneimage_rename(oam_stats_report_memblock_info_to_sdt)
#define oam_stats_report_mempool_info_to_sdt       plat_oneimage_rename(oam_stats_report_mempool_info_to_sdt)
#define oam_stats_report_timer_info_to_sdt         plat_oneimage_rename(oam_stats_report_timer_info_to_sdt)
#define oam_stats_report_usr_info                  plat_oneimage_rename(oam_stats_report_usr_info)
#define oam_wal_func_fook_register                 plat_oneimage_rename(oam_wal_func_fook_register)
#define oam_wal_func_fook_unregister               plat_oneimage_rename(oam_wal_func_fook_unregister)
#define oam_get_feature_id                         plat_oneimage_rename(oam_get_feature_id)
#define oam_show_feature_list                      plat_oneimage_rename(oam_show_feature_list)
#define oam_print_to_console                       plat_oneimage_rename(oam_print_to_console)
#define oam_print_to_sdt                           plat_oneimage_rename(oam_print_to_sdt)
#define oam_report_dft_params_to_sdt               plat_oneimage_rename(oam_report_dft_params_to_sdt)
#define oam_get_feature_name                       plat_oneimage_rename(oam_get_feature_name)
#define board_clk_init                             plat_oneimage_rename(board_clk_init)

#endif
#endif /* end of platform_oneimage_define.h */

