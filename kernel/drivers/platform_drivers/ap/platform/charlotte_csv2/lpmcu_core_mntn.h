#ifndef LPMCU_CORE_MNTN_H
#define LPMCU_CORE_MNTN_H 
#define CONFIG_LPMCU_MNTN_RISCV 1
enum {
 RDR_BACKUP_IDEX0 = 0,
 RDR_BACKUP_IDEX1,
 RDR_BACKUP_IDEX2,
 RDR_BACKUP_IDEX3,
 RDR_BACKUP_IDEX_MAX
};
struct exc_spec_data {
 unsigned char reset_reason[RDR_BACKUP_IDEX_MAX];
 unsigned int slice;
 unsigned int rtc;
 unsigned int lr;
 unsigned int pc;
 unsigned int sp;
 unsigned int mcause;
 unsigned char exc_trace;
 unsigned char ddr_exc;
 unsigned short irq_id;
 unsigned int task_id;
 unsigned int pc_async;
 unsigned int lr_async;
 unsigned int mtval;
};
#define RISCV_REG_NUM 32
struct rdr_lpmcu_cregs {
 unsigned int x[RISCV_REG_NUM];
 unsigned int mepc;
 unsigned int mcause;
 unsigned int mtval;
 unsigned int mstatus;
 unsigned int pc;
 unsigned int sp;
 unsigned int cxcptsc;
};
struct rdr_reg_backup {
 unsigned int idex;
 unsigned int reason[RDR_BACKUP_IDEX_MAX];
 struct rdr_lpmcu_cregs core_regs[RDR_BACKUP_IDEX_MAX];
};
#endif
