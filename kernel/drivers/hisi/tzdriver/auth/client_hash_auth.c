/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 * Description: function for CA code hash auth
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
#include "client_hash_auth.h"
#include <linux/string.h>
#include <linux/mutex.h>
#include <linux/types.h>
#include <linux/rwsem.h>
#ifdef CONFIG_AUTH_SUPPORT_UNAME
#include <linux/fs.h>
#endif
#ifdef CONFIG_CLIENT_AUTH
#include <linux/mm.h>
#include <linux/dcache.h>
#include <linux/mm_types.h>
#include <linux/highmem.h>
#include <linux/cred.h>
#include <linux/slab.h>
#include <linux/sched/mm.h>
#endif
#ifdef CONFIG_AUTH_HASH
#include <crypto/hash.h>
#endif
#include <securec.h>

#include "tc_ns_log.h"
#include "auth_base_impl.h"

#ifdef CONFIG_AUTH_HASH
#define SHA256_DIGEST_LENGTH 32
#define FIXED_PKG_NAME_LENGTH 256
struct sdesc_hash {
	struct shash_desc shash;
	char ctx[];
};
#endif

#if defined(CONFIG_ANDROID_HIDL) || defined(CONFIG_MDC_HAL_AUTH)

static int check_proc_state(bool is_hidl, struct task_struct **hidl_struct,
	const struct tc_ns_client_context *context)
{
	bool check_value = false;

	if (is_hidl) {
		rcu_read_lock();
		*hidl_struct = pid_task(find_vpid(context->calling_pid),
			PIDTYPE_PID);
		check_value = !*hidl_struct ||
			(*hidl_struct)->state == TASK_DEAD;
		if (check_value) {
			tloge("task is dead\n");
			rcu_read_unlock();
			return -EFAULT;
		}

		get_task_struct(*hidl_struct);
		rcu_read_unlock();
		return EOK;
	}

	return EOK;
}

static int get_hidl_client_task(bool is_hidl_task, struct tc_ns_client_context *context,
	struct task_struct **cur_struct)
{
	int ret;
	struct task_struct *hidl_struct = NULL;

	ret = check_proc_state(is_hidl_task, &hidl_struct, context);
	if (ret)
		return ret;

	if (hidl_struct)
		*cur_struct = hidl_struct;
	else
		*cur_struct = current;

	return EOK;
}

#endif

#ifdef CONFIG_CLIENT_AUTH
#define LIBTEEC_CODE_PAGE_SIZE 8
#define DEFAULT_TEXT_OFF 0
#define LIBTEEC_NAME_MAX_LEN 50
const char g_libso[KIND_OF_SO][LIBTEEC_NAME_MAX_LEN] = {
						"libteec_vendor.so",
#ifndef CONFIG_CMS_CAHASH_AUTH
#ifndef CONFIG_CADAEMON_AUTH
						"libteec.huawei.so",
#else
						"libteec.so",
#endif
#endif
};

static int find_lib_code_area(struct mm_struct *mm,
	struct vm_area_struct **lib_code_area, int so_index)
{
	struct vm_area_struct *vma = NULL;
	bool is_valid_vma = false;
	bool is_so_exists = false;
	bool param_check = (!mm || !mm->mmap ||
		!lib_code_area || so_index >= KIND_OF_SO);

	if (param_check) {
		tloge("illegal input params\n");
		return -EFAULT;
	}
	for (vma = mm->mmap; vma; vma = vma->vm_next) {
		is_valid_vma = (vma->vm_file &&
			vma->vm_file->f_path.dentry &&
			vma->vm_file->f_path.dentry->d_name.name);
		if (is_valid_vma) {
			is_so_exists = !strcmp(g_libso[so_index],
				vma->vm_file->f_path.dentry->d_name.name);
			if (is_so_exists && (vma->vm_flags & VM_EXEC)) {
				*lib_code_area = vma;
				tlogd("so name is %s\n",
					vma->vm_file->f_path.dentry->d_name.name);
				return EOK;
			}
		}
	}
	return -EFAULT;
}

struct get_code_info {
	unsigned long code_start;
	unsigned long code_end;
	unsigned long code_size;
};
static int update_so_hash(struct mm_struct *mm,
	struct task_struct *cur_struct, struct shash_desc *shash, int so_index)
{
	struct vm_area_struct *vma = NULL;
	int rc = -EFAULT;
	struct get_code_info code_info;
	unsigned long in_size;
	struct page *ptr_page = NULL;
	void *ptr_base = NULL;

	if (find_lib_code_area(mm, &vma, so_index)) {
		tlogd("get lib code vma area failed\n");
		return -EFAULT;
	}

	code_info.code_start = vma->vm_start;
	code_info.code_end = vma->vm_end;
	code_info.code_size = code_info.code_end - code_info.code_start;

	while (code_info.code_start < code_info.code_end) {
		// Get a handle of the page we want to read
#if (KERNEL_VERSION(5, 10, 0) <= LINUX_VERSION_CODE)
		rc = get_user_pages_remote(mm, code_info.code_start,
			1, FOLL_FORCE, &ptr_page, NULL, NULL);
#else
		rc = get_user_pages_remote(cur_struct, mm, code_info.code_start,
			1, FOLL_FORCE, &ptr_page, NULL, NULL);
#endif
		if (rc != 1) {
			tloge("get user pages locked error[0x%x]\n", rc);
			rc = -EFAULT;
			break;
		}

		ptr_base = kmap_atomic(ptr_page);
		if (!ptr_base) {
			rc = -EFAULT;
			put_page(ptr_page);
			break;
		}
		in_size = (code_info.code_size > PAGE_SIZE) ? PAGE_SIZE : code_info.code_size;

		rc = crypto_shash_update(shash, ptr_base, in_size);
		if (rc) {
			kunmap_atomic(ptr_base);
			put_page(ptr_page);
			break;
		}
		kunmap_atomic(ptr_base);
		put_page(ptr_page);
		code_info.code_start += in_size;
		code_info.code_size = code_info.code_end - code_info.code_start;
	}
	return rc;
}

/* Calculate the SHA256 library digest */
static int calc_task_so_hash(unsigned char *digest, uint32_t dig_len,
	struct task_struct *cur_struct, int so_index)
{
	struct mm_struct *mm = NULL;
	int rc;
	size_t size;
	size_t shash_size;
	struct sdesc *desc = NULL;

	if (!digest || dig_len != SHA256_DIGEST_LENTH) {
		tloge("tee hash: digest is NULL\n");
		return -EFAULT;
	}

	shash_size = crypto_shash_descsize(get_shash_handle());
	size = sizeof(desc->shash) + shash_size;
	if (size < sizeof(desc->shash) || size < shash_size) {
		tloge("size overflow\n");
		return -ENOMEM;
	}

	desc = kzalloc(size, GFP_KERNEL);
	if (ZERO_OR_NULL_PTR((unsigned long)(uintptr_t)desc)) {
		tloge("alloc desc failed\n");
		return -ENOMEM;
	}

	desc->shash.tfm = get_shash_handle();
	if (crypto_shash_init(&desc->shash)) {
		kfree(desc);
		return -EFAULT;
	}

	mm = get_task_mm(cur_struct);
	if (!mm) {
		tloge("so does not have mm struct\n");
		if (memset_s(digest, MAX_SHA_256_SZ, 0, dig_len))
			tloge("memset digest failed\n");
		kfree(desc);
		return -EFAULT;
	}

	down_read(&mm_sem_lock(mm));
	rc = update_so_hash(mm, cur_struct, &desc->shash, so_index);
	up_read(&mm_sem_lock(mm));
	mmput(mm);
	if (!rc)
		rc = crypto_shash_final(&desc->shash, digest);

	kfree(desc);
	return rc;
}

static int proc_calc_hash(uint8_t kernel_api, struct tc_ns_session *session,
	struct task_struct *cur_struct, uint32_t pub_key_len)
{
	int rc, i;
	int so_found = 0;

	mutex_crypto_hash_lock();
	if (kernel_api == TEE_REQ_FROM_USER_MODE) {
		for (i = 0; so_found < NUM_OF_SO && i < KIND_OF_SO; i++) {
			rc = calc_task_so_hash(session->auth_hash_buf + MAX_SHA_256_SZ * so_found,
				(uint32_t)SHA256_DIGEST_LENTH, cur_struct, i);
			if (!rc)
				so_found++;
		}
		if (so_found != NUM_OF_SO)
			tlogd("so library found: %d\n", so_found);
	} else {
		tlogd("request from kernel\n");
	}

#ifdef CONFIG_ASAN_DEBUG
	tloge("so auth disabled for ASAN debug\n");
	uint32_t so_hash_len = MAX_SHA_256_SZ * NUM_OF_SO;
	errno_t sret = memset_s(session->auth_hash_buf, so_hash_len, 0, so_hash_len);
	if (sret) {
		mutex_crypto_hash_unlock();
		tloge("memset so hash failed\n");
		return -EFAULT;
	}
#endif

	rc = calc_task_hash(session->auth_hash_buf + MAX_SHA_256_SZ * NUM_OF_SO,
		(uint32_t)SHA256_DIGEST_LENTH, cur_struct, pub_key_len);
	if (rc) {
		mutex_crypto_hash_unlock();
		tloge("tee calc ca hash failed\n");
		return -EFAULT;
	}
	mutex_crypto_hash_unlock();
	return EOK;
}

int calc_client_auth_hash(struct tc_ns_dev_file *dev_file,
	struct tc_ns_client_context *context, struct tc_ns_session *session)
{
	int ret;
	struct task_struct *cur_struct = NULL;
	bool check = false;
#if defined(CONFIG_ANDROID_HIDL) || defined(CONFIG_MDC_HAL_AUTH)
	bool is_hidl_srvc = false;
#endif
	check = (!dev_file || !context || !session);
	if (check) {
		tloge("bad params\n");
		return -EFAULT;
	}

	if (tee_init_shash_handle("sha256")) {
		tloge("init code hash error\n");
		return -EFAULT;
	}

#if defined(CONFIG_ANDROID_HIDL) || defined(CONFIG_MDC_HAL_AUTH)
	if (!current->mm) {
		tlogd("kernel thread need not check\n");
		ret = ENTER_BYPASS_CHANNEL;
	} else {
#ifdef CONFIG_CADAEMON_AUTH
		/* for OH */
		ret = check_cadaemon_auth();
#else
		/* for HO and MDC\DC */
		ret = check_hidl_auth();
#endif
	}
	if (ret != CHECK_ACCESS_SUCC) {
		if (ret != ENTER_BYPASS_CHANNEL) {
			tloge("hidl service may be exploited ret 0x%x\n", ret);
			return -EACCES;
		}
		/* native\kernel ca take this branch */
	} else {
		/* android hidl\mdc secmgr(libteec\kms) take this branch */
		is_hidl_srvc = true;
	}
	ret = get_hidl_client_task(is_hidl_srvc, context, &cur_struct);
	if (ret)
		return -EFAULT;
#else
	cur_struct = current;
#endif

	ret = proc_calc_hash(dev_file->kernel_api, session, cur_struct, dev_file->pub_key_len);
#if defined(CONFIG_ANDROID_HIDL) || defined(CONFIG_MDC_HAL_AUTH)
	if (is_hidl_srvc)
		put_task_struct(cur_struct);
#endif
	return ret;
}
#endif

#ifdef CONFIG_AUTH_HASH
#define UID_LEN 16
static int construct_hashdata(struct tc_ns_dev_file *dev_file,
	uint8_t *buf, uint32_t buf_len)
{
	int ret;
	ret = memcpy_s(buf, buf_len, dev_file->pkg_name, dev_file->pkg_name_len);
	if (ret) {
		tloge("memcpy_s failed\n");
		goto error;
	}
	buf += dev_file->pkg_name_len;
	buf_len -= dev_file->pkg_name_len;
	ret = memcpy_s(buf, buf_len, dev_file->pub_key, dev_file->pub_key_len);
	if (ret) {
		tloge("memcpy_s failed\n");
		goto error;
	}
	return 0;
error:
	return -EFAULT;
}

static struct sdesc_hash *init_sdesc(struct crypto_shash *alg)
{
	struct sdesc_hash *sdesc;
	size_t size;

	size = sizeof(struct shash_desc) + crypto_shash_descsize(alg);
	sdesc = kmalloc(size, GFP_KERNEL);
	if (sdesc == NULL)
		return ERR_PTR(-ENOMEM);
	sdesc->shash.tfm = alg;
	return sdesc;
}

static int calc_hash(struct crypto_shash *alg,
	const unsigned char *data, unsigned int datalen, unsigned char *digest)
{
	struct sdesc_hash *sdesc;
	int ret;

	sdesc = init_sdesc(alg);
	if (IS_ERR(sdesc)) {
		pr_info("can't alloc sdesc\n");
		return PTR_ERR(sdesc);
	}

	ret = crypto_shash_digest(&sdesc->shash, data, datalen, digest);
	kfree(sdesc);
	return ret;
}

static int do_sha256(const unsigned char *data, uint32_t datalen,
	unsigned char *out_digest, uint8_t digest_len)
{
	int ret;
	struct crypto_shash *alg;
	const char *hash_alg_name = "sha256";
	if (digest_len != SHA256_DIGEST_LENGTH) {
		tloge("error digest_len\n");
		return -1;
	}

	alg = crypto_alloc_shash(hash_alg_name, 0, 0);
	if(IS_ERR_OR_NULL(alg)) {
		tloge("can't alloc alg %s, PTR_ERR alg is %ld\n", hash_alg_name, PTR_ERR(alg));
		return PTR_ERR(alg);
	}
	ret = calc_hash(alg, data, datalen, out_digest);
	if (ret != 0) {
		tloge("calc hash failed\n");
		crypto_free_shash(alg);
		alg = NULL;
		return -1;
	}
	crypto_free_shash(alg);
	alg = NULL;
	return 0;
}

int set_login_information_hash(struct tc_ns_dev_file *hash_dev_file)
{
	int ret = 0;
	uint8_t *indata = NULL;
	if (hash_dev_file == NULL) {
		tloge("wrong caller info, cal hash stopped\n");
		return -1;
	}
	mutex_lock(&hash_dev_file->cainfo_hash_setup_lock);

	if (!(hash_dev_file->cainfo_hash_setup)) {
		unsigned char digest[SHA256_DIGEST_LENGTH] = {0};
		uint8_t digest_len = sizeof(digest);

		uint32_t indata_len;
#ifdef CONFIG_AUTH_SUPPORT_UNAME
		/* username using fixed length to cal hash */
		if (hash_dev_file->pub_key_len >= FIXED_PKG_NAME_LENGTH) {
			tloge("username is too loog\n");
			ret = -1;
			goto error;
		}
		indata_len = hash_dev_file->pkg_name_len + FIXED_PKG_NAME_LENGTH;
#else
		indata_len = hash_dev_file->pkg_name_len + hash_dev_file->pub_key_len;
#endif
		indata = kzalloc(indata_len, GFP_KERNEL);
		if (ZERO_OR_NULL_PTR((unsigned long)(uintptr_t)indata)) {
			tloge("indata kmalloc fail\n");
			ret = -1;
			goto error;
		}

		ret = construct_hashdata(hash_dev_file, indata, indata_len);
		if (ret != 0) {
			tloge("construct hashdata failed\n");
			goto error;
		}

		ret = do_sha256((unsigned char *)indata, indata_len, digest, digest_len);
		if (ret != 0) {
			tloge("do sha256 failed\n");
			goto error;
		}

		ret = memcpy_s(hash_dev_file->pkg_name, MAX_PACKAGE_NAME_LEN, digest, digest_len);
		if (ret != 0) {
			tloge("memcpy_s failed\n");
			goto error;
		}
		hash_dev_file->pkg_name_len = SHA256_DIGEST_LENGTH;
		hash_dev_file->cainfo_hash_setup = true;
	}

error:
	if (!ZERO_OR_NULL_PTR((unsigned long)(uintptr_t)indata))
		kfree(indata);

	mutex_unlock(&hash_dev_file->cainfo_hash_setup_lock);
	return ret;
}
#endif

#ifdef CONFIG_AUTH_SUPPORT_UNAME
#define PASSWD_FILE "/etc/passwd"
#define UID_POS     2U
#define DECIMAL 10
static int uid_compare(uint32_t uid, const char* uid_str, uint32_t uid_len)
{
	uint32_t uid_num = 0;
	for (uint32_t i = 0; i < uid_len; i++) {
		bool is_number = uid_str[i] >= '0' && uid_str[i] <= '9';
		if (!is_number) {
			tloge("passwd info wrong format: uid missing\n");
			return -1;
		}
		uid_num = DECIMAL * uid_num + (uid_str[i] - '0');
	}
	return (uid_num == uid) ? 0 : -1;
}

/* "username:[encrypted password]:uid:gid:[comments]:home directory:login shell" */
static uint32_t parse_uname(uint32_t uid, char *username, int buffer_len)
{
	char *str = username;
	char *token = strsep(&str, ":");
	char *temp_name = token; // first tokon is username, need to check uid
	int index = 0;
	while(token != NULL && index < UID_POS) {
		token = strsep(&str, ":");
		index++;
	}
	if (token == NULL)
		return -1;
	if (uid_compare(uid, token, strlen(token)) != 0)
		return -1;
	if (strcpy_s(username, buffer_len, temp_name) != EOK)
		return -1;
	return strlen(temp_name);
}
static int read_line(char *buf, int buf_len, struct file *fp, loff_t *offset)
{
	if (offset == NULL) {
		tloge("offset is null while read file\n");
		return -1;
	}
	ssize_t ret = kernel_read(fp, buf, buf_len, offset);
	if (ret < 0)
		return -1;
	ssize_t i = 0;
	/* read buf_len, need to find first '\n' */
	while(i < ret) {
		if (i >= buf_len)
			break;
		if (buf[i] == '\n')
			break;
		i++;
	}
	if (i < ret)
		*offset -= (loff_t)(ret - i);
	if (i < buf_len)
		buf[i] = '\0';
	return 0;
}

/* get username by uid,
* on linux, user info is stored in system file "/etc/passwd",
* each line represents a user, fields are separated by ':',
* formatted as such: "username:[encrypted password]:uid:gid:[comments]:home directory:login shell"
*/
int tc_ns_get_uname(uint32_t uid, char *username, int buffer_len, uint32_t *out_len)
{
	if (username == NULL || out_len == NULL || buffer_len != FIXED_PKG_NAME_LENGTH) {
		tloge("params is null\n");
		return -1;
	}
	struct file *f = NULL;
	loff_t offset = 0;
	f = filp_open(PASSWD_FILE, O_RDONLY, 0);
	if (IS_ERR(f)) {
		tloge("kernel open passwd file failed\n");
		return -1;
	}
	while (read_line(username, buffer_len, f, &offset) == 0) {
		uint32_t ret = parse_uname(uid, username, buffer_len);
		if (ret >= 0) {
			*out_len = ret;
			filp_close(f, NULL);
			return 0;
		}
	}
	filp_close(f, NULL);
	return -1;
}
#endif
