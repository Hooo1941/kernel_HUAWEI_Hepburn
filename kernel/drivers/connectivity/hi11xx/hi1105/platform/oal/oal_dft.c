/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description:dft function module
 * Author: @CompanyNameTag
 */

#define HISI_LOG_TAG "[DFT]"

#include "oal_dft.h"
#ifdef _PRE_CONFIG_CONN_HISI_SYSFS_SUPPORT
#include "oal_kernel_file.h"
#endif
#include "securec.h"

char *g_mpxx_loglevel_format[] = {
    KERN_ERR "[MPXX]"
    "[E]",
    KERN_WARNING "[MPXX]"
    "[W]",
    KERN_INFO "[MPXX]"
    "[I]",
    KERN_INFO "[MPXX]"
    "[D]",
    KERN_INFO "[MPXX]"
    "[V]"
};

int32_t g_mpxx_loglevel = MPXX_LOG_INFO;
oal_debug_module_param(g_mpxx_loglevel, int, S_IRUGO | S_IWUSR);

int32_t g_chip_err_block = 0; /* default 0 */
oal_debug_module_param(g_chip_err_block, int, S_IRUGO | S_IWUSR);
oal_module_symbol(g_chip_err_block);

int g_print_limit_enable = 1;
oal_debug_module_param(g_print_limit_enable, int, S_IRUGO | S_IWUSR);
oal_module_symbol(g_print_limit_enable);

#ifdef _PRE_OAL_FEATURE_KEY_PROCESS_TRACE
oal_define_spinlock(g_dft_head_lock);
oal_list_create_head(g_dft_head);
oal_module_symbol(g_dft_head_lock);
oal_module_symbol(g_dft_head);

#ifdef _PRE_CONFIG_CONN_HISI_SYSFS_SUPPORT
#define KERN_PRINT BIT0
#define BUFF_PRINT BIT1

OAL_STATIC uint32_t g_dft_print_mask = (KERN_PRINT | BUFF_PRINT);
oal_debug_module_param(g_dft_print_mask, uint, S_IRUGO | S_IWUSR);
OAL_STATIC void oal_dft_timestamp_init(struct rtc_time *first_tm, oal_timespec_stru *first_tv,
                                       struct rtc_time *last_tm, oal_timespec_stru *last_tv,
                                       oal_dft_trace_item *pst_dft_item, int32_t *last_time_flag)
{
    memset_s(first_tm, sizeof(struct rtc_time), 0, sizeof(struct rtc_time));
    memcpy_s(first_tv, sizeof(oal_timespec_stru), &pst_dft_item->first_timestamp,
             sizeof(pst_dft_item->first_timestamp));
    first_tv->tv_sec -= sys_tz.tz_minuteswest * OAL_SEC_PER_MIN;
    oal_rtc_ktime_to_tm(*first_tv, first_tm);

    memset_s(last_tm, sizeof(struct rtc_time), 0, sizeof(struct rtc_time));
    memcpy_s(last_tv, sizeof(oal_timespec_stru), &pst_dft_item->last_timestamp, sizeof(pst_dft_item->last_timestamp));
    if (last_tv->tv_sec) {
        last_tv->tv_sec -= sys_tz.tz_minuteswest * OAL_SEC_PER_MIN;
        oal_rtc_ktime_to_tm(*last_tv, last_tm);
        *last_time_flag = 1;
    }
}

OAL_STATIC void oal_dft_kern_info_printk(int32_t last_time_flag, oal_dft_trace_item *pst_dft_item,
                                         struct rtc_time first_tm, struct rtc_time last_tm)
{
    if (!last_time_flag) {
        printk(KERN_INFO "%s,1st at %02d-%02d %02d:%02d:%02d,total:%u\n",
               pst_dft_item->name,
               first_tm.tm_mon + 1,
               first_tm.tm_mday,
               first_tm.tm_hour, first_tm.tm_min, first_tm.tm_sec,
               pst_dft_item->trace_count);
    } else {
        printk(KERN_INFO "%s,1st at %02d-%02d %02d:%02d:%02d,last at %02d-%02d %02d:%02d:%02d,total:%u\n",
               pst_dft_item->name,
               first_tm.tm_mon + 1,
               first_tm.tm_mday,
               first_tm.tm_hour, first_tm.tm_min, first_tm.tm_sec,
               last_tm.tm_mon + 1,
               last_tm.tm_mday,
               last_tm.tm_hour, last_tm.tm_min, last_tm.tm_sec,
               pst_dft_item->trace_count);
    }
}

OAL_STATIC int32_t oal_dft_dump_key_info_printk(int32_t key_type)
{
    unsigned long flags;
    int32_t count = 0;

    int32_t last_time_flag;

    oal_timespec_stru first_tv;
    oal_timespec_stru last_tv;
    struct rtc_time first_tm;
    struct rtc_time last_tm;

    oal_list_entry_stru *pst_pos = NULL;
    oal_dft_trace_item *pst_dft_item = NULL;

    oal_spin_lock_irq_save(&g_dft_head_lock, &flags);
    oal_list_search_for_each(pst_pos, &g_dft_head)
    {
        oal_spin_unlock_irq_restore(&g_dft_head_lock, &flags);
        last_time_flag = 0;
        pst_dft_item = oal_list_get_entry(pst_pos, oal_dft_trace_item, list);
        if (key_type != pst_dft_item->dft_type) {
            oal_spin_lock_irq_save(&g_dft_head_lock, &flags);
            continue;
        }

        oal_dft_timestamp_init(&first_tm, &first_tv, &last_tm, &last_tv, pst_dft_item, &last_time_flag);

        oal_dft_kern_info_printk(last_time_flag, pst_dft_item, first_tm, last_tm);

        oal_spin_lock_irq_save(&g_dft_head_lock, &flags);
    }
    oal_spin_unlock_irq_restore(&g_dft_head_lock, &flags);

    return count;
}

OAL_STATIC int32_t oal_dft_kern_info_printbuf_first(char *buf, int32_t *count, uint32_t len,
                                                    oal_dft_trace_item *pst_dft_item, struct rtc_time first_tm)
{
    int32_t ret;

    ret = snprintf_s(buf + *count, len - *count, len - *count - 1,
                     "%s,1st at %02d-%02d %02d:%02d:%02d,total:%u\n",
                     pst_dft_item->name,
                     first_tm.tm_mon + 1,
                     first_tm.tm_mday,
                     first_tm.tm_hour, first_tm.tm_min, first_tm.tm_sec,
                     pst_dft_item->trace_count);
    if (ret < 0) {
        oal_io_print("log str format err line[%d]\n", __LINE__);
        return -OAL_EFAIL;
    }
    *count += ret;

    return OAL_SUCC;
}

OAL_STATIC int32_t oal_dft_kern_info_printbuf_last(char *buf, int32_t *count, uint32_t len,
                                                   oal_dft_trace_item *pst_dft_item, struct rtc_time first_tm,
                                                   struct rtc_time last_tm)
{
    int32_t ret;

    ret = snprintf_s(buf + *count, len - *count, len - *count - 1,
                     "%s,1st at %02d-%02d %02d:%02d:%02d,last at %02d-%02d %02d:%02d:%02d,total:%u\n",
                     pst_dft_item->name,
                     first_tm.tm_mon + 1,
                     first_tm.tm_mday,
                     first_tm.tm_hour, first_tm.tm_min, first_tm.tm_sec,
                     last_tm.tm_mon + 1,
                     last_tm.tm_mday,
                     last_tm.tm_hour, last_tm.tm_min, last_tm.tm_sec,
                     pst_dft_item->trace_count);
    if (ret < 0) {
        oal_io_print("log str format err line[%d]\n", __LINE__);
        return -OAL_EFAIL;
    }
    *count += ret;

    return OAL_SUCC;
}

OAL_STATIC int32_t oal_dft_dump_key_info_printbuf(char *buf, uint32_t len, int32_t key_type, int32_t *count)
{
    int32_t ret;
    unsigned long flags;
    int32_t last_time_flag;

    oal_timespec_stru first_tv;
    oal_timespec_stru last_tv;
    struct rtc_time first_tm;
    struct rtc_time last_tm;

    oal_list_entry_stru *pst_pos = NULL;
    oal_dft_trace_item *pst_dft_item = NULL;

    oal_spin_lock_irq_save(&g_dft_head_lock, &flags);
    oal_list_search_for_each(pst_pos, &g_dft_head)
    {
        oal_spin_unlock_irq_restore(&g_dft_head_lock, &flags);
        last_time_flag = 0;
        pst_dft_item = oal_list_get_entry(pst_pos, oal_dft_trace_item, list);
        if (key_type != pst_dft_item->dft_type) {
            oal_spin_lock_irq_save(&g_dft_head_lock, &flags);
            continue;
        }

        oal_dft_timestamp_init(&first_tm, &first_tv, &last_tm, &last_tv, pst_dft_item, &last_time_flag);

        if (buf == NULL) {
            oal_io_print("buf is null poniter\n");
            return -OAL_EFAIL;
        }

        if (!last_time_flag) {
            ret = oal_dft_kern_info_printbuf_first(buf, count, len, pst_dft_item, first_tm);
            if (ret != OAL_SUCC) {
                return -OAL_EFAIL;
            }
        } else {
            ret = oal_dft_kern_info_printbuf_last(buf, count, len, pst_dft_item, first_tm, last_tm);
            if (ret != OAL_SUCC) {
                return -OAL_EFAIL;
            }
        }
        oal_spin_lock_irq_save(&g_dft_head_lock, &flags);
    }
    oal_spin_unlock_irq_restore(&g_dft_head_lock, &flags);

    return OAL_SUCC;
}

OAL_STATIC int32_t oal_dft_dump_key_info(char *buf, uint32_t len, int32_t key_type, uint32_t print_tag)
{
    int32_t count = 0;
    int32_t ret;
    if (print_tag & KERN_PRINT) {
        count = oal_dft_dump_key_info_printk(key_type);
    }
    if (print_tag & BUFF_PRINT) {
        ret = oal_dft_dump_key_info_printbuf(buf, len, key_type, &count);
        if (ret != OAL_SUCC) {
            return ret;
        }
    }

    return count;
}

void oal_dft_print_error_key_info(void)
{
    int32_t i;
    int ret;
    printk(KERN_WARNING "[E]dump error key_info:\n");
    for (i = (OAL_DFT_TRACE_BUTT - 1); i >= OAL_DFT_TRACE_FAIL; i--) {
        ret = oal_dft_dump_key_info(NULL, 0, i, (KERN_PRINT));
        if (ret < 0) {
            oal_io_print("oal_dft_print_error_key_info[%d]::info str format err.\n", i);
        }
    }
}
oal_module_symbol(oal_dft_print_error_key_info);

void oal_dft_print_all_key_info(void)
{
    int32_t i;
    int ret;
    for (i = 0; i < OAL_DFT_TRACE_BUTT; i++) {
        ret = oal_dft_dump_key_info(NULL, 0, i, (KERN_PRINT));
        if (ret < 0) {
            oal_io_print("oal_dft_print_all_key_info[%d]::info str format err.\n", i);
        }
    }
}
oal_module_symbol(oal_dft_print_all_key_info);

OAL_STATIC ssize_t oal_dft_get_key_info(struct kobject *dev, struct kobj_attribute *attr, char *buf)
{
    int32_t i;
    int32_t ret = 0;
    int32_t count = 0;
    uint32_t dft_mask;

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

    dft_mask = g_dft_print_mask & (KERN_PRINT | BUFF_PRINT);
    g_dft_print_mask = dft_mask;

    if (oal_list_is_empty(&g_dft_head)) {
        printk(KERN_INFO "key info empty\n");
        ret = snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "key info empty\n");
        if (ret < 0) {
            oal_io_print("log str format err line[%d]\n", __LINE__);
        }
        return ret;
    }

    if (OAL_DFT_TRACE_BUTT == 0) {
        return ret;
    }

    for (i = (OAL_DFT_TRACE_BUTT - 1); i >= 0; i--) {
        ret = oal_dft_dump_key_info(buf + count, PAGE_SIZE - count, i, dft_mask);
        if (ret < 0) {
            oal_io_print("oal_dft_dump_key_info::info str format err.\n");
            return count;
        }
        count += ret;
    }

    return count;
}
#ifndef _PRE_WINDOWS_SUPPORT
OAL_STATIC struct kobj_attribute g_dev_attr_key_info =
    __ATTR(key_info, S_IRUGO, oal_dft_get_key_info, NULL);
#endif

#ifdef MPXX_DRV_VERSION
OAL_STATIC int32_t oal_dft_print_version_info(char *buf)
{
    int32_t ret;
    ret = snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%s\n", MPXX_DRV_VERSION);
    if (ret < 0) {
        oal_io_print("log str format err line[%d]\n", __LINE__);
    }
    return ret;
}

OAL_STATIC ssize_t oal_dft_get_version_info(struct kobject *dev, struct kobj_attribute *attr, char *buf)
{
    int32_t ret = 0;

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

    ret += oal_dft_print_version_info(buf + ret);

    return ret;
}
#ifndef _PRE_WINDOWS_SUPPORT
OAL_STATIC struct kobj_attribute g_dev_attr_version =
    __ATTR(version, S_IRUGO, oal_dft_get_version_info, NULL);
#endif
#endif

OAL_STATIC struct attribute *g_dft_sysfs_entries[] = {
    &g_dev_attr_key_info.attr,
#ifdef MPXX_DRV_VERSION
    &g_dev_attr_version.attr,
#endif
    NULL
};

OAL_STATIC struct attribute_group g_dft_attribute_group = {
    .name = "dft",
    .attrs = g_dft_sysfs_entries,
};

OAL_STATIC int32_t dft_sysfs_entry_init(void)
{
    int32_t ret;
    oal_kobject *pst_root_object = NULL;
    pst_root_object = oal_get_sysfs_root_object();
    if (pst_root_object == NULL) {
        oal_io_print("{frw_sysfs_entry_init::get sysfs root object failed!}\n");
        return -OAL_EFAIL;
    }

    ret = sysfs_create_group(pst_root_object, &g_dft_attribute_group);
    if (ret) {
        oal_io_print("{frw_sysfs_entry_init::sysfs create group failed!}\n");
        return ret;
    }
    return OAL_SUCC;
}

OAL_STATIC void dft_sysfs_entry_exit(void)
{
    oal_kobject *pst_root_object = NULL;
    pst_root_object = oal_get_sysfs_root_object();
    if (pst_root_object != NULL) {
        sysfs_remove_group(pst_root_object, &g_dft_attribute_group);
    }
}
#endif

int32_t oal_dft_init(void)
{
    int32_t ret = OAL_SUCC;
#ifdef _PRE_WINDOWS_SUPPORT
    oal_spin_lock_init(&g_dft_head_lock);
#endif
#ifdef _PRE_CONFIG_CONN_HISI_SYSFS_SUPPORT
    ret = dft_sysfs_entry_init();
#endif
    return ret;
}

void oal_dft_exit(void)
{
#ifdef _PRE_CONFIG_CONN_HISI_SYSFS_SUPPORT
    dft_sysfs_entry_exit();
#endif
}

oal_module_symbol(oal_dft_init);
oal_module_symbol(oal_dft_exit);
#endif

