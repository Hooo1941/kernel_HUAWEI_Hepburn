/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description: rpmbmspc ffa interface for kernel redirect to secure partition
 * Create: 2023-09-08
 */

#include <linux/init.h>
#include <linux/mutex.h>
#include <platform_include/see/ffa/ffa_rpmbmspc.h>

const struct ffa_ops *g_ffa_rpmbmspc_ops = NULL;
struct ffa_device *g_ffa_rpmbmspc_dev = NULL;
static struct mutex g_local_buf_lock;

const struct ffa_device_id g_rpmbmspc_ffa_device_id[] = {
	/* uuid = <0x165b35eb 0x9a8c475d 0x8810f319 0xe801572b> */
	{{{0xeb, 0x35, 0x5b, 0x16, 0x5d, 0x47, 0x8c, 0x9a,
	   0x19, 0xf3, 0x10, 0x88, 0x2b, 0x57, 0x01, 0xe8}}},
	{{{}}}
};

static void rpmbmspc_ffa_remove(struct ffa_device *ffa_dev)
{
	(void)ffa_dev;
	pr_err("rpmbmspc: rpmbmspc_ffa_remove remove\n");
}

static int rpmbmspc_ffa_probe(struct ffa_device *ffa_dev)
{
	g_ffa_rpmbmspc_dev = ffa_dev;
	if (!g_ffa_rpmbmspc_dev) {
		pr_err("rpmbmspc: failed to get ffa_dev!\n");
		return -ENOENT;
	}

	g_ffa_rpmbmspc_ops = ffa_dev->ops;
	if (!g_ffa_rpmbmspc_ops) {
		pr_err("rpmbmspc: fail to get ffa_ops\n");
		return -ENOENT;
	}

	g_ffa_rpmbmspc_ops->mode_32bit_set(ffa_dev);
	pr_err("rpmbmspc ffa probe OK!\n");
	return 0;
}

static struct ffa_driver g_rpmbmspc_ffa_driver = {
	.name = "RPMBMSPC_FFA",
	.probe = rpmbmspc_ffa_probe,
	.remove = rpmbmspc_ffa_remove,
	.id_table = g_rpmbmspc_ffa_device_id,
};

static int rpmbmspc_ffa_init(void)
{
	pr_err("rpmbmspc ffa init\n");
	mutex_init(&g_local_buf_lock);
	return ffa_register(&g_rpmbmspc_ffa_driver);
}

int ffa_rpmbmspc_send_msg_with_args(struct ffa_send_direct_data *args)
{
	int ret;

	if (g_ffa_rpmbmspc_ops == NULL || g_ffa_rpmbmspc_dev == NULL) {
		pr_err("rpmbmspc : ffa not init\n");
		return -1;
	}

	ret = g_ffa_rpmbmspc_ops->sync_send_receive(g_ffa_rpmbmspc_dev, args);

	return ret;
}

int ffa_rpmbmspc_send_msg(u64 arg0, u64 arg1, u64 arg2, u64 arg3)
{
	int ret;
	struct ffa_send_direct_data args = {
		.data0 = arg0,
		.data1 = arg1,
		.data2 = arg2,
		.data3 = arg3,
		.data4 = 0,
	};

	if (g_ffa_rpmbmspc_ops == NULL || g_ffa_rpmbmspc_dev == NULL) {
		pr_err("rpmbmspc:ffa not init\n");
		return -1;
	}

	ret = g_ffa_rpmbmspc_ops->sync_send_receive(g_ffa_rpmbmspc_dev, &args);
	if (ret != 0) {
		pr_err("%s: sync_send_receive fail:%d\n", __func__, ret);
		return -1;
	}

	return (int)args.data1;
}

/*
 * send message to rpmbmspc sp with share memory
 * only arg0-arg2 can be used
 * local buffer size: u8 local_buf[FFA_LOCAL_BUFFER_MAX]
 * success: return 0
 */
int ffa_rpmbmspc_send_msg_with_shmem(struct ffa_send_direct_data *args,
				     u8 *local_buf, u64 dir)
{
	int ret;

	if (g_ffa_rpmbmspc_ops == NULL || g_ffa_rpmbmspc_dev == NULL) {
		pr_err("rpmbmspc:ffa not init\n");
		return -1;
	}

	mutex_lock(&g_local_buf_lock);
	ret = g_ffa_rpmbmspc_ops->sync_send_receive_with_shmem(
				  g_ffa_rpmbmspc_dev, args,
				  (void *)local_buf, dir);
	mutex_unlock(&g_local_buf_lock);

	return ret;
}

subsys_initcall(rpmbmspc_ffa_init);
