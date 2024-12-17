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

#include <linux/init.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/kobject.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/ctype.h>

#include <securec.h>
#include "pfa.h"
#include "pfa_core.h"
#include "pfa_dbg.h"
#include "pfa_ip_entry.h"
#include "pfa_mac_entry.h"
#include "pfa_port.h"
#include "pfa_hal_reg.h"
#include "pfa_hal_desc.h"

#include "pfa_sysfs.h"

#define PFA_STR_LEN 20

struct pfa_cmd_handler_s g_pfa_cmd;

struct pfa_filter_param g_mac_filter_array[] = {
    { "mac", filter_mac_addr },
    { "action", filter_action },
    { "reaction", filter_mac_reaction },
    { "redirect", filter_mac_redirect },
};

struct pfa_filter_param g_mac_forward_array[] = {
    { "action", forward_action },
};

struct pfa_filter_param g_mac_pdu_array[] = {
    { "action", filter_action },
    { "index", mac_pdu_index },
    { "rule_type", mac_pdu_rule_type },
    { "mask", mac_pdu_mask },
    { "modem_id", mac_pdu_modem_id },
    { "pdu_session_status", mac_pdu_session_status },
    { "eth_type", mac_pdu_eth_type },

    { "s_tag_vid_start", mac_pdu_s_tag_vid_start },
    { "s_tag_pcp_dei", mac_pdu_s_tag_pcp_dei },
    { "s_tag_vid_end", mac_pdu_s_tag_vid_end },
    { "s_tag_pcp_dei_mask", mac_pdu_s_tag_pcp_dei_mask },
    { "c_tag_vid_start", mac_pdu_c_tag_vid_start },
    { "c_tag_pcp_dei", mac_pdu_c_tag_pcp_dei },
    { "c_tag_vid_end", mac_pdu_c_tag_vid_end },
    { "c_tag_pcp_dei_mask", mac_pdu_c_tag_pcp_dei_mask },
    { "DMAC", mac_pdu_dmac },
    { "pdu_session_id", mac_pdu_session_id },
    { "pdu_session_vld", mac_pdu_session_vld },
    { "fc_head", mac_pdu_fc_head },
};

struct pfa_filter_param g_ip_filter_array[] = {
    { "action", filter_action },
    { "filter_mask", filter_ip_mask },
    { "src_port_low", filter_src_port_low },
    { "src_port_high", filter_src_port_high },
    { "dst_port_low", filter_dest_port_low },
    { "dst_port_high", filter_dest_port_high },

    { "src_addr", filter_ip_src_addr },
    { "src_addr_mask", filter_ip_src_addr_mask },
    { "dst_addr", filter_ip_dest_addr },
    { "dst_addr_mask", filter_ip_dest_addr_mask },
    { "protocol", filter_ipv4_protocol },
    { "tos", filter_tos },
};

struct pfa_filter_param g_ipv6_filter_array[] = {
    { "action", filter_action },
    { "filter_mask", filter_ip_mask },
    { "src_port_low", filter_src_port_low },
    { "src_port_high", filter_src_port_high },
    { "dst_port_low", filter_dest_port_low },
    { "dst_port_high", filter_dest_port_high },

    { "src_addr", filter_ipv6_src_addr },
    { "src_addr_mask", filter_ipv6_src_addr_mask },
    { "dst_addr", filter_ipv6_dest_addr },
    { "dst_addr_mask", filter_ipv6_dest_addr_mask },
    { "protocol", filter_ipv6_protocol },
    { "tc", filter_ipv6_tc },
    { "next_hdr", filter_ipv6_next_hdr },
    { "flow_label", filter_ipv6_flow_label },
};

struct pfa_filter_param g_ip_qos_array[] = {
    { "action", filter_action },
    { "filter_mask", ip_qos_filter_mask },
    { "src_port_low", ip_qos_sport_min },
    { "src_port_high", ip_qos_sport_max },
    { "dst_port_low", ip_qos_dport_min },
    { "dst_port_high", ip_qos_dport_max },
    { "src_addr", ip_qos_sip },
    { "src_addr_mask", ip_qos_sip_mask },
    { "dst_addr", ip_qos_dip },
    { "dst_addr_mask", ip_qos_dip_mask },
    { "protocol", ip_qos_protocol_or_next_hdr },
    { "Priority", ip_qos_priority },
    { "discard_time", ip_qos_discard_time },
    { "next_hdr", ip_qos_protocol_or_next_hdr },
    { "flow_label", ip_qos_flow_label },
    { "dscp_max", ip_qos_dscp_max },
    { "dscp_min", ip_qos_dscp_min },
};

struct pfa_filter_param g_pfa_black_white[] = {
    { "mode", pfa_black_white_mode_set },
    { "black_white", pfa_black_white_set },
    { "action", set_action },
};

struct pfa_filter_param g_pfa_set_eth_minlen_array[] = {
    { "value", pfa_eth_min_len_value_set },
    { "action", set_action },
};

struct pfa_filter_param g_udp_lmtnum_array[] = {
    { "portno", pfa_udp_lim_portno_set },
    { "udp_rate", udp_rate_set },
    { "action", set_action },
};

struct pfa_filter_param g_lmt_byte_array[] = {
    { "portno", pfa_udp_lim_portno_set },
    { "byte_rate", port_rate_set },
    { "action", set_action },
};

struct pfa_filter_param g_lmt_time_array[] = {
    { "time", pfa_port_lim_time_set },
    { "action", set_action },
};

struct pfa_filter_param g_pfa_dynamic_switch[] = {
    { "switch", pfa_switch },
    { "action", set_action },
};

struct pfa_filter_param g_pfa_l4_portnum_array[] = {
    { "protocol", pfa_l4_protocol_get },
    { "portnum", pfa_l4_portnum_get },
    { "action", filter_action },
};

#ifdef PFA_DEBUG
int g_last_filter_type = 0;
#endif

// function declaration
int pfa_input(const char *buf, const char *fmt, ...)
{
    int ret;
    va_list arguments;

    va_start(arguments, fmt);
    ret = vsscanf_s(buf, fmt, arguments);
    va_end(arguments);

    if (ret <= 0) {
        PFA_ERR("vsscanf_s err ret:%d.\n", ret);
        return -1;
    }

    return ret;
}

int pfa_mac_filt_act(void *mac_cmd_handler)
{
    struct pfa_cmd_handler_s *cmd_handler = (struct pfa_cmd_handler_s *)mac_cmd_handler;
    struct pfa_mac_filt_entry mac_filter = {0};
    int ret;

    ret = memcpy_s(&mac_filter.mac_hi, sizeof(mac_filter.mac_hi), cmd_handler->u.mac_content.mac, sizeof(unsigned int));
    if (unlikely(ret)) {
        PFA_ERR("memcpy_s fail.\n");
    }
    ret = memcpy_s(&mac_filter.mac_lo, sizeof(mac_filter.mac_lo), cmd_handler->u.mac_content.mac + sizeof(unsigned int),
                   sizeof(unsigned short));
    if (unlikely(ret)) {
        PFA_ERR("memcpy_s fail.\n");
    }
    mac_filter.redir_act = cmd_handler->u.mac_content.action;
    mac_filter.redir_port = cmd_handler->u.mac_content.redirect;

    if (cmd_handler->action == PFA_ENTRY_DELETE) {
        ret = pfa_mac_filt_del(&mac_filter);
        if (ret) {
            PFA_ERR("pfa_mac_filt_del err.\n");
        }
    } else if (cmd_handler->action == PFA_ENTRY_ADD) {
        ret = pfa_mac_filt_add(&mac_filter);
        if (ret) {
            PFA_ERR("pfa_mac_filt_add err.\n");
        }
    }

    return ret;
}

int pfa_ipv4_filt_act(void *ipv4_cmd_handler)
{
    struct pfa_cmd_handler_s *cmd_handler = (struct pfa_cmd_handler_s *)ipv4_cmd_handler;
    struct pfa_ipv4_filt_entry ipv4_filter;
    int ret;

    if (cmd_handler->u.ip_content.dst_port_low > cmd_handler->u.ip_content.dst_port_high) {
        PFA_ERR("invaild ip port number. \n");
        return PFA_FAIL;
    }

    if (cmd_handler->u.ip_content.src_port_low > cmd_handler->u.ip_content.src_port_high) {
        PFA_ERR("invaild ip port number. \n");
        return PFA_FAIL;
    }

    ret = memset_s(&ipv4_filter, sizeof(ipv4_filter), 0, sizeof(struct pfa_ipv4_filt_entry));
    if (ret) {
        PFA_ERR("memset_s err.\n");
    }

    ipv4_filter.sip = cmd_handler->u.ip_content.u.pfa_ipv4.src_addr;
    ipv4_filter.sip = cpu_to_be32(ipv4_filter.sip);
    ipv4_filter.sip_mask = cmd_handler->u.ip_content.u.pfa_ipv4.src_addr_mask;
    ipv4_filter.dip = cmd_handler->u.ip_content.u.pfa_ipv4.dst_addr;
    ipv4_filter.dip = cpu_to_be32(ipv4_filter.dip);
    ipv4_filter.dip_mask = cmd_handler->u.ip_content.u.pfa_ipv4.dst_addr_mask;
    ipv4_filter.tos = cmd_handler->u.ip_content.u.pfa_ipv4.tos;
    ipv4_filter.protocol = cmd_handler->u.ip_content.u.pfa_ipv4.protocol;

    ipv4_filter.filt_mask = cmd_handler->u.ip_content.filter_mask;
    ipv4_filter.dport_max = cpu_to_be16(cmd_handler->u.ip_content.dst_port_high);
    ipv4_filter.dport_min = cpu_to_be16(cmd_handler->u.ip_content.dst_port_low);
    ipv4_filter.sport_max = cpu_to_be16(cmd_handler->u.ip_content.src_port_high);
    ipv4_filter.sport_min = cpu_to_be16(cmd_handler->u.ip_content.src_port_low);

    if (cmd_handler->action == PFA_ENTRY_DELETE) {
        ret = pfa_mac_entry_config(PFA_TAB_CTRL_DEL_IPV4_FILT, (unsigned int *)(&ipv4_filter),
            sizeof(struct pfa_ipv4_filt_entry) / sizeof(unsigned int));
        if (ret) {
            PFA_ERR("del ipv4 filter enty fail,please check entry exit.");
        } else {
            PFA_ERR("del ipv4 filter enty sucess.");
        }
    } else if (cmd_handler->action == PFA_ENTRY_ADD) {
        ret = pfa_mac_entry_config(PFA_TAB_CTRL_ADD_IPV4_FILT, (unsigned int *)(&ipv4_filter),
            sizeof(struct pfa_ipv4_filt_entry) / sizeof(unsigned int));
        if (ret) {
            PFA_ERR("add ipv4 filter enty fail.");
        } else {
            PFA_ERR("add ipv4 filter enty sucess.");
        }
    }

    return ret;
}

int pfa_ipv6_filt_act(void *ipv6_cmd_handler)
{
    struct pfa_cmd_handler_s *cmd_handler = (struct pfa_cmd_handler_s *)ipv6_cmd_handler;
    struct pfa_ipv6_filt_entry ipv6_filter;
    int ret;

    if (cmd_handler->u.ip_content.dst_port_low > cmd_handler->u.ip_content.dst_port_high) {
        PFA_ERR("invaild ip port number. \n");
        return PFA_FAIL;
    }

    if (cmd_handler->u.ip_content.src_port_low > cmd_handler->u.ip_content.src_port_high) {
        PFA_ERR("invaild ip port number. \n");
        return PFA_FAIL;
    }

    ret = memset_s(&ipv6_filter, sizeof(ipv6_filter), 0, sizeof(struct pfa_ipv6_filt_entry));
    if (ret) {
        PFA_ERR("memset_s err.\n");
    }
    ipv6_filter.sip[0] = cpu_to_be32(cmd_handler->u.ip_content.u.pfa_ipv6.src_addr[0]);
    ipv6_filter.sip[1] = cpu_to_be32(cmd_handler->u.ip_content.u.pfa_ipv6.src_addr[1]);
    ipv6_filter.sip[2] = cpu_to_be32(cmd_handler->u.ip_content.u.pfa_ipv6.src_addr[2]); /* 2, ip addr shift */
    ipv6_filter.sip[3] = cpu_to_be32(cmd_handler->u.ip_content.u.pfa_ipv6.src_addr[3]); /* 3, ip addr shift */
    ipv6_filter.sip_mask = cmd_handler->u.ip_content.u.pfa_ipv6.src_addr_mask;

    ipv6_filter.dip[0] = cpu_to_be32(cmd_handler->u.ip_content.u.pfa_ipv6.dst_addr[0]);
    ipv6_filter.dip[1] = cpu_to_be32(cmd_handler->u.ip_content.u.pfa_ipv6.dst_addr[1]);
    ipv6_filter.dip[2] = cpu_to_be32(cmd_handler->u.ip_content.u.pfa_ipv6.dst_addr[2]); /* 2, ip addr shift */
    ipv6_filter.dip[3] = cpu_to_be32(cmd_handler->u.ip_content.u.pfa_ipv6.dst_addr[3]); /* 3, ip addr shift */
    ipv6_filter.dip_mask = cmd_handler->u.ip_content.u.pfa_ipv6.dst_addr_mask;

    ipv6_filter.tc = cmd_handler->u.ip_content.u.pfa_ipv6.tc;
    ipv6_filter.next_hdr = cmd_handler->u.ip_content.u.pfa_ipv6.next_hdr;
    ipv6_filter.filt_mask = cmd_handler->u.ip_content.filter_mask;
    ipv6_filter.dport_max = cpu_to_be16(cmd_handler->u.ip_content.dst_port_high);
    ipv6_filter.dport_min = cpu_to_be16(cmd_handler->u.ip_content.dst_port_low);
    ipv6_filter.sport_max = cpu_to_be16(cmd_handler->u.ip_content.src_port_high);
    ipv6_filter.sport_min = cpu_to_be16(cmd_handler->u.ip_content.src_port_low);

    if (cmd_handler->action == PFA_ENTRY_DELETE) {
        ipv6_filter.flow_label = cmd_handler->u.ip_content.u.pfa_ipv6.flow_label;
        ret = pfa_mac_entry_config(PFA_TAB_CTRL_DEL_IPV6_FILT, (unsigned int *)(&ipv6_filter),
            sizeof(struct pfa_ipv6_filt_entry) / sizeof(unsigned int));
    } else if (cmd_handler->action == PFA_ENTRY_ADD) {
        ret = pfa_mac_entry_config(PFA_TAB_CTRL_ADD_IPV6_FILT, (unsigned int *)(&ipv6_filter),
            sizeof(struct pfa_ipv6_filt_entry) / sizeof(unsigned int));
    }

    return ret;
}

void ipv4_filt_dump(unsigned int *pfa_tab, int i)
{
    struct pfa_ipv4_filter_entry_in_sram *ipv4_filter_entry = NULL;

    if (pfa_tab[1] & IS_IP_FL_ENTRY) {
        ipv4_filter_entry = (struct pfa_ipv4_filter_entry_in_sram *)pfa_tab;
    } else {
        return;
    }
    PFA_ERR("[ipv4 entry (%d)] \nsip_mask:%d  dip_mask:%d  protocol:%d  tos:%d  filter_mask:0x%x  "
            " sport_min:%d  sport_max:%d  dport_min:%d  dport_max:%d\n",
            i, ipv4_filter_entry->sip_mask & 0xff, ipv4_filter_entry->dip_mask & 0xff,
            ipv4_filter_entry->protocol & 0xff, ipv4_filter_entry->tos & 0xff, ipv4_filter_entry->filter_mask & 0xff,
            be16_to_cpu(ipv4_filter_entry->sport_min & 0xffff), be16_to_cpu(ipv4_filter_entry->sport_max & 0xffff),
            be16_to_cpu(ipv4_filter_entry->dport_min & 0xffff), be16_to_cpu(ipv4_filter_entry->dport_max & 0xffff));
}

void ipv6_filt_dump(unsigned int *pfa_tab, int i)
{
    struct pfa_ipv6_filter_entry_in_sram *ipv6_filter_entry = NULL;

    if (pfa_tab[1] & IS_IP_FL_ENTRY) {
        ipv6_filter_entry = (struct pfa_ipv6_filter_entry_in_sram *)pfa_tab;
    } else {
        return;
    }
    PFA_ERR("[ipv6 entry (%d)] \nsip_mask:%d  dip_mask:%d  next_hdr:0x%x  tc:%d  filter_mask:0x%x  "
            "flow_label:0x%x sport_min:%d  sport_max:%d  dport_min:%d  dport_max:%d\n",
            i, ipv6_filter_entry->sip_mask & 0xff, ipv6_filter_entry->dip_mask & 0xff,
            ipv6_filter_entry->next_hdr & 0xff, ipv6_filter_entry->tc & 0xff, ipv6_filter_entry->filter_mask & 0xff,
            ipv6_filter_entry->flow_label & 0xff, be16_to_cpu(ipv6_filter_entry->sport_min & 0xffff),
            be16_to_cpu(ipv6_filter_entry->sport_max & 0xffff), be16_to_cpu(ipv6_filter_entry->dport_min & 0xffff),
            be16_to_cpu(ipv6_filter_entry->dport_max & 0xffff));
}

static  inline void pfa_l4_portnum_dump(struct pfa *pfa)
{
    unsigned int i;
    unsigned short portnum = 0;
    pfa_l4_cfg_t l4_portnum_cfg;

    for (i = 0; i < MAX_L4_PORTNUM_CFG_NUM; i++) {
        l4_portnum_cfg.u32 = pfa_readl(pfa->regs, PFA_L4_PORTNUM_CFG(i));
        if (l4_portnum_cfg.u32) {
            if (l4_portnum_cfg.bits.pfa_l4_portnum_cfg & BIT(16)) { /* bit 16 */
                portnum = be16_to_cpu((unsigned short)(l4_portnum_cfg.bits.pfa_l4_portnum_cfg & 0xFFFF));
                PFA_ERR("TCP l4_portnum : %d \n", portnum);
            } else if (l4_portnum_cfg.bits.pfa_l4_portnum_cfg & BIT(17)) { /* bit 17 */
                portnum = be16_to_cpu((unsigned short)(l4_portnum_cfg.bits.pfa_l4_portnum_cfg & 0xFFFF));
                PFA_ERR("UDP l4_portnum : %d \n", portnum);
            } else {
                l4_portnum_cfg.bits.pfa_l4_portnum_cfg = 0;
                pfa_writel(pfa->regs, PFA_L4_PORTNUM_CFG(i), l4_portnum_cfg.u32);
            }
        }
    }
    return;
}

static inline void pfa_l4_portnum_del(struct pfa *pfa, int cfg)
{
    unsigned int i;
    pfa_l4_cfg_t l4_portnum_cfg;

    for (i = 0; i < MAX_L4_PORTNUM_CFG_NUM; i++) {
        l4_portnum_cfg.u32 = pfa_readl(pfa->regs, PFA_L4_PORTNUM_CFG(i));
        if (l4_portnum_cfg.bits.pfa_l4_portnum_cfg == cfg) {
            l4_portnum_cfg.bits.pfa_l4_portnum_cfg = 0;
            pfa_writel(pfa->regs, PFA_L4_PORTNUM_CFG(i), l4_portnum_cfg.u32);
            break;
        }
    }
    return;
}

static inline void pfa_l4_portnum_add(struct pfa *pfa, int cfg)
{
    unsigned int i;
    pfa_l4_cfg_t l4_portnum_cfg;

    for (i = 0; i < MAX_L4_PORTNUM_CFG_NUM; i++) {
        l4_portnum_cfg.u32 = pfa_readl(pfa->regs, PFA_L4_PORTNUM_CFG(i));
        if (!l4_portnum_cfg.u32) {
            l4_portnum_cfg.bits.pfa_l4_portnum_cfg = cfg;
            pfa_writel(pfa->regs, PFA_L4_PORTNUM_CFG(i), l4_portnum_cfg.u32);
            break;
        }
    }
    return;
}

int pfa_l4_portnum_cfg(int cfg, unsigned int add_del_dump)
{
    struct pfa *pfa = &g_pfa;

    switch (add_del_dump) {
        case ADD: /* add */
            pfa_l4_portnum_add(pfa, cfg);
            break;
        case DEL: /* del */
            pfa_l4_portnum_del(pfa, cfg);
            break;
        case DUMP: /* dump */
            pfa_l4_portnum_dump(pfa);
            break;
        default:
            return -ENOMEM;
    }

    return 0;
}

int pfa_port_l4portno_to_cpu_del(unsigned short portnum, int l4_type)
{
    unsigned int cfg = 0;

    switch (l4_type) {
        case IPPROTO_UDP:
            cfg = cpu_to_be16(portnum) | BIT(17); /* bit 17 */
            break;
        case IPPROTO_TCP:
            cfg = cpu_to_be16(portnum) | BIT(16); /* bit 16 */
            break;
        default:
            PFA_ERR("L4 type %d is not supported! \n", l4_type);
            return -EPERM;
    }

    if (!pfa_l4_portnum_cfg(cfg, DEL)) {
        return 0;
    }

    return -ENOMEM;
}

int pfa_port_l4portno_to_cpu_add(unsigned short portnum, int l4_type)
{
    unsigned int cfg = 0;
    int ret;

    switch (l4_type) {
        case IPPROTO_UDP:
            cfg = cpu_to_be16(portnum) | BIT(17); /* bit 17 */
            break;
        case IPPROTO_TCP:
            cfg = cpu_to_be16(portnum) | BIT(16); /* bit 16 */
            break;
        default:
            PFA_ERR("L4 type %d is not supported! \n", l4_type);
            return -EPERM;
    }

    ret = pfa_port_l4portno_to_cpu_del(portnum, l4_type);
    if (ret) {
        PFA_ERR("pfa_port_l4portno_to_cpu_del fail. \n");
    }

    if (!pfa_l4_portnum_cfg(cfg, ADD)) {
        return 0;
    }

    PFA_ERR("l4_portnum_cfg already full! \n");
    return -ENOMEM;
}

void pfa_port_l4portno_to_cpu_dump(void)
{
    int ret;

    ret = pfa_l4_portnum_cfg(0, DUMP);
    if (ret) {
        PFA_ERR("pfa_l4_portnum_cfg fail. \n");
    }
    return;
}

/*
 * function name:  pfa_mac_validate
 * description: do some basic mac check
 * mac format  01:02:03:04:05:06
 */
int pfa_mac_validate(void)
{
    return PFA_OK;
}

struct pfa_cmd_type_s {
    char *str;
    int type;
    int num_funcs;
    struct pfa_filter_param *filter_parser_ops;
    char *logs;
};

struct pfa_cmd_type_s g_pfa_cmd_type[] = {
    {
        .str = "ipv4",
        .type = PFA_IP_FILTER_TABLE,
        .filter_parser_ops = g_ip_filter_array,
        .num_funcs = ARRAY_SIZE(g_ip_filter_array),
        .logs = "ipv4 filter:"
    },
    {
        .str = "ipv6",
        .type = PFA_IPV6_FILTER_TABLE,
        .filter_parser_ops = g_ipv6_filter_array,
        .num_funcs = ARRAY_SIZE(g_ipv6_filter_array),
        .logs = "ipv6 filter:"
    },
    {
        .str = "mac_filter",
        .type = PFA_MAC_FILTER_TABLE,
        .filter_parser_ops = g_mac_filter_array,
        .num_funcs = ARRAY_SIZE(g_mac_filter_array),
        .logs = "mac filter:"
    },
    {
        .str = "mac_forward",
        .type = PFA_MAC_FORWARD_TABLE,
        .filter_parser_ops = g_mac_forward_array,
        .num_funcs = ARRAY_SIZE(g_mac_forward_array),
        .logs = "mac forward:"
    },
    {
        .str = "filter_type",
        .type = PFA_BLACK_WHITE_LIST,
        .filter_parser_ops = g_pfa_black_white,
        .num_funcs = ARRAY_SIZE(g_pfa_black_white),
        .logs = "filter type:"
    },
    {
        .str = "udp_limit",
        .type = PFA_UDP_LMTNUM_TABLE,
        .filter_parser_ops = g_udp_lmtnum_array,
        .num_funcs = ARRAY_SIZE(g_udp_lmtnum_array),
        .logs = "udp limit:"
    },
    {
        .str = "byte_limit",
        .type = PFA_LMTBYTE_TABLE,
        .filter_parser_ops = g_lmt_byte_array,
        .num_funcs = ARRAY_SIZE(g_lmt_byte_array),
        .logs = "byte limit:"
    },
    {
        .str = "limit_time",
        .type = PFA_LMT_TIME_TABLE,
        .filter_parser_ops = g_lmt_time_array,
        .num_funcs = ARRAY_SIZE(g_lmt_time_array),
        .logs = "limit time:"
    },
    {
        .str = "min_len",
        .type = PFA_ETH_MIN_LEN,
        .filter_parser_ops = g_pfa_set_eth_minlen_array,
        .num_funcs = ARRAY_SIZE(g_pfa_set_eth_minlen_array),
        .logs = "min len:"
    },
    {
        .str = "switch",
        .type = PFA_DYNAMIC_SWITCH,
        .filter_parser_ops = g_pfa_dynamic_switch,
        .num_funcs = ARRAY_SIZE(g_pfa_dynamic_switch),
        .logs = "dynamic switch mode:"
    },
    {
        .str = "l4_portnum",
        .type = PFA_L4_PORTNUM,
        .filter_parser_ops = g_pfa_l4_portnum_array,
        .num_funcs = ARRAY_SIZE(g_pfa_l4_portnum_array),
        .logs = "l4 port number:"
    },
    {
        .str = "del_all_entrys",
        .type = PFA_DEL_ALL_ENTRYS,
        .filter_parser_ops = NULL,
        .num_funcs = 0,
        .logs = "del_all_entrys:"
    },
    {
        .str = "mac_ursp",
        .type = PFA_MAC_PDU_TABLE,
        .filter_parser_ops = g_mac_pdu_array,
        .num_funcs = ARRAY_SIZE(g_mac_pdu_array),
        .logs = "mac ursp:"
    },
    {
        .str = "ipv4_qos",
        .type = PFA_IPV4_QOS_TABLE,
        .filter_parser_ops = g_ip_qos_array,
        .num_funcs = ARRAY_SIZE(g_ip_qos_array),
        .logs = "ipv4 qos"
    },
    {
        .str = "ipv6_qos",
        .type = PFA_IPV6_QOS_TABLE,
        .filter_parser_ops = g_ip_qos_array,
        .num_funcs = ARRAY_SIZE(g_ip_qos_array),
        .logs = "ipv6 qos:"
    },
};


int pfa_sysfs_cmd_type(const char *buf, unsigned int buf_len)
{
    int ret;
    int i;

    ret = memset_s(&g_pfa_cmd, sizeof(g_pfa_cmd), 0, sizeof(struct pfa_cmd_handler_s));
    if (ret) {
        PFA_ERR("pfa_sysfs_cmd_type fail \n");
    }

    if (strcmp(buf, "ipv4") == 0) {
        g_pfa_cmd.u.ip_content.iptype = 0;
    }
    if (strcmp(buf, "ipv6") == 0) {
        g_pfa_cmd.u.ip_content.iptype = 1;
    }
    if (strcmp(buf, "ipv4_qos") == 0) {
        g_pfa_cmd.u.ip_qos.iptype = 0;
    }
    if (strcmp(buf, "ipv6_qos") == 0) {
        g_pfa_cmd.u.ip_qos.iptype = 1;
    }

    for (i = 0; i < sizeof(g_pfa_cmd_type) / sizeof(g_pfa_cmd_type[0]); i++) {
        if (strcmp(buf, g_pfa_cmd_type[i].str) == 0) {
            g_pfa_cmd.type = g_pfa_cmd_type[i].type;
            g_pfa_cmd.filter_parser_ops = g_pfa_cmd_type[i].filter_parser_ops;
            g_pfa_cmd.num_funcs = g_pfa_cmd_type[i].num_funcs;
            return PFA_OK;
        }
    }

    return PFA_FAIL;
}

int filter_mac_addr(const char *buf)
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

    if (pfa_mac_validate() != PFA_OK) {
        PFA_ERR("mac validate fail\n");
        return PFA_FAIL;
    }

    ret = memcpy_s(g_pfa_cmd.u.mac_content.mac, sizeof(g_pfa_cmd.u.mac_content.mac), mac, sizeof(mac));
    if (ret) {
        PFA_ERR("memcpy fail\n");
    }

    return PFA_OK;
}

int filter_action(const char *buf)
{
    if (0 == strcmp(buf, "add")) {
        g_pfa_cmd.action = PFA_ENTRY_ADD;
    } else if (0 == strcmp(buf, "delete")) {
        g_pfa_cmd.action = PFA_ENTRY_DELETE;
    } else if (0 == strcmp(buf, "dump")) {
        g_pfa_cmd.action = PFA_ENTRY_DUMP;
    } else {
        PFA_ERR("action %s is not supported !\n", buf);
        return PFA_FAIL;
    }

    PFA_ERR("action = %s\n", buf);
    return PFA_OK;
}

int forward_action(const char *buf)
{
    if (0 == strcmp(buf, "dump")) {
        g_pfa_cmd.action = PFA_ENTRY_DUMP;
    } else {
        PFA_ERR("action %s is not supported !\n", buf);
        return PFA_FAIL;
    }

    PFA_ERR("action = %s\n", buf);
    return PFA_OK;
}

int filter_mac_reaction(const char *buf)
{
    int ret;
    unsigned redir_action;
    ret = pfa_input(buf, "%4u", &redir_action);
    if (ret < 0) {
        return ret;
    }
    g_pfa_cmd.u.mac_content.action = redir_action;
    PFA_ERR("redir acriont = %d\n", redir_action);
    return PFA_OK;
}

int filter_mac_redirect(const char *buf)
{
    int ret;
    unsigned int port_index = 255;

    ret = pfa_input(buf, "%3d", &port_index);
    if (ret < 0) {
        return ret;
    }
    if (port_index > PFA_PORT_NUM) {
        PFA_ERR("redir port can't be %d\n", port_index);
        return PFA_FAIL;
    }

    g_pfa_cmd.u.mac_content.redirect = port_index;
    PFA_ERR("redir port = %d\n", port_index);
    return PFA_OK;
}

int filter_ip_mask(const char *buf)
{
    int ret;
    int mask;

    ret = pfa_input(buf, "%4x", &mask);
    if (ret < 0) {
        return ret;
    }
    g_pfa_cmd.u.ip_content.filter_mask = (unsigned int)mask;
    PFA_ERR("mask = 0x%x\n", mask);

    return PFA_OK;
}

int filter_ip_addr(const char *buf, int type)
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
        g_pfa_cmd.u.ip_content.u.pfa_ipv4.dst_addr = (ip[0] << 24) | (ip[1] << 16) | /* 16,24 ip addr shitf */
                                                     (ip[2] << 8) | ip[3]; /* 2,3,8 ip addr shitf */
    } else if (type == PFA_SRC_IP) {
        // src_addr
        g_pfa_cmd.u.ip_content.u.pfa_ipv4.src_addr = (ip[0] << 24) | (ip[1] << 16) | /* 16,24 ip addr shitf */
                                                     (ip[2] << 8) | ip[3]; /* 2,3,8 ip addr shitf */
        PFA_ERR("sip:*, *, %d, %d \n", ip[2], ip[3]); /* 2,3 ip addr shitf */
    } else {
        PFA_ERR("type = %d", type);
        return PFA_FAIL;
    }

    return PFA_OK;
}

int filter_ip_src_addr(const char *buf)
{
    return filter_ip_addr(buf, PFA_SRC_IP);
}

int filter_ip_dest_addr(const char *buf)
{
    return filter_ip_addr(buf, PFA_DEST_IP);
}

int filter_ip_addr_mask(const char *buf, int type)
{
    int ret;
    unsigned int mask;

    ret = pfa_input(buf, "%4d", &mask);
    if (ret < 0) {
        return ret;
    }

    if (mask > PFA_IPV4_MASK_LEN) {
        PFA_ERR("mask = %d", mask);
        return PFA_FAIL;
    }

    if (type == PFA_DEST_IP) {
        // dst_mask
        PFA_ERR("dmask:%d \n", mask);
        g_pfa_cmd.u.ip_content.u.pfa_ipv4.dst_addr_mask = mask;
    } else if (type == PFA_SRC_IP) {
        // src_mask
        PFA_ERR("smask:%d \n", mask);
        g_pfa_cmd.u.ip_content.u.pfa_ipv4.src_addr_mask = mask;
    } else {
        PFA_ERR("type = %d\n", type);
        return PFA_FAIL;
    }

    return PFA_OK;
}

int filter_ip_src_addr_mask(const char *buf)
{
    return filter_ip_addr_mask(buf, PFA_SRC_IP);
}

int filter_ip_dest_addr_mask(const char *buf)
{
    return filter_ip_addr_mask(buf, PFA_DEST_IP);
}

int filter_ipv4_protocol(const char *buf)
{
    if (0 == strcmp(buf, "tcp")) {
        g_pfa_cmd.u.ip_content.u.pfa_ipv4.protocol = IPPROTO_TCP;
    } else if (0 == strcmp(buf, "udp")) {
        g_pfa_cmd.u.ip_content.u.pfa_ipv4.protocol = IPPROTO_UDP;
    } else {
        PFA_ERR("protocol %s error ,and protocol should be tcp or udp\n", buf);
        return PFA_FAIL;
    }

    PFA_ERR("protocol = %s\n", buf);
    return PFA_OK;
}

int filter_tos(const char *buf)
{
    int ret;
    unsigned int tos = PFA_IPV4_TOS_DEF;
    ret = pfa_input(buf, "%1d", &tos);
    if (ret < 0) {
        return ret;
    }

    if (tos > PFA_IPV4_TOS_MAX) {
        PFA_ERR("tos = %d ,out of rage!\n", tos);
        return PFA_FAIL;
    }

    g_pfa_cmd.u.ip_content.u.pfa_ipv4.tos = tos;
    PFA_ERR("tos = %d\n", tos);
    return PFA_OK;
}

int filter_src_port_low(const char *buf)
{
    int ret;
    unsigned int src_port_low;
    ret = pfa_input(buf, "%5d", &src_port_low);
    if (ret < 0) {
        return ret;
    }
    g_pfa_cmd.u.ip_content.src_port_low = src_port_low;
    PFA_ERR("src port low is %d\n", src_port_low);
    return PFA_OK;
}

int filter_src_port_high(const char *buf)
{
    int ret;
    unsigned int src_port_high;
    ret = pfa_input(buf, "%5d", &src_port_high);
    if (ret < 0) {
        return ret;
    }
    g_pfa_cmd.u.ip_content.src_port_high = src_port_high;
    PFA_ERR("src port high is %d\n", src_port_high);
    return PFA_OK;
}

int filter_dest_port_low(const char *buf)
{
    int ret;
    unsigned int dst_port_low;
    ret = pfa_input(buf, "%5d", &dst_port_low);
    if (ret < 0) {
        return ret;
    }
    g_pfa_cmd.u.ip_content.dst_port_low = dst_port_low;
    PFA_ERR("dst port low is %d\n", dst_port_low);
    return PFA_OK;
}

int filter_dest_port_high(const char *buf)
{
    int ret;
    unsigned int dst_port_high;
    ret = pfa_input(buf, "%5d", &dst_port_high);
    if (ret < 0) {
        return ret;
    }
    g_pfa_cmd.u.ip_content.dst_port_high = dst_port_high;
    PFA_ERR("dst port high is %d\n", dst_port_high);
    return PFA_OK;
}

int pfa_black_white_mode_set(const char *buf)
{
    if (0 == strcmp(buf, "mac")) {
        g_pfa_cmd.pfa_blackwhite_mode = 0;
    } else if (0 == strcmp(buf, "ip")) {
        g_pfa_cmd.pfa_blackwhite_mode = 1;
    } else {
        PFA_ERR("paramate error! mode can't be %s\n", buf);
        return PFA_FAIL;
    }
    PFA_ERR("change %s black/white\n", buf);
    return PFA_OK;
}

int pfa_black_white_set(const char *buf)
{
    if (0 == strcmp(buf, "black")) {
        g_pfa_cmd.pfa_blackwhite = 1;
    } else if (0 == strcmp(buf, "white")) {
        g_pfa_cmd.pfa_blackwhite = 0;
    } else {
        PFA_ERR("paramate error! please input black or white\n");
        return PFA_FAIL;
    }
    PFA_ERR("change %s black/white\n", buf);
    return PFA_OK;
}

int set_action(const char *buf)
{
    if (0 == strcmp(buf, "set")) {
        g_pfa_cmd.pfa_param_action = PFA_PARAM_SET;
    } else if (0 == strcmp(buf, "get")) {
        g_pfa_cmd.pfa_param_action = PFA_PARAM_GET;
    } else {
        PFA_ERR("paramate error! action should be set or get");
        return PFA_FAIL;
    }

    PFA_ERR("action = %s\n", buf);
    return PFA_OK;
}

int pfa_eth_min_len_value_set(const char *buf)
{
    int ret;

    ret = pfa_input(buf, "%4d", &g_pfa_cmd.pfa_eth_set_min_len);
    if (ret < 0) {
        return ret;
    }
    PFA_TRACE("input value is %d\n", g_pfa_cmd.pfa_eth_set_min_len);

    // pfa_eth_minlen bit 0-10bits
    if (g_pfa_cmd.pfa_eth_set_min_len >= PFA_ETH_SET_MAX_LEN || g_pfa_cmd.pfa_eth_set_min_len <= PFA_ETH_SET_MIN_LEN) {
        PFA_ERR("out of range(17-2047):input value %d\n", g_pfa_cmd.pfa_eth_set_min_len);
        return PFA_FAIL;
    }

    PFA_ERR("set eth min len %d\n", g_pfa_cmd.pfa_eth_set_min_len);
    return PFA_OK;
}

int pfa_udp_lim_portno_set(const char *buf)
{
    int ret;
    unsigned int udp_lim_portno;

    ret = pfa_input(buf, "%4d", &udp_lim_portno);
    if (ret < 0) {
        return ret;
    }
    if (udp_lim_portno >= PFA_PORT_NUM) {
        PFA_ERR("out of range:port num can't be %d\n", udp_lim_portno);
        return PFA_FAIL;
    }
    g_pfa_cmd.pfa_udp_lim_portno = udp_lim_portno;
    PFA_ERR("set %d port udp limite\n", g_pfa_cmd.pfa_udp_lim_portno);
    return PFA_OK;
}

int udp_rate_set(const char *buf)
{
    int ret;
    unsigned int udp_rate;

    ret = pfa_input(buf, "0x%8x", &udp_rate);
    if (ret < 0) {
        return ret;
    }
    if (udp_rate > PFA_PORT_UDP_LIM_MAX) {
        PFA_ERR("out of range:udp limit can't be 0x%x\n", udp_rate);
        return PFA_FAIL;
    }
    g_pfa_cmd.pfa_udp_rate = udp_rate;

    PFA_ERR("set udp limite:0x%x \n", g_pfa_cmd.pfa_udp_rate);
    return PFA_OK;
}

int port_rate_set(const char *buf)
{
    int ret;

    ret = pfa_input(buf, "0x%8x", &g_pfa_cmd.pfa_port_rate);
    if (ret < 0) {
        return ret;
    }
    if (g_pfa_cmd.pfa_port_rate > PFA_PORT_BYTE_LIM_MAX) {
        PFA_ERR("out of range:byte limit can't be 0x%x\n", g_pfa_cmd.pfa_port_rate);
        return PFA_FAIL;
    }

    PFA_ERR("set byte limite:0x%x\n", g_pfa_cmd.pfa_port_rate);
    return PFA_OK;
}

int pfa_port_lim_time_set(const char *buf)
{
    int ret;

    ret = pfa_input(buf, "0x%4x", &g_pfa_cmd.pfa_port_lim_time);
    if (ret < 0) {
        return ret;
    }
    if (g_pfa_cmd.pfa_port_lim_time > PFA_PORT_LIM_TIME_MAX) {
        PFA_ERR("out of range:byte limit can't be 0x%x\n", g_pfa_cmd.pfa_port_lim_time);
        return PFA_FAIL;
    }

    PFA_ERR("set limite time 0x%x\n", g_pfa_cmd.pfa_port_lim_time);
    return PFA_OK;
}

int pfa_switch(const char *buf)
{
    if (0 == strcmp(buf, "on")) {
        g_pfa_cmd.onoff = 0;
    } else if (0 == strcmp(buf, "off")) {
        g_pfa_cmd.onoff = 1;
    } else {
        PFA_ERR("parameter error! switch should be on or off\n");
        return PFA_FAIL;
    }

    PFA_ERR("set switch %s\n", buf);
    return PFA_OK;
}

int pfa_l4_protocol_get(const char *buf)
{
    if (0 == strcmp(buf, "tcp")) {
        g_pfa_cmd.pfa_l4_protocol = IPPROTO_TCP;
    } else if (0 == strcmp(buf, "udp")) {
        g_pfa_cmd.pfa_l4_protocol = IPPROTO_UDP;
    } else {
        PFA_ERR("protocol %s error ,and protocol should be tcp or udp\n", buf);
        return PFA_FAIL;
    }

    PFA_ERR("protocol = %s\n", buf);
    return PFA_OK;
}

int pfa_l4_portnum_get(const char *buf)
{
    int ret;
    int tmp;
    ret = pfa_input(buf, "%5d", &tmp);
    if (ret < 0) {
        return ret;
    }
    g_pfa_cmd.pfa_l4_port_num = (u16)tmp;
    PFA_ERR("set l4 portnum : %d\n", g_pfa_cmd.pfa_l4_port_num);
    return PFA_OK;
}

int char_to_num(char c)
{
    if ((c >= '0') && (c <= '9')) {
        return c - '0';
    } else if ((c >= 'a') && (c <= 'f')) {
        return c - 'a' + 10; /* 10 dec */
    } else if ((c >= 'A') && (c <= 'F')) {
        return c - 'A' + 10; /* 10 dec */
    } else {
        return -1;
    }
}

void ipv6_resolution(char *ipv6_l_input, unsigned int *ipv6_m, unsigned int ipv6_m_len, int *m, int *n)
{
    int *l = m;
    int i = 0;
    char *ipv6_l = ipv6_l_input;

    while ('\0' != *ipv6_l) {
        if (':' == *ipv6_l) {
            l = n;
            ipv6_l++;
            continue;
        }
        while ((':' != *ipv6_l) && ('\0' != *ipv6_l)) {
            ipv6_m[i] = ipv6_m[i] * 16 + char_to_num(*ipv6_l); /* 16 hex */
            ipv6_l++;
            continue;
        }
        if ('\0' == *ipv6_l) {
            (*l)++;
            return;
        }
        ipv6_l++;
        (*l)++;
        i++;
    }
    return;
}

int pfa_build_ipv6_addr(unsigned int *ipv6, int type, int table_type)
{
    int ret = 0;
    unsigned int addr[4]; /* 4 is ip size */

    addr[0] = (ipv6[0] << 16) | ipv6[1]; /* 16 bit shift */
    addr[1] = (ipv6[2] << 16) | ipv6[3]; /* 2,3,16 ipv6 addr shift */
    addr[2] = (ipv6[4] << 16) | ipv6[5]; /* 2,4,5,16 ipv6 addr shift */
    addr[3] = (ipv6[6] << 16) | ipv6[7]; /* 3,6,7,16 ipv6 addr shift */
    if (table_type == PFA_IP_FILTER_TYPE) {
        if (type == PFA_DEST_IP) {
            ret = memcpy_s(g_pfa_cmd.u.ip_content.u.pfa_ipv6.dst_addr, sizeof(g_pfa_cmd.u.ip_content.u.pfa_ipv6.dst_addr), addr, sizeof(addr));
        } else if (type == PFA_SRC_IP) {
            ret = memcpy_s(g_pfa_cmd.u.ip_content.u.pfa_ipv6.src_addr, sizeof(g_pfa_cmd.u.ip_content.u.pfa_ipv6.src_addr), addr, sizeof(addr));
        } else {
            PFA_TRACE("type = %d", type);
            return PFA_FAIL;
        }
    } else if (table_type == PFA_IP_QOS_TYPE) {
        if (type == PFA_DEST_IP) {
            ret = memcpy_s(g_pfa_cmd.u.ip_qos.dip, sizeof(g_pfa_cmd.u.ip_qos.dip), addr, sizeof(addr));
        } else if (type == PFA_SRC_IP) {
            ret = memcpy_s(g_pfa_cmd.u.ip_qos.sip, sizeof(g_pfa_cmd.u.ip_qos.sip), addr, sizeof(addr));
        } else {
            PFA_TRACE("type = %d", type);
            return PFA_FAIL;
        }
    } else {
        PFA_TRACE("table_type = %d", table_type);
        return PFA_FAIL;
    }

    return ret;
}

int filter_ipv6_addr(const char *buf, int type, int table_type)
{
    int ret;
    unsigned int ipv6[PFA_IPV6_ADDR_LEN] = {0};
    unsigned int ipv6_m[PFA_IPV6_ADDR_LEN] = {0};
    char ipv6_l[PFA_IPV6_ADDR_BUF] = {0};
    int m = 0;
    int n = 0;
    int i = 0;
    int d;

    ret = pfa_input(buf, "%49s", ipv6_l, PFA_IPV6_ADDR_BUF);
    if (ret < 0) {
        return ret;
    }
    PFA_TRACE("%s\n", ipv6_l);
    ipv6_resolution(ipv6_l, ipv6_m, sizeof(ipv6_m), &m, &n);
    PFA_TRACE("ipv6_m  *:*:*:%x:%x:%x:%x:%x\n", ipv6_m[0], ipv6_m[1], ipv6_m[2], ipv6_m[3], /* 2,3 ipv6 addr shift */
              ipv6_m[4], ipv6_m[5], ipv6_m[6], ipv6_m[7]); /* 4,5,6,7 ipv6 addr shift */

    d = PFA_IPV6_ADDR_LEN - m - n;

    for (i = 0; i < m; i++) {
        ipv6[i] = ipv6_m[i];
    }

    for (i = 0; i < d; i++) {
        ipv6[m + i] = 0;
    }

    for (i = 0; i < n; i++) {
        ipv6[m + d + i] = ipv6_m[m + i];
    }

    return pfa_build_ipv6_addr(ipv6, type, table_type);
}

int filter_ipv6_src_addr(const char *buf)
{
    return filter_ipv6_addr(buf, PFA_SRC_IP, PFA_IP_FILTER_TYPE);
}

int filter_ipv6_dest_addr(const char *buf)
{
    return filter_ipv6_addr(buf, PFA_DEST_IP, PFA_IP_FILTER_TYPE);
}

int filter_ipv6_addr_mask(const char *buf, int type)
{
    int ret;
    unsigned int mask;

    ret = pfa_input(buf, "%4d", &mask);
    if (ret < 0) {
        return ret;
    }

    if (mask > PFA_IPV6_MASK_LEN) {
        PFA_ERR("out of range, mask shoule less then 128");
        return PFA_FAIL;
    }

    if (type == PFA_DEST_IP) {
        // dst_addr
        PFA_ERR("dmask:%d \n", mask);
        g_pfa_cmd.u.ip_content.u.pfa_ipv6.dst_addr_mask = mask;
    } else if (type == PFA_SRC_IP) {
        PFA_ERR("smask:%d \n", mask);
        g_pfa_cmd.u.ip_content.u.pfa_ipv6.src_addr_mask = mask;
    } else {
        PFA_ERR("type = %d error \n", type);
    }

    return PFA_OK;
}
int filter_ipv6_src_addr_mask(const char *buf)
{
    return filter_ipv6_addr_mask(buf, PFA_SRC_IP);
}

int filter_ipv6_dest_addr_mask(const char *buf)
{
    return filter_ipv6_addr_mask(buf, PFA_DEST_IP);
}

int filter_ipv6_protocol(const char *buf)
{
    if (0 == strcmp(buf, "tcp")) {
        g_pfa_cmd.u.ip_content.u.pfa_ipv6.protocol = IPPROTO_TCP;
    } else if (0 == strcmp(buf, "udp")) {
        g_pfa_cmd.u.ip_content.u.pfa_ipv6.protocol = IPPROTO_UDP;
    } else {
        PFA_ERR("protocol %s error ,and protocol should be tcp or udp\n", buf);
        return PFA_FAIL;
    }

    return PFA_OK;
}

int filter_ipv6_tc(const char *buf)
{
    int ret;
    unsigned int tc = PFA_IPV6_TC_DEF;
    ret = pfa_input(buf, "%1d", &tc);
    if (ret < 0) {
        return ret;
    }

    if (tc >= PFA_IPV6_TC_MAX) {
        PFA_ERR("tc = %s out of range\n", buf);
        return PFA_FAIL;
    }

    g_pfa_cmd.u.ip_content.u.pfa_ipv6.tc = tc;
    return PFA_OK;
}

int filter_ipv6_next_hdr(const char *buf)
{
    int ret;
    unsigned int hdr;
    ret = pfa_input(buf, "%4d", &hdr);
    if (ret < 0) {
        return ret;
    }

    g_pfa_cmd.u.ip_content.u.pfa_ipv6.next_hdr = hdr;
    return PFA_OK;
}

int filter_ipv6_flow_label(const char *buf)
{
    int ret;
    unsigned int label;
    ret = pfa_input(buf, "%4d", &label);
    if (ret < 0) {
        return ret;
    }

    g_pfa_cmd.u.ip_content.u.pfa_ipv6.flow_label = label;
    return PFA_OK;
}

static char pfa_dash2underscore(char c)
{
    if (c == '-') {
        return '_';
    }
    return c;
}

bool pfa_parameqn(const char *a, const char *b, size_t n)
{
    size_t i;

    for (i = 0; i < n; i++) {
        if (pfa_dash2underscore(a[i]) != pfa_dash2underscore(b[i]))
            return false;
    }
    return true;
}

bool pfa_parameq(const char *a, const char *b)
{
    return pfa_parameqn(a, b, strlen(a) + 1);
}

/*
 * cmd line parse routine begins here
 */
static int pfa_parse_one(char *param, const char *val)
{
    int i;
    int err;

    PFA_TRACE("param=%s,strlen(param)=%d,val=%s\n", param, strlen(param), val);

    if (val == NULL) {
        return -EINVAL;
    }

    /* Find parameter */
    PFA_TRACE("num:%d\n", g_pfa_cmd.num_funcs);
    for (i = 0; i < g_pfa_cmd.num_funcs; i++) {
        if (pfa_parameq(param, g_pfa_cmd.filter_parser_ops[i].name)) {
            /* No one handled NULL, so do it here. */
            PFA_TRACE("name:%s\n", g_pfa_cmd.filter_parser_ops[i].name);
            err = g_pfa_cmd.filter_parser_ops[i].setup_func(val);
            return err;
        }
    }

    PFA_ERR("unknown argument: %s\n", param);
    return PFA_FAIL;
}

static char *find_next_arg(char *args, char **param, char **val)
{
    unsigned int i;
    unsigned int equals = 0;
    char *next = NULL;

    for (i = 0; args[i]; i++) {
        if (isspace(args[i])) {
            break;
        }
        if (equals == 0) {
            if (args[i] == '=') {
                equals = i;
            }
        }
    }

    *param = args;
    if (!equals) {
        *val = NULL;
    } else {
        args[equals] = '\0';
        *val = args + equals + 1;
    }

    if (args[i]) {
        args[i] = '\0';
        next = args + i + 1;
    } else {
        next = args + i;
    }

    /* Chew up trailing spaces. */
    return skip_spaces(next);
}

int exe_pfa_mac_filter_cmd(void)
{
    int ret = PFA_OK;

    if (g_pfa_cmd.action == PFA_ENTRY_DUMP) {
        pfa_macfl_entry_dump();
    } else {
        if (g_pfa_cmd.type == PFA_MAC_FILTER_TABLE) {
            // table_ops ipv4_filt_act
            ret = pfa_mac_filt_act(&g_pfa_cmd);
            if (ret) {
                PFA_ERR("pfa mac filt act err.\n");
            }
        }
    }

    if (ret) {
        PFA_ERR("exe pfa mac filter cmd error!\n");
    }
    return ret;
}

int exe_pfa_mac_forward_cmd(void)
{
    if (g_pfa_cmd.action == PFA_ENTRY_DUMP) {
        pfa_macfw_entry_dump();
    }

    return 0;
}

int exe_pfa_black_white_cmd(enum pfa_tab_list_type list_type, unsigned int black_white)
{
    pfa_black_white_t black_white_reg;
    struct pfa *pfa = &g_pfa;

    black_white_reg.u32 = pfa_readl(pfa->regs, PFA_BLACK_WHITE);
    if (g_pfa_cmd.pfa_param_action == PFA_PARAM_SET) {
        switch (list_type) {
            case PFA_TAB_LIST_MAC_FILT:
                black_white_reg.bits.pfa_mac_black_white = !!black_white;
                pfa_writel(pfa->regs, PFA_BLACK_WHITE, black_white_reg.u32);
                break;

            case PFA_TAB_LIST_IPV4_FILT:
            case PFA_TAB_LIST_IPV6_FILT:
                black_white_reg.bits.pfa_ip_black_white = !!black_white;
                pfa_writel(pfa->regs, PFA_BLACK_WHITE, black_white_reg.u32);
                break;

            default:
                PFA_ERR("invalid list_type(%d)\n", list_type);
                break;
        }
        PFA_TRACE("pfa set black white \n");
    } else if (g_pfa_cmd.pfa_param_action == PFA_PARAM_GET) {
        PFA_ERR("mac:%s, ip:%s\n", black_white_reg.bits.pfa_mac_black_white ? "black" : "white",
                black_white_reg.bits.pfa_ip_black_white ? "black" : "white");
    } else {
        PFA_ERR("black/white set parameter error!\n");
    }
    return 0;
}

void pfa_min_len_act(unsigned int min_len, unsigned int is_write)
{
    struct pfa *pfa = &g_pfa;
    unsigned int minlen;

    minlen = pfa_readl(pfa->regs, PFA_ETH_MINLEN);

    if (is_write) {
        pfa_writel(pfa->regs, PFA_ETH_MINLEN, min_len);
    }
    PFA_ERR("eth min len:0x%x\n", minlen);
    return;
}

int exe_pfa_eth_min_len_cmd(void)
{
    if (g_pfa_cmd.type == PFA_ETH_MIN_LEN) {
        if (g_pfa_cmd.pfa_param_action == PFA_PARAM_SET) {
            pfa_min_len_act(g_pfa_cmd.pfa_eth_set_min_len, 1);
        } else if (g_pfa_cmd.pfa_param_action == PFA_PARAM_GET) {
            pfa_min_len_act(g_pfa_cmd.pfa_eth_set_min_len, 0);
        } else {
            PFA_ERR("eth_min_len set parameter error!\n");
        }
    }
    return 0;
}

int exe_pfa_ip_filter_cmd(void)
{
    int ret = PFA_OK;

    if (g_pfa_cmd.action == PFA_ENTRY_DUMP) {
        if (g_pfa_cmd.u.ip_content.iptype == 0) {
            pfa_ipv4_filter_dump();
        }

        if (g_pfa_cmd.u.ip_content.iptype == 1) {
            pfa_ipv6_filter_dump();
        }
    } else {
        if (g_pfa_cmd.type == PFA_IP_FILTER_TABLE) {
            // table_ops ipv4_filt_act
            ret = pfa_ipv4_filt_act(&g_pfa_cmd);
            if (ret) {
                PFA_ERR("pfa ipv4 filt act err.\n");
            }
        } else if (g_pfa_cmd.type == PFA_IPV6_FILTER_TABLE) {
            // table_ops ipv6_filt_act
            ret = pfa_ipv6_filt_act(&g_pfa_cmd);
            if (ret) {
                PFA_ERR("pfa ipv6 filt act err.\n");
            }
        }
    }
    return ret;
}

unsigned int pfa_udp_limit_num_action(unsigned int portno, unsigned int udp_limi_max, unsigned int is_write)
{
    struct pfa *pfa = &g_pfa;
    pfa_port_lmttime_t en_status;

    if (is_write) {
        pfa_writel(pfa->regs, PFA_PORTX_UDP_LMTNUM(portno), udp_limi_max);
        en_status.u32 = pfa_readl(pfa->regs, PFA_PORT_LMTTIME);
        if (udp_limi_max != 0) {
            en_status.bits.pfa_port_lmt_en |= 1 << portno;
        } else {
            en_status.bits.pfa_port_lmt_en &= ~(1 << portno);
        }
        pfa_writel(pfa->regs, PFA_PORT_LMTTIME, en_status.u32);
        return 0;
    }

    return pfa_readl(pfa->regs, PFA_PORTX_UDP_LMTNUM(portno));
}

unsigned int pfa_set_byte_limit_num(unsigned int portno, unsigned int byte_limi_max, unsigned int is_write)
{
    struct pfa *pfa = &g_pfa;
    pfa_port_lmttime_t en_status;

    if (is_write) {
        pfa_writel(pfa->regs, PFA_PORTX_LMTBYTE(portno), byte_limi_max);
        en_status.u32 = pfa_readl(pfa->regs, PFA_PORT_LMTTIME);
        if (byte_limi_max != 0) {
            en_status.bits.pfa_port_lmt_en |= 1 << portno;
        } else {
            en_status.bits.pfa_port_lmt_en &= ~(1 << portno);
        }
        pfa_writel(pfa->regs, PFA_PORT_LMTTIME, en_status.u32);

        return 0;
    }

    return pfa_readl(pfa->regs, PFA_PORTX_LMTBYTE(portno));
}

void pfa_set_port_limit(unsigned int limit_time, unsigned int is_write)
{
    struct pfa *pfa = &g_pfa;
    pfa_port_lmttime_t lim_time;
    lim_time.u32 = pfa_readl(pfa->regs, PFA_PORT_LMTTIME);

    if (is_write) {
        lim_time.bits.pfa_port_lmttime = limit_time;
        pfa_writel(pfa->regs, PFA_PORT_LMTTIME, lim_time.u32);
    }
    PFA_TRACE("limit time:0x%x\n", lim_time.u32);
}

int exe_pfa_dynamic_swich_cmd(void)
{
    int ret = PFA_OK;
    if (g_pfa_cmd.onoff) {
        pfa_transfer_pause();
    } else {
        pfa_transfer_restart();
    }
    PFA_ERR("dynamic swich is %s \n", g_pfa_cmd.onoff ? "off" : "on");

    return ret;
}

int exe_pfa_port_l4portno_cmd(void)
{
    int ret = PFA_OK;

    if (g_pfa_cmd.action == PFA_ENTRY_ADD) {
        ret = pfa_port_l4portno_to_cpu_add(g_pfa_cmd.pfa_l4_port_num, g_pfa_cmd.pfa_l4_protocol);
    } else if (g_pfa_cmd.action == PFA_ENTRY_DELETE) {
        ret = pfa_port_l4portno_to_cpu_del(g_pfa_cmd.pfa_l4_port_num, g_pfa_cmd.pfa_l4_protocol);
    } else if (g_pfa_cmd.action == PFA_ENTRY_DUMP) {
        pfa_port_l4portno_to_cpu_dump();
    } else {
        PFA_ERR("l4 portnum set parameter error!\n");
    }

    return ret;
}

int exe_pfa_black_white_list_cmd(void)
{
    return exe_pfa_black_white_cmd(g_pfa_cmd.pfa_blackwhite_mode, g_pfa_cmd.pfa_blackwhite);
}

int exe_pfa_udp_limit_cmd(void)
{
    pfa_udp_limit_num_action(g_pfa_cmd.pfa_udp_lim_portno, g_pfa_cmd.pfa_udp_rate, g_pfa_cmd.pfa_param_action);
    return 0;
}

int exe_pfa_byte_limit_cmd(void)
{
    pfa_set_byte_limit_num(g_pfa_cmd.pfa_udp_lim_portno, g_pfa_cmd.pfa_port_rate, g_pfa_cmd.pfa_param_action);
    return 0;
}

int exe_pfa_port_limit_cmd(void)
{
    pfa_set_port_limit(g_pfa_cmd.pfa_port_lim_time, g_pfa_cmd.pfa_param_action);
    return 0;
}

int exe_pfa_del_all_entrys_cmd(void)
{
    pfa_ip_fw_del_all_entry();
    PFA_ERR("not support for now. \n");
    return 0;
}

struct pfa_sys_cmd_handler g_pfa_sys_cmd_handler[PFA_SYS_CMD_TYPE_END] = {0};
void pfa_sys_cmd_handler_init(void)
{
    g_pfa_sys_cmd_handler[PFA_IP_FILTER_TABLE].callback = exe_pfa_ip_filter_cmd;
    g_pfa_sys_cmd_handler[PFA_IPV6_FILTER_TABLE].callback = exe_pfa_ip_filter_cmd;
    g_pfa_sys_cmd_handler[PFA_MAC_FILTER_TABLE].callback = exe_pfa_mac_filter_cmd;
    g_pfa_sys_cmd_handler[PFA_MAC_FORWARD_TABLE].callback = exe_pfa_mac_forward_cmd;
    g_pfa_sys_cmd_handler[PFA_BLACK_WHITE_LIST].callback = exe_pfa_black_white_list_cmd;
    g_pfa_sys_cmd_handler[PFA_ETH_MIN_LEN].callback = exe_pfa_eth_min_len_cmd;
    g_pfa_sys_cmd_handler[PFA_UDP_LMTNUM_TABLE].callback = exe_pfa_udp_limit_cmd;
    g_pfa_sys_cmd_handler[PFA_LMTBYTE_TABLE].callback = exe_pfa_byte_limit_cmd;
    g_pfa_sys_cmd_handler[PFA_LMT_TIME_TABLE].callback = exe_pfa_port_limit_cmd;
    g_pfa_sys_cmd_handler[PFA_DYNAMIC_SWITCH].callback = exe_pfa_dynamic_swich_cmd;
    g_pfa_sys_cmd_handler[PFA_L4_PORTNUM].callback = exe_pfa_port_l4portno_cmd;
    g_pfa_sys_cmd_handler[PFA_DEL_ALL_ENTRYS].callback = exe_pfa_del_all_entrys_cmd;
    g_pfa_sys_cmd_handler[PFA_MAC_PDU_TABLE].callback = exe_pfa_mac_pdu_cmd;
    g_pfa_sys_cmd_handler[PFA_IPV4_QOS_TABLE].callback = exe_pfa_ip_qos_cmd;
    g_pfa_sys_cmd_handler[PFA_IPV6_QOS_TABLE].callback = exe_pfa_ip_qos_cmd;
}

int exe_pfa_cmd(void)
{
    if (g_pfa_cmd.type <= PFA_SYS_CMD_TYPE_START || g_pfa_cmd.type >= PFA_SYS_CMD_TYPE_END) {
        PFA_ERR("g_pfa_cmd.type = %d, error", g_pfa_cmd.type);
        return PFA_FAIL;
    }

    return g_pfa_sys_cmd_handler[g_pfa_cmd.type].callback();
}

int filter_cmd_parser(char *args, unsigned int args_len)
{
    char *param = NULL;
    char *val = NULL;
    int ret;
    char type_str[PFA_STR_LEN] = {0};

    if (args == NULL) {
        return PFA_FAIL;
    }

    // clean all field
    ret = memset_s(&g_pfa_cmd, sizeof(g_pfa_cmd), 0, sizeof(struct pfa_cmd_handler_s));
    if (ret) {
        PFA_ERR("pfa_sysfs_cmd_type g_pfa_cmd clean fail. \n");
    }

    /* clean leading spaces */
    args = skip_spaces(args);
    if (args == NULL) {
        return PFA_FAIL;
    }
    // cmdline must start with  "type=xx"  xx : ip, ipv6, mac
    // get type first
    if (0 != strncmp(args, "type=", strlen("type="))) {
        filter_cmd_help(args);
        return PFA_FAIL;
    }

    ret = memset_s(type_str, sizeof(type_str), 0, PFA_STR_LEN);
    if (ret) {
        PFA_ERR("pfa_sysfs_cmd_type type_str clean fail \n");
    }
    // need to limit size of type_str
    ret = pfa_input(args, "type=%15s", type_str, PFA_STR_LEN);
    if (ret < 0) {
        return ret;
    }
    ret = pfa_sysfs_cmd_type(type_str, sizeof(type_str));
    if (ret == PFA_FAIL) {
        PFA_ERR("cmd: %s %s fail\n", args, type_str);
    }

    args = args + strlen(type_str) + strlen("type=");
    args = skip_spaces(args);

    while ((args != NULL) && *args) {
        args = find_next_arg(args, &param, &val);
        ret = pfa_parse_one(param, val);
        switch (ret) {
            case PFA_FAIL:
                break;
            case PFA_OK:
                break;
            default:
                PFA_ERR("'%s' invalid for parameter '%s'\n", val ? : "", param);
                return ret;
        }
    }

    return ret;
}

void filter_cmd_help(char *args)
{
    if (args != NULL) {
        PFA_TRACE("cmd: %s\n", args);
    }

    PFA_ERR("cmd should start with type=ip, type=ipv6 or type=mac\n");

    return;
}

void pfa_ipv4_filter_dump(void)
{
    unsigned int pfa_tab[PFA_IPV4_TAB_WORD_NO];
    int i, j;
    struct pfa *pfa = &g_pfa;

    pfa->hal->dbgen_en();

    for (i = 0; i < PFA_IPV4_TAB_NUMBER; i++) {
        for (j = 0; j < PFA_IPV4_TAB_WORD_NO; j++) {
            pfa_tab[j] = pfa_readl(pfa->regs, PFA_IPV4_TAB(i * PFA_IPV4_TAB_WORD_NO + j));
        }
        ipv4_filt_dump(pfa_tab, i);
    }
    pfa->hal->dbgen_dis();
}

void pfa_ipv6_filter_dump(void)
{
    struct pfa *pfa = &g_pfa;
    unsigned int pfa_tab[PFA_IPV6_TAB_WORD_NO];
    int i, j;

    pfa->hal->dbgen_en();

    for (i = 0; i < PFA_IPV6_TAB_NUMBER; i++) {
        for (j = 0; j < PFA_IPV6_TAB_WORD_NO; j++) {
            pfa_tab[j] = pfa_readl(pfa->regs, PFA_IPV6_TAB(i * PFA_IPV6_TAB_WORD_NO + j));
        }
        ipv6_filt_dump(pfa_tab, i);
    }
    pfa->hal->dbgen_dis();
}
