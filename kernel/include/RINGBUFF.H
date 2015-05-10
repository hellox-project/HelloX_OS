//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Nov,20 2007
//    Module Name               : RINGBUFF.H
//    Module Funciton           : 
//                                This module countains ring buffer object'sdefinition.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __RINGBUFF_H__
#define __RINGBUFF_H__

#ifdef __cplusplus
extern "C" {
#endif

//The definition of ring buffer object.
BEGIN_DEFINE_OBJECT(__RING_BUFFER)
    INHERIT_FROM_COMMON_OBJECT    //Inherit from common object.
	DWORD*          lpBuffer;     //Ring buffer.
    volatile DWORD  dwCount;      //How many elements in this buffer currently.
	DWORD           dwBuffLength; //Ring buffer's length.
	__EVENT*        eventWait;    //Event object to wait on.
	volatile DWORD  dwHeader;     //Ring buffer header pointer.
	volatile DWORD  dwTail;       //Ring buffer tail pointer.

	//Operation routines.
	BOOL            (*SetBuffLength)(__COMMON_OBJECT* lpThis,
		                             DWORD            dwNewLength);
	BOOL            (*GetElement)(__COMMON_OBJECT* lpThis,
		                          DWORD*           lpdwElement,
								  DWORD            dwMillionSecond);
	BOOL            (*AddElement)(__COMMON_OBJECT* lpThis,
		                          DWORD            dwElement);
END_DEFINE_OBJECT(__RING_BUFFER)

#define DEFAULT_RING_BUFFER_LENGTH 64  //The default length of the ring buffer,can be
                                       //changed by calling SetBuffLength.

//Initializing and uninitializing routines for ring buffer object.

BOOL RbInitialize(__COMMON_OBJECT* lpThis);
VOID RbUninitialize(__COMMON_OBJECT* lpThis);

#ifdef __cplusplus
}
#endif

#endif  //__RINGBUFF_H__
