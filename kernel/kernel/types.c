//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Sep,16 2005
//    Module Name               : TYPES.CPP
//    Module Funciton           : 
//                                This module countains the implementation of several types
//                                supported by Hello China.
//                                The operating interface,i.e,the function that operate these
//                                data structures,are alse implemented here.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#include "StdAfx.h"

/* 
 * Exclusive operations for 64 bits integer, under
 * 32 bits CPU. 
 * CMPXCHG8B is used to make sure the operation
 * is atomic. This instruction's operation as:
 * 1) Compare EDX:EAX with m64;
 * 2) If equal, set ZF and load ECX:EBX into m64;
 * 3) Else, clear ZF and load m64 into EDX:EAX.
 */
void LockedIncrement64(__u64* large_int)
{
	__asm {
		mov esi, large_int
		mov eax, dword ptr[esi]
		mov edx, dword ptr[esi + 4]
	__AGAIN:
		mov ebx, eax
		mov ecx, edx
		add ebx, 0x01
		adc ecx, 0
		lock cmpxchg8b [esi]
		jnz __AGAIN
	}
}

void LockedDecrement64(__u64* large_int)
{
	__asm {
		mov esi, large_int
		mov eax, dword ptr[esi]
		mov edx, dword ptr[esi + 4]
	__AGAIN:
		mov ebx, eax
		mov ecx, edx
		sub ebx, 0x01
		sbb ecx, 0
		lock cmpxchg8b[esi]
		jnz __AGAIN
	}
}

void LockedGet64(__u64* dest, __u64* src)
{
	__asm {
		mov esi, src
		mov eax, dword ptr[esi]
		mov edx, dword ptr[esi + 4]
	__AGAIN:
		mov ebx, eax
		mov ecx, edx
		lock cmpxchg8b[esi]
		jnz __AGAIN
		mov esi, dest
		mov dword ptr[esi], ebx
		mov dword ptr[esi + 4], ecx
	}
}

void LockedPut64(__u64* dest, __u64* src)
{
	__asm {
		mov esi, src
		mov ebx, dword ptr[esi]
		mov ecx, dword ptr[esi + 4]
		mov esi, dest
		mov eax, dword ptr[esi]
		mov edx, dword ptr[esi + 4]
	__AGAIN:
		lock cmpxchg8b[esi]
		jnz __AGAIN
	}
}

//
//The implementation of unsigned 64 bits add.
//
VOID u64Add(__U64* lpu64_1,__U64* lpu64_2,__U64* lpu64_result)
{
	if((NULL == lpu64_1) || (NULL == lpu64_2) || (NULL == lpu64_result)) //Parameters check.
	{
	   return;
	}

#ifdef __I386__  //Use asmble language to accelate speed.
#ifdef __GCC__
	__asm__ __volatile__(
			".code32				\n\t"
			"pushl		%%eax		\n\t"
			"pushl		%%ebx		\n\t"
			"pushl		%%ecx		\n\t"
			"pushl		%%edx		\n\t"
			"movl	%1,		%%eax	\n\t"
			"movl	%2,		%%ebx	\n\t"
			"movl	%0,		%%ecx	\n\t"
			"movl	(%%eax),	%%edx	\n\t"
			"addl	(%%ebx),	%%edx	\n\t"
			"movl	%%edx,		(%%ecx)	\n\t"
			"movl	4(%%eax),	%%edx	\n\t"
			"adcl	4(%%ebx),	%%edx	\n\t"
			"movl	%%edx,		4(%%ecx)\n\t"
			"popl	%%edx				\n\t"
			"popl	%%ecx				\n\t"
			"popl	%%ebx				\n\t"
			"popl	%%eax				\n\t"
			:"=r"(lpu64_result): "r"(lpu64_1), "r"(lpu64_2): "memory"
	);
#else
	__asm{
		push eax
        push ebx
        push ecx
        push edx
        mov eax,lpu64_1
        mov ebx,lpu64_2
        mov ecx,lpu64_result
        mov edx,dword ptr [eax]       //Load low part of first operand to edx.
        add edx,dword ptr [ebx]       //Add low part of two operands together.
        mov dword ptr [ecx],edx       //Save low part to result
        mov edx,dword ptr [eax + 4]   //Load high part of first operand to edx
        adc edx,dword ptr [ebx + 4]   //Add high part and carry bit together.
        mov dword ptr [ecx + 4],edx   //Save high part to result.
        pop edx
        pop ecx
        pop ebx
        pop eax
	}
#endif
#else
	  lpu64_result->dwLowPart = lpu64_1->dwLowPart + lpu64_2->dwLowPart;
	  if((lpu64_result->dwLowPart < lpu64_1->dwLowPart) || (lpu64_result->dwLowPart < lpu64_2->dwLowPart))
		{
			  lpu64_result->dwHighPart += 1;
		}
		lpu64_result->dwHighPart = lpu64_1->dwHighPart + lpu64_2->dwHighPart;
#endif
}

//
//The implementation of unsigned 64 bits integer subtraction.
//

VOID u64Sub(__U64* lpu64_1,__U64* lpu64_2,__U64* lpu64_result)
{
	if((NULL == lpu64_1) || (NULL == lpu64_2) || 
	   (NULL == lpu64_result)) //Parameters check.
	   return;

#ifdef __I386__
#ifdef __GCC__
	__asm__ __volatile__(
			".code32			\n\t"
			"pushl	%%eax		\n\t"
			"pushl	%%ebx		\n\t"
			"pushl	%%ecx		\n\t"
			"pushl	%%edx		\n\t"
			"movl	%1,		%%eax	\n\t"
			"movl	%2,		%%ebx	\n\t"
			"movl	%0,		%%ecx	\n\t"
			"movl	(%%eax),	%%edx	\n\t"
			"subl	(%%ebx),	%%edx	\n\t"
			"movl	%%edx,		(%%ecx)	\n\t"
			"movl	4(%%eax),	%%edx	\n\t"
			"sbbl	4(%%ebx),	%%edx	\n\t"
			"movl	%%edx,		4(%%ecx)\n\t"
			"popl	%%edx				\n\t"
			"popl	%%ecx				\n\t"
			"popl	%%ebx				\n\t"
			"popl	%%eax				\n\t"
			:"=r"(lpu64_result) :"r"(lpu64_1), "r"(lpu64_2): "memory"
			);
#else
	__asm{
        push eax
        push ebx
        push ecx
        push edx
        mov eax,lpu64_1
        mov ebx,lpu64_2
        mov ecx,lpu64_result
        mov edx,dword ptr [eax]
        sub edx,dword ptr [ebx]
        mov dword ptr [ecx],edx      //Save low part.
        mov edx,dword ptr [eax + 4]
        sbb edx,dword ptr [ebx + 4]
        mov dword ptr [ecx + 4],edx  //Save high part.
        pop edx
        pop ecx
        pop ebx
        pop eax
    }
#endif
#else
#endif
}

//
//The implementation of unsigned 64 bits integer's compare.
//

BOOL EqualTo(__U64* lpu64_1,__U64* lpu64_2)    //TRUE if lpu64_1 == lpu64_2.
{
	if((NULL == lpu64_1) || (NULL == lpu64_2)) //Parameters check.
		return FALSE;
	return (lpu64_1->dwHighPart == lpu64_2->dwHighPart) && 
		   (lpu64_1->dwLowPart  == lpu64_2->dwLowPart);
}

BOOL LessThan(__U64* lpu64_1,__U64* lpu64_2)   //TRUE if lpu64_1 < lpu64_2.
{
	if((NULL == lpu64_1) || (NULL == lpu64_2)) //Parameters check.
		return FALSE;
	
	if(lpu64_1->dwHighPart <  lpu64_2->dwHighPart)
		return TRUE;
	if((lpu64_1->dwHighPart == lpu64_2->dwHighPart) &&
	   (lpu64_1->dwLowPart  <  lpu64_2->dwLowPart))
	   return TRUE;
	return FALSE;
}


BOOL MoreThan(__U64* lpu64_1,__U64* lpu64_2)   //TRUE if lpu64_1 > lpu64_2.
{
	if((NULL == lpu64_1) || (NULL == lpu64_2)) //Parameters check.
		return FALSE;

	if(lpu64_1->dwHighPart > lpu64_2->dwHighPart)
		return TRUE;
	if((lpu64_1->dwHighPart == lpu64_2->dwHighPart) &&
	   (lpu64_1->dwLowPart  >  lpu64_2->dwLowPart))
	   return TRUE;
	return FALSE;
}

//
//The implementation of unsigned 64 bits integer's shift operations.
//

VOID u64RotateLeft(__U64* lpu64_1,DWORD dwTimes)   //Shift dwTimes bit(s) of lpu64_1 to left.
{
	if((NULL == lpu64_1) || (0 == dwTimes))        //Parameters check.
		return;

#ifdef __I386__
#ifdef __GCC__
__asm__ __volatile__(
			".code32			\n\t"
			"pushl	%%eax		\n\t"
			"pushl	%%ebx		\n\t"
			"pushl	%%ecx		\n\t"
			"movl	%0,		%%eax	\n\t"
			"movl	%1,		%%ecx	\n\t"
			"_2_BEGIN:				\n\t"
			"shll	$1,		(%%eax)	\n\t"
			"rcll	$1,		4(%%eax)\n\t"
			"loop	_2_BEGIN		\n\t"
			"popl	%%ecx			\n\t"
			"popl	%%ebx			\n\t"
			"popl	%%eax			\n\t"
			::"r"(lpu64_1),"r"(dwTimes):"memory");
#else
	__asm{
		push eax
        push ebx
        push ecx
        mov eax,lpu64_1
        mov ecx,dwTimes
__BEGIN:
        shl dword ptr [eax],1;      //Shift low part.
        rcl dword ptr [eax + 4],1;  //Shift high part,including carry bit.
        loop __BEGIN
        pop ecx
        pop ebx
        pop eax
	}
#endif

#else
#endif
}

VOID u64RotateRight(__U64* lpu64_1,DWORD dwTimes)  //Shift dwTimes bit(s) of lpu64_1 to right.
{
	if((NULL == lpu64_1) || (0 == dwTimes))  //Parameters check.
		return;
#ifdef __I386__
#ifdef __GCC__
	__asm__ __volatile__(
			".code32					\n\t"
			"pushl	%%eax				\n\t"
			"pushl	%%ecx				\n\t"
			"movl 	%0,	%%eax			\n\t"
			"movl	%1,	%%ecx			\n\t"
			"_0_BEGIN:					\n\t"
			"shrl 	$1, 4(%%eax)			\n\t"
			"rcrl	$1,	(%%eax)				\n\t"
			"loop	_0_BEGIN				\n\t"
			"popl	%%ecx					\n\t"
			"popl	%%eax					\n\t"
			::"r"(lpu64_1),"r"(dwTimes):"memory");
#else
	__asm{
		push eax
        push ecx
        mov eax,lpu64_1
        mov ecx,dwTimes
__BEGIN:
        shr dword ptr [eax + 4],1    //Shift high part first.
        rcr dword ptr [eax],1        //Shift left part then.
        loop __BEGIN
        pop ecx
        pop eax
	}
#endif
#else
#endif
}

//
//The implementation of unsigned 64 bit integer's divid.
// DX : AX = *lpu64_1, CX : BX = *lpu64_2, SI : DI = remainder.
// When finished, DX : AX is the result.
//
VOID u64Div(__U64* lpu64_1,__U64* lpu64_2,__U64* lpResult,__U64* lpRemainder)
{
#ifdef __I386__
#ifdef __GCC__
__asm__ __volatile__ (
			".code32						\n\t"
			"pushl					%%eax	\n\t"
			"pushl					%%ebx	\n\t"
			"pushl					%%ecx	\n\t"
			"pushl					%%edx	\n\t"
			"pushl					%%esi	\n\t"
			"pushl 					%%edi	\n\t"
			"pushl					%%ebp	\n\t"
			"movl 0x08(%%ebp),		%%esi	\n\t"
			"movl (%%esi),			%%eax	\n\t"
			"movl 0x04(%%esi),		%%edx	\n\t"
			"movl 0x0c(%%ebp),		%%esi	\n\t"
			"movl (%%esi),			%%ebx	\n\t"
			"movl 0x04(%%esi),		%%ecx	\n\t"
			"xorl %%esi,			%%esi	\n\t"
			"xorl %%edi,			%%edi	\n\t"
			"movl $64,				%%ebp	\n\t"
			"_1_BEGIN:						\n\t"
			"shll	$1,				%%eax	\n\t"
			"rcll	$1,				%%edx	\n\t"
			"rcll	$1,				%%edi	\n\t"
			"rcll	$1,				%%esi	\n\t"
			"cmpl	%%ecx,			%%esi	\n\t"
			"ja		__GOESINTO				\n\t"
			"ja		__TRYNEXT				\n\t"
			"cmpl 	%%ebx,			%%edi	\n\t"
			"jb __TRYNEXT					\n\t"
			"__GOESINTO:					\n\t"
			"subl 	%%ebx,			%%edi	\n\t"
			"sbbl 	%%ecx,			%%esi	\n\t"
			"incl 	%%eax					\n\t"
			"__TRYNEXT:						\n\t"
			"decl 	%%ebp					\n\t"
			"jne _1_BEGIN					\n\t"
			"popl %%ebp						\n\t"
			"movl 0x10(%%ebp),		%%ebx	\n\t"
			"movl %%eax,				(%%ebx)	\n\t"
			"movl %%edx,			0x04(%%ebx)	\n\t"
			"movl 0x14(%%ebp),		%%ebx		\n\t"
			"movl %%edi,			(%%ebx)		\n\t"
			"movl %%esi,			0x04(%%ebx)	\n\t"
			"popl %%edi		\n\t"
			"popl %%esi		\n\t"
			"popl %%edx		\n\t"
			"popl %%ecx		\n\t"
			"popl %%ebx		\n\t"
			"popl %%eax		\n\t"
			:::"memory");
#else
	__asm
	{
		push eax
		push ebx
		push ecx
		push edx
		push esi
		push edi
		push ebp
		mov esi,dword ptr [ebp + 0x08]
		mov eax,dword ptr [esi]
		mov edx,dword ptr [esi + 0x04]
		mov esi,dword ptr [ebp + 0x0C]
		mov ebx,dword ptr [esi]
		mov ecx,dword ptr [esi + 0x04]
		xor esi,esi
		xor edi,edi
		mov ebp,64
__BEGIN:
		shl eax,1
		rcl edx,1
		rcl edi,1
		rcl esi,1
		cmp esi,ecx
		ja __GOESINTO
		jb __TRYNEXT
		cmp edi,ebx
		jb __TRYNEXT
__GOESINTO:
		sub edi,ebx
		sbb esi,ecx
		inc eax
__TRYNEXT:
		dec ebp
		jne __BEGIN
		pop ebp
		mov ebx,dword ptr [ebp + 0x10]
		mov dword ptr [ebx],eax
		mov dword ptr [ebx + 0x04],edx
		mov ebx,dword ptr [ebp + 0x14]
		mov dword ptr [ebx],edi
		mov dword ptr [ebx + 0x04],esi
		pop edi
		pop esi
		pop edx
		pop ecx
		pop ebx
		pop eax
	}
#endif
#else
#endif
}

//
//A routine used to convert a 64 bits unsigned integer in hex format to a string.
//

//
//A helper routine.
//

/*static BOOL Hex2Str(DWORD dwSrc,LPSTR strBuffer)  //Convert the hex format to string.
{
	BOOL bResult = FALSE;
	BYTE bt = 0x00;
	
	if(NULL == strBuffer)        //Parameter check.
		return bResult;

	for(WORD i = 0;i < 8;i ++)
	{
		bt = LOBYTE(LOWORD(dwSrc));
		bt = bt & 0x0f;     //Get the low 4 bits.
		if(bt < 10)              //Should to convert to number.
		{
			bt += '0';
			strBuffer[7 - i] = bt;
		}
		else                     //Should to convert to character.
		{
			bt -= 10;
			bt += 'A';
			strBuffer[7 - i] = bt;
		}
		dwSrc = dwSrc >> 0x04;   //Continue to process the next 4 bits.
	}

	strBuffer[8] = 0x00;         //Add the string's terminal sign.
	return TRUE;
}*/

BOOL u64Hex2Str(__U64* lpu64,LPSTR lpszResult)
{
	if((NULL == lpu64) || (NULL == lpszResult))    //Parameters check.
		return FALSE;

	if(!Hex2Str(lpu64->dwHighPart,lpszResult))     //Convert the high part first.
		return FALSE;
	return Hex2Str(lpu64->dwLowPart,lpszResult + 8);  //Convert the low part then.
}
