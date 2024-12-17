#ifndef __HCK_PCIE__
#define __HCK_PCIE__

#include <linux/types.h>
#include <platform_include/basicplatform/linux/hck_vendor_hooks.h>

/*
 * Following tracepoints are not exported in tracefs and provide a
 * mechanism for vendor modules to hook and extend functionality
 */

struct pcie_port;
struct device;
struct list_head;
struct pci_dev;

DECLARE_HCK_VH(kport_pcie_refclk_host_vote,
	TP_PROTO(struct pcie_port *pp, u32 vote),
	TP_ARGS(pp, vote));

DECLARE_HCK_VH(kport_pcie_release_resource,
	TP_PROTO(struct device *dev, struct list_head *resources),
	TP_ARGS(dev, resources));

DECLARE_HCK_VH(kport_pcie_bypass_pm,
	TP_PROTO(struct pci_dev *dev, bool *err),
	TP_ARGS(dev, err));

DECLARE_HCK_VH(kport_pcie_stub_class,
	TP_PROTO(u32 *class, u32 new_class),
	TP_ARGS(class, new_class));

DECLARE_HCK_VH(kport_pcie_stub_bar,
	TP_PROTO(u32 *number),
	TP_ARGS(number));

#endif /* __HCK_PCIE__ */
