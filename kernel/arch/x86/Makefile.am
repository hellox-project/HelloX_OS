noinst_LIBRARIES = libarch.a
libarch_a_SOURCES = arch_x86.c  bios.c  biosvga.c  hellocn.c bootsect.asm  hdbs.asm  miniker.asm  ntfsbs.asm  realinit.asm  realinit_g.asm


AM_CPPFLAGS=-nostdlib -m32 -D_POSIX_ -I$(top_srcdir)/kernel/include -I$(top_srcdir)/kernel/config -I$(top_srcdir)/kernel/lib/sys -D_TIME_T_DEFINED -D_M_IX86
