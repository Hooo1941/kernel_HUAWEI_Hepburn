/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description: gpio board-level implementation
 * Author: @CompanyNameTag
 */
#include <linux/notifier.h>
#include <linux/suspend.h>
#include <linux/pm_wakeup.h>

#include "plat_debug.h"
#include "board.h"
#include "board_pm.h"

#define BOARD_SR_NOTIFIER_PRIORITY   1

int board_pm_notify_handler(struct notifier_block *notify_block, unsigned long mode, void *unused)
{
    ps_print_info("board_suspend_notify mode[%lu]\n", mode);
    if (g_st_board_info.bd_ops.pm_notify != NULL) {
        return g_st_board_info.bd_ops.pm_notify(mode, unused);
    }
    return 0;
}

static struct notifier_block g_board_sr_notifier = {
    .notifier_call = board_pm_notify_handler,
    .priority = BOARD_SR_NOTIFIER_PRIORITY,
};

int board_pm_notifier_register(void)
{
    int ret;

    ret = register_pm_notifier(&g_board_sr_notifier);
    if (ret < 0) {
        ps_print_warning("register pm_notifier failed, ret = %d!\n", ret);
        return ret;
    }
    return BOARD_SUCC;
}
