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
#include <linux/bug.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/pid.h>
#include <linux/sched.h>
#include <linux/init_task.h>
#include <linux/kthread.h>
#include <linux/list.h>
#include <linux/spinlock.h>

#include "msg_base.h"

static struct msg_cookie *tskque_get(struct msg_tskque *que)
{
    struct msg_cookie *msg = NULL;
    if (!list_empty(&que->msg_list)) {
        msg = list_first_entry(&que->msg_list, struct msg_cookie, m_list);
        list_del(&msg->m_list);
    }
    return msg;
}

int msg_tskq_get_num(struct task_struct *tsk, unsigned qid)
{
    struct msg_tskque_grp *grp = NULL;
    struct msg_tskque *que = NULL;
    if (IS_ERR_OR_NULL(tsk)) {
        return MSG_ERR_EINVAL;
    }

    grp = msg_tqg_search(tsk);
    if (grp == NULL || qid >= grp->q_num) {
        return MSG_ERR_ENXIO;
    }
    que = &grp->queue[qid];
    return que->msg_cnt;
}

int msg_tskq_send(struct task_struct *tsk, unsigned qid, void *msg, int send_flags)
{
    int knock;
    unsigned long flags;
    struct msg_cookie *pmsg = NULL;
    struct msg_tskque *que = NULL;
    struct msg_tskque_grp *grp = NULL;

    if (msg == NULL || IS_ERR_OR_NULL(tsk)) {
        return MSG_ERR_EINVAL;
    }
    pmsg = (struct msg_cookie *)((char *)msg - sizeof(struct msg_cookie));
    grp = msg_tqg_search(tsk);
    if (grp == NULL || qid >= grp->q_num) {
        return MSG_ERR_ENXIO;
    }

    que = &grp->queue[qid];

    knock = 1;
    spin_lock_irqsave(&grp->lock, flags);
    if (PENDING_ISSET(&que->pending, PENDDING_ON_M)) {
        PENDING_CLR(&que->pending, PENDDING_ON_M);
        que->r_msg = pmsg;
    } else if (PENDING_ISSET(&grp->pending, PENDDING_ON_M)) {
        PENDING_CLR(&grp->pending, PENDDING_ON_M);
        grp->r_msg = pmsg;
    } else if (PENDING_ISSET(&grp->pending, PENDDING_ON_V)) {
        PENDING_CLR(&grp->pending, PENDDING_ON_V);
        if (send_flags) {
            list_add(&pmsg->m_list, &que->msg_list);
        } else {
            list_add_tail(&pmsg->m_list, &que->msg_list);
        }
        que->msg_cnt++;
        grp->msg_cnt++;
    } else {
        if (send_flags) {
            list_add(&pmsg->m_list, &que->msg_list);
        } else {
            list_add_tail(&pmsg->m_list, &que->msg_list);
        }
        que->msg_cnt++;
        grp->msg_cnt++;
        knock = 0;
    }
    spin_unlock_irqrestore(&grp->lock, flags);

    if (knock) {
        wake_up_process(tsk);
        smp_wmb();
    }

    return 0; // not return ret;
}

/* 检查在当前是否直接有符合的消息 */
static struct msg_cookie *_msg_tskq_recv_now(struct msg_tskque_grp *grp, unsigned qid)
{
    int i;
    unsigned long flags;
    struct msg_cookie *msg = NULL;
    struct msg_tskque *que = NULL;

    spin_lock_irqsave(&grp->lock, flags);
    if (qid == MDRV_MSG_QID_ANY) {
        BUG_ON(grp->pending != 0);
        BUG_ON(grp->r_msg != NULL);
        for (i = 0; i < grp->q_num && msg == NULL; i++) {
            que = &grp->queue[qid];
            msg = tskque_get(que);
            if (msg != NULL) {
                break;
            }
        }
        if (msg == NULL) {
            PENDING_SET(&grp->pending, PENDDING_ON_M);
        }
    } else {
        que = &grp->queue[qid];
        BUG_ON(que->pending != 0);
        BUG_ON(que->r_msg != NULL);
        msg = tskque_get(que);
        if (msg == NULL) {
            PENDING_SET(&que->pending, PENDDING_ON_M);
        }
    }
    if (msg != NULL) {
        que->msg_cnt--;
        grp->msg_cnt--;
    }
    spin_unlock_irqrestore(&grp->lock, flags);
    return msg;
}

/* 检查在超时时间内是否有符合的消息 */
static struct msg_cookie *_msg_tskq_recv_intime(struct msg_tskque_grp *grp, unsigned qid, unsigned time_out)
{
    int ret;
    unsigned long flags;
    ktime_t expires, expire_off;
    ktime_t *to = NULL;
    long slack = 0;
    struct msg_cookie *msg = NULL;
    struct msg_tskque *que = NULL;

    if (time_out == MDRV_MSG_WAIT_FOREVER) {
        to = NULL;
    } else {
        expire_off = ktime_set(time_out / HZ, (time_out % HZ) * (NSEC_PER_SEC / HZ));
        expires = ktime_add(ktime_get_raw(), expire_off);
        to = &expires;
        if (!rt_task(current)) {
            slack = current->timer_slack_ns;
        }
    }

    /* for循环，是为了防止condition4
    在任务第二次recv的时候可能收到sender迟来的为了第一次recv而送出的wakeup，导致第二次recv直接被唤醒返回
    概率极低 */
    for (;;) {
        /* stage 2 设置超时，调用schedule让出调度。返回的情况如下几种
        1, 被sender调用wakeup唤醒了，有消息。最常见情况(normal contion) --ok
        2，被信号或者超时唤醒了，在stage3前，sender一直无动作。(normal contion) --ok
        3，被信号或者超时唤醒了，在stage3前，sender塞来了消息，sender调用了wakeup(race conditn 无效wakeup) --ok
        4，被信号或者超时唤醒了, 在stage3前，sender塞来了消息，sender还没调用wakeup(race conditon)
        5，stage1刚检查完，还没调用schedule，sender任务已经塞了数据，并且调用了wakeup(race condtion 无效唤醒竞争)--ok
        */
        ret = schedule_hrtimeout_range(to, slack, HRTIMER_MODE_ABS);
        if (ret && signal_pending(current)) {
            // should not possible
            msg_crit("signal detected pid = %d, ret = %d\n", current->pid, ret);
        }
        smp_rmb();

        set_current_state(TASK_INTERRUPTIBLE);
        /* stage 3: 唤醒后判定 */
        spin_lock_irqsave(&grp->lock, flags);

        // 只要PENDDING_ON_M清掉了，必然sender塞了消息(condition 1,3,4)
        if (qid == MDRV_MSG_QID_ANY) {
            if (!PENDING_ISSET(&grp->pending, PENDDING_ON_M)) {
                msg = grp->r_msg;
                grp->r_msg = NULL;
            }
        } else {
            que = &grp->queue[qid];
            if (!PENDING_ISSET(&que->pending, PENDDING_ON_M)) {
                msg = que->r_msg;
                que->r_msg = NULL;
            }
        }
        spin_unlock_irqrestore(&grp->lock, flags);
        if (msg != NULL) {
            break;
        }
        // singal interrupted(impossible) or time out
        if (to != NULL && ktime_compare(*to, ktime_get_raw()) <= 0) {
            break;
        }
    }

    return msg;
}


int msg_tskq_recv(unsigned int qid, void **outmsg, unsigned int time_out)
{
    unsigned long flags;

    struct msg_cookie *msg = NULL;
    struct msg_tskque *que = NULL;
    struct msg_tskque_grp *grp = NULL;

    if (outmsg == NULL) {
        return MSG_ERR_EINVAL;
    }

    grp = msg_tqg_search(current);
    if (grp == NULL || (qid >= grp->q_num && qid != MDRV_MSG_QID_ANY)) {
        return MSG_ERR_ENXIO;
    }

    /* stage 0: 避免无效唤醒(condition5)，set state需要在条件检查之前 */
    set_current_state(TASK_INTERRUPTIBLE);

    /* stage 1： 睡眠条件检查，直接查询现有消息 */
    msg = _msg_tskq_recv_now(grp, qid);
    /* 有现成的消息，normal condtion */
    if (msg != NULL) {
        set_current_state(TASK_RUNNING);
        *outmsg = (void *)((char *)msg + sizeof(struct msg_cookie));
        return 0;
    }
    msg = _msg_tskq_recv_intime(grp, qid, time_out);
    // stage4 主要为了清除pending标志，顺便给一次取消息的最后机会, 如果msg已经取到，则pending标志已经被sender清除
    if (msg == NULL) {
        /* 如果超时，退出前再给一次机会看看能不能meet(符合) --能走到这个条件 一定是超时了，不然上个函数出不来 */
        spin_lock_irqsave(&grp->lock, flags);
        if (qid == MDRV_MSG_QID_ANY) {
            if (!PENDING_ISSET(&grp->pending, PENDDING_ON_M)) {
                msg = grp->r_msg;
                grp->r_msg = NULL;
            } else {
                PENDING_CLR(&grp->pending, PENDDING_ON_M);
            }
        } else {
            que = &grp->queue[qid];
            if (!PENDING_ISSET(&que->pending, PENDDING_ON_M)) {
                msg = que->r_msg;
                que->r_msg = NULL;
            } else {
                PENDING_CLR(&que->pending, PENDDING_ON_M);
            }
        }
        spin_unlock_irqrestore(&grp->lock, flags);
    }

    set_current_state(TASK_RUNNING);
    if (msg != NULL) {
        *outmsg = (void *)((char *)msg + sizeof(struct msg_cookie));
        return 0;
    } else {
        return MSG_ERR_ETIME;
    }
}
