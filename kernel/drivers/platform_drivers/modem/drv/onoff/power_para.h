/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
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

#ifndef __POWER_PARA_H__
#define __POWER_PARA_H__

struct sysboot_dpart_info {
    unsigned int boot_part;
    unsigned int switch_type;
    unsigned int is_switched;
    unsigned int dpart_enable;
    unsigned int curr_boot_times;
    unsigned int escape_flag;
    unsigned int boot_part_switched;
};

/*****************************************************************************
     *                           Attention                           *
 * Description : Driver for sysboot
 * Core        : Acore、Ccore
 * Header File : the following head files need to be modified at the same time
 * : /acore/kernel/platform_drivers/onoff/power_para.h
     /hiboot/private / drivers/sysboot/sysboot_main.h
 ******************************************************************************/
typedef enum {
    SYSBOOT_MONITOR_DISABLED = 0,
    SYSBOOT_MONITOR_ENABLED,
} sysboot_monitor_state_e;

#ifdef CONFIG_MODEM_ONOFF_PARA
int sysboot_get_monitor_flag(void);
struct sysboot_dpart_info *sysboot_get_dpart_info(void);
#else
static inline int sysboot_get_monitor_flag(void)
{
    return SYSBOOT_MONITOR_DISABLED;
}
static inline struct sysboot_dpart_info *sysboot_get_dpart_info(void)
{
    return NULL;
}
#endif
/*****************************************************************************
     *                           Attention End                           *
 ******************************************************************************/

#endif
