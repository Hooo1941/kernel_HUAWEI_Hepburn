#ifndef __HCK_CLK__
#define __HCK_CLK__

#include <linux/types.h>
#include <platform_include/basicplatform/linux/hck_vendor_hooks.h>

/*
 * Following tracepoints are not exported in tracefs and provide a
 * mechanism for vendor modules to hook and extend functionality
 */

struct clk;
struct clk_gate;

DECLARE_HCK_VH(clk_enable,
	TP_PROTO(struct clk *clk),
	TP_ARGS(clk));

DECLARE_HCK_VH(clk_set_freq,
	TP_PROTO(struct clk *clk, unsigned long rate),
	TP_ARGS(clk, rate));

DECLARE_HCK_VH(clk_scgt_always_on,
	TP_PROTO(struct clk_gate *gate, int set, bool *ret),
	TP_ARGS(gate, set, ret));

#endif /* __HCK_CLK__ */
