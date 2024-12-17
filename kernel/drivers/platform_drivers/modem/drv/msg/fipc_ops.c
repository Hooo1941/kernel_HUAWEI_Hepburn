/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2021. All rights reserved.
 * Description: msg
 * Author:
 * Created: 2021-12-10
 * Last Modified:
 * History:
 * Modification: Create file
 */

#include "fipc_ops.h"
#include "msg_lite.h"
#include "msg_mntn.h"

int msg_fipc_default_cb(fipc_event event, void *arg, const fipc_eventinfo *event_info)
{
    struct channel_ctrl_t* chn_ctrl = NULL;
    struct cmsg_ctrl_t* cmsg_ctrl = get_cmsg_ctrl();
    msg_trace("msg_fipc_default_cb in\n");

    if (event == FIPC_EVENT_DATA_ARRV) {
        chn_ctrl = (struct channel_ctrl_t*)arg;
        __pm_stay_awake(cmsg_ctrl->default_ctrl[chn_ctrl->cid]->cmsg_task.worker_wakelock);
        up(&cmsg_ctrl->default_ctrl[chn_ctrl->cid]->cmsg_task.worker_sem);
    } else {
        msg_err("fipc Something went wrong\n");
        return -1;
    }
    msg_trace("msg_fipc_default_cb out\n");
    return 0;
}

int fipc_default_open_send(struct channel_ctrl_t* chn_ctrl)
{
    int ret;
    void *addr = NULL;
    dma_addr_t dma_addr;
    fipc_chn_attr_t attr;
    fipc_chn_attr_t *pattr = &attr;

    struct fipc_cmsg_chn_t *fipc_cmsg_chn = (struct fipc_cmsg_chn_t *)(chn_ctrl->cmsg_chn);
    addr = msg_dma_buf_alloc(fipc_cmsg_chn->buf_sz, &dma_addr);
    if (addr == NULL) {
        msg_err("msg dma mem alloc error\n");
        return -1;
    }

    ret = bsp_fipc_chn_attr_init(&attr);
    if (ret) {
        msg_err("chan attr init error\n");
        return -1;
    }

    pattr->chnid = chn_ctrl->phychnid;
    pattr->type = FIPC_CHN_TYPE_SEND;
    pattr->pa = dma_addr;
    pattr->va = addr;
    pattr->size = fipc_cmsg_chn->buf_sz;
    pattr->cbk = NULL;
    pattr->cbk_arg = NULL;

    ret = bsp_fipc_chn_open(&fipc_cmsg_chn->handle, &attr);
    if (ret) {
        msg_err("bsp_fipc_chn_open %d error ret=%x\n", pattr->chnid, ret);
        msg_dma_buf_free(pattr->size, pattr->va, pattr->pa);
        return -1;
    }
    return 0;
}

int fipc_default_open_recv(struct channel_ctrl_t* chn_ctrl)
{
    int ret;
    void *addr = NULL;
    dma_addr_t dma_addr;
    fipc_chn_attr_t attr;
    fipc_chn_attr_t *pattr = &attr;

    struct fipc_cmsg_chn_t *fipc_cmsg_chn = (struct fipc_cmsg_chn_t *)(chn_ctrl->cmsg_chn);
    addr = msg_dma_buf_alloc(fipc_cmsg_chn->buf_sz, &dma_addr);
    if (addr == NULL) {
        msg_err("msg dma mem alloc error\n");
        return -1;
    }
    ret = bsp_fipc_chn_attr_init(&attr);
    if (ret) {
        msg_err("chan attr init error\n");
        return -1;
    }

    pattr->chnid = chn_ctrl->phychnid;
    pattr->type = FIPC_CHN_TYPE_RECV;
    pattr->pa = dma_addr;
    pattr->va = addr;
    pattr->size = fipc_cmsg_chn->buf_sz;
    pattr->cbk_arg = chn_ctrl;
    pattr->cbk = msg_fipc_default_cb;
    pattr->irq_wakeup_cbk = msg_irq_wakeup_set;
    pattr->irq_cbk_arg = NULL;

    ret = bsp_fipc_chn_open(&fipc_cmsg_chn->handle, &attr);
    if (ret) {
        msg_err("bsp_fipc_chn_open %d error ret=%x\n", pattr->chnid, ret);
        msg_dma_buf_free(pattr->size, pattr->va, pattr->pa);
        return -1;
    }
    return 0;
}

int fipc_default_send(struct channel_ctrl_t* chn_ctrl, msg_blkx3_desc_t *send_desc)
{
    s32 ret, try_times;
    struct fipc_cmsg_chn_t *fipc_chn = (struct fipc_cmsg_chn_t *)(chn_ctrl->cmsg_chn);
    for (try_times = 0; try_times < CMSG_SEND_TRY_WAIT_TIMES; try_times++) {
        ret = bsp_fipc_chn_blks_send(fipc_chn->handle, (fipc_blk_desc_t *)send_desc, 0);
        if (ret == FIPC_ERR_EAGAIN) {
            continue;
        }
        break;
    };
    if (ret < 0) {
        msg_err("msg send err, ret=%d\n", ret);
    }

    if(ret == FIPC_ERR_EAGAIN) {
        return MSG_ERR_EAGAIN;
    }

    return ret;
}

int fipc_default_recv(struct channel_ctrl_t *chn_ctrl, void *buf, u32 len)
{
    s32 ret;
    struct fipc_cmsg_chn_t *fipc_chn = (struct fipc_cmsg_chn_t *)(chn_ctrl->cmsg_chn);
    ret = bsp_fipc_chn_recv(fipc_chn->handle, buf, len, 0);
    if (ret != (int)(len)) {
        msg_err("fipc_default_recv too small,len=%d, ret = %d\n", len, ret);
        return MSG_ERR_EBADMSG;
    }
    return ret;
}

int fipc_nextpkt_len(struct channel_ctrl_t *chn_ctrl)
{
    s32 ret;
    ioctl_nxtpkt pktinfo;
    struct fipc_cmsg_chn_t *fipc_chn = (struct fipc_cmsg_chn_t *)(chn_ctrl->cmsg_chn);
    ret = bsp_fipc_chn_ioctl(fipc_chn->handle, FIPC_IOCTL_CHN_NXTPKT_INF, &pktinfo, sizeof(pktinfo));
    if (ret) {
        msg_err("fipc ioctrl err\n");
        return -1;
    }
    return pktinfo.len;
}

int fipc_phychn_init(struct channel_ctrl_t *channel_ctrl, device_node_s *pnode, struct cmsg_ctrl_t* cmsg_ctrl)
{
    u32 ret, dir, logic_id, phychnid, smsg_sz, buf_sz;
    struct fipc_cmsg_chn_t *fipc_cmsg_chn = NULL;
    ret = (u32)bsp_dt_property_read_u32(pnode, "dir", &dir);
    ret |= (u32)bsp_dt_property_read_u32(pnode, "chnid", &logic_id);
    ret |= (u32)bsp_dt_property_read_u32(pnode, "smsg_sz", &smsg_sz);
    ret |= (u32)bsp_dt_property_read_u32(pnode, "buf_sz", &buf_sz);
    if (ret) {
        msg_err("dts err!\n");
        return -1;
    }
    if (channel_ctrl->mode == FIPC_DEFAULT_CHN && cmsg_ctrl->default_ctrl[channel_ctrl->cid] == NULL) {
        cmsg_ctrl->default_ctrl[channel_ctrl->cid] = channel_ctrl;
    } else if (channel_ctrl->mode != FIPC_DEFAULT_CHN) {
        if (cmsg_ctrl->custom_ctrl_idx >= CUSTOM_CHANNEL_MAX) {
            msg_err("exceed custom ctrl max\n");
            return -1;
        }
        cmsg_ctrl->custom_ctrl[cmsg_ctrl->custom_ctrl_idx++] = channel_ctrl;
    }
    fipc_cmsg_chn = (struct fipc_cmsg_chn_t *)osl_malloc(sizeof(struct fipc_cmsg_chn_t));
    if (fipc_cmsg_chn == NULL) {
        return -1;
    }
    (void)memset_s(fipc_cmsg_chn, sizeof(struct fipc_cmsg_chn_t), 0, sizeof(struct fipc_cmsg_chn_t));
    phychnid = get_phy_chnid(logic_id);    /* 由逻辑通道id得到实际通道id */
    if ((s32)phychnid < 0) {
        msg_err("phyid not found\n");
        return -1;
    }
    channel_ctrl->phychnid = phychnid;
    channel_ctrl->cmsg_chn = fipc_cmsg_chn;
    channel_ctrl->smsg_sz = smsg_sz;
    fipc_cmsg_chn->buf_sz = buf_sz;
    if (MSG_PHYCHN_SEND(dir)) {
        ret = (u32)channel_ctrl->ops->s_open(channel_ctrl);
    }
    if (MSG_PHYCHN_RECV(dir)) {
        if (channel_ctrl->ops->task_init != NULL) {
            ret = channel_ctrl->ops->task_init(channel_ctrl);
            if (ret) {
                return -1;
            }
        }
        ret = (u32)channel_ctrl->ops->r_open(channel_ctrl);
    }
    return ret;
}
