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

#ifndef __MDRV_BMI_LEDS_H__
#define __MDRV_BMI_LEDS_H__

#include <linux/device.h>
#include <linux/leds.h>
#include <linux/mutex.h>
#include <linux/regulator/consumer.h>

struct iot_led_platdata {
    char *name; /* 内核led类 */
    unsigned long full_on, full_off, fade_on, fade_off; /* led呼吸模式参数 */
    int (*led_breath_set)(struct led_classdev *led_cdev, unsigned long *full_on, unsigned long *full_off,
        unsigned long *fade_on, unsigned long *fade_off); /* led呼吸模式接口 */
    struct regulator *pregulator; /* 为led提供驱动的regulator */
    struct mutex mlock; /* led资源锁 */
};

struct iot_led_device {
    struct led_classdev cdev;        /* 内核led类 */
    struct iot_led_platdata *pdata;  /* led设备的平台特性 */
    struct work_struct work;         /* 亮灭操作需要使用工作队列完成 */
    int brightness;                  /* 需要被设置的亮度 */
    unsigned int category_id;        /* 种类id */
    unsigned int led_id;             /* 某种类led序号 */
    unsigned int default_brightness; /* 默认的亮度 */
    const char *default_trigger;     /* 默认trigger */
};


typedef void (*mbb_led_behavior)(struct iot_led_device *led);

/**
 * @brief 设置led亮度
 *
 * @par 描述:
 * 内核态中设置led亮度接口
 *
 *
 * @retval 非0，设置失败
 * @retval 0，设置成功
 *
 */
int mdrv_led_brightness_set(enum led_brightness brightness, int led_id);


/**
 * @brief 将给出的亮度转换成最接近的电流档位
 *
 * @par 描述:
 * 将给出的亮度转换成最接近的电流档位
 *
 * @param[in]  brightness led亮度
 * @param[in]  led_id led id号
 *
 * @retval 亮度值对应的电流档位
 *
 */
int mdrv_brightness_to_limit(int led_id, int led_brightness);

/**
 * @brief led设备行为注册接口
 *
 * @par 描述:
 * led设备行为注册接口，用于填充led设备中各种特性接口
 *
 * @param[in]  register_behavior 用于填充led行为的接口
 *
 * @retval 非0，注册失败
 * @retval 0，注册成功
 *
 */
int mdrv_led_behavior_register(mbb_led_behavior register_behavior);

/**
 * @brief led获取led_device接口
 *
 * @par 描述:
 * 用于获取led_device，更改led_device属性
 *
 * @param[in]  led_id led id号，编号从0开始
 *
 * @retval 非0，获取失败
 * @retval 0，获取成功
 */
int mdrv_get_led_device(int led_id, struct iot_led_device **led_device);

#endif
