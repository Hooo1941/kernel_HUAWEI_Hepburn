/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : HMAC模块下，AP与STA公用的管理帧接口实现的源文件
 * 作     者 :
 * 创建日期 : 2013年4月10日
 */
#include "oal_kernel_file.h"
#include "wlan_spec.h"
#include "mac_vap.h"
#include "mac_ie.h"
#include "mac_frame.h"
#include "hmac_mgmt_bss_comm.h"
#include "mac_resource.h"
#include "hmac_device.h"
#include "hmac_resource.h"
#include "hmac_fsm.h"
#include "hmac_encap_frame.h"
#include "hmac_tx_amsdu.h"
#include "hmac_mgmt_ap.h"
#include "hmac_mgmt_sta.h"
#include "hmac_blockack.h"
#include "hmac_p2p.h"
#include "hmac_dfx.h"
#include "hmac_csi.h"
#include "hmac_encap_frame_ap.h"
#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_MGMT_BSS_COMM_C

#ifdef _PRE_WLAN_FEATURE_SNIFFER
#ifdef CONFIG_HW_SNIFFER
#include <hwnet/ipv4/sysctl_sniffer.h>
#endif
#endif
#include "securec.h"
#include "securectype.h"

#define FILE_LEN 128
#define BASE_YEAR 1900
/*
 * 行:代表VAP 的协议能力
 * 列:代表USER 的协议能力
 */
#if defined(_PRE_WLAN_FEATURE_11AX) || defined(_PRE_WLAN_FEATURE_11AX_ROM)
uint8_t g_auc_avail_protocol_mode[WLAN_PROTOCOL_BUTT][WLAN_PROTOCOL_BUTT] = {
    { WLAN_LEGACY_11A_MODE, WLAN_PROTOCOL_BUTT,   WLAN_PROTOCOL_BUTT,   WLAN_PROTOCOL_BUTT,
      WLAN_PROTOCOL_BUTT,   WLAN_LEGACY_11A_MODE, WLAN_LEGACY_11A_MODE, WLAN_PROTOCOL_BUTT,
      WLAN_PROTOCOL_BUTT,   WLAN_PROTOCOL_BUTT,   WLAN_LEGACY_11A_MODE },    // 11A
    { WLAN_PROTOCOL_BUTT,   WLAN_LEGACY_11B_MODE, WLAN_LEGACY_11B_MODE, WLAN_LEGACY_11B_MODE,
      WLAN_LEGACY_11B_MODE, WLAN_LEGACY_11B_MODE, WLAN_LEGACY_11B_MODE, WLAN_PROTOCOL_BUTT,
      WLAN_PROTOCOL_BUTT,   WLAN_PROTOCOL_BUTT,   WLAN_LEGACY_11B_MODE },    // 11B
    { WLAN_PROTOCOL_BUTT,   WLAN_PROTOCOL_BUTT,   WLAN_LEGACY_11G_MODE, WLAN_LEGACY_11G_MODE,
      WLAN_LEGACY_11G_MODE, WLAN_LEGACY_11G_MODE, WLAN_LEGACY_11G_MODE, WLAN_PROTOCOL_BUTT,
      WLAN_PROTOCOL_BUTT,   WLAN_LEGACY_11G_MODE, WLAN_LEGACY_11G_MODE },    // 11G
    { WLAN_PROTOCOL_BUTT,   WLAN_LEGACY_11B_MODE, WLAN_LEGACY_11G_MODE, WLAN_MIXED_ONE_11G_MODE,
      WLAN_MIXED_ONE_11G_MODE, WLAN_MIXED_ONE_11G_MODE, WLAN_MIXED_ONE_11G_MODE, WLAN_PROTOCOL_BUTT,
      WLAN_PROTOCOL_BUTT,   WLAN_PROTOCOL_BUTT,   WLAN_PROTOCOL_BUTT },    // ONE_11G
    { WLAN_PROTOCOL_BUTT,   WLAN_PROTOCOL_BUTT,   WLAN_LEGACY_11G_MODE, WLAN_MIXED_ONE_11G_MODE,
      WLAN_MIXED_TWO_11G_MODE, WLAN_MIXED_ONE_11G_MODE, WLAN_MIXED_ONE_11G_MODE, WLAN_PROTOCOL_BUTT,
      WLAN_PROTOCOL_BUTT,   WLAN_PROTOCOL_BUTT,   WLAN_PROTOCOL_BUTT },    // TWO_11G
    { WLAN_LEGACY_11A_MODE, WLAN_LEGACY_11B_MODE, WLAN_LEGACY_11G_MODE, WLAN_MIXED_ONE_11G_MODE,
      WLAN_MIXED_ONE_11G_MODE, WLAN_HT_MODE,      WLAN_HT_MODE,         WLAN_HT_ONLY_MODE,
      WLAN_PROTOCOL_BUTT,   WLAN_HT_11G_MODE,     WLAN_HT_MODE },    // HT
    { WLAN_LEGACY_11A_MODE, WLAN_LEGACY_11B_MODE, WLAN_LEGACY_11G_MODE, WLAN_MIXED_ONE_11G_MODE,
      WLAN_MIXED_ONE_11G_MODE, WLAN_HT_MODE,      WLAN_VHT_MODE,        WLAN_HT_ONLY_MODE,
      WLAN_HT_ONLY_MODE,    WLAN_PROTOCOL_BUTT,   WLAN_VHT_MODE },    // VHT
    { WLAN_PROTOCOL_BUTT,   WLAN_PROTOCOL_BUTT,   WLAN_PROTOCOL_BUTT,   WLAN_PROTOCOL_BUTT,
      WLAN_PROTOCOL_BUTT,   WLAN_HT_ONLY_MODE,    WLAN_HT_ONLY_MODE,    WLAN_HT_ONLY_MODE,
      WLAN_HT_ONLY_MODE,    WLAN_HT_ONLY_MODE,    WLAN_HT_ONLY_MODE },    // HT_ONLY
    { WLAN_PROTOCOL_BUTT,   WLAN_PROTOCOL_BUTT,   WLAN_PROTOCOL_BUTT,   WLAN_PROTOCOL_BUTT,
      WLAN_PROTOCOL_BUTT,   WLAN_PROTOCOL_BUTT,   WLAN_VHT_ONLY_MODE,   WLAN_PROTOCOL_BUTT,
      WLAN_VHT_ONLY_MODE,   WLAN_PROTOCOL_BUTT,   WLAN_PROTOCOL_BUTT },    // VHT_ONLY
    { WLAN_PROTOCOL_BUTT,   WLAN_PROTOCOL_BUTT,   WLAN_LEGACY_11G_MODE, WLAN_LEGACY_11G_MODE,
      WLAN_LEGACY_11G_MODE, WLAN_HT_11G_MODE,     WLAN_PROTOCOL_BUTT,   WLAN_HT_ONLY_MODE,
      WLAN_PROTOCOL_BUTT,   WLAN_HT_11G_MODE,     WLAN_PROTOCOL_BUTT },    // HT_11G
    { WLAN_LEGACY_11A_MODE, WLAN_LEGACY_11B_MODE, WLAN_LEGACY_11G_MODE, WLAN_MIXED_ONE_11G_MODE,
      WLAN_MIXED_ONE_11G_MODE, WLAN_HT_MODE,      WLAN_VHT_MODE,        WLAN_HT_ONLY_MODE,
      WLAN_VHT_ONLY_MODE,   WLAN_PROTOCOL_BUTT,   WLAN_HE_MODE },    /* he */
};
#else
uint8_t g_auc_avail_protocol_mode[WLAN_PROTOCOL_BUTT][WLAN_PROTOCOL_BUTT] = {
    { WLAN_LEGACY_11A_MODE, WLAN_PROTOCOL_BUTT,   WLAN_PROTOCOL_BUTT,   WLAN_PROTOCOL_BUTT,
      WLAN_PROTOCOL_BUTT,   WLAN_LEGACY_11A_MODE, WLAN_LEGACY_11A_MODE, WLAN_PROTOCOL_BUTT,
      WLAN_PROTOCOL_BUTT,   WLAN_PROTOCOL_BUTT },
    { WLAN_PROTOCOL_BUTT,   WLAN_LEGACY_11B_MODE, WLAN_LEGACY_11B_MODE, WLAN_LEGACY_11B_MODE,
      WLAN_LEGACY_11B_MODE, WLAN_LEGACY_11B_MODE, WLAN_LEGACY_11B_MODE, WLAN_PROTOCOL_BUTT,
      WLAN_PROTOCOL_BUTT,   WLAN_PROTOCOL_BUTT },
    { WLAN_PROTOCOL_BUTT,   WLAN_PROTOCOL_BUTT,   WLAN_LEGACY_11G_MODE, WLAN_LEGACY_11G_MODE,
      WLAN_LEGACY_11G_MODE, WLAN_LEGACY_11G_MODE, WLAN_LEGACY_11G_MODE, WLAN_PROTOCOL_BUTT,
      WLAN_PROTOCOL_BUTT,   WLAN_LEGACY_11G_MODE },
    { WLAN_PROTOCOL_BUTT,   WLAN_LEGACY_11B_MODE, WLAN_LEGACY_11G_MODE, WLAN_MIXED_ONE_11G_MODE,
      WLAN_MIXED_ONE_11G_MODE, WLAN_MIXED_ONE_11G_MODE, WLAN_MIXED_ONE_11G_MODE, WLAN_PROTOCOL_BUTT,
      WLAN_PROTOCOL_BUTT,   WLAN_LEGACY_11G_MODE },
    { WLAN_PROTOCOL_BUTT,   WLAN_PROTOCOL_BUTT,   WLAN_LEGACY_11G_MODE, WLAN_MIXED_ONE_11G_MODE,
      WLAN_MIXED_TWO_11G_MODE, WLAN_MIXED_ONE_11G_MODE, WLAN_MIXED_ONE_11G_MODE, WLAN_PROTOCOL_BUTT,
      WLAN_PROTOCOL_BUTT,   WLAN_LEGACY_11G_MODE },
    { WLAN_LEGACY_11A_MODE, WLAN_LEGACY_11B_MODE, WLAN_LEGACY_11G_MODE, WLAN_MIXED_ONE_11G_MODE,
      WLAN_MIXED_ONE_11G_MODE, WLAN_HT_MODE,      WLAN_HT_MODE,         WLAN_HT_ONLY_MODE,
      WLAN_PROTOCOL_BUTT,   WLAN_HT_11G_MODE },
    { WLAN_LEGACY_11A_MODE, WLAN_LEGACY_11B_MODE, WLAN_LEGACY_11G_MODE, WLAN_MIXED_ONE_11G_MODE,
      WLAN_MIXED_ONE_11G_MODE, WLAN_HT_MODE,      WLAN_VHT_MODE,        WLAN_HT_ONLY_MODE,
      WLAN_VHT_ONLY_MODE,   WLAN_PROTOCOL_BUTT },
    { WLAN_PROTOCOL_BUTT,   WLAN_PROTOCOL_BUTT,   WLAN_PROTOCOL_BUTT,   WLAN_PROTOCOL_BUTT,
      WLAN_PROTOCOL_BUTT,   WLAN_HT_ONLY_MODE,    WLAN_HT_ONLY_MODE,    WLAN_HT_ONLY_MODE,
      WLAN_HT_ONLY_MODE,    WLAN_HT_ONLY_MODE },
    { WLAN_PROTOCOL_BUTT,   WLAN_PROTOCOL_BUTT,   WLAN_PROTOCOL_BUTT,   WLAN_PROTOCOL_BUTT,
      WLAN_PROTOCOL_BUTT,   WLAN_PROTOCOL_BUTT,   WLAN_VHT_ONLY_MODE,   WLAN_PROTOCOL_BUTT,
      WLAN_VHT_ONLY_MODE,   WLAN_PROTOCOL_BUTT },
    { WLAN_PROTOCOL_BUTT,   WLAN_PROTOCOL_BUTT,   WLAN_LEGACY_11G_MODE, WLAN_LEGACY_11G_MODE,
      WLAN_LEGACY_11G_MODE, WLAN_HT_11G_MODE,     WLAN_PROTOCOL_BUTT,   WLAN_HT_ONLY_MODE,
      WLAN_PROTOCOL_BUTT,   WLAN_HT_11G_MODE },
};
#endif

/*
 * 日    期  : 2015年3月24日
 * 修改内容  : 新生成函数
 */
void hmac_rx_ba_session_decr(hmac_vap_stru *hmac_vap, uint8_t tidno)
{
    if (mac_mib_get_RxBASessionNumber(&hmac_vap->st_vap_base_info) == 0) {
        oam_warning_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_BA,
            "{hmac_rx_ba_session_decr::tid[%d] rx_session already zero.}", tidno);
        return;
    }

    mac_mib_decr_RxBASessionNumber(&hmac_vap->st_vap_base_info);

    oam_warning_log2(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_BA,
                     "{hmac_rx_ba_session_decr::tid[%d] tx_session[%d] remove.}",
                     tidno, mac_mib_get_RxBASessionNumber(&hmac_vap->st_vap_base_info));
}

/*
 * 1.日    期  : 2015年3月24日
  *   修改内容  : 新生成函数
 */
void hmac_tx_ba_session_decr(hmac_vap_stru *hmac_vap, uint8_t tidno)
{
    if (mac_mib_get_TxBASessionNumber(&hmac_vap->st_vap_base_info) == 0) {
        oam_warning_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_BA,
                         "{hmac_tx_ba_session_decr::tid[%d] tx_session already zero.}", tidno);
        return;
    }

    mac_mib_decr_TxBASessionNumber(&hmac_vap->st_vap_base_info);

    oam_warning_log2(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_BA,
                     "{hmac_tx_ba_session_decr::tid[%d] tx_session[%d] remove.}",
                     tidno, mac_mib_get_TxBASessionNumber(&hmac_vap->st_vap_base_info));
}

/*
 * 1.日    期  : 2013年4月9日
  *   修改内容  : 新生成函数
 */
uint16_t hmac_mgmt_encap_addba_req(hmac_vap_stru *hmac_vap, uint8_t *data, dmac_ba_tx_stru *tx_ba, uint8_t tid)
{
    uint16_t index;
    uint16_t ba_param;
    if (oal_any_null_ptr3(hmac_vap, data, tx_ba)) {
        oam_error_log0(0, OAM_SF_BA, "{hmac_mgmt_encap_addba_req::hmac_vap or data or tx_ba is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

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
    /* Frame Control Field 中只需要设置Type/Subtype值，其他设置为0 */
    mac_hdr_set_frame_control(data, WLAN_PROTOCOL_VERSION | WLAN_FC0_TYPE_MGT | WLAN_FC0_SUBTYPE_ACTION);

    /* DA is address of STA requesting association */
    oal_set_mac_addr(data + WLAN_HDR_ADDR1_OFFSET, tx_ba->puc_dst_addr);

    /* SA的值为dot11MACAddress的值 */
    oal_set_mac_addr(data + WLAN_HDR_ADDR2_OFFSET, mac_mib_get_StationID(&hmac_vap->st_vap_base_info));

    oal_set_mac_addr(data + WLAN_HDR_ADDR3_OFFSET, hmac_vap->st_vap_base_info.auc_bssid);

    /*************************************************************************/
    /*                Set the contents of the frame body                     */
    /*************************************************************************/
    /* 将索引指向frame body起始位置 */
    index = MAC_80211_FRAME_LEN;

    /* 设置Category */
    data[index++] = MAC_ACTION_CATEGORY_BA;

    /* 设置Action */
    data[index++] = MAC_BA_ACTION_ADDBA_REQ;

    /* 设置Dialog Token */
    data[index++] = tx_ba->uc_dialog_token;

    /*
        设置Block Ack Parameter set field
        bit0 - AMSDU Allowed
        bit1 - Immediate or Delayed block ack
        bit2-bit5 - TID
        bit6-bit15 -  Buffer size
    */
    ba_param = tx_ba->en_amsdu_supp;                   /* bit0 */
    ba_param |= (tx_ba->uc_ba_policy << 1);            /* bit1 */
    ba_param |= (tid << 2);                            /* bit2 */
    ba_param |= (uint16_t)(tx_ba->us_baw_size << 6);   /* bit6 */

    data[index++] = (uint8_t)(ba_param & 0xFF);
    data[index++] = (uint8_t)((ba_param >> 8) & 0xFF); /* bit8 */

    /* 设置BlockAck timeout */
    data[index++] = (uint8_t)(tx_ba->us_ba_timeout & 0xFF);
    data[index++] = (uint8_t)((tx_ba->us_ba_timeout >> 8) & 0xFF); /* bit8 */

    /*
        Block ack starting sequence number字段由硬件设置
        bit0-bit3 fragmentnumber
        bit4-bit15: sequence number
    */
    /* us_buf_seq此处暂不填写，在dmac侧会补充填写 */
    *(uint16_t *)&data[index++] = 0;
    index++;

    /* 返回的帧长度中不包括FCS */
    return index;
}

OAL_STATIC void hmac_mgmt_encap_addba_rsp_set_ba_param(hmac_vap_stru *pst_vap,
    hmac_user_stru *pst_hmac_user, hmac_ba_rx_stru *pst_addba_rsp, uint8_t uc_tid, uint16_t *pus_ba_param)
{
#ifdef _PRE_WLAN_FEATURE_11AX
    oal_bool_enum_uint8 is_he_implemented;

    // 降低圈复杂度
    is_he_implemented = (g_wlan_spec_cfg->feature_11ax_is_open &&
                            ((OAL_TRUE == mac_mib_get_HEOptionImplemented(&pst_vap->st_vap_base_info)) &&
                            (mac_user_is_he_user(&pst_hmac_user->st_user_base_info))));
    /* 标志vap工作在11ax */
    if (is_he_implemented) {
        *pus_ba_param |= (uint16_t)(pst_hmac_user->aus_tid_baw_size[uc_tid] << 6); /* BA param set的BIT6~BIT15 */
    } else {
        *pus_ba_param |= (uint16_t)(pst_addba_rsp->us_baw_size << 6); /* 最大聚合个数是BA param set的BIT6~BIT15 */
    }
#else
    *pus_ba_param |= (uint16_t)(pst_addba_rsp->us_baw_size << 6); /* 最大聚合个数是BA param set的BIT6~BIT15 */
#endif
    hmac_btcoex_set_addba_rsp_ba_param(pst_vap, pst_hmac_user, pst_addba_rsp, pus_ba_param);
}
/*
 * 1.日    期  : 2013年4月9日
  *   修改内容  : 新生成函数
 */
uint16_t hmac_mgmt_encap_addba_rsp(hmac_vap_stru *hmac_vap, uint8_t *data,
    hmac_ba_rx_stru *addba_rsp, uint8_t tid, uint8_t status)
{
    uint16_t index;
    uint16_t ba_param;
    hmac_user_stru *pst_hmac_user = NULL;
    if (oal_any_null_ptr3(hmac_vap, data, addba_rsp)) {
        oam_error_log0(0, OAM_SF_BA, "{hmac_mgmt_encap_addba_req::hmac_vap or data or addba_rsp is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
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
    mac_hdr_set_frame_control(data, WLAN_PROTOCOL_VERSION | WLAN_FC0_TYPE_MGT | WLAN_FC0_SUBTYPE_ACTION);
    /* DA is address of STA requesting association */
    oal_set_mac_addr(data + WLAN_HDR_ADDR1_OFFSET, addba_rsp->puc_transmit_addr);
    /* SA is the dot11MACAddress */
    oal_set_mac_addr(data + WLAN_HDR_ADDR2_OFFSET, mac_mib_get_StationID(&hmac_vap->st_vap_base_info));
    oal_set_mac_addr(data + WLAN_HDR_ADDR3_OFFSET, hmac_vap->st_vap_base_info.auc_bssid);
    /*************************************************************************/
    /*                Set the contents of the frame body                     */
    /*************************************************************************/
    /*************************************************************************/
    /*             ADDBA Response Frame - Frame Body                         */
    /*    ---------------------------------------------------------------    */
    /*    | Category | Action | Dialog | Status  | Parameters | Timeout |    */
    /*    ---------------------------------------------------------------    */
    /*    | 1        | 1      | 1      | 2       | 2          | 2       |    */
    /*    ---------------------------------------------------------------    */
    /*                                                                       */
    /*************************************************************************/
    /* Initialize index and the frame data pointer */
    index = MAC_80211_FRAME_LEN;
    /* Action Category设置 */
    data[index++] = MAC_ACTION_CATEGORY_BA;
    /* 特定Action种类下的action的帧类型 */
    data[index++] = MAC_BA_ACTION_ADDBA_RSP;
    /* Dialog Token域设置，需要从req中copy过来 */
    data[index++] = addba_rsp->uc_dialog_token;
    /* 状态域设置 */
    data[index++] = status;
    data[index++] = 0;
    /* Block Ack Parameter设置 */
    /* B0 - AMSDU Support, B1- Immediate or Delayed block ack */
    /* B2-B5 : TID, B6-B15: Buffer size */
    ba_param = addba_rsp->en_amsdu_supp;        /* BIT0 */
    ba_param |= (addba_rsp->uc_ba_policy << 1); /* BIT1 */
    ba_param |= (tid << 2);                     /* BIT2 */
    /* 手动设置聚合个数，屏蔽删建BA时不采用64聚合 */
    pst_hmac_user = mac_vap_get_hmac_user_by_addr(&(hmac_vap->st_vap_base_info), addba_rsp->puc_transmit_addr,
        WLAN_MAC_ADDR_LEN);
    if (pst_hmac_user != NULL) {
        hmac_mgmt_encap_addba_rsp_set_ba_param(hmac_vap, pst_hmac_user, addba_rsp, tid, &ba_param);
    } else {
        ba_param |= (uint16_t)(addba_rsp->us_baw_size << 6); /* BIT6 */
    }
    data[index++] = (uint8_t)(ba_param & 0xFF);
    data[index++] = (uint8_t)((ba_param >> 8) & 0xFF); /* BIT8 */
    /* BlockAck timeout值设置 解决timeout兼容性问题 */
    data[index++] = 0x00;
    data[index++] = 0x00;
    /* 返回的帧长度中不包括FCS */
    return index;
}

/*
 * 1.日    期  : 2013年4月9日
  *   修改内容  : 新生成函数
 */
uint16_t hmac_mgmt_encap_delba(hmac_vap_stru *pst_vap, uint8_t *puc_data, uint8_t *puc_addr, uint8_t uc_tid,
    mac_delba_initiator_enum_uint8 en_initiator, uint8_t reason)
{
    uint16_t us_index;

    if (oal_any_null_ptr3(pst_vap, puc_data, puc_addr)) {
        oam_error_log0(0, OAM_SF_BA, "{hmac_mgmt_encap_delba::vap or data or addr is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

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
    mac_hdr_set_frame_control(puc_data, WLAN_PROTOCOL_VERSION | WLAN_FC0_TYPE_MGT | WLAN_FC0_SUBTYPE_ACTION);

    /* duration */
    puc_data[BYTE_OFFSET_2] = 0;
    puc_data[BYTE_OFFSET_3] = 0;

    /* DA is address of STA requesting association */
    oal_set_mac_addr(puc_data + WLAN_HDR_ADDR1_OFFSET, puc_addr);

    /* SA is the dot11MACAddress */
    oal_set_mac_addr(puc_data + WLAN_HDR_ADDR2_OFFSET, mac_mib_get_StationID(&pst_vap->st_vap_base_info));

    oal_set_mac_addr(puc_data + WLAN_HDR_ADDR3_OFFSET, pst_vap->st_vap_base_info.auc_bssid);

    /* seq control */
    puc_data[BYTE_OFFSET_22] = 0;
    puc_data[BYTE_OFFSET_23] = 0;

    /*************************************************************************/
    /*                Set the contents of the frame body                     */
    /*************************************************************************/
    /*************************************************************************/
    /*             DELBA Response Frame - Frame Body                         */
    /*        -------------------------------------------------              */
    /*        | Category | Action |  Parameters | Reason code |              */
    /*        -------------------------------------------------              */
    /*        | 1        | 1      |       2     | 2           |              */
    /*        -------------------------------------------------              */
    /*                          Parameters                                   */
    /*                  -------------------------------                      */
    /*                  | Reserved | Initiator |  TID  |                     */
    /*                  -------------------------------                      */
    /*             bit  |    11    |    1      |  4    |                     */
    /*                  --------------------------------                     */
    /*************************************************************************/
    /* Initialize index and the frame data pointer */
    us_index = MAC_80211_FRAME_LEN;

    /* Category */
    puc_data[us_index++] = MAC_ACTION_CATEGORY_BA;

    /* Action */
    puc_data[us_index++] = MAC_BA_ACTION_DELBA;

    /* DELBA parameter set */
    /* B0 - B10 -reserved */
    /* B11 - initiator */
    /* B12-B15 - TID */
    puc_data[us_index++] = 0;
    puc_data[us_index] = (uint8_t)(en_initiator << 3); /* bit3 */
    puc_data[us_index++] |= (uint8_t)(uc_tid << 4);    /* bit4 */

    /* Reason field */
    /* Reason can be either of END_BA, QSTA_LEAVING, UNKNOWN_BA */
    puc_data[us_index++] = reason;
    puc_data[us_index++] = 0;

    /* 返回的帧长度中不包括FCS */
    return us_index;
}

OAL_STATIC uint32_t hmac_mgmt_tx_ba_check(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    mac_action_mgmt_args_stru *action_args)
{
    mac_vap_stru *mac_vap = NULL;
    mac_device_stru *mac_dev = NULL;

    if (oal_any_null_ptr3(hmac_vap, hmac_user, action_args)) {
        oam_error_log0(0, OAM_SF_BA, "{hmac_mgmt_tx_ba_check::hmac_vap or hmac_user or action_args is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    mac_vap = &(hmac_vap->st_vap_base_info);
    /*  确定vap处于工作状态 */
    if (mac_vap->en_vap_state == MAC_VAP_STATE_BUTT) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_BA,
            "{hmac_mgmt_tx_ba_check:: vap has been down/del, vap_state[%d].}", mac_vap->en_vap_state);
        return OAL_FAIL;
    }
    /*  END */
    /* 获取device结构 */
    mac_dev = mac_res_get_dev(mac_vap->uc_device_id);
    if (mac_dev == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_BA, "{hmac_mgmt_tx_ba_check::mac_dev null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    return OAL_SUCC;
}

static oal_netbuf_stru *hmac_mgmt_ba_session_alloc(uint8_t vap_id)
{
    oal_netbuf_stru *ba_session = NULL;

    ba_session = oal_mem_netbuf_alloc(OAL_NORMAL_NETBUF, WLAN_MEM_NETBUF_SIZE2, OAL_NETBUF_PRIORITY_MID);
    if (ba_session == NULL) {
        oam_error_log0(vap_id, OAM_SF_BA, "{hmac_mgmt_ba_session_alloc::ba_session null.}");
        return ba_session;
    }
    oal_mem_netbuf_trace(ba_session, OAL_TRUE);
    oal_netbuf_prev(ba_session) = NULL;
    oal_netbuf_next(ba_session) = NULL;
    return ba_session;
}

static void hmac_mgmt_set_tx_ba_para(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, mac_vap_stru *mac_vap,
    mac_action_mgmt_args_stru *action_args, dmac_ba_tx_stru *tx_ba)
{
    uint8_t tidno = (uint8_t)(action_args->arg1);

    hmac_vap->uc_ba_dialog_token++;
    tx_ba->uc_dialog_token = hmac_vap->uc_ba_dialog_token;
    /* 非802.11ax协议下ADDBA窗口大小最大不能超过64 */
    if (hmac_user->st_user_base_info.en_avail_protocol_mode == WLAN_HE_MODE) {
        tx_ba->us_baw_size = (uint16_t)(action_args->arg2);
    } else {
        tx_ba->us_baw_size = oal_min((uint16_t)(action_args->arg2), WLAN_AMPDU_TX_MAX_NUM);
    }
    tx_ba->uc_ba_policy = (uint8_t)(action_args->arg3);
    tx_ba->us_ba_timeout = (uint16_t)(action_args->arg4);
    tx_ba->puc_dst_addr = hmac_user->st_user_base_info.auc_user_mac_addr;

    /* 发端对AMPDU+AMSDU的支持 */
    tx_ba->en_amsdu_supp = (oal_bool_enum_uint8)mac_mib_get_AmsduPlusAmpduActive(mac_vap);
#ifdef _PRE_WLAN_FEATURE_PWL
    /* pwl模式不支持amsdu */
    if (mac_vap->pwl_enable == OAL_TRUE) {
        tx_ba->en_amsdu_supp = OAL_FALSE;
    }
#endif

    if (tx_ba->en_amsdu_supp == OAL_FALSE) {
        hmac_user_set_amsdu_not_support(hmac_user, tidno);
    } else {
        hmac_user_set_amsdu_support(hmac_user, tidno);
    }
    return;
}

static void hmac_mgmt_tx_set_ctx(uint16_t frame_len, uint8_t tidno, uint16_t assoc_id, dmac_ba_tx_stru st_tx_ba,
    dmac_ctx_action_event_stru *wlan_ctx_action)
{
    wlan_ctx_action->us_frame_len = frame_len;
    wlan_ctx_action->uc_hdr_len = MAC_80211_FRAME_LEN;
    wlan_ctx_action->en_action_category = MAC_ACTION_CATEGORY_BA;
    wlan_ctx_action->uc_action = MAC_BA_ACTION_ADDBA_REQ;
    wlan_ctx_action->us_user_idx = assoc_id;
    wlan_ctx_action->uc_tidno = tidno;
    wlan_ctx_action->uc_dialog_token = st_tx_ba.uc_dialog_token;
    wlan_ctx_action->uc_ba_policy = st_tx_ba.uc_ba_policy;
    wlan_ctx_action->us_baw_size = st_tx_ba.us_baw_size;
    wlan_ctx_action->us_ba_timeout = st_tx_ba.us_ba_timeout;
    wlan_ctx_action->en_amsdu_supp = st_tx_ba.en_amsdu_supp;
    return;
}

static void hmac_mgmt_tx_addba_req_cb_init(oal_netbuf_stru *addba_req, uint16_t frame_len,
    uint8_t frame_type, uint8_t frame_subtype)
{
    mac_tx_ctl_stru *tx_ctl = NULL;

    memset_s(oal_netbuf_cb(addba_req), oal_netbuf_cb_size(), 0, oal_netbuf_cb_size());
    tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(addba_req);
    mac_get_cb_mpdu_len(tx_ctl) = frame_len + sizeof(dmac_ctx_action_event_stru);
    mac_get_cb_frame_type(tx_ctl) = frame_type;
    mac_get_cb_frame_subtype(tx_ctl) = frame_subtype;
}

static uint32_t hmac_tx_action_event(mac_vap_stru *mac_vap, oal_netbuf_stru *netbuf, uint16_t frame_len)
{
    frw_event_mem_stru *event_mem = NULL; /* 申请事件返回的内存指针 */
    frw_event_stru *event = NULL;
    dmac_tx_event_stru *tx_event = NULL;
    uint32_t ret;

    event_mem = frw_event_alloc_m(sizeof(dmac_tx_event_stru));
    if (event_mem == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_BA, "{hmac_tx_action_event::event_mem alloc failed}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    event = frw_get_event_stru(event_mem);
    frw_event_hdr_init(&(event->st_event_hdr), FRW_EVENT_TYPE_WLAN_CTX, DMAC_WLAN_CTX_EVENT_SUB_TYPE_MGMT,
        sizeof(dmac_tx_event_stru), FRW_EVENT_PIPELINE_STAGE_1, mac_vap->uc_chip_id, mac_vap->uc_device_id,
        mac_vap->uc_vap_id);

    /* 填写事件payload */
    tx_event = (dmac_tx_event_stru *)(event->auc_event_data);
    tx_event->pst_netbuf = netbuf;
    tx_event->us_frame_len = frame_len + sizeof(dmac_ctx_action_event_stru);
    ret = frw_event_dispatch_event(event_mem);
    if (ret != OAL_SUCC) {
        oam_error_log1(mac_vap->uc_vap_id, OAM_SF_BA, "{hmac_tx_action_event::frw_event_dispatch failed[%d].}", ret);
    }
    frw_event_free_m(event_mem);
    return ret;
}

/*
 * 1.日    期  : 2013年4月14日
  *   修改内容  : 新生成函数
 */
uint32_t hmac_mgmt_tx_addba_req(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    mac_action_mgmt_args_stru *action_args)
{
    mac_vap_stru *mac_vap = NULL;
    oal_netbuf_stru *addba_req = NULL;
    dmac_ba_tx_stru st_tx_ba = {0};
    uint8_t tidno;
    uint16_t frame_len;
    dmac_ctx_action_event_stru st_wlan_ctx_action;
    uint32_t ret;

    ret = hmac_mgmt_tx_ba_check(hmac_vap, hmac_user, action_args);
    if (ret != OAL_SUCC) {
        return ret;
    }
    mac_vap = &(hmac_vap->st_vap_base_info);

    /* 申请ADDBA_REQ管理帧内存 */
    addba_req = hmac_mgmt_ba_session_alloc(mac_vap->uc_vap_id);
    if (addba_req == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    tidno = (uint8_t)(action_args->arg1);
    /* 对tid对应的txBA会话状态加锁 */
    oal_spin_lock_bh(&(hmac_user->ast_tid_info[tidno].st_ba_tx_info.st_ba_status_lock));

    hmac_mgmt_set_tx_ba_para(hmac_vap, hmac_user, mac_vap, action_args, &st_tx_ba);

    /* 调用封装管理帧接口 */
    frame_len = hmac_mgmt_encap_addba_req(hmac_vap, oal_netbuf_data(addba_req), &st_tx_ba, tidno);
    memset_s((uint8_t *)&st_wlan_ctx_action, sizeof(st_wlan_ctx_action), 0, sizeof(st_wlan_ctx_action));
    /* 赋值要传入Dmac的信息 */
    hmac_mgmt_tx_set_ctx(frame_len, tidno, hmac_user->st_user_base_info.us_assoc_id, st_tx_ba, &st_wlan_ctx_action);

    if (memcpy_s((uint8_t *)(oal_netbuf_data(addba_req) + frame_len), (WLAN_MEM_NETBUF_SIZE2 - frame_len),
        (uint8_t *)&st_wlan_ctx_action, sizeof(dmac_ctx_action_event_stru)) != EOK) {
        oam_error_log0(0, OAM_SF_BA, "hmac_mgmt_tx_addba_req::memcpy fail!");
        oal_netbuf_free(addba_req);
        oal_spin_unlock_bh(&(hmac_user->ast_tid_info[tidno].st_ba_tx_info.st_ba_status_lock));
        return OAL_FAIL;
    }
    oal_netbuf_put(addba_req, (frame_len + sizeof(dmac_ctx_action_event_stru)));

    hmac_mgmt_tx_addba_req_cb_init(addba_req, frame_len, WLAN_CB_FRAME_TYPE_ACTION, WLAN_ACTION_BA_ADDBA_REQ);
    ret = hmac_tx_action_event(mac_vap, addba_req, frame_len);
    if (ret != OAL_SUCC) {
        oal_netbuf_free(addba_req);
        oal_spin_unlock_bh(&(hmac_user->ast_tid_info[tidno].st_ba_tx_info.st_ba_status_lock));
        return ret;
    }
    /* 更新对应的TID信息 */
    hmac_user->ast_tid_info[tidno].st_ba_tx_info.en_ba_status = DMAC_BA_INPROGRESS;
    hmac_user->ast_tid_info[tidno].st_ba_tx_info.uc_dialog_token = st_tx_ba.uc_dialog_token;
    hmac_user->ast_tid_info[tidno].st_ba_tx_info.uc_ba_policy = st_tx_ba.uc_ba_policy;
    mac_mib_incr_TxBASessionNumber(mac_vap);
    /* 启动ADDBA超时计时器 */
    frw_timer_create_timer_m(&hmac_user->ast_tid_info[tidno].st_ba_tx_info.st_addba_timer,
        hmac_mgmt_tx_addba_timeout, WLAN_ADDBA_TIMEOUT, &hmac_user->ast_tid_info[tidno].st_ba_tx_info.st_alarm_data,
        OAL_FALSE, OAM_MODULE_ID_HMAC, hmac_vap->st_vap_base_info.core_id);
    /* 对tid对应的tx BA会话状态解锁 */
    oal_spin_unlock_bh(&(hmac_user->ast_tid_info[tidno].st_ba_tx_info.st_ba_status_lock));
    return OAL_SUCC;
}

static uint32_t hmac_mgmt_tx_addba_rsp_check(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    hmac_ba_rx_stru *ba_rx_info)
{
    mac_vap_stru *mac_vap = NULL;
    mac_device_stru *mac_dev = NULL;

    if (oal_any_null_ptr3(hmac_vap, hmac_user, ba_rx_info)) {
        oam_error_log0(0, OAM_SF_BA, "{hmac_mgmt_tx_addba_rsp::hmac_vap or hmac_user or ba_rx_info is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    mac_vap = &(hmac_vap->st_vap_base_info);
    /*  确定vap处于工作状态 */
    if (mac_vap->en_vap_state == MAC_VAP_STATE_BUTT) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_BA,
            "{hmac_mgmt_tx_addba_rsp:: vap has been down/del, vap_state[%d].}", mac_vap->en_vap_state);
        return OAL_FAIL;
    }
    /*  END */
    /* 获取device结构 */
    mac_dev = mac_res_get_dev(mac_vap->uc_device_id);
    if (mac_dev == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_BA,
                       "{hmac_mgmt_tx_addba_rsp::send addba rsp failed, mac_dev null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    return OAL_SUCC;
}


static void hmac_mgmt_tx_addba_rsp_fill_cb(mac_tx_ctl_stru *tx_ctl, hmac_user_stru *hmac_user,
    uint8_t tid, uint16_t frame_len)
{
    mac_get_cb_tx_user_idx(tx_ctl) = hmac_user->st_user_base_info.us_assoc_id;
    mac_get_cb_wme_tid_type(tx_ctl) = tid;
    mac_get_cb_mpdu_len(tx_ctl) = frame_len + sizeof(dmac_ctx_action_event_stru);
    mac_get_cb_frame_type(tx_ctl) = WLAN_CB_FRAME_TYPE_ACTION;
    mac_get_cb_frame_subtype(tx_ctl) = WLAN_ACTION_BA_ADDBA_RSP;
}
/*
 * 1.日    期  : 2014年11月29日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_mgmt_tx_addba_rsp(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, hmac_ba_rx_stru *ba_rx_info,
    uint8_t tid, uint8_t status)
{
    mac_vap_stru *mac_vap = NULL;
    dmac_ctx_action_event_stru wlan_ctx_action;
    oal_netbuf_stru *addba_rsp = NULL;
    uint16_t frame_len;
    uint32_t ret;
    mac_tx_ctl_stru *tx_ctl = NULL;

    ret = hmac_mgmt_tx_addba_rsp_check(hmac_vap, hmac_user, ba_rx_info);
    if (ret != OAL_SUCC) {
        return ret;
    }
    mac_vap = &(hmac_vap->st_vap_base_info);
    /* 申请ADDBA_RSP管理帧内存 */
    addba_rsp = hmac_mgmt_ba_session_alloc(mac_vap->uc_vap_id);
    if (addba_rsp == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    frame_len = hmac_mgmt_encap_addba_rsp(hmac_vap, oal_netbuf_data(addba_rsp), ba_rx_info, tid, status);

    memset_s((uint8_t *)&wlan_ctx_action, sizeof(wlan_ctx_action), 0, sizeof(wlan_ctx_action));
    wlan_ctx_action.en_action_category = MAC_ACTION_CATEGORY_BA;
    wlan_ctx_action.uc_action = MAC_BA_ACTION_ADDBA_RSP;
    wlan_ctx_action.uc_hdr_len = MAC_80211_FRAME_LEN;
    wlan_ctx_action.us_baw_size = hmac_user->aus_tid_baw_size[tid];
    wlan_ctx_action.us_frame_len = frame_len;
    wlan_ctx_action.us_user_idx = hmac_user->st_user_base_info.us_assoc_id;
    wlan_ctx_action.uc_tidno = tid;
    wlan_ctx_action.uc_status = status;
    wlan_ctx_action.us_ba_timeout = ba_rx_info->us_ba_timeout;
    wlan_ctx_action.en_back_var = ba_rx_info->en_back_var;
    wlan_ctx_action.uc_lut_index = ba_rx_info->uc_lut_index;
    wlan_ctx_action.us_baw_start = ba_rx_info->us_baw_start;
    wlan_ctx_action.uc_ba_policy = ba_rx_info->uc_ba_policy;

    if (memcpy_s((uint8_t *)(oal_netbuf_data(addba_rsp) + frame_len), (WLAN_MEM_NETBUF_SIZE2 - frame_len),
        (uint8_t *)&wlan_ctx_action, sizeof(dmac_ctx_action_event_stru)) != EOK) {
        oam_warning_log0(0, 0, "{hmac_mgmt_tx_addba_rsp::memcpy fail}");
    }

    oal_netbuf_put(addba_rsp, (frame_len + sizeof(dmac_ctx_action_event_stru)));

    /* 填写netbuf的cb字段，供发送管理帧和发送完成接口使用 */
    memset_s(oal_netbuf_cb(addba_rsp), oal_netbuf_cb_size(), 0, oal_netbuf_cb_size());
    tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(addba_rsp);
    hmac_mgmt_tx_addba_rsp_fill_cb(tx_ctl, hmac_user, tid, frame_len);

    ret = hmac_tx_action_event(mac_vap, addba_rsp, frame_len);
    if (ret != OAL_SUCC) {
        oal_netbuf_free(addba_rsp);
    }
    ba_rx_info->en_ba_status = DMAC_BA_COMPLETE;
    return ret;
}

OAL_STATIC void hmac_set_delba_netbuf_cb(oal_netbuf_stru *delba_netbuf, uint16_t frame_len)
{
    mac_tx_ctl_stru *tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(delba_netbuf);

    memset_s(tx_ctl, oal_netbuf_cb_size(), 0, oal_netbuf_cb_size());
    mac_get_cb_mpdu_len(tx_ctl) = frame_len + sizeof(dmac_ctx_action_event_stru);
    mac_get_cb_frame_type(tx_ctl) = WLAN_CB_FRAME_TYPE_ACTION;
    mac_get_cb_frame_subtype(tx_ctl) = WLAN_ACTION_BA_DELBA;
    mac_get_cb_frame_header_length(tx_ctl) = MAC_80211_FRAME_LEN;
}

OAL_STATIC uint32_t hmac_set_delba_ctx_action (
    hmac_user_stru *hmac_user, mac_action_mgmt_args_stru *action_args,
    oal_netbuf_stru *delba_netbuf, uint16_t frame_len)
{
    dmac_ctx_action_event_stru wlan_ctx_action = {0};

    wlan_ctx_action.us_frame_len = frame_len;
    wlan_ctx_action.uc_hdr_len = MAC_80211_FRAME_LEN;
    wlan_ctx_action.en_action_category = MAC_ACTION_CATEGORY_BA;
    wlan_ctx_action.uc_action = MAC_BA_ACTION_DELBA;
    wlan_ctx_action.us_user_idx = hmac_user->st_user_base_info.us_assoc_id;
    wlan_ctx_action.uc_tidno = (uint8_t)(action_args->arg1);
    wlan_ctx_action.uc_initiator = (mac_delba_initiator_enum_uint8)(action_args->arg2);
    if (memcpy_s((uint8_t *)(oal_netbuf_data(delba_netbuf) + frame_len), (WLAN_MEM_NETBUF_SIZE2 - frame_len),
        (uint8_t *)&wlan_ctx_action, sizeof(dmac_ctx_action_event_stru)) != EOK) {
        oam_error_log0(0, OAM_SF_BA, "hmac_set_delba_ctx_action::memcpy fail!");
        /* 释放管理帧内存到netbuf内存池 */
        /* 对tid对应的tx BA会话状态解锁 */
        return OAL_FAIL;
    }
    return OAL_SUCC;
}

OAL_STATIC uint32_t hmac_mgmt_send_delba_netbuf(
    hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, mac_action_mgmt_args_stru *action_args)
{
    oal_netbuf_stru *delba_netbuf = NULL;
    uint16_t frame_len;
    uint8_t tidno = (uint8_t)(action_args->arg1);

    /* 申请DEL_BA管理帧内存 */
    delba_netbuf = oal_mem_netbuf_alloc(OAL_NORMAL_NETBUF, WLAN_MEM_NETBUF_SIZE2, OAL_NETBUF_PRIORITY_MID);
    if (delba_netbuf == NULL) {
        oam_error_log0(0, OAM_SF_BA, "{hmac_mgmt_send_delba_netbuf::delba_netbuf null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    oal_mem_netbuf_trace(delba_netbuf, OAL_TRUE);
    oal_netbuf_prev(delba_netbuf) = NULL;
    oal_netbuf_next(delba_netbuf) = NULL;
    /* 调用封装管理帧接口 */
    frame_len = hmac_mgmt_encap_delba(hmac_vap, (uint8_t *)oal_netbuf_header(delba_netbuf), action_args->puc_arg5,
        tidno, (mac_delba_initiator_enum_uint8)(action_args->arg2), (uint8_t)action_args->arg3);
    if (hmac_set_delba_ctx_action(hmac_user, action_args, delba_netbuf, frame_len) != OAL_SUCC) {
        oal_netbuf_free(delba_netbuf);
        return OAL_FAIL;
    }
    oal_netbuf_put(delba_netbuf, (frame_len + sizeof(dmac_ctx_action_event_stru)));
    hmac_chr_set_del_ba_info(tidno, (uint16_t)action_args->arg3);
    hmac_set_delba_netbuf_cb(delba_netbuf, frame_len);
    if (hmac_tx_action_event(&hmac_vap->st_vap_base_info, delba_netbuf, frame_len) != OAL_SUCC) {
        oal_netbuf_free(delba_netbuf);
        return OAL_FAIL;
    }
    return OAL_SUCC;
}

/*
 * 1.日    期  : 2013年4月14日
  *   修改内容  : 新生成函数
 */
uint32_t hmac_mgmt_tx_delba(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, mac_action_mgmt_args_stru *action_args)
{
    mac_device_stru *mac_device = NULL;
    mac_vap_stru *mac_vap = NULL;
    mac_delba_initiator_enum_uint8 initiator;
    uint8_t tidno;

    if (oal_any_null_ptr3(hmac_vap, hmac_user, action_args)) {
        oam_error_log0(0, OAM_SF_BA, "{hmac_mgmt_tx_delba::hmac_vap or hmac_user or action_args is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    mac_vap = &(hmac_vap->st_vap_base_info);
    /*  需确定vap处于工作状态 */
    if (mac_vap->en_vap_state == MAC_VAP_STATE_BUTT) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_BA, "{hmac_mgmt_tx_delba:vap_state[%d].}", mac_vap->en_vap_state);
        return OAL_FAIL;
    }
    /*  END */
    mac_device = mac_res_get_dev(mac_vap->uc_device_id);
    if (mac_device == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_BA, "{hmac_mgmt_tx_delba::mac_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    initiator = (mac_delba_initiator_enum_uint8)(action_args->arg2);
    tidno = (uint8_t)(action_args->arg1);
    /* 对tid对应的tx BA会话状态加锁 */
    oal_spin_lock_bh(&(hmac_user->ast_tid_info[tidno].st_ba_tx_info.st_ba_status_lock));
    /*  接收方向判断接收方向status，发送方向判断发送方向status */
    /*
     * ,接收方向delba发送失败，或者对端没有收到，要再次发送delba时需要放宽判断,
     * 这里主要保证rx delba始终可以发送即可，无其他影响
     */
    if (initiator == MAC_ORIGINATOR_DELBA) {
        if (hmac_user->ast_tid_info[tidno].st_ba_tx_info.en_ba_status == DMAC_BA_INIT) {
            oal_spin_unlock_bh(&(hmac_user->ast_tid_info[tidno].st_ba_tx_info.st_ba_status_lock));
            return OAL_SUCC;
        }
    }
    if (hmac_mgmt_send_delba_netbuf(hmac_vap, hmac_user, action_args) != OAL_SUCC) {
        oal_spin_unlock_bh(&(hmac_user->ast_tid_info[tidno].st_ba_tx_info.st_ba_status_lock));
        return OAL_FAIL;
    }
    if (initiator == MAC_RECIPIENT_DELBA) {
        /* 更新对应的TID信息 */
        hmac_ba_reset_rx_handle(mac_device, hmac_user, tidno, OAL_FALSE);
    } else {
        /* 更新对应的TID信息 */
        hmac_user->ast_tid_info[tidno].st_ba_tx_info.en_ba_status = DMAC_BA_INIT;
        hmac_tx_ba_session_decr(hmac_vap, hmac_user->ast_tid_info[tidno].uc_tid_no);
        /* 还原设置AMPDU下AMSDU的支持情况 */
        hmac_user_set_amsdu_support(hmac_user, tidno);
    }
    /* 对tid对应的tx BA会话状态解锁 */
    oal_spin_unlock_bh(&(hmac_user->ast_tid_info[tidno].st_ba_tx_info.st_ba_status_lock));
    return OAL_SUCC;
}

/*
 * 1.日    期  : 2013年4月14日
  *   修改内容  : 新生成函数
 */
uint32_t hmac_mgmt_rx_addba_req(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, uint8_t *payload,
    uint32_t frame_body_len)
{
    mac_device_stru *dev = NULL;
    mac_vap_stru *mac_vap = NULL;
    uint8_t tid;
    uint8_t status;
    hmac_ba_rx_stru *ba_rx_info = NULL;
    uint32_t ret;

    if (oal_any_null_ptr3(hmac_vap, hmac_user, payload)) {
        oam_error_log0(0, OAM_SF_BA, "{hmac_mgmt_rx_addba_req::hmac_vap or hmac_user or payload is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    if (frame_body_len < MAC_ADDBA_REQ_FRAME_BODY_LEN) {
        oam_warning_log1(0, OAM_SF_BA, "{frame_body_len[%d] < MAC_ADDBA_REQ_FRAME_BODY_LEN.}", frame_body_len);
        return OAL_FAIL;
    }
    mac_vap = &(hmac_vap->st_vap_base_info);

    /* 11n以上能力才可接收ampdu */
    if ((!(mac_vap->en_protocol >= WLAN_HT_MODE)) ||
        (!(hmac_user->st_user_base_info.en_protocol_mode >= WLAN_HT_MODE))) {
        oam_warning_log2(mac_vap->uc_vap_id, OAM_SF_BA,
            "{hmac_mgmt_rx_addba_req::ampdu not supprot or not open,vap protocol mode:%d,user protocol mode:%d.}",
            mac_vap->en_protocol, hmac_user->st_user_base_info.en_protocol_mode);
        return OAL_SUCC;
    }

    /* 获取device结构 */
    dev = mac_res_get_dev(mac_vap->uc_device_id);
    if (dev == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_BA,
            "{hmac_mgmt_rx_addba_req::addba req receive failed, device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /******************************************************************/
    /*       ADDBA Request Frame - Frame Body                         */
    /* --------------------------------------------------------------- */
    /* | Category | Action | Dialog | Parameters | Timeout | SSN     | */
    /* --------------------------------------------------------------- */
    /* | 1        | 1      | 1      | 2          | 2       | 2       | */
    /* --------------------------------------------------------------- */
    /*                                                                 */
    /******************************************************************/
    tid = (payload[BYTE_OFFSET_3] & 0x3C) >> 2; /* bit2 */
    if (tid >= WLAN_TID_MAX_NUM) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_BA,
                         "{hmac_mgmt_rx_addba_req::addba req receive failed, tid %d overflow.}", tid);
        return OAL_ERR_CODE_ARRAY_OVERFLOW;
    }

    if (hmac_user->ast_tid_info[tid].pst_ba_rx_info != NULL) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_BA,
                         "{hmac_mgmt_rx_addba_req::addba req received, but tid [%d] already set up.}", tid);
        /*  addba rsp发送失败, 再次收到addba req重新建立BA */
        hmac_ba_reset_rx_handle(dev, hmac_user, tid, OAL_FALSE);
    }

    /* ADDBA REQ baw size来区分重排序队列是64还是256 */
    ba_rx_info = (hmac_ba_rx_stru *)oal_mem_alloc_m(OAL_MEM_POOL_ID_LOCAL,
                                                    (uint16_t)sizeof(hmac_ba_rx_stru), OAL_TRUE);
    if (ba_rx_info == NULL) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_BA, "{hmac_mgmt_rx_addba_req::alloc fail. tid[%d], baw[%d].}", tid);
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_user->ast_tid_info[tid].pst_ba_rx_info = ba_rx_info;

    /* Ba会话参数初始化 */
    ba_rx_info->en_ba_status = DMAC_BA_INIT;
    ba_rx_info->uc_dialog_token = payload[BYTE_OFFSET_2];
    ba_rx_info->uc_ba_policy = (payload[BYTE_OFFSET_3] & 0x02) >> 1;
    ba_rx_info->us_ba_timeout = payload[BYTE_OFFSET_5] | (payload[BYTE_OFFSET_6] << 8); /* bit8 */
    /* 初始化接收窗口 */
    ba_rx_info->us_baw_start = (payload[BYTE_OFFSET_7] >> 4) | (payload[BYTE_OFFSET_8] << 4); /* bit4 */
    ba_rx_info->us_baw_size = (payload[BYTE_OFFSET_3] & 0xC0) >> 6; /* bit6 */
    ba_rx_info->us_baw_size |= (payload[BYTE_OFFSET_4] << 2); /* bit2 */

    wlan_chip_ba_rx_hdl_init(hmac_vap, hmac_user, tid);

    /* profiling测试中，接收端不删除ba */
    frw_timer_create_timer_m(&(hmac_user->ast_tid_info[tid].st_ba_timer), hmac_ba_timeout_fn,
        hmac_vap->us_rx_timeout[WLAN_WME_TID_TO_AC(tid)],
        &(ba_rx_info->st_alarm_data), OAL_FALSE, OAM_MODULE_ID_HMAC, dev->core_id);

    mac_mib_incr_RxBASessionNumber(mac_vap);

    /* 判断建立能否成功 */
    status = hmac_mgmt_check_set_rx_ba_ok(hmac_vap, hmac_user, ba_rx_info, dev, &(hmac_user->ast_tid_info[tid]));
    if (status == MAC_SUCCESSFUL_STATUSCODE) {
        hmac_user->ast_tid_info[tid].pst_ba_rx_info->en_ba_status = DMAC_BA_INPROGRESS;
    }

    ret = hmac_mgmt_tx_addba_rsp(hmac_vap, hmac_user, ba_rx_info, tid, status);
    oam_warning_log4(mac_vap->uc_vap_id, OAM_SF_BA,
        "{hmac_mgmt_rx_addba_req::process addba req receive and send addba rsp, tid[%d], \
        status[%d], baw_size[%d], RXAMSDU[%d].}\r\n",
        tid, status, ba_rx_info->us_baw_size, ba_rx_info->en_amsdu_supp);
    if ((status != MAC_SUCCESSFUL_STATUSCODE) || (ret != 0)) {
        /*  Mpxx bug fix  /00185449 2015/05/08 */
        /* hmac_user->ast_tid_info[tid].pst_ba_rx_info修改为在函数中置空，与其他调用一致 */
        hmac_ba_reset_rx_handle(dev, hmac_user, tid, OAL_FALSE);
    }

    return OAL_SUCC;
}
/*
 * 1.日    期  : 2013年4月22日
  *   修改内容  : 新生成函数
 */
uint32_t hmac_mgmt_tx_addba_timeout(void *arg)
{
    hmac_vap_stru *vap = NULL; /* vap指针 */
    uint8_t *usr_dst_mac = NULL; /* 保存用户目的地址的指针 */
    hmac_user_stru *hmac_user = NULL;
    mac_action_mgmt_args_stru action_args;
    dmac_ba_alarm_stru *alarm_data = NULL;
    uint32_t ret;

    if (arg == NULL) {
        oam_error_log0(0, OAM_SF_BA, "{hmac_mgmt_tx_addba_timeout::p_arg null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    alarm_data = (dmac_ba_alarm_stru *)arg;
    hmac_user = (hmac_user_stru *)mac_res_get_hmac_user(alarm_data->us_mac_user_idx);
    if (hmac_user == NULL) {
        oam_warning_log1(0, OAM_SF_BA, "{hmac_mgmt_tx_addba_timeout::pst_hmac_user[%d] null.}",
                         alarm_data->us_mac_user_idx);
        return OAL_ERR_CODE_PTR_NULL;
    }

    usr_dst_mac = hmac_user->st_user_base_info.auc_user_mac_addr;
    vap = (hmac_vap_stru *)mac_res_get_hmac_vap(alarm_data->uc_vap_id);
    if (oal_unlikely(vap == NULL)) {
        oam_error_log0(hmac_user->st_user_base_info.uc_vap_id, OAM_SF_BA,
                       "{hmac_mgmt_tx_addba_timeout::pst_hmac_user null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 生成DELBA帧 */
    action_args.uc_category = MAC_ACTION_CATEGORY_BA;
    action_args.uc_action = MAC_BA_ACTION_DELBA;
    action_args.arg1 = alarm_data->uc_tid; /* 该数据帧对应的TID号 */
    action_args.arg2 = MAC_ORIGINATOR_DELBA; /* DELBA中，触发删除BA会话的发起端 */
    action_args.arg3 = MAC_QSTA_TIMEOUT; /* DELBA中代表删除reason */
    action_args.puc_arg5 = usr_dst_mac; /* DELBA中代表目的地址 */

    oam_warning_log1(hmac_user->st_user_base_info.uc_vap_id, OAM_SF_BA,
        "{hmac_mgmt_tx_addba_timeout::TID[%d] add timeout.}", alarm_data->uc_tid);
    ret = hmac_mgmt_tx_delba(vap, hmac_user, &action_args);

    return ret;
}

/*
 * 1.日    期  : 2013年5月29日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_tx_mgmt_send_event(mac_vap_stru *pst_vap, oal_netbuf_stru *pst_mgmt_frame, uint16_t us_frame_len)
{
    frw_event_mem_stru *pst_event_mem = NULL;
    frw_event_stru *pst_event = NULL;
    uint32_t ret;
    dmac_tx_event_stru *pst_ctx_stru = NULL;

    if (pst_vap == NULL || pst_mgmt_frame == NULL) {
        oam_error_log0(0, OAM_SF_SCAN, "{hmac_tx_mgmt_send_event::vap or mgmt_frame is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 抛事件给DMAC,让DMAC完成配置VAP创建 */
    pst_event_mem = frw_event_alloc_m(sizeof(dmac_tx_event_stru));
    if (oal_unlikely(pst_event_mem == NULL)) {
        oam_error_log0(pst_vap->uc_vap_id, OAM_SF_SCAN, "{hmac_tx_mgmt_send_event::pst_event_mem null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_event = frw_get_event_stru(pst_event_mem);

    /* 填写事件头 */
    frw_event_hdr_init(&(pst_event->st_event_hdr), FRW_EVENT_TYPE_WLAN_CTX, DMAC_WLAN_CTX_EVENT_SUB_TYPE_MGMT,
        sizeof(dmac_tx_event_stru), FRW_EVENT_PIPELINE_STAGE_1, pst_vap->uc_chip_id, pst_vap->uc_device_id,
        pst_vap->uc_vap_id);

    pst_ctx_stru = (dmac_tx_event_stru *)pst_event->auc_event_data;
    pst_ctx_stru->pst_netbuf = pst_mgmt_frame;
    pst_ctx_stru->us_frame_len = us_frame_len;
#ifdef _PRE_WLAN_FEATURE_SNIFFER
#ifdef CONFIG_HW_SNIFFER
    proc_sniffer_write_file(NULL, 0, (uint8_t *)(oal_netbuf_header(pst_mgmt_frame)), us_frame_len, 1);
#endif
#endif

    ret = frw_event_dispatch_event(pst_event_mem);
    if (ret != OAL_SUCC) {
        oam_error_log1(pst_vap->uc_vap_id, OAM_SF_SCAN,
                       "{hmac_tx_mgmt_send_event::frw_event_dispatch_event failed[%d].}", ret);
        frw_event_free_m(pst_event_mem);
        return ret;
    }

    /* 释放事件 */
    frw_event_free_m(pst_event_mem);

    return OAL_SUCC;
}
static oal_bool_enum_uint8 hmac_send_deauth_filter(mac_vap_stru *mac_vap, const unsigned char *da_addr)
{
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    hmac_user_stru *hmac_user = NULL;
    uint32_t now = 0;
    if (hmac_vap == NULL) {
        oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_send_deauth_filter::hmac_vap null.}");
        return OAL_FALSE;
    }
    /* 找到user就直接返回；未关联用户再走下面流程,防止空口一直存在未关联用户非法帧一直回复deauth */
    hmac_user = mac_vap_get_hmac_user_by_addr(&(hmac_vap->st_vap_base_info), (uint8_t *)da_addr, WLAN_MAC_ADDR_LEN);
    if (hmac_user != NULL) {
        return OAL_FALSE;
    }

    if (hmac_vap->tx_deauth_start_tsf == 0) {
        hmac_vap->tx_deauth_start_tsf = (uint32_t)oal_time_get_stamp_ms();
        return OAL_FALSE;
    }
    now = (uint32_t)oal_time_get_stamp_ms();
    /* 100ms以内不再回复deauth，避免deauth发送攻击导致其他用户数据不通问题 */
    if ((uint32_t)oal_time_get_runtime(hmac_vap->tx_deauth_start_tsf, now) >= HMAC_AP_RX_FRAME_NO_RSP_MAX_TIME) {
        hmac_vap->tx_deauth_start_tsf = 0;
        return OAL_FALSE;
    } else {
        oam_warning_log0(0, OAM_SF_CONN, "{hmac_send_deauth_filter:: filter tx deauth}");
        return OAL_TRUE;
    }
}
/*
 * 1.日    期  : 2013年7月1日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void hmac_mgmt_send_deauth_frame(mac_vap_stru *mac_vap, const unsigned char *da_addr,
    uint16_t err_code, oal_bool_enum_uint8 is_protected)
{
    uint16_t deauth_len;
    oal_netbuf_stru *deauth = NULL;
    mac_tx_ctl_stru *tx_ctl = NULL;
    uint32_t ret;

    if (oal_any_null_ptr2(mac_vap, da_addr)) {
        oam_error_log0(0, OAM_SF_AUTH, "{hmac_mgmt_send_deauth_frame::param null.}");
        return;
    }

    /*  发送去认证前需确定vap处于工作状态 */
    if (mac_vap->en_vap_state == MAC_VAP_STATE_BUTT) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_AUTH,
            "{hmac_mgmt_send_deauth_frame:: vap has been down/del, vap_state[%d].}", mac_vap->en_vap_state);
        return;
    }
    if (hmac_send_deauth_filter(mac_vap, da_addr) == OAL_TRUE) {
        return;
    }

    deauth = (oal_netbuf_stru *)oal_mem_netbuf_alloc(OAL_NORMAL_NETBUF, WLAN_MEM_NETBUF_SIZE2, OAL_NETBUF_PRIORITY_MID);
    oal_mem_netbuf_trace(deauth, OAL_TRUE);

    if (deauth == NULL) {
        /* Reserved Memory pool tried for high priority deauth frames */
        deauth = (oal_netbuf_stru *)oal_mem_netbuf_alloc(OAL_NORMAL_NETBUF,
            WLAN_MEM_NETBUF_SIZE2, OAL_NETBUF_PRIORITY_MID);
        if (deauth == NULL) {
            oam_error_log0(mac_vap->uc_vap_id, OAM_SF_AUTH, "{hmac_mgmt_send_deauth_frame::deauth null.}");
            return;
        }
    }
    memset_s(oal_netbuf_cb(deauth), oal_netbuf_cb_size(), 0, oal_netbuf_cb_size());
    deauth_len = hmac_mgmt_encap_deauth(mac_vap, (uint8_t *)oal_netbuf_header(deauth), da_addr, err_code);
    if (deauth_len == 0) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_AUTH, "{hmac_mgmt_send_deauth_frame:: deauth_len = 0.}");
        oal_netbuf_free(deauth);
        return;
    }
    oal_netbuf_put(deauth, deauth_len);
    /* MIB variables related to deauthentication are updated */
    /* 增加发送去认证帧时的维测信息 */
    oam_warning_log4(mac_vap->uc_vap_id, OAM_SF_CONN,
        "{hmac_mgmt_send_deauth_frame:: DEAUTH TX :  to %2x:XX:XX:XX:%2x:%2x, status code[%d]}",
        da_addr[BYTE_OFFSET_0], da_addr[BYTE_OFFSET_4], da_addr[BYTE_OFFSET_5], err_code);
    tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(deauth); /* 获取cb结构体 */
    mac_get_cb_mpdu_len(tx_ctl) = deauth_len;           /* dmac发送需要的mpdu长度 */
    /* 发送完成需要获取user结构体 */
    if (mac_vap_set_cb_tx_user_idx(mac_vap, tx_ctl, da_addr) != OAL_SUCC) {
        oam_warning_log3(mac_vap->uc_vap_id, OAM_SF_AUTH, "(hmac_mgmt_send_deauth_frame::find user \
            xx:xx:xx:%2x:%2x:%2x fail.}", da_addr[BYTE_OFFSET_3], da_addr[BYTE_OFFSET_4], da_addr[BYTE_OFFSET_5]);
    }
    /* Buffer this frame in the Memory Queue for transmission */
    ret = hmac_tx_mgmt_send_event(mac_vap, deauth, deauth_len);
    if (ret != OAL_SUCC) {
        oal_netbuf_free(deauth);
        oam_warning_log1(mac_vap->uc_vap_id, 0, "{hmac_mgmt_send_deauth_frame::tx_mgmt_send_event failed[%d].}", ret);
    }
}
/*
 * 1.日    期  : 2013年8月27日
  *   修改内容  : 新生成函数
 */
uint32_t hmac_config_send_deauth(mac_vap_stru *pst_mac_vap, uint8_t *puc_da)
{
    hmac_user_stru *pst_hmac_user = NULL;

    if (oal_unlikely(oal_any_null_ptr2(pst_mac_vap, puc_da))) {
        oam_error_log0(0, OAM_SF_AUTH, "{hmac_config_send_deauth::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hmac_user = mac_vap_get_hmac_user_by_addr(pst_mac_vap, puc_da, WLAN_MAC_ADDR_LEN);
    if (pst_hmac_user == NULL) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_AUTH, "{hmac_config_send_deauth::pst_hmac_user null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (pst_hmac_user->st_user_base_info.en_user_asoc_state != MAC_USER_STATE_ASSOC) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_AUTH,
                         "{hmac_config_send_deauth::the user is unassociated.}");
        return OAL_FAIL;
    }

    /* 发去认证帧 */
    hmac_mgmt_send_deauth_frame(pst_mac_vap, puc_da, MAC_AUTH_NOT_VALID, OAL_FALSE);

    /* 删除用户 */
    hmac_user_del(pst_mac_vap, pst_hmac_user);

    return OAL_SUCC;
}

/*
 * 1.日    期  : 2014年1月2日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void hmac_mgmt_send_disassoc_frame(mac_vap_stru *pst_mac_vap, uint8_t *puc_da,
    uint16_t us_err_code, oal_bool_enum_uint8 en_is_protected)
{
    uint16_t us_disassoc_len;
    oal_netbuf_stru *pst_disassoc = 0;
    mac_tx_ctl_stru *pst_tx_ctl = NULL;
    uint32_t ret;

    if (oal_any_null_ptr2(pst_mac_vap, puc_da)) {
        oam_error_log0(0, OAM_SF_ASSOC, "{hmac_mgmt_send_disassoc_frame::mac_vap or da is null.}");
        return;
    }

    /*  发送去关联前需确定vap处于工作状态 */
    if (pst_mac_vap->en_vap_state == MAC_VAP_STATE_BUTT) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_ASSOC,
                         "{hmac_mgmt_send_disassoc_frame:: vap has been down/del, vap_state[%d].}",
                         pst_mac_vap->en_vap_state);
        return;
    }

    pst_disassoc = (oal_netbuf_stru *)oal_mem_netbuf_alloc(OAL_NORMAL_NETBUF,
                                                           WLAN_MEM_NETBUF_SIZE2, OAL_NETBUF_PRIORITY_MID);
    if (pst_disassoc == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_ASSOC, "{hmac_mgmt_send_disassoc_frame::pst_disassoc null.}");
        return;
    }

    oal_mem_netbuf_trace(pst_disassoc, OAL_TRUE);

    memset_s(oal_netbuf_cb(pst_disassoc), oal_netbuf_cb_size(), 0, oal_netbuf_cb_size());

    us_disassoc_len = hmac_mgmt_encap_disassoc(pst_mac_vap, (uint8_t *)oal_netbuf_header(pst_disassoc),
                                               puc_da, us_err_code);
    if (us_disassoc_len == 0) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_ASSOC, "{hmac_mgmt_send_disassoc_frame:: us_disassoc_len = 0.}");
        oal_netbuf_free(pst_disassoc);
        return;
    }

    /* 增加发送去关联帧时的维测信息 */
    oam_warning_log4(pst_mac_vap->uc_vap_id, OAM_SF_CONN,
        "{hmac_mgmt_send_disassoc_frame:: DISASSOC tx, Because of err_code[%d], \
        send disassoc frame to dest addr, da[%2X:XX:XX:XX:%2X:%2X].}",
        us_err_code, puc_da[BYTE_OFFSET_0], puc_da[BYTE_OFFSET_4], puc_da[BYTE_OFFSET_5]);

    pst_tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_disassoc);
    mac_get_cb_mpdu_len(pst_tx_ctl) = us_disassoc_len;
    /* 填写非法值,发送完成之后获取用户为NULL,直接释放去认证帧  */
    ret = mac_vap_set_cb_tx_user_idx(pst_mac_vap, pst_tx_ctl, puc_da);
    if (ret != OAL_SUCC) {
        oam_warning_log3(pst_mac_vap->uc_vap_id, OAM_SF_ASSOC,
                         "(hmac_mgmt_send_disassoc_frame::fail to find user by xx:xx:xx:0x:0x:0x.}",
                         puc_da[BYTE_OFFSET_3], puc_da[BYTE_OFFSET_4], puc_da[BYTE_OFFSET_5]);
    }

    oal_netbuf_put(pst_disassoc, us_disassoc_len);

    /* 加入发送队列 */
    ret = hmac_tx_mgmt_send_event(pst_mac_vap, pst_disassoc, us_disassoc_len);
    if (ret != OAL_SUCC) {
        oal_netbuf_free(pst_disassoc);
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_ASSOC,
                         "{hmac_mgmt_send_disassoc_frame::hmac_tx_mgmt_send_event failed[%d].}", ret);
    }
}

OAL_STATIC void hmac_mgmt_update_qos_by_ie(uint8_t *payload, uint16_t msg_len, mac_vap_stru *mac_vap,
    hmac_user_stru **hmac_user)
{
    uint8_t *ie = NULL;

    ie = mac_get_wmm_ie(payload, msg_len);
    if (ie != NULL) {
        /*  当用户关联的时候，要根据用户信息和AP自身信息来更新用户最终可用的qos信息。 */
        mac_user_set_qos(&((*hmac_user)->st_user_base_info),
                         mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11QosOptionImplemented);
        return;
    }

    if (is_sta(mac_vap)) { /*  */
        /* 如果关联用户之前就是没有携带wmm ie, 再查找HT CAP能力 */
        ie = mac_find_ie(MAC_EID_HT_CAP, payload, msg_len);
        if (ie != NULL) {
            /*  再查找HT CAP能力第2字节BIT 5 short GI for 20M 能力位 */
            if ((ie[BYTE_OFFSET_1] >= 2) && (ie[BYTE_OFFSET_2] & BIT5)) { // 2表示ht cap长度的校验值
                mac_user_set_qos(&((*hmac_user)->st_user_base_info), OAL_TRUE);
            }
        }
    }
    return;
}

/*
 * 日     期  : 2013年10月24日
 * 作     者  : wifi
 * 修改内容 : 新生成函数
 */
void hmac_mgmt_update_assoc_user_qos_table(uint8_t *payload, uint16_t msg_len, hmac_user_stru *hmac_user)
{
    mac_vap_stru *mac_vap = NULL;

    /* 如果关联用户之前就是wmm使能的，什么都不用做，直接返回  */
    if (hmac_user->st_user_base_info.st_cap_info.bit_qos == OAL_TRUE) {
        return;
    }

    mac_user_set_qos(&hmac_user->st_user_base_info, OAL_FALSE);

    mac_vap = (mac_vap_stru *)mac_res_get_mac_vap(hmac_user->st_user_base_info.uc_vap_id);
    if (oal_unlikely(mac_vap == NULL)) {
        oam_error_log0(hmac_user->st_user_base_info.uc_vap_id, OAM_SF_ASSOC,
                       "{hmac_mgmt_update_assoc_user_qos_table::pst_mac_vap null.}");
        return;
    }

    hmac_mgmt_update_qos_by_ie(payload, msg_len, mac_vap, &hmac_user);
}

/*
 * 1.日    期  : 2014年7月2日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
#ifdef _PRE_WLAN_FEATURE_TXBF
void hmac_mgmt_update_11ntxbf_cap(uint8_t *puc_payload, hmac_user_stru *pst_hmac_user)
{
    mac_11ntxbf_vendor_ie_stru *pst_vendor_ie;
    if (puc_payload == NULL) {
        return;
    }

    /* 检测到vendor ie */
    pst_vendor_ie = (mac_11ntxbf_vendor_ie_stru *)puc_payload;

    if (pst_vendor_ie->uc_len < (sizeof(mac_11ntxbf_vendor_ie_stru) - MAC_IE_HDR_LEN)) {
        oam_warning_log1(0, OAM_SF_ANY, "hmac_mgmt_update_11ntxbf_cap:invalid vendor ie len[%d]",
                         pst_vendor_ie->uc_len);
        return;
    }

    pst_hmac_user->st_user_base_info.st_cap_info.bit_11ntxbf = pst_vendor_ie->st_11ntxbf.bit_11ntxbf;

    return;
}
#endif /* #ifdef _PRE_WLAN_FEATURE_TXBF */

/*
 * 1.日    期  : 2013年12月6日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_check_bss_cap_info(uint16_t us_cap_info, mac_vap_stru *pst_mac_vap)
{
    hmac_vap_stru *pst_hmac_vap;
    uint32_t ret;
    wlan_mib_desired_bsstype_enum en_bss_type;

    /* 获取CAP INFO里BSS TYPE */
    en_bss_type = mac_get_bss_type(us_cap_info);

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == NULL) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_check_bss_cap_info::pst_hmac_vap null.}");
        return OAL_FAIL;
    }

    /* 比较BSS TYPE是否一致 不一致，如果是STA仍然发起入网，增加兼容性，其它模式则返回不支持 */
    if (en_bss_type != mac_mib_get_DesiredBSSType(pst_mac_vap)) {
        oam_warning_log2(pst_mac_vap->uc_vap_id, OAM_SF_ASSOC,
                         "{hmac_check_bss_cap_info::vap_bss_type[%d] is different from asoc_user_bss_type[%d].}",
                         mac_mib_get_DesiredBSSType(pst_mac_vap), en_bss_type);
    }

    if (OAL_TRUE == mac_mib_get_WPSActive(pst_mac_vap)) {
        return OAL_TRUE;
    }

    /* 比较CAP INFO中privacy位，检查是否加密，加密不一致，返回失败 */
    ret = mac_check_mac_privacy(us_cap_info, (uint8_t *)pst_mac_vap);
    if (ret != OAL_TRUE) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_ASSOC,
                         "{hmac_check_bss_cap_info::mac privacy capabilities is different.}");
    }

    return OAL_TRUE;
}
OAL_STATIC OAL_INLINE oal_bool_enum_uint8 hmac_user_protocol_mode_vap_protocol_check(
    mac_user_stru *pst_mac_user, mac_vap_stru *pst_mac_vap)
{
    /* 兼容性问题：思科AP 2.4G（11b）和5G(11a)共存时发送的assoc rsp帧携带的速率分别是11g和11b，导致STA创建用户时通知算法失败，
    Autorate失效，DBAC情况下，DBAC无法启动已工作的VAP状态无法恢复的问题 临时方案，建议针对对端速率异常的情况统一分析优化 */
    return (((pst_mac_user->en_protocol_mode == WLAN_LEGACY_11B_MODE) &&
             (pst_mac_vap->en_protocol == WLAN_LEGACY_11A_MODE)) ||
            ((pst_mac_user->en_protocol_mode == WLAN_LEGACY_11G_MODE) &&
             (pst_mac_vap->en_protocol == WLAN_LEGACY_11B_MODE))) ? OAL_TRUE : OAL_FALSE;
}

OAL_STATIC void hmac_set_usr_proto_mode_by_band(mac_vap_stru *mac_vap, hmac_user_stru *hmac_user,
    mac_user_stru *mac_user)
{
    if (mac_vap->st_channel.en_band == WLAN_BAND_5G) { /* 判断是否是5G */
        mac_user_set_protocol_mode(mac_user, WLAN_LEGACY_11A_MODE);
    } else {
        if (hmac_is_support_11grate(hmac_user->st_op_rates.auc_rs_rates,
            hmac_user->st_op_rates.uc_rs_nrates) == OAL_TRUE) {
            mac_user_set_protocol_mode(mac_user, WLAN_LEGACY_11G_MODE);
            if (hmac_is_support_11brate(hmac_user->st_op_rates.auc_rs_rates,
                hmac_user->st_op_rates.uc_rs_nrates) == OAL_TRUE) {
                mac_user_set_protocol_mode(mac_user, WLAN_MIXED_ONE_11G_MODE);
            }
        } else if (hmac_is_support_11brate(hmac_user->st_op_rates.auc_rs_rates,
            hmac_user->st_op_rates.uc_rs_nrates) == OAL_TRUE) {
            mac_user_set_protocol_mode(mac_user, WLAN_LEGACY_11B_MODE);
        } else {
            oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_set_user_protocol_mode::set usr proto fail.}");
        }
    }
    return;
}

OAL_STATIC void hmac_set_usr_proto_mode_core(mac_vap_stru *mac_vap, hmac_user_stru *hmac_user,
    mac_user_stru *mac_user, mac_user_ht_hdl_stru *mac_ht_hdl, mac_vht_hdl_stru *mac_vht_hdl)
{
#ifdef _PRE_WLAN_FEATURE_11AX
    if (mac_user_is_he_user(&hmac_user->st_user_base_info)) {
        mac_user_set_protocol_mode(mac_user, WLAN_HE_MODE);
        return;
    }
#endif
    if (mac_vht_hdl->en_vht_capable == OAL_TRUE) {
        mac_user_set_protocol_mode(mac_user, WLAN_VHT_MODE);
    } else if (mac_ht_hdl->en_ht_capable == OAL_TRUE) {
        mac_user_set_protocol_mode(mac_user, WLAN_HT_MODE);
    } else {
        hmac_set_usr_proto_mode_by_band(mac_vap, hmac_user, mac_user);
    }
    return;
}

void hmac_set_user_protocol_mode(mac_vap_stru *mac_vap, hmac_user_stru *hmac_user)
{
    mac_user_stru *mac_user = &hmac_user->st_user_base_info;
    mac_user_ht_hdl_stru *mac_ht_hdl = &(mac_user->st_ht_hdl);
    mac_vht_hdl_stru *mac_vht_hdl = &(mac_user->st_vht_hdl);

    hmac_set_usr_proto_mode_core(mac_vap, hmac_user, mac_user, mac_ht_hdl, mac_vht_hdl);

    if (hmac_user_protocol_mode_vap_protocol_check(mac_user, mac_vap) == OAL_TRUE) {
        mac_user_set_protocol_mode(mac_user, mac_vap->en_protocol);
        if (memcpy_s((void *)&(hmac_user->st_op_rates), sizeof(hmac_user->st_op_rates),
            (void *)&(mac_vap->st_curr_sup_rates.st_rate), sizeof(mac_rate_stru)) != EOK) {
            oam_error_log0(0, OAM_SF_ANY, "hmac_set_user_protocol_mode::memcpy fail!");
            return;
        }
    }

    /*
     * 兼容性问题：ws880配置11a时beacon和probe resp帧中协议vht能力，
     * vap的protocol能力要根据关联响应帧的实际能力刷新成实际11a能力
     */
    if ((mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_STA) &&
        (mac_vap->en_protocol == WLAN_VHT_MODE) && (mac_user->en_protocol_mode == WLAN_LEGACY_11A_MODE)) {
        mac_vap_init_by_protocol(mac_vap, mac_user->en_protocol_mode);
    }

    mac_vap_tx_data_set_user_htc_cap(mac_vap, &hmac_user->st_user_base_info);
}
/*
 * 1.日    期  : 2015年3月21日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void hmac_user_init_rates(hmac_user_stru *pst_hmac_user)
{
    memset_s((uint8_t *)(&pst_hmac_user->st_op_rates), sizeof(pst_hmac_user->st_op_rates),
             0, sizeof(pst_hmac_user->st_op_rates));
}

/*
 * 1.日    期  : 2015年3月21日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint8_t hmac_add_user_rates(hmac_user_stru *pst_hmac_user, uint8_t uc_rates_cnt, uint8_t *puc_rates)
{
    if (pst_hmac_user->st_op_rates.uc_rs_nrates + uc_rates_cnt > WLAN_USER_MAX_SUPP_RATES) {
        uc_rates_cnt = WLAN_USER_MAX_SUPP_RATES - pst_hmac_user->st_op_rates.uc_rs_nrates;
    }

    if (memcpy_s(&(pst_hmac_user->st_op_rates.auc_rs_rates[pst_hmac_user->st_op_rates.uc_rs_nrates]),
        WLAN_USER_MAX_SUPP_RATES - pst_hmac_user->st_op_rates.uc_rs_nrates, puc_rates, uc_rates_cnt) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "hmac_add_user_rates::memcpy fail!");
        return uc_rates_cnt;
    }
    pst_hmac_user->st_op_rates.uc_rs_nrates += uc_rates_cnt;

    return uc_rates_cnt;
}

/*
 * 1.日    期  : 2020年4月9日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t hmac_rx_mgmt_event_set_intf_name(hmac_vap_stru *hmac_vap, hmac_rx_mgmt_event_stru *mgmt_frame,
    mac_rx_ctl_stru *rx_info)
{
    mac_ieee80211_frame_stru *pst_frame_hdr = NULL;
    mac_vap_stru *mac_vap = &(hmac_vap->st_vap_base_info);
    uint8_t hal_vap_id;
    int32_t ret;

    ret = memcpy_s(mgmt_frame->ac_name, sizeof(mgmt_frame->ac_name),
        hmac_vap->pst_net_device->name, OAL_IF_NAME_SIZE);
    if (ret != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "hmac_rx_mgmt_event_set_intf_name::memcpy ac_name failed!");
    }
    if (!is_legacy_vap(mac_vap)) {
        /*
         * 仅针对P2P设备做处理。P2P vap 存在一个vap 对应多个hal_vap 情况，
         * 非P2P vap 不存在一个vap 对应多个hal_vap 情况
         */
        /* 对比接收到的管理帧vap_id 是否和vap 中hal_vap_id 相同 */
        /* 从管理帧cb字段中的hal vap id 的相应信息查找对应的net dev 指针 */
        pst_frame_hdr = (mac_ieee80211_frame_stru *)mac_get_rx_cb_mac_hdr(rx_info);
        hal_vap_id = mac_get_rx_cb_hal_vap_id(rx_info);
        if (oal_compare_mac_addr(pst_frame_hdr->auc_address1, mac_mib_get_p2p0_dot11StationID(mac_vap)) == OAL_SUCC) {
            /* 第二个net dev槽 */
            ret = memcpy_s(mgmt_frame->ac_name, sizeof(mgmt_frame->ac_name),
                hmac_vap->pst_p2p0_net_device->name, OAL_IF_NAME_SIZE);
            if (ret != EOK) {
                oam_error_log0(0, OAM_SF_ANY, "mac_mib_get_p2p0_dot11StationID:memcpy ac_name failed!");
            }
        } else if (oal_compare_mac_addr(pst_frame_hdr->auc_address1, mac_mib_get_StationID(mac_vap)) == OAL_SUCC) {
            ret = memcpy_s(mgmt_frame->ac_name, sizeof(mgmt_frame->ac_name),
                hmac_vap->pst_net_device->name, OAL_IF_NAME_SIZE);
            if (ret != EOK) {
                oam_error_log0(0, OAM_SF_ANY, "mac_mib_get_StationID:memcpy ac_name failed!");
            }
        } else if ((hal_vap_id == g_wlan_spec_cfg->other_bss_id) && (is_p2p_cl(mac_vap) || is_p2p_dev(mac_vap)) &&
                   (pst_frame_hdr->st_frame_control.bit_sub_type == WLAN_PROBE_REQ)) {
            if (!hmac_get_feature_switch(HMAC_MIRACAST_SINK_SWITCH)) {
                return OAL_FAIL;
            }
            ret = memcpy_s(mgmt_frame->ac_name, sizeof(mgmt_frame->ac_name),
                hmac_vap->pst_p2p0_net_device->name, OAL_IF_NAME_SIZE);
            if (ret != EOK) {
                oam_error_log0(0, OAM_SF_ANY, ":memcpy ac_name failed!");
            }
        } else {
            return OAL_FAIL;
        }
    }

    return OAL_SUCC;
}

OAL_STATIC uint32_t hmac_send_mgmt_to_host_pre_handle(hmac_vap_stru *hmac_vap, frw_event_mem_stru **event_mem)
{
    mac_device_stru *mac_device = NULL;

    mac_device = mac_res_get_dev(hmac_vap->st_vap_base_info.uc_device_id);
    if (mac_device == NULL) {
        oam_warning_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN, "{hmac_send_mgmt_to_host::mac_dev null.}");
        return OAL_FAIL;
    }

    /* 抛关联一个新的sta完成事件到WAL */
    *event_mem = frw_event_alloc_m(sizeof(hmac_rx_mgmt_event_stru));
    if (*event_mem == NULL) {
        oam_error_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN, "{hmac_send_mgmt_to_host::malloc fail}");
        return OAL_FAIL;
    }

    return OAL_SUCC;
}

void hmac_send_mgmt_to_host(hmac_vap_stru *hmac_vap, oal_netbuf_stru *buf, uint16_t len, int32_t freq)
{
    frw_event_mem_stru *event_mem = NULL;
    frw_event_stru *event = NULL;
    hmac_rx_mgmt_event_stru *mgmt_frame = NULL;
    mac_rx_ctl_stru *rx_info = (mac_rx_ctl_stru *)oal_netbuf_cb(buf);
    uint8_t *mgmt_data = NULL;
    uint32_t ret = hmac_send_mgmt_to_host_pre_handle(hmac_vap, &event_mem);
    if (ret != OAL_SUCC) {
        return;
    }
    if (len > WLAN_MEM_NETBUF_SIZE2) { // 长度保护: 防止下方申请内存溢出
        oam_error_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
            "{hmac_send_mgmt_to_host::frame len[%d] invalid.}", len);
        frw_event_free_m(event_mem);
        return;
    }

    mgmt_data = (uint8_t *)oal_memalloc(len);
    if (mgmt_data == NULL) {
        oam_error_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN, "{hmac_send_mgmt_to_host::malloc fail.}");
        frw_event_free_m(event_mem);
        return;
    }

    if (memcpy_s(mgmt_data, len, (uint8_t *)mac_get_rx_cb_mac_header_addr(rx_info), len) != EOK) {
        oam_error_log0(0, OAM_SF_SCAN, "hmac_send_mgmt_to_host::memcpy fail!");
        frw_event_free_m(event_mem);
        oal_free(mgmt_data);
        return;
    }

    /* 填写事件 */
    event = frw_get_event_stru(event_mem);

    frw_event_hdr_init(&(event->st_event_hdr), FRW_EVENT_TYPE_HOST_CTX, HMAC_HOST_CTX_EVENT_SUB_TYPE_RX_MGMT,
        sizeof(hmac_rx_mgmt_event_stru), FRW_EVENT_PIPELINE_STAGE_0, hmac_vap->st_vap_base_info.uc_chip_id,
        hmac_vap->st_vap_base_info.uc_device_id, hmac_vap->st_vap_base_info.uc_vap_id);

    /* 填写上报管理帧数据 */
    mgmt_frame = (hmac_rx_mgmt_event_stru *)(event->auc_event_data);
    mgmt_frame->puc_buf = (uint8_t *)mgmt_data;
    mgmt_frame->us_len = len;
    mgmt_frame->l_freq = freq;
    mgmt_frame->event_type = mac_get_rx_cb_nan_flag(rx_info) ?
        HMAC_RX_MGMT_EVENT_TYPE_NAN : HMAC_RX_MGMT_EVENT_TYPE_NORMAL;
    oal_netbuf_len(buf) = len;

    if (hmac_rx_mgmt_event_set_intf_name(hmac_vap, mgmt_frame, rx_info) != OAL_SUCC) {
        frw_event_free_m(event_mem);
        oal_free(mgmt_data);
        return;
    }

    /* 分发事件 */
    if (frw_event_dispatch_event(event_mem) != OAL_SUCC) {
        oal_free(mgmt_data);
    }
    frw_event_free_m(event_mem);
}

/*
 * 1.日    期  : 2014年12月25日
  *   修改内容  : 新生成函数
 */
uint32_t hmac_wpas_mgmt_tx(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    oal_netbuf_stru *pst_netbuf_mgmt_tx = NULL;
    mac_tx_ctl_stru *pst_tx_ctl = NULL;
    uint32_t ret;
    hmac_mgmt_frame_stru *pst_mgmt_tx = NULL;
    mac_device_stru *pst_mac_device = NULL;
    //  解决管理帧发送过程中已关联的用户user_index的问题，从而解决向已关联用户发送管理帧时节能缓存的问题
    uint8_t ua_dest_mac_addr[WLAN_MAC_ADDR_LEN];
    uint16_t us_user_idx;

    pst_mgmt_tx = (hmac_mgmt_frame_stru *)puc_param;
    if (pst_mgmt_tx->frame == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_P2P, "{hmac_wpas_mgmt_tx::frame is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    pst_netbuf_mgmt_tx = pst_mgmt_tx->frame;
    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (pst_mac_device == NULL) {
        oal_netbuf_free(pst_netbuf_mgmt_tx);
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_P2P, "{hmac_wpas_mgmt_tx::pst_mac_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_P2P, "{hmac_wpas_mgmt_tx::mgmt frame id=[%d]}",
        pst_mgmt_tx->mgmt_frame_id);

    memset_s(oal_netbuf_cb(pst_netbuf_mgmt_tx), sizeof(mac_tx_ctl_stru), 0, sizeof(mac_tx_ctl_stru));
    /*
     *  解决管理帧发送过程中已关联的用户user_index的问题，
     * 从而解决向已关联用户发送管理帧时节能缓存的问题
     */
    mac_get_addr1(oal_netbuf_data(pst_netbuf_mgmt_tx), ua_dest_mac_addr, WLAN_MAC_ADDR_LEN);
    us_user_idx = g_wlan_spec_cfg->invalid_user_id;
    /* 管理帧可能发给已关联的用户，也可能发给未关联的用户。已关联的用户可以找到，未关联的用户找不到。不用判断返回值 */
    ret = mac_vap_find_user_by_macaddr(pst_mac_vap, ua_dest_mac_addr, &us_user_idx);

    pst_tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_netbuf_mgmt_tx); /* 获取cb结构体 */
    mac_get_cb_mpdu_len(pst_tx_ctl) = oal_netbuf_len(pst_netbuf_mgmt_tx); /* dmac发送需要的mpdu长度 */
    mac_get_cb_tx_user_idx(pst_tx_ctl) = us_user_idx;                  /* 发送完成需要获取user结构体 */
    mac_get_cb_is_need_resp(pst_tx_ctl) = OAL_TRUE;
    mac_get_cb_is_needretry(pst_tx_ctl) = OAL_TRUE;
    mac_get_cb_mgmt_frame_id(pst_tx_ctl) = pst_mgmt_tx->mgmt_frame_id;
    /* 仅用subtype做识别, 置action会误入dmac_tx_action_process */
    mac_get_cb_frame_type(pst_tx_ctl) = WLAN_CB_FRAME_TYPE_MGMT;
    mac_get_cb_frame_subtype(pst_tx_ctl) = WLAN_ACTION_P2PGO_FRAME_SUBTYPE;

    /* Buffer this frame in the Memory Queue for transmission */
    ret = hmac_tx_mgmt_send_event(pst_mac_vap, pst_netbuf_mgmt_tx, oal_netbuf_len(pst_netbuf_mgmt_tx));
    if (ret != OAL_SUCC) {
        oal_netbuf_free(pst_netbuf_mgmt_tx);

        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_P2P,
                         "{hmac_wpas_mgmt_tx::hmac_tx_mgmt_send_event failed[%d].}", ret);
        return OAL_FAIL;
    }

    return OAL_SUCC;
}

/*
 * 1.日    期  : 2015年7月25日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void hmac_rx_mgmt_send_to_host(hmac_vap_stru *pst_hmac_vap, oal_netbuf_stru *pst_netbuf)
{
    mac_rx_ctl_stru *pst_rx_info;
    int32_t l_freq;

    pst_rx_info = (mac_rx_ctl_stru *)oal_netbuf_cb(pst_netbuf);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0))
    l_freq = oal_ieee80211_channel_to_frequency(pst_rx_info->uc_channel_number,
        (pst_rx_info->uc_channel_number > 14) ? NL80211_BAND_5GHZ : NL80211_BAND_2GHZ); // 14信道
#else
    l_freq = oal_ieee80211_channel_to_frequency(pst_rx_info->uc_channel_number,
        (pst_rx_info->uc_channel_number > 14) ? IEEE80211_BAND_5GHZ : IEEE80211_BAND_2GHZ); // 14信道
#endif
    hmac_send_mgmt_to_host(pst_hmac_vap, pst_netbuf, pst_rx_info->us_frame_len, l_freq);
}

#ifdef _PRE_WLAN_FEATURE_LOCATION
uint8_t g_auc_send_csi_buf[HMAC_CSI_SEND_BUF_LEN] = { 0 };
uint8_t g_auc_send_ftm_buf[HMAC_FTM_SEND_BUF_LEN] = { 0 };

OAL_STATIC uint32_t hmac_netlink_location_csi_ie_first_fragment_proc(hmac_location_event_stru *location,
    uint8_t *csi_buf, oal_time_stru local_time)
{
    uint32_t idx = 0;
    memset_s(csi_buf, HMAC_CSI_SEND_BUF_LEN, 0, HMAC_CSI_SEND_BUF_LEN);
    *(uint32_t *)&csi_buf[idx] = (uint32_t)NUM_2_BITS;
    idx += 8; /* Type 4 Bytes + len 4 Bytes = 8 Bytes */

    /* mac1 6Bytes */
    ret = memcpy_s(&csi_buf[idx], WLAN_MAC_ADDR_LEN, location->auc_mac_server, WLAN_MAC_ADDR_LEN);
    if (ret != EOK) {
        return OAL_FAIL;
    }
    idx += WLAN_MAC_ADDR_LEN;
    /* mac2 6Bytes */
    ret = memcpy_s(&csi_buf[idx], WLAN_MAC_ADDR_LEN, location->auc_mac_client, WLAN_MAC_ADDR_LEN);
    if (ret != EOK) {
        return OAL_FAIL;
    }
    idx += WLAN_MAC_ADDR_LEN;
    /* timestamp23Bytes */
    ret = snprintf_s(csi_buf + idx, HMAC_FTM_SEND_BUF_LEN, HMAC_FTM_SEND_BUF_LEN - 1,
        "%04d-%02d-%02d %02d:%02d:%02d.%03ld", local_time.tm_year + BASE_YEAR,
        local_time.tm_mon + 1, local_time.tm_mday, local_time.tm_hour, local_time.tm_min,
        local_time.tm_sec, st_tv.tv_usec / HMAC_S_TO_MS);
    if (ret < 0) {
        oam_error_log0(0, OAM_SF_FTM, "hmac_netlink_location_send::snprintf_s error!");
        return OAL_FAIL;
    }
    idx += ret;

    /* rssi snr */
    ret = memcpy_s(&csi_buf[idx], MAC_REPORT_RSSIINFO_SNR_LEN, payload + NUM_3_BITS, MAC_REPORT_RSSIINFO_SNR_LEN);
    if (ret != EOK) {
        return OAL_FAIL;
    }
    oam_warning_log1(0, OAM_SF_FTM, "{hmac_netlink_location_send::len[%d].}", idx + MAC_REPORT_RSSIINFO_SNR_LEN);
    return OAL_SUCC;
}

OAL_STATIC uint32_t hmac_netlink_location_send_csi_ie(hmac_location_event_stru *location, uint16_t action_len,
    oal_time_stru local_time)
{
    uint8_t *payload = NULL;
    payload = (uint8_t *)(location->auc_payload);
    uint8_t *csi_buf = g_auc_send_csi_buf;
    uint32_t *csi_buf_len = NULL;
    int32_t ret;
    /* 第一片 */
    if ((payload[0] == 0) && ((payload[1] == 0) || (payload[1] == 1))) {
        if (hmac_netlink_location_csi_ie_first_fragment_proc(location, csi_buf, local_time) != OAL_SUCC) {
            return OAL_FAIL;
        }
    }

    csi_buf_len = (uint32_t *)&csi_buf[BYTE_OFFSET_4];
    if (*csi_buf_len + action_len - MAC_CSI_LOCATION_INFO_LEN > HMAC_CSI_SEND_BUF_LEN) {
        oam_error_log0(0, OAM_SF_FTM, "{hmac_netlink_location_send::puc_send_buf not enough.}");
        return OAL_FAIL;
    }

    ret = memcpy_s(&csi_buf[*csi_buf_len], action_len - MAC_CSI_LOCATION_INFO_LEN,
                   payload + NUM_3_BITS + MAC_REPORT_RSSIINFO_SNR_LEN, action_len - MAC_CSI_LOCATION_INFO_LEN);
    if (ret != EOK) {
        return OAL_FAIL;
    }
    *csi_buf_len += action_len - MAC_CSI_LOCATION_INFO_LEN;

    /* 最后一片 */
    if (location->auc_payload[BYTE_OFFSET_2] == 1) {
        drv_netlink_location_send((void *)csi_buf, *csi_buf_len);
        oam_warning_log1(0, OAM_SF_FTM, "{hmac_netlink_location_send::send len[%d].}", *csi_buf_len);
    }
    return OAL_SUCC;
}

OAL_STATIC uint32_t hmac_netlink_location_send_ftm_ie(hmac_location_event_stru *location, oal_time_stru local_time)
{
    uint8_t *ftm_buf = g_auc_send_ftm_buf;
    uint32_t idx = 0;
    uint8_t *payload = NULL;
    int32_t ret;

    memset_s(ftm_buf, HMAC_FTM_SEND_BUF_LEN, 0, HMAC_FTM_SEND_BUF_LEN);
    *(uint32_t *)&ftm_buf[idx] = (uint32_t)NUM_3_BITS;
    idx += NUM_4_BITS;
    *(uint32_t *)&ftm_buf[idx] = 99; /* 99表示Follow Up Dialog Token的数值 */
    idx += NUM_4_BITS;
    ret = memcpy_s(&ftm_buf[idx], WLAN_MAC_ADDR_LEN, location->auc_mac_server, WLAN_MAC_ADDR_LEN);
    if (ret != EOK) {
        oam_error_log0(0, OAM_SF_FTM, "hmac_netlink_location_send::memcpy mac_server fail!");
        return OAL_FAIL;
    }
    idx += WLAN_MAC_ADDR_LEN;
    ret = memcpy_s(&ftm_buf[idx], WLAN_MAC_ADDR_LEN, location->auc_mac_client, WLAN_MAC_ADDR_LEN);
    if (ret != EOK) {
        oam_error_log0(0, OAM_SF_FTM, "hmac_netlink_location_send::memcpy mac_client fail!");
        return OAL_FAIL;
    }
    idx += WLAN_MAC_ADDR_LEN;
    ret = snprintf_s(ftm_buf + idx, HMAC_FTM_SEND_BUF_LEN,
                     HMAC_FTM_SEND_BUF_LEN - 1, "%04d-%02d-%02d %02d:%02d:%02d.%03ld",
                     local_time.tm_year + BASE_YEAR, local_time.tm_mon + 1, local_time.tm_mday, local_time.tm_hour,
                     local_time.tm_min, local_time.tm_sec, st_tv.tv_usec / HMAC_S_TO_MS);
    if (ret < 0) {
        return OAL_FAIL;
    }
    idx += ret;
    payload = (uint8_t *)(location->auc_payload);
    ret = memcpy_s(&ftm_buf[idx], (HMAC_FTM_SEND_BUF_LEN - idx), payload, LOCATION_REAL_LEN);
    if (ret != EOK) {
        oam_error_log0(0, OAM_SF_FTM, "hmac_netlink_location_send::memcpy payload fail!");
        return OAL_FAIL;
    }
    drv_netlink_location_send((void *)ftm_buf, 99); /* 99表示buf长度 */
    return OAL_SUCC;
}

/*
 * 1.日    期  : 2017年11月3日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t hmac_netlink_location_send(hmac_vap_stru *pst_hmac_vap, oal_netbuf_stru *pst_netbuf)
{
    hmac_location_event_stru *location = NULL;
    oal_timespec_stru st_tv;
    oal_time_stru local_time;
    uint16_t action_len;
    mac_rx_ctl_stru *pst_rx_ctrl;
    int32_t ret;

    location = (hmac_location_event_stru *)mac_get_80211_mgmt_payload(pst_netbuf);
    pst_rx_ctrl = (mac_rx_ctl_stru *)oal_netbuf_cb(pst_netbuf);
    action_len = pst_rx_ctrl->us_frame_len;

    if (action_len < MAC_CSI_LOCATION_INFO_LEN) {
        oam_error_log1(0, OAM_SF_FTM, "{hmac_netlink_location_send: unexpected len %d}", action_len);
        return OAL_ERR_CODE_MSG_LENGTH_ERR;
    }
    oal_do_gettimeofday(&st_tv);
    oal_get_real_time_m(&local_time);

    switch (location->uc_location_type) {
        case MAC_HISI_LOCATION_CSI_IE:
            ret = hmac_netlink_location_send_csi_ie(location, action_len, local_time);
            break;
        default:
            return OAL_SUCC;
    }

    return ret;
}
#endif
#if defined(_PRE_WLAN_FEATURE_LOCATION)
OAL_STATIC uint32_t hmac_proc_location_action(hmac_vap_stru *pst_hmac_vap, oal_netbuf_stru *pst_netbuf)
{
    return OAL_SUCC;
}
/*
 * 1.日    期  : 2017年11月2日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint8_t hmac_huawei_action_process(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf)
{
    uint8_t action_pub_vend_type;
    uint16_t payload_len;
    uint8_t *payload = NULL;

    payload = mac_get_rx_payload(netbuf, &payload_len);
    if (oal_any_null_ptr1(payload)) {
        return OAL_FALSE;
    }
    if (payload_len < MAC_ACTION_CATEGORY_AND_CODE_LEN + MAC_OUI_LEN + MAC_ACTION_VENDOR_TYPE_LEN) {
        return OAL_FALSE;
    }

    if (oal_memcmp(payload + MAC_ACTION_CATEGORY_AND_CODE_LEN, g_auc_huawei_oui, MAC_OUI_LEN) != 0) {
        return OAL_FALSE;
    }
    action_pub_vend_type = payload[MAC_ACTION_VENDOR_TYPE_OFFSET];

    oam_warning_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_RX, "{hmac_ap_up_rx_action::hmac location get.}");
    switch (action_pub_vend_type) {
        case MAC_HISI_LOCATION_CSI_IE:
            /* 将其他设备上报的私有信息去掉ie头抛事件到hmac进行保存 */
            /* type | mac_s | mac_c   | csi or ftm or rssi   */
            /* csi 信息注意长度 */
            hmac_proc_location_action(hmac_vap, netbuf);
#ifdef _PRE_WLAN_FEATURE_LOCATION
            hmac_netlink_location_send(hmac_vap, netbuf);
#endif
            break;
        default:
            break;
    }
    return OAL_TRUE;
}
#endif
/*
 * 1.日    期  : 2014年12月31日
  *   修改内容  : 新生成函数
 */
uint32_t hmac_mgmt_tx_event_status(mac_vap_stru *pst_mac_vap, uint8_t uc_len, uint8_t *puc_param)
{
    dmac_crx_mgmt_tx_status_stru *pst_mgmt_tx_status_param = NULL;
    dmac_crx_mgmt_tx_status_stru *pst_mgmt_tx_status_param_2wal = NULL;
    frw_event_mem_stru *pst_event_mem = NULL;
    frw_event_stru *pst_event = NULL;
    uint32_t ret;

    if (puc_param == NULL) {
        oam_error_log0(0, OAM_SF_P2P, "{hmac_mgmt_tx_event_status::puc_param is null!}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mgmt_tx_status_param = (dmac_crx_mgmt_tx_status_stru *)puc_param;

    oam_warning_log3(pst_mac_vap->uc_vap_id, OAM_SF_P2P,
        "{hmac_mgmt_tx_event_status::dmac tx mgmt status report.userindx[%d], tx mgmt status[%d], frame_id[%d]}",
        pst_mgmt_tx_status_param->us_user_idx,
        pst_mgmt_tx_status_param->uc_dscr_status,
        pst_mgmt_tx_status_param->mgmt_frame_id);

    /* 抛管理帧发送完成事件到WAL */
    pst_event_mem = frw_event_alloc_m(sizeof(dmac_crx_mgmt_tx_status_stru));
    if (pst_event_mem == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_P2P, "{hmac_mgmt_tx_event_status::pst_event_mem null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 填写事件 */
    pst_event = frw_get_event_stru(pst_event_mem);

    frw_event_hdr_init(&(pst_event->st_event_hdr),
                       FRW_EVENT_TYPE_HOST_CTX,
                       HMAC_HOST_CTX_EVENT_SUB_TYPE_MGMT_TX_STATUS,
                       sizeof(dmac_crx_mgmt_tx_status_stru),
                       FRW_EVENT_PIPELINE_STAGE_0,
                       pst_mac_vap->uc_chip_id,
                       pst_mac_vap->uc_device_id,
                       pst_mac_vap->uc_vap_id);

    pst_mgmt_tx_status_param_2wal = (dmac_crx_mgmt_tx_status_stru *)(pst_event->auc_event_data);
    pst_mgmt_tx_status_param_2wal->uc_dscr_status = pst_mgmt_tx_status_param->uc_dscr_status;
    pst_mgmt_tx_status_param_2wal->mgmt_frame_id = pst_mgmt_tx_status_param->mgmt_frame_id;

    /* 分发事件 */
    ret = frw_event_dispatch_event(pst_event_mem);
    frw_event_free_m(pst_event_mem);

    return ret;
}

oal_bool_enum_uint8 hmac_is_equal_rates(uint8_t rate1, uint8_t rate2)
{
    return ((rate1 & 0x7f) == (rate2 & 0x7f));
}

/*
 * 1.日    期  : 2013年10月16日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void hmac_vap_set_user_avail_rates(mac_vap_stru *pst_mac_vap, hmac_user_stru *pst_hmac_user)
{
    mac_user_stru *pst_mac_user = NULL;
    mac_curr_rateset_stru *pst_mac_vap_rate = NULL;
    hmac_rate_stru *pst_hmac_user_rate;
    mac_rate_stru st_avail_op_rates;
    uint8_t uc_mac_vap_rate_num;
    uint8_t uc_hmac_user_rate_num;
    uint8_t uc_vap_rate_idx;
    uint8_t uc_user_rate_idx;
    uint8_t uc_user_avail_rate_idx = 0;
    uint8_t vap_rate, user_rate;

    /* 获取VAP和USER速率的结构体指针 */
    pst_mac_user = &(pst_hmac_user->st_user_base_info);
    pst_mac_vap_rate = &(pst_mac_vap->st_curr_sup_rates);
    pst_hmac_user_rate = &(pst_hmac_user->st_op_rates);
    memset_s((uint8_t *)(&st_avail_op_rates), sizeof(mac_rate_stru), 0, sizeof(mac_rate_stru));

    uc_mac_vap_rate_num = pst_mac_vap_rate->st_rate.uc_rs_nrates;
    uc_hmac_user_rate_num = pst_hmac_user_rate->uc_rs_nrates;

    for (uc_vap_rate_idx = 0; uc_vap_rate_idx < uc_mac_vap_rate_num; uc_vap_rate_idx++) {
        vap_rate = pst_mac_vap_rate->st_rate.ast_rs_rates[uc_vap_rate_idx].uc_mac_rate;
        if (vap_rate <= 0) {
            continue;
        }
        for (uc_user_rate_idx = 0; uc_user_rate_idx < uc_hmac_user_rate_num; uc_user_rate_idx++) {
            user_rate = pst_hmac_user_rate->auc_rs_rates[uc_user_rate_idx];
            if (hmac_is_equal_rates(vap_rate, user_rate) == OAL_TRUE) {
                /* 速率相同时，如果BIT7基础速率不同，则BIT7置0，仅取速率值 */
                st_avail_op_rates.auc_rs_rates[uc_user_avail_rate_idx] =
                    (vap_rate == user_rate) ? (vap_rate) : (vap_rate & 0x7F);
                uc_user_avail_rate_idx++;
                st_avail_op_rates.uc_rs_nrates++;
            }
        }
    }

    mac_user_set_avail_op_rates(pst_mac_user, st_avail_op_rates.uc_rs_nrates, st_avail_op_rates.auc_rs_rates);
    oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
        "{hmac_vap_set_user_avail_rates::uc_avail_op_rates_num=[%d].}", st_avail_op_rates.uc_rs_nrates);
}

/*
 *   功    能: 计算最大AMPDU长度
  */
void hmac_calc_tx_ampdu_maxsize(mac_user_stru *mac_user, wlan_protocol_enum_uint8 protocol_mode)
{
    if (protocol_mode == WLAN_HT_MODE) {
        /* HT(0-3): maximum length of A-MPDU = 2 ^ (13 + Maximum A-MPDU Length Exponent) - 1 */
        mac_user->ampdu_maxsize = (uint32_t)(1U << (13 + mac_user->st_ht_hdl.uc_max_rx_ampdu_factor)) - 1;
    } else if (protocol_mode == WLAN_VHT_MODE) {
        /* VHT(0-7): maximum length of A-MPDU = 2 ^ (13 + Maximum A-MPDU Length Exponent) - 1 */
        mac_user->ampdu_maxsize = (uint32_t)(1U << (13 + mac_user->st_vht_hdl.bit_max_ampdu_len_exp)) - 1;
    } else if (protocol_mode == WLAN_HE_MODE) {
        /* HE(0-7): maximum length of A-MPDU = 2 ^ (20 + Maximum A-MPDU Length Exponent) - 1 */
        mac_user->ampdu_maxsize = (uint32_t)(1U << (20 +
            mac_user->st_he_hdl.st_he_cap_ie.st_he_mac_cap.bit_max_ampdu_length_exponent)) - 1;
        mac_user->ampdu_maxsize = oal_min(mac_user->ampdu_maxsize, 6500631); // 6500631: the upper bound of A-MPDU size
    } else {
        mac_user->ampdu_maxsize = 65535; // 65535: default max A-MPDU size
    }
    oam_warning_log2(0, OAM_SF_ANY, "{tx_ampdu_maxsize::protocol[%d] len[%d].}",
        protocol_mode, mac_user->ampdu_maxsize);
}

static void hmac_proc_ht_txbf_ie(mac_user_ht_hdl_stru *pst_ht_hdl, uint8_t *puc_ht_cap_ie, uint16_t us_offset)
{
    uint32_t tmp_txbf_elem;
    uint16_t us_tmp_info_elem, us_tmp_txbf_low;

    /********************************************/
    /*  解析 Tx Beamforming Field               */
    /********************************************/
    us_tmp_info_elem = oal_make_word16(puc_ht_cap_ie[us_offset], puc_ht_cap_ie[us_offset + 1]);
    us_tmp_txbf_low = oal_make_word16(puc_ht_cap_ie[us_offset + NUM_2_BITS],
        puc_ht_cap_ie[us_offset + NUM_3_BITS]);
    tmp_txbf_elem = oal_make_word32(us_tmp_info_elem, us_tmp_txbf_low);
    pst_ht_hdl->bit_imbf_receive_cap = (tmp_txbf_elem & BIT0);
    pst_ht_hdl->bit_receive_staggered_sounding_cap = ((tmp_txbf_elem & BIT1) >> NUM_1_BITS);
    pst_ht_hdl->bit_transmit_staggered_sounding_cap = ((tmp_txbf_elem & BIT2) >> NUM_2_BITS);
    pst_ht_hdl->bit_receive_ndp_cap = ((tmp_txbf_elem & BIT3) >> NUM_3_BITS);
    pst_ht_hdl->bit_transmit_ndp_cap = ((tmp_txbf_elem & BIT4) >> NUM_4_BITS);
    pst_ht_hdl->bit_imbf_cap = ((tmp_txbf_elem & BIT5) >> NUM_5_BITS);
    pst_ht_hdl->bit_calibration = ((tmp_txbf_elem & 0x000000C0) >> NUM_6_BITS);
    pst_ht_hdl->bit_exp_csi_txbf_cap = ((tmp_txbf_elem & BIT8) >> NUM_8_BITS);
    pst_ht_hdl->bit_exp_noncomp_txbf_cap = ((tmp_txbf_elem & BIT9) >> NUM_9_BITS);
    pst_ht_hdl->bit_exp_comp_txbf_cap = ((tmp_txbf_elem & BIT10) >> NUM_10_BITS);
    pst_ht_hdl->bit_exp_csi_feedback = ((tmp_txbf_elem & 0x00001800) >> NUM_11_BITS);
    pst_ht_hdl->bit_exp_noncomp_feedback = ((tmp_txbf_elem & 0x00006000) >> NUM_13_BITS);
    pst_ht_hdl->bit_exp_comp_feedback = ((tmp_txbf_elem & 0x0001C000) >> NUM_15_BITS);
    pst_ht_hdl->bit_min_grouping = ((tmp_txbf_elem & 0x00060000) >> NUM_17_BITS);
    pst_ht_hdl->bit_csi_bfer_ant_number = ((tmp_txbf_elem & 0x001C0000) >> NUM_19_BITS);
    pst_ht_hdl->bit_noncomp_bfer_ant_number = ((tmp_txbf_elem & 0x00600000) >> NUM_21_BITS);
    pst_ht_hdl->bit_comp_bfer_ant_number = ((tmp_txbf_elem & 0x01C00000) >> NUM_23_BITS);
    pst_ht_hdl->bit_csi_bfee_max_rows = ((tmp_txbf_elem & 0x06000000) >> NUM_25_BITS);
    pst_ht_hdl->bit_channel_est_cap = ((tmp_txbf_elem & 0x18000000) >> NUM_27_BITS);
}
/*
 * 1.日    期  : 2015年6月8日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_proc_ht_cap_ie(mac_vap_stru *pst_mac_vap, mac_user_stru *pst_mac_user, uint8_t *puc_ht_cap_ie)
{
    uint8_t uc_mcs_bmp_index;
    mac_user_ht_hdl_stru *pst_ht_hdl = NULL;
    uint16_t us_ht_cap_info;
    uint16_t us_offset = 0;

    if (oal_any_null_ptr3(pst_mac_vap, pst_mac_user, puc_ht_cap_ie)) {
        oam_warning_log0(0, OAM_SF_ROAM, "{hmac_proc_ht_cap_ie::mac_vap or mac_user or cap_ie is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 至少支持11n才进行后续的处理 */
    if (mac_mib_get_HighThroughputOptionImplemented(pst_mac_vap) == OAL_FALSE) {
        return OAL_SUCC;
    }

    mac_user_set_ht_capable(pst_mac_user, OAL_TRUE);

    pst_ht_hdl = &pst_mac_user->st_ht_hdl;

    /* 带有 HT Capability Element 的 AP，标示它具有HT capable. */
    pst_ht_hdl->en_ht_capable = OAL_TRUE;

    us_offset += MAC_IE_HDR_LEN;

    /********************************************/
    /*     解析 HT Capabilities Info Field      */
    /********************************************/
    us_ht_cap_info = oal_make_word16(puc_ht_cap_ie[us_offset], puc_ht_cap_ie[us_offset + 1]);

    /* 检查STA所支持的LDPC编码能力 B0，0:不支持，1:支持 */
    pst_ht_hdl->bit_ldpc_coding_cap = (us_ht_cap_info & BIT0);

    /* 提取AP所支持的带宽能力  */
    pst_ht_hdl->bit_supported_channel_width = ((us_ht_cap_info & BIT1) >> 1);

    /* 检查空间复用节能模式 B2~B3 */
    mac_ie_proc_sm_power_save_field(pst_mac_user, (uint8_t)((us_ht_cap_info & (BIT3 | BIT2)) >> 2));

    /* 提取AP支持Greenfield情况 */
    pst_ht_hdl->bit_ht_green_field = ((us_ht_cap_info & BIT4) >> NUM_4_BITS);

    /* 提取AP支持20MHz Short-GI情况 */
    pst_ht_hdl->bit_short_gi_20mhz = ((us_ht_cap_info & BIT5) >> NUM_5_BITS);

    /* 提取AP支持40MHz Short-GI情况 */
    pst_ht_hdl->bit_short_gi_40mhz = ((us_ht_cap_info & BIT6) >> NUM_6_BITS);

    /* 提取AP支持STBC PPDU情况 */
    pst_ht_hdl->bit_rx_stbc = (uint8_t)((us_ht_cap_info & (BIT9 | BIT8)) >> NUM_8_BITS);

    /* 提取AP 40M上DSSS/CCK的支持情况 */
    pst_ht_hdl->bit_dsss_cck_mode_40mhz = ((us_ht_cap_info & BIT12) >> NUM_12_BITS);

    /* 提取AP L-SIG TXOP 保护的支持情况 */
    pst_ht_hdl->bit_lsig_txop_protection = ((us_ht_cap_info & BIT15) >> NUM_15_BITS);

    us_offset += MAC_HT_CAPINFO_LEN;

    /********************************************/
    /*     解析 A-MPDU Parameters Field         */
    /********************************************/
    /* 提取 Maximum Rx A-MPDU factor (B1 - B0) */
    pst_ht_hdl->uc_max_rx_ampdu_factor = (puc_ht_cap_ie[us_offset] & 0x03);
    /* 提取 Minmum Rx A-MPDU factor (B3 - B2) */
    pst_ht_hdl->uc_min_mpdu_start_spacing = (puc_ht_cap_ie[us_offset] >> 2) & 0x07;
    hmac_calc_tx_ampdu_maxsize(pst_mac_user, WLAN_HT_MODE);

    us_offset += MAC_HT_AMPDU_PARAMS_LEN;

    /********************************************/
    /*     解析 Supported MCS Set Field         */
    /********************************************/
    for (uc_mcs_bmp_index = 0; uc_mcs_bmp_index < WLAN_HT_MCS_BITMASK_LEN; uc_mcs_bmp_index++) {
        pst_ht_hdl->uc_rx_mcs_bitmask[uc_mcs_bmp_index] =
            (mac_mib_get_SupportedMCSTxValue(pst_mac_vap, uc_mcs_bmp_index)) &
           (*(uint8_t *)(puc_ht_cap_ie + us_offset + uc_mcs_bmp_index));
    }

    pst_ht_hdl->uc_rx_mcs_bitmask[WLAN_HT_MCS_BITMASK_LEN - 1] &= 0x1F;

    us_offset += MAC_HT_SUP_MCS_SET_LEN;

    /********************************************/
    /* 解析 HT Extended Capabilities Info Field */
    /********************************************/
    us_ht_cap_info = oal_make_word16(puc_ht_cap_ie[us_offset], puc_ht_cap_ie[us_offset + 1]);

    /* 提取 HTC support Information */
    pst_ht_hdl->uc_htc_support = ((us_ht_cap_info & BIT10) >> NUM_10_BITS);

    us_offset += MAC_HT_EXT_CAP_LEN;
    hmac_proc_ht_txbf_ie(pst_ht_hdl, puc_ht_cap_ie, us_offset);

    return OAL_SUCC;
}


static void hmac_proc_vht_cap_info_field(
    mac_vap_stru *mac_vap, hmac_user_stru *hmac_user, mac_vht_hdl_stru *vht_hdl, uint8_t *vht_cap_ie, uint16_t msg_idx)
{
    uint16_t vht_cap_filed_low = oal_make_word16(vht_cap_ie[msg_idx], vht_cap_ie[msg_idx + 1]);
    uint16_t vht_cap_filed_high = oal_make_word16(vht_cap_ie[msg_idx + NUM_2_BITS], vht_cap_ie[msg_idx + NUM_3_BITS]);
    uint32_t vht_cap_field = oal_make_word32(vht_cap_filed_low, vht_cap_filed_high);

    /* 解析VHT capablities info field */
    /* 解析max_mpdu_length 参见11ac协议 Table 8-183u */
    vht_hdl->bit_max_mpdu_length = (vht_cap_field & (BIT1 | BIT0));
    vht_hdl->us_max_mpdu_length = mac_ie_get_max_mpdu_len_by_vht_cap(vht_hdl->bit_max_mpdu_length);
    hmac_user_set_amsdu_level_by_max_mpdu_len(hmac_user, vht_hdl->us_max_mpdu_length);

    /* 解析supported_channel_width */
    vht_hdl->bit_supported_channel_width = ((vht_cap_field & (BIT3 | BIT2)) >> NUM_2_BITS);

    /* 解析rx_ldpc */
    vht_hdl->bit_rx_ldpc = ((vht_cap_field & BIT4) >> NUM_4_BITS);

    /* 解析short_gi_80mhz和short_gi_160mhz支持情况 */
    vht_hdl->bit_short_gi_80mhz = ((vht_cap_field & BIT5) >> NUM_5_BITS);
    vht_hdl->bit_short_gi_80mhz &= mac_mib_get_VHTShortGIOptionIn80Implemented(mac_vap);

    vht_hdl->bit_short_gi_160mhz = ((vht_cap_field & BIT6) >> NUM_6_BITS);
    vht_hdl->bit_short_gi_160mhz &= mac_mib_get_VHTShortGIOptionIn160and80p80Implemented(mac_vap);

    /* 解析tx_stbc 和rx_stbc */
    vht_hdl->bit_tx_stbc = ((vht_cap_field & BIT7) >> NUM_7_BITS);
    vht_hdl->bit_rx_stbc = ((vht_cap_field & (BIT10 | BIT9 | BIT8)) >> NUM_8_BITS);

    /* 解析su_beamformer_cap和su_beamformee_cap */
    vht_hdl->bit_su_beamformer_cap = ((vht_cap_field & BIT11) >> NUM_11_BITS);
    vht_hdl->bit_su_beamformee_cap = ((vht_cap_field & BIT12) >> NUM_12_BITS);

    /* 解析num_bf_ant_supported */
    vht_hdl->bit_num_bf_ant_supported = ((vht_cap_field & (BIT15 | BIT14 | BIT13)) >> NUM_13_BITS);

    /* 以对端天线数初始化可用最大空间流 */
    hmac_user->st_user_base_info.en_avail_bf_num_spatial_stream = vht_hdl->bit_num_bf_ant_supported;

    /* 解析num_sounding_dim */
    vht_hdl->bit_num_sounding_dim = ((vht_cap_field & (BIT18 | BIT17 | BIT16)) >> NUM_16_BITS);

    /* 解析mu_beamformer_cap和mu_beamformee_cap */
    vht_hdl->bit_mu_beamformer_cap = ((vht_cap_field & BIT19) >> NUM_19_BITS);
    vht_hdl->bit_mu_beamformee_cap = ((vht_cap_field & BIT20) >> NUM_20_BITS);

    /* 解析vht_txop_ps */
    vht_hdl->bit_vht_txop_ps = ((vht_cap_field & BIT21) >> NUM_21_BITS);
#ifdef _PRE_WLAN_FEATURE_TXOPPS
    if (vht_hdl->bit_vht_txop_ps == OAL_TRUE && OAL_TRUE == mac_mib_get_txopps(mac_vap)) {
        mac_vap_set_txopps(mac_vap, OAL_TRUE);
    }
#endif
    /* 解析htc_vht_capable */
    vht_hdl->bit_htc_vht_capable = ((vht_cap_field & BIT22) >> NUM_22_BITS);

    /* 解析max_ampdu_len_exp */
    vht_hdl->bit_max_ampdu_len_exp = ((vht_cap_field & (BIT25 | BIT24 | BIT23)) >> NUM_23_BITS);

    /* 解析vht_link_adaptation */
    vht_hdl->bit_vht_link_adaptation = ((vht_cap_field & (BIT27 | BIT26)) >> NUM_26_BITS);

    /* 解析rx_ant_pattern */
    vht_hdl->bit_rx_ant_pattern = ((vht_cap_field & BIT28) >> NUM_28_BITS);

    /* 解析tx_ant_pattern */
    vht_hdl->bit_tx_ant_pattern = ((vht_cap_field & BIT29) >> NUM_29_BITS);

    /* 解析extend_nss_bw_supp */
    vht_hdl->bit_extend_nss_bw_supp = ((vht_cap_field & (BIT31 | BIT30)) >> NUM_30_BITS);
}

static uint32_t hmac_proc_vht_rx_mcs_map_field(mac_vht_hdl_stru *vht_hdl, uint8_t *vht_cap_ie, uint16_t msg_idx)
{
    /* 解析rx_mcs_map;  :修改rx mcs map解析错误问题 */
    uint16_t rx_mcs_map = oal_make_word16(vht_cap_ie[msg_idx], vht_cap_ie[msg_idx + 1]);

    return memcpy_s(&(vht_hdl->st_rx_max_mcs_map), sizeof(mac_rx_max_mcs_map_stru),
                    &rx_mcs_map, sizeof(mac_rx_max_mcs_map_stru)) == EOK ? OAL_SUCC : OAL_FAIL;
}

static void hmac_proc_vht_rx_highest_supp_longgi_data(mac_vht_hdl_stru *vht_hdl, uint8_t *vht_cap_ie, uint16_t msg_idx)
{
    /* 解析rx_highest_supp_logGi_data */
    vht_hdl->bit_rx_highest_rate = oal_make_word16(vht_cap_ie[msg_idx], vht_cap_ie[msg_idx + 1]) & (0x1FFF);
}

static uint32_t hmac_proc_vht_tx_mcs_map_field(mac_vht_hdl_stru *vht_hdl, uint8_t *vht_cap_ie, uint16_t msg_idx)
{
    /* 解析tx_mcs_map */
    uint16_t tx_mcs_map = oal_make_word16(vht_cap_ie[msg_idx], vht_cap_ie[msg_idx + 1]);

    return memcpy_s(&(vht_hdl->st_tx_max_mcs_map), sizeof(mac_tx_max_mcs_map_stru),
                    &tx_mcs_map, sizeof(mac_tx_max_mcs_map_stru)) == EOK ? OAL_SUCC : OAL_FAIL;
}

static void hmac_proc_vht_tx_highest_supp_longgi_data(mac_vht_hdl_stru *vht_hdl, uint8_t *vht_cap_ie, uint16_t msg_idx)
{
    /* 解析tx_highest_supp_logGi_data */
    vht_hdl->bit_tx_highest_rate = oal_make_word16(vht_cap_ie[msg_idx], vht_cap_ie[msg_idx + 1]) & (0x1FFF);
}

/*
 * 1.日    期  : 2013年8月8日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_proc_vht_cap_ie(mac_vap_stru *mac_vap, hmac_user_stru *hmac_user, uint8_t *vht_cap_ie)
{
    mac_vht_hdl_stru vht_hdl = { 0 };
    uint16_t msg_idx = 0;

    /* 解析vht cap IE */
    if (oal_any_null_ptr3(mac_vap, hmac_user, vht_cap_ie)) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_proc_vht_cap_ie::mac_vap or hmac_user or vht_cap_ie is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (vht_cap_ie[1] < MAC_VHT_CAP_IE_LEN) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_proc_vht_cap_ie::invalid vht cap ie len[%d].}", vht_cap_ie[1]);
        return OAL_FAIL;
    }

    /* 支持11ac，才进行后续的处理 */
    if (!mac_mib_get_VHTOptionImplemented(mac_vap)) {
        return OAL_SUCC;
    }

    mac_user_get_vht_hdl(&hmac_user->st_user_base_info, &vht_hdl);

    /* 进入此函数代表user支持11ac */
    vht_hdl.en_vht_capable = OAL_TRUE;

#ifdef _PRE_WLAN_FEATURE_11AC2G
    /* 定制化实现如果不支持11ac2g模式，则关掉vht cap */
    if ((mac_vap->st_cap_flag.bit_11ac2g == OAL_FALSE) && (mac_vap->st_channel.en_band == WLAN_BAND_2G)) {
        vht_hdl.en_vht_capable = OAL_FALSE;
    }
#endif

    msg_idx += MAC_IE_HDR_LEN;
    hmac_proc_vht_cap_info_field(mac_vap, hmac_user, &vht_hdl, vht_cap_ie, msg_idx);

    msg_idx += MAC_VHT_CAP_INFO_FIELD_LEN;
    if (hmac_proc_vht_rx_mcs_map_field(&vht_hdl, vht_cap_ie, msg_idx) != OAL_SUCC) {
        oam_error_log0(0, 0, "{hmac_proc_vht_cap_ie::rx mcs map field proc failed}");
        return OAL_FAIL;
    }

    msg_idx += MAC_VHT_CAP_RX_MCS_MAP_FIELD_LEN;
    hmac_proc_vht_rx_highest_supp_longgi_data(&vht_hdl, vht_cap_ie, msg_idx);

    msg_idx += MAC_VHT_CAP_RX_HIGHEST_DATA_FIELD_LEN;
    if (hmac_proc_vht_tx_mcs_map_field(&vht_hdl, vht_cap_ie, msg_idx) != OAL_SUCC) {
        oam_error_log0(0, 0, "{hmac_proc_vht_cap_ie::tx mcs map field proc failed}");
        return OAL_FAIL;
    }

    msg_idx += MAC_VHT_CAP_TX_MCS_MAP_FIELD_LEN;
    hmac_proc_vht_tx_highest_supp_longgi_data(&vht_hdl, vht_cap_ie, msg_idx);

    mac_user_set_vht_hdl(&hmac_user->st_user_base_info, &vht_hdl);
    hmac_calc_tx_ampdu_maxsize(&hmac_user->st_user_base_info, WLAN_VHT_MODE);
    return OAL_SUCC;
}
/*
 * 功能描述  : 将对端速率集中的速率记录到user中，并去除重复速率,当长度超出限制时告警
 * 1.日    期  : 2018年1月4日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void hmac_add_and_clear_repeat_op_rates(uint8_t *puc_ie_rates,
    uint8_t uc_ie_num_rates, hmac_rate_stru *pst_op_rates)
{
    uint8_t uc_ie_rates_idx, uc_user_rates_idx;

    for (uc_ie_rates_idx = 0; uc_ie_rates_idx < uc_ie_num_rates; uc_ie_rates_idx++) {
        /* 判断该速率是否已经记录在op中 */
        for (uc_user_rates_idx = 0; uc_user_rates_idx < pst_op_rates->uc_rs_nrates; uc_user_rates_idx++) {
            if (hmac_is_equal_rates(puc_ie_rates[uc_ie_rates_idx], pst_op_rates->auc_rs_rates[uc_user_rates_idx])) {
                break;
            }
        }

        /* 相等时，说明ie中的速率与op中的速率都不相同，可以加入op的速率集中 */
        if (uc_user_rates_idx == pst_op_rates->uc_rs_nrates) {
            /* 当长度超出限制时告警，不加入op rates中 */
            if (pst_op_rates->uc_rs_nrates == WLAN_USER_MAX_SUPP_RATES) {
                oam_warning_log0(0, OAM_SF_ANY,
                    "{hmac_add_and_clear_repeat_op_rates::user option rates more then WLAN_USER_MAX_SUPP_RATES.}");
                break;
            }
            pst_op_rates->auc_rs_rates[pst_op_rates->uc_rs_nrates++] = puc_ie_rates[uc_ie_rates_idx];
        }
    }
}

OAL_STATIC void hmac_process_rates_ie_in_assoc_frame(
    mac_vap_stru *mac_vap, hmac_user_stru *hmac_user, uint8_t *payload, uint32_t payload_len)
{
    uint8_t *rates_ie = NULL;
    uint8_t rates_num;
    rates_ie = mac_find_ie(MAC_EID_RATES, payload, (int32_t)payload_len);
    if (rates_ie == NULL) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_ANY,
            "{hmac_process_rates_ie_in_assoc_frame::unsupport basic rates vap mode[%d]}", mac_vap->en_vap_mode);
        return;
    }
    rates_num = rates_ie[1];
    /* 判断supported rates长度，当长度为0时告警，当长度超出限制时，在hmac_add_and_clear_repeat_op_rates里告警 */
    if (rates_num < MAC_MIN_RATES_LEN) {
        oam_warning_log2(mac_vap->uc_vap_id, OAM_SF_ANY,
            "{hmac_process_rates_ie_in_assoc_frame::supported rates length less then MAC_MIN_RATES_LEN \
            vap mode[%d] rates_num[%d]}", mac_vap->en_vap_mode, rates_num);
    } else {
        hmac_add_and_clear_repeat_op_rates(rates_ie + MAC_IE_HDR_LEN, rates_num, &(hmac_user->st_op_rates));
    }
}
OAL_STATIC void hmac_process_ext_rates_ie_in_assoc_frame(
    mac_vap_stru *mac_vap, hmac_user_stru *hmac_user, uint8_t *payload, uint32_t payload_len)
{
    uint8_t *ext_rates_ie = NULL;
    uint8_t ext_rates_num;
    ext_rates_ie = mac_find_ie(MAC_EID_XRATES, payload, (int32_t)payload_len);
    if (ext_rates_ie == NULL) {
        if (mac_vap->st_channel.en_band == WLAN_BAND_2G) {
            /* ，5G不存在扩展速率 */
            oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_ANY,
                "{hmac_process_ext_rates_ie_in_assoc_frame::unsupport xrates vap mode[%d]}", mac_vap->en_vap_mode);
        }
        return;
    }
    ext_rates_num = ext_rates_ie[1];
    /* AP模式下，协议大于11a且小等11G时，不允许对端携带拓展速率集，STA模式下不做限制 */
    if ((mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_AP) &&
        ((mac_vap->en_protocol <= WLAN_LEGACY_11G_MODE) && (mac_vap->en_protocol > WLAN_LEGACY_11A_MODE))) {
        oam_warning_log2(mac_vap->uc_vap_id, OAM_SF_ANY,
            "{hmac_process_ext_rates_ie_in_assoc_frame::invaild xrates rate vap protocol[%d] ext_rates_num[%d]}",
            mac_vap->en_protocol, ext_rates_num);
    } else {
        if (ext_rates_num < MAC_MIN_XRATE_LEN) {
            oam_warning_log2(mac_vap->uc_vap_id, OAM_SF_ANY,
                "{hmac_process_ext_rates_ie_in_assoc_frame::extended supported rates length less then \
                MAC_MIN_RATES_LEN vap mode[%d] num_rates[%d]}", mac_vap->en_vap_mode, ext_rates_num);
        } else {
            hmac_add_and_clear_repeat_op_rates(ext_rates_ie + MAC_IE_HDR_LEN, ext_rates_num, &hmac_user->st_op_rates);
        }
    }
}
/*
 * 1.日    期  : 2013年11月27日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_ie_proc_assoc_user_legacy_rate(uint8_t *puc_payload,
    uint32_t us_rx_len, hmac_user_stru *pst_hmac_user)
{
    mac_user_stru *pst_mac_user = NULL;
    mac_vap_stru *pst_mac_vap = NULL;
    if ((puc_payload == NULL) || (pst_hmac_user == NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "hmac_ie_proc_assoc_user_legacy_rate:params is NULL");
        return OAL_ERR_CODE_PTR_NULL;
    }
    pst_mac_user = &(pst_hmac_user->st_user_base_info);
    pst_mac_vap = mac_res_get_mac_vap(pst_mac_user->uc_vap_id);
    if (pst_mac_vap == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_ie_proc_assoc_user_legacy_rate::pst_mac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    /* user的可选速率集此时应该为空 */
    if (pst_hmac_user->st_op_rates.uc_rs_nrates != 0) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                         "{hmac_ie_proc_assoc_user_legacy_rate::the number of user option rates is not 0.}");
    }
    /* 1.处理基础速率集 */
    hmac_process_rates_ie_in_assoc_frame(pst_mac_vap, pst_hmac_user, puc_payload, us_rx_len);
    /* 2.处理扩展梳理集 */
    hmac_process_ext_rates_ie_in_assoc_frame(pst_mac_vap, pst_hmac_user, puc_payload, us_rx_len);
    if (pst_hmac_user->st_op_rates.uc_rs_nrates == 0) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_ie_proc_assoc_user_legacy_rate::rate is 0.}");
        return OAL_FAIL;
    }
    return OAL_SUCC;
}
