/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description: plat_cust.c header file
 * Author: @CompanyNameTag
 */

#ifndef PLAT_CUST_H
#define PLAT_CUST_H

#include "plat_type.h"

int32_t set_plat_cust_buf(uint8_t *memory, uint32_t len);
uint8_t *get_plat_cust_addr(void);
uint32_t get_plat_cust_len(void);
void plat_cust_parameter_free(void);
#endif
