/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: als para table ams tmd3702 header file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#ifndef __ALS_PARA_TABLE_SYN3133_H__
#define __ALS_PARA_TABLE_SYN3133_H__

#include "als_para_table.h"

syn3133_als_para_table *als_get_syn3133_table_by_id(uint32_t id);
syn3133_als_para_table *als_get_syn3133_first_table(void);
uint32_t als_get_syn3133_table_count(void);

#endif /* __ALS_PARA_TABLE_SYN3133_H__ */
