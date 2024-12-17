#undef TRACE_SYSTEM
#define TRACE_SYSTEM mips_mem

#if !defined(_TRACE_MIPS_MEM_H) || defined(TRACE_HEADER_MULTI_READ)
#define _TRACE_MIPS_MEM_H

TRACE_EVENT(check_l3_ddr_freq_change,
	TP_PROTO(int cpu, struct pcounts *pcount, int l3_ratio, int ddr_ratio, int l3_freq, int ddr_freq),

	TP_ARGS(cpu, pcount, l3_ratio, ddr_ratio, l3_freq, ddr_freq),

	TP_STRUCT__entry(
		__field(	int,	cpu			)
		__field(	u64,	inst     		)
		__field(	u64,    cycle   		)
		__field(	u64,	l2refill        	)
		__field(	u64,	l3refill        	)
		__field(	int,	l3_ratio	        )
		__field(	int,	ddr_ratio		)
		__field(	int,	l3_freq 	        )
		__field(	int,	ddr_freq 	        )
	),

	TP_fast_assign(
		__entry->cpu     = cpu;
		__entry->inst    = pcount->inst;
		__entry->cycle   = pcount->cycle;
		__entry->l2refill   = pcount->l2refill;
		__entry->l3refill   = pcount->l3refill;
		__entry->l3_ratio = l3_ratio;
		__entry->ddr_ratio = ddr_ratio;
		__entry->l3_freq = l3_freq;
		__entry->ddr_freq = ddr_freq;
	),
	TP_printk("cpu=%d inst=%llu, cycle=%llu l2refill=%llu l3refill=%llu"
		  "l3_ratio=%d ddr_ratio=%d l3_freq=%d ddr_freq=%d",
		  __entry->cpu, __entry->inst, __entry->cycle, __entry->l2refill, __entry->l3refill, __entry->l3_ratio,
		  __entry->ddr_ratio, __entry->l3_freq, __entry->ddr_freq)
);
#endif /* _TRACE_CPUFREQ_MIPS_H */

/* This part must be outside protection */
#include <trace/define_trace.h>