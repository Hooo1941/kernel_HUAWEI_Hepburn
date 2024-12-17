/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2020. All rights reserved.
 * Description: the partition table.
 */

#ifndef _SCORPIO_CSV2_PARTITION_H_
#define _SCORPIO_CSV2_PARTITION_H_

#include "partition_macro.h"
#include "partition_macro_plus.h"
#include "partition_def.h"

static const struct partition partition_table_emmc[] = {
    {"0", 0, 0, 0},
};

static const struct partition partition_table_ufs[] = {
    {PART_XLOADER,                0,              2 * 1024,    UFS_PART_0},
    {PART_RESERVED0,              0,              2 * 1024,    UFS_PART_1},
    {PART_PTABLE,                 0,                   512,    UFS_PART_2}, /* ptable           512K    p0 */
    {PART_FRP,                 512,          512, UFS_PART_2}, /* frp                512KB p1 */
    {PART_PERSIST,                1 * 1024,       6 * 1024,    UFS_PART_2}, /* persist         6144K    p2 */
    {PART_RESERVED1,             7 * 1024,         1024, UFS_PART_2}, /* reserved1         1024KB p3 */
    {PART_PTABLE_LU3,             0,                   512,    UFS_PART_3}, /* ptable_lu3       512K    p0 */
    {PART_VRL,                 512,          512, UFS_PART_3}, /* vrl                512KB p1 */
    {PART_VRL_BACKUP,                1024,          512, UFS_PART_3}, /* vrl_backup         512KB p2 */
    {PART_MODEM_SECURE,              1536,         8704, UFS_PART_3}, /* modem_secure      8704KB p3 */
    {PART_NVME,                 10 * 1024,     4 * 1024, UFS_PART_3}, /* nvme                 4MB p4 */
    {PART_CONN_CALIDATA,        14 * 1024,         1024, UFS_PART_3}, /* conn_calidata     1024KB p5 */
    {PART_CTF,                  15 * 1024,     1 * 1024, UFS_PART_3}, /* ctf               1024KB p6 */
    {PART_OEMINFO,              16 * 1024,    96 * 1024, UFS_PART_3}, /* oeminfo             96MB p7 */
    {PART_SECURE_STORAGE,      112 * 1024,    32 * 1024, UFS_PART_3}, /* secure_storage      32MB p8 */
    {PART_MODEMNVM_FACTORY,    144 * 1024,    16 * 1024, UFS_PART_3}, /* modemnvm_factory    16MB p9 */
    {PART_MODEMNVM_BACKUP,     160 * 1024,    16 * 1024, UFS_PART_3}, /* modemnvm_backup     16MB p10 */
    {PART_MODEMNVM_IMG,        176 * 1024,    46 * 1024, UFS_PART_3}, /* modemnvm_img        46MB p11 */
    {PART_HISEE_ENCOS,         222 * 1024,     4 * 1024, UFS_PART_3}, /* hisee_encos          4MB p12 */
    {PART_VERITYKEY,           226 * 1024,     1 * 1024, UFS_PART_3}, /* veritykey         1024KB p13 */
    {PART_DDR_PARA,            227 * 1024,     1 * 1024, UFS_PART_3}, /* ddr_para          1024KB p14 */
    {PART_LOWPOWER_PARA,       228 * 1024,     1 * 1024, UFS_PART_3}, /* lowpower_para     1024KB p15 */
    {PART_BATT_TP_PARA,        229 * 1024,     1 * 1024, UFS_PART_3}, /* batt_tp_para      1024KB p16 */
    {PART_RESERVED2,           230 * 1024,    21 * 1024, UFS_PART_3}, /* reserved2           21MB p17 */
    {PART_SPLASH2,             251 * 1024,    80 * 1024, UFS_PART_3}, /* splash2             80MB p18 */
    {PART_BOOTFAIL_INFO,       331 * 1024,     2 * 1024, UFS_PART_3}, /* bootfail_info        2MB p19 */
    {PART_MISC,                333 * 1024,     2 * 1024, UFS_PART_3}, /* misc                 2MB p20 */
    {PART_DFX,                 335 * 1024,    16 * 1024, UFS_PART_3}, /* dfx                 16MB p21 */
    {PART_RRECORD,             351 * 1024,    16 * 1024, UFS_PART_3}, /* rrecord             16MB p22 */
    {PART_CACHE,               367 * 1024,   104 * 1024, UFS_PART_3}, /* cache              104MB p23 */
    {PART_METADATA,            471 * 1024,    16 * 1024, UFS_PART_3}, /* metadata            16MB p24 */
    {PART_RESERVED3,           487 * 1024,     4 * 1024, UFS_PART_3}, /* reserved3            4MB p25 */
    {PART_TOC,                 491 * 1024,     1 * 1024, UFS_PART_3}, /* toc               1024KB p26 */
    {PART_BL2,                 492 * 1024,     4 * 1024, UFS_PART_3}, /* bl2                  4MB p27 */
    {PART_FW_LPM3,             496 * 1024,          512, UFS_PART_3}, /* fw_lpm3            512KB p28 */
    {PART_FW_CPU_LPCTRL,       (496 * 1024 + 512),  256, UFS_PART_3}, /* fw_cpu_lpctrl    256K    p29 */
    {PART_FW_GPU_LPCTRL,       (496 * 1024 + 768),  128, UFS_PART_3}, /* fw_gpu_lpctrl    128K    p30 */
    {PART_FW_DDR_LPCTRL,       (496 * 1024 + 896),  128, UFS_PART_3}, /* fw_ddr_lpctrl    128K    p31 */
    {PART_NPU,                 497 * 1024,     8 * 1024, UFS_PART_3}, /* npu                  8MB p32 */
    {PART_IVP,                 505 * 1024,     2 * 1024, UFS_PART_3}, /* ivp                  2MB p33 */
    {PART_DACC,                507 * 1024,          512, UFS_PART_3}, /* dacc               512KB p34 */
    {PART_VENC,               (507 * 1024 + 512),   512, UFS_PART_3}, /* venc               512KB p35 */
    {PART_HISEE_IMG,           508 * 1024,     4 * 1024, UFS_PART_3}, /* hisee_img            4MB p36 */
    {PART_HHEE,                512 * 1024,     4 * 1024, UFS_PART_3}, /* hhee                 4MB p37 */
    {PART_HISEE_FS,            516 * 1024,     8 * 1024, UFS_PART_3}, /* hisee_fs             8MB p38 */
    {PART_FASTBOOT,            524 * 1024,    12 * 1024, UFS_PART_3}, /* fastboot            12MB p39 */
    {PART_VECTOR,              536 * 1024,     4 * 1024, UFS_PART_3}, /* vector               4MB p40 */
    {PART_ISP_BOOT,            540 * 1024,     2 * 1024, UFS_PART_3}, /* isp_boot             2MB p41 */
    {PART_ISP_FIRMWARE,        542 * 1024,    14 * 1024, UFS_PART_3}, /* isp_firmware        14MB p42 */
    {PART_FW_HIFI,             556 * 1024,    12 * 1024, UFS_PART_3}, /* fw_hifi             12MB p43 */
    {PART_TEEOS,               568 * 1024,     8 * 1024, UFS_PART_3}, /* teeos                8MB p44 */
    {PART_SENSORHUB,           576 * 1024,    13 * 1024, UFS_PART_3}, /* sensorhub           13MB p45 */
    {PART_SENSORHUB_LOG_DIC,   589 * 1024,     3 * 1024, UFS_PART_3}, /* sensorhub_log_dic    3MB p46 */
    {PART_HKRR,                592 * 1024,    16 * 1024, UFS_PART_3}, /* hkrr                16MB p47 */
    {PART_FW_UFSDEV,           608 * 1024,     4 * 1024, UFS_PART_3}, /* fw_ufsdev            4MB p48 */
    {PART_RESERVED4,           612 * 1024,    24 * 1024, UFS_PART_3}, /* reserved4           24MB p49 */
#ifdef CONFIG_SANITIZER_ENABLE
    {PART_ERECOVERY_RAMDISK,   636 * 1024,    12 * 1024, UFS_PART_3}, /* erecovery_ramdisk   12MB p50 */
    {PART_ERECOVERY_VENDOR,    648 * 1024,     8 * 1024, UFS_PART_3}, /* erecovery_vendor     8MB p51 */
    {PART_BOOT,                656 * 1024,    81 * 1024, UFS_PART_3}, /* boot                81MB p52 */
    {PART_RECOVERY,            737 * 1024,    85 * 1024, UFS_PART_3}, /* recovery            85MB p53 */
    {PART_ERECOVERY,           822 * 1024,    12 * 1024, UFS_PART_3}, /* erecovery           12MB p54 */
    {PART_RESERVED,            834 * 1024,    49 * 1024, UFS_PART_3}, /* reserved            49MB p55 */
    {PART_THEE,                883 * 1024,     4 * 1024, UFS_PART_3}, /* thee                 4MB p56 */
    {PART_TZSP,                887 * 1024,    12 * 1024, UFS_PART_3}, /* tzsp                12MB p57 */
    {PART_RECOVERY_RAMDISK,    899 * 1024,    32 * 1024, UFS_PART_3}, /* recovery_ramdisk    32MB p58 */
    {PART_RECOVERY_VENDOR,     931 * 1024,    24 * 1024, UFS_PART_3}, /* recovery_vendor     24MB p59 */
#elif defined CONFIG_HWSANITIZER_ENABLE
    {PART_ERECOVERY_RAMDISK,   636 * 1024,    40 * 1024, UFS_PART_3}, /* erecovery_ramdisk   40MB p50 */
    {PART_ERECOVERY_VENDOR,    676 * 1024,    24 * 1024, UFS_PART_3}, /* erecovery_vendor    24MB p51 */
    {PART_BOOT,                700 * 1024,    46 * 1024, UFS_PART_3}, /* boot                46MB p52 */
    {PART_RECOVERY,            746 * 1024,    45 * 1024, UFS_PART_3}, /* recovery            45MB p53 */
    {PART_ERECOVERY,           791 * 1024,    45 * 1024, UFS_PART_3}, /* erecovery           45MB p54 */
    {PART_RESERVED,            836 * 1024,    39 * 1024, UFS_PART_3}, /* reserved            39MB p55 */
    {PART_THEE,                875 * 1024,     4 * 1024, UFS_PART_3}, /* thee                 4MB p56 */
    {PART_TZSP,                879 * 1024,    12 * 1024, UFS_PART_3}, /* tzsp                12MB p57 */
    {PART_RECOVERY_RAMDISK,    891 * 1024,    40 * 1024, UFS_PART_3}, /* recovery_ramdisk    40MB p58 */
    {PART_RECOVERY_VENDOR,     931 * 1024,    24 * 1024, UFS_PART_3}, /* recovery_vendor     24MB p59 */
#else
    {PART_ERECOVERY_RAMDISK,   636 * 1024,    32 * 1024, UFS_PART_3}, /* erecovery_ramdisk   32MB p50 */
    {PART_ERECOVERY_VENDOR,    668 * 1024,    24 * 1024, UFS_PART_3}, /* erecovery_vendor    24MB p51 */
    {PART_BOOT,                692 * 1024,    46 * 1024, UFS_PART_3}, /* boot                46MB p52 */
    {PART_RECOVERY,            738 * 1024,    45 * 1024, UFS_PART_3}, /* recovery            45MB p53 */
    {PART_ERECOVERY,           783 * 1024,    45 * 1024, UFS_PART_3}, /* erecovery           45MB p54 */
    {PART_RESERVED,            828 * 1024,    55 * 1024, UFS_PART_3}, /* reserved            55MB p55 */
    {PART_THEE,                883 * 1024,     4 * 1024, UFS_PART_3}, /* thee                 4MB p56 */
    {PART_TZSP,                887 * 1024,    12 * 1024, UFS_PART_3}, /* tzsp                12MB p57 */
    {PART_RECOVERY_RAMDISK,    899 * 1024,    32 * 1024, UFS_PART_3}, /* recovery_ramdisk    32MB p58 */
    {PART_RECOVERY_VENDOR,     931 * 1024,    24 * 1024, UFS_PART_3}, /* recovery_vendor     24MB p59 */
#endif
    {PART_ENG_SYSTEM,          955 * 1024,    12 * 1024, UFS_PART_3}, /* eng_system          12MB p60 */
    {PART_ENG_VENDOR,          967 * 1024,    20 * 1024, UFS_PART_3}, /* eng_vendor          20MB p61 */
    {PART_FW_DTB,              987 * 1024,     8 * 1024, UFS_PART_3}, /* fw_dtb               8MB p62 */
    {PART_DTBO,                995 * 1024,    14 * 1024, UFS_PART_3}, /* dtbo                14MB p63 */
    {PART_TRUSTFIRMWARE,      1009 * 1024,     2 * 1024, UFS_PART_3}, /* trustfirmware        2MB p64 */
    {PART_MODEM_FW,           1011 * 1024,   124 * 1024, UFS_PART_3}, /* modem_fw           124MB p65 */
    {PART_MODEM_VENDOR,       1135 * 1024,    10 * 1024, UFS_PART_3}, /* modem_vendor        10MB p66 */
    {PART_MODEM_PATCH_NV,     1145 * 1024,     4 * 1024, UFS_PART_3}, /* modem_patch_nv       4MB p67 */
    {PART_MODEM_DRIVER,       1149 * 1024,    20 * 1024, UFS_PART_3}, /* modem_driver        20MB p68 */
    {PART_MODEMNVM_UPDATE,    1169 * 1024,    16 * 1024, UFS_PART_3}, /* modemnvm_update     16MB p69 */
    {PART_MODEMNVM_CUST,      1185 * 1024,    16 * 1024, UFS_PART_3}, /* modemnvm_cust       16MB p70 */
    {PART_RAMDISK,            1201 * 1024,     2 * 1024, UFS_PART_3}, /* ramdisk              2MB p71 */
    {PART_VBMETA_SYSTEM,      1203 * 1024,     1 * 1024, UFS_PART_3}, /* vbmeta_system     1024KB p72 */
    {PART_VBMETA_VENDOR,      1204 * 1024,     1 * 1024, UFS_PART_3}, /* vbmeta_vendor     1024KB p73 */
    {PART_VBMETA_ODM,         1205 * 1024,     1 * 1024, UFS_PART_3}, /* vbmeta_odm        1024KB p74 */
    {PART_VBMETA_CUST,        1206 * 1024,     1 * 1024, UFS_PART_3}, /* vbmeta_cust       1024KB p75 */
    {PART_VBMETA_HW_PRODUCT,  1207 * 1024,     1 * 1024, UFS_PART_3}, /* vbmeta_hw_product 1024KB p76 */
    {PART_RECOVERY_VBMETA,    1208 * 1024,     2 * 1024, UFS_PART_3}, /* recovery_vbmeta      2MB p77 */
    {PART_ERECOVERY_VBMETA,   1210 * 1024,     2 * 1024, UFS_PART_3}, /* erecovery_vbmeta     2MB p78 */
    {PART_VBMETA,             1212 * 1024,     4 * 1024, UFS_PART_3}, /* vbmeta               4MB p79 */
    {PART_KPATCH,             1216 * 1024,     4 * 1024, UFS_PART_3}, /* kpatch               4MB p80 */
    {PART_PATCH,              1220 * 1024,    32 * 1024, UFS_PART_3}, /* patch               32MB p81 */
    {PART_SEPLAT,             1252 * 1024,     4 * 1024, UFS_PART_3}, /* seplat                4MB p82 */
#ifdef CONFIG_FACTORY_MODE
    {PART_PREAS,              1256 * 1024,   368 * 1024, UFS_PART_3}, /* preas              368MB p83 */
    {PART_PREAVS,             1624 * 1024,    32 * 1024, UFS_PART_3}, /* preavs              32MB p84 */
    {PART_SUPER,              1656 * 1024,  9956 * 1024, UFS_PART_3}, /* super             9956MB p85 */
    {PART_VERSION,           11612 * 1024,   576 * 1024, UFS_PART_3}, /* version            576MB p86 */
    {PART_PRELOAD,           12188 * 1024,  1400 * 1024, UFS_PART_3}, /* preload           1400MB p87 */
    {PART_HIBENCH_IMG,       13588 * 1024,   128 * 1024, UFS_PART_3}, /* hibench_img        128MB p88 */
    {PART_HIBENCH_DATA,      13716 * 1024,   512 * 1024, UFS_PART_3}, /* hibench_data       512MB p89 */
    {PART_FLASH_AGEING,      14228 * 1024,   512 * 1024, UFS_PART_3}, /* flash_ageing       512MB p90 */
    {PART_HIBENCH_LOG,       14740 * 1024,    32 * 1024, UFS_PART_3}, /* hibench_log         32MB p91 */
    {PART_HIBENCH_LPM3,      14772 * 1024,     2 * 1024, UFS_PART_3}, /* hibench_lpm3         2MB p92 */
    {PART_USERDATA,          14774 * 1024,(8UL) * 1024 * 1024, UFS_PART_3}, /* userdata        8G p93 */
#elif defined CONFIG_MARKET_INTERNAL
#ifdef CONFIG_USE_EROFS
    {PART_PREAS,              1256 * 1024,    12 * 1024, UFS_PART_3}, /* preas               12MB p83 */
    {PART_PREAVS,             1268 * 1024,    12 * 1024, UFS_PART_3}, /* preavs              12MB p84 */
    {PART_SUPER,              1280 * 1024,  8626 * 1024, UFS_PART_3}, /* super             8626MB p85 */
    {PART_VERSION,            9906 * 1024,   576 * 1024, UFS_PART_3}, /* version            576MB p86 */
    {PART_PRELOAD,           10482 * 1024,  1400 * 1024, UFS_PART_3}, /* preload           1400MB p87 */
    {PART_USERDATA,          11882 * 1024,(8UL) * 1024 * 1024, UFS_PART_3}, /* userdata        8G p88 */
#else
    {PART_PREAS,              1256 * 1024,    12 * 1024, UFS_PART_3}, /* preas               12MB p83 */
    {PART_PREAVS,             1268 * 1024,    12 * 1024, UFS_PART_3}, /* preavs              12MB p84 */
    {PART_SUPER,              1280 * 1024, 10208 * 1024, UFS_PART_3}, /* super             10208MB p85 */
    {PART_VERSION,           11488 * 1024,   576 * 1024, UFS_PART_3}, /* version            576MB p86 */
    {PART_PRELOAD,           12064 * 1024,  1144 * 1024, UFS_PART_3}, /* preload           1144MB p87 */
    {PART_USERDATA,          13208 * 1024,(8UL) * 1024 * 1024, UFS_PART_3}, /* userdata        8G p88 */
#endif
#elif defined CONFIG_MARKET_OVERSEA
#ifdef CONFIG_USE_EROFS
    {PART_PREAS,              1256 * 1024,    12 * 1024, UFS_PART_3}, /* preas               12MB p83 */
    {PART_PREAVS,             1268 * 1024,    12 * 1024, UFS_PART_3}, /* preavs              12MB p84 */
    {PART_SUPER,              1280 * 1024,  8410 * 1024, UFS_PART_3}, /* super             8410MB p85 */
    {PART_VERSION,            9690 * 1024,   576 * 1024, UFS_PART_3}, /* version            576MB p86 */
    {PART_PRELOAD,           10266 * 1024,  1400 * 1024, UFS_PART_3}, /* preload           1400MB p87 */
    {PART_USERDATA,          11666 * 1024,(8UL) * 1024 * 1024, UFS_PART_3}, /* userdata        8G p88 */
#else /* end erofs */
    {PART_PREAS,              1256 * 1024,    12 * 1024, UFS_PART_3}, /* preas               12MB p83 */
    {PART_PREAVS,             1268 * 1024,    12 * 1024, UFS_PART_3}, /* preavs              12MB p84 */
    {PART_SUPER,              1280 * 1024, 10688 * 1024, UFS_PART_3}, /* super             10688MB p85 */
    {PART_VERSION,           11968 * 1024,   576 * 1024, UFS_PART_3}, /* version            576MB p86 */
    {PART_PRELOAD,           12544 * 1024,  1144 * 1024, UFS_PART_3}, /* preload           1144MB p87 */
    {PART_USERDATA,          13688 * 1024,(8UL) * 1024 * 1024, UFS_PART_3}, /* userdata        8G p88 */
#endif
#elif defined CONFIG_GCOV_PARTITION_SUPPORT_ENABLE
    {PART_PREAS,              1256 * 1024,    12 * 1024, UFS_PART_3}, /* preas               12MB p83 */
    {PART_PREAVS,             1268 * 1024,    12 * 1024, UFS_PART_3}, /* preavs              12MB p84 */
    {PART_SUPER,              1280 * 1024, 10988 * 1024, UFS_PART_3}, /* super             10988MB p85 */
    {PART_VERSION,           12268 * 1024,   576 * 1024, UFS_PART_3}, /* version            576MB p86 */
    {PART_PRELOAD,           12844 * 1024,  1144 * 1024, UFS_PART_3}, /* preload           1144MB p87 */
    {PART_USERDATA,          13988 * 1024,(8UL) * 1024 * 1024, UFS_PART_3}, /* userdata        8G p88 */
#else
    {PART_PREAS,              1256 * 1024,    12 * 1024, UFS_PART_3}, /* preas               12MB p83 */
    {PART_PREAVS,             1268 * 1024,    12 * 1024, UFS_PART_3}, /* preavs              12MB p84 */
    {PART_SUPER,              1280 * 1024, 10988 * 1024, UFS_PART_3}, /* super             10988MB p85 */
    {PART_VERSION,           12268 * 1024,   576 * 1024, UFS_PART_3}, /* version            576MB p86 */
    {PART_PRELOAD,           12844 * 1024,  1144 * 1024, UFS_PART_3}, /* preload           1144MB p87 */
#if defined(CONFIG_BOOT_UEFI) && defined(CONFIG_PARTITION_ENG)
    {PART_FTTEST,            13988 * 1024,   192 * 1024, UFS_PART_3}, /* fttest             192MB p88 */
    {PART_DDRTEST,           14180 * 1024,    48 * 1024, UFS_PART_3}, /* ddrtest             48MB p89 */
    {PART_HITEST_GPU,        14228 * 1024,   512 * 1024, UFS_PART_3}, /* hitest_gpu         512MB p90 */
    {PART_HITEST_ASP_DHRYSTONE, 14740 * 1024,    20 * 1024, UFS_PART_3}, /* hitest_asp_dhrystone  20MB p91 */
    {PART_HITEST_ASP_TEST,   14760 * 1024,   300 * 1024, UFS_PART_3}, /* hitest_asp_test    300MB p92 */
    {PART_HITEST_SENSORHUB,  15060 * 1024,     6 * 1024, UFS_PART_3}, /* hitest_sensorhub     6MB p93 */
    {PART_HITEST_ISP,        15066 * 1024,    25 * 1024, UFS_PART_3}, /* hitest_isp          25MB p94 */
    {PART_HITEST_ISP_AVS,    15091 * 1024,    25 * 1024, UFS_PART_3}, /* hitest_isp_avs      25MB p95 */
    {PART_HITEST_NPU,        15116 * 1024,    80 * 1024, UFS_PART_3}, /* hitest_npu          80MB p96 */
    {PART_HITEST_DSS,        15196 * 1024,    18 * 1024, UFS_PART_3}, /* hitest_dss          18MB p97 */
    {PART_DSS_DISP_BUF,      15214 * 1024,    64 * 1024, UFS_PART_3}, /* dss_disp_buf        64MB p98 */
    {PART_HITEST_JPEGENC,    15278 * 1024,   200 * 1024, UFS_PART_3}, /* hitest_jpegenc     200MB p99 */
    {PART_HITEST_HIPP,       15478 * 1024,   100 * 1024, UFS_PART_3}, /* hitest_hipp        100MB p100 */
    {PART_HIBENCH_LPM3,      15578 * 1024,     2 * 1024, UFS_PART_3}, /* hibench_lpm3         2MB p101 */
    {PART_JPEG_DEC_TEST_SRC, 15580 * 1024,    20 * 1024, UFS_PART_3}, /* jpeg_dec_test_src   20MB p102 */
    {PART_JPEG_DEC_TEST_DEST, 15600 * 1024,    80 * 1024, UFS_PART_3}, /* jpeg_dec_test_dest  80MB p103 */
    {PART_VCODE_DATA,        15680 * 1024,   130 * 1024, UFS_PART_3}, /* vcode_data         130MB p104 */
    {PART_VCODE_MAX_DATA,    15810 * 1024,   140 * 1024, UFS_PART_3}, /* vcode_max_data     140MB p105 */
    {PART_MODEM_TSP_TEST,    15950 * 1024,    79 * 1024, UFS_PART_3}, /* modem_tsp_test      79MB p106 */
    {PART_MODEM_TSP_DT,      16029 * 1024,     1 * 1024, UFS_PART_3}, /* modem_tsp_dt      1024KB p107 */
    {PART_MODEM_TSP_AVS,     16030 * 1024,    80 * 1024, UFS_PART_3}, /* modem_tsp_avs       80MB p108 */
    {PART_MODEM_PDE_TEST,    16110 * 1024,     4 * 1024, UFS_PART_3}, /* modem_pde_test       4MB p109 */
    {PART_USERDATA,          16114 * 1024,(8UL) * 1024 * 1024, UFS_PART_3}, /* userdata        8G p110 */
#else
    {PART_USERDATA,          13988 * 1024,(8UL) * 1024 * 1024, UFS_PART_3}, /* userdata        8G p88 */
#endif
#endif
    {"0", 0, 0, 0},
};

#endif
