/*
 *  Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 *  Description: the partition table.
 */

#ifndef _SAGITTARIUS_PARTITION_H_
#define _SAGITTARIUS_PARTITION_H_

#include "partition_macro.h"
#include "partition_def.h"

static const struct partition partition_table_emmc[] = {
    {"0", 0, 0, 0},
};

static const struct partition partition_table_ufs[] = {
    {PART_XLOADER,                        0,      2 * 1024,    UFS_PART_0},
    {PART_RESERVED0,                      0,      2 * 1024,    UFS_PART_1},
    {PART_PTABLE,                         0,           512,    UFS_PART_2}, /* ptable           512K     */
    {PART_FRP,                        512,          512, UFS_PART_2}, /* frp                512KB p1 */
    {PART_PERSIST,                   1024,     6 * 1024, UFS_PART_2}, /* persist              6MB p2 */
    {PART_RESERVED1,             7 * 1024,         1024, UFS_PART_2}, /* reserved1         1024KB p3 */
    {PART_PTABLE_LU3,                     0,           512,    UFS_PART_3}, /* ptable_lu3       512K    p0 */
    {PART_VRL,                        512,          512, UFS_PART_3}, /* vrl                512KB p1 */
    {PART_VRL_BACKUP,                1024,          512, UFS_PART_3}, /* vrl_backup         512KB p2 */
    {PART_MODEM_SECURE,              1536,         8704, UFS_PART_3}, /* modem_secure      8704KB p3 */
    {PART_NVME,                 10 * 1024,     5 * 1024, UFS_PART_3}, /* nvme                 5MB p4 */
    {PART_CTF,                  15 * 1024,     1 * 1024, UFS_PART_3}, /* ctf               1024KB p5 */
    {PART_OEMINFO,              16 * 1024,    96 * 1024, UFS_PART_3}, /* oeminfo             96MB p6 */
    {PART_SECURE_STORAGE,      112 * 1024,    32 * 1024, UFS_PART_3}, /* secure_storage      32MB p7 */
    {PART_MODEMNVM_FACTORY,    144 * 1024,    16 * 1024, UFS_PART_3}, /* modemnvm_factory    16MB p8 */
    {PART_MODEMNVM_BACKUP,     160 * 1024,    16 * 1024, UFS_PART_3}, /* modemnvm_backup     16MB p9 */
    {PART_MODEMNVM_IMG,        176 * 1024,    46 * 1024, UFS_PART_3}, /* modemnvm_img        46MB p10 */
    {PART_VERITYKEY,           222 * 1024,     1 * 1024, UFS_PART_3}, /* veritykey         1024KB p11 */
    {PART_DDR_PARA,            223 * 1024,     1 * 1024, UFS_PART_3}, /* ddr_para          1024KB p12 */
    {PART_LOWPOWER_PARA,       224 * 1024,     1 * 1024, UFS_PART_3}, /* lowpower_para     1024KB p13 */
    {PART_BATT_TP_PARA,        225 * 1024,     1 * 1024, UFS_PART_3}, /* batt_tp_para      1024KB p14 */
    {PART_BL2,                 226 * 1024,     4 * 1024, UFS_PART_3}, /* bl2                  4MB p15 */
    {PART_RESERVED2,           230 * 1024,    25 * 1024, UFS_PART_3}, /* reserved2           25MB p16 */
    {PART_SPLASH2,             255 * 1024,    80 * 1024, UFS_PART_3}, /* splash2             80MB p17 */
    {PART_BOOTFAIL_INFO,       335 * 1024,     2 * 1024, UFS_PART_3}, /* bootfail_info        2MB p18 */
    {PART_MISC,                337 * 1024,     2 * 1024, UFS_PART_3}, /* misc                 2MB p19 */
    {PART_DFX,                 339 * 1024,    16 * 1024, UFS_PART_3}, /* dfx                 16MB p20 */
    {PART_RRECORD,             355 * 1024,    16 * 1024, UFS_PART_3}, /* rrecord             16MB p21 */
    {PART_CACHE,               371 * 1024,   104 * 1024, UFS_PART_3}, /* cache              104MB p22 */
    {PART_METADATA,            475 * 1024,    16 * 1024, UFS_PART_3}, /* metadata            16MB p23 */
    {PART_RESERVED3,           491 * 1024,     4 * 1024, UFS_PART_3}, /* reserved3            4MB p24 */
    {PART_TOC,                 495 * 1024,     1 * 1024, UFS_PART_3}, /* toc               1024KB p25 */
    {PART_FW_LPM3,             496 * 1024,     1 * 1024, UFS_PART_3}, /* fw_lpm3           1024KB p26 */
    {PART_NPU,                 497 * 1024,     8 * 1024, UFS_PART_3}, /* npu                  8MB p27 */
    {PART_IVP,                 505 * 1024,     2 * 1024, UFS_PART_3}, /* ivp                  2MB p28 */
    {PART_HDCP,                507 * 1024,     1 * 1024, UFS_PART_3}, /* hdcp              1024KB p29 */
    {PART_HHEE,                508 * 1024,     4 * 1024, UFS_PART_3}, /* hhee                 4MB p30 */
    {PART_FASTBOOT,            512 * 1024,    12 * 1024, UFS_PART_3}, /* fastboot            12MB p31 */
    {PART_VECTOR,              524 * 1024,     4 * 1024, UFS_PART_3}, /* vector               4MB p32 */
    {PART_ISP_BOOT,            528 * 1024,     2 * 1024, UFS_PART_3}, /* isp_boot             2MB p33 */
    {PART_ISP_FIRMWARE,        530 * 1024,    14 * 1024, UFS_PART_3}, /* isp_firmware        14MB p34 */
    {PART_FW_HIFI,             544 * 1024,    12 * 1024, UFS_PART_3}, /* fw_hifi             12MB p35 */
    {PART_TEEOS,               556 * 1024,     8 * 1024, UFS_PART_3}, /* teeos                8MB p36 */
    {PART_SENSORHUB,           564 * 1024,    16 * 1024, UFS_PART_3}, /* sensorhub           16MB p37 */
    {PART_HKRR,                580 * 1024,    16 * 1024, UFS_PART_3}, /* hkrr                16MB p38 */
    {PART_FW_UFSDEV,           596 * 1024,     4 * 1024, UFS_PART_3}, /* fw_ufsdev            4MB p39 */
    {PART_RESERVED4,           600 * 1024,    24 * 1024, UFS_PART_3}, /* reserved4           24MB p40 */
#ifdef CONFIG_SANITIZER_ENABLE
    {PART_ERECOVERY_RAMDISK,   624 * 1024,    12 * 1024, UFS_PART_3}, /* erecovery_ramdisk   12MB p41 */
    {PART_ERECOVERY_VENDOR,    636 * 1024,     8 * 1024, UFS_PART_3}, /* erecovery_vendor     8MB p42 */
    {PART_BOOT,                644 * 1024,    81 * 1024, UFS_PART_3}, /* boot                81MB p43 */
    {PART_RECOVERY,            725 * 1024,    85 * 1024, UFS_PART_3}, /* recovery            85MB p44 */
    {PART_ERECOVERY,           810 * 1024,    12 * 1024, UFS_PART_3}, /* erecovery           12MB p45 */
    {PART_RECOVERY_RAMDISK,    822 * 1024,    32 * 1024, UFS_PART_3}, /* recovery_ramdisk    32MB p46 */
    {PART_RECOVERY_VENDOR,     854 * 1024,    24 * 1024, UFS_PART_3}, /* recovery_vendor     24MB p47 */
    {PART_RESERVED,            878 * 1024,    49 * 1024, UFS_PART_3}, /* reserved            13MB p48 */
#elif defined CONFIG_HWSANITIZER_ENABLE
    {PART_ERECOVERY_RAMDISK,   624 * 1024,    40 * 1024, UFS_PART_3}, /* erecovery_ramdisk   32MB p41 */
    {PART_ERECOVERY_VENDOR,    664 * 1024,    24 * 1024, UFS_PART_3}, /* erecovery_vendor    24MB p42 */
    {PART_BOOT,                688 * 1024,    46 * 1024, UFS_PART_3}, /* boot                46MB p43 */
    {PART_RECOVERY,            734 * 1024,    45 * 1024, UFS_PART_3}, /* recovery            45MB p44 */
    {PART_ERECOVERY,           779 * 1024,    45 * 1024, UFS_PART_3}, /* erecovery           45MB p45 */
    {PART_RECOVERY_RAMDISK,    824 * 1024,    40 * 1024, UFS_PART_3}, /* recovery_ramdisk    32MB p46 */
    {PART_RECOVERY_VENDOR,     864 * 1024,    24 * 1024, UFS_PART_3}, /* recovery_vendor     24MB p47 */
    {PART_RESERVED,            888 * 1024,    39 * 1024, UFS_PART_3}, /* reserved            55MB p48 */
#else
    {PART_ERECOVERY_RAMDISK,   624 * 1024,    32 * 1024, UFS_PART_3}, /* erecovery_ramdisk   32MB p41 */
    {PART_ERECOVERY_VENDOR,    656 * 1024,    24 * 1024, UFS_PART_3}, /* erecovery_vendor    24MB p42 */
    {PART_BOOT,                680 * 1024,    46 * 1024, UFS_PART_3}, /* boot                46MB p43 */
    {PART_RECOVERY,            726 * 1024,    45 * 1024, UFS_PART_3}, /* recovery            45MB p44 */
    {PART_ERECOVERY,           771 * 1024,    45 * 1024, UFS_PART_3}, /* erecovery           45MB p45 */
    {PART_RECOVERY_RAMDISK,    816 * 1024,    32 * 1024, UFS_PART_3}, /* recovery_ramdisk    32MB p46 */
    {PART_RECOVERY_VENDOR,     848 * 1024,    24 * 1024, UFS_PART_3}, /* recovery_vendor     24MB p47 */
    {PART_RESERVED,            872 * 1024,    55 * 1024, UFS_PART_3}, /* reserved            55MB p48 */
#endif
    {PART_ENG_SYSTEM,          927 * 1024,    12 * 1024, UFS_PART_3}, /* eng_system          12MB p49 */
    {PART_ENG_VENDOR,          939 * 1024,    20 * 1024, UFS_PART_3}, /* eng_vendor          20MB p50 */
    {PART_FW_DTB,              959 * 1024,     8 * 1024, UFS_PART_3}, /* fw_dtb               8MB p51 */
    {PART_DTBO,                967 * 1024,    14 * 1024, UFS_PART_3}, /* dtbo                14MB p52 */
    {PART_TRUSTFIRMWARE,       981 * 1024,     2 * 1024, UFS_PART_3}, /* trustfirmware        2MB p53 */
    {PART_MODEM_FW,            983 * 1024,   124 * 1024, UFS_PART_3}, /* modem_fw           124MB p54 */
    {PART_MODEM_VENDOR,       1107 * 1024,    10 * 1024, UFS_PART_3}, /* modem_vendor        10MB p55 */
    {PART_MODEM_PATCH_NV,     1117 * 1024,    16 * 1024, UFS_PART_3}, /* modem_patch_nv      16MB p56 */
    {PART_MODEM_DRIVER,       1133 * 1024,    20 * 1024, UFS_PART_3}, /* modem_driver        20MB p57 */
    {PART_MODEMNVM_UPDATE,    1153 * 1024,    16 * 1024, UFS_PART_3}, /* modemnvm_update     16MB p58 */
    {PART_MODEMNVM_CUST,      1169 * 1024,    16 * 1024, UFS_PART_3}, /* modemnvm_cust       16MB p59 */
    {PART_RAMDISK,            1185 * 1024,     2 * 1024, UFS_PART_3}, /* ramdisk              2MB p60 */
    {PART_VBMETA_SYSTEM,      1187 * 1024,     1 * 1024, UFS_PART_3}, /* vbmeta_system     1024KB p61 */
    {PART_VBMETA_VENDOR,      1188 * 1024,     1 * 1024, UFS_PART_3}, /* vbmeta_vendor     1024KB p62 */
    {PART_VBMETA_ODM,         1189 * 1024,     1 * 1024, UFS_PART_3}, /* vbmeta_odm        1024KB p63 */
    {PART_VBMETA_CUST,        1190 * 1024,     1 * 1024, UFS_PART_3}, /* vbmeta_cust       1024KB p64 */
    {PART_VBMETA_HW_PRODUCT,  1191 * 1024,     1 * 1024, UFS_PART_3}, /* vbmeta_hw_product 1024KB p65 */
    {PART_RECOVERY_VBMETA,    1192 * 1024,     2 * 1024, UFS_PART_3}, /* recovery_vbmeta      2MB p66 */
    {PART_ERECOVERY_VBMETA,   1194 * 1024,     2 * 1024, UFS_PART_3}, /* erecovery_vbmeta     2MB p67 */
    {PART_VBMETA,             1196 * 1024,     4 * 1024, UFS_PART_3}, /* vbmeta               4MB p68 */
    {PART_KPATCH,             1200 * 1024,     4 * 1024, UFS_PART_3}, /* kpatch               4MB p69 */
    {PART_PATCH,              1204 * 1024,    32 * 1024, UFS_PART_3}, /* patch               32MB p70 */
#ifdef CONFIG_FACTORY_MODE
    {PART_PREAS,              1236 * 1024,  1280 * 1024, UFS_PART_3}, /* preas             1280MB p71 */
    {PART_PREAVS,             2516 * 1024,    32 * 1024, UFS_PART_3}, /* preavs              32MB p72 */
    {PART_SUPER,              2548 * 1024,  8728 * 1024, UFS_PART_3}, /* super             8728MB p73 */
    {PART_VERSION,           11276 * 1024,   576 * 1024, UFS_PART_3}, /* version            576MB p74 */
    {PART_PRELOAD,           11852 * 1024,  1144 * 1024, UFS_PART_3}, /* preload           1144MB p75 */
    {PART_HIBENCH_IMG,       12996 * 1024,   128 * 1024, UFS_PART_3}, /* hibench_img        128MB p76 */
    {PART_HIBENCH_DATA,      13124 * 1024,   768 * 1024, UFS_PART_3}, /* hibench_data       768MB p77 */
    {PART_FLASH_AGEING,      13892 * 1024,   512 * 1024, UFS_PART_3}, /* flash_ageing       512MB p78 */
    {PART_HIBENCH_LOG,       14404 * 1024,    32 * 1024, UFS_PART_3}, /* hibench_log         32MB p79 */
    {PART_HIBENCH_LPM3,      14436 * 1024,    32 * 1024, UFS_PART_3}, /* hibench_lpm3        32MB p80 */
    {PART_USERDATA,          14468 * 1024,(8UL) * 1024 * 1024, UFS_PART_3}, /* userdata        8G p81 */
#else
#ifdef CONFIG_USE_EROFS
#ifdef CONFIG_MARKET_INTERNAL
    {PART_PREAS,              1236 * 1024,    12 * 1024, UFS_PART_3}, /* preas               12MB p71 */
    {PART_PREAVS,             1248 * 1024,    12 * 1024, UFS_PART_3}, /* preavs              12MB p72 */
    {PART_SUPER,              1260 * 1024,  8602 * 1024, UFS_PART_3}, /* super             8602MB p73 */
    {PART_VERSION,            9862 * 1024,   576 * 1024, UFS_PART_3}, /* version            576MB p74 */
    {PART_PRELOAD,           10438 * 1024,  1504 * 1024, UFS_PART_3}, /* preload           1504MB p75 */
    {PART_USERDATA,          11942 * 1024,(8UL) * 1024 * 1024, UFS_PART_3}, /* userdata        8G p76 */

#elif defined CONFIG_MARKET_OVERSEA
    {PART_PREAS,              1236 * 1024,    12 * 1024, UFS_PART_3}, /* preas               12MB p71 */
    {PART_PREAVS,             1248 * 1024,    12 * 1024, UFS_PART_3}, /* preavs              12MB p72 */
    {PART_SUPER,              1260 * 1024,  8050 * 1024, UFS_PART_3}, /* super             8050MB p73 */
    {PART_VERSION,            9310 * 1024,   576 * 1024, UFS_PART_3}, /* version            576MB p74 */
    {PART_PRELOAD,            9886 * 1024,  1144 * 1024, UFS_PART_3}, /* preload           1144MB p75 */
    {PART_USERDATA,          11030 * 1024,(8UL) * 1024 * 1024, UFS_PART_3}, /* userdata        8G p76 */
#elif defined CONFIG_MARKET_FULL_INTERNAL
    {PART_PREAS,              1236 * 1024,   296 * 1024, UFS_PART_3}, /* preas              296MB p71 */
    {PART_PREAVS,             1532 * 1024,    32 * 1024, UFS_PART_3}, /* preavs              32MB p72 */
    {PART_SUPER,              1564 * 1024,  6794 * 1024, UFS_PART_3}, /* super             6794MB p73 */
    {PART_VERSION,            8358 * 1024,   576 * 1024, UFS_PART_3}, /* version            576MB p74 */
    {PART_PRELOAD,            8934 * 1024,  1144 * 1024, UFS_PART_3}, /* preload           1144MB p75 */
    {PART_USERDATA,          10078 * 1024,(8UL) * 1024 * 1024, UFS_PART_3}, /* userdata        8G p76 */
#elif defined CONFIG_MARKET_FULL_OVERSEA
    {PART_PREAS,              1236 * 1024,  1024 * 1024, UFS_PART_3}, /* preas             1024 * 1024KB p71 */
    {PART_PREAVS,             2260 * 1024,    32 * 1024, UFS_PART_3}, /* preavs              32MB p72 */
    {PART_SUPER,              2292 * 1024,  6762 * 1024, UFS_PART_3}, /* super             6762MB p73 */
    {PART_VERSION,            9054 * 1024,   576 * 1024, UFS_PART_3}, /* version            576MB p74 */
    {PART_PRELOAD,            9630 * 1024,  1144 * 1024, UFS_PART_3}, /* preload           1144MB p75 */
    {PART_USERDATA,          10774 * 1024,(8UL) * 1024 * 1024, UFS_PART_3}, /* userdata        8G p76 */
#else
    {PART_PREAS,              1236 * 1024,  1280 * 1024, UFS_PART_3}, /* preas             1280MB p71 */
    {PART_PREAVS,             2516 * 1024,    32 * 1024, UFS_PART_3}, /* preavs              32MB p72 */
    {PART_SUPER,              2548 * 1024,  8024 * 1024, UFS_PART_3}, /* super             8024MB p73 */
    {PART_VERSION,           10572 * 1024,   576 * 1024, UFS_PART_3}, /* version            576MB p74 */
    {PART_PRELOAD,           11148 * 1024,  1144 * 1024, UFS_PART_3}, /* preload           1144MB p75 */
    {PART_USERDATA,          12292 * 1024,(8UL) * 1024 * 1024, UFS_PART_3}, /* userdata        8G p76 */
#endif
#else /* do not use erofs */
#ifdef CONFIG_MARKET_INTERNAL
    {PART_SUPER,              1236 * 1024,  9258 * 1024, UFS_PART_3}, /* super             9258MB p71 */
    {PART_VERSION,           10494 * 1024,   576 * 1024, UFS_PART_3}, /* version            576MB p72 */
    {PART_PRELOAD,           11070 * 1024,  1144 * 1024, UFS_PART_3}, /* preload           1144MB p73 */
    {PART_USERDATA,          12214 * 1024,(8UL) * 1024 * 1024, UFS_PART_3}, /* userdata        8G p74 */
#elif defined CONFIG_MARKET_OVERSEA
    {PART_SUPER,              1236 * 1024, 10386 * 1024, UFS_PART_3}, /* super             10386MB p71 */
    {PART_VERSION,           11622 * 1024,   576 * 1024, UFS_PART_3}, /* version            576MB p72 */
    {PART_PRELOAD,           12198 * 1024,  1144 * 1024, UFS_PART_3}, /* preload           1144MB p73 */
    {PART_USERDATA,          13342 * 1024,(8UL) * 1024 * 1024, UFS_PART_3}, /* userdata        8G p74 */
#elif defined CONFIG_MARKET_FULL_INTERNAL
    {PART_PREAS,              1236 * 1024,   368 * 1024, UFS_PART_3}, /* preas              368MB p71 */
    {PART_PREAVS,             1604 * 1024,    32 * 1024, UFS_PART_3}, /* preavs              32MB p72 */
    {PART_SUPER,              1636 * 1024,  8906 * 1024, UFS_PART_3}, /* super             8906MB p73 */
    {PART_VERSION,           10542 * 1024,   576 * 1024, UFS_PART_3}, /* version            576MB p74 */
    {PART_PRELOAD,           11118 * 1024,  1144 * 1024, UFS_PART_3}, /* preload           1144MB p75 */
    {PART_USERDATA,          12262 * 1024,(8UL) * 1024 * 1024, UFS_PART_3}, /* userdata        8G p76 */
#elif defined CONFIG_MARKET_FULL_OVERSEA
    {PART_PREAS,              1236 * 1024,  1280 * 1024, UFS_PART_3}, /* preas             1280MB p71 */
    {PART_PREAVS,             2516 * 1024,    32 * 1024, UFS_PART_3}, /* preavs              32MB p72 */
    {PART_SUPER,              2548 * 1024,  9122 * 1024, UFS_PART_3}, /* super             9122MB p73 */
    {PART_VERSION,           11670 * 1024,   576 * 1024, UFS_PART_3}, /* version            576MB p74 */
    {PART_PRELOAD,           12246 * 1024,  1144 * 1024, UFS_PART_3}, /* preload           1144MB p75 */
    {PART_USERDATA,          13390 * 1024,(8UL) * 1024 * 1024, UFS_PART_3}, /* userdata        8G p76 */
#else
    {PART_PREAS,              1236 * 1024,  1280 * 1024, UFS_PART_3}, /* preas             1280MB p71 */
    {PART_PREAVS,             2516 * 1024,    32 * 1024, UFS_PART_3}, /* preavs              32MB p72 */
    {PART_SUPER,              2548 * 1024,  8728 * 1024, UFS_PART_3}, /* super             8728MB p73 */
    {PART_VERSION,           11276 * 1024,   576 * 1024, UFS_PART_3}, /* version            576MB p74 */
    {PART_PRELOAD,           11852 * 1024,  1144 * 1024, UFS_PART_3}, /* preload           1144MB p75 */
#if defined(CONFIG_PARTITION_ENG)
    {PART_HITEST_HIPP,       12996 * 1024,   100 * 1024, UFS_PART_3}, /* hitest_hipp        100MB p76 */
    {PART_USERDATA,          13096 * 1024,(8UL) * 1024 * 1024, UFS_PART_3}, /* userdata        8G p77 */
#else
    {PART_USERDATA,          12996 * 1024,(8UL) * 1024 * 1024, UFS_PART_3}, /* userdata        8G p76 */
#endif
#endif
#endif
#endif
    {PART_PTABLE_LU4,                   0,         512,    UFS_PART_4}, /* ptable_lu4        512K   p0 */
    {PART_RESERVED12,                 512,         1536, UFS_PART_4}, /* reserved12        1536KB p1 */
    {PART_USERDATA2,             2 * 1024,(4UL) * 1024 * 1024, UFS_PART_4}, /* userdata2       4G p2 */
    {"0", 0, 0, 0},
};

#endif
