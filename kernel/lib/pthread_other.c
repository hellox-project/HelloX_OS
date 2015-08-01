
#include <StdAfx.h>
#include <types.h>
#include <kapi.h>
#include <pthread.h>
#include <time.h>

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
 * Condition Variable Attribute Functions
 */
 int  pthread_condattr_init (pthread_condattr_t * attr)
 {
	 return S_OK;
 }

 int  pthread_condattr_destroy (pthread_condattr_t * attr)
 {
	 return S_OK;
 }

 int  pthread_condattr_getpshared (const pthread_condattr_t * attr,int *pshared)
 {
	 return S_OK;
 }

 int  pthread_condattr_setpshared (pthread_condattr_t * attr, int pshared)
 {
	 return S_OK;
 }

/*
 * Condition Variable Functions
 */
 int  pthread_cond_init (pthread_cond_t * cond, const pthread_condattr_t * attr)
 {
	 __CONDITION* pCondition = NULL;

	 if(NULL == cond)
	 {
		 return EINVAL;
	 }
	 *cond = (pthread_cond_t)KMemAlloc(sizeof(pthread_cond_t),KMEM_SIZE_TYPE_ANY);
	 if(NULL == *cond)
	 {
		 return EINVAL;
	 }

	 (*cond)->cond = CreateCondition(0);
	 if(NULL == (*cond)->cond)
	 {
		 KMemFree(*cond,KMEM_SIZE_TYPE_ANY,0);
		 return EINVAL;
	 }
	 return S_OK;
 }

 int  pthread_cond_destroy (pthread_cond_t * cond)
 {
	 __CONDITION* pCond = NULL;
	 
	 if(NULL == cond)
	 {
		 return EINVAL;
	 }
	 if(NULL == (*cond)->cond)
	 {
		 return EINVAL;
	 }
	 DestroyCondition((HANDLE)((*cond)->cond));
	 KMemFree((*cond),KMEM_SIZE_TYPE_ANY,0);
	 return S_OK;
 }

 int  pthread_cond_wait (pthread_cond_t * cond, pthread_mutex_t * mutex)
 {
	 __CONDITION*    pCond   = NULL;
	 __MUTEX*        pMutex  = NULL;

	 if((NULL == cond) || (NULL == mutex))
	 {
		 return EINVAL;
	 }
	 if((NULL == (*cond)->cond) || (NULL == (*mutex)->event))
	 {
		 return EINVAL;
	 }
	 pCond  = (__CONDITION*)(*cond)->cond;
	 pMutex = (__MUTEX*)(*mutex)->event;

	 if(OBJECT_WAIT_RESOURCE == pCond->CondWait((__COMMON_OBJECT*)pCond,(__COMMON_OBJECT*)pMutex))
	 {
		 return S_OK;
	 }
	 return EINVAL;
 }

 int  pthread_cond_timedwait (pthread_cond_t * cond, pthread_mutex_t * mutex, const struct timespec *abstime)
 {
	 __CONDITION*    pCond   = NULL;
	 __MUTEX*        pMutex  = NULL;
	 DWORD           waitTime = 0;

	 if((NULL == cond) || (NULL == mutex) || (NULL == abstime))
	 {
		 return EINVAL;
	 }
	 if((NULL == (*cond)->cond) || (NULL == (*mutex)->event))
	 {
		 return EINVAL;
	 }
	 pCond  = (__CONDITION*)(*cond)->cond;
	 pMutex = (__MUTEX*)(*mutex)->event;

	 //Should revise later.
	 waitTime  = abstime->tv_sec * 1000;
	 waitTime += abstime->tv_nsec / 1000;

	 if(OBJECT_WAIT_RESOURCE == pCond->CondWaitTimeout((__COMMON_OBJECT*)pCond,(__COMMON_OBJECT*)pMutex,waitTime))
	 {
		 return S_OK;
	 }
	 return EINVAL;
 }

 int  pthread_cond_signal (pthread_cond_t * cond)
 {
	 __CONDITION* pCond = NULL;

	 if(NULL == cond)
	 {
		 return EINVAL;
	 }
	 if(NULL == (*cond)->cond)
	 {
		 return EINVAL;
	 }
	 pCond->CondSignal((__COMMON_OBJECT*)pCond);
	 return S_OK;
 }

 int  pthread_cond_broadcast (pthread_cond_t * cond)
 {
	 __CONDITION* pCond = NULL;

	 if(NULL == cond)
	 {
		 return EINVAL;
	 }
	 if(NULL == (*cond)->cond)
	 {
		 return EINVAL;
	 }
	 pCond->CondBroadcast((__COMMON_OBJECT*)pCond);
	 return S_OK;
 }

/*
 * Scheduling
 */
 /**
  *gaojie
 int  pthread_setschedparam (pthread_t thread, int policy, const struct sched_param *param)
 {
	 return S_OK;
 }
  */
/**
 * gaojie
 *
 int  pthread_getschedparam (pthread_t thread, int *policy,struct sched_param *param)
 {
	 return S_OK;
 }
 */

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
