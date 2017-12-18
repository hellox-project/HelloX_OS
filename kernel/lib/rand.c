//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Feb,05 2017
//    Module Name               : rand.c
//    Module Funciton           : 
//                                Standard C rand() routine's implementation
//                                code.
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#include "stdlib.h"

/**
* Random number's seed.
*/
static unsigned long next = 1;

/**
* Helper routine to generate a random number.
*/
static long do_rand(unsigned long* value)
{
	long quotient, remainder, t;

	quotient = *value / 127773L;
	remainder = *value % 127773L;
	t = 16807L * remainder - 2836L * quotient;
	if (t <= 0)
	{
		t += 0x7FFFFFFFL;
	}

	return ((*value = t) % ((unsigned long)RAND_MAX + 1));
}

/**
* Return a random number according the current seed.
*/
long rand()
{
	return do_rand(&next);
}

/**
* Update the seed value.
*/
void srand(unsigned long seed)
{
	next = seed;
}
