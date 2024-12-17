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
#include <bsp_sysboot.h>
#include "oeminfo_core.h"

static int oeminfo_para_check(u32 idx, void *buf, u32 id_max)
{
    if (idx >= id_max) {
        oeminfo_log("private idx(%d) is err\n", idx);
        return OEMINFO_PARA_ERR;
    }

    if (buf == NULL) {
        oeminfo_log("data buf is NULL\n");
        return OEMINFO_PARA_ERR;
    }

    return OEMINFO_OK;
}

static int oeminfo_rgn_read(enum oeminfo_mgr mgr, u32 idx, void *buf, u32 buf_len)
{
    int unit_size = oeminfo_get_unit_size(OEMINFO_PRIVATE_MGR);
    if (unit_size < 0) {
        return unit_size;
    }

    return oeminfo_read(mgr, idx * unit_size, buf, buf_len);

}

static int oeminfo_rgn_write_sync(enum oeminfo_mgr mgr, u32 idx, void *buf, u32 buf_len,
            int (*write_handle)(enum oeminfo_mgr id, u32 offset, const void *data_buf, u32 data_len))
{
    int unit_size = oeminfo_get_unit_size(mgr);
    if (unit_size < 0) {
        return unit_size;
    }

    if (buf_len > (u32)unit_size - sizeof(struct oeminfo_data_head)) {
        oeminfo_log("buf len(%d) is larger than %ld\n", buf_len, unit_size - sizeof(struct oeminfo_data_head));
        return OEMINFO_PARA_ERR;
    }

    return write_handle(mgr, idx * unit_size, buf, buf_len);
}

int mdrv_oeminfo_comm_rgn_read(enum oeminfo_comm_rgn_id idx, void *buf, unsigned int buf_len)
{
    int ret = oeminfo_para_check(idx, buf, OEMINFO_COMM_RGN_MAX);
    if (ret) {
        return ret;
    }

    return oeminfo_rgn_read(OEMINFO_COMM_MGR, idx, buf, buf_len);
}
EXPORT_SYMBOL_GPL(mdrv_oeminfo_comm_rgn_read);

int mdrv_oeminfo_comm_rgn_write_async(enum oeminfo_comm_rgn_id idx, void *buf, unsigned int buf_len)
{
    int ret = oeminfo_para_check(idx, buf, OEMINFO_COMM_RGN_MAX);
    if (ret) {
        return ret;
    }

    return oeminfo_rgn_write_sync(OEMINFO_COMM_MGR, idx, buf, buf_len, oeminfo_write_async);
}
EXPORT_SYMBOL_GPL(mdrv_oeminfo_comm_rgn_write_async);

int mdrv_oeminfo_comm_rgn_write_sync(enum oeminfo_comm_rgn_id idx, void *buf, unsigned int buf_len)
{
    int ret = oeminfo_para_check(idx, buf, OEMINFO_COMM_RGN_MAX);
    if (ret) {
        return ret;
    }

    return oeminfo_rgn_write_sync(OEMINFO_COMM_MGR, idx, buf, buf_len, oeminfo_write_sync);
}
EXPORT_SYMBOL_GPL(mdrv_oeminfo_comm_rgn_write_sync);

int bsp_oeminfo_private_rgn_read(enum oeminfo_private_rgn_id idx, void *buf, u32 buf_len)
{
    int ret = oeminfo_para_check(idx, buf, OEMINFO_PRIVATE_RGN_MAX);
    if (ret) {
        return ret;
    }

    return oeminfo_rgn_read(OEMINFO_PRIVATE_MGR, idx, buf, buf_len);
}
EXPORT_SYMBOL_GPL(bsp_oeminfo_private_rgn_read);

int bsp_oeminfo_private_rgn_write_async(enum oeminfo_private_rgn_id idx, void *buf, u32 buf_len)
{
    int ret = oeminfo_para_check(idx, buf, OEMINFO_PRIVATE_RGN_MAX);
    if (ret) {
        return ret;
    }

    return oeminfo_rgn_write_sync(OEMINFO_PRIVATE_MGR, idx, buf, buf_len, oeminfo_write_async);
}
EXPORT_SYMBOL_GPL(bsp_oeminfo_private_rgn_write_async);

int bsp_oeminfo_private_rgn_write_sync(enum oeminfo_private_rgn_id idx, void *buf, u32 buf_len)
{
    int ret = oeminfo_para_check(idx, buf, OEMINFO_PRIVATE_RGN_MAX);
    if (ret) {
        return ret;
    }

    return oeminfo_rgn_write_sync(OEMINFO_PRIVATE_MGR, idx, buf, buf_len, oeminfo_write_sync);
}
EXPORT_SYMBOL_GPL(bsp_oeminfo_private_rgn_write_sync);

int mdrv_oeminfo_cust_rgn_read(unsigned int offset, void *buf, unsigned int buf_len)
{
    if (buf == NULL) {
        oeminfo_log("buf is NULL\n");
        return OEMINFO_PARA_ERR;
    }

    return oeminfo_read(OEMINFO_CUSTOM_MGR, offset, buf, buf_len);
}
EXPORT_SYMBOL_GPL(mdrv_oeminfo_cust_rgn_read);

int mdrv_oeminfo_cust_rgn_write_sync(unsigned int offset, const void *buf, unsigned int buf_len)
{
    if (buf == NULL) {
        oeminfo_log("buf is NULL\n");
        return OEMINFO_PARA_ERR;
    }

    return oeminfo_write_sync(OEMINFO_CUSTOM_MGR, offset, buf, buf_len);
}
EXPORT_SYMBOL_GPL(mdrv_oeminfo_cust_rgn_write_sync);

int mdrv_oeminfo_cust_rgn_write_async(unsigned int offset, const void *buf, unsigned int buf_len)
{
    if (buf == NULL) {
        oeminfo_log("data buf is NULL\n");
        return OEMINFO_PARA_ERR;
    }

    return oeminfo_write_async(OEMINFO_CUSTOM_MGR, offset, buf, buf_len);
}
EXPORT_SYMBOL_GPL(mdrv_oeminfo_cust_rgn_write_async);
