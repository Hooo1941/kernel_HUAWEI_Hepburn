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

/*lint --e{537}*/
/*lint -save -e958*/
#include <linux/module.h>
#include <linux/string.h>
#include <linux/atomic.h>
#include <linux/kthread.h>
#include <linux/printk.h>
#include <securec.h>
#include <adrv_version.h>
#include <osl_sem.h>
#include <bsp_pcie.h>
#include <bsp_version.h>


#undef THIS_MODU
#define THIS_MODU mod_version
#undef ver_print_error
#define ver_print_error(fmt, ...) (printk(KERN_ERR "<%s> " fmt, __FUNCTION__, ##__VA_ARGS__))
struct version_pcie_msg {
    struct productid_pcie_msg msg;
    osl_sem_id sem_lock;
};

static struct version_pcie_msg g_version_extra_modem;

#ifdef CONFIG_VERSION_EXTRA_MODEM_A
/* VERSION_TASK not define, just resolve warnning */
#ifdef VERSION_TASK
int bsp_version_msg_task(void *data)
{
    int ret = 0;
    struct pcie_trans_msg_info send_msg = {0};

    do {
        osl_sem_down(&g_version_extra_modem.sem_lock);
        ver_print_error("version pcie task start-handler\n");
        send_msg.msi_id = PCIE_MSI_ID_VERSION;
        send_msg.cmd_len = sizeof(struct productid_pcie_msg);
        g_version_extra_modem.msg.msg_type = PRODUCTID_MSG_UPDATE;
        send_msg.cmd_data_addr = &g_version_extra_modem.msg;
        printk(KERN_ERR "send product_id:0x%x\n", g_version_extra_modem.msg.product_id);
        ret = bsp_pcie_rc_send_msg(&send_msg);
        if (ret) {
            ver_print_error("version pcie send msg failed:0x%x\n", ret);
            continue;
        }

        ver_print_error("version send product_id seccuss\n");
    } while (1);
}
#endif
void bsp_version_extra_boardid_init(void)
{
    g_version_extra_modem.msg.product_id = get_cmd_product_id();
}

irqreturn_t bsp_version_msi_handler(int irq, void *dev_info)
{
    u32 ret;
    struct productid_pcie_msg msg_data = {0};
    struct pcie_trans_msg_info send_msg = {0};
    struct pcie_trans_msg_info resp_msg = {
        .msi_id = 0,
        .cmd_len = 0,
        .cmd_data_addr = &msg_data,
    };

    if (bsp_pcie_rc_read_msg(&resp_msg)) {
        ver_print_error("version get product_id pcie msg error\n");
        return IRQ_HANDLED;
    }

    bsp_pcie_rc_read_clear();
    send_msg.msi_id = PCIE_MSI_ID_VERSION;
    send_msg.cmd_len = sizeof(struct productid_pcie_msg);
    g_version_extra_modem.msg.msg_type = PRODUCTID_MSG_UPDATE;
    send_msg.cmd_data_addr = &g_version_extra_modem.msg;
    ret = bsp_pcie_rc_send_msg(&send_msg);
    if (ret) {
        ver_print_error("version pcie send msg failed:0x%x\n", ret);
        return IRQ_HANDLED;
    }

    ver_print_error("version send product_id:0x%x seccuss\n", g_version_extra_modem.msg.product_id);

    return IRQ_HANDLED;
}

u32 bsp_version_extra_modem_init(void)
{
    u32 ret;

    bsp_version_extra_boardid_init();
    printk(KERN_ERR "version product_id:0x%x\n", g_version_extra_modem.msg.product_id);

    ret = bsp_pcie_receive_msi_request(PCIE_MSI_ID_VERSION, bsp_version_msi_handler, "version_msg_msi",
                                       NULL);
    if (ret) {
        ver_print_error("version pcie msi request failed:0x%x\n", ret);
        return ret;
    }

    return VER_OK;
}
#endif
/*
 * 功能描述: acore版本号初始化
 */
int bsp_version_acore_init(void)
{
    int ret = 0;

    printk(KERN_ERR "version start init\n");
#ifdef CONFIG_VERSION_EXTRA_MODEM_A
    ret = bsp_version_extra_modem_init();
    if (ret) {
        ver_print_error("version extra modem init failed\n");
    }
#endif
    printk(KERN_ERR "version init end\n");
    return ret;
}

#if !IS_ENABLED(CONFIG_MODEM_DRIVER)
/*lint -restore*/
/* 注意:需在nv模块初始化之后 */
module_init(bsp_version_acore_init);
#endif/*lint -restore*/

