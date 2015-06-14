ACLOCAL_AMFLAGS = -I m4

AUTOMAKE_OPTIONS = foreign
SUBDIRS = kernel kapi kthread network shell drivers/x86 lib osentry fs



HX_FLAGS =
HX_INCLUDES =

HX_INCLUDES += -I$(top_srcdir)/kernel/include -I$(top_srcdir)/kernel/config -I$(top_srcdir)/kernel/lib/sys -I$(top_srcdir)/kernel/lib

HX_FLAGS += -D_POSIX_  -D_TIME_T_DEFINED -D_M_IX86
HX_FLAGS += -nostdlib -nostdinc -m32 
HX_FLAGS += $(HX_INCLUDES)
HX_FLAGS += -Wall


LDADD = 
##LDADD += ./osentry/.libs/libosentry.a
LDADD += ./lib/libhellolib.a
LDADD += ./drivers/x86/libdrivers.a
LDADD += ./shell/libshell.a
LDADD += ./network/libnetwork.a
LDADD += ./kthread/libkthread.a
LDADD += ./arch/x86/libarch.a
LDADD += ./fs/libfs.a
LDADD += ./kernel/libkernel.a
LDADD += ./kapi/libkapi.a


AM_CPPFLAGS=$(HX_FLAGS)

bin_PROGRAMS = hellox_kerel
hellox_kerel_SOURCES = osentry/drventry.c	osentry/mem_scat.c osentry/os_entry.c
hellox_kerel_CPPFLAGS = $(AM_CPPFLAGS)
hellox_kerel_LDADD = $(LDADD)


