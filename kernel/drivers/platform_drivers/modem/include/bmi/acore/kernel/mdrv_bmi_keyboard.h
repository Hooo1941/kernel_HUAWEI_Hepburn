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
 *  @brief   飞龙架构2.0 keyboard模块对外头文件
 *  @file    mdrv_bmi_keyboard.h
 *  @version v1.0
 *  @date    2023.07.25
 *  @note    该文件增删改需通过接口变更评审
 *  <ul><li>v1.0|2023.07.25|创建文件</li></ul>
 *  @since
 */

#ifndef _MDRV_BMI_KEYBOARD_H_
#define _MDRV_BMI_KEYBOARD_H_

#include <linux/input.h>
#include <linux/gpio_keys.h>
#include <linux/timer.h>
#include <linux/spinlock.h>
#include <linux/device.h>

#define MAX_KEY_LABEL_LEN 32

struct gpio_button_data {
    const struct gpio_keys_button *button;
    struct device *dev;
    struct input_dev *input;
    struct timer_list timer;
    unsigned int timer_debounce; /* in msecs */
    unsigned int irq;
    spinlock_t lock;
    unsigned char label[MAX_KEY_LABEL_LEN]; /* 电源名字 */
    unsigned int disabled : 1; /* 是否使能 */
    unsigned int key_pressed : 1; /* 按键抬起/按下状态 */
    unsigned int is_suspend : 1; /* GPIO控制的按键是否进入休眠 */
    unsigned int event_type : 2; /* 按键是否在休眠中未唤醒前被按下 */
};

struct key_cust_ops {
    int (*init)(void *data); /* 驱动初始化 */
    void (*exit)(void); /* 驱动卸载 */
    void (*power_key_press)(void); /* 电源键按下 */
    void (*power_key_release)(void); /* 电源键抬起 */
    void (*gpio_key_handle)(struct gpio_button_data *bdata); /* gpio key处理 */
};

/**
 * @brief 用于注册产品线定制操作钩子的接口
 *
 * @par 描述:
 * 根据pattr的描述，打开通道，通道必须打开之后才能进行数据收发
 *
 * @param[in]  cust_ops, 定制操作钩子
 *
 * @retval 0表示注册成功，非0表示注册失败
 */
int mdrv_key_cust_ops_register(struct key_cust_ops *cust_ops);

/**
 * @brief 用于电源按键抬起事件消息上报
 *
 * @par 描述:
 * 电源按键抬起事件消息上报
 *
 * @param[in]  data, power key按键信息结构体
 *
 */
void mdrv_key_report_power_key_up(void *data);

/**
 * @brief 用于电源按键按下事件消息上报
 *
 * @par 描述:
 * 电源按键按下事件消息上报
 *
 * @param[in]  data, power key按键信息结构体
 *
 */
void mdrv_key_report_power_key_down(void *data);

#endif
