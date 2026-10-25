/*
 * compat.h - type/macro compatibility shims for the cFS port.
 * Provides standard C types/macros/constants that hxlib's headers do not
 * define yet.  (Candidates for migration into hxlib later.)
 */

#ifndef __CFS_COMPAT_H__
#define __CFS_COMPAT_H__

#include <stddef.h>

/* Difference of two pointers (missing from hxlib stddef.h). */
#ifndef _PTRDIFF_T_DEFINED
#define _PTRDIFF_T_DEFINED
typedef int ptrdiff_t;
#endif

/* Group ID type (missing from hxlib stddef.h, which only has uid_t). */
#ifndef _GID_T_DEFINED
#define _GID_T_DEFINED
typedef int gid_t;
#endif

/*
 * Force 64-bit time_t.  hxlib's time.h defines time_t as "long long"
 * (8 bytes) while hxlib's sys/types.h defines it as "long" (4 bytes).
 * Whichever header is included first wins, so struct stat ends up with
 * a different size depending on include order.  Forcing it here makes
 * time_t (and therefore struct stat) consistent everywhere, matching
 * hxlib's stat() implementation.
 */
#ifndef _TIME_T_DEFINED
#define _TIME_T_DEFINED
typedef long long time_t;
#endif

/* NULL pointer (hxlib defines it in hellox.h, not stddef.h). */
#ifndef NULL
#define NULL ((void *)0)
#endif

/* offsetof macro (missing from hxlib stddef.h). */
#ifndef offsetof
#define offsetof(type, member) ((size_t)&(((type *)0)->member))
#endif

/* Signal-atomic integer type (missing from hxlib signal.h). */
#ifndef _SIG_ATOMIC_T_DEFINED
#define _SIG_ATOMIC_T_DEFINED
typedef int sig_atomic_t;
#endif

/* ENOTSUP is an alias of EOPNOTSUPP (missing from hxlib errno.h). */
#ifndef ENOTSUP
#define ENOTSUP EOPNOTSUPP
#endif

/* ------------------------------------------------------------------ */
/* signal.h constants (hxlib only defines a few bit-flag signals)      */
/* ------------------------------------------------------------------ */
#ifndef SIGHUP
#define SIGHUP 0x20
#endif
#ifndef SIGABRT
#define SIGABRT 0x40
#endif
#ifndef SIGSEGV
#define SIGSEGV 0x80
#endif
#ifndef SIGILL
#define SIGILL 0x100
#endif
#ifndef SIGBUS
#define SIGBUS 0x200
#endif
#ifndef SIGFPE
#define SIGFPE 0x400
#endif
#ifndef SIGUSR2
#define SIGUSR2 0x800
#endif
#ifndef SIGRTMIN
#define SIGRTMIN 0x1000
#endif
#ifndef SIGRTMAX
#define SIGRTMAX 0x2000
#endif

/* ------------------------------------------------------------------ */
/* unistd.h sysconf() arguments                                        */
/* ------------------------------------------------------------------ */
#ifndef _SC_PAGESIZE
#define _SC_PAGESIZE 1
#endif
#ifndef _SC_CLK_TCK
#define _SC_CLK_TCK 2
#endif
#ifndef _SC_SIGQUEUE_MAX
#define _SC_SIGQUEUE_MAX 3
#endif

/* ------------------------------------------------------------------ */
/* pthread / sched additions                                           */
/* ------------------------------------------------------------------ */

/* Priority inheritance mutex protocol (missing from hxlib pthread.h). */
#ifndef PTHREAD_PRIO_INHERIT
#define PTHREAD_PRIO_INHERIT 1
#endif

/* hxlib's struct sched_param uses "__sched_priority" instead of the
 * POSIX name "sched_priority". */
#ifndef sched_priority
#define sched_priority __sched_priority
#endif

/* Scheduling policies (missing from hxlib sched.h). */
#ifndef SCHED_OTHER
#define SCHED_OTHER 0
#endif
#ifndef SCHED_FIFO
#define SCHED_FIFO 1
#endif
#ifndef SCHED_RR
#define SCHED_RR 2
#endif

/* Socket address family type (missing from hxlib sockets.h). */
#ifndef _SA_FAMILY_T_DEFINED
#define _SA_FAMILY_T_DEFINED
typedef unsigned short sa_family_t;
#endif

/* clock_nanosleep flags (missing from hxlib time.h). */
#ifndef TIMER_ABSTIME
#define TIMER_ABSTIME 1
#endif

/* ------------------------------------------------------------------ */
/* cFE target config values (normally provided via CMake -D flags)     */
/* ------------------------------------------------------------------ */
#ifndef CFE_CPU_ID_VALUE
#define CFE_CPU_ID_VALUE 1
#endif
#ifndef CFE_CPU_NAME_VALUE
#define CFE_CPU_NAME_VALUE "cpu1"
#endif
#ifndef CFE_SPACECRAFT_ID_VALUE
#define CFE_SPACECRAFT_ID_VALUE 0x42
#endif

#endif  /* __CFS_COMPAT_H__ */
