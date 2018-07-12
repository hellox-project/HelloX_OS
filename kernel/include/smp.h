//***********************************************************************/
//    Author                    : Garry
//    Original Date             : July 12,2018
//    Module Name               : smp.h
//    Module Funciton           : 
//                                Symmentric Multiple Processor related constants,
//                                structures and routines.
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __SMP_H__
#define __SMP_H__

/* spin lock and it's operations. */
typedef volatile unsigned int __SPIN_LOCK;

#define SPIN_LOCK_VALUE_UNLOCK 0
#define SPIN_LOCK_VALUE_LOCK 1

/* Initial value is unlocked. */
#define SPIN_LOCK_INIT_VALUE SPIN_LOCK_VALUE_UNLOCK

/* 
 * Architecture specific spin lock operations,must be implemented in
 * arch specific source code.
 * These routines must not be called directly except and only except the
 * macros follows.
 */
void __raw_spin_lock(__SPIN_LOCK* sl);
void __raw_spin_unlock(__SPIN_LOCK* sl);

/* 
 * Macros to operate the spin lock.
 * Declare and initialize a spin lock.
 */
#define __DECLARE_SPIN_LOCK(sl_name) __SPIN_LOCK sl_name = SPIN_LOCK_INIT_VALUE

/* Acquire and release the specified spin lock. */
#define __ACQUIRE_SPIN_LOCK(sl_name) __raw_spin_lock(&sl_name)
#define __RELEASE_SPIN_LOCK(sl_name) __raw_spin_unlock(&sl_name)

#endif //__SMP_H__
