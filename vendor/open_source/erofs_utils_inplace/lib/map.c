// SPDX-License-Identifier: GPL-2.0+
/*
 * map.c
 *
 * Copyright (C) 2018 HUAWEI, Inc.
 *             http://www.huawei.com/
 * Created by Li Guifu <bluce.liguifu@huawei.com>
 */
#define _LARGEFILE64_SOURCE
#include "erofs/map.h"
#include <stdlib.h>
#include <sys/stat.h>
#include "erofs/io.h"

#define pr_fmt(fmt) "EROFS IO: " FUNC_LINE_FMT fmt "\n"
#include "erofs/print.h"

#define EROFS_BLOCK_MAP_FILE_POSTFIX    ".bin"
#define BUF_RANGE_SIZE (256)
/* *.map.bin used for diff at HOTA update
 * source block: data size + flags
 * if size is negative, it means this block is NOT a compressed block
 * if size is positive, it means this block is a compressed block
 * low 8bit data       |------|--|
 *     0 bit is inline file flags
 *     1 bit is compressed file flags
 *     2~3 bit is reserved
 *     4~7 bit is compress algorithm version
 */
#define MKFS_DIFF_SHIFT_8_BITS         8
#define MKFS_DIFF_MASK_8_BITS          0xFF
#define EROFS_FILE_INLINE_FLAGS_SHIFT  0
#define EROFS_FILE_INLINE_FLAGS_MASK   0x01
#define EROFS_COMPR_FILE_FLAGS_SHIFT   1
#define EROFS_COMPR_FILE_FLAGS_MASK    0x02
#define EROFS_COMPR_ALGOR_VER_SHIFT    4
#define EROFS_COMPR_ALGOR_VER_MASK     0xF0

/* diff multi blocks 150MB = 4KB 38400 */
#define EROFS_MULTI_BLKS_THRESHOLD     38400
#define EROFS_MULTI_BLKS_POSTFIX       "erofs_postfix"

static int erofs_devfd_bin = -1;
static FILE *c_blist_fp;

int map_open_binary(const char *devname)
{
	int fd = -1;

	fd = open(devname, O_WRONLY | O_CREAT | O_TRUNC,
		  S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
	if (fd < 0) {
		erofs_err("open [%s] failed!! fd:[%d]", devname, fd);
		return -errno;
	}

	erofs_devfd_bin = fd;
	return fd;
}

int dev_write_binary(void *buf, u64 offset, size_t len)
{
	if (erofs_devfd_bin < 0)
		return 0;

	ssize_t ret = pwrite64(erofs_devfd_bin, buf, len, (off64_t)offset);

	if (ret < 0 || ret != (ssize_t)len) {
		erofs_err("Failed to write data into device - [%llu, %ld] ret:%ld",
			  (long long unsigned)offset, len, ret);
		return -errno;
	}

	return 0;
}

void dev_close_binary(void)
{
	if (erofs_devfd_bin >= 0) {
		close(erofs_devfd_bin);
		erofs_devfd_bin = -1;
	}
}

/*
 *.map.bin used for diff at HOTA update
 * source block: data size + flags
 * if size is negative, it means this block is NOT a compressed block
 * if size is positive, it means this block is a compressed block
 * low 8bit data    |xxxx--xx|
 *     0 bit is inline file flags
 *     1 bit is file compressed flags
 *     2 bit is reserved
 *     3 bit is reserved
 *     4~7 bit is compress algorithm version, min is 1, max is 15
 *         if the entry is 0th entry corresponding to 0th block
 *     _size is the size of source data which compressed
 */
static s32 format_blk_entry_size(unsigned int fmode, u8 isuncompr, int _size)
{
	u32 size;

	if (isuncompr)
		size = (u32)(-_size); // NOT a compressed block
	else
		size = (u32)_size; // a compressed block

	size = size << MKFS_DIFF_SHIFT_8_BITS;
	size &= ~MKFS_DIFF_MASK_8_BITS;

	switch (fmode) {
	case EROFS_INODE_FLAT_INLINE:
		size |= 0x1 << EROFS_FILE_INLINE_FLAGS_SHIFT;
		break;
	case EROFS_INODE_FLAT_COMPRESSION:
	case EROFS_INODE_FLAT_COMPRESSION_LEGACY:
		size |= 0x1 << EROFS_COMPR_FILE_FLAGS_SHIFT;
		break;
	default:
		break;
	}

	return (s32)size;
}

/*
 * fmode: file compressed flags
 * dmode: means this block is a compressed block or not
 * addr: base address of these blocks
 * size: the source data size map to this block in the disk image
 */
int erofs_output_mapbin_block(u8 fmode, u8 isuncompr, u32 addr, u64 size)
{
	s32 fsize;
	int ret;

	erofs_info("block: fmode=%u isuncompr=%u addr=%u size=%llu",
		   fmode, isuncompr, addr, (long long)size);
	if (size == 0)
		return 0;

	fsize = format_blk_entry_size(fmode, isuncompr, size);
	fsize = cpu_to_le32(fsize);

	ret = dev_write_binary(&fsize, addr * sizeof(s32), sizeof(s32));
	if (ret)
		return ret;

	return 0;
}

/*
 * fmode: file compressed flags
 * dmode: means this block is a compressed block or not
 * base_blk: base address of these blocks
 * size: ranges of blocks sum size
 */
int erofs_output_mapbin_blocks(u8 fmode, u8 isuncompr, u32 base_blk, u64 size)
{
	int ret;
	s32 fsize;
	u64 blk_cnt = size / EROFS_BLKSIZ;
	u32 last_blk_size = size % EROFS_BLKSIZ;
	u32 base = base_blk;
	static char buf[EROFS_BLKSIZ];

	if (size == 0)
		return 0;

	erofs_info("blocks: fmode=%u isuncompr=%u base=%u size=%llu",
		   fmode, isuncompr, base_blk, (long long)size);
	while (blk_cnt > 0) {
		u32 i;
		u64 count;
		const u64 page_blks = EROFS_BLKSIZ / sizeof(s32);

		memset(buf, 0, EROFS_BLKSIZ);
		fsize = format_blk_entry_size(fmode, isuncompr, EROFS_BLKSIZ);
		fsize = cpu_to_le32(fsize);

		if (blk_cnt > page_blks) {
			count = page_blks;
			blk_cnt = blk_cnt - page_blks;
		} else {
			count = blk_cnt;
			blk_cnt = 0;
		}

		for (i = 0; i < count; i++)
			memcpy(buf + i * sizeof(s32), &fsize, sizeof(s32));

		ret = dev_write_binary(buf, base * sizeof(s32), count * sizeof(s32));
		if (ret)
			return ret;
		base += count;
	}

	if (fmode != EROFS_INODE_FLAT_INLINE && last_blk_size > 0) {
		ret = erofs_output_mapbin_block(fmode, isuncompr, base, last_blk_size);
		if (ret)
			return ret;
	}

	return 0;
}

/*
 * output block number in format
 * FILE* f  :fd that write block info into
 * char* file: file path
 * const char separator :separator between file and block number
 * u32 base_blkaddr :the start number of these blocks
 * u32 blks :the sum of these blocks
 * int serial : block group serial number
 *  < 0 (-1) it means this file dnot need divide blocks
 */
static int map_format_blocks(FILE *f, char *file, const char separator,
			     u32 base_blkaddr, u32 blks, u32 serial)
{
	int ret;
	u32 base = base_blkaddr;
	u32 blk_cnt = blks;
	char pbuf[PATH_MAX + BUF_RANGE_SIZE] = {0};

	if (serial > 0)
		ret = snprintf(pbuf, sizeof(pbuf), "%s_%s_%05u%c%u",
			       file, EROFS_MULTI_BLKS_POSTFIX,
			       serial, separator, base);

	else
		ret = snprintf(pbuf, sizeof(pbuf), "%s%c%u",
			       file, separator, base);

	if (ret < 0 || ret >= PATH_MAX) {
		erofs_err("snprintf errorly ret[%d]", ret);
		return -EFAULT;
	}

	if (blk_cnt > 1) {
		ret = snprintf(pbuf + ret, sizeof(pbuf) - ret,
			       "-%u", base + blk_cnt - 1);
		if (ret < 0 || ret >= PATH_MAX) {
			erofs_err("snprintf errorly ret[%d]", ret);
			return -EFAULT;
		}
	}

	ret = fprintf(f, "%s\n", pbuf);
	if (ret < 0) {
		erofs_err("fprintf write failed ret=%d", ret);
		return ret;
	}

	return 0;
}

int erofs_output_map_blocks(const char *file, u32 base, u32 blocks)
{
	char filepath[PATH_MAX + 1] = { 0 };
	const char separator = ' ';
	const char *pfile = file;
	u32 serial = 0;
	u32 blk_cnt = blocks;
	u32 base_blkaddr = base;
	u32 cnt;

	if (!c_blist_fp)
		return 0;

	pfile = pfile + strlen(cfg.c_src_path) + 1; // 1 means skip /
	int ret = snprintf(filepath, PATH_MAX, "%s/%s", cfg.mount_point, pfile);

	if (ret < 0 || ret >= PATH_MAX) {
		erofs_err("snprintf errorly ret[%d]", ret);
		return -EINVAL;
	}

	do {
		if (blk_cnt > cfg.c_multi_blks_thr) {
			cnt = cfg.c_multi_blks_thr;
			blk_cnt -= cfg.c_multi_blks_thr;
		} else {
			cnt = blk_cnt;
			blk_cnt = 0;
		}

		map_format_blocks(c_blist_fp, filepath,
				  separator, base_blkaddr, cnt, serial);
		base_blkaddr += cnt;
		serial++;
	} while (blk_cnt > 0);

	return 0;
}

/*
 * this function will save compress algorithm version into the 0th entry
 * in the *.map.bin file, blk_addr should be 0
 * this function should be call after map_open_binary()
 *
 * if map.bin has been created, we need save compress
 * algorithm version number into the 0th entry of map.bin
 * the 0th block is used to store super block and shared xattr
 */
int erofs_map_save_compr_algo_ver(unsigned int version, unsigned int blk_addr)
{
	s32 data;
	u64 addr = blknr_to_addr(blk_addr) * sizeof(s32);

	erofs_info("version=%d blk_addr=%" PRIu64 "", version, addr);
	data = version << EROFS_COMPR_ALGOR_VER_SHIFT;
	data &= EROFS_COMPR_ALGOR_VER_MASK;
	data = cpu_to_le32(data);
	dev_write_binary(&data, addr, sizeof(data));

	return 0;
}

int erofs_map_file_init(const char *file)
{
	int ret;
	char map_bin[PATH_MAX + 1] = {0};

	if (strlen(file) >= (PATH_MAX + strlen(EROFS_BLOCK_MAP_FILE_POSTFIX))) {
		erofs_err("map file[%s] is too long", file);
		return -EINVAL;
	}

	ret = snprintf(map_bin, sizeof(map_bin), "%s%s",
		       file, EROFS_BLOCK_MAP_FILE_POSTFIX);
	if (ret < 0 || ret >= PATH_MAX) {
		erofs_err("snprintf errorly ret[%d]", ret);
		return -EINVAL;
	}

	/* for map.bin file */
	ret = map_open_binary(map_bin);
	if (ret < 0)
		return ret;

	/* for map file */
	FILE *filep = fopen(file, "w");

	if (!filep) {
		erofs_err("failed to open block_list_file: %s\n", strerror(errno));
		dev_close_binary();
		return -EINVAL;
	}

	c_blist_fp = filep;
	return 0;
}

void erofs_map_file_close(void)
{
	if (c_blist_fp) {
		fclose(c_blist_fp);
		c_blist_fp = NULL;
	}

	dev_close_binary();
}

