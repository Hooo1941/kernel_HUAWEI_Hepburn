/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 :
 * 创建日期 : 2020年6月17日
 */

#ifndef WAL_CFG_IOCTL_H
#define WAL_CFG_IOCTL_H
#include "mac_vap.h"
#include "oal_ext_if.h"
#ifdef _PRE_WLAN_FEATURE_DFR
#include "wal_dfx.h"
#include "hmac_dfx.h"
#endif
#include "hmac_blacklist.h"
#include "hmac_roam_main.h"
#ifdef _PRE_WLAN_FEATURE_WMMAC
#include "hmac_wmmac.h"
#endif
#include "wlan_chip_i.h"

#define CMD_NAME_MAX_LEN        80 /* 字符串中每个单词的最大长度(原20) */
#define CMD_VALUE_MAX_LEN       10 /* 字符串中某个对应变量取值的最大位数 */
#define CMD_PARAMS_MAX_CNT      20 /* 命令最大参数个数 */
#define MIN_VALUE_INDEX         0 /* 参数检查最小值的index */
#define MAX_VALUE_INDEX         1 /* 参数检查最大值的index */
#define PARAM_CHECK_TABLE_LEN   2 /* 每个参数检查表有两个值最大值和最小值 */
#define MIN_INT32               (-2147483648U)
#define MAX_INT32               2147483647

typedef uint32_t (*package_params_func)(
    mac_vap_stru *mac_vap, uint32_t *params, uint8_t *new_params, uint16_t *new_params_len);
typedef uint32_t (*process_cmd_func)(mac_vap_stru *mac_vap, uint32_t *params);

typedef struct {
    uint8_t *name;                      /* 命令关键字 */
    uint8_t param_num;                  /* 命令参数个数 */
    const int32_t *param_check_table;   /* 参数有效性检查表 */
    process_cmd_func process_cmd;       /* host处理命令函数指针 */
    package_params_func package_params; /* 参数组装函数 */
    oal_bool_enum_uint8 to_device;      /* 是否下发到device */
    uint32_t cfg_id;                    /* 抛事件的ID号 */
} host_cmd_stru;

uint32_t wal_process_cmd(int8_t *cmd);
uint32_t wal_process_cmd_params(oal_net_device_stru *netdev, uint32_t cmd_id, int32_t *params);
uint32_t wal_get_cmd_id(int8_t *cmd, uint32_t *cmd_id, uint32_t *offset);
#endif

