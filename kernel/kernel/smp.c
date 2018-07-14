//***********************************************************************/
//    Author                    : Garry
//    Original Date             : July 12,2018
//    Module Name               : smp.c
//    Module Funciton           : 
//                                Symmentric Multiple Processor related source code.
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#include <StdAfx.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/* Only available when SMP is enabled. */
#if defined(__CFG_SYS_SMP)
#include "smp.h"

/* Initializer of Processor Manager. */
static BOOL Initialize(__PROCESSOR_MANAGER* pMgr)
{
	BUG_ON(NULL == pMgr);
	pMgr->pDomainList = NULL;
	__INIT_SPIN_LOCK(pMgr->spin_lock);
	return TRUE;
}

/* 
 * Add one processor into system,this routine is invoked in process of system boot,
 * when a new processor(logical CPU) is detected.
 */
static BOOL AddProcessor(uint8_t domainid, uint8_t chipid, uint8_t coreid, uint8_t lcpuid)
{
	unsigned long dwFlags = 0;
	__PROCESSOR_NODE* pNode = NULL;
	__PROCESSOR_NODE* pDomainNode = NULL;
	__PROCESSOR_NODE* pChipNode = NULL;
	__PROCESSOR_NODE* pCoreNode = NULL;
	__PROCESSOR_NODE* pLogicalCPUNode = NULL;
	BOOL bResult = FALSE;

	/* Must be synchronized in SMP environment. */
	__ENTER_CRITICAL_SECTION_SMP(ProcessorManager.spin_lock, dwFlags);
	/* Locate the domain by domainid,create a new domain node if can not located. */
	pDomainNode = ProcessorManager.pDomainList;
	while (pDomainNode)
	{
		/* This level's node type must be DOMAIN. */
		BUG_ON(pDomainNode->nodeType != PROCESSOR_NODE_TYPE_DOMAIN);
		if (domainid == pDomainNode->nodeID)
		{
			/* Found it. */
			break;
		}
		pDomainNode = pDomainNode->pNext;
	}
	/* Not exist,allocate a new one domain. */
	if (NULL == pDomainNode) 
	{
		pDomainNode = (__PROCESSOR_NODE*)_hx_malloc(sizeof(__PROCESSOR_NODE));
		if (NULL == pDomainNode)
		{
			__LEAVE_CRITICAL_SECTION_SMP(ProcessorManager.spin_lock, dwFlags);
			goto __TERMINAL;
		}
		pDomainNode->nodeID = domainid;
		pDomainNode->nodeType = PROCESSOR_NODE_TYPE_DOMAIN;
		pDomainNode->pChildHead = NULL;
		pDomainNode->pChildTail = NULL;
		pDomainNode->pNext = NULL;

		/* Link the domain node into list. */
		pDomainNode->pNext = ProcessorManager.pDomainList;
		ProcessorManager.pDomainList = pDomainNode;
	}

	/* 
	 * Now we located the domainnode,and try to locate the corresponding chip by chipid.
	 * Create a new one and link it into domain's child list if not exist.
	 */
	pChipNode = pDomainNode->pChildHead;
	while (pChipNode)
	{
		BUG_ON(pChipNode->nodeType != PROCESSOR_NODE_TYPE_CHIP);
		if (pChipNode->nodeID == chipid)
		{
			/* Found the corresponding chip node. */
			break;
		}
		pChipNode = pChipNode->pNext;
	}
	if (NULL == pChipNode)
	{
		pChipNode = (__PROCESSOR_NODE*)_hx_malloc(sizeof(__PROCESSOR_NODE));
		if (NULL == pChipNode)
		{
			__LEAVE_CRITICAL_SECTION_SMP(ProcessorManager.spin_lock, dwFlags);
			goto __TERMINAL;
		}
		/* Initialize it and link it into domain's child list. */
		pChipNode->nodeID = chipid;
		pChipNode->nodeType = PROCESSOR_NODE_TYPE_CHIP;
		pChipNode->pChildHead = NULL;
		pChipNode->pChildTail = NULL;
		pChipNode->pNext = NULL;

		if (NULL == pDomainNode->pChildHead)
		{
			/* First child. */
			pDomainNode->pChildHead = pChipNode;
			pDomainNode->pChildTail = pChipNode;
		}
		else
		{
			BUG_ON(NULL == pDomainNode->pChildTail);
			pDomainNode->pChildTail->pNext = pChipNode;
			pDomainNode->pChildTail = pChipNode;
		}
	}

	/* Chip node located,then check the core node using the same process. */
	pCoreNode = pChipNode->pChildHead;
	while (pCoreNode)
	{
		BUG_ON(pCoreNode->nodeType != PROCESSOR_NODE_TYPE_CORE);
		if (pCoreNode->nodeID == coreid)
		{
			break;
		}
		pCoreNode = pCoreNode->pNext;
	}
	if (NULL == pCoreNode)
	{
		pCoreNode = (__PROCESSOR_NODE*)_hx_malloc(sizeof(__PROCESSOR_NODE));
		if (NULL == pCoreNode)
		{
			__LEAVE_CRITICAL_SECTION_SMP(ProcessorManager.spin_lock, dwFlags);
			goto __TERMINAL;
		}
		pCoreNode->nodeID = coreid;
		pCoreNode->nodeType = PROCESSOR_NODE_TYPE_CORE;
		pCoreNode->pChildHead = NULL;
		pCoreNode->pChildTail = NULL;
		pCoreNode->pNext = NULL;

		if (NULL == pChipNode->pChildHead)
		{
			/* First node. */
			pChipNode->pChildHead = pCoreNode;
			pChipNode->pChildTail = pCoreNode;
		}
		else
		{
			BUG_ON(NULL == pChipNode->pChildTail);
			pChipNode->pChildTail->pNext = pCoreNode;
			pChipNode->pChildTail = pCoreNode;
		}
	}

	/* Now logical CPU's turn. */
	pLogicalCPUNode = pCoreNode->pChildHead;
	while (pLogicalCPUNode)
	{
		BUG_ON(pLogicalCPUNode->nodeType != PROCESSOR_NODE_TYPE_LCPU);
		if (pLogicalCPUNode->nodeID == lcpuid)
		{
			break;
		}
		pLogicalCPUNode = pLogicalCPUNode->pNext;
	}
	if (NULL == pLogicalCPUNode)
	{
		pLogicalCPUNode = (__PROCESSOR_NODE*)_hx_malloc(sizeof(__PROCESSOR_NODE));
		if (NULL == pLogicalCPUNode)
		{
			__LEAVE_CRITICAL_SECTION_SMP(ProcessorManager.spin_lock, dwFlags);
			goto __TERMINAL;
		}
		/* Initialize it and link to core node's child list. */
		pLogicalCPUNode->nodeID = lcpuid;
		pLogicalCPUNode->nodeType = PROCESSOR_NODE_TYPE_LCPU;
		pLogicalCPUNode->pChildHead = NULL;
		pLogicalCPUNode->pChildTail = NULL;
		pLogicalCPUNode->pNext = NULL;

		if (NULL == pCoreNode->pChildHead)
		{
			/* First element. */
			pCoreNode->pChildHead = pLogicalCPUNode;
			pCoreNode->pChildTail = pLogicalCPUNode;
		}
		else
		{
			BUG_ON(NULL == pCoreNode->pChildTail);
			pCoreNode->pChildTail->pNext = pLogicalCPUNode;
			pCoreNode->pChildTail = pLogicalCPUNode;
		}
	}
	__LEAVE_CRITICAL_SECTION_SMP(ProcessorManager.spin_lock, dwFlags);

	/* Mark as success. */
	bResult = TRUE;

__TERMINAL:
	return bResult;
}

/* Show out all CPU informatin. */
static VOID ShowCPU(__PROCESSOR_MANAGER* pMgr)
{
	__PROCESSOR_NODE* pDomainNode = NULL;
	__PROCESSOR_NODE* pChipNode = NULL;
	__PROCESSOR_NODE* pCoreNode = NULL;
	__PROCESSOR_NODE* pLogicalCPUNode = NULL;

	BUG_ON(NULL == pMgr);
	pDomainNode = pMgr->pDomainList;
	while (pDomainNode)
	{
		_hx_printf("  domain:%d\r\n", pDomainNode->nodeID);
		pChipNode = pDomainNode->pChildHead;
		while (pChipNode)
		{
			_hx_printf("    chip:%d\r\n", pChipNode->nodeID);
			pCoreNode = pChipNode->pChildHead;
			while (pCoreNode)
			{
				_hx_printf("      core:%d\r\n", pCoreNode->nodeID);
				pLogicalCPUNode = pCoreNode->pChildHead;
				while (pLogicalCPUNode)
				{
					_hx_printf("        logical CPU:%d\r\n", pLogicalCPUNode->nodeID);
					pLogicalCPUNode = pLogicalCPUNode->pNext;
				}
				pCoreNode = pCoreNode->pNext;
			}
			pChipNode = pChipNode->pNext;
		}
		pDomainNode = pDomainNode->pNext;
	}
}

/* The global Processor Manager object. */
__PROCESSOR_MANAGER ProcessorManager = {
	NULL, /* Domain list header. */
	SPIN_LOCK_INIT_VALUE, /* spin_lock. */

	Initialize, /* Initializer. */
	AddProcessor, /* AddProcessor routine. */
	ShowCPU /* ShowCPU routine. */
};

#endif //__CFG_SYS_SMP
