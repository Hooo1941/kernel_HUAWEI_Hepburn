/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
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
#include <linux/kernel.h>
#include <osl_types.h>
#include <bsp_dt.h>
#include <bsp_print.h>
#include <dump_modid.h>
#include <adrv_noc.h>

#undef THIS_MODU
#define THIS_MODU mod_noc

#define MDM_NOC_REG_CNT  5

int noc_dump_register_init(void)
{
    int ret;
    unsigned int number;
    unsigned int index = 0;
    device_node_s *child = NULL;
    device_node_s *dev_node = NULL;
    struct noc_err_para_s  mdm_noc_para = { 0 };

    dev_node = bsp_dt_find_compatible_node(NULL, NULL, "modem,peri_noc");
    if (dev_node == NULL) {
        return BSP_ERROR;
    }

    if (bsp_dt_property_read_u32(dev_node, "number", &number)) {
        return BSP_ERROR;
    }
    if (number > MDM_NOC_REG_CNT) {
        return BSP_ERROR;
    }

    bsp_dt_for_each_child_of_node(dev_node, child) {
        ret = bsp_dt_property_read_u32(child, "masterid", &mdm_noc_para.masterid);
        if (ret) {
            return BSP_ERROR;
        }
        ret = bsp_dt_property_read_u32(child, "targetflow", &mdm_noc_para.targetflow);
        if (ret) {
            return BSP_ERROR;
        }

        ret = bsp_dt_property_read_u32(child, "bus", &mdm_noc_para.bus);
        if (ret) {
            return BSP_ERROR;
        }

        noc_modid_register(mdm_noc_para, RDR_MODEM_NOC_MOD_ID);

        index++;
        if (index > number) {
            return BSP_ERROR;
        }
    }
    bsp_err("noc dump register ok\n");

    return BSP_OK;
}

#if !IS_ENABLED(CONFIG_MODEM_DRIVER)
module_init(noc_dump_register_init);
#endif

