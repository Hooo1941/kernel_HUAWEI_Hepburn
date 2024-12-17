/*
* Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
* 功能说明 : FTM帧
* 作    者 : wifi
* 创建日期 : 2022年11月22日
*/

#include "oal_kernel_file.h"
#include "mac_data.h"
#include "hmac_resource.h"
#include "hmac_host_tx_data.h"
#include "hmac_tx_data.h"
#include "hmac_config.h"
#include "plat_pm_wlan.h"
#include "hmac_ftm.h"
#include "host_hal_ext_if.h"
#include "hmac_scan.h"
#include "hmac_roam_scan.h"
#include "oam_event_wifi.h"
#include "securec.h"
#include "securectype.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_FTM_FRAME_C

#ifdef _PRE_WLAN_FEATURE_FTM
#define MAC_FTMP_LEN 9

uint8_t hmac_ftm_get_the_power_of_two(uint16_t num)
{
    uint8_t exponent = 0;
    while (num >> 1) {
        exponent++;
        num = num >> 1;
    }

    return exponent;
}

void hmac_ftm_set_mgmt_mac_hdr(uint8_t *mgmt_hdr, uint8_t *addr1, uint8_t *addr2, uint8_t *addr3)
{
    /* 设置 Frame Control field */
    mac_hdr_set_frame_control(mgmt_hdr, WLAN_PROTOCOL_VERSION | WLAN_FC0_TYPE_MGT | WLAN_FC0_SUBTYPE_ACTION);

    /* 设置分片序号为0 */
    mac_hdr_set_fragment_number(mgmt_hdr, 0);

    /* 设置 address1(接收端): AP  (BSSID) */
    oal_set_mac_addr(mgmt_hdr + WLAN_HDR_ADDR1_OFFSET, addr1);

     /* 设置 address2(发送端): dot11StationID */
    oal_set_mac_addr(mgmt_hdr + WLAN_HDR_ADDR2_OFFSET, addr2);

    /* 设置 address3: AP  (BSSID) */
    oal_set_mac_addr(mgmt_hdr + WLAN_HDR_ADDR3_OFFSET, addr3);
}

OAL_STATIC uint16_t hmac_ftm_set_lci_field(uint8_t *payload_addr)
{
    uint16_t len = 0;
    payload_addr[len++] = MAC_EID_MEASREQ;          /* Element ID */
    payload_addr[len++] = 4;                        /* 4 is Length */
    payload_addr[len++] = 1;                        /* 1 is Measurement Token */
    payload_addr[len++] = 0;                        /* Measurement Req Mode */
    payload_addr[len++] = RM_RADIO_MEASUREMENT_LCI; /* Measurement Type */
    payload_addr[len++] = 1; /* Location Subjest, 0: local, 1: remote, 2: Third Party */
    return len;
}

OAL_STATIC uint16_t hmac_ftm_set_civic_field(uint8_t *payload_addr)
{
    uint16_t len = 0;
    payload_addr[len++] = MAC_EID_MEASREQ;                     /* Element ID */
    payload_addr[len++] = 8;                                   /* 8 is Length */
    payload_addr[len++] = 2;                                   /* 2 is Measurement Token */
    payload_addr[len++] = 0;                                   /* Measurement Req Mode */
    payload_addr[len++] = RM_RADIO_MEASUREMENT_LOCATION_CIVIC; /* Measurement Type */

    /*****************************************************************************************************/
    /* Location Civic Request */
    /* ------------------------------------------------------------------------------------------------- */
    /* |Location Subject |Civic Location Type |Location Service Interval Units|Location Service Interva|
        Optional Subelements| */
    /* ------------------------------------------------------------------------------------------------- */
    /* |1                |1                   | 1                             | 2                      |
        var                 | */
    /* ------------------------------------------------------------------------------------------------- */
    /*****************************************************************************************************/
    payload_addr[len++] = 1; /* Location Subjest, 0: local, 1: remote, 2: Third Party */
    payload_addr[len++] = 0;
    payload_addr[len++] = 0;
    payload_addr[len++] = 0; /* 0 means only a single Location Civic report is requested */
    payload_addr[len++] = 0;
    return len;
}

static uint8_t hmac_ftm_responder_set_dialog_token(uint8_t *payload_addr,
    ftm_responder_stru *ftm_resp_session)
{
    /* 立即测量和非立即测量模式dialog token值计算不同 */
    if (ftm_resp_session->is_asap_on == OAL_TRUE) {
        if ((ftm_resp_session->current_left_burst_cnt == 1) &&
            (ftm_resp_session->dialog_token >=
            (ftm_resp_session->ftms_per_burst * ftm_resp_session->burst_cnt))) {
            *payload_addr = 0;
            *(payload_addr + 1) = ftm_resp_session->dialog_token - 1;
            ftm_resp_session->current_left_burst_cnt--;
        } else {
            *payload_addr = ftm_resp_session->dialog_token;
            *(payload_addr + 1) = ftm_resp_session->dialog_token - 1;
            ftm_resp_session->dialog_token++;
            ftm_resp_session->follow_up_dialog_token++;
        }
    } else {
        if ((ftm_resp_session->current_left_burst_cnt == 1) &&
            (ftm_resp_session->dialog_token >=
            (ftm_resp_session->ftms_per_burst * ftm_resp_session->burst_cnt + 1))) {
            *payload_addr = 0;
            *(payload_addr + 1) = ftm_resp_session->dialog_token - 1;
            ftm_resp_session->current_left_burst_cnt--;
        } else {
            *payload_addr = ftm_resp_session->dialog_token;
            if (ftm_resp_session->dialog_token == 2) { /* 2: dialog_token */
                *(payload_addr + 1) = 0;
            } else {
                *(payload_addr + 1) = ftm_resp_session->dialog_token - 1;
            }
            ftm_resp_session->dialog_token++;
            ftm_resp_session->follow_up_dialog_token++;
        }
    }

    return FTM_FRAME_DIALOG_TOKEN_LEN;
}

static uint8_t hmac_ftm_responder_set_loc_civic_field(uint8_t *payload_addr,
    ftm_responder_stru *ftm_resp_session)
{
    uint8_t len = 0;
    /*****************************************************************************************************/
    /* LCI Measurement Request (Measurement Request IE) */
    /* -------------------------------------------------------------------------------------------------- */
    /* |Element ID |Length |Measurement Token| Measurement Req Mode|Measurement Type  | Measurement Req | */
    /* -------------------------------------------------------------------------------------------------- */
    /* |1          |1      | 1               | 1                   |1                  |var             | */
    /* -------------------------------------------------------------------------------------------------- */
    /******************************************************************************************************/
    /* 封装LCI   Report field */
    if (ftm_resp_session->lci_ie == OAL_TRUE) {
        payload_addr[len++] = MAC_EID_MEASREQ;                   /* Element ID */
        payload_addr[len++] = MAC_MEASUREMENT_REQUEST_IE_OFFSET; /* Length */
        payload_addr[len++] = 1;                                 /* Measurement Token */
        payload_addr[len++] = 0;                                 /* Measurement Req Mode */
        payload_addr[len++] = RM_RADIO_MEASUREMENT_LCI;          /* Measurement Type */
    }

    /* 封装LOCATION_CIVIC   field */
    if (ftm_resp_session->civic_ie == OAL_TRUE) {
        payload_addr[len++] = MAC_EID_MEASREQ;                     /* Element ID */
        payload_addr[len++] = MAC_MEASUREMENT_REQUEST_IE_OFFSET;   /* Length */
        payload_addr[len++] = 1;                                   /* Measurement Token */
        payload_addr[len++] = 0;                                   /* Measurement Req Mode */
        payload_addr[len++] = RM_RADIO_MEASUREMENT_LOCATION_CIVIC; /* Measurement Type */
    }

    return len;
}

static uint8_t hmac_ftm_responder_set_tsf_sync_info(uint8_t *payload_addr,
    ftm_responder_stru *ftm_resp_session)
{
    uint8_t len = 0;
    int32_t ret;

    /*************************************************************/
    /* FTM Synchronization Information element */
    /* --------------------------------------------------------- */
    /* |Element ID |Length |Element ID Extension | TSF Sync Info| */
    /* --------------------------------------------------------- */
    /* |1          |1      | 1                   | 4            | */
    /* --------------------------------------------------------- */
    /*************************************************************/
    if (ftm_resp_session->ftm_sync_info == OAL_TRUE) {
        payload_addr[len++] = MAC_EID_FTMSI; /* Element ID */
        payload_addr[len++] = FTM_TSF_SYNC_INFO_LEN; /* Length */
        payload_addr[len++] = MAC_EID_EXT_FTMSI;
        payload_addr += len;
        ret = memcpy_s(payload_addr, sizeof(uint32_t), &(ftm_resp_session->tsf), sizeof(uint32_t));
        if (ret != EOK) {
            oam_error_log1(0, 0, "memcpy fail!ret[%d]", ret);
        }

        len += sizeof(uint32_t);
    }

    return len;
}

OAL_STATIC uint8_t hmac_ftm_set_ht_format(wlan_bw_cap_enum_uint8 band_cap)
{
    uint8_t format_and_bandwidth;

    switch (band_cap) {
        case WLAN_BW_CAP_20M:
            format_and_bandwidth = 9;  // 9 : HT mixed - 20M
            break;
        case WLAN_BW_CAP_40M:
            format_and_bandwidth = 11; // 11: HT mixed - 20M
            break;
        default:
            format_and_bandwidth = 9;  // 9 : HT mixed - 20M
            break;
    }

    return format_and_bandwidth;
}

OAL_STATIC OAL_INLINE uint8_t hmac_ftm_set_vht_format(wlan_bw_cap_enum_uint8 band_cap)
{
    uint8_t format_and_bandwidth;

    switch (band_cap) {
        case WLAN_BW_CAP_20M:
            format_and_bandwidth = 10; // 10: VHT - 20M
            break;
        case WLAN_BW_CAP_40M:
            format_and_bandwidth = 12; // 12: VHT - 40M
            break;
        case WLAN_BW_CAP_80M:
            format_and_bandwidth = 13; // 13: VHT - 80M
            break;
        case WLAN_BW_CAP_80PLUS80:
            format_and_bandwidth = 14; // 14: VHT - 80M+80M
            break;
        case WLAN_BW_CAP_160M:
            format_and_bandwidth = 15; // 15: VHT(two separate RF LOs) - 160M
            break;
        default:
            format_and_bandwidth = 9;  // 9 : HT mixed - 20M
            break;
    }

    return format_and_bandwidth;
}

void hmac_ftm_set_format_and_bandwidth(wlan_bw_cap_enum_uint8 band_cap,
    wlan_phy_protocol_enum_uint8 protocol_mode, mac_ftm_parameters_ie_stru *hmac_mac_ftmp)
{
    hmac_mac_ftmp->bit_format_and_bandwidth = 9; // 9(default) : HT mixed - 20M

    if (protocol_mode == WLAN_HT_PHY_PROTOCOL_MODE) {
        hmac_mac_ftmp->bit_format_and_bandwidth = hmac_ftm_set_ht_format(band_cap);
    } else if (protocol_mode == WLAN_VHT_PHY_PROTOCOL_MODE) {
        hmac_mac_ftmp->bit_format_and_bandwidth = hmac_ftm_set_vht_format(band_cap);
    }
}

void hmac_ftm_initialize_parameter_ie(mac_ftm_parameters_ie_stru *mac_ftmp, ftm_responder_stru *ftm_resp_session)
{
    mac_ftmp->uc_eid = MAC_EID_FTMP;
    mac_ftmp->uc_len = MAC_FTMP_LEN;
    mac_ftmp->bit_number_of_bursts_exponent = 0;
    mac_ftmp->bit_burst_duration = FTM_BURST_DURATION;
    mac_ftmp->bit_partial_tsf_timer_no_preference = 0;
    mac_ftmp->bit_asap_capable = 1;
    mac_ftmp->bit_status_indication = OAL_TRUE; /* Reserved */

     /* ini STA no preference or res sta can't support ini sta's Number of Bursts Exponent */
    if (ftm_resp_session->burst_cnt == FTM_INI_STA_NO_PREF_BURST_NUM) {
        ftm_resp_session->burst_cnt = FTM_INI_STA_NO_PREF_RES_STA_BURST_NUM;
    } else {
        if ((ftm_resp_session->burst_cnt * ftm_resp_session->ftms_per_burst) > FTM_RES_STA_DIALOG_TOKEN_MAX) {
            mac_ftmp->bit_status_indication = FTM_STATUS_REQUEST_INCAPABLE;
        }
    }

    mac_ftmp->bit_burst_duration = hmac_ftm_get_the_power_of_two((uint16_t)ftm_resp_session->burst_duration) +
        FTM_MIN_DURATION;
    ftm_resp_session->status_indication = mac_ftmp->bit_status_indication;

    mac_ftmp->bit_value = 0;                    /* Reserved */
}

OAL_STATIC void hmac_ftm_responder_set_partial_tsf(ftm_responder_stru *ftm_res_session,
    mac_ftm_parameters_ie_stru *mac_ftm_paras)
{
    uint32_t pstf_offset;
    /* ASAP = 1 PTSF Timer set to a value less than 10 ms from the reception of the most recent initial FTM Req frame */
    if (ftm_res_session->is_asap_on == OAL_TRUE) {
        pstf_offset = FTM_RES_ASAP_SET_PTSF_OFFSET;
    } else {
        pstf_offset = FTM_RES_ASAP_SET_PTSF_OFFSET + ftm_res_session->res_ptsf_offset;
    }
    mac_ftm_paras->us_partial_tsf_timer =
        (uint16_t)(((ftm_res_session->tsf + pstf_offset) >> BIT_OFFSET_10) % BIT16);

    oam_warning_log4(0, OAM_SF_FTM, "hmac_ftm_responder_set_partial_tsf::tsf[0x%X], res_ptsf_offset[%u us] \
        (tsf + 3000us + res_ptsf_offset)[0x%X] PTSF[0x%X]", ftm_res_session->tsf,
        ftm_res_session->res_ptsf_offset, (ftm_res_session->tsf + pstf_offset),
        mac_ftm_paras->us_partial_tsf_timer);
}

static uint8_t hmac_ftm_responder_set_ftm_para(uint8_t *payload_addr, ftm_responder_stru *ftm_resp_session)
{
    mac_ftm_parameters_ie_stru *mac_ftm_param = NULL;
    uint16_t burst_cnt;

    /*******************************************************************/
    /* Fine Timing Measurement Parameters element */
    /* --------------------------------------------------------------- */
    /* |Element ID |Length |Fine Timing Measurement Parameters| */
    /* --------------------------------------------------------------- */
    /* |1          |1      | 9                                | */
    /* --------------------------------------------------------------- */
    /*******************************************************************/
    /*****************************************************************************************************************/
    /* Fine Timing Measurement Parameters */
    /* --------------------------------------------------------------------------------------------------------------*/
    /* |B0             B1 |B2 B6 |B7       | B8                   B11 |B12        B15 |B16       B23 |B24      B39 | */
    /* --------------------------------------------------------------------------------------------------------------*/
    /* |Status Indication|Value|Reserved|Number of Bursts Exponent|Burst Duration|Min Delta FTM |Partial TSF Timer | */
    /* --------------------------------------------------------------------------------------------------------------*/
    /* |B40                             |B41          |B42  |B43        B47 |B48  B49 |B50       B55 |B56      B71 | */
    /* --------------------------------------------------------------------------------------------------------------*/
    /* |Partial TSF Timer No Preference|ASAP Capable|ASAP|FTMs per Burst|Reserved|Format And Bandwidth|Burst Period| */
    /*****************************************************************************************************************/
    if (ftm_resp_session->ftm_parameters == OAL_TRUE) {
        /* 封装 Fine Timing Measurement Parameters element */
        mac_ftm_param = (mac_ftm_parameters_ie_stru *)payload_addr;
        memset_s(mac_ftm_param, sizeof(mac_ftm_parameters_ie_stru), 0, sizeof(mac_ftm_parameters_ie_stru));

        hmac_ftm_initialize_parameter_ie(mac_ftm_param, ftm_resp_session);
        burst_cnt = ftm_resp_session->burst_cnt;
        ftm_resp_session->current_left_burst_cnt = ftm_resp_session->burst_cnt;
        while (burst_cnt >> 1) {
            mac_ftm_param->bit_number_of_bursts_exponent++;
            burst_cnt = burst_cnt >> 1;
        }
        hmac_ftm_responder_set_partial_tsf(ftm_resp_session, mac_ftm_param);
        mac_ftm_param->bit_asap = ftm_resp_session->is_asap_on;

        /* 设置带宽协议 */
        hmac_ftm_set_format_and_bandwidth(ftm_resp_session->band_cap, ftm_resp_session->prot_format, mac_ftm_param);

        /* 每个burst发ftm帧的个数，ftm帧之间的间隔，以及burst的间隔 */
        mac_ftm_param->bit_ftms_per_burst = ftm_resp_session->ftms_per_burst;
        mac_ftm_param->uc_min_delta_ftm = ftm_resp_session->min_delta_ftm;
        mac_ftm_param->us_burst_period = ftm_resp_session->burst_period;

        return sizeof(mac_ftm_parameters_ie_stru);
    }

    return 0;
}

static uint16_t hmac_encap_ftm_frame(hmac_vap_stru *hmac_vap, oal_netbuf_stru *buffer, void *param)
{
    ftm_responder_stru *ftm_responder_session = (ftm_responder_stru *)param;
    uint8_t *mgmt_hdr = oal_netbuf_header(buffer);
    uint8_t *payload_addr = NULL;
    uint16_t idx = 0;
    int32_t ret;
    uint8_t *addr3 = NULL;

    if (oal_any_null_ptr2(buffer, ftm_responder_session)) {
        oam_error_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM, "{hmac_encap_ftm_frame::failed}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (hmac_vap->st_vap_base_info.en_vap_mode == WLAN_VAP_MODE_BSS_STA) {
        addr3 = ftm_responder_session->mac_ra;
    } else {
        addr3 = hmac_vap->st_vap_base_info.auc_bssid;
    }

    hmac_ftm_set_mgmt_mac_hdr(mgmt_hdr, ftm_responder_session->mac_ra,
        mac_mib_get_StationID(&hmac_vap->st_vap_base_info), addr3);

    /***************************************************************************************************/
    /* Fine Timing Measurement frame format */
    /* ----------------------------------------------------------------------------------------------- */
    /* |Category |Public Action |Dialog Token |Follow Up Dialog Token |TOD |TOA |TOD Error |TOA Error | */
    /* ----------------------------------------------------------------------------------------------- */
    /* |1        |1             |1            |1                      |6   |6   |2         |2         | */
    /* ----------------------------------------------------------------------------------------------- */
    /* |LCI Report (optional) | */
    /* ----------------------------------------------------------------------------------------------- */
    /* |Variable                                    | */
    /* ----------------------------------------------------------------------------------------------- */
    /* |Location Civic MeasuremenRequest (optional) | */
    /* ----------------------------------------------------------------------------------------------- */
    /* |Variable                                    | */
    /* ----------------------------------------------------------------------------------------------- */
    /* |Fine Timing MeasuremenParameters (optional) | */
    /* ----------------------------------------------------------------------------------------------- */
    /* |Variable                                    | */
    /* ----------------------------------------------------------------------------------------------- */
    /* |FTM Synchronization Information (optional) | */
    /* ----------------------------------------------------------------------------------------------- */
    /* |Variable                                    | */
    /* ----------------------------------------------------------------------------------------------- */
    /***************************************************************************************************/
    payload_addr = mac_get_80211_mgmt_payload(buffer);
    if (payload_addr == NULL) {
        oam_warning_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM, "hmac_encap_ftm_frame::payload_addr NULL!");
        return 0;
    }

    payload_addr[idx++] = MAC_ACTION_CATEGORY_PUBLIC; /* Category */
    payload_addr[idx++] = MAC_PUB_FTM;                /* Public Action */

    idx += hmac_ftm_responder_set_dialog_token(payload_addr + idx, ftm_responder_session);
    ret = memcpy_s(payload_addr + idx, FTM_TODA_LENGTH, &ftm_responder_session->toda_info.tod,
        FTM_TODA_LENGTH);
    idx += FTM_TODA_LENGTH;
    ret += memcpy_s(payload_addr + idx, FTM_TODA_LENGTH, &ftm_responder_session->toda_info.toa,
        FTM_TODA_LENGTH);
    idx += FTM_TODA_LENGTH;
    ret += memcpy_s(payload_addr + idx, FTM_TODA_ERROR_LENGTH, &ftm_responder_session->toda_info.tod_error,
        FTM_TODA_ERROR_LENGTH);
    idx += FTM_TODA_ERROR_LENGTH;
    ret += memcpy_s(payload_addr + idx, FTM_TODA_ERROR_LENGTH, &ftm_responder_session->toda_info.toa_error,
        FTM_TODA_ERROR_LENGTH);
    idx += FTM_TODA_ERROR_LENGTH;

    if (ret != EOK) {
        oam_error_log1(hmac_vap->st_vap_base_info.uc_vap_id, 0, "memcpy fail!ret[%d]", ret);
    }

    idx += hmac_ftm_responder_set_loc_civic_field(payload_addr + idx, ftm_responder_session);
    idx += hmac_ftm_responder_set_ftm_para(payload_addr + idx, ftm_responder_session);
    idx += hmac_ftm_responder_set_tsf_sync_info(payload_addr + idx, ftm_responder_session);

    return (uint16_t)(idx + MAC_80211_FRAME_LEN);
}

OAL_STATIC void hmac_ftm_initiator_set_mac_ftmp_field(mac_ftm_parameters_ie_stru *hmac_mac_ftmp,
    ftm_initiator_stru *ftm_initiator_session)
{
    hmac_mac_ftmp->uc_eid = MAC_EID_FTMP;
    hmac_mac_ftmp->uc_len = MAC_FTMP_LEN;

    hmac_mac_ftmp->bit_status_indication = 0;
    hmac_mac_ftmp->bit_value = 0;

    /* 回合个数(2^number_of_bursts_exponent) */
    hmac_mac_ftmp->bit_number_of_bursts_exponent = hmac_ftm_get_the_power_of_two(ftm_initiator_session->burst_cnt);

    hmac_mac_ftmp->bit_burst_duration = FTM_BURST_DURATION;
    hmac_mac_ftmp->us_partial_tsf_timer = 0;
    hmac_mac_ftmp->bit_partial_tsf_timer_no_preference = 1;
    hmac_mac_ftmp->bit_asap_capable = 0;  // FTM认证，协议9.4.2.168 iFTMR中该bit应为0
    hmac_mac_ftmp->bit_asap = ftm_initiator_session->is_asap_on;
    hmac_mac_ftmp->bit_ftms_per_burst = ftm_initiator_session->ftms_per_burst;
    hmac_mac_ftmp->uc_min_delta_ftm = ftm_initiator_session->min_delta_ftm;
    hmac_mac_ftmp->us_burst_period = ftm_initiator_session->burst_period;
}

static uint16_t hmac_ftm_encap_request_frame(hmac_vap_stru *hmac_vap, oal_netbuf_stru *buffer, void *param)
{
    ftm_initiator_stru *ftm_initiator_session = (ftm_initiator_stru *)param;
    uint8_t *mgmt_hdr;
    uint8_t *payload_addr;
    uint16_t idx = 0;
    mac_ftm_parameters_ie_stru *hmac_mac_ftmp = NULL;
    uint8_t *addr3 = NULL;
    hal_host_device_stru *hal_device = NULL;

    if (oal_any_null_ptr3(hmac_vap, buffer, ftm_initiator_session)) {
        oam_error_log0(0, OAM_SF_FTM, "{hmac_ftm_encap_request_frame::hmac_vap or buffer or tx_ba is null.}");
        return 0;
    }

    hal_device = hal_get_host_device(hmac_vap->hal_dev_id);
    mgmt_hdr = oal_netbuf_header(buffer);
    payload_addr = mac_get_80211_mgmt_payload(buffer);
    if (oal_any_null_ptr3(hal_device, mgmt_hdr, payload_addr)) {
        oam_error_log0(0, OAM_SF_FTM, "{hmac_ftm_encap_request_frame::fail ot get mgmt_hdr/payload!}");
        return 0;
    }
    /* FTM认证，非关联状态下，bssid为全F */
    if ((hmac_vap->st_vap_base_info.en_vap_state == MAC_VAP_STATE_UP) ||
        (hmac_vap->st_vap_base_info.en_vap_state == MAC_VAP_STATE_PAUSE)) {
        addr3 = ftm_initiator_session->bssid;
    } else {
        addr3 = BROADCAST_MACADDR;
    }

    hmac_ftm_set_mgmt_mac_hdr(mgmt_hdr, ftm_initiator_session->bssid,
        mac_mib_get_StationID(&hmac_vap->st_vap_base_info), addr3);

    /* 配置FTM白名单寄存器，否则中断上报后PPU信息错误 */
    hal_host_ftm_set_white_list(hal_device, 0, ftm_initiator_session->bssid);

    /*************************************************************************************/
    /* FTM Request frame - Frame Body */
    /* --------------------------------------------------------------------------------- */
    /* |Category |Public Action |Trigger | LCI Measurement Request(optional)| */
    /* --------------------------------------------------------------------------------- */
    /* |1        |1             |1       |Variable                          | */
    /* --------------------------------------------------------------------------------- */
    /* |LCI Report (optional) | */
    /* --------------------------------------------------------------------------------- */
    /* |Variable                                    | */
    /* --------------------------------------------------------------------------------- */
    /* |Location Civic MeasuremenRequest (optional) | */
    /* --------------------------------------------------------------------------------- */
    /* |Variable                                    | */
    /* --------------------------------------------------------------------------------- */
    /* |Fine Timing MeasuremenParameters (optional) | */
    /* --------------------------------------------------------------------------------- */
    /* |Variable                                    | */
    /* --------------------------------------------------------------------------------- */
    /*************************************************************************************/

    payload_addr[idx++] = MAC_ACTION_CATEGORY_PUBLIC; /* Category */
    payload_addr[idx++] = MAC_PUB_FTM_REQ;            /* Public Action */
    payload_addr[idx++] = 1;                          /* Trigger */

    /* FTM认证，增加LCI，Location civic信元 */
    /*****************************************************************************************************/
    /* LCI Measurement Request (Measurement Request IE) */
    /* ------------------------------------------------------------------------------------------------- */
    /* |Element ID |Length |Measurement Token| Measurement Req Mode|Measurement Type  | Measurement Req | */
    /* ------------------------------------------------------------------------------------------------- */
    /* |1          |1      | 1               | 1                   |1                  |var             | */
    /* ------------------------------------------------------------------------------------------------- */
    /*****************************************************************************************************/
    if (ftm_initiator_session->is_initial_ftm_request == OAL_TRUE) {
        /* 封装Measurement Request IE */
        if (ftm_initiator_session->lci_ie == OAL_TRUE) {
            idx += hmac_ftm_set_lci_field(payload_addr + idx);
        }
        /* 封装Measurement Request IE */
        if (ftm_initiator_session->civic_ie == OAL_TRUE) {
            idx += hmac_ftm_set_civic_field(payload_addr + idx);
        }
    }

    /*******************************************************************/
    /* Fine Timing Measurement Parameters element */
    /* --------------------------------------------------------------- */
    /* |Element ID |Length |Fine Timing Measurement Parameters| */
    /* --------------------------------------------------------------- */
    /* |1          |1      | 9                                | */
    /* --------------------------------------------------------------- */
    /*******************************************************************/
    /*****************************************************************************************************************/
    /* Fine Timing Measurement Parameters */
    /* -------------------------------------------------------------------------------------------------------------*/
    /* |B0             B1 |B2 B6 |B7       | B8                   B11 |B12        B15 |B16       B23 |B24      B39 | */
    /* -------------------------------------------------------------------------------------------------------------*/
    /* |Status Indication |Value|Reserved|Number of Bursts Exponent|Burst Duration|Min Delta FTM|Partial TSF Timer| */
    /* --------------------------------------------------------------------------------------------------------------*/
    /* |B40                            |B41        |B42  |B43        B47 |B48  B49 |B50          B55 |B56      B71 | */
    /* --------------------------------------------------------------------------------------------------------------*/
    /* |Partial TSF Timer No Preference|ASAP Capable|ASAP|FTMs per Burst|Reserved|Format And Bandwidth|Burst Period| */
    /*****************************************************************************************************************/
    /* 封装 Fine Timing Measurement Parameters element */
    if (ftm_initiator_session->is_initial_ftm_request == OAL_TRUE) {
        hmac_mac_ftmp = (mac_ftm_parameters_ie_stru *)&(payload_addr[idx]);
        memset_s(hmac_mac_ftmp, sizeof(mac_ftm_parameters_ie_stru), 0, sizeof(mac_ftm_parameters_ie_stru));

        /* 填充参数 */
        hmac_ftm_initiator_set_mac_ftmp_field(hmac_mac_ftmp, ftm_initiator_session);

        /* 获取带宽 */
        hmac_ftm_set_format_and_bandwidth(ftm_initiator_session->band_cap,
                                          ftm_initiator_session->prot_format, hmac_mac_ftmp);

        idx = idx + sizeof(mac_ftm_parameters_ie_stru);
    }

    return (uint16_t)(idx + MAC_80211_FRAME_LEN);
}

OAL_STATIC void hmac_initialize_meas_rpt_ie(mac_meas_rpt_ie_stru *meas_rpt_ie)
{
    meas_rpt_ie->uc_eid = MAC_EID_MEASREP;
    meas_rpt_ie->uc_token = 1;
    memset_s(&(meas_rpt_ie->st_rptmode), sizeof(mac_meas_rpt_mode_stru), 0, sizeof(mac_meas_rpt_mode_stru));
    meas_rpt_ie->uc_rpttype = RM_RADIO_MEASUREMENT_FTM_RANGE;
}

uint16_t hmac_set_meas_rpt_ie_field(mac_meas_rpt_ie_stru *meas_rpt_ie, ftm_range_rpt_stru *ftm_range_rpt)
{
    uint8_t *data = NULL;
    uint16_t idx = 0;
    uint16_t i;
    int32_t ret = EOK;
    uint32_t range = 0;
    uint8_t range_len = 3; /* 3 is Range len all is zero */

    /******************************************************************************************/
    /* Fine Timing Measurement Range report */
    /* --------------------------------------------------------------------------------------- */
    /* |Range Entry Count |Range Entry |Error Entry Count| Error Entry |Optional Subelements | */
    /* --------------------------------------------------------------------------------------- */
    /* |1                 |M x 15      |1                | N x 11      | var                 | */
    /* --------------------------------------------------------------------------------------- */
    /******************************************************************************************/
    data = meas_rpt_ie->auc_meas_rpt;
    /******************************************************************************/
    /* Range Entry */
    /* --------------------------------------------------------------------------- */
    /* |Measurement Start Time |BSSID |Range |Max Range Error Exponent |Reserved | */
    /* --------------------------------------------------------------------------- */
    /* |4                      |6     |3     |  1                     | 1        | */
    /* --------------------------------------------------------------------------- */
    /******************************************************************************/
    data[idx++] = ftm_range_rpt->range_entry_count;
    /* FTM认证，打桩部分字段为0 */
    for (i = 0; i < ftm_range_rpt->range_entry_count; i++) {
        ret += memcpy_s(&data[idx], MEAS_RPT_IE_LEN - idx,
                        &ftm_range_rpt->ftm_range_entry[i].measurement_start_time, sizeof(uint32_t));
        idx += sizeof(uint32_t);
        ret += memcpy_s(&data[idx], MEAS_RPT_IE_LEN - idx,
                        ftm_range_rpt->ftm_range_entry[i].bssid, WLAN_MAC_ADDR_LEN);
        idx += WLAN_MAC_ADDR_LEN;
        ret += memcpy_s(&data[idx], range_len, &range, range_len);
        idx += range_len;
        data[idx++] = 0;  /* reserved */
        data[idx++] = 0;  /* reserved */
    }

    /***********************************************/
    /* Error Entry */
    /* -------------------------------------------- */
    /* |Measurement Start Time |BSSID |Error Code | */
    /* -------------------------------------------- */
    /* |4                      |6     |1          | */
    /* -------------------------------------------- */
    /***********************************************/
    data[idx++] = ftm_range_rpt->error_entry_count;
    for (i = 0; i < ftm_range_rpt->error_entry_count; i++) {
        ret += memcpy_s(&data[idx], MEAS_RPT_IE_LEN - idx,
                        &ftm_range_rpt->ftm_error_entry[i].measurement_start_time, sizeof(uint32_t));
        idx += sizeof(uint32_t);
        ret += memcpy_s(&data[idx], MEAS_RPT_IE_LEN - idx,
                        ftm_range_rpt->ftm_error_entry[i].bssid, WLAN_MAC_ADDR_LEN);
        idx += WLAN_MAC_ADDR_LEN;
        data[idx++] = ftm_range_rpt->ftm_error_entry[i].error_code;
    }
    if (ret != EOK) {
        oam_error_log0(0, OAM_SF_FTM, "hmac_set_meas_rpt_ie_field: memcopy fail!\n");
    }
    return idx;
}

static uint16_t hmac_encap_radio_measurement_report_frame(hmac_vap_stru *hmac_vap, oal_netbuf_stru *buffer, void *param)
{
    hmac_ftm_stru *ftm_info = (hmac_ftm_stru *)param;
    mac_vap_stru *mac_vap = &hmac_vap->st_vap_base_info;
    uint8_t *mac_header = oal_netbuf_header(buffer);
    uint8_t *payload_addr = mac_get_80211_mgmt_payload(buffer);
    mac_meas_rpt_ie_stru *meas_rpt_ie = NULL;
    uint16_t idx = 0;
    uint16_t ie_len;
    ftm_range_rpt_stru *ftm_range_rpt = NULL;

    ftm_range_rpt = &(ftm_info->ftm_range_rpt);

    /*************************************************************************/
    /* Management Frame Format */
    /* -------------------------------------------------------------------- */
    /* |Frame Control|Duration|DA|SA|BSSID|Sequence Control|Frame Body|FCS| */
    /* -------------------------------------------------------------------- */
    /* | 2           |2       |6 |6 |6    |2               |0 - 2312  |4  | */
    /* -------------------------------------------------------------------- */
    /*************************************************************************/
    hmac_ftm_set_mgmt_mac_hdr(mac_header, mac_vap->auc_bssid,
                              mac_mib_get_StationID(mac_vap), mac_vap->auc_bssid);

     /*************************************************************************************/
    /* Radio Measurement Report frame format */
    /* --------------------------------------------------------------------------------- */
    /* |Category |Radio Measurement Action |Dialog Token | Measurement Report Elements| */
    /* --------------------------------------------------------------------------------- */
    /* |1        |1                       |1             |Variable                    | */
    /* --------------------------------------------------------------------------------- */
    /*************************************************************************************/
    payload_addr[idx++] = MAC_ACTION_CATEGORY_RADIO_MEASURMENT;   /* Category */
    payload_addr[idx++] = MAC_RM_ACTION_RADIO_MEASUREMENT_REPORT; /* Public Action */
    payload_addr[idx++] = 1;                                      /* Dialog Token */

    /*************************************************************************/
    /* Measurement Report IE - Frame Body */
    /* --------------------------------------------------------------------- */
    /* |Element ID |Length |Meas Token| Meas Rpt Mode | Meas Type | Meas Rpt| */
    /* --------------------------------------------------------------------- */
    /* |1          |1      | 1        |  1            | 1         | var */
    /* --------------------------------------------------------------------- */
    /*************************************************************************/
    meas_rpt_ie = (mac_meas_rpt_ie_stru *)&payload_addr[idx];
    hmac_initialize_meas_rpt_ie(meas_rpt_ie);
    ie_len = hmac_set_meas_rpt_ie_field(meas_rpt_ie, ftm_range_rpt);
    meas_rpt_ie->uc_len = 3 + (uint8_t)ie_len; /* 帧体偏移位置3 */
    idx += (2 + meas_rpt_ie->uc_len);  /* 帧体偏移位置2 */

    return (uint16_t)(idx + MAC_80211_FRAME_LEN);
}

static uint16_t hmac_encap_neighbour_report_request_frame(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf, void *param)
{
    mac_neighbor_report_req_stru *nbr_req = (mac_neighbor_report_req_stru *)param;
    mac_vap_stru *mac_vap = &hmac_vap->st_vap_base_info;
    uint16_t idx = 0;
    uint8_t *data = NULL;
    uint8_t *mac_header = (uint8_t *)oal_netbuf_header(netbuf);

    /*************************************************************************/
    /*                        Management Frame Format                        */
    /* --------------------------------------------------------------------  */
    /* |Frame Control|Duration|DA|SA|BSSID|Sequence Control|Frame Body|FCS|  */
    /* --------------------------------------------------------------------  */
    /* | 2           |2       |6 |6 |6    |2               |0 - 2312  |4  |  */
    /* --------------------------------------------------------------------  */
    /*                                                                       */
    /*************************************************************************/
    /*************************************************************************/
    /*                Set the fields in the frame header                     */
    /*************************************************************************/
    /* All the fields of the Frame Control Field are set to zero. Only the   */
    /* Type/Subtype field is set.                                            */
    hmac_ftm_set_mgmt_mac_hdr(mac_header, nbr_req->auc_bssid, mac_mib_get_StationID(mac_vap), nbr_req->auc_bssid);

    /*************************************************************************/
    /*                Set the contents of the frame body                     */
    /*************************************************************************/
    /*************************************************************************/
    /*             Neighbor report request Frame - Frame Body                */
    /*        -------------------------------------------------              */
    /*        | Category | Action |  Dialog Token | Opt SubEle |             */
    /*        -------------------------------------------------              */
    /*        | 1        | 1      |       1       | Var        |             */
    /*        -------------------------------------------------              */
    /*************************************************************************/
    /* Initialize index and the frame data pointer */
    data = mac_get_80211_mgmt_payload(netbuf);
    /* Category */
    data[idx++] = MAC_ACTION_CATEGORY_RADIO_MEASURMENT;

    /* Action */
    data[idx++] = MAC_RM_ACTION_NEIGHBOR_REPORT_REQUEST;

    /* Dialog Token */
    data[idx++] = 1;

    /*
    * FTM认证，组帧neighbour report Request，需要包含lci和location civic信元
    */
    /*****************************************************************************************************/
    /* LCI Measurement Request (Measurement Request IE) */
    /* ------------------------------------------------------------------------------------------------- */
    /* |Element ID |Length |Measurement Token| Measurement Req Mode|Measurement Type  | Measurement Req | */
    /* ------------------------------------------------------------------------------------------------- */
    /* |1          |1      | 1               | 1                   |1                  |var             | */
    /* ------------------------------------------------------------------------------------------------- */
    /*****************************************************************************************************/
    /* 封装Measurement Request IE */
    idx += hmac_ftm_set_lci_field(data + idx);
    /* 封装Measurement Request IE */
    idx += hmac_ftm_set_civic_field(data + idx);

    return (uint16_t)(idx + MAC_80211_FRAME_LEN);
}

static void hmac_ftm_tx_cb_init(hmac_ftm_generate_netbuf_ctx_stru *ctx)
{
    mac_tx_ctl_stru *tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(ctx->netbuf);

    memset_s(tx_ctl, oal_netbuf_cb_size(), 0, oal_netbuf_cb_size());
    mac_get_cb_frame_type(tx_ctl) = WLAN_CB_FRAME_TYPE_ACTION;
    mac_get_cb_frame_subtype(tx_ctl) = ctx->frame_type;

    if (!ctx->user_mac_addr) {
        return;
    }

    if (mac_vap_set_cb_tx_user_idx(&ctx->hmac_vap->st_vap_base_info, tx_ctl, ctx->user_mac_addr) != OAL_SUCC) {
        oam_warning_log3(0, 0, "(hmac_ftm_tx_cb_init::fail to find user[%02X:XX:XX:XX:%02X:%02X]}",
            ctx->user_mac_addr[MAC_ADDR_0], ctx->user_mac_addr[MAC_ADDR_4], ctx->user_mac_addr[MAC_ADDR_5]);
    }
}

static void hmac_mgmt_tx_set_ftm_ctx(hmac_ftm_generate_netbuf_ctx_stru *ctx, uint8_t *ftm_ctx_action_addr)
{
    hmac_ftm_stru *hmac_ftm_info = (hmac_ftm_stru *)ctx->hmac_vap->feature_ftm;
    mac_ftm_action_event_stru *ftm_ctx_action = (mac_ftm_action_event_stru *)ftm_ctx_action_addr;

    if (!hmac_ftm_info) {
        return;
    }

    ftm_ctx_action->frame_len = ctx->payload_len;
    ftm_ctx_action->hdr_len = MAC_80211_FRAME_LEN;
    ftm_ctx_action->cali = hmac_ftm_info->cali;
    ftm_ctx_action->chain_selection = hmac_ftm_info->ftm_chain_selection;
    ftm_ctx_action->band_cap = ctx->band_cap;
    ftm_ctx_action->prot_format = ctx->prot_format;
    ftm_ctx_action->divider = hmac_ftm_info->divider;
}

static void hmac_ftm_tx_action_init(hmac_ftm_generate_netbuf_ctx_stru *ctx)
{
    if (!ctx->need_ftm_action) {
        return;
    }

    hmac_mgmt_tx_set_ftm_ctx(ctx, (uint8_t *)(oal_netbuf_data(ctx->netbuf) + ctx->payload_len));
    ctx->payload_len += sizeof(mac_ftm_action_event_stru);
}

static void hmac_ftm_tx_netbuf_init(hmac_ftm_generate_netbuf_ctx_stru *ctx)
{
    hmac_ftm_tx_action_init(ctx);
    hmac_ftm_tx_cb_init(ctx);
    oal_netbuf_put(ctx->netbuf, ctx->payload_len);
}

typedef struct {
    uint8_t frame_type;
    uint16_t (*func)(hmac_vap_stru *, oal_netbuf_stru *, void *);
} hmac_ftm_encap_frame_func_stru;

hmac_ftm_encap_frame_func_stru g_ftm_encap_frame_handler[] = {
    { WLAN_ACTION_FTM_RESPONE, hmac_encap_ftm_frame },
    { WLAN_ACTION_FTM_REQUEST, hmac_ftm_encap_request_frame },
    { WLAN_ACTION_FTM_RADIO_MEASUREMENT_REPORT, hmac_encap_radio_measurement_report_frame },
    { WLAN_ACTION_FTM_NEIGHBOR_REPORT_REQUEST, hmac_encap_neighbour_report_request_frame },
};

static uint16_t hmac_ftm_encap_frame(hmac_ftm_generate_netbuf_ctx_stru *ctx)
{
    uint8_t index;
    uint8_t entries = sizeof(g_ftm_encap_frame_handler) / sizeof(hmac_ftm_encap_frame_func_stru);

    for (index = 0; index < entries; index++) {
        if (g_ftm_encap_frame_handler[index].frame_type != ctx->frame_type) {
            continue;
        }

        if (!g_ftm_encap_frame_handler[index].func) {
            break;
        }

        return g_ftm_encap_frame_handler[index].func(ctx->hmac_vap, ctx->netbuf, ctx->param);
    }

    return 0;
}

static void hmac_ftm_generate_netbuf(hmac_ftm_generate_netbuf_ctx_stru *ctx)
{
    ctx->netbuf = oal_mem_netbuf_alloc(OAL_NORMAL_NETBUF, WLAN_MEM_NETBUF_SIZE2, OAL_NETBUF_PRIORITY_MID);
    if (!ctx->netbuf) {
        return;
    }

    oal_mem_netbuf_trace(ctx->netbuf, OAL_TRUE);
    oal_netbuf_prev(ctx->netbuf) = NULL;
    oal_netbuf_next(ctx->netbuf) = NULL;

    ctx->payload_len = hmac_ftm_encap_frame(ctx);
    hmac_ftm_tx_netbuf_init(ctx);
}

uint32_t hmac_ftm_send_frame(hmac_ftm_generate_netbuf_ctx_stru *ctx)
{
    hmac_ftm_generate_netbuf(ctx);
    if (!ctx->netbuf) {
        return OAL_FAIL;
    }

    if (hmac_tx_mgmt_send_event(&ctx->hmac_vap->st_vap_base_info, ctx->netbuf, ctx->payload_len) != OAL_SUCC) {
        oal_netbuf_free(ctx->netbuf);
        return OAL_FAIL;
    }

    return OAL_SUCC;
}
#endif
