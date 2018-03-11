//***********************************************************************/
//    Author                    : Garry
//    Original Date             : March 10,2018
//    Module Name               : cli.c
//    Module Funciton           : 
//                                Command line interface functions for HelloX,
//                                which can be invoked by application per requirement.
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#include <hellox.h>
#include <stdlib.h>
#include <string.h>

/* Local buffer to hold the user's key stroke. */
static char* cmd_buffer = NULL;
/* 
 * Prompt string,it's the application's filename,without 
 * surfix(.exe or others).
 */
static char prompt_string[CMD_PARAMETER_LEN + 1] = { 0 };

/* Show prompt string. */
static void ShowPrompt()
{
	_hx_printf("[%s]", prompt_string);
}

/* A helper routine to collect user's key stroke into command buffer. */
static int get_user_stroke(char* buffer, size_t len)
{
	char*                       pDataBuffer = buffer;
	char*                       pCurrPos = NULL;
	DWORD                       dwDefaultSize = len; //Command buffer's length.
	BOOL                        bCtrlDown = FALSE;
	char                        bt;
	WORD                        wr = 0x0700;
	MSG                         Msg;
	int                         ret = -1;

	//Parameter checking.
	if ((NULL == buffer) || (NULL == len))
	{
		goto __TERMINAL;
	}
	pCurrPos = pDataBuffer;

	while (TRUE)
	{
		if (GetMessage(&Msg))
		{
			if (KERNEL_MESSAGE_AKDOWN == Msg.wCommand)    //This is a key down message.
			{
				bt = (BYTE)Msg.dwParam;
				switch (bt)
				{
				case VK_RETURN:
					/* Enter key,terminate command buffer and return. */
					if ((DWORD)(pCurrPos - pDataBuffer) < dwDefaultSize)
					{
						*pCurrPos = '\0';
						ret = pCurrPos - pDataBuffer;
						/* Change a new line. */
						ChangeLine();
						GotoHome();
						goto __TERMINAL;
					}
					break;
				case VK_BACKSPACE:
					/* Delete current character and move back cursor. */
					if (*pCurrPos == '\n')
					{
						pCurrPos -= 2;     //Skip the \r\n.
					}
					else
					{
						pCurrPos -= 1;
					}
					GotoPrev();
					break;
				default:
					/* Save the key stroke into buffer,reserve one byte for terminator(0). */
					if ((DWORD)(pCurrPos - pDataBuffer) < dwDefaultSize - 1)
					{
						*pCurrPos++ = bt;
						wr += bt;
						PrintChar(wr);
						wr = 0x0700;
					}
					break;
				}
			}
			else
			{
				if (KERNEL_MESSAGE_VKDOWN == Msg.wCommand)
				{
					bt = (BYTE)Msg.dwParam;
					if (VK_CONTROL == bt)
					{
						bCtrlDown = TRUE;
					}
				}
				if (KERNEL_MESSAGE_VKUP == Msg.wCommand)
				{
					bt = (BYTE)Msg.dwParam;
					if (VK_CONTROL == bt)    //Control key up.
					{
						bCtrlDown = FALSE;
					}
				}
			}
		}
	}

__TERMINAL:
	return ret;
}

/* Return the user's input command. */
int getcmd(char** argv, int argc)
{
	int ret = 0;
	int cmd_len = 0;

	if ((NULL == argv) || (argc <= 0))
	{
		goto __TERMINAL;
	}
	if (argc > CMD_PARAMETER_COUNT)
	{
		goto __TERMINAL;
	}

	/* Allocate command buffer if the routine is called at first time. */
	if (NULL == cmd_buffer)
	{
		cmd_buffer = (char*)_hx_malloc(CMD_BUFFER_LENGTH);
		if (NULL == cmd_buffer)
		{
			goto __TERMINAL;
		}
	}
	/* Show command prompt. */
	ShowPrompt();
	/* Get command string. */
	cmd_len = get_user_stroke(cmd_buffer, CMD_BUFFER_LENGTH);
	if (cmd_len < 0)
	{
		/* No user input. */
		goto __TERMINAL;
	}
	/* Splitter the command buffer into string parameters. */
	char byParamNum = 0, byParamPos = 0;
	char* pCmdPos = cmd_buffer;
	while (*pCmdPos)
	{
		/* Skip the leading space char in command string. */
		if (' ' == *pCmdPos)
		{
			pCmdPos++;
			continue;
		}

		/* Save the parameter into argv. */
		argv[byParamNum] = pCmdPos;
		while ((' ' != *pCmdPos) && (*pCmdPos) && (byParamPos < CMD_PARAMETER_LEN))
		{
			pCmdPos++;
			byParamPos++;
		}
		byParamNum++;
		byParamPos = 0;
		if (' ' != *pCmdPos)
		{
			if (*pCmdPos)
			{
				*pCmdPos = 0; /* Set terminator. */
			}
			break;
		}
		*pCmdPos++ = '\0';

		if (byParamNum == argc)
		{
			break;
		}
	}
	ret = byParamNum;

__TERMINAL:
	return ret;
}

/* 
 * Application specific entry point,this routine is invoked by library,
 * and every application should implement a instance of this routine,
 * in it's source file.
 * It's the equivalent of the main routine in standard C library.
 */
extern int _hx_main(int argc, char* argv[]);

/* 
 * Entry point of the application suported by this library suit.
 * This routine is visible to application loader in HelloX kernel,
 * and the application specified entry point,named _hx_main,will
 * be invoked in this routine.
 * Library specified initializations,destructions,and other support
 * mechanisms,are implemented in this wrapping routine.
 */
int _hx_entry(int argc, char* argv[])
{
	char* app_name = NULL;
	int ret = -1, length = 0;

	/* Argument's number must not exceed the max value. */
	if ((argc > CMD_PARAMETER_COUNT) || (0 == argc))
	{
		_hx_printf("Too many or too few arguments[%d].\r\n", argc);
		goto __TERMINAL;
	}
	if (NULL == argv)
	{
		_hx_printf("Argument should not be empty.\r\n");
		goto __TERMINAL;
	}

	/* Obtain the application's name,without suffix,as prompt string. */
	app_name = strrchr(argv[0], '\\');
	if (NULL == app_name)
	{
		_hx_printf("Can not obtain name from[%s].\r\n", argv[0]);
		goto __TERMINAL;
	}

	if (strlen(app_name) < 2)
	{
		_hx_printf("Application file name too short[%s].\r\n", app_name);
		goto __TERMINAL;
	}
	app_name += 1; /* Skip the slash. */
	length = 0;
	while (*app_name && ('.' != *app_name) && (length < CMD_PARAMETER_LEN))
	{
		prompt_string[length++] = *app_name++;
	}
	prompt_string[length] = 0;

	/* Call the application specified entry point. */
	ret = _hx_main(argc, argv);

__TERMINAL:
	/* Release the command buffer memory if allocated. */
	if (cmd_buffer)
	{
		_hx_free(cmd_buffer);
	}
	return ret;
}
