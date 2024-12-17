/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2020. All rights reserved.
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

#ifndef _FIPC_DEVICE_H
#define _FIPC_DEVICE_H

#include "fipc_platform.h"

#define FIPC_IRQ_CNT 2
#define VERSION_NUM_MAX 5
#define FIPC_CPU_ID_MAX 16
#define FIPC_CHN_ID_MAX 64
/* 每个寄存器存放16个PIPE的安全配置，需要4个寄存器来存放 */
#define FIPC_PIPE_SEC_REGS_CNT (FIPC_CHN_ID_MAX >> 4)
#define FIPC_DFT_INVAILD_VALUE 0xFFFFFFFF

#define FIPC_PIPE_ADDR_ALIGN 0x8
#define FIPC_PIPE_ADDR_ALIGN_MASK (FIPC_PIPE_ADDR_ALIGN - 1)
#define FIPC_PIPE_PUSHPTR_ALIGN 0x4
#define FIPC_PIPE_PUSHPTR_ALIGN_MASK (FIPC_PIPE_PUSHPTR_ALIGN - 1)
#define FIPC_PIPE_DEPTH_ALIGN 0x80
#define FIPC_PIPE_DEPTH_ALIGN_MASK (FIPC_PIPE_DEPTH_ALIGN - 1)
#define FIPC_PIPE_DEPTH_MIN 0x80
#define FIPC_PIPE_DEPTH_MAX 0xFF80

/* DMA操作默认对齐字节数 */
#define FIPC_DMA_DFT_ALIGN 0x4
#define FIPC_DMA_DFT_ALIGN_MASK (FIPC_DMA_DFT_ALIGN - 1)

#define FIPC_CHN_MIN_ALIGNBYTES 0x4
#define FIPC_CHN_DEFAULT_ALIGNBYTES 0x4

#define FIPC_HALT_QUERY_WAIT_TIMES 200

/* 芯片给出的值 */
#define FIPC_OPIPE_STOP_WAIT 100
#define FIPC_IPIPE_STOP_WAIT 300
#define FIPC_IPIPE_IDLE_WAIT 300

/* DMA请求默认值 */
#define FIPC_DMA_REQ_SEL_INVAILD 0xFFFFFFFF

struct fipc_descriptor {
    u32 msg_len : 16; /* [0-15] */
    u32 P_pos : 1;    /* [16]P */
    u32 S_pos : 1;    /* [17]S */
    u32 D_pos : 1;    /* [18]D */
    u32 M_pos : 1;    /* [19]M */
    u32 N_pos : 1;    /* [20]N */
    u32 C_pos : 1;    /* [21]C */
    u32 F_pos : 1;    /* [22]F */
    u32 V_pos : 1;    /* [23]V */
    u32 II_pos : 1;   /* [24]II */
    u32 OI_pos : 1;   /* [25]OI */
    u32 SI_pos : 1;   /* [26]SI */
    u32 DI_pos : 1;   /* [27]DI */
    u32 reserved : 4; /* [28-31] */
};

struct fipc_bd1_s {
    struct fipc_descriptor cfg;
} __attribute__((aligned(4)));

/* 数据包头保持4字节对齐(芯片约束) */
struct fipc_packet {
    u32 user_id;   /* same with v100 */
    u16 len;       /* same with v100 */
    u16 task_id;   /* same with v100 */
    u32 timestamp; /* same with v100 该数据包发送时间戳 */
    u32 align_bytes;
    u32 ldrvchn_id; /* 维测，不做判据 */
    u32 rdrvchn_id; /* 维测，不做判据 */
    u32 recvtime1;  /* 该数据包接收时间戳 */
    u32 recvtime2;  /* 该数据包接收时间戳 */
};                  /* 驱动包头最大32字节对齐预留 */

struct fipc_send_packet {
    struct fipc_bd1_s bd_head;
    struct fipc_packet data_head;
};

struct fipc_bd2_s {
    struct fipc_descriptor cfg;
    u32 src_addr_l;
    struct {
        u32 src_addr_h : 8; /* [0-8] -> src_addr[32:39] */
        u32 reserved : 24;
    }ptr;
} __attribute__((aligned(4)));

enum FIPC_INT_READ_LEV {
    FIPC_INT_RD_UNDERFLOW = 0x1,
    FIPC_INT_RD_OVERFLOW = 0x2,
    FIPC_INT_RD_DATA_ARV = 0x4,
    FIPC_INT_RD_ARV_TIMEOUT = 0x8,
    FIPC_INT_RD_PIPE_WAKEUP = 0x10
};

enum FIPC_INT_WRITE_LEV {
    FIPC_INT_WR_UNDERFLOW = 0x1,
    FIPC_INT_WR_OVERFLOW = 0x2,
    FIPC_INT_WR_DATA_DONE = 0x4,
    FIPC_INT_WR_PIPE_ERR = 0x8,
    FIPC_INT_WR_TRANSCNT_FLIP = 0x10,
    FIPC_INT_WR_ABORT = 0x20 /* 这个中断一定不要开，因为应用中强停通道可能停其他核的 */
};

/* 通道状态 */
enum fipc_chn_stat {
    FIPC_CHN_INIT = 0x0001,         /* 初始化完的状态，以及调用close之后的状态 */
    FIPC_CHN_OPEN = 0x0002,         /* 调用open后打开的状态 */
    FIPC_CHN_OPENNOSR = 0x0003,     /* 某些特殊通道调用open后打开的状态 */
    FIPC_CHN_OSUSPENDING = 0x0004,  /* 低功耗正在关闭opipe的状态 */
    FIPC_CHN_OSUSPENDED = 0x0005,   /* 低功耗已经关闭opipe的状态 */
    FIPC_CHN_ISUSPENDING = 0x0006,  /* 低功耗正在关闭ipipe的状态 */
    FIPC_CHN_ISUSPENDED = 0x0007,   /* 低功耗已经关闭ipipe的状态 */
    FIPC_CHN_SUSPENDED = 0x0008,    /* 低功耗已经完成empty检查，完成低功耗的状态 */
    FIPC_CHN_DSSSUSPENDED = 0x0009, /* 某些特殊通道低功耗已经完成empty检查，完成低功耗的状态 */
    FIPC_CHN_CLOSING = 0x000A,      /* 调用close的中间的状态 */
    FIPC_CHN_ORESETTING = 0x000B,   /* 单独复位正在关闭opipe的状态 */
    FIPC_CHN_ORESETTED = 0x000C,    /* 单独复位已经关闭opipe的状态 */
    FIPC_CHN_IRESETTING = 0x000D,   /* 单独复位正在关闭ipipe的状态 */
    FIPC_CHN_IRESETTED = 0x000E,    /* 单独复位已经关闭ipipe的状态 */
    FIPC_CHN_SIRESETTING = 0x000F,    /* 单独复位正在关闭影子ipipe的状态 */
    FIPC_CHN_SIRESETTED = 0x0010,     /* 单独复位已经关闭影子ipipe的状态 */
    FIPC_CHN_RESETTED = 0x00011,     /* 单独复位已经完成关闭pipe的状态 */
    FIPC_CHN_OPEN_DELAY = 0x00012,     /* 延迟开启，当时初始化完成，但尚未开启通道 */
};

struct compat_attr {
    u32 pa;    /* ringbuf物理地址，硬件有对齐要求 8Bytes */
    void *va;  /* ringbuf虚拟地址 */
    u32 depth; /* depth，硬件有对齐要求 128Bytes */
    u32 buf_sz;
    u32 align_bytes;
    u32 dym_flags;
    u32 flup;    /* 上水线配置 pipe_ctrl.union_struct.up_thrh */
    u32 fldn;    /* 下水线配置 pipe_ctrl.union_struct.dn_thrh */
    u32 req_sel;    /* 外设请求选择编号 pipe_ctrl.union_struct.req_sel */
    u32 int_lev; /* 中断配置 FIPC_INT_WRITE_LEV for opipe and FIPC_INT_READ_LEV for ipipe */
    u32 timeout; /* [5]：超时计时粒度，1表示按照ms计数，0表示按照us计数
                    [4:0]：超时阈值，计数器计数到达阈值，上报数据到达超时中断，0表示不计时
                */
};

union glb_ctrl {
    u32 val;
    struct {
        u32 global_srst : 1; /* FIPC模块全局软复位控制，自动清零。1：请求复位.0：不请求复位 */
        u32 clkauto_en : 1;  /* 自动门控使能控制位。0：不门控,1：FIPC不使能，时钟关闭 */
        u32 push_en : 1;     /* 低功耗流程推送功能控制信位。0:不推送,1:推送 */
        u32 hresp_bypass : 1; /* AHB总线hresp返回error控制位。0：总线访问出错，返回error.1：总线访问出错，不返回error */
        u32 bus_err_bypass : 1; /* AXI总线resp error控制位。0：处理总线resp error.1：不处理总线resp error */
        u32 reserved : 26;      /* reserved */
        u32 fipc_ilde : 1;      /* fipc_idle */
    } union_stru;
};

struct fipc_glb_cfg {
    u32 glb_sec;
    u32 core_sec;
    u32 opipe_sec0;
    u32 opipe_sec1;
    u32 opipe_sec2;
    u32 opipe_sec3;
    u32 ipipe_sec0;
    u32 ipipe_sec1;
    u32 ipipe_sec2;
    u32 ipipe_sec3;
    union glb_ctrl ctrl_info;
    u32 os_cfg;
    u32 trans_stat;
    u32 trans_data_cnt;
};

union opipe_ctrl {
    u32 val;
    struct {
        u32 local_en : 1; /* 1:pipe使能 0:pipe不使能 */
        u32 type : 1; /* PIPE类型。0：opipe与ipipe有关联(MSG通道);1：opipe与ipipe无关联（DMA通道） */
        u32 priorv2 : 2; /* 高低优先级队列配置，支持4个优先级，0为高优先级，3为低优先级。 */
        u32 up_thrh : 2; /* PIPE上水线：00：一个数据块或则一个消息;01：1/2深度;10：3/4深度;11：7/8深度 */
        u32 dn_thrh : 2; /* PIPE下水线：00：一个数据块或则一个消息;01：1/8深度;10：1/4深度;11：1/2深度 */
        u32 pipe_abort : 1; /* PIPE abort控制，由local CPU控制，1：opipe abort; 0：opipe不abort */
        u32 reserved1 : 3;
        u32 req_sel : 4; /* 外设请求选择求号。如果是存储器到存储器模式，忽略该控制信号。最大支持16个外设请求, 静态配置。 */
        u32 reserved2 : 4;
        u32 priorv3 : 2; /* 高低优先级队列配置，支持4个优先级，0为高优先级，3为低优先级。 */
        u32 reserved3 : 6;
        u32 peri_en : 1; /* 存储器和存储器交互或者存储器和外设交互的控制信号：1：存储器和外设之间交互，对应的BD只能是格式四；0：存储器和存储器之间交互；*/
        u32 reserved4 : 3;
    } union_stru;
};

union ipipe_ctrl {
    u32 val;
    struct {
        u32 local_en : 1; /* 1:pipe使能 0:pipe不使能 */
        u32 type : 2; /* PIPE类型。00：内部通道的ipipe;01：外部通道的实体ipipe;10：外部通道的影子ipipe;11：保留 */
        u32 prior : 2; /* 高低优先级队列配置，支持4个优先级，0为高优先级，3为低优先级。 */
        u32 up_thrh : 2; /* PIPE上水线：00：一个数据块或则一个消息;01：1/2深度;10：3/4深度;11：7/8深度 */
        u32 dn_thrh : 2; /* PIPE下水线：00：一个数据块或则一个消息;01：1/8深度;10：1/4深度;11：1/2深度 */
        u32 arv_timeout : 6; /* 数据到达超时：[5]：超时计时粒度，1表示按照ms计数，0表示按照us计数[4:0]：超时阈值，计数器计数到达阈值，上报数据到达超时中断，0表示不计时
                              */
        u32 reserved : 17;
    } union_stru;
};

union opipe_stat {
    u32 val;
    struct {
        u32 is_empty : 1; /* 1:空0:不空 */
        u32 is_full : 1;  /* 1:满0:不满 */
        u32 is_idle : 1;  /* 1:idle  0:busy  */
        u32 reserved : 13;
        u32 left_cnt : 16; /* 该BD剩余的数据量，单位为BYTE；1、只在存储器和外设场景下有效；2、只在对应通道处于IDLE态有效； */
    } union_stru;
};

union ipipe_stat {
    u32 val;
    struct {
        u32 is_empty : 1; /* 1:空0:不空 */
        u32 is_full : 1;  /* 1:满0:不满 */
        u32 is_idle : 1;  /* 1:idle  0:busy  */
        u32 pt_stat : 2;  /* 00:不在低功耗状态  01:请求下电10:拒绝下电11:允许下电  */
        u32 reserved : 27;
    } union_stru;
};
/* clang-format off */
enum {
    FIPC_NOT_PM_STAT = 0x0,
    FIPC_PM_REQ_PD = 0x1,
    FIPC_PM_REFUSE_PD = 0x2,
    FIPC_PM_ALLOW_PD = 0x3
};
/* clang-format on */
struct opipe_config {
    u32 id; /* pipe编号  0-63 */
    void *ringbuf_va;
    fipcsoc_ptr_t ringbuf_pa;
    u32 depth;    /* 通道使用的buff大小， 128Bytes对齐 */
    u32 ram_size; /* 用户传入的buff大小 */
    u32 wptr;
    u32 rptr;
    u32 *ptr_vaddr;
    fipcsoc_ptr_t ptr_paddr;
    union opipe_ctrl opipe_uctrl;
    u32 int_level;
};

struct ipipe_config {
    u32 id;
    void *ringbuf_va;
    fipcsoc_ptr_t ringbuf_pa;
    u32 depth;
    u32 ram_size;
    u32 wptr;
    u32 rptr;
    u32 seek;
    u32 *ptr_vaddr;
    fipcsoc_ptr_t ptr_paddr;
    fipcsoc_ptr_t relv_pipe_paddr;
    union ipipe_ctrl ipipe_uctrl;
    u32 int_level;
};

/* used for fipc_channel.type */
#define FIPC_CHN_TYPE_MSG_OUTSEND 0x0
#define FIPC_CHN_TYPE_MSG_OUTRECV 0x1
#define FIPC_CHN_TYPE_MSG_INRSEND 0x2
#define FIPC_CHN_TYPE_MSG_INRRECV 0x3
#define FIPC_CHN_TYPE_DMA 0x4

/* 这几个宏要与 mdrv_fipc.h 里面的MDRV_ATTR对应 */
#define FIPC_DYM_SF_NOARRVEVENT (1UL << 0)
#define FIPC_DYM_SF_ENOFLOWACT (1UL << 1)
#define FIPC_DYM_SF_ENIFLOWACT (1UL << 2)
#define FIPC_DYM_SF_NOWAKEUPACT (1UL << 3)
#define FIPC_DYM_SF_DPMSRIGNORE (1UL << 4)
#define FIPC_DYM_SF_SENDDONEEN (1UL << 5)
#define FIPC_DYM_SF_PERITRANS (1UL << 6)
#define FIPC_DYM_SF_DYMRIGHT (1UL << 7)
#define FIPC_DYM_SF_TIMEOUTINTEN (FIPC_ATTR_FLAGS_TIMEOUTINTEN)
#define FIPC_DYM_SF_DSSCTRLSR (1UL << 31)

/* 这种通道在核间传递信息时，不带eicc头，不需要进行维测信息处理，包括接收端打时间戳 */
#define FIPC_CHN_SF_NO_EICC_HEADER (1UL << 26)
/* 导出通道ringbuf时需要进行处理 */
#define FIPC_CHN_SF_DUMP_RINGBUF_CARE (1UL << 27)
/* 这种通道在HIFI单独复位中需要处理 */
#define FIPC_CHN_SF_HIFIRST_CARE (1UL << 28)
/* 这种通道在单独复位中需要处理 */
#define FIPC_CHN_SF_MDMRST_CARE (1UL << 29)
/* 远端通道为非安全通道 */
#define FIPC_CHN_SF_REMOTE_UNSEC (1UL << 30)
/* 用于FPGA,EMU,ESL等场景，FIPC总是上电的，所以不需要唤醒，可以直接数据收发 */
#define FIPC_CHN_SF_REMOTE_READY (1UL << 31)

#define FIPC_IPIPE_THRESTHOLD_MASK (0x3)
#define FIPC_IPIPE_TIMEOUT_MASK (0x3f)

struct fipc_channel {
    u32 user_id;     /* 系统初始化时赋值 */
    u32 type;        /* 通道类型，系统初始化时赋值 ,不可改变 */
    u32 ldrvchn_id;  /* 通道类型，系统初始化时赋值 ,不可改变 */
    u32 rdrvchn_id;  /* 通道类型，系统初始化时赋值 ,不可改变 */
    u32 dym_flags;   /* 通道初始化的时候就确定的通道属性，在运行过程中不会改变 */
    u32 const_flags; /* 配置文件中决定的通道属性，系统初始化时赋值 ,不可改变 */
    u32 align_bytes; /* 消息头和消息体对齐方式，默认按照硬件约束4字节对齐,通道初始化确定，在运行过程中不会改变 */
    u32 state;       /* 通道状态: 标识发送通道是否可用 */
    u32 remote_awake; /* 通道睡眠状态标记 */
    u32 irq_ts;       /* 最后一次响应中断的时间 */
    spinlock_t lock;  /* 保护通道互斥的spin锁 */
    struct opipe_config *opipe_cfg;
    struct ipipe_config *ipipe_cfg;
    fipc_cbk cbk;  /* fipc事件发生时的回调函数 */
    void *cbk_arg; /* 回调时，透传的信息，用户需要保证cbk_arg的生命周期 */
    fipc_irq_wakeup_cbk irq_wakeup_cbk;
    void *irq_cbk_arg;
    struct compat_attr *pattr;
};

/*lint -esym(528,isMsgSendChn) */
static inline int isMsgSendChn(struct fipc_channel *pchannel)
{
    return (pchannel->type == FIPC_CHN_TYPE_MSG_OUTSEND || pchannel->type == FIPC_CHN_TYPE_MSG_INRSEND);
}
/*lint -esym(528,isMsgRecvChn) */
static inline int isMsgRecvChn(struct fipc_channel *pchannel)
{
    return (pchannel->type == FIPC_CHN_TYPE_MSG_OUTRECV || pchannel->type == FIPC_CHN_TYPE_MSG_INRRECV);
}

/* 中断不合并， 收发各自合并-即二合一， 收和发都合并-即四合一 */
#define FIPC_IRQ_TYPE_ORIGIN 0
#define FIPC_IRQ_TYPE_OIMERG 1
#define FIPC_IRQ_TYPE_ALMERG 2
struct irq_bundle {
    u32 cpu_id;               /* 用于读中断状态和判断通道是否归属自己 */
    u32 irq[FIPC_IRQ_CNT];    /* 一组最多FIPC_IRQ_CNT个中断 */
    struct fipc_device *pdev; /* 用于反向索引dev */
};

/* used for fipc_device.state */
#define FIPC_DEV_STATE_WORKING 1
#define FIPC_DEV_STATE_SLEEPING 2
#define FIPC_DEV_STATE_WAIT 3
/* used for fipc_device.ctrl_level */
#define FIPC_DEV_CONTROL_LEVEL_IGNORE 0
#define FIPC_DEV_CONTROL_LEVEL_IRQCLR 1
#define FIPC_DEV_CONTROL_LEVEL_GUEST 2
#define FIPC_DEV_CONTROL_LEVEL_HOST 3
#define FIPC_DEV_CONTROL_LEVEL_PART 4

struct fipc_device {
    u32 id; /* fipc设备在soc里面的编号 */
    u32 ctrl_level;
    u32 pipepair_cnt;
    void *base_va;
    fipcsoc_ptr_t base_pa;
    u32 *push_ptr_va;
    fipcsoc_ptr_t push_ptr_pa;
    u32 state;
    u32 irq_type;
    struct irq_bundle *maps[FIPC_CPU_ID_MAX]; /* 大部分设备只有一组，这样节省内存 */
    spinlock_t lock;
    u32 glb_ctrl;
    struct fipc_channel *channels[FIPC_CHN_ID_MAX];
    void *of_node;
    u32 version;
    struct fipc_config_diff *config_diff;
};

int fipc_devices_init(void);
struct fipc_device *fipc_device_get_fast(u32 devid);
struct fipc_channel *fipc_channel_get_fast(u32 usr_chnid);
struct fipc_channel *fipc_channel_get_byidx(u32 idx);
int fipc_dev_chn_get(u32 phy_chn_id, struct fipc_device **ppdev, struct fipc_channel **ppchannel);
int fipc_dev_chn_get_byid(u32 user_id, struct fipc_device **ppdev, struct fipc_channel **ppchannel);
int fipc_dev_chn_get_byidx(u32 chn_idx, struct fipc_device **ppdev, struct fipc_channel **ppchannel);

#endif
