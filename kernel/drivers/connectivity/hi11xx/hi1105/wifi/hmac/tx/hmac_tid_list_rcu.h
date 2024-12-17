/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : Hmac tid队列操作模块(RCU)
 * 作    者 : wifi
 * 创建日期 : 2021年8月12日
 */

#ifndef HMAC_TID_LIST_RCU_H
#define HMAC_TID_LIST_RCU_H

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include "linux/rculist.h"
#include "hmac_tid.h"

typedef struct {
    struct list_head list;
    oal_spin_lock_stru lock;
    const hmac_tid_list_ops *ops;
} hmac_tid_rcu_list_stru;

static inline void hmac_tid_list_init_rcu(hmac_tid_rcu_list_stru *tid_list, const hmac_tid_list_ops *ops)
{
    INIT_LIST_HEAD_RCU(&tid_list->list);
    oal_spin_lock_init(&tid_list->lock);
    tid_list->ops = ops;
}

static inline uint8_t hmac_tid_entry_equal_rcu(hmac_tid_info_stru *tid_info_entry, void *tid_info)
{
    if (tid_info_entry->user_index == ((hmac_tid_info_stru *)tid_info)->user_index &&
        tid_info_entry->tid_no == ((hmac_tid_info_stru *)tid_info)->tid_no) {
        return OAL_RETURN;
    }

    return OAL_CONTINUE;
}

static inline uint8_t hmac_tid_list_for_each_rcu(hmac_tid_rcu_list_stru *tid_list, hmac_tid_list_func func, void *param)
{
    uint8_t ret;

    if (oal_unlikely(!tid_list->ops->for_each_rcu)) {
        return OAL_FALSE;
    }

    rcu_read_lock();
    ret = tid_list->ops->for_each_rcu(func, param);
    rcu_read_unlock();

    return ret;
}

static inline uint8_t hmac_tid_entry_exists_rcu(hmac_tid_rcu_list_stru *tid_list, hmac_tid_info_stru *tid_info)
{
    return hmac_tid_list_for_each_rcu(tid_list, hmac_tid_entry_equal_rcu, (void *)tid_info);
}

static inline uint32_t hmac_tid_list_enqueue_rcu_nolock(hmac_tid_rcu_list_stru *tid_list, hmac_tid_info_stru *tid_info)
{
    if (hmac_tid_entry_exists_rcu(tid_list, tid_info)) {
        return OAL_FAIL;
    }

    list_add_rcu((struct list_head *)tid_list->ops->entry_getter(tid_info), &tid_list->list);

    return OAL_SUCC;
}

static inline uint32_t hmac_tid_list_enqueue_rcu(hmac_tid_rcu_list_stru *tid_list, hmac_tid_info_stru *tid_info)
{
    uint32_t ret;

    if (oal_unlikely(!tid_list->ops->entry_getter)) {
        return OAL_FAIL;
    }

    oal_spin_lock_bh(&tid_list->lock);
    ret = hmac_tid_list_enqueue_rcu_nolock(tid_list, tid_info);
    oal_spin_unlock_bh(&tid_list->lock);

    return ret;
}

static inline uint32_t hmac_tid_list_delete_rcu_nolock(hmac_tid_rcu_list_stru *tid_list, hmac_tid_info_stru *tid_info)
{
    if (!hmac_tid_entry_exists_rcu(tid_list, tid_info)) {
        return OAL_FAIL;
    }

    list_del_rcu((struct list_head *)tid_list->ops->entry_getter(tid_info));

    return OAL_SUCC;
}

static inline uint32_t hmac_tid_list_delete_rcu(hmac_tid_rcu_list_stru *tid_list, hmac_tid_info_stru *tid_info)
{
    uint32_t ret;

    if (oal_unlikely(!tid_list->ops->entry_getter)) {
        return OAL_FAIL;
    }

    oal_spin_lock_bh(&tid_list->lock);
    ret = hmac_tid_list_delete_rcu_nolock(tid_list, tid_info);
    oal_spin_unlock_bh(&tid_list->lock);
    synchronize_rcu();

    return ret;
}

#endif
#endif
