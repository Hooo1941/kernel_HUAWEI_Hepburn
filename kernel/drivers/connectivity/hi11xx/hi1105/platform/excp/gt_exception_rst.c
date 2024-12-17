/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description: gt_exception_function
 * Author: @CompanyNameTag
 */
#include "plat_debug.h"
#include "plat_exception_rst.h"
#include "gt_exception_rst.h"

int32_t gt_device_reset(void)
{
    ps_print_info("reset gt device by gt_en gpio\n");
    /* To implement later */

    return EXCEPTION_SUCCESS;
}

void gt_memdump_finish(void)
{
    g_gt_memdump_cfg.is_working = 0;
}

int32_t gt_notice_hal_memdump(void)
{
    return notice_hal_memdump(&g_gt_memdump_cfg, CMD_READM_GT_PCIE);
}

int32_t gt_memdump_enquenue(uint8_t *buf_ptr, uint16_t count)
{
    return excp_memdump_queue(buf_ptr, count, &g_gt_memdump_cfg);
}
