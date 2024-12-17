// SPDX-License-Identifier: GPL-2.0-only
/*
 * Branch Record Buffer Extension Driver.
 *
 * Copyright (C) 2021 ARM Limited
 *
 * Author: Anshuman Khandual <anshuman.khandual@arm.com>
 */
#include "brbe.h"

#define BRBFCR_BRANCH_ALL	(BRBFCR_EL1_DIRECT | BRBFCR_EL1_INDIRECT | \
				 BRBFCR_EL1_RTN | BRBFCR_EL1_INDCALL | \
				 BRBFCR_EL1_DIRCALL | BRBFCR_EL1_CONDDIR)

#define BRBE_FCR_MASK (BRBFCR_BRANCH_ALL)
#define BRBE_CR_MASK  (BRBCR_EL1_EXCEPTION | BRBCR_EL1_ERTN | BRBCR_EL1_CC | \
		       BRBCR_EL1_MPRED | BRBCR_EL1_E1BRE | BRBCR_EL1_E0BRE)

bool armv8pmu_branch_valid(struct perf_event *event)
{
	/*
	 * If the event does not have at least one of the privilege
	 * branch filters as in PERF_SAMPLE_BRANCH_PLM_ALL, the core
	 * perf will adjust its value based on perf event's existing
	 * privilege level via attr.exclude_[user|kernel|hv].
	 *
	 * As event->attr.branch_sample_type might have been changed
	 * when the event reaches here, it is not possible to figure
	 * out whether the event originally had HV privilege request
	 * or got added via the core perf. Just report this situation
	 * once and continue ignoring if there are other instances.
	 */
	if (event->attr.branch_sample_type & PERF_SAMPLE_BRANCH_HV)
		pr_warn_once("branch filter not supported - hypervisor privilege\n");

	if (event->attr.branch_sample_type & PERF_SAMPLE_BRANCH_ABORT_TX) {
		pr_warn_once("branch filter not supported - aborted transaction\n");
		return false;
	}

	if (event->attr.branch_sample_type & PERF_SAMPLE_BRANCH_NO_TX) {
		pr_warn_once("branch filter not supported - no transaction\n");
		return false;
	}

	if (event->attr.branch_sample_type & PERF_SAMPLE_BRANCH_IN_TX) {
		pr_warn_once("branch filter not supported - in transaction\n");
		return false;
	}
	return true;
}

static void branch_records_alloc(struct arm_pmu *armpmu)
{
	struct pmu_hw_events *events;
	int cpu;

	for_each_possible_cpu(cpu) {
		events = per_cpu_ptr(armpmu->hw_events, cpu);

		events->branches = kmalloc(sizeof(struct branch_records), GFP_KERNEL);
		WARN_ON(!events->branches);
	}
}

void armv8pmu_branch_probe(struct arm_pmu *armpmu)
{
	struct brbe_hw_attr *brbe_attr;
	u64 aa64dfr0, brbidr;
	unsigned int brbe;

	brbe_attr = kmalloc(sizeof(struct brbe_hw_attr), GFP_KERNEL);
	armpmu->hw_attr = brbe_attr;
	WARN_ON(!brbe_attr);

	aa64dfr0 = read_sysreg_s(SYS_ID_AA64DFR0_EL1);
	brbe = cpuid_feature_extract_unsigned_field(aa64dfr0, ID_AA64DFR0_EL1_BRBE_SHIFT);
	if (!brbe)
		return;

	if (brbe == ID_AA64DFR0_EL1_BRBE_IMP)
		brbe_attr->brbe_v1p1 = false;

	if (brbe == ID_AA64DFR0_EL1_BRBE_BRBE_V1P1)
		brbe_attr->brbe_v1p1 = true;

	brbidr = read_sysreg_s(SYS_BRBIDR0_EL1);
	brbe_attr->brbe_format = brbe_fetch_format(brbidr);
	if (brbe_attr->brbe_format != BRBIDR0_EL1_FORMAT_0)
		return;

	brbe_attr->brbe_cc = brbe_fetch_cc_bits(brbidr);
	if (brbe_attr->brbe_cc != BRBIDR0_EL1_CC_20_BIT)
		return;

	brbe_attr->brbe_nr = brbe_fetch_numrec(brbidr);
	if (!valid_brbe_nr(brbe_attr->brbe_nr))
		return;

	branch_records_alloc(armpmu);
	armpmu->features |= ARM_PMU_BRANCH_STACK;
}

static u64 branch_type_to_brbfcr(int branch_type)
{
	u64 brbfcr = 0;

	if (branch_type & PERF_SAMPLE_BRANCH_ANY) {
		brbfcr |= BRBFCR_BRANCH_ALL;
		return brbfcr;
	}

	if (branch_type & PERF_SAMPLE_BRANCH_ANY_CALL)
		brbfcr |= (BRBFCR_EL1_INDCALL | BRBFCR_EL1_DIRCALL);

	if (branch_type & PERF_SAMPLE_BRANCH_ANY_RETURN)
		brbfcr |= BRBFCR_EL1_RTN;

	if (branch_type & PERF_SAMPLE_BRANCH_IND_CALL)
		brbfcr |= BRBFCR_EL1_INDCALL;

	if (branch_type & PERF_SAMPLE_BRANCH_COND)
		brbfcr |= BRBFCR_EL1_CONDDIR;

	if (branch_type & PERF_SAMPLE_BRANCH_IND_JUMP)
		brbfcr |= BRBFCR_EL1_INDIRECT;

	if (branch_type & PERF_SAMPLE_BRANCH_CALL)
		brbfcr |= BRBFCR_EL1_DIRCALL;

	return brbfcr;
}

static u64 branch_type_to_brbcr(int branch_type)
{
	u64 brbcr = (BRBCR_EL1_CC | BRBCR_EL1_MPRED);

	if (branch_type & PERF_SAMPLE_BRANCH_USER)
		brbcr |= BRBCR_EL1_E0BRE;

	if (branch_type & PERF_SAMPLE_BRANCH_KERNEL)
		brbcr |= BRBCR_EL1_E1BRE;

	if (branch_type & PERF_SAMPLE_BRANCH_NO_CYCLES)
		brbcr &= ~BRBCR_EL1_CC;

	if (branch_type & PERF_SAMPLE_BRANCH_NO_FLAGS)
		brbcr &= ~BRBCR_EL1_MPRED;

	/*
	 * The exception and exception return branches could be
	 * captured, irrespective of the perf event's privilege.
	 * If the perf event does not have enough privilege for
	 * a given exception level, then addresses which falls
	 * under that exception level will be reported as zero
	 * for the captured branch record, creating source only
	 * or target only records.
	 */
	if (branch_type & PERF_SAMPLE_BRANCH_ANY) {
		brbcr |= BRBCR_EL1_EXCEPTION;
		brbcr |= BRBCR_EL1_ERTN;
	}

	if (branch_type & PERF_SAMPLE_BRANCH_ANY_CALL)
		brbcr |= BRBCR_EL1_EXCEPTION;

	if (branch_type & PERF_SAMPLE_BRANCH_ANY_RETURN)
		brbcr |= BRBCR_EL1_ERTN;

	return brbcr;
}

void armv8pmu_branch_enable(struct perf_event *event)
{
	u64 branch_type = event->attr.branch_sample_type;
	u64 brbfcr, brbcr;

	brbfcr = read_sysreg_s(SYS_BRBFCR_EL1);
	brbfcr &= ~BRBFCR_EL1_BANK_MASK;
	brbfcr &= ~(BRBFCR_EL1_EnI | BRBFCR_EL1_PAUSED | BRBE_FCR_MASK);
	brbfcr |= (branch_type_to_brbfcr(branch_type) & BRBE_FCR_MASK);
	write_sysreg_s(brbfcr, SYS_BRBFCR_EL1);
	isb();

	brbcr = read_sysreg_s(SYS_BRBCR_EL1);
	brbcr &= ~BRBE_CR_MASK;
	brbcr |= BRBCR_EL1_FZP;
	brbcr |= (BRBCR_EL1_TS_PHYSICAL << BRBCR_EL1_TS_SHIFT);
	brbcr |= (branch_type_to_brbcr(branch_type) & BRBE_CR_MASK);
	write_sysreg_s(brbcr, SYS_BRBCR_EL1);
	isb();
	armv8pmu_branch_reset();
}

void armv8pmu_branch_disable(struct perf_event *event)
{
	u64 brbcr = read_sysreg_s(SYS_BRBCR_EL1);

	brbcr &= ~(BRBCR_EL1_E0BRE | BRBCR_EL1_E1BRE);
	write_sysreg_s(brbcr, SYS_BRBCR_EL1);
	isb();
}

static int brbe_fetch_perf_type(u64 brbinf, bool *new_branch_type)
{
	int brbe_type = brbe_fetch_type(brbinf);
	*new_branch_type = false;

	switch (brbe_type) {
	case BRBINF_EL1_TYPE_UNCOND_DIR:
		return PERF_BR_UNCOND;
	case BRBINF_EL1_TYPE_INDIR:
		return PERF_BR_IND;
	case BRBINF_EL1_TYPE_DIR_LINK:
		return PERF_BR_CALL;
	case BRBINF_EL1_TYPE_INDIR_LINK:
		return PERF_BR_IND_CALL;
	case BRBINF_EL1_TYPE_RET_SUB:
		return PERF_BR_RET;
	case BRBINF_EL1_TYPE_COND_DIR:
		return PERF_BR_COND;
	case BRBINF_EL1_TYPE_CALL:
		return PERF_BR_CALL;
	case BRBINF_EL1_TYPE_TRAP:
		return PERF_BR_SYSCALL;
	case BRBINF_EL1_TYPE_RET_EXCPT:
		return PERF_BR_ERET;
	case BRBINF_EL1_TYPE_IRQ:
		return PERF_BR_IRQ;
	case BRBINF_EL1_TYPE_DEBUG_HALT:
		*new_branch_type = true;
		return PERF_BR_ARM64_DEBUG_HALT;
	case BRBINF_EL1_TYPE_SERROR:
		return PERF_BR_SERROR;
	case BRBINF_EL1_TYPE_INST_DEBUG:
		*new_branch_type = true;
		return PERF_BR_ARM64_DEBUG_INST;
	case BRBINF_EL1_TYPE_DATA_DEBUG:
		*new_branch_type = true;
		return PERF_BR_ARM64_DEBUG_DATA;
	case BRBINF_EL1_TYPE_ALGN_FAULT:
		*new_branch_type = true;
		return PERF_BR_NEW_FAULT_ALGN;
	case BRBINF_EL1_TYPE_INST_FAULT:
		*new_branch_type = true;
		return PERF_BR_NEW_FAULT_INST;
	case BRBINF_EL1_TYPE_DATA_FAULT:
		*new_branch_type = true;
		return PERF_BR_NEW_FAULT_DATA;
	case BRBINF_EL1_TYPE_FIQ:
		*new_branch_type = true;
		return PERF_BR_ARM64_FIQ;
	case BRBINF_EL1_TYPE_DEBUG_EXIT:
		*new_branch_type = true;
		return PERF_BR_ARM64_DEBUG_EXIT;
	default:
		pr_warn("unknown branch type captured\n");
		return PERF_BR_UNKNOWN;
	}
}

static int brbe_fetch_perf_priv(u64 brbinf)
{
	int brbe_el = brbe_fetch_el(brbinf);

	switch (brbe_el) {
	case BRBINF_EL1_EL_EL0:
		return PERF_BR_PRIV_USER;
	case BRBINF_EL1_EL_EL1:
		return PERF_BR_PRIV_KERNEL;
	case BRBINF_EL1_EL_EL2:
		if (is_kernel_in_hyp_mode())
			return PERF_BR_PRIV_KERNEL;
		return PERF_BR_PRIV_HV;
	default:
		pr_warn("unknown branch privilege captured\n");
		return PERF_BR_PRIV_UNKNOWN;
	}
}

static void capture_brbe_flags(struct pmu_hw_events *cpuc, struct perf_event *event,
			       u64 brbinf, int idx)
{
	int branch_type, type = brbe_record_valid(brbinf);
	bool new_branch_type;

	if (!branch_sample_no_cycles(event))
		cpuc->branches->branch_entries[idx].cycles = brbe_fetch_cycles(brbinf);

	branch_type = brbe_fetch_perf_type(brbinf, &new_branch_type);
	if (new_branch_type) {
		cpuc->branches->branch_entries[idx].type = PERF_BR_EXTEND_ABI;
		cpuc->branches->branch_entries[idx].new_type = branch_type;
	} else {
		cpuc->branches->branch_entries[idx].type = branch_type;
	}

	if (!branch_sample_no_flags(event)) {
		/*
		 * BRBINF_LASTFAILED does not indicate that the last transaction
		 * got failed or aborted during the current branch record itself.
		 * Rather, this indicates that all the branch records which were
		 * in transaction until the curret branch record have failed. So
		 * the entire BRBE buffer needs to be processed later on to find
		 * all branch records which might have failed.
		 */
		cpuc->branches->branch_entries[idx].abort = brbinf & BRBINF_EL1_LASTFAILED;

		/*
		 * All these information (i.e transaction state and mispredicts)
		 * are not available for target only branch records.
		 */
		if (type != BRBINF_EL1_VALID_TARGET) {
			cpuc->branches->branch_entries[idx].mispred = brbinf & BRBINF_EL1_MPRED;
			cpuc->branches->branch_entries[idx].predicted = !(brbinf & BRBINF_EL1_MPRED);
			cpuc->branches->branch_entries[idx].in_tx = brbinf & BRBINF_EL1_T;
		}
	}

	/*
	* All these information (i.e branch privilege level) are not
	* available for source only branch records.
	*/
	if (type != BRBINF_EL1_VALID_SOURCE)
		cpuc->branches->branch_entries[idx].priv = brbe_fetch_perf_priv(brbinf);
}

/*
 * A branch record with BRBINF_EL1.LASTFAILED set, implies that all
 * preceding consecutive branch records, that were in a transaction
 * (i.e their BRBINF_EL1.TX set) have been aborted.
 *
 * Similarly BRBFCR_EL1.LASTFAILED set, indicate that all preceding
 * consecutive branch records upto the last record, which were in a
 * transaction (i.e their BRBINF_EL1.TX set) have been aborted.
 *
 * --------------------------------- -------------------
 * | 00 | BRBSRC | BRBTGT | BRBINF | | TX = 1 | LF = 0 | [TX success]
 * --------------------------------- -------------------
 * | 01 | BRBSRC | BRBTGT | BRBINF | | TX = 1 | LF = 0 | [TX success]
 * --------------------------------- -------------------
 * | 02 | BRBSRC | BRBTGT | BRBINF | | TX = 0 | LF = 0 |
 * --------------------------------- -------------------
 * | 03 | BRBSRC | BRBTGT | BRBINF | | TX = 1 | LF = 0 | [TX failed]
 * --------------------------------- -------------------
 * | 04 | BRBSRC | BRBTGT | BRBINF | | TX = 1 | LF = 0 | [TX failed]
 * --------------------------------- -------------------
 * | 05 | BRBSRC | BRBTGT | BRBINF | | TX = 0 | LF = 1 |
 * --------------------------------- -------------------
 * | .. | BRBSRC | BRBTGT | BRBINF | | TX = 0 | LF = 0 |
 * --------------------------------- -------------------
 * | 61 | BRBSRC | BRBTGT | BRBINF | | TX = 1 | LF = 0 | [TX failed]
 * --------------------------------- -------------------
 * | 62 | BRBSRC | BRBTGT | BRBINF | | TX = 1 | LF = 0 | [TX failed]
 * --------------------------------- -------------------
 * | 63 | BRBSRC | BRBTGT | BRBINF | | TX = 1 | LF = 0 | [TX failed]
 * --------------------------------- -------------------
 *
 * BRBFCR_EL1.LASTFAILED == 1
 *
 * Here BRBFCR_EL1.LASTFAILED failes all those consecutive and also
 * in transaction branches near the end of the BRBE buffer.
 */
static void process_branch_aborts(struct pmu_hw_events *cpuc)
{
	struct brbe_hw_attr *brbe_attr = (struct brbe_hw_attr *) cpuc->percpu_pmu->hw_attr;
	u64 brbfcr = read_sysreg_s(SYS_BRBFCR_EL1);
	bool lastfailed = !!(brbfcr & BRBFCR_EL1_LASTFAILED);
	int idx = brbe_attr->brbe_nr - 1;

	do {
		if (cpuc->branches->branch_entries[idx].in_tx) {
			cpuc->branches->branch_entries[idx].abort = lastfailed;
		} else {
			lastfailed = cpuc->branches->branch_entries[idx].abort;
			cpuc->branches->branch_entries[idx].abort = false;
		}
	} while (idx--, idx >= 0);
}

void armv8pmu_branch_reset(void)
{
	asm volatile(BRB_IALL);
	isb();
}

void armv8pmu_branch_read(struct pmu_hw_events *cpuc, struct perf_event *event)
{
	struct brbe_hw_attr *brbe_attr = (struct brbe_hw_attr *) cpuc->percpu_pmu->hw_attr;
	u64 brbinf, brbfcr, brbcr, saved_priv;
	int idx;

	brbcr = read_sysreg_s(SYS_BRBCR_EL1);
	brbfcr = read_sysreg_s(SYS_BRBFCR_EL1);

	/* Ensure pause on PMU interrupt is enabled */
	WARN_ON_ONCE(~brbcr & BRBCR_EL1_FZP);

	/* Save and clear the privilege */
	saved_priv =  brbcr & (BRBCR_EL1_E0BRE | BRBCR_EL1_E1BRE);
	brbcr &= ~(BRBCR_EL1_E0BRE | BRBCR_EL1_E1BRE);

	/* Pause the buffer */
	brbfcr |= BRBFCR_EL1_PAUSED;

	write_sysreg_s(brbcr, SYS_BRBCR_EL1);
	write_sysreg_s(brbfcr, SYS_BRBFCR_EL1);
	isb();

	for (idx = 0; idx < brbe_attr->brbe_nr; idx++) {
		select_brbe_bank_index(idx);
		brbinf = get_brbinf_reg(idx);
		/*
		 * There are no valid entries anymore on the buffer.
		 * Abort the branch record processing to save some
		 * cycles and also reduce the capture/process load
		 * for the user space as well.
		 */
		if (brbe_invalid(brbinf))
			break;

		perf_clear_branch_entry_bitfields(&cpuc->branches->branch_entries[idx]);
		if (brbe_valid(brbinf)) {
			cpuc->branches->branch_entries[idx].from =  get_brbsrc_reg(idx);
			cpuc->branches->branch_entries[idx].to =  get_brbtgt_reg(idx);
		} else if (brbe_source(brbinf)) {
			cpuc->branches->branch_entries[idx].from =  get_brbsrc_reg(idx);
			cpuc->branches->branch_entries[idx].to = 0;
		} else if (brbe_target(brbinf)) {
			cpuc->branches->branch_entries[idx].from = 0;
			cpuc->branches->branch_entries[idx].to =  get_brbtgt_reg(idx);
		}
		capture_brbe_flags(cpuc, event, brbinf, idx);
	}
	cpuc->branches->branch_stack.nr = idx;
	cpuc->branches->branch_stack.hw_idx = -1ULL;
	process_branch_aborts(cpuc);

	/* Restore privilege, enable pause on PMU interrupt */
	brbcr |= saved_priv;
	brbcr |= BRBCR_EL1_FZP;

	/* Unpause the buffer */
	brbfcr &= ~BRBFCR_EL1_PAUSED;

	write_sysreg_s(brbcr, SYS_BRBCR_EL1);
	write_sysreg_s(brbfcr, SYS_BRBFCR_EL1);
	isb();
	armv8pmu_branch_reset();
}
