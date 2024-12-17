/**
 * Copyright (c) @CompanyNameMagicTag 2024-2024. All rights reserved. \n
 *
 * Description:  \n
 * Author: @CompanyNameTag \n
 * History: \n
 * 2024-01-27, Create file. \n
 */

#include "tiot_packet_manager.h"
#include "tiot_osal.h"

#ifdef CONFIG_BOARD_DYNAMIC_ALLOC
void tiot_packet_free_packet(tiot_packet *packet)
{
    tiot_packet_rx_node *pos, *next;
    tiot_list_head_create(tmp_head);
    if ((packet == NULL) || (packet->head == NULL)) {
        return;
    }
    tiot_list_replace(packet->head, &tmp_head);
    /* 使用临时节点遍历删除. */
    tiot_list_for_each_entry_safe(pos, next, &tmp_head, node, tiot_packet_rx_node, tiot_packet_rx_node) {
        if (pos != NULL) {
            tiot___list_del_entry(&pos->node);
            osal_kfree(pos);
        }
    }
    osal_kfree(packet->head);
}
#endif

void tiot_packet_queue_deinit(tiot_packet_queue *queue)
{
#ifdef CONFIG_BOARD_DYNAMIC_ALLOC
    /* 动态申请buffer时才需要清除. */
    /* 释放所有未处理缓存数据. */
    tiot_packet *packet = NULL;
    tiot_packet_free_packet(&queue->active_node);
    do {
        packet = (tiot_packet *)tiot_circ_queue_dequeue(&queue->queue);
        if (packet == NULL) {
            break;
        }
        tiot_packet_free_packet(packet);
    } while (packet != NULL);
#endif
    osal_spin_lock_destroy(&queue->queue.lock);
    osal_wait_destroy(&queue->rx_wait);
}

tiot_packet *tiot_packet_manager_get_packet(tiot_packet_manager *manager, uint16_t queue_id)
{
    tiot_packet_queue *queue = tiot_packet_manager_find_queue(manager, queue_id);
    if (queue == NULL) {
        return NULL;
    }
#ifdef CONFIG_BOARD_DYNAMIC_ALLOC
    if (queue->active_node.head == NULL) {
        queue->active_node.head = (struct tiot_list_head *)osal_kmalloc(sizeof(struct tiot_list_head), OSAL_GFP_KERNEL);
        if (queue->active_node.head == NULL) {
            return NULL;
        }
        tiot_init_list_head(queue->active_node.head);
    }
#endif
    return &queue->active_node;
}

int32_t tiot_packet_manager_init(tiot_packet_manager *manager)
{
    uint16_t i, j;
    if (manager->match == NULL) {
        return -1;
    }
    for (i = 0; i < manager->queue_num; i++) {
        if (tiot_packet_queue_init((tiot_packet_queue *)&(manager->queues[i])) != 0) {
            for (j = 0; j < i; j++) { /* 释放之前已初始化的queue */
                tiot_packet_queue_deinit((tiot_packet_queue *)&manager->queues[j]);
            }
            return -1;
        }
    }
    return 0;
}
