// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2024. All rights reserved.
 *
 * t91407.c
 *
 * t91407 driver
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

#include "t91407.h"
#include <securec.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of_gpio.h>
#include <linux/fs.h>
#include <linux/moduleparam.h>
#include <linux/cdev.h>
#include <linux/platform_device.h>
#include <linux/sysfs.h>
#include <linux/spinlock.h>
#include <linux/time.h>
#include <huawei_platform/log/hw_log.h>
#include <chipset_common/hwpower/battery/battery_type_identify.h>
#include <chipset_common/hwpower/common_module/power_common_macro.h>
#include <chipset_common/hwpower/common_module/power_devices_info.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/common_module/power_supply_interface.h>
#include <huawei_platform/power/power_mesg_srv.h>
#include "t91407_api.h"
#include "t91407_swi.h"

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif
#define HWLOG_TAG battct_t91407
HWLOG_REGIST();

#define T91407_DFT_IC_INDEX 0

static bool t91407_check_lock(unsigned char *content)
{
	if (strstr(content, "unlocked"))
		return false;
	hwlog_info("[%s] success: locked\n", __func__);
	return true;
}

static enum batt_ic_type t91407_get_ic_type(void)
{
	return TMC_T91407_TYPE;
}

static int t91407_get_uid(struct platform_device *pdev,
	const unsigned char **uuid, unsigned int *uuid_len)
{
	struct t91407_dev *di = NULL;
	int ret;

	if (!uuid || !uuid_len || !pdev) {
		hwlog_err("[%s] pointer NULL\n", __func__);
		return -EINVAL;
	}

	di = platform_get_drvdata(pdev);
	if (!di)
		return -ENODEV;

	ret = t91407_read_romid(di, di->mem.uid);
	if (ret) {
		hwlog_err("[%s] read uid error\n", __func__);
		return -EINVAL;
	}

	*uuid = di->mem.uid;
	*uuid_len = T91407_UID_LEN;
	return 0;
}

static int t91407_get_batt_type(struct platform_device *pdev,
	const unsigned char **type, unsigned int *type_len)
{
	struct t91407_dev *di = NULL;
	unsigned char type_temp[T91407_BATTTYP_LEN] = { 0 };

	if (!pdev || !type || !type_len) {
		hwlog_err("[%s] pointer NULL\n", __func__);
		return -EINVAL;
	}

	di = platform_get_drvdata(pdev);
	if (!di)
		return -ENODEV;
	if (!di->mem.batt_type_ready) {
		if (t91407_operate_file(di, T91407_FILE_BATTTYPE, T91407_READ, type_temp, T91407_BATTTYP_LEN)) {
			hwlog_err("[%s] read battery type err\n", __func__);
			return -EINVAL;
		}
		di->mem.batt_type[0] = type_temp[1];
		di->mem.batt_type[1] = type_temp[0];
		di->mem.batt_type_ready = true;
	}
	*type = di->mem.batt_type;
	*type_len = T91407_BATTTYP_LEN;
	hwlog_info("[%s] Btp0:0x%x; Btp1:0x%x, ic_index:%u\n", __func__, di->mem.batt_type[0],
		di->mem.batt_type[1], di->ic_index);
	return 0;
}

static int t91407_get_batt_sn(struct platform_device *pdev,
	struct power_genl_attr *res, const unsigned char **sn, unsigned int *sn_size)
{
	struct t91407_dev *di = NULL;
	int ret;
	char write[T91407_DATA_READ_LEN] = { 0 };

	if (!pdev || !sn || !sn_size) {
		hwlog_err("[%s] pointer NULL\n", __func__);
		return -EINVAL;
	}
	(void)res;
	di = platform_get_drvdata(pdev);
	if (!di)
		return -EINVAL;
	if (!di->mem.sn_ready) {
		memset_s(di->mem.sn, T91407_SN_ASC_LEN, 0, T91407_SN_ASC_LEN);
		ret = t91407_operate_file(di, T91407_FILE_SN, T91407_READ, di->mem.sn, T91407_SN_ASC_LEN);
		if (ret) {
			hwlog_info("[%s] read sn error\n", __func__);
			return -EINVAL;
		}
		di->mem.sn_ready = true;
	}
	t91407_print_data(di->mem.sn, T91407_SN_ASC_LEN, write, T91407_DATA_READ_LEN);
	hwlog_info("[%s] cache data is %s\n", __func__, write);
	*sn = di->mem.sn;
	*sn_size = T91407_SN_ASC_LEN;
	return 0;
}

static int t91407_certification(struct platform_device *pdev,
	struct power_genl_attr *key_res, enum key_cr *result)
{
	int ret;
	struct t91407_dev *di = NULL;

	if (!pdev || !key_res || !result || !key_res->data) {
		hwlog_err("[%s] pointer NULL\n", __func__);
		return -EINVAL;
	}
	if (!key_res->len) {
		hwlog_err("[%s] key_res length NULL\n", __func__);
		return -EINVAL;
	}
	di = platform_get_drvdata(pdev);
	if (!di)
		return -ENODEV;

	ret = t91407_do_authentication(di, key_res);
	if (ret) {
		hwlog_err("[%s] ecce fail, ic_index:%u\n", __func__, di->ic_index);
		*result = KEY_FAIL_UNMATCH;
		return 0;
	}

	*result = KEY_PASS;
	return 0;
}

static int t91407_ct_read(struct t91407_dev *di)
{
	return t91407_read_romid(di, di->mem.uid);
}

static void t91407_crc16_cal(uint8_t *msg, int len, uint16_t *crc)
{
	int i, j;
	uint16_t crc_mul = 0xA001; /* G(x) = x ^ 16 + x ^ 15 + x ^ 2 + 1 */

	*crc = T91407_CRC16_INIT_VAL;
	for (i = 0; i < len; i++) {
		*crc ^= *(msg++);
		for (j = 0; j < T91407_BIT_P_BYT; j++) {
			if (*crc & T91407_ODD_MASK)
				*crc = ((*crc >> 1) ^ crc_mul);
			else
				*crc >>= 1;
		}
	}
}

static int t91407_act_read(struct t91407_dev *di)
{
	int ret = 0;
	uint16_t crc_act_read = 0;
	uint16_t crc_act_cal = 0;

	if (di->mem.act_sign_ready)
		return ret;

	if (t91407_operate_file(di, T91407_FILE_ACT, T91407_READ, di->mem.act_sign, T91407_ACT_LEN)) {
		hwlog_info("[%s] act_sig read error, ic_index:%u\n", __func__, di->ic_index);
		return -EAGAIN;
	}
	ret = memcpy_s((u8 *)&crc_act_read, T91407_ACT_CRC_LEN, &di->mem.act_sign[T91407_ACT_CRC_BYT0],
		T91407_ACT_CRC_LEN);
	if (ret != EOK) {
		hwlog_err("[%s] memcpy_s failed\n", __func__);
		return T91407_FAILED;
	}
	t91407_crc16_cal(di->mem.act_sign, (int)(di->mem.act_sign[0] + 1), &crc_act_cal);
	ret = (crc_act_cal != crc_act_read);
	if (!ret)
		di->mem.act_sign_ready = true;
	hwlog_info("[%s] act_sig crc ret = %d!\n", __func__, ret);
	return ret;
}

#ifndef BATTBD_FORCE_MATCH
static int t91407_set_act_signature(struct platform_device *pdev,
	enum res_type type, const struct power_genl_attr *res)
{
	(void)pdev;
	(void)type;
	(void)res;
	hwlog_info("[%s] operation banned in user mode\n", __func__);
	return 0;
}
#else
static int t91407_set_act_signature(struct platform_device *pdev,
	enum res_type type, const struct power_genl_attr *res)
{
	int ret;
	uint16_t crc_act;
	uint8_t act[T91407_ACT_LEN] = { 0 };
	struct t91407_dev *di = NULL;
	unsigned char buf[T91407_LOCK_BUF_LEN] = { 0 };

	if (!pdev || !res) {
		hwlog_err("[%s] pointer NULL\n", __func__);
		return -EINVAL;
	}

	di = platform_get_drvdata(pdev);
	if (!di)
		return -ENODEV;

	if (res->len > T91407_ACT_LEN) {
		hwlog_err("[%s] input act_sig oversize, ic_index:%u\n", __func__, di->ic_index);
		return -EINVAL;
	}

	ret = t91407_operate_file(di, T91407_FILE_ACT, T91407_LOCK_STATUS, buf, T91407_LOCK_BUF_LEN);
	if (ret)
		goto act_sig_set_fail;
	if (t91407_check_lock(buf)) {
		hwlog_err("[%s] cert locked, act set abandon, ic_index:%u\n", __func__, di->ic_index);
		goto act_sig_set_succ;
	}

	if ((memcpy_s(act, T91407_ACT_LEN, res->data, res->len)) != EOK)
		hwlog_err("[%s] memcpy_s failed\n", __func__);
	t91407_crc16_cal(act, res->len, &crc_act);
	if ((memcpy_s((act + sizeof(act) - sizeof(crc_act)), sizeof(crc_act),
		(uint8_t *)&crc_act, sizeof(crc_act))) != EOK)
		hwlog_err("[%s] memcpy_s failed\n", __func__);
	switch (type) {
	case RES_ACT:
		ret = t91407_operate_file(di, T91407_FILE_ACT, T91407_WRITE, act, T91407_ACT_LEN);
		if (ret)
			goto act_sig_set_fail;
		ret = t91407_act_read(di);
		if (ret)
			goto act_sig_set_fail;
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

static int t91407_prepare(struct platform_device *pdev, enum res_type type,
	struct power_genl_attr *res)
{
	int ret;
	struct t91407_dev *di = NULL;

	hwlog_info("[%s] t91407_prepare begin, type:%d\n", __func__, type);

	if (!pdev || !res) {
		hwlog_err("[%s] pointer NULL\n", __func__);
		return -EINVAL;
	}

	di = platform_get_drvdata(pdev);
	if (!di)
		return -ENODEV;

	switch (type) {
	case RES_CT:
		ret = t91407_ct_read(di);
		if (ret) {
			hwlog_err("[%s] res_ct read fail, ic_index:%u\n", __func__, di->ic_index);
			goto prepare_fail;
		}
		res->data = (const unsigned char *)di->mem.uid;
		res->len = T91407_UID_LEN;
		break;
	case RES_ACT:
		ret = t91407_act_read(di);
		if (ret) {
			hwlog_err("[%s] res_act read fail, ic_index:%u\n", __func__, di->ic_index);
			goto prepare_fail;
		}
		res->data = (const unsigned char *)di->mem.act_sign;
		res->len = T91407_ACT_LEN;
		break;
	default:
		hwlog_err("[%s] invalid option, ic_index:%u\n", __func__, di->ic_index);
		res->data = NULL;
		res->len = 0;
	}

	hwlog_info("[%s] success!\n", __func__);
	return 0;
prepare_fail:
	return -EINVAL;
}

static int t91407_cyclk_set(struct t91407_dev *di)
{
	int ret;
	unsigned char buf[T91407_LOCK_BUF_LEN] = { 0 };

	if (!di)
		return -ENODEV;

	ret = t91407_operate_file(di, T91407_FILE_LOCK, T91407_LOCK, NULL, 0);
	if (ret) {
		hwlog_err("[%s] lock fail or is locked, ic_index:%u\n", __func__, di->ic_index);
		return -EAGAIN;
	}
	ret = t91407_operate_file(di, T91407_FILE_LOCK, T91407_LOCK_STATUS, buf, T91407_LOCK_BUF_LEN);
	if (ret) {
		hwlog_err("[%s] check batt state fail, ic_index:%u\n", __func__, di->ic_index);
		return -EAGAIN;
	}

	if (!t91407_check_lock(buf)) {
		hwlog_err("[%s] lock fail, ic_index:%u\n", __func__, di->ic_index);
		return -EINVAL;
	}
	return 0;
}

static int t91407_set_batt_as_org(struct t91407_dev *di)
{
	int ret;

	ret = t91407_cyclk_set(di);
	if (ret)
		hwlog_err("[%s] set_batt_as_org fail, ic_index:%u\n", __func__, di->ic_index);
	else
		di->mem.source = BATTERY_ORIGINAL;
	return ret;
}

static int t91407_set_batt_org(struct t91407_dev *di, void *value)
{
	int ret;
	unsigned char buf[T91407_LOCK_BUF_LEN] = { 0 };

	ret = t91407_operate_file(di, T91407_FILE_LOCK, T91407_LOCK_STATUS, buf, T91407_LOCK_BUF_LEN);
	if (ret) {
		hwlog_err("[%s] check batt state fail, ic_index:%u\n", __func__, di->ic_index);
		return 0;
	}

	if (t91407_check_lock(buf)) {
		hwlog_info("[%s] has been org, quit work, ic_index:%u\n", __func__, di->ic_index);
		return 0;
	}

	if (*((enum batt_source *)value) == BATTERY_ORIGINAL) {
		ret = t91407_set_batt_as_org(di);
		if (ret) {
			hwlog_err("[%s] set_batt_as_org fail, ic_index:%u\n", __func__, di->ic_index);
			return -EINVAL;
		}
	}
	return 0;
}

#ifndef BATTBD_FORCE_MATCH
static int t91407_set_cert_ready(struct t91407_dev *di)
{
	hwlog_info("[%s] operation banned in user mode, ic_index:%u\n", __func__, di->ic_index);
	return 0;
}
#else
static int t91407_set_cert_ready(struct t91407_dev *di)
{
	int ret;
	unsigned char buf[T91407_LOCK_BUF_LEN] = { 0 };

	ret = t91407_operate_file(di, T91407_FILE_ACT, T91407_LOCK_STATUS, buf, T91407_LOCK_BUF_LEN);
	if (ret) {
		hwlog_err("[%s] check cert lock state fail, ic_index:%u\n", __func__, di->ic_index);
		return ret;
	}
	if (t91407_check_lock(buf)) {
		hwlog_err("[%s] already set cert ready, ic_index:%u\n", __func__, di->ic_index);
		di->mem.cet_rdy = CERT_READY;
		return ret;
	}

	ret = t91407_operate_file(di, T91407_FILE_ACT, T91407_LOCK, NULL, 0);
	if (ret) {
		hwlog_err("[%s] lock fail\n", __func__);
		return ret;
	}
	ret = t91407_operate_file(di, T91407_FILE_ACT, T91407_LOCK_STATUS, buf, T91407_LOCK_BUF_LEN);
	if (ret) {
		hwlog_err("[%s] check cert lock state fail, ic_index:%u\n", __func__, di->ic_index);
		return ret;
	}
	if (!t91407_check_lock(buf)) {
		hwlog_err("[%s] set_cert_ready fail, ic_index:%u\n", __func__, di->ic_index);
		return -EINVAL;
	} else {
		di->mem.cet_rdy = CERT_READY;
	}
	return ret;
}
#endif /* BATTBD_FORCE_MATCH */

static void t91407_u64_to_byte_array(uint64_t data, uint8_t *arr)
{
	int i;

	for (i = 0; i < T91407_BYTE_COUNT_PER_U64; ++i)
		arr[i] = (data >> (i * T91407_BIT_COUNT_PER_BYTE));
}

/*
 * Note: arr length must be 8
 */
static void t91407_byte_array_to_u64(uint64_t *data, uint8_t *arr)
{
	int i;

	*data = 0;
	for (i = 0; i < T91407_BYTE_COUNT_PER_U64; ++i)
		*data += (uint64_t)arr[i] << (i * T91407_BIT_COUNT_PER_BYTE);
}

/*
 * Note: arr length must not be smaller than 16
 */
static int t91407_hex_array_to_u64(uint64_t *data, uint8_t *arr)
{
	uint64_t val;
	int i;

	*data = 0;
	for (i = 0; i < T91407_HEX_COUNT_PER_U64; ++i) {
		if ((arr[i] >= '0') && (arr[i] <= '9')) { /* number */
			val = arr[i] - '0';
		} else if ((arr[i] >= 'a') && (arr[i] <= 'f')) { /* lowercase */
			val = arr[i] - 'a' + T91407_HEX_NUMBER_BASE;
		} else if ((arr[i] >= 'A') && (arr[i] <= 'F')) { /* uppercase */
			val = arr[i] - 'A' + T91407_HEX_NUMBER_BASE;
		} else {
			hwlog_err("[%s] failed int arr[%d]=%d\n",
				__func__, i, arr[i]);
			*data = 0;
			return -EINVAL;
		}
		*data += (val << ((T91407_HEX_COUNT_PER_U64 - 1 - i) *
			T91407_BIT_COUNT_PER_HEX));
	}
	return 0;
}

static int t91407_write_group_sn(struct t91407_dev *di, void *value)
{
	unsigned char buf[T91407_LOCK_BUF_LEN] = { 0 };
	uint8_t arr[T91407_IC_GROUP_SN_LENGTH];
	int ret;

	ret = t91407_operate_file(di, T91407_FILE_GSN, T91407_LOCK_STATUS, buf, T91407_LOCK_BUF_LEN);
	if (ret) {
		hwlog_err("[%s] check batt state fail, ic_index:%u\n", __func__, di->ic_index);
		return ret;
	}
	if (t91407_check_lock(buf)) {
		hwlog_err("[%s] group sn already locked, ic_index:%u\n", __func__, di->ic_index);
		return 0;
	}

	t91407_u64_to_byte_array(*((uint64_t *)value), arr);
	ret = t91407_operate_file(di, T91407_FILE_GSN, T91407_WRITE, arr, T91407_IC_GROUP_SN_LENGTH);
	if (ret) {
		hwlog_err("[%s] write group sn fail, ic_index:%u\n", __func__, di->ic_index);
		return ret;
	}
	ret = t91407_operate_file(di, T91407_FILE_GSN, T91407_LOCK, NULL, 0);
	if (ret) {
		hwlog_err("[%s] lock fail or is locked, ic_index:%u\n", __func__, di->ic_index);
		return ret;
	}
	ret = t91407_operate_file(di, T91407_FILE_GSN, T91407_LOCK_STATUS, buf, T91407_LOCK_BUF_LEN);
	if (ret) {
		hwlog_err("[%s] check batt state fail, ic_index:%u\n", __func__, di->ic_index);
		return ret;
	}
	if (!t91407_check_lock(buf)) {
		hwlog_err("[%s] lock fail, ic_index:%u\n", __func__, di->ic_index);
		return -EINVAL;
	}

	di->mem.group_sn_ready = false;
	return 0;
}

static int t91407_set_batt_safe_info(struct platform_device *pdev,
	enum batt_safe_info_t type, void *value)
{
	int ret;
	struct t91407_dev *di = NULL;

	hwlog_info("[%s] t91407_set_batt_safe_info type:%d\n", __func__, type);
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
		ret = t91407_set_batt_org(di, value);
		if (ret) {
			hwlog_err("[%s] set batt org fail, ic_index:%u\n", __func__, di->ic_index);
			goto battinfo_set_fail;
		}
		break;
	case BATT_CERT_READY:
		ret = t91407_set_cert_ready(di);
		if (ret) {
			hwlog_err("[%s] set_cert_ready fail, ic_index:%u\n", __func__, di->ic_index);
			goto battinfo_set_fail;
		}
		break;
	case BATT_MATCH_INFO:
		ret = t91407_write_group_sn(di, value);
		if (ret) {
			hwlog_err("[%s] write group sn fail, ic_index:%u\n", __func__, di->ic_index);
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

static int t91407_get_batt_spare_part(struct t91407_dev *di, void *value)
{
	int ret;
	unsigned char buf[T91407_LOCK_BUF_LEN] = { 0 };

	ret = t91407_operate_file(di, T91407_FILE_LOCK, T91407_LOCK_STATUS, buf, T91407_LOCK_BUF_LEN);
	if (ret) {
		hwlog_err("[%s] check batt state fail, ic_index:%u\n", __func__, di->ic_index);
		return -EINVAL;
	}

	if (t91407_check_lock(buf))
		*(enum batt_source *)value = BATTERY_ORIGINAL;
	else
		*(enum batt_source *)value = BATTERY_SPARE_PART;
	return 0;
}

static int t91407_get_batt_cert_ready(struct t91407_dev *di, void *value)
{
	int ret;
	unsigned char buf[T91407_LOCK_BUF_LEN] = { 0 };

	ret = t91407_operate_file(di, T91407_FILE_ACT, T91407_LOCK_STATUS, buf, T91407_LOCK_BUF_LEN);
	if (ret) {
		hwlog_err("[%s] check cert lock state fail, ic_index:%u\n", __func__, di->ic_index);
		return -EINVAL;
	}
	if (!t91407_check_lock(buf))
		hwlog_err("[%s] cert unready, ic_index:%u\n", __func__, di->ic_index);
	*(enum batt_cert_state *)value = CERT_READY;
	return 0;
}

static int t91407_get_batt_match_info(struct t91407_dev *di, void *value)
{
	int ret;
	uint64_t hash_group_sn = 0;
	uint8_t byte_group_sn[T91407_IC_GROUP_SN_LENGTH] = { 0 };

	if (!di->mem.group_sn_ready) {
		ret = t91407_operate_file(di, T91407_FILE_GSN, T91407_READ, byte_group_sn, T91407_IC_GROUP_SN_LENGTH);
		if (ret) {
			hwlog_err("[%s] read group sn fail, ic_index:%u\n", __func__, di->ic_index);
			return -EINVAL;
		}

		t91407_byte_array_to_u64(&hash_group_sn, byte_group_sn);

		if (!hash_group_sn) {
			hwlog_err("[%s] hash_group_sn is null, ic_index:%u\n", __func__, di->ic_index);
			return -EINVAL;
		}
		ret = snprintf_s(di->mem.group_sn, T91407_SN_ASC_LEN + 1, T91407_SN_ASC_LEN,
			"%016llX", hash_group_sn);
		if (ret < 0) {
			hwlog_err("[%s] snprintf_s is failed\n", __func__);
			return -EINVAL;
		}
		di->mem.group_sn_ready = true;
	}
	*(uint8_t **)value = di->mem.group_sn;
	return 0;
}

static int t91407_get_batt_safe_info(struct platform_device *pdev,
	enum batt_safe_info_t type, void *value)
{
	int ret;
	struct t91407_dev *di = NULL;

	hwlog_info("[%s] type:%d\n", __func__, type);
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
		ret = t91407_get_batt_spare_part(di, value);
		if (ret) {
			hwlog_err("[%s] check batt state fail, ic_index:%u\n", __func__, di->ic_index);
			goto battinfo_get_fail;
		}
		break;
	case BATT_CERT_READY:
		ret = t91407_get_batt_cert_ready(di, value);
		if (ret) {
			hwlog_err("[%s] check cert lock state fail, ic_index:%u\n", __func__, di->ic_index);
			goto battinfo_get_fail;
		}
		break;
	case BATT_MATCH_INFO:
		ret = t91407_get_batt_match_info(di, value);
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

static void t91407_set_ic_status(struct platform_device *pdev, int ic_status)
{
	struct t91407_dev *di = NULL;

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
		t91407_gpio_dir_input(di->onewire_gpio);
		break;
	case BAT_TYPE_DEV_LOW:
		t91407_gpio_dir_output(di->onewire_gpio, T91407_LOW_VOLTAGE);
		break;
	default:
		hwlog_err("[%s] invalid option, ic_index:%u\n", __func__, di->ic_index);
	}
}

static int t91407_ic_check(struct t91407_dev *di)
{
	int retry;
	int ret;

	bat_type_apply_mode(BAT_ID_SN);
	for (retry = 0; retry < T91407_MAX_ICCHECK_RETRY_COUNT; retry++) {
		ret = t91407_ic_ck(di);
		if (!ret)
			break;
	}
	bat_type_release_mode(true);
	hwlog_info("[%s] ic check ret =%d, ic_index:%u\n", __func__, ret, di->ic_index);
	return ret;
}

static int t91407_ct_ops_register(struct platform_device *pdev,
	struct batt_ct_ops *bco)
{
	int ret;
	struct t91407_dev *di = NULL;

	hwlog_info("[%s] start\n", __func__);

	if (!bco || !pdev) {
		hwlog_err("[%s] bco/pdev NULL\n", __func__);
		return -ENXIO;
	}
	di = platform_get_drvdata(pdev);
	if (!di)
		return -EINVAL;
	ret = t91407_ic_ck(di);
	if (ret) {
		hwlog_err("[%s] ic check fail, ic_index:%u\n",
			__func__, di->ic_index);
		return -ENXIO;
	}
	hwlog_info("[%s] ic matched, ic_index:%u\n", __func__, di->ic_index);

	bco->get_ic_type = t91407_get_ic_type;
	bco->get_ic_uuid = t91407_get_uid;
	bco->get_batt_type = t91407_get_batt_type;
	bco->get_batt_sn = t91407_get_batt_sn;
	bco->certification = t91407_certification;
	bco->set_act_signature = t91407_set_act_signature;
	bco->prepare = t91407_prepare;
	bco->set_batt_safe_info = t91407_set_batt_safe_info;
	bco->get_batt_safe_info = t91407_get_batt_safe_info;
	bco->set_ic_status = t91407_set_ic_status;
	bco->power_down = NULL;
	return 0;
}

static void t91407_hex_to_str(char *pdest, const char *psrc, int ilen)
{
	char ddl, ddh;
	int i;

	for (i = 0; i < ilen; i++) {
		ddh = '0' + psrc[i] / T91407_ADDR_OFFSET_BIT4; /* 高4位 */
		ddl = '0' + psrc[i] % T91407_ADDR_OFFSET_BIT4; /* 低4位 */
		if (ddh > T91407_ASCLL_OF_NUMBER)
			ddh = ddh + T91407_OFFSET_LETTER;
		if (ddl > T91407_ASCLL_OF_NUMBER)
			ddl = ddl + T91407_OFFSET_LETTER;
		pdest[i * 2] = ddh;
		pdest[i * 2 + 1] = ddl;
	}
	pdest[ilen * 2] = '\0';
}

static int t91407_get_group_sn(struct platform_device *pdev, char *group_sn)
{
	int ret;
	struct t91407_dev *di = platform_get_drvdata(pdev);
	uint64_t hash_group_sn = 0;
	uint8_t byte_group_sn[T91407_IC_GROUP_SN_LENGTH] = { 0 };

	if (!di)
		return -ENODEV;
	if (!di->mem.group_sn_ready) {
		ret = t91407_operate_file(di, T91407_FILE_GSN, T91407_READ, byte_group_sn, T91407_IC_GROUP_SN_LENGTH);
		if (ret) {
			hwlog_err("[%s] read group sn fail, ic_index:%u\n", __func__, di->ic_index);
			return -EINVAL;
		}
		t91407_byte_array_to_u64(&hash_group_sn, byte_group_sn);
		ret = snprintf_s(di->mem.group_sn, T91407_SN_ASC_LEN + 1, T91407_SN_ASC_LEN,
			"%016llX", hash_group_sn);
		if (ret < 0) {
			hwlog_err("[%s] snprintf_s is failed\n", __func__);
			return -EINVAL;
		}
		di->mem.group_sn_ready = true;
	}
	ret = memcpy_s(group_sn, T91407_SN_ASC_LEN + 1, di->mem.group_sn, T91407_SN_ASC_LEN + 1);
	if (ret != EOK) {
		hwlog_err("[%s] memcpy_s failed\n", __func__);
		return -EINVAL;
	}
	return 0;
}

static int t91407_group_sn_write(struct platform_device *pdev,
	uint8_t *group_sn)
{
	int ret;
	struct t91407_dev *di = platform_get_drvdata(pdev);
	int8_t buf[T91407_IC_GROUP_SN_LENGTH];
	uint64_t val = 0;
	int i;

	if (!di)
		return -ENODEV;

	ret = t91407_hex_array_to_u64(&val, group_sn);
	if (ret) {
		hwlog_err("[%s] hex to u64 fail\n", __func__);
		return -EINVAL;
	}
	hwlog_err("[%s] val = %016llX\n", __func__, val);
	t91407_u64_to_byte_array(val, buf);
	for (i = 0; i < T91407_IC_GROUP_SN_LENGTH; ++i)
		hwlog_err("[%s] buf[%d] = %u\n", __func__, i, buf[i]);

	ret = t91407_operate_file(di, T91407_FILE_GSN, T91407_WRITE, buf, T91407_IC_GROUP_SN_LENGTH);
	if (ret) {
		hwlog_err("[%s] write group sn fail, ic_index:%u\n", __func__, di->ic_index);
		return -EINVAL;
	}
	di->mem.group_sn_ready = false;
	return 0;
}

static ssize_t ic_type_show(struct device *dev, struct device_attribute *attr,
	char *buf)
{
	return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "ic type: %d", t91407_get_ic_type());
}

static ssize_t uid_show(struct device *dev, struct device_attribute *attr,
	char *buf)
{
	struct t91407_dev *di = NULL;
	char uid[T91407_UID_HEX_LEN + 1] = { 0 };

	dev_get_drv_data(di, dev);
	if (!di)
		return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "Error data");

	bat_type_apply_mode(BAT_ID_SN);
	if (t91407_read_romid(di, di->mem.uid)) {
		hwlog_err("[%s] read uid error\n", __func__);
		bat_type_release_mode(true);
		return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "read uid error");
	}
	bat_type_release_mode(true);
	t91407_hex_to_str(uid, di->mem.uid, T91407_UID_LEN);
	return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%s", uid);
}

static ssize_t batt_type_show(struct device *dev, struct device_attribute *attr,
	char *buf)
{
	struct t91407_dev *di = NULL;
	int ret;
	uint8_t type[T91407_BATTTYP_LEN] = { 0 };
	uint8_t type_temp[T91407_BATTTYP_LEN] = { 0 };

	dev_get_drv_data(di, dev);
	if (!di)
		return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "Error data");
	if (!di->mem.batt_type_ready) {
		bat_type_apply_mode(BAT_ID_SN);
		ret = (t91407_operate_file(di, T91407_FILE_BATTTYPE, T91407_READ, type_temp, T91407_BATTTYP_LEN));
		bat_type_release_mode(true);
		if (ret) {
			hwlog_err("[%s] read battery type err\n", __func__);
			return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "read batt type error");
		}
		di->mem.batt_type[0] = type_temp[1];
		di->mem.batt_type[1] = type_temp[0];
		di->mem.batt_type_ready = true;
	}
	type[0] = di->mem.batt_type[0];
	type[1] = di->mem.batt_type[1];

	return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "[%s] Btp0:0x%x; Btp1:0x%x, ic_index:%u\n",
		__func__, type[T91407_PKVED_IND], type[T91407_CELVED_IND], di->ic_index);
}

static ssize_t sn_show(struct device *dev, struct device_attribute *attr,
	char *buf)
{
	struct t91407_dev *di = NULL;
	int ret;
	char write[T91407_DATA_READ_LEN] = { 0 };

	dev_get_drv_data(di, dev);
	if (!di)
		return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "Error data");
	if (!di->mem.sn_ready) {
		memset_s(di->mem.sn, T91407_SN_ASC_LEN, 0, T91407_SN_ASC_LEN);
		bat_type_apply_mode(BAT_ID_SN);
		ret = t91407_operate_file(di, T91407_FILE_SN, T91407_READ, di->mem.sn, T91407_SN_ASC_LEN);
		bat_type_release_mode(true);
		if (ret) {
			hwlog_err("[%s] batt type iqr fail\n", __func__);
			return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "read sn error");
		}
		di->mem.sn_ready = true;
	}
	t91407_print_data(di->mem.sn, T91407_SN_ASC_LEN, write, T91407_DATA_READ_LEN);
	hwlog_info("[%s] SN[ltoh]: %s, ic_index:%u\n", __func__, write, di->ic_index);
	return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1,
		"SN [ltoh]: %c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c\n", di->mem.sn[0], di->mem.sn[1], di->mem.sn[2],
		di->mem.sn[3], di->mem.sn[4], di->mem.sn[5], di->mem.sn[6], di->mem.sn[7], di->mem.sn[8], di->mem.sn[9],
		di->mem.sn[10], di->mem.sn[11], di->mem.sn[12], di->mem.sn[13], di->mem.sn[14], di->mem.sn[15]);
}

static ssize_t group_sn_show(struct device *dev, struct device_attribute *attr,
	char *buf)
{
	struct t91407_dev *di = NULL;
	int ret;
	char group_sn[T91407_IC_GROUP_SN_LENGTH + 1] = { 0 };
	struct platform_device *pdev = NULL;

	pdev = container_of(dev, struct platform_device, dev);
	dev_get_drv_data(di, dev);
	if (!di || !pdev)
		return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "Error data");

	bat_type_apply_mode(BAT_ID_SN);
	ret = t91407_get_group_sn(pdev, group_sn);
	bat_type_release_mode(true);
	if (ret) {
		hwlog_err("[%s] get group sn fail\n", __func__);
		return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "get group sn failed");
	}
	return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "group sn: %s", group_sn);
}

static ssize_t group_sn_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int ret;
	struct t91407_dev *di;
	uint8_t byte_group_sn[T91407_IC_GROUP_SN_LENGTH] = { 0 };
	struct platform_device *pdev = NULL;

	pdev = container_of(dev, struct platform_device, dev);
	dev_get_drv_data(di, dev);

	if (!di || !pdev || (count > T91407_IC_GROUP_SN_LENGTH))
		return -EINVAL;

	ret = memcpy_s(byte_group_sn, T91407_IC_GROUP_SN_LENGTH, buf, count);
	if (ret != EOK) {
		hwlog_err("[%s] memcpy_s failed\n", __func__);
		return -EINVAL;
	}
	bat_type_apply_mode(BAT_ID_SN);
	ret = t91407_group_sn_write(pdev, byte_group_sn);
	bat_type_release_mode(true);
	if (ret)
		hwlog_err("[%s] write group sn fail\n", __func__);
	return count;
}

static ssize_t actsig_show(struct device *dev, struct device_attribute *attr,
	char *buf)
{
	int ret;
	struct t91407_dev *di = NULL;
	char act[T91407_ACT_HEX_LEN + 1] = { 0 };

	dev_get_drv_data(di, dev);
	if (!di)
		return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "Error data");

	bat_type_apply_mode(BAT_ID_SN);
	ret = t91407_act_read(di);
	bat_type_release_mode(true);
	if (ret) {
		hwlog_err("[%s] res_act read fail, ic_index:%u\n", __func__, di->ic_index);
		return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "get actsig failed");
	}
	t91407_hex_to_str(act, di->mem.act_sign, T91407_ACT_LEN);
	return scnprintf(buf, PAGE_SIZE, "Act = %s\n", act);
}

static ssize_t actsig_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int ret;
	uint8_t act_data[T91407_ACT_LEN];
	struct power_genl_attr res;
	enum res_type type = RES_ACT;
	struct t91407_dev *di;
	struct platform_device *pdev = NULL;

	pdev = container_of(dev, struct platform_device, dev);
	dev_get_drv_data(di, dev);
	if (!di || !pdev || (count > T91407_ACT_LEN))
		return -EINVAL;

	ret = memcpy_s(act_data, count, buf, count);
	if (ret != EOK) {
		hwlog_err("[%s] memcpy_s failed\n", __func__);
		return -EINVAL;
	}
	res.data = act_data;
	res.len = count;

	bat_type_apply_mode(BAT_ID_SN);
	ret = t91407_set_act_signature(pdev, type, &res);
	bat_type_release_mode(true);
	if (ret)
		hwlog_err("[%s] act set fail\n", __func__);
	return count;
}

static ssize_t org_spar_show(struct device *dev, struct device_attribute *attr,
	char *buf)
{
	int ret;
	enum batt_safe_info_t type = BATT_SPARE_PART;
	enum batt_source batt_spar;
	struct t91407_dev *di;
	struct platform_device *pdev = NULL;

	pdev = container_of(dev, struct platform_device, dev);
	dev_get_drv_data(di, dev);
	if (!di || !pdev)
		return -EINVAL;

	bat_type_apply_mode(BAT_ID_SN);
	ret = t91407_get_batt_safe_info(pdev, type, (void *)&batt_spar);
	bat_type_release_mode(true);
	if (ret) {
		hwlog_err("[%s] batt spar check %d fail\n", __func__, ret);
		return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "batt spar check failed");
	}
	return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "batt: %d", batt_spar);
}

static ssize_t org_spar_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int ret;
	long val;
	enum batt_safe_info_t type = BATT_SPARE_PART;
	struct t91407_dev *di;
	struct platform_device *pdev = NULL;

	pdev = container_of(dev, struct platform_device, dev);
	dev_get_drv_data(di, dev);
	if (!di || !pdev)
		return -EINVAL;

	if (kstrtol(buf, POWER_BASE_DEC, &val) < 0) {
		hwlog_err("[%s] val is not valid!, ic_index:%u\n",
			__func__, di->ic_index);
		return count;
	}
	bat_type_apply_mode(BAT_ID_SN);
	ret = t91407_set_batt_safe_info(pdev, type, (void *)&val);
	bat_type_release_mode(true);
	if (ret)
		hwlog_err("[%s] org set %d fail\n", __func__, ret);
	return count;
}

static ssize_t cert_status_show(struct device *dev, struct device_attribute *attr,
	char *buf)
{
	int ret;
	struct t91407_dev *di;
	unsigned char cert_status_buf[T91407_LOCK_BUF_LEN] = { 0 };

	dev_get_drv_data(di, dev);
	if (!di)
		return -EINVAL;

	bat_type_apply_mode(BAT_ID_SN);
	ret = t91407_operate_file(di, T91407_FILE_ACT, T91407_LOCK_STATUS, cert_status_buf, T91407_LOCK_BUF_LEN);
	bat_type_release_mode(true);
	if (ret) {
		hwlog_err("[%s] check lock state fail, ic_index:%u\n", __func__, di->ic_index);
		return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "check lock state fail");
	}
	return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "cert status: %s", cert_status_buf);
}

static ssize_t cert_status_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int ret;
	long val = 0;
	struct t91407_dev *di;
	enum batt_safe_info_t type = BATT_CERT_READY;
	struct platform_device *pdev = NULL;

	pdev = container_of(dev, struct platform_device, dev);
	dev_get_drv_data(di, dev);
	if (!di || !pdev)
		return -EINVAL;

	if ((kstrtol(buf, POWER_BASE_DEC, &val) < 0) || (val != 1)) {
		hwlog_err("[%s] : val is not valid!, ic_index:%u\n",
			__func__, di->ic_index);
		return count;
	}

	bat_type_apply_mode(BAT_ID_SN);
	ret = t91407_set_batt_safe_info(pdev, type, (void *)&val);
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
static DEVICE_ATTR_RW(org_spar);
static DEVICE_ATTR_RW(cert_status);
static DEVICE_ATTR_RW(group_sn);
#endif /* BATTERY_LIMIT_DEBUG */

static const struct attribute *t91407_batt_checker_attrs[] = {
	&dev_attr_ic_type.attr,
	&dev_attr_uid.attr,
	&dev_attr_batt_type.attr,
	&dev_attr_sn.attr,
#ifdef ONEWIRE_STABILITY_DEBUG
	&dev_attr_actsig.attr,
	&dev_attr_org_spar.attr,
	&dev_attr_cert_status.attr,
	&dev_attr_group_sn.attr,
#endif /* BATTERY_LIMIT_DEBUG */
	NULL, /* sysfs_create_files need last one be NULL */
};

static int t91407_sysfs_create(struct platform_device *pdev)
{
	return sysfs_create_files(&pdev->dev.kobj, t91407_batt_checker_attrs);
}

static void t91407_parse_protocol(struct t91407_dev *di, struct device_node *np)
{
	power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"ow_tau_delay", &di->t91407_swi.ow_tau_delay, T91407_DEFAULT_TAU_DELAY);

	di->t91407_swi.ow_bit0_delay = di->t91407_swi.ow_tau_delay;
	di->t91407_swi.ow_bit1_delay = di->t91407_swi.ow_tau_delay * T91407_SWI_DELAY_BIT_1_TIMES;
	di->t91407_swi.ow_stop_delay = di->t91407_swi.ow_tau_delay * T91407_SWI_DELAY_STOP_TIMES;
	di->t91407_swi.ow_timeout_delay = di->t91407_swi.ow_tau_delay * T91407_SWI_DELAY_TIMEOUT_TIMES;
	di->t91407_swi.ow_timeout_bit_delay = di->t91407_swi.ow_tau_delay * T91407_SWI_DELAY_TIMEOUT_BIT_TIMES;
	di->t91407_swi.ow_execution_delay = T91407_DELAY_OF_EXECUTION;
}

static int t91407_parse_dts(struct t91407_dev *di, struct device_node *np)
{
	if (power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "ic_index", &di->ic_index, T91407_DFT_IC_INDEX))
		hwlog_err("[%s] ic_index not given in dts\n", __func__);
	hwlog_info("[%s] ic_index = 0x%x\n", __func__, di->ic_index);

	t91407_parse_protocol(di, np);
	return 0;
}

static int t91407_gpio_init(struct platform_device *pdev,
	struct t91407_dev *di)
{
	int ret;

	di->onewire_gpio = of_get_named_gpio(pdev->dev.of_node,
		"onewire-gpio", 0);

	if (!gpio_is_valid(di->onewire_gpio)) {
		hwlog_err("[%s] onewire_gpio is not valid, ic_index:%u\n", __func__, di->ic_index);
		return -EINVAL;
	}
	/* get the gpio */
	ret = devm_gpio_request(&pdev->dev, di->onewire_gpio, "onewire_t91407");
	if (ret) {
		hwlog_err("[%s] gpio request failed %d, ic_index:%u\n",
			__func__, di->onewire_gpio, di->ic_index);
		return ret;
	}
	t91407_gpio_dir_input(di->onewire_gpio);
	return 0;
}

static int t91407_probe(struct platform_device *pdev)
{
	int ret;
	struct t91407_dev *di = NULL;
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

	ret = t91407_parse_dts(di, np);
	if (ret) {
		hwlog_err("[%s] dts parse fail, ic_index:%u\n", __func__, di->ic_index);
		goto probe_fail;
	}

	ret = t91407_gpio_init(pdev, di);
	if (ret) {
		hwlog_err("[%s] gpio init fail, ic_index:%u\n", __func__, di->ic_index);
		goto probe_fail;
	}

	ret = t91407_ic_check(di);
	if (ret)
		goto ic_ck_fail;

	di->reg_node.ic_memory_release = NULL;
	di->reg_node.ic_dev = pdev;
	di->reg_node.ct_ops_register = t91407_ct_ops_register;
	add_to_aut_ic_list(&di->reg_node);

	ret = t91407_sysfs_create(pdev);
	if (ret)
		hwlog_err("[%s] sysfs create fail, ic_index:%u\n", __func__, di->ic_index);

	power_dev_info = power_devices_info_register();
	if (power_dev_info) {
		power_dev_info->dev_name = pdev->dev.driver->name;
		power_dev_info->dev_id = 0;
		power_dev_info->ver_id = 0;
	}

	platform_set_drvdata(pdev, di);

	hwlog_info("[%s] success!\n", __func__);
	return 0;

ic_ck_fail:
	devm_gpio_free(&pdev->dev, di->onewire_gpio);
probe_fail:
	hwlog_info("[%s] probe_fail\n", __func__);
	kfree(di);
	return ret;
}

static int t91407_remove(struct platform_device *pdev)
{
	return 0;
}

static const struct of_device_id t91407_ow_dt_match[] = {
	{
		.compatible = "tmc,t91407",
		.data = NULL,
	},
	{},
};

static struct platform_driver t91407_driver = {
	.probe          = t91407_probe,
	.remove         = t91407_remove,
	.driver         = {
		.owner      = THIS_MODULE,
		.name       = "t91407",
		.of_match_table = t91407_ow_dt_match,
	},
};

static int __init t91407_init(void)
{
	return platform_driver_register(&t91407_driver);
}

static void __exit t91407_exit(void)
{
	platform_driver_unregister(&t91407_driver);
}

subsys_initcall_sync(t91407_init);
module_exit(t91407_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("t91407 ic");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
