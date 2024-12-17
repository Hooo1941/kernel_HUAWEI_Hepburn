/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
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
 */
#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/errno.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/semaphore.h>
#include <linux/mutex.h>
#include <linux/delay.h>

#include "securec.h"

#include "mdrv_msg.h"
#include "msg_id.h"
#include "mdrv_diag.h"

#include "rrc_pcs_interface.h"
#include "pcs_log.h"
#include "pcs_bc_cap.h"
#include "pcs_rrc_msg_proc.h"

struct pcs_msg_list {
	struct list_head list;
	u32 len;
	struct pcs_msg_head *msg;
};

struct pcs_msg_ctx_s {
	struct list_head list;
	struct msg_chn_hdl *msgchn_hdl;
	struct mutex mutex;
	struct semaphore sem;
	u8 rcv_flag;    /* Indicates whether a message is being received. */
	u8 exit_flag;
};

struct task_struct *pcs_task = NULL;
struct pcs_msg_ctx_s pcs_msg_ctx;

STATIC bool pcs_is_msg_invalid(const struct msg_addr *src, void *data, u32 len)
{
	if (unlikely(src == NULL)) {
		pcs_log_e("src null");
		return true;
	}

	if (unlikely(data == NULL)) {
		pcs_log_e("msg null");
		return true;
	}

	if (unlikely(len < sizeof(struct pcs_msg_head))) {
		pcs_log_e("len %d", len);
		return true;
	}

	return false;
}

STATIC void pcs_show_msg_ctx(void)
{
	struct pcs_msg_list *node = NULL;
	struct list_head *pos = NULL;

	pcs_log_i("entry");

	mutex_lock(&(pcs_msg_ctx.mutex));
	list_for_each(pos, &pcs_msg_ctx.list) {
		node = list_entry(pos, struct pcs_msg_list, list);
		pcs_log_i("len: %d, msg_type: %d, modem_id: %d",
			  node->len, node->msg->msg_type, node->msg->modem_id);
	}
	mutex_unlock(&(pcs_msg_ctx.mutex));
}

STATIC int pcs_rcv_msg(const struct msg_addr *src, void *data, unsigned len)
{
	struct pcs_msg_list *node = NULL;
	struct pcs_msg_head *msg = NULL;
	int ret = 0;

	pcs_msg_ctx.rcv_flag = 1;

	pcs_log_h("entry");

	if (pcs_msg_ctx.exit_flag) {
		pcs_log_e("pcs thread has exited!");
		ret = -ESRCH;
		goto err;
	}

	if (pcs_is_msg_invalid(src, data, len)) {
		ret = -EINVAL;
		goto err;
	}

	node = kzalloc(sizeof(*node), GFP_KERNEL);
	if (node == NULL) {
		pcs_log_e("kzalloc node fail");
		ret = -ENOMEM;
		goto err;
	}

	msg = kzalloc(len, GFP_KERNEL);
	if (msg == NULL) {
		pcs_log_e("kzalloc msg fail");
		ret = -ENOMEM;
		goto err_alloc_msg;
	}

	if (memcpy_s(msg, len, data, len)) {
		pcs_log_e("memcpy_s msg fail");
		ret = -EINVAL;
		goto err_memcpy;
	}

	node->len = len;
	node->msg = msg;

	mutex_lock(&(pcs_msg_ctx.mutex));
	list_add_tail(&node->list, &pcs_msg_ctx.list);
	mutex_unlock(&(pcs_msg_ctx.mutex));

	pcs_show_msg_ctx();

	up(&pcs_msg_ctx.sem);
	pcs_msg_ctx.rcv_flag = 0;

	return 0;

err_memcpy:
	kfree(msg);

err_alloc_msg:
	kfree(node);

err:
	pcs_msg_ctx.rcv_flag = 0;
	return ret;
}

int pcs_send_msg(struct pcs_msg_head *msg, u32 len)
{
	struct msg_addr dst = {
		.core = MSG_CORE_TSP,
		.chnid = MSG_CHN_NRRC,
	};

	pcs_log_h("entry");

	if (mdrv_msgchn_lite_sendto(pcs_msg_ctx.msgchn_hdl, &dst, msg, len) != 0) {
		pcs_log_e("mdrv_msgchn_lite_sendto fail");
		return -EINVAL;
	}

	return 0;
}

STATIC int pcs_open_msgchn(void)
{
	struct msgchn_attr attr = {0};

	mdrv_msgchn_attr_init(&attr);
	attr.chnid = MSG_CHN_NRRC;
	attr.coremask = MSG_CORE_MASK(MSG_CORE_TSP);
	attr.lite_notify = pcs_rcv_msg;

	pcs_msg_ctx.msgchn_hdl = mdrv_msgchn_lite_open(&attr);
	if (pcs_msg_ctx.msgchn_hdl == NULL) {
		pcs_log_e("mdrv_msgchn_lite_open fail");
		return -EINVAL;
	}

	return 0;
}

STATIC void pcs_init_msg_ctx(void)
{
	INIT_LIST_HEAD(&pcs_msg_ctx.list);
	mutex_init(&pcs_msg_ctx.mutex);
	sema_init(&pcs_msg_ctx.sem, 0);
	pcs_msg_ctx.msgchn_hdl = NULL;
	pcs_msg_ctx.rcv_flag = 0;
	pcs_msg_ctx.exit_flag = 0;
}

STATIC void pcs_proc_msg(void)
{
	struct pcs_msg_list *node = NULL;
	struct list_head *pos = NULL;
	struct list_head *n = NULL;
	struct list_head doing_list = LIST_HEAD_INIT(doing_list);
	u32 msg_type;

	pcs_log_d("entry");

	mutex_lock(&pcs_msg_ctx.mutex);
	list_replace_init(&pcs_msg_ctx.list, &doing_list);
	mutex_unlock(&pcs_msg_ctx.mutex);

	list_for_each_safe(pos, n, &doing_list) {
		node = list_entry(pos, struct pcs_msg_list, list);
		list_del(&node->list);

		msg_type = node->msg->msg_type;
		if (msg_type >= NRRC_PCS_BC_CAP_REQ &&
		    msg_type < NRRC_PCS_MSG_BUTT)
			pcs_bc_cap_msg_proc(node->msg, node->len);
		else if (msg_type >= RRC_PCS_READ_FILE_REQ &&
			 msg_type <= RRC_PCS_READ_FILE_ACK)
			pcs_rrc_msg_proc(node->msg, node->len);
		else
			pcs_log_e("msg type[%d] invalid", msg_type);

		kfree(node->msg);
		kfree(node);
	}
}

STATIC void pcs_deinit_msg_ctx(void)
{
	struct pcs_msg_list *node = NULL;
	struct list_head *pos = NULL;
	struct list_head *n = NULL;

	while (pcs_msg_ctx.rcv_flag) {
		msleep(1);
	}

	mutex_lock(&pcs_msg_ctx.mutex);
	list_for_each_safe(pos, n, &pcs_msg_ctx.list) {
		node = list_entry(pos, struct pcs_msg_list, list);
		list_del(&node->list);
		kfree(node->msg);
		kfree(node);
		node = NULL;
	}
	mutex_unlock(&pcs_msg_ctx.mutex);

	mutex_destroy(&pcs_msg_ctx.mutex);
}

STATIC int pcs_thread_task(void *args)
{
	pcs_log_h("entry");

	while (!pcs_msg_ctx.exit_flag) {
		down(&pcs_msg_ctx.sem);
		pcs_proc_msg();
	}

	while (!kthread_should_stop())
		msleep(1);

	pcs_deinit_msg_ctx();
	pcs_bc_cap_deinit();
	pcs_rrc_deinit();

	return 0;
}

int pcs_module_init(void)
{
	int ret = 0;

	ret |= pcs_bc_cap_init();
	ret |= pcs_rrc_init();
	if (ret)
		return ret;

	pcs_init_msg_ctx();
	ret = pcs_open_msgchn();
	if (ret) {
		pcs_msg_ctx.exit_flag = 1;
		pcs_deinit_msg_ctx();
		return ret;
	}

	pcs_task = kthread_run(pcs_thread_task, NULL, "pcs_thread");
	if (IS_ERR(pcs_task)) {
		pcs_log_e("creat task error, ret = %ld", PTR_ERR(pcs_task));
		return PTR_ERR(pcs_task);
	}

	return 0;
}

void pcs_module_exit(void)
{
	pcs_log_h("entry");
	pcs_msg_ctx.exit_flag = 1;
	up(&pcs_msg_ctx.sem);
	kthread_stop(pcs_task);
}

#if !IS_ENABLED(CONFIG_MODEM_DRIVER)
module_init(pcs_module_init);
module_exit(pcs_module_exit);
#endif
