/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description: CHR exception type define
 * Author: @CompanyNameTag
 */

#ifndef CHR_ERRNO_H
#define CHR_ERRNO_H

/* 宏定义 */
#define CHR_SYSTEM_OFFSET 19
#define CHR_LAYER_OFFSET  16
#define CHR_EVENT_OFFSET  8
#define CHR_ERROR_OFFSET  0
#define CHR_SYSTEM_MASK   0x1F
#define CHR_LAYER_MASK    0x07
#define CHR_EVENT_MASK    0xFF
#define CHR_ERROR_MASK    0xFF

/*
 *                        CHR ERRNO 错误码结构
 * |---------------------------------------------------------------------------|
 * | 31~24 (8)bits| 23~19 (5)bits | 18~16 (3)bits | 15~8 (8)bits | 7~0 (8)bits |
 * |---------------------------------------------------------------------------|
 * |   Reserved    |    system     |     layer     |    event    |   error     |
 * |---------------------------------------------------------------------------|
 *
 * |---------------------------------------------------------------------------|
 * |   位段   |      取值       |                    用途                      |
 * |---------------------------------------------------------------------------|
 * | Reserved |      保留       |                    保留                      |
 * |---------------------------------------------------------------------------|
 * | system   | chr_system_enum | 标识异常子系统 plat wifi gnss bt fm nfc ir   |
 * |---------------------------------------------------------------------------|
 * | layer    | chr_layer_enum  | 标识异常所处位置   dev      drv      hal     |
 * |---------------------------------------------------------------------------|
 * | event    |CHR_XX_EVENT_ENUM| 标识异常触发事件                             |
 * |---------------------------------------------------------------------------|
 * | error    |CHR_XX_ERROR_ENUM| 标识异常触发因素,详细描述触发异常事件的原因  |
 * |---------------------------------------------------------------------------|
 */
typedef enum {
    CHR_SYSTEM_PLAT = 0x01 << CHR_SYSTEM_OFFSET,
    CHR_SYSTEM_WIFI = 0x02 << CHR_SYSTEM_OFFSET,
    CHR_SYSTEM_GNSS = 0x03 << CHR_SYSTEM_OFFSET,
    CHR_SYSTEM_BT = 0x04 << CHR_SYSTEM_OFFSET,
    CHR_SYSTEM_FM = 0x05 << CHR_SYSTEM_OFFSET,
    CHR_SYSTEM_NFC = 0x06 << CHR_SYSTEM_OFFSET,
    CHR_SYSTEM_IR = 0x07 << CHR_SYSTEM_OFFSET,
    CHR_SYSTEM_MUTT = 0x08 << CHR_SYSTEM_OFFSET,
} chr_system_enum;

typedef enum {
    CHR_LAYER_DEV = 0x01 << CHR_LAYER_OFFSET,
    CHR_LAYER_DRV = 0x02 << CHR_LAYER_OFFSET,
    CHR_LAYER_HAL = 0x03 << CHR_LAYER_OFFSET,
} chr_layer_enum;

typedef enum {
    /* CHR_WIFI_DRV */
    CHR_WIFI_DRV_EVENT_OPEN = 0x01,
    CHR_WIFI_DRV_EVENT_SCAN = 0x02,
    CHR_WIFI_DRV_EVENT_CONNECT = 0x03,
    CHR_WIFI_DRV_EVENT_DISCONNECT = 0x04,
    CHR_WIFI_DRV_EVENT_CLOSE = 0x05,
    CHR_WIFI_DRV_EVENT_EXCEPTION = 0x06,
    CHR_WIFI_DRV_EVENT_FAKELINK = 0x07,

    /* CHR_WIFI_DRV platform event */
    CHR_WIFI_DRV_EVENT_PLAT = 0x08, // 平台杂糅，待解耦

    /* CHR_WIIF_DRV_SOFTAP event */
    CHR_WIFI_DRV_EVENT_SOFTAP_CONNECT = 0x09,
    CHR_WIFI_DRV_EVENT_SOFTAP_DISCONNECT = 0x0A,
    CHR_WIFI_DRV_EVENT_SOFTAP_PASSIVE_DISCONNECT = 0x0B,
    CHR_WIFI_DRV_EVENT_MIMO_TO_SISO_FAIL = 0x0C,
    CHR_WIFI_DRV_EVENT_HT_SELF_CURE_OCCUR = 0x0D,

    CHR_WIFI_DRV_EVENT_RING_TX = 0x0E,

    /* CHR_WIFI_DEV */
    CHR_WIFI_DEV_EVENT_CHIP = 0x01, // host使用不规范，待整改
    CHR_WIFI_DEV_EVENT_RING_RX = 0x02,
} chr_wifi_drv_event_enum;

typedef enum {
    /* CHR_GNSS_HAL */
    CHR_GNSS_HAL_EVENT_SYSCALL = 0x01,
    CHR_GNSS_HAL_EVENT_EXCEPTION = 0x02,
    CHR_GNSS_HAL_EVENT_TIME = 0x03,
    CHR_GNSS_HAL_EVENT_INJECT = 0x04,

    /* CHR_GNSS_DRV platform event */
    CHR_GNSS_DRV_EVENT_PLAT = 0x05,
} chr_gnss_event_enum;

typedef enum {
    /* CHR_PLT_DRV */
    CHR_PLT_DRV_EVENT_INIT = 0x01,
    CHR_PLT_DRV_EVENT_EXIT = 0x02,
    CHR_PLT_DRV_EVENT_DFR = 0x03,
    CHR_PLT_DRV_EVENT_SDIO = 0x04,
    CHR_PLT_DRV_EVENT_UART = 0x05,
    CHR_PLT_DRV_EVENT_PCIE = 0x06,
    CHR_PLT_DRV_EVENT_GPIO = 0x07,
    CHR_PLT_DRV_EVENT_CLK = 0x08,
    CHR_PLT_DRV_EVENT_OPEN = 0x09,
    CHR_PLT_DRV_EVENT_CLOSE = 0x0a,
    CHR_PLT_DRV_EVENT_PM = 0x0b,
    CHR_PLT_DRV_EVENT_DTS = 0x0c,
    CHR_PLT_DRV_EVENT_DEV = 0x0d,
    CHR_PLT_DRV_EVENT_FW = 0x0e,
    CHR_PLT_DRV_EVENT_RST = 0x0f,
    CHR_PLT_DRV_EVENT_INI = 0x10,
    CHR_PLT_DRV_EVENT_GUGONG = 0x11,
} chr_plt_drv_event_enum;

typedef enum {
    /* CHR_WIFI_DRV_EVENT_OPEN */
    CHR_WIFI_DRV_ERROR_POWER_ON = 0x03, /* cover */
    CHR_WIFI_DRV_ERROR_CUSTOM_CALL = 0x04,
    CHR_WIFI_DRV_ERROR_POWER_ON_SET_MAC_ADDR = 0x05,     /* 新增 */
    CHR_WIFI_DRV_ERROR_POWER_ON_NO_CUSTOM_CALL = 0x08,   /* 新增 */
    CHR_WIFI_DRV_ERROR_POWER_ON_CALL_TIMEOUT = 0x09,     /* 新增 */

    /* CHR_WIFI_DRV_EVENT_SCAN */
    CHR_WIFI_DRV_ERROR_SCAN_REFUSED = 0x01,
    CHR_WIFI_DRV_ERROR_SCAN_TIMEOUT = 0x02,
    CHR_WIFI_DRV_ERROR_SCAN_ZERO = 0x03,

    /* CHR_WIFI_DRV_EVENT_CONNECT */
    CHR_WIFI_DRV_ERROR_CONNECT_CMD = 0x01,
    CHR_WIFI_DRV_ERROR_AUTH_TIMEOUT = 0x02,
    CHR_WIFI_DRV_ERROR_ASSOC_TIMEOUT = 0x03,
    CHR_WIFI_DRV_ERROR_AUTH_REJECTED = 0x04,
    CHR_WIFI_DRV_ERROR_ASSOC_REJECTED = 0x05,

    /* CHR_WIFI_DRV_EVENT_DISCONNECT */
    CHR_WIFI_DRV_ERROR_LINKLOSS = 0x01,
    CHR_WIFI_DRV_ERROR_KEEPALIVE_TIMEOUT = 0x02,
    CHR_WIFI_DRV_ERROR_CHANNEL_CHANGE = 0x03,

    /* CHR_WIFI_DRV_EVENT_CLOSE */
    CHR_WIFI_DRV_ERROR_POWER_OFF = 0x01,

    /* CHR_WIFI_DRV_EVENT_FAKELINK */
    CHR_WIFI_DRV_ERROR_RX_NO_BUFFER = 0x01,
    CHR_WIFI_DRV_ERROR_ARP_TX_FAIL = 0x02,
    CHR_WIFI_DRV_ERROR_EAPOL_TX_FAIL = 0x03,
    CHR_WIFI_DRV_ERROR_DHCP_TX_FAIL = 0x04,
    CHR_WIFI_DRV_ERROR_RF_OVERHEAT_EXCEPTION = 0x05,
    CHR_WIFI_DRV_ERROR_INTERFERENCE = 0x06,

    /* CHR_WIFI_DRV_EVENT_RING_TX */
    CHR_WIFI_DRV_ERROR_TX_RING_SWITCH = 0x01,
    CHR_WIFI_DRV_ERROR_TX_RING_INIT   = 0x02,
    CHR_WIFI_DRV_ERROR_TX_RING_ALLOC  = 0x03,
    CHR_WIFI_DRV_ERROR_TX_RING_STUCK  = 0x04,

    /* CHR_WIFI_DRV_EVENT_RING_RX */
    CHR_WIFI_DEV_ERROR_RX_RING_INIT   = 0x01,

    /* CHR_WIFI_DEV_EVENT_CHIP */
    CHR_WIFI_DEV_ERROR_FEM_FAIL = 0x01,  // host使用不规范，待整改
    CHR_WIFI_DEV_ERROR_GPIO = 0x03,
    CHR_WIFI_DEV_ERROR_IOMUX = 0x05,
} chr_wifi_drv_error_enum;

typedef enum {
    /* CHR_GNSS_HAL_EVENT_SYSCALL */
    CHR_GNSS_HAL_ERROR_OPEN_CMD = 0x01,
    CHR_GNSS_HAL_ERROR_READ_CMD = 0x02,
    CHR_GNSS_HAL_ERROR_WRITE_CMD = 0x03,
    CHR_GNSS_HAL_ERROR_FLUSH_CMD = 0x04,
    CHR_GNSS_HAL_ERROR_CLOSE_CMD = 0x05,
    CHR_GNSS_HAL_ERROR_CHMOD_CMD = 0x06,
    CHR_GNSS_HAL_ERROR_SOCKET_CREATE_CMD = 0x07,
    CHR_GNSS_HAL_ERROR_SOCKET_CONNECT_CMD = 0x08,
    CHR_GNSS_HAL_ERROR_PIPE_CREATE_CMD = 0x09,
    CHR_GNSS_HAL_ERROR_EPOLL_REGISTER_CMD = 0x0A,
    CHR_GNSS_HAL_ERROR_EPOLL_HUP_CMD = 0x0B,
    CHR_GNSS_HAL_ERROR_THREAD_CREATE_CMD = 0x0C,

    /* CHR_GNSS_HAL_EVENT_EXCEPTION */
    CHR_GNSS_HAL_ERROR_REBOOT_CMD = 0x01,
    CHR_GNSS_HAL_ERROR_TIMEOUT_CMD = 0x02,
    CHR_GNSS_HAL_ERROR_DATA_LOST_CMD = 0x03,
    CHR_GNSS_HAL_ERROR_DATA_WRONG_CMD = 0x04,
    CHR_GNSS_HAL_ERROR_ACK_LOST_CMD = 0x05,

    /* CHR_GNSS_HAL_EVENT_TIME */
    CHR_GNSS_HAL_ERROR_RTC_CMD = 0x01,

    /* CHR_GNSS_HAL_EVENT_INJECT */
    CHR_GNSS_HAL_ERROR_TIME_INJECT_CMD = 0x01,
    CHR_GNSS_HAL_ERROR_LOC_INJECT_CMD = 0x02,
    CHR_GNSS_HAL_ERROR_EPH_INJECT_CMD = 0x03,
} chr_gnss_error_enum;

typedef enum {
    CHR_PLAT_DRV_ERROR_FIRMWARE_DOWN = 0x01,
    CHR_PLAT_DRV_ERROR_SDIO_INIT = 0x02,
    CHR_PLAT_DRV_ERROR_OPEN_UART = 0x03,
    CHR_PLAT_DRV_ERROR_CFG_UART = 0x04,
    CHR_PLAT_DRV_ERROR_OPEN_BCPU = 0x05,
    CHR_PLAT_DRV_ERROR_BCPU_BOOTUP = 0x06,
    CHR_PLAT_DRV_ERROR_CLOSE_BCPU = 0x07,
    CHR_PLAT_DRV_ERROR_OPEN_WCPU = 0x08,
    CHR_PLAT_DRV_ERROR_WCPU_BOOTUP = 0x09,
    CHR_PLAT_DRV_ERROR_CLOSE_WCPU = 0x0A,
    CHR_PLAT_DRV_ERROR_OPEN_THREAD = 0x0B,
    CHR_PLAT_DRV_ERROR_CLOSE_THREAD = 0x0C,
    CHR_PLAT_DRV_ERROR_WAKEUP_DEV = 0x0D,
    CHR_PLAT_DRV_ERROR_RECV_LASTWORD = 0x0E,
    CHR_PLAT_DRV_ERROR_BEAT_TIMEOUT = 0x0F,
    CHR_PLAT_DRV_ERROR_PCIE_INIT = 0x10,
    CHR_PLAT_DRV_ERROR_ALLOC_MEM = 0x11,
    CHR_PLAT_DRV_ERROR_WIFI_RECOVERY = 0x12, /* wifi-bfgx_status_recovery */
    CHR_PLAT_DRV_ERROR_BFGX_RECOVERY = 0x13, /* wifi-bfgx_status_recovery */
    CHR_PLAT_DRV_ERROR_DEVICE_PANIC = 0x14,  /* add  */

    CHR_PLAT_DRV_ERROR_SDIO_FAIL_FIRMWARE_DOWN = 0x15, /* 新增 */
    CHR_PLAT_DRV_ERROR_CFG_FAIL_FIRMWARE_DOWN = 0x16,  /* 新增 */
    CHR_PLAT_DRV_ERROR_RESUME_FIRMWARE_DOWN = 0x17,    /* 新增 */

    CHR_PLAT_DRV_ERROR_PCIE_LINK_DOWN = 0x18,
    CHR_PLAT_DRV_ERROR_WKUP_GPIO_PCIE_LINK_DOWN = 0x19,    /* 新增 */
    CHR_PLAT_DRV_ERROR_INTX_ISR_PCIE_LINK_DOWN = 0x1A,     /* 新增 */
    CHR_PLAT_DRV_ERROR_D2H_EDMA_PCIE_LINK_DOWN = 0x1B,     /* 新增 */
    CHR_PLAT_DRV_ERROR_D2H_TRANSFER_PCIE_LINK_DOWN = 0x1C, /* 新增 */

    CHR_PLAT_DRV_ERROR_SDIO_WR_FAIL = 0x1D,           /* 新增 */
    CHR_PLAT_DRV_ERROR_SDIO_INIT_PROB_FAIL = 0x1F,    /* 新增 */
    CHR_PLAT_DRV_ERROR_SDIO_INIT_RX_DATA_PROC = 0x20, /* 新增 */
    CHR_PLAT_DRV_ERROR_SDIO_INIT_ISR = 0x21,          /* 新增 */

    CHR_PLAT_DRV_ERROR_SLEEP_FORBID = 0x22, /* 新增 */

    CHR_PLAT_DRV_ERROR_WIFI_SYSTEM_DFR_SUCC = 0x23,
    CHR_PLAT_DRV_ERROR_BFGX_SYSTEM_DFR_SUCC = 0x24,
    CHR_PLAT_DRV_ERROR_INI_READ_FAILED      = 0x27,

    /* start number 40 */
    CHR_PLAT_DRV_ERROR_HCC_DEADLOCK = 0x28, /* 连续两次HCC流控死锁 */

    /* start number 60
     * CHR_PLT_DRV_EVENT_INIT
     */
    CHR_PLAT_DRV_ERROR_BOARD_DRV_PROB = 0x3c,
    CHR_PLAT_DRV_ERROR_PLAT_INIT = 0x3d,
    CHR_PLAT_DRV_ERROR_BOARD_GPIO_INIT = 0x3e,
    CHR_PLAT_DRV_ERROR_EEPROM_READ_INIT = 0x3f,
    CHR_PLAT_DRV_ERROR_PCIE_PROBE_FAIL = 0x40,
    CHR_PLAT_DRV_ERROR_PCIE_PROBE_TIMEOUT = 0x41,
    CHR_PLAT_DRV_ERROR_PCIE_DRV_REG_FAIL = 0x42,

    /* start number 80
     * CHR_PLT_DRV_EVENT_EXIT
     */
    /* start number 100
     * CHR_PLT_DRV_EVENT_DFR
     */
    CHR_PLAT_DRV_ERROR_EXCP_WIFI_HANDLE = 0x64,

    /* start number 120
     * CHR_PLT_DRV_EVENT_SDIO
     */
    CHR_PLAT_DRV_ERROR_SDIO_ENUM = 0x78,
    CHR_PLAT_DRV_ERROR_SDIO_MODULE = 0x79,

    /* start number 140
     * CHR_PLT_DRV_EVENT_UART
     */
    CHR_PLAT_DRV_ERROR_UART_PRINT = 0x8c,
    CHR_PLAT_DRV_ERROR_UART_BAURD = 0x8d,
    CHR_PLAT_DRV_ERROR_OPEN_TTY = 0x8e,

    /* start number 160
     * CHR_PLT_DRV_EVENT_PCIE
     */
    CHR_PLAT_DRV_ERROR_PCIE_SPEED_CHANGE_FAIL = 0xA0,
    CHR_PLAT_DRV_ERROR_PCIE_SEND_MESSAGE_FAIL = 0xA1,
    CHR_PLAT_DRV_ERROR_PCIE_POWERON_LINK_FAIL = 0xA2,
    CHR_PLAT_DRV_ERROR_PCIE_CHECK_LINK_STATE_EXCP = 0xA3,
    CHR_PLAT_DRV_ERROR_PCIE_ENUM_FAIL = 0xA4,
    CHR_PLAT_DRV_ERROR_PCIE_ENUM_TRY_SUCC = 0xA5,

    /* start number 180
     * CHR_PLT_DRV_EVENT_GPIO
     */
    CHR_PLAT_DRV_ERROR_BOARD_GPIO = 0xB4,
    CHR_PLAT_DRV_ERROR_SSI_GPIO = 0xB5,
    CHR_PLAT_DRV_ERROR_POWER_GPIO = 0xB6,
    CHR_PLAT_DRV_ERROR_WAKEUP_GPIO = 0xB7,
    CHR_PLAT_DRV_ERROR_SUBSYS_POWER_GPIO = 0xB8,

    /* start number 200
     * CHR_PLT_DRV_EVENT_CLK
     */
    CHR_PLAT_DRV_ERROR_32K_CLK_EN = 0xC8,

    /* start number 220
     * CHR_PLT_DRV_EVENT_OPEN
     */
    CHR_PLAT_DRV_ERROR_OPEN_BT = 0xDC,
    CHR_PLAT_DRV_ERROR_OPEN_GNSS = 0xDD,
    CHR_PLAT_DRV_ERROR_POWER_UP_BFGX = 0XDE,
    CHR_PLAT_DRV_ERROR_WIFI_OPEN_BCPU = 0xDF,
    CHR_PLAT_DRV_ERROR_BFGX_PWRON_BY_WIFI = 0xE0,
    CHR_PLAT_DRV_ERROR_POWER_UP_WIFI = 0XE1,
    CHR_PLAT_DRV_ERROR_POWER_ON_EXCP = 0xE2,
    CHR_PLAT_DRV_ERROR_POWER_ON_NON_BUS = 0xE3,
    CHR_PLAT_DRV_ERROR_HITALK_WCPU_BOOTUP = 0xE4,
    CHR_PLAT_DRV_ERROR_HITALK_POWER_UP_WIFI = 0XE5,
    CHR_PLAT_DRV_ERROR_HITALK_POWER_ON_EXCP = 0xE6,
    CHR_PLAT_DRV_ERROR_HITALK_POWER_ON_NON_BUS = 0xE7,
    CHR_PLAT_DRV_ERROR_BCPU_BOOTUP_WITH_WIFI_ON = 0xE8,
    CHR_PLAT_DRV_ERROR_BCPU_BOOTUP_WITH_WIFI_OFF = 0xE9,
    CHR_PLAT_DRV_ERROR_FILE_OPEN = 0xEA,

    /* start number 240
     * CHR_PLT_DRV_EVENT_CLOSE
     */
    CHR_PLAT_DRV_ERROR_CLOSE_BT = 0xF0,
    CHR_PLAT_DRV_ERROR_CLOSE_GNSS = 0xF1,
    CHR_PLAT_DRV_ERROR_WIFI_CLOSE_BCPU = 0xF2,
    CHR_PLAT_DRV_ERROR_HITALK_CLOSE_WCPU = 0xF3,

    /* start number 280
     * CHR_PLT_DRV_EVENT_PM
     */
    CHR_PLAT_DRV_ERROR_BFG_WKUP_DEV = 0x118,
    CHR_PLAT_DRV_ERROR_WIFI_WKUP_DEV = 0x119,
    CHR_PLAT_DRV_ERROR_PM_INIT_NO_BUS = 0x11A,
    CHR_PLAT_DRV_ERROR_PM_SDIO_NO_READY = 0x11B,
    CHR_PLAT_DRV_ERROR_WIFI_SLEEP_REQ = 0x11C,
    CHR_PLAT_DRV_ERROR_PM_WKUP_NON_BUS = 0x11D,
    CHR_PLAT_DRV_ERROR_VMIN_WLAN_POWER_FAIL = 0x11E,
    CHR_PLAT_DRV_ERROR_VMIN_BFGX_POWER_FAIL = 0x11F,
    CHR_PLAT_DRV_ERROR_VMIN_WLAN_PLL_REG_WRITE_FAIL = 0x120,
    CHR_PLAT_DRV_ERROR_VMIN_WLAN_PHY_REG_WRITE_FAIL = 0x121,
    CHR_PLAT_DRV_ERROR_VMIN_WLAN_CALI_TIMEOUT = 0x122,

    /* start number 300
     * CHR_PLT_DRV_EVENT_DTS
     */
    CHR_PLAT_DRV_ERROR_32K_CLK_DTS = 0x12C,
    CHR_PLAT_DRV_ERROR_PWR_PINCTRL = 0x12D,
    CHR_PLAT_DRV_ERROR_XLDO_GPIO_PINMUX = 0x12E,
    CHR_PLAT_DRV_ERROR_XLDO_GPIO_LEVEL = 0x12F,

    /* start number 320
     * CHR_PLT_DRV_EVENT_DEV
     */
    CHR_PLAT_DRV_ERROR_WIFI_DEV_PANIC = 0x140,
    CHR_PLAT_DRV_ERROR_BFG_DEV_PANIC = 0x141,

    /* start number 340
     * CHR_PLT_DRV_EVENT_FW
     */
    CHR_PLAT_DRV_ERROR_WIFI_FW_DOWN = 0x154,
    CHR_PLAT_DRV_ERROR_BFG_FW_DOWN = 0x155,
    CHR_PLAT_DRV_ERROR_FW_SDIO_INIT = 0x156,

    /* start number 360
     * CHR_PLT_DRV_EVENT_RST
     */
    CHR_PLAT_DRV_ERROR_WIFI_SYSTEM_WCPU_BOOTUP = 0x168,
    CHR_PLAT_DRV_ERROR_BFGX_SYSTEM_RST_BOOTUP = 0x169,
    CHR_PLAT_DRV_ERROR_BFGX_SYSTEM_RST_RECOVERY = 0x16A, /* 362 */
    CHR_PLAT_DRV_ERROR_SUB_SYSTEM_RST = 0x16B,
    CHR_PLAT_DRV_ERROR_ALL_SYSTEM_RST = 0x16C,
    CHR_PLAT_DRV_ERROR_SUB_SYSTEM_RST_SUCC = 0x16D,
    CHR_PLAT_DRV_ERROR_ALL_SYSTEM_RST_SUCC = 0x16E,
    CHR_PLAT_DRV_ERROR_SUB_SYSTEM_RST_FAIL = 0x16F,
    CHR_PLAT_DRV_ERROR_ALL_SYSTEM_RST_FAIL = 0x170,
    CHR_PLAT_DRV_ERROR_W_SYSTEM_RST = 0x171,
    CHR_PLAT_DRV_ERROR_B_SYSTEM_RST = 0x172,
    CHR_PLAT_DRV_ERROR_GLF_SYSTEM_RST = 0x173,

    CHR_PLAT_DRV_ERROR_CORE_RST = 0x174,
    CHR_PLAT_DRV_ERROR_CORE_RST_SUCC = 0x175,
    CHR_PLAT_DRV_ERROR_CORE_RST_FAIL = 0x176,

    /* start number 390
     * CHR_PLT_DRV_EVENT_INI
     */
    CHR_PLAT_DRV_ERROR_INI_MISS = 0x186,

    /* start number 410
     * CHR_PLT_DRV_EVENT_GUGONG
    */
    CHR_PLAT_DRV_ERROR_GUGONG_OPEN_CMD_FAIL = 0x19A,
    CHR_PLAT_DRV_ERROR_GUGONG_OPEN_FAIL = 0x19B,
    CHR_PLAT_DRV_ERROR_GUGONG_NO_CLOSE_CMD = 0x19C,
    CHR_PLAT_DRV_ERROR_GUGONG_ALL_SYSTEM_RST_SUCC = 0x19D,
    CHR_PLAT_DRV_ERROR_GUGONG_QUERY_STATUS_FAIL = 0x19E,
    CHR_PLAT_DRV_ERROR_GUGONG_WIFI_POWER_ON = 0x19F,
    CHR_PLAT_DRV_ERROR_GUGONG_BT_POWER_ON = 0x1A0,
} chr_plat_drv_error_enum;

typedef enum {
    CHR_PLAT_INNER_ERROR_BUS_FRC_ERR       = 0x1,
    CHR_PLAT_INNER_ERROR_BFG_DISALLOW_SLEEP = 0x2,
    CHR_PLAT_INNER_ERROR_PMU_EVENT_REG_ERR = 0x3,
    CHR_PLAT_INNER_ERROR_WIFI_PC_LR = 0x4,
} chr_plat_inner_error_enum;

typedef enum {
    CHR_WIFI_HT_SELF_CURE_ASSOC = 0,
    CHR_WIFI_HT_SELF_CURE_RAOM  = 1,
}chr_wifi_ht_self_cure_occur_enum;

/* CHR ERRNO 错误码各位段组装 */
#define chr_combine_event_error(event, error) \
    ((((event) & CHR_EVENT_MASK) << CHR_EVENT_OFFSET) | (((error) & CHR_ERROR_MASK) << CHR_ERROR_OFFSET))

#define chr_plat_dev(event, error) \
    (CHR_SYSTEM_PLAT | CHR_LAYER_DEV | chr_combine_event_error(event, error))
#define chr_plat_drv(event, error) \
    (CHR_SYSTEM_PLAT | CHR_LAYER_DRV | chr_combine_event_error(event, error))
#define chr_plat_hal(event, error) \
    (CHR_SYSTEM_PLAT | CHR_LAYER_HAL | chr_combine_event_error(event, error))

#define chr_wifi_dev(event, error) \
    (CHR_SYSTEM_WIFI | CHR_LAYER_DEV | chr_combine_event_error(event, error))
#define chr_wifi_drv(event, error) \
    (CHR_SYSTEM_WIFI | CHR_LAYER_DRV | chr_combine_event_error(event, error))
#define chr_wifi_hal(event, error) \
    (CHR_SYSTEM_WIFI | CHR_LAYER_HAL | chr_combine_event_error(event, error))

#define chr_gnss_dev(event, error) \
    (CHR_SYSTEM_GNSS | CHR_LAYER_DEV | chr_combine_event_error(event, error))
#define chr_gnss_drv(event, error) \
    (CHR_SYSTEM_GNSS | CHR_LAYER_DRV | chr_combine_event_error(event, error))
#define chr_gnss_hal(event, error) \
    (CHR_SYSTEM_GNSS | CHR_LAYER_HAL | chr_combine_event_error(event, error))

#define chr_bt_dev(event, error) \
    (CHR_SYSTEM_BT | CHR_LAYER_DEV | chr_combine_event_error(event, error))
#define chr_bt_drv(event, error) \
    (CHR_SYSTEM_BT | CHR_LAYER_DRV | chr_combine_event_error(event, error))
#define chr_bt_hal(event, error) \
    (CHR_SYSTEM_BT | CHR_LAYER_HAL | chr_combine_event_error(event, error))

#define chr_fm_dev(event, error) \
    (CHR_SYSTEM_FM | CHR_LAYER_DEV | chr_combine_event_error(event, error))
#define chr_fm_drv(event, error) \
    (CHR_SYSTEM_FM | CHR_LAYER_DRV | chr_combine_event_error(event, error))
#define chr_fm_hal(event, error) \
    (CHR_SYSTEM_FM | CHR_LAYER_HAL | chr_combine_event_error(event, error))

#define chr_nfc_dev(event, error) \
    (CHR_SYSTEM_NFC | CHR_LAYER_DEV | chr_combine_event_error(event, error))
#define chr_nfc_drv(event, error) \
    (CHR_SYSTEM_NFC | CHR_LAYER_DRV | chr_combine_event_error(event, error))
#define chr_nfc_hal(event, error) \
    (CHR_SYSTEM_NFC | CHR_LAYER_HAL | chr_combine_event_error(event, error))

#define chr_ir_dev(event, error) \
    (CHR_SYSTEM_IR | CHR_LAYER_DEV | chr_combine_event_error(event, error))
#define chr_ir_drv(event, error) \
    (CHR_SYSTEM_IR | CHR_LAYER_DRV | chr_combine_event_error(event, error))
#define chr_ir_hal(event, error) \
    (CHR_SYSTEM_IR | CHR_LAYER_HAL | chr_combine_event_error(event, error))

#endif /* end of chr_errno.h */
