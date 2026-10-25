//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Jul,12 2024
//    Module Name               : wchar.c
//    Module Funciton           : 
//                                Wide string operations.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#include "hellox.h"
#include "stddef.h"
#include "limits.h"
#include "ctype.h"
#include "stdlib.h"
#include "wchar.h"

/* wstring comparation. */
int wcscmp(
	const wchar_t* src,
	const wchar_t* dst
)
{
	int ret = 0;
	while (!(ret = *(wchar_t *)src - *(wchar_t *)dst) && *dst)
		++src, ++dst;
	if (ret < 0)
		ret = -1;
	else if (ret > 0)
		ret = 1;
	return(ret);
}

size_t wcslen(const wchar_t * s)
{
	int i;
	for (i = 0; s[i]; i++);
	return i;
}

/* wstring copy,array bound is guaranteed. */
wchar_t* wcsncpy(wchar_t *dest, const wchar_t *src, size_t n)
{
	wchar_t *strRtn = dest;
	while (n && (*dest++ = *src++))
	{
		n--;
	}
	if (n) {
		while (--n)
			*dest++;  //There may be bug...
	}
	return strRtn;
}

/* wstring comparation, array bound guaranteed. */
int wcsncmp(const wchar_t * s1, const wchar_t * s2, size_t n)
{
	if (!n)
		return(0);

	while (--n && *s1 && *s1 == *s2)
	{
		s1++;
		s2++;
	}
	return(*s1 - *s2);
}
