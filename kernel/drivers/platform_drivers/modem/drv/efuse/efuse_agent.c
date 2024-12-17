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
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/pm_wakeup.h>
#include <linux/version.h>
#include <product_config.h>
#include <bsp_llt.h>
#include <bsp_sec_call.h>
#include <bsp_efuse.h>
#include <bsp_slice.h>
#include <securec.h>
#include <bsp_msg.h>
#include <msg_id.h>
#include "efuse_plat.h"
#include <adrv_efuse.h>
#include "efuse_agent.h"
#include "efuse_dump.h"
#include "efuse_driver.h"
#include "efuse_error.h"
#include "osl_sem.h"

#define THIS_MODU mod_efuse

efuse_agent_info_s g_efuse_ccore_agent_info;
efuse_agent_info_s g_efuse_agent_info;

void bsp_efuse_agent_opt_sec_read(efuse_data_s *msg, u32 opt_core)
{
    bsp_efuse_ops_prepare();
    EFUSE_RECORD_EVENT(EFUSE_ACORE_SEC_CALL_START);
    msg->ret = (s32)bsp_sec_call(FUNC_EFUSE_SEC_OPT, opt_core);
    EFUSE_RECORD_EVENT(EFUSE_ACORE_SEC_CALL_DONE);

    bsp_efuse_ops_complete();

    return;
}
void bsp_efuse_agent_opt_sec_write(efuse_data_s *msg, u32 opt_core)
{
    efuse_print_info("efuse write group=%d, length=%d\n", msg->start, msg->len);
    EFUSE_RECORD_EVENT(EFUSE_ACORE_SEC_CALL_START);
    if ((msg->start == EFUSE_GRP_HUK) && (!is_in_llt())) {
        msg->ret = (s32)set_efuse_kce_value((unsigned char *)msg->buf, (unsigned int)msg->len * EFUSE_GROUP_SIZE, 0);
    }
    bsp_efuse_ops_prepare();

    msg->ret = (s32)bsp_sec_call(FUNC_EFUSE_SEC_OPT, opt_core);

    (void)memset_s((void *)msg->buf, sizeof(u32) * EFUSE_HUK_SIZE, 0, sizeof(u32) * msg->len);
    bsp_efuse_ops_complete();
    EFUSE_RECORD_EVENT(EFUSE_ACORE_SEC_CALL_DONE);
}

static int efuse_msg_proc(const struct msg_addr *src, void *buf, u32 len)
{
    if (src == NULL || buf == NULL || len != sizeof(efuse_data_s)) {
        efuse_print_error("efuse msg recv err, len=0x%x", len);
        return -1;
    }

    if (src->core == MSG_CORE_TSP) {
        (void)memcpy_s((void *)&g_efuse_agent_info.ccore_msg, sizeof(g_efuse_agent_info.ccore_msg), buf, sizeof(efuse_data_s));
        /* Returns %false if @work was already on a queue, %true otherwise. */
        queue_work(g_efuse_agent_info.wq, &g_efuse_agent_info.ccore_work);
    } else if (src->core == MSG_CORE_LPM) {
        (void)memcpy_s((void *)&g_efuse_agent_info.lpmcu_msg, sizeof(g_efuse_agent_info.lpmcu_msg), buf, sizeof(efuse_data_s));
        /* Returns %false if @work was already on a queue, %true otherwise. */
        queue_work(g_efuse_agent_info.wq, &g_efuse_agent_info.lpmcu_work);
    } else {
        efuse_print_error("src->core = %d!\n", src->core);
        return -1;
    }
    pm_stay_awake(g_efuse_agent_info.dev);

    EFUSE_RECORD_EVENT(EFUSE_ACORE_MSG_PROC_DONE);
    return 0;
}

void efuse_handle_ccore_work(struct work_struct *work)
{
    efuse_data_s *msg;
    osl_sem_down(&g_efuse_agent_info.workqueue_sem);
    msg = &g_efuse_agent_info.ccore_msg;
    EFUSE_RECORD_EVENT(EFUSE_ACORE_HANDLE_WORK_START);

    if (msg->opt == EFUSE_OPT_SEC_WRITE) {
        bsp_efuse_agent_opt_sec_write(msg, EFUSE_OPT_CCORE_CORE);
    } else {
        bsp_efuse_agent_opt_sec_read(msg, EFUSE_OPT_CCORE_CORE);
    }

    pm_relax(g_efuse_agent_info.dev);
    EFUSE_RECORD_EVENT(EFUSE_ACORE_HANDLE_WORK_DONE);
    osl_sem_up(&g_efuse_agent_info.workqueue_sem);
    return;
}

void efuse_handle_lpmcu_work(struct work_struct *work)
{
    efuse_data_s *msg;
    osl_sem_down(&g_efuse_agent_info.workqueue_sem);
    msg = &g_efuse_agent_info.lpmcu_msg;
    EFUSE_RECORD_EVENT(EFUSE_ACORE_HANDLE_WORK_START);

    if (msg->opt == EFUSE_OPT_SEC_READ) {
        bsp_efuse_agent_opt_sec_read(msg, EFUSE_OPT_LPMCU_CORE);
    }

    pm_relax(g_efuse_agent_info.dev);
    EFUSE_RECORD_EVENT(EFUSE_ACORE_HANDLE_WORK_DONE);
    osl_sem_up(&g_efuse_agent_info.workqueue_sem);
    return;
}

int modem_efuse_probe(struct platform_device *pdev)
{
    int ret;
    struct msgchn_attr lite_attr = { 0 };
    g_efuse_agent_info.dev = &pdev->dev;
    bsp_msgchn_attr_init(&lite_attr);
    lite_attr.chnid = MSG_CHN_EFUSE;
    lite_attr.coremask = MSG_CORE_MASK(MSG_CORE_TSP) | MSG_CORE_MASK(MSG_CORE_LPM);
    lite_attr.lite_notify = efuse_msg_proc;
    ret = bsp_msg_lite_open(&g_efuse_agent_info.msghdl, &lite_attr);
    if (ret) {
        efuse_print_error("msg open err %d\n", ret);
        return ret;
    }

    device_init_wakeup(g_efuse_agent_info.dev, true);

    INIT_WORK(&g_efuse_agent_info.ccore_work, efuse_handle_ccore_work);
    INIT_WORK(&g_efuse_agent_info.lpmcu_work, efuse_handle_lpmcu_work);
    g_efuse_agent_info.wq = alloc_ordered_workqueue("modem_efuse_wq", 0);
    if (g_efuse_agent_info.wq == NULL) {
        efuse_print_error("wq create fail\n");
        return EFUSE_ERROR;
    }

    osl_sem_init(1, &g_efuse_agent_info.workqueue_sem);
    efuse_debug_init();
    bsp_err("modem efuse agent init ok.\n");

    EFUSE_RECORD_EVENT(EFUSE_ACORE_INIT_SUCCESS);
    return EFUSE_OK;
}

static struct platform_device g_modem_efuse_device = {
    .name = "modem_efuse",
    .id = -1,
};

struct platform_driver g_modem_efuse_driver = {
    .probe      = modem_efuse_probe,
    .driver     = {
        .name     = "modem_efuse",
        .owner    = THIS_MODULE,
        .probe_type = PROBE_FORCE_SYNCHRONOUS,
    },
};

int modem_efuse_agent_init(void)
{
    int ret;
    ret = platform_device_register(&g_modem_efuse_device);
    if (ret) {
        efuse_print_error("modem efuse device register err %d\n", ret);
        return ret;
    }

    ret = platform_driver_register(&g_modem_efuse_driver);
    if (ret) {
        platform_device_unregister(&g_modem_efuse_device);
        efuse_print_error("modem efuse driver register err %d\n", ret);
    }
    return ret;
}

#if !IS_ENABLED(CONFIG_MODEM_DRIVER)
module_init(modem_efuse_agent_init);
#endif

