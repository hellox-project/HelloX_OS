//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Oct 28, 2021
//    Module Name               : dbm.c
//    Module Funciton           : 
//                                DBM, denotes Display Buffer Manager in
//                                hellox, responses for the management of
//                                display buffer functions. All content
//                                appear in screen will be put into this
//                                buffer, and other applications could fetch
//                                from it to do farther processing.
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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "dbm.h"

/* Initialize routine of DBM. */
static BOOL __Initialize()
{
	BOOL bResult = FALSE;
	char* buffer = NULL;

#if defined(__CFG_SYS_SMP)
	__INIT_SPIN_LOCK(DispBufferManager.spin_lock, "sl_dbm");
#endif 

	/* Create the display content buffer. */
	buffer = (char*)_hx_malloc(DISPLAY_BUFFER_LENGTH);
	if (NULL == buffer)
	{
		_hx_printf("[%s]out of memory.\r\n", __func__);
		goto __TERMINAL;
	}
	memset(buffer, 0, DISPLAY_BUFFER_LENGTH);

	DispBufferManager.disp_buffer = buffer;
	DispBufferManager.buffer_head = DispBufferManager.buffer_rear = 0;

	bResult = TRUE;
__TERMINAL:
	return bResult;
}

/* 
 * Put a character into display buffer. 
 * The index of this char in buffer will be
 * returned.
 */
static int __PutChar(char disp_char)
{
	int char_index = -1;
	unsigned long ulFlags;

	/* Could not be invoked in process of initiazation. */
	if (IN_SYSINITIALIZATION())
	{
		goto __TERMINAL;
	}

	BUG_ON(NULL == DispBufferManager.disp_buffer);

	__ENTER_CRITICAL_SECTION_SMP(DispBufferManager.spin_lock, ulFlags);
	DispBufferManager.disp_buffer[DispBufferManager.buffer_rear] = disp_char;
	char_index = DispBufferManager.buffer_rear;
	DispBufferManager.buffer_rear++;
	if (DispBufferManager.buffer_rear == DISPLAY_BUFFER_LENGTH)
	{
		/* Reach end of buffer, round back. */
		DispBufferManager.buffer_rear = 0;
	}
	__LEAVE_CRITICAL_SECTION_SMP(DispBufferManager.spin_lock, ulFlags);

__TERMINAL:
	return char_index;
}

/* Put a string into display buffer. */
static int __PutString(const char* disp_string, int length)
{
	int char_index = -1;
	int string_length = length;
	unsigned long ulFlags;

	/* Could not be invoked in process of initiazation. */
	if (IN_SYSINITIALIZATION())
	{
		goto __TERMINAL;
	}
	BUG_ON((NULL == DispBufferManager.disp_buffer) || (NULL == disp_string));

	/* String could not exceed the defined value. */
	if (string_length > DISPLAY_BUFFER_LENGTH)
	{
		string_length = DISPLAY_BUFFER_LENGTH;
	}

	__ENTER_CRITICAL_SECTION_SMP(DispBufferManager.spin_lock, ulFlags);
	/* Get the start position first. */
	char_index = DispBufferManager.buffer_rear;
	/* Put to buffer one by one. */
	for (int i = 0; i < string_length; i++)
	{
		DispBufferManager.disp_buffer[DispBufferManager.buffer_rear] = disp_string[i];
		DispBufferManager.buffer_rear++;
		if (DispBufferManager.buffer_rear == DISPLAY_BUFFER_LENGTH)
		{
			/* Reach end of buffer, round back. */
			DispBufferManager.buffer_rear = 0;
		}
	}
	__LEAVE_CRITICAL_SECTION_SMP(DispBufferManager.spin_lock, ulFlags);

__TERMINAL:
	return char_index;
}

/* 
 * Get some content from display buffer, no block mode.
 * How many bytes got will be returned, the request length is
 * denoted by buffer_len.
 * @start denotes where to fetch from.
 */
static int __GetBuffer(char* buffer, int buffer_len, int* start)
{
	int ret = -1;
	int get_length = buffer_len;
	int start_pos = 0;
	unsigned long ulFlags;

	/* Could not be invoked in process of initiazation. */
	if (IN_SYSINITIALIZATION())
	{
		goto __TERMINAL;
	}

	BUG_ON((NULL == DispBufferManager.disp_buffer) || 
		(NULL == buffer) || 
		(NULL == start));
	start_pos = *start;

	if (start_pos > DISPLAY_BUFFER_LENGTH)
	{
		/* Invalid start position. */
		goto __TERMINAL;
	}
	/* Requested length must not exceed defined buffer length. */
	if (get_length > DISPLAY_BUFFER_LENGTH)
	{
		get_length = DISPLAY_BUFFER_LENGTH;
	}

	/* Get from buffer one by one, under spin lock protected. */
	ret = 0;
	__ENTER_CRITICAL_SECTION_SMP(DispBufferManager.spin_lock, ulFlags);
	while (get_length--)
	{
		if (start_pos == DISPLAY_BUFFER_LENGTH)
		{
			start_pos = 0;
		}
		if (start_pos == DispBufferManager.buffer_rear)
		{
			/* Reach the end of buffer. */
			break;
		}
		buffer[ret++] = DispBufferManager.disp_buffer[start_pos++];
	}
	__LEAVE_CRITICAL_SECTION_SMP(DispBufferManager.spin_lock, ulFlags);
	
	/* 
	 * Return the position after fetching, so the  
	 * caller could invoke this routine again iterately.
	 */
	*start = start_pos;

__TERMINAL:
	return ret;
}

/*
 * Get some content from display buffer, block mode, @timeout
 * denotes how many million seconds to wait.
 * How many bytes got will be returned, the request length is
 * denoted by buffer_len.
 * @start denotes where to fetch from.
 */
static int __GetBufferBlock(char* buffer, int buffer_len, int* start, unsigned long timeout)
{
	/* Not implemented yet. */
	UNIMPLEMENTED_ROUTINE_CALLED;
	return -1;
}

/* Get current position of rear of buffer ring. */
static int __GetCurrentRear()
{
	return DispBufferManager.buffer_rear;
}

/* The global display buffer manager object. */
__DISPLAY_BUFFER_MANAGER DispBufferManager = {
#if defined(__CFG_SYS_SMP)
	SPIN_LOCK_INIT_VALUE,                 //spin_lock.
#endif 
	
	NULL,                                 //disp_buffer.
	0,                                    //buffer_head.
	0,                                    //buffer-rear.

	/* Operation routines. */
	__Initialize,                         //Initialize.
	__PutChar,                            //PutChar.
	__PutString,                          //PutString.
	__GetBuffer,                          //GetBuffer.
	__GetBufferBlock,                     //GetBufferBlock.
	__GetCurrentRear,                     //GetCurrentRear.
};
