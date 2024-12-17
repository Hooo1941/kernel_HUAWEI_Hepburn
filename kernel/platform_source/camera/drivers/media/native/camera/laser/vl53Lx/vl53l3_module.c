/*
 * vl53l3_module.c
 * SOC camera driver source file
 * Copyright (C) 2023-2023 Huawei Technology Co., Ltd.
 */
/*lint -save -e574*/
#include "laser_module.h"
#include "hw_laser_dmd.h"

#define DRIVER_NAME "vendor,vl53l3"


extern int stmvl53l3_probe(struct i2c_client *client,
	const struct i2c_device_id *id);
extern int stmvl53l3_remove(struct i2c_client *client);
extern long stmvl53l3_laser_ioctl(void *hw_data, unsigned int cmd, void  *p);

extern int laser_probe(struct i2c_client *client,
	const struct i2c_device_id *id);
extern int laser_remove(struct i2c_client *client);

static laser_module_intf_t laser_devices[] = {
	{ "VL53L3", &stmvl53l3_probe, &stmvl53l3_remove, &stmvl53l3_laser_ioctl },
};

static int match_index = -1;

static long hw_laser_ioctl(void *hw_data, unsigned int cmd, void *p)
{
	int rc = 0;

	if (!hw_data) {
		laser_err("client is null");
		return -EINVAL;
	}

	if (match_index >= 0 && match_index < ARRAY_SIZE(laser_devices))
		rc = laser_devices[match_index].laser_ioctl(hw_data, cmd, p);

	return rc;
}

static int vl53l3_data_remove(struct i2c_client *client)
{
	int rc = 0;

	if (!client) {
		laser_err("client is null");
		return -EINVAL;
	}

	if (match_index >= 0 && match_index < ARRAY_SIZE(laser_devices))
		rc = laser_devices[match_index].data_remove(client);

	return rc;
}

static int vl53l3_data_init(struct i2c_client *client,
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
		report_laser_probe_state(HWLASER_VL53L3, PROBE_FAIL);
		laser_err("No st laser probe success");
		return -1;
	}

	match_index = i;
	laser_info("probed success sensor index = %d", i);
	return 0;
}

hw_laser_fn_t vl53l3_module_fn = {
	.laser_ioctl = hw_laser_ioctl,
};

static hw_laser_ctrl_t laser_module_ctrl = {
	.func_tbl = &vl53l3_module_fn,
	.data = NULL,
};

static const struct i2c_device_id vl53l3_module_id[] = {
	{ "vl53l3_970",  (unsigned long)&laser_module_ctrl},
	{},
};

MODULE_DEVICE_TABLE(i2c, vl53l3_module_id);

static const struct of_device_id vl53l3_module_of_match[] = {
	{ .compatible = "vendor,vl53l3_970", },
	{},
};
MODULE_DEVICE_TABLE(of, vl53l3_module_of_match);

static int laser_module_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int rc = 0;
	int match_times = 3;

	if (!client || !id)
		return -EINVAL;

	laser_info("module probe enter");
	do {
		match_times--;
		rc = vl53l3_data_init(client, id);
	} while (rc != 0 && match_times > 0);

	if (rc == 0) {
		laser_info("vl53l3 data init success");
		rc = laser_probe(client, id);
	}

	return rc;
}

static int laser_module_remove(struct i2c_client *client)
{
	int rc = 0;

	if (!client)
		return -EINVAL;

	laser_info("module remove enter");
	rc = vl53l3_data_remove(client);
	if (rc == 0)
		rc = laser_remove(client);
	return rc;
}

static struct i2c_driver vl53l3_i2c_driver = {
	.driver = {
		.name = DRIVER_NAME,
		.of_match_table = of_match_ptr(vl53l3_module_of_match),
	},
	.probe = laser_module_probe,
	.remove = laser_module_remove,
	.id_table = vl53l3_module_id,
};

static int __init vl53l3_module_init(void)
{
	laser_err("%s enter\n", __func__);
	return i2c_add_driver(&vl53l3_i2c_driver);
}

static void __exit vl53l3_module_exit(void)
{
	laser_err("%s enter", __func__);
	i2c_del_driver(&vl53l3_i2c_driver);
}

module_init(vl53l3_module_init);
module_exit(vl53l3_module_exit);
MODULE_DESCRIPTION("vl53l3 driver");
/*lint -restore*/