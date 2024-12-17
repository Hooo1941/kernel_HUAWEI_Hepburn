/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description: board_bfgx.c header file
 * Author: @CompanyNameTag
 */

#ifndef BOARD_BFGX_H
#define BOARD_BFGX_H
#include "bfgx_core.h"

struct bfgx_prj_desc {
    struct bfgx_subsys_dev_desc *subsys_desc;
    int32_t desc_num;
};

struct bfgx_prj_desc *board_bfgx_get_prj_desc(void);
#endif // BOARD_BFGX_H
