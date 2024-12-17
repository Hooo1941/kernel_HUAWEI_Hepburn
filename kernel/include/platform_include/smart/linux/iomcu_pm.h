/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019. All rights reserved.
 * Description: iomcu_pm.h.
 * Create: 2019/11/05
 */
#ifndef __IOMCU_PM_H
#define __IOMCU_PM_H
#include <platform_include/smart/linux/base/ap/protocol.h>

#ifdef __cplusplus
extern "C" {
#endif

enum sub_power_status {
	SUB_POWER_ON,
	SUB_POWER_OFF
};

struct iomcu_power_status {
	uint8_t power_status;
	uint8_t app_status[TAG_END];
	uint32_t idle_time;
	uint64_t active_app_during_suspend;
};

/**
 * @brief tell ap status to iomcu
 *
 * @param ap_st   : ap status, refer to ST_XXX.
 * @return int    : SUCCESS or FAIL.
 */
int tell_ap_status_to_mcu(int ap_st);

/**
 * @brief tell fold status to iomcu
 *
 * @param ap_st   : ap status, refer to ST_XXX.
 * @return int    : SUCCESS or FAIL.
 */
#ifdef CONFIG_CONTEXTHUB_DSS_TRISTATEID
int tell_fold_status_to_mcu(int ap_st);
#endif

/**
 * @brief Get IOMCU SR status
 *
 * @return sys_status_t : refer to sys_status_t.
 */
sys_status_t get_iom3_sr_status(void);

/**
 * @brief enter contexthub s4 status
 *
 * @return int : 0 is ok, otherwise error.
 */
int sensorhub_pm_s4_entry(void);

/**
 * @brief get iomcu power state.
 *
 * @return int   : refer to ST_xxx
 */
int get_iomcu_power_state(void);

/**
 * @brief atomic get iomcu power status.
 *
 * @param p      : iomcu power status.
 * @return NA
 */
void atomic_get_iomcu_power_status(struct iomcu_power_status *p);

/**
 * @brief register iomcu pm notifier.
 *
 * @param nb     : notifier.
 * @return int   : SUCCESS or FAIL.
 */
int register_iomcu_pm_notifier(struct notifier_block *nb);

/**
 * @brief unregister iomcu pm notifier.
 *
 * @param nb     : notifier.
 * @return int   : SUCCESS or FAIL.
 */
int unregister_iomcu_pm_notifier(struct notifier_block *nb);

#ifdef __cplusplus
}
#endif
#endif /* __IOMCU_PM_H */
