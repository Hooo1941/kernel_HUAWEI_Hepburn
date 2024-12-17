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
#include <linux/version.h>
#include <linux/io.h>
#include <linux/gpio.h>
#include <linux/slab.h>
#include <linux/time.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/clk.h>
#include <linux/msi.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/irqdesc.h>
#include <linux/irqdomain.h>
#include <linux/irqchip/chained_irq.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/kthread.h>
#include <linux/printk.h>
#include <linux/pci.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/of_platform.h>
#include <linux/regulator/consumer.h>
#include <linux/dma-mapping.h>
#include <linux/syscalls.h>
#include <linux/version.h>
#include <securec.h>
#include <bsp_pcie.h>
#include <bsp_dump.h>
#include <bsp_print.h>
#include <bsp_modem_patch.h>
#include <bsp_blk.h>
#include "modem_boot.h"
#include "modem_load.h"
#define THIS_MODU mod_modem_load

#define SYS_OPEN_MODE 0660
struct modem_load_info g_modem_load_info;
struct pcie_msi_cmd_send_data *g_cmd_send_data = NULL;
struct pcie_msi_cmd_resp_data *g_cmd_resp_data = NULL;
void *g_boot_load_memory = NULL;
void *g_boot_xloader_dbg_memory = NULL;

struct modem_load_debug_info g_modem_load_dbg_info[MODEM_LOAD_IMG_MAX];
struct modem_load_debug_info g_modem_load_xloader_dbg_info;

void *g_dma_img_virt_addr = NULL;
unsigned long g_dma_img_phy_addr = 0;

struct modem_load_img_info g_boot_image_info[MODEM_LOAD_IMG_MAX] = {
    {  .img_type = MODEM_LOAD_IMG_NVDEFAULT,
       .file_type = MODEM_LOAD_FILE_NON_FS,
       .dir = NVM_IMAGE_PATH,
       .name = PARTITION_MODEM_NVDEFAULT_NAME,
       .hot_patch_dir = MODEM_FW_HOT_PATCH_IMAGE_PATH,
    },
    { .img_type = MODEM_LOAD_IMG_NVBACK,
      .file_type =MODEM_LOAD_FILE_NON_FS,
      .dir =  NVM_IMAGE_PATH,
      .name = PARTITION_MODEM_NVBACKUP_NAME,
      .hot_patch_dir = MODEM_FW_HOT_PATCH_IMAGE_PATH,
    },
    { .img_type = MODEM_LOAD_IMG_NVIMG,
      .file_type = MODEM_LOAD_FILE_FS,
      .dir = NVM_IMAGE_PATH,
      .name = PARTITION_MODEM_NVIMG_NAME,
      .hot_patch_dir = MODEM_FW_HOT_PATCH_IMAGE_PATH,
    },
    { .img_type = MODEM_LOAD_IMG_NVDLOAD,
      .file_type = MODEM_LOAD_FILE_NON_FS,
      .dir = NVM_IMAGE_PATH,
      .name = PARTITION_MODEM_NVDLOAD_NAME,
      .hot_patch_dir = MODEM_FW_HOT_PATCH_IMAGE_PATH,
    },
    { .img_type = MODEM_LOAD_IMG_NVCUST,
      .file_type = MODEM_LOAD_FILE_NON_FS,
      .dir = NVM_IMAGE_PATH,
      .name = PARTITION_MODEM_NVDLOAD_CUST_NAME,
      .hot_patch_dir = MODEM_FW_HOT_PATCH_IMAGE_PATH,
    },
    { .img_type = MODEM_LOAD_IMG_UCE,
      .file_type = MODEM_LOAD_FILE_FS,
      .dir = MODEM_FW_IMAGE_PATH,
      .name = "uce.bin",
      .hot_patch_dir = MODEM_FW_HOT_PATCH_IMAGE_PATH,
    },
    { .img_type = MODEM_LOAD_IMG_HIBOOT,
      .file_type = MODEM_LOAD_FILE_FS,
      .dir = MODEM_FW_IMAGE_PATH,
      .name = "hiboot.img",
      .hot_patch_dir = MODEM_FW_HOT_PATCH_IMAGE_PATH,
    },
    { .img_type = MODEM_LOAD_IMG_HIBOOT_TEST,
      .file_type = MODEM_LOAD_FILE_FS,
      .dir = MODEM_FW_IMAGE_PATH,
      .name = "hiboot_ddrtest.img",
      .hot_patch_dir = MODEM_FW_HOT_PATCH_IMAGE_PATH,
    },
    { .img_type = MODEM_LOAD_IMG_DTS,
      .file_type = MODEM_LOAD_FILE_FS,
      .dir = MODEM_FW_IMAGE_PATH,
      .name = "modem_dt.img",
      .hot_patch_dir = MODEM_FW_HOT_PATCH_IMAGE_PATH,
    },
    { .img_type = MODEM_LOAD_IMG_LPM3,
      .file_type = MODEM_LOAD_FILE_FS,
      .dir = MODEM_FW_IMAGE_PATH,
      .name = "lpmcu.bin",
      .hot_patch_dir = MODEM_FW_HOT_PATCH_IMAGE_PATH,
    },
    { .img_type = MODEM_LOAD_IMG_HIFI,
      .file_type = MODEM_LOAD_FILE_FS,
      .dir = MODEM_FW_IMAGE_PATH,
      .name = "hifi.bin",
      .hot_patch_dir = MODEM_FW_HOT_PATCH_IMAGE_PATH,
    },
    { .img_type = MODEM_LOAD_IMG_TEEOS,
      .file_type = MODEM_LOAD_FILE_FS,
      .dir = MODEM_FW_IMAGE_PATH,
      .name = "trustedcore.img",
      .hot_patch_dir = MODEM_FW_HOT_PATCH_IMAGE_PATH,
    },
    { .img_type = MODEM_LOAD_IMG_BL31,
      .file_type = MODEM_LOAD_FILE_FS,
      .dir = MODEM_FW_IMAGE_PATH,
      .name = "bl31.bin",
      .hot_patch_dir = MODEM_FW_HOT_PATCH_IMAGE_PATH,
    },
    { .img_type = MODEM_LOAD_IMG_BOOTIMG,
      .file_type = MODEM_LOAD_FILE_FS,
      .dir = MODEM_FW_IMAGE_PATH,
      .name = "boot.img",
      .hot_patch_dir = MODEM_FW_HOT_PATCH_IMAGE_PATH,
    },
};

static s32 modem_load_access(s8 *path, s32 mode)
{
    unsigned long old_fs;
    s32 ret;

    old_fs = get_fs();
    set_fs((unsigned long)KERNEL_DS);

    ret = sys_access(path, mode);

    set_fs(old_fs);

    return ret;
}

static int modem_load_get_file_name(char *file_name, const struct modem_load_img_info *image)
{
    u32 ret;
    /* 如果支持热补丁，则先查看热补丁是否存在，如果热补丁存在，加载补丁镜像 */
#ifdef CONFIG_MLOADER_HOT_PATCH
    u32 fail_time;

    fail_time = modem_hot_patch_get_load_fail_time();
    if (fail_time < 0x3) {
        file_name[0] = '\0';
        ret = (u32)strncat_s(file_name, FILE_NAME_MAX_LEN, image->hot_patch_dir, strlen(image->hot_patch_dir));
        ret |= (u32)strncat_s(file_name, FILE_NAME_MAX_LEN, image->name, strlen(image->name));
        if (ret != EOK) {
            bsp_err("strcat hot patch image %s failed, ret = 0x%x\n", image->name, ret);
            return (int)ret;
        }
        if (!modem_load_access((s8 *)file_name, 0)) {
            ret = mloader_update_patch_info(image->name, PUT_PATCH_SUCESS);
            if (ret == 0) {
                bsp_err("load hot patch img %s!\n", image->name);
                return 0;
            }
        }
    }
#endif
    file_name[0] = '\0';
    ret = (u32)strncat_s(file_name, FILE_NAME_MAX_LEN, image->dir, strlen(image->dir));
    ret |= (u32)strncat_s(file_name, FILE_NAME_MAX_LEN, image->name, strlen(image->name));
    if (ret != EOK) {
        bsp_err("strcat image %s failed, ret = 0x%x\n", image->name, ret);
        return (int)ret;
    }

    return 0;
}

int modem_load_get_base_addr(void)
{
    unsigned long ep_boot_load_addr;
    unsigned long ep_boot_load_size;

    /* get ep boot load addr */
    ep_boot_load_addr = bsp_pcie_boot_get_bar_addr(PCIE_BAR_BOOT_LOAD);
    ep_boot_load_size = bsp_pcie_boot_get_bar_size(PCIE_BAR_BOOT_LOAD);
    if (!ep_boot_load_addr || !ep_boot_load_size) {
        bsp_err("ep_boot_load_addr 0x%lx, ep_boot_load_size 0x%lx\n", ep_boot_load_addr, ep_boot_load_size);
        return -1;
    }

    g_modem_load_info.pcie_load_img_virt_addr = (void *)ioremap(ep_boot_load_addr, ep_boot_load_size);
    if (g_modem_load_info.pcie_load_img_virt_addr == NULL) {
        bsp_err("pcie_load_img_virt_addr ioremap fail!\n");
        return -1;
    }

    return 0;
}

void modem_load_put_base_addr(void)
{
    if (g_modem_load_info.pcie_load_img_virt_addr != NULL) {
        iounmap(g_modem_load_info.pcie_load_img_virt_addr);
        g_modem_load_info.pcie_load_img_virt_addr = NULL;
    }
}

int modem_load_get_file_size(const char *filename)
{
    int ret;
    struct kstat kstattmp = { 0 };
    mm_segment_t old_fs;

    old_fs = get_fs();
    set_fs((mm_segment_t)KERNEL_DS);

    ret = vfs_stat(filename, &kstattmp);
    if (ret) {
        bsp_err("get file %s size failed, ret = 0x%x ", filename, ret);
        goto err;
    }

    set_fs(old_fs);
    return kstattmp.size;

err:
    set_fs(old_fs);
    return ret;
}

#ifdef CONFIG_MLOADER_HOT_PATCH
static int modem_load_get_xloader_patch_path(char *xloader_path)
{
    int ret = 0;
    u32 fail_time = modem_hot_patch_get_load_fail_time();
    if (fail_time < 0x3) {
        if (!modem_load_access((s8 *)MODEM_FW_XLOADER_HOT_PATCH_PATH, 0)) {
            ret = strncat_s(xloader_path, FILE_NAME_MAX_LEN, MODEM_FW_XLOADER_HOT_PATCH_PATH,
                strlen(MODEM_FW_XLOADER_HOT_PATCH_PATH));
            if (ret != EOK) {
                bsp_err("strcat hot patch path failed\n");
                return ret;
            }
            ret = mloader_update_patch_info("xloader.bin", PUT_PATCH_SUCESS);
            if (ret == 0) {
                bsp_err("load hot patch img xloader.bin!\n");
            }
        }
        xloader_path[0] = '\0';
    }

    return ret;
}
#endif

static int modem_load_get_xloader_path(char *xloader_path)
{
    int ddr_work_mode = ddr_test_get_boot_flag();
#ifdef CONFIG_MLOADER_HOT_PATCH
    int ret = -1;
    u32 fail_time;
#endif
    bsp_err("DDR Test Flag = %d", ddr_work_mode);
    xloader_path[0] = '\0';
    switch (ddr_work_mode) {
        case 0x0: /* load xloader.bin */
#ifdef CONFIG_MLOADER_HOT_PATCH
            ret = modem_get_hot_patch_nve_info_before_load();
            if (ret != 0) {
                bsp_err("fail to get hot patch info from nve.\n");
            }
            ret = modem_load_get_xloader_patch_path(xloader_path);
            break;
#endif
            ret = strncat_s(xloader_path, FILE_NAME_MAX_LEN, MODEM_FW_XLOADER_PATH, strlen(MODEM_FW_XLOADER_PATH));
            break;
        case 0x1: /* load xloader_test.bin */
            ret = strncat_s(xloader_path, FILE_NAME_MAX_LEN, MODEM_FW_XLOADER_DDRTEST, strlen(MODEM_FW_XLOADER_DDRTEST));
            break;
        case 0x2: /* load xloader_ddr_mntn.bin */
            ret = strncat_s(xloader_path, FILE_NAME_MAX_LEN, MODEM_FW_XLOADER_MNTN_PATH,
                strlen(MODEM_FW_XLOADER_MNTN_PATH));
            break;
        default:
            bsp_err("get DDR Test Flag = %d invalid ", ddr_work_mode);
            break;
    }

    return ret;
}

static int modem_load_read_xloader_data(struct file *file_handle, int xloader_size, u32 *file_length)
{
    loff_t offset_adp;
    void *mem_addr = g_boot_load_memory;
    void *bar_addr = g_modem_load_info.pcie_load_img_virt_addr;
    int ret_len, ret;


    do {
        offset_adp = (loff_t)file_length;
        ret_len = kernel_read(file_handle, (char *)mem_addr, 1024U, &offset_adp);
        if (ret_len < 0) {
            bsp_err("fail to read xloader\n");
            ret = ret_len;
            return ret;
        }
        memcpy_toio(bar_addr, mem_addr, ret_len);
        bar_addr += ret_len;
        mem_addr += ret_len;
        *file_length += (u32)ret_len;
    } while ((ret_len > 0) && (*file_length < (u32)xloader_size));

    return 0;
}

int modem_load_send_xloader(void)
{
    int ret, xloader_size;
    u32 file_length = 0;
    u32 read_feedback;
    struct file *file_handle = NULL;
    char file_name[FILE_NAME_MAX_LEN] = {0};
    char *file_name_ptr = &file_name[0];

    ret = modem_load_get_xloader_path(file_name_ptr);
    if (ret) {
        bsp_err("get xloader path failed, ret = 0x%x\n", ret);
        return ret;
    }
    file_handle = filp_open(file_name, O_RDONLY, 0);
    if (IS_ERR_OR_NULL(file_handle)) {
        bsp_err("fail to open file '%s', ret = %ld\n", file_name, PTR_ERR(file_handle));
        return -1;
    }
    g_modem_load_xloader_dbg_info.load_flag = 1;

    xloader_size = modem_load_get_file_size(file_name);
    if (xloader_size > MODEM_BOOT_LOAD_SIZE) {
        bsp_err("xloader size too large, size = %d\n", xloader_size);
        ret = -1;
        goto err_read_file;
    }
    g_modem_load_xloader_dbg_info.receive_len = xloader_size;

    ret = modem_load_read_xloader_data(file_handle, xloader_size, &file_length);
    if (ret) {
        goto err_read_file;
    }

    read_feedback = readl(g_modem_load_info.pcie_load_img_virt_addr + file_length - 0x4);
    g_modem_load_xloader_dbg_info.send_len = file_length;
    g_modem_load_xloader_dbg_info.finish_status = PCIE_CMD_RESP_BOOT;

err_read_file:
    filp_close(file_handle, NULL);

    return ret;
}

irqreturn_t modem_load_msi_int_isr(int irq, void *dev_info)
{
    struct pcie_trans_msg_info resp_msg = {
        .msi_id = 0,
        .cmd_len = 0,
        .cmd_data_addr = (void *)g_cmd_resp_data,
    };

    if (bsp_pcie_rc_read_msg(&resp_msg)) {
        bsp_err("read pcie msg error, check msg len and cmd data addr!\n");
        return IRQ_HANDLED;
    }

    bsp_pcie_rc_read_clear();

    if (g_cmd_resp_data->cmd_type >= MODEM_LOAD_IMG_MAX) {
        bsp_err("cmd_type(%d) error!\n", g_cmd_resp_data->cmd_type);
        return IRQ_HANDLED;
    }

    g_modem_load_dbg_info[g_cmd_resp_data->cmd_type].intr_cnt++;

    up(&g_modem_load_info.msi_cmd_semaphore);
    return IRQ_HANDLED;
}

int modem_load_read_fs(struct pcie_msi_cmd_resp_data *pdata, int *ret_len)
{
    int ret;
    char file_name[FILE_NAME_MAX_LEN] = {0};
    char *file_name_ptr = &file_name[0];
    struct modem_load_img_info *st_img = NULL;
    struct file *file_handle = NULL;
    loff_t offset_adp;

    st_img = &g_boot_image_info[pdata->cmd_type];
    ret = modem_load_get_file_name(file_name_ptr, st_img);
    if (ret) {
        bsp_err("fail to get file name, ret = %d\n", ret);
        return ret;
    }

    bsp_err("loading %s\n", st_img->name);

    file_handle = filp_open(file_name, O_RDONLY, 0);
    if (IS_ERR_OR_NULL(file_handle)) {
        bsp_err("fail to open '%s', ret = %ld\n", file_name, PTR_ERR(file_handle));
        ret = (int)PTR_ERR(file_handle);
        return ret;
    }
    offset_adp = pdata->offset;
    *ret_len = kernel_read(file_handle, (char *)g_modem_load_info.pcie_dma_virt_addr, pdata->des_len, &offset_adp);

    if ((*ret_len < 0) || (*ret_len > PCIE_IMAGE_DMA_SIZE)) {
        bsp_err("fail to read '%s', ret_len = 0x%x\n", file_name, *ret_len);
        ret = -1;
        goto error_read_file;
    }

error_read_file:
    filp_close(file_handle, NULL);
    return ret;
}

int modem_load_read_non_fs(struct pcie_msi_cmd_resp_data *pdata, int *ret_len)
{
    int ret;

    ret = bsp_blk_read(g_boot_image_info[pdata->cmd_type].name, pdata->offset, g_modem_load_info.pcie_dma_virt_addr,
        pdata->des_len);
    if (ret) {
        bsp_err("read %s failed, ret = %d!\n", g_boot_image_info[pdata->cmd_type].name, ret);
    }

    *ret_len = pdata->des_len;
    return ret;
}

int modem_load_read_file(struct pcie_msi_cmd_resp_data *pdata, int *ret_len)
{
    int ret = 0;

    if (g_boot_image_info[pdata->cmd_type].file_type == MODEM_LOAD_FILE_FS) {
        ret = modem_load_read_fs(pdata, ret_len);
        if (ret) {
            bsp_err("read fs file fail, ret %d.\n", ret);
            return ret;
        }
    } else if (g_boot_image_info[pdata->cmd_type].file_type == MODEM_LOAD_FILE_NON_FS) {
        ret = modem_load_read_non_fs(pdata, ret_len);
        if (ret) {
            bsp_err("read fs file fail, ret %d.\n", ret);
            return ret;
        }
    } else {
        bsp_err("read file type[%d] error.\n", g_boot_image_info[pdata->cmd_type].file_type);
        return -1;
    }

    return ret;
}

int modem_load_img_type_check(enum modem_load_img_type img_type)
{
    if (img_type >= MODEM_LOAD_IMG_MAX) {
        bsp_err("msg image type read error, imagetype %d. \n", img_type);
        return -1;
    }

    if (img_type == MODEM_LOAD_IMG_HIBOOT) {
        modem_load_save_xloader();
    }

    if (img_type == MODEM_LOAD_FINISH) {
        g_modem_load_dbg_info[img_type].load_flag = 1;
        return MODEM_BOOT_LOAD_FINISH;
    } else {
        return 0;
    }
}

void bsp_modem_load_shutdown_handle(void)
{
    bsp_err("modem load shutdown start \n");

    bsp_modem_load_thread_hold_enable();

    msleep(100U); // wait for last ep dma read complete
    bsp_err("modem load shutdown complete \n");
}

static int modem_load_get_thread_flag(unsigned int nr)
{
    return g_modem_load_info.thread_flags & BIT(nr);
}

static int modem_load_send_msi_msg(struct pcie_msi_cmd_resp_data *pdata, struct pcie_trans_msg_info *send_msg,
    int transfer_size)
{
    int ret;

    g_cmd_send_data->cmd_type = pdata->cmd_type;
    g_cmd_send_data->ap_len = transfer_size;
    g_cmd_send_data->cmd_status = PCIE_CMD_RESP_BOOT;
    send_msg->msi_id = PCIE_MSI_ID_SYSBOOT;
    send_msg->cmd_len = sizeof(struct pcie_msi_cmd_send_data);
    send_msg->cmd_data_addr = (void *)g_cmd_send_data;

    ret = bsp_pcie_rc_send_msg(send_msg);
    if (ret) {
        bsp_err("img <%s> msg write fail, ret = %d\n", g_boot_image_info[pdata->cmd_type].name, ret);
    }

    return ret;
}

static int modem_load_recv_msi_msg(struct pcie_msi_cmd_resp_data *pdata)
{
    int ret = 0;

    pdata->cmd_type = g_cmd_resp_data->cmd_type;
    pdata->offset = g_cmd_resp_data->offset;
    pdata->des_len = g_cmd_resp_data->des_len;
    g_modem_load_dbg_info[pdata->cmd_type].load_start++;

    if (pdata->des_len > PCIE_IMAGE_DMA_SIZE) {
        bsp_err("msg transfer_size error %d \n", pdata->des_len);
        ret = -1;
    }

    return ret;
}

static void modem_load_record_debug_info(struct pcie_msi_cmd_resp_data *pdata, int transfer_size)
{
    g_modem_load_dbg_info[pdata->cmd_type].load_flag++;
    g_modem_load_dbg_info[pdata->cmd_type].receive_offset = pdata->offset;
    g_modem_load_dbg_info[pdata->cmd_type].receive_len = pdata->des_len;
    g_modem_load_dbg_info[pdata->cmd_type].send_len = transfer_size;
    g_modem_load_dbg_info[pdata->cmd_type].finish_status = PCIE_CMD_RESP_BOOT;
}

static int modem_load_thread(void *data)
{
    int ret = 0;
    int transfer_size = 0;
    struct pcie_msi_cmd_resp_data pdata = { 0 };
    struct pcie_trans_msg_info send_msg = { 0 };

    do {
        if (down_interruptible(&g_modem_load_info.msi_cmd_semaphore) != 0) {
            bsp_err("down_interruptible return error\n");
            continue;
        }

        if (modem_load_get_thread_flag(MODEM_LOAD_THREAD_FLAG_BIT_SIGNAL_HALT)) {
            set_bit(MODEM_LOAD_THREAD_FLAG_BIT_STATE_HALT, &g_modem_load_info.thread_flags);
            bsp_err("modem load thread receive halt signal\n");
            continue;
        }

        /* read msi bar msg from global variable, then clear */
        if (modem_load_recv_msi_msg(&pdata)) {
            continue;
        }

        ret = modem_load_img_type_check(pdata.cmd_type);
        if (!ret) {
            ret = memset_s(g_cmd_resp_data, sizeof(*g_cmd_resp_data), 0, sizeof(struct pcie_msi_cmd_resp_data));
            if (ret) {
                bsp_err("fail to clear memory, ret = %d\n", ret);
                continue;
            }
            /* read image file to malloc buffer */
            ret = modem_load_read_file(&pdata, &transfer_size);
            if (ret) {
                bsp_err("fail to read file, ret = %d\n", ret);
                continue;
            }

            /* read finish, send msi bar msg to modem */
            if (modem_load_send_msi_msg(&pdata, &send_msg, transfer_size)) {
                continue;
            }
        } else if (ret == MODEM_BOOT_LOAD_FINISH) {
            bsp_modem_boot_trigger_event(MODEM_BOOT_STATE_LOAD_DONE);
            continue;
        } else {
            continue;
        }

        modem_load_record_debug_info(&pdata, transfer_size);
    } while (1);

    return ret;
}

void modem_load_debug_print(void)
{
    u32 i = 0;

    bsp_err("*************************** MODEM BOOT LOAD DEBUG INFO BEGIN ************************************** \n");
    bsp_err("                          intr_cnt; load_start; load_flag; receive_offset; receive_len; send_len; "
        "finish_flag \n");
    bsp_err("<xx> A->B xloader.bin     : %08d;  %08d; %08d; 0x%08x; 0x%08x; 0x%08x; %08d; \r\n",
        g_modem_load_xloader_dbg_info.intr_cnt, g_modem_load_xloader_dbg_info.load_start,
        g_modem_load_xloader_dbg_info.load_flag, g_modem_load_xloader_dbg_info.receive_offset,
        g_modem_load_xloader_dbg_info.receive_len, g_modem_load_xloader_dbg_info.send_len,
        g_modem_load_xloader_dbg_info.finish_status);

    for (i = 0; i < MODEM_LOAD_IMG_MAX; i++) {
        bsp_err("<%02d> %-20s : %08d;  %08d;  %08d; 0x%08x; 0x%08x; 0x%08x; %08d; \r\n", i, g_boot_image_info[i].name,
            g_modem_load_dbg_info[i].intr_cnt, g_modem_load_dbg_info[i].load_start, g_modem_load_dbg_info[i].load_flag,
            g_modem_load_dbg_info[i].receive_offset, g_modem_load_dbg_info[i].receive_len,
            g_modem_load_dbg_info[i].send_len, g_modem_load_dbg_info[i].finish_status);
    }
    bsp_err("**************************** MODEM BOOT LOAD DEBUG INFO END ************************************* \n");
    bsp_pcie_msg_space_debug();
}

void modem_load_debug_clear(void)
{
    int ret;
    ret = memset_s(&g_modem_load_xloader_dbg_info, sizeof(struct modem_load_debug_info), 0,
        sizeof(struct modem_load_debug_info));
    if (ret) {
        bsp_err("fail to clear xloader debug info, ret = %d\n", ret);
    }

    ret = memset_s(g_modem_load_dbg_info, sizeof(struct modem_load_debug_info) * MODEM_LOAD_IMG_MAX, 0,
        sizeof(struct modem_load_debug_info) * MODEM_LOAD_IMG_MAX);
    if (ret) {
        bsp_err("fail to clear modem load debug info, ret = %d\n", ret);
    }
}

int modem_xloader_dbg_mem_init(void)
{
    int ret = 0;

#ifdef CONFIG_XLOADER_DBG
    if (g_boot_xloader_dbg_memory == NULL) {
        g_boot_xloader_dbg_memory = (void *)kzalloc(MODEM_BOOT_LOAD_SIZE, GFP_KERNEL);
        if (g_boot_xloader_dbg_memory == NULL) {
            bsp_err("fail to alloc xloader debug memory\n");
        }
    } else {
        ret = memset_s(g_boot_xloader_dbg_memory, MODEM_BOOT_LOAD_SIZE, 0, MODEM_BOOT_LOAD_SIZE);
        if (ret) {
            bsp_err("fail to clear xloader debug memory, ret = %d\n", ret);
        }
    }
#endif

    return ret;
}

int modem_load_save_file(char *filename, const void *data, u32 length, u32 max_size)
{
    int ret = 0;
#ifdef CONFIG_XLOADER_DBG
    int ret_len;
    int fd;
    unsigned long old_fs;

    old_fs = get_fs();
    set_fs((mm_segment_t)KERNEL_DS);

    fd = sys_open(filename, O_RDWR | O_CREAT, SYS_OPEN_MODE);
    if (fd < 0) {
        bsp_err("fail to create & open file:%s, fd=%d\n", filename, fd);
        set_fs(old_fs);
        return -1;
    }

    ret_len = sys_write(fd, data, length);
    if (ret_len != length) {
        bsp_err("fail to write file:%s, ret=%d\n", filename, ret_len);
        ret = -1;
        (void)sys_close(fd);
        set_fs(old_fs);
        return -1;
    }

    (void)sys_close(fd);
    set_fs(old_fs);
#endif
    return ret;
}

/* xloader boot info save to TCM, AP read fromio to memory, and save to file */
void modem_load_save_xloader(void)
{
#ifdef CONFIG_XLOADER_DBG
    int ret = 0;

    bsp_err("MODEM XLOADER BOOT INFO GET BEGIN\n");
    if (g_boot_xloader_dbg_memory == NULL) {
        bsp_err("[MODEM_XLOADER_DBG] xloader start success, memory free.\n");
        ret = 0x10;
        goto over;
    }

    memcpy_fromio(g_boot_xloader_dbg_memory, g_modem_load_info.pcie_load_img_virt_addr, MODEM_BOOT_LOAD_SIZE);
    bsp_err("[MODEM_XLOADER_DBG] copy B.TCM to A.memory success.\n");

    ret = modem_load_save_file("/data/dfx_logs/xloader_dbg.bin", g_boot_xloader_dbg_memory, MODEM_BOOT_LOAD_SIZE,
        MODEM_BOOT_LOAD_SIZE);
    if (ret != 0) {
        bsp_err("[MODEM_XLOADER_DBG] save A.memory to file failed, ret = %d.\n", ret);
        goto over;
    }
    bsp_err("[MODEM_XLOADER_DBG] save A.memory to file /data/dfx_logs/xloader_dbg.bin\n");

over:
    bsp_err("MODEM XLOADER BOOT INFO GET END\n");
    return;
#endif
}

static int modem_load_resource_init(void)
{
    struct platform_device *pdev = NULL;
    /* PCIE启动BAR1发送xloader的空间 */
    g_boot_load_memory = (void *)kzalloc(MODEM_BOOT_LOAD_SIZE, GFP_KERNEL);
    if (g_boot_load_memory == NULL) {
        bsp_err("fail to kzalloc load memory sapce.\n");
        return -1;
    }

    g_cmd_resp_data = (struct pcie_msi_cmd_resp_data *)kzalloc(sizeof(struct pcie_msi_cmd_resp_data), GFP_KERNEL);
    if (g_cmd_resp_data == NULL) {
        bsp_err("fail to kzalloc resp data sapce.\n");
        goto err_free_load_memory;
    }

    g_cmd_send_data = (struct pcie_msi_cmd_send_data *)kzalloc(sizeof(struct pcie_msi_cmd_send_data), GFP_KERNEL);
    if (g_cmd_send_data == NULL) {
        bsp_err("fail to kzalloc send data sapce.\n");
        goto err_free_resp_data;
    }

    pdev = modem_boot_get_device();
    if (pdev == NULL) {
        bsp_err("fail to get_paltform_device.\n");
        goto err_free_resp_data;
    }

    g_dma_img_virt_addr =
        dma_zalloc_coherent(&pdev->dev, PCIE_IMAGE_DMA_SIZE, (dma_addr_t *)&g_dma_img_phy_addr, GFP_KERNEL);
    if (g_dma_img_virt_addr == NULL) {
        bsp_err("malloc fail\n");
        goto err_free_send_data;
    }

    g_modem_load_info.pcie_dma_virt_addr = g_dma_img_virt_addr;
    g_modem_load_info.pcie_dma_phy_addr = (void *)(uintptr_t)g_dma_img_phy_addr;
    g_cmd_send_data->ap_addr = g_dma_img_phy_addr;

    return 0;

err_free_send_data:
    kfree(g_cmd_send_data);
    g_cmd_send_data = NULL;
err_free_resp_data:
    kfree(g_cmd_resp_data);
    g_cmd_resp_data = NULL;
err_free_load_memory:
    kfree(g_boot_load_memory);
    g_boot_load_memory = NULL;

    return -1;
}

int bsp_modem_load_init(void)
{
    struct task_struct *modem_load_task = NULL;

    sema_init(&g_modem_load_info.msi_cmd_semaphore, 0);
    wakeup_source_init(&g_modem_load_info.load_wakelock, "modem_load_wake_lock");

    modem_load_task = kthread_run(modem_load_thread, NULL, "modem_load_thread");
    if (IS_ERR_OR_NULL(modem_load_task)) {
        bsp_err("fail to create sysfs file, ret = %ld\n", PTR_ERR(modem_load_task));
        return -1;
    }

    return modem_load_resource_init();
}

static int modem_load_prepare_init()
{
    int ret;
    ret = bsp_pcie_boot_enumerate();
    if (ret) {
        bsp_err("fail to enum pcie device, ret = %d\n", ret);
        return ret;
    }

    ret = bsp_pcie_resource_init();
    if (ret) {
        bsp_err("fail to init pcie resource, ret = %d\n", ret);
        return ret;
    }

    return ret;
}

int bsp_modem_load_start(void)
{
    int ret, error_ret;

    __pm_stay_awake(&g_modem_load_info.load_wakelock);

    ret = bsp_pcie_boot_init();
    if (ret) {
        bsp_err("fail to get pcie device dts info, ret = %d\n", ret);
        return ret;
    }

    ret = modem_load_prepare_init();
    if (ret) {
        goto error_enumerate;
    }

    ret = modem_load_get_base_addr();
    if (ret) {
        bsp_err("fail to get bar addr, ret = %d\n", ret);
        goto error_get_base_addr;
    }

    ret = modem_load_send_xloader();
    if (ret) {
        bsp_err("fail to send xloader, ret = %d\n", ret);
        goto error_send_xloader;
    }

    ret = modem_xloader_dbg_mem_init();
    if (ret) {
        bsp_err("xloader dbg memory init fail, ret = %d\n", ret);
    }

    ret = bsp_pcie_receive_msi_request(PCIE_MSI_ID_SYSBOOT, modem_load_msi_int_isr, "boot_msg_msi", NULL);
    if (ret) {
        bsp_err("fail to init msi interrupt, ret = %d\n", ret);
        goto error_send_xloader;
    }

    bsp_pcie_boot_notify();

    return 0;

error_send_xloader:
    modem_load_put_base_addr();

error_get_base_addr:
    bsp_pcie_resource_exit();

error_enumerate:
    error_ret = bsp_pcie_boot_driver_uninstall();
    if (error_ret) {
        bsp_err("fail to unregister pcie boot driver, ret = %d\n", error_ret);
    }

    return ret;
}

void bsp_modem_load_stop(void)
{
    int ret;

    bsp_pcie_boot_msi_free(PCIE_MSI_ID_SYSBOOT);

    modem_load_put_base_addr();

    bsp_pcie_resource_exit();

    ret = bsp_pcie_boot_driver_uninstall();
    if (ret) {
        bsp_err("fail to unregister pcie boot driver, ret = %d\n", ret);
    }
    __pm_relax(&g_modem_load_info.load_wakelock);
}
#define MODME_LOAD_GET_THREAD_FLAG_TIMEOUT 300
void bsp_modem_load_thread_hold_enable(void)
{
    int timeout;

    clear_bit(MODEM_LOAD_THREAD_FLAG_BIT_STATE_HALT, &g_modem_load_info.thread_flags);
    set_bit(MODEM_LOAD_THREAD_FLAG_BIT_SIGNAL_HALT, &g_modem_load_info.thread_flags);
    bsp_err("[MODEM_LOAD]send modem load thread halt signal\n");
    up(&g_modem_load_info.msi_cmd_semaphore);

    timeout = MODME_LOAD_GET_THREAD_FLAG_TIMEOUT;
    while (!modem_load_get_thread_flag(MODEM_LOAD_THREAD_FLAG_BIT_STATE_HALT)) {
        msleep(10U);
        timeout--;
        if (timeout < 0) {
            bsp_err("wait for load thread halt timeout\n");
            return;
        }
    }
    bsp_err("modem load thread has halt\n");
}

void bsp_modem_load_thread_hold_disable(void)
{
    clear_bit(MODEM_LOAD_THREAD_FLAG_BIT_SIGNAL_HALT, &g_modem_load_info.thread_flags);
    bsp_err("clear modem load thread halt signal\n");
}
