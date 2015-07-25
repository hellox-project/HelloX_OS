//***********************************************************************/
//    Author                    : Garry
//    Original Date             : May,27 2004
//    Module Name               : hellocn.h
//    Module Funciton           : 
//                                This module countains the definations,
//                                data types,and procedures.
//    Last modified Author      : Garry
//    Last modified Date        : June 4 of 2013
//    Last modified Content     :
//                                1. All basic data typs are moved into types.h file,
//                                   this file will be obsoleted in future's version.
//    Lines number              :
//***********************************************************************/

#ifndef __HELLO_CHINA__
#define __HELLO_CHINA__

#ifdef __cplusplus
extern "C" {
#endif

#define __MINIKER_BASE      0x00100000    //Mini-kernal's base address.
                                                                
#define __MINIKER_LEN       48*1024       //The mini-kernal's length.
#define __MASTER_BASE       0x00110000    //The master's base address.

//PrintStr's base address.
#define __PRINTSTR_BASE     (__MINIKER_LEN + __MINIKER_BASE - 0x04)
//ChangeAttr's base address.
#define __CHANGEATTR_BASE   (__MINIKER_LEN + __MINIKER_BASE - 0x08)
//ClearScreen's base address.
#define __CLEARSCREEN_BASE  (__MINIKER_LEN + __MINIKER_BASE - 0x0c)
//PrintCh's base address.
#define __PRINTCH_BASE      (__MINIKER_LEN + __MINIKER_BASE - 0x10)
//GotoHome's base address.
#define __GOTOHOME_BASE     (__MINIKER_LEN + __MINIKER_BASE - 0x14)
//ChangeLine's base address.
#define __CHANGELINE_BASE   (__MINIKER_LEN + __MINIKER_BASE - 0x18)

#define __HEXTOSTR_BASE     (__MINIKER_LEN + __MINIKER_BASE - 0x1c)
#define __STRCPY_BASE       (__MINIKER_LEN + __MINIKER_BASE - 0x20)
#define __STRLEN_BASE       (__MINIKER_LEN + __MINIKER_BASE - 0x24)

//SetKeyHandler's base address.
#define __SETNOTIFYOS_BASE  __MINIKER_LEN + __MINIKER_BASE - 0x28

//GotoPrev's base address.
#define __GOTOPREV_BASE     __MINIKER_LEN + __MINIKER_BASE - 0x2c

//Timer handler's base address
#define __TIMERHANDLER_BASE (__MINIKER_LEN + __MINIKER_BASE - 0x30)

//Set GDT entry handler's base address.
#define __SETGDTENTRY_BASE  __MINIKER_LEN + __MINIKER_BASE - 0x34

//Clear GDT entry handler's base address.
#define __CLGDTENTRY_BASE   __MINIKER_LEN + __MINIKER_BASE - 0x38

//
//Handler procedure's defination.
//The key driver will call this procedure when a key down/up event occured.
//
typedef VOID (*KEY_HANDLER)(DWORD);    //In this procedure,the parameter
                                       //is a double word,it can be split
                                       //into two word,the high word is
                                       //reserved,not used,and the low
                                       //word is used as following:
                                       // low byte  : code,if this key
                                       //             event is a extend
                                       //             key,then it's the
                                       //             key's scan code,if
                                       //             the key is a ascii
                                       //             key,then it's the
                                       //             key's ascii code.
                                       // high byte : interrupt as following:
                                       // bit 1     : 1 if Shift key down
                                       // bit 2     : 1 if alt key down
                                       // bit 3     : 1 if ctrl key down
                                       // bit 4     : 1 if capslock key down
                                       // bit 5     : reserved
                                       // bit 6     : reserved
                                       // bit 7     : 1 if is a key up event
                                       // bit 8     : 1 if is a extend key.

typedef VOID(*INT_HANDLER)(DWORD);          //General interrupt handler.

//
//Virtual keys defined by OS kernel to handle different 
//physical key boards.
//
#define VK_ESC             27
#define VK_RETURN          13
#define VK_TAB             9
#define VK_CAPS_LOCK       20
#define VK_SHIFT           10
#define VK_CONTROL         17
#define VK_MENU            18    //Menu key in windows complying key board.
#define VK_BACKSPACE       8
#define VK_LWIN            91    //Left windows key in windows complying kb.
#define VK_RWIN            92    //Right windows key.
#define VK_INSERT          45
#define VK_HOME            36
#define VK_PAGEUP          33
#define VK_PAGEDOWN        34
#define VK_END             35
#define VK_DELETE          46
#define VK_LEFTARROW       37
#define VK_UPARROW         38
#define VK_RIGHTARROW      39
#define VK_DOWNARROW       40
#define VK_ALT             5     //Alt key,defined by Hello China.

#define VK_F1              112
#define VK_F2              113
#define VK_F3              114
#define VK_F4              115
#define VK_F5              116
#define VK_F6              117
#define VK_F7              118
#define VK_F8              119
#define VK_F9              120
#define VK_F10             121
#define VK_F11             122
#define VK_F12             123

#define VK_NUMLOCK         144
#define VK_NUMPAD0         96
#define VK_NUMPAD1         97
#define VK_NUMPAD2         98
#define VK_NUMPAD3         99
#define VK_NUMPAD4         100
#define VK_NUMPAD5         101
#define VK_NUMPAD6         102
#define VK_NUMPAD7         103
#define VK_NUMPAD8         104
#define VK_NUMPAD9         105
#define VK_DECIMAL         110
#define VK_NUMPADMULTIPLY  106
#define VK_NUMPADADD       107
#define VK_NUMPADSUBSTRACT 109
#define VK_NUMPADDIVIDE    111

#define VK_PAUSE           19
#define VK_SCROLL          145
#define VK_PRINTSC         6    //This value is defined by Hello China.

//Messages sent by keyboard driver to kernel or thread.
#define ASCII_KEY_DOWN          MSG_KEY_DOWN
#define ASCII_KEY_UP            MSG_KEY_UP
#define VIRTUAL_KEY_DOWN        203
#define VIRTUAL_KEY_UP          204

//
//Control bits.
//
#define KEY_SHIFT_STATUS       0x00000100
#define KEY_ALT_STATUS         0x00000200
#define KEY_CTRL_STATUS        0x00000400
#define KEY_CAPS_LOCK_STATUS   0x00000800

#define KEY_EVENT_UP           0x00004000
#define KEY_EVENT_EXTEND_KEY   0x00008000

//
//Macros to simple the programming.
//
#define ShiftKeyDown(para)     (para & KEY_SHIFT_STATUS)
#define AltKeyDown(para)       (para & KEY_ALT_STATUS)
#define CtrlKeyDown(para)      (para & KEY_CTRL_STATUS)
#define CapsLockKeyDown(para)  (para & KEY_CAPS_LOCK_STATUS)

#define KeyUpEvent(para)       (para & KEY_EVENT_UP)
#define IsExtendKey(para)      (para & KEY_EVENT_EXTEND_KEY)

//
//Error handling routines or macros definition.
//
#define __ERROR_HANDLER(level,reason,pszmsg) \
	ErrorHandler(level,reason,pszmsg)

VOID ErrorHandler(DWORD dwLevel,DWORD dwReason,LPSTR pszMsg);

VOID __BUG(LPSTR,DWORD);
#define BUG() \
	__BUG(__FILE__,__LINE__)

#define ERROR_LEVEL_FATAL       0x00000001
#define ERROR_LEVEL_CRITICAL    0x00000002
#define ERROR_LEVEL_IMPORTANT   0x00000004
#define ERROR_LEVEL_ALARM       0x00000008
#define ERROR_LEVEL_INFORM      0x00000010

//
//Some kernal mode procedures,implemented in Mini-kernal,the Master and
//device drivers can call these procedures directly.
//
//Print out a string at current position.
void PrintStr(const char* pszMsg);
//Clear the whole screen.
void ClearScreen(void);
//Print out a color character at current position.
void PrintCh(unsigned short ch);
//Goto the current line's home.
void GotoHome(void);
//Change to the next line.
void ChangeLine(void);
//Goto previous position.
void GotoPrev(void);

//General Interrupt handler pro-type.
typedef VOID (*__GENERAL_INTERRUPT_HANDLER)(DWORD,LPVOID);

//Set general interrupt handler.
INT_HANDLER SetGeneralIntHandler(__GENERAL_INTERRUPT_HANDLER);

//
//Write a byte to port.
//
VOID WriteByteToPort(UCHAR,            //The byte to write out.
					 WORD);            //I/O port.

//
//Read string of data from port.
//
VOID ReadByteStringFromPort(LPVOID,        //The local buffer to store string data.
			    			DWORD,         //How many bytes to be read.
				    		WORD);         //I/O port.

//
//Write string data to port.
//
VOID WriteByteStringToPort(LPVOID,         //The data buffer to be write out.
			    		   DWORD,          //How many byte to write out.
				    	   WORD);          //I/O port.

VOID ReadWordFromPort(WORD* pWord,
					  WORD  wPort);

VOID WriteWordToPort(WORD,
					 WORD);

VOID ReadWordStringFromPort(LPVOID,
							DWORD,
							WORD);

VOID WriteWordStringToPort(LPVOID,
						   DWORD,
						   WORD);

#ifdef __cplusplus
}
#endif
#endif //hellocn.h
