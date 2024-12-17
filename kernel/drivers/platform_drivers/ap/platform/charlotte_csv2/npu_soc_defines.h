#ifndef __NPU_SOC_DEFINES_H__
#define __NPU_SOC_DEFINES_H__ 
#include "soc_npu_hwts_interface.h"
#define SOC_MID_NPU_HWTS_TINY 32
#define SOC_MID_NPU_HWTS_LITE 33
#define SOC_MID_NPU_TINY_CORE0 34
#define SOC_MID_NPU_LITE_CORE0 35
#define SOC_MID_NPU_TINY_CORE1 36
#define SOC_MID_NPU_LITE_CORE1 37
#define SOC_MID_NPU_SDMA_TINY 38
#define SOC_MID_NPU_SDMA_LITE 39
#define SOC_MID_NPU_TS_CPU 40
#define SOC_MID_NPU_SMMU_TCU 41
#define SOC_MID_IOMCU_M7 1
#define SOC_MID_ACPU_FCM_PP_QICE 46
#define SOC_MID_ISP_DMA 106
#define HWTS_DEFAULT_MID_VALUES (((SOC_MID_IOMCU_M7) << (SOC_NPU_HWTS_HWTS_HOST_MID_CFG_hwts_sensorhub_mid_START)) | \
    ((SOC_MID_ISP_DMA) << (SOC_NPU_HWTS_HWTS_HOST_MID_CFG_hwts_isp_mid_START)) | \
    ((SOC_MID_ACPU_FCM_PP_QICE) << (SOC_NPU_HWTS_HWTS_HOST_MID_CFG_hwts_acpu_mid_START)) | \
    ((SOC_MID_NPU_TS_CPU) << (SOC_NPU_HWTS_HWTS_HOST_MID_CFG_hwts_tscpu_mid_START)))
#define SOC_SMMU_SID_TSCPU 12
#define SOC_SMMU_SID_AICORE_SDMA_NONSEC 13
#define SOC_SMMU_SID_AICORE_SDMA_SEC 16
#define NPU_TZPC_SEC 0
#define NPU_TZPC_NONSEC 1
#define NPU_EASC_SEC 0
#define NPU_EASC_NONSEC 1
#define NPU_EASC_UNLOCKED 0
#define NPU_EASC_LOCKED 1
#define NPU_EASC_TYPE_RD 1
#define NPU_EASC_TYPE_WR 2
#define NPU_EASC_TYPE_WR_RD 3
#define NPU_EASC_REGION_OFFSET 0x24
#define NPU_EASC_REGION_MAX_COUNT 8
#define HWTS_SEC_EN_NONSEC_RW_AVAILABLE 0
#define AICORE_UNLOCK_MAGICNUM 0x19101920
#define AICORE_INTERLEAVE_GRANULARITY 2
#define AICORE_INIT_TIMEOUT 30000
#define HWTS_PREFETCH_HEAD_OFFSET 0xB0000
#define HWTS_PREFETCH_HEAD_PER_SIZE 128
#define NPU_HWTS_RTSQ_TINY_BITMAP 0x00000000000000FFULL
#define NPU_HWTS_RTSQ_LITE_UNSEC_BITMAP 0x0FFFFFF03FFFFF00ULL
#define NPU_HWTS_RTSQ_ISPNN_BITMAP 0x0000000FC0000000ULL
#define NPU_HWTS_RTSQ_LITE_SEC_BITMAP 0xF000000000000000ULL
#define HWTS_SQ_SLOT_SIZE 64
struct npu_hwts_static_swap_buff {
    SOC_NPU_HWTS_HWTS_SQ_BASE_ADDR0_UNION sq_base_addr0;
    SOC_NPU_HWTS_HWTS_SQ_BASE_ADDR1_UNION sq_base_addr1;
    SOC_NPU_HWTS_HWTS_SQ_CFG0_UNION sq_cfg0;
    SOC_NPU_HWTS_HWTS_SQ_CFG1_UNION sq_cfg1;
    SOC_NPU_HWTS_HWTS_SQ_CFG2_UNION sq_cfg2;
    unsigned int res0;
    SOC_NPU_HWTS_HWTS_SQ_CFG3_UNION sq_cfg3;
    unsigned int res1;
    SOC_NPU_HWTS_HWTS_CQ_BASE_ADDR0_UNION cq_base_addr0;
    SOC_NPU_HWTS_HWTS_CQ_BASE_ADDR1_UNION cq_base_addr1;
    SOC_NPU_HWTS_HWTS_CQ_CFG0_UNION cq_cfg0;
    SOC_NPU_HWTS_HWTS_SQ_CONTEXT_SWITCH_BUF_CTRL0_UNION sq_context_switch_ctrl0;
    SOC_NPU_HWTS_HWTS_SQ_CONTEXT_SWITCH_BUF_CTRL1_UNION sq_context_switch_ctrl1;
    unsigned int res2;
    unsigned int res3;
    unsigned int res4;
};
enum npu_hwts_sqe_type {
    NPU_HWTS_SQE_AICORE = 0,
    NPU_HWTS_SQE_AICPU = 1,
    NPU_HWTS_SQE_VECTOR_CORE = 2,
    NPU_HWTS_SQE_PLACE_HOLDER = 3,
    NPU_HWTS_SQE_EVENT_RECORD = 4,
    NPU_HWTS_SQE_EVENT_WAIT = 5,
    NPU_HWTS_SQE_NOTIFY_RECORD = 6,
    NPU_HWTS_SQE_NOTIFY_WAIT = 7,
    NPU_HWTS_SQE_WRITE_VALUE = 8,
    NPU_HWTS_SQE_MEMCPY = 9,
    NPU_HWTS_SQE_TYPE_RESV,
};
enum npu_hwts_ph_task_type {
    NPU_HWTS_PH_SQE_NORMAL = 0,
    NPU_HWTS_PH_SQE_LABEL_SWITCH = 1,
    NPU_HWTS_PH_SQE_LABEL_GOTO = 2,
    NPU_HWTS_PH_SQE_TYPE_RESV,
};
struct hwts_sqe_head {
    unsigned int type : 6;
    unsigned int l1_lock : 1;
    unsigned int l1_unlock : 1;
    unsigned int ie : 1;
    unsigned int pre_p : 1;
    unsigned int post_p : 1;
    unsigned int wr_cqe : 1;
    unsigned int rd_cond : 1;
    unsigned int res0 : 3;
    unsigned int block_dim : 16;
    unsigned int stream_id : 16;
    unsigned int task_id : 16;
};
struct hwts_kernel_sqe {
    unsigned int type : 6;
    unsigned int l1_lock : 1;
    unsigned int l1_unlock : 1;
    unsigned int ie : 1;
    unsigned int pre_p : 1;
    unsigned int post_p : 1;
    unsigned int wr_cqe : 1;
    unsigned int rd_cond : 1;
    unsigned int res0 : 3;
    unsigned int block_dim : 16;
    unsigned int stream_id : 16;
    unsigned int task_id : 16;
    unsigned int pc_addr_low;
    unsigned int pc_addr_high : 16;
    unsigned int kernel_credit : 8;
    unsigned int res1 : 3;
    unsigned int icache_prefetch_cnt : 5;
    unsigned int param_addr_low;
    unsigned int param_addr_high : 16;
    unsigned int l2_in_main : 8;
    unsigned int res2 : 8;
    unsigned int literal_addr_low;
    unsigned int literal_addr_high : 16;
    unsigned int res3 : 16;
    unsigned int literal_base_ub;
    unsigned int res4;
    unsigned int literal_buff_len;
    unsigned int res5;
    unsigned int l2_ctrl_addr_low;
    unsigned int l2_ctrl_addr_high : 16;
    unsigned int res6 : 16;
    unsigned long long aicore_stack_base;
};
struct hwts_label_switch_sqe {
    unsigned long long right;
    unsigned short true_label_idx;
    unsigned char condition;
    unsigned char res0[5];
};
struct hwts_label_goto_sqe {
    unsigned short label_idx;
    unsigned short res0[7];
};
struct hwts_ph_sqe {
    unsigned char type : 6;
    unsigned char l1_lock : 1;
    unsigned char l1_unlock : 1;
    unsigned char ie : 1;
    unsigned char pre_p : 1;
    unsigned char post_p : 1;
    unsigned char wr_cqe : 1;
    unsigned char res0 : 4;
    unsigned short task_type;
    unsigned short stream_id;
    unsigned short task_id;
    union {
        struct hwts_label_switch_sqe label_switch;
        struct hwts_label_goto_sqe label_goto;
    } u;
    unsigned int res2[10];
};
struct hwts_event_sqe {
    unsigned int type : 6;
    unsigned int res0 : 2;
    unsigned int ie : 1;
    unsigned int pre_p : 1;
    unsigned int post_p : 1;
    unsigned int wr_cqe : 1;
    unsigned int res1 : 20;
    unsigned short stream_id;
    unsigned short task_id;
    unsigned int event_id : 10;
    unsigned int res2 : 22;
    unsigned int res3 : 16;
    unsigned int kernel_credit : 8;
    unsigned int res4 : 8;
    unsigned int res5[12];
};
struct hwts_notify_sqe {
    unsigned char type : 6;
    unsigned char res0 : 2;
    unsigned char ie : 1;
    unsigned char pre_p : 1;
    unsigned char post_p : 1;
    unsigned char wr_cqe : 1;
    unsigned char reserved : 4;
    unsigned short res1;
    unsigned short stream_id;
    unsigned short task_id;
    unsigned int notify_id : 10;
    unsigned int res2 : 22;
    unsigned int res3 : 16;
    unsigned int kernel_credit : 8;
    unsigned int res4 : 8;
    unsigned int res5[12];
};
struct hwts_write_val_sqe {
    unsigned char type : 6;
    unsigned char res0 : 2;
    unsigned char ie : 1;
    unsigned char pre_p : 1;
    unsigned char post_p : 1;
    unsigned char wr_cqe : 1;
    unsigned char reserved : 4;
    unsigned short res1;
    unsigned short stream_id;
    unsigned short task_id;
    unsigned int reg_addr_low;
    unsigned short reg_addr_high;
    unsigned short awsize : 3;
    unsigned short snoop : 1;
    unsigned short res2 : 4;
    unsigned short awcache : 4;
    unsigned short awprot : 3;
    unsigned short va : 1;
    unsigned int write_value_part0;
    unsigned int write_value_part1;
    unsigned int write_value_part2;
    unsigned int write_value_part3;
    unsigned int write_value_part4;
    unsigned int write_value_part5;
    unsigned int write_value_part6;
    unsigned int write_value_part7;
    unsigned int res3[4];
};
struct hwts_memcpy_sqe {
    unsigned char type : 6;
    unsigned char l1_lock : 1;
    unsigned char l1_unlock : 1;
    unsigned char ie : 1;
    unsigned char pre_p : 1;
    unsigned char post_p : 1;
    unsigned char wr_cqe : 1;
    unsigned char reserved : 4;
    unsigned short res0;
    unsigned short stream_id;
    unsigned short task_id;
    unsigned int res1;
    unsigned int res2 : 16;
    unsigned int kernel_credit : 8;
    unsigned int res3 : 8;
    unsigned int ie_dma : 1;
    unsigned int mode : 3;
    unsigned int res4 : 4;
    unsigned int w_pattern : 8;
    unsigned int res5 : 4;
    unsigned int message0 : 12;
    unsigned int src_streamid : 8;
    unsigned int src_substreamid : 8;
    unsigned int dst_streamid : 8;
    unsigned int dst_substreamid : 8;
    unsigned int res6;
    unsigned int length;
    unsigned int src_addr_low;
    unsigned int src_addr_high : 16;
    unsigned int res7 : 15;
    unsigned int src_addr_high_p : 1;
    unsigned int dst_addr_low;
    unsigned int dst_addr_high : 16;
    unsigned int res8 : 15;
    unsigned int dst_addr_high_p : 1;
    unsigned int res10[4];
};
struct hwts_cqe {
    volatile unsigned short p : 1;
    volatile unsigned short w : 1;
    volatile unsigned short evt : 1;
    volatile unsigned short res0 : 1;
    volatile unsigned short sq_id : 10;
    volatile unsigned short res1 : 2;
    volatile unsigned short sq_head;
    volatile unsigned short stream_id;
    volatile unsigned short task_id;
    volatile unsigned int syscnt_low;
    volatile unsigned int syscnt_high;
};
enum intr_hub_l2_0_index {
    INTR_NPU2ACPU_PCR_IRQ = 0,
    INTR_NPU2ACPU_RST_REQ,
    INTR_NPU2ACPU_TS_WDOG_INT,
    INTR_NPU2ACPU_TS_WDOG_RES,
    INTR_NPU2ACPU_HWTS_DFX_AF_INT,
    INTR_NPU2ACPU_HWTS_DEBUG_NS,
    INTR_NPU2ACPU_HWTS_AICPU_NS,
    INTR_NPU2ACPU_EASC,
    INTR_NPU2ACPU_OVER_CURRENT,
    INTR_NPU2QICE_SW_DONE_ACPU,
    NPU_INTR_HUB_L2_0_COUNT,
};
enum intr_hub_l2_1_index {
    INTR_NPU2ACPU_HWTS_DEBUG_S = 0,
    INTR_NPU2ACPU_HWTS_AICPU_S,
    NPU_INTR_HUB_L2_1_COUNT,
};
enum intr_hub_l2_2_index {
    INTR_NPU2ACPU_TINY_HWTS_DFX_AF_INT,
    INTR_NPU2ACPU_TINY_HWTS_DEBUG,
    INTR_NPU2ACPU_DMMU,
    NPU_INTR_HUB_L2_2_COUNT,
};
struct npu_easc_region_cfg {
    unsigned int start_addr_low;
    unsigned int start_addr_high;
    unsigned int end_addr_low;
    unsigned int end_addr_high;
    unsigned int mid0 : 8;
    unsigned int mid1 : 8;
    unsigned int mid2 : 8;
    unsigned int mid3 : 8;
    unsigned int secure : 1;
    unsigned int wr_rd_type : 2;
};
#define AICORE_BASE_ADDR_SHIFT 0x14
#endif
