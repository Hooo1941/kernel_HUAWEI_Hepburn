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

#include <linux/kernel.h>
#include <securec.h>
#include <bsp_rfile.h>
#include <bsp_nvim.h>
#include "nv_comm.h"
#include "nv_partition.h"

char g_img_file[NV_IMGFILE_MAX][NV_IMGFILE_MAX_LEN] = {
    "nv_ctrl.bin",
    "nv_resume.bin",
    "nv_rdonly.bin",
    "nv_rdwr.bin",
    "tag.bin",
    "nv_rdwr.bk",
    "carrier/nv_comm.bin",
    "carrier/nv_carrier.bin",
    "nv_flag.bin",
    "nv_ftyrst.bin"
};

u32 nv_image_path_init(struct nvm_fs *nvmfs)
{
    u32 ret;
    u32 i;
    struct image_partition *image = &nvmfs->image;
    char *mountpoint = image->pinfo.mountpoint;
    char *filename = NULL;

    ret = strncpy_s(mountpoint, NV_PATH_LEN, nvmfs->mount_path, \
        strnlen((char *)nvmfs->mount_path, NV_MOUNT_PATH_LEN));
    if (ret) {
        nv_printf("copy image mountpoint fail, ret:0x%x\n", ret);
        return ret;
    }

    ret = strncat_s(mountpoint, NV_PATH_LEN, (char *)NV_IMAGE_MOUNTPOINT, \
        strlen((char *)NV_IMAGE_MOUNTPOINT));
    if (ret) {
        nv_printf("strncat image mountpoint fail, ret:0x%x\n", ret);
        return ret;
    }

    nv_printf("nv image mountpoint:%s\n", mountpoint);
    for (i = 0; i < NV_IMGFILE_MAX; i++) {
        image->finfo[i].type = i;
        filename = image->finfo[i].filename;
        ret = strncpy_s(filename, NV_PATH_LEN, mountpoint, strnlen(mountpoint, NV_MOUNT_PATH_LEN));
        if (ret) {
            nv_printf("copy image %d filename fail, ret:0x%x\n", i, ret);
            return ret;
        }

        ret = strncat_s(filename, NV_PATH_LEN, g_img_file[i], strnlen(g_img_file[i], NV_MOUNT_PATH_LEN));
        if (ret) {
            nv_printf("strncat image %d filename fail, ret:0x%x\n", i, ret);
            return ret;
        }
        nv_debug_printf(NV_INFO, "nv %d filename:%s\n", i, filename);
    }

    image->fcnt = NV_IMGFILE_MAX;
    return NV_OK;
}

static u32 nv_img_write(s8 *filename, s8 *buf, u32 length)
{
    nv_printf("nv file write should not in\n");
    return NV_ERROR;
}

static u32 nv_img_get_file_info(u32 filetype, char **filename, s8 **buf, u32 *length)
{
    u32 sntype = 0;
    u32 offset = 0;
    struct section_info *sninfo = NULL;
    u64 dtaddr = nv_get_mem_addr();
    struct nv_ctrl_info *ctrl_info = nv_get_ctrl_info();
    struct image_partition *image = nv_get_image_partition();

    if ((dtaddr == 0) || (ctrl_info == NULL)) {
        nv_printf("nv data memory init err\n");
        return BSP_ERR_NV_MEM_INIT_FAIL;
    }

    sninfo = nv_get_sninfo();
    switch (filetype) {
        case NV_HD:
            sntype = NV_SECTION_HD;
            *buf = (s8 *)((uintptr_t)dtaddr + offset);
            *length = ctrl_info->hdsize;
            *filename = image->finfo[filetype].filename;
            break;
        case NV_RS:
        case NV_RO:
        case NV_RW:
        case NV_TAG:
            sntype = filetype;
            offset = sninfo[sntype - 1].offset;
            *buf = (s8 *)((uintptr_t)dtaddr + offset);
            *length = sninfo[sntype - 1].length;
            *filename = image->finfo[filetype].filename;
            break;
        case NV_RWBK:
            sntype = NV_SECTION_RW;
            offset = sninfo[sntype - 1].offset;
            *buf = (s8 *)((uintptr_t)dtaddr + offset);
            *length = sninfo[sntype - 1].length;
            *filename = image->finfo[filetype].filename;
            break;
        default:
            return BSP_ERR_NV_NO_FILE;
    }

    return NV_OK;
}

/* write section data to each file */
static u32 nv_img_snwrite(u32 filetype, u32 backup_type)
{
    u32 ret;
    u32 length;
    s8 *buf = NULL;
    char *filename = NULL;
    struct nv_backup_cust_cb *cb = NULL;

    ret = nv_img_get_file_info(filetype, &filename, &buf, &length);
    if (ret != NV_OK) {
        nv_printf("get nv file info err, ret=%d\n", ret);
        return ret;
    }

    if (backup_type & NV_BACKUP_BASE_MASK) {
        ret = nv_img_write(filename, buf, length);
        if (ret != NV_OK) {
            nv_printf("write nv file:%s err\n", filename);
            return ret;
        }
    }

    cb = nv_get_backup_cust_cb();
    if ((backup_type & NV_BACKUP_BAK_MASK) && (cb != NULL) && (cb->img_write != NULL)) {
        ret = cb->img_write(g_img_file[filetype], buf, length);
        if (ret != NV_OK) {
            nv_printf("nv_get_backup_cust_cb write nv file:%s err\n", g_img_file[filetype]);
            return ret;
        }
    }

    return NV_OK;
}

static u32 nv_img_single_rm(u32 type)
{
    u32 ret;
    struct image_partition *image = nv_get_image_partition();

    if (type >= NV_IMGFILE_MAX) {
        nv_printf("rm invalid %d image file type\n", type);
        return BSP_ERR_NV_INVALID_PARAM;
    }

    ret = (u32)nv_file_remove((s8 *)image->finfo[type].filename);
    if (ret) {
        nv_printf("remove image %s file err,ret:%d\n", image->finfo[type].filename, ret);
    }

    return ret;
}

static u32 nv_img_flag_read(void)
{
    u32 ret = -1;
    u32 flag = 0;
    struct file *fp;
    struct image_partition *image = nv_get_image_partition();
    char *filename = image->finfo[NV_FLG].filename;

    fp = nv_file_open((s8 *)filename, RFILE_RDONLY, 0640); /* 0640:open mode read only */
    if (IS_ERR(fp)) {
        nv_printf("open %s rd file fail ret=0x%lx\n", filename, PTR_ERR((const void *)fp));
        return ret;
    }

    ret = (u32)nv_file_read(fp, (s8 *)&flag, 0, sizeof(u32));
    nv_file_close(fp);
    if (ret != sizeof(u32)) {
        nv_printf("read %s file fail\n", filename);
        return ret;
    }

    return flag;
}

static u32 nv_img_flag_write(u32 flag, u32 backup_type)
{
    u32 ret;
    struct image_partition *image = nv_get_image_partition();
    char *filename = image->finfo[NV_FLG].filename;
    struct nv_backup_cust_cb *cb = NULL;

    if (backup_type & NV_BACKUP_BASE_MASK) {
        ret = nv_img_write((s8 *)filename, (s8 *)&flag, sizeof(u32));
        if (ret != NV_OK) {
            nv_printf("write %s file fail\n", filename);
            return ret;
        }
    }

    cb = nv_get_backup_cust_cb();
    if ((backup_type & NV_BACKUP_BAK_MASK) && (cb != NULL) && (cb->img_write != NULL)) {
        ret = cb->img_write((s8 *)g_img_file[NV_FLG], (s8 *)&flag, sizeof(u32));
        if (ret != NV_OK) {
            nv_printf("nv_get_backup_cust_cb write %s file fail\n", filename);
            return ret;
        }
    }

    return NV_OK;
}

static u32 nv_img_ftyrst_write(u32 flag)
{
    u32 ret;
    struct image_partition *image = nv_get_image_partition();
    char *filename = image->finfo[NV_FTYRST].filename;

    ret = nv_img_write((s8 *)filename, (s8 *)&flag, sizeof(u32));
    if (ret != NV_OK) {
        nv_printf("write %s file fail\n", filename);
        return ret;
    }

    return NV_OK;
}


u32 nv_img_rmflg(void)
{
    return nv_img_single_rm(NV_FLG);
}

static u32 nv_img_rwwrite(u32 backup_type)
{
    u32 ret;

    if (nv_img_flag_read() == NVM_IMG_BIN) {
        ret = nv_img_snwrite(NV_RWBK, backup_type);
        if (ret != NV_OK) {
            nv_printf("write image rw bak err\n");
            return ret;
        }

        ret = nv_img_flag_write(NVM_IMG_BAK, backup_type);
        if (ret != NV_OK) {
            nv_printf("write image flag bak err\n");
            return ret;
        }
    } else {
        ret = nv_img_snwrite(NV_RW, backup_type);
        if (ret != NV_OK) {
            nv_printf("write image rw bin err\n");
            return ret;
        }

        ret = nv_img_flag_write(NVM_IMG_BIN, backup_type);
        if (ret != NV_OK) {
            nv_printf("write image flag bin err\n");
            return ret;
        }
    }

    return NV_OK;
}

static u32 nv_img_rwbuf_write(u32 type, u32 buf, u32 backup_type)
{
    u32 ret, sntype, length;
    s8 *virbuf = NULL;
    s8 *filename = NULL;
    u64 dtaddr = nv_get_mem_addr();
    struct section_info *sninfo = nv_get_sninfo();
    struct image_partition *image = nv_get_image_partition();
    struct nvm_info *info = nv_get_nvminfo();
    struct nv_backup_cust_cb *cb = NULL;

    if ((dtaddr == 0) || (sninfo == NULL)) {
        nv_printf("nv data memory init err\n");
        return BSP_ERR_NV_MEM_INIT_FAIL;
    }

    if ((type != NV_RW) && (type != NV_RWBK)) {
        nv_printf("invalid rwbuf file type");
        return BSP_ERR_NV_INVALID_PARAM;
    }

    sntype = NV_SECTION_RW;
    length = sninfo[sntype - 1].length;
    filename = image->finfo[type].filename;
    virbuf = (s8 *)(uintptr_t)((unsigned long)buf - info->paddr + info->dtaddr);

    nv_debug_printf(NV_INFO, "virbuf:0x%llx, phy buf:0x%x\n", (u64)(uintptr_t)virbuf, buf);
    if (backup_type & NV_BACKUP_BASE_MASK) {
        ret = nv_img_write(filename, virbuf, length);
        if (ret != NV_OK) {
            nv_printf("write rwbuf nv file:%s err\n", filename);
            return ret;
        }
    }

    cb = nv_get_backup_cust_cb();
    if ((backup_type & NV_BACKUP_BAK_MASK) && (cb != NULL) && (cb->img_write != NULL)) {
        ret = cb->img_write(g_img_file[type], virbuf, length);
        if (ret != NV_OK) {
            nv_printf("nv_get_backup_cust_cb write rwbuf nv file:%s err\n", filename);
            return ret;
        }
    }

    return NV_OK;
}

u32 nv_img_rwflush(u32 buf, u32 backup_type)
{
    u32 ret;

    if (nv_img_flag_read() == NVM_IMG_BIN) {
        ret = nv_img_rwbuf_write(NV_RWBK, buf, backup_type);
        if (ret != NV_OK) {
            nv_printf("write image rw bak err\n");
            return ret;
        }

        ret = nv_img_flag_write(NVM_IMG_BAK, backup_type);
        if (ret != NV_OK) {
            nv_printf("write image flag bak err\n");
            return ret;
        }
    } else {
        ret = nv_img_rwbuf_write(NV_RW, buf, backup_type);
        if (ret != NV_OK) {
            nv_printf("write image rw bin err\n");
            return ret;
        }

        ret = nv_img_flag_write(NVM_IMG_BIN, backup_type);
        if (ret != NV_OK) {
            nv_printf("write image flag bin err\n");
            return ret;
        }
    }

    return NV_OK;
}

/* flush section data to image file */
u32 nv_img_single_flush(u32 type, u32 backup_type)
{
    u32 ret = 0;

    switch (type) {
        case NV_HD:
        case NV_RS:
        case NV_RO:
        case NV_TAG:
            ret = nv_img_snwrite(type, backup_type);
            break;
        case NV_RW:
        case NV_RWBK:
            ret = nv_img_rwwrite(backup_type);
            break;
        default:
            nv_printf("nv flush invalid img type\n");
            return BSP_ERR_NV_NOT_SUPPORT_ERR;
    }

    if (ret != NV_OK) {
        nv_printf("nv flush %d img err\n", type);
        return ret;
    }

    return NV_OK;
}

u32 nv_img_tag_flush(u32 backup_type)
{
    u32 ret;

    ret = nv_img_single_flush(NV_TAG, backup_type);
    if (ret != NV_OK) {
        nv_printf("nv img tag file flush err\n");
        return ret;
    }

    return NV_OK;
}

u32 nv_img_rm_tag(void)
{
    s32 ret;
    struct image_partition *image = nv_get_image_partition();

    ret = nv_file_remove((s8 *)image->finfo[NV_TAG].filename);
    if (ret) {
        nv_printf("remove image tag file err,ret:%d\n", ret);
    }

    return ret;
}

#define NV_NEED_FTYRST (0x76566e4e)

u32 nv_img_ftyrst_flush(void)
{
    u32 ret;

    ret = nv_img_ftyrst_write(NV_NEED_FTYRST);
    if (ret != NV_OK) {
        nv_printf("nv img tag file flush err\n");
        return ret;
    }

    return NV_OK;
}

/* flush nv data to image not include tag section */
u32 nv_img_flush_data(u32 backup_type)
{
    u32 ret = 0;
    u32 i;
    struct image_partition *image = nv_get_image_partition();

    for (i = 0; i <= NV_RW; i++) {
        ret = nv_img_single_flush(i, backup_type);
        if (ret != NV_OK) {
            nv_printf("nv img file:%s write err\n", image->finfo[i].filename);
            return ret;
        }

        nv_printf("nv img file:%s write success\n", image->finfo[i].filename);
    }

    return NV_OK;
}

u32 nv_img_flush_base(u32 backup_type)
{
    u32 ret = 0;
    u32 i;
    struct image_partition *image = nv_get_image_partition();

    for (i = 0; i <= NV_TAG; i++) {
        ret = nv_img_single_flush(i, backup_type);
        if (ret != NV_OK) {
            nv_printf("nv img file:%s write err\n", image->finfo[i].filename);
            return ret;
        }

        nv_printf("nv img file:%s write success\n", image->finfo[i].filename);
    }

    return NV_OK;
}

u32 nv_img_flush_carrier(u32 backup_type)
{
    u32 i, ret, length;
    u64 mbnaddr = nv_get_mbnaddr();
    struct section_info *sninfo = NULL;
    struct nv_ctrl_info *ctrl_info = (struct nv_ctrl_info *)(uintptr_t)mbnaddr;
    struct image_partition *image = nv_get_image_partition();
    struct nv_backup_cust_cb *cb = NULL;

    if (ctrl_info->magicnum != NV_HEAD_MAGICNUM) {
        nv_printf("nv carrier data null, no need flush\n");
        return NV_OK;
    }

    sninfo = (struct section_info *)((uintptr_t)ctrl_info + ctrl_info->snidxoff);

    length = ctrl_info->hdsize;
    for (i = 0; i < ctrl_info->sncnt; i++) {
        length += sninfo[i].length;
    }

    if (length > NV_MBN_SIZE) {
        nv_printf("nv carrier data over size err\n");
        return BSP_ERR_NV_OVER_MEM_ERR;
    }

    if (backup_type & NV_BACKUP_BASE_MASK) {
        ret = nv_img_write(image->finfo[NV_CCM].filename, (s8 *)(uintptr_t)mbnaddr, length);
        if (ret != NV_OK) {
            nv_printf("nv carrier write err\n");
            return ret;
        }
    }

    cb = nv_get_backup_cust_cb();
    if ((backup_type & NV_BACKUP_BAK_MASK) && (cb != NULL) && (cb->img_write != NULL)) {
        ret = cb->img_write(g_img_file[NV_CCM], (s8 *)(uintptr_t)mbnaddr, length);
        if (ret != NV_OK) {
            nv_printf("nv_get_backup_cust_cb nv carrier write err\n");
            return ret;
        }
    }

    nv_printf("flush image carrier comm success\n");
    return NV_OK;
}

/* remove hd,rs,ro,rw,tag,rwbak,flg image file */
void nv_img_rm_all(void)
{
    u32 i;
    s32 ret = 0;

    for (i = 0; i < NV_IMGFILE_MAX; i++) {
        ret = nv_img_single_rm(i);
        if (ret) {
            nv_printf("remove image %d file fail(ignore)\n", i);
        }
    }
}
