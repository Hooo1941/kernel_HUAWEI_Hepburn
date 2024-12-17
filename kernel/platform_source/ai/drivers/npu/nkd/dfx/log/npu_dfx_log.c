/*
 * npu_dfx_log.c
 *
 * about npu dfx log
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
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
#include "npu_dfx_log.h"

#include <linux/uaccess.h>
#include <linux/list.h>
#include "npu_log.h"
#include "npu_dfx.h"
#include "npu_dfx_cq.h"
#include "npu_dfx_sq.h"
#include "npu_common.h"
#include "npu_manager.h"
#include "npu_platform.h"
#include "npu_ioctl_services.h"
#include "npu_mailbox_msg.h"
#include <securec.h>
#include "npd_interface.h"

#ifndef STATIC
#define STATIC static
#endif

static const char *char_driver_name = "log_drv";

log_char_dev_global_info_t g_log_dev_info;

static int log_get_channel_index(int channel_id, unsigned int *channel_idx)
{
	int ret = 0;

	if (channel_id == LOG_CHANNEL_TS_ID)
		*channel_idx = LOG_CHANNEL_TS_IDX;
	else
		ret = -1;

	return ret;
}

STATIC char *log_devnode(struct device *dev, umode_t *mode)
{
	unused(dev);
	if (mode != NULL)
		*mode = 0666;
	return NULL;
}

static int log_get_channel_share_memory(struct log_channel_info_t *channel_info)
{
	struct npu_dfx_desc *dfx_desc = NULL;
	struct npu_platform_info *plat_info = NULL;
	unsigned long can_use_size;

	if (channel_info->channel_id != LOG_CHANNEL_TS_ID) {
		npu_drv_err("channel_id(%u) is invalid\n", channel_info->channel_id);
		return IDE_DRV_ERROR;
	}

	plat_info = npu_plat_get_info();
	if (plat_info == NULL) {
		npu_drv_err("get platform info fail\n");
		return IDE_DRV_ERROR;
	}

	dfx_desc = &plat_info->resmem_info.dfx_desc[NPU_DFX_DEV_LOG];
	channel_info->phy_addr = dfx_desc->bufs->base;
	// a half memory use ts, other half memory use ai core
	can_use_size = (dfx_desc->bufs->len) >> 1;

	channel_info->buf_size = (unsigned int)can_use_size;
	channel_info->vir_addr = (unsigned char *)ioremap_wc(
		channel_info->phy_addr, can_use_size);
	if (channel_info->vir_addr == NULL) {
		npu_drv_err("ioremap_wc failed\n");
		return IDE_DRV_ERROR;
	}

	npu_drv_debug("phy_addr = %pK, vir_addr = %pK, buf_size : %x\n",
		(void *)(uintptr_t)channel_info->phy_addr,
		(void *)(uintptr_t)channel_info->vir_addr,
		channel_info->buf_size);

	return IDE_DRV_OK;
}

static int log_channel_init(void)
{
	unsigned int channel_idx = LOG_CHANNEL_NUM;
	log_channel_info_t *channel_info = NULL;
	log_buf_ptr_t *buf_ptr = NULL;
	int ret = 0;

	for (channel_idx = 0; channel_idx < LOG_CHANNEL_NUM; channel_idx++) {
		channel_info =
			(log_channel_info_t *)&g_log_dev_info.log_channel[channel_idx];

		mutex_lock(&channel_info->cmd_mutex);
		if (channel_idx == LOG_CHANNEL_TS_IDX)
			channel_info->channel_id = LOG_CHANNEL_TS_ID;

		if (channel_info->vir_addr == NULL) {
			ret = log_get_channel_share_memory(channel_info);
			if (ret != IDE_DRV_OK) {
				npu_drv_err("get shm_memory failed!\n");
				mutex_unlock(&channel_info->cmd_mutex);
				return IDE_DRV_ERROR;
			}
		}

		channel_info->channel_state = LOG_CHANNEL_ENABLE;
		channel_info->poll_flag = POLL_INVALID;
		channel_info->channel_type = 0;
		channel_info->status = 0;
		buf_ptr = (log_buf_ptr_t *)(channel_info->vir_addr);
		buf_ptr->buf_read  = 0;
		buf_ptr->buf_write = 0;
		buf_ptr->buf_len = channel_info->buf_size;
		buf_ptr->log_level = LOG_LEVEL_DEFAULT;
		init_waitqueue_head(&channel_info->log_wait);

		mutex_unlock(&channel_info->cmd_mutex);
	}

	return IDE_DRV_OK;
}

static void log_channel_release(void)
{
	unsigned int channel_idx;
	log_channel_info_t *channel_info = NULL;

	for (channel_idx = 0; channel_idx < LOG_CHANNEL_NUM; channel_idx++) {
		channel_info =
			(log_channel_info_t *)&g_log_dev_info.log_channel[channel_idx];

		mutex_lock(&channel_info->cmd_mutex);
		channel_info->channel_state = LOG_CHANNEL_DISABLE;
		if (channel_info->vir_addr != NULL) {
			iounmap((void *)channel_info->vir_addr);
			channel_info->vir_addr = NULL;
		}
		mutex_unlock(&channel_info->cmd_mutex);
	}
}

static int log_set(log_ioctl_para_t *log_para)
{
	log_channel_info_t *channel_info = NULL;
	int ret;
	unsigned int channel_idx = LOG_CHANNEL_NUM;
	log_buf_ptr_t *buf_ptr = NULL;

	ret = log_check_ioctl_para(log_para);
	if (ret != IDE_DRV_OK) {
		npu_drv_err("para invalid\n");
		return IDE_DRV_ERROR;
	}

	if (log_get_channel_index(log_para->channel_id, &channel_idx)) {
		npu_drv_err("get log channel index fail. channel_id: %u\n",
			log_para->channel_id);
		return IDE_DRV_ERROR;
	}

	channel_info =
		(log_channel_info_t *)&g_log_dev_info.log_channel[channel_idx];

	mutex_lock(&channel_info->cmd_mutex);
	if (channel_info->channel_state != LOG_CHANNEL_ENABLE) {
		npu_drv_err("Channel:%d not create\n", log_para->channel_id);
		mutex_unlock(&channel_info->cmd_mutex);
		return IDE_DRV_ERROR;
	}

	if (channel_info->vir_addr == NULL) {
		npu_drv_err("Channel:%d vir addr error\n", log_para->channel_id);
		mutex_unlock(&channel_info->cmd_mutex);
		return IDE_DRV_ERROR;
	}

	buf_ptr = (log_buf_ptr_t *)(channel_info->vir_addr);
	buf_ptr->log_level = log_para->log_level;
	channel_info->ioctl_para.log_level = log_para->log_level;

	mutex_unlock(&channel_info->cmd_mutex);
	return IDE_DRV_OK;
}

static int log_read_ringbuffer(log_ioctl_para_t *log_para, unsigned int buf_read,
	unsigned int buf_write, log_channel_info_t *channel_info)
{
	unsigned int buf_len = 0;
	unsigned int copy_len = 0;
	unsigned int buf_remain = 0;
	unsigned char *base = NULL;
	unsigned char *copy_from = NULL;
	log_buf_ptr_t *buf_ptr = NULL;
	char *out_buf = log_para->out_buf;

	buf_ptr = (log_buf_ptr_t *)channel_info->vir_addr;
	// Log content base address
	base = channel_info->vir_addr + DATA_BUF_HEAD;
	buf_len = (unsigned int)(channel_info->buf_size - DATA_BUF_HEAD);

	// 4-byte alignment is written on the TSCH side. The alignment may be deleted here.
	if (buf_read < buf_write) {
		copy_from = base + buf_read;
		copy_len = buf_write - buf_read;
		copy_len = log_para->buf_size <= copy_len ? log_para->buf_size : copy_len;
		if (copy_to_user_safe((void __user *)out_buf, (void *)copy_from, copy_len) != 0) {
			npu_drv_warn("copy log fail\n");
			return IDE_DRV_ERROR;
		}
		atomic_set((atomic_t *)&buf_ptr->buf_read, (int)(buf_read + copy_len));
		log_para->buf_size = copy_len;
		log_para->ret = IDE_DRV_OK;
		return IDE_DRV_OK;
	}

	copy_from = base + buf_read;
	copy_len = buf_len - buf_read;
	// read > write scene, just cpoy tail section
	if (log_para->buf_size <= copy_len) {
		copy_len = log_para->buf_size;
		if (copy_to_user_safe((void __user *)out_buf, (void *)copy_from, copy_len) != 0) {
			npu_drv_warn("copy log fail\n");
			return IDE_DRV_ERROR;
		}
		atomic_set((atomic_t *)&buf_ptr->buf_read, (int)(buf_read + copy_len) % buf_len);
		log_para->buf_size = copy_len;
		log_para->ret = IDE_DRV_OK;
		return IDE_DRV_OK;
	}

	// read > write scene, cpoy tail + head section
	if (copy_to_user_safe((void __user *)out_buf, (void *)copy_from, copy_len) != 0) {
		npu_drv_warn("copy log fail\n");
		return IDE_DRV_ERROR;
	}
	buf_remain = log_para->buf_size - copy_len;
	buf_remain = buf_remain <= buf_write ? buf_remain : buf_write;
	if (buf_remain != 0) {
		if (copy_to_user_safe((void __user *)(out_buf + copy_len),
			(void *)base, buf_remain) != 0) {
			npu_drv_warn("copy log fail\n");
			return IDE_DRV_ERROR;
		}
	}
	atomic_set((atomic_t *)&buf_ptr->buf_read, (int)buf_remain);
	log_para->buf_size = copy_len + buf_remain;
	log_para->ret = IDE_DRV_OK;
	return IDE_DRV_OK;
}

static int log_start(log_ioctl_para_t *log_para)
{
	unsigned int channel_idx = LOG_CHANNEL_NUM;
	log_channel_info_t *channel_info = NULL;

	if (log_get_channel_index(log_para->channel_id, &channel_idx)) {
		npu_drv_err("get log channel index fail. channel_id: %u\n",
			log_para->channel_id);
		return IDE_DRV_ERROR;
	}

	channel_info = (log_channel_info_t *)&g_log_dev_info.log_channel[channel_idx];
	channel_info->status = 1;

	log_para->ret = IDE_DRV_OK;
	return IDE_DRV_OK;
}

static int log_stop(log_ioctl_para_t *log_para)
{
	unsigned int channel_idx = LOG_CHANNEL_NUM;
	log_channel_info_t *channel_info = NULL;

	if (log_get_channel_index(log_para->channel_id, &channel_idx)) {
		npu_drv_err("get log channel index fail. channel_id: %u\n",
			log_para->channel_id);
		return IDE_DRV_ERROR;
	}

	channel_info = (log_channel_info_t *)&g_log_dev_info.log_channel[channel_idx];
	channel_info->status = 0;
	wake_up(&channel_info->log_wait);

	log_para->ret = IDE_DRV_OK;
	return IDE_DRV_OK;
}

int log_wakeup(unsigned int channel_id)
{
	unsigned int channel_idx = LOG_CHANNEL_NUM;
	log_channel_info_t *channel_info = NULL;

	if (log_get_channel_index((int)channel_id, &channel_idx)) {
		npu_drv_err("get log channel index fail. channel_id: %u\n", channel_id);
		return IDE_DRV_ERROR;
	}

	channel_info = (log_channel_info_t *)&g_log_dev_info.log_channel[channel_idx];
	wake_up(&channel_info->log_wait);
	return IDE_DRV_OK;
}

static void log_wait_event(log_channel_info_t *channel_info)
{
	struct npu_dev_ctx *cur_dev_ctx = NULL;

	cur_dev_ctx = get_dev_ctx_by_id(0);
	wait_event_interruptible(channel_info->log_wait,
		cur_dev_ctx->pm.ts_work_status == NPU_TS_WORK || channel_info->status == 0);
}

static int log_read(log_ioctl_para_t *log_para)
{
	int channel_id;
	unsigned int buf_read;
	unsigned int buf_write;
	unsigned int buf_len = 0;
	log_buf_ptr_t *buf_ptr = NULL;
	log_channel_info_t *channel_info = NULL;
	unsigned int channel_idx = LOG_CHANNEL_NUM;

	int ret = log_check_ioctl_para(log_para);
	log_para->ret = IDE_DRV_ERROR;
	cond_return_error(ret != IDE_DRV_OK, IDE_DRV_ERROR, "para invalid\n");
	cond_return_error(log_para->out_buf == NULL, IDE_DRV_ERROR, "out buf is null\n");
	cond_return_error(log_para->buf_size == 0, IDE_DRV_ERROR, "out buff size is 0\n");

	channel_id = log_para->channel_id;
	cond_return_error(log_get_channel_index(channel_id, &channel_idx),
		IDE_DRV_ERROR, "get log channel index fail. channel_id: %u\n",
		channel_id);

	channel_info = (log_channel_info_t *)&g_log_dev_info.log_channel[channel_idx];

	mutex_lock(&channel_info->cmd_mutex);
	cond_goto_error(channel_info->channel_state != LOG_CHANNEL_ENABLE,
		fail_free, ret, IDE_DRV_ERROR,
		"Channel:%d not create\n", channel_id);

	cond_goto_error(channel_info->vir_addr == NULL, fail_free, ret,
		IDE_DRV_ERROR, "Channel:%d vir_addr is null\n", channel_id);

	buf_ptr = (log_buf_ptr_t *)channel_info->vir_addr;
	buf_read = buf_ptr->buf_read;
	buf_write = (unsigned int)atomic_read((atomic_t *)&buf_ptr->buf_write);
	buf_len = (unsigned int)(channel_info->buf_size - DATA_BUF_HEAD);
	/* print_num is set to control log's print for every channel; and always ret; */
	if (buf_write >= buf_len || buf_read >= buf_len) {
		if (channel_info->print_num < LOG_PRINT_MAX) {
			npu_drv_err("log's buffer is error. channel_id = %d, len = %lu, read = %u, write = %u\n",
				channel_id, buf_len, buf_read, buf_write);
			channel_info->print_num++;
		}
		ret = IDE_DRV_ERROR;
		goto fail_free;
	}

	cond_goto_nolog(buf_write == buf_read, fail_setlog_free, ret, IDE_DRV_OK);
	ret = log_read_ringbuffer(log_para, buf_read, buf_write, channel_info);
	if (ret != IDE_DRV_OK) {
		npu_drv_warn("read log ringbuffer error, ret = %d, channel_id = %d, buf_read = %u, buf_write = %u\n",
			ret, channel_id, buf_read, buf_write);
		goto fail_free;
	}

	mutex_unlock(&channel_info->cmd_mutex);
	return IDE_DRV_OK;

fail_setlog_free:
	log_para->buf_size = 0;
	log_para->ret = IDE_DRV_OK;
fail_free:
	mutex_unlock(&channel_info->cmd_mutex);
	mutex_unlock(&g_log_dev_info.proc_ctx_mutex);
	log_wait_event(channel_info);
	mutex_lock(&g_log_dev_info.proc_ctx_mutex);

	return ret;
}

int log_tsch_dump(char *buff, uint32_t len)
{
	unsigned int buf_len = 0;
	log_buf_ptr_t *buf_ptr = NULL;
	unsigned char *base = NULL;
	unsigned int buf_read;
	unsigned int buf_write;
	errno_t rc = EOK;

	/* dump all tsch logs (128K) */
	log_channel_info_t *channel_info = (log_channel_info_t *)&g_log_dev_info.log_channel[LOG_CHANNEL_TS_IDX];
	cond_return_error(len == 0, IDE_DRV_ERROR, "output len = 0\n");
	cond_return_error(channel_info->channel_state != LOG_CHANNEL_ENABLE, IDE_DRV_ERROR, "tsch log not create\n");
	cond_return_error(channel_info->vir_addr == NULL, IDE_DRV_ERROR, "tsch log vir_addr is null\n");
	cond_return_error(channel_info->buf_size <= DATA_BUF_HEAD, IDE_DRV_ERROR,
		"channel buf_size = 0x%x is invalid\n", channel_info->buf_size);

	buf_ptr = (log_buf_ptr_t *)channel_info->vir_addr;
	buf_len = (unsigned int)(channel_info->buf_size - DATA_BUF_HEAD);
	buf_write = buf_ptr->buf_write;

	cond_return_error(buf_write >= buf_len, IDE_DRV_ERROR, "tsch log vir_addr is null\n");
	cond_return_error(len >= buf_len, IDE_DRV_ERROR, "output len=0x%x >= buf_len=0x%x\n", len, buf_len);

	buf_read = (buf_write + buf_len - len) % buf_len;
	base = channel_info->vir_addr + DATA_BUF_HEAD;

	if (buf_read <= buf_write) {
		rc = memcpy_s(buff, buf_write - buf_read, base + buf_read, buf_write - buf_read);
		if (rc != EOK)
			npu_drv_err("memcpy_s failed\n");
	} else {
		rc = memcpy_s(buff, buf_len - buf_read, base + buf_read, buf_len - buf_read);
		if (rc != EOK)
			npu_drv_err("memcpy_s failed\n");
		rc = memcpy_s(buff + buf_len - buf_read, buf_write, base, buf_write);
		if (rc != EOK)
			npu_drv_err("memcpy_s failed\n");
	}

	return IDE_DRV_OK;
}

int log_check_ioctl_para(const struct log_ioctl_para_t *log_para)
{
	struct npu_platform_info *plat_info = NULL;
	struct npu_dfx_desc *dfx_desc = NULL;
	u8 i;

	if (log_para == NULL) {
		npu_drv_err("NULL para\n");
		return IDE_DRV_ERROR;
	}

	plat_info = npu_plat_get_info();
	if (plat_info == NULL) {
		npu_drv_err("get platform info fail\n");
		return IDE_DRV_ERROR;
	}

	dfx_desc = &plat_info->resmem_info.dfx_desc[NPU_DFX_DEV_LOG];
	if (dfx_desc->channel_num > NPU_DFX_CHANNEL_MAX_RESOURCE) {
		npu_drv_err("channel_num:%d cofig error\n", dfx_desc->channel_num);
		return IDE_DRV_ERROR;
	}

	for (i = 0; i < dfx_desc->channel_num; i++) {
		if (dfx_desc->channels[i] == (u32)(log_para->channel_id))
			break;
	}

	if (i == dfx_desc->channel_num) {
		npu_drv_err("channel_id:%d create invalid\n", log_para->channel_id);
		return IDE_DRV_ERROR;
	}

	return 0;
}

static long log_drv_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int ret = 0;
	unsigned int drv_cmd;
	struct log_ioctl_para_t log_para = {0};
	log_char_dev_proc_info_t *proc_ctx = NULL;

	proc_ctx = (log_char_dev_proc_info_t *)file->private_data;
	cond_return_error(proc_ctx == NULL, IDE_DRV_ERROR, "get proc_ctx fail\n");

	if (copy_from_user_safe(&log_para, (void __user *)(uintptr_t)arg, sizeof(log_para))) {
		npu_drv_err("copy from user error\n");
		return IDE_DRV_ERROR;
	}

	drv_cmd = _IOC_NR(cmd);
	mutex_lock(&g_log_dev_info.proc_ctx_mutex);
	switch (drv_cmd) {
	case LOG_SET:
		ret = log_set((log_ioctl_para_t *)&log_para);
		break;
	case LOG_READ:
		ret = (atomic_read(&proc_ctx->is_enable) != 0) ? log_read((log_ioctl_para_t *)&log_para) : IDE_DRV_OK;
		break;
	case LOG_START:
		ret = log_start((log_ioctl_para_t *)&log_para);
		break;
	case LOG_STOP:
		ret = log_stop((log_ioctl_para_t *)&log_para);
		break;
	default:
	    mutex_unlock(&g_log_dev_info.proc_ctx_mutex);
		npu_drv_err("log ioctl cmd:%d illegal\n", drv_cmd);
		return IDE_DRV_ERROR;
	}
	mutex_unlock(&g_log_dev_info.proc_ctx_mutex);

	if (copy_to_user_safe((void __user *)(uintptr_t)arg, &log_para, sizeof(log_para)) != 0) {
		npu_drv_err("copy to user error\n");
		return IDE_DRV_ERROR;
	}

	return ret;
}

static int log_drv_open(struct inode *inode, struct file *filp)
{
	log_char_dev_proc_info_t *cur_proc_ctx = NULL;
	unused(inode);

	cur_proc_ctx = kzalloc(sizeof(log_char_dev_proc_info_t), GFP_KERNEL);
	cond_return_error(cur_proc_ctx == NULL, IDE_DRV_ERROR, "alloc memory for proc_ctx failed\n");

	mutex_lock(&g_log_dev_info.proc_ctx_mutex);
	atomic_set(&cur_proc_ctx->is_enable, false);
	if (list_empty_careful(&g_log_dev_info.proc_ctx_list))
		atomic_set(&cur_proc_ctx->is_enable, true);
	list_add(&cur_proc_ctx->dev_ctx_list, &g_log_dev_info.proc_ctx_list);
	mutex_unlock(&g_log_dev_info.proc_ctx_mutex);

	filp->private_data = (void *)cur_proc_ctx;

	return IDE_DRV_OK;
}

static int log_drv_release(struct inode *inode, struct file *filp)
{
	log_char_dev_proc_info_t *cur_proc_ctx = NULL;
	log_char_dev_proc_info_t *proc_ctx = NULL;
	struct list_head *pos = NULL;
	struct list_head *n = NULL;
	unused(inode);

	cur_proc_ctx = (log_char_dev_proc_info_t *)filp->private_data;
	cond_return_error(cur_proc_ctx == NULL, IDE_DRV_ERROR, "get proc_ctx fail\n");

	mutex_lock(&g_log_dev_info.proc_ctx_mutex);
	list_for_each_safe(pos, n, &g_log_dev_info.proc_ctx_list) {
		if (&cur_proc_ctx->dev_ctx_list == pos) {
			list_del(pos);
			break;
		}
	}
	if ((atomic_read(&cur_proc_ctx->is_enable) != 0) && (list_empty_careful(&g_log_dev_info.proc_ctx_list) == 0)) {
		proc_ctx = list_first_entry(&g_log_dev_info.proc_ctx_list, log_char_dev_proc_info_t, dev_ctx_list);
		if (proc_ctx != NULL)
			atomic_set(&proc_ctx->is_enable, true);
	}
	mutex_unlock(&g_log_dev_info.proc_ctx_mutex);

	kfree(cur_proc_ctx);
	filp->private_data = NULL;

	return IDE_DRV_OK;
}

static int log_drv_init(unsigned short dev_id)
{
	int channel_idx;
	struct log_device_info_t *log_dev_info = &(g_log_dev_info.log_dev_info);

	log_dev_info->device_state = DEV_UNUSED;
	log_dev_info->cmd_verify = 1;
	log_dev_info->device_id = dev_id;
	log_dev_info->poll_head = 0;
	log_dev_info->poll_tail = 0;

	g_log_dev_info.poll_box = kzalloc(
		sizeof(log_poll_info_t) * LOG_POLL_DEPTH, GFP_KERNEL);
	if (g_log_dev_info.poll_box == NULL) {
		npu_drv_err("pollbox kzalloc is error\n");
		return IDE_DRV_ERROR;
	}

	(void)memset_s(g_log_dev_info.log_channel,
		sizeof(log_channel_info_t) * LOG_CHANNEL_NUM,
		0, sizeof(log_channel_info_t) * LOG_CHANNEL_NUM);

	for (channel_idx = 0; channel_idx < LOG_CHANNEL_NUM; channel_idx++)
		mutex_init(&g_log_dev_info.log_channel[channel_idx].cmd_mutex);

	INIT_LIST_HEAD(&g_log_dev_info.proc_ctx_list);
	mutex_init(&g_log_dev_info.proc_ctx_mutex);

	return IDE_DRV_OK;
}

static void log_drv_uninit(void)
{
	mutex_destroy(&g_log_dev_info.proc_ctx_mutex);

	(void)memset_s(g_log_dev_info.log_channel,
		sizeof(log_channel_info_t) * LOG_CHANNEL_NUM,
		0, sizeof(log_channel_info_t) * LOG_CHANNEL_NUM);

	if (g_log_dev_info.poll_box != NULL) {
		kfree((void *)g_log_dev_info.poll_box);
		g_log_dev_info.poll_box = NULL;
	}
}

const struct file_operations log_drv_fops = {
	.owner = THIS_MODULE,
	.open = log_drv_open,
	.release = log_drv_release,
	.unlocked_ioctl = log_drv_ioctl,
	.compat_ioctl = log_drv_ioctl,
};

static int log_drv_register_cdev(void)
{
	int ret;
	unsigned int major;
	dev_t devno;

	struct char_device *priv =
		(struct char_device *)&g_log_dev_info.char_dev_log;

	priv->devno = 0;
	ret = alloc_chrdev_region(&priv->devno, 0, 1, char_driver_name);
	if (ret < 0)
		return IDE_DRV_ERROR;

	/* init and add char device */
	major = MAJOR(priv->devno);
	devno = MKDEV(major, 0);
	cdev_init(&priv->cdev, &log_drv_fops);
	priv->cdev.owner = THIS_MODULE;
	priv->cdev.ops = &log_drv_fops;

	if (cdev_add(&priv->cdev, devno, 1)) {
		unregister_chrdev_region(devno, 1);
		return IDE_DRV_ERROR;
	}

	priv->dev_class = class_create(THIS_MODULE, char_driver_name);
	if (IS_ERR(priv->dev_class)) {
		(void)unregister_chrdev_region(devno, 1);
		(void)cdev_del(&priv->cdev);
		return IDE_DRV_ERROR;
	}
	if (priv->dev_class != NULL)
		priv->dev_class->devnode = log_devnode;

	g_log_dev_info.log_dev = device_create(priv->dev_class, NULL, devno,
		NULL, "%s", char_driver_name);
	if (g_log_dev_info.log_dev == NULL) {
		npu_drv_err("device create error.devno = %d\n", devno);
		(void)class_destroy(priv->dev_class);
		(void)unregister_chrdev_region(devno, 1);
		(void)cdev_del(&priv->cdev);
		return IDE_DRV_ERROR;
	}

	return IDE_DRV_OK;
}

static void log_drv_free_cdev(void)
{
	struct char_device *priv = &(g_log_dev_info.char_dev_log);
	(void)device_destroy(priv->dev_class, priv->devno);
	(void)class_destroy(priv->dev_class);
	(void)unregister_chrdev_region(priv->devno, 1);
	(void)cdev_del(&priv->cdev);
}

int log_drv_module_init(void)
{
	int ret = IDE_DRV_OK;
	const unsigned short dev_id = 0;

	(void)memset_s(&g_log_dev_info, sizeof(log_char_dev_global_info_t), 0,
		sizeof(log_char_dev_global_info_t));

	ret = log_drv_register_cdev();
	if (ret != IDE_DRV_OK) {
		npu_drv_err("create character device fail\n");
		return IDE_DRV_ERROR;
	}

	ret = log_drv_init(dev_id);
	if (ret != IDE_DRV_OK) {
		log_drv_free_cdev();
		npu_drv_err("log drv init fail\n");
		return IDE_DRV_ERROR;
	}

	ret = log_channel_init();
	if (ret != IDE_DRV_OK) {
		log_drv_uninit();
		log_drv_free_cdev();
		npu_drv_err("log drv init fail\n");
		return IDE_DRV_ERROR;
	}

	npu_drv_info("device (%u) log drv load ok\n", dev_id);

	return IDE_DRV_OK;
}

void log_drv_module_exit(void)
{
	log_channel_release();

	log_drv_uninit();

	/* char device free at last */
	log_drv_free_cdev();

	npu_drv_info("device (%u) log drv exit\n",
		g_log_dev_info.log_dev_info.device_id);
}

