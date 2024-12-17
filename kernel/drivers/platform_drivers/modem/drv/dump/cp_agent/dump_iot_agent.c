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
#include <osl_types.h>
#include <bsp_reset.h>
#include "dump_agent.h"
#include "dump_exc_table.h"

#undef THIS_MODU
#define THIS_MODU mod_dump

void dump_get_cp_exc_node_idx(u32 *idx, u32 modid)
{
    u32 i;
    u32 size = 0;
    dump_mdm_exc_table_info_s *exc_tab = dump_get_cp_exc_tab(&size);
    for (i = 0; i < size; i++) {
        if (exc_tab[i].src_modid_start <= modid && modid <= exc_tab[i].src_modid_end) {
            *idx = i;
            return;
        }
    }
    dump_error("modid invalid\n");
    *idx = dump_get_cp_default_tab_idx();
}

void dump_fill_cp_exc_modid(struct rdr_exception_info_s *rdr_exc, u32 idx, u32 *rdr_modid, u32 modid)
{
    u32 size = 0;
    dump_mdm_exc_table_info_s *exc_tab = dump_get_cp_exc_tab(&size);

    if ((rdr_exc == NULL) || (idx >= size)) {
        return;
    }
    rdr_exc->e_exce_type = exc_tab[idx].e_exce_type;
    rdr_exc->e_modid = exc_tab[idx].exc_info.exc_modid;
    rdr_exc->e_modid_end = exc_tab[idx].exc_info.exc_modid;
    *rdr_modid = exc_tab[idx].exc_info.exc_modid;
}

