/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description: gf61 gpio board-level implementation
 * Author: @CompanyNameTag
 */

#include <linux/device.h>
#include <linux/of_gpio.h>

#include "plat_debug.h"
#include "oal_hcc_bus.h"
#include "plat_gpio.h"
#include "oal_ext_if.h"

#ifdef _PRE_CONFIG_USE_DTS
/*
 * 车机产品有独立域复位功能，复位时，要在车机的微内核中操作各EN拉低。HOST驱动感知不到。
 * gf61的电源芯片较为特殊，拉低后，需要延时500ms才能完全断电，HOST驱动初始化获取chip power on管脚时的拉
 * 低动作保证不了延时500ms
 * 所以做定制化处理，一启动，检测到chip power on管脚为高，则先拉低。
 */
static int32_t board_get_host_independent_domain_reset_enable(void)
{
    int32_t value = 0;
    int32_t ret;
    int32_t enable = OAL_FALSE;

    ret = get_cust_conf_int32(INI_MODU_PLAT, "plat_host_independent_domain_reset_enable", &value);
    if ((ret == INI_SUCC) && (value == 1)) {
        enable = OAL_TRUE;
        ps_print_info("plat_host_independent_domain_reset_enable enable");
    }
    return enable;
}

static void board_chip_power_en_gpio_init(void)
{
    int32_t ret = board_get_host_independent_domain_reset_enable();
    int32_t physical_gpio = 0;
    int32_t level;

    if (ret == OAL_FALSE) {
        return;
    }
    ret = get_board_gpio(DTS_NODE_HISI_MPXX, DTS_PROP_GPIO_MPXX_POWEN_ON, &physical_gpio);
    if (ret != BOARD_SUCC) {
        ps_print_info("get dts prop %s failed\n", DTS_PROP_GPIO_MPXX_POWEN_ON);
        return;
    }
    ret = gpio_request(physical_gpio, PROC_NAME_GPIO_POWEN_ON);
    if (ret != BOARD_SUCC) {
        ps_print_err("(%s)_request_in fail.\n", PROC_NAME_GPIO_POWEN_ON);
        return;
    }
    level = gpio_get_value(physical_gpio);
    if (level != 1) {
        gpio_free(physical_gpio);
        return;
    }

    gpio_direction_output(physical_gpio, 0);
    ps_print_warning("chip power en enable from start");
    oal_msleep(500); // power en拉低后，延时500ms
    ps_print_warning("chip power en disable for 500ms");
    gpio_free(physical_gpio);
}
#endif

#ifdef _PRE_QISHAN_BOARD
/*
 * 产品qishanV0板bfgx wakeup gpio 为特殊的OD门GPIO，要先配置成输出高，才能再配置成输入
 */
void board_qishan_gpio_handle(void)
{
    int32_t ret;
    int32_t physical_gpio = 0;
    /* bfgx wake host gpio request */
    ret = get_board_gpio(DTS_NODE_MPXX_BFGX, DTS_PROP_MPXX_GPIO_BFGX_WAKEUP_HOST, &physical_gpio);
    if (ret != BOARD_SUCC) {
        ps_print_info("get dts prop %s failed\n", DTS_PROP_MPXX_GPIO_BFGX_WAKEUP_HOST);
        return;
    }

    ret = board_gpio_request(physical_gpio, PROC_NAME_GPIO_BFGX_WAKEUP_HOST, GPIO_DIRECTION_OUTPUT);
    if (ret != BOARD_SUCC) {
        ps_print_err("request gpio %s failed\n", PROC_NAME_GPIO_BFGX_WAKEUP_HOST);
        return;
    }
    oal_gpio_direction_output(physical_gpio, GPIO_HIGHLEVEL);
    gpio_free(physical_gpio);
}
#endif

STATIC struct conn_gpio_node g_gpio_node[] = {
    {
        .pin = {
            .name = PROC_NAME_GPIO_POWEN_ON,
#if defined(_PRE_PRODUCT_SYLINCOM) || defined(_PRE_PRODUCT_SLCONTEST)
            .is_key_gpio = OAL_FALSE,
#else
            .is_key_gpio = OAL_TRUE,
#endif
            .type = CHIP_POWR,
#ifdef _PRE_EAGLE_HOST_BOARD
            .invert = OAL_TRUE,
#endif
            .init = {
                .req_sts = REQ_ONE,
#if (defined _PRE_QISHAN_BOARD) || (defined _PRE_EAGLE_CAMERA_BOARD)
                .flags = GPIOF_OUT_INIT_HIGH
#else
                .flags = GPIOF_OUT_INIT_LOW
#endif
            },
        },
        .attr = {
            .mode = MODE_DTS,
            .u = {
                .dts = {
                    .compatibale = DTS_NODE_HISI_MPXX,
                    .label_name = DTS_PROP_GPIO_MPXX_POWEN_ON
                }
            },
        }
    },
    {
        .pin = {
            .name = PROC_NAME_GPIO_G_POWEN_ON,
            .is_key_gpio = OAL_FALSE,
            .type = G_POWER,
            .init = {
                .req_sts = REQ_ONE,
#ifdef _PRE_QISHAN_BOARD
                .flags = GPIOF_OUT_INIT_HIGH
#else
                .flags = GPIOF_OUT_INIT_LOW
#endif
            },
        },
        .attr = {
            .mode = MODE_DTS,
            .u = {
                .dts = {
                    .compatibale = DTS_NODE_HISI_MPXX,
                    .label_name = DTS_PROP_GPIO_G_POWEN_ON_ENABLE
                }
            },
        }
    },
#ifndef BFGX_UART_DOWNLOAD_SUPPORT
    {
        .pin = {
            .name = PROC_NAME_GPIO_WLAN_POWEN_ON,
            .is_key_gpio = OAL_FALSE,
            .type = W_POWER,
            .init = {
                .req_sts = REQ_ONE,
#ifdef _PRE_QISHAN_BOARD
                .flags = GPIOF_OUT_INIT_HIGH
#else
                .flags = GPIOF_OUT_INIT_LOW
#endif
            },
        },
        .attr = {
            .mode = MODE_DTS,
            .u = {
                .dts = {
                    .compatibale = DTS_NODE_HISI_MPXX,
                    .label_name = DTS_PROP_GPIO_WLAN_POWEN_ON_ENABLE
                }
            },
        }
    },
    {
        .pin = {
            .name = PROC_NAME_GPIO_GT_POWEN_ON,
#if defined(_PRE_PRODUCT_SYLINCOM) || defined(_PRE_PRODUCT_SLCONTEST)
            .is_key_gpio = OAL_FALSE,
#else
            .is_key_gpio = OAL_TRUE,
#endif
            .type = GT_POWER,
#ifdef _PRE_EAGLE_HOST_BOARD
            .invert = OAL_TRUE,
#endif
            .init = {
                .req_sts = REQ_ONE,
#if (defined _PRE_QISHAN_BOARD) || (defined _PRE_EAGLE_CAMERA_BOARD)
                .flags = GPIOF_OUT_INIT_HIGH
#else
                .flags = GPIOF_OUT_INIT_LOW
#endif
            },
        },
        .attr = {
            .mode = MODE_DTS,
            .u = {
                .dts = {
                    .compatibale = DTS_NODE_HISI_MPXX,
                    .label_name = DTS_PROP_GPIO_GT_POWEN_ON_ENABLE
                }
            },
        }
    },
#endif
    {
        .pin = {
            .name = PROC_NAME_GPIO_BFGX_POWEN_ON,
            .is_key_gpio = OAL_FALSE,
            .type = BFGX_POWER,
            .init = {
                .req_sts = REQ_ONE,
#ifdef _PRE_QISHAN_BOARD
                .flags = GPIOF_OUT_INIT_HIGH
#else
                .flags = GPIOF_OUT_INIT_LOW
#endif
            },
        },
        .attr = {
            .mode = MODE_DTS,
            .u = {
                .dts = {
                    .compatibale = DTS_NODE_HISI_MPXX,
                    .label_name = DTS_PROP_GPIO_BFGX_POWEN_ON_ENABLE
                }
            },
        }
    },
    {
        .pin = {
            .name = PROC_NAME_GPIO_SSI_CLK,
            .is_key_gpio = OAL_FALSE,
            .type = SSI_CLK,
            .init = {
                .req_sts = REQ_ONE,
                .flags = GPIOF_OUT_INIT_LOW
            },
        },
        .attr = {
            .mode = MODE_DTS,
            .u = {
                .dts = {
                    .compatibale = DTS_NODE_HISI_MPXX,
                    .label_name = DTS_PROP_GPIO_MPXX_GPIO_SSI_CLK
                }
            },
        }
    },
    {
        .pin = {
            .name = PROC_NAME_GPIO_SSI_DATA,
            .is_key_gpio = OAL_FALSE,
            .type = SSI_DATA,
            .init = {
                .req_sts = REQ_ONE,
                .flags = GPIOF_OUT_INIT_LOW
            },
        },
        .attr = {
            .mode = MODE_DTS,
            .u = {
                .dts = {
                    .compatibale = DTS_NODE_HISI_MPXX,
                    .label_name = DTS_PROP_GPIO_MPXX_GPIO_SSI_DATA
                }
            },
        }
    },
#ifndef BFGX_UART_DOWNLOAD_SUPPORT
    {
        .pin = {
            .name = PROC_NAME_GPIO_HOST_WAKEUP_GT,
            .is_key_gpio = OAL_FALSE,
            .type = HOST_WKUP_GT,
            .init = {
                .req_sts = REQ_ONE,
#ifdef _PRE_QISHAN_BOARD
                .flags = GPIOF_OUT_INIT_HIGH
#else
                .flags = GPIOF_OUT_INIT_LOW
#endif
            },
        },
        .attr = {
            .mode = MODE_DTS,
            .u = {
                .dts = {
                    .compatibale = DTS_NODE_HISI_GT,
                    .label_name = DTS_PROP_GPIO_HOST_WAKEUP_GT
                }
            },
        }
    },
#endif
    {
        .pin = {
            .name = PROC_NAME_GPIO_HOST_WAKEUP_SLE,
            .is_key_gpio = OAL_FALSE,
            .type = HOST_WKUP_SLE,
            .init = {
                .req_sts = REQ_ONE,
#ifdef _PRE_QISHAN_BOARD
                .flags = GPIOF_OUT_INIT_HIGH
#else
                .flags = GPIOF_OUT_INIT_LOW
#endif
            },
        },
        .attr = {
            .mode = MODE_DTS,
            .u = {
                .dts = {
                    .compatibale = DTS_NODE_MPXX_SLE,
                    .label_name = DTS_PROP_GPIO_HOST_WAKEUP_SLE
                }
            },
        }
    },
#ifndef BFGX_UART_DOWNLOAD_SUPPORT
    {
        .pin = {
            .name = PROC_NAME_GPIO_HOST_WAKEUP_WLAN,
            .is_key_gpio = OAL_FALSE,
            .type = HOST_WKUP_W,
            .init = {
                .req_sts = REQ_ONE,
#ifdef _PRE_QISHAN_BOARD
                .flags = GPIOF_OUT_INIT_HIGH
#else
                .flags = GPIOF_OUT_INIT_LOW
#endif
            },
        },
        .attr = {
            .mode = MODE_DTS,
            .u = {
                .dts = {
                    .compatibale = DTS_NODE_MPXX_WIFI,
                    .label_name = DTS_PROP_GPIO_HOST_WAKEUP_WLAN
                }
            },
        }
    },
#endif
    {
        .pin = {
            .name = PROC_NAME_GPIO_BFGX_WAKEUP_HOST,
            .is_key_gpio = OAL_FALSE,
            .type = BFGX_WKUP_HOST,
            .init = {
                .req_sts = REQ_ONE,
                .flags = GPIOF_IN
            },
        },
        .attr = {
            .mode = MODE_DTS,
            .u = {
                .dts = {
                    .compatibale = DTS_NODE_MPXX_BFGX,
                    .label_name = DTS_PROP_MPXX_GPIO_BFGX_WAKEUP_HOST
                }
            },
        },
        .irq = {
            .name = "bfgx_wake_host",
            .flags = IRQF_DISABLED | IRQF_TRIGGER_RISING | IRQF_NO_SUSPEND,
            .init_set_enable = OAL_FALSE
        },
    },
#ifndef BFGX_UART_DOWNLOAD_SUPPORT
    {
        .pin = {
            .name = PROC_NAME_GPIO_GT_WAKEUP_HOST,
            .is_key_gpio = OAL_FALSE,
            .type = GT_WKUP_HOST,
            .init = {
                .req_sts = REQ_ONE,
                .flags = GPIOF_IN
            },
        },
        .attr = {
            .mode = MODE_DTS,
            .u = {
                .dts = {
                    .compatibale = DTS_NODE_HISI_GT,
                    .label_name = DTS_PROP_GPIO_GT_WAKEUP_HOST
                }
            },
        },
        .irq = {
            .name = "gt_wake_host",
            .flags = IRQF_NO_SUSPEND | IRQF_TRIGGER_RISING | IRQF_DISABLED,
            .init_set_enable = OAL_FALSE
        },
    },
    {
        .pin = {
            .name = PROC_NAME_GPIO_WLAN_WAKEUP_HOST,
            .is_key_gpio = OAL_FALSE,
            .type = W_WKUP_HOST,
            .init = {
                .req_sts = REQ_ONE,
                .flags = GPIOF_IN
            },
        },
        .attr = {
            .mode = MODE_DTS,
            .u = {
                .dts = {
                    .compatibale = DTS_NODE_MPXX_WIFI,
                    .label_name = DTS_PROP_MPXX_GPIO_WLAN_WAKEUP_HOST
                }
            },
        },
        .irq = {
            .name = "wlan_wake_host",
            .flags = IRQF_NO_SUSPEND | IRQF_TRIGGER_RISING | IRQF_DISABLED,
            .init_set_enable = OAL_FALSE
        },
    },
#endif
    {
        .pin = {
            .name = PROC_NAME_GPIO_SLE_WAKEUP_HOST,
            .is_key_gpio = OAL_FALSE,
            .type = G_WKUP_HOST,
            .init = {
                .req_sts = REQ_ONE,
                .flags = GPIOF_IN
            },
        },
        .attr = {
            .mode = MODE_DTS,
            .u = {
                .dts = {
                    .compatibale = DTS_NODE_MPXX_SLE,
                    .label_name = DTS_PROP_GPIO_SLE_WAKEUP_HOST
                }
            },
        },
        .irq = {
            .name = "gle_wake_host",
            .flags = IRQF_NO_SUSPEND | IRQF_TRIGGER_RISING | IRQF_DISABLED,
            .init_set_enable = OAL_FALSE
        },
    }
};

STATIC struct conn_gpio_chip g_drv_gpio_chip = {
    .node = g_gpio_node,
    .node_num = oal_array_size(g_gpio_node),
};

int32_t gf61_gpio_init(struct platform_device *pdev)
{
    int32_t ret;

#ifdef _PRE_QISHAN_BOARD
    board_qishan_gpio_handle();
#endif

#ifdef _PRE_CONFIG_USE_DTS
    board_chip_power_en_gpio_init();
#endif
    ret = conn_gpio_init(&g_drv_gpio_chip);
    if (ret < 0) {
        ps_print_err("gf61 gpio init fail\n");
        return ret;
    }

    return OAL_SUCC;
}

void gf61_gpio_remove(struct platform_device *pdev)
{
    conn_gpio_remove();
}

