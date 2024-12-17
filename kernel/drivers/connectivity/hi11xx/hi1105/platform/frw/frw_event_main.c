/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description: Implementation of external event management interfaces
 *              (for IPC interfaces and service interfaces)
 * Author: @CompanyNameTag
 */

/* 头文件包含 */
#include "frw_event_main.h"
#include "frw_task.h"
#if defined(_PRE_CONFIG_CONN_HISI_SYSFS_SUPPORT) && defined(_PRE_CONFIG_HISI_PANIC_DUMP_SUPPORT)
#include "oal_kernel_file.h"
#endif
#include "securec.h"
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include "plat_exception_rst.h"
#endif
#include "frw_timer.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_FRW_EVENT_MAIN_C

/*
 * 结构名  : frw_event_ipc_register_stru
 * 结构说明: IPC模块注册结构体
 */
struct _frw_event_ipc_register_stru_ {
    uint32_t (*p_frw_event_deploy_pipeline_func)(frw_event_mem_stru *pst_event_mem, uint8_t *puc_deploy_result);
    uint32_t (*p_frw_ipc_event_queue_full_func)(void);
    uint32_t (*p_frw_ipc_event_queue_empty_func)(void);
};

/* IPC注册管理实体 */
OAL_STATIC frw_event_ipc_register_stru g_ipc_register;

#ifdef _PRE_CONFIG_HISI_PANIC_DUMP_SUPPORT
#ifdef _PRE_FRW_EVENT_PROCESS_TRACE_DEBUG
OAL_STATIC int32_t frw_trace_print_event_item(frw_event_trace_item_stru *pst_event_trace,
                                              char *buf, int32_t buf_len)
{
    int32_t ret;
    uint64_t rem_nsec;
    uint64_t timestamp = pst_event_trace->timestamp;

    rem_nsec = do_div(timestamp, 1000000000);   /* 1000000000:由秒转化为纳秒 */
    do_div(rem_nsec, 1000);
    ret = snprintf_s(buf, buf_len, buf_len - 1, "%u,%u,%u,%u,%5lu.%06lu\n",
                     pst_event_trace->st_event_seg.uc_vap_id,
                     pst_event_trace->st_event_seg.en_pipeline,
                     pst_event_trace->st_event_seg.en_type,
                     pst_event_trace->st_event_seg.uc_sub_type,
                     timestamp,
                     rem_nsec);      /* 1000:由纳秒转化为微秒 */
    if (ret < 0) {
        oal_io_print("log str format err line[%d]\n", __LINE__);
    }
    return ret;
}
#endif

#define CHECK_ERROR_RETURN                                              \
    do {                                                                \
        if (ret < 0) {                                                  \
            oal_io_print("log str format err line[%d]\n", __LINE__);    \
            return count;                                               \
        }                                                               \
        count += ret;                                                   \
    } while (0)

OAL_STATIC int32_t frw_print_trace_stat_ctx(char *buf, int32_t buf_len, uint32_t ul_core_id, frw_subsys_context *ctx)
{
    int32_t ret;
    int32_t count = 0;
    uint32_t i, loop_cnt, loop_start_idx;
    if (ctx->event_manager[ul_core_id].pst_frw_trace->ul_over_flag == 1) {
        loop_cnt = CONFIG_FRW_MAX_TRACE_EVENT_NUMS;
        loop_start_idx = ctx->event_manager[ul_core_id].pst_frw_trace->ul_current_pos;
    } else {
        loop_cnt = ctx->event_manager[ul_core_id].pst_frw_trace->ul_current_pos;
        loop_start_idx = 0;
    }

    for (i = 0; i < loop_cnt; i++) {
        uint32_t idx = (i + loop_start_idx) % CONFIG_FRW_MAX_TRACE_EVENT_NUMS;
        ret = frw_trace_print_event_item(&ctx->event_manager[ul_core_id].pst_frw_trace->st_trace_item[idx],
            buf + count, buf_len - count);
        if (ret < 0) {
            oal_io_print("log str format err line[%d]\n", __LINE__);
            return count;
        }
        count += ret;
    }
    return count;
}

OAL_STATIC int32_t frw_print_panic_stat_ctx(void *data, char *buf, int32_t buf_len, frw_subsys_context *ctx)
{
    int32_t ret;
    int32_t count = 0;
    uint32_t ul_core_id;
#if defined(_PRE_FRW_TIMER_BIND_CPU) && defined(CONFIG_NR_CPUS)
    uint32_t cpu_id;
#endif

#ifdef _PRE_OAL_FEATURE_TASK_NEST_LOCK
    if (ctx->event_task_lock->claimer) {
        ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "frw task lock claimer:%s\n",
                         ctx->event_task_lock->claimer->comm);
        CHECK_ERROR_RETURN;
    }
#endif
#ifdef _PRE_FRW_EVENT_PROCESS_TRACE_DEBUG
    for (ul_core_id = 0; ul_core_id < WLAN_FRW_MAX_NUM_CORES; ul_core_id++) {
        ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "last pc:%s,line:%d\n",
                         ctx->event_manager[ul_core_id].pst_frw_trace->pst_func_name,
                         ctx->event_manager[ul_core_id].pst_frw_trace->line_num);
        CHECK_ERROR_RETURN;

#if (_PRE_FRW_FEATURE_PROCCESS_ENTITY_TYPE == _PRE_FRW_FEATURE_PROCCESS_ENTITY_THREAD)
        ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1,
                         "task thread total cnt:%u,event cnt:%u,empty max count:%u\n",
                         ctx->event_task[ul_core_id].ul_total_loop_cnt, ctx->event_task[ul_core_id].ul_total_event_cnt,
                         ctx->event_task[ul_core_id].ul_max_empty_count);
        CHECK_ERROR_RETURN;
#endif

        ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "frw event trace buff:\n");
        CHECK_ERROR_RETURN;

#if defined(_PRE_FRW_TIMER_BIND_CPU) && defined(CONFIG_NR_CPUS)
        for (cpu_id = 0; cpu_id < CONFIG_NR_CPUS; cpu_id++) {
            if (!ctx->frw_timer_cpu_count[cpu_id]) {
                continue;
            }

            ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "[cpu:%u]count:%u\n",
                             cpu_id, ctx->frw_timer_cpu_count[cpu_id]);
            CHECK_ERROR_RETURN;
        }
#endif
        ret = frw_print_trace_stat_ctx(buf + count, buf_len - count, ul_core_id, ctx);
        CHECK_ERROR_RETURN;
    }
#endif
    return count;
}

OAL_STATIC int32_t frw_print_panic_stat_wifi(void *data, char *buf, int32_t buf_len)
{
    return frw_print_panic_stat_ctx(data, buf, buf_len, WIFI_FRW_CONTEXT);
}

OAL_STATIC declare_wifi_panic_stru(frw_panic_stat, frw_print_panic_stat_wifi);
#endif

OAL_STATIC uint32_t frw_event_init_event_queue_ctx(frw_subsys_context *ctx)
{
    uint32_t ul_core_id;
    uint16_t us_qid;
    uint32_t ret;

    for (ul_core_id = 0; ul_core_id < WLAN_FRW_MAX_NUM_CORES; ul_core_id++) {
        /* 循环初始化事件队列 */
        for (us_qid = 0; us_qid < ctx->event_queue_cfg_table_size; us_qid++) {
            ret = frw_event_queue_init_ctx(&ctx->event_manager[ul_core_id].st_event_queue[us_qid],
                                           ctx->event_queue_cfg_table[us_qid].uc_weight,
                                           ctx->event_queue_cfg_table[us_qid].en_policy, FRW_EVENT_QUEUE_STATE_INACTIVE,
                                           ctx->event_queue_cfg_table[us_qid].uc_max_events, ctx);
            if (oal_unlikely(ret != OAL_SUCC)) {
                oam_warning_log1(0, OAM_SF_FRW,
                                 "{frw_event_init_event_queue, frw_event_queue_init return != OAL_SUCC!%d}", ret);
                return ret;
            }
        }
    }

    return OAL_SUCC;
}

OAL_STATIC void frw_event_destroy_event_queue_ctx(uint32_t ul_core_id, frw_subsys_context *ctx)
{
    uint16_t us_qid;

    /* 循环销毁事件队列 */
    for (us_qid = 0; us_qid < ctx->event_queue_cfg_table_size; us_qid++) {
        frw_event_queue_destroy_ctx(&ctx->event_manager[ul_core_id].st_event_queue[us_qid], ctx);
    }
}

/*
 * 函 数 名  : frw_event_init_sched
 * 功能描述  : 初始化调度器
 */
OAL_STATIC uint32_t frw_event_init_sched_ctx(frw_subsys_context *ctx)
{
    uint32_t ul_core_id;
    uint16_t us_qid;
    uint32_t ret;

    for (ul_core_id = 0; ul_core_id < WLAN_FRW_MAX_NUM_CORES; ul_core_id++) {
        /* 循环初始化调度器 */
        for (us_qid = 0; us_qid < FRW_SCHED_POLICY_BUTT; us_qid++) {
            ret = frw_event_sched_init(&ctx->event_manager[ul_core_id].st_sched_queue[us_qid]);
            if (oal_unlikely(ret != OAL_SUCC)) {
                oam_warning_log1(0, OAM_SF_FRW,
                                 "{frw_event_init_sched, frw_event_sched_init return != OAL_SUCC!%d}", ret);
                return ret;
            }
        }
    }

    return OAL_SUCC;
}

#ifdef _PRE_FRW_EVENT_PROCESS_TRACE_DEBUG
OAL_STATIC uint32_t frw_event_trace_init_ctx(frw_subsys_context *ctx)
{
    uint32_t ul_core_id;
    for (ul_core_id = 0; ul_core_id < WLAN_FRW_MAX_NUM_CORES; ul_core_id++) {
        ctx->event_manager[ul_core_id].pst_frw_trace =
            (frw_event_trace_stru *)vmalloc(sizeof(frw_event_trace_stru));
        if (ctx->event_manager[ul_core_id].pst_frw_trace == NULL) {
            oal_io_print("frw_event_init_sched coreid:%u, alloc frw event trace %u bytes failed! \n",
                         ul_core_id, (uint32_t)sizeof(frw_event_trace_stru));
            return OAL_ERR_CODE_PTR_NULL;
        }
        memset_s((void *)ctx->event_manager[ul_core_id].pst_frw_trace,
                 sizeof(frw_event_trace_stru), 0, sizeof(frw_event_trace_stru));
    }
    return OAL_SUCC;
}

OAL_STATIC void frw_event_trace_exit_ctx(frw_subsys_context *ctx)
{
    uint32_t ul_core_id;
    for (ul_core_id = 0; ul_core_id < WLAN_FRW_MAX_NUM_CORES; ul_core_id++) {
        if (ctx->event_manager[ul_core_id].pst_frw_trace != NULL) {
            vfree(ctx->event_manager[ul_core_id].pst_frw_trace);
            ctx->event_manager[ul_core_id].pst_frw_trace = NULL;
        }
    }
}
#endif

/*
 * 函 数 名  : frw_event_dispatch_event
 * 功能描述  : 事件分发接口(分发事件至核间通讯、事件队列、或者查表寻找相应事件处理函数)
 * 输入参数  : pst_event_mem: 指向事件内存块的指针
 */
uint32_t frw_event_dispatch_event_ctx(frw_event_mem_stru *pst_event_mem, frw_subsys_context *ctx)
{
#if (_PRE_MULTI_CORE_MODE_PIPELINE_AMP == _PRE_MULTI_CORE_MODE)
    uint8_t en_deploy;
    uint32_t ret;
#endif
    if (oal_unlikely(oal_any_null_ptr1(ctx))) {
        oam_error_log0(0, OAM_SF_FRW, "{frw_event_dispatch_event_ctx:: invalid point}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    if (oal_unlikely(pst_event_mem == NULL)) {
        oam_error_log0(0, OAM_SF_FRW, "{frw_event_dispatch_event: pst_event_mem is null ptr!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

#if (_PRE_MULTI_CORE_MODE_PIPELINE_AMP == _PRE_MULTI_CORE_MODE)
    /* 如果没有开启核间通信，则根据事件分段号处理事件(入队或者执行相应的处理函数) */
    if (st_ipc_register.p_frw_event_deploy_pipeline_func == NULL) {
        return frw_event_process_ctx(pst_event_mem, ctx);
    }

    ret = st_ipc_register.p_frw_event_deploy_pipeline_func(pst_event_mem, &en_deploy);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_FRW,
                         "{frw_event_dispatch_event, p_frw_event_deploy_pipeline_func return != OAL_SUCC!%d}", ret);
        return ret;
    }

    /* 如果为核间通信，则直接返回成功。否则，根据事件分段号处理事件 */
    if (en_deploy == FRW_EVENT_DEPLOY_IPC) {
        return OAL_SUCC;
    }
#endif

    return frw_event_process_ctx(pst_event_mem, ctx);
}

#if defined(_PRE_CONFIG_CONN_HISI_SYSFS_SUPPORT) && defined(_PRE_CONFIG_HISI_PANIC_DUMP_SUPPORT)
OAL_STATIC ssize_t frw_get_event_trace_ctx(struct kobject *dev, struct kobj_attribute *attr, char *buf,
                                           frw_subsys_context *ctx)
{
    int ret = 0;

    if (buf == NULL) {
        oal_io_print("buf is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (attr == NULL) {
        oal_io_print("attr is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (dev == NULL) {
        oal_io_print("dev is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    ret += frw_print_panic_stat_ctx(NULL, buf, PAGE_SIZE - ret, ctx);
    return ret;
}

OAL_STATIC ssize_t frw_get_event_trace_gt(struct kobject *dev, struct kobj_attribute *attr, char *buf)
{
    return frw_get_event_trace_ctx(dev, attr, buf, GT_FRW_CONTEXT);
}

OAL_STATIC ssize_t frw_get_event_trace_wifi(struct kobject *dev, struct kobj_attribute *attr, char *buf)
{
    return frw_get_event_trace_ctx(dev, attr, buf, WIFI_FRW_CONTEXT);
}

STATIC struct kobj_attribute g_dev_attr_event_trace_gt =
    __ATTR(event_trace, S_IRUGO, frw_get_event_trace_gt, NULL);
STATIC struct kobj_attribute g_dev_attr_event_trace_wifi =
    __ATTR(event_trace, S_IRUGO, frw_get_event_trace_wifi, NULL);

OAL_STATIC struct attribute *g_frw_sysfs_entries_gt[] = {
    &g_dev_attr_event_trace_gt.attr,
    NULL
};

OAL_STATIC struct attribute *g_frw_sysfs_entries_wifi[] = {
    &g_dev_attr_event_trace_wifi.attr,
    NULL
};

OAL_STATIC struct attribute_group g_frw_attribute_group_gt = {
    .name = "frw_gt",
    .attrs = g_frw_sysfs_entries_gt,
};

OAL_STATIC struct attribute_group g_frw_attribute_group_wifi = {
    .name = "frw_wifi",
    .attrs = g_frw_sysfs_entries_wifi,
};

OAL_STATIC uint32_t frw_sysfs_entry_init_ctx(frw_subsys_context *ctx)
{
    int32_t ret;
    oal_kobject *pst_root_object = NULL;
    struct attribute_group *group;
    pst_root_object = oal_get_sysfs_root_object();
    if (pst_root_object == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{frw_sysfs_entry_init::get sysfs root object failed!}");
        return OAL_EINVAL;
    }

    if (frw_context_is_gt(ctx)) {
        group = &g_frw_attribute_group_gt;
    } else {
        group = &g_frw_attribute_group_wifi;
    }

    ret = oal_debug_sysfs_create_group(pst_root_object, group);
    if (ret) {
        oam_error_log1(0, OAM_SF_ANY, "{frw_sysfs_entry_init::sysfs create group failed, ret = %d!}", ret);
        return OAL_EINVAL;
    }
    return OAL_SUCC;
}

OAL_STATIC void frw_sysfs_entry_exit_ctx(frw_subsys_context *ctx)
{
    oal_kobject *pst_root_object = NULL;
    struct attribute_group *group;

    if (frw_context_is_gt(ctx)) {
        group = &g_frw_attribute_group_gt;
    } else {
        group = &g_frw_attribute_group_wifi;
    }

    pst_root_object = oal_get_sysfs_root_object();
    if (pst_root_object != NULL) {
        oal_debug_sysfs_remove_group(pst_root_object, group);
    }
}
#endif

uint32_t frw_event_init_ctx(frw_subsys_context *ctx)
{
    uint32_t ret;

    memset_s(&g_ipc_register, sizeof(g_ipc_register), 0, sizeof(g_ipc_register));

#ifdef _PRE_OAL_FEATURE_TASK_NEST_LOCK
    oal_smp_task_lock_init(ctx->event_task_lock);
#endif

    /* 初始化事件队列 */
    ret = frw_event_init_event_queue_ctx(ctx);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_FRW, "{frw_event_init, frw_event_init_event_queue != OAL_SUCC!%d}", ret);
        return ret;
    }

    /* 初始化调度器 */
    ret = frw_event_init_sched_ctx(ctx);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_FRW, "frw_event_init, frw_event_init_sched != OAL_SUCC!%d", ret);
        return ret;
    }

#ifdef _PRE_FRW_EVENT_PROCESS_TRACE_DEBUG
    ret = frw_event_trace_init_ctx(ctx);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_error_log1(0, OAM_SF_FRW, "frw_event_init, frw_event_trace_init != OAL_SUCC!%d", ret);
        return ret;
    }
#endif

#ifdef _PRE_CONFIG_HISI_PANIC_DUMP_SUPPORT
    if (frw_context_is_wifi(ctx)) {
        hwifi_panic_log_register(&frw_panic_stat, NULL);
    }
#endif

    if (frw_context_is_gt(ctx)) {
        frw_task_event_handler_register_ctx(frw_event_process_all_event_gt, ctx);
        ctx->max_vap_cnt = GT_SERVICE_VAP_MAX_NUM_PER_DEVICE;
    } else {
        frw_task_event_handler_register_ctx(frw_event_process_all_event_wifi, ctx);
        ctx->max_vap_cnt = WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE;
    }

#if defined(_PRE_CONFIG_CONN_HISI_SYSFS_SUPPORT) && defined(_PRE_CONFIG_HISI_PANIC_DUMP_SUPPORT)
    ret = frw_sysfs_entry_init_ctx(ctx);
#endif

    return ret;
}

uint32_t frw_event_exit_ctx(frw_subsys_context *ctx)
{
    uint32_t ul_core_id;
#if defined(_PRE_CONFIG_CONN_HISI_SYSFS_SUPPORT) && defined(_PRE_CONFIG_HISI_PANIC_DUMP_SUPPORT)
    frw_sysfs_entry_exit_ctx(ctx);
#endif

#ifdef _PRE_FRW_EVENT_PROCESS_TRACE_DEBUG
    frw_event_trace_exit_ctx(ctx);
#endif

    for (ul_core_id = 0; ul_core_id < WLAN_FRW_MAX_NUM_CORES; ul_core_id++) {
        /* 销毁事件队列 */
        frw_event_destroy_event_queue_ctx(ul_core_id, ctx);
    }

    return OAL_SUCC;
}

void frw_event_sub_rx_adapt_table_init(frw_event_sub_table_item_stru *pst_sub_table, uint32_t ul_table_nums,
                                       frw_event_mem_stru *(*p_rx_adapt_func)(frw_event_mem_stru *))
{
    uint32_t i;
    frw_event_sub_table_item_stru *pst_curr_table = NULL;
    if (oal_unlikely(oal_any_null_ptr1(pst_sub_table))) {
        oam_error_log0(0, OAM_SF_FRW, "{frw_event_sub_rx_adapt_table_init:: invalid point}");
        return;
    }
    for (i = 0; i < ul_table_nums; i++) {
        pst_curr_table = pst_sub_table + i;
        pst_curr_table->p_rx_adapt_func = p_rx_adapt_func;
    }
}

/*
 * 函 数 名  : frw_event_queue_enqueue
 * 功能描述  : 将事件内存放入相应的事件队列
 * 输入参数  : pst_event_mem: 指向事件内存块的指针
 */
uint32_t frw_event_queue_enqueue(frw_event_queue_stru *pst_event_queue, frw_event_mem_stru *pst_event_mem)
{
    uint32_t ret;
    oal_uint irq_flag;

    if (oal_unlikely((pst_event_queue == NULL) || (pst_event_mem == NULL))) {
        oal_warn_on(1);
        return OAL_FAIL;
    }

    oal_spin_lock_irq_save(&pst_event_queue->st_lock, &irq_flag);
    ret = oal_queue_enqueue(&pst_event_queue->st_queue, (void *)pst_event_mem);
    oal_spin_unlock_irq_restore(&pst_event_queue->st_lock, &irq_flag);
    return ret;
}

/*
 * 函 数 名  : frw_event_queue_dequeue
 * 功能描述  : 事件内存出队
 * 输入参数  : pst_event_queue: 事件队列
 */
frw_event_mem_stru *frw_event_queue_dequeue(frw_event_queue_stru *pst_event_queue)
{
    frw_event_mem_stru *pst_event_mem = NULL;
    oal_uint irq_flag;

    if (oal_unlikely(pst_event_queue == NULL)) {
        oal_warn_on(1);
        return NULL;
    }

    oal_spin_lock_irq_save(&pst_event_queue->st_lock, &irq_flag);
    pst_event_mem = (frw_event_mem_stru *)oal_queue_dequeue(&pst_event_queue->st_queue);
    oal_spin_unlock_irq_restore(&pst_event_queue->st_lock, &irq_flag);
    return pst_event_mem;
}

/*
 * 函 数 名  : frw_event_post_event
 * 功能描述  : 将事件内存放入相应的事件队列
 * 输入参数  : pst_event_mem: 指向事件内存块的指针
 */
uint32_t frw_event_post_event_ctx(frw_event_mem_stru *pst_event_mem, uint32_t ul_core_id, frw_subsys_context *ctx)
{
    uint16_t us_qid;
    frw_event_mgmt_stru *pst_event_mgmt = NULL;
    frw_event_queue_stru *pst_event_queue = NULL;
    uint32_t ret;
    frw_event_hdr_stru *pst_event_hdr = NULL;
    frw_event_sched_queue_stru *pst_sched_queue = NULL;
    oal_mem_pool_context_stru *mem_ctx = frw_context_get_mem_ctx(ctx);
    if (oal_unlikely(oal_any_null_ptr3(pst_event_mem, ctx, mem_ctx))) {
        oam_error_log0(0, OAM_SF_FRW, "{frw_event_post_event_ctx:: invalid point}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取事件队列ID */
    ret = frw_event_to_qid(pst_event_mem, &us_qid);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_FRW, "{frw_event_post_event, frw_event_to_qid return != OAL_SUCC!%d}", ret);
        return ret;
    }

    if (oal_unlikely(ul_core_id >= WLAN_FRW_MAX_NUM_CORES)) {
        oam_error_log1(0, OAM_SF_FRW, "{frw_event_post_event, array overflow!%d}", ul_core_id);
        return OAL_ERR_CODE_ARRAY_OVERFLOW;
    }

    /* 根据核号 + 队列ID，找到相应的事件队列 */
    pst_event_mgmt = &ctx->event_manager[ul_core_id];

    pst_event_queue = &pst_event_mgmt->st_event_queue[us_qid];

    /* 检查policy */
    if (oal_unlikely(pst_event_queue->en_policy >= FRW_SCHED_POLICY_BUTT)) {
        oam_error_log1(0, OAM_SF_FRW, "{frw_event_post_event, array overflow!%d}", pst_event_queue->en_policy);
        return OAL_ERR_CODE_ARRAY_OVERFLOW;
    }

    /* 获取调度队列 */
    pst_sched_queue = &pst_event_mgmt->st_sched_queue[pst_event_queue->en_policy];

    /* 先取得引用，防止enqueue与取得引用之间被释放 */
    pst_event_mem->uc_user_cnt++;

    /* 事件入队 */
    ret = frw_event_queue_enqueue(pst_event_queue, pst_event_mem);
    if (oal_unlikely(ret != OAL_SUCC)) {
        pst_event_hdr = (frw_event_hdr_stru *)(frw_get_event_data(pst_event_mem));
        oam_warning_log4(0, OAM_SF_FRW,
                         "frw_event_post_event:: enqueue fail. core %d, type %d, sub type %d, pipeline %d ",
                         ul_core_id, pst_event_hdr->en_type, pst_event_hdr->uc_sub_type, pst_event_hdr->en_pipeline);

        oam_warning_log4(0, OAM_SF_FRW, "event info: type: %d, sub type: %d, pipeline: %d,max num:%d",
                         pst_event_hdr->en_type, pst_event_hdr->uc_sub_type, pst_event_hdr->en_pipeline,
                         pst_event_queue->st_queue.uc_max_elements);

        /* 释放事件内存引用 */
        frw_event_free_append_line(pst_event_mem, mem_ctx);

        return ret;
    }

    /* 根据所属调度策略，将事件队列加入可调度队列 */
    ret = frw_event_sched_activate_queue(pst_sched_queue, pst_event_queue);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_error_log1(0, OAM_SF_FRW,
                       "{frw_event_post_event, frw_event_sched_activate_queue return != OAL_SUCC! %d}", ret);
        return ret;
    }

    frw_task_sched_ctx(ul_core_id, ctx);

    return OAL_SUCC;
}

/*
 * 函 数 名  : frw_event_table_register_ctx
 * 功能描述  : 注册相应事件对应的事件处理函数
 * 输入参数  : en_type:       事件类型
             en_pipeline:   事件分段号
             pst_sub_table: 事件子表指针
 */
void frw_event_table_register_ctx(uint8_t en_type,
    frw_event_pipeline_enum en_pipeline,
    frw_event_sub_table_item_stru *pst_sub_table,
    frw_subsys_context *ctx)
{
    uint8_t index;

    if (oal_unlikely(oal_any_null_ptr2(pst_sub_table, ctx))) {
        oam_error_log0(0, OAM_SF_FRW, "{frw_event_table_register_ctx:: invalid point}");
        return;
    }

    /* 根据事件类型及分段号计算事件表索引 */
    index = (((uint8_t)en_type << 1) | ((uint8_t)en_pipeline & 0x01));

    if (oal_unlikely(index >= FRW_EVENT_TABLE_MAX_ITEMS)) {
        oam_error_log1(0, OAM_SF_FRW, "{frw_event_table_register_ctx, array overflow! %d}", index);
        return;
    }

    ctx->event_table[index].pst_sub_table = pst_sub_table;
}

/*
 * 函 数 名  : frw_event_deploy_register
 * 功能描述  : 供event deploy模块注册事件部署接口
 * 输入参数  : p_func: 事件部署接口
 */
void frw_event_deploy_register(f_frw_event_deploy p_func)
{
    if (oal_unlikely(p_func == NULL)) {
        oal_warn_on(1);
        return;
    }
    g_ipc_register.p_frw_event_deploy_pipeline_func = p_func;
}

/*
 * 函 数 名  : frw_event_ipc_event_queue_full_register
 * 功能描述  : 供IPC模块注册核间中断频度管理接口
 * 输入参数  : p_func: 核间中断频度管理接口
 */
void frw_event_ipc_event_queue_full_register(uint32_t (*p_func)(void))
{
    if (oal_unlikely(p_func == NULL)) {
        oal_warn_on(1);
        return;
    }
    g_ipc_register.p_frw_ipc_event_queue_full_func = p_func;
}

/*
 * 函 数 名  : frw_event_ipc_event_queue_empty_register
 * 功能描述  : 供IPC模块注册核间中断频度管理接口
 * 输入参数  : p_func: 核间中断频度管理接口
 */
void frw_event_ipc_event_queue_empty_register(uint32_t (*p_func)(void))
{
    if (oal_unlikely(p_func == NULL)) {
        oal_warn_on(1);
        return;
    }
    g_ipc_register.p_frw_ipc_event_queue_empty_func = p_func;
}

void frw_event_process_all_event_ctx(oal_uint ui_data, frw_subsys_context *ctx)
{
    uint32_t core_id;
    frw_event_mem_stru *pst_event_mem = NULL;
    frw_event_sched_queue_stru *pst_sched_queue = NULL;
    frw_event_hdr_stru *pst_event_hrd = NULL;
    uint32_t mac_process_event = FRW_PROCESS_MAX_EVENT;
    oal_mem_pool_context_stru *mem_ctx = frw_context_get_mem_ctx(ctx);
    if (oal_unlikely(oal_any_null_ptr2(mem_ctx, ctx))) {
        oam_error_log0(0, OAM_SF_FRW, "{frw_event_process_all_event_ctx:: invalid point}");
        return;
    }
    /* 获取核号 */
    core_id = oal_get_core_id();
    if (oal_unlikely(core_id >= WLAN_FRW_MAX_NUM_CORES)) {
        oam_error_log1(0, OAM_SF_FRW, "{frw_event_process_all_event, array overflow! %d}", core_id);
        return;
    }
    pst_sched_queue = ctx->event_manager[core_id].st_sched_queue;
    /* 调用事件调度模块，选择一个事件 */
    pst_event_mem = (frw_event_mem_stru *)frw_event_schedule(pst_sched_queue);
    while (pst_event_mem != NULL) {
        /* 获取事件头结构 */
        pst_event_hrd = (frw_event_hdr_stru *)frw_get_event_data(pst_event_mem);
#ifdef _PRE_FRW_EVENT_PROCESS_TRACE_DEBUG
        /* trace the event serial */
        frw_event_trace_ctx(pst_event_mem, core_id, ctx);
#endif
        /* 根据事件找到对应的事件处理函数 */
        frw_event_task_lock_ctx(ctx);
        frw_event_lookup_process_entry_ctx(pst_event_mem, pst_event_hrd, ctx);
        frw_event_task_unlock_ctx(ctx);
        /* 释放事件内存 */
        frw_event_free_append_line(pst_event_mem, mem_ctx);
#if (_PRE_FRW_FEATURE_PROCCESS_ENTITY_TYPE == _PRE_FRW_FEATURE_PROCCESS_ENTITY_THREAD)
        if (oal_likely(core_id < WLAN_FRW_MAX_NUM_CORES)) {
            ctx->event_task[core_id].ul_total_event_cnt++;
        }
#endif
#ifdef _PRE_FRW_EVENT_PROCESS_TRACE_DEBUG
        frw_event_last_pc_trace_ctx(__FUNCTION__, __LINE__, core_id, ctx);
#endif
        /* 在CPU能力较弱时，可能出现tasklet始终不退出问题 */
        if (--mac_process_event) {
            /* 调用事件调度模块，选择一个事件 */
            pst_event_mem = (frw_event_mem_stru *)frw_event_schedule(pst_sched_queue);
        } else {
            frw_task_sched_ctx(core_id, ctx);
            break;
        }
    }
#ifdef _PRE_FRW_EVENT_PROCESS_TRACE_DEBUG
    frw_event_last_pc_trace_ctx(__FUNCTION__, __LINE__, core_id, ctx);
#endif
}

void frw_event_process_all_event_gt(oal_uint ui_data)
{
    frw_event_process_all_event_ctx(ui_data, GT_FRW_CONTEXT);
}

void frw_event_process_all_event_wifi(oal_uint ui_data)
{
    frw_event_process_all_event_ctx(ui_data, WIFI_FRW_CONTEXT);
}

void frw_event_process_all_event_when_wlan_close_ctx(frw_subsys_context *ctx)
{
    uint32_t core_id;
    frw_event_mem_stru *event_mem = NULL;
    frw_event_sched_queue_stru *sched_queue = NULL;
    frw_event_hdr_stru *event_hrd = NULL;
    uint32_t mac_process_event = FRW_PROCESS_MAX_EVENT;
    oal_mem_pool_context_stru *mem_ctx = frw_context_get_mem_ctx(ctx);
    if (oal_unlikely(oal_any_null_ptr2(mem_ctx, ctx))) {
        oam_error_log0(0, OAM_SF_FRW, "{frw_event_process_all_event_when_wlan_close_ctx:: invalid point}");
        return;
    }
    /* 获取核号 */
    core_id = oal_get_core_id();
    if (oal_unlikely(core_id >= WLAN_FRW_MAX_NUM_CORES)) {
        oam_error_log1(0, OAM_SF_FRW, "{frw_event_process_all_event_when_wlan_close, array overflow! %d}", core_id);
        return;
    }
    frw_event_task_lock_ctx(ctx);
    sched_queue = ctx->event_manager[core_id].st_sched_queue;
    /* 调用事件调度模块，选择一个事件 */
    event_mem = (frw_event_mem_stru *)frw_event_schedule(sched_queue);
    while (event_mem != NULL) {
        /* 获取事件头结构 */
        event_hrd = (frw_event_hdr_stru *)frw_get_event_data(event_mem);
#ifdef _PRE_FRW_EVENT_PROCESS_TRACE_DEBUG
        /* trace the event serial */
        frw_event_trace_ctx(event_mem, core_id, ctx);
#endif
        /* 根据事件找到对应的事件处理函数 */
        frw_event_lookup_process_entry_ctx(event_mem, event_hrd, ctx);
        /* 释放事件内存 */
        frw_event_free_append_line(event_mem, mem_ctx);
#if (_PRE_FRW_FEATURE_PROCCESS_ENTITY_TYPE == _PRE_FRW_FEATURE_PROCCESS_ENTITY_THREAD)
        if (oal_likely(core_id < WLAN_FRW_MAX_NUM_CORES)) {
            ctx->event_task[core_id].ul_total_event_cnt++;
        }
#endif
#ifdef _PRE_FRW_EVENT_PROCESS_TRACE_DEBUG
        frw_event_last_pc_trace_ctx(__FUNCTION__, __LINE__, core_id, ctx);
#endif
        /* 在CPU能力较弱时，可能出现tasklet始终不退出问题 */
        if (--mac_process_event) {
            /* 调用事件调度模块，选择一个事件 */
            event_mem = (frw_event_mem_stru *)frw_event_schedule(sched_queue);
        } else {
            frw_task_sched_ctx(core_id, ctx);
            break;
        }
    }
    frw_event_task_unlock_ctx(ctx);
#ifdef _PRE_FRW_EVENT_PROCESS_TRACE_DEBUG
    frw_event_last_pc_trace_ctx(__FUNCTION__, __LINE__, core_id, ctx);
#endif
}

uint32_t frw_event_handle_event_queue_ctx(frw_event_queue_stru *pst_event_queue, oal_mem_pool_context_stru *mem_ctx,
    frw_subsys_context *ctx)
{
    uint32_t event_succ = 0;
    frw_event_mem_stru *pst_event_mem = NULL;
    frw_event_hdr_stru *pst_event_hrd = NULL;
    /* flush所有的event */
    while (pst_event_queue->st_queue.uc_element_cnt != 0) {
        pst_event_mem = (frw_event_mem_stru *)frw_event_queue_dequeue(pst_event_queue);
        if (pst_event_mem == NULL) {
            continue;
        }

        /* 获取事件头结构 */
        pst_event_hrd = (frw_event_hdr_stru *)frw_get_event_data(pst_event_mem);

        /* 根据事件找到对应的事件处理函数 */
        frw_event_lookup_process_entry_ctx(pst_event_mem, pst_event_hrd, ctx);

        /* 释放事件内存 */
        frw_event_free_append_line(pst_event_mem, mem_ctx);

        event_succ++;
    }
    return event_succ;
}


uint32_t frw_event_flush_event_queue_ctx(uint8_t uc_event_type, frw_subsys_context *ctx)
{
    uint32_t core_id;
    uint16_t us_qid;
    uint8_t uc_vap_id;
    frw_event_mgmt_stru *pst_event_mgmt = NULL;
    frw_event_queue_stru *pst_event_queue = NULL;
    uint32_t event_succ = 0;
    oal_mem_pool_context_stru *mem_ctx = frw_context_get_mem_ctx(ctx);
    if (oal_unlikely(oal_any_null_ptr2(mem_ctx, ctx))) {
        oam_error_log0(0, OAM_SF_FRW, "{frw_event_flush_event_queue_ctx:: invalid point}");
        return 0;
    }

    /* 遍历每个核的每个vap对应的事件队列 */
    for (core_id = 0; core_id < WLAN_FRW_MAX_NUM_CORES; core_id++) {
        for (uc_vap_id = 0; uc_vap_id < ctx->max_vap_cnt; uc_vap_id++) {
            us_qid = uc_vap_id * FRW_EVENT_TYPE_BUTT + uc_event_type;

            /* 根据核号 + 队列ID，找到相应的事件队列 */
            pst_event_mgmt = &ctx->event_manager[core_id];
            pst_event_queue = &pst_event_mgmt->st_event_queue[us_qid];

            event_succ += frw_event_handle_event_queue_ctx(pst_event_queue, mem_ctx, ctx);

            /* 如果事件队列变空，需要将其从调度队列上删除，并将事件队列状态置为不活跃(不可被调度) */
            if (pst_event_queue->st_queue.uc_element_cnt == 0) {
                frw_event_sched_deactivate_queue(
                    &ctx->event_manager[core_id].st_sched_queue[pst_event_queue->en_policy], pst_event_queue);
            }
        }
    }

    return event_succ;
}

/*
 * 函 数 名  : frw_event_dump_event
 * 功能描述  : 打印事件
 * 输入参数  : puc_event: 事件结构体首地址
 */
void frw_event_dump_event(uint8_t *puc_event)
{
    frw_event_stru *pst_event = (frw_event_stru *)puc_event;
    frw_event_hdr_stru *pst_event_hdr;
    uint8_t *puc_payload;
    uint32_t event_length;
    uint32_t loop;
    if (oal_unlikely(oal_any_null_ptr1(puc_event))) {
        oam_error_log0(0, OAM_SF_FRW, "{frw_event_dump_event:: invalid point}");
        return;
    }
    pst_event_hdr = &pst_event->st_event_hdr;
    puc_payload = pst_event->auc_event_data;
    event_length = pst_event_hdr->us_length - (uint16_t)FRW_EVENT_HDR_LEN;

    oal_io_print("==================event==================\n");
    oal_io_print("type     : [%02X]\n", pst_event_hdr->en_type);
    oal_io_print("sub type : [%02X]\n", pst_event_hdr->uc_sub_type);
    oal_io_print("length   : [%X]\n", pst_event_hdr->us_length);
    oal_io_print("pipeline : [%02X]\n", pst_event_hdr->en_pipeline);
    oal_io_print("chip id  : [%02X]\n", pst_event_hdr->uc_chip_id);
    oal_io_print("device id: [%02X]\n", pst_event_hdr->uc_device_id);
    oal_io_print("vap id   : [%02X]\n", pst_event_hdr->uc_vap_id);

    oal_io_print("payload: \n");

    for (loop = 0; loop < event_length; loop += (uint32_t)sizeof(uint32_t)) {
        oal_io_print("%02X %02X %02X %02X\n", puc_payload[loop], puc_payload[loop + 1],
                     puc_payload[loop + 2], puc_payload[loop + 3]);   /* 1、2、3: 以字节为单位打印事件内容 */
    }
}

/*
 * 函 数 名  : frw_event_get_info_from_event_queue
 * 功能描述  : 从事件队列中获取每一个事件的事件头信息
 * 输入参数  : pst_event_queue: 事件队列
 */
OAL_STATIC void frw_event_get_info_from_event_queue(frw_event_queue_stru *pst_event_queue)
{
    frw_event_stru *pst_event = NULL;
    oal_queue_stru *pst_queue = NULL;
    frw_event_mem_stru *pst_event_mem = NULL;
    uint8_t uc_loop;

    pst_queue = &pst_event_queue->st_queue;

    for (uc_loop = 0; uc_loop < pst_queue->uc_element_cnt; uc_loop++) {
        pst_event_mem = (frw_event_mem_stru *)pst_queue->pul_buf[uc_loop];
        pst_event = frw_get_event_stru(pst_event_mem);

        oal_io_print("frw event info:vap %d,type = %d,subtype=%d,pipe=%d, user_cnt: %u, pool_id: %u, \
                     subpool_id: %u, len: %u.\n",
                     pst_event->st_event_hdr.uc_vap_id,
                     pst_event->st_event_hdr.en_type,
                     pst_event->st_event_hdr.uc_sub_type,
                     pst_event->st_event_hdr.en_pipeline,
                     pst_event_mem->uc_user_cnt,
                     pst_event_mem->en_pool_id,
                     pst_event_mem->uc_subpool_id,
                     pst_event_mem->us_len);
    }
}

/*
 * 函 数 名  : frw_event_get_info_from_sched_queue
 * 功能描述  : 从调度队列中获取每一个事件的事件头信息
 * 输入参数  : pst_event_queue: 事件队列
 */
OAL_STATIC void frw_event_get_info_from_sched_queue(frw_event_sched_queue_stru *pst_sched_queue)
{
    oal_dlist_head_stru *pst_dlist = NULL;
    frw_event_queue_stru *pst_event_queue = NULL;
    /* 获取调度队列中的每一个事件队列 */
    oal_dlist_search_for_each(pst_dlist, &pst_sched_queue->st_head)
    {
        pst_event_queue = oal_dlist_get_entry(pst_dlist, frw_event_queue_stru, st_list);

        /* 获取队列中每一个事件的事件头信息 */
        oal_spin_lock(&pst_event_queue->st_lock);
        frw_event_get_info_from_event_queue(pst_event_queue);
        oal_spin_unlock(&pst_event_queue->st_lock);
    }
    return;
}

/*
 * 函 数 名  : frw_event_queue_info
 * 功能描述  : 将事件队列中的事件个数以及每个事件的事件头信息上报.
 *             从调度队列找到每一个存在事件的事件队列，然后获取事件个数并得到每一个
 *             事件的事件头信息
 */
uint32_t frw_event_queue_info_ctx(frw_subsys_context *ctx)
{
    uint32_t core_id;
    uint16_t us_qid;
    frw_event_sched_queue_stru *pst_sched_queue = NULL;
    frw_event_queue_stru *pst_event_queue = NULL;
    frw_event_mgmt_stru *pst_event_mgmt = NULL;
    oal_uint irq_flag;
    if (oal_unlikely(oal_any_null_ptr1(ctx))) {
        oam_error_log0(0, OAM_SF_FRW, "{frw_event_queue_info_ctx:: invalid point}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取核号 */
    core_id = oal_get_core_id();
    oal_io_print("frw_event_queue_info get core id is %d.\n", core_id);

    for (core_id = 0; core_id < WLAN_FRW_MAX_NUM_CORES; core_id++) {
        oal_io_print("-------------frw_event_queue_info core id is %d--------------.\n", core_id);
        for (us_qid = 0; us_qid < ctx->event_queue_cfg_table_size; us_qid++) {
            pst_event_queue = &ctx->event_manager[core_id].st_event_queue[us_qid];
            oal_spin_lock_irq_save(&pst_event_queue->st_lock, &irq_flag);

            if (pst_event_queue->st_queue.uc_element_cnt != 0) {
                oal_io_print("qid %d,state %d, event num %d, max event num %d, weigt_cnt %d,head idx %d,tail idx %d, \
                             prev=0x%p, next=0x%p\n",
                             us_qid, pst_event_queue->en_state, pst_event_queue->st_queue.uc_element_cnt,
                             pst_event_queue->st_queue.uc_max_elements,
                             pst_event_queue->uc_weight, pst_event_queue->st_queue.uc_head_index,
                             pst_event_queue->st_queue.uc_tail_index,
                             pst_event_queue->st_list.pst_prev, pst_event_queue->st_list.pst_next);
                frw_event_get_info_from_event_queue(pst_event_queue);
            }
            oal_spin_unlock_irq_restore(&pst_event_queue->st_lock, &irq_flag);
        }
        /* 根据核号，找到相应的事件管理结构体 */
        pst_event_mgmt = &ctx->event_manager[core_id];

        /* 遍历获取调度队列 */
        for (us_qid = 0; us_qid < FRW_SCHED_POLICY_BUTT; us_qid++) {
            /* 获取事件管理结构体中的调度队列 */
            pst_sched_queue = &pst_event_mgmt->st_sched_queue[us_qid];
            oal_spin_lock_irq_save(&pst_sched_queue->st_lock, &irq_flag);
            /* 获取调度队列中每个事件队列的每个事件的信息 */
            if (!oal_dlist_is_empty(&pst_sched_queue->st_head)) {
                frw_event_get_info_from_sched_queue(pst_sched_queue);
            } else {
                oal_io_print("Schedule queue %d empty\n", us_qid);
            }
            oal_spin_unlock_irq_restore(&pst_sched_queue->st_lock, &irq_flag);
        }
    }

    return OAL_SUCC;
}

/*
 * 函 数 名  : frw_event_vap_pause_event
 * 功能描述  : 设置特定VAP的所有事件队列的VAP状态为暂停，停止调度，允许继续入队
 */
void frw_event_vap_pause_event_ctx(uint8_t uc_vap_id, frw_subsys_context *ctx)
{
    uint32_t core_id;
    uint16_t us_qid;
    frw_event_mgmt_stru *pst_event_mgmt = NULL;
    frw_event_queue_stru *pst_event_queue = NULL;
    frw_event_sched_queue_stru *pst_sched_queue = NULL;
    if (oal_unlikely(oal_any_null_ptr1(ctx))) {
        oam_error_log0(0, OAM_SF_FRW, "{frw_event_vap_pause_event_ctx:: invalid point}");
        return;
    }
    core_id = oal_get_core_id();
    if (oal_unlikely(core_id >= WLAN_FRW_MAX_NUM_CORES)) {
        oam_error_log1(0, OAM_SF_FRW, "{frw_event_process_all_event, array overflow!%d}", core_id);
        return;
    }

    /* 根据核号，找到相应的事件管理 */
    pst_event_mgmt = &ctx->event_manager[core_id];

    /* 根据队列ID，找到相应的VAP的第一个事件队列 */
    pst_event_queue = &pst_event_mgmt->st_event_queue[uc_vap_id * FRW_EVENT_TYPE_BUTT];

    /* 如果事件队列已经被pause的话，直接返回，不然循环中调度队列总权重会重复减去事件队列的权重 */
    if (pst_event_queue->en_vap_state == FRW_VAP_STATE_PAUSE) {
        return;
    }

    for (us_qid = 0; us_qid < FRW_EVENT_TYPE_BUTT; us_qid++) {
        /* 根据队列ID，找到相应的事件队列 */
        pst_event_queue = &pst_event_mgmt->st_event_queue[uc_vap_id * FRW_EVENT_TYPE_BUTT + us_qid];
        pst_sched_queue = &ctx->event_manager[core_id].st_sched_queue[pst_event_queue->en_policy];

        frw_event_sched_pause_queue(pst_sched_queue, pst_event_queue);
    }
}

/*
 * 函 数 名  : frw_event_vap_resume_event
 * 功能描述  : 设置特定VAP的所有事件队列的VAP状态为恢复，可调度
 */
void frw_event_vap_resume_event_ctx(uint8_t uc_vap_id, frw_subsys_context *ctx)
{
    uint32_t core_id;
    uint16_t us_qid;
    frw_event_mgmt_stru *pst_event_mgmt = NULL;
    frw_event_queue_stru *pst_event_queue = NULL;
    frw_event_sched_queue_stru *pst_sched_queue = NULL;
    if (oal_unlikely(oal_any_null_ptr1(ctx))) {
        oam_error_log0(0, OAM_SF_FRW, "{frw_event_vap_resume_event_ctx:: invalid point}");
        return;
    }
    core_id = oal_get_core_id();
    if (oal_unlikely(core_id >= WLAN_FRW_MAX_NUM_CORES)) {
        oam_error_log1(0, OAM_SF_FRW, "{frw_event_process_all_event, array overflow!%d}", core_id);
        return;
    }

    /* 根据核号，找到相应的事件管理 */
    pst_event_mgmt = &ctx->event_manager[core_id];

    /* 根据队列ID，找到相应的VAP的第一个事件队列 */
    pst_event_queue = &pst_event_mgmt->st_event_queue[uc_vap_id * FRW_EVENT_TYPE_BUTT];

    /* 如果事件队列已经被resume的话，直接返回，不然循环中调度队列总权重会重复减去事件队列的权重 */
    if (pst_event_queue->en_vap_state == FRW_VAP_STATE_RESUME) {
        return;
    }

    for (us_qid = 0; us_qid < FRW_EVENT_TYPE_BUTT; us_qid++) {
        /* 根据队列ID，找到相应的事件队列 */
        pst_event_queue = &pst_event_mgmt->st_event_queue[uc_vap_id * FRW_EVENT_TYPE_BUTT + us_qid];
        pst_sched_queue = &ctx->event_manager[core_id].st_sched_queue[pst_event_queue->en_policy];

        frw_event_sched_resume_queue(pst_sched_queue, pst_event_queue);
    }

    /* 唤醒线程 */
    frw_task_sched_ctx(core_id, ctx);
}

/*
 * 函 数 名  : frw_event_vap_flush_event
 * 功能描述  : 冲刷指定VAP、指定事件类型的所有事件，同时可以指定是丢弃这些事件还是全部处理
 * 输入参数  : uc_vap_id:     VAP ID值
 *             en_event_type: 事件类型
 *             en_drop:       事件丢弃(1)或者处理(0)
 */
uint32_t frw_event_vap_flush_event_ctx(uint8_t uc_vap_id,
    uint8_t en_event_type,
    oal_bool_enum_uint8 en_drop,
    frw_subsys_context *ctx)
{
    uint32_t core_id;
    uint16_t us_qid;
    frw_event_mgmt_stru *pst_event_mgmt = NULL;
    frw_event_queue_stru *pst_event_queue = NULL;
    frw_event_mem_stru *pst_event_mem = NULL;
    frw_event_hdr_stru *pst_event_hrd = NULL;
    oal_mem_pool_context_stru *mem_ctx = frw_context_get_mem_ctx(ctx);
    if (oal_unlikely(oal_any_null_ptr2(mem_ctx, ctx))) {
        oam_error_log0(0, OAM_SF_FRW, "{frw_event_vap_flush_event_ctx:: invalid point}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    /* 获取核号 */
    core_id = oal_get_core_id();
    if (oal_unlikely(core_id >= WLAN_FRW_MAX_NUM_CORES)) {
        oam_error_log1(0, OAM_SF_FRW, "{frw_event_vap_flush_event, array overflow!%d}", core_id);
        return OAL_ERR_CODE_ARRAY_OVERFLOW;
    }

    if (en_event_type == FRW_EVENT_TYPE_WLAN_TX_COMP) {
        uc_vap_id = 0;
    }

    us_qid = uc_vap_id * FRW_EVENT_TYPE_BUTT + en_event_type;

    /* 根据核号 + 队列ID，找到相应的事件队列 */
    pst_event_mgmt = &ctx->event_manager[core_id];
    pst_event_queue = &pst_event_mgmt->st_event_queue[us_qid];

    /* 如果事件队列本身为空，没有事件，不在调度队列，返回错误 */
    if (pst_event_queue->st_queue.uc_element_cnt == 0) {
        return OAL_FAIL;
    }

    /* flush所有的event */
    while (pst_event_queue->st_queue.uc_element_cnt != 0) {
        pst_event_mem = (frw_event_mem_stru *)frw_event_queue_dequeue(pst_event_queue);
        if (pst_event_mem == NULL) {
            return OAL_ERR_CODE_PTR_NULL;
        }

        /* 处理事件，否则直接释放事件内存而丢弃事件 */
        if (en_drop == 0) {
            /* 获取事件头结构 */
            pst_event_hrd = (frw_event_hdr_stru *)frw_get_event_data(pst_event_mem);

            /* 根据事件找到对应的事件处理函数 */
            frw_event_lookup_process_entry_ctx(pst_event_mem, pst_event_hrd, ctx);
        }

        /* 释放事件内存 */
        frw_event_free_append_line(pst_event_mem, mem_ctx);
    }

    /* 若事件队列已经变空，需要将其从调度队列上删除，并将事件队列状态置为不活跃(不可被调度) */
    if (pst_event_queue->st_queue.uc_element_cnt == 0) {
        frw_event_sched_deactivate_queue(
            &ctx->event_manager[core_id].st_sched_queue[pst_event_queue->en_policy], pst_event_queue);
    } else {
        oam_error_log1(uc_vap_id, OAM_SF_FRW, "{flush vap event failed, left!=0: type=%d}", en_event_type);
    }

    return OAL_SUCC;
}

/*
 * 函 数 名  : frw_event_get_sched_queue
 * 功能描述  : 用于VAP暂停、恢复和冲刷函数进行IT测试，获取frw_event_main.c中定义的全局事件的调度队列
 * 输入参数  : ul_core_id: 核号
 *             en_policy:  调度策略
 * 返 回 值  : 调度队列指针
 */
frw_event_sched_queue_stru *frw_event_get_sched_queue_ctx(uint32_t core_id, uint8_t en_policy, frw_subsys_context *ctx)
{
    if (oal_unlikely((core_id >= WLAN_FRW_MAX_NUM_CORES) || (en_policy >= FRW_SCHED_POLICY_BUTT))) {
        return NULL;
    }
    if (oal_unlikely(oal_any_null_ptr1(ctx))) {
        oam_error_log0(0, OAM_SF_FRW, "{frw_event_get_sched_queue_ctx:: invalid point}");
        return NULL;
    }
    return &(ctx->event_manager[core_id].st_sched_queue[en_policy]);
}

/*
 * 函 数 名  : frw_is_event_queue_empty
 * 功能描述  : 判断所有VAP对应的事件队列是否为空
 */
oal_bool_enum_uint8 frw_is_event_queue_empty_ctx(uint8_t uc_event_type, frw_subsys_context *ctx)
{
    uint32_t core_id;
    uint8_t uc_vap_id;
    uint16_t us_qid;
    frw_event_mgmt_stru *pst_event_mgmt = NULL;
    frw_event_queue_stru *pst_event_queue = NULL;
    if (oal_unlikely(oal_any_null_ptr1(ctx))) {
        oam_error_log0(0, OAM_SF_FRW, "{frw_is_event_queue_empty_ctx:: invalid point}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    /* 获取核号 */
    core_id = oal_get_core_id();
    if (oal_unlikely(core_id >= WLAN_FRW_MAX_NUM_CORES)) {
        oam_error_log1(0, OAM_SF_FRW, "{frw_event_post_event, core id = %d overflow!}", core_id);

        return OAL_ERR_CODE_ARRAY_OVERFLOW;
    }

    pst_event_mgmt = &ctx->event_manager[core_id];

    /* 遍历该核上每个VAP对应的事件队列， */
    for (uc_vap_id = 0; uc_vap_id < ctx->max_vap_cnt; uc_vap_id++) {
        us_qid = uc_vap_id * FRW_EVENT_TYPE_BUTT + uc_event_type;

        /* 根据核号 + 队列ID，找到相应的事件队列 */
        pst_event_queue = &pst_event_mgmt->st_event_queue[us_qid];

        if (pst_event_queue->st_queue.uc_element_cnt != 0) {
            return OAL_FALSE;
        }
    }

    return OAL_TRUE;
}

/*
 * 函 数 名  : frw_is_vap_event_queue_empty
 * 功能描述  : 根据核id和事件类型，判断vap事件队列是否空
 * 输入参数  : ul_core_id: 核id; event_type:  事件ID;
 */
oal_bool_enum_uint8 frw_is_vap_event_queue_empty_ctx(uint32_t core_id, uint8_t uc_vap_id, uint8_t event_type,
                                                     frw_subsys_context *ctx)
{
    frw_event_mgmt_stru *pst_event_mgmt;
    frw_event_queue_stru *pst_event_queue;
    uint16_t us_qid;

#if (_PRE_OS_VERSION_WIN32_RAW == _PRE_OS_VERSION)
    us_qid = (uint16_t)event_type;
#else
    us_qid = (uint16_t)(uc_vap_id * FRW_EVENT_TYPE_BUTT + event_type);
#endif

    /* 根据核号 + 队列ID，找到相应的事件队列 */
    pst_event_mgmt = &ctx->event_manager[core_id];

    pst_event_queue = &pst_event_mgmt->st_event_queue[us_qid];

    if (pst_event_queue->st_queue.uc_element_cnt != 0) {
        return OAL_FALSE;
    }

    return OAL_TRUE;
}

/*
 * 函 数 名  : frw_task_thread_condition_check
 * 功能描述  : 判断是否有事件需要调度
 */
uint8_t frw_task_thread_condition_check_ctx(uint32_t core_id, frw_subsys_context *ctx)
{
    /*
     * 返回OAL_TRUE
     *  1.调度队列非空
     *  2.调度队列里有非pause的队列
     */
    uint8_t sched_policy;
    oal_uint irq_flag = 0;
    oal_dlist_head_stru *pst_list = NULL;
    frw_event_sched_queue_stru *pst_sched_queue = NULL;
    frw_event_queue_stru *pst_event_queue = NULL;

    pst_sched_queue = ctx->event_manager[core_id].st_sched_queue;

    for (sched_policy = 0; sched_policy < FRW_SCHED_POLICY_BUTT; sched_policy++) {
        oal_spin_lock_irq_save(&pst_sched_queue[sched_policy].st_lock, &irq_flag);
        /* 遍历整个调度链表 */
        oal_dlist_search_for_each(pst_list, &pst_sched_queue[sched_policy].st_head)
        {
            pst_event_queue = oal_dlist_get_entry(pst_list, frw_event_queue_stru, st_list);
            if (pst_event_queue->st_queue.uc_element_cnt == 0) {
                continue;
            }

            /* 如果事件队列的vap_state为暂停，则跳过，继续挑选下一个事件队列 */
            if (pst_event_queue->en_vap_state == FRW_VAP_STATE_PAUSE) {
                continue;
            }
            /* 找到事件队列非空 */
            oal_spin_unlock_irq_restore(&pst_sched_queue[sched_policy].st_lock, &irq_flag);
            return OAL_TRUE;
        }
        oal_spin_unlock_irq_restore(&pst_sched_queue[sched_policy].st_lock, &irq_flag);
    }
    /* 空返回OAL_FALSE */
    return OAL_FALSE;
}

oal_module_symbol(frw_event_dispatch_event_ctx);
oal_module_symbol(frw_event_post_event_ctx);
oal_module_symbol(frw_event_table_register_ctx);
oal_module_symbol(frw_event_dump_event);
oal_module_symbol(frw_event_process_all_event_ctx);
oal_module_symbol(frw_event_process_all_event_when_wlan_close_ctx);
oal_module_symbol(frw_event_flush_event_queue_ctx);
oal_module_symbol(frw_event_queue_info_ctx);
oal_module_symbol(frw_event_vap_pause_event_ctx);
oal_module_symbol(frw_event_vap_resume_event_ctx);
oal_module_symbol(frw_event_vap_flush_event_ctx);
oal_module_symbol(frw_event_get_sched_queue_ctx);

oal_module_symbol(frw_is_event_queue_empty_ctx);
oal_module_symbol(frw_event_sub_rx_adapt_table_init);
