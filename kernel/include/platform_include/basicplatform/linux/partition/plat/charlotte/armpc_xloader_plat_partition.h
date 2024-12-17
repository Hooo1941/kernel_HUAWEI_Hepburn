#ifndef _PISCES_XLOADER_PLAT_PARTITION_OHOS_H_
#define _PISCES_XLOADER_PLAT_PARTITION_OHOS_H_

#include "partition_macro.h"
#include "partition_macro_plus.h"


#define XLOADER3_VRL_INDEX_A    10
#define BL2_VRL_INDEX           3
#define BL2_VRL_INDEX_B         4
#define FASTBOOT_VRL_INDEX_A    28
#define VECTOR_VRL_INDEX_A      30

static const struct partition partition_table_emmc[] = {
    {"0", 0, 0, 0},
};

static const struct partition partition_table_ufs[] = {
    {PART_XLOADER_A,                      0,              4 * 1024,      UFS_PART_0},
    {PART_XLOADER_B,                    0,                4 * 1024,      UFS_PART_1},
    {PART_BOOT_CTRL,            (2 * 1024 + 512),   512, UFS_PART_2}, /* boot ctrl        512K    p5 */
    {PART_NVME,                  3 * 1024,     4 * 1024, UFS_PART_2}, /* nvme                 4MB p6 */
    {PART_DDR_PARA,            103 * 1024,     1 * 1024, UFS_PART_2}, /* ddr_para          1024KB p8 */
    {PART_LOWPOWER_PARA_A,     104 * 1024,     1 * 1024, UFS_PART_2}, /* lowpower_para_a   1024KB p9 */
    {PART_LOWPOWER_PARA_B,     105 * 1024,     1 * 1024, UFS_PART_2}, /* lowpower_para_b   1024KB p10 */
    {PART_DFX,                 106 * 1024,    16 * 1024, UFS_PART_2}, /* dfx                 16MB p11 */
    {PART_BL2_A,               122 * 1024,     4 * 1024, UFS_PART_2}, /* bl2_a                4MB p12 */
    {PART_BL2_B,               126 * 1024,     4 * 1024, UFS_PART_2}, /* bl2_b                4MB p13 */
    {PART_FW_CPU_LPCTRL_A,     131 * 1024,          256, UFS_PART_2}, /* fw_cpu_lpctrl_a    256KB p16 */
    {PART_FW_CPU_LPCTRL_B,       (131 * 1024 + 256),           256,      UFS_PART_2}, /* fw_cpu_lpctrl    256K    p17 */
    {PART_FW_GPU_LPCTRL_A,       (131 * 1024 + 512),           128,      UFS_PART_2}, /* fw_gpu_lpctrl    128K    p18 */
    {PART_FW_GPU_LPCTRL_B,       (131 * 1024 + 640),           128,      UFS_PART_2}, /* fw_gpu_lpctrl    128K    p19 */
    {PART_FW_DDR_LPCTRL_A,       (131 * 1024 + 768),           128,      UFS_PART_2}, /* fw_ddr_lpctrl    128K    p20 */
    {PART_FW_DDR_LPCTRL_B,       (131 * 1024 + 896),           128,      UFS_PART_2}, /* fw_ddr_lpctrl    128K    p21 */
    {PART_FASTBOOT_A,          134 * 1024,    12 * 1024, UFS_PART_2}, /* fastboot_a          12MB p26 */
    {PART_FASTBOOT_B,          146 * 1024,    12 * 1024, UFS_PART_2}, /* fastboot_b          12MB p27 */
    {PART_VECTOR_A,            158 * 1024,     4 * 1024, UFS_PART_2}, /* vector_a             4MB p28 */
    {PART_VECTOR_B,            162 * 1024,     4 * 1024, UFS_PART_2}, /* vector_b             4MB p29 */
#ifdef FACTORY_VERSION
    {PART_HIBENCH_IMG,       123906 * 1024,   128 * 1024,              UFS_PART_3}, /* hibench_img        128MB p87 */
    {PART_HIBENCH_LOG,       125058 * 1024,    32 * 1024,              UFS_PART_3}, /* hibench_log         32MB p90 */
#endif
    {"0", 0, 0, 0},
};

#endif