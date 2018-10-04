//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Aug 03,2018
//    Module Name               : apic.c
//    Module Funciton           : 
//                                APIC(Advanced Programmable Interrupt Controller)
//                                related source code.
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
#include <string.h>
#include <smp.h>

#include "apic.h"

/* Routines to operate local APIC. */
/* Acknowledge interrupt. */
static BOOL lapicAckInterrupt(__INTERRUPT_CONTROLLER* pThis, unsigned int intVector)
{
	BOOL bResult = FALSE;

	/* Check parameters. */
	if (NULL == pThis)
	{
		goto __TERMINAL;
	}
	/* Check object signature. */
	if (pThis->objectSignature != KERNEL_OBJECT_SIGNATURE)
	{
		goto __TERMINAL;
	}
	if ((intVector < pThis->vectorStart) || (intVector > pThis->vectorEnd))
	{
		/* Not belong to this controller. */
		goto __TERMINAL;
	}
	/* OK,acknowledge the specified interrupt. */
	__writel(0, pThis->pBase + LAPIC_REGISTER_EOI);
	bResult = TRUE;

__TERMINAL:
	return bResult;
}

/* Send initial IPI. */
static BOOL lapicSend_Init_IPI(__INTERRUPT_CONTROLLER* pThis, unsigned int destination)
{
	BOOL bResult = FALSE;
	uint8_t* pBase = NULL;

	/* Check parameters. */
	if (NULL == pThis)
	{
		goto __TERMINAL;
	}
	/* Check object signature. */
	if (pThis->objectSignature != KERNEL_OBJECT_SIGNATURE)
	{
		goto __TERMINAL;
	}
	pBase = pThis->pBase;
	BUG_ON(NULL == pBase);
	/* OK,send initial IPI. */
	if (INTERRUPT_DESTINATION_ALL == destination)
	{
		/* Write the high dword to 0. */
		__writel(0, (pBase + LAPIC_REGISTER_ICR_HIGH));
		/*
		 * Write low dword as follows:
		 * Vector: 0x00
		 * Delivery mode: Startup
		 * Destination mode: ignored(0)
		 * Level: ignored(1)
		 * Trigger mode: ignored(0)
		 * Shorthand: All excluding self(3)
		 */
		__writel(0x0C4500, (pBase + LAPIC_REGISTER_ICR_LOW));
		/* Wait a short while. */
		__MicroDelay(100);
		bResult = TRUE;
		goto __TERMINAL;
	}
	else
	{
		_hx_printf("%s:not supported yet.\r\n", __func__);
		goto __TERMINAL;
	}
__TERMINAL:
	return bResult;
}

/* Send Startup IPI. */
static BOOL lapicSend_Start_IPI(__INTERRUPT_CONTROLLER* pThis, unsigned int destination)
{
	BOOL bResult = FALSE;
	uint8_t* pBase = NULL;

	/* Check parameters. */
	if (NULL == pThis)
	{
		goto __TERMINAL;
	}
	/* Check object signature. */
	if (pThis->objectSignature != KERNEL_OBJECT_SIGNATURE)
	{
		goto __TERMINAL;
	}
	pBase = pThis->pBase;
	BUG_ON(NULL == pBase);

	/* OK,send startup IPI. */
	if (INTERRUPT_DESTINATION_ALL == destination)
	{
		/* Set high dword to 0. */
		__writel(0, (pBase + LAPIC_REGISTER_ICR_HIGH));
		/* 
		 * Write low dword,vector is the trampoline's base address,
		 * which is in miniker and defined as __SMP_TRAMPOLINE_BASE.
		 * Must shift 12 bits to right before write.
		 */
		uint32_t low_dword = 0x0C4600;
		low_dword |= (0xFF & (__SMP_TRAMPOLINE_BASE >> 12));
		__writel(low_dword, (pBase + LAPIC_REGISTER_ICR_LOW));
		/* Wait a short while. */
		__MicroDelay(100);
		bResult = TRUE;
		goto __TERMINAL;
	}
	else
	{
		_hx_printf("%s:not support yet.\r\n", __func__);
		goto __TERMINAL;
	}

__TERMINAL:
	return bResult;
}

/* Send general IPI. */
static BOOL lapicSend_IPI(__INTERRUPT_CONTROLLER* pThis, int destination, unsigned int ipiType)
{
	BOOL bResult = FALSE;

	/* Check parameters. */
	if (NULL == pThis)
	{
		goto __TERMINAL;
	}
	/* Check object signature. */
	if (pThis->objectSignature != KERNEL_OBJECT_SIGNATURE)
	{
		goto __TERMINAL;
	}
	/* OK,send general IPI. */

__TERMINAL:
	return bResult;
}

/* 
 * Handler of APIC's local timer interrupt. 
 * This interrupt will driver the AP wakeup from halt status,
 * accumulate time stampe counter and do some other things
 * specific to AP...
 */
static BOOL APICTimerIntHandler(LPVOID lpEsp, LPVOID lpParam)
{
	return TRUE;
}

/* Setup local APIC's timer. */
static BOOL SetupAPICTimer(__INTERRUPT_CONTROLLER* pIntCtrl)
{
	uint32_t apic_freq = 0xFFFFFFFF;
	uint8_t* pBase = NULL;
	BOOL bResult = FALSE;
	static __COMMON_OBJECT* pTmrInt = NULL;

	BUG_ON(NULL == pIntCtrl);
	BUG_ON(pIntCtrl->objectSignature != KERNEL_OBJECT_SIGNATURE);
	pBase = pIntCtrl->pBase;

	/*
	 * Measure the frequency of local APIC's timer,using it's
	 * one shot mode and __MicroDelay routine.
	 */
	__writel(INTERRUPT_VECTOR_APICTMR, pBase + LAPIC_REGISTER_LVT_TIMER);
	/* Use divider 16. */
	__writel(0x03, pBase + LAPIC_REGISTER_TIMER_DIV);
	/* Set APIC's initial timer counter to maximal. */
	__writel(0xFFFFFFFF, pBase + LAPIC_REGISTER_TIMER_INITCNT);
	/* Delay a short while to let APIC count down. */
	__MicroDelay(SYSTEM_TIME_SLICE * 1000 * 16);
	/* Stop the APIC timer. */
	__writel(LAPIC_LVT_INT_DISABLE, pBase + LAPIC_REGISTER_LVT_TIMER);
	/* Make sure the timer is stop before the following read. */
	__MEMORY_BARRIER();
	/* Get how many ticks the APIC has elapsed. */
	apic_freq -= __readl(pBase + LAPIC_REGISTER_TIMER_CURRCNT);
	if (0xFFFFFFFF == apic_freq)
	{
		/* Local APIC may fail,since the timer is not count down. */
		_hx_printk("Local APIC[%d] may fail,give up.\r\n", __CURRENT_PROCESSOR_ID);
		goto __TERMINAL;
	}
	_hx_printk("APIC ticks in time[%dms] = 0x%X\r\n",
		SYSTEM_TIME_SLICE * 16, apic_freq);
	apic_freq /= 16;

	/* 
	 * Setup the APIC's timer interrupt handler,only BSP 
	 * is responsing for this,all APs just save the already created
	 * interrupt object into interrupt controller object for use.
	 */
	if (NULL == pTmrInt)
	{
		/* Current processor must be BSP. */
		BUG_ON(!__CURRENT_PROCESSOR_IS_BSP());
		pTmrInt = (__COMMON_OBJECT*)ConnectInterrupt(APICTimerIntHandler, NULL, INTERRUPT_VECTOR_APICTMR);
		if (NULL == pTmrInt)
		{
			_hx_printk("%s:failed to connect interrupt.\r\n", __func__);
			goto __TERMINAL;
		}
		pIntCtrl->pIntObj = pTmrInt;
	}
	else
	{
		/*
		 * Execution in AP,timer interrupt already connected by BSP. 
		 */
		BUG_ON(__CURRENT_PROCESSOR_IS_BSP());
		pIntCtrl->pIntObj = pTmrInt;
	}

	/* 
	 * We know the frequency of APIC approximitely,setup the 
	 * APIC timer now,the frequency of APIC timer is lower LAPIC_TIMER_INT_MULTIPLIXER
	 * times than system clock.
	 */
	if (apic_freq > (0xFFFFFFFF / LAPIC_TIMER_INT_MULTIPLIXER))
	{
		apic_freq = 0xFFFFFFFF;
	}
	else
	{
		apic_freq *= LAPIC_TIMER_INT_MULTIPLIXER;
	}
	__writel(LAPIC_TIMER_MODE_PERIODIC | INTERRUPT_VECTOR_APICTMR,
		pBase + LAPIC_REGISTER_LVT_TIMER);
	__writel(0x03, pBase + LAPIC_REGISTER_TIMER_DIV);
	__writel(apic_freq, pBase + LAPIC_REGISTER_TIMER_INITCNT);
	__writel(0, pBase + LAPIC_REGISTER_EOI);

	/* Setup APIC timer OK. */
	bResult = TRUE;

__TERMINAL:
	return bResult;
}

/* Initializer of local APIC controller. */
static BOOL lapicInitialize(__INTERRUPT_CONTROLLER* pIntCtrl)
{
	BOOL bResult = FALSE;
	uint8_t* pBase = NULL;

	BUG_ON(NULL == pIntCtrl);
	BUG_ON(pIntCtrl->objectSignature != KERNEL_OBJECT_SIGNATURE);
	pBase = pIntCtrl->pBase;

	/* Enable APIC now. */
	uint32_t sivr = __readl(pBase + LAPIC_REGISTER_SIVR);
	sivr |= 0x1FF; /* Set suprious interrupt vector as 0xFF. */
	__writel(sivr, pBase + LAPIC_REGISTER_SIVR);
	sivr = __readl(pBase + LAPIC_REGISTER_SIVR);
#if 0
	_hx_printk("Local APIC[%d]:base = 0x%X,type = %s,sivr = 0x%X\r\n", pIntCtrl->id,
		pIntCtrl->pBase,
		pIntCtrl->typeName,
		sivr);
#endif
	/* Wait for the APIC controller to stable. */
	__MicroDelay(10000);
	/* Setup APIC's local timer. */
	if (!SetupAPICTimer(pIntCtrl))
	{
		goto __TERMINAL;
	}

	bResult = TRUE;
__TERMINAL:
	return bResult;
}

/*
* Creates and initializes an interrupt controller object,
* given it's base address,id,and type.
* This like a class factory of interrupt controller.
*/
__INTERRUPT_CONTROLLER* CreateInterruptController(uint8_t* pBase, unsigned int id, 
	unsigned int type,
	unsigned char vectorStart,
	unsigned char vectorEnd)
{
	__INTERRUPT_CONTROLLER* pIntCtrl = NULL;

	/* Check parameters. */
	if (NULL == pBase)
	{
		goto __TERMINAL;
	}
	if ((type != APIC_TYPE_IO) &&
		(type != APIC_TYPE_XAPIC) &&
		(type != APIC_TYPE_2XAPIC))
	{
		goto __TERMINAL;
	}

	/* Create the interrupt controller object. */
	pIntCtrl = (__INTERRUPT_CONTROLLER*)_hx_calloc(1, sizeof(__INTERRUPT_CONTROLLER));
	if (NULL == pIntCtrl)
	{
		goto __TERMINAL;
	}
	pIntCtrl->objectSignature = KERNEL_OBJECT_SIGNATURE;
	pIntCtrl->pBase = pBase;
	pIntCtrl->id = id;
	pIntCtrl->vectorStart = vectorStart;
	pIntCtrl->vectorEnd = vectorEnd;
	pIntCtrl->pIntObj = NULL;

	switch (type)
	{
	case APIC_TYPE_IO:
		pIntCtrl->typeName = APIC_NAME_IO;
		_hx_printk("WARNING: Not implemented yet.\r\n");
		break;
	case APIC_TYPE_XAPIC:
		pIntCtrl->Initialize = lapicInitialize;
		pIntCtrl->AckInterrupt = lapicAckInterrupt;
		pIntCtrl->Send_Init_IPI = lapicSend_Init_IPI;
		pIntCtrl->Send_IPI = lapicSend_IPI;
		pIntCtrl->Send_Start_IPI = lapicSend_Start_IPI;
		pIntCtrl->typeName = APIC_NAME_XAPIC;
		break;
	case APIC_TYPE_2XAPIC: /* Use xAPIC compatiable mode. */
		pIntCtrl->Initialize = lapicInitialize;
		pIntCtrl->AckInterrupt = lapicAckInterrupt;
		pIntCtrl->Send_Init_IPI = lapicSend_Init_IPI;
		pIntCtrl->Send_IPI = lapicSend_IPI;
		pIntCtrl->Send_Start_IPI = lapicSend_Start_IPI;
		pIntCtrl->typeName = APIC_NAME_2XAPIC;
		break;
	}

__TERMINAL:
	return pIntCtrl;
}

/* Destroy the given interrupt controller object. */
void DestroyInterruptController(__INTERRUPT_CONTROLLER* pIntCtrl)
{
	if (NULL == pIntCtrl)
	{
		return;
	}
	/* Check object signature. */
	if (pIntCtrl->objectSignature != KERNEL_OBJECT_SIGNATURE)
	{
		return;
	}
	/* Release it. */
	pIntCtrl->objectSignature = 0;
	_hx_free(pIntCtrl);
}
