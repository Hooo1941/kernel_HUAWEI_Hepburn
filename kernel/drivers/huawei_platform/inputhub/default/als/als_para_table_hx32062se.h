/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: als para table hx32062se header file
 * Author: huangjiaqing <huangjiaqing4@huawei.com>
 * Create: 2023-06-12
 */

#ifndef __ALS_PARA_TABLE_HX32062SE_H__
#define __ALS_PARA_TABLE_HX32062SE_H__

#include "als_para_table.h"

hx32062se_als_para_table *als_get_hx32062se_table_by_id(uint32_t id);
hx32062se_als_para_table *als_get_hx32062se_first_table(void);
uint32_t als_get_hx32062se_table_count(void);

#endif /* __ALS_PARA_TABLE_HX32062SE_H__ */
