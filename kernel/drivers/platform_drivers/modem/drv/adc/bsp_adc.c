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

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <osl_types.h>
#include <osl_bio.h>
#include <osl_sem.h>
#include <bsp_slice.h>
#include <bsp_reset.h>
#include <product_config.h>
#include <bsp_pcie.h>
#include <adrv_adc.h>

struct adc_recv_msg {
    int adc_value;
    int seq;
};
struct adc_send_msg {
    unsigned int channel;
    unsigned int mode;
    int seq;
};
struct adc_msg {
    osl_sem_id adc_sem;
    osl_sem_id data_resem;
    struct adc_send_msg adc_msg;
    struct adc_recv_msg adc_data_s;
    spinlock_t adc_lock;
    int err_id;
};

struct adc_msg g_adc_msg;

int adc_mbb_icc_callback(u32 channel_id, u32 len, void *context)
{
    int ret;
    unsigned long flags = 0;
    struct adc_recv_msg icc_msg = { 0, 0 };

    ret = bsp_icc_read(channel_id, (u8 *)&icc_msg, sizeof(struct adc_recv_msg));
    if (ret != (int)len) {
        printk(KERN_ERR "[adc] icc read size %d, return 0x%x.\n", len, ret);
        return ret;
    }
    spin_lock_irqsave(&(g_adc_msg.adc_lock), flags);
    if (g_adc_msg.adc_msg.seq != icc_msg.seq) {
        printk(KERN_ERR "seq num is not right,should be 0x%x,now is 0x%x.\n", g_adc_msg.adc_msg.seq, icc_msg.seq);
        spin_unlock_irqrestore(&(g_adc_msg.adc_lock), flags);
        return -1;
    }
    spin_unlock_irqrestore(&(g_adc_msg.adc_lock), flags);

    g_adc_msg.adc_data_s.adc_value = icc_msg.adc_value;
    g_adc_msg.adc_data_s.seq = icc_msg.seq;

    osl_sem_up(&(g_adc_msg.adc_sem));

    return 0;
}
#define MDM_TEMP_ERROR (-33000)
#define MDM_TEMP_TIMEOUT 2000
int bsp_adc_get_voltage(struct mdm_adc_s *adc_param)
{
    int ret;
    u32 len = sizeof(struct adc_send_msg);
    struct adc_send_msg icc_msg;
    u32 timeout_jiffies = msecs_to_jiffies(MDM_TEMP_TIMEOUT);
    unsigned long flags = 0;

    osl_sem_down(&(g_adc_msg.data_resem));

    icc_msg.channel = adc_param->channel;
    icc_msg.mode = adc_param->mode;
    icc_msg.seq = g_adc_msg.adc_msg.seq;

    ret = bsp_pcie_rc_vote_lock(PCIE_USER_TEMP, 0);
    if (ret != 0) {
        printk(KERN_ERR "pcie is already asleep, 0x%x!\n", ret);
        goto lock_err;
    }
    ret = bsp_icc_send(ICC_CPU_APLUSB, ADC_ICC_CHN_ACORE_ACORE_DATA, (u8 *)&icc_msg, len);
    if (ret != (int)len) {
        printk(KERN_ERR "icc send fail! 0x%x.\n", ret);
        goto icc_err;
    }

    ret = osl_sem_downtimeout(&(g_adc_msg.adc_sem), timeout_jiffies);
    spin_lock_irqsave(&(g_adc_msg.adc_lock), flags);
    g_adc_msg.adc_msg.seq++;
    spin_unlock_irqrestore(&(g_adc_msg.adc_lock), flags);
    if (ret != 0) {
        printk(KERN_ERR "wait icc back fail 0x%x!\n", ret);
        goto icc_err;
    }

    ret = bsp_pcie_rc_vote_unlock(PCIE_USER_TEMP);
    if (ret != 0) {
        printk(KERN_ERR "pcie release lock fail 0x%x!\n", ret);
        goto lock_err;
    }

    ret = g_adc_msg.adc_data_s.adc_value;

    osl_sem_up(&(g_adc_msg.data_resem));

    return ret;

icc_err:
    bsp_pcie_rc_vote_unlock(PCIE_USER_TEMP);
lock_err:
    osl_sem_up(&(g_adc_msg.data_resem));

    g_adc_msg.err_id = ret;

    ret = MDM_TEMP_ERROR;

    return ret;
}

int bsp_adc_get_voltage_debug(int channel, int mode)
{
    int ret;
    struct mdm_adc_s param;

    param.channel = channel;
    param.mode = mode;

    ret = bsp_adc_get_voltage(&param);
    printk(KERN_ERR "bsp_adc_get_voltage = %d.\n", ret);

    return ret;
}

void adc_icc_register(void)
{
    int ret;

    ret = bsp_icc_event_register(ADC_ICC_CHN_ACORE_ACORE_DATA, adc_mbb_icc_callback, NULL, NULL, NULL);
    if (ret != 0) {
        printk(KERN_ERR "[adc] icc register fail! ret = %d.\n", ret);
        return;
    }

    ret = mdm_adc_get_value_register(bsp_adc_get_voltage);
    if (ret != 0) {
        printk(KERN_ERR "[adc] ap register fail! ret = %d.\n", ret);
        return;
    }
}

int adc_modem_init(void)
{
    osl_sem_init(0, &(g_adc_msg.adc_sem));
    osl_sem_init(1, &(g_adc_msg.data_resem));
    spin_lock_init(&(g_adc_msg.adc_lock));

    g_adc_msg.adc_msg.seq = bsp_get_slice_value();

    pcie_icc_valid_register(adc_icc_register);

    printk(KERN_ERR "[adc] adc_modem_init ok !\n");

    return 0;
}

#if !IS_ENABLED(CONFIG_MODEM_DRIVER)
module_init(adc_modem_init);
#endif
