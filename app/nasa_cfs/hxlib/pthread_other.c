//***********************************************************************/
//    Author                    : Garry
//    Original Date             : April 17,2015
//    Module Name               : pthread_other.c
//    Module Funciton           : 
//                                Simulates POSIX pthread operations, which
//                                are widely refered by applications. Some of
//                                the pthread lib routines just keep space since
//                                HelloX kernel does not support them.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#include <hellox.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <pthread.h>
#include <time.h>
#include <stdlib.h>

/*
 * Spinlock Functions
 */
 int  pthread_spin_init (pthread_spinlock_t * lock, int pshared)
 {
	 return S_OK;
 }

 int  pthread_spin_destroy (pthread_spinlock_t * lock)
 {
	 return S_OK;
 }

 int  pthread_spin_lock (pthread_spinlock_t * lock)
 {
	 return S_OK;
 }

 int  pthread_spin_trylock (pthread_spinlock_t * lock)
 {
	 return S_OK;
 }

 int  pthread_spin_unlock (pthread_spinlock_t * lock)
 {
	 return S_OK;
 }

/*
 * Barrier Functions
 */
 int  pthread_barrier_init (pthread_barrier_t * barrier, const pthread_barrierattr_t * attr, unsigned int count)
 {
	 return S_OK;
 }

 int  pthread_barrier_destroy (pthread_barrier_t * barrier)
 {
	 return S_OK;
 }

 int  pthread_barrier_wait (pthread_barrier_t * barrier)
 {
	 return S_OK;
 }

/*
 * Scheduling
 */
 int  pthread_setschedparam (pthread_t thread, int policy, const struct sched_param *param)
 {
	 return S_OK;
 }

 int  pthread_getschedparam (pthread_t thread, int *policy,struct sched_param *param)
 {
	 return S_OK;
 }

 int  pthread_setconcurrency (int n)
 {
	 return S_OK;
 }
 
 int  pthread_getconcurrency ()
 {
	 return S_OK;
 }

 /*
 * Read-Write Lock Functions
 */
 int  pthread_rwlock_init(pthread_rwlock_t *lock, const pthread_rwlockattr_t *attr)
 {
	 return S_OK;
 }

 int  pthread_rwlock_destroy(pthread_rwlock_t *lock)
 {
	 return S_OK;
 }

 int  pthread_rwlock_tryrdlock(pthread_rwlock_t *lock)
 {
	 return S_OK;
 }

 int  pthread_rwlock_trywrlock(pthread_rwlock_t *lock)
 {
	 return S_OK;
 }

 int  pthread_rwlock_rdlock(pthread_rwlock_t *lock)
 {
	 return S_OK;
 }

 /**
  * gaojie
  *
 int  pthread_rwlock_timedrdlock(pthread_rwlock_t *lock, const struct timespec *abstime)
 {
	 return S_OK;
 }
  */

 int  pthread_rwlock_wrlock(pthread_rwlock_t *lock)
 {
	 return S_OK;
 }
/**
 * gaojie
 *
 int  pthread_rwlock_timedwrlock(pthread_rwlock_t *lock,const struct timespec *abstime)
 {
	 return S_OK;
 }
 */

 int  pthread_rwlock_unlock(pthread_rwlock_t *lock)
 {
	 return S_OK;
 }

 int  pthread_rwlockattr_init (pthread_rwlockattr_t * attr)
 {
	 return S_OK;
 }

 int  pthread_rwlockattr_destroy (pthread_rwlockattr_t * attr)
 {
	 return S_OK;
 }

 int  pthread_rwlockattr_getpshared (const pthread_rwlockattr_t * attr, int *pshared)
 {
	 return S_OK;
 }

 int  pthread_rwlockattr_setpshared (pthread_rwlockattr_t * attr, int pshared)
 {
	 return S_OK;
 }

/*
 * Compatibility with Linux.
 */
 int  pthread_mutexattr_setkind_np(pthread_mutexattr_t * attr, int kind)
 {
	 return S_OK;
 }

 int  pthread_mutexattr_getkind_np(pthread_mutexattr_t * attr, int *kind)
 {
	 return S_OK;
 }

/*
 * Possibly supported by other POSIX threads implementations
 */
 /**
  * gaojie
  *
 int  pthread_delay_np (struct timespec * interval)
 {
	 return S_OK;
 }
  */

 int  pthread_num_processors_np(void)
 {
	 return S_OK;
 }
