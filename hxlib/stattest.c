/*
 * Test the functions in stat.h.
 */

#include <hellox.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <io.h>
#include <sys/stat.h>

int _stat_main(int argc, char* argv[])
{
	char current_wd[MAX_FILE_NAME_LEN];

	getcwd(current_wd, MAX_FILE_NAME_LEN);
	printf("CWD: %s\r\n", current_wd);

	struct stat _stat;

	if (argc < 2)
	{
		printf("No file name secified.\r\n");
		return -1;
	}

	if (stat(argv[1], &_stat) == 0)
	{
		/* Show file attributes info. */
		if (_stat.st_mode & S_IFREG)
		{
			printf("File size: %d\n", _stat.st_size);
			printf("File mode: 0x%X\n", _stat.st_mode);
		}
		if (_stat.st_mode & S_IFDIR)
		{
			printf("Dir name: %s\n", argv[1]);
			printf("Dir mode: 0x%X\n", _stat.st_mode);
		}
	}
	else {
		printf("Invoke stat on[%s] fail.\n",
			argv[1]);
	}
	return 0;
}

int __main(int argc, char* argv[])
{
	HANDLE hFile = NULL;
	BY_HANDLE_FILE_INFORMATION fileInfo;

	if (argc < 2)
	{
		printf("No file name secified.\r\n");
		return -1;
	}

	hFile = CreateFile(argv[1], FILE_ACCESS_READWRITE, 0, NULL);
	if (NULL == hFile)
	{
		printf("Can not open file[%s]\n", argv[1]);
		return -1;
	}

	if (!GetFileInformationByHandle(hFile, &fileInfo))
	{
		printf("Can not get file info[%d]\n", GetLastError());
		CloseFile(hFile);
		return -1;
	}

	/* Show out file information. */
	printf("File info as follows:\n");
	printf("  Size: %d\n", fileInfo.nFileSizeLow);
	printf("  ID: %d/%d\n", fileInfo.nFileIndexHigh, fileInfo.nFileIndexLow);
	printf("  Attr: 0x%X\n", fileInfo.dwFileAttributes);
	CloseFile(hFile);

	return 0;
}
