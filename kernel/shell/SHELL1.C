//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Jun 11,2011
//    Module Name               : shell1.cpp
//    Module Funciton           : 
//                                This module countains shell procedures.
//                                Some functions in shell.cpp originally are moved to this file to reduce the
//                                shell.cpp's size.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/
#ifndef __STDAFX_H__
#include "StdAfx.h"
#endif
#include "kapi.h"
#include "shell.h"
#include "string.h"
#include "stdio.h"

//Handler of version command.
VOID VerHandler(__CMD_PARA_OBJ* pCmdParaObj)
{
	/*GotoHome();
	ChangeLine();
	PrintStr(VERSION_INFO);
	GotoHome();
	ChangeLine();
	PrintStr(SLOGAN_INFO);*/
	
	//CD_ChangeLine();
	PrintLine(VERSION_INFO);
	PrintLine(SLOGAN_INFO);
}

//Handler for memory,this routine print out the memory layout and memory usage status.
VOID MemHandler(__CMD_PARA_OBJ* pCmdParaObj)
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
VOID SysInfoHandler(__CMD_PARA_OBJ* pCmdParaObj)
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
	return;
#else   //Only x86 platform is supported yet.
	//GotoHome();
	//ChangeLine();
	CD_PrintString("    This operation can not supported on no-I386 platform.",TRUE);
	
	return;
#endif
}

//Handler for help command.
VOID HlpHandler(__CMD_PARA_OBJ* pCmdParaObj)           //Command 'help' 's handler.
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
	LPSTR strNetApp      = "    network      : Network diagnostic application.";
	LPSTR strLoadappApp  = "    loadapp      : Load application module and execute it.";
	LPSTR strGUIApp      = "    gui          : Load GUI module and enter GUI mode.";
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
	PrintLine(strNetApp);
	PrintLine(strFdiskApp);
	PrintLine(strLoadappApp);
	PrintLine(strGUIApp);
#ifdef __CFG_APP_JVM
	PrintLine(strJvmApp);
#endif //__CFG_APP_JVM
	PrintLine(strReboot);
	PrintLine(strCls);
}

//A helper routine used to load the specified binary application module into memory.
//  @hBinFile       : The handle of the module file;
//  @dwStartAddress : Load address.
//
static BOOL LoadBinModule(HANDLE hBinFile,DWORD dwStartAddress)
{
	BYTE*   pBuffer    = (BYTE*)dwStartAddress;
	BYTE*   pTmpBuff   = NULL;
	DWORD   dwReadSize = 0;
	BOOL    bResult    = FALSE;

	//Parameter check.
	if(dwStartAddress <= 0x00100000)  //End 1M space is reserved.
	{
		goto __TERMINAL;
	}

	//Allocate a temporary buffer to hold file content.
	pTmpBuff = (BYTE*)KMemAlloc(4096,KMEM_SIZE_TYPE_ANY);
	if(NULL == pTmpBuff)
	{
		goto __TERMINAL;
	}
	//Try to read the first 4K bytes from file.
	if(!ReadFile(hBinFile,
		4096,
		pTmpBuff,
		&dwReadSize))
	{
		goto __TERMINAL;
	}
	if(dwReadSize <= 4)  //Too short,invalid format.
	{
		goto __TERMINAL;
	}
	//Verify the validation of the bin file format.
	if(0xE9909090 != *(DWORD*)pTmpBuff)  //Invalid binary file format.
	{
		goto __TERMINAL;
	}
	//Format is ok,try to load it.
	memcpy(pBuffer,pTmpBuff,dwReadSize);  //Copy the first block into target.
	pBuffer += dwReadSize;     //Adjust the target pointer.
	while(dwReadSize == 4096)  //File size larger than 4k,continue to load it.
	{
		if(!ReadFile(hBinFile,
			4096,
			pBuffer,
			&dwReadSize))
		{
			goto __TERMINAL;
		}
		pBuffer += dwReadSize;  //Move target pointer.
	}
	bResult = TRUE;

__TERMINAL:
	if(NULL != pTmpBuff)  //Should release the memory.
	{
		KMemFree(pTmpBuff,KMEM_SIZE_TYPE_ANY,0);
	}
	return bResult;
}

//A helper routine used to launch the loaded binary module.
static BOOL ExecuteBinModule(DWORD dwStartAddress,LPVOID pParams)
{
	__KERNEL_THREAD_OBJECT*   hKernelThread  = NULL;
	BOOL                      bResult        = FALSE;

	if(dwStartAddress <= 0x00100000) //Low end 1M memory is reserved.
	{
		goto __TERMINAL;
	}
	//Create a kernel thread to run the binary module.
	hKernelThread = KernelThreadManager.CreateKernelThread(
		(__COMMON_OBJECT*)&KernelThreadManager,
		0,
		KERNEL_THREAD_STATUS_READY,
		PRIORITY_LEVEL_NORMAL,
		(__KERNEL_THREAD_ROUTINE)dwStartAddress,
		pParams,
		NULL,
		NULL);
	if(NULL == hKernelThread)  //Can not create the thread.
	{
		goto __TERMINAL;
	}
	//Switch input focus to the thread.
	DeviceInputManager.SetFocusThread(
		(__COMMON_OBJECT*)&DeviceInputManager,
		(__COMMON_OBJECT*)hKernelThread);
	hKernelThread->WaitForThisObject((__COMMON_OBJECT*)hKernelThread);  //Block shell to wait module over.
	//Destroy the module's kernel thread.
	KernelThreadManager.DestroyKernelThread(
		(__COMMON_OBJECT*)&KernelThreadManager,
		(__COMMON_OBJECT*)hKernelThread);
	//Switch back input focus to shell.
	DeviceInputManager.SetFocusThread(
		(__COMMON_OBJECT*)&DeviceInputManager,
		NULL);
	bResult = TRUE;

__TERMINAL:
	return bResult;
}

//Handler for loadapp command.
VOID LoadappHandler(__CMD_PARA_OBJ* pCmdParaObj)
{		
	CHAR      FullPathName[128]  = {0};  //Full name of binary file.
	DWORD     dwStartAddr        = 0;       //Load address of the module.
	HANDLE    hBinFile           = NULL;
	

	if(pCmdParaObj->byParameterNum < 2)
	{
		PrintLine("Please specify both app module name and load address.");
		goto __TERMINAL;
	}

	if((0 == pCmdParaObj->Parameter[0][0]) || (0 == pCmdParaObj->Parameter[1][0]))
	{
		PrintLine("Invalid parameter(s).");
		goto __TERMINAL;
	}

	//Construct the full path and name.
	strcpy(FullPathName,"C:\\PTHOUSE\\");
	strcat(FullPathName,pCmdParaObj->Parameter[0]);
	if(!Str2Hex(pCmdParaObj->Parameter[1],&dwStartAddr))
	{
		PrintLine("Invalid load address.");
		goto __TERMINAL;
	}
	//Try to open the binary file.
	hBinFile = CreateFile(
		FullPathName,
		FILE_ACCESS_READ,
		0,
		NULL);
	if(NULL == hBinFile)
	{
		PrintLine("Can not open the specified file in OS root directory.");
		goto __TERMINAL;
	}
	//Try to load and execute it.
	if(!LoadBinModule(hBinFile,dwStartAddr))
	{
		PrintLine("Can not load the specified binary file.");
		goto __TERMINAL;
	}
	if(!ExecuteBinModule(dwStartAddr,NULL))
	{
		PrintLine("Can not execute the binary module.");
		goto __TERMINAL;
	}
__TERMINAL:

	if(NULL != hBinFile)  //Destroy it.
	{
		CloseFile(hBinFile);
	}
		
	return;
}

//Handler for GUI command,it only call LoadappHandler by given
//the GUI module's name and it's start address after loaded into
//memory.
VOID GUIHandler(__CMD_PARA_OBJ* pCmdParaObj)
{
	HANDLE	hBinFile = NULL;
	CHAR    FullPathName[64];  //Full name of binary file.
	DWORD	dwStartAddr = 0x170000;

	strcpy(FullPathName, "C:\\PTHOUSE\\hcngui.bin");
	//Try to open the binary file.
	hBinFile = CreateFile(
		FullPathName,
		FILE_ACCESS_READ,
		0,
		NULL);
	if(NULL == hBinFile)
	{
		PrintLine("Can not open the specified file in OS root directory.");
		goto __TERMINAL;
	}
	//Try to load and execute it.
	if(!LoadBinModule(hBinFile,dwStartAddr))
	{
		PrintLine("Can not load the specified binary file.");
		goto __TERMINAL;
	}
	if(!ExecuteBinModule(dwStartAddr,NULL))
	{
		PrintLine("Can not execute the binary module.");
		goto __TERMINAL;
	}

__TERMINAL:
	if(NULL != hBinFile)  //Destroy it.
	{
		CloseFile(hBinFile);
	}
}
