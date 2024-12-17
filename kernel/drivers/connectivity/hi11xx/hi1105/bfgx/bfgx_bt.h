/**
 * Copyright (c) @CompanyNameMagicTag 2022-2023. All rights reserved.
 *
 * Description: bfgx_bt.c header file
 * Author: @CompanyNameTag
 */

#ifndef BFGX_BT_H
#define BFGX_BT_H
#define BT_IOCTL_FAKE_CLOSE_CMD     100
#define BT_IOCTL_HCISETPROTO        101
#define BT_IOCTL_HCIUNSETPROTO      102
#define BT_IOCTL_OPEN               103
#define BT_IOCTL_RELEASE            104

#define BT_FAKE_CLOSE               1
#define BT_REAL_CLOSE               0

struct bfgx_subsys_driver_desc *get_bt_default_drv_desc(void);
#endif // _BFGX_BT_H_
