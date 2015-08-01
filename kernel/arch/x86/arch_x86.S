	.file	"arch_x86.c"
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
	.type	ReadCmosData, @function
ReadCmosData:
.LFB7:
	.file 1 "arch_x86.c"
	.loc 1 311 0
	.cfi_startproc
.LVL0:
	.loc 1 313 0
#APP
# 313 "arch_x86.c" 1
	.code32				
	pushl	%ebp			
	movl	%esp,	%ebp	
	pushl	%ebx			
	pushl	%edx			
	movl	0x08(%ebp),%ebx	
	xorw	%ax,	%ax	
	movb	%dl,	%al	
	outb	%al,	$0x70	
	inb	$0x71,	%al	
	movw	%ax,	(%ebx)	
	popl	%edx			
	popl	%ebx			
	leave					
	ret					
	
# 0 "" 2
.LVL1:
	.loc 1 356 0
#NO_APP
	ret
	.cfi_endproc
.LFE7:
	.size	ReadCmosData, .-ReadCmosData
	.section	.text.unlikely
.LCOLDE0:
	.text
.LHOTE0:
	.section	.text.unlikely
.LCOLDB1:
	.text
.LHOTB1:
	.p2align 4,,15
	.globl	HardwareInitialize
	.type	HardwareInitialize, @function
HardwareInitialize:
.LFB0:
	.loc 1 41 0
	.cfi_startproc
	.loc 1 45 0
	movl	$1, %eax
	ret
	.cfi_endproc
.LFE0:
	.size	HardwareInitialize, .-HardwareInitialize
	.section	.text.unlikely
.LCOLDE1:
	.text
.LHOTE1:
	.section	.text.unlikely
.LCOLDB2:
	.text
.LHOTB2:
	.p2align 4,,15
	.globl	__SwitchTo
	.type	__SwitchTo, @function
__SwitchTo:
.LFB1:
	.loc 1 53 0
	.cfi_startproc
.LVL2:
	.loc 1 55 0
#APP
# 55 "arch_x86.c" 1
	.code32						
	 pushl 	%ebp					
	movl	%esp,	%ebp			
	movl	0x08(%ebp),	%esp	
	popl	%ebp	
	popl	%edi	
	popl	%esi	
	popl	%edx	
	popl	%ecx	
	popl	%ebx	
	movb	$0x20,	%al	
	outb	%al,	$0x20	
	outb	%al,	$0xa0	
	popl	%eax			
	iret					
	
# 0 "" 2
#NO_APP
	ret
	.cfi_endproc
.LFE1:
	.size	__SwitchTo, .-__SwitchTo
	.section	.text.unlikely
.LCOLDE2:
	.text
.LHOTE2:
	.section	.text.unlikely
.LCOLDB3:
	.text
.LHOTB3:
	.p2align 4,,15
	.globl	__SaveAndSwitch
	.type	__SaveAndSwitch, @function
__SaveAndSwitch:
.LFB2:
	.loc 1 110 0
	.cfi_startproc
.LVL3:
	.loc 1 113 0
#APP
# 113 "arch_x86.c" 1
	.code32								
	movl	%esp,	dwTmpEbp                  	
	popl	dwTmpEip		                      	
	movl	%eax,	dwTmpEax   				
	pushf             					
	xorl	%eax,	%eax     				
	movw	%cs,	%ax          				
	pushl	%eax           				
	pushl	dwTmpEip      				
	pushl dwTmpEax      				
	pushl %ebx                          	
	pushl %ecx                          	
	pushl %edx                          	
	pushl %esi                          	
	pushl %edi                          	
	pushl %ebp                          	
	             	                     	
	movl %ebp,dwTmpEbp                  		
	movl 0x04(%ebp),	%ebx				
	movl %esp,	(%ebx)					
	                                     
	movl 0x08(%ebp),	%ebx				
	movl (%ebx),		%esp   			  
	popl %ebp                              
	popl %edi                              
	popl %esi                              
	popl %edx                            
	popl %ecx                 			
	popl %ebx         					
	popl %eax							
	iret
# 0 "" 2
#NO_APP
	ret
	.cfi_endproc
.LFE2:
	.size	__SaveAndSwitch, .-__SaveAndSwitch
	.section	.text.unlikely
.LCOLDE3:
	.text
.LHOTE3:
	.section	.text.unlikely
.LCOLDB4:
	.text
.LHOTB4:
	.p2align 4,,15
	.globl	EnableVMM
	.type	EnableVMM, @function
EnableVMM:
.LFB3:
	.loc 1 191 0
	.cfi_startproc
	.loc 1 193 0
	movl	$2031616, %eax
#APP
# 193 "arch_x86.c" 1
	.code32			
	pushl	%eax		
	movl	%eax,	%eax	
	movl	%eax,		%cr3	
	movl	%cr0,		%eax	
	orl	$0x80000000,	%eax	
	movl	%eax,		%cr0	
	popl	%eax				
	
# 0 "" 2
#NO_APP
	ret
	.cfi_endproc
.LFE3:
	.size	EnableVMM, .-EnableVMM
	.section	.text.unlikely
.LCOLDE4:
	.text
.LHOTE4:
	.section	.text.unlikely
.LCOLDB5:
	.text
.LHOTB5:
	.p2align 4,,15
	.globl	HaltSystem
	.type	HaltSystem, @function
HaltSystem:
.LFB4:
	.loc 1 221 0
	.cfi_startproc
	.loc 1 223 0
#APP
# 223 "arch_x86.c" 1
	hlt	
	
# 0 "" 2
#NO_APP
	ret
	.cfi_endproc
.LFE4:
	.size	HaltSystem, .-HaltSystem
	.section	.text.unlikely
.LCOLDE5:
	.text
.LHOTE5:
	.section	.text.unlikely
.LCOLDB6:
	.text
.LHOTB6:
	.p2align 4,,15
	.globl	InitKernelThreadContext
	.type	InitKernelThreadContext, @function
InitKernelThreadContext:
.LFB5:
	.loc 1 237 0
	.cfi_startproc
.LVL4:
	.loc 1 237 0
	movl	4(%esp), %edx
	movl	8(%esp), %ecx
	.loc 1 241 0
	testl	%edx, %edx
	je	.L7
	testl	%ecx, %ecx
	je	.L7
	.loc 1 253 0
	movl	140(%edx), %eax
.LVL5:
	.loc 1 255 0
	movl	%edx, -4(%eax)
.LVL6:
	.loc 1 256 0
	movl	$0, -8(%eax)
.LVL7:
	.loc 1 266 0
	subl	$48, %eax
.LVL8:
	.loc 1 257 0
	movl	$512, 36(%eax)
.LVL9:
	.loc 1 258 0
	movl	$8, 32(%eax)
	.loc 1 259 0
	movl	%ecx, 28(%eax)
	.loc 1 260 0
	movl	$0, 24(%eax)
	.loc 1 261 0
	movl	$0, 20(%eax)
	.loc 1 262 0
	movl	$0, 16(%eax)
	.loc 1 263 0
	movl	$0, 12(%eax)
	.loc 1 264 0
	movl	$0, 8(%eax)
	.loc 1 265 0
	movl	$0, 4(%eax)
	.loc 1 266 0
	movl	$0, (%eax)
	movl	%eax, 84(%edx)
.LVL10:
.L7:
	rep ret
	.cfi_endproc
.LFE5:
	.size	InitKernelThreadContext, .-InitKernelThreadContext
	.section	.text.unlikely
.LCOLDE6:
	.text
.LHOTE6:
	.section	.text.unlikely
.LCOLDB7:
	.text
.LHOTB7:
	.p2align 4,,15
	.globl	__GetTsc
	.type	__GetTsc, @function
__GetTsc:
.LFB6:
	.loc 1 275 0
	.cfi_startproc
.LVL11:
	.loc 1 277 0
#APP
# 277 "arch_x86.c" 1
	.code32		
	pushl 	%eax	
	pushl	%edx	
	pushl	%ebx	
	rdtsc			
	movl	0x08(%ebp),	%ebx	
	movl	%eax,			(%ebx)	
	movl	%edx,		0x04(%ebx)	
	popl	%ebx	
	popl	%edx	
	popl	%eax	
	
# 0 "" 2
#NO_APP
	ret
	.cfi_endproc
.LFE6:
	.size	__GetTsc, .-__GetTsc
	.section	.text.unlikely
.LCOLDE7:
	.text
.LHOTE7:
	.section	.text.unlikely
.LCOLDB8:
	.text
.LHOTB8:
	.p2align 4,,15
	.globl	__GetTime
	.type	__GetTime, @function
__GetTime:
.LFB8:
	.loc 1 360 0
	.cfi_startproc
.LVL12:
	pushl	%ebx
	.cfi_def_cfa_offset 8
	.cfi_offset 3, -8
	.loc 1 360 0
	movl	8(%esp), %ebx
	.loc 1 361 0
	movl	$9, %edx
	movl	%ebx, %eax
	call	ReadCmosData
.LVL13:
	.loc 1 362 0
	leal	1(%ebx), %eax
	movl	$8, %edx
	call	ReadCmosData
.LVL14:
	.loc 1 363 0
	leal	2(%ebx), %eax
	movl	$7, %edx
	call	ReadCmosData
.LVL15:
	.loc 1 365 0
	leal	3(%ebx), %eax
	movl	$4, %edx
	call	ReadCmosData
.LVL16:
	.loc 1 366 0
	leal	4(%ebx), %eax
	movl	$2, %edx
	call	ReadCmosData
.LVL17:
	.loc 1 367 0
	leal	5(%ebx), %eax
	xorl	%edx, %edx
	call	ReadCmosData
.LVL18:
	.loc 1 370 0
	movzbl	(%ebx), %ecx
	movl	%ecx, %eax
	andl	$15, %ecx
	shrb	$4, %al
	leal	(%eax,%eax,4), %edx
	leal	(%ecx,%edx,2), %eax
	.loc 1 371 0
	movzbl	1(%ebx), %ecx
	.loc 1 370 0
	movb	%al, (%ebx)
	.loc 1 371 0
	movl	%ecx, %eax
	andl	$15, %ecx
	shrb	$4, %al
	leal	(%eax,%eax,4), %edx
	leal	(%ecx,%edx,2), %eax
	.loc 1 372 0
	movzbl	2(%ebx), %ecx
	.loc 1 371 0
	movb	%al, 1(%ebx)
	.loc 1 372 0
	movl	%ecx, %eax
	andl	$15, %ecx
	shrb	$4, %al
	leal	(%eax,%eax,4), %edx
	leal	(%ecx,%edx,2), %eax
	.loc 1 373 0
	movzbl	3(%ebx), %ecx
	.loc 1 372 0
	movb	%al, 2(%ebx)
	.loc 1 373 0
	movl	%ecx, %eax
	andl	$15, %ecx
	shrb	$4, %al
	leal	(%eax,%eax,4), %edx
	leal	(%ecx,%edx,2), %eax
	.loc 1 374 0
	movzbl	4(%ebx), %ecx
	.loc 1 373 0
	movb	%al, 3(%ebx)
	.loc 1 374 0
	movl	%ecx, %eax
	andl	$15, %ecx
	shrb	$4, %al
	leal	(%eax,%eax,4), %edx
	leal	(%ecx,%edx,2), %eax
	.loc 1 375 0
	movzbl	5(%ebx), %ecx
	.loc 1 374 0
	movb	%al, 4(%ebx)
	.loc 1 375 0
	movl	%ecx, %eax
	andl	$15, %ecx
	shrb	$4, %al
	leal	(%eax,%eax,4), %edx
	leal	(%ecx,%edx,2), %eax
	movb	%al, 5(%ebx)
	.loc 1 376 0
	popl	%ebx
	.cfi_restore 3
	.cfi_def_cfa_offset 4
	ret
	.cfi_endproc
.LFE8:
	.size	__GetTime, .-__GetTime
	.section	.text.unlikely
.LCOLDE8:
	.text
.LHOTE8:
	.section	.text.unlikely
.LCOLDB9:
	.text
.LHOTB9:
	.p2align 4,,15
	.globl	__MicroDelay
	.type	__MicroDelay, @function
__MicroDelay:
.LFB9:
	.loc 1 381 0
	.cfi_startproc
.LVL19:
	pushl	%esi
	.cfi_def_cfa_offset 8
	.cfi_offset 6, -8
	pushl	%ebx
	.cfi_def_cfa_offset 12
	.cfi_offset 3, -12
	subl	$28, %esp
	.cfi_def_cfa_offset 40
.LVL20:
.LBB6:
.LBB7:
	.loc 1 277 0
#APP
# 277 "arch_x86.c" 1
	.code32		
	pushl 	%eax	
	pushl	%edx	
	pushl	%ebx	
	rdtsc			
	movl	0x08(%ebp),	%ebx	
	movl	%eax,			(%ebx)	
	movl	%edx,		0x04(%ebx)	
	popl	%ebx	
	popl	%edx	
	popl	%eax	
	
# 0 "" 2
#NO_APP
.LBE7:
.LBE6:
	.loc 1 387 0
	movl	40(%esp), %eax
	.loc 1 386 0
	movl	$0, 12(%esp)
	.loc 1 387 0
	movl	%eax, 8(%esp)
	.loc 1 389 0
	pushl	$10
	.cfi_def_cfa_offset 44
	leal	12(%esp), %esi
	pushl	%esi
	.cfi_def_cfa_offset 48
	call	u64RotateLeft
.LVL21:
	.loc 1 390 0
	addl	$12, %esp
	.cfi_def_cfa_offset 36
	leal	12(%esp), %ebx
.LVL22:
	pushl	%ebx
	.cfi_def_cfa_offset 40
	pushl	%esi
	.cfi_def_cfa_offset 44
	pushl	%ebx
	.cfi_def_cfa_offset 48
	call	u64Add
.LVL23:
	addl	$16, %esp
	.cfi_def_cfa_offset 32
	.p2align 4,,10
	.p2align 3
.L14:
.LVL24:
.LBB8:
.LBB9:
	.loc 1 277 0
#APP
# 277 "arch_x86.c" 1
	.code32		
	pushl 	%eax	
	pushl	%edx	
	pushl	%ebx	
	rdtsc			
	movl	0x08(%ebp),	%ebx	
	movl	%eax,			(%ebx)	
	movl	%edx,		0x04(%ebx)	
	popl	%ebx	
	popl	%edx	
	popl	%eax	
	
# 0 "" 2
#NO_APP
.LBE9:
.LBE8:
	.loc 1 394 0
	subl	$8, %esp
	.cfi_def_cfa_offset 40
	pushl	%ebx
	.cfi_def_cfa_offset 44
	pushl	%esi
	.cfi_def_cfa_offset 48
	call	MoreThan
.LVL25:
	addl	$16, %esp
	.cfi_def_cfa_offset 32
	testl	%eax, %eax
	je	.L14
	.loc 1 400 0
	addl	$20, %esp
	.cfi_def_cfa_offset 12
	popl	%ebx
	.cfi_restore 3
	.cfi_def_cfa_offset 8
.LVL26:
	popl	%esi
	.cfi_restore 6
	.cfi_def_cfa_offset 4
.LVL27:
	ret
	.cfi_endproc
.LFE9:
	.size	__MicroDelay, .-__MicroDelay
	.section	.text.unlikely
.LCOLDE9:
	.text
.LHOTE9:
	.section	.text.unlikely
.LCOLDB10:
	.text
.LHOTB10:
	.p2align 4,,15
	.globl	__outd
	.type	__outd, @function
__outd:
.LFB10:
	.loc 1 403 0
	.cfi_startproc
.LVL28:
	.loc 1 405 0
	movl	8(%esp), %edx
	movl	4(%esp), %eax
#APP
# 405 "arch_x86.c" 1
	.code32		
	pushl	%eax	
	pushl	%edx	
	movw	%ax,	%dx	
	movl	%edx,	%eax	
	outw	%eax,	%dx	
	popl	%edx	
	popl	%eax	
	
# 0 "" 2
#NO_APP
	ret
	.cfi_endproc
.LFE10:
	.size	__outd, .-__outd
	.section	.text.unlikely
.LCOLDE10:
	.text
.LHOTE10:
	.section	.text.unlikely
.LCOLDB11:
	.text
.LHOTB11:
	.p2align 4,,15
	.globl	__ind
	.type	__ind, @function
__ind:
.LFB11:
	.loc 1 431 0
	.cfi_startproc
.LVL29:
	.loc 1 434 0
	movl	4(%esp), %eax
#APP
# 434 "arch_x86.c" 1
	.code32	
	pushl	%eax	
	pushl	%edx	
	movb	%ax,	%dx	
	inw	%dx,	%eax	
	movl	%eax,	%eax	
	popl	%edx	
	popl	%eax	
	
# 0 "" 2
.LVL30:
	.loc 1 458 0
#NO_APP
	ret
	.cfi_endproc
.LFE11:
	.size	__ind, .-__ind
	.section	.text.unlikely
.LCOLDE11:
	.text
.LHOTE11:
	.section	.text.unlikely
.LCOLDB12:
	.text
.LHOTB12:
	.p2align 4,,15
	.globl	__outb
	.type	__outb, @function
__outb:
.LFB12:
	.loc 1 461 0
	.cfi_startproc
.LVL31:
	.loc 1 463 0
	movl	8(%esp), %edx
	movl	4(%esp), %eax
#APP
# 463 "arch_x86.c" 1
	.code32		    
	pushl %eax		
	pushl %edx        
	movb %al, %al      
	movw %dx, %dx      
	outb %al, %dx    
	popl %edx 	    
	popl %eax			
	
# 0 "" 2
#NO_APP
	ret
	.cfi_endproc
.LFE12:
	.size	__outb, .-__outb
	.section	.text.unlikely
.LCOLDE12:
	.text
.LHOTE12:
	.section	.text.unlikely
.LCOLDB13:
	.text
.LHOTB13:
	.p2align 4,,15
	.globl	__inb
	.type	__inb, @function
__inb:
.LFB13:
	.loc 1 489 0
	.cfi_startproc
.LVL32:
	.loc 1 492 0
	movl	4(%esp), %eax
#APP
# 492 "arch_x86.c" 1
	.code32				
	xorl 	%eax,	%eax 	
	pushl 	%edx    		
	movw 	%ax,		%dx	
	inb 	%dx,	%al    
	popl	%edx     		
	movb 	%al,	%cl 		
	
# 0 "" 2
#NO_APP
	movl	%ecx, %eax
.LVL33:
	.loc 1 514 0
	ret
	.cfi_endproc
.LFE13:
	.size	__inb, .-__inb
	.section	.text.unlikely
.LCOLDE13:
	.text
.LHOTE13:
	.section	.text.unlikely
.LCOLDB14:
	.text
.LHOTB14:
	.p2align 4,,15
	.globl	__inw
	.type	__inw, @function
__inw:
.LFB14:
	.loc 1 517 0
	.cfi_startproc
.LVL34:
	.loc 1 520 0
	movl	4(%esp), %eax
#APP
# 520 "arch_x86.c" 1
	.code32					
	pushl %eax				
	pushl %edx				
	movw %ax,%dx				
	inb %dx,%ax				
	movb %ax,%ax				
	popl %edx					
	popl %eax					
	
# 0 "" 2
.LVL35:
	.loc 1 544 0
#NO_APP
	ret
	.cfi_endproc
.LFE14:
	.size	__inw, .-__inw
	.section	.text.unlikely
.LCOLDE14:
	.text
.LHOTE14:
	.section	.text.unlikely
.LCOLDB15:
	.text
.LHOTB15:
	.p2align 4,,15
	.globl	__outw
	.type	__outw, @function
__outw:
.LFB15:
	.loc 1 547 0
	.cfi_startproc
.LVL36:
	.loc 1 549 0
	movl	8(%esp), %edx
	movl	4(%esp), %eax
#APP
# 549 "arch_x86.c" 1
	.code32	
	pushl %eax				
			pushl 	%edx             	
			movl %ax,	%ax          	
			movl %ax,	%dx         	
			outw %ax, 	%dx           	
			popl %edx				 	
			popl %eax					
	
# 0 "" 2
#NO_APP
	ret
	.cfi_endproc
.LFE15:
	.size	__outw, .-__outw
	.section	.text.unlikely
.LCOLDE15:
	.text
.LHOTE15:
	.section	.text.unlikely
.LCOLDB16:
	.text
.LHOTB16:
	.p2align 4,,15
	.globl	__inws
	.type	__inws, @function
__inws:
.LFB16:
	.loc 1 577 0
	.cfi_startproc
.LVL37:
	.loc 1 581 0
#APP
# 581 "arch_x86.c" 1
		.code32								
		pushl 	%ebp                                      
		movl 	%esp,	%ebp                               
		pushl %ecx                                      
		pushl %edx                                      
		pushl %edi                                      
		movl 	0x08(%ebp),	%edi				     
		movl 	0x0c(%ebp),	%ecx						
		shrl 	$0x01,			%ecx                          
		movw 	0x10(%ebp),	%dx				
		cld                                        	
		rep 	insw                                    
		popl %edi                                     
		popl %edx                                     
		popl %ecx                                     
		leave                                      	
		ret                                		
	
# 0 "" 2
#NO_APP
	ret
	.cfi_endproc
.LFE16:
	.size	__inws, .-__inws
	.section	.text.unlikely
.LCOLDE16:
	.text
.LHOTE16:
	.section	.text.unlikely
.LCOLDB17:
	.text
.LHOTB17:
	.p2align 4,,15
	.globl	__outws
	.type	__outws, @function
__outws:
.LFB17:
	.loc 1 627 0
	.cfi_startproc
.LVL38:
	.loc 1 630 0
#APP
# 630 "arch_x86.c" 1
	.code32			  				
	 pushl %ebp                        
	 movl %esp, %ebp                  
	 pushl %ecx                           
	 pushl %edx                           
	 pushl %esi                           
	 movw 0x10(%ebp), %dx			
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
.LFE17:
	.size	__outws, .-__outws
	.section	.text.unlikely
.LCOLDE17:
	.text
.LHOTE17:
	.section	.text.unlikely
.LCOLDB18:
	.text
.LHOTB18:
	.p2align 4,,15
	.globl	uname
	.type	uname, @function
uname:
.LFB18:
	.loc 1 671 0
	.cfi_startproc
.LVL39:
	.loc 1 671 0
	movl	4(%esp), %eax
	.loc 1 673 0
	movl	$83, %edx
	movl	$1819043144, (%eax)
	movl	$1327519855, 4(%eax)
	movw	%dx, 8(%eax)
	.loc 1 674 0
	movl	$538976288, 195(%eax)
	movl	$1819043144, 199(%eax)
	movl	$1444960367, 203(%eax)
	movl	$943140401, 207(%eax)
	movb	$0, 211(%eax)
	.loc 1 675 0
	movl	$842219849, 260(%eax)
	movb	$0, 264(%eax)
	.loc 1 676 0
	movl	$538976288, 130(%eax)
	movl	$1819043144, 134(%eax)
	movl	$1444960367, 138(%eax)
	movl	$943140401, 142(%eax)
	movb	$0, 146(%eax)
	.loc 1 678 0
	xorl	%eax, %eax
	ret
	.cfi_endproc
.LFE18:
	.size	uname, .-uname
	.section	.text.unlikely
.LCOLDE18:
	.text
.LHOTE18:
	.local	dwTmpEbp
	.comm	dwTmpEbp,4,4
	.local	dwTmpEax
	.comm	dwTmpEax,4,4
	.local	dwTmpEip
	.comm	dwTmpEip,4,4
.Letext0:
	.section	.text.unlikely
.Letext_cold0:
	.file 2 "../../../kernel/include/types.h"
	.file 3 "../../../kernel/include/commobj.h"
	.file 4 "../../../kernel/include/objqueue.h"
	.file 5 "../../../kernel/include/ktmgr.h"
	.file 6 "../../../kernel/include/../arch/x86/arch.h"
	.file 7 "/usr/include/sys/utsname.h"
	.section	.debug_info,"",@progbits
.Ldebug_info0:
	.long	0xc3a
	.value	0x4
	.long	.Ldebug_abbrev0
	.byte	0x4
	.uleb128 0x1
	.long	.LASF146
	.byte	0x1
	.long	.LASF147
	.long	.LASF148
	.long	.Ldebug_ranges0+0
	.long	0
	.long	.Ldebug_line0
	.uleb128 0x2
	.long	.LASF0
	.byte	0x2
	.byte	0x20
	.long	0x30
	.uleb128 0x3
	.byte	0x1
	.byte	0x8
	.long	.LASF2
	.uleb128 0x2
	.long	.LASF1
	.byte	0x2
	.byte	0x21
	.long	0x42
	.uleb128 0x3
	.byte	0x2
	.byte	0x7
	.long	.LASF3
	.uleb128 0x2
	.long	.LASF4
	.byte	0x2
	.byte	0x22
	.long	0x54
	.uleb128 0x3
	.byte	0x4
	.byte	0x7
	.long	.LASF5
	.uleb128 0x2
	.long	.LASF6
	.byte	0x2
	.byte	0x23
	.long	0x54
	.uleb128 0x3
	.byte	0x1
	.byte	0x6
	.long	.LASF7
	.uleb128 0x3
	.byte	0x2
	.byte	0x5
	.long	.LASF8
	.uleb128 0x4
	.byte	0x4
	.byte	0x5
	.string	"int"
	.uleb128 0x3
	.byte	0x4
	.byte	0x7
	.long	.LASF9
	.uleb128 0x3
	.byte	0x8
	.byte	0x4
	.long	.LASF10
	.uleb128 0x3
	.byte	0x4
	.byte	0x4
	.long	.LASF11
	.uleb128 0x2
	.long	.LASF12
	.byte	0x2
	.byte	0x39
	.long	0x9b
	.uleb128 0x5
	.byte	0x4
	.uleb128 0x6
	.byte	0x8
	.byte	0x2
	.byte	0x5f
	.long	0xbe
	.uleb128 0x7
	.long	.LASF13
	.byte	0x2
	.byte	0x60
	.long	0x54
	.byte	0
	.uleb128 0x7
	.long	.LASF14
	.byte	0x2
	.byte	0x61
	.long	0x54
	.byte	0x4
	.byte	0
	.uleb128 0x2
	.long	.LASF15
	.byte	0x2
	.byte	0x62
	.long	0x9d
	.uleb128 0x8
	.long	.LASF26
	.byte	0x20
	.byte	0x3
	.byte	0x1a
	.long	0x136
	.uleb128 0x7
	.long	.LASF16
	.byte	0x3
	.byte	0x1b
	.long	0x49
	.byte	0
	.uleb128 0x7
	.long	.LASF17
	.byte	0x3
	.byte	0x1c
	.long	0x49
	.byte	0x4
	.uleb128 0x7
	.long	.LASF18
	.byte	0x3
	.byte	0x1d
	.long	0x49
	.byte	0x8
	.uleb128 0x7
	.long	.LASF19
	.byte	0x3
	.byte	0x1e
	.long	0x136
	.byte	0xc
	.uleb128 0x7
	.long	.LASF20
	.byte	0x3
	.byte	0x1f
	.long	0x136
	.byte	0x10
	.uleb128 0x7
	.long	.LASF21
	.byte	0x3
	.byte	0x20
	.long	0x136
	.byte	0x14
	.uleb128 0x7
	.long	.LASF22
	.byte	0x3
	.byte	0x23
	.long	0x14b
	.byte	0x18
	.uleb128 0x7
	.long	.LASF23
	.byte	0x3
	.byte	0x24
	.long	0x15c
	.byte	0x1c
	.byte	0
	.uleb128 0x9
	.byte	0x4
	.long	0xc9
	.uleb128 0xa
	.long	0x5b
	.long	0x14b
	.uleb128 0xb
	.long	0x136
	.byte	0
	.uleb128 0x9
	.byte	0x4
	.long	0x13c
	.uleb128 0xc
	.long	0x15c
	.uleb128 0xb
	.long	0x136
	.byte	0
	.uleb128 0x9
	.byte	0x4
	.long	0x151
	.uleb128 0x2
	.long	.LASF24
	.byte	0x3
	.byte	0x25
	.long	0xc9
	.uleb128 0xa
	.long	0x5b
	.long	0x17c
	.uleb128 0xb
	.long	0x17c
	.byte	0
	.uleb128 0x9
	.byte	0x4
	.long	0x162
	.uleb128 0x9
	.byte	0x4
	.long	0x16d
	.uleb128 0xc
	.long	0x193
	.uleb128 0xb
	.long	0x17c
	.byte	0
	.uleb128 0x9
	.byte	0x4
	.long	0x188
	.uleb128 0x3
	.byte	0x4
	.byte	0x7
	.long	.LASF25
	.uleb128 0x8
	.long	.LASF27
	.byte	0x10
	.byte	0x4
	.byte	0x17
	.long	0x1dd
	.uleb128 0x7
	.long	.LASF28
	.byte	0x4
	.byte	0x18
	.long	0x17c
	.byte	0
	.uleb128 0x7
	.long	.LASF29
	.byte	0x4
	.byte	0x19
	.long	0x49
	.byte	0x4
	.uleb128 0x7
	.long	.LASF30
	.byte	0x4
	.byte	0x1a
	.long	0x1dd
	.byte	0x8
	.uleb128 0x7
	.long	.LASF31
	.byte	0x4
	.byte	0x1b
	.long	0x1dd
	.byte	0xc
	.byte	0
	.uleb128 0x9
	.byte	0x4
	.long	0x1a0
	.uleb128 0x2
	.long	.LASF32
	.byte	0x4
	.byte	0x1c
	.long	0x1a0
	.uleb128 0x8
	.long	.LASF33
	.byte	0x40
	.byte	0x4
	.byte	0x20
	.long	0x297
	.uleb128 0x7
	.long	.LASF16
	.byte	0x4
	.byte	0x21
	.long	0x49
	.byte	0
	.uleb128 0x7
	.long	.LASF17
	.byte	0x4
	.byte	0x21
	.long	0x49
	.byte	0x4
	.uleb128 0x7
	.long	.LASF18
	.byte	0x4
	.byte	0x21
	.long	0x49
	.byte	0x8
	.uleb128 0x7
	.long	.LASF19
	.byte	0x4
	.byte	0x21
	.long	0x17c
	.byte	0xc
	.uleb128 0x7
	.long	.LASF20
	.byte	0x4
	.byte	0x21
	.long	0x17c
	.byte	0x10
	.uleb128 0x7
	.long	.LASF21
	.byte	0x4
	.byte	0x21
	.long	0x17c
	.byte	0x14
	.uleb128 0x7
	.long	.LASF22
	.byte	0x4
	.byte	0x21
	.long	0x182
	.byte	0x18
	.uleb128 0x7
	.long	.LASF23
	.byte	0x4
	.byte	0x21
	.long	0x193
	.byte	0x1c
	.uleb128 0x7
	.long	.LASF34
	.byte	0x4
	.byte	0x22
	.long	0x1e3
	.byte	0x20
	.uleb128 0x7
	.long	.LASF35
	.byte	0x4
	.byte	0x23
	.long	0x49
	.byte	0x30
	.uleb128 0x7
	.long	.LASF36
	.byte	0x4
	.byte	0x24
	.long	0x2b0
	.byte	0x34
	.uleb128 0x7
	.long	.LASF37
	.byte	0x4
	.byte	0x29
	.long	0x2ca
	.byte	0x38
	.uleb128 0x7
	.long	.LASF38
	.byte	0x4
	.byte	0x2d
	.long	0x2ea
	.byte	0x3c
	.byte	0
	.uleb128 0xa
	.long	0x5b
	.long	0x2b0
	.uleb128 0xb
	.long	0x17c
	.uleb128 0xb
	.long	0x17c
	.uleb128 0xb
	.long	0x49
	.byte	0
	.uleb128 0x9
	.byte	0x4
	.long	0x297
	.uleb128 0xa
	.long	0x5b
	.long	0x2ca
	.uleb128 0xb
	.long	0x17c
	.uleb128 0xb
	.long	0x17c
	.byte	0
	.uleb128 0x9
	.byte	0x4
	.long	0x2b6
	.uleb128 0xa
	.long	0x17c
	.long	0x2e4
	.uleb128 0xb
	.long	0x17c
	.uleb128 0xb
	.long	0x2e4
	.byte	0
	.uleb128 0x9
	.byte	0x4
	.long	0x49
	.uleb128 0x9
	.byte	0x4
	.long	0x2d0
	.uleb128 0x2
	.long	.LASF39
	.byte	0x4
	.byte	0x31
	.long	0x1ee
	.uleb128 0x8
	.long	.LASF40
	.byte	0x2c
	.byte	0x5
	.byte	0x34
	.long	0x398
	.uleb128 0x7
	.long	.LASF41
	.byte	0x5
	.byte	0x35
	.long	0x49
	.byte	0
	.uleb128 0xd
	.string	"wCS"
	.byte	0x5
	.byte	0x36
	.long	0x37
	.byte	0x4
	.uleb128 0x7
	.long	.LASF42
	.byte	0x5
	.byte	0x37
	.long	0x37
	.byte	0x6
	.uleb128 0x7
	.long	.LASF43
	.byte	0x5
	.byte	0x38
	.long	0x49
	.byte	0x8
	.uleb128 0x7
	.long	.LASF44
	.byte	0x5
	.byte	0x39
	.long	0x49
	.byte	0xc
	.uleb128 0x7
	.long	.LASF45
	.byte	0x5
	.byte	0x3a
	.long	0x49
	.byte	0x10
	.uleb128 0x7
	.long	.LASF46
	.byte	0x5
	.byte	0x3b
	.long	0x49
	.byte	0x14
	.uleb128 0x7
	.long	.LASF47
	.byte	0x5
	.byte	0x3c
	.long	0x49
	.byte	0x18
	.uleb128 0x7
	.long	.LASF48
	.byte	0x5
	.byte	0x3d
	.long	0x49
	.byte	0x1c
	.uleb128 0x7
	.long	.LASF49
	.byte	0x5
	.byte	0x3e
	.long	0x49
	.byte	0x20
	.uleb128 0x7
	.long	.LASF50
	.byte	0x5
	.byte	0x3f
	.long	0x49
	.byte	0x24
	.uleb128 0x7
	.long	.LASF51
	.byte	0x5
	.byte	0x40
	.long	0x49
	.byte	0x28
	.byte	0
	.uleb128 0x2
	.long	.LASF52
	.byte	0x5
	.byte	0x41
	.long	0x2fb
	.uleb128 0x8
	.long	.LASF53
	.byte	0x8
	.byte	0x5
	.byte	0x81
	.long	0x3d4
	.uleb128 0x7
	.long	.LASF54
	.byte	0x5
	.byte	0x82
	.long	0x37
	.byte	0
	.uleb128 0x7
	.long	.LASF55
	.byte	0x5
	.byte	0x83
	.long	0x37
	.byte	0x2
	.uleb128 0x7
	.long	.LASF56
	.byte	0x5
	.byte	0x84
	.long	0x49
	.byte	0x4
	.byte	0
	.uleb128 0x2
	.long	.LASF57
	.byte	0x5
	.byte	0x86
	.long	0x3a3
	.uleb128 0xe
	.long	.LASF58
	.value	0x204
	.byte	0x5
	.byte	0xd0
	.long	0x5f6
	.uleb128 0x7
	.long	.LASF16
	.byte	0x5
	.byte	0xd1
	.long	0x49
	.byte	0
	.uleb128 0x7
	.long	.LASF17
	.byte	0x5
	.byte	0xd1
	.long	0x49
	.byte	0x4
	.uleb128 0x7
	.long	.LASF18
	.byte	0x5
	.byte	0xd1
	.long	0x49
	.byte	0x8
	.uleb128 0x7
	.long	.LASF19
	.byte	0x5
	.byte	0xd1
	.long	0x17c
	.byte	0xc
	.uleb128 0x7
	.long	.LASF20
	.byte	0x5
	.byte	0xd1
	.long	0x17c
	.byte	0x10
	.uleb128 0x7
	.long	.LASF21
	.byte	0x5
	.byte	0xd1
	.long	0x17c
	.byte	0x14
	.uleb128 0x7
	.long	.LASF22
	.byte	0x5
	.byte	0xd1
	.long	0x182
	.byte	0x18
	.uleb128 0x7
	.long	.LASF23
	.byte	0x5
	.byte	0xd1
	.long	0x193
	.byte	0x1c
	.uleb128 0x7
	.long	.LASF59
	.byte	0x5
	.byte	0xd2
	.long	0x605
	.byte	0x20
	.uleb128 0x7
	.long	.LASF60
	.byte	0x5
	.byte	0xd2
	.long	0x49
	.byte	0x24
	.uleb128 0x7
	.long	.LASF61
	.byte	0x5
	.byte	0xd3
	.long	0x398
	.byte	0x28
	.uleb128 0x7
	.long	.LASF62
	.byte	0x5
	.byte	0xd4
	.long	0x60b
	.byte	0x54
	.uleb128 0x7
	.long	.LASF63
	.byte	0x5
	.byte	0xd5
	.long	0x49
	.byte	0x58
	.uleb128 0x7
	.long	.LASF64
	.byte	0x5
	.byte	0xd6
	.long	0x17c
	.byte	0x5c
	.uleb128 0x7
	.long	.LASF65
	.byte	0x5
	.byte	0xd7
	.long	0x49
	.byte	0x60
	.uleb128 0x7
	.long	.LASF66
	.byte	0x5
	.byte	0xd9
	.long	0x611
	.byte	0x64
	.uleb128 0x7
	.long	.LASF67
	.byte	0x5
	.byte	0xe1
	.long	0x49
	.byte	0x68
	.uleb128 0x7
	.long	.LASF68
	.byte	0x5
	.byte	0xe2
	.long	0x49
	.byte	0x6c
	.uleb128 0x7
	.long	.LASF69
	.byte	0x5
	.byte	0xe4
	.long	0x49
	.byte	0x70
	.uleb128 0x7
	.long	.LASF70
	.byte	0x5
	.byte	0xe5
	.long	0x49
	.byte	0x74
	.uleb128 0x7
	.long	.LASF71
	.byte	0x5
	.byte	0xe6
	.long	0x49
	.byte	0x78
	.uleb128 0x7
	.long	.LASF72
	.byte	0x5
	.byte	0xe7
	.long	0x90
	.byte	0x7c
	.uleb128 0x7
	.long	.LASF73
	.byte	0x5
	.byte	0xe8
	.long	0x90
	.byte	0x80
	.uleb128 0x7
	.long	.LASF74
	.byte	0x5
	.byte	0xe9
	.long	0x5b
	.byte	0x84
	.uleb128 0x7
	.long	.LASF75
	.byte	0x5
	.byte	0xea
	.long	0x49
	.byte	0x88
	.uleb128 0x7
	.long	.LASF76
	.byte	0x5
	.byte	0xeb
	.long	0x90
	.byte	0x8c
	.uleb128 0x7
	.long	.LASF77
	.byte	0x5
	.byte	0xec
	.long	0x626
	.byte	0x90
	.uleb128 0x7
	.long	.LASF78
	.byte	0x5
	.byte	0xed
	.long	0x90
	.byte	0x94
	.uleb128 0x7
	.long	.LASF79
	.byte	0x5
	.byte	0xf1
	.long	0x62c
	.byte	0x98
	.uleb128 0xf
	.long	.LASF80
	.byte	0x5
	.byte	0xf2
	.long	0x63c
	.value	0x198
	.uleb128 0xf
	.long	.LASF81
	.byte	0x5
	.byte	0xf3
	.long	0x63c
	.value	0x199
	.uleb128 0xf
	.long	.LASF82
	.byte	0x5
	.byte	0xf4
	.long	0x63c
	.value	0x19a
	.uleb128 0xf
	.long	.LASF83
	.byte	0x5
	.byte	0xf5
	.long	0x30
	.value	0x19b
	.uleb128 0xf
	.long	.LASF84
	.byte	0x5
	.byte	0xf6
	.long	0x611
	.value	0x19c
	.uleb128 0xf
	.long	.LASF85
	.byte	0x5
	.byte	0xf8
	.long	0x49
	.value	0x1a0
	.uleb128 0xf
	.long	.LASF86
	.byte	0x5
	.byte	0xf9
	.long	0x49
	.value	0x1a4
	.uleb128 0xf
	.long	.LASF87
	.byte	0x5
	.byte	0xfa
	.long	0x641
	.value	0x1a8
	.uleb128 0xf
	.long	.LASF88
	.byte	0x5
	.byte	0xfd
	.long	0x651
	.value	0x1c8
	.uleb128 0x10
	.long	.LASF89
	.byte	0x5
	.value	0x100
	.long	0x651
	.value	0x1cc
	.uleb128 0x10
	.long	.LASF90
	.byte	0x5
	.value	0x102
	.long	0x656
	.value	0x1d0
	.uleb128 0x11
	.string	"TLS"
	.byte	0x5
	.value	0x105
	.long	0x666
	.value	0x1f0
	.uleb128 0x10
	.long	.LASF91
	.byte	0x5
	.value	0x10e
	.long	0x651
	.value	0x200
	.byte	0
	.uleb128 0xa
	.long	0x49
	.long	0x605
	.uleb128 0xb
	.long	0x17c
	.byte	0
	.uleb128 0x9
	.byte	0x4
	.long	0x5f6
	.uleb128 0x9
	.byte	0x4
	.long	0x398
	.uleb128 0x9
	.byte	0x4
	.long	0x2f0
	.uleb128 0xa
	.long	0x49
	.long	0x626
	.uleb128 0xb
	.long	0x90
	.byte	0
	.uleb128 0x9
	.byte	0x4
	.long	0x617
	.uleb128 0x12
	.long	0x3d4
	.long	0x63c
	.uleb128 0x13
	.long	0x199
	.byte	0x1f
	.byte	0
	.uleb128 0x14
	.long	0x30
	.uleb128 0x12
	.long	0x30
	.long	0x651
	.uleb128 0x13
	.long	0x199
	.byte	0x1f
	.byte	0
	.uleb128 0x14
	.long	0x49
	.uleb128 0x12
	.long	0x17c
	.long	0x666
	.uleb128 0x13
	.long	0x199
	.byte	0x7
	.byte	0
	.uleb128 0x12
	.long	0x90
	.long	0x676
	.uleb128 0x13
	.long	0x199
	.byte	0x3
	.byte	0
	.uleb128 0x15
	.long	.LASF92
	.byte	0x5
	.value	0x110
	.long	0x3df
	.uleb128 0x9
	.byte	0x4
	.long	0x676
	.uleb128 0x2
	.long	.LASF93
	.byte	0x6
	.byte	0x18
	.long	0x193
	.uleb128 0xe
	.long	.LASF94
	.value	0x186
	.byte	0x7
	.byte	0x30
	.long	0x6eb
	.uleb128 0x7
	.long	.LASF95
	.byte	0x7
	.byte	0x33
	.long	0x6eb
	.byte	0
	.uleb128 0x7
	.long	.LASF96
	.byte	0x7
	.byte	0x36
	.long	0x6eb
	.byte	0x41
	.uleb128 0x7
	.long	.LASF97
	.byte	0x7
	.byte	0x39
	.long	0x6eb
	.byte	0x82
	.uleb128 0x7
	.long	.LASF98
	.byte	0x7
	.byte	0x3b
	.long	0x6eb
	.byte	0xc3
	.uleb128 0xf
	.long	.LASF99
	.byte	0x7
	.byte	0x3e
	.long	0x6eb
	.value	0x104
	.uleb128 0xf
	.long	.LASF100
	.byte	0x7
	.byte	0x45
	.long	0x6eb
	.value	0x145
	.byte	0
	.uleb128 0x12
	.long	0x66
	.long	0x6fb
	.uleb128 0x13
	.long	0x199
	.byte	0x40
	.byte	0
	.uleb128 0x16
	.long	.LASF142
	.byte	0x1
	.value	0x112
	.byte	0x1
	.long	0x715
	.uleb128 0x17
	.long	.LASF149
	.byte	0x1
	.value	0x112
	.long	0x715
	.byte	0
	.uleb128 0x9
	.byte	0x4
	.long	0xbe
	.uleb128 0x18
	.long	.LASF150
	.byte	0x1
	.value	0x136
	.long	0x74
	.long	.LFB7
	.long	.LFE7-.LFB7
	.uleb128 0x1
	.byte	0x9c
	.long	0x754
	.uleb128 0x19
	.long	.LASF101
	.byte	0x1
	.value	0x136
	.long	0x754
	.long	.LLST0
	.uleb128 0x1a
	.long	.LASF102
	.byte	0x1
	.value	0x136
	.long	0x25
	.uleb128 0x1
	.byte	0x52
	.byte	0
	.uleb128 0x9
	.byte	0x4
	.long	0x37
	.uleb128 0x1b
	.long	.LASF108
	.byte	0x1
	.byte	0x28
	.long	0x5b
	.long	.LFB0
	.long	.LFE0-.LFB0
	.uleb128 0x1
	.byte	0x9c
	.uleb128 0x1c
	.long	.LASF104
	.byte	0x1
	.byte	0x34
	.long	.LFB1
	.long	.LFE1-.LFB1
	.uleb128 0x1
	.byte	0x9c
	.long	0x793
	.uleb128 0x1d
	.long	.LASF103
	.byte	0x1
	.byte	0x34
	.long	0x60b
	.uleb128 0x2
	.byte	0x91
	.sleb128 0
	.byte	0
	.uleb128 0x1c
	.long	.LASF105
	.byte	0x1
	.byte	0x6c
	.long	.LFB2
	.long	.LFE2-.LFB2
	.uleb128 0x1
	.byte	0x9c
	.long	0x7c5
	.uleb128 0x1d
	.long	.LASF106
	.byte	0x1
	.byte	0x6c
	.long	0x7c5
	.uleb128 0x2
	.byte	0x91
	.sleb128 0
	.uleb128 0x1d
	.long	.LASF107
	.byte	0x1
	.byte	0x6d
	.long	0x7c5
	.uleb128 0x2
	.byte	0x91
	.sleb128 4
	.byte	0
	.uleb128 0x9
	.byte	0x4
	.long	0x60b
	.uleb128 0x1e
	.long	.LASF109
	.byte	0x1
	.byte	0xbe
	.long	.LFB3
	.long	.LFE3-.LFB3
	.uleb128 0x1
	.byte	0x9c
	.uleb128 0x1f
	.long	.LASF151
	.byte	0x1
	.byte	0xdc
	.long	.LFB4
	.long	.LFE4-.LFB4
	.uleb128 0x1
	.byte	0x9c
	.uleb128 0x1c
	.long	.LASF110
	.byte	0x1
	.byte	0xeb
	.long	.LFB5
	.long	.LFE5-.LFB5
	.uleb128 0x1
	.byte	0x9c
	.long	0x83c
	.uleb128 0x20
	.long	.LASF111
	.byte	0x1
	.byte	0xeb
	.long	0x682
	.long	.LLST1
	.uleb128 0x20
	.long	.LASF112
	.byte	0x1
	.byte	0xec
	.long	0x688
	.long	.LLST2
	.uleb128 0x21
	.long	.LASF113
	.byte	0x1
	.byte	0xee
	.long	0x2e4
	.long	.LLST3
	.uleb128 0x22
	.long	.LASF75
	.byte	0x1
	.byte	0xef
	.long	0x49
	.byte	0
	.byte	0
	.uleb128 0x23
	.long	0x6fb
	.long	.LFB6
	.long	.LFE6-.LFB6
	.uleb128 0x1
	.byte	0x9c
	.long	0x858
	.uleb128 0x24
	.long	0x708
	.uleb128 0x2
	.byte	0x91
	.sleb128 0
	.byte	0
	.uleb128 0x25
	.long	.LASF114
	.byte	0x1
	.value	0x167
	.long	.LFB8
	.long	.LFE8-.LFB8
	.uleb128 0x1
	.byte	0x9c
	.long	0x920
	.uleb128 0x1a
	.long	.LASF115
	.byte	0x1
	.value	0x167
	.long	0x920
	.uleb128 0x2
	.byte	0x91
	.sleb128 0
	.uleb128 0x26
	.long	.LVL13
	.long	0x71b
	.long	0x897
	.uleb128 0x27
	.uleb128 0x1
	.byte	0x50
	.uleb128 0x3
	.byte	0x91
	.sleb128 0
	.byte	0x6
	.uleb128 0x27
	.uleb128 0x1
	.byte	0x52
	.uleb128 0x1
	.byte	0x39
	.byte	0
	.uleb128 0x26
	.long	.LVL14
	.long	0x71b
	.long	0x8b3
	.uleb128 0x27
	.uleb128 0x1
	.byte	0x50
	.uleb128 0x5
	.byte	0x91
	.sleb128 0
	.byte	0x6
	.byte	0x23
	.uleb128 0x1
	.uleb128 0x27
	.uleb128 0x1
	.byte	0x52
	.uleb128 0x1
	.byte	0x38
	.byte	0
	.uleb128 0x26
	.long	.LVL15
	.long	0x71b
	.long	0x8cf
	.uleb128 0x27
	.uleb128 0x1
	.byte	0x50
	.uleb128 0x5
	.byte	0x91
	.sleb128 0
	.byte	0x6
	.byte	0x23
	.uleb128 0x2
	.uleb128 0x27
	.uleb128 0x1
	.byte	0x52
	.uleb128 0x1
	.byte	0x37
	.byte	0
	.uleb128 0x26
	.long	.LVL16
	.long	0x71b
	.long	0x8eb
	.uleb128 0x27
	.uleb128 0x1
	.byte	0x50
	.uleb128 0x5
	.byte	0x91
	.sleb128 0
	.byte	0x6
	.byte	0x23
	.uleb128 0x3
	.uleb128 0x27
	.uleb128 0x1
	.byte	0x52
	.uleb128 0x1
	.byte	0x34
	.byte	0
	.uleb128 0x26
	.long	.LVL17
	.long	0x71b
	.long	0x907
	.uleb128 0x27
	.uleb128 0x1
	.byte	0x50
	.uleb128 0x5
	.byte	0x91
	.sleb128 0
	.byte	0x6
	.byte	0x23
	.uleb128 0x4
	.uleb128 0x27
	.uleb128 0x1
	.byte	0x52
	.uleb128 0x1
	.byte	0x32
	.byte	0
	.uleb128 0x28
	.long	.LVL18
	.long	0x71b
	.uleb128 0x27
	.uleb128 0x1
	.byte	0x50
	.uleb128 0x5
	.byte	0x91
	.sleb128 0
	.byte	0x6
	.byte	0x23
	.uleb128 0x5
	.uleb128 0x27
	.uleb128 0x1
	.byte	0x52
	.uleb128 0x1
	.byte	0x30
	.byte	0
	.byte	0
	.uleb128 0x9
	.byte	0x4
	.long	0x25
	.uleb128 0x25
	.long	.LASF116
	.byte	0x1
	.value	0x17c
	.long	.LFB9
	.long	.LFE9-.LFB9
	.uleb128 0x1
	.byte	0x9c
	.long	0x9c1
	.uleb128 0x1a
	.long	.LASF117
	.byte	0x1
	.value	0x17c
	.long	0x49
	.uleb128 0x2
	.byte	0x91
	.sleb128 0
	.uleb128 0x29
	.long	.LASF118
	.byte	0x1
	.value	0x17e
	.long	0xbe
	.uleb128 0x2
	.byte	0x91
	.sleb128 -32
	.uleb128 0x29
	.long	.LASF119
	.byte	0x1
	.value	0x17f
	.long	0xbe
	.uleb128 0x2
	.byte	0x91
	.sleb128 -24
	.uleb128 0x2a
	.long	0x6fb
	.long	.LBB6
	.long	.LBE6-.LBB6
	.byte	0x1
	.value	0x181
	.long	0x987
	.uleb128 0x2b
	.long	0x708
	.long	.LLST4
	.byte	0
	.uleb128 0x2a
	.long	0x6fb
	.long	.LBB8
	.long	.LBE8-.LBB8
	.byte	0x1
	.value	0x189
	.long	0x9a5
	.uleb128 0x2b
	.long	0x708
	.long	.LLST5
	.byte	0
	.uleb128 0x2c
	.long	.LVL21
	.long	0xbf6
	.uleb128 0x2c
	.long	.LVL23
	.long	0xc0c
	.uleb128 0x2c
	.long	.LVL25
	.long	0xc27
	.byte	0
	.uleb128 0x25
	.long	.LASF120
	.byte	0x1
	.value	0x192
	.long	.LFB10
	.long	.LFE10-.LFB10
	.uleb128 0x1
	.byte	0x9c
	.long	0x9f6
	.uleb128 0x1a
	.long	.LASF121
	.byte	0x1
	.value	0x192
	.long	0x37
	.uleb128 0x2
	.byte	0x91
	.sleb128 0
	.uleb128 0x1a
	.long	.LASF122
	.byte	0x1
	.value	0x192
	.long	0x49
	.uleb128 0x2
	.byte	0x91
	.sleb128 4
	.byte	0
	.uleb128 0x2d
	.long	.LASF126
	.byte	0x1
	.value	0x1ae
	.long	0x49
	.long	.LFB11
	.long	.LFE11-.LFB11
	.uleb128 0x1
	.byte	0x9c
	.long	0xa30
	.uleb128 0x1a
	.long	.LASF121
	.byte	0x1
	.value	0x1ae
	.long	0x37
	.uleb128 0x2
	.byte	0x91
	.sleb128 0
	.uleb128 0x2e
	.long	.LASF123
	.byte	0x1
	.value	0x1b0
	.long	0x49
	.long	.LLST6
	.byte	0
	.uleb128 0x25
	.long	.LASF124
	.byte	0x1
	.value	0x1cc
	.long	.LFB12
	.long	.LFE12-.LFB12
	.uleb128 0x1
	.byte	0x9c
	.long	0xa65
	.uleb128 0x2f
	.string	"_bt"
	.byte	0x1
	.value	0x1cc
	.long	0x30
	.uleb128 0x2
	.byte	0x91
	.sleb128 0
	.uleb128 0x1a
	.long	.LASF125
	.byte	0x1
	.value	0x1cc
	.long	0x37
	.uleb128 0x2
	.byte	0x91
	.sleb128 4
	.byte	0
	.uleb128 0x2d
	.long	.LASF127
	.byte	0x1
	.value	0x1e8
	.long	0x30
	.long	.LFB13
	.long	.LFE13-.LFB13
	.uleb128 0x1
	.byte	0x9c
	.long	0xa9d
	.uleb128 0x1a
	.long	.LASF125
	.byte	0x1
	.value	0x1e8
	.long	0x37
	.uleb128 0x2
	.byte	0x91
	.sleb128 0
	.uleb128 0x29
	.long	.LASF128
	.byte	0x1
	.value	0x1ea
	.long	0x30
	.uleb128 0x1
	.byte	0x50
	.byte	0
	.uleb128 0x2d
	.long	.LASF129
	.byte	0x1
	.value	0x204
	.long	0x37
	.long	.LFB14
	.long	.LFE14-.LFB14
	.uleb128 0x1
	.byte	0x9c
	.long	0xad7
	.uleb128 0x1a
	.long	.LASF121
	.byte	0x1
	.value	0x204
	.long	0x37
	.uleb128 0x2
	.byte	0x91
	.sleb128 0
	.uleb128 0x2e
	.long	.LASF130
	.byte	0x1
	.value	0x206
	.long	0x37
	.long	.LLST7
	.byte	0
	.uleb128 0x25
	.long	.LASF131
	.byte	0x1
	.value	0x222
	.long	.LFB15
	.long	.LFE15-.LFB15
	.uleb128 0x1
	.byte	0x9c
	.long	0xb0c
	.uleb128 0x1a
	.long	.LASF132
	.byte	0x1
	.value	0x222
	.long	0x37
	.uleb128 0x2
	.byte	0x91
	.sleb128 0
	.uleb128 0x1a
	.long	.LASF121
	.byte	0x1
	.value	0x222
	.long	0x37
	.uleb128 0x2
	.byte	0x91
	.sleb128 4
	.byte	0
	.uleb128 0x25
	.long	.LASF133
	.byte	0x1
	.value	0x240
	.long	.LFB16
	.long	.LFE16-.LFB16
	.uleb128 0x1
	.byte	0x9c
	.long	0xb50
	.uleb128 0x1a
	.long	.LASF134
	.byte	0x1
	.value	0x240
	.long	0x920
	.uleb128 0x2
	.byte	0x91
	.sleb128 0
	.uleb128 0x1a
	.long	.LASF135
	.byte	0x1
	.value	0x240
	.long	0x49
	.uleb128 0x2
	.byte	0x91
	.sleb128 4
	.uleb128 0x1a
	.long	.LASF121
	.byte	0x1
	.value	0x240
	.long	0x37
	.uleb128 0x2
	.byte	0x91
	.sleb128 8
	.byte	0
	.uleb128 0x25
	.long	.LASF136
	.byte	0x1
	.value	0x272
	.long	.LFB17
	.long	.LFE17-.LFB17
	.uleb128 0x1
	.byte	0x9c
	.long	0xb94
	.uleb128 0x1a
	.long	.LASF134
	.byte	0x1
	.value	0x272
	.long	0x920
	.uleb128 0x2
	.byte	0x91
	.sleb128 0
	.uleb128 0x1a
	.long	.LASF135
	.byte	0x1
	.value	0x272
	.long	0x49
	.uleb128 0x2
	.byte	0x91
	.sleb128 4
	.uleb128 0x1a
	.long	.LASF121
	.byte	0x1
	.value	0x272
	.long	0x37
	.uleb128 0x2
	.byte	0x91
	.sleb128 8
	.byte	0
	.uleb128 0x30
	.long	.LASF137
	.byte	0x7
	.byte	0x51
	.long	0x74
	.long	.LFB18
	.long	.LFE18-.LFB18
	.uleb128 0x1
	.byte	0x9c
	.long	0xbbd
	.uleb128 0x1a
	.long	.LASF138
	.byte	0x1
	.value	0x29e
	.long	0xbbd
	.uleb128 0x2
	.byte	0x91
	.sleb128 0
	.byte	0
	.uleb128 0x9
	.byte	0x4
	.long	0x693
	.uleb128 0x31
	.long	.LASF139
	.byte	0x1
	.byte	0x63
	.long	0x49
	.uleb128 0x5
	.byte	0x3
	.long	dwTmpEip
	.uleb128 0x31
	.long	.LASF140
	.byte	0x1
	.byte	0x64
	.long	0x49
	.uleb128 0x5
	.byte	0x3
	.long	dwTmpEax
	.uleb128 0x31
	.long	.LASF141
	.byte	0x1
	.byte	0x65
	.long	0x49
	.uleb128 0x5
	.byte	0x3
	.long	dwTmpEbp
	.uleb128 0x32
	.long	.LASF143
	.byte	0x2
	.byte	0x7c
	.long	0xc0c
	.uleb128 0xb
	.long	0x715
	.uleb128 0xb
	.long	0x49
	.byte	0
	.uleb128 0x32
	.long	.LASF144
	.byte	0x2
	.byte	0x69
	.long	0xc27
	.uleb128 0xb
	.long	0x715
	.uleb128 0xb
	.long	0x715
	.uleb128 0xb
	.long	0x715
	.byte	0
	.uleb128 0x33
	.long	.LASF145
	.byte	0x2
	.byte	0x76
	.long	0x5b
	.uleb128 0xb
	.long	0x715
	.uleb128 0xb
	.long	0x715
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
	.uleb128 0x55
	.uleb128 0x17
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x10
	.uleb128 0x17
	.byte	0
	.byte	0
	.uleb128 0x2
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
	.uleb128 0x3
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
	.byte	0
	.byte	0
	.uleb128 0x6
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
	.uleb128 0x7
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
	.uleb128 0x8
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
	.uleb128 0x9
	.uleb128 0xf
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0xa
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
	.uleb128 0xb
	.uleb128 0x5
	.byte	0
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0xc
	.uleb128 0x15
	.byte	0x1
	.uleb128 0x27
	.uleb128 0x19
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0xd
	.uleb128 0xd
	.byte	0
	.uleb128 0x3
	.uleb128 0x8
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
	.uleb128 0xe
	.uleb128 0x13
	.byte	0x1
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0xb
	.uleb128 0x5
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0xf
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
	.uleb128 0x5
	.byte	0
	.byte	0
	.uleb128 0x10
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
	.uleb128 0x11
	.uleb128 0xd
	.byte	0
	.uleb128 0x3
	.uleb128 0x8
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
	.uleb128 0x12
	.uleb128 0x1
	.byte	0x1
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x13
	.uleb128 0x21
	.byte	0
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2f
	.uleb128 0xb
	.byte	0
	.byte	0
	.uleb128 0x14
	.uleb128 0x35
	.byte	0
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x15
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
	.uleb128 0x16
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
	.uleb128 0x20
	.uleb128 0xb
	.uleb128 0x1
	.uleb128 0x13
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
	.byte	0
	.byte	0
	.uleb128 0x18
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
	.uleb128 0x19
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
	.uleb128 0x1a
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
	.uleb128 0x1b
	.uleb128 0x2e
	.byte	0
	.uleb128 0x3f
	.uleb128 0x19
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
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
	.uleb128 0x1c
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
	.uleb128 0x1d
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
	.uleb128 0x1e
	.uleb128 0x2e
	.byte	0
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
	.byte	0
	.byte	0
	.uleb128 0x1f
	.uleb128 0x2e
	.byte	0
	.uleb128 0x3f
	.uleb128 0x19
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
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
	.uleb128 0x20
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
	.uleb128 0x17
	.byte	0
	.byte	0
	.uleb128 0x21
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
	.uleb128 0x2
	.uleb128 0x17
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
	.uleb128 0x1c
	.uleb128 0xb
	.byte	0
	.byte	0
	.uleb128 0x23
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x31
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
	.uleb128 0x24
	.uleb128 0x5
	.byte	0
	.uleb128 0x31
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x18
	.byte	0
	.byte	0
	.uleb128 0x25
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
	.uleb128 0x26
	.uleb128 0x4109
	.byte	0x1
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x31
	.uleb128 0x13
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x27
	.uleb128 0x410a
	.byte	0
	.uleb128 0x2
	.uleb128 0x18
	.uleb128 0x2111
	.uleb128 0x18
	.byte	0
	.byte	0
	.uleb128 0x28
	.uleb128 0x4109
	.byte	0x1
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x31
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x29
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
	.uleb128 0x18
	.byte	0
	.byte	0
	.uleb128 0x2a
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
	.uleb128 0x2b
	.uleb128 0x5
	.byte	0
	.uleb128 0x31
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x17
	.byte	0
	.byte	0
	.uleb128 0x2c
	.uleb128 0x4109
	.byte	0
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x31
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x2d
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
	.uleb128 0x2e
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
	.uleb128 0x2f
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
	.uleb128 0x30
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
	.uleb128 0x31
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
	.uleb128 0x2
	.uleb128 0x18
	.byte	0
	.byte	0
	.uleb128 0x32
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
	.uleb128 0x33
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
	.uleb128 0x3c
	.uleb128 0x19
	.byte	0
	.byte	0
	.byte	0
	.section	.debug_loc,"",@progbits
.Ldebug_loc0:
.LLST0:
	.long	.LVL0
	.long	.LVL1
	.value	0x1
	.byte	0x50
	.long	.LVL1
	.long	.LFE7
	.value	0x4
	.byte	0xf3
	.uleb128 0x1
	.byte	0x50
	.byte	0x9f
	.long	0
	.long	0
.LLST1:
	.long	.LVL4
	.long	.LVL10
	.value	0x2
	.byte	0x91
	.sleb128 0
	.long	.LVL10
	.long	.LFE5
	.value	0x1
	.byte	0x52
	.long	0
	.long	0
.LLST2:
	.long	.LVL4
	.long	.LVL10
	.value	0x2
	.byte	0x91
	.sleb128 4
	.long	.LVL10
	.long	.LFE5
	.value	0x1
	.byte	0x51
	.long	0
	.long	0
.LLST3:
	.long	.LVL4
	.long	.LVL5
	.value	0x2
	.byte	0x30
	.byte	0x9f
	.long	.LVL5
	.long	.LVL6
	.value	0x3
	.byte	0x70
	.sleb128 -4
	.byte	0x9f
	.long	.LVL6
	.long	.LVL7
	.value	0x3
	.byte	0x70
	.sleb128 -8
	.byte	0x9f
	.long	.LVL7
	.long	.LVL8
	.value	0x3
	.byte	0x70
	.sleb128 -12
	.byte	0x9f
	.long	.LVL8
	.long	.LVL9
	.value	0xa
	.byte	0x91
	.sleb128 0
	.byte	0x6
	.byte	0x23
	.uleb128 0x8c
	.byte	0x6
	.byte	0x3c
	.byte	0x1c
	.byte	0x9f
	.long	0
	.long	0
.LLST4:
	.long	.LVL20
	.long	.LVL22
	.value	0x3
	.byte	0x91
	.sleb128 -24
	.byte	0x9f
	.long	.LVL22
	.long	.LVL26
	.value	0x1
	.byte	0x53
	.long	.LVL26
	.long	.LFE9
	.value	0x3
	.byte	0x91
	.sleb128 -24
	.byte	0x9f
	.long	0
	.long	0
.LLST5:
	.long	.LVL24
	.long	.LVL27
	.value	0x1
	.byte	0x56
	.long	.LVL27
	.long	.LFE9
	.value	0x3
	.byte	0x91
	.sleb128 -32
	.byte	0x9f
	.long	0
	.long	0
.LLST6:
	.long	.LVL29
	.long	.LVL30
	.value	0x2
	.byte	0x30
	.byte	0x9f
	.long	.LVL30
	.long	.LFE11
	.value	0x1
	.byte	0x50
	.long	0
	.long	0
.LLST7:
	.long	.LVL34
	.long	.LVL35
	.value	0x2
	.byte	0x30
	.byte	0x9f
	.long	.LVL35
	.long	.LFE14
	.value	0x1
	.byte	0x50
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
	.long	.Ltext0
	.long	.Letext0
	.long	0
	.long	0
	.section	.debug_line,"",@progbits
.Ldebug_line0:
	.section	.debug_str,"MS",@progbits,1
.LASF23:
	.string	"Uninitialize"
.LASF133:
	.string	"__inws"
.LASF94:
	.string	"utsname"
.LASF123:
	.string	"dwRet"
.LASF144:
	.string	"u64Add"
.LASF124:
	.string	"__outb"
.LASF120:
	.string	"__outd"
.LASF38:
	.string	"GetHeaderElement"
.LASF61:
	.string	"KernelThreadContext"
.LASF138:
	.string	"__name"
.LASF112:
	.string	"lpStartAddr"
.LASF9:
	.string	"unsigned int"
.LASF98:
	.string	"version"
.LASF82:
	.string	"ucCurrentMsgNum"
.LASF93:
	.string	"__KERNEL_THREAD_WRAPPER"
.LASF109:
	.string	"EnableVMM"
.LASF148:
	.string	"/media/gaojie/Dev/hellox/HelloX_OS/kernel/arch/x86"
.LASF74:
	.string	"bUsedMath"
.LASF56:
	.string	"dwParam"
.LASF134:
	.string	"pBuffer"
.LASF116:
	.string	"__MicroDelay"
.LASF83:
	.string	"ucAligment"
.LASF121:
	.string	"wPort"
.LASF14:
	.string	"dwHighPart"
.LASF34:
	.string	"ElementHeader"
.LASF33:
	.string	"tag__PRIORITY_QUEUE"
.LASF37:
	.string	"DeleteFromQueue"
.LASF119:
	.string	"u64TargetTsc"
.LASF129:
	.string	"__inw"
.LASF108:
	.string	"HardwareInitialize"
.LASF111:
	.string	"lpKernelThread"
.LASF41:
	.string	"dwEFlags"
.LASF11:
	.string	"float"
.LASF128:
	.string	"uRet"
.LASF4:
	.string	"DWORD"
.LASF6:
	.string	"BOOL"
.LASF137:
	.string	"uname"
.LASF151:
	.string	"HaltSystem"
.LASF146:
	.string	"GNU C 4.9.2 20150212 (Red Hat 4.9.2-6) -m32 -mtune=generic -march=i686 -g -O2"
.LASF32:
	.string	"__PRIORITY_QUEUE_ELEMENT"
.LASF117:
	.string	"dwmSeconds"
.LASF64:
	.string	"lpOwnProcess"
.LASF113:
	.string	"lpStackPtr"
.LASF13:
	.string	"dwLowPart"
.LASF57:
	.string	"__KERNEL_THREAD_MESSAGE"
.LASF68:
	.string	"dwScheduleCounter"
.LASF101:
	.string	"pData"
.LASF115:
	.string	"pDate"
.LASF122:
	.string	"dwVal"
.LASF55:
	.string	"wParam"
.LASF19:
	.string	"lpPrevObject"
.LASF58:
	.string	"tag__KERNEL_THREAD_OBJECT"
.LASF145:
	.string	"MoreThan"
.LASF91:
	.string	"dwSuspendFlags"
.LASF79:
	.string	"KernelThreadMsg"
.LASF130:
	.string	"wRet"
.LASF143:
	.string	"u64RotateLeft"
.LASF65:
	.string	"dwThreadStatus"
.LASF102:
	.string	"nPort"
.LASF7:
	.string	"char"
.LASF62:
	.string	"lpKernelThreadContext"
.LASF12:
	.string	"LPVOID"
.LASF88:
	.string	"dwWaitingStatus"
.LASF147:
	.string	"arch_x86.c"
.LASF86:
	.string	"dwLastError"
.LASF17:
	.string	"dwObjectID"
.LASF92:
	.string	"__KERNEL_THREAD_OBJECT"
.LASF114:
	.string	"__GetTime"
.LASF18:
	.string	"dwObjectSize"
.LASF99:
	.string	"machine"
.LASF40:
	.string	"tag__KERNEL_THREAD_CONTEXT"
.LASF44:
	.string	"dwEAX"
.LASF140:
	.string	"dwTmpEax"
.LASF81:
	.string	"ucMsgQueueTrial"
.LASF36:
	.string	"InsertIntoQueue"
.LASF0:
	.string	"BYTE"
.LASF149:
	.string	"lpResult"
.LASF50:
	.string	"dwEBP"
.LASF141:
	.string	"dwTmpEbp"
.LASF45:
	.string	"dwEBX"
.LASF1:
	.string	"WORD"
.LASF67:
	.string	"dwThreadPriority"
.LASF84:
	.string	"lpMsgWaitingQueue"
.LASF105:
	.string	"__SaveAndSwitch"
.LASF66:
	.string	"lpWaitingQueue"
.LASF118:
	.string	"u64CurrTsc"
.LASF142:
	.string	"__GetTsc"
.LASF46:
	.string	"dwECX"
.LASF127:
	.string	"__inb"
.LASF126:
	.string	"__ind"
.LASF132:
	.string	"wVal"
.LASF21:
	.string	"lpObjectOwner"
.LASF28:
	.string	"lpObject"
.LASF48:
	.string	"dwESI"
.LASF51:
	.string	"dwESP"
.LASF107:
	.string	"lppNewContext"
.LASF90:
	.string	"MultipleWaitObjectArray"
.LASF49:
	.string	"dwEDI"
.LASF47:
	.string	"dwEDX"
.LASF8:
	.string	"short int"
.LASF100:
	.string	"__domainname"
.LASF125:
	.string	"_port"
.LASF59:
	.string	"WaitForThisObject"
.LASF136:
	.string	"__outws"
.LASF135:
	.string	"dwBuffLen"
.LASF73:
	.string	"lpDefaultHeap"
.LASF104:
	.string	"__SwitchTo"
.LASF70:
	.string	"dwTotalRunTime"
.LASF16:
	.string	"dwObjectType"
.LASF22:
	.string	"Initialize"
.LASF76:
	.string	"lpInitStackPointer"
.LASF52:
	.string	"__KERNEL_THREAD_CONTEXT"
.LASF78:
	.string	"lpRoutineParam"
.LASF69:
	.string	"dwReturnValue"
.LASF25:
	.string	"sizetype"
.LASF29:
	.string	"dwPriority"
.LASF5:
	.string	"long unsigned int"
.LASF96:
	.string	"nodename"
.LASF71:
	.string	"dwTotalMemSize"
.LASF150:
	.string	"ReadCmosData"
.LASF2:
	.string	"unsigned char"
.LASF131:
	.string	"__outw"
.LASF89:
	.string	"dwMultipleWaitFlags"
.LASF103:
	.string	"lpContext"
.LASF26:
	.string	"tag__COMMON_OBJECT"
.LASF54:
	.string	"wCommand"
.LASF77:
	.string	"KernelThreadRoutine"
.LASF24:
	.string	"__COMMON_OBJECT"
.LASF85:
	.string	"dwUserData"
.LASF80:
	.string	"ucMsgQueueHeader"
.LASF63:
	.string	"dwThreadID"
.LASF53:
	.string	"tag__KERNEL_THREAD_MESSAGE"
.LASF106:
	.string	"lppOldContext"
.LASF87:
	.string	"KernelThreadName"
.LASF3:
	.string	"short unsigned int"
.LASF97:
	.string	"release"
.LASF72:
	.string	"lpHeapObject"
.LASF110:
	.string	"InitKernelThreadContext"
.LASF35:
	.string	"dwCurrElementNum"
.LASF31:
	.string	"lpPrevElement"
.LASF39:
	.string	"__PRIORITY_QUEUE"
.LASF10:
	.string	"double"
.LASF60:
	.string	"dwObjectSignature"
.LASF15:
	.string	"__U64"
.LASF20:
	.string	"lpNextObject"
.LASF30:
	.string	"lpNextElement"
.LASF42:
	.string	"wReserved"
.LASF95:
	.string	"sysname"
.LASF43:
	.string	"dwEIP"
.LASF139:
	.string	"dwTmpEip"
.LASF27:
	.string	"tag__PRIORITY_QUEUE_ELEMENT"
.LASF75:
	.string	"dwStackSize"
	.ident	"GCC: (GNU) 4.9.2 20150212 (Red Hat 4.9.2-6)"
	.section	.note.GNU-stack,"",@progbits
