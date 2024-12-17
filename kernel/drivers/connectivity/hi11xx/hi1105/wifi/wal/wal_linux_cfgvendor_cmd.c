/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : Linux cfgvendor 命令
 * 作    者 : wifi1
 * 创建日期 : 2020年10月16日
 */
#include "wal_linux_cfgvendor.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_WAL_LINUX_CFGVENDOR_CMD_C

OAL_STATIC const oal_wiphy_vendor_command_stru g_wal_vendor_cmds[] = {
    {   {   .vendor_id = OUI_VENDOR,
            .subcmd = GSCAN_SUBCMD_GET_CHANNEL_LIST
        },
        .flags = WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_NETDEV,
        .doit = wal_cfgvendor_get_channel_list,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
        .policy = wal_andr_wifi_attr_policy,
        .maxattr = ANDR_WIFI_ATTRIBUTE_MAX,
#endif
    },

    {   {   .vendor_id = OUI_VENDOR,
            .subcmd = ANDR_WIFI_SET_COUNTRY
        },
        .flags = WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_NETDEV,
        .doit = wal_cfgvendor_set_country,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
        .policy = wal_andr_wifi_attr_policy,
        .maxattr = ANDR_WIFI_ATTRIBUTE_MAX,
#endif
    },
    {   {   .vendor_id = OUI_VENDOR,
            .subcmd = ANDR_WIFI_SUBCMD_GET_FEATURE_SET
        },
        .flags = WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_NETDEV,
        .doit = wal_cfgvendor_get_feature_set,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
        .policy = VENDOR_CMD_RAW_DATA,
#endif
    },
    {   {   .vendor_id = OUI_VENDOR,
            .subcmd = ANDR_WIFI_RANDOM_MAC_OUI
        },
        .flags = WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_NETDEV,
        .doit = wal_cfgvendor_set_random_mac_oui,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
        .policy = wal_gscan_attr_policy,
        .maxattr = GSCAN_ATTRIBUTE_MAX - 1,
#endif
    },
    {   {   .vendor_id = OUI_VENDOR,
            .subcmd = WIFI_SUBCMD_SET_BSSID_BLACKLIST
        },
        .flags = WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_NETDEV,
        .doit = wal_cfgvendor_set_bssid_blacklist,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
        .policy = wal_gscan_attr_policy,
        .maxattr = GSCAN_ATTRIBUTE_MAX - 1,
#endif
    },
    {   {   .vendor_id = OUI_VENDOR,
            .subcmd = WIFI_SUBCMD_FW_ROAM_POLICY
        },
        .flags = WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_NETDEV,
        .doit = wal_cfgvendor_set_roam_policy,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
        .policy = wal_gscan_attr_policy,
        .maxattr = GSCAN_ATTRIBUTE_MAX - 1,
#endif
    },
    {   {   .vendor_id = OUI_VENDOR,
            .subcmd = DEBUG_GET_FEATURE
        },
        .flags = WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_NETDEV,
        .doit = wal_cfgvendor_dbg_get_feature,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
        .policy = VENDOR_CMD_RAW_DATA,
#endif
    },
    {   {   .vendor_id = OUI_VENDOR,
            .subcmd = DEBUG_GET_VER
        },
        .flags = WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_NETDEV,
        .doit = wal_cfgvendor_dbg_get_version,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
        .policy = VENDOR_CMD_RAW_DATA,
#endif
    },
    {   {   .vendor_id = OUI_VENDOR,
            .subcmd = DEBUG_GET_RING_STATUS
        },
        .flags = WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_NETDEV,
        .doit = wal_cfgvendor_dbg_get_ring_status,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
        .policy = VENDOR_CMD_RAW_DATA,
#endif
    },
    {   {   .vendor_id = OUI_VENDOR,
            .subcmd = DEBUG_TRIGGER_MEM_DUMP
        },
        .flags = WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_NETDEV,
        .doit = wal_cfgvendor_dbg_trigger_mem_dump,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
        .policy = VENDOR_CMD_RAW_DATA,
#endif
    },
    {   {   .vendor_id = OUI_VENDOR,
            .subcmd = DEBUG_START_LOGGING
        },
        .flags = WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_NETDEV,
        .doit = wal_cfgvendor_dbg_start_logging,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
        .policy = VENDOR_CMD_RAW_DATA,
#endif
    },
    {   {   .vendor_id = OUI_VENDOR,
            .subcmd = DEBUG_GET_RING_DATA
        },
        .flags = WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_NETDEV,
        .doit = wal_cfgvendor_dbg_get_ring_data,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
        .policy = VENDOR_CMD_RAW_DATA,
#endif
    },
    {   {   .vendor_id = OUI_VENDOR,
            .subcmd = WIFI_OFFLOAD_SUBCMD_START_MKEEP_ALIVE
        },
        .flags = WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_NETDEV,
        .doit = wl_cfgvendor_start_vowifi_nat_keep_alive,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
        .policy = wal_mkeep_alive_attr_policy,
        .maxattr = MKEEP_ALIVE_ATTRIBUTE_MAX,
#endif
    },
    {   {   .vendor_id = OUI_VENDOR,
            .subcmd = WIFI_OFFLOAD_SUBCMD_STOP_MKEEP_ALIVE
        },
        .flags = WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_NETDEV,
        .doit = wl_cfgvendor_stop_vowifi_nat_keep_alive,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
        .policy = wal_mkeep_alive_attr_policy,
        .maxattr = MKEEP_ALIVE_ATTRIBUTE_MAX,
#endif
    },
    {   {   .vendor_id = OUI_VENDOR,
            .subcmd = LSTATS_SUBCMD_GET_INFO
        },
        .flags = WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_NETDEV,
        .doit = wal_cfgvendor_lstats_get_info,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
        .policy = VENDOR_CMD_RAW_DATA,
#endif
    },
#ifdef _PRE_WLAN_FEATURE_APF
    {   {   .vendor_id = OUI_VENDOR,
            .subcmd = APF_SUBCMD_GET_CAPABILITIES
        },
        .flags = WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_NETDEV,
        .doit = wal_cfgvendor_apf_get_capabilities,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
        .policy = wal_apf_attr_policy,
        .maxattr = APF_ATTRIBUTE_MAX,
#endif
    },

    {   {   .vendor_id = OUI_VENDOR,
            .subcmd = APF_SUBCMD_SET_FILTER
        },
        .flags = WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_NETDEV,
        .doit = wal_cfgvendor_apf_set_filter,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
        .policy = wal_apf_attr_policy,
        .maxattr = APF_ATTRIBUTE_MAX,
#endif
    },
#endif

#ifdef _PRE_WLAN_FEATURE_NAN
    {   {   .vendor_id = OUI_VENDOR,
            .subcmd = WIFI_SUBCMD_NAN_SET_PARAM
        },
        .flags = WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_NETDEV,
        .doit = wal_cfgvendor_nan_set_param,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
        .policy = wal_nan_attr_policy,
        .maxattr = NAN_ATTRIBUTE_MAX,
#endif
    },
    {   {   .vendor_id = OUI_VENDOR,
            .subcmd = WIFI_SUBCMD_NAN_ENABLE
        },
        .flags = WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_NETDEV,
        .doit = wal_cfgvendor_nan_enable,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
        .policy = wal_nan_attr_policy,
        .maxattr = NAN_ATTRIBUTE_MAX,
#endif
    },
    {   {   .vendor_id = OUI_VENDOR,
            .subcmd = WIFI_SUBCMD_NAN_DISABLE
        },
        .flags = WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_NETDEV,
        .doit = wal_cfgvendor_nan_disable,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
        .policy = wal_nan_attr_policy,
        .maxattr = NAN_ATTRIBUTE_MAX,
#endif
    },
    {   {   .vendor_id = OUI_VENDOR,
            .subcmd = WIFI_SUBCMD_NAN_PUBLISH_REQUEST
        },
        .flags = WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_NETDEV,
        .doit = wal_cfgvendor_nan_tx_publish,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
        .policy = wal_nan_attr_policy,
        .maxattr = NAN_ATTRIBUTE_MAX,
#endif
    },
    {   {   .vendor_id = OUI_VENDOR,
            .subcmd = WIFI_SUBCMD_NAN_CANCEL_PUBLISH_REQUEST
        },
        .flags = WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_NETDEV,
        .doit = wal_cfgvendor_nan_cancel_tx,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
        .policy = wal_nan_attr_policy,
        .maxattr = NAN_ATTRIBUTE_MAX,
#endif
    },
    {   {   .vendor_id = OUI_VENDOR,
            .subcmd = WIFI_SUBCMD_NAN_SUBSCRIBE_REQUEST
        },
        .flags = WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_NETDEV,
        .doit = wal_cfgvendor_nan_tx_subscribe,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
        .policy = wal_nan_attr_policy,
        .maxattr = NAN_ATTRIBUTE_MAX,
#endif
    },
    {   {   .vendor_id = OUI_VENDOR,
            .subcmd = WIFI_SUBCMD_NAN_SUBSCRIBE_CANCEL_REQUEST
        },
        .flags = WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_NETDEV,
        .doit = wal_cfgvendor_nan_cancel_tx,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
        .policy = wal_nan_attr_policy,
        .maxattr = NAN_ATTRIBUTE_MAX,
#endif
    },
    {   {   .vendor_id = OUI_VENDOR,
            .subcmd = WIFI_SUBCMD_NAN_FOLLOWUP_REQUEST
        },
        .flags = WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_NETDEV,
        .doit = wal_cfgvendor_nan_tx_followup,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
        .policy = wal_nan_attr_policy,
        .maxattr = NAN_ATTRIBUTE_MAX,
#endif
    },
#endif
#ifdef _PRE_WLAN_FEATURE_FTM
    {   {   .vendor_id = OUI_VENDOR,
            .subcmd = RTT_SUBCMD_SET_CONFIG
        },
        .flags = WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_NETDEV,
        .doit = wal_cfgvendor_rtt_set_config,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
        .policy = wal_rtt_attr_policy,
        .maxattr = RTT_ATTRIBUTE_MAX,
#endif
    },
#endif
#ifdef _PRE_WLAN_CHBA_MGMT
    {   {   .vendor_id = OUI_VENDOR,
            .subcmd = WIFI_SUBCMD_HID2D_CONN_NOTIFY
        },
        .flags = WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_NETDEV,
        .doit = wal_cfgvendor_chba_conn_notify,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
        .policy = wal_chba_conn_ntf_attr_policy,
        .maxattr = MAC_CHBA_CONN_NTF_ATTR_MAX,
#endif
    },
#endif

};

static const oal_nl80211_vendor_cmd_info_stru g_wal_vendor_events[] = {
    { OUI_HISI, HISI_VENDOR_EVENT_UNSPEC },
    { OUI_HISI, HISI_VENDOR_EVENT_PRIV_STR },
    { OUI_VENDOR, WIFI_SUBCMD_CHBA_READY_TO_CONN },
    [NAN_EVENT_RX] = { OUI_HISI, NAN_EVENT_RX },
    [VENDOR_RTT_COMPLETE_EVENT] = { OUI_HISI, VENDOR_RTT_COMPLETE_EVENT },
    [VENDOR_FT_CONNECT_FAIL_EVENT] = { OUI_HISI, VENDOR_FT_CONNECT_FAIL_EVENT },
    [VENDOR_11V_ROAM_SUCC_EVENT] = { OUI_HISI, VENDOR_11V_ROAM_SUCC_EVENT },
#ifdef _PRE_WLAN_FEATURE_PWL
    [VENDOR_REQ_PWL_PRE_KEY_EVENT] = { OUI_HISI, VENDOR_REQ_PWL_PRE_KEY_EVENT },
#endif
    [VENDOR_ROAM_EXTERNAL_AUTH] = { OUI_HISI, VENDOR_ROAM_EXTERNAL_AUTH },
};

void wal_cfgvendor_init(oal_wiphy_stru *wiphy)
{
    wiphy->vendor_commands = g_wal_vendor_cmds;
    wiphy->n_vendor_commands = oal_array_size(g_wal_vendor_cmds);
    wiphy->vendor_events = g_wal_vendor_events;
    wiphy->n_vendor_events = oal_array_size(g_wal_vendor_events);
}

void wal_cfgvendor_deinit(oal_wiphy_stru *wiphy)
{
    wiphy->vendor_commands = NULL;
    wiphy->vendor_events = NULL;
    wiphy->n_vendor_commands = 0;
    wiphy->n_vendor_events = 0;
}
