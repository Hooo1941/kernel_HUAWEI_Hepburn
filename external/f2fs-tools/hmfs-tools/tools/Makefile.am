## Makefile.am

AM_CPPFLAGS = ${libuuid_CFLAGS} -I$(top_srcdir)/include
AM_CFLAGS = -Wall
sbin_PROGRAMS = f2fstat fibmap.f2fs parse.f2fs
f2fstat_SOURCES = f2fstat.c
fibmap_f2fs_SOURCES = fibmap.c
parse_f2fs_SOURCES = f2fs_io_parse.c

if LINUX
sbin_PROGRAMS += hmfscrypt
hmfscrypt_SOURCES = f2fscrypt.c sha512.c
hmfscrypt_LDFLAGS = -luuid
dist_man_MANS = f2fscrypt.8
endif

SUBDIRS = sg_write_buffer f2fs_io
