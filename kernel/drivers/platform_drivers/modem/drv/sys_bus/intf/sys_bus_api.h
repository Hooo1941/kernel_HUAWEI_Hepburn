/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
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

#ifndef __SYS_BUS_API_H__
#define __SYS_BUS_API_H__

#include <bsp_print.h>

#undef THIS_MODU
#define THIS_MODU mod_sys_bus

#define SYS_BUS_NAME_LEN 32     // IP测试模块注册sys_bus_register_observer名字的长度上限，包括结束符"\0"。

#define SYS_BUS_DTS_STATUS_ENABLE "enable"

#define SYS_BUS_TASK_PRIORITY   62
#define SYS_BUS_TASK_STACK_SIZE 0x1000

#define SYS_BUS_CPU_MASK_0  ((u32)1 << (u32)0)
#define SYS_BUS_CPU_MASK_1  ((u32)1 << (u32)1)


// 接口返回值
typedef enum {
    E_SYS_BUS_RET_OK,               // 操作成功
    E_SYS_BUS_RET_ALREADY_EXIST,    // 已经存在
    E_SYS_BUS_RET_NULL_PTR,         // 入参为空
    E_SYS_BUS_RET_ERR,              // 操作异常
    
    E_SYS_BUS_RET_BUTT
}sys_bus_ret_e;

// 总线压测事件
typedef enum {
    E_SYS_BUS_EVT_START,    // 启动总线压测
    E_SYS_BUS_EVT_STOP,     // 停止总线压测
    E_SYS_BUS_EVT_REPORT,   // 打印信息，包括但不限于校验结果，调试信息等
    
    E_SYS_BUS_EVT_BUTT
}sys_bus_evt_e;

/*
 * 总线压测事件处理函数的类型定义。
 * 通过事件处理函数的类型定义各IP的事件处理函数函数，并注册在压测框架。
 * 压测框架在收到串口命令时调用注册的该处理函数。
 *
 * param1 [IN] 类型#sys_bus_evt_e，总线压测事件。
 * retval   E_SYS_BUS_RET_OK：成功；其他：失败
 */
typedef int (*sys_bus_observer_entry)(sys_bus_evt_e);

/*
 * 总线压测注册事件回调处理函数。
 * 压测框架在收到串口命令时调用注册的回调函数通知各IP压测模块。
 *
 * param1 [IN] 类型#const char *，压测的IP名字（ex. edma）。
 * param1 [IN] 类型#sys_bus_observer_entry，事件处理回调函数。
 * retval   E_SYS_BUS_RET_OK：成功；其他：失败
 */
extern int sys_bus_register_observer(const char *name, sys_bus_observer_entry entry);

/*
 * IP压测模块上报校验失败的处理函数。
 * 如果IP压测模块校验数据失败，调用该接口通知压测框架停止压测，以保护出错现场。
 *
 * param1 [IN] 类型#const char *，压测的IP名字（ex. edma）。
 */
extern void sys_bus_verify_failed(const char *name);


#define sys_bus_err(fmt, ...)       (bsp_err(fmt, ##__VA_ARGS__))
#define sys_bus_trace(fmt, ...)     (bsp_err("<%s> <%d> "fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__))

#endif /* __SYS_BUS_API_H__ */

