/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : WPI标准计算mic功能
 */
#if (defined(_PRE_WLAN_FEATURE_WAPI) || defined(_PRE_WLAN_FEATURE_PWL))
#include "hmac_wpi_mic.h"
#include "mac_frame_inl.h"
#include "hmac_sms4.h"

/*
 * 函数功能:为计算mic的数据申请内存
 */
uint8_t *hmac_wpi_mic_data_alloc(oal_bool_enum_uint8 is_qos, uint16_t payload_len, uint16_t *mic_data_len)
{
    uint16_t mic_part1_len;
    uint16_t mic_part2_len;
    uint8_t *mic_data = NULL;

    mic_part1_len = (is_qos == OAL_TRUE) ? SMS4_MIC_PART1_QOS_LEN : SMS4_MIC_PART1_NO_QOS_LEN;

    /* 按照协议，补齐不足位，16字节对齐 */
    mic_part2_len = padding_m(payload_len, SMS4_PADDING_LEN);

    *mic_data_len = mic_part1_len + mic_part2_len;

    mic_data = oal_mem_alloc_m(OAL_MEM_POOL_ID_LOCAL, mic_part1_len + mic_part2_len, OAL_TRUE);
    if (mic_data == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "hmac_mic_data_alloc:alloc fail");
        return NULL;
    }
    return mic_data;
}

/*
 * 函数功能:准备计算mic的数据
 */
uint32_t hmac_prepare_wpi_mic_data(mac_ieee80211_frame_stru *mac_hdr, uint8_t *payload, uint16_t payload_len,
    uint16_t pc, uint8_t *mic_data, uint16_t mic_data_len)
{
    uint8_t is_qos;
    uint8_t *mic_ori = NULL;

    memset_s(mic_data, mic_data_len, 0, mic_data_len);
    mic_ori = mic_data;

    /* frame control */
    if (memcpy_s(mic_data, mic_data_len,
        (uint8_t *)&(mac_hdr->st_frame_control), sizeof(mac_hdr->st_frame_control)) != EOK) {
        return OAL_FAIL;
    }
    mic_data[0] &= ~(BIT4 | BIT5 | BIT6); /* sub type */
    mic_data[1] &= ~(BIT3 | BIT4 | BIT5); /* retry, pwr Mgmt, more data */
    mic_data[1] |= BIT6;

    mic_data += sizeof(mac_hdr->st_frame_control);

    /* addr1 */
    mac_get_addr1((uint8_t *)mac_hdr, mic_data, OAL_MAC_ADDR_LEN);
    mic_data += OAL_MAC_ADDR_LEN;

    /* addr2 */
    mac_get_address2((uint8_t *)mac_hdr, mic_data, OAL_MAC_ADDR_LEN);
    mic_data += OAL_MAC_ADDR_LEN;

    /* 序列控制 */
    memset_s(mic_data, OAL_SEQ_CTL_LEN, 0, OAL_SEQ_CTL_LEN);
    mic_data[0] = (uint8_t)(mac_hdr->bit_frag_num);
    mic_data += BYTE_OFFSET_2;

    /* addr3 */
    mac_get_address3((uint8_t *)mac_hdr, mic_data, OAL_MAC_ADDR_LEN);
    mic_data += OAL_MAC_ADDR_LEN;

    /* 跳过addr4 */
    mic_data += OAL_MAC_ADDR_LEN;

    /* qos ctrl */
    is_qos = mac_frame_is_qos_data(&mac_hdr->st_frame_control);
    if (is_qos == OAL_TRUE) {
        mac_get_qos_ctrl((uint8_t *)mac_hdr, mic_data, MAC_QOS_CTL_LEN);
        mic_data += MAC_QOS_CTL_LEN;
    }
    /* 填充PWL 的PC字段 或 WAPI 的key_index */
    *mic_data = (uint8_t)(pc & 0x00ff);
    *(mic_data + 1) = (uint8_t)((pc & 0xff00) >> BIT_OFFSET_8);
    mic_data += BYTE_OFFSET_2;

    /* 填充payload_len 协议写明大端字节序 */
    *mic_data = (uint8_t)((payload_len & 0xff00) >> BIT_OFFSET_8);
    *(mic_data + 1) = (uint8_t)(payload_len & 0x00ff);
    mic_data += BYTE_OFFSET_2;

    /************填充第2部分*******************/
    mic_ori += ((is_qos == OAL_TRUE) ? SMS4_MIC_PART1_QOS_LEN : SMS4_MIC_PART1_NO_QOS_LEN);
    if (memcpy_s(mic_ori, payload_len, payload, payload_len) != EOK) {
        return OAL_FAIL;
    }
    return OAL_SUCC;
}

/*
 * 功能描述 : 计算mic
 * 日   期 : 2012年5月2日
 * 修改内容 : 新生成函数
 */
uint32_t hmac_wpi_calc_mic(uint8_t *data, uint32_t data_len,
    uint8_t *iv, uint8_t *key, uint8_t *mic, uint8_t mic_len)
{
    uint32_t mic_tmp[WPI_MIC_LEN >> 2]; /* mic按照4字节为单位进行计算 */
    uint32_t loop;
    uint32_t data_group_num = data_len >> 4; /* 数据每16字节一组进行mic计算 */
    uint32_t *in = NULL;
    uint32_t ext_key[RK_LEN] = {0};

    if ((data == NULL) || (iv == NULL) || (key == NULL) ||  (mic == NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "hmac_wpi_calc_mic: params is null");
        return OAL_FAIL;
    }
    if ((data_len == 0) || (data_len > MAX_MIC_LEN) || (mic_len != WPI_MIC_LEN)) {
        oam_error_log2(0, OAM_SF_ANY, "hmac_wpi_calc_mic:data_len[%d], mic_len[%d]", data_len, mic_len);
        return OAL_FAIL;
    }
    hmac_sms4_keyext(key, ext_key, sizeof(ext_key));
    in = (uint32_t *)data;
    hmac_sms4_crypt(iv, (uint8_t *)mic_tmp, ext_key, sizeof(ext_key));

    for (loop = 0; loop < data_group_num ; loop++) {
        mic_tmp[BYTE_OFFSET_0] ^= in[BYTE_OFFSET_0];
        mic_tmp[BYTE_OFFSET_1] ^= in[BYTE_OFFSET_1];
        mic_tmp[BYTE_OFFSET_2] ^= in[BYTE_OFFSET_2];
        mic_tmp[BYTE_OFFSET_3] ^= in[BYTE_OFFSET_3];
        in += SMS4_STEP;
        hmac_sms4_crypt((uint8_t *)mic_tmp, (uint8_t *)mic_tmp, ext_key, sizeof(ext_key));
    }
    if (memcpy_s(mic, WPI_MIC_LEN, mic_tmp, WPI_MIC_LEN) != EOK) {
        return OAL_FAIL;
    }
    return OAL_SUCC;
}
#endif
