//***********************************************************************/
//    Author                    : Garry
//    Original Date             : May 11,2026
//    Module Name               : vesa.c
//    Module Funciton           : 
//                                Wrapper or implementation code for VESA
//                                standard, used by GUI module.
//                                
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#include <StdAfx.h>
#include <process.h>
#include <stdio.h>
#include "bios.h"
#include "vesa.h"

/*
 * TODO: Should use mutex to protect
 * the buffer pointer and global VBE
 * memory block, since SwitchToGraphic
 * maybe invoked by multiple threads.
 * A global Controller maybe created
 * for each display controller and
 * initialized in process of system
 * boot, mutex should be created in
 * this procedure.
 */

/* Frame buffer's lineary address. */
static LPVOID g_pFrameBuffer = NULL;

/* 
 * Switch display mode into text from
 * Graphic. 
 * It just invokes the agent handler
 * in BIOS interface __biosSwitchToText.
 * BIOS interface then call the ASM code
 * in realinit.bin module, using the
 * function index and unified entry address.
 */
void SwitchToText()
{
	__VBE_INFO* pVbeInfo = (__VBE_INFO*)VBE_INFO_START;
	__PROCESS_OBJECT* pProcess = ProcessManager.GetCurrentProcess((__COMMON_OBJECT*)&ProcessManager);
	__VIRTUAL_MEMORY_MANAGER* pVmmMgr = NULL;

	BUG_ON(NULL == pProcess);
	pVmmMgr = pProcess->pMemMgr;
	BUG_ON(NULL == pVmmMgr);

	/* Just return if already in text mode. */
	if (!pVbeInfo->IfInGraphic)
	{
		return;
	}

	/* Release display memory */
	pVmmMgr->VirtualFree(
		(__COMMON_OBJECT*)pVmmMgr,
		g_pFrameBuffer);
	g_pFrameBuffer = NULL;

	/* Clear graphic mode flag. */
	pVbeInfo->IfInGraphic = 0;

	/* Switch to text mode. */
	__biosSwitchToText();
}

/*
 * Switch display mode into graphic.
 * Procedures as follows:
 * 1. Invoke BIOS interface __biosSwitchToGraphic, then
 *    the display will enter GRAPHIC mode and VBE block
 *    will be stored into __VBE_INFO_START address;
 * 2. Calculate the frame buffer size and map it into
 *    line space, could be accessed from user;
 * 3. Copy the whole VBE memory block into input parameter;
 * 4. Return the frame buffer's lineary address so that
 *    user GUI module can access directly;
 * 5. NULL will be returned in any case of error.
 * Please be noted the I/O space is maped to CURRENT
 * process and kernel.
 */
LPVOID SwitchToGraphic(__VBE_INFO* pUserVbeInfo)
{
	int disp_mem_sz = 0;
	LPVOID dispMemAddr = NULL;
	int pixels = 0, i = 0;
	__VBE_INFO* pVbeInfo = (__VBE_INFO*)VBE_INFO_START;
	__PROCESS_OBJECT* pProcess = ProcessManager.GetCurrentProcess((__COMMON_OBJECT*)&ProcessManager);
	__VIRTUAL_MEMORY_MANAGER* pVmmMgr = NULL;
	BOOL bResult = FALSE;

	BUG_ON(NULL == pProcess);
	pVmmMgr = pProcess->pMemMgr;
	BUG_ON(NULL == pVmmMgr);

	/* If already in graphic mode then just return. */
	if (pVbeInfo->IfInGraphic)
	{
		goto __TERMINAL;
	}

	/* Switch controller to graphic mode. */
	if (!__biosSwitchToGraphic())
	{
		goto __TERMINAL;
	}

	/* Get the display memory's size. */
	disp_mem_sz = pVbeInfo->ModeInfo.xresolution;
	disp_mem_sz *= pVbeInfo->ModeInfo.yresolution;
	disp_mem_sz *= pVbeInfo->ModeInfo.bitsperpixel;
	disp_mem_sz /= 8;

	/*
	 * Map display memory into current process's 
	 * virtual space.
	 * Can be accessed from user since the frame
	 * buffer will be returned to user.
	 */
	dispMemAddr = pVmmMgr->VirtualAlloc(
		(__COMMON_OBJECT*)pVmmMgr,
		(LPVOID)pVbeInfo->ModeInfo.physbaseptr,
		disp_mem_sz,
		VIRTUAL_AREA_ALLOCATE_IO | VIRTUAL_AREA_ALLOCATE_IOSPACE,
		VIRTUAL_AREA_ACCESS_RW | VIRTUAL_AREA_ACCESS_USER,
		"dispmem", NULL);
	if (NULL == dispMemAddr)
	{
		_hx_printf("[%s]can not map display I/O.\r\n", __func__);
		goto __TERMINAL;
	}

	/* 
	 * Save frame buffer to global, so as can be 
	 * released later.
	 */
	g_pFrameBuffer = dispMemAddr;

	/* Copy back the VBE info block. */
	memcpy(pUserVbeInfo, pVbeInfo, sizeof(__VBE_INFO));

	bResult = TRUE;

__TERMINAL:
	if (!bResult)
	{
		/* Switch to text mode. */
		__biosSwitchToText();

		/* Release frame buffer. */
		if (dispMemAddr)
		{
			pVmmMgr->VirtualFree(
				(__COMMON_OBJECT*)pVmmMgr,
				dispMemAddr);
			g_pFrameBuffer = NULL;
		}

		return NULL;
	}

	BUG_ON(NULL == dispMemAddr);
	return dispMemAddr;
}
