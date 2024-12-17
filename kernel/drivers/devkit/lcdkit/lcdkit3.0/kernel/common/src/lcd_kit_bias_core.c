/*
 * lcd_kit_bias_core.c
 *
 * lcdkit bias function for lcd bias ic driver
 *
 * Copyright (c) 2024-2024 Huawei Technologies Co., Ltd.
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

#include <linux/module.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/err.h>
#include <linux/delay.h>
#include <securec.h>

#include "lcd_kit_bias_core.h"
#include "lcd_kit_common.h"
#include "lcd_kit_bias.h"

#define LCD_BIAS_IC_COMP_LEN 128
#define BIAS_IC_CMD_LEN 5
#define DEFAULT_VOL_VAL 0x0F

static struct lcd_kit_bias_device *g_pbias_dev = NULL;
static char g_bias_ic_name[LCD_BIAS_IC_NAME_LEN] = "default";

/* bias ic vol */
static struct bias_ic_voltage common_vol_table[] = {
	{ LCD_BIAS_VOL_400, BIAS_COMMON_VOL_40 },
	{ LCD_BIAS_VOL_410, BIAS_COMMON_VOL_41 },
	{ LCD_BIAS_VOL_420, BIAS_COMMON_VOL_42 },
	{ LCD_BIAS_VOL_430, BIAS_COMMON_VOL_43 },
	{ LCD_BIAS_VOL_440, BIAS_COMMON_VOL_44 },
	{ LCD_BIAS_VOL_450, BIAS_COMMON_VOL_45 },
	{ LCD_BIAS_VOL_460, BIAS_COMMON_VOL_46 },
	{ LCD_BIAS_VOL_470, BIAS_COMMON_VOL_47 },
	{ LCD_BIAS_VOL_480, BIAS_COMMON_VOL_48 },
	{ LCD_BIAS_VOL_490, BIAS_COMMON_VOL_49 },
	{ LCD_BIAS_VOL_500, BIAS_COMMON_VOL_50 },
	{ LCD_BIAS_VOL_510, BIAS_COMMON_VOL_51 },
	{ LCD_BIAS_VOL_520, BIAS_COMMON_VOL_52 },
	{ LCD_BIAS_VOL_530, BIAS_COMMON_VOL_53 },
	{ LCD_BIAS_VOL_540, BIAS_COMMON_VOL_54 },
	{ LCD_BIAS_VOL_550, BIAS_COMMON_VOL_55 },
	{ LCD_BIAS_VOL_560, BIAS_COMMON_VOL_56 },
	{ LCD_BIAS_VOL_570, BIAS_COMMON_VOL_57 },
	{ LCD_BIAS_VOL_580, BIAS_COMMON_VOL_58 },
	{ LCD_BIAS_VOL_590, BIAS_COMMON_VOL_59 },
	{ LCD_BIAS_VOL_600, BIAS_COMMON_VOL_60 },
};

static struct bias_ic_config g_bias_ic_config[] = {
	{"hw_aw37503", ARRAY_SIZE(common_vol_table), &common_vol_table[0] },
	{"hw_nex10001", ARRAY_SIZE(common_vol_table), &common_vol_table[0] },
	{"hw_jw1360", ARRAY_SIZE(common_vol_table), &common_vol_table[0] }
};

static void lcd_kit_get_bias_config(int vsp, int vsn,
	unsigned char *outvsp, unsigned char *outvsn)
{
	int i;
	int j;

	if ((outvsp == NULL) || (outvsn == NULL)) {
		LCD_KIT_ERR("input parameter is NULL\n");
		return;
	}

	for (i = 0; i < ARRAY_SIZE(g_bias_ic_config); i++) {
		if (strcmp(g_bias_ic_name, g_bias_ic_config[i].name))
			continue;
		for (j = 0; j < g_bias_ic_config[i].len; j++) {
			if (g_bias_ic_config[i].vol_table[j].voltage == vsp) {
				*outvsp = g_bias_ic_config[i].vol_table[j].value;
				break;
			}
		}
		if (j == g_bias_ic_config[i].len) {
			LCD_KIT_INFO("not found vsp voltage, use default\n");
			*outvsp = DEFAULT_VOL_VAL;
		}

		for (j = 0; j < g_bias_ic_config[i].len; j++) {
			if (g_bias_ic_config[i].vol_table[j].voltage == vsn) {
				*outvsn  = g_bias_ic_config[i].vol_table[j].value;
				break;
			}
		}

		if (j == g_bias_ic_config[i].len) {
			LCD_KIT_INFO("not found neg voltage, use default voltage\n");
			*outvsn = DEFAULT_VOL_VAL;
		}
		break;
	}

	if (i == ARRAY_SIZE(g_bias_ic_config)) {
		LCD_KIT_INFO("not found right voltage, use default voltage\n");
		*outvsp = DEFAULT_VOL_VAL;
		*outvsn = DEFAULT_VOL_VAL;
	}
}

static int lcd_kit_bias_read_byte(struct i2c_client *client,
	unsigned char reg, unsigned char *pdata)
{
	int ret;

	ret = i2c_smbus_read_byte_data(client, reg);
	if (ret < 0) {
		dev_err(&client->dev, "failed to read 0x%x\n", reg);
		return ret;
	}
	*pdata = (unsigned char)ret;

	return ret;
}

static int lcd_kit_bias_write_byte(struct i2c_client *client,
	unsigned char reg, unsigned char data)
{
	int ret;

	ret = i2c_smbus_write_byte_data(client, reg, data);
	if (ret < 0)
		dev_err(&client->dev, "failed to write 0x%.2x\n", reg);
	else
		LCD_KIT_INFO("register 0x%x value = 0x%x\n", reg, data);
	return ret;
}

static int lcd_kit_bias_update_bit(struct i2c_client *client,
	unsigned char reg, unsigned char mask, unsigned char *pdata)
{
	int ret;
	unsigned char tmp = 0;

	ret = lcd_kit_bias_read_byte(client, reg, &tmp);
	if (ret < 0) {
		dev_err(&client->dev, "failed to read 0x%.2x\n", reg);
		return ret;
	}

	tmp = (unsigned char)ret;
	tmp |= (*pdata);
	tmp &= ~mask;

	*pdata = tmp;
	return ret;
}

static void lcd_kit_bias_propname_cat(char *pdest, const char *psrc,
	int dest_len)
{
	if ((pdest == NULL) || (psrc == NULL) ||
		(dest_len == 0)) {
		LCD_KIT_ERR("input invalid\n");
		return;
	}
	if (memset_s(pdest, dest_len, 0, dest_len) != EOK) {
		LCD_KIT_ERR("memset_s fail\n");
		return;
	}
	if (snprintf_s(pdest, dest_len, dest_len - 1, "%s,%s", g_bias_ic_name, psrc) < 0) {
		LCD_KIT_ERR("snprintf_s fail\n");
		return;
	}
}

void lcd_kit_parse_bias_param(struct device_node *pnp,
	const char *node_str, unsigned int *pval)
{
	char tmp_buf[LCD_BIAS_IC_COMP_LEN] = {0};
	unsigned int tmp_val;
	int ret = 0;

	if ((node_str == NULL) || (pval == NULL) || (pnp == NULL)) {
		LCD_KIT_ERR("input invalid\n");
		return;
	}

	lcd_kit_bias_propname_cat(tmp_buf, node_str, sizeof(tmp_buf));
	ret = of_property_read_u32(pnp, tmp_buf, &tmp_val);
	*pval = (!ret ? tmp_val : 0);
}

static int lcd_kit_parse_bias_cmd(struct device_node *np,
	const char *propertyname, struct bias_ic_cmd *pcmd)
{
	char tmp_buf[LCD_BIAS_IC_COMP_LEN] = {0};
	struct property *prop = NULL;
	unsigned int *buf = NULL;
	int ret = 0;

	if ((np == NULL) || (propertyname == NULL) || (pcmd == NULL)) {
		LCD_KIT_ERR("input parameter is NULL\n");
		return LCD_KIT_FAIL;
	}
	lcd_kit_bias_propname_cat(tmp_buf, propertyname, sizeof(tmp_buf));
	if (memset_s(pcmd, sizeof(struct bias_ic_cmd), 0, sizeof(struct bias_ic_cmd)) != EOK) {
		LCD_KIT_ERR("memset_s fail\n");
		return LCD_KIT_FAIL;
	}
	prop = of_find_property(np, tmp_buf, NULL);
	if (prop == NULL) {
		LCD_KIT_ERR("%s is not config\n", tmp_buf);
		return LCD_KIT_FAIL;
	}
	if (prop->length != BIAS_IC_CMD_LEN * sizeof(unsigned int)) {
		LCD_KIT_ERR("%s number is not right\n", tmp_buf);
		return LCD_KIT_FAIL;
	}
	buf = kzalloc(prop->length, GFP_KERNEL);
	if (buf == NULL) {
		LCD_KIT_ERR("malloc fail\n");
		return LCD_KIT_FAIL;
	}
	ret = of_property_read_u32_array(np, tmp_buf, buf, BIAS_IC_CMD_LEN);
	if (ret < 0) {
		LCD_KIT_ERR("get %s config fail\n", tmp_buf);
		kfree(buf);
		buf = NULL;
		return LCD_KIT_FAIL;
	}
	/* 0:ops_type 1:cmd_reg  2:cmd_val  3:cmd_mask 4:delay */
	pcmd->ops_type = buf[0];
	pcmd->cmd_reg = buf[1];
	pcmd->cmd_val = buf[2];
	pcmd->cmd_mask = buf[3];
	pcmd->delay = buf[4];
	LCD_KIT_INFO("%s: type is 0x%x  reg is 0x%x  val is 0x%x mask is 0x%x\n",
		propertyname, pcmd->ops_type, pcmd->cmd_reg,
		pcmd->cmd_val, pcmd->cmd_mask);
	kfree(buf);
	buf = NULL;

	return LCD_KIT_OK;
}

static bool lcd_kit_bias_device_verify(struct lcd_kit_bias_info *pbias_config)
{
	int ret;
	unsigned char val;
	ret = lcd_kit_bias_read_byte(g_pbias_dev->client, pbias_config->verify_cmds.cmd_reg, &val);
	if (ret < 0 || ((val & pbias_config->verify_cmds.cmd_mask) != pbias_config->verify_cmds.cmd_val)) {
		LCD_KIT_ERR("verify failed\n");
		return false;
	}
	LCD_KIT_INFO("%s verify ok, verify_val = 0x%x\n", g_bias_ic_name, ret);
	return true;
}

static void lcd_kit_bias_parse_init_cmds_config(struct device_node *np,
	struct lcd_kit_bias_info *pbias_config)
{
	int ret;
	struct property *prop = NULL;
	int i;
	unsigned int *buf = NULL;
	char tmp_buf[LCD_BIAS_IC_COMP_LEN] = {0};

	if ((pbias_config == NULL) || (np == NULL)) {
		LCD_KIT_ERR("input parameter is NULL\n");
		return;
	}

	lcd_kit_parse_bias_param(np, "bias_ic_num_of_kernel_init_cmd", &pbias_config->num_of_init_cmd);
	lcd_kit_bias_propname_cat(tmp_buf, "bias_ic_kernel_init_cmd", sizeof(tmp_buf));
	prop = of_find_property(np, tmp_buf, NULL);
	if (prop == NULL) {
		LCD_KIT_ERR("kernel init cmds is not config\n");
		return;
	}

	if (prop->length != (pbias_config->num_of_init_cmd * BIAS_IC_CMD_LEN * sizeof(unsigned int))) {
		LCD_KIT_ERR("init cmds number is not right\n");
		return;
	}
	buf = kzalloc(prop->length, GFP_KERNEL);
	if (buf == NULL) {
		LCD_KIT_ERR("malloc fail\n");
		return;
	}
	lcd_kit_bias_propname_cat(tmp_buf, "bias_ic_kernel_init_cmd", sizeof(tmp_buf));
	ret = of_property_read_u32_array(np, tmp_buf, buf, prop->length / sizeof(unsigned int));
	if (ret < 0) {
		LCD_KIT_ERR("get kernel init cmds config fail\n");
		kfree(buf);
		buf = NULL;
		return;
	}
	for (i = 0; i < pbias_config->num_of_init_cmd; i++) {
		pbias_config->init_cmds[i].ops_type = buf[i * BIAS_IC_CMD_LEN];
		pbias_config->init_cmds[i].cmd_reg = buf[i * BIAS_IC_CMD_LEN + 1];
		pbias_config->init_cmds[i].cmd_val = buf[i * BIAS_IC_CMD_LEN + 2]; // bits
		pbias_config->init_cmds[i].cmd_mask = buf[i * BIAS_IC_CMD_LEN + 3];
		pbias_config->init_cmds[i].delay = buf[i * BIAS_IC_CMD_LEN + 4];
		LCD_KIT_INFO("init i is %d, tpye is 0x%x reg is 0x%x val is 0x%x mask is 0x%x\n",
			i, pbias_config->init_cmds[i].ops_type, pbias_config->init_cmds[i].cmd_reg,
			pbias_config->init_cmds[i].cmd_val, pbias_config->init_cmds[i].cmd_mask);
	}
	kfree(buf);
	buf = NULL;
}

static int lcd_kit_bias_get_config(struct device_node *np, struct lcd_kit_bias_info *pbias_config)
{
	int ret;

	if ((pbias_config == NULL) || (np == NULL)) {
		LCD_KIT_ERR("input parameter is NULL\n");
		return -ENODEV;
	}

	ret = lcd_kit_parse_bias_cmd(np, "bias_ic_verify_cmd", &pbias_config->verify_cmds);
	if (ret < 0) {
		LCD_KIT_ERR("parse bias_ic_verify_cmd fail\n");
		return -ENODEV;
	}

	lcd_kit_bias_parse_init_cmds_config(np, pbias_config);

	ret = lcd_kit_parse_bias_cmd(np, "bias_ic_vsp_cmd", &pbias_config->vsp_cmd);
	if (ret < 0) {
		LCD_KIT_ERR("parse vsp_cmd fail\n");
		return -ENODEV;
	}

	ret = lcd_kit_parse_bias_cmd(np, "bias_ic_vsn_cmd", &pbias_config->vsn_cmd);
	if (ret < 0) {
		LCD_KIT_ERR("parse vsn_cmd fail\n");
		return -ENODEV;
	}

	if (!lcd_kit_bias_device_verify(pbias_config)) {
		LCD_KIT_ERR("bias_device_verify failed\n");
		return -ENODEV;
	}

	return ret;
}

static void lcd_kit_bias_reg_init(void)
{
	int i;
	struct bias_ic_cmd *init = NULL;

	LCD_KIT_INFO("bias ic reg init\n");
	if (g_pbias_dev == NULL) {
		LCD_KIT_ERR(" bias device is NULL\n");
		return;
	}

	init = g_pbias_dev->config.init_cmds;

	for (i = 0; i < g_pbias_dev->config.num_of_init_cmd; i++) {
		if (init[i].ops_type == REG_READ_MODE) {
			lcd_kit_bias_read_byte(g_pbias_dev->client, init[i].cmd_reg, &init[i].cmd_val);
		} else if (init[i].ops_type == REG_UPDATE_MODE) {
			lcd_kit_bias_update_bit(g_pbias_dev->client, init[i].cmd_reg,
				init[i].cmd_mask, &init[i].cmd_val);
		}
	}

	for (i = 0; i < g_pbias_dev->config.num_of_init_cmd; i++) {
		if (init[i].ops_type == REG_WRITE_MODE || init[i].ops_type == REG_UPDATE_MODE) {
			lcd_kit_bias_write_byte(g_pbias_dev->client, init[i].cmd_reg, init[i].cmd_val);

			if (init[i].delay)
				mdelay(init[i].delay);
		}
	}
}

static int lcd_kit_set_bias_power_down(int vpos, int vneg)
{
	unsigned char vsp_vol = 0;
	unsigned char vsn_vol = 0;
	int ret;

	lcd_kit_get_bias_config(vpos, vneg, &vsp_vol, &vsn_vol);
	ret = lcd_kit_bias_write_byte(g_pbias_dev->client,
		g_pbias_dev->config.vsp_cmd.cmd_reg, vsp_vol);
	if (ret < 0) {
		LCD_KIT_ERR("write vsp failed\n");
		return ret;
	}

	ret = lcd_kit_bias_write_byte(g_pbias_dev->client,
		g_pbias_dev->config.vsn_cmd.cmd_reg, vsn_vol);
	if (ret < 0) {
		LCD_KIT_ERR("write vsn failed\n");
		return ret;
	}

	return ret;
}

static int lcd_kit_set_bias(int vpos, int vneg)
{
	int ret;
	unsigned char vsp_vol = 0;
	unsigned char vsn_vol = 0;

	lcd_kit_get_bias_config(vpos, vneg, &vsp_vol, &vsn_vol);
	lcd_kit_bias_reg_init();

	ret = lcd_kit_bias_write_byte(g_pbias_dev->client,
		g_pbias_dev->config.vsp_cmd.cmd_reg, vsp_vol);
	if (ret < 0) {
		LCD_KIT_ERR("write vsp failed\n");
		return ret;
	}

	ret = lcd_kit_bias_write_byte(g_pbias_dev->client,
		g_pbias_dev->config.vsn_cmd.cmd_reg, vsn_vol);
	if (ret < 0) {
		LCD_KIT_ERR("write vsn failed\n");
		return ret;
	}

	return ret;
}

static struct lcd_kit_bias_ops bias_ops = {
	.set_bias_voltage = lcd_kit_set_bias,
	.set_bias_power_down = lcd_kit_set_bias_power_down,
};

static int lcd_kit_bias_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int ret = 0;

	LCD_KIT_INFO("enter\n");
	if (client == NULL || client->adapter == NULL) {
		LCD_KIT_ERR("client is null or adapter is NULL pointer\n");
		return -EINVAL;
	}
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		dev_err(&client->dev, "i2c functionality check fail\n");
		return -EOPNOTSUPP;
	}
	g_pbias_dev = devm_kzalloc(&client->dev, sizeof(struct lcd_kit_bias_device), GFP_KERNEL);
	if (g_pbias_dev == NULL) {
		LCD_KIT_ERR("Failed to allocate memory\n");
		return -ENOMEM;
	}
	i2c_set_clientdata(client, g_pbias_dev);
	g_pbias_dev->dev = &client->dev;
	g_pbias_dev->client = client;
	if (client->dev.of_node == NULL) {
		LCD_KIT_ERR("of_node is NULL\n");
		ret = -EINVAL;
		goto failed;
	}

	ret = lcd_kit_bias_get_config(client->dev.of_node, &g_pbias_dev->config);
	if (ret < 0) {
		LCD_KIT_ERR("%s_get_config failed\n", g_bias_ic_name);
		ret = -EINVAL;
		goto failed;
	}
	LCD_KIT_INFO("%s inited succeed\n", g_bias_ic_name);

	lcd_kit_bias_register(&bias_ops);
	LCD_KIT_INFO("exit\n");
	return ret;

failed:
	devm_kfree(&client->dev, g_pbias_dev);
	g_pbias_dev = NULL;
	return ret;
}

static struct i2c_device_id bias_ic_id[] = {
	{ "bias_ic_common", 0 },
	{},
};

MODULE_DEVICE_TABLE(i2c, bias_ic_id);

static struct of_device_id lcd_kit_bias_match_table[] = {
	{ .compatible = "bias_ic_common", },
	{},
};

static struct i2c_driver lcd_kit_bias_driver = {
	.probe = lcd_kit_bias_probe,
	.driver = {
		.name = "bias_ic_common",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(lcd_kit_bias_match_table),
	},
	.id_table = bias_ic_id,
};

static void lcd_kit_get_bias_ic_name(void)
{
	struct device_node *np = NULL;
	char *bias_type = NULL;
	int len;

	np = of_find_compatible_node(NULL, NULL, "huawei,lcd_panel_type");
	if (!np) {
		LCD_KIT_ERR("NOT FOUND device node %s!\n", "huawei,lcd_panel_type");
		return;
	}

	bias_type = (char *)of_get_property(np, "lcd_bias_type", NULL);
	if (bias_type) {
		len = strlen(bias_type);
		if (len > LCD_BIAS_IC_NAME_LEN - 1)
			len = LCD_BIAS_IC_NAME_LEN - 1;
		if (strncpy_s(g_bias_ic_name, LCD_BIAS_IC_NAME_LEN - 1, bias_type, len) != EOK) {
			LCD_KIT_ERR("strncpy_s fail\n");
			return;
		}
		g_bias_ic_name[LCD_BIAS_IC_NAME_LEN - 1] = 0;
	}
}

static int __init lcd_kit_bias_init(void)
{
	char compatible_name[LCD_BIAS_IC_COMP_LEN] = {0};
	int len;

	lcd_kit_get_bias_ic_name();
	LCD_KIT_INFO("g_bias_ic_name is %s\n", g_bias_ic_name);
	if (!strcmp(g_bias_ic_name, "default"))
		return LCD_KIT_OK;

	if (memcpy_s(bias_ic_id[0].name, LCD_BIAS_IC_NAME_LEN, g_bias_ic_name,
		LCD_BIAS_IC_NAME_LEN) != EOK) {
		LCD_KIT_ERR("memcpy_s fail\n");
		return LCD_KIT_FAIL;
	}
	lcd_kit_bias_driver.driver.name = g_bias_ic_name;
	if (snprintf_s(compatible_name, sizeof(compatible_name), sizeof(compatible_name) - 1,
		"huawei,%s", g_bias_ic_name) < 0) {
		LCD_KIT_ERR("snprintf_s fail\n");
		return LCD_KIT_FAIL;
	}
	len = strlen(compatible_name);
	if (len >= LCD_BIAS_IC_COMP_LEN)
		len = LCD_BIAS_IC_COMP_LEN - 1;
	if (memcpy_s(lcd_kit_bias_match_table[0].compatible, LCD_BIAS_IC_COMP_LEN - 1,
		compatible_name, len) != EOK) {
		LCD_KIT_ERR("memcpy_s fail\n");
		return LCD_KIT_FAIL;
	}
	lcd_kit_bias_match_table[0].compatible[LCD_BIAS_IC_COMP_LEN - 1] = 0;

	return i2c_add_driver(&lcd_kit_bias_driver);
}

static void __exit lcd_kit_bias_exit(void)
{
	i2c_del_driver(&lcd_kit_bias_driver);
}

late_initcall(lcd_kit_bias_init);
module_exit(lcd_kit_bias_exit);

MODULE_AUTHOR("Huawei Technologies Co., Ltd");
MODULE_LICENSE("GPL");

 