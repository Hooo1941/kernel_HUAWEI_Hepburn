/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: gear ctrl implement
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/kernel.h>
#include <linux/mutex.h>
#include <linux/kobject.h>
#include <linux/blkdev.h>
#include "mas_blk_gear_ctrl_sysfs.h"
#include "mas_blk_busy_idle_interface.h"
#include "mas_blk_latency_interface.h"
#include "mas_blk_core_interface.h"

struct mas_queue_sysfs_entry {
	struct attribute attr;
	ssize_t (*show)(struct request_queue *, char *);
	ssize_t (*store)(struct request_queue *, const char *, size_t);
};

#define mas_queue_wo_entry(_prefix, _mname, _name)                             \
	static struct mas_queue_sysfs_entry _prefix##mas_##_mname##_entry = {  \
		.attr = { .name = (_name), .mode = S_IWUSR },                  \
		.store = _prefix##mas_##_mname##_store,                        \
	}

#define mas_queue_ro_entry(_prefix, _mname, _name)                             \
	static struct mas_queue_sysfs_entry _prefix##mas_##_mname##_entry = {  \
		.attr = { .name = (_name), .mode = S_IRUGO },                  \
		.show = _prefix##mas_##_mname##_show,                          \
	}

#define mas_queue_rw_entry(_prefix, _mname, _name)                             \
	static struct mas_queue_sysfs_entry _prefix##mas_##_mname##_entry = {  \
		.attr = { .name = (_name), .mode = S_IRUSR | S_IWUSR },        \
		.show = _prefix##mas_##_mname##_show,                          \
		.store = _prefix##mas_##_mname##_store,                        \
	}

#if defined(CONFIG_MAS_DEBUG_FS)
#ifdef CONFIG_MAS_UFS_GEAR_CTRL
mas_queue_rw_entry(, gear_ctrl, "gear_ctrl");
mas_queue_rw_entry(, gear_auto_enable, "gear_auto_enable");
mas_queue_rw_entry(, gear_ufs_rate, "gear_ufs_rate");
mas_queue_rw_entry(, gear_fg_num, "gear_fg_num");
mas_queue_rw_entry(, gear_sync_num, "gear_sync_num");
mas_queue_rw_entry(, min_gear_stay_time, "min_gear_stay_time");
mas_queue_rw_entry(, gear_flow_cnt_per_ms, "gear_flow_cnt_per_ms");
mas_queue_rw_entry(, gear_flow_threshold_size_per_100us, "gear_flow_threshold_size_per_100us");
mas_queue_rw_entry(, gear_flow_threshold_inflat_size, "gear_threshold_inflat_size");
#endif
#ifdef CONFIG_MAS_MQ_USING_CPP
mas_queue_rw_entry(, queue_cpplus_debug_en, "cpplus_debug_en");
mas_queue_ro_entry(, queue_cpp_io_sum_cnt, "cpp_io_sum_cnt");
mas_queue_ro_entry(, queue_cpp_io_cnt, "cpp_io_cnt");
mas_queue_ro_entry(, queue_lrb_in_use, "lrb_in_use");
#endif
#ifdef CONFIG_MAS_MCQ
mas_queue_ro_entry(, mcq_per_cpu_tags, "per_cpu_tags");
mas_queue_ro_entry(, mcq_dfa_enable, "mas_mcq_dfa");
#endif /* CONFIG_MAS_MCQ */
#endif


#ifdef CONFIG_MAS_DEBUG_FS
mas_queue_ro_entry(__cfi_, queue_status, "mas_queue_feature_status");
mas_queue_wo_entry(__, queue_io_latency_warning_threshold, "io_latency_warning_threshold");
mas_queue_wo_entry(__cfi_, queue_busyidle_enable, "busy_idle_enable");
mas_queue_wo_entry(__cfi_, queue_busyidle_statistic_reset, "busy_idle_statistic_reset");
mas_queue_ro_entry(__cfi_, queue_busyidle_statistic, "busy_idle_statistic");
mas_queue_ro_entry(__cfi_, queue_hw_idle_enable, "hw_idle_enable");
mas_queue_rw_entry(__cfi_, queue_io_prio_sim, "io_prio_sim");
mas_queue_ro_entry(__cfi_, queue_io_flush_time_debug, "io_flush_time_debug");
mas_queue_ro_entry(__cfi_, queue_idle_state, "idle_state");
mas_queue_ro_entry(, queue_tz_write_bytes, "tz_write_bytes");
#ifdef CONFIG_MAS_UNISTORE_PRESERVE
mas_queue_ro_entry(, queue_device_pwron_info, "device_pwron_info");
mas_queue_ro_entry(, queue_stream_oob_info, "stream_oob_info");
mas_queue_wo_entry(, queue_device_reset_ftl, "device_reset_ftl");
mas_queue_ro_entry(, queue_device_read_section, "device_read_section");
mas_queue_ro_entry(, queue_device_read_pu_size, "device_read_pu_size");
mas_queue_wo_entry(, queue_device_fs_sync_done, "fs_sync_done");
mas_queue_wo_entry(, queue_device_rescue_block_inject_data, "rescue_block_inject_data");
mas_queue_wo_entry(, queue_device_data_move, "data_move");
mas_queue_wo_entry(, queue_device_data_move_num, "data_move_num");
mas_queue_ro_entry(, queue_device_slc_mode_configuration, "slc_mode_configuration");
mas_queue_ro_entry(, queue_device_sync_read_verify, "sync_read_verify");
mas_queue_wo_entry(, queue_device_sync_read_verify_cp_verify_l4k, "sync_read_verify_cp_verify");
mas_queue_wo_entry(, queue_device_sync_read_verify_cp_open_l4k, "sync_read_verify_cp_open");
mas_queue_wo_entry(, queue_device_bad_block_notify_regist, "bad_block_notify_regist");
mas_queue_wo_entry(, queue_device_bad_block_total_num, "bad_block_total_num");
mas_queue_wo_entry(, queue_device_bad_block_bad_num, "bad_block_bad_num");
mas_queue_rw_entry(, queue_unistore_debug_en, "unistore_debug_en");
mas_queue_rw_entry(, queue_recovery_debug_on, "reset_recovery_debug");
mas_queue_ro_entry(, queue_recovery_page_cnt, "reset_recovery_page");
mas_queue_ro_entry(, queue_reset_cnt, "reset_cnt");
mas_queue_rw_entry(, queue_enable_disorder, "disorder_enabled");
mas_queue_rw_entry(, queue_max_recovery_num, "max_recovery_num");
mas_queue_rw_entry(, queue_recovery_del_num, "recovery_del_num");
mas_queue_ro_entry(, queue_unistore_en, "unistore_enabled");
mas_queue_ro_entry(, queue_async_fail_cnt, "async_submit_bio_fail");
static struct mas_queue_sysfs_entry queue_device_config_mapping_partition = {
	.attr = { .name = "device_config_mapping_partition",
		  .mode = S_IRUGO | S_IWUSR },
	.show = mas_queue_device_config_mapping_partition_show,
	.store = mas_queue_device_config_mapping_partition_store,
};
#endif /* CONFIG_MAS_UNISTORE_PRESERVE */
#ifdef CONFIG_MAS_MQ_USING_CP
mas_queue_rw_entry(, queue_cp_enabled, "cp_enabled");
mas_queue_rw_entry(, queue_cp_debug_en, "cp_debug_en");
mas_queue_rw_entry(, queue_cp_limit, "cp_limit");
#endif /* CONFIG_MAS_MQ_USING_CP */
#ifdef CONFIG_MAS_ORDER_PRESERVE
mas_queue_rw_entry(, queue_order_enabled, "order_enabled");
mas_queue_rw_entry(, queue_order_debug_en, "order_debug_en");
#endif /* CONFIG_MAS_ORDER_PRESERVE */
#endif /* CONFIG_MAS_DEBUG_FS */

static ssize_t queue_usr_ctrl_store(struct request_queue *q, const char *page, size_t count)
{
	int ret;

	ret = queue_var_store(&q->mas_queue.usr_ctrl_n, page, count);
	if (ret < 0)
		q->mas_queue.usr_ctrl_n = 0;
	mas_blk_queue_usr_ctrl_set(q);
	return ret;
}
static struct mas_queue_sysfs_entry queue_usr_ctrl_entry = {
	.attr = {.name = "usr_ctrl", .mode =  S_IWUSR | S_IWGRP },
	.show = NULL,
	.store = queue_usr_ctrl_store,
};


static struct attribute *mas_queue_attrs[] = {
	&queue_usr_ctrl_entry.attr,
#if defined(CONFIG_MAS_DEBUG_FS)
#ifdef CONFIG_MAS_UFS_GEAR_CTRL
	&mas_gear_ctrl_entry.attr,
	&mas_gear_auto_enable_entry.attr,
	&mas_gear_ufs_rate_entry.attr,
	&mas_gear_fg_num_entry.attr,
	&mas_gear_sync_num_entry.attr,
	&mas_min_gear_stay_time_entry.attr,
	&mas_gear_flow_cnt_per_ms_entry.attr,
	&mas_gear_flow_threshold_size_per_100us_entry.attr,
	&mas_gear_flow_threshold_inflat_size_entry.attr,
#endif
#ifdef CONFIG_MAS_MQ_USING_CPP
	&mas_queue_cpplus_debug_en_entry.attr,
	&mas_queue_cpp_io_sum_cnt_entry.attr,
	&mas_queue_cpp_io_cnt_entry.attr,
	&mas_queue_lrb_in_use_entry.attr,
#endif
#ifdef CONFIG_MAS_MCQ
	&mas_mcq_per_cpu_tags_entry.attr,
	&mas_mcq_dfa_enable_entry.attr,
#endif
#endif
#ifdef CONFIG_MAS_DEBUG_FS
	&__cfi_mas_queue_status_entry.attr,
	&__mas_queue_io_latency_warning_threshold_entry.attr,
	&__cfi_mas_queue_busyidle_enable_entry.attr,
	&__cfi_mas_queue_busyidle_statistic_reset_entry.attr,
	&__cfi_mas_queue_busyidle_statistic_entry.attr,
	&__cfi_mas_queue_hw_idle_enable_entry.attr,
	&__cfi_mas_queue_io_prio_sim_entry.attr,
	&__cfi_mas_queue_io_flush_time_debug_entry.attr,
	&__cfi_mas_queue_idle_state_entry.attr,
	&mas_queue_tz_write_bytes_entry.attr,
#ifdef CONFIG_MAS_UNISTORE_PRESERVE
	&mas_queue_device_pwron_info_entry.attr,
	&mas_queue_stream_oob_info_entry.attr,
	&mas_queue_device_reset_ftl_entry.attr,
	&mas_queue_device_read_section_entry.attr,
	&mas_queue_device_read_pu_size_entry.attr,
	&queue_device_config_mapping_partition.attr,
	&mas_queue_device_fs_sync_done_entry.attr,
	&mas_queue_device_rescue_block_inject_data_entry.attr,
	&mas_queue_device_data_move_entry.attr,
	&mas_queue_device_data_move_num_entry.attr,
	&mas_queue_device_slc_mode_configuration_entry.attr,
	&mas_queue_device_sync_read_verify_entry.attr,
	&mas_queue_device_sync_read_verify_cp_verify_l4k_entry.attr,
	&mas_queue_device_sync_read_verify_cp_open_l4k_entry.attr,
	&mas_queue_device_bad_block_notify_regist_entry.attr,
	&mas_queue_device_bad_block_total_num_entry.attr,
	&mas_queue_device_bad_block_bad_num_entry.attr,
	&mas_queue_unistore_debug_en_entry.attr,
	&mas_queue_recovery_debug_on_entry.attr,
	&mas_queue_recovery_page_cnt_entry.attr,
	&mas_queue_reset_cnt_entry.attr,
	&mas_queue_enable_disorder_entry.attr,
	&mas_queue_max_recovery_num_entry.attr,
	&mas_queue_recovery_del_num_entry.attr,
	&mas_queue_unistore_en_entry.attr,
	&mas_queue_async_fail_cnt_entry.attr,
#endif /* CONFIG_MAS_UNISTORE_PRESERVE */
#ifdef CONFIG_MAS_MQ_USING_CP
	&mas_queue_cp_enabled_entry.attr,
	&mas_queue_cp_debug_en_entry.attr,
	&mas_queue_cp_limit_entry.attr,
#endif /* CONFIG_MAS_MQ_USING_CP */
#ifdef CONFIG_MAS_ORDER_PRESERVE
	&mas_queue_order_enabled_entry.attr,
	&mas_queue_order_debug_en_entry.attr,
#endif /* CONFIG_MAS_ORDER_PRESERVE */
#endif /* CONFIG_MAS_DEBUG_FS */
	NULL,
};

struct attribute_group mas_queue_attr_group = {
	.attrs = mas_queue_attrs,
};

int mas_blk_register_queue(struct request_queue *q)
{
	return sysfs_create_group(&q->kobj, &mas_queue_attr_group);
}
EXPORT_SYMBOL_GPL(mas_blk_register_queue);
