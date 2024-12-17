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

#include <linux/ctype.h>
#include <mdrv_errno.h>
#include <product_config_drv_ap.h>
#include <bsp_print.h>
#include <bsp_reset.h>
#include <bsp_llt.h>
#include <bsp_dump.h>
#include "reset_dev.h"

#undef THIS_MODU
#define THIS_MODU mod_reset
#define reset_print_err(fmt, ...) (bsp_err("[%s] " fmt, __FUNCTION__, ##__VA_ARGS__))

#ifdef CONFIG_MODEM_RESET
static int g_modem_power_off_flag = 0;
spinlock_t g_modem_power_spinlock;
#endif

static void reset_system_error(u32 mod_id)
{
    system_error(mod_id, 0, 0, NULL, 0);
}

#ifdef CONFIG_MODEM_RESET
void reset_dev_init(void)
{
    spin_lock_init(&g_modem_power_spinlock);
}

// 根据传入的文件节点值做相应的动作
// 已经下电状态，不再响应单独复位
// 单独复位过程中，不响应下电动作
// 两个rild或多线程同时调用时，有可能同时触发power off, 需要保证状态机正确性
// 所有过程必须等前一个过程执行结束才可以进来

static void reset_modem_handle(unsigned long dev_val)
{
    unsigned long lock_flag;

    spin_lock_irqsave(&g_modem_power_spinlock, lock_flag);
    /* 单独复位不能重入跟dump不能重入存在时间配合问题，因此删除单独复位的重入判断，由dump保证调用不重入 */
    if (g_modem_power_off_flag) {
        reset_print_err("modem has been power off, reset rejected!\n");
        spin_unlock_irqrestore(&g_modem_power_spinlock, lock_flag);
        return;
    }
    if (dev_val == DRV_MODEM_OFF) {
        g_modem_power_off_flag = 1;
    }
    spin_unlock_irqrestore(&g_modem_power_spinlock, lock_flag);

    switch (dev_val) {
        case DRV_MODEM_RESET:
            reset_print_err("modem reset %d\n", DRV_MODEM_RESET);
            reset_system_error(DRV_ERRNO_RESET_SIM_SWITCH);
            break;
        case DRV_MODEM_OFF:
            bsp_modem_power_off();
            reset_print_err("modem power off %d\n", DRV_MODEM_OFF);
            break;
        case DRV_MODEM_RILD_SYS_ERR:
            reset_print_err("modem reset using system_error by rild %d\n", DRV_MODEM_RILD_SYS_ERR);
            reset_system_error(DRV_ERRNO_RESET_RILD);
            break;
        default:
            reset_print_err("Modem power : invalid val[%ld] to modem power !!!!\n", dev_val);
            break;
    }
    return;
}
#endif /* CONFIG_MODEM_RESET */

static int reset_parse_input(const char *buf, size_t count, unsigned long *rslt)
{
    unsigned long dev_val = 0;
    int i;

    if ((count != 3U) && (count != 2U)) {
        reset_print_err("buf len err: %d\n", (int)count);
        return RESET_ERROR;
    }

    // buffer 转换为整数val, count 包含结束符
    for (i = 0; i < count - 1; i++) {
        if (!isdigit(buf[i])) {
            reset_print_err("buf[%d] err: %c\n", i, buf[i]);
            return RESET_ERROR;
        }
        dev_val = dev_val * 10U + (buf[i] - '0');
    }

    if ((dev_val != DRV_MODEM_RESET) && (dev_val != DRV_MODEM_RILD_SYS_ERR) && (dev_val != DRV_MODEM_OFF)) {
        reset_print_err("Modem power : invalid val[%ld] to modem power!!!!\n", dev_val);
        return RESET_ERROR;
    }

    reset_print_err("Modem power set to %lu !!!\n", dev_val);
    *rslt = dev_val;
    return RESET_OK;
}


// step1: mbb 产品暂不支持单独复位，文件节点处宏控不调用本接口
// step2: buffer校验，buffer 是两位整数, 且取值范围受限,其余输入直接返回输入异常
// step3: llt过程中触发单独复位, 单独复位不响应 （llt框架保证杀掉ril）
// step4: 手机产品，单独复位关闭场景下，需触发整机复位(特性宏或nv关闭单独复位)
// step5: 手机产品，单独复位打开情况下，保证两个rild的情况下，或者多个任务同时写节点的情况下，状态机不会冲突
ssize_t bsp_reset_set_dev(const char *buf, size_t count)
{
    unsigned long dev_val = 0;

    if (buf == NULL) {
        reset_print_err("buf is null!\n");
        return -EINVAL;
    }

    if (is_in_llt()) {
        reset_print_err("modem reset trigger fail, is in llt!\n");
        return (ssize_t)count;
    }

    if (reset_parse_input(buf, count, &dev_val) != RESET_OK) {
        reset_print_err("input parse fail!\n");
        return -EINVAL;
    }

#ifdef CONFIG_MODEM_RESET
    if (bsp_reset_is_feature_on() == MDM_RESET_SUPPORT) {
        reset_modem_handle(dev_val);
        return (ssize_t)count;
    }
#endif

    // 关闭单独复位后，rild触发的单独复位，需要触发整机复位
    reset_print_err("modem reset feature off， reboot!\n");
    reset_system_error(DRV_ERRNO_RESET_REBOOT_REQ);
    return (ssize_t)count;
}
