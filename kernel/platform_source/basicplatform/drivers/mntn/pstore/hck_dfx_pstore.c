#include <linux/init.h>
#include <linux/printk.h>
#include <platform_include/basicplatform/linux/dfx_pstore.h>
#include <platform_include/basicplatform/linux/hck/mntn/hck_dfx_pstore.h>

static void lvh_ramoops_parse_hdt(struct platform_device *pdev,
	struct ramoops_platform_data *pdata, int *ret, int *err)
{
	*ret = true;
	*err = ramoops_parse_hdt(pdev, pdata);
}

static int __init hck_dfx_pstore_register(void)
{
	REGISTER_HCK_VH(hck_dfx_pstore_save_log, dfx_save_pstore_log);
	REGISTER_HCK_VH(hck_dfx_pstore_registe_info_to_mntndump, registe_info_to_mntndump);
	REGISTER_HCK_VH(hck_dfx_pstore_ramoops_parse_hdt, lvh_ramoops_parse_hdt);
	return 0;
}

early_initcall(hck_dfx_pstore_register);
