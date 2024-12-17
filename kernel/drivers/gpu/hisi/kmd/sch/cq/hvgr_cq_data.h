/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#ifndef HVGR_CQ_DATA_H
#define HVGR_CQ_DATA_H

#include <linux/version.h>
#include <linux/semaphore.h>
#include <linux/spinlock.h>
#include <linux/mutex.h>
#include <linux/workqueue.h>
#include <linux/sync_file.h>
#include <linux/completion.h>
#include <linux/rbtree.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
#include <linux/kthread.h>
#include <linux/idr.h>
#endif

#include "apicmd/hvgr_cq_defs.h"
#include "apicmd/hvgr_wq_defs.h"
#include "hvgr_softq.h"
#include "hvgr_msync_defs.h"
#include "hvgr_sch_policy_manager.h"

struct hvgr_ctx;
/**
 * CQ_STATUS_DIVE : This queue context is free.
 * CQ_STATUS_IDLE : This queue had been allocated, but not add into schedule.
 * CQ_STATUS_PENDING : This queue is not empty, and input to the pending list.
 * CQ_STATUS_RUNNING : This queue is binnding to the GPU.
 * CQ_STATUS_TIMEOUT : This queue is running timeout in GPU.
 * CQ_STATUS_STOPING : The STOP cmd sent to GPU and waiting for STOP interrupt.
 * CQ_STATUS_STOPED : CPU receive STOP interrupt.
 */
enum cq_status {
	CQ_STATUS_DIVE = 0,
	CQ_STATUS_IDLE,
	CQ_STATUS_PENDING,
	CQ_STATUS_RUNNING,
	CQ_STATUS_TIMEOUT,
	CQ_STATUS_STOPPING,
	CQ_STATUS_STOPPED,
	CQ_STATUS_ERROR,
};

enum cq_sch_state_status {
	HVGR_CQ_START_STATE =0,
	HVGR_CQ_ALLOC_STATE,
	HVGR_CQ_SUBMIT_STATE,
	HVGR_CQ_BIND_STATE,
	HVGR_CQ_UNBIND_STATE,
	HVGR_CQ_FREE_STATE,
};

struct cq_dfx_time {
	bool sch_is_free;
	bool submit_is_first;

	uint64_t alloc_time;
	uint64_t submit_time;
	uint64_t bind_time;
	uint64_t unbind_time;
	uint64_t free_time;
	uint64_t run_time;

	uint32_t bind_count;
	uint32_t unbind_count;
};

#define HVGR_THRO_ACTION_RESTORE_INITAL (0)
#define HVGR_THRO_ACTION_BIND           (1)
#define HVGR_THRO_ACTION_DO_STOP        (2)
#define HVGR_THRO_ACTION_JOB_BOUNDARY   (3)

#define CQ_TOKEN_NUMS                        TOKEN_NUMS
#define CQ_SCH_UNBIND_TIME_OUT               5
#define CQ_SCH_CL_UNBIND_TIME_OUT            5
#define CQ_SCH_CL_UNBIND_TIME_OUT_FPGA       20

#define CQ_KMD_CTX_START_SUBMIT              (1U << 0)
#define CQ_KMD_CTX_ERROR_FLAG                (1U << 1)
#define CQ_KMD_CTX_PAGE_FAULT                (1 << 2)
#define CQ_KMD_CTX_TIME_OUT_HANG             (1 << 3)
#define CQ_KMD_CTX_GPU_FAULT                 (1 << 4)
#define CQ_KMD_CTX_MOD_WORKMODE              (1 << 5)

#define HVGR_TOKEN_DEP_NUMS            8U
#define HVGR_FENCE_TIMEOUT_TIME        2000000000u /* 2s */
#define HVGR_FENCE_TIMEOUT_TIME_FPGA   20000000000u /* 20s */

#define HVGR_TOKEN_CB_WORK_RUNNING          (1ULL << 0)
#define HVGR_TOKEN_CB_RESOURCE              (1 << 1)
#define HVGR_TOKEN_CB_WAIT_FENCE            (1ULL << 2)
#define HVGR_TOKEN_CB_TRIGGER_FENCE         (1ULL << 3)
#define HVGR_TOKEN_HOLD_EN_IN               (1ULL << 4)
#define HVGR_TOKEN_CB_WQ_WAIT               (1ULL << 5)
#define HVGR_TOKEN_NORMAL_ENTRY             (1ULL << 6)
#define HVGR_TOKEN_SC_UPDATE_POLICY         (1ULL << 7)
#define HVGR_TOKEN_HOLD_EN_OUT              (1ULL << 8)
#define HVGR_TOKEN_FREQ_HINT                (1ULL << 9)
#define HVGR_TOKEN_NOTIFY_TOP               (1ULL << 10)
#define HVGR_TOKEN_CL_PROFILING_EXEC        (1ULL << 11)
#define HVGR_TOKEN_CL_PROFILING_COMPLETE    (1ULL << 12)

#define HVGR_TOKEN_FREQ_HIGH_START     (1)
#define HVGR_TOKEN_FREQ_MED_START      (2)
#define HVGR_TOKEN_FREQ_LOW_START      (3)
#define HVGR_TOKEN_FREQ_END            (4)

#define BASE_JM_MAX_NR_SLOTS            8
#define BASE_JM_SLOT0                   0
#define BASE_JM_SLOT1                   1
#define BASE_JM_SLOT2                   2
#define BASE_JM_SLOT3                   3
#define BASE_JM_SLOT4                   4
#define BASE_JM_SLOT5                   5
#define BASE_JM_SLOT6                   6
#define BASE_JM_SLOT7                   7
#define KMD_JOB_SLOT_MASK               ((u32)0xFF)
/* Bounding volume hierarchies generator job chain type */
#define BASE_JD_REQ_BVHG ((u32)1 << 19)

/* Ray tracing task controller job chain type */
#define BASE_JD_REQ_RTTC ((u32)1 << 20)
#define OPCODE2_SET_JC_FOR_PROTECT_MODE 100

enum base_cq_event_code {
	BASE_CQ_EVENT_NOT_STARTED = 0x00,
	BASE_CQ_EVENT_DONE = 0x01,
	BASE_CQ_EVENT_TERMINATED = 0x04,
	BASE_CQ_EVENT_JOB_CANCELLED = 0x05,
};

enum protect_flag {
	CQ_PROTECT_MODE_IN       = 0,
	CQ_PROTECT_MODE_OUT      = 1,
	CQ_PROTECT_MODE_INVALID  = 2,
};

enum cq_subchannel {
	SUB_CHANNEL_TYPE_RENDER = 0,
	SUB_CHANNEL_TYPE_BIN = 1,
	SUB_CHANNEL_TYPE_COMPUTE = 2,
	SUB_CHANNEL_TYPE_RT_TREE_BUILD = 3,
	SUB_CHANNEL_TYPE_RT_TRAVERSE = 4,
	SUB_CHANNEL_TYPE_MIN = SUB_CHANNEL_TYPE_RENDER,
	SUB_CHANNEL_TYPE_MAX = SUB_CHANNEL_TYPE_RT_TRAVERSE,
	SUB_CHANNEL_TYPE_WHOLE,
};

#define CQ_QUEUE_ENTRY_NUMBER             4096U
struct cq_cmdq_entry {
	uint32_t rsvd[8];
};

struct hvgr_cq_ctx {
	struct list_head pending_item;
	enum cq_status status;
	enum cq_sch_state_status cq_sch_state_type;
	/**
	 * flags : is bitset for kmd_cq_ctx object attribute.
	 * use defined CQ_KMD_CTX_ ...
	 * status : The CQ status.
	 * channel_id : The GPU physic channel for running.
	 */
	uint32_t flags;
	uint32_t channel_id;
	uint32_t priority;
	/* Running_counter : User for fault timer. */
	uint32_t running_counter;
	uint32_t semp_timeout_counter;
	/* Recorder previous index for fault timer. */
	uint32_t put_index;
	uint32_t get_index;
	/* Recorder index when unbind. */
	uint32_t put_index_unbind;
	uint32_t get_index_unbind;
	/* Pointer to SW Writeable context */
	struct cq_sw_writeable_context *sw_wr_ctx; /* for kernel space */
	uint64_t sw_ctx_addr; /* for user space */

	uint32_t queue_id;
	struct hvgr_ctx *ctx;
	struct hvgr_cq_status *cq_status;
	struct hvgr_mem_area *cq_status_area;
	uint32_t err_flags; /* for kernel space to judge the cq status */

	/*
	 * gpu_context_buffer is data for GPU context buffer.
	 * It contain SW Writeable buffer and Ring Buffer.
	 */
	struct hvgr_mem_area *gpu_context_buffer;

	/* work queue defs */
	struct hvgr_soft_q *sq_ctx;
	struct hvgr_mem_area *wq_mem_region;
	spinlock_t wq_kwq_lock;
	uint32_t wq_notify_count;
	volatile bool wq_kwq_is_running;
	struct workqueue_struct *wq_kwq;
	struct work_struct wq_kwq_handle;
	atomic_t wq_hold_count;

	/* Follow attributes are different between HW and cmodel. */
	wait_queue_head_t queue_done_wait;
	/* this timer just for, when gpu hang, backup to call schedule_out. */
	struct timer_list backup_timer;
	/* In protect mode ,this timer just for, when gpu hang, backup to call schedule_out. */
	struct timer_list backup_timer_protect_mode;
	bool empty_flag;
	bool cl_tried;
	bool dump_mem_flag;
#if defined(HVGR_SUPPORT_SPEC_LOW) && HVGR_SUPPORT_SPEC_LOW != 0
	bool mod_priv_his;
#endif
	struct mutex dump_mem_lock;
	uint32_t cur_dvfs_status;
	uint32_t pre_dvfs_status;

	/* Save channel_id and bufferuse status for the buf full irq asynchronous processing. */
	uint32_t last_channel_id;
	atomic_t is_order_buffer_inuse;
	atomic_t is_sync_buffer_inuse;
	struct mutex dump_order_lock;
	struct mutex dump_sync_lock;

#ifdef CONFIG_HVGR_DFX_MEM_DUMP
	struct work_struct queue_fault_work;
#endif
	struct hvgr_mem_area *queue_entry_area;
	struct cq_cmdq_entry *cq_entry_base;
	struct cq_dfx_time cq_schtime_dfx;
	uint64_t cq_entry_base_uva;

#ifdef CONFIG_DFX_DEBUG_FS
	struct work_struct order_dump_work;
	struct work_struct sync_dump_work;
#endif
};

struct hvgr_cq_semp_info {
	struct hvgr_mem_area *semp_buffer;
	u64  semp_kva;
	u64  semp_uva;
	uint32_t size;
};

struct token_wq_entry {
	struct list_head link;
	wq_work_entry wq_work;
	uint64_t jit_addrs[WQ_JIT_NUMS];
};

struct profiling_info {
	uint64_t wait_semp_addr;
	uint64_t wait_semp_value;
	uint64_t timestamp_addr;
	uint64_t soc_timestamp;
	uint64_t os_timestamp;
};

struct token_callback_data {
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0))
	struct work_struct work;
#else
	struct kthread_work work;
#endif
	spinlock_t lock;
	uint64_t flags;

	struct hvgr_ctx *ctx;
	struct hvgr_cq_ctx *cq_ctx;
	struct hvgr_msync cq_event;

	/* Use for extra memory data. */
	void *res_data;

	/* Use for trigger/wait fence. */
	struct dma_fence *fence_data;
	struct dma_fence_cb fence_cb;
	struct hrtimer fence_timeout_timer;
	bool ai_freq_enabled;

	/* Use for workqueue wait. */
	struct token_wq_entry *tw_entry;

	uint32_t dep_own_token[HVGR_TOKEN_DEP_NUMS];
	uint32_t dep_own_nums;
#ifdef CONFIG_DFX_DEBUG_FS
	/* When token in callback table, set flag will be set to 1,
	 * when del from callback table, set it to 0.
	 */
	uint32_t set_flag;
#endif
	/* Use for sc update policy data */
	void *policy_info;
	uint32_t policy_info_num;

	/* Use for freq hint data */
	uint32_t freq_hint;

	/* Use for cl profiling data */
	struct profiling_info profiling_data;

	/* token */
	uint32_t token;
};

/*
 * hvgr_cq_ctx_data is a part of hvgr_ctx. It contain the
 * CQ data that are belong this hvgr_ctx.
 */
struct hvgr_cq_ctx_data {
	struct hvgr_cq_ctx cq_ctx[CQ_QUEUE_NUM_PER_CTX];
	struct hvgr_cq_semp_info semp_info;

	uint64_t dma_fence_ctx;
	atomic_t dma_fence_seqno;

	spinlock_t callback_lock;
	struct token_callback_data *token_callback[CQ_TOKEN_NUMS];

	/* Use for workqueue atomic semp. */
	struct mutex as_lock[CQ_TOKEN_NUMS];
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0))
	struct workqueue_struct *token_callback_wq;
#else
	struct kthread_worker *token_callback_wq;
#endif

	/* list for workqueue entry */
	struct list_head wq_entry_list;

	char *uio_ptr;
	spinlock_t semp_lock;
	/* True means semaphore res exit, false means free. */
	bool semp_res_flag;
	/* Record fence totol cnt. */
	atomic_t trigger_total_cnt;
	atomic_t wait_total_cnt;
	/* Record whether fence cnt which not finish. */
	atomic_t trigger_uncomplte_cnt;
	atomic_t wait_uncomplte_cnt;
	/* Record whether fence cnt which has finish. */
	atomic_t trigger_complte_cnt;
	atomic_t wait_complte_cnt;

	spinlock_t event_lock;
	struct mutex event_data_lock;
	uint32_t token_finish_wr;
	uint32_t token_finish_rd;
	struct msync_event token_finish[CQ_TOKEN_NUMS];
	bool err_disp;

#ifdef HVGR_DATAN_JOB_TIMELINE
	/* Chain id buffer */
	uint32_t *chain_id_buf; /* for kernel space */
	uint64_t chain_id_addr; /* for user space */
	uint32_t size;          /* Chain id buffer size */
	/* Job chain start timestamp buffer */
	uint64_t *start_timestamp_kva; /* for kernel space */
	uint64_t start_timestamp_uva; /* for user space */
	uint32_t start_timestamp_size;          /* size */
	/* Job chain emd timestamp buffer */
	uint64_t *end_timestamp_kva; /* for kernel space */
	uint64_t end_timestamp_uva; /* for user space */
	uint32_t end_timestamp_size;          /* size */
#endif
};

struct cq_pending {
	struct list_head pending_list;
	uint32_t dequeue_counter;
	uint32_t weight;
	uint32_t real;
};

struct cq_channel {
	uint32_t channel_id;
	bool available;
	bool timer_running_flag;
	bool timer_should_run;
	struct hrtimer schedule_timer;
	struct hvgr_cq_ctx *running_queue;
	struct hvgr_cq_dev *global_cq_data;
};

enum schedule_timer_status {
	SCHEDULE_TIMER_STOP = 0,
	SCHEDULE_TIMER_START,
	SCHEDULE_TIMER_RUNNING,
};

/* fault by miscctrl will reset gpu direct, other's will try to softstop queue firstly. */
enum cq_fault_type {
	CQ_FAULT_NO_FAULT = 0,
	CQ_FAULT_NORMAL = CQ_FAULT_NO_FAULT, /* not miscctrl */
	CQ_FAULT_MISCCTRL = 1,
};

enum hvgr_cq_gpu_sch_status {
	/* The GPU is idle, no job running on gpu. */
	HVGR_GPU_IDLE,

	/*
	 * The GPU is running job now.
	 */
	HVGR_GPU_RUNNING,
};

struct cq_hw_cfg {
	u32 work_mode;
	u32 cq_sch_mode;
	u32 bc_send_mode;
	u32 rt_rbc_send_mode;
	u32 cq_semp_wait_timeout_switch;
	u32 cq_serial_timeslice;
	u32 cq_multi_chain_enable;
	u32 cq_bind_waitcycle;
	u32 cq_queue_empty_timeout;
	u32 cq_semp_wait_timeout;
	u32 cq_schedule_unbind_time;
	u32 cq_cl_affinity;
	bool cq_entry_boundary;
	bool cq_order_dump;
	bool cq_sync_dump;
};

enum cq_enter_protected_state {
	HVGR_CQ_ENTER_PROTECTED_CHECK = 0,
	HVGR_CQ_ENTER_PROTECTED_WAIT_IDLE,
	/* Write reg to enter protect mode. */
	HVGR_CQ_ENTER_PROTECTED_CONFIG,
	/* End state: Prepare coherency change. */
	HVGR_CQ_ENTER_PROTECTED_FINISHED,
};

enum cq_exit_protected_state {
	HVGR_CQ_EXIT_PROTECTED_CHECK = 0,
	HVGR_CQ_EXIT_PROTECTED_WAIT_IDLE,
	/* Wait for the L2 to become idle in preparation for the reset. */
	/* Issue the protected reset. */
	HVGR_CQ_EXIT_PROTECTED_RESET,
	/* End state: Wait for the reset to complete. */
	HVGR_CQ_EXIT_PROTECTED_RESET_WAIT,
};

struct cq_protect_mode {
	bool protected_mode;
	bool protected_mode_transition;
	u32 protected_mode_refcount;

	/* record the queue need running in protect. */
	struct hvgr_cq_ctx *running_queue;
	/* When we received two hold_en at the same time, we also need save the second queue. */
	struct hvgr_cq_ctx *bak_running_queue[CQ_CHANNEL_NUM - 1];
	uint32_t get_index;  /* record running queue get_index when into protect mode. */
	uint32_t get_index_bak;  /* record bak running queue get_index when into protect mode. */
	union {
		enum cq_enter_protected_state enter;
		enum cq_exit_protected_state exit;
	} protected_state;
};
/**
 * hvgr_cq_dev is a part of hvgr_device. It contain
 * the CQ data of this GPU device.
 */
struct hvgr_cq_dev {
	spinlock_t schedule_lock;
	spinlock_t cq_mmu_lock; /* cq mode, will lock mmu. */
	struct mutex submit_lock;
	struct mutex err_recover_lock;

	struct cq_channel  channel_info[CQ_CHANNEL_NUM];
	struct cq_pending pending_queue[CQ_PRIORITY_MAX];
	uint32_t real_weight_sum;
	uint32_t no_real_weight_sum;
	uint32_t real_counter_sum;
	uint32_t no_real_counter_sum;
	long reset_timeout_normal;
	long reset_timeout_cl;
	enum cq_fault_type fault_type;
	uint32_t free_as;  /* This is a bit set for free ASID */

	bool submit_allow;
	atomic_t sch_switch_kref;
	wait_queue_head_t stop_queue_done_wait;
	enum hvgr_cq_gpu_sch_status gpu_sch_sts;
	struct cq_hw_cfg hw_cfg;
	struct cq_protect_mode protect_mode_info;

	/* This used to cq schedule call pm power on interface. */
	atomic_t pm_pwr_flag;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0))
	struct workqueue_struct *sched_pm_wq;
	struct work_struct sched_pm_work;
#else
	struct kthread_worker *sched_pm_wq;
	struct kthread_work sched_pm_work;
#endif

	uint32_t channel_sche_times;
	struct workqueue_struct *queue_fault_wq;
	int collect_pid;
	uint32_t order_dump_pid;
	uint32_t sync_dump_pid;
	struct workqueue_struct *entry_dump_wq;

	struct hvgr_sch_policy_opts policy_opts[HVGR_POLICY_MAX];
	uint32_t cur_policy;
	bool fence_timeout_enable;
	bool semp_timeout_enable;
	bool entry_timeout_switch;
	bool cq_suspend;
	bool sch_time_enable;

	/* Just make sure submit reg write only by one at the same time. */
	spinlock_t write_submit_lock;
	struct hvgr_ctx *jcd_ctx[BASE_JM_MAX_NR_SLOTS];
	uint32_t jcd_count;
	spinlock_t protect_mode_switch;

#if defined(HVGR_SUPPORT_SPEC_LOW) && HVGR_SUPPORT_SPEC_LOW != 0
	struct hvgr_cq_ctx  *cq_spec_low;
#endif
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
	struct ida queue_ida;
#endif
};

struct cq_cmdq_inline_head_protect {
	uint32_t sync : 1;
	uint32_t in_line : 1;
	uint32_t reserve1 : 30;
};

struct cq_cmdq_protect_info {
	struct cq_cmdq_inline_head_protect head;

	uint32_t opcode1 : 1;
	uint32_t opcode2 : 10;
	uint32_t i_mm : 1;
	uint32_t sub_channel_enable : 1;
	uint32_t sub_channel_type : 3;
	uint32_t reserve : 16;

	uint32_t sem_addr_low;
	uint32_t sem_addr_high : 16;
	uint32_t reserve1 : 16;

	uint32_t reserve2[4];
};

struct cq_cmdq_inline_notify_protect {
	struct cq_cmdq_inline_head_protect head;

	uint32_t opcode1 : 1;
	uint32_t opcode2 : 10;
	uint32_t i_mm : 1;
	uint32_t sub_channel_enable : 1;
	uint32_t sub_channel_type : 3;
	uint32_t signal_op : 3;
	uint32_t condition : 13;

	uint32_t reserve0;

	uint32_t reserve1 : 16;
	uint32_t irq_en : 1;
	uint32_t hold_en : 1;
	uint32_t token : 14;

	uint32_t reserve[4];
};

struct base_jcd {
	uint64_t reserve[8];
};
#endif
