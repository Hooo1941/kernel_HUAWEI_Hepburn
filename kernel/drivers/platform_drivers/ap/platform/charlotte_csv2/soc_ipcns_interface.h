#ifndef __SOC_IPCNS_INTERFACE_H__
#define __SOC_IPCNS_INTERFACE_H__ 
#ifdef __cplusplus
#if __cplusplus
    extern "C" {
#endif
#endif
#ifndef __SOC_H_FOR_ASM__
#define SOC_IPCNS_MBX_SOURCE_ADDR(base,i) ((base) + (0x000+(i)*128UL))
#define SOC_IPCNS_MBX_DSET_ADDR(base,i) ((base) + (0x004+(i)*128UL))
#define SOC_IPCNS_MBX_DCLEAR_ADDR(base,i) ((base) + (0x008+(i)*128UL))
#define SOC_IPCNS_MBX_DSTATUS_ADDR(base,i) ((base) + (0x00C+(i)*128UL))
#define SOC_IPCNS_MBX_MODE_ADDR(base,i) ((base) + (0x010+(i)*128UL))
#define SOC_IPCNS_MBX_IMASK_ADDR(base,i) ((base) + (0x014+(i)*128UL))
#define SOC_IPCNS_MBX_ICLR_ADDR(base,i) ((base) + (0x018+(i)*128UL))
#define SOC_IPCNS_MBX_SEND_ADDR(base,i) ((base) + (0x01C+(i)*128UL))
#define SOC_IPCNS_MBX_DATA0_ADDR(base,i) ((base) + (0x020+(i)*128UL))
#define SOC_IPCNS_MBX_DATA1_ADDR(base,i) ((base) + (0x024+(i)*128UL))
#define SOC_IPCNS_MBX_DATA2_ADDR(base,i) ((base) + (0x028+(i)*128UL))
#define SOC_IPCNS_MBX_DATA3_ADDR(base,i) ((base) + (0x02C+(i)*128UL))
#define SOC_IPCNS_MBX_DATA4_ADDR(base,i) ((base) + (0x030+(i)*128UL))
#define SOC_IPCNS_MBX_DATA5_ADDR(base,i) ((base) + (0x034+(i)*128UL))
#define SOC_IPCNS_MBX_DATA6_ADDR(base,i) ((base) + (0x038+(i)*128UL))
#define SOC_IPCNS_MBX_DATA7_ADDR(base,i) ((base) + (0x03C+(i)*128UL))
#define SOC_IPCNS_MBX_DATA8_ADDR(base,i) ((base) + (0x040+(i)*128UL))
#define SOC_IPCNS_MBX_DATA9_ADDR(base,i) ((base) + (0x044+(i)*128UL))
#define SOC_IPCNS_MBX_DATA10_ADDR(base,i) ((base) + (0x048+(i)*128UL))
#define SOC_IPCNS_MBX_DATA11_ADDR(base,i) ((base) + (0x04C+(i)*128UL))
#define SOC_IPCNS_CPUJ_IMST_ADDR(base,j,n) ((base) + (0x8000+(j)*8+(n)*256UL))
#define SOC_IPCNS_CPUJ_IRST_ADDR(base,j,n) ((base) + (0x8004+(j)*8+(n)*256UL))
#define SOC_IPCNS_IPC_MBX_ACTIVE_ADDR(base,n) ((base) + (0x9000+(n)*4UL))
#define SOC_IPCNS_IPC_LP_STATE_ADDR(base) ((base) + (0x9024UL))
#define SOC_IPCNS_IPC_LOCK_ADDR(base) ((base) + (0xA000UL))
#else
#define SOC_IPCNS_MBX_SOURCE_ADDR(base,i) ((base) + (0x000+(i)*128))
#define SOC_IPCNS_MBX_DSET_ADDR(base,i) ((base) + (0x004+(i)*128))
#define SOC_IPCNS_MBX_DCLEAR_ADDR(base,i) ((base) + (0x008+(i)*128))
#define SOC_IPCNS_MBX_DSTATUS_ADDR(base,i) ((base) + (0x00C+(i)*128))
#define SOC_IPCNS_MBX_MODE_ADDR(base,i) ((base) + (0x010+(i)*128))
#define SOC_IPCNS_MBX_IMASK_ADDR(base,i) ((base) + (0x014+(i)*128))
#define SOC_IPCNS_MBX_ICLR_ADDR(base,i) ((base) + (0x018+(i)*128))
#define SOC_IPCNS_MBX_SEND_ADDR(base,i) ((base) + (0x01C+(i)*128))
#define SOC_IPCNS_MBX_DATA0_ADDR(base,i) ((base) + (0x020+(i)*128))
#define SOC_IPCNS_MBX_DATA1_ADDR(base,i) ((base) + (0x024+(i)*128))
#define SOC_IPCNS_MBX_DATA2_ADDR(base,i) ((base) + (0x028+(i)*128))
#define SOC_IPCNS_MBX_DATA3_ADDR(base,i) ((base) + (0x02C+(i)*128))
#define SOC_IPCNS_MBX_DATA4_ADDR(base,i) ((base) + (0x030+(i)*128))
#define SOC_IPCNS_MBX_DATA5_ADDR(base,i) ((base) + (0x034+(i)*128))
#define SOC_IPCNS_MBX_DATA6_ADDR(base,i) ((base) + (0x038+(i)*128))
#define SOC_IPCNS_MBX_DATA7_ADDR(base,i) ((base) + (0x03C+(i)*128))
#define SOC_IPCNS_MBX_DATA8_ADDR(base,i) ((base) + (0x040+(i)*128))
#define SOC_IPCNS_MBX_DATA9_ADDR(base,i) ((base) + (0x044+(i)*128))
#define SOC_IPCNS_MBX_DATA10_ADDR(base,i) ((base) + (0x048+(i)*128))
#define SOC_IPCNS_MBX_DATA11_ADDR(base,i) ((base) + (0x04C+(i)*128))
#define SOC_IPCNS_CPUJ_IMST_ADDR(base,j,n) ((base) + (0x8000+(j)*8+(n)*256))
#define SOC_IPCNS_CPUJ_IRST_ADDR(base,j,n) ((base) + (0x8004+(j)*8+(n)*256))
#define SOC_IPCNS_IPC_MBX_ACTIVE_ADDR(base,n) ((base) + (0x9000+(n)*4))
#define SOC_IPCNS_IPC_LP_STATE_ADDR(base) ((base) + (0x9024))
#define SOC_IPCNS_IPC_LOCK_ADDR(base) ((base) + (0xA000))
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int source : 13;
        unsigned int reserved : 19;
    } reg;
} SOC_IPCNS_MBX_SOURCE_UNION;
#endif
#define SOC_IPCNS_MBX_SOURCE_source_START (0)
#define SOC_IPCNS_MBX_SOURCE_source_END (12)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dset : 13;
        unsigned int reserved : 19;
    } reg;
} SOC_IPCNS_MBX_DSET_UNION;
#endif
#define SOC_IPCNS_MBX_DSET_dset_START (0)
#define SOC_IPCNS_MBX_DSET_dset_END (12)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dclear : 13;
        unsigned int reserved : 19;
    } reg;
} SOC_IPCNS_MBX_DCLEAR_UNION;
#endif
#define SOC_IPCNS_MBX_DCLEAR_dclear_START (0)
#define SOC_IPCNS_MBX_DCLEAR_dclear_END (12)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dstatus : 13;
        unsigned int reserved : 19;
    } reg;
} SOC_IPCNS_MBX_DSTATUS_UNION;
#endif
#define SOC_IPCNS_MBX_DSTATUS_dstatus_START (0)
#define SOC_IPCNS_MBX_DSTATUS_dstatus_END (12)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int auto_answer : 1;
        unsigned int auto_link : 1;
        unsigned int reserved_0 : 2;
        unsigned int state_status : 4;
        unsigned int reserved_1 : 24;
    } reg;
} SOC_IPCNS_MBX_MODE_UNION;
#endif
#define SOC_IPCNS_MBX_MODE_auto_answer_START (0)
#define SOC_IPCNS_MBX_MODE_auto_answer_END (0)
#define SOC_IPCNS_MBX_MODE_auto_link_START (1)
#define SOC_IPCNS_MBX_MODE_auto_link_END (1)
#define SOC_IPCNS_MBX_MODE_state_status_START (4)
#define SOC_IPCNS_MBX_MODE_state_status_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int int_mask : 13;
        unsigned int reserved : 19;
    } reg;
} SOC_IPCNS_MBX_IMASK_UNION;
#endif
#define SOC_IPCNS_MBX_IMASK_int_mask_START (0)
#define SOC_IPCNS_MBX_IMASK_int_mask_END (12)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int int_clear : 13;
        unsigned int reserved : 19;
    } reg;
} SOC_IPCNS_MBX_ICLR_UNION;
#endif
#define SOC_IPCNS_MBX_ICLR_int_clear_START (0)
#define SOC_IPCNS_MBX_ICLR_int_clear_END (12)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int send : 13;
        unsigned int reserved : 19;
    } reg;
} SOC_IPCNS_MBX_SEND_UNION;
#endif
#define SOC_IPCNS_MBX_SEND_send_START (0)
#define SOC_IPCNS_MBX_SEND_send_END (12)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int data0 : 32;
    } reg;
} SOC_IPCNS_MBX_DATA0_UNION;
#endif
#define SOC_IPCNS_MBX_DATA0_data0_START (0)
#define SOC_IPCNS_MBX_DATA0_data0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int data1 : 32;
    } reg;
} SOC_IPCNS_MBX_DATA1_UNION;
#endif
#define SOC_IPCNS_MBX_DATA1_data1_START (0)
#define SOC_IPCNS_MBX_DATA1_data1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int data2 : 32;
    } reg;
} SOC_IPCNS_MBX_DATA2_UNION;
#endif
#define SOC_IPCNS_MBX_DATA2_data2_START (0)
#define SOC_IPCNS_MBX_DATA2_data2_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int data3 : 32;
    } reg;
} SOC_IPCNS_MBX_DATA3_UNION;
#endif
#define SOC_IPCNS_MBX_DATA3_data3_START (0)
#define SOC_IPCNS_MBX_DATA3_data3_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int data4 : 32;
    } reg;
} SOC_IPCNS_MBX_DATA4_UNION;
#endif
#define SOC_IPCNS_MBX_DATA4_data4_START (0)
#define SOC_IPCNS_MBX_DATA4_data4_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int data5 : 32;
    } reg;
} SOC_IPCNS_MBX_DATA5_UNION;
#endif
#define SOC_IPCNS_MBX_DATA5_data5_START (0)
#define SOC_IPCNS_MBX_DATA5_data5_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int data6 : 32;
    } reg;
} SOC_IPCNS_MBX_DATA6_UNION;
#endif
#define SOC_IPCNS_MBX_DATA6_data6_START (0)
#define SOC_IPCNS_MBX_DATA6_data6_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int data7 : 32;
    } reg;
} SOC_IPCNS_MBX_DATA7_UNION;
#endif
#define SOC_IPCNS_MBX_DATA7_data7_START (0)
#define SOC_IPCNS_MBX_DATA7_data7_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int data8 : 32;
    } reg;
} SOC_IPCNS_MBX_DATA8_UNION;
#endif
#define SOC_IPCNS_MBX_DATA8_data8_START (0)
#define SOC_IPCNS_MBX_DATA8_data8_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int data9 : 32;
    } reg;
} SOC_IPCNS_MBX_DATA9_UNION;
#endif
#define SOC_IPCNS_MBX_DATA9_data9_START (0)
#define SOC_IPCNS_MBX_DATA9_data9_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int data10 : 32;
    } reg;
} SOC_IPCNS_MBX_DATA10_UNION;
#endif
#define SOC_IPCNS_MBX_DATA10_data10_START (0)
#define SOC_IPCNS_MBX_DATA10_data10_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int data11 : 32;
    } reg;
} SOC_IPCNS_MBX_DATA11_UNION;
#endif
#define SOC_IPCNS_MBX_DATA11_data11_START (0)
#define SOC_IPCNS_MBX_DATA11_data11_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int int_mask_status : 32;
    } reg;
} SOC_IPCNS_CPUJ_IMST_UNION;
#endif
#define SOC_IPCNS_CPUJ_IMST_int_mask_status_START (0)
#define SOC_IPCNS_CPUJ_IMST_int_mask_status_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int int_raw_status : 32;
    } reg;
} SOC_IPCNS_CPUJ_IRST_UNION;
#endif
#define SOC_IPCNS_CPUJ_IRST_int_raw_status_START (0)
#define SOC_IPCNS_CPUJ_IRST_int_raw_status_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int mbx_active_status : 32;
    } reg;
} SOC_IPCNS_IPC_MBX_ACTIVE_UNION;
#endif
#define SOC_IPCNS_IPC_MBX_ACTIVE_mbx_active_status_START (0)
#define SOC_IPCNS_IPC_MBX_ACTIVE_mbx_active_status_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int lp_state : 3;
        unsigned int reserved : 29;
    } reg;
} SOC_IPCNS_IPC_LP_STATE_UNION;
#endif
#define SOC_IPCNS_IPC_LP_STATE_lp_state_START (0)
#define SOC_IPCNS_IPC_LP_STATE_lp_state_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ipc_lock : 32;
    } reg;
} SOC_IPCNS_IPC_LOCK_UNION;
#endif
#define SOC_IPCNS_IPC_LOCK_ipc_lock_START (0)
#define SOC_IPCNS_IPC_LOCK_ipc_lock_END (31)
#ifdef __cplusplus
#if __cplusplus
    }
#endif
#endif
#endif
