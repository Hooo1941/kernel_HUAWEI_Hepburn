/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2018. All rights reserved.
 * foss@huawei.com
 *
 * If distributed as part of the Linux kernel, the following license terms
 * apply:
 *
 * * This program is free software; you can redistribute it and/or modify
 * * it under the terms of the GNU General Public License version 2 and
 * * only version 2 as published by the Free Software Foundation.
 * *
 * * This program is distributed in the hope that it will be useful,
 * * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * * GNU General Public License for more details.
 * *
 * * You should have received a copy of the GNU General Public License
 * * along with this program; if not, write to the Free Software
 * * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
 *
 * Otherwise, the following license terms apply:
 *
 * * Redistribution and use in source and binary forms, with or without
 * * modification, are permitted provided that the following conditions
 * * are met:
 * * 1) Redistributions of source code must retain the above copyright
 * *    notice, this list of conditions and the following disclaimer.
 * * 2) Redistributions in binary form must reproduce the above copyright
 * *    notice, this list of conditions and the following disclaimer in the
 * *    documentation and/or other materials provided with the distribution.
 * * 3) Neither the name of Huawei nor the names of its contributors may
 * *    be used to endorse or promote products derived from this software
 * *    without specific prior written permission.
 *
 * * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include "bsp_fipc.h"
#include "msg_plat.h"
#include "msg_base.h"
#include "msg_mem.h"
#include "msg_core.h"
#include "msg_lite.h"
#include "msg_cmsg.h"
#include "msg_reset.h"
#include "fipc_ops.h"

struct cmsg_ctrl_t g_cmsg_ctrl = { 0 };

struct channel_map g_channel_map[] = {
    /* acpu */
    {0x0, FIPC_CHN_SEND_AP2TSP_MDRV_MSG},
    {0x1, FIPC_CHN_RECV_TSP2AP_MDRV_MSG},
    /* lpmcu */
    {0x2, FIPC_CHN_SEND_AP2LPM_MDRV_MSG},
    {0x3, FIPC_CHN_RECV_LPM2AP_MDRV_MSG},
    /* hifi */
    {0x4, FIPC_CHN_SEND_AP2HIFI_MDRV_MSG},
    {0x5, FIPC_CHN_RECV_HIFI2AP_MDRV_MSG},
    /* dpa */
    {0x6, FIPC_CHN_SEND_AP2DPA_MDRV_MSG},
    {0x7, FIPC_CHN_RECV_DPA2AP_MDRV_MSG},
};

static struct channel_ops g_channel_ops[] = {
    {
        .phy_init = fipc_phychn_init,
        .s_open = fipc_default_open_send,
        .r_open = fipc_default_open_recv,
        .send = fipc_default_send,
        .recv = fipc_default_recv,
        .task_init = cmsg_default_task_init,
        .nextpkt_len = fipc_nextpkt_len
    },
    {
        .phy_init = fipc_phychn_init,
        .s_open = fipc_default_open_send,
        .r_open = fipc_default_open_recv,
        .send = fipc_default_send,
        .recv = fipc_default_recv,
        .task_init = NULL,
        .nextpkt_len = fipc_nextpkt_len
    }
};

static struct channel_device_match g_channel_match[] = {
    {
        .ops = &g_channel_ops[0x0],
        .mode = FIPC_DEFAULT_CHN,
        .device_name = "default_fipc",
    },
    {
        .ops = &g_channel_ops[0x1],
        .mode = FIPC_TRANSPARENT_CHN,
        .device_name = "bind_fipc",
    }
};

int match_channel_ops(struct channel_ctrl_t *chn_ctrl, u32 mode)
{
    u32 i;
    for (i = 0; i < sizeof(g_channel_match) / sizeof(g_channel_match[0]); i++) {
        if (g_channel_match[i].mode == mode) {
            chn_ctrl->ops = g_channel_match[i].ops;
            return 0;
        }
    }
    return -1;
}

struct cmsg_ctrl_t *get_cmsg_ctrl(void)
{
    return &g_cmsg_ctrl;
}

int get_phy_chnid(u32 logic_id)
{
    u32 i;
    for (i = 0; i < sizeof(g_channel_map) / sizeof(g_channel_map[0]); i++) {
        if (g_channel_map[i].logic_id == logic_id) {
            return g_channel_map[i].phy_id;
        }
    }
    return -1;
}

static inline struct channel_ctrl_t *cmsg_get_ctrl(unsigned cid)
{
    if (cid >= MSG_CORE_MAX) {
        return NULL;
    }
    return g_cmsg_ctrl.default_ctrl[cid];
}

void *get_custom_ctrl(u32 chnid)
{
    u32 i;
    for (i = 0; i < g_cmsg_ctrl.custom_ctrl_idx; i++) {
        if (g_cmsg_ctrl.custom_ctrl[i]->phychnid == chnid) {
            return g_cmsg_ctrl.custom_ctrl[i];
        }
    }
    return NULL;
}

static inline int forward_short_msg(void *msg, u32 send_flags)
{
    int ret;
    struct mid_unit_prv *pprv_midctrl = NULL;
    struct msg_head *hdr = (struct msg_head *)msg;

    pprv_midctrl = msg_get_private_midctrl(MDRV_MSG_HEAD_GET_SRCID(hdr));
    if (pprv_midctrl == NULL || pprv_midctrl->magic == MSG_MID_PRV_REG_MAGIC) {
        return MSG_TRACE_CROSS_BEFORE;
    }
    ret = mdrv_msg_send(msg, send_flags);
    return ret;
}

static int recv_short_msg(struct channel_ctrl_t *chn_ctrl, struct cmsg_hdr *phdr)
{
    int ret;
    s16 trace_code = MSG_TRACE_CROSS_AFTER;
    void *msg = NULL;
    struct msg_head hdr;
    struct msg_flag *msg_flag = NULL;
    u32 send_flags = __MSG_NO_TS;
    msg_dump_s info;
    /* This may be obmited */
    if (phdr->len <= MDRV_MSG_HEAD_LEN) {
        return MSG_ERR_EBADMSG;
    }

    ret = chn_ctrl->ops->recv(chn_ctrl, &hdr, sizeof(struct msg_head));
    if (ret != (int)sizeof(struct msg_head)) {
        msg_crit("recv_short_msg too small,phdr->len=%d, ret = %d\n", phdr->len, ret);
        return MSG_ERR_EBADMSG;
    }

    /* 这个检查 确保fipc里的数据长度,和msg自己认为的数据长度一致 */
    if (phdr->len != MDRV_MSG_HEAD_GET_LEN(&hdr) + sizeof(struct msg_head)) {
        msg_crit("recv_short_msg len chk fail,phdr->len=%d, msg len = %d\n", phdr->len, MDRV_MSG_HEAD_GET_LEN(&hdr));
        return MSG_ERR_EBADMSG;
    }
    msg = mdrv_msg_alloc(MDRV_MSG_HEAD_GET_SRCID(&hdr), phdr->len, MSG_FULL_LEN);
    if (msg == NULL) {
        msg_crit("recv_short_msg alloc %d failed\n", phdr->len);
        return MSG_ERR_ENOMEM;
    }

    if (memcpy_s(msg, phdr->len, &hdr, sizeof(struct msg_head))) {
        mdrv_msg_free(MDRV_MSG_HEAD_GET_SRCID(&hdr), msg);
        return MSG_ERR_ENOMEM;
    }

    ret = chn_ctrl->ops->recv(chn_ctrl, (void *)((char *)msg + sizeof(struct msg_head)), phdr->len - sizeof(struct msg_head));
    if (ret != (int)(phdr->len - sizeof(struct msg_head))) {
        mdrv_msg_free(MDRV_MSG_HEAD_GET_SRCID(&hdr), msg);
        return MSG_ERR_EIO;
    }
    msg_flag = (struct msg_flag *)&(MSG_HEAD_GET_FLAG(msg));
    if (msg_flag->urgent) {
        send_flags |= MSG_UG;
    }
    get_msg_dump_info(&info, msg);
    cross_dump_record(&info, DUMP_RECV_FLAG, MSG_TRACE_CROSS_BEFORE);
    ret = forward_short_msg(msg, send_flags);
    if (ret != 0) {
        msg_err("msg crosscore forward failed ret=%d\n", ret);
        mdrv_msg_free(MDRV_MSG_HEAD_GET_SRCID(&hdr), msg);
        trace_code = ret;
    }
    cross_dump_record(&info, DUMP_RECV_FLAG, trace_code);
    return 0;
}

static int recv_lite_msg(struct channel_ctrl_t *chn_ctrl, struct cmsg_hdr *phdr)
{
    int ret;
    s16 trace_code = MSG_TRACE_LITE_AFTER;
    void *msg = NULL;
    struct cmsg_lite_exthdr *lite_exthdr = NULL;
    struct msg_addr src_addr;
    struct msg_addr dst_addr;

    u32 alloc_len = phdr->len + sizeof(struct cmsg_lite_exthdr);
    msg = mdrv_msg_alloc(DRV_MID_MSG, alloc_len, MSG_FULL_LEN);
    if (msg == NULL) {
        msg_crit("recv_lite_msg alloc %d failed\n", phdr->len);
        return MSG_ERR_ENOMEM;
    }
    ret = chn_ctrl->ops->recv(chn_ctrl, msg, phdr->len + sizeof(struct cmsg_lite_exthdr));
    if (ret != (int)(phdr->len + sizeof(struct cmsg_lite_exthdr))) {
        mdrv_msg_free(DRV_MID_MSG, msg);
        msg_crit("recv_lite_msg recv err len=%d ret=%d\n", phdr->len, ret);
        return MSG_ERR_EIO;
    }
    lite_exthdr = (struct cmsg_lite_exthdr *)msg;

    src_addr.core = (lite_exthdr->src >> 0x10);
    src_addr.chnid = (lite_exthdr->src & 0xFFFF);
    dst_addr.core = (lite_exthdr->dst >> 0x10);
    dst_addr.chnid = (lite_exthdr->dst & 0xFFFF);
    lite_dump_record(&src_addr, &dst_addr, phdr->len, get_lite_dump_pos(dst_addr.chnid), MSG_TRACE_LITE_BEFORE);
    ret = MSG_TRACE_LITE_BEFORE;
    if (chn_ctrl->cid == src_addr.core) {
        ret = msg_lite_callback(&src_addr, &dst_addr, (void *)(lite_exthdr + 1), phdr->len);
    }
    if (ret != 0) {
        trace_code = ret;
        msg_err("msg fw is incomplete 0x%x->0x%x ret=%d\n", (unsigned)lite_exthdr->src, (unsigned)lite_exthdr->dst, ret);
    }
    lite_dump_record(&src_addr, &dst_addr, phdr->len, get_lite_dump_pos(dst_addr.chnid), trace_code);
    mdrv_msg_free(DRV_MID_MSG, msg);
    return 0;
}

static int _msg_recv_process(struct channel_ctrl_t *chn_ctrl, u32 pktlen)
{
    int ret;
    struct cmsg_hdr hdr;
    struct cmsg_hdr *phdr = &hdr;
    u32 hdr_len;

    ret = chn_ctrl->ops->recv(chn_ctrl, phdr, sizeof(struct cmsg_hdr));
    if (ret != (int)sizeof(struct cmsg_hdr)) {
        msg_err("msg_recv_process unexpected\n");
        /* skip this packet */
        return MSG_ERR_EBADMSG;
    }

    if (phdr->type == CMSG_TYPE_LITE) {
        hdr_len = sizeof(struct cmsg_hdr) + sizeof(struct cmsg_lite_exthdr);
    } else {
        hdr_len = sizeof(struct cmsg_hdr);
    }

    /* 重要：这个检查成立，就可以认为phdr->len是可信的，这样后面不用再检查phdr->len溢出等 */
    if (pktlen != phdr->len + hdr_len) {
        msg_crit("msg_recv_process len check failed\n");
        return MSG_ERR_EBADMSG;
    }
    if (phdr->type == CMSG_TYPE_SINGLE) {
        ret = recv_short_msg(chn_ctrl, phdr);
        if (ret) {
            msg_print("recv short msg err, ret=%d\n", ret);
            return -1;
        }
    } else if (phdr->type == CMSG_TYPE_LITE) {
        ret = recv_lite_msg(chn_ctrl, phdr);
        if (ret) {
            msg_print("recv short msg err, ret=%d\n", ret);
            return -1;
        }
    } else {
        msg_print("recv long msg err\n");
        return -1;
    }
    return 0;
}

int msg_recv_process(struct channel_ctrl_t *chn_ctrl)
{
    int ret;
    ret = chn_ctrl->ops->nextpkt_len(chn_ctrl);
    if (ret == 0) {
        /* 这属于正常情况,不要打印 */
        return MSG_ERR_EAGAIN;
    }
    if (ret < (int)sizeof(struct cmsg_hdr)) {
        msg_crit("msg_recv_process unexpected\n");
        /* skip this packet */
        return -1;
    }

    return _msg_recv_process(chn_ctrl, (u32)ret);
}

static int msg_recv_proc(void *arg)
{
    int ret;
    struct channel_ctrl_t *ctrl = (struct channel_ctrl_t *)arg;
    struct channel_ctrl_t *task_ctrl = g_cmsg_ctrl.default_ctrl[ctrl->cid];
    while (!kthread_should_stop()) {
        down(&task_ctrl->cmsg_task.worker_sem);
        do {
            ret = msg_recv_process(ctrl);
        } while (ret == 0);
        __pm_relax(task_ctrl->cmsg_task.worker_wakelock);
    };
    return 0;
}

static int send_short_msg(struct channel_ctrl_t *chn_ctrl, void *msg, unsigned msg_len)
{
    int ret;
    struct cmsg_hdr hdr;
    struct cmsg_hdr *phdr = &hdr;

    msg_blkx3_desc_t send_desc;

    phdr->type = CMSG_TYPE_SINGLE;
    phdr->rsv = 0;
    phdr->flags = 0;
    phdr->len = (u16)msg_len;

    send_desc.cnt = 0x2;
    send_desc.datablk[0].len = sizeof(hdr);
    send_desc.datablk[0].buf = phdr;
    send_desc.datablk[1].len = msg_len;
    send_desc.datablk[1].buf = msg;

    ret = chn_ctrl->ops->send(chn_ctrl, &send_desc);
    if ((unsigned)ret != sizeof(hdr) + msg_len) {
        return ret;
    }
    return 0;
}

static int send_lite_msg(struct channel_ctrl_t *chn_ctrl, const struct msg_addr *src_addr,
    const struct msg_addr *dst_addr, void *buf, u32 len)
{
    int ret;
    struct cmsg_hdr hdr;
    struct cmsg_lite_exthdr lite_exthdr;
    msg_blkx3_desc_t send_desc;

    hdr.type = CMSG_TYPE_LITE;
    hdr.rsv = 0;
    hdr.flags = 0;
    hdr.len = (u16)len;

    lite_exthdr.src = (src_addr->core << 0x10) | (src_addr->chnid);
    lite_exthdr.dst = (dst_addr->core << 0x10) | (dst_addr->chnid);

    send_desc.cnt = 0x3;
    send_desc.datablk[0x0].len = sizeof(hdr);
    send_desc.datablk[0x0].buf = &hdr;
    send_desc.datablk[0x1].len = sizeof(lite_exthdr);
    send_desc.datablk[0x1].buf = &lite_exthdr;
    send_desc.datablk[0x2].len = len;
    send_desc.datablk[0x2].buf = buf;

    ret = chn_ctrl->ops->send(chn_ctrl, &send_desc);
    if (ret < 0) {
        msg_err("msg send msg_len err,real send msg_len=%d, user send msg_len=%ld\n", ret, sizeof(struct cmsg_hdr) + len);
        return ret;
    }
    return 0;
}

/* 协议栈用默认使用fipc,不做切换 */
int msg_crosscore_send(unsigned cid, void *msg)
{
    int ret;
    unsigned msg_len, cmsg_len;
    unsigned long lockflags;
    struct channel_ctrl_t *chn_ctrl = NULL;
    chn_ctrl = cmsg_get_ctrl(cid);
    if (chn_ctrl == NULL || chn_ctrl->cmsg_task.status != MSG_CHN_STATUS_INITED) {
        return -1;
    }

    if (msg_host_is_resetting(cid)) {
        return MSG_ERR_EHOSTRST;
    }

    msg_len = MDRV_MSG_HEAD_LEN + MDRV_MSG_HEAD_GET_LEN(msg);
    cmsg_len = sizeof(struct cmsg_hdr) + msg_len;
    spin_lock_irqsave(&(chn_ctrl->s_lock), lockflags);
    if (cmsg_len <= chn_ctrl->smsg_sz) {
        ret = send_short_msg(chn_ctrl, msg, msg_len);
    } else {
        ret = -1;
    }
    spin_unlock_irqrestore(&(chn_ctrl->s_lock), lockflags);
    return ret;
}

int msg_crosscore_send_lite(const struct msg_addr *src_addr, const struct msg_addr *dst_addr, void *buf, u32 len)
{
    int ret;
    unsigned long lockflags;
    struct channel_ctrl_t *chn_ctrl = NULL;
    chn_ctrl = cmsg_get_ctrl(dst_addr->core);
    if (chn_ctrl == NULL || chn_ctrl->cmsg_task.status != MSG_CHN_STATUS_INITED) {
        return -1;
    }

    if (msg_host_is_resetting(dst_addr->core)) {
        return MSG_ERR_EHOSTRST;
    }

    spin_lock_irqsave(&(chn_ctrl->s_lock), lockflags);
    ret = send_lite_msg(chn_ctrl, src_addr, dst_addr, buf, len);
    spin_unlock_irqrestore(&(chn_ctrl->s_lock), lockflags);
    return ret;
}

int cmsg_default_task_init(struct channel_ctrl_t *channel_ctrl)
{
    struct sched_param param = {
        .sched_priority = 0x60
    };
    struct channel_ctrl_t *task_ctrl = g_cmsg_ctrl.default_ctrl[channel_ctrl->cid];
    if (task_ctrl == NULL || task_ctrl->cmsg_task.status != MSG_CHN_STATUS_UNINIT) {
        return -1;
    }
    task_ctrl->cmsg_task.status = MSG_CHN_STATUS_INITING;
    spin_lock_init(&(task_ctrl->s_lock));
    sema_init(&task_ctrl->cmsg_task.worker_sem, 0);
    task_ctrl->cmsg_task.worker_wakelock = wakeup_source_register(NULL, "msg_wl");
    task_ctrl->cmsg_task.worker = kthread_run(msg_recv_proc, channel_ctrl, "msg%d", channel_ctrl->cid);
    if (IS_ERR(task_ctrl->cmsg_task.worker)) {
        msg_err("kthread_run fail\n");
        return MSG_ERR_ESRCH;
    }
    sched_setscheduler(task_ctrl->cmsg_task.worker, SCHED_FIFO, &param);
    task_ctrl->cmsg_task.status = MSG_CHN_STATUS_INITED;
    return 0;
}

int cmsg_phychn_init(device_node_s *pnode, u32 cpuid)
{
    s32 ret;
    u32 mode;
    struct channel_ctrl_t *channel_ctrl = NULL;
    if (pnode == NULL) {
        return -1;
    }
    ret = bsp_dt_property_read_u32(pnode, "mode", &mode);
    if (ret) {
        return 0; /* not a phychn DTS */
    }
    channel_ctrl = (struct channel_ctrl_t *)osl_malloc(sizeof(struct channel_ctrl_t));
    if (channel_ctrl == NULL) {
        return -1;
    }
    (void)memset_s(channel_ctrl, sizeof(struct channel_ctrl_t), 0, sizeof(struct channel_ctrl_t));
    channel_ctrl->cid = cpuid;
    channel_ctrl->mode = mode;
    ret = match_channel_ops(channel_ctrl, mode);
    if (ret) {
        return -1;
    }
    ret = channel_ctrl->ops->phy_init(channel_ctrl, pnode, &g_cmsg_ctrl);
    return ret;
}

int msgid_sendchn_init(device_node_s *pnode, struct msg_chn_hdl *msg_hdl)
{
    s32 ret;
    u32 send_chnid;
    struct channel_ctrl_t *channel_ctrl = NULL;
    ret = bsp_dt_property_read_u32(pnode, "send_chnid", &send_chnid);
    if (ret) {
        return -1;
    }
    ret = get_phy_chnid(send_chnid); /* 由逻辑发送通道id得到实际发送通道id */
    if (ret < 0) {
        return -1;
    }
    channel_ctrl = get_custom_ctrl(ret);
    if (channel_ctrl == NULL) {
        return -1;
    }
    msg_hdl->cmsg_hdl = channel_ctrl;
    return 0;
}

int msgid_recvchn_init(device_node_s *pnode, struct msg_chn_hdl *msg_hdl)
{
    s32 ret;
    u32 recv_chnid;
    struct channel_ctrl_t *channel_ctrl = NULL;
    ret = bsp_dt_property_read_u32(pnode, "recv_chnid", &recv_chnid);
    if (ret) {
        return 0; /* 不包含recv_chnid的节点 */
    }
    ret = get_phy_chnid(recv_chnid); /* 由逻辑发送通道id得到实际发送通道id */
    if (ret < 0) {
        return -1;
    }
    channel_ctrl = get_custom_ctrl(ret);
    if (channel_ctrl == NULL) {
        return -1;
    }
    if (channel_ctrl->mode == FIPC_MEGA_CHN) {
        channel_ctrl->msg_hdl = msg_hdl; /* MEGA通道channel和msg_hdl一对一绑定 */
    }
    return 0;
}

int cmsg_msgid_init(device_node_s *pnode)
{
    s32 ret;
    u32 msgid;
    struct msg_chn_hdl *msg_hdl = NULL;
    ret = bsp_dt_property_read_u32(pnode, "msgid", &msgid);
    if (ret) {
        return 0; /* not a msgid DTS */
    }
    msg_hdl = get_msgchn_hdl(msgid);
    if (msg_hdl == NULL) {
        return -1;
    }
    ret = msgid_sendchn_init(pnode, msg_hdl);
    if (ret) {
        return -1;
    }
    ret = msgid_recvchn_init(pnode, msg_hdl);
    if (ret) {
        return -1;
    }
    return 0;
}

int cmsg_cpugroup_init(device_node_s *pnode)
{
    s32 ret;
    u32 cpuid;
    device_node_s child_node;
    device_node_s *local_node = &child_node;
    ret = bsp_dt_property_read_u32(pnode, "cpuid", &cpuid);
    if (ret) {
        msg_err("cpuid error %d!\n", cpuid);
        return -1;
    }

    if (cpuid >= MSG_CORE_MAX) {
        return -1;
    }

    bsp_dt_for_each_child_of_node(pnode, local_node)
    {
        ret = cmsg_phychn_init(local_node, cpuid);
        if (ret) {
            msg_err("cmsg_phychn_init error!\n");
            return -1;
        }
    }
    bsp_dt_for_each_child_of_node(pnode, local_node)
    {
        ret = cmsg_msgid_init(local_node);
        if (ret) {
            msg_err("cmsg_msgid_init error!\n");
            return -1;
        }
    }
    return 0;
}


static int cmsg_dummy_probe(struct platform_device *pdev)
{
    of_dma_configure(&pdev->dev, NULL, true);
    dma_set_mask_and_coherent(&pdev->dev, ~(0x1FULL)); // 32 bytes align
    return 0;
}

int cmsg_resume_wrapper(struct device *dev)
{
    msg_dump_resume();
    return 0;
}

static const struct dev_pm_ops g_cmsg_pmops = {
    .resume_noirq = cmsg_resume_wrapper,
};

static struct platform_device g_cmsg_plat_device = {
    .name = "cmsg_plat",
};

static struct platform_driver g_cmsg_plat_driver = {
    .probe = cmsg_dummy_probe,
    .driver =
        {
            .name = "cmsg_plat",
            .owner = THIS_MODULE,
            .pm = &g_cmsg_pmops,
        },
};

void *msg_dma_buf_alloc(size_t size, dma_addr_t *dma_handle)
{
    void *mem_addr = NULL;
    mem_addr = dma_alloc_coherent(&g_cmsg_plat_device.dev, size, dma_handle, GFP_KERNEL);
    if (mem_addr == NULL) {
        return NULL;
    }
    return mem_addr;
}

void msg_dma_buf_free(size_t size, void *cpu_addr, dma_addr_t dma_handle)
{
    dma_free_coherent(&g_cmsg_plat_device.dev, size, cpu_addr, dma_handle);
    return;
}

int msg_crosscore_init(void)
{
    s32 ret;
    device_node_s *ptr_device_node = NULL;
    device_node_s child_node;
    device_node_s *local_node = &child_node;
    const char *node_path = "/msg_event";

    ptr_device_node = bsp_dt_find_node_by_path(node_path);
    if (ptr_device_node == NULL) {
        return -1;
    }

    ret = platform_device_register(&g_cmsg_plat_device);
    if (ret) {
        return ret;
    }
    ret = platform_driver_register(&g_cmsg_plat_driver);
    if (ret) {
        platform_device_unregister(&g_cmsg_plat_device);
        return ret;
    }

    bsp_dt_for_each_child_of_node(ptr_device_node, local_node)
    {
        ret = cmsg_cpugroup_init(local_node);
        if (ret) {
            msg_err("cmsg_cpugroup_init error!\n");
            return -1;
        }
    }
    return 0;
}
