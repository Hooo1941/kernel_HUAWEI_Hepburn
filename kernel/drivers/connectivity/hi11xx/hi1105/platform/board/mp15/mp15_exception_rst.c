/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description: Rectifying MP15 Exceptions
 * Author: @CompanyNameTag
 */

/* 头文件包含 */
#include "board.h"
#include "plat_exception_rst.h"
#include "mp13_exception_rst.h"
#include "mp15_exception_rst.h"


excp_chip_cfg_t g_mp15_chip_cfg = {
    .chip_type = BOARD_VERSION_MP15,
    .chip_name = BOARD_VERSION_NAME_MP15
};
excp_chip_cfg_t *get_mp15_chip_cfg(void)
{
    excp_chip_cfg_t *chip_mp13_cfg = get_mp13_chip_cfg();
    memcpy_s(&g_mp15_chip_cfg, sizeof(excp_chip_cfg_t), chip_mp13_cfg, sizeof(excp_chip_cfg_t));
    g_mp15_chip_cfg.chip_type = BOARD_VERSION_MP15;
    g_mp15_chip_cfg.chip_name = BOARD_VERSION_NAME_MP15;
    return &g_mp15_chip_cfg;
}
