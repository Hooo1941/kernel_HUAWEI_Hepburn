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
#include <bsp_dt.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/ktime.h>
#include <linux/timekeeping.h>
#include <osl_types.h>
#include <osl_malloc.h>
#include <osl_thread.h>
#include <bsp_dump.h>
#include <bsp_slice.h>
#include <bsp_dump_mem.h>
#include <bsp_file_ops.h>
#include <bsp_sysctrl.h>
#include <dump_frame.h>
#include "dump_logs.h"
#include "dump_log_strategy.h"
#include "dump_avs_escape.h"

#ifdef BSP_CONFIG_PHONE_TYPE

#define DUMP_LOG_ROOT "/data/hisi_logs"
#define DUMP_AVS_ESCAPE_RESET_LOG_DIR "/data/hisi_logs/modem_log"
#define DUMP_AVS_ESCAPE_RESET_LOG_PATH ("/data/hisi_logs/modem_log" "/reset.log")

#define THIS_MODU mod_dump

#define DUMP_AVS_RST_INFO_MAGIC 0x5A5AA5A5

#define DUMP_AVS_RSTLOG_TASK_PRIO 96
#define DUMP_AVS_RST_RECORD_NUM 20
#define DUMP_REG_OFFSET_MAX 0xFFC

#define DUMP_WAIT_AVS_RESETLOG_CNT 60
#define DUMP_WAIT_AVS_RESETLOG_SLEEP_MS 5000
struct dump_avs_rst_info {
    u32 magic;
    u8 avs_disable_flag;
    u8 total_item;
    u8 cur_item;
    u8 rsv;
    u32 rst_time[DUMP_AVS_RST_RECORD_NUM];
};

struct dump_avs_dts_cfg {
    u32 off_time_th;
    u32 off_freq_th;
    u32 on_time_th;
    u32 on_freq_th;
    u32 pctrl_offset;
};

struct dump_avs_escape_cfg {
    struct dump_avs_dts_cfg dts_cfg;
    u32 *pctrl_base;
    struct dump_avs_rst_info *rst_info;
};

enum dump_avs_errno_e {
    DUMP_AVS_OK = 0,
    DUMP_AVS_CFG_NODE_NOT_FOUND = -1,
    DUMP_AVS_CFG_ATTR_NOT_FOUND = -2,
    DUMP_AVS_DIR_CREATE_FAIL = -3,
    DUMP_AVS_FILE_CREATE_FAIL = -4,
    DUMP_AVS_FILE_OPEN_FAIL = -5,
    DUMP_AVS_FILE_READ_FAIL = -6,
    DUMP_AVS_FILE_UPDATE_FAIL = -7,
    DUMP_AVS_FILE_PARA_CHECK_FAIL = -8,
    DUMP_AVS_MALLOC_FAIL = -9,
    DUMP_AVS_MAP_PCTRL = -10,
    DUMP_AVS_CFG_ATTR_CHECK_FAIL = -11,
    DUMP_AVS_TASK_INIT_FAIL = -12,
};

__ro_after_init struct dump_avs_escape_cfg g_dump_avs_escape_cfg;
const u32 g_dump_avs_chk_modid[] = { 0x1000, 0xc0000200 };
u32 g_dump_avs_inited;

void dump_avs_set_pctrl(u32 status)
{
    writel(status, (void *)g_dump_avs_escape_cfg.pctrl_base + g_dump_avs_escape_cfg.dts_cfg.pctrl_offset);
}

u32 dump_avs_get_state(void)
{
    u32 status;
    if (!g_dump_avs_inited) {
        return 0;
    }
    status= readl((void *)g_dump_avs_escape_cfg.pctrl_base + g_dump_avs_escape_cfg.dts_cfg.pctrl_offset);
    return !!status;
}

int dump_avs_record_flush(void)
{
    struct file *fd = NULL;
    unsigned long fs;
    int ret;
    loff_t pos = 0;

    fd = dump_open(DUMP_AVS_ESCAPE_RESET_LOG_PATH, O_CREAT | O_RDWR | O_SYNC, DUMP_LOG_FILE_AUTH);
    if (fd == NULL) {
        return DUMP_AVS_FILE_OPEN_FAIL;
    }

    fs = get_fs();
    set_fs((mm_segment_t)KERNEL_DS);

    ret = kernel_write(fd, g_dump_avs_escape_cfg.rst_info, sizeof(struct dump_avs_rst_info), &pos);
    if (ret != sizeof(struct dump_avs_rst_info)) {
        dump_error("dump avs file write fail\n");
    }


    (void)vfs_fsync(fd, 0);
    (void)filp_close(fd, NULL);
    set_fs(fs);
    return DUMP_AVS_OK;
}

void dump_avs_update_record(u32 time)
{
    u32 cur_idx = g_dump_avs_escape_cfg.rst_info->cur_item;
    g_dump_avs_escape_cfg.rst_info->rst_time[cur_idx] = time;
    g_dump_avs_escape_cfg.rst_info->cur_item = (g_dump_avs_escape_cfg.rst_info->cur_item + 1) % DUMP_AVS_RST_RECORD_NUM;
    dump_avs_record_flush();
}

void dump_avs_clear_record(void)
{
    (void)memset_s(g_dump_avs_escape_cfg.rst_info->rst_time, sizeof(g_dump_avs_escape_cfg.rst_info->rst_time), 0,
                   sizeof(g_dump_avs_escape_cfg.rst_info->rst_time));
    g_dump_avs_escape_cfg.rst_info->cur_item = 0;
    dump_avs_record_flush();
}

void dump_avs_record_once(void)
{
    time64_t txc = ktime_get_real_seconds();
    u32 cur_idx = g_dump_avs_escape_cfg.rst_info->cur_item;
    u32 last_on_cmp_idx= (cur_idx - g_dump_avs_escape_cfg.dts_cfg.on_freq_th) % DUMP_AVS_RST_RECORD_NUM;
    u32 last_off_cmp_idx = (cur_idx - g_dump_avs_escape_cfg.dts_cfg.off_freq_th) % DUMP_AVS_RST_RECORD_NUM;
    u32 last_on_cmp = g_dump_avs_escape_cfg.rst_info->rst_time[last_on_cmp_idx];
    u32 last_off_cmp = g_dump_avs_escape_cfg.rst_info->rst_time[last_off_cmp_idx];
    u32 cur_time = (u32)txc;

    if (!g_dump_avs_escape_cfg.rst_info->avs_disable_flag) {
        /* 当前已打开AVS，检查是否要关闭 */
        if (last_off_cmp && (cur_time > last_off_cmp) &&
            (cur_time - last_off_cmp <= g_dump_avs_escape_cfg.dts_cfg.off_time_th)) {
            g_dump_avs_escape_cfg.rst_info->avs_disable_flag = 1;
            dump_avs_set_pctrl(1);
            dump_avs_clear_record();
        } else {
            dump_avs_update_record(cur_time);
        }
    } else {
        /* 当前已关闭AVS，检查是否要打开 */
        if (last_on_cmp && (cur_time > last_on_cmp) &&
            (cur_time - last_on_cmp <= g_dump_avs_escape_cfg.dts_cfg.on_time_th)) {
            g_dump_avs_escape_cfg.rst_info->avs_disable_flag = 0;
            dump_avs_set_pctrl(0);
            dump_avs_clear_record();
        } else {
            dump_avs_update_record(cur_time);
        }
    }
}

void dump_save_avs_escape_log(const char *dir_name)
{
    u32 i;
    u32 found = 0;
    dump_exception_info_s *exception = NULL;
    exception = (dump_exception_info_s *)dump_get_cur_exc_info();

    UNUSED(dir_name);
    if (g_dump_avs_inited != 1) {
        return;
    }
    if ((exception == NULL) || (exception->core != DUMP_CPU_LRCCPU)) {
        dump_print("avs_chk:no excinfo or not cp_exc\n");
        return;
    }
    for (i = 0; i < ARRAY_SIZE(g_dump_avs_chk_modid); i++) {
        if (g_dump_avs_chk_modid[i] == exception->mod_id) {
            found = 1;
            break;
        }
    }
    if (found) {
        dump_avs_record_once();
    }
}

int dump_avs_file_init(struct file *fd)
{
    int ret;
    loff_t pos = 0;
    g_dump_avs_escape_cfg.rst_info->magic = DUMP_AVS_RST_INFO_MAGIC;
    g_dump_avs_escape_cfg.rst_info->total_item = DUMP_AVS_RST_RECORD_NUM;
    ret = kernel_write(fd, g_dump_avs_escape_cfg.rst_info, sizeof(struct dump_avs_rst_info), &pos);
    if (ret != sizeof(struct dump_avs_rst_info)) {
        dump_error("dump avs file init fail %d\n", ret);
        /* 写文件失败不影响重新统计异常信息，只是之前的信息丢失了，默认关avs，并重新统计异常复位信息 */
    }
    return DUMP_AVS_OK;
}

int dump_read_avs_log_from_file(struct file *fd)
{
    int ret;
    ret = kernel_read(fd, g_dump_avs_escape_cfg.rst_info, sizeof(struct dump_avs_rst_info), 0);
    if (ret != sizeof(struct dump_avs_rst_info)) {
        dump_error("dump avs file read fail\n");
        return dump_avs_file_init(fd);
    }
    if ((g_dump_avs_escape_cfg.rst_info->magic != DUMP_AVS_RST_INFO_MAGIC) || 
        (g_dump_avs_escape_cfg.rst_info->total_item != DUMP_AVS_RST_RECORD_NUM) ||
        (g_dump_avs_escape_cfg.rst_info->cur_item >= DUMP_AVS_RST_RECORD_NUM)) {
        dump_error("dump avs file check fail\n");
        return dump_avs_file_init(fd);
    }
    return DUMP_AVS_OK;
}

int dump_avs_log_init(void)
{
    int ret = DUMP_AVS_OK;
    unsigned long fs;
    loff_t pos = 0;
    struct file *fd = NULL;

    fd = dump_open(DUMP_AVS_ESCAPE_RESET_LOG_PATH, O_CREAT | O_RDWR | O_SYNC, DUMP_LOG_FILE_AUTH);
    if (fd == NULL) {
        return DUMP_AVS_FILE_OPEN_FAIL;
    }

    fs = get_fs();
    set_fs((mm_segment_t)KERNEL_DS);
    pos = vfs_llseek(fd, 0, SEEK_END);
    if (pos < sizeof(struct dump_avs_rst_info)) {
        ret = dump_avs_file_init(fd);
    } else {
        ret = dump_read_avs_log_from_file(fd);
    }

    (void)vfs_fsync(fd, 0);
    (void)filp_close(fd, NULL);
    set_fs(fs);

    return ret;
}

int dump_avs_cfg_init(void)
{
    device_node_s *dev_node = NULL;

    dev_node = bsp_dt_find_compatible_node(NULL, NULL, "driver,dump_avs_escape");
    if (unlikely(dev_node == NULL)) {
        return DUMP_AVS_CFG_NODE_NOT_FOUND;
    }
    if (bsp_dt_property_read_u32_array(dev_node, "cfg", (u32 *)&g_dump_avs_escape_cfg.dts_cfg,
                                       sizeof(g_dump_avs_escape_cfg.dts_cfg) / sizeof(u32))) {
        return DUMP_AVS_CFG_ATTR_NOT_FOUND;
    }
    if ((g_dump_avs_escape_cfg.dts_cfg.off_freq_th >= DUMP_AVS_RST_RECORD_NUM) ||
        (g_dump_avs_escape_cfg.dts_cfg.on_freq_th >= DUMP_AVS_RST_RECORD_NUM)) {
        return DUMP_AVS_CFG_ATTR_CHECK_FAIL;
    }

    if ((g_dump_avs_escape_cfg.dts_cfg.pctrl_offset > DUMP_REG_OFFSET_MAX) ||
        (g_dump_avs_escape_cfg.dts_cfg.pctrl_offset & 0x3)) { /* 0x3表示4字节对齐 */
        return DUMP_AVS_CFG_ATTR_CHECK_FAIL;
    }
    g_dump_avs_escape_cfg.pctrl_base = (u32 *)bsp_sysctrl_addr_byindex(SYSCTRL_AP_PERI);
    if (g_dump_avs_escape_cfg.pctrl_base == NULL) {
        return DUMP_AVS_MAP_PCTRL;
    }
    return DUMP_AVS_OK;
}

int dump_avs_file_init_task(void *data)
{
    int ret;
    unsigned long fs;
    struct kstat stat;
    int timeout = DUMP_WAIT_AVS_RESETLOG_CNT;
    u32 ready = 0;

    while (timeout) {
        timeout--;
        fs = get_fs();
        set_fs(KERNEL_DS);
        ret = vfs_stat(DUMP_LOG_ROOT, &stat);
        set_fs(fs);
        if (ret) {
            msleep(DUMP_WAIT_AVS_RESETLOG_SLEEP_MS);
            continue; 
        }
        ready = 1;
        break;
    }
    if (!ready) {
        dump_error("check logdir ret %d\n", ret);
        osl_free(g_dump_avs_escape_cfg.rst_info);
        return DUMP_AVS_FILE_READ_FAIL;
    }
    ret = dump_avs_log_init();
    if (ret != DUMP_AVS_OK) {
        dump_error("init dump_avs cfg ret %d\n", ret);
        osl_free(g_dump_avs_escape_cfg.rst_info);
        return ret;
    }
    g_dump_avs_inited = 1;
    dump_avs_set_pctrl(!!g_dump_avs_escape_cfg.rst_info->avs_disable_flag);
    return 0;
}

int dump_creat_avs_file_init_task(void)
{
    OSL_TASK_ID pid;

    if (osl_task_init("mdmd_rstlog", DUMP_AVS_RSTLOG_TASK_PRIO, 0x2000, (OSL_TASK_FUNC)dump_avs_file_init_task, NULL,
                      &pid)) {
        dump_error("creat save_modem_bootlog task error!\n");
        return DUMP_AVS_TASK_INIT_FAIL;
    }
    dump_print("creat resave task ok\n");
    return DUMP_AVS_OK;
}

int dump_avs_escape_init(void)
{
    int ret;
    ret = dump_avs_cfg_init();
    if (ret != DUMP_AVS_OK) {
        dump_error("init dump_avs cfg ret %d\n", ret);
        return ret;
    }
    g_dump_avs_escape_cfg.rst_info = osl_malloc(sizeof(struct dump_avs_rst_info));
    if (g_dump_avs_escape_cfg.rst_info == NULL) {
        return DUMP_AVS_MALLOC_FAIL;
    }
    (void)memset_s(g_dump_avs_escape_cfg.rst_info, sizeof(struct dump_avs_rst_info), 0, sizeof(struct dump_avs_rst_info));
    ret = dump_creat_avs_file_init_task();
    if (ret != DUMP_AVS_OK) {
        osl_free(g_dump_avs_escape_cfg.rst_info);
        return ret;
    }
    dump_print("triger rstlog init task\n");
    bsp_dump_register_log_notifier(DUMP_SAVE_RESET_LOG, dump_save_avs_escape_log);
    return DUMP_AVS_OK;
}

#else

int dump_avs_escape_init(void)
{
    return 0;
}

u32 dump_avs_get_state(void)
{
    return 0;
}

#endif
