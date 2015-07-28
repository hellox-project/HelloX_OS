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

#include <StdAfx.h>
#include <arch.h>
#include <stdio.h>
#include <sys/utsname.h>


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

//Exception table.
typedef struct{
	char*  description;
	BOOL   bErrorCode;
}__EXCEPTION_TABLE;

#define MAX_EXCEP_TABLE_SIZE 20  //Maximal exceptions available under current processor.

static __EXCEPTION_TABLE __ExcepTable[MAX_EXCEP_TABLE_SIZE] = {
	{ "Divide Error(#DE)", FALSE },
	{ "Reserved(#DB)", FALSE },
	{ "NMI Interrupt", FALSE },
	{ "Breakpoint(#BP)", FALSE },
	{ "Overflow(#OF)", FALSE },
	{ "Bound Range Exceed(#BR)", FALSE },
	{ "Invalid Opcode(#UD)", FALSE },
	{ "Device Not Available(#NM)", FALSE },
	{ "Double Fault(#DF)", TRUE },
	{ "Coprocessor Segment Overrun", FALSE },
	{ "Invalid TSS(#TS)", TRUE },
	{ "Segment Not Present(#NP)", TRUE },
	{ "Stack Segment Fault(#SS)", TRUE },
	{ "General Protection(#GP)", TRUE },
	{ "Page Fault(#PF)", TRUE },
	{ "Internal reserved", FALSE },
	{ "x87 Floating point error(#MF)", FALSE },
	{ "Alignment check(#AC)", TRUE },
	{ "Machine Check(#MC)", FALSE },
	{ "SIMD Floating-Point Exception(#XM)", FALSE }
};

//Exception specific operations.
static VOID ExcepSpecificOps(LPVOID pESP, UCHAR ucVector)
{
	DWORD excepAddr;

	if (14 == ucVector)  //Page fault.
	{
#ifdef __GCC__
		__asm__ __volatile__(
			".code32            \n\t"
			"pushl       %%eax     \n\t"
			"movl        %%cr2,     %%eax     \n\t"
			"movl        %%eax, %0                \n\t"
			"popl        %%eax                       \n\t"
			:"=g"(excepAddr) :  : "memory");
#else
		__asm{
			push eax
				mov eax, cr2
				mov excepAddr, eax
				pop eax
		}
#endif
		_hx_printf("\tException addr: 0x%X.\r\n", excepAddr);
	}
}

//Processor specified exception handler,for x86.
VOID PSExcepHandler(LPVOID pESP, UCHAR ucVector)
{
	if (ucVector >= MAX_EXCEP_TABLE_SIZE)  //Invalid exception number.
	{
		_hx_printf("\tInvalid exception number(#%d) for x86.\r\n", ucVector);
		return;
	}
	//Show detail information about the exception.
	_hx_printf("\tException Desc: %s.\r\n", __ExcepTable[ucVector].description);
	if (__ExcepTable[ucVector].bErrorCode)
	{
		_hx_printf("\tError Code: 0x%X.\r\n", *((DWORD*)pESP + 7));
		_hx_printf("\tEIP: 0x%X.\r\n", *((DWORD*)pESP + 8));
		_hx_printf("\tCS: 0x%X.\r\n", *((DWORD*)pESP + 9));
		_hx_printf("\tEFlags: 0x%X.\r\n", *((DWORD*)pESP + 10));
	}
	else  //Without error code pushed in stack.
	{
		_hx_printf("\tEIP: 0x%X.\r\n", *((DWORD*)pESP + 7));
		_hx_printf("\tCS: 0x%X.\r\n", *((DWORD*)pESP + 8));
		_hx_printf("\tEFlags: 0x%X.\r\n", *((DWORD*)pESP + 9));
	}

	//Check if specific operation exists for the exception.
	ExcepSpecificOps(pESP, ucVector);
	return;
}

//
//This routine switches the current executing path to the new one identified
//by lpContext.
//
#ifndef __GCC__
__declspec(naked)
#endif
VOID __SwitchTo(__KERNEL_THREAD_CONTEXT* lpContext)
{
#ifdef __GCC__
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
#ifndef __GCC__
__declspec(naked)
#endif
VOID __SaveAndSwitch(__KERNEL_THREAD_CONTEXT** lppOldContext,
									   __KERNEL_THREAD_CONTEXT** lppNewContext)
{

#ifdef __GCC__
	__asm__(
	".code32								\n\t"
	"movl	%%esp,	%0                  	\n\t"
	"popl	%1		                      	\n\t"
	"movl	%%eax,	%2   				\n\t"
	"pushf             					\n\t"
	"xorl	%%eax,	%%eax     				\n\t"
	"movw	%%cs,	%%ax          				\n\t"
	"pushl	%%eax           				\n\t"
	"pushl	%3      						\n\t"
	"pushl  %4      						\n\t"
	"pushl %%ebx                          	\n\t"
	"pushl %%ecx                          	\n\t"
	"pushl %%edx                          	\n\t"
	"pushl %%esi                          	\n\t"
	"pushl %%edi                          	\n\t"
	"pushl %%ebp                          	\n\t"
	"             	                     	\n\t"
	"movl %5, %%ebp                  		\n\t"
	"movl 0x04(%%ebp),	%%ebx				\n\t"
	"movl %%esp,	(%%ebx)					\n\t"
	"                                     	\n\t"
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
#ifdef __GCC__
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
#ifdef __GCC__
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
#ifdef __GCC__
	__asm__(
		".code32		\n\t"
		"pushl	%%ebp	\n\t"
		"movl	%%esp,	%%ebp	\n\t"
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
		"popl	%%ebp	\n\t"
		"ret			\n\t"
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
#ifndef __GCC__
	_declspec(naked)
#endif
ReadCmosData(WORD* pData,BYTE nPort)
{
#ifdef __GCC__
	__asm__(
	".code32				\n\t"
	"pushl	%%ebp			\n\t"
	"movl	%%esp,	%%ebp	\n\t"
	"pushl	%%ebx			\n\t"
	"pushl	%%edx			\n\t"
	"movl	0x08(%%ebp),%%ebx	\n\t"
	"xorw	%%ax,	%%ax	\n\t"
	"movb	%0,	%%al		\n\t"
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
#ifdef __GCC__
	asm volatile("outl %0,%1" : : "a" (dwVal), "dN" (wPort));

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
#ifdef __GCC__
	asm volatile("inl %1,%0" : "=a" (dwRet) : "dN" (wPort));
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
#ifdef __GCC__
	asm volatile("outb %0,%1" : : "a" (_bt), "dN" (_port));

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
#ifdef __GCC__
	asm volatile("inb %1,%0" : "=a" (uRet) : "dN" (_port));

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
#ifdef __GCC__
	asm volatile("inw %1,%0" : "=a" (wRet) : "dN" (wPort));
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
#ifdef __GCC__
	asm volatile("outw %0,%1" : : "a" (wVal), "dN" (wPort));
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

#ifndef __GCC__
__declspec(naked)
#endif
VOID __inws(BYTE* pBuffer,DWORD dwBuffLen,WORD wPort)
{
#ifdef __I386__

#ifdef __GCC__
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

#ifndef __GCC__
__declspec(naked)
#endif
VOID __outws(BYTE* pBuffer,DWORD dwBuffLen,WORD wPort)
{
#ifdef __I386__
#ifdef __GCC__
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
	" leave                             \n\t"
	" ret			                	\n\t"	//retn
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
