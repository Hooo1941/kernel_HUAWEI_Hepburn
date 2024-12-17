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

#include <linux/export.h>
#include <mdrv_bmi_oeminfo.h>
#include "oeminfo_cfg.h"

#define OEMINFO_COMM_ADDR 0x0
#define OEMINFO_COMM_SIZE 0x2000  // 8KB
#define OEMINFO_PRIVATE_ADDR OEMINFO_COMM_SIZE
#define OEMINFO_PRIVATE_SIZE 0x4000 // 16KB
#define OEMINFO_CUSTOM_ADDR (OEMINFO_PRIVATE_ADDR + OEMINFO_PRIVATE_SIZE)
#define OEMINFO_CUSTOM_SIZE 0x8000 // 32KB

#define OEMINFO_COMM_DATA_BLK_SIZE 256
#define OEMINFO_PRIVATE_DATA_BLK_SIZE 256


struct oeminfo_cfg_info g_oeminfo_cfg_info[OEMINFO_MGR_MAX] = {
    {OEMINFO_COMM_ADDR, OEMINFO_COMM_SIZE, OEMINFO_COMM_DATA_BLK_SIZE},
    {OEMINFO_PRIVATE_ADDR, OEMINFO_PRIVATE_SIZE, OEMINFO_PRIVATE_DATA_BLK_SIZE},
    {OEMINFO_CUSTOM_ADDR, OEMINFO_CUSTOM_SIZE, 0},
};

struct oeminfo_cfg_info *oeminfo_get_cfg_info(void)
{
    return g_oeminfo_cfg_info;
}

int mdrv_oeminfo_set_cust_rgn_size(unsigned int size)
{
    g_oeminfo_cfg_info[OEMINFO_CUSTOM_MGR].size = size;
    return OEMINFO_OK;
}
EXPORT_SYMBOL_GPL(mdrv_oeminfo_set_cust_rgn_size);
