/*
 * cloud_network_signature.h
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

#ifndef _CLOUD_NETWORK_SIGNATURE_H_
#define _CLOUD_NETWORK_SIGNATURE_H_

#include <linux/types.h>
#include "cloud_network_honhdr.h"

#define BUFFER_SHA256 32
#define HMAC_ID_NUMS 256

struct signature_param {
	struct crypto_shash *tfm;
};

int sign_hon_skb(char* data, int data_len, char* digest);
bool check_sign_hon_skb(char *data, unsigned int data_len, __u8 hmac_id, struct hmachdr *hmach);
int cloud_network_set_key(char* key, int key_len, __u8 hamc_id);
void cloud_network_sign_init(void);
void cloud_network_sign_deinit(void);
#endif /* _CLOUD_NETWORK_SIGNATURE_H_ */