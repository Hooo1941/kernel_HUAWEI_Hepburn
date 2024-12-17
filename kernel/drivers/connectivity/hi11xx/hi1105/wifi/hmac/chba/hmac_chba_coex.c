/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * Description: CHBA 2.0 sync mgmt implementation
 * Author: wifi
 * Create: 2021-06-17
 */

/* 1 Í·ÎÄ¼þ°üº¬ */
#include "hmac_chba_coex.h"
#include "hmac_chba_frame.h"
#include "hmac_chba_sync.h"
#include "hmac_chba_chan_switch.h"
#include "hmac_mgmt_sta.h"
#include "hmac_resource.h"
#include "hmac_roam_alg.h"
#include "hmac_chan_mgmt.h"
#include "wlan_chip_i.h"
#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_CHBA_COEX_C
#ifdef _PRE_WLAN_CHBA_MGMT
/* STA/GO CSA、STA漫游、CHBA切信道时满足DBAC时通知上层，此函数调用处已判断CHBA共存条件并且满足DBAC */
void hmac_chba_coex_switch_chan_dbac_rpt(mac_channel_stru *mac_channel, uint8_t rpt_type)
{
    mac_chba_adjust_chan_info chan_notify = { 0 };

    /* 通知FWK因需要切信道满足DBAC，将切换信道信息上报 */
    chan_notify.report_type = HMAC_CHBA_CHAN_SWITCH_REPORT;
    chan_notify.chan_number = mac_channel->uc_chan_number;
    chan_notify.bandwidth = mac_channel->en_bandwidth;
    chan_notify.switch_type = rpt_type;
    chan_notify.status_code = 0;
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    oal_cfg80211_chba_report((uint8_t *)(&chan_notify), sizeof(chan_notify));
#endif
    oam_warning_log3(0, OAM_SF_CHBA, "hmac_chba_coex_switch_chan_dbac_rpt::already rpt, chan:%d, bw:%d, rpt_type:%d",
        mac_channel->uc_chan_number, mac_channel->en_bandwidth, rpt_type);
    return;
}

oal_bool_enum_uint8 hmac_chba_coex_chan_is_in_list(uint8_t *coex_chan_lists,
    uint8_t chan_idx, uint8_t chan_number)
{
    uint8_t idx;
    for (idx = 0; idx < chan_idx; idx++) {
        if (chan_number == coex_chan_lists[idx]) {
            return OAL_TRUE;
        }
    }
    return OAL_FALSE;
}
int32_t wal_ioctl_chba_set_coex_cmd_para_check(mac_chba_set_coex_chan_info *cfg_coex_chan_info)
{
    if (cfg_coex_chan_info->cfg_cmd_type >= HMAC_CHBA_COEX_CFG_TYPE_BUTT) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_ioctl_chba_set_coex:cfg_cmd_type invalid.}");
        return -OAL_EFAIL;
    }
    if (cfg_coex_chan_info->supported_channel_cnt > WLAN_5G_CHANNEL_NUM) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_ioctl_chba_set_coex:supported_channel_cnt invalid.}");
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}
int32_t wal_ioctl_chba_set_coex_start_check(oal_net_device_stru *net_dev)
{
    mac_vap_stru *mac_vap = oal_net_dev_priv(net_dev);
    hmac_vap_stru *hmac_vap = NULL;

    if (mac_vap == NULL) {
        return -OAL_EFAIL;
    }
    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_vap == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, 0, "{wal_ioctl_chba_set_coex::pst_hmac_vap null.}");
        return -OAL_EFAIL;
    }
    if (hmac_chba_vap_start_check(hmac_vap) == OAL_FALSE) {
        return -OAL_EFAIL;
    }
    return OAL_SUCC;
}
int32_t wal_ioctl_chba_set_coex_cfg_event(oal_net_device_stru *net_dev,
    mac_chba_set_coex_chan_info *cfg_coex_chan_info)
{
    wal_msg_write_stru write_msg;
    int32_t rslt;

    // 抛事件到hmac保存共存信息
    wal_write_msg_hdr_init(&write_msg, WLAN_CFGID_CHBA_SET_COEX_CHAN_INFO, sizeof(mac_chba_set_coex_chan_info));

    /* 设置配置命令参数 */
    if (memcpy_s(write_msg.auc_value, sizeof(write_msg.auc_value),
        (const void *)cfg_coex_chan_info, sizeof(mac_chba_set_coex_chan_info)) != EOK) {
        return -OAL_EFAIL;
    }

    rslt = wal_send_cfg_event(net_dev, WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_chba_set_coex_chan_info), (uint8_t *)&write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(rslt != OAL_SUCC)) {
        return -OAL_EFAIL;
    }
    return OAL_SUCC;
}

/*
 * 功能描述  : chba设置共存vap支持的信道列表
 * 日    期  : 2021年10月30日
 */
int32_t wal_ioctl_chba_set_supp_coex_chan_list(oal_net_device_stru *net_dev, int8_t *command,
    wal_wifi_priv_cmd_stru priv_cmd)
{
    int32_t ret;
    mac_chba_set_coex_chan_info cfg_coex_chan_info = {0};
    uint8_t valid_chan_idx = 0;
    uint8_t idx = 0;
    uint8_t chan_number;
    /* 非CHBA VAP不处理 */
    if (wal_ioctl_chba_set_coex_start_check(net_dev) != OAL_SUCC) {
        return -OAL_EFAIL;
    }
    /* 长度校验 */
    ret = wal_ioctl_judge_input_param_length(priv_cmd, CMD_SET_SUPP_COEX_CHAN_LIST_LEN,
        sizeof(mac_chba_set_coex_chan_info));
    if (ret != OAL_SUCC) {
        return -OAL_EFAIL;
    }
    /* 数据拷贝 */
    if (memcpy_s(&cfg_coex_chan_info, sizeof(mac_chba_set_coex_chan_info),
        command + CMD_SET_SUPP_COEX_CHAN_LIST_LEN + 1, sizeof(mac_chba_set_coex_chan_info)) != EOK) {
        return -OAL_EFAIL;
    }
    /* 消息类型、信道列表个数校验 */
    if (wal_ioctl_chba_set_coex_cmd_para_check(&cfg_coex_chan_info) != OAL_SUCC) {
        return -OAL_EFAIL;
    }
    /* 上层下发共存信道列表也包含STA可漫游的信道等 */
    /* 解析信道列表 */
    while (idx < cfg_coex_chan_info.supported_channel_cnt) {
        chan_number = cfg_coex_chan_info.supported_channel_list[idx++];
        if (hmac_chba_is_valid_channel(chan_number) != OAL_SUCC) {
            return -OAL_EFAIL;
        }
        if (hmac_chba_coex_chan_is_in_list(cfg_coex_chan_info.supported_channel_list, valid_chan_idx, chan_number)) {
            oam_warning_log1(0, OAM_SF_CHBA, "set_supp_coex_chan_list::chan %d already in list.", chan_number);
            continue;
        }
        cfg_coex_chan_info.supported_channel_list[valid_chan_idx++] = chan_number;
    }
    cfg_coex_chan_info.supported_channel_cnt = valid_chan_idx;
    return wal_ioctl_chba_set_coex_cfg_event(net_dev, &cfg_coex_chan_info);
}

int32_t wal_ioctl_chba_set_coex(oal_net_device_stru *net_dev, int8_t *command)
{
    uint32_t ret;
    uint32_t off_set = 0;
    int8_t arg[WAL_HIPRIV_CMD_NAME_MAX_LEN] = { 0 };
    uint8_t chan_idx = 0;
    mac_chba_set_coex_chan_info cfg_coex_chan_info;
    uint8_t chan_number;

    if (wal_ioctl_chba_set_coex_start_check(net_dev) != OAL_SUCC) {
        return -OAL_EFAIL;
    }

    memset_s(&cfg_coex_chan_info, sizeof(mac_chba_set_coex_chan_info), 0, sizeof(mac_chba_set_coex_chan_info));
    /* 获取共存信道配置类型 */
    ret = wal_get_cmd_one_arg(command, arg, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        return -OAL_EFAIL;
    }
    cfg_coex_chan_info.cfg_cmd_type = (uint8_t)oal_atoi(arg);
    command += off_set;
    /* 获取信道个数 */
    ret = wal_get_cmd_one_arg(command, arg, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        return -OAL_EFAIL;
    }
    cfg_coex_chan_info.supported_channel_cnt = (uint8_t)oal_atoi(arg);
    if (wal_ioctl_chba_set_coex_cmd_para_check(&cfg_coex_chan_info) != OAL_SUCC) {
        return -OAL_EFAIL;
    }
    command += off_set;
    /* 解析信道列表 */
    while (chan_idx < cfg_coex_chan_info.supported_channel_cnt) {
        ret = wal_get_cmd_one_arg(command, arg, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
        if (ret != OAL_SUCC) {
            return -OAL_EFAIL;
        }
        chan_number = (uint8_t)oal_atoi(arg);
        if (hmac_chba_is_valid_channel(chan_number) != OAL_SUCC) {
            return -OAL_EFAIL;
        }
        if (hmac_chba_coex_chan_is_in_list(cfg_coex_chan_info.supported_channel_list, chan_idx, chan_number)) {
            oam_warning_log1(0, OAM_SF_CHBA, "wal_ioctl_chba_set_coex::chan %d already in list.", chan_number);
            command += off_set;
            continue;
        }
        cfg_coex_chan_info.supported_channel_list[chan_idx++] = chan_number;
        command += off_set;
    }
    cfg_coex_chan_info.supported_channel_cnt = chan_idx;
    return wal_ioctl_chba_set_coex_cfg_event(net_dev, &cfg_coex_chan_info);
}
uint32_t hmac_chba_coex_info_changed_report(mac_vap_stru *mac_vap, uint8_t len, uint8_t *param)
{
    hmac_chba_report_coex_chan_info rpt_coex_info;
    mac_chba_rpt_coex_info *d2h_coex_info = (mac_chba_rpt_coex_info *)param;
    uint8_t d2h_chan_idx = 0;
    uint8_t rpt_chan_idx = 0;
    uint8_t chan_number;

    memset_s(&rpt_coex_info, sizeof(rpt_coex_info), 0, sizeof(rpt_coex_info));
    rpt_coex_info.report_type = HMAC_CHBA_COEX_CHAN_INFO_REPORT;
    memcpy_s(rpt_coex_info.dev_mac_addr, WLAN_MAC_ADDR_LEN, d2h_coex_info->dev_mac_addr, WLAN_MAC_ADDR_LEN);

    while (d2h_chan_idx < d2h_coex_info->coex_chan_cnt) {
        chan_number = d2h_coex_info->coex_chan_lists[d2h_chan_idx];
        if (hmac_chba_is_valid_channel(chan_number) != OAL_SUCC ||
            hmac_chba_coex_chan_is_in_list(rpt_coex_info.coex_chan_lists, rpt_chan_idx, chan_number) == OAL_TRUE) {
            oam_warning_log1(0, OAM_SF_CHBA, "hmac_chba_coex_info_changed_report:invalid number %d or already in list.",
                chan_number);
            d2h_chan_idx++;
            continue;
        }
        rpt_coex_info.coex_chan_lists[rpt_chan_idx++] = chan_number;
        d2h_chan_idx++;
    }
    rpt_coex_info.coex_chan_cnt = rpt_chan_idx;
    if (rpt_coex_info.coex_chan_cnt != 0) {
        oam_warning_log3(0, 0, "hmac_chba_coex_info_changed_report::peer coex cnt[%d], first chan[%d], last[%d]",
            rpt_coex_info.coex_chan_cnt, rpt_coex_info.coex_chan_lists[0],
            rpt_coex_info.coex_chan_lists[rpt_coex_info.coex_chan_cnt - 1]);
    } else {
        oam_warning_log0(0, OAM_SF_ANY, "hmac_chba_coex_info_changed_report::peer coex cnt 0, no coex list!");
    }

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    oal_cfg80211_chba_report((uint8_t *)(&rpt_coex_info), sizeof(rpt_coex_info));
#endif
    return OAL_SUCC;
}

void hmac_chba_coex_island_info_report(mac_chba_self_conn_info_stru *self_conn_info)
{
    hmac_chba_report_island_info rpt_island_info;
    uint8_t dev_idx;

    rpt_island_info.report_type = HMAC_CHBA_COEX_ISLAND_INFO_REPORT;
    rpt_island_info.island_dev_cnt = self_conn_info->island_device_cnt;

    oam_warning_log1(0, OAM_SF_ANY,
        "hmac_chba_coex_island_info_report::island dev cnt[%d]", rpt_island_info.island_dev_cnt);
    for (dev_idx = 0; dev_idx < self_conn_info->island_device_cnt; dev_idx++) {
        memcpy_s(rpt_island_info.island_dev_lists[dev_idx], WLAN_MAC_ADDR_LEN,
            self_conn_info->island_device_list[dev_idx].mac_addr, WLAN_MAC_ADDR_LEN);
    }

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    oal_cfg80211_chba_report((uint8_t *)(&rpt_island_info), sizeof(rpt_island_info));
#endif
    return;
}

uint32_t hmac_config_chba_set_coex_chan(mac_vap_stru *mac_vap, uint16_t len, uint8_t *params)
{
    hmac_device_stru *hmac_device = NULL;
    mac_chba_set_coex_chan_info *cfg_coex_chan_info = NULL;
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    hmac_chba_vap_stru *chba_vap_info = NULL;
    uint32_t ret;

    if (hmac_vap == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    chba_vap_info = hmac_vap->hmac_chba_vap_info;
    hmac_device = hmac_res_get_mac_dev(mac_vap->uc_device_id);
    if (oal_unlikely(hmac_device == NULL || chba_vap_info == NULL)) {
        oam_error_log0(mac_vap->uc_device_id, OAM_SF_ANY, "{hmac_config_chba_set_coex_chan::para null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    cfg_coex_chan_info = (mac_chba_set_coex_chan_info *)params;
    if (cfg_coex_chan_info->supported_channel_cnt != 0) {
        oam_warning_log4(0, OAM_SF_ANY, "hmac_config_chba_set_coex_chan::cfg_cmd_type[%d][0:self dev 1:island], \
            coex_chan_cnt[%d], first chan[%d], last chan[%d]", cfg_coex_chan_info->cfg_cmd_type,
            cfg_coex_chan_info->supported_channel_cnt, cfg_coex_chan_info->supported_channel_list[0],
            cfg_coex_chan_info->supported_channel_list[cfg_coex_chan_info->supported_channel_cnt - 1]);
    } else {
        oam_warning_log1(0, OAM_SF_ANY, "hmac_config_chba_set_coex_chan::cfg_cmd_type[%d][0:self dev 1:island], \
            coex_chan_cnt is 0", cfg_coex_chan_info->cfg_cmd_type);
    }
    // 将本设备共存信道信息保存在chba_vap下，岛内信息保存在mac_device下
    if (cfg_coex_chan_info->cfg_cmd_type == HMAC_CHBA_SELF_DEV_COEX_CFG_TYPE) {
        chba_vap_info->self_coex_chan_cnt = cfg_coex_chan_info->supported_channel_cnt;
        memset_s(chba_vap_info->self_coex_channels_list, WLAN_5G_CHANNEL_NUM, 0, WLAN_5G_CHANNEL_NUM);
        if (memcpy_s(chba_vap_info->self_coex_channels_list, WLAN_5G_CHANNEL_NUM,
            cfg_coex_chan_info->supported_channel_list, cfg_coex_chan_info->supported_channel_cnt) != EOK) {
            return OAL_FAIL;
        }
        hmac_chba_save_update_beacon_pnf(hmac_chba_sync_get_domain_bssid());
        return OAL_SUCC;
    } else if (cfg_coex_chan_info->cfg_cmd_type == HMAC_CHBA_ISLAND_COEX_CFG_TYPE) {
        hmac_device->island_coex_info.island_coex_chan_cnt = cfg_coex_chan_info->supported_channel_cnt;
        if (memcpy_s(hmac_device->island_coex_info.island_coex_channels_list, WLAN_5G_CHANNEL_NUM,
            cfg_coex_chan_info->supported_channel_list, cfg_coex_chan_info->supported_channel_cnt) != EOK) {
            return OAL_FAIL;
        }
    }
    // 抛事件到dmac
    ret = hmac_config_send_event(mac_vap, WLAN_CFGID_CHBA_SET_COEX_CHAN_INFO, sizeof(mac_chba_set_coex_chan_info),
        (uint8_t *)cfg_coex_chan_info);
    if (ret != OAL_SUCC) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_config_chba_set_coex_chan:set coex chan err[%u]}", ret);
        return ret;
    }
    return OAL_SUCC;
}

static void hmac_chba_dbac_vap_init_set_rx_reorder_timeout(hmac_vap_stru *hmac_vap)
{
    uint8_t ac_type;

    for (ac_type = WLAN_WME_AC_BE; ac_type < WLAN_WME_AC_BUTT; ac_type++) {
        hmac_vap->us_rx_timeout[ac_type] = CHBA_DBAC_BA_RX_DEFAULT_TIMEOUT;
        hmac_vap->us_rx_timeout_min[ac_type] = CHBA_DBAC_BA_RX_DEFAULT_TIMEOUT;
    }
    oam_warning_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ANY,
        "{hmac_chba_dbac_vap_init_set_rx_reorder_timeout:set chba rx_timeout[%d]}", CHBA_DBAC_BA_RX_DEFAULT_TIMEOUT);
}

/*
 * 函 数 名  :hmac_chba_dbac_vap_set_rx_reorder_timeout
 * 功能描述  : CHBA dbac模式配置重排序超时定时器定时门限参数
 *   日    期  : 2022年08月12日
 *   作    者  : wifi7
 *   修改内容  : 新生成函数
 */
void hmac_chba_dbac_vap_set_rx_reorder_timeout(uint8_t is_dbac_running)
{
    hmac_vap_stru *hmac_vap = NULL;
    hmac_chba_vap_stru *chba_vap_info = NULL;

    /* chba vap未启动, 无需处理 */
    chba_vap_info = hmac_get_up_chba_vap_info();
    if (chba_vap_info == NULL) {
        return;
    }
    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(chba_vap_info->mac_vap_id);
    if (hmac_vap == NULL) {
        oam_error_log0(0, OAM_SF_CHBA, "{hmac_chba_dbac_vap_set_rx_reorder_timeout:hmac_vap is NULL!}");
        return;
    }

    /* 退出dbac模式, 恢复CHBA vap rx重排序定时器超时配置 */
    if (is_dbac_running == OAL_FALSE) {
        hmac_vap_init_set_rx_reorder_timeout(hmac_vap);
        oam_warning_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ANY,
            "{hmac_chba_dbac_vap_set_rx_reorder_timeout:reset chba vap rx_reorder_timeout}");
        return;
    }

    /* chba进入dbac模式, 初始化chba独有的强制移窗超时定时器定时时间门限配置 */
    hmac_chba_dbac_vap_init_set_rx_reorder_timeout(hmac_vap);
}

/* 功能描述 : 从mac_vap指针数组列表中查找chba vap */
static mac_vap_stru *hmac_chba_coex_find_chba_vap_from_vap_list(mac_vap_stru **up_mac_vap, uint32_t up_vap_num)
{
    uint8_t idx;
    mac_vap_stru *mac_chba_vap = NULL;

    /* chba定制化开关未开启 */
    if (!(g_optimized_feature_switch_bitmap & BIT(CUSTOM_OPTIMIZE_CHBA))) {
        return NULL;
    }

    /* 从up vap列表中找寻chba vap, 如果不存在则无需处理 */
    for (idx = 0; idx < up_vap_num; idx++) {
        if (mac_is_chba_mode(up_mac_vap[idx]) == OAL_TRUE) {
            mac_chba_vap = up_mac_vap[idx];
            break;
        }
    }
    return mac_chba_vap;
}

/*
 * 功能描述 : chba判断是否支持dbac+dbac共存
 */
oal_bool_enum_uint8 hmac_chba_coex_is_support_dbdc_dbac(mac_vap_stru **up_mac_vap, uint32_t up_vap_num)
{
    mac_vap_stru *mac_chba_vap = NULL;

    /* 从up vap列表中找寻chba vap, 如果不存在则无需处理 */
    mac_chba_vap = hmac_chba_coex_find_chba_vap_from_vap_list(up_mac_vap, up_vap_num);
    if (mac_chba_vap == NULL) {
        return OAL_TRUE;
    }

    /* chba支持2vap的dbdc_dbac */
    if (up_vap_num == 2) {
        return OAL_TRUE;
    }

    return wlan_chip_is_support_chba_dbdc_dbac();
}
#endif
