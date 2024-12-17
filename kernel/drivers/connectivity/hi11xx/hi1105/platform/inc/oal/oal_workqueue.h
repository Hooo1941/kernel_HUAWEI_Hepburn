/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description: oal_workqueue.c header file
 * Author: @CompanyNameTag
 */

#ifndef OAL_WORKQUEUE_H
#define OAL_WORKQUEUE_H

/* 其他头文件包含 */
#include "oal_types.h"
#include "oal_hardware.h"
#include "arch/oal_workqueue.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_OAL_WORKQUEUE_H

/* 函数声明 */
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
extern int32_t oal_workqueue_schedule(oal_work_stru *pst_work);
extern int32_t oal_workqueue_delay_schedule(oal_delayed_work *pst_work, unsigned long delay);
extern uint32_t oal_workqueue_init(void);
extern uint32_t oal_workqueue_exit(void);
#endif

#endif /* end of oal_workqueue.h */
