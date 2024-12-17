// SPDX-License-Identifier: GPL-2.0
/*
 * file_mgr.c
 *
 * file manager source file for file transfer
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
#include "file_mgr.h"

#include <linux/fs.h>
#include <linux/file.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/stat.h>
#include <linux/uaccess.h>
#include <securec.h>
#include <huawei_platform/log/hw_log.h>
#include "ext_sensorhub_channel.h"

#define HWLOG_TAG file_trans
HWLOG_REGIST();
#define MAX_FILE_TANSFER_LEN (5 * 1024 * 1024) /* support max file data: 5MB */

struct transfer_file_ops {
	int (*pre_process)(struct transfer_file *file);
	int (*data_process)(struct transfer_file *file, unsigned char *data, u32 len);
};

static int transfer_file_read_prev(struct transfer_file *file)
{
	int ret;

	if (!file)
		return -EINVAL;

	file->filp = filp_open(file->filepath, O_RDONLY, 0);
	if (IS_ERR(file->filp)) {
		ret = PTR_ERR(file->filp);
		hwlog_err("%s file:%s not found[ret:%d]", __func__, file->filepath, ret);
		return -EFAULT;
	}

	ret = vfs_llseek(file->filp, file->offset, SEEK_SET);
	if (ret < 0) {
		hwlog_err("lseek %s failed from origin", file->filepath);
		filp_close(file->filp, NULL);
		return -EFAULT;
	}
	file->state = PRE_PROCESSED;
	return 0;
}

static int transfer_file_read(struct transfer_file *file, unsigned char *data, u32 len)
{
	int num;
	mm_segment_t old_fs;

	if (!file || !data || len == 0)
		return -EINVAL;

	/* the read len longer than left, set to left size */
	if (len > file->offset + file->len - file->c_offset)
		len = file->offset + file->len - file->c_offset;

	old_fs = get_fs();
	set_fs(KERNEL_DS);
	num = vfs_read(file->filp, data, len, &file->filp->f_pos);
	set_fs(old_fs);
	/* need to check file end */
	if (num <= 0) {
		hwlog_err("%s read file error, num: %d", __func__, num);
		file->state = END;
		return -EFAULT;
	}
	file->c_offset += num;
	file->dst_offset += num;
	if (file->c_offset >= file->offset + file->len) {
		hwlog_info("read end offset:%u len:%u", file->c_offset, file->len);
		file->state = END;
	}

	return num;
}

static int transfer_file_write_prev(struct transfer_file *file)
{
	int ret;

	if (!file)
		return -EINVAL;
	/* open with file mode 0666 */
	file->filp = filp_open(file->filepath, O_CREAT | O_WRONLY, 0666);
	if (IS_ERR(file->filp)) {
		ret = PTR_ERR(file->filp);
		hwlog_err("%s file:%s not found[ret:%d]", __func__, file->filepath, ret);
		return -EFAULT;
	}

	ret = vfs_llseek(file->filp, file->offset, SEEK_SET);
	if (ret < 0) {
		hwlog_err("lseek %s failed from origin", file->filepath);
		filp_close(file->filp, NULL);
		return -EFAULT;
	}
	file->state = PRE_PROCESSED;
	return 0;
}

static int transfer_file_write(struct transfer_file *file, unsigned char *data, u32 len)
{
	int num;
	mm_segment_t old_fs;

	if (!file)
		return -EINVAL;

	old_fs = get_fs();
	set_fs(KERNEL_DS);
	num = vfs_write(file->filp, data, len, &file->filp->f_pos);
	if (num != len) {
		set_fs(old_fs);
		hwlog_err("%s error, num: %d, aim:%u", __func__, num, len);
		file->state = END;
		return -EFAULT;
	}
	vfs_fsync(file->filp, 0);
	set_fs(old_fs);
	file->c_offset += num;
	if (file->c_offset >= file->offset + file->len) {
		hwlog_info("write end offset:%u len:%u", file->c_offset, file->len);
		file->state = END;
	}

	return num;
}

static int transfer_buffer_read_prev(struct transfer_file *file)
{
	u32 len = 0;
	unsigned char *buffer = get_read_map_mem(&len);
	u32 header_len = sizeof(u8) + sizeof(u32);

	if (!buffer || len < header_len)
		return -EFAULT;

	if (!file)
		return -EINVAL;

	/* check len */
	if (file->len > len - header_len - file->offset)
		return -EINVAL;

	file->filp = NULL;
	file->state = PRE_PROCESSED;
	return 0;
}

static unsigned char *mem_read_buffer(void)
{
	u32 len = 0;
	unsigned char *buffer = get_read_map_mem(&len);
	u32 header_len = sizeof(u8) + sizeof(u32);

	if (!buffer || len < header_len)
		return NULL;

	return buffer + header_len;
}

static int transfer_buffer_read(struct transfer_file *file, unsigned char *data, u32 len)
{
	int ret;
	unsigned char *read_buffer = NULL;

	if (!file || !data || len == 0)
		return -EINVAL;

	read_buffer = mem_read_buffer();
	if (!read_buffer)
		return -EFAULT;

	/* the read len longer than left, set to left size */
	if (len > file->offset + file->len - file->c_offset)
		len = file->offset + file->len - file->c_offset;

	/* buffer is valid when trigger transfer, do not check here */
	ret = memcpy_s(data, len, read_buffer + file->c_offset, len);
	if (ret != EOK) {
		hwlog_err("%s read buffer error, ret: %d", __func__, ret);
		file->state = END;
		return -EFAULT;
	}
	file->c_offset += len;
	file->dst_offset += len;
	if (file->c_offset >= file->offset + file->len) {
		hwlog_info("read buffer end offset:%u len:%u", file->c_offset, file->len);
		file->state = END;
	}

	return len;
}

static int transfer_buffer_write_prev(struct transfer_file *file)
{
	u32 len = 0;
	u8 *type = NULL;
	u32 *data_len = NULL;
	u32 header_len = sizeof(u8) + sizeof(u32);
	unsigned char *buffer = get_write_map_mem(&len);

	if (!buffer || len < header_len)
		return -EFAULT;

	if (!file)
		return -EINVAL;

	/* check len */
	if (file->len > len - header_len - file->offset)
		return -EINVAL;
	/* set header(u8 type + u32 data length) */
	type = buffer;
	*type = 0;
	data_len = (u32 *)&buffer[sizeof(*type)];
	*data_len = file->len;

	file->filp = NULL;
	file->state = PRE_PROCESSED;
	return 0;
}

static unsigned char *mem_write_buffer(void)
{
	u32 len = 0;
	unsigned char *buffer = get_write_map_mem(&len);
	u32 header_len = sizeof(u8) + sizeof(u32);

	if (!buffer || len < header_len)
		return NULL;

	return buffer + header_len;
}

static int transfer_buffer_write(struct transfer_file *file, unsigned char *data, u32 len)
{
	int ret;
	unsigned char *write_buffer = NULL;

	if (!file)
		return -EINVAL;

	write_buffer = mem_write_buffer();
	if (!write_buffer)
		return -EFAULT;

	/* buffer is valid when trigger transfer, do not check here */
	ret = memcpy_s(write_buffer + file->c_offset, len, data, len);
	if (ret != EOK) {
		hwlog_err("%s write buffer error, ret: %d", __func__, ret);
		file->state = END;
		return -EFAULT;
	}

	file->c_offset += len;
	if (file->c_offset >= file->offset + file->len) {
		hwlog_info("write buffer end offset:%u len:%u", file->c_offset, file->len);
		file->state = END;
	}

	return len;
}

static size_t stat_file_len(const char *filepath)
{
	struct kstat stat;
	int ret;

	if (!filepath)
		return 0;

	ret = memset_s(&stat, sizeof(stat), 0, sizeof(stat));
	if (ret != EOK)
		hwlog_warn("%s memset error", __func__);

	vfs_stat(filepath, &stat);

	hwlog_info("file:%s stat size=%u\n", filepath, stat.size);
	return stat.size;
}

static struct transfer_file_ops file_ops[] = {
	{
		transfer_file_read_prev,
		transfer_file_read,
	},
	{
		transfer_file_write_prev,
		transfer_file_write,
	},
	{
		transfer_buffer_read_prev,
		transfer_buffer_read,
	},
	{
		transfer_buffer_write_prev,
		transfer_buffer_write,
	},
};

struct transfer_file *initial_transfer_file(const char *filepath, u32 offset,
					    u32 len, enum transfer_ops opt)
{
	int ret;
	char *path = NULL;
	struct transfer_file *file = NULL;
	u32 file_len = stat_file_len(filepath);

	hwlog_info("file:%s, file len:%u offset:%u, len:%u\n", filepath, file_len, offset, len);
	if (len > MAX_FILE_TANSFER_LEN)
		return NULL;

	file = kzalloc(sizeof(*file), GFP_KERNEL);
	if (!file)
		return NULL;

	path = kzalloc(strlen(filepath) + 1, GFP_KERNEL);
	if (!path)
		goto err;

	ret = strcpy_s(path, strlen(filepath) + 1, filepath);
	if (ret != EOK)
		goto err;

	/* initial */
	file->offset = offset;
	file->c_offset = offset;
	file->len = len;
	file->opt = opt;
	file->filepath = path;
	if (opt == TRANSFER_READ) {
		/* if read file and len is 0 means transfer to the file end */
		if (len == 0)
			len = file_len - offset;
		if (len > MAX_FILE_TANSFER_LEN)
			goto err;
		file->len = len;
	}
	if (opt < ARRAY_SIZE(file_ops)) {
		file->pre_process = file_ops[opt].pre_process;
		file->data_process = file_ops[opt].data_process;
	}
	file->state = INITIAL;

	hwlog_info("file:%s, file len:%u offset:%u, len:%u\n",
		   filepath, file_len, offset, file->len);
	return file;
err:
	kfree(file);
	kfree(path);
	return NULL;
}

void release_transfer_file(struct transfer_file *file)
{
	if (!file)
		return;

	if (file->filp && !IS_ERR(file->filp))
		filp_close(file->filp, NULL);

	kfree(file->filepath);
	file->filepath = NULL;
	kfree(file);
	file = NULL;
}
