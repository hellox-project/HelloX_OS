/*
 * Time functions testing. 
 */

#include <hellox.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <math.h>

int _time_main(int argc, char* argv[])
{
	clock_t start_t, end_t;
	unsigned long total_t;
	int i;
	unsigned long long accu = 0;

	start_t = clock();
	printf("Start application, start_t = %lld\n", start_t);

	printf("Begin a big loop, start_t = %lld\n", start_t);
	for (i = 0; i < 80000000; i++)
	{
		accu += i;
		accu %= i;
	}
	end_t = clock();
	printf("End the big loop, end_t = %lld\n", end_t);

	total_t = (unsigned long)(end_t - start_t) / CLOCKS_PER_SEC;
	printf("CPU total time: %ld\n", total_t);

	return(0);

#if 0
	time_t rawtime;
	struct tm *info;
	char buffer[80];

	_time(&rawtime);

	info = _localtime(&rawtime);

	_strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", info);
	printf("Formatted date and time : |%s|\n", buffer);

	return(0);
#endif 
}