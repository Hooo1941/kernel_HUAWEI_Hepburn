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
#ifndef __MODEM_BOOT_H__
#define __MODEM_BOOT_H__

#include <linux/device.h>
#include <linux/pm_wakeup.h>
#include <linux/semaphore.h>
#include <linux/platform_device.h>

#define MODEM_BOOT_STATE_UNINITED (0)
#define MODEM_BOOT_STATE_INITED (1)
#define MODEM_BOOT_STATE_POWER_ON (2)
#define MODEM_BOOT_STATE_LOADING (3)
#define MODEM_BOOT_STATE_LOAD_DONE (4)
#define MODEM_BOOT_STATE_EP_PCIE_DONE (5)
#define MODEM_BOOT_STATE_BOOT_DONE (6)
#define MODEM_BOOT_STATE_RESETTING (7)
#define MODEM_BOOT_STATE_SHUTDOWN (8)

struct modem_boot_state {
    volatile unsigned long thread_flags;

    int current_state;
    int next_state;
    int outer_event;

    u32 gpio_rescan;
    struct wakeup_source boot_wakelock;
    struct wakeup_source work_wakelock;
    struct semaphore boot_evnet_sem;
};
#define MODEM_BOOT_THREAD_FLAG_BIT_SIGNAL_HALT (0)
#define MODEM_BOOT_THREAD_FLAG_BIT_STATE_HALT (1)

int bsp_modem_boot_reset(void);
int bsp_modem_boot_start(void);

void bsp_modem_boot_set_status(int state);
void bsp_modem_boot_trigger_event(int event);
void bsp_modem_boot_set_modemready_state(void);

struct platform_device *modem_boot_get_device(void);

#endif
