#include <linux/types.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <platform_include/basicplatform/linux/rdr_platform.h>
#include <platform_include/basicplatform/linux/dfx_poweroff.h>
#include <platform_include/basicplatform/linux/hck/mntn/hck_dfx_poweroff.h>
#include <linux/version.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0))
extern enum reboot_mode reboot_mode;

static void arm_pm_hrestart(int *ret, char *cmd)
{
	arm_pm_restart_func restart = get_arm_pm_restart_func();
	if (restart) {
		restart(reboot_mode, cmd);
		*ret = true;
	}
}

static int __init hck_dfx_poweroff_register(void)
{
	REGISTER_HCK_VH(hck_dfx_poweroff_system_reset_comm, dfx_pm_system_reset_comm);
	REGISTER_HCK_VH(hck_dfx_poweroff_arm_pm_hrestart, arm_pm_hrestart);
	return 0;
}
early_initcall(hck_dfx_poweroff_register);
#endif
