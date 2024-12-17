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

#include <securec.h>
#include "pfa.h"
#include "pfa_dbg.h"
#include "pfa_mac_entry.h"
#include "pfa_hal_reg.h"
#include "pfa_sysfs.h"
#include <mdrv_pfa.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

int mac_pdu_index(const char *buf)
{
    int ret;
    int idx;

    ret = pfa_input(buf, "%5d", &idx);
    if (ret < 0) {
        return ret;
    }
    g_pfa_cmd.u.mac_pdu.index = (unsigned int)idx;
    PFA_ERR("idx = %d\n", idx);

    return PFA_OK;
}

int mac_pdu_rule_type(const char *buf)
{
    int ret;
    int val;

    ret = pfa_input(buf, "%1d", &val);
    if (ret < 0) {
        return ret;
    }
    g_pfa_cmd.u.mac_pdu.rule_type = (unsigned int)val;
    PFA_ERR("rule_type = %d\n", val);

    return PFA_OK;
}

int mac_pdu_mask(const char *buf)
{
    int ret;
    int val;

    ret = pfa_input(buf, "%6x", &val);
    if (ret < 0) {
        return ret;
    }
    g_pfa_cmd.u.mac_pdu.mask = (unsigned int)val;
    PFA_ERR("mask = %d\n", val);

    return PFA_OK;
}

int mac_pdu_modem_id(const char *buf)
{
    int ret;
    int val;

    ret = pfa_input(buf, "%2d", &val);
    if (ret < 0) {
        return ret;
    }
    g_pfa_cmd.u.mac_pdu.modem_id = (unsigned int)val;
    PFA_ERR("modem_id = %d\n", val);

    return PFA_OK;
}

int mac_pdu_session_status(const char *buf)
{
    int ret;
    int val;

    ret = pfa_input(buf, "%1d", &val);
    if (ret < 0) {
        return ret;
    }
    g_pfa_cmd.u.mac_pdu.pdu_session_status = (unsigned int)val;
    PFA_ERR("pdu_session_status = %d\n", val);

    return PFA_OK;
}

int mac_pdu_eth_type(const char *buf)
{
    int ret;
    int val;

    ret = pfa_input(buf, "%16x", &val);
    if (ret < 0) {
        return ret;
    }
    g_pfa_cmd.u.mac_pdu.eth_type = (unsigned int)val;
    PFA_ERR("eth_type = %d\n", val);

    return PFA_OK;
}

int mac_pdu_s_tag_vid_start(const char *buf)
{
    int ret;
    int val;

    ret = pfa_input(buf, "%12d", &val);
    if (ret < 0) {
        return ret;
    }
    g_pfa_cmd.u.mac_pdu.s_tag_vid_start = (unsigned int)val;
    PFA_ERR("s_tag_vid_start = %d\n", val);

    return PFA_OK;
}

int mac_pdu_s_tag_pcp_dei(const char *buf)
{
    int ret;
    int val;

    ret = pfa_input(buf, "%4d", &val);
    if (ret < 0) {
        return ret;
    }
    g_pfa_cmd.u.mac_pdu.s_tag_pcp_dei = (unsigned int)val;
    PFA_ERR("s_tag_pcp_dei = %d\n", val);

    return PFA_OK;
}

int mac_pdu_s_tag_vid_end(const char *buf)
{
    int ret;
    int val;

    ret = pfa_input(buf, "%12d", &val);
    if (ret < 0) {
        return ret;
    }
    g_pfa_cmd.u.mac_pdu.s_tag_vid_end = (unsigned int)val;
    PFA_ERR("s_tag_vid_end = %d\n", val);

    return PFA_OK;
}

int mac_pdu_s_tag_pcp_dei_mask(const char *buf)
{
    int ret;
    int val;

    ret = pfa_input(buf, "%4x", &val);
    if (ret < 0) {
        return ret;
    }
    g_pfa_cmd.u.mac_pdu.s_tag_pcp_dei_mask = (unsigned int)val;
    PFA_ERR("s_tag_pcp_dei_mask = %d\n", val);

    return PFA_OK;
}

int mac_pdu_c_tag_vid_start(const char *buf)
{
    int ret;
    int val;

    ret = pfa_input(buf, "%12d", &val);
    if (ret < 0) {
        return ret;
    }
    g_pfa_cmd.u.mac_pdu.c_tag_vid_start = (unsigned int)val;
    PFA_ERR("c_tag_vid_start = %d\n", val);

    return PFA_OK;
}

int mac_pdu_c_tag_pcp_dei(const char *buf)
{
    int ret;
    int val;

    ret = pfa_input(buf, "%4d", &val);
    if (ret < 0) {
        return ret;
    }
    g_pfa_cmd.u.mac_pdu.c_tag_pcp_dei = (unsigned int)val;
    PFA_ERR("c_tag_pcp_dei = %d\n", val);

    return PFA_OK;
}

int mac_pdu_c_tag_vid_end(const char *buf)
{
    int ret;
    int val;

    ret = pfa_input(buf, "%12d", &val);
    if (ret < 0) {
        return ret;
    }
    g_pfa_cmd.u.mac_pdu.c_tag_vid_end = (unsigned int)val;
    PFA_ERR("c_tag_vid_end = %d\n", val);

    return PFA_OK;
}

int mac_pdu_c_tag_pcp_dei_mask(const char *buf)
{
    int ret;
    int val;

    ret = pfa_input(buf, "%4x", &val);
    if (ret < 0) {
        return ret;
    }
    g_pfa_cmd.u.mac_pdu.c_tag_pcp_dei_mask = (unsigned int)val;
    PFA_ERR("c_tag_pcp_dei_mask = 0x%x\n", val);

    return PFA_OK;
}

int mac_pdu_dmac(const char *buf)
{
    int ret;
    int mac1[ETH_ALEN];
    char mac[ETH_ALEN];

    ret = pfa_input(buf, "%2x:%2x:%2x:%2x:%2x:%2x", &mac1[0], &mac1[1], &mac1[2], /* 2, mac addr shitf */
        &mac1[3], &mac1[4], &mac1[5]); /* 3,4,5 mac addr shitf */
    if (ret < 0) {
        return ret;
    }

    mac[0] = mac1[0];
    mac[1] = mac1[1];
    mac[2] = mac1[2]; /* 2, mac addr shitf */
    mac[3] = mac1[3]; /* 3, mac addr shitf */
    mac[4] = mac1[4]; /* 4, mac addr shitf */
    mac[5] = mac1[5]; /* 5, mac addr shitf */
    g_pfa_cmd.u.mac_pdu.dmac_hi = mac[3] << 24 | mac[2] << 16 | mac[1] << 8 | mac[0]; /* 2,3, mac addr shitf, 8,16,24 left */
    g_pfa_cmd.u.mac_pdu.dmac_lo = mac[5] << 8 | mac[4]; /* 4,5, mac addr shitf, 8, mac addr shitf */

    return PFA_OK;
}

int mac_pdu_session_id(const char *buf)
{
    int ret;
    int val;

    ret = pfa_input(buf, "%8d", &val);
    if (ret < 0) {
        return ret;
    }
    g_pfa_cmd.u.mac_pdu.pdu_session_id = (unsigned int)val;
    PFA_ERR("pdu_session_id = %d\n", val);

    return PFA_OK;
}

int mac_pdu_session_vld(const char *buf)
{
    int ret;
    int val;

    ret = pfa_input(buf, "%1d", &val);
    if (ret < 0) {
        return ret;
    }
    /* only print */
    PFA_ERR("pdu_session_vld = %d\n", val);

    return PFA_OK;
}

int mac_pdu_fc_head(const char *buf)
{
    g_pfa_cmd.u.mac_pdu.fc_head = PFA_TFT_ETH_ULFC;
    PFA_ERR("fc_head = %d\n", g_pfa_cmd.u.mac_pdu.fc_head);

    return PFA_OK;
}

int pfa_mac_pdu_add(struct pfa_mac_pdu_entry *ent)
{
    if (ent == NULL) {
        return -EINVAL;
    }
    return pfa_mac_entry_config(PFA_TAB_CTRL_ADD_MAC_PDU, (unsigned int *)ent, sizeof(*ent) / sizeof(unsigned int));
}

int pfa_mac_pdu_del(struct pfa_mac_pdu_entry *ent)
{
    if (ent == NULL) {
        return -EINVAL;
    }
    return pfa_mac_entry_config(PFA_TAB_CTRL_DEL_MAC_PDU, (unsigned int *)ent, sizeof(*ent) / sizeof(unsigned int));
}

int pfa_mac_pdu_act(void *mac_cmd_handler)
{
    struct pfa_cmd_handler_s *cmd_handler = (struct pfa_cmd_handler_s *)mac_cmd_handler;
    struct pfa_mac_pdu_entry mac_pdu = {0};
    int ret;

    ret = memcpy_s(&mac_pdu, sizeof(struct pfa_mac_pdu_entry), &cmd_handler->u.mac_pdu, sizeof(struct pfa_mac_pdu_entry));
    if (ret) {
        PFA_ERR("pfa mac pdu memcpy err.\n");
        return -1;
    }

    if (cmd_handler->action == PFA_ENTRY_DELETE) {
        ret = pfa_mac_pdu_del(&mac_pdu);
        if (ret) {
            PFA_ERR("pfa_mac_pdu_del err.\n");
        }
    } else if (cmd_handler->action == PFA_ENTRY_ADD) {
        ret = pfa_mac_pdu_add(&mac_pdu);
        if (ret) {
            PFA_ERR("pfa_mac_pdu_add err.\n");
        }
    }

    return ret;
}

void mac_pdu_dump(unsigned int *pfa_tab, int i)
{
    struct pfa_mac_pdu_entry_ram *mac_pdu_entry = NULL;

    mac_pdu_entry = (struct pfa_mac_pdu_entry_ram *)pfa_tab;

    if (mac_pdu_entry->pdu_session_vld == 0) {
        return;
    }

    PFA_ERR("index (%d) rule_type:%d  mask:%d  modem_id:%d  pdu_session_status:%d  eth_type:0x%x\n",
            mac_pdu_entry->index, mac_pdu_entry->rule_type, mac_pdu_entry->mask & 0xff,
            mac_pdu_entry->modem_id, mac_pdu_entry->pdu_session_status, mac_pdu_entry->eth_type & 0xffff);
    PFA_ERR("s_tag_vid_start:%d  s_tag_pcp_dei:%d  s_tag_vid_end:%d  s_tag_pcp_dei_mask:0x%x\n",
            mac_pdu_entry->s_tag_vid_start, mac_pdu_entry->s_tag_pcp_dei, mac_pdu_entry->s_tag_vid_end,
            mac_pdu_entry->s_tag_pcp_dei_mask);
    PFA_ERR("c_tag_vid_start:%d  c_tag_pcp_dei:%d  c_tag_vid_end:%d  c_tag_pcp_dei_mask:0x%x\n",
            mac_pdu_entry->c_tag_vid_start, mac_pdu_entry->c_tag_pcp_dei, mac_pdu_entry->c_tag_vid_end,
            mac_pdu_entry->c_tag_pcp_dei_mask);
    PFA_ERR("dmac_hi:0x%x  dmac_lo:0x%x  pdu_session_id:%d  pdu_session_vld:%d fc_head:%d \n",
            mac_pdu_entry->dmac_hi, mac_pdu_entry->dmac_lo & 0xffff,
            mac_pdu_entry->pdu_session_id, mac_pdu_entry->pdu_session_vld, mac_pdu_entry->fc_head);
}

void pfa_mac_pdu_entry_dump(void)
{
    unsigned int pfa_tab[PFA_MAC_PDU_TAB_WORD_NO];
    int i, j;
    struct pfa *pfa = &g_pfa;

    pfa->hal->dbgen_en();

    for (i = 0; i < PFA_MAC_PDU_TAB_NUMBER; i++) {
        for (j = 0; j < PFA_MAC_PDU_TAB_WORD_NO; j++) {
            pfa_tab[j] = pfa_readl(pfa->regs, PFA_MAC_PDU_TAB(i * PFA_MAC_PDU_TAB_WORD_NO + j));
        }
        mac_pdu_dump(pfa_tab, i);
    }
    pfa->hal->dbgen_dis();
}

int exe_pfa_mac_pdu_cmd(void)
{
    int ret = PFA_OK;

    if (g_pfa_cmd.action == PFA_ENTRY_DUMP) {
        pfa_mac_pdu_entry_dump();
    } else {
        if (g_pfa_cmd.type == PFA_MAC_PDU_TABLE) {
            ret = pfa_mac_pdu_act(&g_pfa_cmd);
            if (ret) {
                PFA_ERR("pfa mac pdu act err.\n");
            }
        }
    }

    if (ret) {
        PFA_ERR("exe pfa mac pdu cmd error!\n");
    }
    return ret;
}
MODULE_LICENSE("GPL");
EXPORT_SYMBOL(pfa_mac_pdu_add);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
