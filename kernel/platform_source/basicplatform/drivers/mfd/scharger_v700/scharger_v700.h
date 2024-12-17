#ifndef __SCHARGER_V700_H__
#define __SCHARGER_V700_H__
#include <linux/i2c.h>
#include <linux/device.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/of_gpio.h>
#include <linux/regmap.h>
#include <linux/irq.h>
#include <linux/time.h>

#include <chipset_common/hwpower/common_module/power_event_ne.h>
#include <linux/printk.h>
#include <soc_schargerV700_interface.h>

#define MASK 0
#define UNMASK 1
#define SCHARGER_BITS 8
#define SCHARGER_MASK_FIELD 0x7f
#define SCHARGER_MASK_STATE 0x7f
#ifndef NO_IRQ
#define NO_IRQ 0
#endif
#define SCHARGER_IRQ_NAME_SIZE 20
#define SCHARGER_CHIP_IRQ_NAME_SIZE 20
#define ONE_IRQ_GROUP_NUM 8
#define REGMAP_SIZE 6

#define I2C_ERR_MAX_COUNT               5
#define RESET_VAL_ZERO                  0

#define CHIP_VERSION_0                  (REG_VERSION0_RO_REG_0_ADDR(REG_GLB_BASE))
#define CHIP_VERSION_4                  (REG_VERSION4_RO_REG_0_ADDR(REG_GLB_BASE))
#define CHARGE_IC_GOOD                  0
#define CHARGE_IC_BAD                   1
#define CHIP_ID_6526                    0x36323536

#define CHIP_ID_V700                    0x7000
#define CHIP_ID_V710                    0x0007

#define CHG_IRQ_FLAG          REG_IRQ_FLAG_ADDR(REG_IRQ_BASE)
#define CHG_IRQ_PD            (0x1 << 1)
#define CHG_IRQ_NONE_PD       (~(0x1 << 1))

#define COUL_IRQ_BASE         REG_COUL_IRQ_FLAG_ADDR(REG_COUL_NOPWR0_BASE)
#define COUL_IRQ_REGS_NUM     3
#define CHG_IRQ_BASE          REG_IRQ_FLAG_0_ADDR(REG_IRQ_BASE)
#define CHG_IRQ_REGS_NUM      12

#define PD_IRQ_BASE          REG_PD_ALERT_L_ADDR(REG_PD_BASE)
#define PD_IRQ_REGS_NUM      2
#define PD_FAULT_REG         REG_PD_FAULT_STATUS_ADDR(REG_PD_BASE)
#define CLEAR_ALL            0xFF

#define DEBUG_WRITE_PRO_REG   (REG_DEBUG_WRITE_PRO_ADDR(REG_COUL_NOPWR0_BASE))
#define DEBUG_WRITE_PRO_SHIFT (REG_DEBUG_WRITE_PRO_debug_write_pro_START)
#define DEBUG_WRITE_PRO_MSK   (0xFF << DEBUG_WRITE_PRO_SHIFT)
#define WRITE_PRO_UNPROTECTED 0xac
#define WRITE_PRO_PROTECTED   0x5a
#define CLJ_DEBUG1_REG        (REG_CLJ_DEBUG1_ADDR(REG_COUL_NOPWR0_BASE))
#define COUL_CTRL_ONOFF_SHIFT (REG_CLJ_DEBUG1_coul_ctrl_onoff_START)
#define COUL_CTRL_ONOFF_MSK   (0x01 << COUL_CTRL_ONOFF_SHIFT)
#define COUL_OFF              0
#define COUL_ON               1

#define IRQ_DELAY 10
#define IRQ_RETRY_CNT 20

#ifndef unused
#define unused(x) ((void)(x))
#endif

#define reg_cfg(r, m, s, v, b, a)  { .reg = (r), .mask = (m), .shift = (s), .val = (v), .before = (b), .after = (a), }
struct opt_regs {
	u16 reg;
	u8 mask;
	u8 shift;
	u32 val;
	unsigned long before; /* before delay ms */
	unsigned long after; /* after delay ms */
};

struct scharger_v700_priv {
	struct i2c_client *client;
	struct device *dev;
	struct regmap *regmap;
	struct mutex irq_lock;
	struct irq_domain *domain;
	int gpio;
	int irq;
	int irqnum;
	int irqarray;
	int irq_ops;
	unsigned int *irqs;
	int *irq_mask_addr_arry;
	int *irq_addr_arry;
	struct irq_chip irq_chip;
	struct delayed_work irq_work;
	struct rt_mutex i2c_lock;
	unsigned int scharger_version;
};

#endif
