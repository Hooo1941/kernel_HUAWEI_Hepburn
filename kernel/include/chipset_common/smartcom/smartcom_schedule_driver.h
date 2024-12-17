/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */
#ifndef __SMARTCOM_SCHEDULE_DRIVER_H__
#define __SMARTCOM_SCHEDULE_DRIVER_H__

#include <linux/version.h>
#include <net/sock.h>
#include <linux/socket.h>
#include <linux/in.h>
#include <linux/in6.h>
#include <linux/spinlock.h>

#define SMARTCOM_MAX_ACC_APP 5
#define SMARTCOM_UID_ACC_AGE_MAX 1000
#define SMARTCOM_SPEED_CTRL_BASE_WIN_SIZE 10000
#define FAST_SYN_COUNT 5
#define MIN_JIFFIE 1

#define SMARTCOM_SCHEDULE_DRIVER_ACC_HIGH 0x5A

typedef enum {
	SMARTCOM_SCHEDULE_DRIVER_IP_TYPE_INVALID,
	SMARTCOM_SCHEDULE_DRIVER_IP_TYPE_V4,
	SMARTCOM_SCHEDULE_DRIVER_IP_TYPE_V6,
	SMARTCOM_SCHEDULE_DRIVER_IP_TYPE_V4V6
} smartcom_schedule_driver_ip_type;

struct smartcom_schedule_driver_acc_app_info {
	uid_t     uid; /* The uid of accelerate Application */
	uint16_t  age;
	uint16_t  reverse;
};

struct smartcom_schedule_driver_speed_ctrl_info {
	uid_t     uid; /* The uid of foreground Application */
	uint32_t  size; /* The grade of speed control */
	spinlock_t stlocker; /* The Guard Lock of this unit */
};

struct smartcom_schedule_driver_speed_ctrl_data {
	uid_t     uid; /* The uid of foreground Application */
	uint32_t  size; /* The grade of speed control */
};

void smartcom_schedule_driver_init(void);
int smartcom_schedule_driver_clear(void);
bool smartcom_schedule_driver_hook_ul_stub(struct sock *pstSock);
void smartcom_schedule_driver_set_acc_state(struct sock *pst_sock);
void smartcom_schedule_driver_speedctrl_winsize(struct sock *pstSock, uint32_t *win);
void smartcom_schedule_driver_fastsyn(struct sock *pstSock);
void smartcom_schedule_driver_evt_proc(int32_t event, const void *data, uint16_t len);
void smartcom_schedule_driver_set_speedctrl(struct smartcom_schedule_driver_speed_ctrl_info *speedctrl_info,
	uid_t uid_value, uint32_t size);

#endif
