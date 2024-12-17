#include "npu_log.h"
#include "npu_common.h"
#include "npu_except_mgr.h"

void npu_plat_mntn_reset(void)
{
	struct npu_dev_ctx *dev_ctx = get_dev_ctx_by_id(0);

	cond_return_void(dev_ctx == NULL, "dev_ctx is null\n");
	npu_drv_err("enter");
	/* In npu_mntn_rdr_reset we have already check the modid, so we don't need check here again */

	npu_raise_qic_exception(dev_ctx);
}