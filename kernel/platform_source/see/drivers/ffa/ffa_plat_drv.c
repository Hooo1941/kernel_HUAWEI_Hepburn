/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description: platform drivers ffa interface for kernel redirect to secure partition
 * Create: 2023-03-24
 */

#include <linux/init.h>
#include <linux/mutex.h>
#include <platform_include/see/ffa/ffa_plat_drv.h>

const struct ffa_ops *g_ffa_platdrv_ops = NULL;
struct ffa_device *g_ffa_platdrv_dev = NULL;
static struct mutex g_local_buf_lock;

const struct ffa_device_id g_platdrv_ffa_device_id[] = {
	/* uuid = <0xf9e75f33 0xa1ca4f82 0x98e6c802 0x0f20ecb4> */
	{{{0x33, 0x5f, 0xe7, 0xf9, 0x82, 0x4f, 0xca, 0xa1,
	   0x02, 0xc8, 0xe6, 0x98, 0xb4, 0xec, 0x20, 0x0f}}},
	{{{}}}
};

static void platdrv_ffa_remove(struct ffa_device *ffa_dev)
{
	(void)ffa_dev;
	pr_err("platdrv: platdrv_ffa_remove remove\n");
}

static int platdrv_ffa_probe(struct ffa_device *ffa_dev)
{
	g_ffa_platdrv_dev = ffa_dev;
	if (!g_ffa_platdrv_dev) {
		pr_err("platdrv: failed to get ffa_dev!\n");
		return -ENOENT;
	}

	g_ffa_platdrv_ops = ffa_dev->ops;
	if (!g_ffa_platdrv_ops) {
		pr_err("platdrv: fail to get ffa_ops\n");
		return -ENOENT;
	}

	g_ffa_platdrv_ops->mode_32bit_set(ffa_dev);
	pr_err("platdrv ffa probe OK!\n");
	return 0;
}

static struct ffa_driver g_platdrv_ffa_driver = {
	.name = "PLATDRV_FFA",
	.probe = platdrv_ffa_probe,
	.remove = platdrv_ffa_remove,
	.id_table = g_platdrv_ffa_device_id,
};

static int platdrv_ffa_init(void)
{
	pr_err("platdrv ffa init\n");
	mutex_init(&g_local_buf_lock);
	return ffa_register(&g_platdrv_ffa_driver);
}

/*
 * send message to platdrv sp
 * success: return 0
 */
int ffa_platdrv_send_msg(struct ffa_send_direct_data *args)
{
	int ret;

	if (g_ffa_platdrv_ops == NULL || g_ffa_platdrv_dev == NULL) {
		pr_err("platdrv : ffa not init\n");
		return -1;
	}

	ret = g_ffa_platdrv_ops->sync_send_receive(
				     g_ffa_platdrv_dev,
				     args);

	return ret;
}

/*
 * send message to platdrv sp with share memory
 * only arg0-arg2 can be used
 * local buffer size: u8 local_buf[FFA_LOCAL_BUFFER_MAX]
 * success: return 0
 */
int ffa_platdrv_send_msg_with_shmem(struct ffa_send_direct_data *args,
				    u8 *local_buf, u64 dir)
{
	int ret;

	if (g_ffa_platdrv_ops == NULL || g_ffa_platdrv_dev == NULL) {
		pr_err("platdrv : ffa not init\n");
		return -1;
	}

	mutex_lock(&g_local_buf_lock);
	ret = g_ffa_platdrv_ops->sync_send_receive_with_shmem(
				 g_ffa_platdrv_dev, args,
				 (void *)local_buf, dir);
	mutex_unlock(&g_local_buf_lock);

	return ret;
}

subsys_initcall(platdrv_ffa_init);
