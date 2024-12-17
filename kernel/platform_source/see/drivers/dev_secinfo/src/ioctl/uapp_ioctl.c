/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: uapp provide service for user via ioctl
 * Create: 2022/04/20
 */
#include <security_info_main.h>
#include <platform_include/see/dev_secinfo.h>
#include <dev_secinfo_errno.h>
#include <dev_secinfo_plat.h>
#include <linux/device.h>
#include <linux/init.h>
#include <linux/module.h>
#include <platform_include/see/uapp.h>
#include <linux/uaccess.h>
#include <linux/workqueue.h>
#include <linux/sched.h>
#include <linux/syscalls.h>
#include <linux/completion.h>
#include <linux/fs.h>
#include <securec.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
#include <linux/compat.h>
#endif

#define UAPP_QUEUE_NAME                          "uapp_queue"

struct uapp_work_ctx {
	s32 result;
	u64 *paras;
	size_t size;
	struct completion completion;
	struct work_struct work;
	struct workqueue_struct *queue;
};

typedef void (*uapp_work_fn)(struct work_struct *work);

static void uapp_set_enable_work(struct work_struct *work)
{
	u32 ret;
	struct uapp_work_ctx *ctx = NULL;

	ctx = container_of(work, struct uapp_work_ctx, work);
	if (ctx->paras == NULL || ctx->size < 1) {
		dev_secinfo_err("error, paras size=%u\n", ctx->size);
		ctx->result = DEV_SECINFO_ERR_NULL_POINTER;
		return ;
	}

	ret = dev_secinfo_set_uapp_enable_state((u32)ctx->paras[0]);
	dev_secinfo_err("exit, ret=0x%x!\n", ret);
	ctx->result = (s32)ret;
	complete(&ctx->completion);
}

static s32 uapp_work_ctx_init(struct uapp_work_ctx *ctx, u64 *paras, size_t size, uapp_work_fn fn)
{
	struct workqueue_struct *queue = NULL;

	queue = create_singlethread_workqueue(UAPP_QUEUE_NAME);
	if (IS_ERR_OR_NULL(queue)) {
		dev_secinfo_err("create_workqueue error\n");
		return -EFAULT;
	}

	ctx->queue = queue;
	ctx->paras = paras;
	ctx->size = size;
	INIT_WORK(&ctx->work, fn);
	init_completion(&ctx->completion);

	return 0;
}

static void uapp_work_ctx_deinit(struct uapp_work_ctx *ctx)
{
	destroy_workqueue(ctx->queue);
}

static s32 uapp_set_enable_state_by_queue(u32 state)
{
	s32 ret;
	struct uapp_work_ctx ctx = {0};
	u64 paras[] = {
		(u64)state,
	};
	size_t size = dev_secinfo_array_size(paras);

	dev_secinfo_err("enter %s\n", __func__);

	ret = uapp_work_ctx_init(&ctx, paras, size, uapp_set_enable_work);
	if (ret != 0) {
		dev_secinfo_err("error, init uapp work %d!\n", ret);
		return ret;
	}

	queue_work(ctx.queue, &ctx.work);
	wait_for_completion(&ctx.completion);
	ret = ctx.result;

	uapp_work_ctx_deinit(&ctx);
	dev_secinfo_err("exit, ret=0x%x\n", ret);

	return ret;
}

s32 security_info_set_uapp_enable_state(u32 cmd, uintptr_t arg)
{
	s32 ret;
	void __user *argp = (void __user *)arg;
	u32 state;
	uint32_t size_in_cmd = _IOC_SIZE(cmd);

	/* if declare and define argp, static check warning */
	if (argp == NULL) {
		dev_secinfo_err("error, argp is NULL\n");
		return -EFAULT;
	}

	if (cmd != SECURUTY_INFO_SET_UAPP_ENABLE_STATE) {
		dev_secinfo_err("error, cmd=0x%08x\n", cmd);
		return -EFAULT;
	}

	if (size_in_cmd != sizeof(state)) {
		dev_secinfo_err("error, size_in_cmd=0x%08x\n", size_in_cmd);
		return -EFAULT;
	}

	ret = (s32)copy_from_user(&state, argp, sizeof(state));
	if (ret != 0) {
		dev_secinfo_err("error, copy_from uapp enable state ret=0x%08X\n", ret);
		return -EFAULT;
	}

	ret = uapp_set_enable_state_by_queue(state);
	if (ret != UAPP_OK) {
		dev_secinfo_err("error, set uapp enable state ret=0x%08X\n", ret);
		return -EFAULT;
	}

	return 0;
}

s32 security_info_get_uapp_enable_state(u32 cmd, uintptr_t arg)
{
	u32 ret;
	void __user *argp = (void __user *)arg;
	u32 state = 0;
	uint32_t size_in_cmd = _IOC_SIZE(cmd);

	/* if declare and define argp, static check warning */
	if (argp == NULL) {
		dev_secinfo_err("error, argp is NULL\n");
		return -EFAULT;
	}

	if (cmd != SECURUTY_INFO_GET_UAPP_ENABLE_STATE) {
		dev_secinfo_err("error, cmd=0x%08x\n", cmd);
		return -EFAULT;
	}

	if (size_in_cmd != sizeof(state)) {
		dev_secinfo_err("error, size_in_cmd=0x%08x\n", size_in_cmd);
		return -EFAULT;
	}

	ret = uapp_get_enable_state(&state);
	if (ret != UAPP_OK) {
		dev_secinfo_err("error, get uapp enable state ret=0x%08X\n", ret);
		return -EFAULT;
	}

	ret = (u32)copy_to_user(argp, &state, sizeof(state));
	if (ret != 0) {
		dev_secinfo_err("error, copy_to uapp enable state ret=0x%08X\n", ret);
		return -EFAULT;
	}

	return 0;
}

s32 security_info_valid_uapp_bindfile_pubkey(u32 cmd, uintptr_t arg)
{
	u32 ret;
	void __user *argp = (void __user *)arg;
	u32 key_idx = 0;
	uint32_t size_in_cmd = _IOC_SIZE(cmd);

	/* if declare and define argp, static check warning */
	if (argp == NULL) {
		dev_secinfo_err("error, argp is NULL\n");
		return -EFAULT;
	}

	if (size_in_cmd != sizeof(key_idx)) {
		dev_secinfo_err("error, size_in_cmd=0x%08x\n", size_in_cmd);
		return -EFAULT;
	}

	if (cmd != SECURUTY_INFO_VALID_UAPP_BINDFILE_PUBKEY) {
		dev_secinfo_err("error, cmd=0x%08x\n", cmd);
		return -EFAULT;
	}

	ret = (u32)copy_from_user(&key_idx, argp, sizeof(key_idx));
	if (ret != 0) {
		dev_secinfo_err("error, copy_from uapp valid key_idx ret=0x%08X\n", ret);
		return -EFAULT;
	}

	ret = uapp_valid_bindfile_pubkey(key_idx);
	if (ret != UAPP_OK) {
		dev_secinfo_err("error, uapp_valid_bindfile_pubkey ret=0x%08X\n", ret);
		return -EFAULT;
	}

	return 0;
}

s32 security_info_revoke_uapp_bindfile_pubkey(u32 cmd, uintptr_t arg)
{
	u32 ret;
	void __user *argp = (void __user *)arg;
	u32 key_idx = 0;
	uint32_t size_in_cmd = _IOC_SIZE(cmd);

	/* if declare and define argp, static check warning */
	if (argp == NULL) {
		dev_secinfo_err("error, argp is NULL\n");
		return -EFAULT;
	}

	if (cmd != SECURUTY_INFO_REVOKE_UAPP_BINDFILE_PUBKEY) {
		dev_secinfo_err("error, cmd=0x%08x\n", cmd);
		return -EFAULT;
	}

	if (size_in_cmd != sizeof(key_idx)) {
		dev_secinfo_err("error, size_in_cmd=0x%08x\n", size_in_cmd);
		return -EFAULT;
	}

	ret = (u32)copy_from_user(&key_idx, argp, sizeof(key_idx));
	if (ret != 0) {
		dev_secinfo_err("error, copy_from uapp key_idx ret=0x%08X\n", ret);
		return -EFAULT;
	}

	ret = uapp_revoke_bindfile_pubkey(key_idx);
	if (ret != UAPP_OK) {
		dev_secinfo_err("error, uapp_valid_bindfile_pubkey ret=0x%08X\n", ret);
		return -EFAULT;
	}

	return 0;
}

s32 security_info_get_uapp_bindfile_pos(u32 cmd, uintptr_t arg)
{
	u32 ret;
	void __user *argp = (void __user *)arg;
	struct uapp_bindfile_pos bindfile = {{0}, {0}};
	uint32_t size_in_cmd = _IOC_SIZE(cmd);

	/* if declare and define argp, static check warning */
	if (!argp) {
		dev_secinfo_err("error, argp is NULL\n");
		return -EFAULT;
	}

	if (cmd != SECURUTY_INFO_GET_UAPP_BINDFILE_POS) {
		dev_secinfo_err("error, cmd=0x%08x\n", cmd);
		return -EFAULT;
	}

	if (size_in_cmd != sizeof(bindfile)) {
		dev_secinfo_err("error, size_in_cmd=0x%08x\n", size_in_cmd);
		return -EFAULT;
	}

	ret = uapp_get_bindfile_pos(&bindfile);
	if (ret != UAPP_OK) {
		dev_secinfo_err("error 0x%x, get uapp bindfile info\n", ret);
		return -EFAULT;
	}

	ret = (u32)copy_to_user(argp, &bindfile, sizeof(bindfile));
	if (ret != 0) {
		dev_secinfo_err("error 0x%x, copy bindfile to userland\n", ret);
		return -EFAULT;
	}

	return 0;
}

s32 security_info_get_uapp_empower_cert_pos(u32 cmd, uintptr_t arg)
{
	u32 ret;
	void __user *argp = (void __user *)arg;
	struct uapp_file_pos empower_cert = { 0 };
	uint32_t size_in_cmd = _IOC_SIZE(cmd);

	/* if declare and define argp, static check warning */
	if (!argp) {
		dev_secinfo_err("error, argp is NULL\n");
		return -EFAULT;
	}

	if (cmd != SECURUTY_INFO_GET_UAPP_EMPOWER_CERT_POS) {
		dev_secinfo_err("error, cmd=0x%08x\n", cmd);
		return -EFAULT;
	}

	if (size_in_cmd != sizeof(empower_cert)) {
		dev_secinfo_err("error, size_in_cmd=0x%08x\n", size_in_cmd);
		return -EFAULT;
	}

	ret = uapp_get_empower_cert_pos(&empower_cert);
	if (ret != UAPP_OK) {
		dev_secinfo_err("error 0x%x, get uapp empower cert pos\n", ret);
		return -EFAULT;
	}

	ret = (u32)copy_to_user(argp, &empower_cert, sizeof(empower_cert));
	if (ret != 0) {
		dev_secinfo_err("error 0x%x, copy empower_cert pos to userland\n", ret);
		return -EFAULT;
	}

	return 0;
}

s32 security_info_get_rotpk_hash(u32 cmd, uintptr_t arg)
{
	u32 ret;
	void __user *argp = (void __user *)arg;
	struct rotpk_hash hash = {0};
	uint32_t size_in_cmd = _IOC_SIZE(cmd);

	/* if declare and define argp, static check warning */
	if (argp == NULL) {
		dev_secinfo_err("argp is NULL\n");
		return -EFAULT;
	}

	if (cmd != SECURUTY_INFO_GET_ROTPK_HASH) {
		dev_secinfo_err("error, cmd 0x%08x\n", cmd);
		return -EFAULT;
	}

	if (size_in_cmd != sizeof(hash)) {
		dev_secinfo_err("error, size_in_cmd=0x%08x\n", size_in_cmd);
		return -EFAULT;
	}

	ret = uapp_get_rotpk_hash(&hash);
	if (ret != 0) {
		dev_secinfo_err("error 0x%x, get rotpk_hash\n", ret);
		return -EFAULT;
	}

	ret = (u32)copy_to_user(argp, &hash, sizeof(hash));
	if (ret != 0) {
		dev_secinfo_err("error 0x%x,copy rotpk_hash to userland\n", ret);
		return -EFAULT;
	}

	return 0;
}
