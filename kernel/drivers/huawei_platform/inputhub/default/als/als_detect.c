/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: als detect source file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#include "als_detect.h"

#include <linux/delay.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/slab.h>
#include <linux/types.h>

#include <securec.h>

#include "als_sysfs.h"
#include "contexthub_boot.h"
#include "contexthub_debug.h"
#include "contexthub_route.h"
#include "als_tp_color.h"
#ifdef DSS_ALSC_NOISE
#include "display_effect_alsc_interface.h"
#endif
#ifdef DKMD_ALSC_NOISE
#include "dkmd_alsc_interface.h"
#endif

#define ALS_DEVICE_ID_0 0
#define ALS_DEVICE_ID_1 1
#define ALS_DEVICE_ID_2 2
#define ALS_UD_BLOCK_LEN      30
#define ALS_SENSOR_DEFAULT_ID "5"

static char *g_als_sensor_id = ALS_SENSOR_DEFAULT_ID;

static struct als_platform_data als_data[ALS_DEV_COUNT_MAX] = {
	{
	.cfg = DEF_SENSOR_COM_SETTING,
	.threshold_value = 1,
	.ga1 = 4166, /* 4166: parameter for light source 1 */
	.ga2 = 3000, /* 3000: parameter for light source 2 */
	.ga3 = 3750, /* 3750: parameter for light source 3 */
	.coe_b = 0,
	.coe_c = 0,
	.coe_d = 0,
	.gpio_int1 = 206, /* 206: gpio int pin */
	.atime = 0xdb,
	.again = 1,
	.poll_interval = 1000, /* interval 1000 ms */
	.init_time = 150, /* init need 150 ms */
	.als_phone_type = 0,
	.als_phone_version = 0,
	.is_close = 0,
	.tp_info = 0,
	.is_bllevel_supported = 0,
	.is_dc_supported = 0,
	.gpio_func_num = 2, /* set gpio to func 2 */
	.character_switch = 0,
	},
};
static struct als_device_info g_als_dev_info[ALS_DEV_COUNT_MAX];

struct als_platform_data *als_get_platform_data(int32_t tag)
{
	if (tag == TAG_ALS)
		return (&(als_data[ALS_DEVICE_ID_0]));
	else if (tag == TAG_ALS1)
		return (&(als_data[ALS_DEVICE_ID_1]));
	else if (tag == TAG_ALS2)
		return (&(als_data[ALS_DEVICE_ID_2]));

	hwlog_info("%s error, please check tag %d\n", __func__, tag);
	return NULL;
}

struct als_device_info *als_get_device_info(int32_t tag)
{
	if (tag == TAG_ALS)
		return (&(g_als_dev_info[ALS_DEVICE_ID_0]));
	else if (tag == TAG_ALS1)
		return (&(g_als_dev_info[ALS_DEVICE_ID_1]));
	else if (tag == TAG_ALS2)
		return (&(g_als_dev_info[ALS_DEVICE_ID_2]));

	hwlog_info("%s error, please check tag %d\n", __func__, tag);
	return NULL;
}

char *als_get_sensors_id_string(void)
{
	return g_als_sensor_id;
}

void als_get_sensors_id_from_dts(struct device_node *dn)
{
	if (dn == NULL)
		return;
	if (of_property_read_string(dn,
		"als_sensor_id", (const char **)&g_als_sensor_id)) {
		g_als_sensor_id = ALS_SENSOR_DEFAULT_ID;
		hwlog_err("als_sensor_id err\n");
	}
	hwlog_info("als_sensor_id:%s\n", g_als_sensor_id);
}

void read_als_info_from_dts(struct als_device_info *dev_info,
	struct device_node *dn)
{
	int temp = 0;
	int ret;

	if (of_property_read_u32(dn, "tp_color_from_nv", &temp))
		hwlog_err("%s:read tp_color_from_nv fail\n", __func__);
	else
		dev_info->tp_color_from_nv_flag = (uint8_t)temp;

	if (of_property_read_u32(dn, "is_cali_supported", &temp))
		hwlog_err("%s:read als is_cali_supported fail\n", __func__);
	else
		dev_info->is_cali_supported = (uint8_t)temp;

	ret = fill_extend_data_in_dts(dn, "als_mmi_para",
		(unsigned char *)dev_info->als_mmi_para, ALS_MMI_PARA_LEN,
		EXTEND_DATA_TYPE_IN_DTS_WORD);
	if (ret) {
		dev_info->als_mmi_para[ALS_MMI_LUX_MIN_ID] =
			ALS_MMI_LUX_MIN_VAL;
		dev_info->als_mmi_para[ALS_MMI_LUX_MAX_ID] =
			ALS_MMI_LUX_MAX_VAL;
	}
}

void read_als_ud_from_dts(struct als_device_info *dev_info,
	struct device_node *dn)
{
	int temp = 0;
	int ret;
#if ((defined DSS_ALSC_NOISE) || (defined DKMD_ALSC_NOISE))
	int i;
#endif

	if (of_property_read_u32(dn, "als_cali_after_sale", &temp))
		hwlog_err("%s:read als_cali_after_sale fail\n", __func__);
	else
		dev_info->als_support_under_screen_cali = (uint32_t)temp;

	ret = fill_extend_data_in_dts(dn, "als_cali_after_sale_xy",
		(unsigned char *)dev_info->als_ud_cali_xy, ALS_UD_CALI_LEN,
		EXTEND_DATA_TYPE_IN_DTS_HALF_WORD);
	if (ret)
		hwlog_err("als_ud_cali_xy get data fail\n");

	if (of_property_read_u32(dn, "als_ud_nv_type", &temp))
		hwlog_err("%s:read als_ud_nv_type fail\n", __func__);
	else
		dev_info->als_ud_nv_type = (uint8_t)temp;

#if ((defined DSS_ALSC_NOISE) || (defined DKMD_ALSC_NOISE))
	if (of_property_read_u32(dn, "als_ud_size", &temp)) {
		hwlog_err("%s:read als_ud_size fail\n", __func__);
	} else {
		hisi_dss_alsc_init.size = (uint32_t)temp;
		hwlog_info("hisi_dss_alsc_init.size:0x%x\n", hisi_dss_alsc_init.size);
	}

	ret = fill_extend_data_in_dts(dn, "als_ud_addr_block",
		(unsigned char *)(hisi_dss_alsc_init.addr_block), ALS_UD_BLOCK_LEN,
		EXTEND_DATA_TYPE_IN_DTS_WORD);
	if (ret) {
		hwlog_err("als_ud_addr_block get data fail\n");
	} else {
		for (i = 0; i < ALS_UD_BLOCK_LEN; i++)
			hwlog_info("hisi_dss_alsc_init.addr_block:0x%x\n",
				hisi_dss_alsc_init.addr_block[i]);
	}

	ret = fill_extend_data_in_dts(dn, "als_ud_ave_block",
		(unsigned char *)(hisi_dss_alsc_init.ave_block), ALS_UD_BLOCK_LEN,
		EXTEND_DATA_TYPE_IN_DTS_WORD);
	if (ret) {
		hwlog_err("als_ud_ave_block get data fail\n");
	} else {
		for (i = 0; i < ALS_UD_BLOCK_LEN; i++)
			hwlog_info("hisi_dss_alsc_init.ave_block:0x%x\n",
				hisi_dss_alsc_init.ave_block[i]);
	}
#endif
}

static int als_rohm_detect(struct als_device_info *dev_info,
	const char *chip_info)
{
	if (!strncmp(chip_info, "huawei,rohm_bh1745",
		sizeof("huawei,rohm_bh1745"))) {
		dev_info->chip_type = ALS_CHIP_ROHM_RGB;
	} else if (!strncmp(chip_info, "huawei,rohm_bh1749",
		sizeof("huawei,rohm_bh1749"))) {
		dev_info->chip_type = ALS_CHIP_BH1749;
	} else if (!strncmp(chip_info, "huawei,rohm_bu27006muc",
		sizeof("huawei,rohm_bu27006muc"))) {
		dev_info->chip_type = ALS_CHIP_BU27006MUC;
	} else if (!strncmp(chip_info, "huawei,rohm_rpr531_als",
		sizeof("huawei,rohm_rpr531_als"))) {
		dev_info->chip_type = ALS_CHIP_RPR531;
	} else if (!strncmp(chip_info, "huawei,B",
		sizeof("huawei,B"))) {
		dev_info->chip_type = ALS_CHIP_BH1726;
	} else {
		return -1;
	}

	return 0;
}

static int als_avago_detect(struct als_device_info *dev_info,
	const char *chip_info)
{
	if (!strncmp(chip_info, "huawei,avago_apds9251",
		sizeof("huawei,avago_apds9251"))) {
		dev_info->chip_type = ALS_CHIP_AVAGO_RGB;
	} else if (!strncmp(chip_info, "huawei,apds9922_als",
		sizeof("huawei,apds9922_als"))) {
		dev_info->chip_type = ALS_CHIP_APDS9922;
	} else if (!strncmp(chip_info, "huawei,avago_apds9999",
		sizeof("huawei,avago_apds9999"))) {
		dev_info->chip_type = ALS_CHIP_APDS9999_RGB;
	} else if (!strncmp(chip_info, "huawei,avago_apds9253",
		sizeof("huawei,avago_apds9253"))) {
		dev_info->chip_type = ALS_CHIP_APDS9253_RGB;
	} else {
		return -1;
	}

	return 0;
}

static int als_ams_find_tcs3701(const char *chip_info)
{
	if (!strncmp(chip_info, "huawei,ams_tcs3701",
		sizeof("huawei,ams_tcs3701")))
		return 1;
	if (!strncmp(chip_info, "als_s001_002",
		sizeof("als_s001_002")))
		return 1;
	if (!strncmp(chip_info, "huawei,als_s001002",
		sizeof("huawei,als_s001002")))
		return 1;

	return 0;
}

static int als_ams_find_tcs3718(const char *chip_info)
{
	if (!strncmp(chip_info, "huawei,ams_tcs3718",
		sizeof("huawei,ams_tcs3718")))
		return 1;
	if (!strncmp(chip_info, "als_s001_003",
		sizeof("als_s001_003")))
		return 1;
	if (!strncmp(chip_info, "huawei,als_s001003",
		sizeof("huawei,als_s001003")))
		return 1;
	return 0;
}

static int als_ams_detect(struct als_device_info *dev_info,
	const char *chip_info)
{
	if (!strncmp(chip_info, "huawei,ams_tmd3725",
		sizeof("huawei,ams_tmd3725"))) {
		dev_info->chip_type = ALS_CHIP_AMS_TMD3725_RGB;
	} else if (!strncmp(chip_info, "huawei,tmd2745_als",
		sizeof("huawei,tmd2745_als"))) {
		dev_info->chip_type = ALS_CHIP_TMD2745;
	} else if (!strncmp(chip_info, "huawei,ams_tmd3702",
		sizeof("huawei,ams_tmd3702"))) {
		dev_info->chip_type = ALS_CHIP_AMS_TMD3702_RGB;
	} else if (!strncmp(chip_info, "huawei,ams_tcs3707",
		sizeof("huawei,ams_tcs3707"))) {
		dev_info->chip_type = ALS_CHIP_AMS_TCS3707_RGB;
	} else if (!strncmp(chip_info, "huawei,als_s001_008",
		sizeof("huawei,als_s001_008"))) {
		dev_info->chip_type = ALS_CHIP_AMS_TCS3707_RGB;
	} else if (als_ams_find_tcs3701(chip_info)) {
		dev_info->chip_type =  ALS_CHIP_AMS_TCS3701_RGB;
	} else if (!strncmp(chip_info, "huawei,ams_tcs3708",
		sizeof("huawei,ams_tcs3708"))) {
		dev_info->chip_type = ALS_CHIP_AMS_TCS3708_RGB;
	} else if (als_ams_find_tcs3718(chip_info)) {
		dev_info->chip_type = ALS_CHIP_AMS_TCS3718_RGB;
	} else if (!strncmp(chip_info, "huawei,A",
		sizeof("huawei,A"))) {
		dev_info->chip_type = ALS_CHIP_TSL2591;
	} else if (!strncmp(chip_info, "als_s001_001",
		sizeof("als_s001_001"))) {
		/* for old project, wo need to add new machine name */
		dev_info->chip_type = ALS_CHIP_TSL2591;
	} else if (!strncmp(chip_info, "als_s001_009", sizeof("als_s001_009"))) {
		dev_info->chip_type = ALS_CHIP_TSL2585;
	} else if (!strncmp(chip_info, "als,als_s001004",
		sizeof("als,als_s001004"))) {
		/* for old project, wo need to add new machine name */
		dev_info->chip_type = ALS_CHIP_AMS_TMD2702_RGB;
	} else if (!strncmp(chip_info, "huawei,ams_tsl2540",
		sizeof("huawei,ams_tsl2540"))) {
		/* for old project, wo need to add new machine name */
		dev_info->chip_type = ALS_CHIP_AMS_TSL2540_RGB;
	} else if (!strncmp(chip_info, "huawei,cam_als",
		sizeof("huawei,cam_als"))) {
		/* for old project, wo need to add new machine name */
		dev_info->chip_type = ALS_CHIP_CAM_ALS;
	} else if (!strncmp(chip_info, "huawei,als1_s001007",
		sizeof("huawei,als1_s001007")) ||
		!strncmp(chip_info, "huawei,als_s001007",
		sizeof("huawei,als_s001007"))) {
		dev_info->chip_type = ALS_CHIP_TCS3410;
	} else {
		return -1;
	}

	return 0;
}

static int als_liteon_detect(struct als_device_info *dev_info,
	const char *chip_info)
{
	if (!strncmp(chip_info, "huawei,liteon_ltr582",
		sizeof("huawei,liteon_ltr582"))) {
		dev_info->chip_type = ALS_CHIP_LITEON_LTR582;
	} else if (!strncmp(chip_info, "huawei,ltr578_als",
		sizeof("huawei,ltr578_als"))) {
		dev_info->chip_type = ALS_CHIP_LTR578;
	} else if (!strncmp(chip_info, "huawei,liteon_ltr2568",
		sizeof("huawei,liteon_ltr2568"))) {
		dev_info->chip_type = ALS_CHIP_LTR2568;
	} else if (!strncmp(chip_info, "als_s002_001",
		sizeof("als_s002_001"))) {
		/* for old project, wo need to add new machine name */
		dev_info->chip_type = ALS_CHIP_LTR2568;
	} else if (!strncmp(chip_info, "huawei,liteon_ltr2594",
		sizeof("huawei,liteon_ltr2594"))) {
		dev_info->chip_type = ALS_CHIP_LTR2594;
	} else if (!strncmp(chip_info, "huawei,liteon_ltr311",
		sizeof("huawei,liteon_ltr311"))) {
		dev_info->chip_type = ALS_CHIP_LTR311;
	} else if (!strncmp(chip_info, "als_s002_002",
		sizeof("als_s002_002"))) {
		/* for old project, wo need to add new machine name */
		dev_info->chip_type = ALS_CHIP_LTR311;
	} else {
		return -1;
	}

	return 0;
}

static int als_sensortek_detect(struct als_device_info *dev_info,
	const char *chip_info)
{
	if (!strncmp(chip_info, "huawei,arrow_stk3338",
		sizeof("huawei,arrow_stk3338"))) {
		dev_info->chip_type = ALS_CHIP_STK3338;
	} else if (!strncmp(chip_info, "als_s003_001",
		sizeof("als_s003_001"))) {
		/* for old project, wo need to add new machine name */
		dev_info->chip_type = ALS_CHIP_STK3338;
	} else if (!strncmp(chip_info, "huawei,sensortek_stk3638",
		sizeof("huawei,sensortek_stk3638"))) {
		dev_info->chip_type = ALS_CHIP_STK3638;
	} else if (!strncmp(chip_info, "als_s003_002",
		sizeof("als_s003_002"))) {
		/* for old project, wo need to add new machine name */
		dev_info->chip_type = ALS_CHIP_STK3638;
	} else if (!strncmp(chip_info, "huawei,sensortek_stk32670",
		sizeof("huawei,sensortek_stk32670"))) {
		dev_info->chip_type = ALS_CHIP_STK32670;
	} else if (!strncmp(chip_info, "als_s003_003",
		sizeof("als_s003_003"))) {
		/* for old project, wo need to add new machine name */
		dev_info->chip_type = ALS_CHIP_STK32670;
	} else if (!strncmp(chip_info, "huawei,sensortek_stk32671",
		sizeof("huawei,sensortek_stk32671"))) {
		dev_info->chip_type = ALS_CHIP_STK32671;
	} else if (!strncmp(chip_info, "huawei,als_s003006",
		sizeof("huawei,als_s003006"))) {
		dev_info->chip_type = ALS_CHIP_STK37670;
	} else {
		return -1;
	}

	return 0;
}

static int als_vishay_detect(struct als_device_info *dev_info,
	const char *chip_info)
{
	if (!strncmp(chip_info, "huawei,vishay_vcnl36658",
		sizeof("huawei,vishay_vcnl36658"))) {
		dev_info->chip_type = ALS_CHIP_VISHAY_VCNL36658;
	} else if (!strncmp(chip_info, "huawei,vishay_vcnl36832",
		sizeof("huawei,vishay_vcnl36832"))) {
		dev_info->chip_type = ALS_CHIP_VISHAY_VCNL36832;
	} else if (!strncmp(chip_info, "huawei,C",
		sizeof("huawei,C"))) {
		dev_info->chip_type = ALS_CHIP_VEML32185;
	} else {
		return -1;
	}

	return 0;
}

static int als_st_detect(struct als_device_info *dev_info,
	const char *chip_info)
{
	if (!strncmp(chip_info, "huawei,st_vd6281",
		sizeof("huawei,st_vd6281"))) {
		dev_info->chip_type = ALS_CHIP_VD6281;
	} else {
		return -1;
	}

	return 0;
}

static int als_silergy_detect(struct als_device_info *dev_info,
	const char *chip_info)
{
	if (!strncmp(chip_info, "huawei,silergy_syh399",
		sizeof("huawei,silergy_syh399"))) {
		dev_info->chip_type = ALS_CHIP_SYH399;
	} else if (!strncmp(chip_info, "als_s004_001",
		sizeof("als_s004_001"))) {
		/* for old project, we need to add new machine name */
		dev_info->chip_type = ALS_CHIP_SYH399;
	} else if (!strncmp(chip_info, "huawei,sy3133cs",
		sizeof("huawei,sy3133cs"))) {
		dev_info->chip_type = ALS_CHIP_SY3133_RGB;
	} else if (!strncmp(chip_info, "huawei,als_s004_002",
		sizeof("huawei,als_s004_002"))) {
		dev_info->chip_type = ALS_CHIP_SY3133_RGB;
	} else {
		return -1;
	}

	return 0;
}

static int als_sip_detect(struct als_device_info *dev_info,
	const char *chip_info)
{
	if (!strncmp(chip_info, "huawei,sip3623-als",
		sizeof("huawei,sip3623-als"))) {
		dev_info->chip_type = ALS_CHIP_SIP3623;
	} else if (!strncmp(chip_info, "huawei,als_s006001",
		sizeof("huawei,als_s006001"))) {
		dev_info->chip_type = ALS_CHIP_SIP3623;
	} else if (!strncmp(chip_info, "huawei,sip3510-als",
		sizeof("huawei,sip3510-als"))) {
		dev_info->chip_type = ALS_CHIP_SIP3510;
	} else if (!strncmp(chip_info, "huawei,als_s006004",
		sizeof("huawei,als_s006004"))) {
		dev_info->chip_type = ALS_CHIP_SIP3510;
	} else if (!strncmp(chip_info, "huawei,als1_s006005",
		sizeof("huawei,als1_s006005"))) {
		dev_info->chip_type = ALS_CHIP_SIP2326;
	} else if (!strncmp(chip_info, "huawei,als_s006005",
		sizeof("huawei,als_s006005"))) {
		dev_info->chip_type = ALS_CHIP_SIP2326;
	} else if (!strncmp(chip_info, "als_s006_006",
		sizeof("als_s006_006"))) {
		dev_info->chip_type = ALS_CHIP_SIP1225;
	} else if (!strncmp(chip_info, "huawei,als_s006008",
		sizeof("huawei,als_s006008"))) {
		dev_info->chip_type = ALS_CHIP_SIP3642;
	} else if (!strncmp(chip_info, "als_s006_007",
		sizeof("als_s006_007"))) {
		dev_info->chip_type = ALS_CHIP_SIP1226;
	} else {
		return -1;
	}

	return 0;
}

static int als_syn_detect(struct als_device_info *dev_info,
	const char *chip_info)
{
	if (!strncmp(chip_info, "huawei,syn3133",
		sizeof("huawei,syn3133"))) {
		dev_info->chip_type = ALS_CHIP_SYN3133;
	} else if (!strncmp(chip_info, "huawei,als_s004002",
		sizeof("huawei,als_s004002"))) {
		dev_info->chip_type = ALS_CHIP_SYN3133;
	} else if (!strncmp(chip_info, "huawei,syn3136-als",
		sizeof("huawei,syn3136-als"))) {
		dev_info->chip_type = ALS_CHIP_SYN3136;
	} else if (!strncmp(chip_info, "huawei,als_s004003",
		sizeof("huawei,als_s004003"))) {
		dev_info->chip_type = ALS_CHIP_SYN3136;
	} else {
		return -1;
	}

	return 0;
}

static int als_hx_detect(struct als_device_info *dev_info,
	const char *chip_info)
{
	if (!strncmp(chip_info, "huawei,hx32062se-als",
		sizeof("huawei,hx32062se-als"))) {
		dev_info->chip_type = ALS_CHIP_HX32062SE;
	} else if (!strncmp(chip_info, "huawei,als_s007001",
		sizeof("huawei,als_s007001"))) {
		dev_info->chip_type = ALS_CHIP_HX32062SE;
	} else {
		return -1;
	}

	return 0;
}

static int als_get_chip_type_by_compatible(struct als_device_info *dev_info,
	const char *chip_info)
{
	if ((!dev_info) || (!chip_info))
		return -1;

	if (!als_rohm_detect(dev_info, chip_info)) {
	} else if (!als_avago_detect(dev_info, chip_info)) {
	} else if (!als_ams_detect(dev_info, chip_info)) {
	} else if (!als_liteon_detect(dev_info, chip_info)) {
	} else if (!als_vishay_detect(dev_info, chip_info)) {
	} else if (!als_sensortek_detect(dev_info, chip_info)) {
	} else if (!als_st_detect(dev_info, chip_info)) {
	} else if (!als_silergy_detect(dev_info, chip_info)) {
	} else if (!als_sip_detect(dev_info, chip_info)) {
	} else if (!als_syn_detect(dev_info, chip_info)) {
	} else if (!als_hx_detect(dev_info, chip_info)) {
	} else {
		return -1;
	}

	return 0;
}

static void read_als_config1_from_dts(struct device_node *dn,
	struct sensor_detect_manager *sm)
{
	struct als_platform_data *pf_data =
		(struct als_platform_data *)sm->spara;
	int temp;

	temp = of_get_named_gpio(dn, "gpio_int1", 0);
	if (temp < 0)
		hwlog_err("%s:read gpio_int1 fail\n", __func__);
	else
		pf_data->gpio_int1 = (GPIO_NUM_TYPE)temp;

	if (of_property_read_u32(dn, "gpio_int1_sh", &temp))
		hwlog_err("%s:read gpio_int1_sh fail\n", __func__);
	else
		pf_data->gpio_int1_sh = (GPIO_NUM_TYPE)temp;

	if (of_property_read_u32(dn, "als_ud_type", &temp))
		hwlog_err("%s:read als_ud_type fail\n", __func__);
	else
		pf_data->als_ud_type = (GPIO_NUM_TYPE)temp;

	if (of_property_read_u32(dn, "poll_interval", &temp))
		hwlog_err("%s:read als poll_interval fail\n", __func__);
	else
		pf_data->poll_interval = (uint16_t)temp;

	if (of_property_read_u32(dn, "reg", &temp))
		hwlog_err("%s:read als reg fail\n", __func__);
	else
		pf_data->cfg.i2c_address = (uint8_t)temp;

	if (of_property_read_u32(dn, "init_time", &temp))
		hwlog_err("%s:read als init time fail\n", __func__);
	else
		pf_data->init_time = (uint16_t)temp;

	if (of_property_read_u32(dn, "GA1", &temp))
		hwlog_err("%s:read als ga1 fail\n", __func__);
	else
		pf_data->ga1 = temp;

	if (of_property_read_u32(dn, "GA2", &temp))
		hwlog_err("%s:read als ga2 fail\n", __func__);
	else
		pf_data->ga2 = temp;

	if (of_property_read_u32(dn, "als_after_sale_set_bl_mode", &temp))
		hwlog_err("%s:read als als_after_sale_set_bl_mode fail\n", __func__);
	else
		pf_data->set_bl_mode = temp;
}

static void read_als_config2_from_dts(struct device_node *dn,
	struct sensor_detect_manager *sm)
{
	struct als_platform_data *pf_data =
		(struct als_platform_data *)sm->spara;
	int temp = 0;
	int aux = 0;

	if (of_property_read_u32(dn, "GA3", &temp))
		hwlog_err("%s:read als ga3 fail\n", __func__);
	else
		pf_data->ga3 = temp;

	if (of_property_read_u32(dn, "als_phone_type", &temp))
		hwlog_err("%s:read als_phone_type fail\n", __func__);
	else
		pf_data->als_phone_type = (uint8_t)temp;

	if (of_property_read_u32(dn, "als_phone_version", &temp))
		hwlog_err("%s:read als_phone_version fail\n", __func__);
	else
		pf_data->als_phone_version = (uint8_t)temp;

	pf_data->als_phone_tp_colour = get_phone_color();

	if (of_property_read_u32(dn, "atime", &temp))
		hwlog_err("%s:read als atime fail\n", __func__);
	else
		pf_data->atime = (uint8_t)temp;

	if (of_property_read_u32(dn, "again", &temp))
		hwlog_err("%s:read als again fail\n", __func__);
	else
		pf_data->again = (uint8_t)temp;

	if (of_property_read_u32(dn, "is_close", &temp))
		hwlog_err("%s:read als is_close fail\n", __func__);
	else
		pf_data->is_close = (uint8_t)temp;

	if (of_property_read_u32(dn, "aux_prop", &aux))
		hwlog_err("%s:read aux_prop fail\n", __func__);
	else
		hwlog_info("%s:read aux_prop %d\n", __func__, aux);

	if (of_property_read_u32(dn, "file_id", &temp))
		hwlog_err("%s:read als file_id fail\n", __func__);
	else if (aux)
		read_aux_file_list(temp, sm->tag);
	else
		read_dyn_file_list((uint16_t)temp);
}

static void read_als_config3_from_dts(struct device_node *dn,
	struct sensor_detect_manager *sm)
{
	struct als_platform_data *pf_data =
		(struct als_platform_data *)sm->spara;
	int temp = 0;

	if (of_property_read_u32(dn, "is_bllevel_supported", &temp))
		hwlog_err("%s:read als is_bllevel_supported fail\n", __func__);
	else
		pf_data->is_bllevel_supported = (int)temp;

	if (of_property_read_u32(dn, "is_dc_supported", &temp))
		hwlog_err("%s:read als is_dc_supported fail\n", __func__);
	else
		pf_data->is_dc_supported = (int)temp;

	if (of_property_read_u32(dn, "gpio_func_num", &temp))
		hwlog_err("%s:read als gpio_func_num fail\n", __func__);
	else
		pf_data->gpio_func_num = (uint8_t)temp;

	if (of_property_read_u32(dn, "phone_color_num", &temp)) {
		hwlog_err("%s:read phone_color_num fail\n", __func__);
	} else {
		if (temp < MAX_PHONE_COLOR_NUM) {
			if (of_property_read_u32_array(dn, "sleeve_detect_threshhold",
				(uint32_t *)get_sleeve_detect_parameter(),
				temp * 2)) /* sleeve_detect_pare have 2 members */
				hwlog_err("%s:read sleeve_detect_threshhold fail\n", __func__);
		}
	}

	if (of_property_read_u32(dn, "sensor_list_info_id", &temp))
		hwlog_err("%s:read als sensor_list_info_id fail\n", __func__);
	else
		add_sensor_list_info_id((uint16_t)temp);
	if (of_property_read_u32(dn, "character_switch", &temp))
		hwlog_err("%s:read als character_switch fail\n", __func__);
	else
		pf_data->character_switch = (int)temp;
}

void read_als_data_from_dts(struct device_node *dn,
	struct sensor_detect_manager *sm)
{
	struct als_platform_data *pf_data =
		(struct als_platform_data *)sm->spara;
	struct als_device_info *dev_info = NULL;
	char *info = get_sensor_chip_info_address(sm->sensor_id);

	dev_info = als_get_device_info(sm->tag);
	if (!dev_info) {
		hwlog_err("%s: dev_info is NULL, tag=%d\n", __func__, sm->tag);
		return;
	}

	read_chip_info(dn, sm->sensor_id);
	als_get_chip_type_by_compatible(dev_info, (const char *)info);

	hwlog_info("%s:tag %d type %d\n", __func__, sm->tag,
		dev_info->chip_type);

	read_als_config1_from_dts(dn, sm);
	read_als_config2_from_dts(dn, sm);
	read_als_config3_from_dts(dn, sm);
	read_als_info_from_dts(dev_info, dn);
	read_als_ud_from_dts(dev_info, dn);
	read_sensorlist_info(dn, sm->sensor_id);
	select_als_para(pf_data, dev_info, dn);
}

void resend_als_parameters_to_mcu(int32_t tag)
{
	uint32_t s_id = ALS;

	if (als_get_sensor_id_by_tag(tag, &s_id))
		return;

	send_parameter_to_mcu(s_id, SUB_CMD_SET_RESET_PARAM_REQ);
	hwlog_info("%s tag %d\n", __func__, tag);
}

/* recieve als offset data from mcu and write to nv */
int als_data_from_mcu(const struct pkt_header *head)
{
	int ret = -1;

	switch (((pkt_als_calibrate_data_req_t *)head)->subcmd) {
	case SUB_CMD_SET_WRITE_NV_ATTER_SALE:
		ret = als_underscreen_calidata_save(head->tag);
		break;
	default:
		hwlog_err("uncorrect subcmd 0x%x\n",
			((pkt_als_calibrate_data_req_t *)head)->subcmd);
		break;
	}
	return ret;
}

/*
 * For muti_screen && TAG, 1: is coed lcd;  else 0: not coe lcd
 */
static bool is_master_coe_lcd(void)
{
	char coe_info[8] = "190_c09";
	const char *lcd_model = NULL;
	struct device_node *np = NULL;
	struct als_platform_data *pf_data = NULL;

	pf_data = als_get_platform_data(TAG_ALS);
	np = of_find_compatible_node(NULL, NULL, "huawei,lcd_panel_type");
	if (!np) {
		hwlog_err("als not find lcd_panel_type node\n");
		return 0;
	}
	if (pf_data->als_phone_type == ALTB) {
		if (of_property_read_string(np, "lcd_panel_type", &lcd_model)) {
			hwlog_err("als not find lcd_panel_type in dts\n");
			return 0;
		}
		hwlog_info("lcd_panel_type: %s suc in dts!!\n", lcd_model);
		if (!strncmp(lcd_model, coe_info, strlen(coe_info))) {
			hwlog_info("Lcd type is COE, Use Cam ALS\n");
			return 1;
		}
	}
	return 0;
}

int als1_sensor_detect(struct device_node *dn,
	struct sensor_detect_manager *sm, int index)
{
	int ret = 0;
	u32 cam_replace_als_flag = 0;
	struct sensor_detect_manager *p = NULL;
	struct sensor_combo_cfg cfg = { 0 };
	bool is_coe_lcd = false;

	if ((!dn) || (index < 0) || (index >= SENSOR_MAX) || (!sm))
		return -1;

	is_coe_lcd = is_master_coe_lcd();

	hwlog_info("als1 sensor detect: is coe lcd:%d\n", is_coe_lcd);

	ret = of_property_read_u32(dn, "replace_by_cam", &cam_replace_als_flag);
	if (ret) {
		hwlog_info("no replace_by_cam config ret:%d\n", ret);
		cam_replace_als_flag = 0;
	}
	hwlog_info("cam_replace_als_flag: %d\n", cam_replace_als_flag);

	if (cam_replace_als_flag) { // lcd is coe used camera als
		p = sm + index;
		hwlog_info("get replace_by_cam flag %d, use cam als\n", cam_replace_als_flag);
		p->detect_result = DET_SUCC;
		ret = 0;
	} else {  // sensor
		ret = _device_detect(dn, index, &cfg);
		if (!ret) {  // found als1 sensor
			p = sm + index;
			if (is_coe_lcd) {  // lcd is coe, not used
				hwlog_info("device_detect: detected, coe lcd, but not used\n");
				p->detect_result = DET_FAIL;
				ret = -1;
			} else { // normal status
				memcpy_s((void *)p->spara, sizeof(struct sensor_combo_cfg),
						(void *)&cfg, sizeof(struct sensor_combo_cfg));
			}
		}
	}
	return ret;
}


void als_detect_init(struct sensor_detect_manager *sm, uint32_t len)
{
	struct sensor_detect_manager *p = NULL;
	int32_t i;

	if (len <= ALS2) {
		hwlog_err("%s:len=%d\n", __func__, len);
		return;
	}
	g_als_dev_info[ALS_DEVICE_ID_0].obj_tag = TAG_ALS;
	g_als_dev_info[ALS_DEVICE_ID_1].obj_tag = TAG_ALS1;
	g_als_dev_info[ALS_DEVICE_ID_2].obj_tag = TAG_ALS2;
	g_als_dev_info[ALS_DEVICE_ID_0].detect_list_id = ALS;
	g_als_dev_info[ALS_DEVICE_ID_1].detect_list_id = ALS1;
	g_als_dev_info[ALS_DEVICE_ID_2].detect_list_id = ALS2;
	for (i = 0; i < ALS_DEV_COUNT_MAX; i++) {
		g_als_dev_info[i].als_dev_index = i;
		p = sm + g_als_dev_info[i].detect_list_id;
		p->spara = (const void *)&(als_data[i]);
		p->cfg_data_length = sizeof(als_data[i]);
		if (i == 0)
			continue;
		if (memcpy_s(&(als_data[i]), sizeof(als_data[i]),
			&(als_data[0]), sizeof(als_data[0])) != EOK)
			hwlog_info("%s: memcpy_s error i=%d\n", __func__, i);
	}
}
