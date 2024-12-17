#ifndef __MAS_BLK_GEAR_CTRL_SYSFS_H__
#define __MAS_BLK_GEAR_CTRL_SYSFS_H__

#include <linux/kernel.h>
#include <linux/types.h>
#include "blk.h"

ssize_t mas_gear_ctrl_store(struct request_queue *q, const char *page, size_t count);
ssize_t mas_gear_ctrl_show(struct request_queue *q, char *page);
ssize_t mas_gear_auto_enable_store(struct request_queue *q, const char *page, size_t count);
ssize_t mas_gear_auto_enable_show(struct request_queue *q, char *page);
ssize_t mas_gear_ufs_rate_store(struct request_queue *q, const char *page, size_t count);
ssize_t mas_gear_ufs_rate_show(struct request_queue *q, char *page);
ssize_t mas_gear_fg_num_store(struct request_queue *q, const char *page, size_t count);
ssize_t mas_gear_fg_num_show(struct request_queue *q, char *page);
ssize_t mas_gear_flow_cnt_per_ms_store(struct request_queue *q, const char *page, size_t count);
ssize_t mas_gear_flow_cnt_per_ms_show(struct request_queue *q, char *page);
ssize_t mas_gear_flow_threshold_size_per_100us_store(struct request_queue *q, const char *page, size_t count);
ssize_t mas_gear_flow_threshold_size_per_100us_show(struct request_queue *q, char *page);
ssize_t mas_gear_flow_threshold_inflat_size_store(struct request_queue *q, const char *page, size_t count);
ssize_t mas_gear_flow_threshold_inflat_size_show(struct request_queue *q, char *page);
ssize_t mas_gear_sync_num_store(struct request_queue *q, const char *page, size_t count);
ssize_t mas_gear_sync_num_show(struct request_queue *q, char *page);
ssize_t mas_min_gear_stay_time_store(struct request_queue *q, const char *page, size_t count);
ssize_t mas_min_gear_stay_time_show(struct request_queue *q, char *page);
#endif
