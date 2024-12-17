/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved. \n
 *
 * Description: BSXX device info. \n
 * Author: @CompanyNameTag \n
 * History: \n
 * 2023-03-25, Create file. \n
 */

#include "bsxx_device_info.h"
#include "bsxx_board_port.h"
#include "tiot_pm_default.h"
#include "tiot_board_uart_port.h"
#include "tiot_cfg_handle_hiburn.h"
#include "tiot_board_log.h"
#include "tiot_controller.h"
#include "tiot_sys_msg_handle.h"
#ifdef CONFIG_FILE_BY_ARRAY
#include "bsxx_fw_file_array.h"
#endif

#define BSXX_POWERON_WAIT_MS      1      /* Wait time before power on. */
#define BSXX_BOOT_TIME_MS         20     /* Power on ==> Boot time. */
#define BSXX_INIT_TIME_MS         600    /* Boot ==> Init time. */
#define BSXX_WAKEUP_PULSE_MS      1      /* GPIO wake up pulse. */
#define BSXX_WAKEUP_WAIT_MS       300    /* Wake up wait ms. */

#define BSXX_BAUD_CHANGE_WAIT_US  11000  /* UART切换波特率等待时间. Baudrate change wait μs. */

#define BSXX_UART_FW_LOAD_BAUDRATE    115200
/*
 * 数据通信时波特率，host侧和对端需要设置一致，默认115200。
 * 上层波特率如果设置为0，则由porting从dts获取，否则配置为CONFIG_BSXX_UART_BAUDRATE
 */
#define BSXX_UART_DEFAULT_BAUDRATE    0

#define bsxx_uart_attr(flow_en)                \
{                                              \
    .data_bits = TIOT_UART_ATTR_DATABIT_8,     \
    .parity    = TIOT_UART_ATTR_PARITY_NONE,   \
    .stop_bits = TIOT_UART_ATTR_STOPBIT_1,     \
    .flow_ctrl = (flow_en)                     \
}

static const tiot_pm_event_entry g_bsxx_pm_event_map[TIOT_PM_EVENT_MAX] = {
    { TIOT_PM_TAG_POWERON, TIOT_PM_STATE_POWEROFF, tiot_pm_default_power_off },
    /* 上电后等待init消息或业务消息才说明进入work状态 */
    { TIOT_PM_TAG_POWEROFF, TIOT_PM_STATE_INIT, tiot_pm_default_power_on },
    /* 只有sleep态才需要唤醒 */
#ifdef CONFIG_BSXX_WAKEUP_TYPE_UART
    { TIOT_PM_TAG_CAN_WAKEUP, TIOT_PM_STATE_WAKING, tiot_pm_default_wakeup_device_by_uart },
#else
    { TIOT_PM_TAG_CAN_WAKEUP, TIOT_PM_STATE_WAKING, tiot_pm_default_wakeup_device_by_gpio },
#endif
    /* work态和sleep态(rtc自唤醒)下请求睡眠均可允许，其他状态下不允许睡眠 */
    { TIOT_PM_TAG_CAN_SLEEP, TIOT_PM_STATE_SLEEP, tiot_pm_default_request_sleep_handle },
    /* 有device wakeup host管脚时，通过此管脚说明已唤醒，且需要下发disallow消息 */
    { TIOT_PM_TAG_POWERON, TIOT_PM_STATE_WORK, tiot_pm_default_wakeup_ack_handle },
    /* 上报init消息或业务消息说明唤醒成功 */
    { TIOT_PM_TAG_CAN_WORK, TIOT_PM_STATE_WORK, NULL },
    { TIOT_PM_TAG_POWERON, TIOT_PM_STATE_INVALID, tiot_pm_default_work_vote_up },
    { TIOT_PM_TAG_POWERON, TIOT_PM_STATE_INVALID, tiot_pm_default_work_vote_down },
    { TIOT_PM_TAG_POWEROFF, TIOT_PM_STATE_SLEEP, NULL }
};

static void bsxx_firmware_before_load(tiot_fw *fw)
{
    /* 如果添加版本号判断是否加载，在此处添加，需要读取版本号，并打开fw文件进行比较。 */
    /* 加载前设置为固件加载默认波特率, 默认关闭流控。 */
    tiot_uart_config bsxx_firmware_load_config = {
        .baudrate = BSXX_UART_FW_LOAD_BAUDRATE,
        .attr = bsxx_uart_attr(TIOT_UART_ATTR_FLOW_CTRL_DISABLE)
    };
    tiot_controller *ctrl = tiot_container_of(fw, tiot_controller, firmware);
    tiot_xfer_manager *xfer = &ctrl->transfer;
    if (tiot_xfer_set_config(xfer, &bsxx_firmware_load_config) != 0) {
        tiot_print_dbg("[TIoT][bsxx]config before load fail.\r\n");
    }
    /* 加载前先清掉rx_buff */
#ifdef CONFIG_XFER_DEFAULT_RX_BUFF
    if (xfer->xmit_ops->rx_mode == TIOT_XMIT_RX_MODE_BUFFED) {
        circ_buf_flush(&xfer->rx_buff);
    }
#endif
}

static void bsxx_firmware_after_load(tiot_fw *fw)
{
    /* 加载过程中可能切换波特率，加载后设置为业务默认波特率，默认开启流控 */
    tiot_uart_config bsxx_transfer_config = {
        .baudrate = BSXX_UART_DEFAULT_BAUDRATE,
        .attr = bsxx_uart_attr(TIOT_UART_ATTR_FLOW_CTRL_ENABLE)
    };
    tiot_controller *ctrl = tiot_container_of(fw, tiot_controller, firmware);
    tiot_xfer_manager *xfer = &ctrl->transfer;
    if (tiot_xfer_set_config(xfer, &bsxx_transfer_config) != 0) {
        tiot_print_dbg("[TIoT][bsxx]config after load fail.\r\n");
    }
}

static int bsxx_rx_wait_func(const void *param)
{
    /* 通过UART上报数据，不需要拉高管脚来通知读取 */
    (void)param;
    return 0;
}

static tiot_device_info g_bsxx_device_info = {
    .pm_event_map = (tiot_pm_event_entry *)g_bsxx_pm_event_map,
    .pm_event_map_size = sizeof(g_bsxx_pm_event_map) / sizeof(tiot_pm_event_entry),
    .exec_cbs = { bsxx_firmware_before_load, bsxx_firmware_after_load, tiot_fw_ext_cmd_handle_hiburn },
    .fw_path = NULL,
    .xfer_info = { TIOT_XMIT_TYPE_UART, bsxx_rx_wait_func },
    .pkt_sys_msg_handle = tiot_pkt_sys_msg_handle,
    .timings = {
        .power_on_wait_ms = BSXX_POWERON_WAIT_MS,
        .boot_time_ms = BSXX_BOOT_TIME_MS,
        .init_time_ms = BSXX_INIT_TIME_MS,
        .wakeup_pulse_ms = BSXX_WAKEUP_PULSE_MS,
        .wakeup_wait_ms = BSXX_WAKEUP_WAIT_MS,
        .baud_change_wait_us = BSXX_BAUD_CHANGE_WAIT_US,
    }
};

const tiot_device_info *bsxx_device_get_info(void)
{
#ifdef CONFIG_FILE_BY_ARRAY
    g_bsxx_device_info.fw_path = bsxx_fw_file_path_get();
#endif
    return &g_bsxx_device_info;
}
