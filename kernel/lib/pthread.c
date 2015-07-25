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

#include <StdAfx.h>
#include <kapi.h>
#include <stdio.h>
#include <pthread.h>

static int  s_nThreadIndex = 1; 

//////////////////////////////////////////////////////////////////////////
int  pthread_create (pthread_t * tid,	const pthread_attr_t * attr,void *(*start) (void *),void *arg)
{		
	__KERNEL_THREAD_OBJECT*    lpKernelThread  = NULL;
	char                       szThreadName[64]    ; 
	size_t                     stackSize       = 0;

	if((tid == NULL) || (NULL == start))
	{
		return EINVAL;
	}

	if(attr)
	{
		//Use user specified stack size.
		stackSize = (*attr)->stack_size;
	}
	
	_hx_sprintf(szThreadName,"pthread_%d",s_nThreadIndex++);
	lpKernelThread = KernelThreadManager.CreateKernelThread(
		             (__COMMON_OBJECT*)&KernelThreadManager,
		             stackSize,
		             KERNEL_THREAD_STATUS_READY,
		             PRIORITY_LEVEL_NORMAL,
		             (__KERNEL_THREAD_ROUTINE)start,
		             arg,
		             NULL,
		             szThreadName);

	if(NULL == lpKernelThread)  
	{
		return EINVAL;
	}

	*tid = (HANDLE)lpKernelThread;
	return S_OK;
}

//Send a signal to the specified thread.
int  pthread_kill(pthread_t thread, int sig)
{
	return S_OK;
}

int  pthread_detach (pthread_t tid)
{
	return S_OK;
}

int  pthread_equal (pthread_t t1, pthread_t t2)
{
	return (t1 == t2);
}

void  pthread_exit (void *value_ptr)
{
	KernelThreadManager.TerminateKernelThread((__COMMON_OBJECT*)&KernelThreadManager,
		NULL,0);
}

int  pthread_join (pthread_t thread, void **value_ptr)
{
	__KERNEL_THREAD_OBJECT*  lpKernelThread = (__KERNEL_THREAD_OBJECT*)thread;
	*value_ptr = (void*)lpKernelThread->WaitForThisObject((__COMMON_OBJECT*)lpKernelThread);
	return S_OK;
}

pthread_t  pthread_self (void)
{
	return (HANDLE)KernelThreadManager.lpCurrentKernelThread;
}

int  pthread_cancel (pthread_t thread)
{
	return S_OK;
}

int  pthread_setcancelstate (int state, int *oldstate)
{
	return S_OK;
}

int  pthread_setcanceltype (int type, int *oldtype)
{
	return S_OK;
}

void  pthread_testcancel (void)
{
	return;
}

int  pthread_once (pthread_once_t * once_control,  void (*init_routine) (void))
{
	return S_OK;
}

//************************************************************************
//** pthread_attr's implementation.
//************************************************************************

int  pthread_attr_init (pthread_attr_t * attr)
{
	if(NULL == attr)
	{
		return EINVAL;
	}
	*attr = (pthread_attr_t)KMemAlloc(sizeof(struct pthread_attr_t_),KMEM_SIZE_TYPE_ANY);
	if(NULL == *attr)
	{
		return EINVAL;
	}
	(*attr)->detach_state = PTHREAD_CREATE_JOINABLE;
	(*attr)->priority     = PRIORITY_LEVEL_NORMAL;
	(*attr)->stack_ptr    = NULL;
	(*attr)->stack_size   = 0;  //Use default stack size.
	return S_OK;
}

int  pthread_attr_destroy (pthread_attr_t * attr)
{
	if(NULL == attr)
	{
		return EINVAL;
	}
	if(NULL == *attr)
	{
		return EINVAL;
	}
	KMemFree(*attr,KMEM_SIZE_TYPE_ANY,0);
	return S_OK;
}

int  pthread_attr_getdetachstate (const pthread_attr_t * attr,int *detachstate)
{
	if((NULL == attr) || (NULL == detachstate))
	{
		return EINVAL;
	}
	if(NULL == *attr)
	{
		return EINVAL;
	}
	*detachstate = (*attr)->detach_state;
	return S_OK;
}

int  pthread_attr_getstackaddr (const pthread_attr_t * attr,void **stackaddr)
{
	if((NULL == attr) || (NULL == stackaddr))
	{
		return EINVAL;
	}
	if(NULL == *attr)
	{
		return EINVAL;
	}
	*stackaddr = (*attr)->stack_ptr;
	return S_OK;
}

int  pthread_attr_getstacksize (const pthread_attr_t * attr, size_t * stacksize)
{
	if((NULL == attr) || (NULL == stacksize))
	{
		return EINVAL;
	}
	if(NULL == *attr)
	{
		return EINVAL;
	}
	*stacksize = (*attr)->stack_size;
	return S_OK;
}

int  pthread_attr_setdetachstate (pthread_attr_t * attr,int detachstate)
{
	if(NULL == attr)
	{
		return EINVAL;
	}
	if(NULL == *attr)
	{
		return EINVAL;
	}
	(*attr)->detach_state = detachstate;
	return S_OK;
}

int  pthread_attr_setstackaddr (pthread_attr_t * attr, void *stackaddr)
{
	if((NULL == attr) || (NULL == stackaddr))
	{
		return EINVAL;
	}
	if(NULL == *attr)
	{
		return EINVAL;
	}
	(*attr)->stack_ptr = stackaddr;
	return S_OK;
}

int  pthread_attr_setstacksize (pthread_attr_t * attr, size_t stacksize)
{
	if((NULL == attr) || (0 == stacksize))
	{
		return EINVAL;
	}
	if(NULL == *attr)
	{
		return EINVAL;
	}
	(*attr)->stack_size = stacksize;
	return S_OK;
}

int  pthread_key_create (pthread_key_t * key,void (*destructor) (void *))
{
	if(ProcessManager.GetTLSKey((__COMMON_OBJECT*)&ProcessManager,
		key,NULL))
	{
		return S_OK;
	}
	return EINVAL;
}

int  pthread_key_delete (pthread_key_t key)
{
	ProcessManager.ReleaseTLSKey((__COMMON_OBJECT*)&ProcessManager,key,NULL);
	return S_OK;
}

int  pthread_setspecific (pthread_key_t key,const void *value)
{
	if(ProcessManager.SetTLSValue((__COMMON_OBJECT*)&ProcessManager,key,(LPVOID)value))
	{
		return S_OK;
	}
	return EINVAL;
}

void*  pthread_getspecific (pthread_key_t key)
{
	LPVOID value = NULL;

	if(ProcessManager.GetTLSValue((__COMMON_OBJECT*)&ProcessManager,key,&value))
	{
		return value;
	}
	return NULL;
}
