/*
 * dsp_load.c
 *
 * dsp load driver.
 *
 * Copyright (c) 2015-2020 Huawei Technologies Co., Ltd.
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
#include "dsp_load.h"

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/kthread.h>
#include <linux/io.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/completion.h>

#include "audio_log.h"
#include "dsp_om.h"
#include "audio_ddr_map.h"
#include "platform_base_addr_info.h"
#include "rdr_audio_soc.h"
#include "dsp_load_ca.h"

#define DSP_UNLOAD                               0xA5A55A5A
#define DSP_LOADED                               0x0

#define WAIT_MAX_CNT                             500
#define WAIT_INTERVAL_CNT                        200

struct dsp_load {
	unsigned int *dsp_loaded_indicate_addr;
	unsigned int *dsp_uart_to_mem_addr;
};

static DECLARE_COMPLETION(g_dsp_load_complete);
static struct dsp_load g_load_priv;
static struct task_struct *dsp_load_khandler_task;

static void set_dsp_unload(void)
{
	if (g_load_priv.dsp_loaded_indicate_addr == NULL) {
		loge("dsp load indicate addr not remap\n");
		return;
	}

	logi("before set dsp unload, 0x%x\n", readl(g_load_priv.dsp_loaded_indicate_addr));
	writel(DSP_UNLOAD, g_load_priv.dsp_loaded_indicate_addr);
	logi("set dsp unload\n");
}

static void set_dsp_loaded(void)
{
	if (g_load_priv.dsp_loaded_indicate_addr == NULL) {
		loge("dsp load indicate addr not remap\n");
		return;
	}

	logi("before set dsp loaded, 0x%x\n", readl(g_load_priv.dsp_loaded_indicate_addr));
	writel(DSP_LOADED, g_load_priv.dsp_loaded_indicate_addr);
	logi("set dsp loaded\n");
}

static void set_dsp_uart_to_mem_cur_offset(void)
{
	/* set offset address to uart ddr mem */
	if (g_load_priv.dsp_uart_to_mem_addr == NULL) {
		loge("dsp uart to mem cur addr not remap\n");
		return;
	}

	writel(DRV_DSP_UART_TO_MEM_RESERVE_SIZE, g_load_priv.dsp_uart_to_mem_addr);
}

static int wait_to_load_dsp_image(void)
{
	uint32_t wait_cnt = 0;
	int ret = 0;

	/* wait image can be opened, max wait time = 20s */
	while (wait_cnt < WAIT_MAX_CNT) {
		ret = dsp_read_image();
		if (ret == 0)
			break;

		msleep(WAIT_INTERVAL_CNT);
		wait_cnt++;
	}

	if (wait_cnt == WAIT_MAX_CNT) {
		loge("wait timeout, wait_cnt = %u ret: %d\n", wait_cnt, ret);
		return ret;
	}

	logi("wait intime, wait_cnt = %u\n", wait_cnt);

	return 0;
}

static int dsp_load_handler_thread(void *arg)
{
	int ret = 0;

	logi("enter dsp load handler thread\n");

	ret = wait_to_load_dsp_image();
	if (ret) {
		loge("wait to load image fail,  ret = %d\n", ret);
		goto end;
	}

	ret = dsp_load_and_verify_image();
	if (ret) {
		loge("load dsp fail, ret = %d\n", ret);
		goto end;
	}

	logi("load and verify succ\n");

	set_dsp_loaded();
	set_dsp_uart_to_mem_cur_offset();
	set_dsp_img_loaded();
	complete_all(&g_dsp_load_complete);

end:
	logi("exit dsp load handler thread\n");
	return 0;
}

static int create_dsp_load_thread(void)
{
	dsp_load_khandler_task = kthread_run(dsp_load_handler_thread, NULL, "dsp_load_handler_thread");
	if (!dsp_load_khandler_task) {
		pr_err("create dsp load handler thread fail\n");
		return -EBUSY;
	}

	pr_info(LOG_TAG "create dsp load handler thread succ\n");

	return 0;
}

static void destroy_dsp_load_thread(void)
{
	if (dsp_load_khandler_task)
		dsp_load_khandler_task = NULL;
}

void dsp_load_init(unsigned char *unsec_virt_addr)
{
	pr_info(LOG_TAG "dsp load init\n");

	if (unsec_virt_addr == NULL) {
		pr_err(LOG_TAG "para is null\n");
		return;
	}

	g_load_priv.dsp_loaded_indicate_addr = (unsigned int *)(unsec_virt_addr +
		(DRV_DSP_LOADED_INDICATE - DSP_UNSEC_BASE_ADDR));
	g_load_priv.dsp_uart_to_mem_addr = (unsigned int *)(unsec_virt_addr +
		(DRV_DSP_UART_TO_MEM_CUR_ADDR - DSP_UNSEC_BASE_ADDR));

	set_dsp_unload();

	if (strstr(saved_command_line, "recovery_update=1")) {
		pr_info(LOG_TAG "recovery_update, do not load hifi\n");
		return;
	}

	create_dsp_load_thread();
}

void dsp_load_deinit(void)
{
	memset(&g_load_priv, 0, sizeof(g_load_priv));

	destroy_dsp_load_thread();
}

unsigned long wait_dsp_load_completion_timeout(unsigned long timeout)
{
	return wait_for_completion_timeout(&g_dsp_load_complete, timeout);
}