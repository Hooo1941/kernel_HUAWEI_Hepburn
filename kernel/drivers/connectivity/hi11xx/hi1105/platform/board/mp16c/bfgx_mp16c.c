/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 *
 * Description: Creating an MP16C Product bfgx dev External Implementation.
 * Author: @CompanyNameTag
 */
#include "bfgx_bt.h"
#include "bfgx_ir.h"
#include "bfgx_gnss.h"
#include "bfgx_fm.h"
#include "bfgx_sle.h"
#include "bfgx_dev.h"
#include "bfgx_debug.h"
#include "board_bfgx.h"
#include "plat_pm.h"
#include "exception_common.h"

STATIC struct bfgx_subsys_dev_desc g_bfgx_dev_desc[] = {
    {
        .name = HW_BT_DEV_NAME, .uart = BUART, .get_driver_desc = get_bt_default_drv_desc
    },
    {
        .name = HW_BT1_DEV_NAME, .uart = GUART, .get_driver_desc = get_bt_default_drv_desc
    },
    {
        .name = HW_IR_DEV_NAME, .uart = BUART, .get_driver_desc = get_ir_default_drv_desc
    },
    {
        .name = HW_GNSS_DEV_NAME, .uart = GUART, .get_driver_desc = get_gnss_default_drv_desc
    },
    {
        .name = HW_FM_DEV_NAME, .uart = GUART, .get_driver_desc = get_fm_default_drv_desc
    },
    {
        .name = HW_SLE_DEV_NAME, .uart = GUART, .get_driver_desc = get_sle_default_drv_desc
    },
    {
        .name = HW_BFGDBG_DEV_NAME, .uart = BUART, .get_driver_desc = get_bfgxdbg_default_drv_desc
    },
    {
        .name = HW_BFGDBG_DEV_NAME0, .uart = GUART, .get_driver_desc = get_bfgxdbg_default_drv_desc
    },
    {
        .name = HW_EXCP_BFGX_DEV_NAME, .uart = BUART, .get_driver_desc = get_excp_default_drv_desc,
        .private_data = &g_bcpu_memdump_cfg
    },
    {
        .name = HW_EXCP_ME_DEV_NAME, .uart = GUART, .get_driver_desc = get_excp_default_drv_desc,
        .private_data = &g_gcpu_memdump_cfg
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

#ifdef CONFIG_MLPS_OPERATE_CONN
#define MLPS_MPXX 0
typedef int (*connectivity_callback)(int para);
int mlps_ops_conn_function_register(unsigned int tag, connectivity_callback func);
STATIC uint32_t g_sensorhub_register_count = 0;

STATIC int32_t sensorhub_slave_open(int32_t on)
{
    int32_t ret;
    struct ps_core_s *ps_core_d = NULL;
    struct pm_top* pm_top_data = pm_get_top();

    ps_core_d = ps_get_core_reference(GUART);
    if (ps_core_d == NULL) {
        ps_print_err("sensorhub get ps core fail\n");
        return -EFAULT;
    }

    mutex_lock(&(pm_top_data->host_mutex));
    if (on == OAL_TRUE) {
        ret = hw_bfgx_open(ps_core_d, BFGX_NFC);
    } else {
        ret = hw_bfgx_close(ps_core_d, BFGX_NFC);
    }
    mutex_unlock(&(pm_top_data->host_mutex));

    return ret;
}
#endif

struct bfgx_prj_desc *mp16c_bfgx_prj_desc_init(void)
{
    /* MLPS只允许注册一次，重复注册会返回失败 */
#ifdef CONFIG_MLPS_OPERATE_CONN
    if (g_sensorhub_register_count == 0) {
        if (mlps_ops_conn_function_register(MLPS_MPXX, sensorhub_slave_open) != 0) {
            ps_print_err("sensorhub node register fail\n");
        }
        g_sensorhub_register_count++;
    } else {
        ps_print_info("sensorhub node have register\n");
    }
#endif
    return &g_prj_desc;
}

void mp16c_bfgx_prj_desc_remove(void)
{
    /* MLPS开机会整个重启，没有close的接口 */
#ifdef CONFIG_MLPS_OPERATE_CONN
    if (g_sensorhub_register_count != 0) {
        g_sensorhub_register_count = 0;
    }
#endif
}
