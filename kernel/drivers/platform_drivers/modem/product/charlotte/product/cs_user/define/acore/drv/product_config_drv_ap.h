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

#if !defined(__PRODUCT_CONFIG_DRV_AP_H__)
#define __PRODUCT_CONFIG_DRV_AP_H__


#ifndef FEATURE_ON
#define FEATURE_ON 1
#endif


#ifndef FEATURE_OFF
#define FEATURE_OFF 0
#endif


#ifndef ENABLE_BUILD_VARS
#define ENABLE_BUILD_VARS
#endif


#ifndef BSP_CONFIG_PHONE_TYPE
#define BSP_CONFIG_PHONE_TYPE
#endif


#ifndef CONFIG_VERSION_STUB
#endif


#ifndef HW_VERSION_STUB
#define HW_VERSION_STUB 0x2DF5FC15
#endif


#ifndef BOARD_VERSION_STUB
#define BOARD_VERSION_STUB 0x0
#endif


#ifndef CONFIG_NVIM
#define CONFIG_NVIM
#endif


#ifndef CONFIG_NV_FUSION_MSG
#define CONFIG_NV_FUSION_MSG
#endif


#ifndef FEATURE_NVA_ON
#define FEATURE_NVA_ON
#endif


#ifndef FEATURE_NV_FLASH_ON
#endif


#ifndef FEATURE_NV_EMMC_ON
#define FEATURE_NV_EMMC_ON
#endif


#ifndef FEATURE_NV_LFILE_ON
#endif


#ifndef FEATURE_NV_RFILE_ON
#endif


#ifndef FEATURE_DIS_HMAC_CHECK
#endif


#ifndef ENABLE_BUILD_SOCP
#define ENABLE_BUILD_SOCP
#endif


#ifndef CONFIG_DEFLATE
#endif


#ifndef SOCP_V300
#define SOCP_V300
#endif


#ifndef CONFIG_BBPDS
#define CONFIG_BBPDS
#endif


#ifndef ENABLE_BUILD_PRINT
#define ENABLE_BUILD_PRINT
#endif


#ifndef FEATURE_HDS_PRINTLOG
#define FEATURE_HDS_PRINTLOG FEATURE_OFF
#endif


#ifndef FEATURE_HDS_TRANSLOG
#define FEATURE_HDS_TRANSLOG FEATURE_OFF
#endif


#ifndef CONFIG_DIAG_SYSTEM
#define CONFIG_DIAG_SYSTEM
#endif


#ifndef DIAG_SYSTEM_5G
#define DIAG_SYSTEM_5G
#endif


#ifndef DIAG_SYSTEM_FUSION
#define DIAG_SYSTEM_FUSION
#endif


#ifndef CONFIG_DIAG_NETLINK
#define CONFIG_DIAG_NETLINK
#endif


#ifndef CONFIG_DIAG_APCNFCH_SIZE
#define CONFIG_DIAG_APCNFCH_SIZE 262144
#endif


#ifndef CONFIG_DIAG_APLOGCH_SIZE
#define CONFIG_DIAG_APLOGCH_SIZE 524288
#endif


#ifndef CONFIG_DIAG_CNFDSTCH_THRESHOLD
#define CONFIG_DIAG_CNFDSTCH_THRESHOLD 256
#endif


#ifndef CONFIG_DIAG_CNFDSTCH_SIZE
#define CONFIG_DIAG_CNFDSTCH_SIZE 524288
#endif


#ifndef CONFIG_DIAG_LOGDSTCH_THRESHOLD
#define CONFIG_DIAG_LOGDSTCH_THRESHOLD 65536
#endif


#ifndef CONFIG_DIAG_LOGDSTCH_SIZE
#define CONFIG_DIAG_LOGDSTCH_SIZE 1048576
#endif


#ifndef CONFIG_DIAG_DSTCH_FREEBUF_THRD
#define CONFIG_DIAG_DSTCH_FREEBUF_THRD 4232
#endif


#ifndef CONFIG_CHR_FS
#define CONFIG_CHR_FS
#endif


#ifndef CONFIG_MODEM_CORESIGHT
#endif


#ifndef ENABLE_BUILD_OM
#define ENABLE_BUILD_OM
#endif


#ifndef CONFIG_MODEM_MINI_DUMP
#define CONFIG_MODEM_MINI_DUMP
#endif


#ifndef CONFIG_MODEM_FULL_DUMP
#endif


#ifndef ENABLE_BUILD_DUMP_MDM_LPM3
#define ENABLE_BUILD_DUMP_MDM_LPM3
#endif


#ifndef CONFIG_DUMP_PLATFORM_V200
#define CONFIG_DUMP_PLATFORM_V200
#endif


#ifndef CONFIG_TRNG_SEED
#define CONFIG_TRNG_SEED
#endif


#ifndef CONFIG_VRL_SIZE
#define CONFIG_VRL_SIZE 8192
#endif


#ifndef CONFIG_MLOADER
#define CONFIG_MLOADER
#endif


#ifndef CONFIG_LOAD_SEC_IMAGE
#define CONFIG_LOAD_SEC_IMAGE
#endif


#ifndef CONFIG_MODEM_SECBOOT_CLEAN_DDR_USER
#define CONFIG_MODEM_SECBOOT_CLEAN_DDR_USER
#endif


#ifndef CONFIG_DFXCERT_VERIFY
#define CONFIG_DFXCERT_VERIFY
#endif


#ifndef CONFIG_MLOADER_COLD_PATCH
#define CONFIG_MLOADER_COLD_PATCH
#endif


#ifndef CONFIG_COMPRESS_CCORE_IMAGE
#define CONFIG_COMPRESS_CCORE_IMAGE
#endif


#ifndef CONFIG_KERNEL_DTS_DECOUPLED
#define CONFIG_KERNEL_DTS_DECOUPLED
#endif


#ifndef CONFIG_MODEM_DTB_LOAD_IN_KERNEL
#define CONFIG_MODEM_DTB_LOAD_IN_KERNEL
#endif


#ifndef CONFIG_PHONE_VERSION_DIFF_SOC_LEVEL
#define CONFIG_PHONE_VERSION_DIFF_SOC_LEVEL
#endif


#ifndef CONFIG_ENABLE_DTO
#define CONFIG_ENABLE_DTO
#endif


#ifndef CONFIG_MODEM_PINCTRL
#define CONFIG_MODEM_PINCTRL
#endif


#ifndef CONFIG_EFUSE
#define CONFIG_EFUSE
#endif


#ifndef CONFIG_RFILE_ON
#define CONFIG_RFILE_ON
#endif


#ifndef MODEM_FW_PARTITION_SIZE
#define MODEM_FW_PARTITION_SIZE 123731968
#endif


#ifndef MODEM_NV_UPDATE_PARTITION_SIZE
#define MODEM_NV_UPDATE_PARTITION_SIZE 16777216
#endif


#ifndef MODEM_NV_CUST_PARTITION_SIZE
#define MODEM_NV_CUST_PARTITION_SIZE 16777216
#endif


#ifndef CONFIG_NV_FUSION
#define CONFIG_NV_FUSION
#endif


#ifndef CONFIG_MODULE_TIMER
#define CONFIG_MODULE_TIMER
#endif


#ifndef CONFIG_MODEM_TIMER
#define CONFIG_MODEM_TIMER
#endif


#ifndef CONFIG_NMI
#define CONFIG_NMI
#endif


#ifndef CONFIG_MALLOC_UNIFIED
#define CONFIG_MALLOC_UNIFIED
#endif


#ifndef CONFIG_SHARED_MEMORY
#define CONFIG_SHARED_MEMORY
#endif


#ifndef CONFIG_SYSBOOT_PARA
#define CONFIG_SYSBOOT_PARA
#endif


#ifndef CONFIG_MEMORY_LAYOUT
#define CONFIG_MEMORY_LAYOUT
#endif


#ifndef CONFIG_SYSBUS
#endif


#ifndef CONFIG_WAN
#define CONFIG_WAN
#endif


#ifndef CONFIG_PFA
#define CONFIG_PFA
#endif


#ifndef CONFIG_PFA_PHONE_SOC
#define CONFIG_PFA_PHONE_SOC
#endif


#ifndef CONFIG_DRV_LTEV
#define CONFIG_DRV_LTEV
#endif


#ifndef CONFIG_DRA
#define CONFIG_DRA
#endif


#ifndef CONFIG_DRA_V3
#define CONFIG_DRA_V3
#endif


#ifndef FEATURE_HISOCKET
#define FEATURE_HISOCKET FEATURE_OFF
#endif


#ifndef CONFIG_NET_OM
#endif


#ifndef CONFIG_BTEST_PHONE
#define CONFIG_BTEST_PHONE
#endif


#ifndef CONFIG_VDEV
#define CONFIG_VDEV
#endif


#ifndef CONFIG_VDEV_PHONE
#define CONFIG_VDEV_PHONE
#endif


#ifndef CONFIG_VDEV_BRIDGE
#endif


#ifndef CONFIG_VDEV_DEBUG
#endif


#ifndef CONFIG_MCI_CUST_CHN
#endif


#ifndef CONFIG_MCI_ATP
#endif


#ifndef CONFIG_MCI_SOCKET
#endif


#ifndef CONFIG_USB_RELAY
#define CONFIG_USB_RELAY
#endif


#ifndef CONFIG_RELAY_USB_BUFF_NUM
#define CONFIG_RELAY_USB_BUFF_NUM 128
#endif


#ifndef CONFIG_UART_RELAY
#endif


#ifndef ENABLE_TEST_CODE
#endif


#ifndef CONFIG_FIPC_V200
#define CONFIG_FIPC_V200
#endif


#ifndef CONFIG_FIPC_SUPPORT_HIFI_RESET
#define CONFIG_FIPC_SUPPORT_HIFI_RESET
#endif


#ifndef CONFIG_MODEM_MSG
#define CONFIG_MODEM_MSG
#endif


#ifndef CONFIG_MODULE_IPC_FUSION
#endif


#ifndef CONFIG_MODULE_IPCMSG
#define CONFIG_MODULE_IPCMSG
#endif


#ifndef MDRV_TEST
#endif


#ifndef CONFIG_SYSCTRL
#define CONFIG_SYSCTRL
#endif


#ifndef CONFIG_LLT_MDRV
#endif


#ifndef CONFIG_ECDC
#endif


#ifndef CONFIG_MODEM_RESET
#define CONFIG_MODEM_RESET
#endif


#ifndef CONFIG_MPROBE
#endif


#ifndef CONFIG_AP_QIC
#define CONFIG_AP_QIC
#endif


#ifndef CONFIG_CSHELL
#endif


#ifndef MEM_LAYOUT_TYPE
#define MEM_LAYOUT_TYPE 0
#endif

#endif /*__PRODUCT_CONFIG_H__*/ 
