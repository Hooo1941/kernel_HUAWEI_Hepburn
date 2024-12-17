/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved. \n
 *
 * Description: Device info header. \n
 * Author: @CompanyNameTag \n
 * History: \n
 * 2023-03-25, Create file. \n
 */
#ifndef TIOT_DEVICE_INFO_H
#define TIOT_DEVICE_INFO_H

#include "tiot_pm.h"
#include "tiot_firmware.h"
#include "tiot_xfer.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/**
 * @defgroup tiot_common_interface_device Device Interface
 * @ingroup  tiot_common_interface
 * @{
 */

/**
 * @brief  Device common timing specs.
 */
typedef struct {
    const uint32_t power_on_wait_ms;       /*!< Before power on wait time. */
    const uint32_t boot_time_ms;           /*!< Boot wait time. */
    const uint32_t init_time_ms;           /*!< Init time wait time. */
    const uint32_t wakeup_pulse_ms;        /*!< Wakeup pin pulse time. */
    const uint32_t wakeup_wait_ms;         /*!< Wakeup wait time. */
    const uint32_t baud_change_wait_us;    /*!< Baudrate change wait time. */
} tiot_device_timings;

struct _tiot_controller;

/**
 * @brief  Device infomations to be registed in driver.
 */
typedef struct {
    /*!< Power management event map, set NULL if use default event map. */
    tiot_pm_event_entry *pm_event_map;
    uint8_t pm_event_map_size;
    /*!< Firmware callback called before and after commands execute. */
    const tiot_fw_ops exec_cbs;
    const tiot_file_path *fw_path;

    /*!< Transfer */
    tiot_xfer_device_info xfer_info;

    /*!< Tiot packet sys msg handle. */
    void (*pkt_sys_msg_handle)(struct _tiot_controller *ctrl, uint8_t code);

    tiot_device_timings timings;
} tiot_device_info;

/**
 * @}
 */

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif