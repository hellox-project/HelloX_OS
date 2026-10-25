/***********************************************************************/
/*    Module Name   : adapt.h                                          */
/*    Module Funciton:                                                  */
/*       cFS porting adaptation layer for HelloX.                      */
/*       Declares POSIX functions that hxlib does NOT provide yet.     */
/*       These are implemented in adapt.c using existing hxlib          */
/*       capabilities, so the OSAL "hellox" port can be written        */
/*       against a POSIX-like surface without touching hxlib.          */
/*       (May be migrated into hxlib later.)                           */
/***********************************************************************/

#ifndef __CFS_ADAPT_H__
#define __CFS_ADAPT_H__

#include <hellox.h>   /* HANDLE, etc. (must precede pthread.h) */
#include <stddef.h>    /* size_t */
#include <time.h>      /* clockid_t, struct timespec */
#include <stdarg.h>    /* va_list */
#include <signal.h>    /* sigset_t, siginfo_t */
#include <pthread.h>   /* pthread_t, pthread_mutex_t */
#include <sys/statvfs.h> /* struct statvfs, statvfs */

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------ */
/* signal.h additions (not declared in hxlib)                          */
/* ------------------------------------------------------------------ */

typedef void (*sighandler_t)(int);

int      sigismember(const sigset_t *set, int sig);
int      sigtimedwait(const sigset_t *set, siginfo_t *info, const struct timespec *timeout);
int      kill(int pid, int sig);
sighandler_t signal(int sig, sighandler_t handler);

/* ------------------------------------------------------------------ */
/* pthread.h additions (not declared in hxlib)                         */
/* ------------------------------------------------------------------ */

int pthread_setschedprio(pthread_t thread, int prio);
int pthread_mutexattr_setprotocol(pthread_mutexattr_t *attr, int protocol);
int pthread_mutexattr_getprotocol(const pthread_mutexattr_t *attr, int *protocol);

/* ------------------------------------------------------------------ */
/* unistd.h additions (network host identity)                          */
/* ------------------------------------------------------------------ */

int  gethostname(char *name, size_t len);
long gethostid(void);

/* ------------------------------------------------------------------ */
/* fcntl.h additions                                                   */
/* ------------------------------------------------------------------ */

int fcntl(int fd, int cmd, ...);
int fchmod(int fd, int mode);

/* ------------------------------------------------------------------ */
/* stdio.h additions                                                   */
/* ------------------------------------------------------------------ */

int vsnprintf(char *str, size_t size, const char *format, va_list ap);

/* ------------------------------------------------------------------ */
/* time.h additions                                                    */
/* ------------------------------------------------------------------ */

/* Set the value of the given clock (CLOCK_REALTIME). */
int clock_settime(clockid_t clk_id, const struct timespec *tp);

/* Get the resolution of the given clock. */
int clock_getres(clockid_t clk_id, struct timespec *res);

/* High-resolution sleep; supports relative (flags==0) and absolute
 * (flags==TIMER_ABSTIME) modes. */
int clock_nanosleep(clockid_t clk_id, int flags,
                    const struct timespec *req, struct timespec *rem);

#define TIMER_ABSTIME 1

/* Read CLOCK_REALTIME, applying the offset set by clock_settime().
 * (hxlib's clock_gettime() has a fixed realtime baseline; clock_settime
 *  cannot modify it, so the offset is kept here and exposed to the OSAL.) */
int adapt_get_realtime(struct timespec *tp);

/* ------------------------------------------------------------------ */
/* unistd.h additions                                                  */
/* ------------------------------------------------------------------ */

int getpid(void);
int geteuid(void);
int getegid(void);

#define _SC_PAGESIZE         1
#define _SC_CLK_TCK          2
#define _SC_NPROCESSORS_ONLN 3

long sysconf(int name);

/* ------------------------------------------------------------------ */
/* string.h additions                                                  */
/* ------------------------------------------------------------------ */

char *strerror(int errnum);

/* ------------------------------------------------------------------ */
/* sched.h additions                                                   */
/* ------------------------------------------------------------------ */

#define SCHED_OTHER 0
#define SCHED_FIFO  1
#define SCHED_RR    2

int sched_get_priority_max(int policy);
int sched_get_priority_min(int policy);

/* ------------------------------------------------------------------ */
/* arpa/inet.h additions (IPv4 address text<->binary conversion)       */
/* ------------------------------------------------------------------ */

int inet_pton(int af, const char *src, void *dst);
const char *inet_ntop(int af, const void *src, char *dst, int size);

#ifdef __cplusplus
}
#endif

#endif  /* __CFS_ADAPT_H__ */
