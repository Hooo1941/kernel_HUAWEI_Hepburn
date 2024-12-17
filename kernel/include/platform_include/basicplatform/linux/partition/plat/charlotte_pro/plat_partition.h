/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2021. All rights reserved.
 * Description: the partition table.
 */
#ifndef _ANTLIA_PLAT_PARTITION_H_
#define _ANTLIA_PLAT_PARTITION_H_

#include "partition_macro.h"
#include "partition_macro_plus.h"
#include "partition_def.h"

static const struct partition partition_table_emmc[] = {
    {"0", 0, 0, 0},
};

/* The size of the NOR flash is only 32 MB.
 * Therefore, the non-A/B partition table is used.
 * After the formal compression and decompression solution is incorporated,
 * the table is changed to the AB partition.
 */
static struct partition partition_table_ufs[] = {
    {PART_PTABLE,                            0,           136,        SPI_NOR_PART}, /* ptable            136K    p0  */
    {PART_BOOT_CTRL,                       136,             4,        SPI_NOR_PART}, /* boot_ctrl           4k    p1  */
    {PART_NVME,                            140,          1024,        SPI_NOR_PART}, /* nvme              252K    p2  */
    {PART_DDR_PARA,                       1164,          1000,        SPI_NOR_PART}, /* DDR_PARA         1000K    p3  */
    {PART_MSPC_DATA,                      2164,           500,        SPI_NOR_PART}, /* mspc_data         500K    p4  */
    {PART_NPU,                            2664,           196,        SPI_NOR_PART}, /* npu              1488K    p5  */
    {PART_FW_CPU_LPCTRL,                  2860,           136,        SPI_NOR_PART}, /* fw_cpu_lpctrl_a   128K    p11 */
    {PART_FW_GPU_LPCTRL,                  2996,           128,        SPI_NOR_PART}, /* fw_gpu_lpctrl_a   128K    p12 */
    {PART_FW_DDR_LPCTRL,                  3124,           128,        SPI_NOR_PART}, /* fw_ddr_lpctrl_a   128K    p13 */
    {PART_DDR_SWAP,                       3252,           128,        SPI_NOR_PART}, /* ddr_swap_a        600K    p19 */
    {PART_XLOADER_A,                      3380,           672,        SPI_NOR_PART}, /* xloader_a         528K    p7  */
    {PART_BL2,                            4052,          2000,        SPI_NOR_PART}, /* bl2_a            2000K    p8  */
    {PART_MSPC_CODE,                      6052,           236,        SPI_NOR_PART}, /* mspc_code_a       236K    p9  */
    {PART_FW_LPM3,                        6288,           400,        SPI_NOR_PART}, /* fw_lpm3_a         172K    p10 */
    {PART_DDR_XPU_BOOT0,                  6688,           136,        SPI_NOR_PART}, /* fw_ddr_init_a     272K    p14 */
    {PART_DDR_XPU_BOOT1,                  6824,           136,        SPI_NOR_PART}, /* fw_ddr_init_a     272K    p15 */
    {PART_TRUSTFIRMWARE,                  6960,          1024,        SPI_NOR_PART}, /* trustfirmware_a   168K    p16 */
    {PART_FASTBOOT,                       7984,          4608,        SPI_NOR_PART}, /* uefi_a           4608K    p17 */
    {PART_FTTEST,                        12592,          4608,        SPI_NOR_PART}, /* fttest           4608K    p18 */
    {PART_VRL,                           17200,           252,        SPI_NOR_PART}, /* vrl_a             252K    p20 */
    {PART_DTBO,                          17452,            12,        SPI_NOR_PART}, /* dtbo_a            252K    p21 */
    {PART_FW_DTB,                        17464,           580,        SPI_NOR_PART}, /* FW_dtb           1488K    p6  */
    {PART_RESERVED1,                     18044,            20,        SPI_NOR_PART}, /* reserved1        1400K    p22 */
    /* !!!Attention!!! cannot modify xloader_b offset */
    {PART_XLOADER_B,                     18064,           672,        SPI_NOR_PART}, /* xloader_b         528K    p23 */
    {PART_SENSORHUB,                     18736,           500,        SPI_NOR_PART}, /* sensorhub_a       600K    p24 */
    {PART_LOWPOWER_PARA,                 19236,           692,        SPI_NOR_PART}, /* lowpower_para_a   692K    p25 */
    {PART_HHEE,                          19928,           200,        SPI_NOR_PART}, /* hhee_b            252K    p26 */
    {PART_THEE,                          20128,          2048,        SPI_NOR_PART}, /* thee_a             2MB    p27 */
    {PART_TZSP,                          22176,          5120,        SPI_NOR_PART}, /* tzsp_a             5MB    p28 */
    {PART_TEEOS,                         27296,          4096,        SPI_NOR_PART}, /* teeos_a          2556K    p29 */
    {PART_RESERVED2,                     31392,          1356,        SPI_NOR_PART}, /* reserved2        1436K    p30 */
    {PART_RESERVED0,                        0,       2 * 1024,          UFS_PART_1},
    {PART_PTABLE,                            0,           512,          UFS_PART_2}, /* ptable           512K    p0 */
    {PART_FRP,                             512,           512,          UFS_PART_2}, /* frp                512KB p1 */
    {PART_PERSIST,                    1 * 1024,      6 * 1024,          UFS_PART_2}, /* persist         6144K    p2 */
    {PART_RESERVED1,                  7 * 1024,          1024,          UFS_PART_2}, /* reserved1         1024KB p3 */
    {PART_PTABLE_LU3,                        0,           512,          UFS_PART_3}, /* ptable_lu3       512K    p0 */
    {PART_VRL,                             512,           512,          UFS_PART_3}, /* vrl                512KB p1 */
    {PART_VRL_BACKUP,                     1024,           512,          UFS_PART_3}, /* vrl_backup         512KB p2 */
    {PART_MODEM_SECURE,                   1536,          8704,          UFS_PART_3}, /* modem_secure      8704KB p3 */
    {PART_NVME,                      10 * 1024,      4 * 1024,          UFS_PART_3}, /* nvme                 4MB p4 */
    {PART_CONN_CALIDATA,             14 * 1024,          1024,          UFS_PART_3}, /* conn_calidata     1024KB p5 */
    {PART_CTF,                       15 * 1024,      1 * 1024,          UFS_PART_3}, /* ctf               1024KB p6 */
    {PART_OEMINFO,                   16 * 1024,     96 * 1024,          UFS_PART_3}, /* oeminfo             96MB p7 */
    {PART_SECURE_STORAGE,           112 * 1024,     32 * 1024,          UFS_PART_3}, /* secure_storage      32MB p8 */
    {PART_MODEMNVM_FACTORY,         144 * 1024,     16 * 1024,          UFS_PART_3}, /* modemnvm_factory    16MB p9 */
    {PART_MODEMNVM_BACKUP,          160 * 1024,     16 * 1024,          UFS_PART_3}, /* modemnvm_backup     16MB p10 */
    {PART_MODEMNVM_IMG,             176 * 1024,     46 * 1024,          UFS_PART_3}, /* modemnvm_img        46MB p11 */
    {PART_HISEE_ENCOS,              222 * 1024,      4 * 1024,          UFS_PART_3}, /* hisee_encos          4MB p12 */
    {PART_VERITYKEY,                226 * 1024,      1 * 1024,          UFS_PART_3}, /* veritykey         1024KB p13 */
    {PART_DDR_PARA,                 227 * 1024,      1 * 1024,          UFS_PART_3}, /* ddr_para          1024KB p14 */
    {PART_LOWPOWER_PARA,            228 * 1024,      1 * 1024,          UFS_PART_3}, /* lowpower_para     1024KB p15 */
    {PART_BATT_TP_PARA,             229 * 1024,      1 * 1024,          UFS_PART_3}, /* batt_tp_para      1024KB p16 */
    {PART_RESERVED2,               230 * 1024,(20 * 1024 + 752),        UFS_PART_3}, /* reserved2           21MB p17 */
    {PART_DDR_XPU_BOOT0,    (250 * 1024 + 752),           136,          UFS_PART_3}, /* ddr_xpu_boot0      136KB p18 */
    {PART_DDR_XPU_BOOT1,    (250 * 1024 + 888),           136,          UFS_PART_3}, /* ddr_xpu_boot1      136KB p19 */
    {PART_SPLASH2,                  251 * 1024,     80 * 1024,          UFS_PART_3}, /* splash2             80MB p20 */
    {PART_BOOTFAIL_INFO,            331 * 1024,      2 * 1024,          UFS_PART_3}, /* bootfail_info        2MB p21 */
    {PART_MISC,                     333 * 1024,      2 * 1024,          UFS_PART_3}, /* misc                 2MB p22 */
    {PART_DFX,                      335 * 1024,     16 * 1024,          UFS_PART_3}, /* dfx                 16MB p23 */
    {PART_RRECORD,                  351 * 1024,     16 * 1024,          UFS_PART_3}, /* rrecord             16MB p24 */
    {PART_CACHE,                    367 * 1024,    104 * 1024,          UFS_PART_3}, /* cache              104MB p25 */
    {PART_METADATA,                 471 * 1024,     16 * 1024,          UFS_PART_3}, /* metadata            16MB p26 */
    {PART_RESERVED3,                487 * 1024,      4 * 1024,          UFS_PART_3}, /* reserved3            4MB p27 */
    {PART_TOC,                      491 * 1024,      1 * 1024,          UFS_PART_3}, /* toc               1024KB p28 */
    {PART_BL2,                      492 * 1024,      4 * 1024,          UFS_PART_3}, /* bl2                  4MB p29 */
    {PART_FW_LPM3,                  496 * 1024,           512,          UFS_PART_3}, /* fw_lpm3            512KB p30 */
    {PART_FW_CPU_LPCTRL,     (496 * 1024 + 512),          256,          UFS_PART_3}, /* fw_cpu_lpctrl    256K    p31 */
    {PART_FW_GPU_LPCTRL,     (496 * 1024 + 768),          128,          UFS_PART_3}, /* fw_gpu_lpctrl    128K    p32 */
    {PART_FW_DDR_LPCTRL,     (496 * 1024 + 896),          128,          UFS_PART_3}, /* fw_ddr_lpctrl    128K    p33 */
    {PART_NPU,                      497 * 1024,     8 * 1024,           UFS_PART_3}, /* npu                  8MB p34 */
    {PART_IVP,                      505 * 1024,     2 * 1024,           UFS_PART_3}, /* ivp                  2MB p35 */
    {PART_DACC,                     507 * 1024,          256,           UFS_PART_3}, /* dacc               256KB p34 */
    {PART_DACC_LITE,        (507 * 1024 + 256),          256,           UFS_PART_3}, /* dacc_lite          256KB p34 */
    {PART_VENC,             (507 * 1024 + 512),          512,           UFS_PART_3}, /* venc               512KB p37 */
    {PART_HISEE_IMG,                508 * 1024,     4 * 1024,           UFS_PART_3}, /* hisee_img            4MB p38 */
    {PART_HHEE,                     512 * 1024,     4 * 1024,           UFS_PART_3}, /* hhee                 4MB p39 */
    {PART_HISEE_FS,                 516 * 1024,     8 * 1024,           UFS_PART_3}, /* hisee_fs             8MB p40 */
    {PART_FASTBOOT,                 524 * 1024,    12 * 1024,           UFS_PART_3}, /* fastboot            12MB p41 */
    {PART_VECTOR,                   536 * 1024,     4 * 1024,           UFS_PART_3}, /* vector               4MB p42 */
    {PART_ISP_BOOT,                 540 * 1024,     2 * 1024,           UFS_PART_3}, /* isp_boot             2MB p43 */
    {PART_ISP_FIRMWARE,             542 * 1024,    14 * 1024,           UFS_PART_3}, /* isp_firmware        14MB p44 */
    {PART_FW_HIFI,                  556 * 1024,    12 * 1024,           UFS_PART_3}, /* fw_hifi             12MB p45 */
    {PART_TEEOS,                    568 * 1024,     8 * 1024,           UFS_PART_3}, /* teeos                8MB p46 */
    {PART_SENSORHUB,                576 * 1024,    13 * 1024,           UFS_PART_3}, /* sensorhub           13MB p47 */
    {PART_SENSORHUB_LOG_DIC,        589 * 1024,     3 * 1024,           UFS_PART_3}, /* sensorhub_log_dic    3MB p47 */
    {PART_HKRR,                     592 * 1024,    16 * 1024,           UFS_PART_3}, /* hkrr                16MB p48 */
#ifdef CONFIG_SANITIZER_ENABLE
    {PART_ERECOVERY_RAMDISK,        608 * 1024,    12 * 1024,           UFS_PART_3}, /* erecovery_ramdisk   12MB p49 */
    {PART_ERECOVERY_VENDOR,         620 * 1024,     8 * 1024,           UFS_PART_3}, /* erecovery_vendor     8MB p50 */
    {PART_BOOT,                     628 * 1024,    100 * 1024,           UFS_PART_3}, /* boot               100MB p51 */
    {PART_RECOVERY,                 728 * 1024,    85 * 1024,           UFS_PART_3}, /* recovery            85MB p52 */
    {PART_ERECOVERY,                813 * 1024,    12 * 1024,           UFS_PART_3}, /* erecovery           12MB p53 */
    {PART_RESERVED,                 825 * 1024,    14 * 1024,           UFS_PART_3}, /* reserved            14MB p54 */
#else
    {PART_ERECOVERY_RAMDISK,        608 * 1024,    32 * 1024,           UFS_PART_3}, /* erecovery_ramdisk   32MB p49 */
    {PART_ERECOVERY_VENDOR,         640 * 1024,    24 * 1024,           UFS_PART_3}, /* erecovery_vendor    24MB p50 */
    {PART_BOOT,                     664 * 1024,    46 * 1024,           UFS_PART_3}, /* boot                46MB p51 */
    {PART_RECOVERY,                 710 * 1024,    45 * 1024,           UFS_PART_3}, /* recovery            45MB p52 */
    {PART_ERECOVERY,                755 * 1024,    45 * 1024,           UFS_PART_3}, /* erecovery           45MB p53 */
    {PART_RESERVED,                 800 * 1024,    39 * 1024,           UFS_PART_3}, /* reserved            39MB p54 */
#endif
    {PART_THEE,                     839 * 1024,     4 * 1024,           UFS_PART_3}, /* thee                 4MB p55 */
    {PART_TZSP,                     843 * 1024,    12 * 1024,           UFS_PART_3}, /* tzsp                12MB p56 */
    {PART_RECOVERY_RAMDISK,         855 * 1024,    32 * 1024,           UFS_PART_3}, /* recovery_ramdisk    32MB p57 */
    {PART_RECOVERY_VENDOR,          887 * 1024,    24 * 1024,           UFS_PART_3}, /* recovery_vendor     24MB p58 */
    {PART_ENG_SYSTEM,               911 * 1024,    12 * 1024,           UFS_PART_3}, /* eng_system          12MB p59 */
    {PART_ENG_VENDOR,               923 * 1024,    20 * 1024,           UFS_PART_3}, /* eng_vendor          20MB p60 */
    {PART_FW_DTB,                   943 * 1024,     8 * 1024,           UFS_PART_3}, /* fw_dtb               8MB p61 */
    {PART_DTBO,                     951 * 1024,    14 * 1024,           UFS_PART_3}, /* dtbo                14MB p62 */
    {PART_TRUSTFIRMWARE,            965 * 1024,     2 * 1024,           UFS_PART_3}, /* trustfirmware        2MB p63 */
    {PART_MODEM_FW,                 967 * 1024,   124 * 1024,           UFS_PART_3}, /* modem_fw           124MB p64 */
    {PART_MODEM_VENDOR,            1091 * 1024,    10 * 1024,           UFS_PART_3}, /* modem_vendor        10MB p65 */
    {PART_MODEM_PATCH_NV,          1101 * 1024,     4 * 1024,           UFS_PART_3}, /* modem_patch_nv       4MB p66 */
    {PART_MODEM_DRIVER,            1105 * 1024,    20 * 1024,           UFS_PART_3}, /* modem_driver        20MB p67 */
    {PART_MODEMNVM_UPDATE,         1125 * 1024,    16 * 1024,           UFS_PART_3}, /* modemnvm_update     16MB p68 */
    {PART_MODEMNVM_CUST,           1141 * 1024,    16 * 1024,           UFS_PART_3}, /* modemnvm_cust       16MB p69 */
    {PART_RAMDISK,                 1157 * 1024,     2 * 1024,           UFS_PART_3}, /* ramdisk              2MB p70 */
    {PART_VBMETA_SYSTEM,           1159 * 1024,     1 * 1024,           UFS_PART_3}, /* vbmeta_system     1024KB p71 */
    {PART_VBMETA_VENDOR,           1160 * 1024,     1 * 1024,           UFS_PART_3}, /* vbmeta_vendor     1024KB p72 */
    {PART_VBMETA_ODM,              1161 * 1024,     1 * 1024,           UFS_PART_3}, /* vbmeta_odm        1024KB p73 */
    {PART_VBMETA_CUST,             1162 * 1024,     1 * 1024,           UFS_PART_3}, /* vbmeta_cust       1024KB p74 */
    {PART_VBMETA_HW_PRODUCT,       1163 * 1024,     1 * 1024,           UFS_PART_3}, /* vbmeta_hw_product 1024KB p75 */
    {PART_RECOVERY_VBMETA,         1164 * 1024,     2 * 1024,           UFS_PART_3}, /* recovery_vbmeta      2MB p76 */
    {PART_ERECOVERY_VBMETA,        1166 * 1024,     2 * 1024,           UFS_PART_3}, /* erecovery_vbmeta     2MB p77 */
    {PART_VBMETA,                  1168 * 1024,     4 * 1024,           UFS_PART_3}, /* vbmeta               4MB p78 */
    {PART_KPATCH,                  1172 * 1024,     4 * 1024,           UFS_PART_3}, /* kpatch               4MB p79 */
    {PART_PATCH,                   1176 * 1024,    32 * 1024,           UFS_PART_3}, /* patch               32MB p80 */
    {PART_SEPLAT,                  1208 * 1024,     4 * 1024,           UFS_PART_3}, /* seplat               4MB p81 */
#ifdef CONFIG_FACTORY_MODE
    {PART_PREAS,              1212 * 1024,   368 * 1024, UFS_PART_3}, /* preas              368MB p82 */
    {PART_PREAVS,             1580 * 1024,    32 * 1024, UFS_PART_3}, /* preavs              32MB p83 */
    {PART_SUPER,              1612 * 1024,  9656 * 1024, UFS_PART_3}, /* super             9656MB p84 */
    {PART_VERSION,           11268 * 1024,   576 * 1024, UFS_PART_3}, /* version            576MB p85 */
    {PART_PRELOAD,           11844 * 1024,  1144 * 1024, UFS_PART_3}, /* preload           1144MB p86 */
    {PART_HIBENCH_IMG,       12988 * 1024,   128 * 1024, UFS_PART_3}, /* hibench_img        128MB p87 */
    {PART_HIBENCH_DATA,      13116 * 1024,   512 * 1024, UFS_PART_3}, /* hibench_data       512MB p88 */
    {PART_FLASH_AGEING,      13628 * 1024,   512 * 1024, UFS_PART_3}, /* flash_ageing       512MB p89 */
    {PART_HIBENCH_LOG,       14140 * 1024,    32 * 1024, UFS_PART_3}, /* hibench_log         32MB p90 */
    {PART_USERDATA,          14172 * 1024,(16UL) * 1024 * 1024, UFS_PART_3}, /* userdata        6G p91 */
#elif defined CONFIG_MARKET_INTERNAL
    {PART_PREAS,              1212 * 1024,   368 * 1024, UFS_PART_3}, /* preas              368MB p82 */
    {PART_PREAVS,             1580 * 1024,    32 * 1024, UFS_PART_3}, /* preavs              32MB p83 */
    {PART_SUPER,              1612 * 1024, 10208 * 1024, UFS_PART_3}, /* super             10208MB p84 */
    {PART_VERSION,           11820 * 1024,   576 * 1024, UFS_PART_3}, /* version            576MB p85 */
    {PART_PRELOAD,           12396 * 1024,  1144 * 1024, UFS_PART_3}, /* preload           1144MB p86 */
    {PART_USERDATA,          13540 * 1024,(16UL) * 1024 * 1024, UFS_PART_3}, /* userdata        6G p87 */
#elif defined CONFIG_GCOV_PARTITION_SUPPORT_ENABLE
    {PART_PREAS,              1212 * 1024,    32 * 1024, UFS_PART_3}, /* preas               32MB p82 */
    {PART_PREAVS,             1244 * 1024,    32 * 1024, UFS_PART_3}, /* preavs              32MB p83 */
    {PART_SUPER,              1276 * 1024, 10688 * 1024, UFS_PART_3}, /* super             10688MB p84 */
    {PART_VERSION,           11964 * 1024,   576 * 1024, UFS_PART_3}, /* version            576MB p85 */
    {PART_PRELOAD,           12540 * 1024,  1144 * 1024, UFS_PART_3}, /* preload           1144MB p86 */
    {PART_USERDATA,          13684 * 1024,(16UL) * 1024 * 1024, UFS_PART_3}, /* userdata        6G p87 */
#else
    {PART_PREAS,              1212 * 1024,  1280 * 1024, UFS_PART_3}, /* preas             1280MB p82 */
    {PART_PREAVS,             2492 * 1024,    32 * 1024, UFS_PART_3}, /* preavs              32MB p83 */
    {PART_SUPER,              2524 * 1024, 10688 * 1024, UFS_PART_3}, /* super             10688MB p84 */
    {PART_VERSION,           13212 * 1024,   576 * 1024, UFS_PART_3}, /* version            576MB p85 */
    {PART_PRELOAD,           13788 * 1024,  1144 * 1024, UFS_PART_3}, /* preload           1144MB p86 */
#ifdef CONFIG_PARTITION_ENG
    {PART_FTTEST,            14932 * 1024,   192 * 1024, UFS_PART_3}, /* fttest             192MB p87 */
    {PART_DDRTEST,           15124 * 1024,    48 * 1024, UFS_PART_3}, /* ddrtest             48MB p88 */
    {PART_HITEST_GPU,        15172 * 1024,   512 * 1024, UFS_PART_3}, /* hitest_gpu         512MB p89 */
    {PART_HITEST_ASP_DHRYSTONE, 15684 * 1024,    20 * 1024, UFS_PART_3}, /* hitest_asp_dhrystone  20MB p90 */
    {PART_HITEST_ASP_TEST,   15704 * 1024,   300 * 1024, UFS_PART_3}, /* hitest_asp_test    300MB p91 */
    {PART_HITEST_SENSORHUB,  16004 * 1024,     6 * 1024, UFS_PART_3}, /* hitest_sensorhub     6MB p92 */
    {PART_HITEST_ISP,        16010 * 1024,    25 * 1024, UFS_PART_3}, /* hitest_isp          25MB p93 */
    {PART_HITEST_ISP_AVS,    16035 * 1024,    25 * 1024, UFS_PART_3}, /* hitest_isp_avs      25MB p94 */
    {PART_HITEST_NPU,        16060 * 1024,    80 * 1024, UFS_PART_3}, /* hitest_npu          80MB p95 */
    {PART_HITEST_DSS,        16140 * 1024,    18 * 1024, UFS_PART_3}, /* hitest_dss          18MB p96 */
    {PART_DSS_DISP_BUF,      16158 * 1024,    64 * 1024, UFS_PART_3}, /* dss_disp_buf        64MB p97 */
    {PART_HITEST_JPEGENC,    16222 * 1024,   200 * 1024, UFS_PART_3}, /* hitest_jpegenc     200MB p98 */
    {PART_HITEST_HIPP,       16422 * 1024,   100 * 1024, UFS_PART_3}, /* hitest_hipp        100MB p99 */
    {PART_HIBENCH_LPM3,      16522 * 1024,     2 * 1024, UFS_PART_3}, /* hibench_lpm3         2MB p100 */
    {PART_JPEG_DEC_TEST_SRC, 16524 * 1024,    20 * 1024, UFS_PART_3}, /* jpeg_dec_test_src   20MB p101 */
    {PART_JPEG_DEC_TEST_DEST, 16544 * 1024,    80 * 1024, UFS_PART_3}, /* jpeg_dec_test_dest  80MB p102 */
    {PART_VCODE_DATA,        16624 * 1024,   130 * 1024, UFS_PART_3}, /* vcode_data         130MB p103 */
    {PART_VCODE_MAX_DATA,    16754 * 1024,   180 * 1024, UFS_PART_3}, /* vcode_max_data     180MB p104 */
    {PART_USERDATA,          16934 * 1024,(16UL) * 1024 * 1024, UFS_PART_3}, /* userdata        6G p109 */
#else
    {PART_USERDATA,          14932 * 1024,(16UL) * 1024 * 1024, UFS_PART_3}, /* userdata        6G p87 */
#endif
#endif
    {"0", 0, 0, 0},
};

#endif
