noinst_LIBRARIES = libkthread.a
libkthread_a_SOURCES =idle.c  logcat.c

AM_CPPFLAGS =-nostdlib -m32 -D_POSIX_ -I$(top_srcdir)/kernel/include -I$(top_srcdir)/kernel/config  -I$(top_srcdir)/kernel/lib -I$(top_srcdir)/kernel/lib/sys -D_TIME_T_DEFINED -D_M_IX86
