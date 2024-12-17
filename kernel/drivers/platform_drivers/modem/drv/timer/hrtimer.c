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

#include <timer_device.h>
#include <timer_dpm.h>
#include <mdrv_errno.h>
#include <osl_irq.h>
#include <osl_malloc.h>
#include <securec.h>

static inline s32 timer_check_id(u32 timer_id)
{
    timer_device_mgr_s *dev_mgr = timer_get_device_mgr();
    u32 number = dev_mgr->number;
    if (timer_id >= number) {
        timer_err("fail to config.handle is wrong.timer_id=%d, number=%d.\n", timer_id, number);
        return BSP_ERROR;
    }

    return BSP_OK;
}

static inline void timer_set_isr(u32 timer_id, timer_handle_cb routine, s32 para)
{
    timer_device_s *device = timer_get_device(timer_id);
    device->callback = routine;
    device->cb_para = para;
    return;
}

timer_handle mdrv_hrtimer_create(timer_attr_s *attr)
{
    u32 flag;
    u32 timer_id;
    u32 type;
    timer_device_s *device = NULL;
    errno_t ret;
    timer_mode_e mode;

    if (unlikely(attr == NULL)) {
        timer_err("fail to create timer!attr NULL\n");
        return (timer_handle)MDRV_TIMER_ENO_INPUT;
    }
    if (unlikely(attr->callback == NULL)) {
        timer_err("fail to create timer!callback NULL\n");
        return (timer_handle)MDRV_TIMER_ENO_INPUT;
    }

    flag = attr->flag;
    timer_id = 0;
    type = (flag & TIMER_WAKE) ? TIMER_TYPE_WKSRC : TIMER_TYPE_N_WKSRC;
    if (timer_dev_alloc(type, &timer_id)) {
        timer_err("fail to malloc timer.\n");
        return (timer_handle)MDRV_TIMER_ENO_CREATE;
    }

    device = timer_get_device(timer_id);
    if ((flag & TIMER_LOW_FREQ) == 0) {
        timer_set_clk_freq(device, TIMER_CLK_FREQ_19D2M);
    }
    ret = memcpy_s(device->name, sizeof(device->name), attr->name, strlen(attr->name) + 1);
    if (ret != EOK) {
        timer_err("[%s]fail to copy name, ret=0x%x. dst_size=%lu, src_size=%lu.\n",
            attr->name, ret, sizeof(device->name), strlen(attr->name));
    }

    timer_set_isr(timer_id, attr->callback, attr->para);

    mode = (flag & TIMER_PERIOD ? TIMER_MODE_PERIOD : TIMER_MODE_ONESHOT);
    device->driver_ops->set_ctrl_word((void *)(&(device->ctrl_value_latest)), mode, device->base_addr);

    device->time_unit = (flag & TIMER_UNIT_SLICE ? 1 : 0);

    return (timer_handle)timer_id;
}

s32 mdrv_hrtimer_start(timer_handle handle, u32 time)
{
    u32 timer_id = (u32)handle;
    timer_device_s *device = NULL;
    u64 tick_num;

    if (timer_check_id(timer_id) != BSP_OK) {
        return MDRV_ERROR;
    }

    device = timer_get_device(timer_id);
    tick_num = time;
    if (unlikely(device->time_unit == 0)) {
        tick_num = timer_calc_tick_num(time, device->clk_freq);
        if (tick_num > TIMER_MAX_TICK_NUM) {
            timer_err("fail to start.time is too long.time=%d.\n", time);
            return MDRV_ERROR;
        }
    }
    device->load_value_latest = (u32)tick_num;
    device->driver_ops->set_load_value((u32)tick_num, device->base_addr);

    if (device->driver_ops->enable((void *)(&(device->ctrl_value_latest)), device->base_addr) == BSP_OK) {
        device->slice_pre_irqrcvd = bsp_get_slice_value_hrt();
        device->debug_info[device->timeout_count & 0x3].slice_input = device->load_value_latest;
        return MDRV_OK;
    } else {
        return MDRV_ERROR;
    }
}

s32 mdrv_hrtimer_stop(timer_handle handle)
{
    u32 timer_id = (u32)handle;
    timer_device_s *device = NULL;

    if (timer_check_id(timer_id)) {
        return MDRV_ERROR;
    }

    device = timer_get_device(timer_id);
    device->driver_ops->disable((void *)(&(device->ctrl_value_latest)), device->base_addr);

    return MDRV_OK;
}

s32 mdrv_hrtimer_delete(timer_handle handle)
{
    u32 timer_id = (u32)handle;
    timer_device_s *device = NULL;

    if (timer_check_id(timer_id)) {
        return MDRV_ERROR;
    }

    device = timer_get_device(timer_id);
    /* 只有在用户指定为高频时才修改为高频 */
    if (device->clk_freq == TIMER_CLK_FREQ_19D2M) {
        timer_set_clk_freq(device, TIMER_CLK_FREQ_32K);
    }
    timer_dev_free(device);

    return MDRV_OK;
}

s32 mdrv_hrtimer_get_rest_time(timer_handle handle, unsigned int *rest_time)
{
    u32 timer_id = (u32)handle;
    timer_device_s *device = NULL;
    u32 tick_num;
    u64 numerator;

    if (timer_check_id(timer_id)) {
        return MDRV_ERROR;
    }

    device = timer_get_device(timer_id);
    tick_num = device->driver_ops->get_cur_tick(device->base_addr);

    if (device->time_unit == 0) {
        numerator = ((u64)tick_num) * TIMER_SEC_TO_USEC;
        *rest_time = (u32)(numerator / device->clk_freq);
    } else {
        *rest_time = tick_num;
    }

    return MDRV_OK;
}

s32 mdrv_hrtimer_debug_register(timer_handle handle, timer_handle_cb callback, int para)
{
    u32 timer_id = (u32)handle;
    timer_pmdbg_s *debug = NULL;

    if (timer_check_id(timer_id)) {
        return MDRV_ERROR;
    }

    debug = (timer_pmdbg_s *)osl_malloc(sizeof(timer_pmdbg_s));
    if (debug == NULL) {
        return MDRV_ERROR;
    }

    debug->timer_id = timer_id;
    debug->dpm_dbg_func = callback;
    debug->dpm_dbg_para = para;
    timer_dpm_dbg_register(debug);

    return MDRV_OK;
}

void hrtimer_show_trail(int index)
{
    u32 i;
    u32 j;
    timer_device_s *device = NULL;
    timer_device_mgr_s *device_mgr = timer_get_device_mgr();
    u32 number = device_mgr->number;

    if (index == -1) {
        timer_err(" index    name    \n");
        timer_err(" ----------------------\n");
        for (i = 0; i < number; i++) {
            device = timer_get_device(i);
            if (device->used == false) {
                continue;
            }
            timer_err("%-5d%-16s\n", i, device->name);
        }
        return;
    }

    timer_err("  index    name        time_input   time_output  usr_cb_cost\n");
    timer_err("------------------------------------------------------------\n");
    for (i = 0; i < number; i++) {
        if (i == index) {
            device = timer_get_device(i);
            for (j = 0; j < TIMER_DEBUG_INFO_NUMBER; j++) {
                timer_err("  %-5d%-16s%-13d%-13d%-10d\n", j, device->name, device->debug_info[j].slice_input,
                    device->debug_info[j].slice_timeout, device->debug_info[j].slice_cb_cost);
            }
            break;
        }
    }
}
EXPORT_SYMBOL(mdrv_hrtimer_create);
EXPORT_SYMBOL(mdrv_hrtimer_start);
EXPORT_SYMBOL(mdrv_hrtimer_stop);
EXPORT_SYMBOL(mdrv_hrtimer_delete);
EXPORT_SYMBOL(hrtimer_show_trail);
