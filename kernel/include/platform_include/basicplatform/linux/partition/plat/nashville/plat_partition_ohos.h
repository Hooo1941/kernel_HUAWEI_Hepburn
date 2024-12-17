#ifndef _PISCES_PLAT_PARTITION_OHOS_H_
#define _PISCES_PLAT_PARTITION_OHOS_H_

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
    {PART_FRP,                        512,          512, UFS_PART_2}, /* frp                512KB p1 */
    {PART_PERSIST,                   1024,     6 * 1024, UFS_PART_2}, /* persist              6MB p2 */
    {PART_RESERVED1,             7 * 1024,         1024, UFS_PART_2}, /* reserved1         1024KB p3 */
    {PART_PTABLE_LU3,             0,                   512,    UFS_PART_3}, /* ptable_lu3       512K    p0 */
    {PART_VRL,                        512,          512, UFS_PART_3}, /* vrl                512KB p1 */
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
    {PART_LOG,                 251 * 1024,    80 * 1024, UFS_PART_3}, /* log                 80MB p18 */
    {PART_BOOTFAIL_INFO,       331 * 1024,     2 * 1024, UFS_PART_3}, /* bootfail_info        2MB p19 */
    {PART_MISC,                333 * 1024,     2 * 1024, UFS_PART_3}, /* misc                 2MB p20 */
    {PART_DFX,                 335 * 1024,    16 * 1024, UFS_PART_3}, /* dfx                 16MB p21 */
    {PART_RRECORD,             351 * 1024,    16 * 1024, UFS_PART_3}, /* rrecord             16MB p22 */
    {PART_CACHE,               367 * 1024,   104 * 1024, UFS_PART_3}, /* cache              104MB p23 */
    {PART_METADATA,            471 * 1024,    16 * 1024, UFS_PART_3}, /* metadata            16MB p24 */
    {PART_SEPLAT,              487 * 1024,     4 * 1024, UFS_PART_3}, /* seplat               4MB p25 */
    {PART_TOC,                 491 * 1024,     1 * 1024, UFS_PART_3}, /* toc               1024KB p26 */
    {PART_BL2,                 492 * 1024,     4 * 1024, UFS_PART_3}, /* bl2                  4MB p27 */
    {PART_FW_LPM3,             496 * 1024,          512, UFS_PART_3}, /* fw_lpm3            512KB p28 */
    {PART_FW_CPU_LPCTRL,       (496 * 1024 + 512),   256, UFS_PART_3}, /* fw_cpu_lpctrl      256K p29 */
    {PART_FW_GPU_LPCTRL,       (496 * 1024 + 768),   128, UFS_PART_3}, /* fw_gpu_lpctrl      128K p30 */
    {PART_FW_DDR_LPCTRL,       (496 * 1024 + 896),   256, UFS_PART_3}, /* fw_ddr_lpctrl      256K p31 */
    {PART_DDR_XPU_BOOT0,       (497 * 1024 + 128),   256, UFS_PART_3}, /* ddr_xpu_boot0     256KB p32 */
    {PART_DDR_XPU_BOOT1,       (497 * 1024 + 384),  256, UFS_PART_3}, /* ddr_xpu_boot1      256KB p33 */
    {PART_DDR_XPU_BOOT2,       (497 * 1024 + 640),  256, UFS_PART_3}, /* ddr_xpu_boot2      256KB p34 */
    {PART_UCE,                 (497 * 1024 + 896),  160, UFS_PART_3}, /* uce                160KB  p35 */
    {PART_RESERVED3,           (498 * 1024 + 32),  5 * 1024 + 2016, UFS_PART_3}, /* reserved4   7136KB p36 */
    {PART_NPU,                 505 * 1024,     8 * 1024, UFS_PART_3}, /* npu                  8MB p37 */
    {PART_IVP,                 513 * 1024,     2 * 1024, UFS_PART_3}, /* ivp                  2MB p38 */
    {PART_DACC,                515 * 1024,          512, UFS_PART_3}, /* dacc               512KB p39 */
    {PART_VENC,                (515 * 1024 + 512),  512, UFS_PART_3}, /* venc               512KB p40 */
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
    {PART_FW_DFA,              620 * 1024,     1 * 1024, UFS_PART_3}, /* fw_dfa               1MB p54 */
    {PART_RESERVED4,           621 * 1024,    15 * 1024, UFS_PART_3}, /* reserved4           23MB p55 */
    {PART_UPDATER_RAMDISK,     636 * 1024,    32 * 1024, UFS_PART_3}, /* updater_ramdisk     32MB p56 */
    {PART_UPDATER_VENDOR,      668 * 1024,    24 * 1024, UFS_PART_3}, /* updater_vendor      24MB p57 */
#ifdef CONFIG_ASAN_SUPPORT_ENABLE
    {PART_BOOT,                692 * 1024,    48 * 1024, UFS_PART_3}, /* boot                48MB p58 */
    {PART_UPDATER_BOOT,        740 * 1024,    55 * 1024, UFS_PART_3}, /* updater_boot        55MB p59 */
    {PART_UPDATER_BOOT_BAK,    795 * 1024,    55 * 1024, UFS_PART_3}, /* updater_boot_bak    55MB p60 */
    {PART_RESERVED,            850 * 1024,    47 * 1024, UFS_PART_3}, /* reserved            47MB p61 */
    {PART_THEE,                897 * 1024,     4 * 1024, UFS_PART_3}, /* thee                 4MB p62 */
    {PART_TZSP,                901 * 1024,    12 * 1024, UFS_PART_3}, /* tzsp                12MB p63 */
    {PART_UPDATER_RAMDISK_BAK,   913 * 1024,    32 * 1024, UFS_PART_3}, /* updater_ramdisk_bak  32MB p64 */
    {PART_UPDATER_VENDOR_BAK,   945 * 1024,    24 * 1024, UFS_PART_3}, /* updater_vendor_bak  24MB p65 */
    {PART_ENG_SYSTEM,           969 * 1024,    12 * 1024, UFS_PART_3}, /* eng_system          12MB p66 */
    {PART_ENG_CHIPSET,          981 * 1024,    20 * 1024, UFS_PART_3}, /* eng_chipset          20MB p67 */
    {PART_FW_DTB,              1001 * 1024,     8 * 1024, UFS_PART_3}, /* fw_dtb               8MB p68 */
    {PART_DTBO,                1009 * 1024,    14 * 1024, UFS_PART_3}, /* dtbo                14MB p69 */
    {PART_TRUSTFIRMWARE,       1023 * 1024,     2 * 1024, UFS_PART_3}, /* trustfirmware        2MB p70 */
    {PART_MODEM_FW,           1025 * 1024,   124 * 1024, UFS_PART_3}, /* modem_fw           124MB p71 */
    {PART_MODEM_VENDOR,       1149 * 1024,    10 * 1024, UFS_PART_3}, /* modem_vendor        10MB p72 */
    {PART_MODEM_PATCH_NV,     1159 * 1024,     4 * 1024, UFS_PART_3}, /* modem_patch_nv       4MB p73 */
    {PART_MODEM_DRIVER,       1163 * 1024,    20 * 1024, UFS_PART_3}, /* modem_driver        20MB p74 */
    {PART_MODEMNVM_UPDATE,    1183 * 1024,    16 * 1024, UFS_PART_3}, /* modemnvm_update     16MB p75 */
    {PART_MODEMNVM_CUST,      1199 * 1024,    16 * 1024, UFS_PART_3}, /* modemnvm_cust       16MB p76 */
    {PART_RAMDISK,            1215 * 1024,     5 * 1024, UFS_PART_3}, /* ramdisk              5MB p77 */
    {PART_RVT_SYSTEM,         1220 * 1024,     1 * 1024, UFS_PART_3}, /* rvt_system        1024KB p78 */
    {PART_RVT_VENDOR,         1221 * 1024,     1 * 1024, UFS_PART_3}, /* rvt_vendor        1024KB p79 */
    {PART_RVT_CHIP_PROD,      1222 * 1024,     1 * 1024, UFS_PART_3}, /* rvt_chip_prod     1024KB p80 */
    {PART_RVT_CUST,           1223 * 1024,     1 * 1024, UFS_PART_3}, /* rvt_cust          1024KB p81 */
    {PART_RVT_SYS_PROD,       1224 * 1024,     1 * 1024, UFS_PART_3}, /* rvt_sys_prod      1024KB p82 */
    {PART_UPDATER_RVT,        1225 * 1024,     2 * 1024, UFS_PART_3}, /* updater_rvt          2MB p83 */
    {PART_UPDATER_BAK_RVT,    1227 * 1024,     2 * 1024, UFS_PART_3}, /* updater_bak_rvt      2MB p84 */
    {PART_RVT,                1229 * 1024,     4 * 1024, UFS_PART_3}, /* rvt                  4MB p85 */
    {PART_KPATCH,             1233 * 1024,     4 * 1024, UFS_PART_3}, /* kpatch               4MB p86 */
    {PART_PATCH,              1237 * 1024,    32 * 1024, UFS_PART_3}, /* patch               32MB p87 */
    {PART_CUST,               1269 * 1024,    88 * 1024, UFS_PART_3}, /* cust                88MB p88 */
    {PART_CHIP_PROD,          1357 * 1024,   806 * 1024, UFS_PART_3}, /* chip_prod          806MB p89 */
    {PART_VENDOR,             2163 * 1024,  3072 * 1024, UFS_PART_3}, /* vendor             3072MB p90 */
    {PART_SYS_PROD,           5235 * 1024,  1746 * 1024, UFS_PART_3}, /* sys_prod          1746MB p91 */
    {PART_SYSTEM,             6981 * 1024,  3816 * 1024, UFS_PART_3}, /* system            3816MB p92 */
    {PART_HYPERHOLD,          10797 * 1024,  1536 * 1024, UFS_PART_3}, /* hyperhold         1536MB p93 */
    {PART_RESERVED5,          12333 * 1024,   137 * 1024, UFS_PART_3}, /* reserved5          137MB p94 */
    {PART_VERSION,            12470 * 1024,   576 * 1024, UFS_PART_3}, /* version            576MB p95 */
    {PART_PRELOAD,           13046 * 1024,  1400 * 1024, UFS_PART_3}, /* preload           1400MB p96 */
    {PART_USERDATA,          14446 * 1024,(16UL) * 1024 * 1024, UFS_PART_3}, /* userdata        6G p97 */
#else
#ifdef CONFIG_SANITIZER_ENABLE
    {PART_BOOT,                692 * 1024,    48 * 1024, UFS_PART_3}, /* boot                48MB p58 */
    {PART_UPDATER_BOOT,        740 * 1024,    48 * 1024, UFS_PART_3}, /* updater_boot        48MB p59 */
    {PART_UPDATER_BOOT_BAK,    788 * 1024,    48 * 1024, UFS_PART_3}, /* updater_boot_bak    48MB p60 */
    {PART_RESERVED,            836 * 1024,    47 * 1024, UFS_PART_3}, /* reserved            47MB p61 */
#else
    {PART_BOOT,                692 * 1024,    46 * 1024, UFS_PART_3}, /* boot                46MB p58 */
    {PART_UPDATER_BOOT,        738 * 1024,    45 * 1024, UFS_PART_3}, /* updater_boot        45MB p59 */
    {PART_UPDATER_BOOT_BAK,    783 * 1024,    45 * 1024, UFS_PART_3}, /* updater_boot_bak    45MB p60 */
    {PART_RESERVED,            828 * 1024,    55 * 1024, UFS_PART_3}, /* reserved            55MB p61 */
#endif
    {PART_THEE,                883 * 1024,     4 * 1024, UFS_PART_3}, /* thee                 4MB p62 */
    {PART_TZSP,                887 * 1024,    12 * 1024, UFS_PART_3}, /* tzsp                12MB p63 */
    {PART_UPDATER_RAMDISK_BAK,   899 * 1024,    32 * 1024, UFS_PART_3}, /* updater_ramdisk_bak  32MB p64 */
    {PART_UPDATER_VENDOR_BAK,   931 * 1024,    24 * 1024, UFS_PART_3}, /* updater_vendor_bak  24MB p65 */
    {PART_ENG_SYSTEM,          955 * 1024,    12 * 1024, UFS_PART_3}, /* eng_system          12MB p66 */
    {PART_ENG_CHIPSET,          967 * 1024,    20 * 1024, UFS_PART_3}, /* eng_chipset          20MB p67 */
    {PART_FW_DTB,              987 * 1024,     8 * 1024, UFS_PART_3}, /* fw_dtb               8MB p68 */
    {PART_DTBO,                995 * 1024,    14 * 1024, UFS_PART_3}, /* dtbo                14MB p69 */
    {PART_TRUSTFIRMWARE,      1009 * 1024,     2 * 1024, UFS_PART_3}, /* trustfirmware        2MB p70 */
    {PART_MODEM_FW,           1011 * 1024,   124 * 1024, UFS_PART_3}, /* modem_fw           124MB p71 */
    {PART_MODEM_VENDOR,       1135 * 1024,    10 * 1024, UFS_PART_3}, /* modem_vendor        10MB p72 */
    {PART_MODEM_PATCH_NV,     1145 * 1024,     4 * 1024, UFS_PART_3}, /* modem_patch_nv       4MB p73 */
    {PART_MODEM_DRIVER,       1149 * 1024,    20 * 1024, UFS_PART_3}, /* modem_driver        20MB p74 */
    {PART_MODEMNVM_UPDATE,    1169 * 1024,    16 * 1024, UFS_PART_3}, /* modemnvm_update     16MB p75 */
    {PART_MODEMNVM_CUST,      1185 * 1024,    16 * 1024, UFS_PART_3}, /* modemnvm_cust       16MB p76 */
    {PART_RAMDISK,            1201 * 1024,     3 * 1024, UFS_PART_3}, /* ramdisk              3MB p77 */
    {PART_RVT_SYSTEM,         1204 * 1024,     1 * 1024, UFS_PART_3}, /* rvt_system        1024KB p78 */
    {PART_RVT_VENDOR,         1205 * 1024,     1 * 1024, UFS_PART_3}, /* rvt_vendor        1024KB p79 */
    {PART_RVT_CHIP_PROD,      1206 * 1024,     1 * 1024, UFS_PART_3}, /* rvt_chip_prod     1024KB p80 */
    {PART_RVT_CUST,           1207 * 1024,     1 * 1024, UFS_PART_3}, /* rvt_cust          1024KB p81 */
    {PART_RVT_SYS_PROD,       1208 * 1024,     1 * 1024, UFS_PART_3}, /* rvt_sys_prod      1024KB p82 */
    {PART_UPDATER_RVT,        1209 * 1024,     2 * 1024, UFS_PART_3}, /* updater_rvt          2MB p83 */
    {PART_UPDATER_BAK_RVT,    1211 * 1024,     2 * 1024, UFS_PART_3}, /* updater_bak_rvt      2MB p84 */
    {PART_RVT,                1213 * 1024,     4 * 1024, UFS_PART_3}, /* rvt                  4MB p85 */
    {PART_KPATCH,             1217 * 1024,     4 * 1024, UFS_PART_3}, /* kpatch               4MB p86 */
    {PART_PATCH,              1221 * 1024,    32 * 1024, UFS_PART_3}, /* patch               32MB p87 */
    {PART_CUST,               1253 * 1024,    88 * 1024, UFS_PART_3}, /* cust                88MB p88 */
    {PART_CHIP_PROD,          1341 * 1024,   806 * 1024, UFS_PART_3}, /* chip_prod          806MB p89 */
    {PART_VENDOR,             2147 * 1024,   920 * 1024, UFS_PART_3}, /* vendor             920MB p90 */
    {PART_SYS_PROD,           3067 * 1024,  1746 * 1024, UFS_PART_3}, /* sys_prod          1746MB p91 */
    {PART_SYSTEM,             4813 * 1024,  4948 * 1024, UFS_PART_3}, /* system            4948MB p92 */
    {PART_HYPERHOLD,          9761 * 1024,     4 * 1024, UFS_PART_3}, /* hyperhold            4MB p93 */
    {PART_RESERVED5,          9765 * 1024,   137 * 1024, UFS_PART_3}, /* reserved5          137MB p94 */
    {PART_VERSION,            9902 * 1024,   576 * 1024, UFS_PART_3}, /* version            576MB p95 */
    {PART_PRELOAD,           10478 * 1024,  1400 * 1024, UFS_PART_3}, /* preload           1400MB p96 */
#if defined(CONFIG_FACTORY_MODE)
    {PART_HIBENCH_IMG,       11878 * 1024,   128 * 1024, UFS_PART_3}, /* hibench_img        128MB p97 */
    {PART_HIBENCH_LOG,       12006 * 1024,    32 * 1024, UFS_PART_3}, /* hibench_log         32MB p98 */
    {PART_HIBENCH_DATA,      12038 * 1024,   512 * 1024, UFS_PART_3}, /* hibench_data       512MB p99 */
    {PART_FLASH_AGEING,      12550 * 1024,   512 * 1024, UFS_PART_3}, /* flash_ageing       512MB p101 */
    {PART_HIBENCH_LPM3,      13062 * 1024,     2 * 1024, UFS_PART_3}, /* hibench_lpm3         2MB p102 */
    {PART_RESERVED6,         13064 * 1024,  1406 * 1024, UFS_PART_3}, /* reserved6           1406MB p103 */
    {PART_USERDATA,          14470 * 1024,(16UL) * 1024 * 1024, UFS_PART_3}, /* userdata        6G p104 */
#else
    {PART_USERDATA,          11878 * 1024,(16UL) * 1024 * 1024, UFS_PART_3}, /* userdata        6G p97 */
#endif
#endif
    {"0", 0, 0, 0},
};

#endif
