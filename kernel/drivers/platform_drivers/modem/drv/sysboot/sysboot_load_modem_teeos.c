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
#include <linux/fs.h>
#include <linux/slab.h>
#include <bsp_print.h>
#include <bsp_sec_call.h>
#include "sysboot_sec_call.h"

#define THIS_MODU mod_sysboot

#define MODEM_TEEOS_VERIFY_TIME 3
#define MODEM_TEEOS_FILE_NAME_LEN 64
#define SEC_MODEM_FILE_PATH "/vendor/modem/modem_vendor/image/platdrv_libsec_modem.so.sec"

static int sysboot_load_image(const char *buf, int buf_size)
{
    int ret, i;

    ret = sysboot_teek_trans_data_to_os(buf, buf_size);
    if (ret) {
        bsp_err("trans data to os failed, ret = 0x%x\n", ret);
        return ret;
    }

    for (i = 0; i < MODEM_TEEOS_VERIFY_TIME; i++) {
        ret = sysboot_teek_load_modem_teeos();
        bsp_err("modem teeos verify time = %d, ret =0x%x\n", i + 1, ret);
        if (ret) {
            bsp_err("load modem teeos failed, ret = 0x%x\n", ret);
        } else {
            bsp_err("load modem teeos succeed\n");
            break;
        }
    }
    return ret;
}

int sysboot_load_modem_teeos_early_init(void)
{
    int ret;
    loff_t file_size;
    char *file_buffer = NULL;
    struct file *fp = NULL;
    loff_t offset = 0;

    ret = sysboot_teek_init();
    if (ret) {
        return ret;
    }

    fp = filp_open(SEC_MODEM_FILE_PATH, O_RDONLY, 0);
    if (IS_ERR_OR_NULL(fp)) {
        ret = (int)PTR_ERR(fp);
        bsp_err("filp_open(%s) failed, ret:%d", SEC_MODEM_FILE_PATH, ret);
        return ret;
    }

    file_size = i_size_read(file_inode(fp));
    file_buffer = kzalloc(file_size, GFP_KERNEL);
    if (file_buffer == NULL) {
        bsp_err("kzalloc %s failed, file_size = 0x%x\n", SEC_MODEM_FILE_PATH, file_size);
        ret = -ENOMEM;
        goto close_fp;
    }

    ret = kernel_read(fp, file_buffer, file_size, &offset);
    if (ret != file_size) {
        bsp_err("read %s failed, retval %d, require len %d\n", SEC_MODEM_FILE_PATH, ret, file_size);
        if (ret >= 0) {
            ret = -EIO;
        }
        goto free_mem;
    }

    ret = sysboot_load_image(file_buffer, file_size);

free_mem:
    kfree(file_buffer);
close_fp:
    (void)filp_close(fp, NULL);
    sysboot_teek_uninit();
    return ret;
}

int sysboot_load_modem_teeos_later_init(void)
{
    int ret;
    unsigned int param = 0;

    ret = bsp_sec_call(FUNC_MDRV_SYSBOOT_TEEOS_INIT, param);
    if (ret) {
        bsp_err("sec_call return error, ret = 0x%x\n", ret);
    }

    return ret;
}
