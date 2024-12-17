/*
 *
 * dfa aquaila platform header file
 *
 * Copyright (c) 2023 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef _DFA_AQUILA_H
#define _DFA_AQUILA_H

#define RESERVED_DFA_PHYMEM_BASE 0x32000000

/* dfa sys ctrl */
#define acore_dfa_int_dfx(base)           ((base) + 0x0)
#define acore_dfa_int_mask(base)          ((base) + 0x4)
#define acore_dfa_int_set(base)           ((base) + 0x8)
#define acore_dfa_int_status(base)        ((base) + 0xC)
#define acore_dfa_req(base)               ((base) + 0x10)
#define dfa_cpu_halt(base)                ((base) + 0x24)
#define dfa_acore_int_en(base)            ((base) + 0x40)
#define dfa_qic_pd(base)                  ((base) + 0x64)
#define dfa_qic_pd_ack(base)              ((base) + 0x68)
#define acore_dfa_enabled(base)           ((base) + 0x100)
#define acore_dfa_commu_info2(base, bank) ((base) + 0x100 + 0x4 * (bank))
#define dfa_acore_commu_info2(base, bank) ((base) + 0x200 + 0x4 * (bank))

/* dfa que proc */
#define que_proc_msi_device_id(base)      (base)
#define que_proc_que_cr(base)             ((base) + 0x004UL)
#define que_proc_esilba(base)             ((base) + 0x010UL)
#define que_proc_esiuba(base)             ((base) + 0x014UL)
#define que_proc_ram_ctrl(base)           ((base) + 0x018UL)
#define que_proc_ram_status(base)         ((base) + 0x01CUL)
#define que_proc_sq_i_attr0(base, i)      ((base) + 0x020UL + (i) * 0x1CUL)
#define que_proc_sq_i_sqlba0(base, i)     ((base) + 0x024UL + (i) * 0x1CUL)
#define que_proc_sq_i_squba0(base, i)     ((base) + 0x028UL + (i) * 0x1CUL)
#define que_proc_cq_i_attr0(base, i)      ((base) + 0x030UL + (i) * 0x1CUL)
#define que_proc_cq_i_cqlba0(base, s)     ((base) + 0x034UL + (s) * 0x1CUL)
#define que_proc_cq_i_cquba0(base, s)     ((base) + 0x038UL + (s) * 0x1CUL)
#define que_proc_sq_i_sqh(base, i)        ((base) + 0x8E0UL + (i) * 0x8UL)
#define que_proc_sq_i_sqt(base, i)        ((base) + 0x8E4UL + (i) * 0x8UL)
#define que_proc_sq_i_sqis(base, i)       ((base) + 0xB60UL + (i) * 0x8UL)
#define que_proc_cq_i_cqh(base, i)        ((base) + 0xDE0UL + (i) * 0x8UL)
#define que_proc_cq_i_cqt(base, i)        ((base) + 0xDE4UL + (i) * 0x8UL)
#define que_proc_cq_i_cqis(base, i)       ((base) + 0x1060UL + (i) * 0xCUL)
#define que_proc_cq_i_cqie(base, i)       ((base) + 0x1064UL + (i) * 0xCUL)
#define que_proc_dfa_sq_i_rtc(base, j)    ((base) + 0x1420UL + (j) * 0x14UL)
#define que_proc_dfa_sq_i_rts(base, j)    ((base) + 0x1424UL + (j) * 0x14UL)
#define que_proc_dfa_sq_cfig(base, j)     ((base) + 0x1430UL + (j) * 0x14UL)

/* soc actrl */
#define actrl_ao_mem_ctrl2(base)              ((base) + 0x814UL)
#define actrl_hsdt_mem_status(base)           ((base) + 0x9D0UL)
#define actrl_hsdt_sec_ctrl0(base)            ((base) + 0xC04UL)

/* sctrl */
#define sctrl_scefusedata2(base)              ((base) + (0x00CUL))
#define sctrl_scefusedata8(base)              ((base) + (0x060UL))

/* peri crg */
#define crgperiph_peren0(base)                ((base) + (0x000UL))
#define crgperiph_perrsten0(base)             ((base) + (0x060UL))
#define crgperiph_perrstdis0(base)            ((base) + (0x064UL))
#define crgperiph_clkdiv0(base)               ((base) + (0x0A8UL))
#define crgperiph_clkdiv5(base)               ((base) + (0x0BCUL))

/* ao crg */
#define ao_crg_scperdis6(base)                ((base) + 0x1D4UL)

/* hsdt0 ctrl */
#define hsdt0_xctrl_qic_syspdc_pwrd_req(base) ((base) + 0x424UL)
#define hsdt0_xctrl_qic_syspdc_pwrd_ack(base) ((base) + 0x428UL)

/* hsdt0 crg */
#define hsdt0_crg_perrsten0(base)             ((base) + 0x060UL)
#endif