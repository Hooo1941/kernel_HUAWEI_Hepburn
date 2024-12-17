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

#include <product_config.h>
#include <bsp_print.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/stat.h>
#include <linux/string.h>
#include <linux/semaphore.h>

#undef THIS_MODU
#define THIS_MODU mod_avs

#define avs_err(fmt, ...) bsp_err(fmt, ##__VA_ARGS__)
#define VTRIM_NUM 6
#define ICC_CHN_OFFSET 16

struct avs_msg {
    u32 ccpu_profile;
    u32 fastbus_freq;
    u32 slowbus_freq;
    u32 dsp_freq;
    u32 bbp_profile;
    u32 volt;
};

struct semaphore g_avs_sem_lr;
struct semaphore g_avs_sem_nr;

u32 g_avs_vtrim_lr[VTRIM_NUM] = { 0, 1, 2, 3, 4, 5 };
u32 g_avs_vtrim_nr[VTRIM_NUM] = { 0, 1, 2, 3, 4, 5 };
u32 g_avs_num = VTRIM_NUM;

module_param_array(g_avs_vtrim_lr, uint, &g_avs_num, S_IRUGO);
MODULE_PARM_DESC(g_avs_vtrim_lr, "g_avs_vtrim_lr");
module_param_array(g_avs_vtrim_nr, uint, &g_avs_num, S_IRUGO);
MODULE_PARM_DESC(g_avs_vtrim_nr, "g_avs_vtrim_nr");

int avs_send_msg_lr(u32 ccpu_profile, u32 fastbus_freq, u32 slowbus_freq, u32 dsp_freq, u32 bbp_profile, u32 volt)
{
    struct avs_msg msg;
    int msg_size = sizeof(struct avs_msg);
    u32 channel_id = ICC_CHN_IFC << ICC_CHN_OFFSET | IFC_RECV_FUNC_AVS_TEST;
    int ret;

    msg.ccpu_profile = ccpu_profile;
    msg.fastbus_freq = fastbus_freq;
    msg.slowbus_freq = slowbus_freq;
    msg.dsp_freq = dsp_freq;
    msg.bbp_profile = bbp_profile;
    msg.volt = volt;

    ret = bsp_icc_send(ICC_CPU_MODEM, channel_id, (unsigned char *)&msg, msg_size);
    if (ret != msg_size) {
        avs_err("icc msgsize=%d, ret=%d\n", msg_size, ret);
    }

    down(&g_avs_sem_lr);
    return 0;
}
EXPORT_SYMBOL(avs_send_msg_lr);

int avs_send_msg_nr(u32 ccpu_profile, u32 fastbus_freq, u32 slowbus_freq, u32 dsp_freq, u32 bbp_profile, u32 volt)
{
    struct avs_msg msg;
    int msg_size = sizeof(struct avs_msg);
    u32 channel_id = ICC_CHN_NRIFC << ICC_CHN_OFFSET | NRIFC_RECV_FUNC_AVS_TEST;
    int ret;

    msg.ccpu_profile = ccpu_profile;
    msg.fastbus_freq = fastbus_freq;
    msg.slowbus_freq = slowbus_freq;
    msg.dsp_freq = dsp_freq;
    msg.bbp_profile = bbp_profile;
    msg.volt = volt;

    ret = bsp_icc_send(ICC_CPU_NRCCPU, channel_id, (unsigned char *)&msg, msg_size);
    if (ret != msg_size) {
        avs_err("icc msgsize=%d, ret=%d\n", msg_size, ret);
    }

    down(&g_avs_sem_nr);
    return 0;
}
EXPORT_SYMBOL(avs_send_msg_nr);

int avs_icc_read_cb_lr(u32 channel_id, u32 len, void *context)
{
    int ret;
    int msg_size = VTRIM_NUM * sizeof(u32);

    ret = bsp_icc_read(channel_id, (unsigned char *)g_avs_vtrim_lr, msg_size);
    if (ret != msg_size) {
        avs_err("acpu avs icc readcb size=%d ret=%d\n", msg_size, ret);
        return ICC_ERR;
    }
    avs_err("recv msg from lrcore\n");
    up(&g_avs_sem_lr);

    return 0;
}
int avs_icc_read_cb_nr(u32 channel_id, u32 len, void *context)
{
    int ret;
    int msg_size = VTRIM_NUM * sizeof(u32);

    ret = bsp_icc_read(channel_id, (unsigned char *)g_avs_vtrim_nr, msg_size);
    if (ret != msg_size) {
        avs_err("acpu avs icc readcb size=%d ret=%d\n", msg_size, ret);
        return ICC_ERR;
    }
    avs_err("recv msg from nrcore\n");
    up(&g_avs_sem_nr);

    return 0;
}

int avs_test_init(void)
{
    u32 channel_id_lr = ICC_CHN_IFC << ICC_CHN_OFFSET | IFC_RECV_FUNC_AVS_TEST;
    u32 channel_id_nr = ICC_CHN_NRIFC << ICC_CHN_OFFSET | NRIFC_RECV_FUNC_AVS_TEST;
    int ret;

    sema_init(&g_avs_sem_lr, 0);
    sema_init(&g_avs_sem_nr, 0);

    ret = bsp_icc_event_register(channel_id_lr, avs_icc_read_cb_lr, NULL, (write_cb_func)NULL, (void *)NULL);
    if (ret != ICC_OK) {
        avs_err("lr icc init err\n");
    }

    ret = bsp_icc_event_register(channel_id_nr, avs_icc_read_cb_nr, NULL, (write_cb_func)NULL, (void *)NULL);
    if (ret != ICC_OK) {
        avs_err("nr icc init err\n");
    }

    avs_err("avs_test_init ok\n");
    return 0;
}

#if !IS_ENABLED(CONFIG_MODEM_DRIVER)
subsys_initcall(avs_test_init);
#endif
