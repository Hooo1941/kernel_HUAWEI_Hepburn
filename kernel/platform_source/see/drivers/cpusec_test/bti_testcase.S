/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description: kernel bti testcases
 * Create : 2023/2/8
 */

test_bti_fault:
	nop
	ret

.global kernel_bti_testcase
kernel_bti_testcase:
	ldr	x0, =test_bti_fault
	blr	x0
	ret
