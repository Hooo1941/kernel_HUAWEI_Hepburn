

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "oam_ext_if.h"
#include "frw_ext_if.h"
#include "hal_ext_if.h"
#include "mac_device.h"
#include "mac_resource.h"
#include "mac_regdomain.h"
#include "mac_vap.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_MAC_DEVICE_C

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
#ifdef _PRE_WLAN_FEATURE_WMMAC
oal_bool_enum_uint8 g_en_wmmac_switch = OAL_FALSE;
#endif
mac_device_voe_custom_stru g_st_mac_voe_custom_param;
mac_device_voe_custom_stru *mac_get_voe_custom_param_addr(oal_void)
{
    return &g_st_mac_voe_custom_param;
}

#ifdef WIN32
mac_device_rom_stru g_st_mac_device_rom_cb = {
    mac_device_init_cb,
    mac_device_max_band_cb,
};
#else
mac_device_rom_stru g_st_mac_device_rom_cb = {
    .p_device_init_cb = mac_device_init_cb,
    .p_device_max_band_cb = mac_device_max_band_cb,
};
#endif

oal_uint32 g_band_5g_enabled = OAL_TRUE;
oal_uint32 mac_get_band_5g_enabled(oal_void)
{
    return g_band_5g_enabled;
}
oal_void mac_set_band_5g_enabled(oal_uint32 band_5g_enabled)
{
    g_band_5g_enabled = band_5g_enabled;
}

/*****************************************************************************
  3 函数实现
*****************************************************************************/

hal_fcs_protect_type_enum_uint8 mac_fcs_get_protect_type(mac_vap_stru *pst_mac_vap)
{
    hal_fcs_protect_type_enum_uint8 en_protect_type;

    if (pst_mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_AP) {
        en_protect_type = HAL_FCS_PROTECT_TYPE_SELF_CTS;
    } else if (pst_mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_STA) {
        en_protect_type = HAL_FCS_PROTECT_TYPE_NULL_DATA;
    } else {
        en_protect_type = HAL_FCS_PROTECT_TYPE_NONE;
    }

#ifdef _PRE_WLAN_FEATURE_P2P
    if (pst_mac_vap->en_p2p_mode == WLAN_P2P_GO_MODE) {
        if (mac_find_vendor_ie(MAC_WLAN_OUI_WFA,
                               MAC_WLAN_OUI_TYPE_WFA_P2P,
                               pst_mac_vap->ast_app_ie[OAL_APP_BEACON_IE].puc_ie,
                               (oal_int32)pst_mac_vap->ast_app_ie[OAL_APP_BEACON_IE].ul_ie_len) == OAL_PTR_NULL) {
            en_protect_type = HAL_FCS_PROTECT_TYPE_SELF_CTS;
        } else {
            en_protect_type = HAL_FCS_PROTECT_TYPE_NONE;
        }
    }
#endif

    return en_protect_type;
}


oal_uint32 mac_device_exit(mac_device_stru *pst_device)
{
    if (oal_unlikely(pst_device == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_device_exit::pst_device null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_device->uc_vap_num = 0;
    pst_device->uc_sta_num = 0;
    pst_device->st_p2p_info.uc_p2p_device_num = 0;
    pst_device->st_p2p_info.uc_p2p_goclient_num = 0;

    mac_res_free_dev(pst_device->uc_device_id);

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    mac_device_set_state(pst_device, OAL_FALSE);
#endif

    return OAL_SUCC;
}


oal_uint32 mac_chip_exit(mac_board_stru *pst_board, mac_chip_stru *pst_chip)
{
    if (oal_unlikely((pst_chip == OAL_PTR_NULL) || (pst_board == OAL_PTR_NULL))) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{hmac_chip_init::param null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 放入Device自身结构释放 */
    pst_chip->uc_device_nums = 0;

    /* destroy流程最后将状态置为FALSE */
    pst_chip->en_chip_state = OAL_FALSE;

    return OAL_SUCC;
}


oal_uint32 mac_board_exit(mac_board_stru *pst_board)
{
    if (oal_unlikely(pst_board == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{mac_board_exit::pst_board null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    return OAL_SUCC;
}


wlan_bw_cap_enum_uint8 mac_device_max_band(oal_void)
{
    return g_st_mac_device_rom_cb.p_device_max_band_cb();
}


oal_uint32 mac_device_init(
    mac_device_stru *pst_mac_device, oal_uint32 ul_chip_ver, oal_uint8 uc_chip_id, oal_uint8 uc_device_id)
{
    oal_uint32 ul_cb_ret = OAL_SUCC; /* rom cb函数返回值 */

    if (pst_mac_device == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{mac_device_init::pst_mac_device null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 初始化device的索引 */
    pst_mac_device->uc_chip_id = uc_chip_id;
    pst_mac_device->uc_device_id = uc_device_id;

    /* 初始化device级别的一些参数 */
    pst_mac_device->en_max_bandwidth = WLAN_BAND_WIDTH_BUTT;
    pst_mac_device->en_max_band = WLAN_BAND_BUTT;
    pst_mac_device->uc_max_channel = 0;
    pst_mac_device->ul_beacon_interval = WLAN_BEACON_INTVAL_DEFAULT;

    pst_mac_device->uc_tx_chain = 0xf;
    pst_mac_device->uc_rx_chain = 0xf;

#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
    pst_mac_device->st_bss_id_list.us_num_networks = 0;
#endif

    pst_mac_device->en_device_state = OAL_TRUE;
    pst_mac_device->en_reset_switch = OAL_FALSE;
    pst_mac_device->uc_tx_chain = WITP_TX_CHAIN_ZERO;

    /* 默认关闭wmm,wmm超时计数器设为0 */
    pst_mac_device->en_wmm = OAL_TRUE;

    /* 根据芯片版本初始化device能力信息 */
    pst_mac_device->en_protocol_cap = WLAN_PROTOCOL_CAP_VHT;
#if defined(_PRE_PRODUCT_ID_HI110X_HOST)
    pst_mac_device->en_bandwidth_cap = (mac_get_band_5g_enabled()) ? mac_device_max_band() : WLAN_BW_CAP_40M;
    pst_mac_device->en_band_cap = (mac_get_band_5g_enabled()) ? WLAN_BAND_CAP_2G_5G : WLAN_BAND_CAP_2G;
#else
    pst_mac_device->en_bandwidth_cap = mac_device_max_band();
    pst_mac_device->en_band_cap = WLAN_BAND_CAP_2G_5G;
#endif

    pst_mac_device->bit_ldpc_coding = OAL_TRUE;
#ifdef _PRE_WLAN_FEATURE_TXBF
    pst_mac_device->bit_tx_stbc = OAL_FALSE;
    pst_mac_device->bit_su_bfmer = OAL_FALSE;
    pst_mac_device->bit_su_bfmee = OAL_TRUE;
    pst_mac_device->bit_rx_stbc = 1; /* 支持2个空间流 */

#if defined(_PRE_PRODUCT_ID_HI110X_DEV) || defined(_PRE_PRODUCT_ID_HI110X_HOST)
    pst_mac_device->bit_mu_bfmee = OAL_TRUE;
#else
    pst_mac_device->bit_mu_bfmee = OAL_FALSE;
#endif

#else
    pst_mac_device->bit_tx_stbc = OAL_FALSE;
    pst_mac_device->bit_su_bfmer = OAL_FALSE;
    pst_mac_device->bit_su_bfmee = OAL_FALSE;
    pst_mac_device->bit_mu_bfmee = OAL_FALSE;
    pst_mac_device->bit_rx_stbc = 1;
#endif

    /* 初始化vap num统计信息 */
    pst_mac_device->uc_vap_num = 0;
    pst_mac_device->uc_sta_num = 0;
    pst_mac_device->st_p2p_info.uc_p2p_device_num = 0;
    pst_mac_device->st_p2p_info.uc_p2p_goclient_num = 0;
    pst_mac_device->st_p2p_info.pst_primary_net_device = OAL_PTR_NULL; /* 初始化主net_device 为空指针 */

    /* 初始化默认管制域 */
    mac_init_regdomain();

    /* 初始化信道列表 */
    mac_init_channel_list();

    /* 初始化复位状态 */
    MAC_DEV_RESET_IN_PROGRESS(pst_mac_device, OAL_FALSE);
    pst_mac_device->us_device_reset_num = 0;

    /* 默认关闭DBAC特性 */
#ifdef _PRE_WLAN_FEATURE_DBAC
    pst_mac_device->en_dbac_enabled = OAL_TRUE;
#endif
#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
    mac_set_2040bss_switch(pst_mac_device, OAL_FALSE);
#endif
    pst_mac_device->uc_in_suspend = OAL_FALSE;
    pst_mac_device->uc_arpoffload_switch = OAL_FALSE;

    pst_mac_device->uc_wapi = OAL_FALSE;

    /* AGC绑定通道默认为自适应 */
#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
    pst_mac_device->uc_scan_count = 0;
#endif

    /* 初始化随机mac oui为0(3个字节都是0),确保只有系统下发有效mac oui才进行随机mac地址扫描(在随机mac扫描开关打开的情况下) */
    pst_mac_device->en_is_random_mac_addr_scan = OAL_FALSE;
    pst_mac_device->auc_mac_oui[0] = 0x0;
    pst_mac_device->auc_mac_oui[1] = 0x0;
    pst_mac_device->auc_mac_oui[2] = 0x0; /* 初始化随机mac oui第2字节为0(3个字节都是0) */

#ifdef _PRE_WLAN_NARROW_BAND
    /* 初始化硬件窄带能力 */
    pst_mac_device->bit_nb_is_supp = OAL_FALSE;
#endif

    if (g_st_mac_device_rom_cb.p_device_init_cb(pst_mac_device, ul_chip_ver, uc_chip_id, uc_device_id, &ul_cb_ret) ==
        OAL_RETURN) {
        return ul_cb_ret;
    }

    return OAL_SUCC;
}

oal_uint32 mac_chip_init(mac_chip_stru *pst_chip, oal_uint8 uc_chip_id)
{
    return OAL_SUCC;
}

oal_uint32 mac_board_init(mac_board_stru *pst_board)
{
    return OAL_SUCC;
}


mac_vap_stru *mac_device_find_another_up_vap(mac_device_stru *pst_mac_device, oal_uint8 uc_vap_id_self)
{
    oal_uint8 uc_vap_idx;
    mac_vap_stru *pst_mac_vap = OAL_PTR_NULL;

    for (uc_vap_idx = 0; uc_vap_idx < pst_mac_device->uc_vap_num; uc_vap_idx++) {
        pst_mac_vap = mac_res_get_mac_vap(pst_mac_device->auc_vap_id[uc_vap_idx]);
        if (pst_mac_vap == OAL_PTR_NULL) {
            continue;
        }

        if (uc_vap_id_self == pst_mac_vap->uc_vap_id) {
            continue;
        }

        if ((pst_mac_vap->en_vap_state == MAC_VAP_STATE_UP) || pst_mac_vap->en_vap_state == MAC_VAP_STATE_PAUSE
#ifdef _PRE_WLAN_FEATURE_P2P
            || ((pst_mac_vap->en_vap_state == MAC_VAP_STATE_STA_LISTEN) && (pst_mac_vap->us_user_nums > 0))
#endif
#ifdef _PRE_WLAN_FEATURE_ROAM
            || (pst_mac_vap->en_vap_state == MAC_VAP_STATE_ROAMING)
#endif  // _PRE_WLAN_FEATURE_ROAM
) {
            return pst_mac_vap;
        }
    }

    return OAL_PTR_NULL;
}


oal_uint32 mac_device_find_up_vap(mac_device_stru *pst_mac_device, mac_vap_stru **ppst_mac_vap)
{
    oal_uint8 uc_vap_idx;
    mac_vap_stru *pst_mac_vap = OAL_PTR_NULL;

    if (pst_mac_device == NULL) {
        *ppst_mac_vap = NULL;
        return OAL_FAIL;
    }

    for (uc_vap_idx = 0; uc_vap_idx < pst_mac_device->uc_vap_num; uc_vap_idx++) {
        pst_mac_vap = mac_res_get_mac_vap(pst_mac_device->auc_vap_id[uc_vap_idx]);
        if (oal_unlikely(pst_mac_vap == OAL_PTR_NULL)) {
            OAM_WARNING_LOG1(0, OAM_SF_SCAN, "vap is null! vap id is %d", pst_mac_device->auc_vap_id[uc_vap_idx]);

            *ppst_mac_vap = OAL_PTR_NULL;

            return OAL_ERR_CODE_PTR_NULL;
        }

        if ((pst_mac_vap->en_vap_state == MAC_VAP_STATE_UP) || pst_mac_vap->en_vap_state == MAC_VAP_STATE_PAUSE
#ifdef _PRE_WLAN_FEATURE_P2P
            || ((pst_mac_vap->en_vap_state == MAC_VAP_STATE_STA_LISTEN) && (pst_mac_vap->us_user_nums > 0))
#endif
#ifdef _PRE_WLAN_FEATURE_ROAM
            || (pst_mac_vap->en_vap_state == MAC_VAP_STATE_ROAMING)
#endif  // _PRE_WLAN_FEATURE_ROAM
) {
            *ppst_mac_vap = pst_mac_vap;

            return OAL_SUCC;
        }
    }

    *ppst_mac_vap = OAL_PTR_NULL;

    return OAL_FAIL;
}


oal_uint32 mac_device_find_2up_vap(mac_device_stru *pst_mac_device,
                                   mac_vap_stru **ppst_mac_vap1,
                                   mac_vap_stru **ppst_mac_vap2)
{
    mac_vap_stru *pst_vap = OAL_PTR_NULL;
    oal_uint8 uc_vap_idx;
    oal_uint8 ul_up_vap_num = 0;
    mac_vap_stru *past_vap[2] = { 0 };

    for (uc_vap_idx = 0; uc_vap_idx < pst_mac_device->uc_vap_num; uc_vap_idx++) {
        pst_vap = (mac_vap_stru *)mac_res_get_mac_vap(pst_mac_device->auc_vap_id[uc_vap_idx]);
        if (pst_vap == OAL_PTR_NULL) {
            OAM_ERROR_LOG1(0, OAM_SF_ANY, "vap is null, vap id is %d", pst_mac_device->auc_vap_id[uc_vap_idx]);
            continue;
        }

        if ((pst_vap->en_vap_state == MAC_VAP_STATE_UP)
            || (pst_vap->en_vap_state == MAC_VAP_STATE_PAUSE)
#ifdef _PRE_WLAN_FEATURE_ROAM
            || (pst_vap->en_vap_state == MAC_VAP_STATE_ROAMING)
#endif  // _PRE_WLAN_FEATURE_ROAM
            || ((pst_vap->en_vap_state == MAC_VAP_STATE_STA_LISTEN) && (pst_vap->us_user_nums > 0))) {
            past_vap[ul_up_vap_num] = pst_vap;
            ul_up_vap_num++;

            if (ul_up_vap_num >= 2) { /* 因为只需要找到2个up的vap */
                break;
            }
        }
    }

    if (ul_up_vap_num < 2) { /* 函数的功能就是找到2个up的vap,小于2返回失败 */
        return OAL_FAIL;
    }

    *ppst_mac_vap1 = past_vap[0];
    *ppst_mac_vap2 = past_vap[1];

    return OAL_SUCC;
}


oal_uint32 mac_fcs_dbac_state_check(mac_device_stru *pst_mac_device)
{
    mac_vap_stru *pst_mac_vap1 = OAL_PTR_NULL;
    mac_vap_stru *pst_mac_vap2 = OAL_PTR_NULL;
    oal_uint32 ul_ret;

    ul_ret = mac_device_find_2up_vap(pst_mac_device, &pst_mac_vap1, &pst_mac_vap2);
    if (ul_ret != OAL_SUCC) {
        return MAC_FCS_DBAC_IGNORE;
    }

    if (pst_mac_vap1->st_channel.uc_chan_number == pst_mac_vap2->st_channel.uc_chan_number) {
        return MAC_FCS_DBAC_NEED_CLOSE;
    }

    return MAC_FCS_DBAC_NEED_OPEN;
}


oal_uint32 mac_device_find_up_ap(mac_device_stru *pst_mac_device, mac_vap_stru **ppst_mac_vap)
{
    oal_uint8 uc_vap_idx;
    mac_vap_stru *pst_mac_vap = OAL_PTR_NULL;

    for (uc_vap_idx = 0; uc_vap_idx < pst_mac_device->uc_vap_num; uc_vap_idx++) {
        pst_mac_vap = mac_res_get_mac_vap(pst_mac_device->auc_vap_id[uc_vap_idx]);
        if (oal_unlikely(pst_mac_vap == OAL_PTR_NULL)) {
            OAM_WARNING_LOG1(0, OAM_SF_SCAN, "vap is null! vap id is %d", pst_mac_device->auc_vap_id[uc_vap_idx]);
            return OAL_ERR_CODE_PTR_NULL;
        }

        if (((pst_mac_vap->en_vap_state == MAC_VAP_STATE_UP) || (pst_mac_vap->en_vap_state == MAC_VAP_STATE_PAUSE)) &&
            (pst_mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_AP)) {
            *ppst_mac_vap = pst_mac_vap;

            return OAL_SUCC;
        }
    }

    *ppst_mac_vap = OAL_PTR_NULL;

    return OAL_FAIL;
}


oal_uint32 mac_device_find_up_sta(mac_device_stru *pst_mac_device, mac_vap_stru **ppst_mac_vap)
{
    oal_uint8 uc_vap_idx;
    mac_vap_stru *pst_mac_vap = OAL_PTR_NULL;

    for (uc_vap_idx = 0; uc_vap_idx < pst_mac_device->uc_vap_num; uc_vap_idx++) {
        pst_mac_vap = mac_res_get_mac_vap(pst_mac_device->auc_vap_id[uc_vap_idx]);
        if (oal_unlikely(pst_mac_vap == OAL_PTR_NULL)) {
            OAM_WARNING_LOG1(0, OAM_SF_SCAN, "vap is null! vap id is %d", pst_mac_device->auc_vap_id[uc_vap_idx]);

            *ppst_mac_vap = OAL_PTR_NULL;

            return OAL_ERR_CODE_PTR_NULL;
        }

        if (((pst_mac_vap->en_vap_state == MAC_VAP_STATE_UP) || (pst_mac_vap->en_vap_state == MAC_VAP_STATE_PAUSE)) &&
            (pst_mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_STA)) {
            *ppst_mac_vap = pst_mac_vap;

            return OAL_SUCC;
        }
    }

    *ppst_mac_vap = OAL_PTR_NULL;

    return OAL_FAIL;
}


oal_uint32 mac_device_find_up_p2p_go(mac_device_stru *pst_mac_device, mac_vap_stru **ppst_mac_vap)
{
    oal_uint8 uc_vap_idx;
    mac_vap_stru *pst_mac_vap = OAL_PTR_NULL;

    for (uc_vap_idx = 0; uc_vap_idx < pst_mac_device->uc_vap_num; uc_vap_idx++) {
        pst_mac_vap = mac_res_get_mac_vap(pst_mac_device->auc_vap_id[uc_vap_idx]);
        if (oal_unlikely(pst_mac_vap == OAL_PTR_NULL)) {
            OAM_WARNING_LOG1(0, OAM_SF_SCAN, "vap is null! vap id is %d", pst_mac_device->auc_vap_id[uc_vap_idx]);
            continue;
        }

        if (((pst_mac_vap->en_vap_state == MAC_VAP_STATE_UP) || (pst_mac_vap->en_vap_state == MAC_VAP_STATE_PAUSE)) &&
            (pst_mac_vap->en_p2p_mode == WLAN_P2P_GO_MODE)) {
            *ppst_mac_vap = pst_mac_vap;

            return OAL_SUCC;
        }
    }

    *ppst_mac_vap = OAL_PTR_NULL;

    return OAL_FAIL;
}


oal_uint8 mac_device_calc_up_vap_num(mac_device_stru *pst_mac_device)
{
    mac_vap_stru *pst_vap = OAL_PTR_NULL;
    oal_uint8 uc_vap_idx;
    oal_uint8 ul_up_ap_num = 0;

    for (uc_vap_idx = 0; uc_vap_idx < pst_mac_device->uc_vap_num; uc_vap_idx++) {
        pst_vap = (mac_vap_stru *)mac_res_get_mac_vap(pst_mac_device->auc_vap_id[uc_vap_idx]);
        if (pst_vap == OAL_PTR_NULL) {
            OAM_ERROR_LOG1(0, OAM_SF_ANY, "vap is null, vap id is %d",
                           pst_mac_device->auc_vap_id[uc_vap_idx]);
            continue;
        }

        if ((pst_vap->en_vap_state == MAC_VAP_STATE_UP)
            || (pst_vap->en_vap_state == MAC_VAP_STATE_PAUSE)
#ifdef _PRE_WLAN_FEATURE_ROAM
            || (pst_vap->en_vap_state == MAC_VAP_STATE_ROAMING)
#endif  // _PRE_WLAN_FEATURE_ROAM
#ifdef _PRE_WLAN_FEATURE_P2P
            || ((pst_vap->en_vap_state == MAC_VAP_STATE_STA_LISTEN) && (pst_vap->us_user_nums > 0))
#endif
) {
            ul_up_ap_num++;
        }
    }

    return ul_up_ap_num;
}


oal_uint32 mac_device_calc_work_vap_num(mac_device_stru *pst_mac_device)
{
    mac_vap_stru *pst_vap = OAL_PTR_NULL;
    oal_uint8 uc_vap_idx;
    oal_uint8 ul_work_vap_num = 0;

    for (uc_vap_idx = 0; uc_vap_idx < pst_mac_device->uc_vap_num; uc_vap_idx++) {
        pst_vap = (mac_vap_stru *)mac_res_get_mac_vap(pst_mac_device->auc_vap_id[uc_vap_idx]);
        if (pst_vap == OAL_PTR_NULL) {
            OAM_ERROR_LOG1(0, OAM_SF_ANY, "mac_device_calc_work_vap_numv::vap[%d] is null",
                           pst_mac_device->auc_vap_id[uc_vap_idx]);
            continue;
        }

        if ((pst_vap->en_vap_state != MAC_VAP_STATE_INIT) && (pst_vap->en_vap_state != MAC_VAP_STATE_BUTT)) {
            ul_work_vap_num++;
        }
    }

    return ul_work_vap_num;
}


oal_uint32 mac_device_is_p2p_connected(mac_device_stru *pst_mac_device)
{
    oal_uint8 uc_vap_idx;
    mac_vap_stru *pst_mac_vap = OAL_PTR_NULL;

    for (uc_vap_idx = 0; uc_vap_idx < pst_mac_device->uc_vap_num; uc_vap_idx++) {
        pst_mac_vap = mac_res_get_mac_vap(pst_mac_device->auc_vap_id[uc_vap_idx]);
        if (oal_unlikely(pst_mac_vap == OAL_PTR_NULL)) {
            OAM_WARNING_LOG1(0, OAM_SF_P2P, "vap is null! vap id is %d", pst_mac_device->auc_vap_id[uc_vap_idx]);
            return OAL_FALSE;
        }
        if ((IS_P2P_GO(pst_mac_vap) || IS_P2P_CL(pst_mac_vap)) &&
            (pst_mac_vap->us_user_nums > 0)) {
            return OAL_TRUE;
        }
    }
    return OAL_FALSE;
}


oal_void mac_device_set_vap_id(
    mac_device_stru *pst_mac_device, mac_vap_stru *pst_mac_vap, oal_uint8 uc_vap_idx,
    wlan_vap_mode_enum_uint8 en_vap_mode, wlan_p2p_mode_enum_uint8 en_p2p_mode, oal_uint8 is_add_vap)
{
#ifdef _PRE_WLAN_FEATURE_P2P
    oal_uint8 uc_vap_tmp_idx = 0;
    mac_vap_stru *pst_tmp_vap = OAL_PTR_NULL;
#endif

    if (is_add_vap) {
        /* ?offload???,????HMAC????? */
        pst_mac_device->auc_vap_id[pst_mac_device->uc_vap_num++] = uc_vap_idx;

        /* device?sta???1 */
        if (en_vap_mode == WLAN_VAP_MODE_BSS_STA) {
            pst_mac_device->uc_sta_num++;

            /* ???uc_assoc_vap_id??????ap??? */
            pst_mac_vap->uc_assoc_vap_id = 0xff;
        }

#ifdef _PRE_WLAN_FEATURE_P2P
        pst_mac_vap->en_p2p_mode = en_p2p_mode;
        mac_inc_p2p_num(pst_mac_vap);
        /*  ??GO?????????0,???WLAN??????GO?????? */
        if (IS_P2P_GO(pst_mac_vap)) {
            for (uc_vap_tmp_idx = 0; uc_vap_tmp_idx < pst_mac_device->uc_vap_num; uc_vap_tmp_idx++) {
                pst_tmp_vap = (mac_vap_stru *)mac_res_get_mac_vap(pst_mac_device->auc_vap_id[uc_vap_tmp_idx]);
                if (pst_tmp_vap == OAL_PTR_NULL) {
                    OAM_ERROR_LOG1(0, OAM_SF_SCAN, "{dmac_config_add_vap::pst_mac_vap null,vap_idx=%d.}",
                                   pst_mac_device->auc_vap_id[uc_vap_tmp_idx]);
                    continue;
                }

                if ((pst_tmp_vap->en_vap_state == MAC_VAP_STATE_UP) && (pst_tmp_vap != pst_mac_vap)) {
                    pst_mac_vap->st_channel.en_band = pst_tmp_vap->st_channel.en_band;
                    pst_mac_vap->st_channel.en_bandwidth = pst_tmp_vap->st_channel.en_bandwidth;
                    pst_mac_vap->st_channel.uc_chan_number = pst_tmp_vap->st_channel.uc_chan_number;
                    pst_mac_vap->st_channel.uc_idx = pst_tmp_vap->st_channel.uc_idx;
                    break;
                }
            }
        }
#endif
    } else {
        /* ?offload???,????HMAC????? */
        pst_mac_device->auc_vap_id[pst_mac_device->uc_vap_num--] = 0;

        /* device?sta???1 */
        if (en_vap_mode == WLAN_VAP_MODE_BSS_STA) {
            pst_mac_device->uc_sta_num--;

            /* ???uc_assoc_vap_id??????ap??? */
            pst_mac_vap->uc_assoc_vap_id = 0xff;
        }

#ifdef _PRE_WLAN_FEATURE_P2P
        pst_mac_vap->en_p2p_mode = en_p2p_mode;
        mac_dec_p2p_num(pst_mac_vap);
#endif
    }
}

oal_void mac_device_set_dfr_reset(mac_device_stru *pst_mac_device, oal_uint8 uc_device_reset_in_progress)
{
    pst_mac_device->uc_device_reset_in_progress = uc_device_reset_in_progress;
}

oal_void mac_device_set_state(mac_device_stru *pst_mac_device, oal_uint8 en_device_state)
{
    pst_mac_device->en_device_state = en_device_state;
}

oal_void mac_device_set_channel(mac_device_stru *pst_mac_device, mac_cfg_channel_param_stru *pst_channel_param)
{
    pst_mac_device->uc_max_channel = pst_channel_param->uc_channel;
    pst_mac_device->en_max_band = pst_channel_param->en_band;
    pst_mac_device->en_max_bandwidth = pst_channel_param->en_bandwidth;
}

oal_void mac_device_get_channel(mac_device_stru *pst_mac_device, mac_cfg_channel_param_stru *pst_channel_param)
{
    pst_channel_param->uc_channel = pst_mac_device->uc_max_channel;
    pst_channel_param->en_band = pst_mac_device->en_max_band;
    pst_channel_param->en_bandwidth = pst_mac_device->en_max_bandwidth;
}

oal_void mac_device_set_txchain(mac_device_stru *pst_mac_device, oal_uint8 uc_tx_chain)
{
    pst_mac_device->uc_tx_chain = uc_tx_chain;
}

oal_void mac_device_set_rxchain(mac_device_stru *pst_mac_device, oal_uint8 uc_rx_chain)
{
    pst_mac_device->uc_rx_chain = uc_rx_chain;
}

oal_void mac_device_set_beacon_interval(mac_device_stru *pst_mac_device, oal_uint32 ul_beacon_interval)
{
    pst_mac_device->ul_beacon_interval = ul_beacon_interval;
}

oal_void mac_device_inc_active_user(mac_device_stru *pst_mac_device)
{
    /* ?????+1 */
    pst_mac_device->uc_active_user_cnt++;
}

oal_void mac_device_dec_active_user(mac_device_stru *pst_mac_device)
{
    if (pst_mac_device->uc_active_user_cnt) {
        pst_mac_device->uc_active_user_cnt--;
    }
}


oal_void *mac_device_get_all_rates(mac_device_stru *pst_dev)
{
    return (oal_void *)pst_dev->st_mac_rates_11g;
}


oal_uint32 mac_device_find_legacy_sta(mac_device_stru *pst_mac_device, mac_vap_stru **ppst_mac_vap)
{
    oal_uint8 uc_vap_idx;
    mac_vap_stru *pst_mac_vap = OAL_PTR_NULL;

    for (uc_vap_idx = 0; uc_vap_idx < pst_mac_device->uc_vap_num; uc_vap_idx++) {
        pst_mac_vap = mac_res_get_mac_vap(pst_mac_device->auc_vap_id[uc_vap_idx]);
        if (oal_unlikely(pst_mac_vap == OAL_PTR_NULL)) {
            OAM_WARNING_LOG1(0, OAM_SF_SCAN, "vap is null! vap id is %d", pst_mac_device->auc_vap_id[uc_vap_idx]);

            *ppst_mac_vap = OAL_PTR_NULL;

            return OAL_ERR_CODE_PTR_NULL;
        }

        if (IS_LEGACY_STA(pst_mac_vap)) {
            *ppst_mac_vap = pst_mac_vap;

            return OAL_SUCC;
        }
    }

    *ppst_mac_vap = OAL_PTR_NULL;

    return OAL_FAIL;
}
oal_module_symbol(mac_device_set_vap_id);
oal_module_symbol(mac_device_set_dfr_reset);
oal_module_symbol(mac_device_set_state);
oal_module_symbol(mac_device_get_channel);
oal_module_symbol(mac_device_set_channel);
oal_module_symbol(mac_device_set_txchain);
oal_module_symbol(mac_device_set_rxchain);
oal_module_symbol(mac_device_set_beacon_interval);
oal_module_symbol(mac_device_inc_active_user);
oal_module_symbol(mac_device_dec_active_user);

oal_module_symbol(mac_device_init);
oal_module_symbol(mac_chip_init);
oal_module_symbol(mac_board_init);

oal_module_symbol(mac_device_exit);
oal_module_symbol(mac_chip_exit);
oal_module_symbol(mac_board_exit);

oal_module_symbol(mac_device_find_up_vap);
oal_module_symbol(mac_device_find_another_up_vap);
oal_module_symbol(mac_device_find_up_ap);
oal_module_symbol(mac_device_calc_up_vap_num);
oal_module_symbol(mac_device_calc_work_vap_num);
oal_module_symbol(mac_device_is_p2p_connected);
oal_module_symbol(mac_device_find_2up_vap);
oal_module_symbol(mac_device_find_up_p2p_go);

