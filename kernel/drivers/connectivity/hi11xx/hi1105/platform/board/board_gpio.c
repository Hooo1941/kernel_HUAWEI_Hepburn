/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description: gpio board-level implementation
 * Author: @CompanyNameTag
 */

#include "plat_debug.h"
#include "gpio_mp16.h"
#include "gpio_mp16c.h"
#include "gpio_gf61.h"
#include "gpio_mpxx.h"
#include "plat_gpio.h"
#include "board.h"

struct board_gpio_ops {
    int32_t (*init)(struct platform_device *pdev);
    void (*remove)(struct platform_device *pdev);
};

struct board_gpio_chip_str {
    int32_t chip_type;
    struct board_gpio_ops ops;
};

STATIC struct board_gpio_chip_str g_board_gpio_chip_arr[] = {
    {
        BOARD_VERSION_MP13,
        .ops = {
            .init = mpxx_gpio_init,
            .remove = mpxx_gpio_remove
        }
    },
    {
        BOARD_VERSION_MP15,
        .ops = {
            .init = mpxx_gpio_init,
            .remove = mpxx_gpio_remove
        }
    },
    {
        BOARD_VERSION_MP16,
        .ops = {
            .init = mp16_gpio_init,
            .remove = mp16_gpio_remove
        }
    },
    {
        BOARD_VERSION_MP16C,
        .ops = {
            .init = mp16c_gpio_init,
            .remove = mp16c_gpio_remove
        }
    },
    {
        BOARD_VERSION_GF61,
        .ops = {
            .init = gf61_gpio_init,
            .remove = gf61_gpio_remove
        }
    },
};

struct board_gpio_str {
    int32_t chip_num;
    int32_t chip_index;
    int32_t inited;
    struct board_gpio_chip_str *gpio_chip;
    struct platform_device *pdev;
};

STATIC struct board_gpio_str g_board_gpio = {
    .gpio_chip = g_board_gpio_chip_arr,
    .chip_num = oal_array_size(g_board_gpio_chip_arr)
};

STATIC OAL_INLINE int32_t _board_gpio_init(struct board_gpio_chip_str *bd_chip,
                                           struct platform_device *pdev)
{
    int32_t ret;
    if (bd_chip->ops.init != NULL) {
        ret = bd_chip->ops.init(pdev);
        if (ret < 0) {
            return ret;
        }
    }

    return OAL_SUCC;
}

STATIC OAL_INLINE void _board_gpio_remove(struct board_gpio_chip_str *bd_chip,
                                          struct platform_device *pdev)
{
    if (bd_chip->ops.remove != NULL) {
        bd_chip->ops.remove(pdev);
    }
}

int32_t conn_board_gpio_init(struct platform_device *pdev)
{
    int32_t chiptype;
    uint32_t index;
    struct board_gpio_chip_str *gpio_chip;

    chiptype = get_mpxx_subchip_type();
    if (chiptype < 0) {
        ps_print_err("chip type get error\n");
        return -OAL_FAIL;
    }

    gpio_chip = g_board_gpio.gpio_chip;
    for (index = 0; index < g_board_gpio.chip_num; index++) {
        if (gpio_chip[index].chip_type == chiptype) {
            g_board_gpio.chip_index = index;
            break;
        }
    }

    if (_board_gpio_init(&gpio_chip[index], pdev) != OAL_SUCC) {
        ps_print_err("chip[%d] conn get gpio fail\n", gpio_chip[index].chip_type);
        _board_gpio_remove(&gpio_chip[index], pdev);
        return -OAL_FAIL;
    }

    g_board_gpio.inited = OAL_TRUE;

    return OAL_SUCC;
}

void conn_board_gpio_remove(struct platform_device *pdev)
{
    int32_t chip_index;
    struct board_gpio_chip_str *gpio_chip;

    if (g_board_gpio.inited != OAL_TRUE) {
        return;
    }

    chip_index = g_board_gpio.chip_index;
    gpio_chip = &(g_board_gpio.gpio_chip[chip_index]);

    _board_gpio_remove(gpio_chip, pdev);
    g_board_gpio.inited = OAL_FALSE;
}

