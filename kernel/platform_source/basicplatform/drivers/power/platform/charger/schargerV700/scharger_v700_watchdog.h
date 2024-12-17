#ifndef __SCHARGER_V700_WATCHDOG_H__
#define __SCHARGER_V700_WATCHDOG_H__

#include <soc_schargerV700_interface.h>

#include <linux/slab.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <linux/regmap.h>
#include <linux/delay.h>
#include <linux/time.h>
#include <linux/printk.h>
#include <securec.h>

#define WATCHDOG_CTRL_REG               (REG_WDT_CTRL_ADDR(REG_GLB_BASE))
#define WATCHDOG_TIMER_SHIFT            (0)
#define WATCHDOG_TIMER_MSK              (0x03 << WATCHDOG_TIMER_SHIFT)
#define WATCHDOG_TIMER_01_S             1
#define WATCHDOG_TIMER_02_S             2
#define WATCHDOG_TIMER_05_S             5
#define WATCHDOG_TIMER_40_S             40

#define WATCHDOG_SOFT_RST_REG           (REG_WDT_SOFT_RST_ADDR(REG_GLB_BASE))
#define WD_RST_N_SHIFT                  (0)
#define WD_RST_N_MSK                    (0x01 << WD_RST_N_SHIFT)
#define WATCHDOG_TIMER_RST              (1 << WD_RST_N_SHIFT)

#define WATCHDOG_EN_REG                 (REG_SC_WDT_EN_ADDR(REG_GLB_BASE))
#define WATCHDOG_EN_SHIFT               (0)
#define WATCHDOG_EN_MSK                 (0x01 << WATCHDOG_EN_SHIFT)

#define WATCHDOG_WORK_TIMEOUT           2000
#define WATCHDOG_WORK_TIMEOUT_S         2

int scharger_set_watchdog_timer(int value);
int scharger_reset_watchdog_timer(void);
int scharger_set_watchdog_timer_ms(int ms);
#endif
