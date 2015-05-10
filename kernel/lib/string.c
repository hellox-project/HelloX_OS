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

#include <stdafx.h>
#include <string.h>
#include <limits.h>

/*Purpose:
*       memmove() copies a source memory buffer to a destination memory buffer.
*       This routine recognize overlapping buffers to avoid propogation.
*       For cases where propogation is not a problem, memcpy() can be used.
*
*/

//
//String operation functions implementation.
//
BOOL StrCmp(LPSTR strSrc,LPSTR strDes)  //Compare the two strings,if equal,returns
                                        //TRUE,otherwise,returns FALSE.
{
	//BOOL bResult = FALSE;
	WORD wIndex = 0x0000;

	if((NULL == strSrc) || (NULL == strDes))  //Parameter check.
	{
		return FALSE;
	}

	while(strSrc[wIndex] && strDes[wIndex] && (strSrc[wIndex] == strDes[wIndex]))
	{
		wIndex ++;
	}

	return strSrc[wIndex] == strDes[wIndex] ? TRUE : FALSE;
}

WORD StrLen(LPSTR strSrc)        //Get the string's length.
                                 //If the string's lenght is less than 
								 //MAX_STRING_LEN,returns the actual string's
								 //length,otherwise,returns MAX_STRING_LEN.
{
	WORD wStrLen = 0x00;

	if(NULL == strSrc)
	{
		return -1;
	}

	while(strSrc[wStrLen] && (MAX_STRING_LEN > wStrLen))
		wStrLen ++;

	return wStrLen;
}

BOOL Hex2Str(DWORD dwSrc,LPSTR strBuffer)  //Convert the hex format to string.
{
	BOOL bResult = FALSE;
	BYTE bt = 0x00;
	int  i;
	
	if(NULL == strBuffer)        //Parameter check.
		return bResult;

	for(i = 0;i < 8;i ++)
	{
		bt = (BYTE)dwSrc;   //LOBYTE(LOWORD(dwSrc));
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
}

//
//Convert 32 bit int number to string.
//
BOOL Int2Str(DWORD dwNum,LPSTR pszResult)
{
	BOOL bResult = FALSE;
	BYTE bt;
	BYTE index = 0;
	BYTE sw;

	if(NULL == pszResult)
		return bResult;

	do{
		bt =  (BYTE)(dwNum % 10);
		bt += '0';
		pszResult[index++] = bt;
		dwNum /= 10;
	}while(dwNum);
	pszResult[index] = 0;        //Set the terminal sign.
	
	for(bt = 0;bt < index/2;bt ++)  //Inverse the string.
	{
		sw = pszResult[bt];
		pszResult[bt] = pszResult[index - bt -1];
		pszResult[index - bt - 1] = sw;
	}

	bResult = TRUE;
	return bResult;
}

//
//Print a string at a new line.
//
VOID PrintLine(LPSTR pszStr)
{
	CD_PrintString(pszStr,FALSE);
	GotoHome();
	ChangeLine();
}

//
//Copy the first string, to the second string buffer.
//
VOID StrCpy(LPSTR strSrc,LPSTR strDes)
{
	DWORD dwIndex = 0;

	if((NULL == strSrc) || (NULL == strDes))  //Parameter check.
	{
		return;
	}
	
	while(strSrc[dwIndex])
	{
		strDes[dwIndex] = strSrc[dwIndex];
		dwIndex ++;
	}
	strDes[dwIndex] = 0;
}

//
//Convert the string's low character to uper character.
//Such as,the input string is "abcdefg",then,the output
//string would be "ABCDEFG".
//

VOID ConvertToUper(LPSTR pszSource)
{
	BYTE     bt        = 'a' - 'A';
	//DWORD    dwIndex   = 0x0000;
	DWORD    dwMaxLen  = MAX_STRING_LEN;

	if(NULL == pszSource)
	{
		return;
	}

	while(*pszSource)
	{
		if((*pszSource >= 'a') && ( *pszSource <= 'z'))
		{
			*pszSource -= bt;
		}
		pszSource ++;
		dwMaxLen --;
		if(0 == dwMaxLen)
			break;
	}

	return;
}

//
//Convert the string to hex number.
//If success,it returns TRUE,else,returns FALSE.
//
BOOL Str2Hex(LPSTR pszSrc,DWORD* pdwResult)
{
	BOOL     bResult  = FALSE;
	DWORD    dwResult = 0x00000000;
	if((NULL == pszSrc) || (NULL == pdwResult))  //Parameters check.
		return bResult;

	if(StrLen(pszSrc) > 8)                      //If the string's length is longer
		                                        //than the max hex number length.
		return bResult;

	ConvertToUper(pszSrc);                      //Convert to uper character.

	while(*pszSrc)
	{
		dwResult <<= 4;
		switch(*pszSrc)
		{
		case '0':
			dwResult += 0;
			break;
		case '1':
			dwResult += 1;
			break;
		case '2':
			dwResult += 2;
			break;
		case '3':
			dwResult += 3;
			break;
		case '4':
			dwResult += 4;
			break;
		case '5':
			dwResult += 5;
			break;
		case '6':
			dwResult += 6;
			break;
		case '7':
			dwResult += 7;
			break;
		case '8':
			dwResult += 8;
			break;
		case '9':
			dwResult += 9;
			break;
		case 'A':
			dwResult += 10;
			break;
		case 'B':
			dwResult += 11;
			break;
		case 'C':
			dwResult += 12;
			break;
		case 'D':
			dwResult += 13;
			break;
		case 'E':
			dwResult += 14;
			break;
		case 'F':
			dwResult += 15;
			break;
		default:
			bResult = FALSE;
			return bResult;
		}
		pszSrc ++;
	}

	bResult    = TRUE;
	*pdwResult = dwResult;

	return bResult;
}

//
//The implementation of FormString routine.
//This routine formats a string,and copy it into a buffer.
//It's function likes sprintf.
//
INT FormString(LPSTR lpszBuff,LPSTR lpszFmt,LPVOID* lppParam)
{
	DWORD        dwIndex        = 0;
	LPSTR        lpszTmp        = NULL;
	CHAR         Buff[12];

	if((NULL == lpszBuff) || (NULL == lpszBuff))
		return -1;

	lpszTmp = lpszBuff;
	while(*lpszFmt)
	{
		if('%' == *lpszFmt)    //Should process.
		{
			lpszFmt ++;        //Skip '%'.
			switch(*lpszFmt)
			{
			case 'd':    //Convert an integer to string.
				Int2Str(*((DWORD*)lppParam[dwIndex ++]),Buff);  //Convert to string.
				StrCpy(Buff,lpszTmp);
				lpszTmp += StrLen(Buff);
				lpszFmt ++;
				break;
			case 'c':    //Convert a character to string.
				*lpszTmp ++= *((BYTE*)lppParam[dwIndex ++]);
				lpszFmt ++;
				break;
			case 's':    //Append a string.
				StrCpy((LPSTR)lppParam[dwIndex],lpszTmp);
				lpszTmp += StrLen((LPSTR)lppParam[dwIndex ++]);
				lpszFmt ++;
				break;
			case 'x':    //Convert an integer to string in hex.
			case 'X':
				Hex2Str(*((DWORD*)lppParam[dwIndex ++]),Buff);  //Convert to string.
				StrCpy(Buff,lpszTmp);
				lpszTmp += StrLen(Buff);
				lpszFmt ++;
				break;
			default:     //Unsupported now.
				break;
			}
		}
		*lpszTmp = *lpszFmt;
		if(0 == *lpszTmp)    //Reach end.
			break;
		lpszTmp ++;
		lpszFmt ++;
	}

	*lpszTmp = 0;    //End sign.
	return (lpszTmp - lpszBuff);
}

//A helper routine used to convert a string from lowercase to capital.
//The string should be terminated by a zero,i.e,a C string.
VOID ToCapital(LPSTR lpszString)
{
	int nIndex = 0;

	if(NULL == lpszString)
	{
		return;
	}
	while(lpszString[nIndex++])
	{
		if((lpszString[nIndex] >= 'a') && (lpszString[nIndex] <= 'z'))
		{
			lpszString[nIndex] += 'A' - 'a';
		}
	}
}

//string comparation code.
int strcmp (
        const char * src,
        const char * dst
        )
{
        int ret = 0 ;
        while( ! (ret = *(unsigned char *)src - *(unsigned char *)dst) && *dst)
                ++src, ++dst;  
        if ( ret < 0 )
                ret = -1 ;
        else if ( ret > 0 )
                ret = 1 ;
        return( ret );
}

int strlen(const char * s)
{
   int i;
   for (i = 0; s[i]; i++) ;
   return i;
}

char *strcpy(char * dst, const char * src)
{
    char * cp = dst;
    while( *cp++ = *src++ )
            ;               /* Copy src over dst */
    return( dst );
}

char * strcat (
        char * dst,
        const char * src
        )
{
        char * cp = dst;
 
        while( *cp )
                cp++;                   /* find end of dst */
 
        while( *cp++ = *src++ ) ;       /* Copy src to end of dst */
 
        return( dst );                  /* return dst */
 
}

void strtrim(char * dst,int flag)
{
	char* pos   = dst;
	int   len   = 0;
	int   i     = 0;
	
	if(NULL == dst)
	{
		return;
	}

	len = strlen(dst);
	if(len <= 0)
	{
		return; 
	}
	if(flag&TRIM_LEFT)
	{
		while(i < len)
		{
			if(*pos != 0x20) 
			{
				break;
			}

			pos ++;
			i   ++;
		}

		//全是空格
		if(len == i)
		{
			dst[0] = 0;
			return;
		}

		if(i > 0) 
		{
			len -=  i;
			memcpy(dst,pos,len);
			dst[len] = 0;
		}
	}
	
	if(flag&TRIM_RIGHT)
	{
		for(i = len-1; i >= 0;i--)
		{
			if(dst[i] == 0x20)
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

//String copy,array bound is guaranteed.
char* strncpy(char *dest,char *src,unsigned int n)
{
	char *strRtn=dest;
	while(n && (*dest++=*src++))
	{
		n--;
	} 
	if(n){
		while(--n)
			*dest++;  //There may be bug...
	}  
    return strRtn;  
} 

//String comparation,array bound is guaranteed.
int strncmp ( char * s1, char * s2, size_t n)
{
  if ( !n )
   return(0);

  while (--n && *s1 && *s1 == *s2)
  {
     s1++;
     s2++;
  }
  return( *s1 - *s2 );
}

//Find the first bit in a given integer.
int ffs(int x)
{
	int r = 1;
	if(!x)
	{
		return 0;
	}
	if(!(x & 0xFFFF))
	{
		x >>= 16;
		r += 16;
	}
	if(!(x & 0xFF))
	{
		x >>= 8;
		r += 8;
	}
	if(!(x & 0x0F))
	{
		x >>= 4;
		r += 4;
	}
	if(!(x & 3))
	{
		x >>= 2;
		r += 2;
	}
	if(!(x & 1))
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

//String to long.
int strtol(const char *nptr, char **endptr, int base)
{
   const char *p = nptr;
   unsigned long ret;
   int ch;
   unsigned long Overflow;
   int sign = 0, flag, LimitRemainder;
  
   /*
      跳过前面多余的空格，并判断正负符号。
      如果base是0，允许以0x开头的十六进制数，
      以0开头的8进制数。
      如果base是16，则同时也允许以0x开头。
 
   */
   do
   {
      ch = *p++;
   } while (' ' == ch);
  
   if (ch == '-')
   {
      sign = 1;
      ch = *p++;
   }
   else if (ch == '+')
      ch = *p++;
   if ((base == 0 || base == 16) &&
      ch == '0' && (*p == 'x' || *p == 'X'))
   {
      ch = p[1];
      p += 2;
      base = 16;
   }
   if (base == 0)
      base = ch == '0' ? 8 : 10;
 
   Overflow = sign ? -(unsigned long)LONG_MIN : LONG_MAX;
   LimitRemainder = Overflow % (unsigned long)base;
   Overflow /= (unsigned long)base;
 
   for (ret = 0, flag = 0;; ch = *p++)
   {
      /*把当前字符转换为相应运算中需要的值。*/
      if (isdigit(ch))
        ch -= '0';
      else if (isalpha(ch))
        ch -= isupper(ch) ? 'A' - 10 : 'a' - 10;
      else
        break;
      if (ch >= base)
        break;
 
      /*如果产生溢出，则置标志位，以后不再做计算。*/
      if (flag < 0 || ret > Overflow || (ret == Overflow && ch > LimitRemainder))
        flag = -1;
      else
      {
        flag = 1;
        ret *= base;
        ret += ch;
      }
   }
 
   /*
      如果溢出，则返回相应的Overflow的峰值。
      没有溢出，如是符号位为负，则转换为负数。
   */
   if (flag < 0)
      ret = sign ? LONG_MIN : LONG_MAX;
   else if (sign)
      ret = -ret;
  
   /*
      如字符串不为空，则*endptr等于指向nptr结束
      符的指针值；否则*endptr等于nptr的首地址。
   */
   if (endptr != 0)
      *endptr = (char *)(flag ?(p - 1) : nptr);
 
   return ret;
}

char * strrchr(const char * str,int ch)

{
	char *p = (char *)str;
	while (*str) str++;
	while (str-- != p && *str != (char)ch);
	if (*str == (char)ch)
	{
		return( (char *)str );
	}
	return(NULL);
}

char * strstr(const char *s1,const char *s2)
{
	if (*s1 == 0)
	{
		if (*s2)
		{
			return (char *) NULL;
		}
		return (char *) s1;
	}
	while (*s1)
	{
		size_t i;
		i = 0;
		while (1)
		{
			if (s2[i] == 0)
			{
				return (char *) s1;
			}
			if (s2[i] != s1[i])
			{
				break;
			}
			i++;
		}
		s1++;
	}
	return (char *) NULL;
}
