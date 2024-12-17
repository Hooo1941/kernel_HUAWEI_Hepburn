#ifndef _PISCES_XLOADER_PLAT_PARTITION_OHOS_H_
#define _PISCES_XLOADER_PLAT_PARTITION_OHOS_H_

#include "partition_macro.h"
#include "partition_macro_plus.h"


#define XLOADER3_VRL_INDEX_A    22
#define BL2_VRL_INDEX           33
#define FASTBOOT_VRL_INDEX_A    50
#define VECTOR_VRL_INDEX_A      51

static const struct partition partition_table_emmc[] = {
    {"0", 0, 0, 0},
};

static const struct partition partition_table_ufs[] = {
    {PART_XLOADER,                0,                 2 * 1024,    UFS_PART_0},
    {PART_RESERVED1,              7 * 1024,              1024,    UFS_PART_2}, /* reserved1       1024K    p3 */
    {PART_VRL,                    512,                    512,    UFS_PART_3}, /* vrl              512K    p1 */
    {PART_VRL_BACKUP,             1024,                   512,    UFS_PART_3}, /* vrl backup       512K    p2 */
    {PART_NVME,                   10 * 1024,           4 * 1024,    UFS_PART_3}, /* nvme               4M    p4 */
    {PART_DDR_PARA,               227 * 1024,          1 * 1024,    UFS_PART_3}, /* ddr_para           1M    p14 */
    {PART_LOWPOWER_PARA,          228 * 1024,          1 * 1024,    UFS_PART_3}, /* lowpower_para      1M    p15 */
    {PART_DFX,                    335 * 1024,         16 * 1024,    UFS_PART_3}, /* dfx               16M    p20 */
    {PART_BL2,                    492 * 1024,          4 * 1024,    UFS_PART_3}, /* bl2                4M    p25 */
    {PART_FW_CPU_LPCTRL,         (496 * 1024 + 512),   256,    UFS_PART_3}, /* fw_cpu_lpctrl    256K    p27 */
    {PART_FW_GPU_LPCTRL,         (496 * 1024 + 768),   128,    UFS_PART_3}, /* fw_gpu_lpctrl    128K    p28 */
    {PART_FW_DDR_LPCTRL,         (496 * 1024 + 896),   128,    UFS_PART_3}, /* fw_ddr_lpctrl    128K    p29 */
    {PART_DDR_XPU_BOOT0,       (497 * 1024 + 128),   256, UFS_PART_3}, /* ddr_xpu_boot0     256KB p32 */
    {PART_DDR_XPU_BOOT1,       (497 * 1024 + 384),  256, UFS_PART_3}, /* ddr_xpu_boot1      256KB p33 */
    {PART_DDR_XPU_BOOT2,       (497 * 1024 + 640),  256, UFS_PART_3}, /* ddr_xpu_boot2      256KB p34 */
    {PART_UCE,                 (497 * 1024 + 896),  160, UFS_PART_3}, /* uce                160KB  p35 */
    {PART_HISEE_IMG,              516 * 1024,          4 * 1024,    UFS_PART_3}, /* hisee_img     4M    p41 */
    {PART_HISEE_FS,               524 * 1024,          8 * 1024,    UFS_PART_3}, /* hisee_fs           8M    p43 */
    {PART_FASTBOOT,               532 * 1024,         12 * 1024,    UFS_PART_3}, /* fastboot          12M    p44 */
    {PART_VECTOR,                 544 * 1024,          4 * 1024,    UFS_PART_3}, /* vector             4M    p45 */
#ifdef FACTORY_VERSION
    {PART_HIBENCH_IMG,          11878 * 1024,        128 * 1024,    UFS_PART_3}, /* hibench_img        128MB p92 */
    {PART_HIBENCH_LOG,          12006 * 1024,         32 * 1024,    UFS_PART_3}, /* hibench_log         32MB p93 */
    {PART_HITEST_LPM3,          13062 * 1024,          2 * 1024,    UFS_PART_3}, /* hitest_lpm3        2M    p94 */
#elif defined(CONFIG_BOOT_UEFI) && defined(CONFIG_PARTITION_ENG)
#ifdef CONFIG_HISI_DEBUG_FS
    {PART_FTTEST,               11878 * 1024,        192 * 1024,    UFS_PART_3}, /* fttest             192MB p96 */
#endif
#ifdef XLOADER_DDR_TEST
    {PART_DDRTEST,              12070 * 1024,         48 * 1024,    UFS_PART_3}, /* ddrtest             48MB p97 */
#endif

#endif
    {"0", 0, 0, 0},
};

#endif
