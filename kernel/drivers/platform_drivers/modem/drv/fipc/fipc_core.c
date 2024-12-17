/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2020. All rights reserved.
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
#include "fipc_platform.h"
#include "fipc_dts.h"

#include "fipc_device.h"
#include "fipc_driver.h"
#include "fipc_core.h"

void fipc_opipe_enable(struct fipc_device *pdev, struct fipc_channel *pchannel)
{
    u32 reg_offset = pdev->config_diff->core_pipe_irq_off;
    fipc_opipe_startup(pdev->base_va, pchannel->opipe_cfg);
    fipc_opipe_devint_en(pdev->base_va, FIPC_GET_PIPE_ID(pchannel->ldrvchn_id), pchannel->opipe_cfg->int_level);
    fipc_opipe_coreint_en(pdev->base_va, FIPC_GET_PIPE_ID(pchannel->ldrvchn_id),
        FIPC_GET_CORE_ID(pchannel->ldrvchn_id), reg_offset);
    fipc_opipe_local_en(pdev->base_va, FIPC_GET_PIPE_ID(pchannel->ldrvchn_id));
}

static int fipc_opipe_open(struct fipc_device *pdev, struct fipc_channel *pchannel, struct compat_attr *pcompat_attr)
{
    if (pchannel->opipe_cfg == NULL) {
        pchannel->opipe_cfg = fipc_malloc(sizeof(struct opipe_config));
        if (pchannel->opipe_cfg == NULL) {
            return -1;
        }
    }
    (void)memset_s(pchannel->opipe_cfg, sizeof(struct opipe_config), 0, sizeof(struct opipe_config));

    pchannel->opipe_cfg->id = FIPC_GET_PIPE_ID(pchannel->ldrvchn_id);
    pchannel->opipe_cfg->ringbuf_va = pcompat_attr->va;
    pchannel->opipe_cfg->ringbuf_pa = pcompat_attr->pa;
    pchannel->opipe_cfg->depth = pcompat_attr->depth;
    pchannel->opipe_cfg->ram_size = pcompat_attr->buf_sz;

    pchannel->opipe_cfg->ptr_vaddr = &pdev->push_ptr_va[FIPC_GET_PIPE_ID(pchannel->ldrvchn_id)];
    *pchannel->opipe_cfg->ptr_vaddr = pchannel->opipe_cfg->rptr;
    pchannel->opipe_cfg->ptr_paddr = pdev->push_ptr_pa + sizeof(u32) * FIPC_GET_PIPE_ID(pchannel->ldrvchn_id);

    if (isMsgSendChn(pchannel)) {
        pchannel->opipe_cfg->opipe_uctrl.union_stru.priorv2 = pdev->config_diff->opipe_ipc_prior;
        pchannel->opipe_cfg->opipe_uctrl.union_stru.priorv3 = pdev->config_diff->opipe_ipc_prior;
        pchannel->opipe_cfg->opipe_uctrl.union_stru.type = 0;
    } else {
        pchannel->opipe_cfg->opipe_uctrl.union_stru.priorv2 = pdev->config_diff->opipe_dma_prior;
        pchannel->opipe_cfg->opipe_uctrl.union_stru.priorv3 = pdev->config_diff->opipe_dma_prior;
        pchannel->opipe_cfg->opipe_uctrl.union_stru.type = 1;
    }
    /* wakeup中断的几个条件 1，remote en为0 2，opipe不空且(达到上水线或者达到超时) */
    pchannel->opipe_cfg->opipe_uctrl.union_stru.up_thrh = 0;
    pchannel->opipe_cfg->opipe_uctrl.union_stru.dn_thrh = 0;
    pchannel->opipe_cfg->int_level = FIPC_INT_WR_PIPE_ERR;

    /* enable opipe send done interrupt */
    if(pchannel->dym_flags & FIPC_DYM_SF_SENDDONEEN) {
        pchannel->opipe_cfg->int_level |= FIPC_INT_WR_DATA_DONE;
    }

    /* handle requests for peri device transfer */
    if(pchannel->dym_flags & FIPC_DYM_SF_PERITRANS) {
        pchannel->opipe_cfg->opipe_uctrl.union_stru.peri_en = 1;
        pchannel->opipe_cfg->opipe_uctrl.union_stru.req_sel = (pcompat_attr->req_sel & 0xF);
    }

    fipc_opipe_enable(pdev, pchannel);
    return 0;
}

void fipc_ipipe_enable(struct fipc_device *pdev, struct fipc_channel *pchannel)
{
    u32 reg_offset = pdev->config_diff->core_pipe_irq_off;
    fipc_ipipe_startup(pdev->base_va, pchannel->ipipe_cfg);
    fipc_ipipe_devint_en(pdev->base_va, FIPC_GET_PIPE_ID(pchannel->ldrvchn_id), pchannel->ipipe_cfg->int_level);
    fipc_ipipe_coreint_en(pdev->base_va, FIPC_GET_PIPE_ID(pchannel->ldrvchn_id),
        FIPC_GET_CORE_ID(pchannel->ldrvchn_id), reg_offset);

    fipc_ipipe_local_en(pdev->base_va, FIPC_GET_PIPE_ID(pchannel->ldrvchn_id));

    if (pchannel->const_flags & FIPC_CHN_SF_REMOTE_READY) {
        pchannel->remote_awake = 1;
    }

    if (pchannel->type == FIPC_CHN_TYPE_MSG_OUTRECV && pchannel->remote_awake) {
        fipc_ipipe_remote_en(pdev->base_va, FIPC_GET_PIPE_ID(pchannel->ldrvchn_id));
        pchannel->remote_awake = 0;
    }
    if (pchannel->type == FIPC_CHN_TYPE_MSG_INRRECV) {
        fipc_ipipe_devint_en(pdev->base_va, FIPC_GET_PIPE_ID(pchannel->ldrvchn_id), FIPC_INT_RD_PIPE_WAKEUP);
    }
}

static int fipc_ipipe_open(struct fipc_device *pdev, struct fipc_channel *pchannel, struct compat_attr *pcompat_attr)
{
    struct fipc_device *p_relev_dev = NULL;

    if (pchannel->ipipe_cfg == NULL) {
        pchannel->ipipe_cfg = fipc_malloc(sizeof(struct ipipe_config));
        if (pchannel->ipipe_cfg == NULL) {
            return -1;
        }
    }

    (void)memset_s(pchannel->ipipe_cfg, sizeof(struct ipipe_config), 0, sizeof(struct ipipe_config));

    pchannel->ipipe_cfg->id = FIPC_GET_PIPE_ID(pchannel->ldrvchn_id);
    pchannel->ipipe_cfg->ringbuf_va = pcompat_attr->va;
    pchannel->ipipe_cfg->ringbuf_pa = pcompat_attr->pa;
    pchannel->ipipe_cfg->depth = pcompat_attr->depth;
    pchannel->ipipe_cfg->ram_size = pcompat_attr->buf_sz;

    pchannel->ipipe_cfg->ptr_vaddr = &pdev->push_ptr_va[FIPC_GET_PIPE_ID(pchannel->ldrvchn_id)];
    *pchannel->ipipe_cfg->ptr_vaddr = pchannel->ipipe_cfg->wptr;
    pchannel->ipipe_cfg->ptr_paddr = pdev->push_ptr_pa + sizeof(u32) * FIPC_GET_PIPE_ID(pchannel->ldrvchn_id);

    pchannel->ipipe_cfg->ipipe_uctrl.union_stru.prior = 1;
    if (pchannel->type == FIPC_CHN_TYPE_MSG_OUTRECV) {
        pchannel->ipipe_cfg->ipipe_uctrl.union_stru.type = 1;
        p_relev_dev = fipc_device_get_fast(FIPC_GET_DEV_ID(pchannel->rdrvchn_id));
        if (p_relev_dev == NULL) {
            /* impossible */
            fipc_print_error("fipc_device_get_fast relev device fail\n");
            return -1;
        }
        pchannel->ipipe_cfg->relv_pipe_paddr =
            p_relev_dev->base_pa + FIPC_IPIPE_BASE_ADDR_L(FIPC_GET_PIPE_ID(pchannel->rdrvchn_id));
    } else {
        pchannel->ipipe_cfg->ipipe_uctrl.union_stru.type = 0;
    }

    if ((pchannel->dym_flags & FIPC_DYM_SF_NOARRVEVENT) == 0) {
        pchannel->ipipe_cfg->int_level = FIPC_INT_RD_DATA_ARV;
    }
    if (pchannel->dym_flags & FIPC_DYM_SF_TIMEOUTINTEN) {
        pchannel->ipipe_cfg->int_level |= FIPC_INT_RD_ARV_TIMEOUT;
        pchannel->ipipe_cfg->ipipe_uctrl.union_stru.up_thrh = pcompat_attr->flup;
        pchannel->ipipe_cfg->ipipe_uctrl.union_stru.dn_thrh = pcompat_attr->fldn;
        pchannel->ipipe_cfg->ipipe_uctrl.union_stru.arv_timeout = pcompat_attr->timeout;
    }

    fipc_ipipe_enable(pdev, pchannel);
    return 0;
}

static int fipc_shadow_ipipe_open(struct fipc_device *pdev, struct fipc_channel *pchannel)
{
    struct fipc_device *p_relev_dev = NULL;

    if (pchannel->ipipe_cfg == NULL) {
        pchannel->ipipe_cfg = fipc_malloc(sizeof(struct ipipe_config));
        if (pchannel->ipipe_cfg == NULL) {
            return -1;
        }
    }

    (void)memset_s(pchannel->ipipe_cfg, sizeof(struct ipipe_config), 0, sizeof(struct ipipe_config));

    pchannel->ipipe_cfg->id = FIPC_GET_PIPE_ID(pchannel->ldrvchn_id);

    p_relev_dev = fipc_device_get_fast(FIPC_GET_DEV_ID(pchannel->rdrvchn_id));
    if (p_relev_dev == NULL) {
        /* impossible */
        fipc_print_error("fipc_device_get_fast relev device fail\n");
        return -1;
    }

    pchannel->ipipe_cfg->relv_pipe_paddr =
        p_relev_dev->base_pa + FIPC_IPIPE_BASE_ADDR_L(FIPC_GET_PIPE_ID(pchannel->rdrvchn_id));

    pchannel->ipipe_cfg->ipipe_uctrl.union_stru.prior = 1;
    pchannel->ipipe_cfg->ipipe_uctrl.union_stru.type = 0x2;
    /* wakeup中断的几个条件 1，remote en为0 2，opipe不空且(达到上水线或者达到超时) */
    pchannel->ipipe_cfg->ipipe_uctrl.union_stru.arv_timeout = 1;
    return 0;
}

int fipc_channel_open(struct fipc_device *pdev, struct fipc_channel *pchannel, struct compat_attr *pcompat_attr)
{
    int ret = 0;
    if (isMsgRecvChn(pchannel)) {
        ret = fipc_ipipe_open(pdev, pchannel, pcompat_attr);
    } else if (pchannel->type == FIPC_CHN_TYPE_MSG_OUTSEND) {
        if (fipc_shadow_ipipe_open(pdev, pchannel) == 0) {
            ret = fipc_opipe_open(pdev, pchannel, pcompat_attr);
        }
    } else if (pchannel->type == FIPC_CHN_TYPE_MSG_INRSEND || pchannel->type == FIPC_CHN_TYPE_DMA) {
        ret = fipc_opipe_open(pdev, pchannel, pcompat_attr);
    } else {
        ret = -1;
    }

    return ret;
}

/* opipe中还有多少剩余空间 */
static inline u32 opipe_free_space_get(struct fipc_channel *pchannel)
{
    u32 w_off = pchannel->opipe_cfg->wptr;
    u32 r_off = *(volatile u32 *)(pchannel->opipe_cfg->ptr_vaddr);
    u32 bufsz = pchannel->opipe_cfg->depth;
    fipc_print_trace("w_off : 0x%x  r_off : 0x%x\n", (unsigned)w_off, (unsigned)r_off);
    return (w_off >= r_off) ? (bufsz - w_off + r_off) : (r_off - w_off);
}

/* ipipe中还有多少数据未读取 */
static inline u32 ipipe_used_space_get(struct fipc_channel *pchannel)
{
    u32 r_off = pchannel->ipipe_cfg->rptr;
    u32 w_off = *(volatile u32 *)(pchannel->ipipe_cfg->ptr_vaddr);
    u32 bufsz = pchannel->ipipe_cfg->depth;
    fipc_print_trace("w_off : 0x%x  r_off : 0x%x\n", (unsigned)w_off, (unsigned)r_off);
    return (w_off >= r_off) ? (w_off - r_off) : (bufsz + w_off - r_off);
}

static int opipe_put_to_ringbuf(const struct fipc_channel *pchannel, const u8 *data, u32 data_len, u32 *pw_off)
{
    u8 *ringbuf = (u8 *)pchannel->opipe_cfg->ringbuf_va;
    u32 w_off = *(pw_off);
    u32 r_off = *(volatile u32 *)(pchannel->opipe_cfg->ptr_vaddr);
    u32 bufsz = pchannel->opipe_cfg->depth;

    u32 tail_sz, wrapped_sz;

    if (bufsz < w_off || bufsz < r_off) {
        /* big error; */
        return -1;
    }
    /* w_off,r_off never be same as bufsz */
    if (w_off >= r_off) {
        tail_sz = bufsz - w_off;
        wrapped_sz = r_off; /* may be zero */
    } else {
        tail_sz = r_off - w_off;
        wrapped_sz = 0;
    }

    /*
     * 长度不能等于剩余空间，否则写指针会更新为和读指针相同的值,造成芯片认为buf为空
     */
    if (tail_sz + wrapped_sz <= data_len) {
        return -1;
    }

    if (tail_sz >= data_len) {
        if (memcpy_s(ringbuf + w_off, tail_sz, data, data_len)) {
            return -1;
        }
        w_off = (w_off + data_len == bufsz) ? 0 : (w_off + data_len);
    } else {
        if (memcpy_s(ringbuf + w_off, tail_sz, data, tail_sz)) {
            return -1;
        }
        if (memcpy_s(ringbuf, wrapped_sz, data + tail_sz, data_len - tail_sz)) { /*lint !e613*/
            return -1;
        }
        w_off = data_len - tail_sz;
    }
    *(pw_off) = w_off;
    return 0;
}

static int ipipe_get_from_ringbuf(const struct fipc_channel *pchannel, u8 *data, u32 data_len, u32 *pr_off)
{
    u8 *ringbuf = (u8 *)pchannel->ipipe_cfg->ringbuf_va;
    u32 r_off = *(pr_off);
    u32 w_off = *(volatile u32 *)(pchannel->ipipe_cfg->ptr_vaddr);
    u32 bufsz = pchannel->ipipe_cfg->depth;

    u32 tail_sz, wrapped_sz;
    if (bufsz < w_off || bufsz < r_off) {
        /* big error; */
        return -1;
    }
    /* w_off,r_off never be same as bufsz */
    if (w_off >= r_off) {
        tail_sz = w_off - r_off; /* may be zero */
        wrapped_sz = 0;
    } else {
        tail_sz = bufsz - r_off;
        wrapped_sz = w_off; /* may be zero */
    }
    /* big error should be detected by this function caller */
    if (tail_sz + wrapped_sz < data_len) {
        return -1;
    }
    if (tail_sz >= data_len) {
        if (memcpy_s(data, data_len, ringbuf + r_off, data_len)) {
            return -1;
        }
        r_off = (r_off + data_len == bufsz) ? 0 : (r_off + data_len);
    } else {
        if (memcpy_s(data, data_len, ringbuf + r_off, tail_sz)) {
            return -1;
        }
        if (memcpy_s(data + tail_sz, data_len - tail_sz, ringbuf, data_len - tail_sz)) {
            return -1;
        }
        r_off = data_len - tail_sz;
    }
    *(pr_off) = r_off;
    return 0;
}

/* 这个函数依赖fipc buff本身四字节对齐，buff内的使用至少4字节对齐 */
static void ipipe_pkt_ts_set(struct fipc_channel *pchannel)
{
    u8 *ringbuf = NULL;
    u32 r_off, w_off, bufsz, avail_space;

    u32 *ts = NULL;
    u32 ts_off;

    ringbuf = (u8 *)pchannel->ipipe_cfg->ringbuf_va;
    r_off = pchannel->ipipe_cfg->rptr;
    w_off = *(volatile u32 *)(pchannel->ipipe_cfg->ptr_vaddr);
    bufsz = pchannel->ipipe_cfg->depth;
    avail_space = (w_off >= r_off) ? (w_off - r_off) : (bufsz + w_off - r_off);
    if (sizeof(struct fipc_packet) > avail_space) {
        fipc_print_error("ipipe pkt is too small, avail_space = 0x%x\n", (unsigned)avail_space);
        return;
    }
    ts_off = offsetof(struct fipc_packet, recvtime1);
    if (r_off + ts_off >= bufsz) {
        ts = (u32 *)(ringbuf + r_off + ts_off - bufsz);
    } else {
        ts = (u32 *)(ringbuf + r_off + ts_off);
    }
    *ts = pchannel->irq_ts;
    ts_off = offsetof(struct fipc_packet, recvtime2);
    if (r_off + ts_off >= bufsz) {
        ts = (u32 *)(ringbuf + r_off + ts_off - bufsz);
    } else {
        ts = (u32 *)(ringbuf + r_off + ts_off);
    }
    *ts = (*ts) + 1;
    return;
}

int opipe_align_fixed(const struct fipc_channel *pchannel, u32 fixed_len, u32 *pw_off)
{
    u32 w_off = *(pw_off);
    if (w_off + fixed_len >= pchannel->opipe_cfg->depth) {
        w_off = w_off + fixed_len - pchannel->opipe_cfg->depth;
    } else {
        w_off = w_off + fixed_len;
    }
    *pw_off = w_off;
    return 0;
}

int ipipe_align_fixed(const struct fipc_channel *pchannel, u32 fixed_len, u32 *pr_off)
{
    u32 r_off = *(pr_off);
    if (r_off + fixed_len >= pchannel->ipipe_cfg->depth) {
        r_off = r_off + fixed_len - pchannel->ipipe_cfg->depth;
    } else {
        r_off = r_off + fixed_len;
    }
    *pr_off = r_off;
    return 0;
}

int bsp_fipc_chn_attr_init(fipc_chn_attr_t *pattr)
{
    if (pattr == NULL) {
        fipc_print_error("bad param @%d\n", __LINE__);
        return -1;
    }
    pattr->chnid = (u32)FIPC_CHN_INVAILD;
    pattr->type = FIPC_CHN_TYPE_INVAILD;
    pattr->va = NULL;
    pattr->pa = 0;
    pattr->size = 0;
    pattr->cbk = NULL;
    pattr->cbk_arg = NULL;
    pattr->req_sel = FIPC_DMA_REQ_SEL_INVAILD;
    pattr->flags = 0;
    pattr->irq_wakeup_cbk = NULL;
    pattr->irq_cbk_arg = NULL;

    return 0;
}
static int fipc_chn_attr_check(struct fipc_channel *pchannel, fipc_chn_attr_t *pattr, struct compat_attr *pcompat_attr)
{
    if (pattr->chnid == (u32)FIPC_CHN_INVAILD) {
        fipc_print_error("invaild pattr->chnid\n");
        return -1;
    }
    if (isMsgSendChn(pchannel)) {
        if (pattr->type != FIPC_CHN_TYPE_SEND) {
            return -1;
        }
    } else if (isMsgRecvChn(pchannel)) {
        if (pattr->type != FIPC_CHN_TYPE_RECV) {
            return -1;
        }
    } else {
        if (pattr->type != FIPC_CHN_TYPE_SEND) {
            return -1;
        }
    }

    if (pattr->size <= sizeof(struct fipc_packet)) {
        return -1;
    }
    if ((pattr->pa & FIPC_PIPE_ADDR_ALIGN_MASK) || (pattr->size & FIPC_PIPE_DEPTH_ALIGN_MASK)) {
        fipc_print_error("fipc attr align check,pattr->va is 0x%x, pattr->pa is 0x%x pattr->size is 0x%x\n",
            (unsigned)(uintptr_t)pattr->va, (unsigned)pattr->pa, (unsigned)pattr->size);
        return -1;
    }
    if (pattr->flags & FIPC_DYM_SF_TIMEOUTINTEN) {
        if (pattr->flup > FIPC_IPIPE_THRESTHOLD_MASK || pattr->fldn > FIPC_IPIPE_THRESTHOLD_MASK ||
            pattr->timeout > FIPC_IPIPE_TIMEOUT_MASK) {
            fipc_print_error("fipc attr fl and timeout check fail\n");
            return -1;
        }
    }
    pcompat_attr->align_bytes = FIPC_CHN_DEFAULT_ALIGNBYTES;
    pcompat_attr->pa = pattr->pa;
    pcompat_attr->va = pattr->va;
    pcompat_attr->depth = pattr->size;
    pcompat_attr->buf_sz = pattr->size;
    pcompat_attr->dym_flags = pattr->flags;
    pcompat_attr->req_sel = pattr->req_sel;
    pcompat_attr->flup = pattr->flup;
    pcompat_attr->fldn = pattr->fldn;
    pcompat_attr->timeout = pattr->timeout;
    return 0;
}

int fipc_chn_open(struct fipc_device *pdev, struct fipc_channel *pchannel, struct compat_attr *pcompat_attr)
{
    if (pdev == NULL || pchannel == NULL || pcompat_attr == NULL) {
        fipc_print_error("bad param @%d\n", __LINE__);
        return FIPC_ERR_EINVAL;
    }

    if (pdev->ctrl_level != FIPC_DEV_CONTROL_LEVEL_PART) {
        if (fipc_channel_open(pdev, pchannel, pcompat_attr)) {
            fipc_print_error("fipc_channel_open failed @%d\n", __LINE__);
            return -1;
        }
        pchannel->state = (u32)FIPC_CHN_OPEN;
    } else {
        pchannel->pattr = (struct compat_attr *)fipc_malloc(sizeof(struct compat_attr));
        if (pchannel->pattr == NULL) {
            fipc_print_error("malloc failed when fipc open delay channel!\n");
            return -1;
        }
        memcpy_s(pchannel->pattr, sizeof(struct compat_attr), pcompat_attr, sizeof(struct compat_attr));
        pchannel->state = (u32)FIPC_CHN_OPEN_DELAY;
    }
    return 0;
}

int bsp_fipc_chn_open(fipc_chn_t *pchn_hdl, fipc_chn_attr_t *pattr)
{
    fipc_chn_t chn_hdl;
    struct fipc_device *pdev = NULL;
    struct fipc_channel *pchannel = NULL;
    struct fipc_channel *pchannel_fake = NULL;
    struct compat_attr compat_attr;

    if (pchn_hdl == NULL || pattr == NULL) {
        fipc_print_error("bad param @%d\n", __LINE__);
        return FIPC_ERR_EINVAL;
    }
    fipc_print_trace("bsp_fipc_chn_open pattr->chnid=0x%x\n", pattr->chnid);
    /* get device and channel */
    pchannel_fake = fipc_channel_get_fast(pattr->chnid);
    if (pchannel_fake == NULL) {
        fipc_print_error("attr->usr_chnid %d is not defined,check dts please\n", pattr->chnid);
        return FIPC_ERR_ENXIO;
    }

    chn_hdl = pchannel_fake->ldrvchn_id;
    if (fipc_dev_chn_get(FIPC_GET_PHYCHN_ID(chn_hdl), &pdev, &pchannel)) {
        fipc_print_error("fipc_dev_chn_get failed\n");
        return FIPC_ERR_ENXIO;
    }

    if (pchannel->state != (u32)FIPC_CHN_INIT) {
        fipc_print_error("bad state @%d\n", __LINE__);
        return FIPC_ERR_EBADFD;
    }

    if (fipc_chn_attr_check(pchannel, pattr, &compat_attr)) {
        fipc_print_error("bad param @%d\n", __LINE__);
        return -1;
    }
    pchannel->align_bytes = compat_attr.align_bytes;
    pchannel->dym_flags = compat_attr.dym_flags;
    pchannel->cbk = pattr->cbk; /* can be null */
    pchannel->cbk_arg = pattr->cbk_arg;
    pchannel->irq_wakeup_cbk = pattr->irq_wakeup_cbk; /* can be null */
    pchannel->irq_cbk_arg = pattr->irq_cbk_arg;

    *(pchn_hdl) = chn_hdl;
    if (fipc_chn_open(pdev, pchannel, &compat_attr)) {
        return -1;
    }

    return 0;
}

static int fipc_fill_hdr(struct fipc_channel *pchannel, struct fipc_send_packet *ppacket,
    struct fipc_send_helper *phelper)
{
    u32 blkidx;
    u32 head_len;
    u32 total_msglen = 0;

    fipc_blk_desc_t *pblkdesc = (fipc_blk_desc_t *)phelper->send_desc;
    for (blkidx = 0; blkidx < pblkdesc->cnt; blkidx++) {
        if (pblkdesc->datablk[blkidx].buf == NULL || pblkdesc->datablk[blkidx].len == 0) { /* 不支持空消息发送 */
            fipc_print_error("bad param @%d\n", __LINE__);
            return FIPC_ERR_EINVAL;
        }
        total_msglen = total_msglen + pblkdesc->datablk[blkidx].len;
    }
    if (total_msglen == 0 || total_msglen > 0xFFFFUL) {
        fipc_print_error("bad param @%d\n", __LINE__);
        return FIPC_ERR_EINVAL;
    }

    /* 这步有点多余，我们的头一定32bytes对齐 */
    head_len = fipc_roundup(sizeof(struct fipc_packet), (pchannel->align_bytes));
    ppacket->bd_head.cfg.msg_len = head_len + fipc_roundup(total_msglen, pchannel->align_bytes);
    if (phelper->send_flags & FIPC_SEND_FLAGS_NOARRVNOTIFY) {
        ppacket->bd_head.cfg.II_pos = 0;
    } else {
        ppacket->bd_head.cfg.II_pos = 1;
    }
    ppacket->data_head.user_id = pchannel->user_id;
    ppacket->data_head.len = (u16)total_msglen;
    ppacket->data_head.timestamp = fipc_get_slice_value();

    ppacket->data_head.align_bytes = pchannel->align_bytes;
    ppacket->data_head.ldrvchn_id = pchannel->ldrvchn_id;
    ppacket->data_head.rdrvchn_id = pchannel->rdrvchn_id;

    phelper->head_len = head_len;
    phelper->total_msglen = total_msglen;
    phelper->write_len = ppacket->bd_head.cfg.msg_len + sizeof(struct fipc_bd1_s);

    return 0;
}

static int fipc_write_packet(struct fipc_device *pdev, struct fipc_channel *pchannel, struct fipc_send_packet *ppacket,
    struct fipc_send_helper *phelper)
{
    u32 blkidx;
    u32 total_msglen, write_len, space_left;
    u32 wptr;
    fipc_blk_desc_t *pblkdesc = (fipc_blk_desc_t *)phelper->send_desc;

    total_msglen = phelper->total_msglen;
    write_len = phelper->write_len;
    space_left = opipe_free_space_get(pchannel);
    fipc_print_trace("space left : 0x%x\n", (unsigned)space_left);
    if (write_len >= space_left) {
        fipc_print_trace("opipe nospace,write len: 0x%x, space left: 0x%x, send fail\n", write_len, space_left);
        return FIPC_ERR_EAGAIN;
    }

    wptr = pchannel->opipe_cfg->wptr;
    if (opipe_put_to_ringbuf(pchannel, (u8 *)ppacket, sizeof(*ppacket), &wptr)) {
        fipc_print_error("opipe_put_to_ringbuf failed @%d\n", __LINE__);
        return FIPC_ERR_EIO;
    }
    opipe_align_fixed(pchannel, 0, &wptr);

    for (blkidx = 0; blkidx < pblkdesc->cnt; blkidx++) {
        if (opipe_put_to_ringbuf(pchannel, pblkdesc->datablk[blkidx].buf, pblkdesc->datablk[blkidx].len, &wptr)) {
            fipc_print_error("opipe_put_to_ringbuf failed @%d\n", __LINE__);
            return FIPC_ERR_EIO;
        }
    }
    opipe_align_fixed(pchannel, fipc_roundup(total_msglen, pchannel->align_bytes) - total_msglen, &wptr);

    pchannel->opipe_cfg->wptr = wptr;
    fipc_opipe_wptr_update(pdev->base_va, FIPC_GET_PIPE_ID(pchannel->ldrvchn_id), pchannel->opipe_cfg->wptr);
    fipc_print_trace("fipc_opipe_wptr_update wptr: 0x%x\n,rptr:0x%x ", (unsigned)wptr,
        (unsigned)*(volatile u32 *)(pchannel->opipe_cfg->ptr_vaddr));
    return 0;
}

int bsp_fipc_chn_blks_send(fipc_chn_t chn_hdl, fipc_blk_desc_t *pblkdesc, u32 send_flags)
{
    int ret;
    unsigned long flags;
    struct fipc_device *pdev = NULL;
    struct fipc_channel *pchannel = NULL;
    struct fipc_send_packet packet = { { { 0 } }, { 0 } };
    struct fipc_send_helper helper;
    struct fipc_send_helper *phelper = &helper;
    if (pblkdesc == NULL || fipc_dev_chn_get(FIPC_GET_PHYCHN_ID(chn_hdl), &pdev, &pchannel)) {
        fipc_print_error("invaild param for chn_hdl 0x%x\n", (unsigned)chn_hdl);
        return FIPC_ERR_EINVAL;
    }
    if (pchannel->state != (u32)FIPC_CHN_OPEN || !isMsgSendChn(pchannel)) {
        fipc_print_error("bad chn_hdl 0x%x\n", (unsigned)chn_hdl);
        return FIPC_ERR_EBADFD;
    }
    phelper->send_desc = pblkdesc;
    phelper->send_flags = send_flags;
    if (fipc_fill_hdr(pchannel, &packet, phelper)) {
        return FIPC_ERR_EINVAL;
    }

    spin_lock_irqsave(&pchannel->lock, flags);
    ret = fipc_write_packet(pdev, pchannel, &packet, phelper);
    spin_unlock_irqrestore(&pchannel->lock, flags);
    if (ret) {
        return ret;
    }
    return (int)packet.data_head.len;
}

int bsp_fipc_chn_send(fipc_chn_t chn_hdl, void *buf, unsigned len, u32 send_flags)
{
    fipc_blkx2_desc_t send_desc;
    send_desc.cnt = 1;
    send_desc.datablk[0].buf = buf;
    send_desc.datablk[0].len = len;
    return bsp_fipc_chn_blks_send(chn_hdl, (fipc_blk_desc_t *)&send_desc, send_flags);
}

static int fipc_read_hdr(struct fipc_channel *pchannel, struct fipc_packet *ppkthdr, struct fipc_recv_helper *phelper)
{
    u32 used_len;
    used_len = ipipe_used_space_get(pchannel);
    fipc_print_trace("ipipe used length 0x%x\n", (unsigned)used_len);
    if (used_len == 0) {
        phelper->used_len = used_len;
        return 0;
    }
    if (used_len < sizeof(struct fipc_packet)) {
        /* big error */
        return -1;
    }
    phelper->rptr = pchannel->ipipe_cfg->rptr;
    if (ipipe_get_from_ringbuf(pchannel, (u8 *)ppkthdr, sizeof(*ppkthdr), &phelper->rptr)) {
        fipc_print_error("ipipe_get_from_ringbuf failed @%d\n", __LINE__);
        return -1;
    }
    if (ppkthdr->align_bytes != pchannel->align_bytes) {
        fipc_print_error("ipipe_get_from_ringbuf failed @%d\n", __LINE__);
        return -1;
    }
    if (used_len < fipc_roundup(ppkthdr->len, pchannel->align_bytes) +
        fipc_roundup(sizeof(struct fipc_packet), pchannel->align_bytes)) {
        fipc_print_error("ipipe_get_from_ringbuf failed @%d\n", __LINE__);
        return -1;
    }
    phelper->used_len = used_len;
    return 0;
}

int fipc_read_payload(struct fipc_device *pdev, struct fipc_channel *pchannel, struct fipc_packet *ppkthdr,
    struct fipc_recv_helper *phelper)
{
    int ret = 0;
    u32 ret_len = 0;
    if (phelper->payloadskip_len >= ppkthdr->len) {
        fipc_print_error("payloadskip_len check failed @%d\n", __LINE__);
        return -1;
    }
    if (phelper->len < ppkthdr->len - phelper->payloadskip_len) {
        ret = ipipe_get_from_ringbuf(pchannel, (u8 *)phelper->buf, phelper->len, &phelper->seek);
        if (ret == 0) {
            ret_len = phelper->len;
            if ((phelper->recv_flags & FIPC_RECV_FLAGS_DATAPEEK) == 0) {
                pchannel->ipipe_cfg->seek = phelper->seek;
            }
        } else {
            fipc_print_error("ipipe_get_from_ringbuf failed @%d\n", __LINE__);
        }
    } else {
        ret = ipipe_get_from_ringbuf(pchannel, (u8 *)phelper->buf, ppkthdr->len - phelper->payloadskip_len,
            &phelper->seek);
        if (ret == 0) {
            ret_len = ppkthdr->len - phelper->payloadskip_len;
            if ((phelper->recv_flags & FIPC_RECV_FLAGS_DATAPEEK) == 0) {
                ipipe_align_fixed(pchannel, fipc_roundup(ppkthdr->len, pchannel->align_bytes) - ppkthdr->len,
                    &phelper->seek);
                pchannel->ipipe_cfg->seek = phelper->seek;
                pchannel->ipipe_cfg->rptr = phelper->seek; /* round check */
                fipc_ipipe_rptr_update(pdev->base_va, FIPC_GET_PIPE_ID(pchannel->ldrvchn_id),
                    pchannel->ipipe_cfg->rptr);
                fipc_print_trace("fipc_ipipe_rptr_update rptr: 0x%x , wptr: 0x%x\n",
                    (unsigned)pchannel->ipipe_cfg->rptr, (unsigned)*(volatile u32 *)(pchannel->ipipe_cfg->ptr_vaddr));
            }
        } else {
            fipc_print_error("ipipe_get_from_ringbuf failed @%d\n", __LINE__);
        }
    }

    if (ret == 0) {
        return (int)ret_len;
    }
    return -1;
}

int bsp_fipc_chn_recv(fipc_chn_t chn_hdl, void *buf, unsigned len, u32 recv_flags)
{
    int ret;
    unsigned long flags;
    struct fipc_device *pdev = NULL;
    struct fipc_channel *pchannel = NULL;
    struct fipc_packet pkthdr = { 0 };
    struct fipc_recv_helper helper;
    struct fipc_recv_helper *phelper = &helper;

    if (buf == NULL || len == 0) {
        fipc_print_error("bad param for chn_hdl 0x%x\n", (unsigned)chn_hdl);
        return -1;
    }
    if (fipc_dev_chn_get(FIPC_GET_PHYCHN_ID(chn_hdl), &pdev, &pchannel)) {
        fipc_print_error("invaild chn_hdl 0x%x\n", (unsigned)chn_hdl);
        return -1;
    }
    if (pchannel->state != (u32)FIPC_CHN_OPEN || !isMsgRecvChn(pchannel)) {
        fipc_print_error("bad chn_hdl 0x%x\n", (unsigned)chn_hdl);
        return -1;
    }
    phelper->buf = buf;
    phelper->len = len;
    phelper->recv_flags = recv_flags;
    spin_lock_irqsave(&pchannel->lock, flags);

    ret = fipc_read_hdr(pchannel, &pkthdr, phelper);
    if (ret) {
        spin_unlock_irqrestore(&pchannel->lock, flags);
        return ret;
    }
    if (phelper->used_len == 0) { /* empty */
        spin_unlock_irqrestore(&pchannel->lock, flags);
        return 0;
    }
    /* 如果这俩相同，则可以认为是第一次读，seek要跳过fipc头 */
    if (pchannel->ipipe_cfg->seek == pchannel->ipipe_cfg->rptr) {
        phelper->seek = phelper->rptr;
        phelper->payloadskip_len = 0;
    } else {
        phelper->seek = pchannel->ipipe_cfg->seek;
        if (phelper->seek > phelper->rptr) {
            phelper->payloadskip_len = phelper->seek - phelper->rptr;
        } else {
            phelper->payloadskip_len = pchannel->ipipe_cfg->depth - phelper->rptr + phelper->seek;
        }
    }

    ret = fipc_read_payload(pdev, pchannel, &pkthdr, phelper);

    spin_unlock_irqrestore(&pchannel->lock, flags);
    return ret;
}

static int fipc_ioctl_chn_nxtpkt_inf(struct fipc_channel *pchannel, void *arg, u32 size)
{
    unsigned long lockflags;
    u32 rptr;
    struct fipc_packet pkthdr = { 0 };
    ioctl_nxtpkt *pktinfo = (ioctl_nxtpkt *)arg;

    if (pktinfo == NULL || size < sizeof(*pktinfo) || !isMsgRecvChn(pchannel)) {
        return -1;
    }
    spin_lock_irqsave(&pchannel->lock, lockflags);
    rptr = pchannel->ipipe_cfg->rptr;
    if (ipipe_get_from_ringbuf(pchannel, (u8 *)&pkthdr, sizeof(pkthdr), &rptr)) {
        pktinfo->len = 0;
        pktinfo->pkthdl = FIPC_DFT_INVAILD_VALUE; /* wptr和rptr不可能到这里 */
    } else {
        pktinfo->len = pkthdr.len;
        pktinfo->pkthdl = pchannel->ipipe_cfg->rptr;
    }
    spin_unlock_irqrestore(&pchannel->lock, lockflags);
    return 0;
}

int bsp_fipc_chn_ioctl(fipc_chn_t chn_hdl, unsigned req, void *arg, u32 size)
{
    int ret;
    struct fipc_device *pdev = NULL;
    struct fipc_channel *pchannel = NULL;

    if (fipc_dev_chn_get(FIPC_GET_PHYCHN_ID(chn_hdl), &pdev, &pchannel)) {
        fipc_print_error("invaild chn_hdl 0x%x\n", (unsigned)chn_hdl);
        return -1;
    }
    if (req == FIPC_IOCTL_CHN_NXTPKT_INF) {
        ret =  fipc_ioctl_chn_nxtpkt_inf(pchannel, arg, size);
        if(ret) {
            return -1;
        }
        return 0;
    }
    return -1;
}

static int fipc_data_arrive_event_notify(fipc_event event, struct fipc_channel *pchannel)
{
    u32 rptr;
    fipc_eventinfo eventinfo;

    eventinfo.data_arrv.chnid = pchannel->user_id;
    eventinfo.data_arrv.chn_hdl = (fipc_chn_t)pchannel->ldrvchn_id;

    while (ipipe_used_space_get(pchannel)) {
        rptr = pchannel->ipipe_cfg->rptr;
        if (!(pchannel->const_flags & FIPC_CHN_SF_NO_EICC_HEADER)) {
            ipipe_pkt_ts_set(pchannel);
        }
        if (pchannel->cbk) {
            pchannel->cbk(event, pchannel->cbk_arg, &eventinfo);
        }
        if (pchannel->ipipe_cfg->rptr == rptr) {
            break;
        }
    }

    return 0;
}

static int fipc_send_flowup_event_notify(fipc_event event, struct fipc_channel *pchannel)
{
    fipc_print_crit("fipc_send_flowup_event_notify %d pchannel->user_id 0x%x\n", (int)event,
        (unsigned)pchannel->user_id);
    return 0;
}

static int fipc_dma_arrive_event_notify(fipc_event event, struct fipc_channel *pchannel)
{
    fipc_eventinfo eventinfo;

    eventinfo.data_arrv.chnid = pchannel->user_id;
    eventinfo.data_arrv.chn_hdl = (fipc_chn_t)pchannel->ldrvchn_id;

    if (pchannel->cbk) {
        pchannel->cbk(event, pchannel->cbk_arg, &eventinfo);
    }

    return 0;
}

static int fipc_fusion_normal_opipe_event_notify(u32 inttype, struct fipc_device *pdev, struct fipc_channel *pchannel)
{
    int ret = -1;
    switch (inttype) {
        case FIPC_INT_WR_OVERFLOW:
            ret = fipc_send_flowup_event_notify(FIPC_EVENT_SEND_FLOWUP, pchannel);
            break;
        case FIPC_INT_WR_DATA_DONE:
            ret = fipc_dma_arrive_event_notify(FIPC_EVENT_SEND_DONE, pchannel);
            break;
        default:
            fipc_print_crit("opipe inttype 0x%x @dev[%d] is not impl now\n", (unsigned)inttype, (int)pdev->id);
            break;
    }
    return ret;
}

static int fipc_fusion_normal_ipipe_event_notify(u32 inttype, struct fipc_device *pdev, struct fipc_channel *pchannel)
{
    int ret = -1;
    switch (inttype) {
        case FIPC_INT_RD_DATA_ARV:
            ret = fipc_data_arrive_event_notify(FIPC_EVENT_DATA_ARRV, pchannel);
            break;
        default:
            fipc_print_crit("ipipe inttype 0x%x @dev[%d] is not impl now\n", (unsigned)inttype, (int)pdev->id);
            break;
    }
    return ret;
}

int fipc_normal_event_notify(u32 inttype, struct fipc_device *pdev, u32 pipe_id)
{
    int ret;
    struct fipc_channel *pchannel = NULL;
    if (pipe_id >= FIPC_CHN_ID_MAX) {
        fipc_print_error("invaild pipe_id 0x%x\n", (unsigned)pipe_id);
        /* big error */
        return -1;
    }
    pchannel = pdev->channels[pipe_id];
    if (pchannel == NULL) {
        fipc_print_error("no channel relev to pipe_id 0x%x\n", (unsigned)pipe_id);
        return -1;
    }
    pchannel->irq_ts = fipc_get_slice_value();
    if (isMsgRecvChn(pchannel)) {
        ret = fipc_fusion_normal_ipipe_event_notify(inttype, pdev, pchannel);
    } else {
        ret = fipc_fusion_normal_opipe_event_notify(inttype, pdev, pchannel);
    }

    return ret;
}

int fipc_wakeup_event_notify(struct fipc_device *pldev, u32 ipipe_id)
{
    unsigned long flags;
    struct fipc_channel *pchannel = NULL;
    if (ipipe_id >= FIPC_CHN_ID_MAX) {
        fipc_print_error("invaild ipipe_id 0x%x\n", (unsigned)ipipe_id);
        /* big error */
        return -1;
    }
    pchannel = pldev->channels[ipipe_id];
    if (pchannel == NULL) {
        fipc_print_error("no channel relev to ipipe_id 0x%x\n", (unsigned)ipipe_id);
        return -1;
    }

    pchannel->irq_ts = fipc_get_slice_value();
    spin_lock_irqsave(&pchannel->lock, flags);
    if (pchannel->state == FIPC_CHN_OPEN) {
        fipc_print_trace("ipipe_id 0x%x remote en\n", (unsigned)ipipe_id);
        fipc_ipipe_local_en(pldev->base_va, FIPC_GET_PIPE_ID(pchannel->ldrvchn_id));
        fipc_ipipe_remote_en(pldev->base_va, FIPC_GET_PIPE_ID(pchannel->ldrvchn_id));
        pchannel->remote_awake = 0;
    } else {
        fipc_print_trace("ipipe_id 0x%x remote flag\n", (unsigned)ipipe_id);
        pchannel->remote_awake = 1;
    }
    spin_unlock_irqrestore(&pchannel->lock, flags);
    return 0;
}

int fipc_bd2_head_write(struct fipc_device *pdev, struct fipc_channel *pchannel, struct fipc_bd2_s *pbd_head, void *buf,
    unsigned len, u32 flags)
{
    unsigned long lock_flags;
    if (buf == NULL || len == 0) {
        return FIPC_ERR_EIO;
    }
    (void)memset_s(pbd_head, sizeof(*pbd_head), 0, sizeof(struct fipc_bd2_s));
    pbd_head->cfg.S_pos = 1;
    pbd_head->cfg.II_pos = (flags & FIPC_SEND_FLAGS_NOARRVNOTIFY) ? 0 : 1;
    pbd_head->cfg.OI_pos = (flags & FIPC_SEND_FLAGS_NODONENOTIFY) ? 0 : 1;

    pbd_head->cfg.msg_len = (u16)len;
    pbd_head->src_addr_l = (uintptr_t)buf;

    spin_lock_irqsave(&pchannel->lock, lock_flags);
    if (opipe_put_to_ringbuf(pchannel, (u8 *)pbd_head, sizeof(struct fipc_bd2_s), &(pchannel->opipe_cfg->wptr))) {
        fipc_print_error("opipe_put_to_ringbuf failed @%d\n", __LINE__);
        spin_unlock_irqrestore(&pchannel->lock, lock_flags);
        return FIPC_ERR_EIO;
    }
    fipc_opipe_wptr_update(pdev->base_va, FIPC_GET_PIPE_ID(pchannel->ldrvchn_id), pchannel->opipe_cfg->wptr);
    spin_unlock_irqrestore(&pchannel->lock, lock_flags);
    return 0;
}

int bsp_fipc_chn_blk2ring_send(fipc_chn_t chn_hdl, void *buf, unsigned len, u32 flags)
{
    struct fipc_device *pdev = NULL;
    struct fipc_channel *pchannel = NULL;
    struct fipc_bd2_s bd_head;
    u32 ret;
    if (fipc_dev_chn_get(FIPC_GET_PHYCHN_ID(chn_hdl), &pdev, &pchannel)) {
        fipc_print_error("invaild param for chn_hdl 0x%x\n", (unsigned)chn_hdl);
        return FIPC_ERR_EINVAL;
    }
    if (pchannel->state != (u32)FIPC_CHN_OPEN || !isMsgSendChn(pchannel)) {
        fipc_print_error("bad chn_hdl 0x%x\n", (unsigned)chn_hdl);
        return FIPC_ERR_EBADFD;
    }

    ret = fipc_bd2_head_write(pdev, pchannel, &bd_head, buf, len, flags);
    if (ret != 0) {
        return ret;
    }
    return (int)len;
}

int bsp_fipc_chn_ringbuf_rptr_set(fipc_chn_t chn_hdl, unsigned int rptr)
{
    struct fipc_device *pdev = NULL;
    struct fipc_channel *pchannel = NULL;
    unsigned long lock_flags;

    if (fipc_dev_chn_get(FIPC_GET_PHYCHN_ID(chn_hdl), &pdev, &pchannel)) {
        fipc_print_error("invaild param for chn_hdl 0x%x\n", (unsigned)chn_hdl);
        return FIPC_ERR_EINVAL;
    }

    if ((rptr & 0x03) != 0) {
        return FIPC_ERR_EIO;
    }

    if (pchannel->type != FIPC_CHN_TYPE_MSG_INRRECV) {
        return FIPC_ERR_ENXIO;
    }
    if (pchannel->ipipe_cfg == NULL) {
        return FIPC_ERR_ENXIO;
    }
    spin_lock_irqsave(&pchannel->lock, lock_flags);
    pchannel->ipipe_cfg->rptr = rptr;
    fipc_ipipe_rptr_update(pdev->base_va, FIPC_GET_PIPE_ID(pchannel->ldrvchn_id), pchannel->ipipe_cfg->rptr);
    spin_unlock_irqrestore(&pchannel->lock, lock_flags);
    return 0;
}

int bsp_fipc_chn_wptr_get(fipc_chn_t chn_hdl, unsigned int *wptr)
{
    struct fipc_device *pdev = NULL;
    struct fipc_channel *pchannel = NULL;

    if  (wptr == NULL) {
        return FIPC_ERR_EIO;
    }

    if (fipc_dev_chn_get(FIPC_GET_PHYCHN_ID(chn_hdl), &pdev, &pchannel)) {
        fipc_print_error("invaild param for chn_hdl 0x%x\n", (unsigned)chn_hdl);
        return FIPC_ERR_EINVAL;
    }

    if (pchannel->ipipe_cfg != NULL) {
        *wptr = fipc_ipipe_wptr_get(pdev->base_va, FIPC_GET_PIPE_ID(pchannel->ldrvchn_id));
        return 0;
    }
    return FIPC_ERR_ENXIO;
}

int bsp_fipc_chn_rptr_get(fipc_chn_t chn_hdl, unsigned int *rptr)
{
    struct fipc_device *pdev = NULL;
    struct fipc_channel *pchannel = NULL;

    if  (rptr == NULL) {
        return FIPC_ERR_EIO;
    }

    if (fipc_dev_chn_get(FIPC_GET_PHYCHN_ID(chn_hdl), &pdev, &pchannel)) {
        fipc_print_error("invaild param for chn_hdl 0x%x\n", (unsigned)chn_hdl);
        return FIPC_ERR_EINVAL;
    }

    if (pchannel->ipipe_cfg != NULL) {
        *rptr = fipc_ipipe_rptr_get(pdev->base_va, FIPC_GET_PIPE_ID(pchannel->ldrvchn_id));
        return 0;
    }
    if (pchannel->opipe_cfg != NULL) {
        *rptr = fipc_opipe_rptr_get(pdev->base_va, FIPC_GET_PIPE_ID(pchannel->ldrvchn_id));
        return 0;
    }
    return FIPC_ERR_ENXIO;
}


EXPORT_SYMBOL(bsp_fipc_chn_open);
EXPORT_SYMBOL(bsp_fipc_chn_send);
EXPORT_SYMBOL(bsp_fipc_chn_recv);
EXPORT_SYMBOL(bsp_fipc_chn_attr_init);
EXPORT_SYMBOL(bsp_fipc_chn_ioctl);
EXPORT_SYMBOL(bsp_fipc_dump);
