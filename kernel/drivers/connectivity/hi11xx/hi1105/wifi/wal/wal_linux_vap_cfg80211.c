/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : Linux cfg80211接口
 * 作    者 :
 * 创建日期 : 2012年11月8日
 */

#include "securec.h"
#include "securectype.h"

#include "mac_ie.h"
#include "hmac_p2p.h"
#include "hmac_tx_data.h"
#ifdef _PRE_WLAN_CHBA_MGMT
#include "hmac_chba_mgmt.h"
#endif

#include "wal_config_base.h"
#include "wal_cfg_ioctl.h"
#include "wal_linux_cfg80211.h"
#include "wal_linux_netdev_ops.h"
#include "wal_linux_cfgvendor.h"
#ifdef _PRE_CONFIG_HISI_S3S4_POWER_STATE
#include "wal_dfx.h"
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID              OAM_FILE_ID_WAL_LINUX_VAP_CFG80211_C

/*
 * 功能描述  : CFG80211接口添加网络设备发送事件到wal处理
 * 1.日    期  : 2020年8月6日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_cfg80211_add_vap_config(mac_cfg_add_vap_param_stru *add_vap_param,
    mac_vap_stru *cfg_mac_vap, oal_net_device_stru *cfg_net_dev, wlan_vap_mode_enum_uint8 vap_mode,
    wlan_p2p_mode_enum_uint8 p2p_mode)
{
    oal_net_device_stru *net_dev = NULL;
    wal_msg_stru *rsp_msg = NULL;
    wal_msg_write_stru write_msg = { 0 };
    uint32_t err_code;
    int32_t ret;

    /* 获取mac device */
    net_dev = add_vap_param->pst_net_dev;
    oal_netdevice_flags(net_dev) &= ~OAL_IFF_RUNNING; /* 将net device的flag设为down */

    /***************************************************************************
        抛事件到wal层处理
    ***************************************************************************/
    /* 填写消息 */
    wal_write_msg_hdr_init(&write_msg, WLAN_CFGID_ADD_VAP, sizeof(mac_cfg_add_vap_param_stru));
    ((mac_cfg_add_vap_param_stru *)write_msg.auc_value)->pst_net_dev = net_dev;
    ((mac_cfg_add_vap_param_stru *)write_msg.auc_value)->en_vap_mode = vap_mode;
    ((mac_cfg_add_vap_param_stru *)write_msg.auc_value)->uc_cfg_vap_indx = cfg_mac_vap->uc_vap_id;
    ((mac_cfg_add_vap_param_stru *)write_msg.auc_value)->en_p2p_mode = p2p_mode;
    ((mac_cfg_add_vap_param_stru *)write_msg.auc_value)->is_primary_vap = 0;
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    wal_vap_get_wlan_mode_para(&write_msg);
#endif
    /* 发送消息 */
    ret = wal_send_cfg_event(cfg_net_dev, WAL_MSG_TYPE_WRITE,
                             WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_add_vap_param_stru),
                             (uint8_t *)&write_msg, OAL_TRUE, &rsp_msg);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(cfg_mac_vap->uc_vap_id, 0, "{wal_cfg80211_add_vap_config::return err code %d!}", ret);
        return (uint32_t)ret;
    }

    /* 读取返回的错误码 */
    err_code = wal_check_and_release_msg_resp(rsp_msg);
    if (err_code != OAL_SUCC) {
        oam_warning_log1(cfg_mac_vap->uc_vap_id, OAM_SF_ANY,
            "{wal_cfg80211_add_vap_config::hmac add vap fail, err_code[%u]!}\r\n", err_code);
        return err_code;
    }
    wal_set_random_mac_to_mib(net_dev); /* set random mac to mib ; for mp12-cb */

    return OAL_SUCC;
}

/*
 * 函 数 名  : wal_cfg80211_add_vap
 * 功能描述  : CFG80211 接口添加网络设备
 * 1.日    期  : 2015年1月3日
  *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_cfg80211_add_vap(mac_cfg_add_vap_param_stru *add_vap_param)
{
    /*  : P2P change interface时不需要重新初始化和注册netdev.
     * nl80211 netlink pre diot 中会获取rntl_lock互斥锁， 注册net_device 会获取rntl_lock互斥锁，造成了死锁 */
    oal_net_device_stru *net_dev = add_vap_param->pst_net_dev;
    oal_net_device_stru *cfg_net_dev = NULL;
    oal_wireless_dev_stru *wdev = net_dev->ieee80211_ptr;
    mac_wiphy_priv_stru *wiphy_priv = (mac_wiphy_priv_stru *)oal_wiphy_priv(wdev->wiphy);
    mac_vap_stru *cfg_mac_vap = NULL;
    hmac_vap_stru *cfg_hmac_vap = NULL;
    mac_device_stru *mac_device = wiphy_priv->pst_mac_device;
    wlan_vap_mode_enum_uint8 vap_mode = add_vap_param->en_vap_mode;
    wlan_p2p_mode_enum_uint8 p2p_mode = add_vap_param->en_p2p_mode;
    uint32_t ret;

    cfg_mac_vap = (mac_vap_stru *)mac_res_get_mac_vap(mac_device->uc_cfg_vap_id);
    if (oal_unlikely(cfg_mac_vap == NULL)) {
        oam_warning_log1(0, 0, "{wal_cfg80211_add_vap::cfg_mac_vap null vap_id:%d}", mac_device->uc_cfg_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }
    cfg_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_device->uc_cfg_vap_id);
    if (oal_unlikely(cfg_hmac_vap == NULL)) {
        oam_warning_log1(0, 0, "{wal_cfg80211_add_vap::cfg_hmac_vap null vapid:%d}", mac_device->uc_cfg_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }
    cfg_net_dev = cfg_hmac_vap->pst_net_device;
    if (oal_unlikely(cfg_net_dev == NULL)) {
        oam_warning_log0(0, OAM_SF_CFG, "{wal_cfg80211_add_vap::pst_mac_device is null ptr!}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    wal_set_wdev_iftype(wdev, vap_mode, p2p_mode);

    ret = wal_cfg80211_add_vap_config(add_vap_param, cfg_mac_vap, cfg_net_dev, vap_mode, p2p_mode);
    if (ret != OAL_SUCC) {
        return ret;
    }

    return OAL_SUCC;
}

/*
 * 函 数 名  : wal_cfg80211_del_vap
 * 功能描述  : CFG80211 接口删除网络设备
 * 1.日    期  : 2015年1月3日
  *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_cfg80211_del_vap(mac_cfg_del_vap_param_stru *pst_del_vap_param)
{
    wal_msg_write_stru st_write_msg = {0};
    wal_msg_stru *pst_rsp_msg = NULL;
    oal_net_device_stru *pst_net_dev = NULL;
    mac_vap_stru *pst_mac_vap = NULL;

    if (oal_unlikely(pst_del_vap_param == NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_cfg80211_del_vap::pst_del_vap_param null ptr !}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_net_dev = pst_del_vap_param->pst_net_dev;
    /* 设备在up状态不允许删除，必须先down */
    if (oal_unlikely(0 != (OAL_IFF_RUNNING & oal_netdevice_flags(pst_net_dev)))) {
        oam_error_log1(0, OAM_SF_ANY, "{wal_cfg80211_del_vap::device is busy, please down it first %d!}\r\n",
            oal_netdevice_flags(pst_net_dev));
        return OAL_ERR_CODE_CONFIG_BUSY;
    }
    pst_mac_vap = oal_net_dev_priv(pst_net_dev);
    if (oal_unlikely(pst_mac_vap == NULL)) {
        oam_error_log0(0, OAM_SF_CFG, "{wal_cfg80211_del_vap::can't get mac vap from netdevice priv data!}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    /* 设备在up状态不允许删除，必须先down */
    if ((pst_mac_vap->en_vap_state != MAC_VAP_STATE_INIT) &&
        (pst_mac_vap->en_vap_state != MAC_VAP_STATE_STA_SCAN_COMP)) {
        oam_error_log1(0, OAM_SF_ANY, "{wal_cfg80211_del_vap::device is busy, please down it first %d!}\r\n",
            pst_mac_vap->en_vap_state);
        return OAL_ERR_CODE_CONFIG_BUSY;
    }

    /* 不需要释放net_device 结构下的wireless_dev 成员 */
    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    /* 初始化删除vap 参数 */
    ((mac_cfg_del_vap_param_stru *)st_write_msg.auc_value)->pst_net_dev = pst_net_dev;
    ((mac_cfg_del_vap_param_stru *)st_write_msg.auc_value)->en_p2p_mode = pst_del_vap_param->en_p2p_mode;
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_DEL_VAP, sizeof(mac_cfg_del_vap_param_stru));
    if (OAL_SUCC != wal_send_cfg_event(pst_net_dev, WAL_MSG_TYPE_WRITE,
                                       WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_del_vap_param_stru),
                                       (uint8_t *)&st_write_msg, OAL_TRUE, &pst_rsp_msg)) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_cfg80211_del_vap::wal_send_cfg_event fail!}");
        return OAL_EFAIL;
    }
    if (wal_check_and_release_msg_resp(pst_rsp_msg) != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_cfg80211_del_vap::wal_check_and_release_msg_resp fail!}");
        return OAL_EFAIL;
    }

    if (!g_wlan_spec_cfg->p2p_device_gc_use_one_vap) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_cfg80211_del_vap::clear netdev priv.}");
        oal_net_dev_priv(pst_net_dev) = NULL;
    }
    return OAL_SUCC;
}

/*
 * 函 数 名  : wal_find_another_listen_vap
 * 功能描述  : 找到本device下存在其他处于P2P listen状态的vap
 * 1.日    期  : 2019年1月17日
 *   修改内容  : 新生成函数
 */
OAL_STATIC mac_vap_stru *wal_find_another_listen_vap(mac_device_stru *mac_device, uint8_t vap_id_self)
{
    uint8_t vap_idx;
    mac_vap_stru *mac_vap = NULL;

    for (vap_idx = 0; vap_idx < mac_device->uc_vap_num; vap_idx++) {
        mac_vap = mac_res_get_mac_vap(mac_device->auc_vap_id[vap_idx]);
        if (mac_vap == NULL || vap_id_self == mac_vap->uc_vap_id) {
            continue;
        }

        if (mac_vap->en_vap_state == MAC_VAP_STATE_STA_LISTEN) {
            return mac_vap;
        }
    }

    return NULL;
}

/*
 * 函 数 名  : wal_another_vap_stop_p2p_listen
 * 功能描述  : 如果本device下存在其他的vap处于P2P listen状态，则需要取消P2P listen
 * 1.日    期  : 2019年1月17日
 *   修改内容  : 新生成函数
 */
void wal_another_vap_stop_p2p_listen(mac_device_stru *pst_mac_device, uint8_t uc_self_vap_id)
{
    mac_vap_stru *pst_listen_mac_vap = NULL;
    hmac_vap_stru *pst_listen_hmac_vap = NULL;
    int32_t l_ret;

    pst_listen_mac_vap = wal_find_another_listen_vap(pst_mac_device, uc_self_vap_id);
    if (pst_listen_mac_vap == NULL) {
        return;
    }

    pst_listen_hmac_vap = mac_res_get_hmac_vap(pst_listen_mac_vap->uc_vap_id);
    if (oal_unlikely(pst_listen_hmac_vap == NULL)) {
        oam_error_log1(uc_self_vap_id, OAM_SF_CFG,
            "{wal_another_vap_stop_p2p_listen::fail to get hmac vap, listen vap id[%d]}",
            pst_listen_mac_vap->uc_vap_id);
        return;
    }

    oam_warning_log1(uc_self_vap_id, OAM_SF_CFG,
        "{wal_another_vap_stop_p2p_listen::begin to stop p2p listen, listen vap id[%d]}",
        pst_listen_mac_vap->uc_vap_id);

    l_ret = wal_p2p_stop_roc(pst_listen_mac_vap, pst_listen_hmac_vap->pst_net_device);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_error_log1(uc_self_vap_id, OAM_SF_CFG,
            "{wal_another_vap_stop_p2p_listen::fail to stop p2p listen, listen vap id[%d]}",
            pst_listen_mac_vap->uc_vap_id);
        return;
    }
}
/*
 * 函 数 名  : wal_cfg80211_set_ssid
 * 功能描述  : 启动ap
 * 1.日    期  : 2014年12月31日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC int32_t wal_cfg80211_set_ssid(oal_net_device_stru *pst_netdev,
                                         uint8_t *puc_ssid_ie,
                                         uint8_t uc_ssid_len)
{
    wal_msg_write_stru st_write_msg;
    mac_cfg_ssid_param_stru *pst_ssid_param;
    int32_t l_ret;

    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_SSID, sizeof(mac_cfg_ssid_param_stru));

    pst_ssid_param = (mac_cfg_ssid_param_stru *)(st_write_msg.auc_value);
    pst_ssid_param->uc_ssid_len = uc_ssid_len;
    if (EOK != memcpy_s(pst_ssid_param->ac_ssid, sizeof(pst_ssid_param->ac_ssid),
        (int8_t *)puc_ssid_ie, uc_ssid_len)) {
        oam_error_log0(0, OAM_SF_ANY, "wal_cfg80211_set_ssid::memcpy fail!");
        return -OAL_EFAIL;
    }

    l_ret = wal_send_cfg_event(pst_netdev, WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_ssid_param_stru), (uint8_t *)&st_write_msg, OAL_FALSE, NULL);
    if (l_ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_cfg80211_set_ssid::fail to send ssid cfg msg, error[%d]}", l_ret);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}
OAL_STATIC int32_t wal_cfg80211_start_ap_set_ssid(oal_net_device_stru *netdev,
    oal_ap_settings_stru *ap_settings, mac_vap_stru *mac_vap)
{
    uint8_t *puc_ssid_ie = NULL;
    uint8_t uc_ssid_len;
    int32_t l_ssid_len;
    uint8_t auc_ssid_ie[NUM_32_BYTES];
    uint8_t uc_vap_id = mac_vap->uc_vap_id;
    int32_t l_memcpy_ret;
    int32_t l_ret;

    l_ssid_len = (int32_t)ap_settings->ssid_len;
    if ((l_ssid_len > 32) || (l_ssid_len <= 0)) {    /* 32 ssid字符串最大长度 */
        oam_warning_log1(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_start_ap::ssid len error, len[%d].}", l_ssid_len);
        return -OAL_EFAIL;
    }
    memset_s(auc_ssid_ie, sizeof(auc_ssid_ie), 0, sizeof(auc_ssid_ie));
    l_memcpy_ret = memcpy_s(auc_ssid_ie, sizeof(auc_ssid_ie), ap_settings->ssid, (uint32_t)l_ssid_len);
    if (l_memcpy_ret != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "wal_cfg80211_start_ap_set_ssid::memcpy fail!");
        return -OAL_EFAIL;
    }

    puc_ssid_ie = auc_ssid_ie;
    uc_ssid_len = (uint8_t)l_ssid_len;

    if (uc_ssid_len != 0) {
        l_ret = wal_cfg80211_set_ssid(netdev, puc_ssid_ie, uc_ssid_len);
        if (l_ret != OAL_SUCC) {
            oam_warning_log1(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_start_ap::fail send ssid cfg msg, err[%d]}", l_ret);
            return -OAL_EFAIL;
        }
    }
    return OAL_SUCC;
}

OAL_STATIC int32_t wal_cfg80211_start_ap_set_beacon_event(oal_wiphy_stru *wiphy, oal_net_device_stru *netdev,
    oal_ap_settings_stru *ap_settings, mac_vap_stru *mac_vap)
{
    mac_beacon_param_stru st_beacon_param; /* beacon info struct */
    oal_beacon_data_stru *pst_beacon_info = NULL;
    uint8_t uc_vap_id = mac_vap->uc_vap_id;
    wal_msg_write_stru st_write_msg;
    int32_t l_memcpy_ret;
    int32_t l_ret;

    /* 初始化beacon interval 和DTIM_PERIOD 参数 */
    memset_s(&st_beacon_param, sizeof(mac_beacon_param_stru), 0, sizeof(mac_beacon_param_stru));
    st_beacon_param.l_interval = ap_settings->beacon_interval;
    st_beacon_param.l_dtim_period = ap_settings->dtim_period;
    st_beacon_param.uc_hidden_ssid = (ap_settings->hidden_ssid == 1);

#ifdef _PRE_WLAN_CHBA_MGMT
    /* 对于CHBA VAP的特殊启动 */
    if ((g_optimized_feature_switch_bitmap & BIT(CUSTOM_OPTIMIZE_CHBA)) &&
        (oal_strcmp("chba0", netdev->name) == 0)) {
        st_beacon_param.l_interval = MAC_CHBA_DEFALUT_PERIODS_MS;
        st_beacon_param.l_dtim_period = 2; // 2: chba dtim period
        st_beacon_param.uc_hidden_ssid = 1; // 1: chba hidden ssid
    }
#endif
    oam_warning_log3(0, OAM_SF_ANY,
        "{wal_cfg80211_fill_beacon_param::beacon_interval=%d, dtim_period=%d, hidden_ssid=%d!}",
        ap_settings->beacon_interval, ap_settings->dtim_period, ap_settings->hidden_ssid);
    pst_beacon_info = &(ap_settings->beacon);
    l_ret = wal_cfg80211_fill_beacon_param(netdev, pst_beacon_info, &st_beacon_param);
    if (l_ret != OAL_SUCC) {
        oam_error_log1(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_start_ap::failed to fill beacon param, error[%d]}", l_ret);
        return -OAL_EINVAL;
    }

    /* 设置操作类型 */
    st_beacon_param.en_operation_type = MAC_ADD_BEACON;

    /* 填写 msg 消息头 */
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_CFG80211_CONFIG_BEACON, sizeof(mac_beacon_param_stru));

    /* 填写 msg 消息体 */
    l_memcpy_ret = memcpy_s(st_write_msg.auc_value, sizeof(mac_beacon_param_stru),
        &st_beacon_param, sizeof(mac_beacon_param_stru));
    if (l_memcpy_ret != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "wal_cfg80211_start_ap::memcpy fail!");
        return -OAL_EFAIL;
    }

    /* 发送消息 */
    l_ret = wal_send_cfg_event(netdev, WAL_MSG_TYPE_WRITE, WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_beacon_param_stru),
        (uint8_t *)&st_write_msg, OAL_FALSE, NULL);
    if (l_ret != OAL_SUCC) {
        oam_warning_log1(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_start_ap::fail to start addset beacon, err[%d]}", l_ret);
        return -OAL_EFAIL;
    }
#ifdef _PRE_WLAN_CHBA_MGMT
    /* 对于CHBA VAP的特殊启动 */
    if (wal_cfg80211_cfg_chba(wiphy, netdev, mac_vap) != OAL_SUCC) {
        return -OAL_EFAIL;
    }
#endif
    return OAL_SUCC;
}
/*
 * 函 数 名  : wal_cfg80211_set_auth_mode
 * 功能描述  : 配置vap的auth类型
 * 1.日    期  : 2016年10月07日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC int32_t wal_cfg80211_set_auth_mode(oal_net_device_stru *pst_netdev, uint8_t en_auth_algs)
{
    wal_msg_write_stru st_write_msg;
    int32_t l_ret;

    if ((en_auth_algs != WLAN_WITP_AUTH_OPEN_SYSTEM) &&
        (en_auth_algs != WLAN_WITP_AUTH_SHARED_KEY)
#ifdef _PRE_WLAN_FEATURE_SAE
        && (en_auth_algs != WLAN_WITP_AUTH_SAE)
        && (en_auth_algs != WLAN_WITP_AUTH_TBPEKE)
#endif
        && (en_auth_algs != WLAN_WITP_AUTH_AUTOMATIC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_cfg80211_set_auth_mode::en_auth_algs error[%d].}", en_auth_algs);
        return -OAL_EFAIL;
    }

    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_AUTH_MODE, sizeof(uint32_t));
    *((uint32_t *)(st_write_msg.auc_value)) = en_auth_algs; /* 设置配置命令参数 */

    l_ret = wal_send_cfg_event(pst_netdev, WAL_MSG_TYPE_WRITE, WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(uint32_t),
        (uint8_t *)&st_write_msg, OAL_FALSE, NULL);
    if (l_ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ANY, "{wal_cfg80211_set_auth_mode::fail to send auth_tpye cfg msg, error[%d]}", l_ret);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}
#define WAL_START_AP_WAIT_OTHER_CONNECTING_VAP_MAX_TIME 1500 // 单位ms
/* start ap时检查是否有其他入网vap，等待其他vap入网完成再启动ap */
static void wal_cfg80211_start_ap_check_wait_other_connecting_vap(mac_vap_stru *mac_vap)
{
    mac_device_stru *mac_device = NULL;
    mac_vap_stru *join_mac_vap[WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE] = {NULL};
    uint32_t join_vap_num;
    signed long lefttime;
    hmac_vap_stru *join_hmac_vap = NULL;

    mac_device = mac_res_get_dev(mac_vap->uc_device_id);
    if (mac_device == NULL) {
        return;
    }
    /* 查找是否有其他入网sta */
    join_vap_num = mac_device_find_connecting_sta(mac_device, join_mac_vap, WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE);
    if (join_vap_num == 0) {
        return;
    }
    /* 不支持两个vap同时在入网状态并行建链, 如果有的话那么等待第一个入网完成 */
    if (join_vap_num > 1) {
        oam_error_log1(mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_start_ap::join vap num[%d] > 1!}", join_vap_num);
    }

    if (join_mac_vap[0] == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_start_ap::join mac vap null}");
        return;
    }
    oam_warning_log1(join_mac_vap[0]->uc_vap_id, 0, "{wal_cfg80211_start_ap::has join vap, state[%d].}",
        join_mac_vap[0]->en_vap_state);
    join_hmac_vap = mac_res_get_hmac_vap(join_mac_vap[0]->uc_vap_id);
    if (join_hmac_vap == NULL) {
        oam_error_log0(join_mac_vap[0]->uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_start_ap::join hmac vap null}");
        return;
    }
    /* 等待其他vap入网完成，如果超时则继续执行启动流程 */
    lefttime = oal_wait_event_interruptible_timeout_m(join_hmac_vap->query_wait_q,
        join_mac_vap[0]->en_vap_state == MAC_VAP_STATE_UP,
        (uint32_t)oal_msecs_to_jiffies(WAL_START_AP_WAIT_OTHER_CONNECTING_VAP_MAX_TIME));
    if (lefttime <= 0) {
        oam_error_log1(0, OAM_SF_ANY, "{wal_cfg80211_start_ap:: wait for %ld ms timeout!}",
            WAL_START_AP_WAIT_OTHER_CONNECTING_VAP_MAX_TIME);
    }
}
/*
 * 函 数 名  : wal_cfg80211_start_ap
 * 功能描述  : 启动AP,配置AP 参数。
 * 1.日    期  : 2014年12月30日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
int32_t wal_cfg80211_start_ap(oal_wiphy_stru *wiphy,
    oal_net_device_stru *netdev,
    oal_ap_settings_stru *ap_settings)
{
    mac_vap_stru *mac_vap = NULL;
    int32_t l_ret;
    oal_netdev_priv_stru *pst_netdev_priv = NULL;

    oam_warning_log0(0, OAM_SF_ANY, "{wal_cfg80211_start_ap::enter here.}");

    /* 参数合法性检查 */
    if (oal_any_null_ptr4(wiphy, netdev, netdev->ieee80211_ptr, ap_settings)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_cfg80211_start_ap::pst_wiphy or pst_netdev or ap_settings is null!}");
        return -OAL_EINVAL;
    }

    /* 获取vap id */
    mac_vap = oal_net_dev_priv(netdev);
    if (mac_vap == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_cfg80211_start_ap::mac_vap is null}");
        return -OAL_EINVAL;
    }
    wal_cfg80211_start_ap_check_wait_other_connecting_vap(mac_vap);
    wal_force_scan_abort_then_scan_comp(netdev);
    wal_cfg80211_start_ap_set_chba_mode(netdev, mac_vap);

    /*****************************************************************************
        1.设置信道
    *****************************************************************************/
    l_ret = wal_cfg80211_set_channel_info(wiphy, netdev);
    if (l_ret != OAL_SUCC) {
        return -OAL_EFAIL;
    }

    /*****************************************************************************
        2.1 设置ssid信息
    *****************************************************************************/
    l_ret = wal_cfg80211_start_ap_set_ssid(netdev, ap_settings, mac_vap);
    if (l_ret != OAL_SUCC) {
        return -OAL_EFAIL;
    }

    /*****************************************************************************
        2.2 设置auth mode信息
    *****************************************************************************/
    l_ret = wal_cfg80211_set_auth_mode(netdev, ap_settings->auth_type);
    if (l_ret != OAL_SUCC) {
        return -OAL_EFAIL;
    }

    /* 主要区分麒麟产品和非麒麟产品, 正式版本kernel 5.11.0以后添加成员 sae_pwe */
    /* 麒麟产品要使用该成员从kernel高版本移植到低版本 */
    /* 新增特性宏区分_PRE_WLAN_WPA3_R3 */
    /* 主干版本是5.10.43,需要该成员，需要编译 */
    /* hi5601 kernel版本是5.10.79,不需要该成员，本地编译通过. */

    /* mp16c/61, mp16/mp15 本地kernel版本4.14.116,本地编译通过. */
#if (defined(_PRE_WLAN_WPA3_R3) && (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 43)) || \
    (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 11, 0)))
    wal_ioctl_set_sae_pwe(netdev, ap_settings->crypto.sae_pwe);
#endif
    /*****************************************************************************
        3.设置beacon时间间隔、tim period以及安全配置消息等
    *****************************************************************************/
    l_ret = wal_cfg80211_start_ap_set_beacon_event(wiphy, netdev, ap_settings, mac_vap);
    if (l_ret != OAL_SUCC) {
        return l_ret;
    }

    /*****************************************************************************
        4.启动ap
    *****************************************************************************/
    l_ret = wal_start_vap(netdev);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_start_ap::failed start ap, error[%d]}", l_ret);
        return -OAL_EFAIL;
    }

    pst_netdev_priv = (oal_netdev_priv_stru *)oal_net_dev_wireless_priv(netdev);
    if ((pst_netdev_priv->uc_napi_enable) && (!pst_netdev_priv->uc_state)) {
        oal_napi_enable(&pst_netdev_priv->st_napi);
        pst_netdev_priv->uc_state = 1;
    }

    return OAL_SUCC;
}

OAL_STATIC int32_t wal_cfg80211_stop_ap_event(oal_net_device_stru *pst_netdev, mac_vap_stru *pst_mac_vap)
{
    int32_t l_ret;
    wal_msg_write_stru st_write_msg = {0};
    wlan_p2p_mode_enum_uint8 en_p2p_mode;
    oal_wireless_dev_stru *pst_wdev = NULL;

    if (oal_unlikely(pst_netdev == NULL || pst_netdev->ieee80211_ptr == NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_cfg80211_stop_ap_event::pst_netdev or ieee80211_ptr null!}");
        return -OAL_EFAUL;
    }
    /* 填写消息 */
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_DOWN_VAP, sizeof(mac_cfg_start_vap_param_stru));

    pst_wdev = pst_netdev->ieee80211_ptr;
    en_p2p_mode = wal_wireless_iftype_to_mac_p2p_mode(pst_wdev->iftype);
    if (en_p2p_mode == WLAN_P2P_BUTT) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_cfg80211_stop_ap::wal_wireless_iftype_to_mac_p2p_mode return BUFF}");
        return -OAL_EINVAL;
    }
    oam_warning_log1(0, OAM_SF_ANY, "{wal_cfg80211_stop_ap::en_p2p_mode=%u}\r\n", en_p2p_mode);

    ((mac_cfg_start_vap_param_stru *)st_write_msg.auc_value)->en_p2p_mode = en_p2p_mode;

    ((mac_cfg_start_vap_param_stru *)st_write_msg.auc_value)->pst_net_dev = pst_netdev;

    /* 发送消息 */
    l_ret = wal_send_cfg_event(pst_netdev, WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_start_vap_param_stru),
        (uint8_t *)&st_write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_stop_ap::stop ap error[%d]}", l_ret);
        return -OAL_EFAIL;
    }
    return OAL_SUCC;
}

int32_t wal_cfg80211_stop_ap(oal_wiphy_stru *pst_wiphy, oal_net_device_stru *pst_netdev)
{
    mac_vap_stru *pst_mac_vap = NULL;
    oal_netdev_priv_stru *pst_netdev_priv = NULL;
    int32_t ret;
    /* 参数合法性检查 */
    if (oal_any_null_ptr2(pst_wiphy, pst_netdev)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_cfg80211_stop_ap::wiphy or netdev!}");
        return -OAL_EINVAL;
    }

    if (wal_cfg80211_dfr_and_s3s4_param_check() == OAL_FALSE) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_cfg80211_stop_ap:: dfr or s3s4 is processing!}");
        return OAL_SUCC;
    }
#ifdef _PRE_WLAN_CHBA_MGMT
    /* 对于CHBA VAP的特殊启动 */
    if ((g_optimized_feature_switch_bitmap & BIT(CUSTOM_OPTIMIZE_CHBA)) &&
        (oal_strcmp("chba0", pst_netdev->name) == 0)) {
        oam_warning_log0(0, 0, "CHBA: enter stop ap.");
        ret = wal_netdev_stop_ap(pst_netdev);
        wal_del_vap_try_wlan_pm_close();
        return ret;
    }
#endif

    /* 获取vap id */
    pst_mac_vap = oal_net_dev_priv(pst_netdev);
    if (pst_mac_vap == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_cfg80211_stop_ap::can't get mac vap from netdevice priv data!}");
        return -OAL_EINVAL;
    }

    /* 判断是否为非ap模式 */
    if (pst_mac_vap->en_vap_mode != WLAN_VAP_MODE_BSS_AP) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_stop_ap::vap is not in ap mode!}");
        return -OAL_EINVAL;
    }

    /* 如果netdev不是running状态，则不需要down */
    if ((oal_netdevice_flags(pst_netdev) & OAL_IFF_RUNNING) == 0) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_cfg80211_stop_ap::vap is already down!}\r\n");
        return OAL_SUCC;
    }
    ret = wal_cfg80211_stop_ap_event(pst_netdev, pst_mac_vap);
    if (ret != OAL_SUCC) {
        return ret;
    }
    pst_netdev_priv = (oal_netdev_priv_stru *)oal_net_dev_wireless_priv(pst_netdev);
    if (pst_netdev_priv->uc_napi_enable) {
#ifndef WIN32
        oal_netbuf_list_purge(&pst_netdev_priv->st_rx_netbuf_queue);
#endif
        oal_napi_disable(&pst_netdev_priv->st_napi);
        pst_netdev_priv->uc_state = 0;
    }

    return OAL_SUCC;
}
static void wal_cfg80211_change_intf_set_vap_mode(enum nl80211_iftype if_type, wlan_vap_mode_enum_uint8 *vap_mode,
    wlan_p2p_mode_enum_uint8 *p2p_mode)
{
    if (if_type == NL80211_IFTYPE_P2P_CLIENT) {
        *vap_mode = WLAN_VAP_MODE_BSS_STA;
        *p2p_mode = WLAN_P2P_CL_MODE;
        return;
    }
    if (if_type == NL80211_IFTYPE_AP || if_type == NL80211_IFTYPE_AP_VLAN) {
        *vap_mode = WLAN_VAP_MODE_BSS_AP;
        *p2p_mode = WLAN_LEGACY_VAP_MODE;
        return;
    }
    if (if_type == NL80211_IFTYPE_P2P_GO) {
        *vap_mode = WLAN_VAP_MODE_BSS_AP;
        *p2p_mode = WLAN_P2P_GO_MODE;
        return;
    }
}

/* P2P change intf处理流程 */
static int32_t wal_cfg80211_change_intf_process(oal_net_device_stru *net_dev, enum nl80211_iftype if_type)
{
    mac_cfg_del_vap_param_stru del_vap_param;
    mac_cfg_add_vap_param_stru add_vap_param;
    wlan_p2p_mode_enum_uint8 p2p_mode = WLAN_P2P_BUTT;
    wlan_vap_mode_enum_uint8 vap_mode = WLAN_VAP_MODE_BUTT;

    if (oal_unlikely(net_dev == NULL || net_dev->ieee80211_ptr == NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_cfg80211_change_intf_process::net_dev or ieee80211_ptr null!}");
        return -OAL_EFAUL;
    }
    /* 获得vap mode\p2p mode */
    wal_cfg80211_change_intf_set_vap_mode(if_type, &vap_mode, &p2p_mode);
    if (vap_mode == WLAN_VAP_MODE_BUTT || p2p_mode == WLAN_P2P_BUTT) {
        oam_error_log1(0, 0, "{wal_cfg80211_change_virtual_intf::error iftype[%d].}", if_type);
        return -OAL_EFAIL;
    }

    /* 停止VAP */
    wal_netdev_stop(net_dev);

    memset_s(&del_vap_param, sizeof(del_vap_param), 0, sizeof(del_vap_param));
    /* 删除VAP */
    del_vap_param.pst_net_dev = net_dev;
    /* 设备p2p 模式需要从net_device 中获取 */
    del_vap_param.en_p2p_mode = wal_wireless_iftype_to_mac_p2p_mode(net_dev->ieee80211_ptr->iftype);
    if (wal_cfg80211_del_vap(&del_vap_param)) {
        wal_del_vap_try_wlan_pm_close();
        return -OAL_EFAIL;
    }
    /* 后续可以考虑不下电，缩短时间。因为下面又启动vap，会再次上电 */
    wal_del_vap_try_wlan_pm_close();
    memset_s(&add_vap_param, sizeof(add_vap_param), 0, sizeof(add_vap_param));
    /* 重新创建对应模式VAP */
    add_vap_param.pst_net_dev = net_dev;
    add_vap_param.en_vap_mode = vap_mode;
    add_vap_param.en_p2p_mode = p2p_mode;

    if (wal_cfg80211_add_vap(&add_vap_param) != OAL_SUCC) {
        oam_error_log0(0, 0, "{wal_cfg80211_change_virtual_intf::wal_cfg80211_add_vap fail.}");
        return -OAL_EFAIL;
    }

    /* 启动VAP */
    wal_netdev_open(net_dev, OAL_FALSE);

    net_dev->ieee80211_ptr->iftype = if_type;
    return OAL_SUCC;
}
/* 判断接口类型是否支持 */
static oal_bool_enum_uint8 wal_cfg80211_change_intf_is_support_type(enum nl80211_iftype if_type)
{
    return (if_type == NL80211_IFTYPE_STATION) || (if_type == NL80211_IFTYPE_P2P_CLIENT) ||
        (if_type == NL80211_IFTYPE_AP) || (if_type == NL80211_IFTYPE_AP_VLAN) ||
        (if_type == NL80211_IFTYPE_P2P_GO);
}
static oal_bool_enum_uint8 wal_cfg80211_change_intf_check_param(oal_net_device_stru *net_dev,
    enum nl80211_iftype if_type)
{
    if (wal_cfg80211_dfr_and_s3s4_param_check() == OAL_FALSE) {
        oam_warning_log0(0, OAM_SF_CFG, "{wal_cfg80211_change_virtual_intf:: dfr or s3s4 is processing!}");
        return OAL_FALSE;
    }

    /* 检查VAP 当前模式和目的模式是否相同，如果相同则直接返回 */
    if (net_dev->ieee80211_ptr->iftype == if_type) {
        oam_warning_log1(0, 0, "{wal_cfg80211_change_virtual_intf::same iftype[%d], do not need change!}", if_type);
        return OAL_FALSE;
    }

    return OAL_TRUE;
}
static int32_t wal_cfg80211_change_virtual_intf_iftype_sta_handle(oal_net_device_stru *net_dev,
    enum nl80211_iftype iftype)
{
    int32_t ret;
    if (oal_unlikely(net_dev == NULL || net_dev->ieee80211_ptr == NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{virtual_intf_iftype_sta_handle::net_dev or ieee80211_ptr null!}");
        return -OAL_EFAUL;
    }
    if (net_dev->ieee80211_ptr->iftype == NL80211_IFTYPE_AP) {
        /* 下电APUT，切换netdev状态为station */
        ret = wal_netdev_stop_ap(net_dev);
        wal_del_vap_try_wlan_pm_close();
        return ret;
    }

    // P2P_DEVICE 提前创建，不需要通过wpa_supplicant 创建
    net_dev->ieee80211_ptr->iftype = iftype;
    oam_warning_log0(0, OAM_SF_CFG, "{wal_cfg80211_change_virtual_intf::change to station}\r\n");
    return OAL_SUCC;
}
/*
 * 函 数 名  : wal_cfg80211_change_virtual_intf
 * 功能描述  : 转换AP，STA 状态
 * 1.日    期  : 2013年11月13日
  *   修改内容  : 新生成函数
 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 12, 0))
int32_t wal_cfg80211_change_virtual_intf(oal_wiphy_stru *pst_wiphy, oal_net_device_stru *pst_net_dev,
    enum nl80211_iftype en_type, oal_vif_params_stru *pst_params)
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 34))
int32_t wal_cfg80211_change_virtual_intf(oal_wiphy_stru *pst_wiphy, oal_net_device_stru *pst_net_dev,
    enum nl80211_iftype en_type, uint32_t *pul_flags, oal_vif_params_stru *pst_params)
#else
int32_t wal_cfg80211_change_virtual_intf(oal_wiphy_stru *pst_wiphy,
    int32_t l_ifindex, enum nl80211_iftype en_type, uint32_t *pul_flags, oal_vif_params_stru *pst_params)
#endif
{
    mac_vap_stru *pst_mac_vap = NULL;
    /* 入参检查 */
    if (oal_any_null_ptr4(pst_net_dev, pst_net_dev->ieee80211_ptr, pst_wiphy, pst_params)) {
        oam_error_log0(0, OAM_SF_CFG, "{wal_cfg80211_change_virtual_intf::wiphy or params ptr is null!}");
        return -OAL_EINVAL;
    }

    if (wal_cfg80211_change_intf_check_param(pst_net_dev, en_type) == OAL_FALSE) {
        return OAL_SUCC;
    }
    oam_warning_log2(0, OAM_SF_CFG, "{wal_cfg80211_change_virtual_intf::[%d][%d]}",
        (pst_net_dev->ieee80211_ptr->iftype), en_type);

    oal_io_print("wal_cfg80211_change_virtual_intf,dev_name is:%s\n", pst_net_dev->name);
    /* 判断是否为驱动支持的类型 */
    if (wal_cfg80211_change_intf_is_support_type(en_type) == OAL_FALSE) {
        oam_error_log1(0, 0, "{wal_cfg80211_change_virtual_intf::currently do not sup this type[%d]}", en_type);
        return -OAL_EINVAL;
    }
    if (en_type == NL80211_IFTYPE_STATION) {
        return wal_cfg80211_change_virtual_intf_iftype_sta_handle(pst_net_dev, en_type);
    }

    if (en_type == NL80211_IFTYPE_AP) {
        return wal_setup_ap(pst_net_dev);
    }

#ifdef _PRE_WLAN_CHBA_MGMT
    /* Supplicant在create chba0之前会先set mode为GO */
    if (oal_strcmp("chba0", pst_net_dev->name) == 0) {
        return wal_cfg80211_change_intf_chba0_proc(pst_net_dev, en_type);
    }
#endif

    /* 设备为P2P 设备才需要进行change virtual interface */
    pst_mac_vap = oal_net_dev_priv(pst_net_dev);
    if (pst_mac_vap == NULL) {
        oam_error_log0(0, OAM_SF_CFG, "{wal_cfg80211_change_virtual_intf::can't get mac vap from netdevice priv}");
        return -OAL_EINVAL;
    }

    if (is_legacy_vap(pst_mac_vap)) {
        pst_net_dev->ieee80211_ptr->iftype = en_type;
        return OAL_SUCC;
    }
    /* 解决异常情况下,wpa_supplicant下发p2p0设备切换到p2p go/cli模式导致fastboot的问题 */
    if (0 == (oal_strcmp("p2p0", pst_net_dev->name))) {
        oam_warning_log0(0, 0, "{wal_cfg80211_change_virtual_intf::p2p0 netdevice can not change to P2P CLI/GO.}");
        return -OAL_EINVAL;
    }
    /* 如果当前模式和目的模式不同，则需要:
       1. 停止 VAP
       2. 删除 VAP
       3. 重新创建对应模式VAP
       4. 启动VAP
    */
    return wal_cfg80211_change_intf_process(pst_net_dev, en_type);
}

/*
 * 函 数 名  : wal_is_p2p_group_exist
 * 功能描述  : 检查是否存在P2P group
 * 1.日    期  : 2015年9月28日
  *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_is_p2p_group_exist(mac_device_stru *pst_mac_device)
{
    if (OAL_SUCC != hmac_check_p2p_vap_num(pst_mac_device, WLAN_P2P_GO_MODE) ||
        OAL_SUCC != hmac_check_p2p_vap_num(pst_mac_device, WLAN_P2P_CL_MODE)) {
        return OAL_TRUE;
    } else {
        return OAL_FALSE;
    }
}

/*
 * 函 数 名  : wal_set_p2p_group_del_status
 * 功能描述  : 删除P2P group的net dev
 * 1.日    期  : 2019年5月21日
 *   修改内容  : 新生成函数
 */
OAL_STATIC void wal_set_p2p_group_del_status(hmac_vap_stru *pst_hmac_vap,
    oal_net_device_stru *pst_net_dev)
{
    hmac_device_stru *pst_hmac_device = hmac_res_get_mac_dev(pst_hmac_vap->st_vap_base_info.uc_device_id);
    if (oal_unlikely(pst_hmac_device == NULL)) {
        oam_error_log1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ANY,
            "{wal_set_p2p_group_del_status::hmac_device is null, device id[%d]}\r\n",
            pst_hmac_vap->st_vap_base_info.uc_device_id);
        return;
    }

    oam_warning_log3(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ANY,
        "{wal_set_p2p_group_del_status:: push deleting inf task to work queue, device id[%d], \
        vap_mode[%d], p2p_mode[%d]}\r\n", pst_hmac_vap->st_vap_base_info.uc_device_id,
        pst_hmac_vap->st_vap_base_info.en_vap_mode, pst_hmac_vap->st_vap_base_info.en_p2p_mode);

    hmac_set_p2p_status(&pst_hmac_device->p2p_intf_status, P2P_STATUS_IF_DELETING);
}

/*
 * 函 数 名  : wal_del_p2p_group
 * 功能描述  : 删除P2P group
 * 1.日    期  : 2015年9月28日
  *   修改内容  : 新生成函数
 */
uint32_t wal_del_p2p_group(mac_device_stru *pst_mac_device)
{
    uint8_t uc_vap_idx;
    mac_vap_stru *pst_mac_vap = NULL;
    hmac_vap_stru *pst_hmac_vap = NULL;
    oal_net_device_stru *pst_net_dev = NULL;
    mac_cfg_del_vap_param_stru st_del_vap_param;
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev(0);
    oal_bool_enum_uint8 is_find_first_p2p_vap = OAL_FALSE;
    for (uc_vap_idx = 0; uc_vap_idx < pst_mac_device->uc_vap_num; uc_vap_idx++) {
        pst_mac_vap = mac_res_get_mac_vap(pst_mac_device->auc_vap_id[uc_vap_idx]);
        if (oal_unlikely(pst_mac_vap == NULL)) {
            oam_warning_log0(pst_mac_device->auc_vap_id[uc_vap_idx], 0, "{wal_del_p2p_group::get mac vap fail!}");
            continue;
        }

        pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_device->auc_vap_id[uc_vap_idx]);
        if (oal_unlikely(pst_hmac_vap == NULL)) {
            oam_warning_log0(pst_mac_device->auc_vap_id[uc_vap_idx], 0, "{wal_del_p2p_group::get hmac vap fail!}");
            continue;
        }

        pst_net_dev = pst_hmac_vap->pst_net_device;
        if (oal_unlikely(pst_net_dev == NULL)) {
            oam_warning_log0(pst_mac_device->auc_vap_id[uc_vap_idx], 0, "{wal_del_p2p_group::get net device fail!}");
            continue;
        }

        if (is_p2p_go(pst_mac_vap) || is_p2p_cl(pst_mac_vap)) {
            is_find_first_p2p_vap = OAL_TRUE;
            break;
        }
    }
    if (is_find_first_p2p_vap == OAL_FALSE) {
        return OAL_SUCC;
    }
    memset_s(&st_del_vap_param, sizeof(st_del_vap_param), 0, sizeof(st_del_vap_param));
    oal_io_print("wal_del_p2p_group:: ifname %s\r\n", pst_net_dev->name);
    st_del_vap_param.pst_net_dev = pst_net_dev;
    st_del_vap_param.en_vap_mode = pst_mac_vap->en_vap_mode;
    st_del_vap_param.en_p2p_mode = mac_get_p2p_mode(pst_mac_vap);
    oam_warning_log2(pst_mac_vap->uc_vap_id, OAM_SF_P2P, "{wal_del_p2p_group:: vap mode[%d], p2p mode[%d]}",
                     st_del_vap_param.en_vap_mode, st_del_vap_param.en_p2p_mode);
    /* 删除已经存在的P2P group */
    wal_force_scan_complete(pst_net_dev, OAL_TRUE);
    wal_stop_vap(pst_net_dev);

    if (wal_cfg80211_del_vap(&st_del_vap_param) == OAL_SUCC) {
        wal_set_p2p_group_del_status(pst_hmac_vap, pst_net_dev);
        /* 启动linux work queue 删除net_device，不要直接调用unregister_netdev以防wal_netdev_stop死循环 */
        hmac_schedule_del_virtual_intf_work(hmac_device, pst_net_dev);
    }
    return OAL_SUCC;
}
/*
 函 数 名  : wal_cfg80211_add_p2p_interface_init
 功能描述  : p2p go/gc mac 地址初始化
  1.日    期   : 2019年8月30日
    修改内容   : 新生成函数
*/
OAL_STATIC void wal_cfg80211_add_p2p_interface_init(oal_net_device_stru *pst_net_dev, mac_device_stru *pst_mac_device)
{
    uint8_t auc_primary_mac_addr[WLAN_MAC_ADDR_LEN] = {0};    /*  */

    if ((oal_strncmp("p2p-p2p0", pst_net_dev->name, OAL_STRLEN("p2p-p2p0")) != 0) &&
        (oal_strncmp("p2p-wlan0", pst_net_dev->name, OAL_STRLEN("p2p-wlan0")) != 0)) {
        return;
    }

    if (oal_unlikely(pst_mac_device->st_p2p_info.pst_primary_net_device == NULL)) {
        oal_random_ether_addr(auc_primary_mac_addr, WLAN_MAC_ADDR_LEN);
        auc_primary_mac_addr[MAC_ADDR_0] &= (~0x02);
        auc_primary_mac_addr[MAC_ADDR_1] = 0x11;
        auc_primary_mac_addr[MAC_ADDR_2] = 0x02;
    } else {
        if (oal_unlikely(oal_netdevice_mac_addr(pst_mac_device->st_p2p_info.pst_primary_net_device) == NULL)) {
            oam_error_log0(0, OAM_SF_ANY,
                "{wal_cfg80211_add_p2p_interface_init() pst_primary_net_device; addr is null}\r\n");
            return;
        }

        if (memcpy_s(auc_primary_mac_addr, WLAN_MAC_ADDR_LEN,
            oal_netdevice_mac_addr(pst_mac_device->st_p2p_info.pst_primary_net_device), WLAN_MAC_ADDR_LEN) != EOK) {
            oam_error_log0(0, OAM_SF_ANY, "{wal_cfg80211_add_p2p_interface_init::memcpy fail}");
            return;
        }
    }

    oal_set_mac_addr((uint8_t *)oal_netdevice_mac_addr(pst_net_dev), auc_primary_mac_addr);
    pst_net_dev->dev_addr[MAC_ADDR_0] |= 0x02;
    pst_net_dev->dev_addr[MAC_ADDR_4] ^= 0x80;
}
/*
 * 函 数 名  : wal_cfg80211_add_virtual_intf_netdev_init
 * 功能描述  : 初始化net_device
 * 1.日    期  : 2019年11月28日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC void wal_cfg80211_add_virtual_intf_netdev_init(oal_net_device_stru *pst_net_dev,
                                                          oal_wireless_dev_stru *pst_wdev)
{
    /* 对新创建的net_device 初始化对应参数 */
#ifdef CONFIG_WIRELESS_EXT
    pst_net_dev->wireless_handlers = &g_st_iw_handler_def;
#endif
    pst_net_dev->netdev_ops = &g_st_wal_net_dev_ops;

    pst_net_dev->ethtool_ops = &g_st_wal_ethtool_ops;

    oal_netdevice_destructor(pst_net_dev) = oal_net_free_netdev;
    /* host发送报文使能csum硬化功能 */
    if (hmac_ring_tx_enabled() == OAL_TRUE) {
#ifdef _PRE_WLAN_FEATURE_GSO
        oam_warning_log0(0, OAM_SF_ANY, "{wal_cfg80211_add_virtual_intf_netdev_init::[GSO] add NETIF_F_SG}");
        pst_net_dev->features |= NETIF_F_SG;
        pst_net_dev->hw_features |= NETIF_F_SG;
#endif
        pst_net_dev->features    |= NETIF_F_IP_CSUM | NETIF_F_IPV6_CSUM;
        pst_net_dev->hw_features |= NETIF_F_IP_CSUM | NETIF_F_IPV6_CSUM;
    }
    if (g_wlan_spec_cfg->rx_checksum_hw_enable) {
        pst_net_dev->features    |= NETIF_F_RXCSUM;
        pst_net_dev->hw_features |= NETIF_F_RXCSUM;
    }
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 44))
    oal_netdevice_master(pst_net_dev) = NULL;
#endif

    oal_netdevice_ifalias(pst_net_dev) = NULL;
    oal_netdevice_watchdog_timeo(pst_net_dev) = 5;
    oal_netdevice_wdev(pst_net_dev) = pst_wdev;
    oal_netdevice_qdisc(pst_net_dev, NULL);
}


static uint32_t wal_cfg80211_param_check(oal_wiphy_stru *wiphy, const char *name, oal_vif_params_stru *params)
{
    uint32_t ret = OAL_TRUE;
#ifdef  _PRE_CONFIG_HISI_S3S4_POWER_STATE
    if (g_st_recovery_info.device_s3s4_process_flag) {
        oam_warning_log0(0, OAM_SF_CFG, "{wal_cfg80211_add_virtual_intf::s3s4 is processing!}\r\n");
        ret = OAL_FALSE;
    }
#endif

    if (oal_any_null_ptr3(wiphy, name, params)) {
        oam_error_log0(0, OAM_SF_CFG, "{wal_cfg80211_add_virtual_intf::wiphy or name or params is null!}");
        ret = OAL_FALSE;
    }

    return ret;
}

/*
 * 函 数 名  : wal_n80211_set_work_mode
 * 功能描述  : 设置 vap p2p 工作模式
 * 1.日    期  : 2020年10月29日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_set_nl80211_work_mode(enum nl80211_iftype type, wlan_p2p_mode_enum_uint8 *p2p_mode,
    wlan_vap_mode_enum_uint8 *vap_mode, oal_wireless_dev_stru **wdev, oal_net_device_stru *net_dev)
{
    switch (type) {
        case NL80211_IFTYPE_ADHOC:
        case NL80211_IFTYPE_AP_VLAN:
        case NL80211_IFTYPE_WDS:
        case NL80211_IFTYPE_MESH_POINT:
        case NL80211_IFTYPE_MONITOR:
            oam_error_log1(0, OAM_SF_CFG, "{wal_n80211_set_work_mode::Unsupported interface type[%d]!}", type);
            return OAL_FAIL;

        case NL80211_IFTYPE_P2P_DEVICE:
            *wdev = net_dev->ieee80211_ptr;
            return OAL_TRUE;

        case NL80211_IFTYPE_P2P_CLIENT:
            *vap_mode = WLAN_VAP_MODE_BSS_STA;
            *p2p_mode = WLAN_P2P_CL_MODE;
            break;
        case NL80211_IFTYPE_STATION:
            *vap_mode = WLAN_VAP_MODE_BSS_STA;
            *p2p_mode = WLAN_LEGACY_VAP_MODE;
            break;
        case NL80211_IFTYPE_P2P_GO:
            *vap_mode = WLAN_VAP_MODE_BSS_AP;
            *p2p_mode = WLAN_P2P_GO_MODE;
            break;
        case NL80211_IFTYPE_AP:
            *vap_mode = WLAN_VAP_MODE_BSS_AP;
            *p2p_mode = WLAN_LEGACY_VAP_MODE;
            break;
        default:
            oam_error_log1(0, OAM_SF_CFG, "{wal_n80211_set_work_mode::Unsupported interface type[%d]!}", type);
            return OAL_FAIL;
    }
    return OAL_SUCC;
}

/*
 * 函 数 名  : wal_cfg80211_wait_del
 * 功能描述  : 等待删除完成
 * 1.日    期  : 2020年10月29日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_cfg80211_wait_del(hmac_device_stru *hmac_device)
{
    int32_t timeout;
    uint8_t rollback_lock = 0;
    if (rtnl_is_locked()) {
        rtnl_unlock();
        rollback_lock = OAL_TRUE;
    }
    oam_warning_log0(0, OAM_SF_ANY, "{wal_cfg80211_wait_del:Released the lock and wait till IF_DEL is complete!}");
    timeout = oal_wait_event_interruptible_timeout_m(hmac_device->st_netif_change_event,
        (hmac_get_p2p_status(hmac_device->p2p_intf_status, P2P_STATUS_IF_DELETING) == OAL_FALSE),
        (uint32_t)oal_msecs_to_jiffies(WAL_MAX_WAIT_TIME));

    /* put back the rtnl_lock again */
    if (rollback_lock) {
        rtnl_lock();
    }

    if (timeout > 0) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_cfg80211_wait_del::IF DEL is Success!}\r\n");
    } else {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_cfg80211_wait_del::timeount < 0, return -EAGAIN!}\r\n");
        return OAL_FAIL;
    }
    return OAL_SUCC;
}

/*
 * 函 数 名  : wal_cfg80211_search_dev
 * 功能描述  : 根据 dev name 找到 dev
 * 1.日    期  : 2020年10月29日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_cfg80211_search_dev(oal_net_device_stru **net_dev, const char *name,
    mac_device_stru *mac_device, oal_wireless_dev_stru **wdev)
{
    hmac_device_stru *hmac_device = NULL;
    uint32_t ret;
    *net_dev = wal_config_get_netdev(name, OAL_STRLEN(name));
    if (*net_dev != NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_cfg80211_search_dev::the net_device is already exist!}\r\n");
        /* 调用oal_dev_get_by_name后，必须调用oal_dev_put使net_dev的引用计数减一 */
        oal_dev_put(*net_dev);
        *wdev = (*net_dev)->ieee80211_ptr;
        return OAL_TRUE;
    }

    /* 添加net_device 前先判断当前是否正在删除net_device 状态，如果正在删除net_device，则等待删除完成，再添加 */
    hmac_device = hmac_res_get_mac_dev(mac_device->uc_device_id);
    if (hmac_device == NULL) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_cfg80211_search_dev::hmac_device is null!}", mac_device->uc_device_id);
        return OAL_FAIL;
    }

    if (hmac_get_p2p_status(hmac_device->p2p_intf_status, P2P_STATUS_IF_DELETING) == OAL_TRUE) {
        /* 等待删除完成 */
        if (wal_cfg80211_wait_del(hmac_device) != OAL_SUCC) {
            return OAL_FAIL;
        }
    }

    /* 检查wifi 驱动中，P2P group 是否已经创建，如果P2P group 已经创建，则将该P2P group 删除，并且重新创建P2P group */
    if (OAL_TRUE == wal_is_p2p_group_exist(mac_device)) {
        oam_warning_log0(0, OAM_SF_CFG, "{wal_cfg80211_search_dev::found exist p2p group, delet it first!}\r\n");
        ret = wal_del_p2p_group(mac_device);
        /* 后续可以考虑不下电，缩短时间 */
        wal_del_vap_try_wlan_pm_close();
        return ret;
    }
    return OAL_SUCC;
}

/* 申请内存 */
OAL_STATIC uint32_t wal_cfg80211_mem_prep(const char *name, oal_net_device_stru **net_dev,
    oal_wireless_dev_stru **wdev)
{
    int8_t local_name[OAL_IF_NAME_SIZE] = { 0 };
    /* 申请内存 */
    if (EOK != memcpy_s(local_name, OAL_IF_NAME_SIZE, name, OAL_STRLEN(name))) {
        oam_error_log0(0, OAM_SF_ANY, "wal_cfg80211_mem_prep::memcpy fail!");
    }

     /* 此函数第一个入参代表私有长度，此处不涉及为0 */
    *net_dev = oal_net_alloc_netdev_mqs(sizeof(oal_netdev_priv_stru), local_name, oal_ether_setup,
        WLAN_NET_QUEUE_BUTT, 1);
    if (oal_unlikely(*net_dev == NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_cfg80211_mem_prep::net_dev null ptr error!}");
        return OAL_FAIL;
    }

    *wdev = (oal_wireless_dev_stru *)oal_mem_alloc_m(OAL_MEM_POOL_ID_LOCAL, sizeof(oal_wireless_dev_stru), OAL_FALSE);
    if (oal_unlikely(*wdev == NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_cfg80211_mem_prep::alloc mem, wdev null ptr!}");
        /* 异常处理，释放内存 */
        oal_net_free_netdev(*net_dev);
        return OAL_FAIL;
    }
    return OAL_SUCC;
}

static uint8_t g_cfg_vap_id = 0;

/*
 * 函 数 名  : wal_cfg80211_get_vap_info
 * 功能描述  : 获取vap 信息, 申请 dev 内存
 * 1.日    期  : 2020年10月29日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_cfg80211_get_vap_info(hmac_vap_stru **cfg_hmac_vap)
{
    mac_vap_stru *cfg_mac_vap = NULL;
    uint8_t cfg_vap_id = g_cfg_vap_id;

    cfg_mac_vap = (mac_vap_stru *)mac_res_get_mac_vap(cfg_vap_id);
    if (cfg_mac_vap == NULL) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_cfg80211_get_vap_info::cfg_mac_vap is null vap_id:%d!}", cfg_vap_id);
        return OAL_FAIL;
    }
    *cfg_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(cfg_vap_id);
    if (*cfg_hmac_vap == NULL) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_cfg80211_get_vap_info::cfg_hmac_vap is null vap_id:%d!}", cfg_vap_id);
        return OAL_FAIL;
    }
    return OAL_SUCC;
}

/* 异常处理 */
OAL_STATIC void wal_cfg80211_exception_handle(oal_wireless_dev_stru *wdev, oal_net_device_stru *net_dev)
{
    /* 先去注册，后释放 */
    wal_cfg80211_unregister_netdev(net_dev);
    oal_mem_free_m(wdev, OAL_FALSE);
}

/*
 * 函 数 名  : wal_cfg80211_infomation_proc_func
 * 功能描述  : 信息处理函数， 抛事件到wal层处理
 * 1.日    期  : 2020年10月29日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_cfg80211_infomation_proc_func(oal_net_device_stru *net_dev, wlan_p2p_mode_enum_uint8 p2p_mode,
    wlan_vap_mode_enum_uint8 vap_mode, oal_wireless_dev_stru *wdev, hmac_vap_stru *cfg_hmac_vap)
{
    int32_t ret;
    mac_cfg_add_vap_param_stru *add_vap_param = NULL;
    wal_msg_write_stru write_msg = { 0 };
    wal_msg_stru *rsp_msg = NULL;
    mac_vap_stru *mac_vap = NULL;
    oal_net_device_stru *cfg_net_dev = cfg_hmac_vap->pst_net_device;
    /* 填写消息 */
    wal_write_msg_hdr_init(&write_msg, WLAN_CFGID_ADD_VAP, sizeof(mac_cfg_add_vap_param_stru));
    add_vap_param = (mac_cfg_add_vap_param_stru *)(write_msg.auc_value);
    add_vap_param->pst_net_dev = net_dev;
    add_vap_param->en_vap_mode = vap_mode;
    add_vap_param->uc_cfg_vap_indx = g_cfg_vap_id;
    add_vap_param->en_p2p_mode = p2p_mode;
    add_vap_param->is_primary_vap = 0;
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    wal_vap_get_wlan_mode_para(&write_msg);
#endif

    /* 发送消息 */
    ret = wal_send_cfg_event(cfg_net_dev, WAL_MSG_TYPE_WRITE, WAL_MSG_WRITE_MSG_HDR_LENGTH +
        sizeof(mac_cfg_add_vap_param_stru), (uint8_t *)&write_msg, OAL_TRUE, &rsp_msg);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ANY, "{wal_cfg80211_infomation_proc_func::wal_send_cfg_event err code:%d!}", ret);
        wal_cfg80211_exception_handle(wdev, net_dev);
        return OAL_FAIL;
    }

    /* 读取返回的错误码 */
    if (wal_check_and_release_msg_resp(rsp_msg) != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ANY,
            "{wal_cfg80211_infomation_proc_func::wal_cfg80211_infomation_proc_func fail:ul_err_code!}");
        wal_cfg80211_exception_handle(wdev, net_dev);
        return OAL_FAIL;
    }

    wal_set_random_mac_to_mib(net_dev); /* set random mac to mib ; for mp12-cb */

    /* 设置netdevice的，在HMAC层被初始化到MIB中 */
    mac_vap = oal_net_dev_priv(net_dev);
    if (oal_unlikely(mac_vap == NULL)) {
        oam_error_log0(0, OAM_SF_ANY,
            "{wal_cfg80211_infomation_proc_func::oal_net_dev_priv(net_dev) is null ptr.}");
        wal_cfg80211_exception_handle(wdev, net_dev);
        return OAL_FAIL;
    }
    oal_set_mac_addr((uint8_t *)oal_netdevice_mac_addr(net_dev), mac_mib_get_StationID(mac_vap));

    /* 设置VAP UP */
    wal_netdev_open(net_dev, OAL_FALSE);

    oam_warning_log1(0, OAM_SF_CFG, "{wal_cfg80211_infomation_proc_func::succ. vap_id[%d]!}\r\n",
        mac_vap->uc_vap_id);
    return OAL_SUCC;
}

/*
 * 函 数 名  : wal_cfg80211_set_info_func
 * 功能描述  : 设置 mac dev
 * 1.日    期  : 2020年10月29日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_cfg80211_set_info_func(oal_wiphy_stru *wiphy, mac_device_stru **mac_device, const char *name,
    enum nl80211_iftype type, oal_vif_params_stru *params)
{
    mac_wiphy_priv_stru *wiphy_priv = NULL;
    /* 1.1 入参检查 */
    if (wal_cfg80211_param_check(wiphy, name, params) != OAL_TRUE) {
        return OAL_FAIL;
    }
    /* 入参检查无异常后赋值，并调用OAL统一接口 */
    wiphy_priv = oal_wiphy_priv(wiphy);
    if (wiphy_priv == NULL) {
        oam_warning_log0(0, OAM_SF_CFG, "{wal_cfg80211_set_info_func::wiphy_priv is null!}\r\n");
        return OAL_FAIL;
    }
    *mac_device = wiphy_priv->pst_mac_device;
    if ((*mac_device) == NULL) {
        oam_warning_log0(0, OAM_SF_CFG, "{wal_cfg80211_set_info_func::mac_device is null!}\r\n");
        return OAL_FAIL;
    }

    oam_warning_log1(0, OAM_SF_CFG, "{wal_cfg80211_set_info_func::type[%d]!}\r\n", type);
    /* 提示:SDT无法打印%s字符串形式 */
    oal_io_print("wal_cfg80211_set_info_func, device name is:%s\n", name);
    return OAL_SUCC;
}

OAL_STATIC void wal_cfg80211_set_net_device_info(oal_wireless_dev_stru *wdev, oal_net_device_stru *net_dev,
    enum nl80211_iftype type, mac_device_stru *mac_device)
{
    /* 对netdevice进行赋值 */
    wal_cfg80211_add_virtual_intf_netdev_init(net_dev, wdev);

    wdev->iftype = type;
    wdev->wiphy = mac_device->pst_wiphy;
    wdev->netdev = net_dev; /* 给wdev 中的net_device 赋值 */
}
/*
 * 函 数 名  : wal_cfg80211_register_netdev
 * 功能描述  : 内核注册指定类型的net_device,用于需要解mutex lock的应用
 * 1.日    期  : 2015年7月24日
  *   修改内容  : 新生成函数
 */
uint32_t wal_cfg80211_register_netdev(oal_net_device_stru *pst_net_dev)
{
    uint8_t uc_rollback_lock = OAL_FALSE;
    uint32_t ret;

    /*  nl80211 netlink pre_do 中会获取rntl_lock互斥锁，
       注册net_device 会获取rntl_lock互斥锁，造成了死锁 */
    if (rtnl_is_locked()) {
        rtnl_unlock();
        uc_rollback_lock = OAL_TRUE;
    }
    wal_init_netdev_priv(pst_net_dev);

    /* 内核注册net_device, 只返回0 */
    ret = (uint32_t)hmac_net_register_netdev(pst_net_dev);

    if (uc_rollback_lock) {
        rtnl_lock();
    }
    /*  END */
    return ret;
}
OAL_STATIC uint32_t wal_cfg80211_ready_register(wlan_p2p_mode_enum_uint8 p2p_mode, wlan_vap_mode_enum_uint8 vap_mode,
    mac_device_stru *mac_device, oal_wireless_dev_stru *wdev, oal_net_device_stru *net_dev)
{
    uint32_t ret;
    if (wlan_vap_mode_legacy_vap(p2p_mode, vap_mode)) {
        /* 如果创建wlan0， 则保存wlan0 为主net_device,p2p0 和p2p-p2p0 MAC 地址从主netdevice 获取 */
        if (mac_device->st_p2p_info.pst_primary_net_device == NULL) {
            /* 创建wlan0 在加载wifi 驱动时，不需要通过该接口 */
            oam_error_log0(0, 0, "{wal_cfg80211_ready_register::should not go here!}\r\n");
            oal_mem_free_m(wdev, OAL_FALSE);
            oal_net_free_netdev(net_dev);
            return OAL_FAIL;
        }
    }

    /* 防止上层不下发mac addr,导致驱动侧p2p-p2p0-x mac addr未初始化 */
    wal_cfg80211_add_p2p_interface_init(net_dev, mac_device);

    oal_netdevice_flags(net_dev) &= ~OAL_IFF_RUNNING; /* 将net device的flag设为down */

    ret = wal_cfg80211_register_netdev(net_dev);
    if (oal_unlikely(ret != OAL_SUCC)) {
        /* 注册不成功，释放资源 */
        oal_mem_free_m(wdev, OAL_FALSE);
        oal_net_free_netdev(net_dev);
        return OAL_FAIL;
    }
    return OAL_SUCC;
}

/*
 * 函 数 名  : wal_cfg80211_add_virtual_intf
 * 功能描述  : 添加指定类型的net_device
 * 1.日    期  : 2014年11月21日
  *   修改内容  : 新生成函数
 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 12, 0))
oal_wireless_dev_stru *wal_cfg80211_add_virtual_intf(oal_wiphy_stru *wiphy, const char *name,
    unsigned char name_assign_type, enum nl80211_iftype type, oal_vif_params_stru *params)
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0))
oal_wireless_dev_stru *wal_cfg80211_add_virtual_intf(oal_wiphy_stru *wiphy, const char *name,
    unsigned char name_assign_type, enum nl80211_iftype type, uint32_t *pul_flags, oal_vif_params_stru *params)
#else
oal_wireless_dev_stru *wal_cfg80211_add_virtual_intf(oal_wiphy_stru *wiphy, const char *name,
    enum nl80211_iftype type, uint32_t *pul_flags, oal_vif_params_stru *params)
#endif
{
    oal_wireless_dev_stru *wdev = NULL;
    wlan_p2p_mode_enum_uint8 p2p_mode;
    wlan_vap_mode_enum_uint8 vap_mode;
    oal_net_device_stru *net_dev = NULL;
    mac_device_stru *mac_device = NULL;
    hmac_vap_stru *cfg_hmac_vap = NULL;
    uint32_t ret;

    // 入参检测，检测无误后设置数据
    if (wal_cfg80211_set_info_func(wiphy, &mac_device, name, type, params) != OAL_SUCC) {
        return ERR_PTR(-OAL_EBUSY);
    }

    net_dev = mac_device->st_p2p_info.pst_p2p_net_device;
    if (net_dev == NULL) {
        return ERR_PTR(-OAL_EBUSY);
    }
    ret = wal_set_nl80211_work_mode(type, &p2p_mode, &vap_mode, &wdev, net_dev);
    if (ret == OAL_FAIL) {
        return ERR_PTR(-OAL_EBUSY);
    } else if (ret == OAL_TRUE) {
        return wdev;
    }

    /* 如果创建的net device已经存在，直接返回 */
    /* 根据dev_name找到dev */
    ret = wal_cfg80211_search_dev(&net_dev, name, mac_device, &wdev);
    if (ret == OAL_FAIL) {
        return ERR_PTR(-OAL_EBUSY);
    } else if (ret == OAL_TRUE) {
        return wdev;
    }

    /* 获取配置VAP 结构 */
    g_cfg_vap_id = mac_device->uc_cfg_vap_id;
    if (wal_cfg80211_get_vap_info(&cfg_hmac_vap) != OAL_SUCC) {
        return ERR_PTR(-OAL_EBUSY);
    }

    /* 申请内存 */
    if (wal_cfg80211_mem_prep(name, &net_dev, &wdev) != OAL_SUCC) {
        return ERR_PTR(-OAL_EBUSY);
    }

    memset_s(wdev, sizeof(oal_wireless_dev_stru), 0, sizeof(oal_wireless_dev_stru));
    wal_cfg80211_set_net_device_info(wdev, net_dev, type, mac_device);
    if (wal_cfg80211_ready_register(p2p_mode, vap_mode, mac_device, wdev, net_dev) != OAL_SUCC) {
        return ERR_PTR(-OAL_EBUSY);
    }

    /* 抛事件到wal层处理 */
    ret = wal_cfg80211_infomation_proc_func(net_dev, p2p_mode, vap_mode, wdev, cfg_hmac_vap);
    g_cfg_vap_id = 0;  // 清除 cfg_vap_indx
    if (ret != OAL_SUCC) {
        return ERR_PTR(-OAL_EBUSY);
    }

    oam_warning_log1(0, OAM_SF_CFG, "{wal_cfg80211_add_virtual_intf::succ. type[%d]!}\r\n", type);
    return wdev;
}

OAL_STATIC oal_bool_enum_uint8 wal_cfg80211_check_is_primary_netdev(oal_wiphy_stru *wiphy,
    oal_net_device_stru *net_dev)
{
    mac_device_stru *mac_device = NULL;
    mac_wiphy_priv_stru *wiphy_priv = NULL;
    wiphy_priv = oal_wiphy_priv(wiphy);
    if (wiphy_priv == NULL) {
        oam_warning_log0(0, OAM_SF_CFG, "{wal_cfg80211_check_is_primary_netdev::pst_wiphy_priv is null!}");
        return OAL_FALSE;
    }
    mac_device = wiphy_priv->pst_mac_device;
    if (mac_device == NULL) {
        oam_warning_log0(0, OAM_SF_CFG, "{wal_cfg80211_check_is_primary_netdev::pst_mac_device is null!}");
        return OAL_FALSE;
    }
    return (mac_device->st_p2p_info.pst_primary_net_device == net_dev) ? OAL_TRUE : OAL_FALSE;
}

static void wal_init_del_vap_param(wal_msg_write_stru *write_msg, oal_net_device_stru *net_dev,
    wlan_p2p_mode_enum_uint8 p2p_mode)
{
    mac_cfg_del_vap_param_stru *del_vap_param = (mac_cfg_del_vap_param_stru *)(write_msg->auc_value);
    del_vap_param->pst_net_dev = net_dev;
    del_vap_param->en_p2p_mode = p2p_mode;
    wal_write_msg_hdr_init(write_msg, WLAN_CFGID_DEL_VAP, sizeof(mac_cfg_del_vap_param_stru));
}


OAL_STATIC int32_t wal_cfg80211_del_virtual_intf_event(oal_net_device_stru *net_dev,
    hmac_device_stru *hmac_device, mac_vap_stru *pst_mac_vap)
{
    /* 异步去注册net_device */
    wal_msg_write_stru write_msg = {0};
    wal_msg_stru *rsp_msg = NULL;
    int32_t l_ret;

    /* 初始化删除vap 参数 */
    wal_init_del_vap_param(&write_msg, net_dev, mac_get_p2p_mode(pst_mac_vap));

    l_ret = wal_send_cfg_event(net_dev, WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_del_vap_param_stru), (uint8_t *)&write_msg, OAL_TRUE, &rsp_msg);
    if (wal_check_and_release_msg_resp(rsp_msg) != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_cfg80211_del_virtual_intf::wal_check_and_release_msg_resp fail}");
    }
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_cfg80211_del_virtual_intf::return err code %d}", l_ret);
        l_ret = -OAL_EFAIL;
    }
    if (!g_wlan_spec_cfg->p2p_device_gc_use_one_vap && l_ret == OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_cfg80211_del_virtual_intf::clear netdev priv.}");
        oal_net_dev_priv(net_dev) = NULL;
    }
    oam_warning_log1(0, OAM_SF_ANY, "wal_cfg80211_del_virtual_intf:p2p_intf_status[%d]", hmac_device->p2p_intf_status);
    return l_ret;
}
int32_t wal_cfg80211_del_virtual_intf(oal_wiphy_stru *pst_wiphy, oal_wireless_dev_stru *pst_wdev)
{
    oal_net_device_stru *net_dev = NULL;
    mac_vap_stru *pst_mac_vap = NULL;
    hmac_vap_stru *pst_hmac_vap = NULL;
    hmac_device_stru *hmac_device = NULL;
    int32_t ret;

    if (oal_unlikely(oal_any_null_ptr2(pst_wiphy, pst_wdev))) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_cfg80211_del_virtual_intf::pst_wiphy or pst_wdev is null !}");
        return -OAL_EINVAL;
    }

    if (wal_cfg80211_dfr_and_s3s4_param_check() == OAL_FALSE) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_cfg80211_del_virtual_intf:: dfr or s3s4 is processing!}");
        return OAL_SUCC;
    }

    net_dev = pst_wdev->netdev;
    if (oal_unlikely(net_dev == NULL || net_dev->ieee80211_ptr == NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_cfg80211_del_virtual_intf::net_dev or ieee80211_ptr null!}");
        return -OAL_EINVAL;
    }

    pst_mac_vap = oal_net_dev_priv(net_dev);
    if (pst_mac_vap == NULL) {
        oam_warning_log1(0, OAM_SF_ANY, "wal_cfg80211_del_virtual_intf:vap mode[%d]", net_dev->ieee80211_ptr->iftype);
        return -OAL_EINVAL;
    }
    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == NULL) {
        oam_error_log1(0, 0, "{wal_cfg80211_del_virtual_intf::get_hmac_vap fail.vap_id[%u]}", pst_mac_vap->uc_vap_id);
        return -OAL_EINVAL;
    }
    if (wal_cfg80211_check_is_primary_netdev(pst_wiphy, net_dev) == OAL_TRUE) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_cfg80211_del_virtual_intf::cannot del primary netdev}");
        return -OAL_EINVAL;
    }
    oal_net_tx_stop_all_queues(net_dev);
    wal_netdev_stop(net_dev);

    if (wal_wireless_iftype_to_mac_p2p_mode(pst_wdev->iftype) == WLAN_P2P_BUTT) {
        oam_error_log0(0, 0, "{wal_cfg80211_del_virtual_intf::wal_wireless_iftype_to_mac_p2p_mode return BUTT}");
        return -OAL_EINVAL;
    }

    /* 设置删除net_device 标识 */
    hmac_device = hmac_res_get_mac_dev(pst_mac_vap->uc_device_id);
    if (hmac_device == NULL) {
        return -OAL_EINVAL;
    }
    hmac_set_p2p_status(&hmac_device->p2p_intf_status, P2P_STATUS_IF_DELETING);

    /* 启动linux work 删除net_device */
    hmac_schedule_del_virtual_intf_work(hmac_device, net_dev);

    ret = wal_cfg80211_del_virtual_intf_event(net_dev, hmac_device, pst_mac_vap);
    wal_del_vap_try_wlan_pm_close();
    return ret;
}
OAL_STATIC void wal_drv_remain_on_channel_debug_info(mac_vap_stru *mac_vap, hmac_remain_on_channel_param_stru *channel)
{
    if (is_not_p2p_listen_printf(mac_vap) == OAL_TRUE) {
        oam_warning_log4(mac_vap->uc_vap_id, OAM_SF_P2P,
            "{wal_drv_remain_on_channel::SUCC! l_channel=%d, duration=%d, roc type=%d, ull_cookie=0x%x!}\r\n",
            channel->uc_listen_channel, channel->listen_duration, channel->en_roc_type, channel->ull_cookie);
    }
}
OAL_STATIC int32_t wal_drv_check_vap_state(mac_device_stru *pst_mac_device, mac_vap_stru *pst_mac_vap,
    wlan_ieee80211_roc_type_uint8 en_roc_type)
{
    mac_vap_stru *pst_other_vap = NULL;
    uint8_t uc_vap_idx;
    for (uc_vap_idx = 0; uc_vap_idx < pst_mac_device->uc_vap_num; uc_vap_idx++) {
        pst_other_vap = mac_res_get_mac_vap(pst_mac_device->auc_vap_id[uc_vap_idx]);
        if (pst_other_vap == NULL) {
            oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_P2P,
                "{wal_drv_remain_on_channel::vap is null! vap id is %d}", pst_mac_device->auc_vap_id[uc_vap_idx]);
            continue;
        }
        if (oal_value_in_valid_range(pst_other_vap->en_vap_state,
            MAC_VAP_STATE_STA_JOIN_COMP, MAC_VAP_STATE_STA_WAIT_ASOC)
            || (pst_other_vap->en_vap_state == MAC_VAP_STATE_ROAMING)
        ) {
            oam_warning_log2(pst_mac_vap->uc_vap_id, OAM_SF_P2P,
                             "{wal_drv_remain_on_channel::invalid ROC[%d] for roaming or connecting state[%d]!}",
                             en_roc_type, pst_other_vap->en_vap_state);
            return -OAL_EFAIL;
        }
    }
    return OAL_SUCC;
}

OAL_STATIC int32_t wal_drv_set_remain_on_channel_check_valid(oal_net_device_stru *netdev, mac_device_stru *mac_device,
    mac_vap_stru *mac_vap, wlan_ieee80211_roc_type_uint8 roc_type)
{
#ifdef _PRE_WLAN_FEATURE_WAPI
    if (wlan_chip_is_support_hw_wapi() == OAL_FALSE && hmac_user_is_wapi_connected(mac_vap->uc_device_id) == OAL_TRUE) {
        oam_warning_log0(0, OAM_SF_CFG, "{stop p2p remaining under wapi!}");
        return -OAL_EINVAL;
    }
#endif /* #ifdef _PRE_WLAN_FEATURE_WAPI */

    /* check all vap state in case other vap is connecting now */
    if (wal_drv_check_vap_state(mac_device, mac_vap, roc_type) != OAL_SUCC) {
        return -OAL_EFAIL;
    }

    /* tx mgmt roc 优先级低,可以被自己的80211 roc以及80211 scan打断 */
    if (mac_vap->en_vap_state == MAC_VAP_STATE_STA_LISTEN) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_P2P,
            "{wal_drv_set_remain_on_channel_check_valid::new roc type[%d],cancel old roc!}", roc_type);
        if (wal_p2p_stop_roc(mac_vap, netdev) < 0) {
            return -OAL_EFAIL;
        }
    }
    return OAL_SUCC;
}


OAL_STATIC int32_t wal_drv_set_remain_on_channel_para(mac_device_stru *mac_device,
    struct ieee80211_channel *chan, uint32_t duration,
    wlan_ieee80211_roc_type_uint8 roc_type, hmac_remain_on_channel_param_stru *remain_on_channel)
{
    int32_t l_channel;

    l_channel = (int32_t)oal_ieee80211_frequency_to_channel((int32_t)chan->center_freq);

    remain_on_channel->uc_listen_channel = (uint8_t)l_channel;
    remain_on_channel->listen_duration = duration;
    remain_on_channel->st_listen_channel = *chan;
    remain_on_channel->en_listen_channel_type = WLAN_BAND_WIDTH_20M;
    remain_on_channel->en_roc_type = roc_type;

    if (chan->band == HISI_IEEE80211_BAND_2GHZ) {
        remain_on_channel->en_band = WLAN_BAND_2G;
    } else if (chan->band == HISI_IEEE80211_BAND_5GHZ) {
        remain_on_channel->en_band = WLAN_BAND_5G;
    } else {
        oam_warning_log1(mac_device->uc_cfg_vap_id, OAM_SF_P2P,
            "{wal_drv_remain_on_channel::wrong band roc_type[%d]!}\r\n", chan->band);
        return -OAL_EINVAL;
    }

    if ((roc_type == IEEE80211_ROC_TYPE_NORMAL) || (roc_type == IEEE80211_ROC_TYPE_HIGH_PRIO_MGMT_RX)) {
        /* 保存cookie 值，下发给HMAC 和DMAC */
        remain_on_channel->ull_cookie = mac_device->st_p2p_info.ull_last_roc_id;
    }

    return OAL_SUCC;
}

OAL_STATIC int32_t wal_drv_remain_on_channel_send_event(oal_net_device_stru *netdev, wal_msg_write_stru *msg)
{
    wal_msg_stru *rsp_msg = NULL;
    uint32_t err_code;
    int32_t l_ret;

    l_ret = wal_send_cfg_event(netdev, WAL_MSG_TYPE_WRITE, WAL_MSG_WRITE_MSG_HDR_LENGTH +
        sizeof(hmac_remain_on_channel_param_stru), (uint8_t *)msg, OAL_TRUE, &rsp_msg);
    if (l_ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_P2P, "{wal_drv_remain_on_channel::send event err:[%d]!}\r\n", l_ret);
        return -OAL_EFAIL;
    }

    /* 读取返回的错误码 */
    err_code = wal_check_and_release_msg_resp(rsp_msg);
    if (err_code != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_CFG, "{wal_drv_remain_on_channel::msg resp err:[%u]!}", err_code);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}

OAL_STATIC void wal_update_last_roc_id(mac_device_stru *mac_device)
{
    mac_device->st_p2p_info.ull_last_roc_id++;
    if (mac_device->st_p2p_info.ull_last_roc_id == 0) {
        mac_device->st_p2p_info.ull_last_roc_id++;
    }
}
/*
 * 函 数 名  : wal_drv_remain_on_channel
 * 功能描述  : 保持在指定信道
 * 1.日    期  : 2014年11月21日
  *   修改内容  : 新生成函数
 * 2.日    期  : 2017年07月19日
  *   修改内容  : 加入参可以区分是何种roc
 */
int32_t wal_drv_remain_on_channel(oal_wireless_dev_stru *wdev,
    struct ieee80211_channel *chan, uint32_t duration, uint64_t *cookie,
    wlan_ieee80211_roc_type_uint8 roc_type)
{
    wal_msg_write_stru msg = { 0 };
    oal_net_device_stru *netdev = wdev->netdev;
    hmac_remain_on_channel_param_stru remain_on_channel = { 0 };
    int32_t l_ret;
    mac_device_stru *mac_device = NULL;
    mac_vap_stru *mac_vap = NULL;

    if (netdev == NULL) {
        oam_error_log0(0, OAM_SF_P2P, "{wal_drv_remain_on_channel::netdev ptr is null!}\r\n");
        return -OAL_EINVAL;
    }

    /* 通过net_device 找到对应的mac_device_stru 结构 */
    mac_vap = oal_net_dev_priv(netdev);
    if (mac_vap == NULL) {
        oam_warning_log0(0, OAM_SF_P2P, "{wal_drv_remain_on_channel::can't get mac vap from netdevice priv data!}\r\n");
        return -OAL_EINVAL;
    }

    mac_device = (mac_device_stru *)mac_res_get_dev(mac_vap->uc_device_id);
    if (mac_device == NULL) {
        oam_error_log0(0, OAM_SF_P2P, "{wal_drv_remain_on_channel::mac_device ptr is null!}\r\n");
        return -OAL_EINVAL;
    }

    l_ret = wal_drv_set_remain_on_channel_check_valid(netdev, mac_device, mac_vap, roc_type);
    if (l_ret != OAL_SUCC) {
        return l_ret;
    }

    /* 2.1 消息参数准备 */
    wal_drv_set_remain_on_channel_para(mac_device, chan, duration, roc_type, &remain_on_channel);

    if ((roc_type == IEEE80211_ROC_TYPE_NORMAL) || (roc_type == IEEE80211_ROC_TYPE_HIGH_PRIO_MGMT_RX)) {
        /* 将cookie+1值提前，保证驱动侧扫描的cookie值一致避免两次扫描cookie值弄混的现象 */
        /* 设置cookie 值 */ /* cookie值上层调用需要判断是否是这次的发送导致的callback */
        wal_update_last_roc_id(mac_device);
        *cookie = mac_device->st_p2p_info.ull_last_roc_id;
    }
    /***************************************************************************
        抛事件到wal层处理
    ***************************************************************************/
    /* 3.1 填写 msg 消息头 */
    wal_write_msg_hdr_init(&msg, WLAN_CFGID_CFG80211_REMAIN_ON_CHANNEL, sizeof(hmac_remain_on_channel_param_stru));

    /* 3.2 填写 msg 消息体 */
    if (EOK != memcpy_s(msg.auc_value, WAL_MSG_WRITE_MAX_LEN,
        &remain_on_channel, sizeof(hmac_remain_on_channel_param_stru))) {
        oam_error_log0(0, OAM_SF_P2P, "wal_drv_remain_on_channel::memcpy fail!");
    }

    /* 3.3 发送消息 */
    l_ret = wal_drv_remain_on_channel_send_event(netdev, &msg);
    if (l_ret != OAL_SUCC) {
        return l_ret;
    }

    if ((roc_type == IEEE80211_ROC_TYPE_NORMAL) || (roc_type == IEEE80211_ROC_TYPE_HIGH_PRIO_MGMT_RX)) {
        /* 上报暂停在指定信道成功 */
        oal_cfg80211_ready_on_channel(wdev, *cookie, chan, duration, GFP_KERNEL);
    }
    // 封装函数降低行数
    wal_drv_remain_on_channel_debug_info(mac_vap, &remain_on_channel);

    return OAL_SUCC;
}

/*
 * 函 数 名  : wal_cfg80211_remain_on_channel
 * 功能描述  : 保持在指定信道
 * 1.日    期  : 2014年11月21日
  *   修改内容  : 新生成函数
 */
int32_t wal_cfg80211_remain_on_channel(oal_wiphy_stru *pst_wiphy,
    oal_wireless_dev_stru *pst_wdev, struct ieee80211_channel *pst_chan, uint32_t duration, uint64_t *pull_cookie)
{
    if (oal_any_null_ptr4(pst_wiphy, pst_wdev, pst_chan, pull_cookie)) {
        oam_error_log0(0, OAM_SF_P2P, "{wal_cfg80211_remain_on_channel::wiphy/wdev/chan/cookie is null!}");
        return -OAL_EINVAL;
    }
    return wal_drv_remain_on_channel(pst_wdev, pst_chan, duration, pull_cookie, IEEE80211_ROC_TYPE_NORMAL);
}

/*
 * 函 数 名  : wal_cfg80211_cancel_remain_on_channel
 * 功能描述  : 停止保持在指定信道
 * 1.日    期  : 2014年11月21日
  *   修改内容  : 新生成函数
 */
int32_t wal_cfg80211_cancel_remain_on_channel(oal_wiphy_stru *pst_wiphy,
    oal_wireless_dev_stru *pst_wdev, uint64_t ull_cookie)
{
    wal_msg_write_stru st_write_msg = { 0 };
    hmac_remain_on_channel_param_stru st_cancel_remain_on_channel = { 0 };
    wal_msg_stru *pst_rsp_msg = NULL;
    uint32_t err_code;
    oal_net_device_stru *pst_netdev = NULL;
    int32_t l_ret;

    if (oal_any_null_ptr2(pst_wiphy, pst_wdev)) {
        oam_error_log0(0, OAM_SF_P2P, "{wal_cfg80211_cancel_remain_on_channel::pst_wiphy or pst_wdev is null!}");
        return -OAL_EINVAL;
    }

    pst_netdev = pst_wdev->netdev;
    if (pst_netdev == NULL) {
        oam_error_log0(0, OAM_SF_P2P, "{wal_cfg80211_cancel_remain_on_channel::pst_netdev ptr is null!}\r\n");
        return -OAL_EINVAL;
    }

    /***************************************************************************
        抛事件到wal层处理
    ***************************************************************************/
    oam_warning_log1(0, OAM_SF_P2P, "wal_cfg80211_cancel_remain_on_channel[0x%x].", ull_cookie);

    /* 3.1 填写 msg 消息头 */
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_CFG80211_CANCEL_REMAIN_ON_CHANNEL,
        sizeof(hmac_remain_on_channel_param_stru));

    /* 3.2 填写 msg 消息体 */
    if (EOK != memcpy_s(st_write_msg.auc_value, sizeof(hmac_remain_on_channel_param_stru),
                        &st_cancel_remain_on_channel, sizeof(hmac_remain_on_channel_param_stru))) {
        oam_error_log0(0, OAM_SF_P2P, "wal_cfg80211_cancel_remain_on_channel::memcpy fail!");
        return -OAL_EFAIL;
    }

    /* 3.3 发送消息 */
    l_ret = wal_send_cfg_event(pst_netdev, WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(hmac_remain_on_channel_param_stru),
                               (uint8_t *)&st_write_msg, OAL_TRUE, &pst_rsp_msg);
    if (l_ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_P2P,
            "{wal_cfg80211_cancel_remain_on_channel::wal_send_cfg_event return err code: [%d]!}", l_ret);
        return -OAL_EFAIL;
    }

    /* 4.1 读取返回的错误码 */
    err_code = wal_check_and_release_msg_resp(pst_rsp_msg);
    if (err_code != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_P2P,
            "{wal_cfg80211_cancel_remain_on_channel::wal_check_and_release_msg_resp fail return err code:%u!}",
            err_code);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}

/*
 * 函 数 名  : wal_find_oldest_cookie
 * 功能描述  : 寻找最早设置的cookie
 * 1.日    期  : 2018年10月29日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint8_t wal_find_oldest_cookie(cookie_arry_stru *pst_cookie_array)
{
    uint8_t uc_loops;
    uint8_t uc_target_index = 0;

    /* 外部保证只有在cookie用满的情况下才会调用该函数，因此初始index选0即可 */
    for (uc_loops = 1; uc_loops < WAL_COOKIE_ARRAY_SIZE; uc_loops++) {
        if (oal_time_after32(pst_cookie_array[uc_target_index].record_time,
            pst_cookie_array[uc_loops].record_time)) {
            uc_target_index = uc_loops;
        }
    }

    return uc_target_index;
}
