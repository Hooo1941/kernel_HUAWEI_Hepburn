

#ifndef __ALS_PARA_TABLE_TCS3410_H__
#define __ALS_PARA_TABLE_TCS3410_H__

#include "als_para_table.h"

tcs3410_als_para_table *als_get_tcs3410_table_by_id(uint32_t id);
tcs3410_als_para_table *als_get_tcs3410_first_table(void);
uint32_t als_get_tcs3410_table_count(void);

tcs3410_als_para_table *als1_get_tcs3410_table_by_id(uint32_t id);
tcs3410_als_para_table *als1_get_tcs3410_first_table(void);
uint32_t als1_get_tcs3410_table_count(void);

#endif
