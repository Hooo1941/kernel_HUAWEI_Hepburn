/******************************************************************
 * Copyright    Copyright (c) 2020- Hisilicon Technologies CO., Ltd.
 * File name    ipp_core.h
 * Description:
 *
 * Date         2020-04-16 17:36:21
 ******************************************************************/
#ifndef _IPP_CORE_H_INCLUDED__
#define _IPP_CORE_H_INCLUDED__
#include "ippmessage_common.h"

struct ipp_feature_mem_size_t {
	unsigned int work_module;
	unsigned int mem_size;
};

enum HISP_CPE_IRQ_TYPE {
	CPE_IRQ_0 = 0, // ACPU
	MAX_HISP_CPE_IRQ
};

struct hispcpe_s {
	struct miscdevice miscdev;
	struct platform_device *pdev;
	int initialized;
	struct wakeup_source *ipp_wakelock;
	struct mutex ipp_wakelock_mutex;

	struct mutex open_lock;
	unsigned int open_refs;

	struct mutex dev_lock;
	int refs_power_up;

	unsigned int irq_num;
	unsigned int reg_num;
	int irq[MAX_HISP_CPE_IRQ];
	struct resource *r[MAX_HISP_CPE_REG];
	void __iomem *reg[MAX_HISP_CPE_REG];

	unsigned int sid;
	unsigned int ssid;
#ifdef IPP_KERNEL_USE_PCIE_VERIFY
	unsigned int pci_flag;
#endif
};

#define HISP_IPP_PWRUP          _IOWR('C', 0x1001, int)
#define HISP_IPP_PWRDN          _IOWR('C', 0x1002, int)
#define HISP_IPP_CMD_REQUEST    _IOWR('C', 0x2002, int)
#define HISP_IPP_MAP_BUF        _IOWR('C', 0x200C, int)
#define HISP_IPP_UNMAP_BUF      _IOWR('C', 0x200D, int)
#define HISP_IPP_GET_MEM_SIZE   _IOWR('C', 0x200F, int)
// new add r02 for algo map
#define HISP_IPP_MAP_ALGO_BUF   _IOWR('C', 0x200A, int)
#define HISP_IPP_UNMAP_ALGO_BUF _IOWR('C', 0x200B, int)

// only for syzkaller test
#ifdef CONFIG_IPP_DEBUG
#define HISP_IPP_SYZKALLER_TEST _IOWR('C', 0x2003, int)
#endif

int hipp_adapter_probe(struct platform_device *pdev);
void hipp_adapter_remove(void);
int hipp_adapter_register_irq(int irq);
int hipp_adapter_unregister_irq(int irq);
void __iomem *hipp_get_regaddr(unsigned int type);
int get_hipp_smmu_info(int *sid, int *ssid);

#endif
/* **************************************end*********************************** */
