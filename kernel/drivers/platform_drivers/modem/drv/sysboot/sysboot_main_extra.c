/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
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
/*lint --e{528,537,715} */
#include <linux/kernel.h>
#include <linux/module.h>

#include "mdrv_errno.h"
#include "bsp_dump.h"

struct work_struct g_modem_sysboot_work;
u32 g_modem_sysboot_status;

void sysboot_handle_work(struct work_struct *work)
{
    u32 status = g_modem_sysboot_status;

    if (status != DRV_ERROR_USER_RESET) {
        printk(KERN_ERR "[sysboot]modem reset by unknown reason.no action.\n");
        return;
    }

    printk(KERN_ERR "[sysboot]modem normal reset.\n");
    system_error(DRV_ERROR_USER_RESET, 0, 0, NULL, 0);
    return;
}

int bsp_sysboot_data_receive(u32 channel_id, u32 len, void *context)
{
    int ret;
    int length;

    length = bsp_icc_read(channel_id, (u8 *)&g_modem_sysboot_status, (u32)sizeof(u32));
    if (length != (int)sizeof(u32)) {
        printk(KERN_ERR "[sysboot]read len(%d) != expected len(%d)\n", length, (int)sizeof(u32));
        return -1;
    }

    ret = (int)schedule_work(&g_modem_sysboot_work);

    printk(KERN_ERR "[sysboot]sysboot schedule_work begin, ret = %d\n", ret);

    return ret;
}

void bsp_sysboot_icc_channel_register(void)
{
    int ret;
    u32 channel_id = ICC_CHN_APLUSB_IFC << 16U | APLUSB_IFC_FUNC_SYSBOOT;

    ret = bsp_icc_event_register(channel_id, bsp_sysboot_data_receive, NULL, NULL, NULL);
    if (ret) {
        printk(KERN_ERR "[sysboot]fail to register sysboot icc event, ret = %d\n", ret);
    }
}

int bsp_sysboot_extra_init(void)
{
    INIT_WORK(&g_modem_sysboot_work, sysboot_handle_work);
    pcie_icc_valid_register(bsp_sysboot_icc_channel_register);

    printk(KERN_ERR "[sysboot]sysboot extra init ok.\n");
    return 0;
}

#if !IS_ENABLED(CONFIG_MODEM_DRIVER)
module_init(bsp_sysboot_extra_init);
#endif
