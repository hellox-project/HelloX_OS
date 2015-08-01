/* This is an implementation of the threads API of POSIX 1003.1-2001.
 *
 * --------------------------------------------------------------------------
 *
 *      Pthreads-win32 - POSIX Threads Library for Win32
 *      Copyright(C) 1998 John E. Bossom
 *      Copyright(C) 1999,2005 Pthreads-win32 contributors
 * 
 *      Contact Email: rpj@callisto.canberra.edu.au
 * 
 *      The current list of contributors is contained
 *      in the file CONTRIBUTORS included with the source
 *      code distribution. The list can also be seen at the
 *      following World Wide Web location:
 *      http://sources.redhat.com/pthreads-win32/contributors.html
 * 
 *      This library is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU Lesser General Public
 *      License as published by the Free Software Foundation; either
 *      version 2 of the License, or (at your option) any later version.
 * 
 *      This library is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *      Lesser General Public License for more details.
 * 
 *      You should have received a copy of the GNU Lesser General Public
 *      License along with this library in the file COPYING.LIB;
 *      if not, write to the Free Software Foundation, Inc.,
 *      59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#ifndef __PTHREAD_H__
#define __PTHREAD_H__

#include <time.h>
/*
 * See the README file for an explanation of the pthreads-win32 version
 * numbering scheme and how the DLL is named etc.
 */
#define PTW32_VERSION            2,8,0,0
#define PTW32_VERSION_STRING     "2, 8, 0, 0\0"


//#include <setjmp.h>
//#include <limits.h>



/*
 * Boolean values to make us independent of system includes.
 */
enum {
  PTW32_FALSE = 0,
  PTW32_TRUE = (! PTW32_FALSE)
};

/*
 * This is a duplicate of what is in the autoconf config.h,
 * which is only used when building the pthread-win32 libraries.
 */


//#if PTW32_LEVEL >= PTW32_LEVEL_MAX
//#ifdef NEED_ERRNO
//#include "need_errno.h"
//#else
//#include <errno.h>
//#endif
//#endif /* PTW32_LEVEL >= PTW32_LEVEL_MAX */

/*
 * Several systems don't define some error numbers.
 */
#ifndef ENOTSUP
#  define ENOTSUP 48   /* This is the value in Solaris. */
#endif

#ifndef ETIMEDOUT
#  define ETIMEDOUT 10060     /* This is the value in winsock.h. */
#endif

#ifndef ENOSYS
#  define ENOSYS 140     /* Semi-arbitrary value */
#endif

#ifndef EDEADLK
#  ifdef EDEADLOCK
#    define EDEADLK EDEADLOCK
#  else
#    define EDEADLK 36     /* This is the value in MSVC. */
#  endif
#endif


typedef long   *LPLONG;

#define EINVAL                   -1 
#define EPERM                    1
#define EBUSY                    16
#define LONG                     long 
#define PTW32_LONG               long
#define PTW32_LPLONG             long*
#define INFINITE                 0xFFFFFFFF 

//#include <sched.h>

#ifndef SIG_BLOCK
#define SIG_BLOCK 0
#endif /* SIG_BLOCK */

#ifndef SIG_UNBLOCK 
#define SIG_UNBLOCK 1
#endif /* SIG_UNBLOCK */

#ifndef SIG_SETMASK
#define SIG_SETMASK 2
#endif /* SIG_SETMASK */

#ifdef __cplusplus
extern "C"
{
#endif                          /* __cplusplus */

/*
 * -------------------------------------------------------------
 *
 * POSIX 1003.1-2001 Options
 * =========================
 *
 * Options are normally set in <unistd.h>, which is not provided
 * with pthreads-win32.
 *
 * For conformance with the Single Unix Specification (version 3), all of the
 * options below are defined, and have a value of either -1 (not supported)
 * or 200112L (supported).
 *
 * These options can neither be left undefined nor have a value of 0, because
 * either indicates that sysconf(), which is not implemented, may be used at
 * runtime to check the status of the option.
 *
 * _GCC_THREADS (== 200112L)
 *                      If == 200112L, you can use threads
 *
 * _GCC_THREAD_ATTR_STACKSIZE (== 200112L)
 *                      If == 200112L, you can control the size of a thread's
 *                      stack
 *                              pthread_attr_getstacksize
 *                              pthread_attr_setstacksize
 *
 * _GCC_THREAD_ATTR_STACKADDR (== -1)
 *                      If == 200112L, you can allocate and control a thread's
 *                      stack. If not supported, the following functions
 *                      will return ENOSYS, indicating they are not
 *                      supported:
 *                              pthread_attr_getstackaddr
 *                              pthread_attr_setstackaddr
 *
 * _GCC_THREAD_PRIORITY_SCHEDULING (== -1)
 *                      If == 200112L, you can use realtime scheduling.
 *                      This option indicates that the behaviour of some
 *                      implemented functions conforms to the additional TPS
 *                      requirements in the standard. E.g. rwlocks favour
 *                      writers over readers when threads have equal priority.
 *
 * _GCC_THREAD_PRIO_INHERIT (== -1)
 *                      If == 200112L, you can create priority inheritance
 *                      mutexes.
 *                              pthread_mutexattr_getprotocol +
 *                              pthread_mutexattr_setprotocol +
 *
 * _GCC_THREAD_PRIO_PROTECT (== -1)
 *                      If == 200112L, you can create priority ceiling mutexes
 *                      Indicates the availability of:
 *                              pthread_mutex_getprioceiling
 *                              pthread_mutex_setprioceiling
 *                              pthread_mutexattr_getprioceiling
 *                              pthread_mutexattr_getprotocol     +
 *                              pthread_mutexattr_setprioceiling
 *                              pthread_mutexattr_setprotocol     +
 *
 * _GCC_THREAD_PROCESS_SHARED (== -1)
 *                      If set, you can create mutexes and condition
 *                      variables that can be shared with another
 *                      process.If set, indicates the availability
 *                      of:
 *                              pthread_mutexattr_getpshared
 *                              pthread_mutexattr_setpshared
 *                              pthread_condattr_getpshared
 *                              pthread_condattr_setpshared
 *
 * _GCC_THREAD_SAFE_FUNCTIONS (== 200112L)
 *                      If == 200112L you can use the special *_r library
 *                      functions that provide thread-safe behaviour
 *
 * _GCC_READER_WRITER_LOCKS (== 200112L)
 *                      If == 200112L, you can use read/write locks
 *
 * _GCC_SPIN_LOCKS (== 200112L)
 *                      If == 200112L, you can use spin locks
 *
 * _GCC_BARRIERS (== 200112L)
 *                      If == 200112L, you can use barriers
 *
 *      + These functions provide both 'inherit' and/or
 *        'protect' protocol, based upon these macro
 *        settings.
 *
 * -------------------------------------------------------------
 */

/*
 * POSIX Options
 */
#undef _GCC_THREADS
#define _GCC_THREADS 200112L

#undef _GCC_READER_WRITER_LOCKS
#define _GCC_READER_WRITER_LOCKS 200112L

#undef _GCC_SPIN_LOCKS
#define _GCC_SPIN_LOCKS 200112L

#undef _GCC_BARRIERS
#define _GCC_BARRIERS 200112L

#undef _GCC_THREAD_SAFE_FUNCTIONS
#define _GCC_THREAD_SAFE_FUNCTIONS 200112L

#undef _GCC_THREAD_ATTR_STACKSIZE
#define _GCC_THREAD_ATTR_STACKSIZE 200112L

/*
 * The following options are not supported
 */
#undef _GCC_THREAD_ATTR_STACKADDR
#define _GCC_THREAD_ATTR_STACKADDR -1

#undef _GCC_THREAD_PRIO_INHERIT
#define _GCC_THREAD_PRIO_INHERIT -1

#undef _GCC_THREAD_PRIO_PROTECT
#define _GCC_THREAD_PRIO_PROTECT -1

/* TPS is not fully supported.  */
#undef _GCC_THREAD_PRIORITY_SCHEDULING
#define _GCC_THREAD_PRIORITY_SCHEDULING -1

#undef _GCC_THREAD_PROCESS_SHARED
#define _GCC_THREAD_PROCESS_SHARED -1


/*
 * POSIX 1003.1-2001 Limits
 * ===========================
 *
 * These limits are normally set in <limits.h>, which is not provided with
 * pthreads-win32.
 *
 * PTHREAD_DESTRUCTOR_ITERATIONS
 *                      Maximum number of attempts to destroy
 *                      a thread's thread-specific data on
 *                      termination (must be at least 4)
 *
 * PTHREAD_KEYS_MAX
 *                      Maximum number of thread-specific data keys
 *                      available per process (must be at least 128)
 *
 * PTHREAD_STACK_MIN
 *                      Minimum supported stack size for a thread
 *
 * PTHREAD_THREADS_MAX
 *                      Maximum number of threads supported per
 *                      process (must be at least 64).
 *
 * SEM_NSEMS_MAX
 *                      The maximum number of semaphores a process can have.
 *                      (must be at least 256)
 *
 * SEM_VALUE_MAX
 *                      The maximum value a semaphore can have.
 *                      (must be at least 32767)
 *
 */
#undef _GCC_THREAD_DESTRUCTOR_ITERATIONS
#define _GCC_THREAD_DESTRUCTOR_ITERATIONS     4

#undef PTHREAD_DESTRUCTOR_ITERATIONS
#define PTHREAD_DESTRUCTOR_ITERATIONS           _GCC_THREAD_DESTRUCTOR_ITERATIONS

#undef _GCC_THREAD_KEYS_MAX
#define _GCC_THREAD_KEYS_MAX                  128

#undef PTHREAD_KEYS_MAX
#define PTHREAD_KEYS_MAX                        _GCC_THREAD_KEYS_MAX

#undef PTHREAD_STACK_MIN
#define PTHREAD_STACK_MIN                       0

#undef _GCC_THREAD_THREADS_MAX
#define _GCC_THREAD_THREADS_MAX               64

  /* Arbitrary value */
#undef PTHREAD_THREADS_MAX
#define PTHREAD_THREADS_MAX                     2019

#undef _GCC_SEM_NSEMS_MAX
#define _GCC_SEM_NSEMS_MAX                    256

  /* Arbitrary value */
#undef SEM_NSEMS_MAX
#define SEM_NSEMS_MAX                           1024

#undef _GCC_SEM_VALUE_MAX
#define _GCC_SEM_VALUE_MAX                    32767

#undef SEM_VALUE_MAX
#define SEM_VALUE_MAX                           INT_MAX


/*
 * The Open Watcom C/C++ compiler uses a non-standard calling convention
 * that passes function args in registers unless __cdecl is explicitly specified
 * in exposed function prototypes.
 *
 * We force all calls to cdecl even though this could slow Watcom code down
 * slightly. If you know that the Watcom compiler will be used to build both
 * the DLL and application, then you can probably define this as a null string.
 * Remember that pthread.h (this file) is used for both the DLL and application builds.
 */
#define  __cdecl

#if defined(_UWIN) && PTW32_LEVEL >= PTW32_LEVEL_MAX
#   include     <sys/types.h>
#else
/*
 * Generic handle type - intended to extend uniqueness beyond
 * that available with a simple pointer. It should scale for either
 * IA-32 or IA-64.
 */
typedef struct {
    void * p;                   /* Pointer to actual object */
    unsigned int x;             /* Extra information - reuse count etc */
} ptw32_handle_t;

struct sched_param
{
  int __sched_priority;
};

struct pthread_mutex_t_
{
  int  lock_idx;   /* Provides exclusive access to mutex state
				   via the Interlocked* mechanism.
				    0: unlocked/free.
				    1: locked - no other waiters.
				   -1: locked - with possible other waiters.
				   */
  int recursive_count;		/* Number of unlocks a thread needs to perform
				   before the lock is released (recursive
				   mutexes only). */
  int kind;			/* Mutex type. */
  ptw32_handle_t ownerThread;
  void* event;			/* Mutex release notification to waiting threads. */
};

struct pthread_cond_t_
{
	void*    cond;
};

//Thread attributes.
struct pthread_attr_t_{
	void*      stack_ptr;
	size_t     stack_size;
	int        detach_state;
	int        priority;
};

typedef HANDLE pthread_t;
typedef struct pthread_attr_t_ * pthread_attr_t;
typedef struct pthread_once_t_ pthread_once_t;
typedef DWORD pthread_key_t;  //HelloX use DWORD as TLS key.
typedef struct pthread_mutex_t_ * pthread_mutex_t;
typedef struct pthread_mutexattr_t_ * pthread_mutexattr_t;
typedef struct pthread_cond_t_ * pthread_cond_t;
typedef struct pthread_condattr_t_ * pthread_condattr_t;
#endif
typedef struct pthread_rwlock_t_ * pthread_rwlock_t;
typedef struct pthread_rwlockattr_t_ * pthread_rwlockattr_t;
typedef struct pthread_spinlock_t_ * pthread_spinlock_t;
typedef struct pthread_barrier_t_ * pthread_barrier_t;
typedef struct pthread_barrierattr_t_ * pthread_barrierattr_t;

/*
 * ====================
 * ====================
 * POSIX Threads
 * ====================
 * ====================
 */

enum {
/*
 * pthread_attr_{get,set}detachstate
 */
  PTHREAD_CREATE_JOINABLE       = 0,  /* Default */
  PTHREAD_CREATE_DETACHED       = 1,

/*
 * pthread_attr_{get,set}inheritsched
 */
  PTHREAD_INHERIT_SCHED         = 0,
  PTHREAD_EXPLICIT_SCHED        = 1,  /* Default */

/*
 * pthread_{get,set}scope
 */
  PTHREAD_SCOPE_PROCESS         = 0,
  PTHREAD_SCOPE_SYSTEM          = 1,  /* Default */

/*
 * pthread_setcancelstate paramters
 */
  PTHREAD_CANCEL_ENABLE         = 0,  /* Default */
  PTHREAD_CANCEL_DISABLE        = 1,

/*
 * pthread_setcanceltype parameters
 */
  PTHREAD_CANCEL_ASYNCHRONOUS   = 0,
  PTHREAD_CANCEL_DEFERRED       = 1,  /* Default */

/*
 * pthread_mutexattr_{get,set}pshared
 * pthread_condattr_{get,set}pshared
 */
  PTHREAD_PROCESS_PRIVATE       = 0,
  PTHREAD_PROCESS_SHARED        = 1,

/*
 * pthread_barrier_wait
 */
  PTHREAD_BARRIER_SERIAL_THREAD = -1
};

/*
 * ====================
 * ====================
 * Cancelation
 * ====================
 * ====================
 */
#define PTHREAD_CANCELED       ((void *) -1)


/*
 * ====================
 * ====================
 * Once Key
 * ====================
 * ====================
 */
#define PTHREAD_ONCE_INIT       { PTW32_FALSE, 0, 0, 0}

struct pthread_once_t_
{
  int          done;        /* indicates if user function has been executed */
  void *       lock;
  int          reserved1;
  int          reserved2;
};


/*
 * ====================
 * ====================
 * Object initialisers
 * ====================
 * ====================
 */
#define PTHREAD_MUTEX_INITIALIZER ((pthread_mutex_t) -1)
#define PTHREAD_RECURSIVE_MUTEX_INITIALIZER ((pthread_mutex_t) -2)
#define PTHREAD_ERRORCHECK_MUTEX_INITIALIZER ((pthread_mutex_t) -3)

/*
 * Compatibility with LinuxThreads
 */
#define PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP PTHREAD_RECURSIVE_MUTEX_INITIALIZER
#define PTHREAD_ERRORCHECK_MUTEX_INITIALIZER_NP PTHREAD_ERRORCHECK_MUTEX_INITIALIZER

#define PTHREAD_COND_INITIALIZER ((pthread_cond_t) -1)

#define PTHREAD_RWLOCK_INITIALIZER ((pthread_rwlock_t) -1)

#define PTHREAD_SPINLOCK_INITIALIZER ((pthread_spinlock_t) -1)


/*
 * Mutex types.
 */
enum
{
  /* Compatibility with LinuxThreads */
  PTHREAD_MUTEX_FAST_NP,
  PTHREAD_MUTEX_RECURSIVE_NP,
  PTHREAD_MUTEX_ERRORCHECK_NP,
  PTHREAD_MUTEX_TIMED_NP = PTHREAD_MUTEX_FAST_NP,
  PTHREAD_MUTEX_ADAPTIVE_NP = PTHREAD_MUTEX_FAST_NP,
  /* For compatibility with POSIX */
  PTHREAD_MUTEX_NORMAL = PTHREAD_MUTEX_FAST_NP,
  PTHREAD_MUTEX_RECURSIVE = PTHREAD_MUTEX_RECURSIVE_NP,
  PTHREAD_MUTEX_ERRORCHECK = PTHREAD_MUTEX_ERRORCHECK_NP,
  PTHREAD_MUTEX_DEFAULT = PTHREAD_MUTEX_NORMAL
};

typedef struct ptw32_cleanup_t ptw32_cleanup_t;
typedef void (*  ptw32_cleanup_callback_t)(void *);

struct ptw32_cleanup_t
{
  ptw32_cleanup_callback_t routine;
  void *arg;
  struct ptw32_cleanup_t *prev;
};

#ifdef __CLEANUP_SEH
        /*
         * WIN32 SEH version of cancel cleanup.
         */

#define pthread_cleanup_push( _rout, _arg ) \
        { \
            ptw32_cleanup_t     _cleanup; \
            \
        _cleanup.routine        = (ptw32_cleanup_callback_t)(_rout); \
            _cleanup.arg        = (_arg); \
            __try \
              { \

#define pthread_cleanup_pop( _execute ) \
              } \
            __finally \
                { \
                    if( _execute || AbnormalTermination()) \
                      { \
                          (*(_cleanup.routine))( _cleanup.arg ); \
                      } \
                } \
        }

#else /* __CLEANUP_SEH */

#ifdef __CLEANUP_C

        /*
         * C implementation of PThreads cancel cleanup
         */

#define pthread_cleanup_push( _rout, _arg ) \
        { \
            ptw32_cleanup_t     _cleanup; \
            \
            ptw32_push_cleanup( &_cleanup, (ptw32_cleanup_callback_t) (_rout), (_arg) ); \

#define pthread_cleanup_pop( _execute ) \
            (void) ptw32_pop_cleanup( _execute ); \
        }

#else /* __CLEANUP_C */

#ifdef __CLEANUP_CXX

        /*
         * C++ version of cancel cleanup.
         * - John E. Bossom.
         */

        class PThreadCleanup {
          /*
           * PThreadCleanup
           *
           * Purpose
           *      This class is a C++ helper class that is
           *      used to implement pthread_cleanup_push/
           *      pthread_cleanup_pop.
           *      The destructor of this class automatically
           *      pops the pushed cleanup routine regardless
           *      of how the code exits the scope
           *      (i.e. such as by an exception)
           */
      ptw32_cleanup_callback_t cleanUpRout;
          void    *       obj;
          int             executeIt;

        public:
          PThreadCleanup() :
            cleanUpRout( 0 ),
            obj( 0 ),
            executeIt( 0 )
            /*
             * No cleanup performed
             */
            {
            }

          PThreadCleanup(
             ptw32_cleanup_callback_t routine,
                         void    *       arg ) :
            cleanUpRout( routine ),
            obj( arg ),
            executeIt( 1 )
            /*
             * Registers a cleanup routine for 'arg'
             */
            {
            }

          ~PThreadCleanup()
            {
              if ( executeIt && ((void *) cleanUpRout != (void *) 0) )
                {
                  (void) (*cleanUpRout)( obj );
                }
            }

          void execute( int exec )
            {
              executeIt = exec;
            }
        };

        /*
         * C++ implementation of PThreads cancel cleanup;
         * This implementation takes advantage of a helper
         * class who's destructor automatically calls the
         * cleanup routine if we exit our scope weirdly
         */
#define pthread_cleanup_push( _rout, _arg ) \
        { \
            PThreadCleanup  cleanup((ptw32_cleanup_callback_t)(_rout), \
                                    (void *) (_arg) );

#define pthread_cleanup_pop( _execute ) \
            cleanup.execute( _execute ); \
        }

#else


#endif /* __CLEANUP_CXX */

#endif /* __CLEANUP_C */

#endif /* __CLEANUP_SEH */

/*
 * ===============
 * ===============
 * Methods
 * ===============
 * ===============
 */

/*
 * PThread Attribute Functions
 */
 int  pthread_attr_init (pthread_attr_t * attr);

 int  pthread_attr_destroy (pthread_attr_t * attr);

 int  pthread_attr_getdetachstate (const pthread_attr_t * attr,int *detachstate);

 int  pthread_attr_getstackaddr (const pthread_attr_t * attr,void **stackaddr);

 int  pthread_attr_getstacksize (const pthread_attr_t * attr, size_t * stacksize);

 int  pthread_attr_setdetachstate (pthread_attr_t * attr,int detachstate);

 int  pthread_attr_setstackaddr (pthread_attr_t * attr, void *stackaddr);

 int  pthread_attr_setstacksize (pthread_attr_t * attr, size_t stacksize);

 int  pthread_attr_getschedparam (const pthread_attr_t *attr, struct sched_param *param);

 int  pthread_attr_setschedparam (pthread_attr_t *attr, const struct sched_param *param);

 int  pthread_attr_setschedpolicy (pthread_attr_t *, int);

 int  pthread_attr_getschedpolicy (pthread_attr_t *, int *);

 int  pthread_attr_setinheritsched(pthread_attr_t * attr, int inheritsched);

 int  pthread_attr_getinheritsched(pthread_attr_t * attr,int * inheritsched);

 int  pthread_attr_setscope (pthread_attr_t *, int);

 int  pthread_attr_getscope (const pthread_attr_t *,int *);

/*
 * PThread Functions
 */
 int  pthread_create (pthread_t * tid,
                            const pthread_attr_t * attr,
                            void *(*start) (void *),
                            void *arg);

 int  pthread_detach (pthread_t tid);

 int  pthread_equal (pthread_t t1, pthread_t t2);

 void  pthread_exit (void *value_ptr);

 int  pthread_join (pthread_t thread, void **value_ptr);

 pthread_t  pthread_self (void);

 int  pthread_cancel (pthread_t thread);

 int  pthread_setcancelstate (int state, int *oldstate);

 int  pthread_setcanceltype (int type, int *oldtype);

 void  pthread_testcancel (void);

 int  pthread_once (pthread_once_t * once_control,  void (*init_routine) (void));


/*
 * Thread Specific Data Functions
 */
 int  pthread_key_create (pthread_key_t * key,void (*destructor) (void *));

 int  pthread_key_delete (pthread_key_t key);

 int  pthread_setspecific (pthread_key_t key, const void *value);

 void *  pthread_getspecific (pthread_key_t key);


/*
 * Barrier Attribute Functions
 */
 int  pthread_barrierattr_init (pthread_barrierattr_t * attr);

 int  pthread_barrierattr_destroy (pthread_barrierattr_t * attr);

 int  pthread_barrierattr_getpshared (const pthread_barrierattr_t *attr,int *pshared);

 int  pthread_barrierattr_setpshared (pthread_barrierattr_t * attr, int pshared);

/*
 * Mutex Functions
 */
 
 int  pthread_mutexattr_init (pthread_mutexattr_t * attr);

 int  pthread_mutexattr_destroy (pthread_mutexattr_t * attr);

 int  pthread_mutexattr_getpshared (const pthread_mutexattr_t *attr,int *pshared);

 int  pthread_mutexattr_setpshared (pthread_mutexattr_t * attr,int pshared);

 int  pthread_mutexattr_settype (pthread_mutexattr_t * attr, int kind);
 int  pthread_mutexattr_gettype (pthread_mutexattr_t * attr, int *kind);

 int  pthread_mutex_init (pthread_mutex_t * mutex,const pthread_mutexattr_t * attr);

 int  pthread_mutex_destroy (pthread_mutex_t * mutex);

 int  pthread_mutex_lock (pthread_mutex_t * mutex);

 int  pthread_mutex_timedlock(pthread_mutex_t *mutex, const struct timespec *abstime);

 int  pthread_mutex_trylock (pthread_mutex_t * mutex);

 int  pthread_mutex_unlock (pthread_mutex_t * mutex);

/*
 * Spinlock Functions
 */
 int  pthread_spin_init (pthread_spinlock_t * lock, int pshared);

 int  pthread_spin_destroy (pthread_spinlock_t * lock);

 int  pthread_spin_lock (pthread_spinlock_t * lock);

 int  pthread_spin_trylock (pthread_spinlock_t * lock);

 int  pthread_spin_unlock (pthread_spinlock_t * lock);

/*
 * Barrier Functions
 */
 int  pthread_barrier_init (pthread_barrier_t * barrier, const pthread_barrierattr_t * attr, unsigned int count);

 int  pthread_barrier_destroy (pthread_barrier_t * barrier);

 int  pthread_barrier_wait (pthread_barrier_t * barrier);

/*
 * Condition Variable Attribute Functions
 */
 int  pthread_condattr_init (pthread_condattr_t * attr);

 int  pthread_condattr_destroy (pthread_condattr_t * attr);

 int  pthread_condattr_getpshared (const pthread_condattr_t * attr,int *pshared);

 int  pthread_condattr_setpshared (pthread_condattr_t * attr, int pshared);

/*
 * Condition Variable Functions
 */
 int  pthread_cond_init (pthread_cond_t * cond, const pthread_condattr_t * attr);

 int  pthread_cond_destroy (pthread_cond_t * cond);

 int  pthread_cond_wait (pthread_cond_t * cond, pthread_mutex_t * mutex);

 int  pthread_cond_timedwait (pthread_cond_t * cond,pthread_mutex_t * mutex, const struct timespec *abstime);

 int  pthread_cond_signal (pthread_cond_t * cond);

 int  pthread_cond_broadcast (pthread_cond_t * cond);

/*
 * Scheduling
 */
 int  pthread_setschedparam (pthread_t thread, int policy, const struct sched_param *param);

 int  pthread_getschedparam (pthread_t thread, int *policy,struct sched_param *param);

 int  pthread_setconcurrency (int);
 
 int  pthread_getconcurrency (void);

/*
 * Read-Write Lock Functions
 */
 int  pthread_rwlock_init(pthread_rwlock_t *lock, const pthread_rwlockattr_t *attr);

 int  pthread_rwlock_destroy(pthread_rwlock_t *lock);

 int  pthread_rwlock_tryrdlock(pthread_rwlock_t *);

 int  pthread_rwlock_trywrlock(pthread_rwlock_t *);

 int  pthread_rwlock_rdlock(pthread_rwlock_t *lock);

 int  pthread_rwlock_timedrdlock(pthread_rwlock_t *lock, const struct timespec *abstime);

 int  pthread_rwlock_wrlock(pthread_rwlock_t *lock);

 int  pthread_rwlock_timedwrlock(pthread_rwlock_t *lock,const struct timespec *abstime);

 int  pthread_rwlock_unlock(pthread_rwlock_t *lock);

 int  pthread_rwlockattr_init (pthread_rwlockattr_t * attr);

 int  pthread_rwlockattr_destroy (pthread_rwlockattr_t * attr);

 int  pthread_rwlockattr_getpshared (const pthread_rwlockattr_t * attr, int *pshared);

 int  pthread_rwlockattr_setpshared (pthread_rwlockattr_t * attr, int pshared);

/*
 * Signal Functions. Should be defined in <signal.h> but MSVC and MinGW32
 * already have signal.h that don't define these.
 */
 int  pthread_kill(pthread_t thread, int sig);

/*
 * Non-portable functions
 */

/*
 * Compatibility with Linux.
 */
 int  pthread_mutexattr_setkind_np(pthread_mutexattr_t * attr, int kind);
 int  pthread_mutexattr_getkind_np(pthread_mutexattr_t * attr, int *kind);

/*
 * Possibly supported by other POSIX threads implementations
 */
 int  pthread_delay_np (struct timespec * interval);
 int  pthread_num_processors_np(void);

 
#ifdef __cplusplus
}                               /* End of extern "C" */
#endif                          /* __cplusplus */

#ifdef PTW32__HANDLE_DEF
# undef HANDLE
#endif
#ifdef PTW32__DWORD_DEF
# undef DWORD
#endif

#undef PTW32_LEVEL
#undef PTW32_LEVEL_MAX

#endif /* ! RC_INVOKED */

//#endif /* PTHREAD_H */
