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
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/mm.h>
#include <linux/slab.h>

#include <bsp_dump.h>
#include <bsp_slice.h>
#include <bsp_rfile.h>
#include <bsp_mloader.h>
#include <mdrv_sysboot.h>
#include <bsp_modem_patch.h>
#include <bsp_rfile.h>
#include <securec.h>

#include "mloader_err.h"
#include "mloader_debug.h"
#include "mloader_load_patch.h"
#include "bsp_om_enum.h"

#ifdef BSP_CONFIG_PHONE_TYPE
#include <linux/mtd/nve_ap_kernel_interface.h>
#define CONFIG_MLOADER_COLD_PATCH_PHONE
#endif

#ifdef CONFIG_MLOADER_COLD_PATCH
static DEFINE_MUTEX(g_modem_patch_proc_lock);
static struct modem_patch g_modem_patch;

int modem_patch_get_config_info(struct modem_patch_cfg *patch_cfg)
{
#ifdef CONFIG_MLOADER_COLD_PATCH_PHONE
    int ret;
    struct opt_nve_info_user nve_cold_patch = { 0 };

    nve_cold_patch.nv_operation = NV_READ;
    nve_cold_patch.nv_number = NVME_COLD_PATCH_ID;
    nve_cold_patch.valid_size = sizeof(*patch_cfg);

    ret = nve_direct_access_interface(&nve_cold_patch);
    if (ret) {
        mloader_print_err("fail to read nve, nve id = %d, ret = %d\n", NVME_COLD_PATCH_ID, ret);
        return ret;
    }

    ret = memcpy_s(patch_cfg, sizeof(*patch_cfg), &nve_cold_patch.nv_data[0], sizeof(*patch_cfg));
    if (ret) {
        mloader_print_err("fail to memcpy, ret = %d\n", ret);
        return ret;
    }

    return ret;
#else
    return 0;
#endif
}

int modem_patch_save_config_info(struct modem_patch_cfg *patch_cfg)
{
#ifdef CONFIG_MLOADER_COLD_PATCH_PHONE
    int ret;
    struct opt_nve_info_user nve_cold_patch = { 0 };

    nve_cold_patch.nv_operation = NV_WRITE;
    nve_cold_patch.nv_number = NVME_COLD_PATCH_ID;
    nve_cold_patch.valid_size = sizeof(*patch_cfg);

    ret = memcpy_s(&(nve_cold_patch.nv_data[0]), sizeof(*patch_cfg), patch_cfg, sizeof(*patch_cfg));
    if (ret != 0) {
        mloader_print_err("fail to memcpy, ret = %d\n", ret);
        return ret;
    }

    ret = nve_direct_access_interface(&nve_cold_patch);
    if (ret) {
        mloader_print_err("fail to write nve, nve id = %d, ret = %d\n", NVME_COLD_PATCH_ID, ret);
        return ret;
    }
    mloader_print_err("succeed to update nve for modem patch\n");
#endif
    return 0;
}

void modem_patch_state_debug(void)
{
    struct modem_patch_boot_info *info = NULL;
    mloader_print_err("patch status: 0x%x\n", g_modem_patch.cfg.status);
    mloader_print_err("patch update_count: 0x%x\n", g_modem_patch.cfg.update_count);
    mloader_print_err("patch update fail count: 0x%x\n", g_modem_patch.cfg.fail_count);

    osl_sem_downtimeout(&g_modem_patch.boot_info_sem, msecs_to_jiffies(MODEM_PATCH_SEM_TIMEOUT));
    list_for_each_entry(info, &g_modem_patch.boot_info_list, list) {
        mloader_print_err("[%s] patch_exist: 0x%x\n", info->file_name, info->patch_info.patch_exist);
        mloader_print_err("[%s] patch_status: 0x%x\n", info->file_name, info->patch_info.patch_status);
        mloader_print_err("[%s] patch_type: 0x%x\n", info->file_name, info->patch_info.patch_type);
    }
    osl_sem_up(&g_modem_patch.boot_info_sem);
}

ssize_t modem_patch_status_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    int ret;
    long modem_cold_patch_clear = 0;
    ssize_t size = sizeof(struct modem_patch_cfg);

    if ((kstrtol(buf, 0, &modem_cold_patch_clear) < 0) && (modem_cold_patch_clear != 0))
        return count;

    mutex_lock(&g_modem_patch_proc_lock);

    (void)memset_s(&g_modem_patch.cfg, size, 0, size);
    ret = modem_patch_save_config_info(&g_modem_patch.cfg);
    if (ret) {
        mloader_print_err("fail to write nve, ret = %d\n", ret);
    }

    mutex_unlock(&g_modem_patch_proc_lock);

    return count;
}

ssize_t modem_patch_status_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    int ret;
    ssize_t size = 0;
    char *result = NULL;
    struct modem_patch_boot_info *info = NULL;
    if (buf == NULL) {
        return 0;
    }

    result = g_modem_patch.cfg.status ? "HOTA_SUCCESS:" : "HOTA_ERR_STACK:";
    ret = sprintf_s(buf, PAGE_SIZE - 1, "%s%d,%d,%d \n", result, g_modem_patch.cfg.status,
        g_modem_patch.cfg.fail_count, g_modem_patch.cfg.nv_rollback);
    if (ret == -1) {
        mloader_print_err("patch state show err.!\n");
        return 0;
    }
    size += ret;
    size += 1;

    osl_sem_down(&g_modem_patch.boot_info_sem);
    list_for_each_entry(info, &g_modem_patch.boot_info_list, list) {
        ret = sprintf_s(buf + size, PAGE_SIZE - size - 1, "%s:patch_exist = %d,patch_status = %d,patch_type = %d\n", info->file_name,
        info->patch_info.patch_exist, info->patch_info.patch_status, info->patch_info.patch_type);
        if (ret == -1) {
            osl_sem_up(&g_modem_patch.boot_info_sem);
            mloader_print_err("image patch state show err, size = %d.!\n", size);
            return size;
        }
        size += ret;
        size += 1;
    }
    osl_sem_up(&g_modem_patch.boot_info_sem);

    modem_patch_state_debug();
    return size;
}

int bsp_modem_cold_patch_is_exist(void)
{
    struct modem_patch_boot_info *info = NULL;

    osl_sem_downtimeout(&g_modem_patch.boot_info_sem, msecs_to_jiffies(MODEM_PATCH_SEM_TIMEOUT));
    list_for_each_entry(info, &g_modem_patch.boot_info_list, list) {
        if (info->patch_info.patch_exist) {
            osl_sem_up(&g_modem_patch.boot_info_sem);
            return 1;
        }
    }
    osl_sem_up(&g_modem_patch.boot_info_sem);
    return 0;
}

void modem_patch_save_boot_info(void)
{
    int offset = g_modem_patch.dump_offset;
    char *dump_addr = g_modem_patch.dump_addr;
    struct modem_patch_boot_info *info = NULL;

    if (dump_addr != NULL) {
        // save patch config info to dump
        if (memcpy_s(dump_addr + offset, MODEM_PATCH_DUMP_SIZE - offset, &g_modem_patch.cfg, sizeof(struct modem_patch_cfg))) {
            mloader_print_err("update modem fail:memcpy_s failed!\n");
        }
        offset += sizeof(struct modem_patch_cfg);

        // save patch boot info to dump
        osl_sem_downtimeout(&g_modem_patch.boot_info_sem, msecs_to_jiffies(MODEM_PATCH_SEM_TIMEOUT));
        list_for_each_entry(info, &g_modem_patch.boot_info_list, list) {
            mloader_print_err(" update  boot info process start \n");
            if (memcpy_s(dump_addr + offset, MODEM_PATCH_DUMP_SIZE - offset, &info->file_name[0], MLOADER_FILE_NAME_LEN)) {
                mloader_print_err("update patch file name fail :memcpy_s failed!\n");
            }
            offset += MLOADER_FILE_NAME_LEN;
            if (memcpy_s(dump_addr + offset, MODEM_PATCH_DUMP_SIZE - offset, &info->patch_info, sizeof(struct modem_patch_info))) {
                mloader_print_err("update patch info fail :memcpy_s failed!\n");
            }
            offset += sizeof(struct modem_patch_info);
        }
        osl_sem_up(&g_modem_patch.boot_info_sem);
    }
}


void bsp_modem_patch_update_fail_count(void)
{
    int ret;
    struct list_head *p = NULL;
    struct list_head *n = NULL;
    struct modem_patch_boot_info *info = NULL;

    modem_patch_state_debug();

    if (!bsp_modem_cold_patch_is_exist() || (g_modem_patch.cfg.fail_count >= MODEM_UPDATE_FAIL_COUNT_MAX)) {
        goto exit;
    }

    modem_patch_save_boot_info();

    // save patch config info to nve
    mutex_lock(&g_modem_patch_proc_lock);
    g_modem_patch.cfg.fail_count++;
    ret = modem_patch_save_config_info(&g_modem_patch.cfg);
    if (ret) {
        mloader_print_err("fail to write nve, ret = %d\n", ret);
    }
    mutex_unlock(&g_modem_patch_proc_lock);

    mloader_print_err(" update patch fail count seccess\n");
exit:
    osl_sem_downtimeout(&g_modem_patch.boot_info_sem, msecs_to_jiffies(MODEM_PATCH_SEM_TIMEOUT));
    list_for_each_safe(p, n, &g_modem_patch.boot_info_list) {
        info = list_entry(p, struct modem_patch_boot_info, list);
        list_del(&info->list);
        kfree(info);
        info = NULL;
    }
    osl_sem_up(&g_modem_patch.boot_info_sem);
}

struct modem_patch_boot_info* modem_patch_get_list_note(const char *img_name)
{
    int ret;
    int note_count = 0;
    struct modem_patch_boot_info *info = NULL;

    osl_sem_down(&g_modem_patch.boot_info_sem);
    list_for_each_entry(info, &g_modem_patch.boot_info_list, list) {
        if (strcmp(info->file_name, img_name) == 0) {
            osl_sem_up(&g_modem_patch.boot_info_sem);
            goto exit;
        }
        note_count++;
    }
    osl_sem_up(&g_modem_patch.boot_info_sem);

    if (note_count >= MODEM_PATCH_LIST_NOTE_MAX) {
        mloader_print_err("modem patch list note error, note_count = %d.\n", note_count);
        info = NULL;
        goto exit;
    }

    info = kzalloc(sizeof(struct modem_patch_boot_info), GFP_KERNEL);
    if (info == NULL) {
        mloader_print_err("modem patch malloc error\n");
        goto exit;
    }
    ret = strcpy_s(info->file_name, MLOADER_FILE_NAME_LEN, img_name);
    if (ret != EOK) {
        kfree(info);
        info = NULL;
        goto exit;
    }
    osl_sem_down(&g_modem_patch.boot_info_sem);
    list_add_tail(&info->list, &g_modem_patch.boot_info_list);
    osl_sem_up(&g_modem_patch.boot_info_sem);
exit:
    return info;
}

int modem_patch_update_config_info(void)
{
    struct modem_patch_boot_info *info = NULL;
    g_modem_patch.cfg.status = 0;

    /*
     * 如果补丁镜像不存在或补丁镜像未加载，补丁的状态为NOT_LOAD_PATCH；如果补丁镜像与镜像拼接成功，补丁的状态为SPLICE_PATCH_SUCESS；
     * 如果所有补丁镜像的状态为NOT_LOAD_PATCH，表示未加载的补丁镜像，将status设置为0；
     * 如果所有补丁镜像的状态为NOT_LOAD_PATCH或SPLICE_PATCH_SUCCESS（至少一个补丁镜像状态为SPLICE_PATCH_SUCCESS），表示补丁镜像与原镜像拼接成功，将status设置为1
     */
    osl_sem_down(&g_modem_patch.boot_info_sem);
    list_for_each_entry(info, &g_modem_patch.boot_info_list, list) {
        if ((info->patch_info.patch_status != NOT_LOAD_PATCH) && (info->patch_info.patch_status != SPLICE_PATCH_SUCCESS)) {
            mloader_print_err("modify_modem_status, img_name = %s, patch_status = %d\n", info->file_name, info->patch_info.patch_status);
            osl_sem_up(&g_modem_patch.boot_info_sem);
            return 0;
        }
    }

    list_for_each_entry(info, &g_modem_patch.boot_info_list, list) {
        if ((info->patch_info.patch_exist) && (info->patch_info.patch_status != NOT_LOAD_PATCH)) {
            g_modem_patch.cfg.status = 1;
            break;
        }
    }
    osl_sem_up(&g_modem_patch.boot_info_sem);
    return 0;
}

int mloader_get_patch_status(void)
{
    return g_modem_patch.cfg.fail_count >= MODEM_UPDATE_FAIL_COUNT_MAX ? MODEM_PATCH_LOAD_FAIL : MODEM_PATCH_LOAD_SUCCESS;
}

int mloader_set_patch_status(const char *img_name, u32 state)
{
    struct modem_patch_boot_info *info = NULL;

    if (g_modem_patch.cfg.fail_count >= MODEM_UPDATE_FAIL_COUNT_MAX) {
        return 0;
    }

    if (state >= PATCH_STATE_MAX) {
        mloader_print_err("set patch state error,img_name = %s, state = %d \n", img_name, state);
        return -1;
    }

    info = modem_patch_get_list_note(img_name);
    if (info == NULL) {
        return -1;
    }

    info->patch_info.patch_status = state;
    if (info->patch_info.patch_status > NOT_LOAD_PATCH) {
        info->patch_info.patch_exist = 1;
    }
    modem_patch_update_config_info();
    return 0;
}

int mloader_set_patch_type(const char *img_name, u32 type)
{
    struct modem_patch_boot_info *info = NULL;

    if (g_modem_patch.cfg.fail_count >= MODEM_UPDATE_FAIL_COUNT_MAX) {
        return 0;
    }

    info = modem_patch_get_list_note(img_name);
    if (info == NULL) {
        return -1;
    }

    info->patch_info.patch_type = type;
    return 0;
}

int mloader_patch_msg_process(const char *img_name, u32 type_cmd, u32 patch_data)
{
    int result = -1;
    switch (type_cmd) {
        case MLOADER_MSG_GET_PATCH_STATUS:
            result = mloader_get_patch_status();
            break;
        case MLOADER_MSG_SET_PATCH_STATUS:
            result = mloader_set_patch_status(img_name, patch_data);
            break;
        case MLOADER_MSG_SET_PATCH_TYPE:
            result = mloader_set_patch_type(img_name, patch_data);
            break;
        default:
            mloader_print_err("patch msg op err, op = 0x%x\n", type_cmd);
            break;
    }
    return result;
}

int mloader_save_patch_config_info(void)
{
    int ret;
    if (bsp_modem_cold_patch_is_exist()) {
        ret = modem_patch_save_config_info(&g_modem_patch.cfg);
        if (ret) {
            mloader_print_err("update cold patch nve failed!,ret = %d \n", ret);
            return ret;
        }
    }

    mloader_print_err("save patch cfg completed.\n");
    return 0;
}

int bsp_modem_patch_get_state(int *state)
{
    if (g_modem_patch.is_inited == 0) {
        mloader_print_err("get modem patch state fail:init error.\n");
        return -1;
    }

    *state = mloader_get_patch_status();
    return 0;
}

int bsp_modem_patch_set_boot_info(const char *file_name, struct modem_patch_info *info)
{
    struct modem_patch_boot_info *boot_info = NULL;

    if ((file_name == NULL) || (info == NULL)) {
        mloader_print_err("%s set modem patch boot info fail:para error.\n", file_name);
        return -1;
    }

    if (g_modem_patch.is_inited == 0) {
        mloader_print_err("%s set modem patch boot info fail:init error.\n", file_name);
        return -1;
    }

    boot_info = modem_patch_get_list_note(file_name);
    if (boot_info == NULL) {
        mloader_print_err("%s set modem patch boot info fail:note error.\n", file_name);
        return -1;
    }

    boot_info->patch_info = *info;

    modem_patch_update_config_info();

    return 0;
}

int mloader_get_patch_path(const char *file_name, char *patch_path)
{
    int ret;
    char img_name[MLOADER_FILE_PATH_LEN] = { 0 };
    char patch_name[MLOADER_FILE_PATH_LEN] = { 0 };
    const char *patch_dir = MLOADER_COLD_PATCH_PATH;
    char *token = NULL;
    char *next_token = NULL;

    ret = strcpy_s(img_name, MLOADER_FILE_PATH_LEN, file_name);
    if (ret) {
        mloader_print_err("[modem patch] file_name %s too long\n", file_name);
        return ret;
    }

    token = strtok_s(img_name, ".", &next_token);
    if (token == NULL) {
        mloader_print_err("img_name %s wrong\n", img_name);
        return -1;
    }

    ret = sprintf_s(patch_name, MLOADER_FILE_PATH_LEN, "%s%s", token, "_p.bin");
    if (ret < 0) {
        mloader_print_err("fail to fmt modem patch name:  %s\n", token);
        return ret;
    }

    ret = sprintf_s(patch_path, MLOADER_FILE_PATH_LEN, "%s/%s", patch_dir, patch_name);
    if (ret < 0) {
        mloader_print_err("fail to fmt modem patch path: %s, %s\n", patch_dir, patch_name);
        return ret;
    }

    return 0;
}

int mloader_load_patch(const char *file_name, u32 ddr_offset, u32 *patch_size)
{
    int ret;
    char path[MLOADER_FILE_PATH_LEN] = { 0 };
    struct file *fp = NULL;
    u32 vrl_size = bsp_mloader_get_vrl_size();

    if (bsp_fs_tell_load_mode() != BSP_FS_OK) {
        mloader_print_err("fs not ready, skip to load patch\n");
        mloader_set_patch_status(file_name, NOT_LOAD_PATCH);
        *patch_size = 0;
        return 0;
    }

    if (g_modem_patch.cfg.fail_count >= MODEM_UPDATE_FAIL_COUNT_MAX) {
        mloader_print_err("fail_count = %d,tsp not load patch img\n", g_modem_patch.cfg.fail_count);
        mloader_set_patch_status(file_name, NOT_LOAD_PATCH);
        *patch_size = 0;
        return 0;
    }

    ret = mloader_get_patch_path(file_name, path);
    if (ret != 0) {
        mloader_print_err("get %s patch path error,ret = %d. \n", file_name, ret);
        return ret;
    }

    fp = filp_open(path, O_RDONLY, 0);
    if (IS_ERR(fp)) {
        mloader_set_patch_status(file_name, NOT_LOAD_PATCH);
        *patch_size = 0;
        mloader_print_err("patch img not exist, patch_name %s.\n", file_name);
        return 0;
    } else {
        filp_close(fp, NULL);
    }

    *patch_size = bsp_mloader_get_file_size((const char *)path);
    if (*patch_size < vrl_size) {
        mloader_print_err("patch too small, patch_size =  0x%x.\n", *patch_size);
        return -1;
    }

    *patch_size -= vrl_size;
    *patch_size = PAGE_ALIGN(*patch_size);

    ret = bsp_mloader_sec_load_image ((const char *)path, MODEM_IMG_CCPU_REE_PATCH, ddr_offset - *patch_size);
    if (ret) {
        mloader_print_err("tsp load patch img failed\n");
        mloader_set_patch_status(file_name, LOAD_PATCH_FAIL);
        return ret;
    }
    mloader_set_patch_status(file_name, SPLICE_PATCH_SUCCESS);
    g_modem_patch.cfg.status = 1;

    mloader_print_err("image %s load patch img success\n", file_name);
    return 0;
}

static DEVICE_ATTR(modem_image_patch_status, 0660, modem_patch_status_show, modem_patch_status_store);

int mloader_modem_patch_init(struct platform_device *pdev)
{
    int ret;
    /*
     * 系统异常，调用system_error时，如果识别到有补丁，就会调用这个接口
     * 调这个接口一次更新一次打补丁失败的计数；  如果失败次数达到3次，下次启动时就不打补丁了
     */
    (void)bsp_dump_register_hook("modem_cold_patch", bsp_modem_patch_update_fail_count);

    g_modem_patch.dump_addr = (char *)bsp_dump_register_field(DUMP_MODEMAP_COLD_PATCH, "modem_patch", MODEM_PATCH_DUMP_SIZE, 0);
    if (g_modem_patch.dump_addr != NULL) {
        *(int*)g_modem_patch.dump_addr = MODEM_PATCH_DUMP_START;
        g_modem_patch.dump_offset = sizeof(int);
        *(int*)(g_modem_patch.dump_addr + MODEM_PATCH_DUMP_SIZE - sizeof(int)) = MODEM_PATCH_DUMP_END;
    }

    osl_sem_init(1, &g_modem_patch.boot_info_sem);
    INIT_LIST_HEAD(&g_modem_patch.boot_info_list);

    /* get patch info */
    ret = modem_patch_get_config_info(&g_modem_patch.cfg);
    if (ret != 0) {
        mloader_print_err("fail to read patch config, ret 0x%x\n", ret);
        return ret;
    }

    ret = device_create_file(&pdev->dev, &dev_attr_modem_image_patch_status);
    if (ret) {
        mloader_print_err("fail to create modem patch node, ret = %d\n", ret);
        return ret;
    }

    g_modem_patch.is_inited = 1;

    mloader_print_err("[init]modem patch init ok\n");
    return 0;
}
#else // CONFIG_MLOADER_COLD_PATCH
int mloader_modem_patch_init(struct platform_device *pdev)
{
    return 0;
}
int mloader_patch_msg_process(const char *img_name, u32 type_cmd, u32 patch_data)
{
    return 0;
}
int mloader_save_patch_config_info(void)
{
    return 0;
}
int mloader_load_patch(const char *file_name, u32 ddr_offset, u32 *patch_size)
{
    *patch_size = 0;
    return 0;
}
int bsp_modem_cold_patch_is_exist(void)
{
    return 0;
}
int bsp_modem_patch_get_state(int *state)
{
    return 0;
}
int bsp_modem_patch_set_boot_info(const char *file_name, struct modem_patch_info *info)
{
    return 0;
}
#endif // CONFIG_MLOADER_COLD_PATCH

EXPORT_SYMBOL_GPL(bsp_modem_patch_get_state);
EXPORT_SYMBOL_GPL(bsp_modem_patch_set_boot_info);
EXPORT_SYMBOL_GPL(bsp_modem_cold_patch_is_exist);
