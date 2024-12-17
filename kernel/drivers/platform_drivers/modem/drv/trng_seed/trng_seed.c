/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
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
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/pm_wakeup.h>
#include <linux/version.h>
#include <bsp_sec_call.h>
#include <securec.h>
#include <bsp_print.h>

#include <msg_id.h>
#include <bsp_msg.h>

#define THIS_MODU mod_security

#define trng_print(fmt, ...) (bsp_err("[trng]<%s> " fmt "\n", __FUNCTION__, ##__VA_ARGS__))
#define trng_info(fmt, ...) (bsp_info("[trng]<%s> " fmt "\n", __FUNCTION__, ##__VA_ARGS__))

struct trng_driver_info {
    struct device *dev;
    msg_chn_t trng_msghdl;
    struct workqueue_struct *trng_wq;
    struct work_struct trng_work;
};

static struct trng_driver_info g_trng_info;

static int trng_msg_proc(const struct msg_addr *src, void *buf, u32 len)
{
    unsigned int seed_group_number = 0;

    trng_info("receive trng msg");

    if (src == NULL || buf == NULL) {
        return -1;
    }

    if (len != (int)sizeof(seed_group_number)) {
        trng_print("msg read len(%x) != expected len(%lx).", len, sizeof(seed_group_number));
        return -1;
    }
    seed_group_number = *((unsigned int *)buf);
    if (seed_group_number != 1) {
        trng_print("para error, seed_group_number = (%x).", seed_group_number);
        return -1;
    }

    pm_stay_awake(g_trng_info.dev);
    queue_work(g_trng_info.trng_wq, &g_trng_info.trng_work);

    trng_info("trng schedule_work done.");

    return 0;
}

void trng_seed_work(struct work_struct *work)
{
    int ret;
    FUNC_CMD_ID cmd = FUNC_TRNG_SEED_REQUEST;

    ret = bsp_sec_call(cmd, 1);
    if (ret != 0) {
        trng_print("sec_call error,ret is %d.", ret);
    }

    trng_info("trng work done.");
    pm_relax(g_trng_info.dev);

    return;
}

int modem_trng_probe(struct platform_device *pdev)
{
    int ret;
    struct msgchn_attr lite_attr = { 0 };

    g_trng_info.dev = &pdev->dev;

    bsp_msgchn_attr_init(&lite_attr);
    lite_attr.chnid = MSG_CHN_TRNG;
    lite_attr.coremask = MSG_CORE_MASK(MSG_CORE_TSP);
    lite_attr.lite_notify = trng_msg_proc;
    ret = bsp_msg_lite_open(&(g_trng_info.trng_msghdl), &lite_attr);
    if (ret) {
        trng_print("mdrv_msg_register failed,ret=0x%x.", ret);
        return -1;
    }

    device_init_wakeup(g_trng_info.dev, true);

    INIT_WORK(&(g_trng_info.trng_work), trng_seed_work);
    g_trng_info.trng_wq = alloc_ordered_workqueue("modem_trng_wq", 0);
    if (g_trng_info.trng_wq == NULL) {
        trng_print("trng wq create fail.");
        return -1;
    }
    trng_print("[init] trng init OK!");
    return 0;
}
static struct platform_device g_modem_trng_device = {
    .name = "modem_trng",
    .id = -1,
};

struct platform_driver g_modem_trng_driver = {
    .probe      = modem_trng_probe,
    .driver     = {
        .name     = "modem_trng",
        .owner    = THIS_MODULE,
        .probe_type = PROBE_FORCE_SYNCHRONOUS,
    },
};

int bsp_trng_seed_init(void)
{
    int ret;

    ret = platform_device_register(&g_modem_trng_device);
    if (ret) {
        trng_print("modem trng device register err %d.", ret);
        return ret;
    }

    ret = platform_driver_register(&g_modem_trng_driver);
    if (ret) {
        platform_device_unregister(&g_modem_trng_device);
        trng_print("modem trng driver register err %d.", ret);
    }
    return ret;
}

#if !IS_ENABLED(CONFIG_MODEM_DRIVER)
module_init(bsp_trng_seed_init);
#endif
