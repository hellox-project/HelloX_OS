//***********************************************************************/
//    Author                    : Garry
//    Original Date             : May,27 2004
//    Module Name               : hellocn.cpp
//    Module Funciton           : 
//                                This module countains the source code.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#include <StdAfx.h>
#include <arch.h>
#include <console.h>
#include <stdio.h>

#ifdef __I386__  //Only available in x86 based PC platform.

INT_HANDLER SetGeneralIntHandler(__GENERAL_INTERRUPT_HANDLER TimerHandler)
{
#ifdef __GCC__
	__asm__ (
	"pushl	%%ebx							\n\t"
	"pushl	%%ecx							\n\t"
	"movl 	%0,	%%ebx						\n\t"
	"movl 	%1,	%%eax						\n\t"
	"movl (%%ebx),			%%ecx			\n\t"
	"movl %%eax,			(%%ebx)			\n\t"
	"movl %%ecx,			%%eax			\n\t"
	"popl	%%ecx							\n\t"
	"popl	%%ebx							\n\t"
	:
	:"i"(__GENERAL_INT_HANDLER_BASE), "r"(TimerHandler)
	);
#else
	__asm{
		push ebx
		push ecx
		mov ebx,__GENERAL_INT_HANDLER_BASE
		mov eax,TimerHandler
		mov ecx,dword ptr [ebx]
		mov dword ptr [ebx],eax
		mov eax,ecx
		pop ecx
		pop ebx
	}
#endif
}

#ifndef __GCC__
__declspec(naked)
#endif

VOID WriteByteToPort(UCHAR byte,WORD wPort)
{
#ifdef __GCC__
	__asm__ (
	".code32				\n\t"
	"pushl	%%ebp			\n\t"
	"movl	%%esp,	%%ebp	\n\t"
	"pushl	%%edx			\n\t"
	"movb	8(%%ebp),	%%al\n\t"
	"movw	12(%%ebp),	%%ax\n\t"
	"outb	%%al,	%%dx	\n\t"
	"popl	%%edx	\n\t"
	"leave			\n\t"
	"ret			\n\t"
	:
	);
#else
	__asm{
		push ebp
		mov ebp,esp
		push edx
		mov al,byte ptr [ebp + 8]
		mov dx,word ptr [ebp + 12]
		out dx,al
		pop edx
		leave
		retn
	}
#endif
}

#ifndef __GCC__
__declspec(naked)
#endif

VOID ReadByteStringFromPort(LPVOID lpBuffer,DWORD dwBufLen,WORD wPort)
{
#ifdef __GCC__
	__asm__(
		"pushl	%%ebp	\n\t"
		"movl	%%esp,	%%ebp	\n\t"
		"pushl	%%ecx	\n\t"
		"pushl	%%edx	\n\t"
		"pushl	%%edi	\n\t"
		"movl	8(%%ebp),	%%edi	\n\t"
		"movl	12(%%ebp),	%%edi	\n\t"
		"movw	16(%%ebp),	%%dx	\n\t"
		"rep	insb	\n\t"
		"popl	%%edi	\n\t"
		"popl	%%edx	\n\t"
		"popl	%%ecx	\n\t"
		"leave	\n\t"
		"ret	\n\t"
			:
	);

#else
	__asm{
		push ebp
		mov ebp,esp
		//push ebx
		push ecx
		push edx
		push edi
		mov edi,dword ptr [ebp + 8]
		//mov esi,dword ptr [ebx]
		mov ecx,dword ptr [ebp + 12]
		mov dx,word ptr [ebp + 16]
		rep insb
		pop edi
		pop edx
		pop ecx
		//pop ebx
		leave
		retn
	}
#endif
}

#ifndef __GCC__
__declspec(naked)
#endif
VOID WriteByteStringToPort(LPVOID lpBuffer,DWORD dwBufLen,WORD wPort)
{

#ifdef __GCC__
	__asm__(
		"pushl	%%ebp	\n\t"
		"movl	%%esp,	%%ebp	\n\t"
		"pushl	%%ecx	\n\t"
		"pushl	%%edx	\n\t"
		"pushl	%%esi	\n\t"
		"movl	8(%%ebp),	%%esi	\n\t"
		"movl	12(%%ebp),	%%ecx	\n\t"
		"movw	16(%%ebp),	%%dx	\n\t"
		"rep	outsb	\n\t"
		"popl	%%esi	\n\t"
		"popl	%%edx	\n\t"
		"popl	%%ecx	\n\t"
		"leave	\n\t"
		"ret	\n\t"
			:
	);
#else
	__asm{
		push ebp
		mov ebp,esp
		push ecx
		push edx
		push esi
		mov esi,dword ptr [ebp + 8]
		mov ecx,dword ptr [ebp + 12]
		mov dx,word ptr [ebp + 16]
		rep outsb
		pop esi
		pop edx
		pop ecx
		leave
		retn
	}
#endif
}

#ifndef __GCC__
__declspec(naked)
#endif

 VOID ReadWordFromPort(WORD* pWord,WORD wPort)
{
#ifdef __GCC__
	__asm__(
	"pushl	%%ebp	\n\t"
	"movl	%%esp,	%%ebp	\n\t"
	"pushl	%%ebx	\n\t"
	"pushl	%%edx	\n\t"
	"movw	0x0c(%%ebp),	%%dx	\n\t"
	"movl	0x08(%%ebp),	%%ebx	\n\t"
	"inw	%%dx,	%%ax	\n\t"
	"movw	%%ax,	(%%ebx)	\n\t"
	"popl	%%edx	\n\t"
	"popl	%%ebx	\n\t"
	"leave			\n\t"
	"ret			\n\t"
			:
	);
#else
	__asm{
		push ebp
		mov ebp,esp
		push ebx
		push edx
		mov dx,word ptr [ebp + 0x0c]
		mov ebx,dword ptr [ebp + 0x08]
		in ax,dx
		mov word ptr [ebx],ax
		pop edx
		pop ebx
		leave
		retn
	}
#endif
}

#ifndef __GCC__
__declspec(naked)
#endif

 VOID WriteWordToPort(WORD w1,WORD w2)
{
#ifdef __GCC__
	__asm__(
			"pushl	%%ebp           \n\t"
			"movl %%esp,	%%ebp   \n\t"
			"pushw %%dx             \n\t"
			"movw 0x0c(%%ebp),	%%dx\n\t"
			"movw 0x08(%%ebp),	%%ax\n\t"
			"outw %%ax, %%dx        \n\t"
			"popw %%dx              \n\t"
			"leave                  \n\t"
			"ret					\n\t"
			:
	);
#else
	__asm{
		push ebp
		mov ebp,esp
		push dx
		mov dx,word ptr [ebp + 0x0c]
		mov ax,word ptr [ebp + 0x08]
		out dx,ax
		pop dx
		leave
		retn
	}
#endif
}

#ifndef __GCC__
__declspec(naked)
#endif

 VOID ReadWordStringFromPort(LPVOID p1,DWORD d1,WORD w1)
{
#ifdef __GCC__
	__asm__(
	"pushl %%ebp         \n\t                 "
	"movl %%esp, %%ebp   \n\t                 "
	"pushl %%ecx         \n\t                 "
	"pushl %%edx         \n\t                 "
	"pushl %%edi         \n\t                 "
	"movl 0x08(%%ebp),	%%edi				\n\t"
	"movl 0x0c(%%ebp),	%%ecx				\n\t"
	"shrl $0x01,		%%ecx             	\n\t"
	"movw 0x10(%%ebp),	%%dx				\n\t"
	"cld                               		\n\t"
	"rep insw               	           	\n\t"
	"popl %%edi                 	        \n\t  "
	"popl %%edx                     	    \n\t  "
	"popl %%ecx                         	\n\t  "
	"leave		\n\t"
	"ret		\n\t"
			:
	);
#else
	__asm{
		push ebp
		mov ebp,esp
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
}

#ifndef __GCC__
__declspec(naked)
#endif

 VOID WriteWordStringToPort(LPVOID p1,DWORD d1,WORD w1)
{
#ifdef __GCC__
	__asm__(
		"pushl %%ebp				\n\t"
		"movl %%esp, %%ebp          \n\t"
		"pushl %%ecx                \n\t"
		"pushl %%edx                \n\t"
		"pushl %%esi                \n\t"
		"movl 0x0c(%%ebp),	%%esi   \n\t"
		"movl 0x0c(%%ebp),	%%ecx   \n\t"
		"shrl $0x02,		%%ecx   \n\t"
		"movw 0x10(%%ebp),	%%dx    \n\t"
		"rep outsw                  \n\t"
		"popl %%esi                 \n\t"
		"popl %%edx                 \n\t"
		"popl %%ecx                 \n\t"
		"leave                      \n\t"
		"ret						\n\t"
			:
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
		shr ecx,0x02
		mov dx,  word ptr [ebp + 0x10]
		rep outsw
		pop esi
		pop edx
		pop ecx
		leave
		retn
	}
#endif
}

#endif //__I386__

//
//Error handling routines implementing.
//
static VOID FatalErrorHandler(DWORD dwReason,LPSTR lpszMsg)
{
	PrintLine("Error Level : FATAL(1)");
	if(lpszMsg != NULL)
		PrintLine(lpszMsg);
	return;
}

static VOID CriticalErrorHandler(DWORD dwReason,LPSTR lpszMsg)
{
	PrintLine("Error Level : CRITICAL(2)");
	if(lpszMsg != NULL)
		PrintLine(lpszMsg);
	return;
}

static VOID ImportantErrorHandler(DWORD dwReason,LPSTR lpszMsg)
{
	PrintLine("Error Level : IMPORTANT(3)");
	if(lpszMsg != NULL)
		PrintLine(lpszMsg);
	return;
}

static VOID AlarmErrorHandler(DWORD dwReason,LPSTR lpszMsg)
{
	PrintLine("Error Level : ALARM(4)");
	if(lpszMsg != NULL)
		PrintLine(lpszMsg);
	return;
}

static VOID InformErrorHandler(DWORD dwReason,LPSTR lpszMsg)
{
	PrintLine("Error Level : INFORM(5)");
	if(lpszMsg != NULL)
		PrintLine(lpszMsg);
	return;
}

VOID ErrorHandler(DWORD dwLevel,DWORD dwReason,LPSTR lpszMsg)
{
	switch(dwLevel)
	{
	case ERROR_LEVEL_FATAL:
		FatalErrorHandler(dwReason,lpszMsg);
		break;
	case ERROR_LEVEL_CRITICAL:
		CriticalErrorHandler(dwReason,lpszMsg);
		break;
	case ERROR_LEVEL_IMPORTANT:
		ImportantErrorHandler(dwReason,lpszMsg);
		break;
	case ERROR_LEVEL_ALARM:
		AlarmErrorHandler(dwReason,lpszMsg);
		break;
	case ERROR_LEVEL_INFORM:
		InformErrorHandler(dwReason,lpszMsg);
		break;
	default:
		break;
	}
}

/* Show out bug information. */
VOID __BUG(LPSTR lpszFileName,DWORD dwLineNum)
{
	DWORD   dwFlags;
	unsigned int processor_id = __CURRENT_PROCESSOR_ID;
	__KERNEL_THREAD_OBJECT* pKernelThread = __CURRENT_KERNEL_THREAD;
	
	/* Show general bug info. */
	_hx_printk("\r\nBUG encountered[curr_processor = %d].\r\n",processor_id);
	_hx_printk("File name : %s\r\nCode Lines : %d\r\n",lpszFileName,dwLineNum);
	/* Show out specific information according current execution context. */
	if (IN_INTERRUPT())
	{
		/* Show interrupt related information. */
		_hx_printk("In interrupt context,vector:%d.\r\n", System.ucCurrInt[processor_id]);
		/* Show out interrupted kernel thread information. */
		if (pKernelThread)
		{
			_hx_printk("Interrupted kernel thread:%s\r\n", pKernelThread->KernelThreadName);
		}
	}
	else if (IN_SYSINITIALIZATION())
	{
		/* In process of system initialization. */
		_hx_printf("In process of system initialization.\r\n");
	}
	else
	{
		/* In normal thread context. */
		_hx_printf("Current kthread: %s\r\n",
			__CURRENT_KERNEL_THREAD->KernelThreadName);
	}
	/* Current CPU dive into a halt state. */
	__DISABLE_LOCAL_INTERRUPT(dwFlags);
	while (TRUE)
	{
		/* Enter halt state to save energy. */
		HaltSystem();
	}
	__RESTORE_LOCAL_INTERRUPT(dwFlags);
}

/* Print logging header information,such as date,time,level... */
void LogHeader(int level)
{
	BYTE   time[6];

	/* Get time and date from arch spec API. */
	__GetTime(&time[0]);
	_hx_printf("[%d-%d-%d %d:%d:%d]",
		time[0] + 2000,
		time[1],
		time[2],
		time[3],
		time[4],
		time[5]);
}
