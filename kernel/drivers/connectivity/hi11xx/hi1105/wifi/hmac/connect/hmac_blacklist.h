/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : hmac_blacklist.c 的头文件
 * 作    者 :
 * 创建日期 : 2016年5月17日
 * 修改历史 :
 * 1.日    期  : 2014年3月26日
 *   作    者  : wifi
 *   修改内容  : 创建文件
 * 2.日    期  : 2016年5月17日
 *   作    者  :
 *   修改内容  : 从hmac_custom_security.h中分离
 */

#ifndef HMAC_BLAKLIST_H
#define HMAC_BLAKLIST_H

#include "oal_ext_if.h"
#include "mac_vap.h"

#define CS_INVALID_AGING_TIME 0
#define CS_DEFAULT_AGING_TIME 3600
#define CS_DEFAULT_RESET_TIME 3600
#define CS_DEFAULT_THRESHOLD  100

/* 黑名单类型 */
typedef enum {
    CS_BLACKLIST_TYPE_ADD, /* 增加       */
    CS_BLACKLIST_TYPE_DEL, /* 删除       */
    CS_BLACKLIST_TYPE_BUTT
} cs_blacklist_type_enum;

/* 自动黑名单配置参数 */
typedef struct {
    uint8_t uc_enabled;      /* 使能标志 0:未使能  1:使能 */
    uint8_t auc_reserved[BYTE_OFFSET_3]; /* 字节对齐                  */
    uint32_t threshold;   /* 门限                      */
    uint32_t reset_time;  /* 重置时间                  */
    uint32_t aging_time;  /* 老化时间                  */
} hmac_autoblacklist_cfg_stru;

/* 黑白名单配置 */
typedef struct {
    uint8_t uc_type;   /* 配置类型    */
    uint8_t uc_mode;   /* 配置模式    */
    uint8_t auc_sa[WLAN_MAC_ADDR_LEN]; /* mac地址     */
} hmac_blacklist_cfg_stru;

/* 黑名单 */
typedef struct {
    uint8_t auc_mac_addr[OAL_MAC_ADDR_LEN]; /* mac地址          */
    uint8_t auc_reserved[NUM_2_BYTES];                /* 字节对齐         */
    uint32_t cfg_time;                   /* 加入黑名单的时间 */
    uint32_t aging_time;                 /* 老化时间         */
    uint32_t drop_counter;               /* 报文丢弃统计     */
} mac_blacklist_stru;

/* 自动黑名单 */
typedef struct {
    uint8_t auc_mac_addr[OAL_MAC_ADDR_LEN]; /* mac地址  */
    uint8_t auc_reserved[NUM_2_BYTES];                /* 字节对齐 */
    uint32_t cfg_time;                   /* 初始时间 */
    uint32_t asso_counter;               /* 关联计数 */
} mac_autoblacklist_stru;

/* 2.9.12 私有安全增强 */
#define WLAN_BLACKLIST_MAX 32

/* 自动黑名单信息 */
typedef struct {
    uint8_t uc_enabled;                                          /* 使能标志 0:未使能  1:使能 */
    uint8_t list_num;                                            /* 有多少个自动黑名单        */
    uint8_t auc_reserved[NUM_2_BYTES];                                     /* 字节对齐                  */
    uint32_t threshold;                                       /* 门限                      */
    uint32_t reset_time;                                      /* 重置时间                  */
    uint32_t aging_time;                                      /* 老化时间                  */
    mac_autoblacklist_stru ast_autoblack_list[WLAN_BLACKLIST_MAX]; /* 自动黑名单表              */
} mac_autoblacklist_info_stru;

/* 黑白名单信息 */
typedef struct {
    uint8_t uc_mode;                                     /* 黑白名单模式   */
    uint8_t uc_list_num;                                 /* 名单数         */
    uint8_t uc_blacklist_vap_index;                      /* 黑名单vap index */
    uint8_t uc_blacklist_device_index;                   /* 黑名单device index */
    mac_autoblacklist_info_stru st_autoblacklist_info;     /* 自动黑名单信息 */
    mac_blacklist_stru ast_black_list[WLAN_BLACKLIST_MAX]; /* 有效黑白名单表 */
} mac_blacklist_info_stru;

void mac_blacklist_free_pointer(mac_vap_stru *pst_mac_vap, mac_blacklist_info_stru *pst_blacklist_info);
void mac_blacklist_get_pointer(wlan_vap_mode_enum_uint8 en_vap_mode, uint8_t uc_dev_id,
    uint8_t uc_chip_id, uint8_t uc_vap_id, mac_blacklist_info_stru **pst_blacklist_info);
uint32_t hmac_autoblacklist_enable(mac_vap_stru *mac_vap, uint8_t enabled);
uint32_t hmac_autoblacklist_set_aging(mac_vap_stru *mac_vap, uint32_t aging_time);
uint32_t hmac_autoblacklist_set_threshold(mac_vap_stru *mac_vap, uint32_t threshold);
uint32_t hmac_autoblacklist_set_reset_time(mac_vap_stru *mac_vap, uint32_t reset_time);
void hmac_autoblacklist_filter(mac_vap_stru *mac_vap, uint8_t *mac_addr);
uint32_t hmac_blacklist_set_mode(mac_vap_stru *mac_vap, uint8_t mode);
uint32_t hmac_blacklist_get_mode(mac_vap_stru *mac_vap, uint8_t *mode);
uint32_t hmac_blacklist_add(mac_vap_stru *mac_vap, uint8_t *mac_addr, uint32_t aging_time);
uint32_t hmac_blacklist_add_only(mac_vap_stru *mac_vap, uint8_t *mac_addr, uint32_t aging_time);
uint32_t hmac_blacklist_del(mac_vap_stru *mac_vap, uint8_t *mac_addr);
oal_bool_enum_uint8 hmac_blacklist_filter(mac_vap_stru *mac_vap, uint8_t *mac_addr, size_t maclen);
void hmac_show_blacklist_info(mac_vap_stru *mac_vap);
uint32_t hmac_backlist_get_drop(mac_vap_stru *mac_vap, const uint8_t *mac_addr);
uint8_t hmac_backlist_get_list_num(mac_vap_stru *mac_vap);
oal_bool_enum_uint8 hmac_blacklist_get_assoc_ap(mac_vap_stru *cur_vap, uint8_t *mac_addr,
    mac_vap_stru **assoc_vap);

#endif /* end of hmac_blacklist.h */
