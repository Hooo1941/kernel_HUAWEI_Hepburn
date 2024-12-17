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

/*
 * 1 头文件包含
 */
#include <product_config.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/semaphore.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/uaccess.h>
#include <linux/kthread.h>
#include <linux/platform_device.h>
#include <linux/suspend.h>
#include <linux/pm_wakeup.h>
#include <linux/pm.h>
#include <linux/delay.h>
#include <linux/jiffies.h>
#include "osl_sem.h"
#include "osl_spinlock.h"
#include "osl_bio.h"
#include "mdrv_print.h"
#include <bsp_slice.h>
#include "bsp_dump.h"
#include "bsp_print.h"
#include "bsp_odt.h"
#include <securec.h>
#include "syslog_print.h"
#include "bsp_rfile.h"

#define OLD_MATCH ((u32)1)
#define NEW_MATCH ((u32)0)

#define THIS_MODU mod_print
u32 g_print_close = 0;
struct bsp_syslevel_ctrl g_print_sys_level = { BSP_P_ERR, BSP_P_INFO };
struct log_usr_info g_log_user_info[LOG_INFO_MAX];
struct delayed_work g_flush_modem_log_work;
bsp_log_string_pull_func g_print_to_hids_func = NULL;
struct print_device_ctrl_s g_print_device_ctrl;
struct bsp_print_tag g_print_tag[MODU_MAX] = {
    {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO},
    {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO},
    {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO},
    {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO},
    {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO},
    {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO},
    {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO},
    {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO},
    {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO},
    {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO},
    {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO},
    {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO},
    {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO},
    {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO},
    {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO},
    {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO},
    {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO},
    {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO},
    {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO},
    {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO},
    {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO},
    {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO},
    {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO},
    {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO},
    {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO},
    {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO},
    {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO},
    {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO},
    {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO},
    {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO},
    {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO},
    {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO}, {BSP_P_INFO},
};

#define HIDSLEVEL2MODULE(hids_level) ((0xf << 16) | ((hids_level) << 12))
#define LOG_MOD_DELAY_30000 30000
#define LOG_MOD_DELAY_5000 5000
#define LOG_MOD_DELAY_10000 10000

#ifdef BSP_CONFIG_PHONE_TYPE
static DEFINE_SPINLOCK(g_om_log_spinlock);
#define __LOG_BUF_LEN (1 << 17)
static char g_logbuffer[__LOG_BUF_LEN] = {0};
static u32 g_logbuf_index = 0;
#define LOG_BUF_MASK (__LOG_BUF_LEN - 1)
#define LOG_BUF(idx) (g_logbuffer[(idx) & (LOG_BUF_MASK)])
#define LOG_BUF_TIMESLICE_LEN 16

void bsp_log2buf(const char *buf, u32 len)
{
    unsigned long flags;
    u32 i = 0;
    char timeslice[LOG_BUF_TIMESLICE_LEN] = {0};
    u32 length;
    length = snprintf_s(timeslice, sizeof(timeslice), sizeof(timeslice) - 1, "[%-8x]", bsp_get_slice_value());
    if (length < 0)
        printk(KERN_ERR "snprintf_s err!\n");

    spin_lock_irqsave(&g_om_log_spinlock, flags);
    length = strlen(timeslice);

    /* 记录时间戳 */
    for (i = 0; i < length; i++) {
        LOG_BUF(g_logbuf_index) = *(timeslice + i);
        g_logbuf_index++;
    }
    /* 记录buf数据 */
    length = len;
    for (i = 0; i < length; i++) {
        LOG_BUF(g_logbuf_index) = *(buf + i);
        g_logbuf_index++;
    }
    spin_unlock_irqrestore(&g_om_log_spinlock, flags);
    return;
}

void bsp_store_log_buf(char *buf, u32 len)
{
    u32 i = 0;
    u32 tempindex = 0;
    u32 length = 0;
    unsigned long flags;
    if ((buf == NULL) || (!len))
        return;

    if (len > __LOG_BUF_LEN)
        length = __LOG_BUF_LEN;
    else
        length = len;

    spin_lock_irqsave(&g_om_log_spinlock, flags);
    if (g_logbuf_index < (__LOG_BUF_LEN - 1)) {
        tempindex = g_logbuf_index < length ? 0 : g_logbuf_index - length;
    } else {
        tempindex = (g_logbuf_index + __LOG_BUF_LEN - length) & LOG_BUF_MASK;
    }
    for (i = 0; i < length; i++) {
        *(buf + i) = LOG_BUF(tempindex);
        tempindex = (tempindex + 1) & LOG_BUF_MASK;
    }
    spin_unlock_irqrestore(&g_om_log_spinlock, flags);
    return;
}
#endif

void bsp_log_string_pull_hook(bsp_log_string_pull_func hook)
{
    if (hook != NULL) {
        g_print_to_hids_func = hook;
    }
    return;
}

u32 bsp_print_level_to_hids(u32 level)
{
    switch (level) {
        case BSP_P_FATAL:
            return BSP_HIDS_LEVEL_ERROR;
        case BSP_P_ERR:
            return BSP_HIDS_LEVEL_WARNING;
        case BSP_P_WRN:
            return BSP_HIDS_LEVEL_NOTICE;
        default:
            return BSP_HIDS_LEVEL_INFO;
    }
}
EXPORT_SYMBOL(bsp_print_level_to_hids);
#if defined(ENABLE_BUILD_PRINT)
/*
 * 功能描述: 查询模块当前设置的打印级别
 * 输入参数: sel:sel = 0 close print; sel = 1 open print
 * 输出参数: 无
 * 返 回 值: 打印级别
 */
void bsp_print_control(u32 sel)
{
    g_print_close = sel;
    return;
}
/*
 * 功能描述: 系统级别设置
 * 输入参数: u32 console：控制台打印级别；u32 logbuf：default for acore
 * 输出参数: BSP_OK/BSP_ERROR
 */
s32 logs(u32 console, u32 logbuf)
{
    if (console >= BSP_LEVEL_BUTT || logbuf >= BSP_LEVEL_BUTT)
        return BSP_ERROR;
    g_print_sys_level.logbuf_level = logbuf;
    g_print_sys_level.con_level = console;
    return BSP_OK;
}
/*
 * 功能描述: set all modules' level
 * 输入参数: u32 level: mod_level
 * 输出参数: NA
 */
void set_all_module(u32 level)
{
    int i = 0;
    if (level >= BSP_LEVEL_BUTT)
        return;
    for (i = 0; i < MODU_MAX; i++)
        g_print_tag[i].modlevel = level;

    return;
}
/*
 * 功能描述: set module's level according to modid
 * 输入参数: u32 modid: module's id, u32 level: mod_level
 * 输出参数: BSP_OK/BSP_ERROR
 * 返 回 值: 成功/失败
 */
s32 logm(u32 modid, u32 level)
{
    if (modid >= MODU_MAX) {
        bsp_err("modid is error!\n");
        return BSP_ERROR;
    }
    if (level >= BSP_LEVEL_BUTT) {
        bsp_err("level can't over 5!\n");
        return BSP_ERROR;
    }
    if (mod_all == modid) {
        set_all_module(level);
    } else
        g_print_tag[modid].modlevel = level;
    return BSP_OK;
}
/*
 * 功能描述: inquire module's level according to modid
 * 输入参数: u32 modid: module's id
 * 输出参数: NA
 */
void logc(u32 modid)
{
    if (modid >= MODU_MAX) {
        bsp_err("modid is error!\n");
        return;
    }
    bsp_err("con_level:%d logbuf_level:%d mod_level:%d\n\n", g_print_sys_level.con_level,
        g_print_sys_level.logbuf_level, g_print_tag[modid].modlevel);
    return;
}

int bsp_print_hook_register(mdrv_print_hook hook)
{
    int i;
    unsigned long flags;

    if (g_print_device_ctrl.init_done == false) {
        spin_lock_init(&g_print_device_ctrl.regist_lock);
        g_print_device_ctrl.init_done = true;
    }

    spin_lock_irqsave(&g_print_device_ctrl.regist_lock, flags);
    for (i = 0; i < MDRV_PRINT_HOOK_MAX; i++) {
        if (g_print_device_ctrl.print_hook[i].func == NULL) {
            g_print_device_ctrl.print_hook[i].func = hook;
            spin_unlock_irqrestore(&g_print_device_ctrl.regist_lock, flags);
            return i;
        }
    }
    spin_unlock_irqrestore(&g_print_device_ctrl.regist_lock, flags);
    return BSP_ERROR;
}

int bsp_print_hook_unregister(int hook_id)
{
    if (hook_id < 0 || hook_id >= MDRV_PRINT_HOOK_MAX) {
        return BSP_ERROR;
    }

    g_print_device_ctrl.print_hook[hook_id].func = NULL;

    return BSP_OK;
}

void kernel_printk(unsigned int level, const char *buffer)
{
    if (g_print_sys_level.con_level >= level) {
        (void)printk(KERN_ERR "%s", buffer);
    } else if (g_print_sys_level.logbuf_level >= level) {
        (void)printk(KERN_INFO "%s", buffer);
    } else {
        return;
    }
}

void __bsp_print(unsigned int modid, unsigned int level, const char *fmt, va_list arglist)
{
    u32 i;
    char print_buffer[BSP_PRINT_BUF_LEN] = {
        '\0',
    };

    if (vsnprintf_s(print_buffer, BSP_PRINT_BUF_LEN, (BSP_PRINT_BUF_LEN - 1), fmt, arglist) < 0) {
        return;
    }
    print_buffer[BSP_PRINT_BUF_LEN - 1] = '\0';

#ifdef BSP_CONFIG_PHONE_TYPE
    (void)bsp_log2buf(print_buffer, strlen(print_buffer) + 1);
#endif

    kernel_printk(level, (const char *)print_buffer);

    bsp_print_send_hids(level, print_buffer, arglist);

    for (i = 0; i < MDRV_PRINT_HOOK_MAX; i++) {
        if (g_print_device_ctrl.print_hook[i].func != NULL) {
            g_print_device_ctrl.print_hook[i].func(modid, level, print_buffer);
        }
    }
}
/*
 * 功能描述: print
 * 输入参数: u32 modid: module's id, BSP_LOG_LEVEL level: print level, char *fmt: string
 */
void bsp_print(module_tag_e modid, enum bsp_log_level level, char *fmt, ...)
{
    va_list arglist;

    if (modid >= MODU_MAX || level == BSP_PRINT_OFF) {
        return;
    }

    if (g_print_tag[modid].modlevel < level) { // 传入级别低于模块默认级别，返回
        return;
    }

    va_start(arglist, fmt);
    __bsp_print((unsigned int)modid, level, fmt, arglist);
    va_end(arglist);
}

void bsp_print_send_hids(u32 level, const char *print_buf, va_list arglist)
{
    u32 hids_level;
    if (g_print_to_hids_func != NULL) {
        hids_level = bsp_print_level_to_hids(level);
        g_print_to_hids_func(hids_level, print_buf, arglist);
    }
}
EXPORT_SYMBOL_GPL(bsp_print);
EXPORT_SYMBOL_GPL(logs);
EXPORT_SYMBOL_GPL(logm);
EXPORT_SYMBOL_GPL(logc);
EXPORT_SYMBOL_GPL(bsp_print_control);
EXPORT_SYMBOL_GPL(set_all_module);

static inline u32 om_log_writable_size_get(u32 write, u32 read, u32 ring_buffer_size)
{
    return (read > write) ? (read - write) : (ring_buffer_size - write + read);
}

/*
 * 功能描述: 拷贝数据至用户态
 * 输入参数: const char * src, 数据源buffer
 * char * dst，数据目的buffer
 * u32 len, 期望读取的数据长度
 * 输出参数: 无
 * 返 回 值: 实际读取的数据长度
 */
static int do_read_data_to_user(const char *src, char *dst, u32 len)
{
    u32 ret;
    if (dst == NULL || len == 0) {
        return 0;
    }
    /* 返回未能拷贝的数据长度 */
    ret = (u32)copy_to_user(dst, src, len);

    return (int)(len - ret);
}

int om_log_read(struct log_usr_info *usr_info, char *buf, u32 count)
{
    s32 ret = 0;
    u32 msg_len = 0;
    u32 write_p, read_p;
    if (usr_info == NULL) {
        return ret;
    }
    write_p = usr_info->mem->write;
    read_p = usr_info->mem->read;

    if (om_log_writable_size_get(write_p, read_p, usr_info->mem->size) <= LOG_BUFFER_FULL_THRESHOLD) {
        usr_info->mem->read = usr_info->mem->write;
        msg_len = (u32)strlen(LOG_DROPPED_MESSAGE);
        count = min(msg_len, count);
        ret = do_read_data_to_user(LOG_DROPPED_MESSAGE, buf, count);
        bsp_info("%s entry\n", __func__);
    }
    return ret;
}

void om_log_dump_handle(void)
{
    if (g_log_user_info[CCORE_LOG_INFO].mem != NULL && g_log_user_info[CCORE_LOG_INFO].mem->app_is_active) {
        (void)cancel_delayed_work_sync(&g_flush_modem_log_work);
    }
}
EXPORT_SYMBOL(om_log_dump_handle);
void modem_log_save_handle(struct work_struct *work)
{
    static int retry_cnt = 10; /* 10 表示等待10次，即300s，如果连续300s还没准备好，则认为服务不可用,不再加入workqueue */
    u32 ready = 0;
    if (g_log_user_info[CCORE_LOG_INFO].mem != NULL && g_log_user_info[CCORE_LOG_INFO].mem->app_is_active) {
        bsp_modem_log_wakeup_one(&g_log_user_info[CCORE_LOG_INFO]);
        ready = 1;
    }
    if (g_log_user_info[CCORE_TEE_LOG_INFO].mem != NULL && g_log_user_info[CCORE_TEE_LOG_INFO].mem->app_is_active) {
        bsp_modem_log_wakeup_one(&g_log_user_info[CCORE_TEE_LOG_INFO]);
        ready = 1;
    }
#ifndef BSP_CONFIG_PHONE_TYPE
    if (g_log_user_info[MCORE_LOG_INFO].mem != NULL && g_log_user_info[MCORE_LOG_INFO].mem->app_is_active) {
        bsp_modem_log_wakeup_one(&g_log_user_info[MCORE_LOG_INFO]);
        ready = 1;
    }
#endif
    if (ready == 0) {
        retry_cnt--;
    } else {
        retry_cnt = 10; /* 10 表示等待10次,重载retry次数 */
    }
    if (retry_cnt <= 0) {
        return;
    }
    schedule_delayed_work(&g_flush_modem_log_work, msecs_to_jiffies(LOG_MOD_DELAY_30000));
    return;
}
EXPORT_SYMBOL(modem_log_save_handle);
#ifdef CONFIG_PM
static struct notifier_block g_print_suspend_notifier;
static int print_suspend_nb(struct notifier_block *this, unsigned long event, void *ptr)
{
    if (NULL != g_log_user_info[CCORE_LOG_INFO].mem && g_log_user_info[CCORE_LOG_INFO].mem->app_is_active) {
        switch (event) {
            case PM_POST_HIBERNATION:
            case PM_POST_SUSPEND:
                schedule_delayed_work(&g_flush_modem_log_work, msecs_to_jiffies(LOG_MOD_DELAY_5000));
                break;

            case PM_HIBERNATION_PREPARE:
            case PM_SUSPEND_PREPARE:
                (void)cancel_delayed_work_sync(&g_flush_modem_log_work);
                break;
            default:
                break;
        }
    }
    return NOTIFY_OK;
}
#endif

int modem_log_info_init_one(int i)
{
    int ret;
    struct log_usr_info *log_info = &g_log_user_info[i];
    syslog_logmem_module_e logmem_module;
    syslog_logmem_area_info_s area_info;
    if (i == (int)CCORE_LOG_INFO) {
        logmem_module = LOGMEM_CCPU;
        log_info->dev_name = CCORE_LOG_DEV_NAME;
    } else if (i == (int)CCORE_TEE_LOG_INFO) {
        logmem_module = LOGMEM_CCPU_TEE;
        log_info->dev_name = CCORE_TEE_LOG_DEV_NAME;
    } else {
        logmem_module = LOGMEM_MCU;
        log_info->dev_name = MCORE_LOG_DEV_NAME;
    }
    ret = bsp_syslog_get_logmem_info(logmem_module, &area_info);
    if (ret < 0 || area_info.virt_addr == NULL || area_info.length == 0) {
        return BSP_ERROR;
    }
    log_info->mem = (struct log_mem *)area_info.virt_addr;
    log_info->ring_buf = (char *)log_info->mem + sizeof(struct log_mem_stru);
    log_info->mem_is_ok = 1;
    log_info->read_func = (USR_READ_FUNC)om_log_read;
    ret = bsp_modem_log_register(log_info);
    return ret;
}
static int modem_log_info_init(void)
{
#ifndef BSP_CONFIG_PHONE_TYPE
    /* mcore device */
    if (modem_log_info_init_one((int)MCORE_LOG_INFO)) {
        bsp_err("log_mcore register fail\n");
        return BSP_ERROR;
    }
#endif
    /* ccore device */
    if (modem_log_info_init_one((int)CCORE_LOG_INFO)) {
        bsp_err("log_ccore register fail\n");
        return BSP_ERROR;
    }
    /* ccore device */
    if (modem_log_info_init_one((int)CCORE_TEE_LOG_INFO)) {
        bsp_err("no log_ccore_tee dev\n");
        return BSP_ERROR;
    }
    return BSP_OK;
}

int modem_om_log_init(void)
{
    s32 ret;
    if (modem_log_info_init() != BSP_OK) {
        return BSP_ERROR;
    }

    INIT_DELAYED_WORK(&g_flush_modem_log_work, modem_log_save_handle);

    ret = bsp_dump_register_hook("KERNEL_LOG", om_log_dump_handle);
    if (ret == BSP_ERROR) {
        bsp_err("register to dump failed!\n");
        return BSP_ERROR;
    }

#ifdef CONFIG_PM
    /* Register to get PM events */
    g_print_suspend_notifier.notifier_call = print_suspend_nb;
    g_print_suspend_notifier.priority = -1;
    if (register_pm_notifier(&g_print_suspend_notifier)) {
        printk("Failed to register for PM events\n");
        bsp_err("[init] err\n");
        return -1;
    }
    schedule_delayed_work(&g_flush_modem_log_work, msecs_to_jiffies(LOG_MOD_DELAY_10000));
#endif
    bsp_err("[init] ok\n");
    return 0;
}
#if !IS_ENABLED(CONFIG_MODEM_DRIVER)
module_init(modem_om_log_init);
#endif
#endif
u32 bsp_print_get_len_limit(void)
{
    return BSP_PRINT_LEN_LIMIT;
}
EXPORT_SYMBOL_GPL(bsp_print_get_len_limit);
struct log_usr_info* bsp_print_get_log_dev(unsigned int i)
{
    if (i >= (unsigned int)LOG_INFO_MAX) {
        return NULL;
    }
    return &g_log_user_info[i];
}
EXPORT_SYMBOL_GPL(bsp_print_get_log_dev);
struct bsp_syslevel_ctrl bsp_print_get_level(void)
{
    return g_print_sys_level;
}
