/**
 * Copyright (c) @CompanyNameMagicTag 2024-2024. All rights reserved. \n
 *
 * Description: TIoT Packet Manager. \n
 * Author: @CompanyNameTag \n
 * History: \n
 * 2024-01-13, Create file. \n
 */
#ifndef TIOT_PACKET_MANAGER_H
#define TIOT_PACKET_MANAGER_H

#include "tiot_types.h"
#include "tiot_xfer.h"
#include "tiot_circ_queue.h"
#include "tiot_list.h"
#include "tiot_board_log.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/**
 * @defgroup tiot_common_transfer_packet_handle_tiot_packet_manager Packet Manager
 * @ingroup  tiot_common_transfer_packet_handle_tiot_packet
 * @{
 */

/**
 * @brief  TIOT Complete Packet, simple replace for skb_queue.
 */
#ifdef CONFIG_BOARD_DYNAMIC_ALLOC
typedef struct {
    struct tiot_list_head *head;  /* Dynamic Allocated. */
    uint16_t tag;                 /* BINARY | ASCII. */
} tiot_packet;

typedef struct {
    struct tiot_list_head node;
    uint8_t *data;
    uint32_t len;
} tiot_packet_rx_node;
#else
typedef struct {
    uint32_t len;
    uint16_t tag;        /* BINARY | ASCII. */
} tiot_packet;
#endif

typedef struct {
    tiot_circ_queue_t queue;
    const tiot_packet *queue_nodes;
    uint16_t queue_size;
    tiot_packet active_node;
    osal_wait rx_wait;
} tiot_packet_queue;

typedef uint16_t (* tiot_packet_match_func)(uint32_t msg_type);

typedef struct {
    const tiot_packet_queue *queues;
    const uint16_t queue_num;
    tiot_packet_match_func match;
} tiot_packet_manager;

typedef struct {
    tiot_xfer_manager *xfer;
    uint16_t queue_id;
} tiot_packet_manager_query;

#ifdef CONFIG_BOARD_DYNAMIC_ALLOC
/**
 * @if Eng
 * @brief  Free TIoT packlet.
 * @param  packet Pointer to TIoT packet.
 * @else
 * @brief  释放TIoT消息包。
 * @param  packet TIoT消息包指针。
 * @endif
 */
void tiot_packet_free_packet(tiot_packet *packet);
#endif

/**
 * @if Eng
 * @brief  Deinit packet queue.
 * @param  queue Pointer to TIoT packet queue.
 * @else
 * @brief  去初始化包格式队列。
 * @param  queue 包格式队列指针。
 * @endif
 */
void tiot_packet_queue_deinit(tiot_packet_queue *queue);

/**
 * @if Eng
 * @brief  Get packet pointer for queue_id, from packet manager.
 * @param  manager Packet manager.
 * @param  queue_id The index of queue in packet manager.
 * @return Return packet pointer for queue_id，if no such queue or malloc fail, return NULL.
 * @else
 * @brief  从包管理器中获取指定队列ID的包。
 * @param  manager 包管理器。
 * @param  queue_id 包管理器内队列ID。
 * @return 返回指定队列ID的包的指针，如果没有对应队列，或内存申请失败则返回NULL。
 * @endif
 */
tiot_packet *tiot_packet_manager_get_packet(tiot_packet_manager *manager, uint16_t queue_id);

/**
 * @if Eng
 * @brief  init packet manager.
 * @param  manager Packet manager.
 * @else
 * @brief  初始化包包管理器。
 * @param  manager 包管理器。
 * @endif
 */
int32_t tiot_packet_manager_init(tiot_packet_manager *manager);

static inline int32_t tiot_packet_queue_init(tiot_packet_queue *queue)
{
    const uint32_t queue_mem_size = queue->queue_size * sizeof(tiot_packet);
    tiot_circ_queue_init(&queue->queue, queue->queue_size, sizeof(tiot_packet), (uintptr_t)queue->queue_nodes);
    (void)memset_s((void *)queue->queue_nodes, queue_mem_size, 0, queue_mem_size);
    (void)memset_s((void *)&queue->active_node, sizeof(tiot_packet), 0, sizeof(tiot_packet));
    if (osal_wait_init(&queue->rx_wait) != OSAL_SUCCESS) {
        return -1;
    }
    return 0;
}

static inline void tiot_packet_manager_deinit(tiot_packet_manager *manager)
{
    uint16_t i;
    for (i = 0; i < manager->queue_num; i++) {
        tiot_packet_queue_deinit((tiot_packet_queue *)&manager->queues[i]);
    }
}

static inline tiot_packet_queue *tiot_packet_manager_find_queue(tiot_packet_manager *manager, uint16_t queue_id)
{
    tiot_packet_queue *queue = NULL;
    if (manager == NULL) {
        return queue;
    }
    if (queue_id < manager->queue_num) {
        queue = (tiot_packet_queue *)&manager->queues[queue_id];
    }
    return queue;
}

static inline void tiot_packet_manager_enqueue_packet(tiot_packet_manager *manager, uint16_t queue_id)
{
    tiot_packet_queue *queue = tiot_packet_manager_find_queue(manager, queue_id);
    if (queue == NULL) {
        return;
    }
    if (tiot_circ_queue_enqueue(&queue->queue, (uintptr_t)&queue->active_node) != 0) {
        tiot_print_err("[TIoT:rx]circ queue [%d] no space.\r\n", queue_id);
#ifdef CONFIG_BOARD_DYNAMIC_ALLOC
        tiot_packet_free_packet(&queue->active_node);
#endif
    }
#ifdef CONFIG_BOARD_DYNAMIC_ALLOC
    /* 指针转移至循环队列内或已释放. */
    queue->active_node.head = NULL;
#else
    queue->active_node.len = 0;
#endif
}

static inline void tiot_packet_manager_discard_packet(tiot_packet_manager *manager, uint16_t queue_id)
{
    tiot_packet_queue *queue = tiot_packet_manager_find_queue(manager, queue_id);
    if (queue == NULL) {
        return;
    }
#ifdef CONFIG_BOARD_DYNAMIC_ALLOC
    tiot_packet_free_packet(&queue->active_node);
    /* 指针已释放. */
    queue->active_node.head = NULL;
#else
    queue->active_node.len = 0;
#endif
}

static inline int32_t tiot_packet_manager_has_data(tiot_packet_manager *manager, uint16_t queue_id)
{
    tiot_packet_queue *queue;
    queue = tiot_packet_manager_find_queue(manager, queue_id);
    if (queue == NULL) {
        return 0;
    }
    return (tiot_circ_queue_empty(&queue->queue) == 0);
}

/**
 * @}
 */

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif