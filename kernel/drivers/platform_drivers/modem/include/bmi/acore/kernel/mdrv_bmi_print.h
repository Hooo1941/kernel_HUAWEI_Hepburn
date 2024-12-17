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

/**
 * @brief   飞龙架构2.0 print模块对外头文件
 * @file    mdrv_bmi_print.h
 * @version v1.0
 * @date    2023.07.10
 * @note    修改记录(版本号|修订日期|说明)
 * <ul><li>v1.0|xxx.xx.xx|创建文件</li></ul>
 * @since
 */
#ifndef __MDRV_BMI_PRINT_H__
#define __MDRV_BMI_PRINT_H__

/**
* @brief 打印级别
*/
typedef enum {
    MDRV_P_CLOSE = 0,   /**< 0 打印关闭 */
    MDRV_P_CRIT = 1,    /**< 1 关键流程打印，默认打印到串口 */
    MDRV_P_ERR,         /**< 2 ERROR级别打印，默认打印到串口 */
    MDRV_P_WRN,         /**< 3 WARNING级别打印，默认打印到 kernel log_buf */
    MDRV_P_INFO,        /**< 4 INFO级别打印，默认打印到 kernel log_buf */
    MDRV_P_DEBUG,       /**< 5 DEBUG级别打印，默认不打开 */
    MDRV_LEVEL_BUTT
} mdrv_log_level_e;

/**
 * @brief 打印模块枚举值范围
 * @attention
 * <ul><li>枚举值 MDRV_BMI_MODID_END 仅作边界值，为非法枚举值</li></ul>
*/
typedef enum {
    MDRV_BMI_MODID_START = 512,
    MDRV_BMI_MODID_END = 1024,
} mdrv_bmi_modid_e;

/**
 * @brief 子系统枚举值
 * @attention
 * <ul><li>当前支持 xloader、hiboot子系统</li></ul>
*/
typedef enum {
    MDRV_LOGEMEM_XLOADER,
    MDRV_LOGEMEM_HIBOOT,
    MDRV_LOGEMEM_BUTT
} mdrv_logmem_module_e;

/**
 * @brief 存放子系统日志的内存信息
 * @attention
 * <ul><li>该内存数据只允许读取，不允许修改</li></ul>
*/
typedef struct {
    char *virt_addr;         /* 日志内存虚拟地址 */
    unsigned long phy_addr;  /* 日志内存物理地址 */
    unsigned int length;     /* 日志内存长度 */
} mdrv_logmem_info_s;

#define MDRV_PRINT_BUF_LEN  128
#define MDRV_PRINT_HOOK_MAX 4

/**
 * @brief 打印接口
 * @par 描述:
 * 该接口提供打印功能，格式化字符串后打印到串口及hook
 * @attention
 * <ul><li>打印函数</li></ul>
 * <ul><li>模块枚举根据各个组件划分的范围填写，不可随意传入</li></ul>
 * <ul><li>打印字符串长度不可过长，最大长度限制见 MDRV_PRINT_BUF_LEN</li></ul>
 * @param[in]  modid, 模块枚举值。
 * @param[in]  level, 打印级别。
 * @param[in]  fmt,   格式化字符串。
 * @param[out] 无
 * @par 依赖:
 * <ul><li>mdrv_bmi_print.h：该接口声明所在的头文件。</li></ul>
 * @see mdrv_bmi_modid_e 、mdrv_log_level_e
 * @retval void。
 */
void mdrv_print(unsigned int modid, mdrv_log_level_e level, const char *fmt, ...);

/**
 * @brief 打印hook函数类型
 * @par 描述:
 * 打印hook函数类型
 * @attention
 * <ul><li>打印hook函数类型</li></ul>
 * <ul><li>在该hook实现中不允许有任何阻塞性操作</li></ul>
 * <ul><li>在该hook实现中不允许引起任务切换/挂起</li></ul>
 * @param[in]  modid,  模块枚举。
 * @param[in]  level,  打印级别。
 * @param[in]  string, 格式化后字符串。
 * @param[out] 无
 * @par 依赖:
 * <ul><li>mdrv_bmi_print.h：该接口声明所在的头文件。</li></ul>
 *  @retval void。
 */
typedef void (*mdrv_print_hook)(unsigned int modid, unsigned int level, const char *string);

/**
 * @brief 注册打印hook
 * @par 描述:
 * 注册打印hook
 * @attention
 * <ul><li>注册打印hook的函数</li></ul>
 * <ul><li>注册资源最大为 MDRV_PRINT_HOOK_MAX</li></ul>
 * @param[in]  hook, hook函数。
 * @param[out] 无
 * @par 依赖:
 * <ul><li>mdrv_bmi_print.h：该接口声明所在的头文件。</li></ul>
 * @retval 0-3 hook id。
 * @retval -1  注册资源不足。
 */
int mdrv_print_hook_register(mdrv_print_hook hook);

/**
 * @brief 去注册打印hook
 * @par 描述:
 * 去注册打印hook
 * @attention
 * <ul><li>去注册打印hook的函数</li></ul>
 * @param[in]  hook_id, 注册时返回的资源id。
 * @param[out] 无
 * @par 依赖:
 * <ul><li>mdrv_bmi_print.h：该接口声明所在的头文件。</li></ul>
 * @retval 0  去注册成功。
 * @retval -1 去注册失败。
 */
int mdrv_print_hook_unregister(int hook_id);

/**
 * @brief 获取子系统打印日志信息
 * @par 描述:
 * 获取获取子系统打印日志信息
 * @attention
 * <ul><li>获取子系统打印日志信息</li></ul>
 * <ul><li>当获取失败时获取的信息无效，请勿使用</li></ul>
 * @param[in]  type, 子系统枚举值
 * @param[out] info, 子系统日志存放信息
 * @par 依赖:
 * <ul><li>mdrv_bmi_print.h：该接口声明所在的头文件。</li></ul>
 * @see mdrv_logmem_module_e
 * @retval 0  获取成功。
 * @retval -1 获取失败。
 */
int mdrv_get_logmem_info(mdrv_logmem_module_e type, mdrv_logmem_info_s *info);

#endif