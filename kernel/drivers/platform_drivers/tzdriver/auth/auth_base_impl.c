/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 * Description: function for base hash operation
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
#include "auth_base_impl.h"
#include <linux/string.h>
#include <linux/mutex.h>
#include <linux/types.h>
#include <linux/rwsem.h>
#include <linux/path.h>
#include <linux/file.h>
#include <linux/fs.h>

#include <linux/mm.h>
#include <linux/dcache.h>
#include <linux/mm_types.h>
#include <linux/highmem.h>
#include <linux/cred.h>
#include <linux/slab.h>
#if (KERNEL_VERSION(4, 14, 0) <= LINUX_VERSION_CODE)
#include <linux/sched/mm.h>
#endif
#if defined(CONFIG_SELINUX_AUTH_ENBALE) && defined(CONFIG_SECURITY_SELINUX)
#include <linux/security.h>
#endif
#include <securec.h>
#include "tc_ns_log.h"
#include "tc_ns_client.h"
#include "agent.h" /* for get_proc_dpath */
#include "ko_adapt.h"

/* for crypto */
struct crypto_shash *g_shash_handle;
bool g_shash_handle_state = false;
struct mutex g_shash_handle_lock;

void init_crypto_hash_lock(void)
{
	mutex_init(&g_shash_handle_lock);
}

void mutex_crypto_hash_lock(void)
{
	mutex_lock(&g_shash_handle_lock);
}

void mutex_crypto_hash_unlock(void)
{
	mutex_unlock(&g_shash_handle_lock);
}

/* begin: prepare crypto context */
struct crypto_shash *get_shash_handle(void)
{
	return g_shash_handle;
}

void free_shash_handle(void)
{
	if (g_shash_handle) {
		crypto_free_shash(g_shash_handle);
		g_shash_handle_state = false;
		g_shash_handle = NULL;
	}
}

int tee_init_shash_handle(char *hash_type)
{
	long rc;

	if (!hash_type) {
		tloge("tee init crypto: error input parameter\n");
		return -EFAULT;
	}

	mutex_crypto_hash_lock();
	if (g_shash_handle_state) {
		mutex_crypto_hash_unlock();
		return 0;
	}

	g_shash_handle = crypto_alloc_shash(hash_type, 0, 0);
	if (IS_ERR_OR_NULL(g_shash_handle)) {
		rc = PTR_ERR(g_shash_handle);
		tloge("Can not allocate %s reason: %ld\n", hash_type, rc);
		mutex_crypto_hash_unlock();
		return rc;
	}
	g_shash_handle_state = true;

	mutex_crypto_hash_unlock();
	return 0;
}
/* end: prepare crypto context */

/* begin: Calculate the SHA256 file digest */
static int prepare_desc(struct sdesc **desc)
{
	size_t size;
	size_t shash_size;

	shash_size = crypto_shash_descsize(g_shash_handle);
	size = sizeof((*desc)->shash) + shash_size;
	if (size < sizeof((*desc)->shash) || size < shash_size) {
		tloge("size flow\n");
		return -ENOMEM;
	}

	*desc = kzalloc(size, GFP_KERNEL);
	if (ZERO_OR_NULL_PTR((unsigned long)(uintptr_t)(*desc))) {
		tloge("alloc desc failed\n");
		return -ENOMEM;
	}

	return EOK;
}

#define PINED_PAGE_NUMBER 1
static int get_proc_user_pages(struct mm_struct *mm, unsigned long start_code,
	struct page **ptr_page, struct task_struct *cur_struct)
{
#if (KERNEL_VERSION(5, 10, 0) <= LINUX_VERSION_CODE)
	(void)cur_struct;
	return get_user_pages_remote(mm, start_code,
		(unsigned long)PINED_PAGE_NUMBER, FOLL_FORCE, ptr_page, NULL, NULL);
#elif (KERNEL_VERSION(4, 9, 0) <= LINUX_VERSION_CODE)
	return get_user_pages_remote(cur_struct, mm, start_code,
		(unsigned long)PINED_PAGE_NUMBER, FOLL_FORCE, ptr_page, NULL, NULL);
#elif (KERNEL_VERSION(4, 4, 197) == LINUX_VERSION_CODE)
	return get_user_pages_locked(cur_struct, mm, start_code,
		(unsigned long)PINED_PAGE_NUMBER, FOLL_FORCE, ptr_page, NULL);
#else
	return get_user_pages_locked(cur_struct, mm, start_code,
		(unsigned long)PINED_PAGE_NUMBER, 0, 1, ptr_page, NULL);
#endif
}

static int update_task_hash(struct mm_struct *mm,
	struct task_struct *cur_struct, struct shash_desc *shash)
{
	int rc = -1;
	unsigned long in_size;
	struct page *ptr_page = NULL;
	void *ptr_base = NULL;

	unsigned long start_code = mm->start_code;
	unsigned long end_code = mm->end_code;
	unsigned long code_size = end_code - start_code;
	if (code_size == 0) {
		tloge("bad code size\n");
		return -EINVAL;
	}

	while (start_code < end_code) {
		/* Get a handle of the page we want to read */
		rc = get_proc_user_pages(mm, start_code, &ptr_page, cur_struct);
		if (rc != PINED_PAGE_NUMBER) {
			tloge("get user pages error[0x%x]\n", rc);
			rc = -EFAULT;
			break;
		}

		ptr_base = kmap_atomic(ptr_page);
		if (!ptr_base) {
			rc = -EFAULT;
			put_page(ptr_page);
			break;
		}

		in_size = (code_size > PAGE_SIZE) ? PAGE_SIZE : code_size;
		rc = crypto_shash_update(shash, ptr_base, in_size);
		if (rc) {
			kunmap_atomic(ptr_base);
			put_page(ptr_page);
			break;
		}

		kunmap_atomic(ptr_base);
		put_page(ptr_page);
		start_code += in_size;
		code_size = end_code - start_code;
	}
	return rc;
}

int calc_task_hash(unsigned char *digest, uint32_t dig_len,
	struct task_struct *cur_struct, uint32_t pub_key_len)
{
	struct mm_struct *mm = NULL;
	struct sdesc *desc = NULL;
	bool check_value = false;
	int rc;

	check_value = (!cur_struct || !digest ||
		dig_len != SHA256_DIGEST_LENTH);
	if (check_value) {
		tloge("tee hash: input param is error\n");
		return -EFAULT;
	}

	mm = get_task_mm(cur_struct);
	if (!mm) {
		if (memset_s(digest, dig_len, 0, MAX_SHA_256_SZ))
			return -EFAULT;
		tloge("kernel proc need not check\n");
		return EOK;
	}

	if (pub_key_len != sizeof(uint32_t)) {
		tloge("apk need not check\n");
		mmput(mm);
		return EOK;
	}

	if (prepare_desc(&desc) != EOK) {
		mmput(mm);
		tloge("prepare desc failed\n");
		return -ENOMEM;
	}

	desc->shash.tfm = g_shash_handle;
	if (crypto_shash_init(&desc->shash)) {
		tloge("shash init failed\n");
		rc = -ENOMEM;
		goto free_res;
	}

	down_read(&mm_sem_lock(mm));
	if (update_task_hash(mm, cur_struct, &desc->shash)) {
		up_read(&mm_sem_lock(mm));
		rc = -ENOMEM;
		goto free_res;
	}
	up_read(&mm_sem_lock(mm));

	rc = crypto_shash_final(&desc->shash, digest);
free_res:
	mmput(mm);
	kfree(desc);
	return rc;
}
/* end: Calculate the SHA256 file digest */

#if defined(CONFIG_SELINUX_AUTH_ENBALE) && defined(CONFIG_SECURITY_SELINUX)
static int check_proc_selinux_access(const char *s_ctx)
{
	if (s_ctx == NULL) {
		tloge("bad params\n");
		return CHECK_ACCESS_FAIL;
	}

	int rc;
	u32 sid;
	u32 tid;
	u32 s_ctx_len = strnlen(s_ctx, MAX_SCTX_LEN);
	if (s_ctx_len == 0 || s_ctx_len >= MAX_SCTX_LEN) {
		tloge("invalid selinux ctx\n");
		return CHECK_ACCESS_FAIL;
	}

	security_task_getsecid(current, &sid);
	rc = security_secctx_to_secid(s_ctx, s_ctx_len, &tid);
	if (rc != 0) {
		tloge("secctx to sid failed, rc %d", rc);
		return CHECK_ACCESS_FAIL;
	}
	if (sid != tid) {
		tloge("check selinux label failed\n");
		return CHECK_ACCESS_FAIL;
	}

	return EOK;
}
#else
static int check_proc_selinux_access(const char *s_ctx)
{
	(void)s_ctx;
	return 0;
}
#endif

static int get_proc_uid(uid_t *proc_uid)
{
	const struct cred *cred = NULL;
	get_task_struct(current);
	cred = koadpt_get_task_cred(current);
	if (cred == NULL) {
		tloge("cred is NULL\n");
		put_task_struct(current);
		return CHECK_ACCESS_FAIL;
	}

	*proc_uid = cred->uid.val;
	put_cred(cred);
	put_task_struct(current);
	return CHECK_ACCESS_SUCC;
}

static int check_proc_uid_path(const char *auth_ctx)
{
	int ret = 0;
	char str_path_uid[MAX_PATH_SIZE] = { 0 };
	char *pro_dpath = NULL;
	char *k_path = NULL;
	u32 auth_ctx_len;
	uid_t proc_uid;

	if (auth_ctx == NULL) {
		tloge("bad params\n");
		return CHECK_ACCESS_FAIL;
	}

	auth_ctx_len = (u32)strnlen(auth_ctx, MAX_PATH_SIZE);
	if (auth_ctx_len == 0 || auth_ctx_len >= MAX_PATH_SIZE) {
		tloge("invalid uid path\n");
		return CHECK_ACCESS_FAIL;
	}

	k_path = kmalloc(MAX_PATH_SIZE, GFP_KERNEL);
	if (ZERO_OR_NULL_PTR((unsigned long)(uintptr_t)k_path)) {
		tloge("path kmalloc fail\n");
		return CHECK_ACCESS_FAIL;
	}

	pro_dpath = get_proc_dpath(k_path, MAX_PATH_SIZE);
	if (IS_ERR_OR_NULL(pro_dpath)) {
		kfree(k_path);
		return CHECK_ACCESS_FAIL;
	}

	ret = get_proc_uid(&proc_uid);
	if (ret != CHECK_ACCESS_SUCC) {
		tloge("get proc uid faile\n");
		goto clean;
	}

	if (snprintf_s(str_path_uid, MAX_PATH_SIZE, MAX_PATH_SIZE - 1, "%s:%u",
		pro_dpath, (unsigned int)proc_uid) < 0) {
		tloge("snprintf_s path uid failed, ret %d\n", ret);
		ret = CHECK_ACCESS_FAIL;
		goto clean;
	}

	if (strnlen(str_path_uid, MAX_PATH_SIZE) != auth_ctx_len || strncmp(str_path_uid, auth_ctx, auth_ctx_len) != 0)
		ret = ENTER_BYPASS_CHANNEL;
	else
		ret = CHECK_ACCESS_SUCC;
clean:
	kfree(k_path);
	return ret;
}


int check_hidl_auth(void)
{
	int ret = check_proc_uid_path(CA_HIDL_PATH_UID_AUTH_CTX);
	int ret_rec = check_proc_uid_path(CA_HIDL_REC_PATH_UID_AUTH_CTX);
	if (ret != CHECK_ACCESS_SUCC && ret_rec != CHECK_ACCESS_SUCC)
		return ret;

#if defined(CONFIG_SELINUX_AUTH_ENBALE) && defined(CONFIG_SECURITY_SELINUX)
	ret = check_proc_selinux_access(SELINUX_CA_HIDL_LABEL);
	if (ret != EOK) {
		tloge("check hidl selinux label failed, ret %d\n", ret);
		return CHECK_SECLABEL_FAIL;
	}
#endif

	return CHECK_ACCESS_SUCC;
}

#ifdef CONFIG_TEECD_AUTH
int check_teecd_auth(void)
{
	int ret = check_proc_uid_path(TEECD_PATH_UID_AUTH_CTX);
	if (ret != 0) {
		tloge("check teecd path failed, ret %d\n", ret);
		return ret;
	}

#if defined(CONFIG_SELINUX_AUTH_ENBALE) && defined(CONFIG_SECURITY_SELINUX)
	ret = check_proc_selinux_access(SELINUX_TEECD_LABEL);
	if (ret != 0) {
		tloge("check teecd selinux label failed!, ret %d\n", ret);
		return -EACCES;
	}
#endif

	return CHECK_ACCESS_SUCC;
}
#endif
