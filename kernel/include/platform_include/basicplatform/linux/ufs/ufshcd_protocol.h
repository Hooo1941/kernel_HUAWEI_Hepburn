/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: ufs protocol header file
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __UFSHCD_PROTOCOL_H__
#define __UFSHCD_PROTOCOL_H__
#include "ufshcd.h"

struct ctm_info {
	struct ufs_hba	*hba;
	unsigned long	pending;
	unsigned int	ncpl;
};

bool ufshcd_get_dev_cmd_tag(struct ufs_hba *hba, int *tag_out);
int ufshcd_clear_ua_wluns(struct ufs_hba *hba);
int ufshcd_send_request_sense(struct ufs_hba *hba, struct scsi_device *sdp);
bool ufshcd_compl_tm(struct request *req, void *priv, bool reserved);
unsigned int ufshcd_get_tag_from_cmd(struct Scsi_Host *host, struct ufs_hba *hba,
				struct scsi_cmnd *cmd);
void ufshcd_update_dev_cmd_lun(struct ufs_hba *hba,
				      struct ufshcd_lrb *lrbp);
void ufshcd_compose_scsi_cmd(struct scsi_cmnd *cmd,
				    struct scsi_device *device,
				    unsigned char *cdb,
				    unsigned char *sense_buffer,
				    enum dma_data_direction sc_data_direction,
				    struct scatterlist *sglist,
				    unsigned int nseg,
				    unsigned int sg_len);
int ufshcd_send_scsi_sync_cache(struct ufs_hba *hba, struct scsi_device *sdp,
		enum ufs_dev_pwr_mode pwr_mode);
int ufshcd_send_scsi_request_sense(struct ufs_hba *hba,
		struct scsi_device *sdp, unsigned int timeout, bool eh_handle);
#if defined(CONFIG_MAS_ORDER_PRESERVE) || defined(CONFIG_MAS_UNISTORE_PRESERVE)
int ufshcd_send_request_sense_directly(struct scsi_device *sdp, unsigned int timeout, bool eh_handle);
#endif

int ufshcd_send_scsi_ssu(struct ufs_hba *hba, struct scsi_device *sdp, unsigned char *cmd,
		unsigned int timeout, struct scsi_sense_hdr *sshdr);
void ufshcd_rsp_sense_data(struct ufs_hba *hba, struct ufshcd_lrb *lrbp, int scsi_status);

#endif
