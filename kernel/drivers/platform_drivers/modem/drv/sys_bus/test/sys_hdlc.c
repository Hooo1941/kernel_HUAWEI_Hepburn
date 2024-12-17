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

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include "sys_hdlc.h"
#include <sys_bus_api.h>
#include <bsp_dt.h>

sys_hdlc_ctrl g_sys_hdlc = { 0 };

inline sys_hdlc_ctrl* hdlc_sys_get_ctrl(void)
{
    return &g_sys_hdlc;
}

void sys_hdlc_frm_cfg_reg(sys_hdlc_ctrl *ctrl)
{
    unsigned int frm_pfc, frm_acfc, frm_cfg_value, frm_accm, prior_timeout_ctrl_value;
    /* 帧的协议类型 */
    int frm_protocol = 0x805b;

    frm_pfc = 1;
    frm_acfc = 1;
    frm_cfg_value = ((frm_protocol) << 16) | (frm_pfc << 2) | (frm_acfc << 1) | FRM_IN_LLI_1DOR2D_VALUE;
    frm_accm = 0xffffffff;
    prior_timeout_ctrl_value = (0xFF << 16);
    writel(frm_cfg_value, (char *)ctrl->hdlc_reg_base + HDLC_FRM_CFG);
    writel(frm_accm, (char *)ctrl->hdlc_reg_base + HDLC_FRM_ACCM);
    writel(ctrl->hdlc_buf_addr_pa + FRM_IN_LLI_OFFSET, (char *)ctrl->hdlc_reg_base + FRM_IN_LLI_ADDR);
    writel(ctrl->hdlc_buf_addr_pa + FRM_OUT_LLI_OFFSET, (char *)ctrl->hdlc_reg_base + FRM_OUT_LLI_ADDR);
    writel(ctrl->hdlc_buf_addr_pa + FRM_RPT_OFFSET, (char *)ctrl->hdlc_reg_base + FRM_RPT_ADDR);
    writel(FRM_DEF_REPORT_DEPTH, (char *)ctrl->hdlc_reg_base + FRM_RPT_DEP);

    writel(prior_timeout_ctrl_value, (char *)ctrl->hdlc_reg_base + PRIOR_TIMEOUT_CTRL);
    writel(0xFFFFFFFF, (char *)ctrl->hdlc_reg_base + HDLC_FRM_INT_CLR);
    writel(0xFFFFFFFF, (char *)ctrl->hdlc_reg_base + HDLC_DEF_INT_CLR);
    writel(0x1, (char *)ctrl->hdlc_reg_base + HDLC_FRM_EN);
}

void sys_hdlc_def_cfg_reg(sys_hdlc_ctrl *ctrl)
{
    unsigned int frm_pfc, frm_acfc, def_cfg_value, prior_timeout_ctrl_value;
    frm_pfc = 1;
    frm_acfc = 1;
    def_cfg_value = (frm_pfc << 2) | (frm_acfc << 1);
    prior_timeout_ctrl_value = (0xFF << 16);
    writel(def_cfg_value, (char *)ctrl->hdlc_reg_base + HDLC_DEF_CFG);
    writel(ctrl->hdlc_buf_addr_pa + DEF_IN_LLI_OFFSET, (char *)ctrl->hdlc_reg_base + DEF_IN_LLI_ADDR);
    writel(DEF_IN_PKT_LEN_MAX_VALUE, (char *)ctrl->hdlc_reg_base + DEF_IN_PKT_LEN_MAX);
    writel(ctrl->hdlc_out_addr_pa, (char *)ctrl->hdlc_reg_base + DEF_OUT_SPC_ADDR);
    writel(ctrl->hdlc_buf_addr_pa + DEF_RPT_OFFSET, (char *)ctrl->hdlc_reg_base + DEF_RPT_ADDR);
    writel(FRM_DEF_REPORT_DEPTH, (char *)ctrl->hdlc_reg_base + DEF_RPT_DEP);
    writel(DEF_OUT_SPC_DEPTH,  (char *)ctrl->hdlc_reg_base + DEF_OUT_SPC_DEP);
    writel(prior_timeout_ctrl_value, (char *)ctrl->hdlc_reg_base + PRIOR_TIMEOUT_CTRL);
    writel(0xFFFFFFFF, (char *)ctrl->hdlc_reg_base + HDLC_FRM_INT_CLR);
    writel(0xFFFFFFFF, (char *)ctrl->hdlc_reg_base + HDLC_DEF_INT_CLR);
    writel(0x1, (char *)ctrl->hdlc_reg_base + HDLC_DEF_EN);
}

int sys_hdlc_stress_test(sys_hdlc_ctrl *ctrl)
{
    int ulLoop;
    int ulLoop2;
    int ulFrmDataLen = UL_FRM_DATA_LEN; // IP包数据长度
    unsigned int checkaddr_pa;

    /* frame */
    unsigned int frm_stat;
    unsigned int *frm_in_lli = NULL;   // 输入链表的基地址
    unsigned int *frm_rpt = NULL;      // 报告链表的基地址
    unsigned int *frm_out_lli = NULL;  // 输出链表的基地址
    unsigned int *ulDataAddr = NULL;

    /* deframe */
    unsigned int def_stat;
    unsigned int *def_in_lli = NULL;   // 输入链表的基地址
    unsigned int *def_rpt = NULL;      // 报告链表的基地址
    unsigned int *def_out = NULL;      // 解封装后有效帧所在的基地址
    unsigned int *ulCheckAddr = NULL;

    /* frame */
    frm_in_lli = (unsigned int *)((char *)ctrl->hdlc_buf_ptr + FRM_IN_LLI_OFFSET);
    frm_rpt = (unsigned int *)((char *)ctrl->hdlc_buf_ptr + FRM_RPT_OFFSET);
    frm_out_lli = (unsigned int *)((char *)ctrl->hdlc_buf_ptr + FRM_OUT_LLI_OFFSET);

    /* deframe */
    def_in_lli = (unsigned int *)((char *)ctrl->hdlc_buf_ptr + DEF_IN_LLI_OFFSET);
    def_rpt = (unsigned int *)((char *)ctrl->hdlc_buf_ptr + DEF_RPT_OFFSET);
    def_out = (unsigned int *)((char *)ctrl->hdlc_out_ptr);

    /* 设置封装输入链表 */
    for (ulLoop = 0; ulLoop < 10; ulLoop++) {
        /* node数组中是20个指针，指向不同的buffer，前十块buf用于IP包的封装，后十块buf用于有效帧的解封装 */
        ulDataAddr = ctrl->hdlc_node_ptr[ulLoop];
        for (ulLoop2 = 0; ulLoop2 < ulFrmDataLen / 4; ulLoop2++) {
            *(ulDataAddr + ulLoop2) = (unsigned int)0x12345678U;
        }
        /* HDLC master访存，输入链表中必须存入IP包所在buffer的物理地址 */
        *(frm_in_lli + 3 * ulLoop) = ctrl->hdlc_node_addr_pa[ulLoop];
        *(frm_in_lli + 3 * ulLoop + 1) = ulFrmDataLen;
        *(frm_in_lli + 3 * ulLoop + 2) = ctrl->hdlc_buf_addr_pa + FRM_IN_LLI_OFFSET + 12 * ulLoop + 12;
    }
    /* 输入链表的最后一个节点的下一个节点地址为0，表示链表结束 */
    *(frm_in_lli + 3 * ulLoop - 1) = 0;

     /* 设置封装输出链表 */
    for (ulLoop = 0; ulLoop < 10; ulLoop++) {
        *(frm_out_lli + 3 * ulLoop) =  ctrl->hdlc_node_addr_pa[ulLoop + 10];
        *(frm_out_lli + 3 * ulLoop + 1) = 1536;
        *(frm_out_lli + 3 * ulLoop + 2) = ctrl->hdlc_buf_addr_pa + FRM_OUT_LLI_OFFSET + 12 * ulLoop + 12;
    }
    *(frm_out_lli + 3 * ulLoop - 1) =  0;

    /* 配置封装寄存器 */
    sys_hdlc_frm_cfg_reg(ctrl);

    /* 等待封装完成 */
    frm_stat = 0x0;
    while((frm_stat & 0x00000003) == 0x0) {
        frm_stat = readl((char *)ctrl->hdlc_reg_base + HDLC_FRM_STATUS);
    }

    /* 检查封装结果 */
    if ((frm_stat & 0x00000003) != 0x1) // 1bit:封装发生错误  0bit:成功完成一套链表数据操作
    {
        sys_bus_trace("frm status err, frm_stat:%d\n", frm_stat);
        return SYS_HDLC_ERR;
    }

    if ((((frm_stat >> 8) & 0xff) != 10) || (((frm_stat >> 16) & 0xffff) != 10))
    {
        sys_bus_trace("frm output err, frm_stat:%d\n", frm_stat);
        return SYS_HDLC_ERR;
    }

    /* 设置解封装输入链表 */
    for (ulLoop = 0; ulLoop < 10; ulLoop++) {
        *(def_in_lli + 3 * ulLoop) = ctrl->hdlc_node_addr_pa[ulLoop + 10];
        *(def_in_lli + 3 * ulLoop + 1) = *(frm_rpt + 2 * ulLoop + 1); // frm_rpt指向封包后的report结构体数组，每个结构体占8个字节(4-6字节代表输出帧长度,这里作为解封装的输入有效帧长度)
        *(def_in_lli + 3 * ulLoop + 2) = ctrl->hdlc_buf_addr_pa + DEF_IN_LLI_OFFSET + 12 * ulLoop + 12;
    }
    *(def_in_lli + 3 * ulLoop - 1) = 0;

    /* 配置解封装寄存器 */
    sys_hdlc_def_cfg_reg(ctrl);

    /* 等待硬件解封完成 */
    def_stat = 0x0;
    while((def_stat & 0x0000003F) == 0x0) {
        def_stat = readl((char *)ctrl->hdlc_reg_base + HDLC_DEF_STATUS);
    }

    /* 检查解封装结果 */
    if ((def_stat & 0x0000003F) != 0x3) {
        sys_bus_trace("def status err, def_stat:%d\n", def_stat);
        return SYS_HDLC_ERR;
    }
    if (((def_stat >> 8) & 0xffff) != 10) {
        sys_bus_trace("def output err, def_stat:%d\n", def_stat);
        return SYS_HDLC_ERR;
    }
    /* 对比解封装数据是否符合预期 */
    for (ulLoop = 0; ulLoop < 10; ulLoop++) {
        ulDataAddr = ctrl->hdlc_node_ptr[ulLoop];
        checkaddr_pa = *(def_rpt + 2 * ulLoop + 1);
        ulCheckAddr = (unsigned int *)((char *)ctrl->hdlc_out_ptr + (checkaddr_pa - ctrl->hdlc_out_addr_pa)); // 根据相对于物理地址的偏移，找到虚拟地址所在位置
        for (ulLoop2 = 0; ulLoop2 < ulFrmDataLen / 4; ulLoop2++) {
            if (*(ulDataAddr + ulLoop2) != *(ulCheckAddr + ulLoop2)) {
                return SYS_HDLC_ERR;
            }
        }
    }

    sys_bus_trace("run hdlc task successfully!\n");
    return SYS_HDLC_OK;
}

void sys_hdlc_report(void) {}

void sys_hdlc_start(void)
{
    sys_bus_trace("[sys_bus]hdlc, task start\n");
    g_sys_hdlc.run_flag = 1;
}

void sys_hdlc_stop(void)
{
    sys_bus_trace("[sys_bus]hdlc, task stop\n");
    g_sys_hdlc.run_flag = 0;
}

void sys_hdlc_thread(sys_hdlc_ctrl *ctrl)
{
    while (ctrl->run_flag) {
        sys_hdlc_stress_test(ctrl);
        msleep(SYS_HDLC_STOP_DELAY);
    }
}

int sys_hdlc_entry(sys_bus_evt_e event)
{
    sys_bus_trace("[sys_bus]hdlc, rcv evt: %d\n", event);

    if (event == E_SYS_BUS_EVT_START) {
        sys_hdlc_start();
    } else if (event == E_SYS_BUS_EVT_STOP) {
        sys_hdlc_stop();
    } else if (event == E_SYS_BUS_EVT_REPORT) {
        sys_hdlc_report();
    } else {
        sys_bus_trace("[sys_bus]hdlc receive incorrect event: %d\n", event);
        return E_SYS_BUS_RET_ERR;
    }

    return E_SYS_BUS_RET_OK;
}

static int sys_hdlc_uncache_buf_init(sys_hdlc_ctrl *ctrl)
{
    int ulLoop;
    dma_addr_t dma_handle;
    struct device dev1;

    sys_bus_trace("sys_hdlc_init enter.\n ");
    if (memset_s(&dev1, sizeof(dev1), 0, sizeof(dev1))) {
        sys_bus_trace("memset fail.\n ");
        return -1;
    }
    of_dma_configure(&dev1, NULL, true);
    arch_setup_dma_ops(&dev1, 0, 0, NULL, 0);
    dma_set_mask_and_coherent(&dev1, DMA_BIT_MASK(32));

    /* 储存封装、解封装过程中各个链表 */
    ctrl->hdlc_buf_ptr = (unsigned int *)dma_alloc_coherent(&dev1, HDLC_BUF_PTR_LEN, &dma_handle, GFP_KERNEL);
    if (ctrl->hdlc_buf_ptr == NULL) {
        sys_bus_trace("alloc hdlc_buf_ptr failed\n ");
        return -1;
    }
    ctrl->hdlc_buf_addr_pa = (unsigned int)dma_handle;

    /* 储存解封装后的数据包 */
    ctrl->hdlc_out_ptr = (unsigned int *)dma_alloc_coherent(&dev1, HDLC_OUT_PTR_LEN, &dma_handle, GFP_KERNEL);
    if (ctrl->hdlc_out_ptr == NULL) {
        sys_bus_trace("alloc hdlc_out_ptr failed\n ");
        return -1;
    }
    ctrl->hdlc_out_addr_pa = (unsigned int)dma_handle;

    /* 储存封装过程中的输入IP包和输出有效帧（同时作为解封装的输入有效帧） */
    for (ulLoop = 0; ulLoop < 20; ulLoop++) {
        ctrl->hdlc_node_ptr[ulLoop] = (unsigned int *)dma_alloc_coherent(&dev1, HDLC_NODE_PTR_LEN, &dma_handle, GFP_KERNEL);
        if (ctrl->hdlc_node_ptr[ulLoop] == NULL) {
            sys_bus_trace("alloc hdlc_node_ptr failed\n ");
            return -1;
        }
        ctrl->hdlc_node_addr_pa[ulLoop] = (unsigned int)dma_handle;
    }
    return 0;
}

void sys_hdlc_sysbus_test_init(void)
{
    int ret;
    sys_hdlc_ctrl *ctrl = hdlc_sys_get_ctrl();
    if (ctrl == NULL) {
        return;
    }

    ctrl->hdlc_reg_base = ioremap(HDLC_BASE_ADDR, sizeof(u32));
    if (ctrl->hdlc_reg_base == NULL) {
        sys_bus_trace("[sys_hdlc_test]ioremap failed!");
        return;
    }

    /* 申请uncache内存用于测试 */
    ret = sys_hdlc_uncache_buf_init(ctrl);
    if (ret) {
        return;
    }

    ctrl->tsk = kthread_run((void *)sys_hdlc_thread, (sys_hdlc_ctrl *)ctrl, "sys_hdlc");
    if (IS_ERR(ctrl->tsk)) {
        ctrl->tsk = NULL;
        sys_bus_trace("cannot start sys hdlc thread\n");
        return;
    }

    sys_bus_register_observer(SYS_HDLC_NAME, sys_hdlc_entry);

    sys_bus_trace("sys bus test hdlc ok\n");
}
