#ifndef __H_DOZE_H__
#define __H_DOZE_H__ 
enum DOZE_CLIENT {
 DOZE_CLIENT_STUB = 0,
 DOZE_CLIENT_USB,
 DOZE_CLIENT_VDEC,
 DOZE_CLIENT_DFA,
 DOZE_CLIENT_UART4,
 DOZE_CLIENT_UART5,
 DOZE_CLIENT_PCIE0,
 DOZE_CLIENT_PCIE1,
 DOZE_CLIENT_MSPE_REE,
 DOZE_CLIENT_DSS_UNSEC,
 DOZE_VOTE_UNSEC_MAX = 32,
 DOZE_CLIENT_DSS_SEC = DOZE_VOTE_UNSEC_MAX,
 DOZE_CLIENT_MODEM,
 DOZE_CLIENT_MSPC,
 DOZE_CLIENT_MSPE_TEE,
 DOZE_CLIENT_IOM3,
 DOZE_CLIENT_ASP,
 DOZE_VOTE_SEC_MAX = 64,
 DOZE_VOTE_MAX = DOZE_VOTE_SEC_MAX,
};
#define ONE_VOTE_REG_MAX_CLIENT 16
#define doze_vote_en(client) (0x10001U << (client))
#define doze_vote_dis(client) (0x10000U << (client))
#define SOC_SCTRL_SC_DOZE_VOTE0_ADDR(base) ((base) + 0x028UL)
#define SOC_SCTRL_SC_DOZE_VOTE2_ADDR(base) ((base) + 0x9A0UL)
#define SOC_SCTRL_SC_DOZE1_VOTE0_ADDR(base) ((base) + (0x028UL))
#define SOC_SCTRL_SC_DOZE1_VOTE2_ADDR(base) ((base) + (0x9A0UL))
#define SOC_SCTRL_SC_DOZE2_VOTE0_ADDR(base) ((base) + (0x040UL))
#define SOC_SCTRL_SC_DOZE2_VOTE2_ADDR(base) ((base) + (0x9A8UL))
#ifdef CONFIG_SYS_PM_DOZE
#if !defined(SOC_SCTRL_SC_DOZE_VOTE0_ADDR) && !defined(SOC_SCTRL_SC_DOZE1_VOTE0_ADDR)
#error "cannt support doze"
#endif
#if defined(__KERNEL__)
#define doze_vote_base 0
#elif defined(__LPMCU__)
#include <soc_lpmcu_baseaddr_interface.h>
#define doze_vote_base SOC_LPMCU_SCTRL_BASE_ADDR
#elif defined(__IOMCU__)
#include <soc_iomcu_baseaddr_interface.h>
#define doze_vote_base SOC_IOMCU_SCTRL_BASE_ADDR
#elif defined(__TEE__)
#include <soc_acpu_baseaddr_interface.h>
#define doze_vote_base SOC_ACPU_SCTRL_BASE_ADDR
#else
#error "not support this image!!!"
#endif
#ifdef CONFIG_SYS_PM_DOZE_INDEPEND_VOTE
#define REG_NUM_IN_EACH_TYPE 2
#define doze1_vote_addr_unsec SOC_SCTRL_SC_DOZE1_VOTE0_ADDR(doze_vote_base)
#define doze1_vote_addr_sec SOC_SCTRL_SC_DOZE1_VOTE2_ADDR(doze_vote_base)
#define doze2_vote_addr_unsec SOC_SCTRL_SC_DOZE2_VOTE0_ADDR(doze_vote_base)
#define doze2_vote_addr_sec SOC_SCTRL_SC_DOZE2_VOTE2_ADDR(doze_vote_base)
#else
#define REG_NUM_IN_EACH_TYPE 2
#define doze1_vote_addr_unsec SOC_SCTRL_SC_DOZE_VOTE0_ADDR(doze_vote_base)
#define doze1_vote_addr_sec SOC_SCTRL_SC_DOZE_VOTE2_ADDR(doze_vote_base)
#define doze2_vote_addr_unsec SOC_SCTRL_SC_DOZE_VOTE0_ADDR(doze_vote_base)
#define doze2_vote_addr_sec SOC_SCTRL_SC_DOZE_VOTE2_ADDR(doze_vote_base)
#endif
#if defined(__KERNEL__)
int vote_enable_doze1(enum DOZE_CLIENT client);
int vote_disable_doze1(enum DOZE_CLIENT client);
int vote_enable_doze2(enum DOZE_CLIENT client);
int vote_disable_doze2(enum DOZE_CLIENT client);
int vote_enable_doze(enum DOZE_CLIENT client);
int vote_disable_doze(enum DOZE_CLIENT client);
#elif defined(__LPMCU__) || defined(__IOMCU__) || defined(__TEE__)
static inline int vote_enable_doze1(enum DOZE_CLIENT client)
{
    unsigned int max_client_num = (unsigned int)ONE_VOTE_REG_MAX_CLIENT;
    unsigned int reg_num = (unsigned int)REG_NUM_IN_EACH_TYPE;
    unsigned int client_offset = ((unsigned int)(client)) % max_client_num;
    unsigned int reg_offset = (((unsigned int)(client)) / max_client_num) % reg_num;
    if (client >= DOZE_VOTE_MAX)
        return -1;
    else if (client < DOZE_VOTE_UNSEC_MAX)
        *((volatile unsigned int *)doze1_vote_addr_unsec + reg_offset) = doze_vote_en(client_offset);
    else
        *((volatile unsigned int *)doze1_vote_addr_sec + reg_offset) = doze_vote_en(client_offset);
    return 0;
}
static inline int vote_disable_doze1(enum DOZE_CLIENT client)
{
    unsigned int max_client_num = (unsigned int)ONE_VOTE_REG_MAX_CLIENT;
    unsigned int reg_num = (unsigned int)REG_NUM_IN_EACH_TYPE;
    unsigned int client_offset = ((unsigned int)(client)) % max_client_num;
    unsigned int reg_offset = (((unsigned int)(client)) / max_client_num) % reg_num;
    if (client >= DOZE_VOTE_MAX)
        return -1;
    else if (client < DOZE_VOTE_UNSEC_MAX)
        *((volatile unsigned int *)doze1_vote_addr_unsec + reg_offset) = doze_vote_dis(client_offset);
    else
        *((volatile unsigned int *)doze1_vote_addr_sec + reg_offset) = doze_vote_dis(client_offset);
    return 0;
}
static inline int vote_enable_doze2(enum DOZE_CLIENT client)
{
    unsigned int max_client_num = (unsigned int)ONE_VOTE_REG_MAX_CLIENT;
    unsigned int reg_num = (unsigned int)REG_NUM_IN_EACH_TYPE;
    unsigned int client_offset = ((unsigned int)(client)) % max_client_num;
    unsigned int reg_offset = (((unsigned int)(client)) / max_client_num) % reg_num;
    if (client >= DOZE_VOTE_MAX)
        return -1;
    else if (client < DOZE_VOTE_UNSEC_MAX)
        *((volatile unsigned int *)doze2_vote_addr_unsec + reg_offset) = doze_vote_en(client_offset);
    else
        *((volatile unsigned int *)doze2_vote_addr_sec + reg_offset) = doze_vote_en(client_offset);
    return 0;
}
static inline int vote_disable_doze2(enum DOZE_CLIENT client)
{
    unsigned int max_client_num = (unsigned int)ONE_VOTE_REG_MAX_CLIENT;
    unsigned int reg_num = (unsigned int)REG_NUM_IN_EACH_TYPE;
    unsigned int client_offset = ((unsigned int)(client)) % max_client_num;
    unsigned int reg_offset = (((unsigned int)(client)) / max_client_num) % reg_num;
    if (client >= DOZE_VOTE_MAX)
        return -1;
    else if (client < DOZE_VOTE_UNSEC_MAX)
        *((volatile unsigned int *)doze2_vote_addr_unsec + reg_offset) = doze_vote_dis(client_offset);
    else
        *((volatile unsigned int *)doze2_vote_addr_sec + reg_offset) = doze_vote_dis(client_offset);
    return 0;
}
static inline int vote_enable_doze(enum DOZE_CLIENT client)
{
    return vote_enable_doze1(client) + vote_enable_doze2(client);
}
static inline int vote_disable_doze(enum DOZE_CLIENT client)
{
    return vote_disable_doze1(client) + vote_disable_doze2(client);
}
#else
#error "not support this image!!!"
#endif
#else
static inline int vote_enable_doze1(enum DOZE_CLIENT client)
{
 (void)client;
 return 0;
}
static inline int vote_disable_doze1(enum DOZE_CLIENT client)
{
 (void)client;
 return 0;
}
static inline int vote_enable_doze2(enum DOZE_CLIENT client)
{
 (void)client;
 return 0;
}
static inline int vote_disable_doze2(enum DOZE_CLIENT client)
{
 (void)client;
 return 0;
}
static inline int vote_enable_doze(enum DOZE_CLIENT client)
{
 (void)client;
 return 0;
}
static inline int vote_disable_doze(enum DOZE_CLIENT client)
{
 (void)client;
 return 0;
}
#endif
#endif
