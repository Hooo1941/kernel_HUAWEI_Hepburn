/*
 *  Copyright (c) Huawei Technologies Co., Ltd. 2019-2021. All rights reserved.
 *  Description: the partition table.
 */

#ifndef _ANTLIA_CDC_ACE_PLAT_PARTITION_H_
#define _ANTLIA_CDC_ACE_PLAT_PARTITION_H_

#include "partition_macro.h"
#include "partition_macro_plus.h"
#include "partition_def.h"

static const struct partition partition_table_emmc[] = {
    {"0", 0, 0, 0},
};

static struct partition partition_table_ufs[] = {
    {PART_XLOADER_A,                        0,      2 * 1024,      UFS_PART_0},
    {PART_XLOADER_B,                        0,      2 * 1024,      UFS_PART_1},
    {PART_PTABLE,                           0,           512,      UFS_PART_2}, /* ptable             512K    p0 */
    {PART_VRL_A,                     512,          512, UFS_PART_2}, /* vrl_a             512K    p1 */
    {PART_VRL_BACKUP_A,              1024,          512, UFS_PART_2}, /* vrl_backup_a       512KB p2 */
    {PART_RESERVED1_lu2,             1536,          512, UFS_PART_2}, /* reserved1_lu2      512KB p3 */
    {PART_NPU_A,                 2 * 1024,     8 * 1024, UFS_PART_2}, /* npu_a                8MB p4 */
    {PART_HIEPS_A,              10 * 1024,     2 * 1024, UFS_PART_2}, /* hieps_a              2MB p5 */
    {PART_IVP_A,                12 * 1024,     2 * 1024, UFS_PART_2}, /* ivp_a                2MB p6 */
    {PART_HISEE_IMG_A,          14 * 1024,     4 * 1024, UFS_PART_2}, /* hisee_img_a          4MB p7 */
    {PART_HHEE_A,               18 * 1024,     4 * 1024, UFS_PART_2}, /* hhee_a               4MB p8 */
    {PART_FASTBOOT_A,           22 * 1024,    12 * 1024, UFS_PART_2}, /* fastboot_a          12MB p9 */
    {PART_LOWPOWER_PARA_A,      34 * 1024,     1 * 1024, UFS_PART_2}, /* lowpower_para_a   1024KB p10 */
    {PART_BL2_A,                35 * 1024,     4 * 1024, UFS_PART_2}, /* bl2_a                4MB p11 */
    {PART_TEEOS_A,              39 * 1024,     8 * 1024, UFS_PART_2}, /* teeos_a              8MB p12 */
    {PART_FW_LPM3_A,            47 * 1024,          512, UFS_PART_2}, /* fw_lpm3_a          512KB p13 */
    {PART_FW_CPU_LPCTRL_A,  (47 * 1024 + 512),           256,      UFS_PART_2}, /* fw_cpu_lpctrl_a      256K    p14 */
    {PART_FW_GPU_LPCTRL_A,  (47 * 1024 + 768),           128,      UFS_PART_2}, /* fw_gpu_lpctrl_a      128K    p15 */
    {PART_FW_DDR_LPCTRL_A,  (47 * 1024 + 896),           128,      UFS_PART_2}, /* fw_ddr_lpctrl_a      128K    p16 */
    {PART_FW_UFSDEV_A,          48 * 1024,     4 * 1024, UFS_PART_2}, /* fw_ufsdev_a          4MB p17 */
    {PART_RESERVED2_lu2,        52 * 1024,     4 * 1024, UFS_PART_2}, /* reserved2_lu2        4MB p18 */
    {PART_VECTOR_A,             56 * 1024,     4 * 1024, UFS_PART_2}, /* vector_a             4MB p19 */
    {PART_ISP_BOOT_A,           60 * 1024,     2 * 1024, UFS_PART_2}, /* isp_boot_a           2MB p20 */
    {PART_ISP_FIRMWARE_A,       62 * 1024,    14 * 1024, UFS_PART_2}, /* isp_firmware_a      14MB p21 */
    {PART_FW_HIFI_A,            76 * 1024,    12 * 1024, UFS_PART_2}, /* fw_hifi_a           12MB p22 */
#ifdef CONFIG_SANITIZER_ENABLE
    {PART_BOOT_A,               88 * 1024,   100 * 1024, UFS_PART_2}, /* boot_a             100MB p23 */
    {PART_RECOVERY_A,          188 * 1024,    85 * 1024, UFS_PART_2}, /* recovery_a          85MB p24 */
    {PART_RECOVERY_RAMDISK_A,   273 * 1024,    32 * 1024, UFS_PART_2}, /* recovery_ramdisk_a  32MB p25 */
    {PART_RECOVERY_VENDOR_A,   305 * 1024,    24 * 1024, UFS_PART_2}, /* recovery_vendor_a   24MB p26 */
    {PART_ERECOVERY_A,         329 * 1024,    12 * 1024, UFS_PART_2}, /* erecovery_a         12MB p27 */
    {PART_ERECOVERY_RAMDISK_A,   341 * 1024,    12 * 1024, UFS_PART_2}, /* erecovery_ramdisk_a  12MB p28 */
    {PART_ERECOVERY_VENDOR_A,   353 * 1024,     8 * 1024, UFS_PART_2}, /* erecovery_vendor_a   8MB p29 */
    {PART_RESERVED3_lu2,       361 * 1024,    50 * 1024, UFS_PART_2}, /* reserved3_lu2       50MB p30 */
#elif defined CONFIG_HWSANITIZER_ENABLE
    {PART_BOOT_A,               88 * 1024,    46 * 1024, UFS_PART_2}, /* boot_a              46MB p23 */
    {PART_RECOVERY_A,          134 * 1024,    45 * 1024, UFS_PART_2}, /* recovery_a          45MB p24 */
    {PART_RECOVERY_RAMDISK_A,   179 * 1024,    40 * 1024, UFS_PART_2}, /* recovery_ramdisk_a  40MB p25 */
    {PART_RECOVERY_VENDOR_A,   219 * 1024,    24 * 1024, UFS_PART_2}, /* recovery_vendor_a   24MB p26 */
    {PART_ERECOVERY_A,         243 * 1024,    45 * 1024, UFS_PART_2}, /* erecovery_a         45MB p27 */
    {PART_ERECOVERY_RAMDISK_A,   288 * 1024,    40 * 1024, UFS_PART_2}, /* erecovery_ramdisk_a  40MB p28 */
    {PART_ERECOVERY_VENDOR_A,   328 * 1024,    24 * 1024, UFS_PART_2}, /* erecovery_vendor_a  24MB p29 */
    {PART_RESERVED3_lu2,       352 * 1024,    59 * 1024, UFS_PART_2}, /* reserved3_lu2       59MB p30 */
#else
    {PART_BOOT_A,               88 * 1024,    46 * 1024, UFS_PART_2}, /* boot_a              46MB p23 */
    {PART_RECOVERY_A,          134 * 1024,    45 * 1024, UFS_PART_2}, /* recovery_a          45MB p24 */
    {PART_RECOVERY_RAMDISK_A,   179 * 1024,    32 * 1024, UFS_PART_2}, /* recovery_ramdisk_a  32MB p25 */
    {PART_RECOVERY_VENDOR_A,   211 * 1024,    24 * 1024, UFS_PART_2}, /* recovery_vendor_a   24MB p26 */
    {PART_ERECOVERY_A,         235 * 1024,    45 * 1024, UFS_PART_2}, /* erecovery_a         45MB p27 */
    {PART_ERECOVERY_RAMDISK_A,   280 * 1024,    32 * 1024, UFS_PART_2}, /* erecovery_ramdisk_a  32MB p28 */
    {PART_ERECOVERY_VENDOR_A,   312 * 1024,    24 * 1024, UFS_PART_2}, /* erecovery_vendor_a  24MB p29 */
    {PART_RESERVED3_lu2,       336 * 1024,    75 * 1024, UFS_PART_2}, /* reserved3_lu2       75MB p30 */
#endif
    {PART_KPATCH_A,            411 * 1024,     4 * 1024, UFS_PART_2}, /* kpatch_a             4MB p31 */
    {PART_ENG_SYSTEM_A,        415 * 1024,    12 * 1024, UFS_PART_2}, /* eng_system_a        12MB p32 */
    {PART_RAMDISK_A,           427 * 1024,     2 * 1024, UFS_PART_2}, /* ramdisk_a            2MB p33 */
    {PART_VBMETA_SYSTEM_A,     429 * 1024,     1 * 1024, UFS_PART_2}, /* vbmeta_system_a   1024KB p34 */
    {PART_VBMETA_VENDOR_A,     430 * 1024,     1 * 1024, UFS_PART_2}, /* vbmeta_vendor_a   1024KB p35 */
    {PART_VBMETA_ODM_A,        431 * 1024,     1 * 1024, UFS_PART_2}, /* vbmeta_odm_a      1024KB p36 */
    {PART_VBMETA_CUST_A,       432 * 1024,     1 * 1024, UFS_PART_2}, /* vbmeta_cust_a     1024KB p37 */
    {PART_VBMETA_HW_PRODUCT_A,   433 * 1024,     1 * 1024, UFS_PART_2}, /* vbmeta_hw_product_a1024KB p38 */
    {PART_DTBO_A,              434 * 1024,    20 * 1024, UFS_PART_2}, /* dtbo_a              20MB p39 */
    {PART_ENG_VENDOR_A,        454 * 1024,    20 * 1024, UFS_PART_2}, /* eng_vendor_a        20MB p40 */
    {PART_FW_DTB_A,            474 * 1024,     8 * 1024, UFS_PART_2}, /* fw_dtb_a             8MB p41 */
    {PART_RDA_DTB_A,           482 * 1024,     8 * 1024, UFS_PART_2}, /* rda_dtb_a            8MB p42 */
    {PART_MICROVISOR_A,        490 * 1024,   100 * 1024, UFS_PART_2}, /* microvisor_a       100MB p43 */
    {PART_MICROVISOR_RECOVERY_A,   590 * 1024,   110 * 1024, UFS_PART_2}, /* microvisor_recovery_a 110MB p44 */
    {PART_TRUSTFIRMWARE_A,     700 * 1024,     2 * 1024, UFS_PART_2}, /* trustfirmware_a      2MB p45 */
    {PART_RECOVERY_VBMETA_A,   702 * 1024,     2 * 1024, UFS_PART_2}, /* recovery_vbmeta_a    2MB p46 */
    {PART_ERECOVERY_VBMETA_A,   704 * 1024,     2 * 1024, UFS_PART_2}, /* erecovery_vbmeta_a   2MB p47 */
    {PART_VBMETA_A,            706 * 1024,     4 * 1024, UFS_PART_2}, /* vbmeta_a             4MB p48 */
    {PART_PATCH_A,             710 * 1024,    32 * 1024, UFS_PART_2}, /* patch_a             32MB p49 */
    {PART_PREAS_A,             742 * 1024,    12 * 1024, UFS_PART_2}, /* preas_a             12MB p50 */
    {PART_PREAVS_A,            754 * 1024,    12 * 1024, UFS_PART_2}, /* preavs_a            12MB p51 */
    {PART_SAFETY_ISLAND_A,     766 * 1024,     4 * 1024, UFS_PART_2}, /* safety_island_a      4MB p52 */
    {PART_THEE_A,              770 * 1024,     4 * 1024, UFS_PART_2}, /* thee_a               4MB p53 */
    {PART_TZSP_A,              774 * 1024,    12 * 1024, UFS_PART_2}, /* tzsp_a              12MB p54 */
    {PART_DICE_A,              786 * 1024,     1 * 1024, UFS_PART_2}, /* dice_a            1024KB p55 */
    {PART_DACC_A,              787 * 1024,          256, UFS_PART_2}, /* dacc_a             256KB p56 */
    {PART_DACC_LITE_A,    (787 * 1024 + 256),           256,      UFS_PART_2}, /* dacc_lite_a        256K    p57 */
    {PART_VENC_A,         (787 * 1024 + 512),           512,      UFS_PART_2}, /* venc_a             512K    p58 */
    {PART_DDR_XPU_BOOT0_A,     788 * 1024,          136, UFS_PART_2}, /* ddr_xpu_boot0_a    136KB p59 */
    {PART_DDR_XPU_BOOT1_A,(788 * 1024 + 136),           136,      UFS_PART_2}, /* ddr_xpu_boot1_a    136K    p60 */
    {PART_RDA_A,          (788 * 1024 + 272),    200 * 1024,      UFS_PART_2}, /* rda_a              200MB    p61 */
    {PART_RDA_VENDOR_A,   (988 * 1024 + 272),    100 * 1024,      UFS_PART_2}, /* rda_vendor_a    100MB    p62 */
    {PART_PTABLE_LU3,                       0,           512,      UFS_PART_3}, /* ptable             512K    p0 */
    {PART_VRL_B,                     512,          512, UFS_PART_3}, /* vrl_b             512K    p1 */
    {PART_VRL_BACKUP_B,              1024,          512, UFS_PART_3}, /* vrl_backup_b       512KB p2 */
    {PART_RESERVED1_lu3,             1536,          512, UFS_PART_3}, /* reserved1_lu3      512KB p3 */
    {PART_NPU_B,                 2 * 1024,     8 * 1024, UFS_PART_3}, /* npu_b                8MB p4 */
    {PART_HIEPS_B,              10 * 1024,     2 * 1024, UFS_PART_3}, /* hieps_b              2MB p5 */
    {PART_IVP_B,                12 * 1024,     2 * 1024, UFS_PART_3}, /* ivp_b                2MB p6 */
    {PART_HISEE_IMG_B,          14 * 1024,     4 * 1024, UFS_PART_3}, /* hisee_img_b          4MB p7 */
    {PART_HHEE_B,               18 * 1024,     4 * 1024, UFS_PART_3}, /* hhee_b               4MB p8 */
    {PART_FASTBOOT_B,           22 * 1024,    12 * 1024, UFS_PART_3}, /* fastboot_b          12MB p9 */
    {PART_LOWPOWER_PARA_B,      34 * 1024,     1 * 1024, UFS_PART_3}, /* lowpower_para_b   1024KB p10 */
    {PART_BL2_B,                35 * 1024,     4 * 1024, UFS_PART_3}, /* bl2_b                4MB p11 */
    {PART_TEEOS_B,              39 * 1024,     8 * 1024, UFS_PART_3}, /* teeos_b              8MB p12 */
    {PART_FW_LPM3_B,            47 * 1024,          512, UFS_PART_3}, /* fw_lpm3_b          512KB p13 */
    {PART_FW_CPU_LPCTRL_B,  (47 * 1024 + 512),           256,      UFS_PART_3}, /* fw_cpu_lpctrl_b      256K    p14 */
    {PART_FW_GPU_LPCTRL_B,  (47 * 1024 + 768),           128,      UFS_PART_3}, /* fw_gpu_lpctrl_b      128K    p15 */
    {PART_FW_DDR_LPCTRL_B,  (47 * 1024 + 896),           128,      UFS_PART_3}, /* fw_ddr_lpctrl_b      128K    p16 */
    {PART_FW_UFSDEV_B,          48 * 1024,     4 * 1024, UFS_PART_3}, /* fw_ufsdev_b          4MB p17 */
    {PART_RESERVED2_lu3,        52 * 1024,     4 * 1024, UFS_PART_3}, /* reserved2_lu3        4MB p18 */
    {PART_VECTOR_B,             56 * 1024,     4 * 1024, UFS_PART_3}, /* vector_b             4MB p19 */
    {PART_ISP_BOOT_B,           60 * 1024,     2 * 1024, UFS_PART_3}, /* isp_boot_b           2MB p20 */
    {PART_ISP_FIRMWARE_B,       62 * 1024,    14 * 1024, UFS_PART_3}, /* isp_firmware_b      14MB p21 */
    {PART_FW_HIFI_B,            76 * 1024,    12 * 1024, UFS_PART_3}, /* fw_hifi_b           12MB p22 */
#ifdef CONFIG_SANITIZER_ENABLE
    {PART_BOOT_B,               88 * 1024,   100 * 1024, UFS_PART_3}, /* boot_b             100MB p23 */
    {PART_RECOVERY_B,          188 * 1024,    85 * 1024, UFS_PART_3}, /* recovery_b          85MB p24 */
    {PART_RECOVERY_RAMDISK_B,   273 * 1024,    32 * 1024, UFS_PART_3}, /* recovery_ramdisk_b  32MB p25 */
    {PART_RECOVERY_VENDOR_B,   305 * 1024,    24 * 1024, UFS_PART_3}, /* recovery_vendor_b   24MB p26 */
    {PART_ERECOVERY_B,         329 * 1024,    12 * 1024, UFS_PART_3}, /* erecovery_b         12MB p27 */
    {PART_ERECOVERY_RAMDISK_B,   341 * 1024,    12 * 1024, UFS_PART_3}, /* erecovery_ramdisk_b  12MB p28 */
    {PART_ERECOVERY_VENDOR_B,   353 * 1024,     8 * 1024, UFS_PART_3}, /* erecovery_vendor_b   8MB p29 */
    {PART_RESERVED3_lu3,       361 * 1024,    50 * 1024, UFS_PART_3}, /* reserved3_lu3       50MB p30 */
#elif defined CONFIG_HWSANITIZER_ENABLE
    {PART_BOOT_B,               88 * 1024,    46 * 1024, UFS_PART_3}, /* boot_b              46MB p23 */
    {PART_RECOVERY_B,          134 * 1024,    45 * 1024, UFS_PART_3}, /* recovery_b          45MB p24 */
    {PART_RECOVERY_RAMDISK_B,   179 * 1024,    40 * 1024, UFS_PART_3}, /* recovery_ramdisk_b  40MB p25 */
    {PART_RECOVERY_VENDOR_B,   219 * 1024,    24 * 1024, UFS_PART_3}, /* recovery_vendor_b   24MB p26 */
    {PART_ERECOVERY_B,         243 * 1024,    45 * 1024, UFS_PART_3}, /* erecovery_b         45MB p27 */
    {PART_ERECOVERY_RAMDISK_B,   288 * 1024,    40 * 1024, UFS_PART_3}, /* erecovery_ramdisk_b  40MB p28 */
    {PART_ERECOVERY_VENDOR_B,   328 * 1024,    24 * 1024, UFS_PART_3}, /* erecovery_vendor_b  24MB p29 */
    {PART_RESERVED3_lu3,       352 * 1024,    59 * 1024, UFS_PART_3}, /* reserved3_lu3       59MB p30 */
#else
    {PART_BOOT_B,               88 * 1024,    46 * 1024, UFS_PART_3}, /* boot_b              46MB p23 */
    {PART_RECOVERY_B,          134 * 1024,    45 * 1024, UFS_PART_3}, /* recovery_b          45MB p24 */
    {PART_RECOVERY_RAMDISK_B,   179 * 1024,    32 * 1024, UFS_PART_3}, /* recovery_ramdisk_b  32MB p25 */
    {PART_RECOVERY_VENDOR_B,   211 * 1024,    24 * 1024, UFS_PART_3}, /* recovery_vendor_b   24MB p26 */
    {PART_ERECOVERY_B,         235 * 1024,    45 * 1024, UFS_PART_3}, /* erecovery_b         45MB p27 */
    {PART_ERECOVERY_RAMDISK_B,   280 * 1024,    32 * 1024, UFS_PART_3}, /* erecovery_ramdisk_b  32MB p28 */
    {PART_ERECOVERY_VENDOR_B,   312 * 1024,    24 * 1024, UFS_PART_3}, /* erecovery_vendor_b  24MB p29 */
    {PART_RESERVED3_lu3,       336 * 1024,    75 * 1024, UFS_PART_3}, /* reserved3_lu3       75MB p30 */
#endif
    {PART_KPATCH_B,            411 * 1024,     4 * 1024, UFS_PART_3}, /* kpatch_b             4MB p31 */
    {PART_ENG_SYSTEM_B,        415 * 1024,    12 * 1024, UFS_PART_3}, /* eng_system_b        12MB p32 */
    {PART_RAMDISK_B,           427 * 1024,     2 * 1024, UFS_PART_3}, /* ramdisk_b            2MB p33 */
    {PART_VBMETA_SYSTEM_B,     429 * 1024,     1 * 1024, UFS_PART_3}, /* vbmeta_system_b   1024KB p34 */
    {PART_VBMETA_VENDOR_B,     430 * 1024,     1 * 1024, UFS_PART_3}, /* vbmeta_vendor_b   1024KB p35 */
    {PART_VBMETA_ODM_B,        431 * 1024,     1 * 1024, UFS_PART_3}, /* vbmeta_odm_b      1024KB p36 */
    {PART_VBMETA_CUST_B,       432 * 1024,     1 * 1024, UFS_PART_3}, /* vbmeta_cust_b     1024KB p37 */
    {PART_VBMETA_HW_PRODUCT_B,   433 * 1024,     1 * 1024, UFS_PART_3}, /* vbmeta_hw_product_b1024KB p38 */
    {PART_DTBO_B,              434 * 1024,    20 * 1024, UFS_PART_3}, /* dtbo_b              20MB p39 */
    {PART_ENG_VENDOR_B,        454 * 1024,    20 * 1024, UFS_PART_3}, /* eng_vendor_b        20MB p40 */
    {PART_FW_DTB_B,            474 * 1024,     8 * 1024, UFS_PART_3}, /* fw_dtb_b             8MB p41 */
    {PART_RDA_DTB_B,           482 * 1024,     8 * 1024, UFS_PART_3}, /* rda_dtb_b            8MB p42 */
    {PART_MICROVISOR_B,        490 * 1024,   100 * 1024, UFS_PART_3}, /* microvisor_b       100MB p43 */
    {PART_MICROVISOR_RECOVERY_B,   590 * 1024,   110 * 1024, UFS_PART_3}, /* microvisor_recovery_b 110MB p44 */
    {PART_TRUSTFIRMWARE_B,     700 * 1024,     2 * 1024, UFS_PART_3}, /* trustfirmware_b      2MB p45 */
    {PART_RECOVERY_VBMETA_B,   702 * 1024,     2 * 1024, UFS_PART_3}, /* recovery_vbmeta_b    2MB p46 */
    {PART_ERECOVERY_VBMETA_B,   704 * 1024,     2 * 1024, UFS_PART_3}, /* erecovery_vbmeta_b   2MB p47 */
    {PART_VBMETA_B,            706 * 1024,     4 * 1024, UFS_PART_3}, /* vbmeta_b             4MB p48 */
    {PART_PATCH_B,             710 * 1024,    32 * 1024, UFS_PART_3}, /* patch_b             32MB p49 */
    {PART_PREAS_B,             742 * 1024,    12 * 1024, UFS_PART_3}, /* preas_b             12MB p50 */
    {PART_PREAVS_B,            754 * 1024,    12 * 1024, UFS_PART_3}, /* preavs_b            12MB p51 */
    {PART_SAFETY_ISLAND_B,     766 * 1024,     4 * 1024, UFS_PART_3}, /* safety_island_b      4MB p52 */
    {PART_THEE_B,              770 * 1024,     4 * 1024, UFS_PART_3}, /* thee_b               4MB p53 */
    {PART_TZSP_B,              774 * 1024,    12 * 1024, UFS_PART_3}, /* tzsp_b              12MB p54 */
    {PART_DICE_B,              786 * 1024,     1 * 1024, UFS_PART_3}, /* dice_b            1024KB p55 */
    {PART_DACC_B,              787 * 1024,          256, UFS_PART_3}, /* dacc_b             256KB p56 */
    {PART_DACC_LITE_B,    (787 * 1024 + 256),       256, UFS_PART_3}, /* dacc_lite_b        256K    p57 */
    {PART_VENC_B,         (787 * 1024 + 512),       512, UFS_PART_3}, /* venc_b             512K    p58 */
    {PART_DDR_XPU_BOOT0_B,     788 * 1024,          136, UFS_PART_3}, /* ddr_xpu_boot0_b    136KB p59 */
    {PART_DDR_XPU_BOOT1_B, (788 * 1024 + 136),    136, UFS_PART_3}, /* ddr_xpu_boot1_b    136K    p60 */
    {PART_RDA_B,           (788 * 1024 + 272),    200 * 1024,UFS_PART_3}, /* rda_b          200MB    p61 */
    {PART_RDA_VENDOR_B,    (988 * 1024 + 272),    100 * 1024, UFS_PART_3}, /* rda_vendor_b  100MB    p62 */
    {PART_PTABLE_LU4,                       0,           512,      UFS_PART_4}, /* ptable_lu4         512K    p0 */
    {PART_RESERVED1_lu4,              512,          512, UFS_PART_4}, /* reserved1_lu4      512KB p1 */
    {PART_RDA_LOG,                   1024,   500 * 1024, UFS_PART_4}, /* rda_log            500MB p2 */
    {PART_RDA_OEMINFO,         501 * 1024,    16 * 1024, UFS_PART_4}, /* rda_oeminfo         16MB p3 */
    {PART_RDA_RECOVERY_DFX,    517 * 1024,     4 * 1024, UFS_PART_4}, /* rda_recovery_dfx     4MB p4 */
    {PART_RESERVED2_lu4,       521 * 1024,   119 * 1024, UFS_PART_4}, /* reserved2_lu4      119MB p5 */
    {PART_PTABLE_LU5,                       0,           512,      UFS_PART_5}, /* ptable_lu5         512K    p0 */
    {PART_FRP,                        512,          512, UFS_PART_5}, /* frp                512KB p1 */
    {PART_PERSIST,                   1024,     6 * 1024, UFS_PART_5}, /* persist              6MB p2 */
    {PART_BOOT_CTRL,             7 * 1024,     1 * 1024, UFS_PART_5}, /* boot_ctrl         1024KB p3 */
    {PART_NVME,                  8 * 1024,     5 * 1024, UFS_PART_5}, /* nvme                 5MB p4 */
    {PART_CTF,                  13 * 1024,     1 * 1024, UFS_PART_5}, /* ctf               1024KB p5 */
    {PART_OEMINFO,              14 * 1024,    96 * 1024, UFS_PART_5}, /* oeminfo             96MB p6 */
    {PART_BOARD_ID,            110 * 1024,           64, UFS_PART_5}, /* board_id            64KB p7 */
    {PART_RESERVED1_lu5,        (110 * 1024 + 64),  (960 + 1024),      UFS_PART_5}, /* reserved1_lu5 1984KB p8 */
    {PART_SECURE_STORAGE,      112 * 1024,    32 * 1024, UFS_PART_5}, /* secure_storage      32MB p9 */
    {PART_HISEE_ENCOS,         144 * 1024,     4 * 1024, UFS_PART_5}, /* hisee_encos          4MB p10 */
    {PART_VERITYKEY,           148 * 1024,     1 * 1024, UFS_PART_5}, /* veritykey         1024KB p11 */
    {PART_DDR_PARA,            149 * 1024,     1 * 1024, UFS_PART_5}, /* ddr_para          1024KB p12 */
    {PART_BATT_TP_PARA,        150 * 1024,     1 * 1024, UFS_PART_5}, /* batt_tp_para      1024KB p13 */
    {PART_TOC,                 151 * 1024,     1 * 1024, UFS_PART_5}, /* toc               1024KB p14 */
    {PART_SPLASH2,             152 * 1024,    80 * 1024, UFS_PART_5}, /* splash2             80MB p15 */
    {PART_BOOTFAIL_INFO,       232 * 1024,     2 * 1024, UFS_PART_5}, /* bootfail_info        2MB p16 */
    {PART_MISC,                234 * 1024,     2 * 1024, UFS_PART_5}, /* misc                 2MB p17 */
    {PART_DFX,                 236 * 1024,    16 * 1024, UFS_PART_5}, /* dfx                 16MB p18 */
    {PART_RRECORD,             252 * 1024,    16 * 1024, UFS_PART_5}, /* rrecord             16MB p19 */
    {PART_CACHE,               268 * 1024,   104 * 1024, UFS_PART_5}, /* cache              104MB p20 */
    {PART_METADATA,            372 * 1024,    16 * 1024, UFS_PART_5}, /* metadata            16MB p21 */
    {PART_HISEE_FS,            388 * 1024,     8 * 1024, UFS_PART_5}, /* hisee_fs             8MB p22 */
    {PART_CDC_DATA,            396 * 1024,     8 * 1024, UFS_PART_5}, /* cdc_data             8MB p23 */
    {PART_RESERVED2_lu5,       404 * 1024,    84 * 1024, UFS_PART_5}, /* reserved2_lu5       84MB p24 */
    {PART_RDA_DATA,            488 * 1024,   500 * 1024, UFS_PART_5}, /* rda_data           500MB p25 */
    {PART_SUPER,               988 * 1024, 23318 * 1024, UFS_PART_5}, /* super             23318MB p26 */
    {PART_VERSION_A,         24306 * 1024,  2000 * 1024, UFS_PART_5}, /* version_a         2000MB p27 */
    {PART_PRELOAD_A,         26306 * 1024,  1144 * 1024, UFS_PART_5}, /* preload_a         1144MB p28 */
    {PART_RDA_PRODUCT_A,     27450 * 1024,  1500 * 1024, UFS_PART_5}, /* rda_product_a     1500MB p29 */
    {PART_VERSION_B,         28950 * 1024,  2000 * 1024, UFS_PART_5}, /* version_b         2000MB p30 */
    {PART_PRELOAD_B,         30950 * 1024,  1144 * 1024, UFS_PART_5}, /* preload_b         1144MB p31 */
    {PART_RDA_PRODUCT_B,     32094 * 1024,  1500 * 1024, UFS_PART_5}, /* rda_product_b     1500MB p32 */
#ifdef CONFIG_FACTORY_MODE
    {PART_HIBENCH_IMG,       33594 * 1024,   128 * 1024, UFS_PART_5}, /* hibench_img        128MB p33 */
    {PART_HIBENCH_DATA,      33722 * 1024,   512 * 1024, UFS_PART_5}, /* hibench_data       512MB p34 */
    {PART_FLASH_AGEING,      34234 * 1024,   512 * 1024, UFS_PART_5}, /* flash_ageing       512MB p35 */
    {PART_HIBENCH_LOG,       34746 * 1024,    32 * 1024, UFS_PART_5}, /* hibench_log         32MB p36 */
    {PART_HIBENCH_LPM3,      34778 * 1024,    32 * 1024, UFS_PART_5}, /* hibench_lpm3        32MB p37 */
    {PART_SECFLASH_AGEING,   34810 * 1024,    32 * 1024, UFS_PART_5}, /* secflash_ageing     32MB p38 */
    {PART_USERDATA,          34842 * 1024,(4UL) * 1024 * 1024, UFS_PART_5}, /* userdata        4G p39 */
#elif defined CONFIG_PARTITION_ENG
    {PART_FTTEST,            33594 * 1024,   192 * 1024, UFS_PART_5}, /* fttest             192MB p33 */
    {PART_DDRTEST,           33786 * 1024,    48 * 1024, UFS_PART_5}, /* ddrtest             48MB p34 */
    {PART_HITEST_GPU,        33834 * 1024,   512 * 1024, UFS_PART_5}, /* hitest_gpu         512MB p35 */
    {PART_HITEST_ASP_DHRYSTONE, 34346 * 1024,    20 * 1024, UFS_PART_5}, /* hitest_asp_dhrystone  20MB p36 */
    {PART_HITEST_ASP_TEST,   34366 * 1024,   300 * 1024, UFS_PART_5}, /* hitest_asp_test    300MB p37 */
    {PART_HITEST_SENSORHUB,  34666 * 1024,     6 * 1024, UFS_PART_5}, /* hitest_sensorhub     6MB p38 */
    {PART_HITEST_ISP,        34672 * 1024,    25 * 1024, UFS_PART_5}, /* hitest_isp          25MB p39 */
    {PART_HITEST_ISP_AVS,    34697 * 1024,    25 * 1024, UFS_PART_5}, /* hitest_isp_avs      25MB p40 */
    {PART_HITEST_NPU,        34722 * 1024,    80 * 1024, UFS_PART_5}, /* hitest_npu          80MB p41 */
    {PART_HITEST_DSS,        34802 * 1024,    18 * 1024, UFS_PART_5}, /* hitest_dss          18MB p42 */
    {PART_DSS_DISP_BUF,      34820 * 1024,    64 * 1024, UFS_PART_5}, /* dss_disp_buf        64MB p43 */
    {PART_HITEST_JPEGENC,    34884 * 1024,   200 * 1024, UFS_PART_5}, /* hitest_jpegenc     200MB p44 */
    {PART_HITEST_HIPP,       35084 * 1024,   100 * 1024, UFS_PART_5}, /* hitest_hipp        100MB p45 */
    {PART_HIBENCH_LPM3,      35184 * 1024,     2 * 1024, UFS_PART_5}, /* hibench_lpm3         2MB p46 */
    {PART_JPEG_DEC_TEST_SRC, 35186 * 1024,    20 * 1024, UFS_PART_5}, /* jpeg_dec_test_src   20MB p47 */
    {PART_JPEG_DEC_TEST_DEST, 35206 * 1024,    80 * 1024, UFS_PART_5}, /* jpeg_dec_test_dest  80MB p48 */
    {PART_VCODE_DATA,        35286 * 1024,   200 * 1024, UFS_PART_5}, /* vcode_data         200MB p49 */
    {PART_VCODE_MAX_DATA,    35486 * 1024,   140 * 1024, UFS_PART_5}, /* vcode_max_data     140MB p50 */
    {PART_HITEST_SAFETY_ISLAND, 35626 * 1024,     4 * 1024, UFS_PART_5}, /* hitest_safety_island   4MB p51 */
    {PART_USERDATA,          35630 * 1024,(4UL) * 1024 * 1024, UFS_PART_5}, /* userdata        4G p52 */
#else
    {PART_USERDATA,          33594 * 1024,(4UL) * 1024 * 1024, UFS_PART_5}, /* userdata        4G p33 */
#endif
    {"0", 0, 0, 0},
};

#endif
