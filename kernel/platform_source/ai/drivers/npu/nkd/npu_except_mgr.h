#ifndef _NPU_EXCEPT_MGR_H_
#define _NPU_EXCEPT_MGR_H_

#include "npu_common.h"

#define NPU_GLOB_EXCEPT_MASK_ALL         0x3
#define NPU_GLOB_EXCEPT_MASK_LITE        0x1

// consts for the source/reporter/originator of an exception
enum npu_exception_source {
	NPU_EXCEPTION_SRC_GLOBAL_REG = 0, // read GLOBAL_REG to know NPU error triggered by other OS
	NPU_EXCEPTION_SRC_TS, // task scheduler in HW/SW format
	NPU_EXCEPTION_SRC_QIC, // QoS aware InterConnect, used NoC(Network on Chip)before v210
	NPU_EXCEPTION_SRC_TASK_TIMEOUT, // receiver thread waits timeout
	NPU_EXCEPTION_SRC_SERVER, // server process released
	NPU_EXCEPTION_SRC_MAX
};

// constants of the result/reply code for NPU Exception Handler
typedef enum _neh_reply {
	NPU_EXCEPTION_CONTINUE = 0, // continue to dispatch to next handler
	NPU_EXCEPTION_HANDLED,
} neh_reply_t;

struct npu_exception_record {
	enum npu_exception_source ex_source;
	union {
		struct hwts_exception_report_info *ex_hwts_report;
	} u;
	// handling notes in bit masks
	u32 ex_note_powered_down:1;
	u32 ex_note_power_up:1;
};

void npu_raise_timeout_exception(struct npu_dev_ctx *dev_ctx);
void npu_raise_hwts_exception(struct npu_dev_ctx *dev_ctx, struct hwts_exception_report_info *report);
void npu_raise_qic_exception(struct npu_dev_ctx *dev_ctx);
void npu_raise_global_reg_exception(struct npu_dev_ctx *dev_ctx);
void npu_raise_server_released_exception(struct npu_dev_ctx * dev_ctx);

int32_t npu_get_exception_bits(void);
void npu_set_exception_bits(void);
void npu_clear_exception_bits(void);

#endif // _NPU_EXCEPT_MGR_H_