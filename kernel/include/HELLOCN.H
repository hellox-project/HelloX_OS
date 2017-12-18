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
	typedef VOID(*KEY_HANDLER)(DWORD);    //In this procedure,the parameter
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
#define VK_ESC             0x1B
#define VK_RETURN          0x0D
#define VK_TAB             0x09
#define VK_CAPS_LOCK       0x14
#define VK_SHIFT           0x10
#define VK_CONTROL         0x11
#define VK_MENU            0x12
#define VK_PAUSE           0x13
#define VK_CAPITAL         0x14

#define VK_SPACE           0x20
#define VK_BREAK           0x92
#define VK_PRINT           0x2A
#define VK_DOWN            0x28
#define VK_APPS            0x5D

#define VK_BACKSPACE       0x08
#define VK_LWIN            0x5B    //Left windows key in windows complying kb.
#define VK_RWIN            0x5C    //Right windows key.
#define VK_INSERT          0x2D
#define VK_HOME            0x24
#define VK_PAGEUP          0x21
#define VK_PAGEDOWN        0x22
#define VK_END             0x23
#define VK_DELETE          0x2E
#define VK_LEFTARROW       0x25
#define VK_UPARROW         0x26
#define VK_RIGHTARROW      0x27
#define VK_DOWNARROW       0x28
#define VK_ALT             0x05     //Alt key,defined by Hello China.

#define VK_F1              0x70
#define VK_F2              0x71
#define VK_F3              0x72
#define VK_F4              0x73
#define VK_F5              0x74
#define VK_F6              0x75
#define VK_F7              0x76
#define VK_F8              0x77
#define VK_F9              0x78
#define VK_F10             0x79
#define VK_F11             0x7A
#define VK_F12             0x7B
#define VK_F13             0x7C
#define VK_F14             0x7D
#define VK_F15             0x7E
#define VK_F16             0x7F
#define VK_F17             0x80
#define VK_F18             0x81
#define VK_F19             0x82
#define VK_F20             0x83
#define VK_F21             0x84
#define VK_F22             0x85
#define VK_F23             0x86
#define VK_F24             0x87

#define VK_NUMLOCK         0x90
#define VK_NUMPAD0         0x60
#define VK_NUMPAD1         0x61
#define VK_NUMPAD2         0x62
#define VK_NUMPAD3         0x63
#define VK_NUMPAD4         0x64
#define VK_NUMPAD5         0x65
#define VK_NUMPAD6         0x66
#define VK_NUMPAD7         0x67
#define VK_NUMPAD8         0x68
#define VK_NUMPAD9         0x69

#define VK_DECIMAL         0x6E
#define VK_NUMPADMULTIPLY  0x6A
#define VK_NUMPADADD       0x6B
#define VK_NUMPADSUBSTRACT 0x6E
#define VK_NUMPADDIVIDE    0x6F

#define VK_PAUSE           0x13
#define VK_SCROLL          0x91
#define VK_PRINTSC         0x06    //This value is defined by Hello China.

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

	VOID ErrorHandler(DWORD dwLevel, DWORD dwReason, LPSTR pszMsg);

	/* LOG levels. */
#define LOG_LEVEL_INFO    0x01
#define LOG_LEVEL_WARN    0x02
#define LOG_LEVEL_ERROR   0x03
#define LOG_LEVEL_FATAL   0x04

	/* Show out log header information. */
	void LogHeader(int level);

	/* Macro to print out log information. */
#define __LOG LogHeader(0); _hx_printf

	VOID __BUG(LPSTR, DWORD);
#define BUG() \
	__BUG(__FILE__,__LINE__)

#define BUG_ON(cond) \
    do{ \
        if(cond){ \
            BUG(); \
		        } \
	    }while(0)

	/* Show warning message when a empty routine is called. */
#define UNIMPLEMENTED_ROUTINE_CALLED \
	_hx_printf("[ERROR]:Unimplemented routine[%s] called,file:%s.\r\n",\
        __FUNCTION__,__FILE__);

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
	typedef VOID(*__GENERAL_INTERRUPT_HANDLER)(DWORD, LPVOID);

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
