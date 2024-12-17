/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2018-2020. All rights reserved.
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

#ifndef __PCIE_CDEV_DBG_H__
#define __PCIE_CDEV_DBG_H__

#include <linux/kernel.h>
#include <bsp_print.h>
#include <bsp_slice.h>
#include "pcie_cdev.h"
#include <osl_types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define THIS_MODU mod_pcdev
extern struct pcdev_ctx g_pcdev_ctx;

/* gcdev msg level */
#define PCDEV_LEVEL_ERR BIT(0)
#define PCDEV_LEVEL_WARNING BIT(1)
#define PCDEV_LEVEL_TRACE BIT(2)
#define PCDEV_LEVEL_INFO BIT(3)
#define PCDEV_LEVEL_PKT_DBG BIT(4)

#define PCDEV_MAX_PRINT_COUNT 64
#define PCDEV_PRINT_LEN 16

#define PCDEV_ERR(fmt, ...) do { \
    if (g_pcdev_ctx.msg_level & PCDEV_LEVEL_ERR)      \
        bsp_err("<%s>" fmt, __func__, ##__VA_ARGS__); \
} while (0)

#ifdef CONFIG_PCDEV_DEBUG
#define PCDEV_WARNING(fmt, ...) do { \
    if (g_pcdev_ctx.msg_level & PCDEV_LEVEL_WARNING)  \
        bsp_err("<%s>" fmt, __func__, ##__VA_ARGS__); \
} while (0)
#define PCDEV_TRACE(fmt, ...) do { \
    if (g_pcdev_ctx.msg_level & PCDEV_LEVEL_TRACE)    \
        bsp_err("<%s>" fmt, __func__, ##__VA_ARGS__); \
} while (0)

#define PCDEV_INFO(port_num, fmt, ...)                                          \
    if ((g_pcdev_ctx.print_port & (1 << port_num))) {                            \
        do {                                                                    \
            if (g_pcdev_ctx.msg_level & PCDEV_LEVEL_INFO)                        \
                bsp_info("<%s>" fmt, __func__, ##__VA_ARGS__);\
        } while (0);                                                            \
    }


void print_pkt(unsigned int port_num, unsigned char *buf, int len);
#else /* CONFIG_PCDEV_DEBUG */
#define PCDEV_WARNING(fmt, ...)
#define PCDEV_TRACE(fmt, ...)
#define PCDEV_INFO(fmt, ...)

static inline void print_pkt(unsigned int port_num, unsigned char *buf, int len)
{
    return;
}
#endif /* CONFIG_PCDEV_DEBUG */

void pcdev_dbg_timer_start(void);

#ifdef __cplusplus
}
#endif
#endif
