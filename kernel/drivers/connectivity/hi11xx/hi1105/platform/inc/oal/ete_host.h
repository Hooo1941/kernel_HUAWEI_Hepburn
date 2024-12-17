/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description: ete_host.h header file
 * Author: @CompanyNameTag
 */

#ifndef OAL_ETE_HOST_H
#define OAL_ETE_HOST_H
#include "ete_comm.h"
#include "pcie_comm.h"

typedef struct _pcie_ete_trans_txbuf__ {
    uint64_t buff_cpu_addr;
    uint64_t buff_io_addr;
    uint32_t buff_items;
    uint32_t buff_size;
    uint32_t buff_wr;         // 用于记录当前buf使用情况
    uint32_t buff_wr_flag;
    uint32_t buff_rd;
    uint32_t buff_rd_flag;
} pcie_ete_trans_txbuf;

typedef struct _oal_ete_chan_h2d_res__ {
    pcie_ete_ringbuf ring;
    pcie_ete_trans_txbuf tx_buff;
    uint32_t chan_idx;
    int32_t ring_max_num;    /* max num descrs */
    int32_t enabled;
    oal_netbuf_head_stru txq;           /* 正在发送中的netbuf队列 */
    oal_atomic tx_sync_cond;
    oal_spin_lock_stru lock;
} oal_ete_chan_h2d_res;

typedef struct _oal_ete_chan_d2h_res__ {
    pcie_ete_ringbuf ring;
    uint32_t chan_idx;
    int32_t max_len;
    int32_t ring_max_num;    /* max num descrs */
    int32_t enabled;
    oal_netbuf_head_stru rxq;           /* 正在接收中的netbuf队列 */
    oal_spin_lock_stru lock;

    /* kernel thread res */
    oal_atomic rx_cond;
    oal_wait_queue_head_stru rx_wq;
    oal_mutex_stru rx_mem_lock;
    struct task_struct *pst_rx_chan_task;
    char* task_name;
    void* task_data;
} oal_ete_chan_d2h_res;

typedef struct _oal_ete_reg__ {
    void* host_intr_sts_addr;
    void* host_intr_clr_addr;
    void* host_intr_mask_addr;
    void* ete_intr_sts_addr;
    void* ete_intr_clr_addr;
    void* ete_intr_mask_addr;
    void* ete_dr_empty_sts_addr;
    void* ete_dr_empty_clr_addr;
    void* h2d_intr_addr;
} oal_ete_reg;

typedef struct _oal_ete_res__ {
    oal_ete_chan_h2d_res h2d_res[ETE_TX_CHANN_MAX_NUMS];
    oal_ete_chan_d2h_res d2h_res[ETE_RX_CHANN_MAX_NUMS];
    unsigned int host_intr_mask;
    unsigned int pcie_ctl_ete_intr_mask;
    unsigned int ete_ch_dr_empty_intr_mask;
    oal_ete_reg reg;
} oal_ete_res;

OAL_STATIC OAL_INLINE void oal_ete_print_ringbuf_info(pcie_ete_ringbuf *pst_ringbuf)
{
    oal_io_print("[%p]item_len:%u, cpu_addr:0x%llx, io_addr:0x%llx\n",
                 pst_ringbuf, pst_ringbuf->item_len, pst_ringbuf->cpu_addr, pst_ringbuf->io_addr);
    oal_io_print("wr:%u, rd:%u, size:%u, items:%u\n",
                 pst_ringbuf->wr, pst_ringbuf->rd, pst_ringbuf->size, pst_ringbuf->items);
    oal_io_print("wflag:%u  rflag:%u\n",
                 pst_ringbuf->wflag, pst_ringbuf->rflag);
}
#endif
