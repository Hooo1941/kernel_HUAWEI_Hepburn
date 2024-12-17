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

#include <securec.h>
#include <product_config.h>
#include <linux/dma-mapping.h>
#include <linux/of_platform.h>
#include <asm/dma-mapping.h>
#include <asm/cacheflush.h>
#include "osl_malloc.h"
#include "osl_thread.h"
#include "bsp_odt.h"
#include "sys_odt.h"
#include "sys_bus_api.h"

bool g_odt_press_start_flag = false;
u32 g_odt_press_delay = 10;
u32 g_odt_enc_send_cnt = 0;
struct odt_buffer g_sys_odt_buffer = { 0 };

unsigned long odt_press_malloc(u32 size, dma_addr_t *phy_addr)
{
    dma_addr_t address = 0;
    unsigned long vir_addr;
    struct platform_device *pdev = (struct platform_device *)bsp_odt_get_pdev_addr();

    vir_addr = (uintptr_t)((u8 *)dma_alloc_coherent(&pdev->dev, (size_t)size, &address, GFP_KERNEL));
    if (vir_addr == 0) {
        printk(KERN_ERR "[%s] dma_alloc_coherent fail!\n", __FUNCTION__);
        return 0;
    }

    *phy_addr = address;
    return vir_addr;
}

void odt_press_free(unsigned long *vir_addr, const dma_addr_t *phy_addr, u32 size)
{
    struct platform_device *pdev = (struct platform_device *)bsp_odt_get_pdev_addr();

    dma_free_coherent(&pdev->dev, (size_t)size, (void *)vir_addr, *phy_addr);
    return;
}

struct odt_press_case g_odt_press = { 0,
    0x3,
    ODT_CHAN_DEF(ODT_CODER_DEST_CHAN, 0x3),
    0,
    ODT_DATA_TYPE_0,
    ODT_ENCSRC_CHNMODE_LIST,
    ODT_CHAN_PRIORITY_0,
    NULL,
    0xf000,
    NULL,
    0xf000,
    0x8,
    NULL,
    0x20000,
    0x4,
    NULL,
    NULL,
    NULL,
    0,
    0x1000,
    0x64,
    ODT_DATA_TYPE_EN,
    ODT_ENC_DEBUG_DIS,
    0x1FA0
};

void odt_press_set_encdst(struct odt_press_case *press_case)
{
    odt_dst_chan_cfg_s enc_dst_attr;

    if (press_case->dst_set == BSP_TRUE) {
        printk(KERN_ERR "[%s] Enc Dest Set Twice!\n", __FUNCTION__);
        return;
    }

    g_sys_odt_buffer.dst_vir_addr = odt_press_malloc(press_case->output_size, &g_sys_odt_buffer.dst_phy_addr);
    if (g_sys_odt_buffer.dst_vir_addr == 0) {
        printk(KERN_ERR "%s[%d] ALLOC ENC DST FAILED!\n", __FUNCTION__, __LINE__);
        return;
    }

    press_case->output_start = (u8 *)(uintptr_t)g_sys_odt_buffer.dst_vir_addr;
    enc_dst_attr.coder_dst_setbuf.output_start = (u8 *)(uintptr_t)g_sys_odt_buffer.dst_phy_addr;
    enc_dst_attr.coder_dst_setbuf.output_end =
        (u8 *)(uintptr_t)(g_sys_odt_buffer.dst_phy_addr + press_case->output_size - 1);

    enc_dst_attr.coder_dst_setbuf.threshold = 0x2000;
    enc_dst_attr.encdst_thrh = 0x2000;

    if (BSP_OK != bsp_odt_coder_set_dst_chan(press_case->dst_chan_no, &enc_dst_attr)) {
        printk("set enc dst channel failed!\n");

        return;
    }

    bsp_odt_encdst_dsm_init(press_case->dst_chan_no, ODT_DEST_DSM_ENABLE);

    bsp_odt_dst_channel_enable(press_case->dst_chan_no);

    press_case->dst_set = BSP_TRUE;

    return;
}

static s32 odt_press_config_chan(struct odt_press_case *press_case)
{
    u8 *packet = NULL;
    odt_bd_data_s bd_data = { 0 };
    u32 len = 0x1000;
    dma_addr_t phy_addr;
    u32 i;
    s32 ret;

    (void)bsp_odt_register_read_cb(press_case->dst_chan_no, press_case->read_cb);
    if (press_case->mode == ODT_ENCSRC_CHNMODE_LIST) {
        (void)bsp_odt_register_rd_cb(press_case->src_chan_no, press_case->rd_cb);
    }

    if (bsp_odt_start(press_case->src_chan_no)) {
        printk("%s(%d): bsp_odt_start chan=0x%x\n", __FUNCTION__, __LINE__, press_case->src_chan_no);
        return BSP_ERROR;
    }

    packet = (u8 *)(uintptr_t)odt_press_malloc(len, &phy_addr);
    if (packet == NULL) {
        printk(KERN_ERR "packet malloc mem fail, len = 0x%x.\n", len);
        return BSP_ERROR;
    }
    (void)memset_s(packet, len, 0xa5, len);

    bd_data.data_addr = (unsigned long)phy_addr;
    bd_data.data_len = (unsigned short)len;
    bd_data.data_type = ODT_BD_DATA;
    printk("bd_data.data_addr phy = 0x%lx. vir %lx\n", (uintptr_t)bd_data.data_addr, (uintptr_t)packet);

    for (i = 0; i < press_case->input_size / sizeof(bd_data); i++) {
        ret = memcpy_s(press_case->input_start + i * sizeof(bd_data), press_case->input_size - i * sizeof(bd_data),
            &bd_data, sizeof(bd_data));
        if (ret) {
            odt_press_free((unsigned long *)packet, (const dma_addr_t *)&phy_addr, len);
            return ret;
        }
    }

    return BSP_OK;
}

static void odt_press_send_count(void)
{
    g_odt_enc_send_cnt++;
    if (g_odt_enc_send_cnt % 0x400 == 0x3FF) {
        printk("encoder stress send cnt is %d\n", g_odt_enc_send_cnt);
    }
    if (g_odt_enc_send_cnt > 0xFFFFFFF0) {
        g_odt_enc_send_cnt = 0;
    }
}

s32 odt_press_encode_task(void *param)
{
    struct odt_press_case *press_case = (struct odt_press_case *)param;
    u32 len;
    odt_buffer_rw_s wt_buff;
    s32 ret;

    ret = odt_press_config_chan(press_case);
    if (ret) {
        return ret;
    }

    while (g_odt_press_start_flag) {
        if (bsp_odt_get_rd_buffer(press_case->src_chan_no, &wt_buff)) {
            continue;
        }

        if (wt_buff.size + wt_buff.rb_size) {
            (void)bsp_odt_read_rd_done(press_case->src_chan_no, (wt_buff.size + wt_buff.rb_size));
        }

        if (bsp_odt_get_write_buff(press_case->src_chan_no, &wt_buff)) {
            continue;
        }

        len = 0;
        if (wt_buff.size + wt_buff.rb_size >= sizeof(odt_bd_data_s)) {
            if (wt_buff.size >= sizeof(odt_bd_data_s)) {
                len = wt_buff.size;
            }
            if (wt_buff.rb_size >= sizeof(odt_bd_data_s)) {
                len += wt_buff.rb_size;
            }

            if (len <= sizeof(odt_bd_data_s)) {
                continue;
            }

            len -= len % sizeof(odt_bd_data_s);
            len -= sizeof(odt_bd_data_s);
            if (bsp_odt_write_done(press_case->src_chan_no, len)) {
                continue;
            }
        } else {
            msleep(g_odt_press_delay);
        }

        odt_press_send_count();
        msleep(g_odt_press_delay);
    }

    return BSP_OK;
}


void odt_press_setencsrc(struct odt_press_case *press_case)
{
    char task_name[0x20] = {0};
    odt_src_chan_cfg_s encsrc_attr = {0};
    s32 ret;

    g_sys_odt_buffer.src_vir_addr = odt_press_malloc(press_case->input_size, &g_sys_odt_buffer.src_phy_addr);
    if (g_sys_odt_buffer.src_vir_addr == 0) {
        printk(KERN_ERR "%s[%d]:ALLOC ENC SRC FAILED!\n", __FUNCTION__, __LINE__);
        return;
    }

    if (press_case->mode == ODT_ENCSRC_CHNMODE_LIST) {
        g_sys_odt_buffer.rd_vir_addr = odt_press_malloc(press_case->rd_size, &g_sys_odt_buffer.rd_phy_addr);
        if (g_sys_odt_buffer.rd_vir_addr == 0) {
            printk(KERN_ERR "%s[%d] ALLOC ENC SRC RD BUF FAILED!\n", __FUNCTION__, __LINE__);
            return;
        }

        press_case->rd_start = (u8 *)(uintptr_t)g_sys_odt_buffer.rd_vir_addr;
    }

    press_case->input_start = (u8 *)(uintptr_t)g_sys_odt_buffer.src_vir_addr;

    encsrc_attr.data_type_en = press_case->data_type_en;
    encsrc_attr.mode = press_case->mode;
    encsrc_attr.priority = press_case->priority;
    encsrc_attr.dest_chan_id = press_case->dst_chan_no;
    encsrc_attr.coder_src_setbuf.input_start = (u8 *)(uintptr_t)g_sys_odt_buffer.src_phy_addr;
    encsrc_attr.coder_src_setbuf.input_end =
        (u8 *)(uintptr_t)(g_sys_odt_buffer.src_phy_addr + press_case->input_size - 1);

    if (press_case->mode == ODT_ENCSRC_CHNMODE_LIST) {
        encsrc_attr.coder_src_setbuf.rd_input_start = (u8 *)(uintptr_t)g_sys_odt_buffer.rd_phy_addr;
        encsrc_attr.coder_src_setbuf.rd_input_end =
            (u8 *)(uintptr_t)(g_sys_odt_buffer.rd_phy_addr + press_case->rd_size - 1);
        encsrc_attr.coder_src_setbuf.rd_threshold = press_case->threshold;
    }

    if ((ret = bsp_odt_coder_set_src_chan(press_case->src_chan_no, &encsrc_attr)) != BSP_OK) {
        printk(KERN_ERR "alloc enc src channel failed, ret = 0x%x!\n", ret);
        return;
    }

    if (snprintf_s(task_name, sizeof(task_name), 0x1F, "sysbus_odt") < 0) {
        return;
    }

    if (osl_task_init(task_name, 0x14, 0x1000, (OSL_TASK_FUNC)odt_press_encode_task, press_case,
        (OSL_TASK_ID *)&press_case->task_id) != BSP_OK) {
        return;
    }
    printk(KERN_ERR "[%s]  ok!\n", __FUNCTION__);

    return;
}

void sys_odt_start(void)
{
    g_odt_press_start_flag = true;

    bsp_odt_set_stress_flag(ODT_STRESS_FLAG);

    odt_press_set_encdst(&g_odt_press);
    odt_press_setencsrc(&g_odt_press);
}
void sys_odt_stop(void)
{
    odt_buffer_rw_s wt_buff;

    if (g_odt_press_start_flag == false) {
        printk("[%s] bus stress is stopped or do not start!\n", __FUNCTION__);
        return;
    }

    g_odt_press_start_flag = false;
    bsp_odt_set_stress_flag(0);

    if (bsp_odt_get_read_buff(g_odt_press.dst_chan_no, &wt_buff)) {
        printk("[%s] bsp_odt_get_read_buff failed!\n", __FUNCTION__);
        return;
    }

    if ((wt_buff.rb_size + wt_buff.size) == 0) {
        printk("[%s] (wt_buff.rb_size+wt_buff.size) ==0 !\n", __FUNCTION__);
        return;
    }

    if (bsp_odt_read_data_done(g_odt_press.dst_chan_no, (wt_buff.rb_size + wt_buff.size))) {
        printk("[%s] bsp_odt_get_read_buff failed!\n", __FUNCTION__);
        return;
    }
    g_odt_press.rd_start = NULL;

    g_odt_press.dst_set = false;
    bsp_odt_stop(g_odt_press.src_chan_no);
    bsp_odt_free_channel(g_odt_press.src_chan_no);

    bsp_odt_free_enc_dst_chan(g_odt_press.dst_chan_no);
    if (g_sys_odt_buffer.src_phy_addr != 0) {
        odt_press_free(&g_sys_odt_buffer.src_vir_addr, &g_sys_odt_buffer.src_phy_addr, g_odt_press.input_size);
    }
    if (g_sys_odt_buffer.dst_phy_addr != 0) {
        odt_press_free(&g_sys_odt_buffer.dst_vir_addr, &g_sys_odt_buffer.dst_phy_addr, g_odt_press.output_size);
    }
    if (g_sys_odt_buffer.rd_phy_addr != 0) {
        odt_press_free(&g_sys_odt_buffer.rd_vir_addr, &g_sys_odt_buffer.rd_phy_addr, g_odt_press.rd_size);
    }
}

void sys_odt_report(void)
{
    return;
}

int sys_odt_entry(sys_bus_evt_e event)
{
    sys_bus_err("[sys_bus]odt, rcv evt: %d\n", event);

    if (event == E_SYS_BUS_EVT_START) {
        sys_odt_start();
    } else if (event == E_SYS_BUS_EVT_STOP) {
        sys_odt_stop();
    } else if (event == E_SYS_BUS_EVT_REPORT) {
        sys_odt_report();
    } else {
        sys_bus_err("receive incorrect event: %d\n", event);
        return E_SYS_BUS_RET_ERR;
    }

    return E_SYS_BUS_RET_OK;
}

void sys_odt_init(struct device_node *np)
{
    sys_bus_register_observer(SYS_BUS_ODT_NAME, sys_odt_entry);
}
