EXTRA_CFLAGS += -I$(srctree)/drivers/platform_drivers/tzdriver
EXTRA_CFLAGS += -I$(srctree)/platform_source/basicplatform/drivers/rpmb
EXTRA_CFLAGS += -I$(srctree)/platform_source/basicplatform/drivers/mntn/general_see
EXTRA_CFLAGS += -I$(srctree)/platform_source/see/drivers/general_see
EXTRA_CFLAGS += -I$(srctree)/platform_source/see/drivers/crypto_core/atcmd

obj-$(CONFIG_CRYPTO_CORE_APM_KCA_NOTIFY) += crypto_core_apm.o

subdir-ccflags-y              += -Wall -Werror

obj-$(CONFIG_CRYPTO_CORE_SUPPORT_DCS) += crypto_core_dcs.o

obj-$(CONFIG_CRYPTO_CORE_DISABLE_KEY) += atcmd/crypto_core_disable.o

ifeq ($(CONFIG_CRYPTO_CORE_V2), y)
    obj-y += crypto_core_v2.o \
             crypto_core_fs.o \
             crypto_core_power.o \
             crypto_core_smc.o \
             crypto_core_factory_v2.o \
             crypto_core_flash.o \
             atcmd/crypto_core_at.o \
             atcmd/crypto_core_at_install.o \
             atcmd/crypto_core_at_install_v2.o \
             atcmd/crypto_core_factory_task.o
else
    obj-y += crypto_core.o \
             crypto_core_fs.o \
             crypto_core_power.o \
             crypto_core_smc.o \
             crypto_core_upgrade.o \
             crypto_core_factory.o \
             crypto_core_flash.o \
             crypto_core_smx.o \
             atcmd/crypto_core_at.o \
             atcmd/crypto_core_at_install.o \
             atcmd/crypto_core_at_install_v1.o \
             atcmd/crypto_core_at_smx.o \
             atcmd/crypto_core_at_socslt.o \
             atcmd/crypto_core_at_test.o \
             atcmd/crypto_core_factory_task.o
    ifeq ($(CONFIG_DFX_DEBUG_FS), y)
        obj-y += crypto_core_autotest.o
    endif

    EXTRA_CFLAGS += -DCRYPTO_CORE_SUPPORT_AT_SMX \
                    -DCRYPTO_CORE_SUPPORT_AT_SOCSLT \
                    -DCRYPTO_CORE_SUPPORT_AT_TEST \
                    -DCRYPTO_CORE_SUPPORT_UPGRADE
endif
