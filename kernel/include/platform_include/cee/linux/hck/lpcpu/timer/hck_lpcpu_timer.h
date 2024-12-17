#ifndef __HCK_LPCPU_TIMER__
#define __HCK_LPCPU_TIMER__

#include <linux/types.h>
#include <linux/compiler_types.h>
#include <platform_include/basicplatform/linux/hck_vendor_hooks.h>
/*
 * Following tracepoints are not exported in tracefs and provide a
 * mechanism for vendor modules to hook and extend functionality
 */

struct sp804_clkevt;
struct sp804_timer;
DECLARE_HCK_VH(lpcpu_timer_sp804_clock_init,
	       TP_PROTO(struct sp804_clkevt *clkevt, int *hook_flag),
	       TP_ARGS(clkevt, hook_flag));
DECLARE_HCK_VH(lpcpu_timer_sp804_shutdown,
	       TP_PROTO(struct sp804_clkevt *common_clkevt, int *hook_flag),
	       TP_ARGS(common_clkevt, hook_flag));
DECLARE_HCK_VH(lpcpu_timer_sp804_clockevents_init,
	       TP_PROTO(struct sp804_clkevt *common_clkevt, int *hook_flag),
	       TP_ARGS(common_clkevt, hook_flag));
DECLARE_HCK_VH(lpcpu_timer_sp804_timers_disable,
	       TP_PROTO(void __iomem *timer_base, int timer_ctrl, int *hook_flag),
	       TP_ARGS(timer_base, timer_ctrl, hook_flag));
DECLARE_HCK_VH(lpcpu_timer_sp804_of_timers_disable,
	       TP_PROTO(void __iomem *timer1_base, void __iomem *timer2_base,
			int timer_ctrl, int *hook_flag),
	       TP_ARGS(timer1_base, timer2_base, timer_ctrl, hook_flag));
DECLARE_HCK_VH(lpcpu_timer_sp804_clkevt_init,
	       TP_PROTO(struct sp804_clkevt *clkevt, void __iomem *timer_base,
			struct sp804_timer *timer),
	       TP_ARGS(clkevt, timer_base, timer));

void __init hck_lpcpu_timer_register(void);
#endif /* __HCK_LPCPU_TIMER__ */
