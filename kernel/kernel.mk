AM_CFLAGS = -nostdlib -nostdinc -m32 -fno-builtin
AM_CFLAGS += -D_GCC_ -D_POSIX_ -D_M_IX86 -DMEM_LIBC_MALLOC 
AM_CFLAGS += -I$(top_srcdir)/kernel/include -I$(top_srcdir)/kernel/config -I$(top_srcdir)/kernel/lib/sys -I$(top_srcdir)/kernel/lib
