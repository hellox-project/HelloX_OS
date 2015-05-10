//Implementation of setjmp,sigsetjmp,longjmp,siglongjmp.

#include <stdafx.h>
#include <setjmp.h>

#define OFS_EBP 0
#define OFS_EBX 4
#define OFS_EDI 8
#define OFS_ESI 12
#define OFS_ESP 16
#define OFS_EIP 20

//setjmp.
__declspec(naked) int __setjmp(jmp_buf env)
{
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
}

//longjmp.
__declspec(naked) void __longjmp(jmp_buf env,int value)
{
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
}

int sigsetjmp(sigjmp_buf env,int savesigs)
{
	return 0;
}

void siglongjmp(sigjmp_buf env,int value)
{
}

