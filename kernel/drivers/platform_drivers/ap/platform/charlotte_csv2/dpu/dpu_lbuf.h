#ifndef DPU_LBUF_H
#define DPU_LBUF_H 
#include "soc_dpu_interface.h"
#include "soc_dpu_define.h"
#define DPU_DBUF_UNIT 4
#define DPU_DBUF_DEPTH 360
#define DPU_DBUF_REG_NUM 5
#define DPU_DBUF_LEVEL_NUM 2
enum dbuf_thd_type {
 DBUF_THD_RQOS_IN = 0,
 DBUF_THD_RQOS_OUT,
 DBUF_THD_FLUX_REQ_BEF_IN,
 DBUF_THD_FLUX_REQ_BEF_OUT,
 DBUF_THD_TYPE_MAX
};
struct dpu_lbuf_ctl_reg {
 DPU_LBUF_MEM_DSLP_CTRL_UNION mem_dslp_ctrl;
 DPU_LBUF_SOUR_AUTO_CTRL_UNION sour_auto_ctrl;
 DPU_LBUF_DEBUG_CTRL_UNION debug_ctrl;
};
struct dpu_lbuf_dbuf0_reg {
 DPU_LBUF_DBUF0_CTRL_UNION ctrl;
 DPU_LBUF_DBUF0_STATE_UNION state;
 DPU_LBUF_DBUF0_THD_RQOS_UNION thd_rqos;
 DPU_LBUF_DBUF0_THD_DFS_OK_UNION thd_dfs_ok;
 DPU_LBUF_DBUF0_THD_FLUX_REQ_BEF_UNION thd_flux_req_bef;
 DPU_LBUF_DBUF0_THD_FLUX_REQ_AFT_UNION thd_flux_req_aft;
 DPU_LBUF_DBUF0_ONLINE_FILL_LEVEL_UNION online_fill_level;
 DPU_LBUF_DBUF0_THD_FILL_LEV0_UNION thd_fill_lev0;
 DPU_LBUF_DBUF0_PRS_SCN0_UNION prs_scn0;
};
struct dpu_lbuf_dbuf1_reg {
 DPU_LBUF_DBUF1_CTRL_UNION ctrl;
 DPU_LBUF_DBUF1_STATE_UNION state;
 DPU_LBUF_DBUF1_THD_RQOS_UNION thd_rqos;
 DPU_LBUF_DBUF1_THD_DFS_OK_UNION thd_dfs_ok;
 DPU_LBUF_DBUF1_THD_FLUX_REQ_BEF_UNION thd_flux_req_bef;
 DPU_LBUF_DBUF1_THD_FLUX_REQ_AFT_UNION thd_flux_req_aft;
 DPU_LBUF_DBUF1_ONLINE_FILL_LEVEL_UNION online_fill_level;
 DPU_LBUF_DBUF1_THD_FILL_LEV0_UNION thd_fill_lev0;
 DPU_LBUF_DBUF1_PRS_SCN1_UNION prs_scn1;
};
struct dpu_lbuf_dbuf2_reg {
 DPU_LBUF_DBUF2_CTRL_UNION ctrl;
 DPU_LBUF_DBUF2_STATE_UNION state;
 DPU_LBUF_DBUF2_THD_RQOS_UNION thd_rqos;
 DPU_LBUF_DBUF2_ONLINE_FILL_LEVEL_UNION online_fill_level;
 DPU_LBUF_DBUF2_THD_FILL_LEV0_UNION thd_fill_lev0;
 DPU_LBUF_DBUF2_PRS_SCN2_UNION prs_scn2;
};
struct dpu_lbuf_dbuf3_reg {
 DPU_LBUF_DBUF3_CTRL_UNION ctrl;
 DPU_LBUF_DBUF3_STATE_UNION state;
 DPU_LBUF_DBUF3_THD_RQOS_UNION thd_rqos;
 DPU_LBUF_DBUF3_ONLINE_FILL_LEVEL_UNION online_fill_level;
 DPU_LBUF_DBUF3_THD_FILL_LEV0_UNION thd_fill_lev0;
 DPU_LBUF_DBUF3_PRS_SCN3_UNION prs_scn3;
};
struct dpu_lbuf_params {
 struct dpu_lbuf_ctl_reg ctl_reg;
 struct dpu_lbuf_dbuf0_reg dbuf0_reg;
 struct dpu_lbuf_dbuf1_reg dbuf1_reg;
 struct dpu_lbuf_dbuf2_reg dbuf2_reg;
 struct dpu_lbuf_dbuf3_reg dbuf3_reg;
};
#endif
