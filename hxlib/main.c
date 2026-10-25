#include <hellox.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>

/* main routine must exist. */
extern int main(int argc, char* argv[]);

/* Global string to hold path and name. */
extern char g_current_path[];
extern char g_current_name[];

/* 
 * Local helper routine to save 
 * current application's path and
 * name.
 */
static void _save_path_name(const char* full_name)
{
	char path_name[MAX_FILE_NAME_LEN];

	strncpy(path_name, full_name, MAX_FILE_NAME_LEN);
	if (strlen(path_name) >= MAX_FILE_NAME_LEN)
	{
		/* Name and path too long. */
		exit(-1);
	}

	char* name_begin = strrchr(path_name, '\\');
	if (NULL == name_begin)
	{
		/* No name specified. */
		exit(-1);
	}
	/* Spiter into path and name, and save them. */
	*name_begin++ = 0;
	strncpy(g_current_path, path_name, MAX_FILE_NAME_LEN);
	strncpy(g_current_name, name_begin, MAX_FILE_NAME_LEN);
}

/* Entry point of hellox. */
int _hx_main(int argc, char* argv[])
{
	int ret = -1;

	/* Show lib version. */
	_hx_printf("HelloX Lib[%s]\r\n", HELLOX_LIB_VER);

	/* HelloX C library releated initialization. */
	_save_path_name(argv[0]);

	/* Invoke the main routine. */
	ret = main(argc, argv);
	return ret;
}
