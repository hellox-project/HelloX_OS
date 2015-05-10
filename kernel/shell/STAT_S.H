//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Oct,22 2006
//    Module Name               : STAT_S.H
//    Module Funciton           : 
//                                Countains statistics thread's definition and
//                                implementation code.
//                                This is a kernel thread.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

extern __KERNEL_THREAD_OBJECT*  lpStatKernelThread;  //Global variables.

DWORD StatThreadRoutine(LPVOID lpData); //Entry point of the statistics thread.

#define STAT_MSG_SHOWSTATINFO    0x1001
#define STAT_MSG_GETSTATINFO     0x1002
#define STAT_MSG_TERMINAL        0x1003
#define STAT_MSG_LISTDEV         0x1004
#define STAT_MSG_SHOWMEMINFO     0x1005


