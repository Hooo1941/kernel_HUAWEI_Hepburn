// SPDX-License-Identifier: GPL-2.0+
/*
 * erofs-utils/lib/inode.c
 *
 * Copyright (C) 2018-2019 HUAWEI, Inc.
 *             http://www.huawei.com/
 * Created by Li Guifu <bluce.liguifu@huawei.com>
 * with heavy changes by Gao Xiang <gaoxiang25@huawei.com>
 */
#define _GNU_SOURCE
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <dirent.h>
#include "erofs/print.h"
#include "erofs/inode.h"
#include "erofs/cache.h"
#include "erofs/io.h"
#include "erofs/compress.h"
#include "erofs/xattr.h"
#include "erofs/exclude.h"
#ifdef  EROFS_UTILS121
#include "erofs/map.h"
#endif

#define S_SHIFT                 12
static unsigned char erofs_ftype_by_mode[S_IFMT >> S_SHIFT] = {
	[S_IFREG >> S_SHIFT]  = EROFS_FT_REG_FILE,
	[S_IFDIR >> S_SHIFT]  = EROFS_FT_DIR,
	[S_IFCHR >> S_SHIFT]  = EROFS_FT_CHRDEV,
	[S_IFBLK >> S_SHIFT]  = EROFS_FT_BLKDEV,
	[S_IFIFO >> S_SHIFT]  = EROFS_FT_FIFO,
	[S_IFSOCK >> S_SHIFT] = EROFS_FT_SOCK,
	[S_IFLNK >> S_SHIFT]  = EROFS_FT_SYMLINK,
};

static unsigned char erofs_mode_to_ftype(umode_t mode)
{
	return erofs_ftype_by_mode[(mode & S_IFMT) >> S_SHIFT];
}

#define NR_INODE_HASHTABLE	16384

struct list_head inode_hashtable[NR_INODE_HASHTABLE];

void erofs_inode_manager_init(void)
{
	unsigned int i;

	for (i = 0; i < NR_INODE_HASHTABLE; ++i)
		init_list_head(&inode_hashtable[i]);
}

static struct erofs_inode *erofs_igrab(struct erofs_inode *inode)
{
	++inode->i_count;
	return inode;
}

/* get the inode from the (source) inode # */
struct erofs_inode *erofs_iget(dev_t dev, ino_t ino)
{
	struct list_head *head =
		&inode_hashtable[(ino ^ dev) % NR_INODE_HASHTABLE];
	struct erofs_inode *inode;

	list_for_each_entry(inode, head, i_hash)
		if (inode->i_ino[1] == ino && inode->dev == dev)
			return erofs_igrab(inode);
	return NULL;
}

struct erofs_inode *erofs_iget_by_nid(erofs_nid_t nid)
{
	struct list_head *head =
		&inode_hashtable[nid % NR_INODE_HASHTABLE];
	struct erofs_inode *inode;

	list_for_each_entry(inode, head, i_hash)
		if (inode->nid == nid)
			return erofs_igrab(inode);
	return NULL;
}

unsigned int erofs_iput(struct erofs_inode *inode)
{
	struct erofs_dentry *d, *t;

	if (inode->i_count > 1)
		return --inode->i_count;

	list_for_each_entry_safe(d, t, &inode->i_subdirs, d_child)
		free(d);

	list_del(&inode->i_hash);
	free(inode);
	return 0;
}

static int dentry_add_sorted(struct erofs_dentry *d, struct list_head *head)
{
	struct list_head *pos;

	list_for_each(pos, head) {
		struct erofs_dentry *d2 =
			container_of(pos, struct erofs_dentry, d_child);

		if (strcmp(d->name, d2->name) < 0)
			break;
	}
	list_add_tail(&d->d_child, pos);
	return 0;
}

struct erofs_dentry *erofs_d_alloc(struct erofs_inode *parent,
				   const char *name)
{
	struct erofs_dentry *d = malloc(sizeof(*d));

	if (!d)
		return ERR_PTR(-ENOMEM);

	strncpy(d->name, name, EROFS_NAME_LEN - 1);
	d->name[EROFS_NAME_LEN - 1] = '\0';

	dentry_add_sorted(d, &parent->i_subdirs);
	return d;
}

/* allocate main data for a inode */
#ifdef EROFS_UTILS121
__attribute__((unused)) static int __allocate_inode_bh_data(struct erofs_inode *inode,
				    unsigned long nblocks)
#else
static int __allocate_inode_bh_data(struct erofs_inode *inode,
				    unsigned long nblocks)
#endif
{
	struct erofs_buffer_head *bh;
	int ret;

	if (!nblocks) {
		/* it has only tail-end inlined data */
		inode->u.i_blkaddr = NULL_ADDR;
		return 0;
	}

	/* allocate main data buffer */
	bh = erofs_balloc(DATA, blknr_to_addr(nblocks), 0, 0);
	if (IS_ERR(bh))
		return PTR_ERR(bh);

	bh->op = &erofs_skip_write_bhops;
	inode->bh_data = bh;

	/* get blkaddr of the bh */
	ret = erofs_mapbh(bh->block, true);
	DBG_BUGON(ret < 0);

	/* write blocks except for the tail-end block */
	inode->u.i_blkaddr = bh->block->blkaddr;
	return 0;
}

int erofs_prepare_dir_file(struct erofs_inode *dir)
{
	struct erofs_dentry *d;
	unsigned int d_size, i_nlink;
	int ret;

	/* dot is pointed to the current dir inode */
	d = erofs_d_alloc(dir, ".");
	d->inode = erofs_igrab(dir);
	d->type = EROFS_FT_DIR;

	/* dotdot is pointed to the parent dir */
	d = erofs_d_alloc(dir, "..");
	d->inode = erofs_igrab(dir->i_parent);
	d->type = EROFS_FT_DIR;

	/* let's calculate dir size and update i_nlink */
	d_size = 0;
	i_nlink = 0;
	list_for_each_entry(d, &dir->i_subdirs, d_child) {
		int len = strlen(d->name) + sizeof(struct erofs_dirent);

		if (d_size % EROFS_BLKSIZ + len > EROFS_BLKSIZ)
			d_size = round_up(d_size, EROFS_BLKSIZ);
		d_size += len;

		i_nlink += (d->type == EROFS_FT_DIR);
	}
	dir->i_size = d_size;
	/*
	 * if there're too many subdirs as compact form, set nlink=1
	 * rather than upgrade to use extented form instead.
	 */
	if (i_nlink > USHRT_MAX &&
	    dir->inode_isize == sizeof(struct erofs_inode_compact))
		dir->i_nlink = 1;
	else
		dir->i_nlink = i_nlink;

	/* no compression for all dirs */
	dir->datalayout = EROFS_INODE_FLAT_INLINE;

	/* allocate dir main data */
	ret = __allocate_inode_bh_data(dir, erofs_blknr(d_size));
	if (ret)
		return ret;

	/* it will be used in erofs_prepare_inode_buffer */
	dir->idata_size = d_size % EROFS_BLKSIZ;
	return 0;
}

static void fill_dirblock(char *buf, unsigned int size, unsigned int q,
			  struct erofs_dentry *head, struct erofs_dentry *end)
{
	unsigned int p = 0;

	/* write out all erofs_dirents + filenames */
	while (head != end) {
		const unsigned int namelen = strlen(head->name);
		struct erofs_dirent d = {
			.nid = cpu_to_le64(head->nid),
			.nameoff = cpu_to_le16(q),
			.file_type = head->type,
		};

		memcpy(buf + p, &d, sizeof(d));
		memcpy(buf + q, head->name, namelen);
		p += sizeof(d);
		q += namelen;

		head = list_next_entry(head, d_child);
	}
	memset(buf + q, 0, size - q);
}

static int write_dirblock(unsigned int q, struct erofs_dentry *head,
			  struct erofs_dentry *end, erofs_blk_t blkaddr)
{
	char buf[EROFS_BLKSIZ];

	fill_dirblock(buf, EROFS_BLKSIZ, q, head, end);
	return blk_write(buf, blkaddr, 1);
}

int erofs_write_dir_file(struct erofs_inode *dir)
{
	struct erofs_dentry *head = list_first_entry(&dir->i_subdirs,
						     struct erofs_dentry,
						     d_child);
	struct erofs_dentry *d;
	int ret;
	unsigned int q, used, blkno;

	q = used = blkno = 0;

	list_for_each_entry(d, &dir->i_subdirs, d_child) {
		const unsigned int len = strlen(d->name) +
			sizeof(struct erofs_dirent);

		if (used + len > EROFS_BLKSIZ) {
			ret = write_dirblock(q, head, d,
					     dir->u.i_blkaddr + blkno);
			if (ret)
				return ret;

			head = d;
			q = used = 0;
			++blkno;
		}
		used += len;
		q += sizeof(struct erofs_dirent);
	}

	DBG_BUGON(used > EROFS_BLKSIZ);
	if (used == EROFS_BLKSIZ) {
		DBG_BUGON(dir->i_size % EROFS_BLKSIZ);
		DBG_BUGON(dir->idata_size);
		return write_dirblock(q, head, d, dir->u.i_blkaddr + blkno);
	}
	DBG_BUGON(used != dir->i_size % EROFS_BLKSIZ);
	if (used) {
		/* fill tail-end dir block */
		dir->idata = malloc(used);
		if (!dir->idata)
			return -ENOMEM;
		DBG_BUGON(used != dir->idata_size);
		fill_dirblock(dir->idata, dir->idata_size, q, head, d);
	}
	return 0;
}

int erofs_write_file_from_buffer(struct erofs_inode *inode, char *buf)
{
	const unsigned int nblocks = erofs_blknr(inode->i_size);
	int ret;

	inode->datalayout = EROFS_INODE_FLAT_INLINE;

	ret = __allocate_inode_bh_data(inode, nblocks);
	if (ret)
		return ret;

	if (nblocks)
		blk_write(buf, inode->u.i_blkaddr, nblocks);
	inode->idata_size = inode->i_size % EROFS_BLKSIZ;
	if (inode->idata_size) {
		inode->idata = malloc(inode->idata_size);
		if (!inode->idata)
			return -ENOMEM;
		memcpy(inode->idata, buf + blknr_to_addr(nblocks),
		       inode->idata_size);
	}
	return 0;
}

#ifdef  EROFS_UTILS121
/* rules to decide whether a file could be compressed or not */
static bool erofs_file_is_compressible(struct erofs_inode *inode)
{
	char filepath[PATH_MAX + 1] = { 0 };
	const char *pfile = inode->i_srcpath;
	const char **plist = cfg.c_file_uncompr_list;

	pfile = pfile + strlen(cfg.c_src_path) + 1; // 1 means skip **/**
	int ret = snprintf(filepath, PATH_MAX, "%s%s", cfg.mount_point, pfile);
	if (ret < 0 || ret >= PATH_MAX) {
		erofs_err("snprintf errorly ret[%d]", ret);
		return true;
	}

	while (*plist != NULL) {
		if (!strcmp(filepath, *plist))
			return false;
		plist++;
	}
	return true;
}

/* rules to decide whether a file is noinline or not */
static bool erofs_file_is_noinline(const char *file)
{
	const char *extfilename = NULL;
	const char **plist = cfg.c_file_noinline_list;


	extfilename = strrchr(file, '.');
	if (!extfilename)
		return false;

	extfilename++;
	size_t extfnlen = strlen(extfilename);
	if (extfnlen == 0)
		return false;

	while (*plist != NULL) {
		if (!strcmp(extfilename, *plist))
			return true;
		plist++;
	}

	return false;
}

/*get data's layout*/
static const char *get_datalayout(unsigned char layout)
{
	switch (layout) {
	case EROFS_INODE_FLAT_PLAIN:
		return "PLAIN";
	case EROFS_INODE_FLAT_COMPRESSION_LEGACY:
		return "COMPRESSION_LEGACY";
	case EROFS_INODE_FLAT_INLINE:
		return "INLINE";
	case EROFS_INODE_FLAT_COMPRESSION:
		return "COMPRESSION";
	case EROFS_INODE_DATALAYOUT_MAX:
		return "MAX";
	default:
		return "error layout";
	}
}
#endif

#if PATCH_ENABLED
static int write_compressed_file_patch(struct erofs_inode *inode,
				       struct erofs_inode *org_inode)
{
	int ret;

	if (org_inode && is_inode_layout_compression(org_inode)) {
		/* try use old block */
		ret = erofs_write_compressed_file(inode, org_inode, false);
		erofs_info("try first [%s] ret:%d", inode->i_srcpath, ret);
		if (ret == 0)
			ret = erofs_write_compressed_file(inode,
							  org_inode, true);
		else if (ret == -1)
			ret = erofs_write_compressed_file(inode,
							  NULL, true);
	} else {
		ret = erofs_write_compressed_file(inode, NULL, true);
	}
	return ret;
 }

int allocate_inode_bh_data_patch(struct erofs_inode *inode,
				 unsigned long nblocks,
				 struct erofs_inode *org_inode)
{
	/* use old data block first */
	if (org_inode && nblocks <= org_inode->nblocks) {
		inode->u.i_blkaddr = org_inode->startaddr;
		inode->bh_data = org_inode->bh_data;
		return 0;
	} else {
		return __allocate_inode_bh_data(inode, nblocks);
	}
}
#endif

#if PATCH_ENABLED
static int write_uncompressed_file_from_fd(struct erofs_inode *inode, int fd, struct erofs_inode *org_inode)
#else
static int write_uncompressed_file_from_fd(struct erofs_inode *inode, int fd)
#endif
{
	int ret;
	unsigned int nblocks, i;

	inode->datalayout = EROFS_INODE_FLAT_INLINE;
	nblocks = inode->i_size / EROFS_BLKSIZ;

#if PATCH_ENABLED
	ret = allocate_inode_bh_data_patch(inode, nblocks, org_inode);
#else
	ret = __allocate_inode_bh_data(inode, nblocks);
#endif
	if (ret)
		return ret;

#ifdef  EROFS_UTILS121
	inode->nblocks = nblocks;
	inode->startaddr = inode->u.i_blkaddr;
#endif

	for (i = 0; i < nblocks; ++i) {
		char buf[EROFS_BLKSIZ];

		ret = read(fd, buf, EROFS_BLKSIZ);
		if (ret != EROFS_BLKSIZ) {
			if (ret < 0)
				return -errno;
			return -EAGAIN;
		}

		ret = blk_write(buf, inode->u.i_blkaddr + i, 1);
		if (ret)
			return ret;
	}

	/* read the tail-end data */
	inode->idata_size = inode->i_size % EROFS_BLKSIZ;
        inode->u.i_blocks = inode->i_size / EROFS_BLKSIZ;
	if (inode->idata_size) {
		inode->idata = malloc(inode->idata_size);
		if (!inode->idata)
			return -ENOMEM;

		ret = read(fd, inode->idata, inode->idata_size);
		if (ret < inode->idata_size) {
			free(inode->idata);
			inode->idata = NULL;
			return -EIO;
		}
	}
	return 0;
}

#ifdef  EROFS_UTILS121
int erofs_write_file(struct erofs_inode *inode, struct erofs_inode *org_inode)
#else
int erofs_write_file(struct erofs_inode *inode)
#endif
{
	int ret, fd;

	if (!inode->i_size) {
		inode->datalayout = EROFS_INODE_FLAT_PLAIN;
		return 0;
	}

	if (cfg.c_compr_alg_master && erofs_file_is_compressible(inode)) {
#if PATCH_ENABLED
		ret = write_compressed_file_patch(inode, org_inode);
#else
		UNUSED(org_inode);
		ret = erofs_write_compressed_file(inode, NULL, true);
#endif
		if (!ret || ret != -ENOSPC)
			return ret;
	}

	/* fallback to all data uncompressed */
	fd = open(inode->i_srcpath, O_RDONLY | O_BINARY);
	if (fd < 0)
		return -errno;

#if PATCH_ENABLED
	ret = write_uncompressed_file_from_fd(inode, fd, org_inode);
#else
	ret = write_uncompressed_file_from_fd(inode, fd);
#endif
	close(fd);
	return ret;
}

static bool erofs_bh_flush_write_inode(struct erofs_buffer_head *bh)
{
	struct erofs_inode *const inode = bh->fsprivate;
	const u16 icount = EROFS_INODE_XATTR_ICOUNT(inode->xattr_isize);
	erofs_off_t off = erofs_btell(bh, false);
	union {
		struct erofs_inode_compact dic;
		struct erofs_inode_extended die;
	} u = { {0}, };
	int ret;

	switch (inode->inode_isize) {
	case sizeof(struct erofs_inode_compact):
		u.dic.i_format = cpu_to_le16(0 | (inode->datalayout << 1));
		u.dic.i_xattr_icount = cpu_to_le16(icount);
		u.dic.i_mode = cpu_to_le16(inode->i_mode);
		u.dic.i_nlink = cpu_to_le16(inode->i_nlink);
		u.dic.i_size = cpu_to_le32((u32)inode->i_size);

		u.dic.i_ino = cpu_to_le32(inode->i_ino[0]);

		u.dic.i_uid = cpu_to_le16((u16)inode->i_uid);
		u.dic.i_gid = cpu_to_le16((u16)inode->i_gid);

		switch ((inode->i_mode) >> S_SHIFT) {
		case S_IFCHR:
		case S_IFBLK:
		case S_IFIFO:
		case S_IFSOCK:
			u.dic.i_u.rdev = cpu_to_le32(inode->u.i_rdev);
			break;

		default:
			if (is_inode_layout_compression(inode))
				u.dic.i_u.compressed_blocks =
					cpu_to_le32(inode->u.i_blocks);
			else
				u.dic.i_u.raw_blkaddr =
					cpu_to_le32(inode->u.i_blkaddr);
			break;
		}
		break;
	case sizeof(struct erofs_inode_extended):
		u.die.i_format = cpu_to_le16(1 | (inode->datalayout << 1));
		u.die.i_xattr_icount = cpu_to_le16(icount);
		u.die.i_mode = cpu_to_le16(inode->i_mode);
		u.die.i_nlink = cpu_to_le32(inode->i_nlink);
		u.die.i_size = cpu_to_le64(inode->i_size);

		u.die.i_ino = cpu_to_le32(inode->i_ino[0]);

		u.die.i_uid = cpu_to_le16(inode->i_uid);
		u.die.i_gid = cpu_to_le16(inode->i_gid);

		u.die.i_ctime = cpu_to_le64(inode->i_ctime);
		u.die.i_ctime_nsec = cpu_to_le32(inode->i_ctime_nsec);

		switch ((inode->i_mode) >> S_SHIFT) {
		case S_IFCHR:
		case S_IFBLK:
		case S_IFIFO:
		case S_IFSOCK:
			u.die.i_u.rdev = cpu_to_le32(inode->u.i_rdev);
			break;

		default:
			if (is_inode_layout_compression(inode))
				u.die.i_u.compressed_blocks =
					cpu_to_le32(inode->u.i_blocks);
			else
				u.die.i_u.raw_blkaddr =
					cpu_to_le32(inode->u.i_blkaddr);
			break;
		}
		break;
	default:
		erofs_err("unsupported on-disk inode version of nid %llu",
			  (unsigned long long)inode->nid);
		BUG_ON(1);
	}

	ret = dev_write(&u, off, inode->inode_isize);
	if (ret)
		return false;
	off += inode->inode_isize;

	if (inode->xattr_isize) {
		char *xattrs = erofs_export_xattr_ibody(&inode->i_xattrs,
							inode->xattr_isize);
		if (IS_ERR(xattrs))
			return false;

		ret = dev_write(xattrs, off, inode->xattr_isize);
		free(xattrs);
		if (ret)
			return false;

		off += inode->xattr_isize;
	}

	if (inode->extent_isize) {
		/* write compression metadata */
		off = Z_EROFS_VLE_EXTENT_ALIGN(off);
		ret = dev_write(inode->compressmeta, off, inode->extent_isize);
		if (ret)
			return false;
		free(inode->compressmeta);
	}

#ifdef  EROFS_UTILS121
        erofs_info("nid=%lu end_off=%lx data layout=%s, isize:%llu iblocks:%u file:%s",
		   inode->nid, off, get_datalayout(inode->datalayout),
		   (long long)inode->i_size, inode->u.i_blocks, inode->i_srcpath);
#endif
	inode->bh = NULL;
	erofs_iput(inode);
	return erofs_bh_flush_generic_end(bh);
}

static struct erofs_bhops erofs_write_inode_bhops = {
	.flush = erofs_bh_flush_write_inode,
};

int erofs_prepare_tail_block(struct erofs_inode *inode)
{
	struct erofs_buffer_head *bh;
	int ret;

	if (!inode->idata_size)
		return 0;

	bh = inode->bh_data;
	if (!bh) {
		bh = erofs_balloc(DATA, EROFS_BLKSIZ, 0, 0);
		if (IS_ERR(bh))
			return PTR_ERR(bh);
		bh->op = &erofs_skip_write_bhops;

		/* get blkaddr of bh */
		ret = erofs_mapbh(bh->block, true);
		DBG_BUGON(ret < 0);
		inode->u.i_blkaddr = bh->block->blkaddr;

		inode->bh_data = bh;
		return 0;
	}
	/* expend a block as the tail block (should be successful) */
	ret = erofs_bh_balloon(bh, EROFS_BLKSIZ);
	DBG_BUGON(ret);
	return 0;
}

int erofs_prepare_inode_buffer(struct erofs_inode *inode)
{
	unsigned int inodesize;
	struct erofs_buffer_head *bh, *ibh;

	DBG_BUGON(inode->bh || inode->bh_inline);

	inodesize = inode->inode_isize + inode->xattr_isize;
	if (inode->extent_isize)
		inodesize = Z_EROFS_VLE_EXTENT_ALIGN(inodesize) +
			    inode->extent_isize;

	if (is_inode_layout_compression(inode))
		goto noinline;

#ifdef  EROFS_UTILS121
        if (erofs_file_is_noinline(inode->i_srcpath)) {
			inode->datalayout = EROFS_INODE_FLAT_PLAIN;
			goto noinline;
		}
#endif

	/*
	 * if the file size is block-aligned for uncompressed files,
	 * should use EROFS_INODE_FLAT_PLAIN data mapping mode.
	 */
	if (!inode->idata_size)
		inode->datalayout = EROFS_INODE_FLAT_PLAIN;

	bh = erofs_balloc(INODE, inodesize, 0, inode->idata_size);
	if (bh == ERR_PTR(-ENOSPC)) {
		int ret;

		inode->datalayout = EROFS_INODE_FLAT_PLAIN;
noinline:
		/* expend an extra block for tail-end data */
		ret = erofs_prepare_tail_block(inode);
		if (ret)
			return ret;
		bh = erofs_balloc(INODE, inodesize, 0, 0);
		if (IS_ERR(bh))
			return PTR_ERR(bh);
		DBG_BUGON(inode->bh_inline);
	} else if (IS_ERR(bh)) {
		return PTR_ERR(bh);
	} else if (inode->idata_size) {
		inode->datalayout = EROFS_INODE_FLAT_INLINE;

		/* allocate inline buffer */
		ibh = erofs_battach(bh, META, inode->idata_size);
		if (IS_ERR(ibh))
			return PTR_ERR(ibh);

		ibh->op = &erofs_skip_write_bhops;
		inode->bh_inline = ibh;
	}

	bh->fsprivate = erofs_igrab(inode);
	bh->op = &erofs_write_inode_bhops;
	inode->bh = bh;
	return 0;
}

static bool erofs_bh_flush_write_inline(struct erofs_buffer_head *bh)
{
	struct erofs_inode *const inode = bh->fsprivate;
	const erofs_off_t off = erofs_btell(bh, false);
	int ret;

	ret = dev_write(inode->idata, off, inode->idata_size);
	if (ret)
		return false;

	inode->idata_size = 0;
	free(inode->idata);
	inode->idata = NULL;

	erofs_iput(inode);
	return erofs_bh_flush_generic_end(bh);
}

static struct erofs_bhops erofs_write_inline_bhops = {
	.flush = erofs_bh_flush_write_inline,
};

int erofs_write_tail_end(struct erofs_inode *inode)
{
	struct erofs_buffer_head *bh, *ibh;

	bh = inode->bh_data;

	if (!inode->idata_size)
		goto out;

	/* have enough room to inline data */
	if (inode->bh_inline) {
		ibh = inode->bh_inline;

		ibh->fsprivate = erofs_igrab(inode);
		ibh->op = &erofs_write_inline_bhops;
	} else {
		int ret;
		erofs_off_t pos;

		erofs_mapbh(bh->block, true);
		pos = erofs_btell(bh, true) - EROFS_BLKSIZ;
		ret = dev_write(inode->idata, pos, inode->idata_size);
		if (ret)
			return ret;
		if (inode->idata_size < EROFS_BLKSIZ) {
			ret = dev_fillzero(pos + inode->idata_size,
					   EROFS_BLKSIZ - inode->idata_size,
					   false);
			if (ret)
				return ret;
		}
#if !PATCH_ENABLED
		inode->idata_size = 0;
		free(inode->idata);
		inode->idata = NULL;
#endif
	}
out:
	/* now bh_data can drop directly */
	if (bh) {
		/*
		 * Don't leave DATA buffers which were written in the global
		 * buffer list. It will make balloc() slowly.
		 */
#if 0
		bh->op = &erofs_drop_directly_bhops;
#else
		erofs_bdrop(bh, false);
#endif
		inode->bh_data = NULL;
	}
	return 0;
}

static bool erofs_should_use_inode_extended(struct erofs_inode *inode)
{
	if (cfg.c_force_inodeversion == FORCE_INODE_EXTENDED)
		return true;
	if (inode->i_size > UINT_MAX)
		return true;
	if (inode->i_uid > USHRT_MAX)
		return true;
	if (inode->i_gid > USHRT_MAX)
		return true;
	if (inode->i_nlink > USHRT_MAX)
		return true;
	return false;
}

static u32 erofs_new_encode_dev(dev_t dev)
{
	const unsigned int major = major(dev);
	const unsigned int minor = minor(dev);

	return (minor & 0xff) | (major << 8) | ((minor & ~0xff) << 12);
}

#ifdef WITH_ANDROID
int erofs_droid_inode_fsconfig(struct erofs_inode *inode,
			       struct stat64 *st,
			       const char *path)
{
	/* filesystem_config does not preserve file type bits */
	mode_t stat_file_type_mask = st->st_mode & S_IFMT;
	unsigned int uid = 0, gid = 0, mode = 0;
	const char *fspath;
	char *decorated = NULL;

	inode->capabilities = 0;
	if (!cfg.fs_config_file && !cfg.mount_point)
		return 0;

	if (!cfg.mount_point ||
	/* have to drop the mountpoint for rootdir of canned fsconfig */
	    (cfg.fs_config_file && erofs_fspath(path)[0] == '\0')) {
		fspath = erofs_fspath(path);
	} else {
		if (asprintf(&decorated, "%s/%s", cfg.mount_point,
			     erofs_fspath(path)) <= 0)
			return -ENOMEM;
		fspath = decorated;
	}

	if (cfg.fs_config_file)
		canned_fs_config(fspath, S_ISDIR(st->st_mode),
				 cfg.target_out_path,
				 &uid, &gid, &mode, &inode->capabilities);
	else
		fs_config(fspath, S_ISDIR(st->st_mode),
			  cfg.target_out_path,
			  &uid, &gid, &mode, &inode->capabilities);

	erofs_dbg("/%s -> mode = 0x%x, uid = 0x%x, gid = 0x%x, "
		  "capabilities = 0x%" PRIx64 "\n",
		  fspath, mode, uid, gid, inode->capabilities);

	if (decorated)
		free(decorated);
	st->st_uid = uid;
	st->st_gid = gid;
	st->st_mode = mode | stat_file_type_mask;
	return 0;
}
#else
static int erofs_droid_inode_fsconfig(struct erofs_inode *inode,
				      struct stat64 *st,
				      const char *path)
{
	return 0;
}
#endif

int erofs_fill_inode(struct erofs_inode *inode,
		     struct stat64 *st,
		     const char *path)
{
	int err = erofs_droid_inode_fsconfig(inode, st, path);

	if (err)
		return err;
	inode->i_mode = st->st_mode;
	inode->i_uid = st->st_uid;
	inode->i_gid = st->st_gid;
	inode->i_ctime = st->st_ctime;
	inode->i_ctime_nsec = st->st_ctim.tv_nsec;

	switch (cfg.c_timeinherit) {
	case TIMESTAMP_CLAMPING:
		if ((u64)(st->st_ctime) < sbi.build_time)
			break;
	case TIMESTAMP_FIXED:
		inode->i_ctime = sbi.build_time;
		inode->i_ctime_nsec = sbi.build_time_nsec;
	default:
		break;
	}
	inode->i_nlink = 1;	/* fix up later if needed */

	switch (inode->i_mode & S_IFMT) {
	case S_IFCHR:
	case S_IFBLK:
	case S_IFIFO:
	case S_IFSOCK:
		inode->u.i_rdev = erofs_new_encode_dev(st->st_rdev);
	case S_IFDIR:
		inode->i_size = 0;
		break;
	case S_IFREG:
	case S_IFLNK:
		inode->i_size = st->st_size;
		break;
	default:
		return -EINVAL;
	}

	strncpy(inode->i_srcpath, path, sizeof(inode->i_srcpath) - 1);
	inode->i_srcpath[sizeof(inode->i_srcpath) - 1] = '\0';

	inode->dev = st->st_dev;
	inode->i_ino[1] = st->st_ino;

	if (erofs_should_use_inode_extended(inode)) {
		if (cfg.c_force_inodeversion == FORCE_INODE_COMPACT) {
			erofs_err("file %s cannot be in compact form",
				  inode->i_srcpath);
			return -EINVAL;
		}
		inode->inode_isize = sizeof(struct erofs_inode_extended);
	} else {
		inode->inode_isize = sizeof(struct erofs_inode_compact);
	}

	list_add(&inode->i_hash,
		 &inode_hashtable[(st->st_ino ^ st->st_dev) %
				  NR_INODE_HASHTABLE]);
	return 0;
}

struct erofs_inode *erofs_new_inode(void)
{
	static unsigned int counter;
	struct erofs_inode *inode;

	inode = malloc(sizeof(struct erofs_inode));
	if (!inode)
		return ERR_PTR(-ENOMEM);
#ifdef EROFS_UTILS121
	inode->u.i_blocks = 0;
#endif
	inode->i_parent = NULL;	/* also used to indicate a new inode */

	inode->i_ino[0] = counter++;	/* inode serial number */
	inode->i_count = 1;

	init_list_head(&inode->i_subdirs);
	init_list_head(&inode->i_xattrs);

	inode->idata_size = 0;
	inode->xattr_isize = 0;
	inode->extent_isize = 0;

	inode->bh = inode->bh_inline = inode->bh_data = NULL;
	inode->idata = NULL;
	return inode;
}

/* get the inode from the (source) path */
struct erofs_inode *erofs_iget_from_path(const char *path, bool is_src)
{
	struct stat64 st;
	struct erofs_inode *inode;
	int ret;

	/* currently, only source path is supported */
	if (!is_src)
		return ERR_PTR(-EINVAL);

	ret = lstat64(path, &st);
	if (ret)
		return ERR_PTR(-errno);

	/*
	 * lookup in hash table first, if it already exists we have a
	 * hard-link, just return it. Also don't lookup for directories
	 * since hard-link directory isn't allowed.
	 */
	if (!S_ISDIR(st.st_mode)) {
		inode = erofs_iget(st.st_dev, st.st_ino);
		if (inode)
			return inode;
	}

	/* cannot find in the inode cache */
	inode = erofs_new_inode();
	if (IS_ERR(inode))
		return inode;

	ret = erofs_fill_inode(inode, &st, path);
	if (ret)
		return ERR_PTR(ret);

	return inode;
}

#if PATCH_ENABLED
struct erofs_inode *erofs_iget_from_path_patch(const char *path, bool is_src)
{
	struct erofs_inode *inode = erofs_iget_from_path(path, is_src);

	if (IS_ERR(inode))
		return inode;

	list_add(&inode->i_hash, &inode_hashtable[inode->i_ino[1] % NR_INODE_HASHTABLE]);

	return inode;
}
#endif

void erofs_fixup_meta_blkaddr(struct erofs_inode *rootdir)
{
	const erofs_off_t rootnid_maxoffset = 0xffff << EROFS_ISLOTBITS;
	struct erofs_buffer_head *const bh = rootdir->bh;
	erofs_off_t off, meta_offset;

	erofs_mapbh(bh->block, true);
	off = erofs_btell(bh, false);

	if (off > rootnid_maxoffset)
		meta_offset = round_up(off - rootnid_maxoffset, EROFS_BLKSIZ);
	else
		meta_offset = 0;
	sbi.meta_blkaddr = erofs_blknr(meta_offset);
	rootdir->nid = (off - meta_offset) >> EROFS_ISLOTBITS;
}

erofs_nid_t erofs_lookupnid(struct erofs_inode *inode)
{
	struct erofs_buffer_head *const bh = inode->bh;
	erofs_off_t off, meta_offset;

	if (!bh)
		return inode->nid;

	erofs_mapbh(bh->block, true);
	off = erofs_btell(bh, false);

	meta_offset = blknr_to_addr(sbi.meta_blkaddr);
	DBG_BUGON(off < meta_offset);
	return inode->nid = (off - meta_offset) >> EROFS_ISLOTBITS;
}

void erofs_d_invalidate(struct erofs_dentry *d)
{
	struct erofs_inode *const inode = d->inode;

	d->nid = erofs_lookupnid(inode);
	erofs_iput(inode);
}

struct erofs_inode *erofs_mkfs_build_tree(struct erofs_inode *dir)
{
	int ret;
	DIR *_dir;
	struct dirent *dp;
	struct erofs_dentry *d;

#if PATCH_ENABLED
	ret = erofs_prepare_xattr_ibody(dir, NULL);
#else
	ret = erofs_prepare_xattr_ibody(dir);
#endif
	if (ret < 0)
		return ERR_PTR(ret);

	if (!S_ISDIR(dir->i_mode)) {
		if (S_ISLNK(dir->i_mode)) {
			char *const symlink = malloc(dir->i_size);

			if (!symlink)
				return ERR_PTR(-ENOMEM);
			ret = readlink(dir->i_srcpath, symlink, dir->i_size);
			if (ret < 0) {
				free(symlink);
				return ERR_PTR(-errno);
			}

			ret = erofs_write_file_from_buffer(dir, symlink);
			free(symlink);
			if (ret)
				return ERR_PTR(ret);
		} else {
			ret = erofs_write_file(dir, NULL);
			if (ret)
				return ERR_PTR(ret);
		}

		erofs_prepare_inode_buffer(dir);

        if (!is_inode_layout_compression(dir) && dir->u.i_blocks > 0) {
			erofs_output_map_blocks(dir->i_srcpath,
						dir->u.i_blkaddr,
						dir->u.i_blocks);
			erofs_output_mapbin_blocks(dir->datalayout,
						   true, dir->u.i_blkaddr,
						   dir->i_size);
		}
		erofs_write_tail_end(dir);
		return dir;
	}

	_dir = opendir(dir->i_srcpath);
	if (!_dir) {
		erofs_err("%s, failed to opendir at %s: %s",
			  __func__, dir->i_srcpath, erofs_strerror(errno));
		return ERR_PTR(-errno);
	}

	while (1) {
		/*
		 * set errno to 0 before calling readdir() in order to
		 * distinguish end of stream and from an error.
		 */
		errno = 0;
		dp = readdir(_dir);
		if (!dp)
			break;

		if (is_dot_dotdot(dp->d_name) ||
		    !strncmp(dp->d_name, "lost+found", strlen("lost+found")))
			continue;

		/* skip if it's a exclude file */
		if (erofs_is_exclude_path(dir->i_srcpath, dp->d_name))
			continue;

		d = erofs_d_alloc(dir, dp->d_name);
		if (IS_ERR(d)) {
			ret = PTR_ERR(d);
			goto err_closedir;
		}

		/* to count i_nlink for directories */
		d->type = (dp->d_type == DT_DIR ?
			EROFS_FT_DIR : EROFS_FT_UNKNOWN);
	}

	if (errno) {
		ret = -errno;
		goto err_closedir;
	}
	closedir(_dir);

	ret = erofs_prepare_dir_file(dir);
	if (ret)
		goto err;

	ret = erofs_prepare_inode_buffer(dir);
	if (ret)
		goto err;

	if (IS_ROOT(dir))
		erofs_fixup_meta_blkaddr(dir);

	list_for_each_entry(d, &dir->i_subdirs, d_child) {
		char buf[PATH_MAX];
		unsigned char ftype;

		if (is_dot_dotdot(d->name)) {
			erofs_d_invalidate(d);
			continue;
		}

		ret = snprintf(buf, PATH_MAX, "%s/%s",
			       dir->i_srcpath, d->name);
		if (ret < 0 || ret >= PATH_MAX) {
			/* ignore the too long path */
			goto fail;
		}

		d->inode = erofs_mkfs_build_tree_from_path(dir, buf);
		if (IS_ERR(d->inode)) {
			ret = PTR_ERR(d->inode);
fail:
			d->inode = NULL;
			d->type = EROFS_FT_UNKNOWN;
			goto err;
		}

		ftype = erofs_mode_to_ftype(d->inode->i_mode);
		DBG_BUGON(ftype == EROFS_FT_DIR && d->type != ftype);
		d->type = ftype;

		erofs_d_invalidate(d);
		erofs_info("add file %s/%s (nid %llu, type %d)",
			   dir->i_srcpath, d->name, (unsigned long long)d->nid,
			   d->type);
	}
	erofs_write_dir_file(dir);
	erofs_write_tail_end(dir);
	return dir;

err_closedir:
	closedir(_dir);
err:
	return ERR_PTR(ret);
}

struct erofs_inode *erofs_mkfs_build_tree_from_path(struct erofs_inode *parent,
						    const char *path)
{
	struct erofs_inode *const inode = erofs_iget_from_path(path, true);

	if (IS_ERR(inode))
		return inode;

	/* a hardlink to the existed inode */
	if (inode->i_parent) {
		++inode->i_nlink;
		return inode;
	}

	/* a completely new inode is found */
	if (parent)
		inode->i_parent = parent;
	else
		inode->i_parent = inode;	/* rootdir mark */

	return erofs_mkfs_build_tree(inode);
}

