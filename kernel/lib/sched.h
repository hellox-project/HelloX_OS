//***********************************************************************/
//    Author                    : Garry
//    Original Date             : April 17,2015
//    Module Name               : sched.h
//    Module Funciton           : 
//                                In order to port JamVM into HelloX,we must
//                                simulate POSIX schedule operations.
//                                This file defines POSIX schedule related data
//                                types and operation protypes.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __SCHED_H__
#define __SCHED_H__

//Yield the CPU to other processes or threads.
void sched_yield();


#endif  //__SCHED_H__
