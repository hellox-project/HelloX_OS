//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Jun,10 2005
//    Module Name               : IOCTRL_S.CPP
//    Module Funciton           : 
//                                ioctrl(I/O control) is a application in 
//                                kernel mode and is used for debugging device
//                                drivers mainly.
//                                It can view device configure space maped to
//                                system memory,and can change the content of
//                                a user specified location in memory.
//                                It's a useful application in my developing of
//                                device drivers.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#include <StdAfx.h>
#include <string.h>
#include <stdio.h>

#include "shell.h"
#include "fs.h"
#include "ioctrl_s.h"

#define  IOCTRL_PROMPT_STR   "[ioctrl_view]"

/*
 * The IO control application is used to input 
 * or output data to or from io-port/memory.
 * This application can support input/output one 
 *  byte,one word, one byte string or one word string 
 * from/into io-port/memory.
 * Usage description:
 *  #inputb io_port           //Input a byte from port,which is indicated by io_port.
 *  #inputw io_port           //Input a word from port,which is indicated by io_port.
 *  #inputsb io_port str_len  //Input a byte string from port,which is indicated by io_port,
 *                            //str_len is the number of bytes to be inputed.
 *  #inputsw io_port str_len  //The same as inputsb.
 *  #outputb io_port          //Output a byte to port.
 *  #outputw io_port          //Output a word to port.
 *  #outputsb io_port str_len //Output a byte string to port.
 *  #outputsw io_port str_len //Output a word string to port.
 *  #memwb mem_addr data      //Write a byte to the memory address 'mem_addr'.
 *  #memww mem_addr data      //Write a word to the memory address 'mem_addr'.
 *  #memwd mem_addr data      //Write a dword to the memory.
 *  #memrb mem_addr           //Read a byte from memory.
 *  #memrw mem_addr           //Read a word from memory.
 *  #memrd mem_addr           //Read a dword from memory.
 *  #exit                     //Exit this application.
 *  #help                     //Print help information about this application.
 */

/* Work routines of this app. */
static DWORD inputb(__CMD_PARA_OBJ*);
static DWORD inputw(__CMD_PARA_OBJ*);
static DWORD inputd(__CMD_PARA_OBJ*);

static DWORD outputb(__CMD_PARA_OBJ*);
static DWORD outputw(__CMD_PARA_OBJ*);
static DWORD outputd(__CMD_PARA_OBJ*);

static DWORD memwb(__CMD_PARA_OBJ*);
static DWORD memww(__CMD_PARA_OBJ*);
static DWORD memwd(__CMD_PARA_OBJ*);

static DWORD memrb(__CMD_PARA_OBJ*);
static DWORD memrw(__CMD_PARA_OBJ*);
static DWORD memrd(__CMD_PARA_OBJ*);

static DWORD help(__CMD_PARA_OBJ*);
static DWORD exit(__CMD_PARA_OBJ*);

/*
 * A map between IO control command and it's handler.
 */
static struct __IOCTRL_COMMAND_MAP{
	/* command string. */
	LPSTR lpszCommand; 
	/* command handler. */
	DWORD (*CommandRoutine)(__CMD_PARA_OBJ*);
	/* help string. */
	LPSTR lpszHelpInfo;
}IOCtrlCmdMap[] = {
	{"inputb",    inputb,      "  inputb io_port      : Input a byte from IO port."},
	{"inputw",    inputw,      "  inputw io_port      : Input a word from IO port."},
	{"inputd",    inputd,      "  inputd io_port      : Input a dword from IO port."},
	{"outputb",   outputb,     "  outputb io_port val : Output a byte to IO port."},
	{"outputw",   outputw,     "  outputw io_port val : Output a word to IO port."},
	{"outputd",   outputd,     "  outputd io_port val : Output a dword to IO port."},
	{"memwb",     memwb,       "  memwb addr val      : Write a byte to memory location."},
	{"memww",     memww,       "  memww addr val      : Write a word to memory location."},
	{"memwd",     memwd,       "  memwd addr val      : Write a dword to memory location."},
	{"memrb",     memrb,       "  memrb addr          : Read a byte from memory."},
	{"memrw",     memrw,       "  memrw addr          : Read a word from memory."},
	{"memrd",     memrd,       "  memrd addr          : Read a dword from memory."},
	{"help",      help,        "  help                : Print out this screen."},
	{"exit",      exit,        "  exit                : Exit from this application."},
	/* end of the array. */
	{NULL,        NULL,        NULL}
};

/*
 * Entry point of IO control application.
 * This routine does the following:
 *  1. Get input message from kernel thread message 
 *     queue by calling GetMessage;
 *  2. If the message is a key down event,and the key 
 *     is ENTRER,then process the command;
 *  3. Form a command parameter object by calling 
 *     FormParameterObj;
 *  4. Lookup the IO command and handler's map,
 *     to find appropriate handler;
 *  5. Call the handler,command parameter
 *     as the handler's parameter;
 *  6. If the hander returns SHELL_CMD_PARSER_FAILED,
 *     then exit the application;
 *  7. Otherwise,continue the loop.
 */

static DWORD CommandParser(LPCSTR lpszCmdLine)
{
	__CMD_PARA_OBJ* lpParamObj = NULL;
	DWORD dwRetVal = SHELL_CMD_PARSER_INVALID;
	DWORD dwIndex = 0;
	BOOL bValidCmd = FALSE;

	lpParamObj = FormParameterObj(lpszCmdLine);
	if(NULL == lpParamObj)
	{		
		return SHELL_CMD_PARSER_FAILED;
	}

	if(0 != lpParamObj->byParameterNum)
	{
		/* Search command-handler map table. */
		while(IOCtrlCmdMap[dwIndex].lpszCommand)
		{
			if(StrCmp(IOCtrlCmdMap[dwIndex].lpszCommand,
				lpParamObj->Parameter[0]))
			{
				bValidCmd = TRUE;
				break;
			}
			dwIndex ++;
		}

		if(bValidCmd)
		{
			/* Invoke the command's handler. */
			dwRetVal = IOCtrlCmdMap[dwIndex].CommandRoutine(lpParamObj);
		}
		else
		{
			dwRetVal = SHELL_CMD_PARSER_INVALID;			
		}
	}

	ReleaseParameterObj(lpParamObj);
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

/* Start routine of ioctrl application. */
DWORD IoCtrlStart(LPVOID p)
{
	return Shell_Msg_Loop(IOCTRL_PROMPT_STR,CommandParser,QueryCmdName);	

}

/* input byte from port and show it. */
static DWORD inputb(__CMD_PARA_OBJ* lpCmdObj)
{
	WORD wInputPort = 0;
	UCHAR bt = 0;
	DWORD dwInputPort = 0;
	CHAR strBuffer[15] = { 0 };

	if(NULL == lpCmdObj)
	{
		return SHELL_CMD_PARSER_FAILED;
	}

	if(lpCmdObj->byParameterNum < 2)
	{
		/* No port specified. */
		PrintLine("No port specified.");
		return SHELL_CMD_PARSER_SUCCESS;
	}
	/* Convert the string value to hex. */
	if(!Str2Hex(lpCmdObj->Parameter[1],&dwInputPort))
	{
		PrintLine("Invalid port value.");
		return SHELL_CMD_PARSER_SUCCESS;
	}

	/* Read from port. */
	wInputPort = (WORD)dwInputPort;
	bt = __inb(wInputPort);

	dwInputPort =  0;
	dwInputPort += bt;
	strBuffer[0] = ' ';
	strBuffer[1] = ' ';
	strBuffer[2] = ' ';
	strBuffer[3] = ' ';

	/* Show out result. */
	Hex2Str(dwInputPort,&strBuffer[4]);
	PrintLine(strBuffer);

	return SHELL_CMD_PARSER_SUCCESS;
}

/* read word from a port. */
static DWORD inputw(__CMD_PARA_OBJ* lpCmdObj)
{
	WORD wInputPort = 0;
	WORD wr = 0;
	DWORD dwInputPort = 0;
	CHAR strBuffer[15] = { 0 };

	if (NULL == lpCmdObj)
	{
		return SHELL_CMD_PARSER_FAILED;
	}

	if(lpCmdObj->byParameterNum < 2)
	{
		/* No port specified. */
		PrintLine("No port specified.");
		return SHELL_CMD_PARSER_SUCCESS;
	}
	if(!Str2Hex(lpCmdObj->Parameter[1],&dwInputPort))
	{
		PrintLine("Invalid port value.");
		return SHELL_CMD_PARSER_SUCCESS;
	}

	/* Read data and show it. */
	wInputPort = (WORD)(dwInputPort);
	wr = __inw(wInputPort);

	dwInputPort =  0;
	dwInputPort += wr;

	strBuffer[0] = ' ';
	strBuffer[1] = ' ';
	strBuffer[2] = ' ';
	strBuffer[3] = ' ';

	Hex2Str(dwInputPort,&strBuffer[4]);
	PrintLine(strBuffer);

	return SHELL_CMD_PARSER_SUCCESS;
}

/* read and show a 32 bits value from port. */
static DWORD inputd(__CMD_PARA_OBJ* lpParamObj)
{
	DWORD dwVal = 0;
	WORD wPort = 0;
	CHAR strBuffer[15];

	if (NULL == lpParamObj)
	{
		return SHELL_CMD_PARSER_FAILED;
	}

	if(lpParamObj->byParameterNum < 2)
	{
		/* No port specified. */
		PrintLine("No port specified.");
		return SHELL_CMD_PARSER_SUCCESS;
	}

	if(!Str2Hex(lpParamObj->Parameter[1],&dwVal))
	{
		/* Invalid port value. */
		PrintLine("Invalid port specified.");
		return SHELL_CMD_PARSER_SUCCESS;
	}

	wPort = (WORD)(dwVal);

	/* Read from port. */
#ifdef __I386__
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

	/* Show out. */
	strBuffer[0] = ' ';
	strBuffer[1] = ' ';
	strBuffer[2] = ' ';
	strBuffer[3] = ' ';

	Hex2Str(dwVal,&strBuffer[4]);
	PrintLine(strBuffer);

	return SHELL_CMD_PARSER_SUCCESS;
}

/* Output a byte to a port. */
static DWORD outputb(__CMD_PARA_OBJ* lpCmdObj)
{
	UCHAR bt = 0;
	WORD wPort = 0;
	DWORD dwPort = 0;

	if (NULL == lpCmdObj)
	{
		return SHELL_CMD_PARSER_FAILED;
	}

	if(lpCmdObj->byParameterNum < 3)
	{
		PrintLine("No port or value specified.");
		return SHELL_CMD_PARSER_SUCCESS;
	}
	if(!Str2Hex(lpCmdObj->Parameter[1],&dwPort))
	{
		PrintLine("Invalid parameter.");
		return SHELL_CMD_PARSER_SUCCESS;
	}

	wPort = (WORD)(dwPort);
	if(!Str2Hex(lpCmdObj->Parameter[2],&dwPort))
	{
		PrintLine("Invalid parameter.");
		return SHELL_CMD_PARSER_SUCCESS;
	}
	bt = (BYTE)(dwPort);
	__outb(bt,wPort);
	return SHELL_CMD_PARSER_SUCCESS;
}

/* Write a word to port. */
static DWORD outputw(__CMD_PARA_OBJ* lpCmdObj)
{
	WORD wr = 0;
	WORD wPort = 0;
	DWORD dwPort = 0;

	if (NULL == lpCmdObj)
	{
		return SHELL_CMD_PARSER_FAILED;
	}

	if(lpCmdObj->byParameterNum < 3)
	{
		PrintLine("No port or value specified.");
		return SHELL_CMD_PARSER_SUCCESS;
	}
	if(!Str2Hex(lpCmdObj->Parameter[1],&dwPort))
	{
		PrintLine("Invalid port.");
		return SHELL_CMD_PARSER_SUCCESS;
	}
	wPort = (WORD)(dwPort);
	if(!Str2Hex(lpCmdObj->Parameter[2],&dwPort))
	{
		PrintLine("Invalid value.");
		return SHELL_CMD_PARSER_SUCCESS;
	}
	wr = (WORD)(dwPort);
	__outw(wr,wPort);
	return SHELL_CMD_PARSER_SUCCESS;
}

/* Write a dword to port. */
static DWORD outputd(__CMD_PARA_OBJ* lpCmdObj)
{
	WORD wPort = 0;
	DWORD dwVal = 0;
	DWORD dwPort = 0;

	if (NULL == lpCmdObj)
	{
		return SHELL_CMD_PARSER_FAILED;
	}

	if(lpCmdObj->byParameterNum < 3)
	{
		PrintLine("No port or value specified.");
		return SHELL_CMD_PARSER_SUCCESS;
	}

	if(!Str2Hex(lpCmdObj->Parameter[1],&dwPort))
	{
		PrintLine("Invalid port.");
		return SHELL_CMD_PARSER_SUCCESS;
	}

	if(!Str2Hex(lpCmdObj->Parameter[2],&dwVal))
	{
		PrintLine("Invalid value.");
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
	__asm{
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

/* Read a byte from device maped memory. */
static DWORD memwb(__CMD_PARA_OBJ* lpCmdObj)
{
	DWORD dwAddress = 0;
	BYTE bt = 0;
	DWORD dwTmp = 0;

	if (NULL == lpCmdObj)
	{
		return SHELL_CMD_PARSER_FAILED;
	}

	if(lpCmdObj->byParameterNum < 3)
	{
		PrintLine("No addr or value specified.");
		return SHELL_CMD_PARSER_SUCCESS;
	}

	if(!Str2Hex(lpCmdObj->Parameter[1],&dwAddress))
	{
		PrintLine("Invalid address.");
		return SHELL_CMD_PARSER_SUCCESS;
	}
	if(!Str2Hex(lpCmdObj->Parameter[2],&dwTmp))
	{
		PrintLine("Invalid value.");
		return SHELL_CMD_PARSER_SUCCESS;
	}

	bt = (BYTE)(dwTmp);
	__writeb(bt, dwAddress);

	return SHELL_CMD_PARSER_SUCCESS;
}

/* Read a byte from memory location. */
static DWORD memww(__CMD_PARA_OBJ* lpCmdObj)
{
	DWORD dwAddress = 0;
	WORD wr = 0;
	DWORD dwTmp = 0;

	if(NULL == lpCmdObj)
	{
		return SHELL_CMD_PARSER_FAILED;
	}

	if(lpCmdObj->byParameterNum < 3)
	{
		PrintLine("No addr or value specified.");
		return SHELL_CMD_PARSER_SUCCESS;
	}

	if(!Str2Hex(lpCmdObj->Parameter[1],&dwAddress))
	{
		PrintLine("Invalid address.");
		return SHELL_CMD_PARSER_SUCCESS;
	}

	if(!Str2Hex(lpCmdObj->Parameter[2],&dwTmp))
	{
		PrintLine("Invalid value.");
		return SHELL_CMD_PARSER_SUCCESS;
	}

	wr = (WORD)(dwTmp);
	__writew(wr, dwAddress);

	return SHELL_CMD_PARSER_SUCCESS;
}

/* Write a 32 bits value into a memory location. */
static DWORD memwd(__CMD_PARA_OBJ* lpCmdObj)
{
	DWORD dwAddress = 0;
	DWORD dwTmp = 0;

	if (NULL == lpCmdObj)
	{
		return SHELL_CMD_PARSER_FAILED;
	}

	if(lpCmdObj->byParameterNum < 3)
	{
		PrintLine("No addr or value specified.");
		return SHELL_CMD_PARSER_SUCCESS;
	}

	if(!Str2Hex(lpCmdObj->Parameter[1],&dwAddress))
	{
		PrintLine("Invalie address.");
		return SHELL_CMD_PARSER_SUCCESS;
	}

	if(!Str2Hex(lpCmdObj->Parameter[2],&dwTmp))
	{
		PrintLine("Invalid value.");
		return SHELL_CMD_PARSER_SUCCESS;
	}

	__writel(dwTmp, dwAddress);

	return SHELL_CMD_PARSER_SUCCESS;
}

/* Read a byte from memory location. */
static DWORD memrb(__CMD_PARA_OBJ* lpCmdObj)
{
	DWORD dwAddress = 0;
	BYTE bt = 0;
	DWORD dwTmp = 0;
	CHAR strBuffer[16];

	if (NULL == lpCmdObj)
	{
		return SHELL_CMD_PARSER_FAILED;
	}

	if(lpCmdObj->byParameterNum < 2)
	{
		PrintLine("No address specified.");
		return SHELL_CMD_PARSER_SUCCESS;
	}

	if(!Str2Hex(lpCmdObj->Parameter[1],&dwAddress))
	{
		PrintLine("Invalid address.");
		return SHELL_CMD_PARSER_SUCCESS;
	}

	bt = __readb(dwAddress);

	dwTmp += bt;
	/* Convert to string and show it out. */
	Hex2Str(dwTmp,&strBuffer[4]);
	strBuffer[0] = ' ';
	strBuffer[1] = ' ';
	strBuffer[2] = '0';
	strBuffer[3] = 'x';
	PrintLine(strBuffer);
	return SHELL_CMD_PARSER_SUCCESS;
}

/* Read a word from memory location. */
static DWORD memrw(__CMD_PARA_OBJ* lpCmdObj)
{
	DWORD dwAddress = 0;
	WORD wr = 0;
	DWORD dwTmp = 0;
	CHAR strBuffer[16];

	if (NULL == lpCmdObj)
	{
		return SHELL_CMD_PARSER_FAILED;
	}

	if(lpCmdObj->byParameterNum < 2)
	{
		PrintLine("No address specified.");
		return SHELL_CMD_PARSER_SUCCESS;
	}

	if(!Str2Hex(lpCmdObj->Parameter[1],&dwAddress))
	{
		PrintLine("Invalid address.");
		return SHELL_CMD_PARSER_SUCCESS;
	}

	wr = __readw(dwAddress);

	/* Convert to string and show out. */
	dwTmp += wr;
	Hex2Str(dwTmp,&strBuffer[4]);
	strBuffer[0] = ' ';
	strBuffer[1] = ' ';
	strBuffer[2] = '0';
	strBuffer[3] = 'x';
	PrintLine(strBuffer);
	return SHELL_CMD_PARSER_SUCCESS;
}

/* Read a 32 bits integer from memory location. */
static DWORD memrd(__CMD_PARA_OBJ* lpCmdObj)
{
	DWORD dwAddress = 0;
	DWORD dwTmp = 0;
	CHAR strBuffer[16];

	if (NULL == lpCmdObj)
	{
		return SHELL_CMD_PARSER_FAILED;
	}

	if(lpCmdObj->byParameterNum < 2)
	{
		PrintLine("No address specified.");
		return SHELL_CMD_PARSER_SUCCESS;
	}

	if(!Str2Hex(lpCmdObj->Parameter[1],&dwAddress))
	{
		PrintLine("Invalid value.");
		return SHELL_CMD_PARSER_SUCCESS;
	}

	dwTmp = __readl(dwAddress);

	/* Convert to string and show out. */
	Hex2Str(dwTmp, &strBuffer[4]);
	strBuffer[0] = ' ';
	strBuffer[1] = ' ';
	strBuffer[2] = '0';
	strBuffer[3] = 'x';
	PrintLine(strBuffer);

	return SHELL_CMD_PARSER_SUCCESS;
}

static DWORD help(__CMD_PARA_OBJ* lpCmdObj)
{
	DWORD dwMapIndex = 0;

	PrintLine("iotrcl app:");
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
