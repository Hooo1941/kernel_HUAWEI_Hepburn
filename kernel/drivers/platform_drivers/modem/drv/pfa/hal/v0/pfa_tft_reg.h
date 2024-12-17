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

#ifndef PFA_TFT_REG_OFFSET_H
#define PFA_TFT_REG_OFFSET_H

#define PFA_TFT_SRST_REG (0x0)           /* PFA_TFT复位控制寄存器 */
#define PFA_TFT_SRST_STATE_REG (0x4)     /* PFA_TFT复位状态寄存器 */
#define PFA_TFT_CH_EN_REG (0x8)          /* PFA_TFT 通道使能配置寄存器 */
#define PFA_TFT_EN_STATE_REG (0xC)       /* PFA_TFT 通道使能状态寄存器 */
#define PFA_TFT_GATE_REG (0x10)          /* PFA_TFT时钟门控配置寄存器 */
#define PFA_TFT_CTRL_REG (0x14)          /* PFA_TFT全局控制配置寄存器 */
#define PFA_TFT_DMA_CTRL0_REG (0x18)     /* PFA_TFT DMA控制寄存器0 */
#define PFA_TFT_DMA_CTRL1_REG (0x1C)     /* PFA_TFT DMA控制寄存器1 */
#define PFA_TFT_VERSION_REG (0x20)       /* PFA_TFT版本寄存器 */
#define PFA_TFT_STATE0_REG (0x24)        /* PFA_TFT状态寄存器0 */
#define PFA_TFT_STATE1_REG (0x28)        /* PFA_TFT状态寄存器1 */
#define PFA_TFT_INT_CTRL_REG (0x2C)      /* PFA_TFT中断控制寄存器 */
#define PFA_TFT_INT0_SUP_TIME_REG (0x30) /* PFA_TFT中断0压制时间寄存器 */
#define PFA_TFT_INT1_SUP_TIME_REG (0x34) /* PFA_TFT中断1压制时间寄存器 */
#define PFA_TFT_INT2_SUP_TIME_REG (0x38) /* PFA_TFT中断2压制时间寄存器 */
#define PFA_TFT_GEN_INT_REG (0x3C)       /* PFA_TFT中断产生寄存器 */
#define PFA_TFT_INT0_UL_REG (0x40)       /* 上行中断寄存器0 */
#define PFA_TFT_INT0_DL_REG (0x44)       /* 下行中断寄存器0 */
#define PFA_TFT_INT1_UL_REG (0x48)       /* 上行中断寄存器1 */
#define PFA_TFT_INT1_DL_REG (0x4C)       /* 下行中断寄存器1 */
#define PFA_TFT_INT2_UL_REG (0x50)       /* 上行中断寄存器2 */
#define PFA_TFT_INT2_DL_REG (0x54)       /* 下行中断寄存器2 */
#define PFA_TFT_INT0_EN_UL_REG (0x58)    /* 上行中断使能寄存器0 */
#define PFA_TFT_INT0_EN_DL_REG (0x5C)    /* 下行中断使能寄存器0 */
#define PFA_TFT_INT1_EN_UL_REG (0x60)    /* 上行中断使能寄存器1 */
#define PFA_TFT_INT1_EN_DL_REG (0x64)    /* 下行中断使能寄存器1 */
#define PFA_TFT_INT2_EN_UL_REG (0x68)    /* 上行中断使能寄存器2 */
#define PFA_TFT_INT2_EN_DL_REG (0x6C)    /* 下行中断使能寄存器2 */
#define PFA_TFT_INT_STATE_UL_REG (0x70)  /* 上行中断状态寄存器 */
#define PFA_TFT_INT_STATE_DL_REG (0x74)  /* 下行中断状态寄存器 */
#define PFA_TFT_FIFO_STATE0_REG (0x78)   /* PFA_TFT FIFO状态寄存器0 */
#define PFA_TFT_FIFO_STATE1_REG (0x7C)   /* PFA_TFT FIFO状态寄存器1 */
#define PFA_TFT_FIFO_STATE2_REG (0x80)   /* PFA_TFT FIFO状态寄存器2 */
#define PFA_TFT_FIFO_STATE3_REG (0x84)   /* PFA_TFT FIFO状态寄存器3 */
#define PFA_TFT_FIFO_STATE4_REG (0x88)   /* PFA_TFT FIFO状态寄存器4 */
#define PFA_TFT_FIFO_STATE5_REG (0x8C)   /* PFA_TFT FIFO状态寄存器5 */
#define PFA_TFT_SPLIT_EN_REG (0x90)
#define PFA_TFT_TO_ACPU_FLAG_MASK_REG (0x94)
#define PFA_TFT_TIME_OUT_REG (0xA0)                     /* 超时配置寄存器 */
#define PFA_TFT_PKT_LEN_REG (0xA4)                      /* 包长配置寄存器 */
#define PFA_TFT_FILTER_ZERO_INDEX_REG (0xA8)            /* 过滤器起始索引配置寄存器 */
#define PFA_TFT_EF_BADDR_L_REG (0xAC)                   /* 扩展过滤器基地址配置寄存器低位 */
#define PFA_TFT_EF_BADDR_H_REG (0xB0)                   /* 扩展过滤器基地址配置寄存器高位 */
#define FLT_CHAIN_LOOP_REG (0xB4)                   /* 过滤器链表环告警 */
#define PFA_TFT_TRANS_CNT_CTRL_REG (0xB8)               /* 流量统计控制寄存器 */
#define PFA_TFT_TIMER_LOAD_REG (0xBC)                   /* TIMER计数器初始值 */
#define PFA_TFT_TIMER_COUNTER_REG (0xC0)                /* TIMER计数器计数值 */
#define PFA_TFT_CH0_PKT_CNT0_REG (0xC4)                 /* 上行RDQ0数据包计数器计数值。 */
#define PFA_TFT_CH0_PKT_CNT1_REG (0xC8)                 /* 上行RDQ1数据包计数器计数值。 */
#define PFA_TFT_CH1_PKT_CNT_REG (0xCC)                  /* 下行数据包计数器计数值。 */
#define PFA_TFT_CH0_CTRL_REG (0x100)                    /* 上行通道控制寄存器 */
#define PFA_TFT_CH0_STATE_REG (0x104)                   /* 上行通道状态寄存器 */
#define PFA_TFT_CH0_BDQ0_BADDR_L_REG (0x108)            /* 上行通道BDQ0起始地址低位 */
#define PFA_TFT_CH0_BDQ0_BADDR_H_REG (0x10C)            /* 上行通道BDQ0起始地址高位 */
#define PFA_TFT_CH0_BDQ0_SIZE_REG (0x110)               /* 上行通道BDQ0深度 */
#define PFA_TFT_CH0_BDQ0_RPTR_REG (0x114)               /* 上行通道BDQ0读指针 */
#define PFA_TFT_CH0_BDQ0_WPTR_REG (0x118)               /* 上行通道BDQ0写指针 */
#define PFA_TFT_CH0_BDQ0_WADDR_L_REG (0x11C)            /* 上行通道BDQ0写地址低位 */
#define PFA_TFT_CH0_BDQ0_WADDR_H_REG (0x120)            /* 上行通道BDQ0写地址高位 */
#define PFA_TFT_CH0_BDQ0_RADDR_L_REG (0x124)            /* 上行通道BDQ0读地址低位 */
#define PFA_TFT_CH0_BDQ0_RADDR_H_REG (0x128)            /* 上行通道BDQ0读地址高位 */
#define PFA_TFT_CH0_BDQ0_DEPTH_REG (0x12C)              /* 上行通道BDQ0队列深度指示寄存器 */
#define PFA_TFT_CH0_BDQ0_RPTR_UPDATE_ADDR_L_REG (0x130) /* 上行通道BDQ0读指针更新目的地址低位寄存器 */
#define PFA_TFT_CH0_BDQ0_RPTR_UPDATE_ADDR_H_REG (0x134) /* 上行通道BDQ0读指针更新目的地址高位寄存器 */
#define PFA_TFT_CH1_CTRL_REG (0x168)                    /* 下行通道控制寄存器 */
#define PFA_TFT_CH1_STATE_REG (0x16C)                   /* 下行通道状态寄存器 */
#define PFA_TFT_CH1_RDQ_BADDR_L_REG (0x170)             /* 下行通道RDQ起始地址低位 */
#define PFA_TFT_CH1_RDQ_BADDR_H_REG (0x174)             /* 下行通道RDQ起始地址高位 */
#define PFA_TFT_CH1_RDQ_SIZE_REG (0x178)                /* 下行通道RDQ深度 */
#define PFA_TFT_CH1_RDQ_WPTR_REG (0x17C)                /* 下行通道RDQ写指针 */
#define PFA_TFT_CH1_RDQ_RPTR_REG (0x180)                /* 下行通道RDQ读指针 */
#define PFA_TFT_CH1_RDQ_WADDR_L_REG (0x184)             /* 下行通道RDQ写地址低位 */
#define PFA_TFT_CH1_RDQ_WADDR_H_REG (0x188)             /* 下行通道RDQ写地址高位 */
#define PFA_TFT_CH1_RDQ_RADDR_L_REG (0x18C)             /* 下行通道RDQ读地址低位 */
#define PFA_TFT_CH1_RDQ_RADDR_H_REG (0x190)             /* 下行通道RDQ读地址高位 */
#define PFA_TFT_CH1_RDQ_DEPTH_REG (0x194)               /* 下行通道队列深度指示寄存器 */
#define PFA_TFT_CH1_RDQ_WPTR_UPDATE_ADDR_L_REG (0x198) /* 下行通道RDQ队列写指针更新目的地址低位寄存器 */
#define PFA_TFT_CH1_RDQ_WPTR_UPDATE_ADDR_H_REG (0x19C) /* 下行通道RDQ队列写指针更新目的地址高位寄存器 */
#define PFA_TFT_CH1_ADQ_EN_CTRL_REG (0x1A0)            /* 下行通道ADQ使能控制寄存器 */
#define PFA_TFT_CH1_ADQ_SIZE_CTRL_REG (0x1A4)          /* 下行通道ADQ SIZE控制寄存器 */
#define PFA_TFT_CH1_ADQ0_BASE_L_REG (0x1A8)            /* 下行通道ADQ0基址寄存器低位 */
#define PFA_TFT_CH1_ADQ0_BASE_H_REG (0x1AC)            /* 下行通道ADQ0基址寄存器高位 */
#define PFA_TFT_CH1_ADQ0_STAT_REG (0x1B0)              /* 下行通道ADQ0状态寄存器 */
#define PFA_TFT_CH1_ADQ0_WPTR_REG (0x1B4)              /* 下行通道ADQ0写指针寄存器 */
#define PFA_TFT_CH1_ADQ0_RPTR_REG (0x1B8)              /* 下行通道ADQ0读指针寄存器 */
#define PFA_TFT_CH1_ADQ0_RPTR_UPDATE_ADDR_L_REG (0x1BC) /* 下行通道ADQ0读指针更新目的地址低位寄存器 */
#define PFA_TFT_CH1_ADQ0_RPTR_UPDATE_ADDR_H_REG (0x1C0) /* 下行通道ADQ0读指针更新目的地址高位寄存器 */
#define PFA_TFT_CH1_ADQ1_BASE_L_REG (0x1C4)             /* 下行通道ADQ1基址寄存器低位 */
#define PFA_TFT_CH1_ADQ1_BASE_H_REG (0x1C8)             /* 下行通道ADQ1基址寄存器高位 */
#define PFA_TFT_CH1_ADQ1_STAT_REG (0x1CC)               /* 下行通道ADQ1状态寄存器 */
#define PFA_TFT_CH1_ADQ1_WPTR_REG (0x1D0)               /* 下行通道ADQ1写指针寄存器 */
#define PFA_TFT_CH1_ADQ1_RPTR_REG (0x1D4)               /* 下行通道ADQ1读指针寄存器 */
#define PFA_TFT_CH1_ADQ1_RPTR_UPDATE_ADDR_L_REG (0x1D8) /* 下行通道ADQ1读指针更新目的地址低位寄存器 */
#define PFA_TFT_CH1_ADQ1_RPTR_UPDATE_ADDR_H_REG (0x1DC) /* 下行通道ADQ1读指针更新目的地址高位寄存器 */

#define FRAG_WPTR_REG (0x240)                        /* 分片信息记录表当前写指针 */
#define TBL_UPDATE_REQ_REG (0x24C)                   /* REG更新请求寄存器 */
#define TBL_UPDATE_ACK_REG (0x250)                   /* REG更新握手寄存器 */
#define FRAG_TLB_EN_IPV4_REG (0x25C)                 /* IPV4分片信息记录表使能寄存器 */
#define FRAG_TLB_EN_IPV6_REG (0x260)                 /* IPV6分片信息记录表使能寄存器 */
#define TBL_QOS_DRB_EN0_REG (0x264)                  /* QoS Flow ID到DRB ID映射表使能寄存器0 */
#define TBL_QOS_DRB_EN1_REG (0x268)                  /* QoS Flow ID到DRB ID映射表使能寄存器1 */
#define TBL_PDU_DRB_EN_REG (0x26C)                   /* PDU Session ID到DRB ID映射表使能寄存器 */
#define PFA_TFT_FIFO_STATE6_REG (0x270)
#define PFA_TFT_FIFO_STATE7_REG (0x274)
#define PFA_TFT_FIFO_STATE8_REG (0x278)
#define PFA_TFT_CH0_BDQ2_BADDR_L_REG (0x27c)
#define PFA_TFT_CH0_BDQ2_BADDR_H_REG (0x280)
#define PFA_TFT_CH0_BDQ2_SIZE_REG (0x284)
#define PFA_TFT_CH0_BDQ2_RPTR_REG (0x288)
#define PFA_TFT_CH0_BDQ2_WPTR_REG (0x28c)
#define PFA_TFT_CH0_BDQ2_WADDR_L_REG (0x290)
#define PFA_TFT_CH0_BDQ2_WADDR_H_REG (0x294)
#define PFA_TFT_CH0_BDQ2_RADDR_L_REG (0x298)
#define PFA_TFT_CH0_BDQ2_RADDR_H_REG (0x29c)
#define PFA_TFT_CH0_BDQ2_DEPTH_REG (0x2a0)
#define PFA_TFT_CH0_BDQ2_RPTR_UPDATE_ADDR_L_REG (0x2a4)
#define PFA_TFT_CH0_BDQ2_RPTR_UPDATE_ADDR_H_REG (0x2a8)
#define PFA_TFT_CH1_RDQ1_BADDR_L_REG (0x30c)
#define PFA_TFT_CH1_RDQ1_BADDR_H_REG (0x310)
#define PFA_TFT_CH1_RDQ1_SIZE_REG (0x314)
#define PFA_TFT_CH1_RDQ1_WPTR_REG (0x318)
#define PFA_TFT_CH1_RDQ1_RPTR_REG (0x31c)
#define PFA_TFT_CH1_RDQ1_WADDR_L_REG (0x320)
#define PFA_TFT_CH1_RDQ1_WADDR_H_REG (0x324)
#define PFA_TFT_CH1_RDQ1_RADDR_L_REG (0x328)
#define PFA_TFT_CH1_RDQ1_RADDR_H_REG (0x32c)
#define PFA_TFT_CH1_RDQ1_DEPTH_REG (0x330)
#define PFA_TFT_CH1_RDQ1_WPTR_UPDATE_ADDR_L_REG (0x334)
#define PFA_TFT_CH1_RDQ1_WPTR_UPDATE_ADDR_H_REG (0x338)
#define PFA_TFT_CH_PRIOR_LEVEL_CFG_REG (0x33c)
#define PFA_TFT_WRR_CEILING0_REG (0x340)
#define PFA_TFT_WRR_CEILING1_REG (0x344)

#define PFA_TFT_CH1_PKT_CNT1_REG (0x41c)
#define PFA_TFT_CH1_PKT_CNT2_REG (0x420)
#define PFA_TFT_DL_RNRDQ_FULL_CNT_REG (0x424)
#define PFA_TFT_DL_RARDQ_FULL_CNT_REG (0x428)

#define PFA_TFT_DL_RDQ1_FULL_CNT_REG (0x438)
#define PFA_TFT_DL_RDQ2_FULL_CNT_REG (0x43c)

#define PFA_TFT_SC_IDX_MAX 4
#define PFA_TFT_SC_REG0 0x470
#define PFA_TFT_SC_REG(x) (PFA_TFT_SC_REG0 + 0x4 * (x))

#define TBLM_QOS_REFL_REG_G(n) (0x500 + 0x4 * (n))
#define TBLK_PDU_REFL_REG_G(n) (0x600 + 0x4 * (n))
#define TBLM_RAB_REFL_REG_G(n) (0x680 + 0x4 * (n))
#define PFA_TFT_SEC_ATTR_REG (0x800)                    /* PFA_TFT AXI MST SEC属性寄存器 */

#define PFA_TFT_SEC_ATTR1_REG (0x904)
#define PFA_TFT_CH1_BDQ2_BADDR_L_REG (0x908)
#define PFA_TFT_CH1_BDQ2_BADDR_H_REG (0x90c)
#define PFA_TFT_CH1_BDQ2_SIZE_REG (0x910)
#define PFA_TFT_CH1_BDQ2_WPTR_REG (0x914)
#define PFA_TFT_CH1_BDQ2_RPTR_REG (0x918)
#define PFA_TFT_CH1_BDQ2_WADDR_L_REG (0x91c)
#define PFA_TFT_CH1_BDQ2_WADDR_H_REG (0x920)
#define PFA_TFT_CH1_BDQ2_RADDR_L_REG (0x924)
#define PFA_TFT_CH1_BDQ2_RADDR_H_REG (0x928)
#define PFA_TFT_CH1_BDQ2_DEPTH_REG (0x92c)
#define PFA_TFT_CH1_BDQ2_RPTR_UPDATE_ADDR_L_REG (0x930)
#define PFA_TFT_CH1_BDQ2_RPTR_UPDATE_ADDR_H_REG (0x934)
#define PFA_TFT_CH1_RDQ2_BADDR_L_REG (0x938)
#define PFA_TFT_CH1_RDQ2_BADDR_H_REG (0x93c)
#define PFA_TFT_CH1_RDQ2_SIZE_REG (0x940)
#define PFA_TFT_CH1_RDQ2_WPTR_REG (0x944)
#define PFA_TFT_CH1_RDQ2_RPTR_REG (0x948)
#define PFA_TFT_CH1_RDQ2_WADDR_L_REG (0x94c)
#define PFA_TFT_CH1_RDQ2_WADDR_H_REG (0x950)
#define PFA_TFT_CH1_RDQ2_RADDR_L_REG (0x954)
#define PFA_TFT_CH1_RDQ2_RADDR_H_REG (0x958)
#define PFA_TFT_CH1_RDQ2_DEPTH_REG (0x95c)
#define PFA_TFT_CH1_RDQ2_WPTR_UPDATE_ADDR_L_REG (0x960)
#define PFA_TFT_CH1_RDQ2_WPTR_UPDATE_ADDR_H_REG (0x964)
#define PFA_TFT_CH0_BDQ1_BADDR_L_REG (0x96c)
#define PFA_TFT_CH0_BDQ1_BADDR_H_REG (0x970)
#define PFA_TFT_CH0_BDQ1_SIZE_REG (0x974)
#define PFA_TFT_CH0_BDQ1_RPTR_REG (0x978)
#define PFA_TFT_CH0_BDQ1_WPTR_REG (0x97c)
#define PFA_TFT_CH0_BDQ1_WADDR_L_REG (0x980)
#define PFA_TFT_CH0_BDQ1_WADDR_H_REG (0x984)
#define PFA_TFT_CH0_BDQ1_RADDR_L_REG (0x988)
#define PFA_TFT_CH0_BDQ1_RADDR_H_REG (0x98c)
#define PFA_TFT_CH0_BDQ1_DEPTH_REG (0x990)
#define PFA_TFT_CH0_BDQ1_RPTR_UPDATE_ADDR_L_REG (0x994)
#define PFA_TFT_CH0_BDQ1_RPTR_UPDATE_ADDR_H_REG (0x998)
#define PFA_TFT_SEC_AREA_CTRL (0x99c)

#define FLTN_LOCAL_ADDR_REG(n) (FLTN_LOCAL_ADDR0_REG_G(n))
#define PFA_TFT_CH1_BDQ_DEPTH_REG(n) (PFA_TFT_CH1_BDQ0_DEPTH_REG + (n) * 0x30)
#define PFA_TFT_CH0_RDQ_DEPTH_REG(n) (PFA_TFT_CH0_RDQ0_DEPTH_REG + (n) * 0x30)

/* Define the union U_PFA_TFT_CH_EN */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int ul0_en : 1;      /* [0]  */
        unsigned int ul1_en : 1;      /* [1]  */
        unsigned int ul2_en : 1;      /* [0]  */
        unsigned int dl0_en : 1;      /* [1]  */
        unsigned int dl1_en : 1;      /* [0]  */
        unsigned int dl2_en : 1;      /* [1]  */
        unsigned int reserved_0 : 26; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} U_PFA_TFT_CH_EN;

/* Define the union U_PFA_TFT_GATE */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int dma_clk_sel : 1;      /* [0]  */
        unsigned int core_clk_sel : 1;     /* [1]  */
        unsigned int cfg_clk_sel : 1;      /* [2]  */
        unsigned int ram_clk_sel : 1;      /* [3]  */
        unsigned int oam_clk_sel : 1;      /* [4]  */
        unsigned int hac_gif_aclk_sel : 1; /* [5]  */
        unsigned int hac_gif_hclk_sel : 1; /* [6]  */
        unsigned int reserved_0 : 25;      /* [31..7]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} U_PFA_TFT_GATE;

/* Define the union U_PFA_TFT_CTRL */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int filter_seq : 1;        /* [0] */
        unsigned int flt_addr_reverse : 1;  /* [1] */
        unsigned int sp_cfg : 1;            /* [2] */
        unsigned int sp_wrr_sel : 1;        /* [3] */
        unsigned int max_burst_len : 2;     /* [5..4] */
        unsigned int ipv6_nh_sel : 1;       /* [6] */
        unsigned int ah_esp_sel : 1;        /* [7] */
        unsigned int ah_disable : 1;        /* [8] */
        unsigned int esp_disable : 1;       /* [9] */
        unsigned int mfc_en : 1;            /* [10] */
        unsigned int mm_en : 1;             /* [11] */
        unsigned int reserved_3 : 1;        /* [12] */
        unsigned int reserved_2 : 1;        /* [13] */
        unsigned int cd_en_sel : 1;         /* [14] */
        unsigned int qos_flow_id_width : 1; /* [15] */
        unsigned int reserved_1 : 1;        /* [16] */
        unsigned int dl_refl_en : 1;        /* [17] */
        unsigned int dl_relf_as_en : 1;     /* [18] */
        unsigned int dl_rdq_with_head : 1;  /* [19] */
        unsigned int ul_bdq_with_head : 1;  /* [20] */
        unsigned int ul_frag_tbl_en : 1;    /* [21] */
        unsigned int dl_adq_sel_cfg : 1;    /* [22] */
        unsigned int ul_adq_sel_cfg : 1;    /* [23] */
        unsigned int reserved_0 : 8;        /* [31..24] */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} U_PFA_TFT_CTRL;

/* Define the union U_PFA_TFT_TRANS_CNT_CTRL */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int ul_pkt_cnt_en : 1;            /* [0] */
        unsigned int ul_pkt_cnt_clear : 1;         /* [1] */
        unsigned int ul_pkt_cnt_prescaler_sel : 2; /* [3..2] */
        unsigned int reserved_2 : 4;               /* [7..4] */
        unsigned int dl_pkt_cnt_en : 1;            /* [8] */
        unsigned int dl_pkt_cnt_clear : 1;         /* [9] */
        unsigned int dl_pkt_cnt_prescaler_sel : 2; /* [11..10] */
        unsigned int reserved_1 : 4;               /* [15..12] */
        unsigned int timer_en : 1;                 /* [16] */
        unsigned int timer_auto_reload : 1;        /* [17] */
        unsigned int timer_clear : 1;              /* [18] */
        unsigned int reserved_0 : 13;              /* [31..19] */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} U_PFA_TFT_TRANS_CNT_CTRL;

/* Define the union U_PFA_TFT_TIMER_LOAD */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int timer_load : 10; /* [9..0]  */
        unsigned int reserved_0 : 22; /* [31..10]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} U_PFA_TFT_TIMER_LOAD;

/* Define the union U_PFA_TFT_CH1_BDQ0_DEPTH */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int dl_bdq0_depth : 15; /* [14..0]  */
        unsigned int reserved_0 : 17;    /* [31..15]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} U_PFA_TFT_CH1_BDQ0_DEPTH;

/* Define the union U_PFA_TFT_CH0_RDQ0_DEPTH */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int ul_rdq0_depth : 15; /* [14..0]  */
        unsigned int reserved_0 : 17;    /* [31..15]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} U_PFA_TFT_CH0_RDQ0_DEPTH;

typedef union {
    /* Define the struct bits */
    struct {
        unsigned int time_out_cfg : 16;  /* [15..0]  */
        unsigned int time_out_valid : 1; /* [16]  */
        unsigned int reserved_0 : 15;    /* [31..17]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} U_PFA_TFT_TIME_OUT;

/* Define the union U_PFA_TFT_SEC_ATTR */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int ul_wdata_sec_n : 1;     /* [0] */
        unsigned int ul_rd0_sec_n : 1;       /* [1] */
        unsigned int ul_rd1_sec_n : 1;       /* [2] */
        unsigned int ul_bd0_sec_n : 1;       /* [3] */
        unsigned int ul_bd1_sec_n : 1;       /* [4] */
        unsigned int ul_bd2_sec_n : 1;       /* [5] */
        unsigned int ul_ad_sec_n : 1;        /* [6] */
        unsigned int ul_cd_sec_n : 1;        /* [7] */
        unsigned int ul_bdq0_rptr_sec_n : 1; /* [8] */
        unsigned int ul_bdq1_rptr_sec_n : 1; /* [9] */
        unsigned int ul_bdq2_rptr_sec_n : 1; /* [10] */
        unsigned int ul_adq_rptr_sec_n : 1;  /* [11] */
        unsigned int ul_rdq0_wptr_sec_n : 1; /* [12] */
        unsigned int ul_rdq1_wptr_sec_n : 1; /* [13] */
        unsigned int dl_wdata_sec_n : 1;     /* [14] */
        unsigned int dl_rd0_sec_n : 1;       /* [15] */
        unsigned int dl_rd1_sec_n : 1;       /* [16] */
        unsigned int dl_rd2_sec_n : 1;       /* [17] */
        unsigned int dl_bd0_sec_n : 1;       /* [18] */
        unsigned int dl_bd1_sec_n : 1;       /* [19] */
        unsigned int dl_bd2_sec_n : 1;       /* [20] */
        unsigned int dl_ad_sec_n : 1;        /* [21] */
        unsigned int dl_cd_sec_n : 1;        /* [22] */
        unsigned int dl_bdq0_rptr_sec_n : 1; /* [23] */
        unsigned int dl_bdq1_rptr_sec_n : 1; /* [24] */
        unsigned int dl_bdq2_rptr_sec_n : 1; /* [25] */
        unsigned int dl_adq_rptr_sec_n : 1;  /* [26] */
        unsigned int dl_rdq0_wptr_sec_n : 1; /* [27] */
        unsigned int dl_rdq1_wptr_sec_n : 1; /* [28] */
        unsigned int dl_rdq2_wptr_sec_n : 1; /* [29] */
        unsigned int dl_rnrd_sec_n : 1;      /* [30] */
        unsigned int dl_rard_sec_n : 1;      /* [31] */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} U_PFA_TFT_SEC_ATTR;

/* Define the union U_PFA_TFT_SEC_ATTR1 */
typedef union {
    struct {
        unsigned int dl_rnrd_wptr_sec_n : 1;    /* [0] */
        unsigned int dl_rard_wptr_sec_n : 1;    /* [1] */
        unsigned int ef_sec_n : 1;              /* [2] */
        unsigned int dl_wdata_oam_sec_n : 1;    /* [3] */
        unsigned int dl_rdq_oam_wptr_sec_n : 1; /* [4] */
        unsigned int dl_rd_oam_sec_n : 1;       /* [5] */
        unsigned int reserved_0 : 26;           /* [31..6] */
    } bits;

    unsigned int u32;
} U_PFA_TFT_SEC_ATTR1;

/* Define the union U_PFA_TFT_PKT_LEN */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int min_pkt_len : 14; /* [13..0]  */
        unsigned int reserved_0 : 2;   /* [15..14]  */
        unsigned int max_pkt_len : 14; /* [29..16]  */
        unsigned int reserved_1 : 2;   /* [31..30]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} U_PFA_TFT_PKT_LEN;

/* Define the union U_PFA_TFT_CH0_BDQ0_DEPTH */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int ul_bdq0_depth : 15; /* [14..0]  */
        unsigned int reserved_0 : 17;    /* [31..15]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} U_PFA_TFT_CH0_BDQ0_DEPTH;

/* Define the union U_PFA_TFT_CH1_RDQ_DEPTH */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int dl_rdq_depth : 15; /* [14..0]  */
        unsigned int reserved_0 : 17;   /* [31..15]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} U_PFA_TFT_CH1_RDQ_DEPTH;

/* Define the union U_PFA_TFT_CH1_STATE */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int dl_bdq0_busy : 1;            /* [0] */
        unsigned int dl_bdq1_busy : 1;            /* [1] */
        unsigned int dl_bdq2_busy : 1;            /* [2] */
        unsigned int dl_rdq0_full : 1;            /* [3] */
        unsigned int dl_rdq1_full : 1;            /* [4] */
        unsigned int dl_rdq2_full : 1;            /* [5] */
        unsigned int dl_rdq_oam_full : 1;         /* [6] */
        unsigned int dl_rdq0_empty : 1;           /* [7] */
        unsigned int dl_rdq1_empty : 1;           /* [8] */
        unsigned int dl_rdq2_empty : 1;           /* [9] */
        unsigned int dl_rdq_oam_empty : 1;        /* [10] */
        unsigned int dl_bdq0_full : 1;            /* [11] */
        unsigned int dl_bdq1_full : 1;            /* [12] */
        unsigned int dl_bdq2_full : 1;            /* [13] */
        unsigned int dl_bdq0_empty : 1;           /* [14] */
        unsigned int dl_bdq1_empty : 1;           /* [15] */
        unsigned int dl_bdq2_empty : 1;           /* [16] */
        unsigned int dl_rdq0_rptr_invalid : 1;    /* [17] */
        unsigned int dl_rdq1_rptr_invalid : 1;    /* [18] */
        unsigned int dl_rdq2_rptr_invalid : 1;    /* [19] */
        unsigned int dl_rdq_oam_rptr_invalid : 1; /* [20] */
        unsigned int dl_bdq0_wptr_invalid : 1;    /* [21] */
        unsigned int dl_bdq1_wptr_invalid : 1;    /* [22] */
        unsigned int dl_bdq2_wptr_invalid : 1;    /* [23] */
        unsigned int dl_rnrdq_empty : 1;          /* [24] */
        unsigned int dl_rnrdq_full : 1;           /* [25] */
        unsigned int dl_rardq_empty : 1;          /* [26] */
        unsigned int dl_rardq_full : 1;           /* [27] */
        unsigned int dl_rnrdq_rptr_invalid : 1;   /* [28] */
        unsigned int dl_rardq_rptr_invalid : 1;   /* [29] */
        unsigned int reserved_0 : 2;              /* [31..30] */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} U_PFA_TFT_CH1_STATE;

/* Define the union U_PFA_TFT_CH1_ADQ_EN_CTRL */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int dl_adq_en : 2;       /* [1..0]  */
        unsigned int dl_ad_buf_flush : 1; /* [2]  */
        unsigned int reserved_0 : 29;     /* [31..3]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} U_PFA_TFT_CH1_ADQ_EN_CTRL;

/* Define the union U_PFA_TFT_CH1_ADQ_SIZE_CTRL */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int dl_adq0_size_sel : 3; /* [2..0]  */
        unsigned int dl_adq1_size_sel : 3; /* [5..3]  */
        unsigned int dl_adq_empty_th : 10; /* [15..6]  */
        unsigned int dl_adq_plen_th : 16;  /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} U_PFA_TFT_CH1_ADQ_SIZE_CTRL;

typedef union {
    struct {
        unsigned int ul_adq0_size_sel : 3; /* [2..0] */
        unsigned int ul_adq1_size_sel : 3; /* [5..3] */
        unsigned int ul_adq_empty_th : 10; /* [15..6] */
        unsigned int ul_adq_plen_th : 16;  /* [31..16] */
    } bits;
    unsigned int u32;
} U_PFA_TFT_CH0_ADQ_SIZE_CTRL;

typedef union {
    struct {
        unsigned int ul_adq_en : 2;       /* [1..0] */
        unsigned int ul_ad_buf_flush : 1; /* [2] */
        unsigned int reserved_0 : 29;     /* [31..3] */
    } bits;
    unsigned int u32;
} U_PFA_TFT_CH0_ADQ_EN_CTRL;

typedef union {
    struct {
        unsigned int ul_rdq0_clr : 1; /* [0] */
        unsigned int ul_rdq1_clr : 1; /* [1] */
        unsigned int dl_rdq2_clr : 1; /* [2] */
        unsigned int dl_bdq0_clr : 1; /* [3] */
        unsigned int dl_bdq1_clr : 1; /* [4] */
        unsigned int dl_bdq2_clr : 1; /* [5] */
        unsigned int ul_bdq1_clr : 1; /* [6] */
        unsigned int reserved : 1;    /* [7..31] */
    } bits;

    unsigned int u32;
} U_PFA_TFT_SEC_AREA_CTRL;

typedef union {
    struct {
        unsigned int ul_bdq0_busy : 1;         /* [0] */
        unsigned int ul_bdq1_busy : 1;         /* [1] */
        unsigned int ul_bdq2_busy : 1;         /* [2] */
        unsigned int reserved_1 : 1;           /* [3] */
        unsigned int ul_rdq0_full : 1;         /* [4] */
        unsigned int ul_rdq0_empty : 1;        /* [5] */
        unsigned int ul_bdq0_full : 1;         /* [6] */
        unsigned int ul_bdq0_empty : 1;        /* [7] */
        unsigned int ul_rdq1_full : 1;         /* [8] */
        unsigned int ul_rdq1_empty : 1;        /* [9] */
        unsigned int ul_bdq1_full : 1;         /* [10] */
        unsigned int ul_bdq1_empty : 1;        /* [11] */
        unsigned int ul_bdq2_full : 1;         /* [12] */
        unsigned int ul_bdq2_empty : 1;        /* [13] */
        unsigned int ul_rdq0_rptr_invalid : 1; /* [14] */
        unsigned int ul_rdq1_rptr_invalid : 1; /* [15] */
        unsigned int ul_bdq0_wptr_invalid : 1; /* [16] */
        unsigned int ul_bdq1_wptr_invalid : 1; /* [17] */
        unsigned int ul_bdq2_wptr_invalid : 1; /* [18] */
        unsigned int reserved_0 : 13;          /* [31..19] */
    } bits;
    unsigned int u32;
} U_PFA_TFT_CH0_STATE;
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int ul_bdq1_depth : 15; /* [14..0] */
        unsigned int reserved_0 : 17;    /* [31..15] */
    } bits;
    unsigned int u32;
} U_PFA_TFT_CH0_BDQ1_DEPTH;

/* Define the union U_PFA_TFT_CH1_BDQ1_DEPTH */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int dl_bdq1_depth : 15; /* [14..0] */
        unsigned int reserved_0 : 17;    /* [31..15] */
    } bits;

    unsigned int u32;
} U_PFA_TFT_CH1_BDQ1_DEPTH;

/* Define the union U_PFA_TFT_CH1_RDQ2_DEPTH */
typedef union {
    struct {
        unsigned int dl_rdq2_depth : 15; /* [14..0] */
        unsigned int reserved_0 : 17;    /* [31..15] */
    } bits;
    unsigned int u32;
} U_PFA_TFT_CH1_RDQ2_DEPTH;

/* Define the union U_PFA_TFT_CH_RDQ_SIZE */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int rdq_size : 14;          /* [13..0]  */
        unsigned int rdq0_amst_full_th : 14; /* [27..14]  */
        unsigned int reserved_0 : 4;         /* [31..28]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} U_PFA_TFT_CH_RDQ_SIZE;

#endif  // __MODULE_NAME_REG_OFFSET_H__
