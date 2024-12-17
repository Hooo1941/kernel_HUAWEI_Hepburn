

#ifndef __NFC_DEV_ATTR_Q_H__
#define __NFC_DEV_ATTR_Q_H__

#include <linux/i2c.h>

int caxx_attr_init(struct i2c_client *client);
int caxx_attr_deinit(void);
int remove_attribute_sysfs(struct device *dev);
int create_attribute_node(struct i2c_client *client);
unsigned long get_timeout(void);
#endif /* end of nfc_dev_attr_q.h */