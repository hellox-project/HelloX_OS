//***********************************************************************/
//    Author                    : Garry and Tywind
//    Original Date             : April 17,2015
//    Module Name               : pthread_mutex.c
//    Module Funciton           : 
//                                In order to port JamVM into HelloX,we must
//                                simulate POSIX pthread operations,which
//                                are widely refered by JamVM.
//                                This file contains the source code of pthread
//                                simulation mechanism.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/


#include <hellox.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <errno.h>
#include <pthread.h>

int ptw32_mutex_check_need_init(pthread_mutex_t* mutex)
{
	if (NULL == mutex)
	{
		return EINVAL;
	}
	return S_OK;
}

LONG ptw32_InterlockedExchange(LPLONG location, LONG value)
{
	LONG result = 0;

#if defined(_M_IX86) || defined(__i386__)
#if (defined(__clang__) || (defined(__GNUC__)))
    __asm__ __volatile__(
        "xchgl %%eax, (%%ecx)\n\t"  /* XCHG dword ptr[ecx], eax */
        : "=a" (result)             /* 输出：eax 中的值存入 result */
        : "c" (location),           /* 输入：location 放入 ecx */
          "a" (value)               /* 输入：value 放入 eax */
        : "memory"                  /* 告诉编译器内存被修改 */
    );
#elif defined(_MSC_VER)
	__asm {
		PUSH         ecx
		MOV          ecx, dword ptr[location]
		MOV          eax, dword ptr[value]
		XCHG         dword ptr[ecx], eax
		MOV          dword ptr[result], eax
		POP          ecx
	}
#endif
#endif

	return result;
}

PTW32_LONG  ptw32_InterlockedCompareExchange(PTW32_LPLONG location, PTW32_LONG value, PTW32_LONG comparand)
{
	PTW32_LONG result;

#if defined(_M_IX86) || defined(__i386__)
#if (defined(__clang__) || (defined(__GNUC__)))
    __asm__ __volatile__(
        "lock cmpxchgl %%edx, (%%ecx)\n\t"  /* LOCK CMPXCHG dword ptr[ecx], edx */
        : "=a" (result)                     /* 输出：eax 中的旧值存入 result */
        : "c" (location),                   /* 输入：location 放入 ecx */
          "d" (value),                      /* 输入：value 放入 edx */
          "a" (comparand)                   /* 输入：comparand 放入 eax */
        : "memory", "cc"                    /* 内存被修改，条件标志被修改 */
    );
#elif defined(_MSC_VER)
	__asm {
		PUSH         ecx
		PUSH         edx
		MOV          ecx, dword ptr[location]
		MOV          edx, dword ptr[value]
		MOV          eax, dword ptr[comparand]
		LOCK CMPXCHG dword ptr[ecx], edx
		MOV          dword ptr[result], eax
		POP          edx
		POP          ecx
	}
#endif
#endif

	return result;
}

/*
 * Create mutex object and return it through
 * @mutex.
 */
int  pthread_mutex_init(pthread_mutex_t* mutex, const pthread_mutexattr_t* attr)
{
	HANDLE hMutex = NULL;
	
	hMutex = CreateMutex();
	if (NULL == hMutex)
	{
		return -S_FALSE;
	}

	*mutex = (pthread_mutex_t)malloc(sizeof(struct pthread_mutex_t_));
	if (NULL == *mutex)
	{
		DestroyMutex(hMutex);
		return -ENOMEM;
	}

	(*mutex)->lock_idx = 0;
	(*mutex)->recursive_count = 0;
	(*mutex)->kind = PTHREAD_MUTEX_DEFAULT;
	(*mutex)->ownerThread.p = GetCurrentThread();
	(*mutex)->event = hMutex;

	return 0;
}

int  pthread_mutex_destroy(pthread_mutex_t* mutex)
{
	HANDLE hMutex = NULL;

	if (NULL == mutex)
	{
		return -EINVAL;
	}

	if (*mutex == NULL)
	{
		return -EINVAL;
	}

	hMutex = (HANDLE)(*mutex)->event;
	DestroyMutex(hMutex);
	free(*mutex);

	return 0;
}

int  pthread_mutex_lock(pthread_mutex_t* mutex)
{
	HANDLE hMutex = NULL;

	if (*mutex == NULL)
	{
		return -EINVAL;
	}

	if ((*mutex)->event == NULL)
	{
		return -EINVAL;
	}
	hMutex = (HANDLE)(*mutex)->event;

	if (OBJECT_WAIT_RESOURCE == WaitForThisObject(hMutex))
	{
		return 0;
	}

	return -S_FALSE;
}


int  pthread_mutex_unlock(pthread_mutex_t* mutex)
{
	HANDLE hMutex = NULL;

	if (NULL == mutex)
	{
		return -EINVAL;
	}
	if (NULL == (*mutex)->event)
	{
		return -EINVAL;
	}
	hMutex = (HANDLE)(*mutex)->event;
	ReleaseMutex(hMutex);
	return 0;
}

/****
 * gaojie
 *
int  pthread_mutex_timedlock(pthread_mutex_t *mutex, const struct timespec *abstime)
{
	DWORD waitTime = 0;
	__MUTEX* pMutex = NULL;

	if((NULL == mutex) || (NULL == abstime))
	{
		return EINVAL;
	}

	if(NULL == (*mutex)->event)
	{
		return EINVAL;
	}

	//Should revise later.
	pMutex = (__MUTEX*)(*mutex)->event;
	waitTime  = abstime->tv_sec * 1000;
	waitTime += abstime->tv_nsec / 1000;

	if(OBJECT_WAIT_RESOURCE == pMutex->WaitForThisObjectEx((__COMMON_OBJECT*)pMutex,waitTime))
	{
		return S_OK;
	}
	return EINVAL;
}
*/

/*
 * Try to acquire mutex, invokes WaitForThisObjectEx
 * routine of HelloX kernel using time out value as 0.
 * Please be noted that the mutex support recursive
 * acquiring, i.e, if the current owner and the caller
 * is same thread, then it will return RESOURCE.
 */
int  pthread_mutex_trylock(pthread_mutex_t * mutex)
{
	HANDLE hMutex = NULL;
	unsigned long wait_result = 0;

	if (NULL == mutex)
	{
		return -EINVAL;
	}
	if (NULL == (*mutex)->event)
	{
		return -EINVAL;
	}

	hMutex = (HANDLE)(*mutex)->event;
	wait_result = WaitForThisObjectEx(hMutex, 0);	 
	switch (wait_result)
	{
	case OBJECT_WAIT_TIMEOUT:
		/* Object is busy, return EBUSY. */
		return EBUSY;
	case OBJECT_WAIT_RESOURCE:
		return 0;
	case OBJECT_WAIT_DELETED:
		/*
		 * Mutex is released, undefined result according
		 * POSIX, just return EINVAL.
		 */
		return -EINVAL;
	case OBJECT_WAIT_FAILED:
		return -EINVAL;
	}

	return -EINVAL;
}

/*
* Mutex Attribute Functions
*/
int  pthread_mutexattr_init(pthread_mutexattr_t * attr)
{
	return S_OK;
}

int  pthread_mutexattr_destroy(pthread_mutexattr_t * attr)
{
	return S_OK;
}

int  pthread_mutexattr_getpshared(const pthread_mutexattr_t *attr, int *pshared)
{
	return S_OK;
}

int  pthread_mutexattr_setpshared(pthread_mutexattr_t * attr, int pshared)
{
	return S_OK;
}

int  pthread_mutexattr_settype(pthread_mutexattr_t * attr, int kind)
{
	return S_OK;
}

int  pthread_mutexattr_gettype(pthread_mutexattr_t * attr, int *kind)
{
	return S_OK;
}
