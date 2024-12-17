/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
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
 * * 3) Neither the name of ****** nor the names of its contributors may
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

#if !defined(__PRODUCT_CONFIG_PS_AP_H__)
#define __PRODUCT_CONFIG_PS_AP_H__


#ifndef FEATURE_ON
#define FEATURE_ON 1
#endif


#ifndef FEATURE_OFF
#define FEATURE_OFF 0
#endif


#ifndef CONFIG_PS_AP_ENG
#endif


#ifndef CONFIG_RNIC
#define CONFIG_RNIC
#endif


#ifndef CONFIG_RNIC_CUST_NAME
#endif


#ifndef CONFIG_RNIC_LAN_FORWARD
#endif


#ifndef CONFIG_RNIC_SRS
#define CONFIG_RNIC_SRS
#endif


#ifndef CONFIG_RNIC_GRO
#define CONFIG_RNIC_GRO
#endif


#ifndef CONFIG_RNIC_IMS
#define CONFIG_RNIC_IMS
#endif


#ifndef CONFIG_RNIC_TUN
#define CONFIG_RNIC_TUN
#endif


#ifndef CONFIG_RNIC_LTEV
#define CONFIG_RNIC_LTEV
#endif


#ifndef CONFIG_IPS
#define CONFIG_IPS
#endif


#ifndef CONFIG_IPS_NFEXT
#define CONFIG_IPS_NFEXT
#endif


#ifndef CONFIG_RNIC_PKTCAP
#endif


#ifndef CONFIG_RNIC_DPL
#define CONFIG_RNIC_DPL
#endif


#ifndef CONFIG_RNIC_DPL_LATENCY
#endif


#ifndef CONFIG_RNIC_D2D
#define CONFIG_RNIC_D2D
#endif


#ifndef CONFIG_RNIC_MULTI_IF
#endif


#ifndef CONFIG_RNIC_PFA_V2
#endif


#ifndef CONFIG_RNIC_WAKEUP
#endif


#ifndef CONFIG_RNIC_PRIORITY_MAP
#endif


#ifndef CONFIG_RNIC_FORWARD_MODE
#endif


#ifndef CONFIG_PCS
#define CONFIG_PCS
#endif


#ifndef CONFIG_PCS_BC_CAP_DIR
#define CONFIG_PCS_BC_CAP_DIR
#endif


#ifndef FEATURE_BASTET
#define FEATURE_BASTET
#endif


#ifndef CONFIG_RNIC_DUMP
#define CONFIG_RNIC_DUMP
#endif

#endif /*__PRODUCT_CONFIG_H__*/ 
