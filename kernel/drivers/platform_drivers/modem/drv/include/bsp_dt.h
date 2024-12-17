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

#ifndef __BSP_DT_H__
#define __BSP_DT_H__

#include <product_config.h>
#include <osl_types.h>

//注： 因部分版本启动后对dt内存进行了释放, 驱动编码时请调用bsp_dt接口，该接口内部封装了对内存释放情况的判断拦截。


int bsp_dt_load_and_verify_ccore_dt_image(void);

#if defined(BSP_CONFIG_PHONE_TYPE) && defined(CONFIG_KERNEL_DTS_DECOUPLED)
struct property;

struct fdt_device_node {
    int offset;
    const char *name;
    const char *type;
    const char *compatible;
    uint32_t phandle;
    struct fdt_device_node *parent;
    struct fdt_device_node *child;
    struct fdt_device_node *sibling;
    struct fdt_device_node *child_tail;
    unsigned int prop_start;
    unsigned int prop_count;
};

struct fdt_device_id {
    char name[32];        // 32为name的最大长度
    char type[32];        // 32为type的最大长度
    char compatible[128]; // 128为compatible的最大长度
    const void *data;
};

typedef struct fdt_device_node device_node_s;
typedef struct fdt_device_id device_id_s;

#else

#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/of_gpio.h>
#include <linux/of_net.h>
#include <linux/phy.h>
#include <linux/clk.h>
#include <linux/clk-provider.h>

typedef phandle phandle_t;
typedef struct device_node device_node_s;
typedef struct of_device_id device_id_s;

#endif /* non-decoupled */

unsigned int bsp_dt_get_release_status(void);

device_node_s *bsp_dt_find_node_by_path(const char *path);
device_node_s *bsp_dt_find_compatible_node(device_node_s *from, const char *type, const char *compat);
int bsp_dt_property_read_u32(const device_node_s *np, const char *propname, u32 *out_value);
int bsp_dt_property_read_u32_array(const device_node_s *np, const char *propname, u32 *out_values, size_t sz);
int bsp_dt_property_read_u32_index(const device_node_s *np, const char *propname, u32 index, u32 *out_value);
const unsigned int *bsp_dt_prop_next_u32(struct property *prop, const unsigned int *curu, u32 *pu);
int bsp_dt_get_child_count(const device_node_s *np);
device_node_s *bsp_dt_get_child_by_name(const device_node_s *np, const char *name);
int bsp_dt_n_addr_cells(device_node_s *np);
int bsp_dt_n_size_cells(device_node_s *np);
const void *bsp_dt_get_property(const device_node_s *np, const char *name, int *lenp);
u64 bsp_dt_read_number(const __be32 *cell, int size);
struct property *bsp_dt_find_property(const device_node_s *np, const char *name, int *lenp);
int bsp_dt_property_read_string(const device_node_s *np, const char *propname, const char **out_string);
int bsp_dt_property_read_string_index(const device_node_s *np, const char *propname, int index, const char **output);
bool bsp_dt_device_is_available(const device_node_s *device);
device_node_s *bsp_dt_get_next_available_child(const device_node_s *np, device_node_s *prev);

void __iomem *bsp_dt_iomap(device_node_s *np, int index);
unsigned int bsp_dt_irq_parse_and_map(device_node_s *np, int index);
int bsp_dt_property_count_strings(const device_node_s *np, const char *propname);
const char *bsp_dt_prop_next_string(struct property *prop, const char *cur);

#if defined(BSP_CONFIG_PHONE_TYPE) && defined(CONFIG_KERNEL_DTS_DECOUPLED)
#define bsp_dt_for_each_child_of_node(parent_np, child_np)                                                      \
    for (child_np = ((parent_np) != NULL ? (parent_np)->child : NULL); (parent_np) != NULL && child_np != NULL; \
        child_np = child_np->sibling)

#define bsp_dt_for_each_available_child_of_node(parent, child)                 \
    for (child = bsp_dt_get_next_available_child(parent, NULL); child != NULL; \
        child = bsp_dt_get_next_available_child(parent, child))

#define bsp_dt_property_for_each_u32(np, propname, prop, p, u)                                                 \
    for (prop = bsp_dt_find_property(np, propname, NULL), p = bsp_dt_prop_next_u32(prop, NULL, &u); p != NULL; \
        p = bsp_dt_prop_next_u32(prop, p, &u))

#define bsp_dt_property_for_each_string(np, propname, prop, s)                                                \
    for (prop = bsp_dt_find_property(np, propname, NULL), s = bsp_dt_prop_next_string(prop, NULL); s != NULL; \
        s = bsp_dt_prop_next_string(prop, s))
#else // kernel
#define bsp_dt_for_each_child_of_node(parent, child) for_each_child_of_node(parent, child)

#define bsp_dt_for_each_available_child_of_node(node, child) for_each_available_child_of_node(node, child)

#define bsp_dt_property_for_each_u32(child, name, prop, p, u) of_property_for_each_u32(child, name, prop, p, u)

#define bsp_dt_property_for_each_string(np, propname, prop, s) of_property_for_each_string(np, propname, prop, s)

int bsp_dt_property_count_u32_elems(const device_node_s *np, const char *propname);

int bsp_dt_property_read_u64(const device_node_s *np, const char *propname, u64 *out_value);

bool bsp_dt_property_read_bool(const device_node_s *np, const char *propname);

device_node_s *bsp_dt_parse_phandle(const device_node_s *np, const char *phandle_name, int index);

device_node_s *bsp_dt_get_parent(const device_node_s *node);

device_node_s *bsp_dt_find_node_by_name(device_node_s *from, const char *name);

int bsp_dt_get_named_gpio(device_node_s *np, const char *propname, int index);

int bsp_dt_clk_add_provider(device_node_s *np, struct clk *(*clk_src_get)(struct of_phandle_args *args, void *data),
    void *data);

int bsp_dt_get_phy_mode(device_node_s *np, phy_interface_t *interface);

const void *bsp_dt_get_mac_address(device_node_s *np);

struct platform_device *bsp_dt_find_device_by_node(device_node_s *np);

int bsp_dt_platform_populate(device_node_s *root, const struct of_device_id *matches,
    const struct of_dev_auxdata *lookup, struct device *parent);

void bsp_dt_platform_depopulate(struct device *parent);

int bsp_dt_dma_configure(struct device *dev, device_node_s *np, bool force_dma);

const struct of_device_id *bsp_dt_match_device(const struct of_device_id *matches, const struct device *dev);

int bsp_dt_driver_match_device(struct device *dev, const struct device_driver *drv);

int bsp_dt_device_uevent_modalias(struct device *dev, struct kobj_uevent_env *env);

int bsp_dt_alias_get_id(device_node_s *np, const char *stem);

int bsp_dt_device_is_compatible(device_node_s *np,const char *name);

void bsp_dt_add_release_dev(struct device *dev);
#endif

#endif /* __BSP_DT_H__ */
