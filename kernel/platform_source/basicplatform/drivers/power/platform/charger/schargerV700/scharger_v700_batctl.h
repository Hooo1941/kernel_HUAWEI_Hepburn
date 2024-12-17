#ifndef __SCHARGER_V700_BATCTL_H__
#define __SCHARGER_V700_BATCTL_H__

#include <linux/device.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <linux/delay.h>
#include <linux/time.h>
#include <linux/regmap.h>
#include <securec.h>
#include <linux/printk.h>
#include <linux/workqueue.h>

#include <soc_schargerV700_interface.h>

#define BAT_CNCT_SEL_REG                (REG_BAT_CNCT_SEL_ADDR(REG_GLB_BASE))
#define BAT_CNCT_SEL_SHIFT              (REG_BAT_CNCT_SEL_sc_bat_cnct_sel_START)
#define BAT_CNCT_SEL_MSK                (1 << REG_BAT_CNCT_SEL_sc_bat_cnct_sel_START)

#define BAT_SERIES_MODE                 1
#define BAT_PARALLEL_MODE               0

#define BAT_SW_EN_REG                   (REG_BAT_SW_EN_ADDR(REG_GLB_BASE))
#define BAT_SW_EN_SHIFT                 (REG_BAT_SW_EN_sc_bat_sw_en_START)
#define BAT_SW_EN_MSK                   (1 << BAT_SW_EN_SHIFT)

#define BAT_SW_ENABLE                   1
#define BAT_SW_DISABLE                  0

#define BAT_DET_REG                     REG_CHG_TOP_CFG_REG_0_ADDR(REG_ANA_BASE)
#define BATSW_DET_EN_SHIFT                 (REG_CHG_TOP_CFG_REG_0_da_batsw_det_en_START)
#define BATSW_DET_EN_MSK                   (1 << BATSW_DET_EN_SHIFT)

int bat_cnct_mode_get(void);
int bat_cnct_ctrl(int mode);

/* only for factory AT cmd */
int bat_cnct_ctrl_test(int mode);

#endif
