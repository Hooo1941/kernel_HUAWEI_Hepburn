/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : Linux cfg80211接口
 * 作    者 :
 * 创建日期 : 2012年11月8日
 */

#include "securec.h"
#include "securectype.h"
#include "oal_kernel_file.h"

#include "mac_ie.h"
#include "hmac_vap.h"
#include "hmac_config.h"
#include "hmac_chan_mgmt.h"
#include "hmac_tx_data.h"
#ifdef _PRE_WLAN_CHBA_MGMT
#include "hmac_chba_mgmt.h"
#endif

#include "wal_linux_cfg80211.h"
#include "wal_linux_netdev_ops.h"
#include "wal_cfg_ioctl.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID              OAM_FILE_ID_WAL_LINUX_CHBA_CFG80211_C

#ifdef _PRE_WLAN_CHBA_MGMT
/*
 * 功能描述  : 解析下发参数，抛事件到hmac进行配置
 * 1.日    期  : 2021年02月06日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
int32_t wal_cfg80211_cfg_chba(oal_wiphy_stru *wiphy, oal_net_device_stru *netdev,
    mac_vap_stru *mac_vap)
{
    mac_chba_param_stru chba_param;
    wal_msg_write_stru write_msg = {0};
    int32_t err;
    if ((g_optimized_feature_switch_bitmap & BIT(CUSTOM_OPTIMIZE_CHBA)) &&
        (oal_strcmp("chba0", netdev->name) == 0)) {
        memset_s(&chba_param, sizeof(mac_chba_param_stru), 0, sizeof(mac_chba_param_stru));
        /* 配置mac地址 */
        oal_set_mac_addr(chba_param.mac_addr, (uint8_t *)oal_netdevice_mac_addr(netdev));
        chba_param.init_channel = mac_vap->st_channel;
        oam_warning_log3(0, 0, "CHBA: start ap, init channel is chan no[%d], bandwidth[%d], band[%d].",
            chba_param.init_channel.uc_chan_number, chba_param.init_channel.en_bandwidth,
            chba_param.init_channel.en_band);
        /* 抛事件到wal层进行配置 */
        wal_write_msg_hdr_init(&write_msg, WLAN_CFGID_START_CHBA, sizeof(mac_chba_param_stru));
        if (memcpy_s(write_msg.auc_value, sizeof(mac_chba_param_stru),
            &chba_param, sizeof(mac_chba_param_stru)) != EOK) {
            return -OAL_EFAIL;
        }
        err = wal_send_cfg_event(netdev, WAL_MSG_TYPE_WRITE, WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_chba_param_stru),
            (uint8_t *)&write_msg, OAL_FALSE, NULL);
        if (err != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_ANY, "{wal_cfg80211_cfg_chba::Fail to config chba vap, error[%d]}", err);
            return -OAL_EFAIL;
        }
    }
    return OAL_SUCC;
}
#endif

void wal_cfg80211_start_ap_set_chba_mode(oal_net_device_stru *netdev, mac_vap_stru *mac_vap)
{
#ifdef _PRE_WLAN_CHBA_MGMT
    if ((g_optimized_feature_switch_bitmap & BIT(CUSTOM_OPTIMIZE_CHBA)) &&
        (oal_strcmp("chba0", netdev->name) == 0)) {
        mac_vap->chba_mode = CHBA_MODE;
        (hmac_get_chba_vap())->mac_vap_id = mac_vap->uc_vap_id;
    }
#endif
}
#ifdef _PRE_WLAN_CHBA_MGMT
/* chba0接口处理 */
int32_t wal_cfg80211_change_intf_chba0_proc(oal_net_device_stru *net_dev, enum nl80211_iftype if_type)
{
    if (oal_unlikely(net_dev == NULL || net_dev->ieee80211_ptr == NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_cfg80211_change_intf_chba0_proc::net_dev or ieee80211_ptr null!}");
        return -OAL_EFAUL;
    }

    if (!(g_optimized_feature_switch_bitmap & BIT(CUSTOM_OPTIMIZE_CHBA)) || if_type != NL80211_IFTYPE_P2P_GO) {
        oam_warning_log1(0, 0, "CHBA: driver not support chba0 change type[%d].", if_type);
        return -OAL_EINVAL;
    }
    oam_warning_log0(0, 0, "CHBA: set mode to GO.");
    if (wal_setup_ap(net_dev) != OAL_SUCC) {
        return -OAL_EINVAL;
    }
    net_dev->ieee80211_ptr->iftype = if_type;
    return OAL_SUCC;
}
#endif
