/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
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
#include <linux/module.h>
#include <securec.h>
#include <bsp_dump.h>
#include <dump_frame.h>
#include "dump_log_strategy.h"

#undef THIS_MODU
#define THIS_MODU mod_dump

void dump_save_modem_ko(const char *dir_name)
{
#ifdef MODULE
    struct module *module_modem = __module_address((unsigned long)(uintptr_t)dump_save_modem_ko);
    if (module_modem == NULL) {
        return;
    }

    bsp_dump_log_save(dir_name, "modem_ko.bin", module_modem->core_layout.base, NULL, module_modem->core_layout.size);
    dump_error("save modem_ko_core.bin finished\n");
#endif
}

void dump_ko_load_info_init(void)
{
#ifdef MODULE
    struct module *module_dump = NULL;
    struct module *module_modem = NULL;
    if (!dump_is_fulldump()) {
        return;
    }

    /* load info init */
    module_modem = __module_address((unsigned long)(uintptr_t)dump_ko_load_info_init);
    if (module_modem == NULL) {
        return;
    }

    module_dump = (struct module *)bsp_dump_register_field(DUMP_MODEMAP_KO_DUMP, "modem_ko", sizeof(struct module), 0);
    if (module_dump == NULL) {
        dump_error("malloc dump space error 0x%x!\n", (u32)sizeof(struct module));
        return;
    }

    if (EOK != memcpy_s(module_dump, sizeof(struct module), module_modem, sizeof(struct module))) {
        dump_error("save mdmko info err\n");
    }
    bsp_dump_register_log_notifier(DUMP_SAVE_MDM_KO, dump_save_modem_ko);
    return;
#endif
}
