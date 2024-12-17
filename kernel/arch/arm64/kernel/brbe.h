/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Branch Record Buffer Extension Helpers.
 *
 * Copyright (C) 2021 ARM Limited
 *
 * Author: Anshuman Khandual <anshuman.khandual@arm.com>
 */
#ifndef _ARM64_BRBE_H
#define _ARM64_BRBE_H

#define pr_fmt(fmt) "brbe: " fmt

#include <linux/perf/arm_pmu.h>
#include <uapi/linux/perf_event.h>

#define ID_AA64DFR0_EL1_BRBE					GENMASK(55, 52)
#define ID_AA64DFR0_EL1_BRBE_MASK				GENMASK(55, 52)
#define ID_AA64DFR0_EL1_BRBE_SHIFT				52
#define ID_AA64DFR0_EL1_BRBE_WIDTH				4
#define ID_AA64DFR0_EL1_BRBE_NI					UL(0b0000)
#define ID_AA64DFR0_EL1_BRBE_IMP				UL(0b0001)
#define ID_AA64DFR0_EL1_BRBE_BRBE_V1P1			UL(0b0010)

#define REG_BRBINF_EL1							S2_1_C8_C0_0
#define SYS_BRBINF_EL1							sys_reg(2, 1, 8, 0, 0)
#define SYS_BRBINF_EL1_Op0						2
#define SYS_BRBINF_EL1_Op1						1
#define SYS_BRBINF_EL1_CRn						8
#define SYS_BRBINF_EL1_CRm						0
#define SYS_BRBINF_EL1_Op2						0

#define BRBINF_EL1_CCU							GENMASK(46, 46)
#define BRBINF_EL1_CCU_MASK						GENMASK(46, 46)
#define BRBINF_EL1_CCU_SHIFT					46
#define BRBINF_EL1_CCU_WIDTH					1

#define BRBINF_EL1_CC							GENMASK(45, 32)
#define BRBINF_EL1_CC_MASK						GENMASK(45, 32)
#define BRBINF_EL1_CC_SHIFT						32
#define BRBINF_EL1_CC_WIDTH						14

#define BRBINF_EL1_LASTFAILED					GENMASK(17, 17)
#define BRBINF_EL1_LASTFAILED_MASK				GENMASK(17, 17)
#define BRBINF_EL1_LASTFAILED_SHIFT				17
#define BRBINF_EL1_LASTFAILED_WIDTH				1

#define BRBINF_EL1_T							GENMASK(16, 16)
#define BRBINF_EL1_T_MASK						GENMASK(16, 16)
#define BRBINF_EL1_T_SHIFT						16
#define BRBINF_EL1_T_WIDTH						1

#define BRBINF_EL1_TYPE							GENMASK(13, 8)
#define BRBINF_EL1_TYPE_MASK					GENMASK(13, 8)
#define BRBINF_EL1_TYPE_SHIFT					8
#define BRBINF_EL1_TYPE_WIDTH					6
#define BRBINF_EL1_TYPE_UNCOND_DIR				UL(0b000000)
#define BRBINF_EL1_TYPE_INDIR					UL(0b000001)
#define BRBINF_EL1_TYPE_DIR_LINK				UL(0b000010)
#define BRBINF_EL1_TYPE_INDIR_LINK				UL(0b000011)
#define BRBINF_EL1_TYPE_RET_SUB					UL(0b000101)
#define BRBINF_EL1_TYPE_RET_EXCPT				UL(0b000111)
#define BRBINF_EL1_TYPE_COND_DIR				UL(0b001000)
#define BRBINF_EL1_TYPE_DEBUG_HALT				UL(0b100001)
#define BRBINF_EL1_TYPE_CALL					UL(0b100010)
#define BRBINF_EL1_TYPE_TRAP					UL(0b100011)
#define BRBINF_EL1_TYPE_SERROR					UL(0b100100)
#define BRBINF_EL1_TYPE_INST_DEBUG				UL(0b100110)
#define BRBINF_EL1_TYPE_DATA_DEBUG				UL(0b100111)
#define BRBINF_EL1_TYPE_ALGN_FAULT				UL(0b101010)
#define BRBINF_EL1_TYPE_INST_FAULT				UL(0b101011)
#define BRBINF_EL1_TYPE_DATA_FAULT				UL(0b101100)
#define BRBINF_EL1_TYPE_IRQ						UL(0b101110)
#define BRBINF_EL1_TYPE_FIQ						UL(0b101111)
#define BRBINF_EL1_TYPE_DEBUG_EXIT				UL(0b111001)

#define BRBINF_EL1_EL							GENMASK(7, 6)
#define BRBINF_EL1_EL_MASK						GENMASK(7, 6)
#define BRBINF_EL1_EL_SHIFT						6
#define BRBINF_EL1_EL_WIDTH						2
#define BRBINF_EL1_EL_EL0						UL(0b00)
#define BRBINF_EL1_EL_EL1						UL(0b01)
#define BRBINF_EL1_EL_EL2						UL(0b10)
#define BRBINF_EL1_EL_EL3						UL(0b11)

#define BRBINF_EL1_MPRED						GENMASK(5, 5)
#define BRBINF_EL1_MPRED_MASK					GENMASK(5, 5)
#define BRBINF_EL1_MPRED_SHIFT					5
#define BRBINF_EL1_MPRED_WIDTH					1

#define BRBINF_EL1_VALID						GENMASK(1, 0)
#define BRBINF_EL1_VALID_MASK					GENMASK(1, 0)
#define BRBINF_EL1_VALID_SHIFT					0
#define BRBINF_EL1_VALID_WIDTH					2
#define BRBINF_EL1_VALID_NONE					UL(0b00)
#define BRBINF_EL1_VALID_TARGET					UL(0b01)
#define BRBINF_EL1_VALID_SOURCE					UL(0b10)
#define BRBINF_EL1_VALID_FULL					UL(0b11)

#define BRBINF_EL1_RES0	\
	(UL(0) | GENMASK_ULL(63, 47) | GENMASK_ULL(31, 18) | GENMASK_ULL(15, 14) | GENMASK_ULL(4, 2))
#define BRBINF_EL1_RES1							(UL(0))

#define REG_BRBCR_EL1							S2_1_C9_C0_0
#define SYS_BRBCR_EL1							sys_reg(2, 1, 9, 0, 0)
#define SYS_BRBCR_EL1_Op0						2
#define SYS_BRBCR_EL1_Op1						1
#define SYS_BRBCR_EL1_CRn						9
#define SYS_BRBCR_EL1_CRm						0
#define SYS_BRBCR_EL1_Op2						0

#define BRBCR_EL1_EXCEPTION						GENMASK(23, 23)
#define BRBCR_EL1_EXCEPTION_MASK				GENMASK(23, 23)
#define BRBCR_EL1_EXCEPTION_SHIFT				23
#define BRBCR_EL1_EXCEPTION_WIDTH				1

#define BRBCR_EL1_ERTN							GENMASK(22, 22)
#define BRBCR_EL1_ERTN_MASK						GENMASK(22, 22)
#define BRBCR_EL1_ERTN_SHIFT					22
#define BRBCR_EL1_ERTN_WIDTH					1

#define BRBCR_EL1_FZP							GENMASK(8, 8)
#define BRBCR_EL1_FZP_MASK						GENMASK(8, 8)
#define BRBCR_EL1_FZP_SHIFT						8
#define BRBCR_EL1_FZP_WIDTH						1

#define BRBCR_EL1_TS							GENMASK(6, 5)
#define BRBCR_EL1_TS_MASK						GENMASK(6, 5)
#define BRBCR_EL1_TS_SHIFT						5
#define BRBCR_EL1_TS_WIDTH						2
#define BRBCR_EL1_TS_VIRTUAL					UL(0b01)
#define BRBCR_EL1_TS_GST_PHYSICAL				UL(0b10)
#define BRBCR_EL1_TS_PHYSICAL					UL(0b11)

#define BRBCR_EL1_MPRED							GENMASK(4, 4)
#define BRBCR_EL1_MPRED_MASK					GENMASK(4, 4)
#define BRBCR_EL1_MPRED_SHIFT					4
#define BRBCR_EL1_MPRED_WIDTH					1

#define BRBCR_EL1_CC							GENMASK(3, 3)
#define BRBCR_EL1_CC_MASK						GENMASK(3, 3)
#define BRBCR_EL1_CC_SHIFT						3
#define BRBCR_EL1_CC_WIDTH						1

#define BRBCR_EL1_E1BRE							GENMASK(1, 1)
#define BRBCR_EL1_E1BRE_MASK					GENMASK(1, 1)
#define BRBCR_EL1_E1BRE_SHIFT					1
#define BRBCR_EL1_E1BRE_WIDTH					1

#define BRBCR_EL1_E0BRE							GENMASK(0, 0)
#define BRBCR_EL1_E0BRE_MASK					GENMASK(0, 0)
#define BRBCR_EL1_E0BRE_SHIFT					0
#define BRBCR_EL1_E0BRE_WIDTH					1

#define BRBCR_EL1_RES0	\
	(UL(0) | GENMASK_ULL(63, 24) | GENMASK_ULL(21, 9) | GENMASK_ULL(7, 7) | GENMASK_ULL(2, 2))
#define BRBCR_EL1_RES1							(UL(0))

#define REG_BRBFCR_EL1							S2_1_C9_C0_1
#define SYS_BRBFCR_EL1							sys_reg(2, 1, 9, 0, 1)
#define SYS_BRBFCR_EL1_Op0						2
#define SYS_BRBFCR_EL1_Op1						1
#define SYS_BRBFCR_EL1_CRn						9
#define SYS_BRBFCR_EL1_CRm						0
#define SYS_BRBFCR_EL1_Op2						1

#define BRBFCR_EL1_BANK							GENMASK(29, 28)
#define BRBFCR_EL1_BANK_MASK					GENMASK(29, 28)
#define BRBFCR_EL1_BANK_SHIFT					28
#define BRBFCR_EL1_BANK_WIDTH					2
#define BRBFCR_EL1_BANK_FIRST					UL(0b0)
#define BRBFCR_EL1_BANK_SECOND					UL(0b1)

#define BRBFCR_EL1_CONDDIR						GENMASK(22, 22)
#define BRBFCR_EL1_CONDDIR_MASK					GENMASK(22, 22)
#define BRBFCR_EL1_CONDDIR_SHIFT				22
#define BRBFCR_EL1_CONDDIR_WIDTH				1

#define BRBFCR_EL1_DIRCALL						GENMASK(21, 21)
#define BRBFCR_EL1_DIRCALL_MASK					GENMASK(21, 21)
#define BRBFCR_EL1_DIRCALL_SHIFT				21
#define BRBFCR_EL1_DIRCALL_WIDTH				1

#define BRBFCR_EL1_INDCALL						GENMASK(20, 20)
#define BRBFCR_EL1_INDCALL_MASK					GENMASK(20, 20)
#define BRBFCR_EL1_INDCALL_SHIFT				20
#define BRBFCR_EL1_INDCALL_WIDTH				1

#define BRBFCR_EL1_RTN							GENMASK(19, 19)
#define BRBFCR_EL1_RTN_MASK						GENMASK(19, 19)
#define BRBFCR_EL1_RTN_SHIFT					19
#define BRBFCR_EL1_RTN_WIDTH					1

#define BRBFCR_EL1_INDIRECT						GENMASK(18, 18)
#define BRBFCR_EL1_INDIRECT_MASK				GENMASK(18, 18)
#define BRBFCR_EL1_INDIRECT_SHIFT				18
#define BRBFCR_EL1_INDIRECT_WIDTH				1

#define BRBFCR_EL1_DIRECT						GENMASK(17, 17)
#define BRBFCR_EL1_DIRECT_MASK					GENMASK(17, 17)
#define BRBFCR_EL1_DIRECT_SHIFT					17
#define BRBFCR_EL1_DIRECT_WIDTH					1

#define BRBFCR_EL1_EnI							GENMASK(16, 16)
#define BRBFCR_EL1_EnI_MASK						GENMASK(16, 16)
#define BRBFCR_EL1_EnI_SHIFT					16
#define BRBFCR_EL1_EnI_WIDTH					1

#define BRBFCR_EL1_PAUSED						GENMASK(7, 7)
#define BRBFCR_EL1_PAUSED_MASK					GENMASK(7, 7)
#define BRBFCR_EL1_PAUSED_SHIFT					7
#define BRBFCR_EL1_PAUSED_WIDTH					1

#define BRBFCR_EL1_LASTFAILED					GENMASK(6, 6)
#define BRBFCR_EL1_LASTFAILED_MASK				GENMASK(6, 6)
#define BRBFCR_EL1_LASTFAILED_SHIFT				6
#define BRBFCR_EL1_LASTFAILED_WIDTH				1

#define BRBFCR_EL1_RES0	\
	(UL(0) | GENMASK_ULL(63, 30) | GENMASK_ULL(27, 23) | GENMASK_ULL(15, 8) | GENMASK_ULL(5, 0))
#define BRBFCR_EL1_RES1							(UL(0))

#define REG_BRBTS_EL1							S2_1_C9_C0_2
#define SYS_BRBTS_EL1							sys_reg(2, 1, 9, 0, 2)
#define SYS_BRBTS_EL1_Op0						2
#define SYS_BRBTS_EL1_Op1						1
#define SYS_BRBTS_EL1_CRn						9
#define SYS_BRBTS_EL1_CRm						0
#define SYS_BRBTS_EL1_Op2						2

#define BRBTS_EL1_TS							GENMASK(63, 0)
#define BRBTS_EL1_TS_MASK						GENMASK(63, 0)
#define BRBTS_EL1_TS_SHIFT						0
#define BRBTS_EL1_TS_WIDTH						64

#define BRBTS_EL1_RES0							(UL(0))
#define BRBTS_EL1_RES1							(UL(0))

#define REG_BRBINFINJ_EL1						S2_1_C9_C1_0
#define SYS_BRBINFINJ_EL1						sys_reg(2, 1, 9, 1, 0)
#define SYS_BRBINFINJ_EL1_Op0					2
#define SYS_BRBINFINJ_EL1_Op1					1
#define SYS_BRBINFINJ_EL1_CRn					9
#define SYS_BRBINFINJ_EL1_CRm					1
#define SYS_BRBINFINJ_EL1_Op2					0

#define BRBINFINJ_EL1_CCU						GENMASK(46, 46)
#define BRBINFINJ_EL1_CCU_MASK					GENMASK(46, 46)
#define BRBINFINJ_EL1_CCU_SHIFT					46
#define BRBINFINJ_EL1_CCU_WIDTH					1

#define BRBINFINJ_EL1_CC						GENMASK(45, 32)
#define BRBINFINJ_EL1_CC_MASK					GENMASK(45, 32)
#define BRBINFINJ_EL1_CC_SHIFT					32
#define BRBINFINJ_EL1_CC_WIDTH					14

#define BRBINFINJ_EL1_LASTFAILED				GENMASK(17, 17)
#define BRBINFINJ_EL1_LASTFAILED_MASK			GENMASK(17, 17)
#define BRBINFINJ_EL1_LASTFAILED_SHIFT			17
#define BRBINFINJ_EL1_LASTFAILED_WIDTH			1

#define BRBINFINJ_EL1_T							GENMASK(16, 16)
#define BRBINFINJ_EL1_T_MASK					GENMASK(16, 16)
#define BRBINFINJ_EL1_T_SHIFT					16
#define BRBINFINJ_EL1_T_WIDTH					1

#define BRBINFINJ_EL1_TYPE						GENMASK(13, 8)
#define BRBINFINJ_EL1_TYPE_MASK					GENMASK(13, 8)
#define BRBINFINJ_EL1_TYPE_SHIFT				8
#define BRBINFINJ_EL1_TYPE_WIDTH				6
#define BRBINFINJ_EL1_TYPE_UNCOND_DIR			UL(0b000000)
#define BRBINFINJ_EL1_TYPE_INDIR				UL(0b000001)
#define BRBINFINJ_EL1_TYPE_DIR_LINK				UL(0b000010)
#define BRBINFINJ_EL1_TYPE_INDIR_LINK			UL(0b000011)
#define BRBINFINJ_EL1_TYPE_RET_SUB				UL(0b000100)
#define BRBINFINJ_EL1_TYPE_RET_SUB				UL(0b000100)
#define BRBINFINJ_EL1_TYPE_RET_EXCPT			UL(0b000111)
#define BRBINFINJ_EL1_TYPE_COND_DIR				UL(0b001000)
#define BRBINFINJ_EL1_TYPE_DEBUG_HALT			UL(0b100001)
#define BRBINFINJ_EL1_TYPE_CALL					UL(0b100010)
#define BRBINFINJ_EL1_TYPE_TRAP					UL(0b100011)
#define BRBINFINJ_EL1_TYPE_SERROR				UL(0b100100)
#define BRBINFINJ_EL1_TYPE_INST_DEBUG			UL(0b100110)
#define BRBINFINJ_EL1_TYPE_DATA_DEBUG			UL(0b100111)
#define BRBINFINJ_EL1_TYPE_ALGN_FAULT			UL(0b101010)
#define BRBINFINJ_EL1_TYPE_INST_FAULT			UL(0b101011)
#define BRBINFINJ_EL1_TYPE_DATA_FAULT			UL(0b101100)
#define BRBINFINJ_EL1_TYPE_IRQ					UL(0b101110)
#define BRBINFINJ_EL1_TYPE_FIQ					UL(0b101111)
#define BRBINFINJ_EL1_TYPE_DEBUG_EXIT			UL(0b111001)

#define BRBINFINJ_EL1_EL						GENMASK(7, 6)
#define BRBINFINJ_EL1_EL_MASK					GENMASK(7, 6)
#define BRBINFINJ_EL1_EL_SHIFT					6
#define BRBINFINJ_EL1_EL_WIDTH					2
#define BRBINFINJ_EL1_EL_EL0					UL(0b00)
#define BRBINFINJ_EL1_EL_EL1					UL(0b01)
#define BRBINFINJ_EL1_EL_EL2					UL(0b10)
#define BRBINFINJ_EL1_EL_EL3					UL(0b11)

#define BRBINFINJ_EL1_MPRED						GENMASK(5, 5)
#define BRBINFINJ_EL1_MPRED_MASK				GENMASK(5, 5)
#define BRBINFINJ_EL1_MPRED_SHIFT				5
#define BRBINFINJ_EL1_MPRED_WIDTH				1

#define BRBINFINJ_EL1_VALID						GENMASK(1, 0)
#define BRBINFINJ_EL1_VALID_MASK				GENMASK(1, 0)
#define BRBINFINJ_EL1_VALID_SHIFT				0
#define BRBINFINJ_EL1_VALID_WIDTH				2
#define BRBINFINJ_EL1_VALID_NONE				UL(0b00)
#define BRBINFINJ_EL1_VALID_TARGET				UL(0b01)
#define BRBINFINJ_EL1_VALID_SOURCE				UL(0b10)
#define BRBINFINJ_EL1_VALID_FULL				UL(0b00)

#define BRBINFINJ_EL1_RES0	\
	(UL(0) | GENMASK_ULL(63, 47) | GENMASK_ULL(31, 18) | GENMASK_ULL(15, 14) | GENMASK_ULL(4, 2))
#define BRBINFINJ_EL1_RES1						(UL(0))

#define REG_BRBSRCINJ_EL1						S2_1_C9_C1_1
#define SYS_BRBSRCINJ_EL1						sys_reg(2, 1, 9, 1, 1)
#define SYS_BRBSRCINJ_EL1_Op0					2
#define SYS_BRBSRCINJ_EL1_Op1					1
#define SYS_BRBSRCINJ_EL1_CRn					9
#define SYS_BRBSRCINJ_EL1_CRm					1
#define SYS_BRBSRCINJ_EL1_Op2					1

#define BRBSRCINJ_EL1_ADDRESS					GENMASK(63, 0)
#define BRBSRCINJ_EL1_ADDRESS_MASK				GENMASK(63, 0)
#define BRBSRCINJ_EL1_ADDRESS_SHIFT				0
#define BRBSRCINJ_EL1_ADDRESS_WIDTH				64

#define BRBSRCINJ_EL1_RES0						(UL(0))
#define BRBSRCINJ_EL1_RES1						(UL(0))

#define REG_BRBTGTINJ_EL1						S2_1_C9_C1_2
#define SYS_BRBTGTINJ_EL1						sys_reg(2, 1, 9, 1, 2)
#define SYS_BRBTGTINJ_EL1_Op0					2
#define SYS_BRBTGTINJ_EL1_Op1					1
#define SYS_BRBTGTINJ_EL1_CRn					9
#define SYS_BRBTGTINJ_EL1_CRm					1
#define SYS_BRBTGTINJ_EL1_Op2					2

#define BRBTGTINJ_EL1_ADDRESS					GENMASK(63, 0)
#define BRBTGTINJ_EL1_ADDRESS_MASK				GENMASK(63, 0)
#define BRBTGTINJ_EL1_ADDRESS_SHIFT				0
#define BRBTGTINJ_EL1_ADDRESS_WIDTH				64

#define BRBTGTINJ_EL1_RES0						(UL(0))
#define BRBTGTINJ_EL1_RES1						(UL(0))

#define REG_BRBIDR0_EL1							S2_1_C9_C2_0
#define SYS_BRBIDR0_EL1							sys_reg(2, 1, 9, 2, 0)
#define SYS_BRBIDR0_EL1_Op0						2
#define SYS_BRBIDR0_EL1_Op1						1
#define SYS_BRBIDR0_EL1_CRn						9
#define SYS_BRBIDR0_EL1_CRm						2
#define SYS_BRBIDR0_EL1_Op2						0

#define BRBIDR0_EL1_CC							GENMASK(15, 12)
#define BRBIDR0_EL1_CC_MASK						GENMASK(15, 12)
#define BRBIDR0_EL1_CC_SHIFT					12
#define BRBIDR0_EL1_CC_WIDTH					4
#define BRBIDR0_EL1_CC_20_BIT					UL(0b101)

#define BRBIDR0_EL1_FORMAT						GENMASK(11, 8)
#define BRBIDR0_EL1_FORMAT_MASK					GENMASK(11, 8)
#define BRBIDR0_EL1_FORMAT_SHIFT				8
#define BRBIDR0_EL1_FORMAT_WIDTH				4
#define BRBIDR0_EL1_FORMAT_0					UL(0b0)

#define BRBIDR0_EL1_NUMREC						GENMASK(7, 0)
#define BRBIDR0_EL1_NUMREC_MASK					GENMASK(7, 0)
#define BRBIDR0_EL1_NUMREC_SHIFT				0
#define BRBIDR0_EL1_NUMREC_WIDTH				8
#define BRBIDR0_EL1_NUMREC_8					UL(0b1000)
#define BRBIDR0_EL1_NUMREC_16					UL(0b10000)
#define BRBIDR0_EL1_NUMREC_32					UL(0b100000)
#define BRBIDR0_EL1_NUMREC_64					UL(0b1000000)

#define BRBIDR0_EL1_RES0						(UL(0) | GENMASK_ULL(63, 16))
#define BRBIDR0_EL1_RES1						(UL(0))

struct brbe_hw_attr {
	bool	brbe_v1p1;
	int	brbe_cc;
	int	brbe_nr;
	int	brbe_format;
};

/*
 * BRBE Instructions
 *
 * BRB_IALL : Invalidate the entire buffer
 * BRB_INJ  : Inject latest branch record derived from [BRBSRCINJ, BRBTGTINJ, BRBINFINJ]
 */
#define BRB_IALL __emit_inst(0xD5000000 | sys_insn(1, 1, 7, 2, 4) | (0x1f))
#define BRB_INJ  __emit_inst(0xD5000000 | sys_insn(1, 1, 7, 2, 5) | (0x1f))

/*
 * BRBE Buffer Organization
 *
 * BRBE buffer is arranged as multiple banks of 32 branch record
 * entries each. An indivdial branch record in a given bank could
 * be accessedi, after selecting the bank in BRBFCR_EL1.BANK and
 * accessing the registers i.e [BRBSRC, BRBTGT, BRBINF] set with
 * indices [0..31].
 *
 * Bank 0
 *
 *	---------------------------------	------
 *	| 00 | BRBSRC | BRBTGT | BRBINF |	| 00 |
 *	---------------------------------	------
 *	| 01 | BRBSRC | BRBTGT | BRBINF |	| 01 |
 *	---------------------------------	------
 *	| .. | BRBSRC | BRBTGT | BRBINF |	| .. |
 *	---------------------------------	------
 *	| 31 | BRBSRC | BRBTGT | BRBINF |	| 31 |
 *	---------------------------------	------
 *
 * Bank 1
 *
 *	---------------------------------	------
 *	| 32 | BRBSRC | BRBTGT | BRBINF |	| 00 |
 *	---------------------------------	------
 *	| 33 | BRBSRC | BRBTGT | BRBINF |	| 01 |
 *	---------------------------------	------
 *	| .. | BRBSRC | BRBTGT | BRBINF |	| .. |
 *	---------------------------------	------
 *	| 63 | BRBSRC | BRBTGT | BRBINF |	| 31 |
 *	---------------------------------	------
 */
#define BRBE_BANK0_IDX_MIN 0
#define BRBE_BANK0_IDX_MAX 31
#define BRBE_BANK1_IDX_MIN 32
#define BRBE_BANK1_IDX_MAX 63

#define RETURN_READ_BRBSRCN(n) \
	read_sysreg_s(SYS_BRBSRC##n##_EL1)

#define RETURN_READ_BRBTGTN(n) \
	read_sysreg_s(SYS_BRBTGT##n##_EL1)

#define RETURN_READ_BRBINFN(n) \
	read_sysreg_s(SYS_BRBINF##n##_EL1)

#define BRBE_REGN_CASE(n, case_macro) \
	case n: return case_macro(n); break

#define BRBE_REGN_SWITCH(x, case_macro)				\
	do {							\
		switch (x) {					\
		BRBE_REGN_CASE(0, case_macro);			\
		BRBE_REGN_CASE(1, case_macro);			\
		BRBE_REGN_CASE(2, case_macro);			\
		BRBE_REGN_CASE(3, case_macro);			\
		BRBE_REGN_CASE(4, case_macro);			\
		BRBE_REGN_CASE(5, case_macro);			\
		BRBE_REGN_CASE(6, case_macro);			\
		BRBE_REGN_CASE(7, case_macro);			\
		BRBE_REGN_CASE(8, case_macro);			\
		BRBE_REGN_CASE(9, case_macro);			\
		BRBE_REGN_CASE(10, case_macro);			\
		BRBE_REGN_CASE(11, case_macro);			\
		BRBE_REGN_CASE(12, case_macro);			\
		BRBE_REGN_CASE(13, case_macro);			\
		BRBE_REGN_CASE(14, case_macro);			\
		BRBE_REGN_CASE(15, case_macro);			\
		BRBE_REGN_CASE(16, case_macro);			\
		BRBE_REGN_CASE(17, case_macro);			\
		BRBE_REGN_CASE(18, case_macro);			\
		BRBE_REGN_CASE(19, case_macro);			\
		BRBE_REGN_CASE(20, case_macro);			\
		BRBE_REGN_CASE(21, case_macro);			\
		BRBE_REGN_CASE(22, case_macro);			\
		BRBE_REGN_CASE(23, case_macro);			\
		BRBE_REGN_CASE(24, case_macro);			\
		BRBE_REGN_CASE(25, case_macro);			\
		BRBE_REGN_CASE(26, case_macro);			\
		BRBE_REGN_CASE(27, case_macro);			\
		BRBE_REGN_CASE(28, case_macro);			\
		BRBE_REGN_CASE(29, case_macro);			\
		BRBE_REGN_CASE(30, case_macro);			\
		BRBE_REGN_CASE(31, case_macro);			\
		default:					\
			pr_warn("unknown register index\n");	\
			return -1;				\
		}						\
	} while (0)

static inline int buffer_to_brbe_idx(int buffer_idx)
{
	return buffer_idx % 32;
}

static inline u64 get_brbsrc_reg(int buffer_idx)
{
	int brbe_idx = buffer_to_brbe_idx(buffer_idx);

	BRBE_REGN_SWITCH(brbe_idx, RETURN_READ_BRBSRCN);
}

static inline u64 get_brbtgt_reg(int buffer_idx)
{
	int brbe_idx = buffer_to_brbe_idx(buffer_idx);

	BRBE_REGN_SWITCH(brbe_idx, RETURN_READ_BRBTGTN);
}

static inline u64 get_brbinf_reg(int buffer_idx)
{
	int brbe_idx = buffer_to_brbe_idx(buffer_idx);

	BRBE_REGN_SWITCH(brbe_idx, RETURN_READ_BRBINFN);
}

static inline u64 brbe_record_valid(u64 brbinf)
{
	return (brbinf & BRBINF_EL1_VALID_MASK) >> BRBINF_EL1_VALID_SHIFT;
}

static inline bool brbe_invalid(u64 brbinf)
{
	return brbe_record_valid(brbinf) == BRBINF_EL1_VALID_NONE;
}

static inline bool brbe_valid(u64 brbinf)
{
	return brbe_record_valid(brbinf) == BRBINF_EL1_VALID_FULL;
}

static inline bool brbe_source(u64 brbinf)
{
	return brbe_record_valid(brbinf) == BRBINF_EL1_VALID_SOURCE;
}

static inline bool brbe_target(u64 brbinf)
{
	return brbe_record_valid(brbinf) == BRBINF_EL1_VALID_TARGET;
}

static inline int brbe_fetch_cycles(u64 brbinf)
{
	/*
	 * Captured cycle count is unknown and hence
	 * should not be passed on the user space.
	 */
	if (brbinf & BRBINF_EL1_CCU)
		return 0;

	return (brbinf & BRBINF_EL1_CC_MASK) >> BRBINF_EL1_CC_SHIFT;
}

static inline int brbe_fetch_type(u64 brbinf)
{
	return (brbinf & BRBINF_EL1_TYPE_MASK) >> BRBINF_EL1_TYPE_SHIFT;
}

static inline int brbe_fetch_el(u64 brbinf)
{
	return (brbinf & BRBINF_EL1_EL_MASK) >> BRBINF_EL1_EL_SHIFT;
}

static inline int brbe_fetch_numrec(u64 brbidr)
{
	return (brbidr & BRBIDR0_EL1_NUMREC_MASK) >> BRBIDR0_EL1_NUMREC_SHIFT;
}

static inline int brbe_fetch_format(u64 brbidr)
{
	return (brbidr & BRBIDR0_EL1_FORMAT_MASK) >> BRBIDR0_EL1_FORMAT_SHIFT;
}

static inline int brbe_fetch_cc_bits(u64 brbidr)
{
	return (brbidr & BRBIDR0_EL1_CC_MASK) >> BRBIDR0_EL1_CC_SHIFT;
}

static inline void select_brbe_bank(int bank)
{
	static int brbe_current_bank = -1;
	u64 brbfcr;

	if (brbe_current_bank == bank)
		return;

	WARN_ON(bank > 1);
	brbfcr = read_sysreg_s(SYS_BRBFCR_EL1);
	brbfcr &= ~BRBFCR_EL1_BANK_MASK;
	brbfcr |= ((bank << BRBFCR_EL1_BANK_SHIFT) & BRBFCR_EL1_BANK_MASK);
	write_sysreg_s(brbfcr, SYS_BRBFCR_EL1);
	isb();
	brbe_current_bank = bank;
}

static inline void select_brbe_bank_index(int buffer_idx)
{
	switch (buffer_idx) {
	case BRBE_BANK0_IDX_MIN ... BRBE_BANK0_IDX_MAX:
		select_brbe_bank(0);
		break;
	case BRBE_BANK1_IDX_MIN ... BRBE_BANK1_IDX_MAX:
		select_brbe_bank(1);
		break;
	default:
		pr_warn("unsupported BRBE index\n");
	}
}

static inline bool valid_brbe_nr(int brbe_nr)
{
	switch (brbe_nr) {
	case BRBIDR0_EL1_NUMREC_8:
	case BRBIDR0_EL1_NUMREC_16:
	case BRBIDR0_EL1_NUMREC_32:
	case BRBIDR0_EL1_NUMREC_64:
		return true;
	default:
		pr_warn("unsupported BRBE entries\n");
		return false;
	}
}

static inline bool brbe_paused(void)
{
	u64 brbfcr = read_sysreg_s(SYS_BRBFCR_EL1);

	return brbfcr & BRBFCR_EL1_PAUSED;
}

static inline void set_brbe_paused(void)
{
	u64 brbfcr = read_sysreg_s(SYS_BRBFCR_EL1);

	write_sysreg_s(brbfcr | BRBFCR_EL1_PAUSED, SYS_BRBFCR_EL1);
	isb();
}

#endif
