
/* 1 头文件包含 */
#include "oam_ext_if.h"
#include "frw_ext_if.h"
#include "hal_ext_if.h"
#include "mac_device.h"
#include "mac_resource.h"
#include "mac_regdomain.h"
#include "mac_vap.h"

#include "hmac_resource.h"
#include "hmac_device.h"
#include "hmac_vap.h"
#include "hmac_rx_filter.h"

#include "hmac_chan_mgmt.h"

#include "hmac_rx_filter.h"

#include "hmac_hcc_adapt.h"

#include "hmac_config.h"
#include "hmac_resource.h"
#include "hmac_device.h"
#include "hmac_scan.h"
#include "hmac_rx_data.h"
#include "hmac_hcc_adapt.h"
#include "hmac_dfx.h"
#include "hmac_protection.h"

#ifdef _PRE_WLAN_TCP_OPT
#include "mac_data.h"
#include "hmac_tcp_opt_struc.h"
#include "hmac_tcp_opt.h"
#endif
#include "hmac_tx_data.h"

#if ((_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION))
#include <linux/fb.h>
#include <linux/list.h>
#include "plat_pm_wlan.h"
#endif

#include "hmac_dscr_th_opt.h"
#include "securec.h"
#include "securectype.h"
#include "hmac_statistic_data_flow.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_DEVICE_C

/*lint -e578*/ /*lint -e19*/
oal_module_symbol(mac_get_band_5g_enabled);
oal_module_license("GPL");
/*lint +e578*/ /*lint +e19*/
/* 3 函数实现 */

oal_uint32 hmac_device_exit(mac_board_stru *pst_board, mac_chip_stru *pst_chip, hmac_device_stru *pst_hmac_device)
{
    mac_device_stru *pst_device = OAL_PTR_NULL;
    oal_uint32 ul_ret;
    oal_uint32 ul_return;
    hmac_vap_stru *pst_vap = OAL_PTR_NULL;
    mac_cfg_down_vap_param_stru st_down_vap;
    oal_uint8 uc_vap_idx;

    if (pst_hmac_device == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{hmac_device_exit::pst_hmac_device null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 扫描模块去初始化 */
    hmac_scan_exit(pst_hmac_device);

    hmac_rx_dscr_th_deinit(pst_hmac_device);

    pst_device = pst_hmac_device->pst_device_base_info;
    if (pst_device == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{hmac_device_exit::pst_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 亮暗屏去注册 */
#if ((_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION) && (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE))
#ifdef CONFIG_HAS_EARLYSUSPEND
    unregister_early_suspend(&pst_hmac_device->early_suspend);
#endif
#endif
    /* 由于配置vap初始化在HMAC做，所以配置VAP卸载也在HMAC做 */
    uc_vap_idx = pst_device->uc_cfg_vap_id;
    pst_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(uc_vap_idx);
    if (pst_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{hmac_device_exit::cfg_vap null. cfg_vap_id %d}", uc_vap_idx);

        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 取消OBSS保护老化定时器 */
    if (pst_device->st_obss_aging_timer.en_is_registerd == OAL_TRUE) {
        frw_immediate_destroy_timer(&(pst_device->st_obss_aging_timer));
    }

    ul_return = hmac_config_del_vap(&pst_vap->st_vap_base_info,
                                    OAL_SIZEOF(mac_cfg_down_vap_param_stru),
                                    (oal_uint8 *)&st_down_vap);
    if (ul_return != OAL_SUCC) {
        OAM_WARNING_LOG1(pst_vap->st_vap_base_info.uc_vap_id, OAM_SF_ANY, "{del_vap failed[%d].}", ul_return);
        return ul_return;
    }

    for (uc_vap_idx = 0; uc_vap_idx < pst_device->uc_vap_num; uc_vap_idx++) {
        /* 获取最右边一位为1的位数，此值即为vap的数组下标 */
        pst_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_device->auc_vap_id[uc_vap_idx]);
        if (pst_vap == OAL_PTR_NULL) {
            OAM_WARNING_LOG1(0, OAM_SF_ANY, "{hmac_device_exit::mac_res_get_hmac_vap failed vap_idx[%u].}", uc_vap_idx);
            continue;
        }

        ul_return = hmac_vap_destroy(pst_vap);
        if (ul_return != OAL_SUCC) {
            OAM_WARNING_LOG1(pst_vap->st_vap_base_info.uc_vap_id, OAM_SF_ANY, "{vap_destroy failed[%d].}", ul_return);
            return ul_return;
        }
        pst_device->auc_vap_id[uc_vap_idx] = 0;
    }

    /* 释放公共结构体 以及 对应衍生特性 */
    ul_ret = mac_device_exit(pst_device);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{mac_chip_exit::p_device_destroy_fun failed[%d].}", ul_ret);

        return ul_ret;
    }
    /* 指向基础mac device的指针为空 */
    pst_hmac_device->pst_device_base_info = OAL_PTR_NULL;

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 hmac_chip_exit(mac_board_stru *pst_board, mac_chip_stru *pst_chip)
{
    hmac_device_stru *pst_hmac_device = OAL_PTR_NULL;
    oal_uint32 ul_ret;
    oal_uint8 uc_device;

    if (oal_unlikely((pst_chip == OAL_PTR_NULL) || (pst_board == OAL_PTR_NULL))) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{hmac_chip_exit::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    for (uc_device = 0; uc_device < pst_chip->uc_device_nums; uc_device++) {
        pst_hmac_device = hmac_res_get_mac_dev(pst_chip->auc_device_id[uc_device]);

        /* 待挪动位置 释放资源 */
        hmac_res_free_mac_dev(pst_chip->auc_device_id[uc_device]);

        ul_ret = hmac_device_exit(pst_board, pst_chip, pst_hmac_device);
        if (ul_ret != OAL_SUCC) {
            OAM_WARNING_LOG1(0, OAM_SF_ANY, "{hmac_chip_exit::hmac_device_exit failed[%d].}", ul_ret);
            return ul_ret;
        }
    }

    ul_ret = mac_chip_exit(pst_board, pst_chip);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{hmac_chip_exit::mac_chip_exit failed[%d].}", ul_ret);
        return ul_ret;
    }

    return OAL_SUCC;
}


oal_uint32 hmac_board_exit(mac_board_stru *pst_board)
{
    oal_uint8 uc_chip_idx;
    oal_uint32 ul_ret;

    if (oal_unlikely(pst_board == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{hmac_board_exit::pst_board null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    while (pst_board->uc_chip_id_bitmap != 0) {
        /* 获取最右边一位为1的位数，此值即为chip的数组下标 */
        uc_chip_idx = oal_bit_find_first_bit_one_byte(pst_board->uc_chip_id_bitmap);
        if (oal_unlikely(uc_chip_idx >= WLAN_CHIP_MAX_NUM_PER_BOARD)) {
            oam_error_log2(0, OAM_SF_ANY, "{hmac_board_exit::invalid uc_chip_idx[%d] uc_chip_id_bitmap=%d.}",
                           uc_chip_idx, pst_board->uc_chip_id_bitmap);
            return OAL_ERR_CODE_ARRAY_OVERFLOW;
        }

        ul_ret = hmac_chip_exit(pst_board, &(pst_board->ast_chip[uc_chip_idx]));
        if (ul_ret != OAL_SUCC) {
            OAM_WARNING_LOG1(0, OAM_SF_ANY, "{hmac_board_exit::mac_chip_exit failed[%d].}", ul_ret);
            return ul_ret;
        }

        /* 清除对应的bitmap位 */
        oal_bit_clear_bit_one_byte(&pst_board->uc_chip_id_bitmap, uc_chip_idx);
    }

    /* 公共部分的初始化 */
    mac_board_exit(pst_board);

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 hmac_cfg_vap_init(mac_device_stru *pst_device)
{
    oal_int8 ac_vap_netdev_name[MAC_NET_DEVICE_NAME_LENGTH];
    oal_uint32 ul_ret;
    hmac_vap_stru *pst_vap = OAL_PTR_NULL;

    /* 初始化流程中，只初始化配置vap，其他vap需要通过配置添加 */
    /*lint -e413*/
    ul_ret = mac_res_alloc_hmac_vap(&pst_device->uc_cfg_vap_id,
                                    OAL_OFFSET_OF(hmac_vap_stru, st_vap_base_info));
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_device->uc_cfg_vap_id, OAM_SF_ANY, "{hmac_cfg_vap_init::res_alloc_hmac_vap failed[%d].}",
                         ul_ret);
        return ul_ret;
    }
    /*lint +e413*/
    pst_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_device->uc_cfg_vap_id);
    if (pst_vap == OAL_PTR_NULL) {
        oam_warning_log0(pst_device->uc_cfg_vap_id, OAM_SF_ANY, "{hmac_cfg_vap_init::pst_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 把hmac_vap_stru结构体初始化赋值为0 */
    memset_s(pst_vap, OAL_SIZEOF(hmac_vap_stru), 0, OAL_SIZEOF(hmac_vap_stru));

    {
        mac_cfg_add_vap_param_stru st_param = { 0 }; /* 构造配置VAP参数结构体 */
        st_param.en_vap_mode = WLAN_VAP_MODE_CONFIG;
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
        st_param.bit_11ac2g_enable = OAL_TRUE;
        st_param.bit_disable_capab_2ght40 = OAL_FALSE;
#endif
        ul_ret = hmac_vap_init(pst_vap,
                               pst_device->uc_chip_id,
                               pst_device->uc_device_id,
                               pst_device->uc_cfg_vap_id,
                               &st_param);
        if (ul_ret != OAL_SUCC) {
            OAM_WARNING_LOG1(pst_device->uc_cfg_vap_id, OAM_SF_ANY, "{hmac_cfg_vap_init::vap_initfailed[%d].}", ul_ret);
            return ul_ret;
        }
    }
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    /* 此时dmac未上电，帧过滤无法下发 */
#else
    /* 设置帧过滤寄存器 */
    hmac_set_rx_filter_value(&pst_vap->st_vap_base_info);
#endif

    /* 配置vap的id是从0开始编号的，有两个chip时，对应两个配置vap，编号为0,1 */
    snprintf_s(ac_vap_netdev_name, MAC_NET_DEVICE_NAME_LENGTH, MAC_NET_DEVICE_NAME_LENGTH - 1,
               "Hisilicon%d", pst_device->uc_cfg_vap_id);

    ul_ret = hmac_vap_creat_netdev(pst_vap, ac_vap_netdev_name, OAL_SIZEOF(ac_vap_netdev_name),
                                   (oal_int8 *)(pst_device->auc_hw_addr));
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(pst_device->uc_cfg_vap_id, OAM_SF_ANY, "{hmac_cfg_vap_init::vap_creat_netdev failed[%d].}",
                         ul_ret);
        return ul_ret;
    }

    /* 启动OBSS保护老化定时器 */
    hmac_protection_start_timer(pst_vap);

    return OAL_SUCC;
}
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)

oal_void hmac_do_suspend_action(mac_device_stru *pst_mac_device, oal_uint8 uc_in_suspend)
{
    hmac_device_stru *pst_hmac_device;
    oal_uint32 ul_ret;
    mac_vap_stru *pst_cfg_mac_vap = OAL_PTR_NULL;
    mac_cfg_suspend_stru st_suspend;
    oal_uint32 ul_is_wlan_poweron;

    pst_hmac_device = hmac_res_get_mac_dev(pst_mac_device->uc_device_id);
    if (pst_hmac_device == OAL_PTR_NULL) {
        oam_warning_log0(pst_mac_device->uc_cfg_vap_id, OAM_SF_ANY, "{hmac_do_suspend_action::pst_hmac_device null.}");
        return;
    }
    oal_spin_lock(&pst_hmac_device->st_suspend_lock);

    pst_mac_device->uc_in_suspend = uc_in_suspend;

    pst_cfg_mac_vap = (mac_vap_stru *)mac_res_get_mac_vap(pst_mac_device->uc_cfg_vap_id);
    if (pst_cfg_mac_vap == OAL_PTR_NULL) {
        oam_warning_log0(pst_mac_device->uc_cfg_vap_id, OAM_SF_ANY, "{hmac_do_suspend_action::pst_vap null.}");
        oal_spin_unlock(&pst_hmac_device->st_suspend_lock);
        return;
    }

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    ul_is_wlan_poweron = wlan_pm_is_poweron();
#endif
    /* 开了host低功耗并且device已上电才需要将亮暗屏状态同步到device */
    if (wlan_pm_get_switch() && ul_is_wlan_poweron) {
        st_suspend.uc_in_suspend = uc_in_suspend;
        if ((uc_in_suspend == OAL_TRUE) &&
            (g_en_force_pass_filter == OAL_TRUE)
#ifdef _PRE_WLAN_FEATURE_WAPI
            && (pst_hmac_device->pst_device_base_info->uc_wapi != OAL_TRUE)
#endif
) {
            pst_hmac_device->pst_device_base_info->uc_arpoffload_switch = OAL_TRUE;
            st_suspend.uc_arpoffload_switch = OAL_TRUE;
#ifdef _PRE_WLAN_FEATURE_APF
            hmac_set_apf_switch_by_pps(pst_hmac_device, &st_suspend);
#endif
        } else {
            pst_hmac_device->pst_device_base_info->uc_arpoffload_switch = OAL_FALSE;
            st_suspend.uc_arpoffload_switch = OAL_FALSE;
            pst_hmac_device->pst_device_base_info->en_apf_switch = OAL_FALSE;
            st_suspend.uc_apf_switch = OAL_FALSE;
        }

        hmac_wake_lock();
        /***************************************************************************
            抛事件到DMAC层, 同步屏幕最新状态到DMAC
        ***************************************************************************/
        ul_ret = hmac_config_send_event(pst_cfg_mac_vap, WLAN_CFGID_SUSPEND_ACTION_SYN,
                                        OAL_SIZEOF(mac_cfg_suspend_stru),
                                        (oal_uint8 *)&st_suspend);
        if (oal_unlikely(ul_ret != OAL_SUCC)) {
            OAM_WARNING_LOG1(0, OAM_SF_CFG, "{hmac_suspend_action::hmac_config_send_event failed[%d]}", ul_ret);
        }
        hmac_wake_unlock();
    }
    oal_spin_unlock(&pst_hmac_device->st_suspend_lock);
}

#ifdef CONFIG_HAS_EARLYSUSPEND

oal_void hmac_early_suspend(struct early_suspend *early_sup)
{
    hmac_device_stru *pst_hmac_device;

    pst_hmac_device = oal_container_of(early_sup, hmac_device_stru, early_suspend);
    hmac_do_suspend_action(pst_hmac_device->pst_device_base_info, OAL_TRUE);
}


oal_void hmac_late_resume(struct early_suspend *early_sup)
{
    hmac_device_stru *pst_hmac_device;

    pst_hmac_device = oal_container_of(early_sup, hmac_device_stru, early_suspend);
    hmac_do_suspend_action(pst_hmac_device->pst_device_base_info, OAL_FALSE);
}
#endif
#endif

oal_uint32 hmac_send_evt2wal(mac_vap_stru *pst_mac_vap, oal_uint8 uc_evtid, oal_uint8 *puc_evt,
                             oal_uint32 ul_evt_len)
{
    frw_event_mem_stru *pst_event_mem;
    frw_event_stru *pst_event = OAL_PTR_NULL;
    oal_uint32 ul_ret;

    pst_event_mem = frw_event_alloc_m((oal_uint16)ul_evt_len);
    if (pst_event_mem == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_SCAN, "{hmac_mgmt_scan_req_exception::pst_event_mem null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 填写事件 */
    pst_event = (frw_event_stru *)pst_event_mem->puc_data;

    frw_event_hdr_init(&(pst_event->st_event_hdr),
                       FRW_EVENT_TYPE_HOST_CTX,
                       uc_evtid,
                       (oal_uint16)ul_evt_len,
                       FRW_EVENT_PIPELINE_STAGE_0,
                       pst_mac_vap->uc_chip_id,
                       pst_mac_vap->uc_device_id,
                       pst_mac_vap->uc_vap_id);
    if (memcpy_s((oal_void *)pst_event->auc_event_data, ul_evt_len, (oal_void *)puc_evt, ul_evt_len) != EOK) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_SCAN, "{hmac_send_evt2wal::memcpy fail!}");
        frw_event_free_m(pst_event_mem);
        return OAL_FAIL;
    }

    /* 分发事件 */
    ul_ret = frw_event_dispatch_event(pst_event_mem);
    frw_event_free_m(pst_event_mem);
    return ul_ret;
}

oal_uint32 hmac_config_host_dev_init(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
    mac_device_stru *pst_mac_device;
    oal_uint32 ul_loop = 0;
#endif
    hmac_device_stru *pst_hmac_device = OAL_PTR_NULL;

    if (pst_mac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{hmac_device_init:: pst_mac_device NULL pointer!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (pst_mac_device == OAL_PTR_NULL) {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{hmac_device_init:: pst_device[%d] NULL pointer!}", pst_mac_vap->uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    for (ul_loop = 0; ul_loop < MAC_MAX_SUPP_CHANNEL; ul_loop++) {
        pst_mac_device->st_ap_channel_list[ul_loop].us_num_networks = 0;
        pst_mac_device->st_ap_channel_list[ul_loop].en_ch_type = MAC_CH_TYPE_NONE;
    }
#endif

    pst_hmac_device = hmac_res_get_mac_dev(pst_mac_vap->uc_device_id);
    if (oal_unlikely(pst_hmac_device == OAL_PTR_NULL)) {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{hmac_config_host_dev_init::pst_device[%] null!}", pst_mac_vap->uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_rx_dscr_th_init(pst_hmac_device);
    return OAL_SUCC;
}


oal_uint32 hmac_config_host_dev_exit(mac_vap_stru *pst_mac_vap)
{
    hmac_device_stru *pst_hmac_device = hmac_res_get_mac_dev(pst_mac_vap->uc_device_id);

    if (oal_unlikely(pst_hmac_device == OAL_PTR_NULL)) {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{hmac_config_host_dev_exit::pst_device[%] null!}", pst_mac_vap->uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_rx_dscr_th_deinit(pst_hmac_device);

    return OAL_SUCC;
}


oal_uint32 hmac_device_init(oal_uint8 *puc_device_id, mac_chip_stru *pst_chip)
{
    oal_uint8 uc_dev_id;
    mac_device_stru *pst_mac_device = OAL_PTR_NULL;
    hmac_device_stru *pst_hmac_device = OAL_PTR_NULL;
    oal_uint32 ul_ret;
#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
    oal_uint32 ul_loop = 0;
#endif
    /* 申请公共mac device结构体 */
    ul_ret = mac_res_alloc_hmac_dev(&uc_dev_id);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{hmac_device_init::mac_res_alloc_dmac_dev failed[%d].}", ul_ret);

        return OAL_FAIL;
    }

    /* 获取mac device结构体指针 */
    pst_mac_device = mac_res_get_dev(uc_dev_id);
    if (pst_mac_device == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{hmac_device_init::pst_device null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    ul_ret = mac_device_init(pst_mac_device, pst_chip->ul_chip_ver, pst_chip->uc_chip_id, uc_dev_id);
    if (ul_ret != OAL_SUCC) {
        oam_error_log2(0, OAM_SF_ANY, "{mac_device_init failed[%d], chip_ver[0x%x].}", ul_ret, pst_chip->ul_chip_ver);
        mac_res_free_dev(uc_dev_id);
        return ul_ret;
    }

    /* 申请hmac device资源 */
    if (oal_unlikely(hmac_res_alloc_mac_dev(uc_dev_id) != OAL_SUCC)) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{hmac_device_init::hmac_res_alloc_mac_dev failed.}");
        return OAL_FAIL;
    }

    /* 获取hmac device，并进行相关参数赋值 */
    pst_hmac_device = hmac_res_get_mac_dev(uc_dev_id);
    if (oal_unlikely(pst_hmac_device == OAL_PTR_NULL)) {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{hmac_device_init::pst_hmac_device[%] null!}", uc_dev_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 结构体初始化 */
    memset_s(pst_hmac_device, OAL_SIZEOF(hmac_device_stru), 0, OAL_SIZEOF(hmac_device_stru));

    pst_hmac_device->pst_device_base_info = pst_mac_device;

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    oal_spin_lock_init(&pst_hmac_device->st_suspend_lock);
#ifdef CONFIG_HAS_EARLYSUSPEND
    pst_hmac_device->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 20; /* 20表示early_suspend的等级 */
    pst_hmac_device->early_suspend.suspend = hmac_early_suspend;
    pst_hmac_device->early_suspend.resume = hmac_late_resume;
    register_early_suspend(&pst_hmac_device->early_suspend);
#endif
#endif
    /* 扫描模块初始化 */
    hmac_scan_init(pst_hmac_device);

    hmac_rx_dscr_th_init(pst_hmac_device);

    /* 初始化P2P 等待队列 */
    oal_wait_queue_init_head(&(pst_hmac_device->st_netif_change_event));
#ifdef _PRE_WLAN_TCP_OPT
    pst_hmac_device->sys_tcp_tx_ack_opt_enable = DEFAULT_TX_TCP_ACK_OPT_ENABLE;
    pst_hmac_device->sys_tcp_rx_ack_opt_enable = DEFAULT_RX_TCP_ACK_OPT_ENABLE;

#endif

#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
    for (ul_loop = 0; ul_loop < MAC_MAX_SUPP_CHANNEL; ul_loop++) {
        pst_mac_device->st_ap_channel_list[ul_loop].us_num_networks = 0;
        pst_mac_device->st_ap_channel_list[ul_loop].en_ch_type = MAC_CH_TYPE_NONE;
    }
#endif

    /* 初始化device下的rx tx BA会话数目 */
#ifndef _PRE_WLAN_FEATURE_AMPDU_VAP
    pst_mac_device->uc_rx_ba_session_num = 0;
    pst_mac_device->uc_tx_ba_session_num = 0;
#endif
    /* 出参赋值，CHIP中需要保存该device id */
    *puc_device_id = uc_dev_id;

    /* 配置vap初始化 */
    ul_ret = hmac_cfg_vap_init(pst_mac_device);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{hmac_chip_init::hmac_cfg_vap_init failed[%d].}", ul_ret);
        return OAL_FAIL;
    }
    oal_wait_queue_init_head(&pst_hmac_device->st_packet_check.st_check_wait_q);
    /* 初始化低功耗帧过滤统计数据查询等待队列 */
    oal_wait_queue_init_head(&(pst_hmac_device->st_psm_flt_stat_query.st_wait_queue));
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    oal_spin_lock_init(&(g_st_wifi_rxtx_statis.st_txrx_opt_lock));
#endif
    return OAL_SUCC;
}


OAL_STATIC oal_uint32 hmac_chip_init(mac_chip_stru *pst_chip, oal_uint8 uc_chip_id)
{
    oal_uint8 uc_device;
    oal_uint32 ul_ret;
    oal_uint8 uc_device_max;

    oam_info_log0(0, OAM_SF_ANY, "{hmac_chip_init::func enter.}");

    mac_chip_init(pst_chip, uc_chip_id);

    pst_chip->uc_chip_id = uc_chip_id;

    /* CHIP调用接口 oal_get_chip_version */
    pst_chip->ul_chip_ver = WLAN_CHIP_VERSION;

    /* OAL接口获取支持device个数 */
    uc_device_max = oal_chip_get_device_num(pst_chip->ul_chip_ver);

    for (uc_device = 0; uc_device < uc_device_max; uc_device++) {
        /* hmac device结构初始化 */
        ul_ret = hmac_device_init(&pst_chip->auc_device_id[uc_device], pst_chip);
        if (oal_unlikely(ul_ret != OAL_SUCC)) {
            OAM_ERROR_LOG1(0, OAM_SF_ANY, "{hmac_chip_init::hmac_device_init failed[%d].}", ul_ret);
            return OAL_FAIL;
        }
    }

    /* 保存device数量 */
    pst_chip->uc_device_nums = uc_device_max;

    /* 初始化最后再将state置为TRUE */
    pst_chip->en_chip_state = OAL_TRUE;

    oam_info_log0(0, OAM_SF_ANY, "{hmac_chip_init::func out.}");

    return OAL_SUCC;
}


oal_uint32 hmac_board_init(mac_board_stru *pst_board)
{
    oal_uint8 uc_chip;
    oal_uint32 ul_ret;
    oal_uint32 ul_chip_max_num;

    memset_s(pst_board, OAL_SIZEOF(mac_board_stru), 0, OAL_SIZEOF(mac_board_stru));

    mac_board_init(pst_board);

    /* chip支持的最大数由PCIe总线处理提供; */
    ul_chip_max_num = oal_bus_get_chip_num();

    for (uc_chip = 0; uc_chip < ul_chip_max_num; uc_chip++) {
        ul_ret = hmac_chip_init(&pst_board->ast_chip[uc_chip], uc_chip);
        if (ul_ret != OAL_SUCC) {
            oam_warning_log2(0, OAM_SF_ANY, "{hmac_init_event_process::chip_init failed[%d], uc_chip_id[%d].}", ul_ret,
                             uc_chip);
            return OAL_FAIL;
        }

        oal_bit_set_bit_one_byte(&pst_board->uc_chip_id_bitmap, uc_chip);
    }
    return OAL_SUCC;
}
#else

oal_uint32 hmac_device_init(oal_uint8 uc_device_id, mac_chip_stru *pst_dst_chip)
{
    mac_device_stru *pst_mac_device = OAL_PTR_NULL;
    hmac_device_stru *pst_hmac_device = OAL_PTR_NULL;
    oal_uint32 ul_ret;
    oal_uint32 ul_loop = 0;

    if (oal_unlikely(mac_res_alloc_hmac_dev(uc_device_id) != OAL_SUCC)) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{hmac_device_init::mac_res_alloc_hmac_dev failed.}");
        return OAL_FAIL;
    }

    /* 获取mac device结构体指针 */
    pst_mac_device = mac_res_get_dev(uc_device_id);

    ul_ret = mac_device_init(pst_mac_device, pst_dst_chip->ul_chip_ver, pst_dst_chip->uc_chip_id, uc_device_id);
    if (ul_ret != OAL_SUCC) {
        oam_error_log2(0, OAM_SF_ANY, "{device_init failed[%d], chip_ver[0x%x].}", ul_ret, pst_dst_chip->ul_chip_ver);
        mac_res_free_dev(uc_device_id);
        return ul_ret;
    }

    /* 申请hmac device资源 */
    if (oal_unlikely(hmac_res_alloc_mac_dev(uc_device_id) != OAL_SUCC)) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{hmac_device_init::hmac_res_alloc_mac_dev failed.}");
        return OAL_FAIL;
    }

    /* 获取hmac device，并进行相关参数赋值 */
    pst_hmac_device = hmac_res_get_mac_dev(uc_device_id);
    if (oal_unlikely(pst_hmac_device == OAL_PTR_NULL)) {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{hmac_device_init::pst_hmac_device[%d] null!}", uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 结构体初始化 */
    memset_s(pst_hmac_device, OAL_SIZEOF(hmac_device_stru), 0, OAL_SIZEOF(hmac_device_stru));

    pst_hmac_device->pst_device_base_info = pst_mac_device;

    /* 扫描模块初始化 */
    hmac_scan_init(pst_hmac_device);

    /* 初始化P2P 等待队列 */
    oal_wait_queue_init_head(&(pst_hmac_device->st_netif_change_event));
#ifdef _PRE_WLAN_TCP_OPT
    pst_hmac_device->sys_tcp_tx_ack_opt_enable = DEFAULT_TX_TCP_ACK_OPT_ENABLE;
    pst_hmac_device->sys_tcp_rx_ack_opt_enable = DEFAULT_RX_TCP_ACK_OPT_ENABLE;

#endif

#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
    for (ul_loop = 0; ul_loop < MAC_MAX_SUPP_CHANNEL; ul_loop++) {
        pst_mac_device->st_ap_channel_list[ul_loop].us_num_networks = 0;
        pst_mac_device->st_ap_channel_list[ul_loop].en_ch_type = MAC_CH_TYPE_NONE;
    }
#endif

#ifndef _PRE_WLAN_FEATURE_AMPDU_VAP
    /* 初始化device下的rx tx BA会话数目 */
    pst_mac_device->uc_rx_ba_session_num = 0;
    pst_mac_device->uc_tx_ba_session_num = 0;
#endif
    /* 配置vap初始化 */
    ul_ret = hmac_cfg_vap_init(pst_mac_device);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{hmac_chip_init::hmac_cfg_vap_init failed[%d].}", ul_ret);
        return OAL_FAIL;
    }
    return OAL_SUCC;
}


OAL_STATIC oal_uint32 hmac_chip_init(mac_chip_stru *pst_dst_chip,
                                     mac_chip_stru *pst_src_chip,
                                     oal_uint8 uc_chip_id)
{
    oal_uint8 uc_device;
    oal_uint32 ul_ret;
    oal_uint8 uc_dev_id;

    oam_info_log0(0, OAM_SF_ANY, "{hmac_chip_init::func enter.}");

    mac_chip_init(pst_dst_chip, pst_src_chip->uc_chip_id);

    pst_dst_chip->uc_chip_id = pst_src_chip->uc_chip_id;

    pst_dst_chip->ul_chip_ver = pst_src_chip->ul_chip_ver;

    pst_dst_chip->uc_device_nums = pst_src_chip->uc_device_nums;

    for (uc_device = 0; uc_device < pst_dst_chip->uc_device_nums; uc_device++) {
        uc_dev_id = pst_src_chip->auc_device_id[uc_device];

        pst_dst_chip->auc_device_id[uc_device] = uc_dev_id;

        /* hmac device结构初始化 */
        ul_ret = hmac_device_init(uc_dev_id, pst_dst_chip);
        if (oal_unlikely(ul_ret != OAL_SUCC)) {
            OAM_ERROR_LOG1(0, OAM_SF_ANY, "{hmac_chip_init::hmac_device_init failed[%d].}", ul_ret);
            return OAL_FAIL;
        }
    }
    /* 初始化最后再将state置为TRUE */
    pst_dst_chip->en_chip_state = OAL_TRUE;

    oam_info_log0(0, OAM_SF_ANY, "{hmac_chip_init::func out.}");

    return OAL_SUCC;
}


oal_uint32 hmac_board_init(oal_uint32 ul_chip_max_num, mac_chip_stru *pst_chip)
{
    oal_uint8 uc_chip;
    oal_uint32 ul_ret;

    memset_s(&g_st_hmac_board, OAL_SIZEOF(mac_board_stru), 0, OAL_SIZEOF(mac_board_stru));

    mac_board_init(&g_st_hmac_board);

    for (uc_chip = 0; uc_chip < ul_chip_max_num; uc_chip++) {
        ul_ret = hmac_chip_init(&g_st_hmac_board.ast_chip[uc_chip], pst_chip + uc_chip, uc_chip);
        if (ul_ret != OAL_SUCC) {
            oam_warning_log2(0, OAM_SF_ANY, "{hmac_init_event_process::chip_init failed[%d], uc_chip_id[%d].}", ul_ret,
                             uc_chip);
            return OAL_FAIL;
        }
        oal_bit_set_bit_one_byte(&g_st_hmac_board.uc_chip_id_bitmap, uc_chip);
    }
    return OAL_SUCC;
}
#endif


oal_void hmac_device_set_random_mac_for_scan(mac_device_stru *pst_mac_dev, mac_vap_stru *pst_mac_vap)
{
    hmac_device_stru *pst_hmac_dev = OAL_PTR_NULL;

    if (!IS_LEGACY_STA(pst_mac_vap)) {
        return;
    }

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    if (g_wlan_customize.uc_random_mac_addr_scan == OAL_FALSE) {
        return;
    }
#endif

    if (mac_get_band_5g_enabled() == OAL_FALSE) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_SCAN,
                         "hmac_device_set_random_mac_for_scan::Dont support 2G, dont set random mac");
        return;
    }

    pst_hmac_dev = hmac_res_get_mac_dev(pst_mac_dev->uc_device_id);
    if (pst_hmac_dev == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_SCAN, "hmac_device_set_random_mac_for_scan::hmac device is null");
        return;
    }

    oal_random_ether_addr(pst_hmac_dev->st_scan_mgmt.auc_random_mac);
    pst_hmac_dev->st_scan_mgmt.auc_random_mac[0] = pst_mac_dev->auc_mac_oui[0] & 0xfe; /* 保证是单播mac */
    pst_hmac_dev->st_scan_mgmt.auc_random_mac[1] = pst_mac_dev->auc_mac_oui[1];
    /* 由系统下发随机mac地址OUI（第2byte）赋值给 扫描时候用的随机MAC（第2byte） */
    pst_hmac_dev->st_scan_mgmt.auc_random_mac[2] = pst_mac_dev->auc_mac_oui[2];

    oam_warning_log4(pst_mac_vap->uc_vap_id, OAM_SF_SCAN,
                     "{hmac_device_set_random_mac_for_scan::rand_mac_addr[%02X:XX:XX:%02X:%02X:%02X].}",
                     pst_hmac_dev->st_scan_mgmt.auc_random_mac[0],
                     pst_hmac_dev->st_scan_mgmt.auc_random_mac[3], /* auc_random_mac第3byte为参数输出打印 */
                     pst_hmac_dev->st_scan_mgmt.auc_random_mac[4], /* auc_random_mac第4byte为参数输出打印 */
                     pst_hmac_dev->st_scan_mgmt.auc_random_mac[5]); /* auc_random_mac第5byte为参数输出打印 */

    return;
}
