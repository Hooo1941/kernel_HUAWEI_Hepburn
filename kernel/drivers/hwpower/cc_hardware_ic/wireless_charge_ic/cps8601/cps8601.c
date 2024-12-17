// SPDX-License-Identifier: GPL-2.0
/*
 * cps8601.c
 *
 * cps8601 driver
 *
 * Copyright (c) 2022-2022 Huawei Technologies Co., Ltd.
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

#include "cps8601.h"

#define HWLOG_TAG wireless_cps8601
HWLOG_REGIST();

static int cps8601_i2c_read(struct cps8601_dev_info *di, u8 *cmd, int cmd_len, u8 *dat, int dat_len)
{
	int i;

	for (i = 0; i < WLTRX_IC_I2C_RETRY_CNT; i++) {
		if (!power_i2c_read_block(di->client, cmd, cmd_len, dat, dat_len))
			return 0;
		power_usleep(DT_USLEEP_10MS); /* delay for i2c retry */
	}

	return -EIO;
}

static int cps8601_i2c_write(struct cps8601_dev_info *di, u8 *cmd, int cmd_len)
{
	int i;

	for (i = 0; i < WLTRX_IC_I2C_RETRY_CNT; i++) {
		if (!power_i2c_write_block(di->client, cmd, cmd_len))
			return 0;
		power_usleep(DT_USLEEP_10MS); /* delay for i2c retry */
	}

	return -EIO;
}

int cps8601_read_block(struct cps8601_dev_info *di, u16 reg, u8 *data, u8 len)
{
	u8 cmd[CPS8601_ADDR_LEN];

	if (!di || !data) {
		hwlog_err("read_block: para null\n");
		return -EINVAL;
	}

	cmd[0] = reg >> POWER_BITS_PER_BYTE;
	cmd[1] = reg & POWER_MASK_BYTE;

	return cps8601_i2c_read(di, cmd, CPS8601_ADDR_LEN, data, len);
}

int cps8601_write_block(struct cps8601_dev_info *di, u16 reg, u8 *data, u8 len)
{
	int ret;
	u8 *cmd = NULL;

	if (!di || !data) {
		hwlog_err("write_block: para null\n");
		return -EINVAL;
	}
	cmd = kzalloc(sizeof(u8) * (CPS8601_ADDR_LEN + len), GFP_KERNEL);
	if (!cmd)
		return -ENOMEM;

	cmd[0] = reg >> POWER_BITS_PER_BYTE;
	cmd[1] = reg & POWER_MASK_BYTE;
	memcpy(&cmd[CPS8601_ADDR_LEN], data, len);

	ret = cps8601_i2c_write(di, cmd, CPS8601_ADDR_LEN + len);
	kfree(cmd);
	return ret;
}

int cps8601_hw_read_block(struct cps8601_dev_info *di, u32 addr, u8 *data, u8 len)
{
	u8 cmd[CPS8601_HW_ADDR_LEN];

	if (!di || !data) {
		hwlog_err("hw_read_block: para null\n");
		return -EINVAL;
	}

	/* 1dword=4bytes, 1byte=8bits 0xff: byte mask */
	cmd[0] = (u8)((addr >> 24) & 0xff);
	cmd[1] = (u8)((addr >> 16) & 0xff);
	cmd[2] = (u8)((addr >> 8) & 0xff);
	cmd[3] = (u8)((addr >> 0) & 0xff);

	return cps8601_i2c_read(di, cmd, CPS8601_HW_ADDR_LEN, data, len);
}

int cps8601_hw_write_block(struct cps8601_dev_info *di, u32 addr, u8 *data, u8 data_len)
{
	int ret;
	u8 *cmd = NULL;

	if (!di || !data) {
		hwlog_err("hw_write_block: para null\n");
		return -EINVAL;
	}

	cmd = kzalloc(sizeof(u8) * (CPS8601_HW_ADDR_LEN + data_len), GFP_KERNEL);
	if (!cmd)
		return -ENOMEM;

	/* 1dword=4bytes, 1byte=8bits 0xff: byte mask */
	cmd[0] = (u8)((addr >> 24) & 0xff);
	cmd[1] = (u8)((addr >> 16) & 0xff);
	cmd[2] = (u8)((addr >> 8) & 0xff);
	cmd[3] = (u8)((addr >> 0) & 0xff);
	memcpy(&cmd[CPS8601_HW_ADDR_LEN], data, data_len);

	ret = cps8601_i2c_write(di, cmd, CPS8601_HW_ADDR_LEN + data_len);
	kfree(cmd);
	return ret;
}

int cps8601_read_byte(struct cps8601_dev_info *di, u16 reg, u8 *data)
{
	return cps8601_read_block(di, reg, data, POWER_BYTE_LEN);
}

int cps8601_read_word(struct cps8601_dev_info *di, u16 reg, u16 *data)
{
	u8 buff[POWER_WORD_LEN] = { 0 };

	if (!data || cps8601_read_block(di, reg, buff, POWER_WORD_LEN))
		return -EINVAL;

	*data = buff[0] | (buff[1] << POWER_BITS_PER_BYTE);
	return 0;
}

int cps8601_write_byte(struct cps8601_dev_info *di, u16 reg, u8 data)
{
	return cps8601_write_block(di, reg, &data, POWER_BYTE_LEN);
}

int cps8601_write_word(struct cps8601_dev_info *di, u16 reg, u16 data)
{
	u8 buff[POWER_WORD_LEN];

	buff[0] = data & POWER_MASK_BYTE;
	buff[1] = data >> POWER_BITS_PER_BYTE;

	return cps8601_write_block(di, reg, buff, POWER_WORD_LEN);
}

int cps8601_write_word_mask(struct cps8601_dev_info *di, u16 reg, u16 mask, u16 shift, u16 data)
{
	int ret;
	u16 val = 0;

	ret = cps8601_read_word(di, reg, &val);
	if (ret)
		return ret;

	val &= ~mask;
	val |= ((data << shift) & mask);

	return cps8601_write_word(di, reg, val);
}

int cps8601_hw_read_dword(struct cps8601_dev_info *di, u32 addr, u32 *data)
{
	u8 buff[POWER_DWORD_LEN] = { 0 };

	if (!data || cps8601_hw_read_block(di, addr, buff, POWER_DWORD_LEN))
		return -EINVAL;

	*data = buff[0] | (buff[1] << 8) | (buff[2] << 16) | (buff[3] << 24);
	return 0;
}

int cps8601_hw_write_dword(struct cps8601_dev_info *di, u32 addr, u32 data)
{
	return cps8601_hw_write_block(di, addr, (u8 *)&data, POWER_DWORD_LEN);
}

int cps8601_get_chip_id(struct cps8601_dev_info *di, u16 *chip_id)
{
	int ret;

	ret = cps8601_read_word(di, CPS8601_CHIP_ID_ADDR, chip_id);
	hwlog_info("[get_chip_id] chipid=0x%x, ret=%d\n", *chip_id, ret);
	if (ret)
		return ret;

	*chip_id = CPS8601_CHIP_ID;
	return 0;
}

static int cps8601_get_mtp_version(struct cps8601_dev_info *di, u16 *mtp_ver)
{
	return cps8601_read_word(di, CPS8601_MTP_VER_ADDR, mtp_ver);
}

int cps8601_get_chip_info(struct cps8601_dev_info *di, struct cps8601_chip_info *info)
{
	int ret;

	if (!info || !di)
		return -EINVAL;

	ret = cps8601_get_chip_id(di, &info->chip_id);
	ret += cps8601_get_mtp_version(di, &info->mtp_ver);
	if (ret) {
		hwlog_err("get_chip_info: failed\n");
		return ret;
	}

	return 0;
}

int cps8601_get_chip_info_str(struct cps8601_dev_info *di, char *info_str, int len)
{
	int ret;
	struct cps8601_chip_info chip_info = { 0 };

	if (!info_str || (len != CPS8601_CHIP_INFO_STR_LEN))
		return -EINVAL;

	ret = cps8601_get_chip_info(di, &chip_info);
	if (ret)
		return ret;

	memset(info_str, 0, CPS8601_CHIP_INFO_STR_LEN);

	return snprintf(info_str, len, "chip_id:0x%04x mtp_ver:0x%04x",
		chip_info.chip_id, chip_info.mtp_ver);
}

int cps8601_get_mode(struct cps8601_dev_info *di, u8 *mode)
{
	return cps8601_read_byte(di, CPS8601_OP_MODE_ADDR, mode);
}

void cps8601_enable_irq(struct cps8601_dev_info *di)
{
	if (!di)
		return;

	mutex_lock(&di->mutex_irq);
	if (!di->irq_active) {
		hwlog_info("[enable_irq] ++\n");
		enable_irq(di->irq_int);
		di->irq_active = true;
	}
	hwlog_info("[enable_irq] --\n");
	mutex_unlock(&di->mutex_irq);
}

void cps8601_disable_irq_nosync(struct cps8601_dev_info *di)
{
	if (!di)
		return;

	mutex_lock(&di->mutex_irq);
	if (di->irq_active) {
		hwlog_info("[disable_irq_nosync] ++\n");
		disable_irq_nosync(di->irq_int);
		di->irq_active = false;
	}
	hwlog_info("[disable_irq_nosync] --\n");
	mutex_unlock(&di->mutex_irq);
}

void cps8601_chip_enable(bool enable, void *dev_data)
{
}

static void cps8601_irq_work(struct work_struct *work)
{
	int ret;
	u8 mode = 0;
	struct cps8601_dev_info *di = NULL;

	if (!work)
		return;

	di = container_of(work, struct cps8601_dev_info, irq_work);
	if (!di)
		goto exit;

	/* get System Operating Mode */
	ret = cps8601_get_mode(di, &mode);
	if (!ret)
		hwlog_info("[irq_work] mode=0x%x\n", mode);

	/* handler irq */
	if ((mode == CPS8601_OP_MODE_TX) || (mode == CPS8601_OP_MODE_BP))
		cps8601_tx_mode_irq_handler(di);
	else
		hwlog_info("[irq_work] mode=0x%x\n", mode);

exit:
	if (di) {
		cps8601_enable_irq(di);
		power_wakeup_unlock(di->wakelock, false);
	}
}

static irqreturn_t cps8601_interrupt(int irq, void *_di)
{
	struct cps8601_dev_info *di = _di;

	if (!di)
		return IRQ_HANDLED;

	power_wakeup_lock(di->wakelock, false);
	hwlog_info("[interrupt] ++\n");
	if (di->irq_active) {
		disable_irq_nosync(di->irq_int);
		di->irq_active = false;
		schedule_work(&di->irq_work);
	} else {
		hwlog_info("[interrupt] irq is not enable\n");
		power_wakeup_unlock(di->wakelock, false);
	}
	hwlog_info("[interrupt] --\n");

	return IRQ_HANDLED;
}

static int cps8601_dev_check(struct cps8601_dev_info *di)
{
	int ret;
	u16 chip_id = 0;

	wlps_control(di->ic_type, WLPS_TX_PWR_SW, true);
	power_usleep(DT_USLEEP_10MS); /* delay for power on */
	ret = cps8601_get_chip_id(di, &chip_id);
	if (ret) {
		hwlog_err("dev_check: failed\n");
		wlps_control(di->ic_type, WLPS_TX_PWR_SW, false);
		return ret;
	}
	wlps_control(di->ic_type, WLPS_TX_PWR_SW, false);

	di->chip_id = chip_id;
	hwlog_info("[dev_check] chip_id=0x%04x\n", chip_id);
	if (chip_id != CPS8601_CHIP_ID)
		hwlog_err("dev_check: rx_chip not match\n");

	return 0;
}

struct device_node *cps8601_dts_dev_node(void *dev_data)
{
	struct cps8601_dev_info *di = dev_data;

	if (!di || !di->dev)
		return NULL;

	return di->dev->of_node;
}

static int cps8601_gpio_init(struct cps8601_dev_info *di, struct device_node *np)
{
	if (di->default_psy_type != CPS8601_DEFAULT_LOWPOWER)
		return 0;

	if (power_gpio_config_output(np, "gpio_rx_online", "gpio_rx_online",
		&di->gpio_rx_online, 0))
		return -EINVAL;

	if (power_gpio_config_output(np, "gpio_lowpower", "gpio_lowpower",
		&di->gpio_lowpower, 1)) {
		gpio_free(di->gpio_rx_online);
		return -EINVAL;
	}

	return 0;
}

static int cps8601_irq_init(struct cps8601_dev_info *di, struct device_node *np)
{
	if (power_gpio_config_interrupt(np, "gpio_int", "cps8601_int",
		&di->gpio_int, &di->irq_int))
		return -EINVAL;

	if (request_irq(di->irq_int, cps8601_interrupt, IRQF_TRIGGER_FALLING, "cps8601_irq", di)) {
		hwlog_err("irq_init: request cps8601_irq failed\n");
		gpio_free(di->gpio_int);
		return -EINVAL;
	}

	enable_irq_wake(di->irq_int);
	di->irq_active = true;
	INIT_WORK(&di->irq_work, cps8601_irq_work);

	return 0;
}

static void cps8601_register_pwr_dev_info(struct cps8601_dev_info *di)
{
	struct power_devices_info_data *pwr_dev_info = NULL;

	pwr_dev_info = power_devices_info_register();
	if (pwr_dev_info) {
		pwr_dev_info->dev_name = di->dev->driver->name;
		pwr_dev_info->dev_id = di->chip_id;
		pwr_dev_info->ver_id = 0;
	}
}

static void cps8601_ops_unregister(struct wltrx_ic_ops *ops)
{
	if (!ops)
		return;

	if (ops->tx_ops)
		kfree(ops->tx_ops);
	if (ops->fw_ops)
		kfree(ops->fw_ops);
	if (ops->qi_ops)
		kfree(ops->qi_ops);
	kfree(ops);
}

static void cps8601_free_dev_resources(struct cps8601_dev_info *di)
{
	power_wakeup_source_unregister(di->wakelock);
	mutex_destroy(&di->mutex_irq);
	gpio_free(di->gpio_int);
	free_irq(di->irq_int, di);
	if (di->default_psy_type == CPS8601_DEFAULT_LOWPOWER) {
		gpio_free(di->gpio_rx_online);
		gpio_free(di->gpio_lowpower);
	}
}

static int cps8601_request_dev_resources(struct cps8601_dev_info *di, struct device_node *np)
{
	int ret;
	u32 pinctrl_len = (di->default_psy_type == CPS8601_DEFAULT_LOWPOWER) ?
		CPS8601_Q_PINCTRL_LEN : CPS8601_PINCTRL_LEN;

	ret = cps8601_parse_dts(np, di);
	if (ret)
		goto parse_dts_fail;

	ret = power_pinctrl_config(di->dev, "pinctrl-names", pinctrl_len);
	if (ret)
		goto pinctrl_config_fail;

	ret = cps8601_gpio_init(di, np);
	if (ret)
		goto gpio_init_fail;

	ret = cps8601_irq_init(di, np);
	if (ret)
		goto irq_init_fail;

	di->wakelock = power_wakeup_source_register(di->dev, "cps8601_wakelock");
	mutex_init(&di->mutex_irq);

	return 0;

irq_init_fail:
	if (di->default_psy_type == CPS8601_DEFAULT_LOWPOWER) {
		gpio_free(di->gpio_rx_online);
		gpio_free(di->gpio_lowpower);
	}
gpio_init_fail:
pinctrl_config_fail:
parse_dts_fail:
	return ret;
}

static int cps8601_ops_register(struct wltrx_ic_ops *ops, struct cps8601_dev_info *di)
{
	int ret;

	ret = cps8601_fw_ops_register(ops, di);
	if (ret) {
		hwlog_err("ops_register: register fw_ops failed\n");
		return ret;
	}

	ret = cps8601_tx_ops_register(ops, di);
	if (ret) {
		hwlog_err("ops_register: register tx_ops failed\n");
		return ret;
	}

	ret = cps8601_qi_ops_register(ops, di);
	if (ret) {
		hwlog_err("ops_register: register qi_ops failed\n");
		return ret;
	}
	di->g_val.qi_hdl = hwqi_get_handle();

	return 0;
}

static void cps8601_fw_mtp_check(struct cps8601_dev_info *di)
{
	if (power_cmdline_is_powerdown_charging_mode()) {
		di->g_val.mtp_chk_complete = true;
		return;
	}

	INIT_DELAYED_WORK(&di->mtp_check_work, cps8601_fw_mtp_check_work);
	schedule_delayed_work(&di->mtp_check_work, msecs_to_jiffies(WIRELESS_FW_WORK_DELAYED_TIME));
}

static int cps8601_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int ret;
	struct cps8601_dev_info *di = NULL;
	struct device_node *np = NULL;
	struct wltrx_ic_ops *ops = NULL;

	if (!client || !client->dev.of_node)
		return -ENODEV;

	if (wltx_ic_is_ops_registered(id->driver_data))
		return 0;

	di = devm_kzalloc(&client->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;
	ops = kzalloc(sizeof(*ops), GFP_KERNEL);
	if (!ops) {
		devm_kfree(&client->dev, di);
		return -ENOMEM;
	}

	di->dev = &client->dev;
	np = client->dev.of_node;
	di->client = client;
	di->ic_type = id->driver_data;
	i2c_set_clientdata(client, di);

	ret = cps8601_dev_check(di);
	if (ret) {
		ret = -EPROBE_DEFER;
		goto dev_ck_fail;
	}

	ret = cps8601_request_dev_resources(di, np);
	if (ret)
		goto req_dev_res_fail;

	ret = cps8601_ops_register(ops, di);
	if (ret)
		goto ops_regist_fail;

	cps8601_fw_mtp_check(di);
	cps8601_register_pwr_dev_info(di);

	hwlog_info("[ic_type:%u]wireless_chip probe ok\n", di->ic_type);
	return 0;

ops_regist_fail:
	cps8601_free_dev_resources(di);
req_dev_res_fail:
dev_ck_fail:
	cps8601_ops_unregister(ops);
	devm_kfree(&client->dev, di);
	return ret;
}

#ifdef CONFIG_PM
static int cps8601_suspend(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct cps8601_dev_info *di = NULL;

	hwlog_info("[suspend] begin\n");

	if (!client)
		return 0;

	di = i2c_get_clientdata(client);
	if (!di)
		return 0;
	if (di->default_psy_type != CPS8601_DEFAULT_LOWPOWER)
		return 0;

	gpio_set_value(di->gpio_lowpower, CPS8601_Q_LONG_PERIOD);

	hwlog_info("[suspend] end\n");
	return 0;
}

static int cps8601_resume(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct cps8601_dev_info *di = NULL;

	hwlog_info("[resume] begin\n");

	if (!client)
		return 0;

	di = i2c_get_clientdata(client);
	if (!di)
		return 0;
	if (di->default_psy_type != CPS8601_DEFAULT_LOWPOWER)
		return 0;

	gpio_set_value(di->gpio_lowpower, CPS8601_Q_SHORT_PERIOD);

	hwlog_info("[resume] end\n");
	return 0;
}

static const struct dev_pm_ops cps8601_pm_ops = {
	.suspend = cps8601_suspend,
	.resume = cps8601_resume,
};

#define CPS8601_PM_OPS (&cps8601_pm_ops)
#else
#define CPS8601_PM_OPS (NULL)
#endif /* CONFIG_PM */

static void cps8601_shutdown(struct i2c_client *client)
{
	hwlog_info("[shutdown]\n");
}

MODULE_DEVICE_TABLE(i2c, wireless_cps8601);
static const struct i2c_device_id cps8601_i2c_id[] = {
	{ "cps8601", WLTRX_IC_MAIN },
	{ "cps8601_aux", WLTRX_IC_AUX },
	{}
};

static struct i2c_driver cps8601_driver = {
	.probe = cps8601_probe,
	.shutdown = cps8601_shutdown,
	.id_table = cps8601_i2c_id,
	.driver = {
		.owner = THIS_MODULE,
		.name = "wireless_cps8601",
		.pm = CPS8601_PM_OPS,
	},
};

static int __init cps8601_init(void)
{
	return i2c_add_driver(&cps8601_driver);
}

static void __exit cps8601_exit(void)
{
	i2c_del_driver(&cps8601_driver);
}

device_initcall(cps8601_init);
module_exit(cps8601_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("cps8601 module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
