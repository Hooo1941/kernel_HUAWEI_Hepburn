/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description: board.c header file
 * Author: @CompanyNameTag
 */

#ifndef __BOARD_H__
#define __BOARD_H__

/* 其他头文件包含 */
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include <linux/regulator/machine.h>
#endif

#include "plat_type.h"
#ifdef _PRE_HI_DRV_GPIO
#include "gpio_hitv.h"
#endif
#include "board_mp13.h"
#include "board_dts.h"
#include "board_tv.h"
#include "hisi_ini.h"
#include "plat_gpio.h"
#include "ssi_common.h"

/* 宏定义 */
#define BOARD_SUCC 0
#define BOARD_FAIL (-1)

#define VERSION_FPGA 0
#define VERSION_ASIC 1

#define WIFI_TAS_DISABLE 0
#define WIFI_TAS_ENABLE  1

#define PMU_CLK_REQ_DISABLE 0
#define PMU_CLK_REQ_ENABLE  1

#define GPIO_LOWLEVEL  0
#define GPIO_HIGHLEVEL 1

#define NO_NEED_POWER_PREPARE 0
#define NEED_POWER_PREPARE    1

#define TCXO_FREQ_DET_38P4M  1
#define TCXO_FREQ_DET_76P8M  0

#define MPXX_SUBCHIP_NAME_LEN_MAX 128

/* mpxx */
#define BOARD_VERSION_NAME_MP13  "hi1103"
#define BOARD_VERSION_NAME_MP15  "hi1105"
#define BOARD_VERSION_NAME_MP16 "shenkuo"
#define BOARD_VERSION_NAME_MP16C "bisheng"
#define BOARD_VERSION_NAME_GF61  "hi1161"

#define DTS_NODE_HISI_MPXX            "hisilicon,hi110x"
#define DTS_COMP_HISI_MPXX_BOARD_NAME DTS_NODE_HISI_MPXX

/* hisi_gt */
#define DTS_NODE_HISI_GT "hisilicon,hisi_gt"

#define DTS_PROP_MPXX_VERSION "hi110x,asic_version"
#define DTS_PROP_MPXX_PMU_CLK "hi110x,pmu_clk_req"

#define DTS_PROP_MPXX_WIFI_DISABLE "hi110x,wifi_disable"
#define DTS_PROP_MPXX_BFGX_DISABLE "hi110x,bfgx_disable"

#define DTS_PROP_SUBCHIP_TYPE_VERSION "hi110x,subchip_type"

#define DTS_PROP_CLK_32K "huawei,pmu_clk32b"

#define DTS_PROP_MPXX_POWER_PREPARE "hi110x,power_prepare"
#define DTS_PROP_GPIO_TCXO_LEVEL      "hi110x,tcxo_gpio_level"

#define DTS_PROP_MPXX_GPIO_TCXO_FREQ_DETECT  "hi110x,gpio_tcxo_freq_detect"
#define PROC_NAME_MPXX_GPIO_TCXO_FREQ "hi110x_tcxo_freq_detect"

#define DTS_PROP_MPXX_BUCK_MODE   "hi110x,buck_mode"

/* hisi_bfgx */
#define DTS_NODE_MPXX_BFGX            "hisilicon,hisi_bfgx"
#define DTS_NODE_MPXX_PM_SWITCH       "pm_switch"
#define DTS_PROP_GPIO_BFGX_IR_CTRL      "hi110x,gpio_bfgx_ir_ctrl"
#define PROC_NAME_GPIO_BFGX_IR_CTRL     "bfgx_ir_ctrl"
#define DTS_PROP_MPXX_IR_LDO_TYPE     "hi110x,irled_power_type"
#define DTS_PROP_MPXX_IRLED_LDO_POWER "hi110x,irled_power"
#define DTS_PROP_MPXX_IRLED_VOLTAGE   "hi110x,irled_voltage"

#define DTS_PROP_MPXX_COLDBOOT_PARTION      "hi110x,coldboot_partion"

#define DTS_PROP_MPXX_UART_POART            "hi110x,uart_port"
#define DTS_PROP_MPXX_UART_RATE             "hi110x,uart_rate"
#define DTS_PROP_MPXX_UART_PCLK             "hi110x,uart_pclk_normal"

/* hisi_me */
#define DTS_NODE_MPXX_ME            "hisilicon,hisi_me"
#define DTS_PROP_MPXX_GUART_POART   "hi110x,me_uart_port"

/* hisi_sle */
#define DTS_NODE_MPXX_SLE           "hisilicon,hisi_gle"
#define DTS_PROP_MPXX_SLEUART_POART "hi110x,gle_uart_port"

/* hisi_wifi */
#define DTS_NODE_MPXX_WIFI                  "hisilicon,hisi_wifi"
#define DTS_PROP_GPIO_WLAN_FLOWCTRL           "hi110x,gpio_wlan_flow_ctrl"

#define DTS_PROP_MPXX_PCIE_RC_IDX      "hi110x,pcie_rc_idx"
#define DTS_PROP_MPXX_HOST_GPIO_SAMPLE "hi110x,gpio_sample_low"
#define DTS_PROP_MPXX_WIFI_TAS_STATE "hi110x,gpio_wifi_tas_state"

#define DTS_PROP_WIFI_TAS_EN    "hi110x,wifi_tas_enable"
#define PROC_NAME_GPIO_WIFI_TAS "wifi_tas"
#define DTS_PROP_GPIO_WIFI_TAS  "hi110x,gpio_wifi_tas"

/* hisi_cust_cfg */
#define COST_MPXX_COMP_NODE   "hi110x,customize"
#define PROC_NAME_INI_FILE_NAME "ini_file_name"

#define DTS_NODE_HISI_CCIBYPASS "hisi,ccibypass"

#ifdef _PRE_SUSPORT_OEMINFO
#define DTS_WIFI_CALIBRATE_IN_OEMINFO "hi110x,wifi_calibrate_in_oeminfo"
#endif

/* ini cfg */
#define INI_WLAN_DOWNLOAD_CHANNEL "board_info.wlan_download_channel"
#define INI_BFGX_DOWNLOAD_CHANNEL "board_info.bfgx_download_channel"

#define DOWNLOAD_CHANNEL_SDIO "sdio"
#define DOWNLOAD_CHANNEL_PCIE "pcie"
#define DOWNLOAD_CHANNEL_UART "uart"
#define DOWNLOAD_CHANNEL_USB  "usb"

#define INI_FIRMWARE_DOWNLOAD_MODE "firmware_download_mode"

#define BOARD_VERSION_LEN    128
#define DOWNLOAD_CHANNEL_LEN 64

#define VOLATAGE_V_TO_MV 1000
/* STRUCT 定义 */
typedef struct bd_init_s {
    int32_t (*board_gpio_init)(struct platform_device *pdev);           // gpio init
    void (*board_gpio_free)(struct platform_device *pdev);           // gpio free
    int32_t (*board_sys_attr_init)(struct platform_device *pdev);       // 系统属性
    int32_t (*bfgx_dev_power_on)(struct ps_core_s *ps_core_d, uint32_t sys, uint32_t subsys);
    int32_t (*bfgx_dev_power_off)(struct ps_core_s *ps_core_d, uint32_t sys, uint32_t subsys);
    int32_t (*wlan_power_off)(void);
    int32_t (*wlan_power_on)(void);
    int32_t (*gt_power_off)(void);
    int32_t (*gt_power_on)(void);
    int32_t (*board_power_on)(uint32_t ul_subsystem);
    int32_t (*board_power_off)(uint32_t ul_subsystem);
    int32_t (*board_power_reset)(uint32_t ul_subsystem);
    int32_t (*check_evb_or_fpga)(void);
    void (*power_state_change)(int32_t type, int32_t flag);
    void (*board_error_handler)(uint32_t errid);
    int32_t (*board_power_notify)(uint32_t ul_subsystem, int32_t en);
    int32_t (*board_gpio_request)(int32_t physical_gpio, const char *gpio_name, uint32_t direction);
#if defined(_PRE_S4_FEATURE)
    void (*suspend_board_gpio_in_s4)(void);
    void (*resume_board_gpio_in_s4)(void);
#endif
    int32_t (*pm_notify)(unsigned long mode, void *data);
} bd_init_t;

typedef struct _gpio_ssi_ops_ {
    int32_t (*clk_switch)(int32_t ssi_clk);
    int32_t (*aon_reset)(void);
} gpio_ssi_ops;

typedef enum {
    W_SYS = 0x0,
    B_SYS = 0x1,
    G_SYS = 0x2,
    GT_SYS = 0x3,

    SYS_BUTT
}sys_en_enum;

enum board_power_state {
    BOARD_POWER_OFF = 0x0,
    BOARD_POWER_ON = 0x1,
};

/* private data for pm driver */
typedef struct {
    void* dev;
    /* board init ops */
    struct bd_init_s bd_ops;

    gpio_ssi_ops ssi_ops;

    /* flowctrl gpio */
    int32_t flowctrl_gpio;

    int32_t rf_wifi_tas; /* mp13 product */

    /* device hisi board verision */
    const char *chip_type;
    int32_t chip_nr;

    /* how to download firmware */
    int32_t wlan_download_channel;
    int32_t bfgn_download_channel;
    int32_t download_mode;

#ifdef HAVE_HISI_IR
    bool have_ir;
    int32_t irled_power_type;
    int32_t bfgx_ir_ctrl_gpio;
    struct regulator *bfgn_ir_ctrl_ldo;
#endif

    /* mpxx irq info */
    int32_t flowctrl_irq;

    /* mpxx uart info */
    const char *uart_port[UART_BUTT];
    int32_t uart_pclk;

    /* mpxx clk info */
    const char *clk_32k_name;
    struct clk *clk_32k;
    int32_t is_clk_open;

    /* evb or fpga verison */
    int32_t is_asic;

    int32_t is_emu; // low freq emu platform

    int32_t is_wifi_disable;
    int32_t is_bfgx_disable;
    int32_t is_gt_disable;

    int32_t wifi_tas_enable;
    int32_t wifi_tas_state;

    int32_t pmu_clk_share_enable;

    /* buck mode param ,get from ini */
    uint16_t buck_param;
    struct regulator *buck_regulator;

    /* WIFI/BT/GNSS/FM/IR各子系统对buck的控制flag，bitmap控制 */
    uint16_t buck_control_flag;

    /* ini cfg */
    char *ini_file_name;

    /* prepare before board power on */
    int32_t need_power_prepare;
    int32_t tcxo_freq_detect;
    int32_t gpio_tcxo_detect_level;
    const char *coldboot_partion;
} mpxx_board_info;

typedef struct _device_vesion_board {
    uint32_t index;
    const char name[BOARD_VERSION_LEN + 1];
} device_board_version;

typedef struct _download_channel {
    uint32_t index;
    uint8_t name[DOWNLOAD_CHANNEL_LEN + 1];
} download_channel;


enum hisi_device_board {
    /* mpxx 项目使用 0 ~ 999 */
    BOARD_VERSION_MP13 = 1,
    BOARD_VERSION_MP15 = 3,
    BOARD_VERSION_MP16 = 4,
    BOARD_VERSION_MP16C = 5,
    BOARD_VERSION_GF61 = 6,
    BOARD_VERSION_MPXX_BUTT,
};

enum hisi_download_firmware_channel {
    CHANNEL_SDIO = 0x0,
    CHANNEL_PCIE = 0x1,
    CHANNEL_UART = 0x2,
    CHANNEL_USB  = 0x3,
    CHANNEL_DOWNLOAD_BUTT,
};

enum hisi_download_firmware_mode {
    MODE_COMBO = 0x0,          // wifi+bt+gnss子系统混合加载
    MODE_SEPARATE = 0x1,      // wifi,bt,gnss子系统独立加载

    MODE_DOWNLOAD_BUTT,
};


enum board_irled_power_type {
    IR_GPIO_CTRL = 0x0,
    IR_LDO_CTRL = 0x1,
};
#if defined(CONFIG_HUAWEI_OHOS_DSM) || defined(CONFIG_HUAWEI_DSM)
typedef enum sub_system {
    SYSTEM_TYPE_WIFI = 0,
    SYSTEM_TYPE_BT = 1,
    SYSTEM_TYPE_GNSS = 2,
    SYSTEM_TYPE_FM = 3,
    SYSTEM_TYPE_PLATFORM = 4,
    SYSTEM_TYPE_BFG = 5,
    SYSTEM_TYPE_IR = 6,
    SYSTEM_TYPE_NFC = 7,
    SYSTEM_TYPE_BUT,
} sub_system_enum;
#endif

typedef enum _mpxx_release_vtype_ {
    MPXX_VTYPE_RELEASE = 0, /* release version */
    MPXX_VTYPE_RELEASE_DEBUG, /* beta user */
    MPXX_VTYPE_DEBUG, /* debug version */
    MPXX_VTYPE_BUT
} mpxx_release_vtype;

enum mpxx_vmin_mode {
    VMIN_MODE_NORMAL = 0,
    VMIN_MODE_EMERGENCY = 1,
    VMIN_MODE_FACTORY = 2,
    VMIN_MODE_BUTT
};

extern mpxx_board_info g_st_board_info;

#ifdef _PRE_WLAN_EXPORT
extern uint8_t g_wlan_device_mac_addr[NUM_6_BYTES];
#endif

/* 函数声明 */
int32_t board_gpio_request(int32_t physical_gpio, const char *gpio_name, uint32_t direction);
mpxx_board_info *get_hi110x_board_info(void);
int mpxx_is_asic(void);
int hi110x_is_emu(void);
uint16_t mpxx_cmu_is_tcxo_dll(void);
void hi110x_emu_mdelay(int msec);
unsigned int hi110x_get_emu_timeout(unsigned int timeout);
int is_wifi_support(void);
int is_bfgx_support(void);
int is_gt_support(void);
int pmu_clk_request_enable(void);
int is_hi110x_debug_type(void);
int32_t get_mpxx_subchip_type(void);
int32_t mpxx_board_init(void);
void mpxx_board_exit(void);
int32_t board_set_pcie_reset(int32_t is_master, int32_t is_poweron);

#ifndef MPXX_OS_BUILD_VARIANT_USER
#define MPXX_OS_BUILD_VARIANT_ROOT 1
#define MPXX_OS_BUILD_VARIANT_USER 2
#endif
int32_t mpxx_firmware_download_mode(void);
int32_t mpxx_get_os_build_variant(void);
mpxx_release_vtype hi110x_get_release_type(void);
int32_t board_host_wakeup_dev_set(int value);
int32_t board_host_gt_wakeup_dev_set(int value);
int32_t board_wifi_tas_set(int value);
int32_t board_get_wifi_tas_gpio_state(void);
int32_t board_power_on(uint32_t subsystem);
int32_t board_power_off(uint32_t subsystem);
int32_t board_power_reset(uint32_t ul_subsystem);
int32_t board_wlan_gpio_power_on(void *data);
int32_t board_wlan_gpio_power_off(void *data);
void board_chip_power_on(void);
void board_chip_power_off(void);
int32_t board_sys_enable(uint8_t sys);
int32_t board_sys_disable(uint8_t sys);
int32_t enable_board_pmu_clk32k(void);
int32_t disable_board_pmu_clk32k(void);
const uint8_t* get_device_board_name(void);
int32_t board_chiptype_init(void);
int32_t get_board_gpio(const char *gpio_node, const char *gpio_prop, int32_t *physical_gpio);
int32_t board_flowctrl_gpio_init(void);
void board_flowctrl_irq_init(void);
void free_board_flowctrl_gpio(void);
void power_state_change(int32_t type, int32_t flag);
int32_t get_board_custmize(const char *cust_node, const char *cust_prop, const char **cust_prop_val);
int32_t get_board_dts_node(struct device_node **np, const char *node_prop);
#if defined(_PRE_S4_FEATURE)
void suspend_board_gpio_in_s4(void);
void resume_board_gpio_in_s4(void);
#endif
int32_t check_download_channel_pcie(void);
void board_register_pcie_poweron(int (*func)(int32_t data));
void board_register_pcie_poweroff(int (*func)(int32_t data));
int board_power_notify(uint32_t ul_subsystem, int32_t en);
#endif
