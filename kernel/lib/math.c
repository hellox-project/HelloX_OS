//Implements the floating point operations.

#include <math.h>

//Calculate the remainder of x/y.
double fmod(double x,double y)
{
	double temp, ret;
	if (y == 0.0)
	{
		return 0.0;
	}
	temp = floor(x/y);
	ret = x - temp * y;
	if ((x < 0.0) != (y < 0.0))
	{
		ret = ret - y;
	}
	return ret;
}

//Get the floor of x.
double floor(double x)
{
	double y=x;
	if( (*( ( (int *) &y)+1) & 0x80000000)  != 0) //or if(x<0)
	{
		return (float)((int)x)-1;
	}
	else
	{
		return (float)((int)x);
	}
}
