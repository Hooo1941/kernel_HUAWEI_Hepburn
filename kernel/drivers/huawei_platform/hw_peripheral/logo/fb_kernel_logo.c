/*
 * fb_kernel_logo.c
 *
 * show logo in kernel when s4 resume
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
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

#include <linux/module.h>
#include <linux/fb.h>
#include <linux/string.h>
#include <linux/kthread.h>
#include <linux/timer.h>
#include <linux/delay.h>
#include <linux/suspend.h>
#include "res_flash/light_spot_res.h"
#include "res_word_chn/word_chn_res.h"
#include "res_word_eng/word_eng_res.h"
#include "securec.h"

#ifndef SLEEP_MILLI_SEC
#define SLEEP_MILLI_SEC(nMilliSec)\
do { \
	long timeout = (nMilliSec) * HZ / 1000; \
	while(timeout > 0) \
	{ \
		timeout = schedule_timeout(timeout); \
	} \
}while(0);
#endif

#define WORD_DISTANCE_FROM_BOTTOM 240

typedef void (*get_word_data_func)(int line_num, unsigned char *data_buf);

phys_addr_t g_logo_buffer_addr = 0;
static bool vendor_country_chn_flag = false;
struct fb_info *g_info = NULL;
static struct task_struct * g_kernel_logo_thread = NULL;

static int g_panel_w;
static int g_panel_h;
static int g_progress_bar_x;
static int g_progress_bar_y;
static int g_progress_bar_w;
static int g_progress_bar_step_w = 2;
static int g_progress_bar_h = 4;
static int g_progress_bar_offset = 128;
static int g_progress_bar_pass_len = 0;
static unsigned char *g_flash_buff = NULL;

static int kernel_logo_pm_notification(struct notifier_block *nb, unsigned long action, void *data);
static struct notifier_block pm_notifier = { .notifier_call = kernel_logo_pm_notification };

static int open_fb_device(int fbidx)
{
	int ret = 0;

	g_info = get_fb_info_for_logo(fbidx);
	if (!g_info) {
		request_module("fb%d", fbidx);
		g_info = get_fb_info_for_logo(fbidx);
		if (!g_info) {
			return -ENODEV;
		}
	}
	if (IS_ERR(g_info)) {
		return PTR_ERR(g_info);
	}

	lock_fb_info(g_info);
	if (g_info->fbops->fb_open)
		ret = g_info->fbops->fb_open(g_info, 1);
	unlock_fb_info(g_info);

	return ret;
}

static void release_fb_device(void)
{
	int ret = 0;

	if (IS_ERR_OR_NULL(g_info)) {
		pr_err("invalid fb info: %ld\n", PTR_ERR(g_info));
		return;
	}

	lock_fb_info(g_info);
	if (g_info->fbops->fb_release)
		ret = g_info->fbops->fb_release(g_info, 1);
	unlock_fb_info(g_info);
	if (ret) {
		pr_err("fb release ret %d\n", ret);
		return;
	}
}

void init_global_data(void)
{
	g_panel_w = g_info->var.xres;
	g_panel_h = g_info->var.yres;

	g_progress_bar_w = g_panel_h / 2;
	g_progress_bar_x = (g_panel_w - g_progress_bar_w) / 2;
	g_progress_bar_y = g_panel_h / 2 + g_progress_bar_offset;

	return;
}

static void fill_rect_with_data(unsigned char *dst, unsigned char *data, int data_size,
	int dst_width, int dst_height)
{
	int num1 = dst_width / data_size;
	int num2 = dst_width % data_size;
	int copy_size = data_size * 4; /* 4 Byte */

	for (int i = 0; i < dst_height; i++) {
		for (int j = 0; j < num1; j++) {
			(void)memcpy_s(dst + j * copy_size, copy_size, data, copy_size);
		}
		if (num2) {
			(void)memcpy_s(dst + j * copy_size, num2 * 4, data, num2 * 4); /* 4 Byte*/
		}
		dst += (g_panel_w * 4); /* 4 Byte*/
	}

	return;
}

static void set_black_background(bool full_screen)
{
	unsigned char black_background_data_BGRA8888[] = {
		0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255,
		0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255,
		0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255,
		0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255,
		0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255,
	};
	int black_background_data_size = 20;

	int dst_width;
	int dst_height;
	int data_size = black_background_data_size;
	unsigned char *dst = NULL;
	unsigned char *data = (unsigned char *)black_background_data_BGRA8888;

	if (full_screen) {
		dst = (unsigned char *)(g_info->screen_base);
		dst_width = g_panel_w;
		dst_height = g_panel_h;
	} else {
		int y_start = g_progress_bar_y - (FLASH_FRAME_SIZE - g_progress_bar_h) / 2;
		int x_offset = g_progress_bar_x - (FLASH_FRAME_SIZE / 2);
		int *screen = (int *)(g_info->screen_base);
		dst = (unsigned char *)(screen + g_panel_w * y_start + x_offset);
		dst_width = g_progress_bar_w + FLASH_FRAME_SIZE;
		dst_height = FLASH_FRAME_SIZE;
	}

	fill_rect_with_data(dst, data, data_size, dst_width, dst_height);

	return;
}

static void set_progress_background(void)
{
	unsigned char progress_background_data_BGRA8888[] = {
		38, 38, 38, 255, 38, 38, 38, 255, 38, 38, 38, 255, 38, 38, 38, 255,
		38, 38, 38, 255, 38, 38, 38, 255, 38, 38, 38, 255, 38, 38, 38, 255,
		38, 38, 38, 255, 38, 38, 38, 255, 38, 38, 38, 255, 38, 38, 38, 255,
		38, 38, 38, 255, 38, 38, 38, 255, 38, 38, 38, 255, 38, 38, 38, 255,
		38, 38, 38, 255, 38, 38, 38, 255, 38, 38, 38, 255, 38, 38, 38, 255,
	};
	int progress_background_data_size = 20;

	int dst_width = g_progress_bar_w;
	int dst_height = g_progress_bar_h;
	int y_start = g_progress_bar_y;
	int x_offset = g_progress_bar_x;
	int data_size = progress_background_data_size;

	int *screen = (int *)(g_info->screen_base);
	unsigned char *dst = (unsigned char *)(screen + g_panel_w * y_start + x_offset);
	unsigned char *data = (unsigned char *)progress_background_data_BGRA8888;

	fill_rect_with_data(dst, data, data_size, dst_width, dst_height);

	return;
}

static void set_progress_foreground(void)
{
	unsigned char progress_foreground_data_BGRA8888[] = {
		153, 153, 153, 255, 153, 153, 153, 255, 153, 153, 153, 255, 153, 153, 153, 255,
		153, 153, 153, 255, 153, 153, 153, 255, 153, 153, 153, 255, 153, 153, 153, 255,
		153, 153, 153, 255, 153, 153, 153, 255, 153, 153, 153, 255, 153, 153, 153, 255,
		153, 153, 153, 255, 153, 153, 153, 255, 153, 153, 153, 255, 153, 153, 153, 255,
		153, 153, 153, 255, 153, 153, 153, 255, 153, 153, 153, 255, 153, 153, 153, 255,
	};
	int progress_foreground_data_size = 20;

	int dst_width = g_progress_bar_pass_len + g_progress_bar_step_w;
	dst_width = (dst_width > g_progress_bar_w) ? g_progress_bar_w : dst_width;

	int dst_height = g_progress_bar_h;
	int y_start = g_progress_bar_y;
	int x_offset = g_progress_bar_x;
	int data_size = progress_foreground_data_size;

	int *screen = (int *)(g_info->screen_base);
	unsigned char *dst = (unsigned char *)(screen + g_panel_w * y_start + x_offset);
	unsigned char *data = (unsigned char *)progress_foreground_data_BGRA8888;

	fill_rect_with_data(dst, data, data_size, dst_width, dst_height);

	g_progress_bar_pass_len = dst_width;
	return;
}

static void alpha_blending_for_flash_circle(unsigned char *dst, unsigned char *src)
{
	unsigned char alpha = *(src + 3);
	for (int i = 0; i < 3; i++) {
		*(dst + i) = (*(dst + i)) * (255 - alpha) / 255 + (*(src + i)) * alpha / 255;
	}
	return;
}

static void set_flash_circle(int loop_cnt)
{	
	if (g_progress_bar_pass_len >= g_progress_bar_w) {
		return;
	}

	int index = (loop_cnt / 2 ) % FLASH_FRAME_NUM;
	int x_position = g_progress_bar_pass_len + g_progress_bar_x;
	int left = x_position - (FLASH_FRAME_SIZE / 2);
	int top = g_progress_bar_y - (FLASH_FRAME_SIZE / 2) + (g_progress_bar_h / 2);

	int *screen = (int *)(g_info->screen_base);
	int *dst = screen + g_panel_w * top + left;

	get_flash_circle_data(index, g_flash_buff);

	int *src = (int *)g_flash_buff;

	for (int i = 0; i < FLASH_FRAME_SIZE; i++) {
		for (int j = 0; j < FLASH_FRAME_SIZE; j++) {
			alpha_blending_for_flash_circle((unsigned char *)(dst + j), (unsigned char *)(src + j));
		}
		dst += g_panel_w;
		src += FLASH_FRAME_SIZE;
	}

	return;
}

static int kernel_logo_thread(void *unused)
{
	int loop_cnt = 0;

	while (!kthread_should_stop())
	{
		lock_fb_info(g_info);

		set_black_background(false);

		set_progress_background();

		set_progress_foreground();

		set_flash_circle(loop_cnt);

		(void)fb_pan_display(g_info, &(g_info->var));

		unlock_fb_info(g_info);

		loop_cnt++;
		SLEEP_MILLI_SEC(16); /* 16: 60 frame per second */
	}

    g_kernel_logo_thread = NULL;
	return 0;
}

static void change_from_rgb565_to_bgra8888(unsigned short data_rgb565, unsigned char *dst)
{
	unsigned char r = (data_rgb565 >> 11) & 0x1F;
	unsigned char g = (data_rgb565 >> 5) & 0x3F;
	unsigned char b = data_rgb565 & 0x1F;

	r = (r << 3) | (r >> 2);
	g = (g << 2) | (g >> 4);
	b = (b << 3) | (b >> 2);

	*dst = b;
	*(dst + 1) = g;
	*(dst + 2) = r;
	*(dst + 3) = 0xFF;

	return;
}

static void set_logo_from_logo_buff(void)
{
	int left = g_progress_bar_x;
	int top = g_panel_h / 2 - g_panel_h / 10;
	int height = g_panel_h / 10 + 4;
	int width = g_progress_bar_w;

	unsigned int *dst = (unsigned int *)(g_info->screen_base);
	unsigned short *src = (unsigned short *)phys_to_virt(g_logo_buffer_addr);

	src += (g_panel_w * top + left);
	dst += (g_panel_w * top + left);

	unsigned short data_rgb565;
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			data_rgb565 = *(src + j);
			change_from_rgb565_to_bgra8888(data_rgb565, (unsigned char *)(dst + j));
		}
		src += g_panel_w;
		dst += g_panel_w;
	}

	return;
}

static void set_wake_up_word_proc(int word_w, int word_h, int *src, get_word_data_func get_src_data)
{
	int left = (g_panel_w - word_w) / 2;
	int top = g_panel_h - WORD_DISTANCE_FROM_BOTTOM - word_h;
	int height = word_h;
	int width = word_w;

	int *screen = (int *)(g_info->screen_base);
	int *dst = screen + g_panel_w * top + left;

	for (int i = 0; i < height; i++) {
		get_src_data(i, (unsigned char *)src);
		for (int j = 0; j < width; j++) {
			alpha_blending_for_flash_circle((unsigned char *)(dst + j), (unsigned char *)(src + j));
		}
		dst += g_panel_w;
	}
	return;
}

static void set_wake_up_word(void)
{
	if (vendor_country_chn_flag) {
		int src[WORD_CHN_LOGO_W] = {0};
		set_wake_up_word_proc(WORD_CHN_LOGO_W, WORD_CHN_LOGO_H, &src[0], get_word_chn_data);
	} else {
		int src[WORD_ENG_LOGO_W] = {0};
		set_wake_up_word_proc(WORD_ENG_LOGO_W, WORD_ENG_LOGO_H, &src[0], get_word_eng_data);
	}
	return;
}

static void show_kernel_logo(void)
{
	int fbidx = 0;
	int ret = open_fb_device(fbidx);
	if (ret) {
		pr_err("fb_kernel_logo: open fb device ret %d\n", ret);
		return;
	}

	init_global_data();

	if (!(g_info->fbops->fb_buffer_alloc_k)) {
		pr_err("fb_kernel_logo: fb_buffer_alloc_k is null\n");
		release_fb_device();
		return;
	}

	ret = g_info->fbops->fb_buffer_alloc_k(g_info);
	if (ret) {
		pr_err("fb_kernel_logo: fb buffer alloc ret %d\n", ret);
		release_fb_device();
		return;
	}

	set_black_background(true);

	if (g_logo_buffer_addr) {
		set_logo_from_logo_buff();
	} else {
		pr_err("fb_kernel_logo: logo_buffer_addr is null\n", ret);
		release_fb_device();
		return;
	}

	set_wake_up_word();

	g_flash_buff = (unsigned char *)kmalloc(FLASH_DATA_SIZE, GFP_KERNEL);
	if (!g_flash_buff) {
		pr_err("fb_kernel_logo: malloc FLASH_DATA_SIZE failed\n");
		release_fb_device();
		return;
	}

    g_kernel_logo_thread = kthread_run(kernel_logo_thread, NULL, "kernel_logo");

	return;
}

static void stop_kernel_logo(bool release_fb)
{
	if (g_kernel_logo_thread)
	{
		kthread_stop(g_kernel_logo_thread);
	}
    while (g_kernel_logo_thread) { 
		msleep(2);
	}

	if (release_fb) {
		release_fb_device();
	}

	if (g_flash_buff) {
		kfree(g_flash_buff);
	}

	return;
}

static void update_progress_step_w(void)
{
	static int loop = 0;
	static int last_passed_len = 0;

	loop++;
	if (loop == 1 || loop >= 11) {
		return;
	}

	int	frame_num = (g_progress_bar_pass_len - last_passed_len) / g_progress_bar_step_w;
	int remain_frame_num = (10 - loop + 1) * frame_num;
	int remain_len = g_progress_bar_w - g_progress_bar_pass_len;

	g_progress_bar_step_w = (remain_len + remain_frame_num / 2) / remain_frame_num;
	if (loop == 10) {
		g_progress_bar_step_w += 1;
	}

	pr_info("fb_kernel_logo: loop:%d, frame_num:%d, remain_frame_num:%d, remain_len=%d, step_w=%d\n",
		loop - 1, frame_num, remain_frame_num, remain_len, g_progress_bar_step_w);

	last_passed_len = g_progress_bar_pass_len;
	return;
}

static int kernel_logo_pm_notification(struct notifier_block *nb, unsigned long action, void *data)
{
	switch ((pm_s4_logo_state_t)action) {
		case PM_S4_LOGO_STATE_START:
			show_kernel_logo();
			break;
		case PM_S4_LOGO_STATE_STOP:
			stop_kernel_logo(true);
			break;
		case PM_S4_LOGO_STATE_EXIT:
			stop_kernel_logo(true);
			break;
		case PM_S4_LOGO_STATE_LOAD_IMG:
			update_progress_step_w();
			break;
		default:
			break;
	}

	return 0;
}

static int32_t __init kernel_logo_register(void)
{
	int ret = 0;

	ret = register_pm_logo_notifier(&pm_notifier);

	return ret;
}

static void __exit kernel_logo_unregister(void)
{
	stop_kernel_logo(true);
	return;
}

static int __init logo_buffer_addr_setup(char *str)
{
	unsigned long logo_buffer_addr;

	if (sscanf_s(str, "%lx", &logo_buffer_addr) == 1)
		g_logo_buffer_addr = (phys_addr_t)logo_buffer_addr;

	return 1;
}

static int __init vendorcountry_setup(char *str)
{
	pr_info("fb_kernel_logo: vendorcountry is %s\n", str);

	vendor_country_chn_flag = false;
	if (strcmp(str, "all/cn") == 0)
		vendor_country_chn_flag = true;

	return 1;
}

__setup("logo_buffer_addr=", logo_buffer_addr_setup);

__setup("vendorcountry=", vendorcountry_setup);

module_init(kernel_logo_register);
module_exit(kernel_logo_unregister);

MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
MODULE_DESCRIPTION("Kernel Logo Driver");
MODULE_LICENSE("GPL v2");