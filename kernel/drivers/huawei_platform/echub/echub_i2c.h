/*
 * echub/echub_i2c.h
 *
 * echub i2c function interface & definition
 *
 * Copyright (c) 2018-2019 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#ifndef _ARMPC_ECHUB_I2C
#define _ARMPC_ECHUB_I2C

/* add echub i2c define */
#define ECHUB_I2C_WRITE                       (0x00)
#define ECHUB_I2C_READ                        (0x01)
#define ECHUB_I2C_WRITE_DATA_HEAD             (0x03)
#define ECHUB_WRITE_LEN                       (0x07)
#define ECHUB_READ_LEN                        (0x09)
#define REG_DATA_LEN                          (0x01)
#define ECHUB_I2C_ADAPTER                     (0x07)
#define BIT_OFFSET                            (0x08)
#define ECHUB_SEND_BYTE_LEN                   (0x04)
#define ECHUB_SEND_TRANS_LEN                  (0x02)
#define ECHUB_I2C_MSG_MAX_LEN                 (0x100)

/* add CRC check define */
#define REG_ECHUB_LEN                         (0x01)
#define CRC_MASK                              (0x0F)
#define CRC_SHIFT                             (0x04)
#define CALLBACK_LEN                          (0x02)

struct echub_i2c_dev {
	struct device *dev;
	struct i2c_client *client;

	/* read_func() - I2C register read function */
	int (*read_func)(struct echub_i2c_dev *echub_dev, int regoffset, char regdata, char *value, int bytes);
	/* write_func() - I2C register write function */
	int (*write_func)(struct echub_i2c_dev *echub_dev, int regoffset, char regdata);
	int (*transfer_func)(struct echub_i2c_dev *echub_dev, int regoffset, unsigned char *wb, int wblen,
			unsigned char *rb, int rblen);
	/*
	 * Lock protects against activities from other Linux tasks,
	 * but not from changes by other I2C masters.
	 */
	struct mutex lock;
};
struct echub_i2c_dev *get_echub_dev(void);

#endif
