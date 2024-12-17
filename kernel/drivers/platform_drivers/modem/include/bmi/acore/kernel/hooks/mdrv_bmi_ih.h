/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2025. All rights reserved.
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

#ifndef __MDRV_BMI_IH_H__
#define __MDRV_BMI_IH_H__
#include <asm/bug.h>
#include <linux/mutex.h>

#define PARAMS(args...) args
#define IH_PROTO(args...) args
#define IH_ARGS(args...) args

/**
 * @brief hook func, registered by modules
 *
 */
struct __iot_hook_func {
    void *func;
    void *data;
    bool has_data;
};

/**
 * @brief hook struct, need export for modules
 *
 */
struct iot_hook {
    struct mutex mutex;
    struct __iot_hook_func *funcs;
};

/**
 * @brief 注册插桩实现
 * @par 描述: 把具体实现函数注册到__ih_##name结构体中，如果实现函数在KO中，增加对模块引用计数
 * @param[out] data 插桩函数影响的数据结构
 * @param[in] ih 插桩核心数据结构
 * @param[in] handler 插桩函数指针
 * @param[in] has_data 插桩函数是否有需要修改的数据结构
 * @retval 0, 执行成功
 * @retval 非0, 执行失败
 */
int mdrv_bmi_iot_hook_register(struct iot_hook *ih, void *handler, void *data, bool has_data);

/**
 * @brief 注册插桩函数入口
 * @param[in] name 插桩函数名
 */
#define REGISTER_IOT_HOOK(name)    \
    do {    \
        if (register_ih_##name(name))    \
            WARN_ONCE(1, "iot hook register failed!\n");    \
    } while (0)

/**
 * @brief 检查注册函数的入参类型，跳转到实际完成注册的注册函数
 * @param[in] name 插桩函数名
 * @param[in] proto 插桩函数入参类型
 * @param[in] args 插桩函数入参名称
 */
#define __DECLARE_IOT_HOOK(name, proto, args)    \
    extern struct iot_hook __ih_##name;    \
    static inline int    \
    register_ih_##name(void (*handler)(proto))    \
    {    \
        return mdrv_bmi_iot_hook_register(&__ih_##name, (void *)handler, NULL, false);    \
    }

#undef DECLARE_IOT_HOOK
#define DECLARE_IOT_HOOK(name, proto, args)    \
    __DECLARE_IOT_HOOK(name, PARAMS(proto), PARAMS(args))

#endif // __MDRV_BMI_IH_H__
