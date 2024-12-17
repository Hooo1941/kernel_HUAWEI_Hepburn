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
#include <bsp_dt.h>
#include "libfdt.h"
#include <securec.h>
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/io.h>
#include <linux/of_irq.h> // for irq map

#define DT_NODE_ADDR_CELL_DEFAULT 2
#define DT_NODE_SIZE_CELL_DEFAULT 2
#define DT_NODE_INTR_CELL_DEFAULT 3

#define dt_compat_cmp(s1, s2) strcasecmp((s1), (s2))
#define dt_type_cmp(s1, s2) strcasecmp((s1), (s2))

DEFINE_MUTEX(mdmdrv_dt_mutex);

static inline int dt_device_is_compatible(const struct fdt_device_node *dev, const char *compat)
{
    return dt_compat_cmp(dev->compatible, compat) == 0 ? 1 : 0;
}

static inline int dt_device_is_typeof(const struct fdt_device_node *dev, const char *type)
{
    return dt_type_cmp(dev->type, type) == 0 ? 1 : 0;
}

device_node_s *dt_find_node_by_path(device_node_s *node, const char *path)
{
    struct fdt_device_node *child = NULL;
    int len;
    len = strcspn(path, "/");
    if (!len)
        return NULL;

    bsp_dt_for_each_child_of_node(node, child) {
        if (strncmp(path, child->name, len) == 0 && (strlen(child->name) == len)) {
            return child;
        }
    }
    return NULL;
}

device_node_s *bsp_dt_find_node_by_path(const char *path)
{
    struct fdt_device_node *node = NULL;
    struct fdt_device_node *root = g_dt_ctrl.allnodes;


    if (g_dt_ctrl.state == 0) {
        dt_pr_err("dt is not populate\n");
        return NULL;
    }

    if (path == NULL || *path != '/') {
        dt_pr_err("invalid path\n");
        return NULL;
    }

    if (strcmp(path, "/") == 0) {
        return root;
    }

    node = root;

    while (node != NULL && path != NULL) {
        path++;
        node = dt_find_node_by_path(node, path);
        path = strchr(path, '/');
    }

    if (node == NULL) {
        dt_pr_err("no node in path %s\n", path);
    }

    return node;
}

device_node_s *bsp_dt_find_compatible_node(device_node_s *from, const char *type, const char *compat)
{
    struct fdt_device_node *dev = NULL;
    struct fdt_device_node *dev_start = NULL;
    struct fdt_device_node *dev_end = g_dt_ctrl.allnodes + g_dt_ctrl.node_num;

    if (g_dt_ctrl.state == 0) {
        return NULL;
    }
    if (compat == NULL) {
        dt_pr_err("null compatible param\n");
        return NULL;
    }

    dev_start = (from != NULL) ? from : g_dt_ctrl.allnodes;
    for (dev = dev_start; dev < dev_end; dev++) {
        if (type != NULL) {
            if (dev->type == NULL || dt_device_is_typeof(dev, type) != 0) {
                continue;
            }
        }
        if (dev->compatible == NULL || dt_device_is_compatible(dev, compat) == 0) {
            continue;
        }

        return dev;
    }

    dt_pr_debug("hint no node %s\n", compat);
    return NULL;
}

const struct fdt_property *dt_find_fdt_prop(const struct fdt_device_node *fdt_node, const char *propname)
{
    const struct fdt_property *fdt_prop = NULL;
    const char *name = NULL;
    unsigned int index;
    const void *fdt = g_dt_ctrl.fdt_addr;

    for (index = 0; index < fdt_node->prop_count; ++index) {
        fdt_prop = g_dt_ctrl.allprops[fdt_node->prop_start + index];
        name = (const char *)fdt + fdt_off_dt_strings(fdt) + fdt32_to_cpu(fdt_prop->nameoff);
        /* propname is const char pointer type, use strcmp for boot speed */
        if (strcmp(propname, name) == 0) { /*lint !e421*/
            return fdt_prop;
        }
    }

    return NULL;
}

struct property *bsp_dt_find_property(const device_node_s *np, const char *propname, int *lenp)
{
    const struct fdt_device_node *fdt_node = (const struct fdt_device_node *)np;
    const struct fdt_property *fdt_prop = NULL;

    if (fdt_node == NULL || propname == NULL) {
        return NULL;
    }

    fdt_prop = dt_find_fdt_prop(fdt_node, propname);
    if (fdt_prop == NULL) {
        dt_pr_debug("%s no %s property\n", fdt_node->name, propname);
        return NULL;
    }

    if (lenp != NULL) {
        *lenp = (int)fdt32_to_cpu(fdt_prop->len);
    }

    return (struct property *)fdt_prop;
}

const void *bsp_dt_get_property(const device_node_s *np, const char *propname, int *lenp)
{
    const struct fdt_device_node *fdt_node = (const struct fdt_device_node *)np;
    const struct fdt_property *fdt_prop = NULL;
    const void *value_ptr = NULL;

    if (fdt_node == NULL || propname == NULL) {
        return NULL;
    }

    fdt_prop = dt_find_fdt_prop(fdt_node, propname);
    if (fdt_prop == NULL) {
        dt_pr_debug("%s no %s property\n", fdt_node->name, propname);
        return NULL;
    }

    value_ptr = (const void *)fdt_prop->data;
    if (lenp != NULL) {
        *lenp = (int)fdt32_to_cpu(fdt_prop->len);
    }

    return value_ptr;
}

bool bsp_dt_device_is_available(const device_node_s *np)
{
    const char *status = NULL;
    int len = 0;

    status = (const char *)bsp_dt_get_property(np, "status", &len);
    if (status == NULL) {
        dt_pr_debug("no status, default enable\n");
        return true;
    }

    if (len > 0) {
        if (strncmp(status, "okay", (strlen("okay") + 1)) == 0 || strncmp(status, "ok", (strlen("ok") + 1)) == 0) { /*lint !e421*/
            return true;
        }
    }
    return false;
}

const char *bsp_dt_get_node_name(const device_node_s *np)
{
    const struct fdt_device_node *fdt_node = (const struct fdt_device_node *)np;

    if (fdt_node != NULL) {
        return fdt_node->name;
    }
    return NULL;
}

device_node_s *bsp_dt_get_parent(const device_node_s *np)
{
    const struct fdt_device_node *fdt_node = (const struct fdt_device_node *)np;

    if (fdt_node != NULL) {
        return fdt_node->parent;
    }

    return NULL;
}

device_node_s *bsp_dt_get_next_available_child(const device_node_s *np, device_node_s *prev)
{
    const struct fdt_device_node *node_parent = (const struct fdt_device_node *)np;
    const struct fdt_device_node *node_prev = (const struct fdt_device_node *)prev;
    struct fdt_device_node *node_next = NULL;

    if (node_parent == NULL && node_prev == NULL) { /* if both null */
        return NULL;
    }

    node_next = node_prev ? node_prev->sibling : node_parent->child;
    while (node_next != NULL) {
        if (bsp_dt_device_is_available(node_next)) {
            return (device_node_s *)node_next;
        }
        node_next = node_next->sibling;
    }

    return NULL;
}

int bsp_dt_get_child_count(const device_node_s *np)
{
    const struct fdt_device_node *fdt_node = (const struct fdt_device_node *)np;
    struct fdt_device_node *child = NULL;
    int count = 0;

    if (fdt_node == NULL) {
        return 0;
    }

    for (child = fdt_node->child; child != NULL; child = child->sibling) {
        count++;
    }

    return count;
}

device_node_s *bsp_dt_get_child_by_name(const device_node_s *np, const char *name)
{
    const struct fdt_device_node *fdt_node = (const struct fdt_device_node *)np;
    struct fdt_device_node *child = NULL;

    if (fdt_node == NULL || name == NULL) {
        return NULL;
    }

    for (child = fdt_node->child; child != NULL; child = child->sibling) {
        if (child->name != NULL && strcmp(child->name, name) == 0) { /*lint !e421*/
            return (device_node_s *)child;
        }
    }

    return NULL;
}

int bsp_dt_property_read_u8_array(const device_node_s *np,
    const char *propname, unsigned char *out_values, unsigned int sz)
{
    unsigned char *value = NULL;
    int len = 0;

    if (out_values == NULL || sz == 0) {
        return -1;
    }

    value = (unsigned char *)bsp_dt_get_property(np, propname, &len);
    if (value == NULL) {
        dt_pr_debug("no %s property\n", propname);
        return -1;
    }

    if (sz > (unsigned int)len) {
        dt_pr_err("oversize, len=%d\n", len);
        return -1;
    }

    while (sz--) {
        *out_values++ = *value++;
    }

    return 0;
}

int bsp_dt_property_read_u32_array(const device_node_s *np,
    const char *propname, u32 *out_values, size_t sz)
{
    unsigned int *value = NULL;
    int len = 0;

    if (out_values == NULL || sz == 0) {
        return -1;
    }

    value = (unsigned int *)bsp_dt_get_property(np, propname, &len);
    if (value == NULL) {
        dt_pr_debug("%s no %s property\n", np->name, propname);
        return -1;
    }

    if (sz * sizeof(*out_values) > (unsigned int)len) {
        dt_pr_err("oversize, len=%d\n", len);
        return -1;
    }

    while (sz--) {
        *out_values++ = (unsigned int)fdt32_to_cpu(*value++);
    }

    return 0;
}

int bsp_dt_property_read_u32_index(const device_node_s *np,
    const char *propname, unsigned int index, u32 *out_value)
{ /*lint !e578*/
    u32 *value = NULL;
    int len = 0;

    if (out_value == NULL) {
        return -1;
    }

    value = (u32 *)bsp_dt_get_property(np, propname, &len);
    if (value == NULL) {
        dt_pr_debug("%s no %s property\n", np->name, propname);
        return -1;
    }

    if ((index + 1) * sizeof(*out_value) > (unsigned int)len) {
        dt_pr_err("oversize, all-index=%d\n", (unsigned int)len / sizeof(u32));
        return -1;
    }

    *out_value = (u32)fdt32_to_cpu(value[index]);

    return 0;
}

int bsp_dt_property_read_u32(const device_node_s *np, const char *propname, u32 *out_value)
{
    u32 *value = NULL;
    int len = 0;

    if (out_value == NULL) {
        return -1;
    }

    value = (u32 *)bsp_dt_get_property(np, propname, &len);
    if (value == NULL) {
        dt_pr_debug("%s no %s property\n", np->name, propname);
        return -1;
    }

    *out_value = (u32)fdt32_to_cpu(*value);

    return 0;
}

const unsigned int *bsp_dt_prop_next_u32(struct property *prop, const unsigned int *curu, u32 *pu)
{
    const struct fdt_property *fdt_prop = (const struct fdt_property *)prop;
    const unsigned int *curv32 = curu;
    unsigned int len;

    if (fdt_prop == NULL) {
        return NULL;
    }

    len = fdt32_to_cpu(fdt_prop->len);
    if (len == 0) {
        return NULL;
    }

    if (curu == NULL) {
        curv32 = (const unsigned int *)fdt_prop->data;
        if (pu != NULL) {
            *pu = fdt32_to_cpu(*curv32);
        }
        return curv32;
    }

    curv32++;
    if ((unsigned int)(uintptr_t)curv32 >= ((unsigned int)(uintptr_t)fdt_prop->data + len)) {
        return NULL;
    }
    if (pu != NULL) {
        *pu = fdt32_to_cpu(*curv32);
    }
    return curv32;
}

int bsp_dt_property_read_u64(const struct device_node *np, const char *propname, u64 *out_value)
{
    __be32 *cell = NULL;
    u64 value;
    int len = 0;

    if (out_value == NULL) {
        return -1;
    }

    cell = (__be32 *)bsp_dt_get_property((const device_node_s *)np, propname, &len);
    if (cell == NULL) {
        dt_pr_debug("%s no %s property\n", np->name, propname);
        return -1;
    }
    if (len != sizeof(u64)) {
        dt_pr_debug("length error\n", np->name, propname);
        return -1;
    }

    value = fdt32_to_cpu(*cell);
    value <<= 32;  // 左移32位
    value |= fdt32_to_cpu(*(cell + 1));
    *out_value = value;

    return 0;
}

int bsp_dt_n_addr_cells(device_node_s *np)
{
    u32 cells;

    do {
        if (np->parent != NULL) {
            np = np->parent;
        }
        if (bsp_dt_property_read_u32(np, "#address-cells", &cells) == 0) {
            return cells;
        }
    } while (np->parent);

    return DT_NODE_ADDR_CELL_DEFAULT;
}

int bsp_dt_n_size_cells(device_node_s *np)
{
    u32 cells;

    do {
        if (np->parent != NULL) {
            np = np->parent;
        }
        if (bsp_dt_property_read_u32(np, "#size-cells", &cells) == 0) {
            return cells;
        }
    } while (np->parent);

    return DT_NODE_SIZE_CELL_DEFAULT;
}

int bsp_dt_property_read_string_index(const device_node_s *np,
    const char *propname, int index, const char **output)
{ /*lint !e578*/
    const char *p = NULL;
    int len = 0;
    unsigned int l = 0;
    int i;
    unsigned int total = 0;

    if (output == NULL) {
        return -1;
    }

    p = (const char *)bsp_dt_get_property(np, propname, &len);
    if (p == NULL) {
        dt_pr_debug("%s no property %s\n", np->name, propname);
        return -1;
    }
    if (strnlen(p, len) >= (unsigned int)len) { /*lint !e732*/
        dt_pr_err("%s property %s wrong length\n", np->name, propname);
        return -1;
    }

    for (i = 0; total < (unsigned int)len; total += l, p += l) {
        l = strlen(p) + 1; /*lint !e421*/
        if (i++ != index) {
            continue;
        } else {
            *output = p;
            return 0;
        }
    }
    return -1;
}

int bsp_dt_property_read_string(const device_node_s *np,
    const char *propname, const char **out_string)
{
    const char *p = NULL;
    int len = 0;

    if (out_string == NULL) {
        return -1;
    }

    p = (const char *)bsp_dt_get_property(np, propname, &len);
    if (p == NULL) {
        dt_pr_err("%s no property %s\n", np->name, propname);
        return -1;
    }
    if (strnlen(p, len) >= (unsigned int)len) { /*lint !e732*/
        dt_pr_err("%s property %s wrong length\n", np->name, propname);
        return -1;
    }

    *out_string = p;
    return 0;
}

int bsp_dt_property_count_strings(const device_node_s *np, const char *propname)
{
    const char *p = NULL;
    int len = 0;
    unsigned int l = 0;
    int i;
    unsigned int total = 0;

    p = (const char *)bsp_dt_get_property(np, propname, &len);
    if (p == NULL) {
        dt_pr_debug("%s no property %s\n", np->name, propname);
        return -1;
    }
    if (strnlen(p, len) >= (unsigned int)len) { /*lint !e732*/
        dt_pr_err("%s property %s wrong length\n", np->name, propname);
        return -1;
    }

    for (i = 0; total < (unsigned int)len; total += l, p += l) {
        l = strlen(p) + 1; /*lint !e421*/
        i++;
    }
    return i;
}

const char *bsp_dt_prop_next_string(struct property *prop, const char *cur)
{
    struct fdt_property *fdt_prop = (struct fdt_property *)prop;
    const char *curv = cur;
    const char *value = NULL;

    if (fdt_prop == NULL) {
        dt_pr_err("null ptr of prop arg\n");
        return NULL;
    }

    value = (const char *)fdt_prop->data;
    if (cur == NULL) {
        return value;
    }
    curv += strlen(cur) + 1; /*lint !e421*/
    if (curv >= value + fdt32_to_cpu(fdt_prop->len)) {
        return NULL;
    }

    return curv;
}

device_node_s *bsp_dt_find_node_by_phandle(uint32_t phandle)
{
    struct fdt_device_node *dev = NULL;
    struct fdt_device_node *dev_start = g_dt_ctrl.allnodes;
    struct fdt_device_node *dev_end = g_dt_ctrl.allnodes + g_dt_ctrl.node_num;

    for (dev = dev_start; dev < dev_end; ++dev) {
        if (dev->phandle == phandle) {
            return (device_node_s *)dev;
        }
    }

    return NULL;
}

device_node_s *bsp_dt_parse_phandle(const device_node_s *np, const char *phandle_name, int index)
{ /*lint !e578*/
    const unsigned int *phandle_ptr = NULL;
    int size = 0;

    if (index < 0) {
        return NULL;
    }

    phandle_ptr = bsp_dt_get_property(np, phandle_name, &size);
    if (phandle_ptr == NULL) {
        return NULL;
    }

    if (size < (int)sizeof(*phandle_ptr) * (index + 1)) {
        return NULL;
    }

    return bsp_dt_find_node_by_phandle(fdt32_to_cpu(*(phandle_ptr + index)));
}

u64 bsp_dt_read_number(const __be32 *cell, int size)
{
    u64 value = 0;

    if (cell == NULL || size == 0) {
        dt_pr_err("null ptr or zero size in dt_read_number\n");
        return value;
    }

    while (size--) {
        value = (value << 32) | fdt32_to_cpu(*(cell)); // 左移32位
        cell++;
    }

    return value;
}

static int dt_reg_info_get(device_node_s *np, int index, u64 *start, u64 *size)
{ /*lint !e578*/
    const u32 *reg = NULL;
    int cells_size;
    int len = 0;
    int na;
    int ns;

    reg = (const u32 *)bsp_dt_get_property(np, "reg", &len);
    if (reg == NULL) {
        dt_pr_debug("%s cannot find reg property\n", np->name);
        return -1;
    }

    na = bsp_dt_n_addr_cells(np);
    ns = bsp_dt_n_size_cells(np);
    cells_size = na + ns;

    if ((index * cells_size * (int)sizeof(int)) >= len) {
        dt_pr_err("%s:index = %d is out of range\n", np->name, index);
        return -1;
    }

    if (start != NULL) {
        *start = bsp_dt_read_number(reg + index * cells_size, na); /*lint !e679*/
    }
    if (size != NULL) {
        *size = bsp_dt_read_number(reg + index * cells_size + na, ns); /*lint !e679*/
    }

    return 0;
}

/*
 * read reg info and ioremap
 */
void __iomem *bsp_dt_iomap(device_node_s *np, int index)
{ /*lint !e578*/
    u64 start = 0;
    u64 size = 0;

    if (dt_reg_info_get(np, index, &start, &size)) {
        return NULL;
    }

    return ioremap(start, size);
}

static device_node_s *bsp_dt_irq_find_parent(device_node_s *child)
{
    device_node_s *p = NULL;
    uint32_t parent;

    do {
        if (bsp_dt_property_read_u32(child, "interrupt-parent", (u32 *)&parent)) {
            p = child->parent;
        } else  {
            p = bsp_dt_find_node_by_phandle(parent);
        }
        child = p;
    } while (p != NULL && bsp_dt_get_property(p, "#interrupt-cells", NULL) == NULL);

    return p;
}

static int bsp_dt_irq_parse(device_node_s *np, int index, struct of_phandle_args *irq_data)
{
    unsigned int start;
    int ret;
    int i;
    device_node_s *parent = NULL;
    u32 n_intr = 0;

    parent = bsp_dt_irq_find_parent(np);
    if (parent == NULL) {
        dt_pr_err("interrupt parent or cells not well configured\n");
        return -1;
    }
    if (bsp_dt_property_read_u32(parent, "#interrupt-cells", &n_intr) != 0) {
        dt_pr_err("get local interrupt-cells failed for %s\n", np->name);
        return -1;
    }

    if (n_intr > MAX_PHANDLE_ARGS) {
        dt_pr_err("#interrupt-cells num err\n");
        return -1;
    }

    start = (unsigned int)index * n_intr;
    for (i = 0; i < n_intr; ++i) {
        ret = bsp_dt_property_read_u32_index(np, "interrupts", start + i, &irq_data->args[i]);
        if (ret) {
            dt_pr_err("read interrupt data failed for %s\n", np->name);
            return -1;
        }
    }
    irq_data->args_count = n_intr;

    return 0;
}

static unsigned int bsp_dt_irq_map(device_node_s *np, struct of_phandle_args *irq_data)
{
    u32 n_intr = 0;
    unsigned int irq = 0;

    irq_data->np = of_find_node_by_name(NULL, "interrupt-controller");
    if (irq_data->np == NULL) {
        dt_pr_err("find gic failed for %s\n", np->name);
        return 0;
    }
    if (of_property_read_u32(irq_data->np, "#interrupt-cells", &n_intr) != 0) {
        dt_pr_err("get gic interrupts-cells failed for %s\n", np->name);
        goto out;
    }

    if (irq_data->args_count != n_intr) {
        dt_pr_err("irq parse failed with different interrupts-cells for %s\n", np->name);
        goto out;
    }
    irq = irq_create_of_mapping(irq_data);

out:
    of_node_put(irq_data->np);
    return irq;
}

unsigned int bsp_dt_irq_parse_and_map(device_node_s *np, int index)
{
    struct of_phandle_args oirq;

    if (np == NULL || index < 0) {
        return 0;
    }

    if (bsp_dt_irq_parse(np, index, &oirq) != 0) {
        dt_pr_err("irq parse failed for %s\n", np->name);
        return 0;
    }

    return bsp_dt_irq_map(np, &oirq);
}
