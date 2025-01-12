/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 * Description: tui agent for tui display and interact
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */
#ifndef TUI_H
#define TUI_H

#include "teek_ns_client.h"
#include "teek_client_type.h"

#define TEE_TUI_AGENT_ID    0x54554944  /* TUID */

/* tui states */
#define TUI_STATE_UNUSED	0
#define TUI_STATE_CONFIG	1
#define TUI_STATE_RUNNING	2
#define TUI_STATE_ERROR	3

#define TUI_PID_CLEAR   0
#define TUI_PID_CONFIG  1
/* command from secure os */
#define TUI_CMD_ENABLE		1
#define TUI_CMD_DISABLE		2
#define TUI_CMD_POLL		3
#define TUI_CMD_SET_STATE	4
#define TUI_CMD_PAUSE		5
#define TUI_CMD_DO_SYNC		6
#define TUI_CMD_START_DELAY_WORK	7
#define TUI_CMD_CANCEL_DELAY_WORK	8
#define TUI_CMD_LOAD_TTF	9
#define TUI_CMD_FREE_TTF_MEM	11
#define TUI_CMD_EXIT		12


#define TUI_DRV_NAME_MAX	32

/* poll event type from normal to secure */
enum tui_poll_type {
	TUI_POLL_CFG_OK,
	TUI_POLL_CFG_FAIL,
	TUI_POLL_TP,
	TUI_POLL_TICK,
	TUI_POLL_DELAYED_WORK,
	TUI_POLL_TIMEOUT,
	TUI_POLL_RESUME_TUI,
/* For some reasons, we need a method to terminate TUI from no-secure
 * OS, for example the TUI CA maybe killed.
 */
	TUI_POLL_CANCEL,
	TUI_POLL_HANDLE_TUI,  /* for tui to handle event */
	TUI_POLL_NAVI_H_TO_S, /* for navigator hide and show */
	TUI_POLL_NAVI_S_TO_H,
	TUI_POLL_SHS_0_TO_1,  /* for single hand mode switch */
	TUI_POLL_SHS_0_TO_2,
	TUI_POLL_SHS_1_TO_0,
	TUI_POLL_SHS_2_TO_0,
	TUI_POLL_ROTATION_0,  /* for retation switch */
	TUI_POLL_ROTATION_90,
	TUI_POLL_ROTATION_180,
	TUI_POLL_ROTATION_270,
	TUI_POLL_KEYBOARDTYPE_0,
	TUI_POLL_KEYBOARDTYPE_3,
	TUI_POLL_SEMITRANS,
	TUI_POLL_CURSOR,
	TUI_POLL_GETFP,
	TUI_POLL_NOTCH,       /* for tui to get notch height */
	TUI_POLL_DIALOGTIMEOUT,
	TUI_POLL_FOLD,        /* for tui to get fold_screen */
	TUI_POLL_MAX          /* Do Not add type behind this one */
};

/* tui-max should be bigger than TUI_POLL_MAX in tui.h */
static const char *const poll_event_type_name[] = {
	"config-ok",
	"config-fail",
	"tp",
	"tui-tick",
	"tui-delaywork",
	"tui-pause",
	"tui-resume",
	"tui-terminate",
	"tui-handle",
	"tui-hs",
	"tui-sh",
	"tui-01",
	"tui-02",
	"tui-10",
	"tui-20",
	"tui-0",
	"tui-90",
	"tui-180",
	"tui-270",
	"tui_key_board_type0",
	"tui_key_board_type3",
	"tui-SEMI",
	"tui-cursor",
	"tui-gettp",
	"tui-notch",
	"tui-dialogtimeout",
	"tui-fold",
	"tui-max"
};

static const char *const state_name[] = {
	"unused",
	"config",
	"running",
	"error"
};

struct tui_ctl_shm {
	struct {
		int command;
		int value;
		int ret;
	} s2n;
	struct {
		int event_type;
		int value;
		unsigned int addr;
		unsigned int addr_h;
		int tp_info;
		int tp_info_h_addr;
		int status;
		int x;
		int y;
		uint32_t npages;
		uint64_t info_length;
		uint32_t phy_size;
	} n2s;
};

struct tui_msg_node {
	int type;
	int val;
	void *data;
	struct list_head list;
};

typedef int (*tui_drv_init) (void *pdata, int secure);

struct tui_drv_node {
	tui_drv_init init_func;
	void *pdata;
	char name[TUI_DRV_NAME_MAX];
	int state;
	int priority;
	struct list_head list;
};

/* tui-need-memory is calculated dynamically according to the screen resolution */
struct tui_mem {
	unsigned int tui_addr_size;
	unsigned int tui_addr;
	unsigned int tui_addr_h;
	struct device *tui_dev;
	char *tui_virt;
};

struct ttf_mem {
	unsigned int ttf_addr_h;
	unsigned int ttf_addr_l;
	char *ttf_buff_virt;
	unsigned int ttf_file_size;
};

typedef struct tui_memory {
	phys_addr_t tui_ion_phys_addr;
	void *tui_ion_virt_addr;
	size_t len;
	uint32_t size;
	uint32_t configid;
	struct sg_table *tui_sg_table;
	phys_addr_t fb_phys_addr;
	uint32_t npages;
	uint64_t info_length;
} tui_ion_mem;

#ifdef CONFIG_TEE_TUI
extern int ts_tui_report_input(void *finger_data);
extern int tui_fp_notify(void);
int __init init_tui(const struct device *dev);
void free_tui(void);
int tui_send_event(int event, struct teec_tui_parameter *tui_param);
int register_tui_driver(tui_drv_init fun, const char *name,
					 void *pdata);
void unregister_tui_driver(const char *name);
/*
 * TUI has different state that can recieve given types of message,
 * there are 3 APIs to send message.
 * send_tui_msg_config:send message to TUI in config state only.
 */
int send_tui_msg_config(int type, int val, void *data);
void tui_poweroff_work_start(void);

void set_tui_caller_info(unsigned int devid, int pid);
void free_tui_caller_info(void);

unsigned int tui_attach_device(void);
void do_ns_tui_release(void);
int is_tui_in_use(int pid_value);
int tc_ns_tui_event(struct tc_ns_dev_file *dev_file, const void *argp);
bool is_tui_agent(unsigned int agent_id);
#else
static inline bool is_tui_agent(unsigned int agent_id)
{
	(void)agent_id;
	return false;
}

static inline int init_tui(const struct device *dev)
{
	(void)dev;
	return 0;
}

static inline void free_tui(void)
{
}

static inline void unregister_tui_driver(const char *name)
{
	(void)name;
}

static inline int send_tui_msg_config(int type, int val, const void *data)
{
	(void)type;
	(void)val;
	(void)data;
	return 0;
}

static inline void set_tui_caller_info(unsigned int devid, int pid)
{
	(void)devid;
	(void)pid;
}
static inline void free_tui_caller_info(void)
{
}

static inline unsigned int tui_attach_device(void)
{
	return 0;
}
static inline int is_tui_in_use(int pid_value)
{
	(void)pid_value;
	return 0;
}

static inline void do_ns_tui_release(void)
{
}

static inline int tc_ns_tui_event(struct tc_ns_dev_file *dev_file, const void *argp)
{
	(void)dev_file;
	(void)argp;
	return 0;
}
#endif

#endif
