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
#include <bsp_dump.h>
#include <bsp_dump_mem.h>
#include <bsp_print.h>
#include "syslog_dmesg.h"

#define THIS_MODU mod_print

#define SYSLOG_MAX_DMESG_STORE_LEN 0x10000
/*
 * 功能描述: 保存调用bsp_print勾取的临终打印
 */
void dump_save_last_kmsg(void)
{
#ifdef BSP_CONFIG_PHONE_TYPE
    static u32 size = 0;
    u8 *addr = NULL;
    addr = bsp_dump_get_field_addr(DUMP_MODEMAP_LAST_KMSG);
    if (addr == NULL) {
        bsp_dump_get_avaiable_size(&size);
        size = size & 0xfffff000;
        if (size != 0) {
            size = (size > SYSLOG_MAX_DMESG_STORE_LEN) ? SYSLOG_MAX_DMESG_STORE_LEN : size;
            addr = bsp_dump_register_field(DUMP_MODEMAP_LAST_KMSG, "lastkmsg", size, 0);
        }
    }
    if (addr != NULL) {
        bsp_store_log_buf(addr, size);
    }
#endif
}

void print_register_last_kmsg(void)
{
#ifdef BSP_CONFIG_PHONE_TYPE
    if (bsp_dump_register_hook("last_kmsg", dump_save_last_kmsg) == BSP_ERROR) {
        bsp_err("register last_kmsg fail!\n");
    }
#endif
}

EXPORT_SYMBOL(dump_save_last_kmsg);
