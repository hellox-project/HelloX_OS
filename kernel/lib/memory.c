
#include "StdAfx.h"
#include "string.h"

/*
 * Memory manipulating functions,memcpy,
 * memset, memmove and memcmp...
 */

void* __memcpy (void * dst,const void * src,size_t count)
{
	void * ret = dst;

	//copy from lower addresses to higher addresses
	while (count--) 
	{
		*(char *)dst = *(char *)src;
		dst = (char *)dst + 1;
		src = (char *)src + 1;
	}

	return(ret);
}

/* 
 * Memory copy,more optimal than just copy 
 * bytes one by one. 
 */
void* memcpy(void* dst, const void* src, size_t len)
{
	size_t i;

	/* 
	 * Apply machine alignment copy if all
	 * conditions satisfied.
	 */
	if ((uintptr_t)dst % sizeof(long) == 0 &&
		(uintptr_t)src % sizeof(long) == 0 &&
		len % sizeof(long) == 0)
	{
		long* d = dst;
		const long* s = src;
		for (i = 0; i < len / sizeof(long); i++)
		{
			d[i] = s[i];
		}
	}
	else
	{
		if ((uintptr_t)dst % sizeof(short) == 0 &&
			(uintptr_t)src % sizeof(short) == 0 &&
			len % sizeof(short) == 0)
		{
			/* Word alignment,copy one word each time. */
			short* d = dst;
			const short* s = src;
			for (i = 0; i < len / sizeof(short); i++)
			{
				d[i] = s[i];
			}
		}
		else
		{
			/* Use byte copy. */
			char* d = dst;
			const char* s = src;
			for (i = 0; i < len; i++)
			{
				d[i] = s[i];
			}
		}
	}
	return dst;
}

void* memset (void *dst,int val,size_t count)
{
	void *start = dst;

	while (count--) 
	{
		*(char *)dst = (char)val;
		dst = (char *)dst + 1;
	}

	return(start);
}

void* memzero(	void* dst,	size_t count)
{
	return memset(dst,0,count);
}

void* memchr (const void * buf,int chr,size_t cnt)
{
	while ( cnt && (*(unsigned char *)buf != (unsigned char)chr) ) 
	{
		buf = (unsigned char *)buf + 1;
		cnt--;
	}

	return(cnt ? (void *)buf : NULL);
}

int memcmp(const void *buffer1,const void *buffer2,int count)
{
	if (!count) return(0);

	while ( --count && *(char *)buffer1 == *(char *)buffer2)
	{
		buffer1 = (char *)buffer1 + 1;
		buffer2 = (char *)buffer2 + 1;
	}

	return( *((unsigned char *)buffer1) - *((unsigned char *)buffer2) );
}
 
/* Overlap between dst and src could be supported. */
void *memmove(void *dst,const void *src,int n)
{
     char *dp = (char *)dst;
     char *sp = (char *)src; 
     
	 if((NULL == dst) || (NULL == src) || (n <= 0))
	 {
		 return NULL;
	 }

     //Not overlaped.
     if(sp>dp||(sp+n)<dp)
     { 
         while(n--) 
             *(dp++) = *(sp++);
         *dp = '\0';
     }
     else if(sp<dp) //Overlaped.
     {
		 sp += n; 
         dp += n; 
         *dp = '\0'; 
         while(n--)
            *(--dp) = *(--sp); 
     }
     return dst;
}
