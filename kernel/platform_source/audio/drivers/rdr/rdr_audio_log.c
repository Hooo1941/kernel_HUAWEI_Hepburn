/*
 * rdr_audio_log.c
 *
 * audio rdr log
 *
 * Copyright (c) 2023-2023 Huawei Technologies CO., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include "rdr_audio_log.h"

#include <linux/uaccess.h>
#include <linux/fcntl.h>
#include <linux/fs.h>
#include <linux/syscalls.h>
#include <linux/errno.h>
#ifdef CONFIG_VENDOR_FS
#include <platform_include/basicplatform/linux/fs/vendor_fs_interface.h>
#endif
#include "audio_log.h"
#include "rdr_inner.h"
#include "rdr_audio_adapter.h"

#define LOG_TAG "rdr_audio_log"

static int create_dir(const char *path)
{
	long ret = -1;

	WARN_ON(path == NULL);
#ifdef CONFIG_VENDOR_FS
	ret = vendor_access(path, 0);
#else
	ret = ksys_access(path, 0);
#endif
	if (ret != 0) {
		AUDIO_LOGI("need create dir %s", path);
#ifdef CONFIG_VENDOR_FS
		ret = vendor_mkdir(path, 0770);
#else
		ret = ksys_mkdir(path, 0770);
#endif
		if (ret < 0) {
			AUDIO_LOGE("create dir %s failed, ret: %ld", path, ret);
			return -EBADF;
		}
		AUDIO_LOGI("create dir %s successed", path);
	}

	/* change dir limit root-system */
	if (bbox_chown(path, ROOT_UID, SYSTEM_GID, false))
		AUDIO_LOGE("chown %s dir failed", path);
	return 0;
}

static int rdr_audio_create_dir(const char *path)
{
	char cur_path[RDR_FNAME_LEN];
	int index = 0;

	memset(cur_path, 0, RDR_FNAME_LEN);
	if (*path != '/')
		return -ENOTDIR;

	cur_path[index++] = *path++;

	while (*path != '\0') {
		if (*path == '/') {
			if (create_dir(cur_path))
				return -ENOENT;
		}
		cur_path[index] = *path;
		path++;
		index++;
	}
	return 0;
}

static int rdr_audio_loopwrite_open(const char *name, struct file **file)
{
	int ret = 0;

	if (file == NULL) {
		AUDIO_LOGE("loopwrite open failed, param is null");
		return -EINVAL;
	}
	ret = rdr_audio_create_dir(name);
	if (ret != 0) {
		AUDIO_LOGE("create dir fail, name: %s, ret: %d", name, ret);
		return ret;
	}

	*file = filp_open(name, O_CREAT | O_RDWR | O_APPEND, 0660);
	if (IS_ERR_OR_NULL(*file)) {
		AUDIO_LOGE("open failed, name: %s", name);
		return -EBADF;
	}
	return ret;
}

static int rdr_audio_loopwrite_append(struct file *file, const void *address,
	unsigned int length)
{
	long bytes;

	WARN_ON(address == NULL);
	WARN_ON(length == 0);

	if (IS_ERR_OR_NULL(file)) {
		AUDIO_LOGE("loopwrite append failed, param is null");
		return -EINVAL;
	}

	bytes = vfs_write(file, address, (size_t)length, &file->f_pos);
	if (bytes != length) {
		AUDIO_LOGE("write data failed, bytes: %ld", bytes);
		return -EINVAL;
	}
	return 0;
}

static void rdr_audio_loopwrite_close(struct file *file)
{
	long ret;

	if (IS_ERR_OR_NULL(file)) {
		AUDIO_LOGE("loopwrite close failed, param is invalid");
		return;
	}

	ksys_sync();

	ret = filp_close(file, NULL);
	if (ret)
		AUDIO_LOGE("close file failed, ret: %ld", ret);
	return;
}

int rdr_audio_write_file(const char *name, const char *data, unsigned int size)
{
	int ret = 0;
	mm_segment_t oldfs;
	struct file *file = NULL;

	if (!name || !data) {
		AUDIO_LOGE("para is invalid");
		return -EINVAL;
	}

	WARN_ON(size == 0);

	oldfs = get_fs();
	set_fs(KERNEL_DS);
	ret = rdr_audio_loopwrite_open(name, &file);
	if (ret != 0) {
		AUDIO_LOGE("open %s fail", name);
		set_fs(oldfs);
		return ret;
	}

	ret = rdr_audio_loopwrite_append(file, data, size);
	if (ret != 0)
		AUDIO_LOGE("write %s fail", name);

	if (bbox_chown(name, ROOT_UID, SYSTEM_GID, false))
		AUDIO_LOGE("chown %s file failed", name);

	rdr_audio_loopwrite_close(file);

	set_fs(oldfs);
	return ret;
}