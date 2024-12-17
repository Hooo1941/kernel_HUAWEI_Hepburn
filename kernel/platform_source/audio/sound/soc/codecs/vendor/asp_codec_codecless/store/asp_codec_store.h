/*
 * asp_codec_store.h -- codec driver
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
#ifndef __ASP_CODEC_STORE_H__
#define __ASP_CODEC_STORE_H__

#include <sound/soc.h>

#include "asp_codec_utils.h"

struct snd_codec_resource {
	const char *name;
	const struct snd_kcontrol_new *controls;
	unsigned int num_controls;
	const struct snd_soc_dapm_widget *widgets;
	unsigned int num_widgets;
};

struct snd_codec_route {
	const struct snd_soc_dapm_route *routes;
	unsigned int num_routes;
};

int asp_codec_store_merge_path(struct snd_soc_component *codec);
void deinit_codec_path_list(void);
int add_codec_base_dapm_list(const struct snd_codec_resource *codec_source);
int add_codec_platform_dapm_list(const struct snd_codec_resource *codec_source);
void add_codec_add_route(const struct snd_codec_route *codec_route);

#endif

