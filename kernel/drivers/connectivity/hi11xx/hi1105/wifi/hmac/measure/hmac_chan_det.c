/*
* Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
* 功能说明   : 信道测量上报功能
* 创建日期   : 2021年11月9日
*/

/* 1 头文件包含 */
#include "hmac_chan_det.h"
#include "oal_kernel_file.h"
#include "hd_event.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_CHAN_DET_C

/* 2 全局变量定义 */
uint32_t hmac_chan_det_rpt_event_process(frw_event_mem_stru *event_mem)
{
    return OAL_SUCC;
}
