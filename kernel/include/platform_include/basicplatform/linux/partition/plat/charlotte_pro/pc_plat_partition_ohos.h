#ifndef _ANTLIA_PLAT_PARTITION_OHOS_H_
#define _ANTLIA_PLAT_PARTITION_OHOS_H_
 
#include "partition_macro.h"
#include "partition_macro_plus.h"
#include "partition_def.h"
 
static const struct partition partition_table_emmc[] = {
    {"0", 0, 0, 0},
};
 
static const struct partition partition_table_ufs[] = {
    {PART_PTABLE,                      0,           136, SPI_NOR_PART}, /* ptable            136K    p0  */
    {PART_BOOT_CTRL,                  136,            4, SPI_NOR_PART}, /* boot_ctrl            4KB p1 */
    {PART_BL2,                        140,          900, SPI_NOR_PART}, /* bl2                900KB p2 */
    {PART_FW_CPU_LPCTRL,             1040,          256, SPI_NOR_PART}, /* fw_cpu_lpctrl      256KB p3 */
    {PART_FW_GPU_LPCTRL,             1296,          128, SPI_NOR_PART}, /* fw_gpu_lpctrl      128KB p4 */
    {PART_FW_DDR_LPCTRL,             1424,          128, SPI_NOR_PART}, /* fw_ddr_lpctrl      128KB p5 */
    {PART_DDR_XPU_BOOT0,             1552,          136, SPI_NOR_PART}, /* ddr_xpu_boot0      136KB p6 */
    {PART_DDR_XPU_BOOT1,             1688,          136, SPI_NOR_PART}, /* ddr_xpu_boot1      136KB p7 */
    {PART_FW_DTB,                    1824,          516, SPI_NOR_PART}, /* fw_dtb             516KB p8 */
    {PART_DACC,                      2340,          128, SPI_NOR_PART}, /* dacc               128KB p9 */
    {PART_TRUSTFIRMWARE,             2468,          512, SPI_NOR_PART}, /* trustfirmware      512KB p10 */
    {PART_RESERVED2,                 2980,          400, SPI_NOR_PART}, /* reserved2          400KB p11 */
    /* !!!Attention!!! cannot modify xloader_a offset */
    {PART_XLOADER_A,                3380,         1024, SPI_NOR_PART}, /* xloader_a        1024KB p12 */
    {PART_NVME,                      4404,         1024, SPI_NOR_PART}, /* nvme              1024KB p13 */
#ifdef CONFIG_FACTORY_MODE
    {PART_FASTBOOT,                  5428,         5120, SPI_NOR_PART}, /* fastboot             5MB p14 */
    {PART_HIBENCH_IMG,              10548,         4608, SPI_NOR_PART}, /* hibench_img       4608KB p15 */
    {PART_HIBENCH_LOG,              15156,          512, SPI_NOR_PART}, /* hibench_log       512KB p15 */
#else
    {PART_FASTBOOT,                  5428,        10240, SPI_NOR_PART}, /* fastboot            10MB p14 */
#endif
    {PART_UEFI_VB,                  15668,          256, SPI_NOR_PART}, /* uefi_vb            256KB p15 */
    {PART_UEFI_LOGO,                15924,          768, SPI_NOR_PART}, /* uefi_logo          768KB p16 */
    {PART_LOWPOWER_PARA,            16692,          512, SPI_NOR_PART}, /* lowpower_para      512KB p17 */
    {PART_FW_LPM3,                  17204,          512, SPI_NOR_PART}, /* fw_lpm3            512KB p18 */
    {PART_TOC,                      17716,           32, SPI_NOR_PART}, /* toc                 32KB p19 */
    {PART_RESERVED2,                17748,          316, SPI_NOR_PART}, /* reserved2          316KB p20 */
    /* !!!Attention!!! cannot modify xloader_b offset */
    {PART_XLOADER_B,               18064,         1024, SPI_NOR_PART}, /* xloader_b        1024KB p21 */
    {PART_DDR_PARA,                 19088,          256, SPI_NOR_PART}, /* ddr_para           256KB p22 */
    {PART_HDMI,                     19344,           80, SPI_NOR_PART}, /* hdmi                80KB p23 */
    {PART_THEE,                     19424,         2048, SPI_NOR_PART}, /* thee                 2MB p24 */
    {PART_TZSP,                     21472,         3072, SPI_NOR_PART}, /* tzsp                 3MB p25 */
    {PART_TEEOS,                    24544,         4096, SPI_NOR_PART}, /* teeos                4MB p26 */
    {PART_OEMINFO_NOR,              28640,         3072, SPI_NOR_PART}, /* oeminfo_nor          3MB p27 */
    {PART_RPM_MAC,                  31712,            4, SPI_NOR_PART}, /* rpm_mac              4KB p28 */
    {PART_UAPP,                     31716,            4, SPI_NOR_PART}, /* uapp                 4KB p29 */
    {PART_VERIFI_BOOT,              31720,            4, SPI_NOR_PART}, /* verifi_boot          4KB p30 */
    {PART_MSPC_DATA,                31724,          164, SPI_NOR_PART}, /* mspc_data          164KB p31 */
    {PART_MSPC_CODE,                31888,          404, SPI_NOR_PART}, /* mspc_code          404KB p32 */
    {PART_HHEE,                     32292,          200, SPI_NOR_PART}, /* hhee               200KB p33 */
    {PART_RESERVED3,                32492,          256, SPI_NOR_PART}, /* reserved3          256KB p34 */
    {PART_PTABLE_SSD,                   0,               512, SSD_PART}, /* ptable_ssd           512K p0 */
    {PART_PERSIST,                    512,          6 * 1024, SSD_PART}, /* persist              6MB p1 */
    {PART_CONN_CALIDATA,           (6 * 1024 + 512),    1024, SSD_PART}, /* conn_calidata     1024KB p2 */
    {PART_OEMINFO,                 (7 * 1024 + 512),   96 * 1024, SSD_PART}, /* oeminfo             96MB p3 */
    {PART_SECURE_STORAGE,          (103 * 1024 + 512), 32 * 1024, SSD_PART}, /* secure_storage      32MB p4 */
    {PART_SECURE_TEE,              (135 * 1024 + 512), 13 * 1024, SSD_PART}, /* secure_tee          13MB p5 */
    {PART_HISEE_ENCOS,             (148 * 1024 + 512),  4 * 1024, SSD_PART}, /* hisee_encos          4MB p6 */
    {PART_RESERVED1,               (152 * 1024 + 512),  (20 * 1024 + 512), SSD_PART}, /* reserved1           20MB p7 */
    {PART_LOG,                 173 * 1024,    80 * 1024, SSD_PART}, /* log                 80MB p8 */
    {PART_BOOTFAIL_INFO,       253 * 1024,     2 * 1024, SSD_PART}, /* bootfail_info        2MB p9 */
    {PART_MISC,                255 * 1024,     2 * 1024, SSD_PART}, /* misc                 2MB p10 */
    {PART_DFX,                 257 * 1024,    16 * 1024, SSD_PART}, /* dfx                 16MB p11 */
    {PART_RRECORD,             273 * 1024,    16 * 1024, SSD_PART}, /* rrecord             16MB p12 */
    {PART_CACHE,               289 * 1024,   104 * 1024, SSD_PART}, /* cache              104MB p13 */
    {PART_METADATA,            393 * 1024,    16 * 1024, SSD_PART}, /* metadata            16MB p14 */
    {PART_RESERVED2,           409 * 1024,     (3 * 1024 + 512), SSD_PART}, /* reserved2            4MB p15 */
    {PART_NPU,                 (412 * 1024 + 512),     8 * 1024, SSD_PART}, /* npu                  8MB p16 */
    {PART_IVP,                 (420 * 1024 + 512),     2 * 1024, SSD_PART}, /* ivp                  2MB p17 */
    {PART_VENC,                (422 * 1024 + 512),          512, SSD_PART}, /* venc               512KB p18 */
    {PART_HISEE_IMG,           423 * 1024,     4 * 1024, SSD_PART}, /* hisee_img            4MB p19 */
    {PART_HISEE_FS,            427 * 1024,     8 * 1024, SSD_PART}, /* hisee_fs             8MB p20 */
    {PART_VECTOR,              435 * 1024,     4 * 1024, SSD_PART}, /* vector               4MB p21 */
    {PART_ISP_BOOT,            439 * 1024,     2 * 1024, SSD_PART}, /* isp_boot             2MB p22 */
    {PART_ISP_FIRMWARE,        441 * 1024,    14 * 1024, SSD_PART}, /* isp_firmware        14MB p23 */
    {PART_FW_HIFI,             455 * 1024,    12 * 1024, SSD_PART}, /* fw_hifi             12MB p24 */
    {PART_SENSORHUB,           467 * 1024,    13 * 1024, SSD_PART}, /* sensorhub           13MB p25 */
    {PART_SENSORHUB_LOG_DIC,   480 * 1024,     3 * 1024, SSD_PART}, /* sensorhub_log_dic    3MB p26 */
    {PART_RESERVED3,           483 * 1024,    24 * 1024, SSD_PART}, /* reserved3           24MB p27 */
    {PART_ESP,                 507 * 1024,   200 * 1024, SSD_PART}, /* esp                200MB p28 */
    {PART_UPDATER_RAMDISK,     707 * 1024,    32 * 1024, SSD_PART}, /* updater_ramdisk     32MB p29 */
    {PART_UPDATER_VENDOR,      739 * 1024,    24 * 1024, SSD_PART}, /* updater_vendor      24MB p30 */
    {PART_BOOT,                763 * 1024,    65 * 1024, SSD_PART}, /* boot                65MB p31 */
    {PART_UPDATER_BOOT,        828 * 1024,    45 * 1024, SSD_PART}, /* updater_boot        45MB p32 */
    {PART_UPDATER_BOOT_BAK,    873 * 1024,    45 * 1024, SSD_PART}, /* updater_boot_bak    45MB p33 */
    {PART_UPDATER_RAMDISK_BAK,   918 * 1024,    32 * 1024, SSD_PART}, /* updater_ramdisk_bak  32MB p34 */
    {PART_UPDATER_VENDOR_BAK,   950 * 1024,    24 * 1024, SSD_PART}, /* updater_vendor_bak  24MB p35 */
    {PART_ENG_SYSTEM,          974 * 1024,    12 * 1024, SSD_PART}, /* eng_system          12MB p36 */
    {PART_ENG_VENDOR,          986 * 1024,    20 * 1024, SSD_PART}, /* eng_vendor          20MB p37 */
    {PART_DTBO,               1006 * 1024,    14 * 1024, SSD_PART}, /* dtbo                14MB p38 */
    {PART_RAMDISK,            1020 * 1024,     3 * 1024, SSD_PART}, /* ramdisk              3MB p39 */
    {PART_RVT_SYSTEM,         1023 * 1024,     1 * 1024, SSD_PART}, /* rvt_system        1024KB p40 */
    {PART_RVT_VENDOR,         1024 * 1024,     1 * 1024, SSD_PART}, /* rvt_vendor        1024KB p41 */
    {PART_RVT_CHIP_PROD,      1025 * 1024,     1 * 1024, SSD_PART}, /* rvt_chip_prod     1024KB p42 */
    {PART_RVT_CUST,           1026 * 1024,     1 * 1024, SSD_PART}, /* rvt_cust          1024KB p43 */
    {PART_RVT_SYS_PROD,       1027 * 1024,     1 * 1024, SSD_PART}, /* rvt_sys_prod      1024KB p44 */
    {PART_UPDATER_RVT,        1028 * 1024,     2 * 1024, SSD_PART}, /* updater_rvt          2MB p45 */
    {PART_UPDATER_BAK_RVT,    1030 * 1024,     2 * 1024, SSD_PART}, /* updater_bak_rvt      2MB p46 */
    {PART_RVT,                1032 * 1024,     4 * 1024, SSD_PART}, /* rvt                  4MB p47 */
    {PART_SEPLAT,             1036 * 1024,     4 * 1024, SSD_PART}, /* seplat               4MB p48 */
    {PART_KPATCH,             1040 * 1024,     4 * 1024, SSD_PART}, /* kpatch               4MB p49 */
    {PART_PATCH,              1044 * 1024,    32 * 1024, SSD_PART}, /* patch               32MB p50 */
    {PART_CUST,               1076 * 1024,    88 * 1024, SSD_PART}, /* cust                88MB p51 */
    {PART_CHIP_PROD,          1164 * 1024,   616 * 1024, SSD_PART}, /* chip_prod          616MB p52 */
    {PART_VENDOR,             1780 * 1024,   920 * 1024, SSD_PART}, /* vendor             920MB p53 */
    {PART_SYS_PROD,           2700 * 1024,  3600 * 1024, SSD_PART}, /* sys_prod          3600MB p54 */
    {PART_SYSTEM,             6300 * 1024,  3416 * 1024, SSD_PART}, /* system            3416MB p55 */
    {PART_HYPERHOLD,          9716 * 1024,  1536 * 1024, SSD_PART}, /* hyperhold         1536MB p56 */
    {PART_RESERVED4,         11252 * 1024,   138 * 1024, SSD_PART}, /* reserved4          138MB p57 */
    {PART_VERSION,           11390 * 1024,   576 * 1024, SSD_PART}, /* version            576MB p58 */
    {PART_PRELOAD,           11966 * 1024,  1400 * 1024, SSD_PART}, /* preload           1400MB p59 */
#if defined(CONFIG_FACTORY_MODE)
    {PART_HIBENCH1_LOG,       13366 * 1024,    20 * 1024, SSD_PART}, /* hibench1_log         20MB p60 */
    {PART_HIBENCH_DATA,       13386 * 1024,   512 * 1024, SSD_PART}, /* hibench_data       512MB p61 */
    {PART_HIBENCH_LPM3,       13898 * 1024,     2 * 1024, SSD_PART}, /* hibench_lpm3         2MB p62 */
    {PART_USERDATA,          13900 * 1024,(16UL) * 1024 * 1024, SSD_PART}, /* userdata       16G p63 */
#else
    {PART_USERDATA,          13366 * 1024,(16UL) * 1024 * 1024, SSD_PART}, /* userdata       16G p60 */
#endif
    {"0", 0, 0, 0},
};
 
#endif