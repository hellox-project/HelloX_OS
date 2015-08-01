SUBDIRS = kernel kapi kthread network shell lib osentry fs @hx_arch@ @hx_drivers@

ACLOCAL_AMFLAGS = -I m4
AUTOMAKE_OPTIONS = foreign



HX_INCLUDES =

HX_INCLUDES += -I$(top_srcdir)/kernel/include 
HX_INCLUDES += -I$(top_srcdir)/kernel/config 
HX_INCLUDES += -I$(top_srcdir)/kernel/lib/sys -I$(top_srcdir)/kernel/lib

HX_FLAGS =
HX_FLAGS += -nostdlib  -nostdinc -fno-stack-protector -fno-builtin -m32 
<<<<<<< HEAD
#HX_FLAGS += -D_M_IX86 -DHAVE_CONFIG_H -D__CFG_SYS_MMFBL -D__CFG_SYS_DDF
#HX_FLAGS += $(HX_INCLUDES)
#HX_FLAGS += -Wall
=======
HX_FLAGS += -D_M_IX86 -DHAVE_CONFIG_H -D__CFG_SYS_MMFBL -D__CFG_SYS_DDF
HX_FLAGS += $(HX_INCLUDES)
HX_FLAGS += -Wall
>>>>>>> e4ce2ac6f835fcfc9cd6ca449da9c14125af784a


HXLDFLAGS = -e__OS_Entry -Ttext 0x110000
HXLDFLAGS += -nostdlib -nostdinc -m32  #-T$(top_srcdir)/kernel/scripts/kernel.ld
HXLDFLAGS += $(HX_FLAGS)

LDADD = 
LDADD += ./osentry/libosentry.a
LDADD += ./lib/libhellolib.a
LDADD += ./$(hx_drivers)/libdrivers.a
LDADD += ./shell/libshell.a
LDADD += ./network/libnetwork.a
LDADD += ./kthread/libkthread.a
LDADD += ./$(hx_arch)/libarch.a
LDADD += ./fs/libfs.a
LDADD += ./kernel/libkernel.a
LDADD += ./kapi/libkapi.a


AM_CFLAGS=$(HX_FLAGS)

bin_PROGRAMS = kernel.elf
kernel_elf_SOURCES = 
#kernel_elf_CPPFLAGS = $(AM_CPPFLAGS)
kernel_elf_LDFLAGS = $(HXLDFLAGS)
kernel_elf_LDADD = $(LDADD)
