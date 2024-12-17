/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : mac_frame_inl头文件
 * 作    者 : wifi1
 * 创建日期 : 2020年10月16日
 */

#ifndef MAC_FRAME_INL_H
#define MAC_FRAME_INL_H

#include "mac_frame.h"
#include "wlan_spec.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_MAC_FRAME_INL_H

OAL_STATIC OAL_INLINE uint8_t mac_rsn_ie_akm_match_suites_s(uint32_t *pul_suites,
                                                            uint8_t uc_cipher_len,
                                                            uint32_t aul_akm_suite)
{
    uint8_t uc_idx_peer;

    for (uc_idx_peer = 0; uc_idx_peer < uc_cipher_len / sizeof(uint32_t); uc_idx_peer++) {
        if (pul_suites[uc_idx_peer] == aul_akm_suite) {
            return uc_idx_peer;
        }
    }
    return 0xff;
}

/*
 * 函 数 名  : mac_hdr_set_frame_control
 * 功能描述  : This function sets the 'frame control' bits in the MAC header of the
 *             input frame to the given 16-bit value.
 * 1.日    期  : 2012年11月14日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC OAL_INLINE void mac_hdr_set_frame_control(uint8_t *puc_header, uint16_t us_fc)
{
    *(uint16_t *)puc_header = us_fc;
}

/*
 * 函 数 名  : mac_hdr_set_fragment_number
 * 功能描述  : 设置MAC头分片序号字段
 * 1.日    期  : 2013年4月7日
 *   修改内容  : 新生成函数
 */
OAL_STATIC OAL_INLINE void mac_hdr_set_fragment_number(uint8_t *puc_header, uint8_t uc_frag_num)
{
    puc_header[WLAN_HDR_FRAG_OFFSET] &= 0xF0;
    puc_header[WLAN_HDR_FRAG_OFFSET] |= (uc_frag_num & 0x0F);
}

/*
 * 函 数 名  : mac_hdr_set_from_ds
 * 功能描述  : This function sets the 'from ds' bit in the MAC header of the input frame
 *             to the given value stored in the LSB bit.
 *             The bit position of the 'from ds' in the 'frame control field' of the MAC
 *             header is represented by the bit pattern 0x00000010.
 * 1.日    期  : 2012年11月14日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC OAL_INLINE void mac_hdr_set_from_ds(uint8_t *puc_header, uint8_t uc_from_ds)
{
    ((mac_header_frame_control_stru *)(puc_header))->bit_from_ds = uc_from_ds;
}

/*
 * 函 数 名  : mac_hdr_get_from_ds
 * 功能描述  : This function extracts the 'from ds' bit from the MAC header of the input frame.
 *             Returns the value in the LSB of the returned value.
 * 1.日    期  : 2012年11月14日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC OAL_INLINE uint8_t mac_hdr_get_from_ds(uint8_t *puc_header)
{
    return (uint8_t)((mac_header_frame_control_stru *)(puc_header))->bit_from_ds;
}

/*
 * 函 数 名  : mac_hdr_set_to_ds
 * 功能描述  : This function sets the 'to ds' bit in the MAC header of the input frame
 *             to the given value stored in the LSB bit.
 *             The bit position of the 'to ds' in the 'frame control field' of the MAC
 *             header is represented by the bit pattern 0x00000001
 * 1.日    期  : 2012年11月14日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC OAL_INLINE void mac_hdr_set_to_ds(uint8_t *puc_header, uint8_t uc_to_ds)
{
    ((mac_header_frame_control_stru *)(puc_header))->bit_to_ds = uc_to_ds;
}

/*
 * 函 数 名  : mac_hdr_get_to_ds
 * 功能描述  : This function extracts the 'to ds' bit from the MAC header of the input frame.
 *             Returns the value in the LSB of the returned value.
 * 1.日    期  : 2012年11月14日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC OAL_INLINE uint8_t mac_hdr_get_to_ds(uint8_t *puc_header)
{
    return (uint8_t)((mac_header_frame_control_stru *)(puc_header))->bit_to_ds;
}

/*
 * 函 数 名  : mac_get_tid_value
 * 功能描述  : 四地址获取帧头中的tid
 * 1.日    期  : 2013年4月11日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC OAL_INLINE uint8_t mac_get_tid_value(uint8_t *puc_header, oal_bool_enum_uint8 en_is_4addr)
{
    if (en_is_4addr) {
        return (puc_header[MAC_QOS_CTRL_FIELD_OFFSET_4ADDR] & 0x07); /* B0 - B2 */
    } else {
        return (puc_header[MAC_QOS_CTRL_FIELD_OFFSET] & 0x07); /* B0 - B2 */
    }
}

/*
 * 函 数 名  : mac_get_seq_num
 * 功能描述  : 获取接受侦的seqence number
 * 1.日    期  : 2013年4月11日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC OAL_INLINE uint16_t mac_get_seq_num(uint8_t *puc_header)
{
    uint16_t us_seq_num;

    us_seq_num = puc_header[BYTE_OFFSET_23];
    us_seq_num <<= BIT_OFFSET_4;
    us_seq_num |= (puc_header[BYTE_OFFSET_22] >> BIT_OFFSET_4);

    return us_seq_num;
}

/*
 * 函 数 名  : mac_get_bar_start_seq_num
 * 功能描述  : 获取BAR帧中的start seq num值
 * 1.日    期  : 2013年5月2日
 *   修改内容  : 新生成函数
 */
OAL_STATIC OAL_INLINE uint16_t mac_get_bar_start_seq_num(uint8_t *puc_payload)
{
    return ((puc_payload[BYTE_OFFSET_2] & 0xF0) >> BIT_OFFSET_4) | (puc_payload[BYTE_OFFSET_3] << BIT_OFFSET_4);
}

/*
 * 函 数 名  : mac_rx_get_da
 * 功能描述  : 获取收到的帧的目的地址
 *             参考协议 <802.11权威指南> 81页
 * 1.日    期  : 2012年12月10日
 *   修改内容  : 新生成函数
 */
OAL_STATIC OAL_INLINE void mac_rx_get_da(mac_ieee80211_frame_stru *pst_mac_header,
                                         uint8_t **puc_da)
{
    /* IBSS、from AP */
    if (pst_mac_header->st_frame_control.bit_to_ds == 0) {
        *puc_da = pst_mac_header->auc_address1;
    /* WDS、to AP */
    } else {
        *puc_da = pst_mac_header->auc_address3;
    }
}

/*
 * 函 数 名  : mac_rx_get_sa
 * 功能描述  : 获取收到的帧的源地址
 *             参考协议 <802.11权威指南> 81页
 * 1.日    期  : 2012年12月11日
 *   修改内容  : 新生成函数
 */
OAL_STATIC OAL_INLINE void mac_rx_get_sa(mac_ieee80211_frame_stru *pst_mac_header,
                                         uint8_t **puc_sa)
{
    /* IBSS、to AP */
    if (pst_mac_header->st_frame_control.bit_from_ds == 0) {
        *puc_sa = pst_mac_header->auc_address2;
    /* from AP */
    } else if ((pst_mac_header->st_frame_control.bit_from_ds == 1) &&
             (pst_mac_header->st_frame_control.bit_to_ds == 0)) {
        *puc_sa = pst_mac_header->auc_address3;
    /* WDS */
    } else {
        *puc_sa = ((mac_ieee80211_frame_addr4_stru *)pst_mac_header)->auc_address4;
    }
}

/*
 * 函 数 名  : mac_get_transmitter_addr
 * 功能描述  : 获取收到的帧的发送端地址
 *             参考协议 <802.11权威指南> 81页
 * 1.日    期  : 2014年1月20日
 *   修改内容  : 新生成函数
 */
OAL_STATIC OAL_INLINE void mac_get_transmit_addr(mac_ieee80211_frame_stru *pst_mac_header,
                                                 uint8_t **puc_bssid)
{
    /* 对于IBSS, STA, AP, WDS 场景下，获取发送端地址 */
    *puc_bssid = pst_mac_header->auc_address2;
}

/*
 * 函 数 名  : mac_get_submsdu_len
 * 功能描述  : 获取netbuf中submsdu的长度
 * 1.日    期  : 2012年12月11日
 *   修改内容  : 新生成函数
 */
OAL_STATIC OAL_INLINE void mac_get_submsdu_len(uint8_t *puc_submsdu_hdr, uint16_t *pus_submsdu_len)
{
    *pus_submsdu_len = *(puc_submsdu_hdr + MAC_SUBMSDU_LENGTH_OFFSET);
    *pus_submsdu_len = (uint16_t)((*pus_submsdu_len << BIT_OFFSET_8) +
        *(puc_submsdu_hdr + MAC_SUBMSDU_LENGTH_OFFSET + 1));
}

/*
 * 函 数 名  : mac_get_submsdu_pad_len
 * 功能描述  : 获取submsdu需要填充的字节数
 * 1.日    期  : 2012年12月11日
 *   修改内容  : 新生成函数
 */
OAL_STATIC OAL_INLINE void mac_get_submsdu_pad_len(uint16_t us_msdu_len, uint8_t *puc_submsdu_pad_len)
{
    *puc_submsdu_pad_len = us_msdu_len & 0x3;

    if (*puc_submsdu_pad_len) {
        *puc_submsdu_pad_len = (MAC_BYTE_ALIGN_VALUE - *puc_submsdu_pad_len);
    }
}

/*
 * 函 数 名  : mac_get_frame_sub_type
 * 功能描述  : 获取报文的类型和子类型
 * 1.日    期  : 2013年4月18日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC OAL_INLINE uint8_t mac_get_frame_type_and_subtype(uint8_t *puc_mac_header)
{
    return (puc_mac_header[0] & 0xFC);
}

/*
 * 函 数 名  : mac_get_frame_sub_type
 * 功能描述  : 获取报文的子类型
 * 1.日    期  : 2017年9月11日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC OAL_INLINE uint8_t mac_get_frame_sub_type(uint8_t *puc_mac_header)
{
    return (puc_mac_header[0] & 0xF0);
}

/*
 * 函 数 名  : mac_frame_get_subtype_value
 * 功能描述  : 获取802.11帧子类型的值(0~15)
 *             帧第一个字节的高四位
 * 1.日    期  : 2013年12月9日
 *   修改内容  : 新生成函数
 */
OAL_STATIC OAL_INLINE uint8_t mac_frame_get_subtype_value(uint8_t *puc_mac_header)
{
    return ((puc_mac_header[0] & 0xF0) >> BIT_OFFSET_4);
}

/*
 * 函 数 名  : mac_get_frame_type
 * 功能描述  : 获取报文类型
 * 1.日    期  : 2013年9月23日,星期一
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC OAL_INLINE uint8_t mac_get_frame_type(uint8_t *puc_mac_header)
{
    return (puc_mac_header[0] & 0x0C);
}

/*
 * 函 数 名  : mac_frame_get_type_value
 * 功能描述  : 获取80211帧帧类型，取值0~2
 * 1.日    期  : 2013年12月9日
 *   修改内容  : 新生成函数
 */
OAL_STATIC OAL_INLINE uint8_t mac_frame_get_type_value(uint8_t *puc_mac_header)
{
    return (puc_mac_header[0] & 0x0C) >> BIT_OFFSET_2;
}

/*
 * 函 数 名  : mac_set_snap
 * 功能描述  : 设置LLC SNAP, TX流程上调用
 * 1.日    期  : 2012年11月14日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC OAL_INLINE void mac_set_snap(oal_netbuf_stru *pst_buf,
                                        uint16_t us_ether_type)
{
    mac_llc_snap_stru *pst_llc;
    uint16_t uc_use_btep1;
    uint16_t uc_use_btep2;

    /* LLC */
    pst_llc = (mac_llc_snap_stru *)(oal_netbuf_data(pst_buf) + ETHER_HDR_LEN - SNAP_LLC_FRAME_LEN);
    pst_llc->uc_llc_dsap = SNAP_LLC_LSAP;
    pst_llc->uc_llc_ssap = SNAP_LLC_LSAP;
    pst_llc->uc_control = LLC_UI;

    uc_use_btep1 = oal_byteorder_host_to_net_uint16(ETHER_TYPE_AARP);
    uc_use_btep2 = oal_byteorder_host_to_net_uint16(ETHER_TYPE_IPX);
    if (oal_unlikely((uc_use_btep1 == us_ether_type) || (uc_use_btep2 == us_ether_type))) {
        pst_llc->auc_org_code[BYTE_OFFSET_0] = SNAP_BTEP_ORGCODE_0; /* 0x0 */
        pst_llc->auc_org_code[BYTE_OFFSET_1] = SNAP_BTEP_ORGCODE_1; /* 0x0 */
        pst_llc->auc_org_code[BYTE_OFFSET_2] = SNAP_BTEP_ORGCODE_2; /* 0xf8 */
    } else {
        pst_llc->auc_org_code[BYTE_OFFSET_0] = SNAP_RFC1042_ORGCODE_0; /* 0x0 */
        pst_llc->auc_org_code[BYTE_OFFSET_1] = SNAP_RFC1042_ORGCODE_1; /* 0x0 */
        pst_llc->auc_org_code[BYTE_OFFSET_2] = SNAP_RFC1042_ORGCODE_2; /* 0x0 */
    }

    pst_llc->us_ether_type = us_ether_type;

    oal_netbuf_pull(pst_buf, (ETHER_HDR_LEN - SNAP_LLC_FRAME_LEN));
}

/*
 * 函 数 名  : mac_get_auth_alg
 * 功能描述  : 获取认证状态字段
 * 1.日    期  : 2015年3月20日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC OAL_INLINE uint16_t mac_get_auth_alg(uint8_t *payload, uint16_t payload_len)
{
    uint16_t auth_alg;
    if (payload_len < MAC_AUTH_ALG_LEN) {
        return 0xFFFF;
    }
    auth_alg = payload[BIT_OFFSET_1];
    auth_alg = (uint16_t)((auth_alg << BIT_OFFSET_8) | payload[BIT_OFFSET_0]);

    return auth_alg;
}

/*
 * 函 数 名  : mac_get_auth_status
 * 功能描述  : 获取认证状态字段
 * 1.日    期  : 2013年6月27日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC OAL_INLINE uint16_t mac_get_auth_status(uint8_t *paylaod, uint16_t payload_len)
{
    uint16_t auth_status;
    if (payload_len < MAC_AUTH_SEQ_LEN + MAC_AUTH_ALG_LEN + MAC_AUTH_STATUS_LEN) {
        return MAC_UNSPEC_FAIL;
    }
    auth_status = paylaod[BYTE_OFFSET_5];
    auth_status = (uint16_t)((auth_status << BIT_OFFSET_8) | paylaod[BYTE_OFFSET_4]);
    return auth_status;
}

/*
 * 函 数 名  : mac_get_ft_status
 * 功能描述  : 获取FT response的status字段
 * 1.日    期  : 2019年3月13日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC OAL_INLINE uint16_t mac_get_ft_status(uint8_t *puc_mac_hdr)
{
    uint16_t us_auth_status;

    /* 读取ft rsp中的ft status, 相对header的偏移量为14个字节 */
    us_auth_status = puc_mac_hdr[MAC_80211_FRAME_LEN + BYTE_OFFSET_15];
    us_auth_status = (uint16_t)((us_auth_status << BIT_OFFSET_8) | puc_mac_hdr[MAC_80211_FRAME_LEN + BYTE_OFFSET_14]);

    return us_auth_status;
}

/*
 * 函 数 名  : mac_get_auth_seq_num
 * 功能描述  : 获取认证帧序列号
 * 1.日    期  : 2013年6月27日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC OAL_INLINE uint16_t mac_get_auth_seq_num(uint8_t *payload, uint16_t paylaod_len)
{
    uint16_t auth_seq;
    if (paylaod_len < (MAC_AUTH_ALG_LEN + MAC_AUTH_SEQ_LEN)) {
        return 0xFFFF;
    }
    auth_seq = payload[BYTE_OFFSET_3];
    auth_seq = (uint16_t)((auth_seq << BIT_OFFSET_8) | payload[BYTE_OFFSET_2]);
    return auth_seq;
}

/*
 * 函 数 名  : mac_set_wep
 * 功能描述  : 设置protected frame subfield
 * 1.日    期  : 2013年6月28日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC OAL_INLINE void mac_set_wep(uint8_t *puc_hdr, uint8_t uc_wep)
{
    puc_hdr[1] &= 0xBF;
    puc_hdr[1] |= (uint8_t)(uc_wep << BIT_OFFSET_6);
}

/*
 * 函 数 名  : mac_set_protectedframe
 * 功能描述  : 设置帧控制字段的受保护字段
 * 1.日    期  : 2014年1月23日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC OAL_INLINE void mac_set_protectedframe(uint8_t *puc_mac_hdr)
{
    puc_mac_hdr[1] |= 0x40;
}
/*
 * 函 数 名  : mac_get_protectedframe
 * 功能描述  : 获取帧头中保护位信息
 * 1.日    期  : 2015年2月10日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_is_protectedframe(uint8_t *puc_mac_hdr)
{
    mac_ieee80211_frame_stru *pst_mac_hdr = NULL;
    pst_mac_hdr = (mac_ieee80211_frame_stru *)puc_mac_hdr;

    return (oal_bool_enum_uint8)(pst_mac_hdr->st_frame_control.bit_protected_frame);
}

/*
 * 函 数 名  : mac_is_4addr
 * 功能描述  : 是否为4地址
 * 1.日    期  : 2015年5月21日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_is_4addr(uint8_t *puc_mac_hdr)
{
    uint8_t uc_is_tods;
    uint8_t uc_is_from_ds;
    oal_bool_enum_uint8 en_is_4addr;

    uc_is_tods = mac_hdr_get_to_ds(puc_mac_hdr);
    uc_is_from_ds = mac_hdr_get_from_ds(puc_mac_hdr);

    en_is_4addr = uc_is_tods && uc_is_from_ds;

    return en_is_4addr;
}

/*
 * 函 数 名  : mac_get_addr1
 * 功能描述  : 拷贝地址1. 非mp13 rom 使用这些带安全函数接口
 * 1.日    期  : 2013年7月2日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC OAL_INLINE void mac_get_addr1(uint8_t *puc_mac_hdr, uint8_t *puc_addr, uint8_t uc_addr_len)
{
    if (memcpy_s(puc_addr, uc_addr_len, puc_mac_hdr + BYTE_OFFSET_4, WLAN_MAC_ADDR_LEN) != EOK) {
        return;
    }
}
/*
 * 函 数 名  : mac_get_address2
 * 功能描述  : 拷贝地址2
 * 1.日    期  : 2013年6月29日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC OAL_INLINE void mac_get_address2(uint8_t *puc_mac_hdr, uint8_t *puc_addr, uint8_t uc_addr_len)
{
    if (memcpy_s(puc_addr, uc_addr_len, puc_mac_hdr + BYTE_OFFSET_10, WLAN_MAC_ADDR_LEN) != EOK) {
        return;
    }
}

/*
 * 函 数 名  : mac_get_address3
 * 功能描述  : 拷贝地址3
 * 1.日    期  : 2013年7月2日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC OAL_INLINE void mac_get_address3(uint8_t *puc_mac_hdr, uint8_t *puc_addr, uint8_t uc_addr_len)
{
    if (memcpy_s(puc_addr, uc_addr_len, puc_mac_hdr + BYTE_OFFSET_16, WLAN_MAC_ADDR_LEN) != EOK) {
        return;
    }
}

/*
 * 函 数 名  : mac_get_qos_ctrl
 * 功能描述  : 获取mac头中的qos ctrl字段
 * 1.日    期  : 2015年5月21日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 *   其    他  : 3地址情况下使用
 */
OAL_STATIC OAL_INLINE void mac_get_qos_ctrl(uint8_t *puc_mac_hdr,
                                            uint8_t *puc_qos_ctrl,
                                            uint8_t uc_qos_ctl_len)
{
    if (OAL_TRUE != mac_is_4addr(puc_mac_hdr)) {
        if (memcpy_s(puc_qos_ctrl, uc_qos_ctl_len, puc_mac_hdr + MAC_QOS_CTRL_FIELD_OFFSET, MAC_QOS_CTL_LEN) != EOK) {
            return;
        }
        return;
    }

    if (memcpy_s(puc_qos_ctrl, uc_qos_ctl_len, puc_mac_hdr + MAC_QOS_CTRL_FIELD_OFFSET_4ADDR, MAC_QOS_CTL_LEN) != EOK) {
        return;
    }
    return;
}

/*
 * 函 数 名  : mac_get_asoc_status
 * 功能描述  : 获取关联帧中的状态信息
 * 1.日    期  : 2013年7月1日
 *   修改内容  : 新生成函数
 */
OAL_STATIC OAL_INLINE mac_status_code_enum_uint16 mac_get_asoc_status(uint8_t *puc_mac_payload)
{
    mac_status_code_enum_uint16 en_asoc_status;

    en_asoc_status = (puc_mac_payload[BYTE_OFFSET_3] << BIT_OFFSET_8) | puc_mac_payload[BYTE_OFFSET_2];

    return en_asoc_status;
}

/*
 * 函 数 名  : mac_get_asoc_status
 * 功能描述  : 获取关联帧中的关联ID
 * 1.日    期  : 2013年7月1日
 *   修改内容  : 新生成函数
 */
OAL_STATIC OAL_INLINE uint16_t mac_get_asoc_id(uint8_t *puc_mac_payload)
{
    uint16_t us_asoc_id;

    us_asoc_id = puc_mac_payload[BYTE_OFFSET_4] | (puc_mac_payload[BYTE_OFFSET_5] << BIT_OFFSET_8);
    us_asoc_id &= 0x3FFF; /* 取低14位 */

    return us_asoc_id;
}

/*
 * 函 数 名  : wlan_get_bssid
 * 功能描述  : 根据"from ds"bit,从帧中提取bssid(mac地址)
 * 1.日    期  : 2013年7月3日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC OAL_INLINE void mac_get_bssid(uint8_t *puc_mac_hdr, uint8_t *puc_bssid, uint8_t uc_addr_len)
{
    if (1 == mac_hdr_get_from_ds(puc_mac_hdr)) {
        mac_get_address2(puc_mac_hdr, puc_bssid, uc_addr_len);
    } else if (1 == mac_hdr_get_to_ds(puc_mac_hdr)) {
        mac_get_addr1(puc_mac_hdr, puc_bssid, uc_addr_len);
    } else {
        mac_get_address3(puc_mac_hdr, puc_bssid, uc_addr_len);
    }
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_frame_is_qos_data(mac_header_frame_control_stru *frame_control)
{
    return ((frame_control->bit_type == WLAN_DATA_BASICTYPE) && (frame_control->bit_sub_type & WLAN_QOS_DATA));
}
#endif /* end of mac_frame.h */
