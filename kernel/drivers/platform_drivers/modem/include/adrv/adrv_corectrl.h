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

#ifndef ADRV_CORECTRL_H
#define ADRV_CORECTRL_H

#include <linux/module.h>

struct cpufreq_req {
    struct notifier_block nb;
    int cpu;
    unsigned int freq;
};

#ifdef CONFIG_LPCPU_CPUFREQ_DT
/**
 * @brief initlize struct cpufreq
 *
 * @par 描述:
 * initlize struct cpufreq, register cpufreq call chain notifier block
 *
 * @param[in]  req a struct cpufreq_req instance
 * @param[in]  cpu the cpu for which we request frequency
 *
 * @retval zero SUCCESS
 * @retval non-zero FAIL
 */
int lpcpu_cpufreq_init_req(struct cpufreq_req *req, int cpu);

/**
 * @brief exit struct cpufreq
 *
 * @par 描述:
 * iexit struct cpufreq, notifier block
 *
 * @param[in]  req a struct cpufreq_req instance
 *
 * @retval zero SUCCESS
 * @retval non-zero FAIL
 */
void lpcpu_cpufreq_exit_req(struct cpufreq_req *req);

/**
 * @brief update request frequency and trigger a cpufreq scaling
 *
 * @par 描述:
 * update request frequency and trigger a cpufreq scaling
 *
 * @param[in]  req a struct cpufreq_req instance
 * @param[in]  cpu new request frequency
 *
 * @retval NA
 */
void lpcpu_cpufreq_update_req(struct cpufreq_req *req, unsigned int freq);

#else
static inline int lpcpu_cpufreq_init_req(struct cpufreq_req *req, int cpu)
{
    return 0;
}

static inline void lpcpu_cpufreq_exit_req(struct cpufreq_req *req)
{
    return;
}

static inline void lpcpu_cpufreq_update_req(struct cpufreq_req *req, unsigned int freq) {}
#endif

/**
 * @brief disable big cluster isolation for a while
 *
 * @par 描述:
 * disable big cluster isolation for a while
 *
 * @param[in]  timeout the duration we expect for disabling big cluster isolation
 *
 * @retval NA
 */
#ifdef CONFIG_CORE_CTRL
void core_ctl_set_boost(unsigned int timeout);
#else
static inline void core_ctl_set_boost(unsigned int timeout) {}
#endif

#endif /* ADRV_H */
