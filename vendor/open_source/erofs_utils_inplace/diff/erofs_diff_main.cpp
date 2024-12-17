/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2019. All rights reserved.
 * file       : erofs_diff_main.cpp
 * Author     : Li Guifu <bluce.liguifu@huawei.com>
 * Create     : 2019/7/02
 * Description: compress tool used at HOTA update
 */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#define _LARGEFILE64_SOURCE
#include <assert.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <linux/kdev_t.h>
#include <limits.h>
#include <pthread.h>
#include <stdarg.h>
#include <fec/io.h>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <openssl/sha.h>
#include <android-base/logging.h>
#include <android-base/parseint.h>
#include <android-base/strings.h>
#include <android-base/unique_fd.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "erofs/config.h"
#include "erofs/print.h"
#include "erofs/cache.h"
#include "erofs/inode.h"
#include "erofs/io.h"
#include "erofs/compress.h"
#include "erofs/map.h"
#include "erofs/patch.h"
#include "lib/compressor.h"

#ifdef __cplusplus
}
#endif

#include "hwrecovery_common_utility.h"
#include "rss_range.h"

#ifndef O_BINARY
#define O_BINARY 0
#endif

/*
 * erofs_diff_compress** should not be exchanged !!
 * and it must equals to LOCAL_MODULE := erofs_diff_compress
 * at Android.mk
 */
#define EROFS_UTILS_COMPRESS "erofs_diff_compress"
#define EROFS_UTILS_DUMP     "erofs_diff_dump"
#define EROFS_EXIT_CODE(X)   (-(X))

extern struct erofs_compress compresshandle;
extern int compressionlevel;

static const char *file_dump_out = NULL;


int erofs_diff_write_data_to_disk(const void *buf, u64 offset, size_t len,
				  void *token, int argc, char **argv)
{
	int ret;

	if (!file_dump_out)
		return DevFunc(buf, offset, len, token, argc, argv);

	erofs_info("offset=%" PRIu64 " len=%zd", offset, len);
	ret = dev_write(buf, offset, len);
	if (ret) {
		erofs_err("dev_write failed ret=%d", ret);
		return ret;
	}

	return len;
}

int erofs_diff_local_finish(void *token)
{
	if (!file_dump_out)
		return ErofsDiffFinish(token);

	return 0;
}

static int erofs_diff_config_init(void)
{
	int ret;

	erofs_init_configure();
	ret = z_erofs_compress_init();
	if (ret) {
		erofs_err("z_erofs_compress_init() failed ret=%d", ret);
		return ret;
	}

	return 0;
}

static int diff_write_uncompr_blk(struct z_erofs_vle_compress_ctx *ctx,
				  unsigned int *len, char *dst, void *token,
				  int argc, char **argv)
{
	int ret;
	unsigned int count;

	if (sbi.requirements & EROFS_REQUIREMENT_LZ4_0PADDING) {
		/* use shift format */
		count = min(EROFS_BLKSIZ, *len);

		memcpy(dst, ctx->queue + ctx->head, count);
	} else {
		/* fix up clusterofs to 0 if possable */
		if (ctx->head >= ctx->clusterofs) {
			ctx->head -= ctx->clusterofs;
			*len += ctx->clusterofs;
			ctx->clusterofs = 0;
		}

		/* write uncompressed data */
		count = min(EROFS_BLKSIZ, *len);

		memcpy(dst, ctx->queue + ctx->head, count);
	}
	memset(dst + count, 0, EROFS_BLKSIZ - count);

	erofs_dbg("Writing %u uncompressed data to block %u",
		  count, ctx->blkaddr);

	ret = erofs_diff_write_data_to_disk(dst,
					    blknr_to_addr(ctx->blkaddr),
					    EROFS_BLKSIZ, token, argc, argv);
	if (ret < 0 || ret != EROFS_BLKSIZ) {
		erofs_err("devfunc failed count=%d blkaddr=%" PRIu64 " ret=%d",
			  count, blknr_to_addr(ctx->blkaddr), ret);
		return -EIO;
	}

	return count;
}

static int diff_write_compr_blk(const void *buf, erofs_blk_t blkaddr,
				u32 nblocks, void *token,
				int argc, char **argv)
{
	int ret;
	size_t len = blknr_to_addr(nblocks);

	ret = erofs_diff_write_data_to_disk(buf,
					    blknr_to_addr(blkaddr), len,
					    token, argc, argv);
	if (ret < 0 || ret != (int)len) {
		erofs_err("devfunc failed len=%u blkaddr=%" PRIu64 " ret=%d",
			  (u32)len, blknr_to_addr(blkaddr), ret);
		return -EIO;
	}

	return 0;
}

static int diff_vle_compress_one(struct erofs_inode *inode,
			    struct z_erofs_vle_compress_ctx *ctx,
			    bool final, void *token, int argc, char **argv)
{
	struct erofs_compress *const h = &compresshandle;
	unsigned int len = ctx->tail - ctx->head;
	unsigned int count;
	int ret;
	static char dstbuf[EROFS_BLKSIZ * 2];
	char *const dst = dstbuf + EROFS_BLKSIZ;

	while (len) {
		bool raw;

		if (len <= EROFS_BLKSIZ) {
			if (final)
				goto nocompression;
			break;
		}

		count = len;
		ret = erofs_compress_destsize(h, compressionlevel,
					      ctx->queue + ctx->head,
					      &count, dst, EROFS_BLKSIZ);
		if (ret <= 0) {
			if (ret != -EAGAIN) {
				erofs_err("failed to compress %s: %s",
					  inode->i_srcpath,
					  erofs_strerror(ret));
				return ret;
			}
nocompression:
			ret = diff_write_uncompr_blk(ctx, &len, dst, token, argc, argv);
			if (ret < 0)
				return ret;
			count = ret;
			raw = true;
		} else {
			/* write compressed data */
			erofs_dbg("Writing %u compressed data to block %u",
				  count, ctx->blkaddr);

			if (sbi.requirements & EROFS_REQUIREMENT_LZ4_0PADDING) {
				ret = diff_write_compr_blk(dst - (EROFS_BLKSIZ - ret),
							   ctx->blkaddr, 1, token, argc, argv);
			} else {
				ret = diff_write_compr_blk(dst, ctx->blkaddr, 1, token, argc, argv);
			}

			if (ret)
				return ret;
			raw = false;
		}

		ctx->head += count;
		/* write compression indexes for this blkaddr */

		++ctx->blkaddr;
		len -= count;

		if (!final && ctx->head >= EROFS_CONFIG_COMPR_MAX_SZ) {
			const uint qh_aligned = round_down(ctx->head, EROFS_BLKSIZ);
			const uint qh_after = ctx->head - qh_aligned;

			memmove(ctx->queue, ctx->queue + qh_aligned,
				len + qh_after);
			ctx->head = qh_after;
			ctx->tail = qh_after + len;
			break;
		}
	}
	return 0;
}

ssize_t erofs_diff_write(const char *file, void *token, int argc, char **argv)
{
	struct z_erofs_vle_compress_ctx ctx;
	erofs_off_t remaining;
	erofs_blk_t blkaddr = 0;
	int fd = -1;
	int ret;
	struct erofs_inode *inode = NULL;

	inode = __erofs_create_new_inode(file, true);
	if (IS_ERR(inode)) {
		erofs_err("init inode failed file=%s", file);
		return -ENOENT;
	}

	fd = open(inode->i_srcpath, O_RDONLY | O_BINARY);
	if (fd < 0) {
		erofs_err("%s:%s", inode->i_srcpath, strerror(errno));
		ret = -errno;
		goto err_inode;
	}

	ctx.blkaddr = blkaddr;
	ctx.metacur = NULL;
	ctx.head = ctx.tail = 0;
	ctx.clusterofs = 0;
	remaining = inode->i_size;

	while (remaining) {
		const uint readcount = min_t(uint, remaining,
					     sizeof(ctx.queue) - ctx.tail);

		ret = read(fd, ctx.queue + ctx.tail, readcount);
		if (ret < 0 || (uint)ret != readcount) {
			ret = -errno;
			goto err_close;
		}
		remaining -= readcount;
		ctx.tail += readcount;

		/* do one compress round */
		ret = diff_vle_compress_one(inode, &ctx, false, token, argc, argv);
		if (ret)
			goto err_close;
	}

	/* do the final round */
	ret = diff_vle_compress_one(inode, &ctx, true, token, argc, argv);
	if (ret)
		goto err_close;

	inode->nblocks = ctx.blkaddr - blkaddr;
	inode->startaddr = blkaddr;

	erofs_info("compressed %s (%lu bytes) into %u blocks, %u-->%u",
		   inode->i_srcpath, inode->i_size, ctx.blkaddr - blkaddr, blkaddr, ctx.blkaddr);

	close(fd);
	erofs_iput(inode);
	return 0;

err_close:
	close(fd);
err_inode:
	erofs_iput(inode);
	return ret;
}

static void usage(void)
{
	erofs_show_config();
	erofs_dump("usage:\n");
	erofs_dump("Erofs utils only elf name permited:\n");
	erofs_dump("\t[%s]\nor\n\t[%s]\n", EROFS_UTILS_COMPRESS, EROFS_UTILS_DUMP);
	erofs_dump("For diff update compress:\n");
	erofs_dump("\t  %s <srcfile> <...>\n", EROFS_UTILS_COMPRESS);
	erofs_dump("For diff  dump  compress:\n");
	erofs_dump("\t  %s <srcfile> <outfile> <dbglvl>\n", EROFS_UTILS_DUMP);

	exit(EINVAL);
}
/*
 *  notes:
 *  fork exist code is greater than zero
 *  must do check
 *  normal argc should be greater than 3,
 *  if it is 2, that means dump compress data to file
 */
int erofs_diff_compress(int argc, char **argv)
{
	int ret;
	void *token = NULL;
	const char *file = NULL;
	char realPath[PATH_MAX + 1] = { 0x0 };

	file = argv[1];
	if (strlen(file) > PATH_MAX || realpath(file, realPath) == NULL) {
		erofs_err("file path check failed [%s][%s]",
		       file, strerror(errno));
		return -EINVAL;
	}

	token = ErofsDiffInit(argc, argv);
	if (!token) {
		erofs_err("token is NULL !!");
		return -EFAULT;
	}

	ret = erofs_diff_write(realPath, token, argc, argv);
	if (ret < 0) {
		erofs_err("do erofs_diff_write failed ret=%d", ret);
		return ret;
	}
	ret = erofs_diff_local_finish(token);
	if (ret < 0) {
		erofs_err("do erofs_diff_write failed ret=%d", ret);
		return ret;
	}

	return 0;
}

int erofs_diff_dump(int argc, char **argv)
{
	int ret;
	void *token = NULL;
	const char *file;

	if (argc < 4) {
		erofs_err("argc[%d] is less than 4 !!", argc);
		usage();
	}

	file = argv[1];
	file_dump_out = argv[2];
	cfg.c_dbg_lvl = atol(argv[3]);

	erofs_show_config();

	ret = dev_open(file_dump_out);
	if (ret)
		usage();

	ret = erofs_diff_write(file, token, argc, argv);
	if (ret < 0) {
		erofs_err("do erofs_diff_write failed ret=%d", ret);
		goto err_exit;
	}
	ret = erofs_diff_local_finish(token);
	if (ret < 0) {
		erofs_err("do erofs_diff_write failed ret=%d", ret);
		goto err_exit;
	}

	dev_close();
	return 0;

err_exit:
	dev_close();
	return ret;
}

int main(int argc, char **argv)
{
	int ret;
	const char *pname = argv[0];
	const unsigned plen = strlen(pname);
	unsigned pi = 0;
	struct {
		const char *name;
		int (*f)(int, char *[]);
	} f[] = { { NULL, NULL },
		{ EROFS_UTILS_COMPRESS, erofs_diff_compress },
		{ EROFS_UTILS_DUMP, erofs_diff_dump }
	};

	ret = erofs_diff_config_init();
	if (ret) {
		return EROFS_EXIT_CODE(ret);
	}

	if (argc < 3) {
		erofs_err("argc[%d] is less than 3 !!", argc);
		usage();
	}

	for (unsigned i = 1; i < sizeof(f) / sizeof(f[0]); ++i) {
		if (!strcmp(f[i].name, &pname[plen - strlen(f[i].name)])) {
			pi = i;
			break;
		}
	}

	if (pi) {
		erofs_info("pi[%d] %s (erofsutils) v%s",
			pi, f[pi].name, cfg.c_version);
		ret = f[pi].f(argc, argv);
		return EROFS_EXIT_CODE(ret);
	}

	usage();
	return EROFS_EXIT_CODE(-EINVAL);
}

