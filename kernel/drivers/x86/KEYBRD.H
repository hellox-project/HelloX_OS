//------------------------------------------------------------------------
// Author              :     Garry.xin
// Date                :     12 DEC,2008
// Version             :     1.0
// Usage               :     The header file for keyboard driver,the implementation
//                           code is residing in keybrd.cpp file.
// Last modified author  :   Garry.Xin
// Last modified date    :   DEC 26,2011
// Last modified content :   Added the KERNEL_MESSAGE_TERMINAL's definition,the message
//                           will be sent to current focus thread when user press the
//                           ctrl + alt + del combination key.  
//------------------------------------------------------------------------

#ifndef __KEYBRD_H__
#define __KEYBRD_H__

//The main entry for keyboard driver.
BOOL KBDriverEntry(__DRIVER_OBJECT* pDriverObject);

//Interrupt vector of key board.
#define KB_INT_VECTOR 0x21

//ASCII key definition.
#define ASCII_NUT     0
#define ASCII_SOH     1
#define ASCII_STX     2
#define ASCII_ETX     3
#define ASCII_EOT     4
#define ASCII_ENQ     5
#define ASCII_ACK     6
#define ASCII_BEL     7
#define ASCII_BS      8
#define ASCII_HT      9
#define ASCII_LF      10
#define ASCII_VT      11
#define ASCII_FF      12
#define ASCII_CR      13
#define ASCII_SO      14
#define ASCII_SI      15
#define ASCII_DLE     16
#define ASCII_DC1     17
#define ASCII_DC2     18
#define ASCII_DC3     19
#define ASCII_DC4     20
#define ASCII_NAK     21
#define ASCII_SYN     22
#define ASCII_TB      23
#define ASCII_CAN     24
#define ASCII_EM      25
#define ASCII_SUB     26
#define ASCII_ESC     27
#define ASCII_FS      28
#define ASCII_GS      29
#define ASCII_RS      30
#define ASCII_US      31

#define ASCII_SPACE           32
#define ASCII_EXCALMARK       33   //!
#define ASCII_QUOTATIONMARK   34   //"
#define ASCII_POUND           35   //#
#define ASCII_DOLLAR          36   //$
#define ASCII_PERCENT         37   //%
#define ASCII_QUOTA           38   //&
#define ASCII_MINUTE          39   //'
#define ASCII_LEFT_BRACKET    40   //(
#define ASCII_RIGHT_BRACKET   41   //)
#define ASCII_STAR            42   //*
#define ASCII_ADD             43   //+
#define ASCII_COMMA           44   //,
#define ASCII_SUBSTRACT       45   //-
#define ASCII_DOT             46   //.
#define ASCII_REVERSE_SLASH   47   ///

#define ASCII_0               48
#define ASCII_1               49
#define ASCII_2               50
#define ASCII_3               51
#define ASCII_4               52
#define ASCII_5               53
#define ASCII_6               54
#define ASCII_7               55
#define ASCII_8               56
#define ASCII_9               57
#define ASCII_COLON           58   //:
#define ASCII_SEMICOLON       59   //;
#define ASCII_LESS            60   //<
#define ASCII_EQUAL           61   //=
#define ASCII_GREATER         62   //>
#define ASCII_ASK             63   //?
#define ASCII_AT              64   //@

//Messages to indicate which event has occured.
#define ASCII_KEY_DOWN          MSG_KEY_DOWN
#define ASCII_KEY_UP            MSG_KEY_UP
#define VIRTUAL_KEY_DOWN        203
#define VIRTUAL_KEY_UP          204
//#define KERNEL_MESSAGE_TERMINAL 5    //This message will be send to current
                                     //focus thread when user press the combination of
                                     //ctrl + alt + del keys.
#endif   //__KEYBRD_H__
