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
#include "product_config.h"

#include <bsp_dump.h>
#include <bsp_print.h>
#include <bsp_mloader.h>
#include <bsp_rfile.h>
#include <bsp_version.h>
#include <linux/module.h>

#define THIS_MODU mod_rfile

bsp_fs_status_e g_rfile_fs_is_support = BSP_FS_NOT_OK;

bsp_fs_status_e bsp_fs_tell_load_mode(void)
{
    const bsp_version_info_s *version_info = bsp_get_version_info();
    if (version_info != NULL && version_info->plat_type != PLAT_EMU) {
        return BSP_FS_OK;
    }

    return g_rfile_fs_is_support;
}

void bsp_rfile_get_load_mode(long type)
{
    if (type == MLOADER_FS_LOAD) {
        g_rfile_fs_is_support = BSP_FS_OK;
    }
    return;
}

bsp_fs_status_e rfile_get_fs_is_support(void)
{
    return g_rfile_fs_is_support;
}

__init int rfile_get_load_mode_mbb(char *cmdline)
{
    if (cmdline == NULL) {
        bsp_err("cmdline is null.\n");
        return -EINVAL;
    }
    bsp_err("bsp_blk_load_mode :%s\n", cmdline);

    if (strcmp(cmdline, "flash") == 0) {
        g_rfile_fs_is_support = BSP_FS_OK;
    }

    return 0;
}

#ifndef BSP_CONFIG_PHONE_TYPE
early_param("bsp_blk_load_mode", rfile_get_load_mode_mbb);
#endif