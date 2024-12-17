/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : MinTP测试
 * 作    者 : wifi
 * 创建日期 : 2023年1月7日
 */

#include "hmac_mintp_test.h"
#include "hmac_mintp_log.h"
#include "mac_mib.h"
#include "oam_event_wifi.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_MINTP_TEST_C

typedef struct {
    uint8_t inited;
    uint16_t loop_cnt;
    uint16_t netbuf_cnt;
    hmac_user_stru *hmac_user;
    hmac_vap_stru *hmac_vap;
    struct task_struct *test_thread;
} hmac_mintp_test_stru;

hmac_mintp_test_stru g_mintp_test = { 0 };

static void hmac_mintp_test_add_netbuf_to_list(oal_netbuf_stru **list, oal_netbuf_stru **tail, oal_netbuf_stru *next)
{
    if (*list) {
        hmac_mintp_tx_set_next_netbuf(*tail, next);
    } else {
        *list = next;
    }

    *tail = next;
    hmac_mintp_tx_set_next_netbuf(*tail, NULL);
}

#define HMAC_MINTP_TEST_NETBUF_DATA_LEN 1500
#define HMAC_MINTP_TEST_NETBUF_ALLOC_LEN (HMAC_MINTP_TEST_NETBUF_DATA_LEN + 128)
static void hmac_mintp_test_netbuf_init(oal_netbuf_stru *netbuf)
{
    mac_ether_header_stru *mac_header = (mac_ether_header_stru *)oal_netbuf_data(netbuf);

    oal_set_mac_addr(mac_header->auc_ether_dhost, g_mintp_test.hmac_user->st_user_base_info.auc_user_mac_addr);
    oal_set_mac_addr(mac_header->auc_ether_shost, mac_mib_get_StationID(&g_mintp_test.hmac_vap->st_vap_base_info));
    mac_header->us_ether_type = htons(ETH_P_D2D);

    oal_netbuf_put(netbuf, HMAC_MINTP_TEST_NETBUF_DATA_LEN);
    netbuf->protocol = htons(ETH_P_D2D);
}

oal_netbuf_stru *hmac_mintp_test_build_netbuf_list(void)
{
    oal_netbuf_stru *list = NULL;
    oal_netbuf_stru *tail = NULL;
    oal_netbuf_stru *next = NULL;
    uint16_t netbuf_cnt;

    for (netbuf_cnt = 0; netbuf_cnt < g_mintp_test.netbuf_cnt; netbuf_cnt++) {
        next = (oal_netbuf_stru *)oal_mem_netbuf_alloc(OAL_NORMAL_NETBUF,
                                                       HMAC_MINTP_TEST_NETBUF_ALLOC_LEN, OAL_NETBUF_PRIORITY_MID);
        if (!next) {
            hmac_free_netbuf_list(list);
            return NULL;
        }

        hmac_mintp_test_netbuf_init(next);
        hmac_mintp_test_add_netbuf_to_list(&list, &tail, next);
    }

    return list;
}

static int hmac_mintp_test_thread(void *data)
{
    oal_netbuf_stru *list = NULL;
    uint16_t loop_cnt = 0;

    while (loop_cnt++ < g_mintp_test.loop_cnt && !oal_kthread_should_stop()) {
        list = hmac_mintp_test_build_netbuf_list();
        if (!list) {
            oam_error_log0(0, 0, "{hmac_mintp_test_thread::netbuf_list NULL}");
            break;
        }

        hmac_mintp_log((uint8_t *)list, MINTP_LOG_LVL_1, MINTP_LOG_DATATYPE_SKB_HEAD_LIST, MINTP_LOG_TYPE_BUILD_LIST);
        hmac_bridge_vap_xmit_mintp(list, g_mintp_test.hmac_vap->pst_net_device);
        oal_msleep(2); /* sleep 2ms */
    }

    g_mintp_test.test_thread = NULL;
    oam_warning_log0(0, 0, "{hmac_mintp_test_thread::test finished}");

    return 0;
}

uint32_t hmac_config_mintp_test_start(void)
{
    if (!g_mintp_test.inited) {
        return OAL_FAIL;
    }

    if (g_mintp_test.test_thread) {
        return OAL_FAIL;
    }

    g_mintp_test.test_thread = oal_thread_create(hmac_mintp_test_thread, NULL, NULL, "hisi_mintp_test", SCHED_FIFO,
                                                 99,  // 99: priority
                                                 -1); // -1: cpuid(not bind)
    if (!g_mintp_test.test_thread) {
        return OAL_FAIL;
    }

    oam_warning_log0(0, 0, "{hmac_config_mintp_test_start::start test thread succ!}");

    return OAL_SUCC;
}

uint32_t hmac_config_mintp_test_stop(void)
{
    if (!g_mintp_test.inited) {
        return OAL_FAIL;
    }

    if (!g_mintp_test.test_thread) {
        return OAL_FAIL;
    }

    oal_thread_stop(g_mintp_test.test_thread, NULL);
    g_mintp_test.test_thread = NULL;

    g_mintp_test.inited = OAL_FALSE;

    return OAL_SUCC;
}

static uint32_t hmac_config_mintp_test_cfg(mac_vap_stru *mac_vap, hmac_mintp_test_param_stru *param)
{
    uint16_t user_idx;

    if (mac_vap_find_user_by_macaddr(mac_vap, param->user_mac, &user_idx) != OAL_SUCC) {
        oam_error_log0(0, 0, "{hmac_config_mintp_test_cfg::find user by mac failed!}");
        return OAL_FAIL;
    }

    g_mintp_test.hmac_user = mac_res_get_hmac_user(user_idx);
    if (!g_mintp_test.hmac_user) {
        return OAL_FAIL;
    }

    g_mintp_test.hmac_vap = mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (!g_mintp_test.hmac_vap) {
        return OAL_FAIL;
    }

    g_mintp_test.loop_cnt = param->loop_cnt;
    g_mintp_test.netbuf_cnt = param->netbuf_cnt;
    g_mintp_test.inited = OAL_TRUE;

    return OAL_SUCC;
}

uint32_t hmac_config_mintp_test(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    hmac_mintp_test_param_stru *test_param = (hmac_mintp_test_param_stru *)param;

    if (oal_any_null_ptr2(mac_vap, param)) {
        return OAL_FAIL;
    }

    hmac_mintp_log((uint8_t *)test_param, MINTP_LOG_LVL_0, MINTP_LOG_DATATYPE_CMD, MINTP_LOG_TYPE_PARSE_CMD);

    switch (test_param->cmd) {
        case HMAC_MINTP_TEST_CFG:
            return hmac_config_mintp_test_cfg(mac_vap, test_param);
        case HMAC_MINTP_TEST_START:
            return hmac_config_mintp_test_start();
        case HMAC_MINTP_TEST_STOP:
            return hmac_config_mintp_test_stop();
        default:
            return OAL_FAIL;
    }
}
