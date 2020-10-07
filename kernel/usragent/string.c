//***********************************************************************/
//    Author                    : Garry
//    Original Date             : May,28 2004
//    Module Name               : string.cpp
//    Module Funciton           : 
//                                This module and string.h countains the
//                                string operation functions.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#include "hellox.h"
#include "string.h"

/*Purpose:
*       memmove() copies a source memory buffer to a destination memory buffer.
*       This routine recognize overlapping buffers to avoid propogation.
*       For cases where propogation is not a problem, memcpy() can be used.
*
*/

//string comparation code.
int strcmp(
	const char * src,
	const char * dst
)
{
	int ret = 0;
	while (!(ret = *(unsigned char *)src - *(unsigned char *)dst) && *dst)
		++src, ++dst;
	if (ret < 0)
		ret = -1;
	else if (ret > 0)
		ret = 1;
	return(ret);
}

int strlen(const char * s)
{
	int i;
	for (i = 0; s[i]; i++);
	return i;
}

char *strcpy(char * dst, const char * src)
{
	char * cp = dst;
	while (*cp++ = *src++);     /* Copy src over dst */
	return(dst);
}

char * strcat(
	char * dst,
	const char * src
)
{
	char * cp = dst;

	while (*cp)
		cp++;                   /* find end of dst */

	while (*cp++ = *src++);       /* Copy src to end of dst */

	return(dst);                  /* return dst */

}

void strtrim(char * dst, int flag)
{
	char* pos = dst;
	int   len = 0;
	int   i = 0;

	if (NULL == dst)
	{
		return;
	}

	len = strlen(dst);
	if (len <= 0)
	{
		return;
	}
	if (flag&TRIM_LEFT)
	{
		while (i < len)
		{
			if (*pos != 0x20)
			{
				break;
			}

			pos++;
			i++;
		}

		if (len == i)
		{
			dst[0] = 0;
			return;
		}

		if (i > 0)
		{
			len -= i;
			memcpy(dst, pos, len);
			dst[len] = 0;
		}
	}

	if (flag&TRIM_RIGHT)
	{
		for (i = len - 1; i >= 0; i--)
		{
			if (dst[i] == 0x20)
			{
				dst[i] = 0;
			}
			else
			{
				break;
			}
		}
	}

}

/* String copy,array bound is guaranteed. */
char* strncpy(char *dst, const char *src, size_t n)
{
	char *d;

	if (!dst || !src)
		return (dst);
	d = dst;
	for (; *src && n; d++, src++, n--)
		*d = *src;
	while (n--)
		*d++ = '\0';
	return (dst);
}

/* String comparation,array bound is guaranteed. */
int strncmp(const char *s1, const char *s2, size_t n)
{
	if (!s1 || !s2)
		return (0);

	while (n && (*s1 == *s2)) {
		if (*s1 == 0)
			return (0);
		s1++;
		s2++;
		n--;
	}
	if (n)
		return (*s1 - *s2);
	return (0);
}

//Find the first bit in a given integer.
int ffs(int x)
{
	int r = 1;
	if (!x)
	{
		return 0;
	}
	if (!(x & 0xFFFF))
	{
		x >>= 16;
		r += 16;
	}
	if (!(x & 0xFF))
	{
		x >>= 8;
		r += 8;
	}
	if (!(x & 0x0F))
	{
		x >>= 4;
		r += 4;
	}
	if (!(x & 3))
	{
		x >>= 2;
		r += 2;
	}
	if (!(x & 1))
	{
		x >>= 1;
		r += 1;
	}
	return r;
}

#ifndef isdigit
#define isdigit(ch) ((ch >= '0') && (ch <= '9'))
#endif

#ifndef isalpha
#define isalpha(ch) (((ch >= 'a') && (ch <= 'z')) || ((ch >= 'A') && (ch <= 'Z')))
#endif

#ifndef isupper
#define isupper(ch) ((ch >= 'A') && (ch <= 'Z'))
#endif

char * strrchr(const char * str, int ch)

{
	char *p = (char *)str;
	while (*str) str++;
	while (str-- != p && *str != (char)ch);
	if (*str == (char)ch)
	{
		return((char *)str);
	}
	return(NULL);
}

char * strstr(const char *s1, const char *s2)
{
	if (*s1 == 0)
	{
		if (*s2)
		{
			return (char *)NULL;
		}
		return (char *)s1;
	}
	while (*s1)
	{
		size_t i;
		i = 0;
		while (1)
		{
			if (s2[i] == 0)
			{
				return (char *)s1;
			}
			if (s2[i] != s1[i])
			{
				break;
			}
			i++;
		}
		s1++;
	}
	return (char *)NULL;
}

//static int isdigit(int x)
//{
//    if(x<='9'&&x>='0')
//        return 1;
//    else
//        return 0;
//}

#ifdef __GCC__
long atol(const char *nptr)
{
	int c; /* current char */
	long total; /* current total */
	int sign; /* if ''-'', then negative, otherwise positive */

	/* skip whitespace */
	while (isspace((int)(unsigned char)*nptr))
	{
		++nptr;
	}
	c = (int)(unsigned char)*nptr++;
	sign = c; /* save sign indication */
	if (c == '-' || c == '+')
	{
		c = (int)(unsigned char)*nptr++; /* skip sign */
	}
	total = 0;

	while (isdigit(c)) {
		total = 10 * total + (c - '0'); /* accumulate digit */
		c = (int)(unsigned char)*nptr++; /* get next char */
	}

	if (sign == '-')
		return -total;
	else
		return total; /* return result, negated if necessary */
}

/***
*int atoi(char *nptr) - Convert string to long
*
*Purpose:
* Converts ASCII string pointed to by nptr to binary.
* Overflow is not detected. Because of this, we can just use
* atol().
*
*Entry:
* nptr = ptr to string to convert
*
*Exit:
* return int value of the string
*
*Exceptions:
* None - overflow is not detected.
*
*******************************************************************************/

int atoi(const char *nptr)
{
	return (int)atol(nptr);
}

char* itoa(int value, char* string, int radix)
{
	char tmp[33];
	char* tp = tmp;
	int i;
	unsigned v;
	int sign;
	char* sp;

	if (radix > 36 || radix <= 1)
	{
		//__set_errno(EDOM);
		return 0;
	}
	sign = (radix == 10 && value < 0);
	if (sign)
		v = -value;
	else
		v = (unsigned)value;
	while (v || tp == tmp)
	{
		i = v % radix;
		v = v / radix;
		if (i < 10)
			*tp++ = i + '0';
		else
			*tp++ = i + 'a' - 10;
	}

	if (string == 0)
		string = (char*)_hx_malloc((tp - tmp) + sign + 1);
	sp = string;

	if (sign)
		*sp++ = '-';
	while (tp > tmp)
		*sp++ = *--tp;
	*sp = 0;
	return string;
}

#endif

char*  strtok(char* string_org, const char* demial)
{
	static unsigned char* last;
	unsigned char* str;
	const unsigned char* ctrl = (const unsigned char*)demial;
	unsigned char map[32];
	int count;

	for (count = 0; count < 32; count++) {
		map[count] = 0;
	}
	do {
		map[*ctrl >> 3] |= (1 << (*ctrl & 7));
	} while (*ctrl++);
	if (string_org) {
		str = (unsigned char*)string_org;
	}
	else {
		str = last;
	}
	while ((map[*str >> 3] & (1 << (*str & 7))) && *str) {
		str++;
	}
	string_org = (char*)str;
	for (; *str; str++) {
		if (map[*str >> 3] & (1 << (*str & 7))) {
			*str++ = '\0';
			break;
		}
	}
	last = str;
	if (string_org == (char*)str) {
		return NULL;
	}
	else {
		return string_org;
	}
}
