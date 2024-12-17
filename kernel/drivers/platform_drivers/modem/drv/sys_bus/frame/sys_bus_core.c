/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
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

#include "sys_bus_core.h"
#include "sys_bus_api.h"
#include "sys_odt.h"
#include "sys_acore.h"
#include "sys_hdlc.h"

#include "securec.h"

void sys_bus_init_observers(sys_bus_s* sysbus, struct device_node* pressure_np);

sys_bus_s* g_sys_bus = NULL;

typedef void (*sysbus_of_init_fn)(sys_bus_s*, struct device_node*);
typedef void (*sysbus_observer_of_init_fn)(struct device_node*);

static struct of_device_id g_sys_bus_of_devices[] = {
    { .compatible = "sys_bus, bus_pressure", .data = sys_bus_init_observers },
    {},
};

// 注：IP测试模块的初始化函数在此配置，并引用相关头文件
static struct of_device_id g_sys_bus_observers[] = {
    { .compatible = "bus_pressure, odt", .data = sys_odt_init },
    { .compatible = "bus_pressure, acore", .data = sys_acore_sysbus_test_init },
    { .compatible = "bus_pressure, hdlc", .data = sys_hdlc_sysbus_test_init },
    {},
};

void sys_bus_init_observers(sys_bus_s* sysbus, struct device_node* pressure_np)
{
    const struct of_device_id* match = NULL;
    sysbus_observer_of_init_fn init_func = NULL;
    struct device_node* np = NULL;
    const char* status = NULL;
    int ret;

    np = pressure_np;
    for_each_matching_node_and_match(np, g_sys_bus_observers, &match) {
        // 如果IP测试模块在sys_bus.dts中没有配置status属性，或者status属性不为ok，则不对其进行初始化。
        ret = of_property_read_string(np, SYS_BUS_DTS_STATUS, &status);
        if (ret != 0) {
            sys_bus_trace("status is not configured for: %s!\n", np->name);
            continue;
        }
        if ((status != NULL) && (strncmp(status, SYS_BUS_DTS_STATUS_ENABLE, strlen(SYS_BUS_DTS_STATUS_ENABLE)) != 0)) {
            sys_bus_trace("status is disabled for: %s!\n", np->name);
            continue;
        }

        init_func = match->data; /*lint !e158*/
        init_func(np);
    }

    sys_bus_err("init observers ok.\n");
    return;
}

int sys_bus_register_observer(const char* name, sys_bus_observer_entry entry)
{
    sys_bus_observer_s* observer = NULL;
    int ret;

    sys_bus_err("To register observer %s\n", name);

    if (name == NULL) {
        return E_SYS_BUS_RET_NULL_PTR;
    }

    list_for_each_entry(observer, &(g_sys_bus->observers), node) {
        if (strncmp(observer->name, name, SYS_BUS_NAME_LEN - 1) == 0) {
            sys_bus_err("observer already exists!\n");
            return E_SYS_BUS_RET_ALREADY_EXIST;
        }
    }

    observer = (sys_bus_observer_s*)kzalloc(sizeof(sys_bus_observer_s), GFP_KERNEL);
    if (observer == NULL) {
        sys_bus_err("alloc observer mem failed!\n");
        return E_SYS_BUS_RET_NULL_PTR;
    }

    observer->entry = entry;

    ret = strncpy_s(observer->name, SYS_BUS_NAME_LEN, name, strlen(name));
    if (ret != 0) {
        kfree(observer);

        sys_bus_err("cpy name failed!\n");
        return E_SYS_BUS_RET_ERR;
    }

    list_add_tail(&(observer->node), &(g_sys_bus->observers));

    return E_SYS_BUS_RET_OK;
}

void sys_bus_remove_all_observers(void)
{
    struct list_head* head = &(g_sys_bus->observers);
    struct list_head* observer = head->next;

    while (observer != head) {
        list_del(observer);

        sys_bus_err("observer: %s is removed!\n", ((sys_bus_observer_s*)observer)->name);
        kfree(observer);

        observer = head->next;
    }

    return;
}

void sys_bus_remove_specify_observer(const char* name)
{
    sys_bus_observer_s* observer = NULL;

    list_for_each_entry(observer, &(g_sys_bus->observers), node) {
        if (strncmp(observer->name, name, SYS_BUS_NAME_LEN - 1) == 0) {
            list_del(&(observer->node));

            sys_bus_err("observer: %s is removed!\n", observer->name);
            kfree(observer);
            return;
        }
    }

    sys_bus_err("rmv observer: %s which is not found!\n", name);
    return;
}

void sys_bus_remove_observer(const char* name)
{
    sys_bus_err("To rmv observer: %s.\n", name);

    if (name == NULL) {
        sys_bus_remove_all_observers();
    } else {
        sys_bus_remove_specify_observer(name);
    }

    return;
}

void sys_bus_notify_all_observers(sys_bus_evt_e event)
{
    sys_bus_observer_s* observer = NULL;
    int ret;

    list_for_each_entry(observer, &(g_sys_bus->observers), node) {
        ret = observer->entry(event);
        if (ret != E_SYS_BUS_RET_OK) {
            sys_bus_err("notify observer: %s with event %d failed!\n", observer->name, event);
        }
    }

    return;
}

void sys_bus_notify_specify_observer(sys_bus_evt_e event, const char* name)
{
    sys_bus_observer_s* observer = NULL;
    int ret;

    list_for_each_entry(observer, &(g_sys_bus->observers), node) {
        if (strncmp(observer->name, name, SYS_BUS_NAME_LEN - 1) != 0) {
            continue;
        }

        ret = observer->entry(event);
        if (ret != E_SYS_BUS_RET_OK) {
            sys_bus_err("notify observer: %s with event %d failed!\n", observer->name, event);
        }

        return;
    }

    sys_bus_err("notify observer: %s which is not found!\n", name);
    return;
}

// name为NULL，则通知所有IP测试模块；否则只通知name对应的IP测试模块。
void sys_bus_notify_observer(sys_bus_evt_e event, const char* name)
{
    sys_bus_err("To notify observer: %s, evt: %d.\n", name, event);

    if (event >= E_SYS_BUS_EVT_BUTT) {
        return;
    }

    if (name == NULL) {
        sys_bus_notify_all_observers(event);
    } else {
        sys_bus_notify_specify_observer(event, name);
    }

    if (event == E_SYS_BUS_EVT_START) {
        g_sys_bus->is_started = E_SYS_BUS_STATUS_STARTED;
    } else if (event == E_SYS_BUS_EVT_STOP) {
        g_sys_bus->is_started = E_SYS_BUS_STATUS_STOPPED;
    }

    return;
}

// 收到某个IP模块的校验失败，则停止所有IP模块的压测。
void sys_bus_verify_failed(const char* name)
{
    unsigned long flags;

    sys_bus_err("%s verify failed!\n", name);

    if (g_sys_bus->is_started == E_SYS_BUS_STATUS_STARTED) {
        spin_lock_irqsave(&(g_sys_bus->verify_lock), flags);

        if (g_sys_bus->is_started == E_SYS_BUS_STATUS_STARTED) {
            sys_bus_notify_observer(E_SYS_BUS_EVT_STOP, NULL);
        }

        spin_unlock_irqrestore(&(g_sys_bus->verify_lock), flags);
    }

    return;
}

void sys_bus_display(void)
{
    sys_bus_observer_s* observer = NULL;

    sys_bus_err("status     : %d\n", g_sys_bus->is_started);
    sys_bus_err("of_node    : %s\n", g_sys_bus->of_node->name);

    list_for_each_entry(observer, &(g_sys_bus->observers), node) {
        sys_bus_err("observer: %s\n", observer->name);
    }

    return;
}

int sys_bus_init(void)
{
    sys_bus_s* sysbus = NULL;
    sysbus_of_init_fn init_fn = NULL;
    const struct of_device_id* match = NULL;
    struct device_node* np = NULL;
    const char* status = NULL;
    s32 ret;

    sys_bus_err("init start.\n");
    np = of_find_compatible_node(NULL, NULL, "sys_bus");
    if (np == NULL) {
        sys_bus_trace("find sys_bus node error.\n");
        return 0;
    }

    ret = of_property_read_string(np, SYS_BUS_DTS_STATUS, &status);
    if (ret != 0) {
        sys_bus_trace("find status error.\n");
        return 0;
    }
    if ((status != NULL) && (strncmp(status, SYS_BUS_DTS_STATUS_ENABLE, strlen(SYS_BUS_DTS_STATUS_ENABLE)) != 0)) {
        sys_bus_trace("sys_bus init stop.\n");
        return 0;
    }
    sysbus = kzalloc(sizeof(sys_bus_s), GFP_KERNEL);
    if (sysbus == NULL) {
        sys_bus_trace("failed to allocate sysbus!\n");
        return -1;
    }

    ret = memset_s(sysbus, sizeof(sys_bus_s), 0, sizeof(sys_bus_s));
    if (ret != 0) {
        kfree(sysbus);

        sys_bus_trace("memset sysbus error.\n");
        return -1;
    }

    INIT_LIST_HEAD(&(sysbus->observers));
    spin_lock_init(&(sysbus->verify_lock));
    sysbus->of_node = np;
    g_sys_bus = sysbus;

    for_each_matching_node_and_match(np, g_sys_bus_of_devices, &match) {
        init_fn = (sysbus_of_init_fn)match->data;
        init_fn(sysbus, np);
    }

    sys_bus_err("init ok.\n");
    return 0;
}

#if !IS_ENABLED(CONFIG_MODEM_DRIVER)
late_initcall(sys_bus_init);
#endif
