/**
 * Copyright (c) @CompanyNameMagicTag 2024-2024. All rights reserved. \n
 *
 * Description:  \n
 * Author: @CompanyNameTag \n
 * History: \n
 * 2024-01-04, Create file. \n
 */
#include "bsxx_board_port.h"
#include <linux/device.h>
#include <linux/of_gpio.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <linux/tty.h>
#include "tiot_board_pin_port.h"
#include "tiot_board_pin_irq_helper.h"
#include "tiot_board_log.h"
#include "tiot_dts_node.h"
#include "securec.h"

#define TTY_NAME_MAX    20
#define DTS_NODE_COMPATIBLE    "tiot,bsxx"

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0))
#define WAKEIN_IRQF    (IRQF_TRIGGER_RISING | IRQF_NO_SUSPEND)
#else
#define WAKEIN_IRQF    (IRQF_DISABLED | IRQF_TRIGGER_RISING | IRQF_NO_SUSPEND)
#endif

typedef struct {
    bool optional;     /* 是否为可选管脚 */
    const char *prop;  /* dts prop  */
    const char *name;  /* gpio request name */
    int gpio_flags;    /* gpio request 初始配置 */
} tiot_kernel_pin_info;

static uint32_t g_xfer_baudrate = 0;
static bsxx_board_hw_info g_board_hw_info = { 0 };
static bsxx_board_info g_board_info = {
    .cfg_path = NULL,                  /* 如果有文件系统，指向bsxx加载配置文件完整路径/相对路径
                                          例如：linux下"/sys/xxx/xxx/xxx_config.bin",
                                          windows下".\\config_file_path\\xxx_config.bin"
                                          如果无文件系统，设为NULL. */
    .hw_infos = &g_board_hw_info
};

static const tiot_kernel_pin_info g_board_pin_infos[BSXX_PIN_NUM] = {
    { true, DTS_PROP_GPIO_POWEN_ON, NULL, GPIOF_OUT_INIT_LOW },
    { true, DTS_PROP_GPIO_WAKEOUT, NULL, GPIOF_OUT_INIT_LOW },
    { true, DTS_PROP_GPIO_WAKEIN, NULL, GPIOF_IN }
};

const bsxx_board_info *bsxx_board_get_info(void)
{
    return &g_board_info;
}

static struct device_node *board_find_device_node(uint8_t dev_id)
{
    struct device_node *np = NULL;
    struct device_node *dev_np = NULL;
    /* 只使用单个, 忽略dev_id. */
    tiot_unused(dev_id);
    /* 获取 top dts节点 */
    np = of_find_compatible_node(NULL, NULL, DTS_NODE_TIOT);
    if (np == NULL) {
        printk("no compatible node.\r\n");
        return NULL;
    }
    /* 获取子dts节点 */
    dev_np = of_find_compatible_node(np, NULL, DTS_NODE_COMPATIBLE);
    if (dev_np == NULL) {
        printk("no device compatible node.\r\n");
        return NULL;
    }
    return dev_np;
}

static int board_gpio_pins_init(struct device_node *dev_np, uint32_t *pins)
{
    uint8_t i;
    int ret;
    int gpio_nr;
    const tiot_kernel_pin_info *pin_infos = (const tiot_kernel_pin_info *)&g_board_pin_infos;
    /* 设置GPIO 初始pinmux. */
    for (i = 0; i < BSXX_PIN_NUM; i++) {
        gpio_nr = of_get_named_gpio(dev_np, pin_infos[i].prop, 0);
        if (!pin_infos[i].optional && (gpio_nr < 0)) {
            printk("non optional gpio no found: [%d].", i);
            return -1;
        }
        if (gpio_nr < 0) {
            pins[i] = TIOT_PIN_NONE;
            continue;
        }
        ret = gpio_request_one(gpio_nr, pin_infos[i].gpio_flags, NULL);
        if (ret != 0) {
            printk("gpio request one failed [%d].\r\n", gpio_nr);
            return -1;
        }
        if (pin_infos[i].gpio_flags == GPIOF_IN) {
            /* 通过helper管理 irq gpio。 */
            ret = tiot_board_irq_pin_register(gpio_nr, WAKEIN_IRQF);
        }
        if (ret != 0) {
            printk("irq gpio register failed [%d].\r\n", gpio_nr);
            return -1;
        }
        pins[i] = gpio_nr;
    }
    return ret;
}

static int uart_info_init(struct device_node *dev_np)
{
    int ret;
    int str_len;
    const char *tty_name = NULL;
    static char tty_name_backup[TTY_NAME_MAX] = { 0 };

    /* 获取tty name. */
    ret = of_property_read_string(dev_np, DTS_PROP_TTY_DEV_NAME, &tty_name);
    if (ret < 0) {
        printk("tty name get failed.\r\n");
        return -1;
    }
    str_len = strlen(tty_name);
    if (str_len >= TTY_NAME_MAX) {
        printk("tty name too long: %d.\r\n", str_len);
        return -1;
    }
    if (memcpy_s((void *)tty_name_backup, str_len, tty_name, str_len) != EOK) {
        printk("tty name copy fail.\r\n");
        return -1;
    }
    g_board_hw_info.xmit_id = (uintptr_t)tty_name_backup;

    /* 获取波特率. */
    if (of_property_read_u32(dev_np, DTS_PROP_TTY_BAUDRATE, &g_xfer_baudrate) < 0) {
        g_xfer_baudrate = CONFIG_BSXX_UART_BAUDRATE; // 如果dts中没有配置， 使用配置宏设置波特率
    }
    tiot_print_info("xfer baudrate = %u.\r\n", g_xfer_baudrate);
    return 0;
}

/* bsxx板级UART、管脚初始化接口 */
int32_t bsxx_board_init(void *param)
{
    uint8_t dev_id;
    int ret;
    struct device_node *dev_np = NULL;

    dev_id = *(uint8_t *)param;
    dev_np = board_find_device_node(dev_id);
    if (dev_np == NULL) {
        return -1;
    }

    /* 获取tty 相关信息. */
    if (uart_info_init(dev_np) < 0) {
        of_node_put(dev_np);
        return -1;
    }

    /* 获取cfg固件文件路径. */
    ret = of_property_read_string(dev_np, DTS_PROP_CFG_PATH, &g_board_info.cfg_path);
    if (ret < 0) {
        /* ignore cfg path get fail, compatible for file array mode. */
        printk("cfg path get failed.\r\n");
    } else {
        printk("cfg path get: %s.\r\n", g_board_info.cfg_path);
    }
    /* 获取gpio管脚. */
    if (board_gpio_pins_init(dev_np, (uint32_t *)g_board_hw_info.pm_info) != 0) {
        of_node_put(dev_np);
        return -1;
    }
    of_node_put(dev_np);
    return 0;
}

void bsxx_board_deinit(void *param)
{
    uint8_t i;
    tiot_unused(param);
    uint32_t *pins = g_board_hw_info.pm_info;
    for (i = 0; i < BSXX_PIN_NUM; i++) {
        if (pins[i] == TIOT_PIN_NONE) {
            continue;
        }
        gpio_free(pins[i]);
    }
    tiot_board_irq_pin_release();
}

uint32_t bsxx_board_get_baudrate(void)
{
    return g_xfer_baudrate;
}
