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

#ifndef __MDRV_BMI_SND_H__
#define __MDRV_BMI_SND_H__
#include <sound/soc-dai.h>

/**
 * @brief codec_dai_name字符串长度最大值
 */
#define CODEC_DAI_NAME_LEN_MAX 128

/**
 * @brief snd回调函数定义
 */
struct snd_codec_config_info {
    unsigned int fmt; /* *<音频格式，取值范围见sound/soc-dai.h */
    unsigned int bclk_freq; /* *<bclk频率，以HZ为单位，最大不超过81920000 */
    unsigned int chn_num; /* *<声道个数，1或2 */
    unsigned int need_invert; /* *<是否需要翻转sio时钟，在回调函数中配置，默认不翻转 */
};

/**
 * @brief snd回调函数定义
 *
 * @par 描述:
 * 在每次音频录放开始时，codec hw_params阶段触发回调，用于配置codec dai
 *
 * @param[in] codec_dai，codec dai信息
 * @param[in] config，配置信息
 *
 */
typedef int (*snd_codec_config_func)(struct snd_soc_dai *codec_dai, struct snd_codec_config_info *config);

/**
 * @brief snd回调注册
 *
 * @param[in] codec_dai_name，codec dai名字，长度不能超过CODEC_DAI_NAME_LEN_MAX
 * @param[in] func，注册的回调函数

 * @retval !=0，表示函数执行失败。
 * @retval =0，表示函数执行成功。
 *
 * @par 依赖:
 * <ul><li>需要在sound驱动初始化完成后才能注册</li></ul>
 *
 */
int mdrv_snd_register_codec_config(const char *codec_dai_name, snd_codec_config_func func);

#endif
