/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description: BFGX board-level external implementation
 * Author: @CompanyNameTag
 */

#include "bfgx_mp16c.h"
#include "bfgx_gf61.h"
#include "bfgx_mp16.h"
#include "bfgx_mp13.h"
#include "bfgx_mp15.h"
#include "plat_debug.h"
#include "board.h"
#include "board_bfgx.h"

struct board_bfgx_chip_ops {
    struct bfgx_prj_desc* (*init)(void);
    void (*remove)(void);
};

struct board_bfgx_chip {
    int32_t chip_type;
    struct board_bfgx_chip_ops ops;
};

STATIC struct board_bfgx_chip bd_chip[] = {
    {
        .chip_type = BOARD_VERSION_MP16C,
        .ops = {
            .init = mp16c_bfgx_prj_desc_init,
            .remove = mp16c_bfgx_prj_desc_remove
        }
    },
    {
        .chip_type = BOARD_VERSION_MP16,
        .ops = {
            .init = mp16_bfgx_prj_desc_init,
            .remove = mp16_prj_desc_remove
        }
    },
    {
        .chip_type = BOARD_VERSION_MP13,
        .ops = {
            .init = mp13_bfgx_prj_desc_init,
            .remove = mp13_prj_desc_remove
        }
    },
    {
        .chip_type = BOARD_VERSION_MP15,
        .ops = {
            .init = mp15_bfgx_prj_desc_init,
            .remove = mp15_prj_desc_remove
        }
    },
    {
        .chip_type = BOARD_VERSION_GF61,
        .ops = {
            .init = gf61_bfgx_prj_desc_init,
            .remove = gf61_prj_desc_remove
        }
    }
};

struct bfgx_prj_desc *board_bfgx_get_prj_desc(void)
{
    int32_t i, chiptype;

    chiptype = get_mpxx_subchip_type();
    if (chiptype < 0) {
        ps_print_err("chip type get error\n");
        return NULL;
    }

    for (i = 0; i < oal_array_size(bd_chip); i++) {
        if (bd_chip[i].chip_type == chiptype) {
            return bd_chip[i].ops.init();
        }
    }

    return NULL;
}
