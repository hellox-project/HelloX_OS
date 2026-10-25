/*
 * x_malloc.c
 *
 *  Modified: Jun 27, 2018
 *      Author: lightftp
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void *x_malloc(size_t size)
{
	void    *result;

	result = malloc(size);
	if (result == NULL)
	{
		printf("\r\n[%s]Out of memory,req_sz = [%d]\r\n",
			__func__, size);
		abort();
	}

	memset(result, 0, size);

	return result;
}
