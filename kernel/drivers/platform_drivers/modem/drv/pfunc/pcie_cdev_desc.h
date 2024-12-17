/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2018-2020. All rights reserved.
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

#ifndef __PCIE_CDEV_DESC_H__
#define __PCIE_CDEV_DESC_H__

#ifdef __cplusplus
extern "C" {
#endif

#define NUM_R2E_DESC 256
#define NUM_E2R_DESC 256
#define MAX_MSC_SIZE 16

typedef enum UNIFY_IF_PROT_T {
    IF_PROTOCOL_VOID = 0x00,
    IF_PROTOCOL_3G_MODEM = 0x01,
    IF_PROTOCOL_3G_PCUI = 0x02,
    IF_PROTOCOL_3G_DIAG = 0x03,
    IF_PROTOCOL_PCSC = 0x04,
    IF_PROTOCOL_3G_GPS = 0x05,
    IF_PROTOCOL_CTRL = 0x06,
    IF_PROTOCOL_3G_NDIS = 0x07,
    IF_PROTOCOL_NDISDATA = 0x08,
    IF_PROTOCOL_NDISCTRL = 0x09,
    IF_PROTOCOL_BLUETOOTH = 0x0A,
    IF_PROTOCOL_FINGERPRINT = 0x0B,
    IF_PROTOCOL_ACMCTRL = 0x0C,
    IF_PROTOCOL_MMS = 0x0D,
    IF_PROTOCOL_3G_PCVOICE = 0x0E,
    IF_PROTOCOL_DVB = 0x0F,
    IF_PROTOCOL_MDM = 0x10,
    IF_PROTOCOL_NDIS = 0x11,
    IF_PROTOCOL_PCUI = 0x12,
    IF_PROTOCOL_DIAG = 0x13,
    IF_PROTOCOL_GPS = 0x14,
    IF_PROTOCOL_PCVOICE = 0x15,
    IF_PROTOCOL_NCM = 0x16,
    IF_PROTOCOL_CDMA_LOG = 0x17,
    IF_PROTOCOL_AGENT_NV = 0x30,
    IF_PROTOCOL_AGENT_OM = 0x31,
    IF_PROTOCOL_AGENT_MSG = 0x32,
    IF_PROTOCOL_APPVCOM = 0x33,
    IF_PROTOCOL_MODEM = 0x34,
    IF_PROTOCOL_PMOM = 0x35,
    IF_PROTOCOL_RESERVE = 0x40,
    IF_PROTOCOL_CDROM = 0xA1,
    IF_PROTOCOL_SDRAM = 0xA2,
    IF_PROTOCOL_RNDIS = 0xA3,

    IF_PROTOCOL_NOPNP = 0xFF
} UNIFY_IF_PROT;

struct pcdev_ports_desc {
    unsigned int allocked;  // allocked : 0X5A5A
    unsigned int port_num;
};

struct pcdev_desc {
    unsigned int own;  // 0:rx  1:tc
    unsigned int reserved0;
    unsigned long long s_vaddr;
    unsigned long long d_vaddr;
    unsigned int s_dma_low;
    unsigned int s_dma_high;    
    unsigned int d_dma_low;
    unsigned int d_dma_high;
    unsigned int data_size;
    unsigned int reserved1;
    unsigned long long userfield0;  // port
    unsigned long long userfield1;
};

enum pcie_cdev_irq {
    RX_CMP = 0,
    TX_CMP,
    EVENT_SEND,
    READ_SIG_SEND,
    REL_IND_SEND,
    MSC_STRU_SEND,
    DMA_SEND,
    DMA_SEND_CMP,
    IRQ_NUM,
};

union pcie_cdev_status {
    struct {
        unsigned int open : 1;
        unsigned int init : 1;
        unsigned int read_start : 1;
        unsigned int rsv : 29;
    } bits;
    unsigned int u32;
};

struct pcie_cdev_map_ep {
    UNIFY_IF_PROT port_id;
    union pcie_cdev_status self_status;
    union pcie_cdev_status peer_status;
    u32 rcv_irq_status[IRQ_NUM];
    u32 snd_irq_status[IRQ_NUM];

    unsigned int event_type;
    unsigned int rel_ind;
    char read_sig[MAX_MSC_SIZE];
    char msc_stru[MAX_MSC_SIZE];
    unsigned int msc_busy;

    unsigned int rx_wp;
    unsigned int rx_rp;
    unsigned int tx_rp_todo_rc;
    unsigned int tx_rp_dma_cmp_rc;
    unsigned int tx_wp;
    unsigned int tx_rp;
    unsigned int tx_rp_todo_ep;
    unsigned int rx_max_size;
    unsigned int tx_max_size;
    unsigned int reserved;
    struct pcdev_desc rx[NUM_R2E_DESC];
    struct pcdev_desc tx[NUM_E2R_DESC];
};

struct pcie_cdev_map_rc {
    UNIFY_IF_PROT port_id;
    union pcie_cdev_status peer_status;
    union pcie_cdev_status self_status;
    u32 snd_irq_status[IRQ_NUM];
    u32 rcv_irq_status[IRQ_NUM];

    unsigned int event_type;
    unsigned int rel_ind;
    char read_sig[MAX_MSC_SIZE];
    char msc_stru[MAX_MSC_SIZE];
    unsigned int msc_busy;

    // unsigned int is_dma;
    unsigned int tx_wp;
    unsigned int tx_rp;
    unsigned int tx_rp_todo_rc;
    unsigned int tx_rp_dma_cmp_rc;
    unsigned int rx_wp;
    unsigned int rx_rp;
    unsigned int tx_rp_todo_ep;
    unsigned int tx_max_size;
    unsigned int rx_max_size;
    unsigned int reserved;
    struct pcdev_desc tx[NUM_R2E_DESC];
    struct pcdev_desc rx[NUM_E2R_DESC];
};

union pcie_cdev_map {
    struct pcie_cdev_map_ep ep;
    struct pcie_cdev_map_rc rc;
};

void port_desc_map(unsigned int port_num, unsigned int mode);
unsigned long pcie_cdev_get_bar_size(void);

#ifdef __cplusplus
}
#endif
#endif
