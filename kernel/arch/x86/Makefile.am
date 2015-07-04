noinst_LIBRARIES = libarch.a
libarch_a_SOURCES = arch_x86.c  bios.c  biosvga.c  hellocn.c


AM_CPPFLAGS = -nostdlib -nostdinc -m32  -D_M_IX86
AM_CPPFLAGS += -I$(top_srcdir)/kernel/include -I$(top_srcdir)/kernel/config -I$(top_srcdir)/kernel/lib 

