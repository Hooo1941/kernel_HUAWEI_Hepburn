/*
 * drv_venc.c
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

#include "drv_venc.h"
#include "hal_venc.h"
#include "drv_common.h"
#include "venc_regulator.h"
#include "drv_mem.h"
#ifdef VENC_MCORE_ENABLE
#include "drv_venc_mcore.h"
#endif

#ifdef SUPPORT_VENC_FREQ_CHANGE
static uint32_t g_current_venc_freq = 0;
#endif

static int32_t venc_init(void)
{
	if (drv_mem_init() != 0) {
		VCODEC_ERR_VENC("drv_mem_init failed");
		return VCODEC_FAILURE;
	}

	return 0;
}

static void venc_deinit(void)
{
	drv_mem_exit();
}

static int32_t venc_update_hardware_status(struct encode_info *encode_info)
{
	int32_t ret = 0;

	if (encode_info->is_reset == 1) {
		ret = venc_regulator_reset();
		if (ret != 0) {
			VCODEC_ERR_VENC("reset failed");
			return ret;
		}
	}

#ifdef SUPPORT_VENC_FREQ_CHANGE
	mutex_lock(&g_venc_freq_mutex);
	if (g_current_venc_freq != g_venc_freq) {
		encode_info->clock_info.is_set_clock = 1;
		g_current_venc_freq = g_venc_freq;
	}
	mutex_unlock(&g_venc_freq_mutex);
#endif

	if (encode_info->clock_info.is_set_clock == 1) {
		ret = venc_regulator_update(&encode_info->clock_info);
		if (ret != 0)
			VCODEC_WARN_VENC("update clock info failed");
	}

	return ret;
}

static void venc_start_encode(struct encode_info *encode_info,
	struct venc_fifo_buffer *buffer, int32_t core_id)
{
	struct venc_context *ctx = NULL;
	bool is_protected = (encode_info->is_protected == 1 ? true : false);
	venc_entry_t *venc = platform_get_drvdata(venc_get_device());

	ctx = &venc->ctx[core_id];

	if (ctx->is_protected != is_protected)
		ctx->first_cfg_flag = true;

	if (encode_info->reg_cfg_mode == VENC_SET_CFGREGSIMPLE)
		vedu_hal_cfg_reg_simple(encode_info, core_id);
	else
		vedu_hal_cfg_reg(encode_info, core_id);

	ctx->status = VENC_BUSY;
	ctx->start_time = osal_get_sys_time_in_us();
	ctx->is_block = (encode_info->is_block == 1 ? true : false);
	ctx->buffer = buffer;
	ctx->is_protected = is_protected;

	(void)memcpy_s(&ctx->channel, sizeof(struct channel_info), &encode_info->channel, sizeof(struct channel_info));
	if (ctx->is_block == false)
		osal_add_timer(&ctx->timer, INTERRUPT_TIMEOUT_MS);

#ifdef VENC_MCORE_ENABLE
	venc_mcore_start_encode(encode_info, (S_HEVC_AVC_REGS_TYPE *)(ctx->reg_base));
#else
	venc_hal_start_encode((S_HEVC_AVC_REGS_TYPE *)(ctx->reg_base)); /*lint !e826 */
#endif
}

#ifdef VCODECV700
static void venc_drv_set_clk_type(struct encode_info *encode_info)
{
	uint32_t i;
	uint64_t total_load = 0;
	uint32_t low_power_channel_num = 0;
	uint32_t channel_num = 0;
	bool low_power_mode = false;
	struct venc_performance_info *mode = NULL;
	uint32_t mode_num;

	struct venc_file_private* venc_file_private_info = get_venc_file_private();

	for (i = 0; i < MAX_INSTANCES_COUNT; i++) {
		if (!(queue_is_null(venc_file_private_info[i].encode_done_info))) {
			if (ULLONG_MAX - total_load >= venc_file_private_info[i].venc_perf_loads)
				total_load += venc_file_private_info[i].venc_perf_loads;

			channel_num++;
			if (venc_file_private_info[i].venc_low_power_enable_flag)
				low_power_channel_num++;
		}
	}

	if (low_power_channel_num == 1 && channel_num == 1)
		low_power_mode = true;

	mode_num = venc_get_power_profiles(low_power_mode, &mode);

	for (i = 0; i < mode_num; i++) {
		if (total_load <= (uint64_t)(mode[i].loads)) {
			break;
		}
	}

	// using the last mode, when the load is higher than the threshold of the last mode
	if (i >= mode_num)
		i = mode_num - 1;

#ifndef VCODECV500
	// using the last mode in multi instances
	if (channel_num > 1)
		i = mode_num - 1;
#endif

	encode_info->clock_info.clock_type = mode[i].power_level;
	encode_info->clock_info.core_num   = mode[i].core_num;
	VCODEC_DBG_VENC("set power mode: core num is %u, power level is %d",
					encode_info->clock_info.core_num, encode_info->clock_info.clock_type);
}
#endif

static int32_t venc_encode(struct encode_info *encode_info, struct venc_fifo_buffer *buffer)
{
	int32_t core_id;
	venc_entry_t *venc = platform_get_drvdata(venc_get_device());
	bool support_power_control_per_frame =
		is_support_power_control_per_frame() &&
		(encode_info->enable_power_control_per_frame == 1) &&
		(encode_info->is_block == 1);

	int32_t ret = vcodec_venc_down_interruptible(&venc->hw_sem);
	if (ret != 0) {
		VCODEC_FATAL_VENC("down sem failed");
		return ret;
	}

	if (support_power_control_per_frame) {
		ret = venc_regulator_enable();
		if (ret != 0) {
			VCODEC_FATAL_VENC("power up fail, ret %d", ret);
			goto poweron_failed;
		}
	}

#ifdef VCODECV700
	venc_drv_set_clk_type(encode_info);
#endif

	ret = venc_update_hardware_status(encode_info);
	if (ret != 0)
		goto exit;

	venc_set_clock_info(&encode_info->clock_info);

	core_id = venc_regulator_select_idle_core(&venc->event);
	ret = venc_check_coreid(core_id);
	if (ret != 0) {
		VCODEC_ERR_VENC("CORE_ERROR:invalid core ID is %d", core_id);
		goto exit;
	}

	pm_hardware_busy_enter(&venc->ctx[core_id].pm);

	venc_start_encode(encode_info, buffer, core_id);

	if (encode_info->is_block != 1)
		goto exit;

	/* Enter the synchronous mode, wait for the encoding to complete, and fill the queue. */
	ret = venc_drv_get_encode_done_info(buffer, &encode_info->encode_done_info);
	if (ret != 0) {
		VCODEC_FATAL_VENC("wait encode done failed");
		/* Marks the encoder as timeout and resets the encoder in the next frame. */
		venc->ctx[core_id].status = VENC_TIME_OUT;
	}

exit:
	if (support_power_control_per_frame) {
		ret = venc_regulator_disable();
		if (ret != 0)
			VCODEC_FATAL_VENC("power off fail, ret %d", ret);
	}
poweron_failed:
	vcodec_venc_up_interruptible(&venc->hw_sem);
	return ret;
}

static void venc_encode_timeout_proc(int32_t core_id)
{
	int32_t ret;
	struct encode_done_info info = {0};
	struct venc_context *ctx = NULL;
	venc_entry_t *venc = platform_get_drvdata(venc_get_device());
	unsigned long flag;

	VCODEC_WARN_VENC("coreid:%d encode timeout", core_id);

	ret = venc_check_coreid(core_id);
	if (ret != 0) {
		VCODEC_ERR_VENC("CORE_ERROR:invalid core ID is %d", core_id);
		return;
	}
	ctx = &venc->ctx[core_id];

	spin_lock_irqsave(&ctx->lock, flag);
	if (ctx->status != VENC_BUSY) {
		spin_unlock_irqrestore(&ctx->lock, flag);
		VCODEC_FATAL_VENC("core %d: current core status invalid, status: %d",
				core_id, ctx->status);
		return;
	}

	(void)memcpy_s(&info.channel_info, sizeof(struct channel_info), &ctx->channel, sizeof(struct channel_info));
	info.is_timeout = 1;

	ret = push(ctx->buffer, &info);
	if (ret != 0)
		VCODEC_FATAL_VENC("core %d: write buffer failed", core_id);

	pm_hardware_busy_exit(&ctx->pm);
	ctx->status = VENC_TIME_OUT;

	/* wake up preprocess and postprocess */
	venc_drv_osal_give_event(&venc->event);
	venc_drv_osal_give_event(&ctx->buffer->event);
	spin_unlock_irqrestore(&ctx->lock, flag);
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
static void venc_encode_timeout(struct timer_list *data)
{
	struct venc_context *ctx = from_timer(ctx, data, timer);
	if (!ctx) {
		VCODEC_ERR_VENC("ctx is null pointer");
		return;
	}

	venc_encode_timeout_proc(ctx->core_id);
}
#else
static void venc_encode_timeout(unsigned long core_id)
{
	venc_encode_timeout_proc((int32_t)core_id);
}
#endif

static bool is_encode_done(U_FUNC_VCPI_INTSTAT int_status)
{
#ifdef VENC_MCORE_ENABLE
	// when mcu is enabled, eop is taken over by mcu, then mcu uses soft_int_1 to notify CPU.
	if (int_status.bits.vcpi_soft_int1)
		return true;
#endif
	if (int_status.bits.vcpi_int_ve_eop)
		return true;
	if (int_status.bits.vcpi_int_vedu_timeout)
		return true;

	return false;
}

static bool is_isr_support(U_FUNC_VCPI_INTSTAT int_status)
{
	if (is_encode_done(int_status))
		return true;

#ifdef VENC_MCORE_ENABLE
	// when mcu is enabled, slice_end is taken over by mcu, then mcu uses soft_int_0 to notify CPU.
	if (int_status.bits.vcpi_soft_int0)
		return true;
#endif
	if (int_status.bits.vcpi_int_vedu_slice_end)
		return true;

	return false;
}

static int32_t venc_restore_channel_id(struct venc_context *ctx)
{
	uint32_t i;
	struct venc_file_private* venc_file_private_info = get_venc_file_private();
	VCODEC_DBG_VENC("enter %s (): kernel channel ID = %d", __func__, ctx->channel.id);
	for (i = 0; i < MAX_INSTANCES_COUNT; i++) {
		if (venc_file_private_info[i].encode_done_info == ctx->buffer) {
			ctx->channel.id = venc_file_private_info[i].user_channel_id;
			VCODEC_DBG_VENC("User channel ID = %d", ctx->channel.id);
			return 0;
		}
	}

	VCODEC_ERR_VENC("cannot find fifo buffer in the context");
	return VCODEC_FAILURE;
}

static void venc_encode_done(struct venc_context *ctx)
{
	int32_t ret;
	U_FUNC_VCPI_INTSTAT int_status;
	U_VEDU_VCPI_MULTISLC multi_slc;
	S_HEVC_AVC_REGS_TYPE *reg_base = NULL;
	struct encode_done_info info = {0};
	venc_entry_t *venc = platform_get_drvdata(venc_get_device());
	unsigned long flag;

	reg_base = (S_HEVC_AVC_REGS_TYPE *)ctx->reg_base;
	int_status.u32 = reg_base->FUNC_VCPI_INTSTAT.u32;
	multi_slc.u32 = reg_base->VEDU_VCPI_MULTISLC.u32;

	if (!is_isr_support(int_status)) {
		VCODEC_WARN_VENC("not support isr %x", int_status.u32);
		reg_base->VEDU_VCPI_INTCLR.u32 = int_status.u32;
		return;
	}


	spin_lock_irqsave(&ctx->lock, flag);
	if (ctx->status != VENC_BUSY) {
		reg_base->VEDU_VCPI_INTCLR.u32 = int_status.u32;
		spin_unlock_irqrestore(&ctx->lock, flag);
		VCODEC_FATAL_VENC("current core status invalid, core status: %d, isr status %x",
			ctx->status, int_status.u32);
		return;
	}

	if (venc_restore_channel_id(ctx)) {
		spin_unlock_irqrestore(&ctx->lock, flag);
		return;
	}

	/*
	* The interrupt handling function does not independently process the bufferfull interrupt status.
	* When the EOP or slice end interrupts are generated,
	* the information is returned to the user mode by reading the raw interrupt register.
	*/
	info.is_timeout = int_status.bits.vcpi_int_vedu_timeout;
	if (is_encode_done(int_status)) {
		if (!ctx->is_block) {
			if (osal_del_timer(&ctx->timer, false) != 0)
				VCODEC_FATAL_VENC("delete timer fail");

			(void)memcpy_s(&info.channel_info, sizeof(struct channel_info), &ctx->channel, sizeof(struct channel_info));
		}

		venc_hal_get_reg_venc(&info.stream_info, ctx->reg_base);
		ret = push(ctx->buffer, &info);
		if (ret != 0)
			VCODEC_ERR_VENC("write buffer fail");
		venc_drv_osal_give_event(&ctx->buffer->event); /* wake up preprocess */

		pm_hardware_busy_exit(&ctx->pm);
		ctx->status = VENC_IDLE;
		venc_drv_osal_give_event(&venc->event); /* wake up postprocess */
		reg_base->VEDU_VCPI_INTCLR.u32 = 0xFFFFFFFF;
		spin_unlock_irqrestore(&ctx->lock, flag);

		return;
	}

#ifdef SLICE_INT_EN
	if (!ctx->is_block && multi_slc.bits.vcpi_multislc_en) {
		// slice end interrupt
		(void)memcpy_s(&info.channel_info, sizeof(struct channel_info), &ctx->channel, sizeof(struct channel_info));
		venc_hal_get_slice_reg(&info.stream_info, ctx->reg_base);
		ret = push(ctx->buffer, &info);
		if (ret != 0)
			VCODEC_ERR_VENC("write buffer fail");
		venc_drv_osal_give_event(&ctx->buffer->event);
	}
#endif
	reg_base->VEDU_VCPI_INTCLR.u32 = int_status.u32;
	spin_unlock_irqrestore(&ctx->lock, flag);
}

void venc_init_ops(void)
{
	venc_entry_t *venc = platform_get_drvdata(venc_get_device());

	venc->ops.init = venc_init;
	venc->ops.deinit = venc_deinit;
	venc->ops.encode = venc_encode;
	venc->ops.encode_done = venc_encode_done;
	venc->ops.encode_timeout = venc_encode_timeout;

	VCODEC_INFO_VENC("config register by apb bus");
}
