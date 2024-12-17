/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved. \n
 *
 * Description: TIoT pm default. \n
 * Author: @CompanyNameTag \n
 * History: \n
 * 2023-06-09, Create file. \n
 */

#include "tiot_pm_default.h"
#include "tiot_pm_wakelock.h"
#include "tiot_board_pin_port.h"
#include "tiot_board_log.h"
#include "tiot_controller.h"
#include "tiot_xfer_utils.h"
#include "tiot_board.h"
#include "tiot_sys_msg_types.h"
#ifdef CONFIG_PM_WAKEUP_BY_UART
#include "tiot_board_uart_port.h"
#endif

tiot_pm_retcode tiot_pm_default_power_off(tiot_pm *pm)
{
    if (tiot_board_pin_set_level(pm->pm_info[TIOT_PIN_POWER_CTRL], TIOT_PIN_LEVEL_LOW) != 0) {
        return TIOT_PM_EVTRET_FAIL;
    }
    /* 下电时unlock。 */
    tiot_pm_wake_unlock();
    return TIOT_PM_EVTRET_NEXT_STATE;
}

tiot_pm_retcode tiot_pm_default_power_on(tiot_pm *pm)
{
    tiot_controller *ctrl = tiot_container_of(pm, tiot_controller, pm);
    const tiot_device_timings *timings = &ctrl->dev_info->timings;
    uint32_t power_ctrl_pin = pm->pm_info[TIOT_PIN_POWER_CTRL];
    int32_t ret = 0;
    do {
        ret = tiot_board_pin_set_level(power_ctrl_pin, TIOT_PIN_LEVEL_LOW);
        if (ret != 0) {
            tiot_print_err("[TIoT][pm]set pwr pin level low fail.\n");
            break;
        }
        osal_msleep(timings->power_on_wait_ms);
        ret = tiot_board_pin_set_level(power_ctrl_pin, TIOT_PIN_LEVEL_HIGH);
        if (ret != 0) {
            tiot_print_err("[TIoT][pm]set pwr pin level high fail.\n");
            break;
        }
        osal_msleep(timings->boot_time_ms);
    } while (0);

    if (ret != 0) {
        tiot_print_err("[TIoT:pm]power on fail.\n");
        return TIOT_PM_EVTRET_FAIL;
    }
    /* 上电时lock。 */
    tiot_pm_wake_lock();
    return TIOT_PM_EVTRET_NEXT_STATE;
}

#ifdef CONFIG_PM_WAKEUP_BY_UART
/* 不可在中断环境下使用. */
static int32_t tiot_pm_wakeup_check_sleeped(uint32_t wakein_pin)
{
    int32_t ret;
    uint8_t level;
    uint8_t i;
    const uint8_t sleep_check = 10;      /* 等待睡眠，重试10次. */
    const uint8_t sleep_wait_ms = 2;     /* 等待睡眠，每次等待2ms. */
    /* 唤醒时，如果wakein_pin存在，检查wakein_pin拉低后再发送唤醒, 确保主机已休眠. */
    if (wakein_pin == TIOT_PIN_NONE) {
        return 0;
    }
    for (i = 0; i < sleep_check; i++) {
        ret = tiot_board_pin_get_level(wakein_pin, &level);
        if (ret != 0) {
            tiot_print_err("[TIoT:pm]wakein pin level get error.\n");
            return -1;
        }
        if (level == TIOT_PIN_LEVEL_LOW) {
            return 0;
        }
        osal_msleep(sleep_wait_ms);
    }
    /* 此时device侧可能又重新拉高管脚通知有数据上报，保持状态即可, 响应event即转成work. */
    tiot_print_warning("[TIoT:pm]wakein pin level high after check 10 times.\n");
    return -1;
}

tiot_pm_retcode tiot_pm_default_wakeup_device_by_uart(tiot_pm *pm)
{
    tiot_controller *ctrl = tiot_container_of(pm, tiot_controller, pm);
    int32_t ret;
    tiot_print_info("[TIoT][pm]wakeup device enter.\n");
    const uint32_t wakeup_baudrate = 115200;    /* UART唤醒时设为115200波特率. */
    const uint8_t wakeup_cmd = 0x00;
    tiot_xfer_manager *xfer = &ctrl->transfer;
    ret = tiot_pm_wakeup_check_sleeped(pm->pm_info[TIOT_PIN_DEVICE_WAKEUP_HOST]);
    if (ret != 0) {
        return TIOT_PM_EVTRET_KEEP_STATE;
    }

    tiot_uart_config old_config = { 0 };
    tiot_uart_config wakeup_config = { wakeup_baudrate,
        { .data_bits = TIOT_UART_ATTR_DATABIT_8, .parity = TIOT_UART_ATTR_PARITY_NONE,
          .stop_bits = TIOT_UART_ATTR_STOPBIT_1, .flow_ctrl = TIOT_UART_ATTR_FLOW_CTRL_DISABLE } };
    (void)tiot_xfer_get_config(xfer, &old_config);
    (void)tiot_xfer_set_config(xfer, &wakeup_config);
    ret = tiot_xfer_direct_send(xfer, &wakeup_cmd, sizeof(uint8_t));
    (void)tiot_xfer_set_config(xfer, &old_config);
    if (ret != sizeof(uint8_t)) {
        return TIOT_PM_EVTRET_FAIL;
    }
    return TIOT_PM_EVTRET_NEXT_STATE;
}
#endif

tiot_pm_retcode tiot_pm_default_wakeup_device_by_gpio(tiot_pm *pm)
{
    int32_t ret;
    tiot_controller *ctrl = tiot_container_of(pm, tiot_controller, pm);
    const tiot_device_timings *timings = &ctrl->dev_info->timings;
    uint32_t wakeup_device_pin = pm->pm_info[TIOT_PIN_HOST_WAKEUP_DEVICE];

    if (wakeup_device_pin == TIOT_PIN_NONE) {
        tiot_print_err("[TIoT][pm]wakeup pin not set.\n");
        return TIOT_PM_EVTRET_FAIL;
    }

    do {
        ret = tiot_board_pin_set_level(wakeup_device_pin, TIOT_PIN_LEVEL_LOW);
        if (ret != 0) {
            tiot_print_err("[TIoT][pm]set wakeup pin level low fail.\n");
            break;
        }
        osal_msleep(timings->wakeup_pulse_ms);
        ret = tiot_board_pin_set_level(wakeup_device_pin, TIOT_PIN_LEVEL_HIGH);
        if (ret != 0) {
            tiot_print_err("[TIoT][pm]set wakeup pin level high fail.\n");
            break;
        }
        osal_msleep(timings->wakeup_pulse_ms);
        ret = tiot_board_pin_set_level(wakeup_device_pin, TIOT_PIN_LEVEL_LOW);
        if (ret != 0) {
            tiot_print_err("[TIoT][pm]set wakeup pin level low fail.\n");
            break;
        }
    } while (0);

    if (ret != 0) {
        return TIOT_PM_EVTRET_FAIL;
    }
    return TIOT_PM_EVTRET_NEXT_STATE;
}

#ifdef CONFIG_PM_MANAGE_LOWPOWER
tiot_pm_retcode tiot_pm_default_request_sleep_handle(tiot_pm *pm)
{
    const uint8_t allow_sleep_command[] = { 0x7e, 0x00, 0x06, 0x00,
                                            SYS_PL_BFGNI_HOST_ASKTO_SLP,
                                            0x7e };
    int32_t ret;
    tiot_controller *ctrl = NULL;
    tiot_xfer_manager *xfer = NULL;
    if (pm->work_vote == 1) {
        return TIOT_PM_EVTRET_KEEP_STATE;
    }
    ctrl = tiot_container_of(pm, tiot_controller, pm);
    xfer = &ctrl->transfer;
    ret = xfer->xmit_ops->write(&xfer->xmit, allow_sleep_command, sizeof(allow_sleep_command));
    if (ret != sizeof(allow_sleep_command)) {
        return TIOT_PM_EVTRET_FAIL;
    }
    tiot_print_info("[TIoT][pm]allow device sleep.\n");
    /* device侧睡眠时可睡眠 */
    tiot_pm_wake_unlock();
    return TIOT_PM_EVTRET_NEXT_STATE;
}

int32_t tiot_pm_send_disallow_sleep(tiot_xfer_manager *xfer)
{
    const uint8_t disallow_sleep_command[] = { 0x7e, 0x00, 0x06, 0x00,
                                               SYS_PL_BFGNI_HOST_DISALLOW_DEVSLP,
                                               0x7e };
    int32_t ret;
    ret = xfer->xmit_ops->write(&xfer->xmit, disallow_sleep_command, sizeof(disallow_sleep_command));
    if (ret != sizeof(disallow_sleep_command)) {
        tiot_print_alert("[TIoT:fatal]send disallow msg fail.\r\n");
        return -1;
    }
    return 0;
}

tiot_pm_retcode tiot_pm_default_wakeup_ack_handle(tiot_pm *pm)
{
    int32_t ret;
    uint8_t input_level = TIOT_PIN_LEVEL_LOW;
    uint16_t pre_state = pm->state;
    tiot_controller *ctrl = tiot_container_of(pm, tiot_controller, pm);
    tiot_xfer_manager *xfer = &ctrl->transfer;
    uint32_t wakeup_host_pin = pm->pm_info[TIOT_PIN_DEVICE_WAKEUP_HOST];

    if (wakeup_host_pin == TIOT_PIN_NONE) {
        tiot_print_err("[TIoT][pm]wakeup ack event called but dev_wk_host pin is none.\n");
        return TIOT_PM_EVTRET_KEEP_STATE;
    }

    /*
     * host上电后处于INIT状态时，此时接收到device拉高wakeup_host gpio，则不能下发disallow消息，
     * 否则会多lock()一次，导致host后续无法睡下去。
     */
    if (pre_state == TIOT_PM_STATE_INIT) {
        /* INIT -> WORK */
        return TIOT_PM_EVTRET_NEXT_STATE;
    }

    /* 有数据上报时不睡眠 */
    /* 自唤醒流程: device_wake_out --> wake_lock -......-> request_sleep --> wake_unlock */
    /* 当前在唤醒中断内持锁，并直接回复disallow消息。 */

    /* check wakeup_ack */
    ret = tiot_board_pin_get_level(wakeup_host_pin, &input_level);
    if ((ret != 0) || (input_level == TIOT_PIN_LEVEL_LOW)) {
        tiot_print_err("[TIoT][pm]get dev_wk_host pin level fail or false positive [state:0x%x].\n", pre_state);
        return TIOT_PM_EVTRET_KEEP_STATE;
    }
    /* 使用临时flag区分suspend. */
    if (xfer->xmit_suspend & TIOT_XFER_SUSPEND) {
        tiot_print_info("[TIoT:pm]delay disallow msg [state:0x%x].\n", pre_state);
        xfer->xmit_suspend |= TIOT_XFER_DELAY_MSG;
    } else {
        tiot_print_info("[TIoT][pm]send disallow sleep msg [state:0x%x].\n", pre_state);
        if (tiot_pm_send_disallow_sleep(xfer) != 0) {
            return TIOT_PM_EVTRET_FAIL;
        }
    }
    if ((pre_state == TIOT_PM_STATE_SLEEP) || (pre_state == TIOT_PM_STATE_WAKING)) {
        tiot_pm_wake_lock();
    }
    return TIOT_PM_EVTRET_NEXT_STATE;
}

tiot_pm_retcode tiot_pm_default_work_vote_up(tiot_pm *pm)
{
    /* 发送时不睡眠 */
    tiot_pm_wake_lock();
    pm->work_vote = 1;
    return TIOT_PM_EVTRET_KEEP_STATE;
}

tiot_pm_retcode tiot_pm_default_work_vote_down(tiot_pm *pm)
{
    pm->work_vote = 0;
    /* 发送结束后可睡眠 */
    tiot_pm_wake_unlock();
    return TIOT_PM_EVTRET_KEEP_STATE;
}
#endif
