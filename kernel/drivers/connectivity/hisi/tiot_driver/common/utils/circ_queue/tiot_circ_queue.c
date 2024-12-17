/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved. \n
 *
 * Description: Tiot Circular queue header. \n
 * Author: @CompanyNameTag \n
 * History: \n
 * 2023-04-18, Create file. \n
 */
#include "tiot_circ_queue.h"
#ifdef __KERNEL__
#include <linux/compiler.h>
#endif
#include "securec.h"

#ifndef __KERNEL__
#if (defined(__GNUC__))
#define likely(_expr)      __builtin_expect(!!(_expr), 1)
#define unlikely(_expr)    __builtin_expect(!!(_expr), 0)
#else
#define likely(_expr)   (_expr)
#define unlikely(_expr) (_expr)
#endif
#endif

static inline uint16_t tiot_circ_queue_mask(const tiot_circ_queue_t *queue)
{
    return (queue->size - 1);
}

static inline uintptr_t tiot_circ_queue_node(const tiot_circ_queue_t *queue, uint16_t idx)
{
    return (queue->mem_addr + ((idx & tiot_circ_queue_mask(queue)) * queue->node_bytes));
}

static inline uint16_t _tiot_circ_queue_space(tiot_circ_queue_t *queue)
{
    return ((queue->tail - (queue->head + 1)) & tiot_circ_queue_mask(queue));
}

void tiot_circ_queue_init(tiot_circ_queue_t *queue, uint16_t size, uint16_t node_bytes, uintptr_t mem_addr)
{
    if (unlikely((queue == NULL) || (size == 0) || (node_bytes == 0) || (mem_addr == 0))) {
        return;
    }

    /* 队列大小应为2的幂次方 */
    if ((size & (size - 1)) != 0) {
        return;
    }

    osal_spin_lock_init(&queue->lock);

    queue->head = 0;
    queue->tail = 0;
    queue->size = size;
    queue->node_bytes = node_bytes;
    queue->mem_addr = mem_addr;
}

/*
 * 功能描述: 入队列一个节点
 */
int32_t tiot_circ_queue_enqueue(tiot_circ_queue_t *queue, uintptr_t src_node)
{
    uintptr_t dis_node;
    unsigned long flags;

    osal_spin_lock_irqsave(&queue->lock, &flags);

    // 检查队列是否已满
    if (_tiot_circ_queue_space(queue) == 0) {
        osal_spin_unlock_irqrestore(&queue->lock, &flags);
        return -1;
    }

    // 保存节点内容
    dis_node = tiot_circ_queue_node(queue, queue->head);
    if (memcpy_s((void *)dis_node, queue->node_bytes, (void *)src_node, queue->node_bytes) != EOK) {
        osal_spin_unlock_irqrestore(&queue->lock, &flags);
        return -1;
    }

    // 索引更新到下一位置
    queue->head = (queue->head + 1) & tiot_circ_queue_mask(queue);

    osal_spin_unlock_irqrestore(&queue->lock, &flags);
    return 0;
}

/*
 * 功能描述: 从循环队列中，取出最早入队列的节点
 */
uintptr_t tiot_circ_queue_dequeue(tiot_circ_queue_t *queue)
{
    uintptr_t node;
    unsigned long flags;

    osal_spin_lock_irqsave(&queue->lock, &flags);

    // 检查是否为空
    if (queue->head == queue->tail) {
        osal_spin_unlock_irqrestore(&queue->lock, &flags);
        return 0;
    }

    node = tiot_circ_queue_node(queue, queue->tail);

    // 索引更新到下一位置
    queue->tail = (queue->tail + 1) & tiot_circ_queue_mask(queue);

    osal_spin_unlock_irqrestore(&queue->lock, &flags);
    return node;
}

uint16_t tiot_circ_queue_space(tiot_circ_queue_t *queue)
{
    uint16_t space;
    unsigned long flags;

    osal_spin_lock_irqsave(&queue->lock, &flags);
    space = _tiot_circ_queue_space(queue);
    osal_spin_unlock_irqrestore(&queue->lock, &flags);
    return space;
}
