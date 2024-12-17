// SPDX-License-Identifier: GPL-2.0
/*
 * mt5806.c
 *
 * mt5806 driver
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

#include "mt5806.h"

#define HWLOG_TAG wireless_mt5806
HWLOG_REGIST();

static int mt5806_i2c_read(struct mt5806_dev_info *di, u8 *cmd, int cmd_len, u8 *buf, int buf_len)
{
	int i;

	for (i = 0; i < WLTRX_IC_I2C_RETRY_CNT; i++) {
		if (!power_i2c_read_block(di->client, cmd, cmd_len, buf, buf_len))
			return 0;
		power_usleep(DT_USLEEP_10MS);
	}

	return -EIO;
}

static int mt5806_i2c_write(struct mt5806_dev_info *di, u8 *buf, int buf_len)
{
	int i;

	for (i = 0; i < WLTRX_IC_I2C_RETRY_CNT; i++) {
		if (!power_i2c_write_block(di->client, buf, buf_len))
			return 0;
		power_usleep(DT_USLEEP_10MS);
	}

	return -EIO;
}

int mt5806_read_block(struct mt5806_dev_info *di, u16 reg, u8 *data, u8 len)
{
	u8 cmd[MT5806_ADDR_LEN];

	if (!di || !data)
		return -EINVAL;

	cmd[0] = reg >> POWER_BITS_PER_BYTE;
	cmd[1] = reg & POWER_MASK_BYTE;

	return mt5806_i2c_read(di, cmd, MT5806_ADDR_LEN, data, len);
}

int mt5806_write_block(struct mt5806_dev_info *di, u16 reg, u8 *data, u8 data_len)
{
	int ret;
	u8 *cmd = NULL;

	if (!di || !data)
		return -EINVAL;

	cmd = kcalloc(MT5806_ADDR_LEN + data_len, sizeof(u8), GFP_KERNEL);
	if (!cmd)
		return -ENOMEM;

	cmd[0] = reg >> POWER_BITS_PER_BYTE;
	cmd[1] = reg & POWER_MASK_BYTE;
	memcpy(&cmd[MT5806_ADDR_LEN], data, data_len);

	ret = mt5806_i2c_write(di, cmd, MT5806_ADDR_LEN + data_len);
	kfree(cmd);
	return ret;
}

int mt5806_read_byte(struct mt5806_dev_info *di, u16 reg, u8 *data)
{
	return mt5806_read_block(di, reg, data, POWER_BYTE_LEN);
}

int mt5806_write_byte(struct mt5806_dev_info *di, u16 reg, u8 data)
{
	return mt5806_write_block(di, reg, &data, POWER_BYTE_LEN);
}

int mt5806_read_word(struct mt5806_dev_info *di, u16 reg, u16 *data)
{
	u8 buff[POWER_WORD_LEN] = { 0 };

	if (!data || mt5806_read_block(di, reg, buff, POWER_WORD_LEN))
		return -EINVAL;

	*data = buff[0] | buff[1] << POWER_BITS_PER_BYTE;
	return 0;
}

int mt5806_write_word(struct mt5806_dev_info *di, u16 reg, u16 data)
{
	u8 buff[POWER_WORD_LEN];

	buff[0] = data & POWER_MASK_BYTE;
	buff[1] = data >> POWER_BITS_PER_BYTE;

	return mt5806_write_block(di, reg, buff, POWER_WORD_LEN);
}

int mt5806_read_dword(struct mt5806_dev_info *di, u16 reg, u32 *data)
{
	u8 buff[POWER_DWORD_LEN] = { 0 };

	if (!data || mt5806_read_block(di, reg, buff, POWER_DWORD_LEN))
		return -EINVAL;

	/* 1dword=4bytes, 1byte=8bit */
	*data = buff[0] | (buff[1] << 8) | (buff[2] << 16) | (buff[3] << 24);
	return 0;
}

int mt5806_write_dword(struct mt5806_dev_info *di, u16 reg, u32 data)
{
	u8 buff[POWER_DWORD_LEN];

	/* 1dword=4bytes, 1byte=8bit */
	buff[0] = data & POWER_MASK_BYTE;
	buff[1] = data >> 8;
	buff[2] = data >> 16;
	buff[3] = data >> 24;

	return mt5806_write_block(di, reg, buff, POWER_DWORD_LEN);
}

int mt5806_write_dword_mask(struct mt5806_dev_info *di, u16 reg, u32 mask, u32 shift, u32 data)
{
	int ret;
	int i;
	u32 val = 0;
	u8 cmd_synmc = 0;

	for (i = 0; i < MT5806_CMD_RETRY_CNT; i++) {
		ret = mt5806_read_byte(di, MT5806_TX_CMD_SYNC_ADDR, &cmd_synmc);
		if (cmd_synmc || ret) {
			power_usleep(DT_USLEEP_500US);
		} else {
			hwlog_info("[write_dword_mask] write succ %d\n", i);
			break;
		}
	}

	ret = mt5806_read_dword(di, reg, &val);
	if (ret)
		return ret;

	val &= ~mask;
	val |= ((data << shift) & mask);
	hwlog_info("[write_dword_mask] reg = 0x%x,val = 0x%x\n", reg, val);

	ret += mt5806_write_dword(di, reg, val);
	ret += mt5806_write_byte(di, MT5806_TX_CMD_SYNC_ADDR, MT5806_TX_CMD_SYNC_VAL);

	return ret;
}

/*
 * mt5806 chip_info
 */

int mt5806_get_hw_chip_id(struct mt5806_dev_info *di, u16 *chip_id)
{
	return mt5806_read_word(di, MT5806_HW_CHIP_ID_ADDR, chip_id);
}

int mt5806_get_chip_id(struct mt5806_dev_info *di, u16 *chip_id)
{
	return mt5806_read_word(di, MT5806_CHIP_ID_ADDR, chip_id);
}

static int mt5806_get_chip_info(struct mt5806_dev_info *di, struct mt5806_chip_info *info)
{
	int ret;
	u8 chip_info[MT5806_CHIP_INFO_LEN] = { 0 };

	if (!info || !di)
		return -EINVAL;

	ret = mt5806_read_block(di, MT5806_CHIP_INFO_ADDR, chip_info, MT5806_CHIP_INFO_LEN);
	if (ret)
		return ret;

	/*
	 * addr[0:1]: chip unique id
	 * addr[2:2]: customer id
	 * addr[3:3]: hw id
	 * addr[4:5]: minor_ver
	 * addr[6:7]: major_ver
	 * 1byte equal to 8bit
	 */
	info->chip_id   = (u16)(chip_info[0] | (chip_info[1] << 8));
	info->cust_id   = chip_info[2];
	info->hw_id     = chip_info[3];
	info->minor_ver = (u16)(chip_info[4] | (chip_info[5] << 8));
	info->major_ver = (u16)(chip_info[6] | (chip_info[7] << 8));

	return 0;
}

int mt5806_get_chip_fw_version(u8 *data, int len, void *dev_data)
{
	int ret;
	struct mt5806_chip_info chip_info = { 0 };
	struct mt5806_dev_info *di = dev_data;

	/* fw version length must be 4 */
	if (!data || (len != 4)) {
		hwlog_err("get_chip_fw_version: para err\n");
		return -EINVAL;
	}

	ret = mt5806_get_chip_info(di, &chip_info);
	if (ret) {
		hwlog_err("get_chip_fw_version: get chip info failed\n");
		return ret;
	}

	/* byte[0:1]=major_fw_ver, byte[2:3]=minor_fw_ver */
	data[0] = (u8)((chip_info.major_ver >> 0) & POWER_MASK_BYTE);
	data[1] = (u8)((chip_info.major_ver >> POWER_BITS_PER_BYTE) & POWER_MASK_BYTE);
	data[2] = (u8)((chip_info.minor_ver >> 0) & POWER_MASK_BYTE);
	data[3] = (u8)((chip_info.minor_ver >> POWER_BITS_PER_BYTE) & POWER_MASK_BYTE);

	return 0;
}

int mt5806_get_mode(struct mt5806_dev_info *di, u16 *mode)
{
	int ret;

	if (!mode)
		return -EINVAL;

	ret = mt5806_read_word(di, MT5806_OP_MODE_ADDR, mode);
	if (ret) {
		hwlog_err("get_mode: failed\n");
		return ret;
	}
	*mode &= MT5806_SYS_MODE_MASK;
	return 0;
}

void mt5806_enable_irq(struct mt5806_dev_info *di)
{
	if (!di)
		return;

	hwlog_info("[enable_irq] ++, irq_active:%d\n", di->irq_active);
	mutex_lock(&di->mutex_irq);
	if (!di->irq_active) {
		enable_irq(di->irq_int);
		di->irq_active = true;
	}
	mutex_unlock(&di->mutex_irq);
	hwlog_info("[enable_irq] --\n");
}

void mt5806_disable_irq_nosync(struct mt5806_dev_info *di)
{
	if (!di)
		return;

	hwlog_info("[disable_irq_nosync] ++, irq_active:%d\n", di->irq_active);
	mutex_lock(&di->mutex_irq);
	if (di->irq_active) {
		disable_irq_nosync(di->irq_int);
		di->irq_active = false;
	}
	mutex_unlock(&di->mutex_irq);
	hwlog_info("[disable_irq_nosync] --\n");
}

void mt5806_chip_enable(bool enable, void *dev_data)
{
}

void mt5806_chip_reset(void *dev_data)
{
	int ret;
	struct mt5806_dev_info *di = dev_data;

	ret = mt5806_write_dword_mask(di, MT5806_TX_CMD_ADDR, MT5806_TX_CMD_RST_SYS,
		MT5806_TX_CMD_RST_SYS_SHIFT, MT5806_TX_CMD_VAL);
	if (ret)
		hwlog_err("chip_reset: ignore i2c failure, ret:%d\n", ret);
	else
		hwlog_info("[chip_reset] succ\n");
}

static void mt5806_irq_work(struct work_struct *work)
{
	int ret;
	u16 mode = 0;
	struct mt5806_dev_info *di = NULL;

	if (!work)
		return;

	di = container_of(work, struct mt5806_dev_info, irq_work);
	if (!di)
		goto exit;

	/* get System Operating Mode */
	ret = mt5806_get_mode(di, &mode);
	if (!ret)
		hwlog_info("[irq_work] mode=0x%x\n", mode);

	/* handler irq */
	if ((mode == MT5806_OP_MODE_TX) || (mode == MT5806_OP_MODE_SA))
		mt5806_tx_mode_irq_handler(di);
	else
		hwlog_err("irq_work: mode=0x%x\n", mode);

exit:
	if (di) {
		mt5806_enable_irq(di);
		power_wakeup_unlock(di->wakelock, false);
	}
}

static irqreturn_t mt5806_interrupt(int irq, void *_di)
{
	struct mt5806_dev_info *di = _di;

	if (!di)
		return IRQ_HANDLED;

	hwlog_info("[interrupt] ++\n");
	power_wakeup_lock(di->wakelock, false);
	if (di->irq_active) {
		disable_irq_nosync(di->irq_int);
		di->irq_active = false;
		schedule_work(&di->irq_work);
	} else {
		power_wakeup_unlock(di->wakelock, false);
		hwlog_info("interrupt: irq is not enable\n");
	}

	hwlog_info("[interrupt] --\n");

	return IRQ_HANDLED;
}

static int mt5806_dev_check(struct mt5806_dev_info *di)
{
	int ret;
	u16 chip_id = 0;

	wlps_control(di->ic_type, WLPS_TX_PWR_SW, true);
	power_usleep(DT_USLEEP_10MS); /* delay for power on */
	ret = mt5806_get_hw_chip_id(di, &chip_id);
	if (ret) {
		hwlog_err("dev_check: failed\n");
		wlps_control(di->ic_type, WLPS_TX_PWR_SW, false);
		return ret;
	}
	wlps_control(di->ic_type, WLPS_TX_PWR_SW, false);

	di->chip_id = chip_id;
	hwlog_info("[dev_check] chip_id=0x%04x\n", chip_id);
	if ((chip_id == MT5806_HW_CHIP_ID) || (chip_id == MT5806_CHIP_ID_AB))
		return 0;

	hwlog_err("dev_check: rx_chip not match\n");
	return -EINVAL;
}

struct device_node *mt5806_dts_dev_node(void *dev_data)
{
	struct mt5806_dev_info *di = dev_data;

	if (!di || !di->dev)
		return NULL;

	return di->dev->of_node;
}

static int mt5806_gpio_init(struct mt5806_dev_info *di, struct device_node *np)
{
	if (di->default_psy_type != MT5806_DEFAULT_LOWPOWER)
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

static int mt5806_irq_init(struct mt5806_dev_info *di, struct device_node *np)
{
	if (power_gpio_config_interrupt(np, "gpio_int", "mt5806_int",
		&di->gpio_int, &di->irq_int))
		return -EINVAL;

	if (request_irq(di->irq_int, mt5806_interrupt, IRQF_TRIGGER_FALLING, "mt5806_irq", di)) {
		hwlog_err("irq_init: request mt5806_irq failed\n");
		gpio_free(di->gpio_int);
		return -EINVAL;
	}

	enable_irq_wake(di->irq_int);
	di->irq_active = true;
	INIT_WORK(&di->irq_work, mt5806_irq_work);

	return 0;
}

static void mt5806_register_pwr_dev_info(struct mt5806_dev_info *di)
{
	struct power_devices_info_data *pwr_dev_info = NULL;

	pwr_dev_info = power_devices_info_register();
	if (pwr_dev_info) {
		pwr_dev_info->dev_name = di->dev->driver->name;
		pwr_dev_info->dev_id = di->chip_id;
		pwr_dev_info->ver_id = 0;
	}
}

static void mt5806_ops_unregister(struct wltrx_ic_ops *ops)
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

static void mt5806_free_dev_resources(struct mt5806_dev_info *di)
{
	power_wakeup_source_unregister(di->wakelock);
	mutex_destroy(&di->mutex_irq);
	gpio_free(di->gpio_int);
	free_irq(di->irq_int, di);
	if (di->default_psy_type == MT5806_DEFAULT_LOWPOWER) {
		gpio_free(di->gpio_rx_online);
		gpio_free(di->gpio_lowpower);
	}
}

static int mt5806_request_dev_resources(struct mt5806_dev_info *di, struct device_node *np)
{
	int ret;
	u32 pinctrl_len = (di->default_psy_type == MT5806_DEFAULT_LOWPOWER) ?
		MT5806_Q_PINCTRL_LEN : MT5806_PINCTRL_LEN;

	ret = mt5806_parse_dts(np, di);
	if (ret)
		goto parse_dts_fail;

	ret = power_pinctrl_config(di->dev, "pinctrl-names", pinctrl_len);
	if (ret)
		goto pinctrl_config_fail;

	ret = mt5806_gpio_init(di, np);
	if (ret)
		goto gpio_init_fail;

	ret = mt5806_irq_init(di, np);
	if (ret)
		goto irq_init_fail;

	di->wakelock = power_wakeup_source_register(di->dev, "mt5806_wakelock");
	mutex_init(&di->mutex_irq);

	return 0;

irq_init_fail:
	if (di->default_psy_type == MT5806_DEFAULT_LOWPOWER) {
		gpio_free(di->gpio_rx_online);
		gpio_free(di->gpio_lowpower);
	}
gpio_init_fail:
pinctrl_config_fail:
parse_dts_fail:
	return ret;
}

static int mt5806_ops_register(struct wltrx_ic_ops *ops, struct mt5806_dev_info *di)
{
	int ret;

	ret = mt5806_fw_ops_register(ops, di);
	if (ret) {
		hwlog_err("ops_register: register fw_ops failed\n");
		return ret;
	}

	ret = mt5806_tx_ops_register(ops, di);
	if (ret) {
		hwlog_err("ops_register: register tx_ops failed\n");
		return ret;
	}
	ret = mt5806_qi_ops_register(ops, di);
	if (ret) {
		hwlog_err("ops_register: register qi_ops failed\n");
		return ret;
	}
	di->g_val.qi_hdl = hwqi_get_handle();

	return 0;
}

static void mt5806_fw_mtp_check(struct mt5806_dev_info *di)
{
	if (power_cmdline_is_powerdown_charging_mode()) {
		di->g_val.mtp_chk_complete = true;
		return;
	}

	INIT_DELAYED_WORK(&di->mtp_check_work, mt5806_fw_mtp_check_work);
	schedule_delayed_work(&di->mtp_check_work, msecs_to_jiffies(WIRELESS_FW_WORK_DELAYED_TIME));
}

static int mt5806_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int ret;
	struct mt5806_dev_info *di = NULL;
	struct device_node *np = NULL;
	struct wltrx_ic_ops *ops = NULL;

	if (!client || !id || !client->dev.of_node)
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

	ret = mt5806_dev_check(di);
	if (ret) {
		ret = -EPROBE_DEFER;
		goto dev_ck_fail;
	}

	ret = mt5806_request_dev_resources(di, np);
	if (ret)
		goto req_dev_res_fail;

	ret = mt5806_ops_register(ops, di);
	if (ret)
		goto ops_regist_fail;

	mt5806_fw_mtp_check(di);
	mt5806_register_pwr_dev_info(di);

	hwlog_info("[ic_type:%u]wireless_chip probe ok\n", di->ic_type);
	return 0;

ops_regist_fail:
	mt5806_free_dev_resources(di);
req_dev_res_fail:
dev_ck_fail:
	mt5806_ops_unregister(ops);
	devm_kfree(&client->dev, di);
	return ret;
}

static int mt5806_remove(struct i2c_client *client)
{
	struct mt5806_dev_info *l_dev = i2c_get_clientdata(client);

	if (!l_dev)
		return -ENODEV;

	if (l_dev->default_psy_type == MT5806_DEFAULT_LOWPOWER) {
		gpio_free(l_dev->gpio_rx_online);
		gpio_free(l_dev->gpio_lowpower);
	}
	gpio_free(l_dev->gpio_int);
	free_irq(l_dev->irq_int, l_dev);
	mutex_destroy(&l_dev->mutex_irq);
	power_wakeup_source_unregister(l_dev->wakelock);
	cancel_delayed_work(&l_dev->mtp_check_work);
	devm_kfree(&client->dev, l_dev);

	return 0;
}

#ifdef CONFIG_PM
static int mt5806_suspend(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct mt5806_dev_info *di = NULL;

	hwlog_info("[suspend] begin\n");

	if (!client)
		return 0;

	di = i2c_get_clientdata(client);
	if (!di)
		return 0;
	if (di->default_psy_type != MT5806_DEFAULT_LOWPOWER)
		return 0;

	gpio_set_value(di->gpio_lowpower, MT5806_Q_LONG_PERIOD);

	hwlog_info("[suspend] end\n");
	return 0;
}

static int mt5806_resume(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct mt5806_dev_info *di = NULL;

	hwlog_info("[resume] begin\n");

	if (!client)
		return 0;

	di = i2c_get_clientdata(client);
	if (!di)
		return 0;
	if (di->default_psy_type != MT5806_DEFAULT_LOWPOWER)
		return 0;

	gpio_set_value(di->gpio_lowpower, MT5806_Q_SHORT_PERIOD);

	hwlog_info("[resume] end\n");
	return 0;
}

static const struct dev_pm_ops mt5806_pm_ops = {
	.suspend = mt5806_suspend,
	.resume = mt5806_resume,
};

#define MT5806_PM_OPS (&mt5806_pm_ops)
#else
#define MT5806_PM_OPS (NULL)
#endif /* CONFIG_PM */

static void mt5806_shutdown(struct i2c_client *client)
{
	hwlog_info("[shutdown]\n");
}

MODULE_DEVICE_TABLE(i2c, wireless_mt5806);
static const struct of_device_id mt5806_of_match[] = {
	{
		.compatible = "mt,mt5806",
		.data = NULL,
	},
	{
		.compatible = "mt,mt5806_aux",
		.data = NULL,
	},
	{},
};

static const struct i2c_device_id mt5806_i2c_id[] = {
	{ "mt5806", WLTRX_IC_MAIN },
	{ "mt5806_aux", WLTRX_IC_AUX },
	{}
};

static struct i2c_driver mt5806_driver = {
	.probe = mt5806_probe,
	.shutdown = mt5806_shutdown,
	.remove = mt5806_remove,
	.id_table = mt5806_i2c_id,
	.driver = {
		.owner = THIS_MODULE,
		.name = "wireless_mt5806",
		.of_match_table = of_match_ptr(mt5806_of_match),
		.pm = MT5806_PM_OPS,
	},
};

static int __init mt5806_init(void)
{
	return i2c_add_driver(&mt5806_driver);
}

static void __exit mt5806_exit(void)
{
	i2c_del_driver(&mt5806_driver);
}

device_initcall(mt5806_init);
module_exit(mt5806_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("mt5806 module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
