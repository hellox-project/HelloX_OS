/*
  2015.2.12：底层fat差一个文件清空的操作
*/

#include <StdAfx.h>
#include "stdio.h"
#include "io.h"

#define  JVM_ROOT_PATH  "C:\\JVM\\"
#define  FILE_ROOT_PATH  "C:\\"

int remove (const char* name)
{
	IOManager.DeleteFile((__COMMON_OBJECT*)&IOManager,name);
	return S_OK;
}

int rename (const char* sn, const char*  dn)
{
	
	return S_OK;
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

int     chsize (int fd, long size)
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
	else if(size < dwFileSize)
	{
		long  dwOffset   = size;
		
		lseek(fd,dwOffset,SEEK_SET);
		IOManager.SetEndOfFile((__COMMON_OBJECT*)&IOManager,(__COMMON_OBJECT*)fd);
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


int     dup (int fd)
{
	return S_OK;
}

int     dup2 (int fd, int d)
{
	return S_OK;
}
int    eof (int fd)
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
	
	return (dwCurPos == dwFileSize-1)?TRUE:FALSE;
}

long    filelength (int fd)
{
	return tell(fd);
}

int     isatty (int fd)
{
	return S_OK;
}


int     close (int fd)
{
	IOManager.CloseFile((__COMMON_OBJECT*)&IOManager,(__COMMON_OBJECT*)fd);

	return S_OK;
}

int     creat (const char* name , int d)
{
	return open(name,d&O_CREAT);	
}

int     open (const char* name , int oflag, ...)
{
	va_list  ap;
	int      mode; 
	char     filename[FILENAME_MAX];
	char*    p;

	__COMMON_OBJECT*   pFileHandle = NULL;
	DWORD              dwFlage     = FILE_ACCESS_READ;
	DWORD              dwMode      = FILE_OPEN_EXISTING;

	
	if(name == NULL || strlen(name) >= FILENAME_MAX)
	{
		printf("open err 1\n");
		return -1;
	}
	
	p = filename;	
	if(!strstr(name,":")) 
	{
		strcpy(filename,FILE_ROOT_PATH);	
		p += strlen(FILE_ROOT_PATH);
	}
	strcpy(p,name+2);
	
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

	printf("open name=%s,oflag=%d,mode=%d \n",filename,oflag,mode);

	//设置访问模式
	switch(oflag&0x4)
	{
		case O_RDONLY:
			{
			dwFlage = FILE_ACCESS_READ;
			}
			break;
		case O_WRONLY:
			{
            dwFlage = FILE_ACCESS_WRITE;
			}
			break;
		case O_RDWR:
			{
            dwFlage = FILE_ACCESS_READWRITE;
			}
			break;
		default:
			{
			return -1;
			}		
	}

	//设置打开模式
	if(mode&O_CREAT)
	{
		dwMode = FILE_OPEN_NEW;
	}
	else if(mode&O_EXCL)
	{
		dwMode = FILE_OPEN_ALWAYS;
	}

	//打开文件
	pFileHandle = IOManager.CreateFile((__COMMON_OBJECT*)&IOManager,(LPSTR)filename,dwFlage,dwMode,NULL);
	if(NULL == pFileHandle)
	{
		printf("open err 2\n");

		return -1;
	}
	
	//清空文件，从头开始写
	if((oflag&O_TRUNC) && ((oflag&O_WRONLY) || (oflag&O_RDWR)))
	{
		IOManager.SetEndOfFile((__COMMON_OBJECT*)&IOManager,pFileHandle);
	}

	return (pFileHandle)?(int)pFileHandle:-1;
}

long  lseek (int fd, long p , int w)
{
	DWORD dwWhere = w+(FILE_FROM_BEGIN-SEEK_SET);

	if(fd <= 0)
	{
		return -1;
	}
		
	return IOManager.SetFilePointer((__COMMON_OBJECT*)&IOManager,(__COMMON_OBJECT*)fd,(DWORD*)&p,0,dwWhere);
	
}


int   read (int fd , void* buf, unsigned int r)
{
	DWORD  dwRead = 0;

	if(fd <= 0)
	{
		return -1;
	}

	IOManager.ReadFile((__COMMON_OBJECT*)&IOManager,(__COMMON_OBJECT*)fd,r,buf,&dwRead);

	return (int)dwRead;
}

int     write (int fd , const void* buf, unsigned int w)
{
	DWORD  dwWrite = 0;

	if(fd <= 0)
	{
		return -1;
	}

	IOManager.WriteFile((__COMMON_OBJECT*)&IOManager,(__COMMON_OBJECT*)fd,w,(LPVOID)buf,&dwWrite);

	return (int)dwWrite;
}

int  setmode (int fd, int m)
{
	return S_OK;
}

int   sopen (const char* name, int d1, int d2, ...)
{
	return S_OK;
}

long    tell (int fd )
{
	printf("tell fd =%d\n",fd);

	if(fd <= 0)
	{
		
		return -1;
	}

	return (long)IOManager.GetFileSize((__COMMON_OBJECT*)&IOManager,(__COMMON_OBJECT*)fd,NULL);
	
}

int     umask (int fd)
{
	return S_OK;
}

int     unlink (const char* name)
{
	return S_OK;
}


FILE*   fopen(const char * file, const char * fmt)
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

	fd = open(file,flage,mode);

	return (FILE*)fd;
}
int     fclose(FILE* stream)
{
	return close((int)stream);
	
}

size_t  fwrite(const void * buf, size_t size, size_t count, FILE * stream)
{
	 if(NULL == buf || NULL == stream)
	 {
		 return -1;
	 }

	 return	 (size_t)write((int)stream,buf,size*count);	 
}

size_t  fread(void * buf , size_t size , size_t count , FILE* stream)
{
	if(NULL == buf || NULL == stream)
	{
		return -1;
	}

	 return	 (size_t)read((int)stream,buf,size*count);	 
}

int     fseek(FILE * stream, long pos , int where)
{
	return lseek((int)stream,pos,where);	
}

long    ftell(FILE * stream)
{
	return tell((int)stream);

}
