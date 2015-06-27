noinst_LIBRARIES = libhellolib.a
libhellolib_a_SOURCES = atox.c   errno.c   io.c    memory.c   pthread_mutex.c  sched.c   signal.c  stdio.c   sysmem.c ctype.c  getenv.c  math.c  pthread.c  pthread_other.c   setjmp.c  stat.c    string.c  time.c

AM_CPPFLAGS =-nostdlib -nostdinc -m32 -D_POSIX_  -D_TIME_T_DEFINED -D_M_IX86
AM_CPPFLAGS +=-I$(top_srcdir)/kernel/include -I$(top_srcdir)/kernel/config -I$(top_srcdir)/kernel/lib/sys
