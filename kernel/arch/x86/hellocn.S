	.file	"hellocn.c"
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
	.globl	SetGeneralIntHandler
	.type	SetGeneralIntHandler, @function
SetGeneralIntHandler:
.LFB11:
	.file 1 "hellocn.c"
	.loc 1 24 0
	.cfi_startproc
.LVL0:
	.loc 1 26 0
	movl	4(%esp), %eax
#APP
# 26 "hellocn.c" 1
	pushl	%ebx	
	pushl	%ecx	
	movl 	$1097680,	%ebx	
	movl 	%eax,	%eax			
	movl (%ebx),			%ecx			
	movl %ecx,			%eax			
	popl	%ecx							
	popl	%ebx							
	
# 0 "" 2
	.loc 1 51 0
#NO_APP
	ret
	.cfi_endproc
.LFE11:
	.size	SetGeneralIntHandler, .-SetGeneralIntHandler
	.section	.text.unlikely
.LCOLDE0:
	.text
.LHOTE0:
	.section	.text.unlikely
.LCOLDB1:
	.text
.LHOTB1:
	.p2align 4,,15
	.globl	WriteByteToPort
	.type	WriteByteToPort, @function
WriteByteToPort:
.LFB12:
	.loc 1 58 0
	.cfi_startproc
.LVL1:
	.loc 1 60 0
#APP
# 60 "hellocn.c" 1
	.code32				
	pushl	%ebp			
	movl	%esp,	%ebp	
	pushl	%edx			
	movb	8(%ebp),	%al
	movw	12(%ebp),	%ax
	outb	%al,	%dx	
	popl	%edx	
	leave			
	ret			
	
# 0 "" 2
#NO_APP
	ret
	.cfi_endproc
.LFE12:
	.size	WriteByteToPort, .-WriteByteToPort
	.section	.text.unlikely
.LCOLDE1:
	.text
.LHOTE1:
	.section	.text.unlikely
.LCOLDB2:
	.text
.LHOTB2:
	.p2align 4,,15
	.globl	ReadByteStringFromPort
	.type	ReadByteStringFromPort, @function
ReadByteStringFromPort:
.LFB13:
	.loc 1 93 0
	.cfi_startproc
.LVL2:
	.loc 1 95 0
#APP
# 95 "hellocn.c" 1
	pushl	%ebp	
	movl	%esp,	%ebp	
	pushl	%ecx	
	pushl	%edx	
	pushl	%edi	
	movl	8(%ebp),	%edi	
	movl	12(%ebp),	%edi	
	movw	16(%ebp),	%dx	
	rep	insb	
	popl	%edi	
	popl	%edx	
	popl	%ecx	
	leave	
	ret	
	
# 0 "" 2
#NO_APP
	ret
	.cfi_endproc
.LFE13:
	.size	ReadByteStringFromPort, .-ReadByteStringFromPort
	.section	.text.unlikely
.LCOLDE2:
	.text
.LHOTE2:
	.section	.text.unlikely
.LCOLDB3:
	.text
.LHOTB3:
	.p2align 4,,15
	.globl	WriteByteStringToPort
	.type	WriteByteStringToPort, @function
WriteByteStringToPort:
.LFB14:
	.loc 1 140 0
	.cfi_startproc
.LVL3:
	.loc 1 143 0
#APP
# 143 "hellocn.c" 1
	pushl	%ebp	
	movl	%esp,	%ebp	
	pushl	%ecx	
	pushl	%edx	
	pushl	%esi	
	movl	8(%ebp),	%esi	
	movl	12(%ebp),	%ecx	
	movw	16(%ebp),	%dx	
	rep	outsb	
	popl	%esi	
	popl	%edx	
	popl	%ecx	
	leave	
	ret	
	
# 0 "" 2
#NO_APP
	ret
	.cfi_endproc
.LFE14:
	.size	WriteByteStringToPort, .-WriteByteStringToPort
	.section	.text.unlikely
.LCOLDE3:
	.text
.LHOTE3:
	.section	.text.unlikely
.LCOLDB4:
	.text
.LHOTB4:
	.p2align 4,,15
	.globl	ReadWordFromPort
	.type	ReadWordFromPort, @function
ReadWordFromPort:
.LFB15:
	.loc 1 185 0
	.cfi_startproc
.LVL4:
	.loc 1 187 0
#APP
# 187 "hellocn.c" 1
	pushl	%ebp	
	movl	%esp,	%ebp	
	pushl	%ebx	
	pushl	%edx	
	movw	0x0c(%ebp),	%dx	
	movl	0x08(%ebp),	%ebx	
	inw	%dx,	%ax	
	movw	%ax,	(%ebx)	
	popl	%edx	
	popl	%ebx	
	leave			
	ret			
	
# 0 "" 2
#NO_APP
	ret
	.cfi_endproc
.LFE15:
	.size	ReadWordFromPort, .-ReadWordFromPort
	.section	.text.unlikely
.LCOLDE4:
	.text
.LHOTE4:
	.section	.text.unlikely
.LCOLDB5:
	.text
.LHOTB5:
	.p2align 4,,15
	.globl	WriteWordToPort
	.type	WriteWordToPort, @function
WriteWordToPort:
.LFB16:
	.loc 1 225 0
	.cfi_startproc
.LVL5:
	.loc 1 227 0
#APP
# 227 "hellocn.c" 1
	pushl	%ebp           
	movl %esp,	%ebp   
	pushw %dx             
	movw 0x0c(%ebp),	%dx
	movw 0x08(%ebp),	%ax
	outw %ax, %dx        
	popw %dx              
	leave                  
	ret					
	
# 0 "" 2
#NO_APP
	ret
	.cfi_endproc
.LFE16:
	.size	WriteWordToPort, .-WriteWordToPort
	.section	.text.unlikely
.LCOLDE5:
	.text
.LHOTE5:
	.section	.text.unlikely
.LCOLDB6:
	.text
.LHOTB6:
	.p2align 4,,15
	.globl	ReadWordStringFromPort
	.type	ReadWordStringFromPort, @function
ReadWordStringFromPort:
.LFB17:
	.loc 1 259 0
	.cfi_startproc
.LVL6:
	.loc 1 261 0
#APP
# 261 "hellocn.c" 1
	pushl %ebp         
	                 movl %esp, %ebp   
	                 pushl %ecx         
	                 pushl %edx         
	                 pushl %edi         
	                 movl 0x08(%ebp),	%edi				
	movl 0x0c(%ebp),	%ecx				
	shrl $0x01,		%ecx             	
	movw 0x10(%ebp),	%dx				
	cld                               		
	rep insw               	           	
	popl %edi                 	        
	  popl %edx                     	    
	  popl %ecx                         	
	  leave		
	ret		
	
# 0 "" 2
#NO_APP
	ret
	.cfi_endproc
.LFE17:
	.size	ReadWordStringFromPort, .-ReadWordStringFromPort
	.section	.text.unlikely
.LCOLDE6:
	.text
.LHOTE6:
	.section	.text.unlikely
.LCOLDB7:
	.text
.LHOTB7:
	.p2align 4,,15
	.globl	WriteWordStringToPort
	.type	WriteWordStringToPort, @function
WriteWordStringToPort:
.LFB18:
	.loc 1 307 0
	.cfi_startproc
.LVL7:
	.loc 1 309 0
#APP
# 309 "hellocn.c" 1
	pushl %ebp				
	movl %esp, %ebp          
	pushl %ecx                
	pushl %edx                
	pushl %esi                
	movl 0x0c(%ebp),	%esi   
	movl 0x0c(%ebp),	%ecx   
	shrl $0x02,		%ecx   
	movw 0x10(%ebp),	%dx    
	rep outsw                  
	popl %esi                 
	popl %edx                 
	popl %ecx                 
	leave                      
	ret						
	
# 0 "" 2
#NO_APP
	ret
	.cfi_endproc
.LFE18:
	.size	WriteWordStringToPort, .-WriteWordStringToPort
	.section	.text.unlikely
.LCOLDE7:
	.text
.LHOTE7:
	.section	.rodata.str1.1,"aMS",@progbits,1
.LC8:
	.string	"Error Level : FATAL(1)"
.LC9:
	.string	"Error Level : CRITICAL(2)"
.LC10:
	.string	"Error Level : IMPORTANT(3)"
.LC11:
	.string	"Error Level : ALARM(4)"
.LC12:
	.string	"Error Level : INFORM(5)"
	.section	.text.unlikely
.LCOLDB13:
	.text
.LHOTB13:
	.p2align 4,,15
	.globl	ErrorHandler
	.type	ErrorHandler, @function
ErrorHandler:
.LFB24:
	.loc 1 394 0
	.cfi_startproc
.LVL8:
	pushl	%ebx
	.cfi_def_cfa_offset 8
	.cfi_offset 3, -8
	subl	$8, %esp
	.cfi_def_cfa_offset 16
	.loc 1 394 0
	movl	16(%esp), %eax
	movl	24(%esp), %ebx
	.loc 1 395 0
	cmpl	$16, %eax
	ja	.L9
	jmp	*.L12(,%eax,4)
	.section	.rodata
	.align 4
	.align 4
.L12:
	.long	.L9
	.long	.L11
	.long	.L13
	.long	.L9
	.long	.L14
	.long	.L9
	.long	.L9
	.long	.L9
	.long	.L15
	.long	.L9
	.long	.L9
	.long	.L9
	.long	.L9
	.long	.L9
	.long	.L9
	.long	.L9
	.long	.L16
	.text
	.p2align 4,,10
	.p2align 3
.L11:
.LVL9:
.LBB12:
.LBB13:
	.loc 1 355 0
	subl	$12, %esp
	.cfi_def_cfa_offset 28
	pushl	$.LC8
	.cfi_def_cfa_offset 32
	call	PrintLine
.LVL10:
	.loc 1 356 0
	addl	$16, %esp
	.cfi_def_cfa_offset 16
	testl	%ebx, %ebx
	jne	.L17
.LVL11:
	.p2align 4,,10
	.p2align 3
.L9:
.LBE13:
.LBE12:
	.loc 1 415 0
	addl	$8, %esp
	.cfi_remember_state
	.cfi_def_cfa_offset 8
	popl	%ebx
	.cfi_restore 3
	.cfi_def_cfa_offset 4
	ret
	.p2align 4,,10
	.p2align 3
.L16:
	.cfi_restore_state
.LVL12:
.LBB22:
.LBB23:
	.loc 1 387 0
	subl	$12, %esp
	.cfi_def_cfa_offset 28
	pushl	$.LC12
	.cfi_def_cfa_offset 32
	call	PrintLine
.LVL13:
	.loc 1 388 0
	addl	$16, %esp
	.cfi_def_cfa_offset 16
	testl	%ebx, %ebx
	je	.L9
.LVL14:
.L17:
.LBE23:
.LBE22:
.LBB24:
.LBB14:
	.loc 1 357 0
	movl	%ebx, 16(%esp)
.LBE14:
.LBE24:
	.loc 1 415 0
	addl	$8, %esp
	.cfi_remember_state
	.cfi_def_cfa_offset 8
	popl	%ebx
	.cfi_restore 3
	.cfi_def_cfa_offset 4
.LBB25:
.LBB15:
	.loc 1 357 0
	jmp	PrintLine
.LVL15:
	.p2align 4,,10
	.p2align 3
.L13:
	.cfi_restore_state
.LBE15:
.LBE25:
.LBB26:
.LBB27:
	.loc 1 363 0
	subl	$12, %esp
	.cfi_def_cfa_offset 28
	pushl	$.LC9
	.cfi_def_cfa_offset 32
	call	PrintLine
.LVL16:
	.loc 1 364 0
	addl	$16, %esp
	.cfi_def_cfa_offset 16
	testl	%ebx, %ebx
	je	.L9
.LBE27:
.LBE26:
.LBB28:
.LBB16:
	.loc 1 357 0
	movl	%ebx, 16(%esp)
.LBE16:
.LBE28:
	.loc 1 415 0
	addl	$8, %esp
	.cfi_remember_state
	.cfi_def_cfa_offset 8
	popl	%ebx
	.cfi_restore 3
	.cfi_def_cfa_offset 4
.LVL17:
.LBB29:
.LBB17:
	.loc 1 357 0
	jmp	PrintLine
.LVL18:
	.p2align 4,,10
	.p2align 3
.L14:
	.cfi_restore_state
.LBE17:
.LBE29:
.LBB30:
.LBB31:
	.loc 1 371 0
	subl	$12, %esp
	.cfi_def_cfa_offset 28
	pushl	$.LC10
	.cfi_def_cfa_offset 32
	call	PrintLine
.LVL19:
	.loc 1 372 0
	addl	$16, %esp
	.cfi_def_cfa_offset 16
	testl	%ebx, %ebx
	je	.L9
.LBE31:
.LBE30:
.LBB32:
.LBB18:
	.loc 1 357 0
	movl	%ebx, 16(%esp)
.LBE18:
.LBE32:
	.loc 1 415 0
	addl	$8, %esp
	.cfi_remember_state
	.cfi_def_cfa_offset 8
	popl	%ebx
	.cfi_restore 3
	.cfi_def_cfa_offset 4
.LVL20:
.LBB33:
.LBB19:
	.loc 1 357 0
	jmp	PrintLine
.LVL21:
	.p2align 4,,10
	.p2align 3
.L15:
	.cfi_restore_state
.LBE19:
.LBE33:
.LBB34:
.LBB35:
	.loc 1 379 0
	subl	$12, %esp
	.cfi_def_cfa_offset 28
	pushl	$.LC11
	.cfi_def_cfa_offset 32
	call	PrintLine
.LVL22:
	.loc 1 380 0
	addl	$16, %esp
	.cfi_def_cfa_offset 16
	testl	%ebx, %ebx
	je	.L9
.LBE35:
.LBE34:
.LBB36:
.LBB20:
	.loc 1 357 0
	movl	%ebx, 16(%esp)
.LBE20:
.LBE36:
	.loc 1 415 0
	addl	$8, %esp
	.cfi_def_cfa_offset 8
	popl	%ebx
	.cfi_restore 3
	.cfi_def_cfa_offset 4
.LVL23:
.LBB37:
.LBB21:
	.loc 1 357 0
	jmp	PrintLine
.LVL24:
.LBE21:
.LBE37:
	.cfi_endproc
.LFE24:
	.size	ErrorHandler, .-ErrorHandler
	.section	.text.unlikely
.LCOLDE13:
	.text
.LHOTE13:
	.section	.rodata.str1.4,"aMS",@progbits,1
	.align 4
.LC14:
	.string	"\r\nBUG oencountered.\r\nFile name: %s\r\nCode Lines:%d"
	.section	.text.unlikely
.LCOLDB15:
	.text
.LHOTB15:
	.p2align 4,,15
	.globl	__BUG
	.type	__BUG, @function
__BUG:
.LFB25:
	.loc 1 421 0
	.cfi_startproc
.LVL25:
	subl	$16, %esp
	.cfi_def_cfa_offset 20
	.loc 1 425 0
	pushl	24(%esp)
	.cfi_def_cfa_offset 24
	pushl	24(%esp)
	.cfi_def_cfa_offset 28
	pushl	$.LC14
	.cfi_def_cfa_offset 32
	call	_hx_printf
.LVL26:
	.loc 1 428 0
#APP
# 428 "hellocn.c" 1
	pushl %eax		
	pushf				
	popl %eax			
	movl %eax, %eax		
	popl %eax			
	cli				
	
# 0 "" 2
.LVL27:
#NO_APP
	addl	$16, %esp
	.cfi_def_cfa_offset 16
.L25:
	.loc 1 429 0 discriminator 1
	jmp	.L25
	.cfi_endproc
.LFE25:
	.size	__BUG, .-__BUG
	.section	.text.unlikely
.LCOLDE15:
	.text
.LHOTE15:
.Letext0:
	.section	.text.unlikely
.Letext_cold0:
	.file 2 "../../../kernel/include/types.h"
	.file 3 "../../../kernel/include/hellocn.h"
	.file 4 "/usr/include/bits/types.h"
	.file 5 "/usr/include/libio.h"
	.file 6 "/usr/include/stdio.h"
	.file 7 "../../../kernel/include/../lib/string.h"
	.section	.debug_info,"",@progbits
.Ldebug_info0:
	.long	0x766
	.value	0x4
	.long	.Ldebug_abbrev0
	.byte	0x4
	.uleb128 0x1
	.long	.LASF85
	.byte	0x1
	.long	.LASF86
	.long	.LASF87
	.long	.Ltext0
	.long	.Letext0-.Ltext0
	.long	.Ldebug_line0
	.uleb128 0x2
	.byte	0x1
	.byte	0x8
	.long	.LASF0
	.uleb128 0x3
	.long	.LASF2
	.byte	0x2
	.byte	0x21
	.long	0x37
	.uleb128 0x2
	.byte	0x2
	.byte	0x7
	.long	.LASF1
	.uleb128 0x3
	.long	.LASF3
	.byte	0x2
	.byte	0x22
	.long	0x49
	.uleb128 0x2
	.byte	0x4
	.byte	0x7
	.long	.LASF4
	.uleb128 0x2
	.byte	0x1
	.byte	0x6
	.long	.LASF5
	.uleb128 0x2
	.byte	0x2
	.byte	0x5
	.long	.LASF6
	.uleb128 0x4
	.byte	0x4
	.byte	0x5
	.string	"int"
	.uleb128 0x2
	.byte	0x4
	.byte	0x7
	.long	.LASF7
	.uleb128 0x2
	.byte	0x8
	.byte	0x4
	.long	.LASF8
	.uleb128 0x2
	.byte	0x4
	.byte	0x4
	.long	.LASF9
	.uleb128 0x3
	.long	.LASF10
	.byte	0x2
	.byte	0x34
	.long	0x85
	.uleb128 0x5
	.byte	0x4
	.long	0x50
	.uleb128 0x3
	.long	.LASF11
	.byte	0x2
	.byte	0x39
	.long	0x96
	.uleb128 0x6
	.byte	0x4
	.uleb128 0x5
	.byte	0x4
	.long	0x9e
	.uleb128 0x7
	.long	0xa9
	.uleb128 0x8
	.long	0x3e
	.byte	0
	.uleb128 0x3
	.long	.LASF12
	.byte	0x3
	.byte	0x56
	.long	0x98
	.uleb128 0x3
	.long	.LASF13
	.byte	0x3
	.byte	0xd4
	.long	0xbf
	.uleb128 0x5
	.byte	0x4
	.long	0xc5
	.uleb128 0x7
	.long	0xd5
	.uleb128 0x8
	.long	0x3e
	.uleb128 0x8
	.long	0x8b
	.byte	0
	.uleb128 0x2
	.byte	0x4
	.byte	0x7
	.long	.LASF14
	.uleb128 0x2
	.byte	0x1
	.byte	0x6
	.long	.LASF15
	.uleb128 0x2
	.byte	0x8
	.byte	0x5
	.long	.LASF16
	.uleb128 0x2
	.byte	0x8
	.byte	0x7
	.long	.LASF17
	.uleb128 0x3
	.long	.LASF18
	.byte	0x4
	.byte	0x37
	.long	0xe3
	.uleb128 0x3
	.long	.LASF19
	.byte	0x4
	.byte	0x83
	.long	0x107
	.uleb128 0x2
	.byte	0x4
	.byte	0x5
	.long	.LASF20
	.uleb128 0x3
	.long	.LASF21
	.byte	0x4
	.byte	0x84
	.long	0xf1
	.uleb128 0x9
	.long	.LASF51
	.byte	0x94
	.byte	0x5
	.byte	0xf5
	.long	0x299
	.uleb128 0xa
	.long	.LASF22
	.byte	0x5
	.byte	0xf6
	.long	0x5e
	.byte	0
	.uleb128 0xa
	.long	.LASF23
	.byte	0x5
	.byte	0xfb
	.long	0x85
	.byte	0x4
	.uleb128 0xa
	.long	.LASF24
	.byte	0x5
	.byte	0xfc
	.long	0x85
	.byte	0x8
	.uleb128 0xa
	.long	.LASF25
	.byte	0x5
	.byte	0xfd
	.long	0x85
	.byte	0xc
	.uleb128 0xa
	.long	.LASF26
	.byte	0x5
	.byte	0xfe
	.long	0x85
	.byte	0x10
	.uleb128 0xa
	.long	.LASF27
	.byte	0x5
	.byte	0xff
	.long	0x85
	.byte	0x14
	.uleb128 0xb
	.long	.LASF28
	.byte	0x5
	.value	0x100
	.long	0x85
	.byte	0x18
	.uleb128 0xb
	.long	.LASF29
	.byte	0x5
	.value	0x101
	.long	0x85
	.byte	0x1c
	.uleb128 0xb
	.long	.LASF30
	.byte	0x5
	.value	0x102
	.long	0x85
	.byte	0x20
	.uleb128 0xb
	.long	.LASF31
	.byte	0x5
	.value	0x104
	.long	0x85
	.byte	0x24
	.uleb128 0xb
	.long	.LASF32
	.byte	0x5
	.value	0x105
	.long	0x85
	.byte	0x28
	.uleb128 0xb
	.long	.LASF33
	.byte	0x5
	.value	0x106
	.long	0x85
	.byte	0x2c
	.uleb128 0xb
	.long	.LASF34
	.byte	0x5
	.value	0x108
	.long	0x2d1
	.byte	0x30
	.uleb128 0xb
	.long	.LASF35
	.byte	0x5
	.value	0x10a
	.long	0x2d7
	.byte	0x34
	.uleb128 0xb
	.long	.LASF36
	.byte	0x5
	.value	0x10c
	.long	0x5e
	.byte	0x38
	.uleb128 0xb
	.long	.LASF37
	.byte	0x5
	.value	0x110
	.long	0x5e
	.byte	0x3c
	.uleb128 0xb
	.long	.LASF38
	.byte	0x5
	.value	0x112
	.long	0xfc
	.byte	0x40
	.uleb128 0xb
	.long	.LASF39
	.byte	0x5
	.value	0x116
	.long	0x37
	.byte	0x44
	.uleb128 0xb
	.long	.LASF40
	.byte	0x5
	.value	0x117
	.long	0xdc
	.byte	0x46
	.uleb128 0xb
	.long	.LASF41
	.byte	0x5
	.value	0x118
	.long	0x2dd
	.byte	0x47
	.uleb128 0xb
	.long	.LASF42
	.byte	0x5
	.value	0x11c
	.long	0x2ed
	.byte	0x48
	.uleb128 0xb
	.long	.LASF43
	.byte	0x5
	.value	0x125
	.long	0x10e
	.byte	0x4c
	.uleb128 0xb
	.long	.LASF44
	.byte	0x5
	.value	0x12e
	.long	0x96
	.byte	0x54
	.uleb128 0xb
	.long	.LASF45
	.byte	0x5
	.value	0x12f
	.long	0x96
	.byte	0x58
	.uleb128 0xb
	.long	.LASF46
	.byte	0x5
	.value	0x130
	.long	0x96
	.byte	0x5c
	.uleb128 0xb
	.long	.LASF47
	.byte	0x5
	.value	0x131
	.long	0x96
	.byte	0x60
	.uleb128 0xb
	.long	.LASF48
	.byte	0x5
	.value	0x132
	.long	0x65
	.byte	0x64
	.uleb128 0xb
	.long	.LASF49
	.byte	0x5
	.value	0x134
	.long	0x5e
	.byte	0x68
	.uleb128 0xb
	.long	.LASF50
	.byte	0x5
	.value	0x136
	.long	0x2f3
	.byte	0x6c
	.byte	0
	.uleb128 0xc
	.long	.LASF88
	.byte	0x5
	.byte	0x9a
	.uleb128 0x9
	.long	.LASF52
	.byte	0xc
	.byte	0x5
	.byte	0xa0
	.long	0x2d1
	.uleb128 0xa
	.long	.LASF53
	.byte	0x5
	.byte	0xa1
	.long	0x2d1
	.byte	0
	.uleb128 0xa
	.long	.LASF54
	.byte	0x5
	.byte	0xa2
	.long	0x2d7
	.byte	0x4
	.uleb128 0xa
	.long	.LASF55
	.byte	0x5
	.byte	0xa6
	.long	0x5e
	.byte	0x8
	.byte	0
	.uleb128 0x5
	.byte	0x4
	.long	0x2a0
	.uleb128 0x5
	.byte	0x4
	.long	0x119
	.uleb128 0xd
	.long	0x50
	.long	0x2ed
	.uleb128 0xe
	.long	0xd5
	.byte	0
	.byte	0
	.uleb128 0x5
	.byte	0x4
	.long	0x299
	.uleb128 0xd
	.long	0x50
	.long	0x303
	.uleb128 0xe
	.long	0xd5
	.byte	0x27
	.byte	0
	.uleb128 0xf
	.long	.LASF58
	.byte	0x1
	.value	0x161
	.byte	0x1
	.long	0x329
	.uleb128 0x10
	.long	.LASF56
	.byte	0x1
	.value	0x161
	.long	0x3e
	.uleb128 0x10
	.long	.LASF57
	.byte	0x1
	.value	0x161
	.long	0x7a
	.byte	0
	.uleb128 0xf
	.long	.LASF59
	.byte	0x1
	.value	0x169
	.byte	0x1
	.long	0x34f
	.uleb128 0x10
	.long	.LASF56
	.byte	0x1
	.value	0x169
	.long	0x3e
	.uleb128 0x10
	.long	.LASF57
	.byte	0x1
	.value	0x169
	.long	0x7a
	.byte	0
	.uleb128 0xf
	.long	.LASF60
	.byte	0x1
	.value	0x171
	.byte	0x1
	.long	0x375
	.uleb128 0x10
	.long	.LASF56
	.byte	0x1
	.value	0x171
	.long	0x3e
	.uleb128 0x10
	.long	.LASF57
	.byte	0x1
	.value	0x171
	.long	0x7a
	.byte	0
	.uleb128 0xf
	.long	.LASF61
	.byte	0x1
	.value	0x179
	.byte	0x1
	.long	0x39b
	.uleb128 0x10
	.long	.LASF56
	.byte	0x1
	.value	0x179
	.long	0x3e
	.uleb128 0x10
	.long	.LASF57
	.byte	0x1
	.value	0x179
	.long	0x7a
	.byte	0
	.uleb128 0xf
	.long	.LASF62
	.byte	0x1
	.value	0x181
	.byte	0x1
	.long	0x3c1
	.uleb128 0x10
	.long	.LASF56
	.byte	0x1
	.value	0x181
	.long	0x3e
	.uleb128 0x10
	.long	.LASF57
	.byte	0x1
	.value	0x181
	.long	0x7a
	.byte	0
	.uleb128 0x11
	.long	.LASF89
	.byte	0x1
	.byte	0x17
	.long	0xa9
	.long	.LFB11
	.long	.LFE11-.LFB11
	.uleb128 0x1
	.byte	0x9c
	.long	0x3e9
	.uleb128 0x12
	.long	.LASF63
	.byte	0x1
	.byte	0x17
	.long	0xb4
	.uleb128 0x2
	.byte	0x91
	.sleb128 0
	.byte	0
	.uleb128 0x13
	.long	.LASF66
	.byte	0x1
	.byte	0x39
	.long	.LFB12
	.long	.LFE12-.LFB12
	.uleb128 0x1
	.byte	0x9c
	.long	0x41b
	.uleb128 0x12
	.long	.LASF64
	.byte	0x1
	.byte	0x39
	.long	0x25
	.uleb128 0x2
	.byte	0x91
	.sleb128 0
	.uleb128 0x12
	.long	.LASF65
	.byte	0x1
	.byte	0x39
	.long	0x2c
	.uleb128 0x2
	.byte	0x91
	.sleb128 4
	.byte	0
	.uleb128 0x13
	.long	.LASF67
	.byte	0x1
	.byte	0x5c
	.long	.LFB13
	.long	.LFE13-.LFB13
	.uleb128 0x1
	.byte	0x9c
	.long	0x45b
	.uleb128 0x12
	.long	.LASF68
	.byte	0x1
	.byte	0x5c
	.long	0x8b
	.uleb128 0x2
	.byte	0x91
	.sleb128 0
	.uleb128 0x12
	.long	.LASF69
	.byte	0x1
	.byte	0x5c
	.long	0x3e
	.uleb128 0x2
	.byte	0x91
	.sleb128 4
	.uleb128 0x12
	.long	.LASF65
	.byte	0x1
	.byte	0x5c
	.long	0x2c
	.uleb128 0x2
	.byte	0x91
	.sleb128 8
	.byte	0
	.uleb128 0x13
	.long	.LASF70
	.byte	0x1
	.byte	0x8b
	.long	.LFB14
	.long	.LFE14-.LFB14
	.uleb128 0x1
	.byte	0x9c
	.long	0x49b
	.uleb128 0x12
	.long	.LASF68
	.byte	0x1
	.byte	0x8b
	.long	0x8b
	.uleb128 0x2
	.byte	0x91
	.sleb128 0
	.uleb128 0x12
	.long	.LASF69
	.byte	0x1
	.byte	0x8b
	.long	0x3e
	.uleb128 0x2
	.byte	0x91
	.sleb128 4
	.uleb128 0x12
	.long	.LASF65
	.byte	0x1
	.byte	0x8b
	.long	0x2c
	.uleb128 0x2
	.byte	0x91
	.sleb128 8
	.byte	0
	.uleb128 0x13
	.long	.LASF71
	.byte	0x1
	.byte	0xb8
	.long	.LFB15
	.long	.LFE15-.LFB15
	.uleb128 0x1
	.byte	0x9c
	.long	0x4cd
	.uleb128 0x12
	.long	.LASF72
	.byte	0x1
	.byte	0xb8
	.long	0x4cd
	.uleb128 0x2
	.byte	0x91
	.sleb128 0
	.uleb128 0x12
	.long	.LASF65
	.byte	0x1
	.byte	0xb8
	.long	0x2c
	.uleb128 0x2
	.byte	0x91
	.sleb128 4
	.byte	0
	.uleb128 0x5
	.byte	0x4
	.long	0x2c
	.uleb128 0x13
	.long	.LASF73
	.byte	0x1
	.byte	0xe0
	.long	.LFB16
	.long	.LFE16-.LFB16
	.uleb128 0x1
	.byte	0x9c
	.long	0x503
	.uleb128 0x14
	.string	"w1"
	.byte	0x1
	.byte	0xe0
	.long	0x2c
	.uleb128 0x2
	.byte	0x91
	.sleb128 0
	.uleb128 0x14
	.string	"w2"
	.byte	0x1
	.byte	0xe0
	.long	0x2c
	.uleb128 0x2
	.byte	0x91
	.sleb128 4
	.byte	0
	.uleb128 0x15
	.long	.LASF74
	.byte	0x1
	.value	0x102
	.long	.LFB17
	.long	.LFE17-.LFB17
	.uleb128 0x1
	.byte	0x9c
	.long	0x544
	.uleb128 0x16
	.string	"p1"
	.byte	0x1
	.value	0x102
	.long	0x8b
	.uleb128 0x2
	.byte	0x91
	.sleb128 0
	.uleb128 0x16
	.string	"d1"
	.byte	0x1
	.value	0x102
	.long	0x3e
	.uleb128 0x2
	.byte	0x91
	.sleb128 4
	.uleb128 0x16
	.string	"w1"
	.byte	0x1
	.value	0x102
	.long	0x2c
	.uleb128 0x2
	.byte	0x91
	.sleb128 8
	.byte	0
	.uleb128 0x15
	.long	.LASF75
	.byte	0x1
	.value	0x132
	.long	.LFB18
	.long	.LFE18-.LFB18
	.uleb128 0x1
	.byte	0x9c
	.long	0x585
	.uleb128 0x16
	.string	"p1"
	.byte	0x1
	.value	0x132
	.long	0x8b
	.uleb128 0x2
	.byte	0x91
	.sleb128 0
	.uleb128 0x16
	.string	"d1"
	.byte	0x1
	.value	0x132
	.long	0x3e
	.uleb128 0x2
	.byte	0x91
	.sleb128 4
	.uleb128 0x16
	.string	"w1"
	.byte	0x1
	.value	0x132
	.long	0x2c
	.uleb128 0x2
	.byte	0x91
	.sleb128 8
	.byte	0
	.uleb128 0x15
	.long	.LASF76
	.byte	0x1
	.value	0x189
	.long	.LFB24
	.long	.LFE24-.LFB24
	.uleb128 0x1
	.byte	0x9c
	.long	0x6d8
	.uleb128 0x17
	.long	.LASF77
	.byte	0x1
	.value	0x189
	.long	0x3e
	.uleb128 0x2
	.byte	0x91
	.sleb128 0
	.uleb128 0x17
	.long	.LASF56
	.byte	0x1
	.value	0x189
	.long	0x3e
	.uleb128 0x2
	.byte	0x91
	.sleb128 4
	.uleb128 0x17
	.long	.LASF57
	.byte	0x1
	.value	0x189
	.long	0x7a
	.uleb128 0x2
	.byte	0x91
	.sleb128 8
	.uleb128 0x18
	.long	0x303
	.long	.LBB12
	.long	.Ldebug_ranges0+0
	.byte	0x1
	.value	0x18e
	.long	0x61c
	.uleb128 0x19
	.long	0x310
	.long	.LLST0
	.uleb128 0x19
	.long	0x31c
	.long	.LLST1
	.uleb128 0x1a
	.long	.LVL10
	.long	0x74a
	.uleb128 0x1b
	.long	.LVL15
	.long	0x74a
	.uleb128 0x1b
	.long	.LVL18
	.long	0x74a
	.uleb128 0x1b
	.long	.LVL21
	.long	0x74a
	.uleb128 0x1b
	.long	.LVL24
	.long	0x74a
	.byte	0
	.uleb128 0x1c
	.long	0x39b
	.long	.LBB22
	.long	.LBE22-.LBB22
	.byte	0x1
	.value	0x19a
	.long	0x64c
	.uleb128 0x19
	.long	0x3a8
	.long	.LLST2
	.uleb128 0x19
	.long	0x3b4
	.long	.LLST3
	.uleb128 0x1a
	.long	.LVL13
	.long	0x74a
	.byte	0
	.uleb128 0x1c
	.long	0x329
	.long	.LBB26
	.long	.LBE26-.LBB26
	.byte	0x1
	.value	0x191
	.long	0x67c
	.uleb128 0x19
	.long	0x336
	.long	.LLST4
	.uleb128 0x19
	.long	0x342
	.long	.LLST5
	.uleb128 0x1a
	.long	.LVL16
	.long	0x74a
	.byte	0
	.uleb128 0x1c
	.long	0x34f
	.long	.LBB30
	.long	.LBE30-.LBB30
	.byte	0x1
	.value	0x194
	.long	0x6ac
	.uleb128 0x19
	.long	0x35c
	.long	.LLST6
	.uleb128 0x19
	.long	0x368
	.long	.LLST7
	.uleb128 0x1a
	.long	.LVL19
	.long	0x74a
	.byte	0
	.uleb128 0x1d
	.long	0x375
	.long	.LBB34
	.long	.LBE34-.LBB34
	.byte	0x1
	.value	0x197
	.uleb128 0x1e
	.long	0x382
	.uleb128 0x2
	.byte	0x91
	.sleb128 4
	.uleb128 0x19
	.long	0x38e
	.long	.LLST8
	.uleb128 0x1a
	.long	.LVL22
	.long	0x74a
	.byte	0
	.byte	0
	.uleb128 0x15
	.long	.LASF78
	.byte	0x1
	.value	0x1a4
	.long	.LFB25
	.long	.LFE25-.LFB25
	.uleb128 0x1
	.byte	0x9c
	.long	0x734
	.uleb128 0x17
	.long	.LASF79
	.byte	0x1
	.value	0x1a4
	.long	0x7a
	.uleb128 0x2
	.byte	0x91
	.sleb128 0
	.uleb128 0x17
	.long	.LASF80
	.byte	0x1
	.value	0x1a4
	.long	0x3e
	.uleb128 0x2
	.byte	0x91
	.sleb128 4
	.uleb128 0x1f
	.long	.LASF90
	.byte	0x1
	.value	0x1a6
	.long	0x3e
	.uleb128 0x20
	.long	.LASF83
	.byte	0x1
	.value	0x1a9
	.long	0x5e
	.long	0x72a
	.uleb128 0x21
	.byte	0
	.uleb128 0x1a
	.long	.LVL26
	.long	0x75b
	.byte	0
	.uleb128 0x22
	.long	.LASF81
	.byte	0x6
	.byte	0xa8
	.long	0x2d7
	.uleb128 0x22
	.long	.LASF82
	.byte	0x6
	.byte	0xa9
	.long	0x2d7
	.uleb128 0x23
	.long	.LASF84
	.byte	0x7
	.byte	0x26
	.long	0x75b
	.uleb128 0x8
	.long	0x7a
	.byte	0
	.uleb128 0x24
	.long	.LASF83
	.byte	0x1
	.value	0x1a9
	.long	0x5e
	.uleb128 0x21
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
	.uleb128 0x5
	.uleb128 0xf
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
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
	.uleb128 0x15
	.byte	0x1
	.uleb128 0x27
	.uleb128 0x19
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x8
	.uleb128 0x5
	.byte	0
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x9
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
	.uleb128 0xa
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
	.uleb128 0xb
	.byte	0
	.byte	0
	.uleb128 0xd
	.uleb128 0x1
	.byte	0x1
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0xe
	.uleb128 0x21
	.byte	0
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2f
	.uleb128 0xb
	.byte	0
	.byte	0
	.uleb128 0xf
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x27
	.uleb128 0x19
	.uleb128 0x20
	.uleb128 0xb
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x10
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
	.byte	0
	.byte	0
	.uleb128 0x11
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3f
	.uleb128 0x19
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
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
	.uleb128 0x12
	.uleb128 0x5
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x18
	.byte	0
	.byte	0
	.uleb128 0x13
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3f
	.uleb128 0x19
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
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
	.uleb128 0x14
	.uleb128 0x5
	.byte	0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x18
	.byte	0
	.byte	0
	.uleb128 0x15
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
	.uleb128 0x16
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
	.uleb128 0x17
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
	.uleb128 0x18
	.uleb128 0x1d
	.byte	0x1
	.uleb128 0x31
	.uleb128 0x13
	.uleb128 0x52
	.uleb128 0x1
	.uleb128 0x55
	.uleb128 0x17
	.uleb128 0x58
	.uleb128 0xb
	.uleb128 0x59
	.uleb128 0x5
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x19
	.uleb128 0x5
	.byte	0
	.uleb128 0x31
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x17
	.byte	0
	.byte	0
	.uleb128 0x1a
	.uleb128 0x4109
	.byte	0
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x31
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x1b
	.uleb128 0x4109
	.byte	0
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x2115
	.uleb128 0x19
	.uleb128 0x31
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x1c
	.uleb128 0x1d
	.byte	0x1
	.uleb128 0x31
	.uleb128 0x13
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x6
	.uleb128 0x58
	.uleb128 0xb
	.uleb128 0x59
	.uleb128 0x5
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x1d
	.uleb128 0x1d
	.byte	0x1
	.uleb128 0x31
	.uleb128 0x13
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x6
	.uleb128 0x58
	.uleb128 0xb
	.uleb128 0x59
	.uleb128 0x5
	.byte	0
	.byte	0
	.uleb128 0x1e
	.uleb128 0x5
	.byte	0
	.uleb128 0x31
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x18
	.byte	0
	.byte	0
	.uleb128 0x1f
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
	.byte	0
	.byte	0
	.uleb128 0x20
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
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x3c
	.uleb128 0x19
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x21
	.uleb128 0x18
	.byte	0
	.byte	0
	.byte	0
	.uleb128 0x22
	.uleb128 0x34
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x3f
	.uleb128 0x19
	.uleb128 0x3c
	.uleb128 0x19
	.byte	0
	.byte	0
	.uleb128 0x23
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3f
	.uleb128 0x19
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x27
	.uleb128 0x19
	.uleb128 0x3c
	.uleb128 0x19
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x24
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
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x3c
	.uleb128 0x19
	.byte	0
	.byte	0
	.byte	0
	.section	.debug_loc,"",@progbits
.Ldebug_loc0:
.LLST0:
	.long	.LVL9-.Ltext0
	.long	.LVL11-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 4
	.long	0
	.long	0
.LLST1:
	.long	.LVL9-.Ltext0
	.long	.LVL11-.Ltext0
	.value	0x1
	.byte	0x53
	.long	0
	.long	0
.LLST2:
	.long	.LVL12-.Ltext0
	.long	.LVL14-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 4
	.long	0
	.long	0
.LLST3:
	.long	.LVL12-.Ltext0
	.long	.LVL14-.Ltext0
	.value	0x1
	.byte	0x53
	.long	0
	.long	0
.LLST4:
	.long	.LVL15-.Ltext0
	.long	.LVL18-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 4
	.long	0
	.long	0
.LLST5:
	.long	.LVL15-.Ltext0
	.long	.LVL17-.Ltext0
	.value	0x1
	.byte	0x53
	.long	.LVL17-.Ltext0
	.long	.LVL18-1-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 0
	.long	.LVL18-1-.Ltext0
	.long	.LVL18-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 8
	.long	0
	.long	0
.LLST6:
	.long	.LVL18-.Ltext0
	.long	.LVL21-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 4
	.long	0
	.long	0
.LLST7:
	.long	.LVL18-.Ltext0
	.long	.LVL20-.Ltext0
	.value	0x1
	.byte	0x53
	.long	.LVL20-.Ltext0
	.long	.LVL21-1-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 0
	.long	.LVL21-1-.Ltext0
	.long	.LVL21-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 8
	.long	0
	.long	0
.LLST8:
	.long	.LVL21-.Ltext0
	.long	.LVL23-.Ltext0
	.value	0x1
	.byte	0x53
	.long	.LVL23-.Ltext0
	.long	.LVL24-1-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 0
	.long	.LVL24-1-.Ltext0
	.long	.LFE24-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 8
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
	.section	.debug_ranges,"",@progbits
.Ldebug_ranges0:
	.long	.LBB12-.Ltext0
	.long	.LBE12-.Ltext0
	.long	.LBB24-.Ltext0
	.long	.LBE24-.Ltext0
	.long	.LBB25-.Ltext0
	.long	.LBE25-.Ltext0
	.long	.LBB28-.Ltext0
	.long	.LBE28-.Ltext0
	.long	.LBB29-.Ltext0
	.long	.LBE29-.Ltext0
	.long	.LBB32-.Ltext0
	.long	.LBE32-.Ltext0
	.long	.LBB33-.Ltext0
	.long	.LBE33-.Ltext0
	.long	.LBB36-.Ltext0
	.long	.LBE36-.Ltext0
	.long	.LBB37-.Ltext0
	.long	.LBE37-.Ltext0
	.long	0
	.long	0
	.section	.debug_line,"",@progbits
.Ldebug_line0:
	.section	.debug_str,"MS",@progbits,1
.LASF80:
	.string	"dwLineNum"
.LASF77:
	.string	"dwLevel"
.LASF38:
	.string	"_old_offset"
.LASF40:
	.string	"_vtable_offset"
.LASF8:
	.string	"double"
.LASF61:
	.string	"AlarmErrorHandler"
.LASF65:
	.string	"wPort"
.LASF33:
	.string	"_IO_save_end"
.LASF73:
	.string	"WriteWordToPort"
.LASF6:
	.string	"short int"
.LASF14:
	.string	"sizetype"
.LASF43:
	.string	"_offset"
.LASF3:
	.string	"DWORD"
.LASF12:
	.string	"INT_HANDLER"
.LASF27:
	.string	"_IO_write_ptr"
.LASF22:
	.string	"_flags"
.LASF67:
	.string	"ReadByteStringFromPort"
.LASF60:
	.string	"ImportantErrorHandler"
.LASF29:
	.string	"_IO_buf_base"
.LASF85:
	.string	"GNU C 4.9.2 20150212 (Red Hat 4.9.2-6) -m32 -mtune=generic -march=i686 -g -O2"
.LASF34:
	.string	"_markers"
.LASF24:
	.string	"_IO_read_end"
.LASF69:
	.string	"dwBufLen"
.LASF89:
	.string	"SetGeneralIntHandler"
.LASF2:
	.string	"WORD"
.LASF66:
	.string	"WriteByteToPort"
.LASF11:
	.string	"LPVOID"
.LASF68:
	.string	"lpBuffer"
.LASF78:
	.string	"__BUG"
.LASF9:
	.string	"float"
.LASF16:
	.string	"long long int"
.LASF42:
	.string	"_lock"
.LASF20:
	.string	"long int"
.LASF39:
	.string	"_cur_column"
.LASF28:
	.string	"_IO_write_end"
.LASF55:
	.string	"_pos"
.LASF79:
	.string	"lpszFileName"
.LASF64:
	.string	"byte"
.LASF86:
	.string	"hellocn.c"
.LASF54:
	.string	"_sbuf"
.LASF51:
	.string	"_IO_FILE"
.LASF82:
	.string	"stdout"
.LASF0:
	.string	"unsigned char"
.LASF15:
	.string	"signed char"
.LASF17:
	.string	"long long unsigned int"
.LASF7:
	.string	"unsigned int"
.LASF52:
	.string	"_IO_marker"
.LASF41:
	.string	"_shortbuf"
.LASF10:
	.string	"LPSTR"
.LASF26:
	.string	"_IO_write_base"
.LASF50:
	.string	"_unused2"
.LASF62:
	.string	"InformErrorHandler"
.LASF23:
	.string	"_IO_read_ptr"
.LASF48:
	.string	"__pad5"
.LASF30:
	.string	"_IO_buf_end"
.LASF76:
	.string	"ErrorHandler"
.LASF71:
	.string	"ReadWordFromPort"
.LASF5:
	.string	"char"
.LASF13:
	.string	"__GENERAL_INTERRUPT_HANDLER"
.LASF53:
	.string	"_next"
.LASF44:
	.string	"__pad1"
.LASF45:
	.string	"__pad2"
.LASF46:
	.string	"__pad3"
.LASF47:
	.string	"__pad4"
.LASF18:
	.string	"__quad_t"
.LASF90:
	.string	"dwFlags"
.LASF74:
	.string	"ReadWordStringFromPort"
.LASF1:
	.string	"short unsigned int"
.LASF4:
	.string	"long unsigned int"
.LASF21:
	.string	"__off64_t"
.LASF19:
	.string	"__off_t"
.LASF35:
	.string	"_chain"
.LASF59:
	.string	"CriticalErrorHandler"
.LASF87:
	.string	"/media/gaojie/Dev/hellox/HelloX_OS/kernel/arch/x86"
.LASF75:
	.string	"WriteWordStringToPort"
.LASF32:
	.string	"_IO_backup_base"
.LASF81:
	.string	"stdin"
.LASF58:
	.string	"FatalErrorHandler"
.LASF37:
	.string	"_flags2"
.LASF72:
	.string	"pWord"
.LASF49:
	.string	"_mode"
.LASF25:
	.string	"_IO_read_base"
.LASF56:
	.string	"dwReason"
.LASF84:
	.string	"PrintLine"
.LASF31:
	.string	"_IO_save_base"
.LASF36:
	.string	"_fileno"
.LASF63:
	.string	"TimerHandler"
.LASF70:
	.string	"WriteByteStringToPort"
.LASF83:
	.string	"_hx_printf"
.LASF57:
	.string	"lpszMsg"
.LASF88:
	.string	"_IO_lock_t"
	.ident	"GCC: (GNU) 4.9.2 20150212 (Red Hat 4.9.2-6)"
	.section	.note.GNU-stack,"",@progbits
