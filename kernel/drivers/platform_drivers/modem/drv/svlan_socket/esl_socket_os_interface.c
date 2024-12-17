/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2017-2020. All rights reserved.
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


#include "esl_socket_os_interface.h"

unsigned int simsock_readl(const void *addr)
{
    return readl(addr);
}

void simsock_writel(unsigned int val, void *addr)
{
    writel(val, addr);
}

void sim_sleep(int sec)
{
    msleep(sec);
}

sim_dma_addr_t sim_dma_map_single(struct device *dev, void *dam_handler, size_t size, sim_dma_data_direction direction)
{
    sim_dma_addr_t retval;
    retval = dma_map_single(dev, dam_handler, size, direction);
    return retval;
}

void sim_dma_unmap_single(struct device *dev, sim_dma_addr_t dma_handle, size_t size, sim_dma_data_direction direction)
{
    dma_unmap_single(dev, dma_handle, size, direction);
}

void sim_mutex_lock(sim_mutex *m_lock)
{
    mutex_lock(m_lock);
}

void sim_mutex_unlock(sim_mutex *m_unlock)
{
    mutex_unlock(m_unlock);
}

void sim_mutex_init(sim_mutex *m_init)
{
    mutex_init(m_init);
}

int sim_atomic_read(sim_atomic_t *ato_t)
{
    return atomic_read(ato_t);
}

void sim_atomic_set(sim_atomic_t *ato_t, int n)
{
    atomic_set(ato_t, n);
}

int sim_atomic_inc_not_zero(sim_atomic_t *ato_t)
{
    return atomic_inc_not_zero(ato_t);
}

int sim_atomic_dec_and_test(sim_atomic_t *ato_t)
{
    return atomic_dec_and_test(ato_t);
}

int sim_request_irq(unsigned int irq, sim_irq_handler_t handler, unsigned long flags, const char *name, void *dev)
{
    return request_irq(irq, handler, flags, name, dev);
}
