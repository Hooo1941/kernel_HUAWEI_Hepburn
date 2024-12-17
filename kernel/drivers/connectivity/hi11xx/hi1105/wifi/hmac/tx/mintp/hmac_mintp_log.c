/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : MinTP日志
 * 作    者 : wifi
 * 创建日期 : 2023年1月30日
 */

#include "hmac_mintp_log.h"
#include "hmac_mintp_test.h"
#include "hmac_mintp.h"
#include "oam_event_wifi.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_MINTP_LOG_C

static inline void hmac_mintp_mac_addr_dump(uint8_t *mac_addr)
{
    oam_warning_log3(0, 0, "{hmac_mintp_mac_addr_dump::[0x%x:0x%x:XX:XX:XX:0x%x]}",
        mac_addr[BYTE_OFFSET_0], mac_addr[BYTE_OFFSET_1], mac_addr[BYTE_OFFSET_5]);
}

static void hmac_mintp_skb_dump(uint8_t *data)
{
    oal_netbuf_stru *netbuf = (oal_netbuf_stru *)data;
    mac_ether_header_stru *ether = (mac_ether_header_stru *)oal_netbuf_data(netbuf);
    uint16_t len = (uint16_t)oal_netbuf_len(netbuf);

    if (ether->us_ether_type != htons(ETH_P_D2D) && ether->us_ether_type != ntohs(ETH_P_D2D)) {
        return;
    }

    oam_warning_log2(0, 0, "{hmac_mintp_skb_dump::len[%d] ether_type[0x%x] dhost/shost}", len, ether->us_ether_type);
    hmac_mintp_mac_addr_dump(ether->auc_ether_dhost);
    hmac_mintp_mac_addr_dump(ether->auc_ether_shost);
    oam_report_eth_frame(BROADCAST_MACADDR, (uint8_t *)ether, len, OAM_OTA_FRAME_DIRECTION_TYPE_TX);
}

static void hmac_mintp_skb_next_list_dump(uint8_t *data)
{
    oal_netbuf_stru *prev = (oal_netbuf_stru *)data;
    oal_netbuf_stru *next = NULL;

    do {
        next = oal_netbuf_next(prev);
        hmac_mintp_skb_dump((uint8_t *)prev);
        prev = next;
    } while (next);
}

static void hmac_mintp_skb_head_list_dump(uint8_t *data)
{
    oal_netbuf_stru *prev = (oal_netbuf_stru *)data;
    oal_netbuf_stru *next = NULL;

    do {
        next = hmac_mintp_tx_get_next_netbuf(prev);
        hmac_mintp_skb_dump((uint8_t *)prev);
        prev = next;
    } while (next);
}

static void hmac_mintp_cmd_dump(uint8_t *data)
{
    hmac_mintp_test_param_stru *cmd = (hmac_mintp_test_param_stru *)data;

    oam_warning_log3(0, 0, "{hmac_mintp_cmd_dump::cmd[%d] loop[%d] cnt[%d]}", cmd->cmd, cmd->loop_cnt, cmd->netbuf_cnt);
    hmac_mintp_mac_addr_dump(cmd->user_mac);
}

typedef void (*hmac_mintp_log_func)(uint8_t *);
static const hmac_mintp_log_func g_mintp_log_func[MINTP_LOG_DATATYPE_BUTT] = {
    hmac_mintp_skb_dump,            /* MINTP_LOG_DATATYPE_SKB */
    hmac_mintp_skb_next_list_dump,  /* MINTP_LOG_DATATYPE_SKB_NEXT_LIST */
    hmac_mintp_skb_head_list_dump,  /* MINTP_LOG_DATATYPE_SKB_HEAD_LIST */
    hmac_mintp_cmd_dump,            /* MINTP_LOG_DATATYPE_CMD */
};

static uint8_t g_mintp_log_lvl = MINTP_LOG_LVL_DISABLE;

static void hmac_mintp_log_context_print(uint8_t type)
{
    switch (type) {
        case MINTP_LOG_TYPE_PARSE_CMD:
            oam_warning_log0(0, 0, "MINTP::test parse cmd");
            break;
        case MINTP_LOG_TYPE_BUILD_LIST:
            oam_warning_log0(0, 0, "MINTP::test build skb list");
            break;
        case MINTP_LOG_TYPE_TX:
            oam_warning_log0(0, 0, "MINTP::tx skb list");
            break;
        case MINTP_LOG_TYPE_PUSH_RING:
            oam_warning_log0(0, 0, "MINTP::skb push ring");
            break;
        case MINTP_LOG_TYPE_RX:
            oam_warning_log0(0, 0, "MINTP::rx skb");
            break;
        default:
            break;
    }
}

void hmac_mintp_log(uint8_t *data, uint8_t lvl, uint8_t datatype, uint8_t type)
{
    if (g_mintp_log_lvl < lvl || !data || datatype >= MINTP_LOG_DATATYPE_BUTT || type >= MINTP_LOG_TYPE_BUTT) {
        return;
    }

    hmac_mintp_log_context_print(type);

    if (g_mintp_log_func[datatype]) {
        g_mintp_log_func[datatype](data);
    }
}

uint32_t hmac_mintp_set_log_lvl(mac_vap_stru *mac_vap, uint32_t *params)
{
    g_mintp_log_lvl = (uint8_t)params[0];

    oam_warning_log1(0, 0, "{hmac_mintp_set_log_lvl::log lvl set to [%d]}", g_mintp_log_lvl);

    return OAL_SUCC;
}
