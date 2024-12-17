/*
 *  Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 *  Description: the partition table.
 */

#ifndef _SAGITTARIUS_XLOADER_PARTITION_H_
#define _SAGITTARIUS_XLOADER_PARTITION_H_

#include "partition_macro.h"


#define XLOADER3_VRL_INDEX_A    19
#define BL2_VRL_INDEX           21
#define FASTBOOT_VRL_INDEX_A    37
#define VECTOR_VRL_INDEX_A      38

static const struct partition partition_table_emmc[] = {
    {"0", 0, 0, 0},
};

static const struct partition partition_table_ufs[] = {
    {PART_XLOADER,                        0,      2 * 1024,    UFS_PART_0},
    {PART_RESERVED1,               7 * 1024,          1024,    UFS_PART_2}, /* reserved1   1024K    p3 */
    {PART_VRL,                          512,           512,    UFS_PART_3}, /* vrl          512K    p1 */
    {PART_VRL_BACKUP,                  1024,           512,    UFS_PART_3}, /* vrl backup   512K    p2 */
    {PART_NVME,                   10 * 1024,      5 * 1024,    UFS_PART_3}, /* nvme           5M    p4 */
    {PART_DDR_PARA,              223 * 1024,      1 * 1024,    UFS_PART_3}, /* DDR_PARA       1M    p12 */
    {PART_LOWPOWER_PARA,         224 * 1024,      1 * 1024,    UFS_PART_3}, /* lowpower_para  1M    p13 */
    {PART_BL2,                   226 * 1024,      4 * 1024,    UFS_PART_3}, /* bl2            4M    p15 */
    {PART_DFX,                   339 * 1024,     16 * 1024,    UFS_PART_3}, /* dfx           16M    p20 */
    {PART_FASTBOOT,              512 * 1024,     12 * 1024,    UFS_PART_3}, /* fastboot      12M    p31 */
    {PART_VECTOR,                524 * 1024,      4 * 1024,    UFS_PART_3}, /* vector         4M    p32 */
#ifdef FACTORY_VERSION
    {PART_HIBENCH_IMG,         12996 * 1024,    128 * 1024,    UFS_PART_3}, /* hibench_img  128M    p76 */
    {PART_HIBENCH_LOG,         14404 * 1024,     32 * 1024,    UFS_PART_3}, /* hibench_log   32M    p79 */
#endif
#ifdef CONFIG_HISI_DEBUG_FS
    {PART_FTTEST,          24 * 1024 * 1024,    192 * 1024,    UFS_PART_3}, /* fttest       192M */
#endif
#ifdef XLOADER_DDR_TEST
    {PART_DDRTEST,             25068 * 1024,     48 * 1024,     UFS_PART_3}, /* ddrtest         48M */
#endif
    {"0", 0, 0, 0},
};

#endif
