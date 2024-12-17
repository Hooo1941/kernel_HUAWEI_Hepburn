/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved. \n
 *
 * Description: Power Manager module header. \n
 * Author: @CompanyNameTag \n
 * History: \n
 * 2023-03-27, Create file. \n
 */
#ifndef TIOT_PM_H
#define TIOT_PM_H

#include "tiot_types.h"
#include "tiot_circ_queue.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/**
 * @defgroup tiot_common_pm Power Manager
 * @ingroup  tiot_common
 * @{
 */

#define TIOT_PM_EVENT_BUFF_MAX    8

/**
 * @brief  TIoT PM tags.
 */
typedef enum {
    TIOT_PM_TAG_INVALID = 0,                     /*!< Invalid PM tag. */
    TIOT_PM_TAG_POWEROFF = ((uint16_t)1 << 0),   /*!< Power off tag. */
    TIOT_PM_TAG_POWERON  = ((uint16_t)1 << 1),   /*!< Power on tag. */
    /* Low power tags. */
    TIOT_PM_TAG_CAN_WORK  = ((uint16_t)1 << 2),  /*!< Low power can work tag. */
    TIOT_PM_TAG_CAN_SLEEP = ((uint16_t)1 << 3),  /*!< Low power can sleep tag. */
    TIOT_PM_TAG_CAN_WAKEUP = ((uint16_t)1 << 4), /*!< Low power can wakeup tag. */
    TIOT_PM_TAG_MAX = (uint16_t)0xFFFF           /* PM tag max. */
} tiot_pm_tag;

/**
 * @brief  TIoT PM states.
 */
typedef enum {
    TIOT_PM_STATE_INVALID = 0,                        /*!< Invalid PM state. */
    TIOT_PM_STATE_POWEROFF = TIOT_PM_TAG_POWEROFF,    /*!< Power off state. */
    TIOT_PM_STATE_POWERON = TIOT_PM_TAG_POWERON,      /*!< Power on state. */
    /* Low power states */
    /*!< Low power init state, can work. */
    TIOT_PM_STATE_INIT = TIOT_PM_TAG_POWERON | TIOT_PM_TAG_CAN_WORK,
    /*!< Low power work state, can sleep. */
    TIOT_PM_STATE_WORK = TIOT_PM_TAG_POWERON | TIOT_PM_TAG_CAN_SLEEP,
    /*!< Low power sleep state, can sleep, can be waked up. */
    TIOT_PM_STATE_SLEEP = TIOT_PM_TAG_POWERON | TIOT_PM_TAG_CAN_WORK | TIOT_PM_TAG_CAN_WAKEUP,
    /*!< Low power sleep state, can be waked up. */
    TIOT_PM_STATE_WAKING = TIOT_PM_TAG_POWERON | TIOT_PM_TAG_CAN_WAKEUP
} tiot_pm_state;

/**
 * @brief Tiot PM event types.
 */
typedef enum {
    /* OUTPUT */
    TIOT_PM_EVENT_POWEROFF,       /*!< Power off. */
    TIOT_PM_EVENT_POWERON,        /*!< Power on. */
    TIOT_PM_EVENT_WAKEUP,         /*!< Host wakeup device. */
    /* INPUT，for host to manage device lowpower */
    TIOT_PM_EVENT_REQUEST_SLEEP,  /*!< Device request sleep. */
    TIOT_PM_EVENT_WAKEUP_ACK,     /*!< Device wakeup host. */
    TIOT_PM_EVENT_REPORT_WORK,    /*!< Device transmit port report message. */
    /* Vote */
    TIOT_PM_EVENT_WORK_VOTE_UP,   /*!< Host keep device at work state. */
    TIOT_PM_EVENT_WORK_VOTE_DOWN, /*!< Host not keep device at work state. */
    TIOT_PM_EVENT_INIT_SLEEP,     /*!< Init state set sleep. */
    TIOT_PM_EVENT_MAX
} tiot_pm_event;

/**
 * @brief Tiot PM event retcode.
 */
typedef enum {
    TIOT_PM_EVTRET_FAIL = -1,         /* Event handle failed. */
    TIOT_PM_EVTRET_NEXT_STATE = 0,    /* Change to next state. */
    TIOT_PM_EVTRET_KEEP_STATE = 1,    /* Keep current state. */
} tiot_pm_retcode;

struct _tiot_pm;

/**
 * @brief  Tiot PM event handler.
 */
typedef tiot_pm_retcode (* tiot_pm_event_handler)(struct _tiot_pm *pm);

/**
 * @brief Tiot PM event handle entry.
 */
typedef struct {
    uint16_t condition_mask;        /*!< Enter event condition, could be combination of PM tags. */
    uint16_t next_state;            /*!< Next PM state. */
    tiot_pm_event_handler handler;  /*!< PM event handler, NULL mean this event don't need operation. */
} tiot_pm_event_entry;

/**
 * @brief Tiot PM control block.
 */
typedef struct _tiot_pm {
    uint16_t state;                        /*!< Current PM state. */
    uint16_t work_vote;                    /*!< Host require device work vote. */
    const uint32_t *pm_info;               /*!< PM board info, pin indexs. */
    osal_wait state_wait;                  /*!< PM state wait, used for wait some state. */
    tiot_circ_queue_t event_queue;         /*!< PM event queue for syncing events. */
    /*!< PM event queue buffer. */
    tiot_pm_event event_queue_buff[TIOT_PM_EVENT_BUFF_MAX];
    volatile uint8_t is_in_event;          /*!< PM event processing. */
    uint8_t event_map_size;                /*!< PM event handle map size. */
    /*!< PM event handle map, must point to static allocated array. */
    const tiot_pm_event_entry *event_map;
} tiot_pm;

/**
 * @brief  PM control block init.
 * @param  [in]  pm        PM control block.
 * @param  [in]  pm_info   Pointer to board PM info, static allocated, from device saved board info.
 * @param  [in]  event_map Registed PM event handle map, static allocated, from *_device_info.c.
 * @param  [in]  event_map_size Registed PM event handle map size.
 * @return zero if OK;
 *         non-zero if FAIL.
 */
int32_t tiot_pm_init(tiot_pm *pm, const uint32_t *pm_info,
                     const tiot_pm_event_entry *event_map, uint8_t event_map_size);

/**
 * @brief  Set event to PM control block.
 * @param  [in]  pm    PM control block.
 * @param  [in]  event PM event.
 * @return zero if OK;
 *         non-zero if FAIL.
 */
int32_t tiot_pm_set_event(tiot_pm *pm, tiot_pm_event event);

/**
 * @brief  PM control block wait for state.
 * @param  [in]  pm    PM control block.
 * @param  [in]  state PM state，caller must ensure state is valid.
 * @param  [in]  wait_timeout Wait timeout, shound not be too long.
 * @return negative if wait state fail;
 *         zero if wait state timeout;
 *         postive if wait state OK.
 */
int32_t tiot_pm_wait_state(tiot_pm *pm, uint16_t state, uint32_t wait_timeout);

/**
 * @brief  PM control block deinit.
 * @param  [in]  pm        PM control block.
 */
void tiot_pm_deinit(tiot_pm *pm);

/**
 * @}
 */

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif