/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved. \n
 *
 * Description:  \n
 * Author: @CompanyNameTag \n
 * History: \n
 * 2023-08-03, Create file. \n
 */

#include "tiot_packet.h"
#include "tiot_sys_msg_handle.h"
#include "tiot_controller.h"
#include "tiot_board_log.h"
#include "tiot_xfer_utils.h"
#include "securec.h"

typedef enum {
    TIOT_PKT_FRAME_IDLE,
    TIOT_PKT_FRAME_HEAD,
    TIOT_PKT_FRAME_PAYLOAD,
    TIOT_PKT_FRAME_TAIL
} tiot_pkt_rx_frame;

typedef enum {
    TIOT_PKT_RX_SEPERATE_START,
    TIOT_PKT_RX_SEPERATE_COMMON,
    TIOT_PKT_RX_SEPERATE_LAST,
    TIOT_PKT_RX_COMPLETE,
} tiot_pkt_rx_sep_tag;

typedef void (* tiot_pkt_msg_handler)(tiot_packet_context *ctx);

static void tiot_packet_sys_func(tiot_packet_context *ctx);
static void tiot_packet_recv_not_seperated_data(tiot_packet_context *ctx);
static void tiot_packet_recv_seperated_data(tiot_packet_context *ctx);
static int32_t tiot_packet_copy_from_queue(tiot_packet_context *ctx, tiot_packet_queue *queue,
                                           uint8_t *buff, uint32_t buff_len);

static tiot_pkt_msg_handler g_tiot_pkt_msg_handle[MSG_BUTT] = {
    [SYS_MSG] = tiot_packet_sys_func,                      // SYS_MSG = 0x00,         系统串口消息
    [GNSS_FIRST_MSG] = tiot_packet_recv_seperated_data,    // GNSS_FIRST_MSG = 0x02,  GNSS串口消息，第一个分段消息
    [GNSS_COMMON_MSG] = tiot_packet_recv_seperated_data,   // GNSS_COMMON_MSG = 0x03, GNSS串口消息，中间分段消息
    [GNSS_LAST_MSG] = tiot_packet_recv_seperated_data,     // GNSS_LAST_MSG = 0x04,   GNSS串口消息，最后一个分段消息
    [OML_MSG] = tiot_packet_recv_not_seperated_data,       // OML_MSG = 0x0e,         OML串口消息
    [SLE_MSG] = tiot_packet_recv_not_seperated_data,       // SLE_MSG = 0x15,         SLE串口消息
};

static const tiot_packet_subsys_info g_tiot_pkt_subsys_info[SUBSYS_BUTT] = {
    { PKG_SEPRETED, GNSS_FIRST_MSG, GNSS_COMMON_MSG, GNSS_LAST_MSG },
    { PKG_NOT_SEPRETED, OML_MSG, OML_MSG, OML_MSG },
    { PKG_NOT_SEPRETED, SYS_MSG, SYS_MSG, SYS_MSG }
};

/* 将旧packet节点推入队列，若队列已满，则丢包. */
static inline void tiot_cur_packet_to_queue(tiot_packet_context *ctx)
{
    tiot_packet_manager_enqueue_packet(&ctx->rx_manager, ctx->rx_queue_id);
    ctx->frame_cnt = 0;
    ctx->packet = NULL;
}

/* 丢包时释放packet节点，重置当前packet节点为空，收到新数据时重新获取对应节点. */
static inline void tiot_cur_packet_discard(tiot_packet_context *ctx)
{
    tiot_packet_manager_discard_packet(&ctx->rx_manager, ctx->rx_queue_id);
    ctx->frame_cnt = 0;
    ctx->packet = NULL;
}

/* 按照subsys code更新当前packet节点. */
static inline void tiot_cur_packet_update(tiot_packet_context *ctx, uint8_t subsys_code)
{
    uint16_t queue_id = ctx->rx_manager.match(subsys_code);
    if ((ctx->packet == NULL) || (queue_id != ctx->rx_queue_id)) {
        ctx->rx_queue_id = queue_id;
        ctx->packet = tiot_packet_manager_get_packet(&ctx->rx_manager, queue_id);
    }
}

static void tiot_packet_report_work(tiot_packet_context *ctx)
{
    tiot_xfer_manager *xfer;
    if (ctx == NULL) {
        return;
    }
    xfer = ctx->xfer;
    if (xfer == NULL) {
        return;
    }
    tiot_controller *ctrl = tiot_container_of(xfer, tiot_controller, transfer);
    tiot_pm_set_event(&ctrl->pm, TIOT_PM_EVENT_REPORT_WORK);
}

#ifdef CONFIG_BOARD_DYNAMIC_ALLOC
static inline tiot_packet_rx_node *tiot_packet_rx_node_malloc(uint32_t len)
{
    tiot_packet_rx_node *node = (tiot_packet_rx_node *)osal_kmalloc(sizeof(tiot_packet_rx_node) + len, OSAL_GFP_KERNEL);
    if (node == NULL) {
        tiot_print_err("[TIoT]pkt rx node malloc fail.");
        return NULL;
    }
    node->data = (uint8_t *)node + sizeof(tiot_packet_rx_node);
    node->len = len;
    return node;
}
#endif

int32_t tiot_packet_init(tiot_packet_context *ctx, tiot_xfer_manager *xfer, const tiot_packet_context_param *param)
{
    tiot_packet_buffer *buff;
    if ((ctx == NULL) || (xfer == NULL) || (xfer->pkt_context != 0) || (param == NULL)) {
        return -1;
    }

    /* 接收状态机初始状态均为0，只需要memset为0即可. */
    (void)memset_s((void *)ctx, sizeof(tiot_packet_context), 0, sizeof(tiot_packet_context));
    (void)memcpy_s((void *)&ctx->rx_manager, sizeof(tiot_packet_manager),
                   &param->rx_manager_info, sizeof(tiot_packet_manager));

    if (osal_mutex_init(&ctx->tx_packet_mutex) != OSAL_SUCCESS) {
        tiot_print_err("[TIoT][pkt]tx packet mutex init failed.\n");
        return -1;
    }

    if (tiot_packet_manager_init(&ctx->rx_manager) != 0) {
        osal_mutex_destroy(&ctx->tx_packet_mutex);
        return -1;
    }
#ifdef CONFIG_BOARD_DYNAMIC_ALLOC
    tiot_unused(buff);
#else
    buff = param->buff;
    (void)memset_s(buff->start, buff->size, 0, buff->size);
    circ_buf_init(&(ctx->rx_buff.buf), &(ctx->rx_buff.buff_write), &(ctx->rx_buff.buff_read), buff->start, buff->size);
#endif
    ctx->flags = param->flags;
    ctx->xfer = xfer;
    /* 调用tiot_xfer_install_packet后才注册到xfer内. */
    return 0;
}

void tiot_packet_deinit(tiot_packet_context *ctx)
{
    if (ctx == NULL) {
        return;
    }
    ctx->xfer = NULL;
    ctx->packet = NULL;
    tiot_packet_manager_deinit(&ctx->rx_manager);
    osal_mutex_destroy(&ctx->tx_packet_mutex);
}

static void tiot_packet_sys_func(tiot_packet_context *ctx)
{
    uint8_t syschar = ctx->sys_msg;
    tiot_xfer_manager *xfer = ctx->xfer;
    tiot_controller *ctrl = tiot_container_of(xfer, tiot_controller, transfer);

    if (ctrl->dev_info->pkt_sys_msg_handle != NULL) {
        ctrl->dev_info->pkt_sys_msg_handle(ctrl, syschar);
    } else {
        tiot_print_warning("[TIoT][pkt]pkt_sys_msg_handle is NULL.\r\n");
    }
}

static void tiot_packet_cums_recv_handle(tiot_packet_context *ctx)
{
    int32_t copy_len;
    tiot_packet_queue *queue;
    tiot_xfer_cbk_param rx_cbk_param = ctx->xfer->rx_cbk_param;

    /* 直接使用上一次的rx_queue_id. */
    queue = tiot_packet_manager_find_queue(&ctx->rx_manager, ctx->rx_queue_id);
    if (queue == NULL) {
        tiot_print_err("[TIoT:pkt]can't find subsys queue(%d).\r\n", ctx->rx_queue_id);
        return;
    }

    if (rx_cbk_param.rx_callback != NULL) {
        /* 用户注册了rx callback，直接就地处理 */
        copy_len = tiot_packet_copy_from_queue(ctx, queue, rx_cbk_param.buff, rx_cbk_param.buff_len);
        rx_cbk_param.rx_callback(rx_cbk_param.buff, copy_len);
    } else {
        /* 唤醒等待的reader */
        osal_wait_wakeup_interruptible(&queue->rx_wait);
    }
}

static void tiot_packet_recv_not_seperated_data(tiot_packet_context *ctx)
{
    /* 处理之前的包错误, 释放当前包 */
    if (ctx->frame_error == 1) {
        tiot_cur_packet_discard(ctx);
        ctx->frame_error = 0;
        return;
    }
    /* 正常完整包. */
    ctx->packet->tag = TIOT_PKT_TAG_BINARY;
    tiot_cur_packet_to_queue(ctx);
    tiot_packet_cums_recv_handle(ctx);
}

static void tiot_packet_recv_seperated_data(tiot_packet_context *ctx)
{
    uint8_t subsys_code = ctx->head.subsys_code;
    /* 包顺序检测 */
    if (subsys_code == GNSS_FIRST_MSG) {
        /* First 包必须为第一个包 */
        if (ctx->frame_cnt != 1) {
            tiot_print_err("[TIoT:pkt]parse error: expect first packet.\r\n");
            ctx->frame_error = 1;
        }
        return;
    }
    if (subsys_code == GNSS_COMMON_MSG) {
        /* 必须先有fisrt包再有common包，所以收到common包时frame_cnt最小为2 */
        if (ctx->frame_cnt < 2) {
            tiot_print_err("[TIoT:pkt]parse error: expect first-common packet.\r\n");
            ctx->frame_error = 1;
        }
        return;
    }
    /* 收完Last包数据才处理之前的包序错误, 确保完整包被释放 */
    if ((subsys_code == GNSS_LAST_MSG) && (ctx->frame_error == 1)) {
        tiot_print_err("[TIoT:pkt]packet order error, discard the whole packet.\r\n");
        tiot_cur_packet_discard(ctx);
        ctx->frame_error = 0;
        return;
    }
    /* 正常完整包. */
    ctx->packet->tag = TIOT_PKT_TAG_BINARY;
    tiot_cur_packet_to_queue(ctx);
    tiot_packet_cums_recv_handle(ctx);
}

static int32_t tiot_packet_head_check(tiot_packet_head *head)
{
    /* 包头错误!!! */
    if (head->head != TIOT_PKT_HEAD) {
        tiot_print_err("[TIoT:pkt]head error: 0x%x\r\n", head->head);
        return -1;
    }
    /* 子系统错误 */
    if (head->subsys_code >= MSG_BUTT) {
        tiot_print_err("[TIoT:pkt]subsys error: 0x%x\r\n", head->subsys_code);
        return -1;
    }
    /* 系统消息特殊判断, 长度必须为0x6. */
    if ((head->subsys_code == SYS_MSG) && (head->len != (TIOT_PKT_PKG_SIZE + sizeof(uint8_t)))) {
        tiot_print_err("[TIoT:pkt]sys msg len error: 0x%x\r\n", head->len);
        return -1;
    }
    /* payload大小 小于0 或大于最大包大小 */
    if ((head->len <= TIOT_PKT_PKG_SIZE) ||
        (head->len > (TIOT_PKT_PAYLOAD_MAX_SIZE + TIOT_PKT_PKG_SIZE))) {
        tiot_print_err("[TIoT:pkt]payload error: 0x%x\r\n", head->len);
        return -1;
    }
    return 0;
}

static int32_t tiot_packet_head_try_recovery(tiot_packet_context *ctx)
{
    uint8_t i;
    uint8_t *head_data;
    errno_t err;
    head_data = (uint8_t *)&ctx->head;
    /* 出现问题时，先把上一个起始标志位丢弃. */
    for (i = 1; i < sizeof(tiot_packet_head); i++) {
        /* 头部出现其他pkt_head时，可以尝试恢复. */
        if (head_data[i] == TIOT_PKT_HEAD) {
            break;
        }
    }
    if (i == sizeof(tiot_packet_head)) {
        return -1;
    }
    tiot_print_dbg("[TIoT][pkt]head: [%x %x %x %x].\r\n",
                   head_data[0], head_data[1], head_data[2], head_data[3]);  /* 打印恢复前head数据. */
    tiot_print_warning("[TIoT][pkt]head recovery.\r\n");
    err = memmove_s((void *)&ctx->head, sizeof(tiot_packet_head), &head_data[i], sizeof(tiot_packet_head) - i);
    if (err != EOK) {
        tiot_print_err("[TIoT][pkt]memmove_s error.\r\n");
        return -1;
    }
    /* 更新需要读取的个数 & 位置. */
    ctx->frame_need_read = i;
    ctx->pos = &head_data[sizeof(tiot_packet_head) - i];
    return 0;
}

static void tiot_packet_head_done(tiot_packet_context *ctx)
{
    tiot_packet_head *head = &ctx->head;
    uint16_t payload_len = head->len - TIOT_PKT_PKG_SIZE;
#ifdef CONFIG_BOARD_DYNAMIC_ALLOC
    tiot_packet_rx_node *node = NULL;
#endif
    if (tiot_packet_head_check(head) != 0) {
        if (tiot_packet_head_try_recovery(ctx) == 0) {
            /* 恢复成功，继续处理. */
            return;
        }
        goto discard;
    }
    /* 消息头校验OK时，认为work. */
    tiot_packet_report_work(ctx);
    /* 系统消息不分包特殊处理，避免申请内存. */
    if (head->subsys_code == SYS_MSG) {
        ctx->frame_need_read = sizeof(uint8_t);
        ctx->pos = &ctx->sys_msg;
        ctx->current_frame = TIOT_PKT_FRAME_PAYLOAD;
        return;
    }

    tiot_cur_packet_update(ctx, head->subsys_code);
#ifdef CONFIG_BOARD_DYNAMIC_ALLOC
    if (ctx->packet != NULL) {
        node = tiot_packet_rx_node_malloc(payload_len);
        if (node == NULL) {
            ctx->frame_error = 1;
            ctx->pos = NULL;
        } else {
            tiot_list_add_tail(&(node->node), ctx->packet->head);
            ctx->pos = node->data;
        }
    } else {
        tiot_print_err("[TIoT:rx]circ queue no space.\n");
        ctx->frame_error = 1;
        ctx->pos = NULL;
    }
#else
    if ((ctx->packet == NULL) || (circ_buf_query_free(&(ctx->rx_buff.buf)) <= payload_len)) {
        tiot_print_err("[TIoT:rx]circ queue or rx buff no space.\n");
        ctx->frame_error = 1;
        ctx->pos = NULL;
    }
    if (ctx->packet != NULL) {
        ctx->packet->len += payload_len;
    }
#endif
    ctx->frame_need_read = payload_len;
    ctx->frame_cnt++;
    ctx->current_frame = TIOT_PKT_FRAME_PAYLOAD;
    return;

discard:
    tiot_cur_packet_discard(ctx);
    ctx->current_frame = TIOT_PKT_FRAME_IDLE;
}

static void tiot_packet_payload_done(tiot_packet_context *ctx)
{
    ctx->current_frame = TIOT_PKT_FRAME_TAIL;
    ctx->frame_need_read = TIOT_PKT_TAIL_SIZE;
    ctx->pos = &ctx->tail;
}

static void tiot_packet_tail_done(tiot_packet_context *ctx)
{
    uint8_t tail = ctx->tail;
    uint8_t subsys_code;
    if (tail != TIOT_PKT_TAIL) {
        tiot_cur_packet_discard(ctx);
        ctx->current_frame = TIOT_PKT_FRAME_IDLE;
        return;
    }

    /* 在head done内已经确保subsys_code正确 */
    subsys_code = ctx->head.subsys_code;
    if (g_tiot_pkt_msg_handle[subsys_code] != NULL) {
        g_tiot_pkt_msg_handle[subsys_code](ctx);
    }
    ctx->current_frame = TIOT_PKT_FRAME_IDLE;
}

uint32_t tiot_packet_handle(tiot_packet_context *ctx, uint8_t *data, uint32_t data_len)
{
    uint16_t packet_len = (uint16_t)(data_len & 0xFFFF);  /* 取低16位，包格式只需要16位大小. */
    uint32_t handle_len = packet_len < ctx->frame_need_read ? packet_len : ctx->frame_need_read;
    ctx->frame_need_read -= handle_len;

#ifdef CONFIG_BOARD_DYNAMIC_ALLOC
    if (ctx->pos != NULL) {
        (void)memcpy_s(ctx->pos, handle_len, data, handle_len);
        ctx->pos += handle_len;
    } else {
        tiot_print_dbg("[TIoT][rx]packet buff pos is NULL.\n");
    }
#else
    /* 系统消息直接拷贝；buffer放不下时丢包处理，使用frame_error标识. */
    if ((ctx->current_frame == TIOT_PKT_FRAME_PAYLOAD) &&
        (ctx->head.subsys_code != SYS_MSG) &&
        (ctx->frame_error == 0)) {
        circ_buf_write_data(&(ctx->rx_buff.buf), data, handle_len, ctx->rx_buff.buff_write);
        circ_buf_update_write_pos(&(ctx->rx_buff.buf), handle_len);
    } else {
        if (ctx->pos != NULL) {
            (void)memcpy_s(ctx->pos, handle_len, data, handle_len);
            ctx->pos += handle_len;
        } else {
            tiot_print_dbg("[TIoT][rx]packet buff pos is NULL.\n");
        }
    }
#endif

    if (ctx->frame_need_read != 0) {
        return handle_len;
    }

    /* 完成一帧数据 */
    switch (ctx->current_frame) {
        case TIOT_PKT_FRAME_HEAD:
            tiot_packet_head_done(ctx);
            break;
        case TIOT_PKT_FRAME_PAYLOAD:
            tiot_packet_payload_done(ctx);
            break;
        case TIOT_PKT_FRAME_TAIL:
            tiot_packet_tail_done(ctx);
            break;
        default:
            tiot_print_dbg("[TIoT][rx]err frame: %d\n", ctx->current_frame);
            break;
    }
    return handle_len;
}

#ifdef CONFIG_BOARD_DYNAMIC_ALLOC
static inline void tiot_packet_ascii_line_store(tiot_packet_context *ctx, uint8_t *line_start, uint32_t line_len)
{
    tiot_packet_rx_node *node = tiot_packet_rx_node_malloc(line_len);
    if (node == NULL) {
        /* discard */
        return;
    }
    (void)memcpy_s(node->data, line_len, line_start, line_len);
    tiot_list_add_tail(&(node->node), ctx->packet->head);
}
#else
static inline void tiot_packet_ascii_line_store(tiot_packet_context *ctx, uint8_t *line_start, uint32_t line_len)
{
    if (circ_buf_query_free(&(ctx->rx_buff.buf)) <= line_len) {
        tiot_print_err("[TIoT][rx]rx buffer no space for data.");
        return;
    }
    circ_buf_write(&(ctx->rx_buff.buf), line_start, line_len);
    ctx->packet->len += line_len;
}
#endif

static void tiot_packet_ascii_line_handle(tiot_packet_context *ctx, uint8_t *data, uint32_t handle_len)
{
    uint8_t *left = data;
    uint8_t *right = data;
    uint32_t line_len = 0;
    while (right < (data + handle_len)) {
        if (*right != '\n') {
            right++;
            continue;
        }
        /* 找到行尾. */
        line_len = right - left + 1; /* 当前字符也复制 '\n'. */
        tiot_cur_packet_update(ctx, MSG_BUTT); /* ASCII消息使用MSG_BUTT，避免重复. */
        if (ctx->packet != NULL) {
            tiot_packet_ascii_line_store(ctx, left, line_len);
            ctx->packet->tag = TIOT_PKT_TAG_ASCII;
            tiot_cur_packet_to_queue(ctx);
            tiot_packet_cums_recv_handle(ctx);
        } else {
            tiot_print_err("[TIoT:rx]circ queue no space, discard: 0x%x\r\n", line_len);
        }
        left = right + 1;  /* 下一行起始 */
        right = left;
    }
    /* 最后一段没有结尾的情况 */
    line_len = (data + handle_len) - left;
    if (line_len == 0) {
        return;
    }
    tiot_cur_packet_update(ctx, MSG_BUTT); /* ASCII消息使用MSG_BUTT，避免重复. */
    if (ctx->packet != NULL) {
        tiot_packet_ascii_line_store(ctx, left, line_len);
        /* ASCII字符串结束判断. */
        if (ctx->current_frame == TIOT_PKT_FRAME_HEAD) {
            ctx->packet->tag = TIOT_PKT_TAG_ASCII;
            tiot_cur_packet_to_queue(ctx);
            tiot_packet_cums_recv_handle(ctx);
        }
    } else {
        tiot_print_err("[TIoT:rx]circ queue no space, discard: 0x%x\r\n", line_len);
    }
}

uint32_t tiot_packet_ascii_handle(tiot_packet_context *ctx, uint8_t *data, uint32_t data_len)
{
    /* ascii字符串处理 */
    uint32_t handle_len = 0;
    uint8_t *tmp = data;
    /* 寻找包头起始 */
    while (handle_len < data_len) {
        if (*tmp == 0x7e) {
            break;
        }
        tmp++;
        handle_len++;
    }

    if (handle_len < data_len) {
        /* 找到包头 */
        ctx->current_frame = TIOT_PKT_FRAME_HEAD;
        ctx->frame_need_read = TIOT_PKT_HEAD_SIZE;
        ctx->pos = (uint8_t *)&ctx->head;
    }

    if ((handle_len == 0) || !(ctx->flags & TIOT_PKT_FLAGS_SUPPORT_ASCII)) {
        /* 1. 第一个字节即0x7e, 不需要处理. */
        /* 2. 不支持上报ascii数据，直接丢包处理. */
        goto discard;
    }
    /* 支持上报ascii数据时，有数据认为work. */
    tiot_packet_report_work(ctx);
    tiot_packet_ascii_line_handle(ctx, data, handle_len);
discard:
    return handle_len;
}

void tiot_packet_rx_data_parse(tiot_packet_context *ctx, uint8_t *data, uint32_t len)
{
    uint8_t *buff = data;
    uint32_t handle_len = 0;
    uint32_t total_len = len;

    while (total_len) {
        if (ctx->current_frame == TIOT_PKT_FRAME_IDLE) {
            handle_len = tiot_packet_ascii_handle(ctx, buff, total_len);
        } else {
            handle_len = tiot_packet_handle(ctx, buff, total_len);
        }
        buff += handle_len;
        total_len -= handle_len;
    }
}

void tiot_packet_rx_data_store(tiot_xfer_manager *xfer, uint8_t *data, uint32_t len)
{
    tiot_packet_context *ctx = NULL;
    /* 入参已在调用处判空 */
    ctx = (tiot_packet_context *)xfer->pkt_context;
    tiot_packet_rx_data_parse(ctx, data, len);
}

static int tiot_packet_read_out_check(const void *param)
{
    int check;
    tiot_packet_manager_query *query = (tiot_packet_manager_query *)param;
    tiot_xfer_manager *xfer = query->xfer;
    tiot_packet_context *ctx = (tiot_packet_context *)xfer->pkt_context;
    if (ctx == NULL) {
        return 0;
    }
    check = (xfer->rx_wait_func(xfer) != 0) ||
            (tiot_packet_manager_has_data(&ctx->rx_manager, query->queue_id));
    return check;
}

#ifdef CONFIG_BOARD_DYNAMIC_ALLOC
static int32_t tiot_packet_is_valid(tiot_packet *packet)
{
    uint16_t frame_cnt = 0;
    tiot_packet_rx_node *pos;
    if (packet->head == NULL) {
        return -1;
    }
    tiot_list_for_each_entry(pos, packet->head, node, tiot_packet_rx_node) {
        if ((pos->data == NULL) || (pos->len == 0)) {
            return -1;
        }
        frame_cnt++;
    }
    if (frame_cnt == 0) {
        return -1;
    }
    return 0;
}

static int32_t tiot_packet_copy_from_queue(tiot_packet_context *ctx, tiot_packet_queue *queue,
                                           uint8_t *buff, uint32_t buff_len)
{
    tiot_packet *packet = NULL;
    uint8_t *tmp_buff = buff;
    uint32_t copy_len;
    tiot_packet_rx_node *pos;

    do {
        copy_len = 0;
        packet = (tiot_packet *)tiot_circ_queue_dequeue(&queue->queue);
        if (packet == NULL) {
            tiot_print_info("[TIoT][rx]packet queue is empty.\n");
            break;
        }
        if (tiot_packet_is_valid(packet) != 0) {
            tiot_print_err("[TIoT][rx]invalid packet.");
            tiot_packet_free_packet(packet);
            continue;
        }

        tiot_list_for_each_entry(pos, packet->head, node, tiot_packet_rx_node) {
            copy_len += pos->len;
        }
        if (ctx->flags & TIOT_PKT_FLAGS_SUPPORT_ASCII) {
            copy_len += sizeof(uint16_t);
        }
        if (copy_len > buff_len) {
            tiot_print_info("[TIoT][rx]not enough buff len:%d for this packet: %d.", buff_len, copy_len);
            tiot_packet_free_packet(packet);
            continue;
        }
        /* 完整包正常, 复制一包到用户buffer后退出. */
        if (ctx->flags & TIOT_PKT_FLAGS_SUPPORT_ASCII) {
            (void)memcpy_s((void *)tmp_buff, sizeof(uint16_t), (void *)&packet->tag, sizeof(uint16_t));
            tmp_buff += sizeof(uint16_t);
        }
        tiot_list_for_each_entry(pos, packet->head, node, tiot_packet_rx_node) {
            (void)memcpy_s((void *)tmp_buff, pos->len, (void *)pos->data, pos->len);
            tmp_buff += pos->len;
        }
        tiot_packet_free_packet(packet);
        break;
    } while (packet != NULL);

    return copy_len;
}
#else
static int32_t tiot_packet_copy_from_queue(tiot_packet_context *ctx, tiot_packet_queue *queue,
                                           uint8_t *buff, uint32_t buff_len)
{
    uint32_t copy_len;
    uint8_t *tmp_buff = buff;
    tiot_packet *packet;
    do {
        copy_len = 0;
        packet = (tiot_packet *)tiot_circ_queue_dequeue(&queue->queue);
        if (packet == NULL) {
            tiot_print_info("[TIoT][rx]packet queue is empty.");
            break;
        }
        copy_len = packet->len;
        if (ctx->flags & TIOT_PKT_FLAGS_SUPPORT_ASCII) {
            copy_len += sizeof(uint16_t);
        }
        if (copy_len > buff_len) {
            tiot_print_info("[TIoT][rx]not enough buff len:%d for this packet: %d.", buff_len, copy_len);
            (void)circ_buf_update_read_pos(&(ctx->rx_buff.buf), packet->len);
            continue;
        }
        /* 完整包正常, 复制一包到用户buffer后退出. */
        if (ctx->flags & TIOT_PKT_FLAGS_SUPPORT_ASCII) {
            (void)memcpy_s((void *)tmp_buff, sizeof(uint16_t), (void *)&packet->tag, sizeof(uint16_t));
            tmp_buff += sizeof(uint16_t);
        }
        /* 按顺序读即可，不需要记录起始. */
        (void)circ_buf_read(&(ctx->rx_buff.buf), tmp_buff, packet->len);
        break;
    } while (packet != NULL);
    return copy_len;
}
#endif

#ifdef __KERNEL__
static int32_t packet_poll(tiot_packet_manager_query *query, tiot_packet_queue *queue, const tiot_xfer_rx_param *param)
{
    if (tiot_packet_read_out_check(query) != 0) {
        return 1;
    }

    poll_wait(param->filp, (wait_queue_head_t *)(queue->rx_wait.wait), param->poll_wait);

    if (tiot_packet_read_out_check(query) != 0) {
        return 1;
    }
    return 0;
}
#endif

int32_t tiot_packet_read_out(tiot_xfer_manager *xfer, uint8_t *buff, uint32_t len, const tiot_xfer_rx_param *param)
{
    int32_t read_ret;
    tiot_packet_queue *queue;
    tiot_packet_manager_query query;
    tiot_packet_context *ctx = (tiot_packet_context *)xfer->pkt_context;
    if (ctx == NULL) {
        return -1;
    }
    queue = tiot_packet_manager_find_queue(&ctx->rx_manager, param->subsys); /* 对应queue_id */
    if (queue == NULL) {
        return -1;
    }
    query.xfer = xfer;
    query.queue_id = param->subsys;  /* 对应queue_id. */

#ifdef __KERNEL__
    if (param->filp != NULL && param->poll_wait != NULL) { /* poll mode */
        return packet_poll(&query, queue, param);
    }
#endif

    if (param->timeout == 0) { /* 非阻塞. */
        /* 没有数据直接返回. */
        if (tiot_packet_read_out_check(&query) == 0) {
            tiot_print_dbg("[TIoT:pkt][%d]no data to read.\r\n", param->subsys);
            return 0;
        }
    } else { /* 阻塞. */
        /* 等待接收到一个完整包. */
        read_ret = osal_wait_timeout_interruptible(&queue->rx_wait, tiot_packet_read_out_check, &query, param->timeout);
        if (read_ret <= 0) {
            tiot_print_warning("[TIoT:pkt][%d]osal_wait timeout.\r\n", param->subsys);
            return read_ret;
        }
    }

    return tiot_packet_copy_from_queue(ctx, queue, buff, len);
}

#ifdef CONFIG_XFER_SUPPORT_TX_TASK
static int32_t tiot_tx_packet_fill(uint8_t *dest, uint8_t *payload, uint16_t payload_len, uint8_t subsys_code)
{
    uint16_t packet_len = payload_len + TIOT_PKT_PKG_SIZE;
    uint8_t *packet_buff = dest;

    tiot_packet_head *head = (tiot_packet_head *)packet_buff;
    head->head = TIOT_PKT_HEAD;
    head->subsys_code = subsys_code;
    head->len = packet_len;

    if (memcpy_s(&packet_buff[TIOT_PKT_HEAD_SIZE], payload_len, payload, payload_len) != EOK) {
        tiot_print_err("[TIoT][tx]packet memcpy err.\r\n");
        return -1;
    }
    packet_buff[packet_len - 1] = TIOT_PKT_TAIL;
    return 0;
}

int32_t tiot_packet_tx_push(tiot_xfer_manager *xfer, const uint8_t *data, uint32_t len,
                            const tiot_xfer_tx_param *param)
{
    int32_t ret;
    uint16_t payload_len;
    uint32_t total_len;
    const uint8_t *buff;
    uint8_t packet_count;
    const tiot_packet_subsys_info *subsys_info;
    tiot_xfer_tx_data new_data;

    /* data外部已验证 */
    if ((param == NULL) || (len == 0)) {
        tiot_print_err("[TIoT:pkt]tx param is NULL or len is 0.\r\n");
        return -1;
    }

    total_len = len;
    subsys_info = &g_tiot_pkt_subsys_info[param->subsys];
    /* 分包个数: tx_len / 4095 + (tx_len % 4095 ? 1 : 0) ==> (tx_len + (4095 - 1)) / 4095。 */
    packet_count = (total_len + TIOT_PKT_PAYLOAD_MAX_SIZE - 1) / TIOT_PKT_PAYLOAD_MAX_SIZE;
    /* 预先申请 */
    packet_buff = osal_kmalloc(total_len + packet_count * (TIOT_PKT_PKG_SIZE));
    if (packet_buff == NULL) {
        return -1;
    }
    tmp = packet_buff;

    for (i = 0; i < packet_count - 1; i++) {
        payload_len = TIOT_PKT_PAYLOAD_MAX_SIZE;
        if (i == 0) {
            ret = tiot_tx_packet_fill(tmp, buff, payload_len, subsys_info->common);
        } else {
            ret = tiot_tx_packet_fill(tmp, buff, payload_len, subsys_info->first);
        }
        if (ret != 0) {
            /* 复制数据出错，释放buffer */
            osal_kfree(packet_buff);
            return -1;
        }
        tmp += payload_len + TIOT_PKT_PKG_SIZE;
        buff += payload_len;
    }

    payload_len = total_len - (packet_count - 1) * TIOT_PKT_PAYLOAD_MAX_SIZE;
    ret = tiot_tx_packet_fill(tmp, buff, payload_len, subsys_info->last);
    if (ret != 0) {
        /* 复制数据出错，释放buffer */
        osal_kfree(packet_buff);
        return -1;
    }

    new_data.data = packet_buff;
    new_data.len = total_len + packet_count * (TIOT_PKT_PKG_SIZE);
    if (tiot_circ_queue_enqueue(&xfer->tx_queue, (uintptr_t)&new_data) != 0) {
        osal_kfree(packet_buff);
        return -1;
    }
    return (ret >= 0) ? total_len : (-1);
}
#else
static int32_t tiot_tx_packet_push(tiot_xfer_manager *xfer, const uint8_t *payload,
                                   uint16_t payload_len, uint8_t subsys_code)
{
    int32_t ret = 0;
    uint16_t packet_len = payload_len + TIOT_PKT_PKG_SIZE;
    const uint8_t tail = TIOT_PKT_TAIL;
    tiot_packet_head head = { TIOT_PKT_HEAD, subsys_code, packet_len };

    do {
        ret = tiot_xfer_direct_send(xfer, (uint8_t *)&head, sizeof(tiot_packet_head));
        if (ret != sizeof(tiot_packet_head)) {
            ret = -1;
            break;
        }
        ret = tiot_xfer_direct_send(xfer, payload, payload_len);
        if (ret != payload_len) {
            ret = -1;
            break;
        }
        ret = tiot_xfer_direct_send(xfer, &tail, sizeof(uint8_t));
        if (ret != sizeof(uint8_t)) {
            ret = -1;
            break;
        }
    } while (0);

    return (ret >= 0) ? (int32_t)packet_len : (-1);
}

int32_t tiot_packet_tx_push(tiot_xfer_manager *xfer, const uint8_t *data, uint32_t len,
                            const tiot_xfer_tx_param *param)
{
    int32_t ret;
    uint16_t payload_len;
    uint32_t total_len;
    const uint8_t *buff;
    uint8_t packet_count;
    uint8_t i;
    const tiot_packet_subsys_info *subsys_info;
    tiot_packet_context *pkt_ctx;

    /* data外部已验证 */
    if ((param == NULL) || (len == 0)) {
        tiot_print_err("[TIoT:pkt]tx param is NULL or len is 0.\r\n");
        return -1;
    }

    pkt_ctx = (tiot_packet_context*)(xfer->pkt_context);
    if (pkt_ctx == NULL) {
        tiot_print_err("[TIoT][pkt]packet context is NULL.\r\n");
        return -1;
    }

    total_len = len;
    buff = data;
    /* 分包个数: tx_len / 4095 + (tx_len % 4095 ? 1 : 0) ==> (tx_len + (4095 - 1)) / 4095。 */
    packet_count = (total_len + TIOT_PKT_PAYLOAD_MAX_SIZE - 1) / TIOT_PKT_PAYLOAD_MAX_SIZE;
    subsys_info = &g_tiot_pkt_subsys_info[param->subsys];

    /*
     * 多线程并发情况下，必须保证每个线程完整地发完整个packet，中间不能插入同类型的其他packet
     * thread1:  gnss-first           gnss-last
     * thread2:            gnss-first
     * 像如上情况就会出现first-first-last，导致解包出现错误
     */
    osal_mutex_lock(&pkt_ctx->tx_packet_mutex);

    for (i = 0; i < packet_count - 1; i++) {
        payload_len = TIOT_PKT_PAYLOAD_MAX_SIZE;
        if (i == 0) {
            ret = tiot_tx_packet_push(xfer, buff, payload_len, subsys_info->first);
        } else {
            ret = tiot_tx_packet_push(xfer, buff, payload_len, subsys_info->common);
        }

        if (ret == -1) {
            osal_mutex_unlock(&pkt_ctx->tx_packet_mutex);
            return -1;
        }
        buff += payload_len;
    }
    payload_len = total_len - (packet_count - 1) * TIOT_PKT_PAYLOAD_MAX_SIZE;
    ret = tiot_tx_packet_push(xfer, buff, payload_len, subsys_info->last);

    osal_mutex_unlock(&pkt_ctx->tx_packet_mutex);

    return (ret >= 0) ? (int32_t)total_len : (-1);
}
#endif
