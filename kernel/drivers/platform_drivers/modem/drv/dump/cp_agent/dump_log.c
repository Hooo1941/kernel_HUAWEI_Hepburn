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


#include <linux/kernel.h>
#include <linux/vmalloc.h>
#include <mdrv_memory_layout.h>
#include <securec.h>
#include <osl_malloc.h>
#include <bsp_dump.h>
#include <bsp_slice.h>
#include <bsp_nvim.h>
#include <bsp_dt.h>
#include <dump_frame.h>
#include <dump_area.h>
#include <dump_log_strategy.h>
#include <dump_logs.h>
#include "dump_log.h"

#undef THIS_MODU
#define THIS_MODU mod_dump

void dump_save_share_unsec_log(const char *dir_name)
{
    s32 ret;
    ret = dump_save_ddr_file(dir_name, "share_unsec_ddr", "share_unsec.bin", 0);
    if (ret == BSP_OK) {
        dump_print("save share_unsec.bin finished\n");
    }
}

void dump_save_share_nsro_log(const char *dir_name)
{
    s32 ret;
    ret = dump_save_ddr_file(dir_name, "share_nsro_ddr", "share_nsro.bin", 0);
    if (ret == BSP_OK) {
        dump_print("save share_nsro.bin finished\n");
    }
}


void dump_cp_log_init(void)
{
    bsp_dump_register_log_notifier(DUMP_SAVE_SHARE_UNSEC, dump_save_share_unsec_log);
    bsp_dump_register_log_notifier(DUMP_SAVE_SHARE_NSRO, dump_save_share_nsro_log);
}
