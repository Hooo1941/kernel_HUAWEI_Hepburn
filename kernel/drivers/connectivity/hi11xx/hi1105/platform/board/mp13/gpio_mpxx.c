/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description: MP13/MP15 gpio board-level implementation
 * Author: @CompanyNameTag
 */

#include <linux/device.h>
#include <linux/of_gpio.h>
#ifdef CONFIG_PINCTRL
#include <linux/pinctrl/consumer.h>
#endif

#include "plat_pm.h"
#include "plat_debug.h"
#include "gpio_hitv.h"
#include "oal_hcc_bus.h"
#ifdef _PRE_PRODUCT_HI1620S_KUNPENG
#include "chr_user.h"
#endif
#include "oal_ext_if.h"
#include "plat_gpio.h"

#ifdef _PRE_HI_DRV_GPIO
int PdmGetHwVer(void);                         /* Get board type. */
irqreturn_t bfg_wake_host_isr(int irq, void *dev_id);
STATIC gpio_ext_func *g_hisi_gpio_func = NULL;
#ifdef _PRE_PRODUCT_HI3751_PLATO
#define POWER_ON_ENABLE_GPIO (21 * 8 + 0)      /* GPIO21_0 */
#define WLAN_POWER_ON_ENABLE_GPIO (9 * 8 + 2)  /* GPIO9_2 */
#define BFGN_POWER_ON_ENABLE_GPIO (19 * 8 + 6) /* GPIO19_6 */
#define WLAN_WAKEUP_HOST_GPIO (21 * 8 + 1)     /* GPIOP21_1 */
#define HOST_WAKEUP_WLAN_GPIO (9 * 8 + 1)      /* STB_GPIO9_1 */
#else
#define POWER_ON_ENABLE_GPIO (20 * 8 + 5)      /* PMU ON STB_GPIOPWM20_5 */
#define WLAN_POWER_ON_ENABLE_GPIO (18 * 8 + 5) /* S_ADC2/STB_GPIO18_5 */
#define BFGN_POWER_ON_ENABLE_GPIO (18 * 8 + 4) /* LS_ADC1/STB_GPIO18_4 */
#define WLAN_WAKEUP_HOST_GPIO (19 * 8 + 3)     /* STB_GPIOPWM20_5/UART4_CTSN */
#define HOST_WAKEUP_WLAN_GPIO (19 * 8 + 7)     /* HDMI3_DET/STB_GPIOPWM19_7/TSI3_D7 */
#endif
#define BFGN_WAKEUP_HOST_GPIO (19 * 8 + 1)     /* STB_GPIOPWM19_1/UART3_RXD */
#define HOST_WAKEUP_BFGN_GPIO (20 * 8 + 6)
#define HOST_WAKEUP_BFGN_GPIO_HEGEL (20 * 8 + 1)

STATIC struct conn_gpio_node g_hi_drv_gpio_node[] = {
    {
        .pin = {
            .name = PROC_NAME_GPIO_POWEN_ON,
            .is_key_gpio = OAL_TRUE,
            .type = CHIP_POWR,
            .number = POWER_ON_ENABLE_GPIO,
            .init = {
                .req_sts = SKIP,
            },
        },
        .attr = {
            .mode = MODE_NULL,
        }
    },
    {
        .pin = {
            .name = PROC_NAME_GPIO_WLAN_POWEN_ON,
            .is_key_gpio = OAL_TRUE,
            .type = W_POWER,
            .number = WLAN_POWER_ON_ENABLE_GPIO,
            .init = {
                .req_sts = SKIP,
            },
        },
        .attr = {
            .mode = MODE_NULL,
        }
    },
    {
        .pin = {
            .name = PROC_NAME_GPIO_BFGX_POWEN_ON,
            .is_key_gpio = OAL_TRUE,
            .type = BFGX_POWER,
            .number = BFGN_POWER_ON_ENABLE_GPIO,
            .init = {
                .req_sts = SKIP,
            },
        },
        .attr = {
            .mode = MODE_NULL,
        }
    },
    {
        .pin = {
            .name = PROC_NAME_GPIO_SSI_CLK,
            .is_key_gpio = OAL_FALSE,
            .type = SSI_CLK,
            .number = SSI_CLK_HISI_GPIO,
            .init = {
                .req_sts = SKIP,
            },
        },
        .attr = {
            .mode = MODE_NULL,
        }
    },
    {
        .pin = {
            .name = PROC_NAME_GPIO_SSI_DATA,
            .is_key_gpio = OAL_FALSE,
            .type = SSI_DATA,
            .number = SSI_DATA_HISI_GPIO,
            .init = {
                .req_sts = SKIP,
            },
        },
        .attr = {
            .mode = MODE_NULL,
        }
    },
    {
        .pin = {
            .name = PROC_NAME_GPIO_HOST_WAKEUP_WLAN,
            .is_key_gpio = OAL_TRUE,
            .type = HOST_WKUP_W,
            .number = HOST_WAKEUP_WLAN_GPIO,
            .init = {
                .req_sts = SKIP,
            },
        },
        .attr = {
            .mode = MODE_NULL,
        }
    },
    {
        .pin = {
            .name = PROC_NAME_GPIO_HOST_WAKEUP_BFG,
            .is_key_gpio = OAL_TRUE,
            .type = HOST_WKUP_BFGX,
            .number = HOST_WAKEUP_BFGN_GPIO_HEGEL,
            .init = {
                .req_sts = SKIP,
            },
        },
        .attr = {
            .mode = MODE_NULL,
        }
    },
};

STATIC int32_t hitv_ssi_request_gpio(int32_t gpio_nr)
{
    /* Oscar的v811主芯片不支持Linux的DTS机制，目前还在适配标准的内核GPIO，等小海思提供接口后该函数再另行整改
     * 地址 0xF8A21218 0x00000000 RegName:ctrl57; PadName:SD_D2; RegValue: 0x0--SD_D2,0x1--GPIOPWM6_7,0x2--TSI2_D7,
     *                            0x3--NA,0x4--NA,0x5--NA,0x6--NA,0x7--NA
     * 地址 0xF8A2121C 0x00000000 RegName:ctrl58; PadName:SD_D3; RegValue: 0x0--SD_D3,0x1--GPIOPWM7_0,0x2--TSI2_D2,
     */
    (void __iomem *) gpio_mux_addr = ioremap_nocache(GPIO_MUX_REG_ADDR, GPIO_MUX_REG_VALUE);
    if (gpio_mux_addr != NULL) {
        writel(1, gpio_mux_addr);
        writel(1, (void __iomem *)(uintptr_t)((unsigned long)(uintptr_t)gpio_mux_addr + GPIO_MUX_REG_OFFSET));
        iounmap(gpio_mux_addr);
    }

    hitv_gpio_request(gpio_nr, 0);
    hitv_gpio_direction_output(gpio_nr, GPIO_LEVEL_LOW);

    return OAL_SUCC;
}

STATIC int32_t hi_drv_gpio_init(struct conn_gpio_node *gpio_node)
{
    switch (gpio_node->pin.type) {
        case CHIP_POWR:
        case W_POWER:
            hitv_gpio_request(gpio_node->pin.number, 0);
            hitv_gpio_direction_output(gpio_node->pin.number, 1);
            return OAL_SUCC;
        case HOST_WKUP_W:
            hitv_gpio_request(gpio_node->pin.number, 0);
            hitv_gpio_direction_output(gpio_node->pin.number, 0);
            return OAL_SUCC;
        case HOST_WKUP_BFGX:
#ifdef _PRE_PRODUCT_HI3751_PLATO
            hitv_gpio_request(gpio_node->pin.number, 0);
            hitv_gpio_direction_output(gpio_node->pin.number, 0);
            ps_print_info("bfg plato wakeup gpio 20-1 \n");
#else
            if (PdmGetHwVer() == WIFI_PRODUCT_VER_OSCA_550 ||
                PdmGetHwVer() == WIFI_PRODUCT_VER_OSCA_550A) {
                gpio_node->pin.number = HOST_WAKEUP_BFGN_GPIO;
                hitv_gpio_request(gpio_node->pin.number);
                ps_print_info("bfg no hegel wakeup gpio still is 20-6 \n");
            } else {
                gpio_node->pin.number = HOST_WAKEUP_BFGN_GPIO_HEGEL;
                hitv_gpio_request(gpio_node->pin.number);
                /* Req PCIE_rst GPIO */
                hitv_gpio_request(gpio_node->pin.number, GPIO_LEVEL_LOW);
                /* 2.in hegel must change port mode to output. */
                hitv_gpio_direction_output(gpio_node->pin.number, 0);
                ps_print_info("bfg hegel wakeup gpio change to 20-1 \n");
            }
#endif
            break;
        case W_WKUP_HOST:
        case BFGX_WKUP_HOST:
            hitv_gpio_request(gpio_node->pin.number, 1);
            break;
        case SSI_CLK:
        case SSI_DATA:
            hitv_ssi_request_gpio(gpio_node->pin.number);
            break;
    }

    return OAL_SUCC;
}

STATIC void hi_drv_gpio_uninit(struct conn_gpio_node *gpio_node)
{
}

STATIC void bfgx_wake_host(uint32_t para1)
{
    g_hisi_gpio_func->pfn_gpio_clear_bit_int(BFGX_WKAEUP_HOST_GPIO);
    ps_print_info("bgfx_wake_host entering. --rising edge\n");
    (void)bfg_wake_host_isr(0, (void *)pm_get_drvdata(BUART));
}

STATIC void hitv_bfgx_wkup_int_init(void)
{
    int ret;

    ret = g_hisi_gpio_func->pfn_gpio_dir_set_bit(BFGX_WKAEUP_HOST_GPIO, 1);
    ret |= g_hisi_gpio_func->pfn_gpio_set_int_type(BFGX_WKAEUP_HOST_GPIO, BFGX_HI_UNF_GPIO_INTTYPE_UP);
    ps_print_err("HI_DRV_MODULE_GetFunction BT_GPIO TYPE_UP %08x \n", ret);
    ret |= g_hisi_gpio_func->pfn_gpio_register_server_func(BFGX_WKAEUP_HOST_GPIO, bfgx_wake_host);
    ret |= g_hisi_gpio_func->pfn_gpio_clear_bit_int(BFGX_WKAEUP_HOST_GPIO);
    ret |= g_hisi_gpio_func->pfn_gpio_set_int_enable(BFGX_WKAEUP_HOST_GPIO, BFGX_INT_ENABLE);
    ps_print_info("HI_DRV_MODULE_GetFunction BT_GPIO pfnGpio %08x \n", ret);
}

STATIC int32_t hi_drv_irq_register(struct conn_gpio_node *node, oal_irq_handler_t handler, void *dev)
{
    int32_t ret;
    ret = HI_DRV_MODULE_GetFunction(HI_ID_GPIO, (void**)(&g_hisi_gpio_func)); /* TV using hisi interfaces */
    if (ret) {
        return OAL_FAIL;
    }

    hitv_bfgx_wkup_int_init();
    node->irq.is_enabled = OAL_TRUE;
}

STATIC void hi_drv_irq_unregister(struct conn_gpio_node *node, void *dev)
{
}

STATIC uint32_t hi_drv_get_irq_num(int32_t pin_num)
{
    return OAL_SUCC;
}

STATIC int32_t hi_drv_set_gpio_level(struct conn_gpio_node *node, enum gpio_level level)
{
    hitv_gpio_request(node->pin.number, 0);
    return hitv_gpio_direction_output(node->pin.number, level);
}

STATIC enum gpio_level hi_drv_get_gpio_level(struct conn_gpio_node *node)
{
    return (enum gpio_level)hitv_gpio_get_value(node->pin.number);
}

STATIC int32_t hi_drv_set_gpio_input(struct conn_gpio_node *node)
{
    return hitv_gpio_request(node->pin.number, 1);
}

STATIC struct conn_gpio_ops g_hi_tv_ops = {
    .gpio_init = hi_drv_gpio_init,
    .gpio_uninit = hi_drv_gpio_uninit,
    .set_gpio_level = hi_drv_set_gpio_level,
    .get_gpio_level = hi_drv_get_gpio_level,
    .set_gpio_input = hi_drv_set_gpio_input,
    .get_irq_num = hi_drv_get_irq_num,
    .irq_register = hi_drv_irq_register,
    .irq_unregister = hi_drv_irq_unregister
};

STATIC struct conn_gpio_chip g_hi_tv_gpio_chip = {
    .node = g_hi_drv_gpio_node,
    .node_num = oal_array_size(g_hi_drv_gpio_node),
    .ops = &g_hi_tv_ops,
};

#elif defined(_PRE_PRODUCT_HI1620S_KUNPENG)
#define SSI_GPIO_GROUP_ID                       480
#define SSI_GPIO_CLK_INDEX                      27
#define SSI_GPIO_DATA_INDEX                     26
#define POWER_ON_GPIO_ACPI_INDEX                0
#define BFGN_POWER_ON_GPIO_ACPI_INDEX           2
#define WLAN_POWER_ON_GPIO_ACPI_INDEX           1
#define BFGN_WAKEUP_HOST_GPIO_ACPI_INDEX        8
#define WLAN_WAKEUP_HOST_GPIO_ACPI_INDEX        7
#define HOST_WAKEUP_WLAN_GPIO_ACPI_INDEX        3

STATIC struct conn_gpio_node g_kunpeng_drv_gpio_node[] = {
    {
        .pin = {
            .name = PROC_NAME_GPIO_POWEN_ON,
            .is_key_gpio = OAL_TRUE,
            .type = CHIP_POWR,
            .init = {
                .req_sts = SKIP,
                .flags = GPIOD_ASIS
            },
        },
        .attr = {
            .mode = MODE_GPIOLIB,
            .u = {
                .gpiod = {
                    .con_id = NULL,
                    .index = POWER_ON_GPIO_ACPI_INDEX,
                }
            },
        }
    },
#ifndef BFGX_UART_DOWNLOAD_SUPPORT
    {
        .pin = {
            .name = PROC_NAME_GPIO_WLAN_POWEN_ON,
            .is_key_gpio = OAL_TRUE,
            .type = W_POWER,
            .init = {
                .req_sts = SKIP,
                .flags = GPIOD_ASIS
            },
        },
        .attr = {
            .mode = MODE_GPIOLIB,
            .u = {
                .gpiod = {
                    .con_id = NULL,
                    .index = WLAN_POWER_ON_GPIO_ACPI_INDEX,
                }
            },
        }
    },
#endif
    {
        .pin = {
            .name = PROC_NAME_GPIO_BFGX_POWEN_ON,
            .is_key_gpio = OAL_TRUE,
            .type = BFGX_POWER,
            .init = {
                .req_sts = SKIP,
                .flags = GPIOD_ASIS
            },
        },
        .attr = {
            .mode = MODE_GPIOLIB,
            .u = {
                .gpiod = {
                    .con_id = NULL,
                    .index = BFGN_POWER_ON_GPIO_ACPI_INDEX,
                }
            },
        }
    },
    {
        .pin = {
            .name = PROC_NAME_GPIO_SSI_CLK,
            .is_key_gpio = OAL_FALSE,
            .type = SSI_CLK,
            .number = (SSI_GPIO_GROUP_ID + SSI_GPIO_CLK_INDEX),
            .init = {
                .req_sts = REQ_ONE,
                .flags = GPIOF_OUT_INIT_LOW
            },
        },
        .attr = {
            .mode = MODE_NULL,
        }
    },
    {
        .pin = {
            .name = PROC_NAME_GPIO_SSI_DATA,
            .is_key_gpio = OAL_FALSE,
            .type = SSI_DATA,
            .number = (SSI_GPIO_GROUP_ID + SSI_GPIO_DATA_INDEX),
            .init = {
                .req_sts = REQ_ONE,
                .flags = GPIOF_OUT_INIT_LOW
            },
        },
        .attr = {
            .mode = MODE_NULL,
        }
    },
#ifndef BFGX_UART_DOWNLOAD_SUPPORT
    {
        .pin = {
            .name = PROC_NAME_GPIO_HOST_WAKEUP_WLAN,
            .is_key_gpio = OAL_TRUE,
            .type = HOST_WKUP_W,
            .init = {
                .req_sts = SKIP,
                .flags = GPIOD_ASIS
            },
        },
        .attr = {
            .mode = MODE_GPIOLIB,
            .u = {
                .gpiod = {
                    .con_id = NULL,
                    .index = HOST_WAKEUP_WLAN_GPIO_ACPI_INDEX,
                }
            },
        }
    },
#endif
    {
        .pin = {
            .name = PROC_NAME_GPIO_BFGX_WAKEUP_HOST,
            .is_key_gpio = OAL_TRUE,
            .type = BFGX_WKUP_HOST,
            .init = {
                .req_sts = SKIP,
                .flags = GPIOD_ASIS
            },
        },
        .attr = {
            .mode = MODE_GPIOLIB,
            .u = {
                .gpiod = {
                    .con_id = NULL,
                    .index = BFGN_WAKEUP_HOST_GPIO_ACPI_INDEX,
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
            .name = PROC_NAME_GPIO_WLAN_WAKEUP_HOST,
            .is_key_gpio = OAL_TRUE,
            .type = W_WKUP_HOST,
            .init = {
                .req_sts = SKIP,
                .flags = GPIOD_ASIS
            },
        },
        .attr = {
            .mode = MODE_GPIOLIB,
            .u = {
                .gpiod = {
                    .con_id = NULL,
                    .index = WLAN_WAKEUP_HOST_GPIO_ACPI_INDEX,
                }
            },
        },
        .irq = {
            .name = "wlan_wake_host",
            .flags = IRQF_DISABLED | IRQF_TRIGGER_FALLING,
            .init_set_enable = OAL_FALSE
        },
    }
#endif
};

STATIC struct conn_gpio_chip g_kunpeng_drv_gpio_chip = {
    .node = g_kunpeng_drv_gpio_node,
    .node_num = oal_array_size(g_kunpeng_drv_gpio_node)
};

#else
STATIC struct conn_gpio_node g_mpxx_phone_drv_gpio_node[] = {
    {
        .pin = {
            .name = PROC_NAME_GPIO_POWEN_ON,
            .is_key_gpio = OAL_TRUE,
            .type = CHIP_POWR,
            .init = {
                .req_sts = REQ_ONE,
#ifdef _PRE_HI375X_PCIE
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
#ifndef BFGX_UART_DOWNLOAD_SUPPORT
    {
        .pin = {
            .name = PROC_NAME_GPIO_WLAN_POWEN_ON,
            .is_key_gpio = OAL_TRUE,
            .type = W_POWER,
            .init = {
                .req_sts = REQ_ONE,
#ifdef _PRE_HI375X_PCIE
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
#endif
    {
        .pin = {
            .name = PROC_NAME_GPIO_BFGX_POWEN_ON,
            .is_key_gpio = OAL_TRUE,
            .type = BFGX_POWER,
            .init = {
                .req_sts = REQ_ONE,
#ifdef _PRE_HI375X_PCIE
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
        },
    },
    {
        .pin = {
            .name = PROC_NAME_GPIO_SSI_CLK,
            .is_key_gpio = OAL_FALSE,
            .type = SSI_CLK,
            .init = {
                .req_sts = REQ_ONE,
#ifdef _PRE_HI375X_PCIE
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
#ifdef _PRE_HI375X_PCIE
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
                    .label_name = DTS_PROP_GPIO_MPXX_GPIO_SSI_DATA
                }
            },
        }
    },
#ifndef BFGX_UART_DOWNLOAD_SUPPORT
    {
        .pin = {
            .name = PROC_NAME_GPIO_HOST_WAKEUP_WLAN,
            .is_key_gpio = OAL_TRUE,
            .type = HOST_WKUP_W,
            .init = {
                .req_sts = REQ_ONE,
#ifdef _PRE_HI375X_PCIE
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
            .is_key_gpio = OAL_TRUE,
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
#ifdef _PRE_HI375X_PCIE
    {
        .pin = {
            .name = PROC_NAME_GPIO_HOST_WAKEUP_BFG,
            .is_key_gpio = OAL_FALSE,
            .type = HOST_WKUP_BFGX,
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
                    .label_name = DTS_PROP_MPXX_GPIO_HOST_WAKEUP_BFGX
                }
            },
        },
    },
#endif
#ifndef BFGX_UART_DOWNLOAD_SUPPORT
    {
        .pin = {
            .name = PROC_NAME_GPIO_WLAN_WAKEUP_HOST,
            .is_key_gpio = OAL_TRUE,
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
#if (KERNEL_VERSION(3, 10, 44) <= LINUX_VERSION_CODE)
            .flags = IRQF_NO_SUSPEND | IRQF_TRIGGER_RISING | IRQF_DISABLED,
#else
            .flags = IRQF_DISABLED | IRQF_TRIGGER_RISING,
#endif
            .init_set_enable = OAL_FALSE
        },
    }
#endif
};

STATIC struct conn_gpio_chip g_mpxx_drv_gpio_chip = {
    .node = g_mpxx_phone_drv_gpio_node,
    .node_num = oal_array_size(g_mpxx_phone_drv_gpio_node)
};

#endif

STATIC void wlan_wakeup_host_property_init(struct conn_gpio_chip *gpio_chip)
{
    int32_t ret;
    int32_t cfg_val;
    int32_t i;

    ret = get_cust_conf_int32(INI_MODU_PLAT, INI_WLAN_WAKEUP_HOST_REVERSE, &cfg_val);
    if (ret == INI_FAILED) {
        ps_print_warning("get %s failed\n", INI_WLAN_WAKEUP_HOST_REVERSE);
        return;
    }
    ps_print_warning("get %s = %d \n", INI_WLAN_WAKEUP_HOST_REVERSE, cfg_val);
    for (i = 0; i < gpio_chip->node_num; i++) {
        if ((gpio_chip->node)[i].pin.type == W_WKUP_HOST) {
            (gpio_chip->node)[i].pin.invert = (uint32_t)cfg_val;
            break;
        }
    }
}

int32_t mpxx_gpio_init(struct platform_device *pdev)
{
    int32_t ret;
    struct conn_gpio_chip *gpio_chip = NULL;

#ifdef _PRE_HI_DRV_GPIO
    gpio_chip = &g_hi_tv_gpio_chip;
#elif defined(_PRE_PRODUCT_HI1620S_KUNPENG)
    g_kunpeng_drv_gpio_chip.dev = (void *)pdev;
    gpio_chip = &g_kunpeng_drv_gpio_chip;
#else
    gpio_chip = &g_mpxx_drv_gpio_chip;
#endif
    ret = conn_gpio_init(gpio_chip);
    if (ret < 0) {
#ifdef _PRE_PRODUCT_HI1620S_KUNPENG
        chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_WIFI,
            CHR_LAYER_DRV, CHR_WIFI_DEV_EVENT_CHIP, CHR_PLAT_DRV_ERROR_POWER_GPIO);
#endif
        ps_print_err("mpxx gpio init fail\n");
        return ret;
    }

    wlan_wakeup_host_property_init(gpio_chip);

    return OAL_SUCC;
}

void mpxx_gpio_remove(struct platform_device *pdev)
{
    conn_gpio_remove();
}
