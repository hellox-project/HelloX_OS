//***********************************************************************/
//    Author                    : Garry and Tywind
//    Original Date             : April 17,2015
//    Module Name               : pthread.c
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
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

static int  s_nThreadIndex = 1;

/*
 * TLS used to save current pthread's local
 * information, such as the HANDLE of the
 * thread, etc.
 */
static unsigned long thread_info_tls = TLS_OUT_OF_INDEXES;

/* 
 * Local struct used to xfer parameters 
 * to thread wrapper. 
 */
typedef struct tag__PTHREAD_INFO {
	void *(*start)(void*);
	void* thread_arg;
	unsigned long thread_flags;
	HANDLE thread_handle;
}__PTHREAD_INFO;

/*
 * Wrapper routine for each thread that
 * created using pthread library.
 */
static unsigned long __CDECL __pthread_thread_wrapper(void* arg)
{
	__PTHREAD_INFO* thread_info = (__PTHREAD_INFO*)arg;
	
	if ((NULL == thread_info) || (NULL == thread_info->start) 
		|| (NULL == thread_info->thread_handle))
	{
		return EINVAL;
	}

	/* Allocate TLS to save current thread info. */
	if (thread_info_tls >= TLS_OUT_OF_INDEXES)
	{
		thread_info_tls = TlsAlloc();
	}
	if (thread_info_tls >= TLS_OUT_OF_INDEXES)
	{
		return ENOTLS;
	}

	/* Save current thread info. */
	TlsSetValue(thread_info_tls, thread_info);

	/* Invoke the user thread routine. */
	thread_info->start(thread_info->thread_arg);

	/* Release the thread info structure. */
	TlsSetValue(thread_info_tls, NULL);
	_hx_free(thread_info);

	/* Exit the thread. */
	ExitThread(0);
	return 0;
}

/* Create a new thread. */
int pthread_create(pthread_t * tid, const pthread_attr_t * attr, 
	void *(*start) (void *), void *arg)
{
	HANDLE thread_handle = NULL;
	char szThreadName[MAX_THREAD_NAME_LEN + 1];
	size_t stackSize = 0;
	__PTHREAD_INFO* thread_info = NULL;
	unsigned long thread_priority = PRIORITY_LEVEL_NORMAL;

	if ((tid == NULL) || (NULL == start))
	{
		return -EINVAL;
	}

	if (attr)
	{
		/* Use specified stack size. */
		stackSize = (*attr)->stack_size;
		
		/* Use user specified priority leve. */
		thread_priority = (*attr)->schedparam.__sched_priority;
		if (thread_priority > HIGHEST_PRIORITY_LEVEL)
		{
			return -EINVAL;
		}
	}

	if (attr && (*attr)->thread_name)
	{
		/* Use user specified name length. */
		strncpy(szThreadName, (*attr)->thread_name, MAX_THREAD_NAME_LEN);
	}
	else {
		/* Use default thread name. */
		_hx_sprintf(szThreadName, "pthread_%d", s_nThreadIndex++);
	}

	/* Use thread info to xfer parameters. */
	thread_info = (__PTHREAD_INFO*)_hx_malloc(sizeof(__PTHREAD_INFO));
	if (NULL == thread_info)
	{
		return -ENOMEM;
	}
	memset(thread_info, 0, sizeof(__PTHREAD_INFO));
	thread_info->start = start;
	thread_info->thread_arg = arg;

	/* 
	 * Create the user thread, put
	 * it into suspended state at the
	 * begining, then resume it after
	 * creation. This eliminates the
	 * race condition between the user
	 * thread and the current thread.
	 */
	thread_handle = CreateUserThread(
		//THREAD_STATUS_READY,
		THREAD_STATUS_SUSPENDED,
		thread_priority,
		__pthread_thread_wrapper,
		(void*)thread_info,
		szThreadName);

	if (NULL == thread_handle)
	{
		return -EINVAL;
	}

	/* Save thread handle. */
	thread_info->thread_handle = thread_handle;

	/* Resume the thread. */
	ResumeThread(thread_handle);

	*tid = (HANDLE)thread_handle;
	return S_OK;
}

/* Kill a thread. */
int  pthread_kill(pthread_t thread, int sig)
{
	return S_OK;
}

int pthread_detach(pthread_t tid)
{
	/*
	 * Just issue destroy command, decrease
	 * the reference counter.
	 */
	DestroyUserThread((HANDLE)tid);

	return S_OK;
}

int  pthread_equal(pthread_t t1, pthread_t t2)
{
	return (t1 == t2);
}

void  pthread_exit(void *value_ptr)
{
	TerminateKernelThread(
		GetCurrentThread(),
		-1);
}

int pthread_join(pthread_t thread, void **value_ptr)
{
	unsigned long wait_result = 0;
	
	if (NULL == thread)
	{
		return EINVAL;
	}
	wait_result = WaitForThisObject(thread);
	if (value_ptr)
	{
		*value_ptr = (void*)wait_result;
	}

	return S_OK;
}

pthread_t pthread_self(void)
{
	__PTHREAD_INFO* thread_info = NULL;

	if (thread_info_tls >= TLS_OUT_OF_INDEXES)
	{
		/* No TLS created. */
		return NULL;
	}

	thread_info = (__PTHREAD_INFO*)TlsGetValue(thread_info_tls);
	if (NULL == thread_info)
	{
		return NULL;
	}
	return thread_info->thread_handle;
}

int  pthread_cancel(pthread_t thread)
{
	return S_OK;
}

int  pthread_setcancelstate(int state, int *oldstate)
{
	return S_OK;
}

int  pthread_setcanceltype(int type, int *oldtype)
{
	return S_OK;
}

void  pthread_testcancel(void)
{
	return;
}

int  pthread_once(pthread_once_t * once_control, void(*init_routine) (void))
{
	return S_OK;
}

//************************************************************************
//** pthread_attr's implementation.
//************************************************************************

int  pthread_attr_init(pthread_attr_t * attr)
{
	if (NULL == attr)
	{
		return EINVAL;
	}
	*attr = (pthread_attr_t)_hx_malloc(sizeof(struct pthread_attr_t_));
	if (NULL == *attr)
	{
		return EINVAL;
	}
	(*attr)->detach_state = PTHREAD_CREATE_JOINABLE;
	(*attr)->schedparam.__sched_priority = PRIORITY_LEVEL_NORMAL;
	(*attr)->stack_ptr = NULL;
	(*attr)->stack_size = 0;  //Use default stack size.
	return 0;
}

int  pthread_attr_destroy(pthread_attr_t * attr)
{
	if (NULL == attr)
	{
		return EINVAL;
	}
	if (NULL == *attr)
	{
		return EINVAL;
	}
	_hx_free(*attr);
	return 0;
}

int  pthread_attr_getdetachstate(const pthread_attr_t * attr, int *detachstate)
{
	if ((NULL == attr) || (NULL == detachstate))
	{
		return EINVAL;
	}
	if (NULL == *attr)
	{
		return EINVAL;
	}
	*detachstate = (*attr)->detach_state;
	return 0;
}

int  pthread_attr_getstackaddr(const pthread_attr_t * attr, void **stackaddr)
{
	if ((NULL == attr) || (NULL == stackaddr))
	{
		return EINVAL;
	}
	if (NULL == *attr)
	{
		return EINVAL;
	}
	*stackaddr = (*attr)->stack_ptr;
	return 0;
}

int  pthread_attr_getstacksize(const pthread_attr_t * attr, size_t * stacksize)
{
	if ((NULL == attr) || (NULL == stacksize))
	{
		return EINVAL;
	}
	if (NULL == *attr)
	{
		return EINVAL;
	}
	*stacksize = (*attr)->stack_size;
	return 0;
}

int  pthread_attr_setdetachstate(pthread_attr_t * attr, int detachstate)
{
	if (NULL == attr)
	{
		return EINVAL;
	}
	if (NULL == *attr)
	{
		return EINVAL;
	}
	(*attr)->detach_state = detachstate;
	return 0;
}

int  pthread_attr_setstackaddr(pthread_attr_t * attr, void *stackaddr)
{
	if ((NULL == attr) || (NULL == stackaddr))
	{
		return EINVAL;
	}
	if (NULL == *attr)
	{
		return EINVAL;
	}
	(*attr)->stack_ptr = stackaddr;
	return 0;
}

int  pthread_attr_setstacksize(pthread_attr_t * attr, size_t stacksize)
{
	if ((NULL == attr) || (0 == stacksize))
	{
		return EINVAL;
	}
	if (NULL == *attr)
	{
		return EINVAL;
	}
	(*attr)->stack_size = stacksize;
	return 0;
}

int  pthread_attr_getschedparam(const pthread_attr_t *attr, struct sched_param *param)
{
	if ((NULL == attr) || (NULL == param))
	{
		return EINVAL;
	}

	/* Copy schedule param. */
	param->__sched_priority = (*attr)->schedparam.__sched_priority;

	return 0;
}

int  pthread_attr_setschedparam(pthread_attr_t *attr, const struct sched_param *param)
{
	if ((NULL == attr) || (NULL == param))
	{
		return EINVAL;
	}

	/* Copy schedule param. */
	(*attr)->schedparam.__sched_priority = param->__sched_priority;

	return 0;
}

int pthread_key_create(pthread_key_t * key, void(*destructor) (void *))
{
	unsigned long tls_key = TlsAlloc();
	if (tls_key >= TLS_OUT_OF_INDEXES)
	{
		*key = TLS_OUT_OF_INDEXES;
		return -S_FALSE;
	}
	*key = tls_key;
	return S_OK;
}

int pthread_key_delete(pthread_key_t key)
{
	if (key >= TLS_OUT_OF_INDEXES)
	{
		return -S_FALSE;
	}
	TlsFree(key);
	return S_OK;
}

int pthread_setspecific(pthread_key_t key, const void *value)
{
	if (TlsSetValue(key, (LPVOID)value))
	{
		return S_OK;
	}
	return -S_FALSE;
}

void* pthread_getspecific(pthread_key_t key)
{
	LPVOID value = TlsGetValue(key);
	return value;
}
