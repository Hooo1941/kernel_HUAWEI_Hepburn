/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2020. All rights reserved.
 * foss@huawei.com
 *
 * If distributed as part of the Linux kernel, the following license terms
 * apply:
 *
 * * This program is free software; you can redistribute it and/or modify
 * * it under the terms of the GNU General Public License version 2 and
 * * only version 2 as published by the Free Software Foundation.
 * *
 * * This program is distributed in the hope that it will be useful,
 * * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * * GNU General Public License for more details.
 * *
 * * You should have received a copy of the GNU General Public License
 * * along with this program; if not, write to the Free Software
 * * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
 *
 * Otherwise, the following license terms apply:
 *
 * * Redistribution and use in source and binary forms, with or without
 * * modification, are permitted provided that the following conditions
 * * are met:
 * * 1) Redistributions of source code must retain the above copyright
 * *    notice, this list of conditions and the following disclaimer.
 * * 2) Redistributions in binary form must reproduce the above copyright
 * *    notice, this list of conditions and the following disclaimer in the
 * *    documentation and/or other materials provided with the distribution.
 * * 3) Neither the name of Huawei nor the names of its contributors may
 * *    be used to endorse or promote products derived from this software
 * *    without specific prior written permission.
 *
 * * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef __BSP_PCIE_AB_H__
#define __BSP_PCIE_AB_H__

#include <osl_irq.h>
#include <osl_bio.h>
#include <product_config.h>
#include <osl_types.h>

/* ******mult msi definition****** */
#define PCIE_EP_MAX_MSI_NUM  32

/* ******rc and ep msi user num****** */
#define PCIE_RC_MSI_NUM 16 /* rc to ep msi */
#define PCIE_EP_MSI_NUM 16  /* ep to rc msi */
#define PCIE_GIC_MSI_NUM   0x100U

#define EP_TO_RC_MSI_INT_STATE_OFFSET 0x0
#define EP_TO_RC_MSI_INT_ENABLE_OFFSET 0x200

/* -------------------------------PCIE BOOT MSG Definition---------------------------------- */
/* 4K message description:
-----------------------------------------------------------------------------
|         2K ( B -> A )               |         2K ( A -> B )               |
-----------------------------------------------------------------------------
| msi_id | cmd_len | cmd_data         | msi_id | cmd_len | resp_data        |
-----------------------------------------------------------------------------
|   4B   |   4B    | unfixed          |   4B   |   4B    | unfixed          |
-----------------------------------------------------------------------------
*/
/* ****************************************************************************
     *                          Attention                          *
* *****************************************************************************
* Description : Driver for pcie boot definition
* Core        : Acore、Fastboot
* Header File : the following head files need to be modified at the same time
*             : /acore/bsp_pcie.h
*             : /fastboot/bsp_pcie.h
*/
/* ***********************************Start************************************ */
#define PCIE_TRANS_MSG_SIZE               0x1000
#define PCIE_TRANS_EP_MSG_OFFSET          0x0
#define PCIE_TRANS_EP_MSG_SIZE            0x800
#define PCIE_TRANS_RC_MSG_OFFSET          (PCIE_TRANS_EP_MSG_OFFSET + PCIE_TRANS_EP_MSG_SIZE)
#define PCIE_TRANS_RC_MSG_SIZE            0x800
/* **********************************End************************************ */
/* ******PCIE PM Msg Offset****** */
#define PCIE_EP_PM_MSG_OFFSET                0x0100
#define PCIE_RC_PM_MSG_OFFSET                0x0200
#define PCIE_PM_MODE_OFFSET                  0x0300
#define PCIE_RC_PM_TEST_MSG_OFFSET           0x0400
#define PCIE_RC_PM_BAR_TEST_OFFSET           0x0410
#define PCIE_EP_PM_TEST_MSG_OFFSET           0x0500
#define PCIE_EP_PM_BAR_TEST_OFFSET           0x0510
#define PCIE_EP_PM_STATUS_OFFSET             0x0600
#define PCIE_EP_WAKE_RC_USER_OFFSET          0x0610

enum pcie_pm_mode {
    PCIE_LOW_POWER_MODE = 0,
    PCIE_HIGH_PERFORM_MODE = 1,
};

enum pcie_vote_lock_status {
    FORCE_WAKE_GOING,
    FORCE_WAKE_ENABLE,
    NOT_FORCE_AVAILABLE,
    NOT_FORCE_NOT_AVAILABLE,
};

enum rc_pcie_pm_msg {
    RC_PCIE_VOTE_TO_SLEEP = 1,
    RC_PCIE_SLOW_AWAKE_DONE = 2,
    RC_PCIE_SET_SLOW_L1SS = 3,
    RC_PCIE_PM_MSG_NUM
};

enum rc_pcie_pm_rm_event {
    RC_PCIE_SHUTDOWN = 1,
    RC_PCIE_RESET = 2,
    RC_PCIE_PM_RM_EVENT_NUM
};

enum ep_pcie_pm_msg {
    EP_PCIE_PM_LOCK_FREE = 1,
    EP_PCIE_PM_LOCK_BUSY = 2,
    EP_PCIE_PM_MSG_NUM
};

enum ep_pcie_pm_status {
    EP_PCIE_PM_START_LOCK = 1,
    EP_PCIE_PM_LOCK_RELEASE = 2,
    EP_PCIE_IN_SLOW_L1SS = 3,
    EP_PCIE_PM_STATUS_NUM
};

enum pcie_pm_error_state {
    RC_POWER_DOWN_FAIL,
    RC_POWER_UP_FAIL,
    RC_LINK_UP_FAIL,
    EP_POWER_DOWN_TIMEOUT,
    EP_WAIT_LINK_UP_TIMEOUT,
    RC_PCIE_BAR_SPACE_ERR,
    PCIE_LINKDOWN_DETECT,
    RC_POWERUP_WAIT_TIMEOUT,
    PCIE_PM_ERROR_MAX
};

enum pcie_state {
    PCIE_WAKE = 0,
    PCIE_SLEEP = 1,
    PCIE_STATE_NUM
};

enum pm_gpio_status {
    GPIO_CLEAR = 0,
    GPIO_SET = 1,
    GPIO_STATUS_NUM
};

enum pm_test_msg {
    START_EP_PM_TEST = 1,
    START_RC_PM_TEST = 2,
    STOP_PM_TEST = 3,
    CONTINUE_PM_TEST = 4,
    PM_TEST_MSG_NUM
};

enum pcie_pm_event {
    GET_API_SUSPEND,
    GET_API_RESUME,
    GET_PCIE_MSI,
    GET_GPIO,
    SUSPEND_TIMEOUT,
    UNLOCK_CLEAR,
    PRE_POWERUP,
    PCIE_PM_EVENT_MAX
};

enum pcie_pm_state_type {
    CURRENT_STATE,
    INNER_EVENT,
    OUTER_EVENT,
    PCIE_PM_STATE_TYPE_MAX
};

enum pcie_wakelock_id {
    PCIE_PM_DRV,
    PCIE_PM_API,
    PCIE_WAKELOCK_ID_MAX
};

enum pcie_pm_state_id {
    PCIE_PM_STATE_UNINITED,
    PCIE_PM_STATE_RESETTING,
    PCIE_PM_STATE_LINK_UP,
    PCIE_PM_STATE_LINK_IDLE,
    PCIE_PM_STATE_PRESUSPEND,
    PCIE_PM_STATE_POWER_DOWN,
    PCIE_PM_STATE_PRE_POWER_UP,
    PCIE_PM_STATE_RC_CLK_READY,
    PCIE_PM_STATE_EP_PCIE_READY,
    PCIE_PM_STATE_POWER_UP,
    PCIE_PM_STATE_MAX_NUM
};

enum pcie_pm_gpio {
    AP_WAKE_MODEM_GPIO,
    MODEM_WAKE_AP_GPIO,
    AP_STATUS_GPIO,
    MODEM_STATUS_GPIO,
    PCIE_PM_GPIO_MAX_NUM
};

enum pcie_wakeup_type {
    PCIE_WAKEUP_TRY   = 0,
    PCIE_WAKEUP_FORCE = 1,
    PCIE_WAKEUP_BUTT
};

/* ******Definition in pcie kernel stage****** */
// used for ep to rc multi msi advanced function
enum pcie_msi_num {
    PCIE_EP_2MSI = 2,
    PCIE_EP_4MSI = 4,
    PCIE_EP_8MSI = 8,
    PCIE_EP_16MSI = 16,
    PCIE_EP_32MSI = PCIE_EP_MAX_MSI_NUM,
};

// pcie ep user bar id, bar6 or more is controlled by other func ep dev
enum pcie_bar_id_e {
    PCIE_BAR_GIC_MSI = 0,
    PCIE_BAR_MSI_TO_RC = 1,
    PCIE_BAR_CHAR_DEV = 2,
    PCIE_BAR_ETH_DEV = 3,
    PCIE_BAR_EP_DMA_CTL = 4,
    PCIE_BAR_ICC = 5,
    PCIE_BAR_RFS = 6,
    PCIE_BAR_INTX = 7,
    PCIE_BAR_PCIE_TEST = 8,
    PCIE_BAR_DUMP = 9,
    PCIE_BAR_PM_CTL = 10,
    PCIE_BAR_MAX_NUM = 48
};

// pcie ep callback stage id
enum pcie_ep_callback_id {
    PCIE_EP_CB_BAR_CONFIG = 0,
    PCIE_EP_CB_PCIE_INIT_DONE = 1,
    PCIE_EP_CB_EXIT = 2,
    PCIE_EP_CB_LINKDOWN  = 3,
    PCIE_EP_CB_SUSPEND = 4,
    PCIE_EP_CB_RESUME = 5,
    PCIE_EP_CB_POWER_UP = 6,
    PCIE_EP_CB_PRE_LINKUP = 7,
    PCIE_EP_CB_POWER_DOWN = 8,
    PCIE_EP_CB_NUM
};

// pcie rc callback stage id
enum pcie_rc_callback_id {
    PCIE_RC_CB_ENUM_DONE = 0,
    PCIE_RC_CB_EXIT = 1,
    PCIE_RC_CB_LINKDOWN  = 2,
    PCIE_RC_CB_SUSPEND = 3,
    PCIE_RC_CB_RESUME = 4,
    PCIE_RC_CB_POWER_UP = 5,
    PCIE_RC_CB_POWER_DOWN = 6,
    PCIE_RC_CB_NUM
};

enum pcie_rc_boot_stage {
    PCIE_BOOT_LINK,
    PCIE_LOAD_IMAGE_DONE,
    PCIE_SEND_GPIO,
    PCIE_RECEIVE_GPIO,
    PCIE_KERNEL_INIT_DONE,
    PCIE_PM_INIT_DONE,
    PCIE_USER_INIT_DONE,
    PCIE_BOOT_STAGE_NUM,
};


// pcie user id for callback and pcie pm
enum pcie_user_id {
    PCIE_USER_DRV = 0,
    PCIE_USER_EP_PM = 1,
    PCIE_USER_RC_PM = 2,
    PCIE_USER_DUMP = 3,
    PCIE_USER_CHAR_DEV = 4,
    PCIE_USER_RFS = 5,
    PCIE_USER_ETH_DEV = 6,
    PCIE_USER_ICC = 7,
    PCIE_USER_TEST = 8,
    PCIE_USER_MBOOT = 9,
    PCIE_USER_RESET = 10,
    PCIE_USER_TEMP = 11,
    PCIE_USER_PM_TEST = 12,
    PCIE_USER_SPEED_CTRL = 13,
    PCIE_USER_NUM
};

typedef int (*pcie_callback)(u32, u32, void*);

struct pcie_callback_info {
    pcie_callback callback; /* NULL for sync transfer */
    void *callback_args;
};

struct pcie_callback_info_list {
    struct pcie_callback_info list[PCIE_USER_NUM];
};

enum pcie_dma_chn_id {
    PCIE_DMA_CHAR_DEV = 0,
    PCIE_DMA_ETH_DEV = 1,
    PCIE_DMA_ICC = 2,
    PCIE_DMA_RFS = 3,
    PCIE_DMA_DUMP = 4,
    PCIE_DMA_TEST = 5,
    PCIE_DMA_ETH_DEV2 = 6,
    PCIE_DMA_CHN_MAX = 7,
    PCIE_DMA_CHN_NUM
};

enum pcie_dma_transfer_type {
    PCIE_DMA_NORMAL_MODE = 0,
    PCIE_DMA_LINK_MODE = 1,
};

// pcie rc to ep msi user id
enum pcie_rc_to_ep_msi_id {
    PCIE_RC_MSI_RFS = 0,
    PCIE_RC_MSI_CHAR_DEV = 1,
    PCIE_RC_MSI_ETH_DEV = 2,
    PCIE_RC_MSI_ICC = 3,
    PCIE_RC_MSI_DIAG = 4,
    PCIE_RC_MSI_DUMP = 5,
    PCIE_RC_MSI_USER_INIT = 6,
    PCIE_RC_MSI_PM_TEST = 7,
    PCIE_RC_MSI_PM = 8,
    PCIE_RC_MSI_TEST_BEGIN = 9,
    PCIE_RC_MSI_TEST_END = PCIE_RC_MSI_NUM - 1,
    PCIE_RC_MSI_MAX_ID = PCIE_RC_MSI_NUM,
};

// pcie ep to rc msi user id
enum pcie_ep_to_rc_msi_id {
    PCIE_EP_MSI_RFS = 0,
    PCIE_EP_MSI_CHAR_DEV = 1,
    PCIE_EP_MSI_ETH_DEV = 2,
    PCIE_EP_MSI_ICC = 3,
    PCIE_EP_MSI_DIAG = 4,
    PCIE_EP_MSI_DUMP = 5,
    PCIE_EP_MSI_RESET = 6,
    PCIE_EP_MSI_DMA_WRITE = 7,
    PCIE_EP_MSI_DMA_READ = 8,
    PCIE_EP_MSI_PM_STATUS = 9,
    PCIE_EP_MSI_PM_TEST = 10,
    PCIE_EP_WAKE_RC_USER_MSI = 11,
    PCIE_EP_MSI_PM = 12,  // make sure pm is the last one before test case
    PCIE_EP_MSI_TEST_BEGIN = 13,
    PCIE_EP_MSI_TEST_END = PCIE_EP_MSI_NUM - 1,
    PCIE_EP_MSI_MAX_ID = PCIE_EP_MSI_NUM,
};

/* ****************************************************************************
     *                          Attention                          *
* *****************************************************************************
* Description : Driver for pcie boot definition
* Core        : Acore、Fastboot
* Header File : the following head files need to be modified at the same time
*             : /acore/bsp_pcie.h
*             : /fastboot/bsp_pcie.h
* ***********************************Start*************************************/
/* ******Definition in pcie boot stage****** */
enum pcie_msi_cmd_id {
    PCIE_MSI_ID_IDLE = 0,
    PCIE_MSI_ID_SYSBOOT = 1,
    PCIE_MSI_ID_DUMP = 2,
    PCIE_MSI_ID_DDR_TEST = 3,
    PCIE_MSI_ID_VERSION = 4,
    PCIE_MSI_ID_SLICE = 5,
    PCIE_MSI_ID_FB_CMD = 6,
    PCIE_MSI_ID_NUM
};

enum pcie_boot_user_id {
    PCIE_BOOT_SYSOOT = 0,
    PCIE_BOOT_DUMP = 1,
    PCIE_BOOT_USER_NUM
};

enum pcie_boot_callback_id {
    PCIE_BOOT_INIT_DONE = 0,
    PCIE_BOOT_EXIT = 1,
    PCIE_BOOT_SHUTDOWN  = 2,
    PCIE_BOOT_CB_NUM
};

struct pcie_trans_msg_info {
    enum pcie_msi_cmd_id msi_id;
    u32 cmd_len;
    void *cmd_data_addr;
};

enum pcie_boot_bar_id {
    PCIE_BAR_BOOT_LOAD = 0,
    PCIE_BAR_BOOT_MSG = 1,
    PCIE_BAR_BOOT_NUM
};

/* ******definition in pcie boot stage****** */
int bsp_pcie_boot_init(void);
int bsp_pcie_resource_init(void);
void bsp_pcie_resource_exit(void);
int bsp_pcie_boot_enumerate(void);
unsigned long bsp_pcie_boot_get_bar_addr(enum pcie_boot_bar_id bar_index);
unsigned long bsp_pcie_boot_get_bar_size(enum pcie_boot_bar_id bar_index);
void bsp_pcie_boot_notify(void);
int bsp_pcie_boot_rescan(void);
int bsp_pcie_boot_driver_uninstall(void);
int bsp_pcie_boot_cb_register(enum pcie_boot_user_id usr_id,  struct pcie_callback_info *info);
int bsp_pcie_boot_cb_run(enum pcie_boot_callback_id callback_stage);
int bsp_pcie_rc_send_msg(struct pcie_trans_msg_info *msg_info);
int bsp_pcie_rc_read_msg(struct pcie_trans_msg_info *msg_info);
void bsp_pcie_rc_read_clear(void);
int bsp_pcie_receive_msi_request(enum pcie_msi_cmd_id id, irq_handler_t handler, const char *name, void *data);
void bsp_pcie_boot_msi_free(enum pcie_msi_cmd_id id);
void bsp_pcie_msg_space_debug(void);

void bsp_modem_boot_shutdown_handle(void);
void bsp_modem_load_shutdown_handle(void);

/* ******definition in pcie speed control****** */
/* 使用此接口必须任务上下文调用，接口中有持票操作 */
int bsp_pcie_data_volume_set(enum pcie_user_id user_id, int data_volume);
/* 使用此接口必须保证PCIE 用户已经持票，或者在PCIE接口的回调中使用 */
int bsp_pcie_data_volume_set_atomic(enum pcie_user_id user_id, int data_volume);
/* 使用此接口必须任务上下文调用，接口中有持票操作 */
int bsp_pcie_app_clk_req(enum pcie_user_id user_id);
/* 使用此接口必须任务上下文调用，接口中有持票操作 */
int bsp_pcie_app_clk_release(enum pcie_user_id user_id);

/* ******definition in pcie rc****** */
unsigned long bsp_pcie_rc_get_bar_addr(enum pcie_bar_id_e bar_index);
unsigned long bsp_pcie_rc_get_bar_size(enum pcie_bar_id_e bar_index);
int bsp_pcie_rc_msi_request(enum pcie_ep_to_rc_msi_id id, irq_handler_t handler, const char *name, void *data);
int bsp_pcie_rc_msi_free(enum pcie_ep_to_rc_msi_id id);
int bsp_pcie_rc_msi_enable(enum pcie_ep_to_rc_msi_id id);
int bsp_pcie_rc_msi_disable(enum pcie_ep_to_rc_msi_id id);
int bsp_pcie_rc_send_msi(enum pcie_rc_to_ep_msi_id id);
int bsp_pcie_rc_cb_register(enum pcie_user_id usr_id,  struct pcie_callback_info *info);
int bsp_pcie_rc_cb_run(enum pcie_rc_callback_id callback_stage);
void bsp_pcie_rc_irq_disable(void);
void bsp_pcie_rc_irq_enable(void);
int bsp_is_pcie_first_user(enum pcie_ep_to_rc_msi_id msi_id);
int bsp_pcie_modem_kernel_init(void);
int bsp_kernel_pcie_remove(void);
int bsp_pcie_rc_notify_ep_user_init(void);
void bsp_pcie_rc_pm_test_init_register(void);
void bsp_pcie_rc_test_init_register(void);

int bsp_pcie_rc_vote_unlock(enum pcie_user_id user_id);
int bsp_pcie_rc_vote_lock(enum pcie_user_id user_id, int force_wake);
int bsp_pcie_rc_vote_lock_timeout(enum pcie_user_id user_id,  long jiffies);
int bsp_pcie_rc_pm_init(void);
int bsp_pcie_rc_pm_api_init(void);
void bsp_pcie_rc_pm_remove(enum rc_pcie_pm_rm_event rm_event);
int bsp_pcie_pm_modem_reset_cb(void);

int bsp_pcie_rc_get_pm_status(void);
int bsp_pcie_check_vote_clear(u32 wait_ms);

/* ******依赖于ep->rc msi使能，当前暂不可用****** */
int bsp_pcie_rc_dma_isr_register(enum pcie_dma_chn_id chn, u32 direction, pcie_callback call_back, void* arg);

int bsp_pcie_ep_msi_request(enum pcie_rc_to_ep_msi_id id, irq_handler_t handler, const char *name, void *data);
int bsp_pcie_ep_msi_free(enum pcie_rc_to_ep_msi_id id);
int bsp_pcie_ep_msi_enable(enum pcie_rc_to_ep_msi_id id);
int bsp_pcie_ep_msi_disable(enum pcie_rc_to_ep_msi_id id);
int bsp_pcie_ep_callback_msi_register(void);
int bsp_pcie_ep_cb_register(enum pcie_user_id usr_id,  struct pcie_callback_info *info);
int bsp_pcie_ep_cb_run(enum pcie_ep_callback_id callback_stage);
int bsp_pcie_ep_vote_unlock(enum pcie_user_id user_id);
int bsp_pcie_ep_vote_lock(enum pcie_user_id user_id, int force_wake);
int bsp_pcie_ep_pm_init(void);
int bsp_pcie_ep_pm_api_init(void);
int bsp_pcie_ep_get_pm_status(void);
int bsp_pcie_ep_vote_lock_timeout(enum pcie_user_id user_id,  long jiffies);

int bsp_pcie_pm_resume(void);
int bsp_pcie_pm_suspend(void);
void *bsp_pcie_pm_get_bar_virt_addr(void);
int bsp_pcie_dump_init(void);
void bsp_pcie_boot_dbg(unsigned int boot_stage);

#endif
