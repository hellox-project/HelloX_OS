//Implementation of setjmp,sigsetjmp,longjmp,siglongjmp.

#include <StdAfx.h>
#include <setjmp.h>

#define OFS_EBP 0
#define OFS_EBX 4
#define OFS_EDI 8
#define OFS_ESI 12
#define OFS_ESP 16
#define OFS_EIP 20

//setjmp.
//__declspec(naked)
int __setjmp(jmp_buf env)
{
#ifdef _POSIX_
	//TODO gaojie
	__asm__ __volatile__(
			".code32;"
			"movl 4(%%esp), 	%%edx			;"
			"movl (%%esp),	 	%%eax			;"
			"movl %%eax,		OFS_EIP(%%edx)	;"
			"movl %%ebp,		OFS_EBP(%%edx)	;"
			"movl %%ebx,		OFS_EBX(%%edx)	;"
			"movl %%edi,		OFS_EDI(%%edx)	;"
			"movl %%esi,		OFS_ESI(%%edx)	;"
			"movl %%esp,		OFS_ESP(%%edx)	;"
			"xorl %%eax,		%%eax			;"
			"ret								;"
			:::);
#else
	__asm{
		mov edx,4[esp]
		mov eax,[esp]
		mov OFS_EIP[edx],eax
		mov OFS_EBP[edx],ebp
		mov OFS_EBX[edx],ebx
		mov OFS_EDI[edx],edi
		mov OFS_ESI[edx],esi
		mov OFS_ESP[edx],esp
		xor eax,eax
		ret
	}
#endif
}

//longjmp.
//__declspec(naked)
void __longjmp(jmp_buf env,int value)
{
#ifdef _POSIX_
	//TODO gaojie
	__asm__ __volatile__(
			".code32						;"
			"movl 4(%%esp), 		%%edx	;"
			"movl 8(%%esp), 		%%eax	;"
			"movl OFS_ESP(%%edx),	%%esp	;"
			"movl OFS_EIP(%%edx),	%%ebx	;"
			"movl %%ebx,			(%%esp)	;"
			"movl OFS_EBP(%%edx), 	%%ebp	;"
			"movl OFS_EBX(%%edx), 	%%ebx	;"
			"movl OFS_EDI(%%edx), 	%%edi	;"
			"movl OFS_ESI(%%edx), 	%%esi	;"
			"ret;"
			:
			::);
#else
	__asm{
		mov edx,4[esp]
		mov eax,8[esp]
		
		mov esp,OFS_ESP[edx]
		mov ebx,OFS_EIP[edx]
		mov [esp],ebx

		mov ebp,OFS_EBP[edx]
		mov ebx,OFS_EBX[edx]
		mov edi,OFS_EDI[edx]
		mov esi,OFS_ESI[edx]

		ret
	}
#endif
}

int sigsetjmp(sigjmp_buf env,int savesigs)
{
	return 0;
}

void siglongjmp(sigjmp_buf env,int value)
{
}

