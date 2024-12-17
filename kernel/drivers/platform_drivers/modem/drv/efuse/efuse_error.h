/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2020. All rights reserved.
 * foss@huawei.com
 *
 * If distributed as part of the Linux kernel, the following license terms
 * apply:
 *
 * * This program is free software; you can redistribute it and/or modify
 * * it under the terms of the GNU General Public License version 2 and
 * * only version 2 as published by the Free Software Foundation.
 * *
 * * This program is distributed in the hope that it will be useful,
 * * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * * GNU General Public License for more details.
 * *
 * * You should have received a copy of the GNU General Public License
 * * along with this program; if not, write to the Free Software
 * * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
 *
 * Otherwise, the following license terms apply:
 *
 * * Redistribution and use in source and binary forms, with or without
 * * modification, are permitted provided that the following conditions
 * * are met:
 * * 1) Redistributions of source code must retain the above copyright
 * *    notice, this list of conditions and the following disclaimer.
 * * 2) Redistributions in binary form must reproduce the above copyright
 * *    notice, this list of conditions and the following disclaimer in the
 * *    documentation and/or other materials provided with the distribution.
 * * 3) Neither the name of Huawei nor the names of its contributors may
 * *    be used to endorse or promote products derived from this software
 * *    without specific prior written permission.
 *
 * * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */
#ifndef __EFUSE_ERROR_H__
#define __EFUSE_ERROR_H__

#include <bsp_print.h>

#define EFUSE_HUK_VALID (1)
#define EFUSE_HUK_INVALID (0)

#define EFUSE_GROUP_SIZE (4)

/* tee need modify together */
#define EFUSE_OK (0)
#define EFUSE_ERROR (-1)
#define EFUSE_ERROR_ARGS (-2)
#define EFUSE_ERROR_BUFFER_EMPTY (-3)
#define EFUSE_ERROR_REPEAT_WRITE (-4)
#define EFUSE_ERROR_GET_ADDRESS_FAIL (-5)
#define EFUSE_ERROR_MEM_CHECK_FAIL (-6)
#define EFUSE_ERROR_ICC_SEND_FAIL (-7)
#define EFUSE_ERROR_ICC_READ_FAIL (-7)
#define EFUSE_ERROR_CLK_PREPARE_FAIL (-8)
#define EFUSE_ERROR_REGULATOR_GET_FAIL (-9)
#define EFUSE_ERROR_EXIT_PD_TIMEOUT (-10)
#define EFUSE_ERROR_READ_TIMEOUT (-11)
#define EFUSE_ERROR_WRITE_TIMEOUT (-12)
#define EFUSE_ERROR_PRE_WRITE_TIMEOUT (-13)
#define EFUSE_ERROR_GET_LAYOUT_INFO (-14)

#define efuse_print_error(fmt, ...) (bsp_err("<%s> <%d> " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__))
#define efuse_print_info(fmt, ...) (bsp_info(fmt, ##__VA_ARGS__))

#endif
