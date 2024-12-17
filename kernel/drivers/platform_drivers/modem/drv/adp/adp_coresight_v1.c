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
#include <osl_types.h>
#include <osl_cache.h>
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/of_device.h>
#include <linux/slab.h>
#include <bsp_dt.h>
#include <bsp_om_enum.h>
#include <bsp_coresight.h>
#include <bsp_dump.h>
#include <bsp_nvim.h>
#include <bsp_print.h>
#include <securec.h>

#undef THIS_MODU
#define THIS_MODU mod_coresight

#define cs_error(fmt, ...) (void)bsp_err("<%s> line = %d, " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define cs_print(fmt, ...) (void)bsp_err("<%s>" fmt, __FUNCTION__, ##__VA_ARGS__)

#define CPU_NUMS 16 /* max num */
#define OCD_TRACE_DISABLE_VAL 0x00000000
#define OCD_TRACE_CTRL_OFFSET 0x608
#define OCD_TRACE_WAIT_US 10
#define GET_TRACE_DUMP_LEN(len) ((len) + 0x10)
#define CS_ETM_LOCK(addr)                        \
    do {                                         \
        writel(0x0, (addr) + MDM_CORESIGHT_LAR); \
    } while (0)
#define CS_ETM_UNLOCK(addr)                                       \
    do {                                                          \
        writel(MDM_CORESIGHT_UNLOCK, (addr) + MDM_CORESIGHT_LAR); \
    } while (0)
#define TIMEOUT_CNT 10000
#define TIMEOUT_PERIOD 10
#define BUF_HEADER_U32LEN 2
#define STSR_TMC_READY BIT(2)
#define FFCR_STOP_ON_FL BIT(12)
#define FFCR_STOP_ON_FL BIT(12)
#define FFCR_FLUSH_MAN BIT(6)
#define BYTES_PER_WORD 4
#define CORESIGHT_TMC_RSZ 0x004
#define CORESIGHT_TMC_STS 0x00C
#define CORESIGHT_TMC_RRD 0x010
#define CORESIGHT_TMC_FFCR 0x304
#define TMC_CTL_TRACECAPDIS 0
#define CORESIGHT_TMC_CTL 0x020
#define CORESIGHT_TMC_RRP 0x014
#define CORESIGHT_TMC_RWP 0x018
#define CORESIGHT_READL(base, off) readl((void *)((uintptr_t)(base) + (off)))
#define CORESIGHT_WRITEL(base, val, off) writel(val, (void *)((uintptr_t)(base) + (off)))
#define CORESIGHT_LAR 0xFB0
#define CORESIGHT_TMC_LOCK(base)                    \
    do {                                            \
        mb();                                       \
        CORESIGHT_WRITEL(base, 0x0, CORESIGHT_LAR); \
    } while (0)
#define CORESIGHT_TMC_UNLOCK(base)                         \
    do {                                                   \
        /* write a non zero data to unlock */              \
        CORESIGHT_WRITEL(base, 0xC5ACCE55, CORESIGHT_LAR); \
        mb();                                              \
    } while (0)
struct mdmcp_coresight_device_info {
    void *tmc_base;
    struct coresight_etb_data_head_info *etb_buf;
};
struct mdmcp_coresight_device_info g_mdmcp_coresight[CPU_NUMS];
struct reg_bank_info {
    u32 *reg_mask;
    u32 reg_offset;
    u32 reg_nums;
    u32 mask_length;
};

struct reg_group_info {
    struct reg_bank_info reg_bank[MAX_REG_BANKS];
    u32 banks;
};

struct tsp_regs_drvdata {
    u32 *tsp_reg_baseaddr;
    u32 phy_threadid[MAX_THREAD_NUM]; /* logic thread id to physic thread id */
    u32 is_powerdown[MAX_THREAD_NUM];
    u32 *thread_dump_base_addr[MAX_THREAD_NUM];
    u32 *dss_dump_base_addr[MAX_DSS_NUM];
    u32 *mpss_dump_base_addr;
    u32 thread_num;
    u32 dss_num;
    u32 thread_reg_num;
    u32 dss_reg_num;
    u32 mpss_reg_num;
    u32 init;
};
static struct of_device_id g_tsp_regs_match[REG_GROUP_NUMS] = {
    { .compatible = "driver,tsp,thread_regs" },
    { .compatible = "driver,tsp,dss_regs" },
    { .compatible = "driver,tsp,mp_regs" }
};
struct reg_group_info g_tsp_reg_group[REG_GROUP_NUMS]; // 0:thread 1:dss 2:mpss
struct tsp_regs_drvdata g_tsp_regs_drvdata;

#define DSS_BASEADDR(dss) (void *)((uintptr_t)g_tsp_regs_drvdata.tsp_reg_baseaddr + DSS_REGS_OFFSET(dss))
#define THREAD_BASEADDR(dss, thread) (void *)(DSS_BASEADDR(dss) + THREAD_REGS_OFFSET(thread))
#define OCD_BASEADDR(dss, thread) (void *)(THREAD_BASEADDR(dss, thread) + OCD_REGS_OFFSET)


static void *get_mdmcp_etb_buf(u32 cpu)
{
    void *addr = (void *)bsp_dump_get_field_addr(DUMP_TSP_LTRACE0 + cpu);
    return addr;
}
int mdmcp_coresight_wait_value(const void *base, u32 offset, u32 mask, u32 value)
{
    int i;
    u32 val;
    for (i = TIMEOUT_CNT; i > 0; i--) {
        val = CORESIGHT_READL(base, offset);
        if ((val & mask) == (value & mask)) {
            return BSP_OK;
        }
        if (i - 1) {
            udelay(TIMEOUT_PERIOD);
        }
    }
    return BSP_ERROR;
}
static void mdmcp_coresight_etb_stop(void *tmc_base)
{
    u32 reg_value;
    /* stop etb, 配置ETF_FORMAT_FLUSH_CTRL */
    reg_value = (u32)CORESIGHT_READL(tmc_base, CORESIGHT_TMC_FFCR);
    /* FFCR StopOnFl */
    reg_value |= FFCR_STOP_ON_FL;
    CORESIGHT_WRITEL(tmc_base, reg_value, CORESIGHT_TMC_FFCR);
    /* FFCR FlushMem */
    reg_value |= FFCR_FLUSH_MAN;
    CORESIGHT_WRITEL(tmc_base, reg_value, CORESIGHT_TMC_FFCR);
    /* 等待TMCReady */
    if (mdmcp_coresight_wait_value(tmc_base, CORESIGHT_TMC_STS, STSR_TMC_READY, STSR_TMC_READY)) {
        cs_error("save etb time out\n");
    }
    return;
}
void mdmcp_etarce_disable(uint32_t logic_threadid)
{
    u32 phy_threadid = g_tsp_regs_drvdata.phy_threadid[logic_threadid];
    u32 dss = phy_threadid / ARCHDEF_THREAD_PER_DSS;
    u32 thread = phy_threadid % ARCHDEF_THREAD_PER_DSS;
    writel(OCD_TRACE_DISABLE_VAL, (void *)((uintptr_t)OCD_BASEADDR(dss, thread) + OCD_TRACE_CTRL_OFFSET));
    mb();
    udelay(OCD_TRACE_WAIT_US);
}
static void mdmcp_etb_save(void *tmc_base, void *etb_buf)
{
    u32 reg_value, i, len;
    u32 *data = NULL;
    /* unlock etb, 配置ETF_LOCK_ACCESS */
    CORESIGHT_TMC_UNLOCK(tmc_base);
    mdmcp_coresight_etb_stop(tmc_base);
    len = (u32)CORESIGHT_READL(tmc_base, CORESIGHT_TMC_RSZ);
    if (len <= 0 || len > 0x40000) {
        cs_error("len 0x%x err\n", len);
        return;
    }
    /* 导出etb数据 */
    (void)memset_s((void *)(uintptr_t)etb_buf, GET_TRACE_DUMP_LEN(len), 0x0, GET_TRACE_DUMP_LEN(len));
    data = (u32 *)(uintptr_t)(etb_buf + sizeof(struct coresight_etb_data_head_info));
    for (i = 0; i < len; i++) {
        /* read etb, 读取 ETF_RAM_RD_DATA */
        reg_value = (u32)CORESIGHT_READL(tmc_base, CORESIGHT_TMC_RRD);
        if (reg_value == 0xffffffff) {
            cs_error("data %u ok\n", i);
            break;
        }
        *data = reg_value;
        data++;
    }
    /* 0-3字节存放标识码 */
    *((u32 *)(uintptr_t)etb_buf) = (u32)OCD_MAGIC_NUM;
    /* 4-7个字节存放ETB数据长度 */
    *((u32 *)(uintptr_t)etb_buf + 1) = i * BYTES_PER_WORD;
    CORESIGHT_WRITEL(tmc_base, TMC_CTL_TRACECAPDIS, CORESIGHT_TMC_CTL);
    CORESIGHT_WRITEL(tmc_base, TMC_CTL_TRACECAPDIS, CORESIGHT_TMC_RRP);
    CORESIGHT_WRITEL(tmc_base, TMC_CTL_TRACECAPDIS, CORESIGHT_TMC_RWP);
    CORESIGHT_TMC_LOCK(tmc_base);
}
static void mdmcp_etrace_store(u32 cpu)
{
    void *tmc_base = NULL;
    void *etb_buf = NULL;
    u32 phy_threadid = g_tsp_regs_drvdata.phy_threadid[cpu];
    u32 phy_dss = phy_threadid / ARCHDEF_THREAD_PER_DSS;
    int access_state;

    if (cpu >= CPU_NUMS || g_mdmcp_coresight[cpu].tmc_base == NULL) {
        return;
    }
    access_state = bsp_dump_get_modem_access_state(DUMP_MODEM_ACCESS_TSP_DSS0 + phy_dss);
    if (access_state == DUMP_MDMACCESS_OFF) {
        g_tsp_regs_drvdata.is_powerdown[cpu] = 1;
        return;
    }

    if (g_mdmcp_coresight[cpu].etb_buf != NULL) {
        etb_buf = g_mdmcp_coresight[cpu].etb_buf;
    } else {
        etb_buf = get_mdmcp_etb_buf(cpu);
        if (etb_buf == NULL) {
            cs_error("get modem cp cpu%d dump buf failed!\n", cpu);
            return;
        }
        g_mdmcp_coresight[cpu].etb_buf = etb_buf;
    }
    tmc_base = g_mdmcp_coresight[cpu].tmc_base;

    if (*(u32 *)(uintptr_t)etb_buf == OCD_MAGIC_NUM) {
        cs_error("etb %u data has store finished,no need to store again!\n", cpu);
        return;
    }
    /* 该cpu是否已经热插拔 */
    if (*(u32 *)(uintptr_t)etb_buf == OCD_HOTPLUG_MAGICNUM) {
        cs_error("cpu%u has powerdown or hotplug,no need to store data!\n", cpu);
        g_tsp_regs_drvdata.is_powerdown[cpu] = 1;
        return;
    }
    /* disable source */
    mdmcp_etarce_disable(cpu);
    mdmcp_etb_save(tmc_base, etb_buf);
    cs_error("store cpu %u ok\n", cpu);
}

static int mdmcp_coresight_tmc_probe(device_node_s *dev_node)
{
    u32 cpu_index = 0;
    int ret;

    ret = bsp_dt_property_read_u32(dev_node, "cpu_index", &cpu_index);
    if (ret) {
        cs_error("read cpu_index failed,use default value! ret = %d\n", ret);
        cpu_index = 0;
    }
    if (g_mdmcp_coresight[cpu_index].tmc_base == NULL) {
        g_mdmcp_coresight[cpu_index].tmc_base = (void *)bsp_dt_iomap(dev_node, 0);
    } else {
        cs_error("cpu %d have init before,cpu index may be wrong!\n", cpu_index);
    }
    if (g_mdmcp_coresight[cpu_index].tmc_base == NULL) {
        return BSP_ERROR;
    }
    g_mdmcp_coresight[cpu_index].etb_buf = get_mdmcp_etb_buf(cpu_index);
    return 0;
}
static int mdmcp_get_reg_buffer(void)
{
    u32 size;
    u32 i;
    u8 *addr;

    /* register dump area for thread regs */
    size = g_tsp_regs_drvdata.thread_reg_num * sizeof(u32) + 0x4;
    addr = bsp_dump_get_field_addr(DUMP_TSP_THREAD_REGS);
    if (addr == NULL) {
        cs_error("get thread dump area fail\n");
        return BSP_ERROR;
    }
    for (i = 0; i < g_tsp_regs_drvdata.thread_num; i++) {
        g_tsp_regs_drvdata.thread_dump_base_addr[i] = (u32 *)((uintptr_t)addr + (uintptr_t)(size * i));
    }

    /* register dump area for dss regs */
    size = g_tsp_regs_drvdata.dss_reg_num * sizeof(u32) + 0x4;
    addr = bsp_dump_get_field_addr(DUMP_TSP_DSS_REGS);
    if (addr == NULL) {
        cs_error("get dss dump area fail\n");
        return BSP_ERROR;
    }
    for (i = 0; i < g_tsp_regs_drvdata.dss_num; i++) {
        g_tsp_regs_drvdata.dss_dump_base_addr[i] = (u32 *)((uintptr_t)addr + (uintptr_t)(size * i));
    }
    /* register dump area for mpss regs */
    size = g_tsp_regs_drvdata.mpss_reg_num * sizeof(u32) + 0x4;
    addr = bsp_dump_get_field_addr(DUMP_TSP_MPSS_REGS);
    if (addr == NULL) {
        cs_error("get mpss dump area fail\n");
        return BSP_ERROR;
    }
    g_tsp_regs_drvdata.mpss_dump_base_addr = (u32 *)addr;
    return BSP_OK;
}
static inline u32 mdmcp_get_mask_bit(const u32 *reg_mask, u32 pos)
{
    u32 bank = pos / BITS_U32;
    u32 offset = pos & (BITS_U32 - 1);
    u32 ret = reg_mask[bank] & (1 << offset);
    return !!ret;
}

void mdmcp_save_tsp_regs_core(u32 *baseaddr, u32 *dump_addr, struct reg_group_info *regs)
{
    u32 i, n_bit;
    int pos = 1;
    u32 bits;
    u32 *reg_addr = NULL;
    struct reg_bank_info *reg_bank = NULL;
    for (i = 0; i < regs->banks; i++) {
        reg_bank = &regs->reg_bank[i];
        bits = reg_bank->mask_length * BITS_U32;
        reg_addr = (u32 *)((uintptr_t)baseaddr + reg_bank->reg_offset);
        for (n_bit = 0; n_bit < bits; n_bit++) {
            if (mdmcp_get_mask_bit(reg_bank->reg_mask, n_bit)) {
                dump_addr[pos] = (u32)readl((const void *)(uintptr_t)reg_addr);
                pos++;
            }
            reg_addr++;
        }
    }
}

void mdmcp_save_current_thread_regs(u32 logic_threadid)
{
    u32 phy_threadid = g_tsp_regs_drvdata.phy_threadid[logic_threadid];
    u32 dss = phy_threadid / ARCHDEF_THREAD_PER_DSS;
    u32 thread = phy_threadid % ARCHDEF_THREAD_PER_DSS;
    u32 *dump_addr = g_tsp_regs_drvdata.thread_dump_base_addr[logic_threadid];
    if (dump_addr == NULL || dump_addr[0] == DONE_MAGICNUM) {
        return;
    }
    dump_addr[0] = DONE_MAGICNUM;

    mdmcp_save_tsp_regs_core(THREAD_BASEADDR(dss, thread), dump_addr, &g_tsp_reg_group[REG_GROUP_THREAD]);
}

void mdmcp_save_current_dss_regs(u32 logic_threadid)
{
    u32 phy_threadid = g_tsp_regs_drvdata.phy_threadid[logic_threadid];
    u32 phy_dss = phy_threadid / ARCHDEF_THREAD_PER_DSS;
    u32 *dump_addr = g_tsp_regs_drvdata.dss_dump_base_addr[phy_dss];

    if (dump_addr == NULL || dump_addr[0] == DONE_MAGICNUM) {
        return;
    }

    dump_addr[0] = DONE_MAGICNUM;

    mdmcp_save_tsp_regs_core(DSS_BASEADDR(phy_dss), dump_addr, &g_tsp_reg_group[REG_GROUP_DSS]);
}

void mdmcp_save_current_mpss_regs(void)
{
    u32 *dump_addr = g_tsp_regs_drvdata.mpss_dump_base_addr;

    if (dump_addr == NULL || dump_addr[0] == DONE_MAGICNUM) {
        return;
    }

    dump_addr[0] = DONE_MAGICNUM;

    mdmcp_save_tsp_regs_core(g_tsp_regs_drvdata.tsp_reg_baseaddr, dump_addr, &g_tsp_reg_group[REG_GROUP_MPSS]);
}

void mdmcp_tsp_regs_store(u32 logic_threadid)
{
    int access_state;
    u32 phy_threadid = g_tsp_regs_drvdata.phy_threadid[logic_threadid];
    u32 phy_dss = phy_threadid / ARCHDEF_THREAD_PER_DSS;
    if (mdmcp_get_reg_buffer() != BSP_OK) {
        return;
    }
    access_state = bsp_dump_get_modem_access_state(DUMP_MODEM_ACCESS_TSP_DSS0 + phy_dss);
    if (access_state == DUMP_MDMACCESS_ON) {
        goto save_all_regs;
    }
    access_state = bsp_dump_get_modem_access_state(DUMP_MODEM_ACCESS_TSP_MPSS);
    if (access_state == DUMP_MDMACCESS_ON) {
        goto save_mpss_regs;
    }
    if(access_state == DUMP_MDMACCESS_OFF) {
        return;
    }
    /* modem_access_state接口不支持，此时通过自身标记判断 */
    if (g_tsp_regs_drvdata.is_powerdown[logic_threadid]) {
        return;
    }
save_all_regs:
    mdmcp_save_current_thread_regs(logic_threadid);
    mdmcp_save_current_dss_regs(logic_threadid);
save_mpss_regs:
    mdmcp_save_current_mpss_regs();
    return;
}

static void mdmcp_free_reg_group_info(void)
{
    u32 i, j;
    for (i = 0; i < REG_GROUP_NUMS; i++) {
        for (j = 0; j < MAX_REG_BANKS; j++) {
            if (g_tsp_reg_group[i].reg_bank[j].reg_mask != NULL) {
                kfree(g_tsp_reg_group[i].reg_bank[j].reg_mask);
                g_tsp_reg_group[i].reg_bank[j].reg_mask = NULL;
            }
        }
    }
}

static int mdmcp_reginfo_probe(device_node_s *dev_node, int group)
{
    device_node_s *child_node = NULL;
    struct reg_bank_info *bank = NULL;
    u32 reg_banks = 0;
    u32 i = 0;
    int ret;

    if (bsp_dt_property_read_u32_array(dev_node, "reg_banks", &reg_banks, 1) != BSP_OK || reg_banks > MAX_REG_BANKS) {
        mdmcp_free_reg_group_info();
        return BSP_ERROR;
    }
    bsp_dt_for_each_child_of_node(dev_node, child_node)
    {
        if (i >= reg_banks) {
            break;
        }
        bank = &g_tsp_reg_group[group].reg_bank[i];
        ret = bsp_dt_property_read_u32_array(child_node, "reg_offset", &bank->reg_offset, 1);
        if (ret != BSP_OK) {
            mdmcp_free_reg_group_info();
            return BSP_ERROR;
        }
        ret = bsp_dt_property_read_u32_array(child_node, "array_length", &bank->mask_length, 1);
        if (ret != BSP_OK || bank->mask_length > MAX_REG_MASK_LENGTH) {
            mdmcp_free_reg_group_info();
            return BSP_ERROR;
        }
        ret = bsp_dt_property_read_u32_array(child_node, "reg_nums", &bank->reg_nums, 1);
        if (ret != BSP_OK || bank->reg_nums > MAX_REG_NUM || bank->reg_nums > bank->mask_length * BITS_U32) {
            mdmcp_free_reg_group_info();
            return BSP_ERROR;
        }

        bank->reg_mask = (u32 *)kmalloc(sizeof(u32) * bank->mask_length, GFP_KERNEL);
        if (bank->reg_mask == NULL) {
            mdmcp_free_reg_group_info();
            return BSP_ERROR;
        }
        ret = bsp_dt_property_read_u32_array(child_node, "reg_mask", bank->reg_mask, bank->mask_length);
        if (ret != BSP_OK) {
            mdmcp_free_reg_group_info();
            return BSP_ERROR;
        }

        i++;
    }
    g_tsp_reg_group[group].banks = i;
    return BSP_OK;
}

static int mdmcp_baseinfo_probe(device_node_s *dev_node)
{
    int ret;
    int i, j;
    u32 reg_nums[REG_GROUP_NUMS];

    ret = bsp_dt_property_read_u32_array(dev_node, "dss_num", &g_tsp_regs_drvdata.dss_num, 1);
    if (ret != BSP_OK || g_tsp_regs_drvdata.dss_num > MAX_DSS_NUM) {
        cs_error("dss_num not found/invalid\n");
        return BSP_ERROR;
    }
    ret = bsp_dt_property_read_u32_array(dev_node, "thread_num", &g_tsp_regs_drvdata.thread_num, 1);
    if (ret != BSP_OK || g_tsp_regs_drvdata.thread_num > MAX_THREAD_NUM) {
        cs_error("thread_num not found/invalid\n");
        return BSP_ERROR;
    }
    ret = bsp_dt_property_read_u32_array(dev_node, "phy_threadid", g_tsp_regs_drvdata.phy_threadid,
        g_tsp_regs_drvdata.thread_num);
    if (ret != BSP_OK) {
        cs_error("phy_threadid not found/invalid\n");
        return BSP_ERROR;
    }

    g_tsp_regs_drvdata.tsp_reg_baseaddr = bsp_dt_iomap(dev_node, 0);
    if (g_tsp_regs_drvdata.tsp_reg_baseaddr == NULL) {
        cs_error("tsp_reg_baseaddr not found/invalid\n");
        return BSP_ERROR;
    }

    for (i = 0; i < REG_GROUP_NUMS; i++) {
        reg_nums[i] = 0;
        for (j = 0; j < g_tsp_reg_group[i].banks; j++) {
            reg_nums[i] += g_tsp_reg_group[i].reg_bank[j].reg_nums;
        }
    }
    g_tsp_regs_drvdata.thread_reg_num = reg_nums[REG_GROUP_THREAD];
    g_tsp_regs_drvdata.dss_reg_num = reg_nums[REG_GROUP_DSS];
    g_tsp_regs_drvdata.mpss_reg_num = reg_nums[REG_GROUP_MPSS];

    return BSP_OK;
}

int mdmcp_coresight_init(void)
{
    device_node_s *dev_node = NULL;
    device_node_s *child_node = NULL;
    int i;
    int ret;
    /* parse tmc node */
    dev_node = bsp_dt_find_node_by_path("/coresight-etm-cp");
    if (dev_node == NULL) {
        cs_error("can not find coresight-etm-cp node!\n");
        return BSP_ERROR;
    }
    bsp_dt_for_each_child_of_node(dev_node, child_node)
    {
        mdmcp_coresight_tmc_probe(child_node);
    }
    dev_node = bsp_dt_find_compatible_node(NULL, NULL, "driver,dump_tsp_regs");
    if (dev_node == NULL) {
        cs_error("find node fail\n");
        return BSP_OK;
    }

    for (i = 0; i < REG_GROUP_NUMS; i++) {
        child_node = bsp_dt_find_compatible_node(dev_node, NULL, g_tsp_regs_match[i].compatible);
        if (child_node == NULL) {
            cs_error("find node %s fail\n", g_tsp_regs_match[i].compatible);
            return BSP_OK;
        }
        ret = mdmcp_reginfo_probe(child_node, i);
        if (ret != BSP_OK) {
            return ret;
        }
    }

    ret = mdmcp_baseinfo_probe(dev_node);
    if (ret != BSP_OK) {
        cs_error("get baseinfo error!");
        mdmcp_free_reg_group_info();
        return BSP_ERROR;
    }
    g_tsp_regs_drvdata.init = 1;

    cs_print("ok\n");
    return BSP_OK;
}
int mdmcp_coresight_stop(void)
{
    u32 cpu;

    if (!g_tsp_regs_drvdata.init) {
        cs_error("not inited\n");
        return 0;
    }
    for (cpu = 0; cpu < g_tsp_regs_drvdata.thread_num; cpu++) {
        g_tsp_regs_drvdata.is_powerdown[cpu] = 0;
        mdmcp_etrace_store((u32)cpu);
        mdmcp_tsp_regs_store((u32)cpu);
    }

    return 0;
}
#if !IS_ENABLED(CONFIG_MODEM_DRIVER)
late_initcall(mdmcp_coresight_init);
#endif

int bsp_coresight_stop_cp(void)
{
    return mdmcp_coresight_stop();
}