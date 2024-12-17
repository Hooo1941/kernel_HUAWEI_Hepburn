#ifndef __SOC_SCTRL_INTERFACE_H__
#define __SOC_SCTRL_INTERFACE_H__ 
#ifdef __cplusplus
#if __cplusplus
    extern "C" {
#endif
#endif
#ifndef __SOC_H_FOR_ASM__
#define SOC_SCTRL_SCCTRL_ADDR(base) ((base) + (0xF00UL))
#define SOC_SCTRL_SCSYSSTAT_ADDR(base) ((base) + (0xF04UL))
#define SOC_SCTRL_SCSYSSTAT_SHADOW_ADDR(base) ((base) + (0x004UL))
#define SOC_SCTRL_SC_SCLK_SW2FLL_CTRL_ADDR(base) ((base) + (0xF50UL))
#define SOC_SCTRL_SC_SCLK_SW2FLL_STAT_ADDR(base) ((base) + (0xF54UL))
#define SOC_SCTRL_SC_PERI_POWER_STAT_ADDR(base) ((base) + (0xF58UL))
#define SOC_SCTRL_SCPERIISOBYPASS_ADDR(base) ((base) + (0xF5CUL))
#define SOC_SCTRL_SC_TCXO_CTRL_0_ADDR(base) ((base) + (0xF60UL))
#define SOC_SCTRL_SC_TCXO_CTRL_1_ADDR(base) ((base) + (0xF64UL))
#define SOC_SCTRL_SC_TCXO_CTRL_2_ADDR(base) ((base) + (0xF68UL))
#define SOC_SCTRL_SC_TCXO_CTRL_3_ADDR(base) ((base) + (0xF6CUL))
#define SOC_SCTRL_SC_PERI_POWER_CTRL_0_ADDR(base) ((base) + (0xF70UL))
#define SOC_SCTRL_SC_PERI_POWER_CTRL_1_ADDR(base) ((base) + (0xF74UL))
#define SOC_SCTRL_SC_PERI_POWER_CTRL_2_ADDR(base) ((base) + (0xF78UL))
#define SOC_SCTRL_SC_PERI_POWER_CTRL_3_ADDR(base) ((base) + (0xF7CUL))
#define SOC_SCTRL_SC_RESUME_WATCHDOG_CTRL_ADDR(base) ((base) + (0xF80UL))
#define SOC_SCTRL_SCDEEPSLEEPED_ADDR(base) ((base) + (0x008UL))
#define SOC_SCTRL_SC_LP_FLAG_ADDR(base) ((base) + (0x018UL))
#define SOC_SCTRL_SC_INTR_WAKEUP_CTRL_ADDR(base) ((base) + (0x014UL))
#define SOC_SCTRL_SCEFUSEDATA2_ADDR(base) ((base) + (0x00CUL))
#define SOC_SCTRL_SCEFUSEDATA3_ADDR(base) ((base) + (0x010UL))
#define SOC_SCTRL_SCEFUSEDATA8_ADDR(base) ((base) + (0x060UL))
#define SOC_SCTRL_SCEFUSEDATA9_ADDR(base) ((base) + (0x064UL))
#define SOC_SCTRL_SCLPSTATCFG_ADDR(base) ((base) + (0x038UL))
#define SOC_SCTRL_SC_ASP_PWRCFG0_ADDR(base) ((base) + (0x080UL))
#define SOC_SCTRL_SC_ASP_PWRCFG1_ADDR(base) ((base) + (0x084UL))
#define SOC_SCTRL_SCINT_GATHER_STAT_ADDR(base) ((base) + (0x0A0UL))
#define SOC_SCTRL_SCINT_MASK_ADDR(base) ((base) + (0x0A4UL))
#define SOC_SCTRL_SCINT_STAT_ADDR(base) ((base) + (0x0A8UL))
#define SOC_SCTRL_SCDRX_INT_CFG_ADDR(base) ((base) + (0x0ACUL))
#define SOC_SCTRL_SCLPMCUWFI_INT_ADDR(base) ((base) + (0x0B0UL))
#define SOC_SCTRL_SCINT_MASK1_ADDR(base) ((base) + (0x0B4UL))
#define SOC_SCTRL_SCINT_STAT1_ADDR(base) ((base) + (0x0B8UL))
#define SOC_SCTRL_SCINT_MASK2_ADDR(base) ((base) + (0x0BCUL))
#define SOC_SCTRL_SCINT_STAT2_ADDR(base) ((base) + (0x0C0UL))
#define SOC_SCTRL_SCINT_MASK3_ADDR(base) ((base) + (0x0C4UL))
#define SOC_SCTRL_SCINT_STAT3_ADDR(base) ((base) + (0x0C8UL))
#define SOC_SCTRL_SCINT_MASK4_ADDR(base) ((base) + (0x090UL))
#define SOC_SCTRL_SCINT_STAT4_ADDR(base) ((base) + (0x094UL))
#define SOC_SCTRL_SCINT_MASK5_ADDR(base) ((base) + (0x098UL))
#define SOC_SCTRL_SCINT_STAT5_ADDR(base) ((base) + (0x09CUL))
#define SOC_SCTRL_SC_PLL_EN_VOTE_0_ADDR(base) ((base) + (0xF08UL))
#define SOC_SCTRL_SC_PLL_EN_VOTE_1_ADDR(base) ((base) + (0x0D4UL))
#define SOC_SCTRL_SC_PLL_EN_VOTE_2_ADDR(base) ((base) + (0x0D8UL))
#define SOC_SCTRL_SC_PLL_EN_VOTE_3_ADDR(base) ((base) + (0x0DCUL))
#define SOC_SCTRL_SC_PLL_GT_VOTE_0_ADDR(base) ((base) + (0xF0CUL))
#define SOC_SCTRL_SC_PLL_GT_VOTE_1_ADDR(base) ((base) + (0x234UL))
#define SOC_SCTRL_SC_PLL_GT_VOTE_2_ADDR(base) ((base) + (0x238UL))
#define SOC_SCTRL_SC_PLL_GT_VOTE_3_ADDR(base) ((base) + (0x23CUL))
#define SOC_SCTRL_SC_PLL_BP_VOTE_0_ADDR(base) ((base) + (0xF10UL))
#define SOC_SCTRL_SC_PLL_BP_VOTE_1_ADDR(base) ((base) + (0x244UL))
#define SOC_SCTRL_SC_PLL_BP_VOTE_2_ADDR(base) ((base) + (0x248UL))
#define SOC_SCTRL_SC_PLL_BP_VOTE_3_ADDR(base) ((base) + (0x24CUL))
#define SOC_SCTRL_SC_PLL_STAT_ADDR(base) ((base) + (0x0CCUL))
#define SOC_SCTRL_SCSPLLCTRL0_ADDR(base) ((base) + (0xF14UL))
#define SOC_SCTRL_SCSPLLCTRL1_ADDR(base) ((base) + (0xF18UL))
#define SOC_SCTRL_SCSPLLSSCCTRL_ADDR(base) ((base) + (0xF1CUL))
#define SOC_SCTRL_SCSPLLCTRL0_LOCK_STAT_ADDR(base) ((base) + (0xF20UL))
#define SOC_SCTRL_SCSPLLCTRL1_LOCK_STAT_ADDR(base) ((base) + (0xF24UL))
#define SOC_SCTRL_SCSPLLSSCCTRL_LOCK_STAT_ADDR(base) ((base) + (0xF28UL))
#define SOC_SCTRL_SCSPLL_OTHER_LOCK_STAT_ADDR(base) ((base) + (0xF2CUL))
#define SOC_SCTRL_FNPLL_CFG0_ADDR(base) ((base) + (0xF30UL))
#define SOC_SCTRL_FNPLL_CFG1_ADDR(base) ((base) + (0xF34UL))
#define SOC_SCTRL_FNPLL_CFG2_ADDR(base) ((base) + (0xF38UL))
#define SOC_SCTRL_FNPLL_CFG3_ADDR(base) ((base) + (0xF3CUL))
#define SOC_SCTRL_FNPLL_STATE0_ADDR(base) ((base) + (0x1E8UL))
#define SOC_SCTRL_SC_ULPPLL_GT_CFG_ADDR(base) ((base) + (0x088UL))
#define SOC_SCTRL_SC_ULPPLL_1_GT_CFG_ADDR(base) ((base) + (0x08CUL))
#define SOC_SCTRL_SC_ULPPLL_CFG0_ADDR(base) ((base) + (0x110UL))
#define SOC_SCTRL_SC_ULPPLL_CFG1_ADDR(base) ((base) + (0x114UL))
#define SOC_SCTRL_SC_ULPPLL_CFG2_ADDR(base) ((base) + (0x118UL))
#define SOC_SCTRL_SC_ULPPLL_CTRL0_ADDR(base) ((base) + (0x11CUL))
#define SOC_SCTRL_SC_ULPPLL_STATE0_ADDR(base) ((base) + (0x120UL))
#define SOC_SCTRL_SC_ULPPLL_1_CFG0_ADDR(base) ((base) + (0x140UL))
#define SOC_SCTRL_SC_ULPPLL_1_CFG1_ADDR(base) ((base) + (0x144UL))
#define SOC_SCTRL_SC_ULPPLL_1_CFG2_ADDR(base) ((base) + (0x148UL))
#define SOC_SCTRL_SC_ULPPLL_1_CTRL0_ADDR(base) ((base) + (0x14CUL))
#define SOC_SCTRL_SC_ULPPLL_1_STATE0_ADDR(base) ((base) + (0x150UL))
#define SOC_SCTRL_SCAUPLLCTRL0_ADDR(base) ((base) + (0x490UL))
#define SOC_SCTRL_SCAUPLLCTRL1_ADDR(base) ((base) + (0x494UL))
#define SOC_SCTRL_SCAUPLLSSCCTRL_ADDR(base) ((base) + (0x498UL))
#define SOC_SCTRL_AUFNPLL_CFG0_ADDR(base) ((base) + (0x49CUL))
#define SOC_SCTRL_AUFNPLL_CFG1_ADDR(base) ((base) + (0x4A0UL))
#define SOC_SCTRL_AUFNPLL_CFG2_ADDR(base) ((base) + (0x4A4UL))
#define SOC_SCTRL_AUFNPLL_CFG3_ADDR(base) ((base) + (0x4A8UL))
#define SOC_SCTRL_AUFNPLL_STATE0_ADDR(base) ((base) + (0x4B4UL))
#define SOC_SCTRL_SCPEREN0_ADDR(base) ((base) + (0x160UL))
#define SOC_SCTRL_SCPERDIS0_ADDR(base) ((base) + (0x164UL))
#define SOC_SCTRL_SCPERCLKEN0_ADDR(base) ((base) + (0x168UL))
#define SOC_SCTRL_SCPERSTAT0_ADDR(base) ((base) + (0x16CUL))
#define SOC_SCTRL_SCPEREN1_ADDR(base) ((base) + (0x170UL))
#define SOC_SCTRL_SCPERDIS1_ADDR(base) ((base) + (0x174UL))
#define SOC_SCTRL_SCPERCLKEN1_ADDR(base) ((base) + (0x178UL))
#define SOC_SCTRL_SCPERSTAT1_ADDR(base) ((base) + (0x17CUL))
#define SOC_SCTRL_SCPEREN2_ADDR(base) ((base) + (0x190UL))
#define SOC_SCTRL_SCPERDIS2_ADDR(base) ((base) + (0x194UL))
#define SOC_SCTRL_SCPERCLKEN2_ADDR(base) ((base) + (0x198UL))
#define SOC_SCTRL_SCPERSTAT2_ADDR(base) ((base) + (0x19CUL))
#define SOC_SCTRL_SCPEREN3_ADDR(base) ((base) + (0x1A0UL))
#define SOC_SCTRL_SCPERDIS3_ADDR(base) ((base) + (0x1A4UL))
#define SOC_SCTRL_SCPERCLKEN3_ADDR(base) ((base) + (0x1A8UL))
#define SOC_SCTRL_SCPERSTAT3_ADDR(base) ((base) + (0x1ACUL))
#define SOC_SCTRL_SCPEREN4_ADDR(base) ((base) + (0x1B0UL))
#define SOC_SCTRL_SCPERDIS4_ADDR(base) ((base) + (0x1B4UL))
#define SOC_SCTRL_SCPERCLKEN4_ADDR(base) ((base) + (0x1B8UL))
#define SOC_SCTRL_SCPERSTAT4_ADDR(base) ((base) + (0x1BCUL))
#define SOC_SCTRL_SCPEREN5_ADDR(base) ((base) + (0x1C0UL))
#define SOC_SCTRL_SCPERDIS5_ADDR(base) ((base) + (0x1C4UL))
#define SOC_SCTRL_SCPERCLKEN5_ADDR(base) ((base) + (0x1C8UL))
#define SOC_SCTRL_SCPERSTAT5_ADDR(base) ((base) + (0x1CCUL))
#define SOC_SCTRL_SC_AO_CRG_STA_SEP0_ADDR(base) ((base) + (0x188UL))
#define SOC_SCTRL_SCPERRSTEN0_ADDR(base) ((base) + (0x200UL))
#define SOC_SCTRL_SCPERRSTDIS0_ADDR(base) ((base) + (0x204UL))
#define SOC_SCTRL_SCPERRSTSTAT0_ADDR(base) ((base) + (0x208UL))
#define SOC_SCTRL_SCPERRSTEN1_ADDR(base) ((base) + (0x20CUL))
#define SOC_SCTRL_SCPERRSTDIS1_ADDR(base) ((base) + (0x210UL))
#define SOC_SCTRL_SCPERRSTSTAT1_ADDR(base) ((base) + (0x214UL))
#define SOC_SCTRL_SCPERRSTEN2_ADDR(base) ((base) + (0x218UL))
#define SOC_SCTRL_SCPERRSTDIS2_ADDR(base) ((base) + (0x21CUL))
#define SOC_SCTRL_SCPERRSTSTAT2_ADDR(base) ((base) + (0x220UL))
#define SOC_SCTRL_SCCLKDIV0_ADDR(base) ((base) + (0x250UL))
#define SOC_SCTRL_SCCLKDIV1_ADDR(base) ((base) + (0x254UL))
#define SOC_SCTRL_SCCLKDIV2_ADDR(base) ((base) + (0x258UL))
#define SOC_SCTRL_SCCLKDIV3_ADDR(base) ((base) + (0x25CUL))
#define SOC_SCTRL_SCCLKDIV4_ADDR(base) ((base) + (0x260UL))
#define SOC_SCTRL_SCCLKDIV5_ADDR(base) ((base) + (0x264UL))
#define SOC_SCTRL_SCCLKDIV6_ADDR(base) ((base) + (0x268UL))
#define SOC_SCTRL_SCCLKDIV7_ADDR(base) ((base) + (0x26CUL))
#define SOC_SCTRL_SCCLKDIV8_ADDR(base) ((base) + (0x270UL))
#define SOC_SCTRL_SCCLKDIV9_ADDR(base) ((base) + (0x274UL))
#define SOC_SCTRL_SCUFS_AUTODIV_ADDR(base) ((base) + (0x278UL))
#define SOC_SCTRL_SC_INTR_NOCBUS_CTRL_ADDR(base) ((base) + (0x27CUL))
#define SOC_SCTRL_SCCLKDIV10_ADDR(base) ((base) + (0x280UL))
#define SOC_SCTRL_SCCLKDIV11_ADDR(base) ((base) + (0x284UL))
#define SOC_SCTRL_SCCLKDIV12_ADDR(base) ((base) + (0x288UL))
#define SOC_SCTRL_SCCLKDIV13_ADDR(base) ((base) + (0x28CUL))
#define SOC_SCTRL_SCCLKDIV14_ADDR(base) ((base) + (0x290UL))
#define SOC_SCTRL_SCCLKDIV15_ADDR(base) ((base) + (0x294UL))
#define SOC_SCTRL_SCCLKDIV16_ADDR(base) ((base) + (0x298UL))
#define SOC_SCTRL_SCCLKDIV17_ADDR(base) ((base) + (0x29CUL))
#define SOC_SCTRL_SCCLKDIV18_ADDR(base) ((base) + (0x2A0UL))
#define SOC_SCTRL_SCCLKDIV19_ADDR(base) ((base) + (0x2A4UL))
#define SOC_SCTRL_SCCLKDIV20_ADDR(base) ((base) + (0x2A8UL))
#define SOC_SCTRL_SCCLKDIV21_ADDR(base) ((base) + (0x2ACUL))
#define SOC_SCTRL_SCCLKDIV22_ADDR(base) ((base) + (0x2B0UL))
#define SOC_SCTRL_SC_PLL_FSM_NS_VOTE0_ADDR(base) ((base) + (0x2B4UL))
#define SOC_SCTRL_SCPERCTRL0_ADDR(base) ((base) + (0x300UL))
#define SOC_SCTRL_SCPERCTRL1_ADDR(base) ((base) + (0x304UL))
#define SOC_SCTRL_SCPERCTRL2_ADDR(base) ((base) + (0x308UL))
#define SOC_SCTRL_SCPERCTRL3_ADDR(base) ((base) + (0x30CUL))
#define SOC_SCTRL_SCPERCTRL4_ADDR(base) ((base) + (0x310UL))
#define SOC_SCTRL_SCPERCTRL5_ADDR(base) ((base) + (0x314UL))
#define SOC_SCTRL_SCPERCTRL6_ADDR(base) ((base) + (0x318UL))
#define SOC_SCTRL_SCPERCTRL7_ADDR(base) ((base) + (0x31CUL))
#define SOC_SCTRL_SCPERCTRL9_ADDR(base) ((base) + (0x324UL))
#define SOC_SCTRL_SCPERCTRL10_ADDR(base) ((base) + (0x328UL))
#define SOC_SCTRL_SCPERCTRL11_ADDR(base) ((base) + (0x32CUL))
#define SOC_SCTRL_SCPERCTRL12_ADDR(base) ((base) + (0x330UL))
#define SOC_SCTRL_SCPERCTRL13_ADDR(base) ((base) + (0x334UL))
#define SOC_SCTRL_SCPERCTRL14_ADDR(base) ((base) + (0x338UL))
#define SOC_SCTRL_SCPERCTRL15_ADDR(base) ((base) + (0x33CUL))
#define SOC_SCTRL_SCPERCTRL16_ADDR(base) ((base) + (0x340UL))
#define SOC_SCTRL_SCPERCTRL17_ADDR(base) ((base) + (0x344UL))
#define SOC_SCTRL_SC_INTR_NOCBUS_STAT_ADDR(base) ((base) + (0x354UL))
#define SOC_SCTRL_SCPERSTATUS17_ADDR(base) ((base) + (0x358UL))
#define SOC_SCTRL_SCPERSTATUS16_ADDR(base) ((base) + (0x35CUL))
#define SOC_SCTRL_SCPERSTATUS0_ADDR(base) ((base) + (0x360UL))
#define SOC_SCTRL_SCPERSTATUS1_ADDR(base) ((base) + (0x364UL))
#define SOC_SCTRL_SCPERSTATUS2_ADDR(base) ((base) + (0x368UL))
#define SOC_SCTRL_SCPERSTATUS3_ADDR(base) ((base) + (0x36CUL))
#define SOC_SCTRL_SCPERSTATUS4_ADDR(base) ((base) + (0x370UL))
#define SOC_SCTRL_SCPERSTATUS5_ADDR(base) ((base) + (0x374UL))
#define SOC_SCTRL_SCPERSTATUS6_ADDR(base) ((base) + (0x378UL))
#define SOC_SCTRL_SCPERSTATUS7_ADDR(base) ((base) + (0x37CUL))
#define SOC_SCTRL_SCPERSTATUS8_ADDR(base) ((base) + (0x380UL))
#define SOC_SCTRL_SCPERSTATUS9_ADDR(base) ((base) + (0x384UL))
#define SOC_SCTRL_SCPERSTATUS10_ADDR(base) ((base) + (0x388UL))
#define SOC_SCTRL_SCPERSTATUS11_ADDR(base) ((base) + (0x38CUL))
#define SOC_SCTRL_SCPERSTATUS12_ADDR(base) ((base) + (0x390UL))
#define SOC_SCTRL_SCPERSTATUS13_ADDR(base) ((base) + (0x394UL))
#define SOC_SCTRL_SCPERSTATUS14_ADDR(base) ((base) + (0x398UL))
#define SOC_SCTRL_SCPERSTATUS15_ADDR(base) ((base) + (0x39CUL))
#define SOC_SCTRL_SCINNERSTAT_ADDR(base) ((base) + (0x3A0UL))
#define SOC_SCTRL_SCINNERCTRL_ADDR(base) ((base) + (0x3A4UL))
#define SOC_SCTRL_SC_SECOND_INT_MASK_ADDR(base) ((base) + (0x3D0UL))
#define SOC_SCTRL_SC_SECOND_INT_ORG_ADDR(base) ((base) + (0x3D4UL))
#define SOC_SCTRL_SC_SECOND_INT_OUT_ADDR(base) ((base) + (0x3D8UL))
#define SOC_SCTRL_SCMRBBUSYSTAT_ADDR(base) ((base) + (0x3FCUL))
#define SOC_SCTRL_SCSWADDR_ADDR(base) ((base) + (0x400UL))
#define SOC_SCTRL_SCDDRADDR_ADDR(base) ((base) + (0x404UL))
#define SOC_SCTRL_SCDDRDATA_ADDR(base) ((base) + (0x408UL))
#define SOC_SCTRL_SCBAKDATA0_ADDR(base) ((base) + (0x40CUL))
#define SOC_SCTRL_SCBAKDATA1_ADDR(base) ((base) + (0x410UL))
#define SOC_SCTRL_SCBAKDATA2_ADDR(base) ((base) + (0x414UL))
#define SOC_SCTRL_SCBAKDATA3_ADDR(base) ((base) + (0x418UL))
#define SOC_SCTRL_SCBAKDATA4_ADDR(base) ((base) + (0x41CUL))
#define SOC_SCTRL_SCBAKDATA5_ADDR(base) ((base) + (0x420UL))
#define SOC_SCTRL_SCBAKDATA6_ADDR(base) ((base) + (0x424UL))
#define SOC_SCTRL_SCBAKDATA7_ADDR(base) ((base) + (0x428UL))
#define SOC_SCTRL_SCBAKDATA8_ADDR(base) ((base) + (0x42CUL))
#define SOC_SCTRL_SCBAKDATA9_ADDR(base) ((base) + (0x430UL))
#define SOC_SCTRL_SCBAKDATA10_ADDR(base) ((base) + (0x434UL))
#define SOC_SCTRL_SCBAKDATA11_ADDR(base) ((base) + (0x438UL))
#define SOC_SCTRL_SCBAKDATA12_ADDR(base) ((base) + (0x43CUL))
#define SOC_SCTRL_SCBAKDATA13_ADDR(base) ((base) + (0x440UL))
#define SOC_SCTRL_SCBAKDATA14_ADDR(base) ((base) + (0x444UL))
#define SOC_SCTRL_SCBAKDATA15_ADDR(base) ((base) + (0x448UL))
#define SOC_SCTRL_SCBAKDATA16_ADDR(base) ((base) + (0x44CUL))
#define SOC_SCTRL_SCBAKDATA17_ADDR(base) ((base) + (0x450UL))
#define SOC_SCTRL_SCBAKDATA18_ADDR(base) ((base) + (0x454UL))
#define SOC_SCTRL_SCBAKDATA19_ADDR(base) ((base) + (0x458UL))
#define SOC_SCTRL_SCBAKDATA20_ADDR(base) ((base) + (0x45CUL))
#define SOC_SCTRL_SCBAKDATA21_ADDR(base) ((base) + (0x460UL))
#define SOC_SCTRL_SCBAKDATA22_ADDR(base) ((base) + (0x464UL))
#define SOC_SCTRL_SCBAKDATA23_ADDR(base) ((base) + (0x468UL))
#define SOC_SCTRL_SCBAKDATA24_ADDR(base) ((base) + (0x46CUL))
#define SOC_SCTRL_SCBAKDATA25_ADDR(base) ((base) + (0x470UL))
#define SOC_SCTRL_SCBAKDATA26_ADDR(base) ((base) + (0x474UL))
#define SOC_SCTRL_SCBAKDATA27_ADDR(base) ((base) + (0x478UL))
#define SOC_SCTRL_SCBAKDATA28_ADDR(base) ((base) + (0x47CUL))
#define SOC_SCTRL_SCBAKDATA0_MSK_ADDR(base) ((base) + (0x4C0UL))
#define SOC_SCTRL_SCBAKDATA1_MSK_ADDR(base) ((base) + (0x4C4UL))
#define SOC_SCTRL_SCBAKDATA2_MSK_ADDR(base) ((base) + (0x4C8UL))
#define SOC_SCTRL_SCBAKDATA3_MSK_ADDR(base) ((base) + (0x4CCUL))
#define SOC_SCTRL_SCBAKDATA4_MSK_ADDR(base) ((base) + (0x4D0UL))
#define SOC_SCTRL_SCBAKDATA5_MSK_ADDR(base) ((base) + (0x4D4UL))
#define SOC_SCTRL_SCBAKDATA6_MSK_ADDR(base) ((base) + (0x4D8UL))
#define SOC_SCTRL_SCBAKDATA7_MSK_ADDR(base) ((base) + (0x4DCUL))
#define SOC_SCTRL_SCLPMCUCLKEN_ADDR(base) ((base) + (0x960UL))
#define SOC_SCTRL_SCLPMCUCLKDIS_ADDR(base) ((base) + (0x964UL))
#define SOC_SCTRL_SCLPMCUCLKSTAT_ADDR(base) ((base) + (0x968UL))
#define SOC_SCTRL_SCLPMCURSTEN_ADDR(base) ((base) + (0x970UL))
#define SOC_SCTRL_SCLPMCURSTDIS_ADDR(base) ((base) + (0x974UL))
#define SOC_SCTRL_SCLPMCURSTSTAT_ADDR(base) ((base) + (0x978UL))
#define SOC_SCTRL_SCLPMCUCTRL_ADDR(base) ((base) + (0x980UL))
#define SOC_SCTRL_SCLPMCUSTAT_ADDR(base) ((base) + (0x984UL))
#define SOC_SCTRL_SCLPMCURAMCTRL_ADDR(base) ((base) + (0x988UL))
#define SOC_SCTRL_SCBBPDRXSTAT0_ADDR(base) ((base) + (0x530UL))
#define SOC_SCTRL_SCBBPDRXSTAT1_ADDR(base) ((base) + (0x534UL))
#define SOC_SCTRL_SCBBPDRXSTAT2_ADDR(base) ((base) + (0x538UL))
#define SOC_SCTRL_SCBBPDRXSTAT3_ADDR(base) ((base) + (0x53CUL))
#define SOC_SCTRL_SCBBPDRXSTAT4_ADDR(base) ((base) + (0x540UL))
#define SOC_SCTRL_SCA53_EVENT_MASK_ADDR(base) ((base) + (0x550UL))
#define SOC_SCTRL_SCA57_EVENT_MASK_ADDR(base) ((base) + (0x554UL))
#define SOC_SCTRL_SCIOMCU_EVENT_MASK_ADDR(base) ((base) + (0x558UL))
#define SOC_SCTRL_SCLPMCU_EVENT_MASK_ADDR(base) ((base) + (0x55CUL))
#define SOC_SCTRL_SCMCPU_EVENT_MASK_ADDR(base) ((base) + (0x560UL))
#define SOC_SCTRL_SCISPA7_EVENT_MASK_ADDR(base) ((base) + (0x568UL))
#define SOC_SCTRL_SCHIFD_UL_EVENT_MASK_ADDR(base) ((base) + (0x56CUL))
#define SOC_SCTRL_SCNPU_AICPU_EVENT_MASK_ADDR(base) ((base) + (0x570UL))
#define SOC_SCTRL_SCNPU_TSCPU_EVENT_MASK_ADDR(base) ((base) + (0x574UL))
#define SOC_SCTRL_SCNPU_ARCHS_EVENT_MASK_ADDR(base) ((base) + (0x578UL))
#define SOC_SCTRL_SCEPS_EVENT_MASK_ADDR(base) ((base) + (0x57CUL))
#define SOC_SCTRL_SCMDM5G_L2HAC_EVENT_MASK_ADDR(base) ((base) + (0x544UL))
#define SOC_SCTRL_SCMDM5G_CCPU5G_EVENT_MASK_ADDR(base) ((base) + (0x548UL))
#define SOC_SCTRL_SCEVENT_STAT_ADDR(base) ((base) + (0x564UL))
#define SOC_SCTRL_SCIOMCUCLKCTRL_ADDR(base) ((base) + (0x880UL))
#define SOC_SCTRL_SCIOMCUCLKSTAT_ADDR(base) ((base) + (0x584UL))
#define SOC_SCTRL_SCIOMCUCTRL_ADDR(base) ((base) + (0x588UL))
#define SOC_SCTRL_SCIOMCUSTAT_ADDR(base) ((base) + (0x58CUL))
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_0_ADDR(base) ((base) + (0x500UL))
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_0_ADDR(base) ((base) + (0x504UL))
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_0_ADDR(base) ((base) + (0x508UL))
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_1_ADDR(base) ((base) + (0x50CUL))
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_1_ADDR(base) ((base) + (0x510UL))
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_1_ADDR(base) ((base) + (0x514UL))
#define SOC_SCTRL_SCMDMADDR0_NONSEC_ADDR(base) ((base) + (0x700UL))
#define SOC_SCTRL_SCMDMADDR1_NONSEC_ADDR(base) ((base) + (0x704UL))
#define SOC_SCTRL_SCMDMADDR2_NONSEC_ADDR(base) ((base) + (0x708UL))
#define SOC_SCTRL_SCMDMADDR3_NONSEC_ADDR(base) ((base) + (0x70CUL))
#define SOC_SCTRL_SC_LOCK_CFG_SEC_0_ADDR(base) ((base) + (0x8E8UL))
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_0_ADDR(base) ((base) + (0x8ECUL))
#define SOC_SCTRL_SC_LOCK_STAT_SEC_0_ADDR(base) ((base) + (0x8F0UL))
#define SOC_SCTRL_SC_LOCK_CFG_SEC_1_ADDR(base) ((base) + (0x8F4UL))
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_1_ADDR(base) ((base) + (0x8F8UL))
#define SOC_SCTRL_SC_LOCK_STAT_SEC_1_ADDR(base) ((base) + (0x8FCUL))
#define SOC_SCTRL_SCJTAG_SEL_ADDR(base) ((base) + (0x800UL))
#define SOC_SCTRL_SCCFG_DJTAG_ADDR(base) ((base) + (0x814UL))
#define SOC_SCTRL_SCCP15_DISABLE_ADDR(base) ((base) + (0x818UL))
#define SOC_SCTRL_SCCFG_ARM_DBGEN_ADDR(base) ((base) + (0x82CUL))
#define SOC_SCTRL_SCARM_DBGEN_STAT_ADDR(base) ((base) + (0x838UL))
#define SOC_SCTRL_SCEFUSECTRL_ADDR(base) ((base) + (0x83CUL))
#define SOC_SCTRL_SCEFUSESEL_ADDR(base) ((base) + (0x840UL))
#define SOC_SCTRL_SCCHIP_ID0_ADDR(base) ((base) + (0x848UL))
#define SOC_SCTRL_SCCHIP_ID1_ADDR(base) ((base) + (0x84CUL))
#define SOC_SCTRL_SCCPUSECCTRL_ADDR(base) ((base) + (0x850UL))
#define SOC_SCTRL_SCJTAGSD_SW_SEL_ADDR(base) ((base) + (0x854UL))
#define SOC_SCTRL_SCJTAGSYS_SW_SEL_ADDR(base) ((base) + (0x858UL))
#define SOC_SCTRL_SC_INTR_SYSCACHE_SEC_ADDR(base) ((base) + (0x85CUL))
#define SOC_SCTRL_SCARM_DBG_KEY0_ADDR(base) ((base) + (0x860UL))
#define SOC_SCTRL_SCARM_DBG_KEY1_ADDR(base) ((base) + (0x864UL))
#define SOC_SCTRL_SCARM_DBG_KEY2_ADDR(base) ((base) + (0x868UL))
#define SOC_SCTRL_SCARM_DBG_KEY3_ADDR(base) ((base) + (0x86CUL))
#define SOC_SCTRL_SC_SPMI_FAST_CFG_ADDR(base) ((base) + (0x91CUL))
#define SOC_SCTRL_SCSPMIADDR0_ADDR(base) ((base) + (0x920UL))
#define SOC_SCTRL_SCSPMIADDR1_ADDR(base) ((base) + (0x924UL))
#define SOC_SCTRL_SCSPMIADDR2_ADDR(base) ((base) + (0x928UL))
#define SOC_SCTRL_SCSPMIADDR3_ADDR(base) ((base) + (0x92CUL))
#define SOC_SCTRL_SCSPMIADDR4_ADDR(base) ((base) + (0x930UL))
#define SOC_SCTRL_SCSPMIADDR5_ADDR(base) ((base) + (0x934UL))
#define SOC_SCTRL_SCSPMIADDR6_ADDR(base) ((base) + (0x938UL))
#define SOC_SCTRL_SCSPMIADDR7_ADDR(base) ((base) + (0x93CUL))
#define SOC_SCTRL_SCSPMIADDR8_ADDR(base) ((base) + (0x940UL))
#define SOC_SCTRL_SCSPMIADDR9_ADDR(base) ((base) + (0x944UL))
#define SOC_SCTRL_SCSPMIADDR10_ADDR(base) ((base) + (0x948UL))
#define SOC_SCTRL_SCSPMIADDR11_ADDR(base) ((base) + (0x94CUL))
#define SOC_SCTRL_SCSPMIADDR12_ADDR(base) ((base) + (0x950UL))
#define SOC_SCTRL_SCSPMIADDR13_ADDR(base) ((base) + (0x954UL))
#define SOC_SCTRL_SCSPMIADDR14_ADDR(base) ((base) + (0x958UL))
#define SOC_SCTRL_SCSPMIADDR15_ADDR(base) ((base) + (0x95CUL))
#define SOC_SCTRL_SCHISEEGPIOLOCK_ADDR(base) ((base) + (0x894UL))
#define SOC_SCTRL_SCHISEESPILOCK_ADDR(base) ((base) + (0x898UL))
#define SOC_SCTRL_SCHISEEI2CLOCK_ADDR(base) ((base) + (0x89CUL))
#define SOC_SCTRL_SCHISEEIOSEL_ADDR(base) ((base) + (0x8A0UL))
#define SOC_SCTRL_SCPERCTRL0_SEC_ADDR(base) ((base) + (0x8A4UL))
#define SOC_SCTRL_SCPERCTRL1_SEC_ADDR(base) ((base) + (0x8B0UL))
#define SOC_SCTRL_SCPERCTRL2_SEC_ADDR(base) ((base) + (0x8B4UL))
#define SOC_SCTRL_SCPERCTRL3_SEC_ADDR(base) ((base) + (0x8B8UL))
#define SOC_SCTRL_SCPERCTRL4_SEC_ADDR(base) ((base) + (0x8BCUL))
#define SOC_SCTRL_SCPERCTRL5_SEC_ADDR(base) ((base) + (0x8E0UL))
#define SOC_SCTRL_SCPERSTATUS0_SEC_ADDR(base) ((base) + (0x8C0UL))
#define SOC_SCTRL_SCPERSTATUS1_SEC_ADDR(base) ((base) + (0x8C4UL))
#define SOC_SCTRL_SCPERSTATUS2_SEC_ADDR(base) ((base) + (0x8C8UL))
#define SOC_SCTRL_SCPERSTATUS3_SEC_ADDR(base) ((base) + (0x8CCUL))
#define SOC_SCTRL_SC_TCP_VOTE_REQ_0_ADDR(base) ((base) + (0x8D0UL))
#define SOC_SCTRL_SC_TCP_VOTE_STAT_ADDR(base) ((base) + (0x8D4UL))
#define SOC_SCTRL_SC_TCP_VOTE_CTRL0_ADDR(base) ((base) + (0x8D8UL))
#define SOC_SCTRL_SC_TCP_VOTE_REQ_1_ADDR(base) ((base) + (0x8DCUL))
#define SOC_SCTRL_SC_TCP_VOTE_CTRL1_ADDR(base) ((base) + (0x8E4UL))
#define SOC_SCTRL_SC_TCP_VOTE_REQ3_ADD0_ADDR(base) ((base) + (0x808UL))
#define SOC_SCTRL_SC_TCP_VOTE_REQ3_ADD1_ADDR(base) ((base) + (0x80CUL))
#define SOC_SCTRL_SCPEREN0_SEC_ADDR(base) ((base) + (0x900UL))
#define SOC_SCTRL_SCPERDIS0_SEC_ADDR(base) ((base) + (0x904UL))
#define SOC_SCTRL_SCPERCLKEN0_SEC_ADDR(base) ((base) + (0x908UL))
#define SOC_SCTRL_SCPERRSTEN0_SEC_ADDR(base) ((base) + (0x910UL))
#define SOC_SCTRL_SCPERRSTDIS0_SEC_ADDR(base) ((base) + (0x914UL))
#define SOC_SCTRL_SCPERRSTSTAT0_SEC_ADDR(base) ((base) + (0x918UL))
#define SOC_SCTRL_SCPERRSTEN1_SEC_ADDR(base) ((base) + (0x990UL))
#define SOC_SCTRL_SCPERRSTDIS1_SEC_ADDR(base) ((base) + (0x994UL))
#define SOC_SCTRL_SCPERRSTSTAT1_SEC_ADDR(base) ((base) + (0x998UL))
#define SOC_SCTRL_SCPERRSTEN2_SEC_ADDR(base) ((base) + (0x9A0UL))
#define SOC_SCTRL_SCPERRSTDIS2_SEC_ADDR(base) ((base) + (0x9A4UL))
#define SOC_SCTRL_SCPERRSTSTAT2_SEC_ADDR(base) ((base) + (0x9A8UL))
#define SOC_SCTRL_SCPERRSTEN3_SEC_ADDR(base) ((base) + (0x9B0UL))
#define SOC_SCTRL_SCPERRSTDIS3_SEC_ADDR(base) ((base) + (0x9B4UL))
#define SOC_SCTRL_SCPERRSTSTAT3_SEC_ADDR(base) ((base) + (0x9B8UL))
#define SOC_SCTRL_SCPEREN3_SEC_ADDR(base) ((base) + (0x9C0UL))
#define SOC_SCTRL_SCPERDIS3_SEC_ADDR(base) ((base) + (0x9C4UL))
#define SOC_SCTRL_SCPERCLKEN3_SEC_ADDR(base) ((base) + (0x9C8UL))
#define SOC_SCTRL_SCPEREN4_SEC_ADDR(base) ((base) + (0x9D0UL))
#define SOC_SCTRL_SCPERDIS4_SEC_ADDR(base) ((base) + (0x9D4UL))
#define SOC_SCTRL_SCPERCLKEN4_SEC_ADDR(base) ((base) + (0x9D8UL))
#define SOC_SCTRL_SCCLKDIV0_SEC_ADDR(base) ((base) + (0x9E0UL))
#define SOC_SCTRL_SCCLKDIV1_SEC_ADDR(base) ((base) + (0x9E4UL))
#define SOC_SCTRL_SCCLKDIV2_SEC_ADDR(base) ((base) + (0x9E8UL))
#define SOC_SCTRL_SCCLKDIV3_SEC_ADDR(base) ((base) + (0x9ECUL))
#define SOC_SCTRL_SCCLKDIV4_SEC_ADDR(base) ((base) + (0x9F0UL))
#define SOC_SCTRL_SCCLKDIV5_SEC_ADDR(base) ((base) + (0x9F4UL))
#define SOC_SCTRL_SCCLKDIV6_SEC_ADDR(base) ((base) + (0x9F8UL))
#define SOC_SCTRL_SCCLKDIV7_SEC_ADDR(base) ((base) + (0x9FCUL))
#define SOC_SCTRL_SCCLKDIV8_SEC_ADDR(base) ((base) + (0x9DCUL))
#define SOC_SCTRL_SCCLKDIV9_SEC_ADDR(base) ((base) + (0x9CCUL))
#define SOC_SCTRL_SCCLKDIV10_SEC_ADDR(base) ((base) + (0xA0CUL))
#define SOC_SCTRL_SCPERRSTEN4_SEC_ADDR(base) ((base) + (0xA00UL))
#define SOC_SCTRL_SCPERRSTDIS4_SEC_ADDR(base) ((base) + (0xA04UL))
#define SOC_SCTRL_SCPERRSTSTAT4_SEC_ADDR(base) ((base) + (0xA08UL))
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_SEC_ADDR(base) ((base) + (0xA84UL))
#define SOC_SCTRL_SC_IPCLKRST_BYPASS1_SEC_ADDR(base) ((base) + (0xA88UL))
#define SOC_SCTRL_SC_IPCLKRST_BYPASS2_SEC_ADDR(base) ((base) + (0xAA0UL))
#define SOC_SCTRL_SC_CLK_STAT0_SEC_ADDR(base) ((base) + (0xA8CUL))
#define SOC_SCTRL_SC_CLK_STAT1_SEC_ADDR(base) ((base) + (0xA90UL))
#define SOC_SCTRL_PLL_FSM_CTRL0_SEC_ADDR(base) ((base) + (0xA94UL))
#define SOC_SCTRL_PLL_FSM_CTRL1_SEC_ADDR(base) ((base) + (0xA98UL))
#define SOC_SCTRL_PLL_FSM_STAT0_SEC_ADDR(base) ((base) + (0xA9CUL))
#define SOC_SCTRL_SCSOCID0_ADDR(base) ((base) + (0xE00UL))
#define SOC_SCTRL_SCPERSTAT_POR_RESERVED0_ADDR(base) ((base) + (0xE10UL))
#define SOC_SCTRL_SCPERSTAT_POR_RESERVED1_ADDR(base) ((base) + (0xE14UL))
#define SOC_SCTRL_SCPERSTAT_POR_RESERVED2_ADDR(base) ((base) + (0xE18UL))
#define SOC_SCTRL_SCPERSTAT_POR_RESERVED3_ADDR(base) ((base) + (0xE1CUL))
#define SOC_SCTRL_SC_NPU_LPCTRL0_SEC_ADDR(base) ((base) + (0xE20UL))
#define SOC_SCTRL_SC_NPU_LPCTRL1_SEC_ADDR(base) ((base) + (0xE24UL))
#define SOC_SCTRL_SC_NPU_LPCTRL2_SEC_ADDR(base) ((base) + (0xE28UL))
#define SOC_SCTRL_SC_NPU_LPCTRL3_SEC_ADDR(base) ((base) + (0xE2CUL))
#define SOC_SCTRL_SC_NPU_LPCTRL4_SEC_ADDR(base) ((base) + (0xE30UL))
#define SOC_SCTRL_SC_TRACKING_LOG0_ADDR(base) ((base) + (0xE40UL))
#define SOC_SCTRL_SC_TRACKING_LOG1_ADDR(base) ((base) + (0xE44UL))
#define SOC_SCTRL_SC_TRACKING_LOG2_ADDR(base) ((base) + (0xE48UL))
#define SOC_SCTRL_SC_TRACKING_LOG3_ADDR(base) ((base) + (0xE4CUL))
#define SOC_SCTRL_SC_EMU_VERSION_ADDR(base) ((base) + (0xE34UL))
#define SOC_SCTRL_SC_DDRC_CTRL_ADDR(base) ((base) + (0xE38UL))
#else
#define SOC_SCTRL_SCCTRL_ADDR(base) ((base) + (0xF00))
#define SOC_SCTRL_SCSYSSTAT_ADDR(base) ((base) + (0xF04))
#define SOC_SCTRL_SCSYSSTAT_SHADOW_ADDR(base) ((base) + (0x004))
#define SOC_SCTRL_SC_SCLK_SW2FLL_CTRL_ADDR(base) ((base) + (0xF50))
#define SOC_SCTRL_SC_SCLK_SW2FLL_STAT_ADDR(base) ((base) + (0xF54))
#define SOC_SCTRL_SC_PERI_POWER_STAT_ADDR(base) ((base) + (0xF58))
#define SOC_SCTRL_SCPERIISOBYPASS_ADDR(base) ((base) + (0xF5C))
#define SOC_SCTRL_SC_TCXO_CTRL_0_ADDR(base) ((base) + (0xF60))
#define SOC_SCTRL_SC_TCXO_CTRL_1_ADDR(base) ((base) + (0xF64))
#define SOC_SCTRL_SC_TCXO_CTRL_2_ADDR(base) ((base) + (0xF68))
#define SOC_SCTRL_SC_TCXO_CTRL_3_ADDR(base) ((base) + (0xF6C))
#define SOC_SCTRL_SC_PERI_POWER_CTRL_0_ADDR(base) ((base) + (0xF70))
#define SOC_SCTRL_SC_PERI_POWER_CTRL_1_ADDR(base) ((base) + (0xF74))
#define SOC_SCTRL_SC_PERI_POWER_CTRL_2_ADDR(base) ((base) + (0xF78))
#define SOC_SCTRL_SC_PERI_POWER_CTRL_3_ADDR(base) ((base) + (0xF7C))
#define SOC_SCTRL_SC_RESUME_WATCHDOG_CTRL_ADDR(base) ((base) + (0xF80))
#define SOC_SCTRL_SCDEEPSLEEPED_ADDR(base) ((base) + (0x008))
#define SOC_SCTRL_SC_LP_FLAG_ADDR(base) ((base) + (0x018))
#define SOC_SCTRL_SC_INTR_WAKEUP_CTRL_ADDR(base) ((base) + (0x014))
#define SOC_SCTRL_SCEFUSEDATA2_ADDR(base) ((base) + (0x00C))
#define SOC_SCTRL_SCEFUSEDATA3_ADDR(base) ((base) + (0x010))
#define SOC_SCTRL_SCEFUSEDATA8_ADDR(base) ((base) + (0x060))
#define SOC_SCTRL_SCEFUSEDATA9_ADDR(base) ((base) + (0x064))
#define SOC_SCTRL_SCLPSTATCFG_ADDR(base) ((base) + (0x038))
#define SOC_SCTRL_SC_ASP_PWRCFG0_ADDR(base) ((base) + (0x080))
#define SOC_SCTRL_SC_ASP_PWRCFG1_ADDR(base) ((base) + (0x084))
#define SOC_SCTRL_SCINT_GATHER_STAT_ADDR(base) ((base) + (0x0A0))
#define SOC_SCTRL_SCINT_MASK_ADDR(base) ((base) + (0x0A4))
#define SOC_SCTRL_SCINT_STAT_ADDR(base) ((base) + (0x0A8))
#define SOC_SCTRL_SCDRX_INT_CFG_ADDR(base) ((base) + (0x0AC))
#define SOC_SCTRL_SCLPMCUWFI_INT_ADDR(base) ((base) + (0x0B0))
#define SOC_SCTRL_SCINT_MASK1_ADDR(base) ((base) + (0x0B4))
#define SOC_SCTRL_SCINT_STAT1_ADDR(base) ((base) + (0x0B8))
#define SOC_SCTRL_SCINT_MASK2_ADDR(base) ((base) + (0x0BC))
#define SOC_SCTRL_SCINT_STAT2_ADDR(base) ((base) + (0x0C0))
#define SOC_SCTRL_SCINT_MASK3_ADDR(base) ((base) + (0x0C4))
#define SOC_SCTRL_SCINT_STAT3_ADDR(base) ((base) + (0x0C8))
#define SOC_SCTRL_SCINT_MASK4_ADDR(base) ((base) + (0x090))
#define SOC_SCTRL_SCINT_STAT4_ADDR(base) ((base) + (0x094))
#define SOC_SCTRL_SCINT_MASK5_ADDR(base) ((base) + (0x098))
#define SOC_SCTRL_SCINT_STAT5_ADDR(base) ((base) + (0x09C))
#define SOC_SCTRL_SC_PLL_EN_VOTE_0_ADDR(base) ((base) + (0xF08))
#define SOC_SCTRL_SC_PLL_EN_VOTE_1_ADDR(base) ((base) + (0x0D4))
#define SOC_SCTRL_SC_PLL_EN_VOTE_2_ADDR(base) ((base) + (0x0D8))
#define SOC_SCTRL_SC_PLL_EN_VOTE_3_ADDR(base) ((base) + (0x0DC))
#define SOC_SCTRL_SC_PLL_GT_VOTE_0_ADDR(base) ((base) + (0xF0C))
#define SOC_SCTRL_SC_PLL_GT_VOTE_1_ADDR(base) ((base) + (0x234))
#define SOC_SCTRL_SC_PLL_GT_VOTE_2_ADDR(base) ((base) + (0x238))
#define SOC_SCTRL_SC_PLL_GT_VOTE_3_ADDR(base) ((base) + (0x23C))
#define SOC_SCTRL_SC_PLL_BP_VOTE_0_ADDR(base) ((base) + (0xF10))
#define SOC_SCTRL_SC_PLL_BP_VOTE_1_ADDR(base) ((base) + (0x244))
#define SOC_SCTRL_SC_PLL_BP_VOTE_2_ADDR(base) ((base) + (0x248))
#define SOC_SCTRL_SC_PLL_BP_VOTE_3_ADDR(base) ((base) + (0x24C))
#define SOC_SCTRL_SC_PLL_STAT_ADDR(base) ((base) + (0x0CC))
#define SOC_SCTRL_SCSPLLCTRL0_ADDR(base) ((base) + (0xF14))
#define SOC_SCTRL_SCSPLLCTRL1_ADDR(base) ((base) + (0xF18))
#define SOC_SCTRL_SCSPLLSSCCTRL_ADDR(base) ((base) + (0xF1C))
#define SOC_SCTRL_SCSPLLCTRL0_LOCK_STAT_ADDR(base) ((base) + (0xF20))
#define SOC_SCTRL_SCSPLLCTRL1_LOCK_STAT_ADDR(base) ((base) + (0xF24))
#define SOC_SCTRL_SCSPLLSSCCTRL_LOCK_STAT_ADDR(base) ((base) + (0xF28))
#define SOC_SCTRL_SCSPLL_OTHER_LOCK_STAT_ADDR(base) ((base) + (0xF2C))
#define SOC_SCTRL_FNPLL_CFG0_ADDR(base) ((base) + (0xF30))
#define SOC_SCTRL_FNPLL_CFG1_ADDR(base) ((base) + (0xF34))
#define SOC_SCTRL_FNPLL_CFG2_ADDR(base) ((base) + (0xF38))
#define SOC_SCTRL_FNPLL_CFG3_ADDR(base) ((base) + (0xF3C))
#define SOC_SCTRL_FNPLL_STATE0_ADDR(base) ((base) + (0x1E8))
#define SOC_SCTRL_SC_ULPPLL_GT_CFG_ADDR(base) ((base) + (0x088))
#define SOC_SCTRL_SC_ULPPLL_1_GT_CFG_ADDR(base) ((base) + (0x08C))
#define SOC_SCTRL_SC_ULPPLL_CFG0_ADDR(base) ((base) + (0x110))
#define SOC_SCTRL_SC_ULPPLL_CFG1_ADDR(base) ((base) + (0x114))
#define SOC_SCTRL_SC_ULPPLL_CFG2_ADDR(base) ((base) + (0x118))
#define SOC_SCTRL_SC_ULPPLL_CTRL0_ADDR(base) ((base) + (0x11C))
#define SOC_SCTRL_SC_ULPPLL_STATE0_ADDR(base) ((base) + (0x120))
#define SOC_SCTRL_SC_ULPPLL_1_CFG0_ADDR(base) ((base) + (0x140))
#define SOC_SCTRL_SC_ULPPLL_1_CFG1_ADDR(base) ((base) + (0x144))
#define SOC_SCTRL_SC_ULPPLL_1_CFG2_ADDR(base) ((base) + (0x148))
#define SOC_SCTRL_SC_ULPPLL_1_CTRL0_ADDR(base) ((base) + (0x14C))
#define SOC_SCTRL_SC_ULPPLL_1_STATE0_ADDR(base) ((base) + (0x150))
#define SOC_SCTRL_SCAUPLLCTRL0_ADDR(base) ((base) + (0x490))
#define SOC_SCTRL_SCAUPLLCTRL1_ADDR(base) ((base) + (0x494))
#define SOC_SCTRL_SCAUPLLSSCCTRL_ADDR(base) ((base) + (0x498))
#define SOC_SCTRL_AUFNPLL_CFG0_ADDR(base) ((base) + (0x49C))
#define SOC_SCTRL_AUFNPLL_CFG1_ADDR(base) ((base) + (0x4A0))
#define SOC_SCTRL_AUFNPLL_CFG2_ADDR(base) ((base) + (0x4A4))
#define SOC_SCTRL_AUFNPLL_CFG3_ADDR(base) ((base) + (0x4A8))
#define SOC_SCTRL_AUFNPLL_STATE0_ADDR(base) ((base) + (0x4B4))
#define SOC_SCTRL_SCPEREN0_ADDR(base) ((base) + (0x160))
#define SOC_SCTRL_SCPERDIS0_ADDR(base) ((base) + (0x164))
#define SOC_SCTRL_SCPERCLKEN0_ADDR(base) ((base) + (0x168))
#define SOC_SCTRL_SCPERSTAT0_ADDR(base) ((base) + (0x16C))
#define SOC_SCTRL_SCPEREN1_ADDR(base) ((base) + (0x170))
#define SOC_SCTRL_SCPERDIS1_ADDR(base) ((base) + (0x174))
#define SOC_SCTRL_SCPERCLKEN1_ADDR(base) ((base) + (0x178))
#define SOC_SCTRL_SCPERSTAT1_ADDR(base) ((base) + (0x17C))
#define SOC_SCTRL_SCPEREN2_ADDR(base) ((base) + (0x190))
#define SOC_SCTRL_SCPERDIS2_ADDR(base) ((base) + (0x194))
#define SOC_SCTRL_SCPERCLKEN2_ADDR(base) ((base) + (0x198))
#define SOC_SCTRL_SCPERSTAT2_ADDR(base) ((base) + (0x19C))
#define SOC_SCTRL_SCPEREN3_ADDR(base) ((base) + (0x1A0))
#define SOC_SCTRL_SCPERDIS3_ADDR(base) ((base) + (0x1A4))
#define SOC_SCTRL_SCPERCLKEN3_ADDR(base) ((base) + (0x1A8))
#define SOC_SCTRL_SCPERSTAT3_ADDR(base) ((base) + (0x1AC))
#define SOC_SCTRL_SCPEREN4_ADDR(base) ((base) + (0x1B0))
#define SOC_SCTRL_SCPERDIS4_ADDR(base) ((base) + (0x1B4))
#define SOC_SCTRL_SCPERCLKEN4_ADDR(base) ((base) + (0x1B8))
#define SOC_SCTRL_SCPERSTAT4_ADDR(base) ((base) + (0x1BC))
#define SOC_SCTRL_SCPEREN5_ADDR(base) ((base) + (0x1C0))
#define SOC_SCTRL_SCPERDIS5_ADDR(base) ((base) + (0x1C4))
#define SOC_SCTRL_SCPERCLKEN5_ADDR(base) ((base) + (0x1C8))
#define SOC_SCTRL_SCPERSTAT5_ADDR(base) ((base) + (0x1CC))
#define SOC_SCTRL_SC_AO_CRG_STA_SEP0_ADDR(base) ((base) + (0x188))
#define SOC_SCTRL_SCPERRSTEN0_ADDR(base) ((base) + (0x200))
#define SOC_SCTRL_SCPERRSTDIS0_ADDR(base) ((base) + (0x204))
#define SOC_SCTRL_SCPERRSTSTAT0_ADDR(base) ((base) + (0x208))
#define SOC_SCTRL_SCPERRSTEN1_ADDR(base) ((base) + (0x20C))
#define SOC_SCTRL_SCPERRSTDIS1_ADDR(base) ((base) + (0x210))
#define SOC_SCTRL_SCPERRSTSTAT1_ADDR(base) ((base) + (0x214))
#define SOC_SCTRL_SCPERRSTEN2_ADDR(base) ((base) + (0x218))
#define SOC_SCTRL_SCPERRSTDIS2_ADDR(base) ((base) + (0x21C))
#define SOC_SCTRL_SCPERRSTSTAT2_ADDR(base) ((base) + (0x220))
#define SOC_SCTRL_SCCLKDIV0_ADDR(base) ((base) + (0x250))
#define SOC_SCTRL_SCCLKDIV1_ADDR(base) ((base) + (0x254))
#define SOC_SCTRL_SCCLKDIV2_ADDR(base) ((base) + (0x258))
#define SOC_SCTRL_SCCLKDIV3_ADDR(base) ((base) + (0x25C))
#define SOC_SCTRL_SCCLKDIV4_ADDR(base) ((base) + (0x260))
#define SOC_SCTRL_SCCLKDIV5_ADDR(base) ((base) + (0x264))
#define SOC_SCTRL_SCCLKDIV6_ADDR(base) ((base) + (0x268))
#define SOC_SCTRL_SCCLKDIV7_ADDR(base) ((base) + (0x26C))
#define SOC_SCTRL_SCCLKDIV8_ADDR(base) ((base) + (0x270))
#define SOC_SCTRL_SCCLKDIV9_ADDR(base) ((base) + (0x274))
#define SOC_SCTRL_SCUFS_AUTODIV_ADDR(base) ((base) + (0x278))
#define SOC_SCTRL_SC_INTR_NOCBUS_CTRL_ADDR(base) ((base) + (0x27C))
#define SOC_SCTRL_SCCLKDIV10_ADDR(base) ((base) + (0x280))
#define SOC_SCTRL_SCCLKDIV11_ADDR(base) ((base) + (0x284))
#define SOC_SCTRL_SCCLKDIV12_ADDR(base) ((base) + (0x288))
#define SOC_SCTRL_SCCLKDIV13_ADDR(base) ((base) + (0x28C))
#define SOC_SCTRL_SCCLKDIV14_ADDR(base) ((base) + (0x290))
#define SOC_SCTRL_SCCLKDIV15_ADDR(base) ((base) + (0x294))
#define SOC_SCTRL_SCCLKDIV16_ADDR(base) ((base) + (0x298))
#define SOC_SCTRL_SCCLKDIV17_ADDR(base) ((base) + (0x29C))
#define SOC_SCTRL_SCCLKDIV18_ADDR(base) ((base) + (0x2A0))
#define SOC_SCTRL_SCCLKDIV19_ADDR(base) ((base) + (0x2A4))
#define SOC_SCTRL_SCCLKDIV20_ADDR(base) ((base) + (0x2A8))
#define SOC_SCTRL_SCCLKDIV21_ADDR(base) ((base) + (0x2AC))
#define SOC_SCTRL_SCCLKDIV22_ADDR(base) ((base) + (0x2B0))
#define SOC_SCTRL_SC_PLL_FSM_NS_VOTE0_ADDR(base) ((base) + (0x2B4))
#define SOC_SCTRL_SCPERCTRL0_ADDR(base) ((base) + (0x300))
#define SOC_SCTRL_SCPERCTRL1_ADDR(base) ((base) + (0x304))
#define SOC_SCTRL_SCPERCTRL2_ADDR(base) ((base) + (0x308))
#define SOC_SCTRL_SCPERCTRL3_ADDR(base) ((base) + (0x30C))
#define SOC_SCTRL_SCPERCTRL4_ADDR(base) ((base) + (0x310))
#define SOC_SCTRL_SCPERCTRL5_ADDR(base) ((base) + (0x314))
#define SOC_SCTRL_SCPERCTRL6_ADDR(base) ((base) + (0x318))
#define SOC_SCTRL_SCPERCTRL7_ADDR(base) ((base) + (0x31C))
#define SOC_SCTRL_SCPERCTRL9_ADDR(base) ((base) + (0x324))
#define SOC_SCTRL_SCPERCTRL10_ADDR(base) ((base) + (0x328))
#define SOC_SCTRL_SCPERCTRL11_ADDR(base) ((base) + (0x32C))
#define SOC_SCTRL_SCPERCTRL12_ADDR(base) ((base) + (0x330))
#define SOC_SCTRL_SCPERCTRL13_ADDR(base) ((base) + (0x334))
#define SOC_SCTRL_SCPERCTRL14_ADDR(base) ((base) + (0x338))
#define SOC_SCTRL_SCPERCTRL15_ADDR(base) ((base) + (0x33C))
#define SOC_SCTRL_SCPERCTRL16_ADDR(base) ((base) + (0x340))
#define SOC_SCTRL_SCPERCTRL17_ADDR(base) ((base) + (0x344))
#define SOC_SCTRL_SC_INTR_NOCBUS_STAT_ADDR(base) ((base) + (0x354))
#define SOC_SCTRL_SCPERSTATUS17_ADDR(base) ((base) + (0x358))
#define SOC_SCTRL_SCPERSTATUS16_ADDR(base) ((base) + (0x35C))
#define SOC_SCTRL_SCPERSTATUS0_ADDR(base) ((base) + (0x360))
#define SOC_SCTRL_SCPERSTATUS1_ADDR(base) ((base) + (0x364))
#define SOC_SCTRL_SCPERSTATUS2_ADDR(base) ((base) + (0x368))
#define SOC_SCTRL_SCPERSTATUS3_ADDR(base) ((base) + (0x36C))
#define SOC_SCTRL_SCPERSTATUS4_ADDR(base) ((base) + (0x370))
#define SOC_SCTRL_SCPERSTATUS5_ADDR(base) ((base) + (0x374))
#define SOC_SCTRL_SCPERSTATUS6_ADDR(base) ((base) + (0x378))
#define SOC_SCTRL_SCPERSTATUS7_ADDR(base) ((base) + (0x37C))
#define SOC_SCTRL_SCPERSTATUS8_ADDR(base) ((base) + (0x380))
#define SOC_SCTRL_SCPERSTATUS9_ADDR(base) ((base) + (0x384))
#define SOC_SCTRL_SCPERSTATUS10_ADDR(base) ((base) + (0x388))
#define SOC_SCTRL_SCPERSTATUS11_ADDR(base) ((base) + (0x38C))
#define SOC_SCTRL_SCPERSTATUS12_ADDR(base) ((base) + (0x390))
#define SOC_SCTRL_SCPERSTATUS13_ADDR(base) ((base) + (0x394))
#define SOC_SCTRL_SCPERSTATUS14_ADDR(base) ((base) + (0x398))
#define SOC_SCTRL_SCPERSTATUS15_ADDR(base) ((base) + (0x39C))
#define SOC_SCTRL_SCINNERSTAT_ADDR(base) ((base) + (0x3A0))
#define SOC_SCTRL_SCINNERCTRL_ADDR(base) ((base) + (0x3A4))
#define SOC_SCTRL_SC_SECOND_INT_MASK_ADDR(base) ((base) + (0x3D0))
#define SOC_SCTRL_SC_SECOND_INT_ORG_ADDR(base) ((base) + (0x3D4))
#define SOC_SCTRL_SC_SECOND_INT_OUT_ADDR(base) ((base) + (0x3D8))
#define SOC_SCTRL_SCMRBBUSYSTAT_ADDR(base) ((base) + (0x3FC))
#define SOC_SCTRL_SCSWADDR_ADDR(base) ((base) + (0x400))
#define SOC_SCTRL_SCDDRADDR_ADDR(base) ((base) + (0x404))
#define SOC_SCTRL_SCDDRDATA_ADDR(base) ((base) + (0x408))
#define SOC_SCTRL_SCBAKDATA0_ADDR(base) ((base) + (0x40C))
#define SOC_SCTRL_SCBAKDATA1_ADDR(base) ((base) + (0x410))
#define SOC_SCTRL_SCBAKDATA2_ADDR(base) ((base) + (0x414))
#define SOC_SCTRL_SCBAKDATA3_ADDR(base) ((base) + (0x418))
#define SOC_SCTRL_SCBAKDATA4_ADDR(base) ((base) + (0x41C))
#define SOC_SCTRL_SCBAKDATA5_ADDR(base) ((base) + (0x420))
#define SOC_SCTRL_SCBAKDATA6_ADDR(base) ((base) + (0x424))
#define SOC_SCTRL_SCBAKDATA7_ADDR(base) ((base) + (0x428))
#define SOC_SCTRL_SCBAKDATA8_ADDR(base) ((base) + (0x42C))
#define SOC_SCTRL_SCBAKDATA9_ADDR(base) ((base) + (0x430))
#define SOC_SCTRL_SCBAKDATA10_ADDR(base) ((base) + (0x434))
#define SOC_SCTRL_SCBAKDATA11_ADDR(base) ((base) + (0x438))
#define SOC_SCTRL_SCBAKDATA12_ADDR(base) ((base) + (0x43C))
#define SOC_SCTRL_SCBAKDATA13_ADDR(base) ((base) + (0x440))
#define SOC_SCTRL_SCBAKDATA14_ADDR(base) ((base) + (0x444))
#define SOC_SCTRL_SCBAKDATA15_ADDR(base) ((base) + (0x448))
#define SOC_SCTRL_SCBAKDATA16_ADDR(base) ((base) + (0x44C))
#define SOC_SCTRL_SCBAKDATA17_ADDR(base) ((base) + (0x450))
#define SOC_SCTRL_SCBAKDATA18_ADDR(base) ((base) + (0x454))
#define SOC_SCTRL_SCBAKDATA19_ADDR(base) ((base) + (0x458))
#define SOC_SCTRL_SCBAKDATA20_ADDR(base) ((base) + (0x45C))
#define SOC_SCTRL_SCBAKDATA21_ADDR(base) ((base) + (0x460))
#define SOC_SCTRL_SCBAKDATA22_ADDR(base) ((base) + (0x464))
#define SOC_SCTRL_SCBAKDATA23_ADDR(base) ((base) + (0x468))
#define SOC_SCTRL_SCBAKDATA24_ADDR(base) ((base) + (0x46C))
#define SOC_SCTRL_SCBAKDATA25_ADDR(base) ((base) + (0x470))
#define SOC_SCTRL_SCBAKDATA26_ADDR(base) ((base) + (0x474))
#define SOC_SCTRL_SCBAKDATA27_ADDR(base) ((base) + (0x478))
#define SOC_SCTRL_SCBAKDATA28_ADDR(base) ((base) + (0x47C))
#define SOC_SCTRL_SCBAKDATA0_MSK_ADDR(base) ((base) + (0x4C0))
#define SOC_SCTRL_SCBAKDATA1_MSK_ADDR(base) ((base) + (0x4C4))
#define SOC_SCTRL_SCBAKDATA2_MSK_ADDR(base) ((base) + (0x4C8))
#define SOC_SCTRL_SCBAKDATA3_MSK_ADDR(base) ((base) + (0x4CC))
#define SOC_SCTRL_SCBAKDATA4_MSK_ADDR(base) ((base) + (0x4D0))
#define SOC_SCTRL_SCBAKDATA5_MSK_ADDR(base) ((base) + (0x4D4))
#define SOC_SCTRL_SCBAKDATA6_MSK_ADDR(base) ((base) + (0x4D8))
#define SOC_SCTRL_SCBAKDATA7_MSK_ADDR(base) ((base) + (0x4DC))
#define SOC_SCTRL_SCLPMCUCLKEN_ADDR(base) ((base) + (0x960))
#define SOC_SCTRL_SCLPMCUCLKDIS_ADDR(base) ((base) + (0x964))
#define SOC_SCTRL_SCLPMCUCLKSTAT_ADDR(base) ((base) + (0x968))
#define SOC_SCTRL_SCLPMCURSTEN_ADDR(base) ((base) + (0x970))
#define SOC_SCTRL_SCLPMCURSTDIS_ADDR(base) ((base) + (0x974))
#define SOC_SCTRL_SCLPMCURSTSTAT_ADDR(base) ((base) + (0x978))
#define SOC_SCTRL_SCLPMCUCTRL_ADDR(base) ((base) + (0x980))
#define SOC_SCTRL_SCLPMCUSTAT_ADDR(base) ((base) + (0x984))
#define SOC_SCTRL_SCLPMCURAMCTRL_ADDR(base) ((base) + (0x988))
#define SOC_SCTRL_SCBBPDRXSTAT0_ADDR(base) ((base) + (0x530))
#define SOC_SCTRL_SCBBPDRXSTAT1_ADDR(base) ((base) + (0x534))
#define SOC_SCTRL_SCBBPDRXSTAT2_ADDR(base) ((base) + (0x538))
#define SOC_SCTRL_SCBBPDRXSTAT3_ADDR(base) ((base) + (0x53C))
#define SOC_SCTRL_SCBBPDRXSTAT4_ADDR(base) ((base) + (0x540))
#define SOC_SCTRL_SCA53_EVENT_MASK_ADDR(base) ((base) + (0x550))
#define SOC_SCTRL_SCA57_EVENT_MASK_ADDR(base) ((base) + (0x554))
#define SOC_SCTRL_SCIOMCU_EVENT_MASK_ADDR(base) ((base) + (0x558))
#define SOC_SCTRL_SCLPMCU_EVENT_MASK_ADDR(base) ((base) + (0x55C))
#define SOC_SCTRL_SCMCPU_EVENT_MASK_ADDR(base) ((base) + (0x560))
#define SOC_SCTRL_SCISPA7_EVENT_MASK_ADDR(base) ((base) + (0x568))
#define SOC_SCTRL_SCHIFD_UL_EVENT_MASK_ADDR(base) ((base) + (0x56C))
#define SOC_SCTRL_SCNPU_AICPU_EVENT_MASK_ADDR(base) ((base) + (0x570))
#define SOC_SCTRL_SCNPU_TSCPU_EVENT_MASK_ADDR(base) ((base) + (0x574))
#define SOC_SCTRL_SCNPU_ARCHS_EVENT_MASK_ADDR(base) ((base) + (0x578))
#define SOC_SCTRL_SCEPS_EVENT_MASK_ADDR(base) ((base) + (0x57C))
#define SOC_SCTRL_SCMDM5G_L2HAC_EVENT_MASK_ADDR(base) ((base) + (0x544))
#define SOC_SCTRL_SCMDM5G_CCPU5G_EVENT_MASK_ADDR(base) ((base) + (0x548))
#define SOC_SCTRL_SCEVENT_STAT_ADDR(base) ((base) + (0x564))
#define SOC_SCTRL_SCIOMCUCLKCTRL_ADDR(base) ((base) + (0x880))
#define SOC_SCTRL_SCIOMCUCLKSTAT_ADDR(base) ((base) + (0x584))
#define SOC_SCTRL_SCIOMCUCTRL_ADDR(base) ((base) + (0x588))
#define SOC_SCTRL_SCIOMCUSTAT_ADDR(base) ((base) + (0x58C))
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_0_ADDR(base) ((base) + (0x500))
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_0_ADDR(base) ((base) + (0x504))
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_0_ADDR(base) ((base) + (0x508))
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_1_ADDR(base) ((base) + (0x50C))
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_1_ADDR(base) ((base) + (0x510))
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_1_ADDR(base) ((base) + (0x514))
#define SOC_SCTRL_SCMDMADDR0_NONSEC_ADDR(base) ((base) + (0x700))
#define SOC_SCTRL_SCMDMADDR1_NONSEC_ADDR(base) ((base) + (0x704))
#define SOC_SCTRL_SCMDMADDR2_NONSEC_ADDR(base) ((base) + (0x708))
#define SOC_SCTRL_SCMDMADDR3_NONSEC_ADDR(base) ((base) + (0x70C))
#define SOC_SCTRL_SC_LOCK_CFG_SEC_0_ADDR(base) ((base) + (0x8E8))
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_0_ADDR(base) ((base) + (0x8EC))
#define SOC_SCTRL_SC_LOCK_STAT_SEC_0_ADDR(base) ((base) + (0x8F0))
#define SOC_SCTRL_SC_LOCK_CFG_SEC_1_ADDR(base) ((base) + (0x8F4))
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_1_ADDR(base) ((base) + (0x8F8))
#define SOC_SCTRL_SC_LOCK_STAT_SEC_1_ADDR(base) ((base) + (0x8FC))
#define SOC_SCTRL_SCJTAG_SEL_ADDR(base) ((base) + (0x800))
#define SOC_SCTRL_SCCFG_DJTAG_ADDR(base) ((base) + (0x814))
#define SOC_SCTRL_SCCP15_DISABLE_ADDR(base) ((base) + (0x818))
#define SOC_SCTRL_SCCFG_ARM_DBGEN_ADDR(base) ((base) + (0x82C))
#define SOC_SCTRL_SCARM_DBGEN_STAT_ADDR(base) ((base) + (0x838))
#define SOC_SCTRL_SCEFUSECTRL_ADDR(base) ((base) + (0x83C))
#define SOC_SCTRL_SCEFUSESEL_ADDR(base) ((base) + (0x840))
#define SOC_SCTRL_SCCHIP_ID0_ADDR(base) ((base) + (0x848))
#define SOC_SCTRL_SCCHIP_ID1_ADDR(base) ((base) + (0x84C))
#define SOC_SCTRL_SCCPUSECCTRL_ADDR(base) ((base) + (0x850))
#define SOC_SCTRL_SCJTAGSD_SW_SEL_ADDR(base) ((base) + (0x854))
#define SOC_SCTRL_SCJTAGSYS_SW_SEL_ADDR(base) ((base) + (0x858))
#define SOC_SCTRL_SC_INTR_SYSCACHE_SEC_ADDR(base) ((base) + (0x85C))
#define SOC_SCTRL_SCARM_DBG_KEY0_ADDR(base) ((base) + (0x860))
#define SOC_SCTRL_SCARM_DBG_KEY1_ADDR(base) ((base) + (0x864))
#define SOC_SCTRL_SCARM_DBG_KEY2_ADDR(base) ((base) + (0x868))
#define SOC_SCTRL_SCARM_DBG_KEY3_ADDR(base) ((base) + (0x86C))
#define SOC_SCTRL_SC_SPMI_FAST_CFG_ADDR(base) ((base) + (0x91C))
#define SOC_SCTRL_SCSPMIADDR0_ADDR(base) ((base) + (0x920))
#define SOC_SCTRL_SCSPMIADDR1_ADDR(base) ((base) + (0x924))
#define SOC_SCTRL_SCSPMIADDR2_ADDR(base) ((base) + (0x928))
#define SOC_SCTRL_SCSPMIADDR3_ADDR(base) ((base) + (0x92C))
#define SOC_SCTRL_SCSPMIADDR4_ADDR(base) ((base) + (0x930))
#define SOC_SCTRL_SCSPMIADDR5_ADDR(base) ((base) + (0x934))
#define SOC_SCTRL_SCSPMIADDR6_ADDR(base) ((base) + (0x938))
#define SOC_SCTRL_SCSPMIADDR7_ADDR(base) ((base) + (0x93C))
#define SOC_SCTRL_SCSPMIADDR8_ADDR(base) ((base) + (0x940))
#define SOC_SCTRL_SCSPMIADDR9_ADDR(base) ((base) + (0x944))
#define SOC_SCTRL_SCSPMIADDR10_ADDR(base) ((base) + (0x948))
#define SOC_SCTRL_SCSPMIADDR11_ADDR(base) ((base) + (0x94C))
#define SOC_SCTRL_SCSPMIADDR12_ADDR(base) ((base) + (0x950))
#define SOC_SCTRL_SCSPMIADDR13_ADDR(base) ((base) + (0x954))
#define SOC_SCTRL_SCSPMIADDR14_ADDR(base) ((base) + (0x958))
#define SOC_SCTRL_SCSPMIADDR15_ADDR(base) ((base) + (0x95C))
#define SOC_SCTRL_SCHISEEGPIOLOCK_ADDR(base) ((base) + (0x894))
#define SOC_SCTRL_SCHISEESPILOCK_ADDR(base) ((base) + (0x898))
#define SOC_SCTRL_SCHISEEI2CLOCK_ADDR(base) ((base) + (0x89C))
#define SOC_SCTRL_SCHISEEIOSEL_ADDR(base) ((base) + (0x8A0))
#define SOC_SCTRL_SCPERCTRL0_SEC_ADDR(base) ((base) + (0x8A4))
#define SOC_SCTRL_SCPERCTRL1_SEC_ADDR(base) ((base) + (0x8B0))
#define SOC_SCTRL_SCPERCTRL2_SEC_ADDR(base) ((base) + (0x8B4))
#define SOC_SCTRL_SCPERCTRL3_SEC_ADDR(base) ((base) + (0x8B8))
#define SOC_SCTRL_SCPERCTRL4_SEC_ADDR(base) ((base) + (0x8BC))
#define SOC_SCTRL_SCPERCTRL5_SEC_ADDR(base) ((base) + (0x8E0))
#define SOC_SCTRL_SCPERSTATUS0_SEC_ADDR(base) ((base) + (0x8C0))
#define SOC_SCTRL_SCPERSTATUS1_SEC_ADDR(base) ((base) + (0x8C4))
#define SOC_SCTRL_SCPERSTATUS2_SEC_ADDR(base) ((base) + (0x8C8))
#define SOC_SCTRL_SCPERSTATUS3_SEC_ADDR(base) ((base) + (0x8CC))
#define SOC_SCTRL_SC_TCP_VOTE_REQ_0_ADDR(base) ((base) + (0x8D0))
#define SOC_SCTRL_SC_TCP_VOTE_STAT_ADDR(base) ((base) + (0x8D4))
#define SOC_SCTRL_SC_TCP_VOTE_CTRL0_ADDR(base) ((base) + (0x8D8))
#define SOC_SCTRL_SC_TCP_VOTE_REQ_1_ADDR(base) ((base) + (0x8DC))
#define SOC_SCTRL_SC_TCP_VOTE_CTRL1_ADDR(base) ((base) + (0x8E4))
#define SOC_SCTRL_SC_TCP_VOTE_REQ3_ADD0_ADDR(base) ((base) + (0x808))
#define SOC_SCTRL_SC_TCP_VOTE_REQ3_ADD1_ADDR(base) ((base) + (0x80C))
#define SOC_SCTRL_SCPEREN0_SEC_ADDR(base) ((base) + (0x900))
#define SOC_SCTRL_SCPERDIS0_SEC_ADDR(base) ((base) + (0x904))
#define SOC_SCTRL_SCPERCLKEN0_SEC_ADDR(base) ((base) + (0x908))
#define SOC_SCTRL_SCPERRSTEN0_SEC_ADDR(base) ((base) + (0x910))
#define SOC_SCTRL_SCPERRSTDIS0_SEC_ADDR(base) ((base) + (0x914))
#define SOC_SCTRL_SCPERRSTSTAT0_SEC_ADDR(base) ((base) + (0x918))
#define SOC_SCTRL_SCPERRSTEN1_SEC_ADDR(base) ((base) + (0x990))
#define SOC_SCTRL_SCPERRSTDIS1_SEC_ADDR(base) ((base) + (0x994))
#define SOC_SCTRL_SCPERRSTSTAT1_SEC_ADDR(base) ((base) + (0x998))
#define SOC_SCTRL_SCPERRSTEN2_SEC_ADDR(base) ((base) + (0x9A0))
#define SOC_SCTRL_SCPERRSTDIS2_SEC_ADDR(base) ((base) + (0x9A4))
#define SOC_SCTRL_SCPERRSTSTAT2_SEC_ADDR(base) ((base) + (0x9A8))
#define SOC_SCTRL_SCPERRSTEN3_SEC_ADDR(base) ((base) + (0x9B0))
#define SOC_SCTRL_SCPERRSTDIS3_SEC_ADDR(base) ((base) + (0x9B4))
#define SOC_SCTRL_SCPERRSTSTAT3_SEC_ADDR(base) ((base) + (0x9B8))
#define SOC_SCTRL_SCPEREN3_SEC_ADDR(base) ((base) + (0x9C0))
#define SOC_SCTRL_SCPERDIS3_SEC_ADDR(base) ((base) + (0x9C4))
#define SOC_SCTRL_SCPERCLKEN3_SEC_ADDR(base) ((base) + (0x9C8))
#define SOC_SCTRL_SCPEREN4_SEC_ADDR(base) ((base) + (0x9D0))
#define SOC_SCTRL_SCPERDIS4_SEC_ADDR(base) ((base) + (0x9D4))
#define SOC_SCTRL_SCPERCLKEN4_SEC_ADDR(base) ((base) + (0x9D8))
#define SOC_SCTRL_SCCLKDIV0_SEC_ADDR(base) ((base) + (0x9E0))
#define SOC_SCTRL_SCCLKDIV1_SEC_ADDR(base) ((base) + (0x9E4))
#define SOC_SCTRL_SCCLKDIV2_SEC_ADDR(base) ((base) + (0x9E8))
#define SOC_SCTRL_SCCLKDIV3_SEC_ADDR(base) ((base) + (0x9EC))
#define SOC_SCTRL_SCCLKDIV4_SEC_ADDR(base) ((base) + (0x9F0))
#define SOC_SCTRL_SCCLKDIV5_SEC_ADDR(base) ((base) + (0x9F4))
#define SOC_SCTRL_SCCLKDIV6_SEC_ADDR(base) ((base) + (0x9F8))
#define SOC_SCTRL_SCCLKDIV7_SEC_ADDR(base) ((base) + (0x9FC))
#define SOC_SCTRL_SCCLKDIV8_SEC_ADDR(base) ((base) + (0x9DC))
#define SOC_SCTRL_SCCLKDIV9_SEC_ADDR(base) ((base) + (0x9CC))
#define SOC_SCTRL_SCCLKDIV10_SEC_ADDR(base) ((base) + (0xA0C))
#define SOC_SCTRL_SCPERRSTEN4_SEC_ADDR(base) ((base) + (0xA00))
#define SOC_SCTRL_SCPERRSTDIS4_SEC_ADDR(base) ((base) + (0xA04))
#define SOC_SCTRL_SCPERRSTSTAT4_SEC_ADDR(base) ((base) + (0xA08))
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_SEC_ADDR(base) ((base) + (0xA84))
#define SOC_SCTRL_SC_IPCLKRST_BYPASS1_SEC_ADDR(base) ((base) + (0xA88))
#define SOC_SCTRL_SC_IPCLKRST_BYPASS2_SEC_ADDR(base) ((base) + (0xAA0))
#define SOC_SCTRL_SC_CLK_STAT0_SEC_ADDR(base) ((base) + (0xA8C))
#define SOC_SCTRL_SC_CLK_STAT1_SEC_ADDR(base) ((base) + (0xA90))
#define SOC_SCTRL_PLL_FSM_CTRL0_SEC_ADDR(base) ((base) + (0xA94))
#define SOC_SCTRL_PLL_FSM_CTRL1_SEC_ADDR(base) ((base) + (0xA98))
#define SOC_SCTRL_PLL_FSM_STAT0_SEC_ADDR(base) ((base) + (0xA9C))
#define SOC_SCTRL_SCSOCID0_ADDR(base) ((base) + (0xE00))
#define SOC_SCTRL_SCPERSTAT_POR_RESERVED0_ADDR(base) ((base) + (0xE10))
#define SOC_SCTRL_SCPERSTAT_POR_RESERVED1_ADDR(base) ((base) + (0xE14))
#define SOC_SCTRL_SCPERSTAT_POR_RESERVED2_ADDR(base) ((base) + (0xE18))
#define SOC_SCTRL_SCPERSTAT_POR_RESERVED3_ADDR(base) ((base) + (0xE1C))
#define SOC_SCTRL_SC_NPU_LPCTRL0_SEC_ADDR(base) ((base) + (0xE20))
#define SOC_SCTRL_SC_NPU_LPCTRL1_SEC_ADDR(base) ((base) + (0xE24))
#define SOC_SCTRL_SC_NPU_LPCTRL2_SEC_ADDR(base) ((base) + (0xE28))
#define SOC_SCTRL_SC_NPU_LPCTRL3_SEC_ADDR(base) ((base) + (0xE2C))
#define SOC_SCTRL_SC_NPU_LPCTRL4_SEC_ADDR(base) ((base) + (0xE30))
#define SOC_SCTRL_SC_TRACKING_LOG0_ADDR(base) ((base) + (0xE40))
#define SOC_SCTRL_SC_TRACKING_LOG1_ADDR(base) ((base) + (0xE44))
#define SOC_SCTRL_SC_TRACKING_LOG2_ADDR(base) ((base) + (0xE48))
#define SOC_SCTRL_SC_TRACKING_LOG3_ADDR(base) ((base) + (0xE4C))
#define SOC_SCTRL_SC_EMU_VERSION_ADDR(base) ((base) + (0xE34))
#define SOC_SCTRL_SC_DDRC_CTRL_ADDR(base) ((base) + (0xE38))
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int mode_ctrl_soft : 3;
        unsigned int sys_mode : 4;
        unsigned int reserved_0 : 18;
        unsigned int deepsleep_en : 1;
        unsigned int reserved_1 : 2;
        unsigned int sc_pmu_type_sel : 1;
        unsigned int reserved_2 : 3;
    } reg;
} SOC_SCTRL_SCCTRL_UNION;
#endif
#define SOC_SCTRL_SCCTRL_mode_ctrl_soft_START (0)
#define SOC_SCTRL_SCCTRL_mode_ctrl_soft_END (2)
#define SOC_SCTRL_SCCTRL_sys_mode_START (3)
#define SOC_SCTRL_SCCTRL_sys_mode_END (6)
#define SOC_SCTRL_SCCTRL_deepsleep_en_START (25)
#define SOC_SCTRL_SCCTRL_deepsleep_en_END (25)
#define SOC_SCTRL_SCCTRL_sc_pmu_type_sel_START (28)
#define SOC_SCTRL_SCCTRL_sc_pmu_type_sel_END (28)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int fcm_tsensor0_rst_req : 1;
        unsigned int fcm_tsensor1_rst_req : 1;
        unsigned int g3d_tsensor_rst_req : 1;
        unsigned int soft_rst_req : 1;
        unsigned int wd0_rst_req : 1;
        unsigned int wd1_rst_req : 1;
        unsigned int lpmcu_rst_req : 1;
        unsigned int modem_tsensor_rst_req : 1;
        unsigned int iomcu_rst_req : 1;
        unsigned int asp_subsys_wd_req : 1;
        unsigned int venc_wd_rst_req : 1;
        unsigned int modem_wd_rst_req : 1;
        unsigned int ivp32dsp_wd_rst_req : 1;
        unsigned int isp_a7_wd_rst_req : 1;
        unsigned int wait_ddr_selfreflash_timeout : 1;
        unsigned int ddr_fatal_intr : 4;
        unsigned int ddr_uce_wd_rst_req : 4;
        unsigned int ao_wd_rst_req : 1;
        unsigned int ivp32dsp_wd_rst_req_ivp1 : 1;
        unsigned int ao_wd1_rst_req : 1;
        unsigned int ddr_lp_ctrl_wd_rst_req : 1;
        unsigned int sys_ctrl_wd_rst_req : 1;
        unsigned int gpu_lp_ctrl_wd_rst_req : 1;
        unsigned int cpu_lp_ctrl_wd_rst_req : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
    } reg;
} SOC_SCTRL_SCSYSSTAT_UNION;
#endif
#define SOC_SCTRL_SCSYSSTAT_fcm_tsensor0_rst_req_START (0)
#define SOC_SCTRL_SCSYSSTAT_fcm_tsensor0_rst_req_END (0)
#define SOC_SCTRL_SCSYSSTAT_fcm_tsensor1_rst_req_START (1)
#define SOC_SCTRL_SCSYSSTAT_fcm_tsensor1_rst_req_END (1)
#define SOC_SCTRL_SCSYSSTAT_g3d_tsensor_rst_req_START (2)
#define SOC_SCTRL_SCSYSSTAT_g3d_tsensor_rst_req_END (2)
#define SOC_SCTRL_SCSYSSTAT_soft_rst_req_START (3)
#define SOC_SCTRL_SCSYSSTAT_soft_rst_req_END (3)
#define SOC_SCTRL_SCSYSSTAT_wd0_rst_req_START (4)
#define SOC_SCTRL_SCSYSSTAT_wd0_rst_req_END (4)
#define SOC_SCTRL_SCSYSSTAT_wd1_rst_req_START (5)
#define SOC_SCTRL_SCSYSSTAT_wd1_rst_req_END (5)
#define SOC_SCTRL_SCSYSSTAT_lpmcu_rst_req_START (6)
#define SOC_SCTRL_SCSYSSTAT_lpmcu_rst_req_END (6)
#define SOC_SCTRL_SCSYSSTAT_modem_tsensor_rst_req_START (7)
#define SOC_SCTRL_SCSYSSTAT_modem_tsensor_rst_req_END (7)
#define SOC_SCTRL_SCSYSSTAT_iomcu_rst_req_START (8)
#define SOC_SCTRL_SCSYSSTAT_iomcu_rst_req_END (8)
#define SOC_SCTRL_SCSYSSTAT_asp_subsys_wd_req_START (9)
#define SOC_SCTRL_SCSYSSTAT_asp_subsys_wd_req_END (9)
#define SOC_SCTRL_SCSYSSTAT_venc_wd_rst_req_START (10)
#define SOC_SCTRL_SCSYSSTAT_venc_wd_rst_req_END (10)
#define SOC_SCTRL_SCSYSSTAT_modem_wd_rst_req_START (11)
#define SOC_SCTRL_SCSYSSTAT_modem_wd_rst_req_END (11)
#define SOC_SCTRL_SCSYSSTAT_ivp32dsp_wd_rst_req_START (12)
#define SOC_SCTRL_SCSYSSTAT_ivp32dsp_wd_rst_req_END (12)
#define SOC_SCTRL_SCSYSSTAT_isp_a7_wd_rst_req_START (13)
#define SOC_SCTRL_SCSYSSTAT_isp_a7_wd_rst_req_END (13)
#define SOC_SCTRL_SCSYSSTAT_wait_ddr_selfreflash_timeout_START (14)
#define SOC_SCTRL_SCSYSSTAT_wait_ddr_selfreflash_timeout_END (14)
#define SOC_SCTRL_SCSYSSTAT_ddr_fatal_intr_START (15)
#define SOC_SCTRL_SCSYSSTAT_ddr_fatal_intr_END (18)
#define SOC_SCTRL_SCSYSSTAT_ddr_uce_wd_rst_req_START (19)
#define SOC_SCTRL_SCSYSSTAT_ddr_uce_wd_rst_req_END (22)
#define SOC_SCTRL_SCSYSSTAT_ao_wd_rst_req_START (23)
#define SOC_SCTRL_SCSYSSTAT_ao_wd_rst_req_END (23)
#define SOC_SCTRL_SCSYSSTAT_ivp32dsp_wd_rst_req_ivp1_START (24)
#define SOC_SCTRL_SCSYSSTAT_ivp32dsp_wd_rst_req_ivp1_END (24)
#define SOC_SCTRL_SCSYSSTAT_ao_wd1_rst_req_START (25)
#define SOC_SCTRL_SCSYSSTAT_ao_wd1_rst_req_END (25)
#define SOC_SCTRL_SCSYSSTAT_ddr_lp_ctrl_wd_rst_req_START (26)
#define SOC_SCTRL_SCSYSSTAT_ddr_lp_ctrl_wd_rst_req_END (26)
#define SOC_SCTRL_SCSYSSTAT_sys_ctrl_wd_rst_req_START (27)
#define SOC_SCTRL_SCSYSSTAT_sys_ctrl_wd_rst_req_END (27)
#define SOC_SCTRL_SCSYSSTAT_gpu_lp_ctrl_wd_rst_req_START (28)
#define SOC_SCTRL_SCSYSSTAT_gpu_lp_ctrl_wd_rst_req_END (28)
#define SOC_SCTRL_SCSYSSTAT_cpu_lp_ctrl_wd_rst_req_START (29)
#define SOC_SCTRL_SCSYSSTAT_cpu_lp_ctrl_wd_rst_req_END (29)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int fcm_tsensor0_rst_req : 1;
        unsigned int fcm_tsensor1_rst_req : 1;
        unsigned int g3d_tsensor_rst_req : 1;
        unsigned int soft_rst_req : 1;
        unsigned int wd0_rst_req : 1;
        unsigned int wd1_rst_req : 1;
        unsigned int lpmcu_rst_req : 1;
        unsigned int modem_tsensor_rst_req : 1;
        unsigned int iomcu_rst_req : 1;
        unsigned int asp_subsys_wd_req : 1;
        unsigned int venc_wd_rst_req : 1;
        unsigned int modem_wd_rst_req : 1;
        unsigned int ivp32dsp_wd_rst_req : 1;
        unsigned int isp_a7_wd_rst_req : 1;
        unsigned int wait_ddr_selfreflash_timeout : 1;
        unsigned int ddr_fatal_intr : 4;
        unsigned int ddr_uce_wd_rst_req : 4;
        unsigned int ao_wd_rst_req : 1;
        unsigned int ivp32dsp_wd_rst_req_ivp1 : 1;
        unsigned int ao_wd1_rst_req : 1;
        unsigned int ddr_lp_ctrl_wd_rst_req : 1;
        unsigned int sys_ctrl_wd_rst_req : 1;
        unsigned int gpu_lp_ctrl_wd_rst_req : 1;
        unsigned int cpu_lp_ctrl_wd_rst_req : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
    } reg;
} SOC_SCTRL_SCSYSSTAT_SHADOW_UNION;
#endif
#define SOC_SCTRL_SCSYSSTAT_SHADOW_fcm_tsensor0_rst_req_START (0)
#define SOC_SCTRL_SCSYSSTAT_SHADOW_fcm_tsensor0_rst_req_END (0)
#define SOC_SCTRL_SCSYSSTAT_SHADOW_fcm_tsensor1_rst_req_START (1)
#define SOC_SCTRL_SCSYSSTAT_SHADOW_fcm_tsensor1_rst_req_END (1)
#define SOC_SCTRL_SCSYSSTAT_SHADOW_g3d_tsensor_rst_req_START (2)
#define SOC_SCTRL_SCSYSSTAT_SHADOW_g3d_tsensor_rst_req_END (2)
#define SOC_SCTRL_SCSYSSTAT_SHADOW_soft_rst_req_START (3)
#define SOC_SCTRL_SCSYSSTAT_SHADOW_soft_rst_req_END (3)
#define SOC_SCTRL_SCSYSSTAT_SHADOW_wd0_rst_req_START (4)
#define SOC_SCTRL_SCSYSSTAT_SHADOW_wd0_rst_req_END (4)
#define SOC_SCTRL_SCSYSSTAT_SHADOW_wd1_rst_req_START (5)
#define SOC_SCTRL_SCSYSSTAT_SHADOW_wd1_rst_req_END (5)
#define SOC_SCTRL_SCSYSSTAT_SHADOW_lpmcu_rst_req_START (6)
#define SOC_SCTRL_SCSYSSTAT_SHADOW_lpmcu_rst_req_END (6)
#define SOC_SCTRL_SCSYSSTAT_SHADOW_modem_tsensor_rst_req_START (7)
#define SOC_SCTRL_SCSYSSTAT_SHADOW_modem_tsensor_rst_req_END (7)
#define SOC_SCTRL_SCSYSSTAT_SHADOW_iomcu_rst_req_START (8)
#define SOC_SCTRL_SCSYSSTAT_SHADOW_iomcu_rst_req_END (8)
#define SOC_SCTRL_SCSYSSTAT_SHADOW_asp_subsys_wd_req_START (9)
#define SOC_SCTRL_SCSYSSTAT_SHADOW_asp_subsys_wd_req_END (9)
#define SOC_SCTRL_SCSYSSTAT_SHADOW_venc_wd_rst_req_START (10)
#define SOC_SCTRL_SCSYSSTAT_SHADOW_venc_wd_rst_req_END (10)
#define SOC_SCTRL_SCSYSSTAT_SHADOW_modem_wd_rst_req_START (11)
#define SOC_SCTRL_SCSYSSTAT_SHADOW_modem_wd_rst_req_END (11)
#define SOC_SCTRL_SCSYSSTAT_SHADOW_ivp32dsp_wd_rst_req_START (12)
#define SOC_SCTRL_SCSYSSTAT_SHADOW_ivp32dsp_wd_rst_req_END (12)
#define SOC_SCTRL_SCSYSSTAT_SHADOW_isp_a7_wd_rst_req_START (13)
#define SOC_SCTRL_SCSYSSTAT_SHADOW_isp_a7_wd_rst_req_END (13)
#define SOC_SCTRL_SCSYSSTAT_SHADOW_wait_ddr_selfreflash_timeout_START (14)
#define SOC_SCTRL_SCSYSSTAT_SHADOW_wait_ddr_selfreflash_timeout_END (14)
#define SOC_SCTRL_SCSYSSTAT_SHADOW_ddr_fatal_intr_START (15)
#define SOC_SCTRL_SCSYSSTAT_SHADOW_ddr_fatal_intr_END (18)
#define SOC_SCTRL_SCSYSSTAT_SHADOW_ddr_uce_wd_rst_req_START (19)
#define SOC_SCTRL_SCSYSSTAT_SHADOW_ddr_uce_wd_rst_req_END (22)
#define SOC_SCTRL_SCSYSSTAT_SHADOW_ao_wd_rst_req_START (23)
#define SOC_SCTRL_SCSYSSTAT_SHADOW_ao_wd_rst_req_END (23)
#define SOC_SCTRL_SCSYSSTAT_SHADOW_ivp32dsp_wd_rst_req_ivp1_START (24)
#define SOC_SCTRL_SCSYSSTAT_SHADOW_ivp32dsp_wd_rst_req_ivp1_END (24)
#define SOC_SCTRL_SCSYSSTAT_SHADOW_ao_wd1_rst_req_START (25)
#define SOC_SCTRL_SCSYSSTAT_SHADOW_ao_wd1_rst_req_END (25)
#define SOC_SCTRL_SCSYSSTAT_SHADOW_ddr_lp_ctrl_wd_rst_req_START (26)
#define SOC_SCTRL_SCSYSSTAT_SHADOW_ddr_lp_ctrl_wd_rst_req_END (26)
#define SOC_SCTRL_SCSYSSTAT_SHADOW_sys_ctrl_wd_rst_req_START (27)
#define SOC_SCTRL_SCSYSSTAT_SHADOW_sys_ctrl_wd_rst_req_END (27)
#define SOC_SCTRL_SCSYSSTAT_SHADOW_gpu_lp_ctrl_wd_rst_req_START (28)
#define SOC_SCTRL_SCSYSSTAT_SHADOW_gpu_lp_ctrl_wd_rst_req_END (28)
#define SOC_SCTRL_SCSYSSTAT_SHADOW_cpu_lp_ctrl_wd_rst_req_START (29)
#define SOC_SCTRL_SCSYSSTAT_SHADOW_cpu_lp_ctrl_wd_rst_req_END (29)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int sc_sw2fll_req_msk : 4;
        unsigned int sc_sft_sw2fll_req : 2;
        unsigned int reserved_0 : 2;
        unsigned int sclk_sw_mode_ctrl : 2;
        unsigned int reserved_1 : 2;
        unsigned int sc_aotcp_busy_sw2fll_msk : 1;
        unsigned int reserved_2 : 3;
        unsigned int sc_sclk_sw2fll_ctrl_msk : 16;
    } reg;
} SOC_SCTRL_SC_SCLK_SW2FLL_CTRL_UNION;
#endif
#define SOC_SCTRL_SC_SCLK_SW2FLL_CTRL_sc_sw2fll_req_msk_START (0)
#define SOC_SCTRL_SC_SCLK_SW2FLL_CTRL_sc_sw2fll_req_msk_END (3)
#define SOC_SCTRL_SC_SCLK_SW2FLL_CTRL_sc_sft_sw2fll_req_START (4)
#define SOC_SCTRL_SC_SCLK_SW2FLL_CTRL_sc_sft_sw2fll_req_END (5)
#define SOC_SCTRL_SC_SCLK_SW2FLL_CTRL_sclk_sw_mode_ctrl_START (8)
#define SOC_SCTRL_SC_SCLK_SW2FLL_CTRL_sclk_sw_mode_ctrl_END (9)
#define SOC_SCTRL_SC_SCLK_SW2FLL_CTRL_sc_aotcp_busy_sw2fll_msk_START (12)
#define SOC_SCTRL_SC_SCLK_SW2FLL_CTRL_sc_aotcp_busy_sw2fll_msk_END (12)
#define SOC_SCTRL_SC_SCLK_SW2FLL_CTRL_sc_sclk_sw2fll_ctrl_msk_START (16)
#define SOC_SCTRL_SC_SCLK_SW2FLL_CTRL_sc_sclk_sw2fll_ctrl_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int cur_clk_aobus_stat : 2;
        unsigned int reserved_0 : 2;
        unsigned int nxt_clk_aobus_stat : 2;
        unsigned int reserved_1 : 2;
        unsigned int iomcu_sclk_sw2fll_req_ff2 : 1;
        unsigned int asp_sclk_sw2fll_req_ff2 : 1;
        unsigned int sc_sclk_sw2fll_req_to_sys_fsm_pre : 1;
        unsigned int sc_sclk_sw2fll_req_to_sys_fsm : 1;
        unsigned int scfsm_sclk_sw2fll_req : 1;
        unsigned int pwfsm_sclk_sw2fll_req : 1;
        unsigned int sc_sclk_sw2fll_req : 1;
        unsigned int reserved_2 : 1;
        unsigned int tcxo_on_stat : 1;
        unsigned int tcxo_off_stat : 1;
        unsigned int reserved_3 : 2;
        unsigned int reserved_4 : 11;
        unsigned int reserved_5 : 1;
    } reg;
} SOC_SCTRL_SC_SCLK_SW2FLL_STAT_UNION;
#endif
#define SOC_SCTRL_SC_SCLK_SW2FLL_STAT_cur_clk_aobus_stat_START (0)
#define SOC_SCTRL_SC_SCLK_SW2FLL_STAT_cur_clk_aobus_stat_END (1)
#define SOC_SCTRL_SC_SCLK_SW2FLL_STAT_nxt_clk_aobus_stat_START (4)
#define SOC_SCTRL_SC_SCLK_SW2FLL_STAT_nxt_clk_aobus_stat_END (5)
#define SOC_SCTRL_SC_SCLK_SW2FLL_STAT_iomcu_sclk_sw2fll_req_ff2_START (8)
#define SOC_SCTRL_SC_SCLK_SW2FLL_STAT_iomcu_sclk_sw2fll_req_ff2_END (8)
#define SOC_SCTRL_SC_SCLK_SW2FLL_STAT_asp_sclk_sw2fll_req_ff2_START (9)
#define SOC_SCTRL_SC_SCLK_SW2FLL_STAT_asp_sclk_sw2fll_req_ff2_END (9)
#define SOC_SCTRL_SC_SCLK_SW2FLL_STAT_sc_sclk_sw2fll_req_to_sys_fsm_pre_START (10)
#define SOC_SCTRL_SC_SCLK_SW2FLL_STAT_sc_sclk_sw2fll_req_to_sys_fsm_pre_END (10)
#define SOC_SCTRL_SC_SCLK_SW2FLL_STAT_sc_sclk_sw2fll_req_to_sys_fsm_START (11)
#define SOC_SCTRL_SC_SCLK_SW2FLL_STAT_sc_sclk_sw2fll_req_to_sys_fsm_END (11)
#define SOC_SCTRL_SC_SCLK_SW2FLL_STAT_scfsm_sclk_sw2fll_req_START (12)
#define SOC_SCTRL_SC_SCLK_SW2FLL_STAT_scfsm_sclk_sw2fll_req_END (12)
#define SOC_SCTRL_SC_SCLK_SW2FLL_STAT_pwfsm_sclk_sw2fll_req_START (13)
#define SOC_SCTRL_SC_SCLK_SW2FLL_STAT_pwfsm_sclk_sw2fll_req_END (13)
#define SOC_SCTRL_SC_SCLK_SW2FLL_STAT_sc_sclk_sw2fll_req_START (14)
#define SOC_SCTRL_SC_SCLK_SW2FLL_STAT_sc_sclk_sw2fll_req_END (14)
#define SOC_SCTRL_SC_SCLK_SW2FLL_STAT_tcxo_on_stat_START (16)
#define SOC_SCTRL_SC_SCLK_SW2FLL_STAT_tcxo_on_stat_END (16)
#define SOC_SCTRL_SC_SCLK_SW2FLL_STAT_tcxo_off_stat_START (17)
#define SOC_SCTRL_SC_SCLK_SW2FLL_STAT_tcxo_off_stat_END (17)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int pw_nxt_stat : 4;
        unsigned int pw_cur_stat : 4;
        unsigned int peri_iso_en : 1;
        unsigned int peri_buck_en : 1;
        unsigned int reserved_0 : 10;
        unsigned int reserved_1 : 11;
        unsigned int reserved_2 : 1;
    } reg;
} SOC_SCTRL_SC_PERI_POWER_STAT_UNION;
#endif
#define SOC_SCTRL_SC_PERI_POWER_STAT_pw_nxt_stat_START (0)
#define SOC_SCTRL_SC_PERI_POWER_STAT_pw_nxt_stat_END (3)
#define SOC_SCTRL_SC_PERI_POWER_STAT_pw_cur_stat_START (4)
#define SOC_SCTRL_SC_PERI_POWER_STAT_pw_cur_stat_END (7)
#define SOC_SCTRL_SC_PERI_POWER_STAT_peri_iso_en_START (8)
#define SOC_SCTRL_SC_PERI_POWER_STAT_peri_iso_en_END (8)
#define SOC_SCTRL_SC_PERI_POWER_STAT_peri_buck_en_START (9)
#define SOC_SCTRL_SC_PERI_POWER_STAT_peri_buck_en_END (9)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int peri_iso_dynamic_ctrl_bypass : 1;
        unsigned int reserved_0 : 3;
        unsigned int peri_rst_dynamic_ctrl_bypass : 1;
        unsigned int reserved_1 : 27;
    } reg;
} SOC_SCTRL_SCPERIISOBYPASS_UNION;
#endif
#define SOC_SCTRL_SCPERIISOBYPASS_peri_iso_dynamic_ctrl_bypass_START (0)
#define SOC_SCTRL_SCPERIISOBYPASS_peri_iso_dynamic_ctrl_bypass_END (0)
#define SOC_SCTRL_SCPERIISOBYPASS_peri_rst_dynamic_ctrl_bypass_START (4)
#define SOC_SCTRL_SCPERIISOBYPASS_peri_rst_dynamic_ctrl_bypass_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int tcxo_on_time : 24;
        unsigned int reserved : 8;
    } reg;
} SOC_SCTRL_SC_TCXO_CTRL_0_UNION;
#endif
#define SOC_SCTRL_SC_TCXO_CTRL_0_tcxo_on_time_START (0)
#define SOC_SCTRL_SC_TCXO_CTRL_0_tcxo_on_time_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int tcxo_off_time : 24;
        unsigned int reserved : 8;
    } reg;
} SOC_SCTRL_SC_TCXO_CTRL_1_UNION;
#endif
#define SOC_SCTRL_SC_TCXO_CTRL_1_tcxo_off_time_START (0)
#define SOC_SCTRL_SC_TCXO_CTRL_1_tcxo_off_time_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int clkgt_time : 16;
        unsigned int reserved : 16;
    } reg;
} SOC_SCTRL_SC_TCXO_CTRL_2_UNION;
#endif
#define SOC_SCTRL_SC_TCXO_CTRL_2_clkgt_time_START (0)
#define SOC_SCTRL_SC_TCXO_CTRL_2_clkgt_time_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int tcxo_ctrl_sft_cfg : 1;
        unsigned int tcxo_ctrl_by_sft : 1;
        unsigned int tcxo_fast_en_ctrl : 1;
        unsigned int tcxo_ctrl_bypass : 1;
        unsigned int nxt_tcxo_ctrl_stat : 3;
        unsigned int reserved_0 : 1;
        unsigned int cur_tcxo_ctrl_stat : 3;
        unsigned int reserved_1 : 1;
        unsigned int tcxo_en_fast_req : 1;
        unsigned int tcxo_en_req_stat : 1;
        unsigned int clkgt_ctrl : 1;
        unsigned int sysclk_en : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 15;
    } reg;
} SOC_SCTRL_SC_TCXO_CTRL_3_UNION;
#endif
#define SOC_SCTRL_SC_TCXO_CTRL_3_tcxo_ctrl_sft_cfg_START (0)
#define SOC_SCTRL_SC_TCXO_CTRL_3_tcxo_ctrl_sft_cfg_END (0)
#define SOC_SCTRL_SC_TCXO_CTRL_3_tcxo_ctrl_by_sft_START (1)
#define SOC_SCTRL_SC_TCXO_CTRL_3_tcxo_ctrl_by_sft_END (1)
#define SOC_SCTRL_SC_TCXO_CTRL_3_tcxo_fast_en_ctrl_START (2)
#define SOC_SCTRL_SC_TCXO_CTRL_3_tcxo_fast_en_ctrl_END (2)
#define SOC_SCTRL_SC_TCXO_CTRL_3_tcxo_ctrl_bypass_START (3)
#define SOC_SCTRL_SC_TCXO_CTRL_3_tcxo_ctrl_bypass_END (3)
#define SOC_SCTRL_SC_TCXO_CTRL_3_nxt_tcxo_ctrl_stat_START (4)
#define SOC_SCTRL_SC_TCXO_CTRL_3_nxt_tcxo_ctrl_stat_END (6)
#define SOC_SCTRL_SC_TCXO_CTRL_3_cur_tcxo_ctrl_stat_START (8)
#define SOC_SCTRL_SC_TCXO_CTRL_3_cur_tcxo_ctrl_stat_END (10)
#define SOC_SCTRL_SC_TCXO_CTRL_3_tcxo_en_fast_req_START (12)
#define SOC_SCTRL_SC_TCXO_CTRL_3_tcxo_en_fast_req_END (12)
#define SOC_SCTRL_SC_TCXO_CTRL_3_tcxo_en_req_stat_START (13)
#define SOC_SCTRL_SC_TCXO_CTRL_3_tcxo_en_req_stat_END (13)
#define SOC_SCTRL_SC_TCXO_CTRL_3_clkgt_ctrl_START (14)
#define SOC_SCTRL_SC_TCXO_CTRL_3_clkgt_ctrl_END (14)
#define SOC_SCTRL_SC_TCXO_CTRL_3_sysclk_en_START (15)
#define SOC_SCTRL_SC_TCXO_CTRL_3_sysclk_en_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int peri_buck_on_time : 24;
        unsigned int reserved : 8;
    } reg;
} SOC_SCTRL_SC_PERI_POWER_CTRL_0_UNION;
#endif
#define SOC_SCTRL_SC_PERI_POWER_CTRL_0_peri_buck_on_time_START (0)
#define SOC_SCTRL_SC_PERI_POWER_CTRL_0_peri_buck_on_time_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int peri_buck_off_time : 24;
        unsigned int reserved : 8;
    } reg;
} SOC_SCTRL_SC_PERI_POWER_CTRL_1_UNION;
#endif
#define SOC_SCTRL_SC_PERI_POWER_CTRL_1_peri_buck_off_time_START (0)
#define SOC_SCTRL_SC_PERI_POWER_CTRL_1_peri_buck_off_time_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int peri_rst_time : 16;
        unsigned int peri_clk_off_time : 16;
    } reg;
} SOC_SCTRL_SC_PERI_POWER_CTRL_2_UNION;
#endif
#define SOC_SCTRL_SC_PERI_POWER_CTRL_2_peri_rst_time_START (0)
#define SOC_SCTRL_SC_PERI_POWER_CTRL_2_peri_rst_time_END (15)
#define SOC_SCTRL_SC_PERI_POWER_CTRL_2_peri_clk_off_time_START (16)
#define SOC_SCTRL_SC_PERI_POWER_CTRL_2_peri_clk_off_time_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int peri_iso_dis_time : 16;
        unsigned int peri_iso_en_time : 16;
    } reg;
} SOC_SCTRL_SC_PERI_POWER_CTRL_3_UNION;
#endif
#define SOC_SCTRL_SC_PERI_POWER_CTRL_3_peri_iso_dis_time_START (0)
#define SOC_SCTRL_SC_PERI_POWER_CTRL_3_peri_iso_dis_time_END (15)
#define SOC_SCTRL_SC_PERI_POWER_CTRL_3_peri_iso_en_time_START (16)
#define SOC_SCTRL_SC_PERI_POWER_CTRL_3_peri_iso_en_time_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int resume_watchdog_time : 20;
        unsigned int resume_watchdog_bypass : 1;
        unsigned int resume_watchdog_stop_pulse : 1;
        unsigned int reserved_0 : 2;
        unsigned int resume_watchdog_en : 1;
        unsigned int reserved_1 : 3;
        unsigned int sys_ctrl_wd_rst_req : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 2;
    } reg;
} SOC_SCTRL_SC_RESUME_WATCHDOG_CTRL_UNION;
#endif
#define SOC_SCTRL_SC_RESUME_WATCHDOG_CTRL_resume_watchdog_time_START (0)
#define SOC_SCTRL_SC_RESUME_WATCHDOG_CTRL_resume_watchdog_time_END (19)
#define SOC_SCTRL_SC_RESUME_WATCHDOG_CTRL_resume_watchdog_bypass_START (20)
#define SOC_SCTRL_SC_RESUME_WATCHDOG_CTRL_resume_watchdog_bypass_END (20)
#define SOC_SCTRL_SC_RESUME_WATCHDOG_CTRL_resume_watchdog_stop_pulse_START (21)
#define SOC_SCTRL_SC_RESUME_WATCHDOG_CTRL_resume_watchdog_stop_pulse_END (21)
#define SOC_SCTRL_SC_RESUME_WATCHDOG_CTRL_resume_watchdog_en_START (24)
#define SOC_SCTRL_SC_RESUME_WATCHDOG_CTRL_resume_watchdog_en_END (24)
#define SOC_SCTRL_SC_RESUME_WATCHDOG_CTRL_sys_ctrl_wd_rst_req_START (28)
#define SOC_SCTRL_SC_RESUME_WATCHDOG_CTRL_sys_ctrl_wd_rst_req_END (28)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int deepsleeped : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int sleeped : 1;
        unsigned int reserved_3 : 15;
        unsigned int reserved_4 : 11;
        unsigned int reserved_5 : 1;
    } reg;
} SOC_SCTRL_SCDEEPSLEEPED_UNION;
#endif
#define SOC_SCTRL_SCDEEPSLEEPED_deepsleeped_START (0)
#define SOC_SCTRL_SCDEEPSLEEPED_deepsleeped_END (0)
#define SOC_SCTRL_SCDEEPSLEEPED_sleeped_START (4)
#define SOC_SCTRL_SCDEEPSLEEPED_sleeped_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int system_stat_flag : 5;
        unsigned int reserved_0 : 5;
        unsigned int reserved_1 : 5;
        unsigned int reserved_2 : 1;
        unsigned int sc_lp_flag_msk : 16;
    } reg;
} SOC_SCTRL_SC_LP_FLAG_UNION;
#endif
#define SOC_SCTRL_SC_LP_FLAG_system_stat_flag_START (0)
#define SOC_SCTRL_SC_LP_FLAG_system_stat_flag_END (4)
#define SOC_SCTRL_SC_LP_FLAG_sc_lp_flag_msk_START (16)
#define SOC_SCTRL_SC_LP_FLAG_sc_lp_flag_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int intr_wakeup_sctrl2m3_stat : 1;
        unsigned int intr_wakeup_sys_ff2 : 1;
        unsigned int intr_wakeup_sctrl2m3_en_at_suspend : 1;
        unsigned int reserved_0 : 1;
        unsigned int intr_wakeup_sctrl2m3_clr_at_suspend : 1;
        unsigned int reserved_1 : 3;
        unsigned int intr_wakeup_sctrl2m3_en : 1;
        unsigned int reserved_2 : 22;
        unsigned int reserved_3 : 1;
    } reg;
} SOC_SCTRL_SC_INTR_WAKEUP_CTRL_UNION;
#endif
#define SOC_SCTRL_SC_INTR_WAKEUP_CTRL_intr_wakeup_sctrl2m3_stat_START (0)
#define SOC_SCTRL_SC_INTR_WAKEUP_CTRL_intr_wakeup_sctrl2m3_stat_END (0)
#define SOC_SCTRL_SC_INTR_WAKEUP_CTRL_intr_wakeup_sys_ff2_START (1)
#define SOC_SCTRL_SC_INTR_WAKEUP_CTRL_intr_wakeup_sys_ff2_END (1)
#define SOC_SCTRL_SC_INTR_WAKEUP_CTRL_intr_wakeup_sctrl2m3_en_at_suspend_START (2)
#define SOC_SCTRL_SC_INTR_WAKEUP_CTRL_intr_wakeup_sctrl2m3_en_at_suspend_END (2)
#define SOC_SCTRL_SC_INTR_WAKEUP_CTRL_intr_wakeup_sctrl2m3_clr_at_suspend_START (4)
#define SOC_SCTRL_SC_INTR_WAKEUP_CTRL_intr_wakeup_sctrl2m3_clr_at_suspend_END (4)
#define SOC_SCTRL_SC_INTR_WAKEUP_CTRL_intr_wakeup_sctrl2m3_en_START (8)
#define SOC_SCTRL_SC_INTR_WAKEUP_CTRL_intr_wakeup_sctrl2m3_en_END (8)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved : 32;
    } reg;
} SOC_SCTRL_SCEFUSEDATA2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved : 32;
    } reg;
} SOC_SCTRL_SCEFUSEDATA3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved : 32;
    } reg;
} SOC_SCTRL_SCEFUSEDATA8_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved : 32;
    } reg;
} SOC_SCTRL_SCEFUSEDATA9_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int lp_stat_cfg_s0 : 1;
        unsigned int lp_stat_cfg_s1 : 1;
        unsigned int lp_stat_cfg_s2 : 1;
        unsigned int lp_stat_cfg_s3 : 1;
        unsigned int lp_stat_cfg_s4 : 1;
        unsigned int reserved_0 : 26;
        unsigned int reserved_1 : 1;
    } reg;
} SOC_SCTRL_SCLPSTATCFG_UNION;
#endif
#define SOC_SCTRL_SCLPSTATCFG_lp_stat_cfg_s0_START (0)
#define SOC_SCTRL_SCLPSTATCFG_lp_stat_cfg_s0_END (0)
#define SOC_SCTRL_SCLPSTATCFG_lp_stat_cfg_s1_START (1)
#define SOC_SCTRL_SCLPSTATCFG_lp_stat_cfg_s1_END (1)
#define SOC_SCTRL_SCLPSTATCFG_lp_stat_cfg_s2_START (2)
#define SOC_SCTRL_SCLPSTATCFG_lp_stat_cfg_s2_END (2)
#define SOC_SCTRL_SCLPSTATCFG_lp_stat_cfg_s3_START (3)
#define SOC_SCTRL_SCLPSTATCFG_lp_stat_cfg_s3_END (3)
#define SOC_SCTRL_SCLPSTATCFG_lp_stat_cfg_s4_START (4)
#define SOC_SCTRL_SCLPSTATCFG_lp_stat_cfg_s4_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int intr_asp_powerdn_msk : 1;
        unsigned int intr_asp_powerup_msk : 1;
        unsigned int vote_asp_powerupdn_bypass : 1;
        unsigned int reserved_0 : 1;
        unsigned int asp_power_req_lpmcu : 2;
        unsigned int reserved_1 : 2;
        unsigned int asp_power_req_sft0 : 1;
        unsigned int asp_power_req_sft1 : 1;
        unsigned int asp_power_req_sft2 : 1;
        unsigned int asp_power_req_sft3 : 1;
        unsigned int reserved_2 : 4;
        unsigned int sc_asp_pwrcfg0_msk : 16;
    } reg;
} SOC_SCTRL_SC_ASP_PWRCFG0_UNION;
#endif
#define SOC_SCTRL_SC_ASP_PWRCFG0_intr_asp_powerdn_msk_START (0)
#define SOC_SCTRL_SC_ASP_PWRCFG0_intr_asp_powerdn_msk_END (0)
#define SOC_SCTRL_SC_ASP_PWRCFG0_intr_asp_powerup_msk_START (1)
#define SOC_SCTRL_SC_ASP_PWRCFG0_intr_asp_powerup_msk_END (1)
#define SOC_SCTRL_SC_ASP_PWRCFG0_vote_asp_powerupdn_bypass_START (2)
#define SOC_SCTRL_SC_ASP_PWRCFG0_vote_asp_powerupdn_bypass_END (2)
#define SOC_SCTRL_SC_ASP_PWRCFG0_asp_power_req_lpmcu_START (4)
#define SOC_SCTRL_SC_ASP_PWRCFG0_asp_power_req_lpmcu_END (5)
#define SOC_SCTRL_SC_ASP_PWRCFG0_asp_power_req_sft0_START (8)
#define SOC_SCTRL_SC_ASP_PWRCFG0_asp_power_req_sft0_END (8)
#define SOC_SCTRL_SC_ASP_PWRCFG0_asp_power_req_sft1_START (9)
#define SOC_SCTRL_SC_ASP_PWRCFG0_asp_power_req_sft1_END (9)
#define SOC_SCTRL_SC_ASP_PWRCFG0_asp_power_req_sft2_START (10)
#define SOC_SCTRL_SC_ASP_PWRCFG0_asp_power_req_sft2_END (10)
#define SOC_SCTRL_SC_ASP_PWRCFG0_asp_power_req_sft3_START (11)
#define SOC_SCTRL_SC_ASP_PWRCFG0_asp_power_req_sft3_END (11)
#define SOC_SCTRL_SC_ASP_PWRCFG0_sc_asp_pwrcfg0_msk_START (16)
#define SOC_SCTRL_SC_ASP_PWRCFG0_sc_asp_pwrcfg0_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int intr_asp_powerdn_clr : 1;
        unsigned int intr_asp_powerup_clr : 1;
        unsigned int reserved_0 : 2;
        unsigned int intr_asp_powerdn_raw : 1;
        unsigned int intr_asp_powerup_raw : 1;
        unsigned int reserved_1 : 2;
        unsigned int intr_asp_powerdn : 1;
        unsigned int intr_asp_powerup : 1;
        unsigned int reserved_2 : 2;
        unsigned int asp_power_req_hifi_sync : 2;
        unsigned int reserved_3 : 2;
        unsigned int asp_power_req_hifi_stat : 2;
        unsigned int reserved_4 : 2;
        unsigned int asp_power_req_cpu_sync : 2;
        unsigned int reserved_5 : 2;
        unsigned int asp_power_req : 1;
        unsigned int reserved_6 : 3;
        unsigned int curr_asp_power_stat : 2;
        unsigned int reserved_7 : 2;
    } reg;
} SOC_SCTRL_SC_ASP_PWRCFG1_UNION;
#endif
#define SOC_SCTRL_SC_ASP_PWRCFG1_intr_asp_powerdn_clr_START (0)
#define SOC_SCTRL_SC_ASP_PWRCFG1_intr_asp_powerdn_clr_END (0)
#define SOC_SCTRL_SC_ASP_PWRCFG1_intr_asp_powerup_clr_START (1)
#define SOC_SCTRL_SC_ASP_PWRCFG1_intr_asp_powerup_clr_END (1)
#define SOC_SCTRL_SC_ASP_PWRCFG1_intr_asp_powerdn_raw_START (4)
#define SOC_SCTRL_SC_ASP_PWRCFG1_intr_asp_powerdn_raw_END (4)
#define SOC_SCTRL_SC_ASP_PWRCFG1_intr_asp_powerup_raw_START (5)
#define SOC_SCTRL_SC_ASP_PWRCFG1_intr_asp_powerup_raw_END (5)
#define SOC_SCTRL_SC_ASP_PWRCFG1_intr_asp_powerdn_START (8)
#define SOC_SCTRL_SC_ASP_PWRCFG1_intr_asp_powerdn_END (8)
#define SOC_SCTRL_SC_ASP_PWRCFG1_intr_asp_powerup_START (9)
#define SOC_SCTRL_SC_ASP_PWRCFG1_intr_asp_powerup_END (9)
#define SOC_SCTRL_SC_ASP_PWRCFG1_asp_power_req_hifi_sync_START (12)
#define SOC_SCTRL_SC_ASP_PWRCFG1_asp_power_req_hifi_sync_END (13)
#define SOC_SCTRL_SC_ASP_PWRCFG1_asp_power_req_hifi_stat_START (16)
#define SOC_SCTRL_SC_ASP_PWRCFG1_asp_power_req_hifi_stat_END (17)
#define SOC_SCTRL_SC_ASP_PWRCFG1_asp_power_req_cpu_sync_START (20)
#define SOC_SCTRL_SC_ASP_PWRCFG1_asp_power_req_cpu_sync_END (21)
#define SOC_SCTRL_SC_ASP_PWRCFG1_asp_power_req_START (24)
#define SOC_SCTRL_SC_ASP_PWRCFG1_asp_power_req_END (24)
#define SOC_SCTRL_SC_ASP_PWRCFG1_curr_asp_power_stat_START (28)
#define SOC_SCTRL_SC_ASP_PWRCFG1_curr_asp_power_stat_END (29)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int int_wakeup_sys : 1;
        unsigned int drx0_int : 1;
        unsigned int drx1_int : 1;
        unsigned int notdrx_int : 1;
        unsigned int reserved : 28;
    } reg;
} SOC_SCTRL_SCINT_GATHER_STAT_UNION;
#endif
#define SOC_SCTRL_SCINT_GATHER_STAT_int_wakeup_sys_START (0)
#define SOC_SCTRL_SCINT_GATHER_STAT_int_wakeup_sys_END (0)
#define SOC_SCTRL_SCINT_GATHER_STAT_drx0_int_START (1)
#define SOC_SCTRL_SCINT_GATHER_STAT_drx0_int_END (1)
#define SOC_SCTRL_SCINT_GATHER_STAT_drx1_int_START (2)
#define SOC_SCTRL_SCINT_GATHER_STAT_drx1_int_END (2)
#define SOC_SCTRL_SCINT_GATHER_STAT_notdrx_int_START (3)
#define SOC_SCTRL_SCINT_GATHER_STAT_notdrx_int_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int rtc0_int : 1;
        unsigned int rtc1_int : 1;
        unsigned int timer00_int : 1;
        unsigned int timer01_int : 1;
        unsigned int timer10_int : 1;
        unsigned int timer11_int : 1;
        unsigned int timer20_int : 1;
        unsigned int timer21_int : 1;
        unsigned int timer30_int : 1;
        unsigned int timer31_int : 1;
        unsigned int timer40_int : 1;
        unsigned int timer41_int : 1;
        unsigned int timer50_int : 1;
        unsigned int timer51_int : 1;
        unsigned int timer60_int : 1;
        unsigned int timer61_int : 1;
        unsigned int timer70_int : 1;
        unsigned int timer71_int : 1;
        unsigned int timer80_int : 1;
        unsigned int timer81_int : 1;
        unsigned int intr_ipc_arm : 1;
        unsigned int intr_asp_watchdog : 1;
        unsigned int intr_iomcu_wdog : 1;
        unsigned int intr_iomcu_gpio_comb : 1;
        unsigned int intr_iomcu_gpio_comb_sc : 1;
        unsigned int intr_wakeup_iomcu : 1;
    } reg;
} SOC_SCTRL_SCINT_MASK_UNION;
#endif
#define SOC_SCTRL_SCINT_MASK_rtc0_int_START (6)
#define SOC_SCTRL_SCINT_MASK_rtc0_int_END (6)
#define SOC_SCTRL_SCINT_MASK_rtc1_int_START (7)
#define SOC_SCTRL_SCINT_MASK_rtc1_int_END (7)
#define SOC_SCTRL_SCINT_MASK_timer00_int_START (8)
#define SOC_SCTRL_SCINT_MASK_timer00_int_END (8)
#define SOC_SCTRL_SCINT_MASK_timer01_int_START (9)
#define SOC_SCTRL_SCINT_MASK_timer01_int_END (9)
#define SOC_SCTRL_SCINT_MASK_timer10_int_START (10)
#define SOC_SCTRL_SCINT_MASK_timer10_int_END (10)
#define SOC_SCTRL_SCINT_MASK_timer11_int_START (11)
#define SOC_SCTRL_SCINT_MASK_timer11_int_END (11)
#define SOC_SCTRL_SCINT_MASK_timer20_int_START (12)
#define SOC_SCTRL_SCINT_MASK_timer20_int_END (12)
#define SOC_SCTRL_SCINT_MASK_timer21_int_START (13)
#define SOC_SCTRL_SCINT_MASK_timer21_int_END (13)
#define SOC_SCTRL_SCINT_MASK_timer30_int_START (14)
#define SOC_SCTRL_SCINT_MASK_timer30_int_END (14)
#define SOC_SCTRL_SCINT_MASK_timer31_int_START (15)
#define SOC_SCTRL_SCINT_MASK_timer31_int_END (15)
#define SOC_SCTRL_SCINT_MASK_timer40_int_START (16)
#define SOC_SCTRL_SCINT_MASK_timer40_int_END (16)
#define SOC_SCTRL_SCINT_MASK_timer41_int_START (17)
#define SOC_SCTRL_SCINT_MASK_timer41_int_END (17)
#define SOC_SCTRL_SCINT_MASK_timer50_int_START (18)
#define SOC_SCTRL_SCINT_MASK_timer50_int_END (18)
#define SOC_SCTRL_SCINT_MASK_timer51_int_START (19)
#define SOC_SCTRL_SCINT_MASK_timer51_int_END (19)
#define SOC_SCTRL_SCINT_MASK_timer60_int_START (20)
#define SOC_SCTRL_SCINT_MASK_timer60_int_END (20)
#define SOC_SCTRL_SCINT_MASK_timer61_int_START (21)
#define SOC_SCTRL_SCINT_MASK_timer61_int_END (21)
#define SOC_SCTRL_SCINT_MASK_timer70_int_START (22)
#define SOC_SCTRL_SCINT_MASK_timer70_int_END (22)
#define SOC_SCTRL_SCINT_MASK_timer71_int_START (23)
#define SOC_SCTRL_SCINT_MASK_timer71_int_END (23)
#define SOC_SCTRL_SCINT_MASK_timer80_int_START (24)
#define SOC_SCTRL_SCINT_MASK_timer80_int_END (24)
#define SOC_SCTRL_SCINT_MASK_timer81_int_START (25)
#define SOC_SCTRL_SCINT_MASK_timer81_int_END (25)
#define SOC_SCTRL_SCINT_MASK_intr_ipc_arm_START (26)
#define SOC_SCTRL_SCINT_MASK_intr_ipc_arm_END (26)
#define SOC_SCTRL_SCINT_MASK_intr_asp_watchdog_START (27)
#define SOC_SCTRL_SCINT_MASK_intr_asp_watchdog_END (27)
#define SOC_SCTRL_SCINT_MASK_intr_iomcu_wdog_START (28)
#define SOC_SCTRL_SCINT_MASK_intr_iomcu_wdog_END (28)
#define SOC_SCTRL_SCINT_MASK_intr_iomcu_gpio_comb_START (29)
#define SOC_SCTRL_SCINT_MASK_intr_iomcu_gpio_comb_END (29)
#define SOC_SCTRL_SCINT_MASK_intr_iomcu_gpio_comb_sc_START (30)
#define SOC_SCTRL_SCINT_MASK_intr_iomcu_gpio_comb_sc_END (30)
#define SOC_SCTRL_SCINT_MASK_intr_wakeup_iomcu_START (31)
#define SOC_SCTRL_SCINT_MASK_intr_wakeup_iomcu_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int rtc0_int : 1;
        unsigned int rtc1_int : 1;
        unsigned int timer00_int : 1;
        unsigned int timer01_int : 1;
        unsigned int timer10_int : 1;
        unsigned int timer11_int : 1;
        unsigned int timer20_int : 1;
        unsigned int timer21_int : 1;
        unsigned int timer30_int : 1;
        unsigned int timer31_int : 1;
        unsigned int timer40_int : 1;
        unsigned int timer41_int : 1;
        unsigned int timer50_int : 1;
        unsigned int timer51_int : 1;
        unsigned int timer60_int : 1;
        unsigned int timer61_int : 1;
        unsigned int timer70_int : 1;
        unsigned int timer71_int : 1;
        unsigned int timer80_int : 1;
        unsigned int timer81_int : 1;
        unsigned int intr_ipc_arm : 1;
        unsigned int intr_asp_watchdog : 1;
        unsigned int intr_iomcu_wdog : 1;
        unsigned int intr_iomcu_gpio_comb : 1;
        unsigned int intr_wakeup_iomcu : 1;
        unsigned int reserved_6 : 1;
    } reg;
} SOC_SCTRL_SCINT_STAT_UNION;
#endif
#define SOC_SCTRL_SCINT_STAT_rtc0_int_START (6)
#define SOC_SCTRL_SCINT_STAT_rtc0_int_END (6)
#define SOC_SCTRL_SCINT_STAT_rtc1_int_START (7)
#define SOC_SCTRL_SCINT_STAT_rtc1_int_END (7)
#define SOC_SCTRL_SCINT_STAT_timer00_int_START (8)
#define SOC_SCTRL_SCINT_STAT_timer00_int_END (8)
#define SOC_SCTRL_SCINT_STAT_timer01_int_START (9)
#define SOC_SCTRL_SCINT_STAT_timer01_int_END (9)
#define SOC_SCTRL_SCINT_STAT_timer10_int_START (10)
#define SOC_SCTRL_SCINT_STAT_timer10_int_END (10)
#define SOC_SCTRL_SCINT_STAT_timer11_int_START (11)
#define SOC_SCTRL_SCINT_STAT_timer11_int_END (11)
#define SOC_SCTRL_SCINT_STAT_timer20_int_START (12)
#define SOC_SCTRL_SCINT_STAT_timer20_int_END (12)
#define SOC_SCTRL_SCINT_STAT_timer21_int_START (13)
#define SOC_SCTRL_SCINT_STAT_timer21_int_END (13)
#define SOC_SCTRL_SCINT_STAT_timer30_int_START (14)
#define SOC_SCTRL_SCINT_STAT_timer30_int_END (14)
#define SOC_SCTRL_SCINT_STAT_timer31_int_START (15)
#define SOC_SCTRL_SCINT_STAT_timer31_int_END (15)
#define SOC_SCTRL_SCINT_STAT_timer40_int_START (16)
#define SOC_SCTRL_SCINT_STAT_timer40_int_END (16)
#define SOC_SCTRL_SCINT_STAT_timer41_int_START (17)
#define SOC_SCTRL_SCINT_STAT_timer41_int_END (17)
#define SOC_SCTRL_SCINT_STAT_timer50_int_START (18)
#define SOC_SCTRL_SCINT_STAT_timer50_int_END (18)
#define SOC_SCTRL_SCINT_STAT_timer51_int_START (19)
#define SOC_SCTRL_SCINT_STAT_timer51_int_END (19)
#define SOC_SCTRL_SCINT_STAT_timer60_int_START (20)
#define SOC_SCTRL_SCINT_STAT_timer60_int_END (20)
#define SOC_SCTRL_SCINT_STAT_timer61_int_START (21)
#define SOC_SCTRL_SCINT_STAT_timer61_int_END (21)
#define SOC_SCTRL_SCINT_STAT_timer70_int_START (22)
#define SOC_SCTRL_SCINT_STAT_timer70_int_END (22)
#define SOC_SCTRL_SCINT_STAT_timer71_int_START (23)
#define SOC_SCTRL_SCINT_STAT_timer71_int_END (23)
#define SOC_SCTRL_SCINT_STAT_timer80_int_START (24)
#define SOC_SCTRL_SCINT_STAT_timer80_int_END (24)
#define SOC_SCTRL_SCINT_STAT_timer81_int_START (25)
#define SOC_SCTRL_SCINT_STAT_timer81_int_END (25)
#define SOC_SCTRL_SCINT_STAT_intr_ipc_arm_START (26)
#define SOC_SCTRL_SCINT_STAT_intr_ipc_arm_END (26)
#define SOC_SCTRL_SCINT_STAT_intr_asp_watchdog_START (27)
#define SOC_SCTRL_SCINT_STAT_intr_asp_watchdog_END (27)
#define SOC_SCTRL_SCINT_STAT_intr_iomcu_wdog_START (28)
#define SOC_SCTRL_SCINT_STAT_intr_iomcu_wdog_END (28)
#define SOC_SCTRL_SCINT_STAT_intr_iomcu_gpio_comb_START (29)
#define SOC_SCTRL_SCINT_STAT_intr_iomcu_gpio_comb_END (29)
#define SOC_SCTRL_SCINT_STAT_intr_wakeup_iomcu_START (30)
#define SOC_SCTRL_SCINT_STAT_intr_wakeup_iomcu_END (30)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int drx0_int : 1;
        unsigned int drx1_int : 1;
        unsigned int drx2_int : 1;
        unsigned int drx3_int : 1;
        unsigned int drx4_int : 1;
        unsigned int reserved_0 : 1;
        unsigned int drx5_int : 1;
        unsigned int drx6_int : 1;
        unsigned int drx7_int : 1;
        unsigned int drx8_int : 1;
        unsigned int reserved_1 : 21;
        unsigned int intr_iomcu_gpio_comb : 1;
    } reg;
} SOC_SCTRL_SCDRX_INT_CFG_UNION;
#endif
#define SOC_SCTRL_SCDRX_INT_CFG_drx0_int_START (0)
#define SOC_SCTRL_SCDRX_INT_CFG_drx0_int_END (0)
#define SOC_SCTRL_SCDRX_INT_CFG_drx1_int_START (1)
#define SOC_SCTRL_SCDRX_INT_CFG_drx1_int_END (1)
#define SOC_SCTRL_SCDRX_INT_CFG_drx2_int_START (2)
#define SOC_SCTRL_SCDRX_INT_CFG_drx2_int_END (2)
#define SOC_SCTRL_SCDRX_INT_CFG_drx3_int_START (3)
#define SOC_SCTRL_SCDRX_INT_CFG_drx3_int_END (3)
#define SOC_SCTRL_SCDRX_INT_CFG_drx4_int_START (4)
#define SOC_SCTRL_SCDRX_INT_CFG_drx4_int_END (4)
#define SOC_SCTRL_SCDRX_INT_CFG_drx5_int_START (6)
#define SOC_SCTRL_SCDRX_INT_CFG_drx5_int_END (6)
#define SOC_SCTRL_SCDRX_INT_CFG_drx6_int_START (7)
#define SOC_SCTRL_SCDRX_INT_CFG_drx6_int_END (7)
#define SOC_SCTRL_SCDRX_INT_CFG_drx7_int_START (8)
#define SOC_SCTRL_SCDRX_INT_CFG_drx7_int_END (8)
#define SOC_SCTRL_SCDRX_INT_CFG_drx8_int_START (9)
#define SOC_SCTRL_SCDRX_INT_CFG_drx8_int_END (9)
#define SOC_SCTRL_SCDRX_INT_CFG_intr_iomcu_gpio_comb_START (31)
#define SOC_SCTRL_SCDRX_INT_CFG_intr_iomcu_gpio_comb_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int lpmcu_wfi_int : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_SCTRL_SCLPMCUWFI_INT_UNION;
#endif
#define SOC_SCTRL_SCLPMCUWFI_INT_lpmcu_wfi_int_START (0)
#define SOC_SCTRL_SCLPMCUWFI_INT_lpmcu_wfi_int_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int intr_qic_probe_mspc : 1;
        unsigned int intr_qic_probe_iomcu : 1;
        unsigned int intr_qic_probe_asp : 1;
        unsigned int intr_qic_probe_ao : 1;
        unsigned int ao_ipc_mdm : 1;
        unsigned int ao_ipc_gic : 1;
        unsigned int ao_ipc_lpmcu : 1;
        unsigned int reserved_11 : 1;
        unsigned int ao_ipc_asp : 1;
        unsigned int reserved_12 : 1;
        unsigned int ao_ipc_iomcu : 1;
        unsigned int reserved_13 : 1;
        unsigned int reserved_14 : 1;
        unsigned int reserved_15 : 1;
        unsigned int reserved_16 : 1;
        unsigned int reserved_17 : 1;
        unsigned int reserved_18 : 1;
        unsigned int ao_ioc : 1;
        unsigned int se_gpio1 : 1;
        unsigned int mad_int : 1;
        unsigned int intr_ao_wd : 1;
    } reg;
} SOC_SCTRL_SCINT_MASK1_UNION;
#endif
#define SOC_SCTRL_SCINT_MASK1_intr_qic_probe_mspc_START (11)
#define SOC_SCTRL_SCINT_MASK1_intr_qic_probe_mspc_END (11)
#define SOC_SCTRL_SCINT_MASK1_intr_qic_probe_iomcu_START (12)
#define SOC_SCTRL_SCINT_MASK1_intr_qic_probe_iomcu_END (12)
#define SOC_SCTRL_SCINT_MASK1_intr_qic_probe_asp_START (13)
#define SOC_SCTRL_SCINT_MASK1_intr_qic_probe_asp_END (13)
#define SOC_SCTRL_SCINT_MASK1_intr_qic_probe_ao_START (14)
#define SOC_SCTRL_SCINT_MASK1_intr_qic_probe_ao_END (14)
#define SOC_SCTRL_SCINT_MASK1_ao_ipc_mdm_START (15)
#define SOC_SCTRL_SCINT_MASK1_ao_ipc_mdm_END (15)
#define SOC_SCTRL_SCINT_MASK1_ao_ipc_gic_START (16)
#define SOC_SCTRL_SCINT_MASK1_ao_ipc_gic_END (16)
#define SOC_SCTRL_SCINT_MASK1_ao_ipc_lpmcu_START (17)
#define SOC_SCTRL_SCINT_MASK1_ao_ipc_lpmcu_END (17)
#define SOC_SCTRL_SCINT_MASK1_ao_ipc_asp_START (19)
#define SOC_SCTRL_SCINT_MASK1_ao_ipc_asp_END (19)
#define SOC_SCTRL_SCINT_MASK1_ao_ipc_iomcu_START (21)
#define SOC_SCTRL_SCINT_MASK1_ao_ipc_iomcu_END (21)
#define SOC_SCTRL_SCINT_MASK1_ao_ioc_START (28)
#define SOC_SCTRL_SCINT_MASK1_ao_ioc_END (28)
#define SOC_SCTRL_SCINT_MASK1_se_gpio1_START (29)
#define SOC_SCTRL_SCINT_MASK1_se_gpio1_END (29)
#define SOC_SCTRL_SCINT_MASK1_mad_int_START (30)
#define SOC_SCTRL_SCINT_MASK1_mad_int_END (30)
#define SOC_SCTRL_SCINT_MASK1_intr_ao_wd_START (31)
#define SOC_SCTRL_SCINT_MASK1_intr_ao_wd_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int intr_qic_probe_mspc : 1;
        unsigned int intr_qic_probe_iomcu : 1;
        unsigned int intr_qic_probe_asp : 1;
        unsigned int intr_qic_probe_ao : 1;
        unsigned int ao_ipc_mdm : 1;
        unsigned int ao_ipc_gic : 1;
        unsigned int ao_ipc_lpmcu : 1;
        unsigned int reserved_11 : 1;
        unsigned int ao_ipc_asp : 1;
        unsigned int reserved_12 : 1;
        unsigned int ao_ipc_iomcu : 1;
        unsigned int reserved_13 : 1;
        unsigned int reserved_14 : 1;
        unsigned int reserved_15 : 1;
        unsigned int reserved_16 : 1;
        unsigned int reserved_17 : 1;
        unsigned int reserved_18 : 1;
        unsigned int ao_ioc : 1;
        unsigned int se_gpio1 : 1;
        unsigned int mad_int : 1;
        unsigned int intr_ao_wd : 1;
    } reg;
} SOC_SCTRL_SCINT_STAT1_UNION;
#endif
#define SOC_SCTRL_SCINT_STAT1_intr_qic_probe_mspc_START (11)
#define SOC_SCTRL_SCINT_STAT1_intr_qic_probe_mspc_END (11)
#define SOC_SCTRL_SCINT_STAT1_intr_qic_probe_iomcu_START (12)
#define SOC_SCTRL_SCINT_STAT1_intr_qic_probe_iomcu_END (12)
#define SOC_SCTRL_SCINT_STAT1_intr_qic_probe_asp_START (13)
#define SOC_SCTRL_SCINT_STAT1_intr_qic_probe_asp_END (13)
#define SOC_SCTRL_SCINT_STAT1_intr_qic_probe_ao_START (14)
#define SOC_SCTRL_SCINT_STAT1_intr_qic_probe_ao_END (14)
#define SOC_SCTRL_SCINT_STAT1_ao_ipc_mdm_START (15)
#define SOC_SCTRL_SCINT_STAT1_ao_ipc_mdm_END (15)
#define SOC_SCTRL_SCINT_STAT1_ao_ipc_gic_START (16)
#define SOC_SCTRL_SCINT_STAT1_ao_ipc_gic_END (16)
#define SOC_SCTRL_SCINT_STAT1_ao_ipc_lpmcu_START (17)
#define SOC_SCTRL_SCINT_STAT1_ao_ipc_lpmcu_END (17)
#define SOC_SCTRL_SCINT_STAT1_ao_ipc_asp_START (19)
#define SOC_SCTRL_SCINT_STAT1_ao_ipc_asp_END (19)
#define SOC_SCTRL_SCINT_STAT1_ao_ipc_iomcu_START (21)
#define SOC_SCTRL_SCINT_STAT1_ao_ipc_iomcu_END (21)
#define SOC_SCTRL_SCINT_STAT1_ao_ioc_START (28)
#define SOC_SCTRL_SCINT_STAT1_ao_ioc_END (28)
#define SOC_SCTRL_SCINT_STAT1_se_gpio1_START (29)
#define SOC_SCTRL_SCINT_STAT1_se_gpio1_END (29)
#define SOC_SCTRL_SCINT_STAT1_mad_int_START (30)
#define SOC_SCTRL_SCINT_STAT1_mad_int_END (30)
#define SOC_SCTRL_SCINT_STAT1_intr_ao_wd_START (31)
#define SOC_SCTRL_SCINT_STAT1_intr_ao_wd_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int gic_irq_int0 : 1;
        unsigned int gic_irq_int1 : 1;
        unsigned int gic_fiq_int2 : 1;
        unsigned int intr_tsp : 1;
        unsigned int intr_lpmcu_wdog : 1;
        unsigned int lpmcu_wfi_int : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 25;
    } reg;
} SOC_SCTRL_SCINT_MASK2_UNION;
#endif
#define SOC_SCTRL_SCINT_MASK2_gic_irq_int0_START (0)
#define SOC_SCTRL_SCINT_MASK2_gic_irq_int0_END (0)
#define SOC_SCTRL_SCINT_MASK2_gic_irq_int1_START (1)
#define SOC_SCTRL_SCINT_MASK2_gic_irq_int1_END (1)
#define SOC_SCTRL_SCINT_MASK2_gic_fiq_int2_START (2)
#define SOC_SCTRL_SCINT_MASK2_gic_fiq_int2_END (2)
#define SOC_SCTRL_SCINT_MASK2_intr_tsp_START (3)
#define SOC_SCTRL_SCINT_MASK2_intr_tsp_END (3)
#define SOC_SCTRL_SCINT_MASK2_intr_lpmcu_wdog_START (4)
#define SOC_SCTRL_SCINT_MASK2_intr_lpmcu_wdog_END (4)
#define SOC_SCTRL_SCINT_MASK2_lpmcu_wfi_int_START (5)
#define SOC_SCTRL_SCINT_MASK2_lpmcu_wfi_int_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int gic_irq_int0 : 1;
        unsigned int gic_irq_int1 : 1;
        unsigned int gic_fiq_int2 : 1;
        unsigned int intr_tsp : 1;
        unsigned int intr_lpmcu_wdog : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 26;
    } reg;
} SOC_SCTRL_SCINT_STAT2_UNION;
#endif
#define SOC_SCTRL_SCINT_STAT2_gic_irq_int0_START (0)
#define SOC_SCTRL_SCINT_STAT2_gic_irq_int0_END (0)
#define SOC_SCTRL_SCINT_STAT2_gic_irq_int1_START (1)
#define SOC_SCTRL_SCINT_STAT2_gic_irq_int1_END (1)
#define SOC_SCTRL_SCINT_STAT2_gic_fiq_int2_START (2)
#define SOC_SCTRL_SCINT_STAT2_gic_fiq_int2_END (2)
#define SOC_SCTRL_SCINT_STAT2_intr_tsp_START (3)
#define SOC_SCTRL_SCINT_STAT2_intr_tsp_END (3)
#define SOC_SCTRL_SCINT_STAT2_intr_lpmcu_wdog_START (4)
#define SOC_SCTRL_SCINT_STAT2_intr_lpmcu_wdog_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int intr_peri_gpio_wakeup : 5;
        unsigned int reserved_0 : 17;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int vad_int : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 5;
    } reg;
} SOC_SCTRL_SCINT_MASK3_UNION;
#endif
#define SOC_SCTRL_SCINT_MASK3_intr_peri_gpio_wakeup_START (0)
#define SOC_SCTRL_SCINT_MASK3_intr_peri_gpio_wakeup_END (4)
#define SOC_SCTRL_SCINT_MASK3_vad_int_START (24)
#define SOC_SCTRL_SCINT_MASK3_vad_int_END (24)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int intr_peri_gpio_wakeup : 5;
        unsigned int reserved_0 : 17;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int vad_int : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 5;
    } reg;
} SOC_SCTRL_SCINT_STAT3_UNION;
#endif
#define SOC_SCTRL_SCINT_STAT3_intr_peri_gpio_wakeup_START (0)
#define SOC_SCTRL_SCINT_STAT3_intr_peri_gpio_wakeup_END (4)
#define SOC_SCTRL_SCINT_STAT3_vad_int_START (24)
#define SOC_SCTRL_SCINT_STAT3_vad_int_END (24)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int lte_drx_arm_wakeup_int : 1;
        unsigned int tds_drx_arm_wakeup_int : 1;
        unsigned int g1_int_gbbp_to_cpu_on : 1;
        unsigned int g2_int_gbbp_to_cpu_on : 1;
        unsigned int w_cpu_int02 : 1;
        unsigned int cbbp_cpu_int01 : 1;
        unsigned int g3_int_gbbp_to_cpu_on : 1;
        unsigned int w_cpu_int02_2 : 1;
        unsigned int lte2_drx_arm_wakeup_int : 1;
        unsigned int ltev_drx_arm_wakeup_int : 1;
        unsigned int nr_drx_arm_wakeup_int : 1;
        unsigned int nr2_drx_arm_wakeup_int : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int reserved_11 : 1;
        unsigned int reserved_12 : 1;
        unsigned int reserved_13 : 1;
        unsigned int reserved_14 : 1;
        unsigned int reserved_15 : 1;
        unsigned int reserved_16 : 1;
        unsigned int reserved_17 : 1;
        unsigned int reserved_18 : 1;
        unsigned int reserved_19 : 1;
    } reg;
} SOC_SCTRL_SCINT_MASK4_UNION;
#endif
#define SOC_SCTRL_SCINT_MASK4_lte_drx_arm_wakeup_int_START (0)
#define SOC_SCTRL_SCINT_MASK4_lte_drx_arm_wakeup_int_END (0)
#define SOC_SCTRL_SCINT_MASK4_tds_drx_arm_wakeup_int_START (1)
#define SOC_SCTRL_SCINT_MASK4_tds_drx_arm_wakeup_int_END (1)
#define SOC_SCTRL_SCINT_MASK4_g1_int_gbbp_to_cpu_on_START (2)
#define SOC_SCTRL_SCINT_MASK4_g1_int_gbbp_to_cpu_on_END (2)
#define SOC_SCTRL_SCINT_MASK4_g2_int_gbbp_to_cpu_on_START (3)
#define SOC_SCTRL_SCINT_MASK4_g2_int_gbbp_to_cpu_on_END (3)
#define SOC_SCTRL_SCINT_MASK4_w_cpu_int02_START (4)
#define SOC_SCTRL_SCINT_MASK4_w_cpu_int02_END (4)
#define SOC_SCTRL_SCINT_MASK4_cbbp_cpu_int01_START (5)
#define SOC_SCTRL_SCINT_MASK4_cbbp_cpu_int01_END (5)
#define SOC_SCTRL_SCINT_MASK4_g3_int_gbbp_to_cpu_on_START (6)
#define SOC_SCTRL_SCINT_MASK4_g3_int_gbbp_to_cpu_on_END (6)
#define SOC_SCTRL_SCINT_MASK4_w_cpu_int02_2_START (7)
#define SOC_SCTRL_SCINT_MASK4_w_cpu_int02_2_END (7)
#define SOC_SCTRL_SCINT_MASK4_lte2_drx_arm_wakeup_int_START (8)
#define SOC_SCTRL_SCINT_MASK4_lte2_drx_arm_wakeup_int_END (8)
#define SOC_SCTRL_SCINT_MASK4_ltev_drx_arm_wakeup_int_START (9)
#define SOC_SCTRL_SCINT_MASK4_ltev_drx_arm_wakeup_int_END (9)
#define SOC_SCTRL_SCINT_MASK4_nr_drx_arm_wakeup_int_START (10)
#define SOC_SCTRL_SCINT_MASK4_nr_drx_arm_wakeup_int_END (10)
#define SOC_SCTRL_SCINT_MASK4_nr2_drx_arm_wakeup_int_START (11)
#define SOC_SCTRL_SCINT_MASK4_nr2_drx_arm_wakeup_int_END (11)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int lte_drx_arm_wakeup_int : 1;
        unsigned int tds_drx_arm_wakeup_int : 1;
        unsigned int g1_int_gbbp_to_cpu_on : 1;
        unsigned int g2_int_gbbp_to_cpu_on : 1;
        unsigned int w_cpu_int02 : 1;
        unsigned int cbbp_cpu_int01 : 1;
        unsigned int g3_int_gbbp_to_cpu_on : 1;
        unsigned int w_cpu_int02_2 : 1;
        unsigned int lte2_drx_arm_wakeup_int : 1;
        unsigned int ltev_drx_arm_wakeup_int : 1;
        unsigned int nr_drx_arm_wakeup_int : 1;
        unsigned int nr2_drx_arm_wakeup_int : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int reserved_11 : 1;
        unsigned int reserved_12 : 1;
        unsigned int reserved_13 : 1;
        unsigned int reserved_14 : 1;
        unsigned int reserved_15 : 1;
        unsigned int reserved_16 : 1;
        unsigned int reserved_17 : 1;
        unsigned int reserved_18 : 1;
        unsigned int reserved_19 : 1;
    } reg;
} SOC_SCTRL_SCINT_STAT4_UNION;
#endif
#define SOC_SCTRL_SCINT_STAT4_lte_drx_arm_wakeup_int_START (0)
#define SOC_SCTRL_SCINT_STAT4_lte_drx_arm_wakeup_int_END (0)
#define SOC_SCTRL_SCINT_STAT4_tds_drx_arm_wakeup_int_START (1)
#define SOC_SCTRL_SCINT_STAT4_tds_drx_arm_wakeup_int_END (1)
#define SOC_SCTRL_SCINT_STAT4_g1_int_gbbp_to_cpu_on_START (2)
#define SOC_SCTRL_SCINT_STAT4_g1_int_gbbp_to_cpu_on_END (2)
#define SOC_SCTRL_SCINT_STAT4_g2_int_gbbp_to_cpu_on_START (3)
#define SOC_SCTRL_SCINT_STAT4_g2_int_gbbp_to_cpu_on_END (3)
#define SOC_SCTRL_SCINT_STAT4_w_cpu_int02_START (4)
#define SOC_SCTRL_SCINT_STAT4_w_cpu_int02_END (4)
#define SOC_SCTRL_SCINT_STAT4_cbbp_cpu_int01_START (5)
#define SOC_SCTRL_SCINT_STAT4_cbbp_cpu_int01_END (5)
#define SOC_SCTRL_SCINT_STAT4_g3_int_gbbp_to_cpu_on_START (6)
#define SOC_SCTRL_SCINT_STAT4_g3_int_gbbp_to_cpu_on_END (6)
#define SOC_SCTRL_SCINT_STAT4_w_cpu_int02_2_START (7)
#define SOC_SCTRL_SCINT_STAT4_w_cpu_int02_2_END (7)
#define SOC_SCTRL_SCINT_STAT4_lte2_drx_arm_wakeup_int_START (8)
#define SOC_SCTRL_SCINT_STAT4_lte2_drx_arm_wakeup_int_END (8)
#define SOC_SCTRL_SCINT_STAT4_ltev_drx_arm_wakeup_int_START (9)
#define SOC_SCTRL_SCINT_STAT4_ltev_drx_arm_wakeup_int_END (9)
#define SOC_SCTRL_SCINT_STAT4_nr_drx_arm_wakeup_int_START (10)
#define SOC_SCTRL_SCINT_STAT4_nr_drx_arm_wakeup_int_END (10)
#define SOC_SCTRL_SCINT_STAT4_nr2_drx_arm_wakeup_int_START (11)
#define SOC_SCTRL_SCINT_STAT4_nr2_drx_arm_wakeup_int_END (11)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int intr_timer130 : 1;
        unsigned int intr_timer131 : 1;
        unsigned int intr_timer140 : 1;
        unsigned int intr_timer141 : 1;
        unsigned int intr_timer150 : 1;
        unsigned int intr_timer151 : 1;
        unsigned int intr_timer160 : 1;
        unsigned int intr_timer161 : 1;
        unsigned int intr_timer170 : 1;
        unsigned int intr_timer171 : 1;
        unsigned int pcie0_l12wake_int : 1;
        unsigned int intr_ao_wd1 : 1;
        unsigned int intr_wakeup_swing_enter_req : 1;
        unsigned int intr_wakeup_swing_exit_req : 1;
        unsigned int intr_lfc_mode_quit_to_sctrl_and_lpmcu : 1;
        unsigned int intr_usb_sys_error_to_ao_wakeup : 1;
        unsigned int intr_mspc_timer_sysfsm0 : 1;
        unsigned int intr_m7_ahb_dbg_timeout_lpmcu : 1;
        unsigned int pcie1_l12wake_int : 1;
        unsigned int intr_usb_sum_gic_lpmcu_iomcu : 1;
        unsigned int intr_mspc_timer_sysfsm1 : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
    } reg;
} SOC_SCTRL_SCINT_MASK5_UNION;
#endif
#define SOC_SCTRL_SCINT_MASK5_intr_timer130_START (0)
#define SOC_SCTRL_SCINT_MASK5_intr_timer130_END (0)
#define SOC_SCTRL_SCINT_MASK5_intr_timer131_START (1)
#define SOC_SCTRL_SCINT_MASK5_intr_timer131_END (1)
#define SOC_SCTRL_SCINT_MASK5_intr_timer140_START (2)
#define SOC_SCTRL_SCINT_MASK5_intr_timer140_END (2)
#define SOC_SCTRL_SCINT_MASK5_intr_timer141_START (3)
#define SOC_SCTRL_SCINT_MASK5_intr_timer141_END (3)
#define SOC_SCTRL_SCINT_MASK5_intr_timer150_START (4)
#define SOC_SCTRL_SCINT_MASK5_intr_timer150_END (4)
#define SOC_SCTRL_SCINT_MASK5_intr_timer151_START (5)
#define SOC_SCTRL_SCINT_MASK5_intr_timer151_END (5)
#define SOC_SCTRL_SCINT_MASK5_intr_timer160_START (6)
#define SOC_SCTRL_SCINT_MASK5_intr_timer160_END (6)
#define SOC_SCTRL_SCINT_MASK5_intr_timer161_START (7)
#define SOC_SCTRL_SCINT_MASK5_intr_timer161_END (7)
#define SOC_SCTRL_SCINT_MASK5_intr_timer170_START (8)
#define SOC_SCTRL_SCINT_MASK5_intr_timer170_END (8)
#define SOC_SCTRL_SCINT_MASK5_intr_timer171_START (9)
#define SOC_SCTRL_SCINT_MASK5_intr_timer171_END (9)
#define SOC_SCTRL_SCINT_MASK5_pcie0_l12wake_int_START (10)
#define SOC_SCTRL_SCINT_MASK5_pcie0_l12wake_int_END (10)
#define SOC_SCTRL_SCINT_MASK5_intr_ao_wd1_START (11)
#define SOC_SCTRL_SCINT_MASK5_intr_ao_wd1_END (11)
#define SOC_SCTRL_SCINT_MASK5_intr_wakeup_swing_enter_req_START (12)
#define SOC_SCTRL_SCINT_MASK5_intr_wakeup_swing_enter_req_END (12)
#define SOC_SCTRL_SCINT_MASK5_intr_wakeup_swing_exit_req_START (13)
#define SOC_SCTRL_SCINT_MASK5_intr_wakeup_swing_exit_req_END (13)
#define SOC_SCTRL_SCINT_MASK5_intr_lfc_mode_quit_to_sctrl_and_lpmcu_START (14)
#define SOC_SCTRL_SCINT_MASK5_intr_lfc_mode_quit_to_sctrl_and_lpmcu_END (14)
#define SOC_SCTRL_SCINT_MASK5_intr_usb_sys_error_to_ao_wakeup_START (15)
#define SOC_SCTRL_SCINT_MASK5_intr_usb_sys_error_to_ao_wakeup_END (15)
#define SOC_SCTRL_SCINT_MASK5_intr_mspc_timer_sysfsm0_START (16)
#define SOC_SCTRL_SCINT_MASK5_intr_mspc_timer_sysfsm0_END (16)
#define SOC_SCTRL_SCINT_MASK5_intr_m7_ahb_dbg_timeout_lpmcu_START (17)
#define SOC_SCTRL_SCINT_MASK5_intr_m7_ahb_dbg_timeout_lpmcu_END (17)
#define SOC_SCTRL_SCINT_MASK5_pcie1_l12wake_int_START (18)
#define SOC_SCTRL_SCINT_MASK5_pcie1_l12wake_int_END (18)
#define SOC_SCTRL_SCINT_MASK5_intr_usb_sum_gic_lpmcu_iomcu_START (19)
#define SOC_SCTRL_SCINT_MASK5_intr_usb_sum_gic_lpmcu_iomcu_END (19)
#define SOC_SCTRL_SCINT_MASK5_intr_mspc_timer_sysfsm1_START (20)
#define SOC_SCTRL_SCINT_MASK5_intr_mspc_timer_sysfsm1_END (20)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int intr_timer130 : 1;
        unsigned int intr_timer131 : 1;
        unsigned int intr_timer140 : 1;
        unsigned int intr_timer141 : 1;
        unsigned int intr_timer150 : 1;
        unsigned int intr_timer151 : 1;
        unsigned int intr_timer160 : 1;
        unsigned int intr_timer161 : 1;
        unsigned int intr_timer170 : 1;
        unsigned int intr_timer171 : 1;
        unsigned int pcie0_l12wake_int : 1;
        unsigned int intr_ao_wd1 : 1;
        unsigned int intr_wakeup_swing_enter_req : 1;
        unsigned int intr_wakeup_swing_exit_req : 1;
        unsigned int intr_lfc_mode_quit_to_sctrl_and_lpmcu : 1;
        unsigned int intr_usb_sys_error_to_ao_wakeup : 1;
        unsigned int intr_mspc_timer_sysfsm0 : 1;
        unsigned int intr_m7_ahb_dbg_timeout_lpmcu : 1;
        unsigned int pcie1_l12wake_int : 1;
        unsigned int intr_usb_sum_gic_lpmcu_iomcu : 1;
        unsigned int intr_mspc_timer_sysfsm1 : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
    } reg;
} SOC_SCTRL_SCINT_STAT5_UNION;
#endif
#define SOC_SCTRL_SCINT_STAT5_intr_timer130_START (0)
#define SOC_SCTRL_SCINT_STAT5_intr_timer130_END (0)
#define SOC_SCTRL_SCINT_STAT5_intr_timer131_START (1)
#define SOC_SCTRL_SCINT_STAT5_intr_timer131_END (1)
#define SOC_SCTRL_SCINT_STAT5_intr_timer140_START (2)
#define SOC_SCTRL_SCINT_STAT5_intr_timer140_END (2)
#define SOC_SCTRL_SCINT_STAT5_intr_timer141_START (3)
#define SOC_SCTRL_SCINT_STAT5_intr_timer141_END (3)
#define SOC_SCTRL_SCINT_STAT5_intr_timer150_START (4)
#define SOC_SCTRL_SCINT_STAT5_intr_timer150_END (4)
#define SOC_SCTRL_SCINT_STAT5_intr_timer151_START (5)
#define SOC_SCTRL_SCINT_STAT5_intr_timer151_END (5)
#define SOC_SCTRL_SCINT_STAT5_intr_timer160_START (6)
#define SOC_SCTRL_SCINT_STAT5_intr_timer160_END (6)
#define SOC_SCTRL_SCINT_STAT5_intr_timer161_START (7)
#define SOC_SCTRL_SCINT_STAT5_intr_timer161_END (7)
#define SOC_SCTRL_SCINT_STAT5_intr_timer170_START (8)
#define SOC_SCTRL_SCINT_STAT5_intr_timer170_END (8)
#define SOC_SCTRL_SCINT_STAT5_intr_timer171_START (9)
#define SOC_SCTRL_SCINT_STAT5_intr_timer171_END (9)
#define SOC_SCTRL_SCINT_STAT5_pcie0_l12wake_int_START (10)
#define SOC_SCTRL_SCINT_STAT5_pcie0_l12wake_int_END (10)
#define SOC_SCTRL_SCINT_STAT5_intr_ao_wd1_START (11)
#define SOC_SCTRL_SCINT_STAT5_intr_ao_wd1_END (11)
#define SOC_SCTRL_SCINT_STAT5_intr_wakeup_swing_enter_req_START (12)
#define SOC_SCTRL_SCINT_STAT5_intr_wakeup_swing_enter_req_END (12)
#define SOC_SCTRL_SCINT_STAT5_intr_wakeup_swing_exit_req_START (13)
#define SOC_SCTRL_SCINT_STAT5_intr_wakeup_swing_exit_req_END (13)
#define SOC_SCTRL_SCINT_STAT5_intr_lfc_mode_quit_to_sctrl_and_lpmcu_START (14)
#define SOC_SCTRL_SCINT_STAT5_intr_lfc_mode_quit_to_sctrl_and_lpmcu_END (14)
#define SOC_SCTRL_SCINT_STAT5_intr_usb_sys_error_to_ao_wakeup_START (15)
#define SOC_SCTRL_SCINT_STAT5_intr_usb_sys_error_to_ao_wakeup_END (15)
#define SOC_SCTRL_SCINT_STAT5_intr_mspc_timer_sysfsm0_START (16)
#define SOC_SCTRL_SCINT_STAT5_intr_mspc_timer_sysfsm0_END (16)
#define SOC_SCTRL_SCINT_STAT5_intr_m7_ahb_dbg_timeout_lpmcu_START (17)
#define SOC_SCTRL_SCINT_STAT5_intr_m7_ahb_dbg_timeout_lpmcu_END (17)
#define SOC_SCTRL_SCINT_STAT5_pcie1_l12wake_int_START (18)
#define SOC_SCTRL_SCINT_STAT5_pcie1_l12wake_int_END (18)
#define SOC_SCTRL_SCINT_STAT5_intr_usb_sum_gic_lpmcu_iomcu_START (19)
#define SOC_SCTRL_SCINT_STAT5_intr_usb_sum_gic_lpmcu_iomcu_END (19)
#define SOC_SCTRL_SCINT_STAT5_intr_mspc_timer_sysfsm1_START (20)
#define SOC_SCTRL_SCINT_STAT5_intr_mspc_timer_sysfsm1_END (20)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int spll_en_vote : 16;
        unsigned int sc_pll_en_vote_0_msk : 16;
    } reg;
} SOC_SCTRL_SC_PLL_EN_VOTE_0_UNION;
#endif
#define SOC_SCTRL_SC_PLL_EN_VOTE_0_spll_en_vote_START (0)
#define SOC_SCTRL_SC_PLL_EN_VOTE_0_spll_en_vote_END (15)
#define SOC_SCTRL_SC_PLL_EN_VOTE_0_sc_pll_en_vote_0_msk_START (16)
#define SOC_SCTRL_SC_PLL_EN_VOTE_0_sc_pll_en_vote_0_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ulppll_en_vote : 16;
        unsigned int sc_pll_en_vote_1_msk : 16;
    } reg;
} SOC_SCTRL_SC_PLL_EN_VOTE_1_UNION;
#endif
#define SOC_SCTRL_SC_PLL_EN_VOTE_1_ulppll_en_vote_START (0)
#define SOC_SCTRL_SC_PLL_EN_VOTE_1_ulppll_en_vote_END (15)
#define SOC_SCTRL_SC_PLL_EN_VOTE_1_sc_pll_en_vote_1_msk_START (16)
#define SOC_SCTRL_SC_PLL_EN_VOTE_1_sc_pll_en_vote_1_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int lbintjpll_en_vote : 16;
        unsigned int sc_pll_en_vote_2_msk : 16;
    } reg;
} SOC_SCTRL_SC_PLL_EN_VOTE_2_UNION;
#endif
#define SOC_SCTRL_SC_PLL_EN_VOTE_2_lbintjpll_en_vote_START (0)
#define SOC_SCTRL_SC_PLL_EN_VOTE_2_lbintjpll_en_vote_END (15)
#define SOC_SCTRL_SC_PLL_EN_VOTE_2_sc_pll_en_vote_2_msk_START (16)
#define SOC_SCTRL_SC_PLL_EN_VOTE_2_sc_pll_en_vote_2_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ulppll_1_en_vote : 16;
        unsigned int sc_pll_en_vote_3_msk : 16;
    } reg;
} SOC_SCTRL_SC_PLL_EN_VOTE_3_UNION;
#endif
#define SOC_SCTRL_SC_PLL_EN_VOTE_3_ulppll_1_en_vote_START (0)
#define SOC_SCTRL_SC_PLL_EN_VOTE_3_ulppll_1_en_vote_END (15)
#define SOC_SCTRL_SC_PLL_EN_VOTE_3_sc_pll_en_vote_3_msk_START (16)
#define SOC_SCTRL_SC_PLL_EN_VOTE_3_sc_pll_en_vote_3_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int spll_gt_vote : 16;
        unsigned int sc_pll_gt_vote_0_msk : 16;
    } reg;
} SOC_SCTRL_SC_PLL_GT_VOTE_0_UNION;
#endif
#define SOC_SCTRL_SC_PLL_GT_VOTE_0_spll_gt_vote_START (0)
#define SOC_SCTRL_SC_PLL_GT_VOTE_0_spll_gt_vote_END (15)
#define SOC_SCTRL_SC_PLL_GT_VOTE_0_sc_pll_gt_vote_0_msk_START (16)
#define SOC_SCTRL_SC_PLL_GT_VOTE_0_sc_pll_gt_vote_0_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ulppll_gt_vote : 16;
        unsigned int sc_pll_gt_vote_1_msk : 16;
    } reg;
} SOC_SCTRL_SC_PLL_GT_VOTE_1_UNION;
#endif
#define SOC_SCTRL_SC_PLL_GT_VOTE_1_ulppll_gt_vote_START (0)
#define SOC_SCTRL_SC_PLL_GT_VOTE_1_ulppll_gt_vote_END (15)
#define SOC_SCTRL_SC_PLL_GT_VOTE_1_sc_pll_gt_vote_1_msk_START (16)
#define SOC_SCTRL_SC_PLL_GT_VOTE_1_sc_pll_gt_vote_1_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int lbintjpll_gt_vote : 16;
        unsigned int sc_pll_gt_vote_2_msk : 16;
    } reg;
} SOC_SCTRL_SC_PLL_GT_VOTE_2_UNION;
#endif
#define SOC_SCTRL_SC_PLL_GT_VOTE_2_lbintjpll_gt_vote_START (0)
#define SOC_SCTRL_SC_PLL_GT_VOTE_2_lbintjpll_gt_vote_END (15)
#define SOC_SCTRL_SC_PLL_GT_VOTE_2_sc_pll_gt_vote_2_msk_START (16)
#define SOC_SCTRL_SC_PLL_GT_VOTE_2_sc_pll_gt_vote_2_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ulppll_1_gt_vote : 16;
        unsigned int sc_pll_gt_vote_3_msk : 16;
    } reg;
} SOC_SCTRL_SC_PLL_GT_VOTE_3_UNION;
#endif
#define SOC_SCTRL_SC_PLL_GT_VOTE_3_ulppll_1_gt_vote_START (0)
#define SOC_SCTRL_SC_PLL_GT_VOTE_3_ulppll_1_gt_vote_END (15)
#define SOC_SCTRL_SC_PLL_GT_VOTE_3_sc_pll_gt_vote_3_msk_START (16)
#define SOC_SCTRL_SC_PLL_GT_VOTE_3_sc_pll_gt_vote_3_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int spll_bp_vote : 16;
        unsigned int sc_pll_bp_vote_0_msk : 16;
    } reg;
} SOC_SCTRL_SC_PLL_BP_VOTE_0_UNION;
#endif
#define SOC_SCTRL_SC_PLL_BP_VOTE_0_spll_bp_vote_START (0)
#define SOC_SCTRL_SC_PLL_BP_VOTE_0_spll_bp_vote_END (15)
#define SOC_SCTRL_SC_PLL_BP_VOTE_0_sc_pll_bp_vote_0_msk_START (16)
#define SOC_SCTRL_SC_PLL_BP_VOTE_0_sc_pll_bp_vote_0_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ulppll_bp_vote : 16;
        unsigned int sc_pll_bp_vote_1_msk : 16;
    } reg;
} SOC_SCTRL_SC_PLL_BP_VOTE_1_UNION;
#endif
#define SOC_SCTRL_SC_PLL_BP_VOTE_1_ulppll_bp_vote_START (0)
#define SOC_SCTRL_SC_PLL_BP_VOTE_1_ulppll_bp_vote_END (15)
#define SOC_SCTRL_SC_PLL_BP_VOTE_1_sc_pll_bp_vote_1_msk_START (16)
#define SOC_SCTRL_SC_PLL_BP_VOTE_1_sc_pll_bp_vote_1_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int lbintjpll_bp_vote : 16;
        unsigned int sc_pll_bp_vote_2_msk : 16;
    } reg;
} SOC_SCTRL_SC_PLL_BP_VOTE_2_UNION;
#endif
#define SOC_SCTRL_SC_PLL_BP_VOTE_2_lbintjpll_bp_vote_START (0)
#define SOC_SCTRL_SC_PLL_BP_VOTE_2_lbintjpll_bp_vote_END (15)
#define SOC_SCTRL_SC_PLL_BP_VOTE_2_sc_pll_bp_vote_2_msk_START (16)
#define SOC_SCTRL_SC_PLL_BP_VOTE_2_sc_pll_bp_vote_2_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ulppll_1_bp_vote : 16;
        unsigned int sc_pll_bp_vote_3_msk : 16;
    } reg;
} SOC_SCTRL_SC_PLL_BP_VOTE_3_UNION;
#endif
#define SOC_SCTRL_SC_PLL_BP_VOTE_3_ulppll_1_bp_vote_START (0)
#define SOC_SCTRL_SC_PLL_BP_VOTE_3_ulppll_1_bp_vote_END (15)
#define SOC_SCTRL_SC_PLL_BP_VOTE_3_sc_pll_bp_vote_3_msk_START (16)
#define SOC_SCTRL_SC_PLL_BP_VOTE_3_sc_pll_bp_vote_3_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int spll_lock : 1;
        unsigned int spll_en_stat : 1;
        unsigned int spll_gt_stat : 1;
        unsigned int spll_bypass_stat : 1;
        unsigned int ulppll_en_stat : 1;
        unsigned int ulppll_gt_stat : 1;
        unsigned int ulppll_bp_stat : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int ulppll_1_en_stat : 1;
        unsigned int ulppll_1_gt_stat : 1;
        unsigned int ulppll_1_bp_stat : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 16;
    } reg;
} SOC_SCTRL_SC_PLL_STAT_UNION;
#endif
#define SOC_SCTRL_SC_PLL_STAT_spll_lock_START (0)
#define SOC_SCTRL_SC_PLL_STAT_spll_lock_END (0)
#define SOC_SCTRL_SC_PLL_STAT_spll_en_stat_START (1)
#define SOC_SCTRL_SC_PLL_STAT_spll_en_stat_END (1)
#define SOC_SCTRL_SC_PLL_STAT_spll_gt_stat_START (2)
#define SOC_SCTRL_SC_PLL_STAT_spll_gt_stat_END (2)
#define SOC_SCTRL_SC_PLL_STAT_spll_bypass_stat_START (3)
#define SOC_SCTRL_SC_PLL_STAT_spll_bypass_stat_END (3)
#define SOC_SCTRL_SC_PLL_STAT_ulppll_en_stat_START (4)
#define SOC_SCTRL_SC_PLL_STAT_ulppll_en_stat_END (4)
#define SOC_SCTRL_SC_PLL_STAT_ulppll_gt_stat_START (5)
#define SOC_SCTRL_SC_PLL_STAT_ulppll_gt_stat_END (5)
#define SOC_SCTRL_SC_PLL_STAT_ulppll_bp_stat_START (6)
#define SOC_SCTRL_SC_PLL_STAT_ulppll_bp_stat_END (6)
#define SOC_SCTRL_SC_PLL_STAT_ulppll_1_en_stat_START (12)
#define SOC_SCTRL_SC_PLL_STAT_ulppll_1_en_stat_END (12)
#define SOC_SCTRL_SC_PLL_STAT_ulppll_1_gt_stat_START (13)
#define SOC_SCTRL_SC_PLL_STAT_ulppll_1_gt_stat_END (13)
#define SOC_SCTRL_SC_PLL_STAT_ulppll_1_bp_stat_START (14)
#define SOC_SCTRL_SC_PLL_STAT_ulppll_1_bp_stat_END (14)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int spll_en : 1;
        unsigned int spll_bp : 1;
        unsigned int spll_refdiv : 6;
        unsigned int spll_fbdiv : 12;
        unsigned int spll_postdiv1 : 3;
        unsigned int spll_postdiv2 : 3;
        unsigned int reserved : 6;
    } reg;
} SOC_SCTRL_SCSPLLCTRL0_UNION;
#endif
#define SOC_SCTRL_SCSPLLCTRL0_spll_en_START (0)
#define SOC_SCTRL_SCSPLLCTRL0_spll_en_END (0)
#define SOC_SCTRL_SCSPLLCTRL0_spll_bp_START (1)
#define SOC_SCTRL_SCSPLLCTRL0_spll_bp_END (1)
#define SOC_SCTRL_SCSPLLCTRL0_spll_refdiv_START (2)
#define SOC_SCTRL_SCSPLLCTRL0_spll_refdiv_END (7)
#define SOC_SCTRL_SCSPLLCTRL0_spll_fbdiv_START (8)
#define SOC_SCTRL_SCSPLLCTRL0_spll_fbdiv_END (19)
#define SOC_SCTRL_SCSPLLCTRL0_spll_postdiv1_START (20)
#define SOC_SCTRL_SCSPLLCTRL0_spll_postdiv1_END (22)
#define SOC_SCTRL_SCSPLLCTRL0_spll_postdiv2_START (23)
#define SOC_SCTRL_SCSPLLCTRL0_spll_postdiv2_END (25)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int spll_fracdiv : 24;
        unsigned int spll_int_mod : 1;
        unsigned int spll_cfg_vld : 1;
        unsigned int gt_clk_spll : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 4;
    } reg;
} SOC_SCTRL_SCSPLLCTRL1_UNION;
#endif
#define SOC_SCTRL_SCSPLLCTRL1_spll_fracdiv_START (0)
#define SOC_SCTRL_SCSPLLCTRL1_spll_fracdiv_END (23)
#define SOC_SCTRL_SCSPLLCTRL1_spll_int_mod_START (24)
#define SOC_SCTRL_SCSPLLCTRL1_spll_int_mod_END (24)
#define SOC_SCTRL_SCSPLLCTRL1_spll_cfg_vld_START (25)
#define SOC_SCTRL_SCSPLLCTRL1_spll_cfg_vld_END (25)
#define SOC_SCTRL_SCSPLLCTRL1_gt_clk_spll_START (26)
#define SOC_SCTRL_SCSPLLCTRL1_gt_clk_spll_END (26)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int sscg_enable : 1;
        unsigned int sscg_mode : 1;
        unsigned int sscg_rate : 4;
        unsigned int sscg_depth : 3;
        unsigned int sscg_fast_dis : 1;
        unsigned int reserved : 6;
        unsigned int bitmasken : 16;
    } reg;
} SOC_SCTRL_SCSPLLSSCCTRL_UNION;
#endif
#define SOC_SCTRL_SCSPLLSSCCTRL_sscg_enable_START (0)
#define SOC_SCTRL_SCSPLLSSCCTRL_sscg_enable_END (0)
#define SOC_SCTRL_SCSPLLSSCCTRL_sscg_mode_START (1)
#define SOC_SCTRL_SCSPLLSSCCTRL_sscg_mode_END (1)
#define SOC_SCTRL_SCSPLLSSCCTRL_sscg_rate_START (2)
#define SOC_SCTRL_SCSPLLSSCCTRL_sscg_rate_END (5)
#define SOC_SCTRL_SCSPLLSSCCTRL_sscg_depth_START (6)
#define SOC_SCTRL_SCSPLLSSCCTRL_sscg_depth_END (8)
#define SOC_SCTRL_SCSPLLSSCCTRL_sscg_fast_dis_START (9)
#define SOC_SCTRL_SCSPLLSSCCTRL_sscg_fast_dis_END (9)
#define SOC_SCTRL_SCSPLLSSCCTRL_bitmasken_START (16)
#define SOC_SCTRL_SCSPLLSSCCTRL_bitmasken_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved : 32;
    } reg;
} SOC_SCTRL_SCSPLLCTRL0_LOCK_STAT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved : 32;
    } reg;
} SOC_SCTRL_SCSPLLCTRL1_LOCK_STAT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int scspllsscctrl_lock_stat : 16;
        unsigned int reserved : 16;
    } reg;
} SOC_SCTRL_SCSPLLSSCCTRL_LOCK_STAT_UNION;
#endif
#define SOC_SCTRL_SCSPLLSSCCTRL_LOCK_STAT_scspllsscctrl_lock_stat_START (0)
#define SOC_SCTRL_SCSPLLSSCCTRL_LOCK_STAT_scspllsscctrl_lock_stat_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int scclkdiv1_sec_lock_stat : 16;
        unsigned int spll_lock_bypass_stat : 1;
        unsigned int reserved_0 : 3;
        unsigned int spll_lock_bypass : 1;
        unsigned int reserved_1 : 11;
    } reg;
} SOC_SCTRL_SCSPLL_OTHER_LOCK_STAT_UNION;
#endif
#define SOC_SCTRL_SCSPLL_OTHER_LOCK_STAT_scclkdiv1_sec_lock_stat_START (0)
#define SOC_SCTRL_SCSPLL_OTHER_LOCK_STAT_scclkdiv1_sec_lock_stat_END (15)
#define SOC_SCTRL_SCSPLL_OTHER_LOCK_STAT_spll_lock_bypass_stat_START (16)
#define SOC_SCTRL_SCSPLL_OTHER_LOCK_STAT_spll_lock_bypass_stat_END (16)
#define SOC_SCTRL_SCSPLL_OTHER_LOCK_STAT_spll_lock_bypass_START (20)
#define SOC_SCTRL_SCSPLL_OTHER_LOCK_STAT_spll_lock_bypass_END (20)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int sta_reg_sel : 1;
        unsigned int read_en : 1;
        unsigned int input_cfg_en : 1;
        unsigned int test_mode : 1;
        unsigned int unlock_clear : 1;
        unsigned int dll_en : 1;
        unsigned int pll_lock_ate_sel : 2;
        unsigned int test_data_sel : 4;
        unsigned int bias_ctrl : 2;
        unsigned int icp_ctrl : 2;
        unsigned int updn_sel : 1;
        unsigned int reserved : 2;
        unsigned int int_mask : 3;
        unsigned int pfd_div_ratio : 4;
        unsigned int freq_threshold : 6;
    } reg;
} SOC_SCTRL_FNPLL_CFG0_UNION;
#endif
#define SOC_SCTRL_FNPLL_CFG0_sta_reg_sel_START (0)
#define SOC_SCTRL_FNPLL_CFG0_sta_reg_sel_END (0)
#define SOC_SCTRL_FNPLL_CFG0_read_en_START (1)
#define SOC_SCTRL_FNPLL_CFG0_read_en_END (1)
#define SOC_SCTRL_FNPLL_CFG0_input_cfg_en_START (2)
#define SOC_SCTRL_FNPLL_CFG0_input_cfg_en_END (2)
#define SOC_SCTRL_FNPLL_CFG0_test_mode_START (3)
#define SOC_SCTRL_FNPLL_CFG0_test_mode_END (3)
#define SOC_SCTRL_FNPLL_CFG0_unlock_clear_START (4)
#define SOC_SCTRL_FNPLL_CFG0_unlock_clear_END (4)
#define SOC_SCTRL_FNPLL_CFG0_dll_en_START (5)
#define SOC_SCTRL_FNPLL_CFG0_dll_en_END (5)
#define SOC_SCTRL_FNPLL_CFG0_pll_lock_ate_sel_START (6)
#define SOC_SCTRL_FNPLL_CFG0_pll_lock_ate_sel_END (7)
#define SOC_SCTRL_FNPLL_CFG0_test_data_sel_START (8)
#define SOC_SCTRL_FNPLL_CFG0_test_data_sel_END (11)
#define SOC_SCTRL_FNPLL_CFG0_bias_ctrl_START (12)
#define SOC_SCTRL_FNPLL_CFG0_bias_ctrl_END (13)
#define SOC_SCTRL_FNPLL_CFG0_icp_ctrl_START (14)
#define SOC_SCTRL_FNPLL_CFG0_icp_ctrl_END (15)
#define SOC_SCTRL_FNPLL_CFG0_updn_sel_START (16)
#define SOC_SCTRL_FNPLL_CFG0_updn_sel_END (16)
#define SOC_SCTRL_FNPLL_CFG0_int_mask_START (19)
#define SOC_SCTRL_FNPLL_CFG0_int_mask_END (21)
#define SOC_SCTRL_FNPLL_CFG0_pfd_div_ratio_START (22)
#define SOC_SCTRL_FNPLL_CFG0_pfd_div_ratio_END (25)
#define SOC_SCTRL_FNPLL_CFG0_freq_threshold_START (26)
#define SOC_SCTRL_FNPLL_CFG0_freq_threshold_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ctinue_lock_num : 6;
        unsigned int lock_thr : 2;
        unsigned int dc_ac_clk_en : 1;
        unsigned int dtc_ctrl_inv : 1;
        unsigned int reserved : 6;
        unsigned int ana_cfg_fnpll : 16;
    } reg;
} SOC_SCTRL_FNPLL_CFG1_UNION;
#endif
#define SOC_SCTRL_FNPLL_CFG1_ctinue_lock_num_START (0)
#define SOC_SCTRL_FNPLL_CFG1_ctinue_lock_num_END (5)
#define SOC_SCTRL_FNPLL_CFG1_lock_thr_START (6)
#define SOC_SCTRL_FNPLL_CFG1_lock_thr_END (7)
#define SOC_SCTRL_FNPLL_CFG1_dc_ac_clk_en_START (8)
#define SOC_SCTRL_FNPLL_CFG1_dc_ac_clk_en_END (8)
#define SOC_SCTRL_FNPLL_CFG1_dtc_ctrl_inv_START (9)
#define SOC_SCTRL_FNPLL_CFG1_dtc_ctrl_inv_END (9)
#define SOC_SCTRL_FNPLL_CFG1_ana_cfg_fnpll_START (16)
#define SOC_SCTRL_FNPLL_CFG1_ana_cfg_fnpll_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int div2_pd : 1;
        unsigned int fbdiv_rst_n : 1;
        unsigned int refdiv_rst_n : 1;
        unsigned int dll_force_en : 1;
        unsigned int phe_code_a : 2;
        unsigned int phe_code_b : 2;
        unsigned int bbpd_calib_byp : 1;
        unsigned int k_gain_cfg_en : 1;
        unsigned int k_gain_cfg : 6;
        unsigned int k_gain_av_thr : 3;
        unsigned int dtc_test : 1;
        unsigned int dtc_m_cfg : 6;
        unsigned int dtc_o_cfg : 6;
    } reg;
} SOC_SCTRL_FNPLL_CFG2_UNION;
#endif
#define SOC_SCTRL_FNPLL_CFG2_div2_pd_START (0)
#define SOC_SCTRL_FNPLL_CFG2_div2_pd_END (0)
#define SOC_SCTRL_FNPLL_CFG2_fbdiv_rst_n_START (1)
#define SOC_SCTRL_FNPLL_CFG2_fbdiv_rst_n_END (1)
#define SOC_SCTRL_FNPLL_CFG2_refdiv_rst_n_START (2)
#define SOC_SCTRL_FNPLL_CFG2_refdiv_rst_n_END (2)
#define SOC_SCTRL_FNPLL_CFG2_dll_force_en_START (3)
#define SOC_SCTRL_FNPLL_CFG2_dll_force_en_END (3)
#define SOC_SCTRL_FNPLL_CFG2_phe_code_a_START (4)
#define SOC_SCTRL_FNPLL_CFG2_phe_code_a_END (5)
#define SOC_SCTRL_FNPLL_CFG2_phe_code_b_START (6)
#define SOC_SCTRL_FNPLL_CFG2_phe_code_b_END (7)
#define SOC_SCTRL_FNPLL_CFG2_bbpd_calib_byp_START (8)
#define SOC_SCTRL_FNPLL_CFG2_bbpd_calib_byp_END (8)
#define SOC_SCTRL_FNPLL_CFG2_k_gain_cfg_en_START (9)
#define SOC_SCTRL_FNPLL_CFG2_k_gain_cfg_en_END (9)
#define SOC_SCTRL_FNPLL_CFG2_k_gain_cfg_START (10)
#define SOC_SCTRL_FNPLL_CFG2_k_gain_cfg_END (15)
#define SOC_SCTRL_FNPLL_CFG2_k_gain_av_thr_START (16)
#define SOC_SCTRL_FNPLL_CFG2_k_gain_av_thr_END (18)
#define SOC_SCTRL_FNPLL_CFG2_dtc_test_START (19)
#define SOC_SCTRL_FNPLL_CFG2_dtc_test_END (19)
#define SOC_SCTRL_FNPLL_CFG2_dtc_m_cfg_START (20)
#define SOC_SCTRL_FNPLL_CFG2_dtc_m_cfg_END (25)
#define SOC_SCTRL_FNPLL_CFG2_dtc_o_cfg_START (26)
#define SOC_SCTRL_FNPLL_CFG2_dtc_o_cfg_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int scppll_cfg3 : 32;
    } reg;
} SOC_SCTRL_FNPLL_CFG3_UNION;
#endif
#define SOC_SCTRL_FNPLL_CFG3_scppll_cfg3_START (0)
#define SOC_SCTRL_FNPLL_CFG3_scppll_cfg3_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0: 24;
        unsigned int reserved_1: 8;
    } reg;
} SOC_SCTRL_FNPLL_STATE0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ulppll_gt_cfg : 1;
        unsigned int reserved_0 : 3;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 27;
    } reg;
} SOC_SCTRL_SC_ULPPLL_GT_CFG_UNION;
#endif
#define SOC_SCTRL_SC_ULPPLL_GT_CFG_ulppll_gt_cfg_START (0)
#define SOC_SCTRL_SC_ULPPLL_GT_CFG_ulppll_gt_cfg_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ulppll_1_gt_cfg : 1;
        unsigned int reserved_0 : 3;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 27;
    } reg;
} SOC_SCTRL_SC_ULPPLL_1_GT_CFG_UNION;
#endif
#define SOC_SCTRL_SC_ULPPLL_1_GT_CFG_ulppll_1_gt_cfg_START (0)
#define SOC_SCTRL_SC_ULPPLL_1_GT_CFG_ulppll_1_gt_cfg_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int x2_en_cfg : 1;
        unsigned int reserved : 1;
        unsigned int ki_s : 4;
        unsigned int ki_e : 3;
        unsigned int kp_s : 3;
        unsigned int kp_e : 3;
        unsigned int ki_s_x2 : 4;
        unsigned int ki_e_x2 : 3;
        unsigned int kp_s_x2 : 3;
        unsigned int kp_e_x2 : 3;
        unsigned int kdco_ctrl_cfg_en : 1;
        unsigned int kdco_ctrl_cfg : 3;
    } reg;
} SOC_SCTRL_SC_ULPPLL_CFG0_UNION;
#endif
#define SOC_SCTRL_SC_ULPPLL_CFG0_x2_en_cfg_START (0)
#define SOC_SCTRL_SC_ULPPLL_CFG0_x2_en_cfg_END (0)
#define SOC_SCTRL_SC_ULPPLL_CFG0_ki_s_START (2)
#define SOC_SCTRL_SC_ULPPLL_CFG0_ki_s_END (5)
#define SOC_SCTRL_SC_ULPPLL_CFG0_ki_e_START (6)
#define SOC_SCTRL_SC_ULPPLL_CFG0_ki_e_END (8)
#define SOC_SCTRL_SC_ULPPLL_CFG0_kp_s_START (9)
#define SOC_SCTRL_SC_ULPPLL_CFG0_kp_s_END (11)
#define SOC_SCTRL_SC_ULPPLL_CFG0_kp_e_START (12)
#define SOC_SCTRL_SC_ULPPLL_CFG0_kp_e_END (14)
#define SOC_SCTRL_SC_ULPPLL_CFG0_ki_s_x2_START (15)
#define SOC_SCTRL_SC_ULPPLL_CFG0_ki_s_x2_END (18)
#define SOC_SCTRL_SC_ULPPLL_CFG0_ki_e_x2_START (19)
#define SOC_SCTRL_SC_ULPPLL_CFG0_ki_e_x2_END (21)
#define SOC_SCTRL_SC_ULPPLL_CFG0_kp_s_x2_START (22)
#define SOC_SCTRL_SC_ULPPLL_CFG0_kp_s_x2_END (24)
#define SOC_SCTRL_SC_ULPPLL_CFG0_kp_e_x2_START (25)
#define SOC_SCTRL_SC_ULPPLL_CFG0_kp_e_x2_END (27)
#define SOC_SCTRL_SC_ULPPLL_CFG0_kdco_ctrl_cfg_en_START (28)
#define SOC_SCTRL_SC_ULPPLL_CFG0_kdco_ctrl_cfg_en_END (28)
#define SOC_SCTRL_SC_ULPPLL_CFG0_kdco_ctrl_cfg_START (29)
#define SOC_SCTRL_SC_ULPPLL_CFG0_kdco_ctrl_cfg_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int cfg_vld : 1;
        unsigned int cfg_sel : 2;
        unsigned int rst_cfg_n : 1;
        unsigned int test_data_sel : 4;
        unsigned int read_en : 1;
        unsigned int test_ck_en : 1;
        unsigned int filter_rhm_sw_pll : 1;
        unsigned int swcap_en : 1;
        unsigned int delay_sel : 4;
        unsigned int sel_d2a_swcap : 2;
        unsigned int ulppll_ana_cfg : 5;
        unsigned int reserved : 9;
    } reg;
} SOC_SCTRL_SC_ULPPLL_CFG1_UNION;
#endif
#define SOC_SCTRL_SC_ULPPLL_CFG1_cfg_vld_START (0)
#define SOC_SCTRL_SC_ULPPLL_CFG1_cfg_vld_END (0)
#define SOC_SCTRL_SC_ULPPLL_CFG1_cfg_sel_START (1)
#define SOC_SCTRL_SC_ULPPLL_CFG1_cfg_sel_END (2)
#define SOC_SCTRL_SC_ULPPLL_CFG1_rst_cfg_n_START (3)
#define SOC_SCTRL_SC_ULPPLL_CFG1_rst_cfg_n_END (3)
#define SOC_SCTRL_SC_ULPPLL_CFG1_test_data_sel_START (4)
#define SOC_SCTRL_SC_ULPPLL_CFG1_test_data_sel_END (7)
#define SOC_SCTRL_SC_ULPPLL_CFG1_read_en_START (8)
#define SOC_SCTRL_SC_ULPPLL_CFG1_read_en_END (8)
#define SOC_SCTRL_SC_ULPPLL_CFG1_test_ck_en_START (9)
#define SOC_SCTRL_SC_ULPPLL_CFG1_test_ck_en_END (9)
#define SOC_SCTRL_SC_ULPPLL_CFG1_filter_rhm_sw_pll_START (10)
#define SOC_SCTRL_SC_ULPPLL_CFG1_filter_rhm_sw_pll_END (10)
#define SOC_SCTRL_SC_ULPPLL_CFG1_swcap_en_START (11)
#define SOC_SCTRL_SC_ULPPLL_CFG1_swcap_en_END (11)
#define SOC_SCTRL_SC_ULPPLL_CFG1_delay_sel_START (12)
#define SOC_SCTRL_SC_ULPPLL_CFG1_delay_sel_END (15)
#define SOC_SCTRL_SC_ULPPLL_CFG1_sel_d2a_swcap_START (16)
#define SOC_SCTRL_SC_ULPPLL_CFG1_sel_d2a_swcap_END (17)
#define SOC_SCTRL_SC_ULPPLL_CFG1_ulppll_ana_cfg_START (18)
#define SOC_SCTRL_SC_ULPPLL_CFG1_ulppll_ana_cfg_END (22)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved : 32;
    } reg;
} SOC_SCTRL_SC_ULPPLL_CFG2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ulppll_en : 1;
        unsigned int ulppll_bypass : 1;
        unsigned int reserved_0 : 2;
        unsigned int ulppll_postdiv : 3;
        unsigned int reserved_1 : 1;
        unsigned int ulppll_fbdiv : 15;
        unsigned int reserved_2 : 9;
    } reg;
} SOC_SCTRL_SC_ULPPLL_CTRL0_UNION;
#endif
#define SOC_SCTRL_SC_ULPPLL_CTRL0_ulppll_en_START (0)
#define SOC_SCTRL_SC_ULPPLL_CTRL0_ulppll_en_END (0)
#define SOC_SCTRL_SC_ULPPLL_CTRL0_ulppll_bypass_START (1)
#define SOC_SCTRL_SC_ULPPLL_CTRL0_ulppll_bypass_END (1)
#define SOC_SCTRL_SC_ULPPLL_CTRL0_ulppll_postdiv_START (4)
#define SOC_SCTRL_SC_ULPPLL_CTRL0_ulppll_postdiv_END (6)
#define SOC_SCTRL_SC_ULPPLL_CTRL0_ulppll_fbdiv_START (8)
#define SOC_SCTRL_SC_ULPPLL_CTRL0_ulppll_fbdiv_END (22)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved : 32;
    } reg;
} SOC_SCTRL_SC_ULPPLL_STATE0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int x2_en_cfg : 1;
        unsigned int reserved : 1;
        unsigned int ki_s : 4;
        unsigned int ki_e : 3;
        unsigned int kp_s : 3;
        unsigned int kp_e : 3;
        unsigned int ki_s_x2 : 4;
        unsigned int ki_e_x2 : 3;
        unsigned int kp_s_x2 : 3;
        unsigned int kp_e_x2 : 3;
        unsigned int kdco_ctrl_cfg_en : 1;
        unsigned int kdco_ctrl_cfg : 3;
    } reg;
} SOC_SCTRL_SC_ULPPLL_1_CFG0_UNION;
#endif
#define SOC_SCTRL_SC_ULPPLL_1_CFG0_x2_en_cfg_START (0)
#define SOC_SCTRL_SC_ULPPLL_1_CFG0_x2_en_cfg_END (0)
#define SOC_SCTRL_SC_ULPPLL_1_CFG0_ki_s_START (2)
#define SOC_SCTRL_SC_ULPPLL_1_CFG0_ki_s_END (5)
#define SOC_SCTRL_SC_ULPPLL_1_CFG0_ki_e_START (6)
#define SOC_SCTRL_SC_ULPPLL_1_CFG0_ki_e_END (8)
#define SOC_SCTRL_SC_ULPPLL_1_CFG0_kp_s_START (9)
#define SOC_SCTRL_SC_ULPPLL_1_CFG0_kp_s_END (11)
#define SOC_SCTRL_SC_ULPPLL_1_CFG0_kp_e_START (12)
#define SOC_SCTRL_SC_ULPPLL_1_CFG0_kp_e_END (14)
#define SOC_SCTRL_SC_ULPPLL_1_CFG0_ki_s_x2_START (15)
#define SOC_SCTRL_SC_ULPPLL_1_CFG0_ki_s_x2_END (18)
#define SOC_SCTRL_SC_ULPPLL_1_CFG0_ki_e_x2_START (19)
#define SOC_SCTRL_SC_ULPPLL_1_CFG0_ki_e_x2_END (21)
#define SOC_SCTRL_SC_ULPPLL_1_CFG0_kp_s_x2_START (22)
#define SOC_SCTRL_SC_ULPPLL_1_CFG0_kp_s_x2_END (24)
#define SOC_SCTRL_SC_ULPPLL_1_CFG0_kp_e_x2_START (25)
#define SOC_SCTRL_SC_ULPPLL_1_CFG0_kp_e_x2_END (27)
#define SOC_SCTRL_SC_ULPPLL_1_CFG0_kdco_ctrl_cfg_en_START (28)
#define SOC_SCTRL_SC_ULPPLL_1_CFG0_kdco_ctrl_cfg_en_END (28)
#define SOC_SCTRL_SC_ULPPLL_1_CFG0_kdco_ctrl_cfg_START (29)
#define SOC_SCTRL_SC_ULPPLL_1_CFG0_kdco_ctrl_cfg_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int cfg_vld : 1;
        unsigned int cfg_sel : 2;
        unsigned int rst_cfg_n : 1;
        unsigned int test_data_sel : 4;
        unsigned int read_en : 1;
        unsigned int test_ck_en : 1;
        unsigned int filter_rhm_sw_pll : 1;
        unsigned int swcap_en : 1;
        unsigned int delay_sel : 4;
        unsigned int sel_d2a_swcap : 2;
        unsigned int ulppll_ana_cfg : 5;
        unsigned int reserved : 9;
    } reg;
} SOC_SCTRL_SC_ULPPLL_1_CFG1_UNION;
#endif
#define SOC_SCTRL_SC_ULPPLL_1_CFG1_cfg_vld_START (0)
#define SOC_SCTRL_SC_ULPPLL_1_CFG1_cfg_vld_END (0)
#define SOC_SCTRL_SC_ULPPLL_1_CFG1_cfg_sel_START (1)
#define SOC_SCTRL_SC_ULPPLL_1_CFG1_cfg_sel_END (2)
#define SOC_SCTRL_SC_ULPPLL_1_CFG1_rst_cfg_n_START (3)
#define SOC_SCTRL_SC_ULPPLL_1_CFG1_rst_cfg_n_END (3)
#define SOC_SCTRL_SC_ULPPLL_1_CFG1_test_data_sel_START (4)
#define SOC_SCTRL_SC_ULPPLL_1_CFG1_test_data_sel_END (7)
#define SOC_SCTRL_SC_ULPPLL_1_CFG1_read_en_START (8)
#define SOC_SCTRL_SC_ULPPLL_1_CFG1_read_en_END (8)
#define SOC_SCTRL_SC_ULPPLL_1_CFG1_test_ck_en_START (9)
#define SOC_SCTRL_SC_ULPPLL_1_CFG1_test_ck_en_END (9)
#define SOC_SCTRL_SC_ULPPLL_1_CFG1_filter_rhm_sw_pll_START (10)
#define SOC_SCTRL_SC_ULPPLL_1_CFG1_filter_rhm_sw_pll_END (10)
#define SOC_SCTRL_SC_ULPPLL_1_CFG1_swcap_en_START (11)
#define SOC_SCTRL_SC_ULPPLL_1_CFG1_swcap_en_END (11)
#define SOC_SCTRL_SC_ULPPLL_1_CFG1_delay_sel_START (12)
#define SOC_SCTRL_SC_ULPPLL_1_CFG1_delay_sel_END (15)
#define SOC_SCTRL_SC_ULPPLL_1_CFG1_sel_d2a_swcap_START (16)
#define SOC_SCTRL_SC_ULPPLL_1_CFG1_sel_d2a_swcap_END (17)
#define SOC_SCTRL_SC_ULPPLL_1_CFG1_ulppll_ana_cfg_START (18)
#define SOC_SCTRL_SC_ULPPLL_1_CFG1_ulppll_ana_cfg_END (22)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved : 32;
    } reg;
} SOC_SCTRL_SC_ULPPLL_1_CFG2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ulppll_en : 1;
        unsigned int ulppll_bypass : 1;
        unsigned int reserved_0 : 2;
        unsigned int ulppll_postdiv : 3;
        unsigned int reserved_1 : 1;
        unsigned int ulppll_fbdiv : 15;
        unsigned int reserved_2 : 9;
    } reg;
} SOC_SCTRL_SC_ULPPLL_1_CTRL0_UNION;
#endif
#define SOC_SCTRL_SC_ULPPLL_1_CTRL0_ulppll_en_START (0)
#define SOC_SCTRL_SC_ULPPLL_1_CTRL0_ulppll_en_END (0)
#define SOC_SCTRL_SC_ULPPLL_1_CTRL0_ulppll_bypass_START (1)
#define SOC_SCTRL_SC_ULPPLL_1_CTRL0_ulppll_bypass_END (1)
#define SOC_SCTRL_SC_ULPPLL_1_CTRL0_ulppll_postdiv_START (4)
#define SOC_SCTRL_SC_ULPPLL_1_CTRL0_ulppll_postdiv_END (6)
#define SOC_SCTRL_SC_ULPPLL_1_CTRL0_ulppll_fbdiv_START (8)
#define SOC_SCTRL_SC_ULPPLL_1_CTRL0_ulppll_fbdiv_END (22)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved : 32;
    } reg;
} SOC_SCTRL_SC_ULPPLL_1_STATE0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int aupll_en : 1;
        unsigned int aupll_bp : 1;
        unsigned int aupll_refdiv : 6;
        unsigned int aupll_fbdiv : 12;
        unsigned int aupll_postdiv1 : 3;
        unsigned int aupll_postdiv2 : 3;
        unsigned int reserved : 6;
    } reg;
} SOC_SCTRL_SCAUPLLCTRL0_UNION;
#endif
#define SOC_SCTRL_SCAUPLLCTRL0_aupll_en_START (0)
#define SOC_SCTRL_SCAUPLLCTRL0_aupll_en_END (0)
#define SOC_SCTRL_SCAUPLLCTRL0_aupll_bp_START (1)
#define SOC_SCTRL_SCAUPLLCTRL0_aupll_bp_END (1)
#define SOC_SCTRL_SCAUPLLCTRL0_aupll_refdiv_START (2)
#define SOC_SCTRL_SCAUPLLCTRL0_aupll_refdiv_END (7)
#define SOC_SCTRL_SCAUPLLCTRL0_aupll_fbdiv_START (8)
#define SOC_SCTRL_SCAUPLLCTRL0_aupll_fbdiv_END (19)
#define SOC_SCTRL_SCAUPLLCTRL0_aupll_postdiv1_START (20)
#define SOC_SCTRL_SCAUPLLCTRL0_aupll_postdiv1_END (22)
#define SOC_SCTRL_SCAUPLLCTRL0_aupll_postdiv2_START (23)
#define SOC_SCTRL_SCAUPLLCTRL0_aupll_postdiv2_END (25)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int aupll_fracdiv : 24;
        unsigned int aupll_int_mod : 1;
        unsigned int aupll_cfg_vld : 1;
        unsigned int gt_clk_aupll : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 4;
    } reg;
} SOC_SCTRL_SCAUPLLCTRL1_UNION;
#endif
#define SOC_SCTRL_SCAUPLLCTRL1_aupll_fracdiv_START (0)
#define SOC_SCTRL_SCAUPLLCTRL1_aupll_fracdiv_END (23)
#define SOC_SCTRL_SCAUPLLCTRL1_aupll_int_mod_START (24)
#define SOC_SCTRL_SCAUPLLCTRL1_aupll_int_mod_END (24)
#define SOC_SCTRL_SCAUPLLCTRL1_aupll_cfg_vld_START (25)
#define SOC_SCTRL_SCAUPLLCTRL1_aupll_cfg_vld_END (25)
#define SOC_SCTRL_SCAUPLLCTRL1_gt_clk_aupll_START (26)
#define SOC_SCTRL_SCAUPLLCTRL1_gt_clk_aupll_END (26)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int sscg_enable : 1;
        unsigned int sscg_mode : 1;
        unsigned int sscg_rate : 4;
        unsigned int sscg_depth : 3;
        unsigned int sscg_fast_dis : 1;
        unsigned int reserved : 6;
        unsigned int bitmasken : 16;
    } reg;
} SOC_SCTRL_SCAUPLLSSCCTRL_UNION;
#endif
#define SOC_SCTRL_SCAUPLLSSCCTRL_sscg_enable_START (0)
#define SOC_SCTRL_SCAUPLLSSCCTRL_sscg_enable_END (0)
#define SOC_SCTRL_SCAUPLLSSCCTRL_sscg_mode_START (1)
#define SOC_SCTRL_SCAUPLLSSCCTRL_sscg_mode_END (1)
#define SOC_SCTRL_SCAUPLLSSCCTRL_sscg_rate_START (2)
#define SOC_SCTRL_SCAUPLLSSCCTRL_sscg_rate_END (5)
#define SOC_SCTRL_SCAUPLLSSCCTRL_sscg_depth_START (6)
#define SOC_SCTRL_SCAUPLLSSCCTRL_sscg_depth_END (8)
#define SOC_SCTRL_SCAUPLLSSCCTRL_sscg_fast_dis_START (9)
#define SOC_SCTRL_SCAUPLLSSCCTRL_sscg_fast_dis_END (9)
#define SOC_SCTRL_SCAUPLLSSCCTRL_bitmasken_START (16)
#define SOC_SCTRL_SCAUPLLSSCCTRL_bitmasken_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int sta_reg_sel : 1;
        unsigned int read_en : 1;
        unsigned int input_cfg_en : 1;
        unsigned int test_mode : 1;
        unsigned int unlock_clear : 1;
        unsigned int dll_en : 1;
        unsigned int pll_lock_ate_sel : 2;
        unsigned int test_data_sel : 4;
        unsigned int bias_ctrl : 2;
        unsigned int icp_ctrl : 2;
        unsigned int updn_sel : 1;
        unsigned int reserved : 2;
        unsigned int int_mask : 3;
        unsigned int pfd_div_ratio : 4;
        unsigned int freq_threshold : 6;
    } reg;
} SOC_SCTRL_AUFNPLL_CFG0_UNION;
#endif
#define SOC_SCTRL_AUFNPLL_CFG0_sta_reg_sel_START (0)
#define SOC_SCTRL_AUFNPLL_CFG0_sta_reg_sel_END (0)
#define SOC_SCTRL_AUFNPLL_CFG0_read_en_START (1)
#define SOC_SCTRL_AUFNPLL_CFG0_read_en_END (1)
#define SOC_SCTRL_AUFNPLL_CFG0_input_cfg_en_START (2)
#define SOC_SCTRL_AUFNPLL_CFG0_input_cfg_en_END (2)
#define SOC_SCTRL_AUFNPLL_CFG0_test_mode_START (3)
#define SOC_SCTRL_AUFNPLL_CFG0_test_mode_END (3)
#define SOC_SCTRL_AUFNPLL_CFG0_unlock_clear_START (4)
#define SOC_SCTRL_AUFNPLL_CFG0_unlock_clear_END (4)
#define SOC_SCTRL_AUFNPLL_CFG0_dll_en_START (5)
#define SOC_SCTRL_AUFNPLL_CFG0_dll_en_END (5)
#define SOC_SCTRL_AUFNPLL_CFG0_pll_lock_ate_sel_START (6)
#define SOC_SCTRL_AUFNPLL_CFG0_pll_lock_ate_sel_END (7)
#define SOC_SCTRL_AUFNPLL_CFG0_test_data_sel_START (8)
#define SOC_SCTRL_AUFNPLL_CFG0_test_data_sel_END (11)
#define SOC_SCTRL_AUFNPLL_CFG0_bias_ctrl_START (12)
#define SOC_SCTRL_AUFNPLL_CFG0_bias_ctrl_END (13)
#define SOC_SCTRL_AUFNPLL_CFG0_icp_ctrl_START (14)
#define SOC_SCTRL_AUFNPLL_CFG0_icp_ctrl_END (15)
#define SOC_SCTRL_AUFNPLL_CFG0_updn_sel_START (16)
#define SOC_SCTRL_AUFNPLL_CFG0_updn_sel_END (16)
#define SOC_SCTRL_AUFNPLL_CFG0_int_mask_START (19)
#define SOC_SCTRL_AUFNPLL_CFG0_int_mask_END (21)
#define SOC_SCTRL_AUFNPLL_CFG0_pfd_div_ratio_START (22)
#define SOC_SCTRL_AUFNPLL_CFG0_pfd_div_ratio_END (25)
#define SOC_SCTRL_AUFNPLL_CFG0_freq_threshold_START (26)
#define SOC_SCTRL_AUFNPLL_CFG0_freq_threshold_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ctinue_lock_num : 6;
        unsigned int lock_thr : 2;
        unsigned int dc_ac_clk_en : 1;
        unsigned int dtc_ctrl_inv : 1;
        unsigned int reserved : 6;
        unsigned int ana_cfg_fnpll : 16;
    } reg;
} SOC_SCTRL_AUFNPLL_CFG1_UNION;
#endif
#define SOC_SCTRL_AUFNPLL_CFG1_ctinue_lock_num_START (0)
#define SOC_SCTRL_AUFNPLL_CFG1_ctinue_lock_num_END (5)
#define SOC_SCTRL_AUFNPLL_CFG1_lock_thr_START (6)
#define SOC_SCTRL_AUFNPLL_CFG1_lock_thr_END (7)
#define SOC_SCTRL_AUFNPLL_CFG1_dc_ac_clk_en_START (8)
#define SOC_SCTRL_AUFNPLL_CFG1_dc_ac_clk_en_END (8)
#define SOC_SCTRL_AUFNPLL_CFG1_dtc_ctrl_inv_START (9)
#define SOC_SCTRL_AUFNPLL_CFG1_dtc_ctrl_inv_END (9)
#define SOC_SCTRL_AUFNPLL_CFG1_ana_cfg_fnpll_START (16)
#define SOC_SCTRL_AUFNPLL_CFG1_ana_cfg_fnpll_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int div2_pd : 1;
        unsigned int fbdiv_rst_n : 1;
        unsigned int refdiv_rst_n : 1;
        unsigned int dll_force_en : 1;
        unsigned int phe_code_a : 2;
        unsigned int phe_code_b : 2;
        unsigned int bbpd_calib_byp : 1;
        unsigned int k_gain_cfg_en : 1;
        unsigned int k_gain_cfg : 6;
        unsigned int k_gain_av_thr : 3;
        unsigned int dtc_test : 1;
        unsigned int dtc_m_cfg : 6;
        unsigned int dtc_o_cfg : 6;
    } reg;
} SOC_SCTRL_AUFNPLL_CFG2_UNION;
#endif
#define SOC_SCTRL_AUFNPLL_CFG2_div2_pd_START (0)
#define SOC_SCTRL_AUFNPLL_CFG2_div2_pd_END (0)
#define SOC_SCTRL_AUFNPLL_CFG2_fbdiv_rst_n_START (1)
#define SOC_SCTRL_AUFNPLL_CFG2_fbdiv_rst_n_END (1)
#define SOC_SCTRL_AUFNPLL_CFG2_refdiv_rst_n_START (2)
#define SOC_SCTRL_AUFNPLL_CFG2_refdiv_rst_n_END (2)
#define SOC_SCTRL_AUFNPLL_CFG2_dll_force_en_START (3)
#define SOC_SCTRL_AUFNPLL_CFG2_dll_force_en_END (3)
#define SOC_SCTRL_AUFNPLL_CFG2_phe_code_a_START (4)
#define SOC_SCTRL_AUFNPLL_CFG2_phe_code_a_END (5)
#define SOC_SCTRL_AUFNPLL_CFG2_phe_code_b_START (6)
#define SOC_SCTRL_AUFNPLL_CFG2_phe_code_b_END (7)
#define SOC_SCTRL_AUFNPLL_CFG2_bbpd_calib_byp_START (8)
#define SOC_SCTRL_AUFNPLL_CFG2_bbpd_calib_byp_END (8)
#define SOC_SCTRL_AUFNPLL_CFG2_k_gain_cfg_en_START (9)
#define SOC_SCTRL_AUFNPLL_CFG2_k_gain_cfg_en_END (9)
#define SOC_SCTRL_AUFNPLL_CFG2_k_gain_cfg_START (10)
#define SOC_SCTRL_AUFNPLL_CFG2_k_gain_cfg_END (15)
#define SOC_SCTRL_AUFNPLL_CFG2_k_gain_av_thr_START (16)
#define SOC_SCTRL_AUFNPLL_CFG2_k_gain_av_thr_END (18)
#define SOC_SCTRL_AUFNPLL_CFG2_dtc_test_START (19)
#define SOC_SCTRL_AUFNPLL_CFG2_dtc_test_END (19)
#define SOC_SCTRL_AUFNPLL_CFG2_dtc_m_cfg_START (20)
#define SOC_SCTRL_AUFNPLL_CFG2_dtc_m_cfg_END (25)
#define SOC_SCTRL_AUFNPLL_CFG2_dtc_o_cfg_START (26)
#define SOC_SCTRL_AUFNPLL_CFG2_dtc_o_cfg_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int scppll_cfg3 : 32;
    } reg;
} SOC_SCTRL_AUFNPLL_CFG3_UNION;
#endif
#define SOC_SCTRL_AUFNPLL_CFG3_scppll_cfg3_START (0)
#define SOC_SCTRL_AUFNPLL_CFG3_scppll_cfg3_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0: 24;
        unsigned int reserved_1: 8;
    } reg;
} SOC_SCTRL_AUFNPLL_STATE0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0 : 1;
        unsigned int gt_pclk_rtc : 1;
        unsigned int reserved_1 : 1;
        unsigned int gt_pclk_timer0 : 1;
        unsigned int reserved_2 : 1;
        unsigned int gt_clk_mad_32k : 1;
        unsigned int gt_clk_fll_test_src : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int gt_pclk_ao_ioc : 1;
        unsigned int gt_pclk_blpwm2 : 1;
        unsigned int gt_clk_mad_acpu : 1;
        unsigned int gt_clk_jtag_auth : 1;
        unsigned int gt_pclk_syscnt : 1;
        unsigned int gt_clk_syscnt : 1;
        unsigned int reserved_11 : 1;
        unsigned int reserved_12 : 1;
        unsigned int reserved_13 : 1;
        unsigned int reserved_14 : 1;
        unsigned int reserved_15 : 1;
        unsigned int reserved_16 : 1;
        unsigned int gt_clk_asp_tcxo : 1;
        unsigned int gt_pclk_bbpdrx : 1;
        unsigned int reserved_17 : 1;
        unsigned int reserved_18 : 1;
        unsigned int reserved_19 : 1;
    } reg;
} SOC_SCTRL_SCPEREN0_UNION;
#endif
#define SOC_SCTRL_SCPEREN0_gt_pclk_rtc_START (1)
#define SOC_SCTRL_SCPEREN0_gt_pclk_rtc_END (1)
#define SOC_SCTRL_SCPEREN0_gt_pclk_timer0_START (3)
#define SOC_SCTRL_SCPEREN0_gt_pclk_timer0_END (3)
#define SOC_SCTRL_SCPEREN0_gt_clk_mad_32k_START (5)
#define SOC_SCTRL_SCPEREN0_gt_clk_mad_32k_END (5)
#define SOC_SCTRL_SCPEREN0_gt_clk_fll_test_src_START (6)
#define SOC_SCTRL_SCPEREN0_gt_clk_fll_test_src_END (6)
#define SOC_SCTRL_SCPEREN0_gt_pclk_ao_ioc_START (15)
#define SOC_SCTRL_SCPEREN0_gt_pclk_ao_ioc_END (15)
#define SOC_SCTRL_SCPEREN0_gt_pclk_blpwm2_START (16)
#define SOC_SCTRL_SCPEREN0_gt_pclk_blpwm2_END (16)
#define SOC_SCTRL_SCPEREN0_gt_clk_mad_acpu_START (17)
#define SOC_SCTRL_SCPEREN0_gt_clk_mad_acpu_END (17)
#define SOC_SCTRL_SCPEREN0_gt_clk_jtag_auth_START (18)
#define SOC_SCTRL_SCPEREN0_gt_clk_jtag_auth_END (18)
#define SOC_SCTRL_SCPEREN0_gt_pclk_syscnt_START (19)
#define SOC_SCTRL_SCPEREN0_gt_pclk_syscnt_END (19)
#define SOC_SCTRL_SCPEREN0_gt_clk_syscnt_START (20)
#define SOC_SCTRL_SCPEREN0_gt_clk_syscnt_END (20)
#define SOC_SCTRL_SCPEREN0_gt_clk_asp_tcxo_START (27)
#define SOC_SCTRL_SCPEREN0_gt_clk_asp_tcxo_END (27)
#define SOC_SCTRL_SCPEREN0_gt_pclk_bbpdrx_START (28)
#define SOC_SCTRL_SCPEREN0_gt_pclk_bbpdrx_END (28)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0 : 1;
        unsigned int gt_pclk_rtc : 1;
        unsigned int reserved_1 : 1;
        unsigned int gt_pclk_timer0 : 1;
        unsigned int reserved_2 : 1;
        unsigned int gt_clk_mad_32k : 1;
        unsigned int gt_clk_fll_test_src : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int gt_pclk_ao_ioc : 1;
        unsigned int gt_pclk_blpwm2 : 1;
        unsigned int gt_clk_mad_acpu : 1;
        unsigned int gt_clk_jtag_auth : 1;
        unsigned int gt_pclk_syscnt : 1;
        unsigned int gt_clk_syscnt : 1;
        unsigned int reserved_11 : 1;
        unsigned int reserved_12 : 1;
        unsigned int reserved_13 : 1;
        unsigned int reserved_14 : 1;
        unsigned int reserved_15 : 1;
        unsigned int reserved_16 : 1;
        unsigned int gt_clk_asp_tcxo : 1;
        unsigned int gt_pclk_bbpdrx : 1;
        unsigned int reserved_17 : 1;
        unsigned int reserved_18 : 1;
        unsigned int reserved_19 : 1;
    } reg;
} SOC_SCTRL_SCPERDIS0_UNION;
#endif
#define SOC_SCTRL_SCPERDIS0_gt_pclk_rtc_START (1)
#define SOC_SCTRL_SCPERDIS0_gt_pclk_rtc_END (1)
#define SOC_SCTRL_SCPERDIS0_gt_pclk_timer0_START (3)
#define SOC_SCTRL_SCPERDIS0_gt_pclk_timer0_END (3)
#define SOC_SCTRL_SCPERDIS0_gt_clk_mad_32k_START (5)
#define SOC_SCTRL_SCPERDIS0_gt_clk_mad_32k_END (5)
#define SOC_SCTRL_SCPERDIS0_gt_clk_fll_test_src_START (6)
#define SOC_SCTRL_SCPERDIS0_gt_clk_fll_test_src_END (6)
#define SOC_SCTRL_SCPERDIS0_gt_pclk_ao_ioc_START (15)
#define SOC_SCTRL_SCPERDIS0_gt_pclk_ao_ioc_END (15)
#define SOC_SCTRL_SCPERDIS0_gt_pclk_blpwm2_START (16)
#define SOC_SCTRL_SCPERDIS0_gt_pclk_blpwm2_END (16)
#define SOC_SCTRL_SCPERDIS0_gt_clk_mad_acpu_START (17)
#define SOC_SCTRL_SCPERDIS0_gt_clk_mad_acpu_END (17)
#define SOC_SCTRL_SCPERDIS0_gt_clk_jtag_auth_START (18)
#define SOC_SCTRL_SCPERDIS0_gt_clk_jtag_auth_END (18)
#define SOC_SCTRL_SCPERDIS0_gt_pclk_syscnt_START (19)
#define SOC_SCTRL_SCPERDIS0_gt_pclk_syscnt_END (19)
#define SOC_SCTRL_SCPERDIS0_gt_clk_syscnt_START (20)
#define SOC_SCTRL_SCPERDIS0_gt_clk_syscnt_END (20)
#define SOC_SCTRL_SCPERDIS0_gt_clk_asp_tcxo_START (27)
#define SOC_SCTRL_SCPERDIS0_gt_clk_asp_tcxo_END (27)
#define SOC_SCTRL_SCPERDIS0_gt_pclk_bbpdrx_START (28)
#define SOC_SCTRL_SCPERDIS0_gt_pclk_bbpdrx_END (28)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0 : 1;
        unsigned int gt_pclk_rtc : 1;
        unsigned int reserved_1 : 1;
        unsigned int gt_pclk_timer0 : 1;
        unsigned int reserved_2 : 1;
        unsigned int gt_clk_mad_32k : 1;
        unsigned int gt_clk_fll_test_src : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int gt_pclk_ao_ioc : 1;
        unsigned int gt_pclk_blpwm2 : 1;
        unsigned int gt_clk_mad_acpu : 1;
        unsigned int gt_clk_jtag_auth : 1;
        unsigned int gt_pclk_syscnt : 1;
        unsigned int gt_clk_syscnt : 1;
        unsigned int reserved_11 : 1;
        unsigned int reserved_12 : 1;
        unsigned int reserved_13 : 1;
        unsigned int reserved_14 : 1;
        unsigned int reserved_15 : 1;
        unsigned int reserved_16 : 1;
        unsigned int gt_clk_asp_tcxo : 1;
        unsigned int gt_pclk_bbpdrx : 1;
        unsigned int reserved_17 : 1;
        unsigned int reserved_18 : 1;
        unsigned int reserved_19 : 1;
    } reg;
} SOC_SCTRL_SCPERCLKEN0_UNION;
#endif
#define SOC_SCTRL_SCPERCLKEN0_gt_pclk_rtc_START (1)
#define SOC_SCTRL_SCPERCLKEN0_gt_pclk_rtc_END (1)
#define SOC_SCTRL_SCPERCLKEN0_gt_pclk_timer0_START (3)
#define SOC_SCTRL_SCPERCLKEN0_gt_pclk_timer0_END (3)
#define SOC_SCTRL_SCPERCLKEN0_gt_clk_mad_32k_START (5)
#define SOC_SCTRL_SCPERCLKEN0_gt_clk_mad_32k_END (5)
#define SOC_SCTRL_SCPERCLKEN0_gt_clk_fll_test_src_START (6)
#define SOC_SCTRL_SCPERCLKEN0_gt_clk_fll_test_src_END (6)
#define SOC_SCTRL_SCPERCLKEN0_gt_pclk_ao_ioc_START (15)
#define SOC_SCTRL_SCPERCLKEN0_gt_pclk_ao_ioc_END (15)
#define SOC_SCTRL_SCPERCLKEN0_gt_pclk_blpwm2_START (16)
#define SOC_SCTRL_SCPERCLKEN0_gt_pclk_blpwm2_END (16)
#define SOC_SCTRL_SCPERCLKEN0_gt_clk_mad_acpu_START (17)
#define SOC_SCTRL_SCPERCLKEN0_gt_clk_mad_acpu_END (17)
#define SOC_SCTRL_SCPERCLKEN0_gt_clk_jtag_auth_START (18)
#define SOC_SCTRL_SCPERCLKEN0_gt_clk_jtag_auth_END (18)
#define SOC_SCTRL_SCPERCLKEN0_gt_pclk_syscnt_START (19)
#define SOC_SCTRL_SCPERCLKEN0_gt_pclk_syscnt_END (19)
#define SOC_SCTRL_SCPERCLKEN0_gt_clk_syscnt_START (20)
#define SOC_SCTRL_SCPERCLKEN0_gt_clk_syscnt_END (20)
#define SOC_SCTRL_SCPERCLKEN0_gt_clk_asp_tcxo_START (27)
#define SOC_SCTRL_SCPERCLKEN0_gt_clk_asp_tcxo_END (27)
#define SOC_SCTRL_SCPERCLKEN0_gt_pclk_bbpdrx_START (28)
#define SOC_SCTRL_SCPERCLKEN0_gt_pclk_bbpdrx_END (28)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int st_clk_ref_crc : 1;
        unsigned int st_pclk_rtc : 1;
        unsigned int st_pclk_rtc1 : 1;
        unsigned int st_pclk_timer0 : 1;
        unsigned int reserved_0 : 1;
        unsigned int st_pclk_timer1 : 1;
        unsigned int reserved_1 : 1;
        unsigned int st_pclk_timer2 : 1;
        unsigned int reserved_2 : 1;
        unsigned int st_pclk_timer3 : 1;
        unsigned int reserved_3 : 1;
        unsigned int aupll_sscg_idle : 1;
        unsigned int spll_sscg_idle : 1;
        unsigned int st_clk_pcie0_aux : 1;
        unsigned int st_clk_pcie1_aux : 1;
        unsigned int st_pclk_ao_ioc : 1;
        unsigned int st_pclk_blpwm2 : 1;
        unsigned int st_clk_out0 : 1;
        unsigned int st_clk_jtag_auth : 1;
        unsigned int st_pclk_syscnt : 1;
        unsigned int st_clk_syscnt : 1;
        unsigned int st_clk_out2 : 1;
        unsigned int st_clk_hieps_ree : 1;
        unsigned int st_clk_sci0 : 1;
        unsigned int st_clk_sci1 : 1;
        unsigned int st_clk_mad : 1;
        unsigned int st_clk_asp_subsys : 1;
        unsigned int st_clk_asp_tcxo : 1;
        unsigned int st_pclk_bbpdrx : 1;
        unsigned int st_clk_qcr_aspbus : 1;
        unsigned int st_pclk_efusec : 1;
        unsigned int st_clk_spll_sscg : 1;
    } reg;
} SOC_SCTRL_SCPERSTAT0_UNION;
#endif
#define SOC_SCTRL_SCPERSTAT0_st_clk_ref_crc_START (0)
#define SOC_SCTRL_SCPERSTAT0_st_clk_ref_crc_END (0)
#define SOC_SCTRL_SCPERSTAT0_st_pclk_rtc_START (1)
#define SOC_SCTRL_SCPERSTAT0_st_pclk_rtc_END (1)
#define SOC_SCTRL_SCPERSTAT0_st_pclk_rtc1_START (2)
#define SOC_SCTRL_SCPERSTAT0_st_pclk_rtc1_END (2)
#define SOC_SCTRL_SCPERSTAT0_st_pclk_timer0_START (3)
#define SOC_SCTRL_SCPERSTAT0_st_pclk_timer0_END (3)
#define SOC_SCTRL_SCPERSTAT0_st_pclk_timer1_START (5)
#define SOC_SCTRL_SCPERSTAT0_st_pclk_timer1_END (5)
#define SOC_SCTRL_SCPERSTAT0_st_pclk_timer2_START (7)
#define SOC_SCTRL_SCPERSTAT0_st_pclk_timer2_END (7)
#define SOC_SCTRL_SCPERSTAT0_st_pclk_timer3_START (9)
#define SOC_SCTRL_SCPERSTAT0_st_pclk_timer3_END (9)
#define SOC_SCTRL_SCPERSTAT0_aupll_sscg_idle_START (11)
#define SOC_SCTRL_SCPERSTAT0_aupll_sscg_idle_END (11)
#define SOC_SCTRL_SCPERSTAT0_spll_sscg_idle_START (12)
#define SOC_SCTRL_SCPERSTAT0_spll_sscg_idle_END (12)
#define SOC_SCTRL_SCPERSTAT0_st_clk_pcie0_aux_START (13)
#define SOC_SCTRL_SCPERSTAT0_st_clk_pcie0_aux_END (13)
#define SOC_SCTRL_SCPERSTAT0_st_clk_pcie1_aux_START (14)
#define SOC_SCTRL_SCPERSTAT0_st_clk_pcie1_aux_END (14)
#define SOC_SCTRL_SCPERSTAT0_st_pclk_ao_ioc_START (15)
#define SOC_SCTRL_SCPERSTAT0_st_pclk_ao_ioc_END (15)
#define SOC_SCTRL_SCPERSTAT0_st_pclk_blpwm2_START (16)
#define SOC_SCTRL_SCPERSTAT0_st_pclk_blpwm2_END (16)
#define SOC_SCTRL_SCPERSTAT0_st_clk_out0_START (17)
#define SOC_SCTRL_SCPERSTAT0_st_clk_out0_END (17)
#define SOC_SCTRL_SCPERSTAT0_st_clk_jtag_auth_START (18)
#define SOC_SCTRL_SCPERSTAT0_st_clk_jtag_auth_END (18)
#define SOC_SCTRL_SCPERSTAT0_st_pclk_syscnt_START (19)
#define SOC_SCTRL_SCPERSTAT0_st_pclk_syscnt_END (19)
#define SOC_SCTRL_SCPERSTAT0_st_clk_syscnt_START (20)
#define SOC_SCTRL_SCPERSTAT0_st_clk_syscnt_END (20)
#define SOC_SCTRL_SCPERSTAT0_st_clk_out2_START (21)
#define SOC_SCTRL_SCPERSTAT0_st_clk_out2_END (21)
#define SOC_SCTRL_SCPERSTAT0_st_clk_hieps_ree_START (22)
#define SOC_SCTRL_SCPERSTAT0_st_clk_hieps_ree_END (22)
#define SOC_SCTRL_SCPERSTAT0_st_clk_sci0_START (23)
#define SOC_SCTRL_SCPERSTAT0_st_clk_sci0_END (23)
#define SOC_SCTRL_SCPERSTAT0_st_clk_sci1_START (24)
#define SOC_SCTRL_SCPERSTAT0_st_clk_sci1_END (24)
#define SOC_SCTRL_SCPERSTAT0_st_clk_mad_START (25)
#define SOC_SCTRL_SCPERSTAT0_st_clk_mad_END (25)
#define SOC_SCTRL_SCPERSTAT0_st_clk_asp_subsys_START (26)
#define SOC_SCTRL_SCPERSTAT0_st_clk_asp_subsys_END (26)
#define SOC_SCTRL_SCPERSTAT0_st_clk_asp_tcxo_START (27)
#define SOC_SCTRL_SCPERSTAT0_st_clk_asp_tcxo_END (27)
#define SOC_SCTRL_SCPERSTAT0_st_pclk_bbpdrx_START (28)
#define SOC_SCTRL_SCPERSTAT0_st_pclk_bbpdrx_END (28)
#define SOC_SCTRL_SCPERSTAT0_st_clk_qcr_aspbus_START (29)
#define SOC_SCTRL_SCPERSTAT0_st_clk_qcr_aspbus_END (29)
#define SOC_SCTRL_SCPERSTAT0_st_pclk_efusec_START (30)
#define SOC_SCTRL_SCPERSTAT0_st_pclk_efusec_END (30)
#define SOC_SCTRL_SCPERSTAT0_st_clk_spll_sscg_START (31)
#define SOC_SCTRL_SCPERSTAT0_st_clk_spll_sscg_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int gt_clk_asp_dw_axi : 1;
        unsigned int gt_clk_asp_x2h : 1;
        unsigned int gt_clk_asp_h2p : 1;
        unsigned int gt_clk_asp_cfg : 1;
        unsigned int gt_clk_asp_subsys_acpu : 1;
        unsigned int gt_clk_em_sync_top : 1;
        unsigned int gt_clk_ao_camera : 1;
        unsigned int ddr_loopback_en : 1;
        unsigned int gt_clk_hsdt0_pcie_aux : 1;
        unsigned int gt_clk_fll_src_tp : 1;
        unsigned int gt_pclk_timer4 : 1;
        unsigned int gt_clk_qcr_aspbus_acpu : 1;
        unsigned int gt_pclk_timer5 : 1;
        unsigned int reserved_0 : 1;
        unsigned int gt_pclk_timer6 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int gt_pclk_timer8 : 1;
        unsigned int gt_clk_pcie0_aux : 1;
        unsigned int gt_clk_pcie1_aux : 1;
        unsigned int ddr_reset_en : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int gt_clk_noc_aobus2aspbus : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int gt_clk_asp_codec_acpu : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int wait_ddr_selfreflash_done_bypass : 1;
    } reg;
} SOC_SCTRL_SCPEREN1_UNION;
#endif
#define SOC_SCTRL_SCPEREN1_gt_clk_asp_dw_axi_START (0)
#define SOC_SCTRL_SCPEREN1_gt_clk_asp_dw_axi_END (0)
#define SOC_SCTRL_SCPEREN1_gt_clk_asp_x2h_START (1)
#define SOC_SCTRL_SCPEREN1_gt_clk_asp_x2h_END (1)
#define SOC_SCTRL_SCPEREN1_gt_clk_asp_h2p_START (2)
#define SOC_SCTRL_SCPEREN1_gt_clk_asp_h2p_END (2)
#define SOC_SCTRL_SCPEREN1_gt_clk_asp_cfg_START (3)
#define SOC_SCTRL_SCPEREN1_gt_clk_asp_cfg_END (3)
#define SOC_SCTRL_SCPEREN1_gt_clk_asp_subsys_acpu_START (4)
#define SOC_SCTRL_SCPEREN1_gt_clk_asp_subsys_acpu_END (4)
#define SOC_SCTRL_SCPEREN1_gt_clk_em_sync_top_START (5)
#define SOC_SCTRL_SCPEREN1_gt_clk_em_sync_top_END (5)
#define SOC_SCTRL_SCPEREN1_gt_clk_ao_camera_START (6)
#define SOC_SCTRL_SCPEREN1_gt_clk_ao_camera_END (6)
#define SOC_SCTRL_SCPEREN1_ddr_loopback_en_START (7)
#define SOC_SCTRL_SCPEREN1_ddr_loopback_en_END (7)
#define SOC_SCTRL_SCPEREN1_gt_clk_hsdt0_pcie_aux_START (8)
#define SOC_SCTRL_SCPEREN1_gt_clk_hsdt0_pcie_aux_END (8)
#define SOC_SCTRL_SCPEREN1_gt_clk_fll_src_tp_START (9)
#define SOC_SCTRL_SCPEREN1_gt_clk_fll_src_tp_END (9)
#define SOC_SCTRL_SCPEREN1_gt_pclk_timer4_START (10)
#define SOC_SCTRL_SCPEREN1_gt_pclk_timer4_END (10)
#define SOC_SCTRL_SCPEREN1_gt_clk_qcr_aspbus_acpu_START (11)
#define SOC_SCTRL_SCPEREN1_gt_clk_qcr_aspbus_acpu_END (11)
#define SOC_SCTRL_SCPEREN1_gt_pclk_timer5_START (12)
#define SOC_SCTRL_SCPEREN1_gt_pclk_timer5_END (12)
#define SOC_SCTRL_SCPEREN1_gt_pclk_timer6_START (14)
#define SOC_SCTRL_SCPEREN1_gt_pclk_timer6_END (14)
#define SOC_SCTRL_SCPEREN1_gt_pclk_timer8_START (18)
#define SOC_SCTRL_SCPEREN1_gt_pclk_timer8_END (18)
#define SOC_SCTRL_SCPEREN1_gt_clk_pcie0_aux_START (19)
#define SOC_SCTRL_SCPEREN1_gt_clk_pcie0_aux_END (19)
#define SOC_SCTRL_SCPEREN1_gt_clk_pcie1_aux_START (20)
#define SOC_SCTRL_SCPEREN1_gt_clk_pcie1_aux_END (20)
#define SOC_SCTRL_SCPEREN1_ddr_reset_en_START (21)
#define SOC_SCTRL_SCPEREN1_ddr_reset_en_END (21)
#define SOC_SCTRL_SCPEREN1_gt_clk_noc_aobus2aspbus_START (24)
#define SOC_SCTRL_SCPEREN1_gt_clk_noc_aobus2aspbus_END (24)
#define SOC_SCTRL_SCPEREN1_gt_clk_asp_codec_acpu_START (27)
#define SOC_SCTRL_SCPEREN1_gt_clk_asp_codec_acpu_END (27)
#define SOC_SCTRL_SCPEREN1_wait_ddr_selfreflash_done_bypass_START (31)
#define SOC_SCTRL_SCPEREN1_wait_ddr_selfreflash_done_bypass_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int gt_clk_asp_dw_axi : 1;
        unsigned int gt_clk_asp_x2h : 1;
        unsigned int gt_clk_asp_h2p : 1;
        unsigned int gt_clk_asp_cfg : 1;
        unsigned int gt_clk_asp_subsys_acpu : 1;
        unsigned int gt_clk_em_sync_top : 1;
        unsigned int gt_clk_ao_camera : 1;
        unsigned int ddr_loopback_en : 1;
        unsigned int gt_clk_hsdt0_pcie_aux : 1;
        unsigned int gt_clk_fll_src_tp : 1;
        unsigned int gt_pclk_timer4 : 1;
        unsigned int gt_clk_qcr_aspbus_acpu : 1;
        unsigned int gt_pclk_timer5 : 1;
        unsigned int reserved_0 : 1;
        unsigned int gt_pclk_timer6 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int gt_pclk_timer8 : 1;
        unsigned int gt_clk_pcie0_aux : 1;
        unsigned int gt_clk_pcie1_aux : 1;
        unsigned int ddr_reset_en : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int gt_clk_noc_aobus2aspbus : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int gt_clk_asp_codec_acpu : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int wait_ddr_selfreflash_done_bypass : 1;
    } reg;
} SOC_SCTRL_SCPERDIS1_UNION;
#endif
#define SOC_SCTRL_SCPERDIS1_gt_clk_asp_dw_axi_START (0)
#define SOC_SCTRL_SCPERDIS1_gt_clk_asp_dw_axi_END (0)
#define SOC_SCTRL_SCPERDIS1_gt_clk_asp_x2h_START (1)
#define SOC_SCTRL_SCPERDIS1_gt_clk_asp_x2h_END (1)
#define SOC_SCTRL_SCPERDIS1_gt_clk_asp_h2p_START (2)
#define SOC_SCTRL_SCPERDIS1_gt_clk_asp_h2p_END (2)
#define SOC_SCTRL_SCPERDIS1_gt_clk_asp_cfg_START (3)
#define SOC_SCTRL_SCPERDIS1_gt_clk_asp_cfg_END (3)
#define SOC_SCTRL_SCPERDIS1_gt_clk_asp_subsys_acpu_START (4)
#define SOC_SCTRL_SCPERDIS1_gt_clk_asp_subsys_acpu_END (4)
#define SOC_SCTRL_SCPERDIS1_gt_clk_em_sync_top_START (5)
#define SOC_SCTRL_SCPERDIS1_gt_clk_em_sync_top_END (5)
#define SOC_SCTRL_SCPERDIS1_gt_clk_ao_camera_START (6)
#define SOC_SCTRL_SCPERDIS1_gt_clk_ao_camera_END (6)
#define SOC_SCTRL_SCPERDIS1_ddr_loopback_en_START (7)
#define SOC_SCTRL_SCPERDIS1_ddr_loopback_en_END (7)
#define SOC_SCTRL_SCPERDIS1_gt_clk_hsdt0_pcie_aux_START (8)
#define SOC_SCTRL_SCPERDIS1_gt_clk_hsdt0_pcie_aux_END (8)
#define SOC_SCTRL_SCPERDIS1_gt_clk_fll_src_tp_START (9)
#define SOC_SCTRL_SCPERDIS1_gt_clk_fll_src_tp_END (9)
#define SOC_SCTRL_SCPERDIS1_gt_pclk_timer4_START (10)
#define SOC_SCTRL_SCPERDIS1_gt_pclk_timer4_END (10)
#define SOC_SCTRL_SCPERDIS1_gt_clk_qcr_aspbus_acpu_START (11)
#define SOC_SCTRL_SCPERDIS1_gt_clk_qcr_aspbus_acpu_END (11)
#define SOC_SCTRL_SCPERDIS1_gt_pclk_timer5_START (12)
#define SOC_SCTRL_SCPERDIS1_gt_pclk_timer5_END (12)
#define SOC_SCTRL_SCPERDIS1_gt_pclk_timer6_START (14)
#define SOC_SCTRL_SCPERDIS1_gt_pclk_timer6_END (14)
#define SOC_SCTRL_SCPERDIS1_gt_pclk_timer8_START (18)
#define SOC_SCTRL_SCPERDIS1_gt_pclk_timer8_END (18)
#define SOC_SCTRL_SCPERDIS1_gt_clk_pcie0_aux_START (19)
#define SOC_SCTRL_SCPERDIS1_gt_clk_pcie0_aux_END (19)
#define SOC_SCTRL_SCPERDIS1_gt_clk_pcie1_aux_START (20)
#define SOC_SCTRL_SCPERDIS1_gt_clk_pcie1_aux_END (20)
#define SOC_SCTRL_SCPERDIS1_ddr_reset_en_START (21)
#define SOC_SCTRL_SCPERDIS1_ddr_reset_en_END (21)
#define SOC_SCTRL_SCPERDIS1_gt_clk_noc_aobus2aspbus_START (24)
#define SOC_SCTRL_SCPERDIS1_gt_clk_noc_aobus2aspbus_END (24)
#define SOC_SCTRL_SCPERDIS1_gt_clk_asp_codec_acpu_START (27)
#define SOC_SCTRL_SCPERDIS1_gt_clk_asp_codec_acpu_END (27)
#define SOC_SCTRL_SCPERDIS1_wait_ddr_selfreflash_done_bypass_START (31)
#define SOC_SCTRL_SCPERDIS1_wait_ddr_selfreflash_done_bypass_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int gt_clk_asp_dw_axi : 1;
        unsigned int gt_clk_asp_x2h : 1;
        unsigned int gt_clk_asp_h2p : 1;
        unsigned int gt_clk_asp_cfg : 1;
        unsigned int gt_clk_asp_subsys_acpu : 1;
        unsigned int gt_clk_em_sync_top : 1;
        unsigned int gt_clk_ao_camera : 1;
        unsigned int ddr_loopback_en : 1;
        unsigned int gt_clk_hsdt0_pcie_aux : 1;
        unsigned int gt_clk_fll_src_tp : 1;
        unsigned int gt_pclk_timer4 : 1;
        unsigned int gt_clk_qcr_aspbus_acpu : 1;
        unsigned int gt_pclk_timer5 : 1;
        unsigned int reserved_0 : 1;
        unsigned int gt_pclk_timer6 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int gt_pclk_timer8 : 1;
        unsigned int gt_clk_pcie0_aux : 1;
        unsigned int gt_clk_pcie1_aux : 1;
        unsigned int ddr_reset_en : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int gt_clk_noc_aobus2aspbus : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int gt_clk_asp_codec_acpu : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int wait_ddr_selfreflash_done_bypass : 1;
    } reg;
} SOC_SCTRL_SCPERCLKEN1_UNION;
#endif
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_asp_dw_axi_START (0)
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_asp_dw_axi_END (0)
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_asp_x2h_START (1)
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_asp_x2h_END (1)
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_asp_h2p_START (2)
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_asp_h2p_END (2)
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_asp_cfg_START (3)
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_asp_cfg_END (3)
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_asp_subsys_acpu_START (4)
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_asp_subsys_acpu_END (4)
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_em_sync_top_START (5)
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_em_sync_top_END (5)
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_ao_camera_START (6)
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_ao_camera_END (6)
#define SOC_SCTRL_SCPERCLKEN1_ddr_loopback_en_START (7)
#define SOC_SCTRL_SCPERCLKEN1_ddr_loopback_en_END (7)
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_hsdt0_pcie_aux_START (8)
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_hsdt0_pcie_aux_END (8)
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_fll_src_tp_START (9)
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_fll_src_tp_END (9)
#define SOC_SCTRL_SCPERCLKEN1_gt_pclk_timer4_START (10)
#define SOC_SCTRL_SCPERCLKEN1_gt_pclk_timer4_END (10)
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_qcr_aspbus_acpu_START (11)
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_qcr_aspbus_acpu_END (11)
#define SOC_SCTRL_SCPERCLKEN1_gt_pclk_timer5_START (12)
#define SOC_SCTRL_SCPERCLKEN1_gt_pclk_timer5_END (12)
#define SOC_SCTRL_SCPERCLKEN1_gt_pclk_timer6_START (14)
#define SOC_SCTRL_SCPERCLKEN1_gt_pclk_timer6_END (14)
#define SOC_SCTRL_SCPERCLKEN1_gt_pclk_timer8_START (18)
#define SOC_SCTRL_SCPERCLKEN1_gt_pclk_timer8_END (18)
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_pcie0_aux_START (19)
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_pcie0_aux_END (19)
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_pcie1_aux_START (20)
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_pcie1_aux_END (20)
#define SOC_SCTRL_SCPERCLKEN1_ddr_reset_en_START (21)
#define SOC_SCTRL_SCPERCLKEN1_ddr_reset_en_END (21)
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_noc_aobus2aspbus_START (24)
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_noc_aobus2aspbus_END (24)
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_asp_codec_acpu_START (27)
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_asp_codec_acpu_END (27)
#define SOC_SCTRL_SCPERCLKEN1_wait_ddr_selfreflash_done_bypass_START (31)
#define SOC_SCTRL_SCPERCLKEN1_wait_ddr_selfreflash_done_bypass_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int st_clk_asp_dw_axi : 1;
        unsigned int st_clk_asp_x2h : 1;
        unsigned int st_clk_asp_h2p : 1;
        unsigned int st_clk_asp_cfg : 1;
        unsigned int st_clk_mad_axi : 1;
        unsigned int st_clk_noc_aobus2sysbusddrc : 1;
        unsigned int st_clk_ao_camera : 1;
        unsigned int reserved_0 : 1;
        unsigned int st_clk_mad_32k : 1;
        unsigned int st_clk_fll_src_tp : 1;
        unsigned int st_pclk_timer4 : 1;
        unsigned int reserved_1 : 1;
        unsigned int st_pclk_timer5 : 1;
        unsigned int reserved_2 : 1;
        unsigned int st_pclk_timer6 : 1;
        unsigned int reserved_3 : 1;
        unsigned int st_pclk_timer7 : 1;
        unsigned int reserved_4 : 1;
        unsigned int st_pclk_timer8 : 1;
        unsigned int reserved_5 : 1;
        unsigned int st_timerclk_refh : 1;
        unsigned int st_clk_em_sync_top : 1;
        unsigned int swdone_clk_iomcu_fll_src_div : 1;
        unsigned int swdone_clk_aobus_div : 1;
        unsigned int st_clk_noc_aobus2aspbus : 1;
        unsigned int st_clk_noc_aobus2iomcubus : 1;
        unsigned int st_clk_hsdt_subsys : 1;
        unsigned int swdone_clk_asp_subsys_div : 1;
        unsigned int swdone_clk_sci_div : 1;
        unsigned int sw_ack_clk_aobus_sw : 3;
    } reg;
} SOC_SCTRL_SCPERSTAT1_UNION;
#endif
#define SOC_SCTRL_SCPERSTAT1_st_clk_asp_dw_axi_START (0)
#define SOC_SCTRL_SCPERSTAT1_st_clk_asp_dw_axi_END (0)
#define SOC_SCTRL_SCPERSTAT1_st_clk_asp_x2h_START (1)
#define SOC_SCTRL_SCPERSTAT1_st_clk_asp_x2h_END (1)
#define SOC_SCTRL_SCPERSTAT1_st_clk_asp_h2p_START (2)
#define SOC_SCTRL_SCPERSTAT1_st_clk_asp_h2p_END (2)
#define SOC_SCTRL_SCPERSTAT1_st_clk_asp_cfg_START (3)
#define SOC_SCTRL_SCPERSTAT1_st_clk_asp_cfg_END (3)
#define SOC_SCTRL_SCPERSTAT1_st_clk_mad_axi_START (4)
#define SOC_SCTRL_SCPERSTAT1_st_clk_mad_axi_END (4)
#define SOC_SCTRL_SCPERSTAT1_st_clk_noc_aobus2sysbusddrc_START (5)
#define SOC_SCTRL_SCPERSTAT1_st_clk_noc_aobus2sysbusddrc_END (5)
#define SOC_SCTRL_SCPERSTAT1_st_clk_ao_camera_START (6)
#define SOC_SCTRL_SCPERSTAT1_st_clk_ao_camera_END (6)
#define SOC_SCTRL_SCPERSTAT1_st_clk_mad_32k_START (8)
#define SOC_SCTRL_SCPERSTAT1_st_clk_mad_32k_END (8)
#define SOC_SCTRL_SCPERSTAT1_st_clk_fll_src_tp_START (9)
#define SOC_SCTRL_SCPERSTAT1_st_clk_fll_src_tp_END (9)
#define SOC_SCTRL_SCPERSTAT1_st_pclk_timer4_START (10)
#define SOC_SCTRL_SCPERSTAT1_st_pclk_timer4_END (10)
#define SOC_SCTRL_SCPERSTAT1_st_pclk_timer5_START (12)
#define SOC_SCTRL_SCPERSTAT1_st_pclk_timer5_END (12)
#define SOC_SCTRL_SCPERSTAT1_st_pclk_timer6_START (14)
#define SOC_SCTRL_SCPERSTAT1_st_pclk_timer6_END (14)
#define SOC_SCTRL_SCPERSTAT1_st_pclk_timer7_START (16)
#define SOC_SCTRL_SCPERSTAT1_st_pclk_timer7_END (16)
#define SOC_SCTRL_SCPERSTAT1_st_pclk_timer8_START (18)
#define SOC_SCTRL_SCPERSTAT1_st_pclk_timer8_END (18)
#define SOC_SCTRL_SCPERSTAT1_st_timerclk_refh_START (20)
#define SOC_SCTRL_SCPERSTAT1_st_timerclk_refh_END (20)
#define SOC_SCTRL_SCPERSTAT1_st_clk_em_sync_top_START (21)
#define SOC_SCTRL_SCPERSTAT1_st_clk_em_sync_top_END (21)
#define SOC_SCTRL_SCPERSTAT1_swdone_clk_iomcu_fll_src_div_START (22)
#define SOC_SCTRL_SCPERSTAT1_swdone_clk_iomcu_fll_src_div_END (22)
#define SOC_SCTRL_SCPERSTAT1_swdone_clk_aobus_div_START (23)
#define SOC_SCTRL_SCPERSTAT1_swdone_clk_aobus_div_END (23)
#define SOC_SCTRL_SCPERSTAT1_st_clk_noc_aobus2aspbus_START (24)
#define SOC_SCTRL_SCPERSTAT1_st_clk_noc_aobus2aspbus_END (24)
#define SOC_SCTRL_SCPERSTAT1_st_clk_noc_aobus2iomcubus_START (25)
#define SOC_SCTRL_SCPERSTAT1_st_clk_noc_aobus2iomcubus_END (25)
#define SOC_SCTRL_SCPERSTAT1_st_clk_hsdt_subsys_START (26)
#define SOC_SCTRL_SCPERSTAT1_st_clk_hsdt_subsys_END (26)
#define SOC_SCTRL_SCPERSTAT1_swdone_clk_asp_subsys_div_START (27)
#define SOC_SCTRL_SCPERSTAT1_swdone_clk_asp_subsys_div_END (27)
#define SOC_SCTRL_SCPERSTAT1_swdone_clk_sci_div_START (28)
#define SOC_SCTRL_SCPERSTAT1_swdone_clk_sci_div_END (28)
#define SOC_SCTRL_SCPERSTAT1_sw_ack_clk_aobus_sw_START (29)
#define SOC_SCTRL_SCPERSTAT1_sw_ack_clk_aobus_sw_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int reserved_11 : 1;
        unsigned int reserved_12 : 1;
        unsigned int reserved_13 : 1;
        unsigned int reserved_14 : 1;
        unsigned int reserved_15 : 1;
        unsigned int reserved_16 : 1;
        unsigned int reserved_17 : 1;
        unsigned int reserved_18 : 1;
        unsigned int reserved_19 : 1;
        unsigned int reserved_20 : 1;
        unsigned int reserved_21 : 1;
        unsigned int reserved_22 : 1;
        unsigned int reserved_23 : 1;
        unsigned int reserved_24 : 1;
        unsigned int reserved_25 : 1;
        unsigned int gt_clk_ao_tof : 1;
        unsigned int reserved_26 : 1;
        unsigned int reserved_27 : 1;
        unsigned int reserved_28 : 1;
        unsigned int reserved_29 : 1;
        unsigned int gt_clk_mad_axi : 1;
    } reg;
} SOC_SCTRL_SCPEREN2_UNION;
#endif
#define SOC_SCTRL_SCPEREN2_gt_clk_ao_tof_START (26)
#define SOC_SCTRL_SCPEREN2_gt_clk_ao_tof_END (26)
#define SOC_SCTRL_SCPEREN2_gt_clk_mad_axi_START (31)
#define SOC_SCTRL_SCPEREN2_gt_clk_mad_axi_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int reserved_11 : 1;
        unsigned int reserved_12 : 1;
        unsigned int reserved_13 : 1;
        unsigned int reserved_14 : 1;
        unsigned int reserved_15 : 1;
        unsigned int reserved_16 : 1;
        unsigned int reserved_17 : 1;
        unsigned int reserved_18 : 1;
        unsigned int reserved_19 : 1;
        unsigned int reserved_20 : 1;
        unsigned int reserved_21 : 1;
        unsigned int reserved_22 : 1;
        unsigned int reserved_23 : 1;
        unsigned int reserved_24 : 1;
        unsigned int reserved_25 : 1;
        unsigned int gt_clk_ao_tof : 1;
        unsigned int reserved_26 : 1;
        unsigned int reserved_27 : 1;
        unsigned int reserved_28 : 1;
        unsigned int reserved_29 : 1;
        unsigned int gt_clk_mad_axi : 1;
    } reg;
} SOC_SCTRL_SCPERDIS2_UNION;
#endif
#define SOC_SCTRL_SCPERDIS2_gt_clk_ao_tof_START (26)
#define SOC_SCTRL_SCPERDIS2_gt_clk_ao_tof_END (26)
#define SOC_SCTRL_SCPERDIS2_gt_clk_mad_axi_START (31)
#define SOC_SCTRL_SCPERDIS2_gt_clk_mad_axi_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int reserved_11 : 1;
        unsigned int reserved_12 : 1;
        unsigned int reserved_13 : 1;
        unsigned int reserved_14 : 1;
        unsigned int reserved_15 : 1;
        unsigned int reserved_16 : 1;
        unsigned int reserved_17 : 1;
        unsigned int reserved_18 : 1;
        unsigned int reserved_19 : 1;
        unsigned int reserved_20 : 1;
        unsigned int reserved_21 : 1;
        unsigned int reserved_22 : 1;
        unsigned int reserved_23 : 1;
        unsigned int reserved_24 : 1;
        unsigned int reserved_25 : 1;
        unsigned int gt_clk_ao_tof : 1;
        unsigned int reserved_26 : 1;
        unsigned int reserved_27 : 1;
        unsigned int reserved_28 : 1;
        unsigned int reserved_29 : 1;
        unsigned int gt_clk_mad_axi : 1;
    } reg;
} SOC_SCTRL_SCPERCLKEN2_UNION;
#endif
#define SOC_SCTRL_SCPERCLKEN2_gt_clk_ao_tof_START (26)
#define SOC_SCTRL_SCPERCLKEN2_gt_clk_ao_tof_END (26)
#define SOC_SCTRL_SCPERCLKEN2_gt_clk_mad_axi_START (31)
#define SOC_SCTRL_SCPERCLKEN2_gt_clk_mad_axi_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int sw_ack_clk_hieps_ree_sw : 2;
        unsigned int swdone_clk_aonoc_pll_div : 1;
        unsigned int swdone_clk_iomcu_pll_div : 1;
        unsigned int sw_ack_clk_aonoc_sw : 4;
        unsigned int swdone_clk_spi3_ulppll : 1;
        unsigned int swdone_clk_spi3 : 1;
        unsigned int sw_ack_clk_asp_subsys_ini : 4;
        unsigned int reset_dram_n : 1;
        unsigned int swdone_clk_spi5_ulppll : 1;
        unsigned int swdone_clk_spi5 : 1;
        unsigned int swdone_clk_pcie_aux_32kpll_div : 1;
        unsigned int st_clk_hsdt0_pcie_aux : 1;
        unsigned int swdone_clk_dp_audio_pll_ao_div : 1;
        unsigned int st_clk_iomcu_fll_src : 1;
        unsigned int st_pclk_ao_wd : 1;
        unsigned int st_clk_iomcu_pll_div : 1;
        unsigned int reserved : 1;
        unsigned int swdone_clk_mad_fll_div : 1;
        unsigned int st_clk_fll_test_src : 1;
        unsigned int st_pclkdbg_to_iomcu : 1;
        unsigned int st_clk_asp_codec : 1;
        unsigned int swdone_clk_out0_div : 1;
        unsigned int swdone_clk_asp_subsys_fll_div : 1;
        unsigned int swdone_clk_aonoc_fll_div : 1;
        unsigned int swdone_clk_noc_timeout_extref_div : 1;
    } reg;
} SOC_SCTRL_SCPERSTAT2_UNION;
#endif
#define SOC_SCTRL_SCPERSTAT2_sw_ack_clk_hieps_ree_sw_START (0)
#define SOC_SCTRL_SCPERSTAT2_sw_ack_clk_hieps_ree_sw_END (1)
#define SOC_SCTRL_SCPERSTAT2_swdone_clk_aonoc_pll_div_START (2)
#define SOC_SCTRL_SCPERSTAT2_swdone_clk_aonoc_pll_div_END (2)
#define SOC_SCTRL_SCPERSTAT2_swdone_clk_iomcu_pll_div_START (3)
#define SOC_SCTRL_SCPERSTAT2_swdone_clk_iomcu_pll_div_END (3)
#define SOC_SCTRL_SCPERSTAT2_sw_ack_clk_aonoc_sw_START (4)
#define SOC_SCTRL_SCPERSTAT2_sw_ack_clk_aonoc_sw_END (7)
#define SOC_SCTRL_SCPERSTAT2_swdone_clk_spi3_ulppll_START (8)
#define SOC_SCTRL_SCPERSTAT2_swdone_clk_spi3_ulppll_END (8)
#define SOC_SCTRL_SCPERSTAT2_swdone_clk_spi3_START (9)
#define SOC_SCTRL_SCPERSTAT2_swdone_clk_spi3_END (9)
#define SOC_SCTRL_SCPERSTAT2_sw_ack_clk_asp_subsys_ini_START (10)
#define SOC_SCTRL_SCPERSTAT2_sw_ack_clk_asp_subsys_ini_END (13)
#define SOC_SCTRL_SCPERSTAT2_reset_dram_n_START (14)
#define SOC_SCTRL_SCPERSTAT2_reset_dram_n_END (14)
#define SOC_SCTRL_SCPERSTAT2_swdone_clk_spi5_ulppll_START (15)
#define SOC_SCTRL_SCPERSTAT2_swdone_clk_spi5_ulppll_END (15)
#define SOC_SCTRL_SCPERSTAT2_swdone_clk_spi5_START (16)
#define SOC_SCTRL_SCPERSTAT2_swdone_clk_spi5_END (16)
#define SOC_SCTRL_SCPERSTAT2_swdone_clk_pcie_aux_32kpll_div_START (17)
#define SOC_SCTRL_SCPERSTAT2_swdone_clk_pcie_aux_32kpll_div_END (17)
#define SOC_SCTRL_SCPERSTAT2_st_clk_hsdt0_pcie_aux_START (18)
#define SOC_SCTRL_SCPERSTAT2_st_clk_hsdt0_pcie_aux_END (18)
#define SOC_SCTRL_SCPERSTAT2_swdone_clk_dp_audio_pll_ao_div_START (19)
#define SOC_SCTRL_SCPERSTAT2_swdone_clk_dp_audio_pll_ao_div_END (19)
#define SOC_SCTRL_SCPERSTAT2_st_clk_iomcu_fll_src_START (20)
#define SOC_SCTRL_SCPERSTAT2_st_clk_iomcu_fll_src_END (20)
#define SOC_SCTRL_SCPERSTAT2_st_pclk_ao_wd_START (21)
#define SOC_SCTRL_SCPERSTAT2_st_pclk_ao_wd_END (21)
#define SOC_SCTRL_SCPERSTAT2_st_clk_iomcu_pll_div_START (22)
#define SOC_SCTRL_SCPERSTAT2_st_clk_iomcu_pll_div_END (22)
#define SOC_SCTRL_SCPERSTAT2_swdone_clk_mad_fll_div_START (24)
#define SOC_SCTRL_SCPERSTAT2_swdone_clk_mad_fll_div_END (24)
#define SOC_SCTRL_SCPERSTAT2_st_clk_fll_test_src_START (25)
#define SOC_SCTRL_SCPERSTAT2_st_clk_fll_test_src_END (25)
#define SOC_SCTRL_SCPERSTAT2_st_pclkdbg_to_iomcu_START (26)
#define SOC_SCTRL_SCPERSTAT2_st_pclkdbg_to_iomcu_END (26)
#define SOC_SCTRL_SCPERSTAT2_st_clk_asp_codec_START (27)
#define SOC_SCTRL_SCPERSTAT2_st_clk_asp_codec_END (27)
#define SOC_SCTRL_SCPERSTAT2_swdone_clk_out0_div_START (28)
#define SOC_SCTRL_SCPERSTAT2_swdone_clk_out0_div_END (28)
#define SOC_SCTRL_SCPERSTAT2_swdone_clk_asp_subsys_fll_div_START (29)
#define SOC_SCTRL_SCPERSTAT2_swdone_clk_asp_subsys_fll_div_END (29)
#define SOC_SCTRL_SCPERSTAT2_swdone_clk_aonoc_fll_div_START (30)
#define SOC_SCTRL_SCPERSTAT2_swdone_clk_aonoc_fll_div_END (30)
#define SOC_SCTRL_SCPERSTAT2_swdone_clk_noc_timeout_extref_div_START (31)
#define SOC_SCTRL_SCPERSTAT2_swdone_clk_noc_timeout_extref_div_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0: 1;
        unsigned int reserved_1: 1;
        unsigned int reserved_2: 1;
        unsigned int reserved_3: 1;
        unsigned int reserved_4: 1;
        unsigned int reserved_5: 1;
        unsigned int reserved_6: 1;
        unsigned int reserved_7: 1;
        unsigned int reserved_8: 1;
        unsigned int reserved_9: 1;
        unsigned int reserved_10: 1;
        unsigned int reserved_11: 1;
        unsigned int reserved_12: 1;
        unsigned int reserved_13: 1;
        unsigned int reserved_14: 1;
        unsigned int reserved_15: 1;
        unsigned int reserved_16: 1;
        unsigned int reserved_17: 1;
        unsigned int reserved_18: 1;
        unsigned int reserved_19: 1;
        unsigned int reserved_20: 1;
        unsigned int reserved_21: 1;
        unsigned int reserved_22: 1;
        unsigned int reserved_23: 1;
        unsigned int reserved_24: 1;
        unsigned int reserved_25: 1;
        unsigned int reserved_26: 1;
        unsigned int reserved_27: 1;
        unsigned int reserved_28: 1;
        unsigned int reserved_29: 1;
        unsigned int reserved_30: 1;
        unsigned int reserved_31: 1;
    } reg;
} SOC_SCTRL_SCPEREN3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0: 1;
        unsigned int reserved_1: 1;
        unsigned int reserved_2: 1;
        unsigned int reserved_3: 1;
        unsigned int reserved_4: 1;
        unsigned int reserved_5: 1;
        unsigned int reserved_6: 1;
        unsigned int reserved_7: 1;
        unsigned int reserved_8: 1;
        unsigned int reserved_9: 1;
        unsigned int reserved_10: 1;
        unsigned int reserved_11: 1;
        unsigned int reserved_12: 1;
        unsigned int reserved_13: 1;
        unsigned int reserved_14: 1;
        unsigned int reserved_15: 1;
        unsigned int reserved_16: 1;
        unsigned int reserved_17: 1;
        unsigned int reserved_18: 1;
        unsigned int reserved_19: 1;
        unsigned int reserved_20: 1;
        unsigned int reserved_21: 1;
        unsigned int reserved_22: 1;
        unsigned int reserved_23: 1;
        unsigned int reserved_24: 1;
        unsigned int reserved_25: 1;
        unsigned int reserved_26: 1;
        unsigned int reserved_27: 1;
        unsigned int reserved_28: 1;
        unsigned int reserved_29: 1;
        unsigned int reserved_30: 1;
        unsigned int reserved_31: 1;
    } reg;
} SOC_SCTRL_SCPERDIS3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0: 1;
        unsigned int reserved_1: 1;
        unsigned int reserved_2: 1;
        unsigned int reserved_3: 1;
        unsigned int reserved_4: 1;
        unsigned int reserved_5: 1;
        unsigned int reserved_6: 1;
        unsigned int reserved_7: 1;
        unsigned int reserved_8: 1;
        unsigned int reserved_9: 1;
        unsigned int reserved_10: 1;
        unsigned int reserved_11: 1;
        unsigned int reserved_12: 1;
        unsigned int reserved_13: 1;
        unsigned int reserved_14: 1;
        unsigned int reserved_15: 1;
        unsigned int reserved_16: 1;
        unsigned int reserved_17: 1;
        unsigned int reserved_18: 1;
        unsigned int reserved_19: 1;
        unsigned int reserved_20: 1;
        unsigned int reserved_21: 1;
        unsigned int reserved_22: 1;
        unsigned int reserved_23: 1;
        unsigned int reserved_24: 1;
        unsigned int reserved_25: 1;
        unsigned int reserved_26: 1;
        unsigned int reserved_27: 1;
        unsigned int reserved_28: 1;
        unsigned int reserved_29: 1;
        unsigned int reserved_30: 1;
        unsigned int reserved_31: 1;
    } reg;
} SOC_SCTRL_SCPERCLKEN3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0 : 1;
        unsigned int st_pclk_ao_gpio1_se : 1;
        unsigned int gt_aobus_noc_ini : 1;
        unsigned int autodiv_aonoc_pll_stat : 1;
        unsigned int autodiv_aonoc_fll_stat : 1;
        unsigned int st_pclk_timer17 : 1;
        unsigned int reserved_1 : 1;
        unsigned int st_pclk_timer16 : 1;
        unsigned int reserved_2 : 1;
        unsigned int st_pclk_timer15 : 1;
        unsigned int reserved_3 : 1;
        unsigned int st_pclk_timer14 : 1;
        unsigned int reserved_4 : 1;
        unsigned int st_pclk_timer13 : 1;
        unsigned int swdone_clk_out2_div : 1;
        unsigned int st_pclk_ao_ipc : 1;
        unsigned int st_clk_dp_audio_pll_tp : 1;
        unsigned int sw_ack_clk_pcie_aux : 2;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int st_atclk_to_iomcu : 1;
        unsigned int st_clk_noc_timeout_extref_peri : 1;
        unsigned int st_timerclk_refh_peri : 1;
        unsigned int st_clkin_sys_peri : 1;
        unsigned int st_clkin_ref_peri : 1;
        unsigned int st_clk_sys_ini_peri : 1;
        unsigned int swdone_clk_em_sync_top_div : 1;
        unsigned int st_clk_ref_peri : 1;
        unsigned int st_clk_sys_peri : 1;
        unsigned int st_clk_aobus_peri : 1;
        unsigned int st_clk_fll_src_peri : 1;
    } reg;
} SOC_SCTRL_SCPERSTAT3_UNION;
#endif
#define SOC_SCTRL_SCPERSTAT3_st_pclk_ao_gpio1_se_START (1)
#define SOC_SCTRL_SCPERSTAT3_st_pclk_ao_gpio1_se_END (1)
#define SOC_SCTRL_SCPERSTAT3_gt_aobus_noc_ini_START (2)
#define SOC_SCTRL_SCPERSTAT3_gt_aobus_noc_ini_END (2)
#define SOC_SCTRL_SCPERSTAT3_autodiv_aonoc_pll_stat_START (3)
#define SOC_SCTRL_SCPERSTAT3_autodiv_aonoc_pll_stat_END (3)
#define SOC_SCTRL_SCPERSTAT3_autodiv_aonoc_fll_stat_START (4)
#define SOC_SCTRL_SCPERSTAT3_autodiv_aonoc_fll_stat_END (4)
#define SOC_SCTRL_SCPERSTAT3_st_pclk_timer17_START (5)
#define SOC_SCTRL_SCPERSTAT3_st_pclk_timer17_END (5)
#define SOC_SCTRL_SCPERSTAT3_st_pclk_timer16_START (7)
#define SOC_SCTRL_SCPERSTAT3_st_pclk_timer16_END (7)
#define SOC_SCTRL_SCPERSTAT3_st_pclk_timer15_START (9)
#define SOC_SCTRL_SCPERSTAT3_st_pclk_timer15_END (9)
#define SOC_SCTRL_SCPERSTAT3_st_pclk_timer14_START (11)
#define SOC_SCTRL_SCPERSTAT3_st_pclk_timer14_END (11)
#define SOC_SCTRL_SCPERSTAT3_st_pclk_timer13_START (13)
#define SOC_SCTRL_SCPERSTAT3_st_pclk_timer13_END (13)
#define SOC_SCTRL_SCPERSTAT3_swdone_clk_out2_div_START (14)
#define SOC_SCTRL_SCPERSTAT3_swdone_clk_out2_div_END (14)
#define SOC_SCTRL_SCPERSTAT3_st_pclk_ao_ipc_START (15)
#define SOC_SCTRL_SCPERSTAT3_st_pclk_ao_ipc_END (15)
#define SOC_SCTRL_SCPERSTAT3_st_clk_dp_audio_pll_tp_START (16)
#define SOC_SCTRL_SCPERSTAT3_st_clk_dp_audio_pll_tp_END (16)
#define SOC_SCTRL_SCPERSTAT3_sw_ack_clk_pcie_aux_START (17)
#define SOC_SCTRL_SCPERSTAT3_sw_ack_clk_pcie_aux_END (18)
#define SOC_SCTRL_SCPERSTAT3_st_atclk_to_iomcu_START (21)
#define SOC_SCTRL_SCPERSTAT3_st_atclk_to_iomcu_END (21)
#define SOC_SCTRL_SCPERSTAT3_st_clk_noc_timeout_extref_peri_START (22)
#define SOC_SCTRL_SCPERSTAT3_st_clk_noc_timeout_extref_peri_END (22)
#define SOC_SCTRL_SCPERSTAT3_st_timerclk_refh_peri_START (23)
#define SOC_SCTRL_SCPERSTAT3_st_timerclk_refh_peri_END (23)
#define SOC_SCTRL_SCPERSTAT3_st_clkin_sys_peri_START (24)
#define SOC_SCTRL_SCPERSTAT3_st_clkin_sys_peri_END (24)
#define SOC_SCTRL_SCPERSTAT3_st_clkin_ref_peri_START (25)
#define SOC_SCTRL_SCPERSTAT3_st_clkin_ref_peri_END (25)
#define SOC_SCTRL_SCPERSTAT3_st_clk_sys_ini_peri_START (26)
#define SOC_SCTRL_SCPERSTAT3_st_clk_sys_ini_peri_END (26)
#define SOC_SCTRL_SCPERSTAT3_swdone_clk_em_sync_top_div_START (27)
#define SOC_SCTRL_SCPERSTAT3_swdone_clk_em_sync_top_div_END (27)
#define SOC_SCTRL_SCPERSTAT3_st_clk_ref_peri_START (28)
#define SOC_SCTRL_SCPERSTAT3_st_clk_ref_peri_END (28)
#define SOC_SCTRL_SCPERSTAT3_st_clk_sys_peri_START (29)
#define SOC_SCTRL_SCPERSTAT3_st_clk_sys_peri_END (29)
#define SOC_SCTRL_SCPERSTAT3_st_clk_aobus_peri_START (30)
#define SOC_SCTRL_SCPERSTAT3_st_clk_aobus_peri_END (30)
#define SOC_SCTRL_SCPERSTAT3_st_clk_fll_src_peri_START (31)
#define SOC_SCTRL_SCPERSTAT3_st_clk_fll_src_peri_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int gt_clk_memrepair_ao : 1;
        unsigned int gt_clk_memrepair_peri : 1;
        unsigned int reserved_0 : 1;
        unsigned int gt_pclk_ao_ipc_ns : 1;
        unsigned int gt_pclk_ao_loadmonitor : 1;
        unsigned int gt_clk_ao_loadmonitor : 1;
        unsigned int gt_clk_spi5 : 1;
        unsigned int gt_clk_dp_audio_pll : 1;
        unsigned int gt_clk_out2 : 1;
        unsigned int reserved_1 : 1;
        unsigned int gt_clk_spi3 : 1;
        unsigned int gt_clk_qic_spidma : 1;
        unsigned int reserved_2 : 1;
        unsigned int gt_clk_autodiv_ufs_subsys : 1;
        unsigned int gt_clk_ufs_subsys : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int gt_clk_ao_atb_brg : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int gt_clk_hieps_ree : 1;
        unsigned int reserved_10 : 1;
        unsigned int gt_clk_ufs_ref_test : 1;
        unsigned int gt_pclk_ao_loadmonitor_m3 : 1;
        unsigned int reserved_11 : 1;
        unsigned int reserved_12 : 1;
        unsigned int reserved_13 : 1;
        unsigned int reserved_14 : 1;
        unsigned int reserved_15 : 1;
    } reg;
} SOC_SCTRL_SCPEREN4_UNION;
#endif
#define SOC_SCTRL_SCPEREN4_gt_clk_memrepair_ao_START (0)
#define SOC_SCTRL_SCPEREN4_gt_clk_memrepair_ao_END (0)
#define SOC_SCTRL_SCPEREN4_gt_clk_memrepair_peri_START (1)
#define SOC_SCTRL_SCPEREN4_gt_clk_memrepair_peri_END (1)
#define SOC_SCTRL_SCPEREN4_gt_pclk_ao_ipc_ns_START (3)
#define SOC_SCTRL_SCPEREN4_gt_pclk_ao_ipc_ns_END (3)
#define SOC_SCTRL_SCPEREN4_gt_pclk_ao_loadmonitor_START (4)
#define SOC_SCTRL_SCPEREN4_gt_pclk_ao_loadmonitor_END (4)
#define SOC_SCTRL_SCPEREN4_gt_clk_ao_loadmonitor_START (5)
#define SOC_SCTRL_SCPEREN4_gt_clk_ao_loadmonitor_END (5)
#define SOC_SCTRL_SCPEREN4_gt_clk_spi5_START (6)
#define SOC_SCTRL_SCPEREN4_gt_clk_spi5_END (6)
#define SOC_SCTRL_SCPEREN4_gt_clk_dp_audio_pll_START (7)
#define SOC_SCTRL_SCPEREN4_gt_clk_dp_audio_pll_END (7)
#define SOC_SCTRL_SCPEREN4_gt_clk_out2_START (8)
#define SOC_SCTRL_SCPEREN4_gt_clk_out2_END (8)
#define SOC_SCTRL_SCPEREN4_gt_clk_spi3_START (10)
#define SOC_SCTRL_SCPEREN4_gt_clk_spi3_END (10)
#define SOC_SCTRL_SCPEREN4_gt_clk_qic_spidma_START (11)
#define SOC_SCTRL_SCPEREN4_gt_clk_qic_spidma_END (11)
#define SOC_SCTRL_SCPEREN4_gt_clk_autodiv_ufs_subsys_START (13)
#define SOC_SCTRL_SCPEREN4_gt_clk_autodiv_ufs_subsys_END (13)
#define SOC_SCTRL_SCPEREN4_gt_clk_ufs_subsys_START (14)
#define SOC_SCTRL_SCPEREN4_gt_clk_ufs_subsys_END (14)
#define SOC_SCTRL_SCPEREN4_gt_clk_ao_atb_brg_START (17)
#define SOC_SCTRL_SCPEREN4_gt_clk_ao_atb_brg_END (17)
#define SOC_SCTRL_SCPEREN4_gt_clk_hieps_ree_START (23)
#define SOC_SCTRL_SCPEREN4_gt_clk_hieps_ree_END (23)
#define SOC_SCTRL_SCPEREN4_gt_clk_ufs_ref_test_START (25)
#define SOC_SCTRL_SCPEREN4_gt_clk_ufs_ref_test_END (25)
#define SOC_SCTRL_SCPEREN4_gt_pclk_ao_loadmonitor_m3_START (26)
#define SOC_SCTRL_SCPEREN4_gt_pclk_ao_loadmonitor_m3_END (26)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int gt_clk_memrepair_ao : 1;
        unsigned int gt_clk_memrepair_peri : 1;
        unsigned int reserved_0 : 1;
        unsigned int gt_pclk_ao_ipc_ns : 1;
        unsigned int gt_pclk_ao_loadmonitor : 1;
        unsigned int gt_clk_ao_loadmonitor : 1;
        unsigned int gt_clk_spi5 : 1;
        unsigned int gt_clk_dp_audio_pll : 1;
        unsigned int gt_clk_out2 : 1;
        unsigned int reserved_1 : 1;
        unsigned int gt_clk_spi3 : 1;
        unsigned int gt_clk_qic_spidma : 1;
        unsigned int reserved_2 : 1;
        unsigned int gt_clk_autodiv_ufs_subsys : 1;
        unsigned int gt_clk_ufs_subsys : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int gt_clk_ao_atb_brg : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int gt_clk_hieps_ree : 1;
        unsigned int reserved_10 : 1;
        unsigned int gt_clk_ufs_ref_test : 1;
        unsigned int gt_pclk_ao_loadmonitor_m3 : 1;
        unsigned int reserved_11 : 1;
        unsigned int reserved_12 : 1;
        unsigned int reserved_13 : 1;
        unsigned int reserved_14 : 1;
        unsigned int reserved_15 : 1;
    } reg;
} SOC_SCTRL_SCPERDIS4_UNION;
#endif
#define SOC_SCTRL_SCPERDIS4_gt_clk_memrepair_ao_START (0)
#define SOC_SCTRL_SCPERDIS4_gt_clk_memrepair_ao_END (0)
#define SOC_SCTRL_SCPERDIS4_gt_clk_memrepair_peri_START (1)
#define SOC_SCTRL_SCPERDIS4_gt_clk_memrepair_peri_END (1)
#define SOC_SCTRL_SCPERDIS4_gt_pclk_ao_ipc_ns_START (3)
#define SOC_SCTRL_SCPERDIS4_gt_pclk_ao_ipc_ns_END (3)
#define SOC_SCTRL_SCPERDIS4_gt_pclk_ao_loadmonitor_START (4)
#define SOC_SCTRL_SCPERDIS4_gt_pclk_ao_loadmonitor_END (4)
#define SOC_SCTRL_SCPERDIS4_gt_clk_ao_loadmonitor_START (5)
#define SOC_SCTRL_SCPERDIS4_gt_clk_ao_loadmonitor_END (5)
#define SOC_SCTRL_SCPERDIS4_gt_clk_spi5_START (6)
#define SOC_SCTRL_SCPERDIS4_gt_clk_spi5_END (6)
#define SOC_SCTRL_SCPERDIS4_gt_clk_dp_audio_pll_START (7)
#define SOC_SCTRL_SCPERDIS4_gt_clk_dp_audio_pll_END (7)
#define SOC_SCTRL_SCPERDIS4_gt_clk_out2_START (8)
#define SOC_SCTRL_SCPERDIS4_gt_clk_out2_END (8)
#define SOC_SCTRL_SCPERDIS4_gt_clk_spi3_START (10)
#define SOC_SCTRL_SCPERDIS4_gt_clk_spi3_END (10)
#define SOC_SCTRL_SCPERDIS4_gt_clk_qic_spidma_START (11)
#define SOC_SCTRL_SCPERDIS4_gt_clk_qic_spidma_END (11)
#define SOC_SCTRL_SCPERDIS4_gt_clk_autodiv_ufs_subsys_START (13)
#define SOC_SCTRL_SCPERDIS4_gt_clk_autodiv_ufs_subsys_END (13)
#define SOC_SCTRL_SCPERDIS4_gt_clk_ufs_subsys_START (14)
#define SOC_SCTRL_SCPERDIS4_gt_clk_ufs_subsys_END (14)
#define SOC_SCTRL_SCPERDIS4_gt_clk_ao_atb_brg_START (17)
#define SOC_SCTRL_SCPERDIS4_gt_clk_ao_atb_brg_END (17)
#define SOC_SCTRL_SCPERDIS4_gt_clk_hieps_ree_START (23)
#define SOC_SCTRL_SCPERDIS4_gt_clk_hieps_ree_END (23)
#define SOC_SCTRL_SCPERDIS4_gt_clk_ufs_ref_test_START (25)
#define SOC_SCTRL_SCPERDIS4_gt_clk_ufs_ref_test_END (25)
#define SOC_SCTRL_SCPERDIS4_gt_pclk_ao_loadmonitor_m3_START (26)
#define SOC_SCTRL_SCPERDIS4_gt_pclk_ao_loadmonitor_m3_END (26)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int gt_clk_memrepair_ao : 1;
        unsigned int gt_clk_memrepair_peri : 1;
        unsigned int reserved_0 : 1;
        unsigned int gt_pclk_ao_ipc_ns : 1;
        unsigned int gt_pclk_ao_loadmonitor : 1;
        unsigned int gt_clk_ao_loadmonitor : 1;
        unsigned int gt_clk_spi5 : 1;
        unsigned int gt_clk_dp_audio_pll : 1;
        unsigned int gt_clk_out2 : 1;
        unsigned int reserved_1 : 1;
        unsigned int gt_clk_spi3 : 1;
        unsigned int gt_clk_qic_spidma : 1;
        unsigned int reserved_2 : 1;
        unsigned int gt_clk_autodiv_ufs_subsys : 1;
        unsigned int gt_clk_ufs_subsys : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int gt_clk_ao_atb_brg : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int gt_clk_hieps_ree : 1;
        unsigned int reserved_10 : 1;
        unsigned int gt_clk_ufs_ref_test : 1;
        unsigned int gt_pclk_ao_loadmonitor_m3 : 1;
        unsigned int reserved_11 : 1;
        unsigned int reserved_12 : 1;
        unsigned int reserved_13 : 1;
        unsigned int reserved_14 : 1;
        unsigned int reserved_15 : 1;
    } reg;
} SOC_SCTRL_SCPERCLKEN4_UNION;
#endif
#define SOC_SCTRL_SCPERCLKEN4_gt_clk_memrepair_ao_START (0)
#define SOC_SCTRL_SCPERCLKEN4_gt_clk_memrepair_ao_END (0)
#define SOC_SCTRL_SCPERCLKEN4_gt_clk_memrepair_peri_START (1)
#define SOC_SCTRL_SCPERCLKEN4_gt_clk_memrepair_peri_END (1)
#define SOC_SCTRL_SCPERCLKEN4_gt_pclk_ao_ipc_ns_START (3)
#define SOC_SCTRL_SCPERCLKEN4_gt_pclk_ao_ipc_ns_END (3)
#define SOC_SCTRL_SCPERCLKEN4_gt_pclk_ao_loadmonitor_START (4)
#define SOC_SCTRL_SCPERCLKEN4_gt_pclk_ao_loadmonitor_END (4)
#define SOC_SCTRL_SCPERCLKEN4_gt_clk_ao_loadmonitor_START (5)
#define SOC_SCTRL_SCPERCLKEN4_gt_clk_ao_loadmonitor_END (5)
#define SOC_SCTRL_SCPERCLKEN4_gt_clk_spi5_START (6)
#define SOC_SCTRL_SCPERCLKEN4_gt_clk_spi5_END (6)
#define SOC_SCTRL_SCPERCLKEN4_gt_clk_dp_audio_pll_START (7)
#define SOC_SCTRL_SCPERCLKEN4_gt_clk_dp_audio_pll_END (7)
#define SOC_SCTRL_SCPERCLKEN4_gt_clk_out2_START (8)
#define SOC_SCTRL_SCPERCLKEN4_gt_clk_out2_END (8)
#define SOC_SCTRL_SCPERCLKEN4_gt_clk_spi3_START (10)
#define SOC_SCTRL_SCPERCLKEN4_gt_clk_spi3_END (10)
#define SOC_SCTRL_SCPERCLKEN4_gt_clk_qic_spidma_START (11)
#define SOC_SCTRL_SCPERCLKEN4_gt_clk_qic_spidma_END (11)
#define SOC_SCTRL_SCPERCLKEN4_gt_clk_autodiv_ufs_subsys_START (13)
#define SOC_SCTRL_SCPERCLKEN4_gt_clk_autodiv_ufs_subsys_END (13)
#define SOC_SCTRL_SCPERCLKEN4_gt_clk_ufs_subsys_START (14)
#define SOC_SCTRL_SCPERCLKEN4_gt_clk_ufs_subsys_END (14)
#define SOC_SCTRL_SCPERCLKEN4_gt_clk_ao_atb_brg_START (17)
#define SOC_SCTRL_SCPERCLKEN4_gt_clk_ao_atb_brg_END (17)
#define SOC_SCTRL_SCPERCLKEN4_gt_clk_hieps_ree_START (23)
#define SOC_SCTRL_SCPERCLKEN4_gt_clk_hieps_ree_END (23)
#define SOC_SCTRL_SCPERCLKEN4_gt_clk_ufs_ref_test_START (25)
#define SOC_SCTRL_SCPERCLKEN4_gt_clk_ufs_ref_test_END (25)
#define SOC_SCTRL_SCPERCLKEN4_gt_pclk_ao_loadmonitor_m3_START (26)
#define SOC_SCTRL_SCPERCLKEN4_gt_pclk_ao_loadmonitor_m3_END (26)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int st_clk_memrepair_ao : 1;
        unsigned int st_clk_spmi_mst : 1;
        unsigned int st_pclk_ao_tzpc : 1;
        unsigned int st_pclk_ao_ipc_ns : 1;
        unsigned int st_pclk_ao_loadmonitor : 1;
        unsigned int st_clk_ao_loadmonitor : 1;
        unsigned int st_ulppll_1_out : 1;
        unsigned int st_clk_dp_audio_pll : 1;
        unsigned int reserved_0 : 1;
        unsigned int st_clk_qic_spidma : 1;
        unsigned int st_clk_spi3 : 1;
        unsigned int st_clk_spi5 : 1;
        unsigned int st_clk_mfcc : 1;
        unsigned int st_clk_autodiv_ufs_subsys : 1;
        unsigned int st_clk_ufs_subsys : 1;
        unsigned int sw_ack_clk_mad_sw : 2;
        unsigned int st_clk_ao_atb_brg : 1;
        unsigned int st_clk_pll_fsm : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int st_clk_memrepair_peri : 1;
        unsigned int st_pclk_intr_hub : 1;
        unsigned int reserved_3 : 1;
        unsigned int st_clk_aupll_sscg : 1;
        unsigned int st_clk_ufs_ref_test : 1;
        unsigned int reserved_4 : 1;
        unsigned int swdone_clk_timer_div : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int swdone_clk_ao_332m_div : 1;
    } reg;
} SOC_SCTRL_SCPERSTAT4_UNION;
#endif
#define SOC_SCTRL_SCPERSTAT4_st_clk_memrepair_ao_START (0)
#define SOC_SCTRL_SCPERSTAT4_st_clk_memrepair_ao_END (0)
#define SOC_SCTRL_SCPERSTAT4_st_clk_spmi_mst_START (1)
#define SOC_SCTRL_SCPERSTAT4_st_clk_spmi_mst_END (1)
#define SOC_SCTRL_SCPERSTAT4_st_pclk_ao_tzpc_START (2)
#define SOC_SCTRL_SCPERSTAT4_st_pclk_ao_tzpc_END (2)
#define SOC_SCTRL_SCPERSTAT4_st_pclk_ao_ipc_ns_START (3)
#define SOC_SCTRL_SCPERSTAT4_st_pclk_ao_ipc_ns_END (3)
#define SOC_SCTRL_SCPERSTAT4_st_pclk_ao_loadmonitor_START (4)
#define SOC_SCTRL_SCPERSTAT4_st_pclk_ao_loadmonitor_END (4)
#define SOC_SCTRL_SCPERSTAT4_st_clk_ao_loadmonitor_START (5)
#define SOC_SCTRL_SCPERSTAT4_st_clk_ao_loadmonitor_END (5)
#define SOC_SCTRL_SCPERSTAT4_st_ulppll_1_out_START (6)
#define SOC_SCTRL_SCPERSTAT4_st_ulppll_1_out_END (6)
#define SOC_SCTRL_SCPERSTAT4_st_clk_dp_audio_pll_START (7)
#define SOC_SCTRL_SCPERSTAT4_st_clk_dp_audio_pll_END (7)
#define SOC_SCTRL_SCPERSTAT4_st_clk_qic_spidma_START (9)
#define SOC_SCTRL_SCPERSTAT4_st_clk_qic_spidma_END (9)
#define SOC_SCTRL_SCPERSTAT4_st_clk_spi3_START (10)
#define SOC_SCTRL_SCPERSTAT4_st_clk_spi3_END (10)
#define SOC_SCTRL_SCPERSTAT4_st_clk_spi5_START (11)
#define SOC_SCTRL_SCPERSTAT4_st_clk_spi5_END (11)
#define SOC_SCTRL_SCPERSTAT4_st_clk_mfcc_START (12)
#define SOC_SCTRL_SCPERSTAT4_st_clk_mfcc_END (12)
#define SOC_SCTRL_SCPERSTAT4_st_clk_autodiv_ufs_subsys_START (13)
#define SOC_SCTRL_SCPERSTAT4_st_clk_autodiv_ufs_subsys_END (13)
#define SOC_SCTRL_SCPERSTAT4_st_clk_ufs_subsys_START (14)
#define SOC_SCTRL_SCPERSTAT4_st_clk_ufs_subsys_END (14)
#define SOC_SCTRL_SCPERSTAT4_sw_ack_clk_mad_sw_START (15)
#define SOC_SCTRL_SCPERSTAT4_sw_ack_clk_mad_sw_END (16)
#define SOC_SCTRL_SCPERSTAT4_st_clk_ao_atb_brg_START (17)
#define SOC_SCTRL_SCPERSTAT4_st_clk_ao_atb_brg_END (17)
#define SOC_SCTRL_SCPERSTAT4_st_clk_pll_fsm_START (18)
#define SOC_SCTRL_SCPERSTAT4_st_clk_pll_fsm_END (18)
#define SOC_SCTRL_SCPERSTAT4_st_clk_memrepair_peri_START (21)
#define SOC_SCTRL_SCPERSTAT4_st_clk_memrepair_peri_END (21)
#define SOC_SCTRL_SCPERSTAT4_st_pclk_intr_hub_START (22)
#define SOC_SCTRL_SCPERSTAT4_st_pclk_intr_hub_END (22)
#define SOC_SCTRL_SCPERSTAT4_st_clk_aupll_sscg_START (24)
#define SOC_SCTRL_SCPERSTAT4_st_clk_aupll_sscg_END (24)
#define SOC_SCTRL_SCPERSTAT4_st_clk_ufs_ref_test_START (25)
#define SOC_SCTRL_SCPERSTAT4_st_clk_ufs_ref_test_END (25)
#define SOC_SCTRL_SCPERSTAT4_swdone_clk_timer_div_START (27)
#define SOC_SCTRL_SCPERSTAT4_swdone_clk_timer_div_END (27)
#define SOC_SCTRL_SCPERSTAT4_swdone_clk_ao_332m_div_START (31)
#define SOC_SCTRL_SCPERSTAT4_swdone_clk_ao_332m_div_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int gt_pclk_timer17 : 1;
        unsigned int reserved_0 : 1;
        unsigned int gt_pclk_timer16 : 1;
        unsigned int reserved_1 : 1;
        unsigned int gt_pclk_timer15 : 1;
        unsigned int reserved_2 : 1;
        unsigned int gt_pclk_timer14 : 1;
        unsigned int reserved_3 : 1;
        unsigned int gt_pclk_timer13 : 1;
        unsigned int reserved_4 : 1;
        unsigned int gt_clk_i2c9 : 1;
        unsigned int gt_clk_i2c9_asp : 1;
        unsigned int gt_clk_i2c9_lpmcu : 1;
        unsigned int gt_clk_i2c9_mdm : 1;
        unsigned int gt_pclk_intr_hub : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int reserved_11 : 1;
        unsigned int reserved_12 : 1;
        unsigned int reserved_13 : 1;
        unsigned int reserved_14 : 1;
        unsigned int reserved_15 : 1;
        unsigned int reserved_16 : 1;
        unsigned int reserved_17 : 1;
        unsigned int reserved_18 : 1;
        unsigned int reserved_19 : 1;
        unsigned int reserved_20 : 1;
        unsigned int reserved_21 : 1;
    } reg;
} SOC_SCTRL_SCPEREN5_UNION;
#endif
#define SOC_SCTRL_SCPEREN5_gt_pclk_timer17_START (0)
#define SOC_SCTRL_SCPEREN5_gt_pclk_timer17_END (0)
#define SOC_SCTRL_SCPEREN5_gt_pclk_timer16_START (2)
#define SOC_SCTRL_SCPEREN5_gt_pclk_timer16_END (2)
#define SOC_SCTRL_SCPEREN5_gt_pclk_timer15_START (4)
#define SOC_SCTRL_SCPEREN5_gt_pclk_timer15_END (4)
#define SOC_SCTRL_SCPEREN5_gt_pclk_timer14_START (6)
#define SOC_SCTRL_SCPEREN5_gt_pclk_timer14_END (6)
#define SOC_SCTRL_SCPEREN5_gt_pclk_timer13_START (8)
#define SOC_SCTRL_SCPEREN5_gt_pclk_timer13_END (8)
#define SOC_SCTRL_SCPEREN5_gt_clk_i2c9_START (10)
#define SOC_SCTRL_SCPEREN5_gt_clk_i2c9_END (10)
#define SOC_SCTRL_SCPEREN5_gt_clk_i2c9_asp_START (11)
#define SOC_SCTRL_SCPEREN5_gt_clk_i2c9_asp_END (11)
#define SOC_SCTRL_SCPEREN5_gt_clk_i2c9_lpmcu_START (12)
#define SOC_SCTRL_SCPEREN5_gt_clk_i2c9_lpmcu_END (12)
#define SOC_SCTRL_SCPEREN5_gt_clk_i2c9_mdm_START (13)
#define SOC_SCTRL_SCPEREN5_gt_clk_i2c9_mdm_END (13)
#define SOC_SCTRL_SCPEREN5_gt_pclk_intr_hub_START (14)
#define SOC_SCTRL_SCPEREN5_gt_pclk_intr_hub_END (14)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int gt_pclk_timer17 : 1;
        unsigned int reserved_0 : 1;
        unsigned int gt_pclk_timer16 : 1;
        unsigned int reserved_1 : 1;
        unsigned int gt_pclk_timer15 : 1;
        unsigned int reserved_2 : 1;
        unsigned int gt_pclk_timer14 : 1;
        unsigned int reserved_3 : 1;
        unsigned int gt_pclk_timer13 : 1;
        unsigned int reserved_4 : 1;
        unsigned int gt_clk_i2c9 : 1;
        unsigned int gt_clk_i2c9_asp : 1;
        unsigned int gt_clk_i2c9_lpmcu : 1;
        unsigned int gt_clk_i2c9_mdm : 1;
        unsigned int gt_pclk_intr_hub : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int reserved_11 : 1;
        unsigned int reserved_12 : 1;
        unsigned int reserved_13 : 1;
        unsigned int reserved_14 : 1;
        unsigned int reserved_15 : 1;
        unsigned int reserved_16 : 1;
        unsigned int reserved_17 : 1;
        unsigned int reserved_18 : 1;
        unsigned int reserved_19 : 1;
        unsigned int reserved_20 : 1;
        unsigned int reserved_21 : 1;
    } reg;
} SOC_SCTRL_SCPERDIS5_UNION;
#endif
#define SOC_SCTRL_SCPERDIS5_gt_pclk_timer17_START (0)
#define SOC_SCTRL_SCPERDIS5_gt_pclk_timer17_END (0)
#define SOC_SCTRL_SCPERDIS5_gt_pclk_timer16_START (2)
#define SOC_SCTRL_SCPERDIS5_gt_pclk_timer16_END (2)
#define SOC_SCTRL_SCPERDIS5_gt_pclk_timer15_START (4)
#define SOC_SCTRL_SCPERDIS5_gt_pclk_timer15_END (4)
#define SOC_SCTRL_SCPERDIS5_gt_pclk_timer14_START (6)
#define SOC_SCTRL_SCPERDIS5_gt_pclk_timer14_END (6)
#define SOC_SCTRL_SCPERDIS5_gt_pclk_timer13_START (8)
#define SOC_SCTRL_SCPERDIS5_gt_pclk_timer13_END (8)
#define SOC_SCTRL_SCPERDIS5_gt_clk_i2c9_START (10)
#define SOC_SCTRL_SCPERDIS5_gt_clk_i2c9_END (10)
#define SOC_SCTRL_SCPERDIS5_gt_clk_i2c9_asp_START (11)
#define SOC_SCTRL_SCPERDIS5_gt_clk_i2c9_asp_END (11)
#define SOC_SCTRL_SCPERDIS5_gt_clk_i2c9_lpmcu_START (12)
#define SOC_SCTRL_SCPERDIS5_gt_clk_i2c9_lpmcu_END (12)
#define SOC_SCTRL_SCPERDIS5_gt_clk_i2c9_mdm_START (13)
#define SOC_SCTRL_SCPERDIS5_gt_clk_i2c9_mdm_END (13)
#define SOC_SCTRL_SCPERDIS5_gt_pclk_intr_hub_START (14)
#define SOC_SCTRL_SCPERDIS5_gt_pclk_intr_hub_END (14)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int gt_pclk_timer17 : 1;
        unsigned int reserved_0 : 1;
        unsigned int gt_pclk_timer16 : 1;
        unsigned int reserved_1 : 1;
        unsigned int gt_pclk_timer15 : 1;
        unsigned int reserved_2 : 1;
        unsigned int gt_pclk_timer14 : 1;
        unsigned int reserved_3 : 1;
        unsigned int gt_pclk_timer13 : 1;
        unsigned int reserved_4 : 1;
        unsigned int gt_clk_i2c9 : 1;
        unsigned int gt_clk_i2c9_asp : 1;
        unsigned int gt_clk_i2c9_lpmcu : 1;
        unsigned int gt_clk_i2c9_mdm : 1;
        unsigned int gt_pclk_intr_hub : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int reserved_11 : 1;
        unsigned int reserved_12 : 1;
        unsigned int reserved_13 : 1;
        unsigned int reserved_14 : 1;
        unsigned int reserved_15 : 1;
        unsigned int reserved_16 : 1;
        unsigned int reserved_17 : 1;
        unsigned int reserved_18 : 1;
        unsigned int reserved_19 : 1;
        unsigned int reserved_20 : 1;
        unsigned int reserved_21 : 1;
    } reg;
} SOC_SCTRL_SCPERCLKEN5_UNION;
#endif
#define SOC_SCTRL_SCPERCLKEN5_gt_pclk_timer17_START (0)
#define SOC_SCTRL_SCPERCLKEN5_gt_pclk_timer17_END (0)
#define SOC_SCTRL_SCPERCLKEN5_gt_pclk_timer16_START (2)
#define SOC_SCTRL_SCPERCLKEN5_gt_pclk_timer16_END (2)
#define SOC_SCTRL_SCPERCLKEN5_gt_pclk_timer15_START (4)
#define SOC_SCTRL_SCPERCLKEN5_gt_pclk_timer15_END (4)
#define SOC_SCTRL_SCPERCLKEN5_gt_pclk_timer14_START (6)
#define SOC_SCTRL_SCPERCLKEN5_gt_pclk_timer14_END (6)
#define SOC_SCTRL_SCPERCLKEN5_gt_pclk_timer13_START (8)
#define SOC_SCTRL_SCPERCLKEN5_gt_pclk_timer13_END (8)
#define SOC_SCTRL_SCPERCLKEN5_gt_clk_i2c9_START (10)
#define SOC_SCTRL_SCPERCLKEN5_gt_clk_i2c9_END (10)
#define SOC_SCTRL_SCPERCLKEN5_gt_clk_i2c9_asp_START (11)
#define SOC_SCTRL_SCPERCLKEN5_gt_clk_i2c9_asp_END (11)
#define SOC_SCTRL_SCPERCLKEN5_gt_clk_i2c9_lpmcu_START (12)
#define SOC_SCTRL_SCPERCLKEN5_gt_clk_i2c9_lpmcu_END (12)
#define SOC_SCTRL_SCPERCLKEN5_gt_clk_i2c9_mdm_START (13)
#define SOC_SCTRL_SCPERCLKEN5_gt_clk_i2c9_mdm_END (13)
#define SOC_SCTRL_SCPERCLKEN5_gt_pclk_intr_hub_START (14)
#define SOC_SCTRL_SCPERCLKEN5_gt_pclk_intr_hub_END (14)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0 : 2;
        unsigned int ulppll_1_lock : 1;
        unsigned int ulppll_lock : 1;
        unsigned int swdone_clk_blpwm2_div : 1;
        unsigned int st_clk_ao_tcp_h2x : 1;
        unsigned int st_clk_ao_tcp : 1;
        unsigned int st_clk_ao_tcp_32k : 1;
        unsigned int reserved_1 : 1;
        unsigned int sw_ack_clk_spi3_sw : 2;
        unsigned int sw_ack_clk_spi5_sw : 2;
        unsigned int st_clk_qcr_iomcubus : 1;
        unsigned int st_clk_spll_peri : 1;
        unsigned int swdone_clk_i2c9_div : 1;
        unsigned int reserved_2 : 2;
        unsigned int st_clk_sys_ini : 1;
        unsigned int st_clk_ao_tof : 1;
        unsigned int st_pclk_efusec2 : 1;
        unsigned int st_clkin_sys_logic_peri : 1;
        unsigned int st_pclk_ao_wd1 : 1;
        unsigned int st_pclk_hickm : 1;
        unsigned int st_clk_i2c9 : 1;
        unsigned int sw_ack_clk_i2c9_sw : 4;
        unsigned int reserved_3 : 3;
    } reg;
} SOC_SCTRL_SCPERSTAT5_UNION;
#endif
#define SOC_SCTRL_SCPERSTAT5_ulppll_1_lock_START (2)
#define SOC_SCTRL_SCPERSTAT5_ulppll_1_lock_END (2)
#define SOC_SCTRL_SCPERSTAT5_ulppll_lock_START (3)
#define SOC_SCTRL_SCPERSTAT5_ulppll_lock_END (3)
#define SOC_SCTRL_SCPERSTAT5_swdone_clk_blpwm2_div_START (4)
#define SOC_SCTRL_SCPERSTAT5_swdone_clk_blpwm2_div_END (4)
#define SOC_SCTRL_SCPERSTAT5_st_clk_ao_tcp_h2x_START (5)
#define SOC_SCTRL_SCPERSTAT5_st_clk_ao_tcp_h2x_END (5)
#define SOC_SCTRL_SCPERSTAT5_st_clk_ao_tcp_START (6)
#define SOC_SCTRL_SCPERSTAT5_st_clk_ao_tcp_END (6)
#define SOC_SCTRL_SCPERSTAT5_st_clk_ao_tcp_32k_START (7)
#define SOC_SCTRL_SCPERSTAT5_st_clk_ao_tcp_32k_END (7)
#define SOC_SCTRL_SCPERSTAT5_sw_ack_clk_spi3_sw_START (9)
#define SOC_SCTRL_SCPERSTAT5_sw_ack_clk_spi3_sw_END (10)
#define SOC_SCTRL_SCPERSTAT5_sw_ack_clk_spi5_sw_START (11)
#define SOC_SCTRL_SCPERSTAT5_sw_ack_clk_spi5_sw_END (12)
#define SOC_SCTRL_SCPERSTAT5_st_clk_qcr_iomcubus_START (13)
#define SOC_SCTRL_SCPERSTAT5_st_clk_qcr_iomcubus_END (13)
#define SOC_SCTRL_SCPERSTAT5_st_clk_spll_peri_START (14)
#define SOC_SCTRL_SCPERSTAT5_st_clk_spll_peri_END (14)
#define SOC_SCTRL_SCPERSTAT5_swdone_clk_i2c9_div_START (15)
#define SOC_SCTRL_SCPERSTAT5_swdone_clk_i2c9_div_END (15)
#define SOC_SCTRL_SCPERSTAT5_st_clk_sys_ini_START (18)
#define SOC_SCTRL_SCPERSTAT5_st_clk_sys_ini_END (18)
#define SOC_SCTRL_SCPERSTAT5_st_clk_ao_tof_START (19)
#define SOC_SCTRL_SCPERSTAT5_st_clk_ao_tof_END (19)
#define SOC_SCTRL_SCPERSTAT5_st_pclk_efusec2_START (20)
#define SOC_SCTRL_SCPERSTAT5_st_pclk_efusec2_END (20)
#define SOC_SCTRL_SCPERSTAT5_st_clkin_sys_logic_peri_START (21)
#define SOC_SCTRL_SCPERSTAT5_st_clkin_sys_logic_peri_END (21)
#define SOC_SCTRL_SCPERSTAT5_st_pclk_ao_wd1_START (22)
#define SOC_SCTRL_SCPERSTAT5_st_pclk_ao_wd1_END (22)
#define SOC_SCTRL_SCPERSTAT5_st_pclk_hickm_START (23)
#define SOC_SCTRL_SCPERSTAT5_st_pclk_hickm_END (23)
#define SOC_SCTRL_SCPERSTAT5_st_clk_i2c9_START (24)
#define SOC_SCTRL_SCPERSTAT5_st_clk_i2c9_END (24)
#define SOC_SCTRL_SCPERSTAT5_sw_ack_clk_i2c9_sw_START (25)
#define SOC_SCTRL_SCPERSTAT5_sw_ack_clk_i2c9_sw_END (28)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int noc_aobus_idle_flag : 1;
        unsigned int aupll_en_stat : 1;
        unsigned int aupll_bypass_stat : 1;
        unsigned int aupll_gt_stat : 1;
        unsigned int aupll_lock : 1;
        unsigned int reserved : 27;
    } reg;
} SOC_SCTRL_SC_AO_CRG_STA_SEP0_UNION;
#endif
#define SOC_SCTRL_SC_AO_CRG_STA_SEP0_noc_aobus_idle_flag_START (0)
#define SOC_SCTRL_SC_AO_CRG_STA_SEP0_noc_aobus_idle_flag_END (0)
#define SOC_SCTRL_SC_AO_CRG_STA_SEP0_aupll_en_stat_START (1)
#define SOC_SCTRL_SC_AO_CRG_STA_SEP0_aupll_en_stat_END (1)
#define SOC_SCTRL_SC_AO_CRG_STA_SEP0_aupll_bypass_stat_START (2)
#define SOC_SCTRL_SC_AO_CRG_STA_SEP0_aupll_bypass_stat_END (2)
#define SOC_SCTRL_SC_AO_CRG_STA_SEP0_aupll_gt_stat_START (3)
#define SOC_SCTRL_SC_AO_CRG_STA_SEP0_aupll_gt_stat_END (3)
#define SOC_SCTRL_SC_AO_CRG_STA_SEP0_aupll_lock_START (4)
#define SOC_SCTRL_SC_AO_CRG_STA_SEP0_aupll_lock_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0 : 1;
        unsigned int ip_prst_rtc : 1;
        unsigned int reserved_1 : 1;
        unsigned int ip_prst_timer0 : 1;
        unsigned int ip_prst_blpwm2 : 1;
        unsigned int ip_rst_qic_hieps_ree : 1;
        unsigned int ip_rst_hieps_ree_crg : 1;
        unsigned int ip_rst_hieps_ree : 1;
        unsigned int ip_rst_asp_dw_axi : 1;
        unsigned int ip_rst_asp_x2h : 1;
        unsigned int ip_rst_asp_h2p : 1;
        unsigned int ip_rst_em_sync_top : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int ip_prst_ao_ioc : 1;
        unsigned int ip_prst_syscnt : 1;
        unsigned int ip_rst_syscnt : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int reserved_11 : 1;
        unsigned int reserved_12 : 1;
        unsigned int reserved_13 : 1;
        unsigned int ip_prst_bbpdrx : 1;
        unsigned int ip_prst_ao_ipc_ns : 1;
        unsigned int reserved_14 : 1;
        unsigned int sc_rst_src_clr : 1;
        unsigned int sc_pmurst_ctrl : 1;
    } reg;
} SOC_SCTRL_SCPERRSTEN0_UNION;
#endif
#define SOC_SCTRL_SCPERRSTEN0_ip_prst_rtc_START (1)
#define SOC_SCTRL_SCPERRSTEN0_ip_prst_rtc_END (1)
#define SOC_SCTRL_SCPERRSTEN0_ip_prst_timer0_START (3)
#define SOC_SCTRL_SCPERRSTEN0_ip_prst_timer0_END (3)
#define SOC_SCTRL_SCPERRSTEN0_ip_prst_blpwm2_START (4)
#define SOC_SCTRL_SCPERRSTEN0_ip_prst_blpwm2_END (4)
#define SOC_SCTRL_SCPERRSTEN0_ip_rst_qic_hieps_ree_START (5)
#define SOC_SCTRL_SCPERRSTEN0_ip_rst_qic_hieps_ree_END (5)
#define SOC_SCTRL_SCPERRSTEN0_ip_rst_hieps_ree_crg_START (6)
#define SOC_SCTRL_SCPERRSTEN0_ip_rst_hieps_ree_crg_END (6)
#define SOC_SCTRL_SCPERRSTEN0_ip_rst_hieps_ree_START (7)
#define SOC_SCTRL_SCPERRSTEN0_ip_rst_hieps_ree_END (7)
#define SOC_SCTRL_SCPERRSTEN0_ip_rst_asp_dw_axi_START (8)
#define SOC_SCTRL_SCPERRSTEN0_ip_rst_asp_dw_axi_END (8)
#define SOC_SCTRL_SCPERRSTEN0_ip_rst_asp_x2h_START (9)
#define SOC_SCTRL_SCPERRSTEN0_ip_rst_asp_x2h_END (9)
#define SOC_SCTRL_SCPERRSTEN0_ip_rst_asp_h2p_START (10)
#define SOC_SCTRL_SCPERRSTEN0_ip_rst_asp_h2p_END (10)
#define SOC_SCTRL_SCPERRSTEN0_ip_rst_em_sync_top_START (11)
#define SOC_SCTRL_SCPERRSTEN0_ip_rst_em_sync_top_END (11)
#define SOC_SCTRL_SCPERRSTEN0_ip_prst_ao_ioc_START (18)
#define SOC_SCTRL_SCPERRSTEN0_ip_prst_ao_ioc_END (18)
#define SOC_SCTRL_SCPERRSTEN0_ip_prst_syscnt_START (19)
#define SOC_SCTRL_SCPERRSTEN0_ip_prst_syscnt_END (19)
#define SOC_SCTRL_SCPERRSTEN0_ip_rst_syscnt_START (20)
#define SOC_SCTRL_SCPERRSTEN0_ip_rst_syscnt_END (20)
#define SOC_SCTRL_SCPERRSTEN0_ip_prst_bbpdrx_START (27)
#define SOC_SCTRL_SCPERRSTEN0_ip_prst_bbpdrx_END (27)
#define SOC_SCTRL_SCPERRSTEN0_ip_prst_ao_ipc_ns_START (28)
#define SOC_SCTRL_SCPERRSTEN0_ip_prst_ao_ipc_ns_END (28)
#define SOC_SCTRL_SCPERRSTEN0_sc_rst_src_clr_START (30)
#define SOC_SCTRL_SCPERRSTEN0_sc_rst_src_clr_END (30)
#define SOC_SCTRL_SCPERRSTEN0_sc_pmurst_ctrl_START (31)
#define SOC_SCTRL_SCPERRSTEN0_sc_pmurst_ctrl_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0 : 1;
        unsigned int ip_prst_rtc : 1;
        unsigned int reserved_1 : 1;
        unsigned int ip_prst_timer0 : 1;
        unsigned int ip_prst_blpwm2 : 1;
        unsigned int ip_rst_qic_hieps_ree : 1;
        unsigned int ip_rst_hieps_ree_crg : 1;
        unsigned int ip_rst_hieps_ree : 1;
        unsigned int ip_rst_asp_dw_axi : 1;
        unsigned int ip_rst_asp_x2h : 1;
        unsigned int ip_rst_asp_h2p : 1;
        unsigned int ip_rst_em_sync_top : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int ip_prst_ao_ioc : 1;
        unsigned int ip_prst_syscnt : 1;
        unsigned int ip_rst_syscnt : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int reserved_11 : 1;
        unsigned int reserved_12 : 1;
        unsigned int reserved_13 : 1;
        unsigned int ip_prst_bbpdrx : 1;
        unsigned int ip_prst_ao_ipc_ns : 1;
        unsigned int reserved_14 : 1;
        unsigned int sc_rst_src_clr : 1;
        unsigned int sc_pmurst_ctrl : 1;
    } reg;
} SOC_SCTRL_SCPERRSTDIS0_UNION;
#endif
#define SOC_SCTRL_SCPERRSTDIS0_ip_prst_rtc_START (1)
#define SOC_SCTRL_SCPERRSTDIS0_ip_prst_rtc_END (1)
#define SOC_SCTRL_SCPERRSTDIS0_ip_prst_timer0_START (3)
#define SOC_SCTRL_SCPERRSTDIS0_ip_prst_timer0_END (3)
#define SOC_SCTRL_SCPERRSTDIS0_ip_prst_blpwm2_START (4)
#define SOC_SCTRL_SCPERRSTDIS0_ip_prst_blpwm2_END (4)
#define SOC_SCTRL_SCPERRSTDIS0_ip_rst_qic_hieps_ree_START (5)
#define SOC_SCTRL_SCPERRSTDIS0_ip_rst_qic_hieps_ree_END (5)
#define SOC_SCTRL_SCPERRSTDIS0_ip_rst_hieps_ree_crg_START (6)
#define SOC_SCTRL_SCPERRSTDIS0_ip_rst_hieps_ree_crg_END (6)
#define SOC_SCTRL_SCPERRSTDIS0_ip_rst_hieps_ree_START (7)
#define SOC_SCTRL_SCPERRSTDIS0_ip_rst_hieps_ree_END (7)
#define SOC_SCTRL_SCPERRSTDIS0_ip_rst_asp_dw_axi_START (8)
#define SOC_SCTRL_SCPERRSTDIS0_ip_rst_asp_dw_axi_END (8)
#define SOC_SCTRL_SCPERRSTDIS0_ip_rst_asp_x2h_START (9)
#define SOC_SCTRL_SCPERRSTDIS0_ip_rst_asp_x2h_END (9)
#define SOC_SCTRL_SCPERRSTDIS0_ip_rst_asp_h2p_START (10)
#define SOC_SCTRL_SCPERRSTDIS0_ip_rst_asp_h2p_END (10)
#define SOC_SCTRL_SCPERRSTDIS0_ip_rst_em_sync_top_START (11)
#define SOC_SCTRL_SCPERRSTDIS0_ip_rst_em_sync_top_END (11)
#define SOC_SCTRL_SCPERRSTDIS0_ip_prst_ao_ioc_START (18)
#define SOC_SCTRL_SCPERRSTDIS0_ip_prst_ao_ioc_END (18)
#define SOC_SCTRL_SCPERRSTDIS0_ip_prst_syscnt_START (19)
#define SOC_SCTRL_SCPERRSTDIS0_ip_prst_syscnt_END (19)
#define SOC_SCTRL_SCPERRSTDIS0_ip_rst_syscnt_START (20)
#define SOC_SCTRL_SCPERRSTDIS0_ip_rst_syscnt_END (20)
#define SOC_SCTRL_SCPERRSTDIS0_ip_prst_bbpdrx_START (27)
#define SOC_SCTRL_SCPERRSTDIS0_ip_prst_bbpdrx_END (27)
#define SOC_SCTRL_SCPERRSTDIS0_ip_prst_ao_ipc_ns_START (28)
#define SOC_SCTRL_SCPERRSTDIS0_ip_prst_ao_ipc_ns_END (28)
#define SOC_SCTRL_SCPERRSTDIS0_sc_rst_src_clr_START (30)
#define SOC_SCTRL_SCPERRSTDIS0_sc_rst_src_clr_END (30)
#define SOC_SCTRL_SCPERRSTDIS0_sc_pmurst_ctrl_START (31)
#define SOC_SCTRL_SCPERRSTDIS0_sc_pmurst_ctrl_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0 : 1;
        unsigned int ip_prst_rtc : 1;
        unsigned int reserved_1 : 1;
        unsigned int ip_prst_timer0 : 1;
        unsigned int ip_prst_blpwm2 : 1;
        unsigned int ip_rst_qic_hieps_ree : 1;
        unsigned int ip_rst_hieps_ree_crg : 1;
        unsigned int ip_rst_hieps_ree : 1;
        unsigned int ip_rst_asp_dw_axi : 1;
        unsigned int ip_rst_asp_x2h : 1;
        unsigned int ip_rst_asp_h2p : 1;
        unsigned int ip_rst_em_sync_top : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int ip_prst_ao_ioc : 1;
        unsigned int ip_prst_syscnt : 1;
        unsigned int ip_rst_syscnt : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int reserved_11 : 1;
        unsigned int reserved_12 : 1;
        unsigned int reserved_13 : 1;
        unsigned int ip_prst_bbpdrx : 1;
        unsigned int ip_prst_ao_ipc_ns : 1;
        unsigned int reserved_14 : 1;
        unsigned int sc_rst_src_clr : 1;
        unsigned int sc_pmurst_ctrl : 1;
    } reg;
} SOC_SCTRL_SCPERRSTSTAT0_UNION;
#endif
#define SOC_SCTRL_SCPERRSTSTAT0_ip_prst_rtc_START (1)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_prst_rtc_END (1)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_prst_timer0_START (3)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_prst_timer0_END (3)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_prst_blpwm2_START (4)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_prst_blpwm2_END (4)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_rst_qic_hieps_ree_START (5)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_rst_qic_hieps_ree_END (5)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_rst_hieps_ree_crg_START (6)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_rst_hieps_ree_crg_END (6)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_rst_hieps_ree_START (7)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_rst_hieps_ree_END (7)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_rst_asp_dw_axi_START (8)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_rst_asp_dw_axi_END (8)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_rst_asp_x2h_START (9)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_rst_asp_x2h_END (9)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_rst_asp_h2p_START (10)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_rst_asp_h2p_END (10)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_rst_em_sync_top_START (11)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_rst_em_sync_top_END (11)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_prst_ao_ioc_START (18)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_prst_ao_ioc_END (18)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_prst_syscnt_START (19)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_prst_syscnt_END (19)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_rst_syscnt_START (20)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_rst_syscnt_END (20)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_prst_bbpdrx_START (27)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_prst_bbpdrx_END (27)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_prst_ao_ipc_ns_START (28)
#define SOC_SCTRL_SCPERRSTSTAT0_ip_prst_ao_ipc_ns_END (28)
#define SOC_SCTRL_SCPERRSTSTAT0_sc_rst_src_clr_START (30)
#define SOC_SCTRL_SCPERRSTSTAT0_sc_rst_src_clr_END (30)
#define SOC_SCTRL_SCPERRSTSTAT0_sc_pmurst_ctrl_START (31)
#define SOC_SCTRL_SCPERRSTSTAT0_sc_pmurst_ctrl_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ip_rst_spmi : 1;
        unsigned int reserved_0 : 1;
        unsigned int ip_prst_dbg_to_ao : 1;
        unsigned int ip_prst_timer4 : 1;
        unsigned int ip_prst_timer5 : 1;
        unsigned int ip_prst_timer6 : 1;
        unsigned int reserved_1 : 1;
        unsigned int ip_prst_timer8 : 1;
        unsigned int reserved_2 : 1;
        unsigned int ip_rst_trace_ao : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int ip_prst_ao_loadmonitor : 1;
        unsigned int ip_rst_ufs_subsys : 1;
        unsigned int reserved_6 : 1;
        unsigned int ip_rst_dram_tmp_0 : 2;
        unsigned int reserved_7 : 1;
        unsigned int ip_rst_ufs_crg : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int reserved_11 : 1;
        unsigned int reserved_12 : 1;
        unsigned int reserved_13 : 1;
        unsigned int reserved_14 : 1;
        unsigned int reserved_15 : 1;
        unsigned int reserved_16 : 1;
        unsigned int reserved_17 : 1;
        unsigned int reserved_18 : 1;
        unsigned int reserved_19 : 1;
    } reg;
} SOC_SCTRL_SCPERRSTEN1_UNION;
#endif
#define SOC_SCTRL_SCPERRSTEN1_ip_rst_spmi_START (0)
#define SOC_SCTRL_SCPERRSTEN1_ip_rst_spmi_END (0)
#define SOC_SCTRL_SCPERRSTEN1_ip_prst_dbg_to_ao_START (2)
#define SOC_SCTRL_SCPERRSTEN1_ip_prst_dbg_to_ao_END (2)
#define SOC_SCTRL_SCPERRSTEN1_ip_prst_timer4_START (3)
#define SOC_SCTRL_SCPERRSTEN1_ip_prst_timer4_END (3)
#define SOC_SCTRL_SCPERRSTEN1_ip_prst_timer5_START (4)
#define SOC_SCTRL_SCPERRSTEN1_ip_prst_timer5_END (4)
#define SOC_SCTRL_SCPERRSTEN1_ip_prst_timer6_START (5)
#define SOC_SCTRL_SCPERRSTEN1_ip_prst_timer6_END (5)
#define SOC_SCTRL_SCPERRSTEN1_ip_prst_timer8_START (7)
#define SOC_SCTRL_SCPERRSTEN1_ip_prst_timer8_END (7)
#define SOC_SCTRL_SCPERRSTEN1_ip_rst_trace_ao_START (9)
#define SOC_SCTRL_SCPERRSTEN1_ip_rst_trace_ao_END (9)
#define SOC_SCTRL_SCPERRSTEN1_ip_prst_ao_loadmonitor_START (13)
#define SOC_SCTRL_SCPERRSTEN1_ip_prst_ao_loadmonitor_END (13)
#define SOC_SCTRL_SCPERRSTEN1_ip_rst_ufs_subsys_START (14)
#define SOC_SCTRL_SCPERRSTEN1_ip_rst_ufs_subsys_END (14)
#define SOC_SCTRL_SCPERRSTEN1_ip_rst_dram_tmp_0_START (16)
#define SOC_SCTRL_SCPERRSTEN1_ip_rst_dram_tmp_0_END (17)
#define SOC_SCTRL_SCPERRSTEN1_ip_rst_ufs_crg_START (19)
#define SOC_SCTRL_SCPERRSTEN1_ip_rst_ufs_crg_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ip_rst_spmi : 1;
        unsigned int reserved_0 : 1;
        unsigned int ip_prst_dbg_to_ao : 1;
        unsigned int ip_prst_timer4 : 1;
        unsigned int ip_prst_timer5 : 1;
        unsigned int ip_prst_timer6 : 1;
        unsigned int reserved_1 : 1;
        unsigned int ip_prst_timer8 : 1;
        unsigned int reserved_2 : 1;
        unsigned int ip_rst_trace_ao : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int ip_prst_ao_loadmonitor : 1;
        unsigned int ip_rst_ufs_subsys : 1;
        unsigned int reserved_6 : 1;
        unsigned int ip_rst_dram_tmp_0 : 2;
        unsigned int reserved_7 : 1;
        unsigned int ip_rst_ufs_crg : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int reserved_11 : 1;
        unsigned int reserved_12 : 1;
        unsigned int reserved_13 : 1;
        unsigned int reserved_14 : 1;
        unsigned int reserved_15 : 1;
        unsigned int reserved_16 : 1;
        unsigned int reserved_17 : 1;
        unsigned int reserved_18 : 1;
        unsigned int reserved_19 : 1;
    } reg;
} SOC_SCTRL_SCPERRSTDIS1_UNION;
#endif
#define SOC_SCTRL_SCPERRSTDIS1_ip_rst_spmi_START (0)
#define SOC_SCTRL_SCPERRSTDIS1_ip_rst_spmi_END (0)
#define SOC_SCTRL_SCPERRSTDIS1_ip_prst_dbg_to_ao_START (2)
#define SOC_SCTRL_SCPERRSTDIS1_ip_prst_dbg_to_ao_END (2)
#define SOC_SCTRL_SCPERRSTDIS1_ip_prst_timer4_START (3)
#define SOC_SCTRL_SCPERRSTDIS1_ip_prst_timer4_END (3)
#define SOC_SCTRL_SCPERRSTDIS1_ip_prst_timer5_START (4)
#define SOC_SCTRL_SCPERRSTDIS1_ip_prst_timer5_END (4)
#define SOC_SCTRL_SCPERRSTDIS1_ip_prst_timer6_START (5)
#define SOC_SCTRL_SCPERRSTDIS1_ip_prst_timer6_END (5)
#define SOC_SCTRL_SCPERRSTDIS1_ip_prst_timer8_START (7)
#define SOC_SCTRL_SCPERRSTDIS1_ip_prst_timer8_END (7)
#define SOC_SCTRL_SCPERRSTDIS1_ip_rst_trace_ao_START (9)
#define SOC_SCTRL_SCPERRSTDIS1_ip_rst_trace_ao_END (9)
#define SOC_SCTRL_SCPERRSTDIS1_ip_prst_ao_loadmonitor_START (13)
#define SOC_SCTRL_SCPERRSTDIS1_ip_prst_ao_loadmonitor_END (13)
#define SOC_SCTRL_SCPERRSTDIS1_ip_rst_ufs_subsys_START (14)
#define SOC_SCTRL_SCPERRSTDIS1_ip_rst_ufs_subsys_END (14)
#define SOC_SCTRL_SCPERRSTDIS1_ip_rst_dram_tmp_0_START (16)
#define SOC_SCTRL_SCPERRSTDIS1_ip_rst_dram_tmp_0_END (17)
#define SOC_SCTRL_SCPERRSTDIS1_ip_rst_ufs_crg_START (19)
#define SOC_SCTRL_SCPERRSTDIS1_ip_rst_ufs_crg_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ip_rst_spmi : 1;
        unsigned int reserved_0 : 1;
        unsigned int ip_prst_dbg_to_ao : 1;
        unsigned int ip_prst_timer4 : 1;
        unsigned int ip_prst_timer5 : 1;
        unsigned int ip_prst_timer6 : 1;
        unsigned int reserved_1 : 1;
        unsigned int ip_prst_timer8 : 1;
        unsigned int reserved_2 : 1;
        unsigned int ip_rst_trace_ao : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int ip_prst_ao_loadmonitor : 1;
        unsigned int ip_rst_ufs_subsys : 1;
        unsigned int reserved_6 : 1;
        unsigned int ip_rst_dram_tmp_0 : 2;
        unsigned int reserved_7 : 1;
        unsigned int ip_rst_ufs_crg : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int reserved_11 : 1;
        unsigned int reserved_12 : 1;
        unsigned int reserved_13 : 1;
        unsigned int reserved_14 : 1;
        unsigned int reserved_15 : 1;
        unsigned int reserved_16 : 1;
        unsigned int reserved_17 : 1;
        unsigned int reserved_18 : 1;
        unsigned int reserved_19 : 1;
    } reg;
} SOC_SCTRL_SCPERRSTSTAT1_UNION;
#endif
#define SOC_SCTRL_SCPERRSTSTAT1_ip_rst_spmi_START (0)
#define SOC_SCTRL_SCPERRSTSTAT1_ip_rst_spmi_END (0)
#define SOC_SCTRL_SCPERRSTSTAT1_ip_prst_dbg_to_ao_START (2)
#define SOC_SCTRL_SCPERRSTSTAT1_ip_prst_dbg_to_ao_END (2)
#define SOC_SCTRL_SCPERRSTSTAT1_ip_prst_timer4_START (3)
#define SOC_SCTRL_SCPERRSTSTAT1_ip_prst_timer4_END (3)
#define SOC_SCTRL_SCPERRSTSTAT1_ip_prst_timer5_START (4)
#define SOC_SCTRL_SCPERRSTSTAT1_ip_prst_timer5_END (4)
#define SOC_SCTRL_SCPERRSTSTAT1_ip_prst_timer6_START (5)
#define SOC_SCTRL_SCPERRSTSTAT1_ip_prst_timer6_END (5)
#define SOC_SCTRL_SCPERRSTSTAT1_ip_prst_timer8_START (7)
#define SOC_SCTRL_SCPERRSTSTAT1_ip_prst_timer8_END (7)
#define SOC_SCTRL_SCPERRSTSTAT1_ip_rst_trace_ao_START (9)
#define SOC_SCTRL_SCPERRSTSTAT1_ip_rst_trace_ao_END (9)
#define SOC_SCTRL_SCPERRSTSTAT1_ip_prst_ao_loadmonitor_START (13)
#define SOC_SCTRL_SCPERRSTSTAT1_ip_prst_ao_loadmonitor_END (13)
#define SOC_SCTRL_SCPERRSTSTAT1_ip_rst_ufs_subsys_START (14)
#define SOC_SCTRL_SCPERRSTSTAT1_ip_rst_ufs_subsys_END (14)
#define SOC_SCTRL_SCPERRSTSTAT1_ip_rst_dram_tmp_0_START (16)
#define SOC_SCTRL_SCPERRSTSTAT1_ip_rst_dram_tmp_0_END (17)
#define SOC_SCTRL_SCPERRSTSTAT1_ip_rst_ufs_crg_START (19)
#define SOC_SCTRL_SCPERRSTSTAT1_ip_rst_ufs_crg_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int ip_rst_spi5 : 1;
        unsigned int soft_rst_dram_dis : 1;
        unsigned int ip_rst_dram_tmp_1 : 2;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int ip_rst_spi3 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int ip_prst_timer17 : 1;
        unsigned int ip_prst_timer16 : 1;
        unsigned int ip_prst_timer15 : 1;
        unsigned int ip_prst_timer14 : 1;
        unsigned int ip_prst_timer13 : 1;
        unsigned int ip_rst_i2c9 : 1;
        unsigned int reserved_11 : 1;
        unsigned int reserved_12 : 1;
        unsigned int reserved_13 : 1;
        unsigned int reserved_14 : 1;
        unsigned int reserved_15 : 1;
        unsigned int reserved_16 : 1;
        unsigned int reserved_17 : 1;
        unsigned int reserved_18 : 1;
        unsigned int reserved_19 : 1;
        unsigned int reserved_20 : 1;
    } reg;
} SOC_SCTRL_SCPERRSTEN2_UNION;
#endif
#define SOC_SCTRL_SCPERRSTEN2_ip_rst_spi5_START (3)
#define SOC_SCTRL_SCPERRSTEN2_ip_rst_spi5_END (3)
#define SOC_SCTRL_SCPERRSTEN2_soft_rst_dram_dis_START (4)
#define SOC_SCTRL_SCPERRSTEN2_soft_rst_dram_dis_END (4)
#define SOC_SCTRL_SCPERRSTEN2_ip_rst_dram_tmp_1_START (5)
#define SOC_SCTRL_SCPERRSTEN2_ip_rst_dram_tmp_1_END (6)
#define SOC_SCTRL_SCPERRSTEN2_ip_rst_spi3_START (9)
#define SOC_SCTRL_SCPERRSTEN2_ip_rst_spi3_END (9)
#define SOC_SCTRL_SCPERRSTEN2_ip_prst_timer17_START (16)
#define SOC_SCTRL_SCPERRSTEN2_ip_prst_timer17_END (16)
#define SOC_SCTRL_SCPERRSTEN2_ip_prst_timer16_START (17)
#define SOC_SCTRL_SCPERRSTEN2_ip_prst_timer16_END (17)
#define SOC_SCTRL_SCPERRSTEN2_ip_prst_timer15_START (18)
#define SOC_SCTRL_SCPERRSTEN2_ip_prst_timer15_END (18)
#define SOC_SCTRL_SCPERRSTEN2_ip_prst_timer14_START (19)
#define SOC_SCTRL_SCPERRSTEN2_ip_prst_timer14_END (19)
#define SOC_SCTRL_SCPERRSTEN2_ip_prst_timer13_START (20)
#define SOC_SCTRL_SCPERRSTEN2_ip_prst_timer13_END (20)
#define SOC_SCTRL_SCPERRSTEN2_ip_rst_i2c9_START (21)
#define SOC_SCTRL_SCPERRSTEN2_ip_rst_i2c9_END (21)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int ip_rst_spi5 : 1;
        unsigned int soft_rst_dram_dis : 1;
        unsigned int ip_rst_dram_tmp_1 : 2;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int ip_rst_spi3 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int ip_prst_timer17 : 1;
        unsigned int ip_prst_timer16 : 1;
        unsigned int ip_prst_timer15 : 1;
        unsigned int ip_prst_timer14 : 1;
        unsigned int ip_prst_timer13 : 1;
        unsigned int ip_rst_i2c9 : 1;
        unsigned int reserved_11 : 1;
        unsigned int reserved_12 : 1;
        unsigned int reserved_13 : 1;
        unsigned int reserved_14 : 1;
        unsigned int reserved_15 : 1;
        unsigned int reserved_16 : 1;
        unsigned int reserved_17 : 1;
        unsigned int reserved_18 : 1;
        unsigned int reserved_19 : 1;
        unsigned int reserved_20 : 1;
    } reg;
} SOC_SCTRL_SCPERRSTDIS2_UNION;
#endif
#define SOC_SCTRL_SCPERRSTDIS2_ip_rst_spi5_START (3)
#define SOC_SCTRL_SCPERRSTDIS2_ip_rst_spi5_END (3)
#define SOC_SCTRL_SCPERRSTDIS2_soft_rst_dram_dis_START (4)
#define SOC_SCTRL_SCPERRSTDIS2_soft_rst_dram_dis_END (4)
#define SOC_SCTRL_SCPERRSTDIS2_ip_rst_dram_tmp_1_START (5)
#define SOC_SCTRL_SCPERRSTDIS2_ip_rst_dram_tmp_1_END (6)
#define SOC_SCTRL_SCPERRSTDIS2_ip_rst_spi3_START (9)
#define SOC_SCTRL_SCPERRSTDIS2_ip_rst_spi3_END (9)
#define SOC_SCTRL_SCPERRSTDIS2_ip_prst_timer17_START (16)
#define SOC_SCTRL_SCPERRSTDIS2_ip_prst_timer17_END (16)
#define SOC_SCTRL_SCPERRSTDIS2_ip_prst_timer16_START (17)
#define SOC_SCTRL_SCPERRSTDIS2_ip_prst_timer16_END (17)
#define SOC_SCTRL_SCPERRSTDIS2_ip_prst_timer15_START (18)
#define SOC_SCTRL_SCPERRSTDIS2_ip_prst_timer15_END (18)
#define SOC_SCTRL_SCPERRSTDIS2_ip_prst_timer14_START (19)
#define SOC_SCTRL_SCPERRSTDIS2_ip_prst_timer14_END (19)
#define SOC_SCTRL_SCPERRSTDIS2_ip_prst_timer13_START (20)
#define SOC_SCTRL_SCPERRSTDIS2_ip_prst_timer13_END (20)
#define SOC_SCTRL_SCPERRSTDIS2_ip_rst_i2c9_START (21)
#define SOC_SCTRL_SCPERRSTDIS2_ip_rst_i2c9_END (21)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int ip_rst_spi5 : 1;
        unsigned int soft_rst_dram_dis : 1;
        unsigned int ip_rst_dram_tmp_1 : 2;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int ip_rst_spi3 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int ip_prst_timer17 : 1;
        unsigned int ip_prst_timer16 : 1;
        unsigned int ip_prst_timer15 : 1;
        unsigned int ip_prst_timer14 : 1;
        unsigned int ip_prst_timer13 : 1;
        unsigned int ip_rst_i2c9 : 1;
        unsigned int reserved_11 : 1;
        unsigned int reserved_12 : 1;
        unsigned int reserved_13 : 1;
        unsigned int reserved_14 : 1;
        unsigned int reserved_15 : 1;
        unsigned int reserved_16 : 1;
        unsigned int reserved_17 : 1;
        unsigned int reserved_18 : 1;
        unsigned int reserved_19 : 1;
        unsigned int reserved_20 : 1;
    } reg;
} SOC_SCTRL_SCPERRSTSTAT2_UNION;
#endif
#define SOC_SCTRL_SCPERRSTSTAT2_ip_rst_spi5_START (3)
#define SOC_SCTRL_SCPERRSTSTAT2_ip_rst_spi5_END (3)
#define SOC_SCTRL_SCPERRSTSTAT2_soft_rst_dram_dis_START (4)
#define SOC_SCTRL_SCPERRSTSTAT2_soft_rst_dram_dis_END (4)
#define SOC_SCTRL_SCPERRSTSTAT2_ip_rst_dram_tmp_1_START (5)
#define SOC_SCTRL_SCPERRSTSTAT2_ip_rst_dram_tmp_1_END (6)
#define SOC_SCTRL_SCPERRSTSTAT2_ip_rst_spi3_START (9)
#define SOC_SCTRL_SCPERRSTSTAT2_ip_rst_spi3_END (9)
#define SOC_SCTRL_SCPERRSTSTAT2_ip_prst_timer17_START (16)
#define SOC_SCTRL_SCPERRSTSTAT2_ip_prst_timer17_END (16)
#define SOC_SCTRL_SCPERRSTSTAT2_ip_prst_timer16_START (17)
#define SOC_SCTRL_SCPERRSTSTAT2_ip_prst_timer16_END (17)
#define SOC_SCTRL_SCPERRSTSTAT2_ip_prst_timer15_START (18)
#define SOC_SCTRL_SCPERRSTSTAT2_ip_prst_timer15_END (18)
#define SOC_SCTRL_SCPERRSTSTAT2_ip_prst_timer14_START (19)
#define SOC_SCTRL_SCPERRSTSTAT2_ip_prst_timer14_END (19)
#define SOC_SCTRL_SCPERRSTSTAT2_ip_prst_timer13_START (20)
#define SOC_SCTRL_SCPERRSTSTAT2_ip_prst_timer13_END (20)
#define SOC_SCTRL_SCPERRSTSTAT2_ip_rst_i2c9_START (21)
#define SOC_SCTRL_SCPERRSTSTAT2_ip_rst_i2c9_END (21)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int div_asp_subsys : 3;
        unsigned int sc_div_hifidsp : 6;
        unsigned int div_clk_out2 : 4;
        unsigned int sel_ao_asp_32kpll : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int scclkdiv0_msk : 16;
    } reg;
} SOC_SCTRL_SCCLKDIV0_UNION;
#endif
#define SOC_SCTRL_SCCLKDIV0_div_asp_subsys_START (0)
#define SOC_SCTRL_SCCLKDIV0_div_asp_subsys_END (2)
#define SOC_SCTRL_SCCLKDIV0_sc_div_hifidsp_START (3)
#define SOC_SCTRL_SCCLKDIV0_sc_div_hifidsp_END (8)
#define SOC_SCTRL_SCCLKDIV0_div_clk_out2_START (9)
#define SOC_SCTRL_SCCLKDIV0_div_clk_out2_END (12)
#define SOC_SCTRL_SCCLKDIV0_sel_ao_asp_32kpll_START (13)
#define SOC_SCTRL_SCCLKDIV0_sel_ao_asp_32kpll_END (13)
#define SOC_SCTRL_SCCLKDIV0_scclkdiv0_msk_START (16)
#define SOC_SCTRL_SCCLKDIV0_scclkdiv0_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0 : 6;
        unsigned int sc_gt_clk_mad_spll : 1;
        unsigned int reserved_1 : 5;
        unsigned int sel_clk_fll_test_src : 4;
        unsigned int scclkdiv1_msk : 16;
    } reg;
} SOC_SCTRL_SCCLKDIV1_UNION;
#endif
#define SOC_SCTRL_SCCLKDIV1_sc_gt_clk_mad_spll_START (6)
#define SOC_SCTRL_SCCLKDIV1_sc_gt_clk_mad_spll_END (6)
#define SOC_SCTRL_SCCLKDIV1_sel_clk_fll_test_src_START (12)
#define SOC_SCTRL_SCCLKDIV1_sel_clk_fll_test_src_END (15)
#define SOC_SCTRL_SCCLKDIV1_scclkdiv1_msk_START (16)
#define SOC_SCTRL_SCCLKDIV1_scclkdiv1_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int sc_gt_clk_asp_subsys : 1;
        unsigned int gt_hifidsp_clk_div : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int gt_asp_hclk_div : 1;
        unsigned int reserved_2 : 1;
        unsigned int gt_clk_spmi_mst_vote : 10;
        unsigned int scclkdiv2_msk : 16;
    } reg;
} SOC_SCTRL_SCCLKDIV2_UNION;
#endif
#define SOC_SCTRL_SCCLKDIV2_sc_gt_clk_asp_subsys_START (0)
#define SOC_SCTRL_SCCLKDIV2_sc_gt_clk_asp_subsys_END (0)
#define SOC_SCTRL_SCCLKDIV2_gt_hifidsp_clk_div_START (1)
#define SOC_SCTRL_SCCLKDIV2_gt_hifidsp_clk_div_END (1)
#define SOC_SCTRL_SCCLKDIV2_gt_asp_hclk_div_START (4)
#define SOC_SCTRL_SCCLKDIV2_gt_asp_hclk_div_END (4)
#define SOC_SCTRL_SCCLKDIV2_gt_clk_spmi_mst_vote_START (6)
#define SOC_SCTRL_SCCLKDIV2_gt_clk_spmi_mst_vote_END (15)
#define SOC_SCTRL_SCCLKDIV2_scclkdiv2_msk_START (16)
#define SOC_SCTRL_SCCLKDIV2_scclkdiv2_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0 : 10;
        unsigned int reserved_1 : 6;
        unsigned int scclkdiv3_msk : 16;
    } reg;
} SOC_SCTRL_SCCLKDIV3_UNION;
#endif
#define SOC_SCTRL_SCCLKDIV3_scclkdiv3_msk_START (16)
#define SOC_SCTRL_SCCLKDIV3_scclkdiv3_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int div_ao_camera : 6;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int sc_div_asp_hclk : 2;
        unsigned int reserved_2 : 6;
        unsigned int scclkdiv4_msk : 16;
    } reg;
} SOC_SCTRL_SCCLKDIV4_UNION;
#endif
#define SOC_SCTRL_SCCLKDIV4_div_ao_camera_START (0)
#define SOC_SCTRL_SCCLKDIV4_div_ao_camera_END (5)
#define SOC_SCTRL_SCCLKDIV4_sc_div_asp_hclk_START (8)
#define SOC_SCTRL_SCCLKDIV4_sc_div_asp_hclk_END (9)
#define SOC_SCTRL_SCCLKDIV4_scclkdiv4_msk_START (16)
#define SOC_SCTRL_SCCLKDIV4_scclkdiv4_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 10;
        unsigned int reserved_2 : 2;
        unsigned int sel_syscnt : 1;
        unsigned int sc_gt_clk_blpwm2 : 1;
        unsigned int sc_gt_clk_mad_fll : 1;
        unsigned int scclkdiv5_msk : 16;
    } reg;
} SOC_SCTRL_SCCLKDIV5_UNION;
#endif
#define SOC_SCTRL_SCCLKDIV5_sel_syscnt_START (13)
#define SOC_SCTRL_SCCLKDIV5_sel_syscnt_END (13)
#define SOC_SCTRL_SCCLKDIV5_sc_gt_clk_blpwm2_START (14)
#define SOC_SCTRL_SCCLKDIV5_sc_gt_clk_blpwm2_END (14)
#define SOC_SCTRL_SCCLKDIV5_sc_gt_clk_mad_fll_START (15)
#define SOC_SCTRL_SCCLKDIV5_sc_gt_clk_mad_fll_END (15)
#define SOC_SCTRL_SCCLKDIV5_scclkdiv5_msk_START (16)
#define SOC_SCTRL_SCCLKDIV5_scclkdiv5_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int div_mad_spll : 4;
        unsigned int div_syscnt : 4;
        unsigned int reserved_0 : 1;
        unsigned int sel_clk_out2 : 2;
        unsigned int reserved_1 : 5;
        unsigned int scclkdiv6_msk : 16;
    } reg;
} SOC_SCTRL_SCCLKDIV6_UNION;
#endif
#define SOC_SCTRL_SCCLKDIV6_div_mad_spll_START (0)
#define SOC_SCTRL_SCCLKDIV6_div_mad_spll_END (3)
#define SOC_SCTRL_SCCLKDIV6_div_syscnt_START (4)
#define SOC_SCTRL_SCCLKDIV6_div_syscnt_END (7)
#define SOC_SCTRL_SCCLKDIV6_sel_clk_out2_START (9)
#define SOC_SCTRL_SCCLKDIV6_sel_clk_out2_END (10)
#define SOC_SCTRL_SCCLKDIV6_scclkdiv6_msk_START (16)
#define SOC_SCTRL_SCCLKDIV6_scclkdiv6_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int sc_gt_clk_memrepair : 1;
        unsigned int reserved_0 : 1;
        unsigned int div_memrepair : 6;
        unsigned int reserved_1 : 1;
        unsigned int sc_gt_clk_ao_loadmonitor : 1;
        unsigned int div_ao_loadmonitor : 4;
        unsigned int reserved_2 : 2;
        unsigned int scclkdiv7_msk : 16;
    } reg;
} SOC_SCTRL_SCCLKDIV7_UNION;
#endif
#define SOC_SCTRL_SCCLKDIV7_sc_gt_clk_memrepair_START (0)
#define SOC_SCTRL_SCCLKDIV7_sc_gt_clk_memrepair_END (0)
#define SOC_SCTRL_SCCLKDIV7_div_memrepair_START (2)
#define SOC_SCTRL_SCCLKDIV7_div_memrepair_END (7)
#define SOC_SCTRL_SCCLKDIV7_sc_gt_clk_ao_loadmonitor_START (9)
#define SOC_SCTRL_SCCLKDIV7_sc_gt_clk_ao_loadmonitor_END (9)
#define SOC_SCTRL_SCCLKDIV7_div_ao_loadmonitor_START (10)
#define SOC_SCTRL_SCCLKDIV7_div_ao_loadmonitor_END (13)
#define SOC_SCTRL_SCCLKDIV7_scclkdiv7_msk_START (16)
#define SOC_SCTRL_SCCLKDIV7_scclkdiv7_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int div_em_sync_top : 6;
        unsigned int sc_gt_clk_em_sync_top : 1;
        unsigned int sel_mad_mux : 1;
        unsigned int sc_gt_clk_spmi_mst : 1;
        unsigned int div_spmi_mst : 6;
        unsigned int sel_spmi_mst : 1;
        unsigned int scclkdiv8_msk : 16;
    } reg;
} SOC_SCTRL_SCCLKDIV8_UNION;
#endif
#define SOC_SCTRL_SCCLKDIV8_div_em_sync_top_START (0)
#define SOC_SCTRL_SCCLKDIV8_div_em_sync_top_END (5)
#define SOC_SCTRL_SCCLKDIV8_sc_gt_clk_em_sync_top_START (6)
#define SOC_SCTRL_SCCLKDIV8_sc_gt_clk_em_sync_top_END (6)
#define SOC_SCTRL_SCCLKDIV8_sel_mad_mux_START (7)
#define SOC_SCTRL_SCCLKDIV8_sel_mad_mux_END (7)
#define SOC_SCTRL_SCCLKDIV8_sc_gt_clk_spmi_mst_START (8)
#define SOC_SCTRL_SCCLKDIV8_sc_gt_clk_spmi_mst_END (8)
#define SOC_SCTRL_SCCLKDIV8_div_spmi_mst_START (9)
#define SOC_SCTRL_SCCLKDIV8_div_spmi_mst_END (14)
#define SOC_SCTRL_SCCLKDIV8_sel_spmi_mst_START (15)
#define SOC_SCTRL_SCCLKDIV8_sel_spmi_mst_END (15)
#define SOC_SCTRL_SCCLKDIV8_scclkdiv8_msk_START (16)
#define SOC_SCTRL_SCCLKDIV8_scclkdiv8_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int div_ufs_subsys_pll : 6;
        unsigned int div_sys_ufs_subsys : 2;
        unsigned int sc_gt_clk_ufs_subsys_pll : 1;
        unsigned int ufsbus_div_auto_reduce_bypass_acpu : 1;
        unsigned int ufsbus_div_auto_reduce_bypass_mcpu : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int gt_clk_dp_audio_pll_tp : 1;
        unsigned int sc_gt_clk_asp_codec_pll : 1;
        unsigned int sc_gt_clk_asp_subsys_fll : 1;
        unsigned int scclkdiv9_msk : 16;
    } reg;
} SOC_SCTRL_SCCLKDIV9_UNION;
#endif
#define SOC_SCTRL_SCCLKDIV9_div_ufs_subsys_pll_START (0)
#define SOC_SCTRL_SCCLKDIV9_div_ufs_subsys_pll_END (5)
#define SOC_SCTRL_SCCLKDIV9_div_sys_ufs_subsys_START (6)
#define SOC_SCTRL_SCCLKDIV9_div_sys_ufs_subsys_END (7)
#define SOC_SCTRL_SCCLKDIV9_sc_gt_clk_ufs_subsys_pll_START (8)
#define SOC_SCTRL_SCCLKDIV9_sc_gt_clk_ufs_subsys_pll_END (8)
#define SOC_SCTRL_SCCLKDIV9_ufsbus_div_auto_reduce_bypass_acpu_START (9)
#define SOC_SCTRL_SCCLKDIV9_ufsbus_div_auto_reduce_bypass_acpu_END (9)
#define SOC_SCTRL_SCCLKDIV9_ufsbus_div_auto_reduce_bypass_mcpu_START (10)
#define SOC_SCTRL_SCCLKDIV9_ufsbus_div_auto_reduce_bypass_mcpu_END (10)
#define SOC_SCTRL_SCCLKDIV9_gt_clk_dp_audio_pll_tp_START (13)
#define SOC_SCTRL_SCCLKDIV9_gt_clk_dp_audio_pll_tp_END (13)
#define SOC_SCTRL_SCCLKDIV9_sc_gt_clk_asp_codec_pll_START (14)
#define SOC_SCTRL_SCCLKDIV9_sc_gt_clk_asp_codec_pll_END (14)
#define SOC_SCTRL_SCCLKDIV9_sc_gt_clk_asp_subsys_fll_START (15)
#define SOC_SCTRL_SCCLKDIV9_sc_gt_clk_asp_subsys_fll_END (15)
#define SOC_SCTRL_SCCLKDIV9_scclkdiv9_msk_START (16)
#define SOC_SCTRL_SCCLKDIV9_scclkdiv9_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ufsbus_div_auto_reduce_bypass_lpm3 : 1;
        unsigned int ufsbus_auto_waitcfg_in : 10;
        unsigned int ufsbus_auto_waitcfg_out : 10;
        unsigned int ufsbus_div_auto_cfg : 6;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
    } reg;
} SOC_SCTRL_SCUFS_AUTODIV_UNION;
#endif
#define SOC_SCTRL_SCUFS_AUTODIV_ufsbus_div_auto_reduce_bypass_lpm3_START (0)
#define SOC_SCTRL_SCUFS_AUTODIV_ufsbus_div_auto_reduce_bypass_lpm3_END (0)
#define SOC_SCTRL_SCUFS_AUTODIV_ufsbus_auto_waitcfg_in_START (1)
#define SOC_SCTRL_SCUFS_AUTODIV_ufsbus_auto_waitcfg_in_END (10)
#define SOC_SCTRL_SCUFS_AUTODIV_ufsbus_auto_waitcfg_out_START (11)
#define SOC_SCTRL_SCUFS_AUTODIV_ufsbus_auto_waitcfg_out_END (20)
#define SOC_SCTRL_SCUFS_AUTODIV_ufsbus_div_auto_cfg_START (21)
#define SOC_SCTRL_SCUFS_AUTODIV_ufsbus_div_auto_cfg_END (26)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int intr_mask_asp_nonidle_pend : 1;
        unsigned int intr_mask_iomcu_nonidle_pend : 1;
        unsigned int intr_mask_hisee_nonidle_pend : 1;
        unsigned int intr_mask_hieps_nonidle_pend : 1;
        unsigned int intr_mask_hieps_ree_nonidle_pend : 1;
        unsigned int reserved_0 : 3;
        unsigned int intr_clr_asp_nonidle_pend : 1;
        unsigned int intr_clr_iomcu_nonidle_pend : 1;
        unsigned int intr_clr_hisee_nonidle_pend : 1;
        unsigned int intr_clr_hieps_nonidle_pend : 1;
        unsigned int intr_clr_hieps_ree_nonidle_pend : 1;
        unsigned int reserved_1 : 3;
        unsigned int sc_intr_nocbus_ctrl_msk : 16;
    } reg;
} SOC_SCTRL_SC_INTR_NOCBUS_CTRL_UNION;
#endif
#define SOC_SCTRL_SC_INTR_NOCBUS_CTRL_intr_mask_asp_nonidle_pend_START (0)
#define SOC_SCTRL_SC_INTR_NOCBUS_CTRL_intr_mask_asp_nonidle_pend_END (0)
#define SOC_SCTRL_SC_INTR_NOCBUS_CTRL_intr_mask_iomcu_nonidle_pend_START (1)
#define SOC_SCTRL_SC_INTR_NOCBUS_CTRL_intr_mask_iomcu_nonidle_pend_END (1)
#define SOC_SCTRL_SC_INTR_NOCBUS_CTRL_intr_mask_hisee_nonidle_pend_START (2)
#define SOC_SCTRL_SC_INTR_NOCBUS_CTRL_intr_mask_hisee_nonidle_pend_END (2)
#define SOC_SCTRL_SC_INTR_NOCBUS_CTRL_intr_mask_hieps_nonidle_pend_START (3)
#define SOC_SCTRL_SC_INTR_NOCBUS_CTRL_intr_mask_hieps_nonidle_pend_END (3)
#define SOC_SCTRL_SC_INTR_NOCBUS_CTRL_intr_mask_hieps_ree_nonidle_pend_START (4)
#define SOC_SCTRL_SC_INTR_NOCBUS_CTRL_intr_mask_hieps_ree_nonidle_pend_END (4)
#define SOC_SCTRL_SC_INTR_NOCBUS_CTRL_intr_clr_asp_nonidle_pend_START (8)
#define SOC_SCTRL_SC_INTR_NOCBUS_CTRL_intr_clr_asp_nonidle_pend_END (8)
#define SOC_SCTRL_SC_INTR_NOCBUS_CTRL_intr_clr_iomcu_nonidle_pend_START (9)
#define SOC_SCTRL_SC_INTR_NOCBUS_CTRL_intr_clr_iomcu_nonidle_pend_END (9)
#define SOC_SCTRL_SC_INTR_NOCBUS_CTRL_intr_clr_hisee_nonidle_pend_START (10)
#define SOC_SCTRL_SC_INTR_NOCBUS_CTRL_intr_clr_hisee_nonidle_pend_END (10)
#define SOC_SCTRL_SC_INTR_NOCBUS_CTRL_intr_clr_hieps_nonidle_pend_START (11)
#define SOC_SCTRL_SC_INTR_NOCBUS_CTRL_intr_clr_hieps_nonidle_pend_END (11)
#define SOC_SCTRL_SC_INTR_NOCBUS_CTRL_intr_clr_hieps_ree_nonidle_pend_START (12)
#define SOC_SCTRL_SC_INTR_NOCBUS_CTRL_intr_clr_hieps_ree_nonidle_pend_END (12)
#define SOC_SCTRL_SC_INTR_NOCBUS_CTRL_sc_intr_nocbus_ctrl_msk_START (16)
#define SOC_SCTRL_SC_INTR_NOCBUS_CTRL_sc_intr_nocbus_ctrl_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int sel_asp_subsys : 4;
        unsigned int div_asp_subsys_fll : 2;
        unsigned int reserved : 5;
        unsigned int div_fll_src_tp : 5;
        unsigned int scclkdiv10_msk : 16;
    } reg;
} SOC_SCTRL_SCCLKDIV10_UNION;
#endif
#define SOC_SCTRL_SCCLKDIV10_sel_asp_subsys_START (0)
#define SOC_SCTRL_SCCLKDIV10_sel_asp_subsys_END (3)
#define SOC_SCTRL_SCCLKDIV10_div_asp_subsys_fll_START (4)
#define SOC_SCTRL_SCCLKDIV10_div_asp_subsys_fll_END (5)
#define SOC_SCTRL_SCCLKDIV10_div_fll_src_tp_START (11)
#define SOC_SCTRL_SCCLKDIV10_div_fll_src_tp_END (15)
#define SOC_SCTRL_SCCLKDIV10_scclkdiv10_msk_START (16)
#define SOC_SCTRL_SCCLKDIV10_scclkdiv10_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int div_asp_codec : 6;
        unsigned int sel_clk_out0 : 2;
        unsigned int sc_gt_clk_out0 : 1;
        unsigned int sc_gt_clk_out2 : 1;
        unsigned int div_clk_out0 : 4;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int scclkdiv11_msk : 16;
    } reg;
} SOC_SCTRL_SCCLKDIV11_UNION;
#endif
#define SOC_SCTRL_SCCLKDIV11_div_asp_codec_START (0)
#define SOC_SCTRL_SCCLKDIV11_div_asp_codec_END (5)
#define SOC_SCTRL_SCCLKDIV11_sel_clk_out0_START (6)
#define SOC_SCTRL_SCCLKDIV11_sel_clk_out0_END (7)
#define SOC_SCTRL_SCCLKDIV11_sc_gt_clk_out0_START (8)
#define SOC_SCTRL_SCCLKDIV11_sc_gt_clk_out0_END (8)
#define SOC_SCTRL_SCCLKDIV11_sc_gt_clk_out2_START (9)
#define SOC_SCTRL_SCCLKDIV11_sc_gt_clk_out2_END (9)
#define SOC_SCTRL_SCCLKDIV11_div_clk_out0_START (10)
#define SOC_SCTRL_SCCLKDIV11_div_clk_out0_END (13)
#define SOC_SCTRL_SCCLKDIV11_scclkdiv11_msk_START (16)
#define SOC_SCTRL_SCCLKDIV11_scclkdiv11_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int gt_clk_noc_timeout_extref_peri_bypass : 1;
        unsigned int gt_timerclk_refh_peri_bypass : 1;
        unsigned int gt_clkin_sys_peri_bypass : 1;
        unsigned int gt_clkin_ref_peri_bypass : 1;
        unsigned int gt_clk_sys_ini_peri_bypass : 1;
        unsigned int reserved_0 : 1;
        unsigned int gt_clk_ref_peri_bypass : 1;
        unsigned int gt_clk_sys_peri_bypass : 1;
        unsigned int gt_clk_aobus_peri_bypass : 1;
        unsigned int gt_clk_fll_src_peri_bypass : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int gt_clk_spll_peri_bypass : 1;
        unsigned int gt_clkin_sys_logic_peri_bypass : 1;
        unsigned int sel_clk_pcie_aux : 1;
        unsigned int reserved_3 : 1;
        unsigned int scclkdiv12_msk : 16;
    } reg;
} SOC_SCTRL_SCCLKDIV12_UNION;
#endif
#define SOC_SCTRL_SCCLKDIV12_gt_clk_noc_timeout_extref_peri_bypass_START (0)
#define SOC_SCTRL_SCCLKDIV12_gt_clk_noc_timeout_extref_peri_bypass_END (0)
#define SOC_SCTRL_SCCLKDIV12_gt_timerclk_refh_peri_bypass_START (1)
#define SOC_SCTRL_SCCLKDIV12_gt_timerclk_refh_peri_bypass_END (1)
#define SOC_SCTRL_SCCLKDIV12_gt_clkin_sys_peri_bypass_START (2)
#define SOC_SCTRL_SCCLKDIV12_gt_clkin_sys_peri_bypass_END (2)
#define SOC_SCTRL_SCCLKDIV12_gt_clkin_ref_peri_bypass_START (3)
#define SOC_SCTRL_SCCLKDIV12_gt_clkin_ref_peri_bypass_END (3)
#define SOC_SCTRL_SCCLKDIV12_gt_clk_sys_ini_peri_bypass_START (4)
#define SOC_SCTRL_SCCLKDIV12_gt_clk_sys_ini_peri_bypass_END (4)
#define SOC_SCTRL_SCCLKDIV12_gt_clk_ref_peri_bypass_START (6)
#define SOC_SCTRL_SCCLKDIV12_gt_clk_ref_peri_bypass_END (6)
#define SOC_SCTRL_SCCLKDIV12_gt_clk_sys_peri_bypass_START (7)
#define SOC_SCTRL_SCCLKDIV12_gt_clk_sys_peri_bypass_END (7)
#define SOC_SCTRL_SCCLKDIV12_gt_clk_aobus_peri_bypass_START (8)
#define SOC_SCTRL_SCCLKDIV12_gt_clk_aobus_peri_bypass_END (8)
#define SOC_SCTRL_SCCLKDIV12_gt_clk_fll_src_peri_bypass_START (9)
#define SOC_SCTRL_SCCLKDIV12_gt_clk_fll_src_peri_bypass_END (9)
#define SOC_SCTRL_SCCLKDIV12_gt_clk_spll_peri_bypass_START (12)
#define SOC_SCTRL_SCCLKDIV12_gt_clk_spll_peri_bypass_END (12)
#define SOC_SCTRL_SCCLKDIV12_gt_clkin_sys_logic_peri_bypass_START (13)
#define SOC_SCTRL_SCCLKDIV12_gt_clkin_sys_logic_peri_bypass_END (13)
#define SOC_SCTRL_SCCLKDIV12_sel_clk_pcie_aux_START (14)
#define SOC_SCTRL_SCCLKDIV12_sel_clk_pcie_aux_END (14)
#define SOC_SCTRL_SCCLKDIV12_scclkdiv12_msk_START (16)
#define SOC_SCTRL_SCCLKDIV12_scclkdiv12_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int sleep_mode_spi5_bypass : 1;
        unsigned int sleep_mode_dp_audio_bypass : 1;
        unsigned int sleep_mode_spi3_bypass : 1;
        unsigned int sleep_mode_ufs_bypass : 1;
        unsigned int sleep_mode_cssys_bypass : 1;
        unsigned int scclkdiv13_msk : 16;
    } reg;
} SOC_SCTRL_SCCLKDIV13_UNION;
#endif
#define SOC_SCTRL_SCCLKDIV13_sleep_mode_spi5_bypass_START (11)
#define SOC_SCTRL_SCCLKDIV13_sleep_mode_spi5_bypass_END (11)
#define SOC_SCTRL_SCCLKDIV13_sleep_mode_dp_audio_bypass_START (12)
#define SOC_SCTRL_SCCLKDIV13_sleep_mode_dp_audio_bypass_END (12)
#define SOC_SCTRL_SCCLKDIV13_sleep_mode_spi3_bypass_START (13)
#define SOC_SCTRL_SCCLKDIV13_sleep_mode_spi3_bypass_END (13)
#define SOC_SCTRL_SCCLKDIV13_sleep_mode_ufs_bypass_START (14)
#define SOC_SCTRL_SCCLKDIV13_sleep_mode_ufs_bypass_END (14)
#define SOC_SCTRL_SCCLKDIV13_sleep_mode_cssys_bypass_START (15)
#define SOC_SCTRL_SCCLKDIV13_sleep_mode_cssys_bypass_END (15)
#define SOC_SCTRL_SCCLKDIV13_scclkdiv13_msk_START (16)
#define SOC_SCTRL_SCCLKDIV13_scclkdiv13_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int div_memrepair_fll : 4;
        unsigned int sel_clk_ao_camera : 2;
        unsigned int sel_memrepair_peri : 2;
        unsigned int reserved : 4;
        unsigned int sel_memrepair : 2;
        unsigned int sc_gt_clk_ao_camera : 1;
        unsigned int sc_gt_clk_memrepair_fll_div : 1;
        unsigned int scclkdiv14_msk : 16;
    } reg;
} SOC_SCTRL_SCCLKDIV14_UNION;
#endif
#define SOC_SCTRL_SCCLKDIV14_div_memrepair_fll_START (0)
#define SOC_SCTRL_SCCLKDIV14_div_memrepair_fll_END (3)
#define SOC_SCTRL_SCCLKDIV14_sel_clk_ao_camera_START (4)
#define SOC_SCTRL_SCCLKDIV14_sel_clk_ao_camera_END (5)
#define SOC_SCTRL_SCCLKDIV14_sel_memrepair_peri_START (6)
#define SOC_SCTRL_SCCLKDIV14_sel_memrepair_peri_END (7)
#define SOC_SCTRL_SCCLKDIV14_sel_memrepair_START (12)
#define SOC_SCTRL_SCCLKDIV14_sel_memrepair_END (13)
#define SOC_SCTRL_SCCLKDIV14_sc_gt_clk_ao_camera_START (14)
#define SOC_SCTRL_SCCLKDIV14_sc_gt_clk_ao_camera_END (14)
#define SOC_SCTRL_SCCLKDIV14_sc_gt_clk_memrepair_fll_div_START (15)
#define SOC_SCTRL_SCCLKDIV14_sc_gt_clk_memrepair_fll_div_END (15)
#define SOC_SCTRL_SCCLKDIV14_scclkdiv14_msk_START (16)
#define SOC_SCTRL_SCCLKDIV14_scclkdiv14_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int div_mad_fll : 5;
        unsigned int sel_pll_ctrl_tp_ao : 3;
        unsigned int reserved_0 : 6;
        unsigned int sc_gt_clk_dp_audio_pll_ao : 1;
        unsigned int reserved_1 : 1;
        unsigned int scclkdiv15_msk : 16;
    } reg;
} SOC_SCTRL_SCCLKDIV15_UNION;
#endif
#define SOC_SCTRL_SCCLKDIV15_div_mad_fll_START (0)
#define SOC_SCTRL_SCCLKDIV15_div_mad_fll_END (4)
#define SOC_SCTRL_SCCLKDIV15_sel_pll_ctrl_tp_ao_START (5)
#define SOC_SCTRL_SCCLKDIV15_sel_pll_ctrl_tp_ao_END (7)
#define SOC_SCTRL_SCCLKDIV15_sc_gt_clk_dp_audio_pll_ao_START (14)
#define SOC_SCTRL_SCCLKDIV15_sc_gt_clk_dp_audio_pll_ao_END (14)
#define SOC_SCTRL_SCCLKDIV15_scclkdiv15_msk_START (16)
#define SOC_SCTRL_SCCLKDIV15_scclkdiv15_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved : 4;
        unsigned int div_pcie_aux_32kpll : 6;
        unsigned int sc_gt_clk_pcie_aux_32kpll : 1;
        unsigned int div_memrepair_peri_fll : 4;
        unsigned int sc_gt_clk_memrepair_peri_fll_div : 1;
        unsigned int scclkdiv16_msk : 16;
    } reg;
} SOC_SCTRL_SCCLKDIV16_UNION;
#endif
#define SOC_SCTRL_SCCLKDIV16_div_pcie_aux_32kpll_START (4)
#define SOC_SCTRL_SCCLKDIV16_div_pcie_aux_32kpll_END (9)
#define SOC_SCTRL_SCCLKDIV16_sc_gt_clk_pcie_aux_32kpll_START (10)
#define SOC_SCTRL_SCCLKDIV16_sc_gt_clk_pcie_aux_32kpll_END (10)
#define SOC_SCTRL_SCCLKDIV16_div_memrepair_peri_fll_START (11)
#define SOC_SCTRL_SCCLKDIV16_div_memrepair_peri_fll_END (14)
#define SOC_SCTRL_SCCLKDIV16_sc_gt_clk_memrepair_peri_fll_div_START (15)
#define SOC_SCTRL_SCCLKDIV16_sc_gt_clk_memrepair_peri_fll_div_END (15)
#define SOC_SCTRL_SCCLKDIV16_scclkdiv16_msk_START (16)
#define SOC_SCTRL_SCCLKDIV16_scclkdiv16_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int sel_clk_i2c9 : 4;
        unsigned int gt_clk_ulppll_1_peri_bypass : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 5;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int sc_gt_clk_i2c9 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int scclkdiv17_msk : 16;
    } reg;
} SOC_SCTRL_SCCLKDIV17_UNION;
#endif
#define SOC_SCTRL_SCCLKDIV17_sel_clk_i2c9_START (0)
#define SOC_SCTRL_SCCLKDIV17_sel_clk_i2c9_END (3)
#define SOC_SCTRL_SCCLKDIV17_gt_clk_ulppll_1_peri_bypass_START (4)
#define SOC_SCTRL_SCCLKDIV17_gt_clk_ulppll_1_peri_bypass_END (4)
#define SOC_SCTRL_SCCLKDIV17_sc_gt_clk_i2c9_START (13)
#define SOC_SCTRL_SCCLKDIV17_sc_gt_clk_i2c9_END (13)
#define SOC_SCTRL_SCCLKDIV17_scclkdiv17_msk_START (16)
#define SOC_SCTRL_SCCLKDIV17_scclkdiv17_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int div_i2c9 : 5;
        unsigned int div_memrepair_peri : 6;
        unsigned int sc_gt_clk_memrepair_peri : 1;
        unsigned int reserved : 4;
        unsigned int scclkdiv18_msk : 16;
    } reg;
} SOC_SCTRL_SCCLKDIV18_UNION;
#endif
#define SOC_SCTRL_SCCLKDIV18_div_i2c9_START (0)
#define SOC_SCTRL_SCCLKDIV18_div_i2c9_END (4)
#define SOC_SCTRL_SCCLKDIV18_div_memrepair_peri_START (5)
#define SOC_SCTRL_SCCLKDIV18_div_memrepair_peri_END (10)
#define SOC_SCTRL_SCCLKDIV18_sc_gt_clk_memrepair_peri_START (11)
#define SOC_SCTRL_SCCLKDIV18_sc_gt_clk_memrepair_peri_END (11)
#define SOC_SCTRL_SCCLKDIV18_scclkdiv18_msk_START (16)
#define SOC_SCTRL_SCCLKDIV18_scclkdiv18_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int sc_gt_clk_spi5 : 1;
        unsigned int sc_gt_clk_spi5_ulppll : 1;
        unsigned int div_spi5 : 6;
        unsigned int div_spi5_ulppll : 4;
        unsigned int sel_spi5 : 1;
        unsigned int reserved : 3;
        unsigned int scclkdiv19_msk : 16;
    } reg;
} SOC_SCTRL_SCCLKDIV19_UNION;
#endif
#define SOC_SCTRL_SCCLKDIV19_sc_gt_clk_spi5_START (0)
#define SOC_SCTRL_SCCLKDIV19_sc_gt_clk_spi5_END (0)
#define SOC_SCTRL_SCCLKDIV19_sc_gt_clk_spi5_ulppll_START (1)
#define SOC_SCTRL_SCCLKDIV19_sc_gt_clk_spi5_ulppll_END (1)
#define SOC_SCTRL_SCCLKDIV19_div_spi5_START (2)
#define SOC_SCTRL_SCCLKDIV19_div_spi5_END (7)
#define SOC_SCTRL_SCCLKDIV19_div_spi5_ulppll_START (8)
#define SOC_SCTRL_SCCLKDIV19_div_spi5_ulppll_END (11)
#define SOC_SCTRL_SCCLKDIV19_sel_spi5_START (12)
#define SOC_SCTRL_SCCLKDIV19_sel_spi5_END (12)
#define SOC_SCTRL_SCCLKDIV19_scclkdiv19_msk_START (16)
#define SOC_SCTRL_SCCLKDIV19_scclkdiv19_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int div_dp_audio_pll_ao : 4;
        unsigned int gt_clk_aupll_sscg : 1;
        unsigned int div_blpwm2 : 2;
        unsigned int div_spi3 : 6;
        unsigned int sc_gt_clk_spi3_ulppll : 1;
        unsigned int sc_gt_clk_spi3 : 1;
        unsigned int sel_spi3 : 1;
        unsigned int scclkdiv20_msk : 16;
    } reg;
} SOC_SCTRL_SCCLKDIV20_UNION;
#endif
#define SOC_SCTRL_SCCLKDIV20_div_dp_audio_pll_ao_START (0)
#define SOC_SCTRL_SCCLKDIV20_div_dp_audio_pll_ao_END (3)
#define SOC_SCTRL_SCCLKDIV20_gt_clk_aupll_sscg_START (4)
#define SOC_SCTRL_SCCLKDIV20_gt_clk_aupll_sscg_END (4)
#define SOC_SCTRL_SCCLKDIV20_div_blpwm2_START (5)
#define SOC_SCTRL_SCCLKDIV20_div_blpwm2_END (6)
#define SOC_SCTRL_SCCLKDIV20_div_spi3_START (7)
#define SOC_SCTRL_SCCLKDIV20_div_spi3_END (12)
#define SOC_SCTRL_SCCLKDIV20_sc_gt_clk_spi3_ulppll_START (13)
#define SOC_SCTRL_SCCLKDIV20_sc_gt_clk_spi3_ulppll_END (13)
#define SOC_SCTRL_SCCLKDIV20_sc_gt_clk_spi3_START (14)
#define SOC_SCTRL_SCCLKDIV20_sc_gt_clk_spi3_END (14)
#define SOC_SCTRL_SCCLKDIV20_sel_spi3_START (15)
#define SOC_SCTRL_SCCLKDIV20_sel_spi3_END (15)
#define SOC_SCTRL_SCCLKDIV20_scclkdiv20_msk_START (16)
#define SOC_SCTRL_SCCLKDIV20_scclkdiv20_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int div_spi3_ulppll : 4;
        unsigned int reserved_0 : 6;
        unsigned int reserved_1 : 5;
        unsigned int reserved_2 : 1;
        unsigned int scclkdiv21_msk : 16;
    } reg;
} SOC_SCTRL_SCCLKDIV21_UNION;
#endif
#define SOC_SCTRL_SCCLKDIV21_div_spi3_ulppll_START (0)
#define SOC_SCTRL_SCCLKDIV21_div_spi3_ulppll_END (3)
#define SOC_SCTRL_SCCLKDIV21_scclkdiv21_msk_START (16)
#define SOC_SCTRL_SCCLKDIV21_scclkdiv21_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0 : 6;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 6;
        unsigned int reserved_3 : 3;
        unsigned int scclkdiv22_msk : 16;
    } reg;
} SOC_SCTRL_SCCLKDIV22_UNION;
#endif
#define SOC_SCTRL_SCCLKDIV22_scclkdiv22_msk_START (16)
#define SOC_SCTRL_SCCLKDIV22_scclkdiv22_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int aupll_en_fsm_vote : 5;
        unsigned int reserved_0 : 8;
        unsigned int reserved_1 : 3;
        unsigned int sc_pll_fsm_ns_vote0_msk : 16;
    } reg;
} SOC_SCTRL_SC_PLL_FSM_NS_VOTE0_UNION;
#endif
#define SOC_SCTRL_SC_PLL_FSM_NS_VOTE0_aupll_en_fsm_vote_START (0)
#define SOC_SCTRL_SC_PLL_FSM_NS_VOTE0_aupll_en_fsm_vote_END (4)
#define SOC_SCTRL_SC_PLL_FSM_NS_VOTE0_sc_pll_fsm_ns_vote0_msk_START (16)
#define SOC_SCTRL_SC_PLL_FSM_NS_VOTE0_sc_pll_fsm_ns_vote0_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int sc_efusec_mem_ctrl : 26;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 5;
    } reg;
} SOC_SCTRL_SCPERCTRL0_UNION;
#endif
#define SOC_SCTRL_SCPERCTRL0_sc_efusec_mem_ctrl_START (0)
#define SOC_SCTRL_SCPERCTRL0_sc_efusec_mem_ctrl_END (25)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved : 32;
    } reg;
} SOC_SCTRL_SCPERCTRL1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int mdmpll_extclk_en : 8;
        unsigned int fun_sys_clk_stop_tsp : 1;
        unsigned int reserved_0 : 1;
        unsigned int txp_jtag_bypass : 1;
        unsigned int reserved_1 : 5;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 4;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 6;
    } reg;
} SOC_SCTRL_SCPERCTRL2_UNION;
#endif
#define SOC_SCTRL_SCPERCTRL2_mdmpll_extclk_en_START (0)
#define SOC_SCTRL_SCPERCTRL2_mdmpll_extclk_en_END (7)
#define SOC_SCTRL_SCPERCTRL2_fun_sys_clk_stop_tsp_START (8)
#define SOC_SCTRL_SCPERCTRL2_fun_sys_clk_stop_tsp_END (8)
#define SOC_SCTRL_SCPERCTRL2_txp_jtag_bypass_START (10)
#define SOC_SCTRL_SCPERCTRL2_txp_jtag_bypass_END (10)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int efusec2_timeout_bypass : 1;
        unsigned int repair_hs_sw : 1;
        unsigned int repair_iddq : 1;
        unsigned int repair_hs_sw_1 : 1;
        unsigned int repair_iddq_1 : 1;
        unsigned int reserved_0 : 3;
        unsigned int efusec_timeout_bypass : 1;
        unsigned int bbdrx_timeout_bypass : 1;
        unsigned int sleep_mode_cfgbus_bypass : 1;
        unsigned int sleep_mode_lpmcu_bypass : 1;
        unsigned int first_power_on_flag : 1;
        unsigned int sleep_mode_aobus_bypass : 1;
        unsigned int bbp_clk_en : 1;
        unsigned int reserved_1 : 1;
        unsigned int scperctrl3_msk : 16;
    } reg;
} SOC_SCTRL_SCPERCTRL3_UNION;
#endif
#define SOC_SCTRL_SCPERCTRL3_efusec2_timeout_bypass_START (0)
#define SOC_SCTRL_SCPERCTRL3_efusec2_timeout_bypass_END (0)
#define SOC_SCTRL_SCPERCTRL3_repair_hs_sw_START (1)
#define SOC_SCTRL_SCPERCTRL3_repair_hs_sw_END (1)
#define SOC_SCTRL_SCPERCTRL3_repair_iddq_START (2)
#define SOC_SCTRL_SCPERCTRL3_repair_iddq_END (2)
#define SOC_SCTRL_SCPERCTRL3_repair_hs_sw_1_START (3)
#define SOC_SCTRL_SCPERCTRL3_repair_hs_sw_1_END (3)
#define SOC_SCTRL_SCPERCTRL3_repair_iddq_1_START (4)
#define SOC_SCTRL_SCPERCTRL3_repair_iddq_1_END (4)
#define SOC_SCTRL_SCPERCTRL3_efusec_timeout_bypass_START (8)
#define SOC_SCTRL_SCPERCTRL3_efusec_timeout_bypass_END (8)
#define SOC_SCTRL_SCPERCTRL3_bbdrx_timeout_bypass_START (9)
#define SOC_SCTRL_SCPERCTRL3_bbdrx_timeout_bypass_END (9)
#define SOC_SCTRL_SCPERCTRL3_sleep_mode_cfgbus_bypass_START (10)
#define SOC_SCTRL_SCPERCTRL3_sleep_mode_cfgbus_bypass_END (10)
#define SOC_SCTRL_SCPERCTRL3_sleep_mode_lpmcu_bypass_START (11)
#define SOC_SCTRL_SCPERCTRL3_sleep_mode_lpmcu_bypass_END (11)
#define SOC_SCTRL_SCPERCTRL3_first_power_on_flag_START (12)
#define SOC_SCTRL_SCPERCTRL3_first_power_on_flag_END (12)
#define SOC_SCTRL_SCPERCTRL3_sleep_mode_aobus_bypass_START (13)
#define SOC_SCTRL_SCPERCTRL3_sleep_mode_aobus_bypass_END (13)
#define SOC_SCTRL_SCPERCTRL3_bbp_clk_en_START (14)
#define SOC_SCTRL_SCPERCTRL3_bbp_clk_en_END (14)
#define SOC_SCTRL_SCPERCTRL3_scperctrl3_msk_START (16)
#define SOC_SCTRL_SCPERCTRL3_scperctrl3_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ddr_cha_test_ctrl : 1;
        unsigned int ddr_chb_test_ctrl : 1;
        unsigned int ddr_chc_test_ctrl : 1;
        unsigned int ddr_chd_test_ctrl : 1;
        unsigned int xo_mode_a2d : 1;
        unsigned int reserved_0 : 3;
        unsigned int crc_value : 21;
        unsigned int reserved_1 : 3;
    } reg;
} SOC_SCTRL_SCPERCTRL4_UNION;
#endif
#define SOC_SCTRL_SCPERCTRL4_ddr_cha_test_ctrl_START (0)
#define SOC_SCTRL_SCPERCTRL4_ddr_cha_test_ctrl_END (0)
#define SOC_SCTRL_SCPERCTRL4_ddr_chb_test_ctrl_START (1)
#define SOC_SCTRL_SCPERCTRL4_ddr_chb_test_ctrl_END (1)
#define SOC_SCTRL_SCPERCTRL4_ddr_chc_test_ctrl_START (2)
#define SOC_SCTRL_SCPERCTRL4_ddr_chc_test_ctrl_END (2)
#define SOC_SCTRL_SCPERCTRL4_ddr_chd_test_ctrl_START (3)
#define SOC_SCTRL_SCPERCTRL4_ddr_chd_test_ctrl_END (3)
#define SOC_SCTRL_SCPERCTRL4_xo_mode_a2d_START (4)
#define SOC_SCTRL_SCPERCTRL4_xo_mode_a2d_END (4)
#define SOC_SCTRL_SCPERCTRL4_crc_value_START (8)
#define SOC_SCTRL_SCPERCTRL4_crc_value_END (28)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0 : 4;
        unsigned int ao_hpm_clk_div : 6;
        unsigned int ao_hpm_en : 1;
        unsigned int ao_hpmx_en : 1;
        unsigned int ao_hpmx_opc_min_en : 1;
        unsigned int ao_hpm_opc_min_en : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int scperctrl5_msk : 16;
    } reg;
} SOC_SCTRL_SCPERCTRL5_UNION;
#endif
#define SOC_SCTRL_SCPERCTRL5_ao_hpm_clk_div_START (4)
#define SOC_SCTRL_SCPERCTRL5_ao_hpm_clk_div_END (9)
#define SOC_SCTRL_SCPERCTRL5_ao_hpm_en_START (10)
#define SOC_SCTRL_SCPERCTRL5_ao_hpm_en_END (10)
#define SOC_SCTRL_SCPERCTRL5_ao_hpmx_en_START (11)
#define SOC_SCTRL_SCPERCTRL5_ao_hpmx_en_END (11)
#define SOC_SCTRL_SCPERCTRL5_ao_hpmx_opc_min_en_START (12)
#define SOC_SCTRL_SCPERCTRL5_ao_hpmx_opc_min_en_END (12)
#define SOC_SCTRL_SCPERCTRL5_ao_hpm_opc_min_en_START (13)
#define SOC_SCTRL_SCPERCTRL5_ao_hpm_opc_min_en_END (13)
#define SOC_SCTRL_SCPERCTRL5_scperctrl5_msk_START (16)
#define SOC_SCTRL_SCPERCTRL5_scperctrl5_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int peri_fix_io_ret : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 2;
        unsigned int peri_bootio_ret : 1;
        unsigned int peri_bootio_ret_mode : 1;
        unsigned int reserved_2 : 2;
        unsigned int reserved_3 : 2;
        unsigned int reserved_4 : 2;
        unsigned int peri_bootio_ret_hw_enter_bypass : 1;
        unsigned int reserved_5 : 7;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 8;
    } reg;
} SOC_SCTRL_SCPERCTRL6_UNION;
#endif
#define SOC_SCTRL_SCPERCTRL6_peri_fix_io_ret_START (0)
#define SOC_SCTRL_SCPERCTRL6_peri_fix_io_ret_END (0)
#define SOC_SCTRL_SCPERCTRL6_peri_bootio_ret_START (4)
#define SOC_SCTRL_SCPERCTRL6_peri_bootio_ret_END (4)
#define SOC_SCTRL_SCPERCTRL6_peri_bootio_ret_mode_START (5)
#define SOC_SCTRL_SCPERCTRL6_peri_bootio_ret_mode_END (5)
#define SOC_SCTRL_SCPERCTRL6_peri_bootio_ret_hw_enter_bypass_START (12)
#define SOC_SCTRL_SCPERCTRL6_peri_bootio_ret_hw_enter_bypass_END (12)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int sctrl2lm_hard_volt : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int usim_od_en : 1;
        unsigned int sel_suspend_ao2syscache : 1;
        unsigned int reserved_6 : 1;
        unsigned int sleep_mode_to_peri_bypass : 1;
        unsigned int sleep_mode_to_hsdt_bypass : 1;
        unsigned int sleep_mode_to_cpu_crg_bypass : 1;
        unsigned int reserved_7 : 1;
        unsigned int sc_peri_nonboot_io_ret : 1;
        unsigned int pmu_powerhold_protect : 1;
        unsigned int scperctrl7_msk : 16;
    } reg;
} SOC_SCTRL_SCPERCTRL7_UNION;
#endif
#define SOC_SCTRL_SCPERCTRL7_sctrl2lm_hard_volt_START (0)
#define SOC_SCTRL_SCPERCTRL7_sctrl2lm_hard_volt_END (0)
#define SOC_SCTRL_SCPERCTRL7_usim_od_en_START (7)
#define SOC_SCTRL_SCPERCTRL7_usim_od_en_END (7)
#define SOC_SCTRL_SCPERCTRL7_sel_suspend_ao2syscache_START (8)
#define SOC_SCTRL_SCPERCTRL7_sel_suspend_ao2syscache_END (8)
#define SOC_SCTRL_SCPERCTRL7_sleep_mode_to_peri_bypass_START (10)
#define SOC_SCTRL_SCPERCTRL7_sleep_mode_to_peri_bypass_END (10)
#define SOC_SCTRL_SCPERCTRL7_sleep_mode_to_hsdt_bypass_START (11)
#define SOC_SCTRL_SCPERCTRL7_sleep_mode_to_hsdt_bypass_END (11)
#define SOC_SCTRL_SCPERCTRL7_sleep_mode_to_cpu_crg_bypass_START (12)
#define SOC_SCTRL_SCPERCTRL7_sleep_mode_to_cpu_crg_bypass_END (12)
#define SOC_SCTRL_SCPERCTRL7_sc_peri_nonboot_io_ret_START (14)
#define SOC_SCTRL_SCPERCTRL7_sc_peri_nonboot_io_ret_END (14)
#define SOC_SCTRL_SCPERCTRL7_pmu_powerhold_protect_START (15)
#define SOC_SCTRL_SCPERCTRL7_pmu_powerhold_protect_END (15)
#define SOC_SCTRL_SCPERCTRL7_scperctrl7_msk_START (16)
#define SOC_SCTRL_SCPERCTRL7_scperctrl7_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int rst_apb_bbp_on : 1;
        unsigned int rst_19m_bbp_on : 1;
        unsigned int rst_32k_bbp_on : 1;
        unsigned int rst_104m_gbbp1_on : 1;
        unsigned int rst_104m_gbbp2_on : 1;
        unsigned int rst_32k_timer_on : 1;
        unsigned int rst_ahb_bbp_on_n : 1;
        unsigned int bbp_on_ctrl : 25;
    } reg;
} SOC_SCTRL_SCPERCTRL9_UNION;
#endif
#define SOC_SCTRL_SCPERCTRL9_rst_apb_bbp_on_START (0)
#define SOC_SCTRL_SCPERCTRL9_rst_apb_bbp_on_END (0)
#define SOC_SCTRL_SCPERCTRL9_rst_19m_bbp_on_START (1)
#define SOC_SCTRL_SCPERCTRL9_rst_19m_bbp_on_END (1)
#define SOC_SCTRL_SCPERCTRL9_rst_32k_bbp_on_START (2)
#define SOC_SCTRL_SCPERCTRL9_rst_32k_bbp_on_END (2)
#define SOC_SCTRL_SCPERCTRL9_rst_104m_gbbp1_on_START (3)
#define SOC_SCTRL_SCPERCTRL9_rst_104m_gbbp1_on_END (3)
#define SOC_SCTRL_SCPERCTRL9_rst_104m_gbbp2_on_START (4)
#define SOC_SCTRL_SCPERCTRL9_rst_104m_gbbp2_on_END (4)
#define SOC_SCTRL_SCPERCTRL9_rst_32k_timer_on_START (5)
#define SOC_SCTRL_SCPERCTRL9_rst_32k_timer_on_END (5)
#define SOC_SCTRL_SCPERCTRL9_rst_ahb_bbp_on_n_START (6)
#define SOC_SCTRL_SCPERCTRL9_rst_ahb_bbp_on_n_END (6)
#define SOC_SCTRL_SCPERCTRL9_bbp_on_ctrl_START (7)
#define SOC_SCTRL_SCPERCTRL9_bbp_on_ctrl_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ddra_acctl_ioctrl_henl_in : 1;
        unsigned int ddrb_acctl_ioctrl_henl_in : 1;
        unsigned int ddrc_acctl_ioctrl_henl_in : 1;
        unsigned int ddrd_acctl_ioctrl_henl_in : 1;
        unsigned int ddra_pll_en_lat : 1;
        unsigned int ddrb_pll_en_lat : 1;
        unsigned int ddrc_pll_en_lat : 1;
        unsigned int ddrd_pll_en_lat : 1;
        unsigned int reserved_0 : 10;
        unsigned int reserved_1 : 2;
        unsigned int reserved_2 : 4;
        unsigned int reserved_3 : 2;
        unsigned int reserved_4 : 2;
        unsigned int reserved_5 : 4;
    } reg;
} SOC_SCTRL_SCPERCTRL10_UNION;
#endif
#define SOC_SCTRL_SCPERCTRL10_ddra_acctl_ioctrl_henl_in_START (0)
#define SOC_SCTRL_SCPERCTRL10_ddra_acctl_ioctrl_henl_in_END (0)
#define SOC_SCTRL_SCPERCTRL10_ddrb_acctl_ioctrl_henl_in_START (1)
#define SOC_SCTRL_SCPERCTRL10_ddrb_acctl_ioctrl_henl_in_END (1)
#define SOC_SCTRL_SCPERCTRL10_ddrc_acctl_ioctrl_henl_in_START (2)
#define SOC_SCTRL_SCPERCTRL10_ddrc_acctl_ioctrl_henl_in_END (2)
#define SOC_SCTRL_SCPERCTRL10_ddrd_acctl_ioctrl_henl_in_START (3)
#define SOC_SCTRL_SCPERCTRL10_ddrd_acctl_ioctrl_henl_in_END (3)
#define SOC_SCTRL_SCPERCTRL10_ddra_pll_en_lat_START (4)
#define SOC_SCTRL_SCPERCTRL10_ddra_pll_en_lat_END (4)
#define SOC_SCTRL_SCPERCTRL10_ddrb_pll_en_lat_START (5)
#define SOC_SCTRL_SCPERCTRL10_ddrb_pll_en_lat_END (5)
#define SOC_SCTRL_SCPERCTRL10_ddrc_pll_en_lat_START (6)
#define SOC_SCTRL_SCPERCTRL10_ddrc_pll_en_lat_END (6)
#define SOC_SCTRL_SCPERCTRL10_ddrd_pll_en_lat_START (7)
#define SOC_SCTRL_SCPERCTRL10_ddrd_pll_en_lat_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0: 1;
        unsigned int reserved_1: 31;
    } reg;
} SOC_SCTRL_SCPERCTRL11_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0: 6;
        unsigned int reserved_1: 2;
        unsigned int reserved_2: 3;
        unsigned int reserved_3: 21;
    } reg;
} SOC_SCTRL_SCPERCTRL12_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0: 1;
        unsigned int reserved_1: 1;
        unsigned int reserved_2: 30;
    } reg;
} SOC_SCTRL_SCPERCTRL13_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved : 32;
    } reg;
} SOC_SCTRL_SCPERCTRL14_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved : 32;
    } reg;
} SOC_SCTRL_SCPERCTRL15_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int avs_en0 : 1;
        unsigned int avs_en1 : 1;
        unsigned int avs_en2 : 1;
        unsigned int avs_en3 : 1;
        unsigned int avs_en4 : 1;
        unsigned int avs_en5 : 1;
        unsigned int avs_en6 : 1;
        unsigned int avs_en7 : 1;
        unsigned int avs_en8 : 1;
        unsigned int avs_en9 : 1;
        unsigned int avs_en10 : 1;
        unsigned int avs_en11 : 1;
        unsigned int avs_en12 : 1;
        unsigned int avs_en13 : 1;
        unsigned int avs_en14 : 1;
        unsigned int avs_en15 : 1;
        unsigned int scperctrl16_msk : 16;
    } reg;
} SOC_SCTRL_SCPERCTRL16_UNION;
#endif
#define SOC_SCTRL_SCPERCTRL16_avs_en0_START (0)
#define SOC_SCTRL_SCPERCTRL16_avs_en0_END (0)
#define SOC_SCTRL_SCPERCTRL16_avs_en1_START (1)
#define SOC_SCTRL_SCPERCTRL16_avs_en1_END (1)
#define SOC_SCTRL_SCPERCTRL16_avs_en2_START (2)
#define SOC_SCTRL_SCPERCTRL16_avs_en2_END (2)
#define SOC_SCTRL_SCPERCTRL16_avs_en3_START (3)
#define SOC_SCTRL_SCPERCTRL16_avs_en3_END (3)
#define SOC_SCTRL_SCPERCTRL16_avs_en4_START (4)
#define SOC_SCTRL_SCPERCTRL16_avs_en4_END (4)
#define SOC_SCTRL_SCPERCTRL16_avs_en5_START (5)
#define SOC_SCTRL_SCPERCTRL16_avs_en5_END (5)
#define SOC_SCTRL_SCPERCTRL16_avs_en6_START (6)
#define SOC_SCTRL_SCPERCTRL16_avs_en6_END (6)
#define SOC_SCTRL_SCPERCTRL16_avs_en7_START (7)
#define SOC_SCTRL_SCPERCTRL16_avs_en7_END (7)
#define SOC_SCTRL_SCPERCTRL16_avs_en8_START (8)
#define SOC_SCTRL_SCPERCTRL16_avs_en8_END (8)
#define SOC_SCTRL_SCPERCTRL16_avs_en9_START (9)
#define SOC_SCTRL_SCPERCTRL16_avs_en9_END (9)
#define SOC_SCTRL_SCPERCTRL16_avs_en10_START (10)
#define SOC_SCTRL_SCPERCTRL16_avs_en10_END (10)
#define SOC_SCTRL_SCPERCTRL16_avs_en11_START (11)
#define SOC_SCTRL_SCPERCTRL16_avs_en11_END (11)
#define SOC_SCTRL_SCPERCTRL16_avs_en12_START (12)
#define SOC_SCTRL_SCPERCTRL16_avs_en12_END (12)
#define SOC_SCTRL_SCPERCTRL16_avs_en13_START (13)
#define SOC_SCTRL_SCPERCTRL16_avs_en13_END (13)
#define SOC_SCTRL_SCPERCTRL16_avs_en14_START (14)
#define SOC_SCTRL_SCPERCTRL16_avs_en14_END (14)
#define SOC_SCTRL_SCPERCTRL16_avs_en15_START (15)
#define SOC_SCTRL_SCPERCTRL16_avs_en15_END (15)
#define SOC_SCTRL_SCPERCTRL16_scperctrl16_msk_START (16)
#define SOC_SCTRL_SCPERCTRL16_scperctrl16_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int bisr_reserve_in : 4;
        unsigned int dbg_repair_frm_efu : 1;
        unsigned int dbg_repair_frm_efu_1 : 1;
        unsigned int reserved_0 : 1;
        unsigned int sel_suspend_ao2syscache_lpmcu_invar_inuse : 1;
        unsigned int sel_suspend_ao2syscache_ddr_inuse : 1;
        unsigned int sel_suspend_ao2syscache_sysbus_inuse : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 2;
        unsigned int mddrc_syscache_retention_flag : 1;
        unsigned int hifi_load_image_flag : 1;
        unsigned int asp_imgld_flag : 1;
        unsigned int scperctrl17_msk : 16;
    } reg;
} SOC_SCTRL_SCPERCTRL17_UNION;
#endif
#define SOC_SCTRL_SCPERCTRL17_bisr_reserve_in_START (0)
#define SOC_SCTRL_SCPERCTRL17_bisr_reserve_in_END (3)
#define SOC_SCTRL_SCPERCTRL17_dbg_repair_frm_efu_START (4)
#define SOC_SCTRL_SCPERCTRL17_dbg_repair_frm_efu_END (4)
#define SOC_SCTRL_SCPERCTRL17_dbg_repair_frm_efu_1_START (5)
#define SOC_SCTRL_SCPERCTRL17_dbg_repair_frm_efu_1_END (5)
#define SOC_SCTRL_SCPERCTRL17_sel_suspend_ao2syscache_lpmcu_invar_inuse_START (7)
#define SOC_SCTRL_SCPERCTRL17_sel_suspend_ao2syscache_lpmcu_invar_inuse_END (7)
#define SOC_SCTRL_SCPERCTRL17_sel_suspend_ao2syscache_ddr_inuse_START (8)
#define SOC_SCTRL_SCPERCTRL17_sel_suspend_ao2syscache_ddr_inuse_END (8)
#define SOC_SCTRL_SCPERCTRL17_sel_suspend_ao2syscache_sysbus_inuse_START (9)
#define SOC_SCTRL_SCPERCTRL17_sel_suspend_ao2syscache_sysbus_inuse_END (9)
#define SOC_SCTRL_SCPERCTRL17_mddrc_syscache_retention_flag_START (13)
#define SOC_SCTRL_SCPERCTRL17_mddrc_syscache_retention_flag_END (13)
#define SOC_SCTRL_SCPERCTRL17_hifi_load_image_flag_START (14)
#define SOC_SCTRL_SCPERCTRL17_hifi_load_image_flag_END (14)
#define SOC_SCTRL_SCPERCTRL17_asp_imgld_flag_START (15)
#define SOC_SCTRL_SCPERCTRL17_asp_imgld_flag_END (15)
#define SOC_SCTRL_SCPERCTRL17_scperctrl17_msk_START (16)
#define SOC_SCTRL_SCPERCTRL17_scperctrl17_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int intr_stat_asp_nonidle_pend : 1;
        unsigned int intr_stat_iomcu_nonidle_pend : 1;
        unsigned int intr_stat_hisee_nonidle_pend : 1;
        unsigned int intr_stat_hieps_nonidle_pend : 1;
        unsigned int intr_stat_hieps_ree_nonidle_pend : 1;
        unsigned int reserved_0 : 2;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 3;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 2;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 15;
    } reg;
} SOC_SCTRL_SC_INTR_NOCBUS_STAT_UNION;
#endif
#define SOC_SCTRL_SC_INTR_NOCBUS_STAT_intr_stat_asp_nonidle_pend_START (0)
#define SOC_SCTRL_SC_INTR_NOCBUS_STAT_intr_stat_asp_nonidle_pend_END (0)
#define SOC_SCTRL_SC_INTR_NOCBUS_STAT_intr_stat_iomcu_nonidle_pend_START (1)
#define SOC_SCTRL_SC_INTR_NOCBUS_STAT_intr_stat_iomcu_nonidle_pend_END (1)
#define SOC_SCTRL_SC_INTR_NOCBUS_STAT_intr_stat_hisee_nonidle_pend_START (2)
#define SOC_SCTRL_SC_INTR_NOCBUS_STAT_intr_stat_hisee_nonidle_pend_END (2)
#define SOC_SCTRL_SC_INTR_NOCBUS_STAT_intr_stat_hieps_nonidle_pend_START (3)
#define SOC_SCTRL_SC_INTR_NOCBUS_STAT_intr_stat_hieps_nonidle_pend_END (3)
#define SOC_SCTRL_SC_INTR_NOCBUS_STAT_intr_stat_hieps_ree_nonidle_pend_START (4)
#define SOC_SCTRL_SC_INTR_NOCBUS_STAT_intr_stat_hieps_ree_nonidle_pend_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0 : 1;
        unsigned int autodiv_ufs_subsys_stat : 1;
        unsigned int reserved_1 : 2;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 2;
        unsigned int ufsbus_idle_flag : 1;
        unsigned int noc_aobus_idle_flag : 1;
        unsigned int reserved_4 : 1;
        unsigned int aupll_fsm_wsat : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int reserved_11 : 15;
    } reg;
} SOC_SCTRL_SCPERSTATUS17_UNION;
#endif
#define SOC_SCTRL_SCPERSTATUS17_autodiv_ufs_subsys_stat_START (1)
#define SOC_SCTRL_SCPERSTATUS17_autodiv_ufs_subsys_stat_END (1)
#define SOC_SCTRL_SCPERSTATUS17_ufsbus_idle_flag_START (7)
#define SOC_SCTRL_SCPERSTATUS17_ufsbus_idle_flag_END (7)
#define SOC_SCTRL_SCPERSTATUS17_noc_aobus_idle_flag_START (8)
#define SOC_SCTRL_SCPERSTATUS17_noc_aobus_idle_flag_END (8)
#define SOC_SCTRL_SCPERSTATUS17_aupll_fsm_wsat_START (10)
#define SOC_SCTRL_SCPERSTATUS17_aupll_fsm_wsat_END (10)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int sw_ack_clk_memrepair_sw : 3;
        unsigned int sw_ack_clk_aobus_ini : 2;
        unsigned int swdone_clk_asp_codec_pll_div : 1;
        unsigned int reserved_0 : 1;
        unsigned int swdone_clk_ufs_subsys_sys_div : 1;
        unsigned int swdone_clk_ufs_subsys_pll_div : 1;
        unsigned int sw_ack_clk_ufs_subsys_sw : 3;
        unsigned int swdone_clk_spmi_mst : 1;
        unsigned int reserved_1 : 2;
        unsigned int swdone_clk_ao_camera_div : 1;
        unsigned int reserved_2 : 1;
        unsigned int st_pclk_spmi : 1;
        unsigned int sw_ack_clk_hsdt_subsys_sw : 3;
        unsigned int swdone_clk_hsdt_subsys_div : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 2;
        unsigned int sw_ack_clk_memrepair_peri_sw : 3;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 2;
    } reg;
} SOC_SCTRL_SCPERSTATUS16_UNION;
#endif
#define SOC_SCTRL_SCPERSTATUS16_sw_ack_clk_memrepair_sw_START (0)
#define SOC_SCTRL_SCPERSTATUS16_sw_ack_clk_memrepair_sw_END (2)
#define SOC_SCTRL_SCPERSTATUS16_sw_ack_clk_aobus_ini_START (3)
#define SOC_SCTRL_SCPERSTATUS16_sw_ack_clk_aobus_ini_END (4)
#define SOC_SCTRL_SCPERSTATUS16_swdone_clk_asp_codec_pll_div_START (5)
#define SOC_SCTRL_SCPERSTATUS16_swdone_clk_asp_codec_pll_div_END (5)
#define SOC_SCTRL_SCPERSTATUS16_swdone_clk_ufs_subsys_sys_div_START (7)
#define SOC_SCTRL_SCPERSTATUS16_swdone_clk_ufs_subsys_sys_div_END (7)
#define SOC_SCTRL_SCPERSTATUS16_swdone_clk_ufs_subsys_pll_div_START (8)
#define SOC_SCTRL_SCPERSTATUS16_swdone_clk_ufs_subsys_pll_div_END (8)
#define SOC_SCTRL_SCPERSTATUS16_sw_ack_clk_ufs_subsys_sw_START (9)
#define SOC_SCTRL_SCPERSTATUS16_sw_ack_clk_ufs_subsys_sw_END (11)
#define SOC_SCTRL_SCPERSTATUS16_swdone_clk_spmi_mst_START (12)
#define SOC_SCTRL_SCPERSTATUS16_swdone_clk_spmi_mst_END (12)
#define SOC_SCTRL_SCPERSTATUS16_swdone_clk_ao_camera_div_START (15)
#define SOC_SCTRL_SCPERSTATUS16_swdone_clk_ao_camera_div_END (15)
#define SOC_SCTRL_SCPERSTATUS16_st_pclk_spmi_START (17)
#define SOC_SCTRL_SCPERSTATUS16_st_pclk_spmi_END (17)
#define SOC_SCTRL_SCPERSTATUS16_sw_ack_clk_hsdt_subsys_sw_START (18)
#define SOC_SCTRL_SCPERSTATUS16_sw_ack_clk_hsdt_subsys_sw_END (20)
#define SOC_SCTRL_SCPERSTATUS16_swdone_clk_hsdt_subsys_div_START (21)
#define SOC_SCTRL_SCPERSTATUS16_swdone_clk_hsdt_subsys_div_END (21)
#define SOC_SCTRL_SCPERSTATUS16_sw_ack_clk_memrepair_peri_sw_START (25)
#define SOC_SCTRL_SCPERSTATUS16_sw_ack_clk_memrepair_peri_sw_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 4;
        unsigned int reserved_2 : 3;
        unsigned int reserved_3 : 3;
        unsigned int reserved_4 : 10;
        unsigned int reserved_5 : 10;
        unsigned int bbp_apb_bus_slv_stat : 1;
    } reg;
} SOC_SCTRL_SCPERSTATUS0_UNION;
#endif
#define SOC_SCTRL_SCPERSTATUS0_bbp_apb_bus_slv_stat_START (31)
#define SOC_SCTRL_SCPERSTATUS0_bbp_apb_bus_slv_stat_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int bisr_reserve_out : 8;
        unsigned int reserved_0 : 4;
        unsigned int reserved_1 : 20;
    } reg;
} SOC_SCTRL_SCPERSTATUS1_UNION;
#endif
#define SOC_SCTRL_SCPERSTATUS1_bisr_reserve_out_START (0)
#define SOC_SCTRL_SCPERSTATUS1_bisr_reserve_out_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0: 10;
        unsigned int reserved_1: 10;
        unsigned int reserved_2: 10;
        unsigned int reserved_3: 2;
    } reg;
} SOC_SCTRL_SCPERSTATUS2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved : 32;
    } reg;
} SOC_SCTRL_SCPERSTATUS3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int bbdrx_timeout_dbg_info : 32;
    } reg;
} SOC_SCTRL_SCPERSTATUS4_UNION;
#endif
#define SOC_SCTRL_SCPERSTATUS4_bbdrx_timeout_dbg_info_START (0)
#define SOC_SCTRL_SCPERSTATUS4_bbdrx_timeout_dbg_info_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0 : 10;
        unsigned int reserved_1 : 10;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int mdm_tsp_dbg_ack : 1;
        unsigned int reserved_6 : 1;
        unsigned int repair_finish : 1;
        unsigned int reserved_7 : 1;
        unsigned int cfg_ispa7_dbgen_dx : 4;
    } reg;
} SOC_SCTRL_SCPERSTATUS5_UNION;
#endif
#define SOC_SCTRL_SCPERSTATUS5_mdm_tsp_dbg_ack_START (24)
#define SOC_SCTRL_SCPERSTATUS5_mdm_tsp_dbg_ack_END (24)
#define SOC_SCTRL_SCPERSTATUS5_repair_finish_START (26)
#define SOC_SCTRL_SCPERSTATUS5_repair_finish_END (26)
#define SOC_SCTRL_SCPERSTATUS5_cfg_ispa7_dbgen_dx_START (28)
#define SOC_SCTRL_SCPERSTATUS5_cfg_ispa7_dbgen_dx_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int reserved_11 : 1;
        unsigned int reserved_12 : 1;
        unsigned int reserved_13 : 1;
        unsigned int reserved_14 : 1;
        unsigned int reserved_15 : 1;
        unsigned int sc_cfg_arm_dbgen_dx : 4;
        unsigned int sc_cfg_mcu_dbgen_dx : 4;
        unsigned int audio_mmbuf_ctrl_to_sctrl : 8;
    } reg;
} SOC_SCTRL_SCPERSTATUS6_UNION;
#endif
#define SOC_SCTRL_SCPERSTATUS6_sc_cfg_arm_dbgen_dx_START (16)
#define SOC_SCTRL_SCPERSTATUS6_sc_cfg_arm_dbgen_dx_END (19)
#define SOC_SCTRL_SCPERSTATUS6_sc_cfg_mcu_dbgen_dx_START (20)
#define SOC_SCTRL_SCPERSTATUS6_sc_cfg_mcu_dbgen_dx_END (23)
#define SOC_SCTRL_SCPERSTATUS6_audio_mmbuf_ctrl_to_sctrl_START (24)
#define SOC_SCTRL_SCPERSTATUS6_audio_mmbuf_ctrl_to_sctrl_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0: 1;
        unsigned int reserved_1: 1;
        unsigned int reserved_2: 1;
        unsigned int reserved_3: 1;
        unsigned int reserved_4: 1;
        unsigned int reserved_5: 1;
        unsigned int reserved_6: 1;
        unsigned int reserved_7: 1;
        unsigned int reserved_8: 1;
        unsigned int reserved_9: 1;
        unsigned int reserved_10: 1;
        unsigned int reserved_11: 1;
        unsigned int reserved_12: 1;
        unsigned int reserved_13: 1;
        unsigned int reserved_14: 1;
        unsigned int reserved_15: 1;
        unsigned int reserved_16: 1;
        unsigned int reserved_17: 1;
        unsigned int reserved_18: 1;
        unsigned int reserved_19: 1;
        unsigned int reserved_20: 1;
        unsigned int reserved_21: 1;
        unsigned int reserved_22: 1;
        unsigned int reserved_23: 1;
        unsigned int reserved_24: 8;
    } reg;
} SOC_SCTRL_SCPERSTATUS7_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0: 1;
        unsigned int reserved_1: 1;
        unsigned int reserved_2: 1;
        unsigned int reserved_3: 1;
        unsigned int reserved_4: 1;
        unsigned int reserved_5: 1;
        unsigned int reserved_6: 1;
        unsigned int reserved_7: 1;
        unsigned int reserved_8: 1;
        unsigned int reserved_9: 6;
        unsigned int reserved_10: 1;
        unsigned int reserved_11: 1;
        unsigned int reserved_12: 1;
        unsigned int reserved_13: 14;
    } reg;
} SOC_SCTRL_SCPERSTATUS8_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0: 1;
        unsigned int reserved_1: 1;
        unsigned int reserved_2: 1;
        unsigned int reserved_3: 1;
        unsigned int reserved_4: 1;
        unsigned int reserved_5: 1;
        unsigned int reserved_6: 1;
        unsigned int reserved_7: 1;
        unsigned int reserved_8: 1;
        unsigned int reserved_9: 1;
        unsigned int reserved_10: 1;
        unsigned int reserved_11: 1;
        unsigned int reserved_12: 1;
        unsigned int reserved_13: 1;
        unsigned int reserved_14: 1;
        unsigned int reserved_15: 1;
        unsigned int reserved_16: 1;
        unsigned int reserved_17: 1;
        unsigned int reserved_18: 1;
        unsigned int reserved_19: 1;
        unsigned int reserved_20: 1;
        unsigned int reserved_21: 1;
        unsigned int reserved_22: 1;
        unsigned int reserved_23: 1;
        unsigned int reserved_24: 1;
        unsigned int reserved_25: 7;
    } reg;
} SOC_SCTRL_SCPERSTATUS9_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int iomcu_arc_watchdog_reset : 1;
        unsigned int pcie0_100ms_idle_timeout_intr : 1;
        unsigned int pcie1_100ms_idle_timeout_intr : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int iomcu_core_stalled : 1;
        unsigned int iomcu_sys_tf_half_r : 1;
        unsigned int reserved_4 : 13;
        unsigned int reserved_5 : 10;
    } reg;
} SOC_SCTRL_SCPERSTATUS10_UNION;
#endif
#define SOC_SCTRL_SCPERSTATUS10_iomcu_arc_watchdog_reset_START (0)
#define SOC_SCTRL_SCPERSTATUS10_iomcu_arc_watchdog_reset_END (0)
#define SOC_SCTRL_SCPERSTATUS10_pcie0_100ms_idle_timeout_intr_START (1)
#define SOC_SCTRL_SCPERSTATUS10_pcie0_100ms_idle_timeout_intr_END (1)
#define SOC_SCTRL_SCPERSTATUS10_pcie1_100ms_idle_timeout_intr_START (2)
#define SOC_SCTRL_SCPERSTATUS10_pcie1_100ms_idle_timeout_intr_END (2)
#define SOC_SCTRL_SCPERSTATUS10_iomcu_core_stalled_START (7)
#define SOC_SCTRL_SCPERSTATUS10_iomcu_core_stalled_END (7)
#define SOC_SCTRL_SCPERSTATUS10_iomcu_sys_tf_half_r_START (8)
#define SOC_SCTRL_SCPERSTATUS10_iomcu_sys_tf_half_r_END (8)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0: 10;
        unsigned int reserved_1: 10;
        unsigned int reserved_2: 10;
        unsigned int reserved_3: 2;
    } reg;
} SOC_SCTRL_SCPERSTATUS11_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int intr_wakeup_swing_enter_req : 1;
        unsigned int intr_wakeup_swing_exit_req : 1;
        unsigned int reserved_4 : 26;
    } reg;
} SOC_SCTRL_SCPERSTATUS12_UNION;
#endif
#define SOC_SCTRL_SCPERSTATUS12_intr_wakeup_swing_enter_req_START (4)
#define SOC_SCTRL_SCPERSTATUS12_intr_wakeup_swing_enter_req_END (4)
#define SOC_SCTRL_SCPERSTATUS12_intr_wakeup_swing_exit_req_START (5)
#define SOC_SCTRL_SCPERSTATUS12_intr_wakeup_swing_exit_req_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ao_hpm_opc : 10;
        unsigned int ao_hpm_opc_vld : 1;
        unsigned int reserved_0 : 1;
        unsigned int ao_hpmx_opc : 10;
        unsigned int ao_hpmx_opc_vld : 1;
        unsigned int reserved_1 : 9;
    } reg;
} SOC_SCTRL_SCPERSTATUS13_UNION;
#endif
#define SOC_SCTRL_SCPERSTATUS13_ao_hpm_opc_START (0)
#define SOC_SCTRL_SCPERSTATUS13_ao_hpm_opc_END (9)
#define SOC_SCTRL_SCPERSTATUS13_ao_hpm_opc_vld_START (10)
#define SOC_SCTRL_SCPERSTATUS13_ao_hpm_opc_vld_END (10)
#define SOC_SCTRL_SCPERSTATUS13_ao_hpmx_opc_START (12)
#define SOC_SCTRL_SCPERSTATUS13_ao_hpmx_opc_END (21)
#define SOC_SCTRL_SCPERSTATUS13_ao_hpmx_opc_vld_START (22)
#define SOC_SCTRL_SCPERSTATUS13_ao_hpmx_opc_vld_END (22)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0 : 10;
        unsigned int ao_hpm_opc_min : 10;
        unsigned int ao_hpmx_opc_min : 10;
        unsigned int reserved_1 : 2;
    } reg;
} SOC_SCTRL_SCPERSTATUS14_UNION;
#endif
#define SOC_SCTRL_SCPERSTATUS14_ao_hpm_opc_min_START (10)
#define SOC_SCTRL_SCPERSTATUS14_ao_hpm_opc_min_END (19)
#define SOC_SCTRL_SCPERSTATUS14_ao_hpmx_opc_min_START (20)
#define SOC_SCTRL_SCPERSTATUS14_ao_hpmx_opc_min_END (29)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ddra_reset_state : 1;
        unsigned int ddrb_reset_state : 1;
        unsigned int ddrc_reset_state : 1;
        unsigned int ddrd_reset_state : 1;
        unsigned int reserved_0 : 12;
        unsigned int reserved_1 : 16;
    } reg;
} SOC_SCTRL_SCPERSTATUS15_UNION;
#endif
#define SOC_SCTRL_SCPERSTATUS15_ddra_reset_state_START (0)
#define SOC_SCTRL_SCPERSTATUS15_ddra_reset_state_END (0)
#define SOC_SCTRL_SCPERSTATUS15_ddrb_reset_state_START (1)
#define SOC_SCTRL_SCPERSTATUS15_ddrb_reset_state_END (1)
#define SOC_SCTRL_SCPERSTATUS15_ddrc_reset_state_START (2)
#define SOC_SCTRL_SCPERSTATUS15_ddrc_reset_state_END (2)
#define SOC_SCTRL_SCPERSTATUS15_ddrd_reset_state_START (3)
#define SOC_SCTRL_SCPERSTATUS15_ddrd_reset_state_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int memory_repair_bypass : 1;
        unsigned int hardw_ctrl_sel0 : 1;
        unsigned int reserved_0 : 2;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 7;
        unsigned int reserved_5 : 1;
        unsigned int emmc_ufs_sel : 1;
        unsigned int ate_mode : 5;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 6;
        unsigned int reserved_8 : 4;
    } reg;
} SOC_SCTRL_SCINNERSTAT_UNION;
#endif
#define SOC_SCTRL_SCINNERSTAT_memory_repair_bypass_START (0)
#define SOC_SCTRL_SCINNERSTAT_memory_repair_bypass_END (0)
#define SOC_SCTRL_SCINNERSTAT_hardw_ctrl_sel0_START (1)
#define SOC_SCTRL_SCINNERSTAT_hardw_ctrl_sel0_END (1)
#define SOC_SCTRL_SCINNERSTAT_emmc_ufs_sel_START (15)
#define SOC_SCTRL_SCINNERSTAT_emmc_ufs_sel_END (15)
#define SOC_SCTRL_SCINNERSTAT_ate_mode_START (16)
#define SOC_SCTRL_SCINNERSTAT_ate_mode_END (20)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int mali_dbgack_byp : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_SCTRL_SCINNERCTRL_UNION;
#endif
#define SOC_SCTRL_SCINNERCTRL_mali_dbgack_byp_START (0)
#define SOC_SCTRL_SCINNERCTRL_mali_dbgack_byp_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0: 1;
        unsigned int reserved_1: 1;
        unsigned int reserved_2: 1;
        unsigned int reserved_3: 1;
        unsigned int reserved_4: 1;
        unsigned int reserved_5: 1;
        unsigned int reserved_6: 1;
        unsigned int reserved_7: 1;
        unsigned int reserved_8: 1;
        unsigned int reserved_9: 1;
        unsigned int reserved_10: 1;
        unsigned int reserved_11: 1;
        unsigned int reserved_12: 1;
        unsigned int reserved_13: 1;
        unsigned int reserved_14: 18;
    } reg;
} SOC_SCTRL_SC_SECOND_INT_MASK_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0: 1;
        unsigned int reserved_1: 1;
        unsigned int reserved_2: 1;
        unsigned int reserved_3: 1;
        unsigned int reserved_4: 1;
        unsigned int reserved_5: 1;
        unsigned int reserved_6: 1;
        unsigned int reserved_7: 1;
        unsigned int reserved_8: 1;
        unsigned int reserved_9: 1;
        unsigned int reserved_10: 1;
        unsigned int reserved_11: 1;
        unsigned int reserved_12: 1;
        unsigned int reserved_13: 1;
        unsigned int reserved_14: 18;
    } reg;
} SOC_SCTRL_SC_SECOND_INT_ORG_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0: 1;
        unsigned int reserved_1: 1;
        unsigned int reserved_2: 1;
        unsigned int reserved_3: 1;
        unsigned int reserved_4: 1;
        unsigned int reserved_5: 1;
        unsigned int reserved_6: 1;
        unsigned int reserved_7: 1;
        unsigned int reserved_8: 1;
        unsigned int reserved_9: 1;
        unsigned int reserved_10: 1;
        unsigned int reserved_11: 1;
        unsigned int reserved_12: 1;
        unsigned int reserved_13: 1;
        unsigned int reserved_14: 18;
    } reg;
} SOC_SCTRL_SC_SECOND_INT_OUT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int mrb_busy_flag : 32;
    } reg;
} SOC_SCTRL_SCMRBBUSYSTAT_UNION;
#endif
#define SOC_SCTRL_SCMRBBUSYSTAT_mrb_busy_flag_START (0)
#define SOC_SCTRL_SCMRBBUSYSTAT_mrb_busy_flag_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int swaddr : 32;
    } reg;
} SOC_SCTRL_SCSWADDR_UNION;
#endif
#define SOC_SCTRL_SCSWADDR_swaddr_START (0)
#define SOC_SCTRL_SCSWADDR_swaddr_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ddrtrainaddr : 32;
    } reg;
} SOC_SCTRL_SCDDRADDR_UNION;
#endif
#define SOC_SCTRL_SCDDRADDR_ddrtrainaddr_START (0)
#define SOC_SCTRL_SCDDRADDR_ddrtrainaddr_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ddrtraindata : 32;
    } reg;
} SOC_SCTRL_SCDDRDATA_UNION;
#endif
#define SOC_SCTRL_SCDDRDATA_ddrtraindata_START (0)
#define SOC_SCTRL_SCDDRDATA_ddrtraindata_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int bakdata : 32;
    } reg;
} SOC_SCTRL_SCBAKDATA0_UNION;
#endif
#define SOC_SCTRL_SCBAKDATA0_bakdata_START (0)
#define SOC_SCTRL_SCBAKDATA0_bakdata_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int bakdata : 32;
    } reg;
} SOC_SCTRL_SCBAKDATA1_UNION;
#endif
#define SOC_SCTRL_SCBAKDATA1_bakdata_START (0)
#define SOC_SCTRL_SCBAKDATA1_bakdata_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int bakdata : 32;
    } reg;
} SOC_SCTRL_SCBAKDATA2_UNION;
#endif
#define SOC_SCTRL_SCBAKDATA2_bakdata_START (0)
#define SOC_SCTRL_SCBAKDATA2_bakdata_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int bakdata : 32;
    } reg;
} SOC_SCTRL_SCBAKDATA3_UNION;
#endif
#define SOC_SCTRL_SCBAKDATA3_bakdata_START (0)
#define SOC_SCTRL_SCBAKDATA3_bakdata_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int bakdata : 32;
    } reg;
} SOC_SCTRL_SCBAKDATA4_UNION;
#endif
#define SOC_SCTRL_SCBAKDATA4_bakdata_START (0)
#define SOC_SCTRL_SCBAKDATA4_bakdata_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int bakdata : 32;
    } reg;
} SOC_SCTRL_SCBAKDATA5_UNION;
#endif
#define SOC_SCTRL_SCBAKDATA5_bakdata_START (0)
#define SOC_SCTRL_SCBAKDATA5_bakdata_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int bakdata : 32;
    } reg;
} SOC_SCTRL_SCBAKDATA6_UNION;
#endif
#define SOC_SCTRL_SCBAKDATA6_bakdata_START (0)
#define SOC_SCTRL_SCBAKDATA6_bakdata_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int bakdata : 32;
    } reg;
} SOC_SCTRL_SCBAKDATA7_UNION;
#endif
#define SOC_SCTRL_SCBAKDATA7_bakdata_START (0)
#define SOC_SCTRL_SCBAKDATA7_bakdata_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int bakdata : 32;
    } reg;
} SOC_SCTRL_SCBAKDATA8_UNION;
#endif
#define SOC_SCTRL_SCBAKDATA8_bakdata_START (0)
#define SOC_SCTRL_SCBAKDATA8_bakdata_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int bakdata : 32;
    } reg;
} SOC_SCTRL_SCBAKDATA9_UNION;
#endif
#define SOC_SCTRL_SCBAKDATA9_bakdata_START (0)
#define SOC_SCTRL_SCBAKDATA9_bakdata_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int bakdata : 32;
    } reg;
} SOC_SCTRL_SCBAKDATA10_UNION;
#endif
#define SOC_SCTRL_SCBAKDATA10_bakdata_START (0)
#define SOC_SCTRL_SCBAKDATA10_bakdata_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int bakdata : 32;
    } reg;
} SOC_SCTRL_SCBAKDATA11_UNION;
#endif
#define SOC_SCTRL_SCBAKDATA11_bakdata_START (0)
#define SOC_SCTRL_SCBAKDATA11_bakdata_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int bakdata : 32;
    } reg;
} SOC_SCTRL_SCBAKDATA12_UNION;
#endif
#define SOC_SCTRL_SCBAKDATA12_bakdata_START (0)
#define SOC_SCTRL_SCBAKDATA12_bakdata_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int bakdata : 32;
    } reg;
} SOC_SCTRL_SCBAKDATA13_UNION;
#endif
#define SOC_SCTRL_SCBAKDATA13_bakdata_START (0)
#define SOC_SCTRL_SCBAKDATA13_bakdata_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int bakdata : 32;
    } reg;
} SOC_SCTRL_SCBAKDATA14_UNION;
#endif
#define SOC_SCTRL_SCBAKDATA14_bakdata_START (0)
#define SOC_SCTRL_SCBAKDATA14_bakdata_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int bakdata : 32;
    } reg;
} SOC_SCTRL_SCBAKDATA15_UNION;
#endif
#define SOC_SCTRL_SCBAKDATA15_bakdata_START (0)
#define SOC_SCTRL_SCBAKDATA15_bakdata_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int bakdata : 32;
    } reg;
} SOC_SCTRL_SCBAKDATA16_UNION;
#endif
#define SOC_SCTRL_SCBAKDATA16_bakdata_START (0)
#define SOC_SCTRL_SCBAKDATA16_bakdata_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int bakdata : 32;
    } reg;
} SOC_SCTRL_SCBAKDATA17_UNION;
#endif
#define SOC_SCTRL_SCBAKDATA17_bakdata_START (0)
#define SOC_SCTRL_SCBAKDATA17_bakdata_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int bakdata : 32;
    } reg;
} SOC_SCTRL_SCBAKDATA18_UNION;
#endif
#define SOC_SCTRL_SCBAKDATA18_bakdata_START (0)
#define SOC_SCTRL_SCBAKDATA18_bakdata_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int bakdata : 32;
    } reg;
} SOC_SCTRL_SCBAKDATA19_UNION;
#endif
#define SOC_SCTRL_SCBAKDATA19_bakdata_START (0)
#define SOC_SCTRL_SCBAKDATA19_bakdata_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int bakdata : 32;
    } reg;
} SOC_SCTRL_SCBAKDATA20_UNION;
#endif
#define SOC_SCTRL_SCBAKDATA20_bakdata_START (0)
#define SOC_SCTRL_SCBAKDATA20_bakdata_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int bakdata : 32;
    } reg;
} SOC_SCTRL_SCBAKDATA21_UNION;
#endif
#define SOC_SCTRL_SCBAKDATA21_bakdata_START (0)
#define SOC_SCTRL_SCBAKDATA21_bakdata_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int bakdata : 32;
    } reg;
} SOC_SCTRL_SCBAKDATA22_UNION;
#endif
#define SOC_SCTRL_SCBAKDATA22_bakdata_START (0)
#define SOC_SCTRL_SCBAKDATA22_bakdata_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int bakdata : 32;
    } reg;
} SOC_SCTRL_SCBAKDATA23_UNION;
#endif
#define SOC_SCTRL_SCBAKDATA23_bakdata_START (0)
#define SOC_SCTRL_SCBAKDATA23_bakdata_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int bakdata : 32;
    } reg;
} SOC_SCTRL_SCBAKDATA24_UNION;
#endif
#define SOC_SCTRL_SCBAKDATA24_bakdata_START (0)
#define SOC_SCTRL_SCBAKDATA24_bakdata_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int bakdata : 32;
    } reg;
} SOC_SCTRL_SCBAKDATA25_UNION;
#endif
#define SOC_SCTRL_SCBAKDATA25_bakdata_START (0)
#define SOC_SCTRL_SCBAKDATA25_bakdata_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int bakdata : 32;
    } reg;
} SOC_SCTRL_SCBAKDATA26_UNION;
#endif
#define SOC_SCTRL_SCBAKDATA26_bakdata_START (0)
#define SOC_SCTRL_SCBAKDATA26_bakdata_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int bakdata : 32;
    } reg;
} SOC_SCTRL_SCBAKDATA27_UNION;
#endif
#define SOC_SCTRL_SCBAKDATA27_bakdata_START (0)
#define SOC_SCTRL_SCBAKDATA27_bakdata_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int bakdata : 32;
    } reg;
} SOC_SCTRL_SCBAKDATA28_UNION;
#endif
#define SOC_SCTRL_SCBAKDATA28_bakdata_START (0)
#define SOC_SCTRL_SCBAKDATA28_bakdata_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved : 16;
        unsigned int scbakdata0_msk_msk : 16;
    } reg;
} SOC_SCTRL_SCBAKDATA0_MSK_UNION;
#endif
#define SOC_SCTRL_SCBAKDATA0_MSK_scbakdata0_msk_msk_START (16)
#define SOC_SCTRL_SCBAKDATA0_MSK_scbakdata0_msk_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved : 16;
        unsigned int scbakdata1_msk_msk : 16;
    } reg;
} SOC_SCTRL_SCBAKDATA1_MSK_UNION;
#endif
#define SOC_SCTRL_SCBAKDATA1_MSK_scbakdata1_msk_msk_START (16)
#define SOC_SCTRL_SCBAKDATA1_MSK_scbakdata1_msk_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved : 16;
        unsigned int scbakdata2_msk_msk : 16;
    } reg;
} SOC_SCTRL_SCBAKDATA2_MSK_UNION;
#endif
#define SOC_SCTRL_SCBAKDATA2_MSK_scbakdata2_msk_msk_START (16)
#define SOC_SCTRL_SCBAKDATA2_MSK_scbakdata2_msk_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved : 16;
        unsigned int scbakdata3_msk_msk : 16;
    } reg;
} SOC_SCTRL_SCBAKDATA3_MSK_UNION;
#endif
#define SOC_SCTRL_SCBAKDATA3_MSK_scbakdata3_msk_msk_START (16)
#define SOC_SCTRL_SCBAKDATA3_MSK_scbakdata3_msk_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved : 16;
        unsigned int scbakdata4_msk_msk : 16;
    } reg;
} SOC_SCTRL_SCBAKDATA4_MSK_UNION;
#endif
#define SOC_SCTRL_SCBAKDATA4_MSK_scbakdata4_msk_msk_START (16)
#define SOC_SCTRL_SCBAKDATA4_MSK_scbakdata4_msk_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved : 16;
        unsigned int scbakdata5_msk_msk : 16;
    } reg;
} SOC_SCTRL_SCBAKDATA5_MSK_UNION;
#endif
#define SOC_SCTRL_SCBAKDATA5_MSK_scbakdata5_msk_msk_START (16)
#define SOC_SCTRL_SCBAKDATA5_MSK_scbakdata5_msk_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved : 16;
        unsigned int scbakdata6_msk_msk : 16;
    } reg;
} SOC_SCTRL_SCBAKDATA6_MSK_UNION;
#endif
#define SOC_SCTRL_SCBAKDATA6_MSK_scbakdata6_msk_msk_START (16)
#define SOC_SCTRL_SCBAKDATA6_MSK_scbakdata6_msk_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved : 16;
        unsigned int scbakdata7_msk_msk : 16;
    } reg;
} SOC_SCTRL_SCBAKDATA7_MSK_UNION;
#endif
#define SOC_SCTRL_SCBAKDATA7_MSK_scbakdata7_msk_msk_START (16)
#define SOC_SCTRL_SCBAKDATA7_MSK_scbakdata7_msk_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dapclkoff_sys_n : 1;
        unsigned int auto_clkgt_h2p_lpperi_en : 1;
        unsigned int tcm_clk_clkoff_sys_n : 1;
        unsigned int ram_clk_clkoff_sys_n : 1;
        unsigned int aon_clk_clkoff_sys_n : 1;
        unsigned int reserved_0 : 1;
        unsigned int tsen_clk_clkoff_sys_n : 1;
        unsigned int pmc_clk_clkoff_sys_n : 1;
        unsigned int uart_clk_clkoff_sys_n : 1;
        unsigned int funcmbist_clk_clkoff_sys_n : 1;
        unsigned int reserved_1 : 1;
        unsigned int pcrg_clk_clkoff_sys_n : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int tim_clk_clkoff_sys_n : 1;
        unsigned int cssys_clk_clkoff_sys_n : 1;
        unsigned int spi_clk_clkoff_sys_n : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int reserved_11 : 1;
        unsigned int reserved_12 : 1;
        unsigned int reserved_13 : 1;
        unsigned int reserved_14 : 1;
        unsigned int reserved_15 : 1;
        unsigned int reserved_16 : 1;
        unsigned int reserved_17 : 1;
        unsigned int reserved_18 : 1;
    } reg;
} SOC_SCTRL_SCLPMCUCLKEN_UNION;
#endif
#define SOC_SCTRL_SCLPMCUCLKEN_dapclkoff_sys_n_START (0)
#define SOC_SCTRL_SCLPMCUCLKEN_dapclkoff_sys_n_END (0)
#define SOC_SCTRL_SCLPMCUCLKEN_auto_clkgt_h2p_lpperi_en_START (1)
#define SOC_SCTRL_SCLPMCUCLKEN_auto_clkgt_h2p_lpperi_en_END (1)
#define SOC_SCTRL_SCLPMCUCLKEN_tcm_clk_clkoff_sys_n_START (2)
#define SOC_SCTRL_SCLPMCUCLKEN_tcm_clk_clkoff_sys_n_END (2)
#define SOC_SCTRL_SCLPMCUCLKEN_ram_clk_clkoff_sys_n_START (3)
#define SOC_SCTRL_SCLPMCUCLKEN_ram_clk_clkoff_sys_n_END (3)
#define SOC_SCTRL_SCLPMCUCLKEN_aon_clk_clkoff_sys_n_START (4)
#define SOC_SCTRL_SCLPMCUCLKEN_aon_clk_clkoff_sys_n_END (4)
#define SOC_SCTRL_SCLPMCUCLKEN_tsen_clk_clkoff_sys_n_START (6)
#define SOC_SCTRL_SCLPMCUCLKEN_tsen_clk_clkoff_sys_n_END (6)
#define SOC_SCTRL_SCLPMCUCLKEN_pmc_clk_clkoff_sys_n_START (7)
#define SOC_SCTRL_SCLPMCUCLKEN_pmc_clk_clkoff_sys_n_END (7)
#define SOC_SCTRL_SCLPMCUCLKEN_uart_clk_clkoff_sys_n_START (8)
#define SOC_SCTRL_SCLPMCUCLKEN_uart_clk_clkoff_sys_n_END (8)
#define SOC_SCTRL_SCLPMCUCLKEN_funcmbist_clk_clkoff_sys_n_START (9)
#define SOC_SCTRL_SCLPMCUCLKEN_funcmbist_clk_clkoff_sys_n_END (9)
#define SOC_SCTRL_SCLPMCUCLKEN_pcrg_clk_clkoff_sys_n_START (11)
#define SOC_SCTRL_SCLPMCUCLKEN_pcrg_clk_clkoff_sys_n_END (11)
#define SOC_SCTRL_SCLPMCUCLKEN_tim_clk_clkoff_sys_n_START (14)
#define SOC_SCTRL_SCLPMCUCLKEN_tim_clk_clkoff_sys_n_END (14)
#define SOC_SCTRL_SCLPMCUCLKEN_cssys_clk_clkoff_sys_n_START (15)
#define SOC_SCTRL_SCLPMCUCLKEN_cssys_clk_clkoff_sys_n_END (15)
#define SOC_SCTRL_SCLPMCUCLKEN_spi_clk_clkoff_sys_n_START (16)
#define SOC_SCTRL_SCLPMCUCLKEN_spi_clk_clkoff_sys_n_END (16)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dapclkoff_sys_n : 1;
        unsigned int auto_clkgt_h2p_lpperi_en : 1;
        unsigned int tcm_clk_clkoff_sys_n : 1;
        unsigned int ram_clk_clkoff_sys_n : 1;
        unsigned int aon_clk_clkoff_sys_n : 1;
        unsigned int reserved_0 : 1;
        unsigned int tsen_clk_clkoff_sys_n : 1;
        unsigned int pmc_clk_clkoff_sys_n : 1;
        unsigned int uart_clk_clkoff_sys_n : 1;
        unsigned int funcmbist_clk_clkoff_sys_n : 1;
        unsigned int reserved_1 : 1;
        unsigned int pcrg_clk_clkoff_sys_n : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int tim_clk_clkoff_sys_n : 1;
        unsigned int cssys_clk_clkoff_sys_n : 1;
        unsigned int spi_clk_clkoff_sys_n : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int reserved_11 : 1;
        unsigned int reserved_12 : 1;
        unsigned int reserved_13 : 1;
        unsigned int reserved_14 : 1;
        unsigned int reserved_15 : 1;
        unsigned int reserved_16 : 1;
        unsigned int reserved_17 : 1;
        unsigned int reserved_18 : 1;
    } reg;
} SOC_SCTRL_SCLPMCUCLKDIS_UNION;
#endif
#define SOC_SCTRL_SCLPMCUCLKDIS_dapclkoff_sys_n_START (0)
#define SOC_SCTRL_SCLPMCUCLKDIS_dapclkoff_sys_n_END (0)
#define SOC_SCTRL_SCLPMCUCLKDIS_auto_clkgt_h2p_lpperi_en_START (1)
#define SOC_SCTRL_SCLPMCUCLKDIS_auto_clkgt_h2p_lpperi_en_END (1)
#define SOC_SCTRL_SCLPMCUCLKDIS_tcm_clk_clkoff_sys_n_START (2)
#define SOC_SCTRL_SCLPMCUCLKDIS_tcm_clk_clkoff_sys_n_END (2)
#define SOC_SCTRL_SCLPMCUCLKDIS_ram_clk_clkoff_sys_n_START (3)
#define SOC_SCTRL_SCLPMCUCLKDIS_ram_clk_clkoff_sys_n_END (3)
#define SOC_SCTRL_SCLPMCUCLKDIS_aon_clk_clkoff_sys_n_START (4)
#define SOC_SCTRL_SCLPMCUCLKDIS_aon_clk_clkoff_sys_n_END (4)
#define SOC_SCTRL_SCLPMCUCLKDIS_tsen_clk_clkoff_sys_n_START (6)
#define SOC_SCTRL_SCLPMCUCLKDIS_tsen_clk_clkoff_sys_n_END (6)
#define SOC_SCTRL_SCLPMCUCLKDIS_pmc_clk_clkoff_sys_n_START (7)
#define SOC_SCTRL_SCLPMCUCLKDIS_pmc_clk_clkoff_sys_n_END (7)
#define SOC_SCTRL_SCLPMCUCLKDIS_uart_clk_clkoff_sys_n_START (8)
#define SOC_SCTRL_SCLPMCUCLKDIS_uart_clk_clkoff_sys_n_END (8)
#define SOC_SCTRL_SCLPMCUCLKDIS_funcmbist_clk_clkoff_sys_n_START (9)
#define SOC_SCTRL_SCLPMCUCLKDIS_funcmbist_clk_clkoff_sys_n_END (9)
#define SOC_SCTRL_SCLPMCUCLKDIS_pcrg_clk_clkoff_sys_n_START (11)
#define SOC_SCTRL_SCLPMCUCLKDIS_pcrg_clk_clkoff_sys_n_END (11)
#define SOC_SCTRL_SCLPMCUCLKDIS_tim_clk_clkoff_sys_n_START (14)
#define SOC_SCTRL_SCLPMCUCLKDIS_tim_clk_clkoff_sys_n_END (14)
#define SOC_SCTRL_SCLPMCUCLKDIS_cssys_clk_clkoff_sys_n_START (15)
#define SOC_SCTRL_SCLPMCUCLKDIS_cssys_clk_clkoff_sys_n_END (15)
#define SOC_SCTRL_SCLPMCUCLKDIS_spi_clk_clkoff_sys_n_START (16)
#define SOC_SCTRL_SCLPMCUCLKDIS_spi_clk_clkoff_sys_n_END (16)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dapclkoff_sys_n : 1;
        unsigned int auto_clkgt_h2p_lpperi_en : 1;
        unsigned int tcm_clk_clkoff_sys_n : 1;
        unsigned int ram_clk_clkoff_sys_n : 1;
        unsigned int aon_clk_clkoff_sys_n : 1;
        unsigned int reserved_0 : 1;
        unsigned int tsen_clk_clkoff_sys_n : 1;
        unsigned int pmc_clk_clkoff_sys_n : 1;
        unsigned int uart_clk_clkoff_sys_n : 1;
        unsigned int funcmbist_clk_clkoff_sys_n : 1;
        unsigned int reserved_1 : 1;
        unsigned int pcrg_clk_clkoff_sys_n : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int tim_clk_clkoff_sys_n : 1;
        unsigned int cssys_clk_clkoff_sys_n : 1;
        unsigned int spi_clk_clkoff_sys_n : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int reserved_11 : 1;
        unsigned int reserved_12 : 1;
        unsigned int reserved_13 : 1;
        unsigned int reserved_14 : 1;
        unsigned int reserved_15 : 1;
        unsigned int reserved_16 : 1;
        unsigned int reserved_17 : 1;
        unsigned int reserved_18 : 1;
    } reg;
} SOC_SCTRL_SCLPMCUCLKSTAT_UNION;
#endif
#define SOC_SCTRL_SCLPMCUCLKSTAT_dapclkoff_sys_n_START (0)
#define SOC_SCTRL_SCLPMCUCLKSTAT_dapclkoff_sys_n_END (0)
#define SOC_SCTRL_SCLPMCUCLKSTAT_auto_clkgt_h2p_lpperi_en_START (1)
#define SOC_SCTRL_SCLPMCUCLKSTAT_auto_clkgt_h2p_lpperi_en_END (1)
#define SOC_SCTRL_SCLPMCUCLKSTAT_tcm_clk_clkoff_sys_n_START (2)
#define SOC_SCTRL_SCLPMCUCLKSTAT_tcm_clk_clkoff_sys_n_END (2)
#define SOC_SCTRL_SCLPMCUCLKSTAT_ram_clk_clkoff_sys_n_START (3)
#define SOC_SCTRL_SCLPMCUCLKSTAT_ram_clk_clkoff_sys_n_END (3)
#define SOC_SCTRL_SCLPMCUCLKSTAT_aon_clk_clkoff_sys_n_START (4)
#define SOC_SCTRL_SCLPMCUCLKSTAT_aon_clk_clkoff_sys_n_END (4)
#define SOC_SCTRL_SCLPMCUCLKSTAT_tsen_clk_clkoff_sys_n_START (6)
#define SOC_SCTRL_SCLPMCUCLKSTAT_tsen_clk_clkoff_sys_n_END (6)
#define SOC_SCTRL_SCLPMCUCLKSTAT_pmc_clk_clkoff_sys_n_START (7)
#define SOC_SCTRL_SCLPMCUCLKSTAT_pmc_clk_clkoff_sys_n_END (7)
#define SOC_SCTRL_SCLPMCUCLKSTAT_uart_clk_clkoff_sys_n_START (8)
#define SOC_SCTRL_SCLPMCUCLKSTAT_uart_clk_clkoff_sys_n_END (8)
#define SOC_SCTRL_SCLPMCUCLKSTAT_funcmbist_clk_clkoff_sys_n_START (9)
#define SOC_SCTRL_SCLPMCUCLKSTAT_funcmbist_clk_clkoff_sys_n_END (9)
#define SOC_SCTRL_SCLPMCUCLKSTAT_pcrg_clk_clkoff_sys_n_START (11)
#define SOC_SCTRL_SCLPMCUCLKSTAT_pcrg_clk_clkoff_sys_n_END (11)
#define SOC_SCTRL_SCLPMCUCLKSTAT_tim_clk_clkoff_sys_n_START (14)
#define SOC_SCTRL_SCLPMCUCLKSTAT_tim_clk_clkoff_sys_n_END (14)
#define SOC_SCTRL_SCLPMCUCLKSTAT_cssys_clk_clkoff_sys_n_START (15)
#define SOC_SCTRL_SCLPMCUCLKSTAT_cssys_clk_clkoff_sys_n_END (15)
#define SOC_SCTRL_SCLPMCUCLKSTAT_spi_clk_clkoff_sys_n_START (16)
#define SOC_SCTRL_SCLPMCUCLKSTAT_spi_clk_clkoff_sys_n_END (16)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int rst_software_req : 1;
        unsigned int coresight_soft_rst_req : 1;
        unsigned int reserved_0 : 1;
        unsigned int tcm_soft_rst_req : 1;
        unsigned int ram_soft_rst_req : 1;
        unsigned int aon_soft_rst_req : 1;
        unsigned int ccrg_soft_rst_req : 1;
        unsigned int tsen_soft_rst_req : 1;
        unsigned int pmc_soft_rst_req : 1;
        unsigned int uart_soft_rst_req : 1;
        unsigned int funcmbist_soft_rst_req : 1;
        unsigned int reserved_1 : 1;
        unsigned int pcrg_soft_rst_req : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int tim_soft_rst_req : 1;
        unsigned int cssys_soft_rst_req : 1;
        unsigned int spi_soft_rst_req : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int reserved_11 : 1;
        unsigned int reserved_12 : 1;
        unsigned int reserved_13 : 1;
        unsigned int reserved_14 : 1;
        unsigned int reserved_15 : 1;
        unsigned int reserved_16 : 1;
        unsigned int reserved_17 : 1;
    } reg;
} SOC_SCTRL_SCLPMCURSTEN_UNION;
#endif
#define SOC_SCTRL_SCLPMCURSTEN_rst_software_req_START (0)
#define SOC_SCTRL_SCLPMCURSTEN_rst_software_req_END (0)
#define SOC_SCTRL_SCLPMCURSTEN_coresight_soft_rst_req_START (1)
#define SOC_SCTRL_SCLPMCURSTEN_coresight_soft_rst_req_END (1)
#define SOC_SCTRL_SCLPMCURSTEN_tcm_soft_rst_req_START (3)
#define SOC_SCTRL_SCLPMCURSTEN_tcm_soft_rst_req_END (3)
#define SOC_SCTRL_SCLPMCURSTEN_ram_soft_rst_req_START (4)
#define SOC_SCTRL_SCLPMCURSTEN_ram_soft_rst_req_END (4)
#define SOC_SCTRL_SCLPMCURSTEN_aon_soft_rst_req_START (5)
#define SOC_SCTRL_SCLPMCURSTEN_aon_soft_rst_req_END (5)
#define SOC_SCTRL_SCLPMCURSTEN_ccrg_soft_rst_req_START (6)
#define SOC_SCTRL_SCLPMCURSTEN_ccrg_soft_rst_req_END (6)
#define SOC_SCTRL_SCLPMCURSTEN_tsen_soft_rst_req_START (7)
#define SOC_SCTRL_SCLPMCURSTEN_tsen_soft_rst_req_END (7)
#define SOC_SCTRL_SCLPMCURSTEN_pmc_soft_rst_req_START (8)
#define SOC_SCTRL_SCLPMCURSTEN_pmc_soft_rst_req_END (8)
#define SOC_SCTRL_SCLPMCURSTEN_uart_soft_rst_req_START (9)
#define SOC_SCTRL_SCLPMCURSTEN_uart_soft_rst_req_END (9)
#define SOC_SCTRL_SCLPMCURSTEN_funcmbist_soft_rst_req_START (10)
#define SOC_SCTRL_SCLPMCURSTEN_funcmbist_soft_rst_req_END (10)
#define SOC_SCTRL_SCLPMCURSTEN_pcrg_soft_rst_req_START (12)
#define SOC_SCTRL_SCLPMCURSTEN_pcrg_soft_rst_req_END (12)
#define SOC_SCTRL_SCLPMCURSTEN_tim_soft_rst_req_START (15)
#define SOC_SCTRL_SCLPMCURSTEN_tim_soft_rst_req_END (15)
#define SOC_SCTRL_SCLPMCURSTEN_cssys_soft_rst_req_START (16)
#define SOC_SCTRL_SCLPMCURSTEN_cssys_soft_rst_req_END (16)
#define SOC_SCTRL_SCLPMCURSTEN_spi_soft_rst_req_START (17)
#define SOC_SCTRL_SCLPMCURSTEN_spi_soft_rst_req_END (17)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int rst_software_req : 1;
        unsigned int coresight_soft_rst_req : 1;
        unsigned int reserved_0 : 1;
        unsigned int tcm_soft_rst_req : 1;
        unsigned int ram_soft_rst_req : 1;
        unsigned int aon_soft_rst_req : 1;
        unsigned int ccrg_soft_rst_req : 1;
        unsigned int tsen_soft_rst_req : 1;
        unsigned int pmc_soft_rst_req : 1;
        unsigned int uart_soft_rst_req : 1;
        unsigned int funcmbist_soft_rst_req : 1;
        unsigned int reserved_1 : 1;
        unsigned int pcrg_soft_rst_req : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int tim_soft_rst_req : 1;
        unsigned int cssys_soft_rst_req : 1;
        unsigned int spi_soft_rst_req : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int reserved_11 : 1;
        unsigned int reserved_12 : 1;
        unsigned int reserved_13 : 1;
        unsigned int reserved_14 : 1;
        unsigned int reserved_15 : 1;
        unsigned int reserved_16 : 1;
        unsigned int reserved_17 : 1;
    } reg;
} SOC_SCTRL_SCLPMCURSTDIS_UNION;
#endif
#define SOC_SCTRL_SCLPMCURSTDIS_rst_software_req_START (0)
#define SOC_SCTRL_SCLPMCURSTDIS_rst_software_req_END (0)
#define SOC_SCTRL_SCLPMCURSTDIS_coresight_soft_rst_req_START (1)
#define SOC_SCTRL_SCLPMCURSTDIS_coresight_soft_rst_req_END (1)
#define SOC_SCTRL_SCLPMCURSTDIS_tcm_soft_rst_req_START (3)
#define SOC_SCTRL_SCLPMCURSTDIS_tcm_soft_rst_req_END (3)
#define SOC_SCTRL_SCLPMCURSTDIS_ram_soft_rst_req_START (4)
#define SOC_SCTRL_SCLPMCURSTDIS_ram_soft_rst_req_END (4)
#define SOC_SCTRL_SCLPMCURSTDIS_aon_soft_rst_req_START (5)
#define SOC_SCTRL_SCLPMCURSTDIS_aon_soft_rst_req_END (5)
#define SOC_SCTRL_SCLPMCURSTDIS_ccrg_soft_rst_req_START (6)
#define SOC_SCTRL_SCLPMCURSTDIS_ccrg_soft_rst_req_END (6)
#define SOC_SCTRL_SCLPMCURSTDIS_tsen_soft_rst_req_START (7)
#define SOC_SCTRL_SCLPMCURSTDIS_tsen_soft_rst_req_END (7)
#define SOC_SCTRL_SCLPMCURSTDIS_pmc_soft_rst_req_START (8)
#define SOC_SCTRL_SCLPMCURSTDIS_pmc_soft_rst_req_END (8)
#define SOC_SCTRL_SCLPMCURSTDIS_uart_soft_rst_req_START (9)
#define SOC_SCTRL_SCLPMCURSTDIS_uart_soft_rst_req_END (9)
#define SOC_SCTRL_SCLPMCURSTDIS_funcmbist_soft_rst_req_START (10)
#define SOC_SCTRL_SCLPMCURSTDIS_funcmbist_soft_rst_req_END (10)
#define SOC_SCTRL_SCLPMCURSTDIS_pcrg_soft_rst_req_START (12)
#define SOC_SCTRL_SCLPMCURSTDIS_pcrg_soft_rst_req_END (12)
#define SOC_SCTRL_SCLPMCURSTDIS_tim_soft_rst_req_START (15)
#define SOC_SCTRL_SCLPMCURSTDIS_tim_soft_rst_req_END (15)
#define SOC_SCTRL_SCLPMCURSTDIS_cssys_soft_rst_req_START (16)
#define SOC_SCTRL_SCLPMCURSTDIS_cssys_soft_rst_req_END (16)
#define SOC_SCTRL_SCLPMCURSTDIS_spi_soft_rst_req_START (17)
#define SOC_SCTRL_SCLPMCURSTDIS_spi_soft_rst_req_END (17)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int rst_software_req : 1;
        unsigned int coresight_soft_rst_req : 1;
        unsigned int reserved_0 : 1;
        unsigned int tcm_soft_rst_req : 1;
        unsigned int ram_soft_rst_req : 1;
        unsigned int aon_soft_rst_req : 1;
        unsigned int ccrg_soft_rst_req : 1;
        unsigned int tsen_soft_rst_req : 1;
        unsigned int pmc_soft_rst_req : 1;
        unsigned int uart_soft_rst_req : 1;
        unsigned int funcmbist_soft_rst_req : 1;
        unsigned int reserved_1 : 1;
        unsigned int pcrg_soft_rst_req : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int tim_soft_rst_req : 1;
        unsigned int cssys_soft_rst_req : 1;
        unsigned int spi_soft_rst_req : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int reserved_11 : 1;
        unsigned int reserved_12 : 1;
        unsigned int reserved_13 : 1;
        unsigned int reserved_14 : 1;
        unsigned int reserved_15 : 1;
        unsigned int reserved_16 : 1;
        unsigned int reserved_17 : 1;
    } reg;
} SOC_SCTRL_SCLPMCURSTSTAT_UNION;
#endif
#define SOC_SCTRL_SCLPMCURSTSTAT_rst_software_req_START (0)
#define SOC_SCTRL_SCLPMCURSTSTAT_rst_software_req_END (0)
#define SOC_SCTRL_SCLPMCURSTSTAT_coresight_soft_rst_req_START (1)
#define SOC_SCTRL_SCLPMCURSTSTAT_coresight_soft_rst_req_END (1)
#define SOC_SCTRL_SCLPMCURSTSTAT_tcm_soft_rst_req_START (3)
#define SOC_SCTRL_SCLPMCURSTSTAT_tcm_soft_rst_req_END (3)
#define SOC_SCTRL_SCLPMCURSTSTAT_ram_soft_rst_req_START (4)
#define SOC_SCTRL_SCLPMCURSTSTAT_ram_soft_rst_req_END (4)
#define SOC_SCTRL_SCLPMCURSTSTAT_aon_soft_rst_req_START (5)
#define SOC_SCTRL_SCLPMCURSTSTAT_aon_soft_rst_req_END (5)
#define SOC_SCTRL_SCLPMCURSTSTAT_ccrg_soft_rst_req_START (6)
#define SOC_SCTRL_SCLPMCURSTSTAT_ccrg_soft_rst_req_END (6)
#define SOC_SCTRL_SCLPMCURSTSTAT_tsen_soft_rst_req_START (7)
#define SOC_SCTRL_SCLPMCURSTSTAT_tsen_soft_rst_req_END (7)
#define SOC_SCTRL_SCLPMCURSTSTAT_pmc_soft_rst_req_START (8)
#define SOC_SCTRL_SCLPMCURSTSTAT_pmc_soft_rst_req_END (8)
#define SOC_SCTRL_SCLPMCURSTSTAT_uart_soft_rst_req_START (9)
#define SOC_SCTRL_SCLPMCURSTSTAT_uart_soft_rst_req_END (9)
#define SOC_SCTRL_SCLPMCURSTSTAT_funcmbist_soft_rst_req_START (10)
#define SOC_SCTRL_SCLPMCURSTSTAT_funcmbist_soft_rst_req_END (10)
#define SOC_SCTRL_SCLPMCURSTSTAT_pcrg_soft_rst_req_START (12)
#define SOC_SCTRL_SCLPMCURSTSTAT_pcrg_soft_rst_req_END (12)
#define SOC_SCTRL_SCLPMCURSTSTAT_tim_soft_rst_req_START (15)
#define SOC_SCTRL_SCLPMCURSTSTAT_tim_soft_rst_req_END (15)
#define SOC_SCTRL_SCLPMCURSTSTAT_cssys_soft_rst_req_START (16)
#define SOC_SCTRL_SCLPMCURSTSTAT_cssys_soft_rst_req_END (16)
#define SOC_SCTRL_SCLPMCURSTSTAT_spi_soft_rst_req_START (17)
#define SOC_SCTRL_SCLPMCURSTSTAT_spi_soft_rst_req_END (17)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0 : 1;
        unsigned int mpudisable : 1;
        unsigned int nmi_in : 1;
        unsigned int bus_div_clk_sel : 2;
        unsigned int sysresetreq_en : 1;
        unsigned int reserved_1 : 5;
        unsigned int lpmcuidle_bypass : 1;
        unsigned int reserved_2 : 12;
        unsigned int reserved_3 : 8;
    } reg;
} SOC_SCTRL_SCLPMCUCTRL_UNION;
#endif
#define SOC_SCTRL_SCLPMCUCTRL_mpudisable_START (1)
#define SOC_SCTRL_SCLPMCUCTRL_mpudisable_END (1)
#define SOC_SCTRL_SCLPMCUCTRL_nmi_in_START (2)
#define SOC_SCTRL_SCLPMCUCTRL_nmi_in_END (2)
#define SOC_SCTRL_SCLPMCUCTRL_bus_div_clk_sel_START (3)
#define SOC_SCTRL_SCLPMCUCTRL_bus_div_clk_sel_END (4)
#define SOC_SCTRL_SCLPMCUCTRL_sysresetreq_en_START (5)
#define SOC_SCTRL_SCLPMCUCTRL_sysresetreq_en_END (5)
#define SOC_SCTRL_SCLPMCUCTRL_lpmcuidle_bypass_START (11)
#define SOC_SCTRL_SCLPMCUCTRL_lpmcuidle_bypass_END (11)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int sysresetreq_sign_out_stat : 1;
        unsigned int sleeping : 1;
        unsigned int lockup : 1;
        unsigned int halted : 1;
        unsigned int m3_lp_bus_slv_idle_flag : 1;
        unsigned int in_nmi_hdlr : 1;
        unsigned int hard_fault_mode : 1;
        unsigned int lpmcu_lp_h2p_idle : 1;
        unsigned int lpmcu_bus_idle : 1;
        unsigned int reserved_0 : 2;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 19;
    } reg;
} SOC_SCTRL_SCLPMCUSTAT_UNION;
#endif
#define SOC_SCTRL_SCLPMCUSTAT_sysresetreq_sign_out_stat_START (0)
#define SOC_SCTRL_SCLPMCUSTAT_sysresetreq_sign_out_stat_END (0)
#define SOC_SCTRL_SCLPMCUSTAT_sleeping_START (1)
#define SOC_SCTRL_SCLPMCUSTAT_sleeping_END (1)
#define SOC_SCTRL_SCLPMCUSTAT_lockup_START (2)
#define SOC_SCTRL_SCLPMCUSTAT_lockup_END (2)
#define SOC_SCTRL_SCLPMCUSTAT_halted_START (3)
#define SOC_SCTRL_SCLPMCUSTAT_halted_END (3)
#define SOC_SCTRL_SCLPMCUSTAT_m3_lp_bus_slv_idle_flag_START (4)
#define SOC_SCTRL_SCLPMCUSTAT_m3_lp_bus_slv_idle_flag_END (4)
#define SOC_SCTRL_SCLPMCUSTAT_in_nmi_hdlr_START (5)
#define SOC_SCTRL_SCLPMCUSTAT_in_nmi_hdlr_END (5)
#define SOC_SCTRL_SCLPMCUSTAT_hard_fault_mode_START (6)
#define SOC_SCTRL_SCLPMCUSTAT_hard_fault_mode_END (6)
#define SOC_SCTRL_SCLPMCUSTAT_lpmcu_lp_h2p_idle_START (7)
#define SOC_SCTRL_SCLPMCUSTAT_lpmcu_lp_h2p_idle_END (7)
#define SOC_SCTRL_SCLPMCUSTAT_lpmcu_bus_idle_START (8)
#define SOC_SCTRL_SCLPMCUSTAT_lpmcu_bus_idle_END (8)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int lpramctrl_sel : 1;
        unsigned int lpram0_mod : 2;
        unsigned int lpram1_mod : 2;
        unsigned int lpram2_mod : 2;
        unsigned int lpram3_mod : 2;
        unsigned int lpram0ctrl_soft : 3;
        unsigned int lpram1ctrl_soft : 3;
        unsigned int lpram2ctrl_soft : 3;
        unsigned int lpram3ctrl_soft : 3;
        unsigned int memrep_ram_mod : 2;
        unsigned int memrep_ramctrl_soft : 3;
        unsigned int reserved : 6;
    } reg;
} SOC_SCTRL_SCLPMCURAMCTRL_UNION;
#endif
#define SOC_SCTRL_SCLPMCURAMCTRL_lpramctrl_sel_START (0)
#define SOC_SCTRL_SCLPMCURAMCTRL_lpramctrl_sel_END (0)
#define SOC_SCTRL_SCLPMCURAMCTRL_lpram0_mod_START (1)
#define SOC_SCTRL_SCLPMCURAMCTRL_lpram0_mod_END (2)
#define SOC_SCTRL_SCLPMCURAMCTRL_lpram1_mod_START (3)
#define SOC_SCTRL_SCLPMCURAMCTRL_lpram1_mod_END (4)
#define SOC_SCTRL_SCLPMCURAMCTRL_lpram2_mod_START (5)
#define SOC_SCTRL_SCLPMCURAMCTRL_lpram2_mod_END (6)
#define SOC_SCTRL_SCLPMCURAMCTRL_lpram3_mod_START (7)
#define SOC_SCTRL_SCLPMCURAMCTRL_lpram3_mod_END (8)
#define SOC_SCTRL_SCLPMCURAMCTRL_lpram0ctrl_soft_START (9)
#define SOC_SCTRL_SCLPMCURAMCTRL_lpram0ctrl_soft_END (11)
#define SOC_SCTRL_SCLPMCURAMCTRL_lpram1ctrl_soft_START (12)
#define SOC_SCTRL_SCLPMCURAMCTRL_lpram1ctrl_soft_END (14)
#define SOC_SCTRL_SCLPMCURAMCTRL_lpram2ctrl_soft_START (15)
#define SOC_SCTRL_SCLPMCURAMCTRL_lpram2ctrl_soft_END (17)
#define SOC_SCTRL_SCLPMCURAMCTRL_lpram3ctrl_soft_START (18)
#define SOC_SCTRL_SCLPMCURAMCTRL_lpram3ctrl_soft_END (20)
#define SOC_SCTRL_SCLPMCURAMCTRL_memrep_ram_mod_START (21)
#define SOC_SCTRL_SCLPMCURAMCTRL_memrep_ram_mod_END (22)
#define SOC_SCTRL_SCLPMCURAMCTRL_memrep_ramctrl_soft_START (23)
#define SOC_SCTRL_SCLPMCURAMCTRL_memrep_ramctrl_soft_END (25)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int g2_gdrx_depsleep_flag : 1;
        unsigned int g1_gdrx_depsleep_flag : 1;
        unsigned int wdrx_deepsleep_flag : 1;
        unsigned int cdrx_deepsleep_flag_1x : 1;
        unsigned int cdrx_deepsleep_flag_evdo : 1;
        unsigned int reserved : 27;
    } reg;
} SOC_SCTRL_SCBBPDRXSTAT0_UNION;
#endif
#define SOC_SCTRL_SCBBPDRXSTAT0_g2_gdrx_depsleep_flag_START (0)
#define SOC_SCTRL_SCBBPDRXSTAT0_g2_gdrx_depsleep_flag_END (0)
#define SOC_SCTRL_SCBBPDRXSTAT0_g1_gdrx_depsleep_flag_START (1)
#define SOC_SCTRL_SCBBPDRXSTAT0_g1_gdrx_depsleep_flag_END (1)
#define SOC_SCTRL_SCBBPDRXSTAT0_wdrx_deepsleep_flag_START (2)
#define SOC_SCTRL_SCBBPDRXSTAT0_wdrx_deepsleep_flag_END (2)
#define SOC_SCTRL_SCBBPDRXSTAT0_cdrx_deepsleep_flag_1x_START (3)
#define SOC_SCTRL_SCBBPDRXSTAT0_cdrx_deepsleep_flag_1x_END (3)
#define SOC_SCTRL_SCBBPDRXSTAT0_cdrx_deepsleep_flag_evdo_START (4)
#define SOC_SCTRL_SCBBPDRXSTAT0_cdrx_deepsleep_flag_evdo_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ldrx2dbg_abs_timer1 : 32;
    } reg;
} SOC_SCTRL_SCBBPDRXSTAT1_UNION;
#endif
#define SOC_SCTRL_SCBBPDRXSTAT1_ldrx2dbg_abs_timer1_START (0)
#define SOC_SCTRL_SCBBPDRXSTAT1_ldrx2dbg_abs_timer1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ldrx2dbg_abs_timer2 : 32;
    } reg;
} SOC_SCTRL_SCBBPDRXSTAT2_UNION;
#endif
#define SOC_SCTRL_SCBBPDRXSTAT2_ldrx2dbg_abs_timer2_START (0)
#define SOC_SCTRL_SCBBPDRXSTAT2_ldrx2dbg_abs_timer2_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ldrx2dbg_abs_timer_sleep1 : 32;
    } reg;
} SOC_SCTRL_SCBBPDRXSTAT3_UNION;
#endif
#define SOC_SCTRL_SCBBPDRXSTAT3_ldrx2dbg_abs_timer_sleep1_START (0)
#define SOC_SCTRL_SCBBPDRXSTAT3_ldrx2dbg_abs_timer_sleep1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ldrx2dbg_abs_timer_sleep2 : 32;
    } reg;
} SOC_SCTRL_SCBBPDRXSTAT4_UNION;
#endif
#define SOC_SCTRL_SCBBPDRXSTAT4_ldrx2dbg_abs_timer_sleep2_START (0)
#define SOC_SCTRL_SCBBPDRXSTAT4_ldrx2dbg_abs_timer_sleep2_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int a53_event_mask : 1;
        unsigned int a57_event_mask : 1;
        unsigned int iomcu_event_mask : 1;
        unsigned int lpmcu_event_mask : 1;
        unsigned int mcpu_event_mask : 1;
        unsigned int ispa7_event_mask : 1;
        unsigned int hifd_ul_event_mask : 1;
        unsigned int eps_event_mask : 1;
        unsigned int npu_archs_event_mask : 1;
        unsigned int npu_tscpu_event_mask : 1;
        unsigned int npu_aicpu_event_mask : 1;
        unsigned int mdm5g_l2hac_event_mask : 1;
        unsigned int mdm5g_ccpu5g_event_mask : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int reserved_11 : 1;
        unsigned int reserved_12 : 1;
        unsigned int reserved_13 : 1;
        unsigned int reserved_14 : 1;
        unsigned int reserved_15 : 1;
        unsigned int reserved_16 : 1;
        unsigned int reserved_17 : 1;
        unsigned int reserved_18 : 1;
    } reg;
} SOC_SCTRL_SCA53_EVENT_MASK_UNION;
#endif
#define SOC_SCTRL_SCA53_EVENT_MASK_a53_event_mask_START (0)
#define SOC_SCTRL_SCA53_EVENT_MASK_a53_event_mask_END (0)
#define SOC_SCTRL_SCA53_EVENT_MASK_a57_event_mask_START (1)
#define SOC_SCTRL_SCA53_EVENT_MASK_a57_event_mask_END (1)
#define SOC_SCTRL_SCA53_EVENT_MASK_iomcu_event_mask_START (2)
#define SOC_SCTRL_SCA53_EVENT_MASK_iomcu_event_mask_END (2)
#define SOC_SCTRL_SCA53_EVENT_MASK_lpmcu_event_mask_START (3)
#define SOC_SCTRL_SCA53_EVENT_MASK_lpmcu_event_mask_END (3)
#define SOC_SCTRL_SCA53_EVENT_MASK_mcpu_event_mask_START (4)
#define SOC_SCTRL_SCA53_EVENT_MASK_mcpu_event_mask_END (4)
#define SOC_SCTRL_SCA53_EVENT_MASK_ispa7_event_mask_START (5)
#define SOC_SCTRL_SCA53_EVENT_MASK_ispa7_event_mask_END (5)
#define SOC_SCTRL_SCA53_EVENT_MASK_hifd_ul_event_mask_START (6)
#define SOC_SCTRL_SCA53_EVENT_MASK_hifd_ul_event_mask_END (6)
#define SOC_SCTRL_SCA53_EVENT_MASK_eps_event_mask_START (7)
#define SOC_SCTRL_SCA53_EVENT_MASK_eps_event_mask_END (7)
#define SOC_SCTRL_SCA53_EVENT_MASK_npu_archs_event_mask_START (8)
#define SOC_SCTRL_SCA53_EVENT_MASK_npu_archs_event_mask_END (8)
#define SOC_SCTRL_SCA53_EVENT_MASK_npu_tscpu_event_mask_START (9)
#define SOC_SCTRL_SCA53_EVENT_MASK_npu_tscpu_event_mask_END (9)
#define SOC_SCTRL_SCA53_EVENT_MASK_npu_aicpu_event_mask_START (10)
#define SOC_SCTRL_SCA53_EVENT_MASK_npu_aicpu_event_mask_END (10)
#define SOC_SCTRL_SCA53_EVENT_MASK_mdm5g_l2hac_event_mask_START (11)
#define SOC_SCTRL_SCA53_EVENT_MASK_mdm5g_l2hac_event_mask_END (11)
#define SOC_SCTRL_SCA53_EVENT_MASK_mdm5g_ccpu5g_event_mask_START (12)
#define SOC_SCTRL_SCA53_EVENT_MASK_mdm5g_ccpu5g_event_mask_END (12)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int a53_event_mask : 1;
        unsigned int a57_event_mask : 1;
        unsigned int iomcu_event_mask : 1;
        unsigned int lpmcu_event_mask : 1;
        unsigned int mcpu_event_mask : 1;
        unsigned int ispa7_event_mask : 1;
        unsigned int hifd_ul_event_mask : 1;
        unsigned int eps_event_mask : 1;
        unsigned int npu_archs_event_mask : 1;
        unsigned int npu_tscpu_event_mask : 1;
        unsigned int npu_aicpu_event_mask : 1;
        unsigned int mdm5g_l2hac_event_mask : 1;
        unsigned int mdm5g_ccpu5g_event_mask : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int reserved_11 : 1;
        unsigned int reserved_12 : 1;
        unsigned int reserved_13 : 1;
        unsigned int reserved_14 : 1;
        unsigned int reserved_15 : 1;
        unsigned int reserved_16 : 1;
        unsigned int reserved_17 : 1;
        unsigned int reserved_18 : 1;
    } reg;
} SOC_SCTRL_SCA57_EVENT_MASK_UNION;
#endif
#define SOC_SCTRL_SCA57_EVENT_MASK_a53_event_mask_START (0)
#define SOC_SCTRL_SCA57_EVENT_MASK_a53_event_mask_END (0)
#define SOC_SCTRL_SCA57_EVENT_MASK_a57_event_mask_START (1)
#define SOC_SCTRL_SCA57_EVENT_MASK_a57_event_mask_END (1)
#define SOC_SCTRL_SCA57_EVENT_MASK_iomcu_event_mask_START (2)
#define SOC_SCTRL_SCA57_EVENT_MASK_iomcu_event_mask_END (2)
#define SOC_SCTRL_SCA57_EVENT_MASK_lpmcu_event_mask_START (3)
#define SOC_SCTRL_SCA57_EVENT_MASK_lpmcu_event_mask_END (3)
#define SOC_SCTRL_SCA57_EVENT_MASK_mcpu_event_mask_START (4)
#define SOC_SCTRL_SCA57_EVENT_MASK_mcpu_event_mask_END (4)
#define SOC_SCTRL_SCA57_EVENT_MASK_ispa7_event_mask_START (5)
#define SOC_SCTRL_SCA57_EVENT_MASK_ispa7_event_mask_END (5)
#define SOC_SCTRL_SCA57_EVENT_MASK_hifd_ul_event_mask_START (6)
#define SOC_SCTRL_SCA57_EVENT_MASK_hifd_ul_event_mask_END (6)
#define SOC_SCTRL_SCA57_EVENT_MASK_eps_event_mask_START (7)
#define SOC_SCTRL_SCA57_EVENT_MASK_eps_event_mask_END (7)
#define SOC_SCTRL_SCA57_EVENT_MASK_npu_archs_event_mask_START (8)
#define SOC_SCTRL_SCA57_EVENT_MASK_npu_archs_event_mask_END (8)
#define SOC_SCTRL_SCA57_EVENT_MASK_npu_tscpu_event_mask_START (9)
#define SOC_SCTRL_SCA57_EVENT_MASK_npu_tscpu_event_mask_END (9)
#define SOC_SCTRL_SCA57_EVENT_MASK_npu_aicpu_event_mask_START (10)
#define SOC_SCTRL_SCA57_EVENT_MASK_npu_aicpu_event_mask_END (10)
#define SOC_SCTRL_SCA57_EVENT_MASK_mdm5g_l2hac_event_mask_START (11)
#define SOC_SCTRL_SCA57_EVENT_MASK_mdm5g_l2hac_event_mask_END (11)
#define SOC_SCTRL_SCA57_EVENT_MASK_mdm5g_ccpu5g_event_mask_START (12)
#define SOC_SCTRL_SCA57_EVENT_MASK_mdm5g_ccpu5g_event_mask_END (12)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int a53_event_mask : 1;
        unsigned int a57_event_mask : 1;
        unsigned int iomcu_event_mask : 1;
        unsigned int lpmcu_event_mask : 1;
        unsigned int mcpu_event_mask : 1;
        unsigned int ispa7_event_mask : 1;
        unsigned int hifd_ul_event_mask : 1;
        unsigned int eps_event_mask : 1;
        unsigned int npu_archs_event_mask : 1;
        unsigned int npu_tscpu_event_mask : 1;
        unsigned int npu_aicpu_event_mask : 1;
        unsigned int mdm5g_l2hac_event_mask : 1;
        unsigned int mdm5g_ccpu5g_event_mask : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int reserved_11 : 1;
        unsigned int reserved_12 : 1;
        unsigned int reserved_13 : 1;
        unsigned int reserved_14 : 1;
        unsigned int reserved_15 : 1;
        unsigned int reserved_16 : 1;
        unsigned int reserved_17 : 1;
        unsigned int reserved_18 : 1;
    } reg;
} SOC_SCTRL_SCIOMCU_EVENT_MASK_UNION;
#endif
#define SOC_SCTRL_SCIOMCU_EVENT_MASK_a53_event_mask_START (0)
#define SOC_SCTRL_SCIOMCU_EVENT_MASK_a53_event_mask_END (0)
#define SOC_SCTRL_SCIOMCU_EVENT_MASK_a57_event_mask_START (1)
#define SOC_SCTRL_SCIOMCU_EVENT_MASK_a57_event_mask_END (1)
#define SOC_SCTRL_SCIOMCU_EVENT_MASK_iomcu_event_mask_START (2)
#define SOC_SCTRL_SCIOMCU_EVENT_MASK_iomcu_event_mask_END (2)
#define SOC_SCTRL_SCIOMCU_EVENT_MASK_lpmcu_event_mask_START (3)
#define SOC_SCTRL_SCIOMCU_EVENT_MASK_lpmcu_event_mask_END (3)
#define SOC_SCTRL_SCIOMCU_EVENT_MASK_mcpu_event_mask_START (4)
#define SOC_SCTRL_SCIOMCU_EVENT_MASK_mcpu_event_mask_END (4)
#define SOC_SCTRL_SCIOMCU_EVENT_MASK_ispa7_event_mask_START (5)
#define SOC_SCTRL_SCIOMCU_EVENT_MASK_ispa7_event_mask_END (5)
#define SOC_SCTRL_SCIOMCU_EVENT_MASK_hifd_ul_event_mask_START (6)
#define SOC_SCTRL_SCIOMCU_EVENT_MASK_hifd_ul_event_mask_END (6)
#define SOC_SCTRL_SCIOMCU_EVENT_MASK_eps_event_mask_START (7)
#define SOC_SCTRL_SCIOMCU_EVENT_MASK_eps_event_mask_END (7)
#define SOC_SCTRL_SCIOMCU_EVENT_MASK_npu_archs_event_mask_START (8)
#define SOC_SCTRL_SCIOMCU_EVENT_MASK_npu_archs_event_mask_END (8)
#define SOC_SCTRL_SCIOMCU_EVENT_MASK_npu_tscpu_event_mask_START (9)
#define SOC_SCTRL_SCIOMCU_EVENT_MASK_npu_tscpu_event_mask_END (9)
#define SOC_SCTRL_SCIOMCU_EVENT_MASK_npu_aicpu_event_mask_START (10)
#define SOC_SCTRL_SCIOMCU_EVENT_MASK_npu_aicpu_event_mask_END (10)
#define SOC_SCTRL_SCIOMCU_EVENT_MASK_mdm5g_l2hac_event_mask_START (11)
#define SOC_SCTRL_SCIOMCU_EVENT_MASK_mdm5g_l2hac_event_mask_END (11)
#define SOC_SCTRL_SCIOMCU_EVENT_MASK_mdm5g_ccpu5g_event_mask_START (12)
#define SOC_SCTRL_SCIOMCU_EVENT_MASK_mdm5g_ccpu5g_event_mask_END (12)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int a53_event_mask : 1;
        unsigned int a57_event_mask : 1;
        unsigned int iomcu_event_mask : 1;
        unsigned int lpmcu_event_mask : 1;
        unsigned int mcpu_event_mask : 1;
        unsigned int ispa7_event_mask : 1;
        unsigned int hifd_ul_event_mask : 1;
        unsigned int eps_event_mask : 1;
        unsigned int npu_archs_event_mask : 1;
        unsigned int npu_tscpu_event_mask : 1;
        unsigned int npu_aicpu_event_mask : 1;
        unsigned int mdm5g_l2hac_event_mask : 1;
        unsigned int mdm5g_ccpu5g_event_mask : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int reserved_11 : 1;
        unsigned int reserved_12 : 1;
        unsigned int reserved_13 : 1;
        unsigned int reserved_14 : 1;
        unsigned int reserved_15 : 1;
        unsigned int reserved_16 : 1;
        unsigned int reserved_17 : 1;
        unsigned int reserved_18 : 1;
    } reg;
} SOC_SCTRL_SCLPMCU_EVENT_MASK_UNION;
#endif
#define SOC_SCTRL_SCLPMCU_EVENT_MASK_a53_event_mask_START (0)
#define SOC_SCTRL_SCLPMCU_EVENT_MASK_a53_event_mask_END (0)
#define SOC_SCTRL_SCLPMCU_EVENT_MASK_a57_event_mask_START (1)
#define SOC_SCTRL_SCLPMCU_EVENT_MASK_a57_event_mask_END (1)
#define SOC_SCTRL_SCLPMCU_EVENT_MASK_iomcu_event_mask_START (2)
#define SOC_SCTRL_SCLPMCU_EVENT_MASK_iomcu_event_mask_END (2)
#define SOC_SCTRL_SCLPMCU_EVENT_MASK_lpmcu_event_mask_START (3)
#define SOC_SCTRL_SCLPMCU_EVENT_MASK_lpmcu_event_mask_END (3)
#define SOC_SCTRL_SCLPMCU_EVENT_MASK_mcpu_event_mask_START (4)
#define SOC_SCTRL_SCLPMCU_EVENT_MASK_mcpu_event_mask_END (4)
#define SOC_SCTRL_SCLPMCU_EVENT_MASK_ispa7_event_mask_START (5)
#define SOC_SCTRL_SCLPMCU_EVENT_MASK_ispa7_event_mask_END (5)
#define SOC_SCTRL_SCLPMCU_EVENT_MASK_hifd_ul_event_mask_START (6)
#define SOC_SCTRL_SCLPMCU_EVENT_MASK_hifd_ul_event_mask_END (6)
#define SOC_SCTRL_SCLPMCU_EVENT_MASK_eps_event_mask_START (7)
#define SOC_SCTRL_SCLPMCU_EVENT_MASK_eps_event_mask_END (7)
#define SOC_SCTRL_SCLPMCU_EVENT_MASK_npu_archs_event_mask_START (8)
#define SOC_SCTRL_SCLPMCU_EVENT_MASK_npu_archs_event_mask_END (8)
#define SOC_SCTRL_SCLPMCU_EVENT_MASK_npu_tscpu_event_mask_START (9)
#define SOC_SCTRL_SCLPMCU_EVENT_MASK_npu_tscpu_event_mask_END (9)
#define SOC_SCTRL_SCLPMCU_EVENT_MASK_npu_aicpu_event_mask_START (10)
#define SOC_SCTRL_SCLPMCU_EVENT_MASK_npu_aicpu_event_mask_END (10)
#define SOC_SCTRL_SCLPMCU_EVENT_MASK_mdm5g_l2hac_event_mask_START (11)
#define SOC_SCTRL_SCLPMCU_EVENT_MASK_mdm5g_l2hac_event_mask_END (11)
#define SOC_SCTRL_SCLPMCU_EVENT_MASK_mdm5g_ccpu5g_event_mask_START (12)
#define SOC_SCTRL_SCLPMCU_EVENT_MASK_mdm5g_ccpu5g_event_mask_END (12)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int a53_event_mask : 1;
        unsigned int a57_event_mask : 1;
        unsigned int iomcu_event_mask : 1;
        unsigned int lpmcu_event_mask : 1;
        unsigned int mcpu_event_mask : 1;
        unsigned int ispa7_event_mask : 1;
        unsigned int hifd_ul_event_mask : 1;
        unsigned int eps_event_mask : 1;
        unsigned int npu_archs_event_mask : 1;
        unsigned int npu_tscpu_event_mask : 1;
        unsigned int npu_aicpu_event_mask : 1;
        unsigned int mdm5g_l2hac_event_mask : 1;
        unsigned int mdm5g_ccpu5g_event_mask : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int reserved_11 : 1;
        unsigned int reserved_12 : 1;
        unsigned int reserved_13 : 1;
        unsigned int reserved_14 : 1;
        unsigned int reserved_15 : 1;
        unsigned int reserved_16 : 1;
        unsigned int reserved_17 : 1;
        unsigned int reserved_18 : 1;
    } reg;
} SOC_SCTRL_SCMCPU_EVENT_MASK_UNION;
#endif
#define SOC_SCTRL_SCMCPU_EVENT_MASK_a53_event_mask_START (0)
#define SOC_SCTRL_SCMCPU_EVENT_MASK_a53_event_mask_END (0)
#define SOC_SCTRL_SCMCPU_EVENT_MASK_a57_event_mask_START (1)
#define SOC_SCTRL_SCMCPU_EVENT_MASK_a57_event_mask_END (1)
#define SOC_SCTRL_SCMCPU_EVENT_MASK_iomcu_event_mask_START (2)
#define SOC_SCTRL_SCMCPU_EVENT_MASK_iomcu_event_mask_END (2)
#define SOC_SCTRL_SCMCPU_EVENT_MASK_lpmcu_event_mask_START (3)
#define SOC_SCTRL_SCMCPU_EVENT_MASK_lpmcu_event_mask_END (3)
#define SOC_SCTRL_SCMCPU_EVENT_MASK_mcpu_event_mask_START (4)
#define SOC_SCTRL_SCMCPU_EVENT_MASK_mcpu_event_mask_END (4)
#define SOC_SCTRL_SCMCPU_EVENT_MASK_ispa7_event_mask_START (5)
#define SOC_SCTRL_SCMCPU_EVENT_MASK_ispa7_event_mask_END (5)
#define SOC_SCTRL_SCMCPU_EVENT_MASK_hifd_ul_event_mask_START (6)
#define SOC_SCTRL_SCMCPU_EVENT_MASK_hifd_ul_event_mask_END (6)
#define SOC_SCTRL_SCMCPU_EVENT_MASK_eps_event_mask_START (7)
#define SOC_SCTRL_SCMCPU_EVENT_MASK_eps_event_mask_END (7)
#define SOC_SCTRL_SCMCPU_EVENT_MASK_npu_archs_event_mask_START (8)
#define SOC_SCTRL_SCMCPU_EVENT_MASK_npu_archs_event_mask_END (8)
#define SOC_SCTRL_SCMCPU_EVENT_MASK_npu_tscpu_event_mask_START (9)
#define SOC_SCTRL_SCMCPU_EVENT_MASK_npu_tscpu_event_mask_END (9)
#define SOC_SCTRL_SCMCPU_EVENT_MASK_npu_aicpu_event_mask_START (10)
#define SOC_SCTRL_SCMCPU_EVENT_MASK_npu_aicpu_event_mask_END (10)
#define SOC_SCTRL_SCMCPU_EVENT_MASK_mdm5g_l2hac_event_mask_START (11)
#define SOC_SCTRL_SCMCPU_EVENT_MASK_mdm5g_l2hac_event_mask_END (11)
#define SOC_SCTRL_SCMCPU_EVENT_MASK_mdm5g_ccpu5g_event_mask_START (12)
#define SOC_SCTRL_SCMCPU_EVENT_MASK_mdm5g_ccpu5g_event_mask_END (12)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int a53_event_mask : 1;
        unsigned int a57_event_mask : 1;
        unsigned int iomcu_event_mask : 1;
        unsigned int lpmcu_event_mask : 1;
        unsigned int mcpu_event_mask : 1;
        unsigned int ispa7_event_mask : 1;
        unsigned int hifd_ul_event_mask : 1;
        unsigned int eps_event_mask : 1;
        unsigned int npu_archs_event_mask : 1;
        unsigned int npu_tscpu_event_mask : 1;
        unsigned int npu_aicpu_event_mask : 1;
        unsigned int mdm5g_l2hac_event_mask : 1;
        unsigned int mdm5g_ccpu5g_event_mask : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int reserved_11 : 1;
        unsigned int reserved_12 : 1;
        unsigned int reserved_13 : 1;
        unsigned int reserved_14 : 1;
        unsigned int reserved_15 : 1;
        unsigned int reserved_16 : 1;
        unsigned int reserved_17 : 1;
        unsigned int reserved_18 : 1;
    } reg;
} SOC_SCTRL_SCISPA7_EVENT_MASK_UNION;
#endif
#define SOC_SCTRL_SCISPA7_EVENT_MASK_a53_event_mask_START (0)
#define SOC_SCTRL_SCISPA7_EVENT_MASK_a53_event_mask_END (0)
#define SOC_SCTRL_SCISPA7_EVENT_MASK_a57_event_mask_START (1)
#define SOC_SCTRL_SCISPA7_EVENT_MASK_a57_event_mask_END (1)
#define SOC_SCTRL_SCISPA7_EVENT_MASK_iomcu_event_mask_START (2)
#define SOC_SCTRL_SCISPA7_EVENT_MASK_iomcu_event_mask_END (2)
#define SOC_SCTRL_SCISPA7_EVENT_MASK_lpmcu_event_mask_START (3)
#define SOC_SCTRL_SCISPA7_EVENT_MASK_lpmcu_event_mask_END (3)
#define SOC_SCTRL_SCISPA7_EVENT_MASK_mcpu_event_mask_START (4)
#define SOC_SCTRL_SCISPA7_EVENT_MASK_mcpu_event_mask_END (4)
#define SOC_SCTRL_SCISPA7_EVENT_MASK_ispa7_event_mask_START (5)
#define SOC_SCTRL_SCISPA7_EVENT_MASK_ispa7_event_mask_END (5)
#define SOC_SCTRL_SCISPA7_EVENT_MASK_hifd_ul_event_mask_START (6)
#define SOC_SCTRL_SCISPA7_EVENT_MASK_hifd_ul_event_mask_END (6)
#define SOC_SCTRL_SCISPA7_EVENT_MASK_eps_event_mask_START (7)
#define SOC_SCTRL_SCISPA7_EVENT_MASK_eps_event_mask_END (7)
#define SOC_SCTRL_SCISPA7_EVENT_MASK_npu_archs_event_mask_START (8)
#define SOC_SCTRL_SCISPA7_EVENT_MASK_npu_archs_event_mask_END (8)
#define SOC_SCTRL_SCISPA7_EVENT_MASK_npu_tscpu_event_mask_START (9)
#define SOC_SCTRL_SCISPA7_EVENT_MASK_npu_tscpu_event_mask_END (9)
#define SOC_SCTRL_SCISPA7_EVENT_MASK_npu_aicpu_event_mask_START (10)
#define SOC_SCTRL_SCISPA7_EVENT_MASK_npu_aicpu_event_mask_END (10)
#define SOC_SCTRL_SCISPA7_EVENT_MASK_mdm5g_l2hac_event_mask_START (11)
#define SOC_SCTRL_SCISPA7_EVENT_MASK_mdm5g_l2hac_event_mask_END (11)
#define SOC_SCTRL_SCISPA7_EVENT_MASK_mdm5g_ccpu5g_event_mask_START (12)
#define SOC_SCTRL_SCISPA7_EVENT_MASK_mdm5g_ccpu5g_event_mask_END (12)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int a53_event_mask : 1;
        unsigned int a57_event_mask : 1;
        unsigned int iomcu_event_mask : 1;
        unsigned int lpmcu_event_mask : 1;
        unsigned int mcpu_event_mask : 1;
        unsigned int ispa7_event_mask : 1;
        unsigned int hifd_ul_event_mask : 1;
        unsigned int eps_event_mask : 1;
        unsigned int npu_archs_event_mask : 1;
        unsigned int npu_tscpu_event_mask : 1;
        unsigned int npu_aicpu_event_mask : 1;
        unsigned int mdm5g_l2hac_event_mask : 1;
        unsigned int mdm5g_ccpu5g_event_mask : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int reserved_11 : 1;
        unsigned int reserved_12 : 1;
        unsigned int reserved_13 : 1;
        unsigned int reserved_14 : 1;
        unsigned int reserved_15 : 1;
        unsigned int reserved_16 : 1;
        unsigned int reserved_17 : 1;
        unsigned int reserved_18 : 1;
    } reg;
} SOC_SCTRL_SCHIFD_UL_EVENT_MASK_UNION;
#endif
#define SOC_SCTRL_SCHIFD_UL_EVENT_MASK_a53_event_mask_START (0)
#define SOC_SCTRL_SCHIFD_UL_EVENT_MASK_a53_event_mask_END (0)
#define SOC_SCTRL_SCHIFD_UL_EVENT_MASK_a57_event_mask_START (1)
#define SOC_SCTRL_SCHIFD_UL_EVENT_MASK_a57_event_mask_END (1)
#define SOC_SCTRL_SCHIFD_UL_EVENT_MASK_iomcu_event_mask_START (2)
#define SOC_SCTRL_SCHIFD_UL_EVENT_MASK_iomcu_event_mask_END (2)
#define SOC_SCTRL_SCHIFD_UL_EVENT_MASK_lpmcu_event_mask_START (3)
#define SOC_SCTRL_SCHIFD_UL_EVENT_MASK_lpmcu_event_mask_END (3)
#define SOC_SCTRL_SCHIFD_UL_EVENT_MASK_mcpu_event_mask_START (4)
#define SOC_SCTRL_SCHIFD_UL_EVENT_MASK_mcpu_event_mask_END (4)
#define SOC_SCTRL_SCHIFD_UL_EVENT_MASK_ispa7_event_mask_START (5)
#define SOC_SCTRL_SCHIFD_UL_EVENT_MASK_ispa7_event_mask_END (5)
#define SOC_SCTRL_SCHIFD_UL_EVENT_MASK_hifd_ul_event_mask_START (6)
#define SOC_SCTRL_SCHIFD_UL_EVENT_MASK_hifd_ul_event_mask_END (6)
#define SOC_SCTRL_SCHIFD_UL_EVENT_MASK_eps_event_mask_START (7)
#define SOC_SCTRL_SCHIFD_UL_EVENT_MASK_eps_event_mask_END (7)
#define SOC_SCTRL_SCHIFD_UL_EVENT_MASK_npu_archs_event_mask_START (8)
#define SOC_SCTRL_SCHIFD_UL_EVENT_MASK_npu_archs_event_mask_END (8)
#define SOC_SCTRL_SCHIFD_UL_EVENT_MASK_npu_tscpu_event_mask_START (9)
#define SOC_SCTRL_SCHIFD_UL_EVENT_MASK_npu_tscpu_event_mask_END (9)
#define SOC_SCTRL_SCHIFD_UL_EVENT_MASK_npu_aicpu_event_mask_START (10)
#define SOC_SCTRL_SCHIFD_UL_EVENT_MASK_npu_aicpu_event_mask_END (10)
#define SOC_SCTRL_SCHIFD_UL_EVENT_MASK_mdm5g_l2hac_event_mask_START (11)
#define SOC_SCTRL_SCHIFD_UL_EVENT_MASK_mdm5g_l2hac_event_mask_END (11)
#define SOC_SCTRL_SCHIFD_UL_EVENT_MASK_mdm5g_ccpu5g_event_mask_START (12)
#define SOC_SCTRL_SCHIFD_UL_EVENT_MASK_mdm5g_ccpu5g_event_mask_END (12)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int a53_event_mask : 1;
        unsigned int a57_event_mask : 1;
        unsigned int iomcu_event_mask : 1;
        unsigned int lpmcu_event_mask : 1;
        unsigned int mcpu_event_mask : 1;
        unsigned int ispa7_event_mask : 1;
        unsigned int hifd_ul_event_mask : 1;
        unsigned int eps_event_mask : 1;
        unsigned int npu_archs_event_mask : 1;
        unsigned int npu_tscpu_event_mask : 1;
        unsigned int npu_aicpu_event_mask : 1;
        unsigned int mdm5g_l2hac_event_mask : 1;
        unsigned int mdm5g_ccpu5g_event_mask : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int reserved_11 : 1;
        unsigned int reserved_12 : 1;
        unsigned int reserved_13 : 1;
        unsigned int reserved_14 : 1;
        unsigned int reserved_15 : 1;
        unsigned int reserved_16 : 1;
        unsigned int reserved_17 : 1;
        unsigned int reserved_18 : 1;
    } reg;
} SOC_SCTRL_SCNPU_AICPU_EVENT_MASK_UNION;
#endif
#define SOC_SCTRL_SCNPU_AICPU_EVENT_MASK_a53_event_mask_START (0)
#define SOC_SCTRL_SCNPU_AICPU_EVENT_MASK_a53_event_mask_END (0)
#define SOC_SCTRL_SCNPU_AICPU_EVENT_MASK_a57_event_mask_START (1)
#define SOC_SCTRL_SCNPU_AICPU_EVENT_MASK_a57_event_mask_END (1)
#define SOC_SCTRL_SCNPU_AICPU_EVENT_MASK_iomcu_event_mask_START (2)
#define SOC_SCTRL_SCNPU_AICPU_EVENT_MASK_iomcu_event_mask_END (2)
#define SOC_SCTRL_SCNPU_AICPU_EVENT_MASK_lpmcu_event_mask_START (3)
#define SOC_SCTRL_SCNPU_AICPU_EVENT_MASK_lpmcu_event_mask_END (3)
#define SOC_SCTRL_SCNPU_AICPU_EVENT_MASK_mcpu_event_mask_START (4)
#define SOC_SCTRL_SCNPU_AICPU_EVENT_MASK_mcpu_event_mask_END (4)
#define SOC_SCTRL_SCNPU_AICPU_EVENT_MASK_ispa7_event_mask_START (5)
#define SOC_SCTRL_SCNPU_AICPU_EVENT_MASK_ispa7_event_mask_END (5)
#define SOC_SCTRL_SCNPU_AICPU_EVENT_MASK_hifd_ul_event_mask_START (6)
#define SOC_SCTRL_SCNPU_AICPU_EVENT_MASK_hifd_ul_event_mask_END (6)
#define SOC_SCTRL_SCNPU_AICPU_EVENT_MASK_eps_event_mask_START (7)
#define SOC_SCTRL_SCNPU_AICPU_EVENT_MASK_eps_event_mask_END (7)
#define SOC_SCTRL_SCNPU_AICPU_EVENT_MASK_npu_archs_event_mask_START (8)
#define SOC_SCTRL_SCNPU_AICPU_EVENT_MASK_npu_archs_event_mask_END (8)
#define SOC_SCTRL_SCNPU_AICPU_EVENT_MASK_npu_tscpu_event_mask_START (9)
#define SOC_SCTRL_SCNPU_AICPU_EVENT_MASK_npu_tscpu_event_mask_END (9)
#define SOC_SCTRL_SCNPU_AICPU_EVENT_MASK_npu_aicpu_event_mask_START (10)
#define SOC_SCTRL_SCNPU_AICPU_EVENT_MASK_npu_aicpu_event_mask_END (10)
#define SOC_SCTRL_SCNPU_AICPU_EVENT_MASK_mdm5g_l2hac_event_mask_START (11)
#define SOC_SCTRL_SCNPU_AICPU_EVENT_MASK_mdm5g_l2hac_event_mask_END (11)
#define SOC_SCTRL_SCNPU_AICPU_EVENT_MASK_mdm5g_ccpu5g_event_mask_START (12)
#define SOC_SCTRL_SCNPU_AICPU_EVENT_MASK_mdm5g_ccpu5g_event_mask_END (12)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int a53_event_mask : 1;
        unsigned int a57_event_mask : 1;
        unsigned int iomcu_event_mask : 1;
        unsigned int lpmcu_event_mask : 1;
        unsigned int mcpu_event_mask : 1;
        unsigned int ispa7_event_mask : 1;
        unsigned int hifd_ul_event_mask : 1;
        unsigned int eps_event_mask : 1;
        unsigned int npu_archs_event_mask : 1;
        unsigned int npu_tscpu_event_mask : 1;
        unsigned int npu_aicpu_event_mask : 1;
        unsigned int mdm5g_l2hac_event_mask : 1;
        unsigned int mdm5g_ccpu5g_event_mask : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int reserved_11 : 1;
        unsigned int reserved_12 : 1;
        unsigned int reserved_13 : 1;
        unsigned int reserved_14 : 1;
        unsigned int reserved_15 : 1;
        unsigned int reserved_16 : 1;
        unsigned int reserved_17 : 1;
        unsigned int reserved_18 : 1;
    } reg;
} SOC_SCTRL_SCNPU_TSCPU_EVENT_MASK_UNION;
#endif
#define SOC_SCTRL_SCNPU_TSCPU_EVENT_MASK_a53_event_mask_START (0)
#define SOC_SCTRL_SCNPU_TSCPU_EVENT_MASK_a53_event_mask_END (0)
#define SOC_SCTRL_SCNPU_TSCPU_EVENT_MASK_a57_event_mask_START (1)
#define SOC_SCTRL_SCNPU_TSCPU_EVENT_MASK_a57_event_mask_END (1)
#define SOC_SCTRL_SCNPU_TSCPU_EVENT_MASK_iomcu_event_mask_START (2)
#define SOC_SCTRL_SCNPU_TSCPU_EVENT_MASK_iomcu_event_mask_END (2)
#define SOC_SCTRL_SCNPU_TSCPU_EVENT_MASK_lpmcu_event_mask_START (3)
#define SOC_SCTRL_SCNPU_TSCPU_EVENT_MASK_lpmcu_event_mask_END (3)
#define SOC_SCTRL_SCNPU_TSCPU_EVENT_MASK_mcpu_event_mask_START (4)
#define SOC_SCTRL_SCNPU_TSCPU_EVENT_MASK_mcpu_event_mask_END (4)
#define SOC_SCTRL_SCNPU_TSCPU_EVENT_MASK_ispa7_event_mask_START (5)
#define SOC_SCTRL_SCNPU_TSCPU_EVENT_MASK_ispa7_event_mask_END (5)
#define SOC_SCTRL_SCNPU_TSCPU_EVENT_MASK_hifd_ul_event_mask_START (6)
#define SOC_SCTRL_SCNPU_TSCPU_EVENT_MASK_hifd_ul_event_mask_END (6)
#define SOC_SCTRL_SCNPU_TSCPU_EVENT_MASK_eps_event_mask_START (7)
#define SOC_SCTRL_SCNPU_TSCPU_EVENT_MASK_eps_event_mask_END (7)
#define SOC_SCTRL_SCNPU_TSCPU_EVENT_MASK_npu_archs_event_mask_START (8)
#define SOC_SCTRL_SCNPU_TSCPU_EVENT_MASK_npu_archs_event_mask_END (8)
#define SOC_SCTRL_SCNPU_TSCPU_EVENT_MASK_npu_tscpu_event_mask_START (9)
#define SOC_SCTRL_SCNPU_TSCPU_EVENT_MASK_npu_tscpu_event_mask_END (9)
#define SOC_SCTRL_SCNPU_TSCPU_EVENT_MASK_npu_aicpu_event_mask_START (10)
#define SOC_SCTRL_SCNPU_TSCPU_EVENT_MASK_npu_aicpu_event_mask_END (10)
#define SOC_SCTRL_SCNPU_TSCPU_EVENT_MASK_mdm5g_l2hac_event_mask_START (11)
#define SOC_SCTRL_SCNPU_TSCPU_EVENT_MASK_mdm5g_l2hac_event_mask_END (11)
#define SOC_SCTRL_SCNPU_TSCPU_EVENT_MASK_mdm5g_ccpu5g_event_mask_START (12)
#define SOC_SCTRL_SCNPU_TSCPU_EVENT_MASK_mdm5g_ccpu5g_event_mask_END (12)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int a53_event_mask : 1;
        unsigned int a57_event_mask : 1;
        unsigned int iomcu_event_mask : 1;
        unsigned int lpmcu_event_mask : 1;
        unsigned int mcpu_event_mask : 1;
        unsigned int ispa7_event_mask : 1;
        unsigned int hifd_ul_event_mask : 1;
        unsigned int eps_event_mask : 1;
        unsigned int npu_archs_event_mask : 1;
        unsigned int npu_tscpu_event_mask : 1;
        unsigned int npu_aicpu_event_mask : 1;
        unsigned int mdm5g_l2hac_event_mask : 1;
        unsigned int mdm5g_ccpu5g_event_mask : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int reserved_11 : 1;
        unsigned int reserved_12 : 1;
        unsigned int reserved_13 : 1;
        unsigned int reserved_14 : 1;
        unsigned int reserved_15 : 1;
        unsigned int reserved_16 : 1;
        unsigned int reserved_17 : 1;
        unsigned int reserved_18 : 1;
    } reg;
} SOC_SCTRL_SCNPU_ARCHS_EVENT_MASK_UNION;
#endif
#define SOC_SCTRL_SCNPU_ARCHS_EVENT_MASK_a53_event_mask_START (0)
#define SOC_SCTRL_SCNPU_ARCHS_EVENT_MASK_a53_event_mask_END (0)
#define SOC_SCTRL_SCNPU_ARCHS_EVENT_MASK_a57_event_mask_START (1)
#define SOC_SCTRL_SCNPU_ARCHS_EVENT_MASK_a57_event_mask_END (1)
#define SOC_SCTRL_SCNPU_ARCHS_EVENT_MASK_iomcu_event_mask_START (2)
#define SOC_SCTRL_SCNPU_ARCHS_EVENT_MASK_iomcu_event_mask_END (2)
#define SOC_SCTRL_SCNPU_ARCHS_EVENT_MASK_lpmcu_event_mask_START (3)
#define SOC_SCTRL_SCNPU_ARCHS_EVENT_MASK_lpmcu_event_mask_END (3)
#define SOC_SCTRL_SCNPU_ARCHS_EVENT_MASK_mcpu_event_mask_START (4)
#define SOC_SCTRL_SCNPU_ARCHS_EVENT_MASK_mcpu_event_mask_END (4)
#define SOC_SCTRL_SCNPU_ARCHS_EVENT_MASK_ispa7_event_mask_START (5)
#define SOC_SCTRL_SCNPU_ARCHS_EVENT_MASK_ispa7_event_mask_END (5)
#define SOC_SCTRL_SCNPU_ARCHS_EVENT_MASK_hifd_ul_event_mask_START (6)
#define SOC_SCTRL_SCNPU_ARCHS_EVENT_MASK_hifd_ul_event_mask_END (6)
#define SOC_SCTRL_SCNPU_ARCHS_EVENT_MASK_eps_event_mask_START (7)
#define SOC_SCTRL_SCNPU_ARCHS_EVENT_MASK_eps_event_mask_END (7)
#define SOC_SCTRL_SCNPU_ARCHS_EVENT_MASK_npu_archs_event_mask_START (8)
#define SOC_SCTRL_SCNPU_ARCHS_EVENT_MASK_npu_archs_event_mask_END (8)
#define SOC_SCTRL_SCNPU_ARCHS_EVENT_MASK_npu_tscpu_event_mask_START (9)
#define SOC_SCTRL_SCNPU_ARCHS_EVENT_MASK_npu_tscpu_event_mask_END (9)
#define SOC_SCTRL_SCNPU_ARCHS_EVENT_MASK_npu_aicpu_event_mask_START (10)
#define SOC_SCTRL_SCNPU_ARCHS_EVENT_MASK_npu_aicpu_event_mask_END (10)
#define SOC_SCTRL_SCNPU_ARCHS_EVENT_MASK_mdm5g_l2hac_event_mask_START (11)
#define SOC_SCTRL_SCNPU_ARCHS_EVENT_MASK_mdm5g_l2hac_event_mask_END (11)
#define SOC_SCTRL_SCNPU_ARCHS_EVENT_MASK_mdm5g_ccpu5g_event_mask_START (12)
#define SOC_SCTRL_SCNPU_ARCHS_EVENT_MASK_mdm5g_ccpu5g_event_mask_END (12)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int a53_event_mask : 1;
        unsigned int a57_event_mask : 1;
        unsigned int iomcu_event_mask : 1;
        unsigned int lpmcu_event_mask : 1;
        unsigned int mcpu_event_mask : 1;
        unsigned int ispa7_event_mask : 1;
        unsigned int hifd_ul_event_mask : 1;
        unsigned int eps_event_mask : 1;
        unsigned int npu_archs_event_mask : 1;
        unsigned int npu_tscpu_event_mask : 1;
        unsigned int npu_aicpu_event_mask : 1;
        unsigned int mdm5g_l2hac_event_mask : 1;
        unsigned int mdm5g_ccpu5g_event_mask : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int reserved_11 : 1;
        unsigned int reserved_12 : 1;
        unsigned int reserved_13 : 1;
        unsigned int reserved_14 : 1;
        unsigned int reserved_15 : 1;
        unsigned int reserved_16 : 1;
        unsigned int reserved_17 : 1;
        unsigned int reserved_18 : 1;
    } reg;
} SOC_SCTRL_SCEPS_EVENT_MASK_UNION;
#endif
#define SOC_SCTRL_SCEPS_EVENT_MASK_a53_event_mask_START (0)
#define SOC_SCTRL_SCEPS_EVENT_MASK_a53_event_mask_END (0)
#define SOC_SCTRL_SCEPS_EVENT_MASK_a57_event_mask_START (1)
#define SOC_SCTRL_SCEPS_EVENT_MASK_a57_event_mask_END (1)
#define SOC_SCTRL_SCEPS_EVENT_MASK_iomcu_event_mask_START (2)
#define SOC_SCTRL_SCEPS_EVENT_MASK_iomcu_event_mask_END (2)
#define SOC_SCTRL_SCEPS_EVENT_MASK_lpmcu_event_mask_START (3)
#define SOC_SCTRL_SCEPS_EVENT_MASK_lpmcu_event_mask_END (3)
#define SOC_SCTRL_SCEPS_EVENT_MASK_mcpu_event_mask_START (4)
#define SOC_SCTRL_SCEPS_EVENT_MASK_mcpu_event_mask_END (4)
#define SOC_SCTRL_SCEPS_EVENT_MASK_ispa7_event_mask_START (5)
#define SOC_SCTRL_SCEPS_EVENT_MASK_ispa7_event_mask_END (5)
#define SOC_SCTRL_SCEPS_EVENT_MASK_hifd_ul_event_mask_START (6)
#define SOC_SCTRL_SCEPS_EVENT_MASK_hifd_ul_event_mask_END (6)
#define SOC_SCTRL_SCEPS_EVENT_MASK_eps_event_mask_START (7)
#define SOC_SCTRL_SCEPS_EVENT_MASK_eps_event_mask_END (7)
#define SOC_SCTRL_SCEPS_EVENT_MASK_npu_archs_event_mask_START (8)
#define SOC_SCTRL_SCEPS_EVENT_MASK_npu_archs_event_mask_END (8)
#define SOC_SCTRL_SCEPS_EVENT_MASK_npu_tscpu_event_mask_START (9)
#define SOC_SCTRL_SCEPS_EVENT_MASK_npu_tscpu_event_mask_END (9)
#define SOC_SCTRL_SCEPS_EVENT_MASK_npu_aicpu_event_mask_START (10)
#define SOC_SCTRL_SCEPS_EVENT_MASK_npu_aicpu_event_mask_END (10)
#define SOC_SCTRL_SCEPS_EVENT_MASK_mdm5g_l2hac_event_mask_START (11)
#define SOC_SCTRL_SCEPS_EVENT_MASK_mdm5g_l2hac_event_mask_END (11)
#define SOC_SCTRL_SCEPS_EVENT_MASK_mdm5g_ccpu5g_event_mask_START (12)
#define SOC_SCTRL_SCEPS_EVENT_MASK_mdm5g_ccpu5g_event_mask_END (12)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int a53_event_mask : 1;
        unsigned int a57_event_mask : 1;
        unsigned int iomcu_event_mask : 1;
        unsigned int lpmcu_event_mask : 1;
        unsigned int mcpu_event_mask : 1;
        unsigned int ispa7_event_mask : 1;
        unsigned int hifd_ul_event_mask : 1;
        unsigned int eps_event_mask : 1;
        unsigned int npu_archs_event_mask : 1;
        unsigned int npu_tscpu_event_mask : 1;
        unsigned int npu_aicpu_event_mask : 1;
        unsigned int mdm5g_l2hac_event_mask : 1;
        unsigned int mdm5g_ccpu5g_event_mask : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int reserved_11 : 1;
        unsigned int reserved_12 : 1;
        unsigned int reserved_13 : 1;
        unsigned int reserved_14 : 1;
        unsigned int reserved_15 : 1;
        unsigned int reserved_16 : 1;
        unsigned int reserved_17 : 1;
        unsigned int reserved_18 : 1;
    } reg;
} SOC_SCTRL_SCMDM5G_L2HAC_EVENT_MASK_UNION;
#endif
#define SOC_SCTRL_SCMDM5G_L2HAC_EVENT_MASK_a53_event_mask_START (0)
#define SOC_SCTRL_SCMDM5G_L2HAC_EVENT_MASK_a53_event_mask_END (0)
#define SOC_SCTRL_SCMDM5G_L2HAC_EVENT_MASK_a57_event_mask_START (1)
#define SOC_SCTRL_SCMDM5G_L2HAC_EVENT_MASK_a57_event_mask_END (1)
#define SOC_SCTRL_SCMDM5G_L2HAC_EVENT_MASK_iomcu_event_mask_START (2)
#define SOC_SCTRL_SCMDM5G_L2HAC_EVENT_MASK_iomcu_event_mask_END (2)
#define SOC_SCTRL_SCMDM5G_L2HAC_EVENT_MASK_lpmcu_event_mask_START (3)
#define SOC_SCTRL_SCMDM5G_L2HAC_EVENT_MASK_lpmcu_event_mask_END (3)
#define SOC_SCTRL_SCMDM5G_L2HAC_EVENT_MASK_mcpu_event_mask_START (4)
#define SOC_SCTRL_SCMDM5G_L2HAC_EVENT_MASK_mcpu_event_mask_END (4)
#define SOC_SCTRL_SCMDM5G_L2HAC_EVENT_MASK_ispa7_event_mask_START (5)
#define SOC_SCTRL_SCMDM5G_L2HAC_EVENT_MASK_ispa7_event_mask_END (5)
#define SOC_SCTRL_SCMDM5G_L2HAC_EVENT_MASK_hifd_ul_event_mask_START (6)
#define SOC_SCTRL_SCMDM5G_L2HAC_EVENT_MASK_hifd_ul_event_mask_END (6)
#define SOC_SCTRL_SCMDM5G_L2HAC_EVENT_MASK_eps_event_mask_START (7)
#define SOC_SCTRL_SCMDM5G_L2HAC_EVENT_MASK_eps_event_mask_END (7)
#define SOC_SCTRL_SCMDM5G_L2HAC_EVENT_MASK_npu_archs_event_mask_START (8)
#define SOC_SCTRL_SCMDM5G_L2HAC_EVENT_MASK_npu_archs_event_mask_END (8)
#define SOC_SCTRL_SCMDM5G_L2HAC_EVENT_MASK_npu_tscpu_event_mask_START (9)
#define SOC_SCTRL_SCMDM5G_L2HAC_EVENT_MASK_npu_tscpu_event_mask_END (9)
#define SOC_SCTRL_SCMDM5G_L2HAC_EVENT_MASK_npu_aicpu_event_mask_START (10)
#define SOC_SCTRL_SCMDM5G_L2HAC_EVENT_MASK_npu_aicpu_event_mask_END (10)
#define SOC_SCTRL_SCMDM5G_L2HAC_EVENT_MASK_mdm5g_l2hac_event_mask_START (11)
#define SOC_SCTRL_SCMDM5G_L2HAC_EVENT_MASK_mdm5g_l2hac_event_mask_END (11)
#define SOC_SCTRL_SCMDM5G_L2HAC_EVENT_MASK_mdm5g_ccpu5g_event_mask_START (12)
#define SOC_SCTRL_SCMDM5G_L2HAC_EVENT_MASK_mdm5g_ccpu5g_event_mask_END (12)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int a53_event_mask : 1;
        unsigned int a57_event_mask : 1;
        unsigned int iomcu_event_mask : 1;
        unsigned int lpmcu_event_mask : 1;
        unsigned int mcpu_event_mask : 1;
        unsigned int ispa7_event_mask : 1;
        unsigned int hifd_ul_event_mask : 1;
        unsigned int eps_event_mask : 1;
        unsigned int npu_archs_event_mask : 1;
        unsigned int npu_tscpu_event_mask : 1;
        unsigned int npu_aicpu_event_mask : 1;
        unsigned int mdm5g_l2hac_event_mask : 1;
        unsigned int mdm5g_ccpu5g_event_mask : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int reserved_11 : 1;
        unsigned int reserved_12 : 1;
        unsigned int reserved_13 : 1;
        unsigned int reserved_14 : 1;
        unsigned int reserved_15 : 1;
        unsigned int reserved_16 : 1;
        unsigned int reserved_17 : 1;
        unsigned int reserved_18 : 1;
    } reg;
} SOC_SCTRL_SCMDM5G_CCPU5G_EVENT_MASK_UNION;
#endif
#define SOC_SCTRL_SCMDM5G_CCPU5G_EVENT_MASK_a53_event_mask_START (0)
#define SOC_SCTRL_SCMDM5G_CCPU5G_EVENT_MASK_a53_event_mask_END (0)
#define SOC_SCTRL_SCMDM5G_CCPU5G_EVENT_MASK_a57_event_mask_START (1)
#define SOC_SCTRL_SCMDM5G_CCPU5G_EVENT_MASK_a57_event_mask_END (1)
#define SOC_SCTRL_SCMDM5G_CCPU5G_EVENT_MASK_iomcu_event_mask_START (2)
#define SOC_SCTRL_SCMDM5G_CCPU5G_EVENT_MASK_iomcu_event_mask_END (2)
#define SOC_SCTRL_SCMDM5G_CCPU5G_EVENT_MASK_lpmcu_event_mask_START (3)
#define SOC_SCTRL_SCMDM5G_CCPU5G_EVENT_MASK_lpmcu_event_mask_END (3)
#define SOC_SCTRL_SCMDM5G_CCPU5G_EVENT_MASK_mcpu_event_mask_START (4)
#define SOC_SCTRL_SCMDM5G_CCPU5G_EVENT_MASK_mcpu_event_mask_END (4)
#define SOC_SCTRL_SCMDM5G_CCPU5G_EVENT_MASK_ispa7_event_mask_START (5)
#define SOC_SCTRL_SCMDM5G_CCPU5G_EVENT_MASK_ispa7_event_mask_END (5)
#define SOC_SCTRL_SCMDM5G_CCPU5G_EVENT_MASK_hifd_ul_event_mask_START (6)
#define SOC_SCTRL_SCMDM5G_CCPU5G_EVENT_MASK_hifd_ul_event_mask_END (6)
#define SOC_SCTRL_SCMDM5G_CCPU5G_EVENT_MASK_eps_event_mask_START (7)
#define SOC_SCTRL_SCMDM5G_CCPU5G_EVENT_MASK_eps_event_mask_END (7)
#define SOC_SCTRL_SCMDM5G_CCPU5G_EVENT_MASK_npu_archs_event_mask_START (8)
#define SOC_SCTRL_SCMDM5G_CCPU5G_EVENT_MASK_npu_archs_event_mask_END (8)
#define SOC_SCTRL_SCMDM5G_CCPU5G_EVENT_MASK_npu_tscpu_event_mask_START (9)
#define SOC_SCTRL_SCMDM5G_CCPU5G_EVENT_MASK_npu_tscpu_event_mask_END (9)
#define SOC_SCTRL_SCMDM5G_CCPU5G_EVENT_MASK_npu_aicpu_event_mask_START (10)
#define SOC_SCTRL_SCMDM5G_CCPU5G_EVENT_MASK_npu_aicpu_event_mask_END (10)
#define SOC_SCTRL_SCMDM5G_CCPU5G_EVENT_MASK_mdm5g_l2hac_event_mask_START (11)
#define SOC_SCTRL_SCMDM5G_CCPU5G_EVENT_MASK_mdm5g_l2hac_event_mask_END (11)
#define SOC_SCTRL_SCMDM5G_CCPU5G_EVENT_MASK_mdm5g_ccpu5g_event_mask_START (12)
#define SOC_SCTRL_SCMDM5G_CCPU5G_EVENT_MASK_mdm5g_ccpu5g_event_mask_END (12)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int a53_event : 1;
        unsigned int a57_event : 1;
        unsigned int iomcu_event : 1;
        unsigned int lpmcu_event : 1;
        unsigned int mcpu_event : 1;
        unsigned int ispa7_event : 1;
        unsigned int hifd_ul_event : 1;
        unsigned int eps_event : 1;
        unsigned int npu_archs_event : 1;
        unsigned int npu_tscpu_event : 1;
        unsigned int npu_aicpu_event : 1;
        unsigned int mdm5g_l2hac_event : 1;
        unsigned int mdm5g_ccpu5g_event : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int reserved_11 : 1;
        unsigned int reserved_12 : 1;
        unsigned int reserved_13 : 1;
        unsigned int reserved_14 : 1;
        unsigned int reserved_15 : 1;
        unsigned int reserved_16 : 1;
        unsigned int reserved_17 : 1;
        unsigned int reserved_18 : 1;
    } reg;
} SOC_SCTRL_SCEVENT_STAT_UNION;
#endif
#define SOC_SCTRL_SCEVENT_STAT_a53_event_START (0)
#define SOC_SCTRL_SCEVENT_STAT_a53_event_END (0)
#define SOC_SCTRL_SCEVENT_STAT_a57_event_START (1)
#define SOC_SCTRL_SCEVENT_STAT_a57_event_END (1)
#define SOC_SCTRL_SCEVENT_STAT_iomcu_event_START (2)
#define SOC_SCTRL_SCEVENT_STAT_iomcu_event_END (2)
#define SOC_SCTRL_SCEVENT_STAT_lpmcu_event_START (3)
#define SOC_SCTRL_SCEVENT_STAT_lpmcu_event_END (3)
#define SOC_SCTRL_SCEVENT_STAT_mcpu_event_START (4)
#define SOC_SCTRL_SCEVENT_STAT_mcpu_event_END (4)
#define SOC_SCTRL_SCEVENT_STAT_ispa7_event_START (5)
#define SOC_SCTRL_SCEVENT_STAT_ispa7_event_END (5)
#define SOC_SCTRL_SCEVENT_STAT_hifd_ul_event_START (6)
#define SOC_SCTRL_SCEVENT_STAT_hifd_ul_event_END (6)
#define SOC_SCTRL_SCEVENT_STAT_eps_event_START (7)
#define SOC_SCTRL_SCEVENT_STAT_eps_event_END (7)
#define SOC_SCTRL_SCEVENT_STAT_npu_archs_event_START (8)
#define SOC_SCTRL_SCEVENT_STAT_npu_archs_event_END (8)
#define SOC_SCTRL_SCEVENT_STAT_npu_tscpu_event_START (9)
#define SOC_SCTRL_SCEVENT_STAT_npu_tscpu_event_END (9)
#define SOC_SCTRL_SCEVENT_STAT_npu_aicpu_event_START (10)
#define SOC_SCTRL_SCEVENT_STAT_npu_aicpu_event_END (10)
#define SOC_SCTRL_SCEVENT_STAT_mdm5g_l2hac_event_START (11)
#define SOC_SCTRL_SCEVENT_STAT_mdm5g_l2hac_event_END (11)
#define SOC_SCTRL_SCEVENT_STAT_mdm5g_ccpu5g_event_START (12)
#define SOC_SCTRL_SCEVENT_STAT_mdm5g_ccpu5g_event_END (12)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int gt_iomcu_pll_div : 1;
        unsigned int gt_iomcu_sys_div : 1;
        unsigned int gt_clk_qcr_iomcubus : 1;
        unsigned int reserved : 13;
        unsigned int sciomcuclkctrl_msk : 16;
    } reg;
} SOC_SCTRL_SCIOMCUCLKCTRL_UNION;
#endif
#define SOC_SCTRL_SCIOMCUCLKCTRL_gt_iomcu_pll_div_START (0)
#define SOC_SCTRL_SCIOMCUCLKCTRL_gt_iomcu_pll_div_END (0)
#define SOC_SCTRL_SCIOMCUCLKCTRL_gt_iomcu_sys_div_START (1)
#define SOC_SCTRL_SCIOMCUCLKCTRL_gt_iomcu_sys_div_END (1)
#define SOC_SCTRL_SCIOMCUCLKCTRL_gt_clk_qcr_iomcubus_START (2)
#define SOC_SCTRL_SCIOMCUCLKCTRL_gt_clk_qcr_iomcubus_END (2)
#define SOC_SCTRL_SCIOMCUCLKCTRL_sciomcuclkctrl_msk_START (16)
#define SOC_SCTRL_SCIOMCUCLKCTRL_sciomcuclkctrl_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int iomcu_pll_ctrl : 1;
        unsigned int iomcu_pll_div_ctrl : 1;
        unsigned int iomcu_sys_div_ctrl : 1;
        unsigned int gt_iomcu_pll_div_stat : 1;
        unsigned int gt_iomcu_sys_div_stat : 1;
        unsigned int reserved : 27;
    } reg;
} SOC_SCTRL_SCIOMCUCLKSTAT_UNION;
#endif
#define SOC_SCTRL_SCIOMCUCLKSTAT_iomcu_pll_ctrl_START (0)
#define SOC_SCTRL_SCIOMCUCLKSTAT_iomcu_pll_ctrl_END (0)
#define SOC_SCTRL_SCIOMCUCLKSTAT_iomcu_pll_div_ctrl_START (1)
#define SOC_SCTRL_SCIOMCUCLKSTAT_iomcu_pll_div_ctrl_END (1)
#define SOC_SCTRL_SCIOMCUCLKSTAT_iomcu_sys_div_ctrl_START (2)
#define SOC_SCTRL_SCIOMCUCLKSTAT_iomcu_sys_div_ctrl_END (2)
#define SOC_SCTRL_SCIOMCUCLKSTAT_gt_iomcu_pll_div_stat_START (3)
#define SOC_SCTRL_SCIOMCUCLKSTAT_gt_iomcu_pll_div_stat_END (3)
#define SOC_SCTRL_SCIOMCUCLKSTAT_gt_iomcu_sys_div_stat_START (4)
#define SOC_SCTRL_SCIOMCUCLKSTAT_gt_iomcu_sys_div_stat_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0: 1;
        unsigned int nmi_in : 1;
        unsigned int reserved_1: 1;
        unsigned int reserved_2: 29;
    } reg;
} SOC_SCTRL_SCIOMCUCTRL_UNION;
#endif
#define SOC_SCTRL_SCIOMCUCTRL_nmi_in_START (1)
#define SOC_SCTRL_SCIOMCUCTRL_nmi_in_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int iomcu_sysresetreq : 1;
        unsigned int iomcu_m7_state : 3;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int m7_sc_stat : 9;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int iomcu_stat : 10;
    } reg;
} SOC_SCTRL_SCIOMCUSTAT_UNION;
#endif
#define SOC_SCTRL_SCIOMCUSTAT_iomcu_sysresetreq_START (0)
#define SOC_SCTRL_SCIOMCUSTAT_iomcu_sysresetreq_END (0)
#define SOC_SCTRL_SCIOMCUSTAT_iomcu_m7_state_START (1)
#define SOC_SCTRL_SCIOMCUSTAT_iomcu_m7_state_END (3)
#define SOC_SCTRL_SCIOMCUSTAT_m7_sc_stat_START (7)
#define SOC_SCTRL_SCIOMCUSTAT_m7_sc_stat_END (15)
#define SOC_SCTRL_SCIOMCUSTAT_iomcu_stat_START (22)
#define SOC_SCTRL_SCIOMCUSTAT_iomcu_stat_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int resource0lock_cmd0 : 1;
        unsigned int resource0lock_id0 : 3;
        unsigned int resource0lock_cmd1 : 1;
        unsigned int resource0lock_id1 : 3;
        unsigned int resource0lock_cmd2 : 1;
        unsigned int resource0lock_id2 : 3;
        unsigned int resource0lock_cmd3 : 1;
        unsigned int resource0lock_id3 : 3;
        unsigned int resource0lock_cmd4 : 1;
        unsigned int resource0lock_id4 : 3;
        unsigned int resource0lock_cmd5 : 1;
        unsigned int resource0lock_id5 : 3;
        unsigned int resource0lock_cmd6 : 1;
        unsigned int resource0lock_id6 : 3;
        unsigned int resource0lock_cmd7 : 1;
        unsigned int resource0lock_id7 : 3;
    } reg;
} SOC_SCTRL_SC_LOCK_CFG_UNSEC_0_UNION;
#endif
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_0_resource0lock_cmd0_START (0)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_0_resource0lock_cmd0_END (0)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_0_resource0lock_id0_START (1)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_0_resource0lock_id0_END (3)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_0_resource0lock_cmd1_START (4)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_0_resource0lock_cmd1_END (4)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_0_resource0lock_id1_START (5)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_0_resource0lock_id1_END (7)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_0_resource0lock_cmd2_START (8)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_0_resource0lock_cmd2_END (8)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_0_resource0lock_id2_START (9)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_0_resource0lock_id2_END (11)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_0_resource0lock_cmd3_START (12)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_0_resource0lock_cmd3_END (12)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_0_resource0lock_id3_START (13)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_0_resource0lock_id3_END (15)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_0_resource0lock_cmd4_START (16)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_0_resource0lock_cmd4_END (16)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_0_resource0lock_id4_START (17)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_0_resource0lock_id4_END (19)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_0_resource0lock_cmd5_START (20)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_0_resource0lock_cmd5_END (20)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_0_resource0lock_id5_START (21)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_0_resource0lock_id5_END (23)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_0_resource0lock_cmd6_START (24)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_0_resource0lock_cmd6_END (24)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_0_resource0lock_id6_START (25)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_0_resource0lock_id6_END (27)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_0_resource0lock_cmd7_START (28)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_0_resource0lock_cmd7_END (28)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_0_resource0lock_id7_START (29)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_0_resource0lock_id7_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int resource0unlock_cmd0 : 1;
        unsigned int resource0unlock_id0 : 3;
        unsigned int resource0unlock_cmd1 : 1;
        unsigned int resource0unlock_id1 : 3;
        unsigned int resource0unlock_cmd2 : 1;
        unsigned int resource0unlock_id2 : 3;
        unsigned int resource0unlock_cmd3 : 1;
        unsigned int resource0unlock_id3 : 3;
        unsigned int resource0unlock_cmd4 : 1;
        unsigned int resource0unlock_id4 : 3;
        unsigned int resource0unlock_cmd5 : 1;
        unsigned int resource0unlock_id5 : 3;
        unsigned int resource0unlock_cmd6 : 1;
        unsigned int resource0unlock_id6 : 3;
        unsigned int resource0unlock_cmd7 : 1;
        unsigned int resource0unlock_id7 : 3;
    } reg;
} SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_0_UNION;
#endif
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_0_resource0unlock_cmd0_START (0)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_0_resource0unlock_cmd0_END (0)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_0_resource0unlock_id0_START (1)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_0_resource0unlock_id0_END (3)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_0_resource0unlock_cmd1_START (4)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_0_resource0unlock_cmd1_END (4)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_0_resource0unlock_id1_START (5)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_0_resource0unlock_id1_END (7)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_0_resource0unlock_cmd2_START (8)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_0_resource0unlock_cmd2_END (8)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_0_resource0unlock_id2_START (9)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_0_resource0unlock_id2_END (11)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_0_resource0unlock_cmd3_START (12)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_0_resource0unlock_cmd3_END (12)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_0_resource0unlock_id3_START (13)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_0_resource0unlock_id3_END (15)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_0_resource0unlock_cmd4_START (16)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_0_resource0unlock_cmd4_END (16)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_0_resource0unlock_id4_START (17)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_0_resource0unlock_id4_END (19)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_0_resource0unlock_cmd5_START (20)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_0_resource0unlock_cmd5_END (20)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_0_resource0unlock_id5_START (21)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_0_resource0unlock_id5_END (23)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_0_resource0unlock_cmd6_START (24)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_0_resource0unlock_cmd6_END (24)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_0_resource0unlock_id6_START (25)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_0_resource0unlock_id6_END (27)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_0_resource0unlock_cmd7_START (28)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_0_resource0unlock_cmd7_END (28)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_0_resource0unlock_id7_START (29)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_0_resource0unlock_id7_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int resource0lock_st0 : 1;
        unsigned int resource0lock_st_id0 : 3;
        unsigned int resource0lock_st1 : 1;
        unsigned int resource0lock_st_id1 : 3;
        unsigned int resource0lock_st2 : 1;
        unsigned int resource0lock_st_id2 : 3;
        unsigned int resource0lock_st3 : 1;
        unsigned int resource0lock_st_id3 : 3;
        unsigned int resource0lock_st4 : 1;
        unsigned int resource0lock_st_id4 : 3;
        unsigned int resource0lock_st5 : 1;
        unsigned int resource0lock_st_id5 : 3;
        unsigned int resource0lock_st6 : 1;
        unsigned int resource0lock_st_id6 : 3;
        unsigned int resource0lock_st7 : 1;
        unsigned int resource0lock_st_id7 : 3;
    } reg;
} SOC_SCTRL_SC_LOCK_STAT_UNSEC_0_UNION;
#endif
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_0_resource0lock_st0_START (0)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_0_resource0lock_st0_END (0)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_0_resource0lock_st_id0_START (1)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_0_resource0lock_st_id0_END (3)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_0_resource0lock_st1_START (4)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_0_resource0lock_st1_END (4)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_0_resource0lock_st_id1_START (5)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_0_resource0lock_st_id1_END (7)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_0_resource0lock_st2_START (8)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_0_resource0lock_st2_END (8)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_0_resource0lock_st_id2_START (9)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_0_resource0lock_st_id2_END (11)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_0_resource0lock_st3_START (12)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_0_resource0lock_st3_END (12)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_0_resource0lock_st_id3_START (13)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_0_resource0lock_st_id3_END (15)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_0_resource0lock_st4_START (16)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_0_resource0lock_st4_END (16)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_0_resource0lock_st_id4_START (17)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_0_resource0lock_st_id4_END (19)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_0_resource0lock_st5_START (20)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_0_resource0lock_st5_END (20)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_0_resource0lock_st_id5_START (21)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_0_resource0lock_st_id5_END (23)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_0_resource0lock_st6_START (24)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_0_resource0lock_st6_END (24)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_0_resource0lock_st_id6_START (25)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_0_resource0lock_st_id6_END (27)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_0_resource0lock_st7_START (28)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_0_resource0lock_st7_END (28)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_0_resource0lock_st_id7_START (29)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_0_resource0lock_st_id7_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int resource0lock_cmd0 : 1;
        unsigned int resource0lock_id0 : 3;
        unsigned int resource0lock_cmd1 : 1;
        unsigned int resource0lock_id1 : 3;
        unsigned int resource0lock_cmd2 : 1;
        unsigned int resource0lock_id2 : 3;
        unsigned int resource0lock_cmd3 : 1;
        unsigned int resource0lock_id3 : 3;
        unsigned int resource0lock_cmd4 : 1;
        unsigned int resource0lock_id4 : 3;
        unsigned int resource0lock_cmd5 : 1;
        unsigned int resource0lock_id5 : 3;
        unsigned int resource0lock_cmd6 : 1;
        unsigned int resource0lock_id6 : 3;
        unsigned int resource0lock_cmd7 : 1;
        unsigned int resource0lock_id7 : 3;
    } reg;
} SOC_SCTRL_SC_LOCK_CFG_UNSEC_1_UNION;
#endif
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_1_resource0lock_cmd0_START (0)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_1_resource0lock_cmd0_END (0)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_1_resource0lock_id0_START (1)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_1_resource0lock_id0_END (3)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_1_resource0lock_cmd1_START (4)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_1_resource0lock_cmd1_END (4)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_1_resource0lock_id1_START (5)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_1_resource0lock_id1_END (7)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_1_resource0lock_cmd2_START (8)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_1_resource0lock_cmd2_END (8)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_1_resource0lock_id2_START (9)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_1_resource0lock_id2_END (11)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_1_resource0lock_cmd3_START (12)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_1_resource0lock_cmd3_END (12)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_1_resource0lock_id3_START (13)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_1_resource0lock_id3_END (15)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_1_resource0lock_cmd4_START (16)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_1_resource0lock_cmd4_END (16)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_1_resource0lock_id4_START (17)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_1_resource0lock_id4_END (19)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_1_resource0lock_cmd5_START (20)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_1_resource0lock_cmd5_END (20)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_1_resource0lock_id5_START (21)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_1_resource0lock_id5_END (23)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_1_resource0lock_cmd6_START (24)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_1_resource0lock_cmd6_END (24)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_1_resource0lock_id6_START (25)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_1_resource0lock_id6_END (27)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_1_resource0lock_cmd7_START (28)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_1_resource0lock_cmd7_END (28)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_1_resource0lock_id7_START (29)
#define SOC_SCTRL_SC_LOCK_CFG_UNSEC_1_resource0lock_id7_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int resource0unlock_cmd0 : 1;
        unsigned int resource0unlock_id0 : 3;
        unsigned int resource0unlock_cmd1 : 1;
        unsigned int resource0unlock_id1 : 3;
        unsigned int resource0unlock_cmd2 : 1;
        unsigned int resource0unlock_id2 : 3;
        unsigned int resource0unlock_cmd3 : 1;
        unsigned int resource0unlock_id3 : 3;
        unsigned int resource0unlock_cmd4 : 1;
        unsigned int resource0unlock_id4 : 3;
        unsigned int resource0unlock_cmd5 : 1;
        unsigned int resource0unlock_id5 : 3;
        unsigned int resource0unlock_cmd6 : 1;
        unsigned int resource0unlock_id6 : 3;
        unsigned int resource0unlock_cmd7 : 1;
        unsigned int resource0unlock_id7 : 3;
    } reg;
} SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_1_UNION;
#endif
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_1_resource0unlock_cmd0_START (0)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_1_resource0unlock_cmd0_END (0)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_1_resource0unlock_id0_START (1)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_1_resource0unlock_id0_END (3)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_1_resource0unlock_cmd1_START (4)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_1_resource0unlock_cmd1_END (4)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_1_resource0unlock_id1_START (5)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_1_resource0unlock_id1_END (7)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_1_resource0unlock_cmd2_START (8)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_1_resource0unlock_cmd2_END (8)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_1_resource0unlock_id2_START (9)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_1_resource0unlock_id2_END (11)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_1_resource0unlock_cmd3_START (12)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_1_resource0unlock_cmd3_END (12)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_1_resource0unlock_id3_START (13)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_1_resource0unlock_id3_END (15)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_1_resource0unlock_cmd4_START (16)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_1_resource0unlock_cmd4_END (16)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_1_resource0unlock_id4_START (17)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_1_resource0unlock_id4_END (19)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_1_resource0unlock_cmd5_START (20)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_1_resource0unlock_cmd5_END (20)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_1_resource0unlock_id5_START (21)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_1_resource0unlock_id5_END (23)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_1_resource0unlock_cmd6_START (24)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_1_resource0unlock_cmd6_END (24)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_1_resource0unlock_id6_START (25)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_1_resource0unlock_id6_END (27)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_1_resource0unlock_cmd7_START (28)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_1_resource0unlock_cmd7_END (28)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_1_resource0unlock_id7_START (29)
#define SOC_SCTRL_SC_UNLOCK_CFG_UNSEC_1_resource0unlock_id7_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int resource0lock_st0 : 1;
        unsigned int resource0lock_st_id0 : 3;
        unsigned int resource0lock_st1 : 1;
        unsigned int resource0lock_st_id1 : 3;
        unsigned int resource0lock_st2 : 1;
        unsigned int resource0lock_st_id2 : 3;
        unsigned int resource0lock_st3 : 1;
        unsigned int resource0lock_st_id3 : 3;
        unsigned int resource0lock_st4 : 1;
        unsigned int resource0lock_st_id4 : 3;
        unsigned int resource0lock_st5 : 1;
        unsigned int resource0lock_st_id5 : 3;
        unsigned int resource0lock_st6 : 1;
        unsigned int resource0lock_st_id6 : 3;
        unsigned int resource0lock_st7 : 1;
        unsigned int resource0lock_st_id7 : 3;
    } reg;
} SOC_SCTRL_SC_LOCK_STAT_UNSEC_1_UNION;
#endif
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_1_resource0lock_st0_START (0)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_1_resource0lock_st0_END (0)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_1_resource0lock_st_id0_START (1)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_1_resource0lock_st_id0_END (3)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_1_resource0lock_st1_START (4)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_1_resource0lock_st1_END (4)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_1_resource0lock_st_id1_START (5)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_1_resource0lock_st_id1_END (7)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_1_resource0lock_st2_START (8)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_1_resource0lock_st2_END (8)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_1_resource0lock_st_id2_START (9)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_1_resource0lock_st_id2_END (11)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_1_resource0lock_st3_START (12)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_1_resource0lock_st3_END (12)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_1_resource0lock_st_id3_START (13)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_1_resource0lock_st_id3_END (15)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_1_resource0lock_st4_START (16)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_1_resource0lock_st4_END (16)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_1_resource0lock_st_id4_START (17)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_1_resource0lock_st_id4_END (19)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_1_resource0lock_st5_START (20)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_1_resource0lock_st5_END (20)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_1_resource0lock_st_id5_START (21)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_1_resource0lock_st_id5_END (23)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_1_resource0lock_st6_START (24)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_1_resource0lock_st6_END (24)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_1_resource0lock_st_id6_START (25)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_1_resource0lock_st_id6_END (27)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_1_resource0lock_st7_START (28)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_1_resource0lock_st7_END (28)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_1_resource0lock_st_id7_START (29)
#define SOC_SCTRL_SC_LOCK_STAT_UNSEC_1_resource0lock_st_id7_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0: 31;
        unsigned int reserved_1: 1;
    } reg;
} SOC_SCTRL_SCMDMADDR0_NONSEC_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0: 31;
        unsigned int reserved_1: 1;
    } reg;
} SOC_SCTRL_SCMDMADDR1_NONSEC_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0: 31;
        unsigned int reserved_1: 1;
    } reg;
} SOC_SCTRL_SCMDMADDR2_NONSEC_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0: 31;
        unsigned int reserved_1: 1;
    } reg;
} SOC_SCTRL_SCMDMADDR3_NONSEC_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int resource0lock_cmd0 : 1;
        unsigned int resource0lock_id0 : 3;
        unsigned int resource0lock_cmd1 : 1;
        unsigned int resource0lock_id1 : 3;
        unsigned int resource0lock_cmd2 : 1;
        unsigned int resource0lock_id2 : 3;
        unsigned int resource0lock_cmd3 : 1;
        unsigned int resource0lock_id3 : 3;
        unsigned int resource0lock_cmd4 : 1;
        unsigned int resource0lock_id4 : 3;
        unsigned int resource0lock_cmd5 : 1;
        unsigned int resource0lock_id5 : 3;
        unsigned int resource0lock_cmd6 : 1;
        unsigned int resource0lock_id6 : 3;
        unsigned int resource0lock_cmd7 : 1;
        unsigned int resource0lock_id7 : 3;
    } reg;
} SOC_SCTRL_SC_LOCK_CFG_SEC_0_UNION;
#endif
#define SOC_SCTRL_SC_LOCK_CFG_SEC_0_resource0lock_cmd0_START (0)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_0_resource0lock_cmd0_END (0)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_0_resource0lock_id0_START (1)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_0_resource0lock_id0_END (3)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_0_resource0lock_cmd1_START (4)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_0_resource0lock_cmd1_END (4)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_0_resource0lock_id1_START (5)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_0_resource0lock_id1_END (7)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_0_resource0lock_cmd2_START (8)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_0_resource0lock_cmd2_END (8)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_0_resource0lock_id2_START (9)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_0_resource0lock_id2_END (11)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_0_resource0lock_cmd3_START (12)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_0_resource0lock_cmd3_END (12)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_0_resource0lock_id3_START (13)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_0_resource0lock_id3_END (15)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_0_resource0lock_cmd4_START (16)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_0_resource0lock_cmd4_END (16)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_0_resource0lock_id4_START (17)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_0_resource0lock_id4_END (19)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_0_resource0lock_cmd5_START (20)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_0_resource0lock_cmd5_END (20)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_0_resource0lock_id5_START (21)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_0_resource0lock_id5_END (23)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_0_resource0lock_cmd6_START (24)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_0_resource0lock_cmd6_END (24)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_0_resource0lock_id6_START (25)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_0_resource0lock_id6_END (27)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_0_resource0lock_cmd7_START (28)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_0_resource0lock_cmd7_END (28)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_0_resource0lock_id7_START (29)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_0_resource0lock_id7_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int resource0unlock_cmd0 : 1;
        unsigned int resource0unlock_id0 : 3;
        unsigned int resource0unlock_cmd1 : 1;
        unsigned int resource0unlock_id1 : 3;
        unsigned int resource0unlock_cmd2 : 1;
        unsigned int resource0unlock_id2 : 3;
        unsigned int resource0unlock_cmd3 : 1;
        unsigned int resource0unlock_id3 : 3;
        unsigned int resource0unlock_cmd4 : 1;
        unsigned int resource0unlock_id4 : 3;
        unsigned int resource0unlock_cmd5 : 1;
        unsigned int resource0unlock_id5 : 3;
        unsigned int resource0unlock_cmd6 : 1;
        unsigned int resource0unlock_id6 : 3;
        unsigned int resource0unlock_cmd7 : 1;
        unsigned int resource0unlock_id7 : 3;
    } reg;
} SOC_SCTRL_SC_UNLOCK_CFG_SEC_0_UNION;
#endif
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_0_resource0unlock_cmd0_START (0)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_0_resource0unlock_cmd0_END (0)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_0_resource0unlock_id0_START (1)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_0_resource0unlock_id0_END (3)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_0_resource0unlock_cmd1_START (4)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_0_resource0unlock_cmd1_END (4)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_0_resource0unlock_id1_START (5)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_0_resource0unlock_id1_END (7)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_0_resource0unlock_cmd2_START (8)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_0_resource0unlock_cmd2_END (8)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_0_resource0unlock_id2_START (9)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_0_resource0unlock_id2_END (11)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_0_resource0unlock_cmd3_START (12)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_0_resource0unlock_cmd3_END (12)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_0_resource0unlock_id3_START (13)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_0_resource0unlock_id3_END (15)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_0_resource0unlock_cmd4_START (16)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_0_resource0unlock_cmd4_END (16)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_0_resource0unlock_id4_START (17)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_0_resource0unlock_id4_END (19)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_0_resource0unlock_cmd5_START (20)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_0_resource0unlock_cmd5_END (20)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_0_resource0unlock_id5_START (21)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_0_resource0unlock_id5_END (23)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_0_resource0unlock_cmd6_START (24)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_0_resource0unlock_cmd6_END (24)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_0_resource0unlock_id6_START (25)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_0_resource0unlock_id6_END (27)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_0_resource0unlock_cmd7_START (28)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_0_resource0unlock_cmd7_END (28)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_0_resource0unlock_id7_START (29)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_0_resource0unlock_id7_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int resource0lock_st0 : 1;
        unsigned int resource0lock_st_id0 : 3;
        unsigned int resource0lock_st1 : 1;
        unsigned int resource0lock_st_id1 : 3;
        unsigned int resource0lock_st2 : 1;
        unsigned int resource0lock_st_id2 : 3;
        unsigned int resource0lock_st3 : 1;
        unsigned int resource0lock_st_id3 : 3;
        unsigned int resource0lock_st4 : 1;
        unsigned int resource0lock_st_id4 : 3;
        unsigned int resource0lock_st5 : 1;
        unsigned int resource0lock_st_id5 : 3;
        unsigned int resource0lock_st6 : 1;
        unsigned int resource0lock_st_id6 : 3;
        unsigned int resource0lock_st7 : 1;
        unsigned int resource0lock_st_id7 : 3;
    } reg;
} SOC_SCTRL_SC_LOCK_STAT_SEC_0_UNION;
#endif
#define SOC_SCTRL_SC_LOCK_STAT_SEC_0_resource0lock_st0_START (0)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_0_resource0lock_st0_END (0)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_0_resource0lock_st_id0_START (1)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_0_resource0lock_st_id0_END (3)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_0_resource0lock_st1_START (4)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_0_resource0lock_st1_END (4)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_0_resource0lock_st_id1_START (5)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_0_resource0lock_st_id1_END (7)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_0_resource0lock_st2_START (8)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_0_resource0lock_st2_END (8)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_0_resource0lock_st_id2_START (9)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_0_resource0lock_st_id2_END (11)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_0_resource0lock_st3_START (12)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_0_resource0lock_st3_END (12)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_0_resource0lock_st_id3_START (13)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_0_resource0lock_st_id3_END (15)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_0_resource0lock_st4_START (16)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_0_resource0lock_st4_END (16)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_0_resource0lock_st_id4_START (17)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_0_resource0lock_st_id4_END (19)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_0_resource0lock_st5_START (20)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_0_resource0lock_st5_END (20)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_0_resource0lock_st_id5_START (21)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_0_resource0lock_st_id5_END (23)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_0_resource0lock_st6_START (24)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_0_resource0lock_st6_END (24)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_0_resource0lock_st_id6_START (25)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_0_resource0lock_st_id6_END (27)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_0_resource0lock_st7_START (28)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_0_resource0lock_st7_END (28)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_0_resource0lock_st_id7_START (29)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_0_resource0lock_st_id7_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int resource0lock_cmd0 : 1;
        unsigned int resource0lock_id0 : 3;
        unsigned int resource0lock_cmd1 : 1;
        unsigned int resource0lock_id1 : 3;
        unsigned int resource0lock_cmd2 : 1;
        unsigned int resource0lock_id2 : 3;
        unsigned int resource0lock_cmd3 : 1;
        unsigned int resource0lock_id3 : 3;
        unsigned int resource0lock_cmd4 : 1;
        unsigned int resource0lock_id4 : 3;
        unsigned int resource0lock_cmd5 : 1;
        unsigned int resource0lock_id5 : 3;
        unsigned int resource0lock_cmd6 : 1;
        unsigned int resource0lock_id6 : 3;
        unsigned int resource0lock_cmd7 : 1;
        unsigned int resource0lock_id7 : 3;
    } reg;
} SOC_SCTRL_SC_LOCK_CFG_SEC_1_UNION;
#endif
#define SOC_SCTRL_SC_LOCK_CFG_SEC_1_resource0lock_cmd0_START (0)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_1_resource0lock_cmd0_END (0)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_1_resource0lock_id0_START (1)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_1_resource0lock_id0_END (3)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_1_resource0lock_cmd1_START (4)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_1_resource0lock_cmd1_END (4)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_1_resource0lock_id1_START (5)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_1_resource0lock_id1_END (7)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_1_resource0lock_cmd2_START (8)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_1_resource0lock_cmd2_END (8)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_1_resource0lock_id2_START (9)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_1_resource0lock_id2_END (11)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_1_resource0lock_cmd3_START (12)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_1_resource0lock_cmd3_END (12)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_1_resource0lock_id3_START (13)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_1_resource0lock_id3_END (15)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_1_resource0lock_cmd4_START (16)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_1_resource0lock_cmd4_END (16)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_1_resource0lock_id4_START (17)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_1_resource0lock_id4_END (19)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_1_resource0lock_cmd5_START (20)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_1_resource0lock_cmd5_END (20)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_1_resource0lock_id5_START (21)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_1_resource0lock_id5_END (23)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_1_resource0lock_cmd6_START (24)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_1_resource0lock_cmd6_END (24)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_1_resource0lock_id6_START (25)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_1_resource0lock_id6_END (27)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_1_resource0lock_cmd7_START (28)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_1_resource0lock_cmd7_END (28)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_1_resource0lock_id7_START (29)
#define SOC_SCTRL_SC_LOCK_CFG_SEC_1_resource0lock_id7_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int resource0unlock_cmd0 : 1;
        unsigned int resource0unlock_id0 : 3;
        unsigned int resource0unlock_cmd1 : 1;
        unsigned int resource0unlock_id1 : 3;
        unsigned int resource0unlock_cmd2 : 1;
        unsigned int resource0unlock_id2 : 3;
        unsigned int resource0unlock_cmd3 : 1;
        unsigned int resource0unlock_id3 : 3;
        unsigned int resource0unlock_cmd4 : 1;
        unsigned int resource0unlock_id4 : 3;
        unsigned int resource0unlock_cmd5 : 1;
        unsigned int resource0unlock_id5 : 3;
        unsigned int resource0unlock_cmd6 : 1;
        unsigned int resource0unlock_id6 : 3;
        unsigned int resource0unlock_cmd7 : 1;
        unsigned int resource0unlock_id7 : 3;
    } reg;
} SOC_SCTRL_SC_UNLOCK_CFG_SEC_1_UNION;
#endif
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_1_resource0unlock_cmd0_START (0)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_1_resource0unlock_cmd0_END (0)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_1_resource0unlock_id0_START (1)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_1_resource0unlock_id0_END (3)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_1_resource0unlock_cmd1_START (4)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_1_resource0unlock_cmd1_END (4)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_1_resource0unlock_id1_START (5)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_1_resource0unlock_id1_END (7)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_1_resource0unlock_cmd2_START (8)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_1_resource0unlock_cmd2_END (8)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_1_resource0unlock_id2_START (9)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_1_resource0unlock_id2_END (11)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_1_resource0unlock_cmd3_START (12)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_1_resource0unlock_cmd3_END (12)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_1_resource0unlock_id3_START (13)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_1_resource0unlock_id3_END (15)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_1_resource0unlock_cmd4_START (16)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_1_resource0unlock_cmd4_END (16)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_1_resource0unlock_id4_START (17)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_1_resource0unlock_id4_END (19)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_1_resource0unlock_cmd5_START (20)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_1_resource0unlock_cmd5_END (20)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_1_resource0unlock_id5_START (21)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_1_resource0unlock_id5_END (23)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_1_resource0unlock_cmd6_START (24)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_1_resource0unlock_cmd6_END (24)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_1_resource0unlock_id6_START (25)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_1_resource0unlock_id6_END (27)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_1_resource0unlock_cmd7_START (28)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_1_resource0unlock_cmd7_END (28)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_1_resource0unlock_id7_START (29)
#define SOC_SCTRL_SC_UNLOCK_CFG_SEC_1_resource0unlock_id7_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int resource0lock_st0 : 1;
        unsigned int resource0lock_st_id0 : 3;
        unsigned int resource0lock_st1 : 1;
        unsigned int resource0lock_st_id1 : 3;
        unsigned int resource0lock_st2 : 1;
        unsigned int resource0lock_st_id2 : 3;
        unsigned int resource0lock_st3 : 1;
        unsigned int resource0lock_st_id3 : 3;
        unsigned int resource0lock_st4 : 1;
        unsigned int resource0lock_st_id4 : 3;
        unsigned int resource0lock_st5 : 1;
        unsigned int resource0lock_st_id5 : 3;
        unsigned int resource0lock_st6 : 1;
        unsigned int resource0lock_st_id6 : 3;
        unsigned int resource0lock_st7 : 1;
        unsigned int resource0lock_st_id7 : 3;
    } reg;
} SOC_SCTRL_SC_LOCK_STAT_SEC_1_UNION;
#endif
#define SOC_SCTRL_SC_LOCK_STAT_SEC_1_resource0lock_st0_START (0)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_1_resource0lock_st0_END (0)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_1_resource0lock_st_id0_START (1)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_1_resource0lock_st_id0_END (3)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_1_resource0lock_st1_START (4)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_1_resource0lock_st1_END (4)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_1_resource0lock_st_id1_START (5)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_1_resource0lock_st_id1_END (7)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_1_resource0lock_st2_START (8)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_1_resource0lock_st2_END (8)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_1_resource0lock_st_id2_START (9)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_1_resource0lock_st_id2_END (11)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_1_resource0lock_st3_START (12)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_1_resource0lock_st3_END (12)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_1_resource0lock_st_id3_START (13)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_1_resource0lock_st_id3_END (15)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_1_resource0lock_st4_START (16)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_1_resource0lock_st4_END (16)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_1_resource0lock_st_id4_START (17)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_1_resource0lock_st_id4_END (19)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_1_resource0lock_st5_START (20)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_1_resource0lock_st5_END (20)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_1_resource0lock_st_id5_START (21)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_1_resource0lock_st_id5_END (23)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_1_resource0lock_st6_START (24)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_1_resource0lock_st6_END (24)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_1_resource0lock_st_id6_START (25)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_1_resource0lock_st_id6_END (27)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_1_resource0lock_st7_START (28)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_1_resource0lock_st7_END (28)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_1_resource0lock_st_id7_START (29)
#define SOC_SCTRL_SC_LOCK_STAT_SEC_1_resource0lock_st_id7_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0: 1;
        unsigned int reserved_1: 2;
        unsigned int reserved_2: 29;
    } reg;
} SOC_SCTRL_SCJTAG_SEL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int cfg_djtag : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_SCTRL_SCCFG_DJTAG_UNION;
#endif
#define SOC_SCTRL_SCCFG_DJTAG_cfg_djtag_START (0)
#define SOC_SCTRL_SCCFG_DJTAG_cfg_djtag_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int a53_cp15disable0 : 1;
        unsigned int a53_cp15disable1 : 1;
        unsigned int a53_cp15disable2 : 1;
        unsigned int a53_cp15disable3 : 1;
        unsigned int a57_cp15disable0 : 1;
        unsigned int a57_cp15disable1 : 1;
        unsigned int a57_cp15disable2 : 1;
        unsigned int a57_cp15disable3 : 1;
        unsigned int ispa7cp15disable : 1;
        unsigned int gicdisable : 1;
        unsigned int reserved : 22;
    } reg;
} SOC_SCTRL_SCCP15_DISABLE_UNION;
#endif
#define SOC_SCTRL_SCCP15_DISABLE_a53_cp15disable0_START (0)
#define SOC_SCTRL_SCCP15_DISABLE_a53_cp15disable0_END (0)
#define SOC_SCTRL_SCCP15_DISABLE_a53_cp15disable1_START (1)
#define SOC_SCTRL_SCCP15_DISABLE_a53_cp15disable1_END (1)
#define SOC_SCTRL_SCCP15_DISABLE_a53_cp15disable2_START (2)
#define SOC_SCTRL_SCCP15_DISABLE_a53_cp15disable2_END (2)
#define SOC_SCTRL_SCCP15_DISABLE_a53_cp15disable3_START (3)
#define SOC_SCTRL_SCCP15_DISABLE_a53_cp15disable3_END (3)
#define SOC_SCTRL_SCCP15_DISABLE_a57_cp15disable0_START (4)
#define SOC_SCTRL_SCCP15_DISABLE_a57_cp15disable0_END (4)
#define SOC_SCTRL_SCCP15_DISABLE_a57_cp15disable1_START (5)
#define SOC_SCTRL_SCCP15_DISABLE_a57_cp15disable1_END (5)
#define SOC_SCTRL_SCCP15_DISABLE_a57_cp15disable2_START (6)
#define SOC_SCTRL_SCCP15_DISABLE_a57_cp15disable2_END (6)
#define SOC_SCTRL_SCCP15_DISABLE_a57_cp15disable3_START (7)
#define SOC_SCTRL_SCCP15_DISABLE_a57_cp15disable3_END (7)
#define SOC_SCTRL_SCCP15_DISABLE_ispa7cp15disable_START (8)
#define SOC_SCTRL_SCCP15_DISABLE_ispa7cp15disable_END (8)
#define SOC_SCTRL_SCCP15_DISABLE_gicdisable_START (9)
#define SOC_SCTRL_SCCP15_DISABLE_gicdisable_END (9)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int cfg_arm_dbgen : 4;
        unsigned int reserved : 28;
    } reg;
} SOC_SCTRL_SCCFG_ARM_DBGEN_UNION;
#endif
#define SOC_SCTRL_SCCFG_ARM_DBGEN_cfg_arm_dbgen_START (0)
#define SOC_SCTRL_SCCFG_ARM_DBGEN_cfg_arm_dbgen_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int arm_dbgen_wr : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_SCTRL_SCARM_DBGEN_STAT_UNION;
#endif
#define SOC_SCTRL_SCARM_DBGEN_STAT_arm_dbgen_wr_START (0)
#define SOC_SCTRL_SCARM_DBGEN_STAT_arm_dbgen_wr_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int sys_efuse_pad_sel : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_SCTRL_SCEFUSECTRL_UNION;
#endif
#define SOC_SCTRL_SCEFUSECTRL_sys_efuse_pad_sel_START (0)
#define SOC_SCTRL_SCEFUSECTRL_sys_efuse_pad_sel_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0: 1;
        unsigned int reserved_1: 31;
    } reg;
} SOC_SCTRL_SCEFUSESEL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int CHIP_ID0 : 32;
    } reg;
} SOC_SCTRL_SCCHIP_ID0_UNION;
#endif
#define SOC_SCTRL_SCCHIP_ID0_CHIP_ID0_START (0)
#define SOC_SCTRL_SCCHIP_ID0_CHIP_ID0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int CHIP_ID1 : 32;
    } reg;
} SOC_SCTRL_SCCHIP_ID1_UNION;
#endif
#define SOC_SCTRL_SCCHIP_ID1_CHIP_ID1_START (0)
#define SOC_SCTRL_SCCHIP_ID1_CHIP_ID1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int a53_cryptodisable : 3;
        unsigned int a53_sc_cfg : 5;
        unsigned int reserved : 24;
    } reg;
} SOC_SCTRL_SCCPUSECCTRL_UNION;
#endif
#define SOC_SCTRL_SCCPUSECCTRL_a53_cryptodisable_START (0)
#define SOC_SCTRL_SCCPUSECCTRL_a53_cryptodisable_END (2)
#define SOC_SCTRL_SCCPUSECCTRL_a53_sc_cfg_START (3)
#define SOC_SCTRL_SCCPUSECCTRL_a53_sc_cfg_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int jtagsd_sw_sel : 8;
        unsigned int auth_pass : 1;
        unsigned int arm_dbgen_wr1 : 1;
        unsigned int reserved : 15;
        unsigned int jtag_sim_hwselmd : 2;
        unsigned int jtag_sd_hwselmd : 2;
        unsigned int gpio_sd_sim_sel : 1;
        unsigned int jtag_sim_mode : 1;
        unsigned int jtag_sd_mode : 1;
    } reg;
} SOC_SCTRL_SCJTAGSD_SW_SEL_UNION;
#endif
#define SOC_SCTRL_SCJTAGSD_SW_SEL_jtagsd_sw_sel_START (0)
#define SOC_SCTRL_SCJTAGSD_SW_SEL_jtagsd_sw_sel_END (7)
#define SOC_SCTRL_SCJTAGSD_SW_SEL_auth_pass_START (8)
#define SOC_SCTRL_SCJTAGSD_SW_SEL_auth_pass_END (8)
#define SOC_SCTRL_SCJTAGSD_SW_SEL_arm_dbgen_wr1_START (9)
#define SOC_SCTRL_SCJTAGSD_SW_SEL_arm_dbgen_wr1_END (9)
#define SOC_SCTRL_SCJTAGSD_SW_SEL_jtag_sim_hwselmd_START (25)
#define SOC_SCTRL_SCJTAGSD_SW_SEL_jtag_sim_hwselmd_END (26)
#define SOC_SCTRL_SCJTAGSD_SW_SEL_jtag_sd_hwselmd_START (27)
#define SOC_SCTRL_SCJTAGSD_SW_SEL_jtag_sd_hwselmd_END (28)
#define SOC_SCTRL_SCJTAGSD_SW_SEL_gpio_sd_sim_sel_START (29)
#define SOC_SCTRL_SCJTAGSD_SW_SEL_gpio_sd_sim_sel_END (29)
#define SOC_SCTRL_SCJTAGSD_SW_SEL_jtag_sim_mode_START (30)
#define SOC_SCTRL_SCJTAGSD_SW_SEL_jtag_sim_mode_END (30)
#define SOC_SCTRL_SCJTAGSD_SW_SEL_jtag_sd_mode_START (31)
#define SOC_SCTRL_SCJTAGSD_SW_SEL_jtag_sd_mode_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int jtagsys_sw_sel : 8;
        unsigned int reserved_0 : 8;
        unsigned int reserved_1 : 16;
    } reg;
} SOC_SCTRL_SCJTAGSYS_SW_SEL_UNION;
#endif
#define SOC_SCTRL_SCJTAGSYS_SW_SEL_jtagsys_sw_sel_START (0)
#define SOC_SCTRL_SCJTAGSYS_SW_SEL_jtagsys_sw_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int intr_iomcu_enable_syscache_gen : 1;
        unsigned int intr_iomcu_enable_syscache_clr : 1;
        unsigned int intr_iomcu_enable_syscache_msk : 1;
        unsigned int reserved_0 : 1;
        unsigned int intr_iomcu_enable_syscache_raw : 1;
        unsigned int intr_iomcu_enable_syscache_stat : 1;
        unsigned int reserved_1 : 2;
        unsigned int intr_iomcu_disable_syscache_gen : 1;
        unsigned int intr_iomcu_disable_syscache_clr : 1;
        unsigned int intr_iomcu_disable_syscache_msk : 1;
        unsigned int reserved_2 : 1;
        unsigned int intr_iomcu_disable_syscache_raw : 1;
        unsigned int intr_iomcu_disable_syscache_stat : 1;
        unsigned int reserved_3 : 2;
        unsigned int reserved_4 : 16;
    } reg;
} SOC_SCTRL_SC_INTR_SYSCACHE_SEC_UNION;
#endif
#define SOC_SCTRL_SC_INTR_SYSCACHE_SEC_intr_iomcu_enable_syscache_gen_START (0)
#define SOC_SCTRL_SC_INTR_SYSCACHE_SEC_intr_iomcu_enable_syscache_gen_END (0)
#define SOC_SCTRL_SC_INTR_SYSCACHE_SEC_intr_iomcu_enable_syscache_clr_START (1)
#define SOC_SCTRL_SC_INTR_SYSCACHE_SEC_intr_iomcu_enable_syscache_clr_END (1)
#define SOC_SCTRL_SC_INTR_SYSCACHE_SEC_intr_iomcu_enable_syscache_msk_START (2)
#define SOC_SCTRL_SC_INTR_SYSCACHE_SEC_intr_iomcu_enable_syscache_msk_END (2)
#define SOC_SCTRL_SC_INTR_SYSCACHE_SEC_intr_iomcu_enable_syscache_raw_START (4)
#define SOC_SCTRL_SC_INTR_SYSCACHE_SEC_intr_iomcu_enable_syscache_raw_END (4)
#define SOC_SCTRL_SC_INTR_SYSCACHE_SEC_intr_iomcu_enable_syscache_stat_START (5)
#define SOC_SCTRL_SC_INTR_SYSCACHE_SEC_intr_iomcu_enable_syscache_stat_END (5)
#define SOC_SCTRL_SC_INTR_SYSCACHE_SEC_intr_iomcu_disable_syscache_gen_START (8)
#define SOC_SCTRL_SC_INTR_SYSCACHE_SEC_intr_iomcu_disable_syscache_gen_END (8)
#define SOC_SCTRL_SC_INTR_SYSCACHE_SEC_intr_iomcu_disable_syscache_clr_START (9)
#define SOC_SCTRL_SC_INTR_SYSCACHE_SEC_intr_iomcu_disable_syscache_clr_END (9)
#define SOC_SCTRL_SC_INTR_SYSCACHE_SEC_intr_iomcu_disable_syscache_msk_START (10)
#define SOC_SCTRL_SC_INTR_SYSCACHE_SEC_intr_iomcu_disable_syscache_msk_END (10)
#define SOC_SCTRL_SC_INTR_SYSCACHE_SEC_intr_iomcu_disable_syscache_raw_START (12)
#define SOC_SCTRL_SC_INTR_SYSCACHE_SEC_intr_iomcu_disable_syscache_raw_END (12)
#define SOC_SCTRL_SC_INTR_SYSCACHE_SEC_intr_iomcu_disable_syscache_stat_START (13)
#define SOC_SCTRL_SC_INTR_SYSCACHE_SEC_intr_iomcu_disable_syscache_stat_END (13)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int arm_dbg_key0 : 32;
    } reg;
} SOC_SCTRL_SCARM_DBG_KEY0_UNION;
#endif
#define SOC_SCTRL_SCARM_DBG_KEY0_arm_dbg_key0_START (0)
#define SOC_SCTRL_SCARM_DBG_KEY0_arm_dbg_key0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int arm_dbg_key1 : 32;
    } reg;
} SOC_SCTRL_SCARM_DBG_KEY1_UNION;
#endif
#define SOC_SCTRL_SCARM_DBG_KEY1_arm_dbg_key1_START (0)
#define SOC_SCTRL_SCARM_DBG_KEY1_arm_dbg_key1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int arm_dbg_key2 : 32;
    } reg;
} SOC_SCTRL_SCARM_DBG_KEY2_UNION;
#endif
#define SOC_SCTRL_SCARM_DBG_KEY2_arm_dbg_key2_START (0)
#define SOC_SCTRL_SCARM_DBG_KEY2_arm_dbg_key2_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int arm_dbg_key3 : 32;
    } reg;
} SOC_SCTRL_SCARM_DBG_KEY3_UNION;
#endif
#define SOC_SCTRL_SCARM_DBG_KEY3_arm_dbg_key3_START (0)
#define SOC_SCTRL_SCARM_DBG_KEY3_arm_dbg_key3_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ao_soft_fast_cfg_vol_idx : 8;
        unsigned int ao_soft_fast_cfg_vol_req : 1;
        unsigned int ao_soft_fast_cfg_vol_ack_spmi : 1;
        unsigned int reserved_0 : 6;
        unsigned int ao_soft_fast_cfg_vol_idx_spmi : 8;
        unsigned int ao_soft_fast_cfg_vol_chg_spmi : 1;
        unsigned int ao_soft_fast_cfg_vol_send_done : 1;
        unsigned int reserved_1 : 6;
    } reg;
} SOC_SCTRL_SC_SPMI_FAST_CFG_UNION;
#endif
#define SOC_SCTRL_SC_SPMI_FAST_CFG_ao_soft_fast_cfg_vol_idx_START (0)
#define SOC_SCTRL_SC_SPMI_FAST_CFG_ao_soft_fast_cfg_vol_idx_END (7)
#define SOC_SCTRL_SC_SPMI_FAST_CFG_ao_soft_fast_cfg_vol_req_START (8)
#define SOC_SCTRL_SC_SPMI_FAST_CFG_ao_soft_fast_cfg_vol_req_END (8)
#define SOC_SCTRL_SC_SPMI_FAST_CFG_ao_soft_fast_cfg_vol_ack_spmi_START (9)
#define SOC_SCTRL_SC_SPMI_FAST_CFG_ao_soft_fast_cfg_vol_ack_spmi_END (9)
#define SOC_SCTRL_SC_SPMI_FAST_CFG_ao_soft_fast_cfg_vol_idx_spmi_START (16)
#define SOC_SCTRL_SC_SPMI_FAST_CFG_ao_soft_fast_cfg_vol_idx_spmi_END (23)
#define SOC_SCTRL_SC_SPMI_FAST_CFG_ao_soft_fast_cfg_vol_chg_spmi_START (24)
#define SOC_SCTRL_SC_SPMI_FAST_CFG_ao_soft_fast_cfg_vol_chg_spmi_END (24)
#define SOC_SCTRL_SC_SPMI_FAST_CFG_ao_soft_fast_cfg_vol_send_done_START (25)
#define SOC_SCTRL_SC_SPMI_FAST_CFG_ao_soft_fast_cfg_vol_send_done_END (25)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int spmi_avs_addr0 : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_SCTRL_SCSPMIADDR0_UNION;
#endif
#define SOC_SCTRL_SCSPMIADDR0_spmi_avs_addr0_START (0)
#define SOC_SCTRL_SCSPMIADDR0_spmi_avs_addr0_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int spmi_avs_addr1 : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_SCTRL_SCSPMIADDR1_UNION;
#endif
#define SOC_SCTRL_SCSPMIADDR1_spmi_avs_addr1_START (0)
#define SOC_SCTRL_SCSPMIADDR1_spmi_avs_addr1_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int spmi_avs_addr2 : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_SCTRL_SCSPMIADDR2_UNION;
#endif
#define SOC_SCTRL_SCSPMIADDR2_spmi_avs_addr2_START (0)
#define SOC_SCTRL_SCSPMIADDR2_spmi_avs_addr2_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int spmi_avs_addr3 : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_SCTRL_SCSPMIADDR3_UNION;
#endif
#define SOC_SCTRL_SCSPMIADDR3_spmi_avs_addr3_START (0)
#define SOC_SCTRL_SCSPMIADDR3_spmi_avs_addr3_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int spmi_avs_addr4 : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_SCTRL_SCSPMIADDR4_UNION;
#endif
#define SOC_SCTRL_SCSPMIADDR4_spmi_avs_addr4_START (0)
#define SOC_SCTRL_SCSPMIADDR4_spmi_avs_addr4_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int spmi_avs_addr5 : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_SCTRL_SCSPMIADDR5_UNION;
#endif
#define SOC_SCTRL_SCSPMIADDR5_spmi_avs_addr5_START (0)
#define SOC_SCTRL_SCSPMIADDR5_spmi_avs_addr5_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int spmi_avs_addr6 : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_SCTRL_SCSPMIADDR6_UNION;
#endif
#define SOC_SCTRL_SCSPMIADDR6_spmi_avs_addr6_START (0)
#define SOC_SCTRL_SCSPMIADDR6_spmi_avs_addr6_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int spmi_avs_addr7 : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_SCTRL_SCSPMIADDR7_UNION;
#endif
#define SOC_SCTRL_SCSPMIADDR7_spmi_avs_addr7_START (0)
#define SOC_SCTRL_SCSPMIADDR7_spmi_avs_addr7_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int spmi_avs_addr8 : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_SCTRL_SCSPMIADDR8_UNION;
#endif
#define SOC_SCTRL_SCSPMIADDR8_spmi_avs_addr8_START (0)
#define SOC_SCTRL_SCSPMIADDR8_spmi_avs_addr8_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int spmi_avs_addr9 : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_SCTRL_SCSPMIADDR9_UNION;
#endif
#define SOC_SCTRL_SCSPMIADDR9_spmi_avs_addr9_START (0)
#define SOC_SCTRL_SCSPMIADDR9_spmi_avs_addr9_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int spmi_avs_addr10 : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_SCTRL_SCSPMIADDR10_UNION;
#endif
#define SOC_SCTRL_SCSPMIADDR10_spmi_avs_addr10_START (0)
#define SOC_SCTRL_SCSPMIADDR10_spmi_avs_addr10_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int spmi_avs_addr11 : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_SCTRL_SCSPMIADDR11_UNION;
#endif
#define SOC_SCTRL_SCSPMIADDR11_spmi_avs_addr11_START (0)
#define SOC_SCTRL_SCSPMIADDR11_spmi_avs_addr11_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int spmi_avs_addr12 : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_SCTRL_SCSPMIADDR12_UNION;
#endif
#define SOC_SCTRL_SCSPMIADDR12_spmi_avs_addr12_START (0)
#define SOC_SCTRL_SCSPMIADDR12_spmi_avs_addr12_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int spmi_avs_addr13 : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_SCTRL_SCSPMIADDR13_UNION;
#endif
#define SOC_SCTRL_SCSPMIADDR13_spmi_avs_addr13_START (0)
#define SOC_SCTRL_SCSPMIADDR13_spmi_avs_addr13_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int spmi_avs_addr14 : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_SCTRL_SCSPMIADDR14_UNION;
#endif
#define SOC_SCTRL_SCSPMIADDR14_spmi_avs_addr14_START (0)
#define SOC_SCTRL_SCSPMIADDR14_spmi_avs_addr14_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int spmi_avs_addr15 : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_SCTRL_SCSPMIADDR15_UNION;
#endif
#define SOC_SCTRL_SCSPMIADDR15_spmi_avs_addr15_START (0)
#define SOC_SCTRL_SCSPMIADDR15_spmi_avs_addr15_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved : 32;
    } reg;
} SOC_SCTRL_SCHISEEGPIOLOCK_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved : 32;
    } reg;
} SOC_SCTRL_SCHISEESPILOCK_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved : 32;
    } reg;
} SOC_SCTRL_SCHISEEI2CLOCK_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int hisee_gpio1_io_sel : 1;
        unsigned int hisee_gpio2_io_sel : 1;
        unsigned int hisee_gpio3_io_sel : 1;
        unsigned int hisee_spi_io_sel : 1;
        unsigned int hisee_i2c_io_sel : 1;
        unsigned int reserved_0 : 26;
        unsigned int reserved_1 : 1;
    } reg;
} SOC_SCTRL_SCHISEEIOSEL_UNION;
#endif
#define SOC_SCTRL_SCHISEEIOSEL_hisee_gpio1_io_sel_START (0)
#define SOC_SCTRL_SCHISEEIOSEL_hisee_gpio1_io_sel_END (0)
#define SOC_SCTRL_SCHISEEIOSEL_hisee_gpio2_io_sel_START (1)
#define SOC_SCTRL_SCHISEEIOSEL_hisee_gpio2_io_sel_END (1)
#define SOC_SCTRL_SCHISEEIOSEL_hisee_gpio3_io_sel_START (2)
#define SOC_SCTRL_SCHISEEIOSEL_hisee_gpio3_io_sel_END (2)
#define SOC_SCTRL_SCHISEEIOSEL_hisee_spi_io_sel_START (3)
#define SOC_SCTRL_SCHISEEIOSEL_hisee_spi_io_sel_END (3)
#define SOC_SCTRL_SCHISEEIOSEL_hisee_i2c_io_sel_START (4)
#define SOC_SCTRL_SCHISEEIOSEL_hisee_i2c_io_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0: 1;
        unsigned int reserved_1: 1;
        unsigned int reserved_2: 2;
        unsigned int reserved_3: 1;
        unsigned int reserved_4: 2;
        unsigned int reserved_5: 13;
        unsigned int reserved_6: 1;
        unsigned int reserved_7: 1;
        unsigned int reserved_8: 2;
        unsigned int reserved_9: 4;
        unsigned int reserved_10: 4;
    } reg;
} SOC_SCTRL_SCPERCTRL0_SEC_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0: 6;
        unsigned int reserved_1: 26;
    } reg;
} SOC_SCTRL_SCPERCTRL1_SEC_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int lpmcu_arc_dbg_port_sel : 1;
        unsigned int lpmcu_arc_halt_req_a : 1;
        unsigned int lpmcu_arc_run_req_a : 1;
        unsigned int reserved_0 : 3;
        unsigned int reserved_1 : 2;
        unsigned int reserved_2 : 8;
        unsigned int scperctrl2_sec_msk : 16;
    } reg;
} SOC_SCTRL_SCPERCTRL2_SEC_UNION;
#endif
#define SOC_SCTRL_SCPERCTRL2_SEC_lpmcu_arc_dbg_port_sel_START (0)
#define SOC_SCTRL_SCPERCTRL2_SEC_lpmcu_arc_dbg_port_sel_END (0)
#define SOC_SCTRL_SCPERCTRL2_SEC_lpmcu_arc_halt_req_a_START (1)
#define SOC_SCTRL_SCPERCTRL2_SEC_lpmcu_arc_halt_req_a_END (1)
#define SOC_SCTRL_SCPERCTRL2_SEC_lpmcu_arc_run_req_a_START (2)
#define SOC_SCTRL_SCPERCTRL2_SEC_lpmcu_arc_run_req_a_END (2)
#define SOC_SCTRL_SCPERCTRL2_SEC_scperctrl2_sec_msk_START (16)
#define SOC_SCTRL_SCPERCTRL2_SEC_scperctrl2_sec_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0: 3;
        unsigned int reserved_1: 1;
        unsigned int reserved_2: 28;
    } reg;
} SOC_SCTRL_SCPERCTRL3_SEC_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int hisee_gpio1_ack : 4;
        unsigned int hisee_gpio2_ack : 4;
        unsigned int hisee_gpio3_ack : 4;
        unsigned int reserved : 20;
    } reg;
} SOC_SCTRL_SCPERCTRL4_SEC_UNION;
#endif
#define SOC_SCTRL_SCPERCTRL4_SEC_hisee_gpio1_ack_START (0)
#define SOC_SCTRL_SCPERCTRL4_SEC_hisee_gpio1_ack_END (3)
#define SOC_SCTRL_SCPERCTRL4_SEC_hisee_gpio2_ack_START (4)
#define SOC_SCTRL_SCPERCTRL4_SEC_hisee_gpio2_ack_END (7)
#define SOC_SCTRL_SCPERCTRL4_SEC_hisee_gpio3_ack_START (8)
#define SOC_SCTRL_SCPERCTRL4_SEC_hisee_gpio3_ack_END (11)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0 : 8;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 4;
        unsigned int iomcu_debug_sel : 1;
        unsigned int scperctrl5_sec_msk : 16;
    } reg;
} SOC_SCTRL_SCPERCTRL5_SEC_UNION;
#endif
#define SOC_SCTRL_SCPERCTRL5_SEC_iomcu_debug_sel_START (15)
#define SOC_SCTRL_SCPERCTRL5_SEC_iomcu_debug_sel_END (15)
#define SOC_SCTRL_SCPERCTRL5_SEC_scperctrl5_sec_msk_START (16)
#define SOC_SCTRL_SCPERCTRL5_SEC_scperctrl5_sec_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int arc_run_ack : 1;
        unsigned int arc_halt_ack : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 3;
        unsigned int reserved_4 : 8;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int reserved_11 : 9;
        unsigned int reserved_12 : 1;
    } reg;
} SOC_SCTRL_SCPERSTATUS0_SEC_UNION;
#endif
#define SOC_SCTRL_SCPERSTATUS0_SEC_arc_run_ack_START (0)
#define SOC_SCTRL_SCPERSTATUS0_SEC_arc_run_ack_END (0)
#define SOC_SCTRL_SCPERSTATUS0_SEC_arc_halt_ack_START (1)
#define SOC_SCTRL_SCPERSTATUS0_SEC_arc_halt_ack_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved : 32;
    } reg;
} SOC_SCTRL_SCPERSTATUS1_SEC_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved : 32;
    } reg;
} SOC_SCTRL_SCPERSTATUS2_SEC_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved : 32;
    } reg;
} SOC_SCTRL_SCPERSTATUS3_SEC_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int vote_req_0 : 7;
        unsigned int vote_en_0 : 1;
        unsigned int vote_req_1 : 7;
        unsigned int vote_en_1 : 1;
        unsigned int sc_tcp_vote_req_0_msk : 16;
    } reg;
} SOC_SCTRL_SC_TCP_VOTE_REQ_0_UNION;
#endif
#define SOC_SCTRL_SC_TCP_VOTE_REQ_0_vote_req_0_START (0)
#define SOC_SCTRL_SC_TCP_VOTE_REQ_0_vote_req_0_END (6)
#define SOC_SCTRL_SC_TCP_VOTE_REQ_0_vote_en_0_START (7)
#define SOC_SCTRL_SC_TCP_VOTE_REQ_0_vote_en_0_END (7)
#define SOC_SCTRL_SC_TCP_VOTE_REQ_0_vote_req_1_START (8)
#define SOC_SCTRL_SC_TCP_VOTE_REQ_0_vote_req_1_END (14)
#define SOC_SCTRL_SC_TCP_VOTE_REQ_0_vote_en_1_START (15)
#define SOC_SCTRL_SC_TCP_VOTE_REQ_0_vote_en_1_END (15)
#define SOC_SCTRL_SC_TCP_VOTE_REQ_0_sc_tcp_vote_req_0_msk_START (16)
#define SOC_SCTRL_SC_TCP_VOTE_REQ_0_sc_tcp_vote_req_0_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int vote_close_req_0 : 1;
        unsigned int vote_open_req_0 : 1;
        unsigned int vote_close_ack_sync_0 : 1;
        unsigned int vote_open_ack_sync_0 : 1;
        unsigned int reserved_0 : 1;
        unsigned int cur_path_stat_0 : 3;
        unsigned int vote_close_req_1 : 1;
        unsigned int vote_open_req_1 : 1;
        unsigned int vote_close_ack_sync_1 : 1;
        unsigned int vote_open_ack_sync_1 : 1;
        unsigned int reserved_1 : 1;
        unsigned int cur_path_stat_1 : 3;
        unsigned int vote_close_req_2 : 1;
        unsigned int vote_open_req_2 : 1;
        unsigned int vote_close_ack_sync_2 : 1;
        unsigned int vote_open_ack_sync_2 : 1;
        unsigned int reserved_2 : 1;
        unsigned int cur_path_stat_2 : 3;
        unsigned int vote_close_req_3 : 1;
        unsigned int vote_open_req_3 : 1;
        unsigned int vote_close_ack_sync_3 : 1;
        unsigned int vote_open_ack_sync_3 : 1;
        unsigned int reserved_3 : 1;
        unsigned int cur_path_stat_3 : 3;
    } reg;
} SOC_SCTRL_SC_TCP_VOTE_STAT_UNION;
#endif
#define SOC_SCTRL_SC_TCP_VOTE_STAT_vote_close_req_0_START (0)
#define SOC_SCTRL_SC_TCP_VOTE_STAT_vote_close_req_0_END (0)
#define SOC_SCTRL_SC_TCP_VOTE_STAT_vote_open_req_0_START (1)
#define SOC_SCTRL_SC_TCP_VOTE_STAT_vote_open_req_0_END (1)
#define SOC_SCTRL_SC_TCP_VOTE_STAT_vote_close_ack_sync_0_START (2)
#define SOC_SCTRL_SC_TCP_VOTE_STAT_vote_close_ack_sync_0_END (2)
#define SOC_SCTRL_SC_TCP_VOTE_STAT_vote_open_ack_sync_0_START (3)
#define SOC_SCTRL_SC_TCP_VOTE_STAT_vote_open_ack_sync_0_END (3)
#define SOC_SCTRL_SC_TCP_VOTE_STAT_cur_path_stat_0_START (5)
#define SOC_SCTRL_SC_TCP_VOTE_STAT_cur_path_stat_0_END (7)
#define SOC_SCTRL_SC_TCP_VOTE_STAT_vote_close_req_1_START (8)
#define SOC_SCTRL_SC_TCP_VOTE_STAT_vote_close_req_1_END (8)
#define SOC_SCTRL_SC_TCP_VOTE_STAT_vote_open_req_1_START (9)
#define SOC_SCTRL_SC_TCP_VOTE_STAT_vote_open_req_1_END (9)
#define SOC_SCTRL_SC_TCP_VOTE_STAT_vote_close_ack_sync_1_START (10)
#define SOC_SCTRL_SC_TCP_VOTE_STAT_vote_close_ack_sync_1_END (10)
#define SOC_SCTRL_SC_TCP_VOTE_STAT_vote_open_ack_sync_1_START (11)
#define SOC_SCTRL_SC_TCP_VOTE_STAT_vote_open_ack_sync_1_END (11)
#define SOC_SCTRL_SC_TCP_VOTE_STAT_cur_path_stat_1_START (13)
#define SOC_SCTRL_SC_TCP_VOTE_STAT_cur_path_stat_1_END (15)
#define SOC_SCTRL_SC_TCP_VOTE_STAT_vote_close_req_2_START (16)
#define SOC_SCTRL_SC_TCP_VOTE_STAT_vote_close_req_2_END (16)
#define SOC_SCTRL_SC_TCP_VOTE_STAT_vote_open_req_2_START (17)
#define SOC_SCTRL_SC_TCP_VOTE_STAT_vote_open_req_2_END (17)
#define SOC_SCTRL_SC_TCP_VOTE_STAT_vote_close_ack_sync_2_START (18)
#define SOC_SCTRL_SC_TCP_VOTE_STAT_vote_close_ack_sync_2_END (18)
#define SOC_SCTRL_SC_TCP_VOTE_STAT_vote_open_ack_sync_2_START (19)
#define SOC_SCTRL_SC_TCP_VOTE_STAT_vote_open_ack_sync_2_END (19)
#define SOC_SCTRL_SC_TCP_VOTE_STAT_cur_path_stat_2_START (21)
#define SOC_SCTRL_SC_TCP_VOTE_STAT_cur_path_stat_2_END (23)
#define SOC_SCTRL_SC_TCP_VOTE_STAT_vote_close_req_3_START (24)
#define SOC_SCTRL_SC_TCP_VOTE_STAT_vote_close_req_3_END (24)
#define SOC_SCTRL_SC_TCP_VOTE_STAT_vote_open_req_3_START (25)
#define SOC_SCTRL_SC_TCP_VOTE_STAT_vote_open_req_3_END (25)
#define SOC_SCTRL_SC_TCP_VOTE_STAT_vote_close_ack_sync_3_START (26)
#define SOC_SCTRL_SC_TCP_VOTE_STAT_vote_close_ack_sync_3_END (26)
#define SOC_SCTRL_SC_TCP_VOTE_STAT_vote_open_ack_sync_3_START (27)
#define SOC_SCTRL_SC_TCP_VOTE_STAT_vote_open_ack_sync_3_END (27)
#define SOC_SCTRL_SC_TCP_VOTE_STAT_cur_path_stat_3_START (29)
#define SOC_SCTRL_SC_TCP_VOTE_STAT_cur_path_stat_3_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int tcp_busy_stat_sync : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int int_wakeup_sys_ff2 : 1;
        unsigned int reserved_6 : 1;
        unsigned int mad_int_sync : 1;
        unsigned int reserved_7 : 21;
        unsigned int reserved_8 : 1;
    } reg;
} SOC_SCTRL_SC_TCP_VOTE_CTRL0_UNION;
#endif
#define SOC_SCTRL_SC_TCP_VOTE_CTRL0_tcp_busy_stat_sync_START (4)
#define SOC_SCTRL_SC_TCP_VOTE_CTRL0_tcp_busy_stat_sync_END (4)
#define SOC_SCTRL_SC_TCP_VOTE_CTRL0_int_wakeup_sys_ff2_START (7)
#define SOC_SCTRL_SC_TCP_VOTE_CTRL0_int_wakeup_sys_ff2_END (7)
#define SOC_SCTRL_SC_TCP_VOTE_CTRL0_mad_int_sync_START (9)
#define SOC_SCTRL_SC_TCP_VOTE_CTRL0_mad_int_sync_END (9)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int vote_req_2 : 7;
        unsigned int vote_en_2 : 1;
        unsigned int vote_req_3 : 7;
        unsigned int vote_en_3 : 1;
        unsigned int sc_tcp_vote_req_1_msk : 16;
    } reg;
} SOC_SCTRL_SC_TCP_VOTE_REQ_1_UNION;
#endif
#define SOC_SCTRL_SC_TCP_VOTE_REQ_1_vote_req_2_START (0)
#define SOC_SCTRL_SC_TCP_VOTE_REQ_1_vote_req_2_END (6)
#define SOC_SCTRL_SC_TCP_VOTE_REQ_1_vote_en_2_START (7)
#define SOC_SCTRL_SC_TCP_VOTE_REQ_1_vote_en_2_END (7)
#define SOC_SCTRL_SC_TCP_VOTE_REQ_1_vote_req_3_START (8)
#define SOC_SCTRL_SC_TCP_VOTE_REQ_1_vote_req_3_END (14)
#define SOC_SCTRL_SC_TCP_VOTE_REQ_1_vote_en_3_START (15)
#define SOC_SCTRL_SC_TCP_VOTE_REQ_1_vote_en_3_END (15)
#define SOC_SCTRL_SC_TCP_VOTE_REQ_1_sc_tcp_vote_req_1_msk_START (16)
#define SOC_SCTRL_SC_TCP_VOTE_REQ_1_sc_tcp_vote_req_1_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int vote_mng_sw2_path_open_req_0 : 1;
        unsigned int vote_mng_sw2_path_close_req_0 : 1;
        unsigned int vote_mng_sw2_path_open_req_1 : 1;
        unsigned int vote_mng_sw2_path_close_req_1 : 1;
        unsigned int vote_mng_sw2_path_open_req_2 : 1;
        unsigned int vote_mng_sw2_path_close_req_2 : 1;
        unsigned int vote_mng_sw2_path_open_req_3 : 1;
        unsigned int vote_mng_sw2_path_close_req_3 : 1;
        unsigned int reserved : 24;
    } reg;
} SOC_SCTRL_SC_TCP_VOTE_CTRL1_UNION;
#endif
#define SOC_SCTRL_SC_TCP_VOTE_CTRL1_vote_mng_sw2_path_open_req_0_START (0)
#define SOC_SCTRL_SC_TCP_VOTE_CTRL1_vote_mng_sw2_path_open_req_0_END (0)
#define SOC_SCTRL_SC_TCP_VOTE_CTRL1_vote_mng_sw2_path_close_req_0_START (1)
#define SOC_SCTRL_SC_TCP_VOTE_CTRL1_vote_mng_sw2_path_close_req_0_END (1)
#define SOC_SCTRL_SC_TCP_VOTE_CTRL1_vote_mng_sw2_path_open_req_1_START (2)
#define SOC_SCTRL_SC_TCP_VOTE_CTRL1_vote_mng_sw2_path_open_req_1_END (2)
#define SOC_SCTRL_SC_TCP_VOTE_CTRL1_vote_mng_sw2_path_close_req_1_START (3)
#define SOC_SCTRL_SC_TCP_VOTE_CTRL1_vote_mng_sw2_path_close_req_1_END (3)
#define SOC_SCTRL_SC_TCP_VOTE_CTRL1_vote_mng_sw2_path_open_req_2_START (4)
#define SOC_SCTRL_SC_TCP_VOTE_CTRL1_vote_mng_sw2_path_open_req_2_END (4)
#define SOC_SCTRL_SC_TCP_VOTE_CTRL1_vote_mng_sw2_path_close_req_2_START (5)
#define SOC_SCTRL_SC_TCP_VOTE_CTRL1_vote_mng_sw2_path_close_req_2_END (5)
#define SOC_SCTRL_SC_TCP_VOTE_CTRL1_vote_mng_sw2_path_open_req_3_START (6)
#define SOC_SCTRL_SC_TCP_VOTE_CTRL1_vote_mng_sw2_path_open_req_3_END (6)
#define SOC_SCTRL_SC_TCP_VOTE_CTRL1_vote_mng_sw2_path_close_req_3_START (7)
#define SOC_SCTRL_SC_TCP_VOTE_CTRL1_vote_mng_sw2_path_close_req_3_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int vote_req_3_add0 : 16;
        unsigned int sc_tcp_vote_req3_add0_msk : 16;
    } reg;
} SOC_SCTRL_SC_TCP_VOTE_REQ3_ADD0_UNION;
#endif
#define SOC_SCTRL_SC_TCP_VOTE_REQ3_ADD0_vote_req_3_add0_START (0)
#define SOC_SCTRL_SC_TCP_VOTE_REQ3_ADD0_vote_req_3_add0_END (15)
#define SOC_SCTRL_SC_TCP_VOTE_REQ3_ADD0_sc_tcp_vote_req3_add0_msk_START (16)
#define SOC_SCTRL_SC_TCP_VOTE_REQ3_ADD0_sc_tcp_vote_req3_add0_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int vote_req_3_add1 : 16;
        unsigned int sc_tcp_vote_req3_add1_msk : 16;
    } reg;
} SOC_SCTRL_SC_TCP_VOTE_REQ3_ADD1_UNION;
#endif
#define SOC_SCTRL_SC_TCP_VOTE_REQ3_ADD1_vote_req_3_add1_START (0)
#define SOC_SCTRL_SC_TCP_VOTE_REQ3_ADD1_vote_req_3_add1_END (15)
#define SOC_SCTRL_SC_TCP_VOTE_REQ3_ADD1_sc_tcp_vote_req3_add1_msk_START (16)
#define SOC_SCTRL_SC_TCP_VOTE_REQ3_ADD1_sc_tcp_vote_req3_add1_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int gt_pclk_timer1 : 1;
        unsigned int reserved_0 : 1;
        unsigned int gt_pclk_ao_tzpc : 1;
        unsigned int gt_pclk_efusec2 : 1;
        unsigned int gt_pclk_efusec : 1;
        unsigned int gt_clk_out0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int gt_pclk_ao_gpio1_se : 1;
        unsigned int gt_clk_ao_tcp : 1;
        unsigned int gt_clk_ao_tcp_32k : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int gt_pclk_ao_ipc : 1;
        unsigned int reserved_11 : 1;
        unsigned int reserved_12 : 1;
        unsigned int reserved_13 : 1;
        unsigned int reserved_14 : 1;
        unsigned int gt_pclkdbg_to_iomcu : 1;
        unsigned int gt_clk_noc_aobus2iomcubus : 1;
        unsigned int reserved_15 : 1;
        unsigned int reserved_16 : 1;
        unsigned int reserved_17 : 1;
        unsigned int reserved_18 : 1;
        unsigned int reserved_19 : 1;
        unsigned int reserved_20 : 1;
    } reg;
} SOC_SCTRL_SCPEREN0_SEC_UNION;
#endif
#define SOC_SCTRL_SCPEREN0_SEC_gt_pclk_timer1_START (0)
#define SOC_SCTRL_SCPEREN0_SEC_gt_pclk_timer1_END (0)
#define SOC_SCTRL_SCPEREN0_SEC_gt_pclk_ao_tzpc_START (2)
#define SOC_SCTRL_SCPEREN0_SEC_gt_pclk_ao_tzpc_END (2)
#define SOC_SCTRL_SCPEREN0_SEC_gt_pclk_efusec2_START (3)
#define SOC_SCTRL_SCPEREN0_SEC_gt_pclk_efusec2_END (3)
#define SOC_SCTRL_SCPEREN0_SEC_gt_pclk_efusec_START (4)
#define SOC_SCTRL_SCPEREN0_SEC_gt_pclk_efusec_END (4)
#define SOC_SCTRL_SCPEREN0_SEC_gt_clk_out0_START (5)
#define SOC_SCTRL_SCPEREN0_SEC_gt_clk_out0_END (5)
#define SOC_SCTRL_SCPEREN0_SEC_gt_pclk_ao_gpio1_se_START (7)
#define SOC_SCTRL_SCPEREN0_SEC_gt_pclk_ao_gpio1_se_END (7)
#define SOC_SCTRL_SCPEREN0_SEC_gt_clk_ao_tcp_START (8)
#define SOC_SCTRL_SCPEREN0_SEC_gt_clk_ao_tcp_END (8)
#define SOC_SCTRL_SCPEREN0_SEC_gt_clk_ao_tcp_32k_START (9)
#define SOC_SCTRL_SCPEREN0_SEC_gt_clk_ao_tcp_32k_END (9)
#define SOC_SCTRL_SCPEREN0_SEC_gt_pclk_ao_ipc_START (19)
#define SOC_SCTRL_SCPEREN0_SEC_gt_pclk_ao_ipc_END (19)
#define SOC_SCTRL_SCPEREN0_SEC_gt_pclkdbg_to_iomcu_START (24)
#define SOC_SCTRL_SCPEREN0_SEC_gt_pclkdbg_to_iomcu_END (24)
#define SOC_SCTRL_SCPEREN0_SEC_gt_clk_noc_aobus2iomcubus_START (25)
#define SOC_SCTRL_SCPEREN0_SEC_gt_clk_noc_aobus2iomcubus_END (25)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int gt_pclk_timer1 : 1;
        unsigned int reserved_0 : 1;
        unsigned int gt_pclk_ao_tzpc : 1;
        unsigned int gt_pclk_efusec2 : 1;
        unsigned int gt_pclk_efusec : 1;
        unsigned int gt_clk_out0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int gt_pclk_ao_gpio1_se : 1;
        unsigned int gt_clk_ao_tcp : 1;
        unsigned int gt_clk_ao_tcp_32k : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int gt_pclk_ao_ipc : 1;
        unsigned int reserved_11 : 1;
        unsigned int reserved_12 : 1;
        unsigned int reserved_13 : 1;
        unsigned int reserved_14 : 1;
        unsigned int gt_pclkdbg_to_iomcu : 1;
        unsigned int gt_clk_noc_aobus2iomcubus : 1;
        unsigned int reserved_15 : 1;
        unsigned int reserved_16 : 1;
        unsigned int reserved_17 : 1;
        unsigned int reserved_18 : 1;
        unsigned int reserved_19 : 1;
        unsigned int reserved_20 : 1;
    } reg;
} SOC_SCTRL_SCPERDIS0_SEC_UNION;
#endif
#define SOC_SCTRL_SCPERDIS0_SEC_gt_pclk_timer1_START (0)
#define SOC_SCTRL_SCPERDIS0_SEC_gt_pclk_timer1_END (0)
#define SOC_SCTRL_SCPERDIS0_SEC_gt_pclk_ao_tzpc_START (2)
#define SOC_SCTRL_SCPERDIS0_SEC_gt_pclk_ao_tzpc_END (2)
#define SOC_SCTRL_SCPERDIS0_SEC_gt_pclk_efusec2_START (3)
#define SOC_SCTRL_SCPERDIS0_SEC_gt_pclk_efusec2_END (3)
#define SOC_SCTRL_SCPERDIS0_SEC_gt_pclk_efusec_START (4)
#define SOC_SCTRL_SCPERDIS0_SEC_gt_pclk_efusec_END (4)
#define SOC_SCTRL_SCPERDIS0_SEC_gt_clk_out0_START (5)
#define SOC_SCTRL_SCPERDIS0_SEC_gt_clk_out0_END (5)
#define SOC_SCTRL_SCPERDIS0_SEC_gt_pclk_ao_gpio1_se_START (7)
#define SOC_SCTRL_SCPERDIS0_SEC_gt_pclk_ao_gpio1_se_END (7)
#define SOC_SCTRL_SCPERDIS0_SEC_gt_clk_ao_tcp_START (8)
#define SOC_SCTRL_SCPERDIS0_SEC_gt_clk_ao_tcp_END (8)
#define SOC_SCTRL_SCPERDIS0_SEC_gt_clk_ao_tcp_32k_START (9)
#define SOC_SCTRL_SCPERDIS0_SEC_gt_clk_ao_tcp_32k_END (9)
#define SOC_SCTRL_SCPERDIS0_SEC_gt_pclk_ao_ipc_START (19)
#define SOC_SCTRL_SCPERDIS0_SEC_gt_pclk_ao_ipc_END (19)
#define SOC_SCTRL_SCPERDIS0_SEC_gt_pclkdbg_to_iomcu_START (24)
#define SOC_SCTRL_SCPERDIS0_SEC_gt_pclkdbg_to_iomcu_END (24)
#define SOC_SCTRL_SCPERDIS0_SEC_gt_clk_noc_aobus2iomcubus_START (25)
#define SOC_SCTRL_SCPERDIS0_SEC_gt_clk_noc_aobus2iomcubus_END (25)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int gt_pclk_timer1 : 1;
        unsigned int reserved_0 : 1;
        unsigned int gt_pclk_ao_tzpc : 1;
        unsigned int gt_pclk_efusec2 : 1;
        unsigned int gt_pclk_efusec : 1;
        unsigned int gt_clk_out0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int gt_pclk_ao_gpio1_se : 1;
        unsigned int gt_clk_ao_tcp : 1;
        unsigned int gt_clk_ao_tcp_32k : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int gt_pclk_ao_ipc : 1;
        unsigned int reserved_11 : 1;
        unsigned int reserved_12 : 1;
        unsigned int reserved_13 : 1;
        unsigned int reserved_14 : 1;
        unsigned int gt_pclkdbg_to_iomcu : 1;
        unsigned int gt_clk_noc_aobus2iomcubus : 1;
        unsigned int reserved_15 : 1;
        unsigned int reserved_16 : 1;
        unsigned int reserved_17 : 1;
        unsigned int reserved_18 : 1;
        unsigned int reserved_19 : 1;
        unsigned int reserved_20 : 1;
    } reg;
} SOC_SCTRL_SCPERCLKEN0_SEC_UNION;
#endif
#define SOC_SCTRL_SCPERCLKEN0_SEC_gt_pclk_timer1_START (0)
#define SOC_SCTRL_SCPERCLKEN0_SEC_gt_pclk_timer1_END (0)
#define SOC_SCTRL_SCPERCLKEN0_SEC_gt_pclk_ao_tzpc_START (2)
#define SOC_SCTRL_SCPERCLKEN0_SEC_gt_pclk_ao_tzpc_END (2)
#define SOC_SCTRL_SCPERCLKEN0_SEC_gt_pclk_efusec2_START (3)
#define SOC_SCTRL_SCPERCLKEN0_SEC_gt_pclk_efusec2_END (3)
#define SOC_SCTRL_SCPERCLKEN0_SEC_gt_pclk_efusec_START (4)
#define SOC_SCTRL_SCPERCLKEN0_SEC_gt_pclk_efusec_END (4)
#define SOC_SCTRL_SCPERCLKEN0_SEC_gt_clk_out0_START (5)
#define SOC_SCTRL_SCPERCLKEN0_SEC_gt_clk_out0_END (5)
#define SOC_SCTRL_SCPERCLKEN0_SEC_gt_pclk_ao_gpio1_se_START (7)
#define SOC_SCTRL_SCPERCLKEN0_SEC_gt_pclk_ao_gpio1_se_END (7)
#define SOC_SCTRL_SCPERCLKEN0_SEC_gt_clk_ao_tcp_START (8)
#define SOC_SCTRL_SCPERCLKEN0_SEC_gt_clk_ao_tcp_END (8)
#define SOC_SCTRL_SCPERCLKEN0_SEC_gt_clk_ao_tcp_32k_START (9)
#define SOC_SCTRL_SCPERCLKEN0_SEC_gt_clk_ao_tcp_32k_END (9)
#define SOC_SCTRL_SCPERCLKEN0_SEC_gt_pclk_ao_ipc_START (19)
#define SOC_SCTRL_SCPERCLKEN0_SEC_gt_pclk_ao_ipc_END (19)
#define SOC_SCTRL_SCPERCLKEN0_SEC_gt_pclkdbg_to_iomcu_START (24)
#define SOC_SCTRL_SCPERCLKEN0_SEC_gt_pclkdbg_to_iomcu_END (24)
#define SOC_SCTRL_SCPERCLKEN0_SEC_gt_clk_noc_aobus2iomcubus_START (25)
#define SOC_SCTRL_SCPERCLKEN0_SEC_gt_clk_noc_aobus2iomcubus_END (25)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ip_prst_timer1 : 1;
        unsigned int ip_prst_ao_tzpc : 1;
        unsigned int ip_prst_ao_ipc : 1;
        unsigned int reserved_0 : 1;
        unsigned int ip_prst_ao_gpio1_se : 1;
        unsigned int ip_rst_aobus : 1;
        unsigned int ip_rst_aonoc : 1;
        unsigned int ip_prst_intr_hub : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int reserved_11 : 1;
        unsigned int reserved_12 : 1;
        unsigned int reserved_13 : 1;
        unsigned int reserved_14 : 1;
        unsigned int reserved_15 : 1;
        unsigned int reserved_16 : 1;
        unsigned int reserved_17 : 1;
        unsigned int reserved_18 : 1;
        unsigned int reserved_19 : 1;
        unsigned int reserved_20 : 1;
        unsigned int reserved_21 : 1;
        unsigned int reserved_22 : 1;
        unsigned int reserved_23 : 1;
        unsigned int reserved_24 : 1;
    } reg;
} SOC_SCTRL_SCPERRSTEN0_SEC_UNION;
#endif
#define SOC_SCTRL_SCPERRSTEN0_SEC_ip_prst_timer1_START (0)
#define SOC_SCTRL_SCPERRSTEN0_SEC_ip_prst_timer1_END (0)
#define SOC_SCTRL_SCPERRSTEN0_SEC_ip_prst_ao_tzpc_START (1)
#define SOC_SCTRL_SCPERRSTEN0_SEC_ip_prst_ao_tzpc_END (1)
#define SOC_SCTRL_SCPERRSTEN0_SEC_ip_prst_ao_ipc_START (2)
#define SOC_SCTRL_SCPERRSTEN0_SEC_ip_prst_ao_ipc_END (2)
#define SOC_SCTRL_SCPERRSTEN0_SEC_ip_prst_ao_gpio1_se_START (4)
#define SOC_SCTRL_SCPERRSTEN0_SEC_ip_prst_ao_gpio1_se_END (4)
#define SOC_SCTRL_SCPERRSTEN0_SEC_ip_rst_aobus_START (5)
#define SOC_SCTRL_SCPERRSTEN0_SEC_ip_rst_aobus_END (5)
#define SOC_SCTRL_SCPERRSTEN0_SEC_ip_rst_aonoc_START (6)
#define SOC_SCTRL_SCPERRSTEN0_SEC_ip_rst_aonoc_END (6)
#define SOC_SCTRL_SCPERRSTEN0_SEC_ip_prst_intr_hub_START (7)
#define SOC_SCTRL_SCPERRSTEN0_SEC_ip_prst_intr_hub_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ip_prst_timer1 : 1;
        unsigned int ip_prst_ao_tzpc : 1;
        unsigned int ip_prst_ao_ipc : 1;
        unsigned int reserved_0 : 1;
        unsigned int ip_prst_ao_gpio1_se : 1;
        unsigned int ip_rst_aobus : 1;
        unsigned int ip_rst_aonoc : 1;
        unsigned int ip_prst_intr_hub : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int reserved_11 : 1;
        unsigned int reserved_12 : 1;
        unsigned int reserved_13 : 1;
        unsigned int reserved_14 : 1;
        unsigned int reserved_15 : 1;
        unsigned int reserved_16 : 1;
        unsigned int reserved_17 : 1;
        unsigned int reserved_18 : 1;
        unsigned int reserved_19 : 1;
        unsigned int reserved_20 : 1;
        unsigned int reserved_21 : 1;
        unsigned int reserved_22 : 1;
        unsigned int reserved_23 : 1;
        unsigned int reserved_24 : 1;
    } reg;
} SOC_SCTRL_SCPERRSTDIS0_SEC_UNION;
#endif
#define SOC_SCTRL_SCPERRSTDIS0_SEC_ip_prst_timer1_START (0)
#define SOC_SCTRL_SCPERRSTDIS0_SEC_ip_prst_timer1_END (0)
#define SOC_SCTRL_SCPERRSTDIS0_SEC_ip_prst_ao_tzpc_START (1)
#define SOC_SCTRL_SCPERRSTDIS0_SEC_ip_prst_ao_tzpc_END (1)
#define SOC_SCTRL_SCPERRSTDIS0_SEC_ip_prst_ao_ipc_START (2)
#define SOC_SCTRL_SCPERRSTDIS0_SEC_ip_prst_ao_ipc_END (2)
#define SOC_SCTRL_SCPERRSTDIS0_SEC_ip_prst_ao_gpio1_se_START (4)
#define SOC_SCTRL_SCPERRSTDIS0_SEC_ip_prst_ao_gpio1_se_END (4)
#define SOC_SCTRL_SCPERRSTDIS0_SEC_ip_rst_aobus_START (5)
#define SOC_SCTRL_SCPERRSTDIS0_SEC_ip_rst_aobus_END (5)
#define SOC_SCTRL_SCPERRSTDIS0_SEC_ip_rst_aonoc_START (6)
#define SOC_SCTRL_SCPERRSTDIS0_SEC_ip_rst_aonoc_END (6)
#define SOC_SCTRL_SCPERRSTDIS0_SEC_ip_prst_intr_hub_START (7)
#define SOC_SCTRL_SCPERRSTDIS0_SEC_ip_prst_intr_hub_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ip_prst_timer1 : 1;
        unsigned int ip_prst_ao_tzpc : 1;
        unsigned int ip_prst_ao_ipc : 1;
        unsigned int reserved_0 : 1;
        unsigned int ip_prst_ao_gpio1_se : 1;
        unsigned int ip_rst_aobus : 1;
        unsigned int ip_rst_aonoc : 1;
        unsigned int ip_prst_intr_hub : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int reserved_11 : 1;
        unsigned int reserved_12 : 1;
        unsigned int reserved_13 : 1;
        unsigned int reserved_14 : 1;
        unsigned int reserved_15 : 1;
        unsigned int reserved_16 : 1;
        unsigned int reserved_17 : 1;
        unsigned int reserved_18 : 1;
        unsigned int reserved_19 : 1;
        unsigned int reserved_20 : 1;
        unsigned int reserved_21 : 1;
        unsigned int reserved_22 : 1;
        unsigned int reserved_23 : 1;
        unsigned int reserved_24 : 1;
    } reg;
} SOC_SCTRL_SCPERRSTSTAT0_SEC_UNION;
#endif
#define SOC_SCTRL_SCPERRSTSTAT0_SEC_ip_prst_timer1_START (0)
#define SOC_SCTRL_SCPERRSTSTAT0_SEC_ip_prst_timer1_END (0)
#define SOC_SCTRL_SCPERRSTSTAT0_SEC_ip_prst_ao_tzpc_START (1)
#define SOC_SCTRL_SCPERRSTSTAT0_SEC_ip_prst_ao_tzpc_END (1)
#define SOC_SCTRL_SCPERRSTSTAT0_SEC_ip_prst_ao_ipc_START (2)
#define SOC_SCTRL_SCPERRSTSTAT0_SEC_ip_prst_ao_ipc_END (2)
#define SOC_SCTRL_SCPERRSTSTAT0_SEC_ip_prst_ao_gpio1_se_START (4)
#define SOC_SCTRL_SCPERRSTSTAT0_SEC_ip_prst_ao_gpio1_se_END (4)
#define SOC_SCTRL_SCPERRSTSTAT0_SEC_ip_rst_aobus_START (5)
#define SOC_SCTRL_SCPERRSTSTAT0_SEC_ip_rst_aobus_END (5)
#define SOC_SCTRL_SCPERRSTSTAT0_SEC_ip_rst_aonoc_START (6)
#define SOC_SCTRL_SCPERRSTSTAT0_SEC_ip_rst_aonoc_END (6)
#define SOC_SCTRL_SCPERRSTSTAT0_SEC_ip_prst_intr_hub_START (7)
#define SOC_SCTRL_SCPERRSTSTAT0_SEC_ip_prst_intr_hub_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ip_rst_asp_subsys : 1;
        unsigned int ip_rst_asp_subsys_crg : 1;
        unsigned int ip_prst_hickm : 1;
        unsigned int ip_rst_sci0 : 1;
        unsigned int ip_rst_sci1 : 1;
        unsigned int ip_prst_timer2 : 1;
        unsigned int ip_prst_timer3 : 1;
        unsigned int ip_prst_timer7 : 1;
        unsigned int ip_prst_rtc1 : 1;
        unsigned int ip_rst_asp_cfg : 1;
        unsigned int ip_rst_mad : 1;
        unsigned int reserved_0 : 1;
        unsigned int ip_rst_hisee : 1;
        unsigned int reserved_1 : 1;
        unsigned int ip_rst_hieps_tee_crg : 1;
        unsigned int ip_rst_hieps_tee : 1;
        unsigned int ip_rst_pll_fsm : 1;
        unsigned int ip_rst_qic_hieps_tee : 1;
        unsigned int ip_rst_ao_tcp : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int reserved_11 : 1;
        unsigned int reserved_12 : 1;
        unsigned int reserved_13 : 1;
        unsigned int reserved_14 : 1;
    } reg;
} SOC_SCTRL_SCPERRSTEN1_SEC_UNION;
#endif
#define SOC_SCTRL_SCPERRSTEN1_SEC_ip_rst_asp_subsys_START (0)
#define SOC_SCTRL_SCPERRSTEN1_SEC_ip_rst_asp_subsys_END (0)
#define SOC_SCTRL_SCPERRSTEN1_SEC_ip_rst_asp_subsys_crg_START (1)
#define SOC_SCTRL_SCPERRSTEN1_SEC_ip_rst_asp_subsys_crg_END (1)
#define SOC_SCTRL_SCPERRSTEN1_SEC_ip_prst_hickm_START (2)
#define SOC_SCTRL_SCPERRSTEN1_SEC_ip_prst_hickm_END (2)
#define SOC_SCTRL_SCPERRSTEN1_SEC_ip_rst_sci0_START (3)
#define SOC_SCTRL_SCPERRSTEN1_SEC_ip_rst_sci0_END (3)
#define SOC_SCTRL_SCPERRSTEN1_SEC_ip_rst_sci1_START (4)
#define SOC_SCTRL_SCPERRSTEN1_SEC_ip_rst_sci1_END (4)
#define SOC_SCTRL_SCPERRSTEN1_SEC_ip_prst_timer2_START (5)
#define SOC_SCTRL_SCPERRSTEN1_SEC_ip_prst_timer2_END (5)
#define SOC_SCTRL_SCPERRSTEN1_SEC_ip_prst_timer3_START (6)
#define SOC_SCTRL_SCPERRSTEN1_SEC_ip_prst_timer3_END (6)
#define SOC_SCTRL_SCPERRSTEN1_SEC_ip_prst_timer7_START (7)
#define SOC_SCTRL_SCPERRSTEN1_SEC_ip_prst_timer7_END (7)
#define SOC_SCTRL_SCPERRSTEN1_SEC_ip_prst_rtc1_START (8)
#define SOC_SCTRL_SCPERRSTEN1_SEC_ip_prst_rtc1_END (8)
#define SOC_SCTRL_SCPERRSTEN1_SEC_ip_rst_asp_cfg_START (9)
#define SOC_SCTRL_SCPERRSTEN1_SEC_ip_rst_asp_cfg_END (9)
#define SOC_SCTRL_SCPERRSTEN1_SEC_ip_rst_mad_START (10)
#define SOC_SCTRL_SCPERRSTEN1_SEC_ip_rst_mad_END (10)
#define SOC_SCTRL_SCPERRSTEN1_SEC_ip_rst_hisee_START (12)
#define SOC_SCTRL_SCPERRSTEN1_SEC_ip_rst_hisee_END (12)
#define SOC_SCTRL_SCPERRSTEN1_SEC_ip_rst_hieps_tee_crg_START (14)
#define SOC_SCTRL_SCPERRSTEN1_SEC_ip_rst_hieps_tee_crg_END (14)
#define SOC_SCTRL_SCPERRSTEN1_SEC_ip_rst_hieps_tee_START (15)
#define SOC_SCTRL_SCPERRSTEN1_SEC_ip_rst_hieps_tee_END (15)
#define SOC_SCTRL_SCPERRSTEN1_SEC_ip_rst_pll_fsm_START (16)
#define SOC_SCTRL_SCPERRSTEN1_SEC_ip_rst_pll_fsm_END (16)
#define SOC_SCTRL_SCPERRSTEN1_SEC_ip_rst_qic_hieps_tee_START (17)
#define SOC_SCTRL_SCPERRSTEN1_SEC_ip_rst_qic_hieps_tee_END (17)
#define SOC_SCTRL_SCPERRSTEN1_SEC_ip_rst_ao_tcp_START (18)
#define SOC_SCTRL_SCPERRSTEN1_SEC_ip_rst_ao_tcp_END (18)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ip_rst_asp_subsys : 1;
        unsigned int ip_rst_asp_subsys_crg : 1;
        unsigned int ip_prst_hickm : 1;
        unsigned int ip_rst_sci0 : 1;
        unsigned int ip_rst_sci1 : 1;
        unsigned int ip_prst_timer2 : 1;
        unsigned int ip_prst_timer3 : 1;
        unsigned int ip_prst_timer7 : 1;
        unsigned int ip_prst_rtc1 : 1;
        unsigned int ip_rst_asp_cfg : 1;
        unsigned int ip_rst_mad : 1;
        unsigned int reserved_0 : 1;
        unsigned int ip_rst_hisee : 1;
        unsigned int reserved_1 : 1;
        unsigned int ip_rst_hieps_tee_crg : 1;
        unsigned int ip_rst_hieps_tee : 1;
        unsigned int ip_rst_pll_fsm : 1;
        unsigned int ip_rst_qic_hieps_tee : 1;
        unsigned int ip_rst_ao_tcp : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int reserved_11 : 1;
        unsigned int reserved_12 : 1;
        unsigned int reserved_13 : 1;
        unsigned int reserved_14 : 1;
    } reg;
} SOC_SCTRL_SCPERRSTDIS1_SEC_UNION;
#endif
#define SOC_SCTRL_SCPERRSTDIS1_SEC_ip_rst_asp_subsys_START (0)
#define SOC_SCTRL_SCPERRSTDIS1_SEC_ip_rst_asp_subsys_END (0)
#define SOC_SCTRL_SCPERRSTDIS1_SEC_ip_rst_asp_subsys_crg_START (1)
#define SOC_SCTRL_SCPERRSTDIS1_SEC_ip_rst_asp_subsys_crg_END (1)
#define SOC_SCTRL_SCPERRSTDIS1_SEC_ip_prst_hickm_START (2)
#define SOC_SCTRL_SCPERRSTDIS1_SEC_ip_prst_hickm_END (2)
#define SOC_SCTRL_SCPERRSTDIS1_SEC_ip_rst_sci0_START (3)
#define SOC_SCTRL_SCPERRSTDIS1_SEC_ip_rst_sci0_END (3)
#define SOC_SCTRL_SCPERRSTDIS1_SEC_ip_rst_sci1_START (4)
#define SOC_SCTRL_SCPERRSTDIS1_SEC_ip_rst_sci1_END (4)
#define SOC_SCTRL_SCPERRSTDIS1_SEC_ip_prst_timer2_START (5)
#define SOC_SCTRL_SCPERRSTDIS1_SEC_ip_prst_timer2_END (5)
#define SOC_SCTRL_SCPERRSTDIS1_SEC_ip_prst_timer3_START (6)
#define SOC_SCTRL_SCPERRSTDIS1_SEC_ip_prst_timer3_END (6)
#define SOC_SCTRL_SCPERRSTDIS1_SEC_ip_prst_timer7_START (7)
#define SOC_SCTRL_SCPERRSTDIS1_SEC_ip_prst_timer7_END (7)
#define SOC_SCTRL_SCPERRSTDIS1_SEC_ip_prst_rtc1_START (8)
#define SOC_SCTRL_SCPERRSTDIS1_SEC_ip_prst_rtc1_END (8)
#define SOC_SCTRL_SCPERRSTDIS1_SEC_ip_rst_asp_cfg_START (9)
#define SOC_SCTRL_SCPERRSTDIS1_SEC_ip_rst_asp_cfg_END (9)
#define SOC_SCTRL_SCPERRSTDIS1_SEC_ip_rst_mad_START (10)
#define SOC_SCTRL_SCPERRSTDIS1_SEC_ip_rst_mad_END (10)
#define SOC_SCTRL_SCPERRSTDIS1_SEC_ip_rst_hisee_START (12)
#define SOC_SCTRL_SCPERRSTDIS1_SEC_ip_rst_hisee_END (12)
#define SOC_SCTRL_SCPERRSTDIS1_SEC_ip_rst_hieps_tee_crg_START (14)
#define SOC_SCTRL_SCPERRSTDIS1_SEC_ip_rst_hieps_tee_crg_END (14)
#define SOC_SCTRL_SCPERRSTDIS1_SEC_ip_rst_hieps_tee_START (15)
#define SOC_SCTRL_SCPERRSTDIS1_SEC_ip_rst_hieps_tee_END (15)
#define SOC_SCTRL_SCPERRSTDIS1_SEC_ip_rst_pll_fsm_START (16)
#define SOC_SCTRL_SCPERRSTDIS1_SEC_ip_rst_pll_fsm_END (16)
#define SOC_SCTRL_SCPERRSTDIS1_SEC_ip_rst_qic_hieps_tee_START (17)
#define SOC_SCTRL_SCPERRSTDIS1_SEC_ip_rst_qic_hieps_tee_END (17)
#define SOC_SCTRL_SCPERRSTDIS1_SEC_ip_rst_ao_tcp_START (18)
#define SOC_SCTRL_SCPERRSTDIS1_SEC_ip_rst_ao_tcp_END (18)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ip_rst_asp_subsys : 1;
        unsigned int ip_rst_asp_subsys_crg : 1;
        unsigned int ip_prst_hickm : 1;
        unsigned int ip_rst_sci0 : 1;
        unsigned int ip_rst_sci1 : 1;
        unsigned int ip_prst_timer2 : 1;
        unsigned int ip_prst_timer3 : 1;
        unsigned int ip_prst_timer7 : 1;
        unsigned int ip_prst_rtc1 : 1;
        unsigned int ip_rst_asp_cfg : 1;
        unsigned int ip_rst_mad : 1;
        unsigned int reserved_0 : 1;
        unsigned int ip_rst_hisee : 1;
        unsigned int reserved_1 : 1;
        unsigned int ip_rst_hieps_tee_crg : 1;
        unsigned int ip_rst_hieps_tee : 1;
        unsigned int ip_rst_pll_fsm : 1;
        unsigned int ip_rst_qic_hieps_tee : 1;
        unsigned int ip_rst_ao_tcp : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int reserved_11 : 1;
        unsigned int reserved_12 : 1;
        unsigned int reserved_13 : 1;
        unsigned int reserved_14 : 1;
    } reg;
} SOC_SCTRL_SCPERRSTSTAT1_SEC_UNION;
#endif
#define SOC_SCTRL_SCPERRSTSTAT1_SEC_ip_rst_asp_subsys_START (0)
#define SOC_SCTRL_SCPERRSTSTAT1_SEC_ip_rst_asp_subsys_END (0)
#define SOC_SCTRL_SCPERRSTSTAT1_SEC_ip_rst_asp_subsys_crg_START (1)
#define SOC_SCTRL_SCPERRSTSTAT1_SEC_ip_rst_asp_subsys_crg_END (1)
#define SOC_SCTRL_SCPERRSTSTAT1_SEC_ip_prst_hickm_START (2)
#define SOC_SCTRL_SCPERRSTSTAT1_SEC_ip_prst_hickm_END (2)
#define SOC_SCTRL_SCPERRSTSTAT1_SEC_ip_rst_sci0_START (3)
#define SOC_SCTRL_SCPERRSTSTAT1_SEC_ip_rst_sci0_END (3)
#define SOC_SCTRL_SCPERRSTSTAT1_SEC_ip_rst_sci1_START (4)
#define SOC_SCTRL_SCPERRSTSTAT1_SEC_ip_rst_sci1_END (4)
#define SOC_SCTRL_SCPERRSTSTAT1_SEC_ip_prst_timer2_START (5)
#define SOC_SCTRL_SCPERRSTSTAT1_SEC_ip_prst_timer2_END (5)
#define SOC_SCTRL_SCPERRSTSTAT1_SEC_ip_prst_timer3_START (6)
#define SOC_SCTRL_SCPERRSTSTAT1_SEC_ip_prst_timer3_END (6)
#define SOC_SCTRL_SCPERRSTSTAT1_SEC_ip_prst_timer7_START (7)
#define SOC_SCTRL_SCPERRSTSTAT1_SEC_ip_prst_timer7_END (7)
#define SOC_SCTRL_SCPERRSTSTAT1_SEC_ip_prst_rtc1_START (8)
#define SOC_SCTRL_SCPERRSTSTAT1_SEC_ip_prst_rtc1_END (8)
#define SOC_SCTRL_SCPERRSTSTAT1_SEC_ip_rst_asp_cfg_START (9)
#define SOC_SCTRL_SCPERRSTSTAT1_SEC_ip_rst_asp_cfg_END (9)
#define SOC_SCTRL_SCPERRSTSTAT1_SEC_ip_rst_mad_START (10)
#define SOC_SCTRL_SCPERRSTSTAT1_SEC_ip_rst_mad_END (10)
#define SOC_SCTRL_SCPERRSTSTAT1_SEC_ip_rst_hisee_START (12)
#define SOC_SCTRL_SCPERRSTSTAT1_SEC_ip_rst_hisee_END (12)
#define SOC_SCTRL_SCPERRSTSTAT1_SEC_ip_rst_hieps_tee_crg_START (14)
#define SOC_SCTRL_SCPERRSTSTAT1_SEC_ip_rst_hieps_tee_crg_END (14)
#define SOC_SCTRL_SCPERRSTSTAT1_SEC_ip_rst_hieps_tee_START (15)
#define SOC_SCTRL_SCPERRSTSTAT1_SEC_ip_rst_hieps_tee_END (15)
#define SOC_SCTRL_SCPERRSTSTAT1_SEC_ip_rst_pll_fsm_START (16)
#define SOC_SCTRL_SCPERRSTSTAT1_SEC_ip_rst_pll_fsm_END (16)
#define SOC_SCTRL_SCPERRSTSTAT1_SEC_ip_rst_qic_hieps_tee_START (17)
#define SOC_SCTRL_SCPERRSTSTAT1_SEC_ip_rst_qic_hieps_tee_END (17)
#define SOC_SCTRL_SCPERRSTSTAT1_SEC_ip_rst_ao_tcp_START (18)
#define SOC_SCTRL_SCPERRSTSTAT1_SEC_ip_rst_ao_tcp_END (18)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ip_rst_iomcu : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_SCTRL_SCPERRSTEN2_SEC_UNION;
#endif
#define SOC_SCTRL_SCPERRSTEN2_SEC_ip_rst_iomcu_START (0)
#define SOC_SCTRL_SCPERRSTEN2_SEC_ip_rst_iomcu_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ip_rst_iomcu : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_SCTRL_SCPERRSTDIS2_SEC_UNION;
#endif
#define SOC_SCTRL_SCPERRSTDIS2_SEC_ip_rst_iomcu_START (0)
#define SOC_SCTRL_SCPERRSTDIS2_SEC_ip_rst_iomcu_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ip_rst_iomcu : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_SCTRL_SCPERRSTSTAT2_SEC_UNION;
#endif
#define SOC_SCTRL_SCPERRSTSTAT2_SEC_ip_rst_iomcu_START (0)
#define SOC_SCTRL_SCPERRSTSTAT2_SEC_ip_rst_iomcu_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0: 1;
        unsigned int reserved_1: 1;
        unsigned int reserved_2: 30;
    } reg;
} SOC_SCTRL_SCPERRSTEN3_SEC_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0: 1;
        unsigned int reserved_1: 1;
        unsigned int reserved_2: 30;
    } reg;
} SOC_SCTRL_SCPERRSTDIS3_SEC_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0: 1;
        unsigned int reserved_1: 1;
        unsigned int reserved_2: 30;
    } reg;
} SOC_SCTRL_SCPERRSTSTAT3_SEC_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int gt_pclk_hickm : 1;
        unsigned int gt_clk_sci0 : 1;
        unsigned int gt_clk_sci1 : 1;
        unsigned int gt_pclk_timer2 : 1;
        unsigned int reserved_0 : 1;
        unsigned int gt_pclk_timer3 : 1;
        unsigned int reserved_1 : 1;
        unsigned int gt_pclk_timer7 : 1;
        unsigned int reserved_2 : 1;
        unsigned int gt_clk_ref_crc : 1;
        unsigned int gt_pclk_rtc1 : 1;
        unsigned int gt_pclk_ao_wd1 : 1;
        unsigned int gt_pclk_ao_wd : 1;
        unsigned int gt_clk_asp_subsys_lpmcu : 1;
        unsigned int gt_clk_asp_codec_lpm3 : 1;
        unsigned int gt_clk_mfcc : 1;
        unsigned int gt_clk_mad_lpm3 : 1;
        unsigned int gt_clk_qcr_aspbus_lpmcu : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int gt_clk_hsdt_subsys : 6;
        unsigned int gt_atclk_to_iomcu : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
    } reg;
} SOC_SCTRL_SCPEREN3_SEC_UNION;
#endif
#define SOC_SCTRL_SCPEREN3_SEC_gt_pclk_hickm_START (0)
#define SOC_SCTRL_SCPEREN3_SEC_gt_pclk_hickm_END (0)
#define SOC_SCTRL_SCPEREN3_SEC_gt_clk_sci0_START (1)
#define SOC_SCTRL_SCPEREN3_SEC_gt_clk_sci0_END (1)
#define SOC_SCTRL_SCPEREN3_SEC_gt_clk_sci1_START (2)
#define SOC_SCTRL_SCPEREN3_SEC_gt_clk_sci1_END (2)
#define SOC_SCTRL_SCPEREN3_SEC_gt_pclk_timer2_START (3)
#define SOC_SCTRL_SCPEREN3_SEC_gt_pclk_timer2_END (3)
#define SOC_SCTRL_SCPEREN3_SEC_gt_pclk_timer3_START (5)
#define SOC_SCTRL_SCPEREN3_SEC_gt_pclk_timer3_END (5)
#define SOC_SCTRL_SCPEREN3_SEC_gt_pclk_timer7_START (7)
#define SOC_SCTRL_SCPEREN3_SEC_gt_pclk_timer7_END (7)
#define SOC_SCTRL_SCPEREN3_SEC_gt_clk_ref_crc_START (9)
#define SOC_SCTRL_SCPEREN3_SEC_gt_clk_ref_crc_END (9)
#define SOC_SCTRL_SCPEREN3_SEC_gt_pclk_rtc1_START (10)
#define SOC_SCTRL_SCPEREN3_SEC_gt_pclk_rtc1_END (10)
#define SOC_SCTRL_SCPEREN3_SEC_gt_pclk_ao_wd1_START (11)
#define SOC_SCTRL_SCPEREN3_SEC_gt_pclk_ao_wd1_END (11)
#define SOC_SCTRL_SCPEREN3_SEC_gt_pclk_ao_wd_START (12)
#define SOC_SCTRL_SCPEREN3_SEC_gt_pclk_ao_wd_END (12)
#define SOC_SCTRL_SCPEREN3_SEC_gt_clk_asp_subsys_lpmcu_START (13)
#define SOC_SCTRL_SCPEREN3_SEC_gt_clk_asp_subsys_lpmcu_END (13)
#define SOC_SCTRL_SCPEREN3_SEC_gt_clk_asp_codec_lpm3_START (14)
#define SOC_SCTRL_SCPEREN3_SEC_gt_clk_asp_codec_lpm3_END (14)
#define SOC_SCTRL_SCPEREN3_SEC_gt_clk_mfcc_START (15)
#define SOC_SCTRL_SCPEREN3_SEC_gt_clk_mfcc_END (15)
#define SOC_SCTRL_SCPEREN3_SEC_gt_clk_mad_lpm3_START (16)
#define SOC_SCTRL_SCPEREN3_SEC_gt_clk_mad_lpm3_END (16)
#define SOC_SCTRL_SCPEREN3_SEC_gt_clk_qcr_aspbus_lpmcu_START (17)
#define SOC_SCTRL_SCPEREN3_SEC_gt_clk_qcr_aspbus_lpmcu_END (17)
#define SOC_SCTRL_SCPEREN3_SEC_gt_clk_hsdt_subsys_START (21)
#define SOC_SCTRL_SCPEREN3_SEC_gt_clk_hsdt_subsys_END (26)
#define SOC_SCTRL_SCPEREN3_SEC_gt_atclk_to_iomcu_START (27)
#define SOC_SCTRL_SCPEREN3_SEC_gt_atclk_to_iomcu_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int gt_pclk_hickm : 1;
        unsigned int gt_clk_sci0 : 1;
        unsigned int gt_clk_sci1 : 1;
        unsigned int gt_pclk_timer2 : 1;
        unsigned int reserved_0 : 1;
        unsigned int gt_pclk_timer3 : 1;
        unsigned int reserved_1 : 1;
        unsigned int gt_pclk_timer7 : 1;
        unsigned int reserved_2 : 1;
        unsigned int gt_clk_ref_crc : 1;
        unsigned int gt_pclk_rtc1 : 1;
        unsigned int gt_pclk_ao_wd1 : 1;
        unsigned int gt_pclk_ao_wd : 1;
        unsigned int gt_clk_asp_subsys_lpmcu : 1;
        unsigned int gt_clk_asp_codec_lpm3 : 1;
        unsigned int gt_clk_mfcc : 1;
        unsigned int gt_clk_mad_lpm3 : 1;
        unsigned int gt_clk_qcr_aspbus_lpmcu : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int gt_clk_hsdt_subsys : 6;
        unsigned int gt_atclk_to_iomcu : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
    } reg;
} SOC_SCTRL_SCPERDIS3_SEC_UNION;
#endif
#define SOC_SCTRL_SCPERDIS3_SEC_gt_pclk_hickm_START (0)
#define SOC_SCTRL_SCPERDIS3_SEC_gt_pclk_hickm_END (0)
#define SOC_SCTRL_SCPERDIS3_SEC_gt_clk_sci0_START (1)
#define SOC_SCTRL_SCPERDIS3_SEC_gt_clk_sci0_END (1)
#define SOC_SCTRL_SCPERDIS3_SEC_gt_clk_sci1_START (2)
#define SOC_SCTRL_SCPERDIS3_SEC_gt_clk_sci1_END (2)
#define SOC_SCTRL_SCPERDIS3_SEC_gt_pclk_timer2_START (3)
#define SOC_SCTRL_SCPERDIS3_SEC_gt_pclk_timer2_END (3)
#define SOC_SCTRL_SCPERDIS3_SEC_gt_pclk_timer3_START (5)
#define SOC_SCTRL_SCPERDIS3_SEC_gt_pclk_timer3_END (5)
#define SOC_SCTRL_SCPERDIS3_SEC_gt_pclk_timer7_START (7)
#define SOC_SCTRL_SCPERDIS3_SEC_gt_pclk_timer7_END (7)
#define SOC_SCTRL_SCPERDIS3_SEC_gt_clk_ref_crc_START (9)
#define SOC_SCTRL_SCPERDIS3_SEC_gt_clk_ref_crc_END (9)
#define SOC_SCTRL_SCPERDIS3_SEC_gt_pclk_rtc1_START (10)
#define SOC_SCTRL_SCPERDIS3_SEC_gt_pclk_rtc1_END (10)
#define SOC_SCTRL_SCPERDIS3_SEC_gt_pclk_ao_wd1_START (11)
#define SOC_SCTRL_SCPERDIS3_SEC_gt_pclk_ao_wd1_END (11)
#define SOC_SCTRL_SCPERDIS3_SEC_gt_pclk_ao_wd_START (12)
#define SOC_SCTRL_SCPERDIS3_SEC_gt_pclk_ao_wd_END (12)
#define SOC_SCTRL_SCPERDIS3_SEC_gt_clk_asp_subsys_lpmcu_START (13)
#define SOC_SCTRL_SCPERDIS3_SEC_gt_clk_asp_subsys_lpmcu_END (13)
#define SOC_SCTRL_SCPERDIS3_SEC_gt_clk_asp_codec_lpm3_START (14)
#define SOC_SCTRL_SCPERDIS3_SEC_gt_clk_asp_codec_lpm3_END (14)
#define SOC_SCTRL_SCPERDIS3_SEC_gt_clk_mfcc_START (15)
#define SOC_SCTRL_SCPERDIS3_SEC_gt_clk_mfcc_END (15)
#define SOC_SCTRL_SCPERDIS3_SEC_gt_clk_mad_lpm3_START (16)
#define SOC_SCTRL_SCPERDIS3_SEC_gt_clk_mad_lpm3_END (16)
#define SOC_SCTRL_SCPERDIS3_SEC_gt_clk_qcr_aspbus_lpmcu_START (17)
#define SOC_SCTRL_SCPERDIS3_SEC_gt_clk_qcr_aspbus_lpmcu_END (17)
#define SOC_SCTRL_SCPERDIS3_SEC_gt_clk_hsdt_subsys_START (21)
#define SOC_SCTRL_SCPERDIS3_SEC_gt_clk_hsdt_subsys_END (26)
#define SOC_SCTRL_SCPERDIS3_SEC_gt_atclk_to_iomcu_START (27)
#define SOC_SCTRL_SCPERDIS3_SEC_gt_atclk_to_iomcu_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int gt_pclk_hickm : 1;
        unsigned int gt_clk_sci0 : 1;
        unsigned int gt_clk_sci1 : 1;
        unsigned int gt_pclk_timer2 : 1;
        unsigned int reserved_0 : 1;
        unsigned int gt_pclk_timer3 : 1;
        unsigned int reserved_1 : 1;
        unsigned int gt_pclk_timer7 : 1;
        unsigned int reserved_2 : 1;
        unsigned int gt_clk_ref_crc : 1;
        unsigned int gt_pclk_rtc1 : 1;
        unsigned int gt_pclk_ao_wd1 : 1;
        unsigned int gt_pclk_ao_wd : 1;
        unsigned int gt_clk_asp_subsys_lpmcu : 1;
        unsigned int gt_clk_asp_codec_lpm3 : 1;
        unsigned int gt_clk_mfcc : 1;
        unsigned int gt_clk_mad_lpm3 : 1;
        unsigned int gt_clk_qcr_aspbus_lpmcu : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int gt_clk_hsdt_subsys : 6;
        unsigned int gt_atclk_to_iomcu : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
    } reg;
} SOC_SCTRL_SCPERCLKEN3_SEC_UNION;
#endif
#define SOC_SCTRL_SCPERCLKEN3_SEC_gt_pclk_hickm_START (0)
#define SOC_SCTRL_SCPERCLKEN3_SEC_gt_pclk_hickm_END (0)
#define SOC_SCTRL_SCPERCLKEN3_SEC_gt_clk_sci0_START (1)
#define SOC_SCTRL_SCPERCLKEN3_SEC_gt_clk_sci0_END (1)
#define SOC_SCTRL_SCPERCLKEN3_SEC_gt_clk_sci1_START (2)
#define SOC_SCTRL_SCPERCLKEN3_SEC_gt_clk_sci1_END (2)
#define SOC_SCTRL_SCPERCLKEN3_SEC_gt_pclk_timer2_START (3)
#define SOC_SCTRL_SCPERCLKEN3_SEC_gt_pclk_timer2_END (3)
#define SOC_SCTRL_SCPERCLKEN3_SEC_gt_pclk_timer3_START (5)
#define SOC_SCTRL_SCPERCLKEN3_SEC_gt_pclk_timer3_END (5)
#define SOC_SCTRL_SCPERCLKEN3_SEC_gt_pclk_timer7_START (7)
#define SOC_SCTRL_SCPERCLKEN3_SEC_gt_pclk_timer7_END (7)
#define SOC_SCTRL_SCPERCLKEN3_SEC_gt_clk_ref_crc_START (9)
#define SOC_SCTRL_SCPERCLKEN3_SEC_gt_clk_ref_crc_END (9)
#define SOC_SCTRL_SCPERCLKEN3_SEC_gt_pclk_rtc1_START (10)
#define SOC_SCTRL_SCPERCLKEN3_SEC_gt_pclk_rtc1_END (10)
#define SOC_SCTRL_SCPERCLKEN3_SEC_gt_pclk_ao_wd1_START (11)
#define SOC_SCTRL_SCPERCLKEN3_SEC_gt_pclk_ao_wd1_END (11)
#define SOC_SCTRL_SCPERCLKEN3_SEC_gt_pclk_ao_wd_START (12)
#define SOC_SCTRL_SCPERCLKEN3_SEC_gt_pclk_ao_wd_END (12)
#define SOC_SCTRL_SCPERCLKEN3_SEC_gt_clk_asp_subsys_lpmcu_START (13)
#define SOC_SCTRL_SCPERCLKEN3_SEC_gt_clk_asp_subsys_lpmcu_END (13)
#define SOC_SCTRL_SCPERCLKEN3_SEC_gt_clk_asp_codec_lpm3_START (14)
#define SOC_SCTRL_SCPERCLKEN3_SEC_gt_clk_asp_codec_lpm3_END (14)
#define SOC_SCTRL_SCPERCLKEN3_SEC_gt_clk_mfcc_START (15)
#define SOC_SCTRL_SCPERCLKEN3_SEC_gt_clk_mfcc_END (15)
#define SOC_SCTRL_SCPERCLKEN3_SEC_gt_clk_mad_lpm3_START (16)
#define SOC_SCTRL_SCPERCLKEN3_SEC_gt_clk_mad_lpm3_END (16)
#define SOC_SCTRL_SCPERCLKEN3_SEC_gt_clk_qcr_aspbus_lpmcu_START (17)
#define SOC_SCTRL_SCPERCLKEN3_SEC_gt_clk_qcr_aspbus_lpmcu_END (17)
#define SOC_SCTRL_SCPERCLKEN3_SEC_gt_clk_hsdt_subsys_START (21)
#define SOC_SCTRL_SCPERCLKEN3_SEC_gt_clk_hsdt_subsys_END (26)
#define SOC_SCTRL_SCPERCLKEN3_SEC_gt_atclk_to_iomcu_START (27)
#define SOC_SCTRL_SCPERCLKEN3_SEC_gt_atclk_to_iomcu_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int gt_clk_hieps_tee : 1;
        unsigned int reserved_3 : 1;
        unsigned int gt_clk_hieps_tcxo : 1;
        unsigned int reserved_4 : 1;
        unsigned int gt_clk_hisee_pll : 1;
        unsigned int gt_clk_qcr_hisee : 1;
        unsigned int reserved_5 : 1;
        unsigned int gt_clk_hisee_ref : 1;
        unsigned int reserved_6 : 1;
        unsigned int gt_clk_spll_sscg : 1;
        unsigned int gt_clk_aobus : 1;
        unsigned int gt_clk_aonoc2mspcbus : 1;
        unsigned int gt_clk_noc_aobus2sysbusddrc : 1;
        unsigned int gt_clk_autodiv_aonoc_pll : 1;
        unsigned int gt_clk_autodiv_aonoc_fll : 1;
        unsigned int gt_timerclk_refh : 1;
        unsigned int gt_clk_pll_fsm : 1;
        unsigned int gt_clk_fll_src_peri : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int reserved_11 : 1;
        unsigned int reserved_12 : 1;
        unsigned int reserved_13 : 1;
        unsigned int reserved_14 : 1;
        unsigned int reserved_15 : 1;
        unsigned int reserved_16 : 1;
        unsigned int reserved_17 : 1;
    } reg;
} SOC_SCTRL_SCPEREN4_SEC_UNION;
#endif
#define SOC_SCTRL_SCPEREN4_SEC_gt_clk_hieps_tee_START (3)
#define SOC_SCTRL_SCPEREN4_SEC_gt_clk_hieps_tee_END (3)
#define SOC_SCTRL_SCPEREN4_SEC_gt_clk_hieps_tcxo_START (5)
#define SOC_SCTRL_SCPEREN4_SEC_gt_clk_hieps_tcxo_END (5)
#define SOC_SCTRL_SCPEREN4_SEC_gt_clk_hisee_pll_START (7)
#define SOC_SCTRL_SCPEREN4_SEC_gt_clk_hisee_pll_END (7)
#define SOC_SCTRL_SCPEREN4_SEC_gt_clk_qcr_hisee_START (8)
#define SOC_SCTRL_SCPEREN4_SEC_gt_clk_qcr_hisee_END (8)
#define SOC_SCTRL_SCPEREN4_SEC_gt_clk_hisee_ref_START (10)
#define SOC_SCTRL_SCPEREN4_SEC_gt_clk_hisee_ref_END (10)
#define SOC_SCTRL_SCPEREN4_SEC_gt_clk_spll_sscg_START (12)
#define SOC_SCTRL_SCPEREN4_SEC_gt_clk_spll_sscg_END (12)
#define SOC_SCTRL_SCPEREN4_SEC_gt_clk_aobus_START (13)
#define SOC_SCTRL_SCPEREN4_SEC_gt_clk_aobus_END (13)
#define SOC_SCTRL_SCPEREN4_SEC_gt_clk_aonoc2mspcbus_START (14)
#define SOC_SCTRL_SCPEREN4_SEC_gt_clk_aonoc2mspcbus_END (14)
#define SOC_SCTRL_SCPEREN4_SEC_gt_clk_noc_aobus2sysbusddrc_START (15)
#define SOC_SCTRL_SCPEREN4_SEC_gt_clk_noc_aobus2sysbusddrc_END (15)
#define SOC_SCTRL_SCPEREN4_SEC_gt_clk_autodiv_aonoc_pll_START (16)
#define SOC_SCTRL_SCPEREN4_SEC_gt_clk_autodiv_aonoc_pll_END (16)
#define SOC_SCTRL_SCPEREN4_SEC_gt_clk_autodiv_aonoc_fll_START (17)
#define SOC_SCTRL_SCPEREN4_SEC_gt_clk_autodiv_aonoc_fll_END (17)
#define SOC_SCTRL_SCPEREN4_SEC_gt_timerclk_refh_START (18)
#define SOC_SCTRL_SCPEREN4_SEC_gt_timerclk_refh_END (18)
#define SOC_SCTRL_SCPEREN4_SEC_gt_clk_pll_fsm_START (19)
#define SOC_SCTRL_SCPEREN4_SEC_gt_clk_pll_fsm_END (19)
#define SOC_SCTRL_SCPEREN4_SEC_gt_clk_fll_src_peri_START (20)
#define SOC_SCTRL_SCPEREN4_SEC_gt_clk_fll_src_peri_END (20)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int gt_clk_hieps_tee : 1;
        unsigned int reserved_3 : 1;
        unsigned int gt_clk_hieps_tcxo : 1;
        unsigned int reserved_4 : 1;
        unsigned int gt_clk_hisee_pll : 1;
        unsigned int gt_clk_qcr_hisee : 1;
        unsigned int reserved_5 : 1;
        unsigned int gt_clk_hisee_ref : 1;
        unsigned int reserved_6 : 1;
        unsigned int gt_clk_spll_sscg : 1;
        unsigned int gt_clk_aobus : 1;
        unsigned int gt_clk_aonoc2mspcbus : 1;
        unsigned int gt_clk_noc_aobus2sysbusddrc : 1;
        unsigned int gt_clk_autodiv_aonoc_pll : 1;
        unsigned int gt_clk_autodiv_aonoc_fll : 1;
        unsigned int gt_timerclk_refh : 1;
        unsigned int gt_clk_pll_fsm : 1;
        unsigned int gt_clk_fll_src_peri : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int reserved_11 : 1;
        unsigned int reserved_12 : 1;
        unsigned int reserved_13 : 1;
        unsigned int reserved_14 : 1;
        unsigned int reserved_15 : 1;
        unsigned int reserved_16 : 1;
        unsigned int reserved_17 : 1;
    } reg;
} SOC_SCTRL_SCPERDIS4_SEC_UNION;
#endif
#define SOC_SCTRL_SCPERDIS4_SEC_gt_clk_hieps_tee_START (3)
#define SOC_SCTRL_SCPERDIS4_SEC_gt_clk_hieps_tee_END (3)
#define SOC_SCTRL_SCPERDIS4_SEC_gt_clk_hieps_tcxo_START (5)
#define SOC_SCTRL_SCPERDIS4_SEC_gt_clk_hieps_tcxo_END (5)
#define SOC_SCTRL_SCPERDIS4_SEC_gt_clk_hisee_pll_START (7)
#define SOC_SCTRL_SCPERDIS4_SEC_gt_clk_hisee_pll_END (7)
#define SOC_SCTRL_SCPERDIS4_SEC_gt_clk_qcr_hisee_START (8)
#define SOC_SCTRL_SCPERDIS4_SEC_gt_clk_qcr_hisee_END (8)
#define SOC_SCTRL_SCPERDIS4_SEC_gt_clk_hisee_ref_START (10)
#define SOC_SCTRL_SCPERDIS4_SEC_gt_clk_hisee_ref_END (10)
#define SOC_SCTRL_SCPERDIS4_SEC_gt_clk_spll_sscg_START (12)
#define SOC_SCTRL_SCPERDIS4_SEC_gt_clk_spll_sscg_END (12)
#define SOC_SCTRL_SCPERDIS4_SEC_gt_clk_aobus_START (13)
#define SOC_SCTRL_SCPERDIS4_SEC_gt_clk_aobus_END (13)
#define SOC_SCTRL_SCPERDIS4_SEC_gt_clk_aonoc2mspcbus_START (14)
#define SOC_SCTRL_SCPERDIS4_SEC_gt_clk_aonoc2mspcbus_END (14)
#define SOC_SCTRL_SCPERDIS4_SEC_gt_clk_noc_aobus2sysbusddrc_START (15)
#define SOC_SCTRL_SCPERDIS4_SEC_gt_clk_noc_aobus2sysbusddrc_END (15)
#define SOC_SCTRL_SCPERDIS4_SEC_gt_clk_autodiv_aonoc_pll_START (16)
#define SOC_SCTRL_SCPERDIS4_SEC_gt_clk_autodiv_aonoc_pll_END (16)
#define SOC_SCTRL_SCPERDIS4_SEC_gt_clk_autodiv_aonoc_fll_START (17)
#define SOC_SCTRL_SCPERDIS4_SEC_gt_clk_autodiv_aonoc_fll_END (17)
#define SOC_SCTRL_SCPERDIS4_SEC_gt_timerclk_refh_START (18)
#define SOC_SCTRL_SCPERDIS4_SEC_gt_timerclk_refh_END (18)
#define SOC_SCTRL_SCPERDIS4_SEC_gt_clk_pll_fsm_START (19)
#define SOC_SCTRL_SCPERDIS4_SEC_gt_clk_pll_fsm_END (19)
#define SOC_SCTRL_SCPERDIS4_SEC_gt_clk_fll_src_peri_START (20)
#define SOC_SCTRL_SCPERDIS4_SEC_gt_clk_fll_src_peri_END (20)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int gt_clk_hieps_tee : 1;
        unsigned int reserved_3 : 1;
        unsigned int gt_clk_hieps_tcxo : 1;
        unsigned int reserved_4 : 1;
        unsigned int gt_clk_hisee_pll : 1;
        unsigned int gt_clk_qcr_hisee : 1;
        unsigned int reserved_5 : 1;
        unsigned int gt_clk_hisee_ref : 1;
        unsigned int reserved_6 : 1;
        unsigned int gt_clk_spll_sscg : 1;
        unsigned int gt_clk_aobus : 1;
        unsigned int gt_clk_aonoc2mspcbus : 1;
        unsigned int gt_clk_noc_aobus2sysbusddrc : 1;
        unsigned int gt_clk_autodiv_aonoc_pll : 1;
        unsigned int gt_clk_autodiv_aonoc_fll : 1;
        unsigned int gt_timerclk_refh : 1;
        unsigned int gt_clk_pll_fsm : 1;
        unsigned int gt_clk_fll_src_peri : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int reserved_11 : 1;
        unsigned int reserved_12 : 1;
        unsigned int reserved_13 : 1;
        unsigned int reserved_14 : 1;
        unsigned int reserved_15 : 1;
        unsigned int reserved_16 : 1;
        unsigned int reserved_17 : 1;
    } reg;
} SOC_SCTRL_SCPERCLKEN4_SEC_UNION;
#endif
#define SOC_SCTRL_SCPERCLKEN4_SEC_gt_clk_hieps_tee_START (3)
#define SOC_SCTRL_SCPERCLKEN4_SEC_gt_clk_hieps_tee_END (3)
#define SOC_SCTRL_SCPERCLKEN4_SEC_gt_clk_hieps_tcxo_START (5)
#define SOC_SCTRL_SCPERCLKEN4_SEC_gt_clk_hieps_tcxo_END (5)
#define SOC_SCTRL_SCPERCLKEN4_SEC_gt_clk_hisee_pll_START (7)
#define SOC_SCTRL_SCPERCLKEN4_SEC_gt_clk_hisee_pll_END (7)
#define SOC_SCTRL_SCPERCLKEN4_SEC_gt_clk_qcr_hisee_START (8)
#define SOC_SCTRL_SCPERCLKEN4_SEC_gt_clk_qcr_hisee_END (8)
#define SOC_SCTRL_SCPERCLKEN4_SEC_gt_clk_hisee_ref_START (10)
#define SOC_SCTRL_SCPERCLKEN4_SEC_gt_clk_hisee_ref_END (10)
#define SOC_SCTRL_SCPERCLKEN4_SEC_gt_clk_spll_sscg_START (12)
#define SOC_SCTRL_SCPERCLKEN4_SEC_gt_clk_spll_sscg_END (12)
#define SOC_SCTRL_SCPERCLKEN4_SEC_gt_clk_aobus_START (13)
#define SOC_SCTRL_SCPERCLKEN4_SEC_gt_clk_aobus_END (13)
#define SOC_SCTRL_SCPERCLKEN4_SEC_gt_clk_aonoc2mspcbus_START (14)
#define SOC_SCTRL_SCPERCLKEN4_SEC_gt_clk_aonoc2mspcbus_END (14)
#define SOC_SCTRL_SCPERCLKEN4_SEC_gt_clk_noc_aobus2sysbusddrc_START (15)
#define SOC_SCTRL_SCPERCLKEN4_SEC_gt_clk_noc_aobus2sysbusddrc_END (15)
#define SOC_SCTRL_SCPERCLKEN4_SEC_gt_clk_autodiv_aonoc_pll_START (16)
#define SOC_SCTRL_SCPERCLKEN4_SEC_gt_clk_autodiv_aonoc_pll_END (16)
#define SOC_SCTRL_SCPERCLKEN4_SEC_gt_clk_autodiv_aonoc_fll_START (17)
#define SOC_SCTRL_SCPERCLKEN4_SEC_gt_clk_autodiv_aonoc_fll_END (17)
#define SOC_SCTRL_SCPERCLKEN4_SEC_gt_timerclk_refh_START (18)
#define SOC_SCTRL_SCPERCLKEN4_SEC_gt_timerclk_refh_END (18)
#define SOC_SCTRL_SCPERCLKEN4_SEC_gt_clk_pll_fsm_START (19)
#define SOC_SCTRL_SCPERCLKEN4_SEC_gt_clk_pll_fsm_END (19)
#define SOC_SCTRL_SCPERCLKEN4_SEC_gt_clk_fll_src_peri_START (20)
#define SOC_SCTRL_SCPERCLKEN4_SEC_gt_clk_fll_src_peri_END (20)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int div_aobus : 4;
        unsigned int div_iomcu_fll : 2;
        unsigned int sc_gt_clk_sci : 1;
        unsigned int timer_secu_en : 1;
        unsigned int sc_gt_clk_iomcu_fll : 1;
        unsigned int sc_gt_clk_iomcu_pll : 1;
        unsigned int reserved : 1;
        unsigned int hsdt_subsys_clk_sw_req_cfg : 2;
        unsigned int div_sci : 2;
        unsigned int sel_sci : 1;
        unsigned int scclkdiv0_sec_msk : 16;
    } reg;
} SOC_SCTRL_SCCLKDIV0_SEC_UNION;
#endif
#define SOC_SCTRL_SCCLKDIV0_SEC_div_aobus_START (0)
#define SOC_SCTRL_SCCLKDIV0_SEC_div_aobus_END (3)
#define SOC_SCTRL_SCCLKDIV0_SEC_div_iomcu_fll_START (4)
#define SOC_SCTRL_SCCLKDIV0_SEC_div_iomcu_fll_END (5)
#define SOC_SCTRL_SCCLKDIV0_SEC_sc_gt_clk_sci_START (6)
#define SOC_SCTRL_SCCLKDIV0_SEC_sc_gt_clk_sci_END (6)
#define SOC_SCTRL_SCCLKDIV0_SEC_timer_secu_en_START (7)
#define SOC_SCTRL_SCCLKDIV0_SEC_timer_secu_en_END (7)
#define SOC_SCTRL_SCCLKDIV0_SEC_sc_gt_clk_iomcu_fll_START (8)
#define SOC_SCTRL_SCCLKDIV0_SEC_sc_gt_clk_iomcu_fll_END (8)
#define SOC_SCTRL_SCCLKDIV0_SEC_sc_gt_clk_iomcu_pll_START (9)
#define SOC_SCTRL_SCCLKDIV0_SEC_sc_gt_clk_iomcu_pll_END (9)
#define SOC_SCTRL_SCCLKDIV0_SEC_hsdt_subsys_clk_sw_req_cfg_START (11)
#define SOC_SCTRL_SCCLKDIV0_SEC_hsdt_subsys_clk_sw_req_cfg_END (12)
#define SOC_SCTRL_SCCLKDIV0_SEC_div_sci_START (13)
#define SOC_SCTRL_SCCLKDIV0_SEC_div_sci_END (14)
#define SOC_SCTRL_SCCLKDIV0_SEC_sel_sci_START (15)
#define SOC_SCTRL_SCCLKDIV0_SEC_sel_sci_END (15)
#define SOC_SCTRL_SCCLKDIV0_SEC_scclkdiv0_sec_msk_START (16)
#define SOC_SCTRL_SCCLKDIV0_SEC_scclkdiv0_sec_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int div_hsdt_subsys : 4;
        unsigned int sc_gt_clk_hsdt_subsys : 1;
        unsigned int div_hisee_pll : 6;
        unsigned int sc_gt_clk_hisee_pll : 1;
        unsigned int reserved : 4;
        unsigned int scclkdiv1_sec_msk : 16;
    } reg;
} SOC_SCTRL_SCCLKDIV1_SEC_UNION;
#endif
#define SOC_SCTRL_SCCLKDIV1_SEC_div_hsdt_subsys_START (0)
#define SOC_SCTRL_SCCLKDIV1_SEC_div_hsdt_subsys_END (3)
#define SOC_SCTRL_SCCLKDIV1_SEC_sc_gt_clk_hsdt_subsys_START (4)
#define SOC_SCTRL_SCCLKDIV1_SEC_sc_gt_clk_hsdt_subsys_END (4)
#define SOC_SCTRL_SCCLKDIV1_SEC_div_hisee_pll_START (5)
#define SOC_SCTRL_SCCLKDIV1_SEC_div_hisee_pll_END (10)
#define SOC_SCTRL_SCCLKDIV1_SEC_sc_gt_clk_hisee_pll_START (11)
#define SOC_SCTRL_SCCLKDIV1_SEC_sc_gt_clk_hisee_pll_END (11)
#define SOC_SCTRL_SCCLKDIV1_SEC_scclkdiv1_sec_msk_START (16)
#define SOC_SCTRL_SCCLKDIV1_SEC_scclkdiv1_sec_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int sc_gt_clk_hieps_tee : 1;
        unsigned int div_hieps_tee : 6;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 6;
        unsigned int div_aonoc_fll : 2;
        unsigned int scclkdiv2_sec_msk : 16;
    } reg;
} SOC_SCTRL_SCCLKDIV2_SEC_UNION;
#endif
#define SOC_SCTRL_SCCLKDIV2_SEC_sc_gt_clk_hieps_tee_START (0)
#define SOC_SCTRL_SCCLKDIV2_SEC_sc_gt_clk_hieps_tee_END (0)
#define SOC_SCTRL_SCCLKDIV2_SEC_div_hieps_tee_START (1)
#define SOC_SCTRL_SCCLKDIV2_SEC_div_hieps_tee_END (6)
#define SOC_SCTRL_SCCLKDIV2_SEC_div_aonoc_fll_START (14)
#define SOC_SCTRL_SCCLKDIV2_SEC_div_aonoc_fll_END (15)
#define SOC_SCTRL_SCCLKDIV2_SEC_scclkdiv2_sec_msk_START (16)
#define SOC_SCTRL_SCCLKDIV2_SEC_scclkdiv2_sec_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0 : 5;
        unsigned int reserved_1 : 1;
        unsigned int div_aobus_noc : 4;
        unsigned int sc_gt_clk_aobus : 1;
        unsigned int sc_gt_aonoc_pll : 1;
        unsigned int sc_gt_aonoc_fll : 1;
        unsigned int sc_gt_aonoc_sys : 1;
        unsigned int sel_clk_aonoc : 2;
        unsigned int scclkdiv3_sec_msk : 16;
    } reg;
} SOC_SCTRL_SCCLKDIV3_SEC_UNION;
#endif
#define SOC_SCTRL_SCCLKDIV3_SEC_div_aobus_noc_START (6)
#define SOC_SCTRL_SCCLKDIV3_SEC_div_aobus_noc_END (9)
#define SOC_SCTRL_SCCLKDIV3_SEC_sc_gt_clk_aobus_START (10)
#define SOC_SCTRL_SCCLKDIV3_SEC_sc_gt_clk_aobus_END (10)
#define SOC_SCTRL_SCCLKDIV3_SEC_sc_gt_aonoc_pll_START (11)
#define SOC_SCTRL_SCCLKDIV3_SEC_sc_gt_aonoc_pll_END (11)
#define SOC_SCTRL_SCCLKDIV3_SEC_sc_gt_aonoc_fll_START (12)
#define SOC_SCTRL_SCCLKDIV3_SEC_sc_gt_aonoc_fll_END (12)
#define SOC_SCTRL_SCCLKDIV3_SEC_sc_gt_aonoc_sys_START (13)
#define SOC_SCTRL_SCCLKDIV3_SEC_sc_gt_aonoc_sys_END (13)
#define SOC_SCTRL_SCCLKDIV3_SEC_sel_clk_aonoc_START (14)
#define SOC_SCTRL_SCCLKDIV3_SEC_sel_clk_aonoc_END (15)
#define SOC_SCTRL_SCCLKDIV3_SEC_scclkdiv3_sec_msk_START (16)
#define SOC_SCTRL_SCCLKDIV3_SEC_scclkdiv3_sec_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int aobusnoc_auto_waitcfg_out : 10;
        unsigned int aobusnoc_div_auto_cfg : 6;
        unsigned int scclkdiv4_sec_msk : 16;
    } reg;
} SOC_SCTRL_SCCLKDIV4_SEC_UNION;
#endif
#define SOC_SCTRL_SCCLKDIV4_SEC_aobusnoc_auto_waitcfg_out_START (0)
#define SOC_SCTRL_SCCLKDIV4_SEC_aobusnoc_auto_waitcfg_out_END (9)
#define SOC_SCTRL_SCCLKDIV4_SEC_aobusnoc_div_auto_cfg_START (10)
#define SOC_SCTRL_SCCLKDIV4_SEC_aobusnoc_div_auto_cfg_END (15)
#define SOC_SCTRL_SCCLKDIV4_SEC_scclkdiv4_sec_msk_START (16)
#define SOC_SCTRL_SCCLKDIV4_SEC_scclkdiv4_sec_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int aobusnoc_div_auto_reduce_bypass : 1;
        unsigned int aobusnoc_auto_waitcfg_in : 10;
        unsigned int aobus_clk_sw_req_cfg : 2;
        unsigned int sc_gt_clk_aobus_fll_bypass : 1;
        unsigned int reserved : 2;
        unsigned int scclkdiv5_sec_msk : 16;
    } reg;
} SOC_SCTRL_SCCLKDIV5_SEC_UNION;
#endif
#define SOC_SCTRL_SCCLKDIV5_SEC_aobusnoc_div_auto_reduce_bypass_START (0)
#define SOC_SCTRL_SCCLKDIV5_SEC_aobusnoc_div_auto_reduce_bypass_END (0)
#define SOC_SCTRL_SCCLKDIV5_SEC_aobusnoc_auto_waitcfg_in_START (1)
#define SOC_SCTRL_SCCLKDIV5_SEC_aobusnoc_auto_waitcfg_in_END (10)
#define SOC_SCTRL_SCCLKDIV5_SEC_aobus_clk_sw_req_cfg_START (11)
#define SOC_SCTRL_SCCLKDIV5_SEC_aobus_clk_sw_req_cfg_END (12)
#define SOC_SCTRL_SCCLKDIV5_SEC_sc_gt_clk_aobus_fll_bypass_START (13)
#define SOC_SCTRL_SCCLKDIV5_SEC_sc_gt_clk_aobus_fll_bypass_END (13)
#define SOC_SCTRL_SCCLKDIV5_SEC_scclkdiv5_sec_msk_START (16)
#define SOC_SCTRL_SCCLKDIV5_SEC_scclkdiv5_sec_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved : 4;
        unsigned int sel_noc_timeout_extref : 1;
        unsigned int div_noc_timeout_extref : 6;
        unsigned int div_aobus_fll : 5;
        unsigned int scclkdiv6_sec_msk : 16;
    } reg;
} SOC_SCTRL_SCCLKDIV6_SEC_UNION;
#endif
#define SOC_SCTRL_SCCLKDIV6_SEC_sel_noc_timeout_extref_START (4)
#define SOC_SCTRL_SCCLKDIV6_SEC_sel_noc_timeout_extref_END (4)
#define SOC_SCTRL_SCCLKDIV6_SEC_div_noc_timeout_extref_START (5)
#define SOC_SCTRL_SCCLKDIV6_SEC_div_noc_timeout_extref_END (10)
#define SOC_SCTRL_SCCLKDIV6_SEC_div_aobus_fll_START (11)
#define SOC_SCTRL_SCCLKDIV6_SEC_div_aobus_fll_END (15)
#define SOC_SCTRL_SCCLKDIV6_SEC_scclkdiv6_sec_msk_START (16)
#define SOC_SCTRL_SCCLKDIV6_SEC_scclkdiv6_sec_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int aupll_en_vote : 5;
        unsigned int aupll_bypass_vote : 5;
        unsigned int aupll_gt_vote : 5;
        unsigned int reserved : 1;
        unsigned int scclkdiv7_sec_msk : 16;
    } reg;
} SOC_SCTRL_SCCLKDIV7_SEC_UNION;
#endif
#define SOC_SCTRL_SCCLKDIV7_SEC_aupll_en_vote_START (0)
#define SOC_SCTRL_SCCLKDIV7_SEC_aupll_en_vote_END (4)
#define SOC_SCTRL_SCCLKDIV7_SEC_aupll_bypass_vote_START (5)
#define SOC_SCTRL_SCCLKDIV7_SEC_aupll_bypass_vote_END (9)
#define SOC_SCTRL_SCCLKDIV7_SEC_aupll_gt_vote_START (10)
#define SOC_SCTRL_SCCLKDIV7_SEC_aupll_gt_vote_END (14)
#define SOC_SCTRL_SCCLKDIV7_SEC_scclkdiv7_sec_msk_START (16)
#define SOC_SCTRL_SCCLKDIV7_SEC_scclkdiv7_sec_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int noc_asp_mst_i_mainnordpendingtrans_bp : 1;
        unsigned int noc_asp_mst_i_mainnowrpendingtrans_bp : 1;
        unsigned int noc_asp_mst_rt_i_mainnowrpendingtrans_bp : 1;
        unsigned int noc_asp_mst_rt_i_mainnordpendingtrans_bp : 1;
        unsigned int noc_hisee_mst_i_mainnowrpendingtrans_bp : 1;
        unsigned int noc_hisee_mst_i_mainnordpendingtrans_bp : 1;
        unsigned int noc_aobus_t_mainnopendingtrans_bp : 1;
        unsigned int aobus_qcr_dcdr_ao_top_bp : 1;
        unsigned int aobus_mspe2aobus_pwractive_bp : 1;
        unsigned int aobus_tcp_h2x_idle_bp : 1;
        unsigned int aobus_tcp_i_nopending_bp : 1;
        unsigned int aobus_tcp_busy_bp : 1;
        unsigned int reserved : 4;
        unsigned int scclkdiv8_sec_msk : 16;
    } reg;
} SOC_SCTRL_SCCLKDIV8_SEC_UNION;
#endif
#define SOC_SCTRL_SCCLKDIV8_SEC_noc_asp_mst_i_mainnordpendingtrans_bp_START (0)
#define SOC_SCTRL_SCCLKDIV8_SEC_noc_asp_mst_i_mainnordpendingtrans_bp_END (0)
#define SOC_SCTRL_SCCLKDIV8_SEC_noc_asp_mst_i_mainnowrpendingtrans_bp_START (1)
#define SOC_SCTRL_SCCLKDIV8_SEC_noc_asp_mst_i_mainnowrpendingtrans_bp_END (1)
#define SOC_SCTRL_SCCLKDIV8_SEC_noc_asp_mst_rt_i_mainnowrpendingtrans_bp_START (2)
#define SOC_SCTRL_SCCLKDIV8_SEC_noc_asp_mst_rt_i_mainnowrpendingtrans_bp_END (2)
#define SOC_SCTRL_SCCLKDIV8_SEC_noc_asp_mst_rt_i_mainnordpendingtrans_bp_START (3)
#define SOC_SCTRL_SCCLKDIV8_SEC_noc_asp_mst_rt_i_mainnordpendingtrans_bp_END (3)
#define SOC_SCTRL_SCCLKDIV8_SEC_noc_hisee_mst_i_mainnowrpendingtrans_bp_START (4)
#define SOC_SCTRL_SCCLKDIV8_SEC_noc_hisee_mst_i_mainnowrpendingtrans_bp_END (4)
#define SOC_SCTRL_SCCLKDIV8_SEC_noc_hisee_mst_i_mainnordpendingtrans_bp_START (5)
#define SOC_SCTRL_SCCLKDIV8_SEC_noc_hisee_mst_i_mainnordpendingtrans_bp_END (5)
#define SOC_SCTRL_SCCLKDIV8_SEC_noc_aobus_t_mainnopendingtrans_bp_START (6)
#define SOC_SCTRL_SCCLKDIV8_SEC_noc_aobus_t_mainnopendingtrans_bp_END (6)
#define SOC_SCTRL_SCCLKDIV8_SEC_aobus_qcr_dcdr_ao_top_bp_START (7)
#define SOC_SCTRL_SCCLKDIV8_SEC_aobus_qcr_dcdr_ao_top_bp_END (7)
#define SOC_SCTRL_SCCLKDIV8_SEC_aobus_mspe2aobus_pwractive_bp_START (8)
#define SOC_SCTRL_SCCLKDIV8_SEC_aobus_mspe2aobus_pwractive_bp_END (8)
#define SOC_SCTRL_SCCLKDIV8_SEC_aobus_tcp_h2x_idle_bp_START (9)
#define SOC_SCTRL_SCCLKDIV8_SEC_aobus_tcp_h2x_idle_bp_END (9)
#define SOC_SCTRL_SCCLKDIV8_SEC_aobus_tcp_i_nopending_bp_START (10)
#define SOC_SCTRL_SCCLKDIV8_SEC_aobus_tcp_i_nopending_bp_END (10)
#define SOC_SCTRL_SCCLKDIV8_SEC_aobus_tcp_busy_bp_START (11)
#define SOC_SCTRL_SCCLKDIV8_SEC_aobus_tcp_busy_bp_END (11)
#define SOC_SCTRL_SCCLKDIV8_SEC_scclkdiv8_sec_msk_START (16)
#define SOC_SCTRL_SCCLKDIV8_SEC_scclkdiv8_sec_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int div_clk_pll_fsm : 6;
        unsigned int sc_gt_clk_pll_fsm : 1;
        unsigned int div_clk_ao_332m : 3;
        unsigned int reserved : 6;
        unsigned int scclkdiv9_sec_msk : 16;
    } reg;
} SOC_SCTRL_SCCLKDIV9_SEC_UNION;
#endif
#define SOC_SCTRL_SCCLKDIV9_SEC_div_clk_pll_fsm_START (0)
#define SOC_SCTRL_SCCLKDIV9_SEC_div_clk_pll_fsm_END (5)
#define SOC_SCTRL_SCCLKDIV9_SEC_sc_gt_clk_pll_fsm_START (6)
#define SOC_SCTRL_SCCLKDIV9_SEC_sc_gt_clk_pll_fsm_END (6)
#define SOC_SCTRL_SCCLKDIV9_SEC_div_clk_ao_332m_START (7)
#define SOC_SCTRL_SCCLKDIV9_SEC_div_clk_ao_332m_END (9)
#define SOC_SCTRL_SCCLKDIV9_SEC_scclkdiv9_sec_msk_START (16)
#define SOC_SCTRL_SCCLKDIV9_SEC_scclkdiv9_sec_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int div_timer : 6;
        unsigned int sc_gt_clk_timer : 1;
        unsigned int reserved_0 : 3;
        unsigned int reserved_1 : 6;
        unsigned int scclkdiv10_sec_msk : 16;
    } reg;
} SOC_SCTRL_SCCLKDIV10_SEC_UNION;
#endif
#define SOC_SCTRL_SCCLKDIV10_SEC_div_timer_START (0)
#define SOC_SCTRL_SCCLKDIV10_SEC_div_timer_END (5)
#define SOC_SCTRL_SCCLKDIV10_SEC_sc_gt_clk_timer_START (6)
#define SOC_SCTRL_SCCLKDIV10_SEC_sc_gt_clk_timer_END (6)
#define SOC_SCTRL_SCCLKDIV10_SEC_scclkdiv10_sec_msk_START (16)
#define SOC_SCTRL_SCCLKDIV10_SEC_scclkdiv10_sec_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0: 1;
        unsigned int reserved_1: 1;
        unsigned int reserved_2: 1;
        unsigned int reserved_3: 1;
        unsigned int reserved_4: 1;
        unsigned int reserved_5: 1;
        unsigned int reserved_6: 1;
        unsigned int reserved_7: 1;
        unsigned int reserved_8: 1;
        unsigned int reserved_9: 1;
        unsigned int reserved_10: 1;
        unsigned int reserved_11: 1;
        unsigned int reserved_12: 1;
        unsigned int reserved_13: 1;
        unsigned int reserved_14: 1;
        unsigned int reserved_15: 1;
        unsigned int reserved_16: 1;
        unsigned int reserved_17: 1;
        unsigned int reserved_18: 1;
        unsigned int reserved_19: 1;
        unsigned int reserved_20: 1;
        unsigned int reserved_21: 1;
        unsigned int reserved_22: 1;
        unsigned int reserved_23: 1;
        unsigned int reserved_24: 1;
        unsigned int reserved_25: 1;
        unsigned int reserved_26: 1;
        unsigned int reserved_27: 1;
        unsigned int reserved_28: 1;
        unsigned int reserved_29: 1;
        unsigned int reserved_30: 1;
        unsigned int reserved_31: 1;
    } reg;
} SOC_SCTRL_SCPERRSTEN4_SEC_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0: 1;
        unsigned int reserved_1: 1;
        unsigned int reserved_2: 1;
        unsigned int reserved_3: 1;
        unsigned int reserved_4: 1;
        unsigned int reserved_5: 1;
        unsigned int reserved_6: 1;
        unsigned int reserved_7: 1;
        unsigned int reserved_8: 1;
        unsigned int reserved_9: 1;
        unsigned int reserved_10: 1;
        unsigned int reserved_11: 1;
        unsigned int reserved_12: 1;
        unsigned int reserved_13: 1;
        unsigned int reserved_14: 1;
        unsigned int reserved_15: 1;
        unsigned int reserved_16: 1;
        unsigned int reserved_17: 1;
        unsigned int reserved_18: 1;
        unsigned int reserved_19: 1;
        unsigned int reserved_20: 1;
        unsigned int reserved_21: 1;
        unsigned int reserved_22: 1;
        unsigned int reserved_23: 1;
        unsigned int reserved_24: 1;
        unsigned int reserved_25: 1;
        unsigned int reserved_26: 1;
        unsigned int reserved_27: 1;
        unsigned int reserved_28: 1;
        unsigned int reserved_29: 1;
        unsigned int reserved_30: 1;
        unsigned int reserved_31: 1;
    } reg;
} SOC_SCTRL_SCPERRSTDIS4_SEC_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0: 1;
        unsigned int reserved_1: 1;
        unsigned int reserved_2: 1;
        unsigned int reserved_3: 1;
        unsigned int reserved_4: 1;
        unsigned int reserved_5: 1;
        unsigned int reserved_6: 1;
        unsigned int reserved_7: 1;
        unsigned int reserved_8: 1;
        unsigned int reserved_9: 1;
        unsigned int reserved_10: 1;
        unsigned int reserved_11: 1;
        unsigned int reserved_12: 1;
        unsigned int reserved_13: 1;
        unsigned int reserved_14: 1;
        unsigned int reserved_15: 1;
        unsigned int reserved_16: 1;
        unsigned int reserved_17: 1;
        unsigned int reserved_18: 1;
        unsigned int reserved_19: 1;
        unsigned int reserved_20: 1;
        unsigned int reserved_21: 1;
        unsigned int reserved_22: 1;
        unsigned int reserved_23: 1;
        unsigned int reserved_24: 1;
        unsigned int reserved_25: 1;
        unsigned int reserved_26: 1;
        unsigned int reserved_27: 1;
        unsigned int reserved_28: 1;
        unsigned int reserved_29: 1;
        unsigned int reserved_30: 1;
        unsigned int reserved_31: 1;
    } reg;
} SOC_SCTRL_SCPERRSTSTAT4_SEC_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int rtc_clkrst_bypass : 1;
        unsigned int rtc1_clkrst_bypass : 1;
        unsigned int timer0_clkrst_bypass : 1;
        unsigned int timer1_clkrst_bypass : 1;
        unsigned int timer2_clkrst_bypass : 1;
        unsigned int timer3_clkrst_bypass : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int ioc_clkrst_bypass : 1;
        unsigned int syscnt_clkrst_bypass : 1;
        unsigned int sci0_clkrst_bypass : 1;
        unsigned int sci1_clkrst_bypass : 1;
        unsigned int bbpdrx_clkrst_bypass : 1;
        unsigned int reserved_6 : 1;
        unsigned int timer4_clkrst_bypass : 1;
        unsigned int timer5_clkrst_bypass : 1;
        unsigned int timer6_clkrst_bypass : 1;
        unsigned int timer7_clkrst_bypass : 1;
        unsigned int timer8_clkrst_bypass : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int gpio1_se_clkrst_bypass : 1;
        unsigned int reserved_10 : 1;
        unsigned int reserved_11 : 1;
        unsigned int ao_loadmonitor_clkrst_bypass : 1;
        unsigned int spmi_clkrst_bypass : 1;
        unsigned int ao_tzpc_clkrst_bypass : 1;
    } reg;
} SOC_SCTRL_SC_IPCLKRST_BYPASS0_SEC_UNION;
#endif
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_SEC_rtc_clkrst_bypass_START (0)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_SEC_rtc_clkrst_bypass_END (0)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_SEC_rtc1_clkrst_bypass_START (1)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_SEC_rtc1_clkrst_bypass_END (1)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_SEC_timer0_clkrst_bypass_START (2)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_SEC_timer0_clkrst_bypass_END (2)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_SEC_timer1_clkrst_bypass_START (3)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_SEC_timer1_clkrst_bypass_END (3)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_SEC_timer2_clkrst_bypass_START (4)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_SEC_timer2_clkrst_bypass_END (4)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_SEC_timer3_clkrst_bypass_START (5)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_SEC_timer3_clkrst_bypass_END (5)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_SEC_ioc_clkrst_bypass_START (12)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_SEC_ioc_clkrst_bypass_END (12)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_SEC_syscnt_clkrst_bypass_START (13)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_SEC_syscnt_clkrst_bypass_END (13)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_SEC_sci0_clkrst_bypass_START (14)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_SEC_sci0_clkrst_bypass_END (14)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_SEC_sci1_clkrst_bypass_START (15)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_SEC_sci1_clkrst_bypass_END (15)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_SEC_bbpdrx_clkrst_bypass_START (16)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_SEC_bbpdrx_clkrst_bypass_END (16)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_SEC_timer4_clkrst_bypass_START (18)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_SEC_timer4_clkrst_bypass_END (18)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_SEC_timer5_clkrst_bypass_START (19)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_SEC_timer5_clkrst_bypass_END (19)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_SEC_timer6_clkrst_bypass_START (20)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_SEC_timer6_clkrst_bypass_END (20)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_SEC_timer7_clkrst_bypass_START (21)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_SEC_timer7_clkrst_bypass_END (21)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_SEC_timer8_clkrst_bypass_START (22)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_SEC_timer8_clkrst_bypass_END (22)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_SEC_gpio1_se_clkrst_bypass_START (26)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_SEC_gpio1_se_clkrst_bypass_END (26)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_SEC_ao_loadmonitor_clkrst_bypass_START (29)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_SEC_ao_loadmonitor_clkrst_bypass_END (29)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_SEC_spmi_clkrst_bypass_START (30)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_SEC_spmi_clkrst_bypass_END (30)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_SEC_ao_tzpc_clkrst_bypass_START (31)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS0_SEC_ao_tzpc_clkrst_bypass_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ao_ipc_clkrst_bypass : 1;
        unsigned int ao_ipc_ns_clkrst_bypass : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int spi3_clkrst_bypass : 1;
        unsigned int reserved_2 : 1;
        unsigned int ufs_sys_ctrl_clkrst_bypass : 1;
        unsigned int ao_wd_clkrst_bypass : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int ao_tcp_clkrst_bypass : 1;
        unsigned int timer17_clkrst_bypass : 1;
        unsigned int timer16_clkrst_bypass : 1;
        unsigned int timer15_clkrst_bypass : 1;
        unsigned int timer14_clkrst_bypass : 1;
        unsigned int timer13_clkrst_bypass : 1;
        unsigned int spi5_clkrst_bypass : 1;
        unsigned int ao_wd1_clkrst_bypass : 1;
        unsigned int i2c9_clkrst_bypass : 1;
        unsigned int blpwm2_clkrst_bypass : 1;
        unsigned int intr_hub_clkrst_bypass : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int efusec_clkrst_bypass : 1;
        unsigned int efusec2_clkrst_bypass : 1;
        unsigned int hickm_clkrst_bypass : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int reserved_11 : 1;
        unsigned int reserved_12 : 1;
    } reg;
} SOC_SCTRL_SC_IPCLKRST_BYPASS1_SEC_UNION;
#endif
#define SOC_SCTRL_SC_IPCLKRST_BYPASS1_SEC_ao_ipc_clkrst_bypass_START (0)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS1_SEC_ao_ipc_clkrst_bypass_END (0)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS1_SEC_ao_ipc_ns_clkrst_bypass_START (1)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS1_SEC_ao_ipc_ns_clkrst_bypass_END (1)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS1_SEC_spi3_clkrst_bypass_START (4)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS1_SEC_spi3_clkrst_bypass_END (4)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS1_SEC_ufs_sys_ctrl_clkrst_bypass_START (6)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS1_SEC_ufs_sys_ctrl_clkrst_bypass_END (6)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS1_SEC_ao_wd_clkrst_bypass_START (7)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS1_SEC_ao_wd_clkrst_bypass_END (7)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS1_SEC_ao_tcp_clkrst_bypass_START (11)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS1_SEC_ao_tcp_clkrst_bypass_END (11)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS1_SEC_timer17_clkrst_bypass_START (12)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS1_SEC_timer17_clkrst_bypass_END (12)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS1_SEC_timer16_clkrst_bypass_START (13)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS1_SEC_timer16_clkrst_bypass_END (13)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS1_SEC_timer15_clkrst_bypass_START (14)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS1_SEC_timer15_clkrst_bypass_END (14)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS1_SEC_timer14_clkrst_bypass_START (15)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS1_SEC_timer14_clkrst_bypass_END (15)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS1_SEC_timer13_clkrst_bypass_START (16)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS1_SEC_timer13_clkrst_bypass_END (16)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS1_SEC_spi5_clkrst_bypass_START (17)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS1_SEC_spi5_clkrst_bypass_END (17)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS1_SEC_ao_wd1_clkrst_bypass_START (18)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS1_SEC_ao_wd1_clkrst_bypass_END (18)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS1_SEC_i2c9_clkrst_bypass_START (19)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS1_SEC_i2c9_clkrst_bypass_END (19)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS1_SEC_blpwm2_clkrst_bypass_START (20)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS1_SEC_blpwm2_clkrst_bypass_END (20)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS1_SEC_intr_hub_clkrst_bypass_START (21)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS1_SEC_intr_hub_clkrst_bypass_END (21)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS1_SEC_efusec_clkrst_bypass_START (25)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS1_SEC_efusec_clkrst_bypass_END (25)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS1_SEC_efusec2_clkrst_bypass_START (26)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS1_SEC_efusec2_clkrst_bypass_END (26)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS1_SEC_hickm_clkrst_bypass_START (27)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS1_SEC_hickm_clkrst_bypass_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int qic_mspcbus_clkrst_bypass : 1;
        unsigned int qic_aobus2mspcbus_clkrst_bypass : 1;
        unsigned int qic_iomcu_apb_peri_clkrst_bypass : 1;
        unsigned int qic_iomcu_clkrst_bypass : 1;
        unsigned int qic_aobus2iomcubus_clkrst_bypass : 1;
        unsigned int qic_aspbus_clkrst_bypass : 1;
        unsigned int qic_aobus2aspbus_clkrst_bypass : 1;
        unsigned int qcrdcdr_mspcbus_clkrst_bypass : 1;
        unsigned int qcrdcdr_iomcubus_clkrst_bypass : 1;
        unsigned int qcrdcdr_aspbus_clkrst_bypass : 1;
        unsigned int qic_spi_dma_clkrst_bypass : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int reserved_11 : 1;
        unsigned int reserved_12 : 1;
        unsigned int reserved_13 : 1;
        unsigned int reserved_14 : 1;
        unsigned int reserved_15 : 1;
        unsigned int reserved_16 : 1;
        unsigned int reserved_17 : 1;
        unsigned int reserved_18 : 1;
        unsigned int reserved_19 : 1;
        unsigned int reserved_20 : 1;
    } reg;
} SOC_SCTRL_SC_IPCLKRST_BYPASS2_SEC_UNION;
#endif
#define SOC_SCTRL_SC_IPCLKRST_BYPASS2_SEC_qic_mspcbus_clkrst_bypass_START (0)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS2_SEC_qic_mspcbus_clkrst_bypass_END (0)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS2_SEC_qic_aobus2mspcbus_clkrst_bypass_START (1)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS2_SEC_qic_aobus2mspcbus_clkrst_bypass_END (1)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS2_SEC_qic_iomcu_apb_peri_clkrst_bypass_START (2)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS2_SEC_qic_iomcu_apb_peri_clkrst_bypass_END (2)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS2_SEC_qic_iomcu_clkrst_bypass_START (3)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS2_SEC_qic_iomcu_clkrst_bypass_END (3)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS2_SEC_qic_aobus2iomcubus_clkrst_bypass_START (4)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS2_SEC_qic_aobus2iomcubus_clkrst_bypass_END (4)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS2_SEC_qic_aspbus_clkrst_bypass_START (5)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS2_SEC_qic_aspbus_clkrst_bypass_END (5)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS2_SEC_qic_aobus2aspbus_clkrst_bypass_START (6)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS2_SEC_qic_aobus2aspbus_clkrst_bypass_END (6)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS2_SEC_qcrdcdr_mspcbus_clkrst_bypass_START (7)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS2_SEC_qcrdcdr_mspcbus_clkrst_bypass_END (7)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS2_SEC_qcrdcdr_iomcubus_clkrst_bypass_START (8)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS2_SEC_qcrdcdr_iomcubus_clkrst_bypass_END (8)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS2_SEC_qcrdcdr_aspbus_clkrst_bypass_START (9)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS2_SEC_qcrdcdr_aspbus_clkrst_bypass_END (9)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS2_SEC_qic_spi_dma_clkrst_bypass_START (10)
#define SOC_SCTRL_SC_IPCLKRST_BYPASS2_SEC_qic_spi_dma_clkrst_bypass_END (10)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int st_clk_hieps_tee : 1;
        unsigned int reserved_0 : 1;
        unsigned int st_clk_hieps_tcxo : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int st_clk_hisee_pll : 1;
        unsigned int st_clk_qcr_hisee : 1;
        unsigned int st_clk_noc_hiseebus : 1;
        unsigned int st_clk_hisee_ref : 1;
        unsigned int reserved_6 : 1;
        unsigned int st_clk_aonoc2mspcbus : 1;
        unsigned int reserved_7 : 18;
    } reg;
} SOC_SCTRL_SC_CLK_STAT0_SEC_UNION;
#endif
#define SOC_SCTRL_SC_CLK_STAT0_SEC_st_clk_hieps_tee_START (0)
#define SOC_SCTRL_SC_CLK_STAT0_SEC_st_clk_hieps_tee_END (0)
#define SOC_SCTRL_SC_CLK_STAT0_SEC_st_clk_hieps_tcxo_START (2)
#define SOC_SCTRL_SC_CLK_STAT0_SEC_st_clk_hieps_tcxo_END (2)
#define SOC_SCTRL_SC_CLK_STAT0_SEC_st_clk_hisee_pll_START (8)
#define SOC_SCTRL_SC_CLK_STAT0_SEC_st_clk_hisee_pll_END (8)
#define SOC_SCTRL_SC_CLK_STAT0_SEC_st_clk_qcr_hisee_START (9)
#define SOC_SCTRL_SC_CLK_STAT0_SEC_st_clk_qcr_hisee_END (9)
#define SOC_SCTRL_SC_CLK_STAT0_SEC_st_clk_noc_hiseebus_START (10)
#define SOC_SCTRL_SC_CLK_STAT0_SEC_st_clk_noc_hiseebus_END (10)
#define SOC_SCTRL_SC_CLK_STAT0_SEC_st_clk_hisee_ref_START (11)
#define SOC_SCTRL_SC_CLK_STAT0_SEC_st_clk_hisee_ref_END (11)
#define SOC_SCTRL_SC_CLK_STAT0_SEC_st_clk_aonoc2mspcbus_START (13)
#define SOC_SCTRL_SC_CLK_STAT0_SEC_st_clk_aonoc2mspcbus_END (13)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int sw_ack_clk_hieps_tee_sw : 2;
        unsigned int swdone_clk_hieps_tee_div : 1;
        unsigned int reserved_0 : 2;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 2;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 23;
    } reg;
} SOC_SCTRL_SC_CLK_STAT1_SEC_UNION;
#endif
#define SOC_SCTRL_SC_CLK_STAT1_SEC_sw_ack_clk_hieps_tee_sw_START (0)
#define SOC_SCTRL_SC_CLK_STAT1_SEC_sw_ack_clk_hieps_tee_sw_END (1)
#define SOC_SCTRL_SC_CLK_STAT1_SEC_swdone_clk_hieps_tee_div_START (2)
#define SOC_SCTRL_SC_CLK_STAT1_SEC_swdone_clk_hieps_tee_div_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int aupll_fsm_en : 1;
        unsigned int aupll_vote_mode_sel : 1;
        unsigned int aupll_intr_clr : 1;
        unsigned int reserved : 13;
        unsigned int pll_fsm_ctrl0_sec_msk : 16;
    } reg;
} SOC_SCTRL_PLL_FSM_CTRL0_SEC_UNION;
#endif
#define SOC_SCTRL_PLL_FSM_CTRL0_SEC_aupll_fsm_en_START (0)
#define SOC_SCTRL_PLL_FSM_CTRL0_SEC_aupll_fsm_en_END (0)
#define SOC_SCTRL_PLL_FSM_CTRL0_SEC_aupll_vote_mode_sel_START (1)
#define SOC_SCTRL_PLL_FSM_CTRL0_SEC_aupll_vote_mode_sel_END (1)
#define SOC_SCTRL_PLL_FSM_CTRL0_SEC_aupll_intr_clr_START (2)
#define SOC_SCTRL_PLL_FSM_CTRL0_SEC_aupll_intr_clr_END (2)
#define SOC_SCTRL_PLL_FSM_CTRL0_SEC_pll_fsm_ctrl0_sec_msk_START (16)
#define SOC_SCTRL_PLL_FSM_CTRL0_SEC_pll_fsm_ctrl0_sec_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int pll_lock_dbc_en : 1;
        unsigned int pll_fsm_intr_en : 1;
        unsigned int pll_wait_en : 1;
        unsigned int pll_timeout_en : 1;
        unsigned int pll_intr_lmt : 3;
        unsigned int pll_wait_lmt : 8;
        unsigned int pll_timeout_lmt : 8;
        unsigned int reserved : 9;
    } reg;
} SOC_SCTRL_PLL_FSM_CTRL1_SEC_UNION;
#endif
#define SOC_SCTRL_PLL_FSM_CTRL1_SEC_pll_lock_dbc_en_START (0)
#define SOC_SCTRL_PLL_FSM_CTRL1_SEC_pll_lock_dbc_en_END (0)
#define SOC_SCTRL_PLL_FSM_CTRL1_SEC_pll_fsm_intr_en_START (1)
#define SOC_SCTRL_PLL_FSM_CTRL1_SEC_pll_fsm_intr_en_END (1)
#define SOC_SCTRL_PLL_FSM_CTRL1_SEC_pll_wait_en_START (2)
#define SOC_SCTRL_PLL_FSM_CTRL1_SEC_pll_wait_en_END (2)
#define SOC_SCTRL_PLL_FSM_CTRL1_SEC_pll_timeout_en_START (3)
#define SOC_SCTRL_PLL_FSM_CTRL1_SEC_pll_timeout_en_END (3)
#define SOC_SCTRL_PLL_FSM_CTRL1_SEC_pll_intr_lmt_START (4)
#define SOC_SCTRL_PLL_FSM_CTRL1_SEC_pll_intr_lmt_END (6)
#define SOC_SCTRL_PLL_FSM_CTRL1_SEC_pll_wait_lmt_START (7)
#define SOC_SCTRL_PLL_FSM_CTRL1_SEC_pll_wait_lmt_END (14)
#define SOC_SCTRL_PLL_FSM_CTRL1_SEC_pll_timeout_lmt_START (15)
#define SOC_SCTRL_PLL_FSM_CTRL1_SEC_pll_timeout_lmt_END (22)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int aupll_fsm_intr : 1;
        unsigned int aupll_intr_cnt : 3;
        unsigned int aupll_curr_stat : 4;
        unsigned int reserved : 24;
    } reg;
} SOC_SCTRL_PLL_FSM_STAT0_SEC_UNION;
#endif
#define SOC_SCTRL_PLL_FSM_STAT0_SEC_aupll_fsm_intr_START (0)
#define SOC_SCTRL_PLL_FSM_STAT0_SEC_aupll_fsm_intr_END (0)
#define SOC_SCTRL_PLL_FSM_STAT0_SEC_aupll_intr_cnt_START (1)
#define SOC_SCTRL_PLL_FSM_STAT0_SEC_aupll_intr_cnt_END (3)
#define SOC_SCTRL_PLL_FSM_STAT0_SEC_aupll_curr_stat_START (4)
#define SOC_SCTRL_PLL_FSM_STAT0_SEC_aupll_curr_stat_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int version_code : 16;
        unsigned int chip_code : 16;
    } reg;
} SOC_SCTRL_SCSOCID0_UNION;
#endif
#define SOC_SCTRL_SCSOCID0_version_code_START (0)
#define SOC_SCTRL_SCSOCID0_version_code_END (15)
#define SOC_SCTRL_SCSOCID0_chip_code_START (16)
#define SOC_SCTRL_SCSOCID0_chip_code_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved : 32;
    } reg;
} SOC_SCTRL_SCPERSTAT_POR_RESERVED0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved : 32;
    } reg;
} SOC_SCTRL_SCPERSTAT_POR_RESERVED1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved : 32;
    } reg;
} SOC_SCTRL_SCPERSTAT_POR_RESERVED2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved : 32;
    } reg;
} SOC_SCTRL_SCPERSTAT_POR_RESERVED3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int lpdata : 32;
    } reg;
} SOC_SCTRL_SC_NPU_LPCTRL0_SEC_UNION;
#endif
#define SOC_SCTRL_SC_NPU_LPCTRL0_SEC_lpdata_START (0)
#define SOC_SCTRL_SC_NPU_LPCTRL0_SEC_lpdata_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int lpdata : 32;
    } reg;
} SOC_SCTRL_SC_NPU_LPCTRL1_SEC_UNION;
#endif
#define SOC_SCTRL_SC_NPU_LPCTRL1_SEC_lpdata_START (0)
#define SOC_SCTRL_SC_NPU_LPCTRL1_SEC_lpdata_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int lpdata : 32;
    } reg;
} SOC_SCTRL_SC_NPU_LPCTRL2_SEC_UNION;
#endif
#define SOC_SCTRL_SC_NPU_LPCTRL2_SEC_lpdata_START (0)
#define SOC_SCTRL_SC_NPU_LPCTRL2_SEC_lpdata_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int lpdata : 32;
    } reg;
} SOC_SCTRL_SC_NPU_LPCTRL3_SEC_UNION;
#endif
#define SOC_SCTRL_SC_NPU_LPCTRL3_SEC_lpdata_START (0)
#define SOC_SCTRL_SC_NPU_LPCTRL3_SEC_lpdata_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int lpdata : 32;
    } reg;
} SOC_SCTRL_SC_NPU_LPCTRL4_SEC_UNION;
#endif
#define SOC_SCTRL_SC_NPU_LPCTRL4_SEC_lpdata_START (0)
#define SOC_SCTRL_SC_NPU_LPCTRL4_SEC_lpdata_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved : 32;
    } reg;
} SOC_SCTRL_SC_TRACKING_LOG0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved : 32;
    } reg;
} SOC_SCTRL_SC_TRACKING_LOG1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved : 32;
    } reg;
} SOC_SCTRL_SC_TRACKING_LOG2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved : 32;
    } reg;
} SOC_SCTRL_SC_TRACKING_LOG3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int sc_emu_version : 32;
    } reg;
} SOC_SCTRL_SC_EMU_VERSION_UNION;
#endif
#define SOC_SCTRL_SC_EMU_VERSION_sc_emu_version_START (0)
#define SOC_SCTRL_SC_EMU_VERSION_sc_emu_version_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ddrc_csysreq_cfg_sc2aocrg : 4;
        unsigned int ddrc_csysreq_sft_cfg_sc2aocrg : 1;
        unsigned int reserved_0 : 3;
        unsigned int ddrc_csysreq_stat_aocrg2sc : 4;
        unsigned int ddrc_csysack_stat : 4;
        unsigned int reserved_1 : 16;
    } reg;
} SOC_SCTRL_SC_DDRC_CTRL_UNION;
#endif
#define SOC_SCTRL_SC_DDRC_CTRL_ddrc_csysreq_cfg_sc2aocrg_START (0)
#define SOC_SCTRL_SC_DDRC_CTRL_ddrc_csysreq_cfg_sc2aocrg_END (3)
#define SOC_SCTRL_SC_DDRC_CTRL_ddrc_csysreq_sft_cfg_sc2aocrg_START (4)
#define SOC_SCTRL_SC_DDRC_CTRL_ddrc_csysreq_sft_cfg_sc2aocrg_END (4)
#define SOC_SCTRL_SC_DDRC_CTRL_ddrc_csysreq_stat_aocrg2sc_START (8)
#define SOC_SCTRL_SC_DDRC_CTRL_ddrc_csysreq_stat_aocrg2sc_END (11)
#define SOC_SCTRL_SC_DDRC_CTRL_ddrc_csysack_stat_START (12)
#define SOC_SCTRL_SC_DDRC_CTRL_ddrc_csysack_stat_END (15)
#ifdef __cplusplus
#if __cplusplus
    }
#endif
#endif
#endif
