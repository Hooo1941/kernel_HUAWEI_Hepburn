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

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

int ip_qos_filter_ip_addr(const char *buf, int type)
{
    int ret;
    unsigned int ip[4] = {0}; /* 4, ip addr len */

    ret = pfa_input(buf, "%4d.%4d.%4d.%4d", &ip[0], &ip[1], &ip[2], &ip[3]); /* 2,3 ip addr shitf */
    if (ret < 0) {
        return ret;
    }

    if (type == PFA_DEST_IP) {
        // dst_addr
        PFA_ERR("dip:*,*,%d,%d\n", ip[2], ip[3]); /* 2,3 ip addr shitf */
        g_pfa_cmd.u.ip_qos.dip[0] = (ip[0] << 24) | (ip[1] << 16) | /* 16,24 ip addr shitf */
                                    (ip[2] << 8) | ip[3]; /* 2,3,8 ip addr shitf */
    } else if (type == PFA_SRC_IP) {
        // src_addr
        g_pfa_cmd.u.ip_qos.sip[0] = (ip[0] << 24) | (ip[1] << 16) | /* 16,24 ip addr shitf */
                                    (ip[2] << 8) | ip[3]; /* 2,3,8 ip addr shitf */
        PFA_ERR("sip:*, *, %d, %d \n", ip[2], ip[3]); /* 2,3 ip addr shitf */
    } else {
        PFA_ERR("type = %d", type);
        return PFA_FAIL;
    }

    return PFA_OK;
}

int ip_qos_sip(const char *buf)
{
    int ret;

    if (g_pfa_cmd.type == PFA_IPV4_QOS_TABLE) {
        ret = ip_qos_filter_ip_addr(buf, PFA_SRC_IP);
    } else if (g_pfa_cmd.type == PFA_IPV6_QOS_TABLE) {
        ret = filter_ipv6_addr(buf, PFA_SRC_IP, PFA_IP_QOS_TYPE);
    } else {
        PFA_ERR("g_pfa_cmd.type error = %d\n", g_pfa_cmd.type);
        ret = -1;
    }
    return ret;
}

int ip_qos_sip_mask(const char *buf)
{
    int ret;
    int val;

    ret = pfa_input(buf, "%8d", &val);
    if (ret < 0) {
        return ret;
    }
    g_pfa_cmd.u.ip_qos.sip_mask = (unsigned int)val;
    PFA_ERR("sip_mask = %d\n", val);

    return PFA_OK;
}

int ip_qos_dip(const char *buf)
{
    int ret;

    if (g_pfa_cmd.type == PFA_IPV4_QOS_TABLE) {
        ret = ip_qos_filter_ip_addr(buf, PFA_DEST_IP);
    } else if (g_pfa_cmd.type == PFA_IPV6_QOS_TABLE) {
        ret = filter_ipv6_addr(buf, PFA_DEST_IP, PFA_IP_QOS_TYPE);
    } else {
        PFA_ERR("g_pfa_cmd.type error = %d\n", g_pfa_cmd.type);
        ret = -1;
    }
    return ret;
}

int ip_qos_dip_mask(const char *buf)
{
    int ret;
    int val;

    ret = pfa_input(buf, "%8d", &val);
    if (ret < 0) {
        return ret;
    }
    g_pfa_cmd.u.ip_qos.dip_mask = (unsigned int)val;
    PFA_ERR("dip_mask = %d\n", val);

    return PFA_OK;
}

int ip_qos_filter_mask(const char *buf)
{
    int ret;
    int val;

    ret = pfa_input(buf, "%8x", &val);
    if (ret < 0) {
        return ret;
    }
    g_pfa_cmd.u.ip_qos.filter_mask = (unsigned int)val;
    PFA_ERR("filter_mask = %d\n", val);

    return PFA_OK;
}

int ip_qos_sport_min(const char *buf)
{
    int ret;
    int val;

    ret = pfa_input(buf, "%16d", &val);
    if (ret < 0) {
        return ret;
    }
    g_pfa_cmd.u.ip_qos.sport_min = (unsigned int)val;
    PFA_ERR("sport_min = %d\n", val);

    return PFA_OK;
}

int ip_qos_sport_max(const char *buf)
{
    int ret;
    int val;

    ret = pfa_input(buf, "%16d", &val);
    if (ret < 0) {
        return ret;
    }
    g_pfa_cmd.u.ip_qos.sport_max = (unsigned int)val;
    PFA_ERR("sport_max = %d\n", val);

    return PFA_OK;
}

int ip_qos_dport_min(const char *buf)
{
    int ret;
    int val;

    ret = pfa_input(buf, "%16d", &val);
    if (ret < 0) {
        return ret;
    }
    g_pfa_cmd.u.ip_qos.dport_min = (unsigned int)val;
    PFA_ERR("dport_min = %d\n", val);

    return PFA_OK;
}

int ip_qos_dport_max(const char *buf)
{
    int ret;
    int val;

    ret = pfa_input(buf, "%16d", &val);
    if (ret < 0) {
        return ret;
    }
    g_pfa_cmd.u.ip_qos.dport_max = (unsigned int)val;
    PFA_ERR("dport_max = %d\n", val);

    return PFA_OK;
}

int ip_qos_protocol_or_next_hdr(const char *buf)
{
    int ret;
    int val;

    ret = pfa_input(buf, "%8d", &val);
    if (ret < 0) {
        return ret;
    }
    g_pfa_cmd.u.ip_qos.protocol_or_next_hdr = (unsigned int)val;
    PFA_ERR("protocol_or_next_hdr = %d\n", val);

    return PFA_OK;
}

int ip_qos_flow_label(const char *buf)
{
    int ret;
    int val;

    ret = pfa_input(buf, "%20d", &val);
    if (ret < 0) {
        return ret;
    }
    g_pfa_cmd.u.ip_qos.flow_label = (unsigned int)val;
    PFA_ERR("flow_label = %d\n", val);

    return PFA_OK;
}

int ip_qos_discard_time(const char *buf)
{
    int ret;
    int val;

    ret = pfa_input(buf, "%8d", &val);
    if (ret < 0) {
        return ret;
    }
    g_pfa_cmd.u.ip_qos.discard_time = (unsigned int)val;
    PFA_ERR("discard_time = %d\n", val);

    return PFA_OK;
}

int ip_qos_priority(const char *buf)
{
    int ret;
    int val;

    ret = pfa_input(buf, "%3d", &val);
    if (ret < 0) {
        return ret;
    }
    g_pfa_cmd.u.ip_qos.priority = (unsigned int)val;
    PFA_ERR("priority = %d\n", val);

    return PFA_OK;
}

int ip_qos_dscp_min(const char *buf)
{
    int ret;
    int val;

    ret = pfa_input(buf, "%6d", &val);
    if (ret < 0) {
        return ret;
    }
    g_pfa_cmd.u.ip_qos.dscp_min = (unsigned int)val;
    PFA_ERR("dscp_min = %d\n", val);

    return PFA_OK;
}

int ip_qos_dscp_max(const char *buf)
{
    int ret;
    int val;

    ret = pfa_input(buf, "%6d", &val);
    if (ret < 0) {
        return ret;
    }
    g_pfa_cmd.u.ip_qos.dscp_max = (unsigned int)val;
    PFA_ERR("dscp_max = 0x%x\n", val);

    return PFA_OK;
}

int pfa_ip_qos_add(struct pfa_ip_qos_entry *ent)
{
    if (ent == NULL) {
        return -EINVAL;
    }
    return pfa_mac_entry_config(PFA_TAB_CTRL_ADD_IP_QOS, (unsigned int *)ent, sizeof(*ent) / sizeof(unsigned int));
}

int pfa_ip_qos_del(struct pfa_ip_qos_entry *ent)
{
    if (ent == NULL) {
        return -EINVAL;
    }
    return pfa_mac_entry_config(PFA_TAB_CTRL_DEL_IP_QOS, (unsigned int *)ent, sizeof(*ent) / sizeof(unsigned int));
}

int pfa_ip_qos_act(void *mac_cmd_handler)
{
    struct pfa_cmd_handler_s *cmd_handler = (struct pfa_cmd_handler_s *)mac_cmd_handler;
    struct pfa_ip_qos_entry ip_qos;
    int ret;
    int i;

    ret = memcpy_s(&ip_qos, sizeof(struct pfa_ip_qos_entry), &cmd_handler->u.ip_qos, sizeof(struct pfa_ip_qos_entry));
    if (ret) {
        PFA_ERR("pfa ip qos memcpy err.\n");
        return -1;
    }
    for (i = 0; i < 4; i++) {
        ip_qos.sip[i] = cpu_to_be32(ip_qos.sip[i]);
        ip_qos.dip[i] = cpu_to_be32(ip_qos.dip[i]);
    }
    ip_qos.dport_max = cpu_to_be16(cmd_handler->u.ip_qos.dport_max);
    ip_qos.dport_min = cpu_to_be16(cmd_handler->u.ip_qos.dport_min);
    ip_qos.sport_max = cpu_to_be16(cmd_handler->u.ip_qos.sport_max);
    ip_qos.sport_min = cpu_to_be16(cmd_handler->u.ip_qos.sport_min);

    if (cmd_handler->action == PFA_ENTRY_DELETE) {
        ret = pfa_ip_qos_del(&ip_qos);
        if (ret) {
            PFA_ERR("pfa_ip_qos_delete err.\n");
        }
    } else if (cmd_handler->action == PFA_ENTRY_ADD) {
        ret = pfa_ip_qos_add(&ip_qos);
        if (ret) {
            PFA_ERR("pfa_ip_qos_add err.\n");
        }
    }

    return ret;
}

void ip_qos_dump(unsigned int *pfa_tab, int i)
{
    struct pfa_ip_qos_entry_ram *ip_qos_entry = NULL;

    ip_qos_entry = (struct pfa_ip_qos_entry_ram *)pfa_tab;

    if (ip_qos_entry->valid == 0) {
        return;
    }
    PFA_ERR("sip_mask %d  dip_mask:%d  fillter_mask:0x%x  dscp_min:%d \n",
            ip_qos_entry->sip_mask, ip_qos_entry->dip_mask, ip_qos_entry->fillter_mask & 0xff,
            ip_qos_entry->dscp_min);
    PFA_ERR("dscp_max:%d  flow_label:%d iptype:%d  valid:%d\n",
            ip_qos_entry->dscp_max, ip_qos_entry->flow_label, ip_qos_entry->iptype, ip_qos_entry->valid);
    PFA_ERR("src_addr:*, *, 0x%x 0x%x\n",
            ip_qos_entry->sip[2], ip_qos_entry->sip[3]); /* 2 3 is ip idx */
    PFA_ERR("dst_addr:*, *, 0x%x 0x%x\n",
            ip_qos_entry->dip[2], ip_qos_entry->dip[3]); /* 2 3 is ip idx */
    PFA_ERR("sport_min:%d sport_max:%d\n", ip_qos_entry->sport_min, ip_qos_entry->sport_max);
    PFA_ERR("dport_min:%d  dport_max:%d\n", ip_qos_entry->dport_min, ip_qos_entry->dport_max);
    PFA_ERR("protocol_or_next_hdr:%d  priority:%d  discard_time:%d\n",
            ip_qos_entry->protocol_or_next_hdr, ip_qos_entry->priority, ip_qos_entry->discard_time);
}

void pfa_ip_qos_entry_dump(void)
{
    unsigned int pfa_tab[PFA_IP_QOS_TAB_WORD_NO];
    int i, j;
    struct pfa *pfa = &g_pfa;

    pfa->hal->dbgen_en();

    for (i = 0; i < PFA_IP_QOS_TAB_WORD_NO; i++) {
        for (j = 0; j < PFA_IP_QOS_TAB_WORD_NO; j++) {
            pfa_tab[j] = pfa_readl(pfa->regs, PFA_IP_QOS_TAB(i * PFA_IP_QOS_TAB_WORD_NO + j));
        }
        ip_qos_dump(pfa_tab, i);
    }
    pfa->hal->dbgen_dis();
}

int exe_pfa_ip_qos_cmd(void)
{
    int ret = PFA_OK;

    if (g_pfa_cmd.action == PFA_ENTRY_DUMP) {
        pfa_ip_qos_entry_dump();
    } else {
        if (g_pfa_cmd.type == PFA_IPV4_QOS_TABLE || g_pfa_cmd.type == PFA_IPV6_QOS_TABLE) {
            ret = pfa_ip_qos_act(&g_pfa_cmd);
            if (ret) {
                PFA_ERR("pfa ip qos act err.\n");
            }
        }
    }

    if (ret) {
        PFA_ERR("exe pfa ip qos cmd error!\n");
    }
    return ret;
}
MODULE_LICENSE("GPL");
EXPORT_SYMBOL(pfa_ip_qos_add);
EXPORT_SYMBOL(pfa_ip_qos_del);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
