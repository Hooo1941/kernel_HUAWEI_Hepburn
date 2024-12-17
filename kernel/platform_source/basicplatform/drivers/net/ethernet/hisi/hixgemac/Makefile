ccflags-y += -Wextra -Wdate-time -Wfloat-equal -Wformat=2 -Wshadow

obj-y += hixgemac_desc.o hixgemac_mac.o hixgemac_tlu_tx.o hixgemac_tlu_rx.o hixgemac_app.o hixgemac_mdio.o hixgemac_core.o

ifeq ($(CONFIG_NET_VENDOR_HIXGEMAC_10CC),y)
obj-y += hi10cc_fw.o hixgemac_pma_10cc.o
obj-$(CONFIG_DFX_DEBUG_FS) += hi10cc_dfx_dump.o hi10cc_dfx_dump_cp.o hi10cc_dfx_dump_cs.o hi10cc_dfx_dump_tx.o hi10cc_dfx_dump_rx.o hi10cc_dfx.o hi10cc_dfx_atb.o hi10cc_debugfs.o
else
obj-y += hixgemac_pma_gth.o
endif

obj-$(CONFIG_DFX_DEBUG_FS) += hixgemac_debugfs.o hixgemac_loopback.o hixgemac_dfx.o

obj-y += hixgemac.o
