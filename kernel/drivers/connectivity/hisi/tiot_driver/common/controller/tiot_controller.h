/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved. \n
 *
 * Description: TIoT device controller header. \n
 * Author: @CompanyNameTag \n
 * History: \n
 * 2023-03-17, Create file. \n
 */

#ifndef TIOT_CONTROLLER_H
#define TIOT_CONTROLLER_H

#ifndef __KERNEL__
#include <stdbool.h>
#endif
#include "tiot_board.h"
#include "tiot_pm.h"
#include "tiot_firmware.h"
#include "tiot_xfer.h"
#include "tiot_device_info.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/**
 * @defgroup tiot_common_controller Controller
 * @ingroup  tiot_common
 * @{
 */

/**
 * @brief TIoT controller state.
 */
typedef enum {
    TIOT_CONTROLLER_UNINIT,
    TIOT_CONTROLLER_INIT,
    TIOT_CONTROLLER_OPENING,
    TIOT_CONTROLLER_OPEN,
} tiot_controller_state;

/**
 * @brief TIoT controller info provided outside.
 */
typedef struct {
    tiot_board_info *board_info;       /*!< Board infomations. */
    const tiot_device_info *dev_info;  /*!< Reserved device info, must be allocated staticly. */
} tiot_controller_info;

struct _tiot_controller;

/**
 * @brief  TIoT controller ops.
 */
typedef struct {
    int32_t (* open)(struct _tiot_controller *ctrl, void *param);
    void (* close)(struct _tiot_controller *ctrl);
    int32_t (* write)(struct _tiot_controller *ctrl, const uint8_t *data,
                      uint32_t len, const tiot_xfer_tx_param *param);
    int32_t (* read)(struct _tiot_controller *ctrl, uint8_t *buff, uint32_t buff_len, const tiot_xfer_rx_param *param);
} tiot_controller_ops;

/**
 * @brief TIoT device controller.
 */
typedef struct _tiot_controller {
    uint32_t state;                    /*!< Current state. */
    tiot_pm pm;                        /*!< Power Manangement module. */
    tiot_fw firmware;                  /*!< Firmware module. */
    tiot_xfer_manager transfer;        /*!< Transfer manager modele. */
    const tiot_device_info *dev_info;  /*!< Reserved device info, must be allocated staticly. */
    tiot_controller_ops ops;           /*!< Controller ops, open, close, write, read. */
    volatile uint32_t open_count;      /*!< open count. */
    osal_mutex open_mutex;             /*!< open state mutex. */
} tiot_controller;

/**
 * @brief  TIoT find controller by pin from controllers.
 * @param  [in]  ctrls TIoT controller instances array.
 * @param  [in]  ctrl_num TIoT controller instances num.
 * @param  [in]  pin Pin number.
 * @param  [in]  pin_type Pin type.
 * @return if found controller, return controller pointer, else return NULL.
 */
static inline tiot_controller *tiot_find_controller_by_pin(tiot_controller *ctrls, uint8_t ctrl_num,
                                                           uint32_t pin, uint32_t pin_type)
{
    const uint32_t *pm_info;
    uint8_t i;
    for (i = 0; i < ctrl_num; i++) {
        pm_info = ctrls[i].pm.pm_info;
        if (pm_info[pin_type] == pin) {
            return &ctrls[i];
        }
    }
    return NULL;
}

/**
 * @brief  TIoT controller init.
 * @param  [in]  ctrl TIoT controller instance.
 * @param  [in]  info TIoT controller information.
 * @return zero if OK;
 *         non-zero if FAIL.
 */
int32_t tiot_controller_init(tiot_controller *ctrl, tiot_controller_info *info);

/**
 * @brief  TIoT controller open, if already opened, add open count only.
 * @param  [in]  ctrl TIoT controller instance.
 * @param  [in]  param TIoT controller open param.
 * @return zero if OK;
 *         non-zero if FAIL.
 */
int32_t tiot_controller_open(tiot_controller *ctrl, void *param);

/**
 * @brief  TIoT controller check opened or not, must be used after init.
 * @param  [in]  ctrl TIoT controller instance.
 * @return true if Opened;
 *         false if not opened.
 */
bool tiot_controller_check_opened(tiot_controller *ctrl);

/**
 * @brief  TIoT controller close, if opened multiple times, only remove open count.
 * @param  [in]  ctrl TIoT controller instance.
 */
void tiot_controller_close(tiot_controller *ctrl);

/**
 * @brief  TIoT controller deinit.
 * @param  [in]  ctrl TIoT controller instance.
 */
void tiot_controller_deinit(tiot_controller *ctrl);

/**
 * @}
 */

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif
