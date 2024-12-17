/*
 * asp_codec_control.c -- codec driver
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

#include "asp_codec_control.h"

#include <linux/module.h>
#include <linux/clk.h>
#include <linux/regulator/machine.h>
#include <linux/pinctrl/consumer.h>
#include <linux/pm_runtime.h>
#include <linux/version.h>
#include <platform_include/cee/linux/ulsr.h>

#include "audio_log.h"
#include "asp_codec_store.h"

#include "asp_codec_reg.h"
#include "asp_codec_hdmi_adapter.h"

/*lint -e548 -e429*/
#define LOG_TAG "Analog_less_control"

#define ASP_CODECLESS_NAME "asp-codecless"

#define AUTOSUSPEND_DELAY_TIME_MS 200


#define PB_MIN_CHANNELS       2
#define PB_MAX_CHANNELS       32
#define CP_MIN_CHANNELS       1
#define CP_MAX_CHANNELS       64
#define PCM_FORMATS (SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_S24_LE \
				| SNDRV_PCM_FMTBIT_S24_3LE | SNDRV_PCM_FMTBIT_S32_LE)
#define PCM_RATES   SNDRV_PCM_RATE_8000_384000

static struct codec_data *g_codec_data;

static int asp_codec_startup(struct snd_pcm_substream *substream,
	struct snd_soc_dai *dai)
{
	return 0;
}

static int asp_codec_hw_params(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params, struct snd_soc_dai *dai)
{
	return 0;
}

static struct snd_soc_dai_ops asp_codec_dai_ops = {
	.startup   = asp_codec_startup,
	.hw_params = asp_codec_hw_params,
};

static struct snd_soc_dai_driver asp_codec_dai[] = {
	{
		.name = "asp-codec-codecless-dai",
		.playback = {
			.stream_name = "Playback",
			.channels_min = PB_MIN_CHANNELS,
			.channels_max = PB_MAX_CHANNELS,
			.rates = PCM_RATES,
			.formats = PCM_FORMATS
		},
		.capture = {
			.stream_name = "Capture",
			.channels_min = CP_MIN_CHANNELS,
			.channels_max = CP_MAX_CHANNELS,
			.rates = PCM_RATES,
			.formats = PCM_FORMATS
		},
		.ops = &asp_codec_dai_ops,
	},
};

static void set_platform_type(struct snd_soc_component *codec)
{
	struct codec_data *priv = snd_soc_component_get_drvdata(codec);
	struct device_node *np = codec->dev->of_node;
	unsigned int val = 0;

	if (of_property_read_u32(np, "udp_io_config", &val) == 0) {
		AUDIO_LOGI("udp io config is %u", val);

		if (val == 1)
			priv->platform_type = UDP_PLATFORM;
		else
			priv->platform_type = FPGA_PLATFORM;
	} else {
		AUDIO_LOGI("no configed platform type, set fpga");
		priv->platform_type = FPGA_PLATFORM;
	}
}

static int asp_resource_init(struct snd_soc_component *codec)
{
	int ret;
	const char *clk_name = NULL;
	struct codec_data *priv = snd_soc_component_get_drvdata(codec);

	priv->resource.asp_regulator = devm_regulator_get(codec->dev, "asp");
	if (IS_ERR_OR_NULL(priv->resource.asp_regulator)) {
		ret = PTR_ERR(priv->resource.asp_regulator);
		AUDIO_LOGE("get asp regulators err: %d", ret);
		goto get_asp_regulator_err;
	}

	priv->resource.asp_subsys_clk = devm_clk_get(codec->dev, "clk_asp_subsys");
	if (IS_ERR_OR_NULL(priv->resource.asp_subsys_clk)) {
		ret = PTR_ERR(priv->resource.asp_subsys_clk);
		AUDIO_LOGE("get clk asp subsys err: %d", ret);
		goto get_asp_subsys_clk_err;
	}

	if (of_property_read_string(codec->dev->of_node, "clk-asp-codec-name", &clk_name) != 0)
		clk_name = "clk_asp_codec";
	AUDIO_LOGI("clk-asp-codec-name: %s", clk_name);

	priv->resource.asp_codec_clk = devm_clk_get(codec->dev, clk_name);
	if (IS_ERR_OR_NULL(priv->resource.asp_codec_clk)) {
		ret = PTR_ERR(priv->resource.asp_codec_clk);
		AUDIO_LOGE("get clk err: %d", ret);
		goto get_asp_codec_clk_err;
	}

	AUDIO_LOGI("asp resource init ok");

	return 0;

get_asp_codec_clk_err:
	priv->resource.asp_codec_clk = NULL;
get_asp_subsys_clk_err:
	priv->resource.asp_subsys_clk = NULL;
get_asp_regulator_err:
	priv->resource.asp_regulator = NULL;

	return ret;
}

static void asp_resource_deinit(struct snd_soc_component *codec)
{
	struct codec_data *priv = snd_soc_component_get_drvdata(codec);

	priv->resource.asp_regulator = NULL;
	priv->resource.asp_subsys_clk = NULL;
	priv->resource.asp_codec_clk = NULL;

	AUDIO_LOGI("asp resource deinit ok");
}

static int asp_regulator_enable(struct snd_soc_component *codec)
{
	int ret;
	struct codec_data *priv = snd_soc_component_get_drvdata(codec);

	ret = regulator_enable(priv->resource.asp_regulator);
	if (ret) {
		AUDIO_LOGE("asp regulator enable failed, ret: %d", ret);
		return ret;
	}

	AUDIO_LOGI("asp regulator enable done");

	return ret;
}

static void asp_regulator_disable(struct snd_soc_component *codec)
{
	struct codec_data *priv = snd_soc_component_get_drvdata(codec);

	if (!IS_ERR_OR_NULL(priv->resource.asp_regulator)) {
		if (regulator_disable(priv->resource.asp_regulator)) {
			AUDIO_LOGE("can not disable asp regulator");
			return;
		}

		AUDIO_LOGI("asp regulator disable ok");
	}
}

static int asp_clk_enable(struct snd_soc_component *codec)
{
	int ret;
	struct codec_data *priv = snd_soc_component_get_drvdata(codec);

	ret = clk_prepare_enable(priv->resource.asp_subsys_clk);
	if (ret) {
		AUDIO_LOGE("asp subsys clk enable failed, ret: %d", ret);
		return ret;
	}

	ret = clk_prepare_enable(priv->resource.asp_codec_clk);
	if (ret) {
		AUDIO_LOGE("clk enable failed, ret: %d", ret);
		clk_disable_unprepare(priv->resource.asp_subsys_clk);
		return ret;
	}

	AUDIO_LOGI("asp clk enable ok");

	return ret;
}

static void asp_clk_disable(struct snd_soc_component *codec)
{
	struct codec_data *priv = snd_soc_component_get_drvdata(codec);

	if (!IS_ERR_OR_NULL(priv->resource.asp_codec_clk)) {
		clk_disable_unprepare(priv->resource.asp_codec_clk);
		AUDIO_LOGI("clk disable ok");
	}

	if (!IS_ERR_OR_NULL(priv->resource.asp_subsys_clk)) {
		clk_disable_unprepare(priv->resource.asp_subsys_clk);
		AUDIO_LOGI("asp subsys clk disable ok");
	}
}

static int asp_resource_enable(struct snd_soc_component *codec)
{
	int ret;

	ret = asp_regulator_enable(codec);
	if (ret) {
		AUDIO_LOGE("asp regulator enable failed, ret: %d", ret);
		return ret;
	}

	ret = asp_clk_enable(codec);
	if (ret) {
		AUDIO_LOGE("asp clk enable failed, ret: %d", ret);
		asp_regulator_disable(codec);
		return ret;
	}

	AUDIO_LOGI("asp resource enable ok");

	return 0;
}

static void asp_resource_disable(struct snd_soc_component *codec)
{
	asp_clk_disable(codec);
	asp_regulator_disable(codec);
}

static int codec_pinctrl_init(struct snd_soc_component *codec)
{
	int ret;
	struct device *dev = codec->dev;
	struct codec_data *priv = snd_soc_component_get_drvdata(codec);

	/* set gpio i2s1 & i2s2 & dmic1 & dmic2 */
	priv->resource.pctrl = devm_pinctrl_get(dev);
	if (IS_ERR_OR_NULL(priv->resource.pctrl)) {
		priv->resource.pctrl = NULL;
		AUDIO_LOGE("can not get pinctrl");
		return -EFAULT;
	}

	priv->resource.pin_default = pinctrl_lookup_state(priv->resource.pctrl,
		PINCTRL_STATE_DEFAULT);
	if (IS_ERR_OR_NULL(priv->resource.pin_default)) {
		AUDIO_LOGE("can not get default state, errno: %li",
			PTR_ERR(priv->resource.pin_default));
		goto pinctrl_operation_err;
	}

	priv->resource.pin_idle = pinctrl_lookup_state(priv->resource.pctrl, PINCTRL_STATE_IDLE);
	if (IS_ERR_OR_NULL(priv->resource.pin_idle)) {
		AUDIO_LOGE("can not get idle state, errno: %li",
			PTR_ERR(priv->resource.pin_idle));
		goto pinctrl_operation_err;
	}

	ret = pinctrl_select_state(priv->resource.pctrl, priv->resource.pin_default);
	if (ret) {
		AUDIO_LOGE("set pinctrl state failed, ret: %d", ret);
		goto pinctrl_operation_err;
	}

	AUDIO_LOGI("pinctrl init ok");

	return 0;

pinctrl_operation_err:
	priv->resource.pin_idle = NULL;
	priv->resource.pin_default = NULL;
	devm_pinctrl_put(priv->resource.pctrl);
	priv->resource.pctrl = NULL;

	return -EFAULT;
}

static void codec_pinctrl_deinit(struct snd_soc_component *codec)
{
	struct codec_data *priv = snd_soc_component_get_drvdata(codec);

	devm_pinctrl_put(priv->resource.pctrl);
	priv->resource.pctrl = NULL;

	AUDIO_LOGI("pinctrl deinit ok");
}

static void codec_pm_runtime_init(struct snd_soc_component *codec)
{
	struct device_node *np = codec->dev->of_node;
	struct codec_data *priv = snd_soc_component_get_drvdata(codec);

	if (of_property_read_bool(np, "pm_runtime_support"))
		priv->pm.pm_runtime_support = true;
	else
		priv->pm.pm_runtime_support = false;

	AUDIO_LOGI("pm runtime support: %d", priv->pm.pm_runtime_support);

	if (priv->pm.pm_runtime_support) {
		pm_runtime_use_autosuspend(codec->dev);
		pm_runtime_set_autosuspend_delay(codec->dev,
			AUTOSUSPEND_DELAY_TIME_MS);
		pm_runtime_set_active(codec->dev);
		pm_runtime_enable(codec->dev);
	}

	AUDIO_LOGI("pm runtrime init ok");
}

static void codec_pm_runtime_deinit(struct snd_soc_component *codec)
{
	struct codec_data *priv = snd_soc_component_get_drvdata(codec);

	if (priv->pm.pm_runtime_support) {
		pm_runtime_resume(codec->dev);
		pm_runtime_disable(codec->dev);
		pm_runtime_set_suspended(codec->dev);
	}

	AUDIO_LOGI("pm runtrime deinit ok");
}

static int codec_resource_init(struct snd_soc_component *codec)
{
	int ret = 0;
	struct codec_data *priv = snd_soc_component_get_drvdata(codec);

	set_platform_type(codec);

	ret = asp_resource_init(codec);
	if (ret) {
		AUDIO_LOGE("asp resource init error, ret: %d", ret);
		return ret;
	}

	ret = asp_resource_enable(codec);
	if (ret) {
		AUDIO_LOGE("asp resource enable error, ret: %d", ret);
		goto asp_resource_enable_err;
	}

	if (priv->platform_type == FPGA_PLATFORM) {
		priv->ops.fpga_pinctrl_init(codec);
	} else {
		ret = codec_pinctrl_init(codec);
		if (ret) {
			AUDIO_LOGE("pinctrl init error, ret: %d", ret);
			goto pinctrl_init_err;
		}
	}

	codec_pm_runtime_init(codec);
	priv->ops.reset_asp_codec(codec);
	priv->ops.asp_reg_init(codec);
	AUDIO_LOGI("resource init ok");

	return 0;

pinctrl_init_err:
	asp_resource_disable(codec);
asp_resource_enable_err:
	asp_resource_deinit(codec);

	return ret;
}

static void codec_resource_deinit(struct snd_soc_component *codec)
{
	struct codec_data *priv = snd_soc_component_get_drvdata(codec);

	codec_pm_runtime_deinit(codec);

	if (priv->platform_type != FPGA_PLATFORM)
		codec_pinctrl_deinit(codec);

	asp_resource_disable(codec);
	asp_resource_deinit(codec);

	AUDIO_LOGI("resource deinit ok");
}

static int codec_priv_init(struct snd_soc_component *codec)
{
	struct codec_data *priv = g_codec_data;

	snd_soc_component_set_drvdata(codec, priv);
	priv->codec = codec;
	priv->v_codec_reg = kzalloc(sizeof(unsigned int) * priv->v_codec_num, GFP_KERNEL);
	if (priv->v_codec_reg == NULL) {
		AUDIO_LOGE("memory alloc error");
		priv->codec = NULL;
		return -ENOMEM;
	}
	priv->ops.priv_data_init(codec);
	spin_lock_init(&priv->lock);

	AUDIO_LOGI("priv init ok");

	return 0;
}

static void codec_priv_deinit(struct snd_soc_component *codec)
{
	struct codec_data *priv = snd_soc_component_get_drvdata(codec);
	if (priv == NULL || priv->ops.priv_data_deinit == NULL || priv->v_codec_reg == NULL)
		return;

	snd_soc_component_set_drvdata(codec, NULL);
	priv->ops.priv_data_deinit(codec);
	kfree(priv->v_codec_reg);
	priv->v_codec_reg = NULL;
	priv->codec = NULL;

	AUDIO_LOGI("priv deinit ok");
}

static int asp_codec_codec_probe(struct snd_soc_component *codec)
{
	struct snd_soc_dapm_context *dapm = NULL;
	int ret;

	IN_FUNCTION;
	if (codec == NULL) {
		AUDIO_LOGE("parameter is null");
		return -EINVAL;
	}

	dapm = snd_soc_component_get_dapm(codec);
	if (dapm == NULL) {
		AUDIO_LOGE("dapm is null");
		return -EINVAL;
	}

	ret = codec_priv_init(codec);
	if (ret) {
		AUDIO_LOGE("priv init failed, ret: %d", ret);
		goto end;
	}

	ret = codec_resource_init(codec);
	if (ret) {
		AUDIO_LOGE("resource init failed, ret: %d", ret);
		goto resource_init_err;
	}

	ret = snd_soc_dapm_sync(dapm);
	if (ret) {
		AUDIO_LOGE("dapm sync error, errno: %d", ret);
		goto dapm_sync_err;
	}

	ret = asp_codec_store_merge_path(codec);
	if (ret) {
		AUDIO_LOGE("init component driver failed, ret: %d", ret);
		goto dapm_sync_err;
	}


	goto end;

dapm_sync_err:
	codec_resource_deinit(codec);
resource_init_err:
	codec_priv_deinit(codec);
end:
	OUT_FUNCTION;

	return ret;
}

static void asp_codec_codec_remove(struct snd_soc_component *codec)
{
	IN_FUNCTION;

	if (codec == NULL) {
		AUDIO_LOGE("parameter is null");
		return;
	}

	codec_resource_deinit(codec);
	codec_priv_deinit(codec);
	OUT_FUNCTION;

	return;
}

static struct snd_soc_component_driver asp_codec_driver = {
	.name = ASP_CODECLESS_NAME,
	.probe = asp_codec_codec_probe,
	.remove = asp_codec_codec_remove,
	.read = asp_codec_reg_read,
	.write = asp_codec_reg_write,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0))
	.idle_bias_on = false,
#else
	.idle_bias_off = true,
#endif
};

static int asp_codec_probe(struct platform_device *pdev)
{
	int ret;

	IN_FUNCTION;

	if (g_codec_data == NULL) {
		AUDIO_LOGE("codec data is null");
		return -EINVAL;
	}

	if (g_codec_data->platform_reg_array == NULL || g_codec_data->platform_reg_size == 0) {
		AUDIO_LOGE("platform reg array invalid");
		return -EINVAL;
	}

	ret = asp_codec_base_addr_map(g_codec_data);
	if (ret) {
		AUDIO_LOGE("base addr map failed, err code %d", ret);
		asp_codec_base_addr_unmap(g_codec_data);
		return ret;
	}
	dev_set_name(&pdev->dev, "asp-codec-codecless");
	ret = devm_snd_soc_register_component(&pdev->dev, &asp_codec_driver,
		asp_codec_dai, ARRAY_SIZE(asp_codec_dai));
	if (ret) {
		AUDIO_LOGE("regist failed, err code 0x%x", ret);
		asp_codec_base_addr_unmap(g_codec_data);
	}

	OUT_FUNCTION;

	return ret;
}

static int asp_codec_remove(struct platform_device *pdev)
{
	IN_FUNCTION;

	asp_codec_base_addr_unmap(g_codec_data);

	OUT_FUNCTION;

	return 0;
}

#ifdef CONFIG_PM
static int asp_codec_runtime_suspend(struct device *dev)
{
	int ret;
	struct codec_data *priv = dev_get_drvdata(dev);

	IN_FUNCTION;

	if (priv == NULL) {
		AUDIO_LOGE("get drvdata failed");
		return -EINVAL;
	}

	ret = pinctrl_select_state(priv->resource.pctrl, priv->resource.pin_idle);
	if (ret) {
		AUDIO_LOGE("set pinctrl state failed, ret: %d", ret);
		return ret;
	}

	asp_clk_disable(priv->codec);

	OUT_FUNCTION;

	return 0;
}

static int asp_codec_runtime_resume(struct device *dev)
{
	int ret;
	struct codec_data *priv = dev_get_drvdata(dev);

	IN_FUNCTION;

	if (priv == NULL) {
		AUDIO_LOGE("get drvdata failed");
		return -EINVAL;
	}

	ret = asp_clk_enable(priv->codec);
	if (ret) {
		AUDIO_LOGE("can not enable asp resource, ret: %d", ret);
		return ret;
	}

	ret = pinctrl_select_state(priv->resource.pctrl, priv->resource.pin_default);
	if (ret) {
		asp_clk_disable(priv->codec);
		AUDIO_LOGE("set pinctrl state failed, ret: %d", ret);
		return ret;
	}

	OUT_FUNCTION;

	return 0;
}
#endif

#ifdef CONFIG_PM_SLEEP
static int asp_codec_suspend(struct device *dev)
{
	int ret;
	struct codec_data *priv = dev_get_drvdata(dev);

	IN_FUNCTION;
	AUDIO_LOGI("suspend begin");

	if (priv == NULL) {
		AUDIO_LOGE("get drvdata failed");
		return -EINVAL;
	}

	if (priv->pm.pm_runtime_support) {
		ret = pm_runtime_get_sync(dev);
		AUDIO_LOGD("pm suspend ret: %d", ret);
		if (ret < 0) {
			AUDIO_LOGE("pm suspend error, ret: %d", ret);
			return ret;
		}
	}

	AUDIO_LOGD("suspend usage count: %d, runtime status: 0x%x",
		atomic_read(&(dev->power.usage_count)),
		dev->power.runtime_status);
	AUDIO_LOGD("disable depth: %d, asp subsys clk: %d, asp codec clk: %d",
		dev->power.disable_depth,
		clk_get_enable_count(priv->resource.asp_subsys_clk),
		clk_get_enable_count(priv->resource.asp_codec_clk));

	if (!priv->pm.have_dapm || is_ulsr()) {
		/* set pin to low power mode */
		ret = pinctrl_select_state(priv->resource.pctrl, priv->resource.pin_idle);
		if (ret) {
			AUDIO_LOGE("set pin state failed, ret: %d", ret);
			return ret;
		}

		asp_resource_disable(priv->codec);
		priv->pm.asp_pd = true;

		AUDIO_LOGI("suspend have_dapm:%d, is_ulsr:%d", priv->pm.have_dapm, is_ulsr());
	}

	AUDIO_LOGI("suspend end");

	OUT_FUNCTION;

	return 0;
}

static int asp_codec_resume(struct device *dev)
{
	int ret;
	struct codec_data *priv = dev_get_drvdata(dev);

	IN_FUNCTION;

	if (priv == NULL) {
		AUDIO_LOGE("get drvdata failed");
		return -EINVAL;
	}

	if (priv->pm.asp_pd) {
		ret = asp_resource_enable(priv->codec);
		if (ret) {
			AUDIO_LOGE("asp resource enable failed, ret: %d", ret);
			return ret;
		}

		priv->pm.asp_pd = false;
		priv->ops.reset_asp_codec(priv->codec);
		priv->ops.asp_reg_init(priv->codec);

		ret = pinctrl_select_state(priv->resource.pctrl, priv->resource.pin_default);
		if (ret) {
			asp_resource_disable(priv->codec);
			AUDIO_LOGE("set pin state failed, ret: %d", ret);
			return ret;
		}
		AUDIO_LOGI("resume from asp pd");
	}

	if (priv->pm.pm_runtime_support) {
		pm_runtime_mark_last_busy(dev);
		pm_runtime_put_autosuspend(dev);

		pm_runtime_disable(dev);
		pm_runtime_set_active(dev);
		pm_runtime_enable(dev);
	}

	AUDIO_LOGD("resume usage count: %d, status: 0x%x",
		atomic_read(&(dev->power.usage_count)),
		dev->power.runtime_status);
	AUDIO_LOGD("disable depth: %d, asp subsys clk: %d, asp codec clk: %d",
		dev->power.disable_depth,
		clk_get_enable_count(priv->resource.asp_subsys_clk),
		clk_get_enable_count(priv->resource.asp_codec_clk));

	OUT_FUNCTION;

	return 0;
}
#endif

static const struct dev_pm_ops asp_codec_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(asp_codec_suspend, asp_codec_resume)
	SET_RUNTIME_PM_OPS(asp_codec_runtime_suspend,
		asp_codec_runtime_resume, NULL)
};

static const struct of_device_id asp_codec_match[] = {
	{ .compatible = "hisilicon,asp-codec-codecless", },
	{ },
};

static struct platform_driver asp_codec_platform_driver = {
	.driver = {
		.name  = "asp-codec-codecless",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(asp_codec_match),
		.pm = &asp_codec_pm_ops,
	},
	.probe  = asp_codec_probe,
	.remove = asp_codec_remove,
};

void asp_codec_driver_init(struct codec_data *base_data)
{
	if (base_data == NULL) {
		AUDIO_LOGE("codec data is null");
		return;
	}

	g_codec_data = base_data;
	asp_codec_platform_driver.shutdown = base_data->ops.shutdown;
}

static int __init asp_codec_init(void)
{
	return platform_driver_register(&asp_codec_platform_driver);
}
module_init(asp_codec_init);

static void __exit asp_codec_exit(void)
{
	IN_FUNCTION;

	deinit_codec_path_list();
	platform_driver_unregister(&asp_codec_platform_driver);

	OUT_FUNCTION;
}
module_exit(asp_codec_exit);

MODULE_DESCRIPTION("ASoC asp codec driver");
MODULE_LICENSE("GPL");
