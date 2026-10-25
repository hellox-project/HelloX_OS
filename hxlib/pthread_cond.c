//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Oct 13, 2026
//    Module Name               : pthread_cond.c
//    Module Funciton           : 
//                                Simulates POSIX pthread operations, which
//                                are widely refered by applications. 
//                                Condition variable routines are implemented
//                                in this file. Please be noted that some of
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
 * Condition Variable Functions.
 * Init a condition.
 */
int  pthread_cond_init(pthread_cond_t * cond, const pthread_condattr_t * attr)
{
	if (NULL == cond)
	{
		return EINVAL;
	}

	/* Create condition object. */
	*cond = (pthread_cond_t)_hx_malloc(sizeof(struct pthread_cond_t_));
	if (NULL == *cond)
	{
		return ENOMEM;
	}

	/* For debugging. */
	(*cond)->clock_id = 2048;

	(*cond)->cond = CreateCondition();
	if (NULL == (*cond)->cond)
	{
		_hx_free(*cond);
		*cond = NULL;
		return ENOMEM;
	}

	return 0;
}

/* Destroy condition object. */
int  pthread_cond_destroy(pthread_cond_t * cond)
{
	if ((NULL == cond) || (NULL == *cond))
	{
		printf("[%s]with *cond == NULL\r\n", __func__);
		return EINVAL;
	}

	if (NULL == (*cond)->cond)
	{
		printf("[%s]with (*cond)->cond == NULL\r\n", __func__);
		return EINVAL;
	}

	DestroyCondition((HANDLE)((*cond)->cond));
	_hx_free(*cond);
	*cond = NULL;

	return 0;
}

/* Wait a condition. */
int  pthread_cond_wait(pthread_cond_t * cond, pthread_mutex_t * mutex)
{
	HANDLE hCondition = INVALID_HANDLE_VALUE;
	HANDLE hMutex = INVALID_HANDLE_VALUE;
	unsigned long wait_result = 0;

	if ((NULL == cond) || (NULL == mutex) ||
		(NULL == *cond) || (NULL == *mutex))
	{
		return EINVAL;
	}
	if ((NULL == (*cond)->cond) || (NULL == (*mutex)->event))
	{
		return EINVAL;
	}

	hCondition = (HANDLE)(*cond)->cond;
	hMutex = (HANDLE)(*mutex)->event;

	wait_result = ConditionWait(hCondition, hMutex);
	switch (wait_result)
	{
	case OBJECT_WAIT_RESOURCE:
		return 0;

	case OBJECT_WAIT_DELETED:
	case OBJECT_WAIT_FAILED:
		/* Object is released, or internal error(fail). */
		return EINVAL;
	}
	return EINVAL;
}

/*
 * Local helper to calculate the million seconds
 * from now to abstime, since HelloX kernel's implementation
 * of condition uses related time in million seconds.
 */
static uint64_t remaining_ms(const struct timespec *abstime)
{
	struct timespec now;
	clock_gettime(CLOCK_REALTIME, &now); /* Must same source as abstime. */

	int64_t sec = (int64_t)abstime->tv_sec - (int64_t)now.tv_sec;
	int64_t nsec = (int64_t)abstime->tv_nsec - (int64_t)now.tv_nsec;

	if (nsec < 0) {
		sec -= 1;
		nsec += 1000000000LL;
	}

	if (sec < 0) {
		return 0; /* Time out. */
	}

	return (uint64_t)sec * 1000ULL + (uint64_t)nsec / 1000000ULL;
}

/* Wait condition in time out maner. */
int  pthread_cond_timedwait(pthread_cond_t * cond, pthread_mutex_t * mutex,
	const struct timespec *abstime)
{
	HANDLE hCondition = INVALID_HANDLE_VALUE;
	HANDLE hMutex = INVALID_HANDLE_VALUE;
	unsigned long wait_time = 0;
	unsigned long wait_result = 0;

	if ((NULL == cond) || (NULL == mutex) || (NULL == abstime) ||
		(NULL == *cond) || (NULL == *mutex))
	{
		return -EINVAL;
	}
	if ((NULL == (*cond)->cond) || (NULL == (*mutex)->event))
	{
		return -EINVAL;
	}

	/* Get the corresponding kernel object's handles. */
	hCondition = (HANDLE)(*cond)->cond;
	hMutex = (HANDLE)(*mutex)->event;

	/* Convert to related time in million seconds. */
	wait_time = (unsigned long)remaining_ms(abstime);
	if (wait_time <= 0)
	{
		/*
		 * TODO:
		 *   Even time out when this routine is invoked,
		 *   the mutex object must be released and obtained
		 *   again before return, according POSIX. Now for
		 *   simplicity we just return time out, and should
		 *   revise this issue later.
		 */
		return ETIMEDOUT;
	}
	wait_result = ConditionTimeoutWait(hCondition, hMutex, wait_time);
	switch (wait_result)
	{
	case OBJECT_WAIT_TIMEOUT:
		return ETIMEDOUT;
	case OBJECT_WAIT_RESOURCE:
		return 0;
	case OBJECT_WAIT_DELETED:
		/*
		 * Condition is released, undefined result according
		 * POSIX, just return EINVAL.
		 */
		return -EINVAL;
	case OBJECT_WAIT_FAILED:
		return -EINVAL;
	}

	/* Shoud not reach here. */
	return -EINVAL;
}

/* Signal a condition. */
int  pthread_cond_signal(pthread_cond_t * cond)
{
	HANDLE hCondition = INVALID_HANDLE_VALUE;

	if ((NULL == cond) || (NULL == *cond))
	{
		return EINVAL;
	}
	if (NULL == (*cond)->cond)
	{
		return EINVAL;
	}

	/* Invoke kernel's API. */
	hCondition = (HANDLE)((*cond)->cond);
	ConditionSignal(hCondition);

	return 0;
}

/* Broadcast condition object. */
int  pthread_cond_broadcast(pthread_cond_t * cond)
{
	HANDLE hCondition = INVALID_HANDLE_VALUE;

	if ((NULL == cond) || (NULL == *cond))
	{
		return EINVAL;
	}
	if (NULL == (*cond)->cond)
	{
		return EINVAL;
	}

	/* Invoke kernel's API. */
	hCondition = (HANDLE)((*cond)->cond);
	ConditionBroadcast(hCondition);

	return 0;
}

/*
 * Condition Variable Attribute Functions.
 * Just keep space without implementation, may
 * implement them later according requirement.
 */
int  pthread_condattr_init(pthread_condattr_t * attr)
{
	return 0;
}

int  pthread_condattr_destroy(pthread_condattr_t * attr)
{
	return 0;
}

int  pthread_condattr_getpshared(const pthread_condattr_t * attr, int *pshared)
{
	return 0;
}

int  pthread_condattr_setpshared(pthread_condattr_t * attr, int pshared)
{
	return 0;
}
