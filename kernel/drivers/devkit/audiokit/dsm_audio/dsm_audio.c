/*
 * dsm_audio.c
 *
 * dsm audio driver
 *
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
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
#include "dsm_audio.h"
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/string.h>
#include <huawei_platform/log/hw_log.h>

#define ERROR_LEVEL 1
#define INFO_LEVEL  1
#define DEBUG_LEVEL 0

#define HWLOG_TAG dsm_audio
#if DEBUG_LEVEL
#define HWLOG_LEVEL (HWLOG_ERR | HWLOG_WARNING | HWLOG_INFO | HWLOG_DEBUG)
#elif INFO_LEVEL
#define HWLOG_LEVEL (HWLOG_ERR | HWLOG_WARNING | HWLOG_INFO)
#elif ERROR_LEVEL
#define HWLOG_LEVEL (HWLOG_ERR)
#else
#define HWLOG_LEVEL 0
#endif
HWLOG_REGIST_LEVEL(HWLOG_LEVEL);

struct dsm_audio_client {
	struct dsm_client *dsm_client;
	char *dsm_str_info_buffer;
};

#ifdef CONFIG_HISYSEVENT
struct error_no_map {
	int error_no;
	char *name;
};

static int audio_errorno_to_str(int error_no, char *str, int buff_len);

static struct dsm_client_ops audio_dsm_ops = {
	.poll_state = NULL,
	.dump_func = NULL,
	.errorno_to_str = audio_errorno_to_str,
};

static const struct error_no_map audio_error_no_map[] = {
	{ 921001000, "SOC_HIFI_RESET" },
	{ 921001001, "CODEC_HIFI_RESET" },
	{ 921001003, "HIFI_HIGH_CPU" },
	{ 921001005, "CODEC_PLL_UNLOCK" },
	{ 921001006, "CODEC_PLL_CANNOT_LOCK" },
	{ 921001007, "SLIMBUS_READ_ERR" },
	{ 921001008, "SLIMBUS_LOST_MS" },
	{ 921001009, "MBHC_HS_ERR_TYPE" },
	{ 921001031, "SOC_HIFI_3A_ERROR" },
	{ 921001032, "CODEC_HIFI_TIMEOUT" },
	{ 921001034, "THIRD_CODEC_I2C_ERR" },
	{ 921001035, "THIRD_CODEC_PROBE_ERR" },
	{ 921001036, "CODEC_BUNK1_OCP" },
	{ 921001037, "CODEC_BUNK1_SCP" },
	{ 921001038, "CODEC_BUNK2_OCP" },
	{ 921001039, "CODEC_BUNK2_SCP" },
	{ 921001040, "CODEC_CP1_SHORT" },
	{ 921001041, "CODEC_CP2_SHORT" },
	{ 921001044, "HIFIUSB_HC_DIED" },
	{ 921001045, "HIFI_USB_START_ERR" },
	{ 921001047, "HIFI_USB_STOP_ERR" },
	{ 921001051, "CODEC_HIFI_LOAD_FAIL" },
	{ 921001052, "CODEC_HIFI_SYNC_TIMEOUT" },
	{ 921001055, "CODEC_SSI_READ_ONCE" },
	{ 921001057, "CODEC_CHECK_ERR" },
	{ 921002008, "ANC_HS_UNHANDLED_IRQ" },
	{ 921002009, "ANC_HS_I2C_ERR" },
	{ 921003000, "SMARTPA_I2C_ABNORMAL" },
	{ 921003001, "SMARTPA_INT_ABNORMAL" },
};
#endif /* CONFIG_HISYSEVENT */

static struct dsm_dev dsm_audio = {
#ifdef CONFIG_HISYSEVENT
	.name = "AUDIO_DRV",
#else
	.name = DSM_AUDIO_NAME,
#endif /* CONFIG_HISYSEVENT */
	.device_name = NULL,
	.ic_name = NULL,
	.module_name = NULL,
#ifdef CONFIG_HISYSEVENT
	.fops = &audio_dsm_ops,
#else
	.fops = NULL,
#endif /* CONFIG_HISYSEVENT */
	.buff_size = DSM_AUDIO_BUF_SIZE,
};

static struct dsm_dev dsm_smartpa = {
	.name = DSM_SMARTPA_NAME,
	.device_name = NULL,
	.ic_name = NULL,
	.module_name = NULL,
	.fops = NULL,
	.buff_size = DSM_SMARTPA_BUF_SIZE,
};

static struct dsm_dev dsm_anc_hs = {
	.name = DSM_ANC_HS_NAME,
	.device_name = NULL,
	.ic_name = NULL,
	.module_name = NULL,
	.fops = NULL,
	.buff_size = DSM_ANC_HS_BUF_SIZE,
};

static struct dsm_audio_client *dsm_audio_client_table;
static struct dsm_dev *dsm_dev_table[AUDIO_DEVICE_MAX] = {
#ifdef CONFIG_HISYSEVENT
	&dsm_audio, NULL, NULL
#else
	&dsm_audio, &dsm_smartpa, &dsm_anc_hs
#endif /* CONFIG_HISYSEVENT */
};

#ifdef CONFIG_HISYSEVENT
static int audio_errorno_to_str(int error_no, char *str, int buff_len)
{
	int i;
	int audio_error_no_map_len;

	if (str == NULL)
		return -EINVAL;

	audio_error_no_map_len = sizeof(audio_error_no_map) / sizeof(audio_error_no_map[0]);
	for (i = 0; i < audio_error_no_map_len; i++) {
		if (audio_error_no_map[i].error_no == error_no) {
			if (strlen(audio_error_no_map[i].name) >= buff_len)
				return -1;

			strncpy(str, audio_error_no_map[i].name, buff_len - 1);
			hwlog_info("error_no:%d, err_str:%s\n", error_no, str);
			return 0;
		}
	}
	return -1;
}
#endif /* CONFIG_HISYSEVENT */

static int audio_dsm_register(struct dsm_audio_client *dsm_client,
		struct dsm_dev *dsm_dev)
{
	if (dsm_dev == NULL || dsm_client == NULL)
		return -EINVAL;

	dsm_client->dsm_client = dsm_register_client(dsm_dev);
	if (dsm_client->dsm_client == NULL) {
		dsm_client->dsm_client =
			dsm_find_client((char *)dsm_dev->name);
		if (dsm_client->dsm_client == NULL) {
			hwlog_err("dsm_audio_client register failed\n");
			return -ENOMEM;
		}
		hwlog_err("dsm_audio_client find in dsm_server\n");
	}

	dsm_client->dsm_str_info_buffer =
		kzalloc(dsm_dev->buff_size, GFP_KERNEL);
	if (!dsm_client->dsm_str_info_buffer) {
		hwlog_err("dsm audio %s malloc buffer failed\n",
				dsm_dev->name);
		return -ENOMEM;
	}

	return 0;
}

static int audio_dsm_init(void)
{
	int i, ret;
	size_t size;

	if (dsm_audio_client_table == NULL) {
		size = sizeof(*dsm_audio_client_table) * AUDIO_DEVICE_MAX;
		dsm_audio_client_table = kzalloc(size, GFP_KERNEL);
		if (dsm_audio_client_table == NULL) {
			hwlog_err("dsm_audio_client table malloc failed\n");
			return -ENOMEM;
		}
	}

	for (i = 0; i < AUDIO_DEVICE_MAX; i++) {
		if (dsm_audio_client_table[i].dsm_client != NULL ||
			dsm_dev_table[i] == NULL)
			continue;

		ret = audio_dsm_register(dsm_audio_client_table + i,
				dsm_dev_table[i]);
		if (ret)
			hwlog_err("dsm dev %s register failed %d\n",
					dsm_dev_table[i]->name, ret);
	}

	return 0;
}
static void audio_dsm_deinit(void)
{
	int i;

	if (dsm_audio_client_table == NULL)
		return;

	for (i = 0; i < AUDIO_DEVICE_MAX; i++) {
		// kfree(NULL) is safe, check not required
		kfree(dsm_audio_client_table[i].dsm_str_info_buffer);
		dsm_audio_client_table[i].dsm_str_info_buffer = NULL;
	}

	kfree(dsm_audio_client_table);
	dsm_audio_client_table = NULL;
}

int audio_dsm_report_num(enum audio_device_type dev_type, int error_no,
		unsigned int mesg_no)
{
	int err;
#ifdef CONFIG_HISYSEVENT
	dev_type = 0;
#endif /* CONFIG_HISYSEVENT */
	if (dsm_audio_client_table == NULL ||
		dsm_audio_client_table[dev_type].dsm_client == NULL) {
		hwlog_err("dsm_audio_client did not register\n");
		return -EINVAL;
	}

	err = dsm_client_ocuppy(dsm_audio_client_table[dev_type].dsm_client);
	if (err != 0) {
		hwlog_err("user buffer is busy\n");
		return -EBUSY;
	}

	hwlog_debug("report error_no=0x%x, mesg_no=0x%x\n", error_no, mesg_no);
	dsm_client_record(dsm_audio_client_table[dev_type].dsm_client,
			"Message code = 0x%x\n", mesg_no);
	dsm_client_notify(dsm_audio_client_table[dev_type].dsm_client,
			error_no);
	return 0;
}
EXPORT_SYMBOL(audio_dsm_report_num);

int audio_dsm_report_info(enum audio_device_type dev_type, int error_no,
		char *fmt, ...)
{
	int ret, err;
	va_list args;

	hwlog_info("begin, errorno %d, dev_type %d", error_no, dev_type);
#ifdef CONFIG_HISYSEVENT
	dev_type = 0;
#endif /* CONFIG_HISYSEVENT */
	if ((dsm_audio_client_table == NULL) ||
		(dsm_audio_client_table[dev_type].dsm_client == NULL)) {
		hwlog_err("dsm_audio_client did not register\n");
		ret = -EINVAL;
		goto out;
	}

	va_start(args, fmt);
	ret = vsnprintf(dsm_audio_client_table[dev_type].dsm_str_info_buffer,
		dsm_dev_table[dev_type]->buff_size, fmt, args);
	va_end(args);

	err = dsm_client_ocuppy(dsm_audio_client_table[dev_type].dsm_client);
	if (err != 0) {
		hwlog_err("user buffer is busy\n");
		ret = -EBUSY;
		goto out;
	}

	hwlog_debug("report dsm_error_no = %d, %s\n",
			error_no,
			dsm_audio_client_table[dev_type].dsm_str_info_buffer);
	dsm_client_record(dsm_audio_client_table[dev_type].dsm_client, "%s\n",
			dsm_audio_client_table[dev_type].dsm_str_info_buffer);
	dsm_client_notify(dsm_audio_client_table[dev_type].dsm_client,
			error_no);
out:
	return ret;
}
EXPORT_SYMBOL(audio_dsm_report_info);

subsys_initcall_sync(audio_dsm_init);
module_exit(audio_dsm_deinit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Huawei dsm audio driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
