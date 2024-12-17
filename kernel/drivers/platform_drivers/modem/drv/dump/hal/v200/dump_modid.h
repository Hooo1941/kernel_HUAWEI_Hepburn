/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
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

/* DRV	    0x00000000-0x0fffffff */
/* PAM	    0x10000000-0x2fffffff */
/* GU TTF   0x30000000-0x3fffffff */
/* GU WAS   0x40000000-0x4fffffff */
/* GU GAS   0x50000000-0x5fffffff */
/* GUNR NAS 0x60000000-0x6fffffff */
/* GUC PHY  0x70000000-0x75ffffff */
/* EASYRF   0x76000000-0x7fffffff */
/* LNAS	    0xA0000000-0xA3ffffff */
/* LRRC	    0xA4000000-0xA7ffffff */
/* NLL2	    0xA8000000-0xAfffffff */
/* VOICE    0xb0000000-0xb000ffff */
/* UPHY     0xc0000000-0xc0ffffff */
/* FESS BB  0xc1000000-0xc17fffff */
/* FESS RF  0xC1800000-0xC1Ffffff */
/* GPHY     0xc2000000-0xc27fffff */
/* WPHY     0xc2800000-0xc2ffffff */
/* LPHY     0xc3000000-0xc37fffff */
/* NPHY     0xc3800000-0xc3ffffff */
/* IMS      0xE0000000-0xE0ffffff */
/* 5GRRC    0xE1000000-0xEFffffff */
/* AP_DRV   0x00000000-0x0fffffff */

#define RDR_MODEM_CP_CPU_EXC_MOD_ID 0x1000

#define RDR_MODEM_CP_DRV_MOD_ID_START 0x00000000
#define RDR_MODEM_CP_DRV_MOD_ID_END 0x0FFFFFFF

#define RDR_MODEM_CP_PAM_MOD_ID_START 0x10000000
#define RDR_MODEM_CP_PAM_MOD_ID_END 0x2FFFFFFF

#define RDR_MODEM_CP_GUTTF_MOD_ID_START 0x30000000
#define RDR_MODEM_CP_GUTTF_MOD_ID_END 0x3FFFFFFF

#define RDR_MODEM_CP_GUWAS_MOD_ID_START 0x40000000
#define RDR_MODEM_CP_GUWAS_MOD_ID_END 0x4FFFFFFF

#define RDR_MODEM_CP_GUGAS_MOD_ID_START 0x50000000
#define RDR_MODEM_CP_GUGAS_MOD_ID_END 0x5FFFFFFF

#define RDR_MODEM_CP_GUNRNAS_MOD_ID_START 0x60000000
#define RDR_MODEM_CP_GUNRNAS_MOD_ID_END 0x6FFFFFFF

#define RDR_MODEM_CP_GUCPHY_MOD_ID_START 0x70000000
#define RDR_MODEM_CP_GUCPHY_MOD_ID_END 0x75FFFFFF

#define RDR_MODEM_CP_EASYRF_MOD_ID_START 0x76000000
#define RDR_MODEM_CP_EASYRF_MOD_ID_END 0x7FFFFFFF

#define RDR_MODEM_CP_LNAS_MOD_ID_START 0xA0000000
#define RDR_MODEM_CP_LNAS_MOD_ID_END 0xA3FFFFFF

#define RDR_MODEM_CP_LRRC_MOD_ID_START 0xA4000000
#define RDR_MODEM_CP_LRRC_MOD_ID_END 0xA7FFFFFF

#define RDR_MODEM_CP_NLL2_MOD_ID_START 0xA8000000
#define RDR_MODEM_CP_NLL2_MOD_ID_END 0xAFFFFFFF

#define RDR_MODEM_CP_VOICE_MOD_ID_START 0xB0000000
#define RDR_MODEM_CP_VOICE_MOD_ID_END 0xB000FFFF

#define RDR_MODEM_CP_UPHY_MOD_ID_START 0xC0000000
#define RDR_MODEM_CP_UPHY_MOD_ID_END 0xC0FFFFFF

#define RDR_MODEM_CP_FESSBB_MOD_ID_START 0xC1000000
#define RDR_MODEM_CP_FESSBB_MOD_ID_END 0xC17FFFFF

#define RDR_MODEM_CP_FESSRF_MOD_ID_START 0xC1800000
#define RDR_MODEM_CP_FESSRF_MOD_ID_END 0xC1FFFFFF

#define RDR_MODEM_CP_GPHY_MOD_ID_START 0xC2000000
#define RDR_MODEM_CP_GPHY_MOD_ID_END 0xC27FFFFF

#define RDR_MODEM_CP_WPHY_MOD_ID_START 0xC2800000
#define RDR_MODEM_CP_WPHY_MOD_ID_END 0xC2FFFFFF

#define RDR_MODEM_CP_LPHY_MOD_ID_START 0xC3000000
#define RDR_MODEM_CP_LPHY_MOD_ID_END 0xC37FFFFF

#define RDR_MODEM_CP_NPHY_MOD_ID_START 0xC3800000
#define RDR_MODEM_CP_NPHY_MOD_ID_END 0xC3FFFFFF

#define RDR_MODEM_CP_IMS_MOD_ID_START 0xE0000000
#define RDR_MODEM_CP_IMS_MOD_ID_END 0xE0FFFFFF

#define RDR_MODEM_CP_5GRRC_MOD_ID_START 0xE1000000
#define RDR_MODEM_CP_5GRRC_MOD_ID_END 0xEFFFFFFF

#define RDR_MODEM_AP_DRV_MOD_ID_START 0x00000000
#define RDR_MODEM_AP_DRV_MOD_ID_END 0x0fffffff

#define RDR_MODEM_MOD_ID_START 0x00000000
#define RDR_MODEM_MOD_ID_END 0xffffffff

/* MBB平台APSS  组件调用system_error接口复位映射ID      ，在bsp_adump.h中有同样定义 */
#define RDR_AP_DUMP_MODULE_MOD_ID (0x80000200)         /* AP异常，保存log，整机复位，mbb独有 */

/* rdr为CP分配的ID范围为0x82000000~0x82ffffff，定义在bsp_rdr.h中 */
#define RDR_MODEM_AP_MOD_ID 0x82000000                  /* AP异常，保存log，整机复位，phone和mbb都有 */
#define RDR_MODEM_CP_MOD_ID 0x82000001                  /* CP异常，保存log，phone和mbb都有 */
#define RDR_MODEM_REQ_SYSREBOOT_IN_SINGLERST_MOD_ID 0x82000002 /* 在上报了modem单独复位后，单独复位未完成前，来了需要整机复位的异常，使用此id整机复位 */
#define RDR_MODEM_CP_WDT_MOD_ID 0x82000003              /* CP看门狗异常，保存log，phone和mbb都有 */
#define RDR_MODEM_CP_RESET_SIM_SWITCH_MOD_ID 0x82000005 /* 切卡引起的CP单独复位，不保存log，phone独有 */
#define RDR_MODEM_CP_RESET_FAIL_MOD_ID 0x82000006       /* CP单独复位失败，保存log，phone独有 */
#define RDR_MODEM_CP_RESET_RILD_MOD_ID 0x82000007       /* RILD引起的CP单独复位，保存log，phone独有 */
#define RDR_MODEM_CP_RESET_3RD_MOD_ID 0x82000008        /* 3rd modem引起的CP单独复位，保存log，phone独有 */
#define RDR_MODEM_CP_RESET_REBOOT_REQ_MOD_ID 0x82000009 /* CP单独复位为桩时，整机复位，保存log，phone独有 */
#define RDR_MODEM_CP_RESET_USER_RESET_MOD_ID 0x82000010 /* CP单独复位,用户发起，不保存log,PHONE 独有 */
#define RDR_MODEM_CP_RESET_DLOCK_MOD_ID 0x82000011      /* modem总线挂死 */
#define RDR_MODEM_CP_NOC_MOD_ID 0x82000012              /* modem内部触发noc */
#define RDR_MODEM_AP_NOC_MOD_ID 0x82000013              /* modem访问AP触发noc */
#define RDR_MODEM_CP_RESET_FREQUENTLY_MOD_ID 0x82000014 /* c 核频繁单独复位，整机重启 */
/* 主线版本不再使用的rdr modid
RDR_MODEM_NR_MOD_ID 0x82000015
RDR_MODEM_NR_L2HAC_MOD_ID 0x82000016
RDR_MODEM_NR_CCPU_WDT 0x82000017
*/
#define RDR_MODEM_CP_RESET_DRA_MOD_ID 0x82000018        /* dra addr check fail exception, reset modem in mdmap */

#define RDR_MODEM_NOC_MOD_ID 0x82000030                 /* NOC异常, 保存log，phone独有, */
#define RDR_MODEM_DMSS_MOD_ID 0x82000031                /* DMSS异常, 保存log，phone独有, */
#define RDR_PHONE_MDMAP_PANIC_MOD_ID 0x80000030         /* modemap 代码panic后，ap传入的modid */
/* 主线版本不再使用的rdr modid
RDR_MODEM_CP_OAM_MOD_ID 0x82000043
RDR_MODEM_CP_GUL2_MOD_ID 0x82000044
RDR_MODEM_CP_CTTF_MOD_ID 0x82000045
RDR_MODEM_CP_CAS_MOD_ID 0x82000047
RDR_MODEM_CP_CPROC_MOD_ID 0x82000048
RDR_MODEM_CP_GUDSP_MOD_ID 0x8200004b
RDR_MODEM_CP_LPS_MOD_ID 0x8200004c
RDR_MODEM_CP_LMSP_MOD_ID 0x8200004d
*/
#define RDR_MODEM_CP_COMMON_MOD_ID 0x82000040
#define RDR_MODEM_CP_DRV_MOD_ID 0x82000041
#define RDR_MODEM_CP_OSA_MOD_ID 0x82000042
#define RDR_MODEM_CP_GUWAS_MOD_ID 0x82000046
#define RDR_MODEM_CP_GUGAS_MOD_ID 0x82000049
#define RDR_MODEM_CP_GUCNAS_MOD_ID 0x8200004a
#define RDR_MODEM_CP_TLDSP_MOD_ID 0x8200004e
#define RDR_MODEM_CP_CPHY_MOD_ID 0x8200004f
#define RDR_MODEM_CP_IMS_MOD_ID 0x82000050
#define RDR_MODEM_AP_DRV_MOD_ID 0x82000051
#define RDR_MODEM_LPM3_MOD_ID 0x82000057
#define RDR_MODEM_CP_EASYRF_MOD_ID 0x82000060
#define RDR_MODEM_CP_NRDSP_MOD_ID 0x82000061
#define RDR_MODEM_CP_PAM_MOD_ID 0x82000062
#define RDR_MODEM_CP_GUTTF_MOD_ID 0x82000063
#define RDR_MODEM_CP_GUNRNAS_MOD_ID 0x82000064
#define RDR_MODEM_CP_GUCPHY_MOD_ID 0x82000065
#define RDR_MODEM_CP_LNAS_MOD_ID 0x82000066
#define RDR_MODEM_CP_LRRC_MOD_ID 0x82000067
#define RDR_MODEM_CP_NLL2_MOD_ID 0x82000068
#define RDR_MODEM_CP_VOICE_MOD_ID 0x82000069
#define RDR_MODEM_CP_UPHY_MOD_ID 0x8200006A
#define RDR_MODEM_CP_FESSBB_MOD_ID 0x8200006B
#define RDR_MODEM_CP_FESSRF_MOD_ID 0x8200006C
#define RDR_MODEM_CP_GPHY_MOD_ID 0x8200006D
#define RDR_MODEM_CP_WPHY_MOD_ID 0x8200006E
#define RDR_MODEM_CP_LPHY_MOD_ID 0x8200006F
#define RDR_MODEM_CP_NPHY_MOD_ID 0x82000070
#define RDR_MODEM_CP_5GRRC_MOD_ID 0x82000071


#define RDR_MODEM_CODE_PATCH_REVERT_MOD_ID 0x82000100
#define RDR_MODEM_DRV_BUTT_MOD_ID 0x82ffffff
