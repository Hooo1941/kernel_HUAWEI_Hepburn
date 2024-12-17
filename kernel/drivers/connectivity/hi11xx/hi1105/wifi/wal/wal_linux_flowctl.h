/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : wal_linux_flowctl.c 的头文件
 * 作    者 :
 * 创建日期 : 2014年3月3日
 */

#ifndef WAL_LINUX_FLOWCTL_H
#define WAL_LINUX_FLOWCTL_H

/* 1 其他头文件包含 */
#include "oal_ext_if.h"
#include "frw_ext_if.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_WAL_LINUX_FLOWCTL_H

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 1, 0))
uint16_t wal_netdev_select_queue(oal_net_device_stru *pst_dev, oal_netbuf_stru *pst_buf);
#elif(LINUX_VERSION_CODE < KERNEL_VERSION(4, 18, 0))
uint16_t wal_netdev_select_queue(oal_net_device_stru *pst_dev, oal_netbuf_stru *pst_buf,
                                 void *accel_priv, select_queue_fallback_t fallback);
#elif(LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
uint16_t wal_netdev_select_queue(oal_net_device_stru *pst_dev, oal_netbuf_stru *pst_buf,
                                 oal_net_device_stru *pst_buf_dev);
#else
uint16_t wal_netdev_select_queue(oal_net_device_stru *pst_dev, oal_netbuf_stru *pst_buf,
                                 oal_net_device_stru *pst_buf_dev, select_queue_fallback_t fallback);
#endif /* (LINUX_VERSION_CODE < KERNEL_VERSION(4,1,0)) */

#endif /* end of wal_linux_flowctl.h */

