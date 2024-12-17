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

/**
 * @brief   adc模块在acore上的对外头文件
 * @file    mdrv_adc.h
 * @version v1.0
 * @date    2019.08.20
 * @note    修改记录(版本号|修订日期|说明)
 * <ul><li>v1.0|xxx.xx.xx|创建文件</li></ul>
 * @since
 */
#ifndef __MDRV_ADC_H__
#define __MDRV_ADC_H__

#include <mdrv_bmi_adc.h>

/*****************************************************************************
     *                           Attention                           *
* Description : Driver for adc
* Core        : Acore、Ccore、ll1c、mcore
* Header File : the following head files need to be modified at the same time
*             : /bmi/acore/kernel/mdrv_bmi_adc.h
*             : /acore/mdrv_adc.h
*             : /ccore/mdrv_adc.h
*             : /mcore/mdrv_adc.h
******************************************************************************/
/**
 * @brief 共享区域数据边界魔幻数
 */
#define ADC_MAGIC_DATA (0x5A5A5A5A)

/**
 * @brief 支持的物理通道号列表
 */
typedef enum adc_channel_id {
    ADC_CHANNEL_MIN = 0, /**< 0 acore */
    /* HKADC */
    HKADC_CHANNEL_MIN = ADC_CHANNEL_MIN, /**< 0 acore */
    HKADC_CHANNEL_0 = HKADC_CHANNEL_MIN, /**< 0 acore */
    HKADC_CHANNEL_1,                     /**< 1 acore */
    HKADC_CHANNEL_2,                     /**< 2 acore */
    HKADC_CHANNEL_3,                     /**< 3 acore */
    HKADC_CHANNEL_4,                     /**< 4 acore */
    HKADC_CHANNEL_5,                     /**< 5 acore */
    HKADC_CHANNEL_6,                     /**< 6 acore */
    HKADC_CHANNEL_7,                     /**< 7 acore */
    HKADC_CHANNEL_8,                     /**< 8 acore */
    HKADC_CHANNEL_9,                     /**< 9 acore */
    HKADC_CHANNEL_10,                    /**< 10 acore */
    HKADC_CHANNEL_11,                    /**< 11 acore */
    HKADC_CHANNEL_12,                    /**< 12 acore */
    HKADC_CHANNEL_13,                    /**< 13 acore */
    HKADC_CHANNEL_14,                    /**< 14 acore */
    HKADC_CHANNEL_MAX,                   /**< 15 acore */
    /* HKADC END */
    /* XOADC */
    XOADC_CHANNEL_MIN = HKADC_CHANNEL_MAX, /**< 15 acore */
    XOADC_CHANNEL_0 = XOADC_CHANNEL_MIN,   /**< 15 acore */
    XOADC_CHANNEL_MAX,                     /**< 16 acore */
    /* XOADC END */
    ADC_CHANNEL_MAX = XOADC_CHANNEL_MAX, /**< 16 acore */
    ADC_CHANNEL_NUM = ADC_CHANNEL_MAX,   /**< 16 acore */
} adc_channel_id_e;
/**
 * @brief adc信息结构体
 */
typedef struct adc_data {
    unsigned short code;      /**< adc转换的码值 */
    unsigned short code_r;    /**< adc转换的码值按位取反 */
    unsigned short voltage;   /**< adc转换的电压值 */
    unsigned short voltage_r; /**< adc转换的电压值按位取反 */
} adc_data_s;

/**
 * @brief 共享区域数据排布结构体
 */
typedef struct adc_data_shm {
    unsigned int magic_start;              /**< 其值为ADC_MAGIC_DATA */
    struct adc_data data[ADC_CHANNEL_NUM]; /**< 各通道的adc信息 */
    unsigned int magic_end;                /**< 其值为ADC_MAGIC_DATA */
} adc_data_shm_s;

/*****************************************************************************
      *                        Attention   END                        *
 *****************************************************************************/

#endif
