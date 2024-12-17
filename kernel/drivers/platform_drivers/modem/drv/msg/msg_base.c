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
#include <linux/version.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/pid.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/list.h>
#include <linux/rbtree.h>
#include <linux/spinlock.h>

#include "msg_plat.h"
#include "msg_base.h"

struct msg_tskque_ctrl {
    struct spinlock lock;
    struct rb_root root;
};
struct msg_tskque_ctrl g_msg_base_ctrl;

/* get queue group by task */
static struct msg_tskque_grp *tgq_search(struct rb_root *root, struct task_struct *tsk)
{
    struct msg_tskque_grp *grp = NULL;
    struct rb_node *node = root->rb_node;

    while (node != NULL) {
        grp = container_of(node, struct msg_tskque_grp, node);
        if (tsk < grp->tsk)
            node = node->rb_left;
        else if (tsk > grp->tsk)
            node = node->rb_right;
        else
            return grp;
    }
    return NULL;
}

/* insert queue group by task */
static int tqg_insert(struct rb_root *root, struct msg_tskque_grp *newblock)
{
    struct rb_node *parent = NULL;
    struct msg_tskque_grp *grp = NULL;
    struct rb_node **nextnode = &(root->rb_node);

    while (*nextnode) {
        grp = container_of(*nextnode, struct msg_tskque_grp, node);
        parent = *nextnode;
        if (newblock->tsk < grp->tsk) {
            nextnode = &((*nextnode)->rb_left);
        } else if (newblock->tsk > grp->tsk) {
            nextnode = &((*nextnode)->rb_right);
        } else {
            return MSG_ERR_EEXIST;
        }
    }
    rb_link_node(&newblock->node, parent, nextnode);
    rb_insert_color(&newblock->node, root);
    return 0;
}

struct msg_tskque_grp *msg_tqg_search(struct task_struct *tsk)
{
    unsigned long irq_flags;
    struct msg_tskque_grp *grp = NULL;
    spin_lock_irqsave(&g_msg_base_ctrl.lock, irq_flags);
    grp = tgq_search(&g_msg_base_ctrl.root, (struct task_struct *)tsk);
    spin_unlock_irqrestore(&g_msg_base_ctrl.lock, irq_flags);
    return grp;
}

/* create a task_queue group linked to pid */
int msg_tqg_create(struct task_struct *tsk, unsigned qid_num)
{
    int i;
    int ret;
    unsigned long flags;
    unsigned malloc_len;
    struct msg_tskque_grp *grp = NULL;

    if (IS_ERR_OR_NULL(tsk)) {
        return MSG_ERR_EINVAL;
    }

    if (qid_num > MDMMSG_TSKQ_MAX) {
        msg_err("max qnum %d supported\n", MDMMSG_TSKQ_MAX);
        return MSG_ERR_ECHRNG;
    }
    malloc_len = sizeof(struct msg_tskque_grp) + qid_num * sizeof(struct msg_tskque);

    grp = vmalloc(malloc_len);
    if (IS_ERR_OR_NULL(grp)) {
        return MSG_ERR_ENOMEM;
    }
    (void)memset_s(grp, malloc_len, 0, malloc_len);
    grp->tsk = tsk;
    grp->event = 0;
    grp->r_msg = NULL;
    grp->msg_cnt = 0;
    grp->q_num = qid_num;
    PENDING_ZERO(&grp->pending);
    spin_lock_init(&grp->lock);

    for (i = 0; i < qid_num; i++) {
        grp->queue[i].queueid = i;
        grp->queue[i].grp = grp;
        PENDING_ZERO(&grp->queue[i].pending);
        INIT_LIST_HEAD(&grp->queue[i].msg_list);
        grp->queue[i].r_msg = NULL;
        grp->queue[i].msg_cnt = 0;
    }

    spin_lock_irqsave(&g_msg_base_ctrl.lock, flags);
    ret = tqg_insert(&g_msg_base_ctrl.root, grp);
    spin_unlock_irqrestore(&g_msg_base_ctrl.lock, flags);
    if (ret != 0) {
        vfree(grp);
        msg_err("tqg has created @ task %d\n", tsk->pid);
        return ret;
    }
    return 0;
}

int msg_base_init(void)
{
    g_msg_base_ctrl.root = RB_ROOT;
    spin_lock_init(&g_msg_base_ctrl.lock);
    return 0;
}
