/*
 * lcd_kit_backlight_core.c
 *
 * lcdkit backlgiht function for lcd backlight ic driver
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
#include <linux/uaccess.h>
#include <linux/semaphore.h>

#include "lcd_kit_backlight_core.h"
#include "lcd_kit_backlight_linear_to_exp.h"
#include "lcd_kit_common.h"
#include "lcd_kit_bl.h"
#include <securec.h>
#if defined(CONFIG_HUAWEI_DSM)
#include <dsm/dsm_pub.h>
extern struct dsm_client *lcd_dclient;
#endif

#define LCD_BACKLIGHT_POWER_ON 0x0
#define LCD_BACKLIGHT_POWER_OFF 0x1
#define LCD_BACKLIGHT_LEVEL_POWER_ON 0x2
#define LCD_BACKLIGHT_DIMMING_SUPPORT 1
#define LCD_BACKLIGHT_BOOST_OFFSET 6

#define LCD_BACKLIGHT_BASE 10
#define LCD_BACKLIGHT_IC_COMP_LEN 128
#define LCD_PANEL_IC_NAME_LEN 64
#define BACKLIGHT_IC_CMD_LEN 5
#define BACKLIGHT_BL_CMD_LEN 4
#define DEFAULT_VOL_VAL 0x0F
#define BACKLIGHT_TRUE 1
#define BACKLIGHT_FALSE 0
#define BACKLIGHT_GPIO_DIR_OUT 1
#define BACKLIGHT_GPIO_OUT_ONE 1
#define BACKLIGHT_GPIO_OUT_ZERO 0

static struct lcd_kit_backlight_device *g_pbacklight_dev = NULL;
static char g_backlight_ic_name[LCD_BACKLIGHT_IC_NAME_LEN] = "default";
static char g_panel_ic_name[LCD_PANEL_IC_NAME_LEN];

#ifdef LCD_KIT_DEBUG_ENABLE
struct class *g_backlight_class = NULL;
static unsigned int g_sys_reg = 0x00;
static int g_sysfs_created;
#endif

static bool g_vol_mapped = false;
static bool g_backlight_init_status = false;
static struct i2c_adapter *g_backlight_dual_ic_adap = NULL;
static struct backlight_bias_ic_voltage common_vol_table[] = {
	{ LCD_BACKLIGHT_VOL_400, COMMON_VOL_400 },
	{ LCD_BACKLIGHT_VOL_405, COMMON_VOL_405 },
	{ LCD_BACKLIGHT_VOL_410, COMMON_VOL_410 },
	{ LCD_BACKLIGHT_VOL_415, COMMON_VOL_415 },
	{ LCD_BACKLIGHT_VOL_420, COMMON_VOL_420 },
	{ LCD_BACKLIGHT_VOL_425, COMMON_VOL_425 },
	{ LCD_BACKLIGHT_VOL_430, COMMON_VOL_430 },
	{ LCD_BACKLIGHT_VOL_430, COMMON_VOL_435 },
	{ LCD_BACKLIGHT_VOL_440, COMMON_VOL_440 },
	{ LCD_BACKLIGHT_VOL_445, COMMON_VOL_445 },
	{ LCD_BACKLIGHT_VOL_450, COMMON_VOL_450 },
	{ LCD_BACKLIGHT_VOL_455, COMMON_VOL_455 },
	{ LCD_BACKLIGHT_VOL_460, COMMON_VOL_460 },
	{ LCD_BACKLIGHT_VOL_465, COMMON_VOL_465 },
	{ LCD_BACKLIGHT_VOL_470, COMMON_VOL_470 },
	{ LCD_BACKLIGHT_VOL_475, COMMON_VOL_475 },
	{ LCD_BACKLIGHT_VOL_480, COMMON_VOL_480 },
	{ LCD_BACKLIGHT_VOL_485, COMMON_VOL_485 },
	{ LCD_BACKLIGHT_VOL_490, COMMON_VOL_490 },
	{ LCD_BACKLIGHT_VOL_495, COMMON_VOL_495 },
	{ LCD_BACKLIGHT_VOL_500, COMMON_VOL_500 },
	{ LCD_BACKLIGHT_VOL_505, COMMON_VOL_505 },
	{ LCD_BACKLIGHT_VOL_510, COMMON_VOL_510 },
	{ LCD_BACKLIGHT_VOL_515, COMMON_VOL_515 },
	{ LCD_BACKLIGHT_VOL_520, COMMON_VOL_520 },
	{ LCD_BACKLIGHT_VOL_525, COMMON_VOL_525 },
	{ LCD_BACKLIGHT_VOL_530, COMMON_VOL_530 },
	{ LCD_BACKLIGHT_VOL_535, COMMON_VOL_535 },
	{ LCD_BACKLIGHT_VOL_540, COMMON_VOL_540 },
	{ LCD_BACKLIGHT_VOL_545, COMMON_VOL_545 },
	{ LCD_BACKLIGHT_VOL_550, COMMON_VOL_550 },
	{ LCD_BACKLIGHT_VOL_555, COMMON_VOL_555 },
	{ LCD_BACKLIGHT_VOL_560, COMMON_VOL_560 },
	{ LCD_BACKLIGHT_VOL_565, COMMON_VOL_565 },
	{ LCD_BACKLIGHT_VOL_570, COMMON_VOL_570 },
	{ LCD_BACKLIGHT_VOL_575, COMMON_VOL_575 },
	{ LCD_BACKLIGHT_VOL_580, COMMON_VOL_580 },
	{ LCD_BACKLIGHT_VOL_585, COMMON_VOL_585 },
	{ LCD_BACKLIGHT_VOL_590, COMMON_VOL_590 },
	{ LCD_BACKLIGHT_VOL_600, COMMON_VOL_600 },
	{ LCD_BACKLIGHT_VOL_605, COMMON_VOL_605 },
	{ LCD_BACKLIGHT_VOL_610, COMMON_VOL_610 },
	{ LCD_BACKLIGHT_VOL_615, COMMON_VOL_615 },
	{ LCD_BACKLIGHT_VOL_620, COMMON_VOL_620 },
	{ LCD_BACKLIGHT_VOL_625, COMMON_VOL_625 },
	{ LCD_BACKLIGHT_VOL_630, COMMON_VOL_630 },
	{ LCD_BACKLIGHT_VOL_635, COMMON_VOL_635 },
	{ LCD_BACKLIGHT_VOL_640, COMMON_VOL_640 },
	{ LCD_BACKLIGHT_VOL_645, COMMON_VOL_645 },
	{ LCD_BACKLIGHT_VOL_650, COMMON_VOL_650 },
	{ LCD_BACKLIGHT_VOL_655, COMMON_VOL_655 },
	{ LCD_BACKLIGHT_VOL_660, COMMON_VOL_660 },
	{ LCD_BACKLIGHT_VOL_665, COMMON_VOL_665 },
	{ LCD_BACKLIGHT_VOL_670, COMMON_VOL_670 },
	{ LCD_BACKLIGHT_VOL_675, COMMON_VOL_675 },
	{ LCD_BACKLIGHT_VOL_680, COMMON_VOL_680 },
	{ LCD_BACKLIGHT_VOL_685, COMMON_VOL_685 },
	{ LCD_BACKLIGHT_VOL_690, COMMON_VOL_690 },
	{ LCD_BACKLIGHT_VOL_695, COMMON_VOL_695 },
	{ LCD_BACKLIGHT_VOL_700, COMMON_VOL_700 },
	{ LCD_BACKLIGHT_VOL_705, COMMON_VOL_705 },
	{ LCD_BACKLIGHT_VOL_710, COMMON_VOL_710 },
	{ LCD_BACKLIGHT_VOL_715, COMMON_VOL_715 }
};

/* backlight bias ic: ktz8864 */
static struct backlight_bias_ic_voltage ktz8864_vol_table[] = {
	{ LCD_BACKLIGHT_VOL_400, KTZ8864_VOL_400 },
	{ LCD_BACKLIGHT_VOL_405, KTZ8864_VOL_405 },
	{ LCD_BACKLIGHT_VOL_410, KTZ8864_VOL_410 },
	{ LCD_BACKLIGHT_VOL_415, KTZ8864_VOL_415 },
	{ LCD_BACKLIGHT_VOL_420, KTZ8864_VOL_420 },
	{ LCD_BACKLIGHT_VOL_425, KTZ8864_VOL_425 },
	{ LCD_BACKLIGHT_VOL_430, KTZ8864_VOL_430 },
	{ LCD_BACKLIGHT_VOL_430, KTZ8864_VOL_435 },
	{ LCD_BACKLIGHT_VOL_440, KTZ8864_VOL_440 },
	{ LCD_BACKLIGHT_VOL_445, KTZ8864_VOL_445 },
	{ LCD_BACKLIGHT_VOL_450, KTZ8864_VOL_450 },
	{ LCD_BACKLIGHT_VOL_455, KTZ8864_VOL_455 },
	{ LCD_BACKLIGHT_VOL_460, KTZ8864_VOL_460 },
	{ LCD_BACKLIGHT_VOL_465, KTZ8864_VOL_465 },
	{ LCD_BACKLIGHT_VOL_470, KTZ8864_VOL_470 },
	{ LCD_BACKLIGHT_VOL_475, KTZ8864_VOL_475 },
	{ LCD_BACKLIGHT_VOL_480, KTZ8864_VOL_480 },
	{ LCD_BACKLIGHT_VOL_485, KTZ8864_VOL_485 },
	{ LCD_BACKLIGHT_VOL_490, KTZ8864_VOL_490 },
	{ LCD_BACKLIGHT_VOL_495, KTZ8864_VOL_495 },
	{ LCD_BACKLIGHT_VOL_500, KTZ8864_VOL_500 },
	{ LCD_BACKLIGHT_VOL_505, KTZ8864_VOL_505 },
	{ LCD_BACKLIGHT_VOL_510, KTZ8864_VOL_510 },
	{ LCD_BACKLIGHT_VOL_515, KTZ8864_VOL_515 },
	{ LCD_BACKLIGHT_VOL_520, KTZ8864_VOL_520 },
	{ LCD_BACKLIGHT_VOL_525, KTZ8864_VOL_525 },
	{ LCD_BACKLIGHT_VOL_530, KTZ8864_VOL_530 },
	{ LCD_BACKLIGHT_VOL_535, KTZ8864_VOL_535 },
	{ LCD_BACKLIGHT_VOL_540, KTZ8864_VOL_540 },
	{ LCD_BACKLIGHT_VOL_545, KTZ8864_VOL_545 },
	{ LCD_BACKLIGHT_VOL_550, KTZ8864_VOL_550 },
	{ LCD_BACKLIGHT_VOL_555, KTZ8864_VOL_555 },
	{ LCD_BACKLIGHT_VOL_560, KTZ8864_VOL_560 },
	{ LCD_BACKLIGHT_VOL_565, KTZ8864_VOL_565 },
	{ LCD_BACKLIGHT_VOL_570, KTZ8864_VOL_570 },
	{ LCD_BACKLIGHT_VOL_575, KTZ8864_VOL_575 },
	{ LCD_BACKLIGHT_VOL_580, KTZ8864_VOL_580 },
	{ LCD_BACKLIGHT_VOL_585, KTZ8864_VOL_585 },
	{ LCD_BACKLIGHT_VOL_590, KTZ8864_VOL_590 },
	{ LCD_BACKLIGHT_VOL_600, KTZ8864_VOL_600 },
	{ LCD_BACKLIGHT_VOL_605, KTZ8864_VOL_605 },
	{ LCD_BACKLIGHT_VOL_610, KTZ8864_VOL_610 },
	{ LCD_BACKLIGHT_VOL_615, KTZ8864_VOL_615 },
	{ LCD_BACKLIGHT_VOL_620, KTZ8864_VOL_620 },
	{ LCD_BACKLIGHT_VOL_625, KTZ8864_VOL_625 },
	{ LCD_BACKLIGHT_VOL_630, KTZ8864_VOL_630 },
	{ LCD_BACKLIGHT_VOL_635, KTZ8864_VOL_635 },
	{ LCD_BACKLIGHT_VOL_640, KTZ8864_VOL_640 },
	{ LCD_BACKLIGHT_VOL_645, KTZ8864_VOL_645 },
	{ LCD_BACKLIGHT_VOL_650, KTZ8864_VOL_650 },
	{ LCD_BACKLIGHT_VOL_655, KTZ8864_VOL_655 },
	{ LCD_BACKLIGHT_VOL_660, KTZ8864_VOL_660 }
};

static struct backlight_bias_ic_config g_backlight_bias_config[] = {
	{ "hw_lm36274", ARRAY_SIZE(common_vol_table), &common_vol_table[0] },
	{ "hw_nt50356", ARRAY_SIZE(common_vol_table), &common_vol_table[0] },
	{ "hw_rt4831", ARRAY_SIZE(common_vol_table), &common_vol_table[0] },
	{ "hw_ktz8864", ARRAY_SIZE(ktz8864_vol_table), &ktz8864_vol_table[0] }
};

static int lcd_kit_backlight_read_byte(struct i2c_client *client,
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

static int lcd_kit_backlight_write_byte(struct i2c_client *client,
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

static int lcd_kit_backlight_update_bit(struct i2c_client *client,
	unsigned char reg, unsigned char mask, unsigned char data)
{
	int ret;
	unsigned char tmp = 0;

	ret = lcd_kit_backlight_read_byte(client, reg, &tmp);
	if (ret < 0) {
		dev_err(&client->dev, "failed to read 0x%.2x\n", reg);
		return ret;
	}

	tmp = (unsigned char)ret;
	tmp &= (unsigned char)(~mask);
	tmp |= data & mask;

	return lcd_kit_backlight_write_byte(client, reg, tmp);
}

#ifdef LCD_KIT_DEBUG_ENABLE
static ssize_t backlight_reg_addr_store(struct device *dev, struct device_attribute *attr,
	const char *buf, size_t size)
{
	unsigned int reg_addr = 0;
	int ret;

	if ((dev == NULL) || (attr == NULL) || (buf == NULL)) {
		LCD_KIT_ERR("input invalid\n");
		return size;
	}
	ret = sscanf_s(buf, "reg=0x%x", &reg_addr);
	if (ret < 0) {
		LCD_KIT_ERR("check input fail\n");
		return -EINVAL;
	}
	g_sys_reg = reg_addr;
	return size;
}

static DEVICE_ATTR(reg_addr, S_IRUGO|S_IWUSR, NULL, backlight_reg_addr_store);

static ssize_t backlight_reg_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct lcd_kit_backlight_device *pchip = NULL;
	int ret = 0;
	unsigned char val = 0;

	if ((dev == NULL) || (attr == NULL) || (buf == NULL)) {
		ret = snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "input invalid\n");
		return ret;
	}

	pchip = dev_get_drvdata(dev);
	if (pchip == NULL) {
		ret = snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "driver data is null\n");
		return ret;
	}

	ret = lcd_kit_backlight_read_byte(pchip->client, g_sys_reg, &val);
	if (ret < 0)
		goto i2c_error;

	ret = snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "value = 0x%x\n", val);
	return ret;

i2c_error:
	ret = snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "backlight i2c read register error\n");
	return ret;
}

static ssize_t backlight_reg_store(struct device *dev, struct device_attribute *attr,
	const char *buf, size_t size)
{
	struct lcd_kit_backlight_device *pchip = NULL;
	unsigned char reg = 0;
	unsigned char mask = 0;
	unsigned char val = 0;
	int ret = 0;

	if ((dev == NULL) || (attr == NULL) || (buf == NULL)) {
		LCD_KIT_ERR("input invalid\n");
		return size;
	}
	ret = sscanf_s(buf, "reg=0x%x, mask=0x%x, val=0x%x", &reg, &mask, &val);
	if (ret < 0) {
		LCD_KIT_ERR("check input\n");
		return -EINVAL;
	}
	pchip = dev_get_drvdata(dev);
	if (pchip == NULL)
		return -EINVAL;

	ret = lcd_kit_backlight_update_bit(pchip->client, reg, mask, val);
	if (ret < 0) {
		LCD_KIT_ERR("backlight i2c update register error\n");
		return ret;
	}
	return size;
}

static DEVICE_ATTR(reg, S_IRUGO|S_IWUSR, backlight_reg_show, backlight_reg_store);

static ssize_t backlight_bl_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct lcd_kit_backlight_device *pchip = NULL;
	int ret;
	unsigned char bl_val;
	unsigned char bl_lsb;
	unsigned char bl_msb;

	if ((dev == NULL) || (attr == NULL) || (buf == NULL)) {
		ret = snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "input invalid\n");
		return ret;
	}

	pchip = dev_get_drvdata(dev);
	if (pchip == NULL) {
		ret = snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "driver data is null\n");
		return ret;
	}

	ret = lcd_kit_backlight_read_byte(pchip->client,
		pchip->bl_config.bl_lsb_reg_cmd.cmd_reg, &bl_lsb);
	if (ret < 0) {
		ret = snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "backlight i2c read lsb register error\n");
		return ret;
	}

	ret = lcd_kit_backlight_read_byte(pchip->client,
			pchip->bl_config.bl_msb_reg_cmd.cmd_reg, &bl_msb);
	if (ret < 0) {
		ret = snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "backlight i2c read msb register error\n");
		return ret;
	}

	bl_val = (bl_msb << pchip->bl_config.bl_lsb_reg_cmd.val_bits) | bl_lsb;
	ret = snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "bl = 0x%x\n", bl_val);
	return ret;
}

static ssize_t backlight_bl_store(struct device *dev, struct device_attribute *attr,
	const char *buf, size_t size)
{
	struct lcd_kit_backlight_device *pchip = NULL;
	unsigned char bl_val = 0;
	unsigned char bl_msb = 0;
	unsigned char bl_lsb = 0;
	unsigned int val = 0;
	int ret = 0;

	if ((dev == NULL) || (attr == NULL) || (buf == NULL)) {
		LCD_KIT_ERR("input invalid\n");
		return size;
	}
	ret = kstrtouint(buf, LCD_BACKLIGHT_BASE, &val);
	if (ret < 0) {
		LCD_KIT_ERR("check input fail\n");
		return -EINVAL;
	}
	pchip = dev_get_drvdata(dev);
	if (pchip == NULL)
		return -EINVAL;

	bl_val = (unsigned char)val;
	bl_lsb = bl_val & pchip->bl_config.bl_lsb_reg_cmd.cmd_mask;
	bl_msb = (bl_val >> pchip->bl_config.bl_lsb_reg_cmd.val_bits) &
		pchip->bl_config.bl_msb_reg_cmd.cmd_mask;

	if (pchip->bl_config.bl_lsb_reg_cmd.val_bits != 0) {
		ret = lcd_kit_backlight_write_byte(pchip->client,
			pchip->bl_config.bl_lsb_reg_cmd.cmd_reg, bl_lsb);
		if (ret < 0) {
			LCD_KIT_ERR("set backlight failed\n");
			return -EINVAL;
		}
	}
	ret = lcd_kit_backlight_write_byte(pchip->client,
		pchip->bl_config.bl_msb_reg_cmd.cmd_reg, bl_msb);
	if (ret < 0) {
		LCD_KIT_ERR("set backlight failed\n");
		return -EINVAL;
	}
	return size;
}

static DEVICE_ATTR(backlight, S_IRUGO|S_IWUSR, backlight_bl_show, backlight_bl_store);

static struct attribute *backlight_attributes[] = {
	&dev_attr_reg_addr.attr,
	&dev_attr_reg.attr,
	&dev_attr_backlight.attr,
	NULL,
};

static const struct attribute_group backlight_group = {
	.attrs = backlight_attributes,
};
#endif

static void lcd_kit_backlight_propname_cat(char *pdest, const char *psrc,
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
	if (snprintf_s(pdest, dest_len, dest_len - 1, "%s,%s", g_backlight_ic_name, psrc) < 0) {
		LCD_KIT_ERR("snprintf_s fail\n");
		return;
	}
}

static void lcd_kit_backlight_init_propname_cat(char *pdest, const char *psrc,
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
	if (snprintf_s(pdest, dest_len, dest_len - 1, "%s,%s,%s",
		g_backlight_ic_name, g_panel_ic_name, psrc) < 0) {
		LCD_KIT_ERR("snprintf_s fail\n");
		return;
	}
}

void lcd_kit_parse_backlight_param(struct device_node *pnp,
	const char *node_str, unsigned int *pval)
{
	char tmp_buf[LCD_BACKLIGHT_IC_COMP_LEN] = {0};
	unsigned int tmp_val;
	int ret = 0;

	if ((node_str == NULL) || (pval == NULL) || (pnp == NULL)) {
		LCD_KIT_ERR("input invalid\n");
		return;
	}

	lcd_kit_backlight_propname_cat(tmp_buf, node_str, sizeof(tmp_buf));
	ret = of_property_read_u32(pnp, tmp_buf, &tmp_val);
	*pval = (!ret ? tmp_val : 0);
}

void lcd_kit_parse_backlight_init_param(struct device_node *pnp,
	const char *node_str, unsigned int *pval)
{
	char tmp_buf[LCD_BACKLIGHT_IC_COMP_LEN] = {0};
	unsigned int tmp_val;
	int ret = 0;

	if ((node_str == NULL) || (pval == NULL) || (pnp == NULL)) {
		LCD_KIT_ERR("input invalid\n");
		return;
	}

	lcd_kit_backlight_init_propname_cat(tmp_buf, node_str, sizeof(tmp_buf));
	ret = of_property_read_u32(pnp, tmp_buf, &tmp_val);
	*pval = (!ret ? tmp_val : 0);
}

static int lcd_kit_parse_backlight_cmd(struct device_node *np,
	const char *propertyname, struct backlight_ic_cmd *pcmd)
{
	char tmp_buf[LCD_BACKLIGHT_IC_COMP_LEN] = {0};
	struct property *prop = NULL;
	unsigned int *buf = NULL;
	int ret = 0;

	if ((np == NULL) || (propertyname == NULL) || (pcmd == NULL)) {
		LCD_KIT_ERR("input parameter is NULL\n");
		return LCD_KIT_FAIL;
	}
	lcd_kit_backlight_propname_cat(tmp_buf, propertyname, sizeof(tmp_buf));
	if (memset_s(pcmd, sizeof(struct backlight_ic_cmd), 0, sizeof(struct backlight_ic_cmd)) != EOK) {
		LCD_KIT_ERR("memset_s fail\n");
		return LCD_KIT_FAIL;
	}
	prop = of_find_property(np, tmp_buf, NULL);
	if (prop == NULL) {
		LCD_KIT_ERR("%s is not config\n", tmp_buf);
		return LCD_KIT_FAIL;
	}
	if (prop->length != BACKLIGHT_IC_CMD_LEN * sizeof(unsigned int)) {
		LCD_KIT_ERR("%s number is not right\n", tmp_buf);
		return LCD_KIT_FAIL;
	}
	buf = kzalloc(prop->length, GFP_KERNEL);
	if (buf == NULL) {
		LCD_KIT_ERR("malloc fail\n");
		return LCD_KIT_FAIL;
	}
	ret = of_property_read_u32_array(np, tmp_buf,
			buf, BACKLIGHT_IC_CMD_LEN);
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
	LCD_KIT_INFO("%s: type is 0x%x  reg is 0x%x  val is 0x%x   mask is 0x%x\n",
		propertyname, pcmd->ops_type, pcmd->cmd_reg,
		pcmd->cmd_val, pcmd->cmd_mask);
	kfree(buf);
	buf = NULL;

	return LCD_KIT_OK;
}

static int lcd_kit_backlight_parse_base_config(struct device_node *np,
	struct lcd_kit_backlight_info *pbl_config)
{
	if ((pbl_config == NULL) || (np == NULL))
		return LCD_KIT_FAIL;
	lcd_kit_parse_backlight_param(np, "backlight_ic_backlight_level",
		&pbl_config->bl_level);
	lcd_kit_parse_backlight_param(np, "backlight_ic_kernel_ctrl_mode",
		&pbl_config->bl_ctrl_mod);
	lcd_kit_parse_backlight_param(np, "backlight_ic_only_backlight",
		&pbl_config->bl_only);
	lcd_kit_parse_backlight_param(np, "backlight_ic_level_10000",
		&pbl_config->bl_10000_support);
	lcd_kit_parse_backlight_param(np, "backlight_ic_bits_compatible",
		&pbl_config->bits_compatible);
	lcd_kit_parse_backlight_param(np, "backlight_ic_set_msb_before_lsb",
		&pbl_config->msb_before_lsb);
	lcd_kit_parse_backlight_param(np, "backlight_dual_ic",
		&pbl_config->dual_ic);
	if (pbl_config->dual_ic) {
		lcd_kit_parse_backlight_param(np, "backlight_dual_ic_i2c_bus_id",
			&pbl_config->dual_i2c_bus_id);
		LCD_KIT_INFO("dual_ic %u dual bus id %u\n", pbl_config->dual_ic,
			pbl_config->dual_i2c_bus_id);
	}
	lcd_kit_parse_backlight_param(np, "dimming_support",
		&pbl_config->bl_dimming_support);
	lcd_kit_parse_backlight_param(np, "dimming_config",
		&pbl_config->bl_dimming_config);
	lcd_kit_parse_backlight_param(np, "dimming_resume",
		&pbl_config->bl_dimming_resume);
	lcd_kit_parse_backlight_param(np, "dimming_config_reg",
		&pbl_config->dimming_config_reg);

	return LCD_KIT_OK;
}

static void lcd_kit_backlight_parse_bias_config(struct device_node *np,
	struct lcd_kit_backlight_info *pbl_config)
{
	int ret;
	int boost_offset;

	if ((pbl_config == NULL) || (np == NULL)) {
		LCD_KIT_ERR("input parameter is NULL\n");
		return;
	}
	lcd_kit_parse_backlight_param(np, "backlight_ic_init_vsp_index",
		&pbl_config->init_vsp_index);
	lcd_kit_parse_backlight_param(np, "backlight_ic_init_vsn_index",
		&pbl_config->init_vsn_index);
	lcd_kit_parse_backlight_param(np, "backlight_ic_init_boost_index",
		&pbl_config->init_boost_index);
	lcd_kit_parse_backlight_param(np, "backlight_ic_init_update_boost",
		&pbl_config->init_update_boost);
	lcd_kit_parse_backlight_param(np, "backlight_ic_boost_offset",
		&boost_offset);
	if (boost_offset)
		pbl_config->boost_offset = boost_offset;
	else
		pbl_config->boost_offset = LCD_BACKLIGHT_BOOST_OFFSET;

	LCD_KIT_INFO("parse boost_offset is %d\n", pbl_config->boost_offset);

	lcd_kit_parse_backlight_param(np, "backlight_ic_pull_boost_support",
		&pbl_config->pull_down_boost_support);
	if (pbl_config->pull_down_boost_support) {
		lcd_kit_parse_backlight_param(np, "backlight_ic_pull_boost_delay",
			&pbl_config->pull_down_boost_delay);
		ret = lcd_kit_parse_backlight_cmd(np,
			"backlight_ic_pull_down_vsp_cmd",
			&pbl_config->pull_down_vsp_cmd);
		if (ret < 0)
			LCD_KIT_ERR("parse pull down vsp cmd fail\n");
		ret = lcd_kit_parse_backlight_cmd(np,
			"backlight_ic_pull_down_vsn_cmd",
			&pbl_config->pull_down_vsn_cmd);
		if (ret < 0)
			LCD_KIT_ERR("parse pull down vsn cmd fail\n");
		ret = lcd_kit_parse_backlight_cmd(np,
			"backlight_ic_pull_down_boost_cmd",
			&pbl_config->pull_down_boost_cmd);
		if (ret < 0)
			LCD_KIT_ERR("parse pull down boost cmd fail\n");
		ret = lcd_kit_parse_backlight_cmd(np,
			"backlight_ic_bias_enable_cmd",
			&pbl_config->bias_enable_cmd);
		if (ret < 0)
			LCD_KIT_ERR("parse pull down boost cmd fail\n");
	}
}

static void lcd_kit_backlight_parse_gpio_config(struct device_node *np,
	struct lcd_kit_backlight_info *pbl_config)
{
	if ((pbl_config == NULL) || (np == NULL)) {
		LCD_KIT_ERR("input parameter is NULL\n");
		return;
	}

	lcd_kit_parse_backlight_param(np, "hw_enable",
		&pbl_config->hw_en);
	if (pbl_config->hw_en) {
		lcd_kit_parse_backlight_param(np, "hw_en_gpio",
			&pbl_config->hw_en_gpio);
		lcd_kit_parse_backlight_param(np, "gpio_offset",
			&pbl_config->gpio_offset);
		pbl_config->hw_en_gpio += pbl_config->gpio_offset;
		LCD_KIT_INFO("hw_en_gpio is %u\n", pbl_config->hw_en_gpio);
		lcd_kit_parse_backlight_param(np, "bl_on_mdelay",
			&pbl_config->bl_on_mdelay);
		lcd_kit_parse_backlight_param(np, "before_bl_on_mdelay",
			&pbl_config->before_bl_on_mdelay);
		lcd_kit_parse_backlight_param(np, "hw_en_pull_low",
			&pbl_config->hw_en_pull_low);
		if (pbl_config->dual_ic) {
			lcd_kit_parse_backlight_param(np, "dual_hw_en_gpio",
				&pbl_config->dual_hw_en_gpio);
			pbl_config->dual_hw_en_gpio += pbl_config->gpio_offset;
			LCD_KIT_INFO("dual_hw_en_gpio is %u\n",
				pbl_config->dual_hw_en_gpio);
		}
	}
}

static int lcd_kit_backlight_parse_bl_cmd(struct device_node *np,
	const char *propertyname, struct backlight_reg_info *pcmd)
{
	char tmp_buf[LCD_BACKLIGHT_IC_COMP_LEN] = {0};
	struct property *prop = NULL;
	unsigned int *buf = NULL;
	int ret = 0;

	if ((np == NULL) || (propertyname == NULL) || (pcmd == NULL)) {
		LCD_KIT_ERR("input parameter is NULL\n");
		return LCD_KIT_FAIL;
	}
	lcd_kit_backlight_propname_cat(tmp_buf, propertyname, sizeof(tmp_buf));
	if (memset_s(pcmd, sizeof(struct backlight_reg_info), 0, sizeof(struct backlight_reg_info)) != EOK) {
		LCD_KIT_ERR("memset_s fail\n");
		return LCD_KIT_FAIL;
	}
	prop = of_find_property(np, tmp_buf, NULL);
	if (prop == NULL) {
		LCD_KIT_ERR("%s is not config\n", tmp_buf);
		return LCD_KIT_FAIL;
	}
	if (prop->length != BACKLIGHT_BL_CMD_LEN * sizeof(unsigned int)) {
		LCD_KIT_ERR("%s number is not right\n", tmp_buf);
		return LCD_KIT_FAIL;
	}
	buf = kzalloc(prop->length, GFP_KERNEL);
	if (buf == NULL) {
		LCD_KIT_ERR("malloc fail\n");
		return LCD_KIT_FAIL;
	}
	ret = of_property_read_u32_array(np, tmp_buf,
		buf, BACKLIGHT_BL_CMD_LEN);
	if (ret < 0) {
		LCD_KIT_ERR("get %s config fail\n", tmp_buf);
		kfree(buf);
		buf = NULL;
		return LCD_KIT_FAIL;
	}
	/* 0:cmd_reg 1:cmd_val  2:val_bits  3:cmd_mask */
	pcmd->cmd_reg = buf[0];
	pcmd->cmd_val = buf[1];
	pcmd->val_bits = buf[2];
	pcmd->cmd_mask = buf[3];
	LCD_KIT_INFO("%s: val_bits is 0x%x  reg is 0x%x  val is 0x%x   mask is 0x%x\n",
		tmp_buf, pcmd->val_bits, pcmd->cmd_reg, pcmd->cmd_val, pcmd->cmd_mask);
	kfree(buf);
	buf = NULL;

	return LCD_KIT_OK;
}

static void lcd_kit_backlight_parse_backlight_config(struct device_node *np,
	struct lcd_kit_backlight_info *pbl_config)
{
	int ret;

	if ((np == NULL) || (pbl_config == NULL)) {
		LCD_KIT_ERR("input parameter is NULL\n");
		return;
	}
	ret = lcd_kit_backlight_parse_bl_cmd(np, "backlight_ic_level_lsb_cmd",
		&pbl_config->bl_lsb_reg_cmd);
	if (ret < 0)
		LCD_KIT_ERR("parse lsb cmd fail\n");
	ret = lcd_kit_backlight_parse_bl_cmd(np, "backlight_ic_level_msb_cmd",
		&pbl_config->bl_msb_reg_cmd);
	if (ret < 0)
		LCD_KIT_ERR("parse msb cmd fail\n");
	ret = lcd_kit_parse_backlight_cmd(np, "backlight_ic_bl_enable_cmd",
		&pbl_config->bl_enable_cmd);
	if (ret < 0)
		LCD_KIT_ERR("parse bl enable cmd fail\n");
	ret = lcd_kit_parse_backlight_cmd(np, "backlight_ic_bl_disable_cmd",
		&pbl_config->bl_disable_cmd);
	if (ret < 0)
		LCD_KIT_ERR("parse bl disable cmd fail\n");
	ret = lcd_kit_parse_backlight_cmd(np, "backlight_ic_vsp_enable_cmd",
		&pbl_config->vsp_enable_cmd);
	if (ret < 0)
		LCD_KIT_ERR("parse vsp enable cmd fail\n");
	ret = lcd_kit_parse_backlight_cmd(np, "backlight_ic_vsn_enable_cmd",
		&pbl_config->vsn_enable_cmd);
	if (ret < 0)
		LCD_KIT_ERR("parse vsn enable cmd fail\n");
	ret = lcd_kit_parse_backlight_cmd(np, "backlight_ic_vsp_disable_cmd",
		&pbl_config->vsp_disable_cmd);
	if (ret < 0)
		LCD_KIT_ERR("parse vsp disable cmd fail\n");
	ret = lcd_kit_parse_backlight_cmd(np, "backlight_ic_vsn_disable_cmd",
		&pbl_config->vsn_disable_cmd);
	if (ret < 0)
		LCD_KIT_ERR("parse vsn disable cmd fail\n");
	ret = lcd_kit_parse_backlight_cmd(np, "backlight_ic_dev_disable_cmd",
		&pbl_config->disable_dev_cmd);
	if (ret < 0)
		LCD_KIT_ERR("parse dev disable cmd fail\n");
}

static void lcd_kit_backlight_parse_fault_check_config(struct device_node *np,
	struct lcd_kit_backlight_info *pbl_config)
{
	int ret;

	if ((pbl_config == NULL) || (np == NULL)) {
		LCD_KIT_ERR("input parameter is NULL\n");
		return;
	}
	lcd_kit_parse_backlight_param(np, "backlight_ic_fault_check_support",
		&pbl_config->fault_check_enable);
	if (pbl_config->fault_check_enable) {
		ret = lcd_kit_parse_backlight_cmd(np,
			"backlight_ic_ovp_flag_cmd",
			&pbl_config->bl_ovp_flag_cmd);
		if (ret < 0)
			LCD_KIT_ERR("parse ovp flag cmd fail\n");
		ret = lcd_kit_parse_backlight_cmd(np,
			"backlight_ic_ocp_flag_cmd",
			&pbl_config->bl_ocp_flag_cmd);
		if (ret < 0)
			LCD_KIT_ERR("parse ocp flag cmd fail\n");
		ret = lcd_kit_parse_backlight_cmd(np,
			"backlight_ic_tsd_flag_cmd",
			&pbl_config->bl_tsd_flag_cmd);
		if (ret < 0)
			LCD_KIT_ERR("parse tsd flag cmd fail\n");
	}
}

static void lcd_kit_backlight_parse_init_cmds_config(struct device_node *np,
	struct lcd_kit_backlight_info *pbl_config)
{
	int ret;
	struct property *prop = NULL;
	unsigned int i;
	unsigned int *buf = NULL;
	char tmp_buf[LCD_BACKLIGHT_IC_COMP_LEN] = {0};

	if ((pbl_config == NULL) || (np == NULL)) {
		LCD_KIT_ERR("input parameter is NULL\n");
		return;
	}
	lcd_kit_parse_backlight_init_param(np, "backlight_ic_num_of_kernel_init_cmd", &pbl_config->num_of_init_cmds);
	lcd_kit_backlight_init_propname_cat(tmp_buf, "backlight_ic_kernel_init_cmd", sizeof(tmp_buf));
	prop = of_find_property(np, tmp_buf, NULL);
	if (prop == NULL) {
		LCD_KIT_ERR("kernel init cmds is not config\n");
		return;
	}
	if (prop->length != (int)(pbl_config->num_of_init_cmds * BACKLIGHT_IC_CMD_LEN * sizeof(unsigned int))) {
		LCD_KIT_ERR("init cmds number is not right\n");
		return;
	}
	buf = kzalloc(prop->length, GFP_KERNEL);
	if (buf == NULL) {
		LCD_KIT_ERR("malloc fail\n");
		return;
	}
	lcd_kit_backlight_init_propname_cat(tmp_buf, "backlight_ic_kernel_init_cmd", sizeof(tmp_buf));
	ret = of_property_read_u32_array(np, tmp_buf, buf, prop->length / sizeof(unsigned int));
	if (ret < 0) {
		LCD_KIT_ERR("get kernel init cmds config fail\n");
		kfree(buf);
		buf = NULL;
		return;
	}
	for (i = 0; i < pbl_config->num_of_init_cmds; i++) {
		pbl_config->init_cmds[i].ops_type = buf[i * BACKLIGHT_IC_CMD_LEN];
		pbl_config->init_cmds[i].cmd_reg = buf[i * BACKLIGHT_IC_CMD_LEN + 1];
		pbl_config->init_cmds[i].cmd_val = buf[i * BACKLIGHT_IC_CMD_LEN + 2];
		pbl_config->init_cmds[i].cmd_mask = buf[i * BACKLIGHT_IC_CMD_LEN + 3];
		pbl_config->init_cmds[i].delay = buf[i * BACKLIGHT_IC_CMD_LEN + 4];
		LCD_KIT_INFO("init i is %d, tpye is 0x%x  reg is 0x%x  val is 0x%x   mask is 0x%x\n",
		i, pbl_config->init_cmds[i].ops_type, pbl_config->init_cmds[i].cmd_reg,
		pbl_config->init_cmds[i].cmd_val, pbl_config->init_cmds[i].cmd_mask);
	}
	kfree(buf);
	buf = NULL;
}

void lcd_kit_backlight_parse_dts(struct device_node *np,
	struct lcd_kit_backlight_info *pbl_config)
{
	int ret;

	LCD_KIT_INFO("enter\n");
	if ((np == NULL) || (pbl_config == NULL)) {
		LCD_KIT_ERR("input parameter is NULL\n");
		return;
	}
	ret = lcd_kit_backlight_parse_base_config(np, pbl_config);
	if (ret < 0) {
		LCD_KIT_ERR("parse base config fail\n");
		return;
	}
	if (!pbl_config->bl_only)
		lcd_kit_backlight_parse_bias_config(np, pbl_config);
	lcd_kit_backlight_parse_gpio_config(np, pbl_config);
	lcd_kit_backlight_parse_backlight_config(np, pbl_config);
	lcd_kit_backlight_parse_fault_check_config(np, pbl_config);
	lcd_kit_backlight_parse_init_cmds_config(np, pbl_config);
}

static void lcd_kit_backlight_get_bias_value(int vsp, int vsn,
	unsigned char *pos_votage, unsigned char *neg_votage)
{
	int i;
	int j;

	if ((pos_votage == NULL) || (neg_votage == NULL)) {
		LCD_KIT_ERR("input parameter is NULL\n");
		return;
	}

	for (i = 0; i < ARRAY_SIZE(g_backlight_bias_config); i++) {
		if (strcmp(g_backlight_ic_name, g_backlight_bias_config[i].name))
			continue;
		for (j = 0; j < g_backlight_bias_config[i].len; j++) {
			if (g_backlight_bias_config[i].vol_table[j].voltage == (unsigned int)vsp) {
				*pos_votage = g_backlight_bias_config[i].vol_table[j].value;
				break;
			}
		}
		if (j == g_backlight_bias_config[i].len) {
			LCD_KIT_INFO("not found vsp voltage, use default\n");
			*pos_votage = DEFAULT_VOL_VAL;
		}

		for (j = 0; j < g_backlight_bias_config[i].len; j++) {
			if (g_backlight_bias_config[i].vol_table[j].voltage == (unsigned int)vsn) {
				*neg_votage  = g_backlight_bias_config[i].vol_table[j].value;
				break;
			}
		}

		if (j == g_backlight_bias_config[i].len) {
			LCD_KIT_INFO("not found neg voltage, use default voltage\n");
			*neg_votage = DEFAULT_VOL_VAL;
		}
		break;
	}

	if (i == ARRAY_SIZE(g_backlight_bias_config)) {
		LCD_KIT_INFO("not found right voltage, use default voltage\n");
		*pos_votage = DEFAULT_VOL_VAL;
		*neg_votage = DEFAULT_VOL_VAL;
	}
}

static int lcd_kit_backlight_dual_ic_i2c_adapter(void)
{
	struct i2c_adapter *adap = NULL;

	LCD_KIT_INFO("%s in", __func__);
	if (g_pbacklight_dev == NULL) {
		LCD_KIT_ERR(" backlight device is NULL\n");
		return LCD_KIT_FAIL;
	}
	adap = i2c_get_adapter(g_pbacklight_dev->bl_config.dual_i2c_bus_id);
	if (!adap) {
		LCD_KIT_ERR("dual i2c device %d not found\n",
			g_pbacklight_dev->bl_config.dual_i2c_bus_id);
		return LCD_KIT_FAIL;
	}
	g_backlight_dual_ic_adap = adap;
	LCD_KIT_INFO("%s exit", __func__);
	return LCD_KIT_OK;
}

static int lcd_kit_backlight_dual_ic_config(void)
{
	struct i2c_adapter *adap = g_backlight_dual_ic_adap;
	struct i2c_msg msg = {0};
	char buf[2]; /* buf[0] is cmd reg buf[1] is cmd val */
	int ret;
	unsigned int i;

	LCD_KIT_INFO("%s in", __func__);
	if (g_pbacklight_dev == NULL) {
		LCD_KIT_ERR(" backlight device is NULL\n");
		return LCD_KIT_FAIL;
	}
	if (!adap) {
		LCD_KIT_ERR("dual i2c device %d not found\n",
			g_pbacklight_dev->bl_config.dual_i2c_bus_id);
		return LCD_KIT_FAIL;
	}

	msg.addr = g_pbacklight_dev->client->addr;
	msg.flags = g_pbacklight_dev->client->flags;
	msg.len = 2;
	msg.buf = buf;

	for (i = 0; i < g_pbacklight_dev->bl_config.num_of_init_cmds; i++) {
		buf[0] = g_pbacklight_dev->bl_config.init_cmds[i].cmd_reg;
		buf[1] = g_pbacklight_dev->bl_config.init_cmds[i].cmd_val;
		ret = i2c_transfer(adap, &msg, 1);
		LCD_KIT_INFO("%s reg = 0x%x, val = 0x%x\n",
			__func__, buf[0], buf[1]);
		if (g_pbacklight_dev->bl_config.init_cmds[i].delay)
			mdelay(g_pbacklight_dev->bl_config.init_cmds[i].delay);
	}

	return ret;
}

static int lcd_kit_setbacklight_dual_ic(int bl_lsb, int bl_msb)
{
	struct i2c_adapter *adap = g_backlight_dual_ic_adap;
	struct i2c_msg msg = {0};
	struct i2c_msg msg2 = {0};
	char buf[2];  /* buf[0] is cmd reg, buf[1] is cmd val */
	char buf2[2]; /* buf2[0] is cmd reg, buf2[1] is cmd val */
	int ret;
	if (g_pbacklight_dev == NULL) {
		LCD_KIT_ERR(" backlight device is NULL\n");
		return LCD_KIT_FAIL;
	}
	if (!adap) {
		LCD_KIT_ERR("dual i2c device %d not found\n",
			g_pbacklight_dev->bl_config.dual_i2c_bus_id);
		return LCD_KIT_FAIL;
	}
	msg.addr = g_pbacklight_dev->client->addr;
	msg.flags = g_pbacklight_dev->client->flags;
	msg.len = 2;
	msg2 = msg;
	buf[0] = g_pbacklight_dev->bl_config.bl_lsb_reg_cmd.cmd_reg;
	buf[1] = (unsigned int)bl_lsb;
	buf2[0] = g_pbacklight_dev->bl_config.bl_msb_reg_cmd.cmd_reg;
	buf2[1] = (unsigned int)bl_msb;
	if (!g_pbacklight_dev->bl_config.msb_before_lsb) {
		msg.buf = buf;   /* lsb */
		msg2.buf = buf2; /* msb */
	} else {
		msg.buf = buf2;  /* msb */
		msg2.buf = buf;  /* lsb */
	}
	ret = i2c_transfer(adap, &msg, 1);
	LCD_KIT_INFO("%s reg = 0x%x, val = 0x%x\n", __func__, buf[0], buf[1]);
	ret = i2c_transfer(adap, &msg2, 1);
	LCD_KIT_INFO("%s reg = 0x%x, val = 0x%x\n", __func__, buf2[0], buf2[1]);

	return ret;
}
static int dync_dimming_config_dual_ic(unsigned char dim_reg, unsigned char dim_data)
{
	struct i2c_adapter *adap = g_backlight_dual_ic_adap;
	struct i2c_msg msg = {0};

	char buf[2];  /* buf[0] is cmd reg, buf[1] is cmd val */
	int ret;
	if (g_pbacklight_dev == NULL) {
		LCD_KIT_ERR(" backlight device is NULL\n");
		return LCD_KIT_FAIL;
	}
	if (!adap) {
		LCD_KIT_ERR("dual i2c device %d not found\n",
			g_pbacklight_dev->bl_config.dual_i2c_bus_id);
		return LCD_KIT_FAIL;
	}

	msg.addr = g_pbacklight_dev->client->addr;
	msg.flags = g_pbacklight_dev->client->flags;
	msg.len = 2;

	buf[0] = dim_reg;
	buf[1] = dim_data;
	msg.buf = buf;

	ret = i2c_transfer(adap, &msg, 1);
	LCD_KIT_INFO("%s reg = 0x%x, val = 0x%x\n", __func__, buf[0], buf[1]);

	return ret;
}

static int lcd_kit_backlight_ic_config(void)
{
	int ret;
	unsigned int i;

	LCD_KIT_INFO("backlight ic config\n");
	if (g_pbacklight_dev == NULL) {
		LCD_KIT_ERR(" backlight device is NULL\n");
		return LCD_KIT_FAIL;
	}

	for (i = 0; i < g_pbacklight_dev->bl_config.num_of_init_cmds; i++) {
		switch (g_pbacklight_dev->bl_config.init_cmds[i].ops_type) {
		case REG_READ_MODE:
			ret = lcd_kit_backlight_read_byte(g_pbacklight_dev->client,
				g_pbacklight_dev->bl_config.init_cmds[i].cmd_reg,
				&(g_pbacklight_dev->bl_config.init_cmds[i].cmd_val));
			break;
		case REG_WRITE_MODE:
			ret = lcd_kit_backlight_write_byte(g_pbacklight_dev->client,
				g_pbacklight_dev->bl_config.init_cmds[i].cmd_reg,
				g_pbacklight_dev->bl_config.init_cmds[i].cmd_val);
			break;
		case REG_UPDATE_MODE:
			ret = lcd_kit_backlight_update_bit(g_pbacklight_dev->client,
				g_pbacklight_dev->bl_config.init_cmds[i].cmd_reg,
				g_pbacklight_dev->bl_config.init_cmds[i].cmd_mask,
				g_pbacklight_dev->bl_config.init_cmds[i].cmd_val);
			break;
		default:
			break;
		}
		if (ret < 0) {
			LCD_KIT_ERR("operation  reg 0x%x fail\n",
				g_pbacklight_dev->bl_config.init_cmds[i].cmd_reg);
			return ret;
		}
		if (g_pbacklight_dev->bl_config.init_cmds[i].delay)
			mdelay(g_pbacklight_dev->bl_config.init_cmds[i].delay);
	}

	return ret;
}

static int lcd_kit_gpio_dual_request()
{
	int ret;

	if (g_pbacklight_dev->bl_config.dual_hw_en_gpio) {
		ret = gpio_request(g_pbacklight_dev->bl_config.dual_hw_en_gpio, NULL);
		if (ret < 0) {
			LCD_KIT_ERR("can not request  dual_hw_en_gpio\n");
			return LCD_KIT_FAIL;
		}
		ret = gpio_direction_output(g_pbacklight_dev->bl_config.dual_hw_en_gpio,
			BACKLIGHT_GPIO_DIR_OUT);
		if (ret < 0)
			LCD_KIT_ERR("set gpio output not success\n");
		gpio_set_value(g_pbacklight_dev->bl_config.dual_hw_en_gpio,
			BACKLIGHT_GPIO_OUT_ONE);
		LCD_KIT_INFO("lcd_kit_backlight_enable dual_hw_en_gpio = %d suc\n",
			g_pbacklight_dev->bl_config.dual_hw_en_gpio);
	}

	return LCD_KIT_OK;
}

static int lcd_kit_backlight_set_bias(int vpos, int vneg)
{
	int ret;
	unsigned int vsp_index;
	unsigned int vsn_index;
	unsigned int boost_index;
	unsigned char vsp_vol = 0;
	unsigned char vsn_vol = 0;

	if (vpos < 0 || vneg < 0) {
		LCD_KIT_ERR("vpos or vneg is error\n");
		return LCD_KIT_FAIL;
	}

	if (g_pbacklight_dev == NULL)
		return LCD_KIT_FAIL;

	if (g_pbacklight_dev->bl_config.hw_en != 0 &&
		g_pbacklight_dev->bl_config.hw_en_pull_low != 0) {
		ret = gpio_request(g_pbacklight_dev->bl_config.hw_en_gpio, NULL);
		if (ret < 0) {
			LCD_KIT_ERR("can not request  hw_en_gpio\n");
			return LCD_KIT_FAIL;
		}
		ret = gpio_direction_output(g_pbacklight_dev->bl_config.hw_en_gpio,
			BACKLIGHT_GPIO_DIR_OUT);
		if (ret < 0)
			LCD_KIT_ERR("set gpio output not success\n");
		gpio_set_value(g_pbacklight_dev->bl_config.hw_en_gpio, BACKLIGHT_GPIO_OUT_ONE);
		if (g_pbacklight_dev->bl_config.bl_on_mdelay)
			mdelay(g_pbacklight_dev->bl_config.bl_on_mdelay);
	}
	if (g_vol_mapped == BACKLIGHT_FALSE) {
		vsp_index = g_pbacklight_dev->bl_config.init_vsp_index;
		vsn_index = g_pbacklight_dev->bl_config.init_vsn_index;
		lcd_kit_backlight_get_bias_value(vpos, vneg, &vsp_vol, &vsn_vol);
		g_pbacklight_dev->bl_config.init_cmds[vsp_index].cmd_val = vsp_vol;
		g_pbacklight_dev->bl_config.init_cmds[vsn_index].cmd_val = vsn_vol;
		if (g_pbacklight_dev->bl_config.init_update_boost) {
			boost_index = g_pbacklight_dev->bl_config.init_boost_index;
			g_pbacklight_dev->bl_config.init_cmds[boost_index].cmd_val = vsp_vol +
				(unsigned char)g_pbacklight_dev->bl_config.boost_offset;
		}
		g_vol_mapped = BACKLIGHT_TRUE;
	}
	ret = lcd_kit_backlight_ic_config();
	if (ret < 0) {
		LCD_KIT_ERR("set config register failed");
		return ret;
	}
	g_backlight_init_status = true;
	LCD_KIT_INFO("set_bias is successful\n");
	return ret;
}

static void lcd_kit_backlight_enable(void)
{
	int ret;

	if (g_pbacklight_dev->bl_config.hw_en != 0 &&
		g_pbacklight_dev->bl_config.hw_en_pull_low != 0) {
		if (g_pbacklight_dev->bl_config.before_bl_on_mdelay)
			mdelay(g_pbacklight_dev->bl_config.before_bl_on_mdelay);
		if ((g_pbacklight_dev->bl_config.dual_ic) &&
			(g_pbacklight_dev->bl_config.dual_hw_en_gpio != g_pbacklight_dev->bl_config.hw_en_gpio)) {
			ret = lcd_kit_gpio_dual_request();
			if (ret < 0) {
				LCD_KIT_ERR("can not request gpio_dual\n");
				return;
			}
		}
		ret = gpio_request(g_pbacklight_dev->bl_config.hw_en_gpio, NULL);
		if (ret < 0) {
			LCD_KIT_ERR("can not request  hw_en_gpio\n");
			return;
		}
		ret = gpio_direction_output(g_pbacklight_dev->bl_config.hw_en_gpio, BACKLIGHT_GPIO_DIR_OUT);
		if (ret < 0)
			LCD_KIT_ERR("set gpio output not success\n");
		gpio_set_value(g_pbacklight_dev->bl_config.hw_en_gpio, BACKLIGHT_GPIO_OUT_ONE);
		if (g_pbacklight_dev->bl_config.bl_on_mdelay)
			mdelay(g_pbacklight_dev->bl_config.bl_on_mdelay);
	}

	/* chip initialize */
	ret = lcd_kit_backlight_ic_config();
	if (ret < 0) {
		LCD_KIT_ERR("backlight ic init fail!\n");
		return;
	}
	if (g_pbacklight_dev->bl_config.dual_ic) {
		ret = lcd_kit_backlight_dual_ic_config();
		if (ret < 0) {
			LCD_KIT_ERR("dual bl_ic not used\n");
			return;
		}
	}
	g_backlight_init_status = true;
}

static void lcd_kit_backlight_disable(void)
{
	LCD_KIT_INFO("backlight ic disable enter\n");
	if (g_pbacklight_dev->bl_config.hw_en != 0 &&
		g_pbacklight_dev->bl_config.hw_en_pull_low != 0) {
		if (g_pbacklight_dev->bl_config.dual_ic &&
			g_pbacklight_dev->bl_config.dual_hw_en_gpio &&
			g_pbacklight_dev->bl_config.dual_hw_en_gpio != g_pbacklight_dev->bl_config.hw_en_gpio) {
			gpio_set_value(g_pbacklight_dev->bl_config.dual_hw_en_gpio,
				BACKLIGHT_GPIO_OUT_ZERO);
			gpio_free(g_pbacklight_dev->bl_config.dual_hw_en_gpio);
		}
		gpio_set_value(g_pbacklight_dev->bl_config.hw_en_gpio, BACKLIGHT_GPIO_OUT_ZERO);
		gpio_free(g_pbacklight_dev->bl_config.hw_en_gpio);
	}
	g_backlight_init_status = false;
}

static int lcd_kit_backlight_set_ic_disable(void)
{
	int ret;

	if (g_pbacklight_dev == NULL)
		return LCD_KIT_FAIL;

	/* reset backlight ic */
	ret = lcd_kit_backlight_write_byte(g_pbacklight_dev->client,
		g_pbacklight_dev->bl_config.disable_dev_cmd.cmd_reg,
		g_pbacklight_dev->bl_config.disable_dev_cmd.cmd_val);
	if (ret < 0)
		LCD_KIT_ERR("write REG_BL_ENABLE fail\n");

	/* clean up bl val register */
	ret = lcd_kit_backlight_update_bit(g_pbacklight_dev->client,
		g_pbacklight_dev->bl_config.bl_lsb_reg_cmd.cmd_reg,
		g_pbacklight_dev->bl_config.bl_lsb_reg_cmd.cmd_mask, 0);
	if (ret < 0)
		LCD_KIT_ERR("update_bits REG_BL_BRIGHTNESS_LSB fail\n");

	ret = lcd_kit_backlight_write_byte(g_pbacklight_dev->client,
		g_pbacklight_dev->bl_config.bl_msb_reg_cmd.cmd_reg, 0);
	if (ret < 0)
		LCD_KIT_ERR("write REG_BL_BRIGHTNESS_MSB fail!\n");

	if (g_pbacklight_dev->bl_config.hw_en != 0 &&
		g_pbacklight_dev->bl_config.hw_en_pull_low != 0) {
		if (g_pbacklight_dev->bl_config.dual_ic &&
			g_pbacklight_dev->bl_config.dual_hw_en_gpio) {
			gpio_set_value(g_pbacklight_dev->bl_config.dual_hw_en_gpio,
				BACKLIGHT_GPIO_OUT_ZERO);
			gpio_free(g_pbacklight_dev->bl_config.dual_hw_en_gpio);
		}
		gpio_set_value(g_pbacklight_dev->bl_config.hw_en_gpio, BACKLIGHT_GPIO_OUT_ZERO);
		gpio_free(g_pbacklight_dev->bl_config.hw_en_gpio);
	}
	g_backlight_init_status = false;
	return ret;
}

static int lcd_kit_backlight_set_bias_power_down(int vpos, int vneg)
{
	int ret;
	unsigned char vsp = 0;
	unsigned char vsn = 0;

	if (g_pbacklight_dev == NULL)
		return LCD_KIT_FAIL;

	if (!g_pbacklight_dev->bl_config.pull_down_boost_support) {
		LCD_KIT_INFO("No need to pull down BOOST\n");
		return 0;
	}

	lcd_kit_backlight_get_bias_value(vpos, vneg, &vsp, &vsn);
	ret = lcd_kit_backlight_write_byte(g_pbacklight_dev->client,
		g_pbacklight_dev->bl_config.pull_down_vsp_cmd.cmd_reg, vsp);
	if (ret < 0) {
		LCD_KIT_ERR("write pull_down_vsp failed\n");
		return ret;
	}
	ret = lcd_kit_backlight_write_byte(g_pbacklight_dev->client,
		g_pbacklight_dev->bl_config.pull_down_vsn_cmd.cmd_reg, vsn);
	if (ret < 0) {
		LCD_KIT_ERR("write pull_down_vsn failed\n");
		return ret;
	}
	ret = lcd_kit_backlight_write_byte(g_pbacklight_dev->client,
		g_pbacklight_dev->bl_config.bias_enable_cmd.cmd_reg,
		g_pbacklight_dev->bl_config.bias_enable_cmd.cmd_val);
	if (ret < 0) {
		LCD_KIT_ERR("write enable_vsp_vsn failed\n");
		return ret;
	}
	ret = lcd_kit_backlight_write_byte(g_pbacklight_dev->client,
		g_pbacklight_dev->bl_config.pull_down_boost_cmd.cmd_reg,
		g_pbacklight_dev->bl_config.pull_down_boost_cmd.cmd_val);
	if (ret < 0) {
		LCD_KIT_ERR("write boost_vsp_vsn failed\n");
		return ret;
	}
	LCD_KIT_INFO("lcd_kit_pull_boost is successful\n");
	return ret;
}

#if defined CONFIG_HUAWEI_DSM
static void lcd_kit_backlight_ic_ovp_check(struct lcd_kit_backlight_device *pchip,
	int last_level, int level)
{
	unsigned char val = 0;
	int ret;

	if (pchip == NULL) {
		LCD_KIT_ERR("input parameter is NULL\n");
		return;
	}
	if (!pchip->bl_config.bl_ovp_flag_cmd.cmd_mask)
		return;
	ret = lcd_kit_backlight_read_byte(pchip->client,
		pchip->bl_config.bl_ovp_flag_cmd.cmd_reg, &val);
	if (ret < 0) {
		LCD_KIT_ERR("read backlight ic ovp reg fail\n");
		return;
	}
	if ((val & pchip->bl_config.bl_ovp_flag_cmd.cmd_mask) ==
		pchip->bl_config.bl_ovp_flag_cmd.cmd_val) {
		LCD_KIT_INFO("backlight ic check ovp fault val is 0x%x\n", val);
		if (lcd_dclient == NULL)
			return;
		ret = dsm_client_ocuppy(lcd_dclient);
		if (ret != 0) {
			LCD_KIT_ERR("dsm_client_ocuppy fail, ret=%d\n", ret);
			return;
		}
		dsm_client_record(lcd_dclient,
			"%s : last_bkl:%d, cur_bkl:%d, reg val 0x%x=0x%x\n",
			g_backlight_ic_name, last_level, level,
			pchip->bl_config.bl_ovp_flag_cmd.cmd_reg, val);
		dsm_client_notify(lcd_dclient, DSM_LCD_OVP_ERROR_NO);
	}
}

static void lcd_kit_backlight_ic_ocp_check(struct lcd_kit_backlight_device *pchip,
	int last_level, int level)
{
	unsigned char val = 0;
	int ret;

	if (pchip == NULL) {
		LCD_KIT_ERR("input parameter is NULL\n");
		return;
	}
	if (!pchip->bl_config.bl_ocp_flag_cmd.cmd_mask)
		return;
	ret = lcd_kit_backlight_read_byte(pchip->client,
		pchip->bl_config.bl_ocp_flag_cmd.cmd_reg, &val);
	if (ret < 0) {
		LCD_KIT_ERR("read backlight ic ocp reg fail\n");
		return;
	}
	if ((val & pchip->bl_config.bl_ocp_flag_cmd.cmd_mask) ==
		pchip->bl_config.bl_ocp_flag_cmd.cmd_val) {
		LCD_KIT_INFO("backlight ic check ocp fault val is 0x%x\n", val);
		if (lcd_dclient == NULL)
			return;
		ret = dsm_client_ocuppy(lcd_dclient);
		if (ret != 0) {
			LCD_KIT_ERR("dsm_client_ocuppy fail, ret=%d\n", ret);
			return;
		}
		dsm_client_record(lcd_dclient,
			"%s : last_bkl:%d, cur_bkl:%d, reg val 0x%x=0x%x\n",
			g_backlight_ic_name, last_level, level,
			pchip->bl_config.bl_ocp_flag_cmd.cmd_reg, val);
		dsm_client_notify(lcd_dclient, DSM_LCD_BACKLIGHT_OCP_ERROR_NO);
	}
}

static void lcd_kit_backlight_ic_tsd_check(struct lcd_kit_backlight_device *pchip,
	int last_level, int level)
{
	unsigned char val = 0;
	int ret;

	if (pchip == NULL) {
		LCD_KIT_ERR("input parameter is NULL\n");
		return;
	}
	if (!pchip->bl_config.bl_tsd_flag_cmd.cmd_mask)
		return;
	ret = lcd_kit_backlight_read_byte(pchip->client,
		pchip->bl_config.bl_tsd_flag_cmd.cmd_reg, &val);
	if (ret < 0) {
		LCD_KIT_ERR("read backlight ic tsd reg fail\n");
		return;
	}
	if ((val & pchip->bl_config.bl_tsd_flag_cmd.cmd_mask) ==
		pchip->bl_config.bl_tsd_flag_cmd.cmd_val) {
		LCD_KIT_INFO("backlight ic check tsd fault val is 0x%x\n", val);
		if (lcd_dclient == NULL)
			return;
		ret = dsm_client_ocuppy(lcd_dclient);
		if (ret != 0) {
			LCD_KIT_ERR("dsm_client_ocuppy fail, ret=%d\n", ret);
			return;
		}
		dsm_client_record(lcd_dclient,
			"%s : last_bkl:%d, cur_bkl:%d, reg val 0x%x=0x%x\n",
			g_backlight_ic_name, last_level, level,
			pchip->bl_config.bl_tsd_flag_cmd.cmd_reg, val);
		dsm_client_notify(lcd_dclient, DSM_LCD_BACKLIGHT_TSD_ERROR_NO);
	}
}

static void dync_dimming_config_sub(int bl_dimming)
{
	int ret = -1;
	LCD_KIT_INFO("config bl dimming reg[%d]=0x%x\n",
		g_pbacklight_dev->bl_config.dimming_config_reg, bl_dimming);

	ret = lcd_kit_backlight_write_byte(g_pbacklight_dev->client,
		g_pbacklight_dev->bl_config.dimming_config_reg, bl_dimming);
	if (ret < 0)
		LCD_KIT_ERR("dync dimming config error\n");
	if (g_pbacklight_dev->bl_config.dual_ic) {
		ret = dync_dimming_config_dual_ic(g_pbacklight_dev->bl_config.dimming_config_reg, bl_dimming);
		if (ret < 0)
			LCD_KIT_ERR("dync dual dimming config error\n");
		LCD_KIT_INFO("backlight icdync dual dimming config val is reg = 0x%x, avl = 0x%x\n",
			g_pbacklight_dev->bl_config.dimming_config_reg, bl_dimming);
	}
}

static void dync_dimming_config(unsigned int bl_level)
{
	static int bl_work_state = LCD_BACKLIGHT_POWER_ON;
	if (g_pbacklight_dev->bl_config.bl_dimming_support != LCD_BACKLIGHT_DIMMING_SUPPORT)
		return;
	if (bl_level == 0) {
		if (g_pbacklight_dev->bl_config.bl_dimming_resume != 0)
			dync_dimming_config_sub(g_pbacklight_dev->bl_config.bl_dimming_resume);
		bl_work_state = LCD_BACKLIGHT_POWER_OFF;
		return;
	}
	if (bl_work_state == LCD_BACKLIGHT_POWER_OFF) {
		bl_work_state = LCD_BACKLIGHT_POWER_ON;
	} else if (bl_work_state == LCD_BACKLIGHT_POWER_ON) {
		if (g_pbacklight_dev->bl_config.bl_dimming_config != 0)
			dync_dimming_config_sub(g_pbacklight_dev->bl_config.bl_dimming_config);
		bl_work_state = LCD_BACKLIGHT_LEVEL_POWER_ON;
	}
}
static void lcd_kit_backlight_ic_fault_check(struct lcd_kit_backlight_device *pchip,
	int level)
{
	static int last_level = -1;

	if (pchip == NULL) {
		LCD_KIT_ERR("input parameter is NULL\n");
		return;
	}

	if (pchip->bl_config.fault_check_enable &&
		((last_level <= 0 && level != 0) ||
		(last_level > 0 && level == 0))) {
		LCD_KIT_INFO("start backlight ic fault check\n");
		lcd_kit_backlight_ic_ovp_check(pchip, last_level, level);
		lcd_kit_backlight_ic_ocp_check(pchip, last_level, level);
		lcd_kit_backlight_ic_tsd_check(pchip, last_level, level);
	}
	last_level = level;
}
#endif

static unsigned int lcd_kit_backlight_get_level(unsigned int bl_level, unsigned int bl_max_level)
{
	unsigned int level;

	if (g_pbacklight_dev->bl_config.bl_10000_support) {
		level = (unsigned int)bl_lvl_map(bl_level, bl_max_level);
	} else {
		if (g_pbacklight_dev->bl_config.bits_compatible && bl_max_level > 0)
			bl_level = bl_level * g_pbacklight_dev->bl_config.bl_level / bl_max_level;
		level = bl_level;
		if (level > g_pbacklight_dev->bl_config.bl_level)
			level = g_pbacklight_dev->bl_config.bl_level;
	}

	return level;
}

static void lcd_kit_backlight_get_lsb_msb(unsigned int level, unsigned char *bl_msb,
	unsigned char *bl_lsb)
{
	/*
	* This is a rt8555 IC bug, when bl level is 0 or 0x0FF,
	* bl level must add or sub 1, or flickering
	*/
	if (!strcmp(g_backlight_ic_name, "hw_rt8555")) {
		if ((level != 0) && ((level & 0xF) == 0))
			level += 1;
		if ((level & 0xFF) == 0xFF)
			level -= 1;
	} else if (!strcmp(g_backlight_ic_name, "hw_mp3314")) {
		/*
		* This is a mp3314 IC bug, when the backlight level exceeds 4000 and  below 24851
		* bl level must refit the dimming curve.
		* 0.98:Straight line slope. 497:Line intercept.
		*/
		if ((level > 4000) && (level < 24851))
			level = (unsigned int)(level * 98 / 100) + 497;
	}
	*bl_lsb = level & g_pbacklight_dev->bl_config.bl_lsb_reg_cmd.cmd_mask;
	*bl_msb = (level >> g_pbacklight_dev->bl_config.bl_lsb_reg_cmd.val_bits) &
		g_pbacklight_dev->bl_config.bl_msb_reg_cmd.cmd_mask;
}

static int lcd_kit_backlight_trans(unsigned int level)
{
	int ret = LCD_KIT_OK;
	unsigned char bl_msb;
	unsigned char bl_lsb;

	lcd_kit_backlight_get_lsb_msb(level, &bl_msb, &bl_lsb);
	LCD_KIT_INFO("level = %d, bl_msb = %d, bl_lsb = %d\n", level, bl_msb, bl_lsb);

	if ((g_pbacklight_dev->bl_config.bl_lsb_reg_cmd.val_bits != 0) &&
		(!g_pbacklight_dev->bl_config.msb_before_lsb)) {
		ret = lcd_kit_backlight_write_byte(g_pbacklight_dev->client,
			g_pbacklight_dev->bl_config.bl_lsb_reg_cmd.cmd_reg, bl_lsb);
		if (ret < 0) {
			LCD_KIT_ERR("set backlight ic brightness lsb failed!\n");
			return ret;
		}
	}
	ret = lcd_kit_backlight_write_byte(g_pbacklight_dev->client,
		g_pbacklight_dev->bl_config.bl_msb_reg_cmd.cmd_reg, bl_msb);
	if (ret < 0) {
		LCD_KIT_ERR("set backlight ic brightness msb failed!\n");
		return ret;
	}
	if (g_pbacklight_dev->bl_config.msb_before_lsb) {
		ret = lcd_kit_backlight_write_byte(g_pbacklight_dev->client,
			g_pbacklight_dev->bl_config.bl_lsb_reg_cmd.cmd_reg, bl_lsb);
		if (ret < 0) {
			LCD_KIT_ERR("set backlight ic brightness lsb failed!\n");
			return ret;
		}
	}

	if (g_pbacklight_dev->bl_config.dual_ic) {
		ret = lcd_kit_setbacklight_dual_ic(bl_lsb, bl_msb);
		if(ret < 0)
			LCD_KIT_ERR("set backlight dual_ic brightness failed!\n");
	}

	return ret;
}

int lcd_kit_set_backlight(unsigned int bl_level)
{
	int ret = LCD_KIT_FAIL;
	unsigned int level;
	int panel_id = 0;

	if (g_pbacklight_dev == NULL) {
		LCD_KIT_ERR("init fail, return\n");
		return ret;
	}
	if (down_trylock(&(g_pbacklight_dev->sem))) {
		LCD_KIT_INFO("Now in test mode\n");
		return ret;
	}
	LCD_KIT_INFO("bl_level = %d\n", bl_level);

	/* first set backlight, enable ktz8864 */
	if (g_pbacklight_dev->bl_config.bl_only != 0) {
		if ((g_backlight_init_status == false) && (bl_level > 0)) {
			lcd_kit_backlight_enable();
		} else if ((g_backlight_init_status == false) && (bl_level == 0)) {
			up(&(g_pbacklight_dev->sem));
			return LCD_KIT_OK;
		}
	}

	level = lcd_kit_backlight_get_level(bl_level, common_info->bl_level_max);
	if (g_pbacklight_dev->bl_config.bl_ctrl_mod == BL_PWM_I2C_MODE) {
		LCD_KIT_INFO("backlight ic ctrl by mipi pwm, level is %u\n", level);
#ifdef CONFIG_HUAWEI_DSM
		lcd_kit_backlight_ic_fault_check(g_pbacklight_dev, level);
#endif
		up(&(g_pbacklight_dev->sem));
		return LCD_KIT_OK;
	}
#ifdef CONFIG_HUAWEI_DSM
	dync_dimming_config(bl_level);
#endif

	ret = lcd_kit_backlight_trans(level);
	if (ret < 0) {
		up(&(g_pbacklight_dev->sem));
		LCD_KIT_ERR("lcd kit set backlight exit fail\n");
		return ret;
	}

#ifdef CONFIG_HUAWEI_DSM
	lcd_kit_backlight_ic_fault_check(g_pbacklight_dev, level);
#endif
	/* if set backlight level 0, disable ic */
	if (g_pbacklight_dev->bl_config.bl_only != 0) {
		if (g_backlight_init_status == true && bl_level == 0)
			lcd_kit_backlight_disable();
	}
	up(&(g_pbacklight_dev->sem));
	LCD_KIT_INFO("lcd kit set backlight exit succ\n");
	return ret;
}

static struct lcd_kit_bl_ops bl_ops = {
	.set_backlight = lcd_kit_set_backlight,
	.name = "bl_common",
};

static struct lcd_kit_bias_ops bias_ops = {
	.set_bias_voltage = lcd_kit_backlight_set_bias,
	.set_bias_power_down = lcd_kit_backlight_set_bias_power_down,
	.set_ic_disable = lcd_kit_backlight_set_ic_disable,
};

#ifdef LCD_KIT_DEBUG_ENABLE
static int lcd_kit_backlight_probe_create_sys(struct i2c_client *client)
{
	int ret = 0;

	if (!client || !g_pbacklight_dev) {
		LCD_KIT_ERR("input parameter is NULL\n");
		return -1;
	}
	if (g_backlight_class != NULL) {
		g_pbacklight_dev->dev = device_create(g_backlight_class,
			NULL, 0, "%s", g_backlight_ic_name);
		if (IS_ERR(g_pbacklight_dev->dev)) {
			LCD_KIT_ERR("Unable to create device; errno = %ld\n", PTR_ERR(g_pbacklight_dev->dev));
			g_pbacklight_dev->dev = NULL;
			g_sysfs_created = BACKLIGHT_FALSE;
			return -EINVAL;
		} else {
			dev_set_drvdata(g_pbacklight_dev->dev, g_pbacklight_dev);
			ret = sysfs_create_group(&g_pbacklight_dev->dev->kobj, &backlight_group);
			if (ret < 0) {
				LCD_KIT_ERR("Create backlight sysfs group node failed!\n");
				device_destroy(g_backlight_class, 0);
				g_sysfs_created = BACKLIGHT_FALSE;
				return ret;
			}
			g_sysfs_created = BACKLIGHT_TRUE;
		}
	}
	return ret;
}
#endif

static int lcd_kit_backlight_probe_dts_config(struct i2c_client *client)
{
	int ret;

	if (client == NULL) {
		LCD_KIT_ERR("client is null\n");
		return -EINVAL;
	}

	g_pbacklight_dev->client = client;
	if (client->dev.of_node == NULL) {
		LCD_KIT_ERR("of_node is NULL\n");
		return -EINVAL;
	}
	if (client->dev.of_node != NULL)
		bl_ops.name = (char *)of_get_property(client->dev.of_node, "bl_type_string", NULL);
	lcd_kit_backlight_parse_dts(client->dev.of_node, &g_pbacklight_dev->bl_config);
	sema_init(&(g_pbacklight_dev->sem), 1);

	if (g_pbacklight_dev->bl_config.dual_ic) {
		ret = lcd_kit_backlight_dual_ic_i2c_adapter();
		if (ret < 0) 
			LCD_KIT_ERR("%s dual_ic_i2c_adapter fail", __func__);
	}
	if (g_pbacklight_dev->bl_config.hw_en) {
		ret = gpio_request(g_pbacklight_dev->bl_config.hw_en_gpio, NULL);
		if (ret < 0)
			LCD_KIT_ERR("can not request hw_en_gpio\n");
		if ((g_pbacklight_dev->bl_config.dual_hw_en_gpio) && 
		    (g_pbacklight_dev->bl_config.dual_hw_en_gpio != g_pbacklight_dev->bl_config.hw_en_gpio)) {
			ret = gpio_request(g_pbacklight_dev->bl_config.dual_hw_en_gpio, NULL);
			if (ret)
				LCD_KIT_ERR("can not request dual_hw_en_gpio\n");
		}
	}

	return ret;
}

static int lcd_kit_backlight_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
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
	g_pbacklight_dev = devm_kzalloc(&client->dev, sizeof(struct lcd_kit_backlight_device), GFP_KERNEL);
	if (g_pbacklight_dev == NULL) {
		LCD_KIT_ERR("Failed to allocate memory\n");
		return -ENOMEM;
	}

	ret = lcd_kit_backlight_probe_dts_config(client);
	if (ret < 0)
		goto failed;

#ifdef LCD_KIT_DEBUG_ENABLE
	ret = lcd_kit_backlight_probe_create_sys(client);
	if (ret < 0)
		goto failed;
#endif

	if (g_pbacklight_dev->bl_config.bl_only != 0) {
		lcd_kit_bl_register(&bl_ops);
	} else {
		lcd_kit_bias_register(&bias_ops);
		lcd_kit_bl_register(&bl_ops);
	}
	g_backlight_init_status = true;
	LCD_KIT_INFO("exit\n");
	return LCD_KIT_OK;

failed:
	devm_kfree(&client->dev, g_pbacklight_dev);
	g_pbacklight_dev = NULL;
	return ret;
}

static int lcd_kit_backlight_remove(struct i2c_client *client)
{
	if (g_pbacklight_dev == NULL) {
		LCD_KIT_ERR("g_pbacklight_dev is null\n");
		return -EINVAL;
	}
	if (client != NULL) {
		devm_kfree(&client->dev, g_pbacklight_dev);
		g_pbacklight_dev = NULL;
	}

#ifdef LCD_KIT_DEBUG_ENABLE
	if (g_sysfs_created)
		sysfs_remove_group(&client->dev.kobj, &backlight_group);

	if (g_pbacklight_dev->dev != NULL)
		device_destroy(g_backlight_class, 0);

	if (g_backlight_class != NULL) {
		class_destroy(g_backlight_class);
		g_backlight_class = NULL;
	}
#endif

	return LCD_KIT_OK;
}

static struct i2c_device_id backlight_ic_id[] = {
	{ "bl_ic_common", 0 },
	{},
};

MODULE_DEVICE_TABLE(i2c, backlight_ic_id);

static struct of_device_id lcd_kit_backlight_match_table[] = {
	{ .compatible = "bl_ic_common", },
	{},
};

static struct i2c_driver lcd_kit_backlight_driver = {
	.probe = lcd_kit_backlight_probe,
	.remove = lcd_kit_backlight_remove,
	.driver = {
		.name = "bl_ic_common",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(lcd_kit_backlight_match_table),
	},
	.id_table = backlight_ic_id,
};

static void lcd_kit_get_backlight_ic_name(void)
{
	struct device_node *np = NULL;
	char *bl_type = NULL;
	char *panel_type = NULL;
	int len;

	np = of_find_compatible_node(NULL, NULL, "huawei,lcd_panel_type");
	if (!np) {
		LCD_KIT_ERR("NOT FOUND device node %s!\n", "huawei,lcd_panel_type");
		return;
	}

	bl_type = (char *)of_get_property(np, "lcd_bl_type", NULL);
	if (bl_type) {
		len = strlen(bl_type);
		if (len > LCD_BACKLIGHT_IC_NAME_LEN - 1)
			len = LCD_BACKLIGHT_IC_NAME_LEN - 1;
		if (strncpy_s(g_backlight_ic_name, LCD_BACKLIGHT_IC_NAME_LEN, bl_type, len) != EOK) {
			LCD_KIT_ERR("strncpy_s fail\n");
			return;
		}
		g_backlight_ic_name[LCD_BACKLIGHT_IC_NAME_LEN - 1] = 0;
	}

	panel_type = (char *)of_get_property(np, "lcd_panel_type", NULL);
	if (panel_type) {
		len = strlen(panel_type);
		if (len > LCD_PANEL_IC_NAME_LEN - 1)
			len = LCD_PANEL_IC_NAME_LEN - 1;
		if (strncpy_s(g_panel_ic_name, LCD_PANEL_IC_NAME_LEN, panel_type, len) != EOK) {
			LCD_KIT_ERR("strncpy_s fail\n");
			return;
		}
		g_panel_ic_name[LCD_PANEL_IC_NAME_LEN - 1] = 0;
	}
}

static int __init lcd_kit_backlight_init(void)
{
	char compatible_name[LCD_BACKLIGHT_IC_COMP_LEN] = {0};
	int len;

	lcd_kit_get_backlight_ic_name();
	LCD_KIT_INFO("g_backlight_ic_name is %s\n", g_backlight_ic_name);
	if (!strcmp(g_backlight_ic_name, "default"))
		return LCD_KIT_OK;

	if (memcpy_s(backlight_ic_id[0].name, LCD_BACKLIGHT_IC_NAME_LEN,
		g_backlight_ic_name, LCD_BACKLIGHT_IC_NAME_LEN) != EOK) {
		LCD_KIT_ERR("memcpy_s fail\n");
		return LCD_KIT_FAIL;
	}
	lcd_kit_backlight_driver.driver.name = g_backlight_ic_name;
	if (snprintf_s(compatible_name, sizeof(compatible_name), sizeof(compatible_name) - 1,
		"huawei,%s", g_backlight_ic_name) < 0) {
		LCD_KIT_ERR("snprintf_s fail\n");
		return LCD_KIT_FAIL;
	}
	len = strlen(compatible_name);
	if (len >= LCD_BACKLIGHT_IC_COMP_LEN)
		len = LCD_BACKLIGHT_IC_COMP_LEN - 1;
	if (memcpy_s(lcd_kit_backlight_match_table[0].compatible, LCD_BACKLIGHT_IC_COMP_LEN - 1,
		compatible_name, len)!= EOK) {
		LCD_KIT_ERR("memcpy_s fail\n");
		return LCD_KIT_FAIL;
	}
	lcd_kit_backlight_match_table[0].compatible[LCD_BACKLIGHT_IC_COMP_LEN - 1] = 0;

#ifdef LCD_KIT_DEBUG_ENABLE
	g_backlight_class = class_create(THIS_MODULE, "lcd_backlight");
	if (IS_ERR(g_backlight_class)) {
		LCD_KIT_ERR("Unable to create backlight class, errno = %ld\n", PTR_ERR(g_backlight_class));
		g_backlight_class = NULL;
	}
#endif

	return i2c_add_driver(&lcd_kit_backlight_driver);
}

static void __exit lcd_kit_backlight_exit(void)
{
#ifdef LCD_KIT_DEBUG_ENABLE
	if (g_backlight_class != NULL) {
		class_destroy(g_backlight_class);
		g_backlight_class = NULL;
	}
#endif

	if (g_backlight_dual_ic_adap != NULL) {
		i2c_put_adapter(g_backlight_dual_ic_adap);
		g_backlight_dual_ic_adap = NULL;
	}

	i2c_del_driver(&lcd_kit_backlight_driver);
}

late_initcall(lcd_kit_backlight_init);
module_exit(lcd_kit_backlight_exit);

MODULE_AUTHOR("Huawei Technologies Co., Ltd");
MODULE_LICENSE("GPL");

