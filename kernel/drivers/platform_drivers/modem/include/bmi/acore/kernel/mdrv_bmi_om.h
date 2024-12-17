/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
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

/**
 * @brief   飞龙架构2.0 dump/rdr模块对外头文件
 * @file    mdrv_bmi_om.h
 * @version v1.0
 * @date    2023.07.22
 * @note    修改记录(版本号|修订日期|说明)
 * <ul><li>v1.0|xxx.xx.xx|创建文件</li></ul>
 * @since
 */
#ifndef __MDRV_BMI_OM_H__
#define __MDRV_BMI_OM_H__

#include <linux/types.h>
/*****************************************************************************
     *                           Attention                           *
* Description : Driver for dump/rdr
* Core        : Acore、Ccore、mcore
* Header File : the following head files need to be modified at the same time
*             : /acore(hiboot,ccore,mcore,teeos)/bsp_dump_mem.h
******************************************************************************/
#define RDR_AREA_MODULE_MAXLEN 16
#define RDR_AREA_DESC_MAXLEN 48
#define RDR_AREA_DATETIME_MAXLEN 24

struct rdr_base_info_s {
    u32 modid;
    u32 arg1;
    u32 arg2;
    u32 e_core;
    u32 e_type;
    u32 start_flag;
    u32 savefile_flag;
    u32 reboot_flag;
    u8 e_module[RDR_AREA_MODULE_MAXLEN];
    u8 e_desc[RDR_AREA_DESC_MAXLEN];
    u32 timestamp;
    u8 datetime[RDR_AREA_DATETIME_MAXLEN];
};

/*****************************************************************************
      *                        Attention   END                        *
 *****************************************************************************/

/**
 * @brief 获取上次复位相关信息
 * @retval NULL, 复位信息不存在，可能是冷启动等原因导致
 * @retval 非NULL, 复位信息存在
 */
const struct rdr_base_info_s *mdrv_om_get_rdr_base_info(void);

/**
 * @brief 根据异常类型获取异常原因
 * @param[in]  exc_type, 异常类型，一般从rdr_base_info_s获取
 * @retval "UNDEF", 未找到exc_type对应的类型描述
 * @retval 其他, exc_type描述信息
 */
const char *mdrv_om_rdr_get_exception_type(u32 exc_type);

/**
 * @brief dump路径所在文件系统状态回调；该接口必须实现为非阻塞接口
 * @retval 0, 文件系统不可用
 * @retval 1, 文件系统可用
 */
typedef unsigned int (*dump_fs_state_func)(void);

typedef struct {
    dump_fs_state_func fs_state_cb;
} dump_extra_log_cfg_s;

/**
 * @brief 设置定制fulldump日志的配置信息接口
 * @par 描述:
 * 配置log路径等信息
 * @param[in]  cfg, 配置信息,如果没有定制extra log的路径，或者log路径不是u盘等外部设备，cfg里的fs_state_cb可以为NULL
 * @retval 0, 执行成功
 * @retval 非0, 执行失败
 */
unsigned int mdrv_om_set_extra_log_cfg(dump_extra_log_cfg_s *cfg);

#endif
