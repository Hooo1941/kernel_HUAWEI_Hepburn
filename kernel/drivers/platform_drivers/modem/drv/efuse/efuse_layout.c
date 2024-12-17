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
#include <bsp_efuse.h>
#include <bsp_dt.h>
#include <securec.h>
#include "efuse_layout.h"
#include "efuse_error.h"

#define THIS_MODU mod_efuse
efuse_layout_info_s *g_efuse_layout_info = NULL;

efuse_info_s g_efuse_layout_table[] = {
    {"gj_hbk", EFUSE_DRV_GJ_HBK},
    {"die_id", EFUSE_DRV_DIE_ID},
    {"huk", EFUSE_DRV_HUK},
    {"chip_id", EFUSE_CHIP_ID},
    {"apb_rd_huk_dis", EFUSE_APB_RD_HUK_DIS},
    {"apb_rd_scp_dis", EFUSE_APB_RD_SCP_DIS},
    {"arm_dbg_ctrl", EFUSE_ARM_DBG_CTRL},
    {"mp_flags", EFUSE_MP_FLAG},
    {"kce", EFUSE_KCE},
    {"DFT_key", EFUSE_DFT_AUTH_KEY},
    {"usb_boot_dis", EFUSE_USB_BOOT_DIS_CTRL},
    {"coresight_rst_ctrl", EFUSE_CORESIGHT_RST_CTRL},
    {"hifi_dbg_ctrl", EFUSE_HIFI_DBG_CTRL},
    {"coresight_device_ctrl", EFUSE_CS_DEVICE_CTRL},
    {"uart_dbg_ctrl", EFUSE_UART_DBG_CTRL},
    {"pde_dbg_ctrl", EFUSE_PDE_DBG_CTRL},
    {"txp_niden", EFUSE_TXP_NIDEN_CTRL},
    {"acpu_niden", EFUSE_ACPU_NIDEN_CTRL},
    {"sec_dbg_rst_ctrl", EFUSE_SEC_DBG_RST_CTRL},
    {"dft_disable_sel", EFUSE_DFT_DISABLE_SEL},
    {"boot_sel", EFUSE_BOOT_SEL},
    {"DFT_key_rd_ctrl", EFUSE_DFT_AUTH_KEY_RD_CTRL},
    {"arm_dbg_ctrl_dbgen", EFUSE_ARM_DBG_CTRL_DBGEN},
    {"arm_dbg_ctrl_niden", EFUSE_ARM_DBG_CTRL_NIDEN},
    {"arm_dbg_ctrl_spiden", EFUSE_ARM_DBG_CTRL_SPIDEN},
    {"arm_dbg_ctrl_spniden", EFUSE_ARM_DBG_CTRL_SPNIDEN},
    {"jtagen_ctrl", EFUSE_JTAGEN_CTRL},
    {"gmecc_key_rd_ctrl", EFUSE_GMECC_KEY_RD_CTRL},
    {"5g_lan_mac_addr", EFUSE_DRV_5G_LAN_MAC_ADDR},
    {"acpu_op85_freq", EFUSE_DRV_ACPU_OP85_FREQ},
    {"tsp_op85_freq", EFUSE_DRV_TSP_OP85_FREQ},
    {"acpu_only", EFUSE_DRV_ACPU_ONLY},
    {"tsensor_ref", EFUSE_DRV_TSENSOR_REF},
    {"tsensor_000", EFUSE_DRV_TSENSOR_000},
    {"tsensor_001", EFUSE_DRV_TSENSOR_001},
    {"dmc_avs_test_pass", EFUSE_DRV_DMC_AVS_TEST_PASS},
    {"peri_mdm_avs_test_pass", EFUSE_DRV_PERI_MDM_AVS_TEST_PASS},
    {"pansensor_dmc", EFUSE_DRV_PASENSOR_DMC},
    {"pansensor_mdm", EFUSE_DRV_PASENSOR_MDM},
    {"avs_disable", EFUSE_DRV_AVS_DISABLE},
    {"vehicle_flags", EFUSE_DRV_VEHICLE_FLAGS},
    {"peri_mdm_avs_volt_pass", EFUSE_DRV_PERI_MDM_AVS_VOLT_PASS},
    {"dmc_avs_volt_pass", EFUSE_DRV_DMC_AVS_VOLT_PASS},
    {"usb_vendor_device_id", EFUSE_DRV_USB_VENDOR_DEVICE_ID},
    {"bbp_iddq", EFUSE_MDRV_BBP_IDDQ},
    {"bbp_pdt_op85_freq", EFUSE_MDRV_BBP_PDT_OP85_FREQ},
    {"bbp_debug_mem", EFUSE_MDRV_BBP_DEBUFMEM},
    {"bbp_23g_mem", EFUSE_MDRV_BBP_23GMEM},
    {"bbp_pdf0_bypass", EFUSE_MDRV_BBP_PDF0BYPASS},
    {"bbp_pdf1_bypass", EFUSE_MDRV_BBP_PDF1BYPASS},

    {"product_regions", EFUSE_MDRV_PRODUCT_REGIONS},

    {"efuse_layout_max", EFUSE_LAYOUT_ID_MAX},
};

u32 bsp_efuse_get_max_grp_num(void)
{
    return g_efuse_layout_info->max_num;
}

static int efuse_find_name_id(const char *name, efuse_layout_s *info)
{
    u32 i = 0;
    for (i = 0; i < sizeof(g_efuse_layout_table) / sizeof(g_efuse_layout_table[0]); i++) {
        if (strcmp(name, g_efuse_layout_table[i].name) == 0) {
            info->name_id = g_efuse_layout_table[i].id;
            return EFUSE_OK;
        }
    }

    return EFUSE_ERROR_GET_LAYOUT_INFO;
}
int bsp_efuse_get_layout_info(efuse_layout_id_e name_id, efuse_layout_s *layout_info)
{
    u32 i;

    for (i = 0; i < g_efuse_layout_info->info_num; i++) {
        if (name_id == g_efuse_layout_info->info[i].name_id) {
            layout_info->name_id = name_id;
            layout_info->bit = g_efuse_layout_info->info[i].bit;
            layout_info->bit_len = g_efuse_layout_info->info[i].bit_len;
            return EFUSE_OK;
        }
    }
    efuse_print_error("can't find %d layout info", name_id);
    return EFUSE_ERROR;
}
EXPORT_SYMBOL_GPL(bsp_efuse_get_layout_info);

static int layout_node_parse(device_node_s *parent)
{
    int ret;
    u32 num = 0;
    const char *name = NULL;
    device_node_s temp_node;
    device_node_s *child_node = &temp_node;

    ret = bsp_dt_get_child_count(parent);
    if (ret <= 0) {
        goto efuse_dts_end;
    }
    g_efuse_layout_info->info_num = ret;

    g_efuse_layout_info->info = kzalloc(g_efuse_layout_info->info_num * sizeof(efuse_layout_s), GFP_KERNEL);
    if (IS_ERR_OR_NULL(g_efuse_layout_info)) {
        efuse_print_error("malloc buffer fail\n");
        return EFUSE_ERROR_BUFFER_EMPTY;
    }

    if (memset_s(g_efuse_layout_info->info, (size_t)(g_efuse_layout_info->info_num * sizeof(efuse_layout_s)), 
        0, (size_t)(g_efuse_layout_info->info_num + sizeof(efuse_layout_s)))) {
        return EFUSE_ERROR_BUFFER_EMPTY;
    }

    bsp_dt_for_each_child_of_node(parent, child_node)
    {
        if (bsp_dt_property_read_string(child_node, "efuse_name", &name)) {
            return EFUSE_ERROR_GET_LAYOUT_INFO;
        }

        if (efuse_find_name_id(name, &g_efuse_layout_info->info[num])) {
            efuse_print_error("name id get failed");
            return EFUSE_ERROR_GET_LAYOUT_INFO;
        }

        if (bsp_dt_property_read_u32_array(child_node, "bit", &g_efuse_layout_info->info[num].bit, 1)) {
            return EFUSE_ERROR_GET_LAYOUT_INFO;
        }

        if (bsp_dt_property_read_u32_array(child_node, "bit_len", &g_efuse_layout_info->info[num].bit_len, 1)) {
            return EFUSE_ERROR_GET_LAYOUT_INFO;
        }
        num++;
    }

    return EFUSE_OK;
efuse_dts_end:
    g_efuse_layout_info->info_num = 0;
    return EFUSE_ERROR;
}

int efuse_layout_init(void)
{
    int ret;
    device_node_s *node;

    node = bsp_dt_find_compatible_node(NULL, NULL, "modem,efuse_app");
    if (node == NULL) {
        efuse_print_error("device node not found\n");
        return EFUSE_ERROR;
    }

    g_efuse_layout_info = kzalloc(sizeof(efuse_layout_info_s), GFP_KERNEL);
    if (IS_ERR_OR_NULL(g_efuse_layout_info)) {
        efuse_print_error("malloc buffer fail\n");
        return EFUSE_ERROR_BUFFER_EMPTY;
    }

    ret = bsp_dt_property_read_u32(node, "group_num", &g_efuse_layout_info->max_num);
    if (ret) {
        efuse_print_error("dts group_num get failed.");
        return ret;
    }

    ret = layout_node_parse(node);
    if (ret) {
        efuse_print_error("child node info get failed.");
        return ret;
    }
    return EFUSE_OK;
}
