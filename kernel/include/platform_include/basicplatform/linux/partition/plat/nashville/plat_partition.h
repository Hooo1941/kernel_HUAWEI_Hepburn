#ifndef _PISCES_PLAT_PARTITION_H_
#define _PISCES_PLAT_PARTITION_H_

#include "partition_macro.h"
#include "partition_macro_plus.h"
#include "partition_def.h"

static const struct partition partition_table_emmc[] = {
    {"0", 0, 0, 0},
};

static const struct partition partition_table_ufs[] = {
    {PART_XLOADER,                0,              4 * 1024,    UFS_PART_0},
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
    {PART_FW_CPU_LPCTRL,       (496 * 1024 + 512),  256, UFS_PART_3}, /* fw_cpu_lpctrl      256K    p29 */
    {PART_FW_GPU_LPCTRL,       (496 * 1024 + 768),  128, UFS_PART_3}, /* fw_gpu_lpctrl      128K    p30 */
    {PART_FW_DDR_LPCTRL,       (496 * 1024 + 896),  256, UFS_PART_3}, /* fw_ddr_lpctrl      256K    p31 */
    {PART_DDR_XPU_BOOT0,       497 * 1024 + 128,    256, UFS_PART_3}, /* ddr_xpu_boot0      256KB p32 */
    {PART_DDR_XPU_BOOT1,       (497 * 1024 + 384),  256, UFS_PART_3}, /* ddr_xpu_boot1      256KB  p33 */
    {PART_DDR_XPU_BOOT2,       (497 * 1024 + 640),  256, UFS_PART_3}, /* ddr_xpu_boot2      256KB  p34 */
    {PART_UCE,                 (497 * 1024 + 896),  160, UFS_PART_3}, /* uce                160KB  p35 */
    {PART_RESERVED4,           (498 * 1024 + 32),  5 * 1024 + 2016, UFS_PART_3}, /* reserved4   7136KB p36 */
    {PART_NPU,                 505 * 1024,     8 * 1024, UFS_PART_3}, /* npu                  8MB p37 */
    {PART_IVP,                 513 * 1024,     2 * 1024, UFS_PART_3}, /* ivp                  2MB p38 */
    {PART_DACC,                515 * 1024,          512, UFS_PART_3}, /* dacc               512KB p39 */
    {PART_VENC,               (515 * 1024 + 512),   512, UFS_PART_3}, /* venc               512KB p35 */
    {PART_HISEE_IMG,           516 * 1024,     4 * 1024, UFS_PART_3}, /* hisee_img            4MB p41 */
    {PART_HHEE,                520 * 1024,     4 * 1024, UFS_PART_3}, /* hhee                 4MB p42 */
    {PART_HISEE_FS,            524 * 1024,     8 * 1024, UFS_PART_3}, /* hisee_fs             8MB p43 */
    {PART_FASTBOOT,            532 * 1024,    12 * 1024, UFS_PART_3}, /* fastboot            12MB p44 */
    {PART_VECTOR,              544 * 1024,     4 * 1024, UFS_PART_3}, /* vector               4MB p45 */
    {PART_ISP_BOOT,            548 * 1024,     2 * 1024, UFS_PART_3}, /* isp_boot             2MB p46 */
    {PART_ISP_FIRMWARE,        550 * 1024,    14 * 1024, UFS_PART_3}, /* isp_firmware        14MB p47 */
    {PART_FW_HIFI,             564 * 1024,    12 * 1024, UFS_PART_3}, /* fw_hifi             12MB p48 */
    {PART_TEEOS,               576 * 1024,     8 * 1024, UFS_PART_3}, /* teeos                8MB p49 */
    {PART_SENSORHUB,           584 * 1024,    13 * 1024, UFS_PART_3}, /* sensorhub           13MB p50 */
    {PART_SENSORHUB_LOG_DIC,   597 * 1024,     3 * 1024, UFS_PART_3}, /* sensorhub_log_dic    3MB p51 */
    {PART_HKRR,                600 * 1024,    16 * 1024, UFS_PART_3}, /* hkrr                16MB p52 */
    {PART_FW_UFSDEV,           616 * 1024,     4 * 1024, UFS_PART_3}, /* fw_ufsdev            4MB p53 */
    {PART_RESERVED5,           620 * 1024,    24 * 1024, UFS_PART_3}, /* reserved5           24MB p54 */
#ifdef CONFIG_SANITIZER_ENABLE
    {PART_ERECOVERY_RAMDISK,   644 * 1024,    12 * 1024, UFS_PART_3}, /* erecovery_ramdisk   12MB p55 */
    {PART_ERECOVERY_VENDOR,    656 * 1024,     8 * 1024, UFS_PART_3}, /* erecovery_vendor     8MB p56 */
    {PART_BOOT,                664 * 1024,    81 * 1024, UFS_PART_3}, /* boot                81MB p57 */
    {PART_RECOVERY,            745 * 1024,    85 * 1024, UFS_PART_3}, /* recovery            85MB p58 */
    {PART_ERECOVERY,           830 * 1024,    12 * 1024, UFS_PART_3}, /* erecovery           12MB p59 */
    {PART_RESERVED,            842 * 1024,    49 * 1024, UFS_PART_3}, /* reserved            49MB p60 */
    {PART_THEE,                891 * 1024,     4 * 1024, UFS_PART_3}, /* thee                 4MB p61 */
    {PART_TZSP,                895 * 1024,    12 * 1024, UFS_PART_3}, /* tzsp                12MB p62 */
    {PART_RECOVERY_RAMDISK,    907 * 1024,    32 * 1024, UFS_PART_3}, /* recovery_ramdisk    32MB p63 */
    {PART_RECOVERY_VENDOR,     939 * 1024,    24 * 1024, UFS_PART_3}, /* recovery_vendor     24MB p64 */
#elif defined CONFIG_HWSANITIZER_ENABLE
    {PART_ERECOVERY_RAMDISK,   644 * 1024,    40 * 1024, UFS_PART_3}, /* erecovery_ramdisk   40MB p55 */
    {PART_ERECOVERY_VENDOR,    684 * 1024,    24 * 1024, UFS_PART_3}, /* erecovery_vendor    24MB p56 */
    {PART_BOOT,                708 * 1024,    46 * 1024, UFS_PART_3}, /* boot                46MB p57 */
    {PART_RECOVERY,            754 * 1024,    45 * 1024, UFS_PART_3}, /* recovery            45MB p58 */
    {PART_ERECOVERY,           799 * 1024,    45 * 1024, UFS_PART_3}, /* erecovery           45MB p59 */
    {PART_RESERVED,            844 * 1024,    39 * 1024, UFS_PART_3}, /* reserved            39MB p60 */
    {PART_THEE,                883 * 1024,     4 * 1024, UFS_PART_3}, /* thee                 4MB p61 */
    {PART_TZSP,                887 * 1024,    12 * 1024, UFS_PART_3}, /* tzsp                12MB p62 */
    {PART_RECOVERY_RAMDISK,    899 * 1024,    40 * 1024, UFS_PART_3}, /* recovery_ramdisk    40MB p63 */
    {PART_RECOVERY_VENDOR,     939 * 1024,    24 * 1024, UFS_PART_3}, /* recovery_vendor     24MB p64 */
#else
    {PART_ERECOVERY_RAMDISK,   644 * 1024,    32 * 1024, UFS_PART_3}, /* erecovery_ramdisk   32MB p55 */
    {PART_ERECOVERY_VENDOR,    676 * 1024,    24 * 1024, UFS_PART_3}, /* erecovery_vendor    24MB p56 */
    {PART_BOOT,                700 * 1024,    46 * 1024, UFS_PART_3}, /* boot                46MB p57 */
    {PART_RECOVERY,            746 * 1024,    45 * 1024, UFS_PART_3}, /* recovery            45MB p58 */
    {PART_ERECOVERY,           791 * 1024,    45 * 1024, UFS_PART_3}, /* erecovery           45MB p59 */
    {PART_RESERVED,            836 * 1024,    55 * 1024, UFS_PART_3}, /* reserved            55MB p60 */
    {PART_THEE,                891 * 1024,     4 * 1024, UFS_PART_3}, /* thee                 4MB p61 */
    {PART_TZSP,                895 * 1024,    12 * 1024, UFS_PART_3}, /* tzsp                12MB p62 */
    {PART_RECOVERY_RAMDISK,    907 * 1024,    32 * 1024, UFS_PART_3}, /* recovery_ramdisk    32MB p63 */
    {PART_RECOVERY_VENDOR,     939 * 1024,    24 * 1024, UFS_PART_3}, /* recovery_vendor     24MB p64 */
#endif
    {PART_ENG_SYSTEM,          963 * 1024,    12 * 1024, UFS_PART_3}, /* eng_system          12MB p65 */
    {PART_ENG_VENDOR,          975 * 1024,    20 * 1024, UFS_PART_3}, /* eng_vendor          20MB p66 */
    {PART_FW_DTB,              995 * 1024,     8 * 1024, UFS_PART_3}, /* fw_dtb               8MB p67 */
    {PART_DTBO,               1003 * 1024,    14 * 1024, UFS_PART_3}, /* dtbo                14MB p68 */
    {PART_TRUSTFIRMWARE,      1017 * 1024,     2 * 1024, UFS_PART_3}, /* trustfirmware        2MB p69 */
    {PART_MODEM_FW,           1019 * 1024,   124 * 1024, UFS_PART_3}, /* modem_fw           124MB p70 */
    {PART_MODEM_VENDOR,       1143 * 1024,    10 * 1024, UFS_PART_3}, /* modem_vendor        10MB p71 */
    {PART_MODEM_PATCH_NV,     1153 * 1024,     4 * 1024, UFS_PART_3}, /* modem_patch_nv       4MB p72 */
    {PART_MODEM_DRIVER,       1157 * 1024,    20 * 1024, UFS_PART_3}, /* modem_driver        20MB p73 */
    {PART_MODEMNVM_UPDATE,    1177 * 1024,    16 * 1024, UFS_PART_3}, /* modemnvm_update     16MB p74 */
    {PART_MODEMNVM_CUST,      1193 * 1024,    16 * 1024, UFS_PART_3}, /* modemnvm_cust       16MB p75 */
    {PART_RAMDISK,            1209 * 1024,     2 * 1024, UFS_PART_3}, /* ramdisk              2MB p76 */
    {PART_VBMETA_SYSTEM,      1211 * 1024,     1 * 1024, UFS_PART_3}, /* vbmeta_system     1024KB p77 */
    {PART_VBMETA_VENDOR,      1212 * 1024,     1 * 1024, UFS_PART_3}, /* vbmeta_vendor     1024KB p78 */
    {PART_VBMETA_ODM,         1213 * 1024,     1 * 1024, UFS_PART_3}, /* vbmeta_odm        1024KB p79 */
    {PART_VBMETA_CUST,        1214 * 1024,     1 * 1024, UFS_PART_3}, /* vbmeta_cust       1024KB p80 */
    {PART_VBMETA_HW_PRODUCT,  1215 * 1024,     1 * 1024, UFS_PART_3}, /* vbmeta_hw_product 1024KB p81 */
    {PART_RECOVERY_VBMETA,    1216 * 1024,     2 * 1024, UFS_PART_3}, /* recovery_vbmeta      2MB p82 */
    {PART_ERECOVERY_VBMETA,   1218 * 1024,     2 * 1024, UFS_PART_3}, /* erecovery_vbmeta     2MB p83 */
    {PART_VBMETA,             1220 * 1024,     4 * 1024, UFS_PART_3}, /* vbmeta               4MB p84 */
    {PART_KPATCH,             1224 * 1024,     4 * 1024, UFS_PART_3}, /* kpatch               4MB p85 */
    {PART_PATCH,              1228 * 1024,    32 * 1024, UFS_PART_3}, /* patch               32MB p86 */
#ifdef CONFIG_FACTORY_MODE
    {PART_PREAS,              1260 * 1024,   368 * 1024, UFS_PART_3}, /* preas              368MB p87 */
    {PART_PREAVS,             1628 * 1024,    32 * 1024, UFS_PART_3}, /* preavs              32MB p88 */
    {PART_SUPER,              1660 * 1024,  9656 * 1024, UFS_PART_3}, /* super             9656MB p89 */
    {PART_VERSION,           11316 * 1024,   576 * 1024, UFS_PART_3}, /* version            576MB p90 */
    {PART_PRELOAD,           11892 * 1024,  1400 * 1024, UFS_PART_3}, /* preload           1400MB p91 */
    {PART_HIBENCH_IMG,       13292 * 1024,   128 * 1024, UFS_PART_3}, /* hibench_img        128MB p92 */
    {PART_HIBENCH_DATA,      13420 * 1024,   700 * 1024, UFS_PART_3}, /* hibench_data       700MB p93 */
    {PART_FLASH_AGEING,      14120 * 1024,   512 * 1024, UFS_PART_3}, /* flash_ageing       512MB p94 */
    {PART_HIBENCH_LOG,       14632 * 1024,    32 * 1024, UFS_PART_3}, /* hibench_log         32MB p95 */
    {PART_XLOADER_HITEST,    14664 * 1024,     2 * 1024, UFS_PART_3}, /* xloader_lpmcu        2MB p96 */
    {PART_USERDATA,          14666 * 1024,(8UL) * 1024 * 1024, UFS_PART_3}, /* userdata        8G p97 */
#elif defined CONFIG_MARKET_INTERNAL
#ifdef CONFIG_USE_EROFS
    {PART_PREAS,              1260 * 1024,   296 * 1024, UFS_PART_3}, /* preas              296MB p87 */
    {PART_PREAVS,             1556 * 1024,    32 * 1024, UFS_PART_3}, /* preavs              32MB p88 */
    {PART_SUPER,              1588 * 1024,  8322 * 1024, UFS_PART_3}, /* super             8322MB p89 */
    {PART_VERSION,            9910 * 1024,   576 * 1024, UFS_PART_3}, /* version            576MB p90 */
    {PART_PRELOAD,           10486 * 1024,  1400 * 1024, UFS_PART_3}, /* preload           1400MB p91 */
    {PART_USERDATA,          11886 * 1024,(8UL) * 1024 * 1024, UFS_PART_3}, /* userdata        8G p92 */
#else
    {PART_PREAS,              1260 * 1024,   368 * 1024, UFS_PART_3}, /* preas              368MB p87 */
    {PART_PREAVS,             1628 * 1024,    32 * 1024, UFS_PART_3}, /* preavs              32MB p88 */
    {PART_SUPER,              1660 * 1024, 10208 * 1024, UFS_PART_3}, /* super             10208MB p89 */
    {PART_VERSION,           11868 * 1024,   576 * 1024, UFS_PART_3}, /* version            576MB p90 */
    {PART_PRELOAD,           12444 * 1024,  1144 * 1024, UFS_PART_3}, /* preload           1144MB p91 */
    {PART_USERDATA,          13588 * 1024,(8UL) * 1024 * 1024, UFS_PART_3}, /* userdata        8G p92 */
#endif
#elif defined CONFIG_MARKET_OVERSEA
#ifdef CONFIG_USE_EROFS
    {PART_PREAS,              1260 * 1024,  1024 * 1024, UFS_PART_3}, /* preas             1024 * 1024KB p87 */
    {PART_PREAVS,             2284 * 1024,    32 * 1024, UFS_PART_3}, /* preavs              32MB p88 */
    {PART_SUPER,              2316 * 1024,  8282 * 1024, UFS_PART_3}, /* super             8282MB p89 */
    {PART_VERSION,           10598 * 1024,   576 * 1024, UFS_PART_3}, /* version            576MB p90 */
    {PART_PRELOAD,           11174 * 1024,  1400 * 1024, UFS_PART_3}, /* preload           1400MB p91 */
    {PART_USERDATA,          12574 * 1024,(8UL) * 1024 * 1024, UFS_PART_3}, /* userdata        8G p92 */
#else /* end erofs */
    {PART_PREAS,              1260 * 1024,  1280 * 1024, UFS_PART_3}, /* preas             1280MB p87 */
    {PART_PREAVS,             2540 * 1024,    32 * 1024, UFS_PART_3}, /* preavs              32MB p88 */
    {PART_SUPER,              2572 * 1024, 10688 * 1024, UFS_PART_3}, /* super             10688MB p89 */
    {PART_VERSION,           13260 * 1024,   576 * 1024, UFS_PART_3}, /* version            576MB p90 */
    {PART_PRELOAD,           13836 * 1024,  1144 * 1024, UFS_PART_3}, /* preload           1144MB p91 */
    {PART_USERDATA,          14980 * 1024,(8UL) * 1024 * 1024, UFS_PART_3}, /* userdata        8G p92 */
#endif
#elif defined CONFIG_GCOV_PARTITION_SUPPORT_ENABLE
    {PART_PREAS,              1260 * 1024,    32 * 1024, UFS_PART_3}, /* preas               32MB p87 */
    {PART_PREAVS,             1292 * 1024,    32 * 1024, UFS_PART_3}, /* preavs              32MB p88 */
    {PART_SUPER,              1324 * 1024, 10688 * 1024, UFS_PART_3}, /* super             10688MB p89 */
    {PART_VERSION,           12012 * 1024,   576 * 1024, UFS_PART_3}, /* version            576MB p90 */
    {PART_PRELOAD,           12588 * 1024,  1144 * 1024, UFS_PART_3}, /* preload           1144MB p91 */
    {PART_USERDATA,          13732 * 1024,(8UL) * 1024 * 1024, UFS_PART_3}, /* userdata        8G p92 */
#else
    {PART_PREAS,              1260 * 1024,  1280 * 1024, UFS_PART_3}, /* preas             1280MB p87 */
    {PART_PREAVS,             2540 * 1024,    32 * 1024, UFS_PART_3}, /* preavs              32MB p88 */
    {PART_SUPER,              2572 * 1024, 10688 * 1024, UFS_PART_3}, /* super             10688MB p89 */
    {PART_VERSION,           13260 * 1024,   576 * 1024, UFS_PART_3}, /* version            576MB p90 */
    {PART_PRELOAD,           13836 * 1024,  1144 * 1024, UFS_PART_3}, /* preload           1144MB p91 */
#if defined(CONFIG_PARTITION_ENG)
    {PART_FTTEST,            14980 * 1024,   192 * 1024, UFS_PART_3}, /* fttest             192MB p92 */
    {PART_DDRTEST,           15172 * 1024,    48 * 1024, UFS_PART_3}, /* ddrtest             48MB p93 */
    {PART_HITEST_GPU,        15220 * 1024,   512 * 1024, UFS_PART_3}, /* hitest_gpu         512MB p94 */
    {PART_HITEST_ASP_DHRYSTONE, 15732 * 1024,    20 * 1024, UFS_PART_3}, /* hitest_asp_dhrystone  20MB p95 */
    {PART_HITEST_ASP_TEST,   15752 * 1024,   300 * 1024, UFS_PART_3}, /* hitest_asp_test    300MB p96 */
    {PART_HITEST_SENSORHUB,  16052 * 1024,     6 * 1024, UFS_PART_3}, /* hitest_sensorhub     6MB p97 */
    {PART_HITEST_ISP,        16058 * 1024,    25 * 1024, UFS_PART_3}, /* hitest_isp          25MB p98 */
    {PART_HITEST_ISP_AVS,    16083 * 1024,    25 * 1024, UFS_PART_3}, /* hitest_isp_avs      25MB p99 */
    {PART_HITEST_NPU,        16108 * 1024,    80 * 1024, UFS_PART_3}, /* hitest_npu          80MB p100 */
    {PART_HITEST_DSS,        16188 * 1024,    18 * 1024, UFS_PART_3}, /* hitest_dss          18MB p101 */
    {PART_DSS_DISP_BUF,      16206 * 1024,    128 * 1024, UFS_PART_3}, /* dss_disp_buf        64MB p102 */
    {PART_HITEST_JPEGENC,    16334 * 1024,   200 * 1024, UFS_PART_3}, /* hitest_jpegenc     200MB p103 */
    {PART_HITEST_HIPP,       16534 * 1024,   100 * 1024, UFS_PART_3}, /* hitest_hipp        100MB p104 */
    {PART_HIBENCH_LPM3,      16634 * 1024,     2 * 1024, UFS_PART_3}, /* hibench_lpm3         2MB p105 */
    {PART_JPEG_DEC_TEST_SRC, 16636 * 1024,    20 * 1024, UFS_PART_3}, /* jpeg_dec_test_src   20MB p106 */
    {PART_JPEG_DEC_TEST_DEST, 16656 * 1024,    80 * 1024, UFS_PART_3}, /* jpeg_dec_test_dest  80MB p107 */
    {PART_VCODE_DATA,        16736 * 1024,    140 * 1024, UFS_PART_3}, /* vcode_data         140MB p108 */
    {PART_HITEST_FFRT,        16876 * 1024,   20 * 1024, UFS_PART_3}, /* hitest_ffrt         20MB p109 */
    {PART_VCODE_MAX_DATA,    16896 * 1024,   140 * 1024, UFS_PART_3}, /* vcode_max_data     140MB p110 */
    {PART_MODEM_TSP_TEST,    17036 * 1024,   32 * 1024, UFS_PART_3}, /* modem_tsp_test      32MB p111 */
    {PART_MODEM_MTEE,        17068 * 1024,   4 * 1024, UFS_PART_3}, /* modem_mtee           4MB p112 */
    {PART_MODEM_VECTOR_CONFIG,  17072 * 1024,   4 * 1024, UFS_PART_3}, /* modem_vector_config   4MB p113 */
    {PART_MODEM_TSP_DT,      17076 * 1024,     1 * 1024, UFS_PART_3}, /* modem_tsp_dt      1024KB p114 */
    {PART_MODEM_L2DLE,       17077 * 1024,     2 * 1024, UFS_PART_3}, /* modem_l2dle       2M p115 */
    {PART_MODEM_L2DLE_TEST,  17079 * 1024,     2 * 1024, UFS_PART_3}, /* modem_l2dle_test  2M p116 */
    {PART_MODEM_BBP_TEST,     17081 * 1024,    100 * 1024, UFS_PART_3}, /* modem_bbp_test       100MB p117 */
    {PART_MODEM_PSA_TEST,     17181 * 1024,    20 * 1024, UFS_PART_3}, /* modem_psa_test       20MB p118 */
    {PART_MODEM_PDE_TEST,    17201 * 1024,     5 * 1024, UFS_PART_3}, /* modem_pde_test       4MB p119 */
    {PART_XLOADER_HITEST,    17206 * 1024,     2 * 1024, UFS_PART_3}, /* xloader_lpmcu        2MB p120 */
    {PART_USERDATA,          17208 * 1024,(8UL) * 1024 * 1024, UFS_PART_3}, /* userdata        8G p121 */
#else
    {PART_USERDATA,          14980 * 1024,(8UL) * 1024 * 1024, UFS_PART_3}, /* userdata        8G p92 */
#endif
#endif
    {"0", 0, 0, 0},
};

#endif
