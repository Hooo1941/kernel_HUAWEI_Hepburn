/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved. \n
 *
 * Description: Power Manager module. \n
 * Author: @CompanyNameTag \n
 * History: \n
 * 2023-03-27, Create file. \n
 */

#include "tiot_pm.h"
#include "tiot_pm_wakelock.h"
#include "tiot_osal.h"
#include "securec.h"
#include "tiot_controller.h"
#include "tiot_board_log.h"

typedef struct {
    tiot_pm *pm;
    uint16_t state;
} pm_wait_state;

/* 传入的event_map必须为静态申请数组 */
int32_t tiot_pm_init(tiot_pm *pm, const uint32_t *pm_info,
                     const tiot_pm_event_entry *event_map, uint8_t event_map_size)
{
    if ((pm == NULL) || (pm_info == NULL) || (event_map == NULL) || (event_map_size == 0)) {
        tiot_print_dbg("[TIoT][pm]pm or pm info or event_map is NULL.\n");
        return -1;
    }
    if (osal_wait_init(&pm->state_wait) != OSAL_SUCCESS) {
        tiot_print_err("[TIoT][pm]pm wait init fail.\n");
        return -1;
    }

    /* 外部确保为静态变量 */
    pm->pm_info = pm_info;
    pm->event_map = event_map;
    pm->event_map_size = event_map_size;

    /* 此队列仅用于处理中断内产生pm事件情况，按产生顺序排序，预计不会同时在多个中断内产生pm事件，预留8个已经足够. */
    (void)memset_s(pm->event_queue_buff, TIOT_PM_EVENT_BUFF_MAX, 0, TIOT_PM_EVENT_BUFF_MAX);
    tiot_circ_queue_init(&pm->event_queue, TIOT_PM_EVENT_BUFF_MAX,
                         sizeof(tiot_pm_event), (uintptr_t)pm->event_queue_buff);
    pm->is_in_event = 0;
    pm->work_vote = 0;
    pm->state = TIOT_PM_STATE_POWEROFF;
    return 0;
}

static int32_t tiot_pm_handle_event(tiot_pm *pm, tiot_pm_event event)
{
    tiot_pm_retcode ret = TIOT_PM_EVTRET_NEXT_STATE;
    /* 取对应事件的 [判断条件 - 执行处理 - 设置下个状态] 流程 */
    const tiot_pm_event_entry *entry = &pm->event_map[event];

    /* 当前状态不符合tag mask条件, 不需要处理，直接返回OK */
    if ((pm->state & entry->condition_mask) == 0) {
        tiot_print_dbg("[TIoT][pm]state %x not match condition %x, ignore event\n", pm->state, entry->condition_mask);
        return 0;
    }

    /* 当前状态符合tag mask条件，执行对应事件处理 */
    /* 约定：handler为空不需要处理，例如上报业务消息说明芯片处于WORK状态，只需要切换状态即可 */
    if (entry->handler != NULL) {
        ret = entry->handler(pm);
    }

    if (ret == TIOT_PM_EVTRET_NEXT_STATE) {
        /* 处理成功，设为下一个状态 */
        tiot_print_dbg("[TIoT][pm]evt:%x, st:%x, next_st: %x.\n", event, pm->state, entry->next_state);
        pm->state = entry->next_state;
        /* 切换状态时保证设置event. */
        osal_wait_wakeup_interruptible(&pm->state_wait);
    } else if (ret == TIOT_PM_EVTRET_FAIL) {
        /* 处理失败，返回false. */
        tiot_print_dbg("[TIoT][pm]evt:%x, st:%x, event fail.\n", event, pm->state);
        return -1;
    } else {   /* ret == TIOT_PM_EVTRET_KEEP_STATE */
        /* 处理成功，保持当前状态 */
        tiot_print_dbg("[TIoT][pm]evt:%x, st:%x, keep state.\n", event, pm->state);
    }

    return 0;
}

int32_t tiot_pm_set_event(tiot_pm *pm, tiot_pm_event event)
{
    int32_t ret;
    tiot_pm_event *event_node;
    if ((pm == NULL) || (event >= TIOT_PM_EVENT_MAX) || (event >= pm->event_map_size)) {
        tiot_print_dbg("[TIoT][pm]pm is NULL or illegal event.\n");
        return -1;
    }

    tiot_circ_queue_enqueue(&pm->event_queue, (uintptr_t)&event);
    /* 依靠队列防止pm事件处理嵌套，此处不使用互斥锁（可能在中断内产生事件），也不使用自旋锁（不能保证提供自旋锁） */
    if (pm->is_in_event == 1) {
        return 0;
    }

    do {
        event_node = (tiot_pm_event *)tiot_circ_queue_dequeue(&pm->event_queue);
        if (event_node == NULL) {
            return 0;
        }
        /* pm处理过程持唤醒锁，防止pm未转换完状态就直接进sleep。 */
        tiot_pm_wake_lock();
        pm->is_in_event = 1;
        ret = tiot_pm_handle_event(pm, *event_node);
        pm->is_in_event = 0;
        tiot_pm_wake_unlock();
        if (ret != 0) {
            tiot_circ_queue_clear(&pm->event_queue);
            break;
        }
    } while (event_node != NULL);

    return ret;
}

static int pm_wait_state_condition(const void *param)
{
    pm_wait_state *wait_state = (pm_wait_state *)param;
    return (wait_state->pm->state == wait_state->state);
}

int32_t tiot_pm_wait_state(tiot_pm *pm, uint16_t state, uint32_t wait_timeout)
{
    pm_wait_state wait_state = { 0 };
    if (state == TIOT_PM_STATE_INVALID) {
        return -1;
    }
    wait_state.pm = pm;
    wait_state.state = state;
    return osal_wait_timeout_interruptible(&pm->state_wait, pm_wait_state_condition, &wait_state, wait_timeout);
}

void tiot_pm_deinit(tiot_pm *pm)
{
    tiot_circ_queue_clear(&pm->event_queue);
    osal_spin_lock_destroy(&pm->event_queue.lock);
    osal_wait_destroy(&pm->state_wait);
    (void)memset_s(pm->event_queue_buff, TIOT_PM_EVENT_BUFF_MAX, 0, TIOT_PM_EVENT_BUFF_MAX);
    pm->state = TIOT_PM_STATE_INVALID;
    pm->work_vote = 0;
}