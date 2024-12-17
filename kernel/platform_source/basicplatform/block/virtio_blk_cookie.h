/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description: virtio-blk cookie
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#ifndef __VIRTIO_BLK_COOKIE_H__
#define __VIRTIO_BLK_COOKIE_H__
#include <linux/types.h>
#include <linux/blk_types.h>

#define VIRTIO_BLK_COOKIE_TYPE_INLINE_CRYPTO	1 /* Cookie type inline_crypto */
#define VIRTIO_BLK_COOKIE_TYPE_MAS_OPF	2 /* Cookie type MAS cmd priority */

struct virtio_blk_cookie {
	u16 type;
	u16 len;
	u8 value[0];
};

#ifdef CONFIG_VIRTIO_BLK_COOKIE_FE
#ifdef CONFIG_SCSI_UFS_INLINE_CRYPTO
int blk_inline_crypto_cookie_enc(const struct bio *bio, char *buf, int buf_len);
#else
static inline int blk_inline_crypto_cookie_enc(const struct bio *bio, char *buf, int buf_len)
{
	return 0;
}
#endif
#ifdef CONFIG_MAS_VIRTIO_BLK_FE
int mas_virtio_blk_opf_cookie_enc(const struct bio *bio, char *buf, int buf_len);
#else
static inline int mas_virtio_blk_opf_cookie_enc(const struct bio *bio, char *buf, int buf_len)
{
	return 0;
}
#endif
#endif

#ifdef CONFIG_VIRTIO_BLK_COOKIE_BE
#ifdef CONFIG_SCSI_UFS_INLINE_CRYPTO
int blk_inline_crypto_cookie_dec(struct bio *bio, const char *buf, int buf_len);
#else
static inline int blk_inline_crypto_cookie_dec(struct bio *bio, const char *buf, int buf_len)
{
	return 0;
}
#endif

#ifdef CONFIG_MAS_VIRTIO_BLK_BE
int mas_virtio_blk_opf_cookie_dec(struct bio *bio, const char *buf, int buf_len);
#else
static inline int mas_virtio_blk_opf_cookie_dec(struct bio *bio, const char *buf, int buf_len)
{
	return 0;
}
#endif
#endif

#endif
