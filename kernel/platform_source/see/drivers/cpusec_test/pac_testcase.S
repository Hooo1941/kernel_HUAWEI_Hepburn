/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description: pac test for kernel
 * Create : 2023/02/24
 */

#include <asm/asm-offsets.h>

.global	kernel_pac_test_handler

kernel_pac_test_handler:
	mov	x2, x30
	add	x30, x30, #4
	autiasp
	ret
