// SPDX-License-Identifier: GPL-2.0+
/*
 * fm1230.c
 *
 * fm1230 driver
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

#include "fm1230.h"
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of_gpio.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/moduleparam.h>
#include <linux/sched.h>
#include <uapi/linux/sched/types.h>
#include <linux/cdev.h>
#include <linux/delay.h>
#include <linux/compat.h>
#include <linux/uaccess.h>
#include <linux/platform_device.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/ctype.h>
#include <linux/spinlock.h>
#include <linux/time.h>
#include <linux/interrupt.h>
#include <linux/pm_qos.h>
#include <huawei_platform/log/hw_log.h>
#include <chipset_common/hwpower/battery/battery_type_identify.h>
#include <chipset_common/hwpower/common_module/power_common_macro.h>
#include <chipset_common/hwpower/common_module/power_devices_info.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/common_module/power_supply_interface.h>
#include <huawei_platform/power/power_mesg_srv.h>
#include "../batt_aut_checker.h"
#include "fm1230_api.h"
#include "fm1230_swi.h"

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif
#define HWLOG_TAG battct_fm1230
HWLOG_REGIST();

#define FM1230_DFT_IC_INDEX 0
#define FM1230_DFT_FULL_CYC 0

static bool fm1230_check_lock(unsigned char *content)
{
	if (strstr(content, "unlocked"))
		return false;
	return true;
}

static int fm1230_cyclk_set(struct fm1230_dev *di)
{
	int ret;
	unsigned char buf[LOCK_BUF_LEN] = { 0 };

	if (!di)
		return -ENODEV;

	ret = fm1230_operate_file(di, FILE16, LOCK, NULL, 0);
	if (ret) {
		hwlog_err("[%s] lock fail or is locked, ic_index:%u\n", __func__, di->ic_index);
		return -EAGAIN;
	}
	ret = fm1230_operate_file(di, FILE16, LOCK_STATUS, buf, LOCK_BUF_LEN);
	if (ret) {
		hwlog_err("[%s] check batt state fail, ic_index:%u\n", __func__, di->ic_index);
		return -EAGAIN;
	}

	if (!fm1230_check_lock(buf)) {
		hwlog_err("[%s] lock fail, ic_index:%u\n", __func__, di->ic_index);
		return -EINVAL;
	}

	return 0;
}

static enum batt_ic_type fm1230_get_ic_type(void)
{
	return FDW_FM1230_TYPE;
}

static int fm1230_get_uid(struct platform_device *pdev,
	const unsigned char **uuid, unsigned int *uuid_len)
{
	struct fm1230_dev *di = NULL;
	int ret;

	if (!uuid || !uuid_len || !pdev) {
		hwlog_err("[%s] pointer NULL\n", __func__);
		return -EINVAL;
	}

	di = platform_get_drvdata(pdev);
	if (!di)
		return -ENODEV;

	ret = fm1230_read_romid(di, di->mem.uid, MAX_RETRY_COUNT, HALF_SECOND);
	if (ret) {
		hwlog_err("[%s] read uid error\n", __func__);
		return -EINVAL;
	}

	*uuid = di->mem.uid;
	*uuid_len = FM1230_UID_LEN;
	return 0;
}

static int fm1230_get_batt_type(struct platform_device *pdev,
	const unsigned char **type, unsigned int *type_len)
{
	struct fm1230_dev *di = NULL;
	unsigned char type_temp[2] = { 0 };

	if (!pdev || !type || !type_len) {
		hwlog_err("[%s] pointer NULL\n", __func__);
		return -EINVAL;
	}

	di = platform_get_drvdata(pdev);
	if (!di)
		return -ENODEV;

	if (fm1230_operate_file(di, FILE12, READ, type_temp, FM1230_BATTTYP_LEN)) {
		hwlog_err("[%s] read battery type err\n", __func__);
		return -EINVAL;
	}

	di->mem.batt_type[0] = type_temp[1];
	di->mem.batt_type[1] = type_temp[0];
	*type = di->mem.batt_type;
	*type_len = FM1230_BATTTYP_LEN;
	hwlog_info("[%s] Btp0:0x%x; Btp1:0x%x, ic_index:%u\n", __func__, di->mem.batt_type[0],
		di->mem.batt_type[1], di->ic_index);

	return 0;
}

static int fm1230_get_batt_sn(struct platform_device *pdev,
	struct power_genl_attr *res, const unsigned char **sn, unsigned int *sn_size)
{
	struct fm1230_dev *di = NULL;
	int ret;

	if (!pdev || !sn || !sn_size) {
		hwlog_err("[%s] pointer NULL\n", __func__);
		return -EINVAL;
	}
	(void)res;
	di = platform_get_drvdata(pdev);
	if (!di)
		return -EINVAL;
	if (!di->mem.sn_ready) {
		memset(di->mem.sn, 0, FM1230_SN_ASC_LEN);
		ret = fm1230_operate_file(di, FILE13, READ, di->mem.sn, FM1230_SN_ASC_LEN);
		if (ret) {
			hwlog_info("[%s] read sn error\n", __func__);
			return -EINVAL;
		}
		di->mem.sn_ready = true;
	}
	hwlog_info("[%s] cache data is %x %x\n", __func__,
			di->mem.sn[0], di->mem.sn[1]);
	*sn = di->mem.sn;
	*sn_size = FM1230_SN_ASC_LEN;

	return 0;
}

static int fm1230_certification(struct platform_device *pdev,
	struct power_genl_attr *key_res, enum key_cr *result)
{
	int ret;
	struct fm1230_dev *di = NULL;

	if (!pdev || !key_res || !result || !key_res->data || (key_res->len == 0)) {
		hwlog_err("[%s] pointer NULL\n", __func__);
		return -EINVAL;
	}

	di = platform_get_drvdata(pdev);
	if (!di)
		return -ENODEV;

	ret = fm1230_do_authentication(di, key_res, result, ECC_NORMAL);
	if (ret) {
		hwlog_err("[%s] ecce fail, ic_index:%u\n", __func__, di->ic_index);
		*result = KEY_FAIL_UNMATCH;
		return 0;
	}

	*result = KEY_PASS;
	return 0;
}

static int fm1230_ct_read(struct fm1230_dev *di)
{
	return fm1230_read_romid(di, di->mem.uid, MAX_RETRY_COUNT, HALF_SECOND);
}

static void fm1230_crc16_cal(uint8_t *msg, int len, uint16_t *crc)
{
	int i, j;
	uint16_t crc_mul = 0xA001; /* G(x) = x ^ 16 + x ^ 15 + x ^ 2 + 1 */

	*crc = CRC16_INIT_VAL;
	for (i = 0; i < len; i++) {
		*crc ^= *(msg++);
		for (j = 0; j < BIT_P_BYT; j++) {
			if (*crc & ODD_MASK)
				*crc = (*crc >> 1) ^ crc_mul;
			else
				*crc >>= 1;
		}
	}
}

static int fm1230_act_read(struct fm1230_dev *di)
{
	int ret;
	uint16_t crc_act_read;
	uint16_t crc_act_cal;

	if (fm1230_operate_file(di, FILE11, READ, di->mem.act_sign, FM1230_ACT_LEN)) {
		hwlog_info("[%s] act_sig read error, ic_index:%u\n", __func__, di->ic_index);
		return -EAGAIN;
	}

	memcpy((u8 *)&crc_act_read, &di->mem.act_sign[FM1230_ACT_CRC_BYT0], FM1230_ACT_CRC_LEN);
	fm1230_crc16_cal(di->mem.act_sign, (int)(di->mem.act_sign[0] + 1), &crc_act_cal);
	ret = (crc_act_cal != crc_act_read);
	if (ret)
		hwlog_info("[%s] act_sig crc error, ic_index:%u\n", __func__, di->ic_index);

	return ret;
}

#ifndef BATTBD_FORCE_MATCH
static int fm1230_set_act_signature(struct platform_device *pdev,
	enum res_type type, const struct power_genl_attr *res)
{
	(void)pdev;
	(void)type;
	(void)res;
	hwlog_info("[%s] operation banned in user mode\n", __func__);
	return 0;
}
#else

static int fm1230_set_act_signature(struct platform_device *pdev,
	enum res_type type, const struct power_genl_attr *res)
{
	int ret;
	uint16_t crc_act;
	uint8_t act[FM1230_ACT_LEN] = { 0 };
	struct fm1230_dev *di = NULL;
	unsigned char buf[200] = { 0 };

	if (!pdev || !res) {
		hwlog_err("[%s] pointer NULL\n", __func__);
		return -EINVAL;
	}

	di = platform_get_drvdata(pdev);
	if (!di)
		return -ENODEV;

	if (res->len > FM1230_ACT_LEN) {
		hwlog_err("[%s] input act_sig oversize, ic_index:%u\n", __func__, di->ic_index);
		return -EINVAL;
	}
	ret = fm1230_operate_file(di, FILE11, LOCK_STATUS, buf, LOCK_BUF_LEN);
	if (ret) {
		hwlog_err("[%s] check lock state fail, ic_index:%u\n", __func__, di->ic_index);
		goto act_sig_set_succ;
	}
	if (fm1230_check_lock(buf)) {
		hwlog_err("[%s] cert locked, act set abandon, ic_index:%u\n", __func__, di->ic_index);
		goto act_sig_set_succ;
	}

	hwlog_info("[%s] start write act, ic_index:%u\n", __func__, di->ic_index);

	memcpy(act, res->data, res->len);
	fm1230_crc16_cal(act, res->len, &crc_act);
	memcpy(act + sizeof(act) - sizeof(crc_act), (uint8_t *)&crc_act,
		sizeof(crc_act));

	switch (type) {
	case RES_ACT:
		ret = fm1230_operate_file(di, FILE11, WRITE, act, FM1230_ACT_LEN);
		if (ret) {
			hwlog_err("[%s] act write fail, ic_index:%u\n", __func__, di->ic_index);
			goto act_sig_set_fail;
		}
		break;
	default:
		hwlog_err("[%s] invalid option, ic_index:%u\n", __func__, di->ic_index);
		goto act_sig_set_fail;
	}

act_sig_set_succ:
	return 0;
act_sig_set_fail:
	return -EINVAL;
}
#endif /* BATTBD_FORCE_MATCH */

static int fm1230_prepare(struct platform_device *pdev, enum res_type type,
	struct power_genl_attr *res)
{
	int ret;
	struct fm1230_dev *di = NULL;

	if (!pdev || !res) {
		hwlog_err("[%s] pointer NULL\n", __func__);
		return -EINVAL;
	}

	di = platform_get_drvdata(pdev);
	if (!di)
		return -ENODEV;

	switch (type) {
	case RES_CT:
		ret = fm1230_ct_read(di);
		if (ret) {
			hwlog_err("[%s] res_ct read fail, ic_index:%u\n", __func__, di->ic_index);
			goto prepare_fail;
		}
		res->data = (const unsigned char *)di->mem.uid;
		res->len = FM1230_UID_LEN;
		break;
	case RES_ACT:
		ret = fm1230_act_read(di);
		if (ret) {
			hwlog_err("[%s] res_act read fail, ic_index:%u\n", __func__, di->ic_index);
			goto prepare_fail;
		}
		res->data = (const unsigned char *)di->mem.act_sign;
		res->len = FM1230_ACT_LEN;
		break;
	default:
		hwlog_err("[%s] invalid option, ic_index:%u\n", __func__, di->ic_index);
		res->data = NULL;
		res->len = 0;
	}

	return 0;
prepare_fail:
	return -EINVAL;
}

#ifndef BATTBD_FORCE_MATCH
static int fm1230_set_cert_ready(struct fm1230_dev *di)
{
	hwlog_info("[%s] operation banned in user mode, ic_index:%u\n", __func__, di->ic_index);
	return 0;
}
#else

static int fm1230_set_cert_ready(struct fm1230_dev *di)
{
	int ret;
	unsigned char buf[LOCK_BUF_LEN] = { 0 };

	ret = fm1230_operate_file(di, FILE11, LOCK_STATUS, buf, LOCK_BUF_LEN);
	if (ret) {
		hwlog_err("[%s] check cert lock state fail, ic_index:%u\n", __func__, di->ic_index);
		return ret;
	}
	if (fm1230_check_lock(buf)) {
		hwlog_err("[%s] already set cert ready, ic_index:%u\n", __func__, di->ic_index);
		di->mem.cet_rdy = CERT_READY;
		return ret;
	}

	ret = fm1230_operate_file(di, FILE11, LOCK, NULL, 0);
	if (ret) {
		hwlog_err("[%s] lock fail\n", __func__);
		return ret;
	}
	ret = fm1230_operate_file(di, FILE11, LOCK_STATUS, buf, LOCK_BUF_LEN);
	if (ret) {
		hwlog_err("[%s] check cert lock state fail, ic_index:%u\n", __func__, di->ic_index);
		return ret;
	}
	if (!fm1230_check_lock(buf)) {
		hwlog_err("[%s] set_cert_ready fail, ic_index:%u\n", __func__, di->ic_index);
		return -EINVAL;
	} else {
		di->mem.cet_rdy = CERT_READY;
	}
	return ret;
}
#endif /* BATTBD_FORCE_MATCH */

static int fm1230_set_batt_as_org(struct fm1230_dev *di)
{
	int ret;

	ret = fm1230_cyclk_set(di);
	if (ret)
		hwlog_err("[%s] set_batt_as_org fail, ic_index:%u\n", __func__, di->ic_index);
	else
		di->mem.source = BATTERY_ORIGINAL;

	return ret;
}

static void fm1230_u64_to_byte_array(uint64_t data, uint8_t *arr)
{
	int i;

	for (i = 0; i < FM1230_BYTE_COUNT_PER_U64; ++i)
		arr[i] = (data >> (i * FM1230_BIT_COUNT_PER_BYTE));
}

/*
 * Note: arr length must be 8
 */
static void fm1230_byte_array_to_u64(uint64_t *data, uint8_t *arr)
{
	int i;

	*data = 0;
	for (i = 0; i < FM1230_BYTE_COUNT_PER_U64; ++i)
		*data += (uint64_t)arr[i] << (i * FM1230_BIT_COUNT_PER_BYTE);
}

/*
 * Note: arr length must not be smaller than 16
 */
static int fm1230_hex_array_to_u64(uint64_t *data, uint8_t *arr)
{
	uint64_t val;
	int i;

	*data = 0;
	for (i = 0; i < FM1230_HEX_COUNT_PER_U64; ++i) {
		if (arr[i] >= '0' && arr[i] <= '9') { /* number */
			val = arr[i] - '0';
		} else if (arr[i] >= 'a' && arr[i] <= 'f') { /* lowercase */
			val = arr[i] - 'a' + FM1230_HEX_NUMBER_BASE;
		} else if (arr[i] >= 'A' && arr[i] <= 'F') { /* uppercase */
			val = arr[i] - 'A' + FM1230_HEX_NUMBER_BASE;
		} else {
			hwlog_err("[%s] failed int arr[%d]=%d\n",
				__func__, i, arr[i]);
			*data = 0;
			return -EINVAL;
		}
		*data += val << ((FM1230_HEX_COUNT_PER_U64 - 1 - i) *
			FM1230_BIT_COUNT_PER_HEX);
	}
	return 0;
}

static int fm1230_write_group_sn(struct fm1230_dev *di, void *value)
{
	unsigned char buf[LOCK_BUF_LEN] = { 0 };
	uint8_t arr[FM1230_IC_GROUP_SN_LENGTH];
	int ret;

	ret = fm1230_operate_file(di, FILE14, LOCK_STATUS, buf, LOCK_BUF_LEN);
	if (ret) {
		hwlog_err("[%s] check batt state fail, ic_index:%u\n", __func__, di->ic_index);
		return ret;
	}
	if (fm1230_check_lock(buf)) {
		hwlog_err("[%s] group sn already locked, ic_index:%u\n", __func__, di->ic_index);
		return 0;
	}

	fm1230_u64_to_byte_array(*((uint64_t *)value), arr);
	ret = fm1230_operate_file(di, FILE14, WRITE, arr, FM1230_IC_GROUP_SN_LENGTH);
	if (ret) {
		hwlog_err("[%s] write group sn fail, ic_index:%u\n", __func__, di->ic_index);
		return ret;
	}
	ret = fm1230_operate_file(di, FILE14, LOCK, NULL, 0);
	if (ret) {
		hwlog_err("[%s] lock fail or is locked, ic_index:%u\n", __func__, di->ic_index);
		return ret;
	}
	ret = fm1230_operate_file(di, FILE14, LOCK_STATUS, buf, LOCK_BUF_LEN);
	if (ret) {
		hwlog_err("[%s] check batt state fail, ic_index:%u\n", __func__, di->ic_index);
		return ret;
	}
	if (!fm1230_check_lock(buf)) {
		hwlog_err("[%s] lock fail, ic_index:%u\n", __func__, di->ic_index);
		return -EINVAL;
	}

	return 0;
}

static int fm1230_set_batt_org(struct fm1230_dev *di, void *value)
{
	int ret;
	unsigned char buf[LOCK_BUF_LEN] = { 0 };

	ret = fm1230_operate_file(di, FILE16, LOCK_STATUS, buf, LOCK_BUF_LEN);
	if (ret) {
		hwlog_err("[%s] check batt state fail, ic_index:%u\n", __func__, di->ic_index);
		return 0;
	}

	if (fm1230_check_lock(buf)) {
		hwlog_info("[%s] has been org, quit work, ic_index:%u\n", __func__, di->ic_index);
		return 0;
	}

	if (*((enum batt_source *)value) == BATTERY_ORIGINAL) {
		ret = fm1230_set_batt_as_org(di);
		if (ret) {
			hwlog_err("[%s] set_batt_as_org fail, ic_index:%u\n", __func__, di->ic_index);
			return -EINVAL;
		}
	}
	return 0;
}

static int fm1230_set_batt_safe_info(struct platform_device *pdev,
	enum batt_safe_info_t type, void *value)
{
	int ret;
	struct fm1230_dev *di = NULL;

	if (!pdev || !value) {
		hwlog_err("[%s] pointer NULL\n", __func__);
		return -EINVAL;
	}
	di = platform_get_drvdata(pdev);
	if (!di)
		return -EINVAL;

	switch (type) {
	case BATT_CHARGE_CYCLES:
		break;
	case BATT_SPARE_PART:
		ret = fm1230_set_batt_org(di, value);
		if (ret) {
			hwlog_err("[%s] set batt org fail, ic_index:%u\n", __func__, di->ic_index);
			goto battinfo_set_fail;
		}
		break;
	case BATT_CERT_READY:
		ret = fm1230_set_cert_ready(di);
		if (ret) {
			hwlog_err("[%s] set_cert_ready fail, ic_index:%u\n", __func__, di->ic_index);
			goto battinfo_set_fail;
		}
		break;
	case BATT_MATCH_INFO:
		ret = fm1230_write_group_sn(di, value);
		if (ret) {
			hwlog_err("[%s write group sn fail, ic_index:%u\n", __func__, di->ic_index);
			goto battinfo_set_fail;
		}
		break;
	default:
		hwlog_err("[%s] invalid option, ic_index:%u\n", __func__, di->ic_index);
		goto battinfo_set_fail;
	}

	return 0;
battinfo_set_fail:
	return -EINVAL;
}

static int get_batt_spare_part(struct fm1230_dev *di, void *value)
{
	int ret;
	unsigned char buf[LOCK_BUF_LEN] = { 0 };

	ret = fm1230_operate_file(di, FILE16, LOCK_STATUS, buf, LOCK_BUF_LEN);
	if (ret) {
		hwlog_err("[%s] check batt state fail, ic_index:%u\n", __func__, di->ic_index);
		return -EINVAL;
	}

	if (fm1230_check_lock(buf))
		*(enum batt_source *)value = BATTERY_ORIGINAL;
	else
		*(enum batt_source *)value = BATTERY_SPARE_PART;
	return 0;
}

static int get_batt_cert_ready(struct fm1230_dev *di, void *value)
{
	int ret;
	unsigned char buf[LOCK_BUF_LEN] = { 0 };

	ret = fm1230_operate_file(di, FILE11, LOCK_STATUS, buf, LOCK_BUF_LEN);
	if (ret) {
		hwlog_err("[%s] check cert lock state fail, ic_index:%u\n", __func__, di->ic_index);
		return -EINVAL;
	}
	if (!fm1230_check_lock(buf))
		hwlog_err("[%s] cert unready, ic_index:%u\n", __func__, di->ic_index);
	*(enum batt_cert_state *)value = CERT_READY;
	return 0;
}

static int get_batt_match_info(struct fm1230_dev *di, void *value)
{
	int ret;
	uint64_t hash_group_sn = 0;
	uint8_t byte_group_sn[FM1230_IC_GROUP_SN_LENGTH] = { 0 };

	ret = fm1230_operate_file(di, FILE14, READ, byte_group_sn, FM1230_IC_GROUP_SN_LENGTH);
	if (ret) {
		hwlog_err("[%s] read group sn fail, ic_index:%u\n", __func__, di->ic_index);
		return -EINVAL;
	}

	fm1230_byte_array_to_u64(&hash_group_sn, byte_group_sn);

	if (!hash_group_sn) {
		hwlog_err("[%s] hash_group_sn is null, ic_index:%u\n", __func__, di->ic_index);
		return -EINVAL;
	}
	ret = snprintf(di->mem.group_sn, FM1230_SN_ASC_LEN + 1,
		"%016llX", hash_group_sn);
	if (ret < 0) {
		hwlog_err("[%s] snprintf fail\n", __func__);
		return -EINVAL;
	}
	*(uint8_t **)value = di->mem.group_sn;
	return 0;
}

static int fm1230_get_batt_safe_info(struct platform_device *pdev,
	enum batt_safe_info_t type, void *value)
{
	int ret;
	struct fm1230_dev *di = NULL;

	if (!pdev || !value) {
		hwlog_err("[%s] pointer NULL\n", __func__);
		return -EINVAL;
	}
	di = platform_get_drvdata(pdev);
	if (!di)
		return -EINVAL;

	switch (type) {
	case BATT_CHARGE_CYCLES:
		*(int *)value = BATT_INVALID_CYCLES;
		break;
	case BATT_SPARE_PART:
		ret = get_batt_spare_part(di, value);
		if (ret) {
			hwlog_err("[%s] check batt state fail, ic_index:%u\n", __func__, di->ic_index);
			goto battinfo_get_fail;
		}
		break;
	case BATT_CERT_READY:
		ret = get_batt_cert_ready(di, value);
		if (ret) {
			hwlog_err("[%s] check cert lock state fail, ic_index:%u\n", __func__, di->ic_index);
			goto battinfo_get_fail;
		}
		break;
	case BATT_MATCH_INFO:
		ret = get_batt_match_info(di, value);
		if (ret) {
			hwlog_err("[%s] read group sn fail, ic_index:%u\n", __func__, di->ic_index);
			goto battinfo_get_fail;
		}
		break;
	default:
		hwlog_err("[%s] invalid option, ic_index:%u\n", __func__, di->ic_index);
		goto battinfo_get_fail;
	}

	return 0;
battinfo_get_fail:
	return -EINVAL;
}

static void fm1230_set_ic_status(struct platform_device *pdev, int ic_status)
{
	struct fm1230_dev *di = NULL;

	if (!pdev) {
		hwlog_err("[%s] pointer NULL\n", __func__);
		return;
	}
	di = platform_get_drvdata(pdev);
	if (!di) {
		hwlog_err("[%s] di pointer NULL\n", __func__);
		return;
	}
	switch (ic_status) {
	case BAT_TYPE_DEV_IN:
		fm1230_gpio_dir_input(di->onewire_gpio);
		break;
	case BAT_TYPE_DEV_LOW:
		fm1230_gpio_dir_output(di->onewire_gpio, LOW);
		break;
	default:
		hwlog_err("[%s] invalid option, ic_index:%u\n", __func__, di->ic_index);
	}
}

#ifdef BATTBD_FORCE_MATCH
static int fm1230_ecc_ex_check(struct platform_device *pdev,
	struct power_genl_attr *key_res)
{
	struct fm1230_dev *di = NULL;
	enum key_cr result = KEY_UNREADY;

	if (!pdev || !key_res || !key_res->data || (key_res->len == 0)) {
		hwlog_err("[%s] pointer NULL\n", __func__);
		return -EINVAL;
	}

	di = platform_get_drvdata(pdev);
	if (!di) {
		hwlog_err("[%s] di pointer NULL\n", __func__);
		return -ENODEV;
	}

	return fm1230_do_authentication(di, key_res, &result, ECC_EX);
}
#else
static int fm1230_ecc_ex_check(struct platform_device *pdev,
	struct power_genl_attr *key_res)
{
	hwlog_info("user mode prohibit this ops\n", __func__);
	return 0;
}
#endif /* BATTBD_FORCE_MATCH */

static int fm1230_ct_ops_register(struct platform_device *pdev,
	struct batt_ct_ops *bco)
{
	int ret;
	struct fm1230_dev *di = NULL;

	hwlog_info("[%s] start\n", __func__);

	if (!bco || !pdev) {
		hwlog_err("[%s] : bco/pdev NULL\n", __func__);
		return -ENXIO;
	}
	di = platform_get_drvdata(pdev);
	if (!di)
		return -EINVAL;

	ret = fm1230_ic_ck(di);
	if (ret) {
		hwlog_err("[%s] : ic unmatch, ic_index:%u\n", __func__, di->ic_index);
		return -ENXIO;
	}

	hwlog_info("[%s] ic matched, ic_index:%u\n", __func__, di->ic_index);

	bco->get_ic_type = fm1230_get_ic_type;
	bco->get_ic_uuid = fm1230_get_uid;
	bco->get_batt_type = fm1230_get_batt_type;
	bco->get_batt_sn = fm1230_get_batt_sn;
	bco->certification = fm1230_certification;
	bco->set_act_signature = fm1230_set_act_signature;
	bco->prepare = fm1230_prepare;
	bco->set_batt_safe_info = fm1230_set_batt_safe_info;
	bco->get_batt_safe_info = fm1230_get_batt_safe_info;
	bco->set_ic_status = fm1230_set_ic_status;
	bco->rt_check = fm1230_ecc_ex_check;
	bco->power_down = NULL;
	return 0;
}

static int fm1230_get_group_sn(struct platform_device *pdev, uint8_t *group_sn)
{
	int ret;
	struct fm1230_dev *di = platform_get_drvdata(pdev);
	uint64_t hash_group_sn = 0;
	uint8_t byte_group_sn[FM1230_IC_GROUP_SN_LENGTH] = { 0 };

	if (!di)
		return -ENODEV;

	ret = fm1230_operate_file(di, FILE14, READ, byte_group_sn, FM1230_IC_GROUP_SN_LENGTH);
	if (ret) {
		hwlog_err("[%s] read group sn fail, ic_index:%u\n", __func__, di->ic_index);
		return -EINVAL;
	}
	fm1230_byte_array_to_u64(&hash_group_sn, byte_group_sn);
	ret = snprintf(group_sn, FM1230_SN_ASC_LEN + 1,
		"%016llX", hash_group_sn);
	if (ret < 0) {
		hwlog_err("[%s] snprintf fail\n", __func__);
		return -EINVAL;
	}
	return 0;
}

static int fm1230_group_sn_write(struct platform_device *pdev,
	uint8_t *group_sn)
{
	int ret;
	struct fm1230_dev *di = platform_get_drvdata(pdev);
	int8_t buf[FM1230_IC_GROUP_SN_LENGTH];
	uint64_t val = 0;
	int i;

	if (!di)
		return -ENODEV;

	ret = fm1230_hex_array_to_u64(&val, group_sn);
	if (ret) {
		hwlog_err("[%s] hex to u64 fail\n", __func__);
		return -EINVAL;
	}

	hwlog_err("[%s] val = %016llX\n", __func__, val);
	fm1230_u64_to_byte_array(val, buf);
	for (i = 0; i < FM1230_IC_GROUP_SN_LENGTH; ++i)
		hwlog_err("[%s] buf[%d] = %u\n", __func__, i, buf[i]);

	ret = fm1230_operate_file(di, FILE14, WRITE, buf, FM1230_IC_GROUP_SN_LENGTH);
	if (ret) {
		hwlog_err("[%s] write group sn fail, ic_index:%u\n", __func__, di->ic_index);
		return -EINVAL;
	}
	return 0;
}

static ssize_t ic_type_show(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	return snprintf(buf, PAGE_SIZE, "ic type: %d", fm1230_get_ic_type());
}

static ssize_t uid_show(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	struct fm1230_dev *di = NULL;
	int ret;
	uint8_t uid[FM1230_UID_LEN * 2 + 1] = { 0 };

	dev_get_drv_data(di, dev);
	if (!di)
		return snprintf(buf, PAGE_SIZE, "Error data");

	bat_type_apply_mode(BAT_ID_SN);
	ret = fm1230_read_romid(di, di->mem.uid, MAX_RETRY_COUNT, HALF_SECOND);
	bat_type_release_mode(true);
	if (ret) {
		hwlog_err("[%s] read uid error\n", __func__);
		return snprintf(buf, PAGE_SIZE, "read uid error");
	}
	fm_hex_to_str(uid, di->mem.uid, FM1230_UID_LEN);
	return snprintf(buf, PAGE_SIZE, "%s", uid);
}

static ssize_t batt_type_show(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	struct fm1230_dev *di = NULL;
	int ret;
	uint8_t type[FM1230_BATTTYP_LEN] = { 0 };
	uint8_t type_temp[FM1230_BATTTYP_LEN] = { 0 };

	dev_get_drv_data(di, dev);
	if (!di)
		return snprintf(buf, PAGE_SIZE, "Error data");

	bat_type_apply_mode(BAT_ID_SN);
	ret = fm1230_operate_file(di, FILE12, READ, type_temp, FM1230_BATTTYP_LEN);
	bat_type_release_mode(true);
	if (ret) {
		hwlog_err("[%s] read battery type err\n", __func__);
		return snprintf(buf, PAGE_SIZE, "read batt type error");
	}
	type[0] = type_temp[1];
	type[1] = type_temp[0];

	return snprintf(buf, PAGE_SIZE, "[%s] Btp0:0x%x; Btp1:0x%x, ic_index:%u\n",
			__func__, type[FM1230_PKVED_IND], type[FM1230_CELVED_IND], di->ic_index);
}

static ssize_t sn_show(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	struct fm1230_dev *di = NULL;
	int ret;
	uint8_t sn[FM1230_SN_ASC_LEN] = { 0 };

	dev_get_drv_data(di, dev);
	if (!di)
		return snprintf(buf, PAGE_SIZE, "Error data");

	bat_type_apply_mode(BAT_ID_SN);
	ret = fm1230_operate_file(di, FILE13, READ, sn, FM1230_SN_ASC_LEN);
	bat_type_release_mode(true);
	if (ret) {
		hwlog_err("[%s] batt type iqr fail\n", __func__);
		return snprintf(buf, PAGE_SIZE, "read sn error");
	}
	hwlog_info("[%s] SN[ltoh]:%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c, ic_index:%u\n",
			__func__, sn[0], sn[1], sn[2], sn[3], sn[4], sn[5], sn[6], sn[7], sn[8],
			sn[9], sn[10], sn[11], sn[12], sn[13], sn[14], sn[15], di->ic_index);
	return snprintf(buf, PAGE_SIZE,
			"SN [ltoh]: %c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c\n", sn[0],
			sn[1], sn[2], sn[3], sn[4], sn[5], sn[6], sn[7], sn[8],
			sn[9], sn[10], sn[11], sn[12], sn[13], sn[14], sn[15]);
}

static ssize_t group_sn_show(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	struct fm1230_dev *di = NULL;
	int ret;
	uint8_t group_sn[FM1230_SN_ASC_LEN + 1] = { 0 };
	struct platform_device *pdev = NULL;

	pdev = container_of(dev, struct platform_device, dev);
	dev_get_drv_data(di, dev);
	if (!di || !pdev)
		return snprintf(buf, PAGE_SIZE, "Error data");

	bat_type_apply_mode(BAT_ID_SN);
	ret = fm1230_get_group_sn(pdev, group_sn);
	bat_type_release_mode(true);
	if (ret) {
		hwlog_err("[%s] get group sn fail\n", __func__);
		return snprintf(buf, PAGE_SIZE, "get group sn  failed");
	}
	return snprintf(buf, PAGE_SIZE, "group sn: %s", group_sn);
}

static ssize_t group_sn_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int ret;
	struct fm1230_dev *di;
	uint8_t group_sn[FM1230_SN_ASC_LEN] = { 0 };
	struct platform_device *pdev = NULL;

	pdev = container_of(dev, struct platform_device, dev);
	dev_get_drv_data(di, dev);
	if (!di || !pdev || count < FM1230_SN_ASC_LEN)
		return -EINVAL;

	memcpy(group_sn, buf, FM1230_SN_ASC_LEN);
	bat_type_apply_mode(BAT_ID_SN);
	ret = fm1230_group_sn_write(pdev, group_sn);
	bat_type_release_mode(true);
	if (ret)
		hwlog_err("[%s] write group sn fail\n", __func__);

	return count;
}

static ssize_t actsig_show(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	int ret;
	struct fm1230_dev *di = NULL;
	uint8_t act[FM1230_ACT_LEN * 2 + 1];

	dev_get_drv_data(di, dev);
	if (!di)
		return snprintf(buf, PAGE_SIZE, "Error data");

	bat_type_apply_mode(BAT_ID_SN);
	ret = fm1230_act_read(di);
	bat_type_release_mode(true);
	if (ret) {
		hwlog_err("[%s] res_act read fail, ic_index:%u\n", __func__, di->ic_index);
		return snprintf(buf, PAGE_SIZE, "get actsig failed");
	}
	fm_hex_to_str(act, di->mem.act_sign, FM1230_ACT_LEN);
	return scnprintf(buf, PAGE_SIZE, "Act = %s\n", act);
}

static ssize_t actsig_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int ret;
	uint8_t act_data[FM1230_ACT_LEN];
	struct power_genl_attr res;
	enum res_type type = RES_ACT;
	struct fm1230_dev *di;
	struct platform_device *pdev = NULL;

	pdev = container_of(dev, struct platform_device, dev);
	dev_get_drv_data(di, dev);
	if (!di || !pdev || count > FM1230_ACT_LEN)
		return -EINVAL;

	memcpy(act_data, buf, count);
	res.data = act_data;
	res.len = count;
	bat_type_apply_mode(BAT_ID_SN);
	ret = fm1230_set_act_signature(pdev, type, &res);
	bat_type_release_mode(true);
	if (ret)
		hwlog_err("[%s] act set fail\n", __func__);

	return count;
}

static ssize_t battcyc_show(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	int ret;
	enum batt_safe_info_t type = BATT_CHARGE_CYCLES;
	int batt_cyc;
	struct fm1230_dev *di;
	struct platform_device *pdev = NULL;

	pdev = container_of(dev, struct platform_device, dev);
	dev_get_drv_data(di, dev);
	if (!di || !pdev)
		return -EINVAL;

	ret = fm1230_get_batt_safe_info(pdev, type, (void *)&batt_cyc);
	if (ret) {
		hwlog_err("[%s] batt_cyc get fail\n", __func__);
		return snprintf(buf, PAGE_SIZE, "get battcyc failed");
	}

	return snprintf(buf, PAGE_SIZE, "battcyc: %d", batt_cyc);
}

static ssize_t battcyc_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int ret;
	enum batt_safe_info_t type = BATT_CHARGE_CYCLES;
	long cyc_dcr;
	struct fm1230_dev *di;
	struct platform_device *pdev = NULL;

	pdev = container_of(dev, struct platform_device, dev);
	dev_get_drv_data(di, dev);
	if (!di || !pdev)
		return -EINVAL;

	if (kstrtol(buf, POWER_BASE_DEC, &cyc_dcr) < 0 ||
			cyc_dcr < 0 || cyc_dcr > CYC_MAX) {
		hwlog_err("[%s] : val is not valid!, ic_index:%u\n",
				__func__, di->ic_index);
		return count;
	}
	ret = fm1230_set_batt_safe_info(pdev, type, (void *)&cyc_dcr);
	if (ret)
		hwlog_err("[%s] batt_cyc dcr %d fail\n", __func__, cyc_dcr);

	return count;
}

static ssize_t org_spar_show(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	int ret;
	enum batt_safe_info_t type = BATT_SPARE_PART;
	enum batt_source batt_spar;
	struct fm1230_dev *di;
	struct platform_device *pdev = NULL;

	pdev = container_of(dev, struct platform_device, dev);
	dev_get_drv_data(di, dev);
	if (!di || !pdev)
		return -EINVAL;

	bat_type_apply_mode(BAT_ID_SN);
	ret = fm1230_get_batt_safe_info(pdev, type, (void *)&batt_spar);
	bat_type_release_mode(true);
	if (ret) {
		hwlog_err("[%s] batt spar check %d fail\n", __func__, ret);
		return snprintf(buf, PAGE_SIZE, "batt spar check failed");
	}
	return snprintf(buf, PAGE_SIZE, "batt: %d", batt_spar);
}

static ssize_t org_spar_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int ret;
	long val;
	enum batt_safe_info_t type = BATT_SPARE_PART;
	struct fm1230_dev *di;
	struct platform_device *pdev = NULL;

	pdev = container_of(dev, struct platform_device, dev);
	dev_get_drv_data(di, dev);
	if (!di || !pdev)
		return -EINVAL;

	if (kstrtol(buf, POWER_BASE_DEC, &val) < 0) {
		hwlog_err("[%s] : val is not valid!, ic_index:%u\n",
			__func__, di->ic_index);
		return count;
	}
	bat_type_apply_mode(BAT_ID_SN);
	ret = fm1230_set_batt_safe_info(pdev, type, (void *)&val);
	bat_type_release_mode(true);
	if (ret)
		hwlog_err("[%s] org set %d fail\n", __func__, ret);
	return count;
}

static ssize_t cert_status_show(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	int ret;
	struct fm1230_dev *di;
	uint8_t cert_status_buf[LOCK_BUF_LEN] = { 0 };

	dev_get_drv_data(di, dev);
	if (!di)
		return -EINVAL;

	bat_type_apply_mode(BAT_ID_SN);
	ret = fm1230_operate_file(di, FILE11, LOCK_STATUS, cert_status_buf, LOCK_BUF_LEN);
	bat_type_release_mode(true);
	if (ret) {
		hwlog_err("[%s] check lock state fail, ic_index:%u\n", __func__, di->ic_index);
		return snprintf(buf, PAGE_SIZE, "check lock state fail");
	}
	return snprintf(buf, PAGE_SIZE, "cert status: %s", cert_status_buf);
}

static ssize_t cert_status_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int ret;
	long val = 0;
	struct fm1230_dev *di;

	enum batt_safe_info_t type = BATT_CERT_READY;
	struct platform_device *pdev = NULL;

	pdev = container_of(dev, struct platform_device, dev);
	dev_get_drv_data(di, dev);
	if (!di || !pdev)
		return -EINVAL;

	if (kstrtol(buf, POWER_BASE_DEC, &val) < 0 || val != 1) {
			hwlog_err("[%s] : val is not valid!, ic_index:%u\n",
			__func__, di->ic_index);
			return count;
	}
	bat_type_apply_mode(BAT_ID_SN);
	ret = fm1230_set_batt_safe_info(pdev, type, (void *)&val);
	bat_type_release_mode(true);
	if (ret)
		hwlog_err("[%s] lock cert fail, ic_index:%u\n", __func__, di->ic_index);

	return count;
}

static DEVICE_ATTR_RO(ic_type);
static DEVICE_ATTR_RO(uid);
static DEVICE_ATTR_RO(batt_type);
static DEVICE_ATTR_RO(sn);
#ifdef ONEWIRE_STABILITY_DEBUG
static DEVICE_ATTR_RW(actsig);
static DEVICE_ATTR_RW(battcyc);
static DEVICE_ATTR_RW(org_spar);
static DEVICE_ATTR_RW(cert_status);
static DEVICE_ATTR_RW(group_sn);
#endif /* BATTERY_LIMIT_DEBUG */

static const struct attribute *batt_checker_attrs[] = {
	&dev_attr_ic_type.attr,
	&dev_attr_uid.attr,
	&dev_attr_batt_type.attr,
	&dev_attr_sn.attr,
#ifdef ONEWIRE_STABILITY_DEBUG
	&dev_attr_actsig.attr,
	&dev_attr_battcyc.attr,
	&dev_attr_org_spar.attr,
	&dev_attr_cert_status.attr,
	&dev_attr_group_sn.attr,
#endif /* BATTERY_LIMIT_DEBUG */
	NULL, /* sysfs_create_files need last one be NULL */
};

static int fm1230_sysfs_create(struct platform_device *pdev)
{
	if (sysfs_create_files(&pdev->dev.kobj, batt_checker_attrs)) {
		hwlog_err("[%s] Can't create all expected nodes under %s\n",
			  __func__, pdev->dev.kobj.name);
		return BATTERY_DRIVER_FAIL;
	}

	return BATTERY_DRIVER_SUCCESS;
}

static void fm1230_parse_protocol(struct fm1230_dev *di, struct device_node *np)
{
	if (power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"ow_reset_start_delay", &di->fm1230_swi.ow_reset_start_delay, FM1230_DEFAULT_RESET_START_DELAY))
		hwlog_err("[%s] : ow_reset_start_delay not given in dts\n", __func__);

	if (power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"ow_reset_sample_delay", &di->fm1230_swi.ow_reset_sample_delay, FM1230_DEFAULT_RESET_SAMPLE_DELAY))
		hwlog_err("[%s] : ow_reset_sample_delay not given in dts\n", __func__);

	if (power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"ow_reset_end_delay", &di->fm1230_swi.ow_reset_end_delay, FM1230_DEFAULT_RESET_END_DELAY))
		hwlog_err("[%s] : ow_reset_end_delay not given in dts\n", __func__);

	if (power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"ow_write_start_delay", &di->fm1230_swi.ow_write_start_delay, FM1230_DEFAULT_WRITE_START_DELAY))
		hwlog_err("[%s] : oow_write_start_delay not given in dts\n", __func__);

	if (power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"ow_write_low_delay", &di->fm1230_swi.ow_write_low_delay, FM1230_DEFAULT_WRITE_LOW_DELAY))
		hwlog_err("[%s] : ow_write_low_delay not given in dts\n", __func__);

	if (power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"ow_write_high_delay", &di->fm1230_swi.ow_write_high_delay, FM1230_DEFAULT_WRITE_HIGH_DELAY))
		hwlog_err("[%s] : ow_write_high_delay not given in dts\n", __func__);

	if (power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"ow_write_end_delay", &di->fm1230_swi.ow_write_end_delay, FM1230_DEFAULT_WRITE_END_DELAY))
		hwlog_err("[%s] : ow_write_end_delay not given in dts\n", __func__);

	if (power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"ow_read_start_delay", &di->fm1230_swi.ow_read_start_delay, FM1230_DEFAULT_READ_START_DELAY))
		hwlog_err("[%s] : ow_read_start_delay not given in dts\n", __func__);

	if (power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"ow_read_sample_delay", &di->fm1230_swi.ow_read_sample_delay, FM1230_DEFAULT_READ_SAMPLE_DELAY))
		hwlog_err("[%s] : ow_read_sample_delay not given in dts\n", __func__);

	if (power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"ow_read_end_delay", &di->fm1230_swi.ow_read_end_delay, FM1230_DEFAULT_READ_END_DELAY))
		hwlog_err("[%s] : ow_read_end_delay not given in dts\n", __func__);
}

static int fm1230_parse_dts(struct fm1230_dev *di, struct device_node *np)
{
	int ret;

	ret = of_property_read_u32(np, "ic_index", &di->ic_index);
	if (ret) {
		hwlog_err("[%s] : ic_index not given in dts\n", __func__);
		di->ic_index = FM1230_DFT_IC_INDEX;
	}
	hwlog_info("[%s] ic_index = 0x%x\n", __func__, di->ic_index);

	ret = of_property_read_u16(np, "cyc_full", &di->cyc_full);
	if (ret) {
		hwlog_err("[%s] : cyc_full not given in dts, ic_index:%u\n",
			__func__, di->ic_index);
		di->cyc_full = FM1230_DFT_FULL_CYC;
	}
	hwlog_info("[%s] cyc_full = 0x%x, ic_index:%u\n", __func__,
		di->cyc_full, di->ic_index);

	fm1230_parse_protocol(di, np);
	return 0;
}

static int fm1230_gpio_init(struct platform_device *pdev,
	struct fm1230_dev *di)
{
	int ret;

	di->onewire_gpio = of_get_named_gpio(pdev->dev.of_node,
		"onewire-gpio", 0);

	if (!gpio_is_valid(di->onewire_gpio)) {
		hwlog_err("[%s] onewire_gpio is not valid, ic_index:%u\n", __func__, di->ic_index);
		return -EINVAL;
	}
	/* get the gpio */
	ret = devm_gpio_request(&pdev->dev, di->onewire_gpio, "onewire_fm1230");
	if (ret) {
		hwlog_err("[%s] gpio request failed %d, ic_index:%u\n",
			__func__, di->onewire_gpio, di->ic_index);
		return ret;
	}
	gpio_direction_input(di->onewire_gpio);

	return 0;
}

static int fm1230_ic_check(struct fm1230_dev *di)
{
	int retry;
	int ret;

	bat_type_apply_mode(BAT_ID_SN);
	for (retry = 0; retry < MAX_RETRY_COUNT; retry++) {
		ret = fm1230_ic_ck(di);
		if (!ret)
			break;
	}
	bat_type_release_mode(true);
	return ret;
}

static int fm1230_probe(struct platform_device *pdev)
{
	int ret;
	struct fm1230_dev *di = NULL;
	struct device_node *np = NULL;
	struct power_devices_info_data *power_dev_info = NULL;

	hwlog_info("[%s] probe begin\n", __func__);

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	di = kzalloc(sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	di->dev = &pdev->dev;
	np = pdev->dev.of_node;

	ret = fm1230_parse_dts(di, np);
	if (ret) {
		hwlog_err("[%s] dts parse fail, ic_index:%u\n",
			__func__, di->ic_index);
		goto probe_fail;
	}

	ret = fm1230_gpio_init(pdev, di);
	if (ret) {
		hwlog_err("[%s] gpio init fail, ic_index:%u\n",
			__func__, di->ic_index);
		goto probe_fail;
	}

	ret = fm1230_ic_check(di);
	if (ret) {
		hwlog_err("[%s] ic check fail, ic_index:%u\n",
			__func__, di->ic_index);
		goto ic_ck_fail;
	}
	di->reg_node.ic_memory_release = NULL;
	di->reg_node.ic_dev = pdev;
	di->reg_node.ct_ops_register = fm1230_ct_ops_register;
	add_to_aut_ic_list(&di->reg_node);

	ret = fm1230_sysfs_create(pdev);
	if (ret)
		hwlog_err("[%s] : sysfs create fail, ic_index:%u\n",
			__func__, di->ic_index);

	power_dev_info = power_devices_info_register();
	if (power_dev_info) {
		power_dev_info->dev_name = pdev->dev.driver->name;
		power_dev_info->dev_id = 0;
		power_dev_info->ver_id = 0;
	}

	platform_set_drvdata(pdev, di);

	return 0;

ic_ck_fail:
	devm_gpio_free(&pdev->dev, di->onewire_gpio);
probe_fail:
	kfree(di);
	return ret;
}

static int fm1230_remove(struct platform_device *pdev)
{
	return 0;
}

static const struct of_device_id ow_dt_match[] = {
	{
		.compatible = "fdw,fm1230",
		.data = NULL,
	},
	{},
};

static struct platform_driver fm1230_driver = {
	.probe			= fm1230_probe,
	.remove			= fm1230_remove,
	.driver			= {
		.owner		= THIS_MODULE,
		.name		= "fm1230",
		.of_match_table = ow_dt_match,
	},
};

static int __init fm1230_init(void)
{
	return platform_driver_register(&fm1230_driver);
}

static void __exit fm1230_exit(void)
{
	platform_driver_unregister(&fm1230_driver);
}

subsys_initcall_sync(fm1230_init);
module_exit(fm1230_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("fm1230 ic");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
