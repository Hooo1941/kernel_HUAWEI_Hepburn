/*
 * nn_dsp_route.c
 *
 * nn dsp route misc driver.
 *
 * Copyright (c) 2022-2022 Huawei Technologies Co., Ltd.
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

#include "nn_dsp_route.h"

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/workqueue.h>
#include <linux/errno.h>
#include <linux/version.h>
#include <linux/compiler_types.h>
#include <linux/wait.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <asm/io.h>
#include "drv_mailbox_msg.h"
#include "audio_log.h"
#include "audio_ddr_map.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "nn_dsp_route"

#define NN_DSP_ROUTE_CTRL_SIZE_MAX 3
#define NN_DSP_ROUTE_READ_TIMEOUT 40

struct dsp_route_write_output_msg {
	uint16_t msg_id;
};

struct dsp_route_work_info {
	void __iomem *input_mem;
	void __iomem *output_mem;
	uint64_t input_addr;
	uint32_t input_size;
	uint64_t output_addr;
	uint32_t output_size;

	spinlock_t read_lock;
	wait_queue_head_t read_waitq;
	int32_t rx_queue_cnt;
	bool is_dev_open;
	uint32_t queue_max_size;
};

struct dsp_route_data_buf {
	int32_t size;
	uint8_t *data;
};

struct dsp_route_data_node {
	struct list_head list_node;
	struct dsp_route_data_buf list_data;
};

static struct dsp_route_work_info g_work_info;
static LIST_HEAD(g_nn_dsp_route_rx_queue);
uint32_t g_read_dsp_timeout_count;

static int32_t create_dsp_route_data_node(struct dsp_route_data_node **node, uint8_t *data, int32_t size)
{
	struct dsp_route_data_node *new_node = NULL;

	new_node = kzalloc(sizeof(struct dsp_route_data_node), GFP_ATOMIC);
	if (new_node == NULL) {
		AUDIO_LOGE("node alloc error");
		return -ENOMEM;
	}

	new_node->list_data.data = data;
	new_node->list_data.size = size;
	*node = new_node;
	return 0;
}

static void free_dsp_route_data_node(struct dsp_route_data_node **node)
{
	kfree((*node)->list_data.data);
	kfree(*node);
	*node = NULL;
}

static void clear_dsp_route_queue(const struct dsp_route_work_info *work_info, struct list_head *dsp_route_rx_queue)
{
	uint32_t cnt = 0;
	struct dsp_route_data_node *node = NULL;

	while (list_empty_careful(dsp_route_rx_queue) == 0) {
		cnt++;
		if (cnt > work_info->queue_max_size) {
			AUDIO_LOGE("clear queue abnormal, cnt is %u", cnt);
			break;
		}

		node = list_first_entry(dsp_route_rx_queue, struct dsp_route_data_node, list_node);
		list_del_init(&node->list_node);
		kfree(node);
		node = NULL;
	}
	AUDIO_LOGI("clear queue cnt is %u", cnt);
}

static int32_t save_dsp_route_dma_input_data(struct dsp_route_work_info *work_info,
	struct list_head *dsp_route_rx_queue)
{
	int32_t ret = 0;
	uint8_t *input_data = NULL;
	struct dsp_route_data_node *node = NULL;

	spin_lock_bh(&work_info->read_lock);
	if (!work_info->is_dev_open) {
		spin_unlock_bh(&work_info->read_lock);
		AUDIO_LOGE("device is not opened");
		return -EFAULT;
	}

	if (work_info->rx_queue_cnt > work_info->queue_max_size) {
		spin_unlock_bh(&work_info->read_lock);
		wake_up(&work_info->read_waitq);
		return -ENOMEM;
	}

	input_data = kzalloc(work_info->input_size, GFP_ATOMIC);
	if (input_data == NULL) {
		spin_unlock_bh(&work_info->read_lock);
		return -ENOMEM;
	}
	(void)memcpy(input_data, work_info->input_mem, work_info->input_size);
	spin_unlock_bh(&work_info->read_lock);

	ret = create_dsp_route_data_node(&node, input_data, work_info->input_size);
	if (ret != 0) {
		AUDIO_LOGE("create node error");
		kfree(input_data);
		input_data = NULL;
		return -EFAULT;
	}

	spin_lock_bh(&work_info->read_lock);
	list_add_tail(&node->list_node, dsp_route_rx_queue);
	work_info->rx_queue_cnt++;
	spin_unlock_bh(&work_info->read_lock);
	wake_up(&work_info->read_waitq);
	return work_info->input_size;
}

static int32_t get_dsp_route_input_data(struct dsp_route_work_info *work_info, struct list_head *dsp_route_rx_queue,
	char *buf, size_t nbytes)
{
	int32_t ret = 0;
	struct dsp_route_data_node *node = NULL;

	spin_lock_bh(&work_info->read_lock);
	if (list_empty_careful(dsp_route_rx_queue)) {
		spin_unlock_bh(&work_info->read_lock);
		ret = wait_event_interruptible_timeout(work_info->read_waitq, (list_empty_careful(dsp_route_rx_queue) == false),
			msecs_to_jiffies(NN_DSP_ROUTE_READ_TIMEOUT));
		if (ret <= 0) {
			if (g_read_dsp_timeout_count == 100) {
				AUDIO_LOGE("wait timeout");
				g_read_dsp_timeout_count = 0;
			}
			g_read_dsp_timeout_count++;
			return -ETIMEDOUT;
		}

		spin_lock_bh(&work_info->read_lock);
	}

	if (list_empty_careful(dsp_route_rx_queue) == 0) {
		node = list_first_entry(dsp_route_rx_queue, struct dsp_route_data_node, list_node);
		list_del_init(&node->list_node);
		if (work_info->rx_queue_cnt > 0)
			work_info->rx_queue_cnt--;

		spin_unlock_bh(&work_info->read_lock);
		if (copy_to_user(buf, node->list_data.data, nbytes) != 0) {
			ret = -EFAULT;
		} else {
			ret = nbytes;
		}
		free_dsp_route_data_node(&node);
		spin_lock_bh(&work_info->read_lock);
	} else {
		ret = -EAGAIN;
	}

	spin_unlock_bh(&work_info->read_lock);
	return ret;
}

void rcv_nn_dsp_route_msg(enum socdsp_om_work_id work_id, const uint8_t *data, uint32_t len)
{
	int32_t ret = 0;

	ret = save_dsp_route_dma_input_data(&g_work_info, &g_nn_dsp_route_rx_queue);
	if (ret < 0)
		AUDIO_LOGE("save data failed %d", ret);
}

static void init_nn_dsp_route_work_info(void)
{
	g_work_info.input_mem = NULL;
	g_work_info.output_mem = NULL;
	g_work_info.input_addr = HIFI_NN_DSP_ROUTE_INPUT_ADDR;
	g_work_info.input_size = NN_DSP_ROUTE_INPUT_BUFFER_SIZE;
	g_work_info.output_addr = HIFI_NN_DSP_ROUTE_OUTPUT_ADDR;
	g_work_info.output_size = NN_DSP_ROUTE_OUTPUT_BUFFER_SIZE;
	g_work_info.rx_queue_cnt = 0;
	g_work_info.queue_max_size = NN_DSP_ROUTE_CTRL_SIZE_MAX;
	g_work_info.is_dev_open = false;

	g_read_dsp_timeout_count = 0;

	spin_lock_init(&g_work_info.read_lock);
	init_waitqueue_head(&g_work_info.read_waitq);
}

static int open_nn_dsp_route(struct inode *finode, struct file *fd)
{
	spin_lock_bh(&g_work_info.read_lock);
	if (g_work_info.is_dev_open) {
		spin_unlock_bh(&g_work_info.read_lock);
		AUDIO_LOGE("nn dsp route is already opened");
		return -EFAULT;
	}

	clear_dsp_route_queue(&g_work_info, &g_nn_dsp_route_rx_queue);
	g_work_info.rx_queue_cnt = 0;

	g_work_info.is_dev_open = true;
	spin_unlock_bh(&g_work_info.read_lock);
	return 0;
}

static int release_nn_dsp_route(struct inode *finode, struct file *fd)
{
	spin_lock_bh(&g_work_info.read_lock);
	if (!g_work_info.is_dev_open) {
		spin_unlock_bh(&g_work_info.read_lock);
		AUDIO_LOGE("nn dsp route is already closed");
		return -EFAULT;
	}
	g_work_info.is_dev_open = false;
	spin_unlock_bh(&g_work_info.read_lock);

	return 0;
}

static ssize_t read_nn_dsp_route(struct file *file, char __user *buf, size_t nbytes, loff_t *pos)
{
	if (buf == NULL || nbytes == 0 || nbytes > g_work_info.input_size) {
		AUDIO_LOGE("nn dsp route read parameters error");
		return -EINVAL;
	}

	spin_lock_bh(&g_work_info.read_lock);
	if (!g_work_info.is_dev_open) {
		spin_unlock_bh(&g_work_info.read_lock);
		AUDIO_LOGE("nn dsp route is closed");
		return -EFAULT;
	}
	spin_unlock_bh(&g_work_info.read_lock);

	return get_dsp_route_input_data(&g_work_info, &g_nn_dsp_route_rx_queue, buf, nbytes);
}

static void send_dsp_route_write_output_complete_msg(void)
{
	uint32_t ret = 0;
	struct dsp_route_write_output_msg msg = { 0 };
	msg.msg_id = ID_AP_DSP_NN_ROUTE_WRITE_COMPLETE;

	ret = mailbox_send_msg(MAILBOX_MAILCODE_ACPU_TO_HIFI_MISC, &msg, sizeof(msg));
	if (ret != 0)
		AUDIO_LOGE("send write output complete msg error %u", ret);
}

static ssize_t write_nn_dsp_route(struct file *file, const char __user *buf, size_t nbytes, loff_t *pos)
{
	spin_lock_bh(&g_work_info.read_lock);
	if (!g_work_info.is_dev_open) {
		spin_unlock_bh(&g_work_info.read_lock);
		AUDIO_LOGE("nn dsp route is closed");
		return -EFAULT;
	}
	spin_unlock_bh(&g_work_info.read_lock);

	if (buf == NULL || nbytes == 0 || nbytes > g_work_info.output_size) {
		AUDIO_LOGE("nn dsp route write parameters error");
		return -EINVAL;
	}

	if (copy_from_user(g_work_info.output_mem, buf, nbytes) != 0) {
		AUDIO_LOGE("nn dsp route write copy from user error");
		return -EFAULT;
	}

	send_dsp_route_write_output_complete_msg();
	return nbytes;
}

static const struct file_operations g_nn_dsp_route_fops = {
	.owner = THIS_MODULE,
	.open = open_nn_dsp_route,
	.release = release_nn_dsp_route,
	.read = read_nn_dsp_route,
	.write = write_nn_dsp_route,
};

static struct miscdevice g_nn_dsp_route_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "nn_dsp_route",
	.fops = &g_nn_dsp_route_fops,
};

void init_nn_dsp_route(void)
{
	int32_t ret = 0;
	IN_FUNCTION;
	AUDIO_LOGI("init_nn_dsp_route enter");

	ret = misc_register(&g_nn_dsp_route_device);
	if (ret != 0) {
		AUDIO_LOGE("misc register failed, ret %d", ret);
		return;
	}

	init_nn_dsp_route_work_info();

	g_work_info.input_mem = ioremap(g_work_info.input_addr, g_work_info.input_size);
	if (g_work_info.input_mem == NULL) {
		misc_deregister(&g_nn_dsp_route_device);
		AUDIO_LOGE("input_addr io remap error size %u", g_work_info.input_size);
		return;
	}

	g_work_info.output_mem = ioremap(g_work_info.output_addr, g_work_info.output_size);
	if (g_work_info.output_mem == NULL) {
		misc_deregister(&g_nn_dsp_route_device);
		iounmap(g_work_info.input_mem);
		g_work_info.input_mem = NULL;
		AUDIO_LOGE("output ioremap error size %u", g_work_info.output_size);
		return;
	}

	OUT_FUNCTION;
}

void deinit_nn_dsp_route(void)
{
	IN_FUNCTION;
	AUDIO_LOGI("deinit_nn_dsp_route enter");

	if (g_work_info.input_mem != NULL) {
		iounmap(g_work_info.input_mem);
		g_work_info.input_mem = NULL;
	}

	if (g_work_info.output_mem != NULL) {
		iounmap(g_work_info.output_mem);
		g_work_info.output_mem = NULL;
	}

	misc_deregister(&g_nn_dsp_route_device);

	OUT_FUNCTION;
}