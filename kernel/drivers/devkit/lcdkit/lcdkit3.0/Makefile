ifeq ($(CONFIG_DKMD_ENABLE),y)
EXTRA_CFLAGS += -DLCD_KIT_DKMD
obj-$(CONFIG_LCD_KIT_HISI)	+= kernel/dkmd/src/
obj-$(CONFIG_LCD_KIT_HISI) 	+= kernel/dkmd/adapt/
obj-$(CONFIG_LCD_KIT_HISI)	+= kernel/common/src/
obj-$(CONFIG_LCD_KIT_HISI)	+= kernel/dkmd/displayengine/
else
ifeq ($(CONFIG_DPU_FB_V346),y)
EXTRA_CFLAGS += -DLCD_KIT_ALPHA_LHBM
obj-$(CONFIG_LCD_KIT_HISI)	+= kernel/hisi/displayengine/
endif
obj-$(CONFIG_LCD_KIT_HISI)	+= kernel/hisi/src/
obj-$(CONFIG_LCD_KIT_HISI) 	+= kernel/hisi/adapt/
obj-$(CONFIG_LCD_KIT_HISI)	+= kernel/common/src/
obj-$(CONFIG_LCD_KIT_HISI)	+= kernel/common/displayengine/
endif

obj-$(CONFIG_LCD_KIT_MTK)	+= kernel/common/src/
obj-$(CONFIG_LCD_KIT_MTK)	+= kernel/mtk/src/
obj-$(CONFIG_LCD_KIT_MTK)	+= kernel/mtk/adapt/

obj-$(CONFIG_LCD_KIT_QCOM)	+= kernel/qcom/common/src/
obj-$(CONFIG_LCD_KIT_QCOM)	+= kernel/qcom/src/
obj-$(CONFIG_LCD_KIT_QCOM)	+= kernel/qcom/adapt/
obj-$(CONFIG_LCD_KIT_QCOM)	+= kernel/common/displayengine/

