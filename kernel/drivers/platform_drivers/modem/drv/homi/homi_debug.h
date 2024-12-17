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
#ifndef __HOMI_DEBUG_H__
#define __HOMI_DEBUG_H__

#include <bsp_print.h>

#define THIS_MODU mod_homi

enum {
    BSP_HOMI_INIT_ERR = 1,
    BSP_HOMI_MALLOC_ERR,
    BSP_HOMI_PORT_MALLOC_ERR,
    BSP_HOMI_REGISTER_PORT_ERR,
    BSP_HOMI_FIND_PORT_ERR,
    BSP_HOMI_GET_CHAN_ERR,
    BSP_HOMI_CHAN_NOT_CONNECT_ERR,
    BSP_HOMI_MEMCPY_ERR,

    BSP_HOMI_USB_UDI_INIT_ERR,
    BSP_HOMI_USB_UDI_ENABLE_ERR,

    BSP_HOMI_SK_CFG_MALLOC_ERR,
    BSP_HOMI_SK_TYPE_ERR,
    BSP_HOMI_SK_BUF_MALLOC_ERR,
    BSP_HOMI_SK_CREATE_ERR,
    BSP_HOMI_SK_ACCEPT_ERR,
    BSP_HOMI_SK_SEND_TYPE_ERR,
    BSP_HOMI_SK_INVALID_ACPFD_ERR,

    BSP_HOMI_NL_DTS_PORT_CUST_ERR,
    BSP_HOMI_NL_CFG_MALLOC_ERR,
    BSP_HOMI_NL_TYPE_ERR,
    BSP_HOMI_NL_CREATE_ERR,
    BSP_HOMI_NL_NEW_SKB_ERR,
    BSP_HOMI_NL_PUT_NLH_ERR,

    BSP_HOMI_API_PARAMETER_ERR,

    BSP_HOMI_MAX_ERR = -1
};

#define homi_err(fmt, ...) bsp_err(fmt, ##__VA_ARGS__)

#define homi_printf(fmt, ...) bsp_err(fmt, ##__VA_ARGS__)

/* diag debug level */
enum {
    HLEVCRIT,
    HLEVERR,
    HLEVWARN,
    HLEVINFO,
    HLEVDBG,
};

extern int g_homi_dbglevel;

#define homi_debug(level, fmt, ...) do { \
    if (level < g_homi_dbglevel) {     \
        bsp_err(fmt, ##__VA_ARGS__); \
    }                                \
} while (0)

#endif
