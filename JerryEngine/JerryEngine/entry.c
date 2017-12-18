//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Jan,28 2017
//    Module Name               : entry.c
//    Module Funciton           : 
//                                Entry routine of JerryScript engine under HelloX
//                                operating system.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <kapi.h>

#include "jerry-api.h"
#include "jerry-port.h"

/**
 * A local helper routine to show out a jerry value
 * object.
 * Got this routine from the jerry project's home 
 * page in github.com.
 */
static void print_value(const jerry_value_t value)
{
	if (jerry_value_is_undefined(value))
	{
		jerry_port_console("undefined");
	}
	else if (jerry_value_is_null(value))
	{
		jerry_port_console("null");
	}
	else if (jerry_value_is_boolean(value))
	{
		if (jerry_get_boolean_value(value))
		{
			jerry_port_console("true");
		}
		else
		{
			jerry_port_console("false");
		}
	}
	/* Float value */
	else if (jerry_value_is_number(value))
	{
		jerry_port_console("number");
	}
	/* String value */
	else if (jerry_value_is_string(value))
	{
		/* Determining required buffer size */
		jerry_size_t req_sz = jerry_get_string_size(value);
		//jerry_char_t str_buf_p[req_sz];
		jerry_char_t* str_buf_p = NULL;

		str_buf_p = (jerry_char_t*)_hx_malloc(req_sz);
		if (NULL == str_buf_p)
		{
			jerry_port_console("Mem allocate error.\r\n");
			return;
		}
		jerry_string_to_char_buffer(value, str_buf_p, req_sz);
		str_buf_p[req_sz] = '\0';
		jerry_port_console("%s", (const char *)str_buf_p);
		_hx_free(str_buf_p);
	}
	/* Object reference */
	else if (jerry_value_is_object(value))
	{
		jerry_port_console("[JS object]");
	}

	jerry_port_console("\r\n");
}

/**
 * A local helper routine to get user's JavaScript from
 * console input.
 */
static int get_user_script(char* buffer, size_t* len)
{
	BYTE*                       pDataBuffer = buffer;
	BYTE*                       pCurrPos = NULL;
	DWORD                       dwDefaultSize = *len;      //Default file size is 8K.
	BOOL                        bCtrlDown = FALSE;
	BYTE                        bt;
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
				case VK_RETURN:                  //This is a return key.
					if ((DWORD)(pCurrPos - pDataBuffer) < dwDefaultSize - 2)
					{
						*pCurrPos++ = '\r';     //Append a return key.
						*pCurrPos++ = '\n';
						GotoHome();
						ChangeLine();           //Change to next line.
						_hx_printf("> ");       //Show prompt.
					}
					break;
				case VK_BACKSPACE:
					if (*pCurrPos == '\n')  //Enter encountered.
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
					if (('z' == bt) || ('Z' == bt))
					{
						if (bCtrlDown)  //Ctrl+C or Ctrl+Z pressed.
						{
							ret = 0;
							goto __TERMINAL;
						}
					}
					if (('c' == bt) || ('C' == bt))
					{
						if (bCtrlDown)
						{
							ret = -1;
							goto __TERMINAL;
						}
					}
					if ((DWORD)(pCurrPos - pDataBuffer) < dwDefaultSize)
					{
						*pCurrPos++ = bt; //Save this character.
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
	*len = pCurrPos - pDataBuffer;
	GotoHome();
	ChangeLine();
	return ret;
}

/**
 * Maximal user script buffer length.
 */
#define MAX_USER_JS_LEN (1024 * 64)

/**
 * Main entry of Jerry Engine under HelloX.
 */
int _hx_jerry_entry(int argc, char *argv[])
{
	bool is_done = false;
	char* cmd = NULL;
	size_t len = 0;

	/* Initialize engine */
	jerry_init(JERRY_INIT_EMPTY);

	/* Allocate JavaScript buffer to hold user's input. */
	cmd = (char*)_hx_malloc(MAX_USER_JS_LEN);
	if (NULL == cmd)
	{
		_hx_printf("Failed to allocate script buffer.\r\n");
		return -1;
	}

	while (!is_done)
	{
		jerry_port_console("> ");

		//Read command from console.
		len = MAX_USER_JS_LEN;
		if (-1 == get_user_script(cmd,&len))
		{
			is_done = true;
			break;
		}

		_hx_printf("Get user script OK[len = %d],begin to parse.\r\n", len);

		jerry_value_t ret_val;

		/* Evaluate entered command */
		ret_val = jerry_eval((const jerry_char_t *)cmd,
			len,
			false);

		/* If command evaluated successfully, print value, returned by eval */
		if (jerry_value_has_error_flag(ret_val))
		{
			/* Evaluated JS code thrown an exception
			*  and didn't handle it with try-catch-finally */
			jerry_port_console("Unhandled JS exception occured: ");
		}

		print_value(ret_val);
		jerry_release_value(ret_val);
	}

	/* Cleanup engine */
	jerry_cleanup();

	/* Release Java Script buffer. */
	if (cmd)
	{
		_hx_free(cmd);
	}
	return 0;
}

#define JERRY_SIMPLE_SCRIPT "var _PI = Math.PI; var _R = 20; print('Area is:' + _PI * _R * _R);"

int _hx_jerry_entry_(int argc,char* argv[])
{
	char* cmd = NULL;
	int len = 0;

	cmd = (char*)_hx_malloc(MAX_USER_JS_LEN);
	if (NULL == cmd)
	{
		_hx_printf("Failed to allocate JS buffer.\r\n");
		return -1;
	}

	while (TRUE)
	{
		len = MAX_USER_JS_LEN;
		jerry_port_console("> ");
		if (-1 == get_user_script(cmd, &len))
		{
			break;
		}
		jerry_run_simple(cmd, len, JERRY_INIT_EMPTY);
	}
	//return jerry_run_simple(JERRY_SIMPLE_SCRIPT, 
	//	strlen(JERRY_SIMPLE_SCRIPT), 
	//	JERRY_INIT_EMPTY);
	_hx_free(cmd);
	return 0;
}
