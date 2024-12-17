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

#ifndef __MDRV_BMI_LCD_H__
#define __MDRV_BMI_LCD_H__

#include <linux/fb.h>
#include <linux/device.h>
struct lcd_fb_par;
struct lcd_bl;
struct lcd_platform_data;

/**
 * @brief lcd 初始化序列结构
 */
struct lcd_seq {
    uint8_t format;
    uint8_t value;
};

/**
 * @brief lcd屏幕操作接口
 */
struct fbtft_ops {
    int (*set_addr_win)(struct fb_info *info, int xs, int ys, int xe, int ye); /* 设置屏幕显示窗口 */
    void (*reset)(struct lcd_platform_data *pdata); /* lcd复位 */
    void (*init_display)(struct lcd_fb_par *par); /* lcd面板初始化 */
    int (*set_var)(struct lcd_fb_par *par); /* 设置显示旋转角度 */
    void (*sleep_in)(struct fb_info *info); /* lcd进入睡眠模式 */
    void (*sleep_out)(struct fb_info *info); /* lcd退出睡眠模式 */
};

/**
 * @brief lcd屏幕信息
 */
struct lcd_device_info {
    unsigned int width; /* lcd屏幕宽度 */
    unsigned int height; /* lcd屏幕高度 */
    uint32_t lcd_id; /* lcd编号 */
    uint32_t lcd_category; /* lcd种类编号 */
    struct lcd_seq *init_seq; /* lcd初始化序列 */
    int init_seq_sz; /* lcd初始化序列大小 */
    struct fbtft_ops ops; /* lcd面板操作函数 */
    int brightness_offset; /*lcd面板背光偏移*/
};

/**
 * @brief lcd背光操作接口
 */
struct lcd_bl_ops {
    int (*bl_init)(struct lcd_bl *lcdbl, struct platform_device *pdev, struct lcd_platform_data *pdata); /* 背光初始化 */
    void (*bl_set_intensity)(struct led_classdev *led_cladev, enum led_brightness value); /* 设置背光亮度 */
    void (*bl_register)(struct lcd_bl *bl, struct platform_device *pdev); /* 注册背光信息 */
    void (*bl_free)(struct lcd_bl *bl); /* 释放背光信息 */
};

/**
 * @brief lcd 总线操作接口
 **/
struct lcd_bus_ops {
    int32_t (*lcd_bus_init)(struct lcd_device_info *device_info); /* lcd总线初始化 */
    void (*lcd_image_data_transfer)(dma_addr_t src_addr, uint8_t *buf, uint32_t len); /* 通过dma方式传输lcd图像数据 */
    void (*lcd_data_transfer)(uint8_t *data, uint32_t data_len); /* 传输lcd数据 */
    void (*lcd_cmd_transfer)(uint8_t *data); /* 传输lcd命令 */
    void (*lcd_bus_disable_clk)(void); /* lcd总线时钟去使能 */
    void (*lcd_bus_enable_clk)(void); /* lcd总线时钟使能 */
};



typedef struct lcd_device_info *(*lcd_creat)(void);

/**
 * @brief lcd初始化列表注册函数
 *
 * @par 描述:
 * 对于定制化的屏幕，通过此接口，实现屏幕的初始化信息注册，支持多款屏幕定制
 *
 * @param[in]  lcd_creat lcd屏幕创建接口
 * @param[in]  lcd_creat_len lcd屏幕创建数量
 *
 * @retval 0,  表示注册成功
 * @retval 非0, 表示注册失败
 *
 */
int mdrv_register_lcd(lcd_creat* lcd_creat, size_t lcd_creat_len);

/**
 * @brief 获取内核lcd结构信息
 *
 * @par 描述:
 * 通过获取lcd内核的结构信息可以调用lcd的各个控制方法
 *
 *
 * @retval 空,  表示获取失败
 * @retval 非空, 表示获取成功
 *
 */
struct fb_info *mdrv_get_fbinfo(void);

/**
 * @brief 获取lcd总线操作接口
 *
 * @par 描述:
 * 获取lcd总线操作接口，已供显示层做显示开发
 *
 *
 * @retval 空,  表示获取失败
 * @retval 非空, 表示获取成功
 *
 */
struct lcd_bus_ops *mdrv_get_bus_ops(void);

/**
 * @brief 获取lcd背光操作接口
 *
 * @par 描述:
 * 获取lcd背光操作接口
 *
 *
 * @retval 空,  表示获取失败
 * @retval 非空, 表示获取成功
 *
 */
struct lcd_bl_ops *mdrv_get_bl_ops(void);

/**
 * @brief 获取lcd当前的旋转角
 *
 * @par 描述:
 * 获取lcd当前的旋转角
 *
 *
 * @retval 旋转角度
 *
 */
int mdrv_get_lcd_rotate(void);

/**
 * @brief 设置lcd背光亮度
 *
 * @par 描述:
 * 设置lcd背光亮度
 *
 * @param[in]  value lcd屏幕背光亮度
 *
 * @retval 0,  表示设置成功
 * @retval 非0, 表示设置失败
 */
int mdrv_set_lcd_brightness(enum led_brightness value);
#endif
