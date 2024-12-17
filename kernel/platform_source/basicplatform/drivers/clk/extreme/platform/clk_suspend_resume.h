#ifndef __CLK_SUSPEND_RESUME_H
#define __CLK_SUSPEND_RESUME_H

/* Base addr of register */
#define SCTRL_PHY_ADDR 0xFB21B000
#define CRG_PHY_ADDR   0xFFB85000

/* mask of write regiter */
#define MASK_ALL	0xFFFF0000

/* offset addr of SCTRL REG */
#define CLK_DIV_AO 0x260
#define CLK_MUX_AO 0x290
/* offset addr of  PERIL REG */
#define FNPLL_CFG0 0xA00
#define FNPLL_CFG1 0xA04
#define FNPLL_CFG2 0xA08
#define FNPLL_CFG3 0xA0C
#define PPLL1CTRL1 0xA70
#define PPLL2CTRL1 0xA78
#define PPLL3CTRL1 0xA80
#define PPLL7CTRL1 0x83C
#define PPLL1CTRL0 0xA6C
#define PPLL2CTRL0 0xA74
#define PPLL3CTRL0 0xA7C
#define PPLL7CTRL0 0x838
#define PLL_FSM_CTRL1 0xC44
#define PLL_FSM_CTRL0 0xC40
#define PERRSTSTAT0 0x068
#define PERRSTSTAT1 0x074
#define PERRSTSTAT2 0x080
#define PERRSTSTAT3 0x08C
#define PERRSTSTAT4 0x098
#define PERRSTSTAT5 0x0A4
#define GENERAL_SEC_RSTSTAT 0xD08
#define GENERAL_SEC_RSTSTAT1 0xD14
#define GENERAL_SEC_RSTSTAT2 0xD20
#define PERCLKEN0 0x008
#define PERCLKEN1 0x018
#define PERCLKEN2 0x028
#define PERCLKEN3 0x038
#define PERCLKEN4 0x048
#define PERCLKEN5 0x058
#define PERCLKEN6 0x418
#define PERCLKEN7 0x428
#define PERCLKEN8 0x438
#define PERCLKEN9 0x448
#define PERCLKEN10 0x458
#define PERCLKEN11 0x468
#define PERCLKEN12 0x478
#define PERCLKEN13 0x488
#define PERCLKEN14 0x538
#define GENERAL_SEC_PERCLKEN 0xE08
#define GENERAL_SEC_PERCLKEN1 0xE18
#define GENERAL_SEC_PERCLKEN2 0xE28
#define GENERAL_SEC_PERCLKEN3 0xE38
#define CLKDIV0               0x0A8
#define CLKDIV1               0x0AC
#define CLKDIV2               0x0B0
#define CLKDIV3               0x0B4
#define CLKDIV4               0x0B8
#define CLKDIV5               0x0BC
#define CLKDIV6               0x0C0
#define CLKDIV7               0x0C4
#define CLKDIV8               0x0C8
#define CLKDIV9               0x0CC
#define CLKDIV10              0x0D0
#define CLKDIV11              0x0D4
#define CLKDIV12              0x0D8
#define CLKDIV13              0x0DC
#define CLKDIV14              0x0E0
#define CLKDIV15              0x0E4
#define CLKDIV16              0x0E8
#define CLKDIV17              0x0EC
#define CLKDIV18              0x0F0
#define CLKDIV19              0x0F4
#define CLKDIV20              0x0F8
#define CLKDIV21              0x0FC
#define CLKDIV22              0x100
#define CLKDIV23              0x104
#define CLKDIV24              0x108
#define CLKDIV25              0x10C
#define CLKDIV26              0x700
#define CLKDIV27              0x704
#define CLKDIV28              0x708
#define CLKDIV29              0x70C
#define CLKDIV30              0x710
#define CLKDIV31              0x714
#define CLKDIV32              0x718
#define GENERAL_SEC_CLKDIV0   0xB00
#define GENERAL_SEC_CLKDIV1   0xB04
#define GENERAL_SEC_CLKDIV2   0xB08
#define GENERAL_SEC_CLKDIV3   0xB0C
#define GENERAL_SEC_CLKDIV4   0xB10
#define GENERAL_SEC_CLKDIV5   0xB14
#define GENERAL_SEC_CLKDIV6   0xB18
#define GENERAL_SEC_CLKDIV7   0xB1C
#define GENERAL_SEC_CLKDIV8   0xB20
#define GENERAL_SEC_CLKDIV9   0xB24
#define GENERAL_SEC_CLKDIV10  0xB28
#define PPLL1SSCCTRL 0xA9C
#define PPLL2SSCCTRL 0xAA0
#define PPLL3SSCCTRL 0xAA4
#define PPLL7SSCCTRL 0x85C

#define crgperi_reg(x) ((x) + CRG_PHY_ADDR)
#define crgsctrl_reg(x) ((x) + SCTRL_PHY_ADDR)

#define array_size(arr)		(sizeof(arr) / sizeof((arr)[0]))

int clk_hibernate_notify_suspend(struct notifier_block *nb,unsigned long mode,void *_unused);
int clk_hibernate_notify_resume(struct notifier_block *nb,unsigned long mode,void *_unused);

#endif /* __CLK_SUSPEND_RESUME_H */