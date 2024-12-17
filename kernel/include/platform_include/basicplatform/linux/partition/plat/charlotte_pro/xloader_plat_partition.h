/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2021. All rights reserved.
 * Description: the partition table.
 */

#ifndef _ANTLIA_XLOADER_PLAT_PARTITION_H_
#define _ANTLIA_XLOADER_PLAT_PARTITION_H_

#include "partition_macro.h"
#include "partition_macro_plus.h"

#ifdef CONFIG_SPI_NOR_STORAGE
#define XLOADER3_VRL_INDEX_A    21
#define BL2_VRL_INDEX            7
#define FASTBOOT_VRL_INDEX_A    18
#define XLOADER3_VRL_INDEX_B    39
#define BL2_VRL_INDEX_B         25
#define FASTBOOT_VRL_INDEX_B    36
#else
#define XLOADER3_VRL_INDEX_A    22
#define BL2_VRL_INDEX           33
#define FASTBOOT_VRL_INDEX_A    45
#define VECTOR_VRL_INDEX_A      46
#endif

/* used as spi_nor flash */
static const struct partition partition_table_emmc[] = {
    {PART_PTABLE,                            0,           136,        SPI_NOR_PART}, /* ptable            136K    p0 */
    {PART_BOOT_CTRL,                       136,             4,        SPI_NOR_PART}, /* boot_ctrl           4k    p1 */
    {PART_NVME,                            140,          1024,        SPI_NOR_PART}, /* nvme              252K    p2 */
    {PART_DDR_PARA,                       1164,          1000,        SPI_NOR_PART}, /* DDR_PARA         1000K    p3 */
    {PART_FW_CPU_LPCTRL,                  2860,           136,        SPI_NOR_PART}, /* fw_cpu_lpctrl_a   128K    p11 */
    {PART_FW_GPU_LPCTRL,                  2996,           128,        SPI_NOR_PART}, /* fw_gpu_lpctrl_a   128K    p11 */
    {PART_FW_DDR_LPCTRL,                  3124,           128,        SPI_NOR_PART}, /* fw_ddr_lpctrl_a   128K    p12 */
    {PART_XLOADER_A,                      3380,           672,        SPI_NOR_PART}, /* xloader_a         528K    p6 */
    {PART_BL2,                            4052,          2000,        SPI_NOR_PART}, /* bl2_a            2000K    p7 */
    {PART_DDR_XPU_BOOT0,                  6688,           136,        SPI_NOR_PART}, /* fw_ddr_init_a     272K    p13 */
    {PART_DDR_XPU_BOOT1,                  6824,           136,        SPI_NOR_PART}, /* fw_ddr_init_a     272K    p13 */
    {PART_FASTBOOT,                       7984,          4608,        SPI_NOR_PART}, /* uefi_a           4608K    p17 */
    {PART_FTTEST,                        12592,          4608,        SPI_NOR_PART}, /* fttest           4608K    p18 */
    {PART_VRL,                           17200,           252,        SPI_NOR_PART}, /* vrl_a             252K    p20 */
    {PART_RESERVED1,                     18044,            20,        SPI_NOR_PART}, /* reserved1        1400K    p22 */
    {PART_XLOADER_B,                     18064,           672,        SPI_NOR_PART}, /* xloader_b         528K    p23 */
    {PART_LOWPOWER_PARA,                 19236,           692,        SPI_NOR_PART}, /* lowpower_para_a   692K    p25 */
    {PART_RESERVED2,                     31392,          1356,        SPI_NOR_PART}, /* reserved2        1436K    p30 */
    {"0", 0, 0, 0},
};

static const struct partition partition_table_ufs[] = {
    {PART_XLOADER,                          0,        2 * 1024,         UFS_PART_0},
    {PART_RESERVED1,                 7 * 1024,            1024,         UFS_PART_2}, /* reserved1       1024K    p3 */
    {PART_VRL,                            512,             512,         UFS_PART_3}, /* vrl              512K    p1 */
    {PART_VRL_BACKUP,                    1024,             512,         UFS_PART_3}, /* vrl backup       512K    p2 */
    {PART_NVME,                     10 * 1024,        4 * 1024,         UFS_PART_3}, /* nvme               4M    p4 */
    {PART_DDR_PARA,                227 * 1024,        1 * 1024,         UFS_PART_3}, /* DDR_PARA           1M    p14 */
    {PART_LOWPOWER_PARA,           228 * 1024,        1 * 1024,         UFS_PART_3}, /* lowpower_para      1M    p15 */
    {PART_DDR_XPU_BOOT0,    (250 * 1024 + 752),           136,          UFS_PART_3}, /* ddr_xpu_boot0      136KB p53 */
    {PART_DDR_XPU_BOOT1,    (250 * 1024 + 888),           136,          UFS_PART_3}, /* ddr_xpu_boot1      136KB p53 */
    {PART_DFX,                     335 * 1024,       16 * 1024,         UFS_PART_3}, /* dfx               16M    p21 */
    {PART_BL2,                     492 * 1024,        4 * 1024,         UFS_PART_3}, /* bl2                4M    p27 */
    {PART_FW_CPU_LPCTRL,   (496 * 1024 + 512),             256,         UFS_PART_3}, /* fw_cpu_lpctrl    256K    p29 */
    {PART_FW_GPU_LPCTRL,   (496 * 1024 + 768),             128,         UFS_PART_3}, /* fw_gpu_lpctrl    128K    p30 */
    {PART_FW_DDR_LPCTRL,   (496 * 1024 + 896),             128,         UFS_PART_3}, /* fw_ddr_lpctrl    128K    p31 */
    {PART_HISEE_IMG,               508 * 1024,        4 * 1024,         UFS_PART_3}, /* hisee_img          4M    p36 */
    {PART_HISEE_FS,                516 * 1024,        8 * 1024,         UFS_PART_3}, /* hisee_fs           8M    p38 */
    {PART_FASTBOOT,                524 * 1024,       12 * 1024,         UFS_PART_3}, /* fastboot          12M    p39 */
    {PART_VECTOR,                  536 * 1024,        4 * 1024,         UFS_PART_3}, /* vector             4M    p40 */
#ifdef FACTORY_VERSION
    {PART_HIBENCH_IMG,            12988 * 1024,     128 * 1024,         UFS_PART_3}, /* hibench_img      128M    p87 */
    {PART_HIBENCH_LOG,            14140 * 1024,      32 * 1024,         UFS_PART_3}, /* hibench_log       32M    p90 */
#endif
#ifdef CONFIG_PARTITION_ENG
#ifdef CONFIG_HISI_DEBUG_FS
    {PART_FTTEST,                 14932 * 1024,     192 * 1024,         UFS_PART_3}, /* fttest           192M    p85 */
#endif
#ifdef XLOADER_DDR_TEST
    {PART_DDRTEST,                15120 * 1024,      48 * 1024,         UFS_PART_3}, /* ddrtest           48M    p86 */
#endif
#endif
    {"0", 0, 0, 0},
};

#endif
