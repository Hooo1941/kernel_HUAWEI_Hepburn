/*
 * laser_tof8801_module.c
 *
 * SOC camera driver source file
 *
 * Copyright (C) 2024-2024 Huawei Technology Co., Ltd.
 *
 * Date: 2024-02-23
 *
 */

/*lint -save -e574*/
#include "laser_tof8801_module.h"

#define DRIVER_NAME "vendor,tof8801"

extern int tof_probe(struct i2c_client *client,
	const struct i2c_device_id *id);
extern int tof_remove(struct i2c_client *client);
extern long tof_ioctl(void *hw_data, unsigned int cmd, void *p);

extern int laser_probe(struct i2c_client *client,
	const struct i2c_device_id *id);
extern int laser_remove(struct i2c_client *client);

static laser_module_intf_t laser_devices[] = {
	{ "TOF8801", &tof_probe, &tof_remove, &tof_ioctl },
};

static int match_index = -1;

static long hw_laser_tof8801_ioctl(void *hw_data, unsigned int cmd, void  *p)
{
	int rc = 0;

	if (!hw_data) {
		laser_err("parameter error");
		return -EINVAL;
	}
	if (match_index >= 0 && match_index < ARRAY_SIZE(laser_devices)) {
		if (laser_devices[match_index].laser_ioctl) {
			rc = laser_devices[match_index].laser_ioctl(hw_data, cmd, p);
		} else {
			laser_err("Consolasy ioctl is null");
		}
	}
	return rc;
}

static int laser_data_remove(struct i2c_client *client)
{
	int rc = 0;

	if (!client) {
		laser_err("parameter error");
		return -EINVAL;
	}

	if (match_index >= 0 && match_index < ARRAY_SIZE(laser_devices))
		rc = laser_devices[match_index].data_remove(client);

	return rc;
}

static int laser_data_init(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	/* try sub devices */
	int rc = 0;
	int i = 0;

	if (!client || !id) {
		laser_err("parameter error");
		return -EINVAL;
	}

	for (i = 0; i < ARRAY_SIZE(laser_devices); i++) {
		rc = laser_devices[i].data_init(client, id);
		if (rc == 0)
			break;
	}

	if (i == ARRAY_SIZE(laser_devices)) {
		laser_err("No tof laser probe success");
		return -1;
	}

	match_index = i;
	laser_info("probed success sensor index = %d", i);
	return 0;
}

hw_laser_fn_t laser_tof8801_module_fn = {
	.laser_ioctl = hw_laser_tof8801_ioctl,
};

static hw_laser_ctrl_t laser_tof8801_module_ctrl = {
	.func_tbl = &laser_tof8801_module_fn,
	.data = NULL,
};

static const struct i2c_device_id laser_tof8801_module_id[] = {
	{ "tof8801",  (unsigned long)&laser_tof8801_module_ctrl},
	{ },
};
MODULE_DEVICE_TABLE(i2c, laser_tof8801_module_id);

static const struct of_device_id laser_tof8801_module_of_match[] = {
	{ .compatible = "vendor,tof8801", },
	{ },
};
MODULE_DEVICE_TABLE(of, laser_tof8801_module_of_match);

static int laser_tof8801_module_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int rc = 0;
	int match_times = 3;

	if (!client || !id)
		return -EINVAL;

	laser_info("module probe enter");
	do {
		match_times--;
		rc = laser_data_init(client, id);
	} while (rc != 0 && match_times > 0);

	if (rc == 0) {
		laser_info("laser_data_init success");
		rc = laser_probe(client, id);
	}

	return rc;
}

static int laser_tof8801_module_remove(struct i2c_client *client)
{
	int rc = 0;

	if (!client)
		return -EINVAL;

	laser_info("module remove enter");
	rc = laser_data_remove(client);
	if (rc == 0)
		rc = laser_remove(client);
	return rc;
}

static struct i2c_driver laser_tof8801_module_i2c_driver = {
	.driver = {
		.name = DRIVER_NAME,
		.of_match_table = of_match_ptr(laser_tof8801_module_of_match),
	},
	.probe = laser_tof8801_module_probe,
	.remove = laser_tof8801_module_remove,
	.id_table = laser_tof8801_module_id,
};

static int __init laser_tof8801_module_init(void)
{
	laser_err("%s enter\n", __func__);
	return i2c_add_driver(&laser_tof8801_module_i2c_driver);
}

static void __exit laser_tof8801_module_exit(void)
{
	laser_err("%s enter", __func__);
	i2c_del_driver(&laser_tof8801_module_i2c_driver);
}

module_init(laser_tof8801_module_init);
module_exit(laser_tof8801_module_exit);
MODULE_DESCRIPTION("laser driver");
MODULE_LICENSE("GPL v2");
/*lint -restore*/
