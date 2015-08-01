//Implementation of stat routine.

#include <StdAfx.h>
#include <kapi.h>
#include <sys/stat.h>

int __cdecl fstat(int mode, struct stat * _stat)
{
	return 0;
}

int __cdecl stat(const char * name, struct stat * _stat)
{
	return 0;
}

char* __cdecl getcwd(char* buff,size_t len)
{
	return NULL;
}
