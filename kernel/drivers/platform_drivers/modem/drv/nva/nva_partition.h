/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
 * foss@huawei.com
 *
 * If distributed as part of the Linux kernel, the following license terms
 * apply:
 *
 * * This program is free software; you can redistribute it and/or modify
 * * it under the terms of the GNU General Public License version 2 and
 * * only version 2 as published by the Free Software Foundation.
 * *
 * * This program is distributed in the hope that it will be useful,
 * * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * * GNU General Public License for more details.
 * *
 * * You should have received a copy of the GNU General Public License
 * * along with this program; if not, write to the Free Software
 * * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
 *
 * Otherwise, the following license terms apply:
 *
 * * Redistribution and use in source and binary forms, with or without
 * * modification, are permitted provided that the following conditions
 * * are met:
 * * 1) Redistributions of source code must retain the above copyright
 * *    notice, this list of conditions and the following disclaimer.
 * * 2) Redistributions in binary form must reproduce the above copyright
 * *    notice, this list of conditions and the following disclaimer in the
 * *    documentation and/or other materials provided with the distribution.
 * * 3) Neither the name of Huawei nor the names of its contributors may
 * *    be used to endorse or promote products derived from this software
 * *    without specific prior written permission.
 *
 * * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef __NVA_PARTITION_H__
#define __NVA_PARTITION_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define NVA_IMG_PATH "/mnt/modem/modem_nv/image/"
#define NVA_UPDATE_PATH "/vendor/modem/modem_vendor/image/"
#define NVA_UPDATE_BASE_FILE "nva.bin"
#define NVA_PATH_LEN 128
#define NVA_MOUNT_PATH_LEN 64
#define NVA_PARTITION_MAGICNUM (0x6e764e56)
#define NVA_IMG_BIN 0x5a5a5a5a
#define NVA_IMG_BAK 0x7a7a7a7a
#define HASH_RESULT_LEN 32
#define NVA_FILE_NAME_MAX_LEN 32


/* nva file type in image partition */
enum nva_image_type {
    NVA_RW = 0,
    NVA_RWBK,
    NVA_FLG,
    NVA_HASH,
    NVA_IMGFILE_MAX
};

typedef struct {
    u32 magic_num;
    u32 version;
    u32 size;
    u32 id_sum;
    u32 reserve;
} nva_file_info_s;


struct nva_partition_info {
    char mountpoint[NVA_PATH_LEN];
    nva_file_info_s fileinfo;
};

struct nva_update_partition {
    char basefile[NVA_PATH_LEN]; /* nva.bin file name */
    char hashinfo[HASH_RESULT_LEN];
    struct nva_partition_info pinfo;
};

struct nva_img_file_info {
    char filename[NVA_PATH_LEN];
    char hashinfo[HASH_RESULT_LEN]; /* 写入nva之后计算的实时hash */
};

struct nva_img_partition {
    u32 fcnt;
    char hashinfo[HASH_RESULT_LEN];
    struct nva_img_file_info finfo[NVA_IMGFILE_MAX];
    struct nva_partition_info pinfo;
};

struct nva_fs {
    char mount_path[NVA_PATH_LEN];
    struct nva_update_partition update;
    struct nva_img_partition image;
};

u32 nvafs_init(struct nva_fs *nvafs);
u32 nva_load_update(void);
u32 nva_load_img(void);
u32 nva_img_flush(void);
int nva_get_file_len(s8 *file);
u32 nva_data_read(s8 *filename, s8 *buf, u32 offset, u32 length);
u32 nva_data_write(s8 *filename, s8 *buf, u32 length);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif // __NVA_PARTITION_H__
