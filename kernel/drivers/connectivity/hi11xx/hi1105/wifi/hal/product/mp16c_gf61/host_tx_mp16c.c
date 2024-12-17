/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : host侧MP16C芯片相关TX功能
 * 作    者 : wifi
 * 创建日期 : 2021年12月26日
 */

#include "host_tx_mp16c.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HOST_HAL_TX_MP16C_C

#define MP16C_DEVICE_LARGE_TX_RING_NUM 21
#define MP16C_DEVICE_SMALL_TX_RING_NUM 91
#define MP16C_DEVICE_TX_RING_TOTAL_NUM (MP16C_DEVICE_LARGE_TX_RING_NUM + MP16C_DEVICE_SMALL_TX_RING_NUM)
uint8_t mp16c_host_get_device_tx_ring_num(void)
{
    return MP16C_DEVICE_TX_RING_TOTAL_NUM;
}
