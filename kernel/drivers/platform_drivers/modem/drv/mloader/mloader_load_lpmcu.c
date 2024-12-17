/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
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
#include <linux/semaphore.h>
#include <linux/delay.h>
#include <bsp_mloader.h>
#include <bsp_ipc_fusion.h>

#include "mloader_err.h"

#define MLOADER_LPMCU_IMG "lpmcu_modem.bin"
#define LPMCU_MODEM_NOT_READY (0xA5A5A5A5)
#define LPMCU_MODEM_READY (0x5A5A5A5A)
#define LPMCU_MODEM_BOOT_WAIT_TIMEOUT (10000)

static struct semaphore g_lpmcu_ready_sem;
static volatile int g_lpmcu_modem_init = LPMCU_MODEM_NOT_READY;

void lpmcu_ready_int_handle(void *param)
{
    mloader_print_err("lpmcu modem init ok\n");
    g_lpmcu_modem_init = LPMCU_MODEM_READY;
}

int lpmcu_modem_init(void)
{
    int ret;
    ipc_handle_t phandle = NULL;
    u32 i;

    g_lpmcu_modem_init = LPMCU_MODEM_NOT_READY;
    sema_init(&g_lpmcu_ready_sem, 0);
    ret = bsp_ipc_open(IPC_ACPU_INT_SRC_MCPU_VERIFYMDM, &phandle);
    if (ret) {
        mloader_print_err("fail to open ipc, ret = %d\n", ret);
        return ret;
    }

    ret = bsp_ipc_connect(phandle, (ipc_cbk)lpmcu_ready_int_handle, NULL);
    if (ret) {
        mloader_print_err("fail to connect ipc, ret = %d\n", ret);
        return ret;
    }

    ret = bsp_mloader_load_modem_image(MLOADER_LPMCU_IMG, MODEM_IMG_LPMCU, 0);
    if (ret) {
        mloader_print_err("load lpmcu_modem err 0x%x\n", ret);
        return ret;
    }

    for (i = 0; i < LPMCU_MODEM_BOOT_WAIT_TIMEOUT; i++) {
        if (g_lpmcu_modem_init == LPMCU_MODEM_READY) {
            mloader_print_err("lpmcu modem init ok, continue to start modem\n");
            return 0;
        }
        udelay(100);
    }

    mloader_print_err("lpmcu modem init timeout\n");
    while (down_interruptible(&g_lpmcu_ready_sem)) { }
    return MLOADER_ERR_TIMEOUT;
}
