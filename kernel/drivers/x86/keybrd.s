
keybrd.o：     文件格式 elf32-i386


Disassembly of section .text:

00000000 <GetCapitalOther>:

//A helper routine,gives the uper ASCII key in case of SHIFT key made and
//the dual value key(such as 1,2..) is pressed.
static unsigned char GetCapitalOther(unsigned char lcc)
{
	switch(lcc)  //Other chatacters.
   0:	8d 50 d9             	lea    -0x27(%eax),%edx
   3:	80 fa 39             	cmp    $0x39,%dl
   6:	77 15                	ja     1d <GetCapitalOther+0x1d>
   8:	0f b6 d2             	movzbl %dl,%edx
   b:	ff 24 95 00 00 00 00 	jmp    *0x0(,%edx,4)
  12:	8d b6 00 00 00 00    	lea    0x0(%esi),%esi
	{
	case '`':
		return '~';
  18:	b8 7e 00 00 00       	mov    $0x7e,%eax
	case '/':
		return '?';
	default:
		return lcc;
	}
}
  1d:	f3 c3                	repz ret 
  1f:	90                   	nop
	case '=':
		return '+';
	case '[':
		return '{';
	case ']':
		return '}';
  20:	b8 7d 00 00 00       	mov    $0x7d,%eax
  25:	c3                   	ret    
  26:	8d 76 00             	lea    0x0(%esi),%esi
  29:	8d bc 27 00 00 00 00 	lea    0x0(%edi,%eiz,1),%edi
	case 92:         //For '\'.
		return '|';
  30:	b8 7c 00 00 00       	mov    $0x7c,%eax
  35:	c3                   	ret    
  36:	8d 76 00             	lea    0x0(%esi),%esi
  39:	8d bc 27 00 00 00 00 	lea    0x0(%edi,%eiz,1),%edi
	case '-':
		return '_';
	case '=':
		return '+';
	case '[':
		return '{';
  40:	b8 7b 00 00 00       	mov    $0x7b,%eax
  45:	c3                   	ret    
  46:	8d 76 00             	lea    0x0(%esi),%esi
  49:	8d bc 27 00 00 00 00 	lea    0x0(%edi,%eiz,1),%edi
	case '0':
		return ')';
	case '-':
		return '_';
	case '=':
		return '+';
  50:	b8 2b 00 00 00       	mov    $0x2b,%eax
  55:	c3                   	ret    
  56:	8d 76 00             	lea    0x0(%esi),%esi
  59:	8d bc 27 00 00 00 00 	lea    0x0(%edi,%eiz,1),%edi
	case ']':
		return '}';
	case 92:         //For '\'.
		return '|';
	case ';':
		return  ':';
  60:	b8 3a 00 00 00       	mov    $0x3a,%eax
  65:	c3                   	ret    
  66:	8d 76 00             	lea    0x0(%esi),%esi
  69:	8d bc 27 00 00 00 00 	lea    0x0(%edi,%eiz,1),%edi
	case '7':
		return '&';
	case '8':
		return '*';
	case '9':
		return '(';
  70:	b8 28 00 00 00       	mov    $0x28,%eax
  75:	c3                   	ret    
  76:	8d 76 00             	lea    0x0(%esi),%esi
  79:	8d bc 27 00 00 00 00 	lea    0x0(%edi,%eiz,1),%edi
	case '6':
		return '^';
	case '7':
		return '&';
	case '8':
		return '*';
  80:	b8 2a 00 00 00       	mov    $0x2a,%eax
  85:	c3                   	ret    
  86:	8d 76 00             	lea    0x0(%esi),%esi
  89:	8d bc 27 00 00 00 00 	lea    0x0(%edi,%eiz,1),%edi
	case '5':
		return '%';
	case '6':
		return '^';
	case '7':
		return '&';
  90:	b8 26 00 00 00       	mov    $0x26,%eax
  95:	c3                   	ret    
  96:	8d 76 00             	lea    0x0(%esi),%esi
  99:	8d bc 27 00 00 00 00 	lea    0x0(%edi,%eiz,1),%edi
	case '4':
		return '$';
	case '5':
		return '%';
	case '6':
		return '^';
  a0:	b8 5e 00 00 00       	mov    $0x5e,%eax
  a5:	c3                   	ret    
  a6:	8d 76 00             	lea    0x0(%esi),%esi
  a9:	8d bc 27 00 00 00 00 	lea    0x0(%edi,%eiz,1),%edi
	case '3':
		return '#';
	case '4':
		return '$';
	case '5':
		return '%';
  b0:	b8 25 00 00 00       	mov    $0x25,%eax
  b5:	c3                   	ret    
  b6:	8d 76 00             	lea    0x0(%esi),%esi
  b9:	8d bc 27 00 00 00 00 	lea    0x0(%edi,%eiz,1),%edi
	case '2':
		return '@';
	case '3':
		return '#';
	case '4':
		return '$';
  c0:	b8 24 00 00 00       	mov    $0x24,%eax
  c5:	c3                   	ret    
  c6:	8d 76 00             	lea    0x0(%esi),%esi
  c9:	8d bc 27 00 00 00 00 	lea    0x0(%edi,%eiz,1),%edi
	case '1':
		return '!';
	case '2':
		return '@';
	case '3':
		return '#';
  d0:	b8 23 00 00 00       	mov    $0x23,%eax
  d5:	c3                   	ret    
  d6:	8d 76 00             	lea    0x0(%esi),%esi
  d9:	8d bc 27 00 00 00 00 	lea    0x0(%edi,%eiz,1),%edi
	case '`':
		return '~';
	case '1':
		return '!';
	case '2':
		return '@';
  e0:	b8 40 00 00 00       	mov    $0x40,%eax
  e5:	c3                   	ret    
  e6:	8d 76 00             	lea    0x0(%esi),%esi
  e9:	8d bc 27 00 00 00 00 	lea    0x0(%edi,%eiz,1),%edi
	switch(lcc)  //Other chatacters.
	{
	case '`':
		return '~';
	case '1':
		return '!';
  f0:	b8 21 00 00 00       	mov    $0x21,%eax
  f5:	c3                   	ret    
  f6:	8d 76 00             	lea    0x0(%esi),%esi
  f9:	8d bc 27 00 00 00 00 	lea    0x0(%edi,%eiz,1),%edi
	case '8':
		return '*';
	case '9':
		return '(';
	case '0':
		return ')';
 100:	b8 29 00 00 00       	mov    $0x29,%eax
 105:	c3                   	ret    
 106:	8d 76 00             	lea    0x0(%esi),%esi
 109:	8d bc 27 00 00 00 00 	lea    0x0(%edi,%eiz,1),%edi
	case ',':
		return '<';
	case '.':
		return '>';
	case '/':
		return '?';
 110:	b8 3f 00 00 00       	mov    $0x3f,%eax
 115:	c3                   	ret    
 116:	8d 76 00             	lea    0x0(%esi),%esi
 119:	8d bc 27 00 00 00 00 	lea    0x0(%edi,%eiz,1),%edi
	case 39:        //For '.
		return '"';
	case ',':
		return '<';
	case '.':
		return '>';
 120:	b8 3e 00 00 00       	mov    $0x3e,%eax
 125:	c3                   	ret    
 126:	8d 76 00             	lea    0x0(%esi),%esi
 129:	8d bc 27 00 00 00 00 	lea    0x0(%edi,%eiz,1),%edi
	case '9':
		return '(';
	case '0':
		return ')';
	case '-':
		return '_';
 130:	b8 5f 00 00 00       	mov    $0x5f,%eax
 135:	c3                   	ret    
 136:	8d 76 00             	lea    0x0(%esi),%esi
 139:	8d bc 27 00 00 00 00 	lea    0x0(%edi,%eiz,1),%edi
	case ';':
		return  ':';
	case 39:        //For '.
		return '"';
	case ',':
		return '<';
 140:	b8 3c 00 00 00       	mov    $0x3c,%eax
 145:	c3                   	ret    
 146:	8d 76 00             	lea    0x0(%esi),%esi
 149:	8d bc 27 00 00 00 00 	lea    0x0(%edi,%eiz,1),%edi
	case 92:         //For '\'.
		return '|';
	case ';':
		return  ':';
	case 39:        //For '.
		return '"';
 150:	b8 22 00 00 00       	mov    $0x22,%eax
 155:	c3                   	ret    
 156:	8d 76 00             	lea    0x0(%esi),%esi
 159:	8d bc 27 00 00 00 00 	lea    0x0(%edi,%eiz,1),%edi

00000160 <AsciiKeyHandler>:
}

//Key handler to process ASCII keys.
static void AsciiKeyHandler(KEY_UP_DOWN event,          //Key up or down.
							unsigned char ascii)        //Ascii code for this key.
{
 160:	83 ec 1c             	sub    $0x1c,%esp
	__DEVICE_MESSAGE dmsg;
	
	if(event == eKEYDOWN)  //Key is hold(make).
	{
		dmsg.wDevMsgType = ASCII_KEY_DOWN;
 163:	31 d2                	xor    %edx,%edx
 165:	83 7c 24 20 01       	cmpl   $0x1,0x20(%esp)
}

//Key handler to process ASCII keys.
static void AsciiKeyHandler(KEY_UP_DOWN event,          //Key up or down.
							unsigned char ascii)        //Ascii code for this key.
{
 16a:	8b 4c 24 24          	mov    0x24(%esp),%ecx
	__DEVICE_MESSAGE dmsg;
	
	if(event == eKEYDOWN)  //Key is hold(make).
	{
		dmsg.wDevMsgType = ASCII_KEY_DOWN;
 16e:	0f 95 c2             	setne  %dl
 171:	83 c2 01             	add    $0x1,%edx
 174:	66 89 54 24 08       	mov    %dx,0x8(%esp)
	else  //Key is released.
	{
		dmsg.wDevMsgType = ASCII_KEY_UP;
	}

	if(CtrlKeyFlags.ShiftDown && CtrlKeyFlags.CapsLock)  //Both CapsLock and SHIFT
 179:	0f b6 15 04 00 00 00 	movzbl 0x4,%edx
 180:	83 e2 09             	and    $0x9,%edx
 183:	80 fa 09             	cmp    $0x9,%dl
 186:	74 3f                	je     1c7 <AsciiKeyHandler+0x67>
		                                                 //are hold.
	{
		ascii = GetCapitalOther(ascii);
	}
	if(CtrlKeyFlags.ShiftDown && (!CtrlKeyFlags.CapsLock)) //Only SHIFT key is hold.
 188:	80 fa 01             	cmp    $0x1,%dl
 18b:	74 33                	je     1c0 <AsciiKeyHandler+0x60>
		else
		{
			ascii = GetCapitalOther(ascii);
		}
	}
	if((!CtrlKeyFlags.ShiftDown) && CtrlKeyFlags.CapsLock) //Only CapsLock hold.
 18d:	80 fa 08             	cmp    $0x8,%dl
 190:	89 c8                	mov    %ecx,%eax
 192:	75 0b                	jne    19f <AsciiKeyHandler+0x3f>
	{
		if((ascii >= 'a') && (ascii <= 'z')) //Character.
 194:	8d 51 9f             	lea    -0x61(%ecx),%edx
 197:	80 fa 19             	cmp    $0x19,%dl
 19a:	77 03                	ja     19f <AsciiKeyHandler+0x3f>
//A helper routine,to get the capital character given a lower case character.
static unsigned char GetCapitalChar(unsigned char lcc)
{
	if((lcc >= 'a') && (lcc <= 'z')) //English character.
	{
		return (lcc + ('A' - 'a'));
 19c:	8d 41 e0             	lea    -0x20(%ecx),%eax
		{
			ascii = GetCapitalChar(ascii);
		}
	}

	dmsg.dwDevMsgParam = (DWORD)ascii;
 19f:	0f b6 c0             	movzbl %al,%eax
	DeviceInputManager.SendDeviceMessage(
 1a2:	83 ec 04             	sub    $0x4,%esp
		{
			ascii = GetCapitalChar(ascii);
		}
	}

	dmsg.dwDevMsgParam = (DWORD)ascii;
 1a5:	89 44 24 10          	mov    %eax,0x10(%esp)
	DeviceInputManager.SendDeviceMessage(
 1a9:	6a 00                	push   $0x0
 1ab:	8d 44 24 10          	lea    0x10(%esp),%eax
 1af:	50                   	push   %eax
 1b0:	68 00 00 00 00       	push   $0x0
 1b5:	ff 15 08 00 00 00    	call   *0x8
		(__COMMON_OBJECT*)&DeviceInputManager,
		&dmsg,
		NULL); 
	return;
}
 1bb:	83 c4 2c             	add    $0x2c,%esp
 1be:	c3                   	ret    
 1bf:	90                   	nop
	{
		ascii = GetCapitalOther(ascii);
	}
	if(CtrlKeyFlags.ShiftDown && (!CtrlKeyFlags.CapsLock)) //Only SHIFT key is hold.
	{
		if((ascii >= 'a') && (ascii <= 'z')) //Character.
 1c0:	8d 41 9f             	lea    -0x61(%ecx),%eax
 1c3:	3c 19                	cmp    $0x19,%al
 1c5:	76 d5                	jbe    19c <AsciiKeyHandler+0x3c>
		{
			ascii = GetCapitalChar(ascii);
		}
		else
		{
			ascii = GetCapitalOther(ascii);
 1c7:	0f b6 c1             	movzbl %cl,%eax
 1ca:	e8 31 fe ff ff       	call   0 <GetCapitalOther>
 1cf:	eb ce                	jmp    19f <AsciiKeyHandler+0x3f>
 1d1:	eb 0d                	jmp    1e0 <VirtualKeyHandler>
 1d3:	90                   	nop
 1d4:	90                   	nop
 1d5:	90                   	nop
 1d6:	90                   	nop
 1d7:	90                   	nop
 1d8:	90                   	nop
 1d9:	90                   	nop
 1da:	90                   	nop
 1db:	90                   	nop
 1dc:	90                   	nop
 1dd:	90                   	nop
 1de:	90                   	nop
 1df:	90                   	nop

000001e0 <VirtualKeyHandler>:
}

//Key handler to process virtual keys.
static void VirtualKeyHandler(KEY_UP_DOWN event,       //Key down or up.
							  unsigned char VKCode)    //Virtual Key code.
{
 1e0:	83 ec 20             	sub    $0x20,%esp
	__DEVICE_MESSAGE dmsg;
	
	if(event == eKEYDOWN)  //Key is hold(make).
	{
		dmsg.wDevMsgType = VIRTUAL_KEY_DOWN;
 1e3:	31 c0                	xor    %eax,%eax
 1e5:	83 7c 24 24 01       	cmpl   $0x1,0x24(%esp)
 1ea:	0f 95 c0             	setne  %al
 1ed:	66 05 cb 00          	add    $0xcb,%ax
 1f1:	66 89 44 24 0c       	mov    %ax,0xc(%esp)
	}
	else  //Key is released.
	{
		dmsg.wDevMsgType = VIRTUAL_KEY_UP;
	}
	dmsg.dwDevMsgParam = (DWORD)VKCode;
 1f6:	0f b6 44 24 28       	movzbl 0x28(%esp),%eax
 1fb:	89 44 24 10          	mov    %eax,0x10(%esp)
	DeviceInputManager.SendDeviceMessage(
 1ff:	6a 00                	push   $0x0
 201:	8d 44 24 10          	lea    0x10(%esp),%eax
 205:	50                   	push   %eax
 206:	68 00 00 00 00       	push   $0x0
 20b:	ff 15 08 00 00 00    	call   *0x8
		(__COMMON_OBJECT*)&DeviceInputManager,
		&dmsg,
		NULL);
	return;
}
 211:	83 c4 2c             	add    $0x2c,%esp
 214:	c3                   	ret    
 215:	8d 74 26 00          	lea    0x0(%esi,%eiz,1),%esi
 219:	8d bc 27 00 00 00 00 	lea    0x0(%edi,%eiz,1),%edi

00000220 <ShiftHandler>:

//Some special keys processing function.
static void ShiftHandler(KEY_UP_DOWN event,unsigned char VKCode)
{
 220:	83 ec 1c             	sub    $0x1c,%esp
 223:	8b 54 24 20          	mov    0x20(%esp),%edx
 227:	8b 44 24 24          	mov    0x24(%esp),%eax
	if(event == eKEYUP)
 22b:	85 d2                	test   %edx,%edx
 22d:	75 31                	jne    260 <ShiftHandler+0x40>
	{
		CtrlKeyFlags.ShiftDown = 0;
 22f:	80 25 04 00 00 00 fe 	andb   $0xfe,0x4
	{
		dmsg.wDevMsgType = VIRTUAL_KEY_DOWN;
	}
	else  //Key is released.
	{
		dmsg.wDevMsgType = VIRTUAL_KEY_UP;
 236:	ba cc 00 00 00       	mov    $0xcc,%edx
 23b:	66 89 54 24 08       	mov    %dx,0x8(%esp)
	}
	dmsg.dwDevMsgParam = (DWORD)VKCode;
 240:	0f b6 c0             	movzbl %al,%eax
	DeviceInputManager.SendDeviceMessage(
 243:	83 ec 04             	sub    $0x4,%esp
	}
	else  //Key is released.
	{
		dmsg.wDevMsgType = VIRTUAL_KEY_UP;
	}
	dmsg.dwDevMsgParam = (DWORD)VKCode;
 246:	89 44 24 10          	mov    %eax,0x10(%esp)
	DeviceInputManager.SendDeviceMessage(
 24a:	6a 00                	push   $0x0
 24c:	8d 44 24 10          	lea    0x10(%esp),%eax
 250:	50                   	push   %eax
 251:	68 00 00 00 00       	push   $0x0
 256:	ff 15 08 00 00 00    	call   *0x8
	{
		CtrlKeyFlags.ShiftDown = 1;
	}
	VirtualKeyHandler(event,VKCode);
	return;
}
 25c:	83 c4 2c             	add    $0x2c,%esp
 25f:	c3                   	ret    
	{
		CtrlKeyFlags.ShiftDown = 0;
	}
	else  //Key down.
	{
		CtrlKeyFlags.ShiftDown = 1;
 260:	80 0d 04 00 00 00 01 	orb    $0x1,0x4
static void VirtualKeyHandler(KEY_UP_DOWN event,       //Key down or up.
							  unsigned char VKCode)    //Virtual Key code.
{
	__DEVICE_MESSAGE dmsg;
	
	if(event == eKEYDOWN)  //Key is hold(make).
 267:	83 fa 01             	cmp    $0x1,%edx
 26a:	75 ca                	jne    236 <ShiftHandler+0x16>
	{
		dmsg.wDevMsgType = VIRTUAL_KEY_DOWN;
 26c:	b9 cb 00 00 00       	mov    $0xcb,%ecx
 271:	66 89 4c 24 08       	mov    %cx,0x8(%esp)
 276:	eb c8                	jmp    240 <ShiftHandler+0x20>
 278:	90                   	nop
 279:	8d b4 26 00 00 00 00 	lea    0x0(%esi,%eiz,1),%esi

00000280 <CapsLockHandler>:
	VirtualKeyHandler(event,VKCode);
	return;
}

static void CapsLockHandler(KEY_UP_DOWN event,unsigned char VKCode)
{
 280:	83 ec 1c             	sub    $0x1c,%esp
	if(event == eKEYDOWN)
 283:	83 7c 24 20 01       	cmpl   $0x1,0x20(%esp)
	VirtualKeyHandler(event,VKCode);
	return;
}

static void CapsLockHandler(KEY_UP_DOWN event,unsigned char VKCode)
{
 288:	8b 44 24 24          	mov    0x24(%esp),%eax
	if(event == eKEYDOWN)
 28c:	74 32                	je     2c0 <CapsLockHandler+0x40>
	{
		dmsg.wDevMsgType = VIRTUAL_KEY_DOWN;
	}
	else  //Key is released.
	{
		dmsg.wDevMsgType = VIRTUAL_KEY_UP;
 28e:	ba cc 00 00 00       	mov    $0xcc,%edx
 293:	66 89 54 24 08       	mov    %dx,0x8(%esp)
	}
	dmsg.dwDevMsgParam = (DWORD)VKCode;
 298:	0f b6 c0             	movzbl %al,%eax
	DeviceInputManager.SendDeviceMessage(
 29b:	83 ec 04             	sub    $0x4,%esp
	}
	else  //Key is released.
	{
		dmsg.wDevMsgType = VIRTUAL_KEY_UP;
	}
	dmsg.dwDevMsgParam = (DWORD)VKCode;
 29e:	89 44 24 10          	mov    %eax,0x10(%esp)
	DeviceInputManager.SendDeviceMessage(
 2a2:	6a 00                	push   $0x0
 2a4:	8d 44 24 10          	lea    0x10(%esp),%eax
 2a8:	50                   	push   %eax
 2a9:	68 00 00 00 00       	push   $0x0
 2ae:	ff 15 08 00 00 00    	call   *0x8
	{
		CtrlKeyFlags.CapsLock = !CtrlKeyFlags.CapsLock;
		LightOrQuench();  //Lighten or quench CapsLock light.
	}
	VirtualKeyHandler(event,VKCode);
	return;
 2b4:	83 c4 10             	add    $0x10,%esp
}
 2b7:	83 c4 1c             	add    $0x1c,%esp
 2ba:	c3                   	ret    
 2bb:	90                   	nop
 2bc:	8d 74 26 00          	lea    0x0(%esi,%eiz,1),%esi

static void CapsLockHandler(KEY_UP_DOWN event,unsigned char VKCode)
{
	if(event == eKEYDOWN)
	{
		CtrlKeyFlags.CapsLock = !CtrlKeyFlags.CapsLock;
 2c0:	0f b6 15 04 00 00 00 	movzbl 0x4,%edx
 2c7:	89 d1                	mov    %edx,%ecx
 2c9:	83 e2 f7             	and    $0xfffffff7,%edx
 2cc:	f7 d1                	not    %ecx
 2ce:	83 e1 08             	and    $0x8,%ecx
 2d1:	09 ca                	or     %ecx,%edx

//Lighten or quench the indicating lights in key board.
static void LightOrQuench()
{
	unsigned char cmd = 0;
	if(CtrlKeyFlags.CapsLock)
 2d3:	89 d1                	mov    %edx,%ecx

static void CapsLockHandler(KEY_UP_DOWN event,unsigned char VKCode)
{
	if(event == eKEYDOWN)
	{
		CtrlKeyFlags.CapsLock = !CtrlKeyFlags.CapsLock;
 2d5:	88 15 04 00 00 00    	mov    %dl,0x4

//Lighten or quench the indicating lights in key board.
static void LightOrQuench()
{
	unsigned char cmd = 0;
	if(CtrlKeyFlags.CapsLock)
 2db:	83 e1 08             	and    $0x8,%ecx
	{
		cmd |= 0x04;
 2de:	80 f9 01             	cmp    $0x1,%cl
 2e1:	19 c9                	sbb    %ecx,%ecx
 2e3:	f7 d1                	not    %ecx
 2e5:	83 e1 04             	and    $0x4,%ecx
	}
	if(CtrlKeyFlags.NumLock)
 2e8:	83 e2 10             	and    $0x10,%edx
 2eb:	74 03                	je     2f0 <CapsLockHandler+0x70>
	{
		cmd |= 0x02;
 2ed:	83 c9 02             	or     $0x2,%ecx
	}
#ifdef __I386__
#ifdef _POSIX_
	asm (
 2f0:	50                   	push   %eax
 2f1:	52                   	push   %edx
 2f2:	66 ba 60 00          	mov    $0x60,%dx
 2f6:	b0 ed                	mov    $0xed,%al
 2f8:	ee                   	out    %al,(%dx)
 2f9:	90                   	nop
 2fa:	90                   	nop
 2fb:	90                   	nop
 2fc:	88 c8                	mov    %cl,%al
 2fe:	ee                   	out    %al,(%dx)
 2ff:	5a                   	pop    %edx
 300:	58                   	pop    %eax
{
	__DEVICE_MESSAGE dmsg;
	
	if(event == eKEYDOWN)  //Key is hold(make).
	{
		dmsg.wDevMsgType = VIRTUAL_KEY_DOWN;
 301:	b9 cb 00 00 00       	mov    $0xcb,%ecx
 306:	66 89 4c 24 08       	mov    %cx,0x8(%esp)
 30b:	eb 8b                	jmp    298 <CapsLockHandler+0x18>
 30d:	8d 76 00             	lea    0x0(%esi),%esi

00000310 <CtrlHandler>:
	VirtualKeyHandler(event,VKCode);
	return;
}

static void CtrlHandler(KEY_UP_DOWN event,unsigned char VKCode)
{
 310:	83 ec 1c             	sub    $0x1c,%esp
	if(event == eKEYDOWN)  //Ctrl key is hold now.
 313:	83 7c 24 20 01       	cmpl   $0x1,0x20(%esp)
	VirtualKeyHandler(event,VKCode);
	return;
}

static void CtrlHandler(KEY_UP_DOWN event,unsigned char VKCode)
{
 318:	8b 44 24 24          	mov    0x24(%esp),%eax
	if(event == eKEYDOWN)  //Ctrl key is hold now.
 31c:	74 32                	je     350 <CtrlHandler+0x40>
	{
		CtrlKeyFlags.CtrlDown = 1;
	}
	else  //Ctrl key is released.
	{
		CtrlKeyFlags.CtrlDown = 0;
 31e:	80 25 04 00 00 00 fd 	andb   $0xfd,0x4
	{
		dmsg.wDevMsgType = VIRTUAL_KEY_DOWN;
	}
	else  //Key is released.
	{
		dmsg.wDevMsgType = VIRTUAL_KEY_UP;
 325:	ba cc 00 00 00       	mov    $0xcc,%edx
 32a:	66 89 54 24 08       	mov    %dx,0x8(%esp)
	}
	dmsg.dwDevMsgParam = (DWORD)VKCode;
 32f:	0f b6 c0             	movzbl %al,%eax
	DeviceInputManager.SendDeviceMessage(
 332:	83 ec 04             	sub    $0x4,%esp
	}
	else  //Key is released.
	{
		dmsg.wDevMsgType = VIRTUAL_KEY_UP;
	}
	dmsg.dwDevMsgParam = (DWORD)VKCode;
 335:	89 44 24 10          	mov    %eax,0x10(%esp)
	DeviceInputManager.SendDeviceMessage(
 339:	6a 00                	push   $0x0
 33b:	8d 44 24 10          	lea    0x10(%esp),%eax
 33f:	50                   	push   %eax
 340:	68 00 00 00 00       	push   $0x0
 345:	ff 15 08 00 00 00    	call   *0x8
	{
		CtrlKeyFlags.CtrlDown = 0;
	}
	VirtualKeyHandler(event,VKCode);
	return;
}
 34b:	83 c4 2c             	add    $0x2c,%esp
 34e:	c3                   	ret    
 34f:	90                   	nop
{
	__DEVICE_MESSAGE dmsg;
	
	if(event == eKEYDOWN)  //Key is hold(make).
	{
		dmsg.wDevMsgType = VIRTUAL_KEY_DOWN;
 350:	b9 cb 00 00 00       	mov    $0xcb,%ecx

static void CtrlHandler(KEY_UP_DOWN event,unsigned char VKCode)
{
	if(event == eKEYDOWN)  //Ctrl key is hold now.
	{
		CtrlKeyFlags.CtrlDown = 1;
 355:	80 0d 04 00 00 00 02 	orb    $0x2,0x4
{
	__DEVICE_MESSAGE dmsg;
	
	if(event == eKEYDOWN)  //Key is hold(make).
	{
		dmsg.wDevMsgType = VIRTUAL_KEY_DOWN;
 35c:	66 89 4c 24 08       	mov    %cx,0x8(%esp)
 361:	eb cc                	jmp    32f <CtrlHandler+0x1f>
 363:	8d b6 00 00 00 00    	lea    0x0(%esi),%esi
 369:	8d bc 27 00 00 00 00 	lea    0x0(%edi,%eiz,1),%edi

00000370 <AltHandler>:
	VirtualKeyHandler(event,VKCode);
	return;
}

static void AltHandler(KEY_UP_DOWN event,unsigned char VKCode)
{
 370:	83 ec 1c             	sub    $0x1c,%esp
	if(event == eKEYDOWN)  //Alt key is hold now.
 373:	83 7c 24 20 01       	cmpl   $0x1,0x20(%esp)
	VirtualKeyHandler(event,VKCode);
	return;
}

static void AltHandler(KEY_UP_DOWN event,unsigned char VKCode)
{
 378:	8b 44 24 24          	mov    0x24(%esp),%eax
	if(event == eKEYDOWN)  //Alt key is hold now.
 37c:	74 32                	je     3b0 <AltHandler+0x40>
	{
		CtrlKeyFlags.AltDown = 1;
	}
	else
	{
		CtrlKeyFlags.AltDown = 0;
 37e:	80 25 04 00 00 00 fb 	andb   $0xfb,0x4
	{
		dmsg.wDevMsgType = VIRTUAL_KEY_DOWN;
	}
	else  //Key is released.
	{
		dmsg.wDevMsgType = VIRTUAL_KEY_UP;
 385:	ba cc 00 00 00       	mov    $0xcc,%edx
 38a:	66 89 54 24 08       	mov    %dx,0x8(%esp)
	}
	dmsg.dwDevMsgParam = (DWORD)VKCode;
 38f:	0f b6 c0             	movzbl %al,%eax
	DeviceInputManager.SendDeviceMessage(
 392:	83 ec 04             	sub    $0x4,%esp
	}
	else  //Key is released.
	{
		dmsg.wDevMsgType = VIRTUAL_KEY_UP;
	}
	dmsg.dwDevMsgParam = (DWORD)VKCode;
 395:	89 44 24 10          	mov    %eax,0x10(%esp)
	DeviceInputManager.SendDeviceMessage(
 399:	6a 00                	push   $0x0
 39b:	8d 44 24 10          	lea    0x10(%esp),%eax
 39f:	50                   	push   %eax
 3a0:	68 00 00 00 00       	push   $0x0
 3a5:	ff 15 08 00 00 00    	call   *0x8
	{
		CtrlKeyFlags.AltDown = 0;
	}
	VirtualKeyHandler(event,VKCode);
	return;
}
 3ab:	83 c4 2c             	add    $0x2c,%esp
 3ae:	c3                   	ret    
 3af:	90                   	nop
{
	__DEVICE_MESSAGE dmsg;
	
	if(event == eKEYDOWN)  //Key is hold(make).
	{
		dmsg.wDevMsgType = VIRTUAL_KEY_DOWN;
 3b0:	b9 cb 00 00 00       	mov    $0xcb,%ecx

static void AltHandler(KEY_UP_DOWN event,unsigned char VKCode)
{
	if(event == eKEYDOWN)  //Alt key is hold now.
	{
		CtrlKeyFlags.AltDown = 1;
 3b5:	80 0d 04 00 00 00 04 	orb    $0x4,0x4
{
	__DEVICE_MESSAGE dmsg;
	
	if(event == eKEYDOWN)  //Key is hold(make).
	{
		dmsg.wDevMsgType = VIRTUAL_KEY_DOWN;
 3bc:	66 89 4c 24 08       	mov    %cx,0x8(%esp)
 3c1:	eb cc                	jmp    38f <AltHandler+0x1f>
 3c3:	8d b6 00 00 00 00    	lea    0x0(%esi),%esi
 3c9:	8d bc 27 00 00 00 00 	lea    0x0(%edi,%eiz,1),%edi

000003d0 <KBIntHandler>:
	me.Handler(ud,me.VK_Ascii);
}

//Interrupt handler for key board.
static BOOL KBIntHandler(LPVOID pParam,LPVOID pEsp)
{
 3d0:	83 ec 1c             	sub    $0x1c,%esp

#ifdef __I386__
#ifdef _POSIX_
	unsigned char code;

	asm volatile (
 3d3:	31 c0                	xor    %eax,%eax
 3d5:	e4 60                	in     $0x60,%al
 3d7:	88 44 24 0f          	mov    %al,0xf(%esp)
	{
		e0e1Status.e1Status = 1;
		return;
	}

	if(e0e1Status.e0Status)  //The scan code for extension key.
 3db:	0f b6 05 00 00 00 00 	movzbl 0x0,%eax
 3e2:	a8 01                	test   $0x1,%al
 3e4:	74 08                	je     3ee <KBIntHandler+0x1e>
			e0e1Status.e0Status = 0;
			return;
		}
		else
		{
			e0e1Status.e0Status = 0; //Clear the e0 status.
 3e6:	83 e0 fe             	and    $0xfffffffe,%eax
 3e9:	a2 00 00 00 00       	mov    %al,0x0
	if(sc > 0x53)
	{
		return;
	}
	me = KeyBoardMap[sc];
	me.Handler(ud,me.VK_Ascii);
 3ee:	0f b6 05 2c 02 00 00 	movzbl 0x22c,%eax
 3f5:	83 ec 08             	sub    $0x8,%esp
 3f8:	50                   	push   %eax
 3f9:	6a 01                	push   $0x1
 3fb:	ff 15 28 02 00 00    	call   *0x228
//Acknowledge the key board controller.
static void AckKeyBoard()
{
#ifdef __I386__
#ifdef _POSIX_
	asm (
 401:	e4 61                	in     $0x61,%al
 403:	0c 80                	or     $0x80,%al
 405:	90                   	nop
 406:	90                   	nop
 407:	90                   	nop
 408:	e6 61                	out    %al,$0x61
 40a:	24 7f                	and    $0x7f,%al
 40c:	90                   	nop
 40d:	90                   	nop
 40e:	90                   	nop
 40f:	e6 61                	out    %al,$0x61
{
	unsigned char sc = GetScanCode();
	KBInputProcess(sc);  //Process the scan code.
	AckKeyBoard();       //Acknowledge this interrupt.
	return TRUE;
}
 411:	b8 01 00 00 00       	mov    $0x1,%eax
 416:	83 c4 2c             	add    $0x2c,%esp
 419:	c3                   	ret    
 41a:	8d b6 00 00 00 00    	lea    0x0(%esi),%esi

00000420 <DeleteHandler>:
//In this case,i.e,when the key combination occurs,the handler will send a
//extra message,KERNEL_MESSAGE_TERMINAL,to current focus thread.Therefore at
//least 2 messages are sent to current focus thread,one is KERNEL_MESSAGE_
//TERMINAL,another is the Virtual key UP/DOWN message.
static void DeleteHandler(KEY_UP_DOWN event,unsigned char VKCode)
{
 420:	53                   	push   %ebx
 421:	83 ec 18             	sub    $0x18,%esp
	__DEVICE_MESSAGE msg;

	if(event == eKEYDOWN)
 424:	83 7c 24 20 01       	cmpl   $0x1,0x20(%esp)
//In this case,i.e,when the key combination occurs,the handler will send a
//extra message,KERNEL_MESSAGE_TERMINAL,to current focus thread.Therefore at
//least 2 messages are sent to current focus thread,one is KERNEL_MESSAGE_
//TERMINAL,another is the Virtual key UP/DOWN message.
static void DeleteHandler(KEY_UP_DOWN event,unsigned char VKCode)
{
 429:	8b 5c 24 24          	mov    0x24(%esp),%ebx
	__DEVICE_MESSAGE msg;

	if(event == eKEYDOWN)
 42d:	74 31                	je     460 <DeleteHandler+0x40>
	{
		dmsg.wDevMsgType = VIRTUAL_KEY_DOWN;
	}
	else  //Key is released.
	{
		dmsg.wDevMsgType = VIRTUAL_KEY_UP;
 42f:	ba cc 00 00 00       	mov    $0xcc,%edx
 434:	66 89 54 24 08       	mov    %dx,0x8(%esp)
	}
	dmsg.dwDevMsgParam = (DWORD)VKCode;
 439:	0f b6 db             	movzbl %bl,%ebx
	DeviceInputManager.SendDeviceMessage(
 43c:	83 ec 04             	sub    $0x4,%esp
	}
	else  //Key is released.
	{
		dmsg.wDevMsgType = VIRTUAL_KEY_UP;
	}
	dmsg.dwDevMsgParam = (DWORD)VKCode;
 43f:	89 5c 24 10          	mov    %ebx,0x10(%esp)
	DeviceInputManager.SendDeviceMessage(
 443:	6a 00                	push   $0x0
 445:	8d 44 24 10          	lea    0x10(%esp),%eax
 449:	50                   	push   %eax
 44a:	68 00 00 00 00       	push   $0x0
 44f:	ff 15 08 00 00 00    	call   *0x8
				NULL);
			PrintLine("Control + Alt + Delete combined key pressed.");
		}
	}
	VirtualKeyHandler(event,VKCode);
}
 455:	83 c4 10             	add    $0x10,%esp
 458:	83 c4 18             	add    $0x18,%esp
 45b:	5b                   	pop    %ebx
 45c:	c3                   	ret    
 45d:	8d 76 00             	lea    0x0(%esi),%esi
	if(event == eKEYDOWN)
	{
		//When user press the ctrl + alt + del combination keys,
		//KERNEL_MESSAGE_TERMINAL will be sent to current focus
		//thread.
		if(CtrlKeyFlags.AltDown && CtrlKeyFlags.CtrlDown)
 460:	0f b6 05 04 00 00 00 	movzbl 0x4,%eax
 467:	83 e0 06             	and    $0x6,%eax
 46a:	3c 06                	cmp    $0x6,%al
 46c:	74 12                	je     480 <DeleteHandler+0x60>
{
	__DEVICE_MESSAGE dmsg;
	
	if(event == eKEYDOWN)  //Key is hold(make).
	{
		dmsg.wDevMsgType = VIRTUAL_KEY_DOWN;
 46e:	b8 cb 00 00 00       	mov    $0xcb,%eax
 473:	66 89 44 24 08       	mov    %ax,0x8(%esp)
 478:	eb bf                	jmp    439 <DeleteHandler+0x19>
 47a:	8d b6 00 00 00 00    	lea    0x0(%esi),%esi
		//When user press the ctrl + alt + del combination keys,
		//KERNEL_MESSAGE_TERMINAL will be sent to current focus
		//thread.
		if(CtrlKeyFlags.AltDown && CtrlKeyFlags.CtrlDown)
		{
			msg.wDevMsgType = KERNEL_MESSAGE_TERMINAL;
 480:	b9 05 00 00 00       	mov    $0x5,%ecx
			DeviceInputManager.SendDeviceMessage(
 485:	83 ec 04             	sub    $0x4,%esp
		//When user press the ctrl + alt + del combination keys,
		//KERNEL_MESSAGE_TERMINAL will be sent to current focus
		//thread.
		if(CtrlKeyFlags.AltDown && CtrlKeyFlags.CtrlDown)
		{
			msg.wDevMsgType = KERNEL_MESSAGE_TERMINAL;
 488:	66 89 4c 24 04       	mov    %cx,0x4(%esp)
			DeviceInputManager.SendDeviceMessage(
 48d:	6a 00                	push   $0x0
 48f:	8d 44 24 08          	lea    0x8(%esp),%eax
 493:	50                   	push   %eax
 494:	68 00 00 00 00       	push   $0x0
 499:	ff 15 08 00 00 00    	call   *0x8
				(__COMMON_OBJECT*)&DeviceInputManager,
				&msg,
				NULL);
			PrintLine("Control + Alt + Delete combined key pressed.");
 49f:	c7 04 24 00 00 00 00 	movl   $0x0,(%esp)
 4a6:	e8 fc ff ff ff       	call   4a7 <DeleteHandler+0x87>
 4ab:	83 c4 10             	add    $0x10,%esp
 4ae:	eb be                	jmp    46e <DeleteHandler+0x4e>

000004b0 <KBDestroy>:

//Unload entry for key board driver.
static DWORD KBDestroy(__COMMON_OBJECT* lpDriver,
					   __COMMON_OBJECT* lpDevice,
					   __DRCB*          lpDrcb)
{
 4b0:	53                   	push   %ebx
 4b1:	83 ec 14             	sub    $0x14,%esp
 4b4:	8b 5c 24 20          	mov    0x20(%esp),%ebx
	DisconnectInterrupt(g_hIntHandler);  //Release key board interrupt.
 4b8:	ff 35 08 00 00 00    	pushl  0x8
 4be:	e8 fc ff ff ff       	call   4bf <KBDestroy+0xf>
	if(lpDevice)
 4c3:	83 c4 10             	add    $0x10,%esp
 4c6:	85 db                	test   %ebx,%ebx
 4c8:	74 12                	je     4dc <KBDestroy+0x2c>
	{
		IOManager.DestroyDevice((__COMMON_OBJECT*)&IOManager,
 4ca:	83 ec 08             	sub    $0x8,%esp
 4cd:	53                   	push   %ebx
 4ce:	68 00 00 00 00       	push   $0x0
 4d3:	ff 15 24 02 00 00    	call   *0x224
 4d9:	83 c4 10             	add    $0x10,%esp
			(__DEVICE_OBJECT*)lpDevice);
	}
	return 0;
}
 4dc:	83 c4 08             	add    $0x8,%esp
 4df:	31 c0                	xor    %eax,%eax
 4e1:	5b                   	pop    %ebx
 4e2:	c3                   	ret    
 4e3:	8d b6 00 00 00 00    	lea    0x0(%esi),%esi
 4e9:	8d bc 27 00 00 00 00 	lea    0x0(%edi,%eiz,1),%edi

000004f0 <GetScanCode>:
}

//Get scan code from key board data register.
//static
unsigned char GetScanCode()
{
 4f0:	83 ec 10             	sub    $0x10,%esp

#ifdef __I386__
#ifdef _POSIX_
	unsigned char code;

	asm volatile (
 4f3:	31 c0                	xor    %eax,%eax
 4f5:	e4 60                	in     $0x60,%al
 4f7:	88 44 24 0f          	mov    %al,0xf(%esp)
#else
	asm { in al,0x60 };
#endif

#endif
}
 4fb:	83 c4 10             	add    $0x10,%esp
 4fe:	c3                   	ret    
 4ff:	90                   	nop

00000500 <KBDriverEntry>:
	return 0;
}

//Main entry for key board drivers.
BOOL KBDriverEntry(__DRIVER_OBJECT* lpDriverObject)
{
 500:	56                   	push   %esi
 501:	53                   	push   %ebx
 502:	31 db                	xor    %ebx,%ebx
 504:	83 ec 08             	sub    $0x8,%esp
 507:	8b 74 24 14          	mov    0x14(%esp),%esi
	if(!InitKeyBoard())
	{
		goto __TERMINAL;
	}

	g_hIntHandler = ConnectInterrupt(KBIntHandler,
 50b:	6a 21                	push   $0x21
 50d:	6a 00                	push   $0x0
 50f:	68 d0 03 00 00       	push   $0x3d0
 514:	e8 fc ff ff ff       	call   515 <KBDriverEntry+0x15>
		NULL,
		KB_INT_VECTOR);
	if(NULL == g_hIntHandler)  //Can not connect interrupt.
 519:	83 c4 10             	add    $0x10,%esp
 51c:	85 c0                	test   %eax,%eax
	if(!InitKeyBoard())
	{
		goto __TERMINAL;
	}

	g_hIntHandler = ConnectInterrupt(KBIntHandler,
 51e:	a3 08 00 00 00       	mov    %eax,0x8
		NULL,
		KB_INT_VECTOR);
	if(NULL == g_hIntHandler)  //Can not connect interrupt.
 523:	74 2e                	je     553 <KBDriverEntry+0x53>
	{
		goto __TERMINAL;
	}

	//Create driver object for key board.
	lpDevObject = IOManager.CreateDevice((__COMMON_OBJECT*)&IOManager,
 525:	56                   	push   %esi
 526:	6a 00                	push   $0x0
 528:	6a 00                	push   $0x0
 52a:	6a 00                	push   $0x0
 52c:	6a 00                	push   $0x0
 52e:	6a 00                	push   $0x0
 530:	68 00 00 00 00       	push   $0x0
 535:	68 00 00 00 00       	push   $0x0
 53a:	ff 15 20 02 00 00    	call   *0x220
		0,
		0,
		0,
		NULL,
		lpDriverObject);
	if(NULL == lpDevObject)  //Failed to create device object.
 540:	83 c4 20             	add    $0x20,%esp
 543:	85 c0                	test   %eax,%eax
 545:	74 19                	je     560 <KBDriverEntry+0x60>
		PrintLine("KeyBrd Driver: Failed to create device object for keyboard.");
		goto __TERMINAL;
	}

	//Asign call back functions of driver object.
	lpDriverObject->DeviceDestroy = KBDestroy;
 547:	c7 46 4c b0 04 00 00 	movl   $0x4b0,0x4c(%esi)
	
	bResult = TRUE; //Indicate the whole process is successful.
 54e:	bb 01 00 00 00       	mov    $0x1,%ebx
			DisconnectInterrupt(g_hIntHandler);
			g_hIntHandler = NULL;  //Set to initial value.
		}
	}
	return bResult;
}
 553:	83 c4 04             	add    $0x4,%esp
 556:	89 d8                	mov    %ebx,%eax
 558:	5b                   	pop    %ebx
 559:	5e                   	pop    %esi
 55a:	c3                   	ret    
 55b:	90                   	nop
 55c:	8d 74 26 00          	lea    0x0(%esi,%eiz,1),%esi
		0,
		NULL,
		lpDriverObject);
	if(NULL == lpDevObject)  //Failed to create device object.
	{
		PrintLine("KeyBrd Driver: Failed to create device object for keyboard.");
 560:	83 ec 0c             	sub    $0xc,%esp
 563:	68 30 00 00 00       	push   $0x30
 568:	e8 fc ff ff ff       	call   569 <KBDriverEntry+0x69>
 56d:	a1 08 00 00 00       	mov    0x8,%eax
		if(lpDevObject)
		{
			IOManager.DestroyDevice((__COMMON_OBJECT*)&IOManager,
				lpDevObject);
		}
		if(g_hIntHandler)
 572:	83 c4 10             	add    $0x10,%esp
 575:	85 c0                	test   %eax,%eax
 577:	74 da                	je     553 <KBDriverEntry+0x53>
		{
			DisconnectInterrupt(g_hIntHandler);
 579:	83 ec 0c             	sub    $0xc,%esp
 57c:	50                   	push   %eax
 57d:	e8 fc ff ff ff       	call   57e <KBDriverEntry+0x7e>
			g_hIntHandler = NULL;  //Set to initial value.
 582:	83 c4 10             	add    $0x10,%esp
 585:	c7 05 08 00 00 00 00 	movl   $0x0,0x8
 58c:	00 00 00 
		}
	}
	return bResult;
}
 58f:	89 d8                	mov    %ebx,%eax
 591:	83 c4 04             	add    $0x4,%esp
 594:	5b                   	pop    %ebx
 595:	5e                   	pop    %esi
 596:	c3                   	ret    
