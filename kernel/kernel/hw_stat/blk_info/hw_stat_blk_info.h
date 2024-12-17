/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description: get blk info
 * Author: xianghengliang
 * Create: 2023-04-02
 */
#ifndef __HW_STAT_BLK_INFO_H
#define __HW_STAT_BLK_INFO_H

#include <linux/hw_stat.h>
#include <linux/seq_file.h>

void hw_stat_init_blk_info(struct proc_dir_entry *dir);
#endif
