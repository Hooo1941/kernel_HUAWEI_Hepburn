#ifndef _ANTLIA_XLOADER_PLAT_PARTITION_OHOS_H_
#define _ANTLIA_XLOADER_PLAT_PARTITION_OHOS_H_

#include "partition_macro.h"
#include "partition_macro_plus.h"


#define XLOADER3_VRL_INDEX_A    15
#define BL2_VRL_INDEX           31
#define FASTBOOT_VRL_INDEX_A    47
#define VECTOR_VRL_INDEX_A      48

static const struct partition partition_table_emmc[] = {
    {PART_PTABLE,                      0,           136, SPI_NOR_PART}, /* ptable            136K    p0  */
    {PART_BOOT_CTRL,                  136,            4, SPI_NOR_PART}, /* boot_ctrl            4KB p1 */
    {PART_BL2,                        140,          900, SPI_NOR_PART}, /* bl2                900KB p2 */
    {PART_FW_CPU_LPCTRL,             1040,          256, SPI_NOR_PART}, /* fw_cpu_lpctrl      256KB p3 */
    {PART_FW_GPU_LPCTRL,             1296,          128, SPI_NOR_PART}, /* fw_gpu_lpctrl      128KB p4 */
    {PART_FW_DDR_LPCTRL,             1424,          128, SPI_NOR_PART}, /* fw_ddr_lpctrl      128KB p5 */
    {PART_DDR_XPU_BOOT0,             1552,          136, SPI_NOR_PART}, /* ddr_xpu_boot0      136KB p6 */
    {PART_DDR_XPU_BOOT1,             1688,          136, SPI_NOR_PART}, /* ddr_xpu_boot1      136KB p7 */
    /* !!!Attention!!! cannot modify xloader_a offset */
    {PART_XLOADER_A,                 3380,         1024, SPI_NOR_PART}, /* xloader_a         1024KB p13 */
    {PART_NVME,                      4404,         1024, SPI_NOR_PART}, /* nvme              1024KB p13 */
#ifdef FACTORY_VERSION
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
    /* !!!Attention!!! cannot modify xloader_b offset */
    {PART_XLOADER_B,               18064,         1024, SPI_NOR_PART}, /* xxloader_b        1024KB p21 */
    {PART_DDR_PARA,                 19088,          256, SPI_NOR_PART}, /* ddr_para           256KB p22 */
    {"0", 0, 0, 0},
};

static const struct partition partition_table_ufs[] = {
    {"0", 0, 0, 0},
};
#endif
