//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Sep 05,2018
//    Module Name               : pic.h
//    Module Funciton           : 
//                                Programming Interrupt Controller(PIC)'s
//                                constants and protypes.
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __PIC_H__
#define __PIC_H__

/* IO address of PIC 8259A. */
#define PIC1    0x20
#define PIC2    0xA0
#define PIC1_COMMAND PIC1
#define PIC2_COMMAND PIC2
#define PIC1_DATA (PIC1 + 1)
#define PIC2_DATA (PIC2 + 1)

/* Only vectors from 0 ~ 15 are maskable. */
#define __INTERRUPT_IS_MASKABLE(ucVector) ((ucVector >= 0) && (ucVector <= 15))

/* Routines to operate the PIC. */
/* Mask a specified interrupt. */
void __Set_Interrupt_Mask(unsigned char ucVector);

/* Unmask a specified interrupt. */
void __Clear_Interrupt_Mask(unsigned char ucVector);

/* 
 * Mask all interrupts,is called in begin of 
 * system initialization. 
 */
void __Mask_All();

#endif //__PIC_H__
