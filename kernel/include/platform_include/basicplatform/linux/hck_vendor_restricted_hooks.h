 /* SPDX-License-Identifier: GPL-2.0 */

/*
 * Note: we intentionally omit include file ifdef protection
 *  This is due to the way trace events work. If a file includes two
 *  trace event headers under one "CREATE_TRACE_POINTS" the first include
 *  will override the DECLARE_RESTRICTED_HOOK and break the second include.
 */

#include <linux/tracepoint.h>

#if defined(CONFIG_TRACEPOINTS)

#if defined(CONFIG_ANDROID_VENDOR_HOOKS)
#include <trace/hooks/vendor_hooks.h>

#undef REGISTER_HCK_VRH
#define REGISTER_HCK_VRH(name, probe, data)			\
			register_trace_android_##name(probe, data)

int android_rvh_probe_register(struct tracepoint *tp, void *probe, void *data);
	#define rvh_probe_register android_rvh_probe_register
#else
#undef REGISTER_HCK_VRH
#define REGISTER_HCK_VRH(name, probe, data)			\
		register_trace_hck_##name(probe, data)
int hck_rvh_probe_register(struct tracepoint *tp, void *probe, void *data);
	#define rvh_probe_register hck_rvh_probe_register
#endif

#ifdef TRACE_HEADER_MULTI_READ

#define DEFINE_HCK_HOOK_FN(_name, _reg, _unreg, proto, args)		\
	static const char __tpstrtab_##_name[]				\
	__section("__tracepoints_strings") = #_name;			\
	extern struct static_call_key STATIC_CALL_KEY(tp_func_##_name);	\
	int __traceiter_##_name(void *__data, proto);			\
	struct tracepoint __tracepoint_##_name	__used			\
	__section("__tracepoints") = {					\
		.name = __tpstrtab_##_name,				\
		.key = STATIC_KEY_INIT_FALSE,				\
		.static_call_key = &STATIC_CALL_KEY(tp_func_##_name),	\
		.static_call_tramp = STATIC_CALL_TRAMP_ADDR(tp_func_##_name), \
		.iterator = &__traceiter_##_name,			\
		.regfunc = _reg,					\
		.unregfunc = _unreg,					\
		.funcs = NULL };					\
	__TRACEPOINT_ENTRY(_name);					\
	int __traceiter_##_name(void *__data, proto)			\
	{								\
		struct tracepoint_func *it_func_ptr;			\
		void *it_func;						\
									\
		it_func_ptr = (&__tracepoint_##_name)->funcs;		\
		it_func = (it_func_ptr)->func;				\
		do {							\
			__data = (it_func_ptr)->data;			\
			((void(*)(void *, proto))(it_func))(__data, args); \
			it_func = READ_ONCE((++it_func_ptr)->func);	\
		} while (it_func);					\
		return 0;						\
	}								\
	DEFINE_STATIC_CALL(tp_func_##_name, __traceiter_##_name)

#undef DECLARE_HCK_RESTRICTED_HOOK
#define DECLARE_HCK_RESTRICTED_HOOK(name, proto, args, cond) \
	DEFINE_HCK_HOOK_FN(name, NULL, NULL, PARAMS(proto), PARAMS(args))

#if defined(CONFIG_ANDROID_VENDOR_HOOKS)
#undef DECLARE_HCK_VRH
#define DECLARE_HCK_VRH(name, proto, args, cond)			\
		DECLARE_RESTRICTED_HOOK(android_##name,		\
			PARAMS(proto), PARAMS(args), PARAMS(cond))
#else
#undef DECLARE_HCK_VRH
#define DECLARE_HCK_VRH(name, proto, args, cond) 			\
		DECLARE_HCK_RESTRICTED_HOOK(hck_##name, 		\
			PARAMS(proto), PARAMS(args), PARAMS(cond))
#endif

/* prevent additional recursion */
#undef TRACE_HEADER_MULTI_READ
#else /* !TRACE_HEADER_MULTI_READ */

#define DO_HCK_HOOK(name, proto, args, cond)				\
	do {								\
		struct tracepoint_func *it_func_ptr;			\
		void *__data;						\
									\
		if (!(cond))						\
			return;						\
									\
		it_func_ptr = (&__tracepoint_##name)->funcs;		\
		if (it_func_ptr) {					\
			__data = (it_func_ptr)->data;			\
			__DO_TRACE_CALL(name)(args);			\
		}							\
	} while (0)

#define __DECLARE_HCK_HOOK(name, proto, args, cond, data_proto, data_args)	\
	extern int __traceiter_##name(data_proto);			\
	DECLARE_STATIC_CALL(tp_func_##name, __traceiter_##name);	\
	extern struct tracepoint __tracepoint_##name;			\
	static inline void trace_##name(proto)			\
	{								\
		if (static_key_false(&__tracepoint_##name.key))		\
			DO_HCK_HOOK(name,					\
				TP_PROTO(data_proto),			\
				TP_ARGS(data_args),			\
				TP_CONDITION(cond));			\
	}								\
	static inline bool						\
	trace_##name##_enabled(void)					\
	{								\
		return static_key_false(&__tracepoint_##name.key);	\
	}								\
	static inline int						\
	register_trace_##name(void (*probe)(data_proto), void *data) 	\
	{								\
		return rvh_probe_register(&__tracepoint_##name,	\
						  (void *)probe, data);	\
	}								\
	/* vendor hooks cannot be unregistered */
#undef DECLARE_HCK_RESTRICTED_HOOK
#define DECLARE_HCK_RESTRICTED_HOOK(name, proto, args, cond)		\
	__DECLARE_HCK_HOOK(name, PARAMS(proto), PARAMS(args),		\
			cond,						\
			PARAMS(void *__data, proto),			\
			PARAMS(__data, args))

#if defined(CONFIG_ANDROID_VENDOR_HOOKS)
#undef DECLARE_HCK_VRH
#define DECLARE_HCK_VRH(name, proto, args, cond)	\
		DECLARE_RESTRICTED_HOOK(android_##name, \
			PARAMS(proto), PARAMS(args), PARAMS(cond))
#else
#undef DECLARE_HCK_VRH
#define DECLARE_HCK_VRH(name, proto, args, cond) \
		DECLARE_HCK_RESTRICTED_HOOK(hck_##name, \
			PARAMS(proto), PARAMS(args), PARAMS(cond))
#endif
#endif /* TRACE_HEADER_MULTI_READ */

#else /* !CONFIG_TRACEPOINTS */

/* suppress trace hooks */
#ifdef TRACE_HEADER_MULTI_READ
#undef DECLARE_HCK_VRH
#define DECLARE_HCK_VRH(name, proto, args, cond)

/* prevent additional recursion */
#undef TRACE_HEADER_MULTI_READ
#else /* !TRACE_HEADER_MULTI_READ */
#define __DECLARE_HCK_HOOK(name, proto, args, cond, data_proto, data_args)	\
	static inline void trace_##name(proto)			\
	{ }								\
	static inline bool						\
	trace_##name##_enabled(void)					\
	{								\
		return false;	\
	}								\
	static inline int						\
	register_trace_##name(void (*probe)(data_proto), void *data) 	\
	{								\
		return -ENOSYS;						\
	}								\
	/* vendor hooks cannot be unregistered */			\
#undef DECLARE_HCK_RESTRICTED_HOOK
#define DECLARE_HCK_RESTRICTED_HOOK(name, proto, args, cond)		\
	__DECLARE_HCK_HOOK(name, PARAMS(proto), PARAMS(args),		\
			cond,						\
			PARAMS(void *__data, proto),			\
			PARAMS(__data, args))

#undef DECLARE_HCK_VRH
#define DECLARE_HCK_VRH(name, proto, args, cond) \
	DECLARE_HCK_RESTRICTED_HOOK(hck_##name, \
		PARAMS(proto), PARAMS(args), PARAMS(cond))

#endif /* TRACE_HEADER_MULTI_READ */
#endif /* CONFIG_TRACEPOINTS */
