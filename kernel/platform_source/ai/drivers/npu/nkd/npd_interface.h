#ifndef __NPD_INTERFACE_H__
#define __NPD_INTERFACE_H__

#include "npu_proc_ctx.h"
#include "npu_rt_task.h"
#include "npu_except_mgr.h"

#define NPD_REGISTRY_V1 1

//
// bit mask for chip-uarch dependent features.
// complex features should use npd_query_sku_value interface
//
#define NPU_FEATUE_GLOBAL_EXCEPTION_REG       0x1
// other static features can be defined here

// data structure for NPD(NPU Platform Driver) registry
struct npd_registry {
	u8 version; // version of this structure
	u8 npu_arch_version;
	u8 npu_chip_version;
	u8 reserved_u8;
	u32 feature_mask; // configurable features
	u32 reserved_u32;
	int (*npd_dump_exception)(struct npu_dev_ctx *dev_ctx, struct npu_exception_record* er);
	int (*npd_handle_exception)(struct npu_dev_ctx *dev_ctx, struct npu_exception_record* er);
	int (*npd_switch_unit)(struct npu_dev_ctx *dev_ctx, enum npu_sfu_e sfu, enum npu_workmode_flag mode, void *para);
	bool (*npd_query_sku_value)(enum npu_sku_var_e sv);
	int (*npd_send_mail)(void *mailbox, unsigned long mailbox_len, const void *msg, unsigned long msg_len);
	int (*npd_verify_sink_tasks)(struct npu_proc_ctx *proc_ctx, void *stream_buf_addr, u64 ts_sq_len, u32 *core_type);
	int (*npd_format_hw_sqe)(npu_model_desc_info_t *model_desc_info, void *stream_buf_addr, void *hwts_sq_addr,
							u64 ts_sq_len);
	int (*npd_format_sw_sqe)(npu_model_desc_t *model_desc, void *stream_buf_addr, u16 *first_task_id, u8 devid,
							int stream_idx);
	int (*npd_set_resmem)(struct npu_platform_info *plat_info);
	int (*npd_notify)(struct npu_dev_ctx *dev_ctx, npd_notice_t pn, long para1, long para2);
	int (*npd_nosec_get_workmode)(struct npu_proc_ctx *proc_ctx, npu_rt_task_t *comm_task);
};

int nkd_register_plat_driver(struct npd_registry *npd);
int nkd_unregister_plat_driver(void);

#endif // __NPD_INTERFACE_H__
