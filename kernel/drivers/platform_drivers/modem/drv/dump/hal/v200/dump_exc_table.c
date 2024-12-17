/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
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

#include "dump_modid.h"
#include "dump_exc_table.h"

/* e_exce_type 在EXCH_SOURCE枚举中申明，需要AP和MODEM同时修改
 * AP侧modem exc后面的枚举是其他组件，不易变更，因此这里对于目前不存在的exc_type
 * 都使用相似的枚举来表示异常原因，差异大的，直接使用CP_S_EXCEPTION
 */
dump_mdm_exc_table_info_s g_dump_mdmcp_exc_table[] = {
    {
        RDR_CP, CP_S_EXCEPTION, RDR_REBOOT_WAIT, RDR_MODEM_CP_CPU_EXC_MOD_ID,
        RDR_MODEM_CP_CPU_EXC_MOD_ID, DUMP_MDM_EXC_TYPE(RDR_MODEM_CP_COMMON_MOD_ID)
    },
    {
        RDR_CP, CP_S_EXCEPTION, RDR_REBOOT_WAIT, RDR_MODEM_CP_WDT_MOD_ID,
        RDR_MODEM_CP_WDT_MOD_ID, DUMP_MDM_EXC_TYPE(RDR_MODEM_CP_WDT_MOD_ID)
    },
    {
        RDR_CP, CP_S_DRV_EXC, RDR_REBOOT_WAIT, RDR_MODEM_CP_DRV_MOD_ID_START,
        RDR_MODEM_CP_DRV_MOD_ID_END, DUMP_MDM_EXC_TYPE(RDR_MODEM_CP_DRV_MOD_ID)
    },
    {
        RDR_CP, CP_S_PAM_EXC, RDR_REBOOT_WAIT, RDR_MODEM_CP_PAM_MOD_ID_START,
        RDR_MODEM_CP_PAM_MOD_ID_END, DUMP_MDM_EXC_TYPE(RDR_MODEM_CP_PAM_MOD_ID)
    },
    {
        RDR_CP, CP_S_CTTF_EXC, RDR_REBOOT_WAIT, RDR_MODEM_CP_GUTTF_MOD_ID_START,
        RDR_MODEM_CP_GUTTF_MOD_ID_END, DUMP_MDM_EXC_TYPE(RDR_MODEM_CP_GUTTF_MOD_ID)
    },
    {
        RDR_CP, CP_S_GUAS_EXC, RDR_REBOOT_WAIT, RDR_MODEM_CP_GUWAS_MOD_ID_START,
        RDR_MODEM_CP_GUWAS_MOD_ID_END, DUMP_MDM_EXC_TYPE(RDR_MODEM_CP_GUWAS_MOD_ID)
    },
    {
        RDR_CP, CP_S_GUAS_EXC, RDR_REBOOT_WAIT, RDR_MODEM_CP_GUGAS_MOD_ID_START,
        RDR_MODEM_CP_GUGAS_MOD_ID_END, DUMP_MDM_EXC_TYPE(RDR_MODEM_CP_GUGAS_MOD_ID)
    },
    {
        RDR_CP, CP_S_GUCNAS_EXC, RDR_REBOOT_WAIT, RDR_MODEM_CP_GUNRNAS_MOD_ID_START,
        RDR_MODEM_CP_GUNRNAS_MOD_ID_START, DUMP_MDM_EXC_TYPE(RDR_MODEM_CP_GUNRNAS_MOD_ID)
    },
    {
        RDR_CP, CP_S_CPHY_EXC, RDR_REBOOT_WAIT, RDR_MODEM_CP_GUCPHY_MOD_ID_START,
        RDR_MODEM_CP_GUCPHY_MOD_ID_END, DUMP_MDM_EXC_TYPE(RDR_MODEM_CP_GUCPHY_MOD_ID)
    },
    {
        RDR_CP, CP_S_EXCEPTION, RDR_REBOOT_WAIT, RDR_MODEM_CP_EASYRF_MOD_ID_START,
        RDR_MODEM_CP_EASYRF_MOD_ID_END, DUMP_MDM_EXC_TYPE(RDR_MODEM_CP_EASYRF_MOD_ID)
    },
    {
        RDR_CP, CP_S_TLPS_EXC, RDR_REBOOT_WAIT, RDR_MODEM_CP_LNAS_MOD_ID_START,
        RDR_MODEM_CP_LNAS_MOD_ID_END, DUMP_MDM_EXC_TYPE(RDR_MODEM_CP_LNAS_MOD_ID)
    },
    {
        RDR_CP, CP_S_TLPS_EXC, RDR_REBOOT_WAIT, RDR_MODEM_CP_LRRC_MOD_ID_START,
        RDR_MODEM_CP_LRRC_MOD_ID_END, DUMP_MDM_EXC_TYPE(RDR_MODEM_CP_LRRC_MOD_ID)
    },
    {
        RDR_CP, CP_S_TLPS_EXC, RDR_REBOOT_WAIT, RDR_MODEM_CP_NLL2_MOD_ID_START,
        RDR_MODEM_CP_NLL2_MOD_ID_END, DUMP_MDM_EXC_TYPE(RDR_MODEM_CP_NLL2_MOD_ID)
    },
    {
        RDR_CP, CP_S_EXCEPTION, RDR_REBOOT_WAIT, RDR_MODEM_CP_VOICE_MOD_ID_START,
        RDR_MODEM_CP_VOICE_MOD_ID_END, DUMP_MDM_EXC_TYPE(RDR_MODEM_CP_VOICE_MOD_ID)
    },
    {
        RDR_CP, CP_S_EXCEPTION, RDR_REBOOT_WAIT, RDR_MODEM_CP_UPHY_MOD_ID_START,
        RDR_MODEM_CP_UPHY_MOD_ID_END, DUMP_MDM_EXC_TYPE(RDR_MODEM_CP_UPHY_MOD_ID)
    },
    {
        RDR_CP, CP_S_EXCEPTION, RDR_REBOOT_WAIT, RDR_MODEM_CP_FESSBB_MOD_ID_START,
        RDR_MODEM_CP_FESSBB_MOD_ID_END, DUMP_MDM_EXC_TYPE(RDR_MODEM_CP_FESSBB_MOD_ID)
    },
    {
        RDR_CP, CP_S_EXCEPTION, RDR_REBOOT_WAIT, RDR_MODEM_CP_FESSRF_MOD_ID_START,
        RDR_MODEM_CP_FESSRF_MOD_ID_END, DUMP_MDM_EXC_TYPE(RDR_MODEM_CP_FESSRF_MOD_ID)
    },
    {
        RDR_CP, CP_S_EXCEPTION, RDR_REBOOT_WAIT, RDR_MODEM_CP_GPHY_MOD_ID_START,
        RDR_MODEM_CP_GPHY_MOD_ID_END, DUMP_MDM_EXC_TYPE(RDR_MODEM_CP_GPHY_MOD_ID)
    },
    {
        RDR_CP, CP_S_EXCEPTION, RDR_REBOOT_WAIT, RDR_MODEM_CP_WPHY_MOD_ID_START,
        RDR_MODEM_CP_WPHY_MOD_ID_END, DUMP_MDM_EXC_TYPE(RDR_MODEM_CP_WPHY_MOD_ID)
    },
    {
        RDR_CP, CP_S_EXCEPTION, RDR_REBOOT_WAIT, RDR_MODEM_CP_LPHY_MOD_ID_START,
        RDR_MODEM_CP_LPHY_MOD_ID_END, DUMP_MDM_EXC_TYPE(RDR_MODEM_CP_LPHY_MOD_ID)
    },
    {
        RDR_CP, CP_S_EXCEPTION, RDR_REBOOT_WAIT, RDR_MODEM_CP_NPHY_MOD_ID_START,
        RDR_MODEM_CP_NPHY_MOD_ID_END, DUMP_MDM_EXC_TYPE(RDR_MODEM_CP_NPHY_MOD_ID)
    },
    {
        RDR_CP, CP_S_EXCEPTION, RDR_REBOOT_WAIT, RDR_MODEM_CP_IMS_MOD_ID_START,
        RDR_MODEM_CP_IMS_MOD_ID_END, DUMP_MDM_EXC_TYPE(RDR_MODEM_CP_IMS_MOD_ID)
    },
    {
        RDR_CP, CP_S_EXCEPTION, RDR_REBOOT_WAIT, RDR_MODEM_CP_5GRRC_MOD_ID_START,
        RDR_MODEM_CP_5GRRC_MOD_ID_END, DUMP_MDM_EXC_TYPE(RDR_MODEM_CP_5GRRC_MOD_ID)
    },
    {
        RDR_CP, CP_S_EXCEPTION, RDR_REBOOT_WAIT, RDR_MODEM_CP_MOD_ID,
        RDR_MODEM_CP_MOD_ID, DUMP_MDM_EXC_TYPE(RDR_MODEM_CP_MOD_ID)
    },
};

dump_mdm_exc_table_info_s g_dump_mdmap_exc_table[] = {
    {
        RDR_CP, CP_S_NORMALRESET, RDR_REBOOT_WAIT, RDR_MODEM_CP_RESET_SIM_SWITCH_MOD_ID,
        RDR_MODEM_CP_RESET_SIM_SWITCH_MOD_ID, DUMP_MDM_EXC_TYPE(RDR_MODEM_CP_RESET_SIM_SWITCH_MOD_ID)
    },
    {
        RDR_AP, CP_S_RESETFAIL, RDR_REBOOT_NOW, RDR_MODEM_CP_RESET_FAIL_MOD_ID,
        RDR_MODEM_CP_RESET_FAIL_MOD_ID, DUMP_MDM_EXC_TYPE(RDR_MODEM_CP_RESET_FAIL_MOD_ID)
    },
    {
        RDR_AP, CP_S_RESETFAIL, RDR_REBOOT_NOW, RDR_MODEM_CP_RESET_FREQUENTLY_MOD_ID,
        RDR_MODEM_CP_RESET_FREQUENTLY_MOD_ID, DUMP_MDM_EXC_TYPE(RDR_MODEM_CP_RESET_FREQUENTLY_MOD_ID)
    },
    {
        RDR_CP, CP_S_RILD_EXCEPTION, RDR_REBOOT_WAIT, RDR_MODEM_CP_RESET_RILD_MOD_ID,
        RDR_MODEM_CP_RESET_RILD_MOD_ID, DUMP_MDM_EXC_TYPE(RDR_MODEM_CP_RESET_RILD_MOD_ID)
    },
    {
        RDR_CP, CP_S_3RD_EXCEPTION, RDR_REBOOT_WAIT, RDR_MODEM_CP_RESET_3RD_MOD_ID,
        RDR_MODEM_CP_RESET_3RD_MOD_ID, DUMP_MDM_EXC_TYPE(RDR_MODEM_CP_RESET_3RD_MOD_ID)
    },
    {
        RDR_CP, CP_S_MODEMAP, RDR_REBOOT_WAIT, RDR_MODEM_CP_RESET_DRA_MOD_ID,
        RDR_MODEM_CP_RESET_DRA_MOD_ID, DUMP_MDM_EXC_TYPE(RDR_MODEM_CP_RESET_DRA_MOD_ID)
    },
    {
        RDR_AP, CP_S_NORMALRESET, RDR_REBOOT_NOW, RDR_MODEM_CP_RESET_REBOOT_REQ_MOD_ID,
        RDR_MODEM_CP_RESET_REBOOT_REQ_MOD_ID, DUMP_MDM_EXC_TYPE(RDR_MODEM_CP_RESET_REBOOT_REQ_MOD_ID)
    },
    {
        RDR_AP, CP_S_MODEMNOC, RDR_REBOOT_NOW, RDR_MODEM_NOC_MOD_ID,
        RDR_MODEM_NOC_MOD_ID, DUMP_MDM_EXC_TYPE(RDR_MODEM_NOC_MOD_ID)
    },
    {
        RDR_CP, CP_S_NORMALRESET,RDR_REBOOT_WAIT,  RDR_MODEM_CP_RESET_USER_RESET_MOD_ID,
        RDR_MODEM_CP_RESET_USER_RESET_MOD_ID, DUMP_MDM_EXC_TYPE(RDR_MODEM_CP_RESET_USER_RESET_MOD_ID)
    },
    {
        RDR_AP, CP_S_MODEMDMSS, RDR_REBOOT_NOW, RDR_MODEM_DMSS_MOD_ID,
        RDR_MODEM_DMSS_MOD_ID, DUMP_MDM_EXC_TYPE(RDR_MODEM_DMSS_MOD_ID)
    },
    {   RDR_AP, CP_S_MODEMAP, RDR_REBOOT_NOW, RDR_MODEM_CODE_PATCH_REVERT_MOD_ID,
        RDR_MODEM_CODE_PATCH_REVERT_MOD_ID, DUMP_MDM_EXC_TYPE(RDR_MODEM_CODE_PATCH_REVERT_MOD_ID)
    },
    {
        RDR_AP, CP_S_MODEMAP, RDR_REBOOT_NOW, RDR_MODEM_AP_DRV_MOD_ID_START,
        RDR_MODEM_AP_DRV_MOD_ID_END, DUMP_MDM_EXC_TYPE(RDR_MODEM_AP_DRV_MOD_ID)
    },
    {
        RDR_AP, CP_S_MODEMAP, RDR_REBOOT_NOW, RDR_MODEM_AP_MOD_ID,
        RDR_MODEM_AP_MOD_ID, DUMP_MDM_EXC_TYPE(RDR_MODEM_AP_MOD_ID)
    },
};

dump_mdm_exc_table_info_s *dump_get_cp_exc_tab(u32 *size)
{
    *size = ARRAY_SIZE(g_dump_mdmcp_exc_table);
    return g_dump_mdmcp_exc_table;
}

dump_mdm_exc_table_info_s *dump_get_ap_exc_tab(u32 *size)
{
    *size = ARRAY_SIZE(g_dump_mdmap_exc_table);
    return g_dump_mdmap_exc_table;
}

u32 dump_get_ap_default_tab_idx(void)
{
    return ARRAY_SIZE(g_dump_mdmap_exc_table) - 1;
}
u32 dump_get_cp_default_tab_idx(void)
{
    return ARRAY_SIZE(g_dump_mdmcp_exc_table) - 1;
}

EXPORT_SYMBOL(dump_get_cp_exc_tab);
EXPORT_SYMBOL(dump_get_ap_exc_tab);
