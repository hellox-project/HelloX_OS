/***********************************************************************/
/*    Module Name   : adapt.c                                          */
/*    Module Funciton:                                                  */
/*       cFS porting adaptation layer for HelloX.                      */
/*       Implements the POSIX functions declared in adapt.h using      */
/*       existing hxlib capabilities (no kernel change).               */
/***********************************************************************/

#include "adapt.h"

#include <hellox.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include "sockets.h"   /* AF_INET, struct in_addr, inet_aton/inet_ntoa */

/* ================================================================== */
/* stdio.h additions                                                   */
/* ================================================================== */

int vsnprintf(char *str, size_t size, const char *format, va_list ap)
{
    char * buff;
    size_t buff_sz;
    int    n;

    /*
     * hxlib's _hx_vsprintf() is unbounded and writes into the caller's
     * buffer.  Use a per-call heap buffer (as _hx_snprintf does) so that
     * concurrent printf calls from different tasks cannot clobber each
     * other, then truncate into the caller's buffer.
     */
    buff_sz = size + 64;
    if (buff_sz < 256)
    {
        buff_sz = 256;
    }
    if (buff_sz > MAX_PRINTF_BUFFER_SIZE)
    {
        return -1;
    }

    buff = (char *)malloc(buff_sz);
    if (buff == NULL)
    {
        return -1;
    }

    n = _hx_vsprintf(buff, format, ap);

    if (n >= 0 && str != NULL && size > 0)
    {
        if ((size_t)n >= size)
        {
            n = (int)size - 1;
        }
        memcpy(str, buff, (size_t)n);
        str[n] = '\0';
    }

    free(buff);

    return n;
}

/* ================================================================== */
/* time.h additions                                                    */
/* ================================================================== */

/* Offset applied on top of hxlib's CLOCK_REALTIME, set by clock_settime.
 * hxlib's clock_gettime(CLOCK_REALTIME) has a fixed baseline taken from
 * the RTC at init, so "setting the time" is expressed as an offset. */
static long long g_realtime_sec_off  = 0;
static long long g_realtime_nsec_off = 0;

int clock_settime(clockid_t clk_id, const struct timespec *tp)
{
    struct timespec now;

    if (clk_id != CLOCK_REALTIME)
    {
        errno = EINVAL;
        return -1;
    }
    if (tp == NULL || tp->tv_nsec < 0 || tp->tv_nsec >= 1000000000L)
    {
        errno = EINVAL;
        return -1;
    }

    /* offset = target - current realtime */
    if (clock_gettime(CLOCK_REALTIME, &now) != 0)
    {
        return -1;
    }

    g_realtime_sec_off  = tp->tv_sec  - now.tv_sec;
    g_realtime_nsec_off = tp->tv_nsec - now.tv_nsec;
    if (g_realtime_nsec_off < 0)
    {
        g_realtime_nsec_off += 1000000000L;
        g_realtime_sec_off  -= 1;
    }

    return 0;
}

int clock_getres(clockid_t clk_id, struct timespec *res)
{
    if (res == NULL)
    {
        errno = EFAULT;
        return -1;
    }
    if (clk_id != CLOCK_REALTIME && clk_id != CLOCK_MONOTONIC)
    {
        errno = EINVAL;
        return -1;
    }

    /* hxlib's clock is TSC-based; report a conservative 1ms resolution. */
    res->tv_sec  = 0;
    res->tv_nsec = 1000000L;

    return 0;
}

int clock_nanosleep(clockid_t clk_id, int flags,
                    const struct timespec *req, struct timespec *rem)
{
    struct timespec now, rel;

    if (req == NULL || req->tv_sec < 0 || req->tv_nsec < 0 ||
        req->tv_nsec >= 1000000000L)
    {
        errno = EINVAL;
        return -1;
    }

    if (flags & TIMER_ABSTIME)
    {
        /* Sleep until absolute time "req" on the given clock. */
        if (clock_gettime(clk_id, &now) != 0)
        {
            return -1;
        }
        rel.tv_sec  = req->tv_sec  - now.tv_sec;
        rel.tv_nsec = req->tv_nsec - now.tv_nsec;
        if (rel.tv_nsec < 0)
        {
            rel.tv_nsec += 1000000000L;
            rel.tv_sec  -= 1;
        }
        if (rel.tv_sec < 0 || (rel.tv_sec == 0 && rel.tv_nsec <= 0))
        {
            /* Deadline already elapsed. */
            if (rem)
            {
                rem->tv_sec  = 0;
                rem->tv_nsec = 0;
            }
            return 0;
        }
        return nanosleep(&rel, rem);
    }

    /* Relative sleep. */
    return nanosleep(req, rem);
}

int adapt_get_realtime(struct timespec *tp)
{
    if (clock_gettime(CLOCK_REALTIME, tp) != 0)
    {
        return -1;
    }

    tp->tv_sec  += g_realtime_sec_off;
    tp->tv_nsec += g_realtime_nsec_off;
    if (tp->tv_nsec >= 1000000000L)
    {
        tp->tv_nsec -= 1000000000L;
        tp->tv_sec  += 1;
    }
    if (tp->tv_nsec < 0)
    {
        tp->tv_nsec += 1000000000L;
        tp->tv_sec  -= 1;
    }

    return 0;
}

/* ================================================================== */
/* unistd.h additions                                                  */
/* ================================================================== */

int getpid(void)
{
    /* HelloX has no per-process PID concept exposed to user space.
     * cFS only uses it for queue naming, which the hellox OSAL does not
     * rely on. Return a constant. */
    return 1;
}

int geteuid(void)
{
    /* No user/permission model; always root. */
    return 0;
}

int getegid(void)
{
    return 0;
}

long sysconf(int name)
{
    switch (name)
    {
    case _SC_PAGESIZE:
        return 4096L;
    case _SC_CLK_TCK:
        return 100L;
    case _SC_NPROCESSORS_ONLN:
        return 1L;
    default:
        errno = EINVAL;
        return -1L;
    }
}

/* ================================================================== */
/* string.h additions                                                  */
/* ================================================================== */

char *strerror(int errnum)
{
    switch (errnum)
    {
    case 0:          return "Success";
    case EPERM:      return "Operation not permitted";
    case ENOENT:     return "No such file or directory";
    case ESRCH:      return "No such process";
    case EINTR:      return "Interrupted system call";
    case EIO:        return "I/O error";
    case ENXIO:      return "No such device or address";
    case EBADF:      return "Bad file number";
    case EAGAIN:     return "Try again";
    case ENOMEM:     return "Out of memory";
    case EACCES:     return "Permission denied";
    case EFAULT:     return "Bad address";
    case EBUSY:      return "Device or resource busy";
    case EEXIST:     return "File exists";
    case ENODEV:     return "No such device";
    case ENOTDIR:    return "Not a directory";
    case EISDIR:     return "Is a directory";
    case EINVAL:     return "Invalid argument";
    case ENFILE:     return "File table overflow";
    case EMFILE:     return "Too many open files";
    case ENOTTY:     return "Not a typewriter";
    case EFBIG:      return "File too large";
    case ENOSPC:     return "No space left on device";
    case ESPIPE:     return "Illegal seek";
    case EROFS:      return "Read-only file system";
    case EPIPE:      return "Broken pipe";
    case ENAMETOOLONG: return "File name too long";
    case ENOSYS:     return "Function not implemented";
    case ENOTEMPTY:  return "Directory not empty";
    case EMSGSIZE:   return "Message too long";
    case ETIMEDOUT:  return "Connection timed out";
    case ECONNREFUSED: return "Connection refused";
    case EADDRINUSE: return "Address already in use";
    case ENOTCONN:   return "Transport endpoint is not connected";
    case ECONNRESET: return "Connection reset by peer";
    default:         return "Unknown error";
    }
}

/* ================================================================== */
/* sched.h additions                                                   */
/* ================================================================== */

int sched_get_priority_max(int policy)
{
    (void)policy;
    /* hxlib thread priority range: PRIORITY_LEVEL_CRITICAL = 16. */
    return PRIORITY_LEVEL_CRITICAL;
}

int sched_get_priority_min(int policy)
{
    (void)policy;
    /* hxlib thread priority range: PRIORITY_LEVEL_IDLE = 0. */
    return PRIORITY_LEVEL_IDLE;
}

/* ================================================================== */
/* arpa/inet.h additions                                               */
/* ================================================================== */

int inet_pton(int af, const char *src, void *dst)
{
    if (af != AF_INET)
    {
        errno = EAFNOSUPPORT;
        return -1;
    }
    if (src == NULL || dst == NULL)
    {
        errno = EFAULT;
        return -1;
    }

    /* hxlib's inet_aton returns 1 on success, 0 on malformed input. */
    return inet_aton(src, (struct in_addr *)dst);
}

const char *inet_ntop(int af, const void *src, char *dst, int size)
{
    const char *s;

    if (af != AF_INET)
    {
        errno = EAFNOSUPPORT;
        return NULL;
    }
    if (src == NULL || dst == NULL || size <= 0)
    {
        errno = EFAULT;
        return NULL;
    }

    s = inet_ntoa(*(const struct in_addr *)src);
    strncpy(dst, s, (size_t)size);
    dst[size - 1] = '\0';

    return dst;
}

/* ================================================================== */
/* signal.h additions (hxlib signal.c is #if 0'd out; provide no-ops)  */
/* ================================================================== */

int sigfillset(sigset_t *set)
{
    if (set == NULL) { errno = EINVAL; return -1; }
    *set = ~(sigset_t)0;
    return 0;
}

int sigaddset(sigset_t *set, int sig)
{
    if (set == NULL) { errno = EINVAL; return -1; }
    *set |= (sigset_t)sig;
    return 0;
}

int sigdelset(sigset_t *set, int sig)
{
    if (set == NULL) { errno = EINVAL; return -1; }
    *set &= ~(sigset_t)sig;
    return 0;
}

int sigemptyset(sigset_t *set)
{
    if (set == NULL) { errno = EINVAL; return -1; }
    *set = 0;
    return 0;
}

int sigismember(const sigset_t *set, int sig)
{
    if (set == NULL) { errno = EINVAL; return -1; }
    return (*set & (sigset_t)sig) ? 1 : 0;
}

int sigprocmask(int how, sigset_t *set, sigset_t *oldset)
{
    /* HelloX: no per-process signal mask; no-op. */
    return 0;
}

int sigwait(const sigset_t *set, int *sig)
{
    /* HelloX: no signals to wait for. */
    errno = ENOSYS;
    return -1;
}

int sigsuspend(sigset_t *mask)
{
    errno = ENOSYS;
    return -1;
}

int sigtimedwait(const sigset_t *set, siginfo_t *info, const struct timespec *timeout)
{
    errno = ENOSYS;
    return -1;
}

int kill(int pid, int sig)
{
    /* HelloX: no signal delivery; no-op. */
    return 0;
}

sighandler_t signal(int sig, sighandler_t handler)
{
    /* HelloX: cannot install handlers; return "previous" (none). */
    return NULL;
}

/* ================================================================== */
/* pthread.h additions (declared in hxlib but not implemented)         */
/* ================================================================== */

int pthread_attr_setschedpolicy(pthread_attr_t *attr, int policy)
{
    return 0;
}

int pthread_attr_setinheritsched(pthread_attr_t *attr, int inherit)
{
    return 0;
}

int pthread_setschedprio(pthread_t thread, int prio)
{
    /* HelloX has no per-thread scheduler priority API; no-op. */
    return 0;
}

int pthread_mutexattr_setprotocol(pthread_mutexattr_t *attr, int protocol)
{
    return 0;
}

int pthread_mutexattr_getprotocol(const pthread_mutexattr_t *attr, int *protocol)
{
    if (protocol != NULL)
    {
        *protocol = PTHREAD_PRIO_INHERIT;
    }
    return 0;
}

int pthread_mutex_timedlock(pthread_mutex_t *mutex, const struct timespec *abstime)
{
    struct timespec now;
    struct timespec tick;

    tick.tv_sec  = 0;
    tick.tv_nsec = 1000000L; /* 1 ms */

    for (;;)
    {
        if (pthread_mutex_trylock(mutex) == 0)
        {
            return 0;
        }

        clock_gettime(CLOCK_REALTIME, &now);
        if (now.tv_sec > abstime->tv_sec ||
            (now.tv_sec == abstime->tv_sec && now.tv_nsec >= abstime->tv_nsec))
        {
            return ETIMEDOUT;
        }

        nanosleep(&tick, NULL);
    }
}

/* ================================================================== */
/* unistd.h additions (network host identity)                          */
/* ================================================================== */

int gethostname(char *name, size_t len)
{
    if (name == NULL || len == 0)
    {
        errno = EINVAL;
        return -1;
    }
    strncpy(name, "hellox", len);
    name[len - 1] = '\0';
    return 0;
}

long gethostid(void)
{
    return 0;
}

/* ================================================================== */
/* fcntl.h / sys/stat.h additions                                      */
/* ================================================================== */

int fcntl(int fd, int cmd, ...)
{
    /* HelloX: socket non-blocking control is not exposed; no-op. */
    (void)fd;
    (void)cmd;
    return 0;
}

int fchmod(int fd, int mode)
{
    (void)fd;
    (void)mode;
    return 0;
}

int statvfs(const char *path, struct statvfs *buf)
{
    if (buf == NULL)
    {
        errno = EFAULT;
        return -1;
    }

    memset(buf, 0, sizeof(*buf));
    buf->f_bsize   = 512;
    buf->f_frsize  = 512;
    buf->f_blocks  = 0x100000;  /* arbitrary */
    buf->f_bfree   = 0x100000;
    buf->f_bavail  = 0x100000;
    buf->f_namemax = 255;

    return 0;
}
