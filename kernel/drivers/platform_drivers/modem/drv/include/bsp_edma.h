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
#ifndef _BSP_EDMA_H_
#define _BSP_EDMA_H_

#include "product_config.h"
#include "osl_types.h"
#include "mdrv_edma.h"
#include "bsp_print.h"

#include <linux/types.h>

#ifdef __cplusplus /* __cplusplus */
extern "C" {
#endif /* __cplusplus */

typedef dma_addr_t edma_addr_t;

#define MODEM_DMA_INT_DONE 1
#define MODEM_DMA_INT_LLT_DONE 2
#define MODEM_DMA_INT_CONFIG_ERR 4
#define MODEM_DMA_INT_TRANSFER_ERR 8
#define MODEM_DMA_INT_READ_ERR 16

#define MODEM_DMA_P2M 1
#define MODEM_DMA_M2P 2
#define MODEM_DMA_M2M 3

#define EDMA_CHN_FREE 1
#define EDMA_CHN_BUSY 0

#define EDMA_TRANS_WIDTH_8 0x0
#define EDMA_TRANS_WIDTH_16 0x1
#define EDMA_TRANS_WIDTH_32 0x2
#define EDMA_TRANS_WIDTH_64 0x3

#define EDMA_BUR_LEN_1 0x0
#define EDMA_BUR_LEN_2 0x1
#define EDMA_BUR_LEN_3 0x2
#define EDMA_BUR_LEN_4 0x3
#define EDMA_BUR_LEN_5 0x4
#define EDMA_BUR_LEN_6 0x5
#define EDMA_BUR_LEN_7 0x6
#define EDMA_BUR_LEN_8 0x7
#define EDMA_BUR_LEN_9 0x8
#define EDMA_BUR_LEN_10 0x9
#define EDMA_BUR_LEN_11 0xa
#define EDMA_BUR_LEN_12 0xb
#define EDMA_BUR_LEN_13 0xc
#define EDMA_BUR_LEN_14 0xd
#define EDMA_BUR_LEN_15 0xe
#define EDMA_BUR_LEN_16 0xf


/* EDMAC_BASIC_CONFIG?MODEM_DMA_SET_LLI?MODEM_DMA_SET_CONFIG ?? */
/* config------Bit 31 */
#define EDMAC_TRANSFER_CONFIG_SOUR_INC (0X80000000)
/* Bit 30 */
#define EDMAC_TRANSFER_CONFIG_DEST_INC (0X40000000)
#define EDMAC_TRANSFER_CONFIG_BOTH_INC (0XC0000000)

/* Bit 27-24 */
#define EDMAC_TRANSFER_CONFIG_SOUR_BURST_LENGTH_MASK (0xF000000)
#define EDMAC_TRANSFER_CONFIG_SOUR_BURST_LENGTH(_len) ((unsigned int)((_len) << 24))
/* Bit 23-20 */
#define EDMAC_TRANSFER_CONFIG_DEST_BURST_LENGTH_MASK (0xF00000)
#define EDMAC_TRANSFER_CONFIG_DEST_BURST_LENGTH(_len) ((unsigned int)((_len) << 20))

/* Bit18-16 */
#define EDMAC_TRANSFER_CONFIG_SOUR_WIDTH_MASK (0x70000)
#define EDMAC_TRANSFER_CONFIG_SOUR_WIDTH(_len) ((unsigned int)((_len) << 16))
/* Bit14-12 */
#define EDMAC_TRANSFER_CONFIG_DEST_WIDTH_MASK (0x7000)
#define EDMAC_TRANSFER_CONFIG_DEST_WIDTH(_len) ((unsigned int)((_len) << 12))

/* Bit9-4 */
#define EDMAC_TRANSFER_CONFIG_REQUEST(_ulReg) ((_ulReg) << 4)
/* Bit3-2 */
#define EDMAC_TRANSFER_CONFIG_FLOW_DMAC(_len) ((unsigned int)((_len) << 2))

#define EDMAC_TRANSFER_CONFIG_INT_TC_ENABLE (0x2)
#define EDMAC_TRANSFER_CONFIG_INT_TC_DISABLE (0x0)

#define EDMAC_TRANSFER_CONFIG_CHANNEL_ENABLE (0x1)
#define EDMAC_TRANSFER_CONFIG_CHANNEL_DISABLE (0x0)
#define EDMAC_NEXT_LLI_ENABLE 0x2 /* Bit 1 */
/* Bit 15 */
#define EDMAC_TRANSFER_CONFIG_EXIT_ADD_MODE_A_SYNC (0UL)
#define EDMAC_TRANSFER_CONFIG_EXIT_ADD_MODE_AB_SYNC (0x00008000)

#define P2M_CONFIG (EDMAC_TRANSFER_CONFIG_FLOW_DMAC(MEM_PRF_DMA) | EDMAC_TRANSFER_CONFIG_DEST_INC)
#define M2P_CONFIG (EDMAC_TRANSFER_CONFIG_FLOW_DMAC(MEM_PRF_DMA) | EDMAC_TRANSFER_CONFIG_SOUR_INC)
#define M2M_CONFIG \
    (EDMAC_TRANSFER_CONFIG_FLOW_DMAC(MEM_MEM_DMA) | EDMAC_TRANSFER_CONFIG_SOUR_INC | EDMAC_TRANSFER_CONFIG_DEST_INC)

/*
 * old version MODEM_DMA_SET_LLI
 * new name same as bsp. should bsp.h
 */
#define EDMAC_MAKE_LLI_ADDR(_p) (u32)((u32)(_p)&0xFFFFFFE0)

enum edma_req_id {
    EDMA_REQ_START = 0,
    EDMA_SPI0_RX = 0,
    EDMA_SPI0_TX,
    EDMA_SPI1_RX,
    EDMA_SPI1_TX,   /* Acore drv lcd 刷屏 */
    EDMA_LTESIO_RX, /* HIFI */
    EDMA_LTESIO_TX, /* HIFI */
    RESERVE_0,
    RESERVE_1,
    EDMA_HSUART_RX = 8, /* Acore drv 蓝牙语音场景 */
    EDMA_HSUART_TX,     /* Acore drv 蓝牙语音场景 */
    EDMA_UART0_RX,
    EDMA_UART0_TX,
    EDMA_UART1_RX,
    EDMA_UART1_TX,
    EDMA_UART2_RX,
    EDMA_UART2_TX,
    EDMA_UART3_RX,
    EDMA_UART3_TX,
    EDMA_SCI0_RX = 0x10, /* Acore drv SIM卡1收 */
    EDMA_SCI_RX = 0x10,
    EDMA_SCI0_TX, /* Acore drv SIM卡1发 */
    EDMA_SCI1_RX, /* Acore drv SIM卡2收 */
    EDMA_SCI1_TX, /* Acore drv SIM卡2发 */

    EDMA_GBBP0_DBG = 0x14,
    EDMA_BBP_DBG = 0x14, /* gudsp 的请求 bbp搬数 */
    EDMA_GBBP1_GRIF = 0x15,
    EDMA_BBP_GRIF = 0x15, /* gudsp 的请求 bbp采数 */

    EDMA_AMON_SOC = 0x16,     /* Acore drv AXIMON */
    EDMA_AMON_CPUFAST = 0x17, /* Acore drv AXIMON  */
    EDMA_I2C0_RX,
    EDMA_I2C0_TX,
    EDMA_I2C1_RX,
    EDMA_I2C1_TX,
    EDMA_EMI_TRANS,           /* edma_ch16上支持,edma_ch4不支持 */
    /* gudsp 的请求 M2M */
    /* EDMA_GBBP0_DBG      gudsp使用 EDMA_BBP_DBG   实际分配通道 edma ch16 - 6  */
    EDMA_MEMORY_DSP_1 = 0x20, /* 用于turbo译码    实际分配通道 edma ch4  - 0  */
    EDMA_MEMORY_DSP_2,        /* 用于viterb译码   实际分配通道 edma ch4  - 1  */
    /* TL与GU复用 ch16 - 2，为保险每次启用前检查当前通道是否空闲 */
    EDMA_MEMORY_DSP_3,        /* 用于CQI表搬移    实际分配通道 edma ch16 - 2  */
    EDMA_MEMORY_DSP_4,        /* 用于上行编码     实际分配通道 edma ch4  - 2  */
    EDMA_MEMORY_DSP_5,        /* 用于APT表搬移    实际分配通道 edma ch4  - 3  */
    EDMA_MEMORY_DSP_6 = 0x25, /* GUDSP/AHB加载    实际分配通道 edma ch16 - 7  */

    EDMA_PWC_LDSP_TCM, /* Mcore  drv   实际分配通道 edma ch16 - 0  */
    EDMA_PWC_TDSP_TCM, /* Mcore  drv   实际分配通道 edma ch16 - 1  */
    /* TLDSP  的请求定义，没有实际用途 放到MAX前面用于测试0通道 */
    EDMA_LDSP_API_USED_0, /* LDSP加载专有镜像 实际分配通道 edma ch16 - 0  */
    EDMA_REQ_MAX, /* 如果设备请求不小于此值，则为非法请求 */

    /* TLDSP  的请求定义，没有实际用途 */
    EDMA_LDSP_API_USED_1, /* LDSP加载专有镜像 实际分配通道 edma ch16 - 1  */
    /* TL与GU复用 ch16 - 2，为保险每次启用前检查当前通道是否空闲 */
    EDMA_LDSP_LCS_SDR,    /* TLDSP LCS        实际分配通道 edma ch16 - 2  */
    EDMA_LDSP_CSU_SDR,    /* LDSP小区搜索     实际分配通道 edma ch16 - 3  */
    EDMA_LDSP_EMU_SDR,    /* LDSP能量测量     实际分配通道 edma ch16 - 4  */
    EDMA_LDSP_NV_LOADING, /* LDSP动态加载NV   实际分配通道 edma ch16 - 5  */
    EDMA_PCIE0_M2M_RX,
    EDMA_PCIE0_M2M_TX,
    EDMA_PCIE1_M2M_RX,
    EDMA_PCIE1_M2M_TX,
    EDMA_BUS_ANALYSE_1,
    EDMA_BUS_ANALYSE_2,
    EDMA_BUTT,
    AP_UART0_RX = 0,
    AP_UART0_TX = 1,
    AP_UART1_RX = 2,
    AP_UART1_TX = 3,
    AP_SPI1_RX = 10,
    AP_SPI1_TX = 11,
    AP_I2C3_RX = 14,
    AP_I2C3_TX = 15,
    AP_I2C4_RX = 16,
    AP_I2C4_TX = 17,

    AP_SIO2_RX = 6,
    AP_SIO2_TX = 7,
    AP_SIO0_RX = 18,
    AP_SIO0_TX = 19,
    AP_SIO1_RX = 20,
    AP_SIO1_TX = 21,
};

typedef enum edma_trans_type_e {
    MEM_MEM_DMA = 0x00,
    MEM_PRF_DMA = 0x01,
    MEM_PRF_PRF = 0x10
} EDMA_TRANS_TYPE_E;

typedef struct _MODEM_DMA_CB {
    volatile unsigned int lli;
    volatile unsigned int bindx;
    volatile unsigned int cindx;
    volatile unsigned int cnt1;
    volatile unsigned int cnt0;
    volatile unsigned int src_addr;
    volatile unsigned int des_addr;
    volatile unsigned int config;
} edma_cb_s __attribute__((aligned(32)));

typedef void (*channel_isr)(unsigned long channel_arg, unsigned int int_status);

#define mdrv_edma_basic_config(burst_width, burst_len)                                                         \
    (EDMAC_TRANSFER_CONFIG_SOUR_BURST_LENGTH(burst_len) | EDMAC_TRANSFER_CONFIG_DEST_BURST_LENGTH(burst_len) | \
        EDMAC_TRANSFER_CONFIG_SOUR_WIDTH(burst_width) | EDMAC_TRANSFER_CONFIG_DEST_WIDTH(burst_width))
#define EDMAC_BASIC_CONFIG(burst_width, burst_len) mdrv_edma_basic_config(burst_width, burst_len)

#define mdrv_edma_set_lli(addr, last) ((last) ? 0 : (EDMAC_MAKE_LLI_ADDR(addr) | EDMAC_NEXT_LLI_ENABLE))

/* 链式传输时的节点信息 */
struct edma_cb {
    volatile unsigned int lli; /* 指向下个LLI */
    volatile unsigned int bindx;
    volatile unsigned int cindx;
    volatile unsigned int cnt1;
    volatile unsigned int cnt0;     /* 块传输或者LLI传输的每个节点数据长度 <= 65535字节 */
    volatile unsigned int src_addr; /* 物理地址 */
    volatile unsigned int des_addr; /* 物理地址 */
    volatile unsigned int config;
} __attribute__((aligned(32)));

#define EDMA_CX_LLI (0x800)
#define EDMA_CX_CONFIG (0x81C)

/* 错误码定义 */
#ifndef OK
#define OK 0
#endif
#ifndef ERROR
#define ERROR (-1)
#endif

/* 错误码定义 */
#define DMAC_SUCCESS 0
#define DMA_FAIL (-1)

#define DMA_ERROR_BASE -100
#define DMA_CHANNEL_INVALID                     (DMA_ERROR_BASE - 1)
#define DMA_TRXFERSIZE_INVALID                  (DMA_ERROR_BASE - 2)
#define DMA_SOURCE_ADDRESS_INVALID              (DMA_ERROR_BASE - 3)
#define DMA_DESTINATION_ADDRESS_INVALID         (DMA_ERROR_BASE - 4)
#define DMA_MEMORY_ADDRESS_INVALID              (DMA_ERROR_BASE - 5)
#define DMA_PERIPHERAL_ID_INVALID               (DMA_ERROR_BASE - 6)
#define DMA_DIRECTION_ERROR                     (DMA_ERROR_BASE - 7)
#define DMA_TRXFER_ERROR                        (DMA_ERROR_BASE - 8)
#define DMA_LLIHEAD_ERROR                       (DMA_ERROR_BASE - 9)
#define DMA_SWIDTH_ERROR                        (DMA_ERROR_BASE - 0xa)
#define DMA_LLI_ADDRESS_INVALID                 (DMA_ERROR_BASE - 0xb)
#define DMA_TRANS_CONTROL_INVALID               (DMA_ERROR_BASE - 0xc)
#define DMA_MEMORY_ALLOCATE_ERROR               (DMA_ERROR_BASE - 0xd)
#define DMA_NOT_FINISHED                        (DMA_ERROR_BASE - 0xe)
#define DMA_CONFIG_ERROR                        (DMA_ERROR_BASE - 0xf)


/* 错误码定义 */
#define EDMA_SUCCESS DMAC_SUCCESS
#define EDMA_FAIL DMA_FAIL

#define EDMA_ERROR_BASE                     DMA_ERROR_BASE
#define EDMA_CHANNEL_INVALID                DMA_CHANNEL_INVALID
#define EDMA_TRXFERSIZE_INVALID             DMA_TRXFERSIZE_INVALID
#define EDMA_SOURCE_ADDRESS_INVALID         DMA_SOURCE_ADDRESS_INVALID
#define EDMA_DESTINATION_ADDRESS_INVALID    DMA_DESTINATION_ADDRESS_INVALID
#define EDMA_MEMORY_ADDRESS_INVALID         DMA_MEMORY_ADDRESS_INVALID
#define EDMA_PERIPHERAL_ID_INVALID          DMA_PERIPHERAL_ID_INVALID
#define EDMA_DIRECTION_ERROR                DMA_DIRECTION_ERROR
#define EDMA_TRXFER_ERROR                   DMA_TRXFER_ERROR
#define EDMA_LLIHEAD_ERROR                  DMA_LLIHEAD_ERROR
#define EDMA_SWIDTH_ERROR                   DMA_SWIDTH_ERROR
#define EDMA_LLI_ADDRESS_INVALID            DMA_LLI_ADDRESS_INVALID
#define EDMA_TRANS_CONTROL_INVALID          DMA_TRANS_CONTROL_INVALID
#define EDMA_MEMORY_ALLOCATE_ERROR          DMA_MEMORY_ALLOCATE_ERROR
#define EDMA_NOT_FINISHED                   DMA_NOT_FINISHED
#define EDMA_CONFIG_ERROR                   DMA_CONFIG_ERROR

/*
 * 函数void (*channel_isr)(u32 channel_arg, u32 int_status)的参数int_status、
 * 函数s32 bsp_edma_channel_init (enum edma_req_id req,  channel_isr pFunc,
 * u32 channel_arg, u32 int_flag)的参数int_flag 为以下几种中断类型，可组合
 */
#define EDMA_INT_DONE MODEM_DMA_INT_DONE                 /* EDMA传输完成中断 */
#define EDMA_INT_LLT_DONE MODEM_DMA_INT_LLT_DONE         /* 链式EDMA节点传输完成中断 */
#define EDMA_INT_CONFIG_ERR MODEM_DMA_INT_CONFIG_ERR     /* EDMA配置错误导致的中断 */
#define EDMA_INT_TRANSFER_ERR MODEM_DMA_INT_TRANSFER_ERR /* EDMA传输错误导致的中断 */
#define EDMA_INT_READ_ERR MODEM_DMA_INT_READ_ERR         /* EDMA链表读错误导致的中断 */
#define EDMA_INT_ALL 0x1F

/* EDMAC传输方向定义 */
#define EDMA_P2M MODEM_DMA_P2M
#define EDMA_M2P MODEM_DMA_M2P
#define EDMA_M2M MODEM_DMA_M2M

#define EDMA_GET_DEST_ADDR 0
#define EDMA_GET_SOUR_ADDR 1

#define EDMA_DATA_TIMEOUT 10

/* EDMAC流控制与传输类型 */
#define MEM_MEM_EDMA MEM_MEM_DMA /* 内存到内存，DMA流控 */
#define MEM_PRF_EDMA MEM_PRF_DMA /* 内存与外设，DMA流控 */

/*
 * EDMA 对应的具体位   供EDMA 寄存器配置宏
 * EDMAC_BASIC_CONFIG、EDMA_SET_LLI、EDMA_SET_CONFIG 使用
 */
/* Bit3-2 */
#define EDMAC_TRANSFER_CONFIG_FLOW_EDMAC(_len) EDMAC_TRANSFER_CONFIG_FLOW_DMAC(_len)

/* addr:物理地址 */
#define EDMA_SET_LLI(addr, last) mdrv_edma_set_lli(addr, last)

#define MODEM_DMA_SET_CONFIG(req, direction, burst_width, burst_len)                                     \
    (EDMAC_BASIC_CONFIG(burst_width, burst_len) | EDMAC_TRANSFER_CONFIG_REQUEST(req) |                    \
        EDMAC_TRANSFER_CONFIG_INT_TC_ENABLE | EDMAC_TRANSFER_CONFIG_CHANNEL_ENABLE /*lint -save -e506*/ | \
        ((direction == MODEM_DMA_M2M) ? M2M_CONFIG :                                                     \
                                         ((direction == MODEM_DMA_P2M) ? P2M_CONFIG : M2P_CONFIG))) /*lint -restore*/

#define EDMA_SET_CONFIG(req, direction, burst_width, burst_len) \
    MODEM_DMA_SET_CONFIG(req, direction, burst_width, burst_len)

#define EDMA_CHANNEL_DISABLE EDMAC_TRANSFER_CONFIG_CHANNEL_DISABLE
#define EDMA_CHANNEL_ENABLE EDMAC_TRANSFER_CONFIG_CHANNEL_ENABLE
#define EDMA_NEXT_LLI_ENABLE EDMAC_NEXT_LLI_ENABLE

struct EDMA_LLI_ALLOC_ADDRESS_STRU {
    void *s_alloc_virt_address;
    void *s_alloc_phys_address;
};

#ifndef EDMAC_MAKE_LLI_ADDR
/* for acore,cause c_mdrv defined */
#define EDMAC_MAKE_LLI_ADDR(_p) (u32)((u32)(_p) & 0xFFFFFFE0)
#endif

#ifdef CONFIG_EDMA_SUPPORT
s32 bsp_edma_channel_init(enum edma_req_id request, channel_isr pFunc, unsigned long channel_arg, u32 int_flag);
s32 bsp_edma_current_transfer_address(u32 channel_id);
u64 bsp_edma_current_cnt(u32 channel_id);
s32 bsp_edma_channel_stop(u32 channel_id);
s32 bsp_edma_channel_is_idle(u32 channel_id);
s32 bsp_edma_channel_set_config(u32 channel_id, u32 direction, u32 burst_width, u32 burst_len);
s32 bsp_edma_channel_dest_set_config(u32 channel_id, u32 dest_width, u32 dest_len);
s32 bsp_edma_channel_start(u32 channel_id, u32 src_addr, u32 des_addr, u32 len);
s32 bsp_edma_channel_2vec_start(u32 channel_id, u32 src_addr, u32 des_addr, u32 len, u32 size_align);
s32 bsp_edma_channel_async_start(u32 channel_id, u32 src_addr, u32 des_addr, u32 len);
struct edma_cb *bsp_edma_channel_get_lli_addr(u32 channel_id);
s32 bsp_edma_channel_lli_start(u32 channel_id);
s32 bsp_edma_channel_lli_async_start(u32 channel_id);
s32 bsp_edma_channel_free(u32 channel_id);
#else
#define EDMA_STUB_RES 0

static inline s32 bsp_edma_channel_init(enum edma_req_id request, channel_isr pFunc, unsigned long channel_arg, u32 int_flag)
{
    return EDMA_STUB_RES;
}
static inline s32 bsp_edma_current_transfer_address(u32 channel_id)
{
    return EDMA_STUB_RES;
}
static inline u64 bsp_edma_current_cnt(u32 channel_id)
{
    return EDMA_STUB_RES;
}
static inline s32 bsp_edma_channel_stop(u32 channel_id)
{
    return EDMA_STUB_RES;
}
static inline s32 bsp_edma_channel_is_idle(u32 channel_id)
{
    return EDMA_STUB_RES;
}
static inline s32 bsp_edma_channel_set_config(u32 channel_id, u32 direction, u32 burst_width, u32 burst_len)
{
    return EDMA_STUB_RES;
}
static inline s32 bsp_edma_channel_dest_set_config(u32 channel_id, u32 dest_width, u32 dest_len)
{
    return EDMA_STUB_RES;
}
static inline s32 bsp_edma_channel_start(u32 channel_id, u32 src_addr, u32 des_addr, u32 len)
{
    return EDMA_STUB_RES;
}
static inline s32 bsp_edma_channel_2vec_start(u32 channel_id, u32 src_addr, u32 des_addr, u32 len, u32 size_align)
{
    return EDMA_STUB_RES;
}
static inline s32 bsp_edma_channel_async_start(u32 channel_id, u32 src_addr, u32 des_addr, u32 len)
{
    return EDMA_STUB_RES;
}
static inline struct edma_cb *bsp_edma_channel_get_lli_addr(u32 channel_id)
{
    return NULL;
}
static inline s32 bsp_edma_channel_lli_start(u32 channel_id)
{
    return EDMA_STUB_RES;
}
static inline s32 bsp_edma_channel_lli_async_start(u32 channel_id)
{
    return EDMA_STUB_RES;
}
static inline s32 bsp_edma_channel_free(u32 channel_id)
{
    return EDMA_STUB_RES;
}

#endif

#ifdef __cplusplus /* __cplusplus */
}
#endif /* __cplusplus */

#endif /* End of DMADRV_H */
