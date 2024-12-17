/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
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
#include <product_config.h>
#include <securec.h>
#include <osl_malloc.h>
#include <bsp_omp.h>
#include "omp_service.h"


#define THIS_MODU mod_omp

struct omp_server g_omp_server[OMP_SERVER_ID_BUTT] = {0};

void bsp_omp_server_reg(omp_srvid_e srvid, omp_srv_func srvfunc)
{
    if (srvid >= OMP_SERVER_ID_BUTT) {
        return;
    }

    g_omp_server[srvid].srvid = srvid;
    g_omp_server[srvid].srvfunc = srvfunc;
}

void omp_service_process(const u8 *buff, u32 buff_size, const u8 *rb_buff, u32 rb_size)
{
    u32 ret, srvid;
    u8 *data = NULL;
    struct omp_service_msg *srvmsg = NULL;
    u32 total_len = buff_size + rb_size;

    if ((buff == NULL) || (!total_len) || (total_len < sizeof(omp_service_head_s))) {
        omp_printf("omp service process param err\n");
        return;
    }

    data = (u8 *)osl_malloc(total_len);
    if (data == NULL) {
        omp_printf("omp service process malloc data fail\n");
        return;
    }

    if (memcpy_s(data, total_len, buff, buff_size)) {
        omp_printf("omp service process memcpy data fail\n");
        goto freedata;
    }

    if ((rb_buff != NULL) && (rb_size != 0)) {
        if (memcpy_s(data + buff_size, total_len - buff_size, rb_buff, rb_size)) {
            omp_printf("omp service process memcpy ring buf data fail\n");
            goto freedata;
        }
    }

    srvid = OMP_SERVER_ID(data);
    srvmsg = (omp_service_head_s *)data;

    if (srvid >= OMP_SERVER_ID_BUTT) {
        omp_printf("omp server id:0x%x is not support\n", srvid);
        goto freedata;
    }

    if (g_omp_server[srvid].srvfunc == NULL) {
        omp_printf("omp server id:0x%x have not register handler\n", srvid);
        goto freedata;
    }

    ret = g_omp_server[srvid].srvfunc((void *)srvmsg, total_len);
    if (ret) {
        omp_printf("omp server:%d id func process err:%d\n", srvid, ret);
    }

freedata:
    osl_free(data);
    return;
}

EXPORT_SYMBOL(bsp_omp_server_reg);

