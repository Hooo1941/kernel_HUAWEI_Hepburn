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

#ifndef __PFA_PORT_H__
#define __PFA_PORT_H__

#include "pfa.h"

#define PORT_TD_EVT_GAP_DEF 4
#define PORT_RD_EVT_GAP_DEF 0

#define PORT_UDP_LIMIT_TIME_DEF 0
#define PORT_UDP_LIMIT_CNT_DEF 0
#define PORT_RATE_LIMIT_TIME_DEF 0
#define PORT_RATE_LIMIT_BYTE_DEF 0

#define PFA_INTR_DEF_INTEVAL (8)  // unit 30.5us
#define PFA_INTR_MIN_INTERVAL 0x1
#define PFA_CPU_PORT_DEF_MAX_RATE 30000  // unit mbps; After testing,this value corresponds to 1.6Gbps

#define PFA_PER_INTERVAL_CALC(x) ((x)*pfa->irq_interval * 250 / 3 / 32768)

#define PFA_EVENT_DISABLE 0x8000FFFF
#define PFA_EVENT_ENABLE 0x0000FFFF

#define PFA_EVT_AD_EMPTY (32)

#define PFA_DEF_PORT_PRI 0xf
#define PFA_DEF_USB_ENC_PKT_CNT 50
#define PFA_DEF_USB_MAX_SIZE (63 * 1024)
#define PFA_DEF_USB_UNPACK_MAX_SIZE (16 * 1024)
#define PFA_DEF_PACK_ALIGN 0
#define PFA_DEF_PACK_DIV 896

#define PFA_GET_PHY_PORT(x) ((x) & 0xff)
#define PFA_NET_ID_BY_VIR_PORT(x) (((x) >> 8) & 0xf)

typedef union
{
    struct {
        unsigned int portno : 8;
        unsigned int net_id : 8;
        unsigned int reserved : 16;
    } bits;
    unsigned int u32;
} pfa_port_t;

enum pfa_attach_brg_type {
    pfa_attach_brg_brif,
    pfa_attach_brg_normal,
    pfa_attach_brg_mix,
    pfa_attach_brg_br,
    pfa_attach_brg_bottom
};
enum pfa_port_td_copy_cfg {
    PFA_PORT_TD_COPY_DISABLE,
    PFA_PORT_TD_COPY_ENABLE
};

enum pfa_port_stick_cfg {
    PFA_PORT_STICK_DISABLE,
    PFA_PORT_STICK_ENABLE
};

#ifdef CONFIG_PFA_FW
int pfa_alloc_br_port(void);
int pfa_free_br_port(unsigned portno);
void pfa_port_attach_brg_modify(int portno, enum pfa_attach_brg_type attach_brg);
#endif

void pfa_intr_set_interval(unsigned irq_interval);
int pfa_enable_port(int portno);
int pfa_disable_port(struct pfa *pfa, int portno);
void pfa_en_port_stick_mode(struct pfa *pfa, unsigned int bypassport);
void pfa_ackpkt_identify(struct pfa *pfa, unsigned int portno);
void pfa_port_init(struct pfa *pfa);
void pfa_set_port_property_brg(unsigned int portno, unsigned int brg_flag);
void pfa_rwptr_show(void);
void pfa_print_packet_stream(void);
void pfa_port_attach_brg_modify(int portno, enum pfa_attach_brg_type attach_brg);
void pfa_set_cpuport_maxrate(unsigned rate);
int pfa_port_set_mac(unsigned int portno, const unsigned char *mac);
void pfa_relay_port_config_init(struct pfa *pfa);

#endif /* __PFA_PORT_H__ */
