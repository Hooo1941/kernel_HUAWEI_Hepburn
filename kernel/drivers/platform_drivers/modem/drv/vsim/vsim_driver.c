/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/module.h>
#include <product_config.h>
#include <securec.h>
#include <bsp_msg.h>
#include <msg_id.h>
#include <mdrv_bmi_vsim.h>
#include "vsim_driver.h"

#define THIS_MODU mod_vsim

int mdrv_vsim_auth_request_register(event_cb_func vsim_request_cb)
{
    vsim_print_error("vsim send auth request is stub.\n");
    return VSIM_ERROR;
}

EXPORT_SYMBOL(mdrv_vsim_auth_request_register);
