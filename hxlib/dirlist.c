/*
 * Testing application of directory function(dirent.h).
 * It does the following:
 *   1. Open a directory, specified by command;
 *   2. Iterate the direcotry, show it's contents;
 *   3. Close the directory.
 */

#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/stat.h>

int _dirlist_main(int argc, char* argv[])
{
	if (argc < 2)
	{
		printf("No directory secified.\r\n");
		return -1;
	}

	/* Open directory. */
	DIR* dir;
	struct dirent* entry;

	dir = opendir(argv[1]);
	if (NULL == dir)
	{
		printf("Could not open dir[%s]\n", argv[1]);
		return -1;
	}

	/* Iterate the directory. */
	while ((entry = readdir(dir)) != NULL)
	{
		/* Show out the entry. */
		printf("  %s\n", entry->d_name);
	}

	/* Close the directory. */
	closedir(dir);

	return 0;
}