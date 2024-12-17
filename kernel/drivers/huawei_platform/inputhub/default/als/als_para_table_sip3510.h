/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: als para table sip3510 header file
 * Author: huangjiaqing <huangjiaqing4@huawei.com>
 * Create: 2023-06-12
 */

#ifndef __ALS_PARA_TABLE_SIP3510_H__
#define __ALS_PARA_TABLE_SIP3510_H__

#include "als_para_table.h"

sip3510_als_para_table *als_get_sip3510_table_by_id(uint32_t id);
sip3510_als_para_table *als_get_sip3510_first_table(void);
uint32_t als_get_sip3510_table_count(void);

#endif /* __ALS_PARA_TABLE_SIP3510_H__ */
