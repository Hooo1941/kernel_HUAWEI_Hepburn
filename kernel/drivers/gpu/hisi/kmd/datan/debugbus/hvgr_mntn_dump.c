/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2023-2023. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#include <securec.h>

#include "hvgr_log_api.h"
#include "hvgr_mntn_dump.h"
#include "hvgr_file.h"
#include "hvgr_log_api.h"
#include "hvgr_regmap_debug_bus.h"
#include "hvgr_debug_bus_dump.h"

static bool hvgr_mntn_check_disk(void)
{
	if ((hvgr_file_get_free_space(DATA_DIR) >> HVGR_FILE_BYTE2MB) < MIN_FREE_SPACE)
		return false;

	return true;
}

static int hvgr_mntn_get_file_name(struct hvgr_device *gdev, int dump_mode,
	char *file_name, char *mntn_dump_path)
{
	int ret = 0;

	hvgr_info(gdev, HVGR_DATAN, "%s IN", __func__);

	if (dump_mode == HVGR_DEBUG_BUS_DUMP_FIR)
		ret = sprintf_s(file_name, MAX_FILE_NAME_SIZE, HVGR_MNTN_DUMP_DIR,
		mntn_dump_path, HVGR_DEBUG_BUS_DATA_FILE);
	else if (dump_mode == HVGR_DEBUG_BUS_DUMP_SEC)
		ret = sprintf_s(file_name, MAX_FILE_NAME_SIZE, HVGR_MNTN_DUMP_DIR,
		mntn_dump_path, HVGR_DEBUG_BUS_DATA_FILE_DUPLICATED);
	else
		hvgr_err(gdev, HVGR_DATAN, "%s dump mode error", __func__);

	if (ret < 0)
		hvgr_err(gdev, HVGR_DATAN, "%s fail", __func__);

	hvgr_info(gdev, HVGR_DATAN, "%s OUT", __func__);

	return ret;
}

static void hvgr_mntn_write_buffer_to_file(struct hvgr_device *gdev, char *buffer,
	int dump_mode, char *mntn_dump_path)
{
	char file_name[MAX_FILE_NAME_SIZE] = {0};

	if (strlen(buffer) == 0)
		return;

	if (hvgr_mntn_get_file_name(gdev, dump_mode, file_name, mntn_dump_path) < 0)
		return;

	hvgr_debug_bus_write_to_file(buffer, file_name);
}

void hvgr_mntn_debugbus_write_file(struct hvgr_device *gdev, char *mntn_dump_path)
{
	u32 buffer_size;
	struct hvgr_debugbus_dev *bus_dev = NULL;

	if (gdev == NULL || mntn_dump_path == NULL)
		return;

	hvgr_info(gdev, HVGR_DATAN, "%s IN", __func__);

	bus_dev = &gdev->datan_dev.datan_bus_dev;

	if (!hvgr_mntn_check_disk())
		return;

	mutex_lock(&bus_dev->debug_bus_mutex);
	if (bus_dev->debugbus_buf != NULL) {
		hvgr_mntn_write_buffer_to_file(gdev, bus_dev->debugbus_buf,
			HVGR_DEBUG_BUS_DUMP_FIR, mntn_dump_path);
		/*
			* Dump twice for file comparison
			* Same files indicate that gpu job hangs
			* Different files indicate that gpu job timeouts
			*/
		buffer_size = kdatan_debug_bus_get_buffer_size(gdev);
		if (kdatan_need_to_dump_duplicately())
			hvgr_mntn_write_buffer_to_file(
				gdev, &bus_dev->debugbus_buf[buffer_size],
				HVGR_DEBUG_BUS_DUMP_SEC, mntn_dump_path);
		vfree(bus_dev->debugbus_buf);
		bus_dev->debugbus_buf = NULL;
	}
	bus_dev->debugbus_cnt--;
	mutex_unlock(&bus_dev->debug_bus_mutex);

	hvgr_info(gdev, HVGR_DATAN, "%s OUT", __func__);
}
