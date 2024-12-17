/******************************************************************
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2018. All rights reserved.
 * File name  : ipp.h
 * Description:
 *      This ipp.h should only store the macros and enum that can represent
 *      the attributes of each module in this platform.
 *      It's a base head file; The file should not contain other header files except
 *      standard library files.
 * Date       :2022-08-01 16:28:10
 ********************************************************************/

#ifndef _IPP_CS_H_
#define _IPP_CS_H_

#include <linux/types.h>
#include <linux/stddef.h>

#define ISP_IPP_OK                (0)
#define ISP_IPP_ERR               (-1)
#define CVDR_ALIGN_BYTES          (16)  // bytes
#define MAX_CPE_STRIPE_NUM        (25)  // max orb = 22;need review
#define ISP_IPP_CLK               (0)
#define MAP_ALGO_BUF_MAX_SIZE     (0x0F600000) // 257M for cpe_test

enum HISP_CPE_REG_TYPE {
	CPE_TOP,
	CMDLIST_REG,
	CVDR_REG,
	MC_REG,
	KLT_REG,
	ARF_REG,
	REORDER_REG,
	ORB_ENH,
	COMPARE_REG,
	MAX_HISP_CPE_REG
};

int hispcpe_reg_set(unsigned int mode, unsigned int offset, unsigned int value);
unsigned int hispcpe_reg_get(unsigned int mode, unsigned int offset);

#define DEBUG_BIT	(1 << 2)
#define INFO_BIT	(1 << 1)
#define ERROR_BIT	(1 << 0)

#define LOGLEVEL ((INFO_BIT) | (ERROR_BIT)) /* DEBUG_BIT not set */

#define logd(fmt, args...) \
	do { if (LOGLEVEL & DEBUG_BIT) printk("[ispcpe][%s,%d]" fmt "\n", __func__, __LINE__, ##args); } while (0)

#define logi(fmt, args...) \
	do { if (LOGLEVEL & INFO_BIT) printk("[ispcpe][%s,%d]" fmt "\n", __func__, __LINE__, ##args); } while (0)

#define loge(fmt, args...) \
	do { if (LOGLEVEL & ERROR_BIT) printk("[ispcpe][%s,%d]" fmt "\n", __func__, __LINE__, ##args); } while (0)

#ifndef align_down
#define align_down(val, al)  ((unsigned int)(val) & ~((al) - 1))
#endif

#ifndef align_up
#define align_up(val, al) (((unsigned int)(val) + ((al) - 1)) & ~((al) - 1))
#endif

#ifndef align_long_up
#define align_long_up(val, al)	(((unsigned long long)(val) \
	+ (((unsigned long long)(al)) - 1)) & ~(((unsigned long long)(al)) - 1))
#endif

#define loge_if(x) { \
	if (x) { \
		pr_err("'%s' failed\n", #x); \
	} \
}

#define loge_if_null_set_ret(para, ret) \
	do { \
		ret = ISP_IPP_OK; \
		if ((para) == NULL) { \
			pr_err("[ispcpe][%s,%d]Failed: '%s' is NULL\n",__func__, __LINE__, #para); \
			ret = ISP_IPP_ERR; \
		} \
	} while(0)

#define loge_if_set_ret(para, ret) \
	do { \
		ret = ISP_IPP_OK; \
		if (para) { \
			pr_err("[ispcpe][%s,%d]Failed: The '%s' returns an error\n",__func__, __LINE__, #para); \
			ret = ISP_IPP_ERR; \
		} \
	} while(0)

#define MAX_ARRAY_LINE_SIZE 450
#define log_dump_array(indicate, format, target_lut, len, stride) \
	do { \
		char logbuff[MAX_ARRAY_LINE_SIZE] = {0}; \
		unsigned int pos = 0;  \
		unsigned int ii   = 0;  \
		unsigned int tmpStride = stride; \
		logi(indicate); \
		for (ii = 0; ii < (len); ii++) { \
			if(ii !=0 && ii%(tmpStride) == 0){ \
				logi("%s", logbuff); \
				memset_s(logbuff, MAX_ARRAY_LINE_SIZE, 0, MAX_ARRAY_LINE_SIZE); \
				pos = 0; \
			}\
			pos += sprintf_s(logbuff + pos, MAX_ARRAY_LINE_SIZE - pos, format, (target_lut)[ii]); \
		} \
		logi("%s", logbuff); \
	} while (0)


// Has been updated for 02. 2022 07 07
#define JPG_SUBSYS_BASE_ADDR    0x48C00000
#define JPG_TOP_OFFSET          0x00004000
#define JPG_CMDLST_OFFSET       0x00005000
#define JPG_CVDR_OFFSET         0x00006000
#define JPG_MC_OFFSET           0x00008000
#define JPG_KLT_OFFSET          0x0000A000
#define JPG_ARF_OFFSET          0x0000C000
#define JPG_REORDER_OFFSET      0x0000E000
#define JPG_ORB_ENH_OFFSET      0x00010000
#define JPG_COMPARE_OFFSET      0x00014000
#define IPP_BASE_ADDR_TOP       (JPG_SUBSYS_BASE_ADDR + JPG_TOP_OFFSET)
#define IPP_BASE_ADDR_CMDLST    (JPG_SUBSYS_BASE_ADDR + JPG_CMDLST_OFFSET)
#define IPP_BASE_ADDR_CVDR      (JPG_SUBSYS_BASE_ADDR + JPG_CVDR_OFFSET)
#define IPP_BASE_ADDR_MC        (JPG_SUBSYS_BASE_ADDR + JPG_MC_OFFSET)
#define IPP_BASE_ADDR_KLT       (JPG_SUBSYS_BASE_ADDR + JPG_KLT_OFFSET)
#define IPP_BASE_ADDR_ARF       (JPG_SUBSYS_BASE_ADDR + JPG_ARF_OFFSET)
#define IPP_BASE_ADDR_REORDER   (JPG_SUBSYS_BASE_ADDR + JPG_REORDER_OFFSET)
#define IPP_BASE_ADDR_ORB_ENH   (JPG_SUBSYS_BASE_ADDR + JPG_ORB_ENH_OFFSET)
#define IPP_BASE_ADDR_COMPARE   (JPG_SUBSYS_BASE_ADDR + JPG_COMPARE_OFFSET)

#define PCIE_IPP_IRQ_TAG        (0)

enum mem_attr_type_e {
	MMU_READ   = 0,
	MMU_WRITE  = 1,
	MMU_EXEC   = 2,
	MMU_SEC    = 3,
	MMU_CACHE  = 4,
	MMU_DEVICE = 5,
	MMU_INV    = 6,
	MAX_MMUATTR,
};

enum ipp_clk_rate_index_e {
	CLK_RATE_INDEX_TURBO = 0,
	CLK_RATE_INDEX_NORMAL,
	CLK_RATE_INDEX_HSVS,
	CLK_RATE_INDEX_SVS,
	CLK_RATE_INDEX_MAX,
};

#define MAX_HIPP_REFS_CNT    (3)
enum hipp_refs_type_e {
	REFS_TYP_EARCMK = 0,
	REFS_TYP_MAX,
};

#ifdef IPP_UT_DIO_DEBUG // This part is used to print DIO during the UT test.
#define UT_REG_ADDR	         IPP_BASE_ADDR_COMPARE // the start reg address of the current module;
#define IPP_MAX_REG_OFFSET	 0x28 // the Maximum offset address of the current module
#define MAX_DUMP_STRIPE_CNT	 10

extern void set_dump_register_init(unsigned int addr, unsigned int offset, unsigned int input_fmt);
extern void get_register_info(unsigned int reg, unsigned int val);
extern void get_register_incr_info(unsigned int reg, unsigned int reg_num);
extern void get_register_incr_data(unsigned int data);
extern void ut_dump_register(unsigned int stripe_cnt);
#endif

/* *Tag_1 :Stores the hardware attribute information of the current version.* */
#define UNSIGNED_INT_MAX                           0xffffffff
#define LOG_NAME_LEN                               64
#define STRIPE_NUM_EACH_RDR                        1 // 1 stripe: only cfg rdr

#define ARFEATURE_MAX_INPUT_IMG_W_SIZE             (1024)
#define ARFEATURE_MAX_INPUT_IMG_H_SIZE             (1024)
#define ARFEATURE_MIN_INPUT_IMG_W_SIZE             (80)
#define ARFEATURE_MIN_INPUT_IMG_H_SIZE             (60)
#define ARF_DESCRIPTOR_SIZE                        (144)
#define MATCHER_KPT_MAX                            (2047) // the maximum amount of data that the rdr/cmp can process.
#define ARF_OUT_DESC_SIZE                          (144*MATCHER_KPT_MAX + 0x8000)
#define ARF_OUT_COORDINATE_SIZE                    (500*(64/8)) // bytes

#define RDR_OUT_DESC_SIZE                          (160*MATCHER_KPT_MAX + 0x8000)
#define RDR_BLK_H_NUM_MAX                          (16)
#define RDR_BLK_V_NUM_MAX                          (16)
#define RDR_BLK_NUM_MAX                            (RDR_BLK_H_NUM_MAX*RDR_BLK_V_NUM_MAX)

#define CMP_IN_INDEX_NUM                           RDR_OUT_DESC_SIZE
#define CMP_BLK_H_NUM_MAX                          (16)
#define CMP_BLK_V_NUM_MAX                          (16)
#define CMP_BLK_NUM_MAX                            (CMP_BLK_H_NUM_MAX*CMP_BLK_V_NUM_MAX)
#define CMP_INDEX_RANGE_MAX                        (1600)

#define ARF_DMAP_MODE_CNT                          3
#define ARF_DETECTION_MODE                         4
#define KLT_KP_NUM_MAX                             500
#define MAX_WIDTH_ENH_INPUT_IMG_SIZE               (1024)
#define MAX_HEIGHT_ENH_INPUT_IMG_SIZE              (1024)
#define MC_CFG_MODE_INDEX_PAIRS                    (0)
#define MC_CFG_MODE_COORD_PAIRS                    (1)
#define MC_SVD_ITERATIONS_MIN                      (2)
#define MC_SVD_ITERATIONS_MAX                      (30)
#define MC_H_MATRIX_ITERATIONS_MIN                 (1)
#define MC_H_MATRIX_ITERATIONS_MAX                 (4095)
#define MC_COORDINATE_PAIRS                        (1600 * 2)
#define MC_OUT_RESULT_SIZE                         (8 + (9 * 8) + (1200 * 8))  // HEAD + H_MATRIX + MAX_inler_num


/* Start * ---> IPP IRQ BIT */
// orb_enh irq
#define IPP_ORB_ENH_DONE                            0
#define IPP_ORB_ENH_CVDR_VP_WR_EOF_YGUASS           1
#define IPP_ORB_ENH_CVDR_VP_RD_EOF_YHIST            2
#define IPP_ORB_ENH_CVDR_VP_RD_EOF_YIMG             3
#define IPP_ORB_ENH_CVDR_VP_WR_EOF_CMDLST           4
#define IPP_ORB_ENH_CVDR_VP_RD_EOF_CMDLST           5
#define IPP_ORB_ENH_CVDR_VP_WR_SOF_YGUASS           6
#define IPP_ORB_ENH_CVDR_VP_WR_DROPPED_YGUASS       7
#define IPP_ORB_ENH_CVDR_VP_RD_SOF_YHIST            8
#define IPP_ORB_ENH_CVDR_VP_RD_SOF_YIMAGE           9

// arfeature irq
#define IPP_ENH_DONE_IRQ                            0
#define IPP_ARF_DONE_IRQ                            1
#define IPP_ARF_CVDR_VP_WR_EOF_PRY_1                2 // OUT_0
#define IPP_ARF_CVDR_VP_WR_EOF_PRY_2                3 // OUT_1
#define IPP_ARF_CVDR_VP_WR_EOF_DOG_0                4 // OUT_2
#define IPP_ARF_CVDR_VP_WR_EOF_DOG_1                5 // OUT_3
#define IPP_ARF_CVDR_VP_WR_EOF_DOG_2                6 // OUT_4
#define IPP_ARF_CVDR_VP_WR_EOF_DOG_3                7 // OUT_5
#define IPP_ARF_CVDR_VP_WR_EOF_PRY_2_DS             8 // OUT_6
#define IPP_ARF_CVDR_VP_RD_EOF_IN_0                 9 // ORG_IMG and the PRY 1 data
#define IPP_ARF_CVDR_VP_RD_EOF_IN_1                 10 // PRY_2
#define IPP_ARF_CVDR_VP_RD_EOF_IN_2                 11 // DOG_1
#define IPP_ARF_CVDR_VP_RD_EOF_IN_3                 12 // DOG_2
#define IPP_ARF_CVDR_VP_WR_EOF_CMDLST               13
#define IPP_ARF_CVDR_VP_RD_EOF_CMDLST               14
#define IPP_ARF_CVDR_VP_WR_SOF_OUT_0                15
#define IPP_ARF_CVDR_VP_WR_SOF_OUT_1                16
#define IPP_ARF_CVDR_VP_WR_SOF_OUT_2                17
#define IPP_ARF_CVDR_VP_WR_SOF_OUT_3                18
#define IPP_ARF_CVDR_VP_WR_SOF_OUT_4                19
#define IPP_ARF_CVDR_VP_WR_SOF_OUT_5                20
#define IPP_ARF_CVDR_VP_WR_DROPPED_OUT_0            21
#define IPP_ARF_CVDR_VP_WR_DROPPED_OUT_1            22
#define IPP_ARF_CVDR_VP_WR_DROPPED_OUT_2            23
#define IPP_ARF_CVDR_VP_WR_DROPPED_OUT_3            24
#define IPP_ARF_CVDR_VP_WR_DROPPED_OUT_4            25
#define IPP_ARF_CVDR_VP_WR_DROPPED_OUT_5            26
#define IPP_ARF_CVDR_VP_RD_SOF_IN_0                 27
#define IPP_ARF_CVDR_VP_RD_SOF_IN_1                 28
#define IPP_ARF_CVDR_VP_RD_SOF_IN_2                 29

// rdr irq
#define IPP_RDR_IRQ_DONE                            0
#define IPP_RDR_CVDR_VP_RD_EOF_FP                   1
#define IPP_RDR_CVDR_VP_WR_EOF_CMDLST               2
#define IPP_RDR_CVDR_VP_RD_EOF_CMDSLT               3
#define IPP_RDR_CVDR_VP_RD_SOF_FP                   4

// cmp irq
#define IPP_CMP_IRQ_DONE                            0
#define IPP_CMP_CVDR_VP_RD_EOF_FP                   1
#define IPP_CMP_CVDR_VP_WR_EOF_CMDLST               2
#define IPP_CMP_CVDR_VP_RD_EOF_CMDLST               3
#define IPP_CMP_CVDR_VP_RD_SOF_FP                   4

// mc irq
#define IPP_MC_IRQ_DONE                             0
#define IPP_MC_CVDR_VP_WR_EOF_MC_RESULT             1
#define IPP_MC_CVDR_VP_WR_EOF_CMDLST                2
#define IPP_MC_CVDR_VP_RD_EOF_CMDLST                3

// klt irq
#define IPP_KLT_CVDR_VP_RD_SOF_KLT_SRC              19 // KLT_src
#define IPP_KLT_CVDR_VP_WR_DROPPED_NEXT_LAYER_4     18 // KLT_next_layer4
#define IPP_KLT_CVDR_VP_WR_DROPPED_NEXT_LAYER_3     17 // KLT_next_layer3
#define IPP_KLT_CVDR_VP_WR_DROPPED_NEXT_LAYER_2     16 // KLT_next_layer2
#define IPP_KLT_CVDR_VP_WR_DROPPED_NEXT_LAYER_1     15 // KLT_next_layer1
#define IPP_KLT_CVDR_VP_WR_DROPPED_NEXT_LAYER_0     14 // KLT_next_layer0
#define IPP_KLT_CVDR_VP_WR_SOF_NEXT_LAYER_4         13 // KLT_next_layer4
#define IPP_KLT_CVDR_VP_WR_SOF_NEXT_LAYER_3         12 // KLT_next_layer3
#define IPP_KLT_CVDR_VP_WR_SOF_NEXT_LAYER_2         11 // KLT_next_layer2
#define IPP_KLT_CVDR_VP_WR_SOF_NEXT_LAYER_1         10 // KLT_next_layer1
#define IPP_KLT_CVDR_VP_WR_SOF_NEXT_LAYER_0         9  // KLT_next_layer0
#define IPP_KLT_CVDR_VP_RD_EOF_CMDLST               8  // CMDLST
#define IPP_KLT_CVDR_VP_WR_EOF_CMDLST               7  // CMDLST
#define IPP_KLT_CVDR_VP_RD_EOF_KLT_SRC              6  // KLT_src
#define IPP_KLT_CVDR_VP_WR_EOF_NEXT_LAYER_4         5  // KLT_next_layer4
#define IPP_KLT_CVDR_VP_WR_EOF_NEXT_LAYER_3         4  // KLT_next_layer3
#define IPP_KLT_CVDR_VP_WR_EOF_NEXT_LAYER_2         3  // KLT_next_layer2
#define IPP_KLT_CVDR_VP_WR_EOF_NEXT_LAYER_1         2  // KLT_next_layer1
#define IPP_KLT_CVDR_VP_WR_EOF_NEXT_LAYER_0         1  // KLT_next_layer0
#define IPP_KLT_DONE_IRQ                            0  // KLT done
/* End * <--- IPP IRQ BIT */
/* *Tag_1 : End* */


#endif /* _IPP_CS_H_ */

/* ************************************** END ***************************** */

