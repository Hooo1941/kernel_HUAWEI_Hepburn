/*
 * asp_codec_utils.h -- codec driver
 *
 * Copyright (c) 2020 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#ifndef __ASP_CODEC_UTILS_H__
#define __ASP_CODEC_UTILS_H__

#include <sound/soc.h>
#include <linux/version.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>

#ifdef CONFIG_SND_SOC_CODEC_DEBUG
#include "codec_debug.h"
#endif

#include "asp_codec_comm.h"

#define max_val_on_bit(bit) ((1U << (bit)) - 1)
#define mask_on_bit(bit, offset) (max_val_on_bit(bit) << (offset))
#define reset_31_bit(base) ((base) | (0x80000000))

#ifdef LLT_TEST
#define static_t
#else
#define static_t static
#endif

int asp_codec_base_addr_map(struct codec_data *priv);
void asp_codec_base_addr_unmap(struct codec_data *priv);
unsigned int asp_codec_reg_read(struct snd_soc_component *codec, unsigned int reg);
int asp_codec_reg_write(struct snd_soc_component *codec, unsigned int reg, unsigned int value);
int snd_soc_component_update_bits_with_lock(struct snd_soc_component *component,
	unsigned int reg, unsigned int mask, unsigned int val);
int asp_codec_reg_write_direct(unsigned int remap_addr, unsigned int value);
#endif

