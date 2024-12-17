/*
 * cloud_network_signature.c
 *
 * cloud network module implementation
 *
 * Copyright (c) 2023-2023 Huawei Technologies Co., Ltd.
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

#include "cloud_network_signature.h"
#include <linux/crypto.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <crypto/hash.h>
#include <linux/scatterlist.h>
#include <linux/timekeeping.h>
#include "cloud_network_utils.h"

static struct signature_param sign_params[HMAC_ID_NUMS];
static __u8 tx_encrypt_id;

int sign_hon_skb(char* data, int data_len, char* digest)
{
	if (unlikely(sign_params[tx_encrypt_id].tfm == NULL || IS_ERR(sign_params[tx_encrypt_id].tfm)))
		return ERROR;

	(void)memset_s(digest, BUFFER_SHA256, 0, BUFFER_SHA256);
	if (crypto_shash_tfm_digest(sign_params[tx_encrypt_id].tfm, data, data_len, digest)) {
		log_err("crypto shash tfm digest failed, error");
		return ERROR;
	}
	return SUCCESS;
}

bool check_sign_hon_skb(char *data, unsigned int data_len, __u8 hmac_id, struct hmachdr *hmach)
{
	__u8 digest[BUFFER_SHA256];

	if (unlikely(data == NULL || data_len < 0 || hmach == NULL)) {
		log_err("param is invalid, error");
		return false;
	}

	if (sign_params[hmac_id].tfm == NULL) {
		log_info("hmac id no match hmac_key, error");
		return false;
	}

	(void)memset_s(digest, sizeof(digest), 0, sizeof(digest));
	if (crypto_shash_tfm_digest(sign_params[hmac_id].tfm, data, data_len,
		digest)) {
		log_err("crypto shash tfm digest hmac calculated failed, error");
		return false;
	}
	if (memcmp(hmach->hmac, digest, HMAC_SIZE) == 0)
		return true;

	log_err("sign check failed, error");
	return false;
}

int cloud_network_set_key(char* key, int key_len, __u8 hmac_id)
{
	log_info("set key hmac id %u", hmac_id);
	if (key == NULL) {
		log_err("param is invalid");
		return ERROR;
	}
	if (sign_params[hmac_id].tfm != NULL) {
		log_err("key has been set");
		return ERROR;
	}

	sign_params[hmac_id].tfm = crypto_alloc_shash("hmac(sha256)", 0, 0);
	if (unlikely(IS_ERR(sign_params[hmac_id].tfm))) {
		log_err("crypto alloc shash failed, error");
		return ERROR;
	}
	if (crypto_shash_setkey(sign_params[hmac_id].tfm, key, key_len)) {
		log_err("crypto hash setkey failed, error");
		crypto_free_shash(sign_params[hmac_id].tfm);
		sign_params[hmac_id].tfm = NULL;
		return ERROR;
	}
	tx_encrypt_id = hmac_id;
	return SUCCESS;
}

void cloud_network_sign_init()
{
	log_info("cloud network sign init");
	for (int i = 0; i < HMAC_ID_NUMS; i++)
		sign_params[i].tfm = NULL;

	return;
}

void cloud_network_sign_deinit()
{
	log_info("cloud network sign deinit");
	for (int i = 0; i < HMAC_ID_NUMS; i++) {
		if (sign_params[i].tfm != NULL)
			crypto_free_shash(sign_params[i].tfm);
		sign_params[i].tfm = NULL;
	}
	return;
}