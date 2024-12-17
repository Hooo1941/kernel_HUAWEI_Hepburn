/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2021. All rights reserved.
 * Description: DRA driver source code
 */
#ifndef __SC_HAL_H_
#define __SC_HAL_H_

#define GLB_RTL_VER                 (0x0)    /* 版本信息 */
#define GLB_RTL_INFO                (0x4)    /* 代码配置信息0 */
#define GLB_RTL_INF1                (0x8)    /* 代码配置信息1 */
#define GLB_RTL_INF2                (0xC)    /* 代码配置信息2 */
#define GLB_ACPU                    (0x20)   /* GLB_ACPU */
#define GLB_GID_CTRL_0              (0x24)   /* GID控制信息 */
#define GLB_QOS_CTRL_URGT_1         (0x28)   /* QOS相关控制_1(URGT) */
#define GLB_FCM_MID_RT              (0x2C)   /* RT使用FCM_MID路由配置 */
#define GLB_ROUTE_PLC_S             (0x60)   /* 全局Route policy配置寄存器 */
#define GLB_WAY_EN                  (0x80)   /* 控制每个way的cache line分配 */
#define GLB_QOS_CTRL_URGT           (0x100)  /* QOS相关控制(URGT) */
#define GLB_QOS_CTRL_EV             (0x104)  /* QOS相关控制(evict) */
#define GLB_QOS_CTRL_SCH            (0x108)  /* QOS相关控制(schedule) */
#define GLB_SLC_CTRL_LVL            (0x110)  /* 各slice相关控制水线 */
#define GLB_SCQ_CTRL0               (0x140)  /* SCQ控制0 */
#define GLB_SCQ_CTRL1               (0x144)  /* 流控相关控制1 */
#define GLB_CMO_SCH_CTRL            (0x150)  /* CMO调度控制 */
#define GLB_CFC0                    (0x180)  /* 命令流量控制器0(带宽控制和OSTD控制) */
#define GLB_CFC1                    (0x184)  /* 命令流量控制器1(带宽控制和OSTD控制) */
#define GLB_CFC_QOS_ALLOW           (0x18C)  /* cfc四级水线映射关系 */
#define GLB_CFC_BYP_LV              (0x190)  /* bypass水线配置值 */
#define SC_ROUTER_BYP_CTRL          (0x194)  /* SC内部4bit水线映射为8bit水线 */
#define SC_ROUTER_WRR_EN            (0x198)  /* WRR权重配置寄存器 */
#define SC_ROUTER_ACCESS            (0x19C)  /* router模块寄存器 */
#define GLB_RW_DUMMY0_NS            (0x700)  /* GLB_rw_dummy0_NS */
#define GLB_RW_DUMMY1_NS            (0x704)  /* GLB_rw_dummy1_NS */
#define GLB_RW_DUMMY2_NS            (0x708)  /* GLB_rw_dummy2_NS */
#define GLB_RW_DUMMY3_NS            (0x70C)  /* GLB_rw_dummy3_NS */
#define GLB_RC_DUMMY0_NS            (0x710)  /* GLB_rc_dummy0_NS */
#define GLB_RC_DUMMY1_NS            (0x714)  /* GLB_rc_dummy1_NS */
#define GLB_WC_DUMMY0_NS            (0x718)  /* GLB_wc_dummy0_NS */
#define GLB_WC_DUMMY1_NS            (0x71C)  /* GLB_wc_dummy1_NS */
#define GLB_RO_DUMMY0_NS            (0x720)  /* GLB_ro_dummy0_NS */
#define GLB_RO_DUMMY1_NS            (0x724)  /* GLB_ro_dummy1_NS */
#define GLB_W1P_DUMMY_NS            (0x730)  /* GLB_w1p_dummy_NS */
#define GLB_CKG_BYP                 (0x800)  /* 时钟门控bypass */
#define GLB_CKG_EXT                 (0x804)  /* 时钟门控展宽配置 */
#define GLB_FUNC_BYP                (0x808)  /* 逻辑功能bypass */
#define GLB_PARAM_OVRD              (0x80C)  /* 配置参数override */
#define GLB_PARAM_OVRD1             (0x810)  /* 配置参数override */
#define GLB_PMU_CTRL                (0x820)

#define CFG_INT_STATUS              (0x2000) /* 中断状态 */
#define CMO_CFG_INT_INI             (0x2004) /* CMO原始中断状态 */
#define CMO_CFG_INT_EN              (0x2008) /* CMO中断上报使能 */
#define CMO_CFG_INT_CLR             (0x200C) /* CMO中断信息清除 */
#define CFG_CMO_QUEUE_CNT           (0x2018) /* CMO模块的命令队列深度状态 */
#define CFG_CMO_REGULAR_EVT_TIME    (0x2020) /* CMO状态寄存器的eventfinish为1时，定时上报的时间间隔设置 */
#define CFG_READY_WAIT_TIME         (0x2030) /* 时钟门控关闭时，Ready信号拉高前的等待时间 */
#define CFG_CH_MODE                 (0x2034) /* DDR通道，支持双通道，单DDRA，单DDRB三种模式 */
#define CFG_CMO_ERR_CMD_L           (0x2058) /* CMO 异常命令记录 */
#define CFG_CMO_ERR_CMD_H           (0x205C) /* CMO 异常命令记录 */
#define CFG_DFX_DISTR               (0x2078) /* CMO DISTR模块的DFX控制 */
#define CFG_DFX                     (0x207C) /* CFG模块维测信息 */

#define CFG_CMO_STATUS(cmo_st)      (0x2100 + (cmo_st) * 4) /* CMO返回的状态寄存器 */
#define CFG_CMO_CLEAR(cmo_st)       (0x2200 + (cmo_st) * 4) /* CMO状态寄存器清零 */

#define SC_RT_INT_STATUS            (0x2300) /* SC Router中断状态 */
#define SC_RT_INT_INI               (0x2304) /* SC Router原始中断状态 */
#define SC_RT_INT_EN                (0x2308) /* SC Router原始中断上报使能 */
#define SC_RT_INT_CLR               (0x230C) /* SC Router中断信息清除 */
#define SC00_RT_ERR_CMD_L           (0x2310) /* SC00 Router异常命令记录 */
#define SC00_RT_ERR_CMD_H           (0x2314) /* SC00 Router异常命令记录 */
#define SC01_RT_ERR_CMD_L           (0x2318) /* SC01 Router异常命令记录 */
#define SC01_RT_ERR_CMD_H           (0x231C) /* SC01 Router异常命令记录 */
#define SC10_RT_ERR_CMD_L           (0x2320) /* SC10 Router异常命令记录 */
#define SC10_RT_ERR_CMD_H           (0x2324) /* SC10 Router异常命令记录 */
#define SC11_RT_ERR_CMD_L           (0x2328) /* SC11 Router异常命令记录 */
#define SC11_RT_ERR_CMD_H           (0x232C) /* SC11 Router异常命令记录 */

#define GID_QUOTA(gid_idx)          (0x4000 + (gid_idx) * 0x100) /* 各GID的quota水线 */
#define GID_WAY_ALLC(gid_idx)       (0x4004 + (gid_idx) * 0x100) /* 各GID的way_enable_bitmap */
#define GID_ALLC_PLC(gid_idx)       (0x4008 + (gid_idx) * 0x100) /* allocate policy配置寄存器 */
#define GID_MID_FLT0(gid_idx)       (0x400C + (gid_idx) * 0x100) /* filter0的MID配置 */
#define GID_MID_FLT1(gid_idx)       (0x4010 + (gid_idx) * 0x100) /* filter1的MID配置 */

#define CMO_CMD_L                   (0x5000) /* 发向CMO的64bit命令 */
#define CMO_CMD_H                   (0x5004) /* 发向CMO的64bit命令 */

#define SLC_PERF_MUX_MODE(slc)      (0x6000 + (slc) * 0x800) /* slice中PMU中PERF的mux模式选择寄存器 */

// 11111
#define SLC_CM_CNT_EN(slc)             (0x6004 + (slc) * 0x800) /* slice中的统计计数器M使能寄存器 */
#define SLC_CM_CNT_MUX0(slc)           (0x6008 + (slc) * 0x800) /* slice中的公共统计计数器MUX选择寄存器0 */
#define SLC_CM_CNT_MUX1(slc)           (0x600C + (slc) * 0x800) /* slice中的公共统计计数器MUX选择寄存器1 */
#define SLC_CM_CNT_MUX2(slc)           (0x6010 + (slc) * 0x800) /* slice中的公共统计计数器MUX选择寄存器2 */
#define SLC_PMU_CNT0_TH_L(slc)         (0x6014 + (slc) * 0x800) /* 统计寄存器0上报中断的门限值低位 */
#define SLC_PMU_CNT0_TH_H(slc)         (0x6018 + (slc) * 0x800) /* 统计寄存器0上报中断的门限值低位 */
#define SLC_PMU_CNT1_TH_L(slc)         (0x601C + (slc) * 0x800) /* 统计寄存器1上报中断的门限值低位 */
#define SLC_PMU_CNT1_TH_H(slc)         (0x6020 + (slc) * 0x800) /* 统计寄存器0上报中断的门限值低位 */
#define SLC_PMU_CNT2_TH_L(slc)         (0x6024 + (slc) * 0x800) /* 统计寄存器2上报中断的门限值低位 */
#define SLC_PMU_CNT2_TH_H(slc)         (0x6028 + (slc) * 0x800) /* 统计寄存器0上报中断的门限值低位 */
#define SLC_PMU_CNT3_TH_L(slc)         (0x602C + (slc) * 0x800) /* 统计寄存器3上报中断的门限值低位 */
#define SLC_PMU_CNT3_TH_H(slc)         (0x6030 + (slc) * 0x800) /* 统计寄存器0上报中断的门限值低位 */
#define SLC_PMU_CNT4_TH_L(slc)         (0x6034 + (slc) * 0x800) /* 统计寄存器4上报中断的门限值低位 */
#define SLC_PMU_CNT4_TH_H(slc)         (0x6038 + (slc) * 0x800) /* 统计寄存器0上报中断的门限值低位 */
#define SLC_PMU_CNT5_TH_L(slc)         (0x603C + (slc) * 0x800) /* 统计寄存器5上报中断的门限值低位 */
#define SLC_PMU_CNT5_TH_H(slc)         (0x6040 + (slc) * 0x800) /* 统计寄存器0上报中断的门限值低位 */
#define SLC_PMU_CNT6_TH_L(slc)         (0x6044 + (slc) * 0x800) /* 统计寄存器6上报中断的门限值低位 */
#define SLC_PMU_CNT6_TH_H(slc)         (0x6048 + (slc) * 0x800) /* 统计寄存器0上报中断的门限值低位 */
#define SLC_PMU_CNT7_TH_L(slc)         (0x604C + (slc) * 0x800) /* 统计寄存器7上报中断的门限值低位 */
#define SLC_PMU_CNT7_TH_H(slc)         (0x6050 + (slc) * 0x800) /* 统计寄存器0上报中断的门限值低位 */
#define SLC_PMU_CNT8_TH_L(slc)         (0x6054 + (slc) * 0x800) /* 统计寄存器8上报中断的门限值低位 */
#define SLC_PMU_CNT8_TH_H(slc)         (0x6058 + (slc) * 0x800) /* 统计寄存器0上报中断的门限值低位 */
#define SLC_PMU_CNT9_TH_L(slc)         (0x605C + (slc) * 0x800) /* 统计寄存器9上报中断的门限值低位 */
#define SLC_PMU_CNT9_TH_H(slc)         (0x6060 + (slc) * 0x800) /* 统计寄存器0上报中断的门限值低位 */
#define SLC_PMU_CNT10_TH_L(slc)        (0x6064 + (slc) * 0x800) /* 统计寄存器10上报中断的门限值低位 */
#define SLC_PMU_CNT10_TH_H(slc)        (0x6068 + (slc) * 0x800) /* 统计寄存器0上报中断的门限值低位 */
#define SLC_PMU_CNT11_TH_L(slc)        (0x606C + (slc) * 0x800) /* 统计寄存器11上报中断的门限值低位 */
#define SLC_PMU_CNT11_TH_H(slc)        (0x6070 + (slc) * 0x800) /* 统计寄存器0上报中断的门限值低位 */
#define SLC_BP2_CFG(slc)               (0x6094 + (slc) * 0x800) /* PMU BP2 CFG寄存器 */
#define SLC_BP5_CFG(slc)               (0x6098 + (slc) * 0x800) /* PMU BP5 CFG寄存器 */
#define SLC_BP6_CFG(slc)               (0x609C + (slc) * 0x800) /* PMU BP6 CFG寄存器 */
#define SLC_BP7_CFG_0(slc)             (0x60A0 + (slc) * 0x800) /* PMU BP7 CFG寄存器0 */
#define SLC_BP7_CFG_1(slc)             (0x60A4 + (slc) * 0x800) /* PMU BP7 CFG寄存器1 */
#define SLC_BP7_CFG_2(slc)             (0x60A8 + (slc) * 0x800) /* PMU BP7 CFG寄存器2 */
#define SLC_BP8_CFG_0(slc)             (0x60AC + (slc) * 0x800) /* PMU BP8 CFG寄存器0 */
#define SLC_BP8_CFG_1(slc)             (0x60B0 + (slc) * 0x800) /* PMU BP8 CFG寄存器1 */
#define SLC_BP8_CFG_3(slc)             (0x60B4 + (slc) * 0x800) /* PMU BP8 CFG寄存器3 */
#define SLC_BP8_CFG_4(slc)             (0x60B8 + (slc) * 0x800) /* PMU BP8 CFG寄存器4 */
#define SLC_BP9_CFG_0(slc)             (0x60BC + (slc) * 0x800) /* PMU BP9 CFG寄存器0 */
#define SLC_BP10_CFG_0(slc)            (0x60C8 + (slc) * 0x800) /* PMU BP10 CFG寄存器0 */
#define SLC_BP10_CFG_1(slc)            (0x60CC + (slc) * 0x800) /* PMU BP10 CFG寄存器1 */
#define SLC_BP10_CFG_2(slc)            (0x60D0 + (slc) * 0x800) /* PMU BP10 CFG寄存器2 */
#define SLC_BP11_CFG_0(slc)            (0x60D4 + (slc) * 0x800) /* PMU BP11 CFG寄存器0 */
#define SLC_BP11_CFG_1(slc)            (0x60D8 + (slc) * 0x800) /* PMU BP11 CFG寄存器1 */
#define SLC_BP12_CFG_0(slc)            (0x60DC + (slc) * 0x800) /* PMU BP12 CFG寄存器0 */
#define SLC_BP12_CFG_1(slc)            (0x60E0 + (slc) * 0x800) /* PMU BP12 CFG寄存器1 */
#define SLC_BP12_CFG_2(slc)            (0x60E4 + (slc) * 0x800) /* PMU BP12 CFG寄存器2 */
#define SLC_BP14A_CFG_0(slc)           (0x60E8 + (slc) * 0x800) /* PMU BP14第一组CFG寄存器0 */
#define SLC_BP14A_CFG_1(slc)           (0x60EC + (slc) * 0x800) /* PMU BP14第一组CFG寄存器1 */
#define SLC_BP14B_CFG_0(slc)           (0x60F0 + (slc) * 0x800) /* PMU BP14第二组CFG寄存器0 */
#define SLC_BP14B_CFG_1(slc)           (0x60F4 + (slc) * 0x800) /* PMU BP14第二组CFG寄存器1 */
#define SLC_BP15_CFG_0(slc)            (0x60F8 + (slc) * 0x800) /* PMU BP15 CFG寄存器0 */
#define SLC_TCP0_CFG_0(slc)            (0x60FC + (slc) * 0x800) /* PMU TCP0 CFG寄存器0 */
#define SLC_TCP0_CFG_1(slc)            (0x6100 + (slc) * 0x800) /* PMU TCP0 CFG寄存器1 */
#define SLC_TCP0_CFG_2(slc)            (0x6104 + (slc) * 0x800) /* PMU TCP0 CFG寄存器2 */
#define SLC_TCP1_CFG_0(slc)            (0x6108 + (slc) * 0x800) /* PMU TCP1 CFG寄存器0 */
#define SLC_TCP1_CFG_1(slc)            (0x610C + (slc) * 0x800) /* PMU TCP1 CFG寄存器1 */
#define SLC_TCP1_CFG_2(slc)            (0x6110 + (slc) * 0x800) /* PMU TCP1 CFG寄存器2 */
#define SLC_TCP2A_CFG_0(slc)           (0x6114 + (slc) * 0x800) /* PMU TCP2第一组CFG寄存器0 */
#define SLC_TCP2A_CFG_1(slc)           (0x6118 + (slc) * 0x800) /* PMU TCP2第一组CFG寄存器1 */
#define SLC_TCP2B_CFG_0(slc)           (0x611C + (slc) * 0x800) /* PMU TCP2第二组CFG寄存器0 */
#define SLC_TCP2B_CFG_1(slc)           (0x6120 + (slc) * 0x800) /* PMU TCP2第二组CFG寄存器1 */
#define SLC_TCP4_CFG_0(slc)            (0x6134 + (slc) * 0x800) /* PMU TCP4 CFG寄存器0 */
#define SLC_TCP6A_CFG_0(slc)           (0x6140 + (slc) * 0x800) /* PMU TCP6第一组CFG寄存器0 */
#define SLC_TCP6A_CFG_1(slc)           (0x6144 + (slc) * 0x800) /* PMU TCP6第一组CFG寄存器1 */
#define SLC_TCP6B_CFG_0(slc)           (0x6148 + (slc) * 0x800) /* PMU TCP6第二组CFG寄存器0 */
#define SLC_TCP6B_CFG_1(slc)           (0x614C + (slc) * 0x800) /* PMU TCP6第二组CFG寄存器1 */
#define SLC_TCP7A_CFG_0(slc)           (0x6150 + (slc) * 0x800) /* PMU TCP7第一组CFG寄存器0 */
#define SLC_TCP7A_CFG_1(slc)           (0x6154 + (slc) * 0x800) /* PMU TCP7第一组CFG寄存器1 */
#define SLC_TCP7B_CFG_0(slc)           (0x6158 + (slc) * 0x800) /* PMU TCP7第二组CFG寄存器0 */
#define SLC_TCP7B_CFG_1(slc)           (0x615C + (slc) * 0x800) /* PMU TCP7第二组CFG寄存器1 */
#define SLC_TCP9_CFG_0(slc)            (0x6170 + (slc) * 0x800) /* PMU TCP9 CFG寄存器0 */
#define SLC_TCP9_CFG_1(slc)            (0x6174 + (slc) * 0x800) /* PMU TCP9 CFG寄存器1 */
#define SLC_TCP10_CFG_0(slc)           (0x6178 + (slc) * 0x800) /* PMU TCP10 CFG寄存器0 */
#define SLC_DCP03_CFG_0(slc)           (0x617C + (slc) * 0x800) /* PMU DCP0~3 CFG寄存器0 */
#define SLC_DCP6_CFG_0(slc)            (0x6180 + (slc) * 0x800) /* PMU DCP6 CFG寄存器0 */
#define SLC_DCP7_CFG_0(slc)            (0x6184 + (slc) * 0x800) /* PMU DCP7 CFG寄存器0 */
#define SLC_DCP8_CFG_0(slc)            (0x6188 + (slc) * 0x800) /* PMU DCP8 CFG寄存器0 */
#define SLC_DCP9_CFG_0(slc)            (0x618C + (slc) * 0x800) /* PMU DCP9 CFG寄存器0 */
#define SLC_PMP_CFG_0(slc)             (0x6190 + (slc) * 0x800) /* PMU PMP CFG寄存器0 */
#define SLC_PMU_CNT0_L(slc)            (0x6194 + (slc) * 0x800) /* 统计寄存器0计数状态值低位 */
#define SLC_PMU_CNT0_H(slc)            (0x6198 + (slc) * 0x800) /* 统计寄存器0计数状态值高位 */
#define SLC_PMU_CNT1_L(slc)            (0x619C + (slc) * 0x800) /* 统计寄存器1计数状态值低位 */
#define SLC_PMU_CNT1_H(slc)            (0x61A0 + (slc) * 0x800) /* 统计寄存器1计数状态值高位 */
#define SLC_PMU_CNT2_L(slc)            (0x61A4 + (slc) * 0x800) /* 统计寄存器2计数状态值低位 */
#define SLC_PMU_CNT2_H(slc)            (0x61A8 + (slc) * 0x800) /* 统计寄存器2计数状态值高位 */
#define SLC_PMU_CNT3_L(slc)            (0x61AC + (slc) * 0x800) /* 统计寄存器3计数状态值低位 */
#define SLC_PMU_CNT3_H(slc)            (0x61B0 + (slc) * 0x800) /* 统计寄存器3计数状态值高位 */
#define SLC_PMU_CNT4_L(slc)            (0x61B4 + (slc) * 0x800) /* 统计寄存器4计数状态值低位 */
#define SLC_PMU_CNT4_H(slc)            (0x61B8 + (slc) * 0x800) /* 统计寄存器4计数状态值高位 */
#define SLC_PMU_CNT5_L(slc)            (0x61BC + (slc) * 0x800) /* 统计寄存器5计数状态值低位 */
#define SLC_PMU_CNT5_H(slc)            (0x61C0 + (slc) * 0x800) /* 统计寄存器5计数状态值高位 */
#define SLC_PMU_CNT6_L(slc)            (0x61C4 + (slc) * 0x800) /* 统计寄存器6计数状态值低位 */
#define SLC_PMU_CNT6_H(slc)            (0x61C8 + (slc) * 0x800) /* 统计寄存器6计数状态值高位 */
#define SLC_PMU_CNT7_L(slc)            (0x61CC + (slc) * 0x800) /* 统计寄存器7计数状态值低位 */
#define SLC_PMU_CNT7_H(slc)            (0x61D0 + (slc) * 0x800) /* 统计寄存器7计数状态值高位 */
#define SLC_PMU_CNT8_L(slc)            (0x61D4 + (slc) * 0x800) /* 统计寄存器8计数状态值低位 */
#define SLC_PMU_CNT8_H(slc)            (0x61D8 + (slc) * 0x800) /* 统计寄存器8计数状态值高位 */
#define SLC_PMU_CNT9_L(slc)            (0x61DC + (slc) * 0x800) /* 统计寄存器9计数状态值低位 */
#define SLC_PMU_CNT9_H(slc)            (0x61E0 + (slc) * 0x800) /* 统计寄存器9计数状态值高位 */
#define SLC_PMU_CNT10_L(slc)           (0x61E4 + (slc) * 0x800) /* 统计寄存器10计数状态值低位 */
#define SLC_PMU_CNT10_H(slc)           (0x61E8 + (slc) * 0x800) /* 统计寄存器10计数状态值高位 */
#define SLC_PMU_CNT11_L(slc)           (0x61EC + (slc) * 0x800) /* 统计寄存器11计数状态值低位 */
#define SLC_PMU_CNT11_H(slc)           (0x61F0 + (slc) * 0x800) /* 统计寄存器11计数状态值高位 */
#define SLC_PMU_CNT12_L(slc)           (0x61F4 + (slc) * 0x800) /* 统计寄存器12计数状态值低位 */
#define SLC_PMU_CNT12_H(slc)           (0x61F8 + (slc) * 0x800) /* 统计寄存器12计数状态值高位 */
#define SLC_PMU_CNT13_L(slc)           (0x61FC + (slc) * 0x800) /* 统计寄存器13计数状态值低位 */
#define SLC_PMU_CNT13_H(slc)           (0x6200 + (slc) * 0x800) /* 统计寄存器13计数状态值高位 */
#define SLC_PMU_CNT14_L(slc)           (0x6204 + (slc) * 0x800) /* 统计寄存器14计数状态值低位 */
#define SLC_PMU_CNT14_H(slc)           (0x6208 + (slc) * 0x800) /* 统计寄存器14计数状态值高位 */
#define SLC_PMU_CNT15_L(slc)           (0x620C + (slc) * 0x800) /* 统计寄存器15计数状态值低位 */
#define SLC_PMU_CNT15_H(slc)           (0x6210 + (slc) * 0x800) /* 统计寄存器15计数状态值高位 */
#define SLC_PMU_STAT_CYCLE_L(slc)      (0x6214 + (slc) * 0x800) /* PMU CYCLE状态低位 */
#define SLC_PMU_STAT_CYCLE_H(slc)      (0x6218 + (slc) * 0x800) /* PMU CYCLE状态高位 */
#define SLC_PMU_STAT_BUSY_L(slc)       (0x621C + (slc) * 0x800) /* PMU BUSY状态低位 */
#define SLC_PMU_STAT_BUSY_H(slc)       (0x6220 + (slc) * 0x800) /* PMU BUSY状态高位 */
#define SLC_TCP3A_CFG_0(slc)           (0x62E4 + (slc) * 0x800) /* PMU TCP3第一组CFG寄存器0 */
#define SLC_TCP3A_CFG_1(slc)           (0x62E8 + (slc) * 0x800) /* PMU TCP3第一组CFG寄存器1 */
#define SLC_TCP3A_CFG_2(slc)           (0x62EC + (slc) * 0x800) /* PMU TCP3第一组CFG寄存器2 */
#define SLC_TCP3B_CFG_0(slc)           (0x62F4 + (slc) * 0x800) /* PMU TCP3第二组CFG寄存器0 */
#define SLC_TCP3B_CFG_1(slc)           (0x62F8 + (slc) * 0x800) /* PMU TCP3第二组CFG寄存器1 */
#define SLC_TCP3B_CFG_2(slc)           (0x62FC + (slc) * 0x800) /* PMU TCP3第一组CFG寄存器2 */
#define SLC_TCP5A_CFG_0(slc)           (0x6300 + (slc) * 0x800) /* PMU TCP5 CFG寄存器0 */
#define SLC_TCP5A_CFG_1(slc)           (0x6304 + (slc) * 0x800) /* PMU TCP5 CFG寄存器1 */
#define SLC_TCP5A_CFG_2(slc)           (0x6308 + (slc) * 0x800) /* PMU TCP5 CFG寄存器2 */
#define SLC_TCP5B_CFG_0(slc)           (0x6310 + (slc) * 0x800) /* PMU TCP5 CFG寄存器0 */
#define SLC_TCP5B_CFG_1(slc)           (0x6314 + (slc) * 0x800) /* PMU TCP5 CFG寄存器1 */
#define SLC_TCP5B_CFG_2(slc)           (0x6318 + (slc) * 0x800) /* PMU TCP5 CFG寄存器2 */
#define SLC_TCP8A_CFG_0(slc)           (0x6320 + (slc) * 0x800) /* PMU TCP8第一组CFG寄存器0 */
#define SLC_TCP8A_CFG_1(slc)           (0x6324 + (slc) * 0x800) /* PMU TCP8第一组CFG寄存器1 */
#define SLC_TCP8A_CFG_2(slc)           (0x6328 + (slc) * 0x800) /* PMU TCP8第一组CFG寄存器2 */
#define SLC_TCP8B_CFG_0(slc)           (0x6330 + (slc) * 0x800) /* PMU TCP8第二组CFG寄存器0 */
#define SLC_TCP8B_CFG_1(slc)           (0x6334 + (slc) * 0x800) /* PMU TCP8第二组CFG寄存器1 */
#define SLC_TCP8B_CFG_2(slc)           (0x6338 + (slc) * 0x800) /* PMU TCP8第二组CFG寄存器2 */

#define SLC_WAY_PWRUP_STAT(slc)        (0x8000 + (slc) * 0x2000) /* 每个slice的way tag ram掉电状态 */
#define SLC_GID_COUNT(slc, gid_idx)    (0x8010 + (slc) * 0x2000 + (gid_idx) * 0x4) /* 该GID在SLICE中分配的Cacheline计数 */
#define SLC_INT_ST(slc)                (0x8080 + (slc) * 0x2000) /* 各Slice的中断状态 */
#define SLC_INT_INI(slc)               (0x8084 + (slc) * 0x2000) /* 各Slice的原始中断状态 */
#define SLC_INT_CLEAR(slc)             (0x8088 + (slc) * 0x2000) /* 各Slice的中断清零 */
#define SLC_INT_EN(slc)                (0x8090 + (slc) * 0x2000) /* 各Slice的中断使能 */
#define GLB_FLOW_STAT(slc)             (0x8108 + (slc) * 0x2000) /* 基于slice的配置寄存器 */
#define SC_ROUTER_EXPECTED_MID(slc)    (0x810C + (slc) * 0x2000) /* 设置预期mid中断的信息 */
#define SC_ROUTER_UNEXPECTED_MID(slc)  (0x8110 + (slc) * 0x2000) /* 设置非预期mid中断的设置信息 */
#define SC_ROUTER_CMDID_SEL(slc)       (0x8114 + (slc) * 0x2000) /* 配置cmd id读取txwd table */
#define SC_ROUTER_ERR_CLR(slc)         (0x8118 + (slc) * 0x2000) /* router清异常配置寄存器 */
#define GLB_CFC_OST_EN(slc)            (0x8130 + (slc) * 0x2000) /* ostanding模式使能寄存器 */
#define GLB_CFC_RESP_OSTD(slc)         (0x8200 + (slc) * 0x2000) /* CFC outstanding统计 */
#define GLB_FLOW_STATS_IN(slc)         (0x8204 + (slc) * 0x2000) /* 统计计数器大小 */
#define GLB_FLOW_STATS_OUT(slc)        (0x8208 + (slc) * 0x2000) /* 统计计数器大小 */
#define SC_ROUTER_DMC_BUSY(slc)        (0x820C + (slc) * 0x2000) /* 路由模块busy状态指示 */
#define SC_ROUTER_CMD_ST(slc)          (0x8210 + (slc) * 0x2000) /* demux模块握手信息 */
#define SC_DEMUX_DFX_INFO(slc)         (0x8214 + (slc) * 0x2000) /* demux模块dfx信息 */
#define SC_GID_STATE(slc)              (0x8218 + (slc) * 0x2000) /* 16GID当前的路由信息 */
#define SC_WCMD_OST_CNT_00(slc)        (0x821C + (slc) * 0x2000) /* gid0-3的写命令ost计数值 */
#define SC_WCMD_OST_CNT_01(slc)        (0x8220 + (slc) * 0x2000) /* gid4-7的写命令ost计数值 */
#define SC_WCMD_OST_CNT_10(slc)        (0x8224 + (slc) * 0x2000) /* gid8-11的写命令ost计数值 */
#define SC_WCMD_OST_CNT_11(slc)        (0x8228 + (slc) * 0x2000) /* gid12-15的写命令ost计数值 */
#define SC_RCMD_OST_CNT_00(slc)        (0x822C + (slc) * 0x2000) /* gid0-3的读命令ost计数值 */
#define SC_RCMD_OST_CNT_01(slc)        (0x8230 + (slc) * 0x2000) /* gid4-7的读命令ost计数值 */
#define SC_RCMD_OST_CNT_10(slc)        (0x8234 + (slc) * 0x2000) /* gid8-11的读命令ost计数值 */
#define SC_RCMD_OST_CNT_11(slc)        (0x8238 + (slc) * 0x2000) /* gid12-15的读命令ost计数值 */
#define SC_RT_BYP_SWT_CNT(slc)         (0x823C + (slc) * 0x2000) /* Gid的切换周期统计计数器 */
#define SC_OST_CNT_EN(slc)             (0x8240 + (slc) * 0x2000) /* sc bypass信号配置值 */
#define SC_GID_BYPASS(slc)             (0x8244 + (slc) * 0x2000) /* gid_byp配置值 */
#define SC_BYP_CMD_FIFO_INFO(slc)      (0x8248 + (slc) * 0x2000) /* bypass模块cmd通道fifo信息 */
#define SC_BYP_TX_FIFO_INFO(slc)       (0x824C + (slc) * 0x2000) /* bypass模块cmd通道tx fifo信息 */
#define SC_BYP_CMDFLAG_FIFO_RDBLK(slc) (0x8250 + (slc) * 0x2000) /* bypass模块cmdflag通道fifo及block rd fifo信息 */
#define SC_BYP_TXWR_FIFO_INFO(slc)     (0x8254 + (slc) * 0x2000) /* bypass模块DFX相关信息 */
#define SC_BYP_IRD_FIFO_INTLEV(slc)    (0x8258 + (slc) * 0x2000) /* bypass模块读数据DFX相关信息 */
#define SC_IWDB_DFX_INFO(slc)          (0x825C + (slc) * 0x2000) /* router iwdb模块的DFX信息 */
#define SC_MUX_DFX_INFO(slc)           (0x8260 + (slc) * 0x2000) /* router mux模块的DFX信息 */
#define SC_CFC0_BANDWIDTH_ACCU(slc)    (0x8264 + (slc) * 0x2000) /* CFC0带宽流控时计数 */
#define SC_CFC1_BANDWIDTH_ACCU(slc)    (0x8268 + (slc) * 0x2000) /* CFC1带宽流控时计数 */
#define SC_RT_ERR_INT(slc)             (0x826C + (slc) * 0x2000) /* RT中断信号 */
#define SC_ROUTER_CMD_IN_ERR_00(slc)   (0x8270 + (slc) * 0x2000) /* 重复的输入cmd id命令异常 */
#define SC_ROUTER_CMD_IN_ERR_01(slc)   (0x8274 + (slc) * 0x2000) /* 重复的输入cmd id命令异常 */
#define SC_ROUTER_CMD_IN_ERR_10(slc)   (0x8278 + (slc) * 0x2000) /* 重复的输入cmd id命令异常 */
#define SC_ROUTER_CMD_IN_ERR_11(slc)   (0x827C + (slc) * 0x2000) /* 重复的输入cmd id命令异常 */
#define SC_ROUTER_CMD_REC_ERR_00(slc)  (0x8280 + (slc) * 0x2000) /* 重复的rlast或者drsp的id命令异常 */
#define SC_ROUTER_CMD_REC_ERR_01(slc)  (0x8284 + (slc) * 0x2000) /* 重复的rlast或者drsp的id命令异常 */
#define SC_ROUTER_CMD_REC_ERR_10(slc)  (0x8288 + (slc) * 0x2000) /* 重复的rlast或者drsp的id命令异常 */
#define SC_ROUTER_CMD_REC_ERR_11(slc)  (0x828C + (slc) * 0x2000) /* 重复的rlast或者drsp的id命令异常 */

#define SC_ROUTER_CNT_ERR(slc)         (0x8290 + (slc) * 0x2000) /* 异常信息 */
#define SC_GID_BYP_CNT_ERR(slc)        (0x8294 + (slc) * 0x2000) /* 计数器减翻异常上报信息 */
#define SC_RT_MODULE_BUSY(slc)         (0x8298 + (slc) * 0x2000) /* router各个模块busy状态 */
#define SLC_DFX_MODULE(slc)            (0x8400 + (slc) * 0x2000) /* 各个Module的busy或门控状态 */
#define SLC_DFX_SCQ0(slc)              (0x8404 + (slc) * 0x2000) /* SCQ0的状态 */
#define SLC_DFX_SCQ1(slc)              (0x8408 + (slc) * 0x2000) /* SCQ1的状态 */
#define SLC_DFX_SQUE0(slc)             (0x8410 + (slc) * 0x2000) /* SQUE的状态0 */
#define SLC_DFX_SQUE1(slc)             (0x8414 + (slc) * 0x2000) /* SQUE的状态1 */
#define SLC_DFX_REQ0(slc)              (0x8418 + (slc) * 0x2000) /* SQUE的状态0 */
#define SLC_DFX_REQ1(slc)              (0x841C + (slc) * 0x2000) /* SQUE的状态1 */
#define SLC_DFX_BUF(slc)               (0x8420 + (slc) * 0x2000) /* BUF状态 */
#define SLC_DFX_OT(slc)                (0x8424 + (slc) * 0x2000) /* Outstanding状态 */
#define SLC_DFX_FIFO0(slc)             (0x8430 + (slc) * 0x2000) /* FIFO状态0 */
#define SLC_DFX_FIFO1(slc)             (0x8434 + (slc) * 0x2000) /* FIFO状态1 */
#define SLC_DFX_BP(slc)                (0x8440 + (slc) * 0x2000) /* 反压状态 */
#define SLC_DFX_BP1(slc)               (0x8444 + (slc) * 0x2000) /* 反压状态 */
#define SLC_DFX_ERR_INF1(slc)          (0x8454 + (slc) * 0x2000) /* 异常命令报错信息1 */
#define SLC_DFX_ERR_INF2(slc)          (0x8458 + (slc) * 0x2000) /* 异常命令报错信息2 */
#define SLC_DFX_ECCERR(slc)            (0x8460 + (slc) * 0x2000) /* ECC报错信息 */
#define SLC_DFX_MEM0(slc)              (0x8470 + (slc) * 0x2000) /* Memory的状态 */
#define SLC_DFX_MEM1(slc)              (0x8474 + (slc) * 0x2000) /* Memory的状态 */
#define SLC_DFX_SCH_CTRL(slc)          (0x8480 + (slc) * 0x2000) /* SCH模块的DFX控制 */
#define SLC_DFX_SCH_INFO0(slc)         (0x8484 + (slc) * 0x2000) /* SCH模块的DFX信息0 */
#define SLC_DFX_SCH_INFO1(slc)         (0x8488 + (slc) * 0x2000) /* SCH模块的DFX信息1 */
#define SLC_DFX_SCH_INFO2(slc)         (0x848C + (slc) * 0x2000) /* SCH模块的DFX信息2 */
#define SLC_DFX_SCH_INFO3(slc)         (0x8490 + (slc) * 0x2000) /* SCH模块的DFX信息3 */
#define SLC_DFX_SCH_INFO4(slc)         (0x8494 + (slc) * 0x2000) /* SCH模块的DFX信息4 */
#define SLC_DFX_CMOGEN(slc)            (0x84E0 + (slc) * 0x2000) /* CMO 模块关于CMOGEN的DFX控制 */
#define SLC_DFX_MST_ST0(slc)           (0x84F4 + (slc) * 0x2000) /* SLICE MST状态寄存器0 */
#define SLC_DFX_MST_ST1(slc)           (0x84F8 + (slc) * 0x2000) /* SLICE MST状态寄存器1 */
#define SLC_DFX_MST_ST2(slc)           (0x84FC + (slc) * 0x2000) /* SLICE MST状态寄存器2 */
#define SLC_DFX_MST_ST3(slc)           (0x8500 + (slc) * 0x2000) /* SLICE MST状态寄存器3 */
#define SLC_DFX_MST_ST4(slc)           (0x8504 + (slc) * 0x2000) /* SLICE MST状态寄存器4 */
#define SLC_PMU_CTRL(slc)              (0x8508 + (slc) * 0x2000) /* PMU功能使能寄存器 */
#define SLC_PMU_CNT8_11_MODE_SEL(slc)  (0x850C + (slc) * 0x2000) /* PMU统计counter8~11的模式选择使能 */

#define SLC_DFX_CMD_OUTSTANDING(slc, gid_idx)   (0x8510 + (slc) * 0x2000 + (gid_idx) * 8x4) /* SLICE命令outstanding信息 */
#define SLC_DFX_CMD_OSTD_ERR(slc)      (0x8550 + (slc) * 0x2000) /* SLICE命令outstanding异常信息 */
#define SLC_DFX_CMD_OSTD_ERR_CLR(slc)  (0x8554 + (slc) * 0x2000) /* SLICE命令outstanding异常信息清零 */

// 
#define CMO_CMD_ACCESS_ERR_ST 12
#define CMO_OVERFLOW_ST 13
#define CMO_PA_ERR_ST 14
#define CMO_BYWAY_ERR_ST 15
#define CMO_BYGID_ERR_ST 16

#define SC_FC_IRQ_MASK 0x1f000
#define SC_DFX_IRQ_MASK 0x7ff7000

#endif // __SC_HAL_H_