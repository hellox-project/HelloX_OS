	.file	"kapi.c"
	.text
.Ltext0:
	.section	.text.unlikely,"ax",@progbits
.LCOLDB0:
	.text
.LHOTB0:
	.p2align 4,,15
	.section	.text.unlikely
.Ltext_cold0:
	.text
	.globl	CreateKernelThread
	.type	CreateKernelThread, @function
CreateKernelThread:
.LFB0:
	.file 1 "kapi.c"
	.loc 1 315 0
	.cfi_startproc
.LVL0:
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	.loc 1 316 0
	movl	$1, %eax
	.loc 1 315 0
	pushl	%edi
	.cfi_def_cfa_offset 12
	.cfi_offset 7, -12
	pushl	%esi
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	pushl	%ebx
	.cfi_def_cfa_offset 20
	.cfi_offset 3, -20
	.loc 1 316 0
	movl	40(%esp), %ebx
	movl	36(%esp), %edx
	movl	20(%esp), %ecx
	movl	24(%esp), %esi
	movl	28(%esp), %edi
	movl	32(%esp), %ebp
#APP
# 316 "kapi.c" 1
	.code32				
	pushl	44(%esp)				
	pushl	%ebx				
	pushl	%edx				
	pushl	%ebp				
	pushl	%edi  			
	pushl	%esi  			
	pushl	%ecx				
	pushl	$0				
	pushl	%eax				
	int	$0x7F			
	popl	%eax			
	popl	%eax			
	popl	%edx				
	popl	%ebx				
	popl	%esi				
	popl	%edi				
	popl	%eax				
	popl	%ecx				
	popl	44(%esp)				
	
# 0 "" 2
.LVL1:
	.loc 1 320 0
#NO_APP
	popl	%ebx
	.cfi_restore 3
	.cfi_def_cfa_offset 16
	popl	%esi
	.cfi_restore 6
	.cfi_def_cfa_offset 12
	popl	%edi
	.cfi_restore 7
	.cfi_def_cfa_offset 8
	popl	%ebp
	.cfi_restore 5
	.cfi_def_cfa_offset 4
	ret
	.cfi_endproc
.LFE0:
	.size	CreateKernelThread, .-CreateKernelThread
	.section	.text.unlikely
.LCOLDE0:
	.text
.LHOTE0:
	.section	.text.unlikely
.LCOLDB1:
	.text
.LHOTB1:
	.p2align 4,,15
	.globl	DestroyKernelThread
	.type	DestroyKernelThread, @function
DestroyKernelThread:
.LFB1:
	.loc 1 323 0
	.cfi_startproc
.LVL2:
	.loc 1 324 0
	movl	$2, %eax
	movl	4(%esp), %edx
#APP
# 324 "kapi.c" 1
	.code32			
	pushl 	%edx			
	pushl	$0			
	pushl	%eax			
	int	$0x7F		
	popl	%eax		
	popl	%eax		
	popl	%eax			
	
# 0 "" 2
.LVL3:
#NO_APP
	ret
	.cfi_endproc
.LFE1:
	.size	DestroyKernelThread, .-DestroyKernelThread
	.section	.text.unlikely
.LCOLDE1:
	.text
.LHOTE1:
	.section	.text.unlikely
.LCOLDB2:
	.text
.LHOTB2:
	.p2align 4,,15
	.globl	SetLastError
	.type	SetLastError, @function
SetLastError:
.LFB2:
	.loc 1 328 0
	.cfi_startproc
.LVL4:
	.loc 1 329 0
	movl	$3, %eax
	movl	4(%esp), %edx
#APP
# 329 "kapi.c" 1
	.code32			
	pushl 	%edx			
	pushl	$0			
	pushl	%eax			
	int	$0x7F		
	popl	%eax		
	popl	%eax		
	popl	%eax			
	
# 0 "" 2
.LVL5:
	.loc 1 330 0
#NO_APP
	ret
	.cfi_endproc
.LFE2:
	.size	SetLastError, .-SetLastError
	.section	.text.unlikely
.LCOLDE2:
	.text
.LHOTE2:
	.section	.text.unlikely
.LCOLDB3:
	.text
.LHOTB3:
	.p2align 4,,15
	.globl	GetLastError
	.type	GetLastError, @function
GetLastError:
.LFB3:
	.loc 1 333 0
	.cfi_startproc
	.loc 1 334 0
	movl	$4, %eax
#APP
# 334 "kapi.c" 1
	.code32 			
	pushl		$0		
	pushl		%eax		
	int		$0x7F	
	popl		%eax	
	popl		%eax	
	
# 0 "" 2
	.loc 1 335 0
#NO_APP
	ret
	.cfi_endproc
.LFE3:
	.size	GetLastError, .-GetLastError
	.section	.text.unlikely
.LCOLDE3:
	.text
.LHOTE3:
	.section	.text.unlikely
.LCOLDB4:
	.text
.LHOTB4:
	.p2align 4,,15
	.globl	GetThreadID
	.type	GetThreadID, @function
GetThreadID:
.LFB4:
	.loc 1 338 0
	.cfi_startproc
.LVL6:
	.loc 1 339 0
	movl	$5, %eax
	movl	4(%esp), %edx
#APP
# 339 "kapi.c" 1
	.code32			
	pushl 	%edx			
	pushl	$0			
	pushl	%eax			
	int	$0x7F		
	popl	%eax		
	popl	%eax		
	popl	%eax			
	
# 0 "" 2
.LVL7:
	.loc 1 340 0
#NO_APP
	ret
	.cfi_endproc
.LFE4:
	.size	GetThreadID, .-GetThreadID
	.section	.text.unlikely
.LCOLDE4:
	.text
.LHOTE4:
	.section	.text.unlikely
.LCOLDB5:
	.text
.LHOTB5:
	.p2align 4,,15
	.globl	SetThreadPriority
	.type	SetThreadPriority, @function
SetThreadPriority:
.LFB5:
	.loc 1 343 0
	.cfi_startproc
.LVL8:
	.loc 1 344 0
	movl	$6, %eax
	movl	8(%esp), %ecx
	movl	4(%esp), %edx
#APP
# 344 "kapi.c" 1
	.code32				
	pushl	%ecx				
	pushl	%edx				
	pushl	$0				
	pushl	%eax				
	int	$0x7F			
	popl	%eax			
	popl	%eax			
	popl	%eax				
	popl	%edx				
	
# 0 "" 2
.LVL9:
	.loc 1 345 0
#NO_APP
	ret
	.cfi_endproc
.LFE5:
	.size	SetThreadPriority, .-SetThreadPriority
	.section	.text.unlikely
.LCOLDE5:
	.text
.LHOTE5:
	.section	.text.unlikely
.LCOLDB6:
	.text
.LHOTB6:
	.p2align 4,,15
	.globl	GetMessage
	.type	GetMessage, @function
GetMessage:
.LFB6:
	.loc 1 348 0
	.cfi_startproc
.LVL10:
	.loc 1 349 0
	movl	$7, %eax
	movl	4(%esp), %edx
#APP
# 349 "kapi.c" 1
	.code32			
	pushl 	%edx			
	pushl	$0			
	pushl	%eax			
	int	$0x7F		
	popl	%eax		
	popl	%eax		
	popl	%eax			
	
# 0 "" 2
.LVL11:
	.loc 1 350 0
#NO_APP
	ret
	.cfi_endproc
.LFE6:
	.size	GetMessage, .-GetMessage
	.section	.text.unlikely
.LCOLDE6:
	.text
.LHOTE6:
	.section	.text.unlikely
.LCOLDB7:
	.text
.LHOTB7:
	.p2align 4,,15
	.globl	SendMessage
	.type	SendMessage, @function
SendMessage:
.LFB7:
	.loc 1 353 0
	.cfi_startproc
.LVL12:
	.loc 1 354 0
	movl	$8, %eax
	movl	8(%esp), %ecx
	movl	4(%esp), %edx
#APP
# 354 "kapi.c" 1
	.code32				
	pushl	%ecx				
	pushl	%edx				
	pushl	$0				
	pushl	%eax				
	int	$0x7F			
	popl	%eax			
	popl	%eax			
	popl	%eax				
	popl	%edx				
	
# 0 "" 2
.LVL13:
	.loc 1 355 0
#NO_APP
	ret
	.cfi_endproc
.LFE7:
	.size	SendMessage, .-SendMessage
	.section	.text.unlikely
.LCOLDE7:
	.text
.LHOTE7:
	.section	.text.unlikely
.LCOLDB8:
	.text
.LHOTB8:
	.p2align 4,,15
	.globl	Sleep
	.type	Sleep, @function
Sleep:
.LFB8:
	.loc 1 358 0
	.cfi_startproc
.LVL14:
	.loc 1 359 0
	movl	$9, %eax
	movl	4(%esp), %edx
#APP
# 359 "kapi.c" 1
	.code32			
	pushl 	%edx			
	pushl	$0			
	pushl	%eax			
	int	$0x7F		
	popl	%eax		
	popl	%eax		
	popl	%eax			
	
# 0 "" 2
.LVL15:
	.loc 1 360 0
#NO_APP
	ret
	.cfi_endproc
.LFE8:
	.size	Sleep, .-Sleep
	.section	.text.unlikely
.LCOLDE8:
	.text
.LHOTE8:
	.section	.text.unlikely
.LCOLDB9:
	.text
.LHOTB9:
	.p2align 4,,15
	.globl	SetTimer
	.type	SetTimer, @function
SetTimer:
.LFB9:
	.loc 1 367 0
	.cfi_startproc
.LVL16:
	pushl	%edi
	.cfi_def_cfa_offset 8
	.cfi_offset 7, -8
	.loc 1 368 0
	movl	$10, %eax
	.loc 1 367 0
	pushl	%esi
	.cfi_def_cfa_offset 12
	.cfi_offset 6, -12
	pushl	%ebx
	.cfi_def_cfa_offset 16
	.cfi_offset 3, -16
	.loc 1 368 0
	movl	32(%esp), %ebx
	movl	28(%esp), %edx
	movl	16(%esp), %ecx
	movl	20(%esp), %esi
	movl	24(%esp), %edi
#APP
# 368 "kapi.c" 1
	.code32				
	pushl	%ebx				
	pushl	%edx  			
	pushl	%edi				
	pushl	%esi				
	pushl	%ecx				
	pushl	$0				
	pushl	%eax				
	int	$0x7F			
	popl	%eax			
	popl	%eax			
	popl	%edx				
	popl	%ebx				
	popl	%esi				
	popl	%eax				
	popl	%ecx				
	
# 0 "" 2
.LVL17:
	.loc 1 374 0
#NO_APP
	popl	%ebx
	.cfi_restore 3
	.cfi_def_cfa_offset 12
	popl	%esi
	.cfi_restore 6
	.cfi_def_cfa_offset 8
	popl	%edi
	.cfi_restore 7
	.cfi_def_cfa_offset 4
	ret
	.cfi_endproc
.LFE9:
	.size	SetTimer, .-SetTimer
	.section	.text.unlikely
.LCOLDE9:
	.text
.LHOTE9:
	.section	.text.unlikely
.LCOLDB10:
	.text
.LHOTB10:
	.p2align 4,,15
	.globl	CancelTimer
	.type	CancelTimer, @function
CancelTimer:
.LFB10:
	.loc 1 377 0
	.cfi_startproc
.LVL18:
	.loc 1 378 0
	movl	$11, %eax
	movl	4(%esp), %edx
#APP
# 378 "kapi.c" 1
	.code32			
	pushl 	%edx			
	pushl	$0			
	pushl	%eax			
	int	$0x7F		
	popl	%eax		
	popl	%eax		
	popl	%eax			
	
# 0 "" 2
.LVL19:
#NO_APP
	ret
	.cfi_endproc
.LFE10:
	.size	CancelTimer, .-CancelTimer
	.section	.text.unlikely
.LCOLDE10:
	.text
.LHOTE10:
	.section	.text.unlikely
.LCOLDB11:
	.text
.LHOTB11:
	.p2align 4,,15
	.globl	CreateEvent
	.type	CreateEvent, @function
CreateEvent:
.LFB11:
	.loc 1 382 0
	.cfi_startproc
.LVL20:
	.loc 1 383 0
	movl	$12, %eax
	movl	4(%esp), %edx
#APP
# 383 "kapi.c" 1
	.code32			
	pushl 	%edx			
	pushl	$0			
	pushl	%eax			
	int	$0x7F		
	popl	%eax		
	popl	%eax		
	popl	%eax			
	
# 0 "" 2
.LVL21:
	.loc 1 384 0
#NO_APP
	ret
	.cfi_endproc
.LFE11:
	.size	CreateEvent, .-CreateEvent
	.section	.text.unlikely
.LCOLDE11:
	.text
.LHOTE11:
	.section	.text.unlikely
.LCOLDB12:
	.text
.LHOTB12:
	.p2align 4,,15
	.globl	DestroyEvent
	.type	DestroyEvent, @function
DestroyEvent:
.LFB12:
	.loc 1 387 0
	.cfi_startproc
.LVL22:
	.loc 1 388 0
	movl	$13, %eax
	movl	4(%esp), %edx
#APP
# 388 "kapi.c" 1
	.code32			
	pushl 	%edx			
	pushl	$0			
	pushl	%eax			
	int	$0x7F		
	popl	%eax		
	popl	%eax		
	popl	%eax			
	
# 0 "" 2
.LVL23:
#NO_APP
	ret
	.cfi_endproc
.LFE12:
	.size	DestroyEvent, .-DestroyEvent
	.section	.text.unlikely
.LCOLDE12:
	.text
.LHOTE12:
	.section	.text.unlikely
.LCOLDB13:
	.text
.LHOTB13:
	.p2align 4,,15
	.globl	SetEvent
	.type	SetEvent, @function
SetEvent:
.LFB13:
	.loc 1 392 0
	.cfi_startproc
.LVL24:
	.loc 1 393 0
	movl	$14, %eax
	movl	4(%esp), %edx
#APP
# 393 "kapi.c" 1
	.code32			
	pushl 	%edx			
	pushl	$0			
	pushl	%eax			
	int	$0x7F		
	popl	%eax		
	popl	%eax		
	popl	%eax			
	
# 0 "" 2
.LVL25:
	.loc 1 394 0
#NO_APP
	ret
	.cfi_endproc
.LFE13:
	.size	SetEvent, .-SetEvent
	.section	.text.unlikely
.LCOLDE13:
	.text
.LHOTE13:
	.section	.text.unlikely
.LCOLDB14:
	.text
.LHOTB14:
	.p2align 4,,15
	.globl	ResetEvent
	.type	ResetEvent, @function
ResetEvent:
.LFB14:
	.loc 1 397 0
	.cfi_startproc
.LVL26:
	.loc 1 398 0
	movl	$15, %eax
	movl	4(%esp), %edx
#APP
# 398 "kapi.c" 1
	.code32			
	pushl 	%edx			
	pushl	$0			
	pushl	%eax			
	int	$0x7F		
	popl	%eax		
	popl	%eax		
	popl	%eax			
	
# 0 "" 2
.LVL27:
	.loc 1 399 0
#NO_APP
	ret
	.cfi_endproc
.LFE14:
	.size	ResetEvent, .-ResetEvent
	.section	.text.unlikely
.LCOLDE14:
	.text
.LHOTE14:
	.section	.text.unlikely
.LCOLDB15:
	.text
.LHOTB15:
	.p2align 4,,15
	.globl	CreateMutex
	.type	CreateMutex, @function
CreateMutex:
.LFB15:
	.loc 1 402 0
	.cfi_startproc
	.loc 1 403 0
	movl	$16, %eax
#APP
# 403 "kapi.c" 1
	.code32 			
	pushl		$0		
	pushl		%eax		
	int		$0x7F	
	popl		%eax	
	popl		%eax	
	
# 0 "" 2
	.loc 1 404 0
#NO_APP
	ret
	.cfi_endproc
.LFE15:
	.size	CreateMutex, .-CreateMutex
	.section	.text.unlikely
.LCOLDE15:
	.text
.LHOTE15:
	.section	.text.unlikely
.LCOLDB16:
	.text
.LHOTB16:
	.p2align 4,,15
	.globl	DestroyMutex
	.type	DestroyMutex, @function
DestroyMutex:
.LFB16:
	.loc 1 407 0
	.cfi_startproc
.LVL28:
	.loc 1 408 0
	movl	$17, %eax
	movl	4(%esp), %edx
#APP
# 408 "kapi.c" 1
	.code32			
	pushl 	%edx			
	pushl	$0			
	pushl	%eax			
	int	$0x7F		
	popl	%eax		
	popl	%eax		
	popl	%eax			
	
# 0 "" 2
.LVL29:
#NO_APP
	ret
	.cfi_endproc
.LFE16:
	.size	DestroyMutex, .-DestroyMutex
	.section	.text.unlikely
.LCOLDE16:
	.text
.LHOTE16:
	.section	.text.unlikely
.LCOLDB17:
	.text
.LHOTB17:
	.p2align 4,,15
	.globl	ReleaseMutex
	.type	ReleaseMutex, @function
ReleaseMutex:
.LFB17:
	.loc 1 412 0
	.cfi_startproc
.LVL30:
	.loc 1 413 0
	movl	$18, %eax
	movl	4(%esp), %edx
#APP
# 413 "kapi.c" 1
	.code32			
	pushl 	%edx			
	pushl	$0			
	pushl	%eax			
	int	$0x7F		
	popl	%eax		
	popl	%eax		
	popl	%eax			
	
# 0 "" 2
.LVL31:
	.loc 1 414 0
#NO_APP
	ret
	.cfi_endproc
.LFE17:
	.size	ReleaseMutex, .-ReleaseMutex
	.section	.text.unlikely
.LCOLDE17:
	.text
.LHOTE17:
	.section	.text.unlikely
.LCOLDB18:
	.text
.LHOTB18:
	.p2align 4,,15
	.globl	WaitForThisObject
	.type	WaitForThisObject, @function
WaitForThisObject:
.LFB18:
	.loc 1 417 0
	.cfi_startproc
.LVL32:
	.loc 1 418 0
	movl	$19, %eax
	movl	4(%esp), %edx
#APP
# 418 "kapi.c" 1
	.code32			
	pushl 	%edx			
	pushl	$0			
	pushl	%eax			
	int	$0x7F		
	popl	%eax		
	popl	%eax		
	popl	%eax			
	
# 0 "" 2
.LVL33:
	.loc 1 419 0
#NO_APP
	ret
	.cfi_endproc
.LFE18:
	.size	WaitForThisObject, .-WaitForThisObject
	.section	.text.unlikely
.LCOLDE18:
	.text
.LHOTE18:
	.section	.text.unlikely
.LCOLDB19:
	.text
.LHOTB19:
	.p2align 4,,15
	.globl	WaitForThisObjectEx
	.type	WaitForThisObjectEx, @function
WaitForThisObjectEx:
.LFB19:
	.loc 1 422 0
	.cfi_startproc
.LVL34:
	.loc 1 423 0
	movl	$20, %eax
	movl	8(%esp), %ecx
	movl	4(%esp), %edx
#APP
# 423 "kapi.c" 1
	.code32				
	pushl	%ecx				
	pushl	%edx				
	pushl	$0				
	pushl	%eax				
	int	$0x7F			
	popl	%eax			
	popl	%eax			
	popl	%eax				
	popl	%edx				
	
# 0 "" 2
.LVL35:
	.loc 1 425 0
#NO_APP
	ret
	.cfi_endproc
.LFE19:
	.size	WaitForThisObjectEx, .-WaitForThisObjectEx
	.section	.text.unlikely
.LCOLDE19:
	.text
.LHOTE19:
	.section	.text.unlikely
.LCOLDB20:
	.text
.LHOTB20:
	.p2align 4,,15
	.globl	ConnectInterrupt
	.type	ConnectInterrupt, @function
ConnectInterrupt:
.LFB20:
	.loc 1 430 0
	.cfi_startproc
.LVL36:
	.loc 1 432 0
	movl	12(%esp), %eax
	movl	8(%esp), %edx
	movl	4(%esp), %ecx
#APP
# 432 "kapi.c" 1
	.code32			;pushl 	%ebx		;movb	%al,	%bl	;pushl	%ebx				;pushl	%edx					;pushl	%ecx					;pushl	$0					;pushl	SYSCALL_CONNECTINTERRUPT;int	$0x7F					;popl	%eax					;popl	%eax					;popl	%edx						;popl	%eax						;popl	%ebx					;movb	%bl, %cl				;popl	%ebx					;
# 0 "" 2
.LVL37:
	.loc 1 472 0
#NO_APP
	ret
	.cfi_endproc
.LFE20:
	.size	ConnectInterrupt, .-ConnectInterrupt
	.section	.text.unlikely
.LCOLDE20:
	.text
.LHOTE20:
	.section	.text.unlikely
.LCOLDB21:
	.text
.LHOTB21:
	.p2align 4,,15
	.globl	DisconnectInterrupt
	.type	DisconnectInterrupt, @function
DisconnectInterrupt:
.LFB21:
	.loc 1 475 0
	.cfi_startproc
.LVL38:
	.loc 1 476 0
	movl	$22, %eax
	movl	4(%esp), %edx
#APP
# 476 "kapi.c" 1
	.code32			
	pushl 	%edx			
	pushl	$0			
	pushl	%eax			
	int	$0x7F		
	popl	%eax		
	popl	%eax		
	popl	%eax			
	
# 0 "" 2
.LVL39:
#NO_APP
	ret
	.cfi_endproc
.LFE21:
	.size	DisconnectInterrupt, .-DisconnectInterrupt
	.section	.text.unlikely
.LCOLDE21:
	.text
.LHOTE21:
	.section	.text.unlikely
.LCOLDB22:
	.text
.LHOTB22:
	.p2align 4,,15
	.globl	VirtualAlloc
	.type	VirtualAlloc, @function
VirtualAlloc:
.LFB22:
	.loc 1 484 0
	.cfi_startproc
.LVL40:
	pushl	%edi
	.cfi_def_cfa_offset 8
	.cfi_offset 7, -8
	.loc 1 485 0
	movl	$23, %eax
	.loc 1 484 0
	pushl	%esi
	.cfi_def_cfa_offset 12
	.cfi_offset 6, -12
	pushl	%ebx
	.cfi_def_cfa_offset 16
	.cfi_offset 3, -16
	.loc 1 485 0
	movl	32(%esp), %ebx
	movl	28(%esp), %edx
	movl	16(%esp), %ecx
	movl	20(%esp), %esi
	movl	24(%esp), %edi
#APP
# 485 "kapi.c" 1
	.code32				
	pushl	%ebx				
	pushl	%edx  			
	pushl	%edi				
	pushl	%esi				
	pushl	%ecx				
	pushl	$0				
	pushl	%eax				
	int	$0x7F			
	popl	%eax			
	popl	%eax			
	popl	%edx				
	popl	%ebx				
	popl	%esi				
	popl	%eax				
	popl	%ecx				
	
# 0 "" 2
.LVL41:
	.loc 1 491 0
#NO_APP
	popl	%ebx
	.cfi_restore 3
	.cfi_def_cfa_offset 12
	popl	%esi
	.cfi_restore 6
	.cfi_def_cfa_offset 8
	popl	%edi
	.cfi_restore 7
	.cfi_def_cfa_offset 4
	ret
	.cfi_endproc
.LFE22:
	.size	VirtualAlloc, .-VirtualAlloc
	.section	.text.unlikely
.LCOLDE22:
	.text
.LHOTE22:
	.section	.text.unlikely
.LCOLDB23:
	.text
.LHOTB23:
	.p2align 4,,15
	.globl	VirtualFree
	.type	VirtualFree, @function
VirtualFree:
.LFB23:
	.loc 1 494 0
	.cfi_startproc
.LVL42:
	.loc 1 495 0
	movl	$24, %eax
	movl	4(%esp), %edx
#APP
# 495 "kapi.c" 1
	.code32			
	pushl 	%edx			
	pushl	$0			
	pushl	%eax			
	int	$0x7F		
	popl	%eax		
	popl	%eax		
	popl	%eax			
	
# 0 "" 2
.LVL43:
#NO_APP
	ret
	.cfi_endproc
.LFE23:
	.size	VirtualFree, .-VirtualFree
	.section	.text.unlikely
.LCOLDE23:
	.text
.LHOTE23:
	.section	.text.unlikely
.LCOLDB24:
	.text
.LHOTB24:
	.p2align 4,,15
	.globl	CreateFile
	.type	CreateFile, @function
CreateFile:
.LFB24:
	.loc 1 502 0
	.cfi_startproc
.LVL44:
	pushl	%esi
	.cfi_def_cfa_offset 8
	.cfi_offset 6, -8
	.loc 1 503 0
	movl	$25, %eax
	.loc 1 502 0
	pushl	%ebx
	.cfi_def_cfa_offset 12
	.cfi_offset 3, -12
	.loc 1 503 0
	movl	24(%esp), %ebx
	movl	20(%esp), %edx
	movl	12(%esp), %ecx
	movl	16(%esp), %esi
#APP
# 503 "kapi.c" 1
	.code32				
	pushl	%ebx				
	pushl	%edx				
	pushl	%esi				
	pushl	%ecx				
	pushl	$0				
	pushl	%eax				
	int	$0x7F			
	popl	%eax			
	popl	%eax			
	popl	%edx				
	popl	%ebx				
	popl	%eax				
	popl	%ecx				
	
# 0 "" 2
.LVL45:
	.loc 1 508 0
#NO_APP
	popl	%ebx
	.cfi_restore 3
	.cfi_def_cfa_offset 8
	popl	%esi
	.cfi_restore 6
	.cfi_def_cfa_offset 4
	ret
	.cfi_endproc
.LFE24:
	.size	CreateFile, .-CreateFile
	.section	.text.unlikely
.LCOLDE24:
	.text
.LHOTE24:
	.section	.text.unlikely
.LCOLDB25:
	.text
.LHOTB25:
	.p2align 4,,15
	.globl	ReadFile
	.type	ReadFile, @function
ReadFile:
.LFB25:
	.loc 1 514 0
	.cfi_startproc
.LVL46:
	pushl	%esi
	.cfi_def_cfa_offset 8
	.cfi_offset 6, -8
	.loc 1 515 0
	movl	$26, %eax
	.loc 1 514 0
	pushl	%ebx
	.cfi_def_cfa_offset 12
	.cfi_offset 3, -12
	.loc 1 515 0
	movl	24(%esp), %ebx
	movl	20(%esp), %edx
	movl	12(%esp), %ecx
	movl	16(%esp), %esi
#APP
# 515 "kapi.c" 1
	.code32				
	pushl	%ebx				
	pushl	%edx				
	pushl	%esi				
	pushl	%ecx				
	pushl	$0				
	pushl	%eax				
	int	$0x7F			
	popl	%eax			
	popl	%eax			
	popl	%edx				
	popl	%ebx				
	popl	%eax				
	popl	%ecx				
	
# 0 "" 2
.LVL47:
	.loc 1 520 0
#NO_APP
	popl	%ebx
	.cfi_restore 3
	.cfi_def_cfa_offset 8
	popl	%esi
	.cfi_restore 6
	.cfi_def_cfa_offset 4
	ret
	.cfi_endproc
.LFE25:
	.size	ReadFile, .-ReadFile
	.section	.text.unlikely
.LCOLDE25:
	.text
.LHOTE25:
	.section	.text.unlikely
.LCOLDB26:
	.text
.LHOTB26:
	.p2align 4,,15
	.globl	WriteFile
	.type	WriteFile, @function
WriteFile:
.LFB26:
	.loc 1 526 0
	.cfi_startproc
.LVL48:
	pushl	%esi
	.cfi_def_cfa_offset 8
	.cfi_offset 6, -8
	.loc 1 527 0
	movl	$27, %eax
	.loc 1 526 0
	pushl	%ebx
	.cfi_def_cfa_offset 12
	.cfi_offset 3, -12
	.loc 1 527 0
	movl	24(%esp), %ebx
	movl	20(%esp), %edx
	movl	12(%esp), %ecx
	movl	16(%esp), %esi
#APP
# 527 "kapi.c" 1
	.code32				
	pushl	%ebx				
	pushl	%edx				
	pushl	%esi				
	pushl	%ecx				
	pushl	$0				
	pushl	%eax				
	int	$0x7F			
	popl	%eax			
	popl	%eax			
	popl	%edx				
	popl	%ebx				
	popl	%eax				
	popl	%ecx				
	
# 0 "" 2
.LVL49:
	.loc 1 532 0
#NO_APP
	popl	%ebx
	.cfi_restore 3
	.cfi_def_cfa_offset 8
	popl	%esi
	.cfi_restore 6
	.cfi_def_cfa_offset 4
	ret
	.cfi_endproc
.LFE26:
	.size	WriteFile, .-WriteFile
	.section	.text.unlikely
.LCOLDE26:
	.text
.LHOTE26:
	.section	.text.unlikely
.LCOLDB27:
	.text
.LHOTB27:
	.p2align 4,,15
	.globl	CloseFile
	.type	CloseFile, @function
CloseFile:
.LFB27:
	.loc 1 535 0
	.cfi_startproc
.LVL50:
	.loc 1 536 0
	movl	$28, %eax
	movl	4(%esp), %edx
#APP
# 536 "kapi.c" 1
	.code32			
	pushl 	%edx			
	pushl	$0			
	pushl	%eax			
	int	$0x7F		
	popl	%eax		
	popl	%eax		
	popl	%eax			
	
# 0 "" 2
.LVL51:
#NO_APP
	ret
	.cfi_endproc
.LFE27:
	.size	CloseFile, .-CloseFile
	.section	.text.unlikely
.LCOLDE27:
	.text
.LHOTE27:
	.section	.text.unlikely
.LCOLDB28:
	.text
.LHOTB28:
	.p2align 4,,15
	.globl	CreateDirectory
	.type	CreateDirectory, @function
CreateDirectory:
.LFB28:
	.loc 1 541 0
	.cfi_startproc
.LVL52:
	.loc 1 542 0
	movl	$29, %eax
	movl	4(%esp), %edx
#APP
# 542 "kapi.c" 1
	.code32			
	pushl 	%edx			
	pushl	$0			
	pushl	%eax			
	int	$0x7F		
	popl	%eax		
	popl	%eax		
	popl	%eax			
	
# 0 "" 2
.LVL53:
	.loc 1 543 0
#NO_APP
	ret
	.cfi_endproc
.LFE28:
	.size	CreateDirectory, .-CreateDirectory
	.section	.text.unlikely
.LCOLDE28:
	.text
.LHOTE28:
	.section	.text.unlikely
.LCOLDB29:
	.text
.LHOTB29:
	.p2align 4,,15
	.globl	DeleteFile
	.type	DeleteFile, @function
DeleteFile:
.LFB29:
	.loc 1 546 0
	.cfi_startproc
.LVL54:
	.loc 1 547 0
	movl	$30, %eax
	movl	4(%esp), %edx
#APP
# 547 "kapi.c" 1
	.code32			
	pushl 	%edx			
	pushl	$0			
	pushl	%eax			
	int	$0x7F		
	popl	%eax		
	popl	%eax		
	popl	%eax			
	
# 0 "" 2
.LVL55:
	.loc 1 548 0
#NO_APP
	ret
	.cfi_endproc
.LFE29:
	.size	DeleteFile, .-DeleteFile
	.section	.text.unlikely
.LCOLDE29:
	.text
.LHOTE29:
	.section	.text.unlikely
.LCOLDB30:
	.text
.LHOTB30:
	.p2align 4,,15
	.globl	FindFirstFile
	.type	FindFirstFile, @function
FindFirstFile:
.LFB30:
	.loc 1 552 0
	.cfi_startproc
.LVL56:
	.loc 1 553 0
	movl	$31, %eax
	movl	8(%esp), %ecx
	movl	4(%esp), %edx
#APP
# 553 "kapi.c" 1
	.code32				
	pushl	%ecx				
	pushl	%edx				
	pushl	$0				
	pushl	%eax				
	int	$0x7F			
	popl	%eax			
	popl	%eax			
	popl	%eax				
	popl	%edx				
	
# 0 "" 2
.LVL57:
	.loc 1 556 0
#NO_APP
	ret
	.cfi_endproc
.LFE30:
	.size	FindFirstFile, .-FindFirstFile
	.section	.text.unlikely
.LCOLDE30:
	.text
.LHOTE30:
	.section	.text.unlikely
.LCOLDB31:
	.text
.LHOTB31:
	.p2align 4,,15
	.globl	FindNextFile
	.type	FindNextFile, @function
FindNextFile:
.LFB31:
	.loc 1 561 0
	.cfi_startproc
.LVL58:
	pushl	%ebx
	.cfi_def_cfa_offset 8
	.cfi_offset 3, -8
	.loc 1 562 0
	movl	$32, %eax
	movl	12(%esp), %edx
	movl	8(%esp), %ecx
	movl	16(%esp), %ebx
#APP
# 562 "kapi.c" 1
	.code32				
	pushl	%ebx				
	pushl	%edx				
	pushl	%ecx				
	pushl	$0				
	pushl	%eax				
	int	$0x7F			
	popl	%eax			
	popl	%eax			
	popl	%edx				
	popl	%eax				
	popl	%ecx				
	
# 0 "" 2
.LVL59:
	.loc 1 565 0
#NO_APP
	popl	%ebx
	.cfi_restore 3
	.cfi_def_cfa_offset 4
	ret
	.cfi_endproc
.LFE31:
	.size	FindNextFile, .-FindNextFile
	.section	.text.unlikely
.LCOLDE31:
	.text
.LHOTE31:
	.section	.text.unlikely
.LCOLDB32:
	.text
.LHOTB32:
	.p2align 4,,15
	.globl	FindClose
	.type	FindClose, @function
FindClose:
.LFB32:
	.loc 1 569 0
	.cfi_startproc
.LVL60:
	.loc 1 570 0
	movl	$33, %eax
	movl	8(%esp), %ecx
	movl	4(%esp), %edx
#APP
# 570 "kapi.c" 1
	.code32				
	pushl	%ecx				
	pushl	%edx				
	pushl	$0				
	pushl	%eax				
	int	$0x7F			
	popl	%eax			
	popl	%eax			
	popl	%eax				
	popl	%edx				
	
# 0 "" 2
.LVL61:
#NO_APP
	ret
	.cfi_endproc
.LFE32:
	.size	FindClose, .-FindClose
	.section	.text.unlikely
.LCOLDE32:
	.text
.LHOTE32:
	.section	.text.unlikely
.LCOLDB33:
	.text
.LHOTB33:
	.p2align 4,,15
	.globl	GetFileAttributes
	.type	GetFileAttributes, @function
GetFileAttributes:
.LFB33:
	.loc 1 576 0
	.cfi_startproc
.LVL62:
	.loc 1 577 0
	movl	$34, %eax
	movl	4(%esp), %edx
#APP
# 577 "kapi.c" 1
	.code32			
	pushl 	%edx			
	pushl	$0			
	pushl	%eax			
	int	$0x7F		
	popl	%eax		
	popl	%eax		
	popl	%eax			
	
# 0 "" 2
.LVL63:
	.loc 1 578 0
#NO_APP
	ret
	.cfi_endproc
.LFE33:
	.size	GetFileAttributes, .-GetFileAttributes
	.section	.text.unlikely
.LCOLDE33:
	.text
.LHOTE33:
	.section	.text.unlikely
.LCOLDB34:
	.text
.LHOTB34:
	.p2align 4,,15
	.globl	GetFileSize
	.type	GetFileSize, @function
GetFileSize:
.LFB34:
	.loc 1 581 0
	.cfi_startproc
.LVL64:
	.loc 1 582 0
	movl	$35, %eax
	movl	8(%esp), %ecx
	movl	4(%esp), %edx
#APP
# 582 "kapi.c" 1
	.code32				
	pushl	%ecx				
	pushl	%edx				
	pushl	$0				
	pushl	%eax				
	int	$0x7F			
	popl	%eax			
	popl	%eax			
	popl	%eax				
	popl	%edx				
	
# 0 "" 2
.LVL65:
	.loc 1 584 0
#NO_APP
	ret
	.cfi_endproc
.LFE34:
	.size	GetFileSize, .-GetFileSize
	.section	.text.unlikely
.LCOLDE34:
	.text
.LHOTE34:
	.section	.text.unlikely
.LCOLDB35:
	.text
.LHOTB35:
	.p2align 4,,15
	.globl	RemoveDirectory
	.type	RemoveDirectory, @function
RemoveDirectory:
.LFB35:
	.loc 1 587 0
	.cfi_startproc
.LVL66:
	.loc 1 588 0
	movl	$36, %eax
	movl	4(%esp), %edx
#APP
# 588 "kapi.c" 1
	.code32			
	pushl 	%edx			
	pushl	$0			
	pushl	%eax			
	int	$0x7F		
	popl	%eax		
	popl	%eax		
	popl	%eax			
	
# 0 "" 2
.LVL67:
	.loc 1 590 0
#NO_APP
	ret
	.cfi_endproc
.LFE35:
	.size	RemoveDirectory, .-RemoveDirectory
	.section	.text.unlikely
.LCOLDE35:
	.text
.LHOTE35:
	.section	.text.unlikely
.LCOLDB36:
	.text
.LHOTB36:
	.p2align 4,,15
	.globl	SetEndOfFile
	.type	SetEndOfFile, @function
SetEndOfFile:
.LFB36:
	.loc 1 593 0
	.cfi_startproc
.LVL68:
	.loc 1 594 0
	movl	$37, %eax
	movl	4(%esp), %edx
#APP
# 594 "kapi.c" 1
	.code32			
	pushl 	%edx			
	pushl	$0			
	pushl	%eax			
	int	$0x7F		
	popl	%eax		
	popl	%eax		
	popl	%eax			
	
# 0 "" 2
.LVL69:
	.loc 1 595 0
#NO_APP
	ret
	.cfi_endproc
.LFE36:
	.size	SetEndOfFile, .-SetEndOfFile
	.section	.text.unlikely
.LCOLDE36:
	.text
.LHOTE36:
	.section	.text.unlikely
.LCOLDB37:
	.text
.LHOTB37:
	.p2align 4,,15
	.globl	IOControl
	.type	IOControl, @function
IOControl:
.LFB37:
	.loc 1 604 0
	.cfi_startproc
.LVL70:
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	.loc 1 605 0
	movl	$38, %eax
	.loc 1 604 0
	pushl	%edi
	.cfi_def_cfa_offset 12
	.cfi_offset 7, -12
	pushl	%esi
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	pushl	%ebx
	.cfi_def_cfa_offset 20
	.cfi_offset 3, -20
	.loc 1 605 0
	movl	40(%esp), %ebx
	movl	36(%esp), %edx
	movl	20(%esp), %ecx
	movl	24(%esp), %esi
	movl	28(%esp), %edi
	movl	32(%esp), %ebp
#APP
# 605 "kapi.c" 1
	.code32				
	pushl	44(%esp)				
	pushl	%ebx				
	pushl	%edx				
	pushl	%ebp				
	pushl	%edi  			
	pushl	%esi  			
	pushl	%ecx				
	pushl	$0				
	pushl	%eax				
	int	$0x7F			
	popl	%eax			
	popl	%eax			
	popl	%edx				
	popl	%ebx				
	popl	%esi				
	popl	%edi				
	popl	%eax				
	popl	%ecx				
	popl	44(%esp)				
	
# 0 "" 2
.LVL71:
	.loc 1 610 0
#NO_APP
	popl	%ebx
	.cfi_restore 3
	.cfi_def_cfa_offset 16
	popl	%esi
	.cfi_restore 6
	.cfi_def_cfa_offset 12
	popl	%edi
	.cfi_restore 7
	.cfi_def_cfa_offset 8
	popl	%ebp
	.cfi_restore 5
	.cfi_def_cfa_offset 4
	ret
	.cfi_endproc
.LFE37:
	.size	IOControl, .-IOControl
	.section	.text.unlikely
.LCOLDE37:
	.text
.LHOTE37:
	.section	.text.unlikely
.LCOLDB38:
	.text
.LHOTB38:
	.p2align 4,,15
	.globl	SetFilePointer
	.type	SetFilePointer, @function
SetFilePointer:
.LFB38:
	.loc 1 616 0
	.cfi_startproc
.LVL72:
	pushl	%esi
	.cfi_def_cfa_offset 8
	.cfi_offset 6, -8
	.loc 1 617 0
	movl	$39, %eax
	.loc 1 616 0
	pushl	%ebx
	.cfi_def_cfa_offset 12
	.cfi_offset 3, -12
	.loc 1 617 0
	movl	24(%esp), %ebx
	movl	20(%esp), %edx
	movl	12(%esp), %ecx
	movl	16(%esp), %esi
#APP
# 617 "kapi.c" 1
	.code32				
	pushl	%ebx				
	pushl	%edx				
	pushl	%esi				
	pushl	%ecx				
	pushl	$0				
	pushl	%eax				
	int	$0x7F			
	popl	%eax			
	popl	%eax			
	popl	%edx				
	popl	%ebx				
	popl	%eax				
	popl	%ecx				
	
# 0 "" 2
.LVL73:
	.loc 1 621 0
#NO_APP
	popl	%ebx
	.cfi_restore 3
	.cfi_def_cfa_offset 8
	popl	%esi
	.cfi_restore 6
	.cfi_def_cfa_offset 4
	ret
	.cfi_endproc
.LFE38:
	.size	SetFilePointer, .-SetFilePointer
	.section	.text.unlikely
.LCOLDE38:
	.text
.LHOTE38:
	.section	.text.unlikely
.LCOLDB39:
	.text
.LHOTB39:
	.p2align 4,,15
	.globl	FlushFileBuffers
	.type	FlushFileBuffers, @function
FlushFileBuffers:
.LFB39:
	.loc 1 624 0
	.cfi_startproc
.LVL74:
	.loc 1 625 0
	movl	$40, %eax
	movl	4(%esp), %edx
#APP
# 625 "kapi.c" 1
	.code32			
	pushl 	%edx			
	pushl	$0			
	pushl	%eax			
	int	$0x7F		
	popl	%eax		
	popl	%eax		
	popl	%eax			
	
# 0 "" 2
.LVL75:
	.loc 1 627 0
#NO_APP
	ret
	.cfi_endproc
.LFE39:
	.size	FlushFileBuffers, .-FlushFileBuffers
	.section	.text.unlikely
.LCOLDE39:
	.text
.LHOTE39:
	.section	.text.unlikely
.LCOLDB40:
	.text
.LHOTB40:
	.p2align 4,,15
	.globl	CreateDevice
	.type	CreateDevice, @function
CreateDevice:
.LFB40:
	.loc 1 636 0
	.cfi_startproc
.LVL76:
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	.loc 1 637 0
	movl	$41, %eax
	.loc 1 636 0
	pushl	%edi
	.cfi_def_cfa_offset 12
	.cfi_offset 7, -12
	pushl	%esi
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	pushl	%ebx
	.cfi_def_cfa_offset 20
	.cfi_offset 3, -20
	.loc 1 637 0
	movl	40(%esp), %ebx
	movl	36(%esp), %edx
	movl	20(%esp), %ecx
	movl	24(%esp), %esi
	movl	28(%esp), %edi
	movl	32(%esp), %ebp
#APP
# 637 "kapi.c" 1
	.code32				
	pushl	44(%esp)				
	pushl	%ebx				
	pushl	%edx				
	pushl	%ebp				
	pushl	%edi  			
	pushl	%esi  			
	pushl	%ecx				
	pushl	$0				
	pushl	%eax				
	int	$0x7F			
	popl	%eax			
	popl	%eax			
	popl	%edx				
	popl	%ebx				
	popl	%esi				
	popl	%edi				
	popl	%eax				
	popl	%ecx				
	popl	44(%esp)				
	
# 0 "" 2
.LVL77:
	.loc 1 645 0
#NO_APP
	popl	%ebx
	.cfi_restore 3
	.cfi_def_cfa_offset 16
	popl	%esi
	.cfi_restore 6
	.cfi_def_cfa_offset 12
	popl	%edi
	.cfi_restore 7
	.cfi_def_cfa_offset 8
	popl	%ebp
	.cfi_restore 5
	.cfi_def_cfa_offset 4
	ret
	.cfi_endproc
.LFE40:
	.size	CreateDevice, .-CreateDevice
	.section	.text.unlikely
.LCOLDE40:
	.text
.LHOTE40:
	.section	.text.unlikely
.LCOLDB41:
	.text
.LHOTB41:
	.p2align 4,,15
	.globl	DestroyDevice
	.type	DestroyDevice, @function
DestroyDevice:
.LFB41:
	.loc 1 648 0
	.cfi_startproc
.LVL78:
	.loc 1 649 0
	movl	$42, %eax
	movl	4(%esp), %edx
#APP
# 649 "kapi.c" 1
	.code32			
	pushl 	%edx			
	pushl	$0			
	pushl	%eax			
	int	$0x7F		
	popl	%eax		
	popl	%eax		
	popl	%eax			
	
# 0 "" 2
.LVL79:
#NO_APP
	ret
	.cfi_endproc
.LFE41:
	.size	DestroyDevice, .-DestroyDevice
	.section	.text.unlikely
.LCOLDE41:
	.text
.LHOTE41:
	.section	.text.unlikely
.LCOLDB42:
	.text
.LHOTB42:
	.p2align 4,,15
	.globl	KMemAlloc
	.type	KMemAlloc, @function
KMemAlloc:
.LFB42:
	.loc 1 653 0
	.cfi_startproc
.LVL80:
	.loc 1 654 0
	movl	$43, %eax
	movl	8(%esp), %ecx
	movl	4(%esp), %edx
#APP
# 654 "kapi.c" 1
	.code32				
	pushl	%ecx				
	pushl	%edx				
	pushl	$0				
	pushl	%eax				
	int	$0x7F			
	popl	%eax			
	popl	%eax			
	popl	%eax				
	popl	%edx				
	
# 0 "" 2
.LVL81:
	.loc 1 656 0
#NO_APP
	ret
	.cfi_endproc
.LFE42:
	.size	KMemAlloc, .-KMemAlloc
	.section	.text.unlikely
.LCOLDE42:
	.text
.LHOTE42:
	.section	.text.unlikely
.LCOLDB43:
	.text
.LHOTB43:
	.p2align 4,,15
	.globl	KMemFree
	.type	KMemFree, @function
KMemFree:
.LFB43:
	.loc 1 659 0
	.cfi_startproc
.LVL82:
	pushl	%ebx
	.cfi_def_cfa_offset 8
	.cfi_offset 3, -8
	.loc 1 660 0
	movl	$44, %eax
	movl	12(%esp), %edx
	movl	8(%esp), %ecx
	movl	16(%esp), %ebx
#APP
# 660 "kapi.c" 1
	.code32				
	pushl	%ebx				
	pushl	%edx				
	pushl	%ecx				
	pushl	$0				
	pushl	%eax				
	int	$0x7F			
	popl	%eax			
	popl	%eax			
	popl	%edx				
	popl	%eax				
	popl	%ecx				
	
# 0 "" 2
.LVL83:
	.loc 1 663 0
#NO_APP
	popl	%ebx
	.cfi_restore 3
	.cfi_def_cfa_offset 4
	ret
	.cfi_endproc
.LFE43:
	.size	KMemFree, .-KMemFree
	.section	.text.unlikely
.LCOLDE43:
	.text
.LHOTE43:
	.section	.text.unlikely
.LCOLDB44:
	.text
.LHOTB44:
	.p2align 4,,15
	.globl	PrintLine
	.type	PrintLine, @function
PrintLine:
.LFB44:
	.loc 1 666 0
	.cfi_startproc
.LVL84:
	.loc 1 667 0
	movl	$45, %eax
	movl	4(%esp), %edx
#APP
# 667 "kapi.c" 1
	.code32			
	pushl 	%edx			
	pushl	$0			
	pushl	%eax			
	int	$0x7F		
	popl	%eax		
	popl	%eax		
	popl	%eax			
	
# 0 "" 2
.LVL85:
#NO_APP
	ret
	.cfi_endproc
.LFE44:
	.size	PrintLine, .-PrintLine
	.section	.text.unlikely
.LCOLDE44:
	.text
.LHOTE44:
	.section	.text.unlikely
.LCOLDB45:
	.text
.LHOTB45:
	.p2align 4,,15
	.globl	PrintChar
	.type	PrintChar, @function
PrintChar:
.LFB45:
	.loc 1 672 0
	.cfi_startproc
.LVL86:
	.loc 1 673 0
	movzwl	4(%esp), %edx
.LVL87:
	.loc 1 674 0
	movl	$46, %eax
#APP
# 674 "kapi.c" 1
	.code32			
	pushl 	%edx			
	pushl	$0			
	pushl	%eax			
	int	$0x7F		
	popl	%eax		
	popl	%eax		
	popl	%eax			
	
# 0 "" 2
.LVL88:
#NO_APP
	ret
	.cfi_endproc
.LFE45:
	.size	PrintChar, .-PrintChar
	.section	.text.unlikely
.LCOLDE45:
	.text
.LHOTE45:
	.section	.text.unlikely
.LCOLDB46:
	.text
.LHOTB46:
	.p2align 4,,15
	.globl	RegisterSystemCall
	.type	RegisterSystemCall, @function
RegisterSystemCall:
.LFB46:
	.loc 1 680 0
	.cfi_startproc
.LVL89:
	pushl	%ebx
	.cfi_def_cfa_offset 8
	.cfi_offset 3, -8
	.loc 1 681 0
	movl	$47, %eax
	movl	12(%esp), %edx
	movl	8(%esp), %ecx
	movl	16(%esp), %ebx
#APP
# 681 "kapi.c" 1
	.code32				
	pushl	%ebx				
	pushl	%edx				
	pushl	%ecx				
	pushl	$0				
	pushl	%eax				
	int	$0x7F			
	popl	%eax			
	popl	%eax			
	popl	%edx				
	popl	%eax				
	popl	%ecx				
	
# 0 "" 2
.LVL90:
	.loc 1 685 0
#NO_APP
	popl	%ebx
	.cfi_restore 3
	.cfi_def_cfa_offset 4
	ret
	.cfi_endproc
.LFE46:
	.size	RegisterSystemCall, .-RegisterSystemCall
	.section	.text.unlikely
.LCOLDE46:
	.text
.LHOTE46:
	.section	.text.unlikely
.LCOLDB47:
	.text
.LHOTB47:
	.p2align 4,,15
	.globl	ReplaceShell
	.type	ReplaceShell, @function
ReplaceShell:
.LFB47:
	.loc 1 688 0
	.cfi_startproc
.LVL91:
	.loc 1 689 0
	movl	$48, %eax
	movl	4(%esp), %edx
#APP
# 689 "kapi.c" 1
	.code32			
	pushl 	%edx			
	pushl	$0			
	pushl	%eax			
	int	$0x7F		
	popl	%eax		
	popl	%eax		
	popl	%eax			
	
# 0 "" 2
.LVL92:
	.loc 1 691 0
#NO_APP
	ret
	.cfi_endproc
.LFE47:
	.size	ReplaceShell, .-ReplaceShell
	.section	.text.unlikely
.LCOLDE47:
	.text
.LHOTE47:
	.section	.text.unlikely
.LCOLDB48:
	.text
.LHOTB48:
	.p2align 4,,15
	.globl	LoadDriver
	.type	LoadDriver, @function
LoadDriver:
.LFB48:
	.loc 1 694 0
	.cfi_startproc
.LVL93:
	.loc 1 695 0
	movl	$49, %eax
	movl	4(%esp), %edx
#APP
# 695 "kapi.c" 1
	.code32			
	pushl 	%edx			
	pushl	$0			
	pushl	%eax			
	int	$0x7F		
	popl	%eax		
	popl	%eax		
	popl	%eax			
	
# 0 "" 2
.LVL94:
	.loc 1 697 0
#NO_APP
	ret
	.cfi_endproc
.LFE48:
	.size	LoadDriver, .-LoadDriver
	.section	.text.unlikely
.LCOLDE48:
	.text
.LHOTE48:
	.section	.text.unlikely
.LCOLDB49:
	.text
.LHOTB49:
	.p2align 4,,15
	.globl	GetCurrentThread
	.type	GetCurrentThread, @function
GetCurrentThread:
.LFB49:
	.loc 1 700 0
	.cfi_startproc
	.loc 1 701 0
	movl	$50, %eax
#APP
# 701 "kapi.c" 1
	.code32 			
	pushl		$0		
	pushl		%eax		
	int		$0x7F	
	popl		%eax	
	popl		%eax	
	
# 0 "" 2
	.loc 1 702 0
#NO_APP
	ret
	.cfi_endproc
.LFE49:
	.size	GetCurrentThread, .-GetCurrentThread
	.section	.text.unlikely
.LCOLDE49:
	.text
.LHOTE49:
	.section	.text.unlikely
.LCOLDB50:
	.text
.LHOTB50:
	.p2align 4,,15
	.globl	SwitchToGraphic
	.type	SwitchToGraphic, @function
SwitchToGraphic:
.LFB50:
	.loc 1 705 0
	.cfi_startproc
	.loc 1 706 0
	movl	$52, %eax
#APP
# 706 "kapi.c" 1
	.code32 			
	pushl		$0		
	pushl		%eax		
	int		$0x7F	
	popl		%eax	
	popl		%eax	
	
# 0 "" 2
	.loc 1 707 0
#NO_APP
	ret
	.cfi_endproc
.LFE50:
	.size	SwitchToGraphic, .-SwitchToGraphic
	.section	.text.unlikely
.LCOLDE50:
	.text
.LHOTE50:
	.section	.text.unlikely
.LCOLDB51:
	.text
.LHOTB51:
	.p2align 4,,15
	.globl	SwitchToText
	.type	SwitchToText, @function
SwitchToText:
.LFB51:
	.loc 1 710 0
	.cfi_startproc
	.loc 1 711 0
	movl	$53, %eax
#APP
# 711 "kapi.c" 1
	.code32 			
	pushl		$0		
	pushl		%eax		
	int		$0x7F	
	popl		%eax	
	popl		%eax	
	
# 0 "" 2
#NO_APP
	ret
	.cfi_endproc
.LFE51:
	.size	SwitchToText, .-SwitchToText
	.section	.text.unlikely
.LCOLDE51:
	.text
.LHOTE51:
	.section	.text.unlikely
.LCOLDB52:
	.text
.LHOTB52:
	.p2align 4,,15
	.globl	SetFocusThread
	.type	SetFocusThread, @function
SetFocusThread:
.LFB52:
	.loc 1 715 0
	.cfi_startproc
.LVL95:
	.loc 1 716 0
	movl	$54, %eax
	movl	4(%esp), %edx
#APP
# 716 "kapi.c" 1
	.code32			
	pushl 	%edx			
	pushl	$0			
	pushl	%eax			
	int	$0x7F		
	popl	%eax		
	popl	%eax		
	popl	%eax			
	
# 0 "" 2
.LVL96:
	.loc 1 718 0
#NO_APP
	ret
	.cfi_endproc
.LFE52:
	.size	SetFocusThread, .-SetFocusThread
	.section	.text.unlikely
.LCOLDE52:
	.text
.LHOTE52:
.Letext0:
	.section	.text.unlikely
.Letext_cold0:
	.file 2 "kapi.h"
	.section	.debug_info,"",@progbits
.Ldebug_info0:
	.long	0xe1f
	.value	0x4
	.long	.Ldebug_abbrev0
	.byte	0x4
	.uleb128 0x1
	.long	.LASF157
	.byte	0x1
	.long	.LASF158
	.long	.LASF159
	.long	.Ltext0
	.long	.Letext0-.Ltext0
	.long	.Ldebug_line0
	.uleb128 0x2
	.byte	0x1
	.byte	0x8
	.long	.LASF2
	.uleb128 0x3
	.long	.LASF0
	.byte	0x2
	.byte	0x19
	.long	0x25
	.uleb128 0x3
	.long	.LASF1
	.byte	0x2
	.byte	0x1a
	.long	0x42
	.uleb128 0x2
	.byte	0x1
	.byte	0x6
	.long	.LASF3
	.uleb128 0x2
	.byte	0x2
	.byte	0x5
	.long	.LASF4
	.uleb128 0x3
	.long	.LASF5
	.byte	0x2
	.byte	0x1c
	.long	0x5b
	.uleb128 0x4
	.byte	0x4
	.long	0x42
	.uleb128 0x3
	.long	.LASF6
	.byte	0x2
	.byte	0x1d
	.long	0x6c
	.uleb128 0x2
	.byte	0x2
	.byte	0x7
	.long	.LASF7
	.uleb128 0x5
	.byte	0x4
	.byte	0x5
	.string	"int"
	.uleb128 0x2
	.byte	0x4
	.byte	0x7
	.long	.LASF8
	.uleb128 0x3
	.long	.LASF9
	.byte	0x2
	.byte	0x22
	.long	0x8c
	.uleb128 0x2
	.byte	0x4
	.byte	0x7
	.long	.LASF10
	.uleb128 0x2
	.byte	0x4
	.byte	0x5
	.long	.LASF11
	.uleb128 0x3
	.long	.LASF12
	.byte	0x2
	.byte	0x25
	.long	0x8c
	.uleb128 0x3
	.long	.LASF13
	.byte	0x2
	.byte	0x26
	.long	0xb0
	.uleb128 0x6
	.byte	0x4
	.uleb128 0x3
	.long	.LASF14
	.byte	0x2
	.byte	0x28
	.long	0xa5
	.uleb128 0x7
	.long	.LASF160
	.byte	0x8
	.byte	0x2
	.byte	0xf9
	.long	0xee
	.uleb128 0x8
	.long	.LASF15
	.byte	0x2
	.byte	0xfa
	.long	0x61
	.byte	0
	.uleb128 0x8
	.long	.LASF16
	.byte	0x2
	.byte	0xfb
	.long	0x61
	.byte	0x2
	.uleb128 0x8
	.long	.LASF17
	.byte	0x2
	.byte	0xfc
	.long	0x81
	.byte	0x4
	.byte	0
	.uleb128 0x9
	.string	"MSG"
	.byte	0x2
	.byte	0xfd
	.long	0xbd
	.uleb128 0xa
	.byte	0x8
	.byte	0x2
	.byte	0xff
	.long	0x11c
	.uleb128 0xb
	.long	.LASF18
	.byte	0x2
	.value	0x100
	.long	0x81
	.byte	0
	.uleb128 0xb
	.long	.LASF19
	.byte	0x2
	.value	0x101
	.long	0x81
	.byte	0x4
	.byte	0
	.uleb128 0xc
	.long	.LASF20
	.byte	0x2
	.value	0x102
	.long	0xf9
	.uleb128 0xd
	.value	0x23c
	.byte	0x2
	.value	0x104
	.long	0x1b6
	.uleb128 0xb
	.long	.LASF21
	.byte	0x2
	.value	0x105
	.long	0x81
	.byte	0
	.uleb128 0xb
	.long	.LASF22
	.byte	0x2
	.value	0x106
	.long	0x11c
	.byte	0x4
	.uleb128 0xb
	.long	.LASF23
	.byte	0x2
	.value	0x107
	.long	0x11c
	.byte	0xc
	.uleb128 0xb
	.long	.LASF24
	.byte	0x2
	.value	0x108
	.long	0x11c
	.byte	0x14
	.uleb128 0xb
	.long	.LASF25
	.byte	0x2
	.value	0x109
	.long	0x81
	.byte	0x1c
	.uleb128 0xb
	.long	.LASF26
	.byte	0x2
	.value	0x10a
	.long	0x81
	.byte	0x20
	.uleb128 0xb
	.long	.LASF27
	.byte	0x2
	.value	0x10b
	.long	0x81
	.byte	0x24
	.uleb128 0xb
	.long	.LASF28
	.byte	0x2
	.value	0x10c
	.long	0x81
	.byte	0x28
	.uleb128 0xb
	.long	.LASF29
	.byte	0x2
	.value	0x10d
	.long	0x1b6
	.byte	0x2c
	.uleb128 0xe
	.long	.LASF30
	.byte	0x2
	.value	0x10e
	.long	0x1ce
	.value	0x22c
	.byte	0
	.uleb128 0xf
	.long	0x37
	.long	0x1c7
	.uleb128 0x10
	.long	0x1c7
	.value	0x1ff
	.byte	0
	.uleb128 0x2
	.byte	0x4
	.byte	0x7
	.long	.LASF31
	.uleb128 0xf
	.long	0x37
	.long	0x1de
	.uleb128 0x11
	.long	0x1c7
	.byte	0xc
	.byte	0
	.uleb128 0xc
	.long	.LASF32
	.byte	0x2
	.value	0x10f
	.long	0x128
	.uleb128 0xc
	.long	.LASF33
	.byte	0x2
	.value	0x111
	.long	0x1f6
	.uleb128 0x4
	.byte	0x4
	.long	0x1fc
	.uleb128 0x12
	.long	0x81
	.long	0x20b
	.uleb128 0x13
	.long	0xa5
	.byte	0
	.uleb128 0xc
	.long	.LASF34
	.byte	0x2
	.value	0x112
	.long	0x1f6
	.uleb128 0xc
	.long	.LASF35
	.byte	0x2
	.value	0x113
	.long	0x223
	.uleb128 0x4
	.byte	0x4
	.long	0x229
	.uleb128 0x12
	.long	0x9a
	.long	0x23d
	.uleb128 0x13
	.long	0xa5
	.uleb128 0x13
	.long	0xa5
	.byte	0
	.uleb128 0xc
	.long	.LASF36
	.byte	0x2
	.value	0x1d9
	.long	0x223
	.uleb128 0xc
	.long	.LASF37
	.byte	0x2
	.value	0x1de
	.long	0x255
	.uleb128 0x4
	.byte	0x4
	.long	0x25b
	.uleb128 0x12
	.long	0x9a
	.long	0x26a
	.uleb128 0x13
	.long	0xb2
	.byte	0
	.uleb128 0x14
	.long	.LASF46
	.byte	0x1
	.value	0x134
	.long	0xb2
	.long	.LFB0
	.long	.LFE0-.LFB0
	.uleb128 0x1
	.byte	0x9c
	.long	0x2f4
	.uleb128 0x15
	.long	.LASF38
	.byte	0x1
	.value	0x134
	.long	0x81
	.long	.LLST0
	.uleb128 0x15
	.long	.LASF39
	.byte	0x1
	.value	0x135
	.long	0x81
	.long	.LLST1
	.uleb128 0x15
	.long	.LASF40
	.byte	0x1
	.value	0x136
	.long	0x81
	.long	.LLST2
	.uleb128 0x15
	.long	.LASF41
	.byte	0x1
	.value	0x137
	.long	0x1ea
	.long	.LLST3
	.uleb128 0x15
	.long	.LASF42
	.byte	0x1
	.value	0x138
	.long	0xa5
	.long	.LLST4
	.uleb128 0x15
	.long	.LASF43
	.byte	0x1
	.value	0x139
	.long	0xa5
	.long	.LLST5
	.uleb128 0x16
	.long	.LASF44
	.byte	0x1
	.value	0x13a
	.long	0x50
	.uleb128 0x2
	.byte	0x91
	.sleb128 24
	.byte	0
	.uleb128 0x17
	.long	.LASF61
	.byte	0x1
	.value	0x142
	.long	.LFB1
	.long	.LFE1-.LFB1
	.uleb128 0x1
	.byte	0x9c
	.long	0x31b
	.uleb128 0x15
	.long	.LASF45
	.byte	0x1
	.value	0x142
	.long	0xb2
	.long	.LLST6
	.byte	0
	.uleb128 0x14
	.long	.LASF47
	.byte	0x1
	.value	0x147
	.long	0x81
	.long	.LFB2
	.long	.LFE2-.LFB2
	.uleb128 0x1
	.byte	0x9c
	.long	0x346
	.uleb128 0x15
	.long	.LASF48
	.byte	0x1
	.value	0x147
	.long	0x81
	.long	.LLST7
	.byte	0
	.uleb128 0x18
	.long	.LASF70
	.byte	0x1
	.value	0x14c
	.long	0x81
	.long	.LFB3
	.long	.LFE3-.LFB3
	.uleb128 0x1
	.byte	0x9c
	.uleb128 0x14
	.long	.LASF49
	.byte	0x1
	.value	0x151
	.long	0x81
	.long	.LFB4
	.long	.LFE4-.LFB4
	.uleb128 0x1
	.byte	0x9c
	.long	0x387
	.uleb128 0x15
	.long	.LASF45
	.byte	0x1
	.value	0x151
	.long	0xb2
	.long	.LLST8
	.byte	0
	.uleb128 0x14
	.long	.LASF50
	.byte	0x1
	.value	0x156
	.long	0x81
	.long	.LFB5
	.long	.LFE5-.LFB5
	.uleb128 0x1
	.byte	0x9c
	.long	0x3c2
	.uleb128 0x15
	.long	.LASF45
	.byte	0x1
	.value	0x156
	.long	0xb2
	.long	.LLST9
	.uleb128 0x15
	.long	.LASF40
	.byte	0x1
	.value	0x156
	.long	0x81
	.long	.LLST10
	.byte	0
	.uleb128 0x14
	.long	.LASF51
	.byte	0x1
	.value	0x15b
	.long	0x9a
	.long	.LFB6
	.long	.LFE6-.LFB6
	.uleb128 0x1
	.byte	0x9c
	.long	0x3ed
	.uleb128 0x15
	.long	.LASF52
	.byte	0x1
	.value	0x15b
	.long	0x3ed
	.long	.LLST11
	.byte	0
	.uleb128 0x4
	.byte	0x4
	.long	0xee
	.uleb128 0x14
	.long	.LASF53
	.byte	0x1
	.value	0x160
	.long	0x9a
	.long	.LFB7
	.long	.LFE7-.LFB7
	.uleb128 0x1
	.byte	0x9c
	.long	0x42e
	.uleb128 0x15
	.long	.LASF45
	.byte	0x1
	.value	0x160
	.long	0xb2
	.long	.LLST12
	.uleb128 0x15
	.long	.LASF52
	.byte	0x1
	.value	0x160
	.long	0x3ed
	.long	.LLST13
	.byte	0
	.uleb128 0x14
	.long	.LASF54
	.byte	0x1
	.value	0x165
	.long	0x9a
	.long	.LFB8
	.long	.LFE8-.LFB8
	.uleb128 0x1
	.byte	0x9c
	.long	0x459
	.uleb128 0x15
	.long	.LASF55
	.byte	0x1
	.value	0x165
	.long	0x81
	.long	.LLST14
	.byte	0
	.uleb128 0x14
	.long	.LASF56
	.byte	0x1
	.value	0x16a
	.long	0xb2
	.long	.LFB9
	.long	.LFE9-.LFB9
	.uleb128 0x1
	.byte	0x9c
	.long	0x4c4
	.uleb128 0x15
	.long	.LASF57
	.byte	0x1
	.value	0x16a
	.long	0x81
	.long	.LLST15
	.uleb128 0x15
	.long	.LASF55
	.byte	0x1
	.value	0x16b
	.long	0x81
	.long	.LLST16
	.uleb128 0x15
	.long	.LASF58
	.byte	0x1
	.value	0x16c
	.long	0x20b
	.long	.LLST17
	.uleb128 0x15
	.long	.LASF59
	.byte	0x1
	.value	0x16d
	.long	0xa5
	.long	.LLST18
	.uleb128 0x15
	.long	.LASF60
	.byte	0x1
	.value	0x16e
	.long	0x81
	.long	.LLST19
	.byte	0
	.uleb128 0x17
	.long	.LASF62
	.byte	0x1
	.value	0x178
	.long	.LFB10
	.long	.LFE10-.LFB10
	.uleb128 0x1
	.byte	0x9c
	.long	0x4eb
	.uleb128 0x15
	.long	.LASF63
	.byte	0x1
	.value	0x178
	.long	0xb2
	.long	.LLST20
	.byte	0
	.uleb128 0x14
	.long	.LASF64
	.byte	0x1
	.value	0x17d
	.long	0xb2
	.long	.LFB11
	.long	.LFE11-.LFB11
	.uleb128 0x1
	.byte	0x9c
	.long	0x516
	.uleb128 0x15
	.long	.LASF65
	.byte	0x1
	.value	0x17d
	.long	0x9a
	.long	.LLST21
	.byte	0
	.uleb128 0x17
	.long	.LASF66
	.byte	0x1
	.value	0x182
	.long	.LFB12
	.long	.LFE12-.LFB12
	.uleb128 0x1
	.byte	0x9c
	.long	0x53d
	.uleb128 0x15
	.long	.LASF67
	.byte	0x1
	.value	0x182
	.long	0xb2
	.long	.LLST22
	.byte	0
	.uleb128 0x14
	.long	.LASF68
	.byte	0x1
	.value	0x187
	.long	0x81
	.long	.LFB13
	.long	.LFE13-.LFB13
	.uleb128 0x1
	.byte	0x9c
	.long	0x568
	.uleb128 0x15
	.long	.LASF67
	.byte	0x1
	.value	0x187
	.long	0xb2
	.long	.LLST23
	.byte	0
	.uleb128 0x14
	.long	.LASF69
	.byte	0x1
	.value	0x18c
	.long	0x81
	.long	.LFB14
	.long	.LFE14-.LFB14
	.uleb128 0x1
	.byte	0x9c
	.long	0x593
	.uleb128 0x15
	.long	.LASF67
	.byte	0x1
	.value	0x18c
	.long	0xb2
	.long	.LLST24
	.byte	0
	.uleb128 0x18
	.long	.LASF71
	.byte	0x1
	.value	0x191
	.long	0xb2
	.long	.LFB15
	.long	.LFE15-.LFB15
	.uleb128 0x1
	.byte	0x9c
	.uleb128 0x17
	.long	.LASF72
	.byte	0x1
	.value	0x196
	.long	.LFB16
	.long	.LFE16-.LFB16
	.uleb128 0x1
	.byte	0x9c
	.long	0x5d0
	.uleb128 0x15
	.long	.LASF73
	.byte	0x1
	.value	0x196
	.long	0xb2
	.long	.LLST25
	.byte	0
	.uleb128 0x14
	.long	.LASF74
	.byte	0x1
	.value	0x19b
	.long	0x81
	.long	.LFB17
	.long	.LFE17-.LFB17
	.uleb128 0x1
	.byte	0x9c
	.long	0x5fb
	.uleb128 0x15
	.long	.LASF73
	.byte	0x1
	.value	0x19b
	.long	0xb2
	.long	.LLST26
	.byte	0
	.uleb128 0x14
	.long	.LASF75
	.byte	0x1
	.value	0x1a0
	.long	0x81
	.long	.LFB18
	.long	.LFE18-.LFB18
	.uleb128 0x1
	.byte	0x9c
	.long	0x626
	.uleb128 0x15
	.long	.LASF76
	.byte	0x1
	.value	0x1a0
	.long	0xb2
	.long	.LLST27
	.byte	0
	.uleb128 0x14
	.long	.LASF77
	.byte	0x1
	.value	0x1a5
	.long	0x81
	.long	.LFB19
	.long	.LFE19-.LFB19
	.uleb128 0x1
	.byte	0x9c
	.long	0x661
	.uleb128 0x15
	.long	.LASF76
	.byte	0x1
	.value	0x1a5
	.long	0xb2
	.long	.LLST28
	.uleb128 0x15
	.long	.LASF55
	.byte	0x1
	.value	0x1a5
	.long	0x81
	.long	.LLST29
	.byte	0
	.uleb128 0x14
	.long	.LASF78
	.byte	0x1
	.value	0x1ab
	.long	0xb2
	.long	.LFB20
	.long	.LFE20-.LFB20
	.uleb128 0x1
	.byte	0x9c
	.long	0x6ac
	.uleb128 0x15
	.long	.LASF79
	.byte	0x1
	.value	0x1ab
	.long	0x217
	.long	.LLST30
	.uleb128 0x15
	.long	.LASF59
	.byte	0x1
	.value	0x1ac
	.long	0xa5
	.long	.LLST31
	.uleb128 0x15
	.long	.LASF80
	.byte	0x1
	.value	0x1ad
	.long	0x2c
	.long	.LLST32
	.byte	0
	.uleb128 0x17
	.long	.LASF81
	.byte	0x1
	.value	0x1da
	.long	.LFB21
	.long	.LFE21-.LFB21
	.uleb128 0x1
	.byte	0x9c
	.long	0x6d3
	.uleb128 0x15
	.long	.LASF82
	.byte	0x1
	.value	0x1da
	.long	0xb2
	.long	.LLST33
	.byte	0
	.uleb128 0x14
	.long	.LASF83
	.byte	0x1
	.value	0x1df
	.long	0xa5
	.long	.LFB22
	.long	.LFE22-.LFB22
	.uleb128 0x1
	.byte	0x9c
	.long	0x73e
	.uleb128 0x15
	.long	.LASF84
	.byte	0x1
	.value	0x1df
	.long	0xa5
	.long	.LLST34
	.uleb128 0x15
	.long	.LASF85
	.byte	0x1
	.value	0x1e0
	.long	0x81
	.long	.LLST35
	.uleb128 0x15
	.long	.LASF86
	.byte	0x1
	.value	0x1e1
	.long	0x81
	.long	.LLST36
	.uleb128 0x15
	.long	.LASF87
	.byte	0x1
	.value	0x1e2
	.long	0x81
	.long	.LLST37
	.uleb128 0x15
	.long	.LASF88
	.byte	0x1
	.value	0x1e3
	.long	0x73e
	.long	.LLST38
	.byte	0
	.uleb128 0x4
	.byte	0x4
	.long	0x37
	.uleb128 0x17
	.long	.LASF89
	.byte	0x1
	.value	0x1ed
	.long	.LFB23
	.long	.LFE23-.LFB23
	.uleb128 0x1
	.byte	0x9c
	.long	0x76b
	.uleb128 0x15
	.long	.LASF90
	.byte	0x1
	.value	0x1ed
	.long	0xa5
	.long	.LLST39
	.byte	0
	.uleb128 0x14
	.long	.LASF91
	.byte	0x1
	.value	0x1f2
	.long	0xb2
	.long	.LFB24
	.long	.LFE24-.LFB24
	.uleb128 0x1
	.byte	0x9c
	.long	0x7c6
	.uleb128 0x15
	.long	.LASF92
	.byte	0x1
	.value	0x1f2
	.long	0x50
	.long	.LLST40
	.uleb128 0x15
	.long	.LASF93
	.byte	0x1
	.value	0x1f3
	.long	0x81
	.long	.LLST41
	.uleb128 0x15
	.long	.LASF94
	.byte	0x1
	.value	0x1f4
	.long	0x81
	.long	.LLST42
	.uleb128 0x15
	.long	.LASF43
	.byte	0x1
	.value	0x1f5
	.long	0xa5
	.long	.LLST43
	.byte	0
	.uleb128 0x14
	.long	.LASF95
	.byte	0x1
	.value	0x1fe
	.long	0x9a
	.long	.LFB25
	.long	.LFE25-.LFB25
	.uleb128 0x1
	.byte	0x9c
	.long	0x821
	.uleb128 0x15
	.long	.LASF96
	.byte	0x1
	.value	0x1fe
	.long	0xb2
	.long	.LLST44
	.uleb128 0x15
	.long	.LASF97
	.byte	0x1
	.value	0x1ff
	.long	0x81
	.long	.LLST45
	.uleb128 0x15
	.long	.LASF98
	.byte	0x1
	.value	0x200
	.long	0xa5
	.long	.LLST46
	.uleb128 0x15
	.long	.LASF99
	.byte	0x1
	.value	0x201
	.long	0x821
	.long	.LLST47
	.byte	0
	.uleb128 0x4
	.byte	0x4
	.long	0x81
	.uleb128 0x14
	.long	.LASF100
	.byte	0x1
	.value	0x20a
	.long	0x9a
	.long	.LFB26
	.long	.LFE26-.LFB26
	.uleb128 0x1
	.byte	0x9c
	.long	0x882
	.uleb128 0x15
	.long	.LASF96
	.byte	0x1
	.value	0x20a
	.long	0xb2
	.long	.LLST48
	.uleb128 0x15
	.long	.LASF101
	.byte	0x1
	.value	0x20b
	.long	0x81
	.long	.LLST49
	.uleb128 0x15
	.long	.LASF98
	.byte	0x1
	.value	0x20c
	.long	0xa5
	.long	.LLST50
	.uleb128 0x15
	.long	.LASF102
	.byte	0x1
	.value	0x20d
	.long	0x821
	.long	.LLST51
	.byte	0
	.uleb128 0x17
	.long	.LASF103
	.byte	0x1
	.value	0x216
	.long	.LFB27
	.long	.LFE27-.LFB27
	.uleb128 0x1
	.byte	0x9c
	.long	0x8a9
	.uleb128 0x15
	.long	.LASF96
	.byte	0x1
	.value	0x216
	.long	0xb2
	.long	.LLST52
	.byte	0
	.uleb128 0x14
	.long	.LASF104
	.byte	0x1
	.value	0x21c
	.long	0x9a
	.long	.LFB28
	.long	.LFE28-.LFB28
	.uleb128 0x1
	.byte	0x9c
	.long	0x8d4
	.uleb128 0x15
	.long	.LASF105
	.byte	0x1
	.value	0x21c
	.long	0x50
	.long	.LLST53
	.byte	0
	.uleb128 0x14
	.long	.LASF106
	.byte	0x1
	.value	0x221
	.long	0x9a
	.long	.LFB29
	.long	.LFE29-.LFB29
	.uleb128 0x1
	.byte	0x9c
	.long	0x8ff
	.uleb128 0x15
	.long	.LASF92
	.byte	0x1
	.value	0x221
	.long	0x50
	.long	.LLST54
	.byte	0
	.uleb128 0x14
	.long	.LASF107
	.byte	0x1
	.value	0x226
	.long	0xb2
	.long	.LFB30
	.long	.LFE30-.LFB30
	.uleb128 0x1
	.byte	0x9c
	.long	0x93a
	.uleb128 0x15
	.long	.LASF105
	.byte	0x1
	.value	0x226
	.long	0x50
	.long	.LLST55
	.uleb128 0x15
	.long	.LASF108
	.byte	0x1
	.value	0x227
	.long	0x93a
	.long	.LLST56
	.byte	0
	.uleb128 0x4
	.byte	0x4
	.long	0x1de
	.uleb128 0x14
	.long	.LASF109
	.byte	0x1
	.value	0x22e
	.long	0x9a
	.long	.LFB31
	.long	.LFE31-.LFB31
	.uleb128 0x1
	.byte	0x9c
	.long	0x98b
	.uleb128 0x15
	.long	.LASF105
	.byte	0x1
	.value	0x22e
	.long	0x50
	.long	.LLST57
	.uleb128 0x15
	.long	.LASF110
	.byte	0x1
	.value	0x22f
	.long	0xb2
	.long	.LLST58
	.uleb128 0x15
	.long	.LASF108
	.byte	0x1
	.value	0x230
	.long	0x93a
	.long	.LLST59
	.byte	0
	.uleb128 0x17
	.long	.LASF111
	.byte	0x1
	.value	0x237
	.long	.LFB32
	.long	.LFE32-.LFB32
	.uleb128 0x1
	.byte	0x9c
	.long	0x9c2
	.uleb128 0x15
	.long	.LASF105
	.byte	0x1
	.value	0x237
	.long	0x50
	.long	.LLST60
	.uleb128 0x15
	.long	.LASF110
	.byte	0x1
	.value	0x238
	.long	0xb2
	.long	.LLST61
	.byte	0
	.uleb128 0x14
	.long	.LASF112
	.byte	0x1
	.value	0x23f
	.long	0x81
	.long	.LFB33
	.long	.LFE33-.LFB33
	.uleb128 0x1
	.byte	0x9c
	.long	0x9ed
	.uleb128 0x15
	.long	.LASF92
	.byte	0x1
	.value	0x23f
	.long	0x50
	.long	.LLST62
	.byte	0
	.uleb128 0x14
	.long	.LASF113
	.byte	0x1
	.value	0x244
	.long	0x81
	.long	.LFB34
	.long	.LFE34-.LFB34
	.uleb128 0x1
	.byte	0x9c
	.long	0xa28
	.uleb128 0x15
	.long	.LASF96
	.byte	0x1
	.value	0x244
	.long	0xb2
	.long	.LLST63
	.uleb128 0x15
	.long	.LASF114
	.byte	0x1
	.value	0x244
	.long	0x821
	.long	.LLST64
	.byte	0
	.uleb128 0x14
	.long	.LASF115
	.byte	0x1
	.value	0x24a
	.long	0x9a
	.long	.LFB35
	.long	.LFE35-.LFB35
	.uleb128 0x1
	.byte	0x9c
	.long	0xa53
	.uleb128 0x15
	.long	.LASF105
	.byte	0x1
	.value	0x24a
	.long	0x50
	.long	.LLST65
	.byte	0
	.uleb128 0x14
	.long	.LASF116
	.byte	0x1
	.value	0x250
	.long	0x9a
	.long	.LFB36
	.long	.LFE36-.LFB36
	.uleb128 0x1
	.byte	0x9c
	.long	0xa7e
	.uleb128 0x15
	.long	.LASF96
	.byte	0x1
	.value	0x250
	.long	0xb2
	.long	.LLST66
	.byte	0
	.uleb128 0x14
	.long	.LASF117
	.byte	0x1
	.value	0x255
	.long	0x9a
	.long	.LFB37
	.long	.LFE37-.LFB37
	.uleb128 0x1
	.byte	0x9c
	.long	0xb08
	.uleb128 0x15
	.long	.LASF96
	.byte	0x1
	.value	0x255
	.long	0xb2
	.long	.LLST67
	.uleb128 0x15
	.long	.LASF118
	.byte	0x1
	.value	0x256
	.long	0x81
	.long	.LLST68
	.uleb128 0x15
	.long	.LASF119
	.byte	0x1
	.value	0x257
	.long	0x81
	.long	.LLST69
	.uleb128 0x15
	.long	.LASF120
	.byte	0x1
	.value	0x258
	.long	0xa5
	.long	.LLST70
	.uleb128 0x15
	.long	.LASF121
	.byte	0x1
	.value	0x259
	.long	0x81
	.long	.LLST71
	.uleb128 0x15
	.long	.LASF122
	.byte	0x1
	.value	0x25a
	.long	0xa5
	.long	.LLST72
	.uleb128 0x16
	.long	.LASF123
	.byte	0x1
	.value	0x25b
	.long	0x821
	.uleb128 0x2
	.byte	0x91
	.sleb128 24
	.byte	0
	.uleb128 0x14
	.long	.LASF124
	.byte	0x1
	.value	0x264
	.long	0x9a
	.long	.LFB38
	.long	.LFE38-.LFB38
	.uleb128 0x1
	.byte	0x9c
	.long	0xb63
	.uleb128 0x15
	.long	.LASF96
	.byte	0x1
	.value	0x264
	.long	0xb2
	.long	.LLST73
	.uleb128 0x15
	.long	.LASF125
	.byte	0x1
	.value	0x265
	.long	0x821
	.long	.LLST74
	.uleb128 0x15
	.long	.LASF126
	.byte	0x1
	.value	0x266
	.long	0x821
	.long	.LLST75
	.uleb128 0x15
	.long	.LASF127
	.byte	0x1
	.value	0x267
	.long	0x81
	.long	.LLST76
	.byte	0
	.uleb128 0x14
	.long	.LASF128
	.byte	0x1
	.value	0x26f
	.long	0x9a
	.long	.LFB39
	.long	.LFE39-.LFB39
	.uleb128 0x1
	.byte	0x9c
	.long	0xb8e
	.uleb128 0x15
	.long	.LASF96
	.byte	0x1
	.value	0x26f
	.long	0xb2
	.long	.LLST77
	.byte	0
	.uleb128 0x14
	.long	.LASF129
	.byte	0x1
	.value	0x275
	.long	0xb2
	.long	.LFB40
	.long	.LFE40-.LFB40
	.uleb128 0x1
	.byte	0x9c
	.long	0xc18
	.uleb128 0x15
	.long	.LASF130
	.byte	0x1
	.value	0x275
	.long	0x50
	.long	.LLST78
	.uleb128 0x15
	.long	.LASF131
	.byte	0x1
	.value	0x276
	.long	0x81
	.long	.LLST79
	.uleb128 0x15
	.long	.LASF132
	.byte	0x1
	.value	0x277
	.long	0x81
	.long	.LLST80
	.uleb128 0x15
	.long	.LASF133
	.byte	0x1
	.value	0x278
	.long	0x81
	.long	.LLST81
	.uleb128 0x15
	.long	.LASF134
	.byte	0x1
	.value	0x279
	.long	0x81
	.long	.LLST82
	.uleb128 0x15
	.long	.LASF135
	.byte	0x1
	.value	0x27a
	.long	0xa5
	.long	.LLST83
	.uleb128 0x16
	.long	.LASF136
	.byte	0x1
	.value	0x27b
	.long	0xb2
	.uleb128 0x2
	.byte	0x91
	.sleb128 24
	.byte	0
	.uleb128 0x17
	.long	.LASF137
	.byte	0x1
	.value	0x287
	.long	.LFB41
	.long	.LFE41-.LFB41
	.uleb128 0x1
	.byte	0x9c
	.long	0xc3f
	.uleb128 0x15
	.long	.LASF138
	.byte	0x1
	.value	0x287
	.long	0xb2
	.long	.LLST84
	.byte	0
	.uleb128 0x14
	.long	.LASF139
	.byte	0x1
	.value	0x28c
	.long	0xa5
	.long	.LFB42
	.long	.LFE42-.LFB42
	.uleb128 0x1
	.byte	0x9c
	.long	0xc7a
	.uleb128 0x15
	.long	.LASF85
	.byte	0x1
	.value	0x28c
	.long	0x81
	.long	.LLST85
	.uleb128 0x15
	.long	.LASF140
	.byte	0x1
	.value	0x28c
	.long	0x81
	.long	.LLST86
	.byte	0
	.uleb128 0x17
	.long	.LASF141
	.byte	0x1
	.value	0x292
	.long	.LFB43
	.long	.LFE43-.LFB43
	.uleb128 0x1
	.byte	0x9c
	.long	0xcc1
	.uleb128 0x15
	.long	.LASF142
	.byte	0x1
	.value	0x292
	.long	0xa5
	.long	.LLST87
	.uleb128 0x15
	.long	.LASF140
	.byte	0x1
	.value	0x292
	.long	0x81
	.long	.LLST88
	.uleb128 0x15
	.long	.LASF143
	.byte	0x1
	.value	0x292
	.long	0x81
	.long	.LLST89
	.byte	0
	.uleb128 0x17
	.long	.LASF144
	.byte	0x1
	.value	0x299
	.long	.LFB44
	.long	.LFE44-.LFB44
	.uleb128 0x1
	.byte	0x9c
	.long	0xce8
	.uleb128 0x15
	.long	.LASF145
	.byte	0x1
	.value	0x299
	.long	0x50
	.long	.LLST90
	.byte	0
	.uleb128 0x17
	.long	.LASF146
	.byte	0x1
	.value	0x29f
	.long	.LFB45
	.long	.LFE45-.LFB45
	.uleb128 0x1
	.byte	0x9c
	.long	0xd1d
	.uleb128 0x19
	.string	"ch"
	.byte	0x1
	.value	0x29f
	.long	0x61
	.uleb128 0x2
	.byte	0x91
	.sleb128 0
	.uleb128 0x1a
	.long	.LASF161
	.byte	0x1
	.value	0x2a1
	.long	0x81
	.long	.LLST91
	.byte	0
	.uleb128 0x14
	.long	.LASF147
	.byte	0x1
	.value	0x2a6
	.long	0x9a
	.long	.LFB46
	.long	.LFE46-.LFB46
	.uleb128 0x1
	.byte	0x9c
	.long	0xd68
	.uleb128 0x15
	.long	.LASF148
	.byte	0x1
	.value	0x2a6
	.long	0x81
	.long	.LLST92
	.uleb128 0x15
	.long	.LASF149
	.byte	0x1
	.value	0x2a6
	.long	0x81
	.long	.LLST93
	.uleb128 0x1b
	.string	"sde"
	.byte	0x1
	.value	0x2a7
	.long	0x23d
	.long	.LLST94
	.byte	0
	.uleb128 0x14
	.long	.LASF150
	.byte	0x1
	.value	0x2af
	.long	0x9a
	.long	.LFB47
	.long	.LFE47-.LFB47
	.uleb128 0x1
	.byte	0x9c
	.long	0xd93
	.uleb128 0x15
	.long	.LASF151
	.byte	0x1
	.value	0x2af
	.long	0x1ea
	.long	.LLST95
	.byte	0
	.uleb128 0x14
	.long	.LASF152
	.byte	0x1
	.value	0x2b5
	.long	0x9a
	.long	.LFB48
	.long	.LFE48-.LFB48
	.uleb128 0x1
	.byte	0x9c
	.long	0xdbd
	.uleb128 0x1b
	.string	"de"
	.byte	0x1
	.value	0x2b5
	.long	0x249
	.long	.LLST96
	.byte	0
	.uleb128 0x18
	.long	.LASF153
	.byte	0x1
	.value	0x2bb
	.long	0xb2
	.long	.LFB49
	.long	.LFE49-.LFB49
	.uleb128 0x1
	.byte	0x9c
	.uleb128 0x18
	.long	.LASF154
	.byte	0x1
	.value	0x2c0
	.long	0x9a
	.long	.LFB50
	.long	.LFE50-.LFB50
	.uleb128 0x1
	.byte	0x9c
	.uleb128 0x1c
	.long	.LASF162
	.byte	0x1
	.value	0x2c5
	.long	.LFB51
	.long	.LFE51-.LFB51
	.uleb128 0x1
	.byte	0x9c
	.uleb128 0x1d
	.long	.LASF155
	.byte	0x1
	.value	0x2ca
	.long	0xb2
	.long	.LFB52
	.long	.LFE52-.LFB52
	.uleb128 0x1
	.byte	0x9c
	.uleb128 0x15
	.long	.LASF156
	.byte	0x1
	.value	0x2ca
	.long	0xb2
	.long	.LLST97
	.byte	0
	.byte	0
	.section	.debug_abbrev,"",@progbits
.Ldebug_abbrev0:
	.uleb128 0x1
	.uleb128 0x11
	.byte	0x1
	.uleb128 0x25
	.uleb128 0xe
	.uleb128 0x13
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x1b
	.uleb128 0xe
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x6
	.uleb128 0x10
	.uleb128 0x17
	.byte	0
	.byte	0
	.uleb128 0x2
	.uleb128 0x24
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3e
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0xe
	.byte	0
	.byte	0
	.uleb128 0x3
	.uleb128 0x16
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x4
	.uleb128 0xf
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x5
	.uleb128 0x24
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3e
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0x8
	.byte	0
	.byte	0
	.uleb128 0x6
	.uleb128 0xf
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.byte	0
	.byte	0
	.uleb128 0x7
	.uleb128 0x13
	.byte	0x1
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x8
	.uleb128 0xd
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x38
	.uleb128 0xb
	.byte	0
	.byte	0
	.uleb128 0x9
	.uleb128 0x16
	.byte	0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0xa
	.uleb128 0x13
	.byte	0x1
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0xb
	.uleb128 0xd
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x38
	.uleb128 0xb
	.byte	0
	.byte	0
	.uleb128 0xc
	.uleb128 0x16
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0xd
	.uleb128 0x13
	.byte	0x1
	.uleb128 0xb
	.uleb128 0x5
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0xe
	.uleb128 0xd
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x38
	.uleb128 0x5
	.byte	0
	.byte	0
	.uleb128 0xf
	.uleb128 0x1
	.byte	0x1
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x10
	.uleb128 0x21
	.byte	0
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2f
	.uleb128 0x5
	.byte	0
	.byte	0
	.uleb128 0x11
	.uleb128 0x21
	.byte	0
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2f
	.uleb128 0xb
	.byte	0
	.byte	0
	.uleb128 0x12
	.uleb128 0x15
	.byte	0x1
	.uleb128 0x27
	.uleb128 0x19
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x13
	.uleb128 0x5
	.byte	0
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x14
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3f
	.uleb128 0x19
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x27
	.uleb128 0x19
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x6
	.uleb128 0x40
	.uleb128 0x18
	.uleb128 0x2117
	.uleb128 0x19
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x15
	.uleb128 0x5
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x17
	.byte	0
	.byte	0
	.uleb128 0x16
	.uleb128 0x5
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x18
	.byte	0
	.byte	0
	.uleb128 0x17
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3f
	.uleb128 0x19
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x27
	.uleb128 0x19
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x6
	.uleb128 0x40
	.uleb128 0x18
	.uleb128 0x2117
	.uleb128 0x19
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x18
	.uleb128 0x2e
	.byte	0
	.uleb128 0x3f
	.uleb128 0x19
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x27
	.uleb128 0x19
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x6
	.uleb128 0x40
	.uleb128 0x18
	.uleb128 0x2117
	.uleb128 0x19
	.byte	0
	.byte	0
	.uleb128 0x19
	.uleb128 0x5
	.byte	0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x18
	.byte	0
	.byte	0
	.uleb128 0x1a
	.uleb128 0x34
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x17
	.byte	0
	.byte	0
	.uleb128 0x1b
	.uleb128 0x5
	.byte	0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x17
	.byte	0
	.byte	0
	.uleb128 0x1c
	.uleb128 0x2e
	.byte	0
	.uleb128 0x3f
	.uleb128 0x19
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x27
	.uleb128 0x19
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x6
	.uleb128 0x40
	.uleb128 0x18
	.uleb128 0x2117
	.uleb128 0x19
	.byte	0
	.byte	0
	.uleb128 0x1d
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3f
	.uleb128 0x19
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x27
	.uleb128 0x19
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x6
	.uleb128 0x40
	.uleb128 0x18
	.uleb128 0x2117
	.uleb128 0x19
	.byte	0
	.byte	0
	.byte	0
	.section	.debug_loc,"",@progbits
.Ldebug_loc0:
.LLST0:
	.long	.LVL0-.Ltext0
	.long	.LVL1-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 0
	.long	0
	.long	0
.LLST1:
	.long	.LVL0-.Ltext0
	.long	.LVL1-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 4
	.long	0
	.long	0
.LLST2:
	.long	.LVL0-.Ltext0
	.long	.LVL1-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 8
	.long	0
	.long	0
.LLST3:
	.long	.LVL0-.Ltext0
	.long	.LVL1-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 12
	.long	0
	.long	0
.LLST4:
	.long	.LVL0-.Ltext0
	.long	.LVL1-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 16
	.long	0
	.long	0
.LLST5:
	.long	.LVL0-.Ltext0
	.long	.LVL1-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 20
	.long	0
	.long	0
.LLST6:
	.long	.LVL2-.Ltext0
	.long	.LVL3-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 0
	.long	0
	.long	0
.LLST7:
	.long	.LVL4-.Ltext0
	.long	.LVL5-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 0
	.long	0
	.long	0
.LLST8:
	.long	.LVL6-.Ltext0
	.long	.LVL7-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 0
	.long	0
	.long	0
.LLST9:
	.long	.LVL8-.Ltext0
	.long	.LVL9-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 0
	.long	0
	.long	0
.LLST10:
	.long	.LVL8-.Ltext0
	.long	.LVL9-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 4
	.long	0
	.long	0
.LLST11:
	.long	.LVL10-.Ltext0
	.long	.LVL11-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 0
	.long	0
	.long	0
.LLST12:
	.long	.LVL12-.Ltext0
	.long	.LVL13-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 0
	.long	0
	.long	0
.LLST13:
	.long	.LVL12-.Ltext0
	.long	.LVL13-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 4
	.long	0
	.long	0
.LLST14:
	.long	.LVL14-.Ltext0
	.long	.LVL15-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 0
	.long	0
	.long	0
.LLST15:
	.long	.LVL16-.Ltext0
	.long	.LVL17-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 0
	.long	0
	.long	0
.LLST16:
	.long	.LVL16-.Ltext0
	.long	.LVL17-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 4
	.long	0
	.long	0
.LLST17:
	.long	.LVL16-.Ltext0
	.long	.LVL17-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 8
	.long	0
	.long	0
.LLST18:
	.long	.LVL16-.Ltext0
	.long	.LVL17-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 12
	.long	0
	.long	0
.LLST19:
	.long	.LVL16-.Ltext0
	.long	.LVL17-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 16
	.long	0
	.long	0
.LLST20:
	.long	.LVL18-.Ltext0
	.long	.LVL19-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 0
	.long	0
	.long	0
.LLST21:
	.long	.LVL20-.Ltext0
	.long	.LVL21-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 0
	.long	0
	.long	0
.LLST22:
	.long	.LVL22-.Ltext0
	.long	.LVL23-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 0
	.long	0
	.long	0
.LLST23:
	.long	.LVL24-.Ltext0
	.long	.LVL25-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 0
	.long	0
	.long	0
.LLST24:
	.long	.LVL26-.Ltext0
	.long	.LVL27-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 0
	.long	0
	.long	0
.LLST25:
	.long	.LVL28-.Ltext0
	.long	.LVL29-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 0
	.long	0
	.long	0
.LLST26:
	.long	.LVL30-.Ltext0
	.long	.LVL31-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 0
	.long	0
	.long	0
.LLST27:
	.long	.LVL32-.Ltext0
	.long	.LVL33-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 0
	.long	0
	.long	0
.LLST28:
	.long	.LVL34-.Ltext0
	.long	.LVL35-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 0
	.long	0
	.long	0
.LLST29:
	.long	.LVL34-.Ltext0
	.long	.LVL35-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 4
	.long	0
	.long	0
.LLST30:
	.long	.LVL36-.Ltext0
	.long	.LVL37-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 0
	.long	0
	.long	0
.LLST31:
	.long	.LVL36-.Ltext0
	.long	.LVL37-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 4
	.long	0
	.long	0
.LLST32:
	.long	.LVL36-.Ltext0
	.long	.LVL37-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 8
	.long	0
	.long	0
.LLST33:
	.long	.LVL38-.Ltext0
	.long	.LVL39-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 0
	.long	0
	.long	0
.LLST34:
	.long	.LVL40-.Ltext0
	.long	.LVL41-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 0
	.long	0
	.long	0
.LLST35:
	.long	.LVL40-.Ltext0
	.long	.LVL41-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 4
	.long	0
	.long	0
.LLST36:
	.long	.LVL40-.Ltext0
	.long	.LVL41-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 8
	.long	0
	.long	0
.LLST37:
	.long	.LVL40-.Ltext0
	.long	.LVL41-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 12
	.long	0
	.long	0
.LLST38:
	.long	.LVL40-.Ltext0
	.long	.LVL41-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 16
	.long	0
	.long	0
.LLST39:
	.long	.LVL42-.Ltext0
	.long	.LVL43-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 0
	.long	0
	.long	0
.LLST40:
	.long	.LVL44-.Ltext0
	.long	.LVL45-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 0
	.long	0
	.long	0
.LLST41:
	.long	.LVL44-.Ltext0
	.long	.LVL45-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 4
	.long	0
	.long	0
.LLST42:
	.long	.LVL44-.Ltext0
	.long	.LVL45-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 8
	.long	0
	.long	0
.LLST43:
	.long	.LVL44-.Ltext0
	.long	.LVL45-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 12
	.long	0
	.long	0
.LLST44:
	.long	.LVL46-.Ltext0
	.long	.LVL47-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 0
	.long	0
	.long	0
.LLST45:
	.long	.LVL46-.Ltext0
	.long	.LVL47-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 4
	.long	0
	.long	0
.LLST46:
	.long	.LVL46-.Ltext0
	.long	.LVL47-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 8
	.long	0
	.long	0
.LLST47:
	.long	.LVL46-.Ltext0
	.long	.LVL47-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 12
	.long	0
	.long	0
.LLST48:
	.long	.LVL48-.Ltext0
	.long	.LVL49-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 0
	.long	0
	.long	0
.LLST49:
	.long	.LVL48-.Ltext0
	.long	.LVL49-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 4
	.long	0
	.long	0
.LLST50:
	.long	.LVL48-.Ltext0
	.long	.LVL49-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 8
	.long	0
	.long	0
.LLST51:
	.long	.LVL48-.Ltext0
	.long	.LVL49-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 12
	.long	0
	.long	0
.LLST52:
	.long	.LVL50-.Ltext0
	.long	.LVL51-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 0
	.long	0
	.long	0
.LLST53:
	.long	.LVL52-.Ltext0
	.long	.LVL53-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 0
	.long	0
	.long	0
.LLST54:
	.long	.LVL54-.Ltext0
	.long	.LVL55-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 0
	.long	0
	.long	0
.LLST55:
	.long	.LVL56-.Ltext0
	.long	.LVL57-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 0
	.long	0
	.long	0
.LLST56:
	.long	.LVL56-.Ltext0
	.long	.LVL57-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 4
	.long	0
	.long	0
.LLST57:
	.long	.LVL58-.Ltext0
	.long	.LVL59-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 0
	.long	0
	.long	0
.LLST58:
	.long	.LVL58-.Ltext0
	.long	.LVL59-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 4
	.long	0
	.long	0
.LLST59:
	.long	.LVL58-.Ltext0
	.long	.LVL59-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 8
	.long	0
	.long	0
.LLST60:
	.long	.LVL60-.Ltext0
	.long	.LVL61-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 0
	.long	0
	.long	0
.LLST61:
	.long	.LVL60-.Ltext0
	.long	.LVL61-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 4
	.long	0
	.long	0
.LLST62:
	.long	.LVL62-.Ltext0
	.long	.LVL63-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 0
	.long	0
	.long	0
.LLST63:
	.long	.LVL64-.Ltext0
	.long	.LVL65-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 0
	.long	0
	.long	0
.LLST64:
	.long	.LVL64-.Ltext0
	.long	.LVL65-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 4
	.long	0
	.long	0
.LLST65:
	.long	.LVL66-.Ltext0
	.long	.LVL67-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 0
	.long	0
	.long	0
.LLST66:
	.long	.LVL68-.Ltext0
	.long	.LVL69-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 0
	.long	0
	.long	0
.LLST67:
	.long	.LVL70-.Ltext0
	.long	.LVL71-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 0
	.long	0
	.long	0
.LLST68:
	.long	.LVL70-.Ltext0
	.long	.LVL71-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 4
	.long	0
	.long	0
.LLST69:
	.long	.LVL70-.Ltext0
	.long	.LVL71-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 8
	.long	0
	.long	0
.LLST70:
	.long	.LVL70-.Ltext0
	.long	.LVL71-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 12
	.long	0
	.long	0
.LLST71:
	.long	.LVL70-.Ltext0
	.long	.LVL71-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 16
	.long	0
	.long	0
.LLST72:
	.long	.LVL70-.Ltext0
	.long	.LVL71-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 20
	.long	0
	.long	0
.LLST73:
	.long	.LVL72-.Ltext0
	.long	.LVL73-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 0
	.long	0
	.long	0
.LLST74:
	.long	.LVL72-.Ltext0
	.long	.LVL73-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 4
	.long	0
	.long	0
.LLST75:
	.long	.LVL72-.Ltext0
	.long	.LVL73-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 8
	.long	0
	.long	0
.LLST76:
	.long	.LVL72-.Ltext0
	.long	.LVL73-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 12
	.long	0
	.long	0
.LLST77:
	.long	.LVL74-.Ltext0
	.long	.LVL75-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 0
	.long	0
	.long	0
.LLST78:
	.long	.LVL76-.Ltext0
	.long	.LVL77-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 0
	.long	0
	.long	0
.LLST79:
	.long	.LVL76-.Ltext0
	.long	.LVL77-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 4
	.long	0
	.long	0
.LLST80:
	.long	.LVL76-.Ltext0
	.long	.LVL77-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 8
	.long	0
	.long	0
.LLST81:
	.long	.LVL76-.Ltext0
	.long	.LVL77-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 12
	.long	0
	.long	0
.LLST82:
	.long	.LVL76-.Ltext0
	.long	.LVL77-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 16
	.long	0
	.long	0
.LLST83:
	.long	.LVL76-.Ltext0
	.long	.LVL77-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 20
	.long	0
	.long	0
.LLST84:
	.long	.LVL78-.Ltext0
	.long	.LVL79-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 0
	.long	0
	.long	0
.LLST85:
	.long	.LVL80-.Ltext0
	.long	.LVL81-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 0
	.long	0
	.long	0
.LLST86:
	.long	.LVL80-.Ltext0
	.long	.LVL81-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 4
	.long	0
	.long	0
.LLST87:
	.long	.LVL82-.Ltext0
	.long	.LVL83-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 0
	.long	0
	.long	0
.LLST88:
	.long	.LVL82-.Ltext0
	.long	.LVL83-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 4
	.long	0
	.long	0
.LLST89:
	.long	.LVL82-.Ltext0
	.long	.LVL83-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 8
	.long	0
	.long	0
.LLST90:
	.long	.LVL84-.Ltext0
	.long	.LVL85-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 0
	.long	0
	.long	0
.LLST91:
	.long	.LVL87-.Ltext0
	.long	.LVL88-.Ltext0
	.value	0x1
	.byte	0x52
	.long	0
	.long	0
.LLST92:
	.long	.LVL89-.Ltext0
	.long	.LVL90-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 0
	.long	0
	.long	0
.LLST93:
	.long	.LVL89-.Ltext0
	.long	.LVL90-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 4
	.long	0
	.long	0
.LLST94:
	.long	.LVL89-.Ltext0
	.long	.LVL90-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 8
	.long	0
	.long	0
.LLST95:
	.long	.LVL91-.Ltext0
	.long	.LVL92-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 0
	.long	0
	.long	0
.LLST96:
	.long	.LVL93-.Ltext0
	.long	.LVL94-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 0
	.long	0
	.long	0
.LLST97:
	.long	.LVL95-.Ltext0
	.long	.LVL96-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 0
	.long	0
	.long	0
	.section	.debug_aranges,"",@progbits
	.long	0x1c
	.value	0x2
	.long	.Ldebug_info0
	.byte	0x4
	.byte	0
	.value	0
	.value	0
	.long	.Ltext0
	.long	.Letext0-.Ltext0
	.long	0
	.long	0
	.section	.debug_line,"",@progbits
.Ldebug_line0:
	.section	.debug_str,"MS",@progbits,1
.LASF71:
	.string	"CreateMutex"
.LASF80:
	.string	"ucVector"
.LASF144:
	.string	"PrintLine"
.LASF67:
	.string	"hEvent"
.LASF12:
	.string	"BOOL"
.LASF35:
	.string	"__INTERRUPT_HANDLER"
.LASF50:
	.string	"SetThreadPriority"
.LASF23:
	.string	"ftLastAccessTime"
.LASF104:
	.string	"CreateDirectory"
.LASF115:
	.string	"RemoveDirectory"
.LASF79:
	.string	"lpInterruptHandler"
.LASF55:
	.string	"dwMillionSecond"
.LASF103:
	.string	"CloseFile"
.LASF141:
	.string	"KMemFree"
.LASF63:
	.string	"hTimer"
.LASF30:
	.string	"cAlternateFileName"
.LASF4:
	.string	"short int"
.LASF134:
	.string	"dwMaxWriteSize"
.LASF83:
	.string	"VirtualAlloc"
.LASF127:
	.string	"dwMoveFlags"
.LASF31:
	.string	"sizetype"
.LASF68:
	.string	"SetEvent"
.LASF156:
	.string	"hNewThread"
.LASF26:
	.string	"nFileSizeLow"
.LASF51:
	.string	"GetMessage"
.LASF72:
	.string	"DestroyMutex"
.LASF151:
	.string	"shell"
.LASF77:
	.string	"WaitForThisObjectEx"
.LASF158:
	.string	"kapi.c"
.LASF88:
	.string	"lpszRegName"
.LASF44:
	.string	"lpszName"
.LASF15:
	.string	"wCommand"
.LASF46:
	.string	"CreateKernelThread"
.LASF157:
	.string	"GNU C 4.9.2 20150212 (Red Hat 4.9.2-6) -m32 -mtune=generic -march=i686 -g -O2"
.LASF154:
	.string	"SwitchToGraphic"
.LASF113:
	.string	"GetFileSize"
.LASF114:
	.string	"lpdwSizeHigh"
.LASF90:
	.string	"lpVirtualAddr"
.LASF125:
	.string	"lpdwDistLow"
.LASF6:
	.string	"WORD"
.LASF69:
	.string	"ResetEvent"
.LASF65:
	.string	"bInitialStatus"
.LASF41:
	.string	"lpStartRoutine"
.LASF146:
	.string	"PrintChar"
.LASF123:
	.string	"lpdwFilled"
.LASF25:
	.string	"nFileSizeHigh"
.LASF13:
	.string	"LPVOID"
.LASF96:
	.string	"hFile"
.LASF149:
	.string	"dwEndSyscallNum"
.LASF49:
	.string	"GetThreadID"
.LASF19:
	.string	"dwLowDateTime"
.LASF98:
	.string	"lpBuffer"
.LASF58:
	.string	"lpHandler"
.LASF108:
	.string	"pFindData"
.LASF140:
	.string	"dwSizeType"
.LASF150:
	.string	"ReplaceShell"
.LASF137:
	.string	"DestroyDevice"
.LASF16:
	.string	"wParam"
.LASF142:
	.string	"lpMemAddr"
.LASF61:
	.string	"DestroyKernelThread"
.LASF70:
	.string	"GetLastError"
.LASF29:
	.string	"cFileName"
.LASF11:
	.string	"long int"
.LASF147:
	.string	"RegisterSystemCall"
.LASF1:
	.string	"CHAR"
.LASF97:
	.string	"dwReadSize"
.LASF91:
	.string	"CreateFile"
.LASF52:
	.string	"lpMsg"
.LASF84:
	.string	"lpDesiredAddr"
.LASF53:
	.string	"SendMessage"
.LASF93:
	.string	"dwAccessMode"
.LASF128:
	.string	"FlushFileBuffers"
.LASF92:
	.string	"lpszFileName"
.LASF37:
	.string	"__DRIVER_ENTRY"
.LASF101:
	.string	"dwWriteSize"
.LASF57:
	.string	"dwTimerID"
.LASF38:
	.string	"dwStackSize"
.LASF75:
	.string	"WaitForThisObject"
.LASF159:
	.string	"/media/gaojie/Dev/hellox/HelloX_OS/kernel/kapi"
.LASF116:
	.string	"SetEndOfFile"
.LASF42:
	.string	"lpRoutineParam"
.LASF135:
	.string	"lpDevExtension"
.LASF132:
	.string	"dwBlockSize"
.LASF2:
	.string	"unsigned char"
.LASF24:
	.string	"ftLastWriteTime"
.LASF109:
	.string	"FindNextFile"
.LASF129:
	.string	"CreateDevice"
.LASF82:
	.string	"hInterrupt"
.LASF34:
	.string	"__DIRECT_TIMER_HANDLER"
.LASF122:
	.string	"lpOutputBuffer"
.LASF87:
	.string	"dwAccessFlags"
.LASF162:
	.string	"SwitchToText"
.LASF8:
	.string	"unsigned int"
.LASF40:
	.string	"dwPriority"
.LASF0:
	.string	"UCHAR"
.LASF145:
	.string	"lpszInfo"
.LASF5:
	.string	"LPSTR"
.LASF126:
	.string	"lpdwDistHigh"
.LASF59:
	.string	"lpHandlerParam"
.LASF7:
	.string	"short unsigned int"
.LASF111:
	.string	"FindClose"
.LASF112:
	.string	"GetFileAttributes"
.LASF3:
	.string	"char"
.LASF136:
	.string	"hDrvObject"
.LASF81:
	.string	"DisconnectInterrupt"
.LASF107:
	.string	"FindFirstFile"
.LASF139:
	.string	"KMemAlloc"
.LASF56:
	.string	"SetTimer"
.LASF14:
	.string	"HANDLE"
.LASF33:
	.string	"__KERNEL_THREAD_ROUTINE"
.LASF85:
	.string	"dwSize"
.LASF120:
	.string	"lpInputBuffer"
.LASF66:
	.string	"DestroyEvent"
.LASF118:
	.string	"dwCommand"
.LASF64:
	.string	"CreateEvent"
.LASF74:
	.string	"ReleaseMutex"
.LASF155:
	.string	"SetFocusThread"
.LASF110:
	.string	"hFindHandle"
.LASF48:
	.string	"dwNewError"
.LASF100:
	.string	"WriteFile"
.LASF161:
	.string	"dwCh"
.LASF10:
	.string	"long unsigned int"
.LASF78:
	.string	"ConnectInterrupt"
.LASF143:
	.string	"dwMemLength"
.LASF47:
	.string	"SetLastError"
.LASF21:
	.string	"dwFileAttribute"
.LASF17:
	.string	"dwParam"
.LASF95:
	.string	"ReadFile"
.LASF106:
	.string	"DeleteFile"
.LASF9:
	.string	"DWORD"
.LASF62:
	.string	"CancelTimer"
.LASF22:
	.string	"ftCreationTime"
.LASF124:
	.string	"SetFilePointer"
.LASF160:
	.string	"_MSG"
.LASF99:
	.string	"lpdwReadSize"
.LASF138:
	.string	"hDevice"
.LASF152:
	.string	"LoadDriver"
.LASF130:
	.string	"lpszDevName"
.LASF148:
	.string	"dwStartSyscallNum"
.LASF36:
	.string	"__SYSCALL_DISPATCH_ENTRY"
.LASF119:
	.string	"dwInputLen"
.LASF54:
	.string	"Sleep"
.LASF153:
	.string	"GetCurrentThread"
.LASF32:
	.string	"FS_FIND_DATA"
.LASF86:
	.string	"dwAllocateFlags"
.LASF27:
	.string	"dwReserved0"
.LASF28:
	.string	"dwReserved1"
.LASF121:
	.string	"dwOutputLen"
.LASF105:
	.string	"lpszDirName"
.LASF131:
	.string	"dwAttributes"
.LASF133:
	.string	"dwMaxReadSize"
.LASF117:
	.string	"IOControl"
.LASF76:
	.string	"hObject"
.LASF39:
	.string	"dwInitStatus"
.LASF73:
	.string	"hMutex"
.LASF18:
	.string	"dwHighDateTime"
.LASF20:
	.string	"__FILE_TIME"
.LASF60:
	.string	"dwTimerFlags"
.LASF89:
	.string	"VirtualFree"
.LASF45:
	.string	"hThread"
.LASF94:
	.string	"dwShareMode"
.LASF43:
	.string	"lpReserved"
.LASF102:
	.string	"lpdwWrittenSize"
	.ident	"GCC: (GNU) 4.9.2 20150212 (Red Hat 4.9.2-6)"
	.section	.note.GNU-stack,"",@progbits
