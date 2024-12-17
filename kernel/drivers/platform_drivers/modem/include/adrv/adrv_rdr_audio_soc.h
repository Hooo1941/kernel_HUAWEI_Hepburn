/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
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

#ifndef ADRV_ADRV__RDR_AUDIO_SOC_H
#define ADRV_ADRV__RDR_AUDIO_SOC_H

/**
 * @brief used to distinguish whether the callback function is called before modem/dsp reset or after reset
 */
enum DRV_RESET_CALLCBFUN_MOMENT {
	DRV_RESET_CALLCBFUN_RESET_BEFORE, /**< modem/HIFI before reset */
	DRV_RESET_CALLCBFUN_RESET_AFTER, /**< modem/HIFI after reset */
	DRV_RESET_CALLCBFUN_RESETING, /**< modem/HIFI resetting */
	DRV_RESET_CALLCBFUN_MOEMENT_INVALID
};

/**
 * @brief record information of callback functions
 */
typedef int (*hifi_reset_cbfunc)(enum DRV_RESET_CALLCBFUN_MOMENT eparam, int userdata);

/**
 * @brief callback registration function before and after dsp reset
 *
 * @par 描述:
 * callback registration function before and after dsp reset
 *
 * @param[in] pname registration func name
 * @param[in] pcbfun callback registration function
 * @param[in] pcbfun user data
 * @param[in] priolevel registration func priority
 *
 * @retval 0 注册成功
 * @retval 非0 注册失败
 */
int hifireset_regcbfunc(const char *pname, hifi_reset_cbfunc pcbfun, int userdata, int priolevel);

#endif /* ADRV_ADRV__RDR_AUDIO_SOC_H */

