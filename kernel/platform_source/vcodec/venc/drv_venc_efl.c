/*
 * drv_venc_efl.c
 *
 * This is for venc drv.
 *
 * Copyright (c) 2009-2020 Huawei Technologies CO., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include "drv_venc_efl.h"
#include "drv_mem.h"
#include "venc_regulator.h"
#include "hal_venc.h"

static struct venc_file_private g_venc_file_private[MAX_INSTANCES_COUNT];

struct venc_file_private* get_venc_file_private(void)
{
	return g_venc_file_private;
}

static int32_t wait_buffer_filled(struct venc_fifo_buffer *buffer)
{
	int32_t ret;
	uint32_t timeout = venc_regulator_is_fpga() ? FPGA_WAIT_EVENT_TIME_MS : ENCODE_DONE_TIMEOUT_MS;

	ret = OSAL_WAIT_EVENT_TIMEOUT(&buffer->event, !queue_is_empty(buffer), timeout); /*lint !e578 !e666*/
	if (ret != 0)
		VCODEC_ERR_VENC("wait buffer filled timeout");

	return ret;
}

static void venc_shared_fd_list_init(struct venc_fd_list *list)
{
	mutex_init(&list->lock);
	INIT_LIST_HEAD(&list->fdhead);
}

static void venc_shared_fd_list_deinit(struct venc_fd_list *list)
{
	struct venc_fd_listnode *pos = NULL;
	struct venc_fd_listnode *n = NULL;

	mutex_lock(&list->lock);
	list_for_each_entry_safe(pos, n, &list->fdhead, head) {
		VCODEC_ERR_VENC("deinit fd:%d,%d", pos->shared_fd, pos->refcnt);
		dma_buf_put(pos->dmabuf);
		list_del(&pos->head);
		vfree(pos);
	}
	mutex_unlock(&list->lock);
}

static int32_t venc_shared_fd_list_add(struct venc_fd_list *list, int32_t fd)
{
	const int refcnt_max = 1024;
	struct venc_fd_listnode *pos = NULL;
	struct venc_fd_listnode *node = NULL;

	if (fd < 0)
		return 0;

	mutex_lock(&list->lock);
	list_for_each_entry(pos, &list->fdhead, head) {
		if (pos->shared_fd == fd) {
			if (pos->refcnt >= refcnt_max) {
				mutex_unlock(&list->lock);
				VCODEC_ERR_VENC("fd %d maybe error", fd);
				return VCODEC_FAILURE;
			}
			pos->refcnt++;
			mutex_unlock(&list->lock);
			return 0;
		}
	}

	node = vmalloc(sizeof(struct venc_fd_listnode));
	if (!node) {
		mutex_unlock(&list->lock);
		VCODEC_ERR_VENC("malloc fail");
		return VCODEC_FAILURE;
	}
	node->dmabuf = dma_buf_get(fd);
	if (IS_ERR(node->dmabuf)) {
		mutex_unlock(&list->lock);
		vfree(node);
		VCODEC_ERR_VENC("dma get fail");
		return VCODEC_FAILURE;
	}

	node->shared_fd = fd;
	node->refcnt = 1;
	list_add_tail(&node->head, &list->fdhead);
	mutex_unlock(&list->lock);
	return 0;
}

static void venc_shared_fd_list_remove(struct venc_fd_list *list, int32_t fd)
{
	struct venc_fd_listnode *pos = NULL;
	struct venc_fd_listnode *n = NULL;

	if (fd < 0)
		return;

	mutex_lock(&list->lock);
	list_for_each_entry_safe(pos, n, &list->fdhead, head) {
		if (pos->shared_fd == fd) {
			pos->refcnt--;
			if (pos->refcnt <= 0) {
				dma_buf_put(pos->dmabuf);
				list_del(&pos->head);
				vfree(pos);
			}
			mutex_unlock(&list->lock);
			return;
		}
	}

	mutex_unlock(&list->lock);
}

int32_t venc_lock_shared_fd(struct venc_fd_list *list, venc_fd_info_t *info)
{
	int i, ret;
	ret = venc_shared_fd_list_add(list, info->interal_shared_fd);
	if (ret != 0)
		return ret;

	ret = venc_shared_fd_list_add(list, info->image_shared_fd);
	if (ret != 0)
		return ret;

	ret = venc_shared_fd_list_add(list, info->stream_head_shared_fd);
	if (ret != 0)
		return ret;

	for (i = 0; i < MAX_SLICE_NUM; ++i) {
		ret = venc_shared_fd_list_add(list, info->stream_shared_fd[i]);
		if (ret != 0)
			return ret;
	}

	return 0;
}

void venc_unlock_shared_fd(struct venc_fd_list *list, venc_fd_info_t *info)
{
	int i;
	venc_shared_fd_list_remove(list, info->interal_shared_fd);
	venc_shared_fd_list_remove(list, info->image_shared_fd);
	venc_shared_fd_list_remove(list, info->stream_head_shared_fd);
	for (i = 0; i < MAX_SLICE_NUM; ++i)
		venc_shared_fd_list_remove(list, info->stream_shared_fd[i]);
}

static int32_t get_mem_info_by_fd(int32_t fd, mem_buffer_t *buffer)
{
	int32_t ret;

	if (fd < 0) {
		VCODEC_FATAL_VENC("invalid param, share fd is NULL");
		return VCODEC_FAILURE;
	}

	ret = drv_mem_get_map_info(fd, buffer);
	if (ret) {
		VCODEC_FATAL_VENC("share fd map failed");
		return VCODEC_FAILURE;
	}

	drv_mem_put_map_info(buffer);

	return 0;
}

static int32_t get_mem_info(venc_fd_info_t *fd_info, venc_buffer_info_t *mem_info)
{
	int32_t ret;
	int32_t i;

	ret = get_mem_info_by_fd(fd_info->interal_shared_fd, &mem_info->internal_buffer);
	if (ret) {
		VCODEC_FATAL_VENC("Interal buffer map failed");
		return VCODEC_FAILURE;
	}

	ret = get_mem_info_by_fd(fd_info->image_shared_fd, &mem_info->image_buffer);
	if (ret) {
		VCODEC_FATAL_VENC("Image buffer map failed");
		return VCODEC_FAILURE;
	}

	ret = get_mem_info_by_fd(fd_info->stream_head_shared_fd, &mem_info->stream_head_buffer);
	if (ret) {
		VCODEC_FATAL_VENC("streamhead buffer map failed");
		return VCODEC_FAILURE;
	}

	for (i = 0; i < MAX_SLICE_NUM; i++) {
		if (fd_info->stream_shared_fd[i] == -1) {
			continue;
		}

		ret = get_mem_info_by_fd(fd_info->stream_shared_fd[i], &mem_info->stream_buffer[i]);
		if (ret) {
			VCODEC_FATAL_VENC("Stream buffer map failed");
			return VCODEC_FAILURE;
		}
	}

	return 0;
}

static int32_t check_buffer_addr(const S_HEVC_AVC_REGS_TYPE_CFG *reg, venc_fd_info_t *fd_info)
{
	int32_t ret;
	venc_buffer_info_t mem_info;

	(void)memset_s((void *)&mem_info, sizeof(mem_info), 0, sizeof(mem_info));

	ret = get_mem_info(fd_info, &mem_info);
	if (ret) {
		VCODEC_FATAL_VENC("mem shared error");
		return VCODEC_FAILURE;
	}

	ret = vedu_hal_check_addr(reg, &mem_info);
	if (ret) {
		VCODEC_FATAL_VENC("check buffer addr failed");
		return VCODEC_FAILURE;
	}

	return 0;
}

int32_t venc_drv_create_queue(void)
{
	int32_t i;
	int32_t j;

	for (i = 0; i < MAX_INSTANCES_COUNT; i++) {
		g_venc_file_private[i].encode_done_info = create_queue(struct venc_fifo_buffer);
		if (g_venc_file_private[i].encode_done_info == NULL)
			break;
	}

	if (i == MAX_INSTANCES_COUNT)
		return 0;

	for (j = 0; j < i; j++) {
		destory_queue(g_venc_file_private[j].encode_done_info);
		g_venc_file_private[j].encode_done_info = NULL;
	}

	VCODEC_FATAL_VENC("create queue failed");
	return VCODEC_FAILURE;
}

void venc_drv_destroy_queue(void)
{
	int32_t i;

	for (i = 0; i < MAX_INSTANCES_COUNT; i++) {
		if (g_venc_file_private[i].encode_done_info != NULL) {
			destory_queue(g_venc_file_private[i].encode_done_info);
			g_venc_file_private[i].encode_done_info = NULL;
		}
	}
}

int32_t venc_drv_alloc_file_private(struct file *file)
{
	int32_t i;
	int32_t ret;

	if (file == NULL) {
		VCODEC_ERR_VENC("input file is NULL");
		return VCODEC_FAILURE;
	}

	for (i = 0; i < MAX_INSTANCES_COUNT; i++) {
		if (queue_is_null(g_venc_file_private[i].encode_done_info))
			break;
	}

	if (i == MAX_INSTANCES_COUNT) {
		VCODEC_FATAL_VENC("failed to find idle kfifo");
		return VCODEC_FAILURE;
	}

	ret = alloc_queue(g_venc_file_private[i].encode_done_info, MAX_RING_BUFFER_SIZE);
	if (ret != 0) {
		VCODEC_FATAL_VENC("failed to allocate kfifo");
		return VCODEC_FAILURE;
	}

	venc_shared_fd_list_init(&g_venc_file_private[i].fdlist);

	file->private_data = &g_venc_file_private[i];

	return 0;
}

int32_t venc_drv_free_file_private(const struct file *file)
{
	int32_t i;

	if (file == NULL) {
		VCODEC_ERR_VENC("input file is NULL");
		return VCODEC_FAILURE;
	}

	for (i = 0; i < MAX_INSTANCES_COUNT; i++) {
		if (file->private_data == &g_venc_file_private[i]) {
			free_queue(g_venc_file_private[i].encode_done_info);
			g_venc_file_private[i].venc_perf_loads = 0;
			g_venc_file_private[i].venc_low_power_enable_flag = false;
			venc_shared_fd_list_deinit(&g_venc_file_private[i].fdlist);
			return 0;
		}
	}

	VCODEC_ERR_VENC("not find buffer info, free buffer fail");

	return VCODEC_FAILURE;
}

int32_t venc_drv_get_encode_done_info(struct venc_fifo_buffer *buffer, struct encode_done_info *encode_done_info)
{
	int32_t ret;

	if (buffer == NULL || encode_done_info == NULL) {
		VCODEC_ERR_VENC("input is invalid");
		return VCODEC_FAILURE;
	}

	ret = wait_buffer_filled(buffer);
	if (ret != 0) {
		VCODEC_ERR_VENC("wait buffer filled failed");
		return VCODEC_FAILURE;
	}

	ret = pop(buffer, encode_done_info);
	if (ret != 0) {
		VCODEC_ERR_VENC("read encode done info failed");
		return VCODEC_FAILURE;
	}

	return 0;
}

static int32_t venc_drv_update_channel_id(struct encode_info *encode_info, struct venc_file_private *buffer)
{
	uint32_t i;
	VCODEC_DBG_VENC("enter %s (): User channel ID = %d", __func__, encode_info->channel.id);

	for (i = 0; i < MAX_INSTANCES_COUNT; i++) {
		if (&g_venc_file_private[i] == buffer) {
			buffer->user_channel_id = encode_info->channel.id;
			buffer->venc_perf_loads = encode_info->clock_info.loads;
			buffer->venc_low_power_enable_flag =
				(encode_info->clock_info.low_power_enable_flag == 1 ? true : false);
			encode_info->channel.id = i;
			VCODEC_DBG_VENC("kernel channel ID = %d", encode_info->channel.id);
			return 0;
		}
	}
	VCODEC_DBG_VENC("cannot find this file private");
	return VCODEC_FAILURE;
}

int32_t venc_drv_encode(struct encode_info *encode_info, struct venc_file_private *buffer)
{
	uint32_t i;
	venc_entry_t *venc = platform_get_drvdata(venc_get_device());
	if (venc_drv_update_channel_id(encode_info, buffer))
		return VCODEC_FAILURE;

	if (check_buffer_addr(&encode_info->all_reg, &encode_info->channel.mem_info)) {
		VCODEC_FATAL_VENC("check buffer addr error");
		return VCODEC_FAILURE;
	}

	for (i = 0; i < MAX_SUPPORT_CORE_NUM && i < venc_get_core_num(); i++)
		pm_update_account(&venc->ctx[i].pm);

	return venc->ops.encode(encode_info, buffer->encode_done_info);
}

irqreturn_t venc_drv_encode_done(int32_t irq, void *dev_id)
{
	int32_t i = 0;
	venc_entry_t *venc = platform_get_drvdata(venc_get_device());

	VCODEC_DBG_VENC("enter %s ()", __func__);

	for (i = 0; i < MAX_SUPPORT_CORE_NUM; i++) {
		if (venc->ctx[i].irq_num.normal == (uint32_t)irq)
			break;
	}

	if (i == MAX_SUPPORT_CORE_NUM) {
		VCODEC_FATAL_VENC("isr not register");
		return IRQ_HANDLED;
	}

	if (venc->ctx[i].status != VENC_BUSY)
		VCODEC_FATAL_VENC("cord %d: isr invalid core status, status is %d",
				i, venc->ctx[i].status);

	venc->ops.encode_done(&venc->ctx[i]);
	VCODEC_DBG_VENC("out %s ()", __func__);
	return IRQ_HANDLED;
}

int32_t venc_drv_open_vedu(void)
{
	venc_entry_t *venc = platform_get_drvdata(venc_get_device());
	int32_t ret;

	VCODEC_DBG_VENC("enter %s()", __func__);

	(void)memset_s(&venc->backup_info.info, sizeof(struct clock_info), 0, sizeof(struct clock_info));
	ret = venc->ops.init();

	VCODEC_DBG_VENC("exit %s()", __func__);
	return ret;
}

int32_t venc_drv_close_vedu(void)
{
	venc_entry_t *venc = platform_get_drvdata(venc_get_device());

	VCODEC_DBG_VENC("enter %s()", __func__);

	venc_regulator_wait_hardware_idle(&venc->event);
	venc->ops.deinit();

	VCODEC_DBG_VENC("exit %s()", __func__);

	return 0;
}

int32_t venc_drv_suspend_vedu(void)
{
	venc_entry_t *venc = platform_get_drvdata(venc_get_device());

	VCODEC_INFO_VENC("enter %s()", __func__);

	venc_regulator_wait_hardware_idle(&venc->event);

	VCODEC_INFO_VENC("exit %s()", __func__);

	return 0;
}

int32_t venc_drv_resume_vedu(void)
{
	VCODEC_INFO_VENC("enter %s()", __func__);
	VCODEC_INFO_VENC("exit %s()", __func__);
	return 0;
}

inline int32_t venc_check_coreid(int32_t core_id)
{
	if (core_id < VENC_CORE_0 || (uint32_t)core_id >= venc_get_core_num())
		return VCODEC_FAILURE;

	return 0;
}

void venc_drv_init_pm(void)
{
	uint32_t i;
	uint32_t core_num = venc_get_core_num();
	venc_entry_t *venc = platform_get_drvdata(venc_get_device());



	if (!is_support_power_control_per_frame())
		return;

	for (i = 0; i < MAX_SUPPORT_CORE_NUM && i < core_num; i++)
		pm_init_account(&venc->ctx[i].pm);
}

void venc_drv_deinit_pm(void)
{
	uint32_t i;
	uint32_t core_num = venc_get_core_num();
	venc_entry_t *venc = platform_get_drvdata(venc_get_device());

	for (i = 0; i < MAX_SUPPORT_CORE_NUM && i < core_num; i++)
		pm_deinit_account(&venc->ctx[i].pm);
}

void venc_drv_print_pm_info(void)
{
	char buf[PM_INFO_BUFFER_SIZE];
	int32_t ret;
	uint32_t i;
	uint32_t core_num = venc_get_core_num();
	venc_entry_t *venc = platform_get_drvdata(venc_get_device());

	for (i = 0; i < MAX_SUPPORT_CORE_NUM && i < core_num; i++) {
		ret = pm_show_total_account(&venc->ctx[i].pm, buf, sizeof(buf));
		if (ret > 0)
			VCODEC_INFO_VENC("core id %d: %s", i, buf);
	}
}
