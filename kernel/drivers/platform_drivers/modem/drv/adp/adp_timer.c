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

/*lint --e{537,648 }*/
#include <linux/syscore_ops.h>
#include <linux/slab.h>
#include <linux/printk.h>

#include <osl_module.h>
#include <osl_bio.h>

#include <mdrv_timer.h>
#include <drv_comm.h>
#include <bsp_dt.h>
#include <bsp_slice.h>
#include <bsp_version.h>
#include <securec.h>

/*
 * 函数  : mdrv_timer_get_accuracy_timestamp
 * 功能  : 获取BBP定时器的值。用于OAM 时戳
 * 输出  : p_high32bit_value指针参数不能为空，否则会返回失败。
 *         p_low32bit_value如果为空， 则只返回低32bit的值。
 */
int mdrv_timer_get_accuracy_timestamp(unsigned int *p_high32bit_value, unsigned int *p_low32bit_value)
{
    u64 cur_time;

    if (BSP_NULL == p_low32bit_value) {
        return BSP_ERROR;
    }

    if (bsp_slice_getcurtime(&cur_time)) {
        return BSP_ERROR;
    }

    if (BSP_NULL != p_high32bit_value) {
        *p_high32bit_value = (cur_time >> 32); /* 获取高32位 */
    }

    *p_low32bit_value = cur_time & 0xffffffff;

    return BSP_OK;
}

unsigned int mdrv_timer_get_normal_timestamp(void)
{
    return bsp_get_slice_value();
}

unsigned int mdrv_timer_get_hrt_timestamp(void)
{
    return bsp_get_slice_value_hrt();
}

unsigned int mdrv_get_normal_timestamp_freq(void)
{
    return bsp_get_slice_freq();
}

unsigned int mdrv_get_hrt_timestamp_freq(void)
{
    return bsp_get_hrtimer_freq();
}

EXPORT_SYMBOL(mdrv_timer_get_accuracy_timestamp);
EXPORT_SYMBOL(mdrv_timer_get_normal_timestamp);
EXPORT_SYMBOL(mdrv_timer_get_hrt_timestamp);
EXPORT_SYMBOL(mdrv_get_normal_timestamp_freq);
EXPORT_SYMBOL(mdrv_get_hrt_timestamp_freq);
/*lint -restore +e19*/
