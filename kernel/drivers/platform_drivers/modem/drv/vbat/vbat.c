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
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/power_supply.h>
#include <linux/workqueue.h>
#include <bsp_sysctrl.h>
#include <bsp_dt.h>

#define THIS_MODU mod_vbat
#define VBAT_REG_MASK 0xFFFF0000 /* scbakdata2_msk是带掩码寄存器，当mask bit[x+16]为1时，bit[x]的写才有效 */
#define VBAT_REG_OFFSET_MAX 0x1000

struct vbat_info {
    unsigned int vbat_reg_offset;
    unsigned int time;
    struct delayed_work vbat_set_work;
};

struct vbat_info g_vbat_info = { 0 };

static uintptr_t vbat_get_vbat_reg(void)
{
    return ((uintptr_t)bsp_sysctrl_addr_byindex(SYSCTRL_AO) + g_vbat_info.vbat_reg_offset);
}

static void vbat_set_vbat(u32 value)
{
    writel((value | VBAT_REG_MASK), (volatile void *)(uintptr_t)vbat_get_vbat_reg());
}

static unsigned int vbat_get_curr_vbat(void)
{
    struct power_supply *psy = NULL;
    union power_supply_propval data = { 0 };
    int ret;

    psy = power_supply_get_by_name("Battery");
    if (psy == NULL) {
        return 0;
    }

    /* 判断电池是否在位，若不在位电压赋值0 */
    ret = power_supply_get_property(psy, POWER_SUPPLY_PROP_PRESENT, &data);
    if ((ret != 0) || (data.intval == 0)) {
        bsp_err("batt present:%d, ret=0x%x\n", data.intval, ret);
        power_supply_put(psy);
        return 0;
    }

    ret = power_supply_get_property(psy, POWER_SUPPLY_PROP_VOLTAGE_NOW, &data);
    if (ret) {
        bsp_err("get voltage fail\n");
        data.intval = 0;
    }

    power_supply_put(psy);

    return (unsigned int)data.intval / 1000; /* / 1000: uv to mv */
}

static void vbat_set_work(struct work_struct *work)
{
    unsigned int vbat = vbat_get_curr_vbat();
    bsp_err("voltage:%d\n", vbat);
    vbat_set_vbat(vbat);
    queue_delayed_work(system_wq, &g_vbat_info.vbat_set_work,
        msecs_to_jiffies(g_vbat_info.time));
}

static int vbat_of_node_init(void)
{
    device_node_s *node = NULL;
    int ret;
    node = bsp_dt_find_compatible_node(NULL, NULL, "modem,vbat");
    if (node == NULL) {
        bsp_err("dts node not found!\n");
        return -1;
    }

    ret = bsp_dt_property_read_u32(node, "vbat_reg", &g_vbat_info.vbat_reg_offset);
    if (ret) {
        bsp_err("read vbat_reg from dts failed,ret = %d!\n", ret);
        return -1;
    }
    if (g_vbat_info.vbat_reg_offset >= VBAT_REG_OFFSET_MAX) {
        bsp_err("vbat_reg out of range,vbat_reg = %d!\n", g_vbat_info.vbat_reg_offset);
        return -1;
    }

    ret = bsp_dt_property_read_u32(node, "time", &g_vbat_info.time);
    if (ret) {
        bsp_err("read time from dts failed,ret = %d!\n", ret);
        return -1;
    }

    return 0;
}

int vbat_probe(struct platform_device *pdev)
{
    int ret;

    ret = vbat_of_node_init();
    if (ret) {
        bsp_err("vbat of init err\n");
        return -1;
    }

    vbat_set_vbat(vbat_get_curr_vbat());

    INIT_DELAYED_WORK(&g_vbat_info.vbat_set_work, vbat_set_work);
    queue_delayed_work(system_wq, &g_vbat_info.vbat_set_work,
        msecs_to_jiffies(g_vbat_info.time));

    bsp_err("vbat init ok\n");
    return 0;
}

static int vbat_resume(struct device *dev)
{
    queue_delayed_work(system_wq, &g_vbat_info.vbat_set_work,
        msecs_to_jiffies(0));
    return 0;
}

static int vbat_suspend(struct device *dev)
{
    cancel_delayed_work(&g_vbat_info.vbat_set_work);
    return 0;
}

static const struct dev_pm_ops g_vbat_pmops = {
    .suspend = vbat_suspend,
    .resume = vbat_resume,
};

static struct platform_driver g_vbat_pltfm_driver = {
    .probe = vbat_probe,
    .remove = NULL,
    .driver = {
        .name = "mdm_vbat",
        .owner = THIS_MODULE,
        .pm = &g_vbat_pmops,
        .probe_type = PROBE_FORCE_SYNCHRONOUS,
    },
};

static struct platform_device g_vbat_device = {
    .name = "mdm_vbat",
    .id = -1,
    .dev = {
        .init_name = "mdm_vbat",
    },
};

int bsp_vbat_init(void)
{
    bsp_err("vbat_init\n");
    if (platform_device_register(&g_vbat_device)) {
        bsp_err("dev register fail.\n");
        return -1;
    }

    if (platform_driver_register(&g_vbat_pltfm_driver)) {
        bsp_err("drv register fail.\n");
        return -1;
    }

    return 0;
}

MODULE_LICENSE("GPL v2");
