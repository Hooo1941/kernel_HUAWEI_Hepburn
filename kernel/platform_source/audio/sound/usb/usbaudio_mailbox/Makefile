GCOV_PROFILE := $(CONFIG_GCOV_KERNEL)
subdir-ccflags-y += -Werror -Wdate-time -Wfloat-equal -Wformat=2

EXTRA_CFLAGS += -I$(srctree)/platform_source/audio/drivers/hifi_mailbox/mailbox
EXTRA_CFLAGS += -I$(srctree)/platform_source/audio/drivers/hifi_dsp
EXTRA_CFLAGS += -I$(srctree)/include/platform_include/audio/ipc
EXTRA_CFLAGS += -I$(srctree)/platform_source/audio/include

obj-$(CONFIG_USB_AUDIO_MAILBOX) += usbaudio_mailbox.o

