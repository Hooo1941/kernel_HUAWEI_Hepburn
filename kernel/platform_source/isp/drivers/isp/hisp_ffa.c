/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description: hisp ffa message
 * Create: 2023/1/19
 */

#include <linux/module.h>
#include <linux/arm_ffa.h>
#include <hisp_internel.h>
#include <platform_include/see/ffa/ffa_msg_id.h>

#define FUNC_ARGS_NO_USE                (0)
#define FUNC_EXEC_SUCC                  (0)
#define FUNC_EXEC_FAIL                  (-1)

const struct ffa_ops *g_ffa_isp_ops = NULL;
struct ffa_device *g_ffa_isp_dev = NULL;

const struct ffa_device_id hisp_ffa_device_id[] = {
	{0x41, 0x93, 0xeb, 0xb3, 0x4d, 0x49, 0x02, 0xaf,
	 0x49, 0x2b, 0xe5, 0xa1, 0x29, 0x9f, 0x9e, 0x3d},
	{}
};

static void hisp_ffa_remove(struct ffa_device *ffa_dev)
{
	(void)ffa_dev;
	pr_info("FFA_ISP: device remove\n");
}

static int hisp_ffa_probe(struct ffa_device *ffa_dev)
{
	if (ffa_dev == NULL) {
		pr_err("FFA_ISP: ffa_dev is NULL!\n");
		return -EINVAL;
	}

	g_ffa_isp_ops = ffa_dev->ops;
	g_ffa_isp_dev = ffa_dev;
	if (!g_ffa_isp_ops) {
		pr_err("FFA_ISP: failed to get ffa_ops!\n");
		return -ENOENT;
	}

	g_ffa_isp_ops->mode_32bit_set(ffa_dev);
	return FUNC_EXEC_SUCC;
}

static struct ffa_driver hisp_ffa_driver = {
	.name = "FFA_ISP",
	.probe = hisp_ffa_probe,
	.remove = hisp_ffa_remove,
	.id_table = hisp_ffa_device_id,
};

static int hisp_ffa_init(void)
{
	pr_info("FFA_ISP init\n");
	return ffa_register(&hisp_ffa_driver);
}

int hisp_ffa_disreset_ispcpu(unsigned int canary, unsigned int dynboot)
{
	int ret;
	struct ffa_send_direct_data args;

	args.data0 = ISP_FN_DISRESET_ISPCPU;
	args.data1 = canary;
	args.data2 = dynboot;
	if (dynboot == 0x0)
		args.data3 = FUNC_ARGS_NO_USE;
	else
		args.data3 = ISP_DYNBOOT_ENABLE;
	args.data4 = FUNC_ARGS_NO_USE;

	ret = g_ffa_isp_ops->sync_send_receive(g_ffa_isp_dev, &args);
	if (ret != 0) {
		pr_err("Failed FFA_ISP_IPP: %s failed to send_msg!\n", __func__);
		return ret;
	}

	if (args.data0 != ISP_FN_DISRESET_ISPCPU || args.data1 != canary ||
		args.data2 != dynboot) {
		pr_err("Failed FFA_ISP_IPP: %s direct msg receive data error!\n", __func__);
		return FUNC_EXEC_FAIL;
	}

	if (args.data4 != FUNC_EXEC_SUCC) {
		pr_err("Failed FFA_ISP_IPP: %s tzsp func fail errcode[0x%llx]!\n", __func__, args.data4);
		return FUNC_EXEC_FAIL;
	}

	return FUNC_EXEC_SUCC;
}

int hisp_ffa_ispmmu_init(u64 pgd_base)
{
	int ret;
	u64 page_addr;
	struct ffa_send_direct_data args;

	page_addr = pgd_base;

	args.data0 = ISP_FN_ISPSMMU_NS_INIT;
	args.data1 = page_addr;
	args.data2 = FUNC_ARGS_NO_USE;
	args.data3 = FUNC_ARGS_NO_USE;
	args.data4 = FUNC_ARGS_NO_USE;

	ret = g_ffa_isp_ops->sync_send_receive(g_ffa_isp_dev, &args);
	if (ret != 0) {
		pr_err("Failed FFA_ISP_IPP: %s failed to send_msg!\n", __func__);
		return ret;
	}

	if (args.data0 != ISP_FN_ISPSMMU_NS_INIT || args.data1 != page_addr) {
		pr_err("Failed FFA_ISP_IPP: %s direct msg receive data error!\n", __func__);
		return FUNC_EXEC_FAIL;
	}

	if (args.data4 != FUNC_EXEC_SUCC) {
		pr_err("Failed FFA_ISP_IPP: %s tzsp func fail errcode[0x%llx]!\n", __func__, args.data4);
		return FUNC_EXEC_FAIL;
	}

	return FUNC_EXEC_SUCC;
}

#ifdef DEBUG_HISP
int hisp_ffa_media2_power_up(void)
{
	int ret;
	struct ffa_send_direct_data args;

	args.data0 = ISP_FN_MEDIA2_VBUS_PU;
	args.data1 = FUNC_ARGS_NO_USE;
	args.data2 = FUNC_ARGS_NO_USE;
	args.data3 = FUNC_ARGS_NO_USE;
	args.data4 = FUNC_ARGS_NO_USE;

	ret = g_ffa_isp_ops->sync_send_receive(g_ffa_isp_dev, &args);
	if (ret != 0) {
		pr_err("Failed FFA_ISP_IPP: %s failed to send_msg!\n", __func__);
		return ret;
	}

	if (args.data0 != ISP_FN_MEDIA2_VBUS_PU) {
		pr_err("Failed FFA_ISP_IPP: %s direct msg receive data error!\n", __func__);
		return FUNC_EXEC_FAIL;
	}

	if (args.data4 != FUNC_EXEC_SUCC) {
		pr_err("Failed FFA_ISP_IPP: %s tzsp func fail errcode[0x%llx]!\n", __func__, args.data4);
		return FUNC_EXEC_FAIL;
	}

	return FUNC_EXEC_SUCC;
}

int hisp_ffa_media2_power_down(void)
{
	int ret;
	struct ffa_send_direct_data args;

	args.data0 = ISP_FN_MEDIA2_VBUS_PD;
	args.data1 = FUNC_ARGS_NO_USE;
	args.data2 = FUNC_ARGS_NO_USE;
	args.data3 = FUNC_ARGS_NO_USE;
	args.data4 = FUNC_ARGS_NO_USE;

	ret = g_ffa_isp_ops->sync_send_receive(g_ffa_isp_dev, &args);
	if (ret != 0) {
		pr_err("Failed FFA_ISP_IPP: %s failed to send_msg!\n", __func__);
		return ret;
	}

	if (args.data0 != ISP_FN_MEDIA2_VBUS_PD) {
		pr_err("Failed FFA_ISP_IPP: %s direct msg receive data error!\n", __func__);
		return FUNC_EXEC_FAIL;
	}

	if (args.data4 != FUNC_EXEC_SUCC) {
		pr_err("Failed FFA_ISP_IPP: %s tzsp func fail errcode[0x%llx]!\n", __func__, args.data4);
		return FUNC_EXEC_FAIL;
	}

	return FUNC_EXEC_SUCC;
}
#endif

int hisp_ffa_powerup_isptop(void)
{
	int ret;
	struct ffa_send_direct_data args;

	args.data0 = ISP_FN_ISPTOP_PU;
	args.data1 = FUNC_ARGS_NO_USE;
	args.data2 = FUNC_ARGS_NO_USE;
	args.data3 = FUNC_ARGS_NO_USE;
	args.data4 = FUNC_ARGS_NO_USE;

	ret = g_ffa_isp_ops->sync_send_receive(g_ffa_isp_dev, &args);
	if (ret != 0) {
		pr_err("Failed FFA_ISP_IPP: %s failed to send_msg!\n", __func__);
		return ret;
	}

	if (args.data0 != ISP_FN_ISPTOP_PU) {
		pr_err("Failed FFA_ISP_IPP: %s direct msg receive data error!\n", __func__);
		return FUNC_EXEC_FAIL;
	}

	if (args.data4 != FUNC_EXEC_SUCC) {
		pr_err("Failed FFA_ISP_IPP: %s tzsp func fail errcode[0x%llx]!\n", __func__, args.data4);
		return FUNC_EXEC_FAIL;
	}

	return FUNC_EXEC_SUCC;
}

int hisp_ffa_powerdn_isptop(void)
{
	int ret;
	struct ffa_send_direct_data args;

	args.data0 = ISP_FN_ISPTOP_PD;
	args.data1 = FUNC_ARGS_NO_USE;
	args.data2 = FUNC_ARGS_NO_USE;
	args.data3 = FUNC_ARGS_NO_USE;
	args.data4 = FUNC_ARGS_NO_USE;

	ret = g_ffa_isp_ops->sync_send_receive(g_ffa_isp_dev, &args);
	if (ret != 0) {
		pr_err("Failed FFA_ISP_IPP: %s failed to send_msg!\n", __func__);
		return ret;
	}

	if (args.data0 != ISP_FN_ISPTOP_PD) {
		pr_err("Failed FFA_ISP_IPP: %s direct msg receive data error!\n", __func__);
		return FUNC_EXEC_FAIL;
	}

	if (args.data4 != FUNC_EXEC_SUCC) {
		pr_err("Failed FFA_ISP_IPP: %s tzsp func fail errcode[0x%llx]!\n", __func__, args.data4);
		return FUNC_EXEC_FAIL;
	}

	return FUNC_EXEC_SUCC;
}

int hisp_ffa_connect_phy_csi(u32 phy_id, u32 phy_work_mode, u32 csi_idx)
{
	int ret;
	u64 phy_info, csi_info;
	struct ffa_send_direct_data args;

	phy_info = phy_work_mode;
	phy_info <<= 16;
	phy_info += (phy_id & 0x0000FFFF);
	csi_info = (csi_idx & 0x0000FFFF);

	args.data0 = ISP_PHY_CSI_CONNECT;
	args.data1 = phy_info;
	args.data2 = csi_info;
	args.data3 = FUNC_ARGS_NO_USE;
	args.data4 = FUNC_ARGS_NO_USE;

	ret = g_ffa_isp_ops->sync_send_receive(g_ffa_isp_dev, &args);
	if (ret != 0) {
		pr_err("Failed FFA_ISP_IPP: %s failed to send_msg!\n", __func__);
		return ret;
	}

	if (args.data0 != ISP_PHY_CSI_CONNECT || args.data1 != phy_info || args.data2 != csi_info) {
		pr_err("Failed FFA_ISP_IPP: %s direct msg receive data error!\n", __func__);
		return FUNC_EXEC_FAIL;
	}

	if (args.data4 != FUNC_EXEC_SUCC) {
		pr_err("Failed FFA_ISP_IPP: %s tzsp func fail errcode[0x%llx]!\n", __func__, args.data4);
		return FUNC_EXEC_FAIL;
	}

	return FUNC_EXEC_SUCC;
}

int hisp_ffa_disconnect_phy_csi(void)
{
	int ret;
	struct ffa_send_direct_data args;

	args.data0 = ISP_PHY_CSI_DISCONNECT;
	args.data1 = FUNC_ARGS_NO_USE;
	args.data2 = FUNC_ARGS_NO_USE;
	args.data3 = FUNC_ARGS_NO_USE;
	args.data4 = FUNC_ARGS_NO_USE;

	ret = g_ffa_isp_ops->sync_send_receive(g_ffa_isp_dev, &args);
	if (ret != 0) {
		pr_err("Failed FFA_ISP_IPP: %s failed to send_msg!\n", __func__);
		return ret;
	}

	if (args.data0 != ISP_PHY_CSI_DISCONNECT) {
		pr_err("Failed FFA_ISP_IPP: %s direct msg receive data error!\n", __func__);
		return FUNC_EXEC_FAIL;
	}

	if (args.data4 != FUNC_EXEC_SUCC) {
		pr_err("Failed FFA_ISP_IPP: %s tzsp func fail errcode[0x%llx]!\n", __func__, args.data4);
		return FUNC_EXEC_FAIL;
	}

	return FUNC_EXEC_SUCC;
}

int hisp_ffa_send_fiq(void)
{
	int ret;
	struct ffa_send_direct_data args;

	args.data0 = ISP_FN_SEND_FIQ_TO_ISPCPU;
	args.data1 = FUNC_ARGS_NO_USE;
	args.data2 = FUNC_ARGS_NO_USE;
	args.data3 = FUNC_ARGS_NO_USE;
	args.data4 = FUNC_ARGS_NO_USE;

	ret = g_ffa_isp_ops->sync_send_receive(g_ffa_isp_dev, &args);
	if (ret != 0) {
		pr_err("Failed FFA_ISP_IPP: %s failed to send_msg!\n", __func__);
		return ret;
	}

	if (args.data0 != ISP_FN_SEND_FIQ_TO_ISPCPU) {
		pr_err("Failed FFA_ISP_IPP: %s direct msg receive data error!\n", __func__);
		return FUNC_EXEC_FAIL;
	}

	if (args.data4 != FUNC_EXEC_SUCC) {
		pr_err("Failed FFA_ISP_IPP: %s tzsp func fail errcode[0x%llx]!\n", __func__, args.data4);
		return FUNC_EXEC_FAIL;
	}

	return FUNC_EXEC_SUCC;
}

int hisp_ffa_cfg_qos(void)
{
	int ret;
	struct ffa_send_direct_data args;

	args.data0 = ISP_FN_QOS_CFG;
	args.data1 = FUNC_ARGS_NO_USE;
	args.data2 = FUNC_ARGS_NO_USE;
	args.data3 = FUNC_ARGS_NO_USE;
	args.data4 = FUNC_ARGS_NO_USE;

	ret = g_ffa_isp_ops->sync_send_receive(g_ffa_isp_dev, &args);
	if (ret != 0) {
		pr_err("Failed FFA_ISP_IPP: %s failed to send_msg!\n", __func__);
		return ret;
	}

	if (args.data0 != ISP_FN_QOS_CFG) {
		pr_err("Failed FFA_ISP_IPP: %s direct msg receive data error!\n", __func__);
		return FUNC_EXEC_FAIL;
	}

	if (args.data4 != FUNC_EXEC_SUCC) {
		pr_err("Failed FFA_ISP_IPP: %s tzsp func fail errcode[0x%llx]!\n", __func__, args.data4);
		return FUNC_EXEC_FAIL;
	}

	return FUNC_EXEC_SUCC;
}

void hisp_ffa_isp_ecc(void)
{
	int ret;
	struct ffa_send_direct_data args;

	args.data0 = ISP_FN_CFG_ECC;
	args.data1 = FUNC_ARGS_NO_USE;
	args.data2 = FUNC_ARGS_NO_USE;
	args.data3 = FUNC_ARGS_NO_USE;
	args.data4 = FUNC_ARGS_NO_USE;

	ret = g_ffa_isp_ops->sync_send_receive(g_ffa_isp_dev, &args);
	if (ret != 0) {
		pr_err("%s :failed to send ecc msg\n", __func__);
		return;
	}

	if (args.data0 != ISP_FN_CFG_ECC) {
		pr_err("%s :direct msg receive data error\n", __func__);
		return;
	}

	if (args.data4 != FUNC_EXEC_SUCC)
		pr_err("%s :tzsp func fail errcode.0x%llx\n", __func__, args.data4);
}

unsigned int hisp_ffa_clr_memecc_intr(unsigned int type)
{
	int ret;
	struct ffa_send_direct_data args;

	args.data0 = ISP_FN_CLR_MEMECC_INTR;
	args.data1 = type;
	args.data2 = FUNC_ARGS_NO_USE;
	args.data3 = FUNC_ARGS_NO_USE;
	args.data4 = FUNC_ARGS_NO_USE;

	ret = g_ffa_isp_ops->sync_send_receive(g_ffa_isp_dev, &args);
	if (ret != 0) {
		pr_err("%s :failed to send ecc msg\n", __func__);
		return 0;
	}

	if (args.data0 != ISP_FN_CLR_MEMECC_INTR) {
		pr_err("%s :direct msg receive data error\n", __func__);
		return 0;
	}

	if (args.data4 != FUNC_EXEC_SUCC)
		pr_err("%s :tzsp func fail errcode.0x%llx, type.%d\n", __func__, args.data4, type);

	return (unsigned int)args.data1;
}

int hipp_ffa_deinit_smmu_tbu(void)
{
	int ret;
	struct ffa_send_direct_data args;

	args.data0 = IPP_FID_SMMUDISABLE;
	args.data1 = FUNC_ARGS_NO_USE;
	args.data2 = FUNC_ARGS_NO_USE;
	args.data3 = FUNC_ARGS_NO_USE;
	args.data4 = FUNC_ARGS_NO_USE;

	ret = g_ffa_isp_ops->sync_send_receive(g_ffa_isp_dev, &args);
	if (ret != 0) {
		pr_err("Failed FFA_ISP_IPP: %s failed to send_msg!\n", __func__);
		return ret;
	}

	if (args.data0 != IPP_FID_SMMUDISABLE) {
		pr_err("Failed FFA_ISP_IPP: %s direct msg receive data error!\n", __func__);
		return FUNC_EXEC_FAIL;
	}

	if (args.data4 != FUNC_EXEC_SUCC) {
		pr_err("Failed FFA_ISP_IPP: %s tzsp func fail errcode[0x%llx]!\n", __func__, args.data4);
		return FUNC_EXEC_FAIL;
	}

	return FUNC_EXEC_SUCC;
}

int hipp_ffa_init_smmu_tbu(void)
{
	int ret;
	struct ffa_send_direct_data args;

	args.data0 = IPP_FID_SMMUENABLE;
	args.data1 = FUNC_ARGS_NO_USE;
	args.data2 = FUNC_ARGS_NO_USE;
	args.data3 = FUNC_ARGS_NO_USE;
	args.data4 = FUNC_ARGS_NO_USE;

	ret = g_ffa_isp_ops->sync_send_receive(g_ffa_isp_dev, &args);
	if (ret != 0) {
		pr_err("Failed FFA_ISP_IPP: %s failed to send_msg!\n", __func__);
		return ret;
	}

	if (args.data0 != IPP_FID_SMMUENABLE) {
		pr_err("Failed FFA_ISP_IPP: %s direct msg receive data error!\n", __func__);
		return FUNC_EXEC_FAIL;
	}

	if (args.data4 != FUNC_EXEC_SUCC) {
		pr_err("Failed FFA_ISP_IPP: %s tzsp func fail errcode[0x%llx]!\n", __func__, args.data4);
		return FUNC_EXEC_FAIL;
	}

	return FUNC_EXEC_SUCC;
}

int hipp_ffa_cfg_smmu_ssidv(u16 swid, u16 len, u16 sid, u16 ssid)
{
	int ret;
	u64 sec_base_info, sec_id_info;
	struct ffa_send_direct_data args;

	sec_base_info = swid;
	sec_base_info <<= 16;
	sec_base_info += len;

	sec_id_info = sid;
	sec_id_info <<= 16;
	sec_id_info += ssid;

	args.data0 = IPP_FID_SMMUSMRX;
	args.data1 = sec_base_info;
	args.data2 = sec_id_info;
	args.data3 = FUNC_ARGS_NO_USE;
	args.data4 = FUNC_ARGS_NO_USE;

	ret = g_ffa_isp_ops->sync_send_receive(g_ffa_isp_dev, &args);
	if (ret != 0) {
		pr_err("Failed FFA_ISP_IPP: %s failed to send_msg!\n", __func__);
		return ret;
	}

	if (args.data0 != IPP_FID_SMMUSMRX || args.data1 != sec_base_info || args.data2 != sec_id_info) {
		pr_err("Failed FFA_ISP_IPP: %s direct msg receive data error!\n", __func__);
		return FUNC_EXEC_FAIL;
	}

	if (args.data4 != FUNC_EXEC_SUCC) {
		pr_err("Failed FFA_ISP_IPP: %s tzsp func fail errcode[0x%llx]!\n", __func__, args.data4);
		return FUNC_EXEC_FAIL;
	}

	return FUNC_EXEC_SUCC;
}

int hipp_ffa_cfg_smmu_perf(u16 swid, u16 len)
{
	int ret;
	u64 perfect_info;
	struct ffa_send_direct_data args;

	perfect_info = len;
	perfect_info <<= 16;
	perfect_info += swid;

	args.data0 = IPP_FID_SMMUPREFCFG;
	args.data1 = perfect_info;
	args.data2 = FUNC_ARGS_NO_USE;
	args.data3 = FUNC_ARGS_NO_USE;
	args.data4 = FUNC_ARGS_NO_USE;

	ret = g_ffa_isp_ops->sync_send_receive(g_ffa_isp_dev, &args);
	if (ret != 0) {
		pr_err("Failed FFA_ISP_IPP: %s failed to send_msg!\n", __func__);
		return ret;
	}

	if (args.data0 != IPP_FID_SMMUPREFCFG || args.data1 != perfect_info) {
		pr_err("Failed FFA_ISP_IPP: %s direct msg receive data error!\n", __func__);
		return FUNC_EXEC_FAIL;
	}

	if (args.data4 != FUNC_EXEC_SUCC) {
		pr_err("Failed FFA_ISP_IPP: %s tzsp func fail errcode[0x%llx]!\n", __func__, args.data4);
		return FUNC_EXEC_FAIL;
	}

	return FUNC_EXEC_SUCC;
}

module_init(hisp_ffa_init);
