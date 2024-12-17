/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description: hcc bus excpetion,address range for generating the memdump
 * Author: @CompanyNameTag
 */

#ifndef OAL_HCC_BUS_EXCEPTION_H
#define OAL_HCC_BUS_EXCEPTION_H

/* 其他头文件包含 */
#include "plat_type.h"
#include "plat_exception_rst.h"

/* 宏定义 */
/* pilot config **/
#define WCPU_MP13_PILOT_PANIC_MEMDUMP_MAX_SIZE 0x3c00 /* 15KB */
#define WCPU_MP13_PILOT_PANIC_MEMDUMP_MAX_ADDR 0x60080000
/* 0x6007c400 */
#define WCPU_MP13_PILOT_PANIC_MEMDUMP_STORE_ADDR \
    (WCPU_MP13_PILOT_PANIC_MEMDUMP_MAX_ADDR - WCPU_MP13_PILOT_PANIC_MEMDUMP_MAX_SIZE)

#define WCPU_MP13_PILOT_TICM_RAM_MAX_SIZE 0x98000 /* 608KB */
#define WCPU_MP13_PILOT_TICM_RAM_MAX_ADDR 0xa8000
/* 0x10000 */
#define WCPU_MP13_PILOT_TICM_RAM_STORE_ADDR (WCPU_MP13_PILOT_TICM_RAM_MAX_ADDR - WCPU_MP13_PILOT_TICM_RAM_MAX_SIZE)

#define WCPU_MP13_PILOT_DTCM_RAM_MAX_SIZE 0x68000 /* 416KB */
#define WCPU_MP13_PILOT_DTCM_RAM_MAX_ADDR 0x20080000
/* 0x20018000 */
#define WCPU_MP13_PILOT_DTCM_RAM_STORE_ADDR (WCPU_MP13_PILOT_DTCM_RAM_MAX_ADDR - WCPU_MP13_PILOT_DTCM_RAM_MAX_SIZE)

#define WCPU_MP13_PILOT_PKT_MEM_MAX_SIZE 0x80000 /* 512KB */
#define WCPU_MP13_PILOT_PKT_MEM_MAX_ADDR 0x60080000
/* 0x60000000 */
#define WCPU_MP13_PILOT_PKT_MEM_STORE_ADDR (WCPU_MP13_PILOT_PKT_MEM_MAX_ADDR - WCPU_MP13_PILOT_PKT_MEM_MAX_SIZE)
OAL_STATIC struct st_wifi_dump_mem_info g_mp13_pilot_meminfo[] = {
    {
        .file_name = "wifi_device_panic_mem",
        .mem_addr = WCPU_MP13_PILOT_PANIC_MEMDUMP_STORE_ADDR,
        .size = WCPU_MP13_PILOT_PANIC_MEMDUMP_MAX_SIZE, /* 15KB */
    },
    {
        .file_name = "wifi_device_itcm_ram",
        .mem_addr = WCPU_MP13_PILOT_TICM_RAM_STORE_ADDR,
        .size = WCPU_MP13_PILOT_TICM_RAM_MAX_SIZE, /* 608KB */
    },
    {
        .file_name = "wifi_device_dtcm_ram",
        .mem_addr = WCPU_MP13_PILOT_DTCM_RAM_STORE_ADDR,
        .size = WCPU_MP13_PILOT_DTCM_RAM_MAX_SIZE, /* 416KB */
    },
    {
        .file_name = "wifi_device_pkt_mem",
        .mem_addr = WCPU_MP13_PILOT_PKT_MEM_STORE_ADDR,
        .size = WCPU_MP13_PILOT_PKT_MEM_MAX_SIZE, /* 512KB */
    },

};

/* for mp15 */
#define WCPU_MP15_ASIC_PANIC_MEMDUMP_MAX_SIZE 0x3c00 /* 15KB */
#define WCPU_MP15_ASIC_PANIC_MEMDUMP_MAX_ADDR 0x60080000
/* 0x6007c400 */
#define WCPU_MP15_ASIC_PANIC_MEMDUMP_STORE_ADDR \
    (WCPU_MP15_ASIC_PANIC_MEMDUMP_MAX_ADDR - WCPU_MP15_ASIC_PANIC_MEMDUMP_MAX_SIZE)

#define WCPU_MP15_ASIC_ITCM_RAM_MAX_SIZE 0xb0000 /* 640+64KB */
#define WCPU_MP15_ASIC_ITCM_RAM_MAX_ADDR 0xc0000
/* 0x10000 */
#define WCPU_MP15_ASIC_ITCM_RAM_STORE_ADDR (WCPU_MP15_ASIC_ITCM_RAM_MAX_ADDR - WCPU_MP15_ASIC_ITCM_RAM_MAX_SIZE)

#define WCPU_MP15_ASIC_DTCM_RAM_MAX_SIZE 0x68000 /* 416KB */
#define WCPU_MP15_ASIC_DTCM_RAM_MAX_ADDR 0x20080000
/* 0x20018000 */
#define WCPU_MP15_ASIC_DTCM_RAM_STORE_ADDR (WCPU_MP15_ASIC_DTCM_RAM_MAX_ADDR - WCPU_MP15_ASIC_DTCM_RAM_MAX_SIZE)

#define WCPU_MP15_ASIC_PKT_MEM_MAX_SIZE 0x100000 /* 1024KB */
#define WCPU_MP15_ASIC_PKT_MEM_MAX_ADDR 0x60100000
/* 0x60000000 */
#define WCPU_MP15_ASIC_PKT_MEM_STORE_ADDR (WCPU_MP15_ASIC_PKT_MEM_MAX_ADDR - WCPU_MP15_ASIC_PKT_MEM_MAX_SIZE)
OAL_STATIC struct st_wifi_dump_mem_info g_mp15_pilot_asic_meminfo[] = {
    {
        .file_name = "wifi_device_panic_mem",
        .mem_addr = WCPU_MP15_ASIC_PANIC_MEMDUMP_STORE_ADDR,
        .size = WCPU_MP15_ASIC_PANIC_MEMDUMP_MAX_SIZE, /* 15KB */
    },
    {
        .file_name = "wifi_device_itcm_ram",
        .mem_addr = WCPU_MP15_ASIC_ITCM_RAM_STORE_ADDR,
        .size = WCPU_MP15_ASIC_ITCM_RAM_MAX_SIZE, /* 640+64KB */
    },
    {
        .file_name = "wifi_device_dtcm_ram",
        .mem_addr = WCPU_MP15_ASIC_DTCM_RAM_STORE_ADDR,
        .size = WCPU_MP15_ASIC_DTCM_RAM_MAX_SIZE, /* 416KB */
    },
    {
        .file_name = "wifi_device_pkt_mem",
        .mem_addr = WCPU_MP15_ASIC_PKT_MEM_STORE_ADDR,
        .size = WCPU_MP15_ASIC_PKT_MEM_MAX_SIZE, /* 512*2KB */
    },
};

#define WCPU_MP15_FPGA_PANIC_MEMDUMP_MAX_SIZE 0x3c00 /* 15KB */
#define WCPU_MP15_FPGA_PANIC_MEMDUMP_MAX_ADDR 0x60080000
/* 0x6007c400 */
#define WCPU_MP15_FPGA_PANIC_MEMDUMP_STORE_ADDR \
    (WCPU_MP15_FPGA_PANIC_MEMDUMP_MAX_ADDR - WCPU_MP15_FPGA_PANIC_MEMDUMP_MAX_SIZE)

#define WCPU_MP15_FPGA_TICM_RAM_MAX_SIZE 0xa0000 /* 640KB */
#define WCPU_MP15_FPGA_TICM_RAM_MAX_ADDR 0xb0000
/* 0x10000 */
#define WCPU_MP15_FPGA_TICM_RAM_STORE_ADDR (WCPU_MP15_FPGA_TICM_RAM_MAX_ADDR - WCPU_MP15_FPGA_TICM_RAM_MAX_SIZE)

#define WCPU_MP15_FPGA_DTCM_RAM_MAX_SIZE 0x68000 /* 416KB */
#define WCPU_MP15_FPGA_DTCM_RAM_MAX_ADDR 0x20080000
/* 0x20018000 */
#define WCPU_MP15_FPGA_DTCM_RAM_STORE_ADDR (WCPU_MP15_FPGA_DTCM_RAM_MAX_ADDR - WCPU_MP15_FPGA_DTCM_RAM_MAX_SIZE)

#define WCPU_MP15_FPGA_PKT_MEM_MAX_SIZE 0x80000 /* 512KB */
#define WCPU_MP15_FPGA_PKT_MEM_MAX_ADDR 0x60080000
/* 0x60000000 */
#define WCPU_MP15_FPGA_PKT_MEM_STORE_ADDR (WCPU_MP15_FPGA_PKT_MEM_MAX_ADDR - WCPU_MP15_FPGA_PKT_MEM_MAX_SIZE)
OAL_STATIC struct st_wifi_dump_mem_info g_mp15_pilot_fpga_meminfo[] = {
    {
        .file_name = "wifi_device_panic_mem",
        .mem_addr = WCPU_MP15_FPGA_PANIC_MEMDUMP_STORE_ADDR,
        .size = WCPU_MP15_FPGA_PANIC_MEMDUMP_MAX_SIZE, /* 15KB */
    },
    {
        .file_name = "wifi_device_itcm_ram",
        .mem_addr = WCPU_MP15_FPGA_TICM_RAM_STORE_ADDR,
        .size = WCPU_MP15_FPGA_TICM_RAM_MAX_SIZE, /* 640KB */
    },
    {
        .file_name = "wifi_device_dtcm_ram",
        .mem_addr = WCPU_MP15_FPGA_DTCM_RAM_STORE_ADDR,
        .size = WCPU_MP15_FPGA_DTCM_RAM_MAX_SIZE, /* 416KB */
    },
    {
        .file_name = "wifi_device_pkt_mem",
        .mem_addr = WCPU_MP15_FPGA_PKT_MEM_STORE_ADDR,
        .size = WCPU_MP15_FPGA_PKT_MEM_MAX_SIZE, /* 512KB */
    },
};

/* for mp16 */
#define MP16_WL_WRAM_BASEADDR         0x40000
#define MP16_WL_WRAM_LEN              0x170000

#define MP16_WL_S6_SHARE_RAM_BASEADDR 0x2000000
#define MP16_WL_SHARE_RAM_LEN         0xA2000

#define GF61_WL_ITCM_BASEADDR         0x0
#define GF61_WL_ITCM_LEN              0xC8000 /* 800KB */
#define GF61_WL_PKTMEM_BASEADDR       0x2000000
#define GF61_WL_PKTMEM_LEN            0x80000 /* 512KB */
#define GF61_WL_DTCM_BASEADDR         0x20000000
#define GF61_WL_DTCM_LEN              0x80000 /* 512KB */

#define GF61_GT_RAM_BASEADDR          0x50008000
#define GF61_GT_RAM_LEN               0xF8000 /* 992KB */
#define GF61_GT_PKTMEM_BASEADDR       0x50100000
#define GF61_GT_PKTMEM_LEN            0x80000 /* 512KB */

OAL_STATIC struct st_wifi_dump_mem_info g_mp16_pilot_fpga_meminfo[] = {
    {
        /* mp16 对应WSRAM,名字需要修改对应oam_hisi */
        .file_name = "wifi_device_itcm_ram",
        .mem_addr = MP16_WL_WRAM_BASEADDR,
        .size = MP16_WL_WRAM_LEN, /* 1472KB */
    },
    {
        .file_name = "wifi_device_pkt_mem",
        .mem_addr = MP16_WL_S6_SHARE_RAM_BASEADDR,
        .size = MP16_WL_SHARE_RAM_LEN, /* 648KB */
    },
};

OAL_STATIC struct st_wifi_dump_mem_info g_mp16_pilot_asic_meminfo[] = {
    {
        /* mp16 对应WSRAM,名字需要修改对应oam_hisi */
        .file_name = "wifi_device_itcm_ram",
        .mem_addr = MP16_WL_WRAM_BASEADDR,
        .size = MP16_WL_WRAM_LEN, /* 1472KB */
    },
    {
        .file_name = "wifi_device_pkt_mem",
        .mem_addr = MP16_WL_S6_SHARE_RAM_BASEADDR,
        .size = MP16_WL_SHARE_RAM_LEN, /* 648KB */
    },
};

/* for mp16c memdump */
#define WCPU_MP16C_PANIC_MEMDUMP_SAVE_ADDR             0x201BE00
#define WCPU_MP16C_PANIC_MEMDUMP_SIZE                  0x4200  /* 16.5KB */

#define WCPU_MP16C_TRACE_MEMDUMP_SAVE_ADDR             0x400e6000
#define WCPU_MP16C_TRACE_MEMDUMP_SIZE                  0x2000  /* 8KB */

#define WCPU_MP16C_ITCM_RAM_MEMDUMP_SAVE_ADDR          0x4000
#define WCPU_MP16C_ITCM_RAM_MEMDUMP_SIZE               0xDC000 /* 880KB */

#define WCPU_MP16C_DTCM_RAM_MEMDUMP_SAVE_ADDR          0x20000000
#define WCPU_MP16C_DTCM_RAM_MEMDUMP_SIZE               0x80000 /* 512KB */

#define WCPU_MP16C_PKT_MEM_MEMDUMP_SAVE_ADDR           0x02000000
#define WCPU_MP16C_PKT_MEM_MEMDUMP_SIZE                0x80000 /* 512KB */

OAL_STATIC struct st_wifi_dump_mem_info g_mp16c_meminfo[] = {
    {
        .file_name = "wifi_device_panic_mem",
        .mem_addr = WCPU_MP16C_PANIC_MEMDUMP_SAVE_ADDR,
        .size = WCPU_MP16C_PANIC_MEMDUMP_SIZE, /* 16.5KB */
    },
    {
        .file_name = "wifi_device_cpu_trace_mem",
        .mem_addr = WCPU_MP16C_TRACE_MEMDUMP_SAVE_ADDR,
        .size = WCPU_MP16C_TRACE_MEMDUMP_SIZE, /* 8KB */
    },
    {
        .file_name = "wifi_device_itcm_ram",
        .mem_addr = WCPU_MP16C_ITCM_RAM_MEMDUMP_SAVE_ADDR,
        .size = WCPU_MP16C_ITCM_RAM_MEMDUMP_SIZE, /* 880KB */
    },
    {
        .file_name = "wifi_device_dtcm_ram",
        .mem_addr = WCPU_MP16C_DTCM_RAM_MEMDUMP_SAVE_ADDR,
        .size = WCPU_MP16C_DTCM_RAM_MEMDUMP_SIZE, /* 512KB */
    },
    {
        .file_name = "wifi_device_pkt_mem",
        .mem_addr = WCPU_MP16C_PKT_MEM_MEMDUMP_SAVE_ADDR,
        .size = WCPU_MP16C_PKT_MEM_MEMDUMP_SIZE, /* 512KB */
    },
};

/* for gf61 memdump */
#define WCPU_GF61_PANIC_MEMDUMP_SAVE_ADDR             0x201BE00
#define WCPU_GF61_PANIC_MEMDUMP_SIZE                  0x4200  /* 16.5KB */

#define WCPU_GF61_ITCM_RAM_MEMDUMP_SAVE_ADDR          0x0000
#define WCPU_GF61_ITCM_RAM_MEMDUMP_SIZE               0xC8000 /* 800KB */

#define WCPU_GF61_DTCM_RAM_MEMDUMP_SAVE_ADDR          0x20000000
#define WCPU_GF61_DTCM_RAM_MEMDUMP_SIZE               0x80000 /* 512KB */

#define WCPU_GF61_PKT_MEM_MEMDUMP_SAVE_ADDR           0x02000000
#define WCPU_GF61_PKT_MEM_MEMDUMP_SIZE                0x80000 /* 512KB */


OAL_STATIC struct st_wifi_dump_mem_info g_gf61_pilot_wifi_meminfo[] = {
    /* gf61 对应名字需要修改对应oam_hisi */
    {
        .file_name = "wifi_device_panic_mem",
        .mem_addr = WCPU_GF61_PANIC_MEMDUMP_SAVE_ADDR,
        .size = WCPU_GF61_PANIC_MEMDUMP_SIZE, /* 16.5KB */
    },
    {
        .file_name = "wifi_device_itcm_ram",
        .mem_addr = WCPU_GF61_ITCM_RAM_MEMDUMP_SAVE_ADDR,
        .size = WCPU_GF61_ITCM_RAM_MEMDUMP_SIZE, /* 800KB */
    },
        {
        .file_name = "wifi_device_dtcm_ram",
        .mem_addr = WCPU_GF61_DTCM_RAM_MEMDUMP_SAVE_ADDR,
        .size = WCPU_GF61_DTCM_RAM_MEMDUMP_SIZE, /* 512KB */
    },
    {
        .file_name = "wifi_device_pkt_mem",
        .mem_addr = WCPU_GF61_PKT_MEM_MEMDUMP_SAVE_ADDR,
        .size = WCPU_GF61_PKT_MEM_MEMDUMP_SIZE, /* 512KB */
    }
};

OAL_STATIC struct st_wifi_dump_mem_info g_gf61_pilot_gt_meminfo[] = {
    {
        .file_name = "gt_device_itcm_ram",
        .mem_addr = GF61_GT_RAM_BASEADDR,
        .size = GF61_GT_RAM_LEN,
    },
    {
        .file_name = "gt_device_pkt_mem",
        .mem_addr = GF61_GT_PKTMEM_BASEADDR,
        .size = GF61_GT_PKTMEM_LEN,
    },
};

#endif /* end of oal_hcc_bus_exception.h */
