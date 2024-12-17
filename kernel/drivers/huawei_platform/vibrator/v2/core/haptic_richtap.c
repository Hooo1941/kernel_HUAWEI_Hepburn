/*
 * haptic_richtap.c
 *
 * code for vibrator
 *
 * Copyright (c) 2023 Huawei Technologies Co., Ltd.
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

#include <linux/delay.h>
#include <linux/mman.h>

#include "haptic_core.h"
#include "haptic_richtap.h"

#ifdef AAC_RICHTAP_SUPPORT

static void richtap_clean_buf(struct haptic *haptic, int status)
{
	struct mmap_buf_format *opbuf = haptic->start_buf;
	int i = 0;

	for (i = 0; i < RICHTAP_MMAP_BUF_SUM; i++) {
		memset(opbuf->data, 0, RICHTAP_MMAP_BUF_SIZE);
		opbuf->status = status;
		opbuf = opbuf->kernel_next;
	}
}

static void richtap_update_fifo_data(struct haptic *haptic, uint32_t fifo_len)
{
	int32_t samples_left;
	int pos = 0;
	int retry = 3; // set max retry count 3

	do {
		if (haptic->curr_buf->status == MMAP_BUF_DATA_VALID) {
			samples_left = haptic->curr_buf->length - haptic->pos;
			if (samples_left < fifo_len) {
				memcpy(&haptic->rtp_ptr[pos],
				       &haptic->curr_buf->data[haptic->pos],
				       samples_left);
				pos += samples_left;
				fifo_len -= samples_left;
				haptic->curr_buf->status =
					MMAP_BUF_DATA_INVALID;
				haptic->curr_buf->length = 0;
				haptic->curr_buf =
					haptic->curr_buf->kernel_next;
				haptic->pos = 0;
			} else {
				memcpy(&haptic->rtp_ptr[pos],
				       &haptic->curr_buf->data[haptic->pos],
				       fifo_len);
				haptic->pos += fifo_len;
				pos += fifo_len;
				fifo_len = 0;
			}
		} else if (haptic->curr_buf->status == MMAP_BUF_DATA_FINISHED) {
			break;
		} else {
			if (retry-- <= 0) {
				vib_info("invalid data");
				break;
			} else {
				usleep_range(1000, 1000);
			}
		}
	} while (fifo_len > 0 && atomic_read(&haptic->richtap_rtp_mode));
	vib_dbg("update fifo len %d", pos);
	haptic->hops->set_rtp_data(haptic, haptic->rtp_ptr, pos);
}

static bool richtap_rtp_start(struct haptic *haptic)
{
	int cnt = 200; // max wait gls state time
	bool rtp_work_flag = false;
	uint8_t reg_val = 0;

	mutex_lock(&haptic->lock);
	haptic->hops->play_mode(haptic, HAPTIC_RTP_MODE);
	haptic->hops->play_go(haptic, true);
	usleep_range(2000, 2000);

	while (cnt) {
		reg_val = haptic->hops->get_glb_state(haptic);
		if ((reg_val & HAPTIC_GLBRD_STATE_MASK) == HAPTIC_STATE_RTP) {
			cnt = 0;
			rtp_work_flag = true;
			vib_info("RTP_GO! glb_state=0x08");
		} else {
			cnt--;
			vib_dbg("wait for RTP_GO, glb_state=0x%02X", reg_val);
		}
		usleep_range(2000, 2500);
	}

	if (rtp_work_flag == false)
		haptic->hops->play_stop(haptic);

	mutex_unlock(&haptic->lock);
	return rtp_work_flag;
}

static void richtap_rtp_work(struct work_struct *work)
{
	struct haptic *haptic =
		container_of(work, struct haptic, richtap_rtp_work);
	uint32_t retry = 0;
	uint32_t tmp_len = 0;

	if (!haptic)
		return;
	haptic->curr_buf = haptic->start_buf;
	do {
		if (haptic->curr_buf->status == MMAP_BUF_DATA_VALID) {
			if ((tmp_len + haptic->curr_buf->length) >
			    haptic->ram.base_addr) {
				memcpy(&haptic->rtp_ptr[tmp_len],
				       haptic->curr_buf->data,
				       (haptic->ram.base_addr - tmp_len));
				haptic->pos = haptic->ram.base_addr - tmp_len;
				tmp_len = haptic->ram.base_addr;
			} else {
				memcpy(&haptic->rtp_ptr[tmp_len],
				       haptic->curr_buf->data,
				       haptic->curr_buf->length);
				tmp_len += haptic->curr_buf->length;
				haptic->curr_buf->status =
					MMAP_BUF_DATA_INVALID;
				haptic->curr_buf->length = 0;
				haptic->pos = 0;
				haptic->curr_buf =
					haptic->curr_buf->kernel_next;
			}
		} else if (haptic->curr_buf->status == MMAP_BUF_DATA_FINISHED) {
			break;
		} else {
			msleep(1);
		}
	} while (tmp_len < haptic->ram.base_addr && retry++ < 30); // try cont

	vib_dbg("rtp tm_len = %d, retry = %d, haptic->ram.base_addr = %d",
		tmp_len, retry, haptic->ram.base_addr);
	if (richtap_rtp_start(haptic)) {
		haptic->hops->set_rtp_data(haptic, haptic->rtp_ptr, tmp_len);
		haptic->hops->set_rtp_aei(haptic, true);
		haptic->hops->irq_clear(haptic);
		atomic_set(&haptic->richtap_rtp_mode, true);
	}
}

static void richtap_config_stream_mode(struct haptic *haptic)
{
	richtap_clean_buf(haptic, MMAP_BUF_DATA_INVALID);
	mutex_lock(&haptic->lock);
	haptic->hops->irq_clear(haptic);
	haptic->hops->play_stop(haptic);
	mutex_unlock(&haptic->lock);
	haptic->hops->set_rtp_aei(haptic, false);
	atomic_set(&haptic->richtap_rtp_mode, false);
	haptic->hops->upload_lra(haptic, HAPTIC_OSC_CALI_LRA);
	haptic->hops->set_bst_vol(haptic, haptic->vmax);
	schedule_work(&haptic->richtap_rtp_work);
}

static void richtap_config_stop_mode(struct haptic *haptic)
{
	richtap_clean_buf(haptic, MMAP_BUF_DATA_FINISHED);
	mutex_lock(&haptic->lock);
	haptic->hops->irq_clear(haptic);
	haptic->hops->play_stop(haptic);
	mutex_unlock(&haptic->lock);
	haptic->hops->enable_gain(haptic, 0);
	haptic->hops->set_rtp_aei(haptic, false);
	atomic_set(&haptic->richtap_rtp_mode, false);
}

int richtap_rtp_config_init(struct haptic *haptic)
{
	int i;
	struct mmap_buf_format *temp = NULL;

	if (!haptic)
		return -EINVAL;

	haptic->rtp_ptr =
		devm_kmalloc(haptic->dev,
			     RICHTAP_MMAP_BUF_SIZE * RICHTAP_MMAP_BUF_SUM,
			     GFP_KERNEL);
	if (!haptic->rtp_ptr) {
		vib_err("malloc rtp memory failed");
		return -ENOMEM;
	}

	haptic->start_buf = (struct mmap_buf_format *)devm_get_free_pages(
		haptic->dev, GFP_KERNEL, RICHTAP_MMAP_PAGE_ORDER);
	if (!haptic->start_buf) {
		vib_err("get_free_pages failed");
		return -ENOMEM;
	}
	SetPageReserved(virt_to_page(haptic->start_buf));

	// set circular queues
	temp = haptic->start_buf;
	for (i = 1; i < RICHTAP_MMAP_BUF_SUM; i++) { // from page 1 start
		temp->kernel_next = (haptic->start_buf + i);
		temp = temp->kernel_next;
	}
	temp->kernel_next = haptic->start_buf;

	INIT_WORK(&haptic->richtap_rtp_work, richtap_rtp_work);
	atomic_set(&haptic->richtap_rtp_mode, false);
	return 0;
}

irqreturn_t irq_richtap_aei_handle(struct haptic *haptic)
{
	if (!haptic)
		return IRQ_NONE;

	if (!haptic->hops->get_irq_state(haptic)) {
		vib_dbg("haptic rtp fifo almost empty");
		/* modify next line second parameter according to ram.base_addr */
		richtap_update_fifo_data(haptic, (haptic->ram.base_addr >> 1));
		while (!haptic->hops->rtp_get_fifo_afi(haptic) &&
		       atomic_read(&haptic->richtap_rtp_mode) &&
		       (haptic->curr_buf->status == MMAP_BUF_DATA_VALID))
			richtap_update_fifo_data(haptic,
						 (haptic->ram.base_addr >> 2));
	}
	if (haptic->curr_buf->status == MMAP_BUF_DATA_INVALID) {
		haptic->hops->enable_gain(haptic, 0);
		haptic->hops->set_rtp_aei(haptic, false);
		atomic_set(&haptic->richtap_rtp_mode, false);
	}
	return IRQ_HANDLED;
}

long richtap_file_unlocked_ioctl(struct file *filp, unsigned int cmd,
				 unsigned long arg)
{
	struct haptic *haptic = NULL;
	int ret = 0;
	int tmp;

	if (!filp)
		return -EFAULT;
	haptic = (struct haptic *)filp->private_data;
	if (!haptic)
		return -EFAULT;
	vib_info("cmd=0x%x, arg=0x%lx", cmd, arg);
	if (haptic->info.is_enabled_richtap_core != true) {
		vib_info("do not support richtap core feature");
		return -EFAULT;
	}
	switch (cmd) {
	case RICHTAP_GET_HWINFO:
		tmp = RICHTAP_MAGIC;
		if (copy_to_user((void __user *)arg, &tmp, sizeof(int)))
			ret = -EFAULT;
		break;
	case RICHTAP_RTP_MODE:
		break;
	case RICHTAP_OFF_MODE:
		break;
	case RICHTAP_GET_F0:
		tmp = haptic->f0;
		if (copy_to_user((void __user *)arg, &tmp, sizeof(int)))
			ret = -EFAULT;
		break;
	case RICHTAP_SETTING_GAIN:
		if (arg > 0x80)
			arg = 0x80;
		haptic->hops->enable_gain(haptic, 1);
		haptic->hops->set_gain(haptic, (uint8_t)arg);
		break;
	case RICHTAP_STREAM_MODE:
		richtap_config_stream_mode(haptic);
		break;
	case RICHTAP_STOP_MODE:
		richtap_config_stop_mode(haptic);
		break;
	default:
		vib_info("unknown cmd:%u", cmd);
		break;
	}
	return ret;
}

int richtap_file_mmap(struct file *filp, struct vm_area_struct *vma)
{
	unsigned long phys;
	struct haptic *haptic = NULL;
	int ret;
	vm_flags_t vm_flags;

	if (!filp || !vma)
		return -EPERM;
	haptic = (struct haptic *)filp->private_data;
	if (!haptic)
		return -EPERM;

	// only accept PROT_READ, PROT_WRITE and MAP_SHARED from the API of mmap
	vm_flags = calc_vm_prot_bits(PROT_READ | PROT_WRITE, 0) |
		   calc_vm_flag_bits(MAP_SHARED);
	vm_flags |= current->mm->def_flags | VM_MAYREAD | VM_MAYWRITE |
		    VM_MAYEXEC | VM_SHARED | VM_MAYSHARE;
	if (vma && (pgprot_val(vma->vm_page_prot) !=
		    pgprot_val(vm_get_page_prot(vm_flags))))
		return -EPERM;

	if (vma && ((vma->vm_end - vma->vm_start) !=
		    (PAGE_SIZE << RICHTAP_MMAP_PAGE_ORDER)))
		return -ENOMEM;
	phys = virt_to_phys(haptic->start_buf);
	ret = remap_pfn_range(vma, vma->vm_start, (phys >> PAGE_SHIFT),
			      (vma->vm_end - vma->vm_start), vma->vm_page_prot);
	if (ret) {
		vib_err("Error mmap failed");
		return ret;
	}

	return ret;
}

#endif // AAC_RICHTAP_SUPPORT
