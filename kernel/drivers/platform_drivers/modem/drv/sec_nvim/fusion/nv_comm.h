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

#ifndef __NV_COMM_H__
#define __NV_COMM_H__

#include <linux/slab.h>
#include <linux/device.h>
#include <linux/pm_wakeup.h>
#include <osl_thread.h>
#include <bsp_nvim.h>
#include "nv_partition.h"
#include "nv_msg.h"
#include "nv_debug.h"

#define NV_MBN_SIZE (128 * 1024)

#define nv_malloc(a) kmalloc(a, GFP_ATOMIC)
#define nv_free(p) kfree(p)

#define NV_BACKUP_BASE_MASK      (0x1)
#define NV_BACKUP_BAK_MASK      (0x1 << 1)

enum nv_init_state {
    NVM_INIT_IDLE = 0,
    NVM_INIT_START = 1,
    NVM_INIT_READ = 2,
    NVM_INIT_SUCCESS = 3,
    NVM_INIT_FAILED = 4,
};

#define NV_SNFLAG_HD    (1 << NV_SECTION_HD)
#define NV_SNFLAG_RS    (1 << NV_SECTION_RS)
#define NV_SNFLAG_RO    (1 << NV_SECTION_RO)
#define NV_SNFLAG_RW    (1 << NV_SECTION_RW)
#define NV_SNFLAG_TAG   (1 << NV_SECTION_TAG)

#define NV_SNFLAG_HD_UNMASK    (~NV_SNFLAG_HD)
#define NV_SNFLAG_RS_UNMASK    (~NV_SNFLAG_RS)
#define NV_SNFLAG_RO_UNMASK    (~NV_SNFLAG_RO)
#define NV_SNFLAG_RW_UNMASK    (~NV_SNFLAG_RW)
#define NV_SNFLAG_TAG_UNMASK   (~NV_SNFLAG_TAG)

struct rwbuf_info {
    u64 rwaddr;
    u64 rwsize;
    osl_sem_id sem;
    u8 rsv[0x4];
};

struct scblk_info {
    void *buf;
    u32 size;
    u8 rsv[0x4];
};

#define NV_LOAD_BUF_SIZE (15 * 1024 * 1024)
#define NV_GLOBAL_CTRL_SIZE (0x400)

struct nvm_info {
    int nvm_init_state;
    unsigned long paddr;
    u64 dtaddr;   /* nv data base addr */
    u64 dtsize;   /* nv data mem */
    u64 mbnaddr;
    u64 mbnsize;
    u64 ldaddr;
    u64 ldsize;
    u32 flush_time;
    osl_sem_id task_sem;
    OSL_TASK_ID task_id;
    struct wakeup_source *wakelock;
    struct nvm_fs nvmfs;
    struct nvm_msg_info msg_info;
    struct rwbuf_info rwbuf;
    struct nvm_debug_stru debug_info;
};

struct nv_backup_cust_cb {
    int (*img_write)(char *name, char *buf, unsigned int size);
    int (*blk_write)(char *buf, unsigned int size);
};

struct nvm_info *nv_get_nvminfo(void);

u64 nv_get_mem_addr(void);

u64 nv_get_mbnaddr(void);

struct nv_backup_cust_cb *nv_get_backup_cust_cb(void);

struct nv_ctrl_info *nv_get_ctrl_info(void);

struct section_info *nv_get_sninfo(void);

struct item_info *nv_get_item_info(void);

u32 nv_flush_data(struct nv_msg_data *msg_data, u32 *sync_flag);

u32 nv_flush_rwbuf(struct nv_msg_data *msg_data, u32 *sync_flag);

u32 nv_reload_flush(struct nv_msg_data *msg_data, u32 *sync_flag);

u32 nv_update_factory(struct nv_msg_data *msg_data, u32 *sync_flag);

u32 nv_update_backup(struct nv_msg_data *msg_data, u32 *sync_flag);

u32 nv_update_backup_bk(struct nv_msg_data *msg_data, u32 *sync_flag);

u32 nv_factory_reset(struct nv_msg_data *msg_data, u32 *sync_flag);

u32 nv_set_upgrade_flag(void);

u32 nv_set_coldpatch_upgrade_flag(bool flag);

u32 nv_set_ftyrst_flag(void);

u32 nv_upgrade_flush(struct nv_msg_data *msg_data, u32 *sync_flag);

int mdrv_nv_register_backup_cb(struct nv_backup_cust_cb *cb);

#endif
