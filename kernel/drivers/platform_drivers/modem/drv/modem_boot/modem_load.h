/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2016-2018. All rights reserved.
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
#ifndef __MODEM_LOAD_H__
#define __MODEM_LOAD_H__

#include <linux/types.h>
#include <linux/device.h>
#include <linux/pm_wakeup.h>
#include <linux/semaphore.h>
#include <product_config.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MODEM_FW_IMAGE_PATH "/vendor/modem/modem_fw/"
#define MODEM_FW_HOT_PATCH_IMAGE_PATH "/patch_hw/hot_modem_fw/"
#define NVM_IMAGE_PATH "/mnt/modem/mnvm2:0/modem_nv/"
#define MODEM_FW_XLOADER_PATH "/vendor/modem/modem_fw/xloader.bin"
#define MODEM_FW_XLOADER_HOT_PATCH_PATH "/mnt/modem/patch_hw/xloader.bin"
#define MODEM_FW_XLOADER_DDRTEST "/vendor/modem/modem_fw/xloader_test.bin"
#define MODEM_FW_XLOADER_MNTN_PATH "/vendor/modem/modem_fw/xloader_ddr_mntn.bin"
#define MODEM_FW_HIBOOT_DDRTEST "/vendor/modem/modem_fw/hiboot_ddrtest.img"
#define FILE_NAME_MAX_LEN 64

#define MODEM_BOOT_LOAD_SIZE (0x40000)
#define PCIE_IMAGE_DMA_SIZE (0x100000)
#define MODEM_BOOT_LOAD_FINISH (0xF)

#define MODEM_BOOT_MODE_UART "UART"
#define MODEM_BOOT_MODE_PCIE "PCIE"

enum modem_load_img_type {
    MODEM_LOAD_IMG_NVDEFAULT = 0,
    MODEM_LOAD_IMG_NVBACK,
    MODEM_LOAD_IMG_NVIMG,
    MODEM_LOAD_IMG_NVDLOAD,
    MODEM_LOAD_IMG_NVCUST,
    MODEM_LOAD_IMG_UCE = 5,
    MODEM_LOAD_IMG_HIBOOT,
    MODEM_LOAD_IMG_HIBOOT_TEST,
    MODEM_LOAD_IMG_DTS,
    MODEM_LOAD_IMG_LPM3,
    MODEM_LOAD_IMG_HIFI = 10,
    MODEM_LOAD_IMG_TEEOS,
    MODEM_LOAD_IMG_BL31,
    MODEM_LOAD_IMG_BOOTIMG,
    MODEM_LOAD_FINISH,
    MODEM_LOAD_IMG_MAX,
};

enum modem_load_file_type {
    MODEM_LOAD_FILE_FS = 0,
    MODEM_LOAD_FILE_NON_FS = 1,
};

enum pcie_sysboot_cmd_resp {
    PCIE_CMD_RESP_IDLE,
    PCIE_CMD_RESP_LOADING,
    PCIE_CMD_RESP_BOOT,
};

struct modem_load_info {
    volatile unsigned long thread_flags;

    void *pcie_load_img_virt_addr;
    void *pcie_dma_virt_addr;
    void *pcie_dma_phy_addr;

    struct device dev;
    struct semaphore msi_cmd_semaphore;
    struct wakeup_source load_wakelock;
};
#define MODEM_LOAD_THREAD_FLAG_BIT_SIGNAL_HALT (0)
#define MODEM_LOAD_THREAD_FLAG_BIT_STATE_HALT (1)

/* msg AP->MODEM */
struct pcie_msi_cmd_send_data {
    enum modem_load_img_type cmd_type;
    u32 ap_addr; /* ap image address */
    u32 ap_len;  /* ap image length */
    enum pcie_sysboot_cmd_resp cmd_status;
};
/* msg MODEM->AP */
struct pcie_msi_cmd_resp_data {
    enum modem_load_img_type cmd_type;
    u32 des_addr; /* image target address */
    u32 des_len;  /* image target length */
    u32 offset;   /* image offset */
};

struct modem_load_img_info {
    enum modem_load_img_type img_type;
    enum modem_load_file_type file_type;
    const char *dir;
    const char *name;
    const char *hot_patch_dir;
};

struct modem_load_debug_info {
    u32 intr_cnt;
    u32 load_start;
    u32 load_flag;
    u32 receive_offset;
    u32 receive_len;
    u32 send_len;
    u32 finish_status;
};

void modem_load_debug_print(void);
void modem_load_debug_clear(void);
int modem_xloader_dbg_mem_init(void);
int modem_load_save_file(char *filename, const void *data, u32 length, u32 max_size);
void modem_xloader_dbg(void);
int modem_load_debug_reset(void);
int modem_load_mode_get(void);
int modem_load_msi_int_task(void *data);

int bsp_modem_load_init(void);
int bsp_modem_load_start(void);
void bsp_modem_load_stop(void);
void bsp_modem_load_thread_hold_enable(void);
void bsp_modem_load_thread_hold_disable(void);

void modem_load_save_xloader(void);

#endif /* #ifndef __MODEM_LOAD_H__ */
