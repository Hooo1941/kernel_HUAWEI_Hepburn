#ifndef VIRTIO_VIRTIO_LOG_H
#define VIRTIO_VIRTIO_LOG_H

#include <linux/printk.h>
#include <linux/types.h>

#define vfb_debug(msg, ...) \
	do { pr_info("[vfb D]: "msg, ##__VA_ARGS__); } while (0)

#endif
