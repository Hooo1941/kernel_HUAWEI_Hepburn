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

#include "sim_hw_service.h"
#include "sim_hw_mgr.h"
#include "sim_hotplug.h"
#include "securec.h"
#include <adrv_sim.h>
#include <bsp_print.h>

#include "msg_id.h"

#undef THIS_MODU
#define THIS_MODU mod_simhotplug

struct sim_platform_info *g_simhp_info[MAXSIMHPNUM];

int sim_inquire_msg_proc(struct sim_platform_info *info)
{
    int ret = 0;
    sci_msg_data_s sci_msg;
    switch (info->msgfromsci.msg_value) {
        case SIM_CARD_TYPE_INQUIRE:
            if (STATUS_SD2JTAG == get_card1_type()) {
                ret = SIM_CARD_TYPE_JTAG;
            } else {
                ret = SIM_CARD_TYPE_SIM;
            }
            break;

        case SIM_CARD_DET_WHILE_ESIM_INQUIRE:
            sim_log_err("no card det while esim\n");
            ret = SIM_CARD_NO_EXIST_WHILE_ESIM;
            break;

        default:
            sim_log_err("invalid inquire msg.\n");
            ret = SIM_INVALID_INQUIRE_MSG;
            break;
    }

    sci_msg.msg_type = SIM_MSG_INQUIRE_TYPE;
    sci_msg.msg_value = ret;
    ret = sci_send_msg(info, &sci_msg);
    if (ret) {
        sim_log_err("in inquire msg proc, sci_send_msg failed.\n");
    }
    return ret;
}

int sim_power_msg_proc(struct sim_platform_info *info)
{
    int ret = 0;
    sci_msg_data_s sci_msg;

    if (info->msgfromsci.msg_value < SIM_POWER_MSG_BUTT) {
        ret = sim_pmu_ctrl(info);
    } else {
        sim_log_err("invalid power msg.\n");
        ret = SIM_INVALID_POWER_MSG;
    }

    sci_msg.msg_type = SIM_MSG_POWER_TYPE;
    sci_msg.msg_value = ret;
    ret = sci_send_msg(info, &sci_msg);
    if (ret) {
        sim_log_err("send msg failed.\n");
    }
    return ret;
}


int sim_switch_msg_proc(struct sim_platform_info *info)
{
    int ret = 0;
    sci_msg_data_s sci_msg;
    switch (info->msgfromsci.msg_value) {
        case SIM_SET_ESIM_SWITCH:
        case SIM_SET_USIM_SWITCH:
            ret = SIM_ESIM_SWITCH_SUCC;
            sim_log_err("sim switch msg.\n");
            break;

        default:
            sim_log_err("invalid switch msg.\n");
            ret = SIM_INVALID_SWITCH_MSG;
            break;
    }

    sci_msg.msg_type = SIM_MSG_SWITCH_TYPE;
    sci_msg.msg_value = ret;

    ret = sci_send_msg(info, &sci_msg);
    if (ret) {
        sim_log_err("in switch msg proc, sci_send_msg failed.\n");
    }

    return ret;
}


static void sim_product_msg_proc(struct sim_platform_info *info)
{
    int ret;
    sci_msg_data_s sci_msg;
    u32 conv_msg;

    sci_msg.msg_type = info->msgfromsci.msg_type;
    conv_msg = info->msgfromsci.msg_type * SIM_HW_OPS_PER_TYPE + info->msgfromsci.msg_value;
    sim_log_err("hw_adp handle for sim_id %d with msg_type 0x%x.\n", info->sim_id, conv_msg);

    ret = sim_hwadp_handler(info->sim_id, conv_msg);
    sci_msg.msg_value = ret - sci_msg.msg_type * SIM_HW_ERR_CODE_PER_TYPE;

    sim_log_err("sim_id%d: hw_adp ret with msg_type %d, msg_value %d.\n", info->sim_id,
        sci_msg.msg_type, sci_msg.msg_value);
    ret = sci_send_msg(info, &sci_msg);
    if (ret) {
        sim_log_err("hwadp msg handler, sci_send_msg failed.\n");
    }
}

int  sci_exc_event_msg_proc(struct sim_platform_info *info)
{
    sim_log_err("sim%d exc event: 0x%x\n", info->sim_id, info->msgfromsci.msg_value);
    return 0;
}

static void sim_udp_msg_proc(struct sim_platform_info *info)
{
    int ret = 0;
    switch (info->msgfromsci.msg_type) {
        case SIM_MSG_INQUIRE_TYPE:
            ret = sim_inquire_msg_proc(info);
            break;
        case SIM_MSG_POWER_TYPE:
            ret = sim_power_msg_proc(info);
            break;
        case SIM_MSG_EXC_EVENT_TYPE:
            ret = sci_exc_event_msg_proc(info);
            break;

        default:
            ret = -1;
            break;
    }
    sim_log_err("sim%d msg proc done, ret %d.\n", info->sim_id, ret);
}

static int sci_msg_proc(const struct msg_addr *src, void *buf, u32 len)
{
    int ret;
    struct sim_platform_info *info = NULL;
    // 入参检查
    if (src == NULL || buf == NULL || len != sizeof(sci_msg_data_s)) {
        return -1;
    }
    // 消息来源检查
    if (src->core != MSG_CORE_TSP) {
        return -1;
    }
    if (src->chnid == MSG_CHN_SCI0) {
        info = sim_get_plat_info(SIM0);
    } else if (src->chnid == MSG_CHN_SCI1) {
        info = sim_get_plat_info(SIM1);
    }

    if (info == NULL) {
        return -1;
    }

    ret = memcpy_s(&(info->msgfromsci), sizeof(sci_msg_data_s), buf, sizeof(sci_msg_data_s));
    if (ret != 0) {
        sim_log_err("sci msg memcpy failed.\n");
        return -1;
    }

    sim_log_err("sim%d msg type: %d value: %d\n", info->sim_id, info->msgfromsci.msg_type, info->msgfromsci.msg_value);

    if (info->is_udp == 1) {
        sim_udp_msg_proc(info);
    } else {
        sim_product_msg_proc(info);
    }
    return 0;
}

int fusion_sci_msg_init(struct sim_platform_info *info)
{
    int ret;
    struct msgchn_attr lite_attr = { 0 };

    bsp_msgchn_attr_init(&lite_attr);

    if (info->sim_id == SIM0) {
        info->sim_msg_ctrl.chnid = MSG_CHN_SCI0;
    } else if (info->sim_id == SIM1) {
        info->sim_msg_ctrl.chnid = MSG_CHN_SCI1;
    } else {
        sim_log_err("wrong sci_id, sci msg init failed.");
        return -1;
    }

    lite_attr.chnid = info->sim_msg_ctrl.chnid;
    lite_attr.coremask = MSG_CORE_MASK(MSG_CORE_TSP);
    lite_attr.lite_notify = sci_msg_proc;
    ret = bsp_msg_lite_open(&info->sim_msg_ctrl.msghdl, &lite_attr);
    if (ret) {
        return -1;
    }

    return 0;
}

int sci_send_msg(struct sim_platform_info *info, sci_msg_data_s *sci_msg)
{
    struct msg_addr dst;

    dst.core = MSG_CORE_TSP;
    dst.chnid = info->sim_msg_ctrl.chnid;

    if (bsp_msg_lite_sendto(info->sim_msg_ctrl.msghdl, &dst, sci_msg, sizeof(sci_msg_data_s)) != 0) {
        return -1;
    }
    return 0;
}

static int sim_hw_service_probe(struct spmi_device *pdev)
{
    struct device *dev = NULL;
    struct device_node *np = NULL;
    struct sim_platform_info *info = NULL;
    u32 sim_id = 0;
    u32 is_udp = 0;
    int ret;

    sim_log_info("probe sim_hw_service\n");

    if (pdev == NULL) {
        sim_log_err("pdev is NULL.\n");
        return -EINVAL;
    }

    dev = &pdev->dev;
    np = dev->of_node;

    info = devm_kzalloc(&pdev->dev, sizeof(*info), GFP_KERNEL);
    if (info == NULL) {
        sim_log_err("failed to allocate memory.\n");
        return -ENOMEM;
    }

    ret = of_property_read_u32_array(np, "sim_id", &sim_id, 1);
    if (ret) {
        sim_log_err("no sim hotplug sim_id, so return.\n");
        return ret;
    }

    if (sim_id == SIM1 || sim_id == SIM0) {
        info->sim_id = sim_id;
    } else {
        sim_log_err("invalid sim id%d.\n", info->sim_id);
        return -EINVAL;
    }
    ret = of_property_read_u32_array(np, "is_udp", &is_udp, 1);
    if (ret) {
        sim_log_err("no is_udp defined, treat as udp.\n");
        is_udp = 1;
    }
    info->is_udp = is_udp;
    g_simhp_info[info->sim_id] = info;
    info->spmidev = dev;
    sim_log_err("info init start for simid %d.\n", info->sim_id);
    (void)fusion_sci_msg_init(info);
    (void)sim_hw_mgr_init(info);
    (void)chip_sim_hotplug_init(info, pdev);

    dev_set_drvdata(&pdev->dev, info);
    sim_log_err("info init done for simid %d.\n", info->sim_id);

    return 0;
}

static void sim_hw_service_remove(struct spmi_device *pdev)
{
    struct sim_platform_info *info = NULL;

    sim_log_err("simhotplug removing.\n");
    info = dev_get_drvdata(&pdev->dev);
    if (info == NULL) {
        sim_log_err("dev_get_drvdata is NULL.\n");
        return;
    }

    mutex_destroy(&info->sim_hotplug_lock);
    wakeup_source_remove(info->sim_hotplug_wklock);
    __pm_relax(info->sim_hotplug_wklock);

    if (info->sim_hotplug_det_wq != NULL) {
        destroy_workqueue(info->sim_hotplug_det_wq);
    }
    if (info->sim_hotplug_hpd_wq != NULL) {
        destroy_workqueue(info->sim_hotplug_hpd_wq);
    }
    if (info->sim_sci_msg_wq != NULL) {
        destroy_workqueue(info->sim_sci_msg_wq);
    }

    if (info->send_msg_to_cp == 1 && info->sim_debounce_delay_wq) {
        sim_log_info("remove delay work for sim%d\n", info->sim_id);
        cancel_delayed_work(&info->sim_debounce_delay_work);
        flush_workqueue(info->sim_debounce_delay_wq);
        destroy_workqueue(info->sim_debounce_delay_wq);
    }

    if (info->sim_id == SIM1) {
        remove_simhp_node(info->sim_id);
    }

    g_simhp_info[info->sim_id] = NULL;
    dev_set_drvdata(&pdev->dev, NULL);

    return;
}

#ifdef CONFIG_PM
static int sim_hw_service_suspend(struct spmi_device *pdev, pm_message_t state)
{
    struct sim_platform_info *info = NULL;

    info = dev_get_drvdata(&pdev->dev);
    if (info == NULL) {
        sim_log_err("dev_get_drvdata is NULL.\n");
        return -1;
    }

    if (!mutex_trylock(&info->sim_hotplug_lock)) {
        sim_log_err("mutex_trylock failed.\n");
        return 0; /* only show fail, will be ok after next resume */
    }

    return 0;
}

static int sim_hw_service_resume(struct spmi_device *pdev)
{
    struct sim_platform_info *info = NULL;

    info = dev_get_drvdata(&pdev->dev);
    if (info == NULL) {
        sim_log_err("dev_get_drvdata is NULL.\n");
        return -1;
    }

    mutex_unlock(&(info->sim_hotplug_lock));

    return 0;
}
#endif

struct sim_platform_info *sim_get_plat_info(unsigned int sim_id)
{
    if (sim_id < MAXSIMHPNUM) {
        return g_simhp_info[sim_id];
    } else {
        sim_log_err("wrong simid while get platinfo.\n");
        return NULL;
    }
}

static struct of_device_id g_sim_service_of_match[] = {
    {
        .compatible = CHIP_SIM_HOTPLUG0,
    },
    {
        .compatible = CHIP_SIM_HOTPLUG1,
    },
    {},
};

MODULE_DEVICE_TABLE(of, g_sim_service_of_match);

static struct spmi_driver g_sim_service_driver = {
    .probe = sim_hw_service_probe,
    .remove = sim_hw_service_remove,
    .driver = {
        .owner = THIS_MODULE,
        .name = "sim_hw_service",
        .probe_type = PROBE_FORCE_SYNCHRONOUS,
        .of_match_table = of_match_ptr(g_sim_service_of_match),
    },
#ifdef CONFIG_PM
    .suspend = sim_hw_service_suspend,
    .resume = sim_hw_service_resume,
#endif
};

int sim_hw_service_init(void)
{
    return spmi_driver_register(&g_sim_service_driver);
}

static void __exit sim_hw_service_exit(void)
{
    spmi_driver_unregister(&g_sim_service_driver);
}

#if !IS_ENABLED(CONFIG_MODEM_DRIVER)
late_initcall(sim_hw_service_init);
module_exit(sim_hw_service_exit);
#endif
MODULE_DESCRIPTION("Sim HW Service driver");
MODULE_LICENSE("GPL v2");
