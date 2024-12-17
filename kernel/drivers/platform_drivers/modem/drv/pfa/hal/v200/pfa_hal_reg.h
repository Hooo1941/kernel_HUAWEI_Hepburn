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
#ifndef PFA_HAL_REG_H
#define PFA_HAL_REG_H

#include <linux/neighbour.h>
#include <linux/skbuff.h>
#include <linux/rtnetlink.h>
#include <linux/route.h>
#include <linux/dmapool.h>
#include <linux/etherdevice.h>
#include <linux/types.h>
#include <product_config.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define PFA_PORT_NUM 16
#define PFA_CCORE_PORT_BEGIN 11
enum pfa_rsv_port_id {
    PFA_ACORE_LAN_CTRL_ID = 0xff,
    PFA_CCORE_PORT_NO_RSV_MAX
};
#define PFA_PORT_MAP_DEFAULT 0x1

#define PFA_EVT_RD_FULL (0x5)
#define PFA_DPORT_ADQ_AP_SEL 1

#define PFA_POFF_CTRL_OFFSET 0xb48
#define PFA_POFF_CTRL_SD_VALUE 0x20000
#define PFA_POFF_CTRL_DS_VALUE 0x10000
#define PFA_DM_BASE 0x0
#define PFA_IMG_BASE_END 0x1ffff

#define PFA_SCTRL_BASE 0x2E000
#define PFA_VER (PFA_SCTRL_BASE + 0x0)
#define PFA_IDLE (PFA_SCTRL_BASE + 0x4)
#define CLKEN_SEL (PFA_SCTRL_BASE + 0x8)
#define PFA_CORE_HALT (PFA_SCTRL_BASE + 0xC)
#define PFA_SUBMODULE_DEBUG (PFA_SCTRL_BASE + 0x20)
#define PFA_SC_REG0 (PFA_SCTRL_BASE + 0x60)
#define PFA_SC_REG1 (PFA_SCTRL_BASE + 0x64)
#define PFA_SC_REG(x) (PFA_SC_REG0 + 0x4 * (x))
#define STACK_START_ADDR (PFA_SCTRL_BASE + 0x84)
#define STACK_END_ADDR (PFA_SCTRL_BASE + 0x88)
#define SP_VALUE (PFA_SCTRL_BASE + 0xAC)
#define PC_VALUE (PFA_SCTRL_BASE + 0xB0)
#define PFA_QSP_RESP_ERR_STAT (PFA_SCTRL_BASE + 0xB4)
#define PFA_ABNORMAL_INT_STAT (PFA_SCTRL_BASE + 0x0100)
#define PFA_SOFT_INT_RAW (PFA_SCTRL_BASE + 0x0118)
#define PFA_ENABLE_BIT 0
#define PFA_DISABLE_BIT 1
#define PFA_ENABLE_MODEM_PORT_BIT 2
#define PFA_DISABLE_MODEM_PORT_BIT 3
#define PFA_ACPU2PFA_SOFT_INT_STAT (PFA_SCTRL_BASE + 0x0130)
#define PFA_ACPU2PFA_SOFT_INT_MASK (PFA_SCTRL_BASE + 0x0134)
#define PFA_ACPU2PFA_SOFT_INT_RAW (PFA_SCTRL_BASE + 0x0138)
#define PFA_ACPU2PFA_SOFT_INT_SET (PFA_SCTRL_BASE + 0x013C)
#define PFA_ACPU2PFA_SOFT_INT_CLR (PFA_SCTRL_BASE + 0x0140)

#define PFA_PORTX_PROPERTY_ENC_TYPE(x) (PFA_SCTRL_BASE + 0x200 + 0x4 * (x))

#define PFA_DM_NMANGER_BASE 0x5000
#define PFA_PACK_REGION_DIV (PFA_DM_NMANGER_BASE + 0x34)
#define PFA_PACK_MAX_PKT_CNT (PFA_DM_NMANGER_BASE + 0x38)
#define PFA_PACK_MAX_LEN (PFA_DM_NMANGER_BASE + 0x3C)
#define PFA_PACK_REMAIN_LEN (PFA_DM_NMANGER_BASE + 0x40)
#define PFA_PACK_MAX_TIME (PFA_DM_NMANGER_BASE + 0x44)
#define PFA_PACK_ADDR_CTRL (PFA_DM_NMANGER_BASE + 0x48)

#define PFA_BDC_SLV_SEC_REG            (PFA_SCTRL_BASE + 0x880) /* BDQ通道寄存器安全属性配置 */
#define PFA_BDC_MST_RD_BD_SEC_REG      (PFA_SCTRL_BASE + 0x884) /* BDC MST口读BD安全属性配置 */
#define PFA_BDC_MST_RD_PKT_SEC_REG     (PFA_SCTRL_BASE + 0x888) /* BDC MST口读ETH安全属性配置 */
#define PFA_BDC_MST_PUSH_SEC_REG       (PFA_SCTRL_BASE + 0x88C) /* BDC MST口推送BD读指针安全属性配置 */
#define PFA_RDC_SLV_SEC_REG            (PFA_SCTRL_BASE + 0x890) /* RDQ通道寄存器安全属性配置 */
#define PFA_RDC_MST_SEC0_REG           (PFA_SCTRL_BASE + 0x894) /* RDC MST口上报RD安全属性配置 */
#define PFA_RDC_MST_SEC1_REG           (PFA_SCTRL_BASE + 0x898) /* RDC MST口推送RD写指针安全属性配置 */
#define PFA_ADC_SLV_MST_SEC_REG        (PFA_SCTRL_BASE + 0x89C) /* ADC安全属性配置 */
#define PFA_ENG_SEC_REG                (PFA_SCTRL_BASE + 0x8a0) /* ENG安全属性配置 */
#define PFA_OAM_MST_SEC_REG            (PFA_SCTRL_BASE + 0x8a4) /* OAM安全属性配置 */


#define PFA_ENG_BASE 0x40000
#define PFA_TIME (PFA_ENG_BASE + 0x08)
#define PFA_L4_PORTNUM_CFG(x) (PFA_ENG_BASE + 0x0010 + 0x4 * (x))
#define PFA_TAB_CTRL (PFA_ENG_BASE + 0x050)
#define PFA_TAB_ACT_RESULT (PFA_ENG_BASE + 0x054)
#define PFA_TAB_CONTENT(x) (PFA_ENG_BASE + 0x058 + 0x4 * (x))
#define PFA_TAB_CHK_MODE (PFA_ENG_BASE + 0x0100)
#define PFA_TAB_CLR (PFA_ENG_BASE + 0x0104)
#define PFA_HASH_VALUE (PFA_ENG_BASE + 0x0110)
#define PFA_BLACK_WHITE (PFA_ENG_BASE + 0x0200)
#define PFA_AGING_TIME (PFA_ENG_BASE + 0x0204)
#define PFA_PORT_LMTTIME (PFA_ENG_BASE + 0x208)
#define PFA_STICK_MODE (PFA_ENG_BASE + 0x20c)
#define PFA_GLB_DMAC_ADDR_L (PFA_ENG_BASE + 0x220)
#define PFA_GLB_DMAC_ADDR_H (PFA_ENG_BASE + 0x224)
#define PFA_GLB_SMAC_ADDR_L (PFA_ENG_BASE + 0x228)
#define PFA_GLB_SMAC_ADDR_H (PFA_ENG_BASE + 0x22c)

#define PFA_EN (PFA_ENG_BASE + 0x0000) // v200 stub

#define PFA_ETH_TYPE_SEL0_REG  (PFA_ENG_BASE + 0x300) /* 特殊类型包上报CPU */
#define PFA_ETH_TYPE_SEL1_REG  (PFA_ENG_BASE + 0x304) /* 特殊类型包上报CPU */
#define PFA_ETH_TYPE_SEL2_REG  (PFA_ENG_BASE + 0x308) /* 特殊类型包上报CPU */
#define PFA_ETH_TYPE_SEL3_REG  (PFA_ENG_BASE + 0x30c) /* 特殊类型包上报CPU */
#define PFA_ETH_TYPE_SEL4_REG  (PFA_ENG_BASE + 0x310) /* 特殊类型包上报CPU */
#define PFA_ETH_TYPE_SEL5_REG  (PFA_ENG_BASE + 0x314) /* 特殊类型包上报CPU */
#define PFA_ETH_TYPE_SEL6_REG  (PFA_ENG_BASE + 0x318) /* 特殊类型包上报CPU */
#define PFA_ETH_TYPE_SEL7_REG  (PFA_ENG_BASE + 0x31c) /* 特殊类型包上报CPU */
#define PFA_ETH_VLAN_TAG_0 (PFA_ENG_BASE + 0x320)
#define PFA_TOP_TAG_SEL_REG        (PFA_ENG_BASE + 0x324) /* vlan_tpid选择信号 */
#define PFA_TOP_SVLAN_TPID0_REG    (PFA_ENG_BASE + 0x328) /* 单vlan预留的识别寄存器 */
#define PFA_TOP_DVLAN_TPID0_REG    (PFA_ENG_BASE + 0x32C) /* 双vlan预留Stag和Ctag识别寄存器 */
#define PFA_TOP_SVLAN_TPID1_REG    (PFA_ENG_BASE + 0x330) /* 单vlan预留的识别寄存器 */
#define PFA_TOP_DVLAN_TPID1_REG    (PFA_ENG_BASE + 0x334) /* 双vlan预留Stag和Ctag识别寄存器 */
#define PFA_TOP_SVLAN_TPID2_REG    (PFA_ENG_BASE + 0x338) /* 单vlan预留的识别寄存器 */
#define PFA_TOP_DVLAN_TPID2_REG    (PFA_ENG_BASE + 0x33C) /* 双vlan预留Stag和Ctag识别寄存器 */
#define PFA_TOP_SVLAN_TPID3_REG    (PFA_ENG_BASE + 0x340) /* 单vlan预留的识别寄存器 */
#define PFA_TOP_DVLAN_TPID3_REG    (PFA_ENG_BASE + 0x344) /* 双vlan预留Stag和Ctag识别寄存器 */
#define PFA_TOP_SVLAN_TPID_REG(x)  (PFA_TOP_SVLAN_TPID0_REG + 0x4 * (x))
#define PFA_GMAC_PADDING_IDEN_EN (PFA_ENG_BASE + 0x348)
#define PFA_HASH_BADDR_L (PFA_ENG_BASE + 0x350)
#define PFA_HASH_BADDR_H (PFA_ENG_BASE + 0x354)
#define PFA_HASH_ZONE (PFA_ENG_BASE + 0x358)
#define PFA_HASH_RAND (PFA_ENG_BASE + 0x35C)
#define PFA_HASH_L3_PROTO (PFA_ENG_BASE + 0x360)
#define PFA_HASH_WIDTH (PFA_ENG_BASE + 0x364)
#define PFA_HASH_DEPTH (PFA_ENG_BASE + 0x368)
#define PFA_HASH_CACHE_EN (PFA_ENG_BASE + 0x36c)
#define PFA_PORTX_UDP_LMTNUM(x) (PFA_ENG_BASE + 0x370 + 0x20 * (x))
#define PFA_PORTX_PROPERTY(x) (PFA_ENG_BASE + 0x374 + 0x20 * (x))
#define PFA_PORTX_MAC_ADDR_L(x) (PFA_ENG_BASE + 0x378 + 0x20 * (x))
#define PFA_PORTX_MAC_ADDR_H(x) (PFA_ENG_BASE + 0x37C + 0x20 * (x))
#define PFA_PORTX_LMTBYTE(x) (PFA_ENG_BASE + 0x38c + 0x20 * (x)) // V200 is stub!

#define PFA_ADC_BASE 0x0002D000
#define PFA_ADQ_EMPTY_LEVEL (PFA_ADC_BASE + 0x0700)
#define PFA_ADQ_BADDR_L(x) (PFA_ADC_BASE + 0x0800 + 0x40 * (x))
#define PFA_ADQ_BADDR_H(x) (PFA_ADC_BASE + 0x0804 + 0x40 * (x))
#define PFA_ADQ_SIZE(x) (PFA_ADC_BASE + 0x0808 + 0x40 * (x))
#define PFA_ADQ_WPTR(x) (PFA_ADC_BASE + 0x080c + 0x40 * (x))
#define PFA_ADQ_RPTR(x) (PFA_ADC_BASE + 0x0810 + 0x40 * (x))
#define PFA_ADQ_RPTR_UPDATE_ADDR_L(x) (PFA_ADC_BASE + 0x0814 + 0x40 * (x))
#define PFA_ADQ_RPTR_UPDATE_ADDR_H(x) (PFA_ADC_BASE + 0x0818 + 0x40 * (x))
#define PFA_ADQ_CTRL(x) (PFA_ADC_BASE + 0x0824 + 0x40 * (x))
#define PFA_ADQ_DEBUG(x) (PFA_ADC_BASE + 0x082c + 0x40 * (x))
#define PFA_ADQ_EN(x) (PFA_ADC_BASE + 0x0838+0x40 * (x))

#define PFA_ADQ0_EN PFA_ADQ_EN(2)
#define PFA_ADQ0_BADDR_L PFA_ADQ_BADDR_L(2)
#define PFA_ADQ0_BADDR_H PFA_ADQ_BADDR_H(2)
#define PFA_ADQ0_SIZE PFA_ADQ_SIZE(2)
#define PFA_ADQ0_WPTR PFA_ADQ_WPTR(2)
#define PFA_ADQ0_RPTR PFA_ADQ_RPTR(2)
#define PFA_ADQ0_RPTR_UPDATE_ADDR_L PFA_ADQ_RPTR_UPDATE_ADDR_L(2)
#define PFA_ADQ0_RPTR_UPDATE_ADDR_H PFA_ADQ_RPTR_UPDATE_ADDR_H(2)

#define PFA_BDC_BASE 0x0002B000
#define PFA_TDQX_BADDR_L(x) (PFA_BDC_BASE + 0x0 + 0x40 * (x))
#define PFA_TDQX_BADDR_H(x) (PFA_BDC_BASE + 0x4 + 0x40 * (x))
#define PFA_TDQX_LEN(x) (PFA_BDC_BASE + 0x8 + 0x40 * (x))
#define PFA_TDQX_WPTR(x) (PFA_BDC_BASE + 0xc + 0x40 * (x))
#define PFA_TDQX_RPTR(x) (PFA_BDC_BASE + 0x10 + 0x40 * (x))
#define PFA_TDQX_LOCAL_RPTR(x) (PFA_BDC_BASE + 0x14 + 0x40 * (x))
#define PFA_TDQX_ONE_BD_SIZE(x) (PFA_BDC_BASE + 0x18 + 0x40 * (x))
#define PFA_TDQX_RPTR_UPDATE_ADDR_L(x) (PFA_BDC_BASE + 0x1c + 0x40 * (x))
#define PFA_TDQX_RPTR_UPDATE_ADDR_H(x) (PFA_BDC_BASE + 0x20 + 0x40 * (x))
#define PFA_TDQX_STATE(x) (PFA_BDC_BASE + 0x24 + 0x40 * (x))
#define PFA_TDQX_CTRL(x) (PFA_BDC_BASE + 0x28 + 0x40 * (x))
#define PFA_TDQX_EN(x) (PFA_BDC_BASE + 0x3c + 0x40 * (x))
#define PFA_UNPACK_CTRL (PFA_BDC_BASE + 0x800)
#define PFA_UNPACK_MAX_LEN (PFA_BDC_BASE + 0x804)
#define PFA_ETH_MINLEN (PFA_BDC_BASE + 0x808)
#define PFA_ETH_MAXLEN (PFA_BDC_BASE + 0x80c)
#define PFA_IP_MINLEN (PFA_BDC_BASE + 0x810)
#define PFA_IP_MAXLEN (PFA_BDC_BASE + 0x814)
#define PFA_NO_IP_ETH_MINLEN (PFA_BDC_BASE + 0x818)
#define PFA_NO_IP_ETH_MAXLEN (PFA_BDC_BASE + 0x81C)
#define PFA_L2DLE_BD_CFG (PFA_BDC_BASE + 0x820)
#define PFA_BDC_DEBUG (PFA_BDC_BASE + 0x900)

#define PFA_RDC_BASE 0x0002C000
#define PFA_RDQ_FULL_LEVEL (PFA_RDC_BASE + 0x70)
#define PFA_RDQX_BADDR_L(x) (PFA_RDC_BASE + 0x80 + 0x40 * (x))
#define PFA_RDQX_BADDR_H(x) (PFA_RDC_BASE + 0x84 + 0x40 * (x))
#define PFA_RDQX_LEN(x) (PFA_RDC_BASE + 0x88 + 0x40 * (x))
#define PFA_RDQX_WPTR(x) (PFA_RDC_BASE + 0x8c + 0x40 * (x))
#define PFA_RDQX_RPTR(x) (PFA_RDC_BASE + 0x90 + 0x40 * (x))
#define PFA_RDQX_ONE_RD_SIZE(x) (PFA_RDC_BASE + 0x94 + 0x40 * (x))
#define PFA_RDQX_WPTR_UPDATE_ADDR_L(x) (PFA_RDC_BASE + 0x98 + 0x40 * (x))
#define PFA_RDQX_WPTR_UPDATE_ADDR_H(x) (PFA_RDC_BASE + 0x9c + 0x40 * (x))
#define PFA_RDQX_CTRL(x) (PFA_RDC_BASE + 0xa0 + 0x40 * (x))
#define PFA_RDQX_CLR(x) (PFA_RDC_BASE + 0xa4 + 0x40 * (x))
#define PFA_RDQX_DEBUG(x) (PFA_RDC_BASE + 0x0AC + 0x40 * (x))
#define PFA_RDQX_STATE(x) (PFA_RDC_BASE + 0x0B0 + 0x40 * (x))

#define PFA_ICTRL_BASE 0x0002F000
#define PFA_TD_DONE_INTA_STATUS (PFA_ICTRL_BASE + 0x0)
#define PFA_TD_DONE_INTA_MASK (PFA_ICTRL_BASE + 0x4)
#define PFA_TD_DONE_INTA (PFA_ICTRL_BASE + 0x8)
#define PFA_RD_DONE_INTA_STATUS (PFA_ICTRL_BASE + 0x10)
#define PFA_RD_DONE_INTA_MASK (PFA_ICTRL_BASE + 0x14)
#define PFA_RD_DONE_INTA (PFA_ICTRL_BASE + 0x18)
#define PFA_RDQ_FULL_INTA_STATUS (PFA_ICTRL_BASE + 0x20)
#define PFA_RDQ_FULL_INTA_MASK (PFA_ICTRL_BASE + 0x24)
#define PFA_RDQ_FULL_INTA (PFA_ICTRL_BASE + 0x28)
#define PFA_ADQ_EMPTY_INTA_STATUS (PFA_ICTRL_BASE + 0x30)
#define PFA_ADQ_EMPTY_INTA_MASK (PFA_ICTRL_BASE + 0x34)
#define PFA_ADQ_EMPTY_INTA (PFA_ICTRL_BASE + 0x38)
#define PFA_DMA_DONE_INTA_STATUS (PFA_ICTRL_BASE + 0x40)
#define PFA_DMA_DONE_INTA_MASK (PFA_ICTRL_BASE + 0x44)
#define PFA_DMA_DONE_INTA (PFA_ICTRL_BASE + 0x48)
#define PFA_V2X_TD_DONE_INTA_STATUS (PFA_ICTRL_BASE + 0x100)
#define PFA_V2X_TD_DONE_INTA_MASK (PFA_ICTRL_BASE + 0x104)
#define PFA_V2X_TD_DONE_INTA (PFA_ICTRL_BASE + 0x108)
#define PFA_V2X_RD_DONE_INTA_STATUS (PFA_ICTRL_BASE + 0x110)
#define PFA_V2X_RD_DONE_INTA_MASK (PFA_ICTRL_BASE + 0x114)
#define PFA_V2X_RD_DONE_INTA (PFA_ICTRL_BASE + 0x118)
#define PFA_V2X_RDQ_FULL_INTA_MASK (PFA_ICTRL_BASE + 0x124)
#define PFA_V2X_RDQ_FULL_INTA (PFA_ICTRL_BASE + 0x128)
#define PFA_V2X_ADQ_EMPTY_INTA_MASK (PFA_ICTRL_BASE + 0x134)
#define PFA_V2X_ADQ_EMPTY_INTA (PFA_ICTRL_BASE + 0x138)

#define PFA_TD_DONE_DPA_STATUS (PFA_ICTRL_BASE + 0x200)
#define PFA_DMA_DONE_INTDPA (PFA_ICTRL_BASE + 0x248)

#define PFA_INTA_INTERVAL (PFA_ICTRL_BASE + 0x300)
#define PFA_INTV2X_INTERVAL (PFA_ICTRL_BASE + 0x304)
#define PFA_INTDPA_INTERVAL (PFA_ICTRL_BASE + 0x308)

#define PFA_TIME_MAX 0xFFFF
#define PFA_TAB_CTRL_RM_HCACHE 0x8

#ifdef CONFIG_PFA_FW
#define PFA_IPFW_HTABLE_SIZE (64)
#else
#define PFA_IPFW_HTABLE_SIZE (1)
#endif

#define PFA_MAC_TAB_NUMBER (128) // ram: 0x2000--0x27ff, reg:0x2800--0x2fff
#define PFA_MAC_TAB_WORD_NO (4)
#define PFA_MAC_TAB_REG_WORD_NO (2)
#define PFA_IPV4_TAB_NUMBER (16)
#define PFA_IPV4_TAB_WORD_NO (6)
#define PFA_IPV6_TAB_NUMBER (8)
#define PFA_IPV6_TAB_WORD_NO (12)
#define MAX_L4_PORTNUM_CFG_NUM (16)
#define PFA_QOS_MAC_TAB_NUMBER (0)
#define PFA_QOS_MAC_TAB_WORD_NO (3)
#define PFA_MAC_PDU_TAB_NUMBER (32)
#define PFA_MAC_PDU_TAB_WORD_NO (6)
#define PFA_IP_QOS_TAB_NUMBER (0)
#define PFA_IP_QOS_TAB_WORD_NO (14)

#define PFA_TAB_ENTRY_INTERVAL (4)
#define PFA_MAC_ENTRY_NUM (512) // RAM
#define PFA_MAC_REG_ENTRY_NUM (512)
#define PFA_IPV4_ENTRY_NUM (96)
#define PFA_IPV6_ENTRY_NUM (96)
#define PFA_QOS_MAC_ENTRY_NUM (0)
#define PFA_MAC_PDU_ENTRY_NUM (192)
#define PFA_IP_QOS_ENTRY_NUM (0)

#define PFA_TAB (PFA_ENG_BASE + 0x2000)
#define PFA_MAC_TAB(x) (PFA_TAB + PFA_TAB_ENTRY_INTERVAL * (x))
#define PFA_MAC_TAB_REG(x) (PFA_MAC_TAB(PFA_MAC_ENTRY_NUM) + PFA_TAB_ENTRY_INTERVAL * (x))
#define PFA_IPV4_TAB(x) (PFA_MAC_TAB_REG(PFA_MAC_ENTRY_NUM) + PFA_TAB_ENTRY_INTERVAL * (x))
#define PFA_IPV6_TAB(x) (PFA_IPV4_TAB(PFA_IPV4_ENTRY_NUM) + PFA_TAB_ENTRY_INTERVAL * (x))
#define PFA_QOS_MAC_TAB(x) (PFA_IPV6_TAB(PFA_IPV6_ENTRY_NUM) + PFA_TAB_ENTRY_INTERVAL * (x))
#define PFA_MAC_PDU_TAB(x) (PFA_QOS_MAC_TAB(PFA_QOS_MAC_ENTRY_NUM) + PFA_TAB_ENTRY_INTERVAL * (x))
#define PFA_IP_QOS_TAB(x) (PFA_MAC_PDU_TAB(PFA_MAC_PDU_ENTRY_NUM) + PFA_TAB_ENTRY_INTERVAL * (x))

#define PFA_RESET_BAK_REG_NUM 32
#define PFA_STASH_REG_BUTTON 2
extern unsigned int g_pfa_reset_bak_reg[][PFA_STASH_REG_BUTTON];

#define PFA_REG_DUMP_GROUP_NUM 14
#define PFA_DUMP_SIZE (1024 * 6) /* pfa Application 3k sieze */

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
        unsigned int rdq_cnt : 10;
        unsigned int full : 1;
        unsigned int empty : 1;
        unsigned int busy : 1;
        unsigned int reserved : 19;
    } bits;
    unsigned int u32;
} pfa_rdq_debug_t;

typedef union {
    struct {
        unsigned int full : 1;
        unsigned int empty : 1;
        unsigned int busy : 1;
        unsigned int reserved : 30;
    } bits;
    unsigned int u32;
} pfa_rdq_stat_t;

typedef union {
    struct {
        unsigned int full : 1;
        unsigned int empty : 1;
        unsigned int reserved : 30;
    } bits;
    unsigned int u32;
} pfa_bdq_stat_t;

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
#define PFA_CLK_DIV 0x23c
#define PFA_CLK_DIV_DEFAULT 0x3F0020
typedef union {
    struct {
        unsigned int reserved : 4;
        unsigned int pfa_div : 4;
        unsigned int reserved1 : 8;
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
        unsigned int pfa_port_enc_type : 4; // v200 is stub
        unsigned int pfa_port_pad_en : 1;
        unsigned int mac_ip_fw_flag        : 1   ; /* [7] */
        unsigned int pfa_dport_adq_core_type : 1   ; /* [8] */
        unsigned int pfa_port_pass_mode    : 1   ; /* [9] */
        unsigned int pfa_ppp_mac_hdr       : 1   ; /* [10] v200 no use */
        unsigned int pfa_ppp_hdr_user      : 1   ; /* [11] v200 no use */
        unsigned int reserved_0            : 19  ; /* [30..12] */
        unsigned int reserved_1 : 1; /* v200 no use */
    } bits;
    unsigned int u32;
} pfa_port_prop_t;

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
        unsigned int pfa_ncm_max_len : 16;
        unsigned int pfa_rndis_max_len : 16;
    } bits;
    unsigned int u32;
} pfa_unpack_max_len_t;

typedef union {
    struct {
        unsigned int pfa_tab_clr : 8;
        unsigned int reserved : 24;
    } bits;
    unsigned int u32;
} pfa_tab_clt_t;

typedef union {
    struct {
        unsigned int reserved0 : 8;
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

typedef union {
    struct {
        unsigned int bdq_push_en : 1;
        unsigned int reserved : 7;
        unsigned int bdq_pri : 8;
        unsigned int bus_sel : 1;
        unsigned int dis_action_sel : 1;
        unsigned int reserved1 : 14;
    } bits;
    unsigned int u32;
} pfa_bd_ctrl_t;

typedef union {
    struct {
        unsigned int pfa_enable : 1;
        unsigned int pfa_disable : 1;
        unsigned int disable_modem_port : 1;
        unsigned int enable_modem_port : 1;
        unsigned int reserved : 28;
    } bits;
    unsigned int u32;
} pfa_acpu2pfa_soft_int_t;

enum pfa_enc_type_phy {
    PFA_ENC_TPYE_NONE,
    PFA_ENC_TPYE_NCM_NTH16,
    PFA_ENC_TPYE_NCM_NTH32,
    PFA_ENC_TPYE_RNDIS,
    PFA_ENC_TPYE_ACPU,
    PFA_ENC_TPYE_ACPU_V2X,
    PFA_ENC_TPYE_CCPU,
    PFA_ENC_TPYE_CCPU_SP,
    PFA_ENC_TPYE_CCPU_URLLC,
    PFA_ENC_TPYE_CCPU_V2X,
    PFA_ENC_TPYE_L2DLE,
    PFA_ENC_TPYE_BOTTOM
};

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

#endif /* PFA_HAL_REG_H */
