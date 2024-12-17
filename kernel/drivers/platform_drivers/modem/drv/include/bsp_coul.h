/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
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

#ifndef _BSP_COUL_H_
#define _BSP_COUL_H_

#include "product_config.h"
#include "osl_types.h"

struct coul_cali_ops {
    const char *dev_name;
    void *dev_data;
    int (*get_current)(int *);
    int (*get_voltage)(int *);
    int (*get_current_offset)(int *);
    int (*get_current_gain)(unsigned int *);
    int (*get_voltage_offset)(int *);
    int (*get_voltage_gain)(unsigned int *);
    int (*set_current_offset)(int);
    int (*set_current_gain)(unsigned int);
    int (*set_voltage_offset)(int);
    int (*set_voltage_gain)(unsigned int);
    int (*set_cali_mode)(int);
};

struct coul_interface_ops {
    const char *type_name;
    void *dev_data;
    int (*is_coul_ready)(void);
    int (*is_battery_exist)(void);
    int (*get_battery_capacity)(void);
    int (*get_battery_voltage)(void);
    int (*get_battery_current)(void);
    int (*get_battery_avg_current)(void);
    int (*get_battery_temperature)(void);
    int (*get_battery_cycle)(void);
    int (*get_battery_fcc)(void);
    int (*set_vterm_dec)(int);
    int (*set_battery_low_voltage)(int);
    int (*set_battery_last_capacity)(int);
    int (*get_battery_last_capacity)(void);
    int (*get_battery_rm)(void);
};

void coul_interface_ops_register(struct coul_interface_ops *ops);
void coul_cali_ops_register(struct coul_cali_ops *ops);

#endif /* end #define _BSP_COUL_H_*/
