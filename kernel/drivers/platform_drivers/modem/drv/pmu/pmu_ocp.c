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
#include <linux/module.h>
#include <linux/init.h>
#include <adrv_pmu.h>
#include <bsp_dt.h>
#include <osl_list.h>
#include <osl_malloc.h>
#include <bsp_print.h>
#include <product_config.h>
#include <msg_id.h>
#include <bsp_msg.h>
#define THIS_MODU mod_ocp
/*lint --e{64}*/ /* 64:list_for_each_entry */
#define pmic_print_error(fmt, ...) (bsp_err("<%s> " fmt, __FUNCTION__, ##__VA_ARGS__))

struct pmic_volt_ocp {
    int volt_id;
    const char *volt_name;
    struct list_head list;
};

LIST_HEAD(g_modem_ocp_list);
static msg_chn_t g_pmu_ocp_msghdl;

void pmic_cop_list_debug(void)
{
    struct pmic_volt_ocp *ocp_node = NULL;
    list_for_each_entry(ocp_node, &g_modem_ocp_list, list)
    {
        pmic_print_error("volt id %d, volt name %s .\n", ocp_node->volt_id, (char *)ocp_node->volt_name);
    }
}

static int pmic_sent_event(struct pmic_volt_ocp *ocp_node)
{
    int ret;
    struct msg_addr dst;
    dst.core = MSG_CORE_TSP;
    dst.chnid = MSG_CHN_PMU_OCP;

    if (ocp_node == NULL) {
        pmic_print_error("ocp node is null.\n");
        return (-1);
    }

    ret = bsp_msg_lite_sendto(g_pmu_ocp_msghdl, &dst, &ocp_node->volt_id, sizeof(int));
    if (ret) {
        pmic_print_error("voit_id: %d send msg failed, ret = %d\n", ocp_node->volt_id, ret);
    } else {
        pmic_print_error("send volt id %d, %s to cp.\n", ocp_node->volt_id, ocp_node->volt_name);
    }
    return ret;
}

int pmic_msg_init(void)
{
    int ret;
    struct msgchn_attr lite_attr = {0};

    bsp_msgchn_attr_init(&lite_attr);
    lite_attr.chnid = MSG_CHN_PMU_OCP;
    lite_attr.coremask = MSG_CORE_MASK(MSG_CORE_TSP);
    lite_attr.lite_notify = NULL;
    ret = bsp_msg_lite_open(&g_pmu_ocp_msghdl, &lite_attr);
    if (ret != 0) {
        bsp_err("bsp_msg_lite_open fail, ret:%d\n", ret);
        return ret;
    }

    return 0;
}

static int pmic_modem_ocp_handler(char *name)
{
    struct pmic_volt_ocp *ocp_node = NULL;
    if (name == NULL) {
        pmic_print_error("argc is null return\n");
        return (-1);
    }
    /* get name form id */
    list_for_each_entry(ocp_node, &g_modem_ocp_list, list)
    {
        if (!strncmp(ocp_node->volt_name, name, (strlen(name) + 1))) {
            pmic_print_error("handle ocp_volt %s.\n", name);
            break;
        }
    }

    return pmic_sent_event(ocp_node);
}

int pmic_ocp_dts_init(void)
{
    int ret;
    device_node_s *root = NULL;
    device_node_s *child_node = NULL;
    struct pmic_volt_ocp *ocp = NULL;

    /* get dts info */
    root = bsp_dt_find_compatible_node(NULL, NULL, "pmic_ocp_modem");
    if (root == NULL) {
        pmic_print_error("get pmic dts node failed!\n");
        return -1;
    }

    bsp_dt_for_each_available_child_of_node(root, child_node)
    {
        ocp = (struct pmic_volt_ocp *)osl_malloc(sizeof(struct pmic_volt_ocp));
        if (ocp == NULL) {
            pmic_print_error("ocp malloc failed \n");
            return (-1);
        }
        INIT_LIST_HEAD(&ocp->list);
        ret = bsp_dt_property_read_u32(child_node, "id", (u32 *)&ocp->volt_id);
        ret += bsp_dt_property_read_string(child_node, "ocp_name", &ocp->volt_name);
        list_add(&ocp->list, &g_modem_ocp_list);
        if (ret) {
            pmic_print_error("modem pmic ocp read fail! ret is %d\n", ocp->volt_name, ret);
            return ret;
        }
        ret = pmic_special_ocp_register((char *)(ocp->volt_name), pmic_modem_ocp_handler);
        if (ret) {
            pmic_print_error("modem(%s) ocp register fail! ret is %d\n", ocp->volt_name, ret);
            return ret;
        }
        pmic_print_error("modem(%s) ocp register success. \n", ocp->volt_name);
    }

    return 0;
}

void pmu_ocp_handler_debug(void)
{
    (void)pmic_modem_ocp_handler("ldo28");
}

/*lint -save -e429*/
int pmic_modem_ocp_init(void)
{
    int ret;

    ret = pmic_msg_init();
    if (ret) {
        pmic_print_error("pmic msg init failed!\n");
        return ret;
    }

    ret = pmic_ocp_dts_init();
    if (ret) {
        pmic_print_error("pmic ocp dts failed!\n");
        return ret;
    }

    pmic_cop_list_debug();
    pmic_print_error("pmic modem ocp init ok!\n");

    return ret;
}

EXPORT_SYMBOL(pmic_cop_list_debug);
EXPORT_SYMBOL(pmu_ocp_handler_debug);
#if !IS_ENABLED(CONFIG_MODEM_DRIVER)
/*lint -restore +e429*/
module_init(pmic_modem_ocp_init);
#endif
