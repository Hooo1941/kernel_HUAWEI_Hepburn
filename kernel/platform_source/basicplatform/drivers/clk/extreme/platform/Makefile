ifeq ($(CONFIG_BALT_IP_PLATFORM),y)
obj-$(CONFIG_PLAT_CLK)                  += clk-balt-cs.o
endif

ifeq ($(CONFIG_PHOE_IP_PLATFORM),y)
ifeq ($(product_type),cdc)
ifeq ($(chip_type), cs2)
obj-$(CONFIG_PLAT_CLK)                  += clk-phoe-cs2.o
else
obj-$(CONFIG_PLAT_CLK)                  += clk-phoe-cs.o
endif
endif
ifeq ($(product_type),cdc_ace)
obj-$(CONFIG_PLAT_CLK)                  += clk-phoe-ace.o
endif
endif

ifeq ($(CONFIG_BURB_IP_PLATFORM),y)
obj-$(CONFIG_PLAT_CLK)                  += clk-bur.o
endif

ifeq ($(CONFIG_CHAR_IP_PLATFORM),y)
ifeq ($(chip_type), es)
obj-$(CONFIG_PLAT_CLK)                  += clk-char.o
else
ifeq ($(CONFIG_HIBERNATION),y)
obj-$(CONFIG_PLAT_CLK)                  += clk-char-cs.o clk_suspend_resume.o
else
obj-$(CONFIG_PLAT_CLK)                  += clk-char-cs.o
endif
endif
endif

ifeq ($(CONFIG_LEX_IP_PLATFORM),y)
obj-$(CONFIG_PLAT_CLK)                  += clk-lex.o
endif

ifeq ($(CONFIG_NA_IP_PLATFORM),y)
obj-$(CONFIG_PLAT_CLK)                  += clk-na.o
endif

ifeq ($(CONFIG_ATHT_IP_PLATFORM),y)
obj-$(CONFIG_PLAT_CLK)                  += clk-atht.o
endif

ifeq ($(CONFIG_CHARPRO_IP_PLATFORM),y)
ifeq ($(product_type),cdc_ace)
obj-$(CONFIG_PLAT_CLK)                  += clk-charpro-ace.o
else
obj-$(CONFIG_PLAT_CLK)                  += clk-charpro.o
endif
endif

ifeq ($(CONFIG_NSV_IP_PLATFORM),y)
obj-$(CONFIG_PLAT_CLK)                  += clk-nsv.o
endif

ifeq ($(CONFIG_XA_IP_PLATFORM),y)
obj-$(CONFIG_PLAT_CLK)                  += clk-xa.o
endif
