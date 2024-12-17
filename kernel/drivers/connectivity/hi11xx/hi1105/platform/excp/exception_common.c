/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description: Common Exception Recovery Function
 * Author: @CompanyNameTag
 */

/* 头文件包含 */
#include "plat_debug.h"
#include "plat_exception_rst.h"
#include "exception_common.h"

int32_t excp_memdump_queue(uint8_t *buf_ptr, uint16_t count, memdump_info_t *memdump_t)
{
    struct sk_buff *skb = NULL;

    ps_print_dbg("[send] len:%d\n", count);
    if (!memdump_t->is_working) {
        ps_print_err("excp_memdump_queue not allow\n");
        return -EINVAL;
    }
    if (buf_ptr == NULL) {
        ps_print_err("buf_ptr is NULL\n");
        return -EINVAL;
    }
    if (in_atomic() || in_softirq() || in_interrupt() || irqs_disabled()) {
        skb = alloc_skb(count, GFP_ATOMIC);
    } else {
        skb = alloc_skb(count, GFP_KERNEL);
    }

    if (skb == NULL) {
        ps_print_err("can't allocate mem for new debug skb, len=%d\n", count);
        return -EINVAL;
    }

    if (memcpy_s(skb_tail_pointer(skb), count, buf_ptr, count) != EOK) {
        ps_print_err("memcpy_s failed\n");
    }
    skb_put(skb, count);
    skb_queue_tail(&memdump_t->quenue, skb);
    ps_print_dbg("[excp_memdump_queue]qlen:%d,count:%d\n", memdump_t->quenue.qlen, count);
    return 0;
}

OAL_STATIC void excp_memdump_quenue_clear(memdump_info_t *memdump_t)
{
    struct sk_buff *skb = NULL;
    while ((skb = skb_dequeue(&memdump_t->quenue)) != NULL) {
        kfree_skb(skb);
    }
}

/*
 * Prototype    : plat_excp_send_rotate_cmd_2_app
 * Description  : driver send rotate cmd to app for rotate file
 */
OAL_STATIC void plat_excp_send_rotate_cmd_2_app(uint32_t which_dump, memdump_info_t *memdump_info)
{
    struct sk_buff *skb = NULL;

    if (which_dump >= CMD_DUMP_BUFF) {
        ps_print_warning("which dump:%d error\n", which_dump);
        return;
    }
    if (skb_queue_len(&memdump_info->dump_type_queue) > MEMDUMP_ROTATE_QUEUE_MAX_LEN) {
        ps_print_warning("too many dump type in queue,dispose type:%d", which_dump);
        return;
    }

    skb = alloc_skb(sizeof(which_dump), (oal_in_interrupt() || oal_in_atomic()) ?
                    GFP_ATOMIC : GFP_KERNEL);
    if (skb == NULL) {
        ps_print_err("alloc errno skbuff failed! len=%d, errno=%x\n", (int32_t)sizeof(which_dump), which_dump);
        return;
    }
    skb_put(skb, sizeof(which_dump));
    *(uint32_t *)skb->data = which_dump;
    skb_queue_tail(&memdump_info->dump_type_queue, skb);
    ps_print_info("save rotate cmd [%d] in queue\n", which_dump);

    oal_wait_queue_wake_up_interrupt(&memdump_info->dump_type_wait);
}


int32_t notice_hal_memdump(memdump_info_t *memdump_t, uint32_t which_dump)
{
    PS_PRINT_FUNCTION_NAME;
    if (memdump_t->is_working) {
        ps_print_err("is doing memdump\n");
        return -1;
    }
    excp_memdump_quenue_clear(memdump_t);
    plat_excp_send_rotate_cmd_2_app(which_dump, memdump_t);
    memdump_t->is_working = 1;
    return 0;
}

int32_t is_subsystem_rst_enable(void)
{
#ifdef PLATFORM_DEBUG_ENABLE
    struct st_exception_info *pst_exception_data = get_exception_info_reference();
    if (pst_exception_data == NULL) {
        ps_print_err("get exception info reference is error\n");
        return DFR_TEST_DISABLE;
    }
    ps_print_info("#########is_subsystem_rst_enable:%d\n", pst_exception_data->subsystem_rst_en);
    return pst_exception_data->subsystem_rst_en;
#else
    return DFR_TEST_DISABLE;
#endif
}
