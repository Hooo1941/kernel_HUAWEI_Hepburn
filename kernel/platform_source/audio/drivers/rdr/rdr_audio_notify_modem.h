/*
 * rdr_audio_notify_modem.h
 *
 * dsp reset notify modem
 *
 * Copyright (c) 2019-2020 Huawei Technologies CO., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef RDR_AUDIO_NOTIFY_MODEM_H
#define RDR_AUDIO_NOTIFY_MODEM_H

enum dsp_reset_state {
	DSP_RESET_STATE_OFF = 0,
	DSP_RESET_STATE_READY,
	DSP_RESET_STATE_INVALID
};

void dsp_reset_notify_modem(enum dsp_reset_state state);
void dsp_reset_dev_deinit(void);
int dsp_reset_dev_init(void);

#endif /* __RDR_AUDIO_NOTIFY_MODEM_H__ */

