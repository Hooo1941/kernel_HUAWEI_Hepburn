/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 * Description: function definition for base hash operation
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */
#ifndef AUTH_BASE_IMPL_H
#define AUTH_BASE_IMPL_H

#ifndef SELINUX_CA_HIDL_LABEL
#define SELINUX_CA_HIDL_LABEL ""
#endif

#ifndef SELINUX_TEECD_LABEL
#define SELINUX_TEECD_LABEL ""
#endif

#ifndef CA_HIDL_PATH_UID_AUTH_CTX
#define CA_HIDL_PATH_UID_AUTH_CTX ""
#endif

#ifndef TEECD_PATH_UID_AUTH_CTX
#define TEECD_PATH_UID_AUTH_CTX ""
#endif

#ifndef CADAEMON_PATH_UID_AUTH_CTX
#define CADAEMON_PATH_UID_AUTH_CTX ""
#endif

#if ((defined CONFIG_CLIENT_AUTH) || (defined CONFIG_TEECD_AUTH))
#include <linux/version.h>
#if (KERNEL_VERSION(4, 14, 0) <= LINUX_VERSION_CODE)
#include <linux/sched/task.h>
#endif
#include <linux/err.h>
#include <crypto/hash.h>

#define CHECK_ACCESS_SUCC      0
#define CHECK_ACCESS_FAIL      0xffff
#define CHECK_PATH_HASH_FAIL   0xff01
#define CHECK_SECLABEL_FAIL    0xff02
#define CHECK_CODE_HASH_FAIL   0xff03
#define ENTER_BYPASS_CHANNEL   0xff04

#define BUF_MAX_SIZE           1024
#define MAX_PATH_SIZE          512
#define SHA256_DIGEST_LENTH    32
#define MAX_SCTX_LEN           128

struct sdesc {
	struct shash_desc shash;
	char ctx[];
};

int calc_path_hash(bool is_hidl_srvc, unsigned char *digest, unsigned int dig_len);
int calc_task_hash(unsigned char *digest, uint32_t dig_len,
	struct task_struct *cur_struct, uint32_t pub_key_len);

int tee_init_shash_handle(char *hash_type);
void free_shash_handle(void);
struct crypto_shash *get_shash_handle(void);

void init_crypto_hash_lock(void);
void mutex_crypto_hash_lock(void);
void mutex_crypto_hash_unlock(void);
int check_hidl_auth(void);
int check_teecd_auth(void);
#else

static inline void free_shash_handle(void)
{
	return;
}

static void init_crypto_hash_lock(void)
{
	return;
}

static inline int check_teecd_auth(void)
{
	return 0;
}

#endif /* CLIENT_AUTH || TEECD_AUTH */

#ifdef CONFIG_CADAEMON_AUTH
int check_cadaemon_auth(void);
#endif
#endif
