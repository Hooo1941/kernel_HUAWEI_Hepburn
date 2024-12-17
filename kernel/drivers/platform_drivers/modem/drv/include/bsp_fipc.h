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
#ifndef __BSP_FIPC_H__
#define __BSP_FIPC_H__

#include <osl_types.h>

#ifdef __cplusplus /* __cplusplus */
extern "C" {
#endif /* __cplusplus */

/**
 * @brief fipc句柄类型，用于屏蔽不同平台差异
 */
typedef unsigned fipc_chn_t;

/**
 * @brief fipc句柄类型无效值，应该使用此值对fipc_chn_t初始化
 */
#define FIPC_CHN_INVAILD_HDL (0xFFFFFFFFU)

/** @brief 错误码：当前资源不足，请重试 */
#define FIPC_ERR_EAGAIN (-11)

/**
 * @brief fipc通道号，用于打开通道时的标识 fipc_chn_attr_t.chnid
 */
enum fipc_chn_id {
    /* LPM3<-->AP */
    FIPC_CHN_SEND_LPM2AP_MDRV_MSG = 0x00,
    FIPC_CHN_RECV_LPM2AP_MDRV_MSG = 0x01,
    FIPC_CHN_SEND_AP2LPM_MDRV_MSG = 0x02,
    FIPC_CHN_RECV_AP2LPM_MDRV_MSG = 0x03,
    /* LPM3<-->TEEOS */
    FIPC_CHN_SEND_LPM2TEE_MDRV_MSG = 0x04,
    FIPC_CHN_RECV_LPM2TEE_MDRV_MSG = 0x05,
    FIPC_CHN_SEND_TEE2LPM_MDRV_MSG = 0x06,
    FIPC_CHN_RECV_TEE2LPM_MDRV_MSG = 0x07,

    /* TSP<-->LPM3 */
    FIPC_CHN_SEND_TSP2LPM_MDRV_MSG = 0x10,
    FIPC_CHN_RECV_TSP2LPM_MDRV_MSG = 0x11,
    FIPC_CHN_SEND_LPM2TSP_MDRV_MSG = 0x12,
    FIPC_CHN_RECV_LPM2TSP_MDRV_MSG = 0x13,
    /* TSP<-->HIFI */
    FIPC_CHN_SEND_TSP2HIFI_MDRV_MSG = 0x14,
    FIPC_CHN_RECV_TSP2HIFI_MDRV_MSG = 0x15,
    FIPC_CHN_SEND_HIFI2TSP_MDRV_MSG = 0x16,
    FIPC_CHN_RECV_HIFI2TSP_MDRV_MSG = 0x17,
    /* TSP<-->AP */
    FIPC_CHN_SEND_TSP2AP_MDRV_MSG = 0x18,
    FIPC_CHN_RECV_TSP2AP_MDRV_MSG = 0x19,
    FIPC_CHN_SEND_AP2TSP_MDRV_MSG = 0x1A,
    FIPC_CHN_RECV_AP2TSP_MDRV_MSG = 0x1B,
    /* TSP<-->TEEOS */
    FIPC_CHN_SEND_TSP2TEE_MDRV_MSG = 0x1C,
    FIPC_CHN_RECV_TSP2TEE_MDRV_MSG = 0x1D,
    FIPC_CHN_SEND_TEE2TSP_MDRV_MSG = 0x1E,
    FIPC_CHN_RECV_TEE2TSP_MDRV_MSG = 0x1F,
    /* TSP<-->TVP */
    FIPC_CHN_SEND_TSP2TVP_MDRV_MSG = 0x20,
    FIPC_CHN_RECV_TSP2TVP_MDRV_MSG = 0x21,
    FIPC_CHN_SEND_TVP2TSP_MDRV_MSG = 0x22,
    FIPC_CHN_RECV_TVP2TSP_MDRV_MSG = 0x23,

    /* TSP<-->AP */
    FIPC_CHN_SEND_TSP2AP_MDRVRFILE = 0x24,
    FIPC_CHN_RECV_TSP2AP_MDRVRFILE = 0x25,
    FIPC_CHN_SEND_AP2TSP_MDRVRFILE = 0x26,
    FIPC_CHN_RECV_AP2TSP_MDRVRFILE = 0x27,

    FIPC_CHN_SEND_TSP2AP_MDRVNV = 0x28,
    FIPC_CHN_RECV_TSP2AP_MDRVNV = 0x29,
    FIPC_CHN_SEND_AP2TSP_MDRVNV = 0x2A,
    FIPC_CHN_RECV_AP2TSP_MDRVNV = 0x2B,

    /* AP<-->HIFI */
    FIPC_CHN_SEND_AP2HIFI_MDRV_MSG = 0x30,
    FIPC_CHN_RECV_AP2HIFI_MDRV_MSG = 0x31,
    FIPC_CHN_SEND_HIFI2AP_MDRV_MSG = 0x32,
    FIPC_CHN_RECV_HIFI2AP_MDRV_MSG = 0x33,

    FIPC_CHN_SEND_DMA_MDRVDRA0 = 0x40,
    FIPC_CHN_SEND_DMA_MDRVDRA1 = 0x42,
    FIPC_CHN_SEND_DMA_MDRVDRA2 = 0x44,
    FIPC_CHN_SEND_DMA_MDRVDRA3 = 0x46,

    FIPC_CHN_SEND_DMA_MDRVPMSR = 0x48,
    FIPC_CHN_SEND_DMA_PERI0 = 0x49,
    FIPC_CHN_SEND_DMA_PERI1 = 0x4A,
    FIPC_CHN_SEND_DMA_PERI2 = 0x4B,
    FIPC_CHN_SEND_DMA_PERI3 = 0x4C,
    FIPC_CHN_SEND_DMA_PERI4 = 0x4D,
    FIPC_CHN_SEND_DMA_PERI5 = 0x4E,
    FIPC_CHN_SEND_DMA_MEM0 = 0x50,
    FIPC_CHN_SEND_DMA_PERI6 = 0x51,
    FIPC_CHN_SEND_DMA_PERI7 = 0x52,
    FIPC_CHN_SEND_DMA_PERI8 = 0x53,
    FIPC_CHN_SEND_DMA_PERI9 = 0x54,
    FIPC_CHN_SEND_DMA_PERI10 = 0x55,

    /* fipc chn for phy */
    FIPC_CHN_SEND_TSP2TVP_PHY0 = 0x80,
    FIPC_CHN_RECV_TSP2TVP_PHY0 = 0x81,
    FIPC_CHN_SEND_TSP2TVP_PHY1 = 0x82,
    FIPC_CHN_RECV_TSP2TVP_PHY1 = 0x83,
    FIPC_CHN_SEND_TSP2TVP_PHY2 = 0x84,
    FIPC_CHN_RECV_TSP2TVP_PHY2 = 0x85,
    FIPC_CHN_SEND_TSP2TVP_PHY3 = 0x86,
    FIPC_CHN_RECV_TSP2TVP_PHY3 = 0x87,

    FIPC_CHN_SEND_TVP2TSP_PHY0 = 0x88,
    FIPC_CHN_RECV_TVP2TSP_PHY0 = 0x89,
    FIPC_CHN_SEND_TVP2TSP_PHY1 = 0x8A,
    FIPC_CHN_RECV_TVP2TSP_PHY1 = 0x8B,
    FIPC_CHN_SEND_TVP2TSP_PHY2 = 0x8C,
    FIPC_CHN_RECV_TVP2TSP_PHY2 = 0x8D,
    FIPC_CHN_SEND_TVP2TSP_PHY3 = 0x8E,
    FIPC_CHN_RECV_TVP2TSP_PHY3 = 0x8F,

    FIPC_CHN_SEND_TSP2TVP_PHY4 = 0x90,
    FIPC_CHN_RECV_TSP2TVP_PHY4 = 0x91,
    FIPC_CHN_SEND_TSP2TVP_PHY5 = 0x92,
    FIPC_CHN_RECV_TSP2TVP_PHY5 = 0x93,

    FIPC_CHN_SEND_TVP2TSP_PHY4 = 0x98,
    FIPC_CHN_RECV_TVP2TSP_PHY4 = 0x99,
    FIPC_CHN_SEND_TVP2TSP_PHY5 = 0x9A,
    FIPC_CHN_RECV_TVP2TSP_PHY5 = 0x9B,

    /* TSP<-->TEEOS VSIM */
    FIPC_CHN_SEND_TSP2TEE_MDRV_VSIM = 0x100,
    FIPC_CHN_RECV_TSP2TEE_MDRV_VSIM = 0x101,
    FIPC_CHN_SEND_TEE2TSP_MDRV_VSIM = 0x102,
    FIPC_CHN_RECV_TEE2TSP_MDRV_VSIM = 0x103,

    FIPC_CHN_SEND_DMA_PS0 = 0x140,
    FIPC_CHN_SEND_DMA_PS1 = 0x142,
    FIPC_CHN_SEND_DMA_PS2 = 0x144,
    FIPC_CHN_SEND_DMA_PS3 = 0x146,
    FIPC_CHN_SEND_DMA_PS4 = 0x148,
    FIPC_CHN_SEND_DMA_PS5 = 0x14A,
    FIPC_CHN_SEND_DMA_PS6 = 0x14C,
    FIPC_CHN_SEND_DMA_PS7 = 0x14E,

    /* AP<-->DPA*/
    FIPC_CHN_RECV_DPA2AP_MDRV_MSG = 0x150,
    FIPC_CHN_SEND_AP2DPA_MDRV_MSG = 0x151,

    /* support for V2X*/
    FIPC_CHN_SEND_AP2TSP_V2X = 0x160,
    FIPC_CHN_RECV_AP2TSP_V2X = 0x161,
    FIPC_CHN_SEND_TSP2AP_V2X = 0x162,
    FIPC_CHN_RECV_TSP2AP_V2X = 0x163,
    FIPC_CHN_SEND_DMA_V2X = 0x164,

    /* fipc chn for test */
    FIPC_CHN_SEND_TSP2TVP_MDRV_LLT = 0xFFFF0000,
    FIPC_CHN_RECV_TSP2TVP_MDRV_LLT = 0xFFFF0001,
    FIPC_CHN_SEND_TVP2TSP_MDRV_LLT = 0xFFFF0002,
    FIPC_CHN_RECV_TVP2TSP_MDRV_LLT = 0xFFFF0003,

    FIPC_CHN_SEND_TSP2AP_MDRV_LLT = 0xFFFF0010,
    FIPC_CHN_RECV_TSP2AP_MDRV_LLT = 0xFFFF0011,
    FIPC_CHN_SEND_AP2TSP_MDRV_LLT = 0xFFFF0012,
    FIPC_CHN_RECV_AP2TSP_MDRV_LLT = 0xFFFF0013,

    FIPC_CHN_INVAILD = 0xFFFFFFFFUL
};

/**
 * @brief fipc发送通道类型，用于打开通道时fipc_chn_attr_t.type
 */
#define FIPC_CHN_TYPE_SEND 0x0
/**
 * @brief fipc接收通道类型，用于打开通道时fipc_chn_attr_t.type
 */
#define FIPC_CHN_TYPE_RECV 0x1
/**
 * @brief fipc无效通道类型，用于打开通道时fipc_chn_attr_t.type
 */
#define FIPC_CHN_TYPE_INVAILD 0xFFFFFFFFUL

/**
 * @brief fipc回调事件类型
 */
typedef enum {
    FIPC_EVENT_DATA_ARRV,
    FIPC_EVENT_SEND_DONE,
    FIPC_EVENT_SEND_FLOWUP,
    FIPC_EVENT_SEND_FLOWDN,
    FIPC_EVENT_INVAILD,
} fipc_event;
/**
 * @brief fipc回调事件额外参数
 */
typedef union {
    struct {
        unsigned chnid;
        fipc_chn_t chn_hdl;
    } data_arrv; /* used for give information for event FIPC_EVENT_DATA_ARRV */
} fipc_eventinfo;
/**
 * @brief fipc回调
 * @attention
 * <ul><li>此回调直接执行在中断上下文</li></ul>
 * @param[in]  event, 此次回调的事件类型
 * @param[in]  cbk_arg, 用户私有参数，此参数来自于open通道时用户传入
 * @param[in]  info, 此次回调所带的额外信息
 *
 * @retval 0,回调正确处理。
 * @retval 非0,回调未正确处理。
 *
 * @see mdrv_fipc_chn_open
 */
typedef int (*fipc_cbk)(fipc_event event, void *cbk_arg, const fipc_eventinfo *info);
typedef int (*fipc_irq_wakeup_cbk)(void *cbk_arg);

/** @brief 默认情况下, 数据到达，接收方总是有通知的，该标志可以关闭该通知 */
#define FIPC_ATTR_FLAGS_NOARRVEVENT (1U << 0)
/** @brief 设置该标记的通道，DPM低功耗时不管理(这种通道必须是DMA通道) */
#define FIPC_ATTR_FLAGS_DPMSRIGNORE (1U << 4)
/** @brief 设置该标记，表示DMA搬移完成需要上报中断 */
#define FIPC_ATTR_FLAGS_SENDDONEEN (1U << 5)
/** @brief 设置该标记，表示支持DMA外设搬移 */
#define FIPC_ATTR_FLAGS_PERITRANS (1U << 6)
#define FIPC_ATTR_FLAGS_TIMEOUTINTEN (1U << 8)
/** @brief 设置该标记的通道，低功耗管理在特定级别(需要硬件资源支持，使用该标记请与开发者确认) */
#define FIPC_ATTR_FLAGS_DSSCTRLSR (1U << 31)
/**
 * @brief 用户打开通道时的输入参数
 * @attention
 * <ul><li>该结构体在使用前,必须使用fipc_chn_attr_init初始化</li></ul>
 * @see mdrv_fipc_chn_attr_init mdrv_fipc_chn_open
 */
typedef struct {
    unsigned chnid;        /**< 通道的id定义, 在attr_init后用户填写 */
    unsigned type;         /**< 通道收发属性，例如FIPC_CHN_TYPE_SEND ,在attr_init后用户填写 */
    unsigned long long pa; /**< ringbuf物理地址，硬件有对齐要求 8Bytes, 在attr_init后用户填写 */
    void *va;              /**< ringbuf虚拟地址,在attr_init后用户填写 */
    unsigned size;         /**< ringbuf大小，硬件有对齐要求 128Bytes,在attr_init后用户填写 */
    fipc_cbk cbk;          /**< fipc事件发生时的回调函数, 在attr_init后用户填写*/
    void *cbk_arg;         /**< 回调时，透传的用户私有数据，用户需要保证cbk_arg的生命周期,在attr_init后用户填写 */
    unsigned req_sel;      /**< 外设请求选择编号 */
    unsigned flags;        /**< 一些标志，比如唤醒，报中断等, 不清楚用处的在attr_init后不要操作 */
    fipc_irq_wakeup_cbk irq_wakeup_cbk; /**< fipc识别acore唤醒源后执行的回调, 在attr_init后用户填写*/
    void *irq_cbk_arg;  /**< 识别acore唤醒源后执行回调时，透传的用户私有数据，用户需要保证irq_cbk_arg的生命周期,在attr_init后用户填写 */
    unsigned flup;
    unsigned fldn;
    unsigned timeout;
} fipc_chn_attr_t;


typedef struct {
    unsigned cnt;
    struct {
        void *buf;
        unsigned len;
    } datablk[0];
} fipc_blk_desc_t;

typedef struct {
    unsigned cnt;
    struct {
        void *buf;
        unsigned len;
    } datablk[0x2];
} fipc_blkx2_desc_t;

typedef struct {
    unsigned cnt;
    struct {
        void *buf;
        unsigned len;
    } datablk[0x3];
} fipc_blkx3_desc_t;

#define FIPC_SEND_FLAGS_NOARRVNOTIFY (1UL << 0)
#define FIPC_SEND_FLAGS_NODONENOTIFY (1UL << 1)

#define FIPC_RECV_FLAGS_DATAPEEK (1UL << 0)

#define FIPC_IOCTL_CHN_NXTPKT_INF 0x40000002
#define FIPC_IOCTL_CHN_PACKET_SKIP 0x40000003
#define FIPC_IOCTL_CHN_CAPSET_INF 0x40000004
#define FIPC_IOCTL_CHN_STATUS_INF 0x40000005
#define FIPC_IOCTL_CHN_HALT_INF 0x40000006
#define FIPC_IOCTL_CHN_DMACMD_INF 0x40000007
#define FIPC_IOCTL_CHN_CURADDR_INF 0x40000008

typedef struct {
    u32 len;
    u32 pkthdl;
} ioctl_nxtpkt;

struct fipc_chn_status {
    u32 busy;
};

int bsp_fipc_init(void);
/************************************************************************
 * 函 数 名  : bsp_fipc_chn_attr_init
 * 功能描述  : pattr用于描述打开通道的各种参数，该函数用于实现将pattr设置为默认值
 * 输入参数  :
 *            pattr: fipc通道属性描述
 * 输出参数  : 无
 * 返 回 值  :  0          操作成功。
 *             其他        操作失败。
 **************************************************************************/
int bsp_fipc_chn_attr_init(fipc_chn_attr_t *attr);  // for extend

/************************************************************************
 * 函 数 名  : bsp_fipc_chn_open
 * 功能描述  : 根据pattr的描述，打开通道，通道必须打开之后才能进行数据收发
 *             需要先调用bsp_fipc_chn_attr_init对pattr初始化，pattr必须设置的字段参考pattr介绍
 * 输入参数  :
 *            pattr: fipc通道属性
 * 输出参数  :
 *            pchn_hdl:  fipc通道句柄，用于调用其他函数
 * 返 回 值  :  0          操作成功。
 *             其他        操作失败。
 **************************************************************************/
int bsp_fipc_chn_open(fipc_chn_t *pchn_hdl, fipc_chn_attr_t *attr);

/************************************************************************
 * 函 数 名	: bsp_fipc_chn_send
 * 功能描述  : 将一段buf直接发送到目的端
 * 输入参数  :
 *			  chn_hdl: fipc通道句柄，由open通道时得到
 *			  buf: 要发送的数据地址
 *			  len: 要发送的数据长度
 *			  flags: FIPC_SEND_FLAGS，不了解的填0
 * 输出参数  :
 *			  无
 * 返 回 值	:  成功返回实际发送长度，失败返回负值。
 **************************************************************************/
int bsp_fipc_chn_send(fipc_chn_t chn_hdl, void *buf, unsigned len, u32 flags);

/************************************************************************
 * 函 数 名	: bsp_fipc_chn_blks_send
 * 功能描述  : 将多段buf组合直接发送到目的端
 * 输入参数  :
 *			  chn_hdl: fipc通道句柄，由open通道时得到
 *			  blkdesc: 要发送的数据地址,长度等信息
 *			  flags: FIPC_SEND_FLAGS，不了解的填0
 * 输出参数  :
 *			  无
 * 返 回 值	:  成功返回实际发送长度，失败返回负值。
 **************************************************************************/
int bsp_fipc_chn_blks_send(fipc_chn_t chn_hdl, fipc_blk_desc_t *blkdesc, u32 flags);

/************************************************************************
 * 函 数 名	: bsp_fipc_chn_recv
 * 功能描述  : 接收数据
 * 输入参数  :
 *			  chn_hdl: fipc通道句柄，由open通道时得到
 *			  buf: 用于接收数据的buf
 *			  len: 用于接收数据的buf的大小
 *			  flags: FIPC_RECV_FLAGS，不了解的填0
 * 输出参数  :
 *			  无
 * 返 回 值	:  成功实际接收数据长度(若没有数据，返回0)，失败返回负值(比如参数错误等)。
 **************************************************************************/
int bsp_fipc_chn_recv(fipc_chn_t chn_hdl, void *buf, unsigned len, u32 flags);

/************************************************************************
 * 函 数 名	: bsp_fipc_chn_ioctl
 * 功能描述  : arg根据req是输入或者输出
 * 输入参数  :
 *			  chn_hdl: fipc通道句柄，由open通道时得到
 *			  req: 请求命令字 FIPC_IOCTL_CHN*
 *			  arg: 请求命令字对应的输入参数
 *			  size: arg的大小
 * 输出参数  :
 *			  arg: 请求命令字对应的输出参数
 * 返 回 值	:  成功返回0，失败返回负值。
 **************************************************************************/
int bsp_fipc_chn_ioctl(fipc_chn_t chn_hdl, unsigned req, void *arg, u32 size);

/************************************************************************
 * 函 数 名	: bsp_fipc_chn_before_resetting
 * 功能描述  : 单独复位前FIPC通道的处理
 * 输入参数  :
 *			  无
 * 输出参数  :
 *			  无
 * 返 回 值	:  成功返回0，失败返回负值。
 **************************************************************************/
int bsp_fipc_chn_before_resetting(void);

/************************************************************************
 * 函 数 名	: bsp_fipc_chn_after_resetting
 * 功能描述  : 单独复位后FIPC通道的处理
 * 输入参数  :
 *			  无
 * 输出参数  :
 *			  无
 * 返 回 值	:  成功返回0，失败返回负值。
 **************************************************************************/
int bsp_fipc_chn_after_resetting(void);

/************************************************************************
 * 函 数 名	: bsp_fipc_chn_blk2ring_send
 * 功能描述  : 将一段buf直接发送到目的端
 * 输入参数  :
 *			  chn_hdl: fipc通道句柄，由open通道时得到
 *			  buf: 要发送的数据地址
 *			  len: 要发送的数据长度
 *			  flags: FIPC_SEND_FLAGS，按照bit位控制
 *                   FIPC_SEND_FLAGS_NOARRVNOTIFY，不产生数据到达中断
 *                   FIPC_SEND_FLAGS_NODONENOTIFY，不产生搬移完成中断
 * 输出参数  :
 *			  无
 * 返 回 值	:  成功返回实际发送长度，失败返回负值。
 **************************************************************************/
int bsp_fipc_chn_blk2ring_send(fipc_chn_t chn_hdl, void *buf, unsigned len, u32 flags);

/************************************************************************
 * 函 数 名	: bsp_fipc_chn_ringbuf_rptr_set
 * 功能描述  : 更新接收通道的ringbuf的读指针，多线程时注意进行锁保护
 * 输入参数  :
 *			  chn_hdl: fipc通道句柄，由open通道时得到
 *			  rptr: 待更新rptr值
 * 输出参数  :
 *			  无
 * 返 回 值	:  0，更新成功；其他，更新失败。
 **************************************************************************/
int bsp_fipc_chn_ringbuf_rptr_set(fipc_chn_t chn_hdl, unsigned int rptr);

/************************************************************************
 * 函 数 名	: bsp_fipc_recv_chn_wptr_get
 * 功能描述  : 接收端获取接收通道的ringbuf的写指针
 * 输入参数  :
 *			  chn_hdl: fipc通道句柄，由open通道时得到
 * 输出参数  :
 *			  wptr：通道当前的写指针
 * 返 回 值	:  0，更新成功；其他，更新失败。
 * 注意：不可以针对发送通道使用
 **************************************************************************/
int bsp_fipc_chn_wptr_get(fipc_chn_t chn_hdl, unsigned int *wptr);

/************************************************************************
 * 函 数 名	: bsp_fipc_chn_rptr_get
 * 功能描述  : 接收端获取接收通道的ringbuf的读指针
 * 输入参数  :
 *			  chn_hdl: fipc通道句柄，由open通道时得到
 * 输出参数  :
 *			  rptr：接收通道当前的写指针
 * 返 回 值	:  0，更新成功；其他，更新失败。
 * 注意：不可以针对发送通道使用
 **************************************************************************/
int bsp_fipc_chn_rptr_get(fipc_chn_t chn_hdl, unsigned int *rptr);
#ifdef __cplusplus /* __cplusplus */
}
#endif /* __cplusplus */

#endif /*  __BSP_FIPC_H__ */

