//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Jun,10 2005
//    Module Name               : ioctrl_s.h
//    Module Funciton           : 
//                                ioctrl(I/O control) is a application in 
//                                kernel mode and is used for debugging device
//                                drivers mainly.
//                                It can view device configure space maped to
//                                system memory,and can change the content of
//                                a user specified location in memory.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __IOCTRL_S_H__
#define __IOCTRL_S_H__

/* Status code. */
#define IOCTRL_TERMINAL    0x00000001
#define IOCTRL_NORMAL      0x00000000

/* Entry point of ioctrl command. */
DWORD IoCtrlStart(LPVOID);

#endif //__IOCTRL_S_H__
