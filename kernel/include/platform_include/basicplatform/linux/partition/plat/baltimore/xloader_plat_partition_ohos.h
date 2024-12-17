#ifndef _PISCES_XLOADER_PLAT_PARTITION_OHOS_H_
#define _PISCES_XLOADER_PLAT_PARTITION_OHOS_H_

#include "partition_macro.h"
#include "partition_macro_plus.h"


#define XLOADER3_VRL_INDEX_A    20
#define BL2_VRL_INDEX           30
#define FASTBOOT_VRL_INDEX_A    39
#define VECTOR_VRL_INDEX_A      40

static const struct partition partition_table_emmc[] = {
    {"0", 0, 0, 0},
};

static const struct partition partition_table_ufs[] = {
    {PART_XLOADER,                0,                 2 * 1024,    UFS_PART_0},
    {PART_RESERVED1,              7 * 1024,              1024,    UFS_PART_2}, /* reserved1       1024K    p3 */
    {PART_VRL,                    512,                    512,    UFS_PART_3}, /* vrl              512K    p1 */
    {PART_VRL_BACKUP,             1024,                   512,    UFS_PART_3}, /* vrl backup       512K    p2 */
    {PART_NVME,                   10 * 1024,           5 * 1024,    UFS_PART_3}, /* nvme               5M    p4 */
    {PART_DDR_PARA,               227 * 1024,          1 * 1024,    UFS_PART_3}, /* ddr_para           1M    p13 */
    {PART_LOWPOWER_PARA,          228 * 1024,          1 * 1024,    UFS_PART_3}, /* lowpower_para      1M    p14 */
    {PART_DFX,                    439 * 1024,         16 * 1024,    UFS_PART_3}, /* dfx               16M    p19 */
    {PART_BL2,                    492 * 1024,          4 * 1024,    UFS_PART_3}, /* bl2                4M    p24 */
    {PART_HISEE_IMG,              510 * 1024,          4 * 1024,    UFS_PART_3}, /* hisee_img     4M    p30 */
    {PART_HISEE_FS,               518 * 1024,          8 * 1024,    UFS_PART_3}, /* hisee_fs           8M    p32 */
    {PART_FASTBOOT,               526 * 1024,         12 * 1024,    UFS_PART_3}, /* fastboot          12M    p33 */
    {PART_VECTOR,                 538 * 1024,          4 * 1024,    UFS_PART_3}, /* vector             4M    p34 */
#ifdef FACTORY_VERSION
    {PART_HIBENCH_IMG,          12248 * 1024,        128 * 1024,    UFS_PART_3}, /* hibench_img      128M    p78 */
    {PART_HIBENCH_LOG,          13400 * 1024,         32 * 1024,    UFS_PART_3}, /* hibench_log       32M    p81 */
#endif
    {"0", 0, 0, 0},
};

#endif
