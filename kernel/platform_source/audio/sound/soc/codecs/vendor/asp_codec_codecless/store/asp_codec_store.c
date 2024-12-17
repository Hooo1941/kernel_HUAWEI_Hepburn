/*
 * asp_codec_store.c -- codec driver
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

#include "asp_codec_store.h"
#include "audio_log.h"

#define LOG_TAG "Analog_less_store"

struct codec_dapm_list {
	const struct snd_codec_resource *codec_source;
	struct list_head node;
};
static LIST_HEAD(g_codec_override_dapm_list);
static LIST_HEAD(g_codec_base_dapm_list);
const struct snd_codec_route *g_codec_routes = NULL;

static bool is_override_controls(const struct snd_kcontrol_new *control)
{
	struct codec_dapm_list *override_dapm_list = NULL;
	const struct snd_codec_resource *override_codec_source;
	unsigned int i = 0;
	bool is_override = false;

	list_for_each_entry(override_dapm_list, &g_codec_override_dapm_list, node) {
		override_codec_source = override_dapm_list->codec_source;
		for (i = 0; i < override_codec_source->num_controls; ++i) {
			if (strcmp(control->name, override_codec_source->controls[i].name) == 0) {
				is_override = true;
				AUDIO_LOGI("is override component = %s", control->name);
				break;
			}
		}
		if (is_override)
			break;
	}

	return is_override;
}

static int add_base_controls(struct snd_soc_component *codec, const struct snd_codec_resource *codec_source)
{
	unsigned int i = 0;
	int ret = 0;

	for (i = 0; i < codec_source->num_controls; ++i) {
		if (!is_override_controls(&(codec_source->controls[i]))) {
			ret = snd_soc_add_component_controls(codec, &(codec_source->controls[i]), 1);
			if (ret != 0) {
				AUDIO_LOGE("add kcontrol %s failed, ret: %d", codec_source->controls[i].name, ret);
				return ret;
			}
		}
	}
	return ret;
}

static bool is_override_widget(const struct snd_soc_dapm_widget *widget)
{
	struct codec_dapm_list *override_dapm_list = NULL;
	const struct snd_codec_resource *override_codec_source;
	unsigned int i = 0;
	bool is_override = false;

	list_for_each_entry(override_dapm_list, &g_codec_override_dapm_list, node) {
		override_codec_source = override_dapm_list->codec_source;
		for (i = 0; i < override_codec_source->num_widgets; ++i) {
			if (strcmp(widget->name, override_codec_source->widgets[i].name) == 0) {
				is_override = true;
				AUDIO_LOGI("is override widget = %s", widget->name);
				break;
			}
		}
		if (is_override)
			break;
	}

	return is_override;
}

static int add_base_widgets(struct snd_soc_dapm_context *dapm, const struct snd_codec_resource *codec_source)
{
	unsigned int i = 0;
	int ret = 0;

	for (i = 0; i < codec_source->num_widgets; ++i) {
		if (!is_override_widget(&(codec_source->widgets[i]))) {
			ret = snd_soc_dapm_new_controls(dapm, &(codec_source->widgets[i]), 1);
			if (ret != 0) {
				AUDIO_LOGE("add widget %s failed, ret: %d", codec_source->widgets[i].name, ret);
				return ret;
			}
		}
	}
	return ret;
}

static int add_base_source(struct snd_soc_component *codec, struct snd_soc_dapm_context *dapm)
{
	struct list_head *base_dapm_head = NULL;
	struct codec_dapm_list *base_dapm_list = NULL;
	const struct snd_codec_resource *base_codec_source;
	int ret = 0;

	list_for_each(base_dapm_head, &g_codec_base_dapm_list) {
		base_dapm_list = list_entry(base_dapm_head, struct codec_dapm_list, node);
		base_codec_source = base_dapm_list->codec_source;

		ret = add_base_controls(codec, base_codec_source);
		if (ret != 0)
			return ret;

		ret = add_base_widgets(dapm, base_codec_source);
		if (ret != 0)
			return ret;
	}
	return ret;
}

static int add_platform_source(struct snd_soc_component *codec, struct snd_soc_dapm_context *dapm)
{
	struct list_head *override_dapm_head = NULL;
	struct codec_dapm_list *override_dapm_list = NULL;
	const struct snd_codec_resource *override_codec_source;
	int ret = 0;

	list_for_each(override_dapm_head, &g_codec_override_dapm_list) {
		override_dapm_list = list_entry(override_dapm_head, struct codec_dapm_list, node);
		override_codec_source = override_dapm_list->codec_source;

		ret = snd_soc_add_component_controls(codec, override_codec_source->controls, override_codec_source->num_controls);
		if (ret != 0) {
			AUDIO_LOGE("add kcontrols failed, ret: %d", ret);
			return ret;
		}

		ret = snd_soc_dapm_new_controls(dapm, override_codec_source->widgets, override_codec_source->num_widgets);
		if (ret != 0) {
			AUDIO_LOGE("add widgets failed, ret: %d", ret);
			return ret;
		}
	}
	return ret;
}

int asp_codec_store_merge_path(struct snd_soc_component *codec)
{
	struct snd_soc_dapm_context *dapm = NULL;
	int ret = 0;

	if (codec == NULL) {
		AUDIO_LOGE("parameter is null");
		return -EINVAL;
	}

	dapm = snd_soc_component_get_dapm(codec);
	if (dapm == NULL) {
		AUDIO_LOGE("dapm is null");
		return -EINVAL;
	}

	ret = add_base_source(codec, dapm);
	if (ret != 0) {
		AUDIO_LOGE("add base source fail, ret: %d", ret);
		return ret;
	}

	ret = add_platform_source(codec, dapm);
	if (ret != 0) {
		AUDIO_LOGE("add platform source fail, ret: %d", ret);
		return ret;
	}

	ret = snd_soc_dapm_add_routes(dapm, g_codec_routes->routes, g_codec_routes->num_routes);
	if (ret != 0) {
		AUDIO_LOGE("add routes failed, ret: %d", ret);
		return ret;
	}

	return ret;
}

void deinit_codec_path_list(void)
{
	struct codec_dapm_list *dapm_node = NULL;
	struct codec_dapm_list *n = NULL;

	list_for_each_entry_safe(dapm_node, n, &g_codec_base_dapm_list, node) {
		list_del(&dapm_node->node);
		kfree(dapm_node);
	}
	list_for_each_entry_safe(dapm_node, n, &g_codec_override_dapm_list, node) {
		list_del(&dapm_node->node);
		kfree(dapm_node);
	}
	AUDIO_LOGI("deinit codec list");
}

int add_codec_base_dapm_list(const struct snd_codec_resource *codec_source)
{
	struct codec_dapm_list *dapm_node = NULL;

	dapm_node = kzalloc(sizeof(*dapm_node), GFP_KERNEL);
	if (dapm_node == NULL) {
		AUDIO_LOGE("malloc failed");
		return -EINVAL;
	}

	dapm_node->codec_source = codec_source;
	AUDIO_LOGI("add base dapm %s", codec_source->name);
	list_add_tail(&dapm_node->node, &g_codec_base_dapm_list);
	return 0;
}

int add_codec_platform_dapm_list(const struct snd_codec_resource *codec_source)
{
	struct codec_dapm_list *dapm_node = NULL;

	dapm_node = kzalloc(sizeof(*dapm_node), GFP_KERNEL);
	if (dapm_node == NULL) {
		AUDIO_LOGE("malloc failed");
		return -EINVAL;
	}

	dapm_node->codec_source = codec_source;
	AUDIO_LOGI("add override dapm %s", codec_source->name);
	list_add_tail(&dapm_node->node, &g_codec_override_dapm_list);
	return 0;
}

void add_codec_add_route(const struct snd_codec_route *codec_route)
{
	g_codec_routes = codec_route;
}
