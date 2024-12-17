/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description: ddrc ffa interface for kernel redirect to secure partition
 * Create: 2023-04-24
 */

#include <linux/init.h>
#include <linux/mutex.h>
#include <platform_include/see/kernel_ddrc_ffa_init.h>

const struct ffa_ops *g_ffa_ddrc_ops = NULL;
struct ffa_device *g_ffa_ddrc_dev = NULL;
static struct mutex g_local_buf_lock;

const struct ffa_device_id g_ddrc_ffa_device_id[] = {
	/* uuid = <c3e3771c-7c884f82-bd399eba-8b348c75> */
	{{{0x1c, 0x77, 0xe3, 0xc3, 0x82, 0x4f, 0x88, 0x7c,
	   0xba, 0x9e, 0x39, 0xbd, 0x75, 0x8c, 0x34, 0x8b}}},
	{{{}}}
};

static void ddrc_ffa_remove(struct ffa_device *ffa_dev)
{
	(void)ffa_dev;
	pr_err("ddrc: ddrc_ffa_remove remove\n");
}

static int ddrc_ffa_probe(struct ffa_device *ffa_dev)
{
	g_ffa_ddrc_dev = ffa_dev;
	if (!g_ffa_ddrc_dev) {
		pr_err("ddrc: failed to get ffa_dev!\n");
		return -ENOENT;
	}

	g_ffa_ddrc_ops = ffa_dev->ops;
	if (!g_ffa_ddrc_ops) {
		pr_err("ddrc: fail to get ffa_ops\n");
		return -ENOENT;
	}

	g_ffa_ddrc_ops->mode_32bit_set(ffa_dev);
	pr_err("ddrc ffa probe OK!\n");
	return 0;
}

static struct ffa_driver g_ddrc_ffa_driver = {
	.name = "DDRC_FFA",
	.probe = ddrc_ffa_probe,
	.remove = ddrc_ffa_remove,
	.id_table = g_ddrc_ffa_device_id,
};

static int ddrc_ffa_init(void)
{
	pr_err("ddrc ffa init\n");
	mutex_init(&g_local_buf_lock);
	return ffa_register(&g_ddrc_ffa_driver);
}

/*
 * send message to ddrc sp
 * success: return 0
 */
int ffa_ddrc_send_msg(struct ffa_send_direct_data *args)
{
	int ret = g_ffa_ddrc_ops->sync_send_receive(
				     g_ffa_ddrc_dev,
				     args);

	return ret;
}

/*
 * send message to ddrc sp with share memory
 * only arg0-arg2 can be used
 * local buffer size: u8 local_buf[FFA_LOCAL_BUFFER_MAX]
 * success: return 0
 */
int ffa_ddrc_send_msg_with_shmem(struct ffa_send_direct_data *args,
				 u8 *local_buf, u64 dir)
{
	int ret;

	mutex_lock(&g_local_buf_lock);
	ret = g_ffa_ddrc_ops->sync_send_receive_with_shmem(
				 g_ffa_ddrc_dev, args,
				 (void *)local_buf, dir);
	mutex_unlock(&g_local_buf_lock);

	return ret;
}

subsys_initcall(ddrc_ffa_init);
