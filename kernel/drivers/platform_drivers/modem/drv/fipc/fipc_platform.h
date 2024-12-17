/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2020. All rights reserved.
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

#ifndef _FIPC_PLATFORM_H
#define _FIPC_PLATFORM_H
#include <product_config.h>

#include <osl_types.h>
#include <osl_malloc.h>
#include <osl_spinlock.h>

#include <bsp_print.h>
#include <bsp_slice.h>
#include <bsp_version.h>
#include <bsp_dump.h>
#include <bsp_fipc.h>
#include "securec.h"
#include "fipc_diff.h"

#include <linux/version.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/err.h>
#include <linux/delay.h>
#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <linux/uaccess.h>

#ifdef __cplusplus
extern "C" {
#endif
#define FIPC_FEATURE_DUMP_SUPPORT 1

typedef u64 fipcsoc_ptr_t;

static inline u32 FIPC_ADDR_LO(fipcsoc_ptr_t soc_addr)
{
    return (u32)soc_addr;
}
static inline u32 FIPC_ADDR_HI(fipcsoc_ptr_t soc_addr)
{
    return (u32)0;
}
static inline fipcsoc_ptr_t FIPC_ADDR_MK(u32 lo, u32 hi)
{
    return (u32)lo;
}

static inline u32 fipc_get_slice_value(void)
{
    return bsp_get_slice_value();
}

#define THIS_MODU mod_fipc
/* emerg,alert,crit,err,warning,notice,info,debug ,bsp has 5 level  fatal,err,wrn,info,debug */
#define fipc_print_crit(fmt, ...) bsp_fatal(fmt, ##__VA_ARGS__)
#define fipc_print_error(fmt, ...) bsp_err(fmt, ##__VA_ARGS__)
#define fipc_print_warn(fmt, ...) bsp_wrn(fmt, ##__VA_ARGS__)
#define fipc_print_notice(fmt, ...) bsp_info(fmt, ##__VA_ARGS__)
#define fipc_print_info(fmt, ...) bsp_info(fmt, ##__VA_ARGS__)
#define fipc_print_debug(fmt, ...) bsp_debug(fmt, ##__VA_ARGS__)

#define fipc_print_always(fmt, ...) bsp_err(fmt, ##__VA_ARGS__)
#define fipc_print_trace(fmt, ...)

#define fipc_roundup(x, n) (((x) + (n)-1) & (~((n)-1)))

/* clang-format off */
static inline void *fipc_malloc(u32 sz){return osl_malloc(sz);}
static inline void fipc_free(const void *ptr){osl_free(ptr);}
void *fipc_dma_alloc(u32 size, fipcsoc_ptr_t *pa, u32 align);
void *fipc_pushmem_alloc(u32 size, fipcsoc_ptr_t *pa, u32 align);
void *fipc_ioremap(u32 pa, u32 size);
void fipc_iounmap(void *addr);
int bsp_fipc_dump(void);
/* clang-format on */
#ifdef __cplusplus
}
#endif

#endif
