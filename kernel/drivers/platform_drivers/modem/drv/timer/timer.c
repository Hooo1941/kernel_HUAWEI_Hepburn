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

#include <timer_device.h>
#include <bsp_slice.h>
#include <osl_sem.h>
#include <osl_malloc.h>
#include <osl_thread.h>
#include <osl_irq.h>
#include <linux/device.h>
#include <linux/pm_wakeup.h>
#include <securec.h>

#define TIMER_NUM 40 /* 普通timer支持的最大软实例个数 */
#define TIMER_MAX_DBG_NUMBER 8

enum {
    TIMER_LIST_HEAD,
    TIMER_NON_LIST_HEAD
};

struct {
    char name[TIMER_NAME_MAX_LEN];
    u32 tick_num_period;
    u32 tick_num_setting;
} g_timer_debug[TIMER_MAX_DBG_NUMBER];
u32 g_timer_debug_count = 0;

typedef struct {
    char name[TIMER_NAME_MAX_LEN];
    bool overtimed;
    u16 id;
    u16 seq_no;
    u16 mode;                      /* 0：非周期；1：周期 */
    u32 tick_num;                  /* 对应timeout的tick个数 */
    u32 is_del;                    /* 0: 未被删除；1：被删除 */
    u64 expire_slice;              /* 期望的超时时刻 */
    timer_handle_cb routine;
    s32 para;
    struct list_head entry;

    u32 slice_pre_irqrcvd;
} timer_instance_s;

typedef struct {
    u32 timer_id;                /* 分配到的硬件资源的device索引 */
    u8 res_alc_grp;              /* 指示哪些组有空闲的资源，每个bit表示1个组。0：有空闲资源；1：无空闲资源 */
    u8 res_alc_msk[0x5];           /* 每个组的资源分配情况，每个bit表示1个资源。0：空闲态；1：占用态 */
    u16 seq_no[TIMER_NUM];       /* SN校验 */
    bool is_running;             /* 表示device是否正在计时 */
    u64 timeout_slice_latest;    /* 记录最近一次超时中断到来的时刻 */
    struct list_head list_head;  /* 有序链表头 */
    spinlock_t list_lock;
    osl_sem_id task_sem;         /* 空闲时timer任务处于等待该信号量的就绪态 */
    struct wakeup_source *wake_lock;
} timer_ctrl_s;

timer_ctrl_s g_timer_ctrl[TIMER_TYPE_MAX];

typedef union {
    struct {
        u16 seq_no; /* 序列号，防止已经释放了资源的用户再次使用资源，引起错误 */
        u8 type;    /* TIMER_TYPE_N_WKSRC: 非唤醒源；TIMER_TYPE_WKSRC: 唤醒源 */
        s8 id;      /* id须具有表征handler正负的能力，小端系统下放在结构体的最后 */
    };
    timer_handle handle;
} timer_hdl_info_u;

u8 const g_timer_res_map[] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};
u8 const g_timer_res_map_comp[] = {0xfe, 0xfd, 0xfb, 0xf7, 0xef, 0xdf, 0xbf, 0x7f}; /* map表的补码 */
u8 const g_timer_res_unmap[0x100] = {
    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 5,
    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 6,
    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 5,
    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 7,
    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 5,
    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 6,
    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 5,
    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 8
};


/* free接口的并发情况由调用者加锁保护 */
static inline void timer_free(timer_ctrl_s *ctrl, timer_instance_s *instance)
{
    u32 timer_id = instance->id;

    u32 grp_idx = timer_id >> 0x3;
    u32 pos_idx = timer_id & 0x7;

    ctrl->res_alc_grp &= g_timer_res_map_comp[grp_idx];
    ctrl->res_alc_msk[grp_idx] &= g_timer_res_map_comp[pos_idx];

    osl_free((void *)instance);

    return;
}

static inline bool timer_is_inst_alloc(timer_ctrl_s *ctrl, timer_hdl_info_u handle_info)
{
    u32 timer_id = (u32)(handle_info.id);
    u16 seq_no = handle_info.seq_no;
    u32 grp_idx = timer_id >> 0x3;
    u32 tbl_idx = timer_id & 0x7;
    if ((ctrl->res_alc_msk[grp_idx] & g_timer_res_map[tbl_idx]) && (ctrl->seq_no[timer_id] == seq_no)) {
        return true;
    } else {
        return false;
    }
}

/*********************************************
约束：涉及链表操作的接口，并发情况由调用者加锁保护
*********************************************/
static inline timer_instance_s *timer_search_list(timer_ctrl_s *ctrl, u32 timer_id)
{
    timer_instance_s *instance = NULL;

    list_for_each_entry(instance, &(ctrl->list_head), entry) {
        if (instance->id == timer_id) {
            return instance;
        }
    }
    return NULL;
}

static inline s32 timer_insert_list(timer_ctrl_s *ctrl, timer_instance_s *inst_insert)
{
    timer_instance_s *instance = NULL;
    list_for_each_entry(instance, &(ctrl->list_head), entry) {
        if (instance->expire_slice > inst_insert->expire_slice) {
            break;
        }
    }

    list_add_tail(&(inst_insert->entry), &(instance->entry));
    if (ctrl->list_head.next == &(inst_insert->entry)) {
        return TIMER_LIST_HEAD;
    } else {
        return TIMER_NON_LIST_HEAD;
    }
}

static inline void timer_remove_list(timer_ctrl_s *ctrl, timer_instance_s *instance)
{
    list_del_init(&(instance->entry));
    timer_free(ctrl, instance);
    return;
}

static inline s32 timer_alloc(timer_ctrl_s *ctrl, u32 *timer_id)
{
    u32 grp_idx;
    u32 pos_idx;
    u32 id;
    unsigned long flags;
    timer_instance_s *cur_inst = NULL;
    timer_instance_s *next_inst = NULL;
    spin_lock_irqsave(&(ctrl->list_lock), flags);

    list_for_each_entry_safe(cur_inst, next_inst, &(ctrl->list_head), entry) {
        /* 处理删除节点，避免长时间没删除导致资源不够用 只针对没有超时在add中释放，避免在free后使用，其它释放在任务上下文进行 */
        if ((cur_inst->is_del == 1) && (cur_inst->overtimed == false)) {
            timer_remove_list(ctrl, cur_inst);
        }
    }
    /* res_alc_grp */
    if (ctrl->res_alc_grp >= 0x1f) {
        spin_unlock_irqrestore(&(ctrl->list_lock), flags);
        timer_err("Fail to alloc. Resource is exhaust. res_alc_grp=%d.\n", (u32)ctrl->res_alc_grp);
        return BSP_ERROR;
    }

    grp_idx = g_timer_res_unmap[ctrl->res_alc_grp]; /* 确定去哪个组分配资源 */
    pos_idx = g_timer_res_unmap[ctrl->res_alc_msk[grp_idx]]; /* 找到本组中待分配的空闲资源的位置 */
    id = (grp_idx << 0x3) + pos_idx; /* 根据组号和位置索引计算出id */

    /* 更新资源分配情况 */
    ctrl->res_alc_msk[grp_idx] |= g_timer_res_map[pos_idx];
    if (ctrl->res_alc_msk[grp_idx] == 0xff) {
        ctrl->res_alc_grp |= g_timer_res_map[grp_idx];
    }
    ctrl->seq_no[id]++;
    spin_unlock_irqrestore(&(ctrl->list_lock), flags);

    *timer_id = id;

    return BSP_OK;
}

static inline void timer_set_del_status(timer_ctrl_s *ctrl, u32 timer_id)
{
    timer_instance_s *instance = timer_search_list(ctrl, timer_id);
    if (instance != NULL) {
        instance->is_del = 1;
    }
    return;
}

static inline void timer_update_list(timer_ctrl_s *ctrl, timer_instance_s **timeout_instance, u32 instance_num,
    timer_hdl_info_u *handle_info, u32 handle_num)
{
    u32 i;
    timer_instance_s *instance = NULL;

    for (i = 0; i < instance_num; i++) {
        if ((timer_is_inst_alloc(ctrl, handle_info[i])) && (timeout_instance[i]->overtimed == true)) {
            instance = timeout_instance[i];
            list_del_init(&(instance->entry));
            if ((instance->mode == 1) && (instance->is_del == 0)) {
                instance->expire_slice = instance->tick_num + ctrl->timeout_slice_latest;
                instance->overtimed = false;
                (void)timer_insert_list(ctrl, instance);
            } else {
                timer_free(ctrl, instance);
            }
        }
    }
    return;
}

static inline u32 timer_calc_delta_tick(u64 expire_slice)
{
    u64 cur_slice = 0;
    s64 tick_num;
    (void)bsp_slice_getcurtime(&cur_slice);
    tick_num = expire_slice - cur_slice;
    /* arm timer特性，如果时间设置为0则会立即产生中断，不受使能bit控制 */
    if (tick_num < 1) {
        tick_num = 1;
    }

    return (u32)tick_num;
}

static inline void timer_run_list(timer_ctrl_s *ctrl)
{
    u32 tick_num;
    timer_device_s *device = NULL;
    timer_instance_s *first_node = list_first_entry(&(ctrl->list_head), timer_instance_s, entry);
    if (first_node->overtimed == true) {
        ctrl->is_running = false;
        return;
    }

    tick_num = timer_calc_delta_tick(first_node->expire_slice);
    device = timer_get_device(ctrl->timer_id);
    timer_start(device, TIMER_MODE_ONESHOT, tick_num);
    ctrl->is_running = true;

    return;
}

void timer_restart(timer_ctrl_s *ctrl)
{
    if (list_empty(&(ctrl->list_head))) {
        timer_device_s *device = timer_get_device(ctrl->timer_id);
        device->driver_ops->disable((void *)(&(device->ctrl_value_latest)), device->base_addr);
        return;
    }

    timer_run_list(ctrl);

    return;
}

static inline void timer_calc_expire_slice(timer_instance_s *timer, u32 time, u64 cur_slice)
{
    u64 tick_num = timer_calc_tick_num(time, TIMER_CLK_FREQ_32K);
    timer->expire_slice = tick_num + cur_slice;
    timer->tick_num = tick_num;
    return;
}

static inline void timer_add_init(timer_instance_s *instance, timer_attr_s *attr)
{
    u64 cur_slice;
    errno_t ret = memcpy_s(instance->name, sizeof(instance->name),
        attr->name, strlen(attr->name) + 1);
    if (ret != EOK) {
        timer_err("fail to copy name. ret=0x%x. dst_size=%lu, src_size=%lu.\n",
            ret, sizeof(instance->name), strlen(attr->name));
    }
    instance->overtimed = false;
    instance->mode = attr->flag & TIMER_PERIOD ? TIMER_MODE_PERIOD : TIMER_MODE_ONESHOT;
    instance->routine = attr->callback;
    instance->para = attr->para;
    INIT_LIST_HEAD(&(instance->entry));
    (void)bsp_slice_getcurtime(&cur_slice);
    timer_calc_expire_slice(instance, attr->time, cur_slice);

    instance->slice_pre_irqrcvd = (u32)cur_slice;

    return;
}

timer_handle mdrv_timer_add(timer_attr_s *attr)
{
    u32 type;
    u32 timer_id = 0;
    timer_ctrl_s *ctrl = NULL;
    unsigned long flags;
    s32 pos_insert;
    timer_instance_s *instance = NULL;

    timer_hdl_info_u handle_info = {
        .handle = 0
    };

    if ((unlikely(attr == NULL)) || (unlikely(attr->callback == NULL))) {
        timer_err("fail to add timer.attr=%p, callback=%p.\n", attr, attr->callback);
        handle_info.id = MDRV_TIMER_ENO_INPUT;
        return handle_info.handle;
    }

    /* 此处申请的内存可能会在2处释放：1.用户调用delete接口释放；2.单次定时在超时时释放 */
    instance = (timer_instance_s *)osl_malloc(sizeof(timer_instance_s));
    if (instance == NULL) {
        handle_info.id = MDRV_TIMER_ENO_CREATE;
        timer_err("fail to malloc for %s.\n", attr->name);
        return handle_info.handle;
    }
    (void)memset_s(instance, sizeof(timer_instance_s), 0, sizeof(timer_instance_s));

    type = (attr->flag & TIMER_WAKE) ? TIMER_TYPE_WKSRC : TIMER_TYPE_N_WKSRC;
    handle_info.type = type;

    ctrl = (timer_ctrl_s *)(&g_timer_ctrl[type]);
    if (timer_alloc(ctrl, &timer_id)) {
        osl_free((void *)instance);
        handle_info.id = MDRV_TIMER_ENO_CREATE;
        timer_err("fail to alloc resource for %s.\n", attr->name);
        return handle_info.handle;
    }
    instance->id = timer_id;
    handle_info.id = timer_id;

    /* 初始化链表，计算tick个数，设置周期性、回调函数、是否发送消息 */
    timer_add_init(instance, attr);

    /* 对比链表首节点和新用户的超时时刻，如果新用户的超时时刻晚，则直接将新节点插入到链表中；
       如果早，则插入到链表的同时重新启动定时器 */
    spin_lock_irqsave(&(ctrl->list_lock), flags);
    instance->seq_no = ctrl->seq_no[timer_id];
    handle_info.seq_no = ctrl->seq_no[timer_id];
    pos_insert = timer_insert_list(ctrl, instance);
    if (pos_insert == TIMER_LIST_HEAD) {
        timer_run_list(ctrl);
    }
    spin_unlock_irqrestore(&(ctrl->list_lock), flags);

    return handle_info.handle;
}

s32 mdrv_timer_delete(timer_handle handle)
{
    unsigned long flags;
    timer_hdl_info_u handle_info;
    u32 type;
    u32 timer_id;
    u32 seq_no;
    timer_ctrl_s *ctrl;

    handle_info.handle = handle;
    type = (u32)(handle_info.type);
    timer_id = (u32)(handle_info.id);
    seq_no = (u32)handle_info.seq_no;
    if ((type >= TIMER_TYPE_MAX) || (timer_id >= TIMER_NUM)) {
        return MDRV_TIMER_ENO_INPUT;
    }
    ctrl = (timer_ctrl_s *)(&g_timer_ctrl[type]);

    /* SN检测，如果handle的序列号和系统中保存的序列号不一致，
       则有可能是资源已经不属于当前用户了，但是当前用户又错误地调用了delete接口释放资源 */
    if (seq_no != ctrl->seq_no[timer_id]) {
        return MDRV_TIMER_ENO_SN;
    }

    if (timer_is_inst_alloc(ctrl, handle_info) == false) {
        return MDRV_TIMER_ENO_INPUT;
    }

    /**********************************************************************
    case1:链表中存在当前用户。操作步骤：
        1、设置删除标记;
        2、在任务上下文进行删除节点处理
    case2:链表中不存在当前用户，不做任何操作。当前机制下不应该出现此种情况
    ***********************************************************************/
    spin_lock_irqsave(&(ctrl->list_lock), flags);
    timer_set_del_status(ctrl, timer_id);
    spin_unlock_irqrestore(&(ctrl->list_lock), flags);

    return MDRV_OK;
}

u32 timer_pop_timeout_inst(timer_ctrl_s *ctrl, timer_instance_s **timeout_inst)
{

    timer_instance_s *cur_inst = NULL;
    timer_instance_s *next_inst = NULL;
    u32 count = 0;
    u64 cur_expire_slice;

    if (list_empty(&(ctrl->list_head))) {
        return 0;
    }

    (void)bsp_slice_getcurtime(&cur_expire_slice);
    list_for_each_entry_safe(cur_inst, next_inst, &(ctrl->list_head), entry) {
        /* 同时处理与首用户超时时刻只差1个tick的用户 */
        if (cur_inst->expire_slice <= (cur_expire_slice + 1)) {
            cur_inst->overtimed = true;
            timeout_inst[count++] = cur_inst;
        } else {
            break;
        }
    }

    return count;
}

void timer_user_routine(timer_instance_s **instances, u32 node_num,
    timer_hdl_info_u *handle_info, u32 handle_num)
{
    u32 i;
    s32 ret;
    u32 slice_timeout;
    u32 slice = bsp_get_slice_value();
    timer_instance_s *cur_instance = NULL;

    for (i = 0; i < node_num; i++) {
        cur_instance = instances[i];
        handle_info[i].id = (s8)cur_instance->id;
        handle_info[i].seq_no = cur_instance->seq_no;
        if (cur_instance->routine != NULL) {
            cur_instance->routine(cur_instance->para);
        }

        slice_timeout = slice - cur_instance->slice_pre_irqrcvd;
        cur_instance->slice_pre_irqrcvd = slice;
        ret = strcpy_s(g_timer_debug[g_timer_debug_count].name, TIMER_NAME_MAX_LEN, cur_instance->name);
        if (ret != EOK) {
            timer_err("user callback, fail to copy %s.\n", cur_instance->name);
        }
        g_timer_debug[g_timer_debug_count].tick_num_period = slice_timeout;
        g_timer_debug[g_timer_debug_count++].tick_num_setting = cur_instance->tick_num;
        g_timer_debug_count &= 0x7; /* 最大记录8个 */
    }

    return;
}

void timer_timeout_callback(u32 type, timer_ctrl_s *ctrl)
{
    unsigned long flags;
    timer_instance_s *timeout_inst[TIMER_NUM] = { NULL };
    u32 inst_num;
    timer_hdl_info_u handle_info[TIMER_NUM];
    timer_instance_s *cur_inst = NULL;
    timer_instance_s *next_inst = NULL;
    spin_lock_irqsave(&(ctrl->list_lock), flags);
    list_for_each_entry_safe(cur_inst, next_inst, &(ctrl->list_head), entry) {
        /* 处理删除节点，避免在delete接口中free内存导致和用户回调冲突 */
        if (cur_inst->is_del == 1) {
            timer_remove_list(ctrl, cur_inst);
        }
    }

    /* 分四个阶段处理，第一阶段获取当前超时的用户 */
    inst_num = timer_pop_timeout_inst(ctrl, timeout_inst);
    spin_unlock_irqrestore(&(ctrl->list_lock), flags);
    /* 实例被delete接口删除，超时到之后没有超时的节点，不进行回调处理，重启硬定时 */
    if (inst_num != 0) {
        /* 第二阶段处理用户的回调 */
        timer_user_routine(timeout_inst, inst_num, handle_info, TIMER_NUM);
    }

    /* 第三阶段更新定时任务队列。 */
    spin_lock_irqsave(&(ctrl->list_lock), flags);
    timer_update_list(ctrl, timeout_inst, inst_num, handle_info, TIMER_NUM);

    /* 第四阶段重新启动定时器。用户回调里有可能会调用delete接口启动了timer，此处需要判断下定时器的运行状态 */
    if (ctrl->is_running == false) {
        timer_restart(ctrl);
    }
    list_for_each_entry_safe(cur_inst, next_inst, &(ctrl->list_head), entry) {
        /* 处理删除节点，避免在delete接口中free内存导致和用户回调冲突 */
        if (cur_inst->is_del == 1) {
            timer_remove_list(ctrl, cur_inst);
        }
    }
    if (type == TIMER_TYPE_WKSRC) {
        __pm_relax(ctrl->wake_lock);
    }

    spin_unlock_irqrestore(&(ctrl->list_lock), flags);

    return;
}

s32 timer_task(void *obj)
{
    u32 type = (u32)(uintptr_t)obj;
    timer_ctrl_s *ctrl = (timer_ctrl_s *)(&g_timer_ctrl[type]);
    for (;;) {
        osl_sem_down(&(ctrl->task_sem));
        timer_timeout_callback(type, ctrl);
    }
    return BSP_OK;
}

s32 timer_create_task(u32 type, char *task_name)
{
    OSL_TASK_ID task_id = 0;
    s32 ret = osl_task_init(task_name, TIMER_TASK_PRIORITY, TIMER_TASK_STACK_SIZE,
        (void *)((uintptr_t)timer_task), (void *)((uintptr_t)type), &task_id);
    if (ret == ERROR) {
        timer_err("fail to create %s task.\n", task_name);
        return BSP_ERROR;
    }

    return BSP_OK;
}

OSL_IRQ_FUNC(static irqreturn_t, timer_routine, irq_id, arg)
{
    u32 type = (u32)(uintptr_t)arg;
    u32 timer_id;
    timer_device_s *device;
    timer_ctrl_s *ctrl = (timer_ctrl_s *)(&g_timer_ctrl[type]);
    ctrl->is_running = false;
    (void)bsp_slice_getcurtime(&ctrl->timeout_slice_latest);
    timer_id = ctrl->timer_id;

    device = timer_get_device(timer_id);
    if (device->driver_ops->is_int_enable(device->base_addr)) {
        device->driver_ops->clear_int(device->base_addr);
        if (type == TIMER_TYPE_WKSRC) {
            __pm_stay_awake(ctrl->wake_lock);
        }
        osl_sem_up(&(ctrl->task_sem));
    }

    return IRQ_HANDLED;
}

s32 timer_create_instance(u32 type, unsigned long irq_flags)
{
    s32 ret;
    u32 timer_id = 0;
    char *name = (type == TIMER_TYPE_WKSRC) ? "wksrc_timer" : "non_wksrc_timer";
    timer_ctrl_s *ctrl = NULL;
    timer_device_s *device = NULL;

    /* step1:申请硬件资源 */
    ret = timer_dev_alloc(type, &timer_id);
    if (ret) {
        timer_err("fail to malloc %s. ret=0x%x.\n", name, ret);
        return BSP_ERROR;
    }

    /* step2:初始化控制数据 */
    ctrl = (timer_ctrl_s *)(&g_timer_ctrl[type]);
    (void)memset_s(ctrl, sizeof(timer_ctrl_s), 0, sizeof(timer_ctrl_s));
    ctrl->timer_id = timer_id;
    INIT_LIST_HEAD(&(ctrl->list_head));
    spin_lock_init(&(ctrl->list_lock));
    osl_sem_init(SEM_EMPTY, &(ctrl->task_sem));

    /* step3:注册中断 */
    device = timer_get_device(timer_id);
    ret = request_irq(
        device->irq, (irq_handler_t)timer_routine, irq_flags, name, (void *)((uintptr_t)type));
    if (ret) {
        timer_err("fail to register irq for %s. timer_id=%d, irq=%d, ret=0x%x.\n", name, timer_id, device->irq, ret);
        return BSP_ERROR;
    }

    /* step4:创建任务 */
    if (timer_create_task(type, name)) {
        return BSP_ERROR;
    }

    /* step5:注册wake_lock */
    ctrl->wake_lock = wakeup_source_register(NULL, name);\
    if (ctrl->wake_lock == NULL) {
         timer_err("wakeup source err\n");
    }
    (void)memcpy_s(device->name, sizeof(device->name), name, strlen(name) + 1);

    return BSP_OK;
}

s32 timer_instances_init(void)
{
    if (timer_create_instance(TIMER_TYPE_WKSRC, IRQF_NO_SUSPEND)) {
        return BSP_ERROR;
    }

    if (timer_create_instance(TIMER_TYPE_N_WKSRC, 0)) {
        return BSP_ERROR;
    }

    return BSP_OK;
}

void timer_show_trail(void)
{
    u32 i;
    timer_err("  index     name        time_input(32k)   time_output(32k)\n");
    timer_err("------------------------------------------------------------\n");
    for (i = 0; i < TIMER_MAX_DBG_NUMBER; i++) {
        timer_err("  %-5d%-17s%-18d%-20d\n",
            i, g_timer_debug[i].name, g_timer_debug[i].tick_num_setting, g_timer_debug[i].tick_num_period);
    }
}

EXPORT_SYMBOL(mdrv_timer_add);
EXPORT_SYMBOL(mdrv_timer_delete);
EXPORT_SYMBOL(timer_show_trail);
