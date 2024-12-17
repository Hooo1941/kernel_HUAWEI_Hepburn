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
#include <product_config.h>
#include <securec.h>
#include <osl_types.h>
#include <mdrv_msg.h>
#include <msg_id.h>
#include <adrv_om.h>
#include <bsp_print.h>

#undef THIS_MODU
#define THIS_MODU mod_dump

#define RDR_MODEM_REQ_LPMCU_LOG_MOD_ID 0x82ff0001
#define MDMAP_MSG_MAGIC 0xDEADBEEF

struct msg_chn_hdl *g_dump_mdmap_msg_hdl = NULL;

int dump_mdmap_msglite_proc(const struct msg_addr *src, void *msg, unsigned int len)
{
    if ((src == NULL) || (msg == NULL) || (len == 0)) {
        bsp_err("mdmap msglite para null\n");
        return -1;
    }

    if (src->core != MSG_CORE_TSP || src->chnid != MSG_CHN_LOOP1 || len != sizeof(u32)) {
        bsp_err("mdmap msglite para inv\n");
        return -1;
    }

    if (*(u32 *)msg != MDMAP_MSG_MAGIC) {
        bsp_err("mdmap msg magic inv\n");
        return -1;
    }

    rdr_system_error(RDR_MODEM_REQ_LPMCU_LOG_MOD_ID, MDMAP_MSG_MAGIC, 0);

    bsp_err("mdmap msg notify rdr done\n");
    return 0;
}

void dump_mdmap_subsys_exc_agent_init(void)
{
    struct msgchn_attr attr = {0};

    mdrv_msgchn_attr_init(&attr);
    attr.chnid = MSG_CHN_LOOP1;
    attr.coremask = MSG_CORE_MASK(MSG_CORE_TSP);
    attr.lite_notify = dump_mdmap_msglite_proc;

    g_dump_mdmap_msg_hdl = mdrv_msgchn_lite_open(&attr);
    return;
}

void dump_mdmap_fill_lpmcu_log_no_rst_exc_info(struct rdr_exception_info_s *rdr_exc)
{
    rdr_exc->e_modid = (unsigned int)RDR_MODEM_REQ_LPMCU_LOG_MOD_ID;
    rdr_exc->e_modid_end = (unsigned int)RDR_MODEM_REQ_LPMCU_LOG_MOD_ID;
    rdr_exc->e_process_priority = RDR_ERR;
    rdr_exc->e_reboot_priority = RDR_REBOOT_WAIT;
    rdr_exc->e_notify_core_mask = RDR_AP | RDR_MODEMAP | RDR_LPM3;
    rdr_exc->e_reset_core_mask = RDR_MODEMAP;
    rdr_exc->e_from_core = RDR_MODEMAP;
    rdr_exc->e_reentrant = (unsigned int)RDR_REENTRANT_ALLOW;
    rdr_exc->e_exce_type = CP_S_NORMALRESET;
    rdr_exc->e_upload_flag = (unsigned int)RDR_UPLOAD_YES;
    rdr_exc->e_save_log_flags = RDR_SAVE_LOGBUF;
    if (strcpy_s((char *)rdr_exc->e_from_module, sizeof(rdr_exc->e_from_module), "MDMAP") != EOK) {
        bsp_err("[mdmap]cp err\n");
    }
    if (strcpy_s((char *)rdr_exc->e_desc, sizeof(rdr_exc->e_from_module), "mdm req lpmcu log and no rst") != EOK) {
        bsp_err("[mdmap]cp err\n");
    }
}

void dump_mdmap_subsys_exc_register(void)
{
    struct rdr_exception_info_s rdr_exc = { 0 };
    dump_mdmap_fill_lpmcu_log_no_rst_exc_info(&rdr_exc);
    if (!rdr_register_exception(&rdr_exc)) {
        bsp_err("mdmap stub exc register failed!modid=0x%x\n", rdr_exc.e_modid);
    }
}

void mdmap_subsys_dump(u32 modid, u32 etype, u64 coreid, char *logpath, pfn_cb_dump_done fndone)
{
    UNUSED(etype);
    UNUSED(logpath);
    if (fndone != NULL) {
        fndone(modid, coreid);
        bsp_err("mdmap dump stub\n");
    }
}

void mdmap_subsys_reset(unsigned int modid, unsigned int etype, unsigned long long coreid)
{
    UNUSED(modid);
    UNUSED(etype);
    UNUSED(coreid);
    bsp_err("mdmap reset stub\n");
    return;
}

int dump_mdmap_register_subsys_eproc(void)
{
    struct rdr_module_ops_pub soc_ops = {
        .ops_dump = NULL,
        .ops_reset = NULL
    };
    struct rdr_register_module_result soc_rst = { 0, 0, 0 };

    soc_ops.ops_dump = (pfn_dump)mdmap_subsys_dump;
    soc_ops.ops_reset = (pfn_reset)mdmap_subsys_reset;

    if (rdr_register_module_ops(RDR_MODEMAP, &soc_ops, &soc_rst) != BSP_OK) {
        bsp_err("fail to register mdmap subsys rdr ops\n");
        return BSP_ERROR;
    }
    return BSP_OK;
}

int dump_mdmap_subsys_exc_stub_init(void)
{
    dump_mdmap_register_subsys_eproc();
    dump_mdmap_subsys_exc_register();
    dump_mdmap_subsys_exc_agent_init();
    return 0;
}

#if !IS_ENABLED(CONFIG_MODEM_DRIVER)
module_init(dump_mdmap_subsys_exc_stub_init);
#endif
