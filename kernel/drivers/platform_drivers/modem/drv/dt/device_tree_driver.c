/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
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
#include <mdrv_memory_layout.h>
#include <bsp_dt.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/memblock.h>
#include <linux/mm.h>
#include <linux/libfdt.h>
#include <linux/of.h>
#include <linux/slab.h>
#include <linux/kref.h>
#include <linux/kobject.h>
#include <linux/platform_device.h>
#include <linux/device/bus.h>
#include <linux/cpu.h>
#include <linux/list.h>

#ifndef BSP_CONFIG_PHONE_TYPE
#include <hooks/reg/iot_hook_dt.h>
#include <bsp_gpio.h>

struct release_dev {
    struct list_head list;
    struct device *dev;
};

struct dt_release g_dt_release = { 0 };
static struct platform_device g_dt_free_device = {
    .name = "dt_free",
    .id = -1,
};
static struct list_head g_dt_release_list_head;

/*
 * Function:  bsp_dt_get_release_status
 * Brief: 获取dt内存释放状态
 * Return: 0 未释放 , 1 已释放
 */
unsigned int bsp_dt_get_release_status(void)
{
    if (g_dt_release.state == 0) {
        return 0;
    }
    dt_pr_err("The dt memory has been freed, please check the legality of the code\n");
    return 1;
}

/*
 * Function:  bsp_dt_get_release_support
 * Brief: 是否支持对dt内存进行释放
 * Return: 0 不支持 , 1 支持
 */
unsigned int bsp_dt_get_release_support(void)
{
    if (g_dt_release.support == 0) {
        dt_pr_err("not support dt memory free feature\n");
        return 0;
    }
    return 1;
}
EXPORT_SYMBOL(bsp_dt_get_release_support);

/*
 * Function:  bsp_dt_get_free_size
 * Brief: 获取释放掉的dt内存大小
 * Return: 释放掉的dt内存大小, 单位KB
 */
unsigned int bsp_dt_get_free_size(void)
{
    return g_dt_release.free_size;
}
EXPORT_SYMBOL(bsp_dt_get_free_size);

void bsp_dt_set_release_status(void)
{
    g_dt_release.state++;
}

/*
 * Function:  unflatten_dt_alloc
 * Brief: 模拟dt镜像展开时的内存分配计算
 * Return: 内存大小
 */
unsigned long unflatten_dt_alloc(unsigned long total, unsigned long size, unsigned long align)
{
    total = ALIGN(total, align);
    total += size;
    return total;
}

/*
 * Function:  bsp_dt_release_unflatten_memory
 * Brief: 释放dt镜像展开所分配的内存
 * Return: none
 */
void bsp_dt_release_unflatten_memory(void)
{
    struct device_node *np = NULL;
    struct property *pp = NULL;
    void *mem = of_root;
    unsigned long total = 0, len = 0;

    for_each_of_allnodes(np)
    {
        len = sizeof(struct device_node) + strlen(np->full_name) + 1;
        total = unflatten_dt_alloc(total, len, __alignof__(struct device_node));

        for_each_property_of_node(np, pp)
        {
            len = sizeof(struct property);

            if (!strcmp(pp->name, "name")) {
                len += strlen(pp->value) + 1;
            }

            total = unflatten_dt_alloc(total, len, __alignof__(struct property));
        }
    }
    total = ALIGN(total, 4);
    if (be32_to_cpup(mem + total) != 0xdeadbeef) {
        dt_pr_err("unflatten memory end error 0x%x \n", be32_to_cpup(mem + total));
    }

    memblock_free(__pa((uintptr_t)mem), total + 4); // 4为存放魔术字0xdeadbeef的内存
    free_reserved_area((void *)mem, (void *)(mem + total), -1, "dts_populate");
    of_root = NULL;
}

/*
 * Function:  bsp_dt_release_unflatten_memory
 * Brief: 释放存放dt原始镜像的预留内存
 * Return: none
 */
void bsp_dt_release_dtb_memory(void)
{
    unsigned long dt_addr = 0;
    unsigned int size = 0;
    void *dt_virt = NULL;
    dt_addr = mdrv_mem_region_get("acore_dts_ddr", &size);

    dt_virt = __va(dt_addr);
    if (fdt_magic(dt_virt) != FDT_MAGIC) {
        dt_pr_err(" invalid magic\n");
    }

    size = fdt_totalsize(dt_virt);

    memblock_free(dt_addr, size);
    free_reserved_area((void *)dt_virt, (void *)(dt_virt + size), -1, "dtb");
}

/*
 * Function:  bsp_dt_sysfs_remove_bin_file
 * Brief: 删除sysfs文件系统中dt相关的文件
 * Return: none
 */
void bsp_dt_sysfs_remove_bin_file(struct device_node *np, struct property *prop)
{
    sysfs_remove_bin_file(&np->kobj, &prop->attr);
    kfree(prop->attr.attr.name);
}

int bsp_dt_node_is_attached(struct device_node *node)
{
    return node && node->kobj.state_in_sysfs;
}

/*
 * Function:  bsp_dt_release_node_sysfs
 * Brief: 删除sysfs文件系统中的dt节点, 位于sys/firmware/
 * Return: none
 */
void bsp_dt_release_node_sysfs(void)
{
    struct property *pp = NULL;
    struct device_node *np = NULL;

    if (!IS_ENABLED(CONFIG_SYSFS))
        return;

    for_each_of_allnodes(np)
    {
        if (bsp_dt_node_is_attached(np)) {
            for_each_property_of_node(np, pp)
            {
                bsp_dt_sysfs_remove_bin_file(np, pp);
            }

            kobject_del(&np->kobj);
            kobject_put(&np->kobj);
        }

        of_node_put(np);
    }
    kset_unregister(of_kset);
    kernfs_remove_by_name(firmware_kobj->sd, "fdt");
}

/*
 * Function:  bsp_dt_release_syslink
 * Brief: 解除sysfs中platform设备、GPIO设备中的of_node引用
 * Return: none
 */
void bsp_dt_release_syslink(void)
{
    struct device_node *np = NULL;
    struct device *dev = NULL;
    struct bus_type *gpio_bus_type;
    struct bus_type *cpu_bus_type;
    if (!IS_ENABLED(CONFIG_SYSFS))
        return;

    gpio_bus_type = bsp_get_gpio_bus_type();
    dev = get_cpu_device(0);
    cpu_bus_type = dev->bus;
    for_each_of_allnodes(np)
    {
        dev = bus_find_device_by_of_node(&platform_bus_type, np);
        if (dev) {
            sysfs_remove_link(&dev->kobj, "of_node");
            dev->of_node = NULL;
            dt_pr_debug("release %s of_node \n", to_platform_device(dev)->name);
        }
        if (cpu_bus_type) {
            dev = bus_find_device_by_of_node(cpu_bus_type, np);
            if (dev) {
                sysfs_remove_link(&dev->kobj, "of_node");
                dev->of_node = NULL;
            }
        }
        if (gpio_bus_type) {
            dev = bus_find_device_by_of_node(gpio_bus_type, np);
            if (dev) {
                sysfs_remove_link(&dev->kobj, "of_node");
                dev->of_node = NULL;
            }
        }
    }
}

void bsp_dt_release_syslink_supplement(void)
{
    struct list_head *p = NULL;
    struct list_head *n = NULL;
    struct release_dev *release_dev = NULL;
    list_for_each_safe(p, n, &g_dt_release_list_head)
    {
        release_dev = list_entry(p, struct release_dev, list);
        if (release_dev != NULL && release_dev->dev != NULL) {
            sysfs_remove_link(&release_dev->dev->kobj, "of_node");
            release_dev->dev->of_node = NULL;
            dt_pr_err("release dev:%s\n", dev_name(release_dev->dev));
        }
    }
}

/*
 * Function:  dt_get_sys_available_mem_size
 * Brief: 获取当前系统available内存大小
 * Return: available内存大小
 */
unsigned long long dt_get_sys_available_mem_size(void)
{
    struct sysinfo si;
    unsigned long long available;
    unsigned int uint_kb;
    si_meminfo(&si);
    uint_kb = si.mem_unit / 1024; // 除1024, 换算为KB 
    available = (unsigned long long)(si_mem_available() * uint_kb);
    return available;
}

/*
 * Function:  bsp_dt_sysfs_remove_bin_file
 * Brief: 写节点进行内存释放
 * Return: count
 */
ssize_t dt_free_triggr_set(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    unsigned long long available;

    if (bsp_dt_get_release_support() == 0 || bsp_dt_get_release_status()) {
        return (ssize_t)count;
    }

    available = dt_get_sys_available_mem_size();
    bsp_dt_release_syslink();
    bsp_dt_release_syslink_supplement();
    bsp_dt_release_node_sysfs();
    bsp_dt_release_unflatten_memory();
    bsp_dt_release_dtb_memory();
    bsp_dt_set_release_status();
    dt_pr_err("dts memory freed ok. state: %d\n", g_dt_release.state);
    g_dt_release.free_size = dt_get_sys_available_mem_size() - available;

    return (ssize_t)count;
}


// DEVICE_ATTR(_name, _mode, _show, _store)
static DEVICE_ATTR(dt_free_trigger, S_IWUSR, NULL, dt_free_triggr_set);

void iot_hook_dt_add_release_dev(struct device *dev)
{
    struct release_dev *add_dev = NULL;

    if (dev == NULL) {
        dt_pr_err("dev is null\n");
        return;
    }
    add_dev = kzalloc(sizeof(struct release_dev), GFP_KERNEL);
    if (add_dev == NULL) {
        dt_pr_err("<%s> kzalloc failed\n", __func__);
        return;
    }
    add_dev->dev = dev;
    list_add(&add_dev->list, &g_dt_release_list_head);
}

void bsp_dt_add_release_dev(struct device *dev)
{
    iot_hook_dt_add_release_dev(dev);
}

int dt_device_init(void)
{
    int ret = 0;

    device_node_s *node = bsp_dt_find_node_by_path("/dt_free");
    if (node == NULL) {
        dt_pr_err("can not find dt_free node.\n");
        return -1;
    }
    ret = bsp_dt_property_read_u32(node, "support", &g_dt_release.support);
    if (ret) {
        dt_pr_err("can not find support property.\n");
        return ret;
    }

    ret = platform_device_register(&g_dt_free_device);
    if (ret) {
        dt_pr_err("register dt free device failed.\n");
        return ret;
    }

    ret = device_create_file(&g_dt_free_device.dev, &dev_attr_dt_free_trigger);
    if (ret) {
        dt_pr_err("dt device create file failed.\n");
        return ret;
    }
    dt_pr_err("register dt free device success.\n");

    INIT_LIST_HEAD(&g_dt_release_list_head);
    REGISTER_IOT_HOOK(iot_hook_dt_add_release_dev);

    return ret;
}


void dt_device_exit(void)
{
    platform_device_unregister(&g_dt_free_device);
}


#if !IS_ENABLED(CONFIG_MODEM_DRIVER)
arch_initcall(dt_device_init);
module_exit(dt_device_exit);
#endif

#else
unsigned int bsp_dt_get_release_status(void)
{
    return 0;
}

#endif
