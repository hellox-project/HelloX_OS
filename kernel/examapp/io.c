/*
 * I/O operations of standard C library, under HelloX
 * operating system.
 */

#include "hellox.h"
#include "stdio.h"
#include "string.h"
#include "errno.h"
#include "io.h"

/* Remove the specified file. */
int remove (const char* name)
{
	if (DeleteFile((LPSTR)name))
	{
		return S_OK;
	}
	return -EIO;
}

/* Rename a file name from sn to dn. */
int rename (const char* sn, const char*  dn)
{
	return -EIO;
}

int access (const char* name, int m)
{
	switch(m)
	{
		case F_OK:
			{
			}
			break;
		case R_OK:
			{
			}
			break;
		case W_OK:
			{
			}
			break;
	}

	return S_OK;
}

/* Change a file object's size. */
int chsize (int fd, long size)
{
	DWORD  dwFileSize = 0;

	if(fd <= 0)
	{
		return -1;
	}

	dwFileSize = filelength(fd);
	if(size == dwFileSize)
	{
		return size;
	}
	else if((DWORD)size < dwFileSize)
	{
		long  dwOffset   = size;
		
		lseek(fd,dwOffset,SEEK_SET);
		SetEndOfFile((HANDLE)fd);
	}
	else 
	{
		long   dwOffset    = dwFileSize;
		char   temp[128]   = {0};
		int    nLeftData   = size-dwFileSize;
		int    nWriteCount = nLeftData/sizeof(temp);
		int    i;

		lseek(fd,dwOffset,SEEK_SET);

		
		//安装步长填充文件空白区间
		for(i = 0; i<nWriteCount;i++)
		{
			write(fd,temp,sizeof(temp));
		}

		//填充文件空白剩余区间
		if(nWriteCount&sizeof(temp))
		{
			nWriteCount = nWriteCount&sizeof(temp);

			for(i = 0; i<nWriteCount;i++)
			{
				write(fd,temp,1);
			}
		}
	}

	return S_OK;
}

int dup (int fd)
{
	return S_OK;
}

int dup2 (int fd, int d)
{
	return S_OK;
}

int eof (int fd)
{
	DWORD  dwOffset   = 0;
	DWORD  dwCurPos   = 0;
	DWORD  dwFileSize = 0;

	if(fd <= 0)
	{
		return -1;
	}

	dwCurPos   = lseek(fd,dwOffset,SEEK_CUR);	
	dwFileSize = filelength(fd);
	
	return (dwCurPos == dwFileSize-1)? TRUE : FALSE;
}

long filelength (int fd)
{
	return tell(fd);
}

int isatty (int fd)
{
	return S_OK;
}

/* Close the specified file. */
int close (int fd)
{
	if (0 == fd)
	{
		_hx_printf("[%s]invalid fd value: 0\r\n", __func__);
	}
	CloseFile((HANDLE)fd);
	return S_OK;
}

/* Create a new file. */
int creat (const char* name , int d)
{
	return open(name,d&O_CREAT);	
}

/* Open or create a file. */
int open (const char* name , int oflag, ...)
{
	va_list ap;
	int mode; 
	char filename[FILENAME_MAX];
	char* p;
	HANDLE f_handle = NULL;
	unsigned long dwFlag = FILE_ACCESS_READ;
	unsigned long dwMode = FILE_OPEN_ALWAYS;

	/* Parameter checking. */
	if(name == NULL || strlen(name) >= FILENAME_MAX)
	{
		_hx_printf("[%s]invalid parameter value.\r\n", __func__);
		return -EINVAL;
	}
	
	p = filename;	
	if(!strstr(name, ":"))
	{
		strcpy(filename,FILE_ROOT_PATH);	
		p += strlen(FILE_ROOT_PATH);
	}
	strcpy(p, name);
	
	while(*p)	
	{
		if(*p == '/')
		{
			*p  = '\\';
		}
		p ++;
	}
	
	va_start(ap, oflag);
	mode = va_arg(ap, int);
	va_end(ap);

	/* Convert accessing mode to hellox specified. */
	switch(oflag & 0x4)
	{
		case O_RDONLY:
			dwFlag = FILE_ACCESS_READ;
			break;
		case O_WRONLY:
            dwFlag = FILE_ACCESS_WRITE;
			break;
		case O_RDWR:
            dwFlag = FILE_ACCESS_READWRITE;
			break;
		default:
			return -1;	
	}

	/* Convert open mode to hellox specific. */
	if(mode & O_CREAT)
	{
		dwMode = FILE_OPEN_NEW;
	}
	else if(mode & O_EXCL)
	{
		dwMode = FILE_OPEN_ALWAYS;
	}

	/* Open the file. */
	f_handle = CreateFile((LPSTR)filename, dwFlag | dwMode, 0, NULL);
	if(NULL == f_handle)
	{
		_hx_printf("[%s]open file[%s] failed, flag = %d, mode = 0x%X.\r\n",
			__func__, filename, dwFlag, dwMode);
		return -1;
	}
	
	/* Clear file and start from begin. */
	if((oflag & O_TRUNC) && ((oflag & O_WRONLY) || (oflag & O_RDWR)))
	{
		SetEndOfFile(f_handle);
	}

	return (f_handle) ? (int)f_handle : -1;
}

long lseek (int fd, long p , int w)
{
	DWORD dwWhere = w + (FILE_FROM_BEGIN-SEEK_SET);

	if(fd <= 0)
	{
		return -1;
	}
	SetFilePointer((HANDLE)fd, (unsigned long*)&p, NULL, dwWhere);
	return S_OK;
}

/* Read from file. */
int read(int fd , void* buf, unsigned int r)
{
	unsigned long read_sz = 0;

	if(fd <= 0)
	{
		return -1;
	}

	if (ReadFile((HANDLE)fd, r, buf, &read_sz))
	{
		return (int)read_sz;
	}
	else {
		return -EIO;
	}
}

/* Write into file. */
int write (int fd , const void* buf, unsigned int w)
{
	unsigned long write_sz = 0;

	if (WriteFile((HANDLE)fd, w, (LPVOID)buf, &write_sz))
	{
		return (int)write_sz;
	}
	else {
		return -EIO;
	}
}

/* Set accessing mode. */
int setmode (int fd, int m)
{
	return S_OK;
}

int sopen (const char* name, int d1, int d2, ...)
{
	return S_OK;
}

/* Returns the current file pointer's position. */
long tell (int fd )
{
	unsigned long size = 0;
	if(fd <= 0)
	{	
		return -1;
	}
	if (size = GetFileSize((HANDLE)fd, NULL))
	{
		return (long)size;
	}
	return -EIO;
}

int umask (int fd)
{
	return S_OK;
}

int unlink (const char* name)
{
	return S_OK;
}

/* Open a file and returns it's handle. */
FILE* fopen(const char * file, const char * fmt)
{
	int  flage  = 0;
	int  mode   = 0;
	int  fd     = -1;

	if(NULL	 == file)
	{
		return NULL;
	}

	if(strcmp(fmt,"r") == 0)
	{
		flage = O_RDONLY;
	}
	else if(strcmp(fmt,"w") == 0)
	{
		flage = O_WRONLY;
	}
	else if(strcmp(fmt,"r+") == 0)
	{
		flage = O_RDWR;
		mode   = O_EXCL;
	}
	else if(strcmp(fmt,"w+") == 0)
	{
		flage = O_RDWR|O_TRUNC;
		mode   = O_EXCL;
	}
	else if(strcmp(fmt,"a+") == 0)
	{
		flage = O_RDONLY;
		mode   = O_EXCL;
	}

	fd = open(file, flage, mode);
	if (fd < 0)
	{
		/* Open error. */
		return NULL;
	}
	return (FILE*)fd;
}

/* Close a opened file. */
int fclose(FILE* stream)
{
	if (NULL == stream)
	{
		_hx_printf("[%s]invalid stream value: NULL\r\n", __func__);
		return -1;
	}
	return close((int)stream);
}

/* Write into file. */
size_t fwrite(const void* buf, size_t size, size_t count, FILE* stream)
{
	size_t ret = 0;
	int write_result = -1;

	 if(NULL == buf || NULL == stream)
	 {
		 goto __TERMINAL;
	 }
	 write_result = write((int)stream, buf, size * count);
	 if (write_result < 0)
	 {
		 /* For debugging. */
		 _hx_printf("[%s]write error, ret = [%d], req size = [%d], req count = [%d]\r\n",
			 __func__, ret, size, count);
		 goto __TERMINAL;
	 }
	 /* Write ok, return the records number written. */
	 ret = (size_t)write_result / size;

 __TERMINAL:
	 return ret;
}

/* Read from file. */
size_t fread(void* buf , size_t size , size_t count , FILE* stream)
{
	if(NULL == buf || NULL == stream)
	{
		return -1;
	}

	return (size_t)read((int)stream, buf, size * count);	 
}

/* Change file's current pointer. */
int fseek(FILE* stream, long pos, int where)
{
	return lseek((int)stream,pos,where);	
}

/* Get current pointer's position. */
long ftell(FILE* stream)
{
	return tell((int)stream);
}
