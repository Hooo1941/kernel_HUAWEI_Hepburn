#ifndef __QIC_INTERFACE_H__
#define __QIC_INTERFACE_H__ 
enum QIC_CORE_ID {
 QIC_ISP = 0x0,
 QIC_DSS,
 QIC_VENC,
 QIC_VDEC,
 QIC_IVP0,
 QIC_IVP1,
 QIC_IPP,
 QIC_ARPP,
 QIC_EPS,
 QIC_MDM,
 QIC_NPU_AICORE0,
 QIC_NPU_AICORE1,
 QIC_NPU_TS_0,
 QIC_NPU_TS_1,
 QIC_NPU_TCU,
 QIC_NPU_SYS2NPU,
 QIC_NPU_SYSDMA_0,
 QIC_NPU_SYSDMA_1,
 QIC_IOMCU,
 QIC_IOMCU_DMA,
 QIC_ASP,
 QIC_CORE_ID_MAX,
};
struct dfx_errlog_info {
 unsigned int err_addr;
 unsigned int master_id;
 unsigned int opc_type;
 unsigned int safe_flag;
};
#if defined(CONFIG_DFX_SEC_QIC_V100_MODID_REGISTER) || defined(CONFIG_DFX_QIC_MODID_REGISTER)
void dfx_qic_modid_register(unsigned int qic_coreid, unsigned int modid);
#else
static inline void dfx_qic_modid_register(unsigned int qic_coreid, unsigned int modid)
{
    (void)(qic_coreid);
    (void)(modid);
}
#endif
#if defined(CONFIG_DFX_SEC_QIC_V100_MODID_REGISTER)
void dfx_qic_get_errlog_info(struct dfx_errlog_info *errlog_info);
#else
static inline void dfx_qic_get_errlog_info(struct dfx_errlog_info *errlog_info)
{
    (void)(errlog_info);
}
#endif
#endif
