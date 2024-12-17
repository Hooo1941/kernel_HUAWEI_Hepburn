/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : 扫描模块 hmac功能
 * 作    者 :
 * 创建日期 : 2014年11月26日
 */

/* 1 头文件包含 */
#include "hmac_scan.h"
#include "mac_frame_inl.h"
#include "wlan_chip_i.h"
#include "hmac_sme_sta.h"
#include "hmac_roam_scan.h"
#include "hmac_chan_mgmt.h"
#include "hmac_p2p.h"
#include "oam_event_wifi.h"
#ifdef _PRE_WLAN_FEATURE_SNIFFER
#ifdef CONFIG_HW_SNIFFER
#include <hwnet/ipv4/sysctl_sniffer.h>
#endif
#endif
#include "hmac_ht_self_cure.h"
#ifdef _PRE_WLAN_FEATURE_MBO
#include "hmac_mbo.h"
#endif
#ifdef _PRE_WLAN_FEATURE_11AX
#include "hmac_11ax.h"
#endif
#include "hmac_11r.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_SCAN_BSS_C

/*
 * 函 数 名  : hmac_wifi_hide_ssid
 * 功能描述  : 隐藏扫描到的bss的ssid信息
 * 1.日    期  : 2019年4月18日
  *   修改内容  : 新生成函数
 */
OAL_STATIC void hmac_wifi_hide_ssid(uint8_t *frame_body, uint16_t mac_frame_len)
{
    uint8_t *ssid_ie = NULL;
    uint8_t ssid_len = 0;
    uint8_t idx;

    if (frame_body == NULL) {
        return;
    }

    ssid_ie = mac_get_ssid(frame_body, mac_frame_len, &ssid_len);
    /* 保留4位，如果不足4位，则不隐藏 */
    if (ssid_ie == NULL || ssid_len < 4) {
        return;
    }

    for (idx = 2; idx < ssid_len - 2; idx++)  { // 保留前2位和后2位
        *(ssid_ie + idx) = 0x78;  // 0x78转换为ASCII码就是字符'x'
    }
}

/*
 * 函 数 名  : hmac_scan_print_scanned_bss_info
 * 功能描述  : 打印扫描到的bss信息
 * 1.日    期  : 2015年2月28日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void hmac_scan_print_scanned_bss_info(uint8_t device_id)
{
    hmac_device_stru *hmac_device = NULL;
    hmac_bss_mgmt_stru *bss_mgmt = NULL;
    hmac_scanned_bss_info *scanned_bss = NULL;
    mac_bss_dscr_stru *bss_dscr = NULL;
    oal_dlist_head_stru *entry = NULL;
    mac_ieee80211_frame_stru *frame_hdr = NULL;
    uint8_t sdt_parse_hdr[MAC_80211_FRAME_LEN];
    uint8_t *tmp_mac_body_addr = NULL;
    uint8_t frame_sub_type;
    int32_t ret;

    /* 获取hmac device */
    hmac_device = hmac_res_get_mac_dev(device_id);
    if (hmac_device == NULL) {
        oam_warning_log0(0, OAM_SF_SCAN, "{hmac_scan_print_scanned_bss_info::hmac_device null.}");
        return;
    }

    /* 获取指向扫描结果的管理结构体地址 */
    bss_mgmt = &(hmac_device->st_scan_mgmt.st_scan_record_mgmt.st_bss_mgmt);

    /* 获取锁 */
    oal_spin_lock(&(bss_mgmt->st_lock));

    /* 遍历扫描到的bss信息 */
    oal_dlist_search_for_each(entry, &(bss_mgmt->st_bss_list_head)) {
        scanned_bss = oal_dlist_get_entry(entry, hmac_scanned_bss_info, st_dlist_head);
        bss_dscr = &(scanned_bss->st_bss_dscr_info);

        /* 仅显示新申请到的BSS帧 */
        if (scanned_bss->st_bss_dscr_info.en_new_scan_bss == OAL_TRUE) {
            /* 上报beacon和probe帧 */
            frame_hdr = (mac_ieee80211_frame_stru *)bss_dscr->auc_mgmt_buff;

            /* 将beacon中duration字段(2字节)复用为rssi以及channel,方便SDT显示 */
            ret = memcpy_s((uint8_t *)sdt_parse_hdr, MAC_80211_FRAME_LEN, (uint8_t *)frame_hdr, MAC_80211_FRAME_LEN);
            sdt_parse_hdr[BYTE_OFFSET_2] = (uint8_t)bss_dscr->c_rssi;
            sdt_parse_hdr[BYTE_OFFSET_3] = bss_dscr->st_channel.uc_chan_number;
            if (bss_dscr->mgmt_len < MAC_80211_FRAME_LEN) {
                oam_error_log1(0, OAM_SF_SCAN,
                    "{hmac_scan_print_scanned_bss_info::mgmt len[%d] invalid.}", bss_dscr->mgmt_len);
                continue;
            }
            tmp_mac_body_addr = (uint8_t *)oal_memalloc(bss_dscr->mgmt_len - MAC_80211_FRAME_LEN);
            if (oal_unlikely(tmp_mac_body_addr == NULL)) {
                oam_warning_log0(0, OAM_SF_SCAN, "{hmac_scan_print_scanned_bss_info::alloc memory failed}");
                continue;
            }

            ret += memcpy_s(tmp_mac_body_addr, bss_dscr->mgmt_len - MAC_80211_FRAME_LEN,
                (uint8_t *)(bss_dscr->auc_mgmt_buff + MAC_80211_FRAME_LEN), bss_dscr->mgmt_len - MAC_80211_FRAME_LEN);
            if (ret != EOK) {
                oam_error_log0(0, OAM_SF_SCAN, "hmac_scan_print_scanned_bss_info::memcpy fail!");
                oal_free(tmp_mac_body_addr);
                /* 解除锁 */
                oal_spin_unlock(&(bss_mgmt->st_lock));
                return;
            }

            frame_sub_type = mac_get_frame_type_and_subtype((uint8_t *)frame_hdr);
            if ((frame_sub_type == WLAN_FC0_SUBTYPE_BEACON) || (frame_sub_type == WLAN_FC0_SUBTYPE_PROBE_RSP)) {
                hmac_wifi_hide_ssid(tmp_mac_body_addr, bss_dscr->mgmt_len - MAC_80211_FRAME_LEN);
            }

            /* 上报beacon帧或者probe rsp帧 */
            oam_report_80211_frame(BROADCAST_MACADDR, (uint8_t *)sdt_parse_hdr, MAC_80211_FRAME_LEN,
                tmp_mac_body_addr, (uint16_t)bss_dscr->mgmt_len, OAM_OTA_FRAME_DIRECTION_TYPE_RX);
            oal_free(tmp_mac_body_addr);
#ifdef _PRE_WLAN_FEATURE_SNIFFER
#ifdef CONFIG_HW_SNIFFER
            proc_sniffer_write_file(NULL, 0, bss_dscr->auc_mgmt_buff, (uint16_t)bss_dscr->mgmt_len, 0);
#endif
#endif
        }
    }

    /* 解除锁 */
    oal_spin_unlock(&(bss_mgmt->st_lock));

    return;
}

/*
 * 函 数 名  : hmac_scan_alloc_scanned_bss
 * 功能描述  : 申请内存，存储扫描到的bss信息
 * 1.日    期  : 2015年2月2日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC hmac_scanned_bss_info *hmac_scan_alloc_scanned_bss(uint32_t mgmt_len)
{
    hmac_scanned_bss_info *scanned_bss;

    /* 申请内存，存储扫描到的bss信息 */
    scanned_bss = oal_memalloc(sizeof(hmac_scanned_bss_info) + mgmt_len -
                               sizeof(scanned_bss->st_bss_dscr_info.auc_mgmt_buff));
    if (oal_unlikely(scanned_bss == NULL)) {
        oam_warning_log0(0, OAM_SF_SCAN,
                         "{hmac_scan_alloc_scanned_bss::alloc memory failed for storing scanned result.}");
        return NULL;
    }

    /* 为申请的内存清零 */
    memset_s(scanned_bss,
             sizeof(hmac_scanned_bss_info) + mgmt_len - sizeof(scanned_bss->st_bss_dscr_info.auc_mgmt_buff), 0,
             sizeof(hmac_scanned_bss_info) + mgmt_len - sizeof(scanned_bss->st_bss_dscr_info.auc_mgmt_buff));

    /* 初始化链表头节点指针 */
    oal_dlist_init_head(&(scanned_bss->st_dlist_head));

    return scanned_bss;
}

/*
 * 函 数 名  : hmac_scan_add_bss_to_list
 * 功能描述  : 将扫描到的bss添加到链表
 * 1.日    期  : 2015年2月2日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t hmac_scan_add_bss_to_list(hmac_scanned_bss_info *scanned_bss, hmac_device_stru *hmac_device)
{
    hmac_bss_mgmt_stru *bss_mgmt; /* 管理扫描结果的结构体 */

    bss_mgmt = &(hmac_device->st_scan_mgmt.st_scan_record_mgmt.st_bss_mgmt);
    scanned_bss->st_bss_dscr_info.en_new_scan_bss = OAL_TRUE;

    /* 对链表写操作前加锁 */
    oal_spin_lock(&(bss_mgmt->st_lock));

    /* 添加扫描结果到链表中，并更新扫描到的bss计数 */
    oal_dlist_add_tail(&(scanned_bss->st_dlist_head), &(bss_mgmt->st_bss_list_head));

    bss_mgmt->bss_num++;
    /* 解锁 */
    oal_spin_unlock(&(bss_mgmt->st_lock));

    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_scan_del_bss_from_list
 * 功能描述  : 将扫描到的bss添加到链表
 * 1.日    期  : 2015年2月2日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
static uint32_t hmac_scan_del_bss_from_list_nolock(hmac_scanned_bss_info *scanned_bss, hmac_device_stru *hmac_device)
{
    hmac_bss_mgmt_stru *bss_mgmt; /* 管理扫描结果的结构体 */

    bss_mgmt = &(hmac_device->st_scan_mgmt.st_scan_record_mgmt.st_bss_mgmt);

    /* 从链表中删除节点，并更新扫描到的bss计数 */
    oal_dlist_delete_entry(&(scanned_bss->st_dlist_head));

    bss_mgmt->bss_num--;

    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_is_connected_ap_bssid
 * 功能描述  : 判断输入bssid参数是否是关联的AP的bssid,用于不老化已经关联的AP
 * 1.日    期  : 2016年1月5日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC int32_t hmac_is_connected_ap_bssid(uint8_t device_id, uint8_t bssid[WLAN_MAC_ADDR_LEN])
{
    uint8_t vap_idx;
    mac_vap_stru *mac_vap = NULL;
    mac_device_stru *mac_device;

    mac_device = mac_res_get_dev(device_id);
    if (oal_unlikely(mac_device == NULL)) {
        oam_error_log0(0, OAM_SF_SCAN, "{hmac_is_connected_ap_bssid::mac_res_get_dev return null.}");
        return OAL_FALSE;
    }

    for (vap_idx = 0; vap_idx < mac_device->uc_vap_num; vap_idx++) {
        mac_vap = mac_res_get_mac_vap(mac_device->auc_vap_id[vap_idx]);
        if (oal_unlikely(mac_vap == NULL)) {
            oam_warning_log1(0, OAM_SF_P2P, "{hmac_is_connected_ap_bssid::mac_res_get_mac_vap fail! vap id is %d}",
                             mac_device->auc_vap_id[vap_idx]);
            continue;
        }

        if (is_legacy_vap(mac_vap) && (mac_vap->en_vap_state == MAC_VAP_STATE_UP)) {
            if (oal_memcmp(bssid, mac_vap->auc_bssid, WLAN_MAC_ADDR_LEN) == 0) {
                /* 不老化当前关联的AP */
                return OAL_TRUE;
            }
        }
    }

    return OAL_FALSE;
}

/*
 * 函 数 名  : hmac_scan_clean_expire_scanned_bss
 * 功能描述  : 本次扫描请求发起时，清除上次扫描结果中到期的bss信息
 * 1.日    期  : 2015年8月10日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void hmac_scan_clean_expire_scanned_bss(hmac_vap_stru *hmac_vap, hmac_scan_record_stru *scan_record)
{
    oal_dlist_head_stru *entry = NULL;
    oal_dlist_head_stru *entry_tmp = NULL;
    hmac_bss_mgmt_stru *bss_mgmt = NULL;
    hmac_scanned_bss_info *scanned_bss = NULL;
    mac_bss_dscr_stru *bss_dscr = NULL;
    uint32_t curr_time_stamp;

    /* 参数合法性检查 */
    if (scan_record == NULL) {
        oam_error_log0(0, OAM_SF_SCAN, "{hmac_scan_clean_expire_scanned_bss::scan record is null.}");
        return;
    }

    /* 管理扫描的bss结果的结构体 */
    bss_mgmt = &(scan_record->st_bss_mgmt);

    curr_time_stamp = (uint32_t)oal_time_get_stamp_ms();

    /* 对链表写操作前加锁 */
    oal_spin_lock(&(bss_mgmt->st_lock));

    /* 遍历链表，删除上一次扫描结果中到期的bss信息 */
    oal_dlist_search_for_each_safe(entry, entry_tmp, &(bss_mgmt->st_bss_list_head))
    {
        scanned_bss = oal_dlist_get_entry(entry, hmac_scanned_bss_info, st_dlist_head);
        bss_dscr = &(scanned_bss->st_bss_dscr_info);

        scanned_bss->st_bss_dscr_info.en_new_scan_bss = OAL_FALSE;

        //  use oal_time_after32
        if (oal_time_after32(curr_time_stamp, (bss_dscr->timestamp + HMAC_SCAN_MAX_SCANNED_BSS_EXPIRE)) == FALSE) {
            continue;
        }
        /* 产线老化使能 */
        if ((g_pd_bss_expire_time != 0) &&
            (curr_time_stamp - bss_dscr->timestamp < g_pd_bss_expire_time * HMAC_MS_TO_US)) {
            continue;
        }

        /* 不老化当前正在关联的AP */
        if (hmac_is_connected_ap_bssid(scan_record->uc_device_id, bss_dscr->auc_bssid)) {
            bss_dscr->c_rssi = hmac_vap->station_info.signal;
            continue;
        }

        /* 从链表中删除节点，并更新扫描到的bss计数 */
        oal_dlist_delete_entry(&(scanned_bss->st_dlist_head));
        bss_mgmt->bss_num--;

        /* 释放对应内存 */
        oal_free(scanned_bss);
    }

    /* 对链表写操作前加锁 */
    oal_spin_unlock(&(bss_mgmt->st_lock));
    return;
}

/*
 * 函 数 名  : hmac_scan_find_scanned_bss_dscr_by_index
 * 功能描述  : 根据bss index查找对应的bss dscr结构信息
 * 1.日    期  : 2015年2月2日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
mac_bss_dscr_stru *hmac_scan_find_scanned_bss_dscr_by_index(uint8_t device_id, uint32_t bss_index)
{
    oal_dlist_head_stru *entry = NULL;
    hmac_scanned_bss_info *scanned_bss = NULL;
    hmac_device_stru *hmac_device = NULL;
    hmac_bss_mgmt_stru *bss_mgmt = NULL;
    uint8_t loop = 0;

    /* 获取hmac device 结构 */
    hmac_device = hmac_res_get_mac_dev(device_id);
    if (hmac_device == NULL) {
        oam_error_log0(0, OAM_SF_SCAN, "{hmac_scan_find_scanned_bss_by_index::hmac_device is null.}");
        return NULL;
    }

    bss_mgmt = &(hmac_device->st_scan_mgmt.st_scan_record_mgmt.st_bss_mgmt);

    /* 对链表删操作前加锁 */
    oal_spin_lock(&(bss_mgmt->st_lock));

    /* 如果索引大于总共扫描的bss个数，返回异常 */
    if (bss_index >= bss_mgmt->bss_num) {
        oam_warning_log0(0, OAM_SF_SCAN, "{hmac_scan_find_scanned_bss_by_index::no such bss in bss list!}");

        /* 解锁 */
        oal_spin_unlock(&(bss_mgmt->st_lock));
        return NULL;
    }

    /* 遍历链表，返回对应index的bss dscr信息 */
    oal_dlist_search_for_each(entry, &(bss_mgmt->st_bss_list_head))
    {
        scanned_bss = oal_dlist_get_entry(entry, hmac_scanned_bss_info, st_dlist_head);

        /* 相同的bss index返回 */
        if (bss_index == loop) {
            /* 解锁 */
            oal_spin_unlock(&(bss_mgmt->st_lock));
            return &(scanned_bss->st_bss_dscr_info);
        }

        loop++;
    }
    /* 解锁 */
    oal_spin_unlock(&(bss_mgmt->st_lock));

    return NULL;
}

/*
 * 函 数 名  : hmac_scan_find_scanned_bss_by_bssid
 * 功能描述  : 查找相同的bssid的bss是否出现过
 * 1.日    期  : 2015年2月2日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
hmac_scanned_bss_info *hmac_scan_find_scanned_bss_by_bssid(hmac_bss_mgmt_stru *bss_mgmt, const uint8_t *bssid,
    uint8_t bssid_len)
{
    oal_dlist_head_stru *entry = NULL;
    hmac_scanned_bss_info *scanned_bss = NULL;

    /* 遍历链表，查找链表中是否已经存在相同bssid的bss信息 */
    oal_dlist_search_for_each(entry, &(bss_mgmt->st_bss_list_head))
    {
        scanned_bss = oal_dlist_get_entry(entry, hmac_scanned_bss_info, st_dlist_head);
        /* 相同的bssid地址 */
        if (oal_compare_mac_addr(scanned_bss->st_bss_dscr_info.auc_bssid, bssid) == 0) {
            return scanned_bss;
        }
    }

    return NULL;
}

/*
 * 函 数 名  : hmac_scan_get_scanned_bss_by_bssid
 * 功能描述  : 根据mac地址获取扫描结果中的bss信息
 * 1.日    期  : 2017年7月6日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void *hmac_scan_get_scanned_bss_by_bssid(mac_vap_stru *mac_vap, const uint8_t *mac_addr)
{
    hmac_bss_mgmt_stru *bss_mgmt = NULL; /* 管理扫描的bss结果的结构体 */
    hmac_scanned_bss_info *scanned_bss_info = NULL;
    hmac_device_stru *hmac_device;

    /* 获取hmac device 结构 */
    hmac_device = hmac_res_get_mac_dev(mac_vap->uc_device_id);
    if (hmac_device == NULL) {
        oam_warning_log1(0, OAM_SF_SCAN, "{hmac_scan_get_scanned_bss_by_bssid::hmac_device is null, dev id[%d].}",
                         mac_vap->uc_device_id);
        return NULL;
    }

    /* 获取管理扫描的bss结果的结构体 */
    bss_mgmt = &(hmac_device->st_scan_mgmt.st_scan_record_mgmt.st_bss_mgmt);

    oal_spin_lock(&(bss_mgmt->st_lock));

    scanned_bss_info = hmac_scan_find_scanned_bss_by_bssid(bss_mgmt, mac_addr, WLAN_MAC_ADDR_LEN);
    if (scanned_bss_info == NULL) {
        oam_warning_log3(mac_vap->uc_vap_id, OAM_SF_SCAN,
            "{hmac_scan_get_scanned_bss_by_bssid::find the bss failed[%02X:XX:XX:XX:%02X:%02X]}",
            mac_addr[MAC_ADDR_0], mac_addr[MAC_ADDR_4], mac_addr[MAC_ADDR_5]);

        /* 解锁 */
        oal_spin_unlock(&(bss_mgmt->st_lock));
        return NULL;
    }

    /* 解锁 */
    oal_spin_unlock(&(bss_mgmt->st_lock));

    return &(scanned_bss_info->st_bss_dscr_info);
}

/*
 * 函 数 名  : hmac_scan_update_bss_list_wmm
 * 功能描述  : 更新wmm相关信息
 * 1.日    期  : 2013年10月23日
  *   修改内容  : 新生成函数
 */
OAL_STATIC OAL_INLINE void hmac_scan_update_bss_list_wmm(mac_bss_dscr_stru *bss_dscr,
    uint8_t *frame_body, uint16_t frame_len)
{
    uint8_t *ie = NULL;

    bss_dscr->uc_wmm_cap = OAL_FALSE;
    bss_dscr->uc_uapsd_cap = OAL_FALSE;

    ie = mac_get_wmm_ie(frame_body, frame_len);
    if (ie != NULL) {
        bss_dscr->uc_wmm_cap = OAL_TRUE;

        /* --------------------------------------------------------------------------------- */
        /* WMM Information/Parameter Element Format                                          */
        /* ---------------------------------------------------------------------------------- */
        /* EID | IE LEN | OUI | OUIType | OUISubtype | Version | QoSInfo | OUISubtype based | */
        /* --------------------------------------------------------------------------------- */
        /* 1   |   1    |  3  | 1       | 1          | 1       | 1       | ---------------- | */
        /* --------------------------------------------------------------------------------- */
        /* ie[1] IE len 不包含EID和LEN字段,获取QoSInfo，uc_ie_len必须大于7字节长度 */
        /* Check if Bit 7 is set indicating U-APSD capability */
        if ((ie[1] >= 7) && (ie[8] & BIT7)) { /* wmm ie的第8个字节是QoS info字节 */
            bss_dscr->uc_uapsd_cap = OAL_TRUE;
        }
    } else {
        ie = mac_find_ie(MAC_EID_HT_CAP, frame_body, frame_len);
        if (ie != NULL) {
            /*  仅配置11ac、11a,不配11n，beacon无WMM信息，
            有HT信息short GI for 20M为0，以支持qos关联，eapol帧在VO队列发送不出去 */
            /* -------------------------------------------------------------- */
            /* HT capability Information/Parameter Element Format            */
            /* -------------------------------------------------------------- */
            /* EID | IE LEN |  HT capability Info |                 based   | */
            /* -------------------------------------------------------------- */
            /* 1   |   1    |         2           | ------------------------| */
            /* -------------------------------------------------------------- */
            /* ie[1] IE len 不包含EID和LEN字段,获取HT cap Info，uc_ie_len必须大于2字节长度 */
            /* ht cap的第 2,3个字节是HT capability Info信息 */
            /* Check if Bit 5 is set indicating short GI for 20M capability */
            if ((ie[1] >= 2) && (ie[2] & BIT5)) {
                bss_dscr->uc_wmm_cap = OAL_TRUE;
            }
        }
    }
}

#ifdef _PRE_WLAN_FEATURE_11D
/*
 * 函 数 名  : hmac_scan_update_bss_list_country
 * 功能描述  : 解析country IE
 * 1.日    期  : 2013年10月22日
  *   修改内容  : 新生成函数
 */
OAL_STATIC void hmac_scan_update_bss_list_country(mac_bss_dscr_stru *bss_dscr, uint8_t *frame_body, uint16_t frame_len)
{
    uint8_t *ie;

    ie = mac_find_ie(MAC_EID_COUNTRY, frame_body, frame_len);
    /* 国家码不存在, 全部标记为0 */
    if (ie == NULL) {
        bss_dscr->ac_country[BYTE_OFFSET_0] = 0;
        bss_dscr->ac_country[BYTE_OFFSET_1] = 0;
        bss_dscr->ac_country[BYTE_OFFSET_2] = 0;

        return;
    }
    bss_dscr->puc_country_ie = ie;
    /* 国家码采用2个字节,IE LEN必须大于等于2 */
    if (ie[1] >= 2) {
        bss_dscr->ac_country[BYTE_OFFSET_0] = (int8_t)ie[MAC_IE_HDR_LEN];
        bss_dscr->ac_country[BYTE_OFFSET_1] = (int8_t)ie[MAC_IE_HDR_LEN + 1];
        bss_dscr->ac_country[BYTE_OFFSET_2] = 0;
    }
}
#endif
#ifdef _PRE_WLAN_FEATURE_1024QAM
/*
 * 函 数 名  : hmac_scan_update_bss_list_rrm
 * 功能描述  : 判断对方是否携带1024qam
 * 1.日    期  : 2017年12月7日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC void hmac_scan_update_bss_list_1024qam(mac_bss_dscr_stru *bss_dscr, uint8_t *frame_body, uint16_t frame_len)
{
    uint8_t *ie;
    ie = mac_find_vendor_ie(MAC_HUAWEI_VENDER_IE, MAC_HISI_1024QAM_IE, frame_body, frame_len);
    if (ie == NULL) {
        bss_dscr->en_support_1024qam = OAL_FALSE;
    } else {
        bss_dscr->en_support_1024qam = OAL_TRUE;
    }
}
#endif

/*
 * 函 数 名  : hmac_scan_update_11i
 * 功能描述  : 更新11n相关信息
 * 1.日    期  : 2016年11月10日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC void hmac_scan_update_11i(mac_bss_dscr_stru *bss_dscr, uint8_t *frame_body, uint16_t frame_len)
{
    bss_dscr->puc_rsn_ie = mac_find_ie(MAC_EID_RSN, frame_body, (int32_t)(frame_len));
    bss_dscr->puc_wpa_ie = mac_find_vendor_ie(MAC_WLAN_OUI_MICROSOFT, MAC_OUITYPE_WPA,
                                              frame_body, (int32_t)(frame_len));
}
void hmac_scan_check_bss_bw(mac_bss_dscr_stru *bss_dscr, wlan_bw_cap_enum_uint8 *ht_op_bw)
{
    /* 如果是非法信道和带宽，则默认为20M */
    if (mac_regdomain_channel_is_support_bw(bss_dscr->en_channel_bandwidth,
        bss_dscr->st_channel.uc_chan_number, bss_dscr->st_channel.ext6g_band) == OAL_FALSE) {
        oam_warning_log2(0, OAM_SF_SCAN, "{hmac_scan_check_bss_bw::illegal bandwidth.channel[%d], bw[%d]}",
            bss_dscr->st_channel.uc_chan_number, bss_dscr->en_channel_bandwidth);
        bss_dscr->en_channel_bandwidth = WLAN_BAND_WIDTH_20M;
        bss_dscr->st_channel.en_bandwidth = WLAN_BAND_WIDTH_20M;
        *ht_op_bw = WLAN_BW_CAP_20M;
    }
}

/*
 * 函 数 名  : hmac_scan_update_bss_list_11n
 * 功能描述  : 更新11n相关信息
 * 1.日    期  : 2013年10月23日
  *   修改内容  : 新生成函数
 */
OAL_STATIC void hmac_scan_update_bss_list_11n(mac_bss_dscr_stru *bss_dscr, uint8_t *frame_body, uint16_t frame_len)
{
    uint8_t *ie = NULL;
    mac_ht_opern_ac_stru *ht_op = NULL;
    uint8_t sec_chan_offset;
    wlan_bw_cap_enum_uint8 en_ht_cap_bw = WLAN_BW_CAP_20M;
    wlan_bw_cap_enum_uint8 en_ht_op_bw = WLAN_BW_CAP_20M;

    if (hmac_ht_self_cure_in_blacklist(bss_dscr->auc_bssid)) {
        return;
    }

    /* 11n */
    ie = mac_find_ie(MAC_EID_HT_CAP, frame_body, frame_len);
    if ((ie != NULL) && (ie[1] >= 2)) { /* ie长度异常检查,需大于等于2 */
        /* ie[2]是HT Capabilities Info的第1个字节 */
        bss_dscr->en_ht_capable = OAL_TRUE;        /* 支持ht */
        bss_dscr->en_ht_ldpc = (ie[BYTE_OFFSET_2] & BIT0); /* 支持ldpc */
        en_ht_cap_bw = ((ie[BYTE_OFFSET_2] & BIT1) >> 1);      /* 取出支持的带宽 */
        bss_dscr->en_ht_stbc = ((ie[BYTE_OFFSET_2] & BIT7) >> NUM_7_BITS);
    }

    /* 默认20M,如果帧内容未携带HT_OPERATION则可以直接采用默认值 */
    bss_dscr->en_channel_bandwidth = WLAN_BAND_WIDTH_20M;

    ie = mac_find_ie(MAC_EID_HT_OPERATION, frame_body, frame_len);
    if ((ie != NULL) && (ie[1] == sizeof(mac_ht_opern_ac_stru))) {  // 增加ie长度异常检查
        ht_op = (mac_ht_opern_ac_stru *)(ie + MAC_IE_HDR_LEN);

        /* 提取次信道偏移 */
        sec_chan_offset = ht_op->bit_secondary_chan_offset;
        bss_dscr->ht_opern_ccsf2 = ht_op->bit_chan_center_freq_seg2; /* 保存ht operation的ccsf2,用于带宽计算 */
        /* 防止ap的channel width=0, 但channel offset = 1或者3 此时以channel width为主 */
        /* ht cap 20/40 enabled && ht operation 40 enabled */
        if ((ht_op->bit_sta_chan_width != 0) && (en_ht_cap_bw > WLAN_BW_CAP_20M)) {  // cap > 20M才取channel bw
            if (sec_chan_offset == MAC_SCB) {
                bss_dscr->en_channel_bandwidth = WLAN_BAND_WIDTH_40MINUS;
                en_ht_op_bw = WLAN_BW_CAP_40M;
            } else if (sec_chan_offset == MAC_SCA) {
                bss_dscr->en_channel_bandwidth = WLAN_BAND_WIDTH_40PLUS;
                en_ht_op_bw = WLAN_BW_CAP_40M;
            }
        }
    }
    /* 如果是非法信道和带宽，则默认为20M */
    hmac_scan_check_bss_bw(bss_dscr, &en_ht_op_bw);
    /* 将AP带宽能力取声明能力的最小值，防止AP异常发送超过带宽能力数据，造成数据不通 */
    bss_dscr->en_bw_cap = oal_min(en_ht_cap_bw, en_ht_op_bw);

    ie = mac_find_ie(MAC_EID_EXT_CAPS, frame_body, frame_len);
    if ((ie != NULL) && (ie[1] >= 1)) {
        /* Extract 20/40 BSS Coexistence Management Support */
        bss_dscr->uc_coex_mgmt_supp = (ie[BYTE_OFFSET_2] & BIT0);
#ifdef _PRE_WLAN_FEATURE_11AX
        if (g_wlan_spec_cfg->feature_11ax_is_open) {
            /* 查看 BSS Multi BSS支持情况 */
            if (ie[1] > 2) { /* 2: ie长度检查 */
                bss_dscr->st_mbssid_info.bit_ext_cap_multi_bssid_activated = ((ie[BYTE_OFFSET_4] & BIT6) >> NUM_6_BITS);
            }
        }
#endif
    }
}

OAL_STATIC void hmac_scan_update_bss_list_11ac_vht_cap(mac_bss_dscr_stru *bss_dscr,
    uint8_t *frame_body, uint16_t frame_len, uint32_t en_is_vendor_ie, uint8_t vendor_subtype)
{
    uint8_t *ie = NULL;
    oal_bool_enum_uint8 en_vht_capable;
    uint8_t supp_ch_width;

    ie = mac_find_ie(MAC_EID_VHT_CAP, frame_body, frame_len);
    if ((ie != NULL) && (ie[1] >= MAC_VHT_CAP_IE_LEN)) {
        /*  如果AP有VHT IE，但是VHT Capabilities是全0并且RX MCS全部不支持，认为对端AP不支持11ac */
        en_vht_capable = ((ie[BYTE_OFFSET_2] != 0) || (ie[BYTE_OFFSET_3] != 0) || (ie[BYTE_OFFSET_4] != 0) ||
            (ie[BYTE_OFFSET_5] != 0) || (ie[BYTE_OFFSET_6] != 0xff) || (ie[BYTE_OFFSET_7] != 0xff));
        if (en_vht_capable == OAL_TRUE) {
            bss_dscr->en_vht_capable = OAL_TRUE; /* 支持vht */
        }

        /* 说明vendor中携带VHT ie，则设置标志位，assoc req中也需携带vendor+vht ie */
        if (en_is_vendor_ie == OAL_TRUE) {
            bss_dscr->en_vendor_vht_capable = OAL_TRUE;
        }

        /* 提取Supported Channel Width Set */
        supp_ch_width = ((ie[BYTE_OFFSET_2] & (BIT3 | BIT2)) >> NUM_2_BITS);
        bss_dscr->supported_channel_width = supp_ch_width;
        bss_dscr->extend_nss_bw_supp = (ie[BYTE_OFFSET_5] & (BIT6 | BIT7)) >> BIT_OFFSET_6;
        if (bss_dscr->st_channel.en_band != WLAN_BAND_2G) {
            if (supp_ch_width == 0) {                   /* 0: 80MHz */
                bss_dscr->en_bw_cap = WLAN_BW_CAP_80M;
            } else if (supp_ch_width == 1) {            /* 1: 160MHz */
                bss_dscr->en_bw_cap = WLAN_BW_CAP_160M;
            } else if (supp_ch_width == 2) {            /* 2: 80MHz */
                bss_dscr->en_bw_cap = WLAN_BW_CAP_80M;
            }
        }
    } else {
        /* 私有vendor中不包含vht ie，适配BCM 5g 20M 私有协议 */
        if (en_is_vendor_ie == OAL_TRUE) {
            bss_dscr->en_vendor_novht_capable = OAL_TRUE;
            if ((get_mpxx_subchip_type() != BOARD_VERSION_MP13) &&
                ((vendor_subtype == MAC_WLAN_OUI_VENDOR_VHT_SUBTYPE2) ||
                (vendor_subtype == MAC_WLAN_OUI_VENDOR_VHT_SUBTYPE4))) {
                bss_dscr->en_vendor_1024qam_capable = OAL_TRUE;
            } else {
                bss_dscr->en_vendor_1024qam_capable = OAL_FALSE;
            }
        }
    }
}
OAL_STATIC OAL_INLINE void hmac_scan_update_bss_list_11ac_80m_ext(mac_bss_dscr_stru *bss_dscr,
    uint8_t chan_center_freq)
{
    switch (chan_center_freq - bss_dscr->st_channel.uc_chan_number) {
        case CHAN_OFFSET_PLUS_6:
            /***********************************************************************
            | 主20 | 从20 | 从40       |
                        |
                        |中心频率相对于主20偏6个信道
            ************************************************************************/
            bss_dscr->en_channel_bandwidth = WLAN_BAND_WIDTH_80PLUSPLUS;
            break;
        case CHAN_OFFSET_MINUS_2:
            /***********************************************************************
            | 从40        | 主20 | 从20 |
                        |
                        |中心频率相对于主20偏-2个信道
            ************************************************************************/
            bss_dscr->en_channel_bandwidth = WLAN_BAND_WIDTH_80PLUSMINUS;
            break;
        case CHAN_OFFSET_PLUS_2:
            /***********************************************************************
            | 从20 | 主20 | 从40       |
                        |
                        |中心频率相对于主20偏2个信道
            ************************************************************************/
            bss_dscr->en_channel_bandwidth = WLAN_BAND_WIDTH_80MINUSPLUS;
            break;
        case CHAN_OFFSET_MINUS_6:
            /***********************************************************************
            | 从40        | 从20 | 主20 |
                        |
                        |中心频率相对于主20偏-6个信道
            ************************************************************************/
            bss_dscr->en_channel_bandwidth = WLAN_BAND_WIDTH_80MINUSMINUS;
            break;
        default:
            break;
    }
}

OAL_STATIC void hmac_scan_update_bss_list_11ac_160m(mac_bss_dscr_stru *bss_dscr, uint8_t chan_center_freq)
{
    switch (chan_center_freq - bss_dscr->st_channel.uc_chan_number) {
        case CHAN_OFFSET_PLUS_14:
            bss_dscr->en_channel_bandwidth = WLAN_BAND_WIDTH_160PLUSPLUSPLUS;
            break;
        case CHAN_OFFSET_PLUS_10:
            bss_dscr->en_channel_bandwidth = WLAN_BAND_WIDTH_160MINUSPLUSPLUS;
            break;
        case CHAN_OFFSET_PLUS_6:
            bss_dscr->en_channel_bandwidth = WLAN_BAND_WIDTH_160PLUSMINUSPLUS;
            break;
        case CHAN_OFFSET_PLUS_2:
            bss_dscr->en_channel_bandwidth = WLAN_BAND_WIDTH_160MINUSMINUSPLUS;
            break;
        case CHAN_OFFSET_MINUS_2:
            bss_dscr->en_channel_bandwidth = WLAN_BAND_WIDTH_160PLUSPLUSMINUS;
            break;
        case CHAN_OFFSET_MINUS_6:
            bss_dscr->en_channel_bandwidth = WLAN_BAND_WIDTH_160MINUSPLUSMINUS;
            break;
        case CHAN_OFFSET_MINUS_10:
            bss_dscr->en_channel_bandwidth = WLAN_BAND_WIDTH_160PLUSMINUSMINUS;
            break;
        case CHAN_OFFSET_MINUS_14:
            bss_dscr->en_channel_bandwidth = WLAN_BAND_WIDTH_160MINUSMINUSMINUS;
            break;
        default:
            break;
    }
}
OAL_STATIC void hmac_scan_update_bss_list_11ac_80m(mac_bss_dscr_stru *bss_dscr,
    uint8_t chan_center_freq, uint8_t chan_center_freq_1)
{
#ifdef _PRE_WLAN_FEATURE_160M
    if ((chan_center_freq_1 - chan_center_freq == CHAN_OFFSET_PLUS_8) ||
        (chan_center_freq - chan_center_freq_1 == CHAN_OFFSET_PLUS_8)) {
        hmac_scan_update_bss_list_11ac_160m(bss_dscr, chan_center_freq_1);
        return;
    }
#endif
    hmac_scan_update_bss_list_11ac_80m_ext(bss_dscr, chan_center_freq);
}

/*
 * 函 数 名  : hmac_get_vht_ccfs1
 * 功能描述  : 获取ccsf1,以计算带宽
 * 1.日    期  : 2020年11月20日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint8_t hmac_get_vht_ccfs1(mac_bss_dscr_stru *bss_dscr, uint8_t *vht_oper_ie)
{
    uint8_t channel_center_freq_seg1;
    if ((bss_dscr->supported_channel_width == 0) && (bss_dscr->extend_nss_bw_supp != 0)) {
        channel_center_freq_seg1 = (bss_dscr->en_support_max_nss == WLAN_SINGLE_NSS) ? 0 : bss_dscr->ht_opern_ccsf2;
    } else {
        channel_center_freq_seg1 = vht_oper_ie[BYTE_OFFSET_4];
    }
    return channel_center_freq_seg1;
}
/*
 * 函 数 名  : hmac_scan_update_bss_list_11ac
 * 功能描述  : 更新11ac相关bss信息
 * 1.日    期  : 2013年10月9日
  *   修改内容  : 新生成函数
 */
OAL_STATIC void hmac_scan_update_bss_list_11ac(mac_bss_dscr_stru *bss_dscr, uint8_t *frame_body, uint16_t frame_len,
    uint32_t en_is_vendor_ie, uint8_t vendor_subtype)
{
    uint8_t *ie = NULL;
    uint8_t vht_chan_width, chan_center_freq, chan_center_freq_1;

    hmac_scan_update_bss_list_11ac_vht_cap(bss_dscr, frame_body, frame_len, en_is_vendor_ie, vendor_subtype);

    ie = mac_find_ie(MAC_EID_VHT_OPERN, frame_body, frame_len);
    if ((ie != NULL) && (ie[1] >= MAC_VHT_OPERN_LEN)) {
        vht_chan_width = ie[BYTE_OFFSET_2];
        chan_center_freq = ie[BYTE_OFFSET_3];
        chan_center_freq_1 = hmac_get_vht_ccfs1(bss_dscr, ie);
        /* 更新带宽信息 */
        if (vht_chan_width == WLAN_MIB_VHT_OP_WIDTH_20_40) { /* 40MHz */
            /* do nothing，en_channel_bandwidth已经在HT Operation IE中获取 */
        } else if (vht_chan_width == WLAN_MIB_VHT_OP_WIDTH_80) { /* 80MHz */
            hmac_scan_update_bss_list_11ac_80m(bss_dscr, chan_center_freq, chan_center_freq_1);
#ifdef _PRE_WLAN_FEATURE_160M
        } else if (vht_chan_width == WLAN_MIB_VHT_OP_WIDTH_160) { /* 2: 160MHz */
            hmac_scan_update_bss_list_11ac_160m(bss_dscr, chan_center_freq);
#endif
        } else if (vht_chan_width == WLAN_MIB_VHT_OP_WIDTH_80PLUS80) { /* 3: 80+80MHz */
            hmac_scan_update_bss_list_11ac_80m_ext(bss_dscr, chan_center_freq);
        }
    }
}

void hmac_scan_update_sap_mode(mac_bss_dscr_stru *bss_dscr, uint8_t *payload, uint16_t frame_len)
{
    uint8_t *sap_ie;
    uint8_t payload_len;
    mac_sap_mode_enum sap_mode;

    /*******************************************************************************************
     ------------------------------------------------------------------------------------
     |Element ID| Length |        OUI        |OUI TYPE|SAP mode|wired  |SAP rank|rank sta|
                                                               |connect|
     ------------------------------------------------------------------------------------
     |    1     |   1    |         3         |    1   |   1    |   1   |   1   |   1   |
     ------------------------------------------------------------------------------------
     |   0xdd   |  0x08  |      0xAC853D     |   0xb0 |  0/1/2 |   0   | 0/1/2 |   0   |
     ------------------------------------------------------------------------------------
    *********************************************************************************************/
    sap_ie = mac_find_vendor_ie(MAC_HUAWEI_VENDER_IE, MAC_HISI_SAP_IE, payload, frame_len);
    if (sap_ie == NULL) {
        bss_dscr->sap_mode = MAC_SAP_OFF;
        return;
    }

    payload_len = sap_ie[1];
    if (payload_len < MAC_HISI_SAP_IE_LEN) {
        bss_dscr->sap_mode = MAC_SAP_OFF;
        oam_error_log1(0, OAM_SF_RX, "hmac_scan_update_sap_mode::payload_len[%u] err!", payload_len);
        return;
    }

    sap_mode = sap_ie[MAC_IE_HDR_LEN + MAC_OUI_LEN + MAC_OUITYPE_LEN];
    if (sap_mode == MAC_SAP_MASTER || sap_mode == MAC_SAP_SLAVE) {
        bss_dscr->sap_mode = sap_mode;
    }
}

OAL_STATIC OAL_INLINE void hmac_scan_update_hisi_cap_ie(hmac_vap_stru *hmac_vap,
    mac_bss_dscr_stru *bss_dscr, uint8_t *frame_body, uint16_t frame_len)
{
    mac_hisi_cap_vendor_ie_stru hisi_vap_ie;

    if (mac_get_hisi_cap_vendor_ie(frame_body, frame_len, &hisi_vap_ie) == OAL_SUCC) {
        if (hisi_vap_ie.bit_11ax_support != OAL_FALSE && is_custom_open_11ax_switch(&hmac_vap->st_vap_base_info)) {
            bss_dscr->en_he_capable  = hisi_vap_ie.bit_11ax_support;
        }
        bss_dscr->en_dcm_support = hisi_vap_ie.bit_dcm_support;
        bss_dscr->en_p2p_scenes  = hisi_vap_ie.bit_p2p_scenes;
    }
}

OAL_STATIC OAL_INLINE void hmac_scan_update_cap_ie_part1(hmac_vap_stru *hmac_vap,
    mac_bss_dscr_stru *bss_dscr, uint8_t *frame_body, uint16_t frame_len)
{
    /* 11ax */
#ifdef _PRE_WLAN_FEATURE_11AX
    if (g_wlan_spec_cfg->feature_11ax_is_open) {
        hmac_scan_update_11ax_ie(hmac_vap, bss_dscr, frame_body, frame_len);
    }
#endif
    hmac_scan_update_hisi_cap_ie(hmac_vap, bss_dscr, frame_body, frame_len);
}

/*
 * 函 数 名  : hmac_scan_btcoex_backlist_check_by_oui
 * 功能描述  : 通过oui信息判断是否需要加入共存的黑名单中去
 * 1.日    期  : 2019年3月7日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void hmac_scan_btcoex_backlist_check_by_oui(mac_bss_dscr_stru *bss_dscr, uint8_t *frame_body, uint16_t frame_len)
{
    /* 初始化为非黑名单 */
    bss_dscr->en_btcoex_blacklist_chip_oui = 0;

    if (mac_find_vendor_ie(MAC_WLAN_CHIP_OUI_RALINK, MAC_WLAN_CHIP_OUI_TYPE_RALINK, frame_body, frame_len)
        != NULL ||
        mac_find_vendor_ie(MAC_WLAN_CHIP_OUI_SHENZHEN, MAC_WLAN_CHIP_OUI_TYPE_SHENZHEN, frame_body, frame_len)
        != NULL ||
        mac_find_vendor_ie(MAC_WLAN_CHIP_OUI_BROADCOM, MAC_WLAN_CHIP_OUI_TYPE_BROADCOM, frame_body, frame_len)
        != NULL ||
        mac_find_vendor_ie(MAC_WLAN_CHIP_OUI_ATHEROSC, MAC_WLAN_CHIP_OUI_TYPE_ATHEROSC, frame_body, frame_len)
        != NULL ||
        mac_find_vendor_ie(MAC_WLAN_CHIP_OUI_REALTEK, MAC_WLAN_CHIP_OUI_TYPE_REALTEK, frame_body, frame_len)
        != NULL ||
        mac_find_vendor_ie(MAC_WLAN_CHIP_OUI_METALINK, MAC_WLAN_CHIP_OUI_TYPE_METALINK, frame_body, frame_len)
        != NULL) {
        bss_dscr->en_btcoex_blacklist_chip_oui |= MAC_BTCOEX_BLACKLIST_LEV0;
    }

    /*  判断是否为苹果AP */
    if (((mac_find_vendor_ie(MAC_WLAN_CHIP_OUI_APPLE1, MAC_WLAN_CHIP_OUI_TYPE_APPLE_1_1, frame_body, frame_len)
        != NULL) ||
        (mac_find_vendor_ie(MAC_WLAN_CHIP_OUI_APPLE1, MAC_WLAN_CHIP_OUI_TYPE_APPLE_1_2, frame_body, frame_len)
        != NULL) ||
        (mac_find_vendor_ie(MAC_WLAN_CHIP_OUI_APPLE2, MAC_WLAN_CHIP_OUI_TYPE_APPLE_2_1, frame_body, frame_len)
        != NULL)) &&
        (mac_find_vendor_ie(MAC_WLAN_CHIP_OUI_BROADCOM, MAC_WLAN_CHIP_OUI_TYPE_BROADCOM, frame_body, frame_len)
        != NULL)) {
        bss_dscr->en_btcoex_blacklist_chip_oui |= MAC_BTCOEX_BLACKLIST_LEV1;
    }
}

OAL_STATIC OAL_INLINE void hmac_scan_update_bss_by_chip_oui(mac_bss_dscr_stru *bss_dscr,
                                                            uint8_t *frame_body, uint16_t frame_len)
{
    if (mac_find_vendor_ie(MAC_WLAN_CHIP_OUI_ATHEROSC, MAC_WLAN_CHIP_OUI_TYPE_ATHEROSC,
        frame_body, frame_len) != NULL) {
        bss_dscr->en_atheros_chip_oui = OAL_TRUE;
    } else {
        bss_dscr->en_atheros_chip_oui = OAL_FALSE;
    }

    if (mac_find_vendor_ie(MAC_WLAN_CHIP_OUI_BROADCOM, MAC_WLAN_CHIP_OUI_TYPE_BROADCOM,
        frame_body, frame_len) != NULL) {
        bss_dscr->en_roam_blacklist_chip_oui = OAL_TRUE;
    } else {
        bss_dscr->en_roam_blacklist_chip_oui = OAL_FALSE;
    }

    if (mac_find_vendor_ie(MAC_WLAN_CHIP_OUI_MARVELL, MAC_WLAN_CHIP_OUI_TYPE_MARVELL,
        frame_body, frame_len) != NULL) {
        bss_dscr->en_txbf_blacklist_chip_oui = OAL_TRUE;
    } else {
        bss_dscr->en_txbf_blacklist_chip_oui = OAL_FALSE;
    }

    /* 用于识别TP-LINK 7300，记录芯片厂家OUI，其他芯片厂家记录需要时再增加 */
    if (mac_find_vendor_ie(MAC_WLAN_CHIP_OUI_RALINK, MAC_WLAN_CHIP_OUI_TYPE_RALINK, frame_body, frame_len) != NULL) {
        bss_dscr->chip_oui = WLAN_AP_CHIP_OUI_RALINK;
    } else if (mac_find_vendor_ie(MAC_WLAN_CHIP_OUI_BROADCOM, MAC_WLAN_CHIP_OUI_TYPE_BROADCOM, frame_body, frame_len)
        != NULL) {
        bss_dscr->chip_oui = WLAN_AP_CHIP_OUI_BCM;
    } else if (mac_find_vendor_ie(MAC_WLAN_CHIP_OUI_QUALCOMM, MAC_WLAN_CHIP_OUI_TYPE_QUALCOMM, frame_body, frame_len)
        != NULL) {
        bss_dscr->chip_oui = WLAN_AP_CHIP_OUI_QLM;
    }

    /* 识别同时包含RALINK和MTK vendor ie, 可能有keepalive保活兼容性问题 */
    bss_dscr->keepalive_compatible_chip_oui = OAL_FALSE;
    if (mac_find_vendor_ie(MAC_WLAN_CHIP_OUI_RALINK, MAC_WLAN_CHIP_OUI_TYPE_RALINK_2, frame_body, frame_len) != NULL &&
        mac_find_vendor_ie(MAC_WLAN_CHIP_OUI_MTK, MAC_WLAN_CHIP_OUI_TYPE_MTK, frame_body, frame_len) != NULL) {
        bss_dscr->keepalive_compatible_chip_oui = OAL_TRUE;
    }
}

/*
 * 函 数 名  : hmac_scan_update_bss_list_protocol
 * 功能描述  : 更新协议类 bss信息
 * 1.日    期  : 2013年6月25日
  *   修改内容  : 新生成函数
 */
OAL_STATIC OAL_INLINE void hmac_scan_update_bss_list_protocol(hmac_vap_stru *hmac_vap, mac_bss_dscr_stru *bss_dscr,
                                                              uint8_t *frame_body, uint16_t frame_len)
{
    uint8_t *ie = NULL;
    uint16_t offset_vendor_vht = MAC_WLAN_OUI_VENDOR_VHT_HEADER + MAC_IE_HDR_LEN;

    frame_body += MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN;
    frame_len -= MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN;

    /*************************************************************************/
    /*                       Beacon Frame - Frame Body                       */
    /* ---------------------------------------------------------------------- */
    /* |Timestamp|BcnInt|CapInfo|SSID|SupRates|DSParamSet|TIM  |CountryElem | */
    /* ---------------------------------------------------------------------- */
    /* |8        |2     |2      |2-34|3-10    |3         |6-256|8-256       | */
    /* ---------------------------------------------------------------------- */
    /* |PowerConstraint |Quiet|TPC Report|ERP |RSN  |WMM |Extended Sup Rates| */
    /* ---------------------------------------------------------------------- */
    /* |3               |8    |4         |3   |4-255|26  | 3-257            | */
    /* ---------------------------------------------------------------------- */
    /* |BSS Load |HT Capabilities |HT Operation |Overlapping BSS Scan       | */
    /* ---------------------------------------------------------------------- */
    /* |7        |28              |24           |16                         | */
    /* ---------------------------------------------------------------------- */
    /* |Extended Capabilities |                                              */
    /* ---------------------------------------------------------------------- */
    /* |3-8                   |                                              */
    /*************************************************************************/
    /* wmm */
    hmac_scan_update_bss_list_wmm(bss_dscr, frame_body, frame_len);

    /* 11i */
    hmac_scan_update_11i(bss_dscr, frame_body, frame_len);

#ifdef _PRE_WLAN_FEATURE_11D
    /* 11d */
    hmac_scan_update_bss_list_country(bss_dscr, frame_body, frame_len);
#endif

    /* 11n */
    hmac_scan_update_bss_list_11n(bss_dscr, frame_body, frame_len);

    /* rrm */
#if defined(_PRE_WLAN_FEATURE_11K) || defined(_PRE_WLAN_FEATURE_FTM)
    hmac_scan_update_bss_list_rrm(bss_dscr, frame_body, frame_len);
#endif

    /* 11ac */
    hmac_scan_update_bss_list_11ac(bss_dscr, frame_body, frame_len, OAL_FALSE, OAL_FALSE);

    hmac_scan_update_cap_ie_part1(hmac_vap, bss_dscr, frame_body, frame_len);
    /* 查找私有vendor ie */
    ie = mac_find_vendor_ie(MAC_WLAN_OUI_BROADCOM_EPIGRAM, MAC_WLAN_OUI_VENDOR_VHT_TYPE, frame_body, frame_len);
    if ((ie != NULL) && (ie[1] >= MAC_WLAN_OUI_VENDOR_VHT_HEADER)) {
        hmac_scan_update_bss_list_11ac(bss_dscr, ie + offset_vendor_vht,
                                       ie[1] - MAC_WLAN_OUI_VENDOR_VHT_HEADER, OAL_TRUE, ie[BYTE_OFFSET_6]);
    }

#ifdef _PRE_WLAN_FEATURE_1024QAM
    hmac_scan_update_bss_list_1024qam(bss_dscr, frame_body, frame_len);
#endif

#ifdef _PRE_WLAN_FEATURE_MBO
    /* 检测AP是否带含有assoc disallowed attr的MBO IE，MBO定制化开关打开才进行查找对应MBO IE,否则忽略 */
    if (hmac_vap->st_vap_base_info.st_mbo_para_info.uc_mbo_enable == OAL_TRUE) {
        hmac_scan_update_bss_assoc_disallowed_attr(bss_dscr, frame_body, frame_len);
    }
#endif

    /* SAP */
    hmac_scan_update_sap_mode(bss_dscr, frame_body, frame_len);

    hmac_scan_btcoex_backlist_check_by_oui(bss_dscr, frame_body, frame_len);

    hmac_scan_update_bss_by_chip_oui(bss_dscr, frame_body, frame_len);
}

OAL_STATIC uint32_t hmac_find_rate_in_rates(uint8_t rate, uint8_t *update_rate, uint8_t rate_num)
{
    uint32_t k;
    for (k = 0; k < rate_num; k++) {
        if (hmac_is_equal_rates(update_rate[k], rate)) {
            break;
        }
    }
    return k;
}
/*
 * 函 数 名  : hmac_check_bss_supp_rates
 * 功能描述  : 检查速率
 * 1.日    期  : 2016年04月12日
  *   修改内容  : 新生成函数
 */
uint8_t hmac_scan_check_bss_supp_rates(mac_device_stru *mac_dev, uint8_t *rate,
                                       uint8_t bss_rate_num, uint8_t *update_rate, uint8_t rate_size)
{
    mac_data_rate_stru *rates = NULL;
    uint32_t i, j, k;
    uint8_t rate_num = 0;

    if (rate_size > WLAN_USER_MAX_SUPP_RATES) {
        oam_error_log1(0, OAM_SF_SCAN, "{hmac_scan_get_scanned_bss_by_bssid::uc_rate_size err[%d].}", rate_size);
        return rate_num;
    }

    rates = mac_device_get_all_rates(mac_dev);

    for (i = 0; i < bss_rate_num; i++) {
        for (j = 0; j < MAC_DATARATES_PHY_80211G_NUM; j++) {
            if (!((hmac_is_equal_rates(rates[j].uc_mac_rate, rate[i])) && (rate_num < MAC_DATARATES_PHY_80211G_NUM))) {
                continue;
            }

             /* 去除重复速率 */
            k = hmac_find_rate_in_rates(rate[i], update_rate, rate_num);
            /* 当不存在重复速率时，k等于rate_num */
            if (k == rate_num) {
                update_rate[rate_num++] = rate[i];
            }
            break;
        }
    }

    return rate_num;
}
/*
 * 函 数 名  : hmac_scan_rm_repeat_sup_exsup_rates
 * 功能描述  : support_rates和extended_rates去除重复速率，一并合入扫描结果的速率集中
 * 1.日    期  : 2018年2月5日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void hmac_scan_rm_repeat_sup_exsup_rates(mac_bss_dscr_stru *bss_dscr, uint8_t *rates, uint8_t exrate_num)
{
    int i, j;
    for (i = 0; i < exrate_num; i++) {
        /* 去除重复速率 */
        for (j = 0; j < bss_dscr->uc_num_supp_rates; j++) {
            if (hmac_is_equal_rates(rates[i], bss_dscr->auc_supp_rates[j])) {
                break;
            }
        }

        /* 只有不存在重复速率时，j等于bss_dscr->uc_num_supp_rates */
        if (j == bss_dscr->uc_num_supp_rates && bss_dscr->uc_num_supp_rates < WLAN_USER_MAX_SUPP_RATES) {
            bss_dscr->auc_supp_rates[bss_dscr->uc_num_supp_rates++] = rates[i];
        }
    }
}

/* 辅助定位日志 */
void hmac_scan_update_bss_list_rates_log(mac_device_stru *mac_dev, uint8_t *rate, uint8_t num_rates, uint32_t ie_len)
{
    mac_data_rate_stru *rates = NULL;
    uint32_t i, j;
    rates = mac_device_get_all_rates(mac_dev);
    // 打印获取的ie信息
    for (i = 0; i < ie_len; i++) {
        oam_warning_log1(0, OAM_SF_SCAN, "{hmac_scan_update_bss_list_rates_log::rate[%d].}", rate[i]);
    }
    // 打印 11g速率
    for (j = 0; j < MAC_DATARATES_PHY_80211G_NUM; j++) {
        oam_warning_log1(0, OAM_SF_SCAN, "{uc_mac_rate[%d].}", rates[j].uc_mac_rate);
    }
    oam_warning_log2(0, OAM_SF_SCAN, "{ie_len[%d], num_rates[%d].}", ie_len, num_rates);
}

/*
 * 函 数 名  : hmac_scan_remove_11b_rate
 * 功能描述  : 去除5G频段中11b的速率
 * 1.日    期  : 2018年11月14日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
#define MAC_DATARATES_80211B_NUM     4
OAL_STATIC void hmac_scan_remove_11b_rate(mac_bss_dscr_stru *bss_dscr, uint8_t *rate_num)
{
    /* 基础速率大小在IE中表现为原数值大小乘2。11b速率大小为1, 2, 5.5, 11 */
    uint8_t rate_11b[MAC_DATARATES_80211B_NUM] = { 2, 4, 11, 22 };
    uint8_t target_rate[WLAN_USER_MAX_SUPP_RATES] = { 0 };
    uint8_t *avail_rate = bss_dscr->auc_supp_rates;
    uint8_t target_rate_num = 0;
    oal_bool_enum_uint8 en_is_11b_rate;
    uint8_t i, j;

    for (i = 0; i < *rate_num; i++) {
        en_is_11b_rate = OAL_FALSE;

        for (j = 0; j < MAC_DATARATES_80211B_NUM; j++) {
            if (hmac_is_equal_rates(avail_rate[i], rate_11b[j])) {
                en_is_11b_rate = OAL_TRUE;
                break;
            }
        }

        if (!en_is_11b_rate) {
            target_rate[target_rate_num++] = avail_rate[i];
        }
    }

    /* copy原速率集长度，用于清空后面多出来的速率 */
    if (memcpy_s(avail_rate, WLAN_USER_MAX_SUPP_RATES, target_rate, *rate_num) == EOK) {
        *rate_num = target_rate_num;
        return;
    }
    oam_error_log0(0, OAM_SF_ANY, "hmac_scan_remove_11b_rate::memcpy fail!");
}

/*
 * 函 数 名  : hmac_scan_update_bss_list_rates
 * 功能描述  : 更新扫描到bss的速率集
 * 1.日    期  : 2013年7月25日
  *   修改内容  : 新生成函数
 */
OAL_STATIC OAL_INLINE uint32_t hmac_scan_update_bss_list_rates(mac_bss_dscr_stru *bss_dscr, uint8_t *frame_body,
                                                               uint16_t frame_len, mac_device_stru *mac_dev)
{
    uint8_t *ie = NULL;
    uint8_t num_rates = 0;
    uint8_t num_ex_rates;
    uint8_t offset;
    uint8_t rates[WLAN_USER_MAX_SUPP_RATES] = { 0 };

    /* 设置Beacon帧的field偏移量 */
    offset = MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN;

    ie = mac_find_ie(MAC_EID_RATES, frame_body + offset, frame_len - offset);
    if (ie != NULL) {
        num_rates = hmac_scan_check_bss_supp_rates(mac_dev, ie + MAC_IE_HDR_LEN, ie[1], rates, sizeof(rates));
        /*
         *  斐讯FIR304商用AP 11g模式，发送的支持速率集个数为12，
         * 不符合协议规定，为增加兼容性，修改判断分支为可选速率集上限16个
         */
        if (num_rates > WLAN_USER_MAX_SUPP_RATES) {
            oam_warning_log1(0, OAM_SF_SCAN, "{hmac_scan_update_bss_list_rates::num_rates=%d.}", num_rates);
            num_rates = WLAN_USER_MAX_SUPP_RATES;
        } else if (num_rates == 0) {
            hmac_scan_update_bss_list_rates_log(mac_dev, ie + MAC_IE_HDR_LEN, num_rates, ie[1]);
        }

        if (memcpy_s(bss_dscr->auc_supp_rates, WLAN_USER_MAX_SUPP_RATES, rates, num_rates) != EOK) {
            oam_error_log0(0, OAM_SF_SCAN, "hmac_scan_update_bss_list_rates::memcpy fail!");
            return OAL_FAIL;
        }

        bss_dscr->uc_num_supp_rates = num_rates;
    }

    ie = mac_find_ie(MAC_EID_XRATES, frame_body + offset, frame_len - offset);
    if (ie != NULL) {
        num_ex_rates = hmac_scan_check_bss_supp_rates(mac_dev, ie + MAC_IE_HDR_LEN, ie[1], rates, sizeof(rates));
        if (num_rates + num_ex_rates > WLAN_USER_MAX_SUPP_RATES) { /* 超出支持速率个数 */
            oam_warning_log2(0, OAM_SF_SCAN,
                "{hmac_scan_update_bss_list_rates::number of rates too large, num_rates=%d, num_ex_rates=%d.}",
                num_rates, num_ex_rates);
        }

        if (num_ex_rates > 0) {
            /* support_rates和extended_rates去除重复速率，一并合入扫描结果的速率集中 */
            hmac_scan_rm_repeat_sup_exsup_rates(bss_dscr, rates, num_ex_rates);
        }
    }

    /* 5G频段不支持11b速率 */
    if (bss_dscr->st_channel.en_band == WLAN_BAND_5G) {
        hmac_scan_remove_11b_rate(bss_dscr, &bss_dscr->uc_num_supp_rates);
    }

    return OAL_SUCC;
}

/*
 * 函数名  : hmac_scan_is_hidden_ssid
 * 功能描述  : 根据ssid ap的特点,保留带ssid扫描结果并不更新时间戳
 * 1.日    期  : 2017年6月13日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
*/
oal_bool_enum_uint8 hmac_scan_is_hidden_ssid(uint8_t vap_id, hmac_scanned_bss_info *new_bss,
    hmac_scanned_bss_info *old_bss)
{
    /*  wifi 2015-03-23 begin */
    if ((new_bss->st_bss_dscr_info.ac_ssid[0] == '\0') && (old_bss->st_bss_dscr_info.ac_ssid[0] != '\0')) {
        /*  隐藏SSID,若保存过此AP信息,且ssid不为空,此次通过BEACON帧扫描到此AP信息,且SSID为空,则不进行更新 */
        oam_warning_log3(vap_id, OAM_SF_SCAN,
            "{hmac_scan_is_hidden_ssid::find hide ssid:%02X:XX:XX:XX:%02X:%02X,ignore this update.}",
            new_bss->st_bss_dscr_info.auc_bssid[MAC_ADDR_0], new_bss->st_bss_dscr_info.auc_bssid[MAC_ADDR_4],
            new_bss->st_bss_dscr_info.auc_bssid[MAC_ADDR_5]);
        return OAL_TRUE;
    }
    /*  wifi 2015-03-23 end */
    return OAL_FALSE;
}

/*
 * 函 数 名  : hmac_scan_need_update_old_scan_result
 * 功能描述  : 是否需要更新覆盖之前扫描结果
 * 1.日    期  : 2016年6月2日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
oal_bool_enum_uint8 hmac_scan_need_update_old_scan_result(hmac_vap_stru *hmac_vap, hmac_scanned_bss_info *new_bss,
    hmac_scanned_bss_info *old_bss)
{
    hmac_roam_info_stru *roam_info = NULL;
    mac_scan_req_stru *scan_params = NULL;

    if (hmac_vap == NULL) {
        return OAL_FALSE;
    }

    roam_info = (hmac_roam_info_stru *)hmac_vap->pul_roam_info;
    if (roam_info != NULL) {
        scan_params = &roam_info->st_scan_h2d_params.st_scan_params;
        if (scan_params == NULL) {
            return OAL_FALSE;
        }

        /* 11k 11v 流程需要使用最新扫描结果以选择最佳RSSI AP */
        if (scan_params->uc_neighbor_report_process_flag || scan_params->uc_bss_transition_process_flag) {
            return OAL_TRUE;
        }
    }

    /*  该逻辑仅针对一轮扫描过程中出现beacon、probe rsp时的处理，
       如果之前扫描结果为probe rsp，此次为beacon，则不覆盖，否则覆盖 --start */
    if ((((mac_ieee80211_frame_stru *)old_bss->st_bss_dscr_info.auc_mgmt_buff)->st_frame_control.bit_sub_type ==
        WLAN_PROBE_RSP) &&
        (((mac_ieee80211_frame_stru *)new_bss->st_bss_dscr_info.auc_mgmt_buff)->st_frame_control.bit_sub_type ==
        WLAN_BEACON) &&
        (old_bss->st_bss_dscr_info.en_new_scan_bss == OAL_TRUE)) {
        return OAL_FALSE;
    }
    /*   modify 2016/05/31 --end */
    return OAL_TRUE;
}

/*
 * 功能描述  : 更新bss_dscr的rssi
 */
static void hmac_scan_update_bss_dscr_rssi(mac_bss_dscr_stru *bss_dscr,
    mac_scanned_result_extend_info_stru *scan_result_extend_info)
{
    uint8_t chain_index;

    bss_dscr->c_rssi = (int8_t)scan_result_extend_info->l_rssi;
    for (chain_index = 0; chain_index < HD_EVENT_RF_NUM; chain_index++) {
        bss_dscr->rssi[chain_index] = scan_result_extend_info->rssi[chain_index];
        bss_dscr->snr[chain_index] = scan_result_extend_info->snr[chain_index];
    }
}

OAL_STATIC void hmac_scan_update_bss_dscr_basic_info(mac_device_stru *mac_device, hmac_vap_stru *hmac_vap,
    dmac_rx_ctl_stru *rx_ctrl, uint8_t *frame_body, uint16_t frame_body_len, uint16_t frame_len,
    mac_scanned_result_extend_info_stru *scan_result_extend_info, mac_bss_dscr_stru *bss_dscr)
{
    int32_t ret;
    uint8_t frame_channel;
    uint16_t offset = MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN;

    /* 获取管理帧中的信道 */
    frame_channel = mac_ie_get_chan_num(frame_body, frame_body_len, offset, rx_ctrl);

    /* 解析beacon周期与tim周期 */
    bss_dscr->us_beacon_period = mac_get_beacon_period(frame_body);
    bss_dscr->uc_dtim_period = mac_get_dtim_period(frame_body, frame_body_len);
    bss_dscr->uc_dtim_cnt = mac_get_dtim_cnt(frame_body, frame_body_len);

    /* 信道 */
    bss_dscr->st_channel.uc_chan_number = frame_channel;
    bss_dscr->st_channel.en_band = mac_get_band_by_channel_num(frame_channel);
    bss_dscr->st_channel.ext6g_band = wlan_chip_mac_get_6g_flag(rx_ctrl);
    /* 记录速率集 */
    hmac_scan_update_bss_list_rates(bss_dscr, frame_body, frame_body_len, mac_device);

    /* mp13记录支持的最大空间流 */
#ifdef _PRE_WLAN_FEATURE_M2S
    bss_dscr->en_support_opmode = scan_result_extend_info->en_support_opmode;
    bss_dscr->uc_num_sounding_dim = scan_result_extend_info->uc_num_sounding_dim;
#endif
    bss_dscr->en_support_max_nss = scan_result_extend_info->en_support_max_nss;

    /* 协议类相关信息元素的获取 */
    hmac_scan_update_bss_list_protocol(hmac_vap, bss_dscr, frame_body, frame_body_len);

    /* update st_channel.bandwidth in case hmac_sta_update_join_req_params usage error */
    bss_dscr->st_channel.en_bandwidth = bss_dscr->en_channel_bandwidth;
    ret = (int32_t)mac_get_channel_idx_from_num(bss_dscr->st_channel.en_band,
        bss_dscr->st_channel.uc_chan_number, bss_dscr->st_channel.ext6g_band, &bss_dscr->st_channel.uc_chan_idx);
    if (ret == OAL_ERR_CODE_INVALID_CONFIG) {
        oam_warning_log0(0, OAM_SF_SCAN, "{hmac_scan_update_bss_dscr::mac_get_channel_idx_from_num fail.}");
    }

    /* 更新时间戳 */
    bss_dscr->timestamp = (uint32_t)oal_time_get_stamp_ms();
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    bss_dscr->wpa_rpt_time = oal_get_monotonic_boottime();
#endif
    bss_dscr->mgmt_len = frame_len;
}
OAL_STATIC uint32_t hmac_scan_update_bss_dscr_info(mac_bss_dscr_stru *bss_dscr, mac_ieee80211_frame_stru *frame_header,
    mac_scanned_result_extend_info_stru *scan_result_info, uint8_t *frame_body, uint16_t frame_body_len)
{
    int32_t ret = 0;
    uint8_t *ssid = NULL; /* 指向beacon帧中的ssid */
    uint8_t ssid_len;

    /* 解析并保存ssid */
    ssid = mac_get_ssid(frame_body, (int32_t)frame_body_len, &ssid_len);
    if ((ssid != NULL) && (ssid_len != 0)) {
        /* 将查找到的ssid保存到bss描述结构体中 */
        ret += memcpy_s(bss_dscr->ac_ssid, WLAN_SSID_MAX_LEN, ssid, ssid_len);
        bss_dscr->ac_ssid[ssid_len] = '\0';
    }
    if (ret != EOK) {
        oam_error_log0(0, OAM_SF_SCAN, "hmac_scan_update_bss_dscr::memcpy fail!");
        return OAL_FAIL;
    }

    /* 解析bssid */
    oal_set_mac_addr(bss_dscr->auc_mac_addr, frame_header->auc_address2);
    oal_set_mac_addr(bss_dscr->auc_bssid, frame_header->auc_address3);

    /* bss基本信息 */
    bss_dscr->en_bss_type = scan_result_info->en_bss_type;
    bss_dscr->us_cap_info = *((uint16_t *)(frame_body + MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN));
    return OAL_SUCC;
}
/*
 * 功能描述  : 更新scanned_bss
 */
OAL_STATIC uint32_t hmac_scan_update_bss_dscr(hmac_vap_stru *hmac_vap,
    oal_netbuf_stru *netbuf, uint16_t buf_len, hmac_scanned_bss_info *scanned_bss, uint8_t ext_len)
{
    mac_scanned_result_extend_info_stru *scan_result_info = NULL;
    mac_device_stru *mac_device = mac_res_get_dev(hmac_vap->st_vap_base_info.uc_device_id);
    mac_ieee80211_frame_stru *frame_header = NULL;
    uint8_t *frame_body = NULL;
    mac_bss_dscr_stru *bss_dscr = NULL;
    uint8_t *mgmt_frame = (uint8_t *)oal_netbuf_data(netbuf);
    dmac_rx_ctl_stru *rx_ctrl = (dmac_rx_ctl_stru *)oal_netbuf_cb(netbuf);
    uint16_t frame_len = buf_len - wlan_chip_get_scaned_payload_extend_len();
    uint16_t frame_body_len;
    uint32_t ret;
    uint16_t offset = MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN;

    /* 指向netbuf中的上报的扫描结果的扩展信息的位置 */
    scan_result_info = wlan_chip_get_scaned_result_extend_info(netbuf, frame_len);
    if (oal_unlikely((mac_device == NULL) || (scan_result_info == NULL))) {
        return OAL_FAIL;
    }

    /* 拷贝管理帧内容 */
    if (memcpy_s(scanned_bss->st_bss_dscr_info.auc_mgmt_buff, (uint32_t)frame_len, mgmt_frame, (uint32_t)frame_len)
        != EOK) {
        return OAL_FAIL;
    }
    mgmt_frame = scanned_bss->st_bss_dscr_info.auc_mgmt_buff;

#ifdef _PRE_WLAN_FEATURE_ADAPTIVE11R
    /* 根据互通IE填充akm suite(FT)到RSN IE */
    if (ext_len != DEFAULT_AKM_VALUE) {
        hmac_scan_attach_akm_suite_to_rsn_ie(mgmt_frame, &frame_len, ext_len);
    }
#endif

    /* 获取管理帧的帧头和帧体指针 */
    frame_header = (mac_ieee80211_frame_stru *)mgmt_frame;
    frame_body = mgmt_frame + MAC_80211_FRAME_LEN;
    frame_body_len = frame_len - MAC_80211_FRAME_LEN;

    if (frame_body_len <= offset) {
        oam_error_log1(0, OAM_SF_SCAN, "hmac_scan_update_bss_dscr::frame_body_len err[%d]!", frame_body_len);
        return OAL_FAIL;
    }

    /*****************************************************************************
        解析beacon/probe rsp帧，记录到bss_dscr
    *****************************************************************************/
    /* 更新bss信息 */
    bss_dscr = &(scanned_bss->st_bss_dscr_info);
    ret = hmac_scan_update_bss_dscr_info(bss_dscr, frame_header, scan_result_info, frame_body, frame_body_len);
    if (ret != OAL_SUCC) {
        return ret;
    }
    hmac_scan_update_bss_dscr_rssi(bss_dscr, scan_result_info);
    hmac_scan_update_bss_dscr_basic_info(mac_device, hmac_vap,
        rx_ctrl, frame_body, frame_body_len, frame_len, scan_result_info, bss_dscr);
    return OAL_SUCC;
}

void hmac_scan_update_opmode_and_rssi(hmac_scanned_bss_info *new_scanned_bss, hmac_scanned_bss_info *old_scanned_bss)
{
    uint32_t curr_time_stamp;
#ifdef _PRE_WLAN_FEATURE_M2S
    mac_ieee80211_frame_stru *frame_header = NULL;

    /* 只有probe rsp帧中ext cap 宣称支持OPMODE时，对端才确实支持OPMODE，beacon帧和assoc rsp帧中信息不可信 */
    frame_header = (mac_ieee80211_frame_stru *)new_scanned_bss->st_bss_dscr_info.auc_mgmt_buff;
    if (frame_header->st_frame_control.bit_sub_type == WLAN_PROBE_RSP) {
        old_scanned_bss->st_bss_dscr_info.en_support_opmode = new_scanned_bss->st_bss_dscr_info.en_support_opmode;
    }
#endif

    /* 如果老的扫描的bss的信号强度大于当前扫描到的bss的信号强度，更新当前扫描到的信号强度为最强的信号强度 */
    if (old_scanned_bss->st_bss_dscr_info.c_rssi > new_scanned_bss->st_bss_dscr_info.c_rssi) {
        /* 1s中以内就采用之前的BSS保存的RSSI信息，否则就采用新的RSSI信息 */
        curr_time_stamp = (uint32_t)oal_time_get_stamp_ms();
        //  use oal_time_after32
        if (oal_time_after32((curr_time_stamp),
            (old_scanned_bss->st_bss_dscr_info.timestamp + HMAC_SCAN_MAX_SCANNED_RSSI_EXPIRE)) == FALSE) {
            new_scanned_bss->st_bss_dscr_info.c_rssi = old_scanned_bss->st_bss_dscr_info.c_rssi;
        }
    }
}

void hmac_scan_update_time_and_rssi(oal_netbuf_stru *netbuf,
    hmac_scanned_bss_info *new_scanned_bss, hmac_scanned_bss_info *old_scanned_bss)
{
    old_scanned_bss->st_bss_dscr_info.timestamp = (uint32_t)oal_time_get_stamp_ms();
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    old_scanned_bss->st_bss_dscr_info.wpa_rpt_time = oal_get_monotonic_boottime();
#endif
    old_scanned_bss->st_bss_dscr_info.c_rssi = new_scanned_bss->st_bss_dscr_info.c_rssi;
}

uint16_t hmac_scan_check_adaptive11r_need_akm_suite(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf,
    uint16_t buf_len, uint8_t *ext_len)
{
    uint16_t mgmt_len;

    /* 管理帧的长度等于上报的netbuf的长度减去上报的扫描结果的扩展字段的长度 */
    mgmt_len = buf_len - wlan_chip_get_scaned_payload_extend_len();

#ifdef _PRE_WLAN_FEATURE_ADAPTIVE11R
    /* adaptive 11R模式下检查是否需要在RSN IE中添加akm suite */
    if (hmac_vap->bit_adaptive11r == OAL_TRUE) {
        *ext_len = hmac_scan_extend_mgmt_len_needed(netbuf, buf_len);
        if (*ext_len != DEFAULT_AKM_VALUE) {
            mgmt_len += *ext_len;
        }
    }
#endif

    return mgmt_len;
}

/*
 * 函 数 名  : hmac_scan_check_chan
 * 功能描述  : 检测帧中信道信息是否合理
 * 1.日    期  : 2018年3月1日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint8_t hmac_scan_check_chan(oal_netbuf_stru *netbuf, hmac_scanned_bss_info *scanned_bss)
{
    dmac_rx_ctl_stru *rx_ctrl;
    uint8_t curr_chan, chan_num, is_6ghz;
    uint8_t *frame_body;
    uint16_t frame_body_len;
    uint16_t offset = MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN;
    uint8_t *ie_start_addr;

    rx_ctrl = (dmac_rx_ctl_stru *)oal_netbuf_cb(netbuf);
    curr_chan = rx_ctrl->st_rx_info.uc_channel_number;
    is_6ghz = wlan_chip_mac_get_6g_flag(rx_ctrl);
    frame_body = scanned_bss->st_bss_dscr_info.auc_mgmt_buff + MAC_80211_FRAME_LEN;
    frame_body_len = scanned_bss->st_bss_dscr_info.mgmt_len - MAC_80211_FRAME_LEN;

    /* 在DSSS Param set ie中解析chan num */
    ie_start_addr = mac_find_ie(MAC_EID_DSPARMS, frame_body + offset, frame_body_len - offset);
    if ((ie_start_addr != NULL) && (ie_start_addr[1] == MAC_DSPARMS_LEN)) {
        chan_num = ie_start_addr[BYTE_OFFSET_2];
        if (mac_is_channel_num_valid(mac_get_band_by_channel_num(chan_num), chan_num, is_6ghz) != OAL_SUCC) {
            return OAL_FALSE;
        }
    }

    /* 在HT operation ie中解析 chan num */
    ie_start_addr = mac_find_ie(MAC_EID_HT_OPERATION, frame_body + offset, frame_body_len - offset);
    if ((ie_start_addr != NULL) && (ie_start_addr[1] >= 1)) {
        chan_num = ie_start_addr[BYTE_OFFSET_2];
        if (mac_is_channel_num_valid(mac_get_band_by_channel_num(chan_num), chan_num, is_6ghz) != OAL_SUCC) {
            return OAL_FALSE;
        }
    }

    chan_num = scanned_bss->st_bss_dscr_info.st_channel.uc_chan_number;
    if (((curr_chan > chan_num) && (curr_chan - chan_num >= 3)) || // 当前信道相比扫描信道偏移3个及以上
        ((curr_chan < chan_num) && (chan_num - curr_chan >= 3))) { // 扫描信道相比当前信道偏移3个及以上
        return OAL_FALSE;
    }

    return OAL_TRUE;
}

void hmac_scan_save_mbss_info(hmac_vap_stru *hmac_vap,
    mac_multi_bssid_frame_info_stru *mbss_frame_info, hmac_scanned_bss_info *new_scanned_bss)
{
#ifdef _PRE_WLAN_FEATURE_11AX
    if (g_wlan_spec_cfg->feature_11ax_is_open) {
        /* 保存mbss info */
        if (is_custom_open_multi_bssid_switch(&hmac_vap->st_vap_base_info)) {
            memcpy_s(new_scanned_bss->st_bss_dscr_info.st_mbssid_info.auc_transmitted_bssid, WLAN_MAC_ADDR_LEN,
                     mbss_frame_info->auc_transmitted_bssid, WLAN_MAC_ADDR_LEN);
            new_scanned_bss->st_bss_dscr_info.st_mbssid_info.bit_is_non_transimitted_bss =
                mbss_frame_info->bit_is_non_transimitted_bss;
        }
    }
#endif
}

/*
 * 函 数 名  : hmac_scan_proc_scan_result_handle
 * 功能描述  : 处理一帧扫描数据
 * 1.日    期  : 2019年4月3日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_scan_proc_scan_result_handle(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf,
                                           uint16_t buf_len, mac_multi_bssid_frame_info_stru *mbss_frame_info)
{
    uint32_t ret;
    uint8_t vap_id;
    uint16_t mgmt_len;
    uint8_t ext_len = DEFAULT_AKM_VALUE;
    hmac_device_stru *hmac_device = NULL;
    hmac_scanned_bss_info *new_scanned_bss = NULL;
    hmac_scanned_bss_info *old_scanned_bss = NULL;
    hmac_bss_mgmt_stru *bss_mgmt = NULL;

    /* 获取vap id */
    vap_id = hmac_vap->st_vap_base_info.uc_vap_id;

    /* 获取hmac device 结构 */
    hmac_device = hmac_res_get_mac_dev(hmac_vap->st_vap_base_info.uc_device_id);
    if (oal_unlikely(hmac_device == NULL)) {
        oam_error_log0(vap_id, OAM_SF_SCAN, "{hmac_scan_proc_scan_result_handle::hmac_device null.}");

        /* 释放上报的bss信息和beacon或者probe rsp帧的内存 */
        oal_netbuf_free(netbuf);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /***********************************************************************************************/
    /* 对dmac上报的netbuf内容进行解析，netbuf data域的上报的扫描结果的字段的分布                       */
    /* ------------------------------------------------------------------------------------------ */
    /* beacon/probe rsp body  |     帧体后面附加字段(mac_scanned_result_extend_info_stru)          */
    /* ----------------------------------------------------------------------------------------- */
    /* 收到的beacon/rsp的body | rssi(4字节) | channel num(1字节)| band(1字节)|bss_tye(1字节)|填充  */
    /* -----------------------------------------------------------------------------------------*/
    /********************************************************************************************/
    mgmt_len = hmac_scan_check_adaptive11r_need_akm_suite(hmac_vap, netbuf, buf_len, &ext_len);
#ifdef _PRE_WLAN_FEATURE_PWL
    /* pwl模式下补充ssid ie tlv长度 */
    mgmt_len += (hmac_vap->st_vap_base_info.pwl_enable == OAL_TRUE) ? ((NUM_2_BYTES) + (WLAN_SSID_MAX_LEN)) : 0;
#endif
    /* 申请存储扫描结果的内存 */
    new_scanned_bss = hmac_scan_alloc_scanned_bss(mgmt_len);
    if (oal_unlikely(new_scanned_bss == NULL)) {
        oam_error_log0(vap_id, OAM_SF_SCAN, "{hmac_scan_proc_scan_result_handle::alloc memory failed}");

        /* 释放上报的bss信息和beacon或者probe rsp帧的内存 */
        oal_netbuf_free(netbuf);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 更新描述扫描结果的bss dscr结构体 */
    ret = hmac_scan_update_bss_dscr(hmac_vap, netbuf, buf_len, new_scanned_bss, ext_len);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_error_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
                       "{hmac_scan_proc_scan_result_handle::hmac_scan_update_bss_dscr failed[%d].}", ret);

        /* 释放上报的bss信息和beacon或者probe rsp帧的内存 */
        oal_netbuf_free(netbuf);

        /* 释放申请的存储bss信息的内存 */
        oal_free(new_scanned_bss);

        return ret;
    }

    hmac_scan_save_mbss_info(hmac_vap, mbss_frame_info, new_scanned_bss);

    /* 获取管理扫描的bss结果的结构体 */
    bss_mgmt = &(hmac_device->st_scan_mgmt.st_scan_record_mgmt.st_bss_mgmt);
    /* 对链表删操作前加锁 */
    oal_spin_lock(&(bss_mgmt->st_lock));
    /* 判断相同bssid的bss是否已经扫描到 */
    old_scanned_bss = hmac_scan_find_scanned_bss_by_bssid(bss_mgmt, new_scanned_bss->st_bss_dscr_info.auc_bssid,
                                                          WLAN_MAC_ADDR_LEN);
    if (old_scanned_bss == NULL) {
        /* 解锁 */
        oal_spin_unlock(&(bss_mgmt->st_lock));

        goto add_bss;
    }

    hmac_scan_update_opmode_and_rssi(new_scanned_bss, old_scanned_bss);

    if (hmac_scan_is_hidden_ssid(vap_id, new_scanned_bss, old_scanned_bss) == OAL_TRUE) {
        /* 解锁 */
        oal_spin_unlock(&(bss_mgmt->st_lock));

        /* 释放申请的存储bss信息的内存 */
        oal_free(new_scanned_bss);

        /* 释放上报的bss信息和beacon或者probe rsp帧的内存 */
        oal_netbuf_free(netbuf);

        return OAL_SUCC;
    }

    if (hmac_scan_need_update_old_scan_result(hmac_vap, new_scanned_bss, old_scanned_bss) == OAL_FALSE
        || hmac_scan_check_chan(netbuf, new_scanned_bss) == OAL_FALSE) {
        hmac_scan_update_time_and_rssi(netbuf, new_scanned_bss, old_scanned_bss);

        /* 解锁 */
        oal_spin_unlock(&(bss_mgmt->st_lock));

        /* 释放申请的存储bss信息的内存 */
        oal_free(new_scanned_bss);

        /* 释放上报的bss信息和beacon或者probe rsp帧的内存 */
        oal_netbuf_free(netbuf);
        return OAL_SUCC;
    }

    /* 从链表中将原先扫描到的相同bssid的bss节点删除 */
    hmac_scan_del_bss_from_list_nolock(old_scanned_bss, hmac_device);
    /* 解锁 */
    oal_spin_unlock(&(bss_mgmt->st_lock));
    /* 释放内存 */
    oal_free(old_scanned_bss);
add_bss:

    /* 将扫描结果添加到链表中 */
    hmac_scan_add_bss_to_list(new_scanned_bss, hmac_device);
    /* 释放上报的bss信息和beacon或者probe rsp帧的内存 */
    oal_netbuf_free(netbuf);

    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_scan_proc_scanned_bss
 * 功能描述  : 接收每个信道的扫描结果到host侧进行处理
 * 1.日    期  : 2015年2月7日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_scan_proc_scanned_bss(frw_event_mem_stru *event_mem)
{
    frw_event_stru *event = NULL;
    frw_event_hdr_stru *event_hdr = NULL;
    hmac_vap_stru *hmac_vap = NULL;
    oal_netbuf_stru *bss_mgmt_netbuf = NULL;
    dmac_tx_event_stru *dtx_event = NULL;
    mac_multi_bssid_frame_info_stru mbss_frame_info;
    uint32_t ret;

    if (oal_unlikely(event_mem == NULL)) {
        oam_error_log0(0, OAM_SF_SCAN, "{hmac_scan_proc_scanned_bss::event_mem null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取事件头和事件结构体指针 */
    event = frw_get_event_stru(event_mem);
    event_hdr = &(event->st_event_hdr);
    dtx_event = (dmac_tx_event_stru *)event->auc_event_data;
    bss_mgmt_netbuf = dtx_event->pst_netbuf;

    hmac_vap = mac_res_get_hmac_vap(event_hdr->uc_vap_id);
    if (oal_unlikely(hmac_vap == NULL)) {
        oam_error_log0(0, OAM_SF_SCAN, "{hmac_scan_proc_scanned_bss::hmac_vap null.}");

        /* 释放上报的bss信息和beacon或者probe rsp帧的内存 */
        oal_netbuf_free(bss_mgmt_netbuf);
        return OAL_ERR_CODE_PTR_NULL;
    }

#ifdef _PRE_WLAN_FEATURE_11AX
    if (g_wlan_spec_cfg->feature_11ax_is_open) {
        if (is_custom_open_multi_bssid_switch(&hmac_vap->st_vap_base_info)) {
            hmac_scan_proc_multi_bssid_scanned_bss(hmac_vap, bss_mgmt_netbuf, dtx_event->us_frame_len);
        }
    }
#endif

    /* 处理发送Beacon 或Probe Rsp的bss */
    memset_s((uint8_t *)&mbss_frame_info, sizeof(mbss_frame_info), 0, sizeof(mbss_frame_info));
    ret = hmac_scan_proc_scan_result_handle(hmac_vap, bss_mgmt_netbuf, dtx_event->us_frame_len, &mbss_frame_info);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_SCAN, "{hmac_scan_proc_scanned_bss::hmac_scan_proc_scan_result_handle failed.}");
    }

    return OAL_SUCC;
}
