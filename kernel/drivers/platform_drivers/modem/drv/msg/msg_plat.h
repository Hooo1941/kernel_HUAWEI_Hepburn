/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2018. All rights reserved.
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
#ifndef _MSG_PLAT_H_
#define _MSG_PLAT_H_
#include <product_config.h>

#include <linux/version.h>
#include <linux/types.h>
#include <linux/list.h>     /* struct list_head */
#include <linux/rbtree.h>   /* struct rb_node */
#include <linux/kthread.h>  /* struct task_struct */
#include <linux/spinlock.h> /* struct spinlock */
#include <uapi/linux/sched/types.h> /* struct sched_param */

#include <osl_malloc.h>
#include "securec.h"
#include "bsp_print.h"
#include "bsp_slice.h"
#include <bsp_dt.h>
#include "msg_sha_def.h"
// platform definition
#include "bsp_msg.h"

#define THIS_CORE MSG_CORE_APP
#define MSG_CURRENT_CORE_ID MSG_CID_AP
#define MSG_CURRENT_CORE_MID DRV_MID_MSG

#define THIS_MODU mod_msg
#define msg_crit(fmt, ...) bsp_fatal(fmt, ##__VA_ARGS__)
#define msg_err(fmt, ...) bsp_err(fmt, ##__VA_ARGS__)
#define msg_warn(fmt, ...) bsp_wrn(fmt, ##__VA_ARGS__)
#define msg_print(fmt, ...) bsp_info(fmt, ##__VA_ARGS__)
#define msg_always(fmt, ...) bsp_err(fmt, ##__VA_ARGS__)
#define msg_trace(fmt, ...)

#endif
