// SPDX-License-Identifier: GPL-2.0+
/*
 * fm1230_api.c
 *
 * interface for fm1230.c
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

#include "fm1230_api.h"
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
#include <huawei_platform/power/power_mesg_srv.h>
#include "../batt_aut_checker.h"
#include "fm1230.h"
#include "core/fm1230_ecdsa_api.h"

#define HASH_LENGTH             32
#define SIG_LENGTH              64
/* time measurement setting */

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif
#define HWLOG_TAG fm1230_API
HWLOG_REGIST();

static struct fm1230_swi_rdwr_ops *g_pow_se_ops;
static u8 g_ecc_sig_res[ECC_SIG_SIZE];

static void set_sched_affinity_to_current(void)
{
	long retval;
	int current_cpu;

	preempt_disable();
	current_cpu = smp_processor_id();
	preempt_enable();
	retval = sched_setaffinity(0, cpumask_of(current_cpu));
	if (retval)
		hwlog_info("[%s] Set cpu af to current cpu failed %ld\n", __func__, retval);
	else
		hwlog_info("[%s] Set cpu af to current cpu %d\n", __func__, current_cpu);
}

static void set_sched_affinity_to_all(void)
{
	long retval;
	cpumask_t dstp;

	cpumask_setall(&dstp);
	retval = sched_setaffinity(0, &dstp);
	if (retval)
		hwlog_info("[%s] Set cpu af to all valid cpus failed %ld\n", __func__, retval);
	else
		hwlog_info("[%s] Set cpu af to all valid cpus\n", __func__);
}

void fm1230_power_on(struct fm1230_dev *di)
{
	if (!di) {
		hwlog_err("[%s] di is null\n", __func__);
		return;
	}
	gpio_direction_output(di->onewire_gpio, HIGH_VOLTAGE);
	mdelay(FM1230_POWER_ON);
	gpio_direction_input(di->onewire_gpio);
}

void fm1230_power_off(struct fm1230_dev *di)
{
	if (!di) {
		hwlog_err("[%s] di is null\n", __func__);
		return;
	}
	gpio_direction_input(di->onewire_gpio);
}

static void fm1230_dev_on(struct fm1230_dev *di)
{
	set_sched_affinity_to_current();
	local_irq_save(di->irq_flags);
	fm1230_power_on(di);
	g_pow_se_ops->dev_reset(di);
}

static void fm1230_dev_off(struct fm1230_dev *di)
{
	fm1230_power_off(di);
	local_irq_restore(di->irq_flags);
	set_sched_affinity_to_all();
}

static bool is_all_zero(char *buf, int len)
{
	int i;

	for (i = 0; i < len; i++) {
		if (buf[i] != 0)
			return false;
	}

	return true;
}

void fm1230_ops_register(struct fm1230_swi_rdwr_ops *se_ops_swi)
{
	g_pow_se_ops = se_ops_swi;
}

static u8 crc_low_first(u8 *ptr, u8 len)
{
	u8 i;
	u8 crc = 0;

	while (len--) {
		crc ^= *ptr++;
		for (i = 0; i < BYTE_SIZE; ++i) {
			if (crc & 0x01)
				crc = (crc >> 1) ^ CRC_XOR_VAL;
			else
				crc = (crc >> 1);
		}
	}
	return crc;
}

static u16 do_crc16(u16 data, u16 crc16)
{
	/* crc parity */
	const u16 odd_parity[PARTIY_LEN] = { 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0 };

	data = (data ^ (crc16 & 0xff)) & 0xff;
	crc16 >>= 8;
	if (odd_parity[data & 0xf] ^ odd_parity[data >> 4])
		crc16 ^= CRC16_XOR_VAL;
	data <<= 6;
	crc16 ^= data;
	data <<= 1;
	crc16 ^= data;
	return crc16;
}

static u8 owse_transceive(struct fm1230_dev *di, u8 *write_buf, u16 write_len,
	u8 *read_buf, u16 *read_len, u16 delay_ms)
{
	u8 buf[TRANSCEIVE_LEN]; /* send and recv data temp */
	u16 buf_len, crc16, i;

	if (!g_pow_se_ops)
		return RESULT_NONE_ERR;

	g_pow_se_ops->wbyte(di, CMD_SKIP_ROM);
	buf[CMD_START_INDEX] = CMD_START;
	buf[CMD_WRITE_LEN_INDEX] = write_len + CMD_BYTE_LENGTH;
	buf[CMD_SEND_APDU_INDEX] = CMD_SEND_APDU;
	buf_len = CMD_SEND_LENGTH;

	memcpy(&buf[buf_len], write_buf, write_len);
	buf_len += write_len;
	for (i = 0; i < buf_len; i++)
		g_pow_se_ops->wbyte(di, buf[i]);

	mdelay(OW_SE_CALC_CRC_DELAY); /* wait for SE calculate CRC */
	buf[buf_len++] = g_pow_se_ops->rbyte(di);
	buf[buf_len++] = g_pow_se_ops->rbyte(di);

	crc16 = 0;
	for (i = 0; i < buf_len; i++)
		crc16 = do_crc16(buf[i], crc16);

	if (crc16 != CRC16_RESULT) {
		hwlog_err("[%s] crc1:%04X\n", __func__, crc16);
		return RESULT_TXCMD_ERR;
	}

	/* check for strong pull-up */
	g_pow_se_ops->wbyte(di, CMD_RELEASE_BYTE);
	if (delay_ms > 0)
		mdelay(delay_ms);

	mdelay(OW_SE_PROC_CMD_DELAY); /* wait for SE process command */
	buf[0] = g_pow_se_ops->rbyte(di); /* receive dump byte 0xFF */
	buf[0] = g_pow_se_ops->rbyte(di);

	*read_len = buf[0];
	buf_len = *read_len + RECV_PACK_LENGTH;
	for (i = 0; i < *read_len + RECV_DATA_LENGTH; i++)
		buf[i + 1] = g_pow_se_ops->rbyte(di);

	crc16 = 0;
	for (i = 0; i < buf_len; i++)
		crc16 = do_crc16(buf[i], crc16);

	if (crc16 != CRC16_RESULT) {
		hwlog_err("[%s] crc2:%04X\n", __func__, crc16);
		return RESULT_TXDAT_ERR;
	}
	memcpy(read_buf, buf + RECV_DATA_OFFSET, *read_len);
	return RESULT_SUCCESS;
}

static int fm1230_read_raw_romid(struct fm1230_dev *di, char *buf)
{
	u8 i;
	u8 romid[RECV_ROMID_LEN] = { 0 };
	u8 uid_crc;

	if (!g_pow_se_ops) {
		hwlog_err("[%s] g_pow_se_ops is null\n", __func__);
		return RESULT_NONE_ERR;
	}

	fm1230_dev_on(di);
	g_pow_se_ops->wbyte(di, CMD_READ_ROM);
	udelay(OW_READ_ROMID_DELAY);
	for (i = 0; i < SE_UID_LENGTH; i++)
		romid[i] = g_pow_se_ops->rbyte(di);
	fm1230_dev_off(di);

	romid[CRC_INDEX] = crc_low_first(romid, UID_DATA_SIZE);
	uid_crc = romid[CRC_BYTE1] ^ romid[CRC_BYTE2];
#ifdef ONEWIRE_STABILITY_DEBUG
	for (i = 0; i < RECV_ROMID_LEN; i++)
		hwlog_err("[%s] romid[%d] = %x\n", __func__, i, romid[i]);
	hwlog_err("[%s] uid_crc = %x\n", __func__, uid_crc);
#endif
	if (is_all_zero(romid, READ_UID_SIZE)) {
		hwlog_err("[%s] uid all zero\n", __func__);
		return -EINVAL;
	}
	if (uid_crc) {
		hwlog_err("[%s] read uid error, crc: %d\n", __func__, uid_crc);
		return -EINVAL;
	}

	memcpy(buf, romid, READ_UID_SIZE);
	return 0;
}

int fm1230_ic_ck(struct fm1230_dev *di)
{
	if (!di)
		return -ENODEV;

	return fm1230_read_romid(di, di->mem.uid, 1, 0);
}

static bool get_uid_cache(struct fm1230_dev *di, char *buf)
{
	if (di->mem.uid_ready)
		memmove(buf, di->mem.uid, READ_UID_SIZE);
	return di->mem.uid_ready;
}

static void set_uid_cache(struct fm1230_dev *di, char *buf)
{
	if (di->mem.uid_ready)
		return;
	memmove(di->mem.uid, buf, READ_UID_SIZE);
	di->mem.uid_ready = true;
}

int fm1230_read_romid(struct fm1230_dev *di, char *buf, int times, int delay_ms)
{
	int loop;
	int ret;
	char uid_buffer[READ_UID_SIZE];

	if (!di || !buf || (times <= 0) || (delay_ms < 0))
		return -ENODEV;

	if (get_uid_cache(di, buf))
		return 0;
	for (loop = 0; loop < times; loop++) {
		ret = fm1230_read_raw_romid(di, uid_buffer);
		if (!ret)
			break;
		if (delay_ms)
			msleep(delay_ms);
	}
	if (!ret) {
		memcpy(buf, uid_buffer, READ_UID_SIZE);
		set_uid_cache(di, uid_buffer);
	}
	return ret;
}

static u8 ow_apdu_xfer_once(struct fm1230_dev *di, u8 *write_buf, u16 write_len,
	u8 *read_buf, u16 *read_len, u16 delay_ms)
{
	unsigned char ret = 0;
	unsigned short sw = OWSE_XFER_ERR;

	fm1230_dev_on(di);
	ret = owse_transceive(di, write_buf, write_len, read_buf, read_len, delay_ms);
	fm1230_dev_off(di);
	if (ret)
		return ret;
	sw = read_buf[*read_len - 2] << BYTE_SIZE | read_buf[*read_len - 1];
	if (sw != SW_SUCCESS) {
		hwlog_err("[%s] ow_apdu_xfer_sw = %x\n", __func__, sw);
		return RESULT_SESW_ERR;
	}
	*read_len -= SW_BYTE_NUM;
	return RESULT_SUCCESS;
}

/* apdu process */
static u8 ow_apdu_xfer(struct fm1230_dev *di, u8 *write_buf, u16 write_len, u8 *read_buf, u16 *read_len, u16 delay_ms)
{
	unsigned char ret = 0;
	unsigned char loop = 0;

	for (loop = 0; loop < MAX_RETRY_COUNT; loop++) {
		ret = ow_apdu_xfer_once(di, write_buf, write_len, read_buf, read_len, delay_ms);
		if (!ret)
			return RESULT_SUCCESS;
		msleep(HALF_SECOND);
	}
	hwlog_err("[%s] apdu xfer loop : %d\n", __func__, loop);
	return OWSE_XFER_ERR;
}

static int fm1230_get_dev_cert(struct fm1230_dev *di, char *buf)
{
	u8 cmdbuf[BASE_CMD_LENGTH] = { 0, 0xb0, 0x88, 0, 0x90 };
	u8 rbuf[CERT_RECV_LEN]; /* data + sw */
	u16 cmdlen = BASE_CMD_LENGTH;
	u16 rlen;
	char *retbuf = buf;

	if (ow_apdu_xfer(di, cmdbuf, cmdlen, rbuf, &rlen, CMD_READ_BIN_DELAY)) {
		hwlog_err("[%s] read certificate err\n", __func__);
		return -EINVAL;
	}
	fm_hex_to_str(retbuf, rbuf, DEV_CERT_SIZE); /* SE CERT */
	retbuf += DEV_CERT_OFS;
	snprintf(retbuf, CERT_RECV_LEN - DEV_CERT_OFS, "\n");
	return 0;
}

static int fm1230_ecc_signature(struct fm1230_dev *di, const char *buf, size_t count)
{
	u8 cmdbuf[ECC_CMD_LEN] = { 0x80, 0x3e, 0x00, 0x01, 0x28, 0xc2, 0x02,
		0x0a, 0x91, 0xc1, 0x82, 0x00, 0x20 }; /* ecc cmd */
	u8 rbuf[ECC_RECV_LEN]; /* data + sw */
	u16 cmdlen = BASE_CMD_LENGTH + CMD_DATA_LENGTH;
	u16 rlen;
	u8 msg[MSG_LEN] = { 0 };
	u16 msglen;
	u8 ret;

	if (count > ECC_HASH_MAX_SIZE)
		return count;
	msglen = (count - 1) / 2;
	fm_str_to_hex(msg, buf, msglen);
	memcpy(cmdbuf + cmdlen, msg, msglen);
	cmdlen += msglen;
	ret = ow_apdu_xfer(di, cmdbuf, cmdlen, rbuf, &rlen, CMD_ECC_SIGN_DELAY);
	if (!ret)
		memcpy(g_ecc_sig_res, rbuf, rlen);
	return ret;
}

#ifdef BATTBD_FORCE_MATCH
static int fm1230_ecc_signature_ex(struct fm1230_dev *di, const char *buf, size_t count)
{
	u8 cmd_buf[ECC_CMD_LEN] = { 0x80, 0x3e, 0x00, 0x81, 0x28, 0xc2, 0x02,
		0x0a, 0x91, 0xc1, 0x82, 0x00, 0x20 }; /* ecc cmd */
	u8 read_buf[ECC_RECV_LEN]; /* data + sw */
	u16 cmdlen = BASE_CMD_LENGTH + CMD_DATA_LENGTH;
	u16 rlen;
	u8 msg[MSG_LEN] = { 0 };
	u16 msglen;
	u8 ret;

	if (count > ECC_HASH_MAX_SIZE)
		return count;
	msglen = (count - 1) / 2;
	fm_str_to_hex(msg, buf, msglen);
	memcpy(cmd_buf + cmdlen, msg, msglen);
	cmdlen += msglen;
	ret = ow_apdu_xfer_once(di, cmd_buf, cmdlen, read_buf, &rlen, CMD_ECC_EX_SIGN_DELAY);
	if (!ret)
		memcpy(g_ecc_sig_res, read_buf, rlen);
	return ret;
}
#endif /* BATTBD_FORCE_MATCH */

static void fm1230_get_sig_res(char *buf, int len)
{
	fm_hex_to_str(buf, g_ecc_sig_res, ECC_SIG_SIZE);
	snprintf(buf + ECC_SIG_OFS, len - ECC_SIG_OFS, "\n");
	memset(g_ecc_sig_res, 0, sizeof(g_ecc_sig_res));
}

void gen_rand(unsigned char *rnd, int byte_num)
{
	int i;

	for (i = 0; i < byte_num; i++)
		get_random_bytes(&rnd[i], sizeof(unsigned char));
}

void fm_hex_to_str(char *pdest, const char *psrc, int ilen)
{
	char ddl, ddh;
	int i;

	for (i = 0; i < ilen; i++) {
		ddh = 48 + psrc[i] / 16;
		ddl = 48 + psrc[i] % 16;
		if (ddh > 57)
			ddh = ddh + 7;
		if (ddl > 57)
			ddl = ddl + 7;
		pdest[i * 2] = ddh;
		pdest[i * 2 + 1] = ddl;
	}
	pdest[ilen * 2] = '\0';
}

int fm1230_toupper(int c)
{
	if (c >= 97 && c <= 122)
		return c - 32;
	return c;
}

void fm_str_to_hex(char *pdest, const char *psrc, int ilen)
{
	char h1, h2;
	char s1, s2;
	int i;

	for (i = 0; i < ilen; i++) {
		h1 = psrc[2 * i];
		h2 = psrc[2 * i + 1];
		s1 = fm1230_toupper(h1) - 0x30;
		if (s1 > 9)
			s1 -= 7;
		s2 = fm1230_toupper(h2) - 0x30;
		if (s2 > 9)
			s2 -= 7;
		pdest[i] = s1 * 16 + s2;
	}
}

static int get_dev_info(struct fm1230_dev *di, struct power_genl_attr *key_res, uint8_t *dev_info)
{
	int ret;
	char buffer[DEV_INFO_LEN];
	char cert_hash[HASH_LENGTH];
	uint8_t comb_table_qbuf[COMB_TABLE_QBUF_LEN];

	/* init fmecsda */
	fm1230_ecdsa_precombpoint(comb_table_qbuf, COMB_TABLE_QBUF_LEN, key_res->data, key_res->len);

	memset(buffer, 0, sizeof(buffer));
	ret = fm1230_read_romid(di, buffer, MAX_RETRY_COUNT, HALF_SECOND);
	if (ret) {
		hwlog_info("[%s] UID read failed\n", __func__);
		return -EINVAL;
	}
	hwlog_info("[%s] UID read success\n", __func__);
	fm_str_to_hex(dev_info, buffer, READ_UID_SIZE);

	memset(buffer, 0, sizeof(buffer));
	ret = fm1230_get_dev_cert(di, buffer);
	if (ret)
		return -EINVAL;

	hwlog_info("[%s] CERT read success\n", __func__);
	fm_str_to_hex(dev_info + READ_UID_SIZE, buffer, DEV_CERT_SIZE);

	fm1230_sha256(dev_info + READ_UID_SIZE, CERT_LEN, cert_hash);
	ret = fm1230_ecdsa_verify(comb_table_qbuf, COMB_TABLE_QBUF_LEN, key_res->data, key_res->len,
		cert_hash, HASH_LENGTH, dev_info + CERT_LEN + READ_UID_SIZE, SIG_LENGTH);
	hwlog_err("[%s] AP verify cert result: %d\n", __func__, ret); /* 0: success */

	return ret;
}

static int do_ecc_signature(struct fm1230_dev *di, uint8_t *msg_hash,
	uint8_t *sig_res, enum ecc_sign_mode mode)
{
	int ret;
	char buffer[DEV_INFO_LEN]; /* msg_hash */

	fm_hex_to_str(buffer, msg_hash, HASH_LENGTH);
	hwlog_info("[%s] hash:%s\n", __func__, buffer);
	switch (mode) {
	case ECC_NORMAL:
		ret = fm1230_ecc_signature(di, buffer, HASH_LENGTH * 2 + 1);
		break;
#ifdef BATTBD_FORCE_MATCH
	case ECC_EX:
		ret = fm1230_ecc_signature_ex(di, buffer, HASH_LENGTH * 2 + 1);
		break;
#endif /* BATTBD_FORCE_MATCH */
	default:
		hwlog_err("[%s] no operetion\n", __func__);
		return -EINVAL;
	}

	if (ret)
		return -EINVAL;

	fm1230_get_sig_res(buffer, DEV_INFO_LEN);
	fm_str_to_hex(sig_res, buffer, SIG_LENGTH);

	return 0;
}

static int verify_device(struct fm1230_dev *di, uint8_t *dev_cert, enum ecc_sign_mode mode)
{
	int ret;
	unsigned char trnd[TRND_LEN];
	unsigned char trnd_hash[HASH_LENGTH];
	unsigned char sig[SIG_LENGTH];
	uint8_t comb_table_qbuf[COMB_TABLE_QBUF_LEN];

	/* init fmecsda */
	fm1230_ecdsa_precombpoint(comb_table_qbuf, COMB_TABLE_QBUF_LEN, dev_cert + CERT_HEAD, SIG_LENGTH);

	memset(trnd_hash, 0, sizeof(trnd_hash));
	memset(sig, 0, sizeof(sig));

	/* AP gen rand */
	gen_rand(trnd, TRND_LEN);

	/* AP calculate SHA256 */
	fm1230_sha256(trnd, TRND_LEN, trnd_hash);

	/* SE calculate ECC signature */
	ret = do_ecc_signature(di, trnd_hash, sig, mode);
	if (ret) {
		hwlog_err("[%s] ecc_signature err : %d\n", __func__, ret);
		return ret;
	}

	ret = fm1230_ecdsa_verify(comb_table_qbuf, COMB_TABLE_QBUF_LEN, dev_cert + CERT_HEAD, SIG_LENGTH,
		trnd_hash, HASH_LENGTH, sig, SIG_LENGTH);
	hwlog_info("[%s] AP verify signature result: %d\n", __func__, ret);

	return ret;
}

int fm1230_do_authentication(struct fm1230_dev *di, struct power_genl_attr *key_res,
	enum key_cr *result, enum ecc_sign_mode mode)
{
	int ret;
	unsigned char cert[DEV_CERT_SIZE];
	unsigned char romid[READ_UID_SIZE];
	uint8_t dev_info[READ_UID_SIZE + DEV_CERT_SIZE];

	memset(cert, 0, sizeof(cert));
	memset(romid, 0, sizeof(romid));
	memset(dev_info, 0, sizeof(dev_info));

	ret = get_dev_info(di, key_res, dev_info);
	if (ret) {
		hwlog_err("[%s] get_dev_info err: %d\n", __func__, ret);
		*result = KEY_FAIL_UNMATCH;
		return -EAGAIN;
	}
	memcpy(romid, dev_info, READ_UID_SIZE);
	memcpy(cert, dev_info + READ_UID_SIZE, DEV_CERT_SIZE);

	ret = verify_device(di, cert, mode);
	if (ret) {
		hwlog_err("[%s] verify_device err: %d\n", __func__, ret);
		*result = KEY_FAIL_UNMATCH;
		return -EINVAL;
	}

	*result = KEY_PASS;
	return 0;
}

static int fm1230_read_file(struct fm1230_dev *di, int fid, unsigned char *buf, int size)
{
	u8 cmdbuf[BASE_CMD_LENGTH] = { 0, 0xb0, 0x80, 0, 0 }; /* read cmd */
	u8 rbuf[READ_RECV_LEN]; /* data + sw */
	u16 cmdlen = BASE_CMD_LENGTH;
	u16 rlen;

	if (!buf) {
		hwlog_err("[%s] : pointer NULL\n", __func__);
		return -EINVAL;
	}

	cmdbuf[CMD_FID] += fid;
	cmdbuf[CMD_SIZE] += size;

	if (ow_apdu_xfer(di, cmdbuf, cmdlen, rbuf, &rlen, CMD_READ_BIN_DELAY))
		return -EINVAL;
	memcpy(buf, rbuf, size);
	return 0;
}

static int fm1230_write_file(struct fm1230_dev *di, int fid, unsigned char *buf, int size)
{
	u8 cmdbuf[CMD_SIZE_WRITE] = { 0, 0xd6, 0x80, 0, 0 }; /* write cmd */
	u8 rbuf[RECV_LEN];
	u16 cmdlen = BASE_CMD_LENGTH;
	u16 rlen;
	int ret;

	if (!buf) {
		hwlog_err("[%s] : pointer NULL\n", __func__);
		return -EINVAL;
	}

	cmdbuf[CMD_FID] += fid;
	cmdbuf[CMD_SIZE] += size;

	memcpy(cmdbuf + cmdlen, buf, size);
	cmdlen += size;
	ret = ow_apdu_xfer(di, cmdbuf, cmdlen, rbuf, &rlen, CMD_WRITE_BIN_DELAY);
	if (ret)
		return -EAGAIN;
	return 0;
}

static int fm1230_lock_file(struct fm1230_dev *di, int fid)
{
	u8 cmdbuf[PAGE_CMD_LENGTH] = { 0x80, 0xe1, 0, 1, 2, 0, fid }; /* lock cmd */
	u8 rbuf[RECV_LEN];
	u16 cmdlen = PAGE_CMD_LENGTH;
	u16 rlen;

	return ow_apdu_xfer(di, cmdbuf, cmdlen, rbuf, &rlen, CMD_WRITE_BIN_DELAY);
}

static int fm1230_file_lock_sta(struct fm1230_dev *di, int fid, unsigned char *buf, int buf_len)
{
	u8 cmdbuf[PAGE_CMD_LENGTH] = { 0x80, 0xe1, 0, 0, 2, 0, fid }; /* check lock state cmd */
	u8 lock_res[LOCK_RES_LEN] = { 0x9F, 0x70, 1, 1 }; /* lock res mask */
	u8 rbuf[RECV_LEN];
	u16 cmdlen = PAGE_CMD_LENGTH;
	u16 rlen;
	u8 ret;

	if (!buf) {
		hwlog_err("[%s] : pointer NULL\n", __func__);
		return -EINVAL;
	}

	ret = ow_apdu_xfer(di, cmdbuf, cmdlen, rbuf, &rlen, CMD_READ_BIN_DELAY);
	if (ret != RESULT_SUCCESS)
		return ret;
	if (!memcmp(rbuf, lock_res, PAGE_STATE_LENGTH)) {
		snprintf(buf, buf_len, "locked\n");
		return 0;
	}
	snprintf(buf, buf_len, "unlocked\n");

	return 0;
}

int fm1230_operate_file(struct fm1230_dev *di, int file, int operation, unsigned char *buf, int buf_len)
{
	int ret;

	switch (operation) {
	case READ:
		ret = fm1230_read_file(di, file, buf, buf_len);
		break;
	case WRITE:
		ret = fm1230_write_file(di, file, buf, buf_len);
		break;
	case LOCK:
		ret = fm1230_lock_file(di, file);
		break;
	case LOCK_STATUS:
		ret = fm1230_file_lock_sta(di, file, buf, buf_len);
		break;
	default:
		hwlog_err("[%s] no operetion\n", __func__);
		return -EINVAL;
	}

	return ret;
}
