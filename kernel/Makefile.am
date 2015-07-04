ACLOCAL_AMFLAGS = -I m4

AUTOMAKE_OPTIONS = foreign
SUBDIRS = kernel kapi kthread network shell drivers/x86 lib osentry fs



HX_INCLUDES =

HX_INCLUDES += -I$(top_srcdir)/kernel/include 
HX_INCLUDES += -I$(top_srcdir)/kernel/config 
HX_INCLUDES += -I$(top_srcdir)/kernel/lib/sys -I$(top_srcdir)/kernel/lib

HX_FLAGS =
HX_FLAGS += -nostdlib -nostdinc -m32 
HX_FLAGS +=   -D_M_IX86 -DHAVE_CONFIG_H -D__CFG_SYS_MMFBL -D__CFG_SYS_DDF
HX_FLAGS += $(HX_INCLUDES)
HX_FLAGS += -Wall

HXLDFLAGS =
HXLDFLAGS += -nostdlib -nostdinc -m32 -e__OS_Entry -Ttext 0x110000
HXLDFLAGS += $(HX_FLAGS)

LDADD = 
LDADD += ./osentry/libosentry.a
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

bin_PROGRAMS = hellox_kernel
hellox_kernel_SOURCES = 
#hellox_kernel_CPPFLAGS = $(AM_CPPFLAGS)
hellox_kernel_LDFLAGS = $(HXLDFLAGS)
hellox_kernel_LDADD = $(LDADD)
