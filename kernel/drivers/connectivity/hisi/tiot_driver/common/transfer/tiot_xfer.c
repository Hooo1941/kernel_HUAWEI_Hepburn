/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved. \n
 *
 * Description: TIoT transfer module. \n
 * Author: @CompanyNameTag \n
 * History: \n
 * 2023-04-18, Create file. \n
 */
#include "tiot_xfer.h"
#include "tiot_xfer_utils.h"
#include "tiot_board_log.h"

#ifdef CONFIG_XFER_TX_SUPPORT_TASK
#define TIOT_XFER_MANAGER_MAX   16

typedef enum {
    TIOT_XFER_TASK_UNINIT,
    TIOT_XFER_TASK_INIT,
    TIOT_XFER_TASK_ABORT,
} tiot_xfer_task_state;

typedef struct {
    osal_task *task;
    tiot_xfer_manager *xfer_list[TIOT_XFER_MANAGER_MAX];
    volatile uint16_t xfer_count;
    volatile uint16_t state;
    osal_wait tx_wait;
} tiot_xfer_send_task;

/* 单线程 */
static tiot_xfer_send_task g_tiot_xfer_send_task = { 0 };

static int32_t tiot_xfer_send_task_init(tiot_xfer_manager *xfer);
static void tiot_xfer_send_task_quit(tiot_xfer_manager *xfer);

static int32_t tiot_xfer_send_task_queue_init(tiot_xfer_manager *xfer)
{
    /* 仅单一发送队列 */
    const uint32_t tx_queue_node_num = CONFIG_XFER_TX_NODE_NUM;
    uint32_t tx_queue_mem_size = sizeof(tiot_xfer_tx_data) * tx_queue_node_num;
    xfer->tx_queue_mem = (uintptr_t)osal_kmalloc(tx_queue_mem_size, OSAL_GFP_KERNEL);
    if (xfer->tx_queue_mem == 0) {
        tiot_print_err("[TIoT][xfer]tx queue malloc failed.\n");
        return -1;
    }
    (void)memset_s((void *)xfer->tx_queue_mem, tx_queue_mem_size, 0, tx_queue_mem_size);
    tiot_circ_queue_init(&xfer->tx_queue, tx_queue_node_num, sizeof(tiot_xfer_tx_data), xfer->tx_queue_mem);
    return 0;
}

static void tiot_xfer_send_task_queue_deinit(tiot_xfer_manager *xfer)
{
    if (xfer->tx_queue_mem != 0) {
        osal_kfree((void *)xfer->tx_queue_mem);
        xfer->tx_queue_mem = 0;
    }
}
#endif

static inline int32_t tiot_xfer_default_rx_buff_init(tiot_xfer_manager *xfer)
{
#ifdef CONFIG_XFER_DEFAULT_RX_BUFF
    (void)memset_s((void *)xfer->rx_buff_mem, CONFIG_XFER_RX_BUFF_SIZE, 0, CONFIG_XFER_RX_BUFF_SIZE);
    circ_buf_init(&xfer->rx_buff, &xfer->rx_buff_write, &xfer->rx_buff_read,
                  (void *)xfer->rx_buff_mem, CONFIG_XFER_RX_BUFF_SIZE);
    return 0;
#else
    tiot_unused(xfer);
    tiot_print_err("[TIoT][xfer]no rx buff, check tiot_autoconfig.h.\n");
    return -1;
#endif
}

int32_t tiot_xfer_init(tiot_xfer_manager *xfer, uintptr_t xmit_id, const tiot_xfer_device_info *dev_info)
{
    int32_t ret = 0;
    const tiot_xmit_ops *xmit_ops;
    /* 等待函数必须不为空，否则osal_wait_timeout_interruptible不能正常工作. */
    if ((xfer == NULL) || (dev_info == NULL) || (dev_info->rx_wait_func == NULL)) {
        return -1;
    }
    xmit_ops = tiot_xmit_get_ops(dev_info->xmit_type);
    if (xmit_ops == NULL) {
        tiot_print_dbg("[TIoT][xfer]transmit type invalid.\n");
        return -1;
    }
    xfer->tx_queue_mem = 0;
#ifdef CONFIG_XFER_TX_SUPPORT_TASK
    /* 仅单一发送队列 */
    ret = tiot_xfer_send_task_init(xfer);
    if (ret != 0) {
        goto rx_wait_err;
    }
#endif
    if ((xmit_ops->rx_mode == TIOT_XMIT_RX_MODE_BUFFED) && (tiot_xfer_default_rx_buff_init(xfer) != 0)) {
        goto rx_wait_err;
    }
    if (osal_wait_init(&xfer->rx_wait) != OSAL_SUCCESS) {
        tiot_print_err("[TIoT][xfer]rx wait init failed.\n");
        goto rx_wait_err;
    }
    if (osal_mutex_init(&xfer->dev_mutex) != OSAL_SUCCESS) {
        tiot_print_err("[TIoT][xfer]device mutex init failed.\n");
        goto dev_mutex_err;
    }
    if (osal_mutex_init(&xfer->read_mutex) != OSAL_SUCCESS) {
        tiot_print_err("[TIoT][xfer]read mutex init failed.\n");
        goto read_mutex_err;
    }

    /* Transmit实例初始化. */
    xfer->xmit_ops = xmit_ops;
    xfer->xmit.id = xmit_id;
    xfer->xmit_state = TIOT_XMIT_STATE_CLOSE;
    /* 接收初始化 */
    xfer->rx_wait_func = dev_info->rx_wait_func;
    (void)memset_s(&xfer->config, sizeof(struct tiot_xmit_config), 0, sizeof(struct tiot_xmit_config));
     /* 包格式处理初始化 */
    (void)memset_s(&xfer->pkt_ops, sizeof(tiot_xfer_packet_ops), 0, sizeof(tiot_xfer_packet_ops));
    tiot_print_suc("[TIoT][xfer]transfer manager init succ.\n");
    return ret;
read_mutex_err:
    osal_mutex_destroy(&xfer->dev_mutex);
dev_mutex_err:
    osal_wait_destroy(&xfer->rx_wait);
rx_wait_err:
#ifdef CONFIG_XFER_TX_SUPPORT_TASK
    tiot_xfer_send_task_quit(xfer);
#endif
    return -1;
}

void tiot_xfer_deinit(tiot_xfer_manager *xfer)
{
    if (xfer == NULL) {
        tiot_print_dbg("[TIoT][xfer]transfer manager is NULL.\n");
        return;
    }

    tiot_print_info("[TIoT][xfer]transfer manager deinit.\n");
    /* 先关闭xfer保证硬件不再发送接收 */
    tiot_xfer_close(xfer);

    /* Transmit实例去初始化. */
    (void)memset_s(&xfer->xmit, sizeof(tiot_xmit), 0, sizeof(tiot_xmit));
    (void)memset_s(&xfer->config, sizeof(struct tiot_xmit_config), 0, sizeof(struct tiot_xmit_config));
    xfer->xmit_ops = NULL;

    tiot_xfer_uninstall_packet(xfer);

#ifdef CONFIG_XFER_TX_SUPPORT_TASK
    tiot_xfer_send_task_quit(xfer);
#endif
    osal_wait_destroy(&xfer->rx_wait);

    /* 互斥锁初始化 */
    osal_mutex_destroy(&xfer->dev_mutex);
    osal_mutex_destroy(&xfer->read_mutex);
}

int32_t tiot_xfer_install_packet(tiot_xfer_manager *xfer, tiot_xfer_packet_ops *pkt_ops, uintptr_t pkt_context)
{
    if (xfer == NULL) {
        tiot_print_dbg("[TIoT][xfer]transfer manager is NULL.\n");
        return -1;
    }

    osal_mutex_lock(&xfer->dev_mutex);
    (void)memcpy_s(&xfer->pkt_ops, sizeof(tiot_xfer_packet_ops), pkt_ops, sizeof(tiot_xfer_packet_ops));
    xfer->pkt_context = pkt_context;
    osal_mutex_unlock(&xfer->dev_mutex);
    return 0;
}

void tiot_xfer_uninstall_packet(tiot_xfer_manager *xfer)
{
    if (xfer == NULL) {
        tiot_print_dbg("[TIoT][xfer]transfer manager is NULL.\n");
        return;
    }

    osal_mutex_lock(&xfer->dev_mutex);
    xfer->pkt_context = 0;
    memset_s(&xfer->pkt_ops, sizeof(tiot_xfer_packet_ops), 0, sizeof(tiot_xfer_packet_ops));
    osal_mutex_unlock(&xfer->dev_mutex);
}

int32_t tiot_xfer_open(tiot_xfer_manager *xfer, tiot_xfer_open_param *param)
{
    int32_t ret;
    tiot_xmit_callbacks cbs = { tiot_xfer_rx_notify };
    if (xfer == NULL) {
        tiot_print_dbg("[TIoT][xfer]transfer manager is NULL.\n");
        return -1;
    }

    osal_mutex_lock(&xfer->dev_mutex);
    if (xfer->xmit_state != TIOT_XMIT_STATE_CLOSE) {
        osal_mutex_unlock(&xfer->dev_mutex);
        tiot_print_err("[TIoT][xfer]transmit state already opened.\n");
        return -1;
    }
    ret = xfer->xmit_ops->open(&xfer->xmit, &cbs);
    if (ret != 0) {
        osal_mutex_unlock(&xfer->dev_mutex);
        tiot_print_err("[TIoT][xfer]transmit open failed.\n");
        return ret;
    }

    if ((xfer->xmit_ops->rx_mode == TIOT_XMIT_RX_MODE_BUFFED) && (param != NULL)) {
        (void)memcpy_s(&xfer->rx_cbk_param, sizeof(tiot_xfer_cbk_param),
                       &param->rx_cbk_param, sizeof(tiot_xfer_cbk_param));
    } else {
        xfer->rx_cbk_param.rx_callback = NULL;
    }

    xfer->xmit_state = TIOT_XMIT_STATE_OPEN;

    osal_mutex_unlock(&xfer->dev_mutex);
    return ret;
}

int32_t tiot_xfer_set_config(tiot_xfer_manager *xfer, struct tiot_xmit_config *config)
{
    int32_t ret;
    if ((xfer == NULL) || (config == NULL)) {
        tiot_print_dbg("[TIoT][xfer]transfer manager or config is NULL.\n");
        return -1;
    }
    (void)osal_mutex_lock(&xfer->dev_mutex);
    if (xfer->xmit_state == TIOT_XMIT_STATE_CLOSE) {
        tiot_print_dbg("[TIoT][xfer]transmit state closed, cannot set config.\n");
        (void)osal_mutex_unlock(&xfer->dev_mutex);
        return -1;
    }
    ret = xfer->xmit_ops->set_config(&xfer->xmit, config);
    if (ret == 0) {
        (void)memcpy_s(&xfer->config, sizeof(struct tiot_xmit_config), config, sizeof(struct tiot_xmit_config));
    }
    (void)osal_mutex_unlock(&xfer->dev_mutex);
    return ret;
}

int32_t tiot_xfer_get_config(tiot_xfer_manager *xfer, struct tiot_xmit_config *config)
{
    if ((xfer == NULL) || (config == NULL)) {
        tiot_print_dbg("[TIoT][xfer]transfer manager or config is NULL.\n");
        return -1;
    }

    (void)memcpy_s(config, sizeof(struct tiot_xmit_config), &xfer->config, sizeof(struct tiot_xmit_config));
    return 0;
}

void tiot_xfer_close(tiot_xfer_manager *xfer)
{
    if (xfer == NULL) {
        tiot_print_dbg("[TIoT][xfer]transfer manager is NULL.\n");
        return;
    }

    (void)osal_mutex_lock(&xfer->dev_mutex);
    if (xfer->xmit_state == TIOT_XMIT_STATE_CLOSE) {
        (void)osal_mutex_unlock(&xfer->dev_mutex);
        tiot_print_dbg("[TIoT][xfer]transmit state already closed.\n");
        return;
    }

    xfer->xmit_ops->close(&xfer->xmit);
    (void)memset_s(&xfer->rx_cbk_param, sizeof(tiot_xfer_cbk_param), 0, sizeof(tiot_xfer_cbk_param));

    xfer->xmit_state = TIOT_XMIT_STATE_CLOSE;
#ifdef CONFIG_XFER_DEFAULT_RX_BUFF
    (void)memset_s((void *)xfer->rx_buff_mem, CONFIG_XFER_RX_BUFF_SIZE, 0, CONFIG_XFER_RX_BUFF_SIZE);
    circ_buf_flush(&xfer->rx_buff);
#endif
    (void)osal_mutex_unlock(&xfer->dev_mutex);
    return;
}

#ifdef CONFIG_XFER_TX_SUPPORT_TASK
static int tiot_xfer_send_task_wait_condition(void *param)
{
    tiot_xfer_send_task *send_task = &g_tiot_xfer_send_task;
    if (send_task->state == TIOT_XFER_TASK_ABORT) {
        return OSAL_WAIT_CONDITION_TRUE;
    }

    uint16_t xfer_count = g_tiot_xfer_send_task.xfer_count;
    for (uint16_t i = 0; i < xfer_count; i++) {
        if (tiot_circ_queue_empty(&xfer_list[i]->tx_queue) == 0) {
            return OSAL_WAIT_CONDITION_TRUE;
        }
    }
    return 0;
}

static void tiot_xfer_send_schedule(tiot_xfer_manager *xfer)
{
    if ((xfer == NULL) || (xfer->xmit_state == TIOT_XMIT_STATE_CLOSE)) {
        tiot_print_info("[TIoT][xfer]xfer is null or xmit close.");
        return;
    }
    tiot_xfer_tx_data *node = NULL;
    do {
        node = (tiot_xfer_tx_data *)tiot_circ_queue_dequeue(&xfer->tx_queue);
        if (node != NULL) {
            tiot_xfer_direct_send(xfer, node->data, node->len);
        } else {
            tiot_print_info("[TIoT][xfer]tx queue is empty.");
        }
    } while (node != NULL);
}

static void tiot_xfer_send_task_handle(void *param)
{
    (void)param;
    tiot_xfer_manager **xfer_list = g_tiot_xfer_send_task.xfer_list;
    uint16_t xfer_count;
    for (;;) {
        /* 等待某个transfer manager有数据或退出 */
        osal_wait_timeout_interruptible(&g_tiot_xfer_send_task.tx_wait, tiot_xfer_send_task_wait_condition,
                                        NULL, OSAL_WAIT_FOREVER);
        if (g_tiot_xfer_send_task.state == TIOT_XFER_TASK_ABORT) {
            break;
        }
        xfer_count = g_tiot_xfer_send_task.xfer_count;
        for (uint16_t i = 0; i < xfer_count; i++) {
            tiot_xfer_send_schedule(xfer_list[i]);
        }
    }

    /* 退出流程. */
    g_tiot_xfer_send_task.state = TIOT_XFER_TASK_UNINIT;
    osal_wait_wakeup(&g_tiot_xfer_send_task.tx_wait);
}

static int32_t tiot_xfer_send_task_init(tiot_xfer_manager *xfer)
{
    const uint32_t stack_size = 2048;
    tiot_xfer_send_task *send_task = &g_tiot_xfer_send_task;
    if (send_task->xfer_count == 0) {
        if (osal_wait_init(&send_task->tx_wait) != OSAL_SUCCESS) {
            tiot_print_err("[TIoT][xfer]send task wait init failed.\n");
            return -1;
        }
        send_task->task = osal_kthread_create(tiot_xfer_send_task_handle, NULL, "tiot_tx", stack_size);
        if (send_task->task == NULL) {
            osal_wait_destroy(&send_task->tx_wait);
            tiot_print_err("[TIoT][xfer]send task init failed.\n");
            return -1;
        }
    }
    if (tiot_xfer_send_task_queue_init(xfer) != 0) {
        return -1;
    }
    send_task->xfer_count++;
    send_task->xfer_list[send_task->xfer_count] = xfer;
    return 0;
}

static int tiot_xfer_send_task_quit_wait_condition(void *param)
{
    if (g_tiot_xfer_send_task.state == TIOT_XFER_TASK_UNINIT) {
        return OSAL_WAIT_CONDITION_TRUE;
    }
    return 0;
}

/* Transfer manager去初始化时调用 */
static void tiot_xfer_send_task_quit(tiot_xfer_manager *xfer)
{
    tiot_xfer_send_task *send_task = &g_tiot_xfer_send_task;
    uint16_t xfer_count = send_task->xfer_count;
    uint16_t i = 0;
    for (; i < xfer_count; i++) {
        if (send_task->xfer_list[i] == xfer) {
            send_task->xfer_list[i] = NULL;
            break;
        }
    }
    tiot_xfer_send_task_queue_deinit(xfer);
    if (i == xfer_count) {
        /* 未注册 */
        return;
    }
    send_task->xfer_count--;
    /* 当所有transfer manager均去初始化时，释放task */
    if (send_task->xfer_count == 0) {
        g_tiot_xfer_send_task.state = TIOT_XFER_TASK_ABORT;
        osal_wait_wakeup(&g_tiot_xfer_send_task.tx_wait);
        /* 等待任务退出 */
        osal_wait_timeout_interruptible(&g_tiot_xfer_send_task.tx_wait,
                                        tiot_xfer_send_task_quit_wait_condition, NULL, OSAL_WAIT_FOREVER);
        osal_kthread_destroy(g_tiot_xfer_send_task.task, 0);
        g_tiot_xfer_send_task.task = NULL;
        (void)memset_s(&g_tiot_xfer_send_task, sizeof(tiot_xfer_send_task), 0, sizeof(tiot_xfer_send_task));
    }
}
#endif

int32_t tiot_xfer_send(tiot_xfer_manager *xfer, const uint8_t *data, uint32_t len, const tiot_xfer_tx_param *param)
{
    int32_t ret;
#ifdef CONFIG_XFER_TX_SUPPORT_TASK
    tiot_xfer_tx_data node = { data, len };
#endif
    if ((xfer == NULL) || (data == NULL)) {
        tiot_print_err("[TIoT][xfer]transfer manager is NULL or input data is NULL.");
        return -1;
    }

    if (xfer->pkt_ops.tx_push != NULL) {
        ret = xfer->pkt_ops.tx_push(xfer, data, len, param);
    } else {
#ifdef CONFIG_XFER_TX_SUPPORT_TASK
        ret = tiot_circ_queue_enqueue(&xfer->tx_queue, (uintptr_t)&node);
#else
        ret = tiot_xfer_direct_send(xfer, data, len);
#endif
    }

#ifdef CONFIG_XFER_TX_SUPPORT_TASK
    tiot_xfer_send_task *send_task = &g_tiot_xfer_send_task;
    if (ret == 0) {
        osal_wait_wakeup(&send_task->tx_wait);
    }
#endif
    return ret;
}

void tiot_xfer_rx_notify(tiot_xmit *xmit, uint8_t *data, uint32_t len)
{
    int32_t ret = 0;
    tiot_xfer_manager *xfer = (tiot_xfer_manager *)xmit;
    if (xfer == NULL) {
        tiot_print_err("[TIoT][rx]notified transmit is NULL.\n");
        return;
    }

    /* 可能中断内被调用 */
    if (xfer->xmit_state == TIOT_XMIT_STATE_CLOSE) {
        tiot_print_dbg("[TIoT][rx]transmit state closed.\n");
        return;
    }

    if (xfer->xmit_ops->rx_mode == TIOT_XMIT_RX_MODE_BUFFED) {
        if ((data == NULL) || (len == 0)) {
            tiot_print_err("[TIoT][rx]notified data is NULL or len is 0.\n");
            return;
        }
        if (xfer->pkt_ops.rx_data_store != NULL) {
            /* 由包格式决定什么时候唤醒rx_wait */
            xfer->pkt_ops.rx_data_store(xfer, data, len);
            return;
        } else {
            /* 存入默认buffer */
            ret = tiot_xfer_read_store(&xfer->rx_buff, data, len);
        }
    }

    if (ret == 0) {
        osal_wait_wakeup_interruptible(&xfer->rx_wait);
    } else {
        tiot_print_err("[TIoT][rx]circ buff not stored.\n");
    }
}

int32_t tiot_xfer_recv(tiot_xfer_manager *xfer, uint8_t *buff, uint32_t len, const tiot_xfer_rx_param *param)
{
    int32_t ret;
    if ((xfer == NULL) || (param == NULL)) {
        tiot_print_dbg("[TIoT][rx]transfer manager is NULL or param is NULL.\n");
        return -1;
    }

    osal_mutex_lock(&xfer->dev_mutex);
    if (xfer->xmit_state == TIOT_XMIT_STATE_CLOSE) {
        osal_mutex_unlock(&xfer->dev_mutex);
        tiot_print_dbg("[TIoT][rx]transmit state closed.\n");
        return -1;
    }
    osal_mutex_unlock(&xfer->dev_mutex);

    if (xfer->pkt_ops.read_out != NULL) {
        ret =  xfer->pkt_ops.read_out(xfer, buff, len, param);
    } else {
        /* 默认读取路径，固件加载时使用. */
        ret = tiot_xfer_read_out(xfer, buff, len, param->timeout);
    }

    return ret;
}
