//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Sep,18 2006
//    Module Name               : ARCH_X86.CPP
//    Module Funciton           : 
//                                This module countains CPU specific code,in this file,
//                                Intel X86 series CPU's specific code is included.
//
//    Last modified Author      : Garry
//    Last modified Date        : 29 JAN,2009
//    Last modified Content     :
//                                1. __inb,__inw,__ind,__inbs,__inws and __inds are added.
//                                2. __outb,__outw,__outd are added.
//
//    Last modified Author      : Tywind
//    Last modified Date        : 30 JAN,2015
//    Last modified Content     :
//                                __GetTime routine is added,used to read system date and
//                                time from CMOS.
//    Lines number              :
//***********************************************************************/

#ifndef __STDAFX_H__
#include "StdAfx.h"
#endif

#ifndef __ARCH_H__
#include "arch.h"
#endif

#ifndef __UTSNAME_H__
#include <sys/utsname.h>
#endif

#ifdef __I386__  //Only available in x86 based PC platform.

//Architecture related initialization code,this routine will be called in the
//begining of system initialization.
//This routine must be in GLOBAL scope since it will be called by other routines.
BOOL HardwareInitialize()
{
	//x86 related hardware should be initialized here.Return FALSE if there is
	//error that can lead fault of system.
	return TRUE;
}

//
//This routine switches the current executing path to the new one identified
//by lpContext.
//
//__declspec(naked)
VOID __SwitchTo(__KERNEL_THREAD_CONTEXT* lpContext)
{
#ifdef _POSIX_
	__asm__ (
	".code32						\n\t "
	"pushl 	%%ebp					\n\t"
	"movl	%%esp,	%%ebp			\n\t"
	"movl	0x08(%%ebp),	%%esp	\n\t"
	"popl	%%ebp	\n\t"
	"popl	%%edi	\n\t"
	"popl	%%esi	\n\t"
	"popl	%%edx	\n\t"
	"popl	%%ecx	\n\t"
	"popl	%%ebx	\n\t"
	"movb	$0x20,	%%al	\n\t"
	"outb	%%al,	$0x20	\n\t"
	"outb	%%al,	$0xa0	\n\t"
	"popl	%%eax			\n\t"
	"iret					\n\t"
	:	:
	);
#else
	__asm{
		push ebp
		mov ebp,esp
		mov esp,dword ptr [ebp + 0x08]  //Restore ESP.
		pop ebp
		pop edi
		pop esi
		pop edx
		pop ecx
		pop ebx

		mov al,0x20  //Dismiss interrupt controller.
		out 0x20,al
		out 0xa0,al

		pop eax
		iretd
	}
#endif
}

//
//These three global variables are used as temp variables
//by __SaveAndSwitch routine.
//
static DWORD dwTmpEip = 0;
static DWORD dwTmpEax = 0;
static DWORD dwTmpEbp = 0;

//
//This routine saves current kernel thread's context,and switch
//to the new kernel thread.
//
//__declspec(naked)
VOID __SaveAndSwitch(__KERNEL_THREAD_CONTEXT** lppOldContext,
									   __KERNEL_THREAD_CONTEXT** lppNewContext)
{

#ifdef _POSIX_
	__asm__(
	".code32								\n\t"
	"movl	%%esp,	%0                  	\n\t"
	"popl	%1		                      	\n\t"
	"movl	%%eax,	%2   				\n\t"
	"pushf             					\n\t"
	"xorl	%%eax,	%%eax     				\n\t"
	"movw	%%cs,	%%ax          				\n\t"
	"pushl	%%eax           				\n\t"
	"pushl	%3      				\n\t"
	"pushl %4      				\n\t"
	"pushl %%ebx                          	\n\t"
	"pushl %%ecx                          	\n\t"
	"pushl %%edx                          	\n\t"
	"pushl %%esi                          	\n\t"
	"pushl %%edi                          	\n\t"
	"pushl %%ebp                          	\n\t"
	"             	                     	\n\t"
	"movl %%ebp,%5                  		\n\t"
	"movl 0x04(%%ebp),	%%ebx				\n\t"
	"movl %%esp,	(%%ebx)					\n\t"
	"                                     \n\t"
	"movl 0x08(%%ebp),	%%ebx				\n\t"
	"movl (%%ebx),		%%esp   			  \n\t"
	"popl %%ebp                              \n\t"
	"popl %%edi                              \n\t"
	"popl %%esi                              \n\t"
	"popl %%edx                            \n\t"
	"popl %%ecx                 			\n\t"
	"popl %%ebx         					\n\t"
    "popl %%eax							\n\t"
    "iret"
	:"=m"(dwTmpEbp),"=m"(dwTmpEip),"=m"(dwTmpEax)
	:"m"(dwTmpEip),"m"(dwTmpEax),"m"(dwTmpEbp)
	);
#else
	__asm{
		mov dwTmpEbp,esp
		pop dwTmpEip
		mov dwTmpEax,eax //Save EAX.
		pushfd           //Save EFlags.
		xor eax,eax
		mov ax,cs
		push eax         //Save CS.
		push dwTmpEip    //Save EIP.
		push dwTmpEax    //Save EAX.
		push ebx
		push ecx
		push edx
		push esi
		push edi
		push ebp

		//Now,we have build the target stack frame,then save it.
		mov ebp,dwTmpEbp
		mov ebx,dword ptr [ebp + 0x04]
		mov dword ptr [ebx],esp  //Save old stack frame.

		//Restore the new thread's context and switch to it.
		mov ebx,dword ptr [ebp + 0x08]
		mov esp,dword ptr [ebx]  //Restore new stack.
		pop ebp
		pop edi
		pop esi
		pop edx
		pop ecx
		pop ebx
		pop eax
		iretd
	}
#endif
}

//
//Enable Virtual Memory Management mechanism.This routine will be called in
//process of OS initialization if __CFG_SYS_VMM flag is defined.
//
VOID EnableVMM()
{
#ifdef _POSIX_
	__asm__ (
	".code32			\n\t"
	"pushl	%%eax		\n\t"
	"movl	%0,	%%eax	\n\t"
	"movl	%%eax,		%%cr3	\n\t"
	"movl	%%cr0,		%%eax	\n\t"
	"orl	$0x80000000,	%%eax	\n\t"
	"movl	%%eax,		%%cr0	\n\t"
	"popl	%%eax				\n\t"
	:
	:"r"(PD_START)
	);

#else
	__asm{
		push eax
		mov eax,PD_START
		mov cr3,eax
		mov eax,cr0
		or eax,0x80000000
		mov cr0,eax
		pop eax
	}
#endif
}

//Halt current CPU in case of IDLE,it will be called by IDLE thread.
VOID HaltSystem()
{
#ifdef _POSIX_
	__asm__ __volatile__ ("hlt	\n\t");
#else
	__asm{
		hlt
	}
#endif
}

//
//This routine initializes a kernel thread's context.
//This routine's action depends on different platform.
//
VOID InitKernelThreadContext(__KERNEL_THREAD_OBJECT* lpKernelThread,
							 __KERNEL_THREAD_WRAPPER lpStartAddr)
{
	DWORD*        lpStackPtr = NULL;
	DWORD         dwStackSize = 0;

	if((NULL == lpKernelThread) || (NULL == lpStartAddr))  //Invalid parameters.
	{
		return;
	}

//Define a macro to make the code readable.
#define __PUSH(stackptr,val) \
	do{  \
	(DWORD*)(stackptr) --; \
	*((DWORD*)stackptr) = (DWORD)(val); \
	}while(0)

	lpStackPtr = (DWORD*)lpKernelThread->lpInitStackPointer;

	__PUSH(lpStackPtr,lpKernelThread);       //Push lpKernelThread to stack.
	__PUSH(lpStackPtr,NULL);                 //Push a new return address,simulate a call.
	__PUSH(lpStackPtr,INIT_EFLAGS_VALUE);    //Push EFlags.
	__PUSH(lpStackPtr,0x00000008);           //Push CS.
	__PUSH(lpStackPtr,lpStartAddr);  //Push start address.
	__PUSH(lpStackPtr,0);                   //Push eax.
	__PUSH(lpStackPtr,0);
	__PUSH(lpStackPtr,0);
	__PUSH(lpStackPtr,0);
	__PUSH(lpStackPtr,0);
	__PUSH(lpStackPtr,0);
	__PUSH(lpStackPtr,0);

	//Save context.
	lpKernelThread->lpKernelThreadContext = (__KERNEL_THREAD_CONTEXT*)lpStackPtr;
	return;
}

//Get time stamp counter.
VOID __GetTsc(__U64* lpResult)
{
#ifdef _POSIX_
	__asm__(
		".code32		\n\t"
		"pushl 	%%eax	\n\t"
		"pushl	%%edx	\n\t"
		"pushl	%%ebx	\n\t"
		"rdtsc			\n\t"
		"movl	0x08(%%ebp),	%%ebx	\n\t"
		"movl	%%eax,			(%%ebx)	\n\t"
		"movl	%%edx,		0x04(%%ebx)	\n\t"
		"popl	%%ebx	\n\t"
		"popl	%%edx	\n\t"
		"popl	%%eax	\n\t"
			::
	);
#else
	__asm{
		push eax
		push edx
		push ebx
		rdtsc    //Read time stamp counter.
		mov ebx,dword ptr [ebp + 0x08]
		mov dword ptr [ebx],eax
		mov dword ptr [ebx + 0x04],edx
		pop ebx
		pop edx
		pop eax
	}
#endif
}

//A local helper routine used to read CMOS date and time information.
static
//_declspec(naked)
		ReadCmosData(WORD* pData,BYTE nPort)
{
#ifdef _POSIX_
	__asm__(
	".code32				\n\t"
	"pushl	%%ebp			\n\t"
	"movl	%%esp,	%%ebp	\n\t"
	"pushl	%%ebx			\n\t"
	"pushl	%%edx			\n\t"

	"movl	0x08(%%ebp),%%ebx	\n\t"
	"xorw	%%ax,	%%ax	\n\t"
	"movb	%0,	%%al	\n\t"
	"outb	%%al,	$0x70	\n\t"
	"inb	$0x71,	%%al	\n\t"
	"movw	%%ax,	(%%ebx)	\n\t"

	"popl	%%edx			\n\t"
	"popl	%%ebx			\n\t"
	"leave					\n\t"
	"ret					\n\t"	//retn->ret
	:
	:"r"(nPort)
	);

#else
	__asm{
		    push ebp
			mov ebp,esp
			push ebx
			push edx

			mov ebx,dword ptr [ebp + 0x08]

		    xor ax,ax
			mov al,nPort
			out 70h,al
			in al,71h
			mov word ptr [ebx],ax

			pop edx
			pop ebx
			leave
			retn
	}
#endif
}

//Get CMOS date and time.
VOID __GetTime(BYTE* pDate)
{
	ReadCmosData((WORD*)&pDate[0],9);//read year
	ReadCmosData((WORD*)&pDate[1],8);//read month
	ReadCmosData((WORD*)&pDate[2],7);//read day

	ReadCmosData((WORD*)&pDate[3],4);//read hour
	ReadCmosData((WORD*)&pDate[4],2);//read Minute
	ReadCmosData((WORD*)&pDate[5],0);//read second

	//BCD convert
	pDate[0] = BCD_TO_DEC_BYTE(pDate[0]);
	pDate[1] = BCD_TO_DEC_BYTE(pDate[1]);
	pDate[2] = BCD_TO_DEC_BYTE(pDate[2]);
	pDate[3] = BCD_TO_DEC_BYTE(pDate[3]);
	pDate[4] = BCD_TO_DEC_BYTE(pDate[4]);
	pDate[5] = BCD_TO_DEC_BYTE(pDate[5]);
}

#define CLOCK_PER_MICROSECOND 1024  //Assume the CPU's clock is 1G Hz.

VOID __MicroDelay(DWORD dwmSeconds)
{
	__U64    u64CurrTsc;
	__U64    u64TargetTsc;

	__GetTsc(&u64TargetTsc);
	u64CurrTsc.dwHighPart = 0;
	u64CurrTsc.dwLowPart  = dwmSeconds;
	//u64Mul(&u64CurrTsc,CLOCK_PER_MICROSECOND);
	u64RotateLeft(&u64CurrTsc,10);
	u64Add(&u64TargetTsc,&u64CurrTsc,&u64TargetTsc);
	while(TRUE)
	{
		__GetTsc(&u64CurrTsc);
		if(MoreThan(&u64CurrTsc,&u64TargetTsc))
		{
			break;
		}
	}
	return;
}

VOID __outd(WORD wPort,DWORD dwVal)  //Write one double word to a port.
{
#ifdef _POSIX_
	__asm__ (
		".code32		\n\t"
		"pushl	%%eax	\n\t"
		"pushl	%%edx	\n\t"
		"movw	%0,	%%dx	\n\t"
		"movl	%1,	%%eax	\n\t"
		"outw	%%eax,	%%dx	\n\t"
		"popl	%%edx	\n\t"
		"popl	%%eax	\n\t"
		:
		:"r"(wPort), "r"(dwVal)
	);
#else
	__asm{
		push eax
		push edx
		mov dx,wPort
		mov eax,dwVal
		out dx,eax
		pop edx
		pop eax
	}
#endif
}

DWORD __ind(WORD wPort)    //Read one double word from a port.
{
	DWORD    dwRet       = 0;
#ifdef _POSIX_
	__asm__(
	".code32	\n\t"
	"pushl	%%eax	\n\t"
	"pushl	%%edx	\n\t"
	"movb	%1,	%%dx	\n\t"
	"inw	%%dx,	%%eax	\n\t"
	"movl	%%eax,	%0	\n\t"
	"popl	%%edx	\n\t"
	"popl	%%eax	\n\t"
	:"=r"(dwRet)
	:"r"(wPort)
	);
#else
	__asm{
		push eax
		push edx
		mov dx,wPort
		in eax,dx
		mov dwRet,eax
		pop edx
		pop eax
	}
#endif
	return dwRet;
}

VOID __outb(UCHAR _bt,WORD _port)  //Send bt to port.
{
#ifdef _POSIX_
	__asm__(
	".code32		    \n\t"
	"pushl %%eax		\n\t"
	"pushl %%edx        \n\t"
	"movb %0, %%al      \n\t"
	"movw %1, %%dx      \n\t"
	"outb %%al, %%dx    \n\t"
	"popl %%edx 	    \n\t"
	"popl %%eax			\n\t"
	:
	:"r"(_bt),"r"(_port)
	);
#else
	__asm{
		push eax
		push edx
		mov al,_bt
		mov dx,_port
		out dx,al
		pop edx
		pop eax
	}
#endif
}

UCHAR __inb(WORD _port)  //Receive a byte from port.
{
	UCHAR uRet;
#ifdef _POSIX_
	__asm__ __volatile__(
		".code32				\n\t"
		"xorl 	%%eax,	%%eax 	\n\t"
		"pushl 	%%edx    		\n\t"
		"movw 	%1,		%%dx	\n\t"
		"inb 	%%dx,	%%al    \n\t"
		"popl	%%edx     		\n\t"
		"movb 	%%al,	%0 		\n\t"
			:"=c"(uRet)
			:"r"(_port)
	);
#else
	asm{
		xor eax,eax
		push edx
		mov dx,_port
		in al,dx
		pop edx
		mov uRet,al
	}
#endif
	return uRet;
}

WORD __inw(WORD wPort)
{
	WORD    wRet       = 0;
#ifdef _POSIX_
	__asm__ (
			".code32					\n\t"
			"pushl %%eax				\n\t"
			"pushl %%edx				\n\t"
			"movw %1,%%dx				\n\t"
			"inb %%dx,%%ax				\n\t"
			"movb %%ax,%0				\n\t"
			"popl %%edx					\n\t"
			"popl %%eax					\n\t"
			:"=r"(wRet)
			:"r"(wPort)
	);
#else
	__asm{
		push eax
		push edx
		mov dx,wPort
		in ax,dx
		mov wRet,ax
		pop edx
		pop eax
	}
#endif
	return wRet;
}

VOID __outw(WORD wVal,WORD wPort)
{
#ifdef _POSIX_
	__asm__(
		".code32	\n\t"
		"pushl %%eax				\n\t\
		pushl 	%%edx             	\n\t\
		movl %0,	%%ax          	\n\t\
		movl %0,	%%dx         	\n\t\
		outw %%ax, 	%%dx           	\n\t\
		popl %%edx				 	\n\t\
		popl %%eax					\n\t"
			:
			:"r"(wVal),"r"(wPort)
	);

	#else
	__asm{
		push eax
		push edx
		mov ax,wVal
		mov dx,wPort
		out dx,ax
		pop edx
		pop eax
	}
#endif
}

//__declspec(naked)
VOID __inws(BYTE* pBuffer,DWORD dwBuffLen,WORD wPort)
{
#ifdef __I386__

#ifdef _POSIX_
	__asm__ (
	"	.code32								\n\t"
	"	pushl 	%%ebp                                      \n\t"
	"	movl 	%%esp,	%%ebp                               \n\t"
	"	pushl %%ecx                                      \n\t"
	"	pushl %%edx                                      \n\t"
	"	pushl %%edi                                      \n\t"
	"	movl 	0x08(%%ebp),	%%edi				     \n\t"
	"	movl 	0x0c(%%ebp),	%%ecx						\n\t"
	"	shrl 	$0x01,			%%ecx                          \n\t"
	"	movw 	0x10(%%ebp),	%%dx				\n\t"
	"	cld                                        	\n\t"
	"	rep 	insw                                    \n\t"
	"	popl %%edi                                     \n\t"
	"	popl %%edx                                     \n\t"
	"	popl %%ecx                                     \n\t"
	"	leave                                      	\n\t"
	"	ret                                		\n\t"	//retn->ret
			:	:
	);

#else
	__asm{
		push ebp
		mov 	ebp,esp
		push ecx
		push edx
		push edi
		mov edi,dword ptr [ebp + 0x08]
		mov ecx,dword ptr [ebp + 0x0c]
		shr ecx,0x01
		mov dx,  word ptr [ebp + 0x10]
		cld
		rep insw
		pop edi
		pop edx
		pop ecx
		leave
		retn
	}
#endif
#endif
}

//__declspec(naked)
VOID __outws(BYTE* pBuffer,DWORD dwBuffLen,WORD wPort)
{
#ifdef __I386__
#ifdef _POSIX_
	__asm__ (
	".code32			  				\n\t"
	" pushl %%ebp                        \n\t"
	" movl %%esp, %%ebp                  \n\t"
	" pushl %%ecx                           \n\t"
	" pushl %%edx                           \n\t"
	" pushl %%esi                           \n\t"
	" movw 0x10(%%ebp), %%dx			\n\t"
	" rep outsw                        	\n\t"
	" popl %%esi                            \n\t"
	" popl %%edx                            \n\t"
	" popl %%ecx                            \n\t"
	" leave                              \n\t"
	" ret			                \n\t"	//retn
			::
	);

#else
	__asm{
		push ebp
		mov ebp,esp
		push ecx
		push edx
		push esi
		mov esi,dword ptr [ebp + 0x08]
		mov ecx,dword ptr [ebp + 0x0c]
		shr ecx,0x01
		mov dx,  word ptr [ebp + 0x10]
		rep outsw
		pop esi
		pop edx
		pop ecx
		leave
		retn
	}
#endif
#endif
}

//Implemention of uname routine,which is hardware specified.
int uname(struct utsname* __name)
{
	memset(__name,sizeof(struct utsname),0);
	strcpy(__name->sysname,"HelloX OS");
	strcpy(__name->version,OS_VERSION);
	strcpy(__name->machine,"IA32");
	strcpy(__name->release,OS_VERSION);
	return 0;
}

#endif //__I386__
