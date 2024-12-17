

#include "tof_detect.h"

#include <linux/gpio.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/err.h>
#include <huawei_platform/log/hw_log.h>

#include <securec.h>
#include "sensor_config.h"

#ifndef HWLOG_TAG
#define HWLOG_TAG tof_sensor
HWLOG_REGIST();
#endif

#define GPIO_STAT_HIGH 1

static struct tof_platform_data g_tof_data = {};
static struct tof_device_info g_tof_dev_info = {};

struct tof_device_info *tof_get_device_info(int32_t tag)
{
	if (tag == TAG_TOF)
		return (&g_tof_dev_info);
	hwlog_err("[%s-%d]:error, please check tag %d\n", __func__, __LINE__, tag);
	return NULL;
}

static void read_tof_chip_info(enum sensor_detect_list name)
{
	struct tof_device_info *dev_info = NULL;
	char *chip_info = get_sensor_chip_info_address(name);

	if (name == TOF) {
		dev_info = &g_tof_dev_info;
	} else {
		hwlog_err("[%s-%d]:name is not valied name:%d\n", __func__, __LINE__, name);
		return;
	}

	if (strncmp(chip_info, "huawei,vi5300", sizeof("huawei,vi5300")) == 0) {
		dev_info->tof_first_start_flag = 0;
		dev_info->chip_type = TOF_CHIP_VI5300;
		dev_info->tof_enable_flag = 1;
		hwlog_info("[%s-%d]:enable tof:%d succ\n", __func__, __LINE__, name);
	} else {
		hwlog_info("[%s-%d]:tof:%d not find target device info\n",
			__func__, __LINE__, name);
		return;
	}
	hwlog_info("[%s-%d]:tof:%d succ\n", __func__, __LINE__, name);
}

static int32_t tof_dts_info_parse(struct tof_dts_data *data, struct device_node *dn)
{
	int32_t ret;
	int32_t temp;

	if (!dn || !data) {
		hwlog_err("[%s-%d]:dn or data is null ptr\n", __func__, __LINE__);
		return -1;
	}
	ret = of_property_read_u32(dn, "chip_id_register", &data->chip_id_reg);
	if (ret != 0) {
		hwlog_err("[%s-%d]:of get property chip_id_register fail, ret:0x%x\n",
			__func__, __LINE__, ret);
		return -1;
	}
	ret = of_property_read_u32(dn, "chip_id_value", &data->chip_id_val);
	if (ret != 0) {
		hwlog_err("[%s-%d]:of get property chip_id_value fail, ret:0x%x\n",
			__func__, __LINE__, ret);
		return -1;
	}
	ret = of_property_read_u32(dn, "bus_number", &data->bus_num);
	if (ret != 0) {
		hwlog_err("[%s-%d]:of get property bus_number fail, ret:0x%x\n",
			__func__, __LINE__, ret);
		return -1;
	}
	ret = of_property_read_u32(dn, "reg", &data->dev_addr);
	if (ret != 0) {
		hwlog_err("[%s-%d]:of get property reg fail, ret:0x%x\n",
			__func__, __LINE__, ret);
		return -1;
	}
	temp = of_get_named_gpio(dn, "xshut-gpio", 0);
	if (temp < 0) {
		hwlog_err("[%s-%d]:read gpio_xshut fail\n", __func__, __LINE__);
		return -1;
	}
	data->gpio_xshut = (GPIO_NUM_TYPE)temp;
	temp = of_get_named_gpio(dn, "irq-gpio", 0);
	if (temp < 0) {
		hwlog_err("[%s-%d]:read gpio_irq fail\n", __func__, __LINE__);
		return -1;
	}
	data->gpio_irq = (GPIO_NUM_TYPE)temp;
	hwlog_info("[%s-%d]:bus_num:0x%x, reg:0x%x, chip_id_reg:0x%x, chip_id_val:0x%x, \
		gpio_xshut:%d, gpio_irq:%d\n",
		__func__, __LINE__, data->bus_num, data->dev_addr, data->chip_id_reg,
		data->chip_id_val, data->gpio_xshut, data->gpio_irq);
	return 0;
}

static void tof_chip_power_on(struct tof_dts_data *data)
{
	int32_t ret = gpio_direction_output(data->gpio_xshut, GPIO_STAT_HIGH);
	if (ret != 0) {
		hwlog_err("[%s-%d]:gpio_direction_output fail, gpio_xshut:%d, ret:0x%x\n",
			__func__, __LINE__, data->gpio_xshut, ret);
		return;
	}
	hwlog_info("[%s-%d]:tof bus_num:%u, dev_addr:0x%x, gpio_xshut:%d chip power on\n",
		__func__, __LINE__, data->bus_num, data->dev_addr, data->gpio_xshut);
}

static int32_t tof_sensor_detect_ext(struct tof_dts_data *platform_data,
	struct sensor_combo_cfg *cfg, struct device_node *dn,
	struct sensor_detect_manager *sm, int index)
{
	int32_t ret;
	struct sensor_combo_cfg combo_cfg = {};
	struct tof_dts_data dts_data = {};

	ret = tof_dts_info_parse(&dts_data, dn);
	if (ret != 0) {
		hwlog_err("[%s-%d]:tof_dts_info_parse fail\n", __func__, __LINE__);
		return -1;
	}
	// pull up chip voltage gpio
	tof_chip_power_on(&dts_data);

	if (memcpy_s(platform_data, sizeof(struct tof_dts_data), &dts_data,
		sizeof(struct tof_dts_data)) != EOK)
		hwlog_err("[%s-%d]:memcpy_s fail\n", __func__, __LINE__);

	ret = _device_detect(dn, index, &combo_cfg);
	hwlog_info("[%s-%d]:device_detect_ex:%s _device_detect ret:0x%x\n",
		__func__, __LINE__, sm[index].sensor_name_str, ret);
	if (!ret) {
		if (memcpy_s((void *)cfg, sizeof(struct sensor_combo_cfg), (void *)&combo_cfg,
			sizeof(struct sensor_combo_cfg)) != EOK) {
			hwlog_err("[%s-%d]:memcpy_s sensor_combo_cfg fail\n", __func__, __LINE__);
			ret = -1;
		}
		hwlog_info("[%s-%d]:device_detect_ex:%s, memcpy_s data, cfg->bus_num:0x%x, \
			cfg->i2c_addr:0x%x, ret:0x%x\n", __func__, __LINE__,
			sm[index].sensor_name_str, combo_cfg.bus_num, combo_cfg.i2c_address, ret);
	}
	return ret;
}

void read_tofp_data_from_dts(pkt_sys_dynload_req_t *dyn_req, struct device_node *dn)
{
	int temp = 0;

	read_tof_chip_info(TOF);
	if (of_property_read_u32(dn, "file_id", &temp))
		hwlog_err("%s:read tof file_id fail\n", __func__);
	else
		dyn_req->file_list[dyn_req->file_count] = (uint16_t)temp;

	dyn_req->file_count++;

	if (of_property_read_u32(dn, "sensor_list_info_id", &temp))
		hwlog_err("%s:read tof sensor_list_info_id fail\n", __func__);
	else
		add_sensor_list_info_id((uint16_t)temp);
	read_sensorlist_info(dn, TOF);
}

int32_t tof_sensor_detect(struct device_node *dn, struct sensor_detect_manager *sm, int index)
{
	int32_t ret;

	if (!dn || !sm) {
		hwlog_err("[%s-%d]:dn or sm is null ptr\n", __func__, __LINE__);
		return -1;
	}

	ret = tof_sensor_detect_ext(&g_tof_data.vi5300,
		&g_tof_data.cfg, dn, sm, index);
	if (ret != 0)
		hwlog_err("[%s-%d]:tof vi5300 detect fail, ret:0x%x\n",
			__func__, __LINE__, ret);

	return ret;
}

static void signal_tof_detect_init(enum sensor_detect_list s_id,
	struct sensor_detect_manager *sm, uint32_t len)
{
	struct sensor_detect_manager *tof_sm = NULL;

	if (!sm || len <= s_id) {
		hwlog_err("[%s-%d]:sm is null ptr of len:%u is less than TOF:%d\n",
			__func__, __LINE__, len, s_id);
		return;
	}

	tof_sm = sm + s_id;
	tof_sm->spara = (const void *)&g_tof_data;
	tof_sm->cfg_data_length = sizeof(g_tof_data);
}

void tof_detect_init(struct sensor_detect_manager *sm, uint32_t len)
{
	signal_tof_detect_init(TOF, sm, len);
}

