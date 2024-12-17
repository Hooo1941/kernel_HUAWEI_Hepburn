/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2018. All rights reserved.
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
 *
 */

#ifndef __NV_PARTITION_H__
#define __NV_PARTITION_H__

#include "nv_msg.h"
#include "bsp_nvim.h"

#define NV_PARTITION_MAGICNUM (0x6e764e56)
#define NV_HEAD_MAGICNUM (0x224e4944)

#define BLK_MAX_NAME (64)

typedef struct _nv_wr_req {
    u32 itemid;
    u32 modemid;
    u32 offset;
    u8 *pdata;
    u32 size;
} nv_rdwr_req;

#define NV_MAX_MODEM_NUM 3
struct item_info {
    u32 itemid;             /* NV ID */
    u32 offset[NV_MAX_MODEM_NUM];          /* offset of modem0~2 */
    u16 length;             /* length of nv */
    u8  modem_num;          /* modem num */
    u8  priority;           /* write priority */
    u8  resume;             /* resume flag */
    u8  rsv[0x2];             /* reserve */
    u8  snid;               /* ID of section of this nv */
};

/* nv filesystem have rs,ro,rw,tag section, head is a special section
 * enum nv_section_type must be same order as enum nv_image_type
 */
enum nv_section_type {
    NV_SECTION_HD = 0,    /* nv head section */
    NV_SECTION_RS = 1,    /* nv resume section */
    NV_SECTION_RO = 2,    /* nv read only section */
    NV_SECTION_RW = 3,    /* nv read/write secion */
    NV_SECTION_TAG = 4,   /* nv tag section */
    NV_SECTION_END
};

/* nv file in image partition */
enum nv_image_type {
    NV_HD = 0,   /* nv data head file */
    NV_RS,
    NV_RO,
    NV_RW,
    NV_TAG,
    NV_RWBK,
    NV_CCM,
    NV_CCR,
    NV_FLG,
    NV_FTYRST,
    NV_IMGFILE_MAX
};

#define NV_IMGFILE_MAX_LEN 32

#define NV_SECTION_NAME_LEN 20
struct section_info {
    u8 id;
    u8 type;
    u8 rsv[0x2];
    u8 name[NV_SECTION_NAME_LEN];
    u32 itmcnt;    /* count of items section contains */
    u32 offset;
    u32 length;
};

struct nv_ctrl_info {
    u32  magicnum;
    u32  hdsize;                        /* head data size, include section and item index info */
    u8   version[NV_FILE_VER_LEN];                    /* head version */
    u8   modemnum;                      /* modem num */
    u8   crc_mark;
    u32  snidxoff;                      /* offset of section index */
    u32  sncnt;                         /* section cnt */
    u32  snidxsize;                     /* section index size */
    u32  itmidxoff;                     /* offset of item index */
    u32  itmcnt;                        /* item cnt */
    u32  itmidxsize;                    /* item index size */
    u8   rsv[0xC];
    u32  timetag[NV_TIME_TAG_LEN];                    /* time stamp */
    u32  product_version[NV_PRDT_VER_LEN];            /* product version */
};

#define NV_TAG_DATA_LEN 8
#define NV_TAG_TIME_LEN 4
struct tag_info {
    u32 data[NV_TAG_DATA_LEN];
    u32 time[NV_TAG_TIME_LEN];
};
#ifdef BSP_CONFIG_PHONE_TYPE
#define NV_ROOT_PATH "/mnt/modem/modem_nv/"

#else
#define NV_ROOT_PATH "/modem_nv/"
#endif
#define NV_PATH_LEN 64
#define NV_MOUNT_PATH_LEN 32

struct partition_info {
    char mountpoint[NV_PATH_LEN];
    u32 magicnum;
    u32 dtoff;    /* data offset */
    u32 size;    /* not include verify data size */
};

#define NV_IMAGE_MOUNTPOINT "image/"

#define NV_IMAGE_MAGICNUM (0x764e6e56)

struct img_file_info {
    u32 type;
    u32 magicnum;
    char filename[NV_PATH_LEN];
};

struct image_partition {
    u32 fcnt;
    struct img_file_info finfo[NV_IMGFILE_MAX];
    struct tag_info tag[0x2];
    struct partition_info pinfo;
};

#define NV_FS_VER_LEN 8
struct nvm_fs {
    char mount_path[NV_PATH_LEN];
    u32 sncnt;    /* section cnt */
    u32 itmcnt;   /* item cnt */
    u32 modemnum;
    u32 dtlen;    /* nv data length, include ctrl info, nv data, tag data */
    u32 version[NV_FS_VER_LEN];
    struct image_partition image;
};

enum {
    NV_BLK_BACKUP = 0,
    NV_BLK_FACTORY = 1,
    NV_BLK_PATCH = 2,
    NV_BLK_MAX
};

struct nv_rdblk_info {
    u32 magicnum;
    u32 partition;
    u32 offset;
    u32 size;
};

struct cust_info {
    u32 magicnum;
    u32 offset;
    u32 length;
};

typedef struct _nv_dload_head {
    struct cust_info nv_bin;   /* nv.bin文件信息 */
    struct cust_info xnv_map;  /* 双卡cust.xml文件信息 */
    struct cust_info xnv;      /* 双卡xnv.xml文件信息 */
    struct cust_info cust_map; /* 双卡xnv.xml map文件信息 */
    struct cust_info cust;     /* 双卡xnv.xml map文件信息 */
    u32 reserved[0x10];       /* NV支持的总modem数量减2 */
} nv_dload_head;

typedef struct _nv_dload_tail {
    u32 reserve;
    u32 upgrade_flag; /* NV_DLOAD_INVALID_FLAG, 已升级 */
} nv_dload_tail;

/* nv patch flag info */
struct nv_patch_flag_info {
    u32 magicnum;
    u32 offset;
    nv_dload_tail tail;
};

struct nv_patch_info_s {
    u8 patch_exist;    /* *<补丁是否存在 */
    u8 patch_status;   /* *<补丁的状态 */
};

struct nv_scblk_head {
    u32 magicnum;
    u32 size;
    u32 offset;
};

struct nvm_fs *nv_get_nvmfs(void);

struct image_partition *nv_get_image_partition(void);

u32 nv_set_patch_flag(struct nv_msg_data *msg_data, u32 *sync_flag);

u32 nv_blk_read(u32 partition, u8 *buf, u32 offset, u32 size);

u32 nv_blk_write(u32 partition, u8 *buf, u32 offset, u32 size);

struct file *nv_file_open(s8 *path, s32 flags, s32 file_mode);

s32 nv_file_write(struct file *fp, s8 *buf, u32 size);

void nv_file_close(struct file *fp);

s32 nv_file_read(struct file *fp, s8 *buf, u32 offset, u32 size);

s32 nv_file_access(s8 *file, s32 mode);

s32 nv_file_remove(s8 *file);

u32 nv_load_block(struct nv_msg_data *msg_data, u32 *sync_flag);

u32 nv_image_path_init(struct nvm_fs *nvmfs);

u32 nv_img_rmflg(void);

u32 nv_img_rwflush(u32 buf, u32 backup_type);

void nv_img_rmrw(void);

u32 nv_img_single_flush(u32 type, u32 backup_type);

u32 nv_img_tag_flush(u32 backup_type);

u32 nv_img_rm_tag(void);

u32 nv_img_ftyrst_flush(void);

u32 nv_img_flush_data(u32 backup_type);

void nv_img_rm_data(void);

u32 nv_img_flush_base(u32 backup_type);

u32 nv_img_flush_carrier(u32 backup_type);

void nv_img_rm_all(void);

u32 nv_bak_flush(u32 backup_type);

u32 nv_fac_flush(void);

u32 nv_nvmfs_init(void);

u32 nv_get_data_size(u64 dtaddr);

u32 nv_scblk_recv(struct nv_msg_data *msg_data, u32 *sync_flag);

#endif
