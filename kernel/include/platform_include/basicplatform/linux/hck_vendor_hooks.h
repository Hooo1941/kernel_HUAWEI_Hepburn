 /* SPDX-License-Identifier: GPL-2.0 */

/*
 * Note: we intentionally omit include file ifdef protection
 *  This is due to the way trace events work. If a file includes two
 *  trace event headers under one "CREATE_LITE_VENDOR_HOOK" the first include
 *  will override the DECLARE_LITE_RESTRICTED_HOOK and break the second include.
 */

#ifndef __HCK_VENDOR_HOOK_H__
#define __HCK_VENDOR_HOOK_H__
#include <asm/bug.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/rcupdate.h>
#include <linux/tracepoint.h>
#include <linux/module.h>

struct __vh_func {
	void *func;
	void *data;
	bool has_data;
};

struct hck_vendor_hook {
	struct mutex mutex;
	struct __vh_func *funcs;
};
#endif // __HCK_VENDOR_HOOK_H__

#ifdef CREATE_HCK_VENDOR_HOOK

#define DEFINE_HCK_HOOK(name, proto, args)	\
	struct hck_vendor_hook __vh_##name __used	\
	__section("__vendor_hooks") = {	\
		.mutex = __MUTEX_INITIALIZER(__vh_##name.mutex),	\
		.funcs = NULL };	\
	EXPORT_SYMBOL(__vh_##name);	\
	void vh_probe_##name(void *vh_data, proto) { return; }

#undef DECLARE_HCK_VH
#define DECLARE_HCK_VH(_pub_name, proto, args)		\
		DEFINE_HCK_HOOK(hck_##_pub_name, PARAMS(proto), PARAMS(args))

#else // #ifndef CREATE_HCK_VENDOR_HOOK

#define REGISTER_HCK_HOOK(name, probe)	\
	extern typeof(vh_probe_##name) probe;	\
	do {	\
		if (register_vh_##name(probe))	\
			WARN_ONCE(1, "VH register failed!\n");	\
	} while (0)

#define REGISTER_HCK_HOOK_DATA(name, probe, data)	\
	extern typeof(vh_probe_data_##name) probe;	\
	do {	\
		if (register_vh_data_##name(probe, data))	\
			WARN_ONCE(1, "VH register failed!\n");	\
	} while (0)

#define CALL_HCK_VH(name, args...)	\
	call_vh_hck_##name(args)

#define __DECLARE_HCK_HOOK(name, proto, args)	\
	extern struct hck_vendor_hook __vh_##name;	\
	extern void vh_probe_##name(proto);	\
	extern void vh_probe_data_##name(void *vh_data, proto);	\
	static inline void	\
	call_vh_##name(proto)	\
	{	\
		struct __vh_func *funcs = (&__vh_##name)->funcs;	\
		if (funcs && funcs->func) {	\
			if (funcs->has_data)	\
				((void(*)(void *, proto))funcs->func)(funcs->data, args);	\
			else	\
				((void(*)(proto))funcs->func)(args);	\
		}	\
	}	\
	static inline int	\
	__register_vh_##name(void *probe, void *data, bool has_data)	\
	{	\
		int err = 0;	\
		struct __vh_func *funcs;	\
		struct module *mod;	\
		mutex_lock(&__vh_##name.mutex);	\
		funcs = (&__vh_##name)->funcs;	\
		if (funcs) {	\
			if (funcs->func != probe || funcs->data != data)	\
				err = -EBUSY;	\
			goto out;	\
		}	\
		\
		funcs = (struct __vh_func*)kmalloc(sizeof(struct __vh_func), GFP_KERNEL);	\
		if (!funcs) {	\
			err = -ENOMEM;	\
			goto out;	\
		}	\
		\
		funcs->func = probe;	\
		funcs->data = data;	\
		funcs->has_data = has_data;	\
		mod = __module_address((uintptr_t)probe);	\
		if (mod)	\
			(void)try_module_get(mod);	\
		(&__vh_##name)->funcs = funcs;	\
	out:	\
		mutex_unlock(&__vh_##name.mutex);	\
		return err;	\
	}	\
	static inline int	\
	register_vh_##name(void (*probe)(proto))	\
	{	\
		return __register_vh_##name((void *)probe, NULL, false);	\
	}	\
	static inline int	\
	register_vh_data_##name(void (*probe)(void *vh_data, proto), void *data)	\
	{	\
		return __register_vh_##name((void *)probe, data, true);	\
	}

#undef REGISTER_HCK_VH
#define REGISTER_HCK_VH(_pub_name, probe)			\
		REGISTER_HCK_HOOK(hck_##_pub_name, probe)

#undef REGISTER_HCK_VH_DATA
#define REGISTER_HCK_VH_DATA(_pub_name, probe, data)			\
		REGISTER_HCK_HOOK_DATA(hck_##_pub_name, probe, data)

#undef DECLARE_HCK_VH
#define DECLARE_HCK_VH(_pub_name, proto, args)		\
		__DECLARE_HCK_HOOK(hck_##_pub_name, PARAMS(proto), PARAMS(args))

#endif /* CREATE_HCK_VENDOR_HOOK */
