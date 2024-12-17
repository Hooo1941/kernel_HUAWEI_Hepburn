/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 :特性相关命令
 * 创建日期 : 2022年11月19日
 */

#include "oal_types.h"
#include "wal_config.h"
#include "hmac_vap.h"
#include "mac_mib.h"
#include "oal_util.h"
#include "plat_pm_wlan.h"
#include "hmac_resource.h"
#include "wal_linux_ioctl.h"
#include "hmac_roam_main.h"
#ifdef _PRE_WLAN_FEATURE_WMMAC
#include "hmac_wmmac.h"
#endif
#include "wlan_chip_i.h"
#include "hmac_wapi.h"
#ifdef _PRE_WLAN_CHBA_MGMT
#include "hmac_chba_mgmt.h"
#include "hmac_chba_channel_sequence.h"
#endif
#include "hmac_sae.h"
#include "wal_linux_cfg80211.h"
#include "wal_linux_netdev_ops.h"
#include "hmac_tx_data.h"
#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_WAL_VAP_CFG_IOCTL_C

#ifdef WIFI_DEBUG_ENABLE
uint32_t wal_hipriv_add_vap_parse_vap_mode(int8_t *mode, uint32_t len,
    wlan_vap_mode_enum_uint8 *vap_mode, wlan_p2p_mode_enum_uint8 *p2p_mode)
{
    if (!oal_strcmp("ap", mode)) {
        *vap_mode = WLAN_VAP_MODE_BSS_AP;
    } else if (!oal_strcmp("sta", mode)) {
        *vap_mode = WLAN_VAP_MODE_BSS_STA;
    } else if (!oal_strcmp("p2p_device", mode)) { /* 创建P2P 相关VAP */
        *vap_mode = WLAN_VAP_MODE_BSS_STA;
        *p2p_mode = WLAN_P2P_DEV_MODE;
    } else if (!oal_strcmp("p2p_cl", mode)) {
        *vap_mode = WLAN_VAP_MODE_BSS_STA;
        *p2p_mode = WLAN_P2P_CL_MODE;
    } else if (!oal_strcmp("p2p_go", mode)) {
        *vap_mode = WLAN_VAP_MODE_BSS_AP;
        *p2p_mode = WLAN_P2P_GO_MODE;
    } else {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_add_vap::the mode param is invalid!}");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }
    return OAL_SUCC;
}

OAL_STATIC uint32_t wal_hipriv_add_vap_cmd_analyze(oal_net_device_stru **alloc_net_dev, int8_t *pc_param,
    wlan_vap_mode_enum_uint8 *vap_mode, wlan_p2p_mode_enum_uint8 *p2p_mode)
{
    oal_net_device_stru *net_dev = NULL;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = { 0 };
    int8_t ac_mode[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    uint32_t off_set, result;

    /* pc_param指向新创建的net_device的name, 将其取出存放到ac_name中 */
    result = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_add_vap::wal_get_cmd_one_arg err_code %d!}", result);
        return result;
    }

    /* ac_name length不应超过OAL_IF_NAME_SIZE */
    if (OAL_IF_NAME_SIZE <= OAL_STRLEN(ac_name)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_add_vap:: vap name overlength is %d!}", OAL_STRLEN(ac_name));
        /* 输出错误的vap name信息 */
        oal_print_hex_dump((uint8_t *)ac_name, OAL_IF_NAME_SIZE, HEX_DUMP_GROUP_SIZE, "vap name lengh is overlong:");
        return OAL_FAIL;
    }

    pc_param += off_set;

    /* pc_param 指向'ap|sta', 将其取出放到ac_mode中 */
    result = wal_get_cmd_one_arg(pc_param, ac_mode, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (result != OAL_SUCC) {
        return (uint32_t)result;
    }

    /* 解析ac_mode字符串对应的模式 */
    if (wal_hipriv_add_vap_parse_vap_mode(ac_mode, sizeof(ac_mode), vap_mode, p2p_mode) != OAL_SUCC) {
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    /* 如果创建的net device已经存在，直接返回 */
    /* 根据dev_name找到dev */
    net_dev = wal_config_get_netdev(ac_name, OAL_STRLEN(ac_name));
    if (net_dev != NULL) {
        /* 调用oal_dev_get_by_name后，必须调用oal_dev_put使net_dev的引用计数减一 */
        oal_dev_put(net_dev);
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_add_vap::the net_device is already exist!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    net_dev = oal_net_alloc_netdev_mqs(sizeof(oal_netdev_priv_stru), ac_name,
        oal_ether_setup, WLAN_NET_QUEUE_BUTT, 1); /* 此函数第一个入参代表私有长度，此处不涉及为0 */
    if (oal_unlikely(net_dev == NULL)) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_add_vap::net_dev null ptr error!}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    *alloc_net_dev = net_dev;
    return OAL_SUCC;
}
OAL_STATIC oal_wireless_dev_stru *wal_hipriv_alloc_wdev(oal_net_device_stru *net_dev, mac_device_stru *mac_device,
    wlan_p2p_mode_enum_uint8 p2p_mode, wlan_vap_mode_enum_uint8 vap_mode)
{
    oal_wireless_dev_stru *wdev = NULL;
    wdev = (oal_wireless_dev_stru *)oal_mem_alloc_m(OAL_MEM_POOL_ID_LOCAL,
        sizeof(oal_wireless_dev_stru), OAL_FALSE);
    if (oal_unlikely(wdev == NULL)) {
        return NULL;
    }

    memset_s(wdev, sizeof(oal_wireless_dev_stru), 0, sizeof(oal_wireless_dev_stru));
    wdev->netdev = net_dev;
    wal_set_wdev_iftype(wdev, vap_mode, p2p_mode);

    wdev->wiphy = mac_device->pst_wiphy;
    return wdev;
}
static void wal_set_netdev_param(oal_net_device_stru *net_dev, oal_wireless_dev_stru *wdev)
{
#ifdef CONFIG_WIRELESS_EXT
    net_dev->wireless_handlers = &g_st_iw_handler_def;
#endif /* CONFIG_WIRELESS_EXT */
    net_dev->netdev_ops = &g_st_wal_net_dev_ops;

    oal_netdevice_destructor(net_dev) = oal_net_free_netdev;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 44))
    oal_netdevice_master(net_dev) = NULL;
#endif

    oal_netdevice_ifalias(net_dev) = NULL;
    oal_netdevice_watchdog_timeo(net_dev) = 5; /* 5： watchdog_timeo */
    oal_netdevice_wdev(net_dev) = wdev;
    oal_netdevice_qdisc(net_dev, NULL);

    oal_netdevice_flags(net_dev) &= ~OAL_IFF_RUNNING; /* 将net device的flag设为down */
}
OAL_STATIC uint32_t wal_hipriv_add_vap_event_process(oal_net_device_stru *cfg_net_dev, oal_net_device_stru *net_dev,
    wlan_p2p_mode_enum_uint8 p2p_mode, wlan_vap_mode_enum_uint8 vap_mode)
{
    int32_t ret;
    wal_msg_stru *rsp_msg = NULL;
    uint32_t err_code;
    wal_msg_write_stru write_msg = {0};
    mac_vap_stru *cfg_mac_vap = oal_net_dev_priv(cfg_net_dev);
    /* 抛事件到wal层处理 */
    wal_write_msg_hdr_init(&write_msg, WLAN_CFGID_ADD_VAP, sizeof(mac_cfg_add_vap_param_stru));
    ((mac_cfg_add_vap_param_stru *)&write_msg.auc_value)->pst_net_dev = net_dev;
    ((mac_cfg_add_vap_param_stru *)&write_msg.auc_value)->en_vap_mode = vap_mode;
    ((mac_cfg_add_vap_param_stru *)&write_msg.auc_value)->uc_cfg_vap_indx = cfg_mac_vap->uc_vap_id;
    ((mac_cfg_add_vap_param_stru *)&write_msg.auc_value)->en_p2p_mode = p2p_mode;
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    wal_vap_get_wlan_mode_para(&write_msg);
#endif
    /* 发送消息 */
    ret = wal_send_cfg_event(cfg_net_dev, WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_add_vap_param_stru),
        (uint8_t *)&write_msg, OAL_TRUE, &rsp_msg);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(cfg_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_hipriv_add_vap::err %d!}", ret);
        return (uint32_t)ret;
    }

    /* 读取返回的错误码 */
    err_code = wal_check_and_release_msg_resp(rsp_msg);
    if (err_code != OAL_SUCC) {
        oam_warning_log1(cfg_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_hipriv_add_vap::hmac add vap fail[%u]!}", err_code);
        return err_code;
    }
    return OAL_SUCC;
}

OAL_STATIC uint32_t wal_hipriv_register_netdev(oal_net_device_stru *net_dev, mac_device_stru *mac_device,
    wlan_p2p_mode_enum_uint8 p2p_mode)
{
    uint32_t result;
    int32_t ret;
    wal_msg_stru *rsp_msg = NULL;
    mac_vap_stru *mac_vap = NULL;
    wal_msg_write_stru write_msg = {0};

    if (wal_set_random_mac_to_mib(net_dev) != OAL_SUCC) {
        return OAL_FAIL;
    } /* set random mac to mib ; for mp12-cb */

    /* 设置netdevice的，在HMAC层被初始化到MIB中 */
    mac_vap = oal_net_dev_priv(net_dev);
    if (WLAN_P2P_CL_MODE != p2p_mode) {
        mac_vap->vap_last_state = MAC_VAP_STATE_INIT;
        mac_vap->en_vap_state = MAC_VAP_STATE_INIT;
    }

    if (p2p_mode == WLAN_P2P_DEV_MODE) {
        oal_set_mac_addr((uint8_t *)oal_netdevice_mac_addr(net_dev), mac_mib_get_p2p0_dot11StationID(mac_vap));
        mac_device->st_p2p_info.uc_p2p0_vap_idx = mac_vap->uc_vap_id;
    } else {
        oal_set_mac_addr((uint8_t *)oal_netdevice_mac_addr(net_dev), mac_mib_get_StationID(mac_vap));
    }

    /* 注册net_device */
    result = (uint32_t)hmac_net_register_netdev(net_dev);
    if (oal_unlikely(result != OAL_SUCC)) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_ANY,
            "{wal_hipriv_add_vap::hmac_net_register_netdev return error code %d!}", result);

        /* 抛删除vap事件释放刚申请的vap */
        wal_write_msg_hdr_init(&write_msg, WLAN_CFGID_DEL_VAP, sizeof(mac_cfg_del_vap_param_stru));

        ret = wal_send_cfg_event(net_dev, WAL_MSG_TYPE_WRITE,
            WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_del_vap_param_stru),
            (uint8_t *)&write_msg, OAL_TRUE, &rsp_msg);
        if (wal_check_and_release_msg_resp(rsp_msg) == OAL_SUCC) {
            if (wal_del_vap_try_wlan_pm_close() != OAL_SUCC) {
                oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_add_vap::wal_del_vap_try_wlan_pm_close fail.}");
            }
        } else {
            oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_add_vap::wal_check_and_release_msg_resp fail.}");
        }
        if (ret != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_add_vap::wal_send_cfg_event fail,err code %d!}", ret);
        }
        return result;
    }
    return OAL_SUCC;
}
/*
 * 功能描述  : 私有命令，创建VAP
 */
uint32_t wal_hipriv_add_vap(oal_net_device_stru *cfg_net_dev, int8_t *pc_param)
{
    oal_net_device_stru *net_dev = NULL;
    oal_netdev_priv_stru *net_dev_priv = NULL;
    oal_wireless_dev_stru *wdev = NULL;
    mac_vap_stru *cfg_mac_vap = oal_net_dev_priv(cfg_net_dev);
    mac_device_stru *mac_device = NULL;
    wlan_p2p_mode_enum_uint8 p2p_mode = WLAN_LEGACY_VAP_MODE;
    wlan_vap_mode_enum_uint8 vap_mode;
    uint32_t result;
    /* 获取mac device */
    mac_device = mac_res_get_dev(cfg_mac_vap->uc_device_id);
    if (oal_unlikely(mac_device == NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    result = wal_hipriv_add_vap_cmd_analyze(&net_dev, pc_param, &vap_mode, &p2p_mode);
    if (result != OAL_SUCC) {
        return result;
    }

    wdev = wal_hipriv_alloc_wdev(net_dev, mac_device, p2p_mode, vap_mode);
    if (wdev == NULL) {
        oal_net_free_netdev(net_dev);
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_add_vap::wdev null ptr error!}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    /* 对netdevice进行赋值 */
    wal_set_netdev_param(net_dev, wdev);
    net_dev_priv = (oal_netdev_priv_stru *)oal_net_dev_wireless_priv(net_dev);
    wal_set_netdev_priv(net_dev_priv);
    oal_netbuf_list_head_init(&net_dev_priv->st_rx_netbuf_queue);

    result = wal_hipriv_add_vap_event_process(cfg_net_dev, net_dev, p2p_mode, vap_mode);
    if (result != OAL_SUCC) {
        /* 异常处理，释放内存 */
        oal_mem_free_m(wdev, OAL_FALSE);
        oal_net_free_netdev(net_dev);
        return result;
    }
    if ((p2p_mode == WLAN_LEGACY_VAP_MODE) && (mac_device->st_p2p_info.pst_primary_net_device == NULL)) {
        /* 如果创建wlan0， 则保存wlan0 为主net_device,p2p0 和p2p-p2p0 MAC 地址从主netdevice 获取 */
        mac_device->st_p2p_info.pst_primary_net_device = net_dev;
    }

    result = wal_hipriv_register_netdev(net_dev, mac_device, p2p_mode);
    if (result != OAL_SUCC) {
        /* 异常处理，释放内存 */
        oal_mem_free_m(wdev, OAL_FALSE);
        oal_net_free_netdev(net_dev);
        return result;
    }

    return OAL_SUCC;
}

/*
 * 函 数 名  : wal_hipriv_vap_info
 * 功能描述  : 打印vap的所有参数信息
 * 1.日    期  : 2013年5月28日
  *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_vap_info(oal_net_device_stru *net_dev, int8_t *pc_param)
{
    wal_msg_write_stru write_msg = {0};
    int32_t ret;

    wal_write_msg_hdr_init(&write_msg, WLAN_CFGID_VAP_INFO, sizeof(int32_t));

    ret = wal_send_cfg_event(net_dev, WAL_MSG_TYPE_WRITE, WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(int32_t),
        (uint8_t *)&write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_vap_info::err %d!}", ret);
        return (uint32_t)ret;
    }

    return OAL_SUCC;
}

/*
 * 1.日    期  : 2013年9月6日
  *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_reset_device(oal_net_device_stru *pst_cfg_net_dev, int8_t *pc_param)
{
    wal_msg_write_stru st_write_msg;
    int32_t l_ret;
    uint16_t us_len;

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    if (memcpy_s(st_write_msg.auc_value, sizeof(st_write_msg.auc_value),
        pc_param, OAL_STRLEN(pc_param)) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_hipriv_reset_device::memcpy fail}");
        return OAL_FAIL;
    }

    st_write_msg.auc_value[OAL_STRLEN(pc_param)] = '\0';

    us_len = (uint16_t)(OAL_STRLEN(pc_param) + 1);

    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_RESET_HW, us_len);

    l_ret = wal_send_cfg_event(pst_cfg_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + us_len,
                               (uint8_t *)&st_write_msg,
                               OAL_FALSE,
                               NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_reset_device::return err code [%d]!}\r\n", l_ret);
        return (uint32_t)l_ret;
    }

    return OAL_SUCC;
}

/*
 * 功能描述  : 设置复位开关等
 * 1.日    期  : 2014年11月28日
  *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_reset_operate(oal_net_device_stru *pst_cfg_net_dev, int8_t *pc_param)
{
    wal_msg_write_stru st_write_msg;
    int32_t l_ret;
    uint16_t us_len;

    if (oal_unlikely(WAL_MSG_WRITE_MAX_LEN <= OAL_STRLEN(pc_param))) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_reset_operate:: pc_param overlength is %d}\n", OAL_STRLEN(pc_param));
        return OAL_FAIL;
    }

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    if (memcpy_s(st_write_msg.auc_value, sizeof(st_write_msg.auc_value),
        pc_param, OAL_STRLEN(pc_param)) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_hipriv_reset_operate::memcpy fail!}");
        return OAL_FAIL;
    }

    st_write_msg.auc_value[OAL_STRLEN(pc_param)] = '\0';

    us_len = (uint16_t)(OAL_STRLEN(pc_param) + 1);

    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_RESET_HW_OPERATE, us_len);

    l_ret = wal_send_cfg_event(pst_cfg_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + us_len,
                               (uint8_t *)&st_write_msg,
                               OAL_FALSE,
                               NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_reset_operate::return err code [%d]!}\r\n", l_ret);
        return (uint32_t)l_ret;
    }

    return OAL_SUCC;
}

/*
 * 功能描述  : 是否使能基于vap的流分类功能
 * 1.日    期  : 2014年11月24日
  *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_vap_classify_en(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    return wal_hipriv_send_cfg_uint32_data(pst_net_dev, pc_param, WAL_HIPRIV_CMD_MAX_LEN, WLAN_CFGID_VAP_CLASSIFY_EN);
}

/*
 * 功能描述  : 设置vap的流等级
 * 1.日    期  : 2014年11月24日
  *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_vap_classify_tid(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    return wal_hipriv_send_cfg_uint32_data(pst_net_dev, pc_param, WAL_HIPRIV_CMD_MAX_LEN, WLAN_CFGID_VAP_CLASSIFY_TID);
}
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44))
/*
 * 功能描述  : 设置beacon interval
 * 1.日    期  : 2015年1月20日
  *   修改内容  : 新生成函数
 */
uint32_t wal_ioctl_set_beacon_interval(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    wal_msg_write_stru st_write_msg;
    int32_t l_beacon_interval;
    uint32_t off_set;
    int8_t ac_beacon_interval[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    uint32_t ret;
    int32_t l_ret;

    /* 设备在up状态不允许配置，必须先down */
    if (0 != (OAL_IFF_RUNNING & oal_netdevice_flags(pst_net_dev))) {
        oam_error_log1(0, OAM_SF_CFG,
            "{wal_ioctl_set_beacon_interval::device is busy, please down it firs %d!}\r\n",
            oal_netdevice_flags(pst_net_dev));
        return OAL_ERR_CODE_CONFIG_BUSY;
    }

    /* pc_param指向新创建的net_device的name, 将其取出存放到ac_name中 */
    ret = wal_get_cmd_one_arg(pc_param, ac_beacon_interval, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_CFG,
            "{wal_ioctl_set_beacon_interval::wal_get_cmd_one_arg vap name return err_code %d!}\r\n", ret);
        return ret;
    }

    l_beacon_interval = oal_atoi(ac_beacon_interval);
    oam_info_log1(0, OAM_SF_ANY, "{wal_ioctl_set_beacon_interval::l_beacon_interval = %d!}\r\n", l_beacon_interval);

    /***************************************************************************
        抛事件到wal层处理
    ***************************************************************************/
    /* 填写消息 */
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_BEACON_INTERVAL, sizeof(int32_t));
    *((int32_t *)(st_write_msg.auc_value)) = l_beacon_interval;

    /* 发送消息 */
    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(int32_t),
                               (uint8_t *)&st_write_msg,
                               OAL_FALSE,
                               NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_CFG, "{wal_ioctl_set_beacon_interval::return err code %d!}\r\n", l_ret);
        return (uint32_t)l_ret;
    }

    return OAL_SUCC;
}

/*
 * 功能描述  : 私有up命令，代替ifconfig up。3sta场景时，第三个sta无法up，可用此命令up
 * 1.日    期  : 2015年3月6日
  *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_start_vap(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    oam_error_log0(0, OAM_SF_CFG, "DEBUG:: priv start enter.");
    wal_netdev_open(pst_net_dev, OAL_FALSE);
    return OAL_SUCC;
}
#endif


#endif

/*
 * 函 数 名  : wal_start_vap
 * 1.日    期  : 2015年5月22日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
int32_t wal_start_vap(oal_net_device_stru *net_dev)
{
    wal_msg_write_stru write_msg = {0};
    int32_t ret;
    wal_msg_stru *rsp_msg = NULL;
    uint32_t err_code;
    wlan_p2p_mode_enum_uint8 en_p2p_mode;
    oal_wireless_dev_stru *wdev = NULL;

    if (oal_unlikely(net_dev == NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_start_vap::net_dev null!}");
        return -OAL_EFAUL;
    }
    oal_io_print("wal_start_vap,dev_name is:%s\n", net_dev->name);
    /* 抛事件到wal层处理 */
    wal_write_msg_hdr_init(&write_msg, WLAN_CFGID_START_VAP, sizeof(mac_cfg_start_vap_param_stru));
    ((mac_cfg_start_vap_param_stru *)write_msg.auc_value)->pst_net_dev = net_dev;
    wdev = net_dev->ieee80211_ptr;
    if (wdev == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "wal_start_vap:wdev is NULL");
        return -OAL_EFAUL;
    }
    en_p2p_mode = wal_wireless_iftype_to_mac_p2p_mode(wdev->iftype);
    if (WLAN_P2P_BUTT == en_p2p_mode) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_start_vap::wal_wireless_iftype_to_mac_p2p_mode return BUFF}");
        return -OAL_EINVAL;
    }
    ((mac_cfg_start_vap_param_stru *)write_msg.auc_value)->en_p2p_mode = en_p2p_mode;
    oam_warning_log1(0, OAM_SF_ANY, "{wal_start_vap::en_p2p_mode:%d}", en_p2p_mode);
    ((mac_cfg_start_vap_param_stru *)write_msg.auc_value)->en_mgmt_rate_init_flag = OAL_TRUE;

    /* 发送消息 */
    ret = wal_send_cfg_event(net_dev, WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_start_vap_param_stru), (uint8_t *)&write_msg, OAL_TRUE, &rsp_msg);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_start_vap::wal_alloc_cfg_event return err code %d!}", ret);
        return ret;
    }
    /* 处理返回消息 */
    err_code = wal_check_and_release_msg_resp(rsp_msg);
    if (err_code != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_start_vap::hmac start vap fail, err code[%d]!}", err_code);
        return -OAL_EINVAL;
    }
    if ((oal_netdevice_flags(net_dev) & OAL_IFF_RUNNING) == 0) {
        oal_netdevice_flags(net_dev) |= OAL_IFF_RUNNING;
    }
    /* AP模式,启动VAP后,启动发送队列 */
    oal_net_tx_wake_all_queues(net_dev); /* 启动发送队列 */
    return OAL_SUCC;
}

/*
 * 函 数 名  : wal_stop_vap
 * 功能描述  : 停用vap
 * 1.日    期  : 2015年5月25日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
int32_t wal_stop_vap(oal_net_device_stru *net_dev)
{
    wal_msg_write_stru write_msg = {0};
    wal_msg_stru *rsp_msg = NULL;
    int32_t ret;
    wlan_p2p_mode_enum_uint8 en_p2p_mode;
    oal_wireless_dev_stru *wdev = NULL;

    if (oal_unlikely(net_dev == NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_stop_vap::net_dev null!}");
        return -OAL_EFAUL;
    }

    if ((oal_netdevice_flags(net_dev) & OAL_IFF_RUNNING) == 0) {
        /*  解决vap状态与netdevice状态不一致，无法down vap的问题 */
        oam_warning_log0(0, OAM_SF_ANY, "{wal_stop_vap::vap is already down,continue to reset hmac vap state.}");
    }

    oal_io_print("wal_stop_vap,dev_name is:%s\n", net_dev->name);
    /* 抛事件到wal层处理 */
    wal_write_msg_hdr_init(&write_msg, WLAN_CFGID_DOWN_VAP, sizeof(mac_cfg_down_vap_param_stru));
    ((mac_cfg_down_vap_param_stru *)write_msg.auc_value)->pst_net_dev = net_dev;
    wdev = net_dev->ieee80211_ptr;
    if (wdev == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "wal_stop_vap:wdev is NULL");
        return -OAL_EFAUL;
    }
    en_p2p_mode = wal_wireless_iftype_to_mac_p2p_mode(wdev->iftype);
    if (WLAN_P2P_BUTT == en_p2p_mode) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_stop_vap::wal_wireless_iftype_to_mac_p2p_mode return BUFF}");
        return -OAL_EINVAL;
    }
    ((mac_cfg_start_vap_param_stru *)write_msg.auc_value)->en_p2p_mode = en_p2p_mode;

    oam_warning_log1(0, OAM_SF_ANY, "{wal_stop_vap::en_p2p_mode:%d}", en_p2p_mode);

    /* 发送消息 */
    ret = wal_send_cfg_event(net_dev, WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_down_vap_param_stru), (uint8_t *)&write_msg, OAL_TRUE, &rsp_msg);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_stop_vap::wal_alloc_cfg_event return err code %d!}", ret);
        return ret;
    }

    if (OAL_SUCC != wal_check_and_release_msg_resp(rsp_msg)) {
        oam_warning_log0(0, OAM_SF_ANY, "wal_stop_vap::wal_check_and_release_msg_resp fail");
    }

    return OAL_SUCC;
}

/*
 * 功能描述  : 删除vap抛事件到wal层处理
 * 1.日    期  : 2020年8月13日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_hipriv_del_vap_config(oal_net_device_stru *net_dev)
{
    wal_msg_write_stru write_msg = {0};
    wlan_p2p_mode_enum_uint8 p2p_mode;
    wal_msg_stru *rsp_msg = NULL;
    oal_wireless_dev_stru *wdev = NULL;
    mac_vap_stru *mac_vap = NULL;
    int32_t ret;

    if (oal_unlikely(net_dev == NULL || net_dev->ieee80211_ptr == NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_start_vap::net_dev or ieee80211_ptr null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    mac_vap = oal_net_dev_priv(net_dev);
    if (mac_vap == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }
    memset_s(&write_msg, sizeof(wal_msg_write_stru), 0, sizeof(wal_msg_write_stru));

    ((mac_cfg_del_vap_param_stru *)write_msg.auc_value)->pst_net_dev = net_dev;
    wdev = net_dev->ieee80211_ptr;
    p2p_mode = wal_wireless_iftype_to_mac_p2p_mode(wdev->iftype);
    if (p2p_mode == WLAN_P2P_BUTT) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_hipriv_del_vap_config::wal_wireless_iftype_to_mac_p2p_mode return BUFF}");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }
    ((mac_cfg_del_vap_param_stru *)write_msg.auc_value)->en_p2p_mode = p2p_mode;

    wal_write_msg_hdr_init(&write_msg, WLAN_CFGID_DEL_VAP, sizeof(mac_cfg_del_vap_param_stru));

    ret = wal_send_cfg_event(net_dev, WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_del_vap_param_stru), (uint8_t *)&write_msg, OAL_TRUE, &rsp_msg);
    if (wal_check_and_release_msg_resp(rsp_msg) == OAL_SUCC) {
        if (wal_del_vap_try_wlan_pm_close() != OAL_SUCC) {
            oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_del_vap_config::wal_del_vap_try_wlan_pm_close fail}");
        }
    } else {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_del_vap_config::wal_check_and_release_msg_resp fail}");
    }

    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_del_vap_config::err %d}", ret);
#if (_PRE_TEST_MODE != _PRE_TEST_MODE_UT)
        /* 去注册 */
        oal_net_unregister_netdev(net_dev);
        oal_mem_free_m(wdev, OAL_TRUE);
#endif
        return (uint32_t)ret;
    }
#if (_PRE_TEST_MODE != _PRE_TEST_MODE_UT)
    /* 去注册 */
    oal_net_unregister_netdev(net_dev);
    oal_mem_free_m(wdev, OAL_TRUE);
#endif
    return OAL_SUCC;
}

/*
 * 功能描述  : 删除vap
 * 1.日    期  : 2013年5月13日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_del_vap(oal_net_device_stru *net_dev, int8_t *pc_param)
{
    uint32_t ret;
    mac_vap_stru *mac_vap = NULL;
    if (net_dev == NULL) {
        // 访问网络接口的模块可能不止一个,需要上层保证可靠删除
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_del_vap::net_dev is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 设备在up状态不允许删除，必须先down */
    if (oal_unlikely(0 != (OAL_IFF_RUNNING & oal_netdevice_flags(net_dev)))) {
        oam_error_log1(0, OAM_SF_ANY, "{wal_hipriv_del_vap::dev busy, down it first:%d}", oal_netdevice_flags(net_dev));
        return OAL_ERR_CODE_CONFIG_BUSY;
    }

    mac_vap = oal_net_dev_priv(net_dev);
    if (oal_unlikely(mac_vap == NULL)) {
        oam_error_log0(0, OAM_SF_CFG, "{wal_hipriv_del_vap::can't get mac vap from netdevice priv data!}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    if (mac_vap->en_vap_mode == WLAN_VAP_MODE_CONFIG) { /* 配置vap不走命令删除接口 */
        oam_warning_log0(0, OAM_SF_CFG, "{wal_hipriv_del_vap::invalid para,cfg vap!}");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    /* 设备在up状态不允许删除，必须先down */
    if ((mac_vap->en_vap_state != MAC_VAP_STATE_INIT) && (mac_vap->en_p2p_mode != WLAN_P2P_CL_MODE)) {
        oam_error_log1(0, OAM_SF_ANY, "{wal_hipriv_del_vap::device busy, down it first %d!}", mac_vap->en_vap_state);
        return OAL_ERR_CODE_CONFIG_BUSY;
    }

    /* 发消息到wal层处理 */
    ret = wal_hipriv_del_vap_config(net_dev);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_del_vap::send msg fail!}");
        return ret;
    }

    return OAL_SUCC;
}

static oal_bool_enum_uint8 wal_sniffer_up_vap_is_p2p_mode(mac_device_stru *mac_device)
{
    mac_vap_stru *mac_vap = NULL;
    uint8_t vap_idx;

    for (vap_idx = 0; vap_idx < mac_device->uc_vap_num; vap_idx++) {
        mac_vap = (mac_vap_stru *)mac_res_get_mac_vap(mac_device->auc_vap_id[vap_idx]);
        if (mac_vap == NULL) {
            oam_error_log1(0, OAM_SF_ANY, "vap is null, vap id is %d", mac_device->auc_vap_id[vap_idx]);
            continue;
        }

        if (mac_vap->en_vap_state == MAC_VAP_STATE_UP && ((mac_vap->en_p2p_mode == WLAN_P2P_GO_MODE) ||
            (mac_vap->en_p2p_mode == WLAN_P2P_CL_MODE))) {
            oam_warning_log1(0, OAM_SF_ANY,
                "{wal_sniffer_up_vap_is_p2p_mode::has up p2p vap[%d].}", mac_vap->en_p2p_mode);
            return OAL_TRUE;
        }
    }

    return OAL_FALSE;
}

/*
 * 功能描述  : 判断sniffer模式是否处在单STA模式
 * 1.日    期  : 2020年12月9日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
oal_bool_enum_uint8 wal_sniffer_is_single_sta_mode(oal_net_device_stru *net_dev)
{
    oal_wireless_dev_stru *wdev = NULL;
    mac_wiphy_priv_stru *priv = NULL;
    mac_device_stru *mac_device = NULL;
    mac_vap_stru *mac_vap = NULL;
    uint8_t up_vap_num;

    wdev = oal_netdevice_wdev(net_dev);
    if (wdev == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_sniffer_is_single_sta_mode::pst_wdev is null!}");
        return OAL_FALSE;
    }

    priv = (mac_wiphy_priv_stru *)oal_wiphy_priv(wdev->wiphy);
    if (priv == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_sniffer_is_single_sta_mode::pst_mac_device is null!}");
        return OAL_FALSE;
    }
    mac_device = priv->pst_mac_device;
    if (mac_device == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_sniffer_is_single_sta_mode::pst_mac_device is null!}");
        return OAL_FALSE;
    }
    // 计算当前mac_device下处在up状态的vap个数
    // sniffer场景:打开wifi，关联状态:vap 处于up状态；不关联:fake up，此时p2p_device也处于fake up
    up_vap_num = mac_device_get_up_vap_num(mac_device);
    if (up_vap_num > 1) { // 有多个处于up状态的vap返回；等于0说明没有处于up状态的vap，wlan开启但没有关联属于正常场景
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_sniffer_is_single_sta_mode::up_vap_num[%d] invalid, can not start sniffer!}", up_vap_num);
        return OAL_FALSE;
    } else if (up_vap_num == 1) {
        if (wal_sniffer_up_vap_is_p2p_mode(mac_device)) { // 遍历mac_device下所有vap，如果单vap up为p2p，返回
            oam_warning_log0(0, OAM_SF_ANY, "{wal_sniffer_is_single_sta_mode::single up vap is p2p, stop sniffer!}");
            return OAL_FALSE;
        }
    }

    mac_vap = oal_net_dev_priv(net_dev);
    if (mac_vap == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_sniffer_is_single_sta_mode::mac vap null, can not start sniffer!}");
        return OAL_FALSE;
    }
    if (!is_legacy_sta(mac_vap)) { // 单vap up但不是sta或者为p2p up，返回
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_sniffer_is_single_sta_mode::vap mode[%d]not sta, can not start sniffer!}", mac_vap->en_vap_mode);
        return OAL_FALSE;
    }

    return OAL_TRUE;
}

/*
 * 1.日    期  : 2014年12月31日
  *   修改内容  : 新生成函数
 */
wlan_p2p_mode_enum_uint8 wal_wireless_iftype_to_mac_p2p_mode(enum nl80211_iftype if_type)
{
    wlan_p2p_mode_enum_uint8 en_p2p_mode = WLAN_LEGACY_VAP_MODE;

    switch (if_type) {
        case NL80211_IFTYPE_P2P_CLIENT:
            en_p2p_mode = WLAN_P2P_CL_MODE;
            break;
        case NL80211_IFTYPE_P2P_GO:
            en_p2p_mode = WLAN_P2P_GO_MODE;
            break;
        case NL80211_IFTYPE_P2P_DEVICE:
            en_p2p_mode = WLAN_P2P_DEV_MODE;
            break;
        case NL80211_IFTYPE_AP:
        case NL80211_IFTYPE_STATION:
            en_p2p_mode = WLAN_LEGACY_VAP_MODE;
            break;
        default:
            en_p2p_mode = WLAN_P2P_BUTT;
    }
    return en_p2p_mode;
}
int32_t wal_set_random_mac_to_mib_event(oal_net_device_stru *net_dev, mac_vap_stru *mac_vap,
    wlan_p2p_mode_enum_uint8 en_p2p_mode)
{
    uint32_t result;
    frw_event_mem_stru *evt_mem = NULL;
    wal_msg_stru *cfg_msg = NULL;
    wal_msg_write_stru *write_msg = NULL;
    mac_cfg_staion_id_param_stru *pst_param = NULL;
    uint8_t *puc_mac_addr = NULL;

    result = wal_alloc_cfg_event(net_dev, &evt_mem, NULL, &cfg_msg,
        (WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_staion_id_param_stru))); /* 申请事件 */
    if (oal_unlikely(result != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_set_random_mac_to_mib() fail; return %d!}", result);
        return -OAL_ENOMEM;
    }

    /* 填写配置消息 */
    wal_cfg_msg_hdr_init(&(cfg_msg->st_msg_hdr), WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_staion_id_param_stru), (uint8_t)wal_get_msg_sn());

    /* 填写WID消息 */
    write_msg = (wal_msg_write_stru *)cfg_msg->auc_msg_data;
    wal_write_msg_hdr_init(write_msg, WLAN_CFGID_STATION_ID, sizeof(mac_cfg_staion_id_param_stru));

    pst_param = (mac_cfg_staion_id_param_stru *)write_msg->auc_value; /* 填写WID对应的参数 */
    /* 如果使能P2P，需要将netdevice 对应的P2P 模式在配置参数中配置到hmac 和dmac */
    /* 以便底层识别配到p2p0 或p2p-p2p0 cl */
    pst_param->en_p2p_mode = en_p2p_mode;
    if (en_p2p_mode == WLAN_P2P_DEV_MODE) {
        puc_mac_addr = mac_mib_get_p2p0_dot11StationID(mac_vap);
    } else {
        puc_mac_addr = mac_mib_get_StationID(mac_vap);
    }
    oal_set_mac_addr(pst_param->auc_station_id, puc_mac_addr);

    frw_event_dispatch_event(evt_mem); /* 分发事件 */
    frw_event_free_m(evt_mem);

    return OAL_SUCC;
}
/*
 * 函 数 名  : wal_set_random_mac_to_mib for mp12-cb
 * 1.日    期  : 2014年10月29日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 * 2.日    期  : 2014年12月30日
 *   修改内容  : 针对wlan/p2p 共存情况下，p2p0和p2p-p2p0 cl 公用VAP 结构，
 *                将p2p0 MAC 地址保存到mib auc_p2p0_dot11StationID 中。
 */
int32_t wal_set_random_mac_to_mib(oal_net_device_stru *net_dev)
{
    mac_vap_stru *mac_vap = NULL;
    wlan_p2p_mode_enum_uint8 en_p2p_mode = WLAN_LEGACY_VAP_MODE;
    uint8_t *auc_wlan_addr = NULL;
    uint8_t *auc_p2p0_addr = NULL;
    int32_t ret = EOK;

    if (oal_unlikely(net_dev == NULL || net_dev->ieee80211_ptr == NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_set_random_mac_to_mib::net_dev or ieee80211_ptr null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    mac_vap = oal_net_dev_priv(net_dev);
    if (mac_vap == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_set_random_mac_to_mib::mac_vap NULL}");
        return OAL_FAIL;
    }

    if (mac_vap->pst_mib_info == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_set_random_mac_to_mib::vap->mib_info is NULL !}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    auc_wlan_addr = mac_mib_get_StationID(mac_vap);
    auc_p2p0_addr = mac_mib_get_p2p0_dot11StationID(mac_vap);

    switch (net_dev->ieee80211_ptr->iftype) { /* 对于P2P 场景，p2p0 和 p2p-p2p0 MAC 地址从wlan0 获取 */
        case NL80211_IFTYPE_P2P_DEVICE:
            en_p2p_mode = WLAN_P2P_DEV_MODE;
            /* 产生P2P device MAC 地址，将本地mac 地址bit 设置为1 */
            ret += memcpy_s(auc_p2p0_addr, WLAN_MAC_ADDR_LEN, oal_netdevice_mac_addr(net_dev), WLAN_MAC_ADDR_LEN);

            oam_warning_log3(0, OAM_SF_ANY, "wal_set_random_mac_to_mib:p2p0 mac is:%02X:XX:XX:XX:%02X:%02X\n",
                auc_p2p0_addr[0], auc_p2p0_addr[4], auc_p2p0_addr[5]); /* 4 5 macaddr */
            break;
        case NL80211_IFTYPE_P2P_CLIENT:
        case NL80211_IFTYPE_P2P_GO:
            en_p2p_mode =
                (net_dev->ieee80211_ptr->iftype == NL80211_IFTYPE_P2P_GO) ? WLAN_P2P_GO_MODE : WLAN_P2P_CL_MODE;
            /* 根据上层下发值，产生P2P interface MAC 地址 */
            /* 上层不下发，跟随主mac地址,在wal_cfg80211_add_p2p_interface_init初始化 */
            ret += memcpy_s(auc_wlan_addr, WLAN_MAC_ADDR_LEN, oal_netdevice_mac_addr(net_dev), WLAN_MAC_ADDR_LEN);
            break;
        default:
            if (0 == (oal_strcmp("p2p0", net_dev->name))) {
                en_p2p_mode = WLAN_P2P_DEV_MODE;
                /* 产生P2P device MAC 地址，将本地mac 地址bit 设置为1 */
                ret += memcpy_s(auc_p2p0_addr, WLAN_MAC_ADDR_LEN, oal_netdevice_mac_addr(net_dev), WLAN_MAC_ADDR_LEN);
                break;
            }

            en_p2p_mode = WLAN_LEGACY_VAP_MODE;
            ret += memcpy_s(auc_wlan_addr, WLAN_MAC_ADDR_LEN, oal_netdevice_mac_addr(net_dev), WLAN_MAC_ADDR_LEN);
            break;
    }
    if (ret != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "wal_set_random_mac_to_mib::memcpy fail!");
        return OAL_FAIL;
    }
    return wal_set_random_mac_to_mib_event(net_dev, mac_vap, en_p2p_mode);
}

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
/*
 * 函 数 名  : wal_init_probe_resp_status
 * 功能描述  : 根据ini文件设置probe_resp模式
 * 1.日    期  : 2019年9月16日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
#define WAL_PROBE_RESP_LEN 4
OAL_STATIC void wal_init_probe_resp_status(mac_cfg_add_vap_param_stru *write_msg)
{
    uint32_t ini_probe_resp_mode;
    oal_bool_enum_uint8 probe_resp_enable;
    mac_probe_resp_mode_enum probe_resp_status;

    if (hmac_get_feature_switch(HMAC_MIRACAST_SINK_SWITCH)) {
        ini_probe_resp_mode = wlan_chip_get_probe_resp_mode();

        probe_resp_enable  = !!(oal_mask(WAL_PROBE_RESP_LEN, BYTE_OFFSET_4) & ini_probe_resp_mode);
        probe_resp_status  = (oal_mask(WAL_PROBE_RESP_LEN, BYTE_OFFSET_0) & ini_probe_resp_mode);

        write_msg->probe_resp_enable = probe_resp_enable;
        if (probe_resp_status >= MAC_PROBE_RESP_MODE_BUTT) {
            oam_warning_log1(0, OAM_SF_ANY, "{wal_init_probe_resp_status::read ini->probe_resp_status error[%d]}",
                probe_resp_status);
            probe_resp_status = MAC_PROBE_RESP_MODE_ACTIVE;
        }
        write_msg->probe_resp_status = probe_resp_status;
    }
}
#endif
static int32_t wal_wlan_vap_init_get_base_dev_mac_stru(oal_net_device_stru *net_dev, mac_vap_stru **mac_vap,
                                                       oal_wireless_dev_stru **pst_wdev,
                                                       mac_wiphy_priv_stru **wiphy_priv,
                                                       mac_device_stru **mac_device)
{
    *mac_vap = oal_net_dev_priv(net_dev);
    if (*mac_vap != NULL) {
        if ((*mac_vap)->en_vap_state != MAC_VAP_STATE_BUTT) {
            oam_warning_log0(0, OAM_SF_ANY, "{wal_wlan_vap_init_get_base_dev_mac_stru::mac_vap is already exist}");
            return OAL_SUCC;
        }
        /* netdev下的vap已经被删除，需要重新创建和挂载 */
        oam_warning_log0((*mac_vap)->uc_vap_id, OAM_SF_ANY,
                         "{wal_wlan_vap_init_get_base_dev_mac_stru::mac_vap is already free!}");
        oal_net_dev_priv(net_dev) = NULL;
    }
    *pst_wdev = oal_netdevice_wdev(net_dev);
    if (*pst_wdev == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_wlan_vap_init_get_base_dev_mac_stru::pst_wdev is null!}");
        return -OAL_EFAUL;
    }

    *wiphy_priv = (mac_wiphy_priv_stru *)oal_wiphy_priv((*pst_wdev)->wiphy);
    if ((*wiphy_priv) == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "wal_wlan_vap_init_get_base_dev_mac_stru:*wiphy_priv is NULL");
        return -OAL_EFAUL;
    }
    *mac_device = (*wiphy_priv)->pst_mac_device;
    if (*mac_device == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_wlan_vap_init_get_base_dev_mac_stru::pst_mac_device is null!}");
        return -OAL_EFAUL;
    }
    return OAL_SUCC_GO_ON;
}

static int32_t wal_wlan_vap_init_get_cfg_stru(mac_device_stru *mac_device, mac_vap_stru **cfg_mac_vap,
    hmac_vap_stru **pst_cfg_hmac_vap, oal_net_device_stru **cfg_net_dev, wal_msg_write_stru *write_msg)
{
    *cfg_mac_vap = (mac_vap_stru *)mac_res_get_mac_vap(mac_device->uc_cfg_vap_id);
    *pst_cfg_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_device->uc_cfg_vap_id);
    if (oal_any_null_ptr2(*cfg_mac_vap, *pst_cfg_hmac_vap)) {
        oam_warning_log2(0, OAM_SF_ANY,
            "{wal_wlan_vap_init_get_cfg_stru::cfg_mac_vap or pst_cfg_hmac_vap is null! vap_id:%d,deviceid[%d]}",
            mac_device->uc_cfg_vap_id, mac_device->uc_device_id);
        return -OAL_EFAUL;
    }

    *cfg_net_dev = (*pst_cfg_hmac_vap)->pst_net_device;
    if (*cfg_net_dev == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_wlan_vap_init_get_cfg_stru::cfg_net_dev is null!}");
        return -OAL_EFAUL;
    }
    wal_write_msg_hdr_init(write_msg, WLAN_CFGID_ADD_VAP, sizeof(mac_cfg_add_vap_param_stru));
    return OAL_SUCC_GO_ON;
}

static int32_t wal_wlan_mac_cfg_add_vap_param_set(mac_device_stru *mac_device,
                                                  mac_cfg_add_vap_param_stru *add_vap_param,
                                                  oal_wireless_dev_stru *pst_wdev, oal_net_device_stru *net_dev)
{
    int32_t ret;
    int8_t ac_wlan_netdev_name[MAC_NET_DEVICE_NAME_LENGTH];
    int8_t ac_p2p_netdev_name[MAC_NET_DEVICE_NAME_LENGTH];
    char *wlan_netdev_name = ac_wlan_netdev_name;
    char *p2p_netdev_name = ac_p2p_netdev_name;
    /* 通过device id获取wlanX和 p2pX的netdev名 */
    snprintf_s(wlan_netdev_name, NET_DEV_NAME_LEN, NET_DEV_NAME_LEN - 1, "wlan%u", mac_device->uc_device_id);
    snprintf_s(p2p_netdev_name, NET_DEV_NAME_LEN, NET_DEV_NAME_LEN - 1, "p2p%u", mac_device->uc_device_id);
    add_vap_param->en_p2p_mode = WLAN_LEGACY_VAP_MODE;
    /* 仅用于WIFI和AP打开时创建VAP */
    if ((NL80211_IFTYPE_STATION == pst_wdev->iftype) || (NL80211_IFTYPE_P2P_DEVICE == pst_wdev->iftype)) {
        if (oal_any_zero_value2(oal_strcmp(wlan_netdev_name, net_dev->name),
            oal_strcmp(WLAN1_NETDEV_NAME, net_dev->name))) {
            add_vap_param->en_vap_mode = WLAN_VAP_MODE_BSS_STA;
        } else if (0 == (oal_strcmp(p2p_netdev_name, net_dev->name))) {
            add_vap_param->en_vap_mode = WLAN_VAP_MODE_BSS_STA;
            add_vap_param->en_p2p_mode = WLAN_P2P_DEV_MODE;
        } else {
            oam_warning_log1(0, OAM_SF_ANY, "{wal_wlan_mac_cfg_add_vap_param_set::net_dev is not wlan or p2p[%d]!}",
                mac_device->uc_device_id);
            return OAL_SUCC;
        }
    } else if (NL80211_IFTYPE_AP == pst_wdev->iftype) {
        add_vap_param->en_vap_mode = WLAN_VAP_MODE_BSS_AP;
    } else {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_wlan_mac_cfg_add_vap_param_set::net_dev is not wlan0 or p2p0!}");
        return OAL_SUCC;
    }

    oam_warning_log2(0, OAM_SF_ANY, "{wal_wlan_mac_cfg_add_vap_param_set::vap_mode:%d, p2p_mode:%d}",
        add_vap_param->en_vap_mode, add_vap_param->en_p2p_mode);

    if (oal_strcmp(wlan_netdev_name, net_dev->name) == 0) {
        add_vap_param->is_primary_vap = 1;
#ifdef _PRE_WLAN_FEATURE_TAS_ANT_SWITCH
        ret = wal_init_wifi_tas_state();
        if (ret != BOARD_SUCC) {
            oam_warning_log0(0, OAM_SF_ANY, "{wal_init_WiTAS_state ini WiTAS state fail!}");
        }
#endif
    } else {
        add_vap_param->is_primary_vap = 0;
    }
    return OAL_SUCC_GO_ON;
}

static void wal_wlan_vap_init_post_event_msg(oal_net_device_stru *net_dev,
    mac_cfg_add_vap_param_stru *add_vap_param, mac_vap_stru *cfg_mac_vap, wal_msg_write_stru *write_msg)
{
#ifdef _PRE_WLAN_CHBA_MGMT
    /* 如果是chba vap，将该模式同步到dmac */
    if ((g_optimized_feature_switch_bitmap & BIT(CUSTOM_OPTIMIZE_CHBA)) &&
        (oal_strcmp("chba0", net_dev->name) == 0)) {
        add_vap_param->chba_mode = CHBA_MODE;
    }
#endif
    /* 抛事件到wal层处理 */
    add_vap_param->pst_net_dev = net_dev;
    add_vap_param->uc_cfg_vap_indx = cfg_mac_vap->uc_vap_id;
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    wal_vap_get_wlan_mode_para(write_msg);
    wal_init_probe_resp_status((mac_cfg_add_vap_param_stru *)write_msg->auc_value);
#endif
}

static int32_t wal_init_wlan_vap_post_event_deal(oal_net_device_stru *net_dev, oal_net_device_stru *cfg_net_dev,
                                                 mac_vap_stru *cfg_mac_vap, wal_msg_write_stru *write_msg)
{
    uint32_t err_code;
    int32_t ret;
    wal_msg_stru *rsp_msg = NULL;
    /* 发送消息 */
    ret = wal_send_cfg_event(cfg_net_dev, WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_add_vap_param_stru),
        (uint8_t *)write_msg, OAL_TRUE, &rsp_msg);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(cfg_mac_vap->uc_vap_id, OAM_SF_ANY,
                         "{wal_init_wlan_vap_post_event_deal::return err:%d!}", ret);
        return -OAL_EFAIL;
    }
    /* 读取返回的错误码 */
    err_code = wal_check_and_release_msg_resp(rsp_msg);
    if (err_code != OAL_SUCC) {
        oam_warning_log1(cfg_mac_vap->uc_vap_id, 0,
                         "{wal_init_wlan_vap_post_event_deal::hmac add vap fail[%u]}", err_code);
        return -OAL_EFAIL;
    }

    if (OAL_SUCC != wal_set_random_mac_to_mib(net_dev)) {
        oam_warning_log0(cfg_mac_vap->uc_vap_id, 0, "{wal_init_wlan_vap_post_event_deal::set_random_mac_to_mib fail}");
        return -OAL_EFAUL;
    }
    return OAL_SUCC_GO_ON;
}

/*
 * 函 数 名  : wal_init_wlan_vap
 * 功能描述  : 初始化wlan0,p2p0的vap
 * 1.日    期  : 2015年5月22日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
int32_t wal_init_wlan_vap(oal_net_device_stru *net_dev)
{
    oal_net_device_stru *cfg_net_dev = NULL;
    mac_vap_stru *mac_vap = NULL;
    oal_wireless_dev_stru *pst_wdev = NULL;
    mac_wiphy_priv_stru *wiphy_priv = NULL;
    mac_vap_stru *cfg_mac_vap = NULL;
    hmac_vap_stru *pst_cfg_hmac_vap = NULL;
    mac_device_stru *pst_mac_device = NULL;
    wal_msg_write_stru write_msg = {0};
    int32_t ret;
    mac_cfg_add_vap_param_stru *add_vap_param = NULL;
    ret = wal_wlan_vap_init_get_base_dev_mac_stru(net_dev, &mac_vap, &pst_wdev, &wiphy_priv, &pst_mac_device);
    if (ret != OAL_SUCC_GO_ON) {
        return ret;
    }

    ret = wal_wlan_vap_init_get_cfg_stru(pst_mac_device, &cfg_mac_vap, &pst_cfg_hmac_vap, &cfg_net_dev, &write_msg);
    if (ret != OAL_SUCC_GO_ON) {
        return ret;
    }

    add_vap_param = (mac_cfg_add_vap_param_stru *)write_msg.auc_value;

    ret = wal_wlan_mac_cfg_add_vap_param_set(pst_mac_device, add_vap_param, pst_wdev, net_dev);
    if (ret != OAL_SUCC_GO_ON) {
        return ret;
    }

    wal_wlan_vap_init_post_event_msg(net_dev, add_vap_param, cfg_mac_vap, &write_msg);

    ret = wal_init_wlan_vap_post_event_deal(net_dev, cfg_net_dev, cfg_mac_vap, &write_msg);
    if (ret != OAL_SUCC_GO_ON) {
        return ret;
    }

    /* 设置netdevice的，在HMAC层被初始化到MIB中 */
    mac_vap = oal_net_dev_priv(net_dev);
    if (mac_vap == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_init_wlan_vap::OAL_NET_DEV_PRIV(net_dev) null.}");
        return -OAL_EINVAL;
    }

    if (add_vap_param->en_p2p_mode == WLAN_P2P_DEV_MODE) {
        pst_mac_device->st_p2p_info.uc_p2p0_vap_idx = mac_vap->uc_vap_id;
    }
    wal_init_wlan_ap_mode_set_max_user_count_and_mac_filter(net_dev, pst_wdev);
    return OAL_SUCC;
}

int32_t wal_deinit_wlan_vap_event_process(oal_net_device_stru *net_dev, mac_vap_stru *mac_vap,
    wlan_p2p_mode_enum_uint8 en_p2p_mode)
{
    wal_msg_write_stru write_msg = {0};
    wal_msg_stru *rsp_msg = NULL;
    int32_t ret;
    /* 抛事件到wal层处理 */
    memset_s(&write_msg, sizeof(wal_msg_write_stru), 0, sizeof(wal_msg_write_stru));

    ((mac_cfg_del_vap_param_stru *)write_msg.auc_value)->pst_net_dev = net_dev;
    ((mac_cfg_del_vap_param_stru *)write_msg.auc_value)->en_p2p_mode = en_p2p_mode;

    wal_write_msg_hdr_init(&write_msg, WLAN_CFGID_DEL_VAP, sizeof(mac_cfg_del_vap_param_stru));
    ret = wal_send_cfg_event(net_dev, WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_del_vap_param_stru),
        (uint8_t *)&write_msg, OAL_TRUE, &rsp_msg);
    if (wal_check_and_release_msg_resp(rsp_msg) != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY, "wal_deinit_wlan_vap::wal_check_and_release_msg_resp fail.");
        /* can't set net dev's vap ptr to null when del vap wid process failed! */
        return ret;
    }

    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_error_log1(mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_deinit_wlan_vap::return error code %d}", ret);
        if (ret == -OAL_ENOMEM || ret == -OAL_EFAIL) {
            /* wid had't processed */
            return ret;
        }
    }

    return OAL_SUCC;
}
/*
 * 函 数 名  : wal_deinit_wlan_vap
 * 功能描述  : 仅用于WIFI和AP关闭时删除VAP
 * 1.日    期  : 2015年1月3日
  *   修改内容  : 新生成函数
 */
int32_t wal_deinit_wlan_vap(oal_net_device_stru *net_dev)
{
    mac_vap_stru *mac_vap = NULL;
    int32_t ret;
    int8_t ac_wlan_netdev_name[MAC_NET_DEVICE_NAME_LENGTH];
    int8_t ac_p2p_netdev_name[MAC_NET_DEVICE_NAME_LENGTH];
    mac_device_stru *pst_mac_dev = NULL;
    wlan_p2p_mode_enum_uint8 en_p2p_mode = WLAN_LEGACY_VAP_MODE;

    if (oal_unlikely(net_dev == NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_deinit_wlan_vap::pst_del_vap_param null ptr !}");
        return -OAL_EINVAL;
    }

    mac_vap = oal_net_dev_priv(net_dev);
    if (mac_vap == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_deinit_wlan_vap::mac_vap is already null}");
        return OAL_SUCC;
    }

    pst_mac_dev = mac_res_get_dev(mac_vap->uc_device_id);
    if (pst_mac_dev == NULL) {
        oam_warning_log1(0, OAM_SF_ANY, "wal_deinit_wlan_vap::get mac dev id[%d] FAIL", mac_vap->uc_device_id);
        return -OAL_EFAIL;
    }
    /* 通过device id获取netdev名字 */
    snprintf_s(ac_wlan_netdev_name, MAC_NET_DEVICE_NAME_LENGTH, MAC_NET_DEVICE_NAME_LENGTH - 1,
               "wlan%u", pst_mac_dev->uc_device_id);
    snprintf_s(ac_p2p_netdev_name, MAC_NET_DEVICE_NAME_LENGTH, MAC_NET_DEVICE_NAME_LENGTH - 1,
               "p2p%u", pst_mac_dev->uc_device_id);

    /* 仅用于WIFI和AP关闭时删除VAP */
    if (((oal_strcmp(ac_wlan_netdev_name, net_dev->name)) != 0) &&
        ((oal_strcmp(ac_p2p_netdev_name, net_dev->name)) != 0) &&
#ifdef _PRE_WLAN_CHBA_MGMT
        ((oal_strcmp("chba0", net_dev->name)) != 0) &&
#endif
        ((oal_strcmp(WLAN1_NETDEV_NAME, net_dev->name)) != 0)) {
        oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_deinit_wlan_vap::net_dev is not wlan or p2p!}");
        return OAL_SUCC;
    }

    if (0 == oal_strcmp(ac_p2p_netdev_name, net_dev->name)) {
        en_p2p_mode = WLAN_P2P_DEV_MODE;
    }

    oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_deinit_wlan_vap::en_p2p_mode:%d}", en_p2p_mode);

    ret = wal_deinit_wlan_vap_event_process(net_dev, mac_vap, en_p2p_mode);
    if (ret != OAL_SUCC) {
        return ret;
    }
    oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_deinit_wlan_vap::clear netdev priv.}");
    oal_net_dev_priv(net_dev) = NULL;
    return ret;
}
/*
 * 功能描述  : cfg vap h2d
 * 1.日    期  : 2015年6月10日
  *   修改内容  : 新生成函数
 */
int32_t wal_cfg_vap_h2d_event(oal_net_device_stru *net_dev)
{
    mac_wiphy_priv_stru *wiphy_priv = NULL;
    hmac_vap_stru *pst_cfg_hmac_vap = NULL;
    mac_device_stru *pst_mac_device = NULL;
    oal_net_device_stru *cfg_net_dev = NULL;

    int32_t ret;
    wal_msg_stru *rsp_msg = NULL;
    uint32_t err_code;
    wal_msg_write_stru write_msg = {0};

    oal_wireless_dev_stru *wdev = oal_netdevice_wdev(net_dev);
    if (wdev == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_cfg_vap_h2d_event::wdev is null!}");
        return -OAL_EINVAL;
    }

    wiphy_priv = (mac_wiphy_priv_stru *)oal_wiphy_priv(wdev->wiphy);
    if (wiphy_priv == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_cfg_vap_h2d_event::wiphy_priv is null!}");
        return -OAL_EINVAL;
    }
    pst_mac_device = wiphy_priv->pst_mac_device;
    if (pst_mac_device == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_cfg_vap_h2d_event::mac_device is null!}");
        return -OAL_EINVAL;
    }

    pst_cfg_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_device->uc_cfg_vap_id);
    if (pst_cfg_hmac_vap == NULL) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_cfg_vap_h2d_event::mac_res_get_hmac_vap fail.vap_id[%u]}",
            pst_mac_device->uc_cfg_vap_id);
        return -OAL_EINVAL;
    }

    cfg_net_dev = pst_cfg_hmac_vap->pst_net_device;
    if (cfg_net_dev == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_cfg_vap_h2d_event::cfg_net_dev is null!}");
        return -OAL_EINVAL;
    }
    /* 抛事件到wal层处理 */
    wal_write_msg_hdr_init(&write_msg, WLAN_CFGID_CFG_VAP_H2D, sizeof(mac_cfg_vap_h2d_stru));
    ((mac_cfg_vap_h2d_stru *)write_msg.auc_value)->pst_net_dev = cfg_net_dev;

    /* 发送消息 */
    ret = wal_send_cfg_event(cfg_net_dev, WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_vap_h2d_stru), (uint8_t *)&write_msg, OAL_TRUE, &rsp_msg);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_cfg_vap_h2d_event::wal_alloc_cfg_event err %d!}", ret);
        return ret;
    }

    /* 处理返回消息 */
    err_code = wal_check_and_release_msg_resp(rsp_msg);
    if (err_code != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_cfg_vap_h2d_event::hmac cfg vap h2d fail,err code[%u]", err_code);
        return -OAL_EINVAL;
    }

    return OAL_SUCC;
}
/*
 * 函 数 名  : wal_hipriv_setcountry
 * 功能描述  : 设置管制域国家码
 * 1.日    期  : 2015年1月16日
  *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_setcountry(oal_net_device_stru *net_dev, int8_t *pc_param)
{
#ifdef _PRE_WLAN_FEATURE_11D
    int32_t ret;
    uint32_t result;
    uint32_t off_set = 0;
    int8_t ac_arg[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    int8_t *puc_para = NULL;

    /* 设备在up状态不允许配置，必须先down */
    if (0 != (OAL_IFF_RUNNING & oal_netdevice_flags(net_dev))) {
        oam_info_log1(0, OAM_SF_ANY, "{wal_hipriv_setcountry::net_dev flags: %d!}", oal_netdevice_flags(net_dev));
        return OAL_EBUSY;
    }

    /* 获取国家码字符串 */
    result = wal_get_cmd_one_arg(pc_param, ac_arg, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_setcountry::get cmd one arg err[%d]!}", result);
        return result;
    }
    puc_para = &ac_arg[0];

    ret = wal_regdomain_update_for_dfs(net_dev, puc_para);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_setcountry::regdomain_update err %d!}", ret);
        return (uint32_t)ret;
    }
    ret = wal_regdomain_update(net_dev, puc_para);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_setcountry::regdomain_update err %d!}", ret);
        return (uint32_t)ret;
    }
#else
    oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_setcountry::_PRE_WLAN_FEATURE_11D is not define!}");
#endif
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    g_cust_country_code_ignore_flag.en_country_code_ingore_hipriv_flag = OAL_TRUE;
#endif

    return OAL_SUCC;
}
/*
 * 函 数 名  : wal_init_wlan_netdev_set_netdev
 * 功能描述  : 配置netdev
 * 1.日    期  : 2019年11月28日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void wal_init_wlan_netdev_set_netdev(oal_net_device_stru *net_dev, oal_wireless_dev_stru *pst_wdev)
{
#ifdef _PRE_WLAN_FEATURE_GSO
    oam_warning_log0(0, OAM_SF_ANY, "{wal_init_wlan_netdev::[GSO] add NETIF_F_SG}");
    net_dev->features |= NETIF_F_SG;
    net_dev->hw_features |= NETIF_F_SG;
#endif
 /* 对netdevice进行赋值 */
#ifdef CONFIG_WIRELESS_EXT
    net_dev->wireless_handlers = &g_st_iw_handler_def;
#endif
    net_dev->netdev_ops = &g_st_wal_net_dev_ops;
    net_dev->ethtool_ops = &g_st_wal_ethtool_ops;
    oal_netdevice_destructor(net_dev) = oal_net_free_netdev;

    /* host发送报文使能csum硬化功能 */
    if (hmac_ring_tx_enabled() == OAL_TRUE) {
        net_dev->features    |= NETIF_F_IP_CSUM | NETIF_F_IPV6_CSUM;
        net_dev->hw_features |= NETIF_F_IP_CSUM | NETIF_F_IPV6_CSUM;
    }
    if (g_wlan_spec_cfg->rx_checksum_hw_enable) {
        net_dev->features    |= NETIF_F_RXCSUM;
        net_dev->hw_features |= NETIF_F_RXCSUM;
    }
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 44))
    oal_netdevice_master(net_dev) = NULL;
#endif

    oal_netdevice_ifalias(net_dev) = NULL;
    oal_netdevice_watchdog_timeo(net_dev) = 5;
    oal_netdevice_wdev(net_dev) = pst_wdev;
    oal_netdevice_qdisc(net_dev, NULL);
}

void wal_init_netdev_priv(oal_net_device_stru *net_dev)
{
    oal_netdev_priv_stru *net_dev_priv = NULL;
    if (net_dev == NULL) {
        return;
    }
    net_dev_priv = (oal_netdev_priv_stru *)oal_net_dev_wireless_priv(net_dev);
    wal_set_netdev_priv(net_dev_priv);
    net_dev_priv->dev = net_dev;
    oal_netbuf_list_head_init(&net_dev_priv->st_rx_netbuf_queue);
    oal_netif_napi_add(net_dev, &net_dev_priv->st_napi, hmac_rxdata_polling, NAPI_POLL_WEIGHT_LEV1);
}
/*
 * 函 数 名  : wal_get_mac_addr
 * 功能描述  : 初始化mac地址
 * 1.日    期  : 2015年5月25日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC int32_t wal_set_mac_addr(oal_net_device_stru *net_dev)
{
    uint8_t auc_primary_mac_addr[WLAN_MAC_ADDR_LEN] = { 0 }; /*  */
    oal_wireless_dev_stru *pst_wdev = oal_netdevice_wdev(net_dev);
    mac_wiphy_priv_stru *wiphy_priv = (mac_wiphy_priv_stru *)(oal_wiphy_priv(pst_wdev->wiphy));
    mac_device_stru *pst_mac_device = wiphy_priv->pst_mac_device;

    if (oal_unlikely(pst_mac_device->st_p2p_info.pst_primary_net_device == NULL)) {
        /* random mac will be used. mp12-cb (#include <linux/etherdevice.h>)    */
        oal_random_ether_addr(auc_primary_mac_addr, WLAN_MAC_ADDR_LEN);
        auc_primary_mac_addr[BYTE_OFFSET_0] &= (~0x02); /*  wlan0 MAC[0] bit1 需要设置为0 */
        auc_primary_mac_addr[BYTE_OFFSET_1] = 0x11;
        auc_primary_mac_addr[BYTE_OFFSET_2] = 0x02;
    } else {
#ifndef _PRE_PC_LINT
        if (oal_likely(NULL != oal_netdevice_mac_addr(pst_mac_device->st_p2p_info.pst_primary_net_device))) {
            if (EOK != memcpy_s(auc_primary_mac_addr, WLAN_MAC_ADDR_LEN,
                oal_netdevice_mac_addr(pst_mac_device->st_p2p_info.pst_primary_net_device), WLAN_MAC_ADDR_LEN)) {
                oam_error_log0(0, OAM_SF_ANY, "wal_set_mac_addr::memcpy fail!");
                return OAL_FAIL;
            }
        } else {
            oam_warning_log0(0, OAM_SF_ANY, "{wal_get_mac_addr::pst_primary_net_device; addr is null}");
            return OAL_FAIL;
        }
#endif
    }

    switch (pst_wdev->iftype) {
        case NL80211_IFTYPE_P2P_DEVICE: {
            /* 产生P2P device MAC 地址，将本地mac 地址bit 设置为1 */
            auc_primary_mac_addr[MAC_ADDR_0] |= 0x02;
            oal_set_mac_addr((uint8_t *)oal_netdevice_mac_addr(net_dev), auc_primary_mac_addr);
            break;
        }
        default: {
            if (oal_strcmp(WLAN1_NETDEV_NAME, net_dev->name) == 0) {
                auc_primary_mac_addr[MAC_ADDR_3] ^= 0x80; // 0x80代表mac地址第四个数值
            } else {
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
                hwifi_get_mac_addr(auc_primary_mac_addr);
                auc_primary_mac_addr[MAC_ADDR_0] &= (~0x02);
#else
                oal_random_ether_addr(auc_primary_mac_addr, WLAN_MAC_ADDR_LEN);
                auc_primary_mac_addr[MAC_ADDR_0] &= (~0x02);
                auc_primary_mac_addr[MAC_ADDR_1] = 0x11; // 0x11代表mac地址第二个数值
                auc_primary_mac_addr[MAC_ADDR_2] = 0x02; // 0x02代表mac地址第三个数值
#endif /* #ifdef _PRE_PLAT_FEATURE_CUSTOMIZE */
            }
            oal_set_mac_addr((uint8_t *)oal_netdevice_mac_addr(net_dev), auc_primary_mac_addr);
            break;
        }
    }

    return OAL_SUCC;
}

int32_t wal_init_netdev(oal_wiphy_stru *pst_wiphy, mac_wiphy_priv_stru *wiphy_priv, const char *dev_name,
    int8_t *ac_wlan_netdev_name, uint32_t arraylen, enum nl80211_iftype en_type)
{
    oal_wireless_dev_stru *wdev = NULL;
    int32_t ret;
    oal_net_device_stru *net_dev = NULL;

    /* 如果创建的net device已经存在，直接返回 */
    /* 根据dev_name找到dev */
    net_dev = wal_config_get_netdev(dev_name, OAL_STRLEN(dev_name));
    if (net_dev != NULL) {
        /* 调用oal_dev_get_by_name后，必须调用oal_dev_put使net_dev的引用计数减一 */
        oal_dev_put(net_dev);
        oam_warning_log0(0, OAM_SF_ANY, "{wal_init_wlan_netdev::the net_device is already exist!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 此函数第一个入参代表私有长度，此处不涉及为0 */
    net_dev = oal_net_alloc_netdev_mqs(sizeof(oal_netdev_priv_stru), dev_name, oal_ether_setup, WLAN_NET_QUEUE_BUTT, 1);
    if (oal_unlikely(net_dev == NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_init_wlan_netdev::oal_net_alloc_netdev return null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    wdev = (oal_wireless_dev_stru *)oal_mem_alloc_m(OAL_MEM_POOL_ID_LOCAL, sizeof(oal_wireless_dev_stru), OAL_FALSE);
    if (oal_unlikely(wdev == NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_init_wlan_netdev::alloc mem, wdev null!}");
        oal_net_free_netdev(net_dev);
        return OAL_ERR_CODE_PTR_NULL;
    }
    memset_s(wdev, sizeof(oal_wireless_dev_stru), 0, sizeof(oal_wireless_dev_stru));
    wal_init_wlan_netdev_set_netdev(net_dev, wdev);
    wdev->netdev = net_dev;
    wdev->iftype = en_type;
    wdev->wiphy = pst_wiphy;
    wiphy_priv = (mac_wiphy_priv_stru *)(oal_wiphy_priv(pst_wiphy));

    /* 信道跟随--add wlan name 判断 */
    if ((NL80211_IFTYPE_STATION == en_type) && (0 == (oal_strcmp(ac_wlan_netdev_name, dev_name)))) {
        /* 如果创建wlan0， 则保存wlan0 为主net_device,p2p0 和p2p-p2p0 MAC 地址从主netdevice 获取 */
        wiphy_priv->pst_mac_device->st_p2p_info.pst_primary_net_device = net_dev;
    } else if ((NL80211_IFTYPE_STATION == en_type) && (0 == (oal_strcmp(WLAN1_NETDEV_NAME, dev_name)))) {
        /* netdevice 指针暂时挂载，未使用 */
        wiphy_priv->pst_mac_device->st_p2p_info.pst_second_net_device = net_dev;
    } else if (NL80211_IFTYPE_P2P_DEVICE == en_type) {
        wiphy_priv->pst_mac_device->st_p2p_info.pst_p2p_net_device = net_dev;
    }
    oal_netdevice_flags(net_dev) &= ~OAL_IFF_RUNNING; /* 将net device的flag设为down */

    if (OAL_SUCC != wal_set_mac_addr(net_dev)) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_init_wlan_netdev::set mac addr fail}");
    }
    wal_init_netdev_priv(net_dev);

    /* 注册net_device */
    ret = hmac_net_register_netdev(net_dev);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_init_wlan_netdev::hmac_net_register_netdev err:%d!}", ret);
        oal_mem_free_m(wdev, OAL_FALSE);
        oal_net_free_netdev(net_dev);
        return ret;
    }
    return OAL_SUCC;
}
/*
 * 功能描述  : 初始化wlan0和p2p0设备
 */
int32_t wal_init_wlan_netdev(oal_wiphy_stru *pst_wiphy, const char *dev_name)
{
    mac_device_stru *pst_mac_device = NULL;
    mac_wiphy_priv_stru *wiphy_priv = NULL;
    int8_t ac_wlan_netdev_name[MAC_NET_DEVICE_NAME_LENGTH];
    int8_t ac_p2p_netdev_name[MAC_NET_DEVICE_NAME_LENGTH];
    enum nl80211_iftype en_type;
    if (oal_any_null_ptr2(pst_wiphy, dev_name)) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_init_wlan_netdev::pst_wiphy or dev_name is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    wiphy_priv = (mac_wiphy_priv_stru *)(oal_wiphy_priv(pst_wiphy));
    if (wiphy_priv == NULL || wiphy_priv->pst_mac_device == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_init_wlan_netdev::wiphy_priv is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mac_device = wiphy_priv->pst_mac_device;
    /* 通过device id获取netdev名字 */
    snprintf_s(ac_wlan_netdev_name, MAC_NET_DEVICE_NAME_LENGTH, MAC_NET_DEVICE_NAME_LENGTH - 1,
               "wlan%u", pst_mac_device->uc_device_id);
    snprintf_s(ac_p2p_netdev_name, MAC_NET_DEVICE_NAME_LENGTH, MAC_NET_DEVICE_NAME_LENGTH - 1,
               "p2p%u", pst_mac_device->uc_device_id);

    /* 信道跟随--添加hwlan name判断 */
    if (0 == (oal_strcmp(ac_wlan_netdev_name, dev_name)) || 0 == (oal_strcmp(WLAN1_NETDEV_NAME, dev_name))) {
        en_type = NL80211_IFTYPE_STATION;
    } else if (0 == (oal_strcmp(ac_p2p_netdev_name, dev_name))) {
        en_type = NL80211_IFTYPE_P2P_DEVICE;
    } else {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_init_wlan_netdev::dev name is not wlan or p2p}");
        return OAL_SUCC;
    }
    oam_warning_log1(0, OAM_SF_ANY, "{wal_init_wlan_netdev::en_type is %d,2:sta 10:p2p 3:ap}", en_type);

    return wal_init_netdev(pst_wiphy, wiphy_priv, dev_name, ac_wlan_netdev_name, sizeof(ac_wlan_netdev_name), en_type);
}
/*
 * 函 数 名  : wal_setup_ap
 * 功能描述  : wal侧创建ap的总接口
 * 1.日    期  : 2015年12月2日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
int32_t wal_setup_ap(oal_net_device_stru *net_dev)
{
    int32_t ret;
    mac_vap_stru *mac_vap = NULL;
    if (oal_unlikely(net_dev == NULL || net_dev->ieee80211_ptr == NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_setup_ap::net_dev or ieee80211_ptr null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    wal_set_power_mgmt_on(OAL_FALSE);
    ret = wal_set_power_on(net_dev, OAL_TRUE);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_CFG, "{wal_setup_ap::wal_set_power_on fail [%d]!}", ret);
        return ret;
    }
    if (oal_netdevice_flags(net_dev) & OAL_IFF_RUNNING) {
        /* 切换到AP前如果网络设备处于UP状态，需要先down wlan0网络设备 */
        oal_io_print("wal_setup_ap:stop netdevice:%.16s", net_dev->name);
        oam_warning_log0(0, OAM_SF_CFG, "{wal_setup_ap:: oal_iff_running! now, stop netdevice}");
        wal_netdev_stop(net_dev);
    }

    mac_vap = oal_net_dev_priv(net_dev);
    if (mac_vap != NULL) {
        if (mac_vap->en_vap_state != MAC_VAP_STATE_INIT) {
            /* 切换到AP前如果网络设备处于UP状态，需要先down wlan0网络设备 */
            oal_io_print("wal_setup_ap:stop netdevice:%s", net_dev->name);
            wal_netdev_stop(net_dev);
        }
    }

    net_dev->ieee80211_ptr->iftype = NL80211_IFTYPE_AP;

    ret = wal_init_wlan_vap(net_dev);
    /* 单chba下电再上电，只走该接口，需要下发校准数据 */
    wlan_chip_send_20m_all_chn_cali_data(net_dev);
    return ret;
}
