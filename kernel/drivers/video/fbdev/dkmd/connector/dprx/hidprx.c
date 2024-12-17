/** @file
 * Copyright (c) 2020-2020, Hisilicon Tech. Co., Ltd. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#include "hidprx_reg.h"
#include "hidprx_core.h"
#include "hidprx_irq.h"
#include "hidprx.h"
#include <linux/clk.h>
#include <linux/gpio.h>
#include "hidprx_ctrl.h"
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/notifier.h>
#include <linux/platform_drivers/usb/chip_usb.h>
#include <linux/platform_drivers/usb/hiusb_common.h>
/* for test */
uint32_t g_dprx_log_level = DPRX_LOG_LVL_DEBUG;
void set_dprx_debug_level(uint32_t log_level)
{
	g_dprx_log_level = log_level;
}

struct platform_device *g_dprx_pdev[DPRX_CTRL_NUMS_MAX];

static void dprx_clk_disable_unprepare(struct dprx_ctrl *dprx)
{
	int i;
	for (i = 0; i < CLK_DPRX_MAX; i++)
		clk_disable_unprepare(dprx->rx_clk[i]);
}

int dprx_off(struct dprx_ctrl *dprx)
{
	int ret = 0;

	dpu_check_and_return((dprx == NULL), -EINVAL, err, "dprx is NULL!\n");
	dprx_pr_info("+++\n");

	mutex_lock(&dprx->dprx_mutex);
	if (!dprx->power_on) {
		dprx_pr_info("[DPRX] dprx has already off\n");
		mutex_unlock(&dprx->dprx_mutex);
		return 0;
	}

	/* ctrl config */
	dprx_core_off(dprx);

	if (dprx->dprx_sdp_wq != NULL) {
		destroy_workqueue(dprx->dprx_sdp_wq);
		dprx->dprx_sdp_wq = NULL;
	}
	dprx_clk_disable_unprepare(dprx);
	dprx->power_on = false;
	dprx->v_params.video_format_valid = false;
	dprx->msa.msa_valid = false;

	dprx->video_input_params_ready = false;

	mutex_unlock(&dprx->dprx_mutex);

	dprx_pr_info("---\n");

	return ret;
}

static void dprx_clk_prepare_enable(struct dprx_ctrl *dprx)
{
	int i;
	for (i = 0; i < CLK_DPRX_MAX; i++) {
		if (clk_prepare_enable(dprx->rx_clk[i]) != 0) {
			dprx_pr_err("[DP] enable clk failed, i:%d!\n", i);
			break;
		}
	}
}

int dprx_on(struct dprx_ctrl *dprx)
{
	int ret = 0;

	dpu_check_and_return((dprx == NULL), -EINVAL, err, "dprx is NULL!\n");
	dprx_pr_info("+++\n");

	mutex_lock(&dprx->dprx_mutex);
	if (dprx->power_on) {
		dprx_pr_info("[DPRX] dprx has already on\n");
		mutex_unlock(&dprx->dprx_mutex);
		return 0;
	}

	dprx_clk_prepare_enable(dprx);
	/* ctrl config */
	dprx_core_on(dprx);

	/* Check here: In dprx doc, enable clk after ctrl reset */
	dprx->dprx_sdp_wq = create_singlethread_workqueue("dprx_sdp_wq");
	if (dprx->dprx_sdp_wq == NULL) {
		dprx_pr_err("[DP] create dprx->dprx_sdp_wq failed\n");
		mutex_unlock(&dprx->dprx_mutex);
		return -1;
	}

	INIT_WORK(&dprx->dprx_sdp_work, dprx_sdp_wq_handler);

	dprx->power_on = true;

	mutex_unlock(&dprx->dprx_mutex);

	dprx_pr_info("---\n");

	return ret;
}

int disp_dprx_hpd_trigger(TCA_DEV_TYPE_E dev_type, uint32_t intface_type, uint32_t mode, uint8_t id,
							TYPEC_PLUG_ORIEN_E typec_orien)
{
	struct dpu_dprx_device *dprx_data = NULL;
	struct dprx_ctrl *dprx = NULL;
	int ret = 0;

	dpu_check_and_return((id >= DPRX_CTRL_NUMS_MAX), -EINVAL, err, "[DPRX] rx_id >= DPRX_CTRL_NUMS_MAX!\n");
	dpu_check_and_return((g_dprx_pdev[id] == NULL), -EINVAL, err, "[DPRX] g_dprx_pdev is NULL!\n");

	dprx_data = platform_get_drvdata(g_dprx_pdev[id]);
	dpu_check_and_return((dprx_data == NULL), -EINVAL, err, "[DP] dprx_data is NULL!\n");

	dprx = &(dprx_data->dprx);
	dprx_pr_info("[DPRX] dprx%u +\n", dprx->id);
	dprx->intface_type = intface_type; // standard dp or typeC
	dprx->typec_orien = typec_orien; // forward or reverse
	dprx->mode = mode; // edp or dp
	if (dev_type == TCA_DP_IN)
		dprx_on(dprx);
	else
		dprx_off(dprx);

	dprx_pr_info("[DPRX] dprx%u -\n", dprx->id);
	return ret;
}

static int dprx_irq_setup(struct dprx_ctrl *dprx)
{
	int ret;

	dpu_check_and_return((dprx == NULL || dprx->irq == 0), -EINVAL, err, "dprx is NULL!\n");

	ret = devm_request_threaded_irq(&dprx->device->pdev->dev,
		dprx->irq, dprx->dprx_irq_handler, dprx->dprx_threaded_irq_handler,
		IRQF_SHARED | IRQ_LEVEL, "dpu_dprx", (void *)dprx->device);
	if (ret) {
		dprx_pr_err("[DP] Request for irq %u failed!\n", dprx->irq);
		return -EINVAL;
	}
	disable_irq(dprx->irq);

	return 0;
}

static void dprx_link_layer_init(struct dprx_ctrl *dprx)
{
}

static void dprx_ctrl_layer_init(struct dprx_ctrl *dprx)
{
	dprx->dprx_irq_handler = dprx_irq_handler;
	dprx->dprx_threaded_irq_handler = dprx_threaded_irq_handler;
}

static void dprx_init_timing(dss_intfinfo_t *intf_info, uint16_t timing_code)
{
	intf_info->timing_code = timing_code;
	intf_info->format = 6;

	if (timing_code == TIMING_1080P) {
		intf_info->xres = 1920;
		intf_info->yres = 1080;
		intf_info->framerate = 30;
	} else {
		intf_info->xres = 720;
		intf_info->yres = 480;
		intf_info->framerate = 60;
	}
}


static int dprx_device_init(struct dprx_ctrl *dprx)
{
	int ret;

	dpu_check_and_return((dprx == NULL), -EINVAL, err, "dprx is NULL!\n");

	dprx_init_timing(&dprx->intf_info, TIMING_1080P);

	rx_edid_init(&dprx->edid, dprx->edid.valid_block_num, dprx->intf_info.timing_code);
	rx_sdp_init(&dprx->pps_table);

	dprx_link_layer_init(dprx);
	dprx_ctrl_layer_init(dprx);

	ret = dprx_irq_setup(dprx);
	if (ret) {
		dprx_pr_err("[DPRX] dprx set irq fail\n");
		return ret;
	}

	dprx->power_on = false;
	dprx->video_on = false;

	return 0;
}

static int dprx_read_dtsi(struct dpu_dprx_device *dprx_data, struct device_node *node)
{
	int ret;
	uint8_t i;
	char __iomem *rx_base_addrs[DPRX_BASE_ADDR_MAX] = {NULL};

	/**
	 * add clk/base_addr/irq etc.
	 * */

	ret = of_property_read_u32(dprx_data->pdev->dev.of_node, "channel_id", &(dprx_data->dprx.id));
	if (ret) {
		dprx_pr_err("[DPRX] read id fail\n");
		return -EINVAL;
	}
	if (dprx_data->dprx.id >= DPRX_CTRL_NUMS_MAX) {
		dprx_pr_err("[DPRX] invalid id\n");
		return -EINVAL;
	}
	dprx_pr_info("[DPRX] dprx id: %u.\n", dprx_data->dprx.id);

	ret = of_property_read_u32(dprx_data->pdev->dev.of_node, "mode", &(dprx_data->dprx.mode));
	if (ret) {
		dprx_pr_err("[DPRX] read mode fail\n");
		return -EINVAL;
	}
	dprx_pr_info("[DPRX] dprx mode: %u.\n", dprx_data->dprx.mode);

	ret = of_property_read_u32(dprx_data->pdev->dev.of_node, "edid_block_num", &(dprx_data->dprx.edid.valid_block_num));
	if (ret) {
		dprx_pr_err("[DPRX] read edid_block_num fail\n");
		return -EINVAL;
	}
	if (dprx_data->dprx.edid.valid_block_num > MAX_EDID_BLOCK_NUM) {
		dprx_pr_err("[DPRX] error edid_block_num\n");
		return -EINVAL;
	}
	dprx_pr_info("[DPRX] edid_block_num: %u\n", dprx_data->dprx.edid.valid_block_num);

	ret = of_property_read_u32(dprx_data->pdev->dev.of_node, "hpd_gpio", &(dprx_data->dprx.hpd_gpio));
	if (ret) {
		dprx_pr_err("[DPRX] read mode fail\n");
		return -EINVAL;
	}
	dprx_pr_info("[DPRX] dprx hpd_gpio: %u.\n", dprx_data->dprx.hpd_gpio);

	dprx_data->dprx.irq = irq_of_parse_and_map(dprx_data->pdev->dev.of_node, 0);
	if (dprx_data->dprx.irq == 0)
		dprx_pr_err("get irq no fail\n");
	dprx_pr_info("[DPRX] irq_num: %u\n", dprx_data->dprx.irq);

	for (i = 0; i < DPRX_BASE_ADDR_MAX; i++) {
		rx_base_addrs[i] = of_iomap(dprx_data->pdev->dev.of_node, i);
		if (!rx_base_addrs[i]) {
			dprx_pr_err("get rx base addr %u fail", i);
			return -EINVAL;
		}
	}

	dprx_data->dprx.hsdt1_crg_base = rx_base_addrs[DPRX_HSDT1_CRG_BASE];
	dprx_data->dprx.hsdt1_sys_ctrl_base = rx_base_addrs[DPRX_HSDT1_SYS_CTRL_BASE];
	dprx_data->dprx.base = rx_base_addrs[DPRX_CTRL_BASE];
	dprx_data->dprx.edid_base = rx_base_addrs[DPRX_EDID_BASE];
	dprx_data->dprx.ioc_peri_base = rx_base_addrs[IOC_PERI_BASE];
	dprx_data->dprx.fpga_flag = true; /* for test, add dts item for fpga flag */
	for (i = 0; i < CLK_DPRX_MAX; i++)
		dprx_data->dprx.rx_clk[i] = of_clk_get(node, i);
	return 0;
}

static irqreturn_t dprx_cable_det_gpio_irq(int irq, void *ptr)
{
	dprx_pr_info("[dprx] cable_det_gpio irq\n");
	return IRQ_WAKE_THREAD;
}

static irqreturn_t dprx_cable_det_gpio_irq_thread(int irq, void *ptr)
{
	struct dpu_dprx_device *dprx_dev = ptr;
	struct dprx_ctrl *dprx_ctrl = NULL;
	uint32_t is_plug = 0;

	if (dprx_dev == NULL) {
		dprx_pr_err("[dprx]parameter invalid, ptr null\n");
		return 0;
	}
	dprx_ctrl = &(dprx_dev->dprx);

	is_plug = gpio_get_value_cansleep(dprx_ctrl->cable_det_gpio);

	dprx_pr_info("[dprx] is_plug %d\n", is_plug);

	mdelay(5);

	if (is_plug == 1) {
		mdelay(100);
		is_plug = gpio_get_value_cansleep(dprx_ctrl->cable_det_gpio);
		dprx_pr_info("[dprx]mdelay is_plug %d\n", is_plug);
		if (is_plug != 1) {
			dprx_pr_info("[dprx] ignore the irq is_plug %d\n", is_plug);
			dprx_ctrl->cable_det_status = true;
			return 0;
		} else {
			dprx_pr_info("[dprx] hotplug out\n");
			dprx_ctrl->cable_det_status = false;
		}
	} else {
		dprx_pr_info("[dprx] hotplug in\n");
		dprx_ctrl->cable_det_status = true;
	}

	if (dprx_ctrl->cable_det_status == true) {
		disp_dprx_hpd_trigger(TCA_DP_IN, STANDARD_DP_INTERFACE, DP_MODE, dprx_ctrl->id, TYPEC_ORIEN_POSITIVE);
	} else {
		disp_dprx_hpd_trigger(TCA_DP_OUT, STANDARD_DP_INTERFACE, DP_MODE, dprx_ctrl->id, TYPEC_ORIEN_POSITIVE);
	}
	return 0;
}

static int dprx_init_cable_det_gpio(struct dprx_ctrl *dprx)
{
	int ret;
	int irq_num;
	if (dprx->id != 0)
		return 0;
	dprx->cable_det_gpio = 282;

	ret = gpio_request(dprx->cable_det_gpio, "cable_det");
	if (ret) {
		dprx_pr_err("[dprx] Fail[%d] request gpio:%d\n", ret, dprx->cable_det_gpio);
		return ret;
	}

	ret = gpio_direction_input(dprx->cable_det_gpio);
	if (ret < 0) {
		dprx_pr_err("[dprx] Failed to set gpio direction\n");
		return ret;
	}

	irq_num = gpio_to_irq(dprx->cable_det_gpio);
	if (irq_num < 0) {
		dprx_pr_err("[dprx] Failed to get dp_cable_det_gpio irq\n");
		ret = -EINVAL;
		return ret;
	}

	ret = devm_request_threaded_irq(&dprx->device->pdev->dev,
		irq_num, dprx_cable_det_gpio_irq, dprx_cable_det_gpio_irq_thread,
		IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
		"cable_det", (void *)dprx->device);
	if (ret) {
		dprx_pr_info("[dprx] Failed to request press interupt handler\n");
		return ret;
	}
	dprx->cable_det_irq = irq_num;
	dprx_pr_info("[dprx] hpd_irq %d\n", irq_num);

	return 0;
}

static int dprx_init_hpd_gpio(struct dprx_ctrl *dprx)
{
	int ret;
	int i;
	dprx_pr_info("init gpio %d\n", dprx->hpd_gpio);
	ret = gpio_request(dprx->hpd_gpio, "hpd_gpio0");
	if (ret) {
		dprx_pr_err("[dprx] Fail[%d] request gpio:%d\n", ret, dprx->hpd_gpio);
		return ret;
	}
	ret = gpio_direction_output(dprx->hpd_gpio, 0);
	if (ret < 0) {
		dprx_pr_err("[DP] Set gpio-%d output fail\n", dprx->hpd_gpio);
		return -EINVAL;
	}
	return 0;
}

static struct dpu_dprx_device *g_dpu_dprx_dev;
static struct dpu_dprx_device *dprx_get_dev(void)
{
	return g_dpu_dprx_dev;
}

static int dprx_drv_open(struct inode *inode, struct file *filp)
{
	dpu_check_and_return(unlikely(!inode), -EINVAL, err, "inode is null");
	dpu_check_and_return(unlikely(!filp), -EINVAL, err, "filp is null");

	dprx_pr_info("+++\n");

	if (!filp->private_data) {
		filp->private_data = dprx_get_dev();
		if (!filp->private_data)
			dprx_pr_err("private_data null\n");
	}

	dprx_pr_info("---\n");
	return 0;
}

static int dprx_drv_release(struct inode *inode, struct file *filp)
{
	return 0;
}

static long dprx_drv_ioctl(struct file *filp, unsigned int cmd, unsigned long arg, uint32_t id)
{
	void __user *argp = (void __user *)(uintptr_t)arg;
	struct dpu_dprx_device *dprx_dev = NULL;
	dprx_pr_info("[DPRX] dprx_drv_ioctl +\n");
	dpu_check_and_return(unlikely(!filp), -EINVAL, err, "filp is null");
	dpu_check_and_return(unlikely(!filp->private_data), -EINVAL, err, "private_data is null");
	dpu_check_and_return(unlikely(!argp), -EINVAL, err, "argp is null");

	dprx_dev = platform_get_drvdata(g_dprx_pdev[id]);
	dprx_pr_info("[DPRX] cmd : %u\n", cmd);
	switch (cmd) {
	case DISP_INPUTSRC_WAIT_VACTIVE_START:
		return dprx_ctrl_wait_vactive_start(&(dprx_dev->interface_impl), argp);
	case DISP_INPUTSRC_ENABLE_VACTIVE_START:
		return dprx_ctrl_enable_vactive_start(&(dprx_dev->interface_impl), argp);
	case DISP_INPUTSRC_GET_TIMING_INFO:
		return dprx_ctrl_get_timinginfo(&(dprx_dev->interface_impl), argp);
	default:
		dprx_pr_info("unsupported cmd=%#x", cmd);
		return -EINVAL;
	}
	dprx_pr_info("[DPRX] dprx_drv_ioctl -\n");
	return 0;
}

static long dprx0_drv_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	dprx_pr_info("[DPRX] dprx0 ioctrl +\n");
	dpu_check_and_return(unlikely(!filp), -EINVAL, err, "filp is null");
	return dprx_drv_ioctl(filp, cmd, arg, 0);
}

static long dprx1_drv_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	dprx_pr_info("[DPRX] dprx1 ioctrl +\n");
	dpu_check_and_return(unlikely(!filp), -EINVAL, err, "filp is null");
	return dprx_drv_ioctl(filp, cmd, arg, 1);
}

static ssize_t dprx_debug_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return strlen(buf);
}

static ssize_t dprx_debug_store(struct device *device,
			struct device_attribute *attr, const char *buf, size_t count)
{
	return count;
}

static struct device_attribute dprx_attrs[] = {
	__ATTR(dprx_debug, S_IRUGO | S_IWUSR, dprx_debug_show, dprx_debug_store),

	/* TODO: other attrs */
};

static struct file_operations g_hidprx_fops[] = {
	{.owner = THIS_MODULE,
	.open = dprx_drv_open,
	.release = dprx_drv_release,
	.unlocked_ioctl = dprx0_drv_ioctl,
	.compat_ioctl =  dprx0_drv_ioctl,},
	{.owner = THIS_MODULE,
	.open = dprx_drv_open,
	.release = dprx_drv_release,
	.unlocked_ioctl = dprx1_drv_ioctl,
	.compat_ioctl =  dprx1_drv_ioctl,}
};

static char g_dp_port_name[][32] = {HISI_HIDPRX0_CHR_NAME, HISI_HIDPRX1_CHR_NAME};

static void dprx_drv_create_chrdev(struct dpu_dprx_device *dprx_dev)
{
	int size;
	dprx_pr_info("[DPRX] create_chrdev +\n");
	size = sizeof(g_hidprx_fops) / sizeof(g_hidprx_fops[0]);
	if (dprx_dev->dprx.id >= size) {
		dprx_pr_info("[DPRX] create dev Exceeds the capacity. +\n");
		return;
	}
	/* init chrdev info */
	dprx_dev->chrdev.name = g_dp_port_name[dprx_dev->dprx.id];
	dprx_dev->chrdev.fops = &g_hidprx_fops[dprx_dev->dprx.id];
	dprx_dev->chrdev.drv_data = dprx_dev;

	dkmd_create_chrdev(&dprx_dev->chrdev);
	dkmd_create_attrs(dprx_dev->chrdev.chr_dev, dprx_attrs, ARRAY_SIZE(dprx_attrs));
	dprx_pr_info("[DPRX] create_chrdev -\n");
}

static int dprx_usb_chain_notify(struct notifier_block *nb, unsigned long action, void *data)
{
	struct dpu_dprx_device *dprx_data = NULL;
	struct dprx_ctrl *dprx = NULL;
	uint32_t typec_id;
	int ret = 0;
	enum usb_state state = (enum usb_state)action;

	if (data == NULL)
		return NOTIFY_BAD;
	typec_id = *((uint32_t *)data);
	dprx_pr_info("usb notify: state %d, id %d\n", state, typec_id);
	if (typec_id > 1)
		return NOTIFY_DONE;
	dpu_check_and_return((g_dprx_pdev[typec_id] == NULL), NOTIFY_DONE, err, "[DPRX] g_dprx_pdev is NULL!\n");
	dprx_data = platform_get_drvdata(g_dprx_pdev[typec_id]);
	dpu_check_and_return((dprx_data == NULL), NOTIFY_DONE, err, "[DP] dprx_data is NULL!\n");
	dprx = &(dprx_data->dprx);
	if ((dprx->power_on == 0) && (state == USB_STATE_DEVICE || state == USB_STATE_HOST))
		dprx_phy_set_resistance_high_state(dprx);
	return NOTIFY_DONE;
}

static bool g_dprx_usb_notify_is_registered = false;

struct notifier_block g_dp_nb = {
	.notifier_call = dprx_usb_chain_notify,
};

static int dprx_probe(struct platform_device *pdev)
{
	struct dpu_dprx_device *dprx_data = NULL;

	int ret;

	dpu_check_and_return((pdev == NULL), -EINVAL, err, "pdev is NULL!\n");
	dprx_pr_info("[DPRX] dprx probe ++++++\n");

	dprx_data = devm_kzalloc(&pdev->dev, sizeof(*dprx_data), GFP_KERNEL);
	if (!dprx_data) {
		dprx_pr_err("alloc dprx fail\n");
		return -ENOMEM;
	}
	dprx_data->pdev = pdev;
	dprx_data->dprx.device = dprx_data;

	ret = dprx_read_dtsi(dprx_data, pdev->dev.of_node);
	if (ret) {
		dprx_pr_err("[DPRX] read dtsi fail\n");
		return -EINVAL;
	}

	ret = dprx_device_init(&(dprx_data->dprx));
	if (ret) {
		dprx_pr_err("[DPRX] init dprx fail\n");
		return -EINVAL;
	}
	dprx_ctrl_interface_init(&(dprx_data->interface_impl));
	dprx_drv_create_chrdev(dprx_data);
	(void)dprx_init_hpd_gpio(&dprx_data->dprx);
	g_dprx_pdev[dprx_data->dprx.id] = dprx_data->pdev;
	platform_set_drvdata(pdev, dprx_data);
	g_dpu_dprx_dev = dprx_data;
	mutex_init(&(dprx_data->dprx.dprx_mutex));
	if (g_dprx_usb_notify_is_registered == false) {
		ret = chip_usb_state_notifier_register(&g_dp_nb);
		dprx_pr_info("notifier_register:ret %d \n", ret);
		g_dprx_usb_notify_is_registered = true;
	}
	dprx_pr_info("[DPRX] dprx probe ------\n");
	return 0;
}

static void dprx_free_hpd_gpio(struct dprx_ctrl *dprx)
{
	gpio_free(dprx->cable_det_gpio);
	gpio_free(dprx->hpd_gpio);
}

static int dprx_remove(struct platform_device *pdev)
{
	struct dpu_dprx_device *dprx_data = NULL;

	dpu_check_and_return((pdev == NULL), 0, err, "pdev is NULL!\n");

	dprx_data = platform_get_drvdata(pdev);
	if (!dprx_data)
		return 0;

	dprx_pr_info("[DPRX] remove dprx%u++++++\n", dprx_data->dprx.id);
	dprx_free_hpd_gpio(&dprx_data->dprx);
	g_dprx_pdev[dprx_data->dprx.id] = NULL;

	dprx_pr_info("[DPRX] remove dprx%u------\n", dprx_data->dprx.id);
	return 0;
}

uint32_t dprx_ctrl_get_layer_fmt(struct dprx_ctrl *dprx)
{
	uint32_t bit_width;
	uint32_t color_space;
	uint32_t format;

	bit_width = dprx->v_params.bpc;
	color_space = dprx_get_color_space(dprx);

	format = dprx_ctrl_dp2layer_fmt(color_space, bit_width);
	dprx_pr_info("bpp :%d, format value:%d\n", bit_width, format);

	return format;
}
uint32_t g_rx_id = 0;
void set_rxid_getfmt(uint32_t rx_id)
{
	g_rx_id = rx_id;
}

uint32_t dprx_get_layer_fmt(uint32_t src_fmt)
{
	struct dpu_dprx_device *dprx_data = NULL;
	struct dprx_ctrl *dprx = NULL;

	if ((g_rx_id >= DPRX_CTRL_NUMS_MAX) || (g_dprx_pdev[g_rx_id] == NULL)) {
		dprx_pr_info("return src fmt\n");
		return src_fmt;
	}

	dprx_data = platform_get_drvdata(g_dprx_pdev[g_rx_id]);
	if (dprx_data == NULL) {
		dprx_pr_info("return src fmt\n");
		return src_fmt;
	}
	dprx = &(dprx_data->dprx);
	if ((dprx == NULL) || (dprx->power_on == false) || (dprx->v_params.video_format_valid == false)) {
		dprx_pr_info("return src fmt\n");
		return src_fmt;
	}
	return dprx_ctrl_get_layer_fmt(dprx);
}

static const struct of_device_id device_match_table[] = {
	{
		.compatible = DTS_NAME_DPRX0,
		.data = NULL,
	},
	{
		.compatible = DTS_NAME_DPRX1,
		.data = NULL,
	},
	{},
};
MODULE_DEVICE_TABLE(of, device_match_table);

static struct platform_driver dprx_platform_driver = {
	.probe  = dprx_probe,
	.remove = dprx_remove,
	.driver = {
		.name  = DEV_NAME_DPRX,
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(device_match_table),
	}
};

static int __init dprx_register(void)
{
	int ret;
	dprx_pr_info("[DPRX] dprx registing");

	ret = platform_driver_register(&dprx_platform_driver);
	if (ret) {
		dprx_pr_err("platform_driver_register failed, error=%d!\n", ret);
		return ret;
	}

	return ret;
}

static void __exit dprx_unregister(void)
{
	dprx_pr_info("[DPRX] dprx unregisting");
	platform_driver_unregister(&dprx_platform_driver);
}

uint32_t dprx_select_timing(uint8_t rx_id, uint32_t timing_code)
{
	struct dpu_dprx_device *dprx_data = NULL;
	struct dprx_ctrl *dprx;

	dpu_check_and_return((rx_id >= DPRX_CTRL_NUMS_MAX), -EINVAL, err, "invalid rx_id!\n");
	dpu_check_and_return((g_dprx_pdev[rx_id] == NULL), -EINVAL, err, "dprx is NULL!\n");

	dprx_data = platform_get_drvdata(g_dprx_pdev[rx_id]);
	dprx = &dprx_data->dprx;

	dprx_init_timing(&dprx->intf_info, timing_code);
	rx_edid_init(&dprx->edid, dprx->edid.valid_block_num, timing_code);

	dprx_pr_info("[DTE] set dprx%d timing code to %d\n", rx_id, timing_code);

	return 0;
}

int dprx_dump_edid(uint8_t rx_id)
{
	struct dpu_dprx_device *dprx_data = NULL;

	dpu_check_and_return((rx_id >= DPRX_CTRL_NUMS_MAX), -EINVAL, err, "invalid rx_id!\n");
	dpu_check_and_return((g_dprx_pdev[rx_id] == NULL), -EINVAL, err, "dprx is NULL!\n");

	dprx_data = platform_get_drvdata(g_dprx_pdev[rx_id]);
	if (!dprx_data)
		return -1;

	rx_edid_dump(&dprx_data->dprx.edid);

	return 0;
}

uint32_t dprx_enable_videostream(uint8_t rx_id, bool flag)
{
	struct dpu_dprx_device *dprx_data = NULL;

	dpu_check_and_return((rx_id >= DPRX_CTRL_NUMS_MAX), -EINVAL, err, "invalid rx_id!\n");
	dpu_check_and_return((g_dprx_pdev[rx_id] == NULL), -EINVAL, err, "dprx is NULL!\n");

	dprx_data = platform_get_drvdata(g_dprx_pdev[rx_id]);
	if (!dprx_data)
		return -1;
	return 0;
}

#ifdef CONFIG_DKMD_DEBUG_ENABLE
EXPORT_SYMBOL_GPL(set_dprx_debug_level);
EXPORT_SYMBOL_GPL(disp_dprx_hpd_trigger);
EXPORT_SYMBOL_GPL(set_rxid_getfmt);
EXPORT_SYMBOL_GPL(dprx_get_layer_fmt);
EXPORT_SYMBOL_GPL(dprx_select_timing);
EXPORT_SYMBOL_GPL(dprx_dump_edid);
EXPORT_SYMBOL_GPL(dprx_enable_videostream);
#endif

module_init(dprx_register);
module_exit(dprx_unregister);

MODULE_AUTHOR("Graphics Display");
MODULE_DESCRIPTION("Dprx Function Driver");
MODULE_LICENSE("GPL");
