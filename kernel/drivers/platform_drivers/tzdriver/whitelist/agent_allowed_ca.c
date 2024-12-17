/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 * Description: allowed_ext_agent_ca list and functions
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include "agent.h"
#include <linux/uaccess.h>
#include <linux/kernel.h>
#include <securec.h>

static struct ca_info g_allowed_ext_agent_ca[] = {
#ifdef CONFIG_TZDRIVER
	{
		"/vendor/bin/hiaiserver",
		1000,
		TEE_SECE_AGENT_ID,
	},
	{
		"/vendor/bin/hw/vendor.huawei.hardware.\
biometrics.hwfacerecognize@1.1-service",
		1000,
		TEE_FACE_AGENT1_ID,
	},
	{
		"/vendor/bin/hw/vendor.huawei.hardware.\
biometrics.hwfacerecognize@1.1-service",
		1000,
		TEE_FACE_AGENT2_ID,
	},
#endif
};

int is_allowed_agent_ca(const struct ca_info *ca,
	bool check_agent_id)
{
	uint32_t i;
	struct ca_info *tmp_ca = g_allowed_ext_agent_ca;
	const uint32_t nr = ARRAY_SIZE(g_allowed_ext_agent_ca);

	if (!ca)
		return -EFAULT;

	if (!check_agent_id) {
		for (i = 0; i < nr; i++) {
			if (!strncmp(ca->path, tmp_ca->path,
				strlen(tmp_ca->path) + 1) &&
				ca->uid == tmp_ca->uid)
				return 0;
			tmp_ca++;
		}
	} else {
		for (i = 0; i < nr; i++) {
			if (!strncmp(ca->path, tmp_ca->path,
				strlen(tmp_ca->path) + 1) &&
				ca->uid == tmp_ca->uid &&
				ca->agent_id == tmp_ca->agent_id)
				return 0;
			tmp_ca++;
		}
	}
	tlogd("ca-uid is %u, ca_path is %s, agent id is %x\n", ca->uid,
		ca->path, ca->agent_id);

	return -EACCES;
}

