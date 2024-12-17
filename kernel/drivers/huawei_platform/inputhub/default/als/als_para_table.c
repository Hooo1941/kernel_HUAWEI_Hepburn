/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: als para table ams source file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#include "als_para_table.h"

#include <linux/err.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/slab.h>
#include <linux/types.h>

#include <securec.h>

#include "als_para_table_ams.h"
#include "als_para_table_ams_tcs3701.h"
#include "als_para_table_ams_tcs3718.h"
#include "als_para_table_ams_tmd2702.h"
#include "als_para_table_ams_tmd3702.h"
#include "als_para_table_ams_tmd3725.h"
#include "als_para_table_ams_tsl2540.h"
#include "als_para_table_ams_tsl2591.h"
#include "als_para_table_avago.h"
#include "als_para_table_liteon.h"
#include "als_para_table_rohm.h"
#include "als_para_table_sensortek.h"
#include "als_para_table_sensortek_stk3638.h"
#include "als_para_table_silergy.h"
#include "als_para_table_st.h"
#include "als_para_table_vishay.h"
#include "als_para_table_sip3623.h"
#include "als_para_table_sip3642.h"
#include "als_para_table_sip2326.h"
#include "als_para_table_ams_tcs3410.h"
#include "als_para_table_sip3510.h"
#include "als_para_table_hx32062se.h"
#include "als_para_table_syn3133.h"
#include "als_para_table_syn3136.h"
#include "als_sysfs.h"
#include "als_tp_color.h"
#include "contexthub_boot.h"
#include "contexthub_route.h"

static uint8_t charllotte_product[] = {BARRY, ALLEN, ALTB, LEM, ADY, HBN, GRL, ADL};

static bool is_charllotte_product(struct als_platform_data *pf_data)
{
	for (int i = 0; i < ARRAY_SIZE(charllotte_product); i++) {
		if (charllotte_product[i] == pf_data->als_phone_type) {
			hwlog_info("%s true, phone_type=%d\n", __func__, pf_data->als_phone_type);
			return true;
		}
	}
	return false;
}

static void als_set_extend_data(struct als_platform_data *pf_data,
	struct als_device_info *dev_info, const void *data, uint32_t len, bool in_tab)
{
	hwlog_info("%s phone_type=%d phone_version=%d type=%d table_id=%d in_tab=%d\n",
		__func__, pf_data->als_phone_type, pf_data->als_phone_version,
		dev_info->chip_type, dev_info->table_id, (int32_t)in_tab);

	if (memcpy_s(pf_data->als_extend_data, sizeof(pf_data->als_extend_data),
		data, len) != EOK)
		hwlog_err("%s als memcpy_s als_extend_data error\n", __func__);

	dev_info->extend_para_set = 1;
}

void set_als_extend_prameters(struct als_platform_data *pf_data,
	struct als_device_info *dev_info,
	als_para_normal_table *tp_color_table,
	uint32_t arraysize)
{
	uint32_t i;
	u8 tplcd;
	als_para_normal_table *als_tab = tp_color_table;

	if (is_charllotte_product(pf_data))
		tplcd = get_multi_lcd_info(LCD_PANEL_MAIN);
	else
		tplcd = get_tplcd_manufacture();
	hwlog_info("%s phone_type=%d phone_version=%d tp_lcd_manufacture=%d tplcd=%d\n",
		__func__, pf_data->als_phone_type, pf_data->als_phone_version,
		get_tplcd_manufacture(), tplcd);

	if (als_tab == NULL)
		return;
	for (i = 0; i < arraysize; i++) {
		if ((als_tab->phone_type == pf_data->als_phone_type) &&
			(als_tab->phone_version == pf_data->als_phone_version) &&
			((als_tab->tp_manufacture == tplcd) ||
			(als_tab->tp_manufacture == TS_PANEL_UNKNOWN)))
			break;

		als_tab++;
	}

	als_tab -= ((i < arraysize) ? 0 : i);
	dev_info->min_thres = als_tab->als_para[als_tab->len - 1];
	/* 2: second to last is max thres */
	dev_info->max_thres = als_tab->als_para[als_tab->len - 2];

	dev_info->table_id = ((i < arraysize) ? i : 0);

	als_set_extend_data(pf_data, dev_info, als_tab->als_para,
		(sizeof(s16) * als_tab->len > sizeof(als_tab->als_para)) ?
		sizeof(als_tab->als_para) : (sizeof(s16) * als_tab->len),
		(i < arraysize));
}

void select_stk3638_als_data(struct als_platform_data *pf_data,
	struct als_device_info *dev_info)
{
	u8 phone_color = get_phone_color();
	uint32_t table_size;
	uint32_t i;
	stk3638_als_para_table *stk3638_tab = NULL;

	stk3638_tab = als_get_stk3638_first_table();
	table_size = als_get_stk3638_table_count();
	if (stk3638_tab == NULL)
		return;
	for (i = 0; i < table_size; i++) {
		if ((stk3638_tab->phone_type == pf_data->als_phone_type) &&
			(stk3638_tab->phone_version == pf_data->als_phone_version) &&
			(stk3638_tab->tp_manufacture == tp_manufacture ||
			stk3638_tab->tp_manufacture == DEFAULT_TPLCD) &&
			(stk3638_tab->tp_color == phone_color))
			break;

		stk3638_tab++;
	}

	stk3638_tab -= ((i < table_size) ? 0 : i);
	dev_info->min_thres = stk3638_tab->stk3638_para[STK3638_MIN_THRESHOLD_NUM];
	dev_info->max_thres = stk3638_tab->stk3638_para[STK3638_MAX_THRESHOLD_NUM];

	dev_info->table_id = ((i < table_size) ? i : 0);

	als_set_extend_data(pf_data, dev_info, stk3638_tab->stk3638_para,
		sizeof(stk3638_tab->stk3638_para), (i < table_size));
}

void select_liteon_ltr2594_als_data(struct als_platform_data *pf_data,
	struct als_device_info *dev_info)
{
	u8 phone_color = get_phone_color();
	uint32_t table_size;
	uint32_t i;
	ltr2594_als_para_table *ltr2594_tab = NULL;

	ltr2594_tab = als_get_ltr2594_first_table();
	table_size = als_get_ltr2594_table_count();
	if (ltr2594_tab == NULL)
		return;
	for (i = 0; i < table_size; i++) {
		if ((ltr2594_tab->phone_type == pf_data->als_phone_type) &&
			(ltr2594_tab->phone_version == pf_data->als_phone_version) &&
			(ltr2594_tab->tp_manufacture == tp_manufacture ||
			ltr2594_tab->tp_manufacture == DEFAULT_TPLCD) &&
			(ltr2594_tab->tp_color == phone_color))
			break;

		ltr2594_tab++;
	}

	ltr2594_tab -= ((i < table_size) ? 0 : i);
	dev_info->min_thres = ltr2594_tab->ltr2594_para[LTR2594_MIN_THRESHOLD_NUM];
	dev_info->max_thres = ltr2594_tab->ltr2594_para[LTR2594_MAX_THRESHOLD_NUM];

	dev_info->table_id = ((i < table_size) ? i : 0);

	als_set_extend_data(pf_data, dev_info, ltr2594_tab->ltr2594_para,
		sizeof(ltr2594_tab->ltr2594_para), (i < table_size));
}

void set_vd6281_als_extend_prameters(struct als_platform_data *pf_data,
	struct als_device_info *dev_info)
{
	u8 phone_color = get_phone_color();
	uint32_t table_size;
	uint32_t i;
	vd6281_als_para_table *vd6281_tab = NULL;

	vd6281_tab = als_get_vd6281_first_table();
	table_size = als_get_vd6281_table_count();
	if (vd6281_tab == NULL)
		return;
	for (i = 0; i < table_size; i++) {
		if ((vd6281_tab->phone_type == pf_data->als_phone_type) &&
			(vd6281_tab->phone_version == pf_data->als_phone_version) &&
			(vd6281_tab->tp_manufacture == tp_manufacture ||
			vd6281_tab->tp_manufacture == TS_PANEL_UNKNOWN) &&
			(vd6281_tab->tp_color == phone_color))
			break;

		vd6281_tab++;
	}

	vd6281_tab -= ((i < table_size) ? 0 : i);
	dev_info->min_thres = vd6281_tab->vd6281_para[VD6281_MIN_THRESHOLD_NUM];
	dev_info->max_thres = vd6281_tab->vd6281_para[VD6281_MAX_THRESHOLD_NUM];

	dev_info->table_id = ((i < table_size) ? i : 0);

	als_set_extend_data(pf_data, dev_info, vd6281_tab->vd6281_para,
		sizeof(vd6281_tab->vd6281_para), (i < table_size));
}

void set_tsl2591_als_extend_prameters(struct als_platform_data *pf_data,
	struct als_device_info *dev_info)
{
	u8 phone_color = get_phone_color();
	uint32_t table_size;
	uint32_t i;
	tsl2591_als_para_table *tsl2591_tab = NULL;

	tsl2591_tab = als_get_tsl2591_first_table();
	table_size = als_get_tsl2591_table_count();
	if (tsl2591_tab == NULL)
		return;
	for (i = 0; i < table_size; i++) {
		if ((tsl2591_tab->phone_type == pf_data->als_phone_type) &&
			(tsl2591_tab->phone_version == pf_data->als_phone_version) &&
			(tsl2591_tab->tp_manufacture ==
			get_tplcd_manufacture() ||
			tsl2591_tab->tp_manufacture == TS_PANEL_UNKNOWN) &&
			(tsl2591_tab->tp_color == phone_color ||
			tsl2591_tab->tp_color == OTHER))
			break;

		tsl2591_tab++;
	}

	tsl2591_tab -= ((i < table_size) ? 0 : i);
	dev_info->min_thres = tsl2591_tab->tsl2591_para[TSL2591_MIN_THRESHOLD_NUM];
	dev_info->max_thres = tsl2591_tab->tsl2591_para[TSL2591_MAX_THRESHOLD_NUM];

	dev_info->table_id = ((i < table_size) ? i : 0);

	als_set_extend_data(pf_data, dev_info, tsl2591_tab->tsl2591_para,
		sizeof(tsl2591_tab->tsl2591_para), (i < table_size));
}

void set_bh1726_als_extend_prameters(struct als_platform_data *pf_data,
	struct als_device_info *dev_info)
{
	uint32_t table_size;
	uint32_t i;
	bh1726_als_para_table *bh1726_tab = NULL;

	bh1726_tab = als_get_bh1726_first_table();
	table_size = als_get_bh1726_table_count();
	if (bh1726_tab == NULL)
		return;
	for (i = 0; i < table_size; i++) {
		if ((bh1726_tab->phone_type == pf_data->als_phone_type) &&
			(bh1726_tab->phone_version == pf_data->als_phone_version) &&
			(bh1726_tab->tp_manufacture ==
			get_tplcd_manufacture() ||
			bh1726_tab->tp_manufacture == TS_PANEL_UNKNOWN))
			break;

		bh1726_tab++;
	}

	bh1726_tab -= ((i < table_size) ? 0 : i);
	dev_info->min_thres = bh1726_tab->bh1726_para[BH1726_MIN_THRESHOLD_NUM];
	dev_info->max_thres = bh1726_tab->bh1726_para[BH1726_MAX_THRESHOLD_NUM];

	dev_info->table_id = ((i < table_size) ? i : 0);

	als_set_extend_data(pf_data, dev_info, bh1726_tab->bh1726_para,
		sizeof(bh1726_tab->bh1726_para), (i < table_size));
}

void set_rpr531_als_extend_prameters(struct als_platform_data *pf_data,
	struct als_device_info *dev_info)
{
	uint32_t table_size;
	uint32_t i;
	rpr531_als_para_table *rpr531_tab = NULL;

	rpr531_tab = als_get_rpr531_first_table();
	table_size = als_get_rpr531_table_count();
	if (rpr531_tab == NULL)
		return;
	for (i = 0; i < table_size; i++) {
		if ((rpr531_tab->phone_type == pf_data->als_phone_type) &&
			(rpr531_tab->phone_version == pf_data->als_phone_version) &&
			(rpr531_tab->tp_manufacture == tp_manufacture ||
			rpr531_tab->tp_manufacture == TS_PANEL_UNKNOWN))
			break;

		rpr531_tab++;
	}

	rpr531_tab -= ((i < table_size) ? 0 : i);
	dev_info->min_thres = rpr531_tab->rpr531_para[RPR531_MIN_THRESHOLD_NUM];
	dev_info->max_thres = rpr531_tab->rpr531_para[RPR531_MAX_THRESHOLD_NUM];

	dev_info->table_id = ((i < table_size) ? i : 0);

	als_set_extend_data(pf_data, dev_info, rpr531_tab->rpr531_para,
		sizeof(rpr531_tab->rpr531_para), (i < table_size));
}

void set_pinhole_als_extend_parameters(
	struct als_platform_data *pf_data,
	struct als_device_info *dev_info)
{
	uint32_t table_size;
	uint32_t i;
	pinhole_als_para_table *pinhole_tab = NULL;

	pinhole_tab = als_get_pinhole_first_table();
	table_size = als_get_pinhole_table_count();
	if (pinhole_tab == NULL)
		return;
	for (i = 0; i < table_size; i++) {
		if ((pinhole_tab->phone_type == pf_data->als_phone_type) &&
			(pinhole_tab->phone_version == pf_data->als_phone_version) &&
			((dev_info->chip_type == ALS_CHIP_APDS9922 &&
				pinhole_tab->sens_name == APDS9922) ||
			(dev_info->chip_type == ALS_CHIP_LTR578 &&
				pinhole_tab->sens_name == LTR578)) &&
			(pinhole_tab->tp_manufacture == tp_manufacture ||
			pinhole_tab->tp_manufacture == TS_PANEL_UNKNOWN))
			break;

		pinhole_tab++;
	}

	pinhole_tab -= ((i < table_size) ? 0 : i);
	dev_info->min_thres =
		pinhole_tab->pinhole_para[LTR578_APDS9922_MIN_THRESHOLD_NUM];
	dev_info->max_thres =
		pinhole_tab->pinhole_para[LTR578_APDS9922_MAX_THRESHOLD_NUM];

	dev_info->table_id = ((i < table_size) ? i : 0);

	als_set_extend_data(pf_data, dev_info, pinhole_tab->pinhole_para,
		sizeof(pinhole_tab->pinhole_para), (i < table_size));
}

void set_tmd2745_als_extend_parameters(
	struct als_platform_data *pf_data,
	struct als_device_info *dev_info)
{
	uint32_t table_size;
	uint32_t i;
	tmd2745_als_para_table *tmd2745_tab = NULL;

	tmd2745_tab = als_get_tmd2745_first_table();
	table_size = als_get_tmd2745_table_count();
	if (tmd2745_tab == NULL)
		return;
	for (i = 0; i < table_size; i++) {
		if ((tmd2745_tab->phone_type == pf_data->als_phone_type) &&
			(tmd2745_tab->phone_version == pf_data->als_phone_version) &&
			(tmd2745_tab->tp_manufacture == tp_manufacture))
			break;

		tmd2745_tab++;
	}

	tmd2745_tab -= ((i < table_size) ? 0 : i);
	dev_info->min_thres = tmd2745_tab->als_para[TMD2745_MIN_THRESHOLD_NUM];
	dev_info->max_thres = tmd2745_tab->als_para[TMD2745_MAX_THRESHOLD_NUM];

	dev_info->table_id = ((i < table_size) ? i : 0);

	als_set_extend_data(pf_data, dev_info, tmd2745_tab->als_para,
		sizeof(tmd2745_tab->als_para), (i < table_size));
}

void select_rohm_als_data(struct als_platform_data *pf_data,
	struct als_device_info *dev_info)
{
	u8 phone_color = get_phone_color();
	uint32_t table_size;
	uint32_t i;
	bh1745_als_para_table *bh1745_tab = NULL;

	bh1745_tab = als_get_bh1745_first_table();
	table_size = als_get_bh1745_table_count();
	if (bh1745_tab == NULL)
		return;
	for (i = 0; i < table_size; i++) {
		if ((bh1745_tab->phone_type == pf_data->als_phone_type) &&
			(bh1745_tab->phone_version == pf_data->als_phone_version) &&
			(bh1745_tab->tp_lcd_manufacture ==
			get_tplcd_manufacture() ||
			bh1745_tab->tp_lcd_manufacture == DEFAULT_TPLCD) &&
			(bh1745_tab->tp_color == phone_color))
			break;

		bh1745_tab++;
	}

	bh1745_tab -= ((i < table_size) ? 0 : i);
	dev_info->min_thres = bh1745_tab->bh745_para[BH1745_MIN_THRESHOLD_NUM];
	dev_info->max_thres = bh1745_tab->bh745_para[BH1745_MAX_THRESHOLD_NUM];

	dev_info->table_id = ((i < table_size) ? i : 0);

	als_set_extend_data(pf_data, dev_info, bh1745_tab->bh745_para,
		sizeof(bh1745_tab->bh745_para), (i < table_size));
}

void select_rohmbh1749_als_data(struct als_platform_data *pf_data,
	struct als_device_info *dev_info)
{
	u8 phone_color = get_phone_color();
	uint32_t table_size;
	uint32_t i;
	bh1749_als_para_table_t *h1749_tab = NULL;

	h1749_tab = als_get_bh1749_first_table();
	table_size = als_get_bh1749_table_count();
	if (h1749_tab == NULL)
		return;
	for (i = 0; i < table_size; i++) {
		if ((h1749_tab->phone_type == pf_data->als_phone_type) &&
			(h1749_tab->phone_version == pf_data->als_phone_version) &&
			(h1749_tab->tp_lcd_manufacture ==
			get_tplcd_manufacture() ||
			h1749_tab->tp_lcd_manufacture == DEFAULT_TPLCD) &&
			(h1749_tab->tp_color == phone_color))
			break;

		h1749_tab++;
	}

	h1749_tab -= ((i < table_size) ? 0 : i);
	dev_info->min_thres = h1749_tab->bh1749_para[BH1749_MIN_THRESHOLD_NUM];
	dev_info->max_thres = h1749_tab->bh1749_para[BH1749_MAX_THRESHOLD_NUM];

	dev_info->table_id = ((i < table_size) ? i : 0);

	als_set_extend_data(pf_data, dev_info, h1749_tab->bh1749_para,
		sizeof(h1749_tab->bh1749_para), (i < table_size));
}

void select_rohmbu27006muc_als_data(struct als_platform_data *pf_data,
	struct als_device_info *dev_info)
{
	u8 phone_color = get_phone_color();
	uint32_t table_size;
	uint32_t i;
	bu27006muc_als_para_table_t *bu27006muc_tab = NULL;

	bu27006muc_tab = als_get_bu27006muc_first_table();
	table_size = als_get_bu27006muc_table_count();
	if (bu27006muc_tab == NULL)
		return;
	for (i = 0; i < table_size; i++) {
		if ((bu27006muc_tab->phone_type == pf_data->als_phone_type) &&
		    (bu27006muc_tab->phone_version == pf_data->als_phone_version) &&
		    (bu27006muc_tab->tp_lcd_manufacture ==
		    get_tplcd_manufacture() ||
		     bu27006muc_tab->tp_lcd_manufacture == DEFAULT_TPLCD) &&
		    (bu27006muc_tab->tp_color == phone_color))
			break;

		bu27006muc_tab++;
	}

	bu27006muc_tab -= ((i < table_size) ? 0 : i);
	dev_info->min_thres =
		bu27006muc_tab->bu27006muc_para[BU27006MUC_MIN_THRESHOLD_NUM];
	dev_info->max_thres =
		bu27006muc_tab->bu27006muc_para[BU27006MUC_MAX_THRESHOLD_NUM];

	dev_info->table_id = ((i < table_size) ? i : 0);

	als_set_extend_data(pf_data, dev_info, bu27006muc_tab->bu27006muc_para,
		sizeof(bu27006muc_tab->bu27006muc_para), (i < table_size));
}

void select_avago_als_data(struct als_platform_data *pf_data,
	struct als_device_info *dev_info)
{
	u8 phone_color = get_phone_color();
	uint32_t table_size;
	uint32_t i;
	apds9251_als_para_table *apds9251_tab = NULL;

	apds9251_tab = als_get_apds9251_first_table();
	table_size = als_get_apds9251_table_count();
	if (apds9251_tab == NULL)
		return;
	for (i = 0; i < table_size; i++) {
		if ((apds9251_tab->phone_type == pf_data->als_phone_type) &&
			(apds9251_tab->phone_version == pf_data->als_phone_version) &&
			(apds9251_tab->tp_lcd_manufacture ==
			get_tplcd_manufacture() ||
			apds9251_tab->tp_lcd_manufacture == DEFAULT_TPLCD) &&
			(apds9251_tab->tp_color == phone_color))
			break;

		apds9251_tab++;
	}

	apds9251_tab -= ((i < table_size) ? 0 : i);
	dev_info->min_thres =
		apds9251_tab->apds251_para[APDS9251_MIN_THRESHOLD_NUM];
	dev_info->max_thres =
		apds9251_tab->apds251_para[APDS9251_MAX_THRESHOLD_NUM];

	dev_info->table_id = ((i < table_size) ? i : 0);

	als_set_extend_data(pf_data, dev_info, apds9251_tab->apds251_para,
		sizeof(apds9251_tab->apds251_para), (i < table_size));
}

void select_apds9999_als_data(struct als_platform_data *pf_data,
	struct als_device_info *dev_info)
{
	u8 phone_color = get_phone_color();
	uint32_t table_size;
	uint32_t i;
	apds9999_als_para_table *apds9999_tab = NULL;

	apds9999_tab = als_get_apds9999_first_table();
	table_size = als_get_apds9999_table_count();
	if (apds9999_tab == NULL)
		return;
	for (i = 0; i < table_size; i++) {
		if ((apds9999_tab->phone_type == pf_data->als_phone_type) &&
			(apds9999_tab->phone_version == pf_data->als_phone_version) &&
			(apds9999_tab->tp_lcd_manufacture ==
			get_tplcd_manufacture() ||
			apds9999_tab->tp_lcd_manufacture == DEFAULT_TPLCD) &&
			(apds9999_tab->tp_color == phone_color))
			break;

		apds9999_tab++;
	}

	apds9999_tab -= ((i < table_size) ? 0 : i);
	dev_info->min_thres =
		apds9999_tab->apds9999_para[APDS9251_MIN_THRESHOLD_NUM];
	dev_info->max_thres =
		apds9999_tab->apds9999_para[APDS9251_MAX_THRESHOLD_NUM];

	dev_info->table_id = ((i < table_size) ? i : 0);

	als_set_extend_data(pf_data, dev_info, apds9999_tab->apds9999_para,
		sizeof(apds9999_tab->apds9999_para), (i < table_size));
}

void select_ams_tmd3725_als_data(struct als_platform_data *pf_data,
	struct als_device_info *dev_info)
{
	u8 phone_color = get_phone_color();
	uint32_t table_size;
	uint32_t i;
	tmd3725_als_para_table *tmd3725_tab = NULL;

	tmd3725_tab = als_get_tmd3725_first_table();
	table_size = als_get_tmd3725_table_count();
	if (tmd3725_tab == NULL)
		return;
	for (i = 0; i < table_size; i++) {
		if ((tmd3725_tab->phone_type == pf_data->als_phone_type) &&
			(tmd3725_tab->phone_version == pf_data->als_phone_version) &&
			(tmd3725_tab->tp_lcd_manufacture ==
			get_tplcd_manufacture() ||
			tmd3725_tab->tp_lcd_manufacture == DEFAULT_TPLCD) &&
			(tmd3725_tab->tp_color == phone_color))
			break;

		tmd3725_tab++;
	}

	tmd3725_tab -= ((i < table_size) ? 0 : i);
	dev_info->min_thres = tmd3725_tab->tmd3725_para[TMD3725_MIN_THRESHOLD_NUM];
	dev_info->max_thres = tmd3725_tab->tmd3725_para[TMD3725_MAX_THRESHOLD_NUM];

	dev_info->table_id = ((i < table_size) ? i : 0);

	als_set_extend_data(pf_data, dev_info, tmd3725_tab->tmd3725_para,
		sizeof(tmd3725_tab->tmd3725_para), (i < table_size));
}

void select_ams_tmd3702_als_data(struct als_platform_data *pf_data,
	struct als_device_info *dev_info)
{
	u8 phone_color = get_phone_color();
	uint32_t table_size;
	uint32_t i;
	tmd3702_als_para_table *tmd3702_tab = NULL;

	tmd3702_tab = als_get_tmd3702_first_table();
	table_size = als_get_tmd3702_table_count();
	if (tmd3702_tab == NULL)
		return;
	for (i = 0; i < table_size; i++) {
		if ((tmd3702_tab->phone_type == pf_data->als_phone_type) &&
			(tmd3702_tab->phone_version == pf_data->als_phone_version) &&
			(tmd3702_tab->tp_lcd_manufacture ==
			get_tplcd_manufacture() ||
			tmd3702_tab->tp_lcd_manufacture == DEFAULT_TPLCD) &&
			(tmd3702_tab->tp_color == phone_color))
			break;

		tmd3702_tab++;
	}

	tmd3702_tab -= ((i < table_size) ? 0 : i);
	dev_info->min_thres = tmd3702_tab->tmd3702_para[TMD3702_MIN_THRESHOLD_NUM];
	dev_info->max_thres = tmd3702_tab->tmd3702_para[TMD3702_MAX_THRESHOLD_NUM];

	dev_info->table_id = ((i < table_size) ? i : 0);

	als_set_extend_data(pf_data, dev_info, tmd3702_tab->tmd3702_para,
		sizeof(tmd3702_tab->tmd3702_para), (i < table_size));
}

void select_ams_tcs3707_als_data(struct als_platform_data *pf_data,
	struct als_device_info *dev_info)
{
	u8 phone_color = get_phone_color();
	uint32_t table_size;
	uint32_t i;
	u8 tplcd;
	tcs3707_als_para_table *tcs3707_tab = NULL;
	hwlog_info("%s phone_type=%d phone_version=%d tp_lcd_manufacture=%d phone_color=%d\n",
		__func__, pf_data->als_phone_type, pf_data->als_phone_version,
		get_tplcd_manufacture_curr(), phone_color);
	if (pf_data->als_phone_type == PCE || pf_data->als_phone_type == XYAO) {
		init_tp_manufacture_curr();
		tplcd = get_tplcd_manufacture_curr();
		hwlog_info("retry read tplcd_manufacture=%d", tplcd);
	}
	tcs3707_tab = als_get_tcs3707_first_table();
	table_size = als_get_tcs3707_table_count();
	if (tcs3707_tab == NULL)
		return;
	for (i = 0; i < table_size; i++) {
		if ((tcs3707_tab->phone_type == pf_data->als_phone_type) &&
			(tcs3707_tab->phone_version == pf_data->als_phone_version) &&
			(tcs3707_tab->tp_lcd_manufacture ==
			get_tplcd_manufacture() ||
			tcs3707_tab->tp_lcd_manufacture == get_tplcd_manufacture_curr()) &&
			(tcs3707_tab->tp_color == phone_color))
			break;

		tcs3707_tab++;
	}

	tcs3707_tab -= ((i < table_size) ? 0 : i);
	dev_info->min_thres = tcs3707_tab->tcs3707_para[TCS3707_MIN_THRESHOLD_NUM];
	dev_info->max_thres = tcs3707_tab->tcs3707_para[TCS3707_MAX_THRESHOLD_NUM];

	dev_info->table_id = ((i < table_size) ? i : 0);

	als_set_extend_data(pf_data, dev_info, tcs3707_tab->tcs3707_para,
		sizeof(tcs3707_tab->tcs3707_para), (i < table_size));
}

static void select_ams_tcs3701_als_data_ex(struct als_platform_data *pf_data,
	struct als_device_info *dev_info, tcs3701_als_para_table *tcs3701_array,
	uint32_t table_size)
{
	u8 phone_color = get_phone_color();
	u8 tplcd;
	uint32_t i;
	tcs3701_als_para_table *tcs3701_tab = tcs3701_array;
	if (tcs3701_array == NULL)
		return;

	if (is_charllotte_product(pf_data))
		tplcd = get_multi_lcd_info(LCD_PANEL_MAIN);
	else
		tplcd = get_tplcd_manufacture();
	hwlog_info("%s phone_type=%d phone_version=%d tp_lcd_manufacture=%d tplcd=%d, phone_color=%d\n",
		__func__, pf_data->als_phone_type, pf_data->als_phone_version,
		get_tplcd_manufacture(), tplcd, phone_color);

	for (i = 0; i < table_size; i++) {
		if ((tcs3701_tab->phone_type == pf_data->als_phone_type) &&
			(tcs3701_tab->phone_version == pf_data->als_phone_version) &&
			(tcs3701_tab->tp_lcd_manufacture == tplcd ||
			tcs3701_tab->tp_lcd_manufacture == DEFAULT_TPLCD ||
			tcs3701_tab->tp_lcd_manufacture == TS_PANEL_UNKNOWN) &&
			(tcs3701_tab->tp_color == phone_color ||
			tcs3701_tab->tp_color == OTHER))
			break;

		tcs3701_tab++;
	}

	tcs3701_tab -= ((i < table_size) ? 0 : i);
	dev_info->min_thres = tcs3701_tab->tcs3701_para[TCS3701_MIN_THRESHOLD_NUM];
	dev_info->max_thres = tcs3701_tab->tcs3701_para[TCS3701_MAX_THRESHOLD_NUM];

	dev_info->table_id = ((i < table_size) ? i : 0);

	pf_data->tp_info = tplcd;

	als_set_extend_data(pf_data, dev_info, tcs3701_tab->tcs3701_para,
		sizeof(tcs3701_tab->tcs3701_para), (i < table_size));
}

static void select_sip3623_als_data_ex(struct als_platform_data *pf_data,
	struct als_device_info *dev_info, sip3623_als_para_table *sip3623_array,
	uint32_t table_size)
{
	u8 phone_color = get_phone_color();
	u8 tplcd;
	uint32_t i;
	sip3623_als_para_table *sip3623_tab = sip3623_array;

	if (!sip3623_array)
		return;

	if (is_charllotte_product(pf_data))
		tplcd = get_multi_lcd_info(LCD_PANEL_MAIN);
	else
		tplcd = get_tplcd_manufacture();
	hwlog_info("%s phone_type=%d phone_version=%d tp_lcd_manufacture=%d tplcd=%d, phone_color=%d\n",
		__func__, pf_data->als_phone_type, pf_data->als_phone_version,
		get_tplcd_manufacture(), tplcd, phone_color);

	for (i = 0; i < table_size; i++) {
		if ((sip3623_tab->phone_type == pf_data->als_phone_type) &&
			(sip3623_tab->phone_version == pf_data->als_phone_version) &&
			(sip3623_tab->tp_lcd_manufacture == tplcd ||
			sip3623_tab->tp_lcd_manufacture == DEFAULT_TPLCD ||
			sip3623_tab->tp_lcd_manufacture == TS_PANEL_UNKNOWN) &&
			(sip3623_tab->tp_color == phone_color ||
			sip3623_tab->tp_color == OTHER))
			break;
		sip3623_tab++;
	}

	sip3623_tab -= ((i < table_size) ? 0 : i);
	dev_info->min_thres = sip3623_tab->sip3623_para[SIP3623_MIN_THRESHOLD_NUM];
	dev_info->max_thres = sip3623_tab->sip3623_para[SIP3623_MAX_THRESHOLD_NUM];
	dev_info->table_id = ((i < table_size) ? i : 0);
	pf_data->tp_info = get_tplcd_manufacture();

	als_set_extend_data(pf_data, dev_info, sip3623_tab->sip3623_para,
		sizeof(sip3623_tab->sip3623_para), (i < table_size));
}

static void select_sip3642_als_data_ex(struct als_platform_data *pf_data,
	struct als_device_info *dev_info, sip3642_als_para_table *sip3642_array,
	uint32_t table_size)
{
	u8 phone_color = get_phone_color();
	u8 tplcd;
	uint32_t i;
	sip3642_als_para_table *sip3642_tab = sip3642_array;

	if (!sip3642_array)
		return;

	if (is_charllotte_product(pf_data))
		tplcd = get_multi_lcd_info(LCD_PANEL_MAIN);
	else
		tplcd = get_tplcd_manufacture();
	hwlog_info("%s phone_type=%d phone_version=%d tp_lcd_manufacture=%d tplcd=%d, phone_color=%d\n",
		__func__, pf_data->als_phone_type, pf_data->als_phone_version,
		get_tplcd_manufacture(), tplcd, phone_color);

	for (i = 0; i < table_size; i++) {
		if ((sip3642_tab->phone_type == pf_data->als_phone_type) &&
			(sip3642_tab->phone_version == pf_data->als_phone_version) &&
			(sip3642_tab->tp_lcd_manufacture == tplcd ||
			sip3642_tab->tp_lcd_manufacture == DEFAULT_TPLCD ||
			sip3642_tab->tp_lcd_manufacture == TS_PANEL_UNKNOWN) &&
			(sip3642_tab->tp_color == phone_color ||
			sip3642_tab->tp_color == OTHER))
			break;
		sip3642_tab++;
	}

	sip3642_tab -= ((i < table_size) ? 0 : i);
	dev_info->min_thres = sip3642_tab->sip3642_para[SIP3623_MIN_THRESHOLD_NUM];
	dev_info->max_thres = sip3642_tab->sip3642_para[SIP3623_MAX_THRESHOLD_NUM];
	dev_info->table_id = ((i < table_size) ? i : 0);
	pf_data->tp_info = get_tplcd_manufacture();

	als_set_extend_data(pf_data, dev_info, sip3642_tab->sip3642_para,
		sizeof(sip3642_tab->sip3642_para), (i < table_size));
}
static void select_sip2326_als_data_ex(struct als_platform_data *pf_data,
	struct als_device_info *dev_info, sip2326_als_para_table *sip2326_array,
	uint32_t table_size)
{
	u8 phone_color = get_phone_color();
	u8 tplcd;
	uint32_t i;
	sip2326_als_para_table *sip2326_tab = sip2326_array;

	if (!sip2326_array)
		return;

	tplcd = get_tplcd_manufacture();
	for (i = 0; i < table_size; i++) {
		if ((sip2326_tab->phone_type == pf_data->als_phone_type) &&
			(sip2326_tab->phone_version == pf_data->als_phone_version) &&
			(sip2326_tab->tp_lcd_manufacture == tplcd ||
			sip2326_tab->tp_lcd_manufacture == DEFAULT_TPLCD ||
			sip2326_tab->tp_lcd_manufacture == TS_PANEL_UNKNOWN) &&
			(sip2326_tab->tp_color == phone_color ||
			sip2326_tab->tp_color == OTHER))
			break;
		sip2326_tab++;
	}

	sip2326_tab -= ((i < table_size) ? 0 : i);
	dev_info->min_thres = sip2326_tab->sip2326_para[SIP2326_MIN_THRESHOLD_NUM];
	dev_info->max_thres = sip2326_tab->sip2326_para[SIP2326_MAX_THRESHOLD_NUM];
	dev_info->table_id = ((i < table_size) ? i : 0);
	pf_data->tp_info = get_tplcd_manufacture();

	hwlog_info("tpinfo %d\n", pf_data->tp_info);
	als_set_extend_data(pf_data, dev_info, sip2326_tab->sip2326_para,
		sizeof(sip2326_tab->sip2326_para), (i < table_size));
}

static void select_tcs3410_als_data_ex(struct als_platform_data *pf_data,
	struct als_device_info *dev_info, tcs3410_als_para_table *tcs3410_array,
	uint32_t table_size)
{
	u8 phone_color = get_phone_color();
	u8 tplcd;
	uint32_t i;
	tcs3410_als_para_table *tcs3410_tab = tcs3410_array;

	if (!tcs3410_array)
		return;

	tplcd = get_tplcd_manufacture();
	for (i = 0; i < table_size; i++) {
		if ((tcs3410_tab->phone_type == pf_data->als_phone_type) &&
			(tcs3410_tab->phone_version == pf_data->als_phone_version) &&
			(tcs3410_tab->tp_lcd_manufacture == tplcd ||
			tcs3410_tab->tp_lcd_manufacture == DEFAULT_TPLCD ||
			tcs3410_tab->tp_lcd_manufacture == TS_PANEL_UNKNOWN) &&
			(tcs3410_tab->tp_color == phone_color ||
			tcs3410_tab->tp_color == OTHER))
			break;
		tcs3410_tab++;
	}

	tcs3410_tab -= ((i < table_size) ? 0 : i);
	dev_info->min_thres = tcs3410_tab->tcs3410_para[TCS3410_MIN_THRESHOLD_NUM];
	dev_info->max_thres = tcs3410_tab->tcs3410_para[TCS3410_MAX_THRESHOLD_NUM];
	dev_info->table_id = ((i < table_size) ? i : 0);
	pf_data->tp_info = get_tplcd_manufacture();

	hwlog_info("%s tpinfo %d\n", __func__, pf_data->tp_info);
	als_set_extend_data(pf_data, dev_info, tcs3410_tab->tcs3410_para,
		sizeof(tcs3410_tab->tcs3410_para), (i < table_size));
}

static void select_sip3510_als_data_ex(struct als_platform_data *pf_data,
	struct als_device_info *dev_info, sip3510_als_para_table *sip3510_array,
	uint32_t table_size)
{
	u8 phone_color = get_phone_color();
	uint32_t i;
	sip3510_als_para_table *sip3510_tab = sip3510_array;

	if (!sip3510_array)
		return;

	for (i = 0; i < table_size; i++) {
		if ((sip3510_tab->phone_type == pf_data->als_phone_type) &&
			(sip3510_tab->phone_version == pf_data->als_phone_version) &&
			(sip3510_tab->tp_lcd_manufacture ==
			get_tplcd_manufacture() ||
			sip3510_tab->tp_lcd_manufacture == DEFAULT_TPLCD ||
			sip3510_tab->tp_lcd_manufacture == TS_PANEL_UNKNOWN) &&
			(sip3510_tab->tp_color == phone_color ||
			sip3510_tab->tp_color == OTHER))
			break;

		sip3510_tab++;
	}

	sip3510_tab -= ((i < table_size) ? 0 : i);
	dev_info->min_thres = sip3510_tab->sip3510_para[SIP3510_MIN_THRESHOLD_NUM];
	dev_info->max_thres = sip3510_tab->sip3510_para[SIP3510_MAX_THRESHOLD_NUM];
	dev_info->table_id = ((i < table_size) ? i : 0);
	pf_data->tp_info = get_tplcd_manufacture();

	als_set_extend_data(pf_data, dev_info, sip3510_tab->sip3510_para,
		sizeof(sip3510_tab->sip3510_para), (i < table_size));
}

static void select_hx32062se_als_data_ex(struct als_platform_data *pf_data,
	struct als_device_info *dev_info, hx32062se_als_para_table *hx32062se_array,
	uint32_t table_size)
{
	u8 phone_color = get_phone_color();
	uint32_t i;
	hx32062se_als_para_table *hx32062se_tab = hx32062se_array;

	if (!hx32062se_array)
		return;

	for (i = 0; i < table_size; i++) {
		if ((hx32062se_tab->phone_type == pf_data->als_phone_type) &&
			(hx32062se_tab->phone_version == pf_data->als_phone_version) &&
			(hx32062se_tab->tp_lcd_manufacture ==
			get_tplcd_manufacture() ||
			hx32062se_tab->tp_lcd_manufacture == DEFAULT_TPLCD ||
			hx32062se_tab->tp_lcd_manufacture == TS_PANEL_UNKNOWN) &&
			(hx32062se_tab->tp_color == phone_color ||
			hx32062se_tab->tp_color == OTHER))
			break;

		hx32062se_tab++;
	}

	hx32062se_tab -= ((i < table_size) ? 0 : i);
	dev_info->min_thres = hx32062se_tab->hx32062se_para[HX32062SE_MIN_THRESHOLD_NUM];
	dev_info->max_thres = hx32062se_tab->hx32062se_para[HX32062SE_MAX_THRESHOLD_NUM];
	dev_info->table_id = ((i < table_size) ? i : 0);
	pf_data->tp_info = get_tplcd_manufacture();

	als_set_extend_data(pf_data, dev_info, hx32062se_tab->hx32062se_para,
		sizeof(hx32062se_tab->hx32062se_para), (i < table_size));
}

static void select_syn3133_als_data_ex(struct als_platform_data *pf_data,
	struct als_device_info *dev_info, syn3133_als_para_table *syn3133_array,
	uint32_t table_size)
{
	u8 phone_color = get_phone_color();
	u8 tplcd;
	uint32_t i;
	syn3133_als_para_table *syn3133_tab = syn3133_array;

	if (!syn3133_array)
		return;

	if (is_charllotte_product(pf_data))
		tplcd = get_multi_lcd_info(LCD_PANEL_MAIN);
	else
		tplcd = get_tplcd_manufacture();
	hwlog_info("%s phone_type=%d phone_version=%d tp_lcd_manufacture=%d tplcd=%d, phone_color=%d\n",
		__func__, pf_data->als_phone_type, pf_data->als_phone_version,
		get_tplcd_manufacture(), tplcd, phone_color);

	for (i = 0; i < table_size; i++) {
		if ((syn3133_tab->phone_type == pf_data->als_phone_type) &&
			(syn3133_tab->phone_version == pf_data->als_phone_version) &&
			(syn3133_tab->tp_lcd_manufacture == tplcd ||
			syn3133_tab->tp_lcd_manufacture == DEFAULT_TPLCD ||
			syn3133_tab->tp_lcd_manufacture == TS_PANEL_UNKNOWN) &&
			(syn3133_tab->tp_color == phone_color ||
			syn3133_tab->tp_color == OTHER))
			break;

		syn3133_tab++;
	}

	syn3133_tab -= ((i < table_size) ? 0 : i);
	dev_info->min_thres = syn3133_tab->syn3133_para[SYN3133_MIN_THRESHOLD_NUM];
	dev_info->max_thres = syn3133_tab->syn3133_para[SYN3133_MAX_THRESHOLD_NUM];
	dev_info->table_id = ((i < table_size) ? i : 0);
	pf_data->tp_info = tplcd;

	als_set_extend_data(pf_data, dev_info, syn3133_tab->syn3133_para,
		sizeof(syn3133_tab->syn3133_para), (i < table_size));
}

static void select_syn3136_als_data_ex(struct als_platform_data *pf_data,
	struct als_device_info *dev_info, syn3136_als_para_table *syn3136_array,
	uint32_t table_size)
{
	u8 phone_color = get_phone_color();
	uint32_t i;
	syn3136_als_para_table *syn3136_tab = syn3136_array;

	if (!syn3136_array)
		return;

	for (i = 0; i < table_size; i++) {
		if ((syn3136_tab->phone_type == pf_data->als_phone_type) &&
			(syn3136_tab->phone_version == pf_data->als_phone_version) &&
			(syn3136_tab->tp_lcd_manufacture ==
			get_tplcd_manufacture() ||
			syn3136_tab->tp_lcd_manufacture == DEFAULT_TPLCD ||
			syn3136_tab->tp_lcd_manufacture == TS_PANEL_UNKNOWN) &&
			(syn3136_tab->tp_color == phone_color ||
			syn3136_tab->tp_color == OTHER))
			break;

		syn3136_tab++;
	}

	syn3136_tab -= ((i < table_size) ? 0 : i);
	dev_info->min_thres = syn3136_tab->syn3136_para[SYN3136_MIN_THRESHOLD_NUM];
	dev_info->max_thres = syn3136_tab->syn3136_para[SYN3136_MAX_THRESHOLD_NUM];
	dev_info->table_id = ((i < table_size) ? i : 0);
	pf_data->tp_info = get_tplcd_manufacture();

	als_set_extend_data(pf_data, dev_info, syn3136_tab->syn3136_para,
		sizeof(syn3136_tab->syn3136_para), (i < table_size));
}

static void select_ams_tsl540_als_data_ex(struct als_platform_data *pf_data,
	struct als_device_info *dev_info, tsl2540_als_para_table *tsl2540_array,
	uint32_t table_size)
{
	u8 phone_color = get_phone_color();
	uint32_t i;
	tsl2540_als_para_table *tsl2540_tab = tsl2540_array;

	if (!tsl2540_array)
		return;

	for (i = 0; i < table_size; i++) {
		if ((tsl2540_tab->phone_type == pf_data->als_phone_type) &&
			(tsl2540_tab->phone_version == pf_data->als_phone_version) &&
			(tsl2540_tab->tp_lcd_manufacture ==
			get_tplcd_manufacture() ||
			tsl2540_tab->tp_lcd_manufacture == DEFAULT_TPLCD ||
			tsl2540_tab->tp_lcd_manufacture == TS_PANEL_UNKNOWN) &&
			(tsl2540_tab->tp_color == phone_color ||
			tsl2540_tab->tp_color == OTHER))
			break;

		tsl2540_tab++;
	}

	tsl2540_tab -= ((i < table_size) ? 0 : i);
	dev_info->min_thres = tsl2540_tab->tsl2540_para[TSL2540_MIN_THRESHOLD_NUM];
	dev_info->max_thres = tsl2540_tab->tsl2540_para[TSL2540_MAX_THRESHOLD_NUM];
	dev_info->table_id = ((i < table_size) ? i : 0);
	pf_data->tp_info = get_tplcd_manufacture();

	als_set_extend_data(pf_data, dev_info, tsl2540_tab->tsl2540_para,
		sizeof(tsl2540_tab->tsl2540_para), (i < table_size));
}

void select_silergy_sy3133_als_data(struct als_platform_data *pf_data,
	struct als_device_info *dev_info)
{
	u8 phone_color = get_phone_color();
	uint32_t table_size;
	uint32_t i;
	u8 tplcd;
	sy3133_als_para_table *sy3133_tab = NULL;
	hwlog_info("%s phone_type=%d phone_version=%d tp_lcd_manufacture=%d phone_color=%d\n",
		__func__, pf_data->als_phone_type, pf_data->als_phone_version,
		get_tplcd_manufacture_curr(), phone_color);
	if (pf_data->als_phone_type == PCE || pf_data->als_phone_type == XYAO) {
		init_tp_manufacture_curr();
		tplcd = get_tplcd_manufacture_curr();
		hwlog_info("retry read tplcd_manufacture=%d", tplcd);
	}
	sy3133_tab = als_get_sy3133_first_table();
	table_size = als_get_sy3133_table_count();
	if (sy3133_tab == NULL)
		return;
	for (i = 0; i < table_size; i++) {
		if ((sy3133_tab->phone_type == pf_data->als_phone_type) &&
			(sy3133_tab->phone_version == pf_data->als_phone_version) &&
			(sy3133_tab->tp_lcd_manufacture ==
			get_tplcd_manufacture_curr()) &&
			(sy3133_tab->tp_color == phone_color))
			break;

		sy3133_tab++;
	}

	sy3133_tab -= ((i < table_size) ? 0 : i);
	dev_info->min_thres = sy3133_tab->sy3133_para[SY3133_MIN_THRESHOLD_NUM];
	dev_info->max_thres = sy3133_tab->sy3133_para[SY3133_MAX_THRESHOLD_NUM];

	dev_info->table_id = ((i < table_size) ? i : 0);

	als_set_extend_data(pf_data, dev_info, sy3133_tab->sy3133_para,
		sizeof(sy3133_tab->sy3133_para), (i < table_size));
}

void select_ams_tcs3701_als_data(struct als_platform_data *pf_data,
	struct als_device_info *dev_info)
{
	select_ams_tcs3701_als_data_ex(pf_data, dev_info,
		als_get_tcs3701_first_table(),
		als_get_tcs3701_table_count());
}

void select_ams_tcs3708_als_data(struct als_platform_data *pf_data,
	struct als_device_info *dev_info)
{
	select_ams_tcs3701_als_data_ex(pf_data, dev_info,
		als_get_tcs3708_first_table(),
		als_get_tcs3708_table_count());
}

void select_ams_tcs3718_als_data(struct als_platform_data *pf_data,
	struct als_device_info *dev_info)
{
	select_ams_tcs3701_als_data_ex(pf_data, dev_info,
		als_get_tcs3718_first_table(),
		als_get_tcs3718_table_count());
}

void select_sip3623_als_data(struct als_platform_data *pf_data,
	struct als_device_info *dev_info)
{
	select_sip3623_als_data_ex(pf_data, dev_info,
		als_get_sip3623_first_table(),
		als_get_sip3623_table_count());
}

void select_sip3642_als_data(struct als_platform_data *pf_data,
	struct als_device_info *dev_info)
{
	select_sip3642_als_data_ex(pf_data, dev_info,
		als_get_sip3642_first_table(),
		als_get_sip3642_table_count());
}

void select_sip2326_als_data(struct als_platform_data *pf_data,
	struct als_device_info *dev_info)
{
	uint32_t table_count;
	sip2326_als_para_table *first_table = NULL;

	if (dev_info->als_dev_index == 0) { // ALS_DEV_ID_0
		first_table = als_get_sip2326_first_table();
		table_count = als_get_sip2326_table_count();
	} else { // ALS_DEV_ID_1
		first_table = als1_get_sip2326_first_table();
		table_count = als1_get_sip2326_table_count();
	}
	if (first_table == NULL)
		return;
	hwlog_info("%s als_idx %d\n", __func__, dev_info->als_dev_index);
	select_sip2326_als_data_ex(pf_data, dev_info, first_table, table_count);
}

void select_tcs3410_als_data(struct als_platform_data *pf_data,
	struct als_device_info *dev_info)
{
	uint32_t table_count;
	tcs3410_als_para_table *first_table = NULL;

	if (dev_info->als_dev_index == 0) { // ALS_DEV_ID_0
		first_table = als_get_tcs3410_first_table();
		table_count = als_get_tcs3410_table_count();
	} else { // ALS_DEV_ID_1
		first_table = als1_get_tcs3410_first_table();
		table_count = als1_get_tcs3410_table_count();
	}
	if (first_table == NULL)
		return;
	hwlog_info("%s als_idx %d\n", __func__, dev_info->als_dev_index);
	select_tcs3410_als_data_ex(pf_data, dev_info, first_table, table_count);
}

void select_sip3510_als_data(struct als_platform_data *pf_data,
	struct als_device_info *dev_info)
{
	select_sip3510_als_data_ex(pf_data, dev_info,
		als_get_sip3510_first_table(),
		als_get_sip3510_table_count());
}

void select_hx32062se_als_data(struct als_platform_data *pf_data,
	struct als_device_info *dev_info)
{
	select_hx32062se_als_data_ex(pf_data, dev_info,
		als_get_hx32062se_first_table(),
		als_get_hx32062se_table_count());
}

void select_syn3133_als_data(struct als_platform_data *pf_data,
	struct als_device_info *dev_info)
{
	select_syn3133_als_data_ex(pf_data, dev_info,
		als_get_syn3133_first_table(),
		als_get_syn3133_table_count());
}

void select_syn3136_als_data(struct als_platform_data *pf_data,
	struct als_device_info *dev_info)
{
	select_syn3136_als_data_ex(pf_data, dev_info,
		als_get_syn3136_first_table(),
		als_get_syn3136_table_count());
}

void select_ams_tmd2702_als_data(struct als_platform_data *pf_data,
	struct als_device_info *dev_info)
{
	select_ams_tcs3701_als_data_ex(pf_data, dev_info,
		als_get_tmd2702_first_table(),
		als_get_tmd2702_table_count());
}

void select_ams_tsl2540_als_data(struct als_platform_data *pf_data,
	struct als_device_info *dev_info)
{
	select_ams_tsl540_als_data_ex(pf_data, dev_info,
		als_get_tsl2540_first_table(),
		als_get_tsl2540_table_count());
}

void select_vishay_vcnl36658_als_data(
	struct als_platform_data *pf_data,
	struct als_device_info *dev_info)
{
	u8 phone_color = get_phone_color();
	uint32_t table_size;
	uint32_t i;
	vcnl36658_als_para_table *vcnl36658_tab = NULL;

	vcnl36658_tab = als_get_vcnl36658_first_table();
	table_size = als_get_vcnl36658_table_count();
	if (vcnl36658_tab == NULL)
		return;
	for (i = 0; i < table_size; i++) {
		if ((vcnl36658_tab->phone_type == pf_data->als_phone_type) &&
			(vcnl36658_tab->phone_version == pf_data->als_phone_version) &&
			(vcnl36658_tab->tp_lcd_manufacture ==
			get_tplcd_manufacture() ||
			vcnl36658_tab->tp_lcd_manufacture == DEFAULT_TPLCD) &&
			(vcnl36658_tab->tp_color == phone_color))
			break;

		vcnl36658_tab++;
	}

	vcnl36658_tab -= ((i < table_size) ? 0 : i);
	dev_info->min_thres =
		vcnl36658_tab->vcnl36658_para[VCNL36658_MIN_THRESHOLD_NUM];
	dev_info->max_thres =
		vcnl36658_tab->vcnl36658_para[VCNL36658_MAX_THRESHOLD_NUM];

	dev_info->table_id = ((i < table_size) ? i : 0);

	als_set_extend_data(pf_data, dev_info, vcnl36658_tab->vcnl36658_para,
		sizeof(vcnl36658_tab->vcnl36658_para), (i < table_size));
}

void select_liteon_ltr582_als_data(struct als_platform_data *pf_data,
	struct als_device_info *dev_info)
{
	u8 phone_color = get_phone_color();
	uint32_t table_size;
	uint32_t i;
	ltr582_als_para_table *ltr582_tab = NULL;

	ltr582_tab = als_get_ltr582_first_table();
	table_size = als_get_ltr582_table_count();
	if (ltr582_tab == NULL)
		return;
	for (i = 0; i < table_size; i++) {
		if ((ltr582_tab->phone_type == pf_data->als_phone_type) &&
			(ltr582_tab->phone_version == pf_data->als_phone_version) &&
			(ltr582_tab->tp_lcd_manufacture ==
			get_tplcd_manufacture() ||
			ltr582_tab->tp_lcd_manufacture == DEFAULT_TPLCD) &&
			(ltr582_tab->tp_color == phone_color))
			break;

		ltr582_tab++;
	}

	ltr582_tab -= ((i < table_size) ? 0 : i);
	dev_info->min_thres = ltr582_tab->ltr582_para[LTR582_MIN_THRESHOLD_NUM];
	dev_info->max_thres = ltr582_tab->ltr582_para[LTR582_MAX_THRESHOLD_NUM];

	dev_info->table_id = ((i < table_size) ? i : 0);

	als_set_extend_data(pf_data, dev_info, ltr582_tab->ltr582_para,
		sizeof(ltr582_tab->ltr582_para), (i < table_size));
}

