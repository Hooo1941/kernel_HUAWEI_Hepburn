/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: als para table sip1225 source file
 * Author: lisheng
 * Create: 2024-01-19
 */

#ifndef __ALS_PARA_TABLE_SIP1225_H__
#define __ALS_PARA_TABLE_SIP1225_H__

#include "als_para_table.h"

als_para_normal_table *als_get_sip1225_table_by_id(uint32_t id);
als_para_normal_table *als_get_sip1225_first_table(void);
uint32_t als_get_sip1225_table_count(void);

#endif /* __ALS_PARA_TABLE_SIP1225_H__ */
