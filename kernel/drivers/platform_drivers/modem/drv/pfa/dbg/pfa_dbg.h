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

#ifndef __PFA_DBG_H__
#define __PFA_DBG_H__

#include <bsp_print.h>
#include <bsp_dump.h>
#include <bsp_om_enum.h>
#include "pfa.h"

#undef THIS_MODU
#define THIS_MODU mod_pfa

#define PFA_ERR_MODU (0x1000)
#define PFA_ERR_BASE (PFA_ERR_MODU << 16)

#define PFA_RD_LOOP_CNT_MAX 10000

/* pfa msg system */
#define PFA_MSG_ERR BIT(0)
#define PFA_MSG_TRACE BIT(1)
#define PFA_MSG_INFO BIT(2)
#define PFA_MSG_DBG BIT(3)
#define PFA_MSG_BUG BIT(4)
#define PFA_MSG_DUMP BIT(5)
#define PFA_MSG_PRINT BIT(6)
#define PFA_MSG_TUPLE BIT(7)
#define PFA_MSG_CPU_SKB BIT(8)
#define PFA_MSG_XFRM BIT(9)

#define pfa_bug(pfa) do { \
    if ((pfa)->dbg_level & PFA_DBG_BUG_ON_STOP) {     \
        (pfa)->bugon_flag = 1;                        \
    }                                                 \
    if ((pfa)->dbg_level & PFA_DBG_BUG_ON) {          \
        system_error(DRV_ERRNO_PFA_BUG, 0, 0, 0, 0); \
    }                                                 \
    if ((pfa)->dbg_level & PFA_DBG_WARN_ON) {         \
        WARN_ON_ONCE(1);                              \
    }                                                 \
} while (0)

#define PFA_ERR(fmt, ...) do { \
    bsp_err(fmt, ##__VA_ARGS__); \
} while (0)

#define PFA_ERR_ONCE(fmt, ...) do { \
    if (!g_pfa.print_once_flags) {  \
        bsp_err("Warning once \n");  \
        bsp_err(fmt, ##__VA_ARGS__); \
        g_pfa.print_once_flags = 1; \
    }                                \
} while (0)

#define PFA_BUG(args, condition) do { \
    if (condition) {                      \
        bsp_err args;                     \
    }                                     \
                                              \
        if (g_pfa.msg_level & PFA_MSG_BUG) { \
        BUG_ON(condition);                \
    }                                     \
} while (0)

#if defined(CONFIG_PFA_DEBUG)
#define PFA_INFO(fmt, ...) do { \
    if (g_pfa.msg_level & PFA_MSG_INFO)                \
        bsp_info("<%s> " fmt, __func__, ##__VA_ARGS__); \
} while (0)
#define PFA_TRACE(fmt, ...) do { \
    if (g_pfa.msg_level & PFA_MSG_TRACE)                \
        bsp_info("<%s:> " fmt, __func__, ##__VA_ARGS__); \
} while (0)
#define PFA_DBG(fmt, ...) do { \
    if (g_pfa.msg_level & PFA_MSG_DBG)                   \
        bsp_debug("<%s:> " fmt, __func__, ##__VA_ARGS__); \
} while (0)

#define PFA_DUMP(fmt, ...) do { \
    if (g_pfa.msg_level & PFA_MSG_DUMP)                \
        bsp_err("<%s:> " fmt, __func__, ##__VA_ARGS__); \
} while (0)

#define PFA_TUPLE(fmt, ...) do { \
    if (g_pfa.msg_level & PFA_MSG_TUPLE)                \
        bsp_info("<%s:> " fmt, __func__, ##__VA_ARGS__); \
} while (0)

#define PFA_SKB_DUMP(skb) do { \
    if (g_pfa.msg_level & PFA_MSG_PRINT) {                                                          \
        int len = min((int)64, (int)(skb->len));                                                     \
        print_hex_dump(KERN_ERR, "skb dump: ", DUMP_PREFIX_ADDRESS, 16, 1, (skb->data), len, false); \
        bsp_debug("\n");                                                                             \
    }                                                                                                \
} while (0)
#else

#define PFA_INFO(fmt, ...)
#define PFA_TRACE(fmt, ...)
#define PFA_DUMP(fmt, ...)
#define PFA_TUPLE(fmt, ...)
#define PFA_SKB_DUMP(skb)
#define PFA_DBG(fmt, ...)

#endif

/* pfa dbg system */
#define PFA_DBG_BUG_ON BIT(0)
#define PFA_DBG_WARN_ON BIT(1)
#define PFA_DBG_TD_RESULT BIT(2)
#define PFA_DBG_RD_RESULT BIT(3)
#define PFA_DBG_RD_SAVE BIT(6)
#define PFA_DBG_BUG_ON_STOP BIT(7)
#define PFA_DBG_HIDS_UPLOAD BIT(8)
#define PFA_DBG_PTR_WALK BIT(9)

#define pfa_msg_str() __FUNCTION__  // " line "##__LINE__
#define UDP_PKT_LEN 1500
#define NUM_PER_SLICE (32768)
#define BIT_NUM_PER_BYTE (8)
#define PFA_SPEED_BTOMB (1024 * 1024)

struct pfa_om_reg_set {
    unsigned int start;
    unsigned int end;
};

enum pfa_level {
    PFA_DBG_LEVEL0,
    PFA_DBG_LEVEL1,
    PFA_DBG_LEVEL2,
    PFA_DBG_LEVEL3,
    PFA_DBG_LEVEL4,
    PFA_DBG_LEVEL5,
    PFA_DBG_BOTTOM
};

/* pfa debug functions for dbgfs */
void pfa_print_td(unsigned int port_num, unsigned int td_pos);
void pfa_print_rd(unsigned int port_num, unsigned int rd_pos);
void pfa_dev_setting(void);
void pfa_dev_info(void);
void pfa_port_setting(unsigned int portno);
void pfa_port_rd_info(unsigned int portno);
void pfa_port_td_info(unsigned int portno);
void pfa_ipfw_setting(void);
void pfa_port_info_all(unsigned int port_num);
void pfa_get_port_type(unsigned int portno, enum pfa_enc_type enc_type);

void pfa_print_one_td(struct pfa_td_desc *cur_td);
void pfa_print_one_rd(struct pfa_rd_desc *cur_rd);
void pfa_print_pkt_info(unsigned char *data);
void pfa_om_dump_init(struct pfa *pfa);

int pfa_rd_drop_en(int drop_stub_en);
void pfa_help(void);
void pfa_all_port_show(void);
void pfa_cpu_port_info(void);
void pfa_td_info_adv(unsigned int portno);
void pfa_rd_info_adv(unsigned int portno, unsigned int password);
void pfa_ad_info(void);
void pfa_print_pkt_ipv4(unsigned char *data);
void pfa_print_pkt_ipv6(struct iphdr *iph);
void pfa_print_pkt_info(unsigned char *data);
int pfa_set_hids_ul(unsigned int enable, unsigned int portno);
int pfa_set_result_record(unsigned int level);
int pfa_set_msgdbg(unsigned int msg_level);
int pfa_enable_dfs(int en_dfs);
void pfa_dfs_time_set(int time);
void pfa_dfs_time_get(void);
void pfa_dfs_start_limit_set(int value_kbps);
void pfa_dfs_start_limit_get(void);
void pfa_dfs_grade_set(int x, int val1, int val2, int val3);
void pfa_dfs_grade_get(void);
void pfa_dfs_grade_get(void);
void pfa_hds_transreport(void);
int pfa_get_om_reg_size(void);
void pfa_dump_hook(void);
void pfa_port_speed(void);
void pfa_set_rd_loop_cnt(unsigned long long cnt);
void pfa_get_rd_time(void);
void pfa_port_copy_en(unsigned int portno, unsigned int enable);
void pfa_set_bypass(unsigned int port_num, unsigned int bypass_dir, unsigned int en);
void pfa_checksum(struct pfa *pfa, unsigned long long dra_l2_addr, unsigned int len);

#endif /* __PFA_DBG_H__ */
