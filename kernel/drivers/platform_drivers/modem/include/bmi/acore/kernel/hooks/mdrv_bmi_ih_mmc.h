/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
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
#ifndef __MDRV_BMI_IH_MMC_H__
#define __MDRV_BMI_IH_MMC_H__
#include <linux/blk_types.h>
#include <linux/mmc/card.h>
#include <linux/mmc/host.h>
#include <hooks/mdrv_bmi_ih.h>
#include <queue.h>

int mmc_poweroff_notify(struct mmc_card *card, unsigned int notify_type);
int mmc_sleep(struct mmc_host *host);

DECLARE_IOT_HOOK(iot_hook_mmc_ioctl_ffu,
    IH_PROTO(struct mmc_queue *queue, unsigned int cmd, unsigned long arg, int *ret),
    IH_ARGS(queue, cmd, arg, ret));

DECLARE_IOT_HOOK(iot_hook_mmc_copy_cid,
    IH_PROTO(unsigned int *new_cid, unsigned int new_cid_len, unsigned int *old_cid, unsigned int old_cidlen),
    IH_ARGS(new_cid, new_cid_len, old_cid, old_cidlen));

DECLARE_IOT_HOOK(iot_hook_mmc_set_oldcard,
    IH_PROTO(unsigned int *nr_parts, struct mmc_card **oldcard),
    IH_ARGS(nr_parts, oldcard));

DECLARE_IOT_HOOK(iot_hook_mmc_notify_time,
    IH_PROTO(unsigned int *timeout, unsigned int notify_type),
    IH_ARGS(timeout, notify_type));

DECLARE_IOT_HOOK(iot_hook_mmc_suspend,
    IH_PROTO(struct mmc_host *host, bool is_suspend, int *err, int *registered),
    IH_ARGS(host, is_suspend, err, registered));

DECLARE_IOT_HOOK(iot_hook_mmc_resume,
    IH_PROTO(struct mmc_host *host, int *err, int *registered),
    IH_ARGS(host, err, registered));
#endif
