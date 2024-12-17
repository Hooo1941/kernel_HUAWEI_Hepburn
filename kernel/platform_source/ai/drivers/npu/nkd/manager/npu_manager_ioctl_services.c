/*
 * npu_manager_ioctl_services.c
 *
 * about npu manager ioctl services
 *
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
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
#include "npu_manager_ioctl_services.h"

#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/semaphore.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/swap.h>
#include <linux/types.h>
#include <linux/list.h>
#include <linux/spinlock.h>
#include <asm/uaccess.h>
#include <linux/mm_lb/mm_lb.h>

#include "npu_ioctl_services.h"
#include "npu_log.h"
#include "npu_platform.h"
#include "npu_ion.h"
#include "npu_iova.h"

static int npu_manager_ioctl_get_devnum(struct file *filep,
	unsigned int cmd, unsigned long arg)
{
	u32 devnum = 1;
	unused(filep);
	unused(cmd);

	if (copy_to_user_safe((void __user *)(uintptr_t)arg, &devnum, sizeof(u32)))
		return -EFAULT;
	else
		return 0;
}

static int npu_manager_ioctl_get_plat_info(struct file *filep,
	unsigned int cmd, unsigned long arg)
{
	struct npu_plat_info npu_plat_system_info = {0};
	struct npu_platform_info *plat_info = npu_plat_get_info();
	unused(filep);

	if (plat_info == NULL) {
		npu_drv_err("get plat info failed\r\n");
		return -EFAULT;
	}

	npu_plat_system_info.plat_type = (u32)plat_info->plat_type;

	if (copy_to_user_safe((void __user *)(uintptr_t)arg, &npu_plat_system_info, sizeof(struct npu_plat_info))) {
		npu_drv_err("cmd, cmd = %u copy plat_info to user failed\n",
			_IOC_NR(cmd));
		return -EFAULT;
	}

	return 0;
}

static int npu_get_devids(u32 *devices)
{
	u8 i;
	u8 j = 0;

	if (devices == NULL)
		return -EINVAL;

	/* get device id assigned from host,
	 * default dev_id is 0 if there is no host
	 */
	for (i = 0; i < NPU_MAX_DEVICE_NUM; i++)
		devices[j++] = i;

	if (j == 0) {
		npu_drv_err("NO dev_info!!!\n");
		return -EFAULT;
	}

	return 0;
}

static int npu_manager_get_devids(unsigned long arg)
{
	struct npu_manager_devinfo devinfo = {0};

	devinfo.num_dev = 1;
	if (npu_get_devids(devinfo.devids)) {
		npu_drv_err("npu_get_devids failed\n");
		return -EINVAL;
	}
	if (copy_to_user_safe((void __user *)(uintptr_t)arg,
		&devinfo, sizeof(devinfo))) {
		npu_drv_err("copy from user failed\n");
		return -EINVAL;
	}

	return 0;
}

static int npu_manager_svmva_to_devid(struct file *filep,
	unsigned int cmd, unsigned long arg)
{
	struct npu_svm_to_devid npu_svm_devid = {0};
	u32 dev_id = 0;
	unused(filep);
	unused(cmd);

	npu_drv_debug("start\n");

	if (copy_from_user_safe(&npu_svm_devid, (void *)(uintptr_t)arg,
		sizeof(npu_svm_devid))) {
		npu_drv_err("copy_from_user_safe failed\n");
		return -EFAULT;
	}

	npu_svm_devid.src_devid = dev_id;
	npu_svm_devid.dest_devid = dev_id;

	if (copy_to_user_safe((void __user *)(uintptr_t)arg, &npu_svm_devid,
		sizeof(struct npu_svm_to_devid))) {
		npu_drv_err("copy_to_user_safe failed\n");
		return -EFAULT;
	}

	npu_drv_debug("finish\n");

	return 0;
}

static int npu_manager_check_ion(struct file *filep,
	unsigned int cmd, unsigned long arg)
{
	npu_check_ion_t check_ion;
	unused(filep);
	unused(cmd);

	npu_drv_debug("start\n");

	if (copy_from_user_safe(&check_ion,
		(void *)(uintptr_t)arg, sizeof(check_ion))) {
		npu_drv_err("copy_from_user_safe failed\n");
		return -EFAULT;
	}

	npu_ioctl_check_ionfd(&check_ion);
	cond_return_error(check_ion.result != NPU_VALID_ION, -EINVAL, "invalid ion\n");

	if (copy_to_user_safe((void __user *)(uintptr_t)arg,
		&check_ion, sizeof(check_ion))) {
		npu_drv_err("copy_to_user_safe failed\n");
		return -EFAULT;
	}

	npu_drv_debug("npu manager check ion finish\n");

	return 0;
}

/* ION memory IOVA map */
static int npu_manager_ioctl_iova_map(struct file *filep,
	unsigned int cmd, unsigned long arg)
{
	int ret = -1;
#ifdef NPU_ARCH_V100
	unused(filep);
	unused(cmd);
	unused(arg);
#else
	u64 vaddr = 0;
	npu_check_ion_t check_ion = {0};
	struct npu_iova_ioctl_info msg = {0};
	unused(filep);
	unused(cmd);

	ret = copy_from_user_safe(&msg,
		(void __user *)(uintptr_t)arg, sizeof(msg));
	cond_return_error(ret != 0, -EINVAL,
		"fail to copy iova map params, ret = %d\n", ret);

	check_ion.ionfd = msg.fd;
	npu_ioctl_check_ionfd(&check_ion);
	cond_return_error(check_ion.result != NPU_VALID_ION, -EINVAL, "invalid ion\n");

	/* map iova memory */
	ret = npu_iova_map_ion(msg.fd, msg.prot, (unsigned long *)&vaddr);
	cond_return_error(ret != 0, -EINVAL,
		"npu iova map ion fail, ret = %d\n", ret);

	msg.vaddr = vaddr;
	if (copy_to_user_safe((void __user *)(uintptr_t)arg, &msg, sizeof(msg))) {
		npu_drv_err("copy to user safe error\n");
		/* unmap iova memory */
		ret = npu_iova_unmap_ion(msg.fd, vaddr);
		if(ret != 0)
			npu_drv_err("npu iova unmap ion error, ret = %d\n", ret);
		return -EINVAL;
	}
#endif

	return ret;
}

/* ION memory IOVA unmap */
static int npu_manager_ioctl_iova_unmap(struct file *filep,
	unsigned int cmd, unsigned long arg)
{
	int ret = -1;
#ifdef NPU_ARCH_V100
	unused(filep);
	unused(cmd);
	unused(arg);
#else
	struct npu_iova_ioctl_info msg = {0};
	npu_check_ion_t check_ion = {0};
	unused(filep);
	unused(cmd);

	ret = copy_from_user_safe(&msg,
		(void __user *)(uintptr_t)arg, sizeof(msg));
	cond_return_error(ret != 0, -EINVAL,
		"fail to copy iova unmap params, ret = %d\n", ret);

	check_ion.ionfd = msg.fd;
	npu_ioctl_check_ionfd(&check_ion);
	cond_return_error(check_ion.result != NPU_VALID_ION, -EINVAL, "invalid ion\n");

	/* map iova memory */
	ret = npu_iova_unmap_ion(msg.fd, msg.vaddr);
	cond_return_error(ret != 0, -EINVAL,
		"npu iova unmap ion fail, ret = %d\n", ret);
#endif

	return ret;
}

static int npu_manager_get_transway(struct file *filep,
	unsigned int cmd, unsigned long arg)
{
	struct npu_trans_info npu_trans_info = {0};
	int ret;
	unused(filep);
	unused(cmd);

	ret = copy_from_user_safe(&npu_trans_info,
		(void *)(uintptr_t)arg, sizeof(npu_trans_info));
	if (ret) {
		npu_drv_err("copy from user failed, ret: %d\n", ret);
		return -EFAULT;
	}

	npu_trans_info.ways = DRV_SDMA;

	ret = copy_to_user_safe((void __user *)(uintptr_t)arg,
		&npu_trans_info, sizeof(npu_trans_info));
	if (ret) {
		npu_drv_err("copy from user failed\n");
		return -EINVAL;
	}
	return 0;
}

static int npu_manager_devinfo_ioctl(struct file *filep,
	unsigned int cmd, unsigned long arg)
{
	int ret;
	unused(filep);

	switch (cmd) {
	case NPU_MANAGER_GET_DEVIDS:
		ret = npu_manager_get_devids(arg);
		break;
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

static int (*const npu_manager_ioctl_handlers[NPU_MANAGER_CMD_MAX_NR])
	(struct file *filep, unsigned int cmd, unsigned long arg) = {
		[_IOC_NR(NPU_MANAGER_GET_DEVNUM)] = npu_manager_ioctl_get_devnum,
		[_IOC_NR(NPU_MANAGER_GET_PLATINFO)] = npu_manager_ioctl_get_plat_info,
		[_IOC_NR(NPU_MANAGER_GET_DEVIDS)] = npu_manager_devinfo_ioctl,
		[_IOC_NR(NPU_MANAGER_SVMVA_TO_DEVID)] = npu_manager_svmva_to_devid,
		[_IOC_NR(NPU_MANAGER_CHECK_ION)] = npu_manager_check_ion,
		[_IOC_NR(NPU_MANAGER_IOVA_MAP)] = npu_manager_ioctl_iova_map,
		[_IOC_NR(NPU_MANAGER_IOVA_UNMAP)] = npu_manager_ioctl_iova_unmap,
		[_IOC_NR(NPU_MANAGER_GET_TRANSWAY)] = npu_manager_get_transway,
};

long npu_manager_ioctl(struct file *filep, unsigned int cmd, unsigned long arg)
{
	npu_drv_debug("start IOC_NR = %d cmd = %d\n", _IOC_NR(cmd), cmd);
	if (_IOC_NR(cmd) >= NPU_MANAGER_CMD_MAX_NR || arg == 0) {
		npu_drv_err("invalid cmd, cmd = %u\n", _IOC_NR(cmd));
		return -EINVAL;
	}

	if (npu_manager_ioctl_handlers[_IOC_NR(cmd)] == NULL) {
		npu_drv_err("invalid cmd, cmd = %u\n", _IOC_NR(cmd));
		return -EINVAL;
	}

	return npu_manager_ioctl_handlers[_IOC_NR(cmd)](filep, cmd, arg);
}
