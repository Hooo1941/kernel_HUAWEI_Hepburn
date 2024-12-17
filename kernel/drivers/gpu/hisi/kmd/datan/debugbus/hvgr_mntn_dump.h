/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2023-2023. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

/* Backend specific configuration */
#ifndef HVGR_MNTN_DUMP_H
#define HVGR_MNTN_DUMP_H

#include "hvgr_defs.h"

#define HVGR_MNTN_DUMP_DIR      "%s%s"

/*
 * @brief hvgr mntn module dump buffer to file
 */
void hvgr_mntn_debugbus_write_file(struct hvgr_device *gdev, char *mntn_dump_path);

#endif /* HVGR_MNTN_DUMP_H */
