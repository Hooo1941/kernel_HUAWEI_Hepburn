/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : wpi数据加解密
 * 作    者 :
 * 创建日期 : 2015年5月20日
 */
#if (defined(_PRE_WLAN_FEATURE_WAPI) || defined(_PRE_WLAN_FEATURE_PWL))
#include "oal_ext_if.h"
#include "oal_types.h"
#include "hmac_sms4.h"
#include "hmac_wpi.h"
#include "hmac_wapi.h"
#include "hmac_wpi_mic.h"
#include "mac_frame_inl.h"
#include "wlan_spec.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_WPI_C

#define IV_LEN         4                      // 初始化向量长度

/*
 * 函 数 名  : hmac_wpi_encrypt
 * 功能描述  : 对pdu数据进行加密处理
 */
uint32_t hmac_wpi_encrypt(uint8_t *iv, uint8_t *bufin, uint32_t buflen, uint8_t *key, uint8_t *bufout)
{
    uint32_t aul_iv_out[IV_LEN];
    uint32_t *pul_in = NULL;
    uint32_t *pul_out = NULL;
    uint8_t *puc_out = NULL;
    uint8_t *puc_in = NULL;
    uint32_t counter;
    uint32_t comp;
    uint32_t loop;
    uint32_t aul_pr_keyin[RK_LEN] = { 0 };

    if (buflen < 1) {
#ifdef WAPI_DEBUG_MODE
        g_stMacDriverStats.ulSms4OfbInParmInvalid++;
#endif
        oam_error_log1(0, OAM_SF_ANY, "hmac_wpi_encrypt:buflen[%d] less than 1", buflen);
        return OAL_FAIL;
    }

    hmac_sms4_keyext(key, aul_pr_keyin, sizeof(aul_pr_keyin));

    counter = buflen / SMS4_MIC_LEN;
    comp = buflen % SMS4_MIC_LEN;

    /* get the iv */
    hmac_sms4_crypt(iv, (uint8_t *)aul_iv_out, aul_pr_keyin, sizeof(aul_pr_keyin));
    pul_in = (uint32_t *)bufin;
    pul_out = (uint32_t *)bufout;

    for (loop = 0; loop < counter; loop++) {
        pul_out[BYTE_OFFSET_0] = pul_in[BYTE_OFFSET_0] ^ aul_iv_out[BYTE_OFFSET_0];
        pul_out[BYTE_OFFSET_1] = pul_in[BYTE_OFFSET_1] ^ aul_iv_out[BYTE_OFFSET_1];
        pul_out[BYTE_OFFSET_2] = pul_in[BYTE_OFFSET_2] ^ aul_iv_out[BYTE_OFFSET_2];
        pul_out[BYTE_OFFSET_3] = pul_in[BYTE_OFFSET_3] ^ aul_iv_out[BYTE_OFFSET_3];
        hmac_sms4_crypt((uint8_t *)aul_iv_out, (uint8_t *)aul_iv_out, aul_pr_keyin, sizeof(aul_pr_keyin));
        pul_in += SMS4_STEP;
        pul_out += SMS4_STEP;
    }

    puc_in = (uint8_t *)pul_in;
    puc_out = (uint8_t *)pul_out;
    iv = (uint8_t *)aul_iv_out;

    for (loop = 0; loop < comp; loop++) {
        puc_out[loop] = puc_in[loop] ^ iv[loop];
    }

    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_wpi_encrypt
 * 功能描述  : 对pdu数据进行解密处理
 */
uint32_t hmac_wpi_decrypt(uint8_t *iv, uint8_t *bufin, uint32_t buflen, uint8_t *key,
    uint8_t *bufout)
{
    return hmac_wpi_encrypt(iv, bufin, buflen, key, bufout);
}

/*
 * 函 数 名  : hmac_wpi_swap_pn
 * 功能描述  : pn对半交换
 */
void hmac_wpi_swap_pn(uint8_t *pn, uint8_t len)
{
    uint8_t index;
    uint8_t temp;
    uint8_t len_tmp;

    len_tmp = len / 2; /* 2表示取一半长度 */
    for (index = 0; index < len_tmp; index++) {
        temp = pn[index];
        pn[index] = pn[len - 1 - index];
        pn[len - 1 - index] = temp;
    }
}

/*
 * 函 数 名  : hmac_wpi_update_rx_ctl
 * 功能描述  : 更新接收的控制信息
 */
OAL_STATIC void hmac_wpi_update_rx_ctl(oal_netbuf_stru *netbuf, mac_rx_ctl_stru *rx_ctl)
{
    mac_get_rx_cb_mac_header_addr(rx_ctl) = (uint32_t *)oal_netbuf_header(netbuf);
    rx_ctl->us_frame_len = (uint16_t)oal_netbuf_len(netbuf);
}

/*
 * 函 数 名  : hmac_wpi_calc_rx_netbuf_mic
 * 功能描述  : 计算接收netbuf的mic
 */
OAL_STATIC uint32_t hmac_wpi_calc_rx_netbuf_mic(oal_netbuf_stru *netbuf, hmac_decrypt_params_stru *decrypt_params,
    uint8_t *mic, uint8_t mic_len, uint16_t pc, uint8_t *pn)
{
    mac_ieee80211_frame_stru *mac_hdr = (mac_ieee80211_frame_stru *)oal_netbuf_data(netbuf);
    mac_rx_ctl_stru *rx_ctl = (mac_rx_ctl_stru *)oal_netbuf_cb(netbuf);
    uint8_t *payload = NULL;
    uint16_t payload_len;
    uint8_t *mic_data = NULL;
    uint16_t mic_data_len = 0;
    uint32_t ret;
    payload = ((uint8_t *)mac_hdr) + rx_ctl->uc_mac_header_len;
    payload_len = (uint16_t)oal_netbuf_len(netbuf) - rx_ctl->uc_mac_header_len;

    /* 为计算mic的数据申请内存 */
    mic_data = hmac_wpi_mic_data_alloc(mac_frame_is_qos_data(&mac_hdr->st_frame_control), payload_len, &mic_data_len);
    if (mic_data == NULL) {
        oam_error_log1(0, OAM_SF_ANY, "hmac_wpi_calc_rx_netbuf_mic:mic_data alloc fail, payload_len[%d]", payload_len);
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }
    /* 为计算mic预备数据 */
    ret = hmac_prepare_wpi_mic_data(mac_hdr, payload, payload_len, pc, mic_data, mic_data_len);
    if (ret != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ANY, "hmac_wpi_calc_rx_netbuf_mic:hmac_prepare_wpi_mic_data fail");
        oal_mem_free_m(mic_data, OAL_TRUE);
        return ret;
    }

    ret = hmac_wpi_calc_mic(mic_data, mic_data_len, pn, decrypt_params->mic_key, mic, mic_len);
    /* 计算完mic后，释放mic data */
    oal_mem_free_m(mic_data, OAL_TRUE);
    return ret;
}

/*
 * 函 数 名  : hmac_wpi_calc_tx_netbuf_mic
 * 功能描述  : 计算发送netbuf的mic
 * 说明      : 形参(encrypt_params->pc)对于WAPI是keyIndex，对于PWL是pc,准备数据的第四个入参
 */
OAL_STATIC uint32_t hmac_wpi_calc_tx_netbuf_mic(
    oal_netbuf_stru *netbuf, uint8_t *pn, hmac_encrypt_params_stru *encrypt_params, uint8_t *mic, uint8_t mic_len)
{
    mac_tx_ctl_stru *tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(netbuf);
    uint8_t *payload = oal_netbuf_data(netbuf);
    uint16_t payload_len = mac_get_cb_mpdu_len(tx_ctl);
    mac_ieee80211_frame_stru *mac_hdr = mac_get_cb_frame_header_addr(tx_ctl);
    uint8_t *mic_data = NULL;
    uint16_t mic_data_len = 0;
    uint32_t ret;
    /* 为计算mic的数据申请内存 */
    mic_data = hmac_wpi_mic_data_alloc(mac_frame_is_qos_data(&mac_hdr->st_frame_control), payload_len, &mic_data_len);
    if (mic_data == NULL) {
        oam_error_log1(0, OAM_SF_ANY, "hmac_wpi_calc_tx_netbuf_mic:mic_data alloc fail, payload_len[%d]", payload_len);
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }
    /* 为计算mic预备数据 */
    ret = hmac_prepare_wpi_mic_data(mac_hdr, payload, payload_len, encrypt_params->pc, mic_data, mic_data_len);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ANY, "hmac_wpi_calc_tx_netbuf_mic:prepare mic data fail, ret[%d]", ret);
        oal_mem_free_m(mic_data, OAL_TRUE);
        return ret;
    }
    ret = hmac_wpi_calc_mic(mic_data, mic_data_len, pn, encrypt_params->mic_key, mic, mic_len);
    /* 计算完mic后，释放mic data */
    oal_mem_free_m(mic_data, OAL_TRUE);
    return ret;
}

/*
 * 函 数 名  : hmac_wpi_check_rx_netbuf_mic
 * 功能描述  : 检查接收netbuf的mic
 */
OAL_STATIC uint32_t hmac_wpi_check_rx_netbuf_mic(hmac_decrypt_params_stru *decrypt_params,
    oal_netbuf_stru *netbuf, uint16_t pc, uint8_t *pn)
{
    uint8_t calc_mic[WPI_MIC_LEN] = {0};
    uint8_t *mic = oal_netbuf_data(netbuf) + oal_netbuf_len(netbuf);

    if (hmac_wpi_calc_rx_netbuf_mic(netbuf, decrypt_params, calc_mic, WPI_MIC_LEN, pc, pn) != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ANY, "hmac_wpi_check_rx_netbuf_mic:calc mic fail");
        decrypt_params->wpi_rx_debug->rx_mic_calc_fail++;
        return OAL_ERR_CODE_WAPI_MIC_CALC_FAIL;
    }
    if (memcmp(mic, calc_mic, WPI_MIC_LEN) != 0) {
        /* wapi检查mic失败是warning，PWL也统一打warning */
        oam_warning_log0(0, OAM_SF_ANY, "hmac_wpi_check_rx_netbuf_mic:mic is wrong");
        decrypt_params->wpi_rx_debug->rx_mic_diff_fail++;
        return OAL_ERR_CODE_WAPI_MIC_CMP_FAIL;
    }
    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_wpi_decrypt_data
 * 功能描述  : 解密旧netbuf数据到新netbuf
 */
uint32_t hmac_wpi_decrypt_data(
    hmac_decrypt_params_stru *decrypt_params, oal_netbuf_stru *netbuf_new,
    oal_netbuf_stru *netbuf_old, uint8_t *pn, uint16_t pc)
{
    uint8_t *decrypt_data = NULL;
    uint16_t decrypt_data_len;
    uint8_t *payload = (uint8_t *)oal_netbuf_data(netbuf_old);
    uint16_t payload_len = (uint16_t)oal_netbuf_len(netbuf_old);
    mac_rx_ctl_stru *rx_ctl = (mac_rx_ctl_stru *)oal_netbuf_cb(netbuf_old);
    uint8_t mac_hdr_len = rx_ctl->uc_mac_header_len;
    uint8_t pn_tmp[WPI_PN_LEN] = { 0 }; /* 保存pn翻转后的临时变量 */
    uint32_t ret;

    if (memcpy_s(pn_tmp, WPI_PN_LEN, pn, WPI_PN_LEN) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "hmac_wpi_decrypt_data:memcpy_s pn failed");
        return OAL_FAIL;
    }
    hmac_wpi_swap_pn(pn_tmp, WPI_PN_LEN);
    decrypt_data = payload + mac_hdr_len + WPI_HEAD_LEN;
    decrypt_data_len = payload_len - mac_hdr_len - WPI_HEAD_LEN - WPI_MIC_LEN;
    /* 解密原netbuf内的加密数据到新申请的netbuf */
    ret = hmac_wpi_decrypt(pn_tmp, decrypt_data, (decrypt_data_len + WPI_MIC_LEN),
        decrypt_params->data_key, (oal_netbuf_data(netbuf_new) + mac_hdr_len));
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ANY, "hmac_wpi_decrypt_data:hmac_wpi_decrypt fail, ret[%d]", ret);
        return OAL_FAIL;
    }
    /* 检查MIC是否正确 */
    if (hmac_wpi_check_rx_netbuf_mic(decrypt_params, netbuf_new, pc, pn_tmp) != OAL_SUCC) {
        return OAL_ERR_CODE_WAPI_MIC_CALC_FAIL;
    }
    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_wpi_check_rx_cb
 * 功能描述  : 解密前检查钩子
 */
OAL_STATIC uint32_t hmac_wpi_check_rx_cb(hmac_user_stru *hmac_user)
{
    /* 通用流程 */
    if (hmac_user->wpi_cb.hmac_wpi_get_key_cb == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "hmac_wpi_check_rx_cb:hmac_wpi_get_key_cb is null");
        return OAL_FAIL;
    }

    /* 解密流程 */
    if (hmac_user->wpi_cb.hmac_wpi_get_rx_key_index_cb == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "hmac_wpi_check_rx_cb:hmac_wpi_get_rx_key_index_cb is null");
        return OAL_FAIL;
    }
    if (hmac_user->wpi_cb.hmac_wpi_get_rx_pn_cb == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "hmac_wpi_check_rx_cb:hmac_wpi_get_rx_pn_cb is null");
        return OAL_FAIL;
    }
    if (hmac_user->wpi_cb.hmac_wpi_get_rx_pc_cb == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "hmac_wpi_check_rx_cb:hmac_wpi_get_rx_pc_cb is null");
        return OAL_FAIL;
    }

    if (hmac_user->wpi_cb.hmac_wpi_get_user_rx_pn_cb == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "hmac_wpi_check_rx_cb:hmac_wpi_get_user_rx_pn_cb is null");
        return OAL_FAIL;
    }

    if (hmac_user->wpi_cb.hmac_wpi_check_rx_pn == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "hmac_wpi_check_rx_cb:hmac_wpi_check_rx_pn is null");
        return OAL_FAIL;
    }

    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_wpi_get_decrypt_param
 * 功能描述  : 根据hmac_user构造解密参数，以便后续解密过程直接获取
 * 特别说明  : 之后的解密相关数据直接从结构体中获取,不再区分key_index等
 */
OAL_STATIC uint32_t hmac_wpi_get_decrypt_param(hmac_user_stru *hmac_user, oal_netbuf_stru *netbuf,
    hmac_decrypt_params_stru *decrypt_params)
{
    // 外部判断hmac_user 和 netbuf 是否为空
    uint8_t mac_hdr_len;
    mac_rx_ctl_stru *rx_ctl = (mac_rx_ctl_stru *)oal_netbuf_cb(netbuf); // 获取数据帧的控制字节
    uint8_t *payload = (uint8_t *)oal_netbuf_data(netbuf); // 获取数据帧的起始地址
    uint8_t key_index;
    // 获取数据帧MAC头的长度
    mac_hdr_len = rx_ctl->uc_mac_header_len;

    key_index = hmac_user->wpi_cb.hmac_wpi_get_rx_key_index_cb(payload + mac_hdr_len);
    decrypt_params->data_key = hmac_user->wpi_cb.hmac_wpi_get_key_cb(hmac_user, key_index, KEY_TYPE_DATA);
    decrypt_params->mic_key = hmac_user->wpi_cb.hmac_wpi_get_key_cb(hmac_user, key_index, KEY_TYPE_MIC);

    decrypt_params->pn_inc = hmac_user->wpi_cb.hmac_wpi_get_pn_inc(hmac_user);

    // 根据帧类型和子类型获取user rx_pn
    decrypt_params->rx_pn = hmac_user->wpi_cb.hmac_wpi_get_user_rx_pn_cb(hmac_user, rx_ctl);

    decrypt_params->hmac_wpi_get_rx_pc = hmac_user->wpi_cb.hmac_wpi_get_rx_pc_cb;
    decrypt_params->hmac_wpi_get_rx_pn = hmac_user->wpi_cb.hmac_wpi_get_rx_pn_cb; // 获取报文中pn的钩子
    decrypt_params->decrypt_data = hmac_user->wpi_cb.decrypt_data;
    decrypt_params->hmac_wpi_check_rx_pn = hmac_user->wpi_cb.hmac_wpi_check_rx_pn;

    if (oal_any_null_ptr3(decrypt_params->data_key, decrypt_params->mic_key, decrypt_params->rx_pn)) {
        oam_error_log0(0, OAM_SF_ANY, "hmac_wpi_get_decrypt_param:params data has null");
        return OAL_FAIL;
    }
    decrypt_params->wpi_rx_debug = &(hmac_user->wpi_debug.rx_debug);
    return OAL_SUCC;
}

OAL_STATIC uint32_t hmac_wpi_decrypt_rx_netbuf(hmac_decrypt_params_stru *decrypt_params, oal_netbuf_stru *netbuf)
{
    uint8_t mac_hdr_len;
    mac_ieee80211_frame_stru *mac_hdr = NULL;
    mac_rx_ctl_stru *rx_ctl = NULL;
    oal_netbuf_stru *netbuf_new = NULL;
    mac_rx_ctl_stru *rx_ctl_new = NULL;
    uint16_t payload_len;
    uint16_t pc;
    uint8_t *pn; /* 报文中的pn */
    uint8_t pn_check_res;
    /********************************* 准备工作 ****************************************/
    // 获取数据帧的长度
    payload_len = (uint16_t)oal_netbuf_len(netbuf);
    // 获取数据帧的控制字节
    rx_ctl = (mac_rx_ctl_stru *)oal_netbuf_cb(netbuf);
    // 获取数据帧MAC头起始地址
    mac_hdr = (mac_ieee80211_frame_stru *)oal_netbuf_data(netbuf);
    // 获取数据帧MAC头的长度
    mac_hdr_len = rx_ctl->uc_mac_header_len;
    /* 帧体长度有效性判断 */
    if ((mac_hdr_len > MAC_80211_QOS_HTC_4ADDR_FRAME_LEN) &&
        (payload_len < (mac_hdr_len + WPI_HEAD_LEN + WPI_MIC_LEN))) {
        return OAL_FAIL;
    }
    // 将MAC头里帧控制的加密标记置为0
    mac_hdr->st_frame_control.bit_protected_frame = OAL_FALSE;
    pc = decrypt_params->hmac_wpi_get_rx_pc(((uint8_t *)mac_hdr) + mac_hdr_len); // 从原始帧体中获取pc值
    pn = decrypt_params->hmac_wpi_get_rx_pn(((uint8_t *)mac_hdr) + mac_hdr_len); // 从原始帧体中获取pn
    /* 若rx_pn非法，则不进行解密动作 */
    pn_check_res = decrypt_params->hmac_wpi_check_rx_pn(decrypt_params->rx_pn, pn);
    if (pn_check_res == PN_TYPE_INVALID) {
        oam_warning_log0(0, OAM_SF_ANY, "hmac_wpi_decrypt_rx_netbuf:pn invalid");
        return OAL_FAIL;
    }
    /* 1. 为解密数据申请内存 */
    netbuf_new = oal_mem_netbuf_alloc(OAL_NORMAL_NETBUF, WLAN_MEM_NETBUF_SIZE2, OAL_NETBUF_PRIORITY_MID);
    if (netbuf_new == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "hmac_wpi_decrypt_rx_netbuf:alloc memory failed");
        decrypt_params->wpi_rx_debug->rx_netbuf_alloc_fail++;
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }
    /* 2. 将旧netbuf的mac头拷贝到新netbuf中 */
    oal_netbuf_init(netbuf_new, mac_hdr_len);
    if (memcpy_s(oal_netbuf_data(netbuf_new), mac_hdr_len, (uint8_t *)mac_hdr, mac_hdr_len) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "hmac_wpi_decrypt_rx_netbuf:memcpy_s mac_hdr failed");
        oal_netbuf_free(netbuf_new);
        decrypt_params->wpi_rx_debug->rx_mac_hdr_copy_fail++;
        return OAL_FAIL;
    }
    /* 3. 将旧netbuf的cb字段拷贝到新netbuf中 */
    rx_ctl_new = (mac_rx_ctl_stru *)oal_netbuf_cb(netbuf_new);
    if (memcpy_s(rx_ctl_new, MAC_TX_CTL_SIZE, rx_ctl, MAC_TX_CTL_SIZE) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "hmac_wpi_decrypt_rx_netbuf:memcpy_s rx_ctl failed");
        oal_netbuf_free(netbuf_new);
        decrypt_params->wpi_rx_debug->rx_ctrl_copy_fail++;
        return OAL_FAIL;
    }
    oal_netbuf_put(netbuf_new, payload_len - mac_hdr_len - WPI_HEAD_LEN - WPI_MIC_LEN);
    /* 4. 解密原netbuf的加密数据部分 */
    if (decrypt_params->decrypt_data(decrypt_params, netbuf_new, netbuf, pn, pc) != OAL_SUCC) {
        oal_netbuf_free(netbuf_new);
        decrypt_params->wpi_rx_debug->rx_decrypt_fail++;
        return OAL_ERR_CODE_WAPI_DECRYPT_FAIL;
    }
    /* 5. 更新rx ctl */
    hmac_wpi_update_rx_ctl(netbuf_new, rx_ctl_new);
    /* 6. 使用报文中的pn更新到user下RX PN  */
    if ((pn_check_res == PN_TYPE_VALID) && (memcpy_s(decrypt_params->rx_pn, WPI_PN_LEN, pn, WPI_PN_LEN) != EOK)) {
        oam_error_log0(0, OAM_SF_ANY, "hmac_wpi_decrypt_rx_netbuf:update rx pn table failed");
        oal_netbuf_free(netbuf_new);
        return OAL_FAIL;
    }
    /* 8. 将新netbuf插入原netbuf后面(在hmac_wapi_netbuff_rx_handle里会将原netbuf删除) */
    hmac_wpi_insert_netbuf(netbuf, netbuf_new);
    decrypt_params->wpi_rx_debug->rx_decrypt_ok++;
    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_wapi_decrypt
 * 功能描述  : 对netbuf数据进行解密
 */
uint32_t hmac_wpi_decrypt_rx_data(hmac_user_stru *hmac_user, oal_netbuf_stru *netbuf)
{
    hmac_decrypt_params_stru decrypt_params = { 0 };

    if ((hmac_user == NULL) || (netbuf == NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "hmac_wpi_decrypt_rx_data:params is null");
        return OAL_ERR_CODE_PTR_NULL;
    }
    if (hmac_wpi_check_rx_cb(hmac_user) != OAL_SUCC) {
        return OAL_FAIL;
    }

    /* 构造解密需要的参数 */
    if (hmac_wpi_get_decrypt_param(hmac_user, netbuf, &decrypt_params) != OAL_SUCC) {
        return OAL_FAIL;
    }
    /* 解密帧体数据 */
    if (hmac_wpi_decrypt_rx_netbuf(&decrypt_params, netbuf) != OAL_SUCC) {
        return OAL_FAIL;
    }
    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_wpi_update_tx_ctl
 * 功能描述  : 更新发送控制信息
 */
OAL_STATIC void hmac_wpi_update_tx_ctl(oal_netbuf_stru *netbuf, uint16_t payload_len)
{
    mac_tx_ctl_stru *tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(netbuf);
    mac_get_cb_frame_header_addr(tx_ctl) = (mac_ieee80211_frame_stru *)oal_netbuf_data(netbuf);
    mac_get_cb_80211_mac_head_type(tx_ctl) = 1;
    /* 不包括mac hdr */
    mac_get_cb_mpdu_len(tx_ctl) = payload_len;
}

/*
 * 函 数 名  : hmac_wpi_encrypt_data
 * 功能描述  : 加密旧netbuf数据到新netbuf
 */
uint32_t hmac_wpi_encrypt_data(hmac_encrypt_params_stru *encrypt_params,
    oal_netbuf_stru *netbuf_new, oal_netbuf_stru *netbuf_old)
{
    uint16_t payload_len = oal_netbuf_len(netbuf_old); /* payload 长度,不包含帧头长度 */
    uint8_t *payload = oal_netbuf_data(netbuf_old); /* payload的起始位置在snap头 */
    uint8_t *data = NULL;
    uint8_t calc_mic[WPI_MIC_LEN]; // 保存计算MIC
    uint8_t pn[WPI_PN_LEN]; // 保存变换后的pn,用来计算mic和加密
    uint32_t ret;

    /* 获取发送的pn */
    if (memcpy_s(pn, WPI_PN_LEN, encrypt_params->tx_pn, WPI_PN_LEN) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "hmac_wpi_encrypt_data:get tx pn fail");
        return OAL_FAIL;
    }
    /* pn对半交换 */
    hmac_wpi_swap_pn(pn, WPI_PN_LEN);

    /* 计算MIC */
    if (hmac_wpi_calc_tx_netbuf_mic(netbuf_old, pn, encrypt_params, calc_mic, WPI_MIC_LEN) != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ANY, "hmac_wpi_encrypt_data:calc mic fail");
        encrypt_params->wpi_tx_debug->tx_mic_calc_fail++;
        return OAL_ERR_CODE_WAPI_MIC_CALC_FAIL;
    }
    data = oal_mem_alloc_m(OAL_MEM_POOL_ID_LOCAL, payload_len + WPI_MIC_LEN, OAL_TRUE);
    if (data == NULL) {
        oam_error_log1(0, OAM_SF_ANY, "hmac_wpi_encrypt_data:alloc fail, payload_len[%d]", payload_len);
        encrypt_params->wpi_tx_debug->tx_encrypt_data_alloc_fail++;
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }
    /* 拷贝旧netbuf的playload到data，用于加密 */
    if (memcpy_s(data, payload_len + WPI_MIC_LEN, payload, payload_len) != EOK) {
        oam_error_log1(0, OAM_SF_ANY, "hmac_wpi_encrypt_data:copy payload fail, payload_len[%d]", payload_len);
        oal_mem_free_m(data, OAL_TRUE);
        encrypt_params->wpi_tx_debug->tx_playload_copy_fail++;
        return OAL_FAIL;
    }
    /* 拷贝计算的MIC到data */
    if (memcpy_s(data + payload_len, WPI_MIC_LEN, calc_mic, WPI_MIC_LEN) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "hmac_wpi_encrypt_data:copy mic fail");
        oal_mem_free_m(data, OAL_TRUE);
        encrypt_params->wpi_tx_debug->tx_mic_copy_fail++;
        return OAL_FAIL;
    }

    /* 加密，加密需要传的数据格式为 playload + mic */
    ret = hmac_wpi_encrypt(pn, data, payload_len + WPI_MIC_LEN, encrypt_params->data_key,
        oal_netbuf_data(netbuf_new) + WPI_HEAD_LEN + MAC_80211_QOS_HTC_4ADDR_FRAME_LEN);
    if (ret != OAL_SUCC) {
        encrypt_params->wpi_tx_debug->tx_encrypt_fail++;
    }
    oal_mem_free_m(data, OAL_TRUE);
    return ret;
}

/*
 * 函 数 名  : hmac_wpi_check_tx_cb
 * 功能描述  : 加密前检查钩子
 */
OAL_STATIC uint32_t hmac_wpi_check_tx_cb(hmac_user_stru *hmac_user)
{
    /* 通用流程 */
    if (hmac_user->wpi_cb.hmac_wpi_get_key_cb == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "hmac_wpi_check_tx_cb:hmac_wpi_get_key_cb is null");
        return OAL_FAIL;
    }

    /* 加密流程 */
    if (hmac_user->wpi_cb.hmac_wpi_get_tx_key_index_cb == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "hmac_wpi_check_tx_cb:hmac_wpi_get_tx_key_index_cb is null");
        return OAL_FAIL;
    }
    if (hmac_user->wpi_cb.hmac_wpi_get_tx_pn_cb == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "hmac_wpi_check_tx_cb:hmac_wpi_get_tx_pn_cb is null");
        return OAL_FAIL;
    }
    if (hmac_user->wpi_cb.hmac_wpi_set_head_cb == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "hmac_wpi_check_tx_cb:hmac_wpi_set_head_cb is null");
        return OAL_FAIL;
    }
    if (hmac_user->wpi_cb.hmac_wpi_get_pn_inc == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "hmac_wpi_check_tx_cb:hmac_wpi_get_pn_inc is null");
        return OAL_FAIL;
    }
    return OAL_SUCC;
}
/*
 * 函 数 名  : hmac_wpi_get_encrypt_param
 * 功能描述  : 根据hmac_user构造加密参数，以便后续加密过程直接获取
 * 特别说明  : 之后的加密相关数据直接从结构体中获取,不再区分key_index等
 */
OAL_STATIC uint32_t hmac_wpi_get_encrypt_param(hmac_user_stru *hmac_user, hmac_encrypt_params_stru *encrypt_params)
{
    // 外部判断hmac_user是否为空
    uint8_t key_index = hmac_user->wpi_cb.hmac_wpi_get_tx_key_index_cb(hmac_user);

    encrypt_params->data_key = hmac_user->wpi_cb.hmac_wpi_get_key_cb(hmac_user, key_index, KEY_TYPE_DATA);
    encrypt_params->mic_key = hmac_user->wpi_cb.hmac_wpi_get_key_cb(hmac_user, key_index, KEY_TYPE_MIC);
    encrypt_params->pc = (uint16_t)key_index; // 获取pc值

    encrypt_params->tx_pn= hmac_user->wpi_cb.hmac_wpi_get_tx_pn_cb(hmac_user);
    encrypt_params->pn_inc = hmac_user->wpi_cb.hmac_wpi_get_pn_inc(hmac_user);
    encrypt_params->encrypt_data = hmac_user->wpi_cb.encrypt_data;
    if (oal_any_null_ptr3(encrypt_params->data_key, encrypt_params->mic_key, encrypt_params->tx_pn)) {
        oam_error_log0(0, OAM_SF_ANY, "hmac_wpi_encrypt_tx_data:params data has null");
        return OAL_FAIL;
    }
    encrypt_params->wpi_tx_debug = &(hmac_user->wpi_debug.tx_debug);
    encrypt_params->hmac_wpi_set_head = hmac_user->wpi_cb.hmac_wpi_set_head_cb;
    return OAL_SUCC;
}
OAL_STATIC oal_netbuf_stru *hmac_wpi_encrypt_tx_netbuf(hmac_encrypt_params_stru *encrypt_params,
    oal_netbuf_stru *netbuf)
{
    mac_ieee80211_frame_stru *mac_hdr = NULL;
    uint8_t mac_hdr_len;
    oal_netbuf_stru *netbuf_new = NULL;
    mac_tx_ctl_stru *tx_ctl;
    uint16_t payload_len;

    /********************************* 准备工作 ****************************************/
    tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(netbuf);
    mac_hdr = mac_get_cb_frame_header_addr(tx_ctl);
    mac_hdr_len = mac_get_cb_frame_header_length(tx_ctl);
    payload_len = mac_get_cb_mpdu_len(tx_ctl);
    // MAC头设置加密标志
    mac_hdr->st_frame_control.bit_protected_frame = OAL_TRUE;

    /*********************************** 开始加密 ***************************************/
    /* 1. 为加密后的新netbuf申请内存 */
    netbuf_new = oal_mem_netbuf_alloc(OAL_NORMAL_NETBUF, WLAN_MEM_NETBUF_SIZE2, OAL_NETBUF_PRIORITY_MID);
    if (netbuf_new == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "hmac_wpi_encrypt_tx_netbuf:alloc fail");
        encrypt_params->wpi_tx_debug->tx_netbuf_alloc_fail++;
        return NULL;
    }
    /* 2. 将旧netbuf的cb拷贝到新netbuf中 */
    if (memcpy_s(oal_netbuf_cb(netbuf_new), MAC_TX_CTL_SIZE, (uint8_t *)tx_ctl, MAC_TX_CTL_SIZE) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "hmac_wpi_encrypt_tx_netbuf:memcpy_s tx_ctl fail");
        oal_netbuf_free(netbuf_new);
        encrypt_params->wpi_tx_debug->tx_ctrl_copy_fail++;
        return NULL;
    }
    /* 3. 将旧netbuf的mac头拷贝到新netbuf中 */
    oal_netbuf_init(netbuf_new, MAC_80211_QOS_HTC_4ADDR_FRAME_LEN);
    if (memcpy_s(oal_netbuf_data(netbuf_new), MAC_80211_QOS_HTC_4ADDR_FRAME_LEN, mac_hdr, mac_hdr_len) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "hmac_wpi_encrypt_tx_netbuf:memcpy_s mac_hdr fail");
        oal_netbuf_free(netbuf_new);
        encrypt_params->wpi_tx_debug->tx_mac_hdr_copy_fail++;
        return NULL;
    }

    /* 4. 填充加密数据到新netbuf的payload */
    if (encrypt_params->encrypt_data(encrypt_params, netbuf_new, netbuf) != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ANY, "hmac_wpi_encrypt_tx_netbuf:hmac_wpi_encrypt_data fail");
        oal_netbuf_free(netbuf_new);
        return NULL;
    }
    /* 5. 填写wpi头 */
    if (encrypt_params->hmac_wpi_set_head(encrypt_params, netbuf_new) != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ANY, "hmac_wpi_encrypt_tx_netbuf:hmac_set_wpi_head fail");
        oal_netbuf_free(netbuf_new);
        encrypt_params->wpi_tx_debug->tx_set_header_fail++;
        return NULL;
    }
    hmac_wpi_update_tx_ctl(netbuf_new, WPI_HEAD_LEN + payload_len + WPI_MIC_LEN);
    oal_netbuf_put(netbuf_new, payload_len + WPI_MIC_LEN + WPI_HEAD_LEN);
    /* netbuf 的data指针指向payload */
    oal_netbuf_pull(netbuf_new, MAC_80211_QOS_HTC_4ADDR_FRAME_LEN);
    /* 8. 更新tx pn */
    // wapi和pwl更新tx_pn方式相同，故不再使用钩子区分
    hmac_wapi_update_pn(encrypt_params->tx_pn, encrypt_params->pn_inc);
    encrypt_params->wpi_tx_debug->tx_encrypt_ok++;
    return netbuf_new;
}

/*
 * 函 数 名  : hmac_wpi_encrypt_tx_data
 * 功能描述  : 加密发送netbuf
 * 特别说明  : 单播、组播传进来的hmac_user不一样
 */
oal_netbuf_stru *hmac_wpi_encrypt_tx_data(hmac_user_stru *hmac_user, oal_netbuf_stru *netbuf)
{
    oal_netbuf_stru *netbuf_new = NULL;
    uint16_t queue_id;
    hmac_encrypt_params_stru encrypt_params = { 0 };

    if (oal_any_null_ptr2(hmac_user, netbuf)) {
        oam_error_log0(0, OAM_SF_ANY, "hmac_wpi_encrypt_tx_data:params is null");
        return NULL;
    }
    if (hmac_wpi_check_tx_cb(hmac_user) != OAL_SUCC) {
        return NULL;
    }
    /* 构造加密参数 */
    if (hmac_wpi_get_encrypt_param(hmac_user, &encrypt_params) != OAL_SUCC) {
        return NULL;
    }
    queue_id = oal_skb_get_queue_mapping(netbuf);
    /* 加密帧体 ，返回加密后的帧体地址 */
    netbuf_new = hmac_wpi_encrypt_tx_netbuf(&encrypt_params, netbuf);

    /* ack会被调度到data前面，导致大量的pn乱序 */
    if (queue_id == WLAN_TCP_DATA_QUEUE || queue_id == WLAN_TCP_ACK_QUEUE) {
        oal_skb_set_queue_mapping(netbuf_new, WLAN_TCP_DATA_QUEUE);
    } else {
        oal_skb_set_queue_mapping(netbuf_new, queue_id);
    }

    return netbuf_new;
}

OAL_STATIC void hmac_user_debug_display_rx_info(hmac_user_stru *hmac_user)
{
    hmac_wpi_rx_debug *wpi_rx_debug = &(hmac_user->wpi_debug.rx_debug);
    oam_warning_log0(0, OAM_SF_ANY, "-------RX DEBUG INFO:");
    oam_warning_log4(0, OAM_SF_ANY,
        "rx_netbuf_alloc_fail[%u], rx_ctrl_copy_fail[%u], rx_mac_hdr_copy_fail[%u], rx_decrypt_fail[%u]",
        wpi_rx_debug->rx_netbuf_alloc_fail,
        wpi_rx_debug->rx_ctrl_copy_fail,
        wpi_rx_debug->rx_mac_hdr_copy_fail,
        wpi_rx_debug->rx_decrypt_fail);
    oam_warning_log3(0, OAM_SF_ANY, "rx_mic_calc_fail[%u], rx_mic_diff_fail[%u], rx_decrypt_ok[%u]",
        wpi_rx_debug->rx_mic_calc_fail,
        wpi_rx_debug->rx_mic_diff_fail,
        wpi_rx_debug->rx_decrypt_ok);
}

OAL_STATIC void hmac_user_debug_display_tx_info(hmac_user_stru *hmac_user)
{
    hmac_wpi_tx_debug *wpi_tx_debug = &(hmac_user->wpi_debug.tx_debug);
    oam_warning_log0(0, OAM_SF_ANY, "-------TX DEBUG INFO:");
    oam_warning_log4(0, OAM_SF_ANY,
        "tx_netbuf_alloc_fail[%u], tx_ctrl_copy_fail[%u], tx_mac_hdr_copy_fail[%u], tx_mic_calc_fail[%u]",
        wpi_tx_debug->tx_netbuf_alloc_fail,
        wpi_tx_debug->tx_ctrl_copy_fail,
        wpi_tx_debug->tx_mac_hdr_copy_fail,
        wpi_tx_debug->tx_mic_calc_fail);
    oam_warning_log4(0, OAM_SF_ANY,
        "tx_encrypt_data_alloc_fail[%u], tx_playload_copy_fail[%u], tx_mic_copy_fail[%u], tx_encrypt_fail[%u]",
        wpi_tx_debug->tx_encrypt_data_alloc_fail,
        wpi_tx_debug->tx_playload_copy_fail,
        wpi_tx_debug->tx_mic_copy_fail,
        wpi_tx_debug->tx_encrypt_fail);
    oam_warning_log2(0, OAM_SF_ANY, "tx_set_header_fail[%u], tx_encrypt_ok[%u]",
        wpi_tx_debug->tx_set_header_fail,
        wpi_tx_debug->tx_encrypt_ok);
}

/*
 * 函 数 名  : hmac_user_debug_display_wpi_info
 * 功能描述  : 打印pwl内容
 */
uint32_t hmac_user_debug_display_wpi_info(mac_vap_stru *mac_vap, uint16_t usr_idx)
{
    hmac_user_stru *hmac_user = NULL;
    hmac_user_stru *hmac_multi_user = NULL;

    hmac_multi_user = (hmac_user_stru *)mac_res_get_hmac_user(mac_vap->us_multi_user_idx);
    if (hmac_multi_user == NULL) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_ANY,
            "multi usr %u does not exist!", mac_vap->us_multi_user_idx);
        return OAL_FAIL;
    }

    oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_ANY, "**************multi usr info start**************");
    hmac_user_debug_display_rx_info(hmac_multi_user);
    hmac_user_debug_display_tx_info(hmac_multi_user);
    oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_ANY, "**************multi usr info end**************");

    hmac_user = (hmac_user_stru *)mac_res_get_hmac_user(usr_idx);
    if (hmac_user == NULL) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_ANY,
            "ucast usr %u does not exist!", usr_idx);
        return OAL_FAIL;
    }

    oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_ANY, "**************ucast usr info start**************");
    hmac_user_debug_display_rx_info(hmac_user);
    hmac_user_debug_display_tx_info(hmac_user);
    oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_ANY, "**************ucast usr info end**************");
    return OAL_SUCC;
}

#endif
