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
#include <product_config.h>
#include <linux/io.h>
#include <osl_types.h>
#include <osl_bio.h>
#include <bsp_print.h>
#include <bsp_sysboot.h>
#define THIS_MODU mod_sysboot

#if defined(CONFIG_SYSBOOT_PARA) && !defined(CONFIG_LOAD_SEC_IMAGE)

/* 修改SYSBOOT_PARA_OFFSET需要同步修改以下文件:
    modem/drv/ccore/include/fusion/bsp_sysboot.h
    modem/drv/teeos/include/bsp_modem_secboot.h
*/
#define SYSBOOT_PARA_OFFSET 0x300

u32 g_modem_para[MODEM_PARA_MAX];

int bsp_sysboot_set_modem_para(enum modem_para_id id, u32 value)
{
    if (id >= MODEM_PARA_MAX) {
        bsp_err("para id(%d) invalid, please check\n", id);
        return -1;
    }
    g_modem_para[id] = value;
    return 0;
}
int bsp_sysboot_set_boot_para(unsigned long addr, u32 size)
{
    unsigned long paddr;
    void *vaddr = NULL;
    char *para_base_addr = NULL;
    unsigned int i;

    paddr = addr;
    vaddr = ioremap_wc(paddr, size);
    if (vaddr == NULL) {
        bsp_err("ioremap_wc error\n");
        return -ENOMEM;
    }

    para_base_addr = (char *)vaddr + SYSBOOT_PARA_OFFSET;
    for (i = 0; i < MODEM_PARA_MAX; i++) {
        writel(g_modem_para[i], para_base_addr + i * sizeof(u32));
    }

    iounmap(vaddr);
    return 0;
}

#else
int bsp_sysboot_set_boot_para(unsigned long addr, u32 size)
{
    UNUSED(addr);
    UNUSED(size);
    return 0;
}

int bsp_sysboot_set_modem_para(enum modem_para_id id, u32 value)
{
    UNUSED(id);
    UNUSED(value);
    return 0;
}
#endif
