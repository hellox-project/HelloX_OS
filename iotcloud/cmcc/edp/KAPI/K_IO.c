//***********************************************************************/
//    Author                    : twind
//    Original Date             : oct,21 2015
//    Module Name               : k_socket.CPP
//    Module Funciton           : 
//                                All socket in kernel module are wrapped
//                                in this file.
//
//    Lines number              :
//***********************************************************************/

#include "K_IO.H"

//#pragma comment(lib,"bufferoverflow.lib")
#define  NOFLOAT


HANDLE CreateFile(LPSTR lpszFileName,
	DWORD dwAccessMode,
	DWORD dwShareMode,
	LPVOID lpReserved)
{
	SYSCALL_PARAM_4(SYSCALL_CREATEFILE,
		lpszFileName,
		dwAccessMode,
		dwShareMode,
		lpReserved);
}

BOOL ReadFile(HANDLE hFile,
	DWORD dwReadSize,
	LPVOID lpBuffer,
	DWORD* lpdwReadSize)
{
	SYSCALL_PARAM_4(SYSCALL_READFILE,
		hFile,
		dwReadSize,
		lpBuffer,
		lpdwReadSize);
}

BOOL WriteFile(HANDLE hFile,
	DWORD dwWriteSize,
	LPVOID lpBuffer,
	DWORD* lpdwWrittenSize)
{
	SYSCALL_PARAM_4(SYSCALL_WRITEFILE,
		hFile,
		dwWriteSize,
		lpBuffer,
		lpdwWrittenSize);
}

VOID CloseFile(HANDLE hFile)
{
	SYSCALL_PARAM_1(SYSCALL_CLOSEFILE,
		hFile);
}

BOOL CreateDirectory(LPSTR lpszDirName)
{
	SYSCALL_PARAM_1(SYSCALL_CREATEDIRECTORY,lpszDirName);
}

BOOL DeleteFile(LPSTR lpszFileName)
{
	SYSCALL_PARAM_1(SYSCALL_DELETEFILE,lpszFileName);
}

HANDLE FindFirstFile(LPSTR lpszDirName,
	FS_FIND_DATA* pFindData)
{
	SYSCALL_PARAM_2(SYSCALL_FINDFIRSTFILE,
		lpszDirName,
		pFindData);
}

BOOL FindNextFile(LPSTR lpszDirName,
	HANDLE hFindHandle,
	FS_FIND_DATA* pFindData)
{
	SYSCALL_PARAM_3(SYSCALL_FINDNEXTFILE,lpszDirName,
		hFindHandle,
		pFindData);
}

VOID FindClose(LPSTR lpszDirName,
	HANDLE hFindHandle)
{
	SYSCALL_PARAM_2(SYSCALL_FINDCLOSE,
		lpszDirName,
		hFindHandle);
}

DWORD GetFileAttributes(LPSTR lpszFileName)
{
	SYSCALL_PARAM_1(SYSCALL_GETFILEATTRIBUTES,lpszFileName);
}

DWORD GetFileSize(HANDLE hFile,DWORD* lpdwSizeHigh)
{
	SYSCALL_PARAM_2(SYSCALL_GETFILESIZE,hFile,
		lpdwSizeHigh);
}

BOOL RemoveDirectory(LPSTR lpszDirName)
{
	SYSCALL_PARAM_1(SYSCALL_REMOVEDIRECTORY,
		lpszDirName);
}

BOOL SetEndOfFile(HANDLE hFile)
{
	SYSCALL_PARAM_1(SYSCALL_SETENDOFFILE,hFile);
}

BOOL IOControl(HANDLE hFile,
	DWORD dwCommand,
	DWORD dwInputLen,
	LPVOID lpInputBuffer,
	DWORD dwOutputLen,
	LPVOID lpOutputBuffer,
	DWORD* lpdwFilled)
{
	SYSCALL_PARAM_7(SYSCALL_IOCONTROL,hFile,
		dwCommand,dwInputLen,
		lpInputBuffer,
		dwOutputLen,lpOutputBuffer,
		lpdwFilled);
}

BOOL SetFilePointer(HANDLE hFile,
	DWORD* lpdwDistLow,
	DWORD* lpdwDistHigh,
	DWORD dwMoveFlags)
{
	SYSCALL_PARAM_4(SYSCALL_SETFILEPOINTER,hFile,
		lpdwDistLow,
		lpdwDistHigh,
		dwMoveFlags);
}

BOOL FlushFileBuffers(HANDLE hFile)
{
	SYSCALL_PARAM_1(SYSCALL_FLUSHFILEBUFFERS,
		hFile);
}

HANDLE CreateDevice(LPSTR lpszDevName,
	DWORD dwAttributes,
	DWORD dwBlockSize,
	DWORD dwMaxReadSize,
	DWORD dwMaxWriteSize,
	LPVOID lpDevExtension,
	HANDLE hDrvObject)
{
	SYSCALL_PARAM_7(SYSCALL_CREATEDEVICE,
		lpszDevName,
		dwAttributes,
		dwBlockSize,
		dwMaxReadSize,
		dwMaxWriteSize,
		lpDevExtension,
		hDrvObject);
}

VOID DestroyDevice(HANDLE hDevice)
{
	SYSCALL_PARAM_1(SYSCALL_DESTROYDEVICE,hDevice);
}

VOID PrintLine(LPSTR lpszInfo)
{
	SYSCALL_PARAM_1(SYSCALL_PRINTLINE,lpszInfo);
}

VOID PrintChar(WORD ch)
{
	DWORD dwCh = (DWORD)ch;

	SYSCALL_PARAM_1(SYSCALL_PRINTCHAR,dwCh);
}


typedef char *  va_list;
typedef unsigned int     size_t;

#define _INTSIZEOF(n) ( (sizeof(n) + sizeof(int) - 1) & ~(sizeof(int) - 1) )

#define va_start(ap,v)  ( ap = (va_list)&v + _INTSIZEOF(v) )
#define va_arg(ap,t)    ( *(t *)((ap += _INTSIZEOF(t)) - _INTSIZEOF(t)) )
#define va_end(ap)      ( ap = (va_list)0 )

#define is_digit(c) ((c) >= '0' && (c) <= '9')

static char *digits       = "0123456789abcdefghijklmnopqrstuvwxyz";
static char *upper_digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";


#define ZEROPAD 1               // Pad with zero
#define SIGN    2               // Unsigned/signed long
#define PLUS    4               // Show plus
#define SPACE   8               // Space if plus
#define LEFT    16              // Left justified
#define SPECIAL 32              // 0x
#define LARGE   64              // Use 'ABCDEF' instead of 'abcdef'


static int skip_atoi(const char **s)
{
	int i = 0;
	while (is_digit(**s)) i = i*10 + *((*s)++) - '0';
	return i;
}

static char *number(char *str, long num, int base, int size, int precision, int type)
{
	char c, sign, tmp[66];
	char *dig = digits;
	int i;

	if (type & LARGE)  dig = upper_digits;
	if (type & LEFT) type &= ~ZEROPAD;
	if (base < 2 || base > 36) return 0;

	c = (type & ZEROPAD) ? '0' : ' ';
	sign = 0;
	if (type & SIGN)
	{
		if (num < 0)
		{
			sign = '-';
			num = -num;
			size--;
		}
		else if (type & PLUS)
		{
			sign = '+';
			size--;
		}
		else if (type & SPACE)
		{
			sign = ' ';
			size--;
		}
	}

	if (type & SPECIAL)
	{
		if (base == 16)
			size -= 2;
		else if (base == 8)
			size--;
	}

	i = 0;

	if (num == 0)
		tmp[i++] = '0';
	else
	{
		while (num != 0)
		{
			tmp[i++] = dig[((unsigned long) num) % (unsigned) base];
			num = ((unsigned long) num) / (unsigned) base;
		}
	}

	if (i > precision) precision = i;
	size -= precision;
	if (!(type & (ZEROPAD | LEFT))) while (size-- > 0) *str++ = ' ';
	if (sign) *str++ = sign;

	if (type & SPECIAL)
	{
		if (base == 8)
			*str++ = '0';
		else if (base == 16)
		{
			*str++ = '0';
			*str++ = digits[33];
		}
	}

	if (!(type & LEFT)) while (size-- > 0) *str++ = c;
	while (i < precision--) *str++ = '0';
	while (i-- > 0) *str++ = tmp[i];
	while (size-- > 0) *str++ = ' ';

	return str;
}

static char *eaddr(char *str, unsigned char *addr, int size, int precision, int type)
{
	char tmp[24];
	char *dig = digits;
	int i, len;

	if (type & LARGE)  dig = upper_digits;
	len = 0;
	for (i = 0; i < 6; i++)
	{
		if (i != 0) tmp[len++] = ':';
		tmp[len++] = dig[addr[i] >> 4];
		tmp[len++] = dig[addr[i] & 0x0F];
	}

	if (!(type & LEFT)) while (len < size--) *str++ = ' ';
	for (i = 0; i < len; ++i) *str++ = tmp[i];
	while (len < size--) *str++ = ' ';

	return str;
}

static char *iaddr(char *str, unsigned char *addr, int size, int precision, int type)
{
	char tmp[24];
	int i, n, len;

	len = 0;
	for (i = 0; i < 4; i++)
	{
		if (i != 0) tmp[len++] = '.';
		n = addr[i];

		if (n == 0)
			tmp[len++] = digits[0];
		else
		{
			if (n >= 100) 
			{
				tmp[len++] = digits[n / 100];
				n = n % 100;
				tmp[len++] = digits[n / 10];
				n = n % 10;
			}
			else if (n >= 10) 
			{
				tmp[len++] = digits[n / 10];
				n = n % 10;
			}

			tmp[len++] = digits[n];
		}
	}

	if (!(type & LEFT)) while (len < size--) *str++ = ' ';
	for (i = 0; i < len; ++i) *str++ = tmp[i];
	while (len < size--) *str++ = ' ';

	return str;
}

size_t  mystrnlen(const char *str, size_t maxsize)
{
    size_t n;

    
    for (n = 0; n < maxsize && *str; n++, str++)
        ;

    return n;
}


int _hx_vsprintf(char *buf, const char *fmt, va_list args)
{
	
	int len;
	unsigned long num;
	int i, base;
	char *str;
	char *s;

	int flags;            // Flags to number()

	int field_width;      // Width of output field
	int precision;        // Min. # of digits for integers; max number of chars for from string
	int qualifier;        // 'h', 'l', or 'L' for integer fields

	for (str = buf; *fmt; fmt++)
	{
		if (*fmt != '%')
		{
			*str++ = *fmt;
			continue;
		}

		// Process flags
		flags = 0;
repeat:
		fmt++; // This also skips first '%'
		switch (*fmt)
		{
		case '-': flags |= LEFT; goto repeat;
		case '+': flags |= PLUS; goto repeat;
		case ' ': flags |= SPACE; goto repeat;
		case '#': flags |= SPECIAL; goto repeat;
		case '0': flags |= ZEROPAD; goto repeat;
		}

		// Get field width
		field_width = -1;
		if (is_digit(*fmt))
			field_width = skip_atoi(&fmt);
		else if (*fmt == '*')
		{
			fmt++;
			field_width = va_arg(args, int);
			if (field_width < 0)
			{
				field_width = -field_width;
				flags |= LEFT;
			}
		}

		// Get the precision
		precision = -1;
		if (*fmt == '.')
		{
			++fmt;    
			if (is_digit(*fmt))
				precision = skip_atoi(&fmt);
			else if (*fmt == '*')
			{
				++fmt;
				precision = va_arg(args, int);
			}
			if (precision < 0) precision = 0;
		}

		// Get the conversion qualifier
		qualifier = -1;
		if (*fmt == 'h' || *fmt == 'l' || *fmt == 'L')
		{
			qualifier = *fmt;
			fmt++;
		}

		// Default base
		base = 10;

		switch (*fmt)
		{
		case 'c':
			if (!(flags & LEFT)) while (--field_width > 0) *str++ = ' ';
			*str++ = (unsigned char) va_arg(args, int);
			while (--field_width > 0) *str++ = ' ';
			continue;

		case 's':
			s = va_arg(args, char *);
			if (!s) s = "<NULL>";
			len = mystrnlen(s, precision);
			if (!(flags & LEFT)) while (len < field_width--) *str++ = ' ';
			for (i = 0; i < len; ++i) *str++ = *s++;
			while (len < field_width--) *str++ = ' ';
			continue;

		case 'p':
			if (field_width == -1)
			{
				field_width = 2 * sizeof(void *);
				flags |= ZEROPAD;
			}
			str = number(str, (unsigned long) va_arg(args, void *), 16, field_width, precision, flags);
			continue;

		case 'n':
			if (qualifier == 'l')
			{
				long *ip = va_arg(args, long *);
				*ip = (str - buf);
			}
			else
			{
				int *ip = va_arg(args, int *);
				*ip = (str - buf);
			}
			continue;

		case 'A':
			flags |= LARGE;

		case 'a':
			if (qualifier == 'l')
				str = eaddr(str, va_arg(args, unsigned char *), field_width, precision, flags);
			else
				str = iaddr(str, va_arg(args, unsigned char *), field_width, precision, flags);
			continue;

			// Integer number formats - set up the flags and "break"
		case 'o':
			base = 8;
			break;

		case 'X':
			flags |= LARGE;

		case 'x':
			base = 16;
			break;

		case 'd':
		case 'i':
			flags |= SIGN;

		case 'u':
			break;

		default:
			if (*fmt != '%') *str++ = '%';
			if (*fmt)
				*str++ = *fmt;
			else
				--fmt;
			continue;
		}

		if (qualifier == 'l')
			num = va_arg(args, unsigned long);
		else if (qualifier == 'h')
		{
			if (flags & SIGN)
				num = va_arg(args, short);
			else
				num = va_arg(args, unsigned short);
		}
		else if (flags & SIGN)
			num = va_arg(args, int);
		else
			num = va_arg(args, unsigned int);

		str = number(str, num, base, field_width, precision, flags);
	}

	*str = '\0';
	return str - buf;
}



int _hx_sprintf(char *buf, const char *fmt, ...)
{
	va_list args;
	int n;
		
	va_start(args, fmt);
	n = _hx_vsprintf(buf, fmt, args);
	va_end(args);
		
	return 0;
}


int _hx_printf(const char* fmt,...)
{	
	va_list args  ;
	char buff[512] ;//= {0};		
	int   n        ;
	int i          ;

	va_start(args,fmt);
	n = _hx_vsprintf(buff,fmt,args);
	va_end(args);
	
	//convert not visable char  to visable
	for(i=0;i<n;i++)
	{
		if(buff[i] < ' ' )
		{
			buff[i] = ' ';
		}
	}
	PrintLine(buff);
		
	return 0;
}


void* mymemset (void *dst,int val,size_t count)
{
	char* start = (char*)dst;
	size_t   i;

	for(i=0;i<count;i++)	
	{
		*start = (char)val;
		start ++;		
	}

	return(dst);
}

int mystrlen(const char * s)
{
	int i;
	for (i = 0; s[i]; i++) ;
	return i;
}

char* mystrcpy(char * dst, const char * src)
{
	char * cp = dst;
	while( *cp++ = *src++ )
		;               /* Copy src over dst */
	return( dst );
}