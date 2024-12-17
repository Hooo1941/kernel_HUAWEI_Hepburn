#include <linux/types.h>
#include <platform_include/basicplatform/linux/rdr_platform.h>
#include <platform_include/basicplatform/linux/rdr_pub.h>
#include <platform_include/basicplatform/linux/rdr_ap_hook.h>
#include <platform_include/basicplatform/linux/dfx_bbox_diaginfo.h>
#include <platform_include/basicplatform/linux/mntn_record_sp.h>
#include <linux/init.h>
#include <linux/printk.h>
#include <linux/kernel.h>
#include <platform_include/basicplatform/linux/hck/mntn/hck_dfx_bbox.h>
#include <linux/version.h>
#include <linux/sched/debug.h>
#include <asm/system_misc.h>

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0))
static DEFINE_PER_CPU(unsigned int, hck_show_reg_cpumask);

static void hck_dfx_bbox_ipi_cpu_stop(bool state)
{
	if (likely(state))
		dfx_ipi_cpu_stop();
}

static void hck_dfx_bbox_merr_show_regs(bool state)
{
	if (likely(state)) {
		reentrant_exception();
		panic_show_regs();
	}
}

static void hck_dfx_bbox_get_cpumask(bool state)
{
	if (likely(state)) {
		unsigned int cpu_mask = 0x1 << get_cpu();
		__this_cpu_write(hck_show_reg_cpumask, cpu_mask);
	}
}

static void hck_dfx_bbox_show_reg_and_put_cpu(struct pt_regs *regs, u32 size)
{
	if (!(g_cpu_in_ipi_stop & __this_cpu_read(hck_show_reg_cpumask)))
		show_extra_register_data(regs, size);
	printk("\n");
	put_cpu();
}


static void hck_dfx_bbox_print_pc(struct pt_regs *regs, u64 lr)
{
	printk("pc : [<%016llx>] lr : [<%016llx>] pstate: %08llx\n",
		regs->pc, lr, regs->pstate);
}

static void hck_dfx_bbox_print_current(struct task_struct *cur)
{
	printk("TGID: %d Comm: %.20s\n", cur->tgid, cur->group_leader->comm);
}

static void hck_dfx_bbox_panic_on_nowarn(bool state)
{
#ifdef CONFIG_DFX_BB_DEBUG
	if (state)
		panic("scheduling while atomic\n");
#else
	(void)state;
#endif
}

static void show_regs_before_dump_stack(struct pt_regs *regs)
{
	dump_stack();
	__show_regs(regs);
}

static int __init hck_dfx_bbox_register(void)
{
	REGISTER_HCK_VH(hck_dfx_bbox_page_trace_hook, page_trace_hook);
	REGISTER_HCK_VH(hck_dfx_bbox_kmalloc_trace_hook, kmalloc_trace_hook);
	REGISTER_HCK_VH(hck_dfx_bbox_vmalloc_trace_hook, vmalloc_trace_hook);
	REGISTER_HCK_VH(hck_dfx_bbox_time_hook, time_hook);
	REGISTER_HCK_VH(hck_dfx_bbox_irq_register_hook, irq_register_hook);
	REGISTER_HCK_VH(hck_dfx_bbox_set_exception_info, set_exception_info);
	REGISTER_HCK_VH(hck_dfx_bbox_softirq_hook, softirq_hook);
	REGISTER_HCK_VH(hck_dfx_bbox_ipi_cpu_stop, hck_dfx_bbox_ipi_cpu_stop);
	REGISTER_HCK_VH(hck_dfx_bbox_merr_show_regs, hck_dfx_bbox_merr_show_regs);
	REGISTER_HCK_VH(hck_dfx_bbox_get_cpumask, hck_dfx_bbox_get_cpumask);
	REGISTER_HCK_VH(hck_dfx_bbox_show_reg_and_put_cpu, hck_dfx_bbox_show_reg_and_put_cpu);
	REGISTER_HCK_VH(hck_dfx_bbox_task_switch_hook, task_switch_hook);
	REGISTER_HCK_VH(hck_dfx_bbox_tasklet_hook, tasklet_hook);
	REGISTER_HCK_VH(hck_dfx_bbox_cpu_up_diaginfo_record, cpu_up_diaginfo_record);
	REGISTER_HCK_VH(hck_dfx_bbox_irq_trace_hook, irq_trace_hook);
	REGISTER_HCK_VH(hck_dfx_bbox_print_pc, hck_dfx_bbox_print_pc);
	REGISTER_HCK_VH(hck_dfx_bbox_print_current, hck_dfx_bbox_print_current);
	REGISTER_HCK_VH(hck_dfx_bbox_worker_hook, worker_hook);
	REGISTER_HCK_VH(hck_dfx_bbox_panic_on_nowarn, hck_dfx_bbox_panic_on_nowarn);
	REGISTER_HCK_VH(hck_dfx_bbox_show_stack_othercpus, mntn_show_stack_othercpus);
	REGISTER_HCK_VH(hck_dfx_bbox_serror_dump, show_regs_before_dump_stack);
	return 0;
}
early_initcall(hck_dfx_bbox_register);
#endif
