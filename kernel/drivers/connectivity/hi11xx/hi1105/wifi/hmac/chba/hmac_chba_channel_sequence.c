﻿/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : CHBA 信道序列文件
 * 作    者 :
 * 创建日期 : 2022年3月24日
 */

#ifdef _PRE_WLAN_CHBA_MGMT
#include "oal_ext_if.h"
#include "oam_ext_if.h"

#include "hmac_chba_channel_sequence.h"
#include "hmac_chba_function.h"
#include "hmac_chba_frame.h"
#include "hmac_chba_sync.h"
#include "hmac_chba_chan_switch.h"
#include "hmac_config.h"
#include "hmac_chba_coex.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_CHBA_CHANNEL_SEQUENCE_C

static uint32_t chba_channel_seq_level_to_bitmap(uint32_t level)
{
    uint32_t chba_channel_seq_level_to_bitmap_table[CHBA_CHANNEL_SEQ_LEVEL_BUTT] = {
        CHBA_CHANNEL_SEQ_BITMAP_25_PERCENT,
        CHBA_CHANNEL_SEQ_BITMAP_50_PERCENT,
        CHBA_CHANNEL_SEQ_BITMAP_75_PERCENT
    };

    if (level >= CHBA_CHANNEL_SEQ_LEVEL_BUTT) {
        return CHBA_CHANNEL_SEQ_BITMAP_100_PERCENT;
    }
    return chba_channel_seq_level_to_bitmap_table[level];
}

/* 配置 信道序列 bitmap */
void hmac_chba_set_channel_seq_bitmap(hmac_chba_vap_stru *chba_vap_info, uint32_t channel_seq_bitmap)
{
    chba_vap_info->channel_sequence_bitmap = channel_seq_bitmap;
}

/* 配置CHBA第一工作信道 */
static void hmac_chba_set_first_work_channel(hmac_chba_vap_stru *chba_vap_info, mac_channel_stru *first_work_channel)
{
    chba_vap_info->work_channel = *first_work_channel;
}

/* 配置CHBA第二工作信道 */
static void hmac_chba_set_sec_work_channel(hmac_chba_vap_stru *chba_vap_info, mac_channel_stru *sec_work_channel)
{
    chba_vap_info->second_work_channel = *sec_work_channel;
}


/* 判断是否是单CHBA VAP工作 */
static oal_bool_enum_uint8 mac_device_is_single_chba_up(mac_device_stru *mac_device)
{
    uint8_t vap_idx;

    for (vap_idx = 0; vap_idx < mac_device->uc_vap_num; vap_idx++) {
        mac_vap_stru *mac_vap = NULL;
        mac_vap = mac_res_get_mac_vap(mac_device->auc_vap_id[vap_idx]);
        if (oal_unlikely(mac_vap == NULL)) {
            oam_error_log1(0, OAM_SF_ANY, "vap is null! vap id is %d",
                mac_device->auc_vap_id[vap_idx]);
            continue;
        }
        if (!mac_is_chba_mode(mac_vap) && mac_vap_is_up(mac_vap)) {
            return OAL_FALSE;
        }
    }
    return OAL_TRUE;
}


/*
 * 函数名:hmac_chba_set_channel_seq_params
 * 功能:设置CHBA 信道序列参数
 */
uint32_t hmac_chba_set_channel_seq_params(mac_vap_stru *mac_vap, uint32_t *params)
{
    uint32_t channel_seq_bitmap;
    oal_bool_enum_uint8 is_single_chba_up;
    hmac_chba_vap_stru *chba_vap_info = NULL;
    mac_device_stru *mac_device = NULL;
    hmac_vap_stru *hmac_vap = NULL;
    struct chba_channel_sequence_params channel_seq_params;

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_vap == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_CHBA, "{hmac_chba_set_channel_seq_params:fail to get hmac_vap}");
        return OAL_FAIL;
    }
    chba_vap_info = hmac_chba_get_chba_vap_info(hmac_vap);
    if (chba_vap_info == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_CHBA, "{hmac_chba_set_channel_seq_params:not chba vap!}");
        return OAL_FAIL;
    }

    channel_seq_params.data.test_params.channel_seq_level = params[0];  // 0: test cmd 0
    channel_seq_params.data.test_params.first_work_channel = params[1]; // 1: test cmd 1
    channel_seq_params.data.test_params.sec_work_channel = params[2];   // 2: test cmd 2
    /* 根据信道序列等级, 配置channle_sequenc_bitmap */
    channel_seq_bitmap = chba_channel_seq_level_to_bitmap(channel_seq_params.data.test_params.channel_seq_level);
    hmac_chba_set_channel_seq_bitmap(chba_vap_info, channel_seq_bitmap);

    /* 单CHBA vap工作场景，才能配置STA 工作信道 */
    mac_device = mac_res_get_mac_dev();
    is_single_chba_up = mac_device_is_single_chba_up(mac_device);
    if (is_single_chba_up == OAL_TRUE) {
        /* 单CHBA场景，根据命令信道参数构造工作信道结构体 */
        mac_channel_stru tmp_channel = chba_vap_info->work_channel;

        /* 配置social channel 和工作信道 */
        tmp_channel.uc_chan_number = channel_seq_params.data.test_params.first_work_channel;
        if (mac_get_channel_idx_from_num(tmp_channel.en_band, tmp_channel.uc_chan_number,
            tmp_channel.ext6g_band, &tmp_channel.uc_chan_idx) == OAL_SUCC) {
            hmac_config_chba_social_channel(hmac_vap, chba_vap_info, &tmp_channel, CHBA_INVALID_SWITCH_CHAN_SLOT_IDX);
            hmac_chba_set_first_work_channel(chba_vap_info, &tmp_channel);
        }

        /* 配置CHBA第二工作信道 */
        tmp_channel.uc_chan_number = channel_seq_params.data.test_params.sec_work_channel;
        if (mac_get_channel_idx_from_num(tmp_channel.en_band, tmp_channel.uc_chan_number,
            tmp_channel.ext6g_band, &tmp_channel.uc_chan_idx) == OAL_SUCC) {
            hmac_chba_set_sec_work_channel(chba_vap_info, &tmp_channel);
        }
    }

    oam_warning_log4(mac_vap->uc_vap_id, OAM_SF_CHBA,
        "hmac_chba_set_channel_seq_params: level %d, bitmap 0x%08x, first_work_channel %d, sec_work_channel %d",
        channel_seq_params.data.test_params.channel_seq_level,
        chba_vap_info->channel_sequence_bitmap,
        chba_vap_info->work_channel.uc_chan_number,
        chba_vap_info->second_work_channel.uc_chan_number);

    /* 更新信道序列参数后，更新beacon/pnf下发到device */
    hmac_chba_save_update_beacon_pnf(hmac_chba_sync_get_domain_bssid());

    return OAL_SUCC;
}

static void hmac_chba_get_other_vap_channel(mac_device_stru *mac_device, mac_channel_stru *tmp_channel)
{
    uint32_t index;
    mac_vap_stru *mac_vap = NULL;

    for (index = 0; index < mac_device->uc_vap_num; index++) {
        mac_vap = mac_res_get_mac_vap(mac_device->auc_vap_id[index]);
        if (mac_vap == NULL) {
            return;
        }
        if (mac_is_chba_mode(mac_vap) != OAL_TRUE && mac_vap_is_up(mac_vap) == OAL_TRUE) {
            *tmp_channel = mac_vap->st_channel;
            return;
        }
    }
    oam_error_log0(0, OAM_SF_CHBA,
        "hmac_chba_get_other_vap_channel: not find legacy sta/ap or go/gc");
}

/* 同步信道序列bitmap和第二工作信道到device */
static void hmac_chba_sync_channel_seq(hmac_chba_vap_stru *chba_vap_info,
    uint32_t channel_seq_bitmap, mac_channel_stru *sec_channel, chba_chan_seq_cfg_type_enum chba_chan_seq_cfg)
{
    struct chba_channel_sequence_params channel_seq_params;
    mac_vap_stru *chba_mac_vap = mac_res_get_mac_vap(chba_vap_info->mac_vap_id);
    if (chba_mac_vap == NULL) {
        oam_error_log0(chba_vap_info->mac_vap_id, OAM_SF_CHBA,
            "hmac_chba_sync_channel_seq:chba_mac_vap is NULL");
        return;
    }

    channel_seq_params.params_type = CHBA_CHANNEL_SEQ_PARAMS_TYPE_SYNC;
    channel_seq_params.chba_chan_seq_cfg_type = chba_chan_seq_cfg;
    channel_seq_params.data.sync_info_params.channel_seq_bitmap = channel_seq_bitmap;
    channel_seq_params.data.sync_info_params.work_channel = chba_vap_info->work_channel;
    channel_seq_params.data.sync_info_params.sec_work_channel = *sec_channel;

    hmac_config_send_event(chba_mac_vap, WLAN_CFGID_CHBA_SET_CHANNEL_SEQ_PARAMS,
        sizeof(struct chba_channel_sequence_params), (uint8_t *)&(channel_seq_params));
}

/* CHBA信道跟随场景，根据CHBA 工作信道和第二工作信道，选择第二工作信道带宽 */
void hmac_chba_update_sec_work_channel(const mac_channel_stru *work_channel,
    mac_channel_stru *second_work_channel)
{
    wlan_bw_cap_enum_uint8 tmp_bw_cap;
    wlan_bw_cap_enum_uint8 work_channel_bw_cap;
    wlan_bw_cap_enum_uint8 sec_work_channel_bw_cap;

    work_channel_bw_cap = mac_vap_bw_mode_to_bw(work_channel->en_bandwidth);
    sec_work_channel_bw_cap = mac_vap_bw_mode_to_bw(second_work_channel->en_bandwidth);
    if (sec_work_channel_bw_cap >= work_channel_bw_cap) {
        /* 场景1：第二工作信道带宽大于等于工作信道带宽，不用调整 */
        return;
    }

    /* 在第二工作信道带宽小于CHBA工作信道带宽时，查找第二工作信道是否能支持更高带宽，
     * 场景2：如果支持更高带宽，则将第二工作信道带宽配置为高带宽
     * 场景3：如果不能配置更高带宽，则device 在通信时需要降带宽
     */
    tmp_bw_cap = work_channel_bw_cap;
    while (tmp_bw_cap > sec_work_channel_bw_cap) {
        /* 当前仅考虑CHBA 工作在5G */
        wlan_channel_bandwidth_enum_uint8 new_bandwidth;
        wlan_bw_cap_enum_uint8 new_bw_cap;
        new_bandwidth = mac_regdomain_get_bw_by_channel_bw_cap(second_work_channel->uc_chan_number, tmp_bw_cap);
        new_bw_cap = mac_vap_bw_mode_to_bw(new_bandwidth);
        if (new_bw_cap > sec_work_channel_bw_cap) {
            /* 获取到该信道可用较大带宽， */
            second_work_channel->en_bandwidth = new_bandwidth;
            break;
        } else {
            tmp_bw_cap--;
        }
    }
}


static void hmac_chba_update_channel_seq(hmac_chba_vap_stru *chba_vap_info,
    mac_channel_stru *sec_channel, uint32_t channel_seq_bitmap, chba_chan_seq_cfg_type_enum chba_chan_seq_cfg)
{
    /* 关联成功，配置channle_sequenc_bitmap 0x33333333 */
    hmac_chba_set_channel_seq_bitmap(chba_vap_info, channel_seq_bitmap);

    /* 根据chba 和sta 信道和带宽，在信道支持的情况下，选择带宽大的信道作为CHBA第二工作信道 */
    hmac_chba_update_sec_work_channel(&(chba_vap_info->work_channel), sec_channel);

    /* 配置CHBA第二工作信道 */
    hmac_chba_set_sec_work_channel(chba_vap_info, sec_channel);

    oam_warning_log3(0, OAM_SF_CHBA,
        "hmac_chba_update_channel_seq: bitmap 0x%08x, first_work_channel %d, sec_work_channel %d",
        chba_vap_info->channel_sequence_bitmap,
        chba_vap_info->work_channel.uc_chan_number,
        chba_vap_info->second_work_channel.uc_chan_number);

    /* 更新信道序列参数后，更新beacon/pnf下发到device */
    hmac_chba_save_update_beacon_pnf(hmac_chba_sync_get_domain_bssid());

    /* 同步信道序列bitmap和CHBA第二工作信道到device */
    hmac_chba_sync_channel_seq(chba_vap_info, channel_seq_bitmap, sec_channel, chba_chan_seq_cfg);
}


void hmac_chba_update_channel_seq_when_connect(void)
{
    uint32_t channel_seq_bitmap;
    mac_channel_stru sec_channel = { 0 };
    hmac_chba_vap_stru *chba_vap_info = NULL;
    mac_device_stru *mac_device = mac_res_get_mac_dev();
    chba_chan_seq_cfg_type_enum chba_chan_seq_cfg;

    chba_vap_info = hmac_get_up_chba_vap_info();
    if (chba_vap_info == NULL) {
        return;
    }

    if (mac_device->en_dbac_running == OAL_FALSE) {
        /* 非DBAC状态，配置信道序列为全1 */
        channel_seq_bitmap = CHBA_CHANNEL_SEQ_BITMAP_100_PERCENT;
        /* 非DBAC状态，恢复CHBA第二工作信道为CHBA工作信道 */
        sec_channel = chba_vap_info->work_channel;
        chba_chan_seq_cfg = CHBA_CHAN_SEQ_DEFAULT_TYPE; /* chba dbac未运行，配置为默认信道序列 */
    } else {
        /* DBAC状态，更新信道序列bitmap为0x33333333；并更新CHBA第二工作信道为STA信道 */
        channel_seq_bitmap = CHBA_CHANNEL_SEQ_BITMAP_DEFAULT;

        /* 查找legacy sta信道，赋值给chba 第二工作信道 */
        hmac_chba_get_other_vap_channel(mac_device, &sec_channel);
        chba_chan_seq_cfg = CHBA_CHAN_SEQ_CHBA_DBAC_TYPE; /* 配置为chba dbac信道序列 */
    }

    hmac_chba_update_channel_seq(chba_vap_info, &sec_channel, channel_seq_bitmap, chba_chan_seq_cfg);
}
static mac_vap_stru *hmac_chba_dbac_find_non_chba_vap(dmac_dbac_notify_to_hmac_stru *dbac_notify_data)
{
    mac_vap_stru *led_vap = NULL;
    mac_vap_stru *flw_vap = NULL;
    mac_vap_stru *non_chba_vap = NULL;

    led_vap = mac_res_get_mac_vap(dbac_notify_data->led_vap_id);
    flw_vap = mac_res_get_mac_vap(dbac_notify_data->flw_vap_id);
    if (oal_any_null_ptr2(led_vap, flw_vap)) {
        return NULL;
    }

    if (mac_is_chba_mode(led_vap) == OAL_TRUE) {
        non_chba_vap = flw_vap;
    } else if (mac_is_chba_mode(flw_vap) == OAL_TRUE) {
        non_chba_vap = led_vap;
    } else {
        return NULL;
    }
    return non_chba_vap;
}
void hmac_chba_update_channel_seq_when_dbac_notify(dmac_dbac_notify_to_hmac_stru *dbac_notify_data)
{
    uint32_t channel_seq_bitmap;
    mac_channel_stru sec_channel = { 0 };
    hmac_chba_vap_stru *chba_vap_info = NULL;
    mac_vap_stru *non_chba_vap = NULL;
    mac_device_stru *mac_device = mac_res_get_mac_dev();
    chba_chan_seq_cfg_type_enum chba_chan_seq_cfg;

    if (dbac_notify_data == NULL) {
        return;
    }

    chba_vap_info = hmac_get_up_chba_vap_info();
    if (chba_vap_info == NULL) {
        return;
    }
    non_chba_vap = hmac_chba_dbac_find_non_chba_vap(dbac_notify_data);
    if (non_chba_vap == NULL) {
        return;
    }

    if (mac_device->en_dbac_running == OAL_FALSE) {
        /* 非DBAC状态，配置信道序列为全1 */
        channel_seq_bitmap = CHBA_CHANNEL_SEQ_BITMAP_100_PERCENT;
        /* 非DBAC状态，恢复CHBA第二工作信道为CHBA工作信道 */
        sec_channel = chba_vap_info->work_channel;
        chba_chan_seq_cfg = CHBA_CHAN_SEQ_DEFAULT_TYPE; /* chba dbac未运行，配置为默认信道序列 */
    } else {
        /* 如果non vap在csa流程未结束，则先不更新第二工作信道, 防止csa未结束更新带宽无校准数据 */
        if (dbac_notify_data->dbac_non_chba_csa_status == NON_CHBA_CSA_DOING) {
            oam_warning_log0(non_chba_vap->uc_vap_id, OAM_SF_DBAC,
                "{hmac_dbac_non_chba_csa_status_notify::dbac non chba vap in csa, do not update chan seq.}");
            return;
        }
        /* DBAC状态，更新信道序列bitmap为0x33333333；并更新CHBA第二工作信道为STA信道 */
        channel_seq_bitmap = CHBA_CHANNEL_SEQ_BITMAP_DEFAULT;

        /* 查找legacy sta信道，赋值给chba 第二工作信道 */
        sec_channel = non_chba_vap->st_channel;
        chba_chan_seq_cfg = CHBA_CHAN_SEQ_CHBA_DBAC_TYPE; /* 配置为chba dbac信道序列 */
    }

    hmac_chba_update_channel_seq(chba_vap_info, &sec_channel, channel_seq_bitmap, chba_chan_seq_cfg);
}

/* chba dbac fcs配置变化, 更新信道序列配置 */
uint32_t hmac_chba_dbac_update_fcs_config(mac_vap_stru *mac_vap, uint8_t len, uint8_t *param)
{
    dmac_dbac_notify_to_hmac_stru *dbac_notify_data = (dmac_dbac_notify_to_hmac_stru *)param;
    if (mac_is_chba_mode(mac_vap) == OAL_FALSE) {
        oam_error_log0(0, OAM_SF_CHBA, "{hmac_chba_dbac_update_fcs_config:not chba mode!}");
        return OAL_FAIL;
    }
    hmac_chba_update_channel_seq_when_dbac_notify(dbac_notify_data);
    return OAL_SUCC;
}
#endif /* _PRE_WLAN_CHBA_MGMT */
