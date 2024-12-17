#ifndef __HCK_DFX_BBOX__
#define __HCK_DFX_BBOX__

#include <linux/types.h>
#include <platform_include/basicplatform/linux/hck_vendor_hooks.h>
/*
 * Following tracepoints are not exported in tracefs and provide a
 * mechanism for vendor modules to hook and extend functionality
 */

struct page;
struct pt_regs;
struct task_struct;

DECLARE_HCK_VH(hck_dfx_bbox_page_trace_hook,
	TP_PROTO(gfp_t gfp_flag, u8 action, u64 caller, struct page *page, u32 order),
	TP_ARGS(gfp_flag, action, caller, page, order));

DECLARE_HCK_VH(hck_dfx_bbox_kmalloc_trace_hook,
	TP_PROTO(u8 action, u64 caller, u64 va_addr, u64 phy_addr, u32 size),
	TP_ARGS(action, caller, va_addr, phy_addr, size));

DECLARE_HCK_VH(hck_dfx_bbox_vmalloc_trace_hook,
	TP_PROTO(u8 action, u64 caller, u64 va_addr, struct page *page, u64 size),
	TP_ARGS(action, caller, va_addr, page, size));

DECLARE_HCK_VH(hck_dfx_bbox_time_hook,
	TP_PROTO(u64 address, u32 dir),
	TP_ARGS(address, dir));

DECLARE_HCK_VH(hck_dfx_bbox_irq_register_hook,
	TP_PROTO(struct pt_regs *reg),
	TP_ARGS(reg));

DECLARE_HCK_VH(hck_dfx_bbox_set_exception_info,
	TP_PROTO(unsigned long address),
	TP_ARGS(address));

DECLARE_HCK_VH(hck_dfx_bbox_softirq_hook,
	TP_PROTO(u32 softirq_type, u64 address, u32 dir),
	TP_ARGS(softirq_type, address, dir));

DECLARE_HCK_VH(hck_dfx_bbox_ipi_cpu_stop,
	TP_PROTO(bool state),
	TP_ARGS(state));

DECLARE_HCK_VH(hck_dfx_bbox_merr_show_regs,
	TP_PROTO(bool state),
	TP_ARGS(state));

DECLARE_HCK_VH(hck_dfx_bbox_get_cpumask,
	TP_PROTO(bool state),
	TP_ARGS(state));

DECLARE_HCK_VH(hck_dfx_bbox_show_reg_and_put_cpu,
	TP_PROTO(struct pt_regs *regs, u32 size),
	TP_ARGS(regs, size));

DECLARE_HCK_VH(hck_dfx_bbox_task_switch_hook,
	TP_PROTO(const void *pre_task, void *next_task),
	TP_ARGS(pre_task, next_task));

DECLARE_HCK_VH(hck_dfx_bbox_tasklet_hook,
	TP_PROTO(u64 address, u32 dir),
	TP_ARGS(address, dir));

DECLARE_HCK_VH(hck_dfx_bbox_cpu_up_diaginfo_record,
	TP_PROTO(u32 cpu, int status),
	TP_ARGS(cpu, status));

DECLARE_HCK_VH(hck_dfx_bbox_irq_trace_hook,
	TP_PROTO(u32 dir, u32 old_vec, u32 new_vec),
	TP_ARGS(dir, old_vec, new_vec));

DECLARE_HCK_VH(hck_dfx_bbox_print_pc,
	TP_PROTO(struct pt_regs *regs, u64 lr),
	TP_ARGS(regs, lr));

DECLARE_HCK_VH(hck_dfx_bbox_print_current,
	TP_PROTO(struct task_struct *cur),
	TP_ARGS(cur));

DECLARE_HCK_VH(hck_dfx_bbox_worker_hook,
	TP_PROTO(u64 address, u32 dir),
	TP_ARGS(address, dir));

DECLARE_HCK_VH(hck_dfx_bbox_panic_on_nowarn,
	TP_PROTO(bool state),
	TP_ARGS(state));

DECLARE_HCK_VH(hck_dfx_bbox_show_stack_othercpus,
	TP_PROTO(int cpu),
	TP_ARGS(cpu));

DECLARE_HCK_VH(hck_dfx_bbox_serror_dump,
	TP_PROTO(struct pt_regs *regs),
	TP_ARGS(regs));
#endif /* __HCK_DFX_BBOX__ */
