/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: gear ctrl implement
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#ifndef __MAS_BLK_GEAR_CTRL_H__
#define __MAS_BLK_GEAR_CTRL_H__

#include <linux/bio.h>
#include <linux/blk-mq.h>
#include <linux/blkdev.h>
#include <linux/types.h>
#include "blk.h"

#define MAS_GEAR_INVALID_MAIGC_NUM 0xA5A5A5A5
/***************************************************************************************************
***********************************    for ops check&&get     **************************************
***************************************************************************************************/
bool ufs_dss_check_request_queue_ok(struct request_queue *q);
struct lld_gear_ctrl_ops *ufs_dss_get_ops(struct request_queue *q);
/***************************************************************************************************
***********************************    for sysfs    ************************************************
***************************************************************************************************/
extern u32 g_mas_gear_ctrl_flow_threshold_size_per_100us;
extern u32 g_mas_gear_ctrl_inflat_threshold_size;
extern u32 g_mas_gear_ctrl_flow_threshold_cnt_per_ms;
extern bool g_mas_gear_auto_flag;
extern bool g_mas_gear_ctrl_device_abandon;
extern u32 g_mas_min_highest_gear_stay_time;
extern u32 g_mas_ufs_rate;
extern u32 g_mas_fg_num;
extern u32 g_mas_sync_num;
extern u32 g_mas_min_gear_level;
extern u32 g_mas_max_gear_level;

void mas_queue_gear_ctrl_change(struct request_queue *q, u32 set_gear, int cond);

/***************************************************************************************************
******************************    flow control interface    ****************************************
***************************************************************************************************/
void mas_gear_ctrl_flow_init(void);
void mas_gear_ctrl_flow_get(int *flow_cnt , int *flow_size, int *spend_us, int *inflat_size);
void mas_gear_ctrl_flow_add(struct request *req);
void mas_gear_ctrl_flow_destory(void);
void mas_gear_ctrl_flow_delete_wait(struct request *req);
void mas_gear_ctrl_flow_drop(void);
void mas_blk_gear_ctrl_reinit(void);
void mas_gear_ctrl_flow_config(u32 size_per_100us, u32 cnt_per_ms);

void mas_queue_busy_change_gear(struct request_queue *q, struct request *req);
void mas_blk_disable_gear_ctrl(bool enable);
bool mas_blk_check_gear_ctrl_abandon_on_device(void);
void mas_blk_auto_gear_ctrl(bool enable);
void mas_queue_ioback_change_gear(struct request_queue *q, struct request *req);

/***************************************************************************************************
******************************    gear ctrl reason    **********************************************
***************************************************************************************************/

enum {
	GEAR_CTRL_REASON_LRB = 1,
	GEAR_CTRL_REASON_FG_SYNC,
	GEAR_CTRL_REASON_FLOW_SIZE,
	GEAR_CTRL_REASON_FLOW_CNT,
	GEAR_CTRL_REASON_INFLAT_SIZE,
	GEAR_CTRL_REASON_MAX
};

#endif
