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
 * * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
IS"
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

#ifndef __BSP_PFA_H__
#define __BSP_PFA_H__

#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/types.h>
#include <linux/etherdevice.h>
#include <linux/if_ether.h>
#include <linux/netfilter/nf_conntrack_common.h>
#include <mdrv_bmi_pfa.h>

#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_tuple.h>

#include <product_config.h>

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /*__cplusplus*/

#define PFA_RD_UPDATEONLY BIT(0)
#define PFA_RD_INTR_DONE BIT(1)

#define PFA_TD_INTR_DONE BIT(0)
#define PFA_TD_PUSH_USB BIT(1)
#define PFA_TD_KICK_PKT BIT(2)

#define PFA_GETHER_RD_DIV 896
#define PFA_DEFAULT_CPU_PORT 0
#define PFA_DEFAULT_PFA_TFT_PORT 1

#define PFA_ACCELETABLE_PKT_FLAG 1
#define PFA_BYPASS_ENABLE_FLAG 0x5F5F5F5F
#define VCOM_IOCTL_ARP_FLAG 0x1E1E1E1E

enum pfa_reserve_port_vir_id {
    PFA_ACORE_PPP_NDIS_CTRL_VIR_ID = 0,
    PFA_ACORE_LAN_CTRL_VIR_ID,
    PFA_CCORE_PPP_NDIS_CTRL_VIR_ID,
    PFA_RSV_PORT_VIR_ID_MAX
};

struct iport_pfa_tft_addr {
    dma_addr_t ulrd_wptr_addr;  //  refer to PFA_TFT ULBD WPTR
    dma_addr_t dltd_rptr_vir_addr;  //  refer to PFA_TFT DLRD RPTR
    dma_addr_t dltd_rptr_phy_addr;  //  refer to PFA_TFT DLRD RPTR
};

struct iport_pfa_addr {
    dma_addr_t dltd_base_addr;  //  refer to PFA_TFT DLRD BASE
    dma_addr_t ulrd_base_addr;  //  refer to PFA_TFT ULBD BASE
    void* dltd_base_addr_v;  //  refer to PFA_TFT DLRD BASE
    void* ulrd_base_addr_v;  //  refer to PFA_TFT ULBD BASE
    dma_addr_t ulrd_rptr_addr;  //  refer to PFA_TFT ULBD RPTR
    dma_addr_t dltd_wptr_addr;  //  refer to PFA_TFT DLRD WPTR
    void* ulrd_rptr_addr_v;  //  refer to PFA_TFT ULBD RPTR
    void* dltd_wptr_addr_v;  //  refer to PFA_TFT DLRD WPTR
};

struct pfa_usbport_attr {
    enum pfa_enc_type enc_type;             /* encap type */
    unsigned int gether_max_size;
    unsigned int gether_max_pkt_cnt;
    unsigned int gether_timeout;
    unsigned int gether_align_parameter;
    struct net_device *net;
};

struct pfa_tft_port_attr {
    struct iport_pfa_tft_addr pfa_tft_addr;
    struct iport_pfa_addr pfa_addr;
    unsigned int ext_desc_en;
    void (*check_pfa_tft_bd)(void);
};

struct pfa_pkt_om_info {
    unsigned int pfa_dra_addr_lower;
    unsigned int pfa_dra_addr_higher;

    unsigned int pfa_portno:4;    // 0-15
    unsigned int pfa_dir:1;    // 0=td 1=rd;
    unsigned int pfa_pkt_type:3;
    unsigned int pfa_reserved:8;
    unsigned int l3_id:16;    // ip id

    unsigned int l4_seq;
    unsigned int time_stamp;
};

struct pfa_task_time {
    unsigned int int_time;
    unsigned int task_begin_time;
    unsigned int task_end_time;
};

struct pfa_rd_to_cpu_rsn {
    unsigned int pkt_err;
    unsigned int mac_non_stick_v4_v6;
    unsigned int first_mac_ipver_err;
    unsigned int first_mac_dmac_smac_diff;
    unsigned int first_mac_mac_trans_fail;
    unsigned int snd_mac_brg_dmac_fail;
    unsigned int l3_check_l3_protocol_fail;
    unsigned int l3_check_ipv4_hdr_fail;
    unsigned int ipv6_hop_limit_1;
    unsigned int ipv4_hdr_len_err;
    unsigned int ipv4_one_llt_left;
    unsigned int ipv4_slice_pkt;
    unsigned int l4_port_match;
    unsigned int ip_table_mismatch;
    unsigned int stick_arp_pkt;
    unsigned int fwsucc_or_discard;
    unsigned int ecm2usb_bycpu;
};

struct pfa_port_om_info {
    unsigned int rd_finished;
    unsigned int rd_finished_bytes;
    unsigned int rd_finsh_pkt;
    unsigned int rd_droped;
    unsigned int rd_sended;
    unsigned int rd_full;
    unsigned int rd_larger;
    unsigned int td_config;
    unsigned int td_full;
    unsigned int td_complete;
    unsigned int td_fail_drop;
    unsigned int td_comp;
};


struct pfa_cpu_port_om_info {
    unsigned int updonly_comp;
    unsigned int rd_num;
    unsigned int rd_udp_drop;
    unsigned int rd_wan;
    unsigned int rd_wan_fail;
    unsigned int rd_nic;
    unsigned int rd_nic_fail;
    unsigned int rd_netif;
    unsigned int rd_netif_succ;
    unsigned int rd_netif_fail;
};

struct pfa_direct_fw_entry_ctx {
    unsigned int devid:16;
    unsigned int skb_cb_info:1;
    unsigned int add_wan_info:1;
    unsigned int maped:1;
    unsigned int single_check:1;
    unsigned int l2_hdr_len:5;
    unsigned int net_id:2;
    unsigned int reserved:5;
    unsigned char dest_hd[ETH_ALEN]; /* destination mac addr */
    unsigned char src_hd[ETH_ALEN]; /* source mac addr */
};

#if (defined(CONFIG_PFA))
// pfa_port.c
int bsp_pfa_alloc_port(struct pfa_port_comm_attr *attr);
int bsp_pfa_free_port(int portno);
int bsp_pfa_set_portmac(int portno, const char* mac_addr);
int bsp_pfa_update_bypass_portno(unsigned int src_portno, unsigned int dst_portno);
int bsp_pfa_set_usbproperty(struct pfa_usbport_attr *usb_attr);
int bsp_pfa_set_pfa_tftproperty(struct pfa_tft_port_attr *pfa_tft_attr);
int bsp_pfa_enable_port(int portno);
int bsp_pfa_disable_port(int portno);
void bsp_pfa_set_pfa_tftmac(struct ethhdr *mac_addr);
void bsp_pfa_transfer_pause(void);
void bsp_pfa_transfer_restart(void);

int bsp_pfa_update_ipfw_entry(struct wan_dev_info_s* wan_info);

int bsp_pfa_config_td(int portno, unsigned long long dra_l2addr,
    unsigned int len, void *param, unsigned int pfa_flags);
int bsp_pfa_update_net(unsigned int portno, struct net_device *ndev, unsigned int alloced);
int bsp_pfa_get_reserve_port_id(enum pfa_reserve_port_vir_id id);

#else

static inline int bsp_pfa_alloc_port(struct pfa_port_comm_attr *attr){return 0;}
static inline int bsp_pfa_free_port(int portno){return 0;}
static inline int bsp_pfa_set_portmac(int portno, const char* mac_addr){return 0;}
static inline int bsp_pfa_set_usbproperty(struct pfa_usbport_attr *usb_attr){return 0;}
static inline int bsp_pfa_set_pfa_tftproperty(struct pfa_tft_port_attr *pfa_tft_attr){return 0;}
static inline int bsp_pfa_enable_port(int portno){return 0;}
static inline int bsp_pfa_disable_port(int portno){return 0;}
static inline void bsp_pfa_set_pfa_tftmac(struct ethhdr *mac_addr){return;}
static inline void bsp_pfa_transfer_pause(void){return;}
static inline void bsp_pfa_transfer_restart(void){return;}

static inline int bsp_pfa_update_ipfw_entry(struct wan_dev_info_s* wan_info){return 0;}


static inline int bsp_pfa_config_td(int portno, unsigned long long dra_l2addr,
    unsigned int len, void *param, unsigned int pfa_flags){return 0;}
static inline int bsp_pfa_update_net(unsigned int portno, struct net_device *ndev, unsigned int alloced){return 0;}
static inline int bsp_pfa_get_reserve_port_id(enum pfa_reserve_port_vir_id id){return 0;}
static inline int bsp_pfa_update_bypass_portno(unsigned int src_portno, unsigned int dst_portno){return 0;}
#endif /*CONFIG_PFA*/

#if defined(CONFIG_PFA_DIRECT_FW)

void bsp_pfa_add_direct_fw_entry(struct sk_buff *skb, struct pfa_direct_fw_entry_ctx *ctx,
    unsigned int portno);
void bsp_pfa_del_direct_fw_dev(unsigned int devid);
int bsp_pfa_enable_macfw_to_ip(unsigned int vir_portno, unsigned int enable);

#else

static inline void bsp_pfa_add_direct_fw_entry(struct sk_buff *skb,  struct pfa_direct_fw_entry_ctx *ctx,
    unsigned int portno)
{return;}
static inline void bsp_pfa_del_direct_fw_dev(unsigned int devid){return;}
static inline int bsp_pfa_enable_macfw_to_ip(unsigned int vir_portno, unsigned int enable){return 0;}

#endif


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /*__PFA_INTERFACE_H__*/

