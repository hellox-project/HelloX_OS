//Implementation of stat routine.

#include "hellox.h"
#include "stddef.h"
#include "stdlib.h"
#include "string.h"
#include "sys/stat.h"

int __CDECL fstat(int fd, struct stat * _stat)
{
	HANDLE hFile = (HANDLE)fd;
	BY_HANDLE_FILE_INFORMATION fileInfo;
	BOOL bResult = FALSE;
	int ret = -1;

	/* Check parameters. */
	if ((hFile == INVALID_HANDLE_VALUE) || (NULL == hFile) || (NULL == _stat))
	{
		goto __TERMINAL;
	}

	/* Retrieve file information by HANDLE. */
	bResult = GetFileInformationByHandle(hFile, &fileInfo);
	if (!bResult)
	{
		goto __TERMINAL;
	}

	/* Convert into struct stat format. */
	memset(_stat, 0, sizeof(struct stat));
	_stat->st_size = fileInfo.nFileSizeLow;
	if (fileInfo.dwFileAttributes & FILE_ATTR_DIRECTORY)
	{
		_stat->st_mode |= S_IFDIR;
	}
	else {
		_stat->st_mode |= S_IFREG;
	}
	/* Owner has all permissions. */
	_stat->st_mode |= (S_IRUSR | S_IWUSR | S_IXUSR);
	/* Group has all permissions. */
	_stat->st_mode |= (S_IRGRP | S_IWGRP | S_IXGRP);
	/* Other also has all permissions. */
	_stat->st_mode |= (S_IROTH | S_IWOTH | S_IXOTH);

__TERMINAL:
	return ret;
}

int __CDECL stat(const char * name, struct stat * _stat)
{
	FILE_ATTRIBUTE_DATA fad;
	int ret = -1;

	if ((NULL == _stat) || (NULL == name))
	{
		goto __TERMINAL;
	}

	if (!GetFileAttributesEx(name, GetFileExInfoStandard, &fad))
	{
		goto __TERMINAL;
	}

	/* Convert into struct stat format. */
	memset(_stat, 0, sizeof(struct stat));
	_stat->st_size = fad.nFileSizeLow;
	if (fad.dwFileAttributes & FILE_ATTR_DIRECTORY)
	{
		_stat->st_mode |= S_IFDIR;
	}
	else {
		_stat->st_mode |= S_IFREG;
	}
	/* Owner has all permissions. */
	_stat->st_mode |= (S_IRUSR | S_IWUSR | S_IXUSR);
	/* Group has all permissions. */
	_stat->st_mode |= (S_IRGRP | S_IWGRP | S_IXGRP);
	/* Other also has all permissions. */
	_stat->st_mode |= (S_IROTH | S_IWOTH | S_IXOTH);

	ret = 0;

__TERMINAL:
	return ret;
}
