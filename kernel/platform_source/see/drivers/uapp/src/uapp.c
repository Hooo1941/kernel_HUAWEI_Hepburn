/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: uapp source file
 * Create: 2022/04/20
 */

#include <platform_include/see/uapp.h>
#include <platform_include/see/efuse_driver.h>
#include <platform_include/see/dev_secinfo.h>
#include <uapp_utils.h>
#include <linux/device.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_platform.h>
#include <linux/version.h>
#include <linux/arm-smccc.h>       /* arm_smccc_smc */
#include <securec.h>               /* memset_s */

/* kernel to atf smc id */
#define FID_UAPP_GET_ENABLE_STATE              0xC3002F00u
#define FID_UAPP_SET_ENABLE_STATE              0xC3002F01u
#define FID_UAPP_VALID_BINDFILE_PUBKEY         0xC3002F02u
#define FID_UAPP_REVOKE_BINDFILE_PUBKEY        0xC3002F03u

#define UAPP_BINDFILE_PUBKEY_INDEX_MAX         32u

struct smc_arg_wrap {
	uintptr_t x0;
	uintptr_t x1;
	uintptr_t x2;
	uintptr_t x3;
	uintptr_t x4;
};

/*
 * adapt local smc function
 */
static u32 smc_switch_to_atf(struct smc_arg_wrap *arg)
{
	struct arm_smccc_res res;
	arm_smccc_smc(arg->x0, arg->x1, arg->x2, arg->x3,
			arg->x4, 0, 0, 0, &res);
	arg->x1 = res.a1;
	arg->x2 = res.a2;
	arg->x3 = res.a3;
	return (u32)res.a0;
}

/*
 * get uapp enable state
 *
 * @param
 * state              pointer to save state
 *                    [0-DISABLE] [1-ENABLE] [other-INVALID]
 * @return
 * UAPP_OK            operation succeed
 * other              error code
 */
u32 uapp_get_enable_state(u32 *state)
{
	u32 ret;
	u32 val = 0;
	struct efuse_item item = { 0 };

	if (!state) {
		uapp_err ("[%s]: error, state NULL\n", __func__);
		return UAPP_NULL_PTR_ERR;
	}

	item.buf = &val;
	item.buf_size = sizeof(val) / sizeof(u32);
	item.item_vid = EFUSE_KERNEL_UAPP_ENABLE;
	ret = efuse_read_item(&item);
	if (ret != EFUSE_OK) {
		uapp_err("[%s]: error, read efuse ret=0x%08X", __func__, ret);
		return UAPP_READ_EFUSE_ERR;
	}

	*state = (val == 0 ? UAPP_DISABLE : UAPP_ENABLE);
	return UAPP_OK;
}

/*
 * set uapp enable state
 *
 * @param
 * state              the state to set
 *                    [0-DISABLE] [1-ENABLE] [other-INVALID]
 * @return
 * UAPP_OK            operation succeed
 * other              error code
 */
u32 uapp_set_enable_state(u32 state)
{
	u32 ret;

	if (state >= UAPP_ENABLE_STATE_MAX) {
		uapp_err("[%s]: error, invalid state=0x%08X", __func__, state);
		return UAPP_INVALID_PARAM_ERR;
	}

	ret = dev_secinfo_set_uapp_enable_state(state);
	if (ret != DEV_SECINFO_OK) {
		uapp_err("[%s]: error, dev set uapp_en ret=0x%08X", ret);
		return ret;
	}

	return UAPP_OK;
}

/*
 * valid bindfile authkey by key_idx
 *
 * @param
 * key_idx            the key_idx to valid
 *
 * @return
 * UAPP_OK            operation succeed
 * other              error code
 */
u32 uapp_valid_bindfile_pubkey(u32 key_idx)
{
	u32 ret;
	u32 val = 0;
	struct efuse_item item = { 0 };

	if (key_idx >= UAPP_BINDFILE_PUBKEY_INDEX_MAX) {
		uapp_err("[%s]: error, invalid key_idx=0x%08X", __func__, key_idx);
		return UAPP_INVALID_PARAM_ERR;
	}

	item.buf = &val;
	item.buf_size = sizeof(val) / sizeof(u32);
	item.item_vid = EFUSE_KERNEL_UAPP_KEYREVOKE;
	ret = efuse_read_item(&item);
	if (ret != EFUSE_OK) {
		uapp_err("[%s]: error, read efuse ret=0x%08X", __func__, ret);
		return UAPP_READ_EFUSE_ERR;
	}

	if ((val & (1u << key_idx)) != 0) {
		uapp_err("[%s]: error, uapp key_idx=0x%08X is revoked\n", __func__, key_idx);
		return UAPP_AUTHKEY_IS_REVOKED_ERR;
	}

	return UAPP_OK;
}

/*
 * uapp revoke certain bindfile authkey by key_idx
 *
 * @param
 * key_idx            the key_idx to revoke
 *
 * @return
 * UAPP_OK            operation succeed
 * other              error code
 */
u32 uapp_revoke_bindfile_pubkey(u32 key_idx)
{
	u32 ret;
	struct smc_arg_wrap smc_args = { 0 };

	if (key_idx >= UAPP_BINDFILE_PUBKEY_INDEX_MAX) {
		uapp_err("[%s]: error, invalid key_idx=0x%08X", __func__, key_idx);
		return UAPP_INVALID_PARAM_ERR;
	}

	smc_args.x0 = (uintptr_t)FID_UAPP_REVOKE_BINDFILE_PUBKEY;
	smc_args.x1 = (uintptr_t)key_idx;
	ret = smc_switch_to_atf(&smc_args);
	if (ret != UAPP_OK) {
		uapp_err("[%s]: error, smc failed, smc_id=0x%08X, ret=0x%08X", \
		      __func__, smc_args.x0, ret);
		return UAPP_SMC_PROC_ERR;
	}

	return UAPP_OK;
}

#define UAPP_CERT_PATH   "/uapp/cert"

static u32 is_uapp_x509_cert(void)
{
	int ret;
	const char *format = NULL;
	struct device_node *np = NULL;

	uapp_info("enter %s\n", __func__);

	np = of_find_node_by_path(UAPP_CERT_PATH);
	if (!np) {
		uapp_err("error, cannot find node %s\n", UAPP_CERT_PATH);
		return 0;
	}

	ret = of_property_read_string(np, "cert_format", &format);
	if (ret != 0) {
		uapp_err("error 0x%x, cannot read cert_format\n", ret);
		return 0;
	}

	if (strcmp(format, "x509") != 0) {
		uapp_err("uapp not x509 cert\n");
		return 0;
	}

	return 1;
}

#define SHA256_BYTES   0x20
static u32 uapp_get_hbk_hash(struct rotpk_hash *rotpk_hash)
{
	u32 ret;
	errno_t err;
	uint32_t read_size = 0;
	struct efuse_item item = { 0 };
	uint32_t buf[SHA256_BYTES / sizeof(uint32_t)] = { 0 };

	if (!rotpk_hash) {
		uapp_err("[%s]: error, rotpk_hash NULL\n", __func__);
		return UAPP_NULL_PTR_ERR;
	}

	item.buf = buf;
	item.buf_size = (uint32_t)(ARRAY_SIZE(buf));
	item.item_vid = EFUSE_KERNEL_ROTPK_HASH0;
	ret = efuse_read_item(&item);
	if (ret != EFUSE_OK) {
		uapp_err("[%s]: error 0x%x, read rotpk hash0", __func__, ret);
		return UAPP_READ_EFUSE_ERR;
	}

	/* add the read size of first part */
	read_size += item.buf_size;

	item.buf = buf + read_size;
	item.buf_size = (uint32_t)(ARRAY_SIZE(buf)) - read_size;
	item.item_vid = EFUSE_KERNEL_ROTPK_HASH1;
	ret = efuse_read_item(&item);
	if (ret != EFUSE_OK) {
		uapp_err("[%s]: error 0x%x, read rotpk hash1", __func__, ret);
		return UAPP_READ_EFUSE_ERR;
	}

	/* add the read size of second part */
	read_size += item.buf_size;
	err = memcpy_s(&rotpk_hash->val[0], sizeof(rotpk_hash->val), \
			buf, read_size * sizeof(uint32_t));
	if (err != EOK) {
		uapp_err("[%s]: error 0x%x, cp rotph hash", __func__, err);
		return UAPP_READ_EFUSE_ERR;
	}

	/* get the total size of read data */
	rotpk_hash->bytes = read_size * sizeof(uint32_t);
	return UAPP_OK;
}

static u32 uapp_get_gjrotpk_hash(struct rotpk_hash *rotpk_hash)
{
	u32 ret;
	errno_t err;
	struct efuse_item item = { 0 };
	uint32_t buf[SHA256_BYTES / sizeof(uint32_t)] = { 0 };

	if (!rotpk_hash) {
		uapp_err("[%s]: error, rotpk_hash NULL\n", __func__);
		return UAPP_NULL_PTR_ERR;
	}

	item.buf = buf;
	item.buf_size = (uint32_t)(ARRAY_SIZE(buf));
	item.item_vid = EFUSE_KERNEL_GJROTPK_HASH;
	ret = efuse_read_item(&item);
	if (ret != EFUSE_OK) {
		uapp_err("[%s]: error 0x%x, read rotph hash0", __func__, ret);
		return UAPP_READ_EFUSE_ERR;
	}

	err = memcpy_s(&rotpk_hash->val[0], sizeof(rotpk_hash->val), \
			buf, item.buf_size * sizeof(uint32_t));
	if (err != EOK) {
		uapp_err("[%s]: error 0x%x, cp rotph hash", __func__, err);
		return UAPP_READ_EFUSE_ERR;
	}

	/* get the total size of read data */
	rotpk_hash->bytes = item.buf_size * sizeof(uint32_t);
	return UAPP_OK;
}

u32 uapp_get_rotpk_hash(struct rotpk_hash *rotpk_hash)
{
	if (is_uapp_x509_cert() != 0)
		return uapp_get_gjrotpk_hash(rotpk_hash);

	return uapp_get_hbk_hash(rotpk_hash);
}
