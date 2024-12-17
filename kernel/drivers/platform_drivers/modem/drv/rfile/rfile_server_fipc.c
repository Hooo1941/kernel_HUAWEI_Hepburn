/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
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

#include "rfile_server.h"
#include "rfile_server_icc.h"
#include <linux/version.h>
#include <linux/device.h>
#include <linux/of_device.h>
#include <linux/dma-mapping.h>
#include <osl_malloc.h>
#include <bsp_print.h>
#include <bsp_version.h>
#include <bsp_fipc.h>
#include <securec.h>

#define THIS_MODU mod_rfile

struct rfile_fipc_handle {
    fipc_chn_t send_handle;
    fipc_chn_t recv_handle;
    void *send_va;
    void *send_pa;
    void *recv_va;
    void *recv_pa;
};
struct rfile_fipc_handle g_rfile_fipc_handle;

void rfile_dma_init(void)
{
    struct device *pdev = rfile_get_device();
    of_dma_configure(pdev, NULL, true);
    dma_set_mask_and_coherent(pdev, ~(0x1FULL)); // 32 bytes align

    return;
}

void *rfile_dma_alloc(u32 size, dma_addr_t *dma_handle)
{
    void *mem_addr = NULL;
    struct device *pdev = rfile_get_device();

    mem_addr = dma_alloc_coherent(pdev, size, dma_handle, GFP_KERNEL);
    if (IS_ERR_OR_NULL(mem_addr)) {
        bsp_err("<%s> alloc failed\n", __FUNCTION__);
        return NULL;
    }
    return mem_addr;
}

void rfile_dma_free(size_t size, void *cpu_addr, dma_addr_t dma_handle)
{
    struct device *pdev = rfile_get_device();
    dma_free_coherent(pdev, size, cpu_addr, dma_handle);
    return;
}

signed int rfile_fipc_cb(fipc_event event, void *arg, const fipc_eventinfo *event_info)
{
    bsp_debug("<%s> entry.\n", __FUNCTION__);

    if (event != FIPC_EVENT_DATA_ARRV) {
        bsp_err("<%s> fipc went wrong, event = %d.\n", __FUNCTION__, event);
        return -1;
    }
    if (event_info == NULL) {
        bsp_err("<%s> rfile fipc recv info NULL\n", __FUNCTION__);
        return -1;
    }
    if (event_info->data_arrv.chn_hdl != g_rfile_fipc_handle.recv_handle) {
        bsp_err("<%s> rfile fipc recv handle doesn't match \n", __FUNCTION__);
        return -1;
    }
    rfile_icc_common_cb();
    return 0;
}

int rfile_icc_common_recv(void *databuf, u32 datalen)
{
    return bsp_fipc_chn_recv(g_rfile_fipc_handle.recv_handle, databuf, datalen, 0);
}

int rfile_icc_common_send(void *databuf, u32 datalen)
{
    return bsp_fipc_chn_send(g_rfile_fipc_handle.send_handle, databuf, datalen, 0);
}

u32 rfile_icc_chn_len_max_atoc(void)
{
    const bsp_version_info_s *version_info = bsp_get_version_info();
    if ((version_info != NULL) && (version_info->product_type == PRODUCT_PHONE)) {
        return RFILE_LEN_MAX_ATOC_PHONE;
    }
    return RFILE_LEN_MAX_ATOC_MBB;
}

u32 rfile_icc_chn_len_max_ctoa(void)
{
    const bsp_version_info_s *version_info = bsp_get_version_info();
    if ((version_info != NULL) && (version_info->product_type == PRODUCT_PHONE)) {
        return RFILE_LEN_MAX_CTOA_PHONE;
    }
    return RFILE_LEN_MAX_CTOA_MBB;
}

int rfile_fipc_recvchn_init(void)
{
    fipc_chn_attr_t attr = { 0 };
    void *addr = NULL;
    int ret;
    dma_addr_t dma_handle;
    u32 buf_len = rfile_icc_chn_len_max_ctoa() + RFILE_FIPC_PAYLOAD;

    ret = bsp_fipc_chn_attr_init(&attr);
    if (ret) {
        bsp_err("<%s> bsp_fipc_chn_attr_init failed %d.\n", __FUNCTION__, ret);
        return -1;
    }
    addr = rfile_dma_alloc(buf_len, &dma_handle);
    if (addr == NULL) {
        bsp_err("<%s> rfile_dma_alloc failed.\n", __FUNCTION__);
        return -1;
    }
    attr.chnid = FIPC_CHN_RECV_TSP2AP_MDRVRFILE;
    attr.type = FIPC_CHN_TYPE_RECV;
    attr.pa = dma_handle;
    attr.va = addr;
    attr.size = buf_len;
    attr.cbk = rfile_fipc_cb;
    attr.cbk_arg = NULL;

    ret = bsp_fipc_chn_open(&g_rfile_fipc_handle.recv_handle, &attr);
    if (ret) {
        bsp_err("<%s> bsp_fipc_chn_open failed %d.\n", __FUNCTION__, ret);
        rfile_dma_free(buf_len, attr.va, attr.pa);
        return -1;
    }
    g_rfile_fipc_handle.recv_va = attr.va;
    g_rfile_fipc_handle.recv_pa = (void *)(uintptr_t)attr.pa;

    return 0;
}

int rfile_fipc_sendchn_init(void)
{
    fipc_chn_attr_t attr = { 0 };
    void *addr = NULL;
    int ret;
    dma_addr_t dma_handle;
    u32 buf_len = rfile_icc_chn_len_max_atoc() + RFILE_FIPC_PAYLOAD;

    ret = bsp_fipc_chn_attr_init(&attr);
    if (ret) {
        bsp_err("<%s> bsp_fipc_chn_attr_init failed %d.\n", __FUNCTION__, ret);
        return -1;
    }

    addr = rfile_dma_alloc(buf_len, &dma_handle);
    if (addr == NULL) {
        bsp_err("<%s> rfile_dma_alloc failed.\n", __FUNCTION__);
        return -1;
    }
    attr.chnid = FIPC_CHN_SEND_AP2TSP_MDRVRFILE;
    attr.type = FIPC_CHN_TYPE_SEND;
    attr.pa = dma_handle;
    attr.va = addr;
    attr.size = buf_len;
    attr.cbk = NULL;
    attr.cbk_arg = NULL;

    ret = bsp_fipc_chn_open(&g_rfile_fipc_handle.send_handle, &attr);
    if (ret) {
        bsp_err("<%s> bsp_fipc_chn_open failed %d.\n", __FUNCTION__, ret);
        rfile_dma_free(buf_len, attr.va, attr.pa);
        return -1;
    }
    g_rfile_fipc_handle.send_va = attr.va;
    g_rfile_fipc_handle.send_pa = (void *)(uintptr_t)attr.pa;

    return 0;
}

void rfile_fipc_sendchn_exit(void)
{
    u32 buf_len = rfile_icc_chn_len_max_atoc() + RFILE_FIPC_PAYLOAD;

    rfile_dma_free(buf_len, g_rfile_fipc_handle.send_va, (uintptr_t)g_rfile_fipc_handle.send_pa);
    return;
}

void rfile_fipc_recvchn_exit(void)
{
    u32 buf_len = rfile_icc_chn_len_max_ctoa() + RFILE_FIPC_PAYLOAD;

    rfile_dma_free(buf_len, g_rfile_fipc_handle.recv_va, (uintptr_t)g_rfile_fipc_handle.recv_pa);
    return;
}

void rfile_icc_common_exit(void)
{
    if (g_rfile_fipc_handle.send_va != NULL) {
        rfile_fipc_sendchn_exit();
        g_rfile_fipc_handle.send_va = NULL;
        g_rfile_fipc_handle.send_pa = NULL;
    }
    if (g_rfile_fipc_handle.recv_va != NULL) {
        rfile_fipc_recvchn_exit();
        g_rfile_fipc_handle.recv_va = NULL;
        g_rfile_fipc_handle.recv_pa = NULL;
    }
}

int rfile_icc_common_init(void)
{
    rfile_dma_init();
    if (rfile_fipc_sendchn_init()) {
        return -1;
    }
    if (rfile_fipc_recvchn_init()) {
        rfile_icc_common_exit();
        return -1;
    }

    return 0;
}
