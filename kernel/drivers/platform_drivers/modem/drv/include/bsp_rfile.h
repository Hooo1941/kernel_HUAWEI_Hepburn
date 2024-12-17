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

#ifndef __BSP_RFILE_H__
#define __BSP_RFILE_H__

#include <product_config.h>

#ifdef __cplusplus /* __cplusplus */
extern "C" {
#endif /* __cplusplus */

/* ****************************************************************************
 * Attention                           *
 * *****************************************************************************
 * Description : Driver for rfile
 * Core        : Acore/Ccore
 * Header File : the following head files need to be modified at the same time
 * : /acore/kernel/platform_drivers/include/bsp_rfile.h
 * : /ccore/include/ccpu/bsp_rfile.h
 * : /ccore/include/fusion/bsp_rfile.h
 * **************************************************************************** */
#define RFILE_RDONLY (0x00000000)
#define RFILE_WRONLY (0x00000001)
#define RFILE_RDWR (0x00000002)
#define RFILE_CREAT (0x00000040)
#define RFILE_EXCL (0x00000080)
#define RFILE_TRUNC (0x00000200)
#define RFILE_APPEND (0x00000400)
#define RFILE_DIRECTORY (0x00004000)

#define RFILE_SEEK_SET 0 /* absolute offset, was L_SET */
#define RFILE_SEEK_CUR 1 /* relative to current offset, was L_INCR */
#define RFILE_SEEK_END 2 /* relative to end of file, was L_XTND */

#define RFILE_NAME_MAX 255 /* max length of the file name */

typedef enum {
    BSP_FS_OK = 0,
    BSP_FS_NOT_OK = 1,
    BSP_FS_BUTT
} bsp_fs_status_e;

#ifndef CONFIG_RFILE_ON
static inline bsp_fs_status_e bsp_fs_ok(void)
{
    return BSP_FS_NOT_OK;
}
static inline bsp_fs_status_e bsp_fs_tell_load_mode(void)
{
    return BSP_FS_NOT_OK;
}
static inline void bsp_rfile_get_load_mode(long type)
{
    return;
}
#else
/*
 * 功能描述: 判断a核文件系统是否可用
 * 返 回 值: 返回枚举bsp_fs_status_e中的值
 */
bsp_fs_status_e bsp_fs_ok(void);
/*
 * 功能描述: 获取加载模式，是否支持flash文件系统
 * 返 回 值: 返回枚举bsp_fs_status_e中的值
 */
bsp_fs_status_e bsp_fs_tell_load_mode(void);
/*
 * 功能描述: 告知rfile启动方式
 * 返 回 值: 无
 */
void bsp_rfile_get_load_mode(long type);
#endif
void bsp_fs_ok_notify(void);
#ifdef __cplusplus /* __cplusplus */
}
#endif /* __cplusplus */

#endif /*  __BSP_RFILE_H__ */
