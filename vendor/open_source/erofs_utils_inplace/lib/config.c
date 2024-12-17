// SPDX-License-Identifier: GPL-2.0+
/*
 * erofs-utils/lib/config.c
 *
 * Copyright (C) 2018-2019 HUAWEI, Inc.
 *             http://www.huawei.com/
 * Created by Li Guifu <bluce.liguifu@huawei.com>
 */
#include <string.h>
#include "erofs/print.h"
#include "erofs/internal.h"
#ifdef  EROFS_UTILS121
#include <stdlib.h>
#include "limits.h"
/* diff multi blocks 150MB = 4KB * 38400 */
#define EROFS_MULTI_BLKS_THRESHOLD    38400
#define EROFS_MULTI_BLKS_POSTFIX      "erofs_postfix"
#define MKFS_XATTR_MAX_SHARED_REFCOUNT  (8)

#if !defined(PACKAGE_VERSION)
#define PACKAGE_VERSION          "version: v1.2.1"
#endif

#endif

struct erofs_configure cfg;
struct erofs_sb_info sbi;

#ifdef EROFS_UTILS121
static const char *file_uncompr_list[] = {
	"/system/priv-app/HwCamera2/HwCamera2.apk",
	"/system/priv-app/HwSystemManager/HwSystemManager.apk",
	"/system/priv-app/HwStartupGuide/HwStartupGuide.apk",
	"/system/framework/boot-framework.vdex",
	"/system/framework/arm/boot-framework.vdex",
	"/system/framework/arm/boot-framework.art",
	"/system/framework/arm/boot-framework.oat",
	"/system/framework/arm64/boot-framework.vdex",
	"/system/framework/arm64/boot-framework.art",
	"/system/framework/arm64/boot-framework.oat",
	"/system/lib/libc.so",
	"/system/lib64/libc.so",
	"/system/lib/libc++.so",
	"/system/lib64/libc++.so",
	"/system/lib/libEGL.so",
	"/system/lib64/libEGL.so",
	"/system/lib/libart.so",
	"/system/lib64/libart.so",
	NULL,
};

static const char *file_noinline_list[] = {
	"zip",
	"apk",
	"jar",
	NULL
};
#endif

void erofs_init_configure(void)
{
	memset(&cfg, 0, sizeof(cfg));

	cfg.c_dbg_lvl  = 0;
	cfg.c_version  = PACKAGE_VERSION;
	cfg.c_dry_run  = false;
	cfg.c_compr_level_master = -1;
	cfg.c_force_inodeversion = 0;
	cfg.c_inline_xattr_tolerance = 2;
	cfg.c_unix_timestamp = -1;
#ifdef EROFS_UTILS121
        cfg.c_compr_ver_master = EROFS_COMPR_LOCAL_VERSION;
	cfg.mount_point = "/";
	cfg.c_shared_xattr_threshold = MKFS_XATTR_MAX_SHARED_REFCOUNT;
        cfg.c_multi_blks_thr = EROFS_MULTI_BLKS_THRESHOLD;
	cfg.c_img_len    = ULONG_MAX;
        cfg.c_img_path = NULL;
	cfg.c_src_path = NULL;
	cfg.c_compr_alg_master = "lz4hc";
	cfg.c_file_uncompr_list = file_uncompr_list;
	cfg.c_file_noinline_list = file_noinline_list;
#endif

#if PATCH_ENABLED
    cfg.c_patch_path = NULL;
#endif
}

void erofs_show_config(void)
{
	const struct erofs_configure *c = &cfg;
	erofs_dump("\tc_version:           [%8s]\n", c->c_version);
	erofs_dump("\tc_dbg_lvl:           [%8d]\n", c->c_dbg_lvl);
	erofs_dump("\tc_dry_run:           [%8d]\n", c->c_dry_run);
#ifdef EROFS_UTILS121
	erofs_dump("\tc_img_path:            [%8s]\n", c->c_img_path);
	erofs_dump("\tc_src_path:            [%8s]\n", c->c_src_path);
	erofs_dump("\tmount_point:           [%8s]\n", c->mount_point);
	erofs_dump("\tc_compr version:       [%8d]\n", c->c_compr_ver_master);
	erofs_dump("\tc_compr_alg_master:    [%8s]\n", c->c_compr_alg_master);
	erofs_dump("\ttarget_out_path:       [%8s]\n", c->target_out_path);
	erofs_dump("\tfs_config_file:        [%8s]\n", c->fs_config_file);
	erofs_dump("\tc_shared_xattr_thresh: [%8d]\n", c->c_shared_xattr_threshold);
	erofs_dump("\tc_img_len:             [%8" PRIu64 "]\n", c->c_img_len);
	erofs_dump("\tc_multi_blks_thr:      [%8u]\n", c->c_multi_blks_thr);
	erofs_dump("\tc_unix_timestamp:      [%8" PRIi64 "]\n", c->c_unix_timestamp);
	erofs_dump("\tc_inline_xattr_tolerance:          [%8d]\n", c->c_inline_xattr_tolerance);
#endif
#if PATCH_ENABLED
        erofs_dump("\tc_patch_path:          [%8s]\n", c->c_patch_path);
#endif
}

void erofs_exit_configure(void)
{
#ifdef HAVE_LIBSELINUX
	if (cfg.sehnd)
		selabel_close(cfg.sehnd);
#endif

#ifdef EROFS_UTILS121
        if (cfg.c_img_path) {
		free((void *)cfg.c_img_path);
		cfg.c_img_path = NULL;
	}
	if (cfg.c_src_path) {
		free((void *)cfg.c_src_path);
		cfg.c_src_path = NULL;
	}

	if (cfg.target_out_path) {
		free((void *)cfg.target_out_path);
		cfg.target_out_path = NULL;
	}
#endif

#if PATCH_ENABLED
	if (cfg.c_patch_path) {
		free((void *)cfg.c_patch_path);
		cfg.c_patch_path = NULL;
	}
#endif
}

static unsigned int fullpath_prefix;	/* root directory prefix length */

void erofs_set_fs_root(const char *rootdir)
{
	fullpath_prefix = strlen(rootdir);
}

const char *erofs_fspath(const char *fullpath)
{
	const char *s = fullpath + fullpath_prefix;

	while (*s == '/')
		s++;
	return s;
}

#ifdef HAVE_LIBSELINUX
int erofs_selabel_open(const char *file_contexts)
{
	struct selinux_opt seopts[] = {
		{ .type = SELABEL_OPT_PATH, .value = file_contexts }
	};

	if (cfg.sehnd) {
		erofs_info("ignore duplicated file contexts \"%s\"",
			   file_contexts);
		return -EBUSY;
	}

	cfg.sehnd = selabel_open(SELABEL_CTX_FILE, seopts, 1);
	if (!cfg.sehnd) {
		erofs_err("failed to open file contexts \"%s\"",
			  file_contexts);
		return -EINVAL;
	}
	return 0;
}
#endif

