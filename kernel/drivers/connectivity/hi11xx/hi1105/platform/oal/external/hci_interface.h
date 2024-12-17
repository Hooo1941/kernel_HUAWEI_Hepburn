/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description:hci_interface.h header file
 * Author: @CompanyNameTag
 */

#ifndef HCI_INTERFACE_H
#define HCI_INTERFACE_H

#ifdef _PRE_CONFIG_HISI_110X_BLUEZ
#include <net/bluetooth/bluetooth.h>
#include <net/bluetooth/hci_core.h>
#ifdef _PRE_CONFIG_HISI_S3S4_POWER_STATE

#ifdef _PRE_CONFIG_S3_HCI_DEV_OPT

#ifdef _PRE_PRODUCT_ARMPC
int hisi_hci_dev_do_open(struct hci_dev *hdev);
#define external_hci_dev_do_open hisi_hci_dev_do_open
#else
int extend_hci_dev_do_open(struct hci_dev *hdev);
#define external_hci_dev_do_open extend_hci_dev_do_open
#endif /* _PRE_PRODUCT_ARMPC */

#endif  /* _PRE_CONFIG_S3_HCI_DEV_OPT */

#endif  /* _PRE_CONFIG_HISI_S3S4_POWER_STATE */

#endif  /* _PRE_CONFIG_HISI_110X_BLUEZ */

#endif
