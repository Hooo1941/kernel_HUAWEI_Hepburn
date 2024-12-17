/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2020. All rights reserved.
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
#include <product_config.h>
#include <mdrv_memory.h>
#include <osl_malloc.h>
#include <securec.h>
#include <linux/of.h>
#include <bsp_print.h>
#include <bsp_dt.h>
#include "pinctrl_driver.h"

#undef THIS_MODU
#define THIS_MODU mod_pinctrl
#define ios_print_error(fmt, ...) (bsp_err("[pinctrl]:<%s> " fmt, __FUNCTION__, ##__VA_ARGS__))
#define ios_print_debug(fmt, ...) (bsp_debug("[pinctrl]:<%s> " fmt, __FUNCTION__, ##__VA_ARGS__))
void *g_pin_shmem_base = NULL;
struct pinctrl_property_ops g_pinctrl_property_ops;

static int pinctrl_shared_memory_init(void)
{
    phy_addr_t sha_mem_phy;
    unsigned int phy_size;
    g_pin_shmem_base = mdrv_mem_share_get("usshm_pinctrl", &sha_mem_phy, &phy_size, SHM_UNSEC);
    if (g_pin_shmem_base == NULL || phy_size < PINCTRL_SHMEM_SIZE) {
        ios_print_error("error: share mem get fail, base: 0x%p, size: 0x%x\n", g_pin_shmem_base, phy_size);
        return -1;
    }
    return 0;
}

static void convert_base_info(struct pinctrl *pctl, const struct pinctrl_property *prop)
{
    pctl->gpio = prop->gpio_grp * g_pinctrl_property_ops.max_pins + prop->gpio_bit;
    pctl->iomg1_off = prop->iomg1_off;
    pctl->iomg1_val = prop->iomg1_val;
    pctl->iocg_off = prop->iocg_off;
    pctl->iocg_val = prop->iocg_val;
    pctl->gpio_dir = prop->gpio_dir;
    pctl->gpio_val = prop->gpio_val;
    pctl->iomg2_off = 0xfff;
    pctl->iomg2_val = PINCONFIG_NF_VAL;
    pctl->reserve[0] = 0;
    pctl->reserve[1] = 0;
    pctl->reserve[2] = 0;
}

static void convert_extend_info(struct pinctrl *pctl, const struct pinctrl_property *prop)
{
    struct pinctrl_property_extend_info *extend_prop = (struct pinctrl_property_extend_info *)prop;
    pctl->iomg2_off = extend_prop->iomg2_off;
    pctl->iomg2_val = extend_prop->iomg2_val;
}

static void data_convert_base(struct pinctrl *pctl, const struct pinctrl_property *prop, unsigned int len)
{
    unsigned int i;

    for (i = 0; i < len; i++) {
        convert_base_info(&pctl[i], &prop[i]);
    }
}

static void data_convert_extend(struct pinctrl *pctl, const struct pinctrl_property *prop, unsigned int len)
{
    unsigned int i;
    struct pinctrl_property_extend_info *extend_prop = (struct pinctrl_property_extend_info *)prop;

    for (i = 0; i < len; i++) {
        convert_base_info(&pctl[i], (const struct pinctrl_property *)&extend_prop[i]);
        convert_extend_info(&pctl[i], (const struct pinctrl_property *)&extend_prop[i]);
    }
}

int pinctrl_set_sharemem(struct device_node *dev_node, enum modem_pin_ctrl_module module, void *dts_buf, u32 buf_size, u32 *pin_nums)
{
    int ret;
    u32 size, i, offset_addr = 0;
    u32 pin_number[MODEM_PIN_MODE_BUTT] = {0};
    struct pinctrl_head *head = NULL;
    char pinctrl_name[MODEM_PIN_MODE_BUTT][PIN_NAME_LEN] = {"pinctrl-0", "pinctrl-1", "pinctrl-2"};
    struct modem_pinctrl_head *phead = (struct modem_pinctrl_head *)g_pin_shmem_base;

    ret = of_property_read_u32_array(dev_node, "pinctrl-num", pin_number, MODEM_PIN_MODE_BUTT);
    if (ret) {
        ios_print_error("get pin_number fail, ret :%d\n", ret);
        return ret;
    }

    for (i = 0; i < MODEM_PIN_MODE_BUTT; i++) {
        *pin_nums += pin_number[i];
    }
    size = sizeof(struct modem_pinctrl_head) + (*pin_nums) * sizeof(struct pinctrl);
    if (size > PINCTRL_SHMEM_SIZE) {
        ios_print_error("modem pinctrl memory %d expend %d!\n", size, PINCTRL_SHMEM_SIZE);
        return -1;
    }

    head = &(phead->head[module][0]);
    for (i = 0; i < MODEM_PIN_MODE_BUTT; i++) {
        head->size = pin_number[i];
        if (module == MODEM_PIN_CP_DRV && i == 0) {
            offset_addr = sizeof(struct modem_pinctrl_head);
        } else {
            offset_addr = (head - 1)->offset + (head - 1)->size * sizeof(struct pinctrl);
        }
        head->offset = offset_addr;

        if (g_pinctrl_property_ops.element_size * pin_number[i] > PINCTRL_SHMEM_SIZE) {
            ios_print_error("module is:%u, pinctrl_number(%u) err, mode is %u\n", module, pin_number[i], i);
            return -1;
        }
        (void)memset_s((void *)dts_buf, buf_size, 0, buf_size);
        ret = of_property_read_u32_array(dev_node, pinctrl_name[i], dts_buf,
            g_pinctrl_property_ops.element_size * pin_number[i] / sizeof(u32));
        if (ret) {
            ios_print_error("Could not get property, ret=%d, node name:%s\n", ret, pinctrl_name[i]);
            return -1;
        }

        g_pinctrl_property_ops.data_convert((struct pinctrl *)((void *)phead + offset_addr), (struct pinctrl_property *)dts_buf,
            head->size);

        head++;
    }

    return 0;
}

static int pinctrl_info_init(struct device_node *dev_node)
{
    struct modem_pinctrl_head *phead = (struct modem_pinctrl_head *)g_pin_shmem_base;
    void *dts_buf = NULL;
    struct device_node *child_node = NULL;
    u32 module = 0;
    u32 pin_nums = MODEM_PIN_CP_DRV;
    int ret, child_node_count;

    (void)memset_s((void *)phead, PINCTRL_SHMEM_SIZE, 0, PINCTRL_SHMEM_SIZE);

    child_node_count = of_get_child_count(dev_node);
    if (child_node_count > MODEM_PIN_CP_BUTT) {
        ios_print_error("child node count(%d) err\n", child_node_count);
        return -1;
    }

    dts_buf = kmalloc(PINCTRL_SHMEM_SIZE, GFP_KERNEL);
    if (dts_buf == NULL) {
        ios_print_error("malloc dts buf fail\n");
        return -1;
    }

    if (child_node_count == 0) { // 兼容老平台
        phead->pinctrl_module_count = 1;
        ret = pinctrl_set_sharemem(dev_node, MODEM_PIN_CP_DRV, dts_buf, PINCTRL_SHMEM_SIZE, &pin_nums);
    } else {
        phead->pinctrl_module_count = child_node_count;
        bsp_dt_for_each_child_of_node(dev_node, child_node) {
            ret = pinctrl_set_sharemem(child_node, module, dts_buf, PINCTRL_SHMEM_SIZE, &pin_nums);
            if (ret) {
                goto out;
            }
            module++;
        }
    }

out:
    kfree(dts_buf);
    dts_buf = NULL;
    return ret;
}

void modem_pinctrl_debug(int module, int mode)
{
    unsigned int i;
    unsigned int size;
    struct pinctrl *pctl = NULL;
    struct modem_pinctrl_head *phead = g_pin_shmem_base;

    if (module < MODEM_PIN_CP_DRV || module >= MODEM_PIN_CP_BUTT ||
        mode < MODEM_PIN_INIT || mode >= MODEM_PIN_MODE_BUTT) {
        ios_print_error("ctrl module %u or mode %u err\n", module, mode);
        return;
    }

    if (module >= phead->pinctrl_module_count) {
        ios_print_error("module(%u) is larger than %u, not support\n", module, phead->pinctrl_module_count);
        return;
    }

    size = phead->head[module][mode].size;
    pctl = (struct pinctrl *)((char *)phead + phead->head[module][mode].offset);

    ios_print_error("iocfg table size is %d\n", size);
    ios_print_error("  gpio  iomg1  iomg2 iocg  dir  value\n");

    for (i = 0; i < size; i++) {
        ios_print_error("%5d %5d %5d %5d %5d %5d \n", pctl[i].gpio, pctl[i].iomg1_val, pctl[i].iomg2_val, pctl[i].iocg_val, pctl[i].gpio_dir,
            pctl[i].gpio_val);
    }
}

static int property_ops_init(void)
{
    unsigned int ret;
    device_node_s *dev_node = NULL;
    dev_node = bsp_dt_find_compatible_node(NULL, NULL, "modem_pinctrl_ctrl_info");
    if (dev_node == NULL) {
        ios_print_error("Can't find pinctrl info node\n");
        return -1;
    }

    ret = (unsigned int)bsp_dt_property_read_u32(dev_node, "max_pins", &g_pinctrl_property_ops.max_pins);
    ret |= (unsigned int)bsp_dt_property_read_u32(dev_node, "extend_enabled", &g_pinctrl_property_ops.extend_enabled);
    if (ret) {
        ios_print_error("Could not get pinctrl property info, ret=%d\n", ret);
        return ret;
    }

    if (g_pinctrl_property_ops.extend_enabled) {
        g_pinctrl_property_ops.element_size = sizeof(struct pinctrl_property_extend_info);
        g_pinctrl_property_ops.data_convert = data_convert_extend;
    } else {
        g_pinctrl_property_ops.element_size = sizeof(struct pinctrl_property);
        g_pinctrl_property_ops.data_convert = data_convert_base;
    }

    return 0;
}

/* Modem pinctrl extract from the device tree. */
int bsp_modem_pinctrl_dts_parse(void)
{
    int ret;
    struct device_node *dev_node = NULL;

    ios_print_error("parse dts start!\n");

    if (pinctrl_shared_memory_init()) {
        return -1;
    }

    ret = property_ops_init();
    if (ret) {
        return ret;
    }

    dev_node = of_find_compatible_node(NULL, NULL, "arm,modem_pinctrl");
    if (dev_node == NULL) {
        ios_print_error("Can't find modem pinctrl node.\n");
        return -1;
    }

    ret = pinctrl_info_init(dev_node);
    if (ret < 0) {
        ios_print_error("pinctrl info init fail, ret :%d\n", ret);
    }
    ios_print_error("parse dts end!\n");
    return ret;
}
#if !IS_ENABLED(CONFIG_MODEM_DRIVER)
module_init(bsp_modem_pinctrl_dts_parse);
#endif

EXPORT_SYMBOL_GPL(bsp_modem_pinctrl_dts_parse);
