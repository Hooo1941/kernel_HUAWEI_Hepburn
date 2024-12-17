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
#ifndef __BSP_GMAC_H__
#define __BSP_GMAC_H__

#ifdef __cplusplus /* __cplusplus */
extern "C" {
#endif /* __cplusplus */

#ifndef TX_CHN_NR
#define TX_CHN_NR 3
#endif

#ifndef RX_CHN_NR
#define RX_CHN_NR 1
#endif

struct gmac_debug_info {
    unsigned int gmac_id;
    unsigned int is_open;

    /* link info */
    unsigned int line_speed;
    unsigned int xpcs_speed;
    unsigned int xpcs_link;
    unsigned int phy_state;
    unsigned int phy_speed;
    unsigned int phy_link;
    unsigned int phy_duplex;

    unsigned int tx_queue_cnt;
    unsigned int rx_queue_cnt;
    unsigned int ptp_cnt;

    /* tx info */
    unsigned int tx_queue_cur[TX_CHN_NR];
    unsigned int tx_queue_dirty[TX_CHN_NR];
    unsigned long long tx_pkt_cnt[TX_CHN_NR];
    unsigned long long spe_xmit_cnt[TX_CHN_NR];
    /* Transmit errors */
    unsigned long tx_carrier;
    unsigned long long tx_dropped;
    unsigned long long spe_tx_dropped;
    unsigned long spe_rd_full_dropped;
    unsigned long tx_get_maa_fail;

    /* rx info */
    unsigned long long rx_pkt_cnt[RX_CHN_NR];
    unsigned long long rx_errors[RX_CHN_NR];
    unsigned long long rx_dropped[RX_CHN_NR];

    unsigned int rx_skb_alloc[RX_CHN_NR];
    unsigned int rx_skb_free[RX_CHN_NR];
    unsigned int rx_skb_init_free[RX_CHN_NR];
    unsigned int rx_skb_free_ok[RX_CHN_NR];

    /*  gmac test    */
    unsigned long tx_desc_full[TX_CHN_NR];
    unsigned long spe_tx_desc_full[TX_CHN_NR];
    unsigned long enter_xmit[TX_CHN_NR];
    unsigned long enter_spe_xmit[TX_CHN_NR];
    unsigned long xmit_td_full[TX_CHN_NR];
    unsigned long gmac_release;
    unsigned long tx_avail[TX_CHN_NR];
    unsigned long spe_rd_recived;
    unsigned long spe_xmit_fail;
    unsigned long spe_xmit_tasklet;
    unsigned long write_queue_len;
    unsigned long write_queue_len_max;
    unsigned long xmit_enqueue;
    unsigned long xmit_dequeue;

    unsigned int maa_alloc_fail;
    unsigned int csd_locked_cnt;

    /* LPM status */
    unsigned long gmac_suspend;
    unsigned long gmac_resume;
};

struct gmac_report_info {
    unsigned int gmac_id;
    struct gmac_debug_info gmac_info[2]; /* 2 is gmac max number */
};

#ifdef CONFIG_NEW_STMMAC_ETH

int bsp_gmac_report_info(struct gmac_report_info *report_info, unsigned int len);

#else

static inline int bsp_gmac_report_info(struct gmac_report_info *report_info, unsigned int len)
{
    return 0;
}
#endif

#ifdef __cplusplus /* __cplusplus */
}
#endif /* __cplusplus */

#endif /*  __BSP_GMAC_H__ */
