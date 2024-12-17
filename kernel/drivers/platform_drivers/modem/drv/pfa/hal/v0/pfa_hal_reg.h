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

#define PFA_DUMP_SIZE (1024 * 3) /* pfa Application 3k sieze */

#define PFA_DM_BASE 0x0 // stub

#define PFA_EVT_RD_FULL (0x405)
#define PFA_DPORT_ADQ_AP_SEL 0

#define PFA_DBGEN 0x0004
#define RESERVED 0x001C
#define PFA_TIME 0x0030
#define PFA_PFA_TFT_BD_RD_IP_HEAD 0x0034
#define PFA_QOS_LMTTIME 0x00B0
#define PFA_TD_EMPTY_SEL 0x00B4

#define PFA_STICK_MODE 0x061c
#define PFA_PFA_TFT_UL_CNT 0x07AC
#define PFA_PFA_TFT_DL_CNT 0x07B0

#define PFA_PORT_NUM 16
#define PFA_CCORE_PORT_BEGIN 15 // the last port is ack, reserve
enum pfa_rsv_port_id {
    PFA_ACORE_LAN_CTRL_ID = 3,
    PFA_CCORE_PORT_NO_RSV_MAX
};
#define PFA_PORT_MAP_DEFAULT 0xb

#define PFA_RESET_BAK_REG_NUM 16
#define PFA_STASH_REG_BUTTON 2
extern unsigned int g_pfa_reset_bak_reg[][PFA_STASH_REG_BUTTON];

#define PFA_REG_DUMP_GROUP_NUM 9

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

#define CLKEN_SEL 0x00AC

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

#ifdef CONFIG_PFA_FW
#define PFA_IPFW_HTABLE_SIZE (512)
#else
#define PFA_IPFW_HTABLE_SIZE (1)
#endif

#define PFA_MAC_TAB_NUMBER (128)
#define PFA_MAC_TAB_WORD_NO (4)
#define PFA_IPV4_TAB_NUMBER (16)
#define PFA_IPV4_TAB_WORD_NO (6)
#define PFA_IPV6_TAB_NUMBER (8)
#define PFA_IPV6_TAB_WORD_NO (12)
#define MAX_L4_PORTNUM_CFG_NUM (16)
#define PFA_QOS_MAC_TAB_NUMBER (32)
#define PFA_QOS_MAC_TAB_WORD_NO (3)

#define PFA_REG_BASE (0xE5004000)
#define PFA_REG_SIZE (0x4000)
#define PFA_IRQ (215)

#define PFA_EN 0x0000
#define PFA_BAND 0x0008
#define PFA_BLACK_WHITE 0x000C
#define PFA_PORT_LMTTIME 0x0010
#define PFA_ETH_MINLEN 0x0014
#define PFA_ETH_MAXLEN 0x0018
#define PFA_GLB_DMAC_ADDR_L 0x0020
#define PFA_GLB_DMAC_ADDR_H 0x0024
#define PFA_GLB_SMAC_ADDR_L 0x0028
#define PFA_GLB_SMAC_ADDR_H 0x002C
#define PFA_L4_PORTNUM_CFG(x) (0x0040 + 0x4 * (x))
#define PFA_CNT_CLR 0x0080
#define PFA_VER 0x009C
#define PFA_IDLE 0x00A4
#define PFA_MAC_IP_FW_FLAG 0x00A8

#define PFA_TIME_MAX 0xFFFF
#define PFA_IDLE_DEF_VAL 0x1FFFF

#define PFA_OST_CTRL 0x0100
#define PFA_BURST_CTRL 0x0104
#define PFA_WCH_ID 0x0108
#define PFA_RCH_ID 0x010C
#define PFA_WCH_PRI 0x0110
#define PFA_RCH_PRI 0x0114
#define PFA_SC_REG0 0x11C
#define PFA_SC_REG1 0x120
#define PFA_SC_REG(x) (PFA_SC_REG0 + 0x4 * (x))

#define PFA_RDQ_FULL_INTA_STATUS 0x0200
#define PFA_RD_DONE_INTA_STATUS 0x0204
#define PFA_TD_DONE_INTA_STATUS 0x0208
#define PFA_ADQ_EMPTY_INTA_STATUS 0x020C
#define PFA_RDQ_FULL_LEVEL 0x0210
#define PFA_ADQ_EMPTY_LEVEL 0x0214
#define PFA_RDQ_FULL_INTA_MASK 0x0218
#define PFA_RD_DONE_INTA_MASK 0x021C
#define PFA_TD_DONE_INTA_MASK 0x0220
#define PFA_ADQ_EMPTY_INTA_MASK 0x0224
#define PFA_RDQ_FULL_INTC_MASK 0x0228
#define PFA_RD_DONE_INTC_MASK 0x022C
#define PFA_TD_DONE_INTC_MASK 0x0230
#define PFA_ADQ_EMPTY_INTC_MASK 0x0234
#define PFA_RDQ_FULL_INTA 0x0238
#define PFA_RD_DONE_INTA 0x023C
#define PFA_TD_DONE_INTA 0x0240
#define PFA_ADQ_EMPTY_INTA 0x0244
#define PFA_RDQ_FULL_INTC 0x0248
#define PFA_RD_DONE_INTC 0x024C
#define PFA_TD_DONE_INTC 0x0250
#define PFA_ADQ_EMPTY_INTC 0x0254
#define PFA_INTA_INTERVAL 0x0258
#define PFA_INTC_INTERVAL 0x025C
#define PFA_BUS_ERR_INTA_STATUS 0x0260
#define PFA_BUS_ERR_INTA_MASK 0x0264
#define PFA_BUS_ERR_INTC_MASK 0x0268
#define PFA_BUS_ERR_INTA 0x026C
#define PFA_BUS_ERR_INTC 0x0270
#define HASH_CACHE_VALX(x) (0x0300 + 0x4 * (x))

#define PFA_TAB_CTRL 0x0500

#define PFA_TAB_CTRL_RM_HCACHE 0x8

#define PFA_TAB_ACT_RESULT 0x0504
#define PFA_TAB_CONTENT(x) (0x0508 + 0x4 * (x))
#define PFA_HASH_BADDR_L 0x057C
#define PFA_HASH_BADDR_H 0x0580
#define PFA_HASH_ZONE 0x0584
#define PFA_HASH_RAND 0x0588
#define PFA_HASH_L3_PROTO 0x058C
#define PFA_HASH_WIDTH 0x0590
#define PFA_HASH_DEPTH 0x0594
#define PFA_AGING_TIME 0x05A0
#define PFA_TAB_CHK_MODE 0x05A4
#define PFA_TAB_CLR 0x05A8
#define PFA_HASH_VALUE 0x05B0
#define PFA_ACK_IDEN_PORT 0x05B4
#define PFA_ACK_IDEN_EN 0x05B8
#define PFA_GMAC_PADDING_IDEN_EN 0x05BC
#define PFA_HASH_CACHE_EN 0x05C0

#define PFA_PACK_MAX_TIME 0x0600
#define PFA_PACK_CTRL 0x0604
#define PFA_PACK_ADDR_CTRL 0x0608
#define PFA_PACK_REMAIN_LEN 0x060C
#define PFA_UNPACK_CTRL 0x0610
#define PFA_UNPACK_MAX_LEN 0x0614
#define PFA_PACK_REGION_DIV 0x0618
#define PFA_DEBUG_WORD 0x0620

#define PFA_WRR_DEBUG 0x0700
#define PFA_UNPACK_DEBUG 0x0710
#define PFA_ENG_DEBUG 0x0720
#define PFA_PACK_DEBUG 0x0730
#define PFA_DONE_DEBUG 0x0740
#define HAC_GIF_DEBUG 0x0750
#define PFA_LP_RD_BLOCK_CNT 0x0780
#define PFA_HP_RD_BLOCK_CNT 0x0784
#define PFA_PACK_OVERTIME_CNT 0x0788
#define PFA_PACK_PUSH_CNT 0x078C
#define PFA_DEPACK_ERR_CNT 0x0790
#define PFA_LP_RD_BLOCK_CNT0 0x0794
#define PFA_HP_RD_BLOCK_CNT1 0x0798
#define PFA_ROUTE_INFO 0x079C
#define PFA_TDQ_STATUS 0x07A0
#define PFA_RDQ_STATUS 0x07A4
#define PFA_ADQ_STATUS 0x07A8

#define PFA_RDQ_STATUS_EMPYT 0xFFFF0000

#define PFA_TDQX_BADDR_L(x) (0x2800 + 0x40 * (x))
#define PFA_TDQX_BADDR_H(x) (0x2804 + 0x40 * (x))
#define PFA_TDQX_LEN(x) (0x2808 + 0x40 * (x))
#define PFA_TDQX_WPTR(x) (0x280C + 0x40 * (x))
#define PFA_TDQX_RPTR(x) (0x2810 + 0x40 * (x))
#define PFA_TDQX_DEPTH(x) (0x2814 + 0x40 * (x))
#define PFA_RDQX_BADDR_L(x) (0x2818 + 0x40 * (x))
#define PFA_RDQX_BADDR_H(x) (0x281C + 0x40 * (x))
#define PFA_RDQX_LEN(x) (0x2820 + 0x40 * (x))
#define PFA_RDQX_WPTR(x) (0x2824 + 0x40 * (x))
#define PFA_RDQX_RPTR(x) (0x2828 + 0x40 * (x))
#define PFA_RDQX_DEPTH(x) (0x282C + 0x40 * (x))
#define PFA_TDQX_PRI(x) (0x2830 + 0x40 * (x))
#define PFA_PORTX_PROPERTY_ENC_TYPE(x) (0x283c + 0x40 * (x)) // V0 is stub

#define PFA_PFA_TFT_RDQ_WPTR_UPDATE_ADDR_L 0x2C00
#define PFA_PFA_TFT_RDQ_WPTR_UPDATE_ADDR_H 0x2C04
#define PFA_TFT_PFA_BDQ_RPTR_UPDATE_ADDR_L 0x2C08
#define PFA_TFT_PFA_BDQ_RPTR_UPDATE_ADDR_H 0x2C0C

#define PFA_PORTX_UDP_LMTNUM(x) (0x2D00 + 0x30 * (x))
#define PFA_PORTX_LMTBYTE(x) (0x2D04 + 0x30 * (x))
#define PFA_PORTX_PROPERTY(x) (0x2D08 + 0x30 * (x))
#define PFA_PORTX_MAC_ADDR_L(x) (0x2D0C + 0x30 * (x))
#define PFA_PORTX_MAC_ADDR_H(x) (0x2D10 + 0x30 * (x))
#define PFA_PORTX_IPV6_ADDR_1ST(x) (0x2D14 + 0x30 * (x))
#define PFA_PORTX_IPV6_ADDR_2ND(x) (0x2D18 + 0x30 * (x))
#define PFA_PORTX_IPV6_ADDR_3TH(x) (0x2D1C + 0x30 * (x))
#define PFA_PORTX_IPV6_ADDR_4TH(x) (0x2D20 + 0x30 * (x))
#define PFA_PORTX_IPV6_MASK(x) (0x2D24 + 0x30 * (x))
#define PFA_PORTX_IPV4_ADDR(x) (0x2D28 + 0x30 * (x))
#define PFA_PORTX_IPV4_MASK(x) (0x2D2C + 0x30 * (x))

#define PFA_ADQ0_BADDR_L (0x3000)
#define PFA_ADQ0_BADDR_H (0x3004)
#define PFA_ADQ0_SIZE (0x3018)
#define PFA_ADQ0_WPTR (0x3030)
#define PFA_ADQ0_RPTR (0x3034)
#define PFA_ADQ0_LEN (0x3048)
#define PFA_ADQ0_RPTR_UPDATE_ADDR_L (0x3054)
#define PFA_ADQ0_RPTR_UPDATE_ADDR_H (0x3058)
#define PFA_ADQ0_BUF_FLUSH (0x3070)
#define PFA_ADQ0_EN (0x3078)
#define PFA_ADQ_THRESHOLD 0x306C
#define PFA_ADQ1_LEN (0x304c)

#define PFA_TAB_ENTRY_INTERVAL (4)
#define PFA_MAC_ENTRY_NUM (512)    /* mac and mac filter entry total 128, every entry 4 word */
#define PFA_IPV4_ENTRY_NUM (96)    /* ipv4 filter entry total 16, every entry 6 word */
#define PFA_IPV6_ENTRY_NUM (96)    /* ipv6 filter entry total 8, every entry 12 word */
#define PFA_QOS_MAC_ENTRY_NUM (96) /* qos mac entry total 32, every entry 3 word */
#define PFA_MAC_PDU_ENTRY_NUM (0)
#define PFA_MAC_REG_ENTRY_NUM (0)

#define PFA_TAB 0x800
#define PFA_MAC_TAB(x) (PFA_TAB + PFA_TAB_ENTRY_INTERVAL * (x))
#define PFA_IPV4_TAB(x) (PFA_MAC_TAB(PFA_MAC_ENTRY_NUM) + PFA_TAB_ENTRY_INTERVAL * (x))
#define PFA_IPV6_TAB(x) (PFA_IPV4_TAB(PFA_IPV4_ENTRY_NUM) + PFA_TAB_ENTRY_INTERVAL * (x))
#define PFA_QOS_MAC_TAB(x) (PFA_IPV6_TAB(PFA_IPV6_ENTRY_NUM) + PFA_TAB_ENTRY_INTERVAL * (x))

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
        unsigned int pfa_port_attach_brg : 2;
        unsigned int pfa_port_enc_type : 4;
        unsigned int pfa_port_pad_en : 1;
        unsigned int pfa_dport_adq_core_type : 1; // stub
        unsigned int reserved : 23;
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
        unsigned int pfa_port_udp_lmttime : 8;
        unsigned int pfa_port_lmttime : 8;
        unsigned int pfa_port_lmt_en : 16;
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
