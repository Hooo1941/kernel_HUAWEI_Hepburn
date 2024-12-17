/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description: factory_mem.c header file
 * Author: @CompanyNameTag
 */

#ifndef FACTORY_MEM_H
#define FACTORY_MEM_H
#include "oal_util.h"
#include "oal_workqueue.h"

int32_t memcheck_is_working(void);
int32_t device_mem_check(unsigned long long *time);
int32_t wlan_device_mem_check(int32_t l_runing_test_mode);
int32_t wlan_device_mem_check_result(unsigned long long *time);
void wlan_device_mem_check_work(oal_work_stru *pst_worker);
int32_t device_gt_mem_check(void);
int32_t gt_device_mem_check(unsigned long long *time);
int32_t gt_device_mem_check_result(void);
#endif
