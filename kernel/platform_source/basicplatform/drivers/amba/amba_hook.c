#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/of.h>
#include <linux/version.h>
#include <linux/amba/bus.h>
#include <linux/arm-smccc.h>
#include <platform_include/basicplatform/linux/hck/amba/hck_amba.h>

#define HISI_SECURE_GPIO_READ_REG   0xc3001106
#define PID_CID_REG_NUM 4
#define PID_CID_MASK 255
#define PID_CID_OFFSET 8
#define PID_CID_REG_STEP 4
#define PID_REG_START 0x20
#define CID_REG_START 0x10


static unsigned int amba_secure_readl(int offset, u32 base_addr)
{
	struct arm_smccc_res res;

	arm_smccc_1_1_smc(HISI_SECURE_GPIO_READ_REG, offset, base_addr, &res);
	return (u32)res.a1;
}

static void hck_read_amba_id(struct amba_device *dev, u32 size, u32 *pid, u32 *cid, int *hck_ret)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
	int i;
	*hck_ret = 0;

	(void)of_property_read_u32(dev->dev.of_node, "secure-mode", (u32 *)&dev->secure_mode);
	if (dev->secure_mode) {
		for (*pid = 0, i = 0; i < PID_CID_REG_NUM; i++)
			*pid |= (amba_secure_readl((size - PID_REG_START + PID_CID_REG_STEP * i), dev->res.start)
						& PID_CID_MASK) << (i * PID_CID_OFFSET);
		for (*cid = 0, i = 0; i < PID_CID_REG_NUM; i++)
			*cid |= (amba_secure_readl((size - CID_REG_START + PID_CID_REG_STEP * i), dev->res.start)
						& PID_CID_MASK) << (i * PID_CID_OFFSET);
		*hck_ret = 1;
	}
#endif
}

static int hook_amba_init(void)
{
	REGISTER_HCK_VH(read_amba_id, hck_read_amba_id);
	return 0;
}
early_initcall(hook_amba_init);
