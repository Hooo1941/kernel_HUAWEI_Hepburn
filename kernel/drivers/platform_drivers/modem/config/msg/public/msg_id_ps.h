/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
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

#ifndef __MSG_ID_PS_H__
#define __MSG_ID_PS_H__

#include "msg_id_def.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* 提醒! 新增多实例PID请加到g_aPsPidModemIdMapTbl, 新增CDMA的PID请加到g_aPidForCdmaTbl */
#define MDRV_MSG_PS_MIDS \
        PS_MID_START = MSG_PS_MID_START,            /* = 0 */  \
        DEFINE_MSG_MID(OSA_RESERVED_TIMER)          /* = 1 */  \
        DEFINE_MSG_MID(OSA_RESERVED_2)              /* = 2 */  \
        DEFINE_MSG_MID(OSA_RESERVED_3)              /* = 3 */  \
        DEFINE_MSG_MID(OSA_RESERVED_4)              /* = 4 */  \
        /* 卡1的PID定义 */ \
        DEFINE_MSG_MID(I1_UEPS_PID_XSMS)            /* = 5 */  \
        DEFINE_MSG_MID(PID_RESERVED_6)              /* = 6 */  \
        DEFINE_MSG_MID(I1_WUEPS_PID_RESERVED_7)     /* = 7 */  \
        DEFINE_MSG_MID(I1_WUEPS_PID_AWAKE)          /* = 8 */  \
        DEFINE_MSG_MID(I1_DSP_PID_WAKE)             /* = 9 */  \
        DEFINE_MSG_MID(I1_UEPS_PID_XPDS)            /* = 10 */  \
        DEFINE_MSG_MID(I1_WUEPS_PID_WRR)            /* = 11 */  \
        DEFINE_MSG_MID(I1_WUEPS_PID_WCOM)           /* = 12 */  \
        DEFINE_MSG_MID(I1_WUEPS_PID_BMC)            /* = 13 */  \
        DEFINE_MSG_MID(I1_WUEPS_PID_PDCP)           /* = 14 */  \
        DEFINE_MSG_MID(I1_WUEPS_PID_RLC)            /* = 15 */  \
        DEFINE_MSG_MID(I1_WUEPS_PID_MAC)            /* = 16 */  \
        DEFINE_MSG_MID(I1_WUEPS_PID_RLCDL)          /* = 17 */  \
        DEFINE_MSG_MID(I1_WUEPS_PID_MACDL)          /* = 18 */  \
        DEFINE_MSG_MID(I1_WUEPS_PID_RESERVED_19)    /* = 19 */  \
        DEFINE_MSG_MID(I1_WUEPS_PID_USIM)           /* = 20 */  \
        DEFINE_MSG_MID(I1_MAPS_STK_PID)             /* = 21 */  \
        DEFINE_MSG_MID(I1_MAPS_PIH_PID)             /* = 22 */  \
        DEFINE_MSG_MID(I1_MAPS_PB_PID)              /* = 23 */  \
        DEFINE_MSG_MID(I1_UEPS_PID_GRM)             /* = 24 */  \
        DEFINE_MSG_MID(I1_UEPS_PID_DL)              /* = 25 */  \
        DEFINE_MSG_MID(I1_UEPS_PID_LL)              /* = 26 */  \
        DEFINE_MSG_MID(I1_UEPS_PID_SN)              /* = 27 */  \
        DEFINE_MSG_MID(I1_UEPS_PID_GAS)             /* = 28 */  \
        DEFINE_MSG_MID(I1_WUEPS_PID_MM)             /* = 29 */  \
        DEFINE_MSG_MID(I1_WUEPS_PID_MMC)            /* = 30 */  \
        DEFINE_MSG_MID(I1_WUEPS_PID_GMM)            /* = 31 */  \
        DEFINE_MSG_MID(I1_WUEPS_PID_MMA)            /* = 32 */  \
        DEFINE_MSG_MID(I1_WUEPS_PID_CC)             /* = 33 */  \
        DEFINE_MSG_MID(I1_WUEPS_PID_SS)             /* = 34 */  \
        DEFINE_MSG_MID(I1_WUEPS_PID_TC)             /* = 35 */  \
        DEFINE_MSG_MID(I1_WUEPS_PID_SMS)            /* = 36 */  \
        DEFINE_MSG_MID(I1_WUEPS_PID_RABM)           /* = 37 */  \
        DEFINE_MSG_MID(I1_WUEPS_PID_SM)             /* = 38 */  \
        DEFINE_MSG_MID(I1_WUEPS_PID_ADMIN)          /* = 39 */  \
        DEFINE_MSG_MID(I1_WUEPS_PID_TAF)            /* = 40 */  \
        DEFINE_MSG_MID(I1_WUEPS_PID_VC)             /* = 41 */  \
        DEFINE_MSG_MID(I1_WUEPS_PID_DRV_AGENT)      /* = 42 */  \
        DEFINE_MSG_MID(I1_UEPS_PID_MTA)             /* = 43 */  \
        DEFINE_MSG_MID(PID_RESERVED_44)             /* = 44 */  \
        DEFINE_MSG_MID(I1_DSP_PID_SLEEP)            /* = 45 */  \
        DEFINE_MSG_MID(I1_DSP_PID_IDLE)             /* = 46 */  \
        DEFINE_MSG_MID(I1_DSP_PID_APM)              /* = 47 */  \
        DEFINE_MSG_MID(I1_WUEPS_PID_SLEEP)          /* = 48 */  \
        DEFINE_MSG_MID(CPROC_PID_RM = 49)           /* = 49 */  \
        DEFINE_MSG_MID(MSPS_PID_1X_RLAC = 50)       /* = 50 */  \
        DEFINE_MSG_MID(MSPS_PID_1X_RMAC)            /* = 51 */  \
        DEFINE_MSG_MID(MSPS_PID_1X_RRLP)            /* = 52 */  \
        DEFINE_MSG_MID(MSPS_PID_1X_RTESTSO)         /* = 53 */  \
        DEFINE_MSG_MID(MSPS_PID_1X_FTESTSO)         /* = 54 */  \
        DEFINE_MSG_MID(MSPS_PID_1X_FLAC)            /* = 55 */  \
        DEFINE_MSG_MID(MSPS_PID_1X_FMAC)            /* = 56 */  \
        DEFINE_MSG_MID(MSPS_PID_1X_FRLP)            /* = 57 */  \
        DEFINE_MSG_MID(CTTF_PID_HRPD_RSLP)          /* = 58 */  \
        DEFINE_MSG_MID(CTTF_PID_HRPD_RPA)           /* = 59 */  \
        DEFINE_MSG_MID(CTTF_PID_HRPD_RSPS)          /* = 60 */  \
        DEFINE_MSG_MID(CTTF_PID_HRPD_RMAC)          /* = 61 */  \
        DEFINE_MSG_MID(CTTF_PID_HRPD_FSLP)          /* = 62 */  \
        DEFINE_MSG_MID(CTTF_PID_HRPD_FPA)           /* = 63 */  \
        DEFINE_MSG_MID(CTTF_PID_HRPD_FSPS)          /* = 64 */  \
        DEFINE_MSG_MID(CTTF_PID_HRPD_FMAC)          /* = 65 */  \
        DEFINE_MSG_MID(CTTF_PID_HRPD_SIG)           /* = 66 */  \
        DEFINE_MSG_MID(MSPS_PID_PPPC)               /* = 67 */  \
        DEFINE_MSG_MID(CPROC_PID_HRPD_CM = 68)      /* = 68 */  \
        DEFINE_MSG_MID(CPROC_PID_HRPD_SM = 69)      /* = 69 */  \
        DEFINE_MSG_MID(DSP_PID_PROCSTUB = 70)       /* = 70 */  \
        DEFINE_MSG_MID(CPROC_PID_1X_CM  = 71)       /* = 71 */  \
        DEFINE_MSG_MID(CPROC_PID_1X_SM  = 72)       /* = 72 */  \
        DEFINE_MSG_MID(CPROC_PID_1X_DSP = 73)       /* = 73 */  \
        DEFINE_MSG_MID(CPROC_PID_EVDO   = 74)       /* = 74 */  \
        DEFINE_MSG_MID(UEPS_PID_1XCASM  = 75)       /* = 75 */  \
        DEFINE_MSG_MID(UEPS_PID_1XCMEAS = 76)       /* = 76 */  \
        DEFINE_MSG_MID(UEPS_PID_1XCSRCH = 77)       /* = 77 */  \
        DEFINE_MSG_MID(UEPS_PID_HALMP   = 78)       /* = 78 */  \
        DEFINE_MSG_MID(UEPS_PID_HRUP    = 79)       /* = 79 */  \
        DEFINE_MSG_MID(UEPS_PID_HSCP    = 80)       /* = 80 */  \
        DEFINE_MSG_MID(UEPS_PID_HSP     = 81)       /* = 81 */  \
        DEFINE_MSG_MID(UEPS_PID_XREG    = 82)       /* = 82 */  \
        DEFINE_MSG_MID(UEPS_PID_XSD)                /* = 83 */  \
        DEFINE_MSG_MID(UEPS_PID_XCC)                /* = 84 */  \
        DEFINE_MSG_MID(UEPS_PID_HSD)                /* = 85 */  \
        DEFINE_MSG_MID(UEPS_PID_HLU)                /* = 86 */  \
        DEFINE_MSG_MID(UEPS_PID_HSM)                /* = 87 */  \
        DEFINE_MSG_MID(UEPS_PID_EHSM)               /* = 88 */  \
        DEFINE_MSG_MID(I0_UEPS_PID_XSMS)            /* = 89 */  \
        DEFINE_MSG_MID(I0_UEPS_PID_XPDS)            /* = 90 */  \
        DEFINE_MSG_MID(UEPS_PID_HRM)                /* = 91 */  \
        DEFINE_MSG_MID(CDMA_PID_ED     = 92)        /* = 92  CDMA 编解码使用 */  \
        DEFINE_MSG_MID(MSPS_PID_MIP    = 93)        /* = 93 */  \
        DEFINE_MSG_MID(PC_PID_TOOLRFU = 99)         /* = 99  工具已使用 */  \
        DEFINE_MSG_MID(PC_PID_TOOL = 100)           /* = 100 工具已使用 */  \
        /* 新增PID 添加到reserved位置，并将reserved删除 */  \
        DEFINE_MSG_MID(I0_WUEPS_PID_USIM)           /* = 101 */  \
        DEFINE_MSG_MID(I0_MAPS_STK_PID)             /* = 102 */  \
        DEFINE_MSG_MID(I0_MAPS_PIH_PID)             /* = 103 */  \
        DEFINE_MSG_MID(I0_MAPS_PB_PID)              /* = 104 */  \
        DEFINE_MSG_MID(I1_PS_PID_LPP)               /* = 105 */  \
        DEFINE_MSG_MID(NVIM_WRITE_DATA_PID)         /* = 106 */  \
        DEFINE_MSG_MID(PID_SC)                      /* = 107 */  \
        DEFINE_MSG_MID(I1_PS_PID_HPA)               /* = 108 */  \
        DEFINE_MSG_MID(I1_PS_PID_ERRC)              /* = 109 */  \
        DEFINE_MSG_MID(PID_RESERVED110)             /* = 110 */  \
        DEFINE_MSG_MID(UEPS_PID_BASTET)             /* = 111 */  \
        DEFINE_MSG_MID(TLPHY_PID_RTTAGENT)          /* = 112 */  \
        DEFINE_MSG_MID(PID_RESERVED108)             /* = 113 */  \
        DEFINE_MSG_MID(I0_WUEPS_PID_AWAKE)          /* = 114 */  \
        DEFINE_MSG_MID(I0_MSP_L4_L4A_PID)           /* = 115 */  \
        DEFINE_MSG_MID(I1_MSP_L4_L4A_PID)           /* = 116 */  \
        DEFINE_MSG_MID(I0_MSP_SYS_FTM_PID)          /* = 117 */  \
        DEFINE_MSG_MID(PID_RESERVED_118)            /* = 118 */  \
        DEFINE_MSG_MID(I0_MSP_LDSP_PID)             /* = 119 */  \
        DEFINE_MSG_MID(I1_MSP_LDSP_PID)             /* = 120 */  \
        DEFINE_MSG_MID(I1_MSP_SYS_FTM_PID)          /* = 121 */  \
        DEFINE_MSG_MID(I1_PS_PID_ERMM)              /* = 122 */  \
        DEFINE_MSG_MID(I1_PS_PID_MM)                /* = 123 */  \
        DEFINE_MSG_MID(PID_RESERVED_124)            /* = 124 */  \
        DEFINE_MSG_MID(I1_PS_PID_ESM)               /* = 125 */  \
        DEFINE_MSG_MID(I1_PS_PID_RABM)              /* = 126 */  \
        DEFINE_MSG_MID(I1_PS_PID_TC)                /* = 127 */  \
        DEFINE_MSG_MID(I0_UEPS_PID_GAS)             /* = 128 */  \
        DEFINE_MSG_MID(I0_UEPS_PID_DL)              /* = 129 */  \
        DEFINE_MSG_MID(I0_UEPS_PID_GRM)             /* = 130 */  \
        DEFINE_MSG_MID(I0_UEPS_PID_LL)              /* = 131 */  \
        DEFINE_MSG_MID(I0_UEPS_PID_SN)              /* = 132 */  \
        DEFINE_MSG_MID(I0_WUEPS_PID_WRR)            /* = 133 */  \
        DEFINE_MSG_MID(I0_WUEPS_PID_WCOM)           /* = 134 */  \
        DEFINE_MSG_MID(UEPS_PID_CST)                /* = 135 */  \
        DEFINE_MSG_MID(I0_WUEPS_PID_BMC)            /* = 136 */  \
        DEFINE_MSG_MID(I0_WUEPS_PID_PDCP)           /* = 137 */  \
        DEFINE_MSG_MID(I0_WUEPS_PID_RLC)            /* = 138 */  \
        DEFINE_MSG_MID(I0_WUEPS_PID_MAC)            /* = 139 */  \
        DEFINE_MSG_MID(I0_WUEPS_PID_RABM)           /* = 140 */  \
        DEFINE_MSG_MID(I0_WUEPS_PID_MMC)            /* = 141 */  \
        DEFINE_MSG_MID(I0_WUEPS_PID_MM)             /* = 142 */  \
        DEFINE_MSG_MID(I0_WUEPS_PID_GMM)            /* = 143 */  \
        DEFINE_MSG_MID(I0_WUEPS_PID_CC)             /* = 144 */  \
        DEFINE_MSG_MID(I0_WUEPS_PID_SM)             /* = 145 */  \
        DEFINE_MSG_MID(I0_WUEPS_PID_SMS)            /* = 146 */  \
        DEFINE_MSG_MID(I0_WUEPS_PID_SS)             /* = 147 */  \
        DEFINE_MSG_MID(I0_WUEPS_PID_TC)             /* = 148 */  \
        DEFINE_MSG_MID(WUEPS_PID_SMT)               /* = 149 */  \
        DEFINE_MSG_MID(I0_WUEPS_PID_ADMIN)          /* = 150 */  \
        DEFINE_MSG_MID(WUEPS_PID_MUX)               /* = 151 */  \
        DEFINE_MSG_MID(I0_WUEPS_PID_TAF)            /* = 152 */  \
        DEFINE_MSG_MID(WUEPS_PID_PPP)               /* = 153 */  \
        DEFINE_MSG_MID(WUEAPP_PID_MUX)              /* = 154 */  \
        DEFINE_MSG_MID(I0_PS_PID_NAS_COMM)          /* = 155 */  \
        DEFINE_MSG_MID(I1_PS_PID_NAS_COMM)          /* = 156 */  \
        DEFINE_MSG_MID(I0_WUEPS_PID_MMA)            /* = 157 */  \
        DEFINE_MSG_MID(I0_WUEPS_PID_SLEEP)          /* = 158 */  \
        DEFINE_MSG_MID(WUEPS_PID_CBS)               /* = 159 */  \
        DEFINE_MSG_MID(PS_PID_HCALL)                /* = 160 */  \
        DEFINE_MSG_MID(I1_WUEPS_PID_LCS)            /* = 161 */  \
        DEFINE_MSG_MID(I2_WUEPS_PID_LCS)            /* = 162 */  \
        DEFINE_MSG_MID(WUEPS_PID_APCID1)            /* = 163 */  \
        DEFINE_MSG_MID(WUEPS_PID_SPY)               /* = 164 */  \
        DEFINE_MSG_MID(I0_WUEPS_PID_VC)             /* = 165 */  \
        DEFINE_MSG_MID(I0_WUEPS_PID_LCS)            /* = 166 */  \
        DEFINE_MSG_MID(WUEPS_PID_APCID2)            /* = 167 */  \
        DEFINE_MSG_MID(WUEPS_PID_APCID3)            /* = 168 */  \
        DEFINE_MSG_MID(I0_PS_PID_LPP)               /* = 169 */  \
        DEFINE_MSG_MID(PS_PID_CSS)                  /* = 170 */  \
        DEFINE_MSG_MID(I0_PS_PID_ERRC)              /* = 171 */  \
        DEFINE_MSG_MID(I0_PS_PID_ERMM)              /* = 172 */  \
        DEFINE_MSG_MID(I0_PS_PID_MM)                /* = 173 */  \
        DEFINE_MSG_MID(I0_PS_PID_ESM)               /* = 174 */  \
        DEFINE_MSG_MID(I0_PS_PID_RABM)              /* = 175 */  \
        DEFINE_MSG_MID(I0_PS_PID_TC)                /* = 176 */  \
        DEFINE_MSG_MID(I0_PS_PID_IP)                /* = 177 */  \
        DEFINE_MSG_MID(I0_PS_PID_OM)                /* = 178 */  \
        DEFINE_MSG_MID(I0_PS_PID_MAC_UL)            /* = 179 */  \
        DEFINE_MSG_MID(I0_PS_PID_RLC_UL)            /* = 180 */  \
        DEFINE_MSG_MID(I0_PS_PID_MAC_DL)            /* = 181 */  \
        DEFINE_MSG_MID(I0_PS_PID_RLC_DL)            /* = 182 */  \
        DEFINE_MSG_MID(I0_PS_PID_PDCP_UL)           /* = 183 */  \
        DEFINE_MSG_MID(I0_PS_PID_PDCP_DL)           /* = 184 */  \
        DEFINE_MSG_MID(I0_PS_PID_HPA)               /* = 185 */  \
        DEFINE_MSG_MID(PS_PID_MUX)                  /* = 186 */  \
        DEFINE_MSG_MID(PS_PID_MUXOM)                /* = 187 */  \
        DEFINE_MSG_MID(I1_PS_PID_IP)                /* = 188 */  \
        DEFINE_MSG_MID(PID_RESERVED_189)            /* = 189 */  \
        DEFINE_MSG_MID(I0_WUEPS_PID_MACDL)          /* = 190 */  \
        DEFINE_MSG_MID(I0_WUEPS_PID_RLCDL)          /* = 191 */  \
        DEFINE_MSG_MID(PID_RESERVED_192)            /* = 192 */  \
        DEFINE_MSG_MID(UEPS_PID_DICC_C)             /* = 193 */  \
        DEFINE_MSG_MID(I0_WUEPS_PID_DRV_AGENT)      /* = 194 */  \
        DEFINE_MSG_MID(I0_PS_PID_PTT)               /* = 195 */  \
        DEFINE_MSG_MID(I1_PS_PID_OM)                /* = 196 */  \
        DEFINE_MSG_MID(UEPS_PID_CDS)                /* = 197 */  \
        DEFINE_MSG_MID(UEPS_PID_NDCLIENT)           /* = 198 */  \
        DEFINE_MSG_MID(WUEPS_PID_SAR)               /* = 199 */  \
        DEFINE_MSG_MID(UEPS_PID_DHCP)               /* = 200 */  \
        DEFINE_MSG_MID(I0_DSP_PID_APM)              /* = 201 */  \
        DEFINE_MSG_MID(DSP_PID_UPA)                 /* = 202 */  \
        DEFINE_MSG_MID(UEPS_PID_DHCPCLIENT)         /* = 203 */  \
        DEFINE_MSG_MID(I1_PS_PID_MAC_UL)            /* = 204 */  \
        DEFINE_MSG_MID(UEPS_PID_DHCPS)              /* = 205 */  \
        DEFINE_MSG_MID(I0_DSP_PID_WAKE)             /* = 206 */  \
        DEFINE_MSG_MID(I0_DSP_PID_SLEEP)            /* = 207 */  \
        DEFINE_MSG_MID(I0_DSP_PID_IDLE)             /* = 208 */  \
        DEFINE_MSG_MID(I0_UEPS_PID_MTA)             /* = 209 */  \
        DEFINE_MSG_MID(WUEPS_PID_NVIM_FLUSH)        /* = 210 */  \
        DEFINE_MSG_MID(HL1_PID_SNDCMD)              /* = 211 */  \
        DEFINE_MSG_MID(MSPS_PID_1X_FRESERVE)        /* = 212 */  \
        DEFINE_MSG_MID(MSPS_PID_1X_RRESERVE)        /* = 213 */  \
        DEFINE_MSG_MID(TPS_PID_RLC)                 /* = 214 */  \
        DEFINE_MSG_MID(TPS_PID_MAC)                 /* = 215 */  \
        DEFINE_MSG_MID(TPS_PID_PDC)                 /* = 216 */  \
        DEFINE_MSG_MID(TPS_PID_RRC)                 /* = 217 */  \
        DEFINE_MSG_MID(MSP_PID_BBP_AGENT)           /* = 218 */  \
        DEFINE_MSG_MID(UEPS_PID_MTC)                /* = 219 */  \
        DEFINE_MSG_MID(WUEPS_PID_CBPCA)             /* = 220 */  \
        DEFINE_MSG_MID(I0_WUEPS_PID_RESERVED_221)   /* = 221 */  \
        DEFINE_MSG_MID(I0_PS_PID_IMSA)              /* = 222 */  \
        DEFINE_MSG_MID(I0_PS_PID_IMSVA)             /* = 223 */  \
        DEFINE_MSG_MID(UEPS_PID_IPS_CCORE)          /* = 224 */  \
        DEFINE_MSG_MID(UEPS_PID_RRM)                /* = 225 */  \
        DEFINE_MSG_MID(PID_RESERVED_226)            /* = 226 */  \
        DEFINE_MSG_MID(WUEPS_PID_OM_BBP_MASTER)     /* = 227 */  \
        DEFINE_MSG_MID(PS_PID_SL_L2)                /* = 228 */  \
        DEFINE_MSG_MID(PS_PID_SL_L2_DL)             /* = 229 */  \
        DEFINE_MSG_MID(MSP_PID_DRX)                 /* = 230 */  \
        DEFINE_MSG_MID(VOLTE_PID_DIAG)              /* = 231 */  \
        DEFINE_MSG_MID(I2_WUEPS_PID_MM)             /* = 232 */  \
        DEFINE_MSG_MID(I2_WUEPS_PID_MMC)            /* = 233 */  \
        DEFINE_MSG_MID(I2_WUEPS_PID_GMM)            /* = 234 */  \
        DEFINE_MSG_MID(I2_WUEPS_PID_MMA)            /* = 235 */  \
        DEFINE_MSG_MID(I2_WUEPS_PID_CC)             /* = 236 */  \
        DEFINE_MSG_MID(I2_WUEPS_PID_SS)             /* = 237 */  \
        DEFINE_MSG_MID(I2_WUEPS_PID_TC)             /* = 238 */  \
        DEFINE_MSG_MID(I2_WUEPS_PID_SMS)            /* = 239 */  \
        DEFINE_MSG_MID(I2_WUEPS_PID_RABM)           /* = 240 */  \
        DEFINE_MSG_MID(I2_WUEPS_PID_SM)             /* = 241 */  \
        DEFINE_MSG_MID(I2_WUEPS_PID_ADMIN)          /* = 242 */  \
        DEFINE_MSG_MID(I2_WUEPS_PID_TAF)            /* = 243 */  \
        DEFINE_MSG_MID(I2_WUEPS_PID_VC)             /* = 244 */  \
        DEFINE_MSG_MID(I2_WUEPS_PID_DRV_AGENT)      /* = 245 */  \
        DEFINE_MSG_MID(I2_UEPS_PID_MTA)             /* = 246 */  \
        DEFINE_MSG_MID(PID_RESERVED_247)            /* = 247 */  \
        DEFINE_MSG_MID(I2_UEPS_PID_XSMS)            /* = 248 */  \
        DEFINE_MSG_MID(CPROC_PID_1X_CTAS  = 249)    /* = 249 */  \
        DEFINE_MSG_MID(I2_PS_PID_NAS_COMM = 250)    /* = 250 */  \
        DEFINE_MSG_MID(CCPU_PID_CBT)                /* = 251 */  \
        DEFINE_MSG_MID(CCPU_PID_PAM_OM)             /* = 252 */  \
        DEFINE_MSG_MID(DSP_PID_TLPHY)               /* = 253 */  \
        DEFINE_MSG_MID(I2_WUEPS_PID_USIM)           /* = 254 */  \
        DEFINE_MSG_MID(I2_MAPS_STK_PID)             /* = 255 */  \
        DEFINE_MSG_MID(I2_MAPS_PIH_PID)             /* = 256 */  \
        DEFINE_MSG_MID(I2_MAPS_PB_PID)              /* = 257 */  \
        DEFINE_MSG_MID(I2_UEPS_PID_GRM)             /* = 258 */  \
        DEFINE_MSG_MID(I2_UEPS_PID_DL)              /* = 259 */  \
        DEFINE_MSG_MID(I2_UEPS_PID_LL)              /* = 260 */  \
        DEFINE_MSG_MID(I2_UEPS_PID_SN)              /* = 261 */  \
        DEFINE_MSG_MID(I2_UEPS_PID_GAS)             /* = 262 */  \
        DEFINE_MSG_MID(PID_RESERVED_263)            /* = 263 */  \
        DEFINE_MSG_MID(I2_DSP_PID_SLEEP)            /* = 264 */  \
        DEFINE_MSG_MID(I2_DSP_PID_IDLE)             /* = 265 */  \
        DEFINE_MSG_MID(I2_DSP_PID_APM)              /* = 266 */  \
        DEFINE_MSG_MID(I2_WUEPS_PID_SLEEP)          /* = 267 */  \
        DEFINE_MSG_MID(I2_WUEPS_PID_RESERVED_268)   /* = 268 */  \
        DEFINE_MSG_MID(I2_WUEPS_PID_AWAKE)          /* = 269 */  \
        DEFINE_MSG_MID(I2_DSP_PID_WAKE)             /* = 270 */  \
        DEFINE_MSG_MID(I2_UEPS_PID_XPDS)            /* = 271 */  \
        DEFINE_MSG_MID(PID_RESERVED_272)            /* = 272 */  \
        DEFINE_MSG_MID(PID_UPHY_CHR_AGENT)          /* = 273 */  \
        DEFINE_MSG_MID(I0_DSP_PID_RTTSTUB)          /* = 274 */  \
        DEFINE_MSG_MID(I0_DSP_PID_RTTMATCH)         /* = 275 */  \
        DEFINE_MSG_MID(I0_DSP_PID_RTTPREEMPT)       /* = 276 */  \
        DEFINE_MSG_MID(WUEPS_PID_EDA)               /* = 277 */  \
        DEFINE_MSG_MID(I1_PS_PID_RLC_UL)            /* = 278 */  \
        DEFINE_MSG_MID(I1_PS_PID_MAC_DL)            /* = 279 */  \
        DEFINE_MSG_MID(I1_PS_PID_RLC_DL)            /* = 280 */  \
        DEFINE_MSG_MID(I1_PS_PID_PDCP_UL)           /* = 281 */  \
        DEFINE_MSG_MID(I1_PS_PID_PDCP_DL)           /* = 282 */  \
        DEFINE_MSG_MID(I1_PS_PID_PTT)               /* = 283 */  \
        DEFINE_MSG_MID(I1_PS_PID_IMSA)              /* = 284 */  \
        DEFINE_MSG_MID(I1_PS_PID_IMSVA)             /* = 285 */  \
        DEFINE_MSG_MID(I0_DSP_PID_RTTONLINE)        /* = 286 */  \
        DEFINE_MSG_MID(DSP_PID_CSDR_1X_CM_AGENT = 287)    /* = 287 */  \
        DEFINE_MSG_MID(DSP_PID_CSDR_1X_SM_AGENT = 288)    /* = 288 */  \
        DEFINE_MSG_MID(DSP_PID_CSDR_HRPD_CM_AGENT = 289)  /* = 289 */  \
        DEFINE_MSG_MID(DSP_PID_CSDR_HRPD_SM_AGENT = 290)  /* = 290 */  \
        DEFINE_MSG_MID(DSP_PID_GSDR_HIGHPRI_AGENT = 291)  /* = 291 */  \
        DEFINE_MSG_MID(LINKH_PID_LINKH_ARM)         /* = 292 */  \
        DEFINE_MSG_MID(PID_RESERVED_293)            /* = 293 */  \
        DEFINE_MSG_MID(I1_DSP_PID_TLPHY)            /* = 294 */  \
        DEFINE_MSG_MID(DSP_PID_RFM)                 /* = 295 */  \
        DEFINE_MSG_MID(I0_UEPS_PID_REGM)            /* = 296 */  \
        DEFINE_MSG_MID(I1_UEPS_PID_REGM)            /* = 297 */  \
        DEFINE_MSG_MID(I2_UEPS_PID_REGM)            /* = 298 */  \
        DEFINE_MSG_MID(I0_DSP_PID_GPHYLOW)          /* = 299 */  \
        DEFINE_MSG_MID(I1_DSP_PID_GPHYLOW)          /* = 300 */  \
        DEFINE_MSG_MID(I2_DSP_PID_GPHYLOW)          /* = 301 */  \
        DEFINE_MSG_MID(DSP_PID_UPHY_MNTN)           /* = 302 */  \
        DEFINE_MSG_MID(DSP_PID_BBA_CAL)             /* = 303 */  \
        DEFINE_MSG_MID(PHY_PID_PROCESS)             /* = 304 */  \
        DEFINE_MSG_MID(PHY_PID_STARTUP1)            /* = 305,时域版本. */  \
        DEFINE_MSG_MID(UEPS_PID_NRRC_0)             /* = 306 */  \
        DEFINE_MSG_MID(UEPS_PID_NRRM_0)             /* = 307,时域版本. */  \
        DEFINE_MSG_MID(I0_UEPS_PID_DSM)             /* = 308 */  \
        DEFINE_MSG_MID(I1_UEPS_PID_DSM)             /* = 309 */  \
        DEFINE_MSG_MID(I2_UEPS_PID_DSM)             /* = 310 */  \
        DEFINE_MSG_MID(MSPS_PID_PPPS)               /* = 311,ppps */  \
        DEFINE_MSG_MID(UEPS_PID_PSCOMM)             /* = 312,pscomm */  \
        DEFINE_MSG_MID(I1_DSP_PID_RTTSTUB)          /* = 313 */  \
        DEFINE_MSG_MID(I1_DSP_PID_RTTMATCH)         /* = 314 */  \
        DEFINE_MSG_MID(I1_DSP_PID_RTTPREEMPT)       /* = 315 */  \
        DEFINE_MSG_MID(I1_DSP_PID_RTTONLINE)        /* = 316 */  \
        DEFINE_MSG_MID(UEPS_PID_PSUPMEM)            /* = 317 */  \
        DEFINE_MSG_MID(I0_UEPS_PID_CCM)             /* = 318 */  \
        DEFINE_MSG_MID(I1_UEPS_PID_CCM)             /* = 319 */  \
        DEFINE_MSG_MID(I2_UEPS_PID_CCM)             /* = 320 */  \
        DEFINE_MSG_MID(I0_PS_PID_VERRC)             /* = 321 */ /* 新增LTE-V PID  */  \
        DEFINE_MSG_MID(I0_PS_PID_VSYNC)             /* = 322 */  \
        DEFINE_MSG_MID(I0_PS_PID_VMAC_UL)           /* = 323 */  \
        DEFINE_MSG_MID(I0_PS_PID_VRLC_UL)           /* = 324 */  \
        DEFINE_MSG_MID(I0_PS_PID_VPDCP_UL)          /* = 325 */  \
        DEFINE_MSG_MID(I0_PS_PID_VMAC_DL)           /* = 326 */  \
        DEFINE_MSG_MID(I0_PS_PID_VRLC_DL)           /* = 327 */  \
        DEFINE_MSG_MID(I0_PS_PID_VRBM_DL)           /* = 328 */  \
        DEFINE_MSG_MID(I0_PS_PID_VPDCP_DL)          /* = 329 */  \
        DEFINE_MSG_MID(I0_PS_PID_VHPA)              /* = 330 */ /* 新增LTE-V PID  */  \
        DEFINE_MSG_MID(I0_MAPS_EMAT_PID)            /* = 331 */ /* eSIM适配新增 */  \
        DEFINE_MSG_MID(I1_MAPS_EMAT_PID)            /* = 332 */ /* eSIM适配新增 */  \
        DEFINE_MSG_MID(I2_MAPS_EMAT_PID)            /* = 333 */ /* eSIM适配新增 */  \
        DEFINE_MSG_MID(I0_UEPS_PID_GUCALL)          /* = 334 */  \
        DEFINE_MSG_MID(I1_UEPS_PID_GUCALL)          /* = 335 */  \
        DEFINE_MSG_MID(I2_UEPS_PID_GUCALL)          /* = 336 */  \
        DEFINE_MSG_MID(I0_UEPS_PID_XCALL)           /* = 337 */  \
        DEFINE_MSG_MID(I1_UEPS_PID_XCALL)           /* = 338 */  \
        DEFINE_MSG_MID(I2_UEPS_PID_XCALL)           /* = 339 */  \
        DEFINE_MSG_MID(DSP_PID_BBA_RFM_LPC)         /* = 340 */  \
        DEFINE_MSG_MID(CCPU_PID_PAM_MFG)            /* = 341 */  \
        DEFINE_MSG_MID(I0_PS_PID_VNAS)              /* = 342 */ /* 新增LTE-V PID */  \
        DEFINE_MSG_MID(DSP_PID_BBA_CALC)            /* = 343 */ /* 新增 BBA CAL计算模块 */ \
        DEFINE_MSG_MID(I0_PS_PID_VTC)               /* = 344 */  \
        /* LR核在上面添加，NR和L2HAC核在下面添加 */ \
        DEFINE_MSG_MID(I0_UEPS_PID_NRRC)            /* = 345 */ \
        DEFINE_MSG_MID(I0_UEPS_PID_NRRM)            /* = 346 */ \
        DEFINE_MSG_MID(PAM_PID_NBT)                 /* = 347 */ \
        DEFINE_MSG_MID(MFG_PID_AT)                  /* = 348 */ \
        DEFINE_MSG_MID(UEPS_PID_L2MA)               /* = 349 */ \
        DEFINE_MSG_MID(I0_UEPS_PID_SDAP_UL)         /* = 350 */ \
        DEFINE_MSG_MID(I0_UEPS_PID_NPDCP_DL)        /* = 351 */ \
        DEFINE_MSG_MID(I0_UEPS_PID_NRLC_UL_NRT)     /* = 352 */ \
        DEFINE_MSG_MID(I0_UEPS_PID_NRLC_DL)         /* = 353 */ \
        DEFINE_MSG_MID(I0_UEPS_PID_NRLC_UL_RT)      /* = 354 */ \
        DEFINE_MSG_MID(I0_UEPS_PID_NMAC_UL_RT)      /* = 355 */ \
        DEFINE_MSG_MID(I0_UEPS_PID_NPDCP_UL_NRT)    /* = 356 */ \
        DEFINE_MSG_MID(I0_UEPS_PID_NMAC_DL)         /* = 357 */ \
        DEFINE_MSG_MID(I0_UEPS_PID_NRMM)            /* = 358 */ \
        DEFINE_MSG_MID(I0_UEPS_PID_NRSM)            /* = 359 */ \
        DEFINE_MSG_MID(I0_UEPS_PID_PCF)             /* = 360 */ \
        DEFINE_MSG_MID(I0_UEPS_PID_NREAP)           /* = 361 */ \
        DEFINE_MSG_MID(I0_PS_PID_NRTHROT)           /* = 362 */ \
        /* 上面是TSP 下面是HIF 核 */  \
        DEFINE_MSG_MID(DSP_PID_VOICE_RT_HIFI)       /* = 363 */ \
        DEFINE_MSG_MID(DSP_PID_VOICE_HIFI)          /* = 364 */ \
        DEFINE_MSG_MID(DSP_PID_AUDIO_RT)            /* = 365 */ \
        DEFINE_MSG_MID(DSP_PID_AUDIO)               /* = 366 */ \
        DEFINE_MSG_MID(DSP_PID_HIFI_OM)             /* = 367 */ \
        DEFINE_MSG_MID(DSP_PID_VDAL)                /* = 368 */ \
        DEFINE_MSG_MID(DSP_PID_VOICE_LOW)           /* = 369 */ \
        /* 上面是HIF 核 下面是 A 核 */  \
        DEFINE_MSG_MID(MSP_APP_DS_MUX_PID)          /* = 370 */ \
        DEFINE_MSG_MID(PS_PID_APP_NDIS)             /* = 371 */ \
        DEFINE_MSG_MID(PS_PID_APP_PPP)              /* = 372 */ \
        DEFINE_MSG_MID(PID_RESERVED_35)             /* = 373 */ \
        DEFINE_MSG_MID(MSP_APP_DS_DIAG_PID)         /* = 374 */ \
        DEFINE_MSG_MID(PID_RESERVED_375)            /* = 375 */ \
        DEFINE_MSG_MID(PID_RESERVED_37)             /* = 376 */ \
        DEFINE_MSG_MID(ACPU_PID_PCVOICE)            /* = 377 */ \
        DEFINE_MSG_MID(PID_RESERVED_36)             /* = 378 */ \
        DEFINE_MSG_MID(ACPU_PID_PB)                 /* = 379 */ \
        DEFINE_MSG_MID(ACPU_PID_PCS)                /* = 380 */ \
        DEFINE_MSG_MID(UEPS_PID_DICC_A)             /* = 381 */ \
        DEFINE_MSG_MID(ACPU_PID_RNIC)               /* = 382 */ \
        DEFINE_MSG_MID(WUEPS_PID_AT)                /* = 383 */ \
        DEFINE_MSG_MID(ACPU_PID_NFEXT)              /* = 384 */ \
        DEFINE_MSG_MID(ACPU_PID_ADS_UL)             /* = 385 */ \
        DEFINE_MSG_MID(ACPU_PID_ADS_DL)             /* = 386 */ \
        DEFINE_MSG_MID(PID_RESERVED_42)             /* = 387 */ \
        DEFINE_MSG_MID(ACPU_PID_TAFAGENT)           /* = 388 */ \
        DEFINE_MSG_MID(ACPU_PID_RESERVED_44)        /* = 389 */ \
        DEFINE_MSG_MID(ACPU_PID_RESERVED_45)        /* = 390 */ \
        DEFINE_MSG_MID(PID_RESERVED_39)             /* = 391 */ \
        DEFINE_MSG_MID(UEPS_PID_NDSERVER)           /* = 392 */ \
        DEFINE_MSG_MID(PID_RESERVED_40)             /* = 393 */ \
        DEFINE_MSG_MID(PID_RESERVED_41)             /* = 394 */ \
        DEFINE_MSG_MID(DSP_PID_VOICE_RT_ACPU)       /* = 395 */ \
        DEFINE_MSG_MID(DSP_PID_VOICE_ACPU)          /* = 396 */ \
        DEFINE_MSG_MID(MSP_PID_DIAG_FW)             /* = 397 */ \
        DEFINE_MSG_MID(PID_RESERVED_43)             /* = 398 */ \
        DEFINE_MSG_MID(PID_RESERVED_1001d)          /* = 399 */ \
        DEFINE_MSG_MID(PID_RESERVED_400)            /* = 400 */ \
        DEFINE_MSG_MID(ACPU_PID_BASTET_COMM)        /* = 401 */ \
        DEFINE_MSG_MID(ACPU_PID_VOICE_PROXY)        /* = 402 */ \
        DEFINE_MSG_MID(ACPU_PID_ALSA_AGENT)         /* = 403 */ \
        DEFINE_MSG_MID(PID_RESERVED_65570)          /* = 404 */ \
        DEFINE_MSG_MID(ACPU_PID_VOICE_AGENT)        /* = 405 */ \
        DEFINE_MSG_MID(PID_RESERVED_46)             /* = 406 */ \
        DEFINE_MSG_MID(PS_PID_DMS)                  /* = 407 */ \
        DEFINE_MSG_MID(I0_UEPS_PID_NPDCP_UL_RT)     /* = 408 */ \
        DEFINE_MSG_MID(UEPS_PID_APP_OM)             /* = 409 */ \
        DEFINE_MSG_MID(ACPU_PID_TAF)                /* = 410 */ \
        DEFINE_MSG_MID(I1_UEPS_PID_NRRC)            /* = 411 */ \
        DEFINE_MSG_MID(I1_UEPS_PID_NRRM)            /* = 412 */ \
        DEFINE_MSG_MID(I1_UEPS_PID_NRMM)            /* = 413 */ \
        DEFINE_MSG_MID(I1_UEPS_PID_PCF)             /* = 414 */ \
        DEFINE_MSG_MID(I1_UEPS_PID_NRSM)            /* = 415 */ \
        DEFINE_MSG_MID(I1_UEPS_PID_NREAP)           /* = 416 */ \
        DEFINE_MSG_MID(I0_UEPS_PID_APP_NR_DT)       /* = 417 */ \
        DEFINE_MSG_MID(I1_UEPS_PID_APP_NR_DT)       /* = 418 */ \
        DEFINE_MSG_MID(I0_UEPS_PID_APP_LTE_DT)      /* = 419 */ \
        DEFINE_MSG_MID(I1_UEPS_PID_APP_LTE_DT)      /* = 420 */ \
        DEFINE_MSG_MID(UEPS_PID_APP_NR_CHR)         /* = 421 */ \
        DEFINE_MSG_MID(UEPS_PID_APP_LTE_CHR)        /* = 422 */ \
        DEFINE_MSG_MID(I0_PS_PID_SCC)               /* = 423 */ \
        DEFINE_MSG_MID(I1_PS_PID_SCC)               /* = 424 */ \
        DEFINE_MSG_MID(UEPS_PID_L2DS)               /* = 425 */ \
        DEFINE_MSG_MID(UEPS_PID_L2LB)               /* = 426 */ \
        DEFINE_MSG_MID(UEPS_PID_ROHC)               /* = 427 */ \
        DEFINE_MSG_MID(UEPS_PID_TLS)                /* = 428 */ \
        DEFINE_MSG_MID(UEPS_PID_L2DLE)              /* = 429 */ \
        DEFINE_MSG_MID(PS_PID_HCM = 430)            /* = 430 */ \
        DEFINE_MSG_MID(PS_PID_HERRC = 431)          /* = 431 */ \
        DEFINE_MSG_MID(PS_PID_HSYNC = 432)          /* = 432 */ \
        DEFINE_MSG_MID(PS_PID_HPLUM = 433)          /* = 433 */ \
        DEFINE_MSG_MID(PS_PID_LOAD)                 /* = 434 */ /* LOAD模块 */ \
        DEFINE_MSG_MID(PAM_PID_MFGLOAD)             /* = 435 */ /* MFG常驻LOAD模块 */ \
        DEFINE_MSG_MID(PS_PID_HID2D_L2)             /* = 436 */ \
        DEFINE_MSG_MID(I0_PS_PID_CSF)               /* = 437 */ \
        DEFINE_MSG_MID(I1_PS_PID_CSF)               /* = 438 */ \
        DEFINE_MSG_MID(I2_PS_PID_CSF)               /* = 439 */ \
        DEFINE_MSG_MID(I0_PS_PID_MSF)               /* = 440 */ \
        DEFINE_MSG_MID(I1_PS_PID_MSF)               /* = 441 */ \
        DEFINE_MSG_MID(I2_PS_PID_MSF)               /* = 442 */ \
        DEFINE_MSG_MID(I0_PS_PID_SIP)               /* = 443 */ \
        DEFINE_MSG_MID(I1_PS_PID_SIP)               /* = 444 */ \
        DEFINE_MSG_MID(I2_PS_PID_SIP)               /* = 445 */ \
        DEFINE_MSG_MID(I0_PS_PID_UTPT)              /* = 446 */ \
        DEFINE_MSG_MID(I1_PS_PID_UTPT)              /* = 447 */ \
        DEFINE_MSG_MID(I2_PS_PID_UTPT)              /* = 448 */ \
        DEFINE_MSG_MID(I0_PS_PID_ACT)               /* = 449 */ \
        DEFINE_MSG_MID(PID_RESERVED_47)             /* = 450 */ \
        DEFINE_MSG_MID(PID_RESERVED_48)             /* = 451 */ \
        DEFINE_MSG_MID(I0_PS_PID_MVD)               /* = 452 */ \
        DEFINE_MSG_MID(PID_RESERVED_49)             /* = 453 */ \
        DEFINE_MSG_MID(PID_RESERVED_50)             /* = 454 */ \
        DEFINE_MSG_MID(I0_PS_PID_SPR = 455)         /* = 455 */ \
        DEFINE_MSG_MID(I1_PS_PID_SPR = 456)         /* = 456 */ \
        DEFINE_MSG_MID(I0_PS_PID_LRLC_UL_RT)        /* = 457 */ \
        DEFINE_MSG_MID(I0_PS_PID_LRLC_UL_NRT)       /* = 458 */ \
        DEFINE_MSG_MID(PS_PID_SRRC = 459)           /* = 459 */ \
        DEFINE_MSG_MID(I0_UEPS_PID_QOE = 460)       /* = 460 */ \
        DEFINE_MSG_MID(I0_PS_PID_BIP)              /* = 461 */  \
        DEFINE_MSG_MID(I1_PS_PID_BIP)              /* = 462 */  \
        DEFINE_MSG_MID(I2_PS_PID_BIP)              /* = 463 */  \
        DEFINE_MSG_MID(PS_PID_ATP)                 /* = 464 */ \
        DEFINE_MSG_MID(PS_PID_SL2)                 /* = 465 */ \
        DEFINE_MSG_MID(OSA_SYS_MID_BUTT)            /* 协议栈添加新的请在这个之前提交不要在这行之后提交  */  \
        PS_MID_END,                                 /* ps max mid num is 4096-1 = 4095 */

#ifdef __cplusplus
}
#endif
#endif
