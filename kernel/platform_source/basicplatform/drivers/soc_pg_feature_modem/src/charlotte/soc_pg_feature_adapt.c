/*
 * parse soc pg feature
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
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

#include <soc_pg_feature_adapt.h>
#include <linux/export.h>
#include <linux/of.h>
#include <linux/printk.h>
#include <linux/string.h>
#include <platform_include/see/efuse_driver.h>
#include <linux/platform_drivers/soc_pg_feature.h>

static const char *get_pg_soc_chip_type(void)
{
	const char *soc_spec = NULL;
	struct device_node *np = of_find_compatible_node(NULL, NULL, "hisilicon, soc_spec");

	if (!np) {
		pr_err("no soc_spec dts node");
		return MODEM_CHIP_TYPE_CS;
	}

	if (of_property_read_string(np, "soc_spec_set", &soc_spec)) {
		pr_err("no soc_spec_set dts info\n");
		of_node_put(np);
		return EFUSE_SOC_ERROR;
	}
	of_node_put(np);
	pr_err("%s: soc_spec_set is %s\n",__func__, soc_spec);

	return soc_spec;
}

static int read_efuse_to_judge(unsigned int *efuse_value)
{
	uint32_t ret;
	struct efuse_item efuse_partialgood6;
	struct efuse_item efuse_partialgood7;
	uint32_t partialgood6_buf[EFUSE_PARTIALGOOD_FLAG6_BUFF_SIZE];
	uint32_t partialgood7_buf[EFUSE_PARTIALGOOD_FLAG7_BUFF_SIZE];

	efuse_partialgood6.buf = partialgood6_buf;
	efuse_partialgood6.buf_size = EFUSE_PARTIALGOOD_FLAG6_BUFF_SIZE;
	efuse_partialgood6.item_vid = EFUSE_KERNEL_PARTIALGOOD_FLAG6;

	efuse_partialgood7.buf = partialgood7_buf;
	efuse_partialgood7.buf_size = EFUSE_PARTIALGOOD_FLAG7_BUFF_SIZE;
	efuse_partialgood7.item_vid = EFUSE_KERNEL_PARTIALGOOD_FLAG7;

	ret = efuse_read_item(&efuse_partialgood6);
	ret |= efuse_read_item(&efuse_partialgood7);
	if (ret) {
	pr_err("efuse_read_item return error! ret = %d\n", ret);
		return ERROR;
	}

	*efuse_value = (efuse_partialgood6.buf[EFUSE_PG_INDEX] & EFUSE_PG_MASK) >> EFUSE_PG_BIT;
	*efuse_value = (*efuse_value << 1) |
		((efuse_partialgood7.buf[EFUSE_MODEM_INDEX] & EFUSE_MODEM_MASK) >> EFUSE_MODEM_BIT);
	*efuse_value = (*efuse_value << 1) |
		((efuse_partialgood7.buf[EFUSE_BBP_INDEX] & EFUSE_BBP_MASK) >> EFUSE_BBP_BIT);
	*efuse_value = (*efuse_value << 1) |
		((efuse_partialgood7.buf[EFUSE_AVS_INDEX] & EFUSE_AVS_MASK) >> EFUSE_AVS_BIT);
	pr_err("%s: efuse_value = %d\n", __func__, *efuse_value);

	return OK;
}

int get_pg_soc_is_modem_need_load(void)
{
	const char *dts_str = NULL;

	dts_str = get_pg_soc_chip_type();
	if (strncmp(dts_str, EFUSE_SOC_ERROR, strlen(EFUSE_SOC_ERROR)) == 0 ||
			is_have_modem(dts_str) == 0) {
		pr_err("%s: not load modem!!\n", __func__);
		return NOT_LOAD_MODEM;
	}

	return NEED_LOAD_MODEM;
}
EXPORT_SYMBOL(get_pg_soc_is_modem_need_load);

/*
 * Description: get_soc_modem_regulator_strategy
 *              The ret value to indicate mode regulator strategy.
 * interface for modem
 * Input : void
 * Return: int soc chip level
 *         no need to do something ---ret 0
 *         enhance voltage or dehance profile --ret 1
 *         avs strategy --ret 2
 *         do not suppost --ret 3
 */
int get_soc_modem_regulator_strategy(void)
{
	unsigned int efuse_value;
	int ret;

	ret = get_pg_soc_is_modem_need_load();
	if (ret == NOT_LOAD_MODEM)
		return NOT_SUPPOST;

	const char *dts_str = NULL;
	dts_str = get_pg_soc_chip_type();
	if (!strncmp(dts_str, "level1", strlen("level1"))) {
		pr_err("%s: soc_chip_type is level1, need nothing!\n", __func__);
		return NEED_NOTHING;
	}

	ret = read_efuse_to_judge(&efuse_value);
	if (ret == ERROR)
		return NOT_SUPPOST;

	if ((efuse_value == LEVEL_1) || (efuse_value == LEVLE_1_MODEM))
		return NEED_NOTHING;
	else if ((efuse_value != 0) && (((efuse_value | NOT_SUPPORT) == NOT_SUPPORT)))
		return NOT_SUPPOST;
	else if ((efuse_value & LEVEL_AVS) == LEVEL_AVS)
		return NEED_AVS_STRATEGY;
	else if (((efuse_value & LEVEL_2) == LEVEL_2) || (efuse_value == 0))
		return NEED_VOLTAGE_FREQ;

	return NEED_NOTHING;
}
EXPORT_SYMBOL(get_soc_modem_regulator_strategy);

u32 get_kernel_chip_version(void)
{
	return NEED_NOTHING;
}
EXPORT_SYMBOL(get_kernel_chip_version);
