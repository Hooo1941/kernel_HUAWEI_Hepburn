/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: als para table ams tmd3702 header file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#ifndef __ALS_PARA_TABLE_SIP3642_H__
#define __ALS_PARA_TABLE_SIP3642_H__

#include "als_para_table.h"

sip3642_als_para_table *als_get_sip3642_table_by_id(uint32_t id);
sip3642_als_para_table *als_get_sip3642_first_table(void);
uint32_t als_get_sip3642_table_count(void);

#endif
