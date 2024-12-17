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

#include "bbpds.h"
#include <linux/errno.h>
#include <linux/kthread.h>
#include <linux/module.h>
#include "securec.h"
#include "osl_malloc.h"
#include <mdrv_memory_layout.h>
#include "product_config.h"
#include "msg_id.h"
#ifdef BSP_CONFIG_PHONE_TYPE
#include <adrv_om.h>
#endif
#include "bsp_dt.h"
#include "bsp_bbpds.h"


#define THIS_MODU mod_bbpds

bbpds_resv_mem_info_s g_bbpds_reserved_mem;

static s32 bbpds_task_init(void)
{
    struct task_struct *task = NULL;
    s32 ret;

    task = kthread_create(bbpds_proc_task, NULL, "bbpds_task");
    if (IS_ERR(task)) {
        bbpds_error("bbpds_proc_task create fail\n");
        return BSP_ERROR;
    }

    /* create msg queue */
    ret = mdrv_msg_qcreate(task->pid, MSG_QNUM_BBPDS_APSS);
    if (ret) {
        bbpds_error("create queue fail, ret=%x\n", ret);
        return ret;
    }

    /* register mid for msg task */
    ret = mdrv_msg_register_mid(task->pid, DRV_MID_BBPDS_APSS, MSG_QID_BBPDS_APSS);
    if (ret) {
        bbpds_error("register msg mid fail, ret=%x\n", ret);
        return ret;
    }

    wake_up_process(task);

    return BSP_OK;
}

#ifdef BSP_CONFIG_PHONE_TYPE
static s32 bbpds_type_phone_init(void)
{
    s32 ret;

    if (get_mdm_bbpds_mem_flag()) {
        g_bbpds_reserved_mem.phy_addr = mdrv_mem_region_get("odt_bbpds_ddr", &g_bbpds_reserved_mem.buf_size);
        if (g_bbpds_reserved_mem.phy_addr == 0 || g_bbpds_reserved_mem.buf_size == 0) {
            g_bbpds_reserved_mem.buf_usable = BSP_FALSE;
        } else {
            g_bbpds_reserved_mem.buf_usable = BSP_TRUE;
        }
    }

    ret = bbpds_task_init();
    if (ret) {
        bbpds_error("[init] task init fail, ret = 0x%x\n", ret);
        return ret;
    }

    bbpds_crit("[init] ok, ret = 0x%x\n", ret);
    return BSP_OK;
}
#else
static s32 bbpds_type_mbb_init(void)
{
    s32 ret;

    bbpds_buf_init();

    ret = bbpds_task_init();
    if (ret) {
        bbpds_error("[init] task init fail, ret = 0x%x\n", ret);
        return ret;
    }

    bbpds_crit("[init] ok, ret = 0x%x\n", ret);
    return BSP_OK;
}
#endif

/*
 * 函 数 名: bbpds_init
 * 功能描述: 模块初始化函数
 * 输入参数: 无
 * 输出参数: 无
 * 返 回 值: 初始化成功的标识码
 */
s32 bbpds_init(void)
{
#ifdef BSP_CONFIG_PHONE_TYPE
    return bbpds_type_phone_init();
#else
    return bbpds_type_mbb_init();
#endif
}

int bbpds_proc_task(void *data)
{
    bbpds_msg_info_s *msg = NULL;
    void *msg_get = NULL;
    s32 ret;

    for (;;) {
        ret = mdrv_msg_recv(MSG_QID_BBPDS_APSS, &msg_get, 0xffffffff);
        if (ret) {
            bbpds_error("recv msg fail, ret=0x%x\n", ret);
            continue;
        }

        if (msg_get == NULL) {
            bbpds_error("msg recv null\n");
            continue;
        }

        msg = (bbpds_msg_info_s *)msg_get;
        if (msg->src_id == DRV_MID_BBPDS) {
            bbpds_msg_proc(msg);
        } else {
            bbpds_error("unknown msg src id, src_id=0x%x\n", msg->src_id);
        }

        mdrv_msg_free(DRV_MID_BBPDS_APSS, (void *)msg);
    }

    return 0;
}

void bbpds_msg_proc(bbpds_msg_info_s *msg)
{
    u32 msg_id;

    msg_id = ((tsp_bbpds_info_s *)msg->ds_req)->msg_id;
    if (msg_id == BBPDS_GET_DATA_MEM) {
        bbpds_send_mem_info();
    } else {
        bbpds_warning("bbpds msg id invalid, msg_id=0x%x\n", msg_id);
    }

    return;
}

void bbpds_send_mem_info(void)
{
    bbpds_data_mem_info_s bbpds_mem_info;
    tsp_bbpds_info_s *bbpds_msg;
    s32 ret;
    u32 len;

    len = sizeof(tsp_bbpds_info_s) + sizeof(bbpds_data_mem_info_s);
    bbpds_msg = (tsp_bbpds_info_s *)osl_malloc(len);
    if (bbpds_msg == NULL) {
        bbpds_error("osl_malloc fail\n");
        return;
    }

    bbpds_mem_info.mem_useable = g_bbpds_reserved_mem.buf_usable;
    bbpds_mem_info.mem_addr = g_bbpds_reserved_mem.phy_addr;
    bbpds_mem_info.mem_size = g_bbpds_reserved_mem.buf_size;

    bbpds_msg->msg_id = BBPDS_GET_DATA_MEM;
    ret = memcpy_s(bbpds_msg->data, sizeof(bbpds_data_mem_info_s), &bbpds_mem_info, sizeof(bbpds_data_mem_info_s));
    if (ret) {
        bbpds_error("memcpy_s fail, ret=0x%x\n", ret);
        osl_free(bbpds_msg);
        return;
    }

    ret = bbpds_msg_send(bbpds_msg, len, DRV_MID_BBPDS_APSS, DRV_MID_BBPDS);
    if (ret) {
        bbpds_error("send msg fail, ret=0x%x\n", ret);
    }

    osl_free(bbpds_msg);
    return;
}

s32 bbpds_msg_send(const void *data, u32 len, u32 send_mid, u32 recv_mid)
{
    bbpds_msg_info_s *msg = NULL;
    s32 ret;

    msg = (bbpds_msg_info_s *)mdrv_msg_alloc(send_mid, len + sizeof(bbpds_msg_info_s), MSG_FULL_LEN);
    if (msg == NULL) {
        bbpds_error("msg alloc fail\n");
        return BSP_ERROR;
    }

    MDRV_MSG_HEAD_FILL(msg, send_mid, recv_mid, len);

    ret = memcpy_s(msg->ds_req, len, data, len);
    if (ret) {
        bbpds_error("memcpy_s fail, ret=0x%x\n", ret);
        mdrv_msg_free(send_mid, msg);
        return ret;
    }

    ret = mdrv_msg_send(msg, 0);
    if (ret) {
        bbpds_error("msg send fail,ret=0x%x\n", ret);
        mdrv_msg_free(send_mid, msg);
        return ret;
    }

    return BSP_OK;
}

void bbpds_buf_init(void)
{
    unsigned long addr;
    u32 size = 0;

    addr = mdrv_mem_region_get("odt_bbpds_ddr", &size);
    if (addr == 0 || size == 0) {
        g_bbpds_reserved_mem.buf_usable = BSP_FALSE;
        g_bbpds_reserved_mem.phy_addr = 0;
        g_bbpds_reserved_mem.buf_size = 0;
    } else {
        g_bbpds_reserved_mem.buf_usable = BSP_TRUE;
        g_bbpds_reserved_mem.phy_addr = addr;
        g_bbpds_reserved_mem.buf_size = size;
    }
}

u32 bsp_bbpds_get_mem_flag(void)
{
    return g_bbpds_reserved_mem.buf_usable;
}

#if !IS_ENABLED(CONFIG_MODEM_DRIVER)
module_init(bbpds_init);
#endif


