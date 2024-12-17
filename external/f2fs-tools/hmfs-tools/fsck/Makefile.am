## Makefile.am

AM_CPPFLAGS = ${libuuid_CFLAGS} -I$(top_srcdir)/include
AM_CFLAGS = -Wall
sbin_PROGRAMS = fsck.hmfs
noinst_HEADERS = common.h dict.h dqblk_v2.h f2fs.h fsck.h node.h quotaio.h quotaio_tree.h quotaio_v2.h xattr.h queue.h quotaio_cache.h
include_HEADERS = $(top_srcdir)/include/quota.h
fsck_hmfs_SOURCES = main.c fsck.c dump.c mount.c defrag.c resize.c \
		node.c segment.c dir.c sload.c xattr.c \
		dict.c mkquota.c quotaio.c quotaio_tree.c quotaio_v2.c \
		queue.c quotaio_cache.c
fsck_hmfs_LDFLAGS = -lpthread
fsck_hmfs_LDADD = ${libselinux_LIBS} ${libuuid_LIBS} $(top_builddir)/lib/libf2fs.la

install-data-hook:
	ln -sf fsck.hmfs $(DESTDIR)/$(sbindir)/dump.hmfs
	ln -sf fsck.hmfs $(DESTDIR)/$(sbindir)/defrag.hmfs
	ln -sf fsck.hmfs $(DESTDIR)/$(sbindir)/resize.hmfs
	ln -sf fsck.hmfs $(DESTDIR)/$(sbindir)/sload.hmfs
