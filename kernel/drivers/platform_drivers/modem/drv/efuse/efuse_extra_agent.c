/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2020. All rights reserved.
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

/*lint --e{528,715} */

#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/printk.h>
#include <linux/gpio.h>
#include <linux/module.h>
#include <linux/clk.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#include <linux/semaphore.h>
#include <linux/regulator/consumer.h>

#include "product_config.h"
#include "osl_bio.h"
#include "osl_sem.h"
#include "osl_thread.h"
#include "osl_spinlock.h"
#include "soc_clk.h"

#include "bsp_slice.h"
#include "bsp_efuse.h"
#include "efuse_driver.h"

#include "securec.h"

#define THIS_MODU mod_efuse
#ifdef PHONE_EFUSE
#include <adrv_efuse.h>
#endif
#include "efuse_agent.h"

struct work_struct g_modem_efuse_work;

efuse_data_s g_modem_efuse_msg;

void bsp_efuse_agent_opt_write(efuse_data_s *msg)
{
    efuse_print_info("efuse write start group = %d, length = %d\n", msg->start, msg->len);
    msg->ret = set_efuse_kce_value((unsigned char *)msg->buf, (unsigned int)msg->len * EFUSE_GROUP_SIZE, 0);
    efuse_print_info("efuse write finish, ret = %d\n", msg->ret);
}

struct efuse_agent_opt {
    u32 opt_code;
    void (*func_agent_opt)(efuse_data_s *);
};

struct efuse_agent_opt g_efuse_agent_opt[] = {
    { EFUSE_OPT_WRITE, bsp_efuse_agent_opt_write },
};

void efuse_handle_work(struct work_struct *work)
{
    u32 i;
    int length;
    u32 channel_id = ICC_CHANNEL_ID_MAKEUP(ICC_CHN_APLUSB_IFC, APLUSB_IFC_FUNC_EFUSE);

    efuse_data_s *msg = &g_modem_efuse_msg;

    for (i = 0; i < sizeof(g_efuse_agent_opt) / sizeof(g_efuse_agent_opt[0]); i++) {
        if (msg->opt == g_efuse_agent_opt[i].opt_code) {
            g_efuse_agent_opt[i].func_agent_opt(msg);
            break;
        }
    }

    if (i == sizeof(g_efuse_agent_opt) / sizeof(g_efuse_agent_opt[0])) {
        msg->ret = EFUSE_ERROR_ARGS;
        efuse_print_error("error opt, opt = 0x%08X\n", msg->opt);
    }

    if ((msg->opt != EFUSE_OPT_SEC_READ) && (msg->opt != EFUSE_OPT_SEC_WRITE)) {
        length = bsp_icc_send(ICC_CPU_APLUSB, channel_id, (unsigned char *)msg, (u32)sizeof(efuse_data_s));
        if (length != (int)sizeof(efuse_data_s)) {
            msg->ret = EFUSE_ERROR_ICC_SEND_FAIL;
            efuse_print_error("send len(%d) != expected len(%d)\n", length, (int)sizeof(efuse_data_s));
            return;
        }
    }
}

int bsp_efuse_data_receive(u32 channel_id, u32 len, void *context)
{
    int ret;
    int length;

    length = bsp_icc_read(channel_id, (u8 *)&g_modem_efuse_msg, (u32)sizeof(efuse_data_s));
    if (length != (int)sizeof(efuse_data_s)) {
        efuse_print_error("read len(%d) != expected len(%d)\n", length, (int)sizeof(efuse_data_s));
        return EFUSE_ERROR_ICC_READ_FAIL;
    }

    ret = (int)schedule_work(&g_modem_efuse_work);

    efuse_print_info("efuse schedule_work begin, ret = %d\n", ret);

    return EFUSE_OK;
}

void bsp_efuse_icc_channel_register(void)
{
    int ret;
    u32 channel_id = ICC_CHANNEL_ID_MAKEUP(ICC_CHN_APLUSB_IFC, APLUSB_IFC_FUNC_EFUSE);

    ret = bsp_icc_event_register(channel_id, bsp_efuse_data_receive, NULL, NULL, NULL);
    if (ret) {
        efuse_print_error("fail to register efuse icc event, ret = %d\n", ret);
    }
}

int bsp_efuse_agent_init(void)
{
    INIT_WORK(&g_modem_efuse_work, efuse_handle_work);

    pcie_icc_valid_register(bsp_efuse_icc_channel_register);

    efuse_print_info("modem efuse agent init ok\n");

    return EFUSE_OK;
}

#if !IS_ENABLED(CONFIG_MODEM_DRIVER)
module_init(bsp_efuse_agent_init);
#endif
