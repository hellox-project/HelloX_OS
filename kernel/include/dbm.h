//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Oct 28, 2021
//    Module Name               : dbm.h
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

#ifndef __DBM_H__
#define __DBM_H__

#include "config.h"
#include "TYPES.H"

/* Display buffer length. */
#define DISPLAY_BUFFER_LENGTH (64 * 1024)

/* Display buffer manager object. */
typedef struct tag__DISPLAY_BUFFER_MANAGER {
#if defined(__CFG_SYS_SMP)
	/* Spin lock to protect this object under SMP. */
	__SPIN_LOCK spin_lock;
#endif 

	/* 
	 * Buffer management varibles. This buffer 
	 * is a ring buffer, the new coming content will
	 * overwrite the content in buffer rear if full.
	 */
	char* disp_buffer;
	int buffer_head;
	int buffer_rear;

	/* Operation routines. */
	BOOL (*Initialize)();
	int (*PutChar)(char disp_char);
	int (*PutString)(const char* disp_string, int length);
	int (*GetBuffer)(char* buffer, int buffer_len, int* start);
	int (*GetBufferBlock)(char* buffer, int buffer_len, int *start, unsigned long timeout);
	int (*GetCurrentRear)();
}__DISPLAY_BUFFER_MANAGER;

/* Global display buffer manager object. */
extern __DISPLAY_BUFFER_MANAGER DispBufferManager;

#endif //__DBM_H__
