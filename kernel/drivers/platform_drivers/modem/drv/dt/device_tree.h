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

#ifndef __DEVICE_TREE_H__
#define __DEVICE_TREE_H__

#include <osl_types.h>
#include <bsp_print.h>

#undef THIS_MODU
#define THIS_MODU mod_of
#define dt_pr_debug(fmt, ...) (bsp_debug("<%s> " fmt, __FUNCTION__, ##__VA_ARGS__))
#define dt_pr_warn(fmt, ...) (bsp_wrn("<%s> " fmt, __FUNCTION__, ##__VA_ARGS__))
#define dt_pr_info(fmt, ...) (bsp_info("<%s> " fmt, __FUNCTION__, ##__VA_ARGS__))
#define dt_pr_err(fmt, ...) (bsp_err("<%s> " fmt, __FUNCTION__, ##__VA_ARGS__))

#define DTB_OK (0)
#define DTB_ERROR (-1)

#define DT_ERR_DTB_LOAD 0xd00dd001
#define DT_ERR_DTBO_LOAD 0xd00dd002
#define DT_ERR_BAD_BASE_HDR 0xd00dd003
#define DT_ERR_BAD_DTBO_HDR 0xd00dd004
#define DT_ERR_OVERLAY 0xd00ddead
#define DT_ERR_BAD_DTB_HDR 0xd00dfeed
#define DT_ERR_OV_DEPTH 0xd00de001
#define DT_ERR_OV_NODE 0xd00de002
#define DT_ERR_OV_PROP 0xd00de003

#define MODEMID_MASK_BITS (0xff)
#define modemid_k_bits(n) (((n) >> (24)) & (MODEMID_MASK_BITS))
#define modemid_h_bits(n) (((n) >> (16)) & (MODEMID_MASK_BITS))
#define modemid_m_bits(n) (((n) >> (8)) & (MODEMID_MASK_BITS))
#define modemid_l_bits(n) ((n) & (MODEMID_MASK_BITS))

#define DT_NODE_MAX_DEPTH 64
#define MALLOC_LEN_LIMIT (1024 * 1024 * 16)
#define HSDT_MAGIC 0x54445348 /* Master DTB magic "HSDT" */

#define DTS_FILE_PATH_LEN 128

#define MODEM_DT_IMG "modem_dt.img"
#define MODEM_DTO_IMG "modem_dtcust.img"
#define MODEM_FW_DT_IMG "modem_fw_dtb.img"
#define DT_IMG "modem_driver_dt.img"

struct fdt_property;
struct fdt_device_node;

struct dt_ctrl {
    unsigned int state;
    unsigned int err_nr;
    unsigned int node_num;
    unsigned int prop_num;
    const void *fdt_addr;
    struct fdt_device_node *allnodes;
    const struct fdt_property **allprops;
    int root_addr_cells;
    int root_size_cells;
    u32 mem_is_no_left;
};

struct dt_release {
    u32 support;
    u32 state;
    u32 free_size;
};

struct dtb_table {
    u32 magic;
    u32 version;
    u32 num_entries;
};

struct dtb_entry {
    u8 boardid[4]; // 数组长度为4
    u8 reserved[4]; // 数组长度为4
    u32 dtb_size;
    u32 vrl_size;
    u32 dtb_offset;
    u32 vrl_offset;
    u64 dtb_file;
    u64 vrl_file;
};

enum BOARDID_CCORE {
    BOARDID_ACORE_IDX = 0,
    BOARDID_CCORE_IDX = 1,
    BOARDID_FW_IDX = 3, // FW_IDX为fw_dtb的id号
};

#define DTO_VER_LEN      3
typedef struct {
    uint32_t image_type;
    uint8_t  dto_ver[DTO_VER_LEN];
    uint8_t  resv;
} dt_sec_call_s;

extern struct dt_ctrl g_dt_ctrl;

#ifndef BSP_CONFIG_PHONE_TYPE
extern struct kset *of_kset;
#endif

int bsp_dt_get_image_path(char *image_name, char *file_path, u32 buf_len, u32 image_type);
int bsp_dt_parse_image_by_secos(char *image_name, u32 image_type);
int bsp_dt_parse_image_by_kernel(char *image_path, u32 dtb_id, int offset, char *vaddr, u32 vaddr_size);
int bsp_dt_get_dtb_entry(const char *file_name, u32 dtb_id, struct dtb_entry *entry_out, u32 offset);
int bsp_dt_find_dtb_entry(u32 dtb_id, const struct dtb_entry *base_ptr, u32 num, struct dtb_entry *dtb_entry);
unsigned int bsp_dt_get_free_size(void);
unsigned int bsp_dt_get_release_support(void);
void bsp_dt_free_buffer(void *buf);
void *bsp_dt_alloc_buffer(unsigned int bufsz);


#endif /* __DEVICE_TREE_H__ */
