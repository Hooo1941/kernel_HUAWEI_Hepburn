/*
 * HWCXEXT_MBHC.h
 *
 * HWCXEXT_MBHC header file
 *
 * Copyright (c) 2022 Huawei Technologies Co., Ltd.
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

#ifndef __HWCXEXT_MBHC__
#define __HWCXEXT_MBHC__

#include <linux/pm_wakeup.h>
#include <linux/spinlock.h>
#include <sound/soc.h>
#ifdef CONFIG_SWITCH
#include <platform_include/basicplatform/linux/switch.h>
#endif

enum audio_jack_states {
	AUDIO_JACK_NONE = 0,     /* unpluged */
	AUDIO_JACK_HEADSET,      /* pluged 4-pole headset */
	AUDIO_JACK_HEADPHONE,    /* pluged 3-pole headphone */
	AUDIO_JACK_INVERT,       /* pluged invert 4-pole headset */
	AUDIO_JACK_LINEOUT,
	AUDIO_JACK_LINEIN,
	AUDIO_JACK_MIC,
	AUDIO_JACK_PLUG_OUT,
	AUDIO_JACK_HEADSET_NOUSE,
};

enum btn_event {
	BTN_ENENT_INVAILD = 0,
	BTN_PRESS   = 0x0001,
	BTN_RELEASED = 0x0002,
};

enum port_enable {
	PORT_INVAILD = 0,
	PORTA_EN = 0x0001,
	PORTB_EN = 0x0002,
	PORTC_EN = 0x0004,
	PORTD_EN = 0x0004,
};

struct hwcxext_mbhc_priv;

struct hwcxext_mbhc_cb {
	int (*mbhc_check_headset_in)(struct hwcxext_mbhc_priv *mbhc, int timeouts);
	void (*enable_micbias)(struct hwcxext_mbhc_priv *mbhc);
	void (*disable_micbias)(struct hwcxext_mbhc_priv *mbhc);
	void (*enable_jack_detect)(struct hwcxext_mbhc_priv *mbhc);
	int (*get_hs_type_recognize)(struct hwcxext_mbhc_priv *mbhc);
	int (*get_btn_type_recognize)(struct hwcxext_mbhc_priv *mbhc, int *report, int *status);
	void (*dump_regs)(struct hwcxext_mbhc_priv *mbhc);
	void (*clear_irq)(struct hwcxext_mbhc_priv *mbhc);
	void (*set_hs_detec_restart)(struct hwcxext_mbhc_priv *mbhc);
};

/* defination of private data */
struct hwcxext_mbhc_priv {
	struct device *dev;
	struct snd_soc_component *component;
	struct wakeup_source *plug_btn_wake_lock;
	struct wakeup_source *plug_btn_irq_wake_lock;
	struct wakeup_source *plug_wake_lock;
	struct wakeup_source *btn_wake_lock;
	struct wakeup_source *long_btn_wake_lock;
	struct wakeup_source *plug_repeat_detect_lock;

	struct mutex plug_mutex;
	struct mutex status_mutex;
	struct mutex btn_mutex;
	int plug_btn_irq_gpio;
	int plug_btn_irq;
	bool need_reapeat;
	struct snd_soc_jack headset_jack;
	struct snd_soc_jack rear_jack;
	/* headset status */
	enum audio_jack_states hs_status;
	int btn_report;
	bool init_state;

#ifdef CONFIG_SWITCH
	struct switch_dev sdev;
#endif
	struct hwcxext_mbhc_cb *mbhc_cb;

	struct workqueue_struct *irq_plug_btn_wq;
	struct delayed_work irq_plug_btn_work;

	struct workqueue_struct *irq_plug_handle_wq;
	struct delayed_work hs_plug_work;
	struct workqueue_struct *irq_btn_handle_wq;
	struct delayed_work btn_delay_work;

	struct workqueue_struct *long_press_btn_wq;
	struct delayed_work long_press_btn_work;

	struct mutex micbias_vote_mutex;
	atomic_t micbias_vote_cnt;
};

#ifdef CONFIG_HWCXEXT_MBHC_PC
int hwcxext_mbhc_init(struct device *dev,
	struct snd_soc_component *component,
	struct hwcxext_mbhc_priv **mbhc_data,
	struct hwcxext_mbhc_cb *mbhc_cb);
void hwcxext_mbhc_exit(struct device *dev,
	struct hwcxext_mbhc_priv *mbhc_data);
void hwcxext_mbhc_detect_init_state(
	struct hwcxext_mbhc_priv *mbhc_data);
#else
static inline int hwcxext_mbhc_init(struct device *dev,
	struct snd_soc_component *component,
	struct hwcxext_mbhc_priv **mbhc_data,
	struct hwcxext_mbhc_cb *mbhc_cb)
{
	return 0;
}

static inline void hwcxext_mbhc_exit(struct device *dev,
	struct hwcxext_mbhc_priv *mbhc_data)
{
}

static inline void hwcxext_mbhc_detect_init_state(
	struct hwcxext_mbhc_priv *mbhc_data)
{
}
#endif
#endif // HWCXEXT_MBHC
