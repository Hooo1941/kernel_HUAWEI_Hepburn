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

#include "device_tree.h"
#include <securec.h>
#include <bsp_dt.h>
#include <libfdt.h>
#include <linux/of.h>
#include <bsp_mloader.h>

struct dt_ctrl g_dt_ctrl = { 0 };

/*
 * return NULL if failed
 * caller release the allocated buffer
 */
static void *bsp_dt_load_dtb(const char *image_name, u32 dtb_id, unsigned int extra_bufsz)
{
    int ret;
    int rd_bytes;
    u32 offset = 0;
    u32 buffer_size = 0;
    u32 total_size = 0;
    void *fdt = NULL;
    char file_path[DTS_FILE_PATH_LEN] = {0};
    const char *modem_vendor_path = bsp_mloader_get_modem_vendor_path();

    ret = sprintf_s(file_path, DTS_FILE_PATH_LEN, "%s/%s/%s", modem_vendor_path, "image", image_name);
    if (ret < 0) {
        dt_pr_err("fail to fmt modem image path:%s, %s\n",modem_vendor_path, image_name);
        return NULL;
    }

    total_size = bsp_mloader_get_file_size(file_path);
    buffer_size = total_size + extra_bufsz;

    fdt = bsp_dt_alloc_buffer(buffer_size);
    if (fdt == NULL) {
        dt_pr_err("malloc dtb buffer failed, dtb size is %u\n", buffer_size);
        return NULL;
    }

    ret = bsp_dt_parse_image_by_kernel(file_path, dtb_id, offset, fdt, buffer_size);
    if (ret) {
        dt_pr_err("dt parse image %s by kernel failed \n", image_name);
        bsp_dt_free_buffer(fdt);
        fdt = NULL;
        return NULL;
    }

    return fdt;
}

const void *bsp_dt_load(void)
{
    const char *dtimg_name = DT_IMG;
    const u32 dtb_id = BOARDID_ACORE_IDX;
    void *fdt = NULL;

    dt_pr_err("use 0 as modem_id to load base dtb\n");
    fdt = bsp_dt_load_dtb(dtimg_name, dtb_id, 0);
    if (fdt == NULL) {
        g_dt_ctrl.err_nr = DT_ERR_DTB_LOAD;
        return NULL;
    }
    dt_pr_err("load dtb ok from file %s\n", dtimg_name);

    return (const void *)fdt;
}

static int bsp_dt_sum_nodes_and_props(const void *fdt)
{
    int node_offset;
    int prop_offset;
    int depth = -1;

    for (node_offset = fdt_next_node(fdt, -1, &depth); node_offset >= 0 && depth >= 0;
        node_offset = fdt_next_node(fdt, node_offset, &depth)) {
        ++g_dt_ctrl.node_num;

        prop_offset = node_offset;
        for (prop_offset = fdt_first_property_offset(fdt, prop_offset); prop_offset >= 0;
            prop_offset = fdt_next_property_offset(fdt, prop_offset)) {
            ++g_dt_ctrl.prop_num;
        }
    }

    return BSP_OK;
}

static inline const void *dt_get_offset_ptr(const void *fdt, int offset)
{
    return (const char *)fdt + fdt_off_dt_struct(fdt) + offset;
}

static void dt_find_node_chief_props(const void *fdt, struct fdt_device_node *node)
{
    const struct fdt_property *fdt_prop = NULL;
    const char *propname = NULL;
    unsigned int index;

    node->name = ((const struct fdt_node_header *)dt_get_offset_ptr(fdt, node->offset))->name;

    for (index = 0; index < node->prop_count; ++index) {
        fdt_prop = g_dt_ctrl.allprops[node->prop_start + index];
        propname = (const char *)fdt + fdt_off_dt_strings(fdt) + fdt32_to_cpu(fdt_prop->nameoff);
        /* propname is const char pointer type, use strcmp for boot speed , 11为compatible的长度 */
        if (fdt32_to_cpu(fdt_prop->len) > 0 && strncmp(propname, "compatible", 11) == 0) { /*lint !e421*/
            node->compatible = (const char *)fdt_prop->data;
        } else if (fdt32_to_cpu(fdt_prop->len) > 0 &&
            strncmp(propname, "device_type", 12) == 0) { /*lint !e421*/ // 12为device_type的长度
            node->type = (const char *)fdt_prop->data;
        } else if (fdt32_to_cpu(fdt_prop->len) == sizeof(uint32_t) &&
            strncmp(propname, "phandle", strlen(propname)) == 0) { /*lint !e421*/
            node->phandle = (uint32_t)fdt32_to_cpu(*(const fdt32_t *)fdt_prop->data);
        }
    }
}

static int dt_parse_node_prop(const void *fdt, struct fdt_device_node *node)
{
    int offset = node->offset;
    static unsigned int prop_alloc_index;

    for (offset = fdt_first_property_offset(fdt, offset); offset >= 0; offset = fdt_next_property_offset(fdt, offset)) {
        if (prop_alloc_index >= g_dt_ctrl.prop_num) {
            g_dt_ctrl.err_nr = DT_ERR_OV_PROP;
            dt_pr_err("prop table overflow!\n");
            return BSP_ERROR;
        }
        g_dt_ctrl.allprops[prop_alloc_index] = (const struct fdt_property *)dt_get_offset_ptr(fdt, offset);
        if (node->prop_count == 0) {
            node->prop_start = prop_alloc_index;
        }
        node->prop_count += 1;
        ++prop_alloc_index;
    }

    dt_find_node_chief_props(fdt, node);
    return BSP_OK;
}

static int dt_populate_node(const void *fdt, int offset, struct fdt_device_node *dad, struct fdt_device_node **pnp)
{
    struct fdt_device_node *node = NULL;
    static int node_alloc_index;

    if (node_alloc_index >= g_dt_ctrl.node_num) {
        g_dt_ctrl.err_nr = DT_ERR_OV_NODE;
        dt_pr_err("mem overflow!\n");
        return BSP_ERROR;
    }

    node = &g_dt_ctrl.allnodes[node_alloc_index++];
    node->offset = offset;

    if (dad != NULL) {
        node->parent = dad;
        if (dad->child == NULL) {
            dad->child = node;
            dad->child_tail = node;
        } else {
            dad->child_tail->sibling = node;
            dad->child_tail = node;
        }
    }
    *pnp = node;

    if (dt_parse_node_prop(fdt, node) != 0) {
        return BSP_ERROR;
    }

    return BSP_OK;
}

static int bsp_dt_unflatten_nodes(const void *fdt)
{
    int offset;
    int depth = -1;
    struct fdt_device_node *nps[DT_NODE_MAX_DEPTH + 1] = {0};
    int err;

    for (offset = fdt_next_node(fdt, -1, &depth); offset >= 0 && depth >= 0;
        offset = fdt_next_node(fdt, offset, &depth)) {
        if (depth >= DT_NODE_MAX_DEPTH) {
            g_dt_ctrl.err_nr = DT_ERR_OV_DEPTH;
            dt_pr_err("depth overflow!\n");
            return BSP_ERROR;
        }
        err = dt_populate_node(fdt, offset, nps[depth], &nps[depth + 1]); /*lint !e679*/
        if (err != BSP_OK) {
            return BSP_ERROR;
        }
    }

    return BSP_OK;
}

static int bsp_dt_alloc_unflatten_buf(void)
{
    unsigned int len_nodes;
    unsigned int len_props;

    len_nodes = g_dt_ctrl.node_num * sizeof(struct fdt_device_node);
    g_dt_ctrl.allnodes = (struct fdt_device_node *)bsp_dt_alloc_buffer(len_nodes);
    if (g_dt_ctrl.allnodes == NULL) {
        dt_pr_err("malloc device_node buffer failed, size is %u\n", len_nodes);
        goto err_out;
    }

    len_props = g_dt_ctrl.prop_num * sizeof(void *);
    g_dt_ctrl.allprops = (const struct fdt_property **)bsp_dt_alloc_buffer(len_props);
    if (g_dt_ctrl.allprops == NULL) {
        dt_pr_err("malloc fdt_prop ptr buffer failed, size is %u\n", len_props);
        goto err_out;
    }

    return BSP_OK;

err_out:
    bsp_dt_free_buffer(g_dt_ctrl.allnodes);
    g_dt_ctrl.allnodes = NULL;
    return BSP_ERROR;
}

static void bsp_dt_free_unflatten_buf(void)
{
    bsp_dt_free_buffer(g_dt_ctrl.allnodes);
    g_dt_ctrl.allnodes = NULL;
    bsp_dt_free_buffer(g_dt_ctrl.allprops);
    g_dt_ctrl.allprops = NULL;
}

int bsp_dt_parse(const void *fdt)
{
    int ret;

    if (fdt_check_header(fdt)) {
        g_dt_ctrl.err_nr = DT_ERR_BAD_DTB_HDR;
        dt_pr_err("parse modem driver dtb get invalid dt header\n");
        return BSP_ERROR;
    }

    ret = bsp_dt_sum_nodes_and_props(fdt);
    if (ret != BSP_OK) {
        dt_pr_err("parse modem driver dtb first round failed\n");
        return BSP_ERROR;
    }

    ret = bsp_dt_alloc_unflatten_buf();
    if (ret != BSP_OK) {
        dt_pr_err("malloc nodes_and_props failed\n");
        goto err_out;
    }

    if (bsp_dt_unflatten_nodes(fdt)) {
        dt_pr_err("unflatten modem driver dtb failed\n");
        ret = BSP_ERROR;
        goto err_out;
    }

    dt_pr_err("unflatten modem driver dtb success\n");
    return BSP_OK;

err_out:
    bsp_dt_free_unflatten_buf();
    return ret;
}

int bsp_dt_init(void)
{
    dt_pr_err("%s enter\n", __func__);
    g_dt_ctrl.state = 0x0;
    g_dt_ctrl.fdt_addr = bsp_dt_load();
    if (g_dt_ctrl.fdt_addr == NULL) {
        dt_pr_err("%s load modem driver dtb failed\n", __func__);
        return BSP_ERROR;
    }
    g_dt_ctrl.state = 0x1;
    if (bsp_dt_parse(g_dt_ctrl.fdt_addr) != BSP_OK) {
        dt_pr_err("%s parse modem driver dtb failed\n", __func__);
        goto err_out;
    }
    g_dt_ctrl.state = 0x2;
    dt_pr_err("%s exit ok\n", __func__);
    return BSP_OK;

err_out:
    bsp_dt_free_buffer((void *)g_dt_ctrl.fdt_addr);
    g_dt_ctrl.fdt_addr = NULL;
    return BSP_ERROR;
}

static void __exit bsp_dt_exit(void)
{
    bsp_dt_free_unflatten_buf();

    bsp_dt_free_buffer((void *)g_dt_ctrl.fdt_addr);
    g_dt_ctrl.fdt_addr = NULL;
}

#if !IS_ENABLED(CONFIG_MODEM_DRIVER)
module_init(bsp_dt_init);
module_exit(bsp_dt_exit);
#endif
