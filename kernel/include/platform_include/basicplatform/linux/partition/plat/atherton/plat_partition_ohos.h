/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2020. All rights reserved.
 * Description: the partition table.
 */

#ifndef _SCORPIO_PARTITION_H_
#define _SCORPIO_PARTITION_H_

#include "partition_macro.h"
#include "partition_macro_plus.h"
#include "partition_def.h"

static const struct partition partition_table_emmc[] = {
    {PART_XLOADER,                0,             2 * 1024,    EMMC_BOOT_MAJOR_PART},
    {PART_RESERVED0,              0,             2 * 1024,    EMMC_BOOT_BACKUP_PART},
    {PART_PTABLE,                 0,                  512,    EMMC_USER_PART}, /* ptable           512K    p0 */
    {PART_FRP,                    512,                512,    EMMC_USER_PART}, /* frp              512K    p1 */
    {PART_PERSIST,                1024,          6 * 1024,    EMMC_USER_PART}, /* persist            6M    p2 */
    {PART_RESERVED1,              7 * 1024,          1024,    EMMC_USER_PART}, /* reserved1       1024K    p3 */
    {PART_RESERVED6,              8 * 1024,           512,    EMMC_USER_PART}, /* reserved6        512K    p4 */
    {PART_VRL,                    8704,               512,    EMMC_USER_PART}, /* vrl              512K    p5 */
    {PART_VRL_BACKUP,             9 * 1024,           512,    EMMC_USER_PART}, /* vrl backup       512K    p6 */
    {PART_MODEM_SECURE,           9728,              8704,    EMMC_USER_PART}, /* modem_secure    8704K    p7 */
    {PART_NVME,                   18 * 1024,     5 * 1024,    EMMC_USER_PART}, /* nvme               5M    p8 */
    {PART_CTF,                    23 * 1024,     1 * 1024,    EMMC_USER_PART}, /* ctf                1M    p9 */
    {PART_OEMINFO,                24 * 1024,    96 * 1024,    EMMC_USER_PART}, /* oeminfo           96M    p10 */
    {PART_SECURE_STORAGE,         120 * 1024,   32 * 1024,    EMMC_USER_PART}, /* secure storage    32M    p11 */
    {PART_MODEMNVM_FACTORY,       152 * 1024,   16 * 1024,    EMMC_USER_PART}, /* modemnvm factory  16M    p12 */
    {PART_MODEMNVM_BACKUP,        168 * 1024,   16 * 1024,    EMMC_USER_PART}, /* modemnvm backup   16M    p13 */
    {PART_MODEMNVM_IMG,           184 * 1024,   46 * 1024,    EMMC_USER_PART}, /* modemnvm img      46M    p14 */
    {PART_HISEE_ENCOS,            230 * 1024,    4 * 1024,    EMMC_USER_PART}, /* hisee_encos        4M    p15 */
    {PART_VERITYKEY,              234 * 1024,    1 * 1024,    EMMC_USER_PART}, /* veritykey          1M    p16 */
    {PART_DDR_PARA,               235 * 1024,    1 * 1024,    EMMC_USER_PART}, /* DDR_PARA           1M    p17 */
    {PART_LOWPOWER_PARA,          236 * 1024,    1 * 1024,    EMMC_USER_PART}, /* lowpower_para      1M    p18 */
    {PART_BATT_TP_PARA,           237 * 1024,    1 * 1024,    EMMC_USER_PART}, /* batt_tp_para       1M    p19 */
    {PART_RESERVED2,              238 * 1024,   21 * 1024,    EMMC_USER_PART}, /* reserved2         21M    p20 */
    {PART_LOG,                    259 * 1024,   80 * 1024,    EMMC_USER_PART}, /* splash2           80M    p21 */
    {PART_BOOTFAIL_INFO,          339 * 1024,    2 * 1024,    EMMC_USER_PART}, /* bootfail info      2M    p22 */
    {PART_MISC,                   341 * 1024,    2 * 1024,    EMMC_USER_PART}, /* misc               2M    p23 */
    {PART_DFX,                    343 * 1024,   16 * 1024,    EMMC_USER_PART}, /* dfx               16M    p24 */
    {PART_RRECORD,                359 * 1024,   16 * 1024,    EMMC_USER_PART}, /* rrecord           16M    p25 */
    {PART_CACHE,                  375 * 1024,  104 * 1024,    EMMC_USER_PART}, /* cache            104M    p26 */
    {PART_METADATA,               479 * 1024,   16 * 1024,    EMMC_USER_PART}, /* metadata          16M    p27 */
    {PART_RESERVED3,              495 * 1024,    4 * 1024,    EMMC_USER_PART}, /* reserved3A         4M    p28 */
    {PART_TOC,                    499 * 1024,    1 * 1024,    EMMC_USER_PART}, /* toc                1M    p29 */
    {PART_BL2,                    500 * 1024,    4 * 1024,    EMMC_USER_PART}, /* bl2                4M    p30 */
    {PART_FW_LPM3,                504 * 1024,         512,    EMMC_USER_PART}, /* fw_lpm3          512K    p31 */
    {PART_FW_CPU_LPCTRL,       (504 * 1024 + 512),    256,    EMMC_USER_PART}, /* fw_cpu_lpctrl    256K    p32 */
    {PART_FW_GPU_LPCTRL,       (504 * 1024 + 768),    128,    EMMC_USER_PART}, /* fw_gpu_lpctrl    128K    p33 */
    {PART_FW_DDR_LPCTRL,       (504 * 1024 + 896),    128,    EMMC_USER_PART}, /* fw_ddr_lpctrl    128K    p34 */
    {PART_NPU,                    505 * 1024,    8 * 1024,    EMMC_USER_PART}, /* npu                8M    p35 */
    {PART_IVP,                    513 * 1024,    2 * 1024,    EMMC_USER_PART}, /* ivp                2M    p36 */
    {PART_HDCP,                   515 * 1024,    1 * 1024,    EMMC_USER_PART}, /* PART_HDCP          1M    p37 */
    {PART_HISEE_IMG,              516 * 1024,    4 * 1024,    EMMC_USER_PART}, /* part_hisee_img     4M    p38 */
    {PART_HHEE,                   520 * 1024,    4 * 1024,    EMMC_USER_PART}, /* hhee               4M    p39 */
    {PART_HISEE_FS,               524 * 1024,    8 * 1024,    EMMC_USER_PART}, /* hisee_fs           8M    p40 */
    {PART_FASTBOOT,               532 * 1024,   12 * 1024,    EMMC_USER_PART}, /* fastboot          12M    p41 */
    {PART_VECTOR,                 544 * 1024,    4 * 1024,    EMMC_USER_PART}, /* vector             4M    p42 */
    {PART_ISP_BOOT,               548 * 1024,    2 * 1024,    EMMC_USER_PART}, /* isp_boot           2M    p43 */
    {PART_ISP_FIRMWARE,           550 * 1024,   14 * 1024,    EMMC_USER_PART}, /* isp_firmware      14M    p44 */
    {PART_FW_HIFI,                564 * 1024,   12 * 1024,    EMMC_USER_PART}, /* hifi              12M    p45 */
    {PART_TEEOS,                  576 * 1024,    8 * 1024,    EMMC_USER_PART}, /* teeos              8M    p46 */
    {PART_SENSORHUB,              584 * 1024,   16 * 1024,    EMMC_USER_PART}, /* sensorhub         16M    p47 */
#ifdef CONFIG_SANITIZER_ENABLE
    {PART_ERECOVERY_RAMDISK,      600 * 1024,   12 * 1024,    EMMC_USER_PART}, /* erecovery_ramdisk 12M    p48 */
    {PART_ERECOVERY_VENDOR,       612 * 1024,    8 * 1024,    EMMC_USER_PART}, /* erecovery_vendor   8M    p49 */
    {PART_BOOT,                   620 * 1024,   65 * 1024,    EMMC_USER_PART}, /* boot              65M    p50 */
    {PART_RECOVERY,               685 * 1024,   85 * 1024,    EMMC_USER_PART}, /* recovery          85M    p51 */
    {PART_ERECOVERY,              770 * 1024,   12 * 1024,    EMMC_USER_PART}, /* erecovery         12M    p52 */
    {PART_RESERVED,               782 * 1024,   49 * 1024,    EMMC_USER_PART}, /* reserved          49M    p53 */
#else
    {PART_ERECOVERY_RAMDISK,      600 * 1024,   32 * 1024,    EMMC_USER_PART}, /* erecovery_ramdisk 32M    p48 */
    {PART_ERECOVERY_VENDOR,       632 * 1024,   24 * 1024,    EMMC_USER_PART}, /* erecovery_vendor  24M    p49 */
    {PART_BOOT,                   656 * 1024,   30 * 1024,    EMMC_USER_PART}, /* boot              30M    p50 */
    {PART_RECOVERY,               686 * 1024,   45 * 1024,    EMMC_USER_PART}, /* recovery          45M    p51 */
    {PART_ERECOVERY,              731 * 1024,   45 * 1024,    EMMC_USER_PART}, /* erecovery         45M    p52 */
    {PART_RESERVED,               776 * 1024,   55 * 1024,    EMMC_USER_PART}, /* reserved          55M    p53 */
#endif
    {PART_THEE,                   831 * 1024,    4 * 1024,    EMMC_USER_PART}, /* thee               4M    p54 */
    {PART_TZSP,                   835 * 1024,   12 * 1024,    EMMC_USER_PART}, /* tzsp              12M    p55 */
    {PART_RECOVERY_RAMDISK,       847 * 1024,   32 * 1024,    EMMC_USER_PART}, /* recovery_ramdisk  32M    p56 */
    {PART_RECOVERY_VENDOR,        879 * 1024,   24 * 1024,    EMMC_USER_PART}, /* recovery_vendor   24M    p57 */
    {PART_ENG_SYSTEM,             903 * 1024,   12 * 1024,    EMMC_USER_PART}, /* eng_system        12M    p58 */
    {PART_ENG_VENDOR,             915 * 1024,   20 * 1024,    EMMC_USER_PART}, /* eng_vendor        20M    p59 */
    {PART_FW_DTB,                 935 * 1024,    2 * 1024,    EMMC_USER_PART}, /* fw_dtb             2M    p60 */
    {PART_DTBO,                   937 * 1024,   20 * 1024,    EMMC_USER_PART}, /* dtoimage          20M    p61 */
    {PART_TRUSTFIRMWARE,          957 * 1024,    2 * 1024,    EMMC_USER_PART}, /* trustfirmware      2M    p62 */
    {PART_MODEM_FW,               959 * 1024,  124 * 1024,    EMMC_USER_PART}, /* modem_fw         124M    p63 */
    {PART_MODEM_VENDOR,           1083 * 1024,  10 * 1024,    EMMC_USER_PART}, /* modem_vendor     10M     p64 */
    {PART_MODEM_PATCH_NV,        1093 * 1024,    4 * 1024,    EMMC_USER_PART}, /* modem_patch_nv     4M    p65 */
    {PART_MODEM_DRIVER,          1097 * 1024,   20 * 1024,    EMMC_USER_PART}, /* modem_driver      20M    p66 */
    {PART_MODEMNVM_UPDATE,       1117 * 1024,   16 * 1024,    EMMC_USER_PART}, /* modemnvm_update   16M    p67 */
    {PART_MODEMNVM_CUST,         1133 * 1024,   14 * 1024,    EMMC_USER_PART}, /* modemnvm_cust     14M    p68 */
    {PART_RAMDISK,               1147 * 1024,    4 * 1024,    EMMC_USER_PART}, /* ramdisk            4M    p69 */
    {PART_VBMETA_SYSTEM,         1151 * 1024,    1 * 1024,    EMMC_USER_PART}, /* vbmeta_system      1M    p70 */
    {PART_VBMETA_VENDOR,         1152 * 1024,    1 * 1024,    EMMC_USER_PART}, /* vbmeta_vendor      1M    p71 */
    {PART_VBMETA_ODM,            1153 * 1024,    1 * 1024,    EMMC_USER_PART}, /* vbmeta_odm         1M    p72 */
    {PART_VBMETA_CUST,           1154 * 1024,    1 * 1024,    EMMC_USER_PART}, /* vbmeta_cust        1M    p73 */
    {PART_VBMETA_HW_PRODUCT,     1155 * 1024,    1 * 1024,    EMMC_USER_PART}, /* vbmeta_hw_product  1M    p74 */
    {PART_RECOVERY_VBMETA,       1156 * 1024,    2 * 1024,    EMMC_USER_PART}, /* recovery_vbmeta    2M    p75 */
    {PART_ERECOVERY_VBMETA,      1158 * 1024,    2 * 1024,    EMMC_USER_PART}, /* erecovery_vbmeta   2M    p76 */
    {PART_VBMETA,                1160 * 1024,    4 * 1024,    EMMC_USER_PART}, /* vbmeta             4M    p77 */
    {PART_KPATCH,                1164 * 1024,    4 * 1024,    EMMC_USER_PART}, /* kpatch             4M    p78 */
    {PART_PATCH,                 1168 * 1024,   32 * 1024,    EMMC_USER_PART}, /* patch             32M    p79 */
#ifdef CONFIG_FACTORY_MODE
    {PART_PREAS,                 1200 * 1024,   32 * 1024,    EMMC_USER_PART}, /* preas            368M    p80 */
    {PART_PREAVS,                1232 * 1024,   32 * 1024,    EMMC_USER_PART}, /* preavs            32M    p81 */
    {PART_SUPER,                 1264 * 1024, 7656 * 1024,    EMMC_USER_PART}, /* super           9656M    p82 */
    {PART_VERSION,               8920 * 1024,   32 * 1024,    EMMC_USER_PART}, /* version          576M    p83 */
    {PART_PRELOAD,               8952 * 1024,   32 * 1024,    EMMC_USER_PART}, /* preload         1144M    p84 */
    {PART_HIBENCH_IMG,           8984 * 1024,  128 * 1024,    EMMC_USER_PART}, /* hibench_img      128M    p85 */
    {PART_HIBENCH_DATA,          9112 * 1024,  512 * 1024,    EMMC_USER_PART}, /* hibench_data     512M    p86 */
    {PART_FLASH_AGEING,          9624 * 1024,  512 * 1024,    EMMC_USER_PART}, /* FLASH_AGEING     512M    p87 */
    {PART_HIBENCH_LOG,          10136 * 1024,   32 * 1024,    EMMC_USER_PART}, /* HIBENCH_LOG       32M    p88 */
    {PART_USERDATA,             10168 * 1024, (4UL) * 1024 * 1024,    EMMC_USER_PART}, /* userdata   4G    p89 */
#else
    {PART_PREAS,                 1200 * 1024,   32 * 1024,    EMMC_USER_PART}, /* preas           1280M    p80 */
    {PART_PREAVS,                1232 * 1024,   32 * 1024,    EMMC_USER_PART}, /* preavs            32M    p81 */
    {PART_CHIP_PROD,             1264 * 1024,  616 * 1024,    EMMC_USER_PART}, /* super           616M    p82 */
    {PART_VENDOR,                1880 * 1024,  920 * 1024,    EMMC_USER_PART}, /* super           920M    p82 */
    {PART_SYS_PROD,              2800 * 1024, 1936 * 1024,    EMMC_USER_PART}, /* super           1936M    p82 */
    {PART_SYSTEM,                4736 * 1024, 4184 * 1024,    EMMC_USER_PART}, /* super           4184M    p82 */
    {PART_VERSION,               8920 * 1024,   32 * 1024,    EMMC_USER_PART}, /* version          576M    p83 */
    {PART_PRELOAD,               8952 * 1024,   32 * 1024,    EMMC_USER_PART}, /* preload         1144M    p84 */
    {PART_USERDATA,              8984 * 1024, (4UL) * 1024 * 1024,    EMMC_USER_PART}, /* userdata   4G    p85 */
#endif
    {"0", 0, 0, 0},
};

static const struct partition partition_table_ufs[] = {
    {PART_XLOADER,                0,              2 * 1024,    UFS_PART_0},
    {PART_RESERVED0,              0,              2 * 1024,    UFS_PART_1},
    {PART_PTABLE,                 0,                   512,    UFS_PART_2}, /* ptable           512K    p0 */
    {PART_FRP,                    512,                 512,    UFS_PART_2}, /* frp              512K    p1 */
    {PART_PERSIST,                1 * 1024,       6 * 1024,    UFS_PART_2}, /* persist         6144K    p2 */
    {PART_RESERVED1,              7 * 1024,           1024,    UFS_PART_2}, /* reserved1       1024K    p3 */
    {PART_PTABLE_LU3,             0,                   512,    UFS_PART_3}, /* ptable_lu3       512K    p0 */
    {PART_VRL,                    512,                 512,    UFS_PART_3}, /* vrl              512K    p1 */
    {PART_VRL_BACKUP,             1024,                512,    UFS_PART_3}, /* vrl backup       512K    p2 */
    {PART_MODEM_SECURE,           1536,               8704,    UFS_PART_3}, /* modem_secure    8704K    p3 */
    {PART_NVME,                   10 * 1024,      5 * 1024,    UFS_PART_3}, /* nvme               5M    p4 */
    {PART_CTF,                    15 * 1024,      1 * 1024,    UFS_PART_3}, /* PART_CTF           1M    p5 */
    {PART_OEMINFO,                16 * 1024,     96 * 1024,    UFS_PART_3}, /* oeminfo           96M    p6 */
    {PART_SECURE_STORAGE,         112 * 1024,    32 * 1024,    UFS_PART_3}, /* secure storage    32M    p7 */
    {PART_MODEMNVM_FACTORY,       144 * 1024,    16 * 1024,    UFS_PART_3}, /* modemnvm factory  16M    p8 */
    {PART_MODEMNVM_BACKUP,        160 * 1024,    16 * 1024,    UFS_PART_3}, /* modemnvm backup   16M    p9 */
    {PART_MODEMNVM_IMG,           176 * 1024,    46 * 1024,    UFS_PART_3}, /* modemnvm img      46M    p10 */
    {PART_HISEE_ENCOS,            222 * 1024,     4 * 1024,    UFS_PART_3}, /* hisee_encos        4M    p11 */
    {PART_VERITYKEY,              226 * 1024,     1 * 1024,    UFS_PART_3}, /* reserved2          1M    p12 */
    {PART_DDR_PARA,               227 * 1024,     1 * 1024,    UFS_PART_3}, /* DDR_PARA           1M    p13 */
    {PART_LOWPOWER_PARA,          228 * 1024,     1 * 1024,    UFS_PART_3}, /* lowpower_para      1M    p14 */
    {PART_BATT_TP_PARA,           229 * 1024,     1 * 1024,    UFS_PART_3}, /* batt_tp_para       1M    p15 */
    {PART_RESERVED2,              230 * 1024,    21 * 1024,    UFS_PART_3}, /* reserved2         21M    p16 */
    {PART_SPLASH2,                251 * 1024,    80 * 1024,    UFS_PART_3}, /* splash2           80M    p17 */
    {PART_BOOTFAIL_INFO,          331 * 1024,     2 * 1024,    UFS_PART_3}, /* bootfail info      2M    p18 */
    {PART_MISC,                   333 * 1024,     2 * 1024,    UFS_PART_3}, /* misc               2M    p19 */
    {PART_DFX,                    335 * 1024,    16 * 1024,    UFS_PART_3}, /* dfx               16M    p20 */
    {PART_RRECORD,                351 * 1024,    16 * 1024,    UFS_PART_3}, /* rrecord           16M    p21 */
    {PART_CACHE,                  367 * 1024,   104 * 1024,    UFS_PART_3}, /* cache            104M    p22 */
    {PART_METADATA,               471 * 1024,    16 * 1024,    UFS_PART_3}, /* metadata          16M    p23 */
    {PART_RESERVED3,              487 * 1024,     4 * 1024,    UFS_PART_3}, /* reserved3A         4M    p24 */
    {PART_TOC,                    491 * 1024,     1 * 1024,    UFS_PART_3}, /* toc                1M    p25 */
    {PART_BL2,                    492 * 1024,     4 * 1024,    UFS_PART_3}, /* bl2                4M    p26 */
    {PART_FW_LPM3,                496 * 1024,          512,    UFS_PART_3}, /* fw_lpm3          512K    p27 */
    {PART_FW_CPU_LPCTRL,         (496 * 1024 + 512),   256,    UFS_PART_3}, /* fw_cpu_lpctrl    256K    p28 */
    {PART_FW_GPU_LPCTRL,         (496 * 1024 + 768),   128,    UFS_PART_3}, /* fw_gpu_lpctrl    128K    p29 */
    {PART_FW_DDR_LPCTRL,         (496 * 1024 + 896),   128,    UFS_PART_3}, /* fw_ddr_lpctrl    128K    p30 */
    {PART_NPU,                    497 * 1024,     8 * 1024,    UFS_PART_3}, /* npu                8M    p31 */
    {PART_IVP,                    505 * 1024,     2 * 1024,    UFS_PART_3}, /* ivp                2M    p32 */
    {PART_HDCP,                   507 * 1024,     1 * 1024,    UFS_PART_3}, /* PART_HDCP          1M    p33 */
    {PART_HISEE_IMG,              508 * 1024,     4 * 1024,    UFS_PART_3}, /* part_hisee_img     4M    p34 */
    {PART_HHEE,                   512 * 1024,     4 * 1024,    UFS_PART_3}, /* hhee               4M    p35 */
    {PART_HISEE_FS,               516 * 1024,     8 * 1024,    UFS_PART_3}, /* hisee_fs           8M    p36 */
    {PART_FASTBOOT,               524 * 1024,    12 * 1024,    UFS_PART_3}, /* fastboot          12M    p37 */
    {PART_VECTOR,                 536 * 1024,     4 * 1024,    UFS_PART_3}, /* vector             4M    p38 */
    {PART_ISP_BOOT,               540 * 1024,     2 * 1024,    UFS_PART_3}, /* isp_boot           2M    p39 */
    {PART_ISP_FIRMWARE,           542 * 1024,    14 * 1024,    UFS_PART_3}, /* isp_firmware      14M    p40 */
    {PART_FW_HIFI,                556 * 1024,    12 * 1024,    UFS_PART_3}, /* hifi              12M    p41 */
    {PART_TEEOS,                  568 * 1024,     8 * 1024,    UFS_PART_3}, /* teeos              8M    p42 */
    {PART_SENSORHUB,              576 * 1024,    16 * 1024,    UFS_PART_3}, /* sensorhub         16M    p43 */
#ifdef CONFIG_SANITIZER_ENABLE
    {PART_ERECOVERY_RAMDISK,      592 * 1024,    12 * 1024,    UFS_PART_3}, /* erecovery_ramdisk 12M    p44 */
    {PART_ERECOVERY_VENDOR,       604 * 1024,     8 * 1024,    UFS_PART_3}, /* erecovery_vendor   8M    p45 */
    {PART_BOOT,                   612 * 1024,    65 * 1024,    UFS_PART_3}, /* boot              65M    p46 */
    {PART_RECOVERY,               677 * 1024,    85 * 1024,    UFS_PART_3}, /* recovery          85M    p47 */
    {PART_ERECOVERY,              762 * 1024,    12 * 1024,    UFS_PART_3}, /* erecovery         12M    p48 */
    {PART_RESERVED,               774 * 1024,    49 * 1024,    UFS_PART_3}, /* reserved          49M    p49 */
#else
    {PART_ERECOVERY_RAMDISK,      592 * 1024,    32 * 1024,    UFS_PART_3}, /* erecovery_ramdisk 32M    p44 */
    {PART_ERECOVERY_VENDOR,       624 * 1024,    24 * 1024,    UFS_PART_3}, /* erecovery_vendor  24M    p45 */
    {PART_BOOT,                   648 * 1024,    30 * 1024,    UFS_PART_3}, /* boot              30M    p46 */
    {PART_RECOVERY,               678 * 1024,    45 * 1024,    UFS_PART_3}, /* recovery          45M    p47 */
    {PART_ERECOVERY,              723 * 1024,    45 * 1024,    UFS_PART_3}, /* erecovery         45M    p48 */
    {PART_RESERVED,               768 * 1024,    55 * 1024,    UFS_PART_3}, /* reserved          55M    p49 */
#endif
    {PART_THEE,                   823 * 1024,     4 * 1024,    UFS_PART_3}, /* thee               4M    p50 */
    {PART_TZSP,                   827 * 1024,    12 * 1024,    UFS_PART_3}, /* tzsp              12M    p51 */
    {PART_RECOVERY_RAMDISK,       839 * 1024,    32 * 1024,    UFS_PART_3}, /* recovery_ramdisk  32M    p52 */
    {PART_RECOVERY_VENDOR,        871 * 1024,    24 * 1024,    UFS_PART_3}, /* recovery_vendor   24M    p53 */
    {PART_ENG_SYSTEM,             895 * 1024,    12 * 1024,    UFS_PART_3}, /* eng_system        12M    p54 */
    {PART_ENG_VENDOR,             907 * 1024,    20 * 1024,    UFS_PART_3}, /* eng_vendor        20M    p55 */
    {PART_FW_DTB,                 927 * 1024,     2 * 1024,    UFS_PART_3}, /* fw_dtb             2M    p56 */
    {PART_DTBO,                   929 * 1024,    20 * 1024,    UFS_PART_3}, /* dtoimage          20M    p57 */
    {PART_TRUSTFIRMWARE,          949 * 1024,     2 * 1024,    UFS_PART_3}, /* trustfirmware      2M    p58 */
    {PART_MODEM_FW,               951 * 1024,   124 * 1024,    UFS_PART_3}, /* modem_fw         124M    p59 */
    {PART_MODEM_VENDOR,          1075 * 1024,    10 * 1024,    UFS_PART_3}, /* modem_vendor      10M    p60 */
    {PART_MODEM_PATCH_NV,        1085 * 1024,     4 * 1024,    UFS_PART_3}, /* modem_patch_nv     4M    p61 */
    {PART_MODEM_DRIVER,          1089 * 1024,    20 * 1024,    UFS_PART_3}, /* modem_driver      20M    p62 */
    {PART_MODEMNVM_UPDATE,       1109 * 1024,    16 * 1024,    UFS_PART_3}, /* modemnvm_update   16M    p63 */
    {PART_MODEMNVM_CUST,         1125 * 1024,    16 * 1024,    UFS_PART_3}, /* modemnvm_cust     16M    p64 */
    {PART_RAMDISK,               1141 * 1024,     2 * 1024,    UFS_PART_3}, /* ramdisk            2M    p65 */
    {PART_VBMETA_SYSTEM,         1143 * 1024,     1 * 1024,    UFS_PART_3}, /* vbmeta_system      1M    p66 */
    {PART_VBMETA_VENDOR,         1144 * 1024,     1 * 1024,    UFS_PART_3}, /* vbmeta_vendor      1M    p67 */
    {PART_VBMETA_ODM,            1145 * 1024,     1 * 1024,    UFS_PART_3}, /* vbmeta_odm         1M    p68 */
    {PART_VBMETA_CUST,           1146 * 1024,     1 * 1024,    UFS_PART_3}, /* vbmeta_cust        1M    p69 */
    {PART_VBMETA_HW_PRODUCT,     1147 * 1024,     1 * 1024,    UFS_PART_3}, /* vbmeta_hw_product  1M    p70 */
    {PART_RECOVERY_VBMETA,       1148 * 1024,     2 * 1024,    UFS_PART_3}, /* recovery_vbmeta    2M    p71 */
    {PART_ERECOVERY_VBMETA,      1150 * 1024,     2 * 1024,    UFS_PART_3}, /* erecovery_vbmeta   2M    p72 */
    {PART_VBMETA,                1152 * 1024,     4 * 1024,    UFS_PART_3}, /* vbmeta             4M    p73 */
    {PART_KPATCH,                1156 * 1024,     4 * 1024,    UFS_PART_3}, /* kpatch             4M    p74 */
    {PART_PATCH,                 1160 * 1024,    32 * 1024,    UFS_PART_3}, /* patch             32M    p75 */
#ifdef CONFIG_FACTORY_MODE
    {PART_PREAS,                 1192 * 1024,    32 * 1024,    UFS_PART_3}, /* preas            368M    p76 */
    {PART_PREAVS,                1224 * 1024,    32 * 1024,    UFS_PART_3}, /* preavs            32M    p77 */
    {PART_SUPER,                 1256 * 1024,  7656 * 1024,    UFS_PART_3}, /* super           9656M    p78 */
    {PART_VERSION,               8912 * 1024,    32 * 1024,    UFS_PART_3}, /* version          576M    p79 */
    {PART_PRELOAD,               8944 * 1024,    32 * 1024,    UFS_PART_3}, /* preload         1144M    p80 */
    {PART_HIBENCH_IMG,           8976 * 1024,   128 * 1024,    UFS_PART_3}, /* hibench_img      128M    p81 */
    {PART_HIBENCH_DATA,          9104 * 1024,   512 * 1024,    UFS_PART_3}, /* hibench_data     512M    p82 */
    {PART_FLASH_AGEING,          9616 * 1024,   512 * 1024,    UFS_PART_3}, /* FLASH_AGEING     512M    p83 */
    {PART_HIBENCH_LOG,          10128 * 1024,    32 * 1024,    UFS_PART_3}, /* HIBENCH_LOG       32M    p84 */
    {PART_USERDATA,             10160 * 1024,  (4UL) * 1024 * 1024,   UFS_PART_3}, /* userdata    4G    p85 */
#else
    {PART_PREAS,                 1192 * 1024,    32 * 1024,    UFS_PART_3}, /* preas           1280M    p76 */
    {PART_PREAVS,                1224 * 1024,    32 * 1024,    UFS_PART_3}, /* preavs            32M    p77 */
    {PART_SUPER,                 1256 * 1024,  7656 * 1024,    UFS_PART_3}, /* super           9872M    p78 */
    {PART_VERSION,               8912 * 1024,    32 * 1024,    UFS_PART_3}, /* version          576M    p79 */
    {PART_PRELOAD,               8944 * 1024,    32 * 1024,    UFS_PART_3}, /* preload         1144M    p80 */
    {PART_USERDATA,              8976 * 1024,  (4UL) * 1024 * 1024,   UFS_PART_3}, /* userdata    4G    p81 */
#endif
    {PART_PTABLE_LU4,                      0,             512,    UFS_PART_4}, /* ptable_lu4       512K    p0 */
    {PART_RESERVED12,                    512,            1536,    UFS_PART_4}, /* reserved12      1536K    p1 */
    {PART_USERDATA2,                    2048, (4UL) * 1024 * 1024,    UFS_PART_4}, /* userdata2      4G    p2 */
    {"0", 0, 0, 0},
};

#endif
