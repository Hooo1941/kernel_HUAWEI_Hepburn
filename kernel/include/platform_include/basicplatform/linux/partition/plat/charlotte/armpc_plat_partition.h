#ifndef _PISCES_PLAT_PARTITION_OHOS_H_
#define _PISCES_PLAT_PARTITION_OHOS_H_

#include "partition_macro.h"
#include "partition_macro_plus.h"
#include "partition_def.h"

static const struct partition partition_table_emmc[] = {
    {"0", 0, 0, 0},
};

static const struct partition partition_table_ufs[] = {
    {PART_XLOADER_A,                      0,              4 * 1024,      UFS_PART_0},
    {PART_XLOADER_B,                    0,                4 * 1024,      UFS_PART_1},
    {PART_PTABLE,                       0,                   512,      UFS_PART_2}, /* ptable           512K    p0  */
    {PART_VRL_A,                     512,                     512,    UFS_PART_2}, /* vrl_a            512K    p1 */
    {PART_VRL_B,                   2 * 512,                   512,    UFS_PART_2}, /* vrl_b            512K    p2 */
    {PART_VRL_BACKUP_A,            3 * 512,                   512,    UFS_PART_2}, /* vrl backup_a     512K    p3 */
    {PART_VRL_BACKUP_B,          2 * 1024,          512, UFS_PART_2}, /* vrl_backup_b       512KB p4 */
    {PART_BOOT_CTRL,            (2 * 1024 + 512),   512, UFS_PART_2}, /* boot ctrl        512K    p5 */
    {PART_NVME,                  3 * 1024,     4 * 1024, UFS_PART_2}, /* nvme                 4MB p6 */
    {PART_OEMINFO,               7 * 1024,    96 * 1024, UFS_PART_2}, /* oeminfo             96MB p7 */
    {PART_DDR_PARA,            103 * 1024,     1 * 1024, UFS_PART_2}, /* ddr_para          1024KB p8 */
    {PART_LOWPOWER_PARA_A,     104 * 1024,     1 * 1024, UFS_PART_2}, /* lowpower_para_a   1024KB p9 */
    {PART_LOWPOWER_PARA_B,     105 * 1024,     1 * 1024, UFS_PART_2}, /* lowpower_para_b   1024KB p10 */
    {PART_DFX,                 106 * 1024,    16 * 1024, UFS_PART_2}, /* dfx                 16MB p11 */
    {PART_BL2_A,               122 * 1024,     4 * 1024, UFS_PART_2}, /* bl2_a                4MB p12 */
    {PART_BL2_B,               126 * 1024,     4 * 1024, UFS_PART_2}, /* bl2_b                4MB p13 */
    {PART_FW_LPM3_A,           130 * 1024,          512, UFS_PART_2}, /* fw_lpm3_a          512KB p14 */
    {PART_FW_LPM3_B,             (130 * 1024 + 512),  512,      UFS_PART_2}, /* fw_lpm3            512KB p13 */
    {PART_FW_CPU_LPCTRL_A,     131 * 1024,          256, UFS_PART_2}, /* fw_cpu_lpctrl_a    256KB p16 */
    {PART_FW_CPU_LPCTRL_B,       (131 * 1024 + 256),           256,      UFS_PART_2}, /* fw_cpu_lpctrl    256K    p17 */
    {PART_FW_GPU_LPCTRL_A,       (131 * 1024 + 512),           128,      UFS_PART_2}, /* fw_gpu_lpctrl    128K    p18 */
    {PART_FW_GPU_LPCTRL_B,       (131 * 1024 + 640),           128,      UFS_PART_2}, /* fw_gpu_lpctrl    128K    p19 */
    {PART_FW_DDR_LPCTRL_A,       (131 * 1024 + 768),           128,      UFS_PART_2}, /* fw_ddr_lpctrl    128K    p20 */
    {PART_FW_DDR_LPCTRL_B,       (131 * 1024 + 896),           128,      UFS_PART_2}, /* fw_ddr_lpctrl    128K    p21 */
    {PART_DACC_A,              132 * 1024,          512, UFS_PART_2}, /* dacc_a             512KB p22 */
    {PART_DACC_B,             (132 * 1024 + 512),          512, UFS_PART_2}, /* dacc_b             512KB p23 */
    {PART_VENC_A,              133 * 1024,          512, UFS_PART_2}, /* venc_a             512KB p24 */
    {PART_VENC_B,             (133 * 1024 + 512),          512, UFS_PART_2}, /* venc              512KB p25 */
    {PART_FASTBOOT_A,          134 * 1024,    12 * 1024, UFS_PART_2}, /* fastboot_a          12MB p26 */
    {PART_FASTBOOT_B,          146 * 1024,    12 * 1024, UFS_PART_2}, /* fastboot_b          12MB p27 */
    {PART_VECTOR_A,            158 * 1024,     4 * 1024, UFS_PART_2}, /* vector_a             4MB p28 */
    {PART_VECTOR_B,            162 * 1024,     4 * 1024, UFS_PART_2}, /* vector_b             4MB p29 */
    {PART_TEEOS_A,             166 * 1024,    10 * 1024, UFS_PART_2}, /* teeos_a             10MB p30 */
    {PART_TEEOS_B,             176 * 1024,    10 * 1024, UFS_PART_2}, /* teeos_b             10MB p31 */
    {PART_FW_DTB_A,            186 * 1024,     8 * 1024, UFS_PART_2}, /* fw_dtb_a             8MB p32 */
    {PART_FW_DTB_B,            194 * 1024,     8 * 1024, UFS_PART_2}, /* fw_dtb_b             8MB p33 */
    {PART_DTBO_A,              202 * 1024,    14 * 1024, UFS_PART_2}, /* dtbo_a              14MB p34 */
    {PART_DTBO_B,              216 * 1024,    14 * 1024, UFS_PART_2}, /* dtbo_b              14MB p35 */
    {PART_TRUSTFIRMWARE_A,     230 * 1024,     2 * 1024, UFS_PART_2}, /* trustfirmware_a      2MB p36 */
    {PART_TRUSTFIRMWARE_B,     232 * 1024,     2 * 1024, UFS_PART_2}, /* trustfirmware_b      2MB p37 */
    {PART_THEE_A,              234 * 1024,     4 * 1024, UFS_PART_2}, /* thee_a               4MB p38 */
    {PART_THEE_B,              238 * 1024,     4 * 1024, UFS_PART_2}, /* thee_b               4MB p39 */
    {PART_TZSP_A,              242 * 1024,    12 * 1024, UFS_PART_2}, /* tzsp_a              12MB p40 */
    {PART_TZSP_B,              254 * 1024,    12 * 1024, UFS_PART_2}, /* tzsp_b              12MB p41 */
    {PART_FW_VERSION,          266 * 1024,     1 * 1024, UFS_PART_2}, /* fw_version        1024KB p42 */
    {PART_MCU,                 267 * 1024,     1 * 1024, UFS_PART_2}, /* mcu               1024KB p43 */
    {PART_PTABLE_LU3,                   0,                   512,      UFS_PART_3}, /* ptable_lu3       512K    p0 */
    {PART_RESERVED2,                  512,               3 * 512,      UFS_PART_3}, /* reserved2        512K    p1 */
    {PART_ROOT,                  2 * 1024, (120UL) * 1024 * 1024,      UFS_PART_3}, /* root              32G    p3 */
    {PART_EFI,              122882 * 1024,   (1UL) * 1024 * 1024,      UFS_PART_3}, /* efi                1G    p4 */
#ifdef CONFIG_FACTORY_MODE
    {PART_HIBENCH_IMG,       123906 * 1024,   128 * 1024,              UFS_PART_3}, /* hibench_img        128MB p5 */
    {PART_HIBENCH_DATA,      124034 * 1024,   512 * 1024,              UFS_PART_3}, /* hibench_data       512MB p6 */
    {PART_FLASH_AGEING,      124546 * 1024,    512 * 1024,             UFS_PART_3}, /* flash_ageing       512MB p7 */
    {PART_HIBENCH_LOG,       125058 * 1024,    32 * 1024,              UFS_PART_3}, /* hibench_log         32MB p8 */
    {PART_HIBENCH_LPM3,      125090 * 1024,     2 * 1024,              UFS_PART_3}, /* hibench_lpm3         2MB p9 */
    {PART_KERNEL,           125092 * 1024,   (8UL) * 1024 * 1024,      UFS_PART_3}, /* kerneldump         8G    p10 */
    {PART_HOME,             133284 * 1024,  (64UL) * 1024 * 1024,      UFS_PART_3}, /* home              64G    p11 */
#else
#ifdef CONFIG_PRODUCT_ARMPC_USER
    {PART_HOME,             123906 * 1024,  (64UL) * 1024 * 1024,      UFS_PART_3}, /* home              64G    p7 */
#else
    {PART_KERNEL,           123906 * 1024,   (8UL) * 1024 * 1024,      UFS_PART_3}, /* kerneldump         8G    p6 */
    {PART_HOME,             132098 * 1024,  (64UL) * 1024 * 1024,      UFS_PART_3}, /* home              64G    p8 */
#endif
#endif
    {"0", 0, 0, 0},
};

#endif