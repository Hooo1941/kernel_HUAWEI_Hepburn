/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description:usb patch
 * Author: @CompanyNameTag
 */

#include "oal_usb_patch.h"
#include "oal_usb_driver.h"
#include "oal_usb_linux.h"

static void oal_usb_tx_patch_complete(struct urb *urb)
{
    struct oal_usb_xfer_param *param = NULL;

    param = (struct oal_usb_xfer_param *)urb->context;
    oal_usb_tx_complete(urb, param);
}

static void oal_usb_rx_patch_complete(struct urb *urb)
{
    struct oal_usb_xfer_param *param = NULL;

    param = (struct oal_usb_xfer_param *)urb->context;
    oal_usb_rx_complete(urb, param);
}

int32_t oal_usb_patch_read(hcc_bus *bus, uint8_t *buff, int32_t len, uint32_t timeout)
{
    struct oal_usb_dev_container *usb_dev_container = NULL;
    struct oal_usb_dev *usb_dev = NULL;
    struct oal_usb_xfer_param *param = NULL;
    int32_t ret;
    uint8_t *tmp = NULL;

    usb_dev_container = (struct oal_usb_dev_container *)bus->data;
    if (usb_dev_container == NULL) {
        oal_print_mpxx_log(MPXX_LOG_INFO, "usb_dev_container is null");
        return -OAL_ENODEV;
    }

    usb_dev = usb_dev_container->usb_devices[OAL_USB_INTERFACE_BULK][OAL_USB_CHAN_0];
    if (usb_dev == NULL) {
        oal_print_mpxx_log(MPXX_LOG_INFO, "usb_dev_loader is null");
        return -OAL_ENODEV;
    }

    tmp = kzalloc(len, GFP_KERNEL);
    if (tmp == NULL) {
        oal_print_mpxx_log(MPXX_LOG_INFO, "no mem allocate, len:[%d] \n", len);
        return -OAL_ENOMEM;
    }
    param = oal_usb_alloc_xfer_param(usb_dev, tmp, len, timeout, USB_XFER_NO_DMA, oal_usb_rx_patch_complete, NULL);
    ret = oal_usb_bulk_rx(param);
    if (ret < 0) {
        oal_print_mpxx_log(MPXX_LOG_INFO, "rx fail");
    } else {
        ret = memcpy_s(buff, len, tmp, len);
        if (oal_unlikely(ret != EOK)) {
            kfree(tmp);
            return -OAL_ENODEV;
        }
    }
    kfree(tmp);
    return ret;
}

int32_t oal_usb_patch_write(hcc_bus *bus, uint8_t *buff, int32_t len)
{
    struct oal_usb_dev_container *usb_dev_container = NULL;
    struct oal_usb_dev *usb_dev = NULL;
    struct oal_usb_xfer_param *param = NULL;
    int32_t ret;
    uint8_t *tmp = NULL;

    usb_dev_container = (struct oal_usb_dev_container *)bus->data;
    if (usb_dev_container == NULL) {
        oal_print_mpxx_log(MPXX_LOG_INFO, "usb_dev_container is null");
        return -OAL_ENODEV;
    }

    usb_dev = usb_dev_container->usb_devices[OAL_USB_INTERFACE_BULK][OAL_USB_CHAN_0];
    if (usb_dev == NULL) {
        oal_print_mpxx_log(MPXX_LOG_INFO, "usb_dev_loader is null");
        return -OAL_ENODEV;
    }

    tmp = kzalloc(len, GFP_KERNEL);
    if (tmp == NULL) {
        oal_print_mpxx_log(MPXX_LOG_INFO, "no mem allocate, len:[%d] \n", len);
        return -OAL_ENOMEM;
    }
    ret = memcpy_s(tmp, len, buff, len);
    if (oal_unlikely(ret != EOK)) {
        kfree(tmp);
        return -OAL_ENODEV;
    }
    param = oal_usb_alloc_xfer_param(usb_dev, tmp, len, TRANSFER_DEFAULT_TIMEOUT, USB_XFER_NO_DMA,
                                     oal_usb_tx_patch_complete, NULL);
    ret = oal_usb_bulk_tx(param);
    if (ret != OAL_SUCC) {
        oal_print_mpxx_log(MPXX_LOG_INFO, "tx fail");
    }
    kfree(tmp);
    return ret;
}
