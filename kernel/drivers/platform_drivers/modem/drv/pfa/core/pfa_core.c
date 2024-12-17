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

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <linux/module.h>
#include <linux/clk.h>
#include <net/netfilter/nf_conntrack.h>
#include "pfa.h"
#include "pfa_core.h"
#include "pfa_desc.h"
#include "pfa_ip_entry.h"
#include "pfa_mac_entry.h"
#include "pfa_port.h"
#include "pfa_dbg.h"
#include "pfa_interrupt.h"
#include "pfa_direct_fw.h"
#include "pfa_kernel_hook.h"
#include <bsp_syscache.h>
#ifndef CONFIG_PFA_PHONE_SOC
#include <bsp_cpufreq.h>
#include <bsp_pmctrl.h>
#endif


const char *g_pfa_dfs_propname[DFS_MAX_LEVEL] = {
    "pfa_dfs_freq_grade0", "pfa_dfs_freq_grade1", "pfa_dfs_freq_grade2", "pfa_dfs_freq_grade3", "pfa_dfs_freq_grade4",
    "pfa_dfs_freq_grade5", "pfa_dfs_freq_grade6", "pfa_dfs_freq_grade7", "pfa_dfs_freq_grade8", "pfa_dfs_freq_grade9",
};

struct pfa g_pfa;

#define PFA_FRE_STATE_NUM 10
#define PFA_DFS_KHZ 1000
#define PFA_DFS_1K 1024

int pfa_set_clk_rate(struct pfa *pfa, unsigned int freq_value)
{
    int ret;
    unsigned int freq = freq_value;

    if (pfa->clk_pfa_dbg) {
        freq = pfa->clk_pfa_dbg_rate;
    }

    ret = clk_set_rate(pfa->clks[pfa->clk_pfa_index], freq);
    if (ret) {
        PFA_ERR("pfa set freq fail, ret = 0x%x \n", ret);
    }

    ret = clk_set_rate(pfa->clks[pfa->clk_pfa_tft_index], freq);
    if (ret) {
        PFA_ERR("pfa set tft freq fail, ret = 0x%x \n", ret);
    }

    return 0;
}

static inline int pfa_calc_freq_grade(struct pfa *pfa, unsigned int byte)
{
    int i;

    for (i = pfa->pfa_fre_grade_max; i > pfa->last_dfs_grade; i--) {
        if (byte > pfa->freq_grade_array[i].freq_max * PFA_DFS_1K) {
            return i;
        }
    }
    return i;
}

unsigned int pfa_byte_to_grade(unsigned int byte)
{
    struct pfa *pfa = &g_pfa;
    unsigned int grade = pfa->last_dfs_grade;
    int i = 0;

    if (unlikely(pfa->last_dfs_grade >= DFS_MAX_LEVEL)) {
        grade = pfa->pfa_fre_grade_max;
        return grade;
    }

    if (byte >= pfa->freq_grade_array[pfa->last_dfs_grade].freq_max * PFA_DFS_1K) {
        if (pfa->last_byte < pfa->start_byte_limit) {
            grade = pfa->pfa_fre_grade_max;
        } else {
            grade = pfa_calc_freq_grade(pfa, byte);
        }
    } else if (pfa->last_dfs_grade == 0) {
        grade = pfa->last_dfs_grade;
    } else {
        for (i = 0; i < pfa->last_dfs_grade; i++) {
            if (byte < pfa->freq_grade_array[i].freq_min * PFA_DFS_1K) {
                grade = i;
                break;
            }
        }
    }
    return grade;
}

void pfa_set_peri_volt(struct pfa *pfa, enum peri_volt_adjust volt)
{
#if (!defined CONFIG_PFA_PHONE_SOC)
    unsigned int ret;
    profile_id_e level;

    switch (volt) {
        case PERI_VOLT_DOWN:
            level = PROFILE_ID_0;
            break;
        case PERI_VOLT_UP:
            level = PROFILE_ID_1;
            break;
        default:
            level = PROFILE_ID_2;
            break;
    }

    ret = bsp_pmctrl_set_profile_sync(pfa->div_pmctrl, level);
    if (ret != 0) {
        PFA_ERR("pfa set peri volt fail, ret = 0x%x \n", ret);
    }
#else
    PFA_INFO("pfa set peri volt do not exec. \n");
#endif
    return;
}

int pfa_pmctrl_creat(struct pfa *pfa)
{
#if (!defined CONFIG_PFA_PHONE_SOC)
    int ret;
    unsigned int pmctrl_handle = 0;
    static const char name[] = "pfa_dvfs";

    ret = bsp_pmctrl_create(PERI_PMCTRL, name, &pmctrl_handle);
    if (ret != 0) {
        PFA_ERR("pfa pmctrl creat fail, ret = 0x%x \n", ret);
        return ret;
    }
    pfa->div_pmctrl = pmctrl_handle;
    PFA_INFO("pfa pmctrl creat sucess. \n");
#else
    PFA_INFO("pfa pmctrl creat do not exec. \n");
#endif
    return 0;
}

void pfa_set_clk_div_self(struct pfa *pfa, int cur_dfs_grade)
{
    enum peri_volt_adjust peri_volt = PERI_VOLT_NO_CHANGE;
    pfa_div_ctrl_t pfa_div;

    // 配置pfa频率及电压
    if (pfa->div_profile[cur_dfs_grade] + 1 == pfa->clk_div) {
        PFA_INFO("pfa div no change. \n");
        return;
    } else if (pfa->div_profile[cur_dfs_grade] + 1 < pfa->clk_div) {  // pfa 频率只有2档，0档位2分频，其他都为1分频
        peri_volt = PERI_VOLT_UP;                                     // 升压升频
    } else {
        peri_volt = PERI_VOLT_DOWN;  // 降压降频
    }

    if (peri_volt == PERI_VOLT_UP) {
        pfa_set_peri_volt(pfa, PERI_VOLT_UP);
    }

    pfa_div.u32 = pfa->clk_div_default;
    pfa_div.bits.pfa_div = pfa->div_profile[cur_dfs_grade];
    writel(pfa_div.u32, pfa->peri.crg_addr + PFA_CLK_DIV);
    pfa->clk_div = pfa->div_profile[cur_dfs_grade] + 1;
    pfa->dfs_div_cnt++;

    if (peri_volt == PERI_VOLT_DOWN) {
        pfa_set_peri_volt(pfa, PERI_VOLT_DOWN);
    }
}

void pfa_set_freq_volt(struct pfa *pfa, int cur_dfs_grade)
{
    if (pfa->pfa_dfs_div_enable) {
        if (pfa->clk_div_self) {
            pfa_set_clk_div_self(pfa, cur_dfs_grade);
        } else {
            pfa_set_clk_rate(pfa, pfa->div_profile[cur_dfs_grade]);
        }
    }
    return;
}
static inline void pfa_set_ddr_freq(struct pfa *pfa, int cur_dfs_grade)
{
#ifndef CONFIG_PFA_PHONE_SOC
    unsigned int ddr_freq;
    unsigned int cpu_freq;
    int ret;

    ddr_freq = PFA_DFS_KHZ * pfa->ddr_profile[pfa->cur_dfs_grade];
    cpu_freq = PFA_DFS_KHZ * pfa->cpu_profile[pfa->cur_dfs_grade];
    ret = bsp_dfs_set_vote_sync(pfa->dfs_reg_ddr_id, ddr_freq);
    if (ret) {
        PFA_ERR("update ddr freq fail. \n");
        return;
    }

    ret = bsp_dfs_set_vote_sync(pfa->dfs_reg_acore_id, cpu_freq);
    if (ret) {
        PFA_ERR("update cpu freq fail. \n");
        return;
    }
#endif
    return;
}

void pfa_get_total_byte(struct pfa *pfa)
{
    unsigned long td_bytes = 0;
    unsigned long rd_bytes = 0;
    unsigned int dfs_td_count = 0;
    unsigned int dfs_rd_count = 0;
    unsigned int i = 0;
    unsigned int multiple = 1;

    for (i = 0; i < PFA_PORT_NUM; i++) {
        if (pfa->ports[i].ctrl.dfs_speed_multiple_en) {
            multiple = pfa->ports[i].ctrl.dfs_speed_multiple;
        }
        dfs_td_count += pfa->ports[i].stat.td_pkt_complete * multiple;
        dfs_rd_count += pfa->ports[i].stat.rd_finsh_pkt_num * multiple;
    }

    pfa->dfs_td_count = dfs_td_count;
    pfa->dfs_rd_count = dfs_rd_count;

    if (pfa->dfs_td_count >= pfa->dfs_td_count_last) {
        td_bytes = (pfa->dfs_td_count - pfa->dfs_td_count_last) * PFA_AD1_PKT_LEN;
    } else {
        td_bytes = (0xffffffff - pfa->dfs_td_count_last + pfa->dfs_td_count) * PFA_AD1_PKT_LEN;
    }

    if (pfa->dfs_rd_count >= pfa->dfs_rd_count_last) {
        rd_bytes = (pfa->dfs_rd_count - pfa->dfs_rd_count_last) * PFA_AD1_PKT_LEN;
    } else {
        rd_bytes = (0xffffffff - pfa->dfs_rd_count_last + pfa->dfs_rd_count) * PFA_AD1_PKT_LEN;
    }

    if (td_bytes > rd_bytes) {
        pfa->total_byte = td_bytes;
    } else {
        pfa->total_byte = rd_bytes;
    }
}

void pfa_dfs_ctrl_work(struct work_struct *work)
{
    struct pfa *pfa = &g_pfa;

    pfa->dfs_timer_cnt++;

    if (unlikely(!pfa->pfa_dfs_enable)) {
        PFA_ERR("pfa dfs disabled ! \n");
        return;
    }

    pfa_get_total_byte(pfa);

    pfa->byte = (pfa->total_byte / 1024) * 8 * (1000 / pfa->pfa_dfs_time_interval); /* kbps, 1024,8,1000 unit trans */
    pfa->dfs_td_count_last = pfa->dfs_td_count;
    pfa->dfs_rd_count_last = pfa->dfs_rd_count;
    pfa->cur_dfs_grade = pfa_byte_to_grade(pfa->byte);

    if (pfa->cur_dfs_grade != pfa->last_dfs_grade) {
        pfa->dfs_cnt++;
        pfa_set_freq_volt(pfa, pfa->cur_dfs_grade);  // pfa调频调压
        pfa_set_ddr_freq(pfa, pfa->cur_dfs_grade);   // pfa投票ddr频率
    }

    pfa->last_dfs_grade = pfa->cur_dfs_grade;
    pfa->last_byte = pfa->byte;
    pfa->xfrm_dfs_cnt = 0;
    queue_delayed_work(pfa->dfs_workqueue, &pfa->dfs_work, msecs_to_jiffies(pfa->pfa_dfs_time_interval));
}

void pfa_get_dfs_div_cfg(struct pfa *pfa, device_node_s *np)
{
    int ret;

    pfa->pfa_dfs_div_enable = 1;
    ret = bsp_dt_property_read_u32_array(np, "pfa_dfs_div_shifts", (u32 *)pfa->div_profile, pfa->pfa_fre_grade_max + 1);
    if (ret) {
        PFA_ERR(" pfa div profile disabled \n");
        pfa->pfa_dfs_div_enable = 0;
    }
    pfa->clk_div_default = PFA_CLK_DIV_DEFAULT;
    ret = bsp_dt_property_read_u32_array(np, "pfa_dfs_clk_div_default", &pfa->clk_div_default, 1);
    if (ret) {
        PFA_ERR("pfa_dfs_clk_div_default use default, clk_div_default:0x%x\n", pfa->clk_div_default);
    }

    pfa->clk_div_self = 1;
    ret = bsp_dt_property_read_u32_array(np, "pfa_clk_div_self", &pfa->clk_div_self, 1);
    if (ret) {
        PFA_ERR("pfa_clk_div_self use default, pfa_clk_div_self:0x%x\n", pfa->clk_div_self);
    }
    if (pfa->clk_div_self == 0) {
        ret = bsp_dt_property_read_u32_array(np, "clk_pfa_index", &pfa->clk_pfa_index, 1);
        if (ret) {
            PFA_ERR("no clk_pfa_index err!\n");
        }
        ret = bsp_dt_property_read_u32_array(np, "clk_pfa_tft_index", &pfa->clk_pfa_tft_index, 1);
        if (ret) {
            PFA_ERR("no clk_pfa_tft_index err!\n");
        }
    }
}

void pfa_get_dfs_arg(void)
{
    struct pfa *pfa = &g_pfa;
    device_node_s *np = pfa->np;
    int ret;
    unsigned int i = 0;

    pfa->pfa_dfs_enable = 0;
    pfa->pfa_dfs_time_interval = PFA_DFS_INTERVAL_DEFAULT;
    ret = bsp_dt_property_read_u32_array(np, "pfa_fre_grade_max", &pfa->pfa_fre_grade_max, 1);
    if (ret || pfa->pfa_fre_grade_max >= DFS_MAX_LEVEL) {
        PFA_ERR("failed to get pfa_fre_grade_max from dts, ret:0x%x, pfa_fre_grade_max:%d\n", ret, pfa->pfa_fre_grade_max);
        return;
    }

    ret = bsp_dt_property_read_u32_array(np, "pfa_dfs_interval", &pfa->pfa_dfs_time_interval, 1);
    if (ret) {
        PFA_ERR("failed to get pfa_dfs_time_interval from dts\n");
        return;
    }

    for (i = 0; i <= pfa->pfa_fre_grade_max; i++) {
        ret = bsp_dt_property_read_u32_array(np, g_pfa_dfs_propname[i], (u32 *)(pfa->freq_grade_array + i), 3); /* 3 */
        if (ret) {
            PFA_ERR("failed to get pfa_dfs_freq_grade from dts   i=%d\n", i);
            return;
        }
    }

    ret = bsp_dt_property_read_u32_array(np, "pfa_dfs_cpu_shifts", (u32 *)pfa->cpu_profile, pfa->pfa_fre_grade_max + 1);
    if (ret) {
        PFA_ERR(" pfa cpu profile do not support in phone. \n");  // do not return
    }
    ret = bsp_dt_property_read_u32_array(np, "pfa_dfs_ddr_shifts", (u32 *)pfa->ddr_profile, pfa->pfa_fre_grade_max + 1);
    if (ret) {
        PFA_ERR(" pfa ddr profile do not support in phone. \n");  // do not return
    }

    pfa_get_dfs_div_cfg(pfa, np);

    pfa->start_byte_limit = PFA_DEFAULT_START_LIMIT;
    pfa->pfa_dfs_enable = 1;

    return;
}

int pfa_dfs_delay_work_init(struct pfa *pfa)
{
    pfa->dfs_workqueue = create_singlethread_workqueue("pfa_dfs_work");
    if (pfa->dfs_workqueue == NULL) {
        PFA_ERR("create dfs work fail\n");
        return -1;
    }
    INIT_DELAYED_WORK(&pfa->dfs_work, pfa_dfs_ctrl_work);
    queue_delayed_work(pfa->dfs_workqueue, &pfa->dfs_work, pfa->pfa_dfs_time_interval);

    return 0;
}

int pfa_dfs_init(void)
{
    struct pfa *pfa = &g_pfa;
    pfa_div_ctrl_t pfa_div;
    PFA_INFO("pfa->pfa_dfs_enable %u \n", pfa->pfa_dfs_enable);

    pfa_get_dfs_arg();

    if (pfa->pfa_dfs_enable) {
        pfa->cur_dfs_grade = 0;
        pfa->last_dfs_grade = DFS_MAX_LEVEL;
        pfa->dfs_td_count = 0;
        pfa->dfs_td_count_last = 0;
        pfa->dfs_rd_count = 0;
        pfa->dfs_rd_count_last = 0;
        pfa->ddr_freq_requested = 0;
        pfa->byte = 0;
        pfa->last_byte = 0;
        pfa->total_byte = 0;
        pfa->start_byte_limit = 0;
        pfa->dfs_timer_cnt = 0;

#ifdef CONFIG_PFA_PHONE_SOC
        PFA_INFO("pfa dfs at phone mode ! \n");
#else
        PFA_INFO("pfa dfs at mbb mode ! \n");
        bsp_dfs_create_handle(DFS_DDR_FREQ, "pfa", &pfa->dfs_reg_ddr_id);
        bsp_dfs_create_handle(DFS_CPU_FREQ, "pfa", &pfa->dfs_reg_acore_id);
#endif

        if (pfa->pfa_dfs_div_enable) {
            (void)pfa_pmctrl_creat(pfa);

            pfa_div.u32 = pfa->clk_div_default;
            pfa_div.bits.pfa_div = pfa->div_profile[PFA_CLK_DIV_FULL_MIN_LEVEL];
            writel(pfa_div.u32, pfa->peri.crg_addr + PFA_CLK_DIV);
            pfa->clk_div = pfa->div_profile[PFA_CLK_DIV_FULL_MIN_LEVEL] + 1;
            PFA_INFO("pfa_dfs_div_enabled ! set div to %u\n", pfa->clk_div);
        }

        pfa_dfs_delay_work_init(pfa);
        pfa->xfrm_dfs_cnt = 0;
    }
    return 0;
}

void pfa_dfs_get(void)
{
    struct pfa *pfa = &g_pfa;

    PFA_ERR("Settings \n");
    PFA_ERR("pfa->pfa_dfs_enable %u \n", pfa->pfa_dfs_enable);
    PFA_ERR("pfa->pfa_dfs_div_enable %u \n", pfa->pfa_dfs_div_enable);
    PFA_ERR("pfa->pfa_dfs_time_interval : %d \n", pfa->pfa_dfs_time_interval);
    PFA_ERR("pfa->pfa_fre_grade_max: %d \n", pfa->pfa_fre_grade_max);
    PFA_ERR("pfa->dfs_div_threshold_pktnum: %d \n", pfa->dfs_div_threshold_pktnum);

    PFA_ERR("Debug infos \n");
    PFA_ERR("pfa->cur_dfs_grade:%d \n", pfa->cur_dfs_grade);
    PFA_ERR("pfa->last_dfs_grade:%d \n", pfa->last_dfs_grade);
    PFA_ERR("pfa->dfs_td_count:%u \n", pfa->dfs_td_count);
    PFA_ERR("pfa->dfs_td_count_last:%u \n", pfa->dfs_td_count_last);
    PFA_ERR("pfa->dfs_rd_count:%u \n", pfa->dfs_rd_count);
    PFA_ERR("pfa->dfs_rd_count_last:%u \n", pfa->dfs_rd_count_last);
    PFA_ERR("pfa->total_byte:%u \n", pfa->total_byte);
    PFA_ERR("pfa->byte:%u \n", pfa->byte);
    PFA_ERR("pfa->clk_div:%d \n", pfa->clk_div);
    PFA_ERR("pfa->dfs_cnt:%d \n", pfa->dfs_cnt);
    PFA_ERR("pfa->dfs_timer_cnt:%d \n", pfa->dfs_timer_cnt);
}

void pfa_transfer_ctx_init(struct pfa *pfa)
{
    pfa->flags = 0;
    pfa->mask_flags = 0;

    pfa->min_pkt_len = PFA_MIN_PKT_SIZE;
    pfa->cpuport.portno = PFA_PORT_MAGIC;

    pfa->pfa_tftport.portno = PFA_PORT_MAGIC;
    if (pfa->pfa_version == PFA_VERSION_V200) {
        pfa->pfa_tftport.portno = PFA_CCORE_PORT_BEGIN;
    }

    pfa->wanport.portno = PFA_PORT_MAGIC;
    pfa->usbport.portno = PFA_PORT_MAGIC;

    pfa->cpuport.cpu_pkt_max_rate = PFA_CPU_PORT_DEF_MAX_RATE;
    pfa->cpuport.cpu_pktnum_per_interval = PFA_PER_INTERVAL_CALC(pfa->cpuport.cpu_pkt_max_rate);
    pfa->ipfw.ipfw_timeout = PFA_IP_FW_TIMEOUT;
    pfa->macfw.macfw_timeout = PFA_MAC_FW_TIMEOUT;
    pfa->usbport.attr.gether_timeout = PFA_NCM_WRAP_TIMEOUT;

    pfa->hal->transfer_ctx_init(pfa);
}

void pfa_set_update_only_produce_mod(struct pfa *pfa)
{
    pfa->hal->set_update_only_produce_mod(pfa);
}

void __pfa_core_init(struct pfa *pfa)
{
    pfa_black_white_t list;

    pfa_transfer_ctx_init(pfa);

    /* set ip fw hash bucket base and depth ,width unit is word */
    pfa_ip_fw_htab_set();

    /* set ip fw mac fw agint timer */
    pfa_aging_timer_set();

    pfa_set_update_only_produce_mod(pfa);

    // pfa init blck white list
    list.u32 = pfa_readl(pfa->regs, PFA_BLACK_WHITE);
    list.bits.pfa_mac_black_white = 1;
    list.bits.pfa_ip_black_white = 1;
    pfa_writel(pfa->regs, PFA_BLACK_WHITE, list.u32);

    pfa_direct_fw_init(pfa);
    pfa_reset_modem_init();
    spin_lock_init(&pfa->reset_lock);

    pfa->hal->ackpkt_identify(pfa, PFA_DEFAULT_PFA_TFT_PORT);

    pfa_om_dump_init(pfa);

    if (pfa->clock_gate_en) {
        pfa->hal->clk_gate_switch(pfa, true);
    }

    pfa->wakeup_flag = FALSE;

#ifndef CONFIG_PFA_PHONE_SOC
    if (pfa->soft_push) { /* evide dra hard error */
        pfa_adq_timer_init(pfa);
        spin_lock_init(&pfa->pfa_ad_lock);
    }
#endif
    return;
}

int pfa_init_config_sc(struct pfa *pfa)
{
    struct sc_mode mode;
    int ret;

    ret = bsp_sc_get_sc_mode(&mode, SC_READ_ALLOC, SC_WRITE_ALLOC, 0);
    if (ret) {
        bsp_err("get sc mode fail\n");
        return -1;
    }

    pfa->hal->config_sc(pfa, mode.sc_idx, mode.cfg);
    return 0;
}

int pfa_core_init(struct pfa *pfa)
{
    int ret;

    ret = pfa_ad_init(pfa);
    if (ret) {
        PFA_ERR("pfa_ad_init failed, ret:%d\n", ret);
        goto error;
    }

    ret = pfa_interrput_init(pfa);
    if (ret) {
        PFA_ERR("failed to request irq #%d --> %d\n", pfa->irq, ret);
        goto error;
    }

    ret = pfa_init_config_sc(pfa);
    if (ret) {
        goto error;
    }

    pfa_register_hook();

    __pfa_core_init(pfa);

    return 0;

error:
    pfa_ad_exit(pfa);

    return ret;
}

void pfa_core_exit(struct pfa *pfa)
{
    PFA_TRACE("enter\n");

    if (pfa->irq) {
        free_irq(pfa->irq, pfa);
        pfa->irq = 0;
    }
    if (pfa->core_mem_info.memorys != NULL) {
        kfree(pfa->core_mem_info.memorys);
        pfa->core_mem_info.memorys = NULL;
    }

    pfa_ip_entry_table_exit(pfa);
    pfa_mac_entry_table_exit(pfa);

    PFA_TRACE("exit\n");

    return;
}

void pfa_transfer_pause(void)
{
    struct pfa *pfa = &g_pfa;
    unsigned long flags;
    unsigned int tab_clean;

    spin_lock_irqsave(&pfa->macfw.lock, flags);
    pfa_writel(pfa->regs, PFA_HASH_DEPTH, PFA_IPFW_HTABLE_EMPTY_SIZE);
    pfa_writel(pfa->regs, PFA_HASH_BADDR_H, upper_32_bits(pfa->ipfw.hbucket_dma_empty));
    pfa_writel(pfa->regs, PFA_HASH_BADDR_L, lower_32_bits(pfa->ipfw.hbucket_dma_empty));

    pfa->hal->config_hash_cache_en(pfa, 0);

    tab_clean = pfa_readl(pfa->regs, PFA_TAB_CLR);
    tab_clean |= BIT(1);
    tab_clean |= BIT(4); /* get bit 4 */
    pfa_writel(pfa->regs, PFA_TAB_CLR, tab_clean);

    pfa->mask_flags = 1;
    spin_unlock_irqrestore(&pfa->macfw.lock, flags);

    PFA_ERR("pfa hardware forward pause\n");
}

void pfa_transfer_restart(void)
{
    struct pfa *pfa = &g_pfa;
    unsigned long flags;
    unsigned int tab_clean;
    spin_lock_irqsave(&pfa->macfw.lock, flags);
    pfa_writel(pfa->regs, PFA_HASH_BADDR_H, upper_32_bits(pfa->ipfw.hbucket_dma));
    pfa_writel(pfa->regs, PFA_HASH_BADDR_L, lower_32_bits(pfa->ipfw.hbucket_dma));
    pfa_writel(pfa->regs, PFA_HASH_DEPTH, pfa->ipfw.hlist_size);

    pfa->hal->config_hash_cache_en(pfa, 1);

    tab_clean = pfa_readl(pfa->regs, PFA_TAB_CLR);
    tab_clean &= ~(BIT(1));
    tab_clean &= ~(BIT(4)); /* get bit 4 */
    pfa_writel(pfa->regs, PFA_TAB_CLR, tab_clean);

    pfa->mask_flags = 0;
    {
        struct pfa_mac_fw_entry_ext *pos = NULL;
        struct pfa_mac_fw_entry_ext *n = NULL;

        list_for_each_entry_safe(pos, n, &pfa->macfw.backups, list)
        {
            pfa_mac_entry_config(PFA_TAB_CTRL_DEL_MAC_FW, (unsigned int *)&pos->ent,
                sizeof(pos->ent) / sizeof(unsigned int));
            pfa_mac_entry_config(PFA_TAB_CTRL_ADD_MAC_FW, (unsigned int *)&pos->ent,
                sizeof(pos->ent) / sizeof(unsigned int));
        }
    }

    spin_unlock_irqrestore(&pfa->macfw.lock, flags);
    PFA_ERR("pfa hardware forward restart\n");
}

void bsp_pfa_transfer_pause(void)
{
    pfa_transfer_pause();
}

void bsp_pfa_transfer_restart(void)
{
    pfa_transfer_restart();
}

pfa_version_type mdrv_pfa_get_version(void)
{
    if (g_pfa.hal->get_version == NULL) {
        return PFA_VER_ERR;
    }
    return g_pfa.hal->get_version();
}

EXPORT_SYMBOL(g_pfa);
EXPORT_SYMBOL(bsp_pfa_transfer_pause);
EXPORT_SYMBOL(bsp_pfa_transfer_restart);
EXPORT_SYMBOL(pfa_dfs_get);
EXPORT_SYMBOL(mdrv_pfa_get_version);

MODULE_ALIAS("network hardware accelerator driver");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("smart packet engine(pfa) driver");

