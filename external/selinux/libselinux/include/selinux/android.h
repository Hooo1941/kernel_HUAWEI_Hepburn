#ifndef _SELINUX_ANDROID_H_
#define _SELINUX_ANDROID_H_

#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>

#include <selinux/label.h>

#ifdef __cplusplus
extern "C" {
#endif

extern struct selabel_handle* selinux_android_file_context_handle(void);

extern struct selabel_handle* selinux_android_service_context_handle(void);

extern struct selabel_handle* selinux_android_hw_service_context_handle(void);

extern struct selabel_handle* selinux_android_vendor_service_context_handle(void);

extern struct selabel_handle* selinux_android_keystore2_key_context_handle(void);

extern void selinux_android_set_sehandle(const struct selabel_handle *hndl);

extern int selinux_android_load_policy(void);

extern int selinux_android_load_policy_from_fd(int fd, const char *description);

extern int selinux_android_setcon(const char *con);

extern int selinux_android_setcontext(uid_t uid,
				      bool isSystemServer,
				      const char *seinfo,
				      const char *name);

extern int selinux_android_context_with_level(const char * context,
					      char ** newContext,
					      uid_t userid,
					      uid_t appid);

extern int selinux_log_callback(int type, const char *fmt, ...)
    __attribute__ ((format(printf, 2, 3)));

// API to support legacy usecase where full-treble legacy VNDK vendor needs to use this callback.
extern int selinux_vendor_log_callback(int type, const char *fmt, ...)
    __attribute__ ((format(printf, 2, 3)));

#define SELINUX_ANDROID_RESTORECON_NOCHANGE 1
#define SELINUX_ANDROID_RESTORECON_VERBOSE  2
#define SELINUX_ANDROID_RESTORECON_RECURSE  4
#define SELINUX_ANDROID_RESTORECON_FORCE    8
#define SELINUX_ANDROID_RESTORECON_DATADATA 16
#define SELINUX_ANDROID_RESTORECON_SKIPCE   32
#define SELINUX_ANDROID_RESTORECON_CROSS_FILESYSTEMS   64
#define SELINUX_ANDROID_RESTORECON_SKIP_SEHASH         128
extern int selinux_android_restorecon(const char *file, unsigned int flags);

extern int selinux_android_restorecon_pkgdir(const char *pkgdir,
                                             const char *seinfo,
                                             uid_t uid,
                                             unsigned int flags);

extern void selinux_android_seapp_context_init(void);

extern int selinux_android_seapp_context_reload(void);

extern void selinux_android_skip_restorecon_filter_path(const char *src_path, char *filter_path, int flags);

#ifdef __cplusplus
}
#endif
#endif
