/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description: Create the MP13 product bfgx dev for external implementation
 * Author: @CompanyNameTag
 */

#include "plat_pm.h"
#include "bfgx_bt.h"
#include "bfgx_ir.h"
#include "bfgx_gnss.h"
#include "bfgx_fm.h"
#include "bfgx_sle.h"
#include "bfgx_debug.h"
#include "board_bfgx.h"
#include "plat_debug.h"
#include "exception_common.h"

STATIC struct bfgx_subsys_dev_desc g_bfgx_dev_desc[] = {
    {
        .name = HW_BT_DEV_NAME, .uart = BUART, .get_driver_desc = get_bt_default_drv_desc
    },
    {
        .name = HW_IR_DEV_NAME, .uart = BUART, .get_driver_desc = get_ir_default_drv_desc
    },
    {
        .name = HW_GNSS_DEV_NAME, .uart = BUART, .get_driver_desc = get_gnss_default_drv_desc
    },
    {
        .name = HW_FM_DEV_NAME, .uart = BUART, .get_driver_desc = get_fm_default_drv_desc
    },
    {
        .name = HW_BFGDBG_DEV_NAME, .uart = BUART, .get_driver_desc = get_bfgxdbg_default_drv_desc
    },
    {
        .name = HW_EXCP_BFGX_DEV_NAME, .uart = BUART, .get_driver_desc = get_excp_default_drv_desc,
        .private_data = &g_bcpu_memdump_cfg
    },
    {
        .name = HW_EXCP_WIFI_DEV_NAME, .uart = BUART, .get_driver_desc = get_excp_default_drv_desc,
        .private_data = &g_wcpu_memdump_cfg
    }
};

STATIC struct bfgx_prj_desc g_prj_desc = {
    .subsys_desc = g_bfgx_dev_desc,
    .desc_num = oal_array_size(g_bfgx_dev_desc)
};

struct bfgx_prj_desc *mp13_bfgx_prj_desc_init(void)
{
    return &g_prj_desc;
}

void mp13_prj_desc_remove(void)
{
}
