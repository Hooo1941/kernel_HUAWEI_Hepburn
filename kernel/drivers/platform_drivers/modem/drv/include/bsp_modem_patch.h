/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
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

#ifndef __BSP_MODEM_PATCH_H__
#define __BSP_MODEM_PATCH_H__

/* modem_patch_type与modem_patch_status的枚举定义
   修改上述枚举定义，需要同步修改以下文件：
   ccore\mree\plat\include\fusion\bsp_modem_patch.h
*/

#define MODEM_PATCH_LOAD_FAIL 1    /* *<patch加载失败次数超过3次,patch异常 */
#define MODEM_PATCH_LOAD_SUCCESS 0 /* *<patch状态正常 */

enum modem_patch_exist {
    MODEM_PATCH_NOT_EXIST = 0, /* *<patch不存在 */
    MODEM_PATCH_EXIST = 1,     /* *<patch存在 */
};

enum modem_patch_status {
    NOT_LOAD_PATCH = 0,   /* *<不需要加载patch,patch文件不存在 */
    LOAD_PATCH_FAIL,      /* *<加载patch文件异常 */
    LOAD_PATCH_SUCCESS,   /* *<加载和校验patch文件成功 */
    SPLICE_PATCH_FAIL,    /* *<拼接patch文件失败 */
    SPLICE_PATCH_SUCCESS, /* *<拼接patch文件成功 */
    PATCH_STATE_MAX,
};

enum modem_patch_type {
    PATCH_SPLICE = 0,           /* *<patch需要拼接 */
    PATCH_REPLACE,             /* *<使用patch替换old版本 */
    PATCH_TYPE_MAX,
    PATCH_NONEED_LOAD = 0xff, // 不需要加载patch
};

struct modem_patch_info {
    u8 patch_exist;    /* *<补丁是否存在，参考modem_patch_exist结构体 */
    u8 patch_status;   /* *<补丁的状态，参考modem_patch_status结构体 */
    u8 patch_type;     /* *<补丁的类型，参考modem_patch_type结构体 */
    u8 reserved1;
};

/* *
 * @brief 获取patch是否存在。
 *
 * @par 描述:
 * 打开文件。
 *
 * @attention
 * <ul><li>1. 可能引起任务切换/挂起；</li></ul>
 * <ul><li>2. 该接口内含有互斥锁；</li></ul>
 *
 * @retval = MODEM_PATCH_NOT_EXIST，表示不存在patch
 * @retval = MODEM_PATCH_EXIST，表示patch存在
 */
int bsp_modem_cold_patch_is_exist(void);

/* *
 * @brief 获取patch状态是否正常
 *
 * @par 描述:
 * 打开文件。
 *
 * @attention
 * <ul><li>1. 可能引起任务切换/挂起；</li></ul>
 * <ul><li>2. 该接口内含有互斥锁；</li></ul>
 *
 * @param[out]  state：MODEM_PATCH_LOAD_SUCCESS表示patch正常；
 *                     MODEM_PATCH_LOAD_FAIL表示patch异常(加载失败次数超过3次)
 *
 * @retval = 0，表示获取状态成功
 * @retval != 0，表示获取patch状态失败
 */
int bsp_modem_patch_get_state(int *state);

/* *
 * @brief 设置patch的启动状态
 *
 * @par 描述:
 * 更新file_name对应patch的启动状态，补丁推送成功后，手机重启开机完成后，OTA通过设备节点获取各个镜像的补丁拼接状态。
 *
 * @attention
 * <ul><li>1. 可能引起任务切换/挂起；</li></ul>
 * <ul><li>2. 该接口内含有互斥锁；</li></ul>
 * <ul><li>2. 该接口内可能含有内存申请；</li></ul>
 *
 * @param[in]  file_name patch对应镜像的镜像名，镜像名(包括结束符)不能超过32个字符
 * @param[in]  info: patch的状态信息，见modem_patch_info结构体的定义
 *
 * @retval = 0，表示设置patch启动状态成功
 * @retval != 0，表示设置patch启动状态失败
 */
int bsp_modem_patch_set_boot_info(const char *file_name, struct modem_patch_info *info);

#endif
