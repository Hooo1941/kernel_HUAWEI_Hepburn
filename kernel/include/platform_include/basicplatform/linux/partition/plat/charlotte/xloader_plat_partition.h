/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2020. All rights reserved.
 * Description: the partition table.
 */

#ifndef _SCORPIO_XLOADER_PARTITION_H_
#define _SCORPIO_XLOADER_PARTITION_H_

#include "partition_macro.h"
#include "partition_macro_plus.h"


#define XLOADER3_VRL_INDEX_A    22
#define BL2_VRL_INDEX           33
#define FASTBOOT_VRL_INDEX_A    45
#define VECTOR_VRL_INDEX_A      46

static const struct partition partition_table_emmc[] = {
    {"0", 0, 0, 0},
};

static const struct partition partition_table_ufs[] = {
    {PART_XLOADER,                0,              2 * 1024,    UFS_PART_0},
    {PART_VRL,                    512,                 512,    UFS_PART_3}, /* vrl              512K    p1 */
    {PART_VRL_BACKUP,             1024,                512,    UFS_PART_3}, /* vrl backup       512K    p2 */
    {PART_NVME,                   10 * 1024,      4 * 1024,    UFS_PART_3}, /* nvme               4M    p4 */
    {PART_DDR_PARA,               227 * 1024,     1 * 1024,    UFS_PART_3}, /* DDR_PARA           1M    p14 */
    {PART_LOWPOWER_PARA,          228 * 1024,     1 * 1024,    UFS_PART_3}, /* lowpower_para      1M    p15 */
    {PART_DFX,                    335 * 1024,    16 * 1024,    UFS_PART_3}, /* dfx               16M    p21 */
    {PART_BL2,                    492 * 1024,     4 * 1024,    UFS_PART_3}, /* bl2                4M    p27 */
    {PART_FW_CPU_LPCTRL,         (496 * 1024 + 512),   256,    UFS_PART_3}, /* fw_cpu_lpctrl    256K    p29 */
    {PART_FW_GPU_LPCTRL,         (496 * 1024 + 768),   128,    UFS_PART_3}, /* fw_gpu_lpctrl    128K    p30 */
    {PART_FW_DDR_LPCTRL,         (496 * 1024 + 896),   128,    UFS_PART_3}, /* fw_ddr_lpctrl    128K    p31 */
    {PART_HISEE_IMG,              508 * 1024,     4 * 1024,    UFS_PART_3}, /* hisee_img          4M    p36 */
    {PART_HISEE_FS,               516 * 1024,     8 * 1024,    UFS_PART_3}, /* hisee_fs           8M    p38 */
    {PART_FASTBOOT,               524 * 1024,    12 * 1024,    UFS_PART_3}, /* fastboot          12M    p39 */
    {PART_VECTOR,                 536 * 1024,     4 * 1024,    UFS_PART_3}, /* vector             4M    p40 */
#ifdef FACTORY_VERSION
    {PART_HIBENCH_IMG,          13584 * 1024,   128 * 1024,    UFS_PART_3}, /* hibench_img      128M    p87 */
    {PART_FLASH_AGEING,         14224 * 1024,   512 * 1024,    UFS_PART_3}, /* flash_ageing     512MB   p89 */
    {PART_HIBENCH_LOG,          14736 * 1024,    32 * 1024,    UFS_PART_3}, /* hibench_log       32M    p90 */
    {PART_HITEST_LPM3,          14768 * 1024,     2 * 1024,    UFS_PART_3}, /* hitest_lpm3        2M    p91 */
#endif
#if defined(CONFIG_BOOT_UEFI) && defined(CONFIG_PARTITION_ENG)
#ifdef CONFIG_HISI_DEBUG_FS
    {PART_FTTEST,               15272 * 1024,   192 * 1024,    UFS_PART_3}, /* fttest           192M    p87 */
#endif
#ifdef XLOADER_DDR_TEST
    {PART_DDRTEST,              15464 * 1024,    48 * 1024,    UFS_PART_3}, /* ddrtest           48M    p88 */
#endif
#else
#ifdef CONFIG_HISI_DEBUG_FS
    {PART_FTTEST,          30720 * 1024,   192 * 1024,    UFS_PART_3}, /* fttest           192M p87 */
#endif
#ifdef XLOADER_DDR_TEST
    {PART_DDRTEST,         30720 * 1024 + 943 * 1024,   48 * 1024,     UFS_PART_3}, /* ddrtest  48M p88 */
#endif
#endif
    {"0", 0, 0, 0},
};

#endif
