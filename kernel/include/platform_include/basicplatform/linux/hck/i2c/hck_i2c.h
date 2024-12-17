#ifndef __HCK_I2C__
#define __HCK_I2C__

#include <linux/types.h>
#include <platform_include/basicplatform/linux/hck_vendor_hooks.h>

/*
 * Following tracepoints are not exported in tracefs and provide a
 * mechanism for vendor modules to hook and extend functionality
 */

struct dw_i2c_dev;

DECLARE_HCK_VH(i2c_dw_secure_read,
	TP_PROTO(struct dw_i2c_dev *dev, unsigned int reg, unsigned int *val, bool *err),
	TP_ARGS(dev, reg, val, err));

DECLARE_HCK_VH(i2c_dw_secure_write,
	TP_PROTO(struct dw_i2c_dev *dev, unsigned int reg, unsigned int val, bool *err),
	TP_ARGS(dev, reg, val, err));

DECLARE_HCK_VH(i2c_dw_read_comp_type,
	TP_PROTO(struct dw_i2c_dev *dev, unsigned int *reg, bool *err),
	TP_ARGS(dev, reg, err));

DECLARE_HCK_VH(i2c_dw_dma_fifo_cfg,
	TP_PROTO(struct dw_i2c_dev *dev, bool *err),
	TP_ARGS(dev, err));

DECLARE_HCK_VH(i2c_dw_xfer_init_irq_mask,
	TP_PROTO(struct dw_i2c_dev *dev, bool *err),
	TP_ARGS(dev, err));

DECLARE_HCK_VH(i2c_dw_irq_mask_all,
	TP_PROTO(struct dw_i2c_dev *dev),
	TP_ARGS(dev));

DECLARE_HCK_VH(i2c_dw_stop_irq_handler,
	TP_PROTO(struct dw_i2c_dev *dev, unsigned int stat),
	TP_ARGS(dev, stat));

#endif /* end of hck i2c */
