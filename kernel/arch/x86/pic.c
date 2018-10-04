//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Sep 05,2018
//    Module Name               : pic.c
//    Module Funciton           : 
//                                Programming Interrupt Controller(PIC)'s
//                                implementation code. 
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#include <StdAfx.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "arch.h"
#include "pic.h"

/* Mask a specified interrupt. */
void __Set_Interrupt_Mask(unsigned char ucVector)
{
	uint16_t port;
	uint8_t value;

	BUG_ON(ucVector < INTERRUPT_VECTOR_BASE);
	ucVector -= INTERRUPT_VECTOR_BASE;
	if (!__INTERRUPT_IS_MASKABLE(ucVector))
	{
		return;
	}

	if (ucVector < 8)
	{
		port = PIC1_DATA;
	}
	else
	{
		port = PIC2_DATA;
		ucVector -= 8;
	}
	/* Combine the corresponding mask with old value. */
	value = __inb(port) | (1 << ucVector);
	__outb(value, port);
}

/* Unmask a specified interrupt. */
void __Clear_Interrupt_Mask(unsigned char ucVector)
{
#if 1
	uint16_t port;
	uint8_t value;

	BUG_ON(ucVector < INTERRUPT_VECTOR_BASE);
	ucVector -= INTERRUPT_VECTOR_BASE;
	if (!__INTERRUPT_IS_MASKABLE(ucVector))
	{
		return;
	}

	if (ucVector < 8)
	{
		port = PIC1_DATA;
	}
	else
	{
		port = PIC2_DATA;
		ucVector -= 8;
	}
	/* Clear the corresponding mask from the old value. */
	value = __inb(port) & ~(1 << ucVector);
	__outb(value, port);
#endif
}

/*
* Mask all interrupts,is called in begin of
* system initialization.
*/
void __Mask_All()
{
	for (unsigned char i = 0; i < 16; i++)
	{
		if (i == 2)
		{
			/* 
			 * Skip the cascading pin,it should alwarys 
			 * enabled since the sub-level PIC will use
			 * it.
			 */
			continue;
		}
		__Set_Interrupt_Mask(i + INTERRUPT_VECTOR_BASE);
	}
}
