/*
 * asp_codec_utils.c -- codec driver
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

#include "asp_codec_utils.h"

#include <platform_include/basicplatform/linux/mfd/pmic_platform.h>
#include <sound/soc.h>
#include <linux/io.h>
#include <linux/clk.h>
#include <linux/pm_runtime.h>
#include <linux/regulator/consumer.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>

#include "audio_log.h"
#include "asp_codec_reg.h"

#define LOG_TAG "Analog_less_v2_utils"

#define INVALID_REG_VALUE 0xFFFFFFFF

#define INTER_LOCK_TIMEOUT_CNT 300

/*
 * Since the real physical address is required for mapping register addresses,
 * the symbol bit of the base address needs to be added.
 */

static int get_reg_array_index(unsigned int reg, struct asp_codec_reg_page *reg_array, unsigned int size)
{
	unsigned int i;
	for (i = 0; i < size; i++) {
		if (reg >= reg_array[i].page_reg_begin && reg <= reg_array[i].page_reg_end)
			return i;
	}

	AUDIO_LOGE("can not find reg index, reg 0x%x", reg);
	return -1;
}

static void runtime_info_print(struct codec_data *priv)
{
	struct device *dev = NULL;

	if (priv->codec == NULL) {
		AUDIO_LOGE("parameter is null");
		return;
	}

	dev = priv->codec->dev;
	if (dev == NULL) {
		AUDIO_LOGE("dev is null");
		return;
	}

	AUDIO_LOGD("suspend usage count:%d child count:%d",
		atomic_read(&(dev->power.usage_count)),
		atomic_read(&(dev->power.child_count)));
	AUDIO_LOGD("runtime status:0x%x disable depth:%d",
		dev->power.runtime_status, dev->power.disable_depth);
	AUDIO_LOGD("asp subsys clk:%d,clk:%d",
		clk_get_enable_count(priv->resource.asp_subsys_clk),
		clk_get_enable_count(priv->resource.asp_codec_clk));
}

static int runtime_get_sync(struct codec_data *priv)
{
	struct device *dev = NULL;
	int pm_ret = 0;

	IN_FUNCTION;

	if (priv->codec == NULL) {
		AUDIO_LOGE("parameter is null");
		return -EINVAL;
	}

	if (priv->codec->dev == NULL) {
		AUDIO_LOGE("dev is null");
		return -EINVAL;
	}

	dev = priv->codec->dev;

	if (priv->pm.pm_runtime_support) {
		pm_ret = pm_runtime_get_sync(dev);
		AUDIO_LOGD("get pm resume ret:%d", pm_ret);
		if (pm_ret < 0) {
			AUDIO_LOGE("pm resume error, ret:%d", pm_ret);
			return pm_ret;
		}

		runtime_info_print(priv);
	}

	OUT_FUNCTION;

	return pm_ret;
}

static void runtime_put_sync(struct codec_data *priv)
{
	struct device *dev = NULL;

	IN_FUNCTION;

	if (priv->codec == NULL) {
		AUDIO_LOGE("parameter is null");
		return;
	}

	if (priv->codec->dev == NULL) {
		AUDIO_LOGE("dev is null");
		return;
	}

	dev = priv->codec->dev;

	if (priv->pm.pm_runtime_support) {
		pm_runtime_mark_last_busy(dev);
		pm_runtime_put_autosuspend(dev);
		runtime_info_print(priv);
	}

	OUT_FUNCTION;
}

int asp_codec_base_addr_map(struct codec_data *priv)
{
	struct asp_codec_reg_page *reg_array = priv->platform_reg_array;
	unsigned int array_size = priv->platform_reg_size;

	unsigned int i;
	size_t size;
	for (i = 0; i < array_size; i++) {
		if (reg_array[i].page_reg_begin == PAGE_VIRCODEC)
			continue;

		size = reg_array[i].page_reg_end - reg_array[i].page_reg_begin + sizeof(int);
		reg_array[i].reg_base_addr = (char * __force)(ioremap(reset_31_bit(reg_array[i].page_reg_begin), size));
		if (reg_array[i].reg_base_addr == NULL) {
			AUDIO_LOGE("cannot map register memory:%d", i);
			return -ENOMEM;
		}
	}

	return 0;
}

void asp_codec_base_addr_unmap(struct codec_data *priv)
{
	struct asp_codec_reg_page *reg_array = priv->platform_reg_array;
	unsigned int array_size = priv->platform_reg_size;

	unsigned int i;
	for (i = 0; i < array_size; i++) {
		if (reg_array[i].reg_base_addr) {
			iounmap(reg_array[i].reg_base_addr);
			reg_array[i].reg_base_addr = NULL;
		}
	}
}

int asp_codec_reg_write_direct(unsigned int remap_addr, unsigned int value)
{
	void __iomem *reg_base_addr;

	reg_base_addr = (char * __force)(ioremap(reset_31_bit(remap_addr), sizeof(int)));
	if (reg_base_addr == NULL) {
		AUDIO_LOGE("cannot map register memory");
		return -ENOMEM;
	}

	writel(value, reg_base_addr);

	if (reg_base_addr) {
		iounmap(reg_base_addr);
		reg_base_addr = NULL;
	}

	return 0;
}

static_t unsigned int vircodec_reg_read(struct codec_data *priv, unsigned int reg_offset)
{
	if (reg_offset >= priv->v_codec_num)
		return 0;

	return priv->v_codec_reg[reg_offset];
}

static_t unsigned int reg_read(struct codec_data *priv, unsigned int reg)
{
	unsigned int ret = 0;
	unsigned long flags;
	unsigned int reg_offset = 0;
	struct asp_codec_reg_page *reg_array = priv->platform_reg_array;
	unsigned int array_size = priv->platform_reg_size;
	int index = 0;

	reg = clear_31_bit(reg);

	index = get_reg_array_index(reg, reg_array, array_size);
	if (index < 0)
		return INVALID_REG_VALUE;

	if (reg_array[index].is_asp_reg_type && priv->pm.asp_pd) {
		AUDIO_LOGE("asp power down");
		return INVALID_REG_VALUE;
	}

	if (reg_array[index].is_asp_reg_type && runtime_get_sync(priv) < 0) {
		AUDIO_LOGE("runtime resume failed");
		return INVALID_REG_VALUE;
	}

	reg_offset = reg - reg_array[index].page_reg_begin;

	spin_lock_irqsave(&priv->lock, flags);
	if (reg_array[index].is_virtual_reg)
		ret = vircodec_reg_read(priv, reg_offset);
	else
		ret = readl(reg_array[index].reg_base_addr + reg_offset);
	spin_unlock_irqrestore(&priv->lock, flags);

	AUDIO_LOGD("reg read reg = 0x%x, ret = 0x%x", (void *)(uintptr_t)reg, ret);
	if (reg_array[index].is_asp_reg_type)
		runtime_put_sync(priv);

	return ret;
}


static_t void vircodec_reg_write(struct codec_data *priv, unsigned int reg_offset, unsigned int value)
{
	if (reg_offset < priv->v_codec_num)
		priv->v_codec_reg[reg_offset] = value;
}

static_t void reg_write(struct codec_data *priv, unsigned int reg, unsigned int value)
{
	unsigned long flags;
	unsigned int reg_offset = 0;
	struct asp_codec_reg_page *reg_array = priv->platform_reg_array;
	unsigned int array_size = priv->platform_reg_size;
	int index = 0;

	reg = clear_31_bit(reg);
	index = get_reg_array_index(reg, reg_array, array_size);
	if (index < 0)
		return;

	if (reg_array[index].is_asp_reg_type && priv->pm.asp_pd) {
		AUDIO_LOGE("asp power down");
		return;
	}

	if (reg_array[index].is_asp_reg_type && runtime_get_sync(priv) < 0) {
		AUDIO_LOGE("runtime resume failed");
		return;
	}

	reg_offset = reg - reg_array[index].page_reg_begin;

	spin_lock_irqsave(&priv->lock, flags);

	AUDIO_LOGD("write reg = 0x%x, value:0x%x", (void *)(uintptr_t)reg, value);
	if (reg_array[index].is_virtual_reg)
		vircodec_reg_write(priv, reg_offset, value);
	else
		writel(value, reg_array[index].reg_base_addr + reg_offset);
	spin_unlock_irqrestore(&priv->lock, flags);
	if (reg_array[index].is_asp_reg_type)
		runtime_put_sync(priv);
}

unsigned int asp_codec_reg_read(struct snd_soc_component *codec,
	unsigned int reg)
{
	unsigned int ret;
	struct codec_data *priv = NULL;

	IN_FUNCTION;

	if (codec == NULL) {
		AUDIO_LOGE("parameter is null");
		return INVALID_REG_VALUE;
	}

	priv = snd_soc_component_get_drvdata(codec);
	if (priv == NULL) {
		AUDIO_LOGE("codec data is null");
		return INVALID_REG_VALUE;
	}

	ret = reg_read(priv, reg);
	if (ret == INVALID_REG_VALUE)
		AUDIO_LOGW("reg %pK read value 0x%x maybe invalid",
			(void *)(uintptr_t)reg, ret);

	return ret;
}

int asp_codec_reg_write(struct snd_soc_component *codec, unsigned int reg,
	unsigned int value)
{
	struct codec_data *priv = NULL;

	if (codec == NULL) {
		AUDIO_LOGE("parameter is null");
		return -EINVAL;
	}

	priv = snd_soc_component_get_drvdata(codec);
	if (priv == NULL) {
		AUDIO_LOGE("codec data is null");
		return -EINVAL;
	}

	reg_write(priv, reg, value);

	return 0;
}

static void intercore_try_lock(struct snd_soc_component *component)
{
	uint32_t time_cnt = INTER_LOCK_TIMEOUT_CNT;

	while (time_cnt) {
		if (asp_codec_reg_read(component, RESERVED_LOCK_ADDR) == 0) {
			break;
		}
		udelay(10);
		time_cnt--;
	}

	if (time_cnt == 0) {
		AUDIO_LOGE("try lock timeout");
		return;
	}

	asp_codec_reg_write(component, RESERVED_LOCK_ADDR, 1);
}

static void intercore_unlock(struct snd_soc_component *component)
{
	asp_codec_reg_write(component, RESERVED_LOCK_ADDR, 0);
}

int snd_soc_component_update_bits_with_lock(struct snd_soc_component *component,
	unsigned int reg, unsigned int mask, unsigned int val)
{
	int ret;
	intercore_try_lock(component);
	ret = snd_soc_component_update_bits(component, reg, mask, val);
	intercore_unlock(component);
	return ret;
}
