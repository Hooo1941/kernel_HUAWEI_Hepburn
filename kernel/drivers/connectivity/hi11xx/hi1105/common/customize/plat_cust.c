/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description: plat cust
 * Author: @CompanyNameTag
 */

/* 头文件包含 */
#include "plat_cust.h"
#include "oal_util.h"

OAL_STATIC uint8_t *g_plat_cust_data_buf = NULL;
OAL_STATIC uint32_t g_plat_cust_data_len = 0;

int32_t set_plat_cust_buf(uint8_t *memory, uint32_t len)
{
    uint8_t *buffer = NULL;
    buffer = (uint8_t *)os_kzalloc_gfp(len);
    if (buffer == NULL) {
        return -ENOMEM;
    }

    if (memcpy_s(buffer, len, memory, len) != EOK) {
        os_mem_kfree(buffer);
        return -EFAIL;
    }
    g_plat_cust_data_buf = buffer;
    g_plat_cust_data_len = len;
    return SUCC;
}

uint8_t *get_plat_cust_addr(void)
{
    return g_plat_cust_data_buf;
}

uint32_t get_plat_cust_len(void)
{
    return g_plat_cust_data_len;
}

void plat_cust_parameter_free(void)
{
    if (g_plat_cust_data_buf != NULL) {
        os_mem_kfree(g_plat_cust_data_buf);
        g_plat_cust_data_buf = NULL;
    }
}
