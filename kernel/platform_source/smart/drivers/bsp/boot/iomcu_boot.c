/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2021. All rights reserved.
 * Description: Contexthub boot.
 * Create: 2019-06-10
 */
#include "securec.h"
#include <linux/delay.h>
#include <linux/err.h>
#include <platform_include/smart/linux/iomcu_boot.h>
#include <platform_include/smart/linux/iomcu_status.h>
#include <platform_include/smart/linux/iomcu_dump.h>
#include <platform_include/smart/linux/iomcu_ipc.h>
#include <linux/platform_drivers/bsp_syscounter.h>
#include <platform_include/basicplatform/linux/ipc_rproc.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/notifier.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/regulator/consumer.h>
#include <linux/workqueue.h>

static int g_boot_iom3 = STARTUP_IOM3_CMD;
static rproc_id_t ipc_ap_to_iom_mbx = IPC_AO_ACPU_IOM3_MBX1;
static BLOCKING_NOTIFIER_HEAD(iomcu_boot_notifier_list);

int register_iomcu_boot_notifier(struct notifier_block *nb)
{
	return blocking_notifier_chain_register(&iomcu_boot_notifier_list, nb);
}

int unregister_iomcu_boot_notifier(struct notifier_block *nb)
{
	return blocking_notifier_chain_unregister(&iomcu_boot_notifier_list, nb);
}

static int boot_iomcu(void)
{
	int ret;

	ret = RPROC_ASYNC_SEND(ipc_ap_to_iom_mbx, (mbox_msg_t *)&g_boot_iom3, 1);
	if (ret)
		pr_err("RPROC_ASYNC_SEND error in %s\n", __func__);
	return ret;
}

static int iomcu_boot_init(void)
{
	int ret;
	pr_info("%s ++\n", __func__);
	if (get_contexthub_dts_status())
		return -EACCES;

	blocking_notifier_call_chain(&iomcu_boot_notifier_list, IOMCU_BOOT_PRE, NULL);

	ret = boot_iomcu();
	if (ret != 0) {
		pr_err("boot_iomcu err\n");
		return ret;
	}

	blocking_notifier_call_chain(&iomcu_boot_notifier_list, IOMCU_BOOT_POST, NULL);

	pr_info("%s --\n", __func__);
	return 0;
}

static void __exit iomcu_boot_exit(void)
{
	RPROC_PUT(ipc_ap_to_iom_mbx);
}

late_initcall(iomcu_boot_init);
module_exit(iomcu_boot_exit);

MODULE_DESCRIPTION("iomcu boot");
MODULE_LICENSE("GPL");
