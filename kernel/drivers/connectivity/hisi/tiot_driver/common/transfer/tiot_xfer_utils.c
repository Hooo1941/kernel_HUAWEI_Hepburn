/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved. \n
 *
 * Description:  \n
 * Author: @CompanyNameTag \n
 * History: \n
 * 2023-04-26, Create file. \n
 */

#include "tiot_xfer_utils.h"

int32_t tiot_xfer_direct_send(tiot_xfer_manager *xfer, const uint8_t *data, uint32_t len)
{
    int32_t ret;
    const uint8_t *buff = data;
    uint32_t remain_len = len;
    while (remain_len) {
        /* 板级接口可能有单次发送限制. */
        (void)osal_mutex_lock(&xfer->dev_mutex);
        if (xfer->xmit_state != TIOT_XMIT_STATE_OPEN) {
            ret = -1;
        } else {
            ret = xfer->xmit_ops->write(&xfer->xmit, buff, remain_len);
        }
        (void)osal_mutex_unlock(&xfer->dev_mutex);
        /* 阻塞式接口，发送结果为0情况时继续发送 */
        if (ret < 0) {
            return -1;
        }

        remain_len -= ret;
        buff += ret;
    }
    return len;
}

int32_t tiot_xfer_read_store(tiot_circ_buf *cb, uint8_t *data, uint32_t len)
{
    int32_t ret;
    if ((cb == NULL) || (circ_buf_query_free(cb) < len)) {
        return -1;
    }
    ret = circ_buf_write(cb, data, len);
    if (ret == 0) {
        return -1;
    }
    return 0;
}

static int tiot_xfer_read_out_check(const void *param)
{
    tiot_xfer_manager *xfer = (tiot_xfer_manager *)param;
    if ((xfer->rx_wait_func(xfer) != 0) ||
        ((xfer->xmit_ops->rx_mode == TIOT_XMIT_RX_MODE_BUFFED) && (circ_buf_query_busy(&xfer->rx_buff) != 0))) {
        return 1;
    }
    return 0;
}

int32_t tiot_xfer_read_out(tiot_xfer_manager *xfer, uint8_t *buff, uint32_t len, uint32_t timeout_ms)
{
    int32_t read_len;
    int32_t timeout = osal_wait_timeout_interruptible(&xfer->rx_wait, tiot_xfer_read_out_check,
                                                      (const void *)xfer, timeout_ms);
    /* 超时返回 0, 失败返回-1 */
    if (timeout <= 0) {
        return timeout;
    }
    osal_mutex_lock(&xfer->dev_mutex);

    if (xfer->xmit_ops->rx_mode == TIOT_XMIT_RX_MODE_BUFFED) {
        uint32_t buff_data_len = circ_buf_query_busy(&xfer->rx_buff);
        read_len = buff_data_len < len ? buff_data_len : len;
        read_len = circ_buf_read(&xfer->rx_buff, buff, read_len);
        if (read_len == 0) {
            read_len = -1;
        }
    } else {
        read_len = xfer->xmit_ops->read(&xfer->xmit, buff, len);
    }

    osal_mutex_unlock(&xfer->dev_mutex);
    return read_len;
}
