/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : AMPDU聚合、BA处理接口定义源文件
 * 作    者 :
 * 创建日期 : 2014年11月25日
 */

/* 1 头文件包含 */
#include "wlan_spec.h"
#include "mac_vap.h"
#include "hmac_blockack.h"
#include "hmac_main.h"
#include "hmac_rx_data.h"
#include "hmac_mgmt_bss_comm.h"
#include "hmac_user.h"
#include "hmac_auto_adjust_freq.h"
#include "wlan_chip_i.h"
#include "mac_mib.h"
#include "hmac_config.h"
#include "hmac_tx_data.h"
#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_BLOCKACK_C

const uint16_t g_ba_rx_timeout_len[WLAN_WME_AC_PSM + 2] = { // 2 enum num
    HMAC_BA_RX_BE_TIMEOUT,      /* WLAN_WME_AC_BE */
    HMAC_BA_RX_BK_TIMEOUT,      /* WLAN_WME_AC_BK */
    HMAC_BA_RX_VI_TIMEOUT,      /* WLAN_WME_AC_VI */
    HMAC_BA_RX_VO_TIMEOUT,      /* WLAN_WME_AC_VO */
    HMAC_BA_RX_DEFAULT_TIMEOUT, /* WLAN_WME_AC_SOUNDING */
    HMAC_BA_RX_DEFAULT_TIMEOUT, /* WLAN_WME_AC_MGMT */
    HMAC_BA_RX_DEFAULT_TIMEOUT, /* WLAN_WME_AC_MC */
};

OAL_STATIC uint8_t hmac_ba_timeout_param_is_valid(hmac_ba_alarm_stru *alarm_data,
    hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, uint8_t rx_tid)
{
    if (rx_tid >= WLAN_TID_MAX_NUM) {
        oam_error_log1(0, OAM_SF_BA, "{hmac_ba_timeout_fn::tid %d overflow.}", rx_tid);
        return OAL_FALSE;
    }

    if (hmac_vap == NULL) {
        oam_error_log1(0, 0, "{hmac_ba_timeout_fn::vap[%d] NULL}", alarm_data->uc_vap_id);
        return OAL_FALSE;
    }

    if (hmac_user == NULL) {
        oam_error_log1(0, 0, "{hmac_ba_timeout_fn::user[%d] NULL}", alarm_data->us_mac_user_idx);
        return OAL_FALSE;
    }

    return OAL_TRUE;
}


/* 2 全局变量定义 */
/* 3 函数实现 */
/*
 * 函 数 名  : hmac_reorder_ba_timer_start
 * 功能描述  : 有帧进入重排序队列时,需要启动定时器
 * 1.日    期  : 2018年8月4日
 *   修改内容  : 新生成函数
 */
void hmac_reorder_ba_timer_start(hmac_vap_stru *pst_hmac_vap,
                                 hmac_user_stru *pst_hmac_user,
                                 uint8_t uc_tid)
{
    mac_vap_stru *pst_mac_vap = NULL;
    hmac_ba_rx_stru *pst_ba_rx_stru = NULL;
    mac_device_stru *pst_device = NULL;
    uint16_t us_timeout;

    /* 如果超时定时器已经被注册则返回 */
    if (pst_hmac_user->ast_tid_info[uc_tid].st_ba_timer.en_is_registerd == OAL_TRUE) {
        return;
    }

    pst_mac_vap = &pst_hmac_vap->st_vap_base_info;

    pst_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (pst_device == NULL) {
        oam_error_log1(pst_mac_vap->uc_vap_id, OAM_SF_BA,
                       "{hmac_reorder_ba_timer_start::pst_device[%d] null.}",
                       pst_mac_vap->uc_device_id);
        return;
    }

    /* 业务量较小时,使用小周期的重排序定时器,保证及时上报至协议栈;
       业务量较大时,使用大周期的重排序定时器,保证尽量不丢包 */
    if (OAL_FALSE == hmac_wifi_rx_is_busy()) {
        us_timeout = pst_hmac_vap->us_rx_timeout_min[WLAN_WME_TID_TO_AC(uc_tid)];
    } else {
        us_timeout = pst_hmac_vap->us_rx_timeout[WLAN_WME_TID_TO_AC(uc_tid)];
    }

    pst_ba_rx_stru = pst_hmac_user->ast_tid_info[uc_tid].pst_ba_rx_info;

    oal_spin_lock(&(pst_hmac_user->ast_tid_info[uc_tid].st_ba_timer_lock));

    frw_timer_create_timer_m(&(pst_hmac_user->ast_tid_info[uc_tid].st_ba_timer),
                             hmac_ba_timeout_fn,
                             us_timeout,
                             &(pst_ba_rx_stru->st_alarm_data),
                             OAL_FALSE,
                             OAM_MODULE_ID_HMAC,
                             pst_device->core_id);

    oal_spin_unlock(&(pst_hmac_user->ast_tid_info[uc_tid].st_ba_timer_lock));
}
OAL_STATIC uint32_t dbac_need_change_ba_param(mac_device_stru *mac_dev, hmac_vap_stru *hmac_vap)
{
    if (mac_dev == NULL || hmac_vap == NULL) {
        return OAL_FALSE;
    }

    return (mac_dev->en_dbac_running == OAL_TRUE && hmac_vap->st_vap_base_info.en_vap_mode == WLAN_VAP_MODE_BSS_STA);
}
/*
 * 函 数 名  : hmac_ba_rx_hdl_init
 * 功能描述  : 初始化ba相关参数
 * 1.日    期  : 2018年8月4日
 *   修改内容  : 新生成函数
 */
void hmac_ba_rx_hdl_baw_init(hmac_vap_stru *hmac_vap, hmac_user_stru *pst_hmac_user,
    hmac_ba_rx_stru *pst_ba_rx_stru, uint8_t uc_tid)
{
    mac_rx_buffer_size_stru *rx_buffer_size = mac_vap_get_rx_buffer_size();
    int32_t chip_type = get_mpxx_subchip_type();
    mac_device_stru *mac_dev = mac_res_get_dev(hmac_vap->st_vap_base_info.uc_device_id);

    /* 保存ba size实际大小,用于反馈addba rsp */
    pst_hmac_user->aus_tid_baw_size[uc_tid] = pst_ba_rx_stru->us_baw_size;

    /*  解决goldenap新固件版本兼容性问题 */
    if (pst_ba_rx_stru->us_baw_size <= 1) {
        pst_ba_rx_stru->us_baw_size = WLAN_AMPDU_RX_BUFFER_SIZE;
        /* 异常更新反馈值 */
        pst_hmac_user->aus_tid_baw_size[uc_tid] = WLAN_AMPDU_RX_BUFFER_SIZE;
    }

    /* HE设备并且BA窗大于64时, */
    if (g_wlan_spec_cfg->feature_11ax_is_open &&
        ((pst_ba_rx_stru->us_baw_size > WLAN_AMPDU_RX_BUFFER_SIZE) &&
        mac_user_is_he_user(&pst_hmac_user->st_user_base_info))) {
        /* resp反馈值与addba req保持一致,但是BA BAW需要更新,避免兼容性问题,因此不更新user */
        pst_ba_rx_stru->us_baw_size = WLAN_AMPDU_RX_HE_BUFFER_SIZE;

        /* 定制化设定BA BAW, 则取小 */
        if (rx_buffer_size->en_rx_ampdu_bitmap_ini == OAL_TRUE) {
            pst_ba_rx_stru->us_baw_size =
                    oal_min(rx_buffer_size->us_rx_buffer_size, WLAN_AMPDU_RX_HE_BUFFER_SIZE);
        }

        /* 兼容性问题:如果决策的BAW size 小于对端BAW size,那么更新resp帧BAW size,否则按照对端的BAW size反馈 */
        pst_hmac_user->aus_tid_baw_size[uc_tid] =
                    oal_min(pst_hmac_user->aus_tid_baw_size[uc_tid], pst_ba_rx_stru->us_baw_size);
        pst_ba_rx_stru->us_baw_size = pst_hmac_user->aus_tid_baw_size[uc_tid];
    } else {
        /* BA窗小于64时协商BA窗SIZE */
        pst_ba_rx_stru->us_baw_size = oal_min(pst_ba_rx_stru->us_baw_size, WLAN_AMPDU_RX_BUFFER_SIZE);
        /* 非HE设备更新,反馈值与baw同步更新 */
        pst_hmac_user->aus_tid_baw_size[uc_tid] = pst_ba_rx_stru->us_baw_size;
    }
    /* DBAC 减小baw */
    if (dbac_need_change_ba_param(mac_dev, hmac_vap) == OAL_TRUE) {
        pst_ba_rx_stru->us_baw_size = oal_min(pst_ba_rx_stru->us_baw_size, WLAN_AMPDU_RX_BUFFER_SIZE);
        pst_hmac_user->aus_tid_baw_size[uc_tid] = pst_ba_rx_stru->us_baw_size;
    }
    /* 不支持DDR收发产品,关闭256聚合 */
    if ((chip_type >= BOARD_VERSION_MP16) && (hmac_rx_ring_switch_enabled() == OAL_FALSE)) {
        pst_ba_rx_stru->us_baw_size = oal_min(pst_ba_rx_stru->us_baw_size, WLAN_AMPDU_RX_BUFFER_SIZE);
        pst_hmac_user->aus_tid_baw_size[uc_tid] = pst_ba_rx_stru->us_baw_size;
    }

    /* 配置优先级最高，手动配置接收聚合个数 */
    if (rx_buffer_size->en_rx_ampdu_bitmap_cmd == OAL_TRUE) {
        pst_ba_rx_stru->us_baw_size = rx_buffer_size->us_rx_buffer_size;
        pst_hmac_user->aus_tid_baw_size[uc_tid] = pst_ba_rx_stru->us_baw_size;
    }
}

/*
 * 函 数 名  : hmac_ba_rx_hdl_init
 * 功能描述  : 初始化ba相关参数
 * 1.日    期  : 2018年8月4日
 *   修改内容  : 新生成函数
 */
void hmac_ba_rx_hdl_init(hmac_vap_stru *pst_hmac_vap, hmac_user_stru *pst_hmac_user, uint8_t uc_tid)
{
    mac_vap_stru *pst_mac_vap = NULL;
    hmac_ba_rx_stru *pst_ba_rx_stru = NULL;
    hmac_rx_buf_stru *pst_rx_buff = NULL;
    mac_device_stru *mac_dev = mac_res_get_dev(pst_hmac_vap->st_vap_base_info.uc_device_id);
    uint16_t us_reorder_index;

    pst_mac_vap = &(pst_hmac_vap->st_vap_base_info);

    pst_ba_rx_stru = pst_hmac_user->ast_tid_info[uc_tid].pst_ba_rx_info;

    /* 初始化reorder队列 */
    for (us_reorder_index = 0; us_reorder_index < WLAN_AMPDU_RX_HE_BUFFER_SIZE; us_reorder_index++) {
        pst_rx_buff = hmac_get_ba_rx_dhl(pst_ba_rx_stru, us_reorder_index);
        pst_rx_buff->in_use = 0;
        pst_rx_buff->us_seq_num = 0;
        pst_rx_buff->uc_num_buf = 0;
        oal_netbuf_list_head_init(&(pst_rx_buff->st_netbuf_head));
    }

    /* 初始化BAW */
    hmac_ba_rx_hdl_baw_init(pst_hmac_vap, pst_hmac_user, pst_ba_rx_stru, uc_tid);

    /* Ba会话参数初始化 */
    pst_ba_rx_stru->us_baw_end = mac_ba_seqno_add(pst_ba_rx_stru->us_baw_start, (pst_ba_rx_stru->us_baw_size - 1));
    pst_ba_rx_stru->us_baw_tail = mac_ba_seqno_sub(pst_ba_rx_stru->us_baw_start, 1);
    pst_ba_rx_stru->us_baw_head = mac_ba_seqno_sub(pst_ba_rx_stru->us_baw_start, HMAC_BA_BMP_SIZE);
    pst_ba_rx_stru->uc_mpdu_cnt = 0;
    pst_ba_rx_stru->en_is_ba = OAL_TRUE;  // Ba session is processing
    /* DBAC不开启rx amsdu */
    if (dbac_need_change_ba_param(mac_dev, pst_hmac_vap) == OAL_TRUE) {
        pst_hmac_vap->bit_rx_ampduplusamsdu_active = 0;
    }
#ifdef _PRE_WLAN_FEATURE_PWL
    /* pwl模式不支持amsdu */
    if (pst_hmac_vap->st_vap_base_info.pwl_enable == OAL_TRUE) {
        pst_hmac_vap->bit_rx_ampduplusamsdu_active = 0;
    }
#endif
    pst_ba_rx_stru->en_amsdu_supp = ((pst_hmac_vap->bit_rx_ampduplusamsdu_active & pst_hmac_vap->en_ps_rx_amsdu) ?
                                      OAL_TRUE : OAL_FALSE);
    pst_ba_rx_stru->en_back_var = MAC_BACK_COMPRESSED;
    pst_ba_rx_stru->puc_transmit_addr = pst_hmac_user->st_user_base_info.auc_user_mac_addr;

    /* 初始化定时器资源 */
    pst_ba_rx_stru->st_alarm_data.pst_ba = pst_ba_rx_stru;
    pst_ba_rx_stru->st_alarm_data.us_mac_user_idx = pst_hmac_user->st_user_base_info.us_assoc_id;
    pst_ba_rx_stru->st_alarm_data.uc_vap_id = pst_mac_vap->uc_vap_id;
    pst_ba_rx_stru->st_alarm_data.uc_tid = uc_tid;
    pst_ba_rx_stru->st_alarm_data.us_timeout_times = 0;
    pst_ba_rx_stru->st_alarm_data.en_direction = MAC_RECIPIENT_DELBA;
    pst_ba_rx_stru->en_timer_triggered = OAL_FALSE;

    oal_spin_lock_init(&pst_ba_rx_stru->st_ba_lock);
}

/* hw reorder window init */
void hmac_ba_rx_win_init(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, uint8_t rx_tid)
{
    mac_vap_stru *mac_vap = NULL;
    hmac_ba_rx_stru *ba_rx_win = NULL;
    mac_device_stru *mac_dev = mac_res_get_dev(hmac_vap->st_vap_base_info.uc_device_id);
    uint16_t index;

    mac_vap = &(hmac_vap->st_vap_base_info);
    ba_rx_win = hmac_user->ast_tid_info[rx_tid].pst_ba_rx_info;

    /* 初始化BAW */
    hmac_ba_rx_hdl_baw_init(hmac_vap, hmac_user, ba_rx_win, rx_tid);

    /* 1. 初始化接收窗口， */
    /* 1.1 BA接收窗口参数(SSN与窗口大小)由管理面收到BA请求帧后获取并同步到数据面 */
    ba_rx_win->us_last_relseq = mac_ba_seqno_sub(ba_rx_win->us_baw_start, 1);

    /* 1.2 其它窗口参数通过计算获取 */
    ba_rx_win->us_baw_end = mac_ba_seqno_add(ba_rx_win->us_baw_start, (ba_rx_win->us_baw_size - 1));
    ba_rx_win->us_baw_tail = mac_ba_seqno_sub(ba_rx_win->us_baw_start, 1);
    ba_rx_win->us_baw_head = mac_ba_seqno_sub(ba_rx_win->us_baw_start, HMAC_BA_BMP_SIZE);

    /* 1.3 其它辅助参数进行初始化 */
    ba_rx_win->uc_mpdu_cnt = 0;

    oal_spin_lock_init(&ba_rx_win->st_ba_lock);
    oal_atomic_set(&ba_rx_win->ref_cnt, 1);
    ba_rx_win->en_timer_triggered = OAL_FALSE;
    ba_rx_win->en_ba_status = DMAC_BA_INIT;

    for (index = 0; index < WLAN_AMPDU_RX_HE_BUFFER_SIZE; index++) {
        ba_rx_win->ast_re_order_list[index].in_use = 0;
        ba_rx_win->ast_re_order_list[index].us_seq_num = 0;
        ba_rx_win->ast_re_order_list[index].uc_num_buf  = 0; //  可以删除  ?
        oal_netbuf_list_head_init(&(ba_rx_win->ast_re_order_list[index].st_netbuf_head));
    }

    ba_rx_win->en_back_var = MAC_BACK_COMPRESSED;
    ba_rx_win->puc_transmit_addr = hmac_user->st_user_base_info.auc_user_mac_addr;
    ba_rx_win->uc_mpdu_cnt = 0;
    ba_rx_win->en_is_ba = OAL_TRUE;  // Ba session is processing
    /* DBAC不开启rx amsdu */
    if (dbac_need_change_ba_param(mac_dev, hmac_vap) == OAL_TRUE) {
        hmac_vap->bit_rx_ampduplusamsdu_active = 0;
    }
    ba_rx_win->en_amsdu_supp = ((hmac_vap->bit_rx_ampduplusamsdu_active & hmac_vap->en_ps_rx_amsdu) ?
                                      OAL_TRUE : OAL_FALSE);
    /* 初始化定时器资源 */
    ba_rx_win->st_alarm_data.pst_ba = ba_rx_win;
    ba_rx_win->st_alarm_data.us_mac_user_idx = hmac_user->st_user_base_info.us_assoc_id;
    ba_rx_win->st_alarm_data.uc_vap_id = mac_vap->uc_vap_id;
    ba_rx_win->st_alarm_data.uc_tid = rx_tid;
    ba_rx_win->st_alarm_data.us_timeout_times = 0;
    ba_rx_win->st_alarm_data.en_direction = MAC_RECIPIENT_DELBA;
    ba_rx_win->en_timer_triggered = OAL_FALSE;

    memset_s(ba_rx_win->aul_rx_buf_bitmap, sizeof(ba_rx_win->aul_rx_buf_bitmap),
        0, sizeof(ba_rx_win->aul_rx_buf_bitmap));

    /* 1.4 状态置为初始状态 */
    ba_rx_win->en_ba_status = DMAC_BA_INIT;
}


/*
 * 函 数 名  : hmac_ba_buffer_frame_in_reorder
 * 1.日    期  : 2013年4月11日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC hmac_rx_buf_stru *hmac_ba_buffer_frame_in_reorder(hmac_ba_rx_stru *pst_ba_rx_hdl,
                                                             uint16_t us_seq_num,
                                                             mac_rx_ctl_stru *pst_cb_ctrl)
{
    uint16_t us_buf_index;
    hmac_rx_buf_stru *pst_rx_buf;

    us_buf_index = (us_seq_num & (WLAN_AMPDU_RX_HE_BUFFER_SIZE - 1));
    pst_rx_buf = hmac_get_ba_rx_dhl(pst_ba_rx_hdl, us_buf_index);
    if (pst_rx_buf->in_use == 1) {
        hmac_rx_free_netbuf_list(&pst_rx_buf->st_netbuf_head, pst_rx_buf->uc_num_buf);
        pst_ba_rx_hdl->uc_mpdu_cnt--;
        pst_rx_buf->in_use = 0;
        pst_rx_buf->uc_num_buf = 0;
        oam_info_log1(0, OAM_SF_BA, "{hmac_ba_buffer_frame_in_reorder::slot already used, seq[%d].}", us_seq_num);
    }

    if (pst_cb_ctrl->bit_amsdu_enable == OAL_TRUE) {
        if (pst_cb_ctrl->bit_is_first_buffer == OAL_TRUE) {
            if (oal_netbuf_list_len(&pst_rx_buf->st_netbuf_head) != 0) {
                hmac_rx_free_netbuf_list(&pst_rx_buf->st_netbuf_head,
                                         oal_netbuf_list_len(&pst_rx_buf->st_netbuf_head));
                oam_info_log1(0, OAM_SF_BA,
                    "{hmac_ba_buffer_frame_in_reorder::seq[%d] amsdu first buffer, need clear st_netbuf_head first}",
                    us_seq_num);
            }
            pst_rx_buf->uc_num_buf = 0;
        }

        /* offload下,amsdu帧拆成单帧分别上报 */
        pst_rx_buf->uc_num_buf += pst_cb_ctrl->bit_buff_nums;

        /* 遇到最后一个amsdu buffer 才标记in use 为 1 */
        if (pst_cb_ctrl->bit_is_last_buffer == OAL_TRUE) {
            pst_ba_rx_hdl->uc_mpdu_cnt++;
            pst_rx_buf->in_use = 1;
        } else {
            pst_rx_buf->in_use = 0;
        }
    } else {
        pst_rx_buf->uc_num_buf = pst_cb_ctrl->bit_buff_nums;
        pst_ba_rx_hdl->uc_mpdu_cnt++;
        pst_rx_buf->in_use = 1;
    }
    /* Update the buffered receive packet details */
    pst_rx_buf->us_seq_num = us_seq_num;
    pst_rx_buf->rx_time = (uint32_t)oal_time_get_stamp_ms();

    return pst_rx_buf;
}

OAL_STATIC void hmac_ba_send_frames_add_list(hmac_rx_buf_stru *pst_rx_buf, oal_netbuf_stru *pst_netbuf,
    oal_netbuf_head_stru *pst_netbuf_header)
{
    uint8_t uc_loop_index;

    for (uc_loop_index = 0; uc_loop_index < pst_rx_buf->uc_num_buf; uc_loop_index++) {
        pst_netbuf = oal_netbuf_delist(&pst_rx_buf->st_netbuf_head);
        oal_mem_netbuf_trace(pst_netbuf, OAL_FALSE);
        if (pst_netbuf != NULL) {
            oal_netbuf_list_tail_nolock(pst_netbuf_header, pst_netbuf);
        }
    }
}

/*
 * 函 数 名  : hmac_ba_send_frames_with_gap
 * 功能描述  : 冲刷重排序缓冲区至给定的sequence number位置
 * 1.日    期  : 2014年11月25日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t hmac_ba_send_frames_with_gap(hmac_ba_rx_stru *pst_ba_rx_hdl,
                                                 oal_netbuf_head_stru *pst_netbuf_header,
                                                 uint16_t us_last_seqnum,
                                                 mac_vap_stru *pst_vap)
{
    uint8_t uc_num_frms = 0;
    uint16_t us_seq_num;
    hmac_rx_buf_stru *pst_rx_buf = NULL;
    oal_netbuf_stru *pst_netbuf = NULL;

    us_seq_num = pst_ba_rx_hdl->us_baw_start;

    oam_info_log1(pst_vap->uc_vap_id, OAM_SF_BA, "{hmac_ba_send_frames_with_gap::to seq[%d].}", us_last_seqnum);

    while (us_seq_num != us_last_seqnum) {
        pst_rx_buf = hmac_remove_frame_from_reorder_q(pst_ba_rx_hdl, us_seq_num);
        if (pst_rx_buf != NULL) {
            pst_netbuf = oal_netbuf_peek(&pst_rx_buf->st_netbuf_head);
            if (oal_unlikely(pst_netbuf == NULL)) {
                oam_warning_log1(pst_vap->uc_vap_id, OAM_SF_BA,
                                 "{hmac_ba_send_frames_with_gap::gap[%d].\r\n}",
                                 us_seq_num);

                us_seq_num = mac_ba_seqno_add(us_seq_num, 1);
                pst_rx_buf->uc_num_buf = 0;

                continue;
            }
            hmac_ba_send_frames_add_list(pst_rx_buf, pst_netbuf, pst_netbuf_header);
            pst_rx_buf->uc_num_buf = 0;
            uc_num_frms++;
        }

        us_seq_num = mac_ba_seqno_add(us_seq_num, 1);
    }

    oam_warning_log4(pst_vap->uc_vap_id, OAM_SF_BA,
        "{hmac_ba_send_frames_with_gap::old_baw_start[%d], new_baw_start[%d], uc_num_frms[%d], uc_mpdu_cnt=%d.}",
        pst_ba_rx_hdl->us_baw_start, us_last_seqnum, uc_num_frms, pst_ba_rx_hdl->uc_mpdu_cnt);

    return uc_num_frms;
}

/*
 * 函 数 名  : hmac_ba_send_frames_in_order
 * 功能描述  : All MSDUs with sequence number starting from the
               start of the BA-Rx window are processed in order and
               are added to the list which will be passed up to hmac.
               Processing is stopped when the first missing MSDU is encountered.
 * 1.日    期  : 2013年4月11日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint16_t hmac_ba_send_frames_in_order(hmac_ba_rx_stru *pst_ba_rx_hdl,
                                                 oal_netbuf_head_stru *pst_netbuf_header,
                                                 mac_vap_stru *pst_vap)
{
    uint16_t us_seq_num;
    hmac_rx_buf_stru *pst_rx_buf = NULL;
    uint8_t uc_loop_index;
    oal_netbuf_stru *pst_netbuf = NULL;

    us_seq_num = pst_ba_rx_hdl->us_baw_start;

    while ((pst_rx_buf = hmac_remove_frame_from_reorder_q(pst_ba_rx_hdl, us_seq_num)) != NULL) {
        us_seq_num = hmac_ba_seqno_add(us_seq_num, 1);
        pst_netbuf = oal_netbuf_peek(&pst_rx_buf->st_netbuf_head);
        if (pst_netbuf == NULL) {
            oam_warning_log1(pst_vap->uc_vap_id, OAM_SF_BA,
                             "{hmac_ba_send_frames_in_order::[%d] slot error.}",
                             us_seq_num);
            pst_rx_buf->uc_num_buf = 0;
            continue;
        }

        for (uc_loop_index = 0; uc_loop_index < pst_rx_buf->uc_num_buf; uc_loop_index++) {
            pst_netbuf = oal_netbuf_delist(&pst_rx_buf->st_netbuf_head);
            oal_mem_netbuf_trace(pst_netbuf, OAL_FALSE);
            if (pst_netbuf != NULL) {
                oal_netbuf_list_tail_nolock(pst_netbuf_header, pst_netbuf);
            }
        }

        pst_rx_buf->uc_num_buf = 0;
    }

    return us_seq_num;
}

/*
 * 函 数 名  : hmac_ba_buffer_rx_frame
 * 功能描述  : 将报文缓存至重排序队列
 * 1.日    期  : 2014年11月25日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC OAL_INLINE void hmac_ba_buffer_rx_frame(hmac_ba_rx_stru *pst_ba_rx_hdl,
                                                   mac_rx_ctl_stru *pst_cb_ctrl,
                                                   oal_netbuf_head_stru *pst_netbuf_header,
                                                   uint16_t us_seq_num)
{
    hmac_rx_buf_stru *pst_rx_netbuf = NULL;
    oal_netbuf_stru *pst_netbuf = NULL;
    uint8_t uc_netbuf_index;

    /* Get the pointer to the buffered packet */
    pst_rx_netbuf = hmac_ba_buffer_frame_in_reorder(pst_ba_rx_hdl, us_seq_num, pst_cb_ctrl);

    /* all buffers of this frame must be deleted from the buf list */
    for (uc_netbuf_index = pst_cb_ctrl->bit_buff_nums; uc_netbuf_index > 0; uc_netbuf_index--) {
        pst_netbuf = oal_netbuf_delist_nolock(pst_netbuf_header);

        oal_mem_netbuf_trace(pst_netbuf, OAL_TRUE);
        if (oal_unlikely(pst_netbuf != NULL)) {
            /* enqueue reorder queue */
            oal_netbuf_add_to_list_tail(pst_netbuf, &pst_rx_netbuf->st_netbuf_head);
        } else {
            oam_error_log0(pst_cb_ctrl->uc_mac_vap_id, OAM_SF_BA, "{hmac_ba_buffer_rx_frame:netbuff error in amsdu.}");
        }
    }

    if (oal_netbuf_list_len(&pst_rx_netbuf->st_netbuf_head) != pst_rx_netbuf->uc_num_buf) {
        oam_warning_log2(pst_cb_ctrl->uc_mac_vap_id, OAM_SF_BA, "{hmac_ba_buffer_rx_frame: list_len=%d numbuf=%d}",
                         oal_netbuf_list_len(&pst_rx_netbuf->st_netbuf_head), pst_rx_netbuf->uc_num_buf);
        pst_rx_netbuf->uc_num_buf = oal_netbuf_list_len(&pst_rx_netbuf->st_netbuf_head);
    }
}

/*
 * 函 数 名  : hmac_ba_reorder_rx_data
 * 功能描述  : 将重排序队列中可以上传的报文加到buf链表的尾部
 * 1.日    期  : 2013年4月11日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC OAL_INLINE void hmac_ba_reorder_rx_data(hmac_ba_rx_stru *pst_ba_rx_hdl,
                                                   oal_netbuf_head_stru *pst_netbuf_header,
                                                   mac_vap_stru *pst_vap,
                                                   uint16_t us_seq_num)
{
    uint8_t uc_seqnum_pos;
    uint16_t us_temp_winstart;

    uc_seqnum_pos = hmac_ba_seqno_bound_chk(pst_ba_rx_hdl->us_baw_start, pst_ba_rx_hdl->us_baw_end, us_seq_num);
    if (uc_seqnum_pos == DMAC_BA_BETWEEN_SEQLO_SEQHI) {
        pst_ba_rx_hdl->us_baw_start = hmac_ba_send_frames_in_order(pst_ba_rx_hdl, pst_netbuf_header, pst_vap);
        pst_ba_rx_hdl->us_baw_end = mac_ba_seqno_add(pst_ba_rx_hdl->us_baw_start, (pst_ba_rx_hdl->us_baw_size - 1));
    } else if (uc_seqnum_pos == DMAC_BA_GREATER_THAN_SEQHI) {
        us_temp_winstart = hmac_ba_seqno_sub(us_seq_num, (pst_ba_rx_hdl->us_baw_size - 1));

        hmac_ba_send_frames_with_gap(pst_ba_rx_hdl, pst_netbuf_header, us_temp_winstart, pst_vap);
        pst_ba_rx_hdl->us_baw_start = us_temp_winstart;
        pst_ba_rx_hdl->us_baw_start = hmac_ba_send_frames_in_order(pst_ba_rx_hdl, pst_netbuf_header, pst_vap);
        pst_ba_rx_hdl->us_baw_end = hmac_ba_seqno_add(pst_ba_rx_hdl->us_baw_start, (pst_ba_rx_hdl->us_baw_size - 1));
    } else {
        oam_warning_log3(pst_vap->uc_vap_id, OAM_SF_BA,
            "{hmac_ba_reorder_rx_data::else branch seqno[%d] ws[%d] we[%d].}",
            us_seq_num, pst_ba_rx_hdl->us_baw_start, pst_ba_rx_hdl->us_baw_end);
    }
}

/*
 * 函 数 名  : hmac_ba_flush_reorder_q
 * 功能描述  : 冲刷重排序队列
 * 1.日    期  : 2013年4月15日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC void hmac_ba_flush_reorder_q(hmac_ba_rx_stru *pst_rx_ba)
{
    hmac_rx_buf_stru *pst_rx_buf = NULL;
    uint16_t us_index;

    for (us_index = 0; us_index < WLAN_AMPDU_RX_HE_BUFFER_SIZE; us_index++) {
        pst_rx_buf = hmac_get_ba_rx_dhl(pst_rx_ba, us_index);
        if (pst_rx_buf->in_use == OAL_TRUE) {
            hmac_rx_free_netbuf_list(&pst_rx_buf->st_netbuf_head, pst_rx_buf->uc_num_buf);
            pst_rx_buf->in_use = OAL_FALSE;
            pst_rx_buf->uc_num_buf = 0;
            pst_rx_ba->uc_mpdu_cnt--;
        }
    }

    if (pst_rx_ba->uc_mpdu_cnt != 0) {
        oam_warning_log1(0, OAM_SF_BA, "{hmac_ba_flush_reorder_q:: %d mpdu cnt left.}", pst_rx_ba->uc_mpdu_cnt);
    }
}
/*
 * 函 数 名  : hmac_ba_check_rx_aggr
 * 功能描述  : 检查是否能做ba重排序处理
 * 1.日    期  : 2013年11月28日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC OAL_INLINE uint32_t hmac_ba_check_rx_aggr(mac_vap_stru *pst_vap,
                                                     mac_ieee80211_frame_stru *pst_frame_hdr)
{
    /* 该vap是否是ht */
    if (OAL_FALSE == mac_mib_get_HighThroughputOptionImplemented(pst_vap)) {
        oam_info_log0(pst_vap->uc_vap_id, OAM_SF_BA, "{hmac_ba_check_rx_aggr::ht not supported by this vap.}");
        return OAL_FAIL;
    }

    /* 判断该帧是不是qos帧 */
    if ((WLAN_FC0_SUBTYPE_QOS | WLAN_FC0_TYPE_DATA) != ((uint8_t *)pst_frame_hdr)[0]) {
        oam_info_log0(pst_vap->uc_vap_id, OAM_SF_BA, "{hmac_ba_check_rx_aggr::not qos data.}");
        return OAL_FAIL;
    }

    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_ba_need_update_hw_baw
 * 功能描述  : 判断是否需要更新硬件的BAW
 * 1.日    期  : 2014年8月16日
 *   修改内容  : 新生成函数
 */
OAL_STATIC OAL_INLINE oal_bool_enum_uint8 hmac_ba_need_update_hw_baw(hmac_ba_rx_stru *pst_ba_rx_hdl,
                                                                     uint16_t us_seq_num)
{
    if ((OAL_TRUE == hmac_ba_seqno_lt(us_seq_num, pst_ba_rx_hdl->us_baw_start)) &&
        (OAL_FALSE == hmac_ba_rx_seqno_lt(us_seq_num, pst_ba_rx_hdl->us_baw_start))) {
        return OAL_TRUE;
    }

    return OAL_FALSE;
}

uint32_t hmac_ba_filter_and_reorder_rx_data(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    mac_rx_ctl_stru *rx_ctrl, oal_netbuf_head_stru *netbuf_header, hmac_ba_rx_stru *ba_rx_hdl, uint16_t seq_num,
    oal_bool_enum_uint8 *pen_is_ba_buf, uint8_t tid)
{
    uint16_t baw_start_temp;

    /* 暂时保存BA窗口的序列号，用于鉴别是否有帧上报 */
    baw_start_temp = ba_rx_hdl->us_baw_start;

    /* duplicate frame判断 */
    if (OAL_TRUE == hmac_ba_rx_seqno_lt(seq_num, ba_rx_hdl->us_baw_start)) {
        /* 上次非定时器上报，直接删除duplicate frame帧，否则，直接上报 */
        if (ba_rx_hdl->en_timer_triggered == OAL_FALSE) {
            /* 确实已经收到该帧 */
            /*  新增了bitmap记录收包 防止dup误丢包 */
            if (hmac_ba_isset(ba_rx_hdl, seq_num) == OAL_TRUE) {
                HMAC_USER_STATS_PKT_INCR(hmac_user->rx_pkt_drop, 1);
                return OAL_FAIL;
            }
        }

        return OAL_SUCC;
    }

    /* 更新tail */
    if (OAL_TRUE == hmac_ba_seqno_lt(ba_rx_hdl->us_baw_tail, seq_num)) {
        ba_rx_hdl->us_baw_tail = seq_num;
    }

    /* 接收到的帧的序列号等于BAW_START，并且缓存队列帧个数为0，则直接上报给HMAC */
    if ((seq_num == ba_rx_hdl->us_baw_start) && (ba_rx_hdl->uc_mpdu_cnt == 0)
        /* offload 下amsdu帧由于可能多个buffer组成，一律走重排序 */
        && (rx_ctrl->bit_amsdu_enable == OAL_FALSE)) {
        ba_rx_hdl->us_baw_start = mac_ba_seqno_add(ba_rx_hdl->us_baw_start, 1);
        ba_rx_hdl->us_baw_end = mac_ba_seqno_add(ba_rx_hdl->us_baw_end, 1);
    } else {
        /* Buffer the new MSDU */
        *pen_is_ba_buf = OAL_TRUE;

        /* buffer frame to reorder */
        hmac_ba_buffer_rx_frame(ba_rx_hdl, rx_ctrl, netbuf_header, seq_num);

        /* put the reordered netbufs to the end of the list */
        hmac_ba_reorder_rx_data(ba_rx_hdl, netbuf_header, &hmac_vap->st_vap_base_info, seq_num);

        /* Check for Sync loss and flush the reorder queue when one is detected */
        if ((ba_rx_hdl->us_baw_tail == mac_ba_seqno_sub(ba_rx_hdl->us_baw_start, 1)) &&
            (ba_rx_hdl->uc_mpdu_cnt > 0)) {
            oam_warning_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_BA,
                             "{hmac_ba_filter_serv::Sync loss and flush the reorder queue.}");
            hmac_ba_flush_reorder_q(ba_rx_hdl);
        }

        /* 重排序队列刷新后,如果队列中有帧那么启动定时器 */
        if (ba_rx_hdl->uc_mpdu_cnt > 0) {
            hmac_reorder_ba_timer_start(hmac_vap, hmac_user, tid);
        }
    }

    if (baw_start_temp != ba_rx_hdl->us_baw_start) {
        ba_rx_hdl->en_timer_triggered = OAL_FALSE;
    }

    return OAL_SUCC;
}
/*
 * 函 数 名  : hmac_ba_filter_serv
 * 功能描述  : 过滤ampdu的每一个mpdu 有未确认报文需要入重传队列
 * 1.日    期  : 2014年11月25日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_ba_filter_serv(hmac_user_stru *hmac_user, mac_rx_ctl_stru *rx_ctrl, oal_netbuf_head_stru *netbuf_header,
    oal_bool_enum_uint8 *pen_is_ba_buf)
{
    hmac_ba_rx_stru *ba_rx_hdl = NULL;
    uint16_t seq_num;
    uint8_t tid;
    mac_ieee80211_frame_stru *frame_hdr = NULL;
    hmac_vap_stru *hmac_vap = NULL;

    if (oal_any_null_ptr3(netbuf_header, rx_ctrl, pen_is_ba_buf)) {
        oam_error_log0(0, OAM_SF_BA, "{hmac_ba_filter_serv::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    frame_hdr = (mac_ieee80211_frame_stru *)mac_get_rx_cb_mac_header_addr(rx_ctrl);
    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(rx_ctrl->uc_mac_vap_id);
    if (oal_unlikely(hmac_vap == NULL) || oal_unlikely(frame_hdr == NULL)) {
        oam_error_log0(0, OAM_SF_BA, "{hmac_ba_filter_serv::hmac_vap or frame_hdr null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (hmac_ba_check_rx_aggr(&hmac_vap->st_vap_base_info, frame_hdr) != OAL_SUCC) {
        return OAL_SUCC;
    }

    /* 考虑四地址情况获取报文的tid */
    tid = hmac_mac_get_tid_from_frame_hdr((uint8_t *)frame_hdr);

    ba_rx_hdl = hmac_user->ast_tid_info[tid].pst_ba_rx_info;
    if (ba_rx_hdl == NULL) {
        return OAL_SUCC;
    }
    if (ba_rx_hdl->en_ba_status != DMAC_BA_COMPLETE) {
        oam_warning_log1(rx_ctrl->uc_mac_vap_id, OAM_SF_BA, "{hmac_ba_filter_serv::ba_status = %d.",
            ba_rx_hdl->en_ba_status);
        return OAL_SUCC;
    }

    seq_num = mac_get_seq_num((uint8_t *)frame_hdr);

    /*  兼容接收方向聚合和分片共存的情况 */
    if ((oal_bool_enum_uint8)frame_hdr->st_frame_control.bit_more_frag == OAL_TRUE) {
        oam_warning_log1(rx_ctrl->uc_mac_vap_id, OAM_SF_BA,
            "{hmac_ba_filter_serv::We get a frag_frame[seq_num=%d] When BA_session is set UP!", seq_num);
        return OAL_SUCC;
    }

    return hmac_ba_filter_and_reorder_rx_data(hmac_vap, hmac_user, rx_ctrl, netbuf_header, ba_rx_hdl,
        seq_num, pen_is_ba_buf, tid);
}

/*
 * 函 数 名  : hmac_reorder_ba_rx_buffer_bar
 * 功能描述  : This function reorders the Reciver buffer and sends frames to the higher
 *             layer on reception of a Block-Ack-Request frame. It also updates the
 *             receiver buffer window.
 * 1.日    期  : 2014年11月29日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void hmac_reorder_ba_rx_buffer_bar(hmac_ba_rx_stru *pst_rx_ba, uint16_t us_start_seq_num,
    mac_vap_stru *pst_vap)
{
    oal_netbuf_head_stru st_netbuf_head;
    uint8_t uc_seqnum_pos;

    if (pst_rx_ba == NULL) {
        oam_warning_log0(0, OAM_SF_BA, "{hmac_reorder_ba_rx_buffer_bar::rcv a bar, but ba not set up.}");
        return;
    }

    /* 针对 BAR 的SSN和窗口的start_num相等时，不需要移窗 */
    if (pst_rx_ba->us_baw_start == us_start_seq_num) {
        oam_info_log0(0, OAM_SF_BA, "{hmac_reorder_ba_rx_buffer_bar::seq is equal to start num.}");
        return;
    }

    oal_netbuf_list_head_init(&st_netbuf_head);

    uc_seqnum_pos = hmac_ba_seqno_bound_chk(pst_rx_ba->us_baw_start, pst_rx_ba->us_baw_end, us_start_seq_num);
    /* 针对BAR的的SSN在窗口内才移窗 */
    if (uc_seqnum_pos == DMAC_BA_BETWEEN_SEQLO_SEQHI) {
        hmac_ba_send_frames_with_gap(pst_rx_ba, &st_netbuf_head, us_start_seq_num, pst_vap);
        pst_rx_ba->us_baw_start = us_start_seq_num;
        pst_rx_ba->us_baw_start = hmac_ba_send_frames_in_order(pst_rx_ba, &st_netbuf_head, pst_vap);
        pst_rx_ba->us_baw_end = hmac_ba_seqno_add(pst_rx_ba->us_baw_start, (pst_rx_ba->us_baw_size - 1));

        oam_warning_log3(pst_vap->uc_vap_id, OAM_SF_BA,
            "{hmac_reorder_ba_rx_buffer_bar::receive a bar, us_baw_start=%d us_baw_end=%d. us_seq_num=%d.}",
            pst_rx_ba->us_baw_start, pst_rx_ba->us_baw_end, us_start_seq_num);
        hmac_rx_lan_frame(&st_netbuf_head);
    } else if (uc_seqnum_pos == DMAC_BA_GREATER_THAN_SEQHI) {
        /* 异常 */
        oam_warning_log3(pst_vap->uc_vap_id, OAM_SF_BA,
            "{hmac_reorder_ba_rx_buffer_bar::recv a bar and ssn out of winsize,baw_start=%d baw_end=%d seq_num=%d}",
            pst_rx_ba->us_baw_start, pst_rx_ba->us_baw_end, us_start_seq_num);
    } else {
        oam_warning_log3(pst_vap->uc_vap_id, OAM_SF_BA,
            "{hmac_reorder_ba_rx_buffer_bar::recv a bar ssn less than baw_start,baw_start=%d end=%d seq_num=%d}",
            pst_rx_ba->us_baw_start, pst_rx_ba->us_baw_end, us_start_seq_num);
    }
}

/*
 * 函 数 名  : hmac_ba_rx_prepare_bufflist
 * 功能描述  : 从重排序队列中获取skb链
 * 1.日    期  : 2014年5月21日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t hmac_ba_rx_prepare_bufflist(hmac_vap_stru *pst_hmac_vap,
                                                hmac_rx_buf_stru *pst_rx_buf,
                                                oal_netbuf_head_stru *pst_netbuf_head)
{
    oal_netbuf_stru *pst_netbuf;
    uint8_t uc_loop_index;

    pst_netbuf = oal_netbuf_peek(&pst_rx_buf->st_netbuf_head);
    if (pst_netbuf == NULL) {
        oam_warning_log0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_BA,
                         "{hmac_ba_rx_prepare_bufflist::pst_netbuf null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    for (uc_loop_index = 0; uc_loop_index < pst_rx_buf->uc_num_buf; uc_loop_index++) {
        pst_netbuf = oal_netbuf_delist(&pst_rx_buf->st_netbuf_head);
        if (pst_netbuf != NULL) {
            oal_netbuf_add_to_list_tail(pst_netbuf, pst_netbuf_head);
        } else {
            oam_warning_log0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_BA,
                             "{hmac_ba_rx_prepare_bufflist::uc_num_buf in reorder list is error.}");
        }
    }

    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_ba_send_reorder_timeout
 * 功能描述  : 上报重排序队列中超时的报文
 * 1.日    期  : 2014年5月19日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_ba_send_reorder_timeout(hmac_ba_rx_stru *pst_rx_ba, hmac_vap_stru *pst_hmac_vap,
    hmac_ba_alarm_stru *pst_alarm_data, uint32_t *timeout)
{
    uint32_t time_diff, rx_timeout, ret;
    oal_netbuf_head_stru st_netbuf_head;
    uint16_t us_baw_head, us_baw_end, us_baw_start; /* 保存最初的窗口起始序列号 */
    hmac_rx_buf_stru *pst_rx_buf = NULL;
    uint8_t uc_buff_count = 0;
    uint16_t uc_send_count = 0;

    oal_netbuf_list_head_init(&st_netbuf_head);
    us_baw_head = pst_rx_ba->us_baw_start;
    us_baw_start = pst_rx_ba->us_baw_start;
    us_baw_end = hmac_ba_seqno_add(pst_rx_ba->us_baw_tail, 1);
    rx_timeout = (*timeout);

    oal_spin_lock(&pst_rx_ba->st_ba_lock);

    while (us_baw_head != us_baw_end) {
        pst_rx_buf = hmac_get_frame_from_reorder_q(pst_rx_ba, us_baw_head);
        if (pst_rx_buf == NULL) {
            uc_buff_count++;
            us_baw_head = hmac_ba_seqno_add(us_baw_head, 1);
            continue;
        }

        /* 如果冲排序队列中的帧滞留时间小于定时器超时时间,那么暂时不强制flush */
        time_diff = (uint32_t)oal_time_get_stamp_ms() - pst_rx_buf->rx_time;
        if (time_diff < rx_timeout) {
            *timeout = (rx_timeout - time_diff);
            break;
        }

        pst_rx_ba->uc_mpdu_cnt--;
        pst_rx_buf->in_use = 0;

        ret = hmac_ba_rx_prepare_bufflist(pst_hmac_vap, pst_rx_buf, &st_netbuf_head);
        if (ret != OAL_SUCC) {
            uc_buff_count++;
            us_baw_head = hmac_ba_seqno_add(us_baw_head, 1);
            continue;
        }

        uc_send_count++;
        uc_buff_count++;
        us_baw_head = hmac_ba_seqno_add(us_baw_head, 1);
        pst_rx_ba->us_baw_start = hmac_ba_seqno_add(pst_rx_ba->us_baw_start, uc_buff_count);
        pst_rx_ba->us_baw_end = hmac_ba_seqno_add(pst_rx_ba->us_baw_start, (pst_rx_ba->us_baw_size - 1));

        uc_buff_count = 0;
    }

    oal_spin_unlock(&pst_rx_ba->st_ba_lock);

    /* 判断本次定时器超时是否有帧上报 */
    if (us_baw_start != pst_rx_ba->us_baw_start) {
        pst_rx_ba->en_timer_triggered = OAL_TRUE;
        oam_warning_log4(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_BA,
            "{hmac_ba_send_reorder_timeout::new baw_start:%d, old baw_start:%d, rx_timeout:%d, send mpdu cnt:%d.}",
            pst_rx_ba->us_baw_start, us_baw_start, rx_timeout, uc_send_count);
    }

    hmac_rx_lan_frame(&st_netbuf_head);

    return OAL_SUCC;
}

/*
 * 功能描述  : ba会话超时处理
 * 1.日    期  : 2013年4月12日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_ba_timeout_fn(void *p_arg)
{
    hmac_ba_rx_stru *pst_rx_ba = NULL;
    hmac_vap_stru *hmac_vap = NULL;
    hmac_user_stru *hmac_user = NULL;
    hmac_ba_alarm_stru *alarm_data = NULL;
    mac_delba_initiator_enum_uint8 en_direction;
    uint8_t rx_tid;
    mac_device_stru *mac_device = NULL;
    uint32_t timeout;

    alarm_data = (hmac_ba_alarm_stru *)p_arg;
    en_direction = alarm_data->en_direction;
    rx_tid = alarm_data->uc_tid;
    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(alarm_data->uc_vap_id);
    hmac_user = (hmac_user_stru *)mac_res_get_hmac_user(alarm_data->us_mac_user_idx);
    if (OAL_TRUE != hmac_ba_timeout_param_is_valid(alarm_data, hmac_vap, hmac_user, rx_tid)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    mac_device = mac_res_get_dev(hmac_vap->st_vap_base_info.uc_device_id);
    if (mac_device == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (en_direction == MAC_RECIPIENT_DELBA) {
        pst_rx_ba = (hmac_ba_rx_stru *)alarm_data->pst_ba;
        if (pst_rx_ba == NULL) {
            oam_error_log0(0, OAM_SF_BA, "{hmac_ba_timeout_fn::pst_rx_ba is null.}");
            return OAL_ERR_CODE_PTR_NULL;
        }

        /* 接收业务量较少时只能靠超时定时器冲刷重排序队列,为改善游戏帧延时,需要将超时时间设小 */
        if (OAL_FALSE == hmac_wifi_rx_is_busy()) {
            timeout = hmac_vap->us_rx_timeout_min[WLAN_WME_TID_TO_AC(rx_tid)];
        } else {
            timeout = hmac_vap->us_rx_timeout[WLAN_WME_TID_TO_AC(rx_tid)];
        }

        if (pst_rx_ba->uc_mpdu_cnt > 0) {
            wlan_chip_ba_send_reorder_timeout(pst_rx_ba, hmac_vap, alarm_data, &timeout);
        }

        /* 若重排序队列刷新后,依然有缓存帧则需要重启定时器;
           若重排序队列无帧则为了节省功耗不启动定时器,在有帧入队时重启 */
        if (pst_rx_ba->uc_mpdu_cnt > 0) {
            oal_spin_lock(&(hmac_user->ast_tid_info[rx_tid].st_ba_timer_lock));
            /* 此处不需要判断定时器是否已经启动,如果未启动则启动定时器;
               如果此定时器已经启动 */
            frw_timer_create_timer_m(&(hmac_user->ast_tid_info[rx_tid].st_ba_timer),
                hmac_ba_timeout_fn, timeout, alarm_data,
                OAL_FALSE, OAM_MODULE_ID_HMAC, mac_device->core_id);
            oal_spin_unlock(&(hmac_user->ast_tid_info[rx_tid].st_ba_timer_lock));
        }
    } else {
        /* tx ba不删除 */
        frw_timer_create_timer_m(&(hmac_user->ast_tid_info[rx_tid].st_ba_timer),
            hmac_ba_timeout_fn, hmac_vap->us_rx_timeout[WLAN_WME_TID_TO_AC(rx_tid)],
            alarm_data, OAL_FALSE, OAM_MODULE_ID_HMAC, mac_device->core_id);
    }

    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_ba_reset_rx_handle
 * 功能描述  : 重置rx ba结构体
 * 1.日    期  : 2014年12月5日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_ba_reset_rx_handle(mac_device_stru *pst_mac_device,
                                 hmac_user_stru *pst_hmac_user, uint8_t uc_tid,
                                 oal_bool_enum_uint8 en_is_aging)
{
    hmac_vap_stru *pst_hmac_vap = NULL;
    mac_chip_stru *pst_mac_chip = NULL;
    oal_bool_enum en_need_del_lut = OAL_TRUE;
    hmac_ba_rx_stru *pst_ba_rx_info = NULL;

    if (uc_tid >= WLAN_TID_MAX_NUM) {
        oam_error_log1(0, OAM_SF_BA, "{hmac_ba_reset_rx_handle::tid %d overflow.}", uc_tid);
        return OAL_ERR_CODE_ARRAY_OVERFLOW;
    }

    pst_ba_rx_info = pst_hmac_user->ast_tid_info[uc_tid].pst_ba_rx_info;
    if (oal_unlikely((pst_ba_rx_info == NULL) || (pst_ba_rx_info->en_is_ba != OAL_TRUE))) {
        oam_warning_log0(0, OAM_SF_BA, "{hmac_ba_reset_rx_handle::rx ba not set yet.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_ba_rx_info->st_alarm_data.uc_vap_id);
    if (oal_unlikely(pst_hmac_vap == NULL)) {
        oam_error_log0(0, OAM_SF_BA, "{hmac_ba_reset_rx_handle::pst_hmac_vap is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* Step1: disable the flag of ba session */
    pst_ba_rx_info->en_is_ba = OAL_FALSE;

    /* Step2: flush reorder q */
    hmac_ba_flush_reorder_q(pst_ba_rx_info);

    if (pst_ba_rx_info->uc_lut_index == MAC_INVALID_RX_BA_LUT_INDEX) {
        en_need_del_lut = OAL_FALSE;
        oam_warning_log1(0, OAM_SF_BA, "{hmac_ba_reset_rx_handle::no need to del lut index, lut index[%d]}\n",
                         pst_ba_rx_info->uc_lut_index);
    }

    /* Step3: if lut index is valid, del lut index alloc before */
    if ((pst_ba_rx_info->uc_ba_policy == MAC_BA_POLICY_IMMEDIATE) && (en_need_del_lut == OAL_TRUE)) {
        pst_mac_chip = hmac_res_get_mac_chip(pst_mac_device->uc_chip_id);
        if (pst_mac_chip == NULL) {
            return OAL_ERR_CODE_PTR_NULL;
        }
        hmac_ba_del_lut_index(pst_mac_chip->st_lut_table.auc_rx_ba_lut_idx_table, pst_ba_rx_info->uc_lut_index);
    }

    /* Step4: dec the ba session cnt maitence in device struc */
    hmac_rx_ba_session_decr(pst_hmac_vap, uc_tid);

    oal_spin_lock(&(pst_hmac_user->ast_tid_info[uc_tid].st_ba_timer_lock));
    /* Step5: Del Timer */
    if (pst_hmac_user->ast_tid_info[uc_tid].st_ba_timer.en_is_registerd == OAL_TRUE) {
        frw_timer_immediate_destroy_timer_m(&(pst_hmac_user->ast_tid_info[uc_tid].st_ba_timer));
    }
    oal_spin_unlock(&(pst_hmac_user->ast_tid_info[uc_tid].st_ba_timer_lock));

    /* Step6: Free rx handle */
    oal_mem_free_m(pst_ba_rx_info, OAL_TRUE);

    pst_hmac_user->ast_tid_info[uc_tid].pst_ba_rx_info = NULL;

    return OAL_SUCC;
}

OAL_STATIC uint8_t hmac_mgmt_check_rx_ba_policy(hmac_vap_stru *pst_hmac_vap, hmac_ba_rx_stru *pst_ba_rx_info)
{
    /* 立即块确认判断 */
    if (pst_ba_rx_info->uc_ba_policy == MAC_BA_POLICY_IMMEDIATE) {
        if (OAL_FALSE == mac_mib_get_dot11ImmediateBlockAckOptionImplemented(&pst_hmac_vap->st_vap_base_info)) {
            /* 不支持立即块确认 */
            oam_warning_log0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_BA,
                             "{hmac_mgmt_check_set_rx_ba_ok::not support immediate Block Ack.}");
            return MAC_INVALID_REQ_PARAMS;
        } else {
            if (pst_ba_rx_info->en_back_var != MAC_BACK_COMPRESSED) {
                /* 不支持非压缩块确认 */
                oam_warning_log0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_BA,
                                 "{hmac_mgmt_check_set_rx_ba_ok::not support non-Compressed Block Ack.}");
                return MAC_REQ_DECLINED;
            }
        }
    } else if (pst_ba_rx_info->uc_ba_policy == MAC_BA_POLICY_DELAYED) {
        /* 延迟块确认不支持 */
        oam_warning_log0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_BA,
                         "{hmac_mgmt_check_set_rx_ba_ok::not support delayed Block Ack.}");
        return MAC_INVALID_REQ_PARAMS;
    }
    return MAC_SUCCESSFUL_STATUSCODE;
}

/*
 * 函 数 名  : hmac_mgmt_check_set_rx_ba_ok
 * 功能描述  : 从空口接收ADDBA_REQ帧的处理函数
 * 1.日    期  : 2014年11月28日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint8_t hmac_mgmt_check_set_rx_ba_ok(hmac_vap_stru *pst_hmac_vap,
                                     hmac_user_stru *pst_hmac_user,
                                     hmac_ba_rx_stru *pst_ba_rx_info,
                                     mac_device_stru *pst_device,
                                     hmac_tid_stru *pst_tid_info)
{
    mac_chip_stru *pst_mac_chip = NULL;
    uint8_t ba_status;
    uint8_t ret;

    pst_ba_rx_info->uc_lut_index = MAC_INVALID_RX_BA_LUT_INDEX;
    ret = hmac_mgmt_check_rx_ba_policy(pst_hmac_vap, pst_ba_rx_info);
    if (ret != MAC_SUCCESSFUL_STATUSCODE) {
        return ret;
    }

    pst_mac_chip = hmac_res_get_mac_chip(pst_device->uc_chip_id);
    if (pst_mac_chip == NULL) {
        return MAC_REQ_DECLINED;
    }
    if (mac_mib_get_RxBASessionNumber(&pst_hmac_vap->st_vap_base_info) > g_wlan_spec_cfg->max_rx_ba) {
        oam_warning_log1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_BA,
                         "{hmac_mgmt_check_set_rx_ba_ok::uc_rx_ba_session_num[%d] is up to max.}\r\n",
                         mac_mib_get_RxBASessionNumber(&pst_hmac_vap->st_vap_base_info));
        return MAC_REQ_DECLINED;
    }

    /* 获取BA LUT INDEX */
    pst_ba_rx_info->uc_lut_index = hmac_ba_get_lut_index(pst_mac_chip->st_lut_table.auc_rx_ba_lut_idx_table,
                                                         0, HAL_MAX_RX_BA_LUT_SIZE);

    if ((pst_ba_rx_info->uc_lut_index == MAC_INVALID_RX_BA_LUT_INDEX) &&
        (wlan_chip_ba_need_check_lut_idx() == OAL_TRUE)) {
        oam_error_log0(pst_hmac_vap->st_vap_base_info.uc_vap_id, 0, "{hmac_mgmt_check_set_rx_ba_ok:ba lut idx tb full");
        return MAC_REQ_DECLINED;
    }
    ba_status = hmac_btcoex_check_addba_req(pst_hmac_vap, pst_hmac_user);
    if (ba_status != MAC_SUCCESSFUL_STATUSCODE) {
        return ba_status;
    }
    /* 该tid下不允许建BA，配置命令需求 */
    if (pst_tid_info->en_ba_handle_rx_enable == OAL_FALSE) {
        oam_warning_log2(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_BA,
                         "{hmac_mgmt_check_set_rx_ba_ok::uc_tid_no[%d] user_idx[%d] is not available.}",
                         pst_tid_info->uc_tid_no, pst_tid_info->us_hmac_user_idx);
        return MAC_REQ_DECLINED;
    }
    return MAC_SUCCESSFUL_STATUSCODE;
}

/*
 * 函 数 名  : hmac_up_rx_bar
 * 1.日    期  : 2015年1月3日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void hmac_up_rx_bar(hmac_vap_stru *pst_hmac_vap, dmac_rx_ctl_stru *pst_rx_ctl, oal_netbuf_stru *pst_netbuf)
{
    uint8_t *puc_payload = NULL;
    mac_ieee80211_frame_stru *pst_frame_hdr = NULL;
    uint8_t *puc_sa_addr = NULL;
    uint8_t uc_tidno;
    hmac_user_stru *pst_ta_user = NULL;
    uint16_t us_start_seqnum;
    hmac_ba_rx_stru *pst_ba_rx_info = NULL;
    mac_rx_ctl_stru *pst_rx_info = NULL;
    uint16_t us_frame_len;

    if (wlan_chip_check_need_process_bar() == OAL_FALSE) {
        oam_info_log0(0, OAM_SF_BA, "{hmac_up_rx_bar: not need process bar, return.}");
        return;
    }

    pst_frame_hdr = (mac_ieee80211_frame_stru *)mac_get_rx_cb_mac_hdr(&(pst_rx_ctl->st_rx_info));
    puc_sa_addr = pst_frame_hdr->auc_address2;
    pst_rx_info = (mac_rx_ctl_stru *)(&(pst_rx_ctl->st_rx_info));
    us_frame_len = pst_rx_info->us_frame_len - pst_rx_info->uc_mac_header_len;

    /*  获取用户指针 */
    pst_ta_user = mac_vap_get_hmac_user_by_addr(&(pst_hmac_vap->st_vap_base_info), puc_sa_addr, WLAN_MAC_ADDR_LEN);
    if (pst_ta_user == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_up_rx_bar::pst_ta_user  is null.}");
        return;
    }

    /* BAR Control(2) +  BlockAck Starting seq num)(2) */
    if (us_frame_len < 4) {  /* mac header帧头长度小于4，帧格式错误 */
        oam_warning_log2(0, OAM_SF_ANY, "{hmac_up_rx_bar:frame len err. frame len[%d], machdr len[%d].}",
            pst_rx_info->us_frame_len, pst_rx_info->uc_mac_header_len);
        return;
    }

    /* 获取帧头和payload指针 */
    puc_payload = mac_get_rx_payload_addr(&(pst_rx_ctl->st_rx_info), pst_netbuf);

    /*************************************************************************/
    /*                     BlockAck Request Frame Format                     */
    /* --------------------------------------------------------------------  */
    /* |Frame Control|Duration|DA|SA|BAR Control|BlockAck Starting    |FCS|  */
    /* |             |        |  |  |           |Sequence number      |   |  */
    /* --------------------------------------------------------------------  */
    /* | 2           |2       |6 |6 |2          |2                    |4  |  */
    /* --------------------------------------------------------------------  */
    /*                                                                       */
    /*************************************************************************/
    uc_tidno = (puc_payload[1] & 0xF0) >> 4; /* 取4~7bit位为TID编号类别 */
    if (uc_tidno >= WLAN_TID_MAX_NUM) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_up_rx_bar::uc_tidno[%d] invalid.}", uc_tidno);
        return;
    }
    us_start_seqnum = mac_get_bar_start_seq_num(puc_payload);
    pst_ba_rx_info = pst_ta_user->ast_tid_info[uc_tidno].pst_ba_rx_info;

    hmac_reorder_ba_rx_buffer_bar(pst_ba_rx_info, us_start_seqnum, &(pst_hmac_vap->st_vap_base_info));
}

oal_bool_enum_uint8 hmac_is_device_ba_setup(void)
{
    uint8_t uc_vap_id;
    hmac_vap_stru *pst_hmac_vap = NULL;

    for (uc_vap_id = 0; uc_vap_id < WLAN_VAP_SUPPORT_MAX_NUM_LIMIT; uc_vap_id++) {
        pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(uc_vap_id);
        if (pst_hmac_vap == NULL) {
            oam_error_log0(0, OAM_SF_ANY, "{hmac_is_device_ba_setup pst_mac_vap is null.}");
            continue;
        }
        if ((pst_hmac_vap->st_vap_base_info.en_vap_state != MAC_VAP_STATE_UP) &&
            (pst_hmac_vap->st_vap_base_info.en_vap_state != MAC_VAP_STATE_PAUSE)) {
            continue;
        }
        if ((mac_mib_get_TxBASessionNumber(&pst_hmac_vap->st_vap_base_info) != 0) ||
            (mac_mib_get_RxBASessionNumber(&pst_hmac_vap->st_vap_base_info) != 0)) {
            return OAL_TRUE;
        }
    }
    return OAL_FALSE;
}

/*
 * 函 数 名  : hmac_ps_rx_delba
 * 功能描述  : hmac ps业务变化删除BA
 * 1.日    期  : 2019年12月19日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_ps_rx_delba(mac_vap_stru *pst_mac_vap, uint8_t uc_len, uint8_t *puc_param)
{
    hmac_vap_stru *pst_hmac_vap = NULL;
    hmac_user_stru *pst_hmac_user = NULL;
    mac_cfg_delba_req_param_stru st_mac_cfg_delba_param;
    dmac_to_hmac_ps_rx_delba_event_stru *pst_dmac_to_hmac_ps_rx_delba = NULL;
    uint32_t ret;

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (oal_unlikely(pst_hmac_vap == NULL)) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_BA,
                       "{hmac_ps_rx_delba::pst_hmac_vap is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_dmac_to_hmac_ps_rx_delba = (dmac_to_hmac_ps_rx_delba_event_stru *)puc_param;

    pst_hmac_user = mac_res_get_hmac_user(pst_dmac_to_hmac_ps_rx_delba->us_user_id);
    if (oal_unlikely(pst_hmac_user == NULL)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_BA,
                         "{hmac_ps_rx_delba::pst_hmac_user is null! user_id is %d.}",
                         pst_dmac_to_hmac_ps_rx_delba->us_user_id);
        return OAL_ERR_CODE_PTR_NULL;
    }
    /* 刷新HMAC VAP下PS业务时RX AMSDU使能标志 */
    pst_mac_vap->en_ps_rx_amsdu = pst_dmac_to_hmac_ps_rx_delba->en_rx_amsdu;
    pst_mac_vap->uc_ps_type |= pst_dmac_to_hmac_ps_rx_delba->uc_ps_type;
    /* 保证命令优先级最高 */
    if (pst_mac_vap->uc_ps_type & MAC_PS_TYPE_CMD) {
        return OAL_SUCC;
    }
    pst_hmac_vap->en_ps_rx_amsdu = pst_mac_vap->en_ps_rx_amsdu;
    memset_s((uint8_t *)&st_mac_cfg_delba_param, sizeof(st_mac_cfg_delba_param),
             0, sizeof(st_mac_cfg_delba_param));
    oal_set_mac_addr(st_mac_cfg_delba_param.auc_mac_addr, pst_hmac_user->st_user_base_info.auc_user_mac_addr);
    st_mac_cfg_delba_param.en_direction = MAC_RECIPIENT_DELBA;
    st_mac_cfg_delba_param.en_trigger = MAC_DELBA_TRIGGER_PS;

    for (st_mac_cfg_delba_param.uc_tidno = 0;
         st_mac_cfg_delba_param.uc_tidno < WLAN_TID_MAX_NUM; st_mac_cfg_delba_param.uc_tidno++) {
        ret = hmac_config_delba_req(pst_mac_vap, 0, (uint8_t *)&st_mac_cfg_delba_param);
        if (ret != OAL_SUCC) {
            oam_warning_log2(pst_mac_vap->uc_vap_id, OAM_SF_BA,
                "{hmac_ps_rx_delba::delba send failed, ret: %d, tid: %d}",
                ret, st_mac_cfg_delba_param.uc_tidno);
            return ret;
        }
    }
    return OAL_SUCC;
}

/*
 * 功能描述   : 更新ba_handler中的last release seqnum
 * 1.日    期   : 2019-08-22
 *   作    者   : wifi
 *   修改内容   : 新生成函数
 */
void hmac_ba_rx_update_release_seqnum(hmac_ba_rx_stru *ba_rx_hdl, uint16_t release_sn)
{
    if (oal_unlikely((ba_rx_hdl == NULL) || (release_sn > MAX_BA_SN))) {
        oam_error_log1(0, OAM_SF_RX, "{hmac_ba_rx_update_release_seqnum::release_sn[%d].}", release_sn);
        return;
    }

    if ((hmac_baw_rightside(ba_rx_hdl->us_last_relseq, release_sn)) || (ba_rx_hdl->us_last_relseq == release_sn)) {
        ba_rx_hdl->us_last_relseq = release_sn;
        return;
    } else {
        oam_warning_log0(0, OAM_SF_RX, "{hmac_ba_rx_update_release_seqnum::sw req update wrong release sn.}");
        return;
    }
}

/*
 * 功能描述   : 从重排序队列中获取skb链
 * 1.日    期   : 2014年5月21日
 *   作    者   : wifi
 *   修改内容   : 新生成函数
 */
OAL_STATIC uint32_t hmac_ba_rx_prepare_buffer(hmac_vap_stru *hmac_vap,
    hmac_rx_buf_stru *rx_entry, oal_netbuf_head_stru *pnetbuf_head)
{
    oal_netbuf_stru    *netbuf = NULL;
    mac_rx_ctl_stru    *rx_ctl = NULL;

    if (oal_any_null_ptr2(hmac_vap, pnetbuf_head)) {
        oam_error_log0(0, OAM_SF_BA, "{hmac_ba_rx_prepare_buffer::input params null.}");
        return OAL_FAIL;
    }

    if (rx_entry == NULL) {
        oam_warning_log0(0, OAM_SF_BA, "{hmac_ba_rx_prepare_buffer::rx_reo_entry null.}");
        return OAL_FAIL;
    }

    if (oal_netbuf_list_empty(&rx_entry->st_netbuf_head)) {
        oam_error_log0(0, OAM_SF_BA, "{hmac_ba_rx_prepare_buffer::netbuf_head is empty.}");
        return OAL_FAIL;
    }

    /* 按顺序串链缓存帧 */
    while (!oal_netbuf_list_empty(&(rx_entry->st_netbuf_head))) {
        netbuf = oal_netbuf_delist(&(rx_entry->st_netbuf_head));
        if (netbuf == NULL) {
            oam_error_log0(0, OAM_SF_RX, "{hmac_ba_rx_prepare_bufflist::netbuf null.}");
            return OAL_FAIL;
        }

        rx_ctl = (mac_rx_ctl_stru *)oal_netbuf_cb(netbuf);
        rx_ctl->bit_is_reo_timeout = OAL_TRUE;
        oal_netbuf_add_to_list_tail(netbuf, pnetbuf_head);
    }

    return OAL_SUCC;
}


uint32_t  hmac_ba_check_rx_aggrate(hmac_vap_stru *hmac_vap, mac_rx_ctl_stru *rx_ctl)
{
    uint8_t    frame_type;

    if (oal_unlikely(oal_any_null_ptr2(hmac_vap, rx_ctl))) {
        oam_error_log0(0, OAM_SF_RX, "{hmac_ba_check_rx_aggr::input params null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    frame_type = mac_get_frame_type_and_subtype((uint8_t *)&rx_ctl->us_frame_control);
    /* 判断该帧是不是qos帧 */
    if (frame_type != (WLAN_FC0_SUBTYPE_QOS | WLAN_FC0_TYPE_DATA)) {
        oam_info_log0(0, OAM_SF_BA, "{hmac_ba_check_rx_aggr::not qos data.}");
        return OAL_FAIL;
    }
#ifdef _PRE_WLAN_FEATURE_MCAST_AMPDU
    /* 设置组播标记位 */
    if (mac_get_mcast_ampdu_switch() == OAL_TRUE && rx_ctl->bit_mcast_bcast == OAL_TRUE) {
        rx_ctl->bit_mcast_bcast = OAL_FALSE;
        oam_warning_log2(0, OAM_SF_BA, "{hmac_ba_check_rx_aggr::This is a multicast data frame,seqnum[%d], proc[%d].}",
            rx_ctl->us_seq_num, rx_ctl->bit_process_flag);
    }
#endif
    /* 对于STA还需要判断是否是广播帧，考虑兼容某些设备广播帧带qos域 */
    if ((hmac_vap->st_vap_base_info.en_vap_mode == WLAN_VAP_MODE_BSS_STA) &&
        (rx_ctl->bit_mcast_bcast == OAL_TRUE)) {
        oam_info_log0(0, OAM_SF_BA, "{hmac_ba_check_rx_aggr::This is a multicast data frame.}");
        return OAL_FAIL;
    }

    return OAL_SUCC;
}

/*
 * 功能描述   : 增加ba control info的引用计数
 * 1.日    期   : 2018-04-18
 *   作    者   : wifi
 *   修改内容   : 新生成函数
 */
void hmac_ba_rx_hdl_hold(hmac_ba_rx_stru *ba_rx_hdl)
{
    oal_atomic_inc(&ba_rx_hdl->ref_cnt);
}

/*
 * 功能描述   : 递减ba control info的引用计数
 * 1.日    期   : 2018-04-18
 *   作    者   : wifi
 *   修改内容   : 新生成函数
 */
void hmac_ba_rx_hdl_put(hmac_ba_rx_stru *ba_rx_hdl)
{
    oal_atomic_dec(&ba_rx_hdl->ref_cnt);
}

OAL_STATIC uint32_t hmac_rx_frm_diff_time(hmac_rx_buf_stru *rx_buf, uint32_t curr_time)
{
    uint32_t time_diff;

    if (curr_time >= rx_buf->rx_time) {
        time_diff = curr_time - rx_buf->rx_time;
    } else {
        time_diff = (HMAC_U32_MAX - rx_buf->rx_time) + curr_time;
    }

    return time_diff;
}

OAL_STATIC uint16_t hmac_rx_ba_get_baw_head(hmac_ba_rx_stru *rx_ba, uint16_t baw_head)
{
    return (hmac_baw_rightside(baw_head, hmac_ba_seqno_add(rx_ba->us_last_relseq, 1))) ? baw_head :
        hmac_ba_seqno_add(rx_ba->us_last_relseq, 1);
}

/*
 * 功能描述   : 上报重排序队列中超时的报文
 * 1.日    期   : 2014年5月19日
 *   作    者   : wifi
 *   修改内容   : 新生成函数
 */
uint32_t hmac_ba_send_ring_reorder_timeout(hmac_ba_rx_stru *rx_ba,
    hmac_vap_stru *hmac_vap, hmac_ba_alarm_stru *alarm_data, uint32_t *p_timeout)
{
    uint32_t             curr_time, time_diff, rx_timeout;
    uint16_t             baw_head, baw_start, baw_end;
    uint32_t             ret;
    oal_netbuf_head_stru netbuf_head;
    hmac_rx_buf_stru    *rx_buf = NULL;

    oal_netbuf_list_head_init(&netbuf_head);
    baw_head = rx_ba->us_baw_start;
    baw_start = rx_ba->us_baw_start;
    rx_timeout = g_ba_rx_timeout_len[WLAN_WME_TID_TO_AC(alarm_data->uc_tid)];
    baw_end = hmac_ba_seqno_add(rx_ba->us_baw_tail, 1);

    if (baw_head != hmac_ba_seqno_add(rx_ba->us_last_relseq, 1)) {
        /* baw_start eq. us_last_relseq + 1 从较靠左侧的开始判断超时 */
        oam_info_log3(0, 0, "{hmac_ba_send_ring_reorder_timeout:baw_start[%d]!=last_relseq[%d]+1, baw_end = [%d]}",
            baw_head, rx_ba->us_last_relseq, rx_ba->us_baw_tail);
        baw_head = hmac_rx_ba_get_baw_head(rx_ba, baw_head);
    }

    curr_time = (uint32_t)oal_time_get_stamp_ms();
    while (baw_head != baw_end) {
        rx_buf = hmac_get_frame_from_reorder_q(rx_ba, baw_head);
        if (rx_buf == NULL) {
            baw_head = hmac_ba_seqno_add(baw_head, 1);
            continue;
        }

        /* 获取当前时间和收到报文时的差值，差值如果大于定时器timer，则release报文 */
        time_diff = hmac_rx_frm_diff_time(rx_buf, curr_time);
        if (time_diff < rx_timeout) {
            *p_timeout = rx_timeout - time_diff;
            break;
        }
        rx_buf = hmac_remove_frame_from_ring_reorder_q(rx_ba, baw_head);
        ret = hmac_ba_rx_prepare_buffer(hmac_vap, rx_buf, &netbuf_head);

        /* timeout上报中无需check软件是否需要主动上报缓存帧，因为对于rx_ba->us_baw_start之前的缓存帧一定会在此 */
        /* baw_start handler的地方检查了，此处只需更新last release sequence number */
        hmac_ba_rx_update_release_seqnum(rx_ba, baw_head);
        if (ret != OAL_SUCC) {
            baw_head = hmac_ba_seqno_add(baw_head, 1);
            continue;
        }
        baw_head = hmac_ba_seqno_add(baw_head, 1);
    }
    rx_ba->us_baw_start = baw_head;
    if (baw_start != rx_ba->us_baw_start) {
        rx_ba->en_timer_triggered = OAL_TRUE;
    }
    /* To kernel or to wlan */
    if (hmac_vap->st_vap_base_info.en_vap_mode == WLAN_VAP_MODE_BSS_STA) {
        hmac_rx_data_sta_proc(hmac_vap, &netbuf_head);
    }
    if (hmac_vap->st_vap_base_info.en_vap_mode == WLAN_VAP_MODE_BSS_AP) {
        hmac_rx_data_ap_proc(hmac_vap, &netbuf_head);
    }
    return OAL_SUCC;
}

/*
 * 函 数 名   : hmac_ba_buffer_frame_in_reorder
 * 1.日    期   : 2013年4月11日
 *   作    者   : wifi
 *   修改内容   : 新生成函数
 */
OAL_STATIC hmac_rx_buf_stru *hmac_ba_buffer_frame_in_reodr(hmac_ba_rx_stru *ba_rx_hdl,
    uint16_t sn, oal_netbuf_head_stru *pnetbuf_head)
{
    uint16_t        buf_index;
    hmac_rx_buf_stru *rx_buf = NULL;

    buf_index = (sn & (WLAN_AMPDU_RX_HE_BUFFER_SIZE - 1));
    rx_buf = &(ba_rx_hdl->ast_re_order_list[buf_index]);
    if (rx_buf == NULL) {
        return NULL;
    }

    if (rx_buf->in_use) {
        /* 如果缓存位置已经有包，则上报已存包并将新包存入 */
        oal_netbuf_queue_splice_tail_init(&(rx_buf->st_netbuf_head), pnetbuf_head);
        oam_info_log1(0, OAM_SF_BA, "{hmac_ba_buffer_frame_in_reorder::slot already used, seq[%d].}", sn);
    } else {
        if (oal_unlikely(!oal_netbuf_list_empty(&rx_buf->st_netbuf_head))) {
            oam_error_log1(0, OAM_SF_BA, "{hmac_ba_buffer_frame_in_reorder::[MEMORY-LEAK], seq[%d].}", sn);
        }
        ba_rx_hdl->uc_mpdu_cnt++;
    }

    rx_buf->in_use = OAL_TRUE;

    return rx_buf;
}

/*
 * 函 数 名   : hmac_rx_add_netbuf_2roe_list
 * 功能描述   : 把mpdu添加到接收缓存队列
 * 1.日    期   : 2018-04-18
 *   作    者   : wifi
 *   修改内容   : 新生成函数
 */
OAL_STATIC void hmac_ba_add_netbuf_to_reo_list(hmac_ba_rx_stru *ba_rx_hdl,
    oal_netbuf_stru *netbuf_mpdu, oal_netbuf_head_stru *netbuf_head)
{
    mac_rx_ctl_stru  *rx_ctl = NULL;
    hmac_rx_buf_stru *rx_reo_entry = NULL;

    rx_ctl = (mac_rx_ctl_stru *)oal_netbuf_cb(netbuf_mpdu);
    rx_reo_entry = hmac_ba_buffer_frame_in_reodr(ba_rx_hdl, mac_get_rx_cb_seq_num(rx_ctl), netbuf_head);
    if (rx_reo_entry == NULL) {
        return;
    }

    /*
    * 在报文缓存到接收缓存队列的时候，队列中每个元素包含的是skb结构；
    * 如果是amsdu，则会把所有msdu以skb的形式串链
    **/
    rx_reo_entry->us_seq_num = mac_get_rx_cb_seq_num(rx_ctl);
    rx_reo_entry->rx_time = (uint32_t)oal_time_get_stamp_ms();
    oal_netbuf_list_head_init(&(rx_reo_entry->st_netbuf_head));
    hmac_rx_mpdu_to_netbuf_list(&(rx_reo_entry->st_netbuf_head), netbuf_mpdu);
    rx_reo_entry->uc_num_buf = oal_netbuf_list_len(&(rx_reo_entry->st_netbuf_head));
    if (OAL_TRUE == hmac_ba_seqno_lt(ba_rx_hdl->us_baw_tail, mac_get_rx_cb_seq_num(rx_ctl))) {
        ba_rx_hdl->us_baw_tail = mac_get_rx_cb_seq_num(rx_ctl);
    }
    return;
}

/*
 * 函 数 名   : hmac_rx_reo_timer_restart
 * 功能描述   : 重启重排序定时器
 * 1.日    期   : 2018-04-18
 *   作    者   : wifi
 *   修改内容   : 新生成函数
 */
uint32_t hmac_ba_rx_reo_timer_restart(hmac_user_stru *hmac_user,
    hmac_ba_rx_stru *ba_rx_hdl, oal_netbuf_stru *netbuf_mpdu)
{
    uint8_t          rx_tid;
    hmac_vap_stru   *hmac_vap = NULL;
    mac_rx_ctl_stru *rx_ctl = NULL;
    mac_device_stru *mac_device = NULL;

    if (oal_any_null_ptr3(hmac_user, ba_rx_hdl, netbuf_mpdu)) {
        oam_error_log0(0, OAM_SF_RX, "{hmac_ba_rx_reo_timer_restart::invalid param.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    rx_ctl = (mac_rx_ctl_stru *)oal_netbuf_cb(netbuf_mpdu);
    rx_tid = mac_get_rx_cb_tid(rx_ctl);

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(rx_ctl->uc_mac_vap_id);
    if (oal_unlikely(oal_any_null_ptr1(hmac_vap))) {
        oam_error_log1(0, OAM_SF_RX, "{hmac_ba_rx_reo_timer_restart::hmac_vap NULL.}", rx_ctl->uc_mac_vap_id);
        return OAL_FAIL;
    }

    mac_device = mac_res_get_dev(hmac_vap->st_vap_base_info.uc_device_id);
    if (mac_device == NULL) {
        oam_error_log1(0, OAM_SF_BA, "{hmac_reorder_ba_timer_start::mac dev[%d] null.}",
            hmac_vap->st_vap_base_info.uc_device_id);
        return OAL_FAIL;
    }

    oal_spin_lock(&(hmac_user->ast_tid_info[rx_tid].st_ba_timer_lock));
    frw_timer_create_timer_m(&(hmac_user->ast_tid_info[rx_tid].st_ba_timer), hmac_ba_timeout_fn,
        g_ba_rx_timeout_len[WLAN_WME_TID_TO_AC(rx_tid)], &(ba_rx_hdl->st_alarm_data), OAL_FALSE,
        OAM_MODULE_ID_HMAC, mac_device->core_id);
    oal_spin_unlock(&(hmac_user->ast_tid_info[rx_tid].st_ba_timer_lock));

    return OAL_SUCC;
}
void hmac_ba_rx_buffered_data_add_list(hmac_rx_buf_stru *rx_reo_entry, oal_netbuf_stru *netbuf,
    oal_netbuf_head_stru *netbuf_head)
{
    netbuf = oal_netbuf_delist(&(rx_reo_entry->st_netbuf_head));
    if (netbuf != NULL) {
        oal_netbuf_add_to_list_tail(netbuf, netbuf_head);
    }
}

/*
 * 功能描述   : 重排序释放到(sn-1)的序列号
 * 1.日    期   : 2020.7.20
 *   作    者   : wifi
 *   修改内容   : 新生成函数
*  2.日    期   : 2021.4.10
 *   作    者   : wifi
 *   修改内容   : 增加reorder超时逻辑
 */
void hmac_rx_reorder_release_to_sn(hmac_ba_rx_stru *ba_rx_hdl, mac_rx_ctl_stru *rx_ctl,
    uint16_t sn, oal_netbuf_head_stru *rpt_list)
{
    uint16_t          start_sn;
    hmac_rx_buf_stru *rx_reo_entry = NULL;

    /* 从us_last_relseq + 1 一直上报到check_sequence_number -1，因为check_sequence_number走自己的逻辑上报 */
    start_sn = mac_ba_seqno_add(ba_rx_hdl->us_last_relseq, 1);
    if (sn == (mac_ba_seqno_add(ba_rx_hdl->us_last_relseq, 1))) {
        /* check的是last_relseq的下一个包,不需要主动上报任何包 */
        return;
    }
    oam_warning_log3(0, OAM_SF_RX,
        "{hmac_rx_reorder_release_to_sn::release frame.last_relseq[%d] process_flag[%d] release_is_valid[%d]}",
        ba_rx_hdl->us_last_relseq, rx_ctl->bit_process_flag, rx_ctl->bit_release_valid);
    oam_warning_log4(0, OAM_SF_RX,
        "{hmac_rx_reorder_release_to_sn::release frame.sn[%d] release_start_sn[%d] release_end_sn[%d], seq_num[%d]}",
        sn, rx_ctl->bit_release_start_sn, rx_ctl->bit_release_end_sn, rx_ctl->us_seq_num);
    do {
        rx_reo_entry = hmac_remove_frame_from_ring_reorder_q(ba_rx_hdl, start_sn);
        if (rx_reo_entry != NULL) {
            oal_netbuf_queue_splice_tail_init(&rx_reo_entry->st_netbuf_head, rpt_list);
            rx_reo_entry->uc_num_buf = 0;
        }
        start_sn = mac_ba_seqno_add(start_sn, 1);
    } while (start_sn != sn);

    hmac_ba_rx_update_release_seqnum(ba_rx_hdl, (uint16_t)mac_ba_seqno_sub(start_sn, 1));
}

/*
 * 功能描述   : 软件检查ba handler记录的last release sn与输入的sn的关系，上报手中应该上报的帧，更新last release sn
 * 1.日    期   : 2019-08-22
 *   作    者   : wifi
 *   修改内容   : 新生成函数
 */
uint32_t hmac_rx_ba_release_check_sn(hmac_ba_rx_stru *ba_rx_hdl,
    uint16_t sn, mac_rx_ctl_stru *rx_ctl, oal_netbuf_head_stru *rpt_list)
{
    if ((hmac_baw_rightside(ba_rx_hdl->us_last_relseq, sn))) {
        hmac_rx_reorder_release_to_sn(ba_rx_hdl, rx_ctl, sn, rpt_list);
    } else if (ba_rx_hdl->us_last_relseq == sn) {
        return OAL_SUCC;
    } else {
        oam_info_log2(0, OAM_SF_RX, "{hmac_ba_rx_buffered_data_check::input err check sn lrelease[%d] check[%d]}",
            ba_rx_hdl->us_last_relseq, sn);
        oam_info_log2(0, OAM_SF_RX, "{hmac_ba_rx_buffered_data_check::RELEASE_VALID[%d] PROCESS_FLAG[%d]}",
            rx_ctl->bit_release_valid, rx_ctl->bit_process_flag);
        oam_info_log4(0, OAM_SF_RX, "{hmac_ba_rx_buffered_data_check::SSN[%d] SN[%d] REL_START[%d] REL_END[%d].}",
            rx_ctl->bit_start_seqnum, mac_get_rx_cb_seq_num(rx_ctl),
            rx_ctl->bit_release_start_sn, rx_ctl->bit_release_end_sn);
        return OAL_FAIL;
    }

    return OAL_SUCC;
}

OAL_STATIC uint32_t hmac_rx_ba_release_check_sn_failed(hmac_ba_rx_stru *ba_rx_hdl, mac_rx_ctl_stru *rx_ctl,
    uint16_t *release_start_sn, uint16_t release_end_sn)
{
    if (hmac_baw_rightside(ba_rx_hdl->us_last_relseq, release_end_sn)) {
        /* us_release_start_sn小于last_release而us_release_end_sn大于last_release则从last_release+1开始上报 */
        *release_start_sn = mac_ba_seqno_add(ba_rx_hdl->us_last_relseq, 1);
    } else if (ba_rx_hdl->us_last_relseq == release_end_sn) {
        return OAL_FAIL;
    } else {
        oam_warning_log3(0, OAM_SF_RX, "{hmac_rx_release_valid_check:: release_end_sn is less than \
                         last_relseq + 1, release_end_sn [%d] last_relseq [%d], start from [%d].}",
                         release_end_sn, ba_rx_hdl->us_last_relseq, rx_ctl->us_seq_num);
        return OAL_FAIL;
    }
    return OAL_SUCC;
}

/*
 * 功能描述   : release参数有效性检查
 * 1.日    期   : 2020.7.20
 *   作    者   : wifi
 *   修改内容   : 新生成函数
 */
OAL_INLINE uint32_t hmac_rx_release_valid_check(hmac_ba_rx_stru *ba_rx_hdl, mac_rx_ctl_stru  *rx_ctl,
    oal_netbuf_head_stru *rpt_list, uint16_t *release_start_sn, uint16_t release_end_sn)
{
    uint32_t ret;

    if (oal_unlikely(*release_start_sn > MAX_BA_SN || release_end_sn > MAX_BA_SN)) {
        oam_error_log2(0, OAM_SF_RX, "{hmac_rx_release_valid_check::invalid param release_start_sn[%d] end_sn[%d].}",
            *release_start_sn, release_end_sn);
        return OAL_FAIL;
    }

    if ((hmac_baw_rightside(*release_start_sn, release_end_sn)) || (*release_start_sn == release_end_sn)) {
        /* 如果release_start_sn向后跳了，软件主动上报release_start_sn之前的包 */
        ret = hmac_rx_ba_release_check_sn(ba_rx_hdl, *release_start_sn, rx_ctl, rpt_list);
        if (ret != OAL_SUCC) {
            return hmac_rx_ba_release_check_sn_failed(ba_rx_hdl, rx_ctl, release_start_sn, release_end_sn);
        }
    } else {
        oam_warning_log2(0, OAM_SF_RX,
            "{hmac_rx_release_valid_check:: report wrong realese sequence number, start [%d] end [%d].}",
            *release_start_sn, release_end_sn);
        /* buffered frame走到此处则不做操作 */
        return OAL_FAIL;
    }

    return OAL_SUCC;
}

/*
 * 功能描述   : release接收缓存队列中的报文
 * 1.日    期   : 2020.7.20
 *   作    者   : wifi
 *   修改内容   : 新生成函数
 */
void hmac_rx_ba_handle_release_flag(hmac_host_rx_context_stru *rx_context,
    hmac_ba_rx_stru *ba_rx_hdl, oal_netbuf_head_stru *rpt_list)
{
    uint8_t           tid;
    uint16_t          release_end_sn;
    uint16_t          release_start_sn;
    uint32_t          rel_valid_check;
    mac_rx_ctl_stru  *rx_ctl = rx_context->cb;
    hmac_rx_buf_stru *rx_reo_entry = NULL;
    hmac_user_stru  *hmac_user = rx_context->hmac_user;

    if (rx_ctl->bit_release_valid == 0) {
        return;
    }

    /* 如果缓存帧需要处理，则根据描述符中的信息把缓存帧放入pst_netbuf_head链表中 */
    tid = mac_get_rx_cb_tid(rx_ctl);
    release_start_sn = rx_ctl->bit_release_start_sn;
    release_end_sn = rx_ctl->bit_release_end_sn;
    rel_valid_check = hmac_rx_release_valid_check(ba_rx_hdl, rx_ctl, rpt_list, &release_start_sn, release_end_sn);
    if (rel_valid_check != OAL_SUCC) {
        /* 直接返回，不做后续的释放操作 */
        return;
    }

    /* 更新release sequence number */
    hmac_ba_rx_update_release_seqnum(ba_rx_hdl, (uint16_t)release_end_sn);
    release_end_sn = mac_ba_seqno_add(release_end_sn, 1);
    do {
        rx_reo_entry = hmac_remove_frame_from_ring_reorder_q(ba_rx_hdl, release_start_sn);
        if (rx_reo_entry != NULL) {
            oal_netbuf_queue_splice_tail_init(&(rx_reo_entry->st_netbuf_head), rpt_list);
            rx_reo_entry->uc_num_buf = 0;
        }

        release_start_sn = mac_ba_seqno_add(release_start_sn, 1);
    } while ((release_start_sn != release_end_sn) && (ba_rx_hdl->uc_mpdu_cnt != 0));

    /* 新增us_mpdu_cnt判断条件防止芯片上报异常的release sequence造成空循环 */
    /* 如果接收缓存队列里面还有报文，restart timer */
    if (ba_rx_hdl->uc_mpdu_cnt > 0) {
        if (hmac_user->ast_tid_info[tid].st_ba_timer.en_is_registerd == OAL_FALSE) {
            hmac_ba_rx_reo_timer_restart(hmac_user, ba_rx_hdl, rx_context->netbuf);
        }
    }
}

/*
 * 功能描述   : release到当前报文为止的所以缓存帧
 * 1.日    期   : 2020.7.20
 *   作    者   : wifi
 *   修改内容   : 新生成函数
 */
uint32_t hmac_ba_rx_data_release(hmac_host_rx_context_stru *rx_context,
    hmac_ba_rx_stru *ba_rx_hdl, oal_netbuf_head_stru *rpt_list, uint8_t *buff_is_valid)
{
    uint32_t         ret;
    oal_netbuf_stru *netbuf = rx_context->netbuf;
    mac_rx_ctl_stru *rx_ctl = rx_context->cb;

    *buff_is_valid = OAL_FALSE;

    /* 1. 从上次释放报文的seq到当前报文的seq - 1之间的报文全部上报 */
    ret = hmac_rx_ba_release_check_sn(ba_rx_hdl, mac_get_rx_cb_seq_num(rx_ctl), rx_ctl, rpt_list);
    if (ret != OAL_SUCC) {
        rx_ctl->is_before_last_release = OAL_TRUE;
        oam_info_log2(0, OAM_SF_RX,
            "{hmac_ba_rx_data_release:: release a frame sn = [%d] before last release = [%d]}",
            mac_get_rx_cb_seq_num(rx_ctl), ba_rx_hdl->us_last_relseq);
        return ret;
    }

    /* 2.当前buf上报 */
    hmac_rx_mpdu_to_netbuf_list(rpt_list, netbuf);
    /* 单个报文上报，更新release_seqnum */
    hmac_ba_rx_update_release_seqnum(ba_rx_hdl, (uint16_t)mac_get_rx_cb_seq_num(rx_ctl));

    ba_rx_hdl->us_baw_start = rx_ctl->bit_start_seqnum;
    /* 3.继续判断是否需要进行缓存上报 */
    hmac_rx_ba_handle_release_flag(rx_context, ba_rx_hdl, rpt_list);

    /* 4.如果SSN还在last_release_seqnum后面，则可以继续release */
    if (hmac_baw_rightside(ba_rx_hdl->us_last_relseq, rx_ctl->bit_start_seqnum)) {
        hmac_rx_ba_release_check_sn(ba_rx_hdl, rx_ctl->bit_start_seqnum, rx_ctl, rpt_list);
    }

    return OAL_SUCC;
}

/*
 * 功能描述   : HAL_RX_PROC_FLAGS_BUFFER 处理函数
 *              1)如果需要release接收缓存队列中的报文，则先release;
 *              2)缓存当前报文到接收缓存队列。
 * 1.日    期   : 2018-04-18
 *   作    者   : wifi
 *   修改内容   : 新生成函数
 */
uint32_t hmac_ba_rx_data_buffer(hmac_host_rx_context_stru *rx_context,
    hmac_ba_rx_stru *ba_rx_hdl, oal_netbuf_head_stru *rpt_list, uint8_t *buff_is_valid)
{
    oal_netbuf_stru *netbuf = rx_context->netbuf;
    mac_rx_ctl_stru *rx_ctl = rx_context->cb;
    hmac_user_stru  *hmac_user = rx_context->hmac_user;

    *buff_is_valid = OAL_FALSE;
    ba_rx_hdl->us_baw_start = rx_ctl->bit_start_seqnum;
    /* 1.BA window start移到us_baw_start后，硬件后续不会再收us_baw_start之前的包，缓存的us_baw_start之前的包都可以上报 */
    hmac_rx_ba_release_check_sn(ba_rx_hdl, rx_ctl->bit_start_seqnum, rx_ctl, rpt_list);

    /* 2.判断缓存数据是否需要进行上报 */
    hmac_rx_ba_handle_release_flag(rx_context, ba_rx_hdl, rpt_list);

    /* 3.当前数据进行缓存 */
    hmac_ba_add_netbuf_to_reo_list(ba_rx_hdl, netbuf, rpt_list);

    /* 4.启动超时定时器 */
    if (hmac_user->ast_tid_info[mac_get_rx_cb_tid(rx_ctl)].st_ba_timer.en_is_registerd == OAL_FALSE) {
        oam_info_log3(0, OAM_SF_RX, "{hmac_ba_rx_data_buffer::restart timer user id[%d], tid[%d], mpdu cnt[%d].}",
                      rx_ctl->bit_rx_user_id, rx_ctl->bit_rx_tid, ba_rx_hdl->uc_mpdu_cnt);
        hmac_ba_rx_reo_timer_restart(hmac_user, ba_rx_hdl, netbuf);
    }

    return OAL_SUCC;
}

OAL_STATIC uint32_t hmac_mgmt_sync_delba_event(
    mac_vap_stru *mac_vap, mac_user_stru *mac_user, uint8_t tid, uint8_t initiator)
{
    frw_event_mem_stru *event_mem = NULL; /* 申请事件返回的内存指针 */
    frw_event_stru *hmac_to_dmac_crx_sync = NULL;
    dmac_ctx_action_event_stru *wlan_crx_action = NULL;
    /* 抛事件到DMAC处理 */
    event_mem = frw_event_alloc_m(sizeof(dmac_ctx_action_event_stru));
    if (event_mem == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_BA, "{hmac_mgmt_rx_delba::event_mem null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    /* 获得事件指针 */
    hmac_to_dmac_crx_sync = frw_get_event_stru(event_mem);
    /* 填写事件头 */
    frw_event_hdr_init(&(hmac_to_dmac_crx_sync->st_event_hdr),
        FRW_EVENT_TYPE_WLAN_CTX, DMAC_WLAN_CTX_EVENT_SUB_TYPE_BA_SYNC, sizeof(dmac_ctx_action_event_stru),
        FRW_EVENT_PIPELINE_STAGE_1, mac_vap->uc_chip_id, mac_vap->uc_device_id, mac_vap->uc_vap_id);

    /* 填写事件payload */
    wlan_crx_action = (dmac_ctx_action_event_stru *)(hmac_to_dmac_crx_sync->auc_event_data);
    wlan_crx_action->en_action_category = MAC_ACTION_CATEGORY_BA;
    wlan_crx_action->uc_action = MAC_BA_ACTION_DELBA;
    wlan_crx_action->us_user_idx = mac_user->us_assoc_id;
    wlan_crx_action->uc_tidno = tid;
    wlan_crx_action->uc_initiator = initiator;
    /* 分发 */
    frw_event_dispatch_event(event_mem);
    /* 释放事件内存 */
    frw_event_free_m(event_mem);
    return OAL_SUCC;
}
/*
 * 1.日    期  : 2013年4月14日
 *   修改内容  : 新生成函数
 */
uint32_t hmac_mgmt_rx_delba(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, uint8_t *payload,
    uint32_t payload_len)
{
    mac_device_stru *mac_device = NULL;
    hmac_tid_stru *tid = NULL;
    uint8_t tid_id;
    uint8_t initiator;
    uint16_t reason;

    if (oal_unlikely(oal_any_null_ptr3(hmac_vap, hmac_user, payload)) || (payload_len < MAC_DELBA_FRAME_BODY_LEN)) {
        oam_error_log1(0, OAM_SF_BA, "{hmac_mgmt_rx_delba:params is null or payload_len[%d] too short.}", payload_len);
        return OAL_FAIL;
    }
    /* 获取device结构 */
    mac_device = mac_res_get_dev(hmac_vap->st_vap_base_info.uc_device_id);
    if (oal_unlikely(mac_device == NULL)) {
        oam_error_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_BA, "{hmac_mgmt_rx_delba::mac_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /************************************************/
    /*       DELBA Response Frame - Frame Body      */
    /* -------------------------------------------- */
    /* | Category | Action | Parameters | Reason  | */
    /* -------------------------------------------- */
    /* | 1        | 1      | 2          | 2       | */
    /* -------------------------------------------- */
    /*                                              */
    /************************************************/
    tid_id = (payload[BYTE_OFFSET_3] & 0xF0) >> 4; /* bit4 */
    initiator = (payload[BYTE_OFFSET_3] & 0x08) >> 3; /* bit3 */
    reason = (payload[BYTE_OFFSET_4] & 0xFF) | ((payload[BYTE_OFFSET_5] << 8) & 0xFF00); /* bit8 */

    /* tid_id保护，避免数组越界 */
    if (tid_id >= WLAN_TID_MAX_NUM) {
        oam_warning_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_BA,
                         "{hmac_mgmt_rx_delba::delba receive failed, tid_id %d overflow.}", tid_id);
        return OAL_ERR_CODE_ARRAY_OVERFLOW;
    }
    tid = &(hmac_user->ast_tid_info[tid_id]);
    oam_warning_log3(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_BA, "{hmac_mgmt_rx_delba::receive delba from \
        peer sta, tid_id[%d], initiator[%d], reason[%d].}", tid_id, initiator, reason);
    /* 对tid对应的tx BA会话状态加锁 */
    oal_spin_lock_bh(&(tid->st_ba_tx_info.st_ba_status_lock));
    /* 重置BA发送会话 */
    if (initiator == MAC_RECIPIENT_DELBA) {
        if (tid->st_ba_tx_info.en_ba_status == DMAC_BA_INIT) {
            oal_spin_unlock_bh(&(tid->st_ba_tx_info.st_ba_status_lock));
            return OAL_SUCC;
        }
        /* 还原设置AMPDU下AMSDU的支持情况 */
        hmac_user_set_amsdu_support(hmac_user, tid_id);
        hmac_tx_ba_session_decr(hmac_vap, hmac_user->ast_tid_info[tid_id].uc_tid_no);
    } else { /* 重置BA接收会话 */
        hmac_ba_reset_rx_handle(mac_device, hmac_user, tid_id, OAL_FALSE);
    }
    if (hmac_mgmt_sync_delba_event(&hmac_vap->st_vap_base_info, &hmac_user->st_user_base_info,
        tid_id, initiator) != OAL_SUCC) {
        oal_spin_unlock_bh(&(tid->st_ba_tx_info.st_ba_status_lock));
        return OAL_FAIL;
    }
    /* DELBA事件先处理再改状态,防止addba req先处理 */
    if (initiator == MAC_RECIPIENT_DELBA) {
        tid->st_ba_tx_info.en_ba_status = DMAC_BA_INIT;
    }
    /* 对tid对应的tx BA会话状态解锁 */
    oal_spin_unlock_bh(&(tid->st_ba_tx_info.st_ba_status_lock));
    return OAL_SUCC;
}
OAL_STATIC void hmac_mgmt_fill_addba_rsp_event_data(
    hmac_user_stru *hmac_user, frw_event_stru *event, hmac_ba_rx_rsp_stru *ba_rx_rsp)
{
    dmac_ctx_action_event_stru *event_data = (dmac_ctx_action_event_stru *)(event->auc_event_data);
    event_data->en_action_category = MAC_ACTION_CATEGORY_BA;
    event_data->uc_action = MAC_BA_ACTION_ADDBA_RSP;
    event_data->us_user_idx = hmac_user->st_user_base_info.us_assoc_id;
    event_data->uc_status = ba_rx_rsp->status_code;
    event_data->uc_tidno = ba_rx_rsp->tidno;
    event_data->uc_dialog_token = ba_rx_rsp->dialog_token;
    event_data->uc_ba_policy = ba_rx_rsp->ba_policy;
    event_data->us_ba_timeout = ba_rx_rsp->ba_timeout;
    event_data->en_amsdu_supp = ba_rx_rsp->amsdu_supp;
    event_data->us_baw_size = ((ba_rx_rsp->baw_size == 0) ||
        (ba_rx_rsp->baw_size > g_wlan_spec_cfg->max_tx_ampdu_num)) ?
        g_wlan_spec_cfg->max_tx_ampdu_num : ba_rx_rsp->baw_size;
}
OAL_STATIC void hmac_mgmt_get_addba_rsp_info(hmac_ba_rx_rsp_stru *ba_rx_rsp, uint8_t *payload)
{
    /******************************************************************/
    /*       ADDBA Response Frame - Frame Body                        */
    /* --------------------------------------------------------------- */
    /* | Category | Action | Dialog | Status  | Parameters | Timeout | */
    /* --------------------------------------------------------------- */
    /* | 1        | 1      | 1      | 2       | 2          | 2       | */
    /* --------------------------------------------------------------- */
    /*                                                                */
    /******************************************************************/
    ba_rx_rsp->tidno = (payload[BYTE_OFFSET_5] & 0x3C) >> 2; /* bit2 */
    ba_rx_rsp->dialog_token = payload[BYTE_OFFSET_2];
    ba_rx_rsp->status_code = payload[BYTE_OFFSET_3];
    ba_rx_rsp->ba_policy = ((payload[BYTE_OFFSET_5] & 0x02) >> 1); /* bit1 */
    ba_rx_rsp->amsdu_supp = payload[BYTE_OFFSET_5] & BIT0;
    ba_rx_rsp->ba_timeout = payload[BYTE_OFFSET_7] | (payload[BYTE_OFFSET_8] << NUM_8_BITS);
    ba_rx_rsp->baw_size =
        (uint16_t)(((payload[BYTE_OFFSET_5] & 0xC0) >> NUM_6_BITS) | (payload[BYTE_OFFSET_6] << NUM_2_BITS));
}
OAL_STATIC void hmac_mgmt_rx_addba_rsp_result_handle(
    hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, hmac_ba_rx_rsp_stru *ba_rx_rsp)
{
    mac_vap_stru *mac_vap = &(hmac_vap->st_vap_base_info); // 有效性在调用处保证
    hmac_tid_stru *tid_info = NULL;
    uint8_t mib_amsdu_active;

    tid_info = &(hmac_user->ast_tid_info[ba_rx_rsp->tidno]);
    /* 先抛事件，再处理host BA句柄，防止异步发送ADDBA REQ */
    if (ba_rx_rsp->status_code == MAC_SUCCESSFUL_STATUSCODE) {
        /* 设置hmac模块对应的BA句柄的信息 */
        tid_info->st_ba_tx_info.en_ba_status = DMAC_BA_COMPLETE;
        tid_info->st_ba_tx_info.uc_addba_attemps = 0;
        mib_amsdu_active = mac_mib_get_AmsduPlusAmpduActive(mac_vap);
#ifdef _PRE_WLAN_FEATURE_PWL
        /* pwl模式不支持amsdu */
        if (mac_vap->pwl_enable == OAL_TRUE) {
            mib_amsdu_active = OAL_FALSE;
        }
#endif

        if (ba_rx_rsp->amsdu_supp && mib_amsdu_active) {
            hmac_user_set_amsdu_support(hmac_user, ba_rx_rsp->tidno);
        } else {
            hmac_user_set_amsdu_not_support(hmac_user, ba_rx_rsp->tidno);
        }
    } else { /* BA被拒绝 */
        oam_warning_log2(mac_vap->uc_vap_id, OAM_SF_BA,
            "{hmac_mgmt_rx_addba_rsp_result_handle::addba rsp  status err[%d].tid[%d],DEL BA.}",
            ba_rx_rsp->status_code, ba_rx_rsp->tidno);

        hmac_tx_ba_session_decr(hmac_vap, ba_rx_rsp->tidno);

        /* 先抛事件删除dmac旧BA句柄后，再重置HMAC模块信息，确保删除dmac ba事件在下一次ADDBA REQ事件之前到达dmac */
        tid_info->st_ba_tx_info.en_ba_status = DMAC_BA_INIT;
    }
}
OAL_STATIC uint32_t hmac_mgmt_rx_addba_rsp_status_check(hmac_vap_stru *hmac_vap,
    hmac_user_stru *hmac_user, hmac_ba_rx_rsp_stru *ba_rx_rsp, oal_bool_enum_uint8 *is_need_delba)
{
    mac_vap_stru *mac_vap = &(hmac_vap->st_vap_base_info); // 有效性在调用处保证
    hmac_tid_stru *tid_info = NULL;

    tid_info = &(hmac_user->ast_tid_info[ba_rx_rsp->tidno]);
    /*
     * BA状态成功，但token、policy不匹配，需要删除聚合
     *  使用超时机制去删除聚合。降低主动删除聚合后，立马收到匹配的ADDBA RSP，
     * 造成上述两个操作向DMAC抛事件，在DMAC中又不能保序执行带来的异常报错
     */
    if ((tid_info->st_ba_tx_info.en_ba_status == DMAC_BA_INPROGRESS) &&
        (ba_rx_rsp->status_code == MAC_SUCCESSFUL_STATUSCODE)) {
        if ((ba_rx_rsp->dialog_token != tid_info->st_ba_tx_info.uc_dialog_token) ||
            (ba_rx_rsp->ba_policy != tid_info->st_ba_tx_info.uc_ba_policy)) {
            oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_BA,
                "{hmac_mgmt_rx_addba_rsp::addba rsp tid[%d],status SUCC,but token/policy wr}", ba_rx_rsp->tidno);
            oam_warning_log4(mac_vap->uc_vap_id, OAM_SF_BA,
                "{hmac_mgmt_rx_addba_rsp::rsp policy[%d],req policy[%d], rsp dialog[%d], req dialog[%d]}",
                ba_rx_rsp->ba_policy, tid_info->st_ba_tx_info.uc_ba_policy,
                ba_rx_rsp->dialog_token, tid_info->st_ba_tx_info.uc_dialog_token);
            return OAL_SUCC_GO_ON;
        }
    }

    /* 停止计时器 */
    if (tid_info->st_ba_tx_info.st_addba_timer.en_is_registerd == OAL_TRUE) {
        frw_timer_immediate_destroy_timer_m(&tid_info->st_ba_tx_info.st_addba_timer);
    }

    if (tid_info->st_ba_tx_info.en_ba_status == DMAC_BA_INIT) {
        *is_need_delba = TRUE;
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_BA,
            "{hmac_mgmt_rx_addba_rsp::addba rsp is received when ba status is DMAC_BA_INIT.tid[%d]}", ba_rx_rsp->tidno);
        return OAL_SUCC_GO_ON;
    }

    if (tid_info->st_ba_tx_info.en_ba_status == DMAC_BA_COMPLETE) {
        oam_warning_log2(mac_vap->uc_vap_id, OAM_SF_BA,
            "{hmac_mgmt_rx_addba_rsp::addba rsp is received when ba status is DMAC_BA_COMPLETE. tid[%d], status[%d]}",
            ba_rx_rsp->tidno, tid_info->st_ba_tx_info.en_ba_status);
        return OAL_SUCC_GO_ON;
    }
    return OAL_SUCC;
}
OAL_STATIC uint32_t hmac_mgmt_sync_addba_rsp(
    mac_vap_stru *mac_vap, hmac_user_stru *hmac_user, hmac_ba_rx_rsp_stru *ba_rx_rsp)
{
    frw_event_mem_stru *event_mem = NULL; /* 申请事件返回的内存指针 */
    frw_event_stru *event = NULL;
    /* 抛事件到DMAC处理 */
    event_mem = frw_event_alloc_m(sizeof(dmac_ctx_action_event_stru));
    if (event_mem == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_BA, "{hmac_mgmt_rx_addba_rsp::event_mem null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    /* 获得事件指针 */
    event = frw_get_event_stru(event_mem);
    /* 填写事件头 */
    frw_event_hdr_init(&(event->st_event_hdr), FRW_EVENT_TYPE_WLAN_CTX, DMAC_WLAN_CTX_EVENT_SUB_TYPE_BA_SYNC,
        sizeof(dmac_ctx_action_event_stru), FRW_EVENT_PIPELINE_STAGE_1,
        mac_vap->uc_chip_id, mac_vap->uc_device_id, mac_vap->uc_vap_id);
    /* 获取帧体信息，由于DMAC的同步，填写事件payload */
    hmac_mgmt_fill_addba_rsp_event_data(hmac_user, event, ba_rx_rsp);
    /* 分发 */
    frw_event_dispatch_event(event_mem);
    /* 释放事件内存 */
    frw_event_free_m(event_mem);
    return OAL_SUCC;
}
uint32_t hmac_mgmt_rx_addba_rsp(
    hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, uint8_t *payload, uint32_t frame_body_len)
{
    mac_vap_stru *mac_vap = NULL;
    hmac_ba_rx_rsp_stru ba_rx_rsp;
    oal_bool_enum_uint8 is_need_delba = FALSE;

    if (oal_any_null_ptr3(hmac_vap, hmac_user, payload)) {
        oam_error_log0(0, OAM_SF_BA, "{hmac_mgmt_rx_addba_rsp::hmac_vap or hmac_user or payload is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    } else if (frame_body_len < MAC_ADDBA_RSP_FRAME_BODY_LEN) {
        oam_warning_log1(0, OAM_SF_BA, "{frame_body_len[%d] < MAC_ADDBA_REQ_FRAME_BODY_LEN.}", frame_body_len);
        return OAL_FAIL;
    }
    mac_vap = &(hmac_vap->st_vap_base_info);
    memset_s((uint8_t *)&ba_rx_rsp, sizeof(hmac_ba_rx_rsp_stru), 0, sizeof(hmac_ba_rx_rsp_stru));
    hmac_mgmt_get_addba_rsp_info(&ba_rx_rsp, payload);
    /* 协议支持tid为0~15, 02只支持tid0~7 */
    if (ba_rx_rsp.tidno >= WLAN_TID_MAX_NUM) {
        /* 对于tid > max 的resp直接忽略 */
        oam_warning_log3(mac_vap->uc_vap_id, OAM_SF_BA, "{hmac_mgmt_rx_addba_rsp::rsp tid[%d]} token[%d] state[%d]",
                         ba_rx_rsp.tidno, ba_rx_rsp.dialog_token, ba_rx_rsp.status_code);
        return OAL_SUCC;
    }
    /* 对tid对应的tx BA会话状态加锁 */
    oal_spin_lock_bh(&(hmac_user->ast_tid_info[ba_rx_rsp.tidno].st_ba_tx_info.st_ba_status_lock));
    if (hmac_mgmt_rx_addba_rsp_status_check(hmac_vap, hmac_user, &ba_rx_rsp, &is_need_delba) == OAL_SUCC_GO_ON) {
         /* 对tid对应的tx BA会话状态解锁 */
        oal_spin_unlock_bh(&(hmac_user->ast_tid_info[ba_rx_rsp.tidno].st_ba_tx_info.st_ba_status_lock));
        if (is_need_delba == TRUE) {
            hmac_tx_ba_del(hmac_vap, hmac_user, ba_rx_rsp.tidno);
        }
        return OAL_SUCC;
    }
    if (hmac_mgmt_sync_addba_rsp(mac_vap, hmac_user, &ba_rx_rsp) != OAL_SUCC) {
        oal_spin_unlock_bh(&(hmac_user->ast_tid_info[ba_rx_rsp.tidno].st_ba_tx_info.st_ba_status_lock));
        return OAL_FAIL;
    }
    hmac_mgmt_rx_addba_rsp_result_handle(hmac_vap, hmac_user, &ba_rx_rsp);
    /* 对tid对应的tx BA会话状态解锁 */
    oal_spin_unlock_bh(&(hmac_user->ast_tid_info[ba_rx_rsp.tidno].st_ba_tx_info.st_ba_status_lock));
    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_del_ba_event
 * 1.日    期  : 2013年11月21日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_del_ba_event(frw_event_mem_stru *pst_event_mem)
{
    uint8_t uc_tid;
    frw_event_stru *pst_event = NULL;
    hmac_user_stru *pst_hmac_user = NULL;
    hmac_vap_stru *pst_hmac_vap = NULL;

    mac_action_mgmt_args_stru st_action_args; /* 用于填写ACTION帧的参数 */
    hmac_tid_stru *pst_hmac_tid = NULL;
    uint32_t ret;
    dmac_to_hmac_ctx_event_stru *pst_del_ba_event;

    pst_event = frw_get_event_stru(pst_event_mem);

    pst_del_ba_event = (dmac_to_hmac_ctx_event_stru *)pst_event->auc_event_data;

    pst_hmac_user = mac_res_get_hmac_user(pst_del_ba_event->us_user_index);
    if (pst_hmac_user == NULL) {
        /* dmac抛事件到hmac侧删除ba，此时host侧可能已经删除用户了，此时属于正常，直接返回即可 */
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_del_ba_event::pst_hmac_user[%d] null.}",
                         pst_del_ba_event->us_user_index);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hmac_vap = mac_res_get_hmac_vap(pst_del_ba_event->uc_vap_id);
    if (pst_hmac_vap == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_del_ba_event::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    mac_user_set_cur_protocol_mode(&pst_hmac_user->st_user_base_info, pst_del_ba_event->uc_cur_protocol);
    ret = hmac_config_user_info_syn(&(pst_hmac_vap->st_vap_base_info), &(pst_hmac_user->st_user_base_info));
    if (ret != OAL_SUCC) {
        return ret;
    }

    for (uc_tid = 0; uc_tid < WLAN_TID_MAX_NUM; uc_tid++) {
        pst_hmac_tid = &pst_hmac_user->ast_tid_info[uc_tid];

        if (pst_hmac_tid->st_ba_tx_info.en_ba_status == DMAC_BA_INIT) {
            oam_info_log0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ANY,
                          "{hmac_del_ba_event::the tx hdl is not exist.}");
            continue;
        }

        st_action_args.uc_category = MAC_ACTION_CATEGORY_BA;
        st_action_args.uc_action = MAC_BA_ACTION_DELBA;
        st_action_args.arg1 = uc_tid; /* 该数据帧对应的TID号 */
        /* ADDBA_REQ中，buffer_size的默认大小 */
        st_action_args.arg2 = MAC_ORIGINATOR_DELBA;
        st_action_args.arg3 = MAC_UNSPEC_REASON;                                   /* BA会话的确认策略 */
        st_action_args.puc_arg5 = pst_hmac_user->st_user_base_info.auc_user_mac_addr; /* ba会话对应的user */

        /* 删除BA会话 */
        ret = hmac_mgmt_tx_action(pst_hmac_vap, pst_hmac_user, &st_action_args);
        if (ret != OAL_SUCC) {
            oam_warning_log0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ANY,
                             "{hmac_del_ba_event::hmac_mgmt_tx_action failed.}");
            continue;
        }
    }

    return OAL_SUCC;
}
