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
#include <securec.h>
#include <product_config.h>
#ifdef BSP_CONFIG_PHONE_TYPE
#include <linux/mtd/nve_ap_kernel_interface.h>
#endif
#include <osl_types.h>
#include <bsp_version.h>
#include "diagnve.h"
#include "diag_debug.h"


#ifdef BSP_CONFIG_PHONE_TYPE
static int nvme_mdmlog_write(u32 index, const u8 *data, u32 len)
{
    int ret;
    struct opt_nve_info_user pinfo = {0};

    pinfo.nv_operation = NVME_WRITE;
    pinfo.nv_number = NVME_MDMLOG_NUM;
    pinfo.valid_size = NVME_MDMLOG_SIZE;
    ret = memcpy_s(pinfo.nv_data + index, NVME_MDMLOG_SIZE - index, data, len);
    if (ret) {
        diag_err("memcpy fail:%d\n", ret);
        return ret;
    }

    ret = nve_direct_access_interface(&pinfo);
    if (ret) {
        diag_err("write mdmlog nve failed, ret=%d\n", ret);
        return ret;
    }

    diag_err("write nve success!\n");
    return 0;
}

static int nvme_mdmlog_read(u32 index, u8 *buf, u32 len)
{
    int ret;
    struct opt_nve_info_user pinfo = {0};

    pinfo.nv_operation = NVME_READ;
    pinfo.nv_number = NVME_MDMLOG_NUM;
    pinfo.valid_size = NVME_MDMLOG_SIZE;
    ret = nve_direct_access_interface(&pinfo);
    if (ret) {
        diag_err("read mdmlog nve failed:%d\n", ret);
        return ret;
    }

    ret = memcpy_s(buf, NVME_MDMLOG_SIZE, pinfo.nv_data + index, len);
    if (ret) {
        diag_err("memcpy fail,ret=0x%x\n", ret);
        return ret;
    }

    diag_err("read mdmlog nve success !\n");
    return 0;
}

static int set_mdmlog_nve(void)
{
    int ret;
    char mdmlog_buf[NVME_MDMLOG_SIZE] = {0};

    ret = nvme_mdmlog_read(0, mdmlog_buf, NVME_MDMLOG_SIZE);
    if (ret) {
        return ret;
    }

    if (mdmlog_buf[0] != '1') {
        mdmlog_buf[0] = '1';
        ret = nvme_mdmlog_write(0, mdmlog_buf, NVME_MDMLOG_SIZE);
        if (ret) {
            return ret;
        }
    }

    if (mdmlog_buf[0x2] != '1') {
        mdmlog_buf[0x2] = '1';
        ret = nvme_mdmlog_write(0, mdmlog_buf, NVME_MDMLOG_SIZE);
        if (ret) {
            return ret;
        }
    }

    return 0;
}

void mdmlog_nve_init(void)
{
    u32 ret;
    const bsp_version_info_s *ver_info = bsp_get_version_info();

    if ((ver_info == NULL) || (ver_info->udp_flag != HW_VER_HIONE_UDP_MAGIC)) {
        diag_err("get_version_info null or not udp\n");
        return;
    }

    ret = set_mdmlog_nve();
    if (ret) {
        diag_err("diag mdmlog nve set err:%d\n", ret);
    }
}
#else
void mdmlog_nve_init(void)
{
    return;
}
#endif

