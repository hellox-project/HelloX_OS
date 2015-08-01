//***********************************************************************/
//    Author                    : tywind
//    Original Date             : oct,15 2014
//    Module Name               : HITCMD.c
//    Module Funciton           : 
//                                This module countains shell history routes
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __STDAFX_H__
#include <StdAfx.h>
#endif

#include "shell.h"
#include "fs.h"

#include "kapi.h"
#include "string.h"
#include "stdio.h"

typedef struct tag__HISTORY_CMD_INFO
{
	HIS_CMD_OBJ*  pHisCmdArry;	
	INT           nHisCount;
	INT           nSaveCount;	
	INT           nExecHisPos;   

}HISTORY_CMD_INFO;

HISOBJ His_CreateHisObj(INT nHisCount)
{
	HISTORY_CMD_INFO*  pHisInfo = NULL;

	if(nHisCount <= 0)
	{
		return NULL;
	}
	pHisInfo = (HISTORY_CMD_INFO*)KMemAlloc(sizeof(HISTORY_CMD_INFO),KMEM_SIZE_TYPE_ANY);
	if(NULL == pHisInfo)
	{
		return NULL;
	}

	pHisInfo->nHisCount     = nHisCount;
	pHisInfo->nSaveCount    = 0;		
	pHisInfo->nExecHisPos   = -1;
	pHisInfo->pHisCmdArry   = (HIS_CMD_OBJ*)KMemAlloc(sizeof(HIS_CMD_OBJ)*nHisCount,KMEM_SIZE_TYPE_ANY);

	return (HISOBJ)pHisInfo;
}

VOID His_DeleteHisObj(HISOBJ hHisObj)
{
	HISTORY_CMD_INFO*  pHisInfo = (HISTORY_CMD_INFO*)hHisObj;

	if(pHisInfo)
	{
		KMemFree(pHisInfo->pHisCmdArry,KMEM_SIZE_TYPE_ANY,0);
		KMemFree(pHisInfo,KMEM_SIZE_TYPE_ANY,0);
	}
}

//save curcmd to history list
BOOL His_SaveCmd(HISOBJ hHisObj,LPCSTR pCmdStr)
{	
	HISTORY_CMD_INFO*  pHisInfo = (HISTORY_CMD_INFO*)hHisObj;
	INT                nSavePos = 0;
	
	if(NULL == pHisInfo)
	{
		return FALSE;
	}
	
	//判断和最新的历史纪录是否重复
	if(pHisInfo->nSaveCount >0 && StrCmp((LPSTR)pCmdStr,pHisInfo->pHisCmdArry[pHisInfo->nSaveCount-1].CmdStr))
	{
		pHisInfo->nExecHisPos = -1;
		return FALSE;
	}

	if(pHisInfo->nSaveCount >= pHisInfo->nHisCount)
	{
		INT  i = 0;

		//移动指令序列，清除最早的那一个
		for(i=0;i < pHisInfo->nSaveCount-1;i++)
		{
			pHisInfo->pHisCmdArry[i] = pHisInfo->pHisCmdArry[i+1];      
		}	
		nSavePos  = pHisInfo->nSaveCount-1;		
	}
	else
	{		
		nSavePos  = pHisInfo->nSaveCount;		
	}

	StrCpy((LPSTR)pCmdStr,(LPSTR)pHisInfo->pHisCmdArry[nSavePos].CmdStr);	
	if(pHisInfo->nSaveCount  < pHisInfo->nHisCount)
	{
		pHisInfo->nSaveCount ++;
	}

	pHisInfo->nExecHisPos = -1;

	return TRUE;
}

//load history cmd to current line
BOOL His_LoadHisCmd(HISOBJ hHisObj,BOOL bUp,LPSTR pCmdBuf,INT nBufLen)
{
	HISTORY_CMD_INFO*  pHisInfo = (HISTORY_CMD_INFO*)hHisObj;	
	INT                nInc     = 0;


	if(NULL == pHisInfo)
	{
		return FALSE;
	}

	if(pHisInfo->nExecHisPos  == -1)
	{
		pHisInfo->nExecHisPos  = pHisInfo->nSaveCount;	
	}

	if(bUp == TRUE)	
	{
		if(pHisInfo->nExecHisPos <= 0)
		{
			return FALSE;
		}
		nInc = -1;
	}
	else
	{		
		if(pHisInfo->nExecHisPos >= pHisInfo->nSaveCount -1)
		{
			return FALSE;
		}
		nInc = 1;
	}

	pHisInfo->nExecHisPos += nInc;

	StrCpy(pHisInfo->pHisCmdArry[pHisInfo->nExecHisPos].CmdStr,pCmdBuf);

	return TRUE;
}
