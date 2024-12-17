/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: pfa_tft ap driver header
 * Author: shennaiyu
 * Create: 2020-10-12
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

#ifndef __PFA_TFT_H__
#define __PFA_TFT_H__

#include <mdrv_pfa.h>
#include <linux/pm.h>
#include <bsp_pfa.h>
#include <bsp_print.h>
#include <bsp_dt.h>

#undef THIS_MODU
#define THIS_MODU mod_pfa

#define pfa_tft_read(addr) readl_relaxed((unsigned char *)g_pfa_tft_ap.regs + (addr))
#define pfa_tft_write(val, addr) writel_relaxed((unsigned int)(val), (unsigned char *)g_pfa_tft_ap.regs + (addr))
#define SET1(reg, type, a1, b1) (((U_##type *)&(reg))->bits.a1 = (b1))
#define SET2(reg, type, a1, b1, a2, b2) \
    SET1(reg, type, a1, b1);            \
    SET1(reg, type, a2, b2)
#define SET3(reg, type, a1, b1, a2, b2, a3, b3) \
    SET2(reg, type, a1, b1, a2, b2);            \
    SET1(reg, type, a3, b3)
#define SET4(reg, type, a1, b1, a2, b2, a3, b3, a4, b4) \
    SET3(reg, type, a1, b1, a2, b2, a3, b3);            \
    SET1(reg, type, a4, b4)
#define SET5(reg, type, a1, b1, a2, b2, a3, b3, a4, b4, a5, b5) \
    SET4(reg, type, a1, b1, a2, b2, a3, b3, a4, b4);            \
    SET1(reg, type, a5, b5)
#define UPDATE0(reg, type) do { \
    (reg) = pfa_tft_read((type##_REG)); \
    pfa_tft_write(reg, type##_REG); \
    (void)(reg);                  \
} while (0)
#define UPDATE1(reg, type, a1, b1) do { \
    (reg) = pfa_tft_read((type##_REG)); \
    SET1(reg, type, a1, b1);    \
    pfa_tft_write(reg, type##_REG); \
    (void)(reg);                  \
} while (0)
#define UPDATE2(reg, type, a1, b1, a2, b2) do { \
    (reg) = pfa_tft_read((type##_REG));        \
    SET2(reg, type, a1, b1, a2, b2);   \
    pfa_tft_write(reg, type##_REG);        \
    (void)(reg);                         \
} while (0)
#define UPDATE3(reg, type, a1, b1, a2, b2, a3, b3) do { \
    (reg) = pfa_tft_read((type##_REG));                \
    SET3(reg, type, a1, b1, a2, b2, a3, b3);   \
    pfa_tft_write(reg, type##_REG);                \
    (void)(reg);                                 \
} while (0)
#define UPDATE4(reg, type, a1, b1, a2, b2, a3, b3, a4, b4) do { \
    (reg) = pfa_tft_read((type##_REG));                        \
    SET4(reg, type, a1, b1, a2, b2, a3, b3, a4, b4);   \
    pfa_tft_write(reg, type##_REG);                        \
    (void)(reg);                                         \
} while (0)
#define UPDATE5(reg, type, a1, b1, a2, b2, a3, b3, a4, b4, a5, b5) do { \
    (reg) = pfa_tft_read((type##_REG));                                \
    SET5(reg, type, a1, b1, a2, b2, a3, b3, a4, b4, a5, b5);   \
    pfa_tft_write(reg, type##_REG);                                \
    (void)(reg);                                                 \
} while (0)
#define UFIELD(utype, ptr) ((*((utype *)(ptr))).bits)

#define PFA_TFT_DL_DESC_LEN (64)
#define PFA_TFT_UL_DESC_LEN (512)
#define PFA_TFT_DL_AD_MEM_SIZE_DEFAULT 1024
#define INT_REG_SCALE (64)
#define PFA_TFT_UL_INT_MASK 0
#define PFA_TFT_DL_INT_MASK 0x13
#define PFA_TFT_RESET_DTS_ARRAY 5
#define RESERVE_THREE (3)
#define RESERVE_TWO (2)
#define NUM_OF_USERFIELD (3)
#define PACKET_HEAD_LEN (16)
#define PFA_TFT_MAX_RD_SIZE 1024
#define PFA_TFT_MAX_BD_SIZE 512

enum pfa_pkt_num {
    PFA_TFT_NUM_1,
    PFA_TFT_NUM_64,
    PFA_TFT_NUM_128,
    PFA_TFT_NUM_256,
    PFA_TFT_NUM_512,
    PFA_TFT_NUM_TOOLARGE,
    PFA_TFT_NUM_BOTTOM
};

union pfa_tft_addr_u {
    struct {
        unsigned long long ptr : 48;
        unsigned long long reserved : 16;
    } bits;
    unsigned long long addr;
};

union phy_addr_u {
    u64 addr;
    struct {
        u32 addr_l;
        u32 addr_h;
    } u32_t;
};

struct pfa_tft_ulbd_s {
    union pfa_tft_desc_attr_u attr;
    unsigned int input_pointer_l;
    unsigned int input_pointer_h;
    union pfa_tft_desc_id_u id;
    unsigned int reserved0[RESERVE_THREE];
    unsigned int user_field0;
    unsigned int user_field1;
    unsigned int user_field2;
};

union pfa_tft_ad_s {
    struct {
        unsigned int addr_l;
        unsigned int addr_h : 8;
        unsigned int resv : 24;
    } bits;
    dma_addr_t dma;
};

union pfa_tft_dlrd_result_u {
    struct {
        unsigned int bid_qosid : 8;
        unsigned int pf_type : 2;
        unsigned int ff_type : 1;
        unsigned int version_err : 1;
        unsigned int head_len_err : 1;
        unsigned int bd_pkt_noeq : 1;
        unsigned int pkt_parse_err : 1;
        unsigned int bd_cd_noeq : 1;
        unsigned int pkt_len_err : 1;
        unsigned int modem_id : 2;
        unsigned int reserved : 1;
        unsigned int dl_pfacial_flag : 1;
        unsigned int no_tcp_udp : 1;
        unsigned int ip_type : 1;
        unsigned int to_acpu_flag : 1;
        unsigned int pdu_session_id : 8;
    } err;
    unsigned int u32;
};

struct pfa_tft_dlrd_s {
    union pfa_tft_desc_attr_u attr;
    unsigned int input_pointer_l;
    unsigned int input_pointer_h;
    union pfa_tft_desc_id_u id;
    unsigned int reserved0[RESERVE_TWO];
    union pfa_tft_dlrd_result_u result;
    unsigned int user_field[NUM_OF_USERFIELD];
    unsigned int ip_header[PACKET_HEAD_LEN];
};

struct pfa_tft64_ad_s {
    union pfa_tft_addr_u output_ptr0;
    union pfa_tft_addr_u output_ptr1;
} __attribute__((packed, aligned(4)));

struct int_handler {
    char *name;
    unsigned int cnt;
    void (*callback)(void);
};

struct pfa_tft_ctx_s {
    struct device *dev;
    struct clk *clk;
    void *regs;
    device_node_s *np;
    dma_addr_t reg_phy;
    int irq;
    const char *clk_name;
    unsigned int need_reset;
    unsigned int reset_peri_crg[PFA_TFT_RESET_DTS_ARRAY];
    unsigned int soft_push;
    unsigned char *reset_reg;
    unsigned long long dma_mask;
    struct dev_pm_ops *pm;
    unsigned int dl_bdq0_busy;
    unsigned int dl_bdq1_busy;
    unsigned int dl_rdq_depth;
    unsigned int ul_bdq0_depth;
    unsigned int prepare_done;
    unsigned int suspend_done;
    unsigned int resume_done;

    int pfa_portno;
    void *adq0_base;
    dma_addr_t adq0_dma;
    dma_addr_t adq0_wptr;
    unsigned int adq0_size;

    void *adq1_base;
    dma_addr_t adq1_dma;
    dma_addr_t adq1_wptr;
    unsigned int adq1_size;

    void *rdq_base;
    dma_addr_t rdq_dma;
    unsigned int rdq_size;

    void *bdq0_base;
    dma_addr_t bdq0_dma;
    unsigned int bdq0_size;
    unsigned int bdq0_rptr_old;
    unsigned int *bdq0_rptr;
    unsigned int bdq0_wptr;
    dma_addr_t bdq0_rptr_dma;

    void *bdq1_base;
    dma_addr_t bdq1_dma;
    unsigned int bdq1_size;
    unsigned int bdq1_rptr;
    unsigned int bdq1_wptr;
    dma_addr_t bdq1_rptr_dma;
    unsigned long long bdq_recyle_cnt;
    unsigned int invalid_queue_len;
    unsigned int release_num_per_interval[PFA_TFT_NUM_BOTTOM];

    spinlock_t bd_lock;
#if (defined(CONFIG_PFA))
    struct pfa_port_comm_attr attr;
    struct pfa_tft_port_attr pfa_tft_attr;
    void *ulrd_rptr_virt;
#endif
};
extern struct pfa_tft_ctx_s g_pfa_tft_ap;

int pfa_tft_pm_prepare(void);
void pfa_tft_pm_suspend(void);
void pfa_tft_pm_resume(void);
int pfa_tft_init(void *pfa);
unsigned int pfa_get_adq_size_sel(unsigned int adq_size);
void pfa_tft_dl_cnt(void);

#endif
