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

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/interrupt.h>
#include <linux/etherdevice.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <linux/clk.h>
#include <linux/syscore_ops.h>
#include <linux/of_device.h>

#include "bsp_dt.h"
#include <securec.h>
#include <bsp_sysctrl.h>
#include "bsp_version.h"
#include "pfa.h"
#include "pfa_core.h"
#include "pfa_port.h"
#include "pfa_dbg.h"
#include "pfa_entry_hook.h"
#include "pfa_tft.h"
#include "pfa_img.h"
#include "pfa_load.h"
#include "pfa_ip_entry.h"
#include "pfa_mac_entry.h"
#include "pfa_desc.h"
#include "pfa_interrupt.h"
#include "wan.h"

#define PFA_MODULE_NAME "pfa"

void entry_store(void)
{
    struct pfa *pfa = &g_pfa;
    unsigned int *cur_buf = pfa->entry_bak;
    int i;

    for (i = 0; i < pfa->backup_entry_num; i++) {
        *cur_buf = pfa_readl(pfa->regs, PFA_MAC_TAB(i));
        cur_buf++;
    }
}

void entry_restore(void)
{
    int i;
    struct pfa *pfa = &g_pfa;
    unsigned int *cur_buf = pfa->entry_bak;

    for (i = 0; i < pfa->backup_entry_num; i++) {
        pfa_writel(pfa->regs, PFA_MAC_TAB(i), *cur_buf);
        cur_buf++;
    }
}

int pfa_get_clks(struct pfa *pfa, struct device *dev, struct clk ***clks, int *num_clks)
{
    device_node_s *np = pfa->np;
    struct property *prop = NULL;
    const char *name = NULL;
    struct clk *clk = NULL;
    int i;

    if (!dev || !clks || !num_clks) {
        return -EINVAL;
    }

    *num_clks = bsp_dt_property_count_strings(np, "clock-names");
    if (*num_clks < 1 || *num_clks > PFA_CLKS_MAX_NUM) {
        *num_clks = 0;
        return -EINVAL;
    }

    /* clks is struct clk* point array */
    *clks = devm_kcalloc(dev, *num_clks, sizeof(struct clk *), GFP_KERNEL);
    if (!(*clks)) {
        *num_clks = 0;
        return -ENOMEM;
    }

    i = 0;
    bsp_dt_property_for_each_string(np, "clock-names", prop, name)
    {
        if (i == *num_clks) {
            PFA_ERR("clk number error.\n");
            break;
        }

        clk = devm_clk_get(dev, name);
        if (IS_ERR(clk)) {
            PFA_ERR("failed to get %s\n", name);
            return PTR_ERR(clk);
        }

        (*clks)[i] = clk;
        ++i;
    }
    return 0;
}

int pfa_init_clks(struct clk **clks, int num_clks)
{
    int i;
    int ret;

    if (!clks) {
        return -EINVAL;
    }

    for (i = 0; i < num_clks; i++) {
        ret = clk_prepare_enable(clks[i]);
        if (ret) {
            PFA_ERR("[%s]enable clk failed", __func__);
            while (i--) {
                clk_disable_unprepare(clks[i]);
            }
            return ret;
        }
    }

    return 0;
}

static int pfa_clk_enable(struct device *dev)
{
    struct pfa *pfa = &g_pfa;
    int ret;
    PFA_INFO("pfa_clk_enable \n");
    ret = pfa_get_clks(pfa, dev, &pfa->clks, &pfa->clks_num);
    if (ret) {
        PFA_ERR("clk:pfa_clk is NULL, can't find it,please check! ret:%d \n", ret);
        return ret;
    } else {
        ret = pfa_init_clks(pfa->clks, pfa->clks_num);
        if (ret) {
            PFA_ERR("clk:pfa_clk enable failed!!\n");
            return ret;
        }
    }
    return 0;
}

void pfa_get_peri_arg(struct pfa *pfa, device_node_s *np)
{
    struct pfa_pericrg *peri = &g_pfa.peri;
    dma_addr_t percrg_addr;
    int ret;

    peri->complete_flag = 0;

    ret = bsp_dt_property_read_u32_array(np, "pfa_pericrg_addr", &peri->crg_phy_addr, 1);
    if (ret) {
        PFA_ERR("skip find of pfa_pericrg_addr \n");
        return;
    }

    ret = bsp_dt_property_read_u32_array(np, "pfa_pericrg_len", &peri->crg_len, 1);
    if (ret) {
        PFA_ERR("skip find of pfa_pericrg_len\n");
        return;
    }

    percrg_addr = peri->crg_phy_addr;
    peri->crg_addr = ioremap(percrg_addr, peri->crg_len);
    if (peri->crg_addr == NULL) {
        PFA_ERR("pfa get crg base addr fail \n");
        return;
    }

    ret = bsp_dt_property_read_u32_array(np, "pfa_peri_rst_en", &peri->crg_rst_en, 1);
    if (ret) {
        PFA_ERR("skip find of pfa_peri_rst_en\n");
        return;
    }

    ret = bsp_dt_property_read_u32_array(np, "pfa_peri_rst_dis", &peri->crg_rst_dis, 1);
    if (ret) {
        PFA_ERR("skip find of pfa_peri_rst_dis\n");
        return;
    }

    ret = bsp_dt_property_read_u32_array(np, "pfa_peri_rst_stat", &peri->crg_rst_stat, 1);
    if (ret) {
        PFA_ERR("skip find of pfa_peri_rst_stat\n");
        return;
    }

    ret = bsp_dt_property_read_u32_array(np, "pfa_peri_rst_mask", &peri->crg_rst_mask, 1);
    if (ret) {
        PFA_ERR("skip find of pfa_peri_rst_mask\n");
        return;
    }

    peri->complete_flag = 1;

    ret = bsp_dt_property_read_u32_array(np, "clock_gate_en", &pfa->clock_gate_en, 1);
    if (ret) {
        PFA_ERR("skip find of clock_gate_en\n");
        return;
    }
    return;
}

void pfa_get_plat_version(struct pfa *pfa)
{
    const bsp_version_info_s *ver;

    ver = bsp_get_version_info();
    if (ver == NULL) {
        PFA_ERR("pfa get plat version fail\n");
        return;
    }
    pfa->plat_type = ver->plat_type;
}

void pfa_sysctrl_init(struct pfa *pfa)
{
    device_node_s *np = NULL;
    const __be32 *reg_addr_p = NULL;
    u64 reg_phy;
    void __iomem *reg_vir;
    int na;

    pfa_get_plat_version(pfa);

    np = bsp_dt_find_compatible_node(NULL, NULL, "pfa");
    pfa->np = np;

    reg_addr_p = bsp_dt_get_property(np, "reg", NULL);
    if (reg_addr_p == NULL) {
        PFA_ERR("get_address failed.\r\n");
        return;
    }
    na = bsp_dt_n_addr_cells(np);
    reg_phy = bsp_dt_read_number(reg_addr_p, na);
    reg_vir = bsp_dt_iomap(np, 0);
    if (reg_vir == NULL) {
        PFA_ERR("iomap failed.\r\n");
        return;
    }
    pfa->res = reg_phy;
    pfa->regs = reg_vir;

    pfa->irq = bsp_dt_irq_parse_and_map(np, 0);
    if (pfa->irq < 0) {
        PFA_ERR("PFA IRQ configuration information not found\n");
        return;
    }

    pfa_get_peri_arg(pfa, np);

    return;
}

void pfa_sysctrl_exit(void)
{
    return;
}

void pfa_core_unreset(void)
{
    struct pfa_pericrg *peri = &g_pfa.peri;
    unsigned int reg;

    if (peri->complete_flag != 1) {
        return;
    }

    writel(peri->crg_rst_mask, peri->crg_addr + peri->crg_rst_en);
    mdelay(1); /* 1 ms delay */
    writel(peri->crg_rst_mask, peri->crg_addr + peri->crg_rst_dis);
    mdelay(1); /* 1 ms delay */

    reg = readl(peri->crg_addr + peri->crg_rst_stat);
    PFA_INFO("pfa unreset state %x\n", reg);

    return;
}

int pfa_pm_prepare(struct device *pdev)
{
    struct pfa *pfa = &g_pfa;
    unsigned long flags;
    int ret;

    if (pfa->plat_type == PLAT_ESL) {
        pfa->plat_esl++;
        return 0;
    }

    spin_lock_irqsave(&pfa->pm_lock, flags);
    // pfa tft
    ret = pfa_tft_pm_prepare();
    if (ret) {
        spin_unlock_irqrestore(&pfa->pm_lock, flags);
        PFA_ERR("pfa tft prepare fail, ret = %d  \n", ret);
        return -EBUSY;
    }

    // judge pfa_idle
    if (pfa->hal->check_idle(pfa)) {
        pfa->not_idle++;
        pfa_rwptr_show();
        spin_unlock_irqrestore(&pfa->pm_lock, flags);
        return -EBUSY;
    }

    spin_unlock_irqrestore(&pfa->pm_lock, flags);

    return 0;
}

int pfa_pm_suspend(struct device *pdev)
{
    struct pfa *pfa = &g_pfa;
    unsigned long flags;

    if (pfa->plat_type == PLAT_ESL) {
        pfa->plat_esl++;
        return 0;
    }

    spin_lock_irqsave(&pfa->pm_lock, flags);

    // pfa tft
    pfa_tft_pm_suspend();

    // close pfa_en
    pfa->flags &= (~PFA_FLAG_ENABLE);
    pfa->hal->config_disable();

    // open dbgen
    pfa->hal->dbgen_en();

    // store entrys
    entry_store();

    // close dbgen
    pfa->hal->dbgen_dis();

    pfa->halt = 1;

    pfa->hal->config_enable();
    pfa->flags |= PFA_FLAG_ENABLE;
    pfa->flags |= PFA_FLAG_SUSPEND;
    spin_unlock_irqrestore(&pfa->pm_lock, flags);

    pfa->suspend_count++;

    return 0;
}

int pfa_pm_resume(struct device *pdev)
{
    struct pfa *pfa = &g_pfa;
    unsigned long flags;
    unsigned int portno;
    unsigned int intr_interval;

    if (pfa->plat_type == PLAT_ESL) {
        pfa->plat_esl++;
        return 0;
    }

    spin_lock_irqsave(&pfa->pm_lock, flags);
    pfa->wakeup_flag = TRUE;
    pfa->halt = 0;
    intr_interval = pfa_readl(pfa->regs, PFA_INTA_INTERVAL);
    if (intr_interval == PFA_INTR_MIN_INTERVAL) {
        pfa_intr_set_interval(pfa->irq_interval);
        for (portno = 0; portno < PFA_PORT_NUM; portno++) {
            pfa->ports[portno].ctrl.td_free = pfa_readl(pfa->regs, PFA_TDQX_WPTR(portno));
            pfa->ports[portno].ctrl.td_busy = pfa_readl(pfa->regs, PFA_TDQX_RPTR(portno));
            pfa->ports[portno].ctrl.rd_free = pfa_readl(pfa->regs, PFA_RDQX_RPTR(portno));
            pfa->ports[portno].ctrl.rd_busy = pfa_readl(pfa->regs, PFA_RDQX_WPTR(portno));
        }
    }

    pfa->flags &= (~PFA_FLAG_ENABLE);
    pfa->hal->config_disable();

    // open dbgen
    pfa->hal->dbgen_en();

    // restore entrys   timestamp?
    entry_restore();

    // close dbgen
    pfa->hal->dbgen_dis();

    // open pfa_en
    pfa->hal->config_enable();

    // pfa tft
    pfa_tft_pm_resume();

    pfa->flags |= PFA_FLAG_ENABLE;
    pfa->flags &= (~PFA_FLAG_SUSPEND);
    pfa->resume_count++;

    spin_unlock_irqrestore(&pfa->pm_lock, flags);

    return 0;
}

static int pfa_bakspace_init(struct pfa *pfa)
{
    int ret = 0;
    unsigned int entry_word_num = PFA_MAC_ENTRY_NUM + PFA_IPV4_ENTRY_NUM + PFA_IPV6_ENTRY_NUM + PFA_QOS_MAC_ENTRY_NUM +
        PFA_MAC_PDU_ENTRY_NUM + PFA_MAC_REG_ENTRY_NUM;

    pfa->porten_bak = 0;
    pfa->suspend_count = 0;
    pfa->not_idle = 0;

    pfa->backup_entry_num = entry_word_num;
    pfa->entry_bak = kmalloc((sizeof(unsigned int)) * entry_word_num, GFP_ATOMIC);
    if (pfa->entry_bak == NULL) {
        ret = -ENOMEM;
    }

    return ret;
}

static void pfa_bakspace_exit(struct pfa *pfa)
{
    if (pfa->entry_bak != NULL) {
        kfree(pfa->entry_bak);
        pfa->entry_bak = NULL;
    }
}

static struct dev_pm_ops g_pfa_dev_pm_ops = {
    .prepare = pfa_pm_prepare,
    .suspend_noirq = pfa_pm_suspend,
    .resume_noirq = pfa_pm_resume,
};

static int pfa_pm_init(struct pfa *pfa)
{
    int ret;

    ret = pfa_bakspace_init(pfa);
    if (ret) {
        PFA_ERR("failed to alloc memory for backup space. \n");
        return ret;
    }
    spin_lock_init(&pfa->pm_lock);
    return 0;
}

void pfa_nv_init(struct pfa *pfa)
{
    int ret;
#ifdef FEATURE_NVA_ON
    ret = bsp_nv_read(NVA_ID_DRV_PFA_FEATURE, (u8 *)&pfa->pfa_feature, sizeof(pfa->pfa_feature));
#else
    ret = bsp_nvm_read(NV_ID_DRV_PFA_FEATURE, (u8 *)&pfa->pfa_feature, sizeof(pfa->pfa_feature));
#endif
    if (ret) {
        pfa->pfa_feature.smp_sch = 0;
        pfa->pfa_feature.smp_hp = 1;
        PFA_ERR("pfa read nv fail\n");
    } else {
        pfa->usbport.bypass_mode = 0;
        PFA_ERR("[init]pfa smp_sch      %u \n", pfa->pfa_feature.smp_sch);
        PFA_ERR("[init]pfa smp_hp       %u \n", pfa->pfa_feature.smp_hp);
    }
}

int pfa_common_info_init(struct pfa *pfa)
{
    int ret;
    pfa->pfa_version = pfa->hal->get_version();
    pfa->msg_level = PFA_MSG_ERR | PFA_MSG_TRACE;
    pfa->dbg_level = PFA_DBG_TD_RESULT | PFA_DBG_RD_RESULT;

    pfa->halt = 0;

    pfa_port_init(pfa);

    ret = pfa_ip_entry_table_init(pfa);
    if (ret) {
        PFA_ERR("pfa_entry_table_init failed, ret:%d\n", ret);
        return ret;
    }

    ret = pfa_mac_entry_table_init(pfa);
    if (ret) {
        PFA_ERR("pfa_entry_table_init failed, ret:%d\n", ret);
        return ret;
    }

    pfa_ad_dts_init(pfa);

    ret = pfa_interrupt_dts_init(pfa);
    if (ret) {
        return ret;
    }
    wan_port_config_init();
    pfa_relay_port_config_init(pfa);

    return 0;
}

int pfa_hardware_init(struct pfa *pfa)
{
    int ret;

    pfa->init = 1;

    ret = pfa_core_init(pfa);
    if (ret) {
        return ret;
    }

    pfa_hook_register();

    ret = pfa_tft_init(pfa);
    if (ret) {
        return ret;
    }

    ret = wan_init(pfa->dev);
    if (ret) {
        return ret;
    }

    pfa->hal->config_enable();

    return 0;
}


static int pfa_probe(struct platform_device *pdev)
{
    struct pfa *pfa = &g_pfa;
    int ret;

    PFA_ERR("[Init]PFA probe\n");
    of_dma_configure(&pdev->dev, NULL, true);
    dma_set_mask_and_coherent(&pdev->dev, DMA_BIT_MASK(64)); /* 64 bit mask */

    ret = memset_s(pfa, sizeof(*pfa), 0, sizeof(struct pfa));
    if (ret) {
        PFA_ERR("[Init]PFA memset_s err.\n");
    }
    pfa->dev = &pdev->dev;
    pfa->hal = &g_pfa_hal;

    /* sysctrl initialization */
    pfa_sysctrl_init(pfa);
    platform_set_drvdata(pdev, pfa);

    ret = pfa_common_info_init(pfa);
    if (ret) {
        goto pfa_probe_err_core_exit;
    }
    pfa_nv_init(pfa);
    /* register acore dpm funs */
    ret = pfa_pm_init(pfa);
    if (ret) {
        goto pfa_probe_err_core_exit;
    }

    pfa_clk_enable(pfa->dev);
    pfa_core_unreset();

    if (pfa->pfa_version == PFA_VERSION_V0 || pfa->pfa_version == PFA_VERSION_V100) {
        ret = pfa_hardware_init(pfa);
    } else {
        ret = pfa_load_init(pfa);
    }
    if (ret) {
        goto pfa_probe_err_core_exit;
    }

    PFA_ERR("[Init]PFA probe done\n");
    return 0;

pfa_probe_err_core_exit:
    pfa_core_exit(pfa);
    pfa_bakspace_exit(pfa);
    pfa_hook_unregister();
    pfa_sysctrl_exit();
    return ret;
}

static int pfa_remove(struct platform_device *pdev)
{
    struct pfa *pfa = platform_get_drvdata(pdev);
    unsigned int i;

    if (pfa == NULL) {
        PFA_ERR("no pfa exist!\n");
        return -EFAULT;
    }

    pfa->hal->config_disable();

    pfa->flags &= (~PFA_FLAG_ENABLE);

    pfa_core_exit(pfa);

    iounmap(pfa->regs);

    pfa_sysctrl_exit();

    pfa_hook_unregister();

    for (i = 0; i < pfa->clks_num; i++) {
        clk_disable_unprepare(pfa->clks[i]);
    }

    return 0;
}

struct platform_device pfa_device = {
    .name = PFA_MODULE_NAME,
    .id = -1,
    .num_resources = 0,
};

static const struct of_device_id pfa_dt_ids[] = {
    { .compatible = "pfa" },
    { /* sentinel */ }
};

static struct platform_driver pfa_driver = {
    .probe = pfa_probe,
    .remove = pfa_remove,
    .driver =
    {
        .name = PFA_MODULE_NAME,
        .owner = THIS_MODULE,
        .of_match_table = of_match_ptr(pfa_dt_ids),
        .pm = &g_pfa_dev_pm_ops,
        .probe_type = PROBE_FORCE_SYNCHRONOUS,
    },
};

int pfa_init(void)
{
    int ret;

    device_node_s *np = bsp_dt_find_compatible_node(NULL, NULL, "pfa");
    if (np == NULL) {
        return -1;
    }

    if (bsp_dt_device_is_available(np) == false) {  // 如果为false时，如果业务场景真的需要创建设备，自己创建
        ret = platform_device_register(&pfa_device);
        if (ret) {
            PFA_ERR("dev register fail.\n");
            return -1;
        }
    }

    ret = platform_driver_register(&pfa_driver);
    if (ret) {
        PFA_ERR("drv register fail.\n");
        return -1;
    }
    return 0;
}

int bsp_pfa_module_init(void)
{
    int ret;

    ret = pfa_init();
    if (ret) {
        return ret;
    }
    return pfa_dfs_init();
}

#if !IS_ENABLED(CONFIG_MODEM_DRIVER)
module_init(pfa_init);
late_initcall(pfa_dfs_init);
#endif

void pfa_exit(void)
{
    /*
     * don't need kfree platform_data in exit process,
     * platform_driver_unregister will do it.
     */
    platform_driver_unregister(&pfa_driver);

    return;
}

module_exit(pfa_exit);

MODULE_DEVICE_TABLE(of, pfa_dt_ids);
MODULE_AUTHOR("bsp4 network");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("smart packet engine(pfa) driver");

