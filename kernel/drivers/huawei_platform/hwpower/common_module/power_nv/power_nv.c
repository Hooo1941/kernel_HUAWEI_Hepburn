// SPDX-License-Identifier: GPL-2.0
/*
 * power_nv.c
 *
 * nv(non-volatile) interface for power module
 *
 * Copyright (c) 2021-2021 Huawei Technologies Co., Ltd.
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

#include <chipset_common/hwpower/common_module/power_nv.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/mutex.h>
#include <linux/errno.h>
#include <securec.h>
#ifdef CONFIG_NVE_AP_KERNEL
#include <linux/mtd/nve_ap_kernel_interface.h>
#endif
#include <chipset_common/hwpower/common_module/power_printk.h>

#define HWLOG_TAG power_nv
HWLOG_REGIST();

static struct mutex g_power_nv_mutex;

static struct power_nv_data_info g_power_nv_data[] = {
	{ POWER_NV_BLIMSW, 388, "BLIMSW" },
	{ POWER_NV_BBINFO, 389, "BBINFO" },
	{ POWER_NV_HWCOUL, 392, "HWCOUL" },
	{ POWER_NV_COMMON, 426, "BATHEAT" },
	{ POWER_NV_BATCAP, 418, "BATCAP" },
	{ POWER_NV_DEVCOLR, 330, "DEVCOLR" },
	{ POWER_NV_TERMVOL, 425, "TERM_VOL" },
	{ POWER_NV_CHGDIEID, 501, "CHGDIEID" },
	{ POWER_NV_SCCALCUR, 402, "SCCAL" },
	{ POWER_NV_CUROFFSET, 450, "CURCALI" },
	{ POWER_NV_BATACT1, 455, "BATACT1" },
	{ POWER_NV_BATACT2, 456, "BATACT2" },
};

static struct power_nv_data_info *power_nv_get_data(enum power_nv_type type)
{
	int i;
	struct power_nv_data_info *p_data = g_power_nv_data;
	int size = ARRAY_SIZE(g_power_nv_data);

	if ((type < POWER_NV_TYPE_BEGIN) || (type >= POWER_NV_TYPE_END)) {
		hwlog_err("nv_type %d check fail\n", type);
		return NULL;
	}

	for (i = 0; i < size; i++) {
		if (type == p_data[i].type)
			break;
	}

	if (i >= size) {
		hwlog_err("nv_type %d find fail\n", type);
		return NULL;
	}

	hwlog_info("nv [%d]=%d,%u,%s\n",
		i, p_data[i].type, p_data[i].id, p_data[i].name);
	return &p_data[i];
}

#ifdef CONFIG_NVE_AP_KERNEL
static int power_nv_hisi_write(uint32_t nv_number, const char *nv_name,
	const void *data, uint32_t data_len)
{
	struct opt_nve_info_user nv_info;

	if (!nv_name || !data) {
		hwlog_err("nv_name or data is null\n");
		return -EINVAL;
	}

	if (data_len > NVE_NV_DATA_SIZE) {
		hwlog_err("nv write data length %u is invalid\n", data_len);
		return -EINVAL;
	}

	memset(&nv_info, 0, sizeof(nv_info));
	nv_info.nv_operation = NV_WRITE;
	nv_info.nv_number = nv_number;
	strlcpy(nv_info.nv_name, nv_name, NV_NAME_LENGTH);
	nv_info.valid_size = data_len;
	memcpy(&nv_info.nv_data, data, data_len);

	if (nve_direct_access_interface(&nv_info)) {
		hwlog_err("nv %s write fail\n", nv_name);
		return -EINVAL;
	}

	hwlog_info("nv %s,%u write succ\n", nv_name, data_len);
	return 0;
}

static int power_nv_hisi_read(uint32_t nv_number, const char *nv_name,
	void *data, uint32_t data_len)
{
	struct opt_nve_info_user nv_info;

	if (!nv_name || !data) {
		hwlog_err("nv_name or data is null\n");
		return -EINVAL;
	}

	if (data_len > NVE_NV_DATA_SIZE) {
		hwlog_err("nv read data length %u is invalid\n", data_len);
		return -EINVAL;
	}

	memset(&nv_info, 0, sizeof(nv_info));
	nv_info.nv_operation = NV_READ;
	nv_info.nv_number = nv_number;
	strlcpy(nv_info.nv_name, nv_name, NV_NAME_LENGTH);
	nv_info.valid_size = data_len;

	if (nve_direct_access_interface(&nv_info)) {
		hwlog_err("nv %s read fail\n", nv_name);
		return -EINVAL;
	}
	memcpy(data, &nv_info.nv_data, data_len);

	hwlog_info("nv %s,%u read succ\n", nv_name, data_len);
	return 0;
}
#else
static int power_nv_hisi_write(uint32_t nv_number, const char *nv_name,
	const void *data, uint32_t data_len)
{
	return 0;
}

static int power_nv_hisi_read(uint32_t nv_number, const char *nv_name,
	void *data, uint32_t data_len)
{
	return 0;
}
#endif /* CONFIG_NVE_AP_KERNEL */

int power_nv_write(enum power_nv_type type, const void *data, uint32_t data_len)
{
	struct power_nv_data_info *p_data = power_nv_get_data(type);

	if (!p_data)
		return -EINVAL;

	return power_nv_hisi_write(p_data->id, p_data->name, data, data_len);
}

int power_nv_read(enum power_nv_type type, void *data, uint32_t data_len)
{
	struct power_nv_data_info *p_data = power_nv_get_data(type);

	if (!p_data)
		return -EINVAL;

	return power_nv_hisi_read(p_data->id, p_data->name, data, data_len);
}

int power_common_nv_write(enum power_nv_common_type nv_common_type, void *value)
{
	int ret;
	int *bat_heat = NULL;
	struct power_nv_fod_qval *qval = NULL;
	struct power_nv_common_info nv_common_info = { 0 };

	if ((nv_common_type < POWER_NV_COMMON_BEGIN) ||
		(nv_common_type >= POWER_NV_COMMON_END))
		return -EINVAL;

	mutex_lock(&g_power_nv_mutex);
	ret = power_nv_read(POWER_NV_COMMON, &nv_common_info, sizeof(nv_common_info));
	if (ret)
		goto End;

	switch (nv_common_type) {
	case POWER_NV_COMMON_BATHEAT:
		bat_heat = (int *)value;
		if (!bat_heat)
			goto End;
		nv_common_info.bat_heat = (u32)*bat_heat;
		break;
	case POWER_NV_COMMON_QVAL:
		qval = (struct power_nv_fod_qval *)value;
		if (!qval ||
			(memcpy_s(&nv_common_info.qval, sizeof(nv_common_info.qval), qval, sizeof(*qval)) != EOK))
			goto End;
		break;
	default:
		goto End;
	}
	ret = power_nv_write(POWER_NV_COMMON, &nv_common_info, sizeof(nv_common_info));

End:
	mutex_unlock(&g_power_nv_mutex);
	return ret;
}

int power_common_nv_read(enum power_nv_common_type nv_common_type, void *value)
{
	int ret;
	int *bat_heat = NULL;
	struct power_nv_fod_qval *qval = NULL;
	struct power_nv_common_info nv_common_info = { 0 };

	if ((nv_common_type < POWER_NV_COMMON_BEGIN) ||
		(nv_common_type >= POWER_NV_COMMON_END))
		return -EINVAL;

	mutex_lock(&g_power_nv_mutex);
	ret = power_nv_read(POWER_NV_COMMON, &nv_common_info, sizeof(nv_common_info));
	mutex_unlock(&g_power_nv_mutex);
	if (ret)
		return ret;

	switch (nv_common_type) {
	case POWER_NV_COMMON_BATHEAT:
		bat_heat = (int *)value;
		if (bat_heat)
			*bat_heat = (int)nv_common_info.bat_heat;
		break;
	case POWER_NV_COMMON_QVAL:
		qval = (struct power_nv_fod_qval *)value;
		if (qval)
			(void)memcpy_s(qval, sizeof(*qval), &nv_common_info.qval, sizeof(nv_common_info.qval));
		break;
	default:
		break;
	}

	return 0;
}

static int __init power_nv_init(void)
{
	mutex_init(&g_power_nv_mutex);

	return 0;
}

static void __exit power_nv_exit(void)
{
}

subsys_initcall_sync(power_nv_init);
module_exit(power_nv_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("power nv module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
