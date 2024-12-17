/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019. All rights reserved.
 * Description: iomcu_boot.h.
 * Create: 2022/06/09
 */
#ifndef __IOMCU_BOOT_H__
#define __IOMCU_BOOT_H__

#define IOMCU_BOOT_PRE     0
#define IOMCU_BOOT_POST   1
/**
 * @brief register iomcu boot notifier.
 *
 * @param nb     : notifier.
 * @return int   : SUCCESS or FAIL.
 */
int register_iomcu_boot_notifier(struct notifier_block *nb);

/**
 * @brief unregister iomcu boot notifier.
 *
 * @param nb     : notifier.
 * @return int   : SUCCESS or FAIL.
 */
int unregister_iomcu_boot_notifier(struct notifier_block *nb);

#endif /* __IOMCU_BOOT_H__ */
