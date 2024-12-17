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

#include <linux/neighbour.h>
#include <linux/skbuff.h>
#include <linux/rtnetlink.h>
#include <linux/route.h>
#include <linux/dmapool.h>
#include <linux/etherdevice.h>
#include <linux/types.h>
#include <product_config.h>

#ifndef __PFA_REG_H__
#define __PFA_REG_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define PFA_PORT_NUM 8
#define PFA_CCORE_PORT_BEGIN 7 // the last port is ack, reserve
enum pfa_rsv_port_id {
    PFA_ACORE_LAN_CTRL_ID = 0xff,
    PFA_CCORE_PORT_NO_RSV_MAX
};
#define PFA_PORT_MAP_DEFAULT 0x3

#define PFA_DUMP_SIZE (1024 * 3) /* pfa Application 3k sieze */

#define PFA_DM_BASE 0x0 // stub

#define PFA_EVT_RD_FULL (0x405)
#define PFA_DPORT_ADQ_AP_SEL 0

/* top reg base offset 0xc000 */
#define PFA_TOP_BASE 0xC000
#define PFA_CLK_GATE        (PFA_TOP_BASE + 0x8) /* 时钟门控 */
#define PFA_TOP_TAG_SEL_REG        (PFA_TOP_BASE + 0x100) /* vlan_tpid选择信号 */
#define PFA_TOP_SVLAN_TPID0_REG    (PFA_TOP_BASE + 0x104) /* 单vlan预留的识别寄存器 */
#define PFA_TOP_DVLAN_TPID0_REG    (PFA_TOP_BASE + 0x108) /* 双vlan预留Stag和Ctag识别寄存器 */
#define PFA_TOP_SVLAN_TPID1_REG    (PFA_TOP_BASE + 0x10C) /* 单vlan预留的识别寄存器 */
#define PFA_TOP_DVLAN_TPID1_REG    (PFA_TOP_BASE + 0x110) /* 双vlan预留Stag和Ctag识别寄存器 */
#define PFA_TOP_SVLAN_TPID2_REG    (PFA_TOP_BASE + 0x114) /* 单vlan预留的识别寄存器 */
#define PFA_TOP_DVLAN_TPID2_REG    (PFA_TOP_BASE + 0x118) /* 双vlan预留Stag和Ctag识别寄存器 */
#define PFA_TOP_SVLAN_TPID3_REG    (PFA_TOP_BASE + 0x11C) /* 单vlan预留的识别寄存器 */
#define PFA_TOP_DVLAN_TPID3_REG    (PFA_TOP_BASE + 0x120) /* 双vlan预留Stag和Ctag识别寄存器 */
#define PFA_TOP_SVLAN_TPID_REG(x)  (PFA_TOP_SVLAN_TPID0_REG + 0x4 * (x))

#define PFA_EN 0x0000
#define PFA_DBGEN 0x0004
#define PFA_VER 0x0008
#define PFA_IDLE 0x000C
#define PFA_L4_PORTNUM_CFG(x) (0x0010 + 0x4 * (x))
#define PFA_TAB_CTRL 0x050
#define PFA_TAB_ACT_RESULT 0x054
#define PFA_TAB_CONTENT(x) (0x058 + 0x4 * (x))
#define PFA_TAB_CHK_MODE 0x0100
#define PFA_TAB_CLR 0x0104
#define PFA_HASH_VALUE 0x190
#define PFA_CNT_CLR 0x194
#define PFA_ADQ0_BUF_FLUSH 0x3a0
#define PFA_ADQ0_EN 0x3a4
#define PFA_STICK_MODE 0x03a8
#define PFA_BLACK_WHITE 0x40C
#define PFA_PORT_LMTTIME 0x410
#define PFA_ETH_MINLEN 0x0414
#define PFA_ETH_MAXLEN 0x0418
#define PFA_AGING_TIME 0x041c
#define PFA_GLB_DMAC_ADDR_L 0x0420
#define PFA_GLB_DMAC_ADDR_H 0x0424
#define PFA_GLB_SMAC_ADDR_L 0x0428
#define PFA_GLB_SMAC_ADDR_H 0x042C
#define PFA_TIME 0x00430
#define CLKEN_SEL 0x43C
#define PFA_TD_EMPTY_SEL 0x444
#define PFA_ACK_IDEN_PORT 0x448
#define PFA_ACK_IDEN_EN 0x44c
#define PFA_GMAC_PADDING_IDEN_EN 0x0450

#define PFA_ETH_TYPE_SEL0_REG  0x45C /* 特殊类型包上报CPU */
#define PFA_ETH_TYPE_SEL1_REG  0x460 /* 特殊类型包上报CPU */
#define PFA_ETH_TYPE_SEL2_REG  0x464 /* 特殊类型包上报CPU */
#define PFA_ETH_TYPE_SEL3_REG  0x468 /* 特殊类型包上报CPU */
#define PFA_ETH_TYPE_SEL4_REG  0x46C /* 特殊类型包上报CPU */
#define PFA_ETH_TYPE_SEL5_REG  0x470 /* 特殊类型包上报CPU */
#define PFA_ETH_TYPE_SEL6_REG  0x474 /* 特殊类型包上报CPU */
#define PFA_ETH_TYPE_SEL7_REG  0x478 /* 特殊类型包上报CPU */
#define PFA_RDQ_FULL_INTA_STATUS 0x500
#define PFA_RD_DONE_INTA_STATUS 0x0504
#define PFA_TD_DONE_INTA_STATUS 0x0508
#define PFA_ADQ_EMPTY_INTA_STATUS 0x050C
#define PFA_RDQ_FULL_LEVEL 0x0510
#define PFA_ADQ_EMPTY_LEVEL 0x0514
#define PFA_RDQ_FULL_INTA_MASK 0x0518
#define PFA_RD_DONE_INTA_MASK 0x051C
#define PFA_TD_DONE_INTA_MASK 0x0520
#define PFA_ADQ_EMPTY_INTA_MASK 0x0524
#define PFA_RDQ_FULL_INTA 0x0528
#define PFA_RD_DONE_INTA 0x052C
#define PFA_TD_DONE_INTA 0x0530
#define PFA_ADQ_EMPTY_INTA 0x0534
#define PFA_INTA_INTERVAL 0x538
#define PFA_BUS_ERR_INTA_STATUS 0x540
#define PFA_BUS_ERR_INTA_MASK 0x544
#define PFA_HASH_BADDR_L 0x057C
#define PFA_HASH_BADDR_H 0x0580
#define PFA_HASH_ZONE 0x0584
#define PFA_HASH_RAND 0x0588
#define PFA_HASH_L3_PROTO 0x058C
#define PFA_HASH_WIDTH 0x0590
#define PFA_HASH_DEPTH 0x0594
#define PFA_PACK_MAX_TIME 0x0600
#define PFA_PACK_CTRL 0x0604
#define PFA_PACK_ADDR_CTRL 0x0608
#define PFA_PACK_REMAIN_LEN 0x060C
#define PFA_UNPACK_CTRL 0x0610
#define PFA_UNPACK_MAX_LEN 0x0614
#define PFA_PACK_REGION_DIV 0x0618

#define PFA_ADQ0_BADDR_L 0x700
#define PFA_ADQ0_BADDR_H 0x704
#define PFA_ADQ0_SIZE 0x708
#define PFA_ADQ0_WPTR 0x70c
#define PFA_ADQ0_RPTR 0x710
#define PFA_ADQ0_RPTR_UPDATE_ADDR_L 0x714
#define PFA_ADQ0_RPTR_UPDATE_ADDR_H 0x718

#define PFA_TDQX_BADDR_L(x) (0x0800 + 0x40 * (x))
#define PFA_TDQX_BADDR_H(x) (0x0804 + 0x40 * (x))
#define PFA_TDQX_LEN(x) (0x0808 + 0x40 * (x))
#define PFA_TDQX_WPTR(x) (0x080c + 0x40 * (x))
#define PFA_TDQX_RPTR(x) (0x0810 + 0x40 * (x))
#define PFA_TDQX_DEPTH(x) (0x0814 + 0x40 * (x))

#define PFA_RDQX_BADDR_L(x) (0x0818 + 0x40 * (x))
#define PFA_RDQX_BADDR_H(x) (0x081c + 0x40 * (x))
#define PFA_RDQX_LEN(x) (0x0820 + 0x40 * (x))
#define PFA_RDQX_WPTR(x) (0x0824 + 0x40 * (x))
#define PFA_RDQX_RPTR(x) (0x0828 + 0x40 * (x))
#define PFA_RDQX_DEPTH(x) (0x082c + 0x40 * (x))
#define PFA_TDQX_PRI(x) (0x0830 + 0x40 * (x))
#define PFA_PORTX_PROPERTY_ENC_TYPE(x) (0x083c + 0x40 * (x)) // V100 is stub

#define PFA_PORTX_UDP_LMTNUM(x) (0x0A00 + 0x20 * (x))
#define PFA_PORTX_LMTBYTE(x) (0x0A04 + 0x20 * (x))
#define PFA_PORTX_PROPERTY(x) (0x0A08 + 0x20 * (x))
#define PFA_PORTX_MAC_ADDR_L(x) (0x0A0C + 0x20 * (x))
#define PFA_PORTX_MAC_ADDR_H(x) (0x0A10 + 0x20 * (x))

#define PFA_WRR_DEBUG 0x0c00
#define PFA_UNPACK_DEBUG 0x0c10
#define PFA_ENG_DEBUG 0x0c20
#define PFA_PACK_DEBUG 0x0c30
#define PFA_DONE_DEBUG 0x0c40
#define PFA_LP_RD_BLOCK_CNT 0x0c80
#define PFA_HP_RD_BLOCK_CNT 0x0c84
#define PFA_PACK_OVERTIME_CNT 0x0c88
#define PFA_PACK_PUSH_CNT 0x0c8C
#define PFA_DEPACK_ERR_CNT 0x0c90
#define PFA_ROUTE_INFO 0x0c9C
#define PFA_TDQ_STATUS 0x0CA0
#define PFA_RDQ_STATUS 0x0CA4
#define PFA_ADQ_STATUS 0x0cA8
#define PFA_TFT_UL_CNT 0x0cac
#define PFA_TFT_DL_CNT 0x0cb0

#define PFA_RDQ_FULL_INTC_MASK 0xf04
#define PFA_RD_DONE_INTC_MASK 0xf08
#define PFA_TD_DONE_INTC_MASK 0xf0c
#define PFA_ADQ_EMPTY_INTC_MASK 0xf10
#define PFA_BUS_ERR_INTC_MASK 0xf14
#define PFA_BUS_ERR_INTC 0xf18
#define PFA_RDQ_FULL_INTC_STATUS 0xf1c
#define PFA_RD_DONE_INTC_STATUS 0x0f20
#define PFA_TD_DONE_INTC_STATUS 0x0f24
#define PFA_ADQ_EMPTY_INTC_STATUS 0x0f28
#define PFA_BUS_ERR_INTC_STATUS 0x0f2c
#define PFA_INTC_INTERVAL 0xf30
#define PFA_RDQ_FULL_INTC 0xf34
#define PFA_RD_DONE_INTC 0xf38
#define PFA_TD_DONE_INTC 0xf3C
#define PFA_ADQ_EMPTY_INTC 0xf40

#define PFA_TIME_MAX 0xFFFF
#define PFA_IDLE_DEF_VAL 0x1FFFF

#define PFA_TAB_CTRL_RM_HCACHE 0x8

#define PFA_RDQ_STATUS_EMPYT 0xFFFF0000

#ifdef CONFIG_PFA_FW
#define PFA_IPFW_HTABLE_SIZE (64)
#else
#define PFA_IPFW_HTABLE_SIZE (1)
#endif

#define PFA_MAC_TAB_NUMBER (64)
#define PFA_MAC_TAB_WORD_NO (4)
#define PFA_IPV4_TAB_NUMBER (8)
#define PFA_IPV4_TAB_WORD_NO (6)
#define PFA_IPV6_TAB_NUMBER (4)
#define PFA_IPV6_TAB_WORD_NO (12)
#define MAX_L4_PORTNUM_CFG_NUM (16)
#define PFA_QOS_MAC_TAB_NUMBER (32)
#define PFA_QOS_MAC_TAB_WORD_NO (3)
#define PFA_MAC_PDU_TAB_NUMBER (32)
#define PFA_MAC_PDU_TAB_WORD_NO (5)
#define PFA_IP_QOS_TAB_NUMBER (12)
#define PFA_IP_QOS_TAB_WORD_NO (14)

#define PFA_TAB_ENTRY_INTERVAL (4)
#define PFA_MAC_ENTRY_NUM (256)    /* mac and mac filter entry total 64, every entry 4 word */
#define PFA_IPV4_ENTRY_NUM (48)    /* ipv4 filter entry total 8, every entry 6 word */
#define PFA_IPV6_ENTRY_NUM (48)    /* ipv6 filter entry total 4, every entry 12 word */
#define PFA_IP_ENTRY_RESERVE_NUM (96)
#define PFA_QOS_MAC_ENTRY_NUM (96) /* qos mac entry total 32, every entry 3 word */
#define PFA_MAC_PDU_ENTRY_NUM (160) /* pdu mac entry total 32, every entry 5 word */
#define PFA_IP_QOS_ENTRY_NUM (168) /* pdu mac entry total 12, every entry 14 word */
#define PFA_MAC_REG_ENTRY_NUM (0)

#define PFA_TAB 0x1000
#define PFA_MAC_TAB(x) (PFA_TAB + PFA_TAB_ENTRY_INTERVAL * (x))
#define PFA_IPV4_TAB(x) (PFA_MAC_TAB(PFA_MAC_ENTRY_NUM) + PFA_TAB_ENTRY_INTERVAL * (x))
#define PFA_IPV6_TAB(x) (PFA_IPV4_TAB(PFA_IPV4_ENTRY_NUM) + PFA_TAB_ENTRY_INTERVAL * (x))
#define PFA_QOS_MAC_TAB(x) (PFA_IPV6_TAB(PFA_IPV6_ENTRY_NUM + PFA_IP_ENTRY_RESERVE_NUM) + PFA_TAB_ENTRY_INTERVAL * (x))
#define PFA_MAC_PDU_TAB(x) (PFA_QOS_MAC_TAB(PFA_QOS_MAC_ENTRY_NUM) + PFA_TAB_ENTRY_INTERVAL * (x))
#define PFA_IP_QOS_TAB(x) (PFA_MAC_PDU_TAB(PFA_MAC_PDU_ENTRY_NUM) + PFA_TAB_ENTRY_INTERVAL * (x))

#define PFA_RESET_BAK_REG_NUM 5
#define PFA_STASH_REG_BUTTON 2
extern unsigned int g_pfa_reset_bak_reg[][PFA_STASH_REG_BUTTON];

#define PFA_REG_DUMP_GROUP_NUM 6

enum pfa_tab_ctrl_type {
    PFA_TAB_CTRL_ADD_MAC_FILT = 0,
    PFA_TAB_CTRL_ADD_MAC_FW,
    PFA_TAB_CTRL_ADD_IPV4_FILT,
    PFA_TAB_CTRL_ADD_IPV6_FILT,
    PFA_TAB_CTRL_DEL_MAC_FILT,
    PFA_TAB_CTRL_DEL_MAC_FW,
    PFA_TAB_CTRL_DEL_IPV4_FILT,
    PFA_TAB_CTRL_DEL_IPV6_FILT,
    PFA_TAB_CTRL_DEL_HASH_CACHE_ENTRY,
    PFA_TAB_CTRL_ADD_QOS_MAC_LMT,
    PFA_TAB_CTRL_DEL_QOS_MAC_LMT,
    PFA_TAB_CTRL_ADD_MAC_PDU,
    PFA_TAB_CTRL_DEL_MAC_PDU,
    PFA_TAB_CTRL_ADD_IP_QOS,
    PFA_TAB_CTRL_DEL_IP_QOS,
    PFA_TAB_CTRL_BOTTOM
};

typedef union {
    struct {
        unsigned int stick_en : 1;
        unsigned int bypass_en : 1;
        unsigned int bypass_dport : 4;
        unsigned int reserved : 26;
    } bits;
    unsigned int u32;
} pfa_stick_mode_t;

typedef union {
    struct {
        unsigned int qos_limit_period : 20;
        unsigned int qos_limit_en : 1;
        unsigned int reserved : 11;
    } bits;
    unsigned int u32;
} pfa_qos_lmttime_t;

#define PFA_GT_CLK 0x470
#define PFA_GT_CLK_STAT 0x478
#define PFA_GT_CLK_FINIAL_STAT 0x47c
#define PFA_GT_CLK_MASK 0x38000  // bit 15 16 17

#define PFA_CLK_DIV_THR_RATE 900  // max rate while clk_div = 16
#define PFA_CLK_DIV_FULL_MIN_LEVEL 0x0
#define PFA_CLK_DIV_FULL_RATE_LEVEL 0x1

#ifdef CONFIG_PFA_PHONE_SOC
#define PFA_CLK_DIV 0x710
#define PFA_CLK_DIV_DEFAULT 0xF00000

typedef union {
    struct {
        unsigned int reserved_div_0 : 4;
        unsigned int pfa_div : 4;
        unsigned int reserved_div_1 : 8;
        unsigned int reserved_en_0 : 4;
        unsigned int pfa_en : 4;
        unsigned int reserved_2 : 8;
    } bits;
    unsigned int u32;
} pfa_div_ctrl_t;
#else
#define PFA_CLK_DIV 0x18C
#define PFA_CLK_DIV_DEFAULT 0x8F00A032
typedef union {
    struct {
        unsigned int pfa_tft_div : 4;
        unsigned int seceng_div : 4;
        unsigned int pfa_div : 4;
        unsigned int pfa_tft_refclk_sel : 1;
        unsigned int seceng_refclk_sel : 2;
        unsigned int pfa_refclk_sel : 1;
        unsigned int bitmasken : 16;
    } bits;
    unsigned int u32;
} pfa_div_ctrl_t;
#endif



enum pfa_tab_list_type {
    PFA_TAB_LIST_MAC_FILT = 0,
    PFA_TAB_LIST_IPV4_FILT,
    PFA_TAB_LIST_IPV6_FILT,
    PFA_TAB_LIST_BOTTOM
};

enum pfa_brg_type {
    PFA_BRG_BRIF,
    PFA_BRG_NORMAL,
    PFA_BRG_MIX,
    PFA_BRG_BR,
    PFA_BRG_BOTTOM
};

typedef union {
    struct {
        unsigned int pfa_en : 1;
        unsigned int reserved : 31;
    } bits;
    unsigned int u32;
} pfa_en_t;

typedef union {
    struct {
        unsigned int pfa_mac_black_white : 1;
        unsigned int pfa_ip_black_white : 1;
        unsigned int reserved : 30;
    } bits;
    unsigned int u32;
} pfa_black_white_t;

typedef union {
    struct {
        unsigned int    one_vlan_tag_sel      : 1   ; /* [0] */
        unsigned int    reserved_0            : 31  ; /* [31..1] */
    } bits;
    unsigned int    u32;
} eth_vlan_tag_sel;

typedef union {
    struct {
        unsigned int pfa_port_attach_brg : 2;
        unsigned int pfa_port_enc_type : 4;
        unsigned int pfa_port_pad_en : 1;
        unsigned int mac_ip_fw_flag        : 1   ; /* [7] */
        unsigned int pfa_dport_adq_core_type : 1   ; /* [8] */
        unsigned int pfa_port_pass_mode    : 1   ; /* [9] */
        unsigned int pfa_ppp_mac_hdr       : 1   ; /* [10] */
        unsigned int pfa_ppp_hdr_user      : 1   ; /* [11] */
        unsigned int reserved_0            : 19  ; /* [30..12] */
        unsigned int pfa_port_en : 1;
    } bits;
    unsigned int u32;
} pfa_port_prop_t;

typedef union {
    struct {
        unsigned int pfa_dbgen : 1;
        unsigned int reserved : 31;
    } bits;
    unsigned int u32;
} pfa_dbgen_t;

typedef union {
    struct {
        unsigned int pfa_tab_done : 1;
        unsigned int pfa_tab_success : 1;
        unsigned int reserved : 30;
    } bits;
    unsigned int u32;
} pfa_tab_act_result_t;

typedef union {
    struct {
        unsigned int pfa_ip_aging_time : 16;
        unsigned int pfa_mac_aging_time : 16;
    } bits;
    unsigned int u32;
} pfa_ageing_timer_t;

typedef union {
    struct {
        unsigned int pfa_pack_max_pkt_cnt : 7;
        unsigned int reserved_1 : 1;
        unsigned int pfa_pack_max_len : 17;
        unsigned int reserved_0 : 7;
    } bits;
    unsigned int u32;
} pfa_pack_ctrl_t;
typedef union {
    struct {
        unsigned int pfa_ncm_max_len : 16;
        unsigned int pfa_rndis_max_len : 16;
    } bits;
    unsigned int u32;
} pfa_unpack_max_len_t;

typedef union {
    struct {
        unsigned int pfa_tab_clr : 5;
        unsigned int reserved : 27;
    } bits;
    unsigned int u32;
} pfa_tab_clt_t;

typedef union {
    struct {
        unsigned int reserved0 : 8;
        unsigned int pfa_port_lmttime : 8;
        unsigned int pfa_port_lmt_en : 8;
        unsigned int reserved1 : 8;
    } bits;
    unsigned int u32;
} pfa_port_lmttime_t;

typedef union {
    struct {
        unsigned int pfa_l4_portnum_cfg : 18;
        unsigned int reserved : 14;
    } bits;
    unsigned int u32;
} pfa_l4_cfg_t;

static inline unsigned int pfa_readl(const void __iomem *base, unsigned int offset)
{
    return readl(base + offset);
}

static inline unsigned int pfa_readl_relaxed(const void __iomem *base, unsigned int offset)
{
    return readl_relaxed(base + offset);
}

static inline void pfa_writel(void __iomem *base, unsigned int offset, unsigned int value)
{
    writel(value, base + offset);
}

static inline void pfa_writel_relaxed(void __iomem *base, unsigned int offset, unsigned int value)
{
    writel_relaxed(value, base + offset);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __PFA_REG_H__ */
