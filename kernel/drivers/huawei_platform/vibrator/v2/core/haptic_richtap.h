/*
 * haptic_richtap.h
 *
 * code for vibrator
 *
 * Copyright (c) 2023 Huawei Technologies Co., Ltd.
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

#ifndef _HAPTIC_RICHTAP_H_
#define _HAPTIC_RICHTAP_H_

#ifdef AAC_RICHTAP_SUPPORT

#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/mm_types.h>

enum {
	RICHTAP_UNKNOWN = -1,
	RICHTAP_MAGIC = 0x09,
};

enum {
	MMAP_BUF_DATA_VALID = 0x55,
	MMAP_BUF_DATA_FINISHED = 0xAA,
	MMAP_BUF_DATA_INVALID = 0xFF,
};

#define RICHTAP_IOCTL_GROUP         0x52
#define RICHTAP_GET_HWINFO          _IO(RICHTAP_IOCTL_GROUP, 0x03)
#define RICHTAP_SET_FREQ            _IO(RICHTAP_IOCTL_GROUP, 0x04)
#define RICHTAP_SETTING_GAIN        _IO(RICHTAP_IOCTL_GROUP, 0x05)
#define RICHTAP_OFF_MODE            _IO(RICHTAP_IOCTL_GROUP, 0x06)
#define RICHTAP_TIMEOUT_MODE        _IO(RICHTAP_IOCTL_GROUP, 0x07)
#define RICHTAP_RAM_MODE            _IO(RICHTAP_IOCTL_GROUP, 0x08)
#define RICHTAP_RTP_MODE            _IO(RICHTAP_IOCTL_GROUP, 0x09)
#define RICHTAP_STREAM_MODE         _IO(RICHTAP_IOCTL_GROUP, 0x0A)
#define RICHTAP_UPDATE_RAM          _IO(RICHTAP_IOCTL_GROUP, 0x10)
#define RICHTAP_GET_F0              _IO(RICHTAP_IOCTL_GROUP, 0x11)
#define RICHTAP_STOP_MODE           _IO(RICHTAP_IOCTL_GROUP, 0x12)

#define RICHTAP_MMAP_BUF_SIZE       1000
#define RICHTAP_MMAP_PAGE_ORDER     2
#define RICHTAP_MMAP_BUF_SUM        16

#pragma pack(4)
struct mmap_buf_format {
	uint8_t status;
	uint8_t bit;
	int16_t length;
	uint32_t reserve;
	struct mmap_buf_format *kernel_next;
	struct mmap_buf_format *user_next;
	uint8_t data[RICHTAP_MMAP_BUF_SIZE];
};
#pragma pack()

struct haptic;

long richtap_file_unlocked_ioctl(struct file *filp, unsigned int cmd,
				 unsigned long arg);
int richtap_file_mmap(struct file *filp, struct vm_area_struct *vma);
int richtap_rtp_config_init(struct haptic *haptic);
irqreturn_t irq_richtap_aei_handle(struct haptic *haptic);

#endif // AAC_RICHTAP_SUPPORT

#endif // _HAPTIC_RICHTAP_H_
