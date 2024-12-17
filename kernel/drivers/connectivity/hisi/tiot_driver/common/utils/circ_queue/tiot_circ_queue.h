/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved. \n
 *
 * Description: Tiot Circular queue header. \n
 * Author: @CompanyNameTag \n
 * History: \n
 * 2023-04-18, Create file. \n
 */
#ifndef TIOT_CIRC_QUEUE_H
#define TIOT_CIRC_QUEUE_H

#include "tiot_types.h"
#include "tiot_osal.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/**
 * @defgroup tiot_common_util_tiot_circ_queue Circular Queue
 * @ingroup  tiot_common_util
 * @{
 */

typedef struct {
    uint16_t head; // 队列头索引，入队索引
    uint16_t tail; // 队列尾索引，出队索引
    uint16_t size; // 队列总大小,深度
    uint16_t node_bytes; // 每个节点的长度
    uintptr_t mem_addr; // 队列管理的内存空间（size * node_bytes）起始地址
    osal_spinlock lock;
} tiot_circ_queue_t;

/**
 * @brief  TIoT circ queue empty or not.
 * @param  [in]  queue Circular queue.
 * @return if empty return 1, else return 0.
 */
static inline int32_t tiot_circ_queue_empty(tiot_circ_queue_t *queue)
{
    int32_t ret;
    unsigned long flags;
    osal_spin_lock_irqsave(&queue->lock, &flags);
    ret = (queue->head == queue->tail);
    osal_spin_unlock_irqrestore(&queue->lock, &flags);
    return ret;
}

/**
 * @brief  TIoT circ queue clear.
 * @param  queue Circular queue.
 */
static inline void tiot_circ_queue_clear(tiot_circ_queue_t *queue)
{
    unsigned long flags;
    osal_spin_lock_irqsave(&queue->lock, &flags);
    queue->head = 0;
    queue->tail = 0;
    osal_spin_unlock_irqrestore(&queue->lock, &flags);
}

/**
 * @brief  TIoT circ queue init.
 * @param  [in]  queue Circular queue.
 * @param  [in]  size  Queue size, nodes number max is size - 1, must be power of 2.
 * @param  [in]  node_bytes Fixed node size.
 * @param  [in]  mem_addr   Queue memory start address, ensure has init outside, length should be size * node_bytes.
 */
void tiot_circ_queue_init(tiot_circ_queue_t *queue, uint16_t size, uint16_t node_bytes, uintptr_t mem_addr);

/**
 * @brief  TIoT circ queue enqueue.
 * @param  [in]  queue Circular queue.
 * @param  [in]  src_node Node to be enqueued, will be copyed in queue memory.
 * @return if OK return 0, else return -1.
 */
int32_t tiot_circ_queue_enqueue(tiot_circ_queue_t *queue, uintptr_t src_node);

/**
 * @brief  TIoT circ queue dequeue.
 * @param  [in]  queue Circular queue.
 * @return Pointer to deequeued node.
 */
uintptr_t tiot_circ_queue_dequeue(tiot_circ_queue_t *queue);

/**
 * @brief  TIoT circ queue space.
 * @param  [in]  queue Circular queue.
 * @return Circ queue avalible space for nodes.
 */
uint16_t tiot_circ_queue_space(tiot_circ_queue_t *queue);

/**
 * @}
 */

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif