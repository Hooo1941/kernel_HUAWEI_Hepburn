/**
 * Copyright (c) @CompanyNameMagicTag 2022-2023. All rights reserved.
 *
 * Description: bfgx_core.c header file
 * Author: @CompanyNameTag
 */

#ifndef BFGX_CORE_H
#define BFGX_CORE_H
#include <linux/fs.h>
#include <linux/list.h>
#include <linux/miscdevice.h>
#include "hw_bfg_ps.h"

#define HW_BT_DEV_NAME        "hwbt"
#define HW_BT1_DEV_NAME       "hwbt1"
#define HW_FM_DEV_NAME        "hwfm"
#define HW_IR_DEV_NAME        "hwir"
#define HW_GNSS_DEV_NAME      "hwgnss"
#define HW_GNSS_ME_DEV_NAME   "hwgnss_me"
#define HW_SLE_DEV_NAME       "hwsle"
#define HW_BFGDBG_DEV_NAME    "hwbfgdbg"
#define HW_BFGDBG_DEV_NAME0   "hwmeoam"
#define HW_EXCP_BFGX_DEV_NAME "hwbfgexcp"
#define HW_EXCP_WIFI_DEV_NAME "hwwifiexcp"
#define HW_EXCP_ME_DEV_NAME   "hwmeexcp"
#define HW_EXCP_GT_DEV_NAME   "hwgtexcp"

#define BFGX_NOT_RECV_BEAT_INFO   0     /* host没有收到心跳消息 */
#define BFGX_RECV_BEAT_INFO       1     /* host收到心跳消息 */

// 节点驱动层数据结构
struct bfgx_subsys_driver_desc {
    const char *driver_name;
    const struct file_operations *file_ops;
    const struct attribute_group **groups;
};

// 节点设备层数据结构
struct bfgx_subsys_dev_desc {
    const char *name;
    struct bfgx_subsys_driver_desc *(*get_driver_desc)(void);
    uint32_t uart;
    void *private_data;
};

// 每个设备节点描述符
struct bfgx_dev_node {
    struct miscdevice c;
    struct list_head node;
    struct ps_core_s *core;
    atomic_t open_cnt;
    void *dev_data;
};

OAL_STATIC OAL_INLINE const char *get_tty_name(struct ps_plat_s *ps_plat_d)
{
    char *path = "/dev/";
    const char *tty_dev_name = NULL;
    tty_dev_name = ps_plat_d->uart_name + strlen(path);
    return tty_dev_name;
}
struct ps_core_s *get_ps_core_from_miscdev(struct miscdevice *dev);
struct ps_core_s *bfgx_get_core_by_dev_name(const char *dev_name);
struct ps_core_s *ps_get_core_by_uart_name(const char *uart_name);
int32_t hw_ps_init(void);
void hw_ps_exit(void);

enum heart_beat_swith {
    TIMER_DELETE,
    TIMER_RESET
};
void plat_beat_timeout_reset_set(unsigned long arg);
int32_t update_heart_beat_timer(struct ps_core_s *ps_core_d, enum heart_beat_swith on);
void ps_core_show_bfgx_status(void);
bool ps_core_chk_bfgx_active(struct ps_core_s *ps_core_d);
oal_bool_enum ps_core_single_subsys_active(uint8_t subsys);
oal_bool_enum ps_core_ir_only_mode(void);
#endif // _BFGX_CORE_H_