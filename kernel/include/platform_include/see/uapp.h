/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: header of UAPP (Unique Auth Per Phone)
 * Create: 2022/04/20
 */

#ifndef __UAPP_H__
#define __UAPP_H__

#include <linux/types.h>
#include <linux/kernel.h>

/* error code */
#define UAPP_OK                         0x0
#define UAPP_FUNC_UNSUPPORT_ERR         0x3DA7FFFF

#define PTN_NAME_LENGTH_MAX             36
#define ROTPK_HASH_MAX_BYTES            0x40

enum uapp_enable_state {
	UAPP_DISABLE = 0,
	UAPP_ENABLE = 1,
	UAPP_ENABLE_STATE_MAX,
};

struct uapp_file_pos {
	char ptn[PTN_NAME_LENGTH_MAX];
	u64 offset;
	u64 size;
};

struct uapp_bindfile_pos {
	struct uapp_file_pos bindfile;
	struct uapp_file_pos bindfile_backup;
};

struct rotpk_hash {
	u32 bytes;
	u8 val[ROTPK_HASH_MAX_BYTES];
};
#ifdef CONFIG_UAPP
u32 uapp_set_enable_state(u32 state);
u32 uapp_get_enable_state(u32 *state);
u32 uapp_valid_bindfile_pubkey(u32 key_idx);
u32 uapp_revoke_bindfile_pubkey(u32 key_idx);
u32 uapp_get_rotpk_hash(struct rotpk_hash *rotpk_hash);
u32 uapp_get_bindfile_pos(struct uapp_bindfile_pos *bindfile);
u32 uapp_get_empower_cert_pos(struct uapp_file_pos *empower_cert);
#else
static inline u32 uapp_set_enable_state(u32 state)
{
	(void)state;
	pr_err("%s not implement\n", __func__);
	return UAPP_FUNC_UNSUPPORT_ERR;
}

static inline u32 uapp_get_enable_state(u32 *state)
{
	(void)state;
	pr_err("%s not implement\n", __func__);
	return UAPP_FUNC_UNSUPPORT_ERR;
}

static inline u32 uapp_valid_bindfile_pubkey(u32 key_idx)
{
	(void)key_idx;
	pr_err("%s not implement\n", __func__);
	return UAPP_FUNC_UNSUPPORT_ERR;
}

static inline u32 uapp_revoke_bindfile_pubkey(u32 key_idx)
{
	(void)key_idx;
	pr_err("%s not implement\n", __func__);
	return UAPP_FUNC_UNSUPPORT_ERR;
}

static inline u32 uapp_get_rotpk_hash(struct rotpk_hash *rotpk_hash)
{
	(void)rotpk_hash;
	pr_err("%s not implement\n", __func__);
	return UAPP_FUNC_UNSUPPORT_ERR;
}

static inline u32 uapp_get_bindfile_pos(struct uapp_bindfile_pos *bindfile)
{
	(void)bindfile;
	pr_err("%s not implement\n", __func__);
	return UAPP_FUNC_UNSUPPORT_ERR;
}

static inline u32 uapp_get_empower_cert_pos(struct uapp_file_pos *empower_cert)
{
	(void)empower_cert;
	pr_err("%s not implement\n", __func__);
	return UAPP_FUNC_UNSUPPORT_ERR;
}
#endif /* CONFIG_UAPP */

#endif /* __UAPP_H__ */
