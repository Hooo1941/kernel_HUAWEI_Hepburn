/**
 * f2fs_format_utils.c
 *
 * Copyright (c) 2014 Samsung Electronics Co., Ltd.
 *             http://www.samsung.com/
 *
 * Dual licensed under the GPL or LGPL version 2 licenses.
 */
#ifndef _LARGEFILE_SOURCE
#define _LARGEFILE_SOURCE
#endif
#ifndef _LARGEFILE64_SOURCE
#define _LARGEFILE64_SOURCE
#endif
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#ifndef _FILE_OFFSET_BITS
#define _FILE_OFFSET_BITS 64
#endif

#include <f2fs_fs.h>

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#ifndef ANDROID_WINDOWS_HOST
#include <sys/ioctl.h>
#endif
#include <sys/stat.h>
#include <fcntl.h>

#ifdef HAVE_LINUX_FS_H
#include <linux/fs.h>
#endif
#ifdef HAVE_LINUX_FALLOC_H
#include <linux/falloc.h>
#endif

#ifndef BLKDISCARD
#define BLKDISCARD	_IO(0x12,119)
#endif
#ifndef BLKSECDISCARD
#define BLKSECDISCARD	_IO(0x12,125)
#endif

static int trim_device(int i, u_int64_t start, u_int64_t end)
{
#ifndef ANDROID_WINDOWS_HOST
	unsigned long long range[2];
	struct stat *stat_buf;
	struct device_info *dev = c.devices + i;
	u_int64_t bytes = dev->total_sectors * dev->sector_size;
	int fd = dev->fd;

	if (dev->reserved)
		return 0;

	stat_buf = malloc(sizeof(struct stat));
	if (stat_buf == NULL) {
		MSG(1, "\tError: Malloc Failed for trim_stat_buf!!!\n");
		return -1;
	}

	if (fstat(fd, stat_buf) < 0 ) {
		MSG(1, "\tError: Failed to get the device stat!!!\n");
		free(stat_buf);
		return -1;
	}

	range[0] = start;
	range[1] = (end == 0) ? bytes : end;

#if defined(WITH_BLKDISCARD) && defined(BLKDISCARD)
	MSG(0, "Info: [%s] Discarding device\n", dev->path);
	if (S_ISREG(stat_buf->st_mode)) {
#if defined(HAVE_FALLOCATE) && defined(FALLOC_FL_PUNCH_HOLE)
		if (fallocate(fd, FALLOC_FL_PUNCH_HOLE | FALLOC_FL_KEEP_SIZE,
				range[0], range[1]) < 0) {
			MSG(0, "Info: fallocate(PUNCH_HOLE|KEEP_SIZE) is failed\n");
		}
#endif
		free(stat_buf);
		return 0;
	} else if (S_ISBLK(stat_buf->st_mode)) {
		if (dev->zoned_model != F2FS_ZONED_NONE) {
			free(stat_buf);
			return f2fs_reset_zones(i);
		}
#ifdef BLKSECDISCARD
		if (ioctl(fd, BLKSECDISCARD, &range) < 0) {
			MSG(0, "Info: This device doesn't support BLKSECDISCARD\n");
		} else {
			MSG(0, "Info: Secure Discarded %lu MB\n",
					(unsigned long)stat_buf->st_size >> 20);
			free(stat_buf);
			return 0;
		}
#endif
		if (ioctl(fd, BLKDISCARD, &range) < 0) {
			MSG(0, "Info: This device doesn't support BLKDISCARD\n");
		} else {
			MSG(0, "Info: Discarded %llu MB\n", range[1] >> 20);
		}
	} else {
		free(stat_buf);
		return -1;
	}
#endif
	free(stat_buf);
#endif
	return 0;
}

int f2fs_trim_devices(void)
{
	int i;

	for (i = 0; i < c.ndevs; i++)
		if (trim_device(i, 0, 0))
			return -1;
	c.trimmed = 1;
	return 0;
}

int f2fs_trim_range_devices(u_int64_t blk_start, u_int64_t blk_end)
{
	int dev_start, dev_end;
	u_int64_t start_bytes, end_bytes;
	int i;

	dev_start = get_device_index(blk_start);
	dev_end = get_device_index(blk_end);

	if (blk_start > blk_end || dev_start == -1 || dev_end == -1) {
		MSG(0, "Error: wrong trim devices dev %d ~ %d,blkaddr range[%lu, %lu)\n",
			dev_start, dev_end, blk_start, blk_end);
		return -1;
	}

	start_bytes = (blk_start - c.devices[dev_start].start_blkaddr) << F2FS_BLKSIZE_BITS;
	end_bytes = (blk_end - c.devices[dev_end].start_blkaddr) << F2FS_BLKSIZE_BITS;

	if (dev_start == dev_end) {
		if (end_bytes && trim_device(dev_start, start_bytes, end_bytes))
			return -1;
	} else {
		if (trim_device(dev_start++, start_bytes, 0))
			return -1;

		for (i = dev_start; i < dev_end; i++)
			if (trim_device(i, 0, 0))
				return -1;
		if (end_bytes && trim_device(dev_end, 0, end_bytes))
			return -1;
	}

	c.trimmed = 1;
	MSG(0, "Info: trim devices dev %d ~ %d,blkaddr range[%lu, %lu)\n",
			dev_start, dev_end, blk_start, blk_end);

	return 0;
}
