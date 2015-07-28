AM_CFLAGS=-nostdlib -nostdinc -fno-builtin @cflags@
AM_CFLAGS += -D__GCC__ -D_POSIX_ -D_M_IX86 -DMEM_LIBC_MALLOC 
AM_CFLAGS += -I$(top_srcdir)/kernel/include -I$(top_srcdir)/kernel/config -I$(top_srcdir)/kernel/lib/sys -I$(top_srcdir)/kernel/lib
