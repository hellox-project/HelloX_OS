//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Jun 11,2011
//    Module Name               : shell1.cpp
//    Module Funciton           : 
//                                This module countains shell procedures.
//                                Some functions in shell.cpp originally are moved to this file to reduce the
//                                shell.cpp's size.
//    Last modified Author      :tywind
//    Last modified Date        :Aug 25,2015
//    Last modified Content     :modify loadapp and gui route 
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#include "StdAfx.h"
#include "kapi.h"
#include "shell.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include "../appldr/AppLoader.h"


//Handler of version command.
DWORD VerHandler(__CMD_PARA_OBJ* pCmdParaObj)
{
	PrintLine(HELLOX_VERSION_INFO);
	PrintLine(HELLOX_SLOGAN_INFO);
	PrintLine(HELLOX_SPECIAL_INFO);

	return S_OK;
}

//Handler for memory,this routine print out the memory layout and memory usage status.
DWORD  MemHandler(__CMD_PARA_OBJ* pCmdParaObj)
{
	CHAR   buff[256];
	DWORD  dwFlags;

	DWORD  dwPoolSize;
	DWORD  dwFreeSize;
	DWORD  dwFreeBlocks;
	DWORD  dwAllocTimesSuccL;
	DWORD  dwAllocTimesSuccH;
	DWORD  dwAllocTimesL;
	DWORD  dwAllocTimesH;
	DWORD  dwFreeTimesL;
	DWORD  dwFreeTimesH;

	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	dwPoolSize      = AnySizeBuffer.dwPoolSize;
	dwFreeSize      = AnySizeBuffer.dwFreeSize;
	dwFreeBlocks    = AnySizeBuffer.dwFreeBlocks;
	dwAllocTimesSuccL  = AnySizeBuffer.dwAllocTimesSuccL;
	dwAllocTimesSuccH  = AnySizeBuffer.dwAllocTimesSuccH;
	dwAllocTimesL      = AnySizeBuffer.dwAllocTimesL;
	dwAllocTimesH      = AnySizeBuffer.dwAllocTimesH;
	dwFreeTimesL       = AnySizeBuffer.dwFreeTimesL;
	dwFreeTimesH       = AnySizeBuffer.dwFreeTimesH;
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);

	PrintLine("    Free block list algorithm is adopted:");
	//Get and dump out memory usage status.
	_hx_sprintf(buff,"    Total memory size     : %d(0x%X)",dwPoolSize,dwPoolSize);
	PrintLine(buff);
	_hx_sprintf(buff,"    Free memory size      : %d(0x%X)",dwFreeSize,dwFreeSize);
	PrintLine(buff);
	_hx_sprintf(buff,"    Free memory blocks    : %d",dwFreeBlocks);
	PrintLine(buff);
	_hx_sprintf(buff,"    Alloc success times   : %d/%d",dwAllocTimesSuccH,dwAllocTimesSuccL);
	PrintLine(buff);
	_hx_sprintf(buff,"    Alloc operation times : %d/%d",dwAllocTimesH,dwAllocTimesL);
	PrintLine(buff);
	_hx_sprintf(buff,"    Free operation times  : %d/%d",dwFreeTimesH,dwFreeTimesL);
	PrintLine(buff);

	return S_OK;
}

//Local variables for sysinfo command.
LPSTR strHdr[] = {               //I have put the defination of this strings
	                             //in the function SysInfoHandler,but it do
	                             //not work,see the asm code,it generates the
	                             //incorrect asm code!Fuck Bill Gates!.
	"    EDI   :   0x",
	"    ESI   :   0x",
	"    EBP   :   0x",
	"    ESP   :   0x",
	"    EBX   :   0x",
	"    EDX   :   0x",
	"    ECX   :   0x",
	"    EAX   :   0x",
	"    CS-DS :   0x",
	"    FS-GS :   0x",
	"    ES-SS :   0x"};

//#ifdef __I386__
//static CHAR Buffer[] = {"Hello,China!"};
//#endif

//Handler for sysinfo command.
DWORD SysInfoHandler(__CMD_PARA_OBJ* pCmdParaObj)
{
#ifdef __I386__
	DWORD sysContext[11] = {0};
	DWORD bt;

#ifdef __GCC__
	asm __volatile__ (
			".code32			;"
			"pusha				;"
			"pushl	%%eax		;"
			"movl	0x04(%%esp),	%%eax	;"
			"movl	%%eax,		-0x30(%%ebp);"
			"movl	0x08(%%esp),	%%eax	;"
			"movl	%%eax,		-0x2c(%%ebp);"
			"movl	0x0c(%%esp),	%%eax	;"
			"movl	%%eax,	-0x28(%%ebp)	;"
			"movl	0x10(%%esp),	%%eax	;"
			"movl	%%eax,		-0x24(%%ebp);"
			"movl	0x14(%%esp),	%%eax	;"
			"movl	%%eax,			-0x20(%%ebp);"
			"movl	0x18(%%esp),		%%eax	;"
			"movl	%%eax,			-0x1c(%%ebp);"
			"movl	0x1c(%%esp),	%%eax		;"
			"movl	%%eax,		-0x18(%%ebp)	;"
			"movl	0x20(%%esp),	%%eax		;"
			"movl	%%eax,	-0x14(%%ebp)		;"

			"movw	%%cs,	%%ax	;"
			"shll	$0x10,	%%eax	;"
			"movw	%%ds,	%%ax	;"
			"movl	%%eax,	-0x10(%%ebp)	;"
			"movw	%%fs, 	%%ax			;"
			"shll	$0x10,	%%eax			;"
			"movw	%%gs,	%%ax			;"
			"movl	%%eax,	-0x0c(%%ebp)	;"
			"movw	%%es,	%%ax			;"
			"shll	$0x10,	%%eax			;"
			"movw	%%ss,	%%ax			;"
			"movl	%%eax,	-0x08(%%ebp)	;"

			"popl	%%eax					;"
			"popa							;"
			::
	);
#else


	__asm{                       //Get the system information.
		pushad                   //Save all the general registers.
			                     //NOTICE: This operation only get
								 //the current status of system
								 //where this instruction is executed.
        push eax
        mov eax,dword ptr [esp + 0x04]
		mov dword ptr [ebp - 0x30],eax    //Get the eax register's value.
		                                  //Fuck Bill Gates!!!!!
		mov eax,dword ptr [esp + 0x08]
		mov dword ptr [ebp - 0x2c],eax    //Get the ecx value.
		mov eax,dword ptr [esp + 0x0c]
		mov dword ptr [ebp - 0x28],eax    //edx
		mov eax,dword ptr [esp + 0x10]
		mov dword ptr [ebp - 0x24],eax    //ebx
		mov eax,dword ptr [esp + 0x14]
		mov dword ptr [ebp - 0x20],eax    //esp
		mov eax,dword ptr [esp + 0x18]
		mov dword ptr [ebp - 0x1c],eax    //ebp
		mov eax,dword ptr [esp + 0x1c]
		mov dword ptr [ebp - 0x18],eax    //esi
		mov eax,dword ptr [esp + 0x20]
		mov dword ptr [ebp - 0x14],eax    //edi

		mov ax,cs
		shl eax,0x10
		mov ax,ds
		mov dword ptr [ebp - 0x10],eax    //Get cs : ds.
		mov ax,fs
		shl eax,0x10
		mov ax,gs
		mov dword ptr [ebp - 0x0c],eax    //Get fs : gs.
		mov ax,es
		shl eax,0x10
		mov ax,ss
		mov dword ptr [ebp - 0x08],eax   //Get es : ss.

		pop eax
		popad                    //Restore the stack frame.
	}
#endif
	//All system registers are got,then print out them.
   /*GotoHome();
	ChangeLine();
	PrintStr("    System context information(general registers and segment registers):");*/

	CD_ChangeLine();
	CD_PrintString("    System context information(general registers and segment registers):",TRUE);
	for(bt = 0;bt < 11;bt ++)
	{
		CHAR szTemp[64] = {0};

		CD_PrintString(strHdr[bt],FALSE);		
		Hex2Str(sysContext[bt],szTemp);
		CD_PrintString(szTemp,TRUE);	
	}
	return S_OK;
#else   //Only x86 platform is supported yet.
	//GotoHome();
	//ChangeLine();
	CD_PrintString("    This operation can not supported on no-I386 platform.",TRUE);
	
	return S_OK;
#endif
}

//Handler for help command.
DWORD HlpHandler(__CMD_PARA_OBJ* pCmdParaObj)           //Command 'help' 's handler.
{
	LPSTR strHelpTitle   = "    The following commands are available currently:";
	LPSTR strHelpVer     = "    version      : Print out the version information.";
	LPSTR strHelpMem     = "    memory       : Print out current version's memory layout.";
	LPSTR strHelpSysInfo = "    sysinfo      : Print out the system context.";
	LPSTR strSysName     = "    sysname      : Change the system host name.";
	LPSTR strHelpHelp    = "    help         : Print out this screen.";
	LPSTR strSupport     = "    support      : Print out technical support information.";
	LPSTR strTime        = "    time         : Show system date and time.";
	LPSTR strRunTime     = "    runtime      : Display the total run time since last reboot.";
	LPSTR strIoCtrlApp   = "    ioctrl       : Start IO control application.";
	LPSTR strSysDiagApp  = "    sysdiag      : System or hardware diag application.";
	LPSTR strFsApp       = "    fs           : File system operating application.";
	LPSTR strFdiskApp    = "    fdisk        : Hard disk operating application.";
	LPSTR strUsbVideo    = "    usbvideo     : USB video operations.";
	LPSTR strNetApp      = "    network      : Network diagnostic application.";
	LPSTR strLoadappApp  = "    loadapp      : Load application module and execute it.";
	LPSTR strGUIApp      = "    gui          : Load GUI module and enter GUI mode.";
#ifdef __CFG_APP_SSH
	LPSTR strSSH         = "    ssh          : Start a new SSH session.";
#endif

#ifdef __CFG_APP_JVM
	LPSTR strJvmApp      = "    jvm          : Start Java VM to run Java Application.";
#endif  //__CFG_APP_JVM
	LPSTR strReboot      = "    reboot       : Reboot the system.";
	LPSTR strCls         = "    cls          : Clear the whole screen.";

	PrintLine(strHelpTitle);              //Print out the help information line by line.
	PrintLine(strHelpVer);
	PrintLine(strHelpMem);
	PrintLine(strHelpSysInfo);
	PrintLine(strSysName);
	PrintLine(strHelpHelp);
	PrintLine(strSupport);
	PrintLine(strTime);
	PrintLine(strRunTime);
	PrintLine(strIoCtrlApp);
	PrintLine(strSysDiagApp);
	PrintLine(strFsApp);
	PrintLine(strUsbVideo);
	PrintLine(strNetApp);
	PrintLine(strFdiskApp);
	PrintLine(strLoadappApp);
	PrintLine(strGUIApp);
#ifdef __CFG_APP_SSH
	PrintLine(strSSH);
#endif
#ifdef __CFG_APP_JVM
	PrintLine(strJvmApp);
#endif //__CFG_APP_JVM
	PrintLine(strReboot);
	PrintLine(strCls);

	return S_OK;
}

//Handler for loadapp command.
DWORD LoadappHandler(__CMD_PARA_OBJ* pCmdParaObj)
{	
	__CMD_PARA_OBJ* pAppParaObj        = NULL;
	CHAR            FullPathName[128]  = {0};  //Full name of binary file.
	BYTE            i                  = 0; 


	if(pCmdParaObj->byParameterNum < 2)
	{
		PrintLine("Please specify app module's name.");
		goto __TERMINAL;
	}
	
	//Construct the full path and name.
	strcpy(FullPathName,"C:\\PTHOUSE\\");
	strcat(FullPathName,pCmdParaObj->Parameter[1]);
	
	//copy params
	pAppParaObj = CopyParameterObj(pCmdParaObj,1);
		
	RunDynamicAppModule(FullPathName,pAppParaObj);
	
__TERMINAL:

	ReleaseParameterObj(pAppParaObj);
		
	return S_OK;
}

DWORD RunBatCmdLine(LPSTR pCmdLine)
{
	__CMD_PARA_OBJ* pAppParaObj  = NULL;
	
	ClearUnVisableChar(pCmdLine);

	pAppParaObj = FormParameterObj(pCmdLine);

	//exec app
	if(pAppParaObj->byParameterNum >= 2)
	{
		LoadappHandler(pAppParaObj);
	}

	ReleaseParameterObj(pAppParaObj);		
		
	return S_OK;
}

//Handler for Bat command.
DWORD BatHandler(__CMD_PARA_OBJ* pCmdParaObj)
{	
	HANDLE	hBatFile        = NULL;
	DWORD   dwFileSize      = 0;
	DWORD   dwReadSize      = 0;
	LPSTR   pShortName      = NULL;
	CHAR    szBatFile[128]  = {0};
	LPSTR   pBatBuf         = NULL;
	LPSTR   pCurrPos         = NULL;
	LPSTR   pNextPos         = NULL;
	DWORD   i                = 0;

	//Construct the bat full path and name.
	strcpy(szBatFile,"C:\\PTHOUSE\\");
	pShortName = strstr(pCmdParaObj->Parameter[0],"./");
	if(!pShortName)
	{
		PrintLine("error Batfile name.");
		goto __TERMINAL;
	}

	pShortName  += strlen("./");
	strcat(szBatFile,pShortName);
	
	hBatFile = CreateFile(szBatFile,FILE_ACCESS_READ,0,NULL);
	if(hBatFile == NULL)
	{		
		_hx_printf("Batfile open  error =%s \r\n",szBatFile);
		goto __TERMINAL;	
	}

	dwFileSize = GetFileSize(hBatFile,NULL);
	pBatBuf    = (LPSTR)_hx_malloc(dwFileSize+1);
	ReadFile(hBatFile,dwFileSize,pBatBuf,NULL);

	pCurrPos = pBatBuf;
	pNextPos = strstr(pCurrPos,"\n");

	while(pNextPos)
	{					
		*pNextPos = 0; 
				
		RunBatCmdLine(pCurrPos);
		
		pNextPos ++;
		pCurrPos = pNextPos;
		pNextPos = strstr(pCurrPos,"\n");		
	}
	
	if(pCurrPos)
	{
		RunBatCmdLine(pCurrPos);
	}

__TERMINAL:

	if(pBatBuf)
	{
		_hx_free(pBatBuf);
	}

	CloseFile(hBatFile);	

	return S_OK;
}

//Handler for GUI command,it only call LoadappHandler by given
//the GUI module's name and it's start address after loaded into
//memory.
DWORD GUIHandler(__CMD_PARA_OBJ* pCmdParaObj)
{
	//Set current mode indicator to GRAPHIC.
	System.ucReserved1 = 1;
	RunDynamicAppModule("C:\\PTHOUSE\\hcngui.dll",pCmdParaObj);
	//Restore current display mode to TEXT.
	System.ucReserved1 = 0;
	return S_OK;
}
