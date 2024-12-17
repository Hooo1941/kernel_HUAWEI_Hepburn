/******************************************************************
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2018. All rights reserved.
 * File name    cvdr.h
 * Description:
 *
 * Date         2021-11-18
 ******************************************************************/

#ifndef __IPP_CVDR__H__
#define __IPP_CVDR__H__

enum ipp_vp_rd_id_e {
	IPP_VP_RD_CMDLST = 0,
	IPP_VP_RD_ORB_ENH_Y_HIST,
	IPP_VP_RD_ORB_ENH_Y_IMAGE,
	IPP_VP_RD_RDR,
	IPP_VP_RD_CMP,
	IPP_VP_RD_ARF_0, // 5
	IPP_VP_RD_ARF_1,
	IPP_VP_RD_ARF_2,
	IPP_VP_RD_ARF_3,
	IPP_VP_RD_KLT_SRC,
	IPP_VP_RD_MAX,
};

typedef enum {
	IPP_VP_WR_CMDLST = 0,
	IPP_VP_WR_ORB_ENH_Y,
	IPP_VP_WR_ARF_PRY_1,
	IPP_VP_WR_ARF_PRY_2,
	IPP_VP_WR_ARF_DOG_0,
	IPP_VP_WR_ARF_DOG_1, // 5
	IPP_VP_WR_ARF_DOG_2,
	IPP_VP_WR_ARF_DOG_3,
	IPP_VP_WR_ARF_PYR_2_DS,
	IPP_VP_WR_MC_RESULT,
	IPP_VP_WR_KLT_NEXT_LAYER0, // 10
	IPP_VP_WR_KLT_NEXT_LAYER1,
	IPP_VP_WR_KLT_NEXT_LAYER2,
	IPP_VP_WR_KLT_NEXT_LAYER3,
	IPP_VP_WR_KLT_NEXT_LAYER4,
	IPP_VP_WR_MAX,
} ipp_vp_wr_id_e;

enum ipp_nr_rd_id_e {
	IPP_NR_RD_MC = 0,
	IPP_NR_RD_CMP,
	IPP_NR_RD_KLT_PREV_FRAME,
	IPP_NR_RD_KLT_NEXT_FRAME,
	IPP_NR_RD_MAX,
};

typedef enum {
	IPP_NR_WR_RDR = 0,
	IPP_NR_WR_MAX,
} ipp_nr_wr_id_e;

extern unsigned int g_cvdr_vp_wr_id[IPP_VP_WR_MAX];
extern unsigned int g_cvdr_vp_rd_id[IPP_VP_RD_MAX];
extern unsigned int g_cvdr_nr_wr_id[IPP_NR_WR_MAX];
extern unsigned int g_cvdr_nr_rd_id[IPP_NR_RD_MAX];

int ippdev_lock(void);
int ippdev_unlock(void);

#define get_cvdr_vp_wr_port_num(x) g_cvdr_vp_wr_id[x]
#define get_cvdr_vp_rd_port_num(x) g_cvdr_vp_rd_id[x]
#define get_cvdr_nr_wr_port_num(x) g_cvdr_nr_wr_id[x]
#define get_cvdr_nr_rd_port_num(x) g_cvdr_nr_rd_id[x]
#endif /* __IPP_CVDR__H__ */
