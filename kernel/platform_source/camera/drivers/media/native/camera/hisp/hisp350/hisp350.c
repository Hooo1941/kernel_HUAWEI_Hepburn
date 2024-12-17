// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 * Description: platform config file for hisp350
 * Create: 2021-04-10
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
 * GNU General Public License for more details.
 */

#include <linux/compiler.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/module.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/rpmsg.h>
#include <linux/skbuff.h>
#include <linux/types.h>
#include <linux/videodev2.h>
#include <media/v4l2-event.h>
#include <media/v4l2-fh.h>
#include <media/v4l2-subdev.h>
#include <media/videobuf2-core.h>
#include <linux/pm_qos.h>
#include <clocksource/arm_arch_timer.h>
#include <asm/arch_timer.h>
#include <linux/time.h>
#include <linux/jiffies.h>
#include <linux/pm_wakeup.h>
#include <linux/iommu/mm_iommu.h>
#include <platform_include/isp/linux/hisp_remoteproc.h>
#include <platform_include/isp/linux/hisp_mempool.h>
#include <linux/iommu.h>
#include <linux/mutex.h>
#include <platform_include/basicplatform/linux/mfd/pmic_platform.h>
#include <platform_include/smart/linux/sdc.h>

#include <platform_include/camera/native/camera.h>
#include <platform_include/camera/native/hisp350_cfg.h>
#include <cam_buf.h>
#include "cam_log.h"
#include "hisp_intf.h"
#include "isp_msg_pool.h"
#include "platform/sensor_commom.h"
#ifdef CONFIG_KERNEL_CAMERA_ISPV360
#include <platform_include/see/efuse_driver.h>
#include <efuse_kernel_def.h>
#include <soc_efuse_chip_level_flag.h>
#define CUT_PHY_INDEX 2
#endif

#define HISP_MSG_LOG_MOD 100

#define R8_TURBO_ISPCPU_CLK_RATE 1440000000
#define R8_NORMAL_ISPCPU_CLK_RATE 964000000
#define R8_LOWPOWER_ISPCPU_CLK_RATE 964000000
#define R8_ULTRALOW_ISPCPU_CLK_RATE 964000000

#define TIMEOUT_IS_FPGA_BOARD 100000
#define TIMEOUT_IS_NOT_FPGA_BOARD 15000

DEFINE_MUTEX(kernel_rpmsg_service_mutex);
DEFINE_MUTEX(kernel_fd_sgt_map_mutex);
static DEFINE_MUTEX(hisp_pinctrl_lock_mutex);

#ifdef CONFIG_DFX_DEBUG_FS
static struct pm_qos_request qos_request_ddr_down_record;
static int current_ddr_bandwidth = 0;
#endif
static struct wakeup_source *hisp_power_wakelock = NULL;
static struct mutex hisp_wake_lock_mutex;
static struct mutex hisp_power_lock_mutex;
static struct mutex hisp_mem_lock_mutex;

extern void hisp_boot_stat_dump(void);
static void hisp350_deinit_isp_mem(void);

enum isp_clk_level {
	ISP_CLK_LEVEL_TURBO,
	ISP_CLK_LEVEL_NORMAL,
	ISP_CLK_LEVEL_LOWPOWER,
	ISP_CLK_LEVEL_ULTRALOW,
	ISP_CLK_LEVEL_MAX,
};

enum hisp350_mem_pool_attr
{
	MEM_POOL_ATTR_READ_WRITE_CACHE = 0,
	MEM_POOL_ATTR_READ_WRITE_SECURITY,
	MEM_POOL_ATTR_READ_WRITE_ISP_SECURITY,
	MEM_POOL_ATTR_READ_WRITE_CACHE_OFF_LINE,
	MEM_POOL_ATTR_MAX,
};

struct hisp350_mem_pool {
	void *ap_va;
	unsigned int prot;
	unsigned int ion_iova;
	unsigned int r8_iova;
	size_t size;
	size_t align_size;
	int active;
	unsigned int security_isp_mode;
	struct sg_table *sgt;
	unsigned int shared_fd;
	unsigned int is_ap_cached;
	unsigned int usage;
};

struct isp_mem {
	int active;
	struct dma_buf *dmabuf;
};

struct hisp_fd_map {
	int shared_fd;
	unsigned int iova;
	size_t size;
	unsigned int type;
	struct sg_table *sgt;
	struct list_head nd;
	struct hisp_mem_info_s mem_info;
};

/*
 * @brief the instance to talk to hisp driver
 * When Histar ISP is probed, this sturcture will be initialized,
 * the object is used to notify hisp driver when needed.
 * @end
 */
typedef struct _tag_hisp350 {
	hisp_intf_t intf;
	hisp_notify_intf_t *notify;
	char const *name;
	atomic_t opened;
	struct platform_device *pdev; /* by used to get dts node */
	hisp_dt_data_t dt;
	struct hisp350_mem_pool mem_pool[MEM_POOL_ATTR_MAX];
	struct isp_mem mem;
	struct list_head fd_sgt_list;
} hisp350_t;

struct rpmsg_service_info {
	struct mutex sendlock;
	struct rpmsg_device *rpdev;
	struct isp_msg_pool *msg_pool[MAX_HISP_CLIENT];
	struct completion isp_comp;
};

static struct rpmsg_service_info rpmsg_local;
static bool remote_processor_up[MAX_HISP_CLIENT];
static int g_hisp_ref = 0;

#define to_isp(i) container_of(i, hisp350_t, intf)

static uint8_t get_client_id_by_msg_id(uint32_t message_id);
static void hisp350_notify_rpmsg_cb(uint8_t client_id);
static char const *hisp350_get_name(hisp_intf_t *i);
static int hisp350_config(hisp_intf_t *i, void *cfg);

static int hisp350_power_on(hisp_intf_t *i);
static int hisp350_power_off(hisp_intf_t *i);

static int hisp350_open(hisp_intf_t *i);
static int hisp350_close(hisp_intf_t *i);
static int hisp350_send_rpmsg(hisp_intf_t *i,
	struct isp_indirect_msg *msg, size_t len);
static int hisp350_recv_rpmsg(hisp_intf_t *i,
	struct isp_indirect_msg *msg, size_t len);

static int hisp350_set_sec_fw_buffer(struct hisp_cfg_data *cfg);
static int hisp350_release_sec_fw_buffer(void);

#ifdef CONFIG_DFX_DEBUG_FS
static void hisp350_set_ddrfreq(int ddr_bandwidth);
static void hisp350_release_ddrfreq(void);
static void hisp350_update_ddrfreq(int ddr_bandwidth);
#endif

static uint8_t hisp350_remote_processor_up(void)
{
	uint8_t up_count = 0;
	unsigned int i;
	for (i = 0; i < MAX_HISP_CLIENT; ++i)
		if (remote_processor_up[i])
			++up_count;
	return up_count;
}

bool modify_ack_timestamp_in_user(void)
{
	return true;
}

static hisp_vtbl_t s_vtbl_hisp350 = {
	.get_name = hisp350_get_name,
	.config = hisp350_config,
	.send_rpmsg = hisp350_send_rpmsg,
	.recv_rpmsg = hisp350_recv_rpmsg,
	.open = hisp350_open,
	.close = hisp350_close,
};

static hisp350_t s_hisp350 = {
	.intf = {.vtbl = &s_vtbl_hisp350,},
	.name = "hisp350",
};

static uint8_t get_client_id_by_msg_id(uint32_t message_id)
{
	return message_id >> HISP_MESSAGE_ID_CLIENT_BIT;
}

static void hisp350_notify_rpmsg_cb(uint8_t client_id)
{
	hisp_event_t isp_ev;
	isp_ev.kind = HISP_RPMSG_CB | (client_id << HISP_MESSAGE_ID_CLIENT_BIT);
	hisp_notify_intf_rpmsg_cb(s_hisp350.notify, &isp_ev);
}

/* Function declaration */
/**********************************************
 * Power up CSI/DPHY/sensor according to isp req
 * Only called by hisp350_rpmsg_ept_cb when api_name
 * is POWER_REQ, and will send a POWER_RSP to isp
 * after power request done.
 *********************************************/
static int hisp350_rpmsg_ept_cb(struct rpmsg_device *rpdev,
	void *data, int len, void *priv, u32 src)
{
	int rc;
	unsigned int i;

	(void)src;
	(void)rpdev;
	(void)priv;
	if (!data || len <= 0) {
		cam_err("func %s: data NULL or len:%d", __func__, len);
		return -EINVAL;
	}

	uint8_t client_id = get_client_id_by_msg_id(((struct isp_msg_header *)data)->message_id);
	((struct isp_msg_header *)data)->message_id &= (1 << HISP_MESSAGE_ID_CLIENT_BIT) - 1;
	unsigned api_name = ((struct isp_msg_header *)data)->api_name;
	if (client_id >= MAX_HISP_CLIENT) {
		cam_err("func %s: client id is %d, large than %d", __func__, client_id, MAX_HISP_CLIENT);
		return -EINVAL;
	}

	hisp_recvin(data);
	rc = queue_isp_msg(rpmsg_local.msg_pool[client_id], data, len);
	if (rc == 0) {
		hisp_recvx(data); // data saved to msg queue
		hisp350_notify_rpmsg_cb(client_id);
	}

#define MSG_EVENT_SENT 0x3000
	if (api_name == MSG_EVENT_SENT) {
		cam_debug("broadcast event to all client");
		int client_ids[MAX_HISP_CLIENT];
		unsigned client_num = hisp_get_current_client_ids(client_ids);
		for (i = 0; i < client_num; ++i)
			if (client_ids[i] != client_id && client_ids[i] >= 0 && client_ids[i] < MAX_HISP_CLIENT)
				if (queue_isp_msg(rpmsg_local.msg_pool[client_ids[i]], data, len) == 0)
					hisp350_notify_rpmsg_cb(client_ids[i]);
	}

	return rc;
}

static char const *hisp350_get_name(hisp_intf_t *itf)
{
	hisp350_t *isp = NULL;

	if (!itf) {
		cam_err("func %s: itf is NULL", __func__);
		return NULL;
	}

	isp = to_isp(itf);
	if (!isp) {
		cam_err("func %s: isp is NULL", __func__);
		return NULL;
	}
	return isp->name;
}

static int buffer_is_invalid(int share_fd, unsigned int req_addr,
	unsigned int req_size)
{
	int ret;
	struct iommu_format fmt = {0};

	ret = cam_buf_map_iommu(share_fd, &fmt);
	if (ret < 0) {
		cam_err("%s: fail to map iommu", __func__);
		return ret;
	}

	if (req_addr != fmt.iova || req_size > fmt.size) {
		cam_err("%s: req_iova:%#x,  req_size:%u",
			__func__, req_addr, req_size);
		cam_err("%s:real_iova:%#llx, real_size:%llu",
			__func__, fmt.iova, fmt.size);
		ret = -ERANGE;
	}
	cam_buf_unmap_iommu(share_fd, &fmt);
	return ret;
}

static int find_suitable_mem_pool(struct hisp_cfg_data *pcfg)
{
	int ipool;
	if (pcfg->param.type == MAP_TYPE_RAW2YUV) {
		ipool =  MEM_POOL_ATTR_READ_WRITE_CACHE_OFF_LINE;
	} else if (pcfg->param.type == MAP_TYPE_STATIC_ISP_SEC) {
		ipool =  MEM_POOL_ATTR_READ_WRITE_ISP_SECURITY;
	} else {
		for (ipool = 0; ipool < MEM_POOL_ATTR_MAX; ipool++) {
			if (s_hisp350.mem_pool[ipool].prot ==
				pcfg->param.prot) {
				break;
			}
		}

		if (ipool >= MEM_POOL_ATTR_MAX) {
			cam_err("func %s: no pool hit for prot:%d",
				__func__, pcfg->param.prot);
			return -EINVAL;
		}
	}
	return ipool;
}

static void hisp350_convert_mem_info_from_ap(const struct hisp_mem_info_t *ap_mem_info,
	struct hisp_mem_info_s *mem_info)
{
	mem_info->cpu_addr = ap_mem_info->cpu_addr;
	mem_info->cpu_size = ap_mem_info->cpu_size;
	mem_info->fw_mem.cpu_addr = ap_mem_info->fw_mem.cpu_addr;
	mem_info->fw_mem.core_addr = ap_mem_info->fw_mem.core_addr;
	mem_info->fw_mem.size = ap_mem_info->fw_mem.size;
	mem_info->ispfw_mem.cpu_addr = ap_mem_info->ispfw_mem.cpu_addr;
	mem_info->ispfw_mem.core_addr = ap_mem_info->ispfw_mem.core_addr;
	mem_info->ispfw_mem.size = ap_mem_info->ispfw_mem.size;
}

static void hisp350_convert_mem_info_to_ap(const struct hisp_mem_info_s *mem_info, struct hisp_mem_info_t *ap_mem_info)
{
	ap_mem_info->cpu_addr = mem_info->cpu_addr;
	ap_mem_info->cpu_size = mem_info->cpu_size;
	ap_mem_info->fw_mem.cpu_addr = mem_info->fw_mem.cpu_addr;
	ap_mem_info->fw_mem.core_addr = mem_info->fw_mem.core_addr;
	ap_mem_info->fw_mem.size = mem_info->fw_mem.size;
	ap_mem_info->ispfw_mem.cpu_addr = mem_info->ispfw_mem.cpu_addr;
	ap_mem_info->ispfw_mem.core_addr = mem_info->ispfw_mem.core_addr;
	ap_mem_info->ispfw_mem.size = mem_info->ispfw_mem.size;
}

#if defined(CONFIG_SENSOR_DIRECT_CHANNEL)
static struct sdc_mem_info *imu_info = NULL;
static imu_addr_param_t imu_map_param = {0};
static unsigned int imu_map_ref = 0;
static int hisp350_init_imu_buffer(void *cfg)
{
	int ret = 0;
	struct hisp_cfg_data *pcfg = NULL;
	struct hisp_map_info_s map_info = { 0 };
	struct hisp_mem_info_s mem_info = { 0 };
	if (cfg == NULL) {
		cam_err("%s: cfg is NULL", __FUNCTION__);
		return -EINVAL;
	}
	mutex_lock(&kernel_rpmsg_service_mutex);
	pcfg = (struct hisp_cfg_data*)cfg;
	if (imu_map_ref != 0) {
		pcfg->imu_param.iova = imu_map_param.iova;
		pcfg->imu_param.size = imu_map_param.size;
		imu_map_ref++;
		cam_info("imu already mapped ref = %d", imu_map_ref);
		mutex_unlock(&kernel_rpmsg_service_mutex);
		return ret;
	}
	imu_info = sdc_buf_get();
	if (imu_info == NULL) {
		cam_err("%s: sdc buffer get fail", __FUNCTION__);
		mutex_unlock(&kernel_rpmsg_service_mutex);
		return -EINVAL;
	}
	map_info.size = imu_info->size;
	map_info.paddr = imu_info->phy_addr;
	map_info.fw_mem_prot = IOMMU_READ;
	ret = hisp_mem_map_addr(pcfg->imu_param.usage, &map_info, &mem_info);
	if ((ret == 0) && mem_info.cpu_addr != 0) {
		pcfg->imu_param.iova = mem_info.cpu_addr;
		pcfg->imu_param.size = imu_info->size;
		imu_map_param.iova = mem_info.cpu_addr;
		imu_map_param.size = mem_info.cpu_size;
		imu_map_param.usage = pcfg->imu_param.usage;
		imu_map_ref++;
		cam_info("%s: iova = %x, size = %d", __FUNCTION__, pcfg->imu_param.iova, pcfg->imu_param.size);
	} else {
		ret = -ENOMEM;
		cam_err("%s: hisp_mem_map_addr failed", __FUNCTION__);
	}
	mutex_unlock(&kernel_rpmsg_service_mutex);
	return ret;
}

static int hisp350_deinit_imu_buffer(void *cfg)
{
	int ret = 0;
	struct hisp_cfg_data *pcfg = NULL;
	struct hisp_mem_info_s mem_info = { 0 };
	if (cfg == NULL) {
		cam_err("func %s: cfg is NULL", __FUNCTION__);
		return -EINVAL;
	}
	mutex_lock(&kernel_rpmsg_service_mutex);
	if (imu_map_ref == 0) {
		cam_info("imu is not mapped");
		mutex_unlock(&kernel_rpmsg_service_mutex);
		return -EINVAL;
	}
	if (imu_map_ref > 1) {
		imu_map_ref--;
		cam_info("imu already mapped ref = %d", imu_map_ref);
		mutex_unlock(&kernel_rpmsg_service_mutex);
		return ret;
	}
	pcfg = (struct hisp_cfg_data*)cfg;
	cam_info("%s: iova = %x, size = %d", __FUNCTION__, pcfg->imu_param.iova, pcfg->imu_param.size);
	if ((pcfg->imu_param.iova != imu_map_param.iova) || (pcfg->imu_param.size != imu_map_param.size)) {
		cam_err("deinit param error");
		mutex_unlock(&kernel_rpmsg_service_mutex);
		return -EINVAL;
	}
	mem_info.cpu_addr = pcfg->imu_param.iova;
	mem_info.cpu_size = pcfg->imu_param.size;
	ret = hisp_mem_unmap_addr(pcfg->imu_param.usage, mem_info);
	if (ret != 0) {
		cam_err("%s: hisp_mem_unmap_addr failed", __FUNCTION__);
		ret = -EINVAL;
	}
	imu_map_ref = 0;
	imu_map_param.iova = 0;
	imu_map_param.size = 0;
	sdc_buf_put(imu_info);
	imu_info = NULL;
	mutex_unlock(&kernel_rpmsg_service_mutex);
	return ret;
}
#endif

static int hisp350_init_r8isp_memory_pool(void *cfg)
{
	int ipool;
	int ret;
	struct hisp_cfg_data *pcfg = NULL;
	struct sg_table *sgt = NULL;
	enum maptype enm_map_type;
	struct hisp_map_info_s map_info = { 0 };
	struct hisp_mem_info_s mem_info = { 0 };

	if (cfg == NULL) {
		cam_err("%s: cfg is NULL", __func__);
		return -EINVAL;
	}

	pcfg = (struct hisp_cfg_data*)cfg;
	cam_info("%s: pool cfg vaddr=0x%pK, iova=0x%x, size=0x%x", __func__,
		pcfg->param.vaddr, pcfg->param.iova, pcfg->param.size);
	cam_info("%s: type=%d, prot=0x%x align=0x%zx sec=0x%x", __func__,
		pcfg->param.type, pcfg->param.prot,
		pcfg->param.pool_align_size, pcfg->param.security_isp_mode);

	if (!pcfg->param.security_isp_mode &&
		buffer_is_invalid(pcfg->param.sharedFd,
		pcfg->param.iova, pcfg->param.size)) {
		cam_err("check buffer fail");
		return -EINVAL;
	}

	ipool = find_suitable_mem_pool(pcfg);
	if (ipool < 0)
		return ipool;

	enm_map_type = pcfg->param.type;
	if (enm_map_type != MAP_TYPE_DYNAMIC_SEC) {
		if (ipool == MEM_POOL_ATTR_READ_WRITE_CACHE_OFF_LINE) {
			enm_map_type = MAP_TYPE_RAW2YUV;
		} else {
			enm_map_type = pcfg->param.security_isp_mode ?
				MAP_TYPE_STATIC_SEC : MAP_TYPE_DYNAMIC;
		}
	}

	/* take care of putting sgtable. */
	sgt = cam_buf_get_sgtable(pcfg->param.sharedFd);
	if (IS_ERR(sgt)) {
		cam_err("%s: fail to get sgtable", __func__);
		return -ENOENT;
	}

	mutex_lock(&kernel_rpmsg_service_mutex);
	if (pcfg->param.type == MAP_TYPE_DYNAMIC_SEC ||
		pcfg->param.type == MAP_TYPE_STATIC_SEC ||
		pcfg->param.type == MAP_TYPE_STATIC_ISP_SEC) {
		ret = hisp_secmem_ca_map((unsigned int)ipool,
			pcfg->param.sharedFd, pcfg->param.size);
		if (ret != 0) {
			cam_err("%s: hisp_secmem_ca_map failed", __func__);
			mutex_unlock(&kernel_rpmsg_service_mutex);
			cam_buf_put_sgtable(sgt);
			return -ENOMEM;
		}
	}
	map_info.size = pcfg->param.size;
	map_info.fw_mem_prot = pcfg->param.prot;
	ret = hisp_mem_map_addr(pcfg->param.usage, &map_info, &mem_info);
	hisp350_convert_mem_info_to_ap(&mem_info, &pcfg->param.mem_info);
	if (ret != 0) {
		cam_err("%s: hisp_mem_map_addr failed", __func__);
		(void)hisp_secmem_ca_unmap((unsigned int)ipool);
		mutex_unlock(&kernel_rpmsg_service_mutex);
		cam_buf_put_sgtable(sgt);
		return -ENOMEM;
	}

	/* hold sg_table things, release at deinit. */
	s_hisp350.mem_pool[ipool].sgt = sgt;
	s_hisp350.mem_pool[ipool].r8_iova = pcfg->param.mem_info.cpu_addr;
	s_hisp350.mem_pool[ipool].ap_va = pcfg->param.vaddr;
	s_hisp350.mem_pool[ipool].ion_iova = pcfg->param.iova;
	s_hisp350.mem_pool[ipool].size = pcfg->param.size;
	s_hisp350.mem_pool[ipool].align_size = pcfg->param.pool_align_size;
	s_hisp350.mem_pool[ipool].security_isp_mode =
		pcfg->param.security_isp_mode;
	s_hisp350.mem_pool[ipool].shared_fd = pcfg->param.sharedFd;
	s_hisp350.mem_pool[ipool].is_ap_cached = pcfg->param.isApCached;
	s_hisp350.mem_pool[ipool].usage = pcfg->param.usage;
	/*
	 * ion iova isn't equal r8 iova, security or unsecurity, align etc
	 * return r8 iova to daemon, and send to r8 later
	 */
	pcfg->param.iova = pcfg->param.mem_info.cpu_addr;
	s_hisp350.mem_pool[ipool].active = 1;

	cam_info("%s: r8_iova_pool_base=0x%x",
		__func__, s_hisp350.mem_pool[ipool].r8_iova);
	mutex_unlock(&kernel_rpmsg_service_mutex);
	return 0;
}

static int hisp350_deinit_r8isp_memory_pool(void *cfg)
{
	int ipool;
	struct hisp_cfg_data *pcfg = NULL;
	int ret;
	unsigned int prot;
	struct hisp_mem_info_s mem_info = { 0 };

	if (cfg == NULL) {
		cam_err("func %s: cfg is NULL", __func__);
		return -EINVAL;
	}

	ipool = find_suitable_mem_pool(cfg);
	if (ipool < 0)
		return ipool;

	mutex_lock(&kernel_rpmsg_service_mutex);
	if (s_hisp350.mem_pool[ipool].active) {
		s_hisp350.mem_pool[ipool].active = 0;
		pcfg = (struct hisp_cfg_data*)cfg;
		cam_info("%s: hisp_mem_unmap_addr ipool=%d, usage=%d", __func__, (unsigned int)ipool, pcfg->param.usage);
		ret = hisp_mem_unmap_addr(pcfg->param.usage, mem_info);
		if (ret != 0)
			cam_err("%s: hisp_mem_unmap_addr failed",
					__func__);

		if (pcfg->param.type == MAP_TYPE_DYNAMIC_SEC ||
			pcfg->param.type == MAP_TYPE_STATIC_SEC ||
			pcfg->param.type == MAP_TYPE_STATIC_ISP_SEC) {
			ret = hisp_secmem_ca_unmap((unsigned int)ipool);
			if (ret != 0)
				cam_err("%s: hisp_secmem_ca_unmap failed",
					__func__);
		}

		/* release sg_table things. */
		cam_buf_put_sgtable(s_hisp350.mem_pool[ipool].sgt);
	}
	prot = s_hisp350.mem_pool[ipool].prot; // prot should not be memset to 0
	ret = memset_s(&(s_hisp350.mem_pool[ipool]), sizeof(struct hisp350_mem_pool),
		0, sizeof(struct hisp350_mem_pool));
	if (ret != 0) {
		cam_err("memset_s failed %d", __LINE__);
		mutex_unlock(&kernel_rpmsg_service_mutex);
		return -EINVAL;
	}
	s_hisp350.mem_pool[ipool].prot = prot;
	mutex_unlock(&kernel_rpmsg_service_mutex);

	return 0;
}

static int hisp350_alloc_isp_cpu_addr(void *cfg)
{
	struct hisp_cfg_data *pcfg = NULL;
	struct sg_table *sgt = NULL;
	struct hisp_fd_map *fdmap = NULL;
	struct hisp_map_info_s map_info = { 0 };
	int ret;
	struct hisp_mem_info_s mem_info = { 0 };

	if (cfg == NULL) {
		cam_err("%s: cfg is NULL", __func__);
		return -EINVAL;
	}

	fdmap = (struct hisp_fd_map*)kzalloc(sizeof(struct hisp_fd_map),
		GFP_KERNEL);
	if (fdmap == NULL) {
		cam_err("%s: alloc fd map list node error", __func__);
		return -EINVAL;
	}

	pcfg = (struct hisp_cfg_data*)cfg;
	sgt = cam_buf_get_sgtable(pcfg->param.sharedFd);
	if (IS_ERR(sgt)) {
		cam_err("%s: fail to get sgtable", __func__);
		kfree(fdmap);
		return -ENOENT;
	}

	mutex_lock(&kernel_rpmsg_service_mutex);
	cam_info("%s: size=%x, prot=0x%x align=0x%zx, usage=%d", __func__,
		pcfg->param.size,
		pcfg->param.prot, pcfg->param.pool_align_size,
		pcfg->param.usage);
	map_info.fd = pcfg->param.sharedFd;
	map_info.size = pcfg->param.size;
	map_info.fw_mem_size = pcfg->param.map_info.fw_mem_size;
	map_info.ispfw_mem_size = pcfg->param.map_info.ispfw_mem_size;
	map_info.fw_mem_prot = pcfg->param.prot;

	ret = hisp_mem_map_addr(pcfg->param.usage, &map_info, &mem_info);
	hisp350_convert_mem_info_to_ap(&mem_info, &pcfg->param.mem_info);
	if (ret != 0) {
		cam_err("%s: hisp_alloc_cpu_map_addr failed", __func__);
		mutex_unlock(&kernel_rpmsg_service_mutex);
		cam_buf_put_sgtable(sgt);
		kfree(fdmap);
		return ret;
	}
	pcfg->param.iova = pcfg->param.mem_info.fw_mem.cpu_addr;
	/*
	 * ion iova isn't equal r8 iova, security or unsecurity, align etc
	 * return r8 iova to daemon, and send to r8 later
	 */

	mutex_unlock(&kernel_rpmsg_service_mutex);

	fdmap->sgt = sgt;
	fdmap->shared_fd = (int)pcfg->param.sharedFd;
	fdmap->iova = pcfg->param.mem_info.fw_mem.cpu_addr;
	fdmap->size = pcfg->param.size;
	fdmap->type = pcfg->param.usage;
	/* AP alloc, ion info */
	fdmap->mem_info.cpu_addr = pcfg->param.mem_info.cpu_addr;
	fdmap->mem_info.cpu_size = pcfg->param.mem_info.cpu_size;
	/* ISP Kernel alloc, fw info */
	fdmap->mem_info.fw_mem.cpu_addr = pcfg->param.mem_info.fw_mem.cpu_addr;
	fdmap->mem_info.fw_mem.core_addr = pcfg->param.mem_info.fw_mem.core_addr;
	fdmap->mem_info.fw_mem.size = pcfg->param.mem_info.fw_mem.size;
	/* ISP Kernel alloc, ispcore info */
	fdmap->mem_info.ispfw_mem.cpu_addr = pcfg->param.mem_info.ispfw_mem.cpu_addr;
	fdmap->mem_info.ispfw_mem.core_addr = pcfg->param.mem_info.ispfw_mem.core_addr;
	fdmap->mem_info.ispfw_mem.size = pcfg->param.mem_info.ispfw_mem.size;
	mutex_lock(&kernel_fd_sgt_map_mutex);
	cam_info("%s: add list shared_fd %d", __func__, fdmap->shared_fd);
	list_add(&fdmap->nd, &s_hisp350.fd_sgt_list);
	mutex_unlock(&kernel_fd_sgt_map_mutex);
	return 0;
}

static int hisp350_free_isp_cpu_addr(void *cfg)
{
	struct hisp_cfg_data *pcfg = NULL;
	int ret;
	struct hisp_fd_map *fdmap = NULL;
	struct hisp_fd_map *n = NULL;
	struct hisp_mem_info_s mem_info = { 0 };

	if (cfg == NULL) {
		cam_err("func %s: cfg is NULL", __func__);
		return -EINVAL;
	}

	mutex_lock(&kernel_rpmsg_service_mutex);
	pcfg = (struct hisp_cfg_data*)cfg;
	cam_info("%s: size=%x", __func__, pcfg->param.size);
	hisp350_convert_mem_info_from_ap(&pcfg->param.mem_info, &mem_info);
	ret = hisp_mem_unmap_addr(pcfg->param.usage, mem_info);
	if (ret != 0)
		cam_err("%s: hisp_mem_unmap_addr failed", __func__);
	mutex_unlock(&kernel_rpmsg_service_mutex);

	mutex_lock(&kernel_fd_sgt_map_mutex);
	list_for_each_entry_safe(fdmap, n, &s_hisp350.fd_sgt_list, nd) {
		if (fdmap->shared_fd == (int)pcfg->param.sharedFd) {
			cam_info("%s: find shared_fd %d to del",
				__func__, fdmap->shared_fd);
			cam_buf_put_sgtable(fdmap->sgt);
			list_del_init(&fdmap->nd);
			kfree(fdmap);
		}
	}
	mutex_unlock(&kernel_fd_sgt_map_mutex);
	return 0;
}

static int hisp350_free_isp_cpu_addr_force(void)
{
	int ret;
	struct hisp_fd_map *fdmap = NULL;
	struct hisp_fd_map *n = NULL;
	cam_warn("func %s", __func__);

	mutex_lock(&kernel_fd_sgt_map_mutex);
	list_for_each_entry_safe(fdmap, n, &s_hisp350.fd_sgt_list, nd) {
		if (fdmap->iova != 0 && fdmap->sgt != NULL) {
			cam_info("%s: find shared_fd %d to del",
				__func__, fdmap->shared_fd);
			mutex_lock(&kernel_rpmsg_service_mutex);
			ret = hisp_mem_unmap_addr(fdmap->type, fdmap->mem_info);
			if (ret != 0)
				cam_err("%s: hisp_mem_unmap_addr failed",
					__func__);
			mutex_unlock(&kernel_rpmsg_service_mutex);
			cam_buf_put_sgtable(fdmap->sgt);
			list_del_init(&fdmap->nd);
			kfree(fdmap);
		}
	}
	mutex_unlock(&kernel_fd_sgt_map_mutex);
	return 0;
}

/*
 * handle daemon carsh
 * miss ispmanager poweroff
 * miss memory pool deinit
 */
static int hisp350_deinit_r8isp_memory_pool_force(void)
{
	int ipool = 0;
	int ret = 0;
	struct hisp_mem_info_s mem_info = { 0 };
	cam_warn("func %s", __func__);

	mutex_lock(&kernel_rpmsg_service_mutex);
	for (ipool = 0; ipool < MEM_POOL_ATTR_MAX; ipool++) {
		if (s_hisp350.mem_pool[ipool].active) {
			cam_warn("%s: force deiniting pool:%d",
				__func__, ipool);
			s_hisp350.mem_pool[ipool].active = 0;
			ret = hisp_mem_unmap_addr(s_hisp350.mem_pool[ipool].usage, mem_info);
			if (ret != 0)
				cam_err("%s: hisp_mem_unmap_addr failed", __func__);
			cam_buf_put_sgtable(s_hisp350.mem_pool[ipool].sgt);
		}
		ret = memset_s(&(s_hisp350.mem_pool[ipool]),
			sizeof(struct hisp350_mem_pool),
			0,
			sizeof(struct hisp350_mem_pool));
		if (ret != 0) {
			cam_err("memset_s failed %d", __LINE__);
			mutex_unlock(&kernel_rpmsg_service_mutex);
			return -EINVAL;
		}
	}
	mutex_unlock(&kernel_rpmsg_service_mutex);
	return 0;
}

static int hisp350_mem_pool_pre_init(void)
{
	int ret = 0;
	int ipool = 0;
	int prot = 0;

	for (ipool = 0; ipool < MEM_POOL_ATTR_MAX; ipool++) {
		ret = memset_s(&(s_hisp350.mem_pool[ipool]),
			sizeof(struct hisp350_mem_pool),
			0,
			sizeof(struct hisp350_mem_pool));
		if (ret != 0) {
			cam_err("memset_s failed %d", __LINE__);
			return -EINVAL;
		}
		switch (ipool) {
		case MEM_POOL_ATTR_READ_WRITE_CACHE:
		case MEM_POOL_ATTR_READ_WRITE_CACHE_OFF_LINE:
			prot = IOMMU_READ | IOMMU_WRITE | IOMMU_CACHE;
			break;

		case MEM_POOL_ATTR_READ_WRITE_SECURITY:
		case MEM_POOL_ATTR_READ_WRITE_ISP_SECURITY:
			prot = IOMMU_READ | IOMMU_WRITE | IOMMU_CACHE |
				IOMMU_SEC;
			break;

		default:
			prot = -1;
			break;
		}

		cam_debug("%s  ipool %d prot 0x%x", __func__, ipool, prot);

		if (prot < 0) {
			cam_err("%s unkown ipool %d prot 0x%x",
				__func__, ipool, prot);
			return -EINVAL;
		}

		s_hisp350.mem_pool[ipool].prot = (unsigned int)prot;
	}

	return 0;
}

static int hisp350_mem_pool_later_deinit(void)
{
	int ipool = 0;
	int ret = 0;
	struct hisp_mem_info_s mem_info = { 0 };
	cam_debug("%s", __func__);

	for (ipool = 0; ipool < MEM_POOL_ATTR_MAX; ipool++) {
		if (ipool == MEM_POOL_ATTR_READ_WRITE_CACHE_OFF_LINE)
			continue;
		if (s_hisp350.mem_pool[ipool].active) {
			cam_warn("%s: force deiniting pool:%d",
				__func__, ipool);
			s_hisp350.mem_pool[ipool].active = 0;
			ret = hisp_mem_unmap_addr(s_hisp350.mem_pool[ipool].usage, mem_info);
			if (ret != 0)
				cam_err("%s: hisp_mem_unmap_addr failed", __func__);
			cam_buf_put_sgtable(s_hisp350.mem_pool[ipool].sgt);
		}
		ret = memset_s(&s_hisp350.mem_pool[ipool],
			sizeof(struct hisp350_mem_pool),
			0,
			sizeof(struct hisp350_mem_pool));
		if (ret != 0) {
			cam_err("memset_s failed %d", __LINE__);
			return -EINVAL;
		}
	}

	return 0;
}

struct isp_clk_pair {
	const char *name;
	uint32_t ispfunc;
	uint32_t ispfunc2;
	uint32_t isp_cpu_data;
};

#ifdef CAM_CHIP_TYPE_ES // chip_type=es
static struct isp_clk_pair ispclks[] = {
	/* name,      ispfunc,   ispfunc2 */
	{ "turbo",    642000000, 642000000, 0 },
	{ "normal",   480000000, 558000000, 0 },
	{ "lowpower", 335000000, 335000000, 0 },
	{ "ultralow", 209000000, 209000000, 0 },
};
#else // chip_type=cs
#ifdef CONFIG_KERNEL_CAMERA_ISPV360
static struct isp_clk_pair ispclks[] = {
	/* name,      ispfunc, ispfunc2 isp_cpu_data(cpro) */
	{ "turbo",    720000000, 0, 720000000 },
	{ "normal",   558000000, 0, 642000000 },
	{ "lowpower", 335000000, 0, 335000000 },
	{ "ultralow", 335000000, 0, 335000000 },
};
#else
static struct isp_clk_pair ispclks[] = {
	/* name,      ispfunc,   ispfunc2 */
	{ "turbo",    720000000, 0, 0 },
	{ "normal",   558000000, 0, 0 },
	{ "lowpower", 418000000, 0, 0 },
	{ "ultralow", 335000000, 0, 0 },
};
#endif
#endif

static int hisp350_set_clk_rate(int clk_level)
{
	int ret0 = 0;
	int ret1 = 0;
	int ret2 = 0;
	struct isp_clk_pair *ispclk = NULL;

	if (clk_level >= (int)ARRAY_SIZE(ispclks)) {
		cam_warn("voting invalid level: %d", clk_level);
		return -EINVAL;
	}

	ispclk = &ispclks[clk_level];
	if (ispclk->ispfunc != 0) {
		ret0 = hisp_set_clk_rate(ISPFUNC_CLK, ispclk->ispfunc);
#ifdef CONFIG_KERNEL_CAMERA_ISPV360
		ret2 = hisp_set_clk_rate(ISPCPU_DATA_CLK, ispclk->isp_cpu_data);
#endif
	}
	if (ispclk->ispfunc2 != 0)
		ret1 = hisp_set_clk_rate(ISPFUNC2_CLK, ispclk->ispfunc2);

	cam_debug("%s: set clk %s", __func__, ispclk->name);
	if (ret0 < 0 || ret1 < 0 || ret2 < 0) {
		cam_err("%s: set clk fail, ret:%d, %d, %d", __func__, ret0, ret1, ret2);
		return -EFAULT;
	}
	return 0;
}

static int hisp350_set_clk_rate_self_adapt(int clk_level)
{
	int rc = 0;
	int tmp_rc = 0;

	do {
		rc = hisp350_set_clk_rate(clk_level);
		if (rc == 0)
			break;
		tmp_rc = rc;
		cam_info("%s: set to clk level:%d fail, try level:%d", __func__,
			 clk_level, clk_level + 1);
		clk_level += 1; /* attention: plus one for a lower clk level. */
	} while (clk_level < ISP_CLK_LEVEL_MAX);

	return tmp_rc;
}

#ifdef CONFIG_KERNEL_CAMERA_ISPV360
const char *pg_chip_need_cut_csi[] = {
	"level2_cdc",
	"lite_cdc",
};

static bool bypass_csi_by_soc_spec(void) /* pg chip need cut csi according to dts prop value */
{
	const char *soc_spec = NULL;
	int ret;
	unsigned int i;
	struct device_node *np = of_find_compatible_node(NULL, NULL, "hisilicon, soc_spec");
	if (np == NULL) {
		cam_err("%s: of_find_compatible_node fail or normal type chip\n", __func__);
		return false;
	}

	ret = of_property_read_string(np, "soc_spec_set", &soc_spec);
	if (ret < 0) {
		cam_err("%s: read string fail\n", __func__);
		return false;
	}

	for (i = 0; i < sizeof(pg_chip_need_cut_csi) / sizeof(pg_chip_need_cut_csi[0]); ++i) {
		ret = strncmp(soc_spec, pg_chip_need_cut_csi[i], strlen(pg_chip_need_cut_csi[i]));
		if (ret == 0) {
			cam_info("%s: this is pg chip:%s, need cut csi\n", __func__, pg_chip_need_cut_csi[i]);
			return true;
		}
	}

	cam_info("%s: no cut csi\n", __func__);
	return false;
}

static int check_phy_bypass_by_efuse(uint32_t phy_id)
{
	int rc = 0;
	uint32_t val = 0;
	uint32_t csi_efuse_offset = CHIP_LEVEL_EFUSE_FLAG_CSI_START -
	    CHIP_LEVEL_EFUSE_PARTIALGOOD_FLAG8_START; /* 850 according to 360 pg csi efuse design. */
	struct efuse_desc edes = { 0 };
	/* phyA C D E(0 2 3 4) match index 0 1 2 3 of efuse 850~853 */
	uint32_t index = phy_id > 0 ? (phy_id - 1) : phy_id;
	uint32_t vefuse = 0;
	bool need_cut_csi = bypass_csi_by_soc_spec();
	edes.buf = &val;
	edes.buf_size = 1;
	edes.start_bit = csi_efuse_offset;
	edes.bit_cnt = CHIP_LEVEL_EFUSE_FLAG_CSI_SIZE;
	edes.item_vid = EFUSE_KERNEL_PARTIALGOOD_FLAG8;
	rc = efuse_read_value_t(&edes);
	if (rc == EFUSE_OK) {
		cam_info("%s get efuse val:%u", __func__, val);
		if (val == 0) {
			cam_info("no efuse set, regard as bin one");
			return 0;
		}
		if (val == 15 && need_cut_csi) { /* 15 means all csi efuse is 1 */
			if (phy_id == CUT_PHY_INDEX) {
				cam_info("pg chip cut csi 1");
				return 1;
			}
		}
		vefuse = (val >> index) & 1;
		if (vefuse != 1) {
			cam_err("%s efuse bit val:%u indicates phy:%u not ok", __func__, vefuse, phy_id);
			return 1;
		}
	} else {
		cam_err("%s efuse check func fail, not bypass phy:%u", __func__, phy_id);
	}
	return 0;
}
#endif

static int hisp350_phy_csi_connect(void *pdata)
{
	int rc = 0;
	struct phy_csi_connect_info *conn;

	conn = pdata;
	if (!conn) {
		cam_err("%s: data is NULL", __func__);
		return -1;
	}
#ifdef CONFIG_KERNEL_CAMERA_ISPV360
	rc = check_phy_bypass_by_efuse(conn->phy_info.phy_id);
	if (rc != 0) {
		cam_err("%s: phy:%u need bypass %d", __func__, conn->phy_info.phy_id, rc);
		return rc;
	}
#endif
	WARN_ON(sizeof(struct hisp_phy_info_t) != sizeof(conn->phy_info));
	rc = hisp_phy_csi_connect((struct hisp_phy_info_t *)&conn->phy_info,
		conn->csi_index);
	if (rc != 0)
		cam_err("%s: phy csi connect fail:%d", __func__, rc);
	return rc;
}

static int hisp350_config_power_on(hisp_intf_t *i, struct hisp_cfg_data *pcfg)
{
	int rc = 0;
	int client_id = hisp_get_client_id(current->tgid);
	mutex_lock(&hisp_power_lock_mutex);
	if (hisp350_remote_processor_up()) {
		cam_warn("%s hisp350 is still on power-on state", __func__);
		if (client_id >= 0 && client_id < MAX_HISP_CLIENT)
			remote_processor_up[client_id] = true;
		mutex_unlock(&hisp_power_lock_mutex);
		return rc;
	}

	if (pcfg->isSecure == 0) {
		hisp_set_boot_mode(NONSEC_CASE);
	} else if (pcfg->isSecure == 1) {
		hisp_set_boot_mode(SEC_CASE);
	} else {
		cam_info("%s invalid mode", __func__);
	}
	cam_notice("%s power on the hisp350", __func__);
	rc = hisp350_power_on(i);

	mutex_unlock(&hisp_power_lock_mutex);
	return rc;
}

static int hisp350_config_power_off(hisp_intf_t *i)
{
	int rc = 0;
	mutex_lock(&hisp_power_lock_mutex);
	if (hisp350_remote_processor_up()) {
		cam_notice("%s power off the hisp350", __func__);
		rc = hisp350_power_off(i);
	}
	mutex_unlock(&hisp_power_lock_mutex);
	return rc;
}

static int hisp350_config_op_isp(void *cfg)
{
	int rc = 0;
	struct hisp_cfg_data *pcfg = (struct hisp_cfg_data*)(cfg);

	switch (pcfg->cfgtype) {
	case HISP_CONFIG_ISP_TURBO:
		cam_debug("%s HISP_CONFIG_ISP_TURBO", __func__);
		rc = hisp350_set_clk_rate_self_adapt(ISP_CLK_LEVEL_TURBO);
		break;
	case HISP_CONFIG_ISP_NORMAL:
		cam_debug("%s HISP_CONFIG_ISP_NORMAL", __func__);
		rc = hisp350_set_clk_rate_self_adapt(ISP_CLK_LEVEL_NORMAL);
		break;
	case HISP_CONFIG_ISP_LOWPOWER:
		cam_debug("%s HISP_CONFIG_ISP_LOWPOWER", __func__);
		rc = hisp350_set_clk_rate_self_adapt(ISP_CLK_LEVEL_LOWPOWER);
		break;
	case HISP_CONFIG_ISP_ULTRALOW:
		cam_debug("%s HISP_CONFIG_ISP_ULTRALOW", __func__);
		rc = hisp350_set_clk_rate_self_adapt(ISP_CLK_LEVEL_ULTRALOW);
		break;
	case HISP_CONFIG_R8_TURBO:
		cam_debug("%s HISP_CONFIG_R8_TURBO", __func__);
		rc = hisp_set_clk_rate(ISPCPU_CLK, R8_TURBO_ISPCPU_CLK_RATE);
		if (rc < 0) { // some v350 chips may not match 1440M
			cam_debug("%s r8 set turbo 1440M fail, then set normal 964M", __func__);
			rc = hisp_set_clk_rate(ISPCPU_CLK,
				R8_NORMAL_ISPCPU_CLK_RATE);
		}
		break;
	case HISP_CONFIG_R8_NORMAL:
		cam_debug("%s HISP_CONFIG_R8_NORMAL", __func__);
		rc = hisp_set_clk_rate(ISPCPU_CLK, R8_NORMAL_ISPCPU_CLK_RATE);
		break;
	case HISP_CONFIG_R8_LOWPOWER:
		cam_debug("%s HISP_CONFIG_R8_LOWPOWER", __func__);
		rc = hisp_set_clk_rate(ISPCPU_CLK, R8_LOWPOWER_ISPCPU_CLK_RATE);
		break;
	case HISP_CONFIG_R8_ULTRALOW:
		cam_debug("%s HISP_CONFIG_R8_ULTRALOW", __func__);
		rc = hisp_set_clk_rate(ISPCPU_CLK, R8_ULTRALOW_ISPCPU_CLK_RATE);
		break;
	case HISP_CONFIG_GET_SEC_ISPFW_SIZE:
		rc = hisp_secboot_memsize_get_from_type(
			pcfg->secMemType, &pcfg->buf_size);
		break;
	case HISP_CONFIG_SET_SEC_ISPFW_BUFFER:
		rc = hisp350_set_sec_fw_buffer(cfg);
		break;
	case HISP_CONFIG_RELEASE_SEC_ISPFW_BUFFER:
		rc = hisp350_release_sec_fw_buffer();
		break;
	case HISP_CONFIG_SECBOOT_PREPARE:
		rc = hisp_secboot_prepare();
		break;
	case HISP_CONFIG_SECBOOT_UNPREPARE:
		rc = 0;
		break;
	default:
		cam_err("%s: unsupported cmd:%#x", __func__, pcfg->cfgtype);
		break;
	}
	return rc;
}

static int hisp350_config_op(hisp_intf_t *i, void *cfg, int* pflag)
{
	int rc = 0;
	struct hisp_cfg_data *pcfg = (struct hisp_cfg_data*)(cfg);
	*pflag = 0;
	switch (pcfg->cfgtype) {
	case HISP_CONFIG_POWER_ON:
		rc = hisp350_config_power_on(i, pcfg);
		break;
	case HISP_CONFIG_POWER_OFF:
		rc = hisp350_config_power_off(i);
		break;
	case HISP_CONFIG_INIT_MEMORY_POOL:
		rc = hisp350_init_r8isp_memory_pool(cfg);
		break;
	case HISP_CONFIG_DEINIT_MEMORY_POOL:
		rc = hisp350_deinit_r8isp_memory_pool(cfg);
		break;
	case HISP_CONFIG_PROC_TIMEOUT: {
		int client_id = hisp_get_client_id(current->tgid);
		unsigned int message_id = pcfg->cfgdata[0] | (client_id << HISP_MESSAGE_ID_CLIENT_BIT);
		cam_info("%s timeout message_id.0x%x", __func__, message_id);
		hisp_dump_rpmsg_with_id(message_id);
		if (client_id >= 0 && client_id < MAX_HISP_CLIENT)
			dump_isp_ack_ids(rpmsg_local.msg_pool[client_id]);
		break;
	}
	case HISP_CONFIG_SET_MDC_BUFFER:
		rc = hisp_set_mdc_buffer(cfg);
		break;
	case HISP_CONFIG_RELEASE_MDC_BUFFER:
		rc = hisp_release_mdc_buffer();
		break;
	case HISP_CONFIG_PHY_CSI_CONNECT:
		rc = hisp350_phy_csi_connect((void*)(pcfg->cfgdata));
		break;
	case HISP_CONFIG_LOCK_VOLTAGE:
		cam_warn("%s: lock voltage need develop!", __func__);
		break;
	case HISP_CONFIG_ALLOC_ISP_CPU_MEM:
		rc = hisp350_alloc_isp_cpu_addr(cfg);
		break;
	case HISP_CONFIG_FREE_ISP_CPU_MEM:
		rc = hisp350_free_isp_cpu_addr(cfg);
		break;
#if defined(CONFIG_SENSOR_DIRECT_CHANNEL)
	case HISP_CONFIG_INIT_IMU_BUFFER:
		rc = hisp350_init_imu_buffer(cfg);
		break;
	case HISP_CONFIG_DEINIT_IMU_BUFFER:
		rc = hisp350_deinit_imu_buffer(cfg);
		break;
#endif
	default:
		*pflag = 1; /* need continue to judge */
		break;
	}
	return rc;
}

static int hisp350_config(hisp_intf_t *i, void *cfg)
{
	int rc = 0;
	int flag = 0;
	hisp350_t *hi = NULL;
	struct hisp_cfg_data *pcfg = NULL;
	hisp_assert(i != NULL);
	if (cfg == NULL) {
		cam_err("func %s: cfg is NULL", __func__);
		return -1;
	}
	pcfg = (struct hisp_cfg_data*)cfg;
	hi = to_isp(i);
	hisp_assert(hi != NULL);

	rc = hisp350_config_op(i, cfg, &flag);
	if (flag == 1)
		rc = hisp350_config_op_isp(cfg);

	if (rc < 0)
		cam_err("%s: cmd:%#x fail, rc:%u", __func__, pcfg->cfgtype, rc);
	return rc;
}

static int hisp350_open(hisp_intf_t *i)
{
	(void)i;
	cam_info("%s hisp350 device open", __func__);

	mutex_lock(&hisp_power_lock_mutex);
	g_hisp_ref++;
	mutex_unlock(&hisp_power_lock_mutex);
	return 0;
}

static int hisp350_close(hisp_intf_t *i)
{
	int rc = 0;
	bool last_one = hisp_get_client_refs(current->tgid) == 1;
	cam_info("%s hisp350 device close", __func__);
	mutex_lock(&hisp_power_lock_mutex);

	if (g_hisp_ref)
		g_hisp_ref--;

	if (last_one && hisp350_remote_processor_up()) {
		cam_warn("%s hisp350 is still on power-on state, power off it",
			__func__);

		rc = hisp350_power_off(i);
		if (rc != 0)
			cam_err("failed to hisp350 power off");

		if (!hisp350_remote_processor_up()) {
			hisp350_deinit_r8isp_memory_pool_force();
			hisp350_free_isp_cpu_addr_force();
		}
	}

	if (g_hisp_ref == 0) {
		hisp350_deinit_isp_mem();
		cam_buf_close_security_ta();
	}

	mutex_unlock(&hisp_power_lock_mutex);
	return rc;
}

static int hisp350_pinctrl_select_state(struct pinctrl *p, struct pinctrl_state *state)
{
	int rc;

	mutex_lock(&hisp_pinctrl_lock_mutex);
	rc = pinctrl_select_state(p, state);
	mutex_unlock(&hisp_pinctrl_lock_mutex);

	return rc;
}

static int hisp350_power_on_rpproc(hisp350_t *hi)
{
	int rc = 0;
	if (hi == NULL)
		goto FAILED_RET;

	if ((!hw_is_fpga_board()) && (!IS_ERR(hi->dt.pinctrl_default))) {
		rc = hisp350_pinctrl_select_state(hi->dt.pinctrl,
			hi->dt.pinctrl_default);
		if (rc != 0) {
			cam_err("pinctrl select state fail");
			goto FAILED_RET;
		}
	}

	hisp_rpmsgrefs_reset();
	rc = hisp_rproc_enable();

FAILED_RET:
	return rc;
}

static int hisp350_power_on(hisp_intf_t *i)
{
	int rc = 0;
	bool rproc_enabled = false;
	bool hi_opened = false;
	hisp350_t *hi = NULL;
	int client_id = hisp_get_client_id(current->tgid);
	unsigned long current_jiffies = jiffies;
	uint32_t timeout = hw_is_fpga_board() ?
		TIMEOUT_IS_FPGA_BOARD : TIMEOUT_IS_NOT_FPGA_BOARD;

	if (i == NULL)
		return -1;
	if (client_id < 0 || client_id >= MAX_HISP_CLIENT)
		return -1;
	hi = to_isp(i);

	cam_info("%s enter ... ", __func__);

	mutex_lock(&hisp_wake_lock_mutex);
	if (!hisp_power_wakelock->active) {
		__pm_stay_awake(hisp_power_wakelock);
		cam_info("%s hisp power on enter, wake lock", __func__);
	}
	mutex_unlock(&hisp_wake_lock_mutex);

	mutex_lock(&kernel_rpmsg_service_mutex);
	if (!atomic_read((&hi->opened))) {
		reinit_completion(&rpmsg_local.isp_comp);
		rc = hisp350_power_on_rpproc(hi);
		if (rc != 0)
			goto FAILED_RET;
		rproc_enabled = true;

		rc = (int)wait_for_completion_timeout(&rpmsg_local.isp_comp,
			msecs_to_jiffies(timeout));
		if (rc == 0) {
			rc = -ETIME;
			hisp_boot_stat_dump();
			cam_err("wait completion timeout");
			goto FAILED_RET;
		}
		cam_info("%s() %d after wait completion, rc = %d",
			__func__, __LINE__, rc);
		rc = 0;

		atomic_inc(&hi->opened);
		hi_opened = true;
	} else {
		cam_notice("%s isp has been opened", __func__);
	}
	remote_processor_up[client_id] = true;
	if (!rpmsg_local.rpdev) { // rpmsg_device not probed
		rc = -ENODEV;
		cam_err("%s: remote processor not connected", __func__);
		goto FAILED_RET;
	}

	if (hisp350_mem_pool_pre_init()) {
		cam_err("failed to pre init mem pool ");
		rc = -ENOMEM;
		goto FAILED_RET;
	}

	mutex_unlock(&kernel_rpmsg_service_mutex);
	cam_info("%s exit ,power on time:%d... ", __func__,
		jiffies_to_msecs(jiffies - current_jiffies));
	return rc;

FAILED_RET:
	if (hi_opened)
		atomic_dec(&hi->opened);

	if (rproc_enabled)
		hisp_rproc_disable();

	hisp350_mem_pool_later_deinit();
	remote_processor_up[client_id] = false;

	mutex_unlock(&kernel_rpmsg_service_mutex);

	mutex_lock(&hisp_wake_lock_mutex);
	if (hisp_power_wakelock->active) {
		__pm_relax(hisp_power_wakelock);
		cam_err("%s hisp power on failed, wake unlock", __func__);
	}
	mutex_unlock(&hisp_wake_lock_mutex);
	return rc;
}

static int hisp350_power_off(hisp_intf_t *i)
{
	int rc = 0;
	hisp350_t *hi = NULL;
	unsigned long current_jiffies = jiffies;
	int client_id = hisp_get_client_id(current->tgid);
	struct hisp_mem_info_s mem_info;

	if (i == NULL)
		return -1;
	if (client_id < 0 || client_id >= MAX_HISP_CLIENT)
		return -1;
	hi = to_isp(i);

	cam_info("%s enter ... ", __func__);

	/* check the remote processor boot flow */
	remote_processor_up[client_id] = false;
	if (hisp350_remote_processor_up()) {
		rc = 0;
		goto RET;
	}

	mutex_lock(&kernel_rpmsg_service_mutex);
	if (atomic_read((&hi->opened))) {
		hisp_phy_csi_disconnect();
		hisp_rproc_disable();
		if (!hw_is_fpga_board()) {
			if (!IS_ERR(hi->dt.pinctrl_idle)) {
				rc = hisp350_pinctrl_select_state(hi->dt.pinctrl,
					hi->dt.pinctrl_idle);
				if (rc != 0) {
					// Empty.
				}
			}
		}
		atomic_dec(&hi->opened);
	} else {
		cam_notice("%s isp hasn't been opened", __func__);
	}
	hisp350_mem_pool_later_deinit();
#if defined(CONFIG_SENSOR_DIRECT_CHANNEL)
	if (imu_map_ref != 0) {
		mem_info.cpu_addr = imu_map_param.iova;
		mem_info.cpu_size = imu_map_param.size;
		rc = hisp_mem_unmap_addr(imu_map_param.usage, mem_info);
		imu_map_ref = 0;
		imu_map_param.iova = 0;
		imu_map_param.size = 0;
		sdc_buf_put(imu_info);
		imu_info = NULL;
	}
#endif
	mutex_unlock(&kernel_rpmsg_service_mutex);
RET:
	cam_info("%s exit ,power 0ff time:%d... ", __func__,
		jiffies_to_msecs(jiffies - current_jiffies));

	mutex_lock(&hisp_wake_lock_mutex);
	if (hisp_power_wakelock->active) {
		__pm_relax(hisp_power_wakelock);
		cam_info("%s hisp power off exit, wake unlock", __func__);
	}
	mutex_unlock(&hisp_wake_lock_mutex);
	return rc;
}

void hisp_clear_msgs(int client_id)
{
	if (client_id >= 0 && client_id < MAX_HISP_CLIENT)
		clear_isp_msg_pool(rpmsg_local.msg_pool[client_id]);
}

static void hisp350_rpmsg_remove(struct rpmsg_device *rpdev)
{
	unsigned int i;
	cam_info("%s enter ... ", __func__);
	hisp_rpmsgrefs_dump(); // dump rpmsg refs when power off'ed
	for (i = 0; i < MAX_HISP_CLIENT; ++i)
		clear_isp_msg_pool(rpmsg_local.msg_pool[i]); // release isp msg nodes

	mutex_lock(&rpmsg_local.sendlock);
	if (rpdev != rpmsg_local.rpdev)
		cam_warn("%s: removing rpdev not same as probed", __func__);
	rpmsg_local.rpdev = NULL;
	mutex_unlock(&rpmsg_local.sendlock);
	cam_info("%s: rpmsg kernel driver is removed", __func__);
}

#define RPMSG_MAX_PAYLOAD 512
struct isp_msg_holder {
	// rpmsg_hdr takes away some bytes
	uint8_t data[RPMSG_MAX_PAYLOAD - sizeof(struct rpmsg_hdr)];
};

static int hisp350_power_off_rpmsg(struct isp_msg_header* msg_hdr, uint32_t len, int client_id)
{
#define COMMAND_ISP_CPU_POWER_OFF_REQUEST 4127
	int rc;
	if (!msg_hdr || msg_hdr->api_name != COMMAND_ISP_CPU_POWER_OFF_REQUEST)
		return -EINVAL;
	if (client_id >= MAX_HISP_CLIENT || client_id < 0) {
		cam_err("func %s: client id is %d", __func__, client_id);
		return -EINVAL;
	}

	mutex_lock(&hisp_power_lock_mutex);
	if (hisp350_remote_processor_up() <= 1) {
		mutex_unlock(&hisp_power_lock_mutex);
		return -1;
	}

	cam_warn("two daemon run, skip power off rpmsg");
	rc = queue_isp_msg(rpmsg_local.msg_pool[client_id], msg_hdr, len);
	if (rc == 0)
		hisp350_notify_rpmsg_cb(client_id);

	remote_processor_up[client_id] = false;

	mutex_unlock(&hisp_power_lock_mutex);
	return 0;
}

static int hisp350_send_rpmsg(hisp_intf_t *itf, struct isp_indirect_msg *imsg, size_t len)
{
	int rc = 0;
	struct isp_msg_header *msg_hdr = NULL;
	struct isp_msg_holder msg_data = {};
	int client_id = hisp_get_client_id(current->tgid);

	(void)itf;
	if (!imsg || len < sizeof(*imsg)) {
		cam_err("%s: itf or imsg NULL, or len:%zu", __func__, len);
		return -EINVAL;
	}
	if (!imsg->isp_msg || imsg->msg_size > sizeof(msg_data)) {
		cam_err("%s: isp_msg NULL or msg_size:%u too big, max:%zu",
			__func__, len, sizeof(msg_data));
		return -EINVAL;
	}
	if (client_id >= MAX_HISP_CLIENT || client_id < 0) {
		cam_err("func %s: client id is %d", __func__, client_id);
		return -EINVAL;
	}

	rc = copy_from_user(&msg_data, (void __user *)imsg->isp_msg, imsg->msg_size);
	if (rc) {
		cam_err("%s: copy msg from user fail, len:%u",
			__func__, imsg->msg_size);
		return -EFAULT;
	}

	msg_hdr = (struct isp_msg_header *)&msg_data;
	if (!hisp350_power_off_rpmsg(msg_hdr, imsg->msg_size, client_id))
		return rc;

	msg_hdr->message_id |= (client_id << HISP_MESSAGE_ID_CLIENT_BIT);
	cam_debug("%s: client_id:%d, api_name:%#x, message_id:%#x", __func__,
		client_id, msg_hdr->api_name, msg_hdr->message_id);

	hisp_sendin(&msg_data);
	mutex_lock(&rpmsg_local.sendlock); // protect against async remove
	if (rpmsg_local.rpdev)
		rc = rpmsg_send(rpmsg_local.rpdev->ept,
			(void *)&msg_data, imsg->msg_size);
	else
		rc = -ENODEV;
	mutex_unlock(&rpmsg_local.sendlock);
	if (rc)
		cam_err("%s: rpmsg_send ret is %d", __func__, rc);
	return rc;
}

static int hisp350_recv_rpmsg(hisp_intf_t *itf, struct isp_indirect_msg *imsg, size_t len)
{
	int rc;
	struct isp_msg_header msg_hdr;
	int client_id = hisp_get_client_id(current->tgid);

	(void)itf;
	if (!imsg || len < sizeof(*imsg)) {
		cam_err("%s: user imsg NULL or len:%u", __func__, len);
		return -EINVAL;
	}
	if (!imsg->isp_msg) {
		cam_err("%s: user imsg->isp_msg NULL", __func__);
		return -EINVAL;
	}

	if (client_id >= MAX_HISP_CLIENT || client_id < 0) {
		cam_err("func %s: client id is %d", __func__, client_id);
		return -EINVAL;
	}

	rc = dequeue_isp_msg(rpmsg_local.msg_pool[client_id], imsg->isp_msg,
		imsg->msg_size, &msg_hdr);
	if (rc > 0)
		hisp_recvdone((void *)&msg_hdr);

	cam_debug("%s: client_id:%d, recved api_name:%#x, message_id:%#x, rc:%d", __func__,
		client_id, msg_hdr.api_name, msg_hdr.message_id, rc);
	return rc;
}

static int hisp350_set_sec_fw_buffer(struct hisp_cfg_data *cfg)
{
	int rc;
	mutex_lock(&hisp_mem_lock_mutex);
	rc = hisp_set_sec_fw_buffer(cfg);
	if (rc < 0)
		cam_err("%s: fail, rc:%d", __func__, rc);

	if (s_hisp350.mem.active) {
		s_hisp350.mem.active = 0;
		dma_buf_put(s_hisp350.mem.dmabuf);
	}

	s_hisp350.mem.dmabuf = dma_buf_get(cfg->share_fd);
	if (IS_ERR_OR_NULL(s_hisp350.mem.dmabuf)) {
		cam_err("Fail: dma buffer error");
		mutex_unlock(&hisp_mem_lock_mutex);
		return -EFAULT;
	}
	s_hisp350.mem.active = 1;
	mutex_unlock(&hisp_mem_lock_mutex);
	return rc;
}

static int hisp350_release_sec_fw_buffer(void)
{
	int rc;
	int ret;
	mutex_lock(&hisp_mem_lock_mutex);
	rc = hisp_release_sec_fw_buffer();
	if (rc < 0)
		cam_err("%s: fail, rc:%d", __func__, rc);

	if (s_hisp350.mem.active) {
		s_hisp350.mem.active = 0;
		dma_buf_put(s_hisp350.mem.dmabuf);
	}
	ret = memset_s(&(s_hisp350.mem),
		sizeof(struct isp_mem),
		0,
		sizeof(struct isp_mem));
	if (ret != 0)
		cam_warn("%s: fail for memset_s mem_pool", __func__);
	mutex_unlock(&hisp_mem_lock_mutex);
	return rc;
}
static void hisp350_deinit_isp_mem(void)
{
	int ret;
	cam_info("func %s", __func__);
	mutex_lock(&hisp_mem_lock_mutex);
	if (s_hisp350.mem.active) {
		cam_err("sec isp ex,put dmabuf");
		s_hisp350.mem.active = 0;
		dma_buf_put(s_hisp350.mem.dmabuf);
	}

	ret = memset_s(&(s_hisp350.mem),
		sizeof(struct isp_mem),
		0,
		sizeof(struct isp_mem));
	if (ret != 0)
		cam_warn("%s: fail for memset_s mem_pool", __func__);
	mutex_unlock(&hisp_mem_lock_mutex);
	return;
}

#ifdef CONFIG_DFX_DEBUG_FS
static void hisp350_set_ddrfreq(int ddr_bandwidth)
{
	cam_info("%s enter,ddr_bandwidth:%d", __func__, ddr_bandwidth);
}

static void hisp350_release_ddrfreq(void)
{
	cam_info("%s enter", __func__);
	if (current_ddr_bandwidth == 0)
		return;
	cpu_latency_qos_remove_request(&qos_request_ddr_down_record);
	current_ddr_bandwidth = 0;
}

static void hisp350_update_ddrfreq(int ddr_bandwidth)
{
	cam_info("%s enter, ddr_bandwidth:%u", __func__, ddr_bandwidth);
	if (!atomic_read(&s_hisp350.opened)) {
		cam_info("%s: cam is not opened, can not set ddr bandwidth", __func__);
		return;
	}

	if (current_ddr_bandwidth == 0) {
		hisp350_set_ddrfreq(ddr_bandwidth);
	} else if (current_ddr_bandwidth > 0) {
		cpu_latency_qos_update_request(&qos_request_ddr_down_record,
			ddr_bandwidth);
		current_ddr_bandwidth = ddr_bandwidth;
	} else {
		cam_err("%s,current_ddr_bandwidth is invalid", __func__);
	}
}

static ssize_t hisp_ddr_freq_ctrl_show(struct device *dev,
	struct device_attribute *attr,char *buf)
{
	(void)dev;
	(void)attr;
	cam_info("enter %s,current_ddr_bandwidth:%d",
		__func__, current_ddr_bandwidth);

	return scnprintf(buf, PAGE_SIZE, "%d", current_ddr_bandwidth);
}

static ssize_t hisp_ddr_freq_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	long ddr_bandwidth = 0;

	(void)dev;
	(void)attr;
	if (buf == NULL) {
		cam_err("%s,input buffer is invalid", __func__);
		return -EINVAL;
	}

	if (kstrtol(buf, 10, &ddr_bandwidth)) {
		cam_err("%s: input buffer can not be parsed", __func__);
		return -EINVAL;
	}

	if (ddr_bandwidth < 0) {
		cam_err("%s: ddr_bandwidth is invalid", __func__);
		return -EINVAL;
	}

	cam_info("%s: ddr_bandwidth:%ld", __func__, ddr_bandwidth);
	if (ddr_bandwidth == 0)
		hisp350_release_ddrfreq();
	else
		hisp350_update_ddrfreq(ddr_bandwidth);

	return count;
}
#endif

int hisp_extra_pinctrl_index(const char *name)
{
	int i;
	struct extra_pinctrl *ep = s_hisp350.dt.extra_pinctrl;

	if (!name)
		return -EINVAL;

	for (i = 0; i < (int)ARRAY_SIZE(s_hisp350.dt.extra_pinctrl); ++i)
		if (ep->name && strcmp(ep->name, name) == 0)
			return i;
	return -1;
}

int hisp_extra_pinctrl_select_state(int idx, const char *state)
{
	struct extra_pinctrl *ep = s_hisp350.dt.extra_pinctrl;

	if (idx < 0 || idx >= (int)ARRAY_SIZE(s_hisp350.dt.extra_pinctrl) || !state)
		return -EINVAL;

	if (strcmp(state, "default") == 0)
		return hisp350_pinctrl_select_state(s_hisp350.dt.pinctrl, ep[idx].pinctrl_default);
	if (strcmp(state, "idle") == 0)
		return hisp350_pinctrl_select_state(s_hisp350.dt.pinctrl, ep[idx].pinctrl_idle);
	return -1;
}

static int32_t hisp350_rpmsg_probe(struct rpmsg_device *rpdev)
{
	cam_info("%s enter", __func__);
	if (rpmsg_local.rpdev) {
		cam_warn("%s rpdev is already up", __func__);
		return -EBUSY; // if need support multi rpmsg_device, need revise
	}
	if (!rpdev || !rpdev->ept) {
		cam_err("%s: rpdev NULL or rpdev->ept not created!", __func__);
		return -ENOENT;
	}

	rpmsg_local.rpdev = rpdev;
	dev_set_drvdata(&rpdev->dev, &rpmsg_local);

	complete(&rpmsg_local.isp_comp);
	cam_info("new KERNEL connection srv channel: %u -> %u",
		rpdev->src, rpdev->dst);
	return 0;
}

static struct rpmsg_device_id rpmsg_hisp350_id_table[] = {
	{.name = "rpmsg-isp"},
	{},
};

MODULE_DEVICE_TABLE(platform, rpmsg_hisp350_id_table);

static const struct of_device_id s_hisp350_dt_match[] = {
	{
		.compatible = "vendor,chip_isp350",
		.data = &s_hisp350.intf,
	},
	{},
};

MODULE_DEVICE_TABLE(of, s_hisp350_dt_match);
#pragma GCC visibility push(default)
static struct rpmsg_driver rpmsg_hisp350_driver = {
	.drv.name = KBUILD_MODNAME,
	.drv.owner = THIS_MODULE,
	.id_table = rpmsg_hisp350_id_table,
	.probe = hisp350_rpmsg_probe,
	.callback = hisp350_rpmsg_ept_cb,
	.remove = hisp350_rpmsg_remove,
};
#pragma GCC visibility pop

#ifdef CONFIG_DFX_DEBUG_FS
static struct device_attribute hisp_ddr_freq_ctrl_attr =
	__ATTR(ddr_freq_ctrl, 0660, /* 0660 for ATTR mode */
	hisp_ddr_freq_ctrl_show, hisp_ddr_freq_store);
#endif /* CONFIG_DFX_DEBUG_FS */

static int32_t hisp350_platform_probe(struct platform_device* pdev)
{
	int32_t ret = 0;
	unsigned int i;
	cam_info("%s: enter", __func__);
	hisp_power_wakelock = wakeup_source_register(&pdev->dev, "hisp_power_wakelock");
	if (!hisp_power_wakelock) {
		cam_err("%s: wakeup source register failed", __func__);
		return -1;
	}
	mutex_init(&hisp_wake_lock_mutex);
	mutex_init(&hisp_power_lock_mutex);
	mutex_init(&hisp_mem_lock_mutex);
	mutex_init(&rpmsg_local.sendlock);
	init_completion(&rpmsg_local.isp_comp);

	ret = hisp_get_dt_data(pdev, &s_hisp350.dt);
	if (ret < 0) {
		cam_err("%s: get dt failed", __func__);
		goto error;
	}

	for (i = 0; i < MAX_HISP_CLIENT; ++i) {
		rpmsg_local.msg_pool[i] = create_isp_msg_pool();
		if (!rpmsg_local.msg_pool[i]) {
			cam_err("%s: create msg pool failed", __func__);
			ret = -ENOMEM;
			goto error;
		}
	}
	ret = hisp_register(pdev, &s_hisp350.intf, &s_hisp350.notify);
	if (ret == 0) {
		atomic_set(&s_hisp350.opened, 0);
	} else {
		cam_err("%s() %d hisp_register failed with ret %d",
			__func__, __LINE__, ret);
		goto error;
	}

	ret = register_rpmsg_driver(&rpmsg_hisp350_driver);
	if (ret != 0) {
		cam_err("%s() %d register_rpmsg_driver failed with ret %d",
			__func__, __LINE__, ret);
		goto error;
	}

	s_hisp350.pdev = pdev;

	ret = memset_s(&(s_hisp350.mem),
		sizeof(struct isp_mem),
		0,
		sizeof(struct isp_mem));
	if (ret != 0)
		cam_warn("%s: fail for memset_s mem_pool", __func__);
#ifdef CONFIG_DFX_DEBUG_FS
	ret = device_create_file(&pdev->dev, &hisp_ddr_freq_ctrl_attr);
	if (ret < 0) {
		cam_err("%s failed to creat hisp ddr freq ctrl attribute",
			__func__);
		unregister_rpmsg_driver(&rpmsg_hisp350_driver);
		hisp_unregister(s_hisp350.pdev);
		goto error;
	}
#endif
	mutex_lock(&kernel_fd_sgt_map_mutex);
	INIT_LIST_HEAD(&s_hisp350.fd_sgt_list);
	mutex_unlock(&kernel_fd_sgt_map_mutex);

	return 0;
error:
	wakeup_source_unregister(hisp_power_wakelock);
	hisp_power_wakelock = NULL;
	for (i = 0; i < MAX_HISP_CLIENT; ++i) {
		if (rpmsg_local.msg_pool[i])
			destroy_isp_msg_pool(rpmsg_local.msg_pool[i]);
		rpmsg_local.msg_pool[i] = NULL;
	}
	mutex_destroy(&rpmsg_local.sendlock);
	mutex_destroy(&hisp_wake_lock_mutex);
	mutex_destroy(&hisp_power_lock_mutex);
	mutex_destroy(&hisp_mem_lock_mutex);
	cam_notice("%s exit with ret = %d", __func__, ret);
	return ret;
}

#ifdef CONFIG_KERNEL_CAMERA_ISPV360
static int hisp350_platform_suspend(struct device *dev)
{
	int32_t ret = 0;
	(void)dev;
	cam_info("%s enter",__FUNCTION__);
	mutex_lock(&hisp_power_lock_mutex);
	if (hisp350_remote_processor_up()) {
		cam_err("%s fail ret -1",__FUNCTION__);
		ret = -1;
	}
	mutex_unlock(&hisp_power_lock_mutex);
	return ret;
}

static int hisp350_platform_resume(struct device *dev)
{
	cam_info("%s enter",__FUNCTION__);
	(void)dev;
	return 0;
}

static const struct dev_pm_ops asp_codec_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(hisp350_platform_suspend, hisp350_platform_resume)
};
#endif

static struct platform_driver s_hisp350_driver = {
	.probe = hisp350_platform_probe,
	.driver = {
		.name = "vendor,chip_isp350",
		.owner = THIS_MODULE,
		.of_match_table = s_hisp350_dt_match,
#ifdef CONFIG_KERNEL_CAMERA_ISPV360
		.pm = &asp_codec_pm_ops,
#endif
	},
};

#ifndef CONFIG_KERNEL_CAMERA_ISPV360
const char *pg_chip_without_isp[] = {
	"level2_partial_good_modem",
	"level2_partial_good_drv",
	"pc",
	"lite_pc",
	"unknown",
};
#else
const char *pg_chip_without_isp[] = {
	"desktop_pc",
	"unknown",
};
#endif

static bool bypass_isp_init_by_soc_spec(void)
{
	const char *soc_spec = NULL;
	int ret;
	unsigned int i;
	struct device_node *np = of_find_compatible_node(NULL, NULL, "hisilicon, soc_spec");
	if (np == NULL) {
		cam_err("%s: of_find_compatible_node fail or normal type chip\n", __func__);
		return false;
	}

	ret = of_property_read_string(np, "soc_spec_set", &soc_spec);
	if (ret < 0) {
		cam_err("%s: read string fail\n", __func__);
		return false;
	}

	for (i = 0; i < sizeof(pg_chip_without_isp) / sizeof(pg_chip_without_isp[0]); ++i) {
		ret = strncmp(soc_spec, pg_chip_without_isp[i], strlen(pg_chip_without_isp[i]));
		if (ret == 0) {
			cam_info("%s: this is pg chip:%s, need bypass isp\n", __func__, pg_chip_without_isp[i]);
			return true;
		}
	}

	cam_info("%s: no bypass isp\n", __func__);
	return false;
}

static int __init hisp350_init_module(void)
{
	if (bypass_isp_init_by_soc_spec()) {
		cam_info("%s: bypass isp init\n", __func__);
		return 0;
	}
	cam_notice("%s enter", __func__);
	return platform_driver_register(&s_hisp350_driver);
}

static void __exit hisp350_exit_module(void)
{
	if (bypass_isp_init_by_soc_spec()) {
		cam_info("%s: bypass isp init\n", __func__);
		return;
	}
	cam_notice("%s enter", __func__);
	hisp_unregister(s_hisp350.pdev);
	platform_driver_unregister(&s_hisp350_driver);
	wakeup_source_unregister(hisp_power_wakelock);
	hisp_power_wakelock = NULL;
	mutex_destroy(&hisp_wake_lock_mutex);
}

module_init(hisp350_init_module);
module_exit(hisp350_exit_module);
MODULE_DESCRIPTION("hisp350 driver");
MODULE_LICENSE("GPL v2");
