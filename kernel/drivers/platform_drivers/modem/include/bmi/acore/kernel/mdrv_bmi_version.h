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
 *  @brief   飞龙架构2.0 version模块对外头文件
 *  @file    mdrv_bmi_version.h
 *  @version v1.0
 *  @date    2023.07.25
 *  @note    该文件增删改需通过接口变更评审
 *  <ul><li>v1.0|2023.07.25|创建文件</li></ul>
 *  @since
 */

#ifndef _MDRV_BMI_VERSION_H_
#define _MDRV_BMI_VERSION_H_

/**
 * @brief 硬件版本类型（acore）
 */
typedef enum {
    BOARD_TYPE_BBIT = 0, /**< 0表示BBIT */
    BOARD_TYPE_SFT,      /**< 1表示SFT */
    BOARD_TYPE_ASIC,     /**< 2表示ASIC */
    BOARD_TYPE_MAX
} board_actual_type_e;

/**
 * @brief 产品信息结构体（acore）
 */
typedef struct mproduct_info {
    unsigned char *product_name;      /**< 产品名称字符串指针 */
    unsigned int product_name_len;    /**< 产品名称字符串的实际长度（不是最大长度，长度不包括结束符） */
    board_actual_type_e board_a_type; /**< 硬件版本类型，如BBIT/SFT/ASIC */
    unsigned short chip_id;           /**< 芯片平台ID，如B500芯片类型为0xB500 */
    unsigned int product_id;          /* 产品版本号，通过hkadc读取。NV和dts在用 */
} mproduct_info_s;

/**
 * @brief 硬件版本信息结构体（acore）
 */
typedef struct mhw_ver_info {
    int hw_index;                 /**< 硬件版本号，即productid，如0x78000004 */
    unsigned int hw_id_sub;       /**< 硬件子版本号，如0x0表示Ver.A，0x1表示Ver.B */
    unsigned char *hw_full_ver;   /**< 硬件版本信息。 MBB使用 */
    unsigned int hw_full_ver_len; /**< 硬件版本信息字符串的实际长度（不是最大长度，长度不包括结束符） */
    unsigned char *hw_name; /**< 外部产品名称。  MBB产品线根据实际产品命名使用 */
    unsigned int hw_name_len;    /**< 外部产品名称字符串的实际长度（不是最大长度，长度不包括结束符） */
    unsigned char *hw_in_name;   /**< 内部产品名称。
                                    在UDP单板内容同hw_name；MBB产品线根据实际产品命名使用 */
    unsigned int hw_in_name_len; /**< 内部产品名称字符串的实际长度（不是最大长度，长度不包括结束符） */
    unsigned char *firmware;      /* 软件版本号，在ap dts， product_info.dtsi中修改 */
    unsigned int firmware_len;    /* 软件版本号长度 */
    unsigned int chip_level; /**< level信息 */
} mhw_ver_info_s;

/**
 * @brief 软件版本信息结构体
 */
typedef struct msw_ver_info {
    unsigned char *build_time;       /**< 软件编译时间 */
    unsigned int build_time_len;     /**< 软件编译时间长度 */
} msw_ver_info_s;

/**
 * @brief modem版本信息结构体（acore）
 */
typedef struct modem_ver_info {
    mproduct_info_s product_info; /**< 产品版本信息 */
    msw_ver_info_s  sw_ver_info;   /**< 软件版本信息 */
    mhw_ver_info_s hw_ver_info;   /**< 硬件版本信息 */
} modem_ver_info_s;

/**
 * @brief 获取版本信息接口（acore）
 *
 * @par 描述:
 * 该接口返回版本信息结构体的地址，版本信息包含芯片、硬件、软件等版本信息，
 * 可查看modem_ver_info_s结构体成员。
 *
 * @attention
 * <ul><li>版本信息较多，使用时请注意查看modem_ver_info_s结构体及其成员的注释</li></ul>
 *
 * @retval !NULL:版本信息全局变量的地址。
 * @par 依赖:
 * <ul><li>mdrv_version.h：该接口声明所在的头文件。</li></ul>
 *
 * @see modem_ver_info_s
 */
const modem_ver_info_s *mdrv_ver_get_info(void);

#endif

