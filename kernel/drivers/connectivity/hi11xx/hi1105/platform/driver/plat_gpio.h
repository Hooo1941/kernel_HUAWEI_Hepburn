/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description: plat_gpio.c header file
 * Author: @CompanyNameTag
 */

#ifndef PLAT_GPIO_H
#define PLAT_GPIO_H

#include "plat_type.h"
#include "oal_schedule.h"

/* CHIP_POWR */
#define DTS_PROP_GPIO_MPXX_POWEN_ON           "hi110x,gpio_power_on"
#define PROC_NAME_GPIO_POWEN_ON                 "power_on_enable"
/* W_POWER */
#define DTS_PROP_GPIO_WLAN_POWEN_ON_ENABLE      "hi110x,gpio_wlan_power_on"
#define PROC_NAME_GPIO_WLAN_POWEN_ON            "wlan_power_on_enable"
/* GT_POWER */
#define DTS_PROP_GPIO_GT_POWEN_ON_ENABLE        "hi110x,gpio_gt_power_on"
#define PROC_NAME_GPIO_GT_POWEN_ON              "gt_power_on_enable"
/* BFGX_POWR */
#define DTS_PROP_GPIO_BFGX_POWEN_ON_ENABLE      "hi110x,gpio_bfgx_power_on"
#define PROC_NAME_GPIO_BFGX_POWEN_ON            "bfgx_power_on_enable"
/* G_POWR */
#define DTS_PROP_GPIO_G_POWEN_ON_ENABLE         "hi110x,gpio_g_power_on"
#define PROC_NAME_GPIO_G_POWEN_ON               "g_power_on_enable"
/* W_WKUP_HOST */
#define DTS_PROP_MPXX_GPIO_WLAN_WAKEUP_HOST   "hi110x,gpio_wlan_wakeup_host"
#define PROC_NAME_GPIO_WLAN_WAKEUP_HOST         "wlan_wake_host"
/* HOST_WKUP_W */
#define DTS_PROP_GPIO_HOST_WAKEUP_WLAN          "hi110x,gpio_host_wakeup_wlan"
#define PROC_NAME_GPIO_HOST_WAKEUP_WLAN         "host_wakeup_wlan"
/* BFGX_WKUP_HOST */
#define DTS_PROP_MPXX_GPIO_BFGX_WAKEUP_HOST   "hi110x,gpio_bfgx_wakeup_host"
#define PROC_NAME_GPIO_BFGX_WAKEUP_HOST         "bfgx_wake_host"
/* HOST_WKU_BFGX */
#define DTS_PROP_MPXX_GPIO_HOST_WAKEUP_BFGX   "hi110x,gpio_host_wakeup_bfgx"
#define PROC_NAME_GPIO_HOST_WAKEUP_BFG          "host_wakeup_bfg"
/* G_WKUP_HOST */
#define DTS_PROP_MPXX_GPIO_GCPU_WAKEUP_HOST   "hi110x,gpio_gcpu_wakeup_host"
#define PROC_NAME_GPIO_GCPU_WAKEUP_HOST         "gcpu_wake_host"
/* SLE_WKUP_HOST */
#define DTS_PROP_GPIO_SLE_WAKEUP_HOST           "hi110x,gpio_gle_wakeup_host"
#define PROC_NAME_GPIO_SLE_WAKEUP_HOST          "gle_wake_host"
/* HOST_WKUP_SLE */
#define DTS_PROP_GPIO_HOST_WAKEUP_SLE           "hi110x,gpio_host_wakeup_gle"
#define PROC_NAME_GPIO_HOST_WAKEUP_SLE          "host_wake_gle"
/* GT_WKUP_HOST */
#define DTS_PROP_GPIO_GT_WAKEUP_HOST            "hi110x,gpio_gt_wakeup_host"
#define PROC_NAME_GPIO_GT_WAKEUP_HOST           "gt_wake_host"
/* HOST_WKUP_GT */
#define DTS_PROP_GPIO_HOST_WAKEUP_GT            "hi110x,gpio_host_wakeup_gt"
#define PROC_NAME_GPIO_HOST_WAKEUP_GT           "host_wake_gt"
/* SSI_DATA */
#define DTS_PROP_GPIO_MPXX_GPIO_SSI_DATA      "hi110x,gpio_ssi_data"
#define PROC_NAME_GPIO_SSI_DATA                 "hi110x ssi data"
/* SSI_CLK */
#define DTS_PROP_GPIO_MPXX_GPIO_SSI_CLK       "hi110x,gpio_ssi_clk"
#define PROC_NAME_GPIO_SSI_CLK                  "hi110x ssi clk"
/* CLK_FREQ_DET */
#define DTS_PROP_MPXX_GPIO_TCXO_FREQ_DETECT   "hi110x,gpio_tcxo_freq_detect"
#define PROC_NAME_MPXX_GPIO_TCXO_FREQ         "hi110x_tcxo_freq_detect"
/* PCIE0_RST */
#define DTS_PROP_GPIO_MPXX_PCIE0_RST          "hi110x,gpio_pcie0_rst"
#define PROC_NAME_GPIO_PCIE0_RST                "ep_pcie0_rst clk"
/* PCIE1_RST */
#define DTS_PROP_GPIO_MPXX_PCIE1_RST          "hi110x,gpio_pcie1_rst"
#define PROC_NAME_GPIO_PCIE1_RST                "ep_pcie1_rst"
/* W_FLOW_CTRL */
#define DTS_PROP_GPIO_WLAN_FLOWCTRL             "hi110x,gpio_wlan_flow_ctrl"
#define PROC_NAME_GPIO_WLAN_FLOWCTRL            "hi110x_wlan_flowctrl"
/* IR_CTRL */
#define DTS_PROP_GPIO_BFGX_IR_CTRL              "hi110x,gpio_bfgx_ir_ctrl"
#define PROC_NAME_GPIO_BFGX_IR_CTRL             "bfgx_ir_ctrl"
enum gpio_type {
    CHIP_POWR,
    W_POWER,
    GT_POWER,
    BFGX_POWER,
    G_POWER,
    W_WKUP_HOST,
    HOST_WKUP_W,
    BFGX_WKUP_HOST,
    HOST_WKUP_BFGX,
    G_WKUP_HOST,
    SLE_WKUP_HOST,
    HOST_WKUP_SLE,
    GT_WKUP_HOST,
    HOST_WKUP_GT,
    SSI_DATA,
    SSI_CLK,
    CLK_FREQ_DET,
    PCIE0_RST,
    PCIE1_RST,
    SDIO_FLOW_CTRL,
    IR_CTRL,
    GPIO_TYPE_BUFF
};

enum req_init_sts {
    REQ_ONE,        // 调用gpio_request_one 分配过程中同时配置gpio状态
    REQ_ONLY,       // 单独调用gpio_request 仅申请(不建议)
    SKIP,           // 不申请或者用其他方式申请
};

struct conn_pin_init {
    int32_t flags;   // 引脚电平状态
    enum req_init_sts req_sts;
};

struct conn_gpio_pin_info {
    char *name;              // 名字
    int32_t number;          // gpio 引脚号
    uint32_t is_key_gpio;    // 关键gpio，初始化失败需要报错
    uint32_t type;           // 类型，参考enum gpio_type
    uint32_t invert;         // 极性取反
    uint32_t requested;      // 判断是否已经注册
    void *data;              // 私有成员
    struct conn_pin_init init;
};

enum cfg_mode {
    MODE_NULL,
    MODE_DTS,
    MODE_GPIOLIB
};

struct conn_gpio_irq_attr {
    char *name;
    unsigned long flags;
    uint32_t number;
    oal_spin_lock_stru lock;
    uint32_t inited;
    uint32_t is_enabled;
    uint32_t init_set_enable;
    oal_irq_handler_t handler;
    void *dev;
};

struct conn_gpio_dts_attr {
    char *compatibale;
    char *label_name;
};

struct conn_gpiolib_attr {
    char *con_id;
    unsigned int index;
    struct gpio_desc *desc;
};

union attr_method {
    struct conn_gpio_dts_attr dts;
    struct conn_gpiolib_attr gpiod;
};

struct conn_gpio_attr {
    enum cfg_mode mode;
    union attr_method u;
};

struct conn_gpio_node {
    struct conn_gpio_pin_info pin;
    struct conn_gpio_attr attr;
    struct conn_gpio_irq_attr irq;
};

struct conn_gpio_userdata {
    uint8_t level;       // 电平状态
    uint8_t number;      // gpio 引脚号
    uint8_t main_type;   // 主类型 (根据产品需要)
    uint8_t sub_type;    // 从类型 (根据产品需要)
    uint32_t data_len;   // 可扩展的数据长度
    uint8_t data[0];     // 可扩展的数据地址
};

enum gpio_level {
    GPIO_LEVEL_LOW  = 0x0,
    GPIO_LEVEL_HIGH = 0x1,
};

enum dump_type {
    DUMP_GPIO = 0x0,
    DUMP_IRQ = 0x1,
    DUMP_BUFF
};

// 处理从用户态传递下来的数据结构
struct conn_gpio_ops {
    int32_t (*gpio_init)(struct conn_gpio_node *node);
    void (*gpio_uninit)(struct conn_gpio_node *node);
    int32_t (*gpio_request)(struct conn_gpio_node *node);
    void (*gpio_unrequest)(struct conn_gpio_node *node);
    uint32_t (*get_irq_num)(int32_t pin_num);
    int32_t (*irq_register)(struct conn_gpio_node *node, oal_irq_handler_t handler, void *dev);
    void (*irq_unregister)(struct conn_gpio_node *node, void *dev);
    void (*irq_enable)(struct conn_gpio_irq_attr *attr, uint32_t en);
    int32_t (*set_gpio_level)(struct conn_gpio_node *node, enum gpio_level level);
    int32_t (*set_gpio_input)(struct conn_gpio_node *node);
    int32_t (*user_set)(struct conn_gpio_userdata *data, uint8_t OAL_USER *userdata);
    enum gpio_level (*get_gpio_level)(struct conn_gpio_node *node);
};

struct conn_gpio_chip {
    struct conn_gpio_node *node;
    uint32_t node_num;
    struct conn_gpio_ops *ops;
    void *dev;
};

int32_t conn_gpio_init(struct conn_gpio_chip *chip);
void conn_gpio_remove(void);
void conn_gpio_intr_enable(enum gpio_type type, uint32_t en);
int32_t conn_irq_register(enum gpio_type type, oal_irq_handler_t handler, void *dev);
void conn_irq_unregister(enum gpio_type type, void *dev);
void conn_irq_hibernate(void);
void conn_irq_restore(void);
int32_t conn_set_gpio_level(enum gpio_type type, enum gpio_level level);
int32_t conn_set_gpio_input(enum gpio_type type);
int32_t default_set_gpio_level(struct conn_gpio_node *gpio_node, enum gpio_level level);
int32_t conn_set_gpio_level(enum gpio_type type, enum gpio_level level);
int32_t conn_set_gpio_input(enum gpio_type type);
int32_t conn_get_gpio_level(enum gpio_type type);
int32_t conn_get_gpio_type_by_bus_id(uint32_t bus_id);
int32_t conn_get_gpio_number_by_type(enum gpio_type type);
int32_t conn_get_gpio_type_by_number(int32_t gpio_nr);
int32_t conn_gpio_valid(enum gpio_type type);
void conn_dump_info(enum dump_type dump_type);
// 该接口作为与上层gpio交互使用
int32_t conn_user_gpio_config(uint8_t OAL_USER *data);
#endif
