//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Jun,10 2005
//    Module Name               : IOCTRL_S.CPP
//    Module Funciton           : 
//                                This module countains io control application's implemen-
//                                tation code.
//                                This file countains the shell command or application
//                                code,as it's name has ended by "_S".
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

#include "ioctrl_s.h"

#define  IOCTRL_PROMPT_STR   "[ioctrl_view]"
//
//The IO control application is used to input or output data to or from IO port.
//This application is very powerful,it can support input/output one byte,one word,
//one byte string or one word string from/into device port.
//The following are it's usage(# is the prompt of this application):
// #inputb io_port    //Input a byte from port,which is indicated by io_port.
// #inputw io_port    //Input a word from port,which is indicated by io_port.
// #inputsb io_port str_len  //Input a byte string from port,which is indicated by io_port,
//                           //str_len is the number of bytes to be inputed.
// #inputsw io_port str_len  //The same as inputsb.
// #outputb io_port   //Output a byte to port.
// #outputw io_port   //Output a word to port.
// #outputsb io_port str_len //Output a byte string to port.
// #outputsw io_port str_len //Output a word string to port.
// #memwb mem_addr data      //Write a byte to the memory address 'mem_addr'.
// #memww mem_addr data      //Write a word to the memory address 'mem_addr'.
// #memwd mem_addr data      //Write a dword to the memory.
// #memrb mem_addr           //Read a byte from memory.
// #memrw mem_addr           //Read a word from memory.
// #memrd mem_addr           //Read a dword from memory.
// #exit                     //Exit this application.
// #help                     //Print help information about this application.
//

//
//IO command handler routine's definition.
//Their implementations can be found at below of this file.
//

static DWORD inputb(__CMD_PARA_OBJ*);
static DWORD inputw(__CMD_PARA_OBJ*);
static DWORD inputd(__CMD_PARA_OBJ*);
static DWORD inputsb(__CMD_PARA_OBJ*);
static DWORD inputsw(__CMD_PARA_OBJ*);

static DWORD outputb(__CMD_PARA_OBJ*);
static DWORD outputw(__CMD_PARA_OBJ*);
static DWORD outputd(__CMD_PARA_OBJ*);
static DWORD outputsb(__CMD_PARA_OBJ*);
static DWORD outputsw(__CMD_PARA_OBJ*);

static DWORD memwb(__CMD_PARA_OBJ*);
static DWORD memww(__CMD_PARA_OBJ*);
static DWORD memwd(__CMD_PARA_OBJ*);

static DWORD memrb(__CMD_PARA_OBJ*);
static DWORD memrw(__CMD_PARA_OBJ*);
static DWORD memrd(__CMD_PARA_OBJ*);

static DWORD memalloc(__CMD_PARA_OBJ*);
static DWORD memrels(__CMD_PARA_OBJ*);
static DWORD help(__CMD_PARA_OBJ*);
static DWORD exit(__CMD_PARA_OBJ*);

//
//The following is a map between IO control command and it's handler.
//IO control application will lookup this map to find appropriate handler by command
//name,and call the handler to handle this IO command.
//

static struct __IOCTRL_COMMAND_MAP{
	LPSTR              lpszCommand;                        //Command string.
	DWORD              (*CommandRoutine)(__CMD_PARA_OBJ*); //Handler.
	LPSTR              lpszHelpInfo;                       //Help information string.
}IOCtrlCmdMap[] = {
	{"inputb",    inputb,      "  inputb io_port             : Input a byte from IO port."},
	{"inputw",    inputw,      "  inputw io_port             : Input a word from IO port."},
	{"inputd",    inputd,      "  inputd io_port             : Input a dword from IO port."},
	{"inputsb",   inputsb,     "  inputsb io_port            : Input a byte string from IO port."},
	{"inputsw",   inputsw,     "  inputsw io_port            : Input a word string from IO port."},
	{"outputb",   outputb,     "  outputb io_port val        : Output a byte to IO port."},
	{"outputw",   outputw,     "  outputw io_port val        : Output a word to IO port."},
	{"outputd",   outputd,     "  outputd io_port val        : Output a dword to IO port."},
	{"outputsb",  outputsb,    "  outputsb io_port size addr : Output a byte string to IO port."},
	{"outputsw",  outputsw,    "  outputsw io_port size addr : Output a word string to IO port."},
	{"memwb",     memwb,       "  memwb addr val             : Write a byte to memory location."},
	{"memww",     memww,       "  memww addr val             : Write a word to memory location."},
	{"memwd",     memwd,       "  memwd addr val             : Write a dword to memory location."},
	{"memrb",     memrb,       "  memrb addr                 : Read a byte from memory."},
	{"memrw",     memrw,       "  memrw addr                 : Read a word from memory."},
	{"memrd",     memrd,       "  memrd addr                 : Read a dword from memory."},
	{"memalloc",  memalloc,    "  memalloc                   : Allocate a block of memory."},
	{"memrels",   memrels,     "  memrels addr               : Release the memory allocated by memalloc."},
	{"help",      help,        "  help                       : Print out this screen."},
	{"exit",      exit,        "  exit                       : Exit from this application."},
	{NULL,        NULL,        NULL}                 //End of the map.
};

//
//The following is the entry point of IO control application.
//This routine does the following:
// 1. Get input message from kernel thread message queue,by calling GetMessage;
// 2. If the message is a key down event,and the key is ENTRER,then process the command;
// 3. Form a command parameter object by calling FormParameterObj;
// 4. Lookup the IO command and handler's map,to find appropriate handler;
// 5. Call the handler,command parameter as the handler's parameter;
// 6. If the hander returns SHELL_CMD_PARSER_FAILED,then exit the application;
// 7. Otherwise,continue the loop.
//

static DWORD CommandParser(LPCSTR lpszCmdLine)
{
	__CMD_PARA_OBJ*   lpParamObj   = NULL;
	DWORD             dwRetVal     = SHELL_CMD_PARSER_INVALID;
	DWORD             dwIndex      = 0;
	BOOL              bValidCmd    = FALSE;

	lpParamObj = FormParameterObj(lpszCmdLine);
	if(NULL == lpParamObj)    //Can not create parameter object.
	{		
		return SHELL_CMD_PARSER_FAILED;
	}

	if(0 != lpParamObj->byParameterNum)  //Valid parameter(s) exits.
	{
		while(IOCtrlCmdMap[dwIndex].lpszCommand)
		{
			if(StrCmp(IOCtrlCmdMap[dwIndex].lpszCommand,
				lpParamObj->Parameter[0]))
			{
				bValidCmd = TRUE;    //Find the valid command.
				break;
			}
			dwIndex ++;
		}

		if(bValidCmd)  //Handle the command.
		{
			dwRetVal = IOCtrlCmdMap[dwIndex].CommandRoutine(lpParamObj);
		}
		else
		{
			dwRetVal = SHELL_CMD_PARSER_INVALID;			
		}
	}

	ReleaseParameterObj(lpParamObj);  //Release the parameter object.

	return dwRetVal;
}


static DWORD QueryCmdName(LPSTR pMatchBuf,INT nBufLen)
{
	static DWORD dwIndex = 0;

	if(pMatchBuf == NULL)
	{
		dwIndex    = 0;	
		return SHELL_QUERY_CONTINUE;
	}


	if(NULL == IOCtrlCmdMap[dwIndex].lpszCommand)
	{
		dwIndex = 0;
		return SHELL_QUERY_CANCEL;	
	}

	strncpy(pMatchBuf,IOCtrlCmdMap[dwIndex].lpszCommand,nBufLen);
	dwIndex ++;

	return SHELL_QUERY_CONTINUE;	
}

//static CHAR strCmdBuffer[MAX_BUFFER_LEN] = {0};  //Command buffer.
DWORD IoCtrlStart(LPVOID p)
{
	return Shell_Msg_Loop(IOCTRL_PROMPT_STR,CommandParser,QueryCmdName);	

}

//
//The following are the implementations of all command handler.
//The handler's name is the same as command.
//
static DWORD inputb(__CMD_PARA_OBJ* lpCmdObj)
{
	WORD           wInputPort         = 0;
	UCHAR          bt                 = 0;
	DWORD          dwInputPort        = 0;
	CHAR           strBuffer[15]      = {0};

	if(NULL == lpCmdObj)  //Parameter check.
	{
		return SHELL_CMD_PARSER_FAILED;
	}

	if(lpCmdObj->byParameterNum < 2)  //Two small parameters.
	{
		PrintLine("Please input the port where to read.");
		return SHELL_CMD_PARSER_SUCCESS;
	}
	if(!Str2Hex(lpCmdObj->Parameter[1],&dwInputPort))  //Convert the string value to hex.
	{
		PrintLine("Invalid port value.");
		return SHELL_CMD_PARSER_SUCCESS;
	}

	wInputPort = (WORD)dwInputPort;  //Now,wInputPort contains the port number.
	//ReadByteFromPort(&bt,wInputPort);  //Read one byte from port.
	bt = __inb(wInputPort);

	dwInputPort =  0;
	dwInputPort += bt;

	strBuffer[0] = ' ';
	strBuffer[1] = ' ';
	strBuffer[2] = ' ';
	strBuffer[3] = ' ';

	Hex2Str(dwInputPort,&strBuffer[4]);
	PrintLine(strBuffer);    //Print out the byte.

	return SHELL_CMD_PARSER_SUCCESS;
}

static DWORD inputw(__CMD_PARA_OBJ* lpCmdObj)
{
	WORD           wInputPort         = 0;
	WORD           wr                 = 0;
	DWORD          dwInputPort        = 0;
	CHAR           strBuffer[15]      = {0};

	if(NULL == lpCmdObj)  //Parameter check.
		return SHELL_CMD_PARSER_FAILED;

	if(lpCmdObj->byParameterNum < 2)  //Two small parameters.
	{
		PrintLine("Please input the port where to read.");
		return SHELL_CMD_PARSER_SUCCESS;
	}
	if(!Str2Hex(lpCmdObj->Parameter[1],&dwInputPort))  //Convert the string value to hex.
	{
		PrintLine("Invalid port value.");
		return SHELL_CMD_PARSER_SUCCESS;
	}

	wInputPort = (WORD)(dwInputPort);  //Now,wInputPort contains the port number.
	//ReadWordFromPort(&wr,wInputPort);  //Read one byte from port.
	wr = __inw(wInputPort);

	dwInputPort =  0;
	dwInputPort += wr;

	strBuffer[0] = ' ';
	strBuffer[1] = ' ';
	strBuffer[2] = ' ';
	strBuffer[3] = ' ';

	Hex2Str(dwInputPort,&strBuffer[4]);
	PrintLine(strBuffer);    //Print out the byte.

	return SHELL_CMD_PARSER_SUCCESS;
}

//
//The implementation of inputd.
//

static DWORD inputd(__CMD_PARA_OBJ* lpParamObj)
{
	DWORD                dwVal              = 0;
	WORD                 wPort              = 0;
	CHAR                 strBuffer[15];

	if(NULL == lpParamObj)    //Parameter check.
		return SHELL_CMD_PARSER_FAILED;

	if(lpParamObj->byParameterNum < 2)    //Not enough parameters.
	{
		PrintLine("Please input the port value.");
		return SHELL_CMD_PARSER_SUCCESS;
	}

	if(!Str2Hex(lpParamObj->Parameter[1],&dwVal))  //Incorrect port value.
	{
		PrintLine("Please input the port correctly.");
		return SHELL_CMD_PARSER_SUCCESS;
	}

	wPort = (WORD)(dwVal);

#ifdef __I386__               //Read data from port.
#ifdef __GCC__
	__asm__(
			".code32			\n\t"
			"pushl	%%eax		\n\t"
			"pushl	%%edx		\n\t"
			"movw	%0,	%%dx	\n\t"
			"inw	%%dx,	%%ax	\n\t"
			"popl	%%edx			\n\t"
			"popl	%%eax			\n\t"
			::"r"(wPort)
		);
#else
	__asm{
		push eax
		push edx
		mov dx,wPort
		in eax,dx
		mov dwVal,eax
		pop edx
		pop eax
	}
#endif
#else
#endif

	strBuffer[0] = ' ';
	strBuffer[1] = ' ';
	strBuffer[2] = ' ';
	strBuffer[3] = ' ';

	Hex2Str(dwVal,&strBuffer[4]);
	PrintLine(strBuffer);    //Print out the byte.

	return SHELL_CMD_PARSER_SUCCESS;
}

static DWORD inputsb(__CMD_PARA_OBJ* lpCmdObj)
{
	PrintLine("inputsb is handled.");

	return SHELL_CMD_PARSER_SUCCESS;
}

static DWORD inputsw(__CMD_PARA_OBJ* lpCmdObj)
{
	return SHELL_CMD_PARSER_SUCCESS;
}

static DWORD outputb(__CMD_PARA_OBJ* lpCmdObj)
{
	UCHAR            bt         = 0;
	WORD             wPort      = 0;
	DWORD            dwPort     = 0;

	if(NULL == lpCmdObj)  //Parameter check.
		return SHELL_CMD_PARSER_FAILED;

	if(lpCmdObj->byParameterNum < 3)  //Not enough parameters.
	{
		PrintLine("Please input the port and value.");
		return SHELL_CMD_PARSER_SUCCESS;
	}
	if(!Str2Hex(lpCmdObj->Parameter[1],&dwPort))
	{
		PrintLine("Please input port number correctly.");
		return SHELL_CMD_PARSER_SUCCESS;
	}
	wPort = (WORD)(dwPort);    //Now,wPort contains the port where to output.
	if(!Str2Hex(lpCmdObj->Parameter[2],&dwPort))
	{
		PrintLine("Please input the value to output correctly.");
		return SHELL_CMD_PARSER_SUCCESS;
	}
	bt = (BYTE)(dwPort);

	//WriteByteToPort(bt,wPort);    //Write the byte to port.
	__outb(bt,wPort);
	return SHELL_CMD_PARSER_SUCCESS;
}

static DWORD outputw(__CMD_PARA_OBJ* lpCmdObj)
{
	WORD             wr         = 0;
	WORD             wPort      = 0;
	DWORD            dwPort     = 0;

	if(NULL == lpCmdObj)  //Parameter check.
		return SHELL_CMD_PARSER_FAILED;

	if(lpCmdObj->byParameterNum < 3)  //Not enough parameters.
	{
		PrintLine("Please input the port and value.");
		return SHELL_CMD_PARSER_SUCCESS;
	}
	if(!Str2Hex(lpCmdObj->Parameter[1],&dwPort))
	{
		PrintLine("Please input port number correctly.");
		return SHELL_CMD_PARSER_SUCCESS;
	}
	wPort = (WORD)(dwPort);    //Now,wPort contains the port where to output.
	if(!Str2Hex(lpCmdObj->Parameter[2],&dwPort))
	{
		PrintLine("Please input the value to output correctly.");
		return SHELL_CMD_PARSER_SUCCESS;
	}
	wr = (WORD)(dwPort);

	//WriteWordToPort(wr,wPort);    //Write the byte to port.
	__outw(wr,wPort);
	return SHELL_CMD_PARSER_SUCCESS;
}

//
//The implementation of outputd.
//

static DWORD outputd(__CMD_PARA_OBJ* lpCmdObj)
{
	WORD            wPort              = 0;
	DWORD           dwVal              = 0;
	DWORD           dwPort             = 0;

	if(NULL == lpCmdObj)  //Parameter check.
		return SHELL_CMD_PARSER_FAILED;

	if(lpCmdObj->byParameterNum < 3)    //Without enough parameter.
	{
		PrintLine("Please input the port and value.");
		return SHELL_CMD_PARSER_SUCCESS;
	}

	if(!Str2Hex(lpCmdObj->Parameter[1],&dwPort))  //Can not convert the port to hex.
	{
		PrintLine("Please input the port value correctly.");
		return SHELL_CMD_PARSER_SUCCESS;
	}

	if(!Str2Hex(lpCmdObj->Parameter[2],&dwVal))  //Can not convert the value to hex.
	{
		PrintLine("Please input the value correctly.");
		return SHELL_CMD_PARSER_SUCCESS;
	}

	wPort = (WORD)(dwPort);

#ifdef __I386__
#ifdef __GCC__
	__asm__ __volatile__(
	".code32			\n\t"
	"pushl	%%edx		\n\t"
	"pushl	%%eax		\n\t"
	"movw	%0,	%%dx	\n\t"
	"movl	%1,	%%eax	\n\t"
	"outw	%%ax,	%%dx	\n\t"
	"popl	%%eax			\n\t"
	"popl	%%edx			\n\t"
	::"r"(wPort), "r"(dwVal));
#else
	__asm{    //Output the double value to port.
		push edx
		push eax
		mov dx,wPort
		mov eax,dwVal
		out dx,eax
		pop eax
		pop edx
	}
#endif
#else
#endif

	return SHELL_CMD_PARSER_SUCCESS;
}


static DWORD outputsb(__CMD_PARA_OBJ* lpCmdObj)
{
	return SHELL_CMD_PARSER_SUCCESS;
}

static DWORD outputsw(__CMD_PARA_OBJ* lpCmdObj)
{
	return SHELL_CMD_PARSER_SUCCESS;
}

static DWORD memalloc(__CMD_PARA_OBJ* lpCmdObj)
{
	DWORD      dwMemSize      = 0;
	LPVOID     lpMemAddr      = NULL;
	CHAR       strBuffer[16];

	if(NULL == lpCmdObj)  //Parameter check.
		return SHELL_CMD_PARSER_FAILED;

	if(lpCmdObj->byParameterNum < 2) //Not enough parameters.
	{
		PrintLine("Please input the memory size to be allocated.");
		return SHELL_CMD_PARSER_SUCCESS;
	}

	if(!Str2Hex(lpCmdObj->Parameter[1],&dwMemSize)) //Invalid size value.
	{
		PrintLine("Invalid memory size value.");
		return SHELL_CMD_PARSER_SUCCESS;
	}

	lpMemAddr = KMemAlloc(dwMemSize,KMEM_SIZE_TYPE_ANY);
	if(NULL == lpMemAddr)  //Failed to allocate memory.
	{
		PrintLine("Can not allocate memory.");
		return SHELL_CMD_PARSER_SUCCESS;
	}

	Hex2Str((DWORD)lpMemAddr,&strBuffer[4]);  //Convert to string.

	strBuffer[0] = ' ';
	strBuffer[1] = ' ';
	strBuffer[2] = '0';
	strBuffer[3] = 'x';
	PrintLine(strBuffer);    //Print out the result.

	return SHELL_CMD_PARSER_SUCCESS;
}

static DWORD memrels(__CMD_PARA_OBJ* lpCmdObj)
{
	DWORD           dwMemAddr          = 0;

	if(NULL == lpCmdObj)
		return SHELL_CMD_PARSER_FAILED;

	if(lpCmdObj->byParameterNum < 2) //Not enough parameters.
	{
		PrintLine("Please input the memory address to be released.");
		return SHELL_CMD_PARSER_SUCCESS;
	}

	if(!Str2Hex(lpCmdObj->Parameter[1],&dwMemAddr)) //Invalid address value.
	{
		PrintLine("Please input the address correctly.");
		return SHELL_CMD_PARSER_SUCCESS;
	}

	KMemFree((LPVOID)dwMemAddr,KMEM_SIZE_TYPE_ANY,0);  //Release the memory.
	return SHELL_CMD_PARSER_SUCCESS;
}

static DWORD memwb(__CMD_PARA_OBJ* lpCmdObj)
{
	DWORD          dwAddress         = 0;
	BYTE           bt                = 0;
	DWORD          dwTmp             = 0;

	if(NULL == lpCmdObj)  //Parameter check.
		return SHELL_CMD_PARSER_FAILED;

	if(lpCmdObj->byParameterNum < 3)
	{
		PrintLine("Please input the address and value together.");
		return SHELL_CMD_PARSER_SUCCESS;
	}

	if(!Str2Hex(lpCmdObj->Parameter[1],&dwAddress)) //Can not convert the address string to
		                                            //valid address.
	{
		PrintLine("  Please input correct address.");
		return SHELL_CMD_PARSER_SUCCESS;
	}  //Now,dwAddress contains the target address to be write.

	if(!Str2Hex(lpCmdObj->Parameter[2],&dwTmp))  //Invalid value.
	{
		PrintLine("  Please input the correct value.");
		return SHELL_CMD_PARSER_SUCCESS;
	}

	bt = (BYTE)(dwTmp);
	*((BYTE*)dwAddress) = bt;    //Write the byte to the appropriate address.

	return SHELL_CMD_PARSER_SUCCESS;
}

static DWORD memww(__CMD_PARA_OBJ* lpCmdObj)
{
	DWORD          dwAddress         = 0;
	WORD           wr                = 0;
	DWORD          dwTmp             = 0;

	if(NULL == lpCmdObj)  //Parameter check.
	{
		return SHELL_CMD_PARSER_FAILED;
	}

	if(lpCmdObj->byParameterNum < 3)
	{
		PrintLine("Please input the address and value together.");
		return SHELL_CMD_PARSER_SUCCESS;
	}

	if(!Str2Hex(lpCmdObj->Parameter[1],&dwAddress)) //Can not convert the address string to
		                                            //valid address.
	{
		PrintLine("  Please input correct address.");
		return SHELL_CMD_PARSER_SUCCESS;
	}  //Now,dwAddress contains the target address to be write.

	if(!Str2Hex(lpCmdObj->Parameter[2],&dwTmp))  //Invalid value.
	{
		PrintLine("  Please input the correct value.");
		return SHELL_CMD_PARSER_SUCCESS;
	}

	wr = (WORD)(dwTmp);
	*((WORD*)dwAddress) = wr;    //Write the byte to the appropriate address.

	return SHELL_CMD_PARSER_SUCCESS;
}

static DWORD memwd(__CMD_PARA_OBJ* lpCmdObj)
{
	DWORD          dwAddress         = 0;	
	DWORD          dwTmp             = 0;

	if(NULL == lpCmdObj)  //Parameter check.
		return SHELL_CMD_PARSER_FAILED;

	if(lpCmdObj->byParameterNum < 3)
	{
		PrintLine("Please input the address and value together.");
		return SHELL_CMD_PARSER_SUCCESS;
	}

	if(!Str2Hex(lpCmdObj->Parameter[1],&dwAddress)) //Can not convert the address string to
		                                            //valid address.
	{
		PrintLine("  Please input correct address.");
		return SHELL_CMD_PARSER_SUCCESS;
	}  //Now,dwAddress contains the target address to be write.

	if(!Str2Hex(lpCmdObj->Parameter[2],&dwTmp))  //Invalid value.
	{
		PrintLine("  Please input the correct value.");
		return SHELL_CMD_PARSER_SUCCESS;
	}

	*((DWORD*)dwAddress) = dwTmp;    //Write the byte to the appropriate address.

	return SHELL_CMD_PARSER_SUCCESS;
}

static DWORD memrb(__CMD_PARA_OBJ* lpCmdObj)
{
	DWORD          dwAddress         = 0;
	BYTE           bt                = 0;
	DWORD          dwTmp             = 0;
	CHAR           strBuffer[16];

	if(NULL == lpCmdObj)    //Parameter check.
		return SHELL_CMD_PARSER_FAILED;

	if(lpCmdObj->byParameterNum < 2)  //Not enough parameter.
	{
		PrintLine("Please input the address where to read.");
		return SHELL_CMD_PARSER_SUCCESS;
	}

	if(!Str2Hex(lpCmdObj->Parameter[1],&dwAddress)) //Invalid address value.
	{
		PrintLine("Please input the correct address.");
		return SHELL_CMD_PARSER_SUCCESS;
	}

	bt = *((BYTE*)dwAddress);
	dwTmp += bt;
	Hex2Str(dwTmp,&strBuffer[4]);  //Convert to string.

	strBuffer[0] = ' ';
	strBuffer[1] = ' ';
	strBuffer[2] = '0';
	strBuffer[3] = 'x';
	PrintLine(strBuffer);  //Print out the result.
	return SHELL_CMD_PARSER_SUCCESS;
}

static DWORD memrw(__CMD_PARA_OBJ* lpCmdObj)
{
	DWORD          dwAddress         = 0;
	WORD           wr                = 0;
	DWORD          dwTmp             = 0;
	CHAR           strBuffer[16];

	if(NULL == lpCmdObj)    //Parameter check.
		return SHELL_CMD_PARSER_FAILED;

	if(lpCmdObj->byParameterNum < 2)  //Not enough parameter.
	{
		PrintLine("Please input the address where to read.");
		return SHELL_CMD_PARSER_SUCCESS;
	}

	if(!Str2Hex(lpCmdObj->Parameter[1],&dwAddress)) //Invalid address value.
	{
		PrintLine("Please input the correct address.");
		return SHELL_CMD_PARSER_SUCCESS;
	}

	wr = *((WORD*)dwAddress);
	dwTmp += wr;
	Hex2Str(dwTmp,&strBuffer[4]);  //Convert to string.

	strBuffer[0] = ' ';
	strBuffer[1] = ' ';
	strBuffer[2] = '0';
	strBuffer[3] = 'x';
	PrintLine(strBuffer);  //Print out the result.
	return SHELL_CMD_PARSER_SUCCESS;
}

static DWORD memrd(__CMD_PARA_OBJ* lpCmdObj)
{
	DWORD          dwAddress         = 0;
	DWORD          dwTmp             = 0;
	CHAR           strBuffer[16];

	if(NULL == lpCmdObj)    //Parameter check.
		return SHELL_CMD_PARSER_FAILED;

	if(lpCmdObj->byParameterNum < 2)  //Not enough parameter.
	{
		PrintLine("Please input the address where to read.");
		return SHELL_CMD_PARSER_SUCCESS;
	}

	if(!Str2Hex(lpCmdObj->Parameter[1],&dwAddress)) //Invalid address value.
	{
		PrintLine("Please input the correct address.");
		return SHELL_CMD_PARSER_SUCCESS;
	}

	dwTmp = *((DWORD*)dwAddress);
	Hex2Str(dwTmp,&strBuffer[4]);  //Convert to string.

	strBuffer[0] = ' ';
	strBuffer[1] = ' ';
	strBuffer[2] = '0';
	strBuffer[3] = 'x';
	PrintLine(strBuffer);  //Print out the result.

	return SHELL_CMD_PARSER_SUCCESS;
}

static DWORD help(__CMD_PARA_OBJ* lpCmdObj)
{
	DWORD         dwMapIndex        = 0;

	PrintLine("Application is used to control IO port:");
	while(IOCtrlCmdMap[dwMapIndex].lpszHelpInfo)
	{
		PrintLine(IOCtrlCmdMap[dwMapIndex].lpszHelpInfo);
		dwMapIndex ++;
	}

	return SHELL_CMD_PARSER_SUCCESS;
}

static DWORD exit(__CMD_PARA_OBJ* lpCmdObj)
{
	return SHELL_CMD_PARSER_TERMINAL;
}

