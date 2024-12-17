obj-$(CONFIG_HKIP_PRMEM)          += prmem.o prmem_sysfs.o prmem_phys_db.o
obj-$(CONFIG_DEBUG_HKIP_PRMEM)    += test_prmem.o
obj-$(CONFIG_HKIP_PROTECT_CRED)   += cred_harden/hkip_cred.o
obj-$(CONFIG_HKIP_CFI_HARDEN)     += cfi_harden/cfi_harden.o
obj-$(CONFIG_DEBUG_CFI_HARDEN)    += cfi_harden/cfi_harden_test.o
obj-$(CONFIG_DEBUG_CFI_HARDEN_KO) += cfi_harden/cfi_harden_test_ko.o

subdir-ccflags-y += -Wdate-time -Wextra -Wfloat-equal -Wformat=2 -Wshadow
