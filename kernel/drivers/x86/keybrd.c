//------------------------------------------------------------------------
// Author    :    Garry.xin
// Date      :    12 DEC,2008
// Version   :    V1.0
// Usage     :    The implementation code of key board drivers.
//                A key board driver is implemented in this file,and integrated into
//                Hello China V1.6.
//                In consideration of less and less usage of Number Pad(small
//                number key board),and the conflict of scan code in NumPad and
//                normal key board,the Number Pad's implementation is skipped,so
//                you can not use number pad in your key board if you have.But this
//                issue should not be a problem since more and more key boards are
//                designed without number pad.
//------------------------------------------------------------------------
#ifndef __STDAFX_H__
#include <StdAfx.h>
#endif

#ifndef __KAPI_H__
#include <kapi.h>
#endif

#ifndef __KEYBRD_H__
#include "keybrd.h"
#endif


//This module will be available if and only if the DDF function is enabled.
#ifdef __CFG_SYS_DDF

//Enumrator to indicate the key is down or released.
typedef enum{
	eKEYUP = 0,
	eKEYDOWN
}KEY_UP_DOWN;

//Pre-declaration,implemented after the static variables.
static void AsciiKeyHandler(KEY_UP_DOWN event,unsigned char ScanCode);
static void VirtualKeyHandler(KEY_UP_DOWN,unsigned char);
static void ShiftHandler(KEY_UP_DOWN,unsigned char);
static void CapsLockHandler(KEY_UP_DOWN,unsigned char);
static void NumLockHandler(KEY_UP_DOWN,unsigned char);
static void AltHandler(KEY_UP_DOWN event,unsigned char);
static void CtrlHandler(KEY_UP_DOWN event,unsigned char);
static void DeleteHandler(KEY_UP_DOWN event,unsigned char);

//Map entry to associate the handler,scan code(is array index) and ascii code
//or VK value.
typedef struct{
	void (*Handler)(KEY_UP_DOWN,unsigned char);  //Handler for this key.
	unsigned char VK_Ascii;  //The ascii code or virtual key value.
}MAP_ENTRY;
//The map array,the index is key's scan code(make code),it's handler and 
//actual ASCII code or VK code are associated by it's content,which is an object
//of MAP_ENTRY.
static MAP_ENTRY KeyBoardMap[] = {
	{NULL,0},                    //No scan code 0,so this is an empty slot.
	{VirtualKeyHandler,VK_ESC},  //Escape key.
	{AsciiKeyHandler,'1'},       //SC 2
	{AsciiKeyHandler,'2'},       //SC 3
	{AsciiKeyHandler,'3'},       //4
	{AsciiKeyHandler,'4'},       //5
	{AsciiKeyHandler,'5'},       //6
	{AsciiKeyHandler,'6'},       //7
	{AsciiKeyHandler,'7'},       //8
	{AsciiKeyHandler,'8'},       //9
	{AsciiKeyHandler,'9'},       //A
	{AsciiKeyHandler,'0'},       //B
	{AsciiKeyHandler,'-'},       //C
	{AsciiKeyHandler,'='},       //D
	{AsciiKeyHandler,8},         //E,for BACKSPACE.
	{VirtualKeyHandler,VK_TAB},  //F
	{AsciiKeyHandler,'q'},       //10
	{AsciiKeyHandler,'w'},       //11
	{AsciiKeyHandler,'e'},       //12
	{AsciiKeyHandler,'r'},       //13
	{AsciiKeyHandler,'t'},       //14
	{AsciiKeyHandler,'y'},       //15
	{AsciiKeyHandler,'u'},       //16
	{AsciiKeyHandler,'i'},       //17
	{AsciiKeyHandler,'o'},       //18
	{AsciiKeyHandler,'p'},       //19
	{AsciiKeyHandler,'['},       //1A
	{AsciiKeyHandler,']'},       //1B
	{AsciiKeyHandler,13},        //1C,for RETURN.
	{CtrlHandler,VK_CONTROL},    //1D,for CTRL.
	{AsciiKeyHandler,'a'},       //1E
	{AsciiKeyHandler,'s'},       //1F
	{AsciiKeyHandler,'d'},       //20
	{AsciiKeyHandler,'f'},       //21
	{AsciiKeyHandler,'g'},       //22
	{AsciiKeyHandler,'h'},       //23
	{AsciiKeyHandler,'j'},       //24
	{AsciiKeyHandler,'k'},       //25
	{AsciiKeyHandler,'l'},       //26
	{AsciiKeyHandler,';'},       //27
	{AsciiKeyHandler,39},        //28,for '.
	{AsciiKeyHandler,'`'},       //29
	{ShiftHandler,VK_SHIFT},   //2A,for LEFT SHIFT.
	{AsciiKeyHandler,92},        //2B,for '\'.
	{AsciiKeyHandler,'z'},       //2C
	{AsciiKeyHandler,'x'},       //2D
	{AsciiKeyHandler,'c'},       //2E
	{AsciiKeyHandler,'v'},       //2F
	{AsciiKeyHandler,'b'},       //30
	{AsciiKeyHandler,'n'},       //31
	{AsciiKeyHandler,'m'},       //32
	{AsciiKeyHandler,','},       //33
	{AsciiKeyHandler,'.'},       //34
	{AsciiKeyHandler,'/'},       //35
	{ShiftHandler,VK_SHIFT},   //36,for RIGHT SHIFT.
	{VirtualKeyHandler,VK_PRINTSC}, //37,for PRINT SCREEN.
	{AltHandler,VK_ALT},       //38,for ALT.
	{AsciiKeyHandler,' '},       //39,for SPACE.
	{CapsLockHandler,VK_CAPS_LOCK},  //3A,for CAPS LOCK.
	{VirtualKeyHandler,VK_F1},   //3B
	{VirtualKeyHandler,VK_F2},   //3C
	{VirtualKeyHandler,VK_F3},   //3D
	{VirtualKeyHandler,VK_F4},   //3E
	{VirtualKeyHandler,VK_F5},   //3F
	{VirtualKeyHandler,VK_F6},   //40
	{VirtualKeyHandler,VK_F7},   //41
	{VirtualKeyHandler,VK_F8},   //42
	{VirtualKeyHandler,VK_F9},   //43
	//{VirtualKeyHandler,VK_F10},  //44
	{DeleteHandler,VK_F10},
	{VirtualKeyHandler,VK_PAUSE},    //45,for PAUSE/BREAK.
	{VirtualKeyHandler,VK_SCROLL},   //46,for SCROLL LOCK.
	{VirtualKeyHandler,VK_HOME},     //47
	{VirtualKeyHandler,VK_UPARROW},  //48,for UP ARROW.
	{VirtualKeyHandler,VK_PAGEUP},   //49
	{VirtualKeyHandler,VK_NUMPADSUBSTRACT}, //4A,for '-' in NUM PAD.
	{VirtualKeyHandler,VK_LEFTARROW},//4B
	{VirtualKeyHandler,VK_NUMPAD5},  //4C
	{VirtualKeyHandler,VK_RIGHTARROW},  //4D,for RIGHT ARROW.
	{VirtualKeyHandler,VK_NUMPADADD},   //4E,for '+' in NUM PAD.
	{VirtualKeyHandler,VK_END},      //4F,for END.
	{VirtualKeyHandler,VK_DOWNARROW},   //50
	{VirtualKeyHandler,VK_PAGEDOWN},    //51
	{VirtualKeyHandler,VK_INSERT},      //52
	{DeleteHandler,VK_DELETE},      //53
};

//Global variables.
static HANDLE g_hIntHandler  = NULL;

//Some flags to be handle especially.
typedef struct{
	unsigned long ShiftDown : 1;  //Status of shift key,1 for down and 0 for up.
	unsigned long CtrlDown  : 1;  //Status of Control key,1 for down and 0 for up.
	unsigned long AltDown   : 1;  //Status of Alt key,1 for down and 0 for up.
	unsigned long CapsLock  : 1;  //Status of CapsLock key,1 if this key pressed,
	                              //0 if this key pressed again.
	unsigned long NumLock   : 1;  //Status of NumLock.1 if this key pressed,
	                              //and restore to 0 if this key pressed again.
}KEY_FLAGS;
static KEY_FLAGS CtrlKeyFlags = {0};

//Flags to indicate where e1 or e0 key is received.
typedef struct{
	unsigned long e0Status : 1;   //Set to 1 if e0 received.
	unsigned long e1Status : 1;   //Set to 1 if e1 received.
}E0_E1_STATUS;
static E0_E1_STATUS e0e1Status = {0};

//A helper routine,to get the capital character given a lower case character.
static unsigned char GetCapitalChar(unsigned char lcc)
{
	if((lcc >= 'a') && (lcc <= 'z')) //English character.
	{
		return (lcc + ('A' - 'a'));
	}
	return lcc;
}

//Lighten or quench the indicating lights in key board.
static void LightOrQuench()
{
	unsigned char cmd = 0;
	if(CtrlKeyFlags.CapsLock)
	{
		cmd |= 0x04;
	}
	if(CtrlKeyFlags.NumLock)
	{
		cmd |= 0x02;
	}
#ifdef __I386__
#ifdef __GCC__
	__asm__ (
	".code32		;"
	"pushl 	%%eax	;"
	"pushl	%%edx	;"
	"movw	$0x60,	%%dx	;"
	"movb	$0xed,	%%al	;"
	"outb	%%al,	%%dx	;"
	"nop					;"
	"nop					;"
	"nop					;"
	"movb	%0,		%%al	;"
	"outb	%%al,	%%dx	;"
	"popl	%%edx			;"
	"popl	%%eax			;"
	:
	:"r"(cmd)
	);
#else
	__asm
	{
		push eax
		push edx
		mov dx,0x60
		mov al,0xed
		out dx,al
		nop
		nop
		nop
		mov al,cmd
		out dx,al
		pop edx
		pop eax
	}
#endif
#else
#endif
}

//A helper routine,gives the uper ASCII key in case of SHIFT key made and
//the dual value key(such as 1,2..) is pressed.
static unsigned char GetCapitalOther(unsigned char lcc)
{
	switch(lcc)  //Other chatacters.
	{
	case '`':
		return '~';
	case '1':
		return '!';
	case '2':
		return '@';
	case '3':
		return '#';
	case '4':
		return '$';
	case '5':
		return '%';
	case '6':
		return '^';
	case '7':
		return '&';
	case '8':
		return '*';
	case '9':
		return '(';
	case '0':
		return ')';
	case '-':
		return '_';
	case '=':
		return '+';
	case '[':
		return '{';
	case ']':
		return '}';
	case 92:         //For '\'.
		return '|';
	case ';':
		return  ':';
	case 39:        //For '.
		return '"';
	case ',':
		return '<';
	case '.':
		return '>';
	case '/':
		return '?';
	default:
		return lcc;
	}
}

//Key handler to process ASCII keys.
static void AsciiKeyHandler(KEY_UP_DOWN event,          //Key up or down.
							unsigned char ascii)        //Ascii code for this key.
{
	__DEVICE_MESSAGE dmsg;
	
	if(event == eKEYDOWN)  //Key is hold(make).
	{
		dmsg.wDevMsgType = ASCII_KEY_DOWN;
	}
	else  //Key is released.
	{
		dmsg.wDevMsgType = ASCII_KEY_UP;
	}

	if(CtrlKeyFlags.ShiftDown && CtrlKeyFlags.CapsLock)  //Both CapsLock and SHIFT
		                                                 //are hold.
	{
		ascii = GetCapitalOther(ascii);
	}
	if(CtrlKeyFlags.ShiftDown && (!CtrlKeyFlags.CapsLock)) //Only SHIFT key is hold.
	{
		if((ascii >= 'a') && (ascii <= 'z')) //Character.
		{
			ascii = GetCapitalChar(ascii);
		}
		else
		{
			ascii = GetCapitalOther(ascii);
		}
	}
	if((!CtrlKeyFlags.ShiftDown) && CtrlKeyFlags.CapsLock) //Only CapsLock hold.
	{
		if((ascii >= 'a') && (ascii <= 'z')) //Character.
		{
			ascii = GetCapitalChar(ascii);
		}
	}

	dmsg.dwDevMsgParam = (DWORD)ascii;
	DeviceInputManager.SendDeviceMessage(
		(__COMMON_OBJECT*)&DeviceInputManager,
		&dmsg,
		NULL); 
	return;
}

//Key handler to process virtual keys.
static void VirtualKeyHandler(KEY_UP_DOWN event,       //Key down or up.
							  unsigned char VKCode)    //Virtual Key code.
{
	__DEVICE_MESSAGE dmsg;
	
	if(event == eKEYDOWN)  //Key is hold(make).
	{
		dmsg.wDevMsgType = VIRTUAL_KEY_DOWN;
	}
	else  //Key is released.
	{
		dmsg.wDevMsgType = VIRTUAL_KEY_UP;
	}
	dmsg.dwDevMsgParam = (DWORD)VKCode;
	DeviceInputManager.SendDeviceMessage(
		(__COMMON_OBJECT*)&DeviceInputManager,
		&dmsg,
		NULL);
	return;
}

//Some special keys processing function.
static void ShiftHandler(KEY_UP_DOWN event,unsigned char VKCode)
{
	if(event == eKEYUP)
	{
		CtrlKeyFlags.ShiftDown = 0;
	}
	else  //Key down.
	{
		CtrlKeyFlags.ShiftDown = 1;
	}
	VirtualKeyHandler(event,VKCode);
	return;
}

static void CapsLockHandler(KEY_UP_DOWN event,unsigned char VKCode)
{
	if(event == eKEYDOWN)
	{
		CtrlKeyFlags.CapsLock = !CtrlKeyFlags.CapsLock;
		LightOrQuench();  //Lighten or quench CapsLock light.
	}
	VirtualKeyHandler(event,VKCode);
	return;
}

static void CtrlHandler(KEY_UP_DOWN event,unsigned char VKCode)
{
	if(event == eKEYDOWN)  //Ctrl key is hold now.
	{
		CtrlKeyFlags.CtrlDown = 1;
	}
	else  //Ctrl key is released.
	{
		CtrlKeyFlags.CtrlDown = 0;
	}
	VirtualKeyHandler(event,VKCode);
	return;
}

static void AltHandler(KEY_UP_DOWN event,unsigned char VKCode)
{
	if(event == eKEYDOWN)  //Alt key is hold now.
	{
		CtrlKeyFlags.AltDown = 1;
	}
	else
	{
		CtrlKeyFlags.AltDown = 0;
	}
	VirtualKeyHandler(event,VKCode);
	return;
}

static void NumLockHandler(KEY_UP_DOWN event,unsigned char VKCode)
{
	if(event == eKEYDOWN)  //NumLock key is pressed.
	{
		CtrlKeyFlags.NumLock = !CtrlKeyFlags.NumLock;
	}
	VirtualKeyHandler(event,VKCode);
	return;
}

//Handler for delete key,there is a exception case,since ctrl + alt + del
//key combination deemed as a special combination for most computer users.
//In this case,i.e,when the key combination occurs,the handler will send a
//extra message,KERNEL_MESSAGE_TERMINAL,to current focus thread.Therefore at
//least 2 messages are sent to current focus thread,one is KERNEL_MESSAGE_
//TERMINAL,another is the Virtual key UP/DOWN message.
static void DeleteHandler(KEY_UP_DOWN event,unsigned char VKCode)
{
	__DEVICE_MESSAGE msg;

	if(event == eKEYDOWN)
	{
		//When user press the ctrl + alt + del combination keys,
		//KERNEL_MESSAGE_TERMINAL will be sent to current focus
		//thread.
		if(CtrlKeyFlags.AltDown && CtrlKeyFlags.CtrlDown)
		{
			msg.wDevMsgType = KERNEL_MESSAGE_TERMINAL;
			DeviceInputManager.SendDeviceMessage(
				(__COMMON_OBJECT*)&DeviceInputManager,
				&msg,
				NULL);
			PrintLine("Control + Alt + Delete combined key pressed.");
		}
	}
	VirtualKeyHandler(event,VKCode);
}

//Initialize the key board.
//In current implementation,the initialization work is done by BIOS,so
//we no need do futher task here.
//But for other hardware platform,such as a cell phone,here we should
//do something to initialize the input device.
static BOOL InitKeyBoard()
{
	return TRUE;
}

//Get scan code from key board data register.
//static
unsigned char GetScanCode()
{
#ifdef __I386__
#ifdef __GCC__
	unsigned char code = 0;
	__asm__ volatile ("inb $0x60, %%al; movb %%al, %0" : "=m"(code) : );
	return code;

#else
	__asm { in al,0x60 };
#endif

#endif
}

//Acknowledge the key board controller.
static void AckKeyBoard()
{
#ifdef __I386__
#ifdef __GCC__
	__asm__ (
		".code32				;"
		"inb	$0x61,	%%al	;"
		"orb	$0x80,	%%al	;"
		"nop					;"
		"nop					;"
		"nop					;"
		"outb	%%al,	$0x61	;"
		"andb	$0x7F,	%%al	;"
		"nop					;"
		"nop					;"
		"nop					;"
		"outb	%%al,	$0x61	;"
		::
	);
#else
	__asm{
		in al,0x61
		or al,0x80
		nop
		nop
		nop
		out 0x61,al
		and al,0x7f
		nop
		nop
		nop
		out 0x61,al
	}
#endif
#else
#endif
}

//The main routine,used to process scan key code.
static VOID KBInputProcess(unsigned char sc)
{
	MAP_ENTRY me;
	KEY_UP_DOWN ud = eKEYDOWN;

	if(sc == 0xe0)  //Extension key pressed.
	{
		e0e1Status.e0Status = 1;  
		return;
	}
	if(sc == 0xe1)  //Extension key prefix for PAUSE/BREAK key.
	{
		e0e1Status.e1Status = 1;
		return;
	}

	if(e0e1Status.e0Status)  //The scan code for extension key.
	{
		if((sc == 0x2a) || (sc == 0xaa))  //The middum code for PrintScreen,skip it.
		{
			e0e1Status.e0Status = 0;
			return;
		}
		else
		{
			e0e1Status.e0Status = 0; //Clear the e0 status.
		}
	}
	if(e0e1Status.e1Status)  //e1 extension code sequence.
	{
		if((sc == 0x1d) || (sc == 0x9d)) //Middum code sequence,skip them.
		{
			return;
		}
		if(sc == 0x45) //Another e1 pending,clear the present one.
		{
			e0e1Status.e1Status = 0;
			return;
		}
		if(sc == 0xc5)  //Final scan code,use this code to lookup table.
		{
			e0e1Status.e1Status = 0;
			sc -= 0x80; //Process it as a key down event,since the release
			            //of PAUSE/BREAK will not send any scan code.
		}
	}

	if(sc > 0x80)  //Is a key release event.
	{
		sc -= 0x80;
		ud = eKEYUP;
	}

	if(sc > 0x53)
	{
		return;
	}
	me = KeyBoardMap[sc];
	me.Handler(ud,me.VK_Ascii);
}

//Interrupt handler for key board.
static BOOL KBIntHandler(LPVOID pParam,LPVOID pEsp)
{
	unsigned char sc = GetScanCode();
	KBInputProcess(sc);  //Process the scan code.
	AckKeyBoard();       //Acknowledge this interrupt.
	return TRUE;
}

//Unload entry for key board driver.
static DWORD KBDestroy(__COMMON_OBJECT* lpDriver,
					   __COMMON_OBJECT* lpDevice,
					   __DRCB*          lpDrcb)
{
	DisconnectInterrupt(g_hIntHandler);  //Release key board interrupt.
	if(lpDevice)
	{
		IOManager.DestroyDevice((__COMMON_OBJECT*)&IOManager,
			(__DEVICE_OBJECT*)lpDevice);
	}
	return 0;
}

//Main entry for key board drivers.
BOOL KBDriverEntry(__DRIVER_OBJECT* lpDriverObject)
{
	__DEVICE_OBJECT*  lpDevObject = NULL;
	BOOL              bResult     = FALSE;

	//Initialize the key board.
	if(!InitKeyBoard())
	{
		goto __TERMINAL;
	}

	g_hIntHandler = ConnectInterrupt(KBIntHandler,
		NULL,
		KB_INT_VECTOR);
	if(NULL == g_hIntHandler)  //Can not connect interrupt.
	{
		goto __TERMINAL;
	}

	//Create driver object for key board.
	lpDevObject = IOManager.CreateDevice((__COMMON_OBJECT*)&IOManager,
		"\\\\.\\KEYBRD",
		0,
		0,
		0,
		0,
		NULL,
		lpDriverObject);
	if(NULL == lpDevObject)  //Failed to create device object.
	{
		PrintLine("KeyBrd Driver: Failed to create device object for keyboard.");
		goto __TERMINAL;
	}

	//Asign call back functions of driver object.
	lpDriverObject->DeviceDestroy = KBDestroy;
	
	bResult = TRUE; //Indicate the whole process is successful.
__TERMINAL:
	if(!bResult)  //Should release all resource allocated above.
	{
		if(lpDevObject)
		{
			IOManager.DestroyDevice((__COMMON_OBJECT*)&IOManager,
				lpDevObject);
		}
		if(g_hIntHandler)
		{
			DisconnectInterrupt(g_hIntHandler);
			g_hIntHandler = NULL;  //Set to initial value.
		}
	}
	return bResult;
}

#endif
