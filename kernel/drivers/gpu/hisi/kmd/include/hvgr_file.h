/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#ifndef HVGR_FILE_H
#define HVGR_FILE_H

#include "linux/types.h"
#include <linux/fs.h>

#define MAX_FILE_NAME_SIZE    256u

/* BYTE2MB: xxx >> HVGR_FILE_BYTE2MB */
#define HVGR_FILE_BYTE2MB     20

int hvgr_file_delete_file(const char *file);

unsigned long long hvgr_file_get_free_space(const char *dir);

int hvgr_file_get_file_num(const char *dir, const char *file_prefix);

bool hvgr_file_make_dir(const char *dir, umode_t mode);

bool hvgr_file_write_to_file(struct file *data_file, char *buffer, size_t buf_len, loff_t *pos);

#endif
