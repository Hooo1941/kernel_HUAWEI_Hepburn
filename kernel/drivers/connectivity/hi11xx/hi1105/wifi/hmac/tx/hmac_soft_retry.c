/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : Hmac Host tx软重传模块
 * 作    者 : wifi
 * 创建日期 : 2020年7月24日
 */

#include "hmac_soft_retry.h"
#include "hmac_host_tx_data.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_SOFT_RETRY_C

/*
 * 功能描述 : host tx msdu软重传处理入口
 * 1.日    期 : 2020年7月24日
 *   作    者 : wifi
 *   修改内容 : 新生成函数
 */
uint32_t hmac_tx_soft_retry_process(hmac_tid_info_stru *tid_info, oal_netbuf_stru *netbuf)
{
    mac_tx_ctl_stru *tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(netbuf);

    if (mac_get_cb_retried_num(tx_ctl) > MAC_KEY_FRAME_SOFT_RETRY_CNT) {
        return OAL_FAIL;
    }

    oam_warning_log4(0, OAM_SF_TX, "{hmac_tx_soft_retry_process::user[%d] tid[%d] retry frame type[%d] subtype[%d]",
        tid_info->user_index, tid_info->tid_no, mac_get_cb_frame_type(tx_ctl), mac_get_cb_frame_subtype(tx_ctl));

    return hmac_host_tx_tid_enqueue(tid_info, netbuf);
}
