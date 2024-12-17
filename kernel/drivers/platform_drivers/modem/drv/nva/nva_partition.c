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

#include <linux/fs.h>
#include <linux/version.h>
#include <securec.h>
#include <crypto/hash.h>
#include <bsp_rfile.h>
#include <bsp_file_ops.h>
#include <product_config.h>
#include "nva_partition.h"
#include "nva_modem.h"

#undef THIS_MODU
#define THIS_MODU mod_nva

#define NVA_RW_FILE_MODE (0660)
#define NVA_RONLY_FILE_MODE  (0640)

#ifdef BSP_CONFIG_PHONE_TYPE
struct nva_fs *nva_get_nvafs(void)
{
    nva_ctrl_info_s *nvainfo = nva_get_nvainfo();

    return &nvainfo->nvafs;
}

s32 nva_file_remove(s8 *file)
{
    return do_unlinkat(AT_FDCWD, getname(file));
}

struct file *nva_file_open(s8 *path, s32 flags, s32 file_mode)
{
    return filp_open(path, flags, file_mode);
}

s32 nva_file_read(struct file *fp, s8 *buf, u32 offset, u32 size)
{
    s32 ret;
    loff_t pos = offset;

    ret = kernel_read(fp, (void *)buf, size, &pos);
    return ret;
}

s32 nva_file_write(struct file *fp, s8 *buf, u32 size)
{
    s32 ret;
    loff_t pos = 0;

    ret = kernel_write(fp, buf, size, &pos);

    (void)vfs_fsync(fp, 0);

    return ret;
}

s32 nva_file_length(struct file *fp)
{
    int ret;
    int seek;

    ret = vfs_llseek(fp, 0, RFILE_SEEK_END);
    if (ret < 0) {
        nva_printf("seek end file fail:0x%x\n", ret);
        return ret;
    }

    seek = vfs_llseek(fp, 0, RFILE_SEEK_CUR);

    ret = vfs_llseek(fp, 0, RFILE_SEEK_SET);
    if (ret < 0) {
        nva_printf("seek set file fail:0x%x\n", ret);
        return ret;
    }

    return seek;
}

void nva_file_close(struct file *fp)
{
    s32 ret;

    ret = filp_close(fp, NULL);
    if (ret) {
        nva_printf("close fail, ret:%d\n", ret);
    }
}

int nva_get_file_len(s8 *file)
{
    int length;
    struct file *fp;
    fp = nva_file_open(file, (s32)RFILE_RDONLY, (s32)NVA_RW_FILE_MODE);
    if (IS_ERR(fp)) {
        nva_printf("open %s file fail\n", file);
        return -1;
    }
    length = nva_file_length(fp);
    nva_file_close(fp);
    return length;
}

u32 nva_data_read(s8 *filename, s8 *buf, u32 offset, u32 length)
{
    u32 ret;
    struct file *fp;
    fp = nva_file_open(filename, (s32)RFILE_RDONLY, (s32)NVA_RONLY_FILE_MODE);
    if (IS_ERR(fp)) {
        nva_printf("open %s file fail\n", filename);
        return -1;
    }

    ret = (u32)nva_file_read(fp, buf, offset, length);
    nva_file_close(fp);
    if (ret != length) {
        nva_printf("read %s file 0x%x length fail, ret:0x%x\n", filename, length, ret);
        return ret;
    }

    nva_printf("read %s file success\n", filename);
    return NVA_OK;
}

u32 nva_data_write(s8 *filename, s8 *buf, u32 length)
{
    u32 ret;
    s32 flags = RFILE_CREAT | RFILE_TRUNC | RFILE_RDWR;
    struct file *fp;
    fp = nva_file_open((s8 *)filename, flags, NVA_RW_FILE_MODE);
    if (IS_ERR(fp)) {
        nva_printf("open %s file fail\n", filename);
        return -1;
    }
    ret = (u32)nva_file_write(fp, buf, length);
    nva_file_close(fp);
    if (ret != length) {
        nva_printf("write nva file:%s err:%d\n", filename, ret);
        return ret;
    }
    nva_printf("write %s image file success\n", filename);
    return NVA_OK;
}

u32 nva_data_load(s8 *filename, u8 *buf, u32 bufsize)
{
    u32 ret;
    int length;

    length = nva_get_file_len(filename);
    if (length < 0) {
        nva_printf("get %s file length err\n", filename);
        return (u32)length;
    }

    if (length > (int)bufsize) {
        nva_printf("load %s file length oversize err\n", filename);
        return NVA_ERROR;
    }

    ret = nva_data_read(filename, (s8 *)buf, 0, (u32)length);
    if (ret != NVA_OK) {
        nva_printf("load %s file read err\n", filename);
        return ret;
    }

    return NVA_OK;
}

s32 nva_update_path_init(struct nva_fs *nvafs)
{
    s32 ret;
    struct nva_update_partition *update = &nvafs->update;
    char *mountpoint = update->pinfo.mountpoint;
    char *basefile = update->basefile;

    ret = strncpy_s(mountpoint, NVA_PATH_LEN, (char *)NVA_UPDATE_PATH,
        strnlen((char *)NVA_UPDATE_PATH, NVA_MOUNT_PATH_LEN));
    if (ret) {
        nva_printf("copy update mountpoint fail, ret:0x%x\n", ret);
        return ret;
    }

    ret = strncpy_s(basefile, NVA_PATH_LEN, mountpoint, strnlen(mountpoint, NVA_MOUNT_PATH_LEN));
    if (ret) {
        nva_printf("copy update base file fail, ret:0x%x\n", ret);
        return ret;
    }

    ret = strncat_s(basefile, NVA_PATH_LEN, (char *)NVA_UPDATE_BASE_FILE,
        strnlen((char *)NVA_UPDATE_BASE_FILE, NVA_MOUNT_PATH_LEN));
    if (ret) {
        nva_printf("strncat update base file fail, ret:0x%x\n", ret);
        return ret;
    }

    nva_printf("update base file:%s\n", basefile);

    return NVA_OK;
}

u32 nva_update_partition_init(struct nva_fs *nvafs)
{
    u32 ret, offset;
    struct nva_update_partition *update = &nvafs->update;
    nva_file_info_s *fileinfo = &nvafs->update.pinfo.fileinfo;

    ret = nva_data_read((s8 *)update->basefile, (s8 *)fileinfo, 0, sizeof(nva_file_info_s));
    if (ret != NVA_OK) {
        nva_printf("read update ctrl info err\n");
        return ret;
    }

    if (fileinfo->magic_num != NVA_FILE_EXIST) {
        nva_printf("invalid update file magicnum:0x%x\n", fileinfo->magic_num);
        return NVA_FILE_NOT_EXIST;
    }
    offset = 0 + fileinfo->size - HASH_RESULT_LEN * 0x2; /* 尾部预留2个hash位置 */
    ret = nva_data_read((s8 *)update->basefile, (s8 *)update->hashinfo, offset, HASH_RESULT_LEN);
    if (ret != NVA_OK) {
        nva_printf("read update hash info err\n");
        return ret;
    }
    nva_printf("update partition init success\n");
    return NVA_OK;
}

char g_nva_img_file[NVA_IMGFILE_MAX][NVA_FILE_NAME_MAX_LEN] = {
    "nva.bin",
    "nva.bak",
    "nva_flag.bin",
    "nva_hash.bin"
};

s32 nva_image_path_init(struct nva_fs *nvafs)
{
    s32 ret;
    u32 i;
    struct nva_img_partition *image = &nvafs->image;
    char *mountpoint = image->pinfo.mountpoint;
    char *filename = NULL;

    ret = strncpy_s(mountpoint, NVA_PATH_LEN, (char *)NVA_IMG_PATH,
        strnlen((char *)NVA_IMG_PATH, NVA_MOUNT_PATH_LEN));
    if (ret) {
        nva_printf("copy image mountpoint fail, ret:0x%x\n", ret);
        return ret;
    }

    nva_printf("nva image mountpoint:%s\n", mountpoint);
    for (i = 0; i < NVA_IMGFILE_MAX; i++) {
        filename = image->finfo[i].filename;
        ret = strncpy_s(filename, NVA_PATH_LEN, mountpoint, strnlen(mountpoint, NVA_MOUNT_PATH_LEN));
        if (ret) {
            nva_printf("copy image %d filename fail, ret:0x%x\n", i, ret);
            return ret;
        }

        ret = strncat_s(filename, NVA_PATH_LEN, g_nva_img_file[i],
            strnlen(g_nva_img_file[i], NVA_MOUNT_PATH_LEN));
        if (ret) {
            nva_printf("strncat image %d filename fail, ret:0x%x\n", i, ret);
            return ret;
        }
        nva_printf("nva %d filename:%s\n", i, filename);
    }

    image->fcnt = NVA_IMGFILE_MAX;
    return NVA_OK;
}

static u32 nvafs_path_init(struct nva_fs *nvafs)
{
    u32 ret;

    ret = (u32)nva_update_path_init(nvafs);
    if (ret != NVA_OK) {
        nva_printf("nva update path init fail, ret:0x%x\n", ret);
        return ret;
    }

    ret = (u32)nva_image_path_init(nvafs);
    if (ret != NVA_OK) {
        nva_printf("nva image path init fail, ret:0x%x\n", ret);
        return ret;
    }

    return NVA_OK;
}

/* ***********************************************************************
 函 数 名  : nva_hash_sha256_calc
 功能描述  : 计算sha256 hash值
************************************************************************ */
u32 nva_hash_sha256_calc(const u8 *data, u32 len, u8 *hash)
{
    u32 ret = NVA_OK;
    struct crypto_shash *tfm = NULL;
    struct shash_desc *desc = NULL;
    size_t desc_size;
    int flag;

    tfm = crypto_alloc_shash("sha256", 0, 0);
    if ((tfm == NULL) || (IS_ERR(tfm))) {
        nva_printf("crypto_alloc_shash failed!\n");
        return NVA_ERROR;
    }

    desc_size = crypto_shash_descsize(tfm) + sizeof(struct shash_desc);
    desc = (struct shash_desc *)vmalloc(desc_size);
    if (desc == NULL) {
        nva_printf("vmalloc desc failed!\n");
        crypto_free_shash(tfm);
        return NVA_ERROR;
    }

    desc->tfm = tfm;
    flag = crypto_shash_init(desc);
    if (flag < 0) {
        nva_printf("crypto_shash_init failed!ret=0x%x\n", ret);
        goto error;
    }

    ret = crypto_shash_digest(desc, data, len, hash);
    if (ret) {
        nva_printf("crypto_shash_update failed!ret=0x%x \n", ret);
        goto error;
    }

error:
    if (desc != NULL) {
        if (desc->tfm) {
            crypto_free_shash(desc->tfm);
        }
        vfree(desc);
    }
    return ret;
}

static u32 nva_hash_check(char *hashinfo, u32 len)
{
    nva_ctrl_info_s *nvainfo = nva_get_nvainfo();
    nva_file_info_s *fileinfo = (nva_file_info_s *)nvainfo->load_addr;
    u8 hash_result[HASH_RESULT_LEN];
    u32 ret;
    if (len != HASH_RESULT_LEN) {
        nva_printf("err len=0x%x \n", len);
        return NVA_ERROR;
    }
    ret = nva_hash_sha256_calc(nvainfo->load_addr, fileinfo->size - 0x2 * HASH_RESULT_LEN, hash_result);
    if (ret != NVA_OK) {
        nva_printf("calc hash failed!ret=0x%x \n", ret);
        return NVA_ERROR;
    }
    ret = (u32)memcmp((void *)hash_result, (void *)hashinfo, HASH_RESULT_LEN);
    if (ret) {
        nva_printf("hash check fail\n");
        return NVA_ERROR;
    }
    return NVA_OK;
}
u32 nva_img_partition_init(struct nva_fs *nvafs)
{
    u32 ret, offset, i;
    struct nva_img_partition *image = &nvafs->image;
    nva_file_info_s *fileinfo = &image->pinfo.fileinfo;

    ret = nva_data_read((s8 *)image->finfo[NVA_RW].filename, (s8 *)&image->pinfo.fileinfo, 0,
        sizeof(image->pinfo.fileinfo));
    if (ret != NVA_OK) {
        nva_printf("image init read ctrl info fail\n");
        return ret;
    }

    offset = fileinfo->size - HASH_RESULT_LEN * 0x2; /* 尾部预留2个hash位置 */
    for (i = 0; i <= NVA_RWBK; i++) {
        ret = nva_data_read((s8 *)image->finfo[i].filename, (s8 *)image->finfo[i].hashinfo,
            offset, HASH_RESULT_LEN);
        if (ret != NVA_OK) {
            nva_printf("read update hash info err\n");
            return ret;
        }
    }
    ret = nva_data_read((s8 *)image->finfo[NVA_HASH].filename, (s8 *)image->finfo[NVA_HASH].hashinfo,
        0, HASH_RESULT_LEN);
    if (ret != NVA_OK) {
        nva_printf("read update old hash info err\n");
        return ret;
    }
    nva_printf("nv image partition init success\n");
    return NVA_OK;
}

u32 nvafs_init(struct nva_fs *nvafs)
{
    u32 ret;
    ret = nvafs_path_init(nvafs);
    if (ret) {
        nva_printf("nvafs path init fail, ret:0x%x\n", ret);
        return (u32)ret;
    }
    ret = nva_update_partition_init(nvafs);
    if (ret) {
        nva_printf("update partition init fail, ret:0x%x\n", ret);
        return (u32)ret;
    }
    ret = nva_img_partition_init(nvafs);
    if (ret) {
        nva_printf("image partition init fail, ret:0x%x\n", ret);
    }
    return NVA_OK;
}


/* load nv section from image and check hmac */
static u32 nva_img_load_verify(u32 type)
{
    u32 ret;
    nva_ctrl_info_s *nvainfo = nva_get_nvainfo();
    struct nva_fs *nvafs = &nvainfo->nvafs;
    struct nva_img_partition *image = &nvafs->image;
    struct nva_img_file_info *finfo = image->finfo;

    if (type > NVA_RWBK) {
        nva_printf("image load and check invalid %d nv section\n", type);
        return NVA_ERROR;
    }

    ret = nva_data_read((s8 *)finfo[type].filename, (s8 *)(nvainfo->load_addr), 0, image->pinfo.fileinfo.size);
    if (ret != NVA_OK) {
        nva_printf("[load]:read %s file err\n", finfo[type].filename);
        return ret;
    }

    nva_printf("load %s file success\n", finfo[type].filename);

    ret = nva_hash_check(finfo[type].hashinfo, HASH_RESULT_LEN);
    if (ret != NVA_OK) {
        nva_printf("load check nva  %d hash err\n", type);
        return ret;
    }

    return NVA_OK;
}

static u32 nva_img_flag_read(void)
{
    u32 ret;
    u32 flag = 0;
    nva_ctrl_info_s *nvainfo = nva_get_nvainfo();
    struct nva_fs *nvafs = &nvainfo->nvafs;
    struct nva_img_partition *image = &nvafs->image;
    char *filename = image->finfo[NVA_FLG].filename;

    ret = nva_data_read((s8 *)filename, (s8 *)&flag, 0, sizeof(u32));
    if (ret != NVA_OK) {
        nva_printf("get image bin or bak flag err\n");
        return ret;
    }

    return flag;
}

static u32 nva_img_flag_write(u32 flag)
{
    nva_ctrl_info_s *nvainfo = nva_get_nvainfo();
    struct nva_fs *nvafs = &nvainfo->nvafs;
    struct nva_img_partition *image = &nvafs->image;
    char *filename = image->finfo[NVA_FLG].filename;

    return nva_data_write((s8 *)filename, (s8 *)&flag, sizeof(u32));
}

void nva_img_single_rm(u32 type)
{
    u32 ret;
    nva_ctrl_info_s *nvainfo = nva_get_nvainfo();
    struct nva_fs *nvafs = &nvainfo->nvafs;
    struct nva_img_partition *image = &nvafs->image;

    if (type >= NVA_IMGFILE_MAX) {
        nva_printf("remove invalid %d image file\n", type);
        return;
    }

    ret = (u32)nva_file_remove((s8 *)image->finfo[type].filename);
    if (ret) {
        nva_printf("single remove image %d file err\n", type);
    }
}

static u32 nva_update_img_hash(void)
{
    u32 ret;
    nva_ctrl_info_s *nvainfo = nva_get_nvainfo();
    struct nva_fs *nvafs = &nvainfo->nvafs;
    struct nva_img_partition *image = &nvafs->image;
    nva_file_info_s *fileinfo = &image->pinfo.fileinfo;
    u8 hash_result[HASH_RESULT_LEN];
    void *addr = NULL;
    u64 offset;
    offset = fileinfo->size - HASH_RESULT_LEN * 0x2; /* 尾部预留2个hash位置 */
    addr = (void *)(uintptr_t)((uintptr_t)(nvainfo->load_addr) + offset);
    ret = nva_hash_sha256_calc(nvainfo->load_addr, fileinfo->size - 0x2 * HASH_RESULT_LEN, hash_result);
    if (ret != NVA_OK) {
        nva_printf("calc hash failed!ret=0x%x \n", ret);
        return NVA_ERROR;
    }
    if (memcpy_s(addr, HASH_RESULT_LEN, hash_result, sizeof(hash_result))) {
        nva_printf("memcpy hash err\n");
        return NVA_ERROR;
    }
    return NVA_OK;
}

static u32 nva_save_update_hash(void)
{
    u32 ret;
    nva_ctrl_info_s *nvainfo = nva_get_nvainfo();
    struct nva_fs *nvafs = &nvainfo->nvafs;
    struct nva_img_partition *image = &nvafs->image;
    struct nva_update_partition *update = &nvafs->update;
    char *filename = NULL;
    filename = image->finfo[NVA_HASH].filename;
    ret = nva_data_write(filename, update->hashinfo, HASH_RESULT_LEN);
    if (ret != NVA_OK) {
        nva_printf("write hash err\n");
        return ret;
    }
    return NVA_OK;
}

static u32 nva_write_back(void)
{
    u32 ret;
    nva_ctrl_info_s *nvainfo = nva_get_nvainfo();
    struct nva_fs *nvafs = &nvainfo->nvafs;
    struct nva_img_partition *image = &nvafs->image;
    struct nva_update_partition *update = &nvafs->update;
    char *filename = NULL;
    if (nva_img_flag_read() == NVA_IMG_BIN) {
        filename = image->finfo[NVA_RWBK].filename;
        ret = nva_data_write(filename, nvainfo->load_addr, update->pinfo.fileinfo.size);
        if (ret != NVA_OK) {
            nva_printf("write image rw bak err\n");
            return ret;
        }

        ret = nva_img_flag_write(NVA_IMG_BAK);
        if (ret != NVA_OK) {
            nva_printf("write image flag bak err\n");
            return ret;
        }
    } else {
        filename = image->finfo[NVA_RW].filename;
        ret = nva_data_write(filename, nvainfo->load_addr, update->pinfo.fileinfo.size);
        if (ret != NVA_OK) {
            nva_printf("write image rw bin err\n");
            return ret;
        }

        ret = nva_img_flag_write(NVA_IMG_BIN);
        if (ret != NVA_OK) {
            nva_printf("write image flag bin err\n");
            return ret;
        }
    }
    filename = image->finfo[NVA_HASH].filename;
    ret = nva_data_write(filename, update->hashinfo, HASH_RESULT_LEN);
    if (ret != NVA_OK) {
        nva_printf("write hash err\n");
        return ret;
    }
    return NVA_OK;
}

u32 nva_img_flush(void)
{
    u32 ret;
    ret = nva_update_img_hash();
    if (ret) {
        nva_printf("update img hash err\n");
        return NVA_ERROR;
    }
    ret = nva_write_back();
    if (ret) {
        nva_printf("write back err\n");
        return NVA_ERROR;
    }
    return NVA_OK;
}

/* load rw(rwbak) file from image to base + rw section offset */
u32 nva_load_img(void)
{
    u32 ret, flag;

    flag = nva_img_flag_read();
    if (flag == NVA_IMG_BAK) {
        ret = nva_img_load_verify(NVA_RWBK);
        if (ret == NVA_OK) {
            return NVA_OK;
        }

        nva_img_single_rm(NVA_RWBK);
        nva_printf("load rwbak err, start load rwbin\n");
        ret = nva_img_load_verify(NVA_RW);
        if (ret != NVA_OK) {
            nva_printf("nv rw bin load err\n");
            return ret;
        }

        ret = nva_img_flag_write(NVA_IMG_BIN);
        if (ret != NVA_OK) {
            nva_printf("rwload set bin flag err\n");
            return ret;
        }
    } else {
        ret = nva_img_load_verify(NVA_RW);
        if (ret == NVA_OK) {
            return NVA_OK;
        }

        nva_img_single_rm(NVA_RW);
        nva_printf("load rw bin err, start load rwbak\n");
        ret = nva_img_load_verify(NVA_RWBK);
        if (ret != NVA_OK) {
            nva_printf("nv rw bak load err\n");
            return ret;
        }

        ret = nva_img_flag_write(NVA_IMG_BAK);
        if (ret != NVA_OK) {
            nva_printf("rwload set bak flag err\n");
            return ret;
        }
    }

    return NVA_OK;
}

u32 nva_load_update(void)
{
    u32 ret;
    nva_ctrl_info_s *nvainfo = nva_get_nvainfo();
    struct nva_fs *nvafs = &nvainfo->nvafs;
    struct nva_update_partition *update = &nvafs->update;

    ret = nva_data_read((s8 *)update->basefile, (s8 *)(nvainfo->load_addr), 0, update->pinfo.fileinfo.size);
    if (ret != NVA_OK) {
        nva_printf("[load]:read %s file err\n", update->basefile);
        return ret;
    }

    nva_printf("load %s file success\n", update->basefile);

    ret = nva_hash_check(update->hashinfo, HASH_RESULT_LEN);
    if (ret != NVA_OK) {
        nva_printf("load check nva hash err\n");
        return ret;
    }
    ret = nva_save_update_hash();
    if (ret != NVA_OK) {
        nva_printf("save update hash err\n");
        return ret;
    }
    ret = nva_write_back();
    if (ret != NVA_OK) {
        nva_printf("write back err\n");
        return ret;
    }
    return NVA_OK;
}
#endif
