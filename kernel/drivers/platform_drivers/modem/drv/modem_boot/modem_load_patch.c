/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
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
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/mtd/nve_ap_kernel_interface.h>
#include <securec.h>
#include "product_config.h"
#include "modem_load.h"
#include "bsp_dump.h"
#include <mdrv_errno.h>
#include <bsp_modem_patch.h>

#define PATCH_FAIL_COUNT_MAX 3

enum modem_hot_patch_type_e {
    XLOADER_PATCH = 0,
    HIBOOT_PATCH,
    MODEM_DT_PATCH,
    TRUSTED_CORE_PATCH,
    LPMCU_PATCH,
    HIFI_PATCH = 5,
    BOOT_PATCH,
    BL31_PATCH,
    LR_HOT_PATCH,
    NR_HOT_PATCH,
    LR_PHY_HOT_PATCH = 10,
    L2HAC_HOT_PATCH,
    NR_PHY_HOT_PATCH,
    PDE_PDF_HOT_PATCH,
    PDE_CSI_HOT_PATCH,
    RFDSP1_HOT_PATCH = 15,
    RFDSP2_HOT_PATCH,
    RFDSP1_ES_HOT_PATCH,
    RFDSP2_ES_HOT_PATCH,
    RESERVED1,
    MAX_HOT_PATCH = 20,
};

enum hot_patch_icc_op {
    GET_FAIL_COUNT = 0,
    PATCH_UPDATE_STATUS = 1,
};

struct modem_hot_patch_info_s {
    char patch_exist;  // 补丁是否存在
    char patch_status; // 是否已经打补丁
    char result;
    char reserved1;
};

struct modem_hot_patch_name_info_s {
    enum modem_hot_patch_type_e patch_type;
    const char *name;
};

struct mloader_hot_patch_icc_info_s {
    u32 op;
    u32 fail_count;
    u32 epatch_type;
    struct modem_hot_patch_info_s modem_hot_patch_info;
};

struct hot_patch_info_s {
    char cold_patch_status; // 补丁是否已经打上，只要任意一个补丁镜像加载、拼接成功，该成员就会被置1;
    // modem在异常复位时，如果该成员被置为1，更新成员modem_update_fail_count
    char modem_patch_update_count; // 冷补丁更新次数
    char modem_update_fail_count; // modem升级失败次数，当该成员超过3次后，再次启动时就不会加载补丁镜像
    char rev;
    struct modem_patch_info_s modem_patch_info[MAX_HOT_PATCH]; // 分别记录nv、dsp、ccore的补丁信息,剩下的预留
    u32 lr_patch_size;
};

struct hot_patch_work_info_s {
    struct workqueue_struct *hot_patch_wq;
    struct work_struct update_nve_work;
};

struct hot_patch_info_s g_patch_info;
u32 g_hot_patch_count;
struct hot_patch_work_info_s g_hot_patch_work;

#define COLD_PATCH_DUMP_SIZE (128)
char *g_hot_patch_dump_base = NULL;

struct patch_ret_value_s g_patch_ret[MAX_HOT_PATCH];

struct modem_hot_patch_name_info_s g_hot_patch_name_info[MAX_HOT_PATCH] = {
    { XLOADER_PATCH, "xloader.bin" },
    { HIBOOT_PATCH, "hiboot.img" },
    { MODEM_DT_PATCH, "modem_dt.img" },
    { TRUSTED_CORE_PATCH, "trustedcore.img" },
    { LPMCU_PATCH, "lpmcu.bin" },
    { HIFI_PATCH, "hifi.bin" },
    { BOOT_PATCH, "boot.img" },
    { BL31_PATCH, "bl31.bin" },
    { LR_HOT_PATCH, "modem_lr_ccpu.bin" },
    { NR_HOT_PATCH, "modem_nr_ccpu.bin" },
    { LR_PHY_HOT_PATCH, "lr_phy.bin" },
    { L2HAC_HOT_PATCH, "modem_l2hac.bin" },
    { NR_PHY_HOT_PATCH, "nr_phy.bin" },
    { PDE_PDF_HOT_PATCH, "pde_nr_pdf.bin" },
    { PDE_CSI_HOT_PATCH, "pde_nr_csi.bin" },
    { RFDSP1_HOT_PATCH, "rfdsp_hf.bin" },
    { RFDSP2_HOT_PATCH, "rfdsp_sub6g.bin" },
    { RFDSP1_ES_HOT_PATCH, "rfdsp_hf_es.bin" },
    { RFDSP2_ES_HOT_PATCH, "rfdsp_sub6g_es.bin"},
    { RESERVED1, "reserved" },
};

static DEFINE_MUTEX(hot_patch_proc_lock);

struct hot_patch_info_s *mloader_get_hot_patch_info_st(void)
{
    return &g_patch_info;
}

int bsp_nvem_hot_patch_read(struct hot_patch_info_s *p_hot_patch_info)
{
    int ret;

    struct opt_nve_info_user nve_cold_patch = { 0 };
    ssize_t size = sizeof(struct hot_patch_info_s);

    nve_cold_patch.nv_operation = NV_READ;
    nve_cold_patch.nv_number = NVME_COLD_PATCH_ID;
    nve_cold_patch.valid_size = size;

    ret = nve_direct_access_interface(&nve_cold_patch);
    if (ret) {
        printk(KERN_ERR "[modem_hot_patch]nvem_cold_patch_read:nvem read failed!\n");

        return ret;
    }
    ret = memcpy_s(p_hot_patch_info, size, &(nve_cold_patch.nv_data[0]), size);
    if (ret != 0) {
        printk(KERN_ERR "[modem_hot_patch]nvem_cold_patch_read:memcpy_s failed!\n");
        return ret;
    }

    return 0;
}

int bsp_nvem_hot_patch_write(struct hot_patch_info_s *p_hot_patch_info)
{
    int ret;
    struct opt_nve_info_user nve_cold_patch = { 0 };
    ssize_t size = sizeof(struct hot_patch_info_s);

    nve_cold_patch.nv_operation = NV_WRITE;
    nve_cold_patch.nv_number = NVME_COLD_PATCH_ID;
    nve_cold_patch.valid_size = size;

    printk(KERN_ERR "[modem_hot_patch]start update nve for patch!\n");
    ret = memcpy_s(&(nve_cold_patch.nv_data[0]), size, p_hot_patch_info, sizeof(struct hot_patch_info_s));
    if (ret != 0) {
        printk(KERN_ERR "[modem_hot_patch]cold_patch:memcpy_s failed!\n");
        return ret;
    }
    ret = nve_direct_access_interface(&nve_cold_patch);
    if (ret) {
        printk(KERN_ERR "[modem_hot_patch]nvem_cold_patch_write:nvem write failed!\n");
        return ret;
    }
    printk(KERN_ERR "[modem_hot_patch]end update nve for patch!\n");
    return 0;
}

void modem_hot_patch_debug_print(void)
{
    u32 i;

    printk(KERN_ERR "[modem_hot_patch]hot_patch_status: 0x%x\n", g_patch_info.cold_patch_status);
    printk(KERN_ERR "[modem_hot_patch]modem_update_fail_count: 0x%x\n", g_patch_info.modem_update_fail_count);
    for (i = 0; i < MAX_HOT_PATCH; i++) {
        printk(KERN_ERR "[modem_hot_patch]modem_patch_info[%d] patch_exist: 0x%x\n", i,
            g_patch_info.modem_patch_info[i].patch_exist);
        printk(KERN_ERR "[modem_hot_patch]modem_patch_info[%d] patch_status: 0x%x\n", i,
            g_patch_info.modem_patch_info[i].patch_status);
        printk(KERN_ERR "[modem_hot_patch]patch_ret_value[%d].load_ret_val: 0x%x\n", i, g_patch_ret[i].load_ret_val);
    }
}

bool bsp_modem_hot_patch_is_exist(void)
{
    int i;

    for (i = 0; i < MAX_HOT_PATCH; i++) {
        if ((u32)g_patch_info.modem_patch_info[i].patch_exist == 1) {
            return 1;
        }
    }
    return 0;
}

void bsp_modem_hot_patch_update_modem_fail_count(void)
{
    int ret;

    mutex_lock(&hot_patch_proc_lock);
    if (g_hot_patch_dump_base != NULL) {
        if (memcpy_s(g_hot_patch_dump_base, COLD_PATCH_DUMP_SIZE, &g_patch_info, sizeof(struct hot_patch_info_s))) {
            printk(KERN_ERR "[modem_hot_patch]update_modem_fail_count:memcpy_s failed!\n");
        }
    }

    if (!bsp_modem_hot_patch_is_exist()) {
        mutex_unlock(&hot_patch_proc_lock);
        return;
    }

    if (g_patch_info.modem_update_fail_count > PATCH_FAIL_COUNT_MAX) {
        mutex_unlock(&hot_patch_proc_lock);
        return;
    }
    if (g_patch_info.modem_update_fail_count < PATCH_FAIL_COUNT_MAX) {
        g_patch_info.modem_update_fail_count++;
    }
    ret = bsp_nvem_hot_patch_write(&g_patch_info);
    if (ret) {
        printk(KERN_ERR "[modem_hot_patch]bsp_nvem_hot_patch_write :update nevm failed!\n");
    }

    mutex_unlock(&hot_patch_proc_lock);
}

void modem_hot_patch_update_nve_task(struct work_struct *work)
{
    int ret;
    if (bsp_modem_hot_patch_is_exist()) {
        ret = bsp_nvem_hot_patch_write(&g_patch_info);
        if (ret) {
            printk(KERN_ERR "[modem_hot_patch]hot patch update cold patch nve failed in task!\n");
        }
    }
}

void bsp_modem_update_hot_patch_nve(void)
{
    int i;

    g_patch_info.cold_patch_status = 0;

    /*
     * 如果补丁镜像不存在或补丁镜像未加载，补丁的状态为NOT_LOAD_PATCH；如果补丁镜像与镜像拼接成功，补丁的状态为PUT_PATCH_SUCESS；
     * 如果所有补丁镜像的状态为NOT_LOAD_PATCH，表示未加载的补丁镜像，将cold_patch_status设置为0；
     * 如果所有补丁镜像的状态为NOT_LOAD_PATCH或PUT_PATCH_SUCESS（至少一个补丁镜像状态为PUT_PATCH_SUCESS），表示补丁镜像与原镜像拼接成功，将cold_patch_status设置为1
     */
    for (i = 0; i < MAX_HOT_PATCH; i++) {
        if ((g_patch_info.modem_patch_info[i].patch_exist) &&
            (g_patch_info.modem_patch_info[i].patch_status == PUT_PATCH_SUCESS)) {
            g_patch_info.cold_patch_status = 1;
            break;
        }
    }

    for (i = 0; i < MAX_HOT_PATCH; i++) {
        if ((g_patch_info.modem_patch_info[i].patch_status != PUT_PATCH_SUCESS) &&
            (g_patch_info.modem_patch_info[i].patch_status != NOT_LOAD_PATCH)) {
            printk(KERN_ERR
                "[modem_hot_patch]modify_modem_cold_patch_status,line = %d, patch_type = 0x%x, patch_status = %d\n",
                __LINE__, i, g_patch_info.modem_patch_info[i].patch_status);
            g_patch_info.cold_patch_status = 0;
        }
    }

    queue_work(g_hot_patch_work.hot_patch_wq, &(g_hot_patch_work.update_nve_work));
}

ssize_t modem_imag_patch_status_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    int ret;
    ssize_t size = sizeof(struct hot_patch_info_s);
    long modem_cold_patch_clear = 0;
    if ((kstrtol(buf, 10U, &modem_cold_patch_clear) < 0) && (modem_cold_patch_clear != 0)) {
        return 0;
    }

    mutex_lock(&hot_patch_proc_lock);
    (void)memset_s(&g_patch_info, size, 0, size);
    ret = bsp_nvem_hot_patch_write(&g_patch_info);
    if (ret) {
        printk(KERN_ERR "[modem_hot_patch]modem_imag_patch_status_store:write nvem failed!\n");
        mutex_unlock(&hot_patch_proc_lock);
        return 0;
    }
    mutex_unlock(&hot_patch_proc_lock);
    return count;
}

ssize_t modem_imag_patch_status_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    u32 ret;
    u32 i;
    char data_buf[20U] = {0};
    ssize_t size = 0;
    if (buf == NULL) {
        return 0;
    }

    mutex_lock(&hot_patch_proc_lock);
    if (g_patch_info.cold_patch_status) {
        size = strlen("HOTA_SUCCESS:");
        ret = (u32)strncat_s(buf, PAGE_SIZE, "HOTA_SUCCESS:", sizeof("HOTA_SUCCESS:"));
    } else {
        size = strlen("HOTA_ERR_STACK:");
        ret = (u32)strncat_s(buf, PAGE_SIZE, "HOTA_ERR_STACK:", sizeof("HOTA_ERR_STACK:"));
    }
    size += snprintf_s(data_buf, sizeof(data_buf), sizeof(data_buf) - 1, "%d,", g_patch_info.cold_patch_status) + 1;
    ret |= (u32)strncat_s(buf, PAGE_SIZE, data_buf, sizeof(data_buf));

    size +=
        snprintf_s(data_buf, sizeof(data_buf), sizeof(data_buf) - 1, "%d,", g_patch_info.modem_update_fail_count) + 1;
    ret |= (u32)strncat_s(buf, PAGE_SIZE, data_buf, sizeof(data_buf));

    for (i = 0; i < MAX_HOT_PATCH; i++) {
        size += snprintf_s(data_buf, sizeof(data_buf), sizeof(data_buf) - 1, "%d,", i) + 1;
        ret |= (u32)strncat_s(buf, PAGE_SIZE, data_buf, sizeof(data_buf));

        size += snprintf_s(data_buf, sizeof(data_buf), sizeof(data_buf) - 1, "%d,",
            g_patch_info.modem_patch_info[i].patch_exist) +
            1;
        ret |= (u32)strncat_s(buf, PAGE_SIZE, data_buf, sizeof(data_buf));

        size += snprintf_s(data_buf, sizeof(data_buf), sizeof(data_buf) - 1, "%d,",
            g_patch_info.modem_patch_info[i].patch_status) +
            1;
        ret |= (u32)strncat_s(buf, PAGE_SIZE, data_buf, sizeof(data_buf));

        size += snprintf_s(data_buf, sizeof(data_buf), sizeof(data_buf) - 1, "%x,", g_patch_ret[i].load_ret_val) + 1;
        ret |= (u32)strncat_s(buf, PAGE_SIZE, data_buf, sizeof(data_buf));
    }

    if (ret != EOK) {
        printk(KERN_ERR "[modem_hot_patch]patch_status_show:strcat_s failed\n");
    }
    mutex_unlock(&hot_patch_proc_lock);
    return size;
}

u32 modem_hot_patch_get_type(const char *file_name)
{
    u32 i;
    char image_name[FILE_NAME_MAX_LEN];

    for (i = 0; i < MAX_HOT_PATCH; i++) {
        if (strncmp(g_hot_patch_name_info[i].name, (char *)file_name, FILE_NAME_MAX_LEN) == 0) {
            return i;
        }
    }
    return i;
}

void mloader_record_hot_patch_ret_val(enum modem_hot_patch_type_e epatch_type, int value)
{
    if (epatch_type >= MAX_HOT_PATCH) {
        return;
    }

    if (g_patch_info.modem_patch_info[epatch_type].patch_status == PUT_PATCH_FAIL) {
        g_patch_ret[epatch_type].splice_ret_val = value;
        g_patch_info.modem_update_fail_count = PATCH_FAIL_COUNT_MAX;
        g_patch_info.cold_patch_status = 0;
    }
}

u32 modem_hot_patch_get_load_fail_time(void)
{
    return (u32)g_patch_info.modem_update_fail_count;
}

int modem_get_hot_patch_nve_info_before_load(void)
{
    int ret;
    int i;

    printk(KERN_ERR "[modem_hot_patch]modem_get_hot_patch_nve_info_before_load in.\n");
    if (bsp_nvem_hot_patch_read(&g_patch_info)) {
        ret = memset_s(&g_patch_info, sizeof(g_patch_info), 0, sizeof(g_patch_info));
        if (ret != EOK) {
            printk(KERN_ERR "[modem_hot_patch]failed to memset_s cold patch info, ret 0x%x\n", ret);
            return ret;
        }
    }
    g_patch_info.cold_patch_status = 0;

    for (i = 0; i < MAX_HOT_PATCH; i++) {
        g_patch_info.modem_patch_info[i].patch_status = NOT_LOAD_PATCH;
    }

    printk(KERN_ERR "[modem_hot_patch]modem_get_hot_patch_nve_info_before_load out.\n");
    return 0;
}

int mloader_update_patch_info(const char *file_name, enum modem_patch_status status)
{
    u32 epatch_type;

    epatch_type = modem_hot_patch_get_type(file_name);
    if (epatch_type >= MAX_HOT_PATCH) {
        return -1;
    }
    g_patch_info.modem_patch_info[epatch_type].patch_exist = 1;
    g_patch_info.modem_patch_info[epatch_type].patch_status = status;
    return 0;
}

int mloader_get_patch_info(enum modem_hot_patch_type_e epatch_type)
{
    if (epatch_type >= MAX_HOT_PATCH) {
        return -1;
    }

    return g_patch_info.modem_patch_info[epatch_type].patch_status;
}

int modem_send_fail_count_to_b(void)
{
    u32 fail_count;
    int ret;
    struct mloader_hot_patch_icc_info_s mloader_hot_patch_msg;

    fail_count = modem_hot_patch_get_load_fail_time();
    ret = memset_s(&mloader_hot_patch_msg, sizeof(mloader_hot_patch_msg), 0x0,
        sizeof(struct mloader_hot_patch_icc_info_s));
    if (ret) {
        printk(KERN_ERR "[modem_hot_patch]<%s> memset_s error1, ret = %d\n", __FUNCTION__, ret);
        printk(KERN_ERR "[modem_hot_patch]modem_send_fail_count_to_b memset_s fail, ret %d\n", ret);
        return ret;
    }

    mloader_hot_patch_msg.op = GET_FAIL_COUNT;
    mloader_hot_patch_msg.fail_count = fail_count;

    ret = bsp_icc_send(ICC_CPU_APLUSB, ICC_CHN_APLUSB_IFC << 16U | APLUSB_IFC_FUNC_HOT_PATCH,
        (u8 *)(&mloader_hot_patch_msg), sizeof(struct mloader_hot_patch_icc_info_s));
    if (ret != (int)sizeof(struct mloader_hot_patch_icc_info_s)) {
        printk(KERN_ERR "[modem_hot_patch]icc send err in modem_send_fail_count_to_b, ret %d\n", ret);
        return ret;
    }
    return 0;
}

int modem_hot_patch_icc_callback(u32 channel_id, u32 len, void *context)
{
    int ret;

    struct mloader_hot_patch_icc_info_s mloader_hot_patch_msg;
    u32 epatch_type;
    u32 op;
    u32 fail_count;

    ret = memset_s(&mloader_hot_patch_msg, sizeof(mloader_hot_patch_msg), 0x0,
        sizeof(struct mloader_hot_patch_icc_info_s));
    if (ret) {
        printk(KERN_ERR "[modem_hot_patch]<%s> memset_s error, ret = %d\n", __FUNCTION__, ret);
    }

    ret = bsp_icc_read(channel_id, (u8 *)&mloader_hot_patch_msg, sizeof(struct mloader_hot_patch_icc_info_s));
    if ((u32)ret != sizeof(struct mloader_hot_patch_icc_info_s)) {
        printk(KERN_ERR "[modem_hot_patch]<%s> icc read error, ret = %d\n", __FUNCTION__, ret);
        return ret;
    }

    printk(KERN_ERR "[modem_hot_patch]mloader hot patch icc callback wakeup load task\n");
    op = mloader_hot_patch_msg.op;
    if (op == PATCH_UPDATE_STATUS) {
        epatch_type = mloader_hot_patch_msg.epatch_type;
        if (epatch_type >= MAX_HOT_PATCH) {
            printk(KERN_ERR "[modem_hot_patch]mloader hot patch icc callback patch type not right.\n");
            return -1;
        }

        g_patch_info.modem_patch_info[epatch_type].patch_exist = mloader_hot_patch_msg.modem_hot_patch_info.patch_exist;
        g_patch_info.modem_patch_info[epatch_type].patch_status =
            mloader_hot_patch_msg.modem_hot_patch_info.patch_status;

        if (mloader_hot_patch_msg.modem_hot_patch_info.result != 0) {
            mloader_record_hot_patch_ret_val(epatch_type, mloader_hot_patch_msg.modem_hot_patch_info.result);
        }
        g_patch_ret[epatch_type].load_ret_val = mloader_hot_patch_msg.modem_hot_patch_info.result;
        g_hot_patch_count++;
    } else if (op == GET_FAIL_COUNT) {
        ret = modem_send_fail_count_to_b();
        if (ret != 0) {
            printk(KERN_ERR "[modem_hot_patch]fail to send fail count to b.\n");
        }
    }

    return 0;
}

void modem_hot_patch_icc_channel_register(void)
{
    int ret;
    u32 channel_id = ICC_CHN_APLUSB_IFC << 16U | APLUSB_IFC_FUNC_HOT_PATCH;

    printk(KERN_ERR "[modem_hot_patch]modem_hot_patch_icc_channel_register in!\n");
    ret = bsp_icc_event_register(ICC_CHN_APLUSB_IFC << 16U | APLUSB_IFC_FUNC_HOT_PATCH, modem_hot_patch_icc_callback,
        NULL, NULL, NULL);
    if (ret) {
        printk(KERN_ERR "[modem_hot_patch]bsp_modem_hot_patch icc_event_register fail, ret = %d\n", ret);
        return ret;
    }
    printk(KERN_ERR "[modem_hot_patch]modem_hot_patch_icc_channel_register out!\n");
}


int bsp_modem_hot_patch_init(void)
{
    int ret;
    g_hot_patch_dump_base =
        (char *)bsp_dump_register_field(DUMP_MODEMAP_COLD_PATCH, "hot_patch", COLD_PATCH_DUMP_SIZE, 0);
    /*
     * 系统异常，调用system_error时，如果识别到有补丁，就会调用这个接口
     * 调这个接口一次更新一次打补丁失败的计数；  如果失败次数达到3次，下次启动时就不打补丁了
     */
    (void)bsp_dump_register_hook("modem_cold_patch", bsp_modem_hot_patch_update_modem_fail_count);

    g_hot_patch_work.hot_patch_wq = create_singlethread_workqueue("hot_patch_wq");
    INIT_WORK(&(g_hot_patch_work.update_nve_work), modem_hot_patch_update_nve_task);

    g_hot_patch_count = 0;

    pcie_icc_valid_register(modem_hot_patch_icc_channel_register);

    printk(KERN_ERR "[modem_hot_patch]bsp_modem_hot_patch_init ok!\n");
    return 0;
}

#if !IS_ENABLED(CONFIG_MODEM_DRIVER)
module_init(bsp_modem_hot_patch_init);
#endif
