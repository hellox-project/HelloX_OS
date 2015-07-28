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

#include <StdAfx.h>
#include <kapi.h>
#include <pthread.h>
#include <time.h>


int ptw32_mutex_check_need_init (pthread_mutex_t * mutex)
{
	return S_OK;
}

LONG ptw32_InterlockedExchange (LPLONG location,LONG value)
{
	LONG result = 0;

#ifdef __I386__

#ifdef __GCC__
	__asm__ __volatile__(
			".code32					;"
			"PUSHL 		%%ecx			;"
			"MOVL 		(%1),	%%ecx	;"
			"MOVL 		(%2),	%%eax	;"
			"XCHGL 		%%eax, 	(%%ecx)	;"
			"MOVL 		%%eax, 	(%0)	;"
			"POPL 		%%ecx			;"
			:"=r"(result)
			:"r"(location), "r"(value)
			: "memory"
	);
#else
	__asm {
		PUSH         ecx
		MOV          ecx,dword ptr [location]
		MOV          eax,dword ptr [value]
		XCHG         dword ptr [ecx],eax
		MOV          dword ptr [result], eax
		POP          ecx
	}
#endif

#endif

	return result;
}

PTW32_LONG  ptw32_InterlockedCompareExchange (PTW32_LPLONG location,	PTW32_LONG value,	PTW32_LONG comparand)
{
	PTW32_LONG result;


#ifdef __I386__

#ifdef __GCC__
	__asm__ __volatile__ (
			".code32						;"
			"PUSH %%ecx 					;"
			"PUSH %%edx 					;"
			"MOVL 	(%1),  		%%ecx		;"
			"MOVL 	(%2),		%%edx		;"
			"MOVL 	(%3),		%%eax		;"
			"LOCK CMPXCHGL	%%edx, 	(%%ecx)	;"
			"MOVL 			%%eax,	(%0)	;"
			"POP 			%%edx			;"
			"POP 			%%ecx			;"
			: "=r"(result)
			: "r"(location), "r"(value), "r"(comparand)
			: "memory"
			);
#else
	__asm {
		PUSH         ecx
		PUSH         edx
		MOV          ecx,dword ptr [location]
		MOV          edx,dword ptr [value]
		MOV          eax,dword ptr [comparand]
		LOCK CMPXCHG dword ptr [ecx],edx
		MOV          dword ptr [result], eax
		POP          edx
		POP          ecx
	}
#endif

#endif

	return result;
}

//Just create a mutex object and return it through @mutex.
int  pthread_mutex_init (pthread_mutex_t* mutex,const pthread_mutexattr_t * attr)
{
	__MUTEX* pMutex = NULL;

	pMutex = (__MUTEX*)ObjectManager.CreateObject(&ObjectManager,
		NULL,
		OBJECT_TYPE_MUTEX);
	if(NULL == pMutex)
	{
		return EINVAL;
	}

	if(!pMutex->Initialize((__COMMON_OBJECT*)pMutex))  //Can not initialize.
	{
		ObjectManager.DestroyObject(&ObjectManager,(__COMMON_OBJECT*)pMutex);
		return EINVAL;
	}

	*mutex  = (pthread_mutex_t)KMemAlloc(sizeof(struct pthread_mutex_t_),KMEM_SIZE_TYPE_ANY);
	if(NULL == *mutex)
	{
		ObjectManager.DestroyObject(&ObjectManager,(__COMMON_OBJECT*)pMutex);
		return EINVAL;
	}

	(*mutex)->lock_idx         = 0;
	(*mutex)->recursive_count  = 0;
	(*mutex)->kind             = PTHREAD_MUTEX_DEFAULT;
	(*mutex)->ownerThread.p    = KernelThreadManager.lpCurrentKernelThread;
	(*mutex)->event            = pMutex;

	return S_OK;
}

int  pthread_mutex_destroy (pthread_mutex_t* mutex)
{
	__MUTEX*    pMutex = NULL;

	if(mutex == NULL)
	{
		return -1;
	}
	if(*mutex == NULL)
	{
		return -1;
	}

	pMutex = (__MUTEX*)(*mutex)->event;
	ObjectManager.DestroyObject(&ObjectManager,	(__COMMON_OBJECT*)pMutex);

	//Release the memory of *mutex.
	KMemFree(*mutex,KMEM_SIZE_TYPE_ANY,0);

	return S_OK;
}

int  pthread_mutex_lock (pthread_mutex_t* mutex)
{
	__MUTEX* pMutex  = NULL;

	if (*mutex == NULL)
	{
		return EINVAL;
	}

	if((*mutex)->event == NULL)
	{
		return EINVAL;
	}
	pMutex = (__MUTEX*)(*mutex)->event;

	if(OBJECT_WAIT_RESOURCE == pMutex->WaitForThisObject((__COMMON_OBJECT*)pMutex))
	{
		return S_OK;
	}

	return EINVAL;
}


int  pthread_mutex_unlock (pthread_mutex_t * mutex)
{
	__MUTEX* pMutex = NULL;

	if(NULL == mutex)
	{
		return EINVAL;
	}
	if(NULL == (*mutex)->event)
	{
		return EINVAL;
	}
	pMutex = (__MUTEX*)(*mutex)->event;
	pMutex->ReleaseMutex((__COMMON_OBJECT*)pMutex);
	return S_OK;
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

int  pthread_mutex_trylock (pthread_mutex_t * mutex)
{
	__MUTEX* pMutex = NULL;

	if(NULL == mutex)
	{
		return EINVAL;
	}
	if(NULL == (*mutex)->event)
	{
		return EINVAL;
	}
	pMutex = (__MUTEX*)(*mutex)->event;
	if(OBJECT_WAIT_RESOURCE == pMutex->WaitForThisObjectEx((__COMMON_OBJECT*)pMutex,0))
	{
		return S_OK;
	}
	return EINVAL;
}

/*
* Mutex Attribute Functions
*/
int  pthread_mutexattr_init (pthread_mutexattr_t * attr)
{
	return S_OK;
}

int  pthread_mutexattr_destroy (pthread_mutexattr_t * attr)
{
	return S_OK;
}

int  pthread_mutexattr_getpshared (const pthread_mutexattr_t *attr,int *pshared)
{
	return S_OK;
}

int  pthread_mutexattr_setpshared (pthread_mutexattr_t * attr,int pshared)
{
	return S_OK;
}

int  pthread_mutexattr_settype (pthread_mutexattr_t * attr, int kind)
{
	return S_OK;
}

int  pthread_mutexattr_gettype (pthread_mutexattr_t * attr, int *kind)
{
	return S_OK;
}
