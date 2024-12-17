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

#include <securec.h>
#include <linux/version.h>
#include <linux/device.h>
#include <linux/of_device.h>
#include <linux/dma-mapping.h>
#include <osl_malloc.h>
#include <bsp_fipc.h>
#include <bsp_slice.h>
#include <bsp_nvim.h>
#include "nv_comm.h"
#include "nv_msg.h"
#include "nv_debug.h"

static struct platform_device g_modem_nv_device = {
    .name = "modem_nv",
    .id = 0,
    .dev = {
        .init_name = "modem_nv",
    },
};
void nv_dma_init(void)
{
    struct device *pdev = &g_modem_nv_device.dev;
    of_dma_configure(pdev, NULL, true);
    dma_set_mask_and_coherent(pdev, ~(0x1FULL)); // 32 bytes align

    return;
}
/* 仅用于初始化nv设备 */
s32 nv_init_dev(void)
{
    u32 ret;
    ret = (u32)platform_device_register(&g_modem_nv_device);
    if (ret) {
        nv_printf("platform_device_register modem_nv_device fail !\n");
        return -1;
    }
    nv_dma_init();
    nv_printf("init modem nv dev ok\n");
    return NV_OK;
}
void *nv_dma_alloc(u32 size, dma_addr_t *dma_handle)
{
    void *mem_addr = NULL;
    struct device *pdev = &g_modem_nv_device.dev;

    mem_addr = dma_alloc_coherent(pdev, size, dma_handle, GFP_KERNEL);
    if (IS_ERR_OR_NULL(mem_addr)) {
        nv_printf("nv alloc failed\n");
        return NULL;
    }
    return mem_addr;
}

void nv_dma_free(size_t size, void *cpu_addr, dma_addr_t dma_handle)
{
    struct device *pdev = &g_modem_nv_device.dev;
    dma_free_coherent(pdev, size, cpu_addr, dma_handle);
    return;
}

#define NV_CPMSG_TSK_DELAY 5
u32 nv_cpmsg_send(u8 *pdata, u32 len)
{
    int ret = 0;
    u32 send_count;
    struct nvm_msg_info *msg_info = nv_get_msginfo();
    struct nv_fipc_chan *chn = &msg_info->sendchn;

    for (send_count = 0; send_count < NV_MSG_SEND_COUNT; send_count++) {
        ret = bsp_fipc_chn_send(chn->chn, pdata, len, 0);
        nv_dbg_printf("send count:0x%x, ret:0x%x, len:0x%x\n", send_count, ret, len);
        if (ret == NV_CHANNAL_FULL) {
            osl_task_delay(NV_CPMSG_TSK_DELAY);
            continue;
        } else if (ret != len) {
            nv_printf("nv send msg fail, ret:%d\n", ret);
            return (u32)ret;
        } else {
            nv_debug_record(NV_DEBUG_SEND_ICC | (((struct nv_msg_data *)pdata)->msg_type << 0x10));
            return NV_OK;
        }
    }

    nv_printf("nv send fipc msg fail\n");
    return NV_ERROR;
}

void nv_cpmsg_cb(struct nv_msg_data *msg)
{
    u32 ret;
    struct nv_msg_data msg_data = { 0 };
    struct nvm_info *nvminfo = nv_get_nvminfo();

    msg_data.magicnum = NV_MSG_MAGICNUM;
    msg_data.slice = bsp_get_slice_value();
    msg_data.msg_type = NV_ICC_CNF;
    msg_data.msg = 0;
    msg_data.sn = msg->sn;
    msg_data.ret = msg->ret;
    msg_data.rcvlen = 0;
    msg_data.data_len = 0;

    ret = nv_cpmsg_send((u8 *)&msg_data, sizeof(struct nv_msg_data));
    if (ret) {
        nv_printf("send cp msg fail\n");
    }

    __pm_relax(nvminfo->wakelock);
    nvminfo->msg_info.cb_reply++;
}

static u32 nv_handle_recv_msg(struct nv_msg_data *msg_data)
{
    u32 ret = 0;
    struct nvm_info *nvminfo = nv_get_nvminfo();

    nvminfo->msg_info.cb_count++;

    if (msg_data->data_len > NV_MSG_LEN) {
        return BSP_ERR_NV_MEM_SIZE_ERR;
    }

    switch (msg_data->msg_type) {
        case NV_ICC_REQ_FLUSH:
        case NV_ICC_REQ_FLUSH_RWNV:
        case NV_ICC_REQ_LOAD_BACKUP:
        case NV_ICC_REQ_UPDATE_DEFAULT:
        case NV_ICC_REQ_UPDATE_BACKUP:
        case NV_ICC_REQ_UPDATE_BACKUP_BK:
        case NV_ICC_REQ_FACTORY_RESET:
        case NV_ICC_REQ_DATA_WRITEBACK:
        case NV_ICC_REQ_READ_BLK:
        case NV_ICC_REQ_SET_PATCH_FLAG:
        case NV_ICC_REQ_RD_PATCH_STATE:
        case NV_ICC_REQ_WR_PATCH_INFO:
        case NV_ICC_REQ_GET_PRODUCTID:
        case NV_ICC_REQ_WRITE_SCBLK:
            __pm_stay_awake(nvminfo->wakelock);
            ret = nv_awake_msgtask(msg_data);
            break;
        case NV_ICC_REQ_FLUSH_RDWR_ASYNC:
            ret = nv_awake_msgtask(msg_data);
            break;
        default:
            nv_printf("invalid nv msg type %d\n", msg_data->msg_type);
            ret = BSP_ERR_NV_ICC_CHAN_ERR;
    }

    nv_debug_record(NV_DEBUG_RECEIVE_ICC | (msg_data->msg_type << 0x10));

    return ret;
}

/*
 * read fipc msg need in queue, otherwise will read bad msg
 */
static int nv_cpmsg_handle(fipc_event event, void *arg, const fipc_eventinfo *event_info)
{
    int ret;
    struct nvm_msg_info *msg_info = nv_get_msginfo();
    struct nv_fipc_chan *chn = &msg_info->recvchn;
    struct nv_msg_data msg_data = { 0 };

    if ((event != FIPC_EVENT_DATA_ARRV) || (event_info == NULL)) {
        nv_printf("nv fipc msg receive fail\n");
        return -1;
    }

    if (event_info->data_arrv.chn_hdl != chn->chn) {
        nv_printf("nv fipc chan not match\n");
        return -1;
    }

    ret = bsp_fipc_chn_recv(chn->chn, &msg_data, sizeof(struct nv_msg_data), 0);
    if (ret != sizeof(struct nv_msg_data)) {
        nv_printf("nv read fipc msg fail, ret:%d\n", ret);
        return ret;
    }

    if (msg_data.magicnum != NV_MSG_MAGICNUM) {
        nv_printf("invlid msg data magicnum:0x%x\n", msg_data.magicnum);
        return -1;
    }

    nv_dbg_printf("sn:0x%x, msg_type:0x%x recv\n", msg_data.sn, msg_data.msg_type);
    nv_debug_record(NV_DEBUG_READ_ICC | ((msg_data.sn) << 0x10));

    ret = (int)nv_handle_recv_msg(&msg_data);
    if (ret) {
        nv_printf("handle apmsg failed\n");
    }

    return ret;
}

/*
 * nv_fipc_chan_init bufsize must over 32 byte
 */
static int nv_fipc_chan_init(struct nv_fipc_chan *chn, fipc_cbk cbk, unsigned chnid, unsigned type, unsigned bufsize)
{
    int ret;
    void *addr = NULL;
    dma_addr_t dma_handle;

    ret = bsp_fipc_chn_attr_init(&chn->attr);
    if (ret) {
        nv_printf("nv fipc chan init fail, ret:%d\n", ret);
        return (u32)ret;
    }

    addr = nv_dma_alloc(bufsize, &dma_handle);
    if (addr == NULL) {
        nv_printf("nv malloc chan buf fail\b");
        return BSP_ERR_NV_MALLOC_FAIL;
    }

    chn->attr.chnid = chnid;
    chn->attr.type = type;
    chn->attr.pa = dma_handle;
    chn->attr.va = addr;
    chn->attr.size = bufsize;
    chn->attr.cbk = cbk;
    chn->attr.cbk_arg = NULL;

    ret = bsp_fipc_chn_open(&chn->chn, &chn->attr);
    if (ret) {
        nv_dma_free(bufsize, chn->attr.va, chn->attr.pa);
        nv_printf("open nv fipc chan fail, ret:%d\n", ret);
        return ret;
    }

    return 0;
}

u32 nv_cpmsg_chan_init(struct nvm_msg_info *msg_info)
{
    u32 ret;
    struct nv_fipc_chan *chn = NULL;

    chn = &msg_info->recvchn;
    ret = (u32)nv_fipc_chan_init(chn, (fipc_cbk)nv_cpmsg_handle, FIPC_CHN_RECV_TSP2AP_MDRVNV, FIPC_CHN_TYPE_RECV,
        NV_FIPC_C2A_BUF_LEN);
    if (ret) {
        nv_printf("nv recv fipc init fail\n");
        return ret;
    }

    chn = &msg_info->sendchn;
    ret = (u32)nv_fipc_chan_init(chn, NULL, FIPC_CHN_SEND_AP2TSP_MDRVNV, FIPC_CHN_TYPE_SEND, NV_FIPC_A2C_BUF_LEN);
    if (ret) {
        nv_dma_free(NV_FIPC_C2A_BUF_LEN, msg_info->recvchn.attr.va, msg_info->recvchn.attr.pa);
        nv_printf("nv send fipc init fail\n");
        return ret;
    }

    return 0;
}
