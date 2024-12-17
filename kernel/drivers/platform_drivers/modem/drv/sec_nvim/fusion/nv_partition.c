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

#include <securec.h>
#include <product_config.h>
#ifdef BSP_CONFIG_PHONE_TYPE
#include <adrv_partition.h>
#else
#include <bsp_partition.h>
#endif
#include <linux/version.h>
#include <linux/vmalloc.h>
#include <bsp_file_ops.h>
#include <bsp_fipc.h>
#include <bsp_slice.h>
#include <bsp_blk.h>
#include <bsp_rfile.h>
#include <bsp_nvim.h>
#include "nv_comm.h"
#include "nv_msg.h"
#include "nv_partition.h"

static char g_blk_info[NV_BLK_MAX][BLK_MAX_NAME] = {
#ifdef BSP_CONFIG_PHONE_TYPE
    PARTITION_MODEM_NVBACKUP_NAME,
    PARTITION_MODEM_NVDEFAULT_NAME,
    "modem_patch_nv"
#else
    PTABLE_NVBACKLTE,
    PTABLE_NVDEFAULT
#endif
};

struct scblk_info g_scblk_info = { 0 };

struct nvm_fs *nv_get_nvmfs(void)
{
    struct nvm_info *nvminfo = nv_get_nvminfo();

    return &nvminfo->nvmfs;
}

struct image_partition *nv_get_image_partition(void)
{
    struct nvm_fs *nvmfs = nv_get_nvmfs();

    return &nvmfs->image;
}

u32 nv_blk_read(u32 partition, u8 *buf, u32 offset, u32 size)
{
    return (u32)bsp_blk_read(g_blk_info[partition], offset, buf, size);
}

u32 nv_blk_write(u32 partition, u8 *buf, u32 offset, u32 size)
{
    return (u32)bsp_blk_write(g_blk_info[partition], offset, buf, size);
}

s32 nv_file_access(s8 *file, s32 mode)
{
    return bsp_eaccess(file);
}

s32 nv_file_remove(s8 *file)
{
    s32 ret;
    unsigned long fs;
    fs = get_fs();
    set_fs((unsigned long)KERNEL_DS);
    ret = do_unlinkat(AT_FDCWD, getname(file));
    set_fs(fs);
    return ret;
}

static s32 nv_check_path(const char *path)
{
    if (strlen(path) >= NV_PATH_LEN) {
        nv_printf("path too long, len=%d\n", (int)strlen(path));
        return NV_ERROR;
    }

    if (strstr(path, "../")) {
        nv_printf("path is not allowed, len=%s\n", path);
        return NV_ERROR;
    }

    return NV_OK;
}

s32 nv_file_mkdir(const char *dir)
{
    s32 ret;
    unsigned long fs;
    s32 mode = 0770;

    ret = nv_check_path(dir);
    if (ret) {
        return ret;
    }

    fs = get_fs();
    set_fs((mm_segment_t)KERNEL_DS);
    ret = bsp_file_mkdir(dir, mode);
    set_fs(fs);
    if (ret && ret != -EEXIST) {
        nv_printf("sys_mkdir fail,ret=%d\n",ret);
        return ret;
    }

    return NV_OK;
}

struct file *nv_file_open(s8 *path, s32 flags, s32 file_mode)
{
    s64 ret;
    unsigned long fs;
    char log_path[NV_PATH_LEN] = {0};
    char *p = NULL;
    struct file *fd = NULL;

    ret = strcpy_s(log_path, sizeof(log_path), path);
    if (ret) {
        nv_printf("strcpy_s error, ret = %lld\n",ret);
        return NULL;
    }

    p = strrchr(log_path, '/');
    if ((p != NULL) && (p != log_path)) {
        /* 查看上一级目录是否存在，如果不存在则创建此目录 */
        *p = '\0';
        ret = nv_file_mkdir((const char *)log_path);
        if (ret) {
            nv_printf("dump_mkdir fail, ret=%lld\n", ret);
        }
    }

    fs = get_fs();
    set_fs((mm_segment_t)KERNEL_DS);
    fd = filp_open(path, flags, file_mode);
    set_fs(fs);

    return fd;
}

s32 nv_file_read(struct file *fp, s8 *buf, u32 offset, u32 size)
{
    s32 ret;
    loff_t pos = offset;

    ret = kernel_read(fp, (void *)buf, size, &pos);
    return ret;
}

s32 nv_file_write(struct file *fp, s8 *buf, u32 size)
{
    s32 ret;
    loff_t pos = 0;

    ret = kernel_write(fp, buf, size, &pos);

    (void)vfs_fsync(fp, 0);

    return ret;
}

void nv_file_close(struct file *fp)
{
    s32 ret;

    ret = filp_close(fp, NULL);
    if (ret) {
        nv_printf("close fail, ret:%d\n", ret);
    }
}

u32 nv_load_block(struct nv_msg_data *msg_data, u32 *sync_flag)
{
    u32 ret = 0;
    struct nvm_info *nvminfo = nv_get_nvminfo();
    struct nv_rdblk_info *blk = (struct nv_rdblk_info *)msg_data->data;
    struct nv_msg_data *msg = NULL;

    nv_debug_printf(NV_DBG, "read partition %d offset:0x%x, size:0x%x\n", blk->partition, blk->offset, blk->size);
    if (msg_data->data_len != sizeof(struct nv_rdblk_info) || (blk->magicnum != NV_PARTITION_MAGICNUM) ||
        (blk->partition >= NV_BLK_MAX) || (blk->size == 0) || (blk->size > NV_FIPC_MAX_LEN)) {
        nv_printf("invalid blk info\n");
        ret = BSP_ERR_NV_DATA_MAGICNUM_ERR;
        goto out;
    }

    msg = nv_malloc(sizeof(struct nv_msg_data) + blk->size);
    if (msg == NULL) {
        nv_printf("nv load block malloc fail\n");
        ret = BSP_ERR_NV_MALLOC_FAIL;
        goto out;
    }

    msg->magicnum = NV_MSG_MAGICNUM;
    msg->slice = bsp_get_slice_value();
    msg->msg_type = NV_ICC_CNF;
    msg->msg = 0;
    msg->sn = msg_data->sn;
    msg->rcvlen = 0;

    ret = nv_blk_read(blk->partition, msg->data, blk->offset, blk->size);
    if (ret) {
        nv_free(msg);
        nv_printf("nv load block read fail, ret:%d\n", ret);
        goto out;
    }

    msg->data_len = blk->size;
    msg->ret = 0;

    nv_debug_printf(NV_DBG, "send read data sn:0x%x, msg type:0x%x, private data len:0x%x\n", msg->sn, msg->msg_type,
        msg->data_len);
    ret = nv_cpmsg_send((u8 *)msg, sizeof(struct nv_msg_data) + blk->size);
    if (ret) {
        nv_printf("nv load block send sp msg fail\n");
    }

    nv_free(msg);
    *sync_flag = 0;
out:
    __pm_relax(nvminfo->wakelock);
    nvminfo->msg_info.cb_reply++;
    return ret;
}

#define SCBLK_MAX_SIZE 0x8000
static u32 nv_scblk_recv_first(u32 size)
{
    void *addr = NULL;
    if (size == 0 || size > SCBLK_MAX_SIZE || g_scblk_info.buf != NULL) {
        nv_printf("invalid call\n");
        return BSP_ERR_NV_MALLOC_FAIL;
    }
    addr = nv_malloc(size);
    if (addr == NULL) {
        nv_printf("nv malloc chan buf fail\n");
        return BSP_ERR_NV_MALLOC_FAIL;
    }
    g_scblk_info.buf = addr;
    g_scblk_info.size = size;
    return 0;
}

static void scblk_release(void)
{
    if (g_scblk_info.buf != NULL) {
        nv_free(g_scblk_info.buf);
        g_scblk_info.buf = NULL;
    }
    g_scblk_info.size = 0;
}

static u32 nv_scblk_recv_last(void)
{
    u32 ret = 0;
    struct nvm_info *nvminfo = nv_get_nvminfo();
    void *addr = NULL;
    u32 nv_size;
    u64 size;

    if (g_scblk_info.buf == NULL || g_scblk_info.size == 0) {
        nv_printf("end invalid scblk buf\n");
        return BSP_ERR_NV_SCBLK_BUF_ERR;
    }
    nv_size = nv_get_data_size(nvminfo->dtaddr);
    if (nv_size > nvminfo->dtsize) {
        nv_printf("flush factory nv data overflow\n");
        return BSP_ERR_NV_OVER_MEM_ERR;
    }
    size = nv_size + g_scblk_info.size;
    addr = vmalloc(size);
    if (addr == NULL) {
        nv_printf("nv malloc 0x%x chan buf fail\n", (u32)size);
        ret = BSP_ERR_NV_MALLOC_FAIL;
        goto out;
    }
    ret = memcpy_s(addr, size, (void *)(uintptr_t)nvminfo->dtaddr, nv_size);
    if (ret) {
        nv_printf("fail to memcpy, ret = %d\n", ret);
        vfree(addr);
        ret = NV_ERROR;
        goto out;
    }
    ret = memcpy_s(addr + nv_size, size - nv_size, g_scblk_info.buf, g_scblk_info.size);
    if (ret) {
        nv_printf("fail to memcpy, ret = %d\n", ret);
        vfree(addr);
        ret = NV_ERROR;
        goto out;
    }
    ret = nv_blk_write(NV_BLK_FACTORY, (u8 *)(uintptr_t)addr, 0, size);
    if (ret) {
        nv_printf("nv write factory err, ret:0x%x\n", ret);
    }
    vfree(addr);
out:
    scblk_release();
    return ret;
}

u32 nv_scblk_recv(struct nv_msg_data *msg_data, u32 *sync_flag)
{
    u32 ret = 0;
    struct nvm_info *nvminfo = nv_get_nvminfo();
    struct nv_scblk_head *scblk_head = (struct nv_scblk_head *)msg_data->data;
    u8 *data = msg_data->data + sizeof(struct nv_scblk_head);
    u32 length = (msg_data->data_len > sizeof(struct nv_scblk_head)) ?
        (msg_data->data_len - sizeof(struct nv_scblk_head)) : 0;

    UNUSED(sync_flag);
    nv_debug_printf(NV_DBG, "write offset:0x%x, length:0x%x\n", scblk_head->offset, length);
    if (length == 0 || length > NV_FIPC_C2A_MAX_LEN || scblk_head->magicnum != NV_PARTITION_MAGICNUM ||
        (scblk_head->offset + length < length) || ((scblk_head->offset + length) > scblk_head->size)) {
        nv_printf("invalid scblk info\n");
        ret = BSP_ERR_NV_DATA_MAGICNUM_ERR;
        goto out;
    }
    if (scblk_head->offset == 0) {
        if (nv_scblk_recv_first(scblk_head->size) != 0) {
            nv_printf("write invalid scblk buf\n");
            ret = BSP_ERR_NV_MALLOC_FAIL;
            scblk_release();
            goto out;
        }
    }
    if (g_scblk_info.buf == NULL || g_scblk_info.size == 0 || g_scblk_info.size != scblk_head->size) {
        nv_printf("write invalid scblk buf\n");
        ret = BSP_ERR_NV_SCBLK_BUF_ERR;
        scblk_release();
        goto out;
    }
    ret = memcpy_s(g_scblk_info.buf + scblk_head->offset, g_scblk_info.size - scblk_head->offset, data, length);
    if (ret) {
        nv_printf("fail to memcpy, ret = %d\n", ret);
        ret = NV_ERROR;
        scblk_release();
        goto out;
    }
    if ((scblk_head->offset + length) == scblk_head->size) {
        ret = nv_scblk_recv_last();
    }
out:
    __pm_relax(nvminfo->wakelock);
    nvminfo->msg_info.cb_reply++;
    return ret;
}

u32 nv_set_patch_flag(struct nv_msg_data *msg_data, u32 *sync_flag)
{
    u32 ret;
    struct nv_patch_flag_info *info = (struct nv_patch_flag_info *)msg_data->data;
    nv_dload_tail *tail = NULL;
    nv_dload_tail tmp;
    UNUSED(sync_flag);
    if (msg_data->data_len != sizeof(struct nv_patch_flag_info)) {
        nv_printf("invalid len\n");
        return BSP_ERR_NV_DATA_MAGICNUM_ERR;
    }
    tail = &(info->tail);
    if (info->magicnum != NV_PARTITION_MAGICNUM) {
        return BSP_ERR_NV_DATA_MAGICNUM_ERR;
    }
    ret = nv_blk_read(NV_BLK_PATCH, (u8 *)&tmp, info->offset, sizeof(nv_dload_tail));
    if (ret) {
        nv_printf("nv read patch err, ret:0x%x\n", ret);
        return ret;
    }
    tmp.upgrade_flag = tail->upgrade_flag;
    ret = nv_blk_write(NV_BLK_PATCH, (u8 *)&tmp, info->offset, sizeof(nv_dload_tail));
    if (ret) {
        nv_printf("nv write patch err, ret:0x%x\n", ret);
        return ret;
    }
    return NV_OK;
}

static u32 nv_mountpath_init(struct nvm_fs *nvmfs)
{
    u32 ret;

    ret = (u32)nv_file_access((s8 *)NV_ROOT_PATH, 0);
    if (ret) {
        nv_printf("can't access nvm fs mountpath:%s\n", NV_ROOT_PATH);
        return ret;
    }

    ret = strncpy_s((char *)nvmfs->mount_path, NV_PATH_LEN, (char *)NV_ROOT_PATH,
        strlen((char *)NV_ROOT_PATH));
    if (ret) {
        nv_printf("copy mount path fail, ret:0x%x\n", ret);
        return ret;
    }

    return NV_OK;
}

static u32 nvmfs_path_init(struct nvm_fs *nvmfs)
{
    u32 ret;

    ret = nv_mountpath_init(nvmfs);
    if (ret != NV_OK) {
        nv_printf("nv mountpath init fail\n");
        return ret;
    }

    ret = nv_image_path_init(nvmfs);
    if (ret != NV_OK) {
        nv_printf("nv image path init fail\n");
        return ret;
    }

    return NV_OK;
}

u32 nv_nvmfs_init(void)
{
    u32 ret;
    struct nvm_fs *nvmfs = nv_get_nvmfs();

    ret = nvmfs_path_init(nvmfs);
    if (ret != NV_OK) {
        nv_printf("nvmfs path init fail\n");
        return ret;
    }

    nv_printf("nv file system init success\n");
    return NV_OK;
}

u32 nv_get_data_size(u64 dtaddr)
{
    u32 i;
    u32 size;
    struct nv_ctrl_info *ctrl_info = (struct nv_ctrl_info *)(uintptr_t)dtaddr;
    struct section_info *sninfo = NULL;

    if (ctrl_info->magicnum != NV_HEAD_MAGICNUM) {
        nv_printf("invalid nv memory data\n");
        return NV_ERROR;
    }

    size = ctrl_info->hdsize;
    sninfo = (struct section_info *)(dtaddr + ctrl_info->snidxoff);

    for (i = 0; i < ctrl_info->sncnt; i++) {
        size += sninfo[i].length;
    }

    return size;
}
