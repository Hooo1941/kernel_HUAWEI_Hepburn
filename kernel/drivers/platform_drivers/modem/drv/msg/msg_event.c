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
#include <linux/spinlock.h>

#include "msg_base.h"

int msg_evt_send(struct task_struct *tsk, unsigned int event)
{
    int knock;
    unsigned long flags;
    struct msg_tskque_grp *grp = NULL;

    if (event == 0 || IS_ERR_OR_NULL(tsk)) {
        return MSG_ERR_EINVAL;
    }

    grp = msg_tqg_search(tsk);
    if (grp == NULL) {
        return MSG_ERR_ENXIO;
    }

    knock = 0;
    spin_lock_irqsave(&grp->lock, flags);
    grp->event |= event;
    if (PENDING_ISSET(&grp->pending, PENDDING_ON_E)) {
        if (grp->event_opt == MDRV_EVENT_ANY) {
            PENDING_CLR(&grp->pending, PENDDING_ON_E);
            knock = 1;
        } else if (grp->event_opt == MDRV_EVENT_ALL) {
            if ((grp->event & grp->event_req) == grp->event_req) {
                PENDING_CLR(&grp->pending, PENDDING_ON_E);
                knock = 1;
            }
        } else {
            BUG_ON(1);
        }
    }
    spin_unlock_irqrestore(&grp->lock, flags);

    if (knock) {
        wake_up_process(tsk);
        smp_wmb();
    }

    return 0; // not return ret;
}

/* 检查当前是否直接有符合的事件 */
static int _msg_evt_recv_ismeet_now(struct msg_tskque_grp *grp, unsigned req_event, unsigned opt, unsigned *rsp_event)
{
    int equit = 0;
    unsigned long flags;
    spin_lock_irqsave(&grp->lock, flags);
    BUG_ON(grp->pending != 0);
    if (opt == MDRV_EVENT_ANY) {
        if (grp->event & req_event) {
            *rsp_event = grp->event & req_event;
            grp->event = grp->event & (~req_event);
            equit = 1;
        }
    } else {
        if ((grp->event & req_event) == req_event) {
            *rsp_event = req_event;
            grp->event = grp->event & (~req_event);
            equit = 1;
        }
    }
    if (equit == 0) {
        grp->event_opt = opt;
        grp->event_req = req_event;
        PENDING_SET(&grp->pending, PENDDING_ON_E);
    }

    spin_unlock_irqrestore(&grp->lock, flags);
    return equit;
}
/* 检查在超时时间内是否有符合的事件 */
static int _msg_evt_recv_ismeet_intime(struct msg_tskque_grp *grp, unsigned req_event, unsigned time_out,
    unsigned *rsp_event)
{
    int ret;
    unsigned long flags;
    ktime_t expires, expire_off;
    ktime_t *to = NULL;
    long slack = 0;
    int equit = 0;

    if (time_out == MDRV_EVENT_WAIT_FOREVER) {
        to = NULL;
    } else {
        expire_off = ktime_set(time_out / HZ, (time_out % HZ) * (NSEC_PER_SEC / HZ));
        expires = ktime_add(ktime_get_raw(), expire_off);
        to = &expires;
        if (!rt_task(current)) {
            slack = current->timer_slack_ns;
        }
    }

    for (;;) {
        ret = schedule_hrtimeout_range(to, slack, HRTIMER_MODE_ABS);
        if (ret && signal_pending(current)) {
            // should not possible
            msg_crit("signal detected pid = %d\n", current->pid);
        }
        smp_rmb();

        set_current_state(TASK_INTERRUPTIBLE);
        /* stage 3: 唤醒后判定 */
        spin_lock_irqsave(&grp->lock, flags);

        if (!PENDING_ISSET(&grp->pending, PENDDING_ON_E)) {
            *rsp_event = grp->event & req_event;
            grp->event = grp->event & (~req_event);
            equit = 1;
        }
        spin_unlock_irqrestore(&grp->lock, flags);
        if (equit) {
            break;
        }
        // time out
        if (to != NULL && ktime_compare(*to, ktime_get_raw()) <= 0) {
            break;
        }
    }
    return equit;
}

int msg_evt_recv(unsigned int req_event, unsigned int opt, unsigned int time_out, unsigned int *rsp_event)
{
    int equit;
    unsigned long flags;
    struct msg_tskque_grp *grp = NULL;

    if (req_event == 0 || rsp_event == NULL) {
        return MSG_ERR_EINVAL;
    }
    if (opt != MDRV_EVENT_ANY && opt != MDRV_EVENT_ALL) {
        return MSG_ERR_EINVAL;
    }

    grp = msg_tqg_search(current);
    if (grp == NULL) {
        return MSG_ERR_ENXIO;
    }

    set_current_state(TASK_INTERRUPTIBLE);

    equit = _msg_evt_recv_ismeet_now(grp, req_event, opt, rsp_event);
    if (equit) {
        set_current_state(TASK_RUNNING);
        return 0;
    }
    equit = _msg_evt_recv_ismeet_intime(grp, req_event, time_out, rsp_event);
    if (equit == 0) {
        /* 如果超时，退出前再给一次机会看看能不能meet(符合) --能走到这个条件 一定是超时了，不然上个函数出不来 */
        spin_lock_irqsave(&grp->lock, flags);
        if (!PENDING_ISSET(&grp->pending, PENDDING_ON_E)) {
            *rsp_event = grp->event & req_event;
            grp->event = grp->event & (~req_event);
            equit = 1;
        } else {
            PENDING_CLR(&grp->pending, PENDDING_ON_E);
        }

        spin_unlock_irqrestore(&grp->lock, flags);
    }

    set_current_state(TASK_RUNNING);
    if (equit) {
        return 0;
    } else {
        return MSG_ERR_ETIME;
    }
}

/* 检查当前是否直接有符合的事件或者消息 */
static int _msg_evt_vrecv_ismeet_now(struct msg_tskque_grp *grp, unsigned req_event, unsigned opt, unsigned *rsp_event)
{
    int meet = 0;
    unsigned long flags;

    spin_lock_irqsave(&grp->lock, flags);
    BUG_ON(grp->pending != 0);
    if (opt == MDRV_EVENT_ANY) {
        if (grp->event & req_event) {
            *rsp_event = grp->event & req_event;
            grp->event = grp->event & (~req_event);
            meet = 1;
        }
    } else {
        if ((grp->event & req_event) == req_event) {
            *rsp_event = req_event; // same as grp->event & req_event
            grp->event = grp->event & (~req_event);
            meet = 1;
        }
    }
    if (meet == 0) {
        if (grp->msg_cnt) {
            *rsp_event = 0;
            meet = 1;
        } else {
            grp->event_opt = opt;
            grp->event_req = req_event;
            PENDING_SET(&grp->pending, PENDDING_ON_E | PENDDING_ON_V);
        }
    }

    spin_unlock_irqrestore(&grp->lock, flags);
    return meet;
}
/* 检查在超时时间内是否有符合的事件或消息 */
static int _msg_evt_vrecv_ismeet_intime(struct msg_tskque_grp *grp, unsigned req_event, unsigned time_out,
    unsigned *rsp_event)
{
    int ret;
    unsigned long flags;
    ktime_t expires, expire_off;
    ktime_t *to = NULL;
    long slack = 0;
    int meet = 0;
    if (time_out == MDRV_EVENT_WAIT_FOREVER) {
        to = NULL;
    } else {
        expire_off = ktime_set(time_out / HZ, (time_out % HZ) * (NSEC_PER_SEC / HZ));
        expires = ktime_add(ktime_get_raw(), expire_off);
        to = &expires;
        if (!rt_task(current)) {
            slack = current->timer_slack_ns;
        }
    }

    for (;;) {
        ret = schedule_hrtimeout_range(to, slack, HRTIMER_MODE_ABS);
        if (ret && signal_pending(current)) {
            // should not possible
            msg_crit("signal detected pid = %d\n", current->pid);
        }
        smp_rmb();

        set_current_state(TASK_INTERRUPTIBLE);
        /* stage 3: 唤醒后判定 */
        spin_lock_irqsave(&grp->lock, flags);

        if (!PENDING_ISSET(&grp->pending, PENDDING_ON_E)) {
            *rsp_event = grp->event & req_event;
            grp->event = grp->event & (~req_event);
            meet = 1;
            PENDING_CLR(&grp->pending, PENDDING_ON_V);
        } else if (!PENDING_ISSET(&grp->pending, PENDDING_ON_V)) {
            *rsp_event = 0;
            meet = 1;
            PENDING_CLR(&grp->pending, PENDDING_ON_E);
        }
        spin_unlock_irqrestore(&grp->lock, flags);
        if (meet) {
            break;
        }
        // time out
        if (to != NULL && ktime_compare(*to, ktime_get_raw()) <= 0) {
            break;
        }
    }
    return meet;
}

int msg_evt_vrecv(unsigned int req_event, unsigned int opt, unsigned int time_out, unsigned int *rsp_event)
{
    int meet;
    unsigned long flags;

    struct msg_tskque_grp *grp = NULL;

    if (req_event == 0 || rsp_event == NULL) {
        return MSG_ERR_EINVAL;
    }
    if (opt != MDRV_EVENT_ANY && opt != MDRV_EVENT_ALL) {
        return MSG_ERR_EINVAL;
    }

    grp = msg_tqg_search(current);
    if (grp == NULL) {
        return MSG_ERR_ENXIO;
    }

    set_current_state(TASK_INTERRUPTIBLE);

    meet = _msg_evt_vrecv_ismeet_now(grp, req_event, opt, rsp_event);
    if (meet) {
        set_current_state(TASK_RUNNING);
        return 0;
    }

    meet = _msg_evt_vrecv_ismeet_intime(grp, req_event, time_out, rsp_event);
    if (meet == 0) {
        /* 如果超时，退出前再给一次机会看看能不能meet(符合) --能走到这个条件 一定是超时了，不然上个函数出不来 */
        spin_lock_irqsave(&grp->lock, flags);
        if (!PENDING_ISSET(&grp->pending, PENDDING_ON_E)) {
            *rsp_event = grp->event & req_event;
            grp->event = grp->event & (~req_event);
            meet = 1;
            PENDING_CLR(&grp->pending, PENDDING_ON_V);
        } else if (!PENDING_ISSET(&grp->pending, PENDDING_ON_V)) {
            *rsp_event = 0;
            meet = 1;
            PENDING_CLR(&grp->pending, PENDDING_ON_E);
        } else {
            PENDING_CLR(&grp->pending, PENDDING_ON_E | PENDDING_ON_V);
        }
        spin_unlock_irqrestore(&grp->lock, flags);
    }

    set_current_state(TASK_RUNNING);
    if (meet) {
        return 0;
    } else {
        return MSG_ERR_ETIME;
    }
}
