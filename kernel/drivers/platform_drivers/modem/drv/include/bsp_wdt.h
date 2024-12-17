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
#ifndef __BSP_WDT_H__
#define __BSP_WDT_H__

#include <linux/io.h>
#include <bsp_print.h>

#define  wdt_err(fmt, ...)         (bsp_err(fmt, ##__VA_ARGS__))
#define  wdt_info(fmt, ...)        (bsp_info(fmt, ##__VA_ARGS__))

/* 整理后 */
#define WDT_OK                     (0)
#define WDT_ERROR                  (-1)
#define WDT_DEF_CLK_FREQ           (32768)  /* 32khz */
#define WDT_KEEPALIVE_TIME         (15)  /* sec */
#define WDT_TIMEOUT_SEC            (30)  /* sec */

typedef enum _WDT_CORE_ID {
    WDT_CCORE_ID,
    WDT_ACORE_ID,
    WDT_MCORE_ID,
    MAX_WDT_CORE_ID,
} WDT_CORE_ID;

typedef void(*wdt_timeout_cb)(unsigned int);  /* A、C核共用 */

int  bsp_wdt_register_hook(WDT_CORE_ID core_id, wdt_timeout_cb func);
void bsp_wdt_irq_disable(WDT_CORE_ID core_id);
void bsp_wdt_irq_enable(WDT_CORE_ID core_id);
void bsp_wdt_reinit(void);

#endif /*__BSP_WDT_H__*/
