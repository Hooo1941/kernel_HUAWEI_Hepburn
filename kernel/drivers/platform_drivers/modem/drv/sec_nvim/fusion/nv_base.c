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
 *
 */
#include <securec.h>
#include <product_config.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/fs.h>
#include <linux/platform_device.h>
#include <mdrv_memory.h>
#include <bsp_nvim.h>
#include <bsp_slice.h>
#include <bsp_version.h>
#include <bsp_modem_patch.h>
#include <osl_types.h>
#include "nv_comm.h"
#include "nv_msg.h"
#include "nv_partition.h"

struct nvm_info g_nvm_info = { 0 };
struct nv_backup_cust_cb g_nv_backup_cust_cb = { 0 };

struct type_to_handle {
    u32 (*p)(struct nv_msg_data *msg_data, u32 *sync_flag);
};

struct nvm_info *nv_get_nvminfo(void)
{
    return &g_nvm_info;
}

u64 nv_get_mem_addr(void)
{
    return g_nvm_info.dtaddr;
}

u64 nv_get_mbnaddr(void)
{
    return g_nvm_info.mbnaddr;
}

struct nv_backup_cust_cb *nv_get_backup_cust_cb(void)
{
    return &g_nv_backup_cust_cb;
}

struct nv_ctrl_info *nv_get_ctrl_info(void)
{
    u64 dtaddr = g_nvm_info.dtaddr;
    struct nv_ctrl_info *ctrl_info = NULL;

    if (dtaddr == 0) {
        nv_printf("nv mem init err\n");
        return NULL;
    }

    ctrl_info = (struct nv_ctrl_info *)(uintptr_t)dtaddr;
    if (ctrl_info->magicnum != NV_HEAD_MAGICNUM) {
        nv_printf("nv mem data invalid");
        return NULL;
    }

    return ctrl_info;
}

struct section_info *nv_get_sninfo(void)
{
    u64 dtaddr = g_nvm_info.dtaddr;
    struct nv_ctrl_info *ctrl_info = nv_get_ctrl_info();
    struct section_info *sninfo = NULL;

    if (ctrl_info == NULL) {
        return NULL;
    }

    sninfo = (struct section_info *)(uintptr_t)(dtaddr + ctrl_info->snidxoff);

    return sninfo;
}

struct item_info *nv_get_item_info(void)
{
    u64 dtaddr = g_nvm_info.dtaddr;
    struct nv_ctrl_info *ctrl_info = nv_get_ctrl_info();
    struct item_info *item_info = NULL;

    if (ctrl_info == NULL) {
        return NULL;
    }

    item_info = (struct item_info *)(uintptr_t)(dtaddr + ctrl_info->itmidxoff);

    return item_info;
}

u32 nv_read_patch_state(struct nv_msg_data *msg_data, u32 *sync_flag)
{
#ifdef BSP_CONFIG_PHONE_TYPE
    int ret;
    int state;
    struct nv_msg_data *msg = NULL;
    struct nvm_info *nvminfo = nv_get_nvminfo();

    ret = bsp_modem_patch_get_state(&state);
    if (ret != 0) {
        nv_printf("get patch state fail\n");
        goto out;
    }

    msg = nv_malloc(sizeof(struct nv_msg_data) + sizeof(int));
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

    ret = memcpy_s(msg->data, sizeof(int), &state, sizeof(int));
    if (ret) {
        nv_printf("fail to memcpy, ret = %d\n", ret);
        ret = NV_ERROR;
        nv_free(msg);
        goto out;
    }

    msg->data_len = sizeof(int);
    msg->ret = 0;

    ret = nv_cpmsg_send((u8 *)msg, sizeof(struct nv_msg_data) + sizeof(int));
    if (ret) {
        nv_printf("nv read nve send sp msg fail\n");
    }

    nv_free(msg);
    *sync_flag = 0;
out:
    __pm_relax(nvminfo->wakelock);
    nvminfo->msg_info.cb_reply++;
    return ret;
#else
    return NV_OK;
#endif
}

u32 nv_write_patch_info(struct nv_msg_data *msg_data, u32 *sync_flag)
{
#ifdef BSP_CONFIG_PHONE_TYPE
    int ret;
    struct nv_patch_info_s *info = (struct nv_patch_info_s *)msg_data->data;
    struct modem_patch_info patch_info;
    if (msg_data->data_len != sizeof(struct nv_patch_info_s)) {
        nv_printf("invalid patch info\n");
        ret = BSP_ERR_NV_DATA_MAGICNUM_ERR;
        return ret;
    }
    patch_info.patch_exist = info->patch_exist;
    patch_info.patch_status = info->patch_status;
    UNUSED(sync_flag);

    ret = bsp_modem_patch_set_boot_info((const char *)"nv_patch", &patch_info);
    if (ret != 0) {
        nv_printf("fail to set patch info, ret = %d\n", ret);
        return ret;
    }
    nv_printf("succeed to update nv patch info\n");
#else
    UNUSED(sync_flag);
    UNUSED(msg_data);
#endif
    return NV_OK;
}

u32 nv_get_prdtid(struct nv_msg_data *msg_data, u32 *sync_flag)
{
#ifdef BSP_CONFIG_PHONE_TYPE
    u32 cnt = msg_data->msg;
    UNUSED(sync_flag);
    if (cnt != (msg_data->data_len / sizeof(u32))) {
        nv_printf("err msg len fail %u %u\n", cnt, msg_data->data_len);
        return BSP_ERR_NV_INVALID_PARAM;
    }
    return (u32)bsp_version_get_checked_productid((u32 *)msg_data->data, cnt);
#else
    UNUSED(sync_flag);
    UNUSED(msg_data);
    return NV_OK;
#endif
}

struct type_to_handle g_msg_type_to_handle[] =
{
    {nv_load_block},       // NV_ICC_REQ_READ_BLK
    {nv_flush_data},       // NV_ICC_REQ_FLUSH
    {nv_flush_rwbuf},      // NV_ICC_REQ_FLUSH_RWNV
    {nv_reload_flush},     // NV_ICC_REQ_LOAD_BACKUP
    {nv_update_factory},   // NV_ICC_REQ_UPDATE_DEFAULT
    {nv_update_backup},    // NV_ICC_REQ_UPDATE_BACKUP
    {nv_factory_reset},    // NV_ICC_REQ_FACTORY_RESET
    {NULL},                // NV_ICC_REQ_FLUSH_RDWR_ASYNC
    {nv_upgrade_flush},    // NV_ICC_REQ_DATA_WRITEBACK
    {nv_update_backup_bk}, // NV_ICC_REQ_UPDATE_BACKUP_BK
    {nv_set_patch_flag},   // NV_ICC_REQ_SET_PATCH_FLAG
    {nv_read_patch_state}, // NV_ICC_REQ_RD_PATCH_STATE
    {nv_write_patch_info}, // NV_ICC_REQ_WR_PATCH_INFO
    {nv_get_prdtid},       // NV_ICC_REQ_GET_PRODUCTID
    {NULL},                // NV_ICC_CNF
    {nv_scblk_recv},       // NV_ICC_REQ_WRITE_SCBLK
};

u32 nv_deal_msg(struct nv_msg_data *msg_data, u32 *sync_flag)
{
    s32 index;
    u32 ret = NV_ERROR;
    u32 size = sizeof(g_msg_type_to_handle) / sizeof(struct type_to_handle);
    *sync_flag = 1;
    index = (s32)(msg_data->msg_type - NV_ICC_REQ_READ_BLK);
    if (index < 0 || index >= size) {
        nv_printf("msg type %d invalid %d %d\n", msg_data->msg_type, index, (s32)size);
        return ret;
    }
    if (g_msg_type_to_handle[index].p == NULL) {
        nv_printf("handle is null\n");
        return ret;
    }
    ret = g_msg_type_to_handle[index].p(msg_data, sync_flag);
    return ret;
}

int nv_msg_task(void *parm)
{
    u32 ret, sync_flag;
    struct nv_msg_data *msg_data = NULL;
    struct nv_msg_element *msg_element = NULL;
    UNUSED(parm);

    while (1) {
        osl_sem_down(&g_nvm_info.task_sem);

        sync_flag = 0;
        msg_element = nv_get_msg_element();
        if (msg_element == NULL) {
            continue;
        }

        msg_data = msg_element->msg_data;

        nv_debug_printf(NV_INFO, "msg task handle sn:0x%x, msg type:0x%x\n", msg_data->sn, msg_data->msg_type);
        ret = nv_deal_msg(msg_data, &sync_flag);
        msg_data->ret = ret;
        if (ret) {
            nv_printf("nv msg handle fail\n");
        }

        if (sync_flag) {
            nv_cpmsg_cb(msg_data);
        }

        nv_debug_printf(NV_DBG, "sn:0x%x, msg type:0x%x handle end\n", msg_data->sn, msg_data->msg_type);
        nv_free_msgelement(msg_element);
    }

    return NV_OK;
}

#define NV_ALIGN(x, a) __NV_ALIGN((x), (a))
#define __NV_ALIGN(x, a) __NV_ALIGN_MASK((x), (typeof(x))(a)-1)
#define __NV_ALIGN_MASK(x, mask) (((unsigned int)(x) + (mask)) & ~((unsigned int)(mask)))
u32 nv_mem_init(struct nvm_info *nvminfo)
{
    unsigned int addr;
    nvminfo->dtaddr =
        (uintptr_t)mdrv_mem_share_get("nsroshm_nv", &nvminfo->paddr, (unsigned int *)&nvminfo->dtsize, SHM_NSRO);
    if (nvminfo->dtaddr == 0) {
        nv_printf("get sec nv mem err\n");
        return NV_ERROR;
    }
    addr = NV_ALIGN(((unsigned int)(nvminfo->paddr)), 128); /* 要进行内存校验128字节对齐 */
    nvminfo->dtaddr += ((u64)addr - nvminfo->paddr);
    nvminfo->dtsize -= ((u64)addr - nvminfo->paddr);
    nvminfo->paddr = (u64)addr;
    nvminfo->dtaddr += (u64)NV_GLOBAL_CTRL_SIZE; /* hifi在手机上传入地址有1K偏移 */
    nvminfo->paddr += (u64)NV_GLOBAL_CTRL_SIZE;  /* hifi在手机上传入地址有1K偏移 */
    nvminfo->dtsize -= (u64)NV_GLOBAL_CTRL_SIZE;

    nvminfo->mbnaddr = nvminfo->dtaddr + nvminfo->dtsize - NV_MBN_SIZE;
    nvminfo->mbnsize = NV_MBN_SIZE;
    nvminfo->wakelock = wakeup_source_register(NULL, "nv_wakelock");
    if (nvminfo->wakelock == NULL) {
        nv_printf("nv wakeup source err\n");
    }
    return NV_OK;
}
#define NV_MSG_TASK_PRIORITY 15
#define NV_MSG_TASK_STACK_SIZE 2048
int bsp_nvm_init(void)
{
    u32 ret;
    struct nvm_info *nvminfo = &g_nvm_info;

    nv_debug(NV_FUN_KERNEL_INIT, 0, 0, 0, 0);
    nv_printf("nv init start\n");

    (void)memset_s(nvminfo, sizeof(struct nvm_info), 0, sizeof(struct nvm_info));
    nvminfo->nvm_init_state = NVM_INIT_START;
    ret = nv_mem_init(nvminfo);
    if (ret != NV_OK) {
        nv_debug(NV_FUN_KERNEL_INIT, 0x1, 0, 0, 0);
        nv_printf("nv mem init err\n");
        return ret;
    }

    osl_sem_init(0, &nvminfo->task_sem);
    ret = nv_debug_init();
    if (ret != NV_OK) {
        nv_debug(NV_FUN_KERNEL_INIT, 0x2, 0, 0, 0);
        nv_printf("nv debug init fail\n");
    }

    ret = nv_nvmfs_init();
    if (ret != NV_OK) {
        nv_debug(NV_FUN_KERNEL_INIT, 0x3, 0, 0, 0);
        nv_printf("nv file system init fail\n");
        goto nv_init_fail;
    }

    ret = (u32)osl_task_init("nv_msg", NV_MSG_TASK_PRIORITY, NV_MSG_TASK_STACK_SIZE, (OSL_TASK_FUNC)nv_msg_task, NULL,
        (OSL_TASK_ID *)&nvminfo->task_id);
    if (ret) {
        nv_debug(NV_FUN_KERNEL_INIT, 0x4, 0, 0, 0);
        nv_printf("nv icc task init fail\n");
        goto nv_init_fail;
    }

    ret = nv_msg_init();
    if (ret != NV_OK) {
        nv_debug(NV_FUN_KERNEL_INIT, 0x5, 0, 0, 0);
        nv_printf("nv msg init fail\n");
        goto nv_init_fail;
    }

    nvminfo->nvm_init_state = NVM_INIT_SUCCESS;

    nv_printf("nv init success\n");
    return NV_OK;

nv_init_fail:
    nvminfo->nvm_init_state = NVM_INIT_FAILED;
    return (int)ret;
}

#ifdef BSP_CONFIG_PHONE_TYPE
ssize_t modem_nv_read(struct file *file, char __user *buf, size_t len, loff_t *ppos)
{
    return (ssize_t)len;
}

ssize_t modem_nv_write(struct file *file, const char __user *buf, size_t len, loff_t *ppos)
{
    u32 ret;

    ret = nv_set_upgrade_flag();
    if (ret) {
        nv_printf("set nv upgrade flag err, ret:0x%x\n", ret);
        return -EIO;
    }
#ifdef BSP_CONFIG_PHONE_TYPE
    (void)nv_set_coldpatch_upgrade_flag((bool)true);
#endif

    nv_printf("modify upgrade flag success !\n");
    return len;
}
static const struct proc_ops g_modem_nv = {
    .proc_read = modem_nv_read,
    .proc_write = modem_nv_write,
};
#endif

int nv_proc_init(void)
{
#ifdef BSP_CONFIG_PHONE_TYPE
    if (proc_create("ModemNv", 0660, NULL, &g_modem_nv) == NULL) { /* 0660:open mode */
        return -1;
    }
#endif

    return 0;
}

static int modem_nv_probe(struct platform_device *dev)
{
    int ret;

    ret = bsp_nvm_init();
    if (ret) {
        nv_printf("nv init err\n");
        return ret;
    }

    ret = nv_proc_init();
    if (ret) {
        nv_printf("nv proc file init err\n");
        return ret;
    }

    return ret;
}

int modem_nv_init(void)
{
    int ret;
    struct platform_device *dev = NULL;

    if (g_nvm_info.nvm_init_state != 0) {
        show_stack(current, NULL, KERN_ERR);
    }
    ret = nv_init_dev();
    if (ret) {
        return ret;
    }
    ret = modem_nv_probe(dev);

    return ret;
}

int mdrv_nv_register_backup_cb(struct nv_backup_cust_cb *cb)
{
    int ret = 0;

    if (cb == NULL) {
        nv_printf("mdrv_nv_register_backup_cb err, cb=NULL\n");
        return NV_ERROR;
    }

    ret = memcpy_s(&g_nv_backup_cust_cb, sizeof(struct nv_backup_cust_cb), cb, sizeof(struct nv_backup_cust_cb));
    if (ret) {
        nv_printf("g_nv_backup_cust_cb memcpy_s err\n");
        return ret;
    }
    return ret;
}
EXPORT_SYMBOL(mdrv_nv_register_backup_cb);