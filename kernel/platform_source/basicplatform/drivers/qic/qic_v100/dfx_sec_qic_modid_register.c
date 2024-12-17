/*
 * QIC v100 modid register functions source file.
 *
 * Copyright (c) 2023-2023 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/limits.h>
#include <linux/list.h>
#include <linux/spinlock_types.h>
#include <linux/irqchip/arm-gic-v3.h>
#include "dfx_sec_qic_err_probe.h"
#include "dfx_sec_qic_modid_register.h"
#include "mntn_public_interface.h"
#include "platform_include/smart/linux/iomcu_dump.h"

static LIST_HEAD(__qic_modid_list);
static DEFINE_SPINLOCK(__qic_modid_list_lock);
static DEFINE_SPINLOCK(__qic_errlog_lock);
static volatile s32 mod_cnt = 0;
static struct dfx_errlog_info g_errlog_info;

static int dfx_qic_cnt_check(int mod_cnt)
{
	if (mod_cnt > QIC_REGISTER_LIST_MAX_LENGTH)
		return -1;
	else
		return 0;
}
static unsigned int dfx_qic_check_para_registerd(unsigned int qic_coreid, unsigned int modid)
{
	struct qic_coreid_modid_trans_s *p_modid_e = NULL;
	struct list_head *cur = NULL;
	struct list_head *next = NULL;
	unsigned long lock_flag;

	spin_lock_irqsave(&__qic_modid_list_lock, lock_flag);

	list_for_each_safe(cur, next, &__qic_modid_list) {
		p_modid_e = list_entry(cur, struct qic_coreid_modid_trans_s, s_list);
		if (!p_modid_e) {
			pr_err("It might be better to look around here. %s:%d\n", __func__, __LINE__);
			continue;
		}

		if ((p_modid_e->qic_coreid == qic_coreid) ||
			(p_modid_e->modid == modid)) {
			spin_unlock_irqrestore(&__qic_modid_list_lock, lock_flag);
			return QIC_MODID_EXIST;
		}
	}

	spin_unlock_irqrestore(&__qic_modid_list_lock, lock_flag);

	return QIC_MODID_NOT_EXIST;
}

static void __qic_modid_register(struct qic_coreid_modid_trans_s *node)
{
	unsigned long lock_flag;

	spin_lock_irqsave(&__qic_modid_list_lock, lock_flag);
	mod_cnt++;
	list_add_tail(&(node->s_list), &__qic_modid_list);
	spin_unlock_irqrestore(&__qic_modid_list_lock, lock_flag);
}

/*
 * qic modid registe API,for use to registe own process, and before this,
 * qic err use the AP_S_NOC process, after adapt,user can define his own process.
 */
void dfx_qic_modid_register(unsigned int qic_coreid, unsigned int modid)
{
	struct qic_coreid_modid_trans_s *node = NULL;
	int  pret;
	unsigned int ret;

	pret = dfx_qic_cnt_check(mod_cnt);
	if (pret) {
		pr_err("%s node is full!\n", __func__);
		return;
	}

	if (qic_coreid >= QIC_CORE_ID_MAX) {
		pr_err("%s qic coreid is not exist\n", __func__);
		return;
	}

	/*
	 * before registering modid,we have to check that has modid been registered
	 * berore,double check.
	 */
	ret = dfx_qic_check_para_registerd(qic_coreid, modid);
	if (ret == QIC_MODID_EXIST) {
		pr_err("%s node is exist!\n", __func__);
		return;
	}

	node = kzalloc(sizeof(struct qic_coreid_modid_trans_s), GFP_ATOMIC);
	if (node == NULL)
		return;

	node->qic_coreid = qic_coreid;
	node->modid = modid;
	pr_info("[%s]: modid register qic_coreid = 0x%x !\n", __func__, node->qic_coreid);
	pr_info("[%s]: modid register modid = 0x%x !\n", __func__, node->modid);
	pr_info("[%s]: modid register node is ok !\n", __func__);

	/*
	 *  this func is the real func to registe the user's modid and
	 *  user's err judge
	 */
	__qic_modid_register(node);
}

static unsigned int __qic_find_modid(unsigned int qic_coreid)
{
	struct qic_coreid_modid_trans_s *p_modid_e = NULL;
	struct list_head *cur = NULL;
	struct list_head *next = NULL;
	unsigned long lock_flag;
	unsigned int ret = QIC_MODID_NEGATIVE;

	spin_lock_irqsave(&__qic_modid_list_lock, lock_flag);

	list_for_each_safe(cur, next, &__qic_modid_list) {
		p_modid_e = list_entry(cur, struct qic_coreid_modid_trans_s, s_list);
		if (qic_coreid == p_modid_e->qic_coreid) {
			pr_err("[%s]: modid register coreid is equal,value = 0x%x !\n", __func__, qic_coreid);
			ret = p_modid_e->modid;
		}
	}
	spin_unlock_irqrestore(&__qic_modid_list_lock, lock_flag);

	return ret;
}

static unsigned int dfx_qic_modid_refind_by_mid(unsigned int coreid, unsigned int mid)
{
	unsigned int ret_modid = QIC_MODID_NEGATIVE;

	switch (coreid) {
	case QIC_NPU_AICORE0:
		if(mid == SOC_NPU_TINY_CORE0_MID)
			ret_modid = SENSORHUB_TINY_MODID; /* SENSORHUB_TINY_MODID */
		if(mid == SOC_NPU_LITE_CORE0_MID)
			ret_modid = RDR_EXC_TYPE_NOC_NPU2; /* RDR_EXC_TYPE_NOC_NPU2 */
		break;
	case QIC_NPU_TS_0:
		if(mid == SOC_NPU_HWTS_TINY_MID)
			ret_modid = SENSORHUB_TINY_MODID; /* SENSORHUB_TINY_MODID */
		if(mid == SOC_NPU_HWTS_LITE_MID)
			ret_modid = RDR_EXC_TYPE_NOC_NPU5; /* RDR_EXC_TYPE_NOC_NPU5 */
		break;
	case QIC_NPU_TS_1:
		if(mid == SOC_NPU_HWTS_TINY_MID)
			ret_modid = SENSORHUB_TINY_MODID; /* SENSORHUB_TINY_MODID */
		if(mid == SOC_NPU_HWTS_LITE_MID)
			ret_modid = RDR_EXC_TYPE_NOC_NPU6; /* RDR_EXC_TYPE_NOC_NPU6 */
		break;
	default:
		ret_modid = QIC_MODID_NEGATIVE;
		break;
	}
	return ret_modid;
}

unsigned int dfx_qic_modid_find(unsigned int qic_coreid, unsigned int mid)
{
	unsigned int ret = QIC_MODID_NEGATIVE;
	unsigned int tmp_modid;

	if (qic_coreid >= QIC_CORE_ID_MAX)
		return ret;

	ret = __qic_find_modid(qic_coreid);
	if (ret != QIC_MODID_NEGATIVE) {
		tmp_modid = dfx_qic_modid_refind_by_mid(qic_coreid, mid);
		if (tmp_modid != QIC_MODID_NEGATIVE)
			ret = tmp_modid;
	}
	return ret;
}

void dfx_qic_set_errlog_info(struct dfx_errlog_info *errlog_info)
{
	unsigned long lock_flag;

	if (!errlog_info)
		return;
	spin_lock_irqsave(&__qic_errlog_lock, lock_flag);
	g_errlog_info.err_addr  = errlog_info->err_addr;
	g_errlog_info.master_id = errlog_info->master_id;
	g_errlog_info.opc_type  = errlog_info->opc_type;
	g_errlog_info.safe_flag = errlog_info->safe_flag;
	spin_unlock_irqrestore(&__qic_errlog_lock, lock_flag);
}

void dfx_qic_get_errlog_info(struct dfx_errlog_info *errlog_info)
{
	unsigned long lock_flag;

	if (!errlog_info)
		return;
	spin_lock_irqsave(&__qic_errlog_lock, lock_flag);
	errlog_info->err_addr  = g_errlog_info.err_addr;
	errlog_info->master_id = g_errlog_info.master_id;
	errlog_info->opc_type  = g_errlog_info.opc_type;
	errlog_info->safe_flag = g_errlog_info.safe_flag;
	spin_unlock_irqrestore(&__qic_errlog_lock, lock_flag);
}

void dfx_qic_mdm_modid_register(unsigned int modid)
{
	dfx_qic_modid_register(QIC_MDM, modid);
}
EXPORT_SYMBOL(dfx_qic_mdm_modid_register);

