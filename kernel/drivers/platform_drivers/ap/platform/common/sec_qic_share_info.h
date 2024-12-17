#ifndef __SEC_QIC_SHARE_INFO_H__
#define __SEC_QIC_SHARE_INFO_H__ 
#define QIC_SHARE_MEM_OFFSET 0xC00
#define QIC_SHARE_MEM_SIZE 0x400
#define MAX_QIC_DFX_INFO 10
#define MAX_QIC_DFX_NAME_LEN 24
enum qic_crg_type {
 PERI_CRG = 0,
 MEDIA1_CRG,
 MEDIA2_CRG,
};
struct sec_qic_dfx_info {
 unsigned int irq_status;
 unsigned int coreid;
 unsigned int bus_key;
 unsigned int ib_type;
 unsigned long long ib_dfx_low;
 unsigned long long ib_dfx_high;
 char ib_name[MAX_QIC_DFX_NAME_LEN];
 char tb_name[MAX_QIC_DFX_NAME_LEN];
};
#endif
