 /* SPDX-License-Identifier: GPL-2.0 */

/*
 * Note: we intentionally omit include file ifdef protection
 *  This is due to the way trace events work. If a file includes two
 *  trace event headers under one "CREATE_TRACE_POINTS" the first include
 *  will override the DECLARE_RESTRICTED_HOOK and break the second include.
 */

#include <linux/tracepoint.h>

#if defined(CONFIG_ANDROID_VENDOR_HOOKS)
#undef REGISTER_HCK_VNH
#define REGISTER_HCK_VNH(_pub_name, probe, data)			\
		register_trace_android_##_pub_name(probe, data)

#undef DECLARE_HCK_VNH
#define DECLARE_HCK_VNH(_pub_name, proto, args)		\
		DECLARE_TRACE(android_##_pub_name, PARAMS(proto), PARAMS(args))

#else /* !CONFIG_ANDROID_VENDOR_HOOKS */
#undef REGISTER_HCK_VNH
#define REGISTER_HCK_VNH(_pub_name, probe, data)			\
		register_trace_hck_##_pub_name(probe, data)

#undef DECLARE_HCK_VNH
#define DECLARE_HCK_VNH(_pub_name, proto, args)				\
		DECLARE_TRACE(hck_##_pub_name, PARAMS(proto), PARAMS(args))

#endif /* CONFIG_ANDROID_VENDOR_HOOKS */
