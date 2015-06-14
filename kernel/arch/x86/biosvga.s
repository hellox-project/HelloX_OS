	.file	"biosvga.c"
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
	.globl	_VGA_GetDisplayAddr
	.type	_VGA_GetDisplayAddr, @function
_VGA_GetDisplayAddr:
.LFB11:
	.file 1 "biosvga.c"
	.loc 1 43 0
	.cfi_startproc
.LVL0:
	.loc 1 46 0
	movzwl	s_szVgaInfo+6, %edx
	.loc 1 43 0
	movl	4(%esp), %eax
	movl	8(%esp), %ecx
	.loc 1 46 0
	cmpw	%ax, %dx
	jbe	.L4
	.loc 1 46 0 is_stmt 0 discriminator 1
	cmpw	%cx, s_szVgaInfo+4
	jbe	.L4
.LVL1:
	.loc 1 51 0 is_stmt 1
	movzwl	%cx, %ecx
	movzwl	%ax, %eax
	imull	%ecx, %edx
.LVL2:
	addl	%edx, %eax
.LVL3:
	.loc 1 53 0
	movl	s_szVgaInfo, %edx
	leal	(%edx,%eax,2), %eax
	ret
.LVL4:
	.p2align 4,,10
	.p2align 3
.L4:
	.loc 1 48 0
	xorl	%eax, %eax
	.loc 1 54 0
	ret
	.cfi_endproc
.LFE11:
	.size	_VGA_GetDisplayAddr, .-_VGA_GetDisplayAddr
	.section	.text.unlikely
.LCOLDE0:
	.text
.LHOTE0:
	.section	.text.unlikely
.LCOLDB1:
	.text
.LHOTB1:
	.p2align 4,,15
	.globl	_VGA_ScrollLine
	.type	_VGA_ScrollLine, @function
_VGA_ScrollLine:
.LFB12:
	.loc 1 57 0
	.cfi_startproc
.LVL5:
	pushl	%edi
	.cfi_def_cfa_offset 8
	.cfi_offset 7, -8
	pushl	%esi
	.cfi_def_cfa_offset 12
	.cfi_offset 6, -12
.LBB17:
.LBB18:
	.loc 1 48 0
	xorl	%edi, %edi
.LBE18:
.LBE17:
	.loc 1 57 0
	pushl	%ebx
	.cfi_def_cfa_offset 16
	.cfi_offset 3, -16
.LBB20:
.LBB19:
	.loc 1 46 0
	movzwl	s_szVgaInfo+6, %ebx
	testw	%bx, %bx
	je	.L7
	.loc 1 53 0
	cmpw	$0, s_szVgaInfo+4
	cmovne	s_szVgaInfo, %edi
.L7:
.LVL6:
.LBE19:
.LBE20:
	.loc 1 62 0
	movl	16(%esp), %esi
	.loc 1 60 0
	movzwl	s_szVgaInfo+8, %eax
.LVL7:
	.loc 1 62 0
	testl	%esi, %esi
	jne	.L19
.LVL8:
.L6:
	.loc 1 77 0
	popl	%ebx
	.cfi_remember_state
	.cfi_restore 3
	.cfi_def_cfa_offset 12
.LVL9:
	popl	%esi
	.cfi_restore 6
	.cfi_def_cfa_offset 8
	popl	%edi
	.cfi_restore 7
	.cfi_def_cfa_offset 4
	ret
.LVL10:
	.p2align 4,,10
	.p2align 3
.L19:
	.cfi_restore_state
	.loc 1 60 0
	movzwl	%ax, %esi
	.loc 1 59 0
	leal	(%edi,%ebx,2), %eax
.LVL11:
.LBB21:
	.loc 1 67 0
	subl	$4, %esp
	.cfi_def_cfa_offset 20
.LBE21:
	.loc 1 60 0
	subl	$1, %esi
	imull	%ebx, %esi
.LVL12:
.LBB22:
	.loc 1 64 0
	addl	%esi, %esi
.LVL13:
	.loc 1 67 0
	pushl	%esi
	.cfi_def_cfa_offset 24
	pushl	%eax
	.cfi_def_cfa_offset 28
	pushl	%edi
	.cfi_def_cfa_offset 32
	call	memcpy
.LVL14:
	.loc 1 68 0
	addl	$16, %esp
	.cfi_def_cfa_offset 16
	testl	%ebx, %ebx
	je	.L6
	leal	(%edi,%esi), %edx
.LBE22:
	.loc 1 60 0
	xorl	%eax, %eax
.LVL15:
	.p2align 4,,10
	.p2align 3
.L10:
.LBB23:
	.loc 1 70 0 discriminator 3
	movl	$1792, %ecx
	movw	%cx, (%edx,%eax,2)
	.loc 1 68 0 discriminator 3
	addl	$1, %eax
.LVL16:
	cmpl	%eax, %ebx
	jne	.L10
.LBE23:
	.loc 1 77 0
	popl	%ebx
	.cfi_restore 3
	.cfi_def_cfa_offset 12
.LVL17:
	popl	%esi
	.cfi_restore 6
	.cfi_def_cfa_offset 8
.LVL18:
	popl	%edi
	.cfi_restore 7
	.cfi_def_cfa_offset 4
	ret
	.cfi_endproc
.LFE12:
	.size	_VGA_ScrollLine, .-_VGA_ScrollLine
	.section	.text.unlikely
.LCOLDE1:
	.text
.LHOTE1:
	.section	.text.unlikely
.LCOLDB2:
	.text
.LHOTB2:
	.p2align 4,,15
	.globl	VGA_GetDisplayID
	.type	VGA_GetDisplayID, @function
VGA_GetDisplayID:
.LFB13:
	.loc 1 80 0
	.cfi_startproc
	.loc 1 82 0
	movl	$4097, %eax
	ret
	.cfi_endproc
.LFE13:
	.size	VGA_GetDisplayID, .-VGA_GetDisplayID
	.section	.text.unlikely
.LCOLDE2:
	.text
.LHOTE2:
	.section	.text.unlikely
.LCOLDB3:
	.text
.LHOTB3:
	.p2align 4,,15
	.globl	VGA_SetCursorPos
	.type	VGA_SetCursorPos, @function
VGA_SetCursorPos:
.LFB14:
	.loc 1 85 0
	.cfi_startproc
.LVL19:
	pushl	%ebx
	.cfi_def_cfa_offset 8
	.cfi_offset 3, -8
	subl	$24, %esp
	.cfi_def_cfa_offset 32
	.loc 1 93 0
	movzwl	s_szVgaInfo+8, %ebx
	.loc 1 85 0
	movl	36(%esp), %eax
	movl	32(%esp), %edx
	.loc 1 91 0
	movb	$0, 15(%esp)
	.loc 1 93 0
	cmpw	%ax, %bx
	jbe	.L25
	.loc 1 102 0
	movw	%dx, s_szVgaInfo+10
	.loc 1 103 0
	movw	%ax, s_szVgaInfo+12
.L23:
	.loc 1 106 0
	leal	(%eax,%eax,4), %eax
	.loc 1 111 0
	movl	$980, %ecx
	.loc 1 106 0
	sall	$4, %eax
	addl	%edx, %eax
.LVL20:
	.loc 1 111 0
	movl	$981, %edx
	.loc 1 108 0
	movb	%al, 15(%esp)
	.loc 1 107 0
	shrw	$8, %ax
.LVL21:
	.loc 1 111 0
#APP
# 111 "biosvga.c" 1
	movw %ecx,	%dx                              
	movb $14,	%al                              
	outb %al,	%dx                              
	movw %edx,	%dx                              
	movb %al,	%al                              
	outw %al,	%dx                              
	movw %ecx,	%dx                              
	movb $15,	%al                              
	outw %al,	%dx                              
	movw %edx,	%dx                              
	movl %edx,	%al                              
	outb %al,	%dx                              
	
# 0 "" 2
	.loc 1 150 0
#NO_APP
	addl	$24, %esp
	.cfi_remember_state
	.cfi_def_cfa_offset 8
	movl	$1, %eax
	popl	%ebx
	.cfi_restore 3
	.cfi_def_cfa_offset 4
	ret
.LVL22:
	.p2align 4,,10
	.p2align 3
.L25:
	.cfi_restore_state
	.loc 1 95 0
	subl	$12, %esp
	.cfi_def_cfa_offset 44
	pushl	$1
	.cfi_def_cfa_offset 48
	call	_VGA_ScrollLine
.LVL23:
	.loc 1 97 0
	xorl	%eax, %eax
	addl	$16, %esp
	.cfi_def_cfa_offset 32
	xorl	%edx, %edx
	movw	%ax, s_szVgaInfo+10
	.loc 1 98 0
	leal	-1(%ebx), %eax
	movw	%ax, s_szVgaInfo+12
	jmp	.L23
	.cfi_endproc
.LFE14:
	.size	VGA_SetCursorPos, .-VGA_SetCursorPos
	.section	.text.unlikely
.LCOLDE3:
	.text
.LHOTE3:
	.section	.text.unlikely
.LCOLDB4:
	.text
.LHOTB4:
	.p2align 4,,15
	.globl	VGA_ChangeLine
	.type	VGA_ChangeLine, @function
VGA_ChangeLine:
.LFB15:
	.loc 1 153 0
	.cfi_startproc
	.loc 1 155 0
	movzwl	s_szVgaInfo+12, %eax
	cmpw	s_szVgaInfo+4, %ax
	jb	.L30
	.loc 1 161 0
	movl	$1, %eax
	ret
	.p2align 4,,10
	.p2align 3
.L30:
	.loc 1 157 0
	addl	$1, %eax
	.loc 1 153 0
	subl	$20, %esp
	.cfi_def_cfa_offset 24
	.loc 1 157 0
	movzwl	%ax, %eax
	pushl	%eax
	.cfi_def_cfa_offset 28
	pushl	$0
	.cfi_def_cfa_offset 32
	call	VGA_SetCursorPos
.LVL24:
	.loc 1 161 0
	movl	$1, %eax
	addl	$28, %esp
	.cfi_def_cfa_offset 4
	ret
	.cfi_endproc
.LFE15:
	.size	VGA_ChangeLine, .-VGA_ChangeLine
	.section	.text.unlikely
.LCOLDE4:
	.text
.LHOTE4:
	.section	.text.unlikely
.LCOLDB5:
	.text
.LHOTB5:
	.p2align 4,,15
	.globl	VGA_GetDisplayRange
	.type	VGA_GetDisplayRange, @function
VGA_GetDisplayRange:
.LFB16:
	.loc 1 164 0
	.cfi_startproc
.LVL25:
	.loc 1 164 0
	movl	4(%esp), %edx
	movl	8(%esp), %eax
	.loc 1 165 0
	testl	%edx, %edx
	je	.L32
	.loc 1 167 0
	movzwl	s_szVgaInfo+4, %ecx
	movl	%ecx, (%edx)
.L32:
	.loc 1 170 0
	testl	%eax, %eax
	je	.L33
	.loc 1 172 0
	movzwl	s_szVgaInfo+6, %edx
	movl	%edx, (%eax)
.L33:
	.loc 1 176 0
	movl	$1, %eax
	ret
	.cfi_endproc
.LFE16:
	.size	VGA_GetDisplayRange, .-VGA_GetDisplayRange
	.section	.text.unlikely
.LCOLDE5:
	.text
.LHOTE5:
	.section	.text.unlikely
.LCOLDB6:
	.text
.LHOTB6:
	.p2align 4,,15
	.globl	VGA_PrintChar
	.type	VGA_PrintChar, @function
VGA_PrintChar:
.LFB17:
	.loc 1 179 0
	.cfi_startproc
.LVL26:
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	pushl	%edi
	.cfi_def_cfa_offset 12
	.cfi_offset 7, -12
	.loc 1 186 0
	xorl	%eax, %eax
	.loc 1 179 0
	pushl	%esi
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	pushl	%ebx
	.cfi_def_cfa_offset 20
	.cfi_offset 3, -20
	subl	$12, %esp
	.cfi_def_cfa_offset 32
	.loc 1 180 0
	movzwl	s_szVgaInfo+10, %edx
.LVL27:
.LBB24:
.LBB25:
	.loc 1 46 0
	movzwl	s_szVgaInfo+6, %ebx
.LBE25:
.LBE24:
	.loc 1 179 0
	movl	32(%esp), %ecx
	.loc 1 181 0
	movzwl	s_szVgaInfo+12, %esi
.LVL28:
.LBB27:
.LBB26:
	.loc 1 46 0
	cmpw	%bx, %dx
	jnb	.L41
	cmpw	s_szVgaInfo+4, %si
	jnb	.L41
	.loc 1 51 0
	movzwl	%si, %eax
.LVL29:
	movzwl	%bx, %edi
	imull	%eax, %edi
	movl	%edi, %ebp
	movzwl	%dx, %edi
	addl	%ebp, %edi
	.loc 1 53 0
	movl	s_szVgaInfo, %ebp
	leal	0(%ebp,%edi,2), %edi
.LBE26:
.LBE27:
	.loc 1 184 0
	cmpl	$757663, %edi
	ja	.L46
	testl	%edi, %edi
	je	.L46
	.loc 1 189 0
	movsbw	%cl, %cx
	.loc 1 190 0
	addl	$1, %edx
.LVL30:
	.loc 1 189 0
	orb	$7, %ch
	.loc 1 192 0
	cmpw	%bx, %dx
	.loc 1 189 0
	movw	%cx, (%edi)
	.loc 1 192 0
	jnb	.L42
	movzwl	%dx, %edx
.LVL31:
.L43:
	.loc 1 197 0
	subl	$8, %esp
	.cfi_def_cfa_offset 40
	pushl	%eax
	.cfi_def_cfa_offset 44
	pushl	%edx
	.cfi_def_cfa_offset 48
	call	VGA_SetCursorPos
.LVL32:
	.loc 1 199 0
	addl	$16, %esp
	.cfi_def_cfa_offset 32
	movl	$1, %eax
.L41:
	.loc 1 200 0
	addl	$12, %esp
	.cfi_remember_state
	.cfi_def_cfa_offset 20
	popl	%ebx
	.cfi_restore 3
	.cfi_def_cfa_offset 16
	popl	%esi
	.cfi_restore 6
	.cfi_def_cfa_offset 12
.LVL33:
	popl	%edi
	.cfi_restore 7
	.cfi_def_cfa_offset 8
	popl	%ebp
	.cfi_restore 5
	.cfi_def_cfa_offset 4
	ret
.LVL34:
	.p2align 4,,10
	.p2align 3
.L42:
	.cfi_restore_state
	.loc 1 195 0
	leal	1(%esi), %eax
.LVL35:
	xorl	%edx, %edx
	movzwl	%ax, %eax
	jmp	.L43
.LVL36:
.L46:
	.loc 1 186 0
	xorl	%eax, %eax
.LVL37:
	jmp	.L41
	.cfi_endproc
.LFE17:
	.size	VGA_PrintChar, .-VGA_PrintChar
	.section	.text.unlikely
.LCOLDE6:
	.text
.LHOTE6:
	.section	.text.unlikely
.LCOLDB7:
	.text
.LHOTB7:
	.p2align 4,,15
	.globl	VGA_PrintString
	.type	VGA_PrintString, @function
VGA_PrintString:
.LFB18:
	.loc 1 203 0
	.cfi_startproc
.LVL38:
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	pushl	%edi
	.cfi_def_cfa_offset 12
	.cfi_offset 7, -12
	pushl	%esi
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	pushl	%ebx
	.cfi_def_cfa_offset 20
	.cfi_offset 3, -20
	.loc 1 212 0
	xorl	%ebx, %ebx
	.loc 1 203 0
	subl	$28, %esp
	.cfi_def_cfa_offset 48
	.loc 1 204 0
	movzwl	s_szVgaInfo+10, %esi
.LVL39:
.LBB28:
.LBB29:
	.loc 1 46 0
	movzwl	s_szVgaInfo+6, %ecx
.LBE29:
.LBE28:
	.loc 1 203 0
	movl	48(%esp), %ebp
	.loc 1 204 0
	movzwl	s_szVgaInfo+12, %edi
.LVL40:
.LBB31:
.LBB30:
	.loc 1 46 0
	cmpw	%cx, %si
	jnb	.L49
	cmpw	s_szVgaInfo+4, %di
	jnb	.L49
.LVL41:
	.loc 1 51 0
	movzwl	%di, %eax
	movzwl	%cx, %edx
	movzwl	%si, %ebx
	imull	%edx, %eax
	addl	%ebx, %eax
	.loc 1 53 0
	movl	s_szVgaInfo, %ebx
	leal	(%ebx,%eax,2), %ebx
.LVL42:
.LBE30:
.LBE31:
	.loc 1 210 0
	testl	%ebx, %ebx
	je	.L56
	testl	%ebp, %ebp
	je	.L56
.LBB32:
	.loc 1 237 0
	leal	(%edx,%edx), %eax
	negl	%eax
	movl	%eax, 12(%esp)
	jmp	.L50
.LVL43:
	.p2align 4,,10
	.p2align 3
.L51:
	.loc 1 241 0
	cmpl	$757663, %ebx
	ja	.L52
.LVL44:
.L50:
.LBE32:
	.loc 1 215 0
	movzbl	0(%ebp), %eax
	testb	%al, %al
	je	.L52
.LVL45:
.LBB33:
	.loc 1 219 0
	addw	$1792, %ax
	.loc 1 226 0
	addl	$1, %esi
.LVL46:
	.loc 1 222 0
	addl	$2, %ebx
.LVL47:
	.loc 1 219 0
	movw	%ax, -2(%ebx)
.LVL48:
	.loc 1 223 0
	addl	$1, %ebp
.LVL49:
	.loc 1 227 0
	cmpw	%cx, %si
	jb	.L51
.LVL50:
	.loc 1 232 0
	movzwl	s_szVgaInfo+8, %eax
.LVL51:
	.loc 1 230 0
	addl	$1, %edi
.LVL52:
	.loc 1 229 0
	xorl	%esi, %esi
	.loc 1 232 0
	cmpw	%ax, %di
	jb	.L51
	movl	%ecx, 8(%esp)
	.loc 1 235 0
	subl	$12, %esp
	.cfi_def_cfa_offset 60
	.loc 1 234 0
	leal	-1(%eax), %edi
.LVL53:
	.loc 1 235 0
	pushl	$1
	.cfi_def_cfa_offset 64
	call	_VGA_ScrollLine
.LVL54:
	.loc 1 237 0
	addl	28(%esp), %ebx
.LVL55:
	addl	$16, %esp
	.cfi_def_cfa_offset 48
	movl	8(%esp), %ecx
.LVL56:
	.loc 1 241 0
	cmpl	$757663, %ebx
	jbe	.L50
	.p2align 4,,10
	.p2align 3
.L52:
.LBE33:
	.loc 1 248 0
	subl	$8, %esp
	.cfi_def_cfa_offset 56
	movzwl	%di, %edx
	movzwl	%si, %esi
	pushl	%edx
	.cfi_def_cfa_offset 60
	pushl	%esi
	.cfi_def_cfa_offset 64
	.loc 1 255 0
	movl	$1, %ebx
.LVL57:
	.loc 1 248 0
	call	VGA_SetCursorPos
.LVL58:
	.loc 1 250 0
	addl	$16, %esp
	.cfi_def_cfa_offset 48
	cmpl	$1, 52(%esp)
	je	.L60
.LVL59:
.L49:
	.loc 1 256 0
	addl	$28, %esp
	.cfi_remember_state
	.cfi_def_cfa_offset 20
	movl	%ebx, %eax
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
.LVL60:
	.p2align 4,,10
	.p2align 3
.L60:
	.cfi_restore_state
	.loc 1 252 0
	call	VGA_ChangeLine
.LVL61:
	.loc 1 256 0
	addl	$28, %esp
	.cfi_remember_state
	.cfi_def_cfa_offset 20
	movl	%ebx, %eax
	popl	%ebx
	.cfi_restore 3
	.cfi_def_cfa_offset 16
	popl	%esi
	.cfi_restore 6
	.cfi_def_cfa_offset 12
.LVL62:
	popl	%edi
	.cfi_restore 7
	.cfi_def_cfa_offset 8
.LVL63:
	popl	%ebp
	.cfi_restore 5
	.cfi_def_cfa_offset 4
.LVL64:
	ret
.LVL65:
.L56:
	.cfi_restore_state
	.loc 1 212 0
	xorl	%ebx, %ebx
	jmp	.L49
	.cfi_endproc
.LFE18:
	.size	VGA_PrintString, .-VGA_PrintString
	.section	.text.unlikely
.LCOLDE7:
	.text
.LHOTE7:
	.section	.text.unlikely
.LCOLDB8:
	.text
.LHOTB8:
	.p2align 4,,15
	.globl	VGA_GetCursorPos
	.type	VGA_GetCursorPos, @function
VGA_GetCursorPos:
.LFB19:
	.loc 1 259 0
	.cfi_startproc
.LVL66:
	.loc 1 259 0
	movl	4(%esp), %edx
	movl	8(%esp), %eax
	.loc 1 260 0
	testl	%edx, %edx
	je	.L62
	.loc 1 262 0
	movzwl	s_szVgaInfo+10, %ecx
	movw	%cx, (%edx)
.L62:
	.loc 1 265 0
	testl	%eax, %eax
	je	.L63
	.loc 1 267 0
	movzwl	s_szVgaInfo+12, %edx
	movw	%dx, (%eax)
.L63:
	.loc 1 271 0
	movl	$1, %eax
	ret
	.cfi_endproc
.LFE19:
	.size	VGA_GetCursorPos, .-VGA_GetCursorPos
	.section	.text.unlikely
.LCOLDE8:
	.text
.LHOTE8:
	.section	.text.unlikely
.LCOLDB9:
	.text
.LHOTB9:
	.p2align 4,,15
	.globl	VGA_GetString
	.type	VGA_GetString, @function
VGA_GetString:
.LFB20:
	.loc 1 274 0
	.cfi_startproc
.LVL67:
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	pushl	%edi
	.cfi_def_cfa_offset 12
	.cfi_offset 7, -12
.LBB34:
.LBB35:
	.loc 1 48 0
	xorl	%eax, %eax
.LBE35:
.LBE34:
	.loc 1 274 0
	pushl	%esi
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	pushl	%ebx
	.cfi_def_cfa_offset 20
	.cfi_offset 3, -20
	subl	$4, %esp
	.cfi_def_cfa_offset 24
.LBB39:
.LBB36:
	.loc 1 46 0
	movzwl	s_szVgaInfo+6, %edx
.LBE36:
.LBE39:
	.loc 1 274 0
	movl	24(%esp), %esi
	movl	28(%esp), %ebx
.LVL68:
	movl	32(%esp), %edi
.LBB40:
.LBB37:
	.loc 1 46 0
	cmpw	%dx, %si
	jnb	.L71
	cmpw	s_szVgaInfo+4, %bx
	jb	.L80
.LVL69:
.L71:
.LBE37:
.LBE40:
	.loc 1 276 0
	movzwl	s_szVgaInfo+12, %edx
	leal	1(%esi), %ecx
	.loc 1 277 0
	leal	(%ebx,%ebx,4), %ebx
.LVL70:
	movw	%cx, 2(%esp)
	.loc 1 280 0
	movl	36(%esp), %ecx
	.loc 1 277 0
	sall	$4, %ebx
	.loc 1 276 0
	leal	(%edx,%edx,4), %edx
	sall	$4, %edx
	addw	s_szVgaInfo+10, %dx
.LVL71:
	.loc 1 280 0
	testl	%ecx, %ecx
	jle	.L78
.LBB41:
	.loc 1 282 0
	movzbl	(%eax), %esi
.LVL72:
	andl	$127, %esi
.LVL73:
	.loc 1 284 0
	leal	-32(%esi), %ebp
	movl	%ebp, %ecx
	cmpb	$94, %cl
	ja	.L78
	.loc 1 288 0
	movl	%esi, %ecx
	.loc 1 290 0
	addl	$2, %eax
.LVL74:
	.loc 1 288 0
	movb	%cl, (%edi)
.LVL75:
	.loc 1 291 0
	movzwl	2(%esp), %ecx
	addl	%ebx, %ecx
.LVL76:
	.loc 1 293 0
	cmpw	%cx, %dx
	jbe	.L78
	.loc 1 298 0
	cmpl	$757663, %eax
	ja	.L78
	movl	36(%esp), %ebp
	subl	$1, %edx
.LVL77:
	leal	1(%edi), %esi
	subl	%ecx, %edx
	movzwl	%dx, %edx
	addl	%edi, %ebp
	leal	1(%edi,%edx), %ebx
.LVL78:
	jmp	.L74
.LVL79:
	.p2align 4,,10
	.p2align 3
.L75:
	.loc 1 282 0
	movzbl	(%eax), %edx
	andl	$127, %edx
.LVL80:
	.loc 1 284 0
	leal	-32(%edx), %ecx
	cmpb	$94, %cl
	ja	.L78
	.loc 1 290 0
	addl	$2, %eax
.LVL81:
	.loc 1 293 0
	cmpl	%ebx, %esi
	.loc 1 288 0
	movb	%dl, (%esi)
	.loc 1 293 0
	je	.L78
	addl	$1, %esi
.LVL82:
	.loc 1 298 0
	cmpl	$757663, %eax
	ja	.L78
.LVL83:
.L74:
.LBE41:
	.loc 1 280 0 discriminator 2
	cmpl	%ebp, %esi
	jne	.L75
.LVL84:
.L78:
	.loc 1 305 0
	addl	$4, %esp
	.cfi_remember_state
	.cfi_def_cfa_offset 20
	movl	$1, %eax
.LVL85:
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
.LVL86:
	.p2align 4,,10
	.p2align 3
.L80:
	.cfi_restore_state
.LBB42:
.LBB38:
	.loc 1 51 0
	movzwl	%bx, %ecx
	movzwl	%dx, %eax
	movl	%ecx, %edx
.LVL87:
	imull	%eax, %edx
	movzwl	%si, %eax
	addl	%eax, %edx
	.loc 1 53 0
	movl	s_szVgaInfo, %eax
	leal	(%eax,%edx,2), %eax
	jmp	.L71
.LBE38:
.LBE42:
	.cfi_endproc
.LFE20:
	.size	VGA_GetString, .-VGA_GetString
	.section	.text.unlikely
.LCOLDE9:
	.text
.LHOTE9:
	.section	.text.unlikely
.LCOLDB10:
	.text
.LHOTB10:
	.p2align 4,,15
	.globl	VGA_DelString
	.type	VGA_DelString, @function
VGA_DelString:
.LFB21:
	.loc 1 308 0
	.cfi_startproc
.LVL88:
.LBB43:
.LBB44:
	.loc 1 46 0
	movzwl	s_szVgaInfo+6, %ecx
.LBE44:
.LBE43:
	.loc 1 308 0
	pushl	%esi
	.cfi_def_cfa_offset 8
	.cfi_offset 6, -8
	.loc 1 314 0
	xorl	%eax, %eax
	.loc 1 308 0
	pushl	%ebx
	.cfi_def_cfa_offset 12
	.cfi_offset 3, -12
	.loc 1 308 0
	movl	12(%esp), %edx
.LVL89:
	movl	16(%esp), %esi
.LVL90:
	movl	20(%esp), %ebx
.LBB46:
.LBB45:
	.loc 1 46 0
	cmpw	%cx, %dx
	jnb	.L82
	cmpw	s_szVgaInfo+4, %si
	jnb	.L82
.LVL91:
	.loc 1 51 0
	movzwl	%si, %esi
	movzwl	%dx, %edx
	imull	%esi, %ecx
.LVL92:
	addl	%ecx, %edx
.LVL93:
	.loc 1 53 0
	movl	s_szVgaInfo, %ecx
	leal	(%ecx,%edx,2), %edx
.LVL94:
.LBE45:
.LBE46:
	.loc 1 312 0
	testl	%edx, %edx
	je	.L82
.LVL95:
	.loc 1 317 0 discriminator 1
	testl	%ebx, %ebx
	jle	.L84
	.loc 1 321 0
	leal	2(%edx), %eax
.LVL96:
	.loc 1 319 0
	movl	$1792, %esi
.LVL97:
	movw	%si, (%edx)
.LVL98:
	.loc 1 322 0
	cmpl	$757663, %eax
	ja	.L84
	leal	(%edx,%ebx,2), %edx
	jmp	.L85
.LVL99:
	.p2align 4,,10
	.p2align 3
.L86:
	.loc 1 319 0
	movl	$1792, %ecx
	.loc 1 321 0
	addl	$2, %eax
.LVL100:
	.loc 1 319 0
	movw	%cx, -2(%eax)
.LVL101:
	.loc 1 322 0
	cmpl	$757663, %eax
	ja	.L84
.L85:
	.loc 1 317 0 discriminator 2
	cmpl	%edx, %eax
	jne	.L86
.LVL102:
.L84:
	.loc 1 328 0
	movl	$1, %eax
.L82:
	.loc 1 329 0
	popl	%ebx
	.cfi_restore 3
	.cfi_def_cfa_offset 8
	popl	%esi
	.cfi_restore 6
	.cfi_def_cfa_offset 4
	ret
	.cfi_endproc
.LFE21:
	.size	VGA_DelString, .-VGA_DelString
	.section	.text.unlikely
.LCOLDE10:
	.text
.LHOTE10:
	.section	.text.unlikely
.LCOLDB11:
	.text
.LHOTB11:
	.p2align 4,,15
	.globl	VGA_DelChar
	.type	VGA_DelChar, @function
VGA_DelChar:
.LFB22:
	.loc 1 332 0
	.cfi_startproc
.LVL103:
	pushl	%ebx
	.cfi_def_cfa_offset 8
	.cfi_offset 3, -8
	subl	$8, %esp
	.cfi_def_cfa_offset 16
	.loc 1 335 0
	movzwl	s_szVgaInfo+12, %ebx
.LVL104:
	.loc 1 332 0
	movl	16(%esp), %edx
	.loc 1 338 0
	testl	%edx, %edx
	jne	.L92
	.loc 1 340 0
	movzwl	s_szVgaInfo+10, %eax
	testw	%ax, %ax
	je	.L105
	movzwl	s_szVgaInfo+6, %edx
	.loc 1 352 0
	subl	$1, %eax
.LVL105:
.L95:
.LBB47:
.LBB48:
	.loc 1 46 0
	cmpw	%ax, %dx
	ja	.L96
.L106:
	movzwl	%ax, %ecx
	movzwl	%bx, %ebx
.LVL106:
	.loc 1 48 0
	xorl	%eax, %eax
.LVL107:
	.p2align 4,,10
	.p2align 3
.L99:
.LBE48:
.LBE47:
	.loc 1 368 0
	movzwl	2(%eax), %edx
	.loc 1 369 0
	addl	$2, %eax
.LVL108:
	.loc 1 368 0
	movw	%dx, -2(%eax)
.LVL109:
	.loc 1 371 0
	cmpl	$757663, %eax
	jbe	.L99
	.loc 1 378 0
	subl	$8, %esp
	.cfi_def_cfa_offset 24
	pushl	%ebx
	.cfi_def_cfa_offset 28
	pushl	%ecx
	.cfi_def_cfa_offset 32
	call	VGA_SetCursorPos
.LVL110:
	.loc 1 380 0
	addl	$16, %esp
	.cfi_def_cfa_offset 16
	movl	$1, %eax
.L94:
	.loc 1 381 0
	addl	$8, %esp
	.cfi_remember_state
	.cfi_def_cfa_offset 8
	popl	%ebx
	.cfi_restore 3
	.cfi_def_cfa_offset 4
	ret
.LVL111:
	.p2align 4,,10
	.p2align 3
.L92:
	.cfi_restore_state
	.loc 1 342 0
	xorl	%eax, %eax
	.loc 1 355 0
	cmpl	$1, %edx
	jne	.L94
	.loc 1 357 0
	movzwl	s_szVgaInfo+10, %eax
.LVL112:
	movzwl	s_szVgaInfo+6, %edx
.LVL113:
.LBB51:
.LBB49:
	.loc 1 46 0
	cmpw	%ax, %dx
	jbe	.L106
.L96:
	cmpw	%bx, s_szVgaInfo+4
	movzwl	%ax, %ecx
	movzwl	%bx, %ebx
.LVL114:
	ja	.L98
	.loc 1 48 0
	xorl	%eax, %eax
.LVL115:
	jmp	.L99
.LVL116:
	.p2align 4,,10
	.p2align 3
.L105:
.LBE49:
.LBE51:
	.loc 1 342 0 discriminator 1
	xorl	%eax, %eax
	.loc 1 340 0 discriminator 1
	testw	%bx, %bx
	je	.L94
	.loc 1 347 0
	movzwl	s_szVgaInfo+6, %eax
	.loc 1 348 0
	subl	$1, %ebx
.LVL117:
	.loc 1 347 0
	movl	%eax, %edx
	subl	$1, %eax
.LVL118:
	jmp	.L95
.LVL119:
	.p2align 4,,10
	.p2align 3
.L98:
.LBB52:
.LBB50:
	.loc 1 51 0
	movzwl	%dx, %eax
.LVL120:
	.loc 1 53 0
	movl	s_szVgaInfo, %edx
.LVL121:
	.loc 1 51 0
	imull	%ebx, %eax
.LVL122:
	addl	%ecx, %eax
	.loc 1 53 0
	leal	(%edx,%eax,2), %eax
	jmp	.L99
.LBE50:
.LBE52:
	.cfi_endproc
.LFE22:
	.size	VGA_DelChar, .-VGA_DelChar
	.section	.text.unlikely
.LCOLDE11:
	.text
.LHOTE11:
	.section	.text.unlikely
.LCOLDB12:
	.text
.LHOTB12:
	.p2align 4,,15
	.globl	VGA_Clear
	.type	VGA_Clear, @function
VGA_Clear:
.LFB23:
	.loc 1 384 0
	.cfi_startproc
	.loc 1 385 0
	movl	s_szVgaInfo, %eax
.LVL123:
	leal	4000(%eax), %edx
.LVL124:
	.p2align 4,,10
	.p2align 3
.L108:
	.loc 1 390 0 discriminator 3
	movl	$1792, %ecx
	addl	$2, %eax
	movw	%cx, -2(%eax)
	.loc 1 388 0 discriminator 3
	cmpl	%edx, %eax
	jne	.L108
	.loc 1 394 0
	movl	$1, %eax
	ret
	.cfi_endproc
.LFE23:
	.size	VGA_Clear, .-VGA_Clear
	.section	.text.unlikely
.LCOLDE12:
	.text
.LHOTE12:
	.section	.text.unlikely
.LCOLDB13:
	.text
.LHOTB13:
	.p2align 4,,15
	.globl	InitializeVGA
	.type	InitializeVGA, @function
InitializeVGA:
.LFB24:
	.loc 1 397 0
	.cfi_startproc
	.loc 1 399 0
	movl	$25, %eax
	.loc 1 400 0
	movl	$80, %edx
	.loc 1 401 0
	movl	$25, %ecx
	.loc 1 399 0
	movw	%ax, s_szVgaInfo+4
	.loc 1 398 0
	movl	$753664, s_szVgaInfo
	.loc 1 403 0
	movl	$1, %eax
	.loc 1 400 0
	movw	%dx, s_szVgaInfo+6
	.loc 1 401 0
	movw	%cx, s_szVgaInfo+8
	.loc 1 403 0
	ret
	.cfi_endproc
.LFE24:
	.size	InitializeVGA, .-InitializeVGA
	.section	.text.unlikely
.LCOLDE13:
	.text
.LHOTE13:
	.local	s_szVgaInfo
	.comm	s_szVgaInfo,16,4
.Letext0:
	.section	.text.unlikely
.Letext_cold0:
	.file 2 "../../../kernel/include/types.h"
	.file 3 "/usr/include/bits/types.h"
	.file 4 "/usr/include/libio.h"
	.file 5 "/usr/include/stdio.h"
	.section	.debug_info,"",@progbits
.Ldebug_info0:
	.long	0x992
	.value	0x4
	.long	.Ldebug_abbrev0
	.byte	0x4
	.uleb128 0x1
	.long	.LASF101
	.byte	0x1
	.long	.LASF102
	.long	.LASF103
	.long	.Ltext0
	.long	.Letext0-.Ltext0
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
	.uleb128 0x2
	.long	.LASF7
	.byte	0x2
	.byte	0x25
	.long	0x71
	.uleb128 0x3
	.byte	0x1
	.byte	0x6
	.long	.LASF8
	.uleb128 0x3
	.byte	0x2
	.byte	0x5
	.long	.LASF9
	.uleb128 0x4
	.string	"INT"
	.byte	0x2
	.byte	0x27
	.long	0x8a
	.uleb128 0x5
	.byte	0x4
	.byte	0x5
	.string	"int"
	.uleb128 0x3
	.byte	0x4
	.byte	0x7
	.long	.LASF10
	.uleb128 0x3
	.byte	0x8
	.byte	0x4
	.long	.LASF11
	.uleb128 0x3
	.byte	0x4
	.byte	0x4
	.long	.LASF12
	.uleb128 0x2
	.long	.LASF13
	.byte	0x2
	.byte	0x34
	.long	0xb1
	.uleb128 0x6
	.byte	0x4
	.long	0x71
	.uleb128 0x6
	.byte	0x4
	.long	0xbd
	.uleb128 0x7
	.long	0x71
	.uleb128 0x2
	.long	.LASF14
	.byte	0x2
	.byte	0x36
	.long	0xb7
	.uleb128 0x8
	.byte	0x4
	.uleb128 0x3
	.byte	0x4
	.byte	0x7
	.long	.LASF15
	.uleb128 0x3
	.byte	0x1
	.byte	0x6
	.long	.LASF16
	.uleb128 0x3
	.byte	0x8
	.byte	0x5
	.long	.LASF17
	.uleb128 0x3
	.byte	0x8
	.byte	0x7
	.long	.LASF18
	.uleb128 0x2
	.long	.LASF19
	.byte	0x3
	.byte	0x37
	.long	0xdd
	.uleb128 0x2
	.long	.LASF20
	.byte	0x3
	.byte	0x83
	.long	0x101
	.uleb128 0x3
	.byte	0x4
	.byte	0x5
	.long	.LASF21
	.uleb128 0x2
	.long	.LASF22
	.byte	0x3
	.byte	0x84
	.long	0xeb
	.uleb128 0x9
	.long	.LASF52
	.byte	0x94
	.byte	0x4
	.byte	0xf5
	.long	0x293
	.uleb128 0xa
	.long	.LASF23
	.byte	0x4
	.byte	0xf6
	.long	0x8a
	.byte	0
	.uleb128 0xa
	.long	.LASF24
	.byte	0x4
	.byte	0xfb
	.long	0xb1
	.byte	0x4
	.uleb128 0xa
	.long	.LASF25
	.byte	0x4
	.byte	0xfc
	.long	0xb1
	.byte	0x8
	.uleb128 0xa
	.long	.LASF26
	.byte	0x4
	.byte	0xfd
	.long	0xb1
	.byte	0xc
	.uleb128 0xa
	.long	.LASF27
	.byte	0x4
	.byte	0xfe
	.long	0xb1
	.byte	0x10
	.uleb128 0xa
	.long	.LASF28
	.byte	0x4
	.byte	0xff
	.long	0xb1
	.byte	0x14
	.uleb128 0xb
	.long	.LASF29
	.byte	0x4
	.value	0x100
	.long	0xb1
	.byte	0x18
	.uleb128 0xb
	.long	.LASF30
	.byte	0x4
	.value	0x101
	.long	0xb1
	.byte	0x1c
	.uleb128 0xb
	.long	.LASF31
	.byte	0x4
	.value	0x102
	.long	0xb1
	.byte	0x20
	.uleb128 0xb
	.long	.LASF32
	.byte	0x4
	.value	0x104
	.long	0xb1
	.byte	0x24
	.uleb128 0xb
	.long	.LASF33
	.byte	0x4
	.value	0x105
	.long	0xb1
	.byte	0x28
	.uleb128 0xb
	.long	.LASF34
	.byte	0x4
	.value	0x106
	.long	0xb1
	.byte	0x2c
	.uleb128 0xb
	.long	.LASF35
	.byte	0x4
	.value	0x108
	.long	0x2cb
	.byte	0x30
	.uleb128 0xb
	.long	.LASF36
	.byte	0x4
	.value	0x10a
	.long	0x2d1
	.byte	0x34
	.uleb128 0xb
	.long	.LASF37
	.byte	0x4
	.value	0x10c
	.long	0x8a
	.byte	0x38
	.uleb128 0xb
	.long	.LASF38
	.byte	0x4
	.value	0x110
	.long	0x8a
	.byte	0x3c
	.uleb128 0xb
	.long	.LASF39
	.byte	0x4
	.value	0x112
	.long	0xf6
	.byte	0x40
	.uleb128 0xb
	.long	.LASF40
	.byte	0x4
	.value	0x116
	.long	0x42
	.byte	0x44
	.uleb128 0xb
	.long	.LASF41
	.byte	0x4
	.value	0x117
	.long	0xd6
	.byte	0x46
	.uleb128 0xb
	.long	.LASF42
	.byte	0x4
	.value	0x118
	.long	0x2d7
	.byte	0x47
	.uleb128 0xb
	.long	.LASF43
	.byte	0x4
	.value	0x11c
	.long	0x2e7
	.byte	0x48
	.uleb128 0xb
	.long	.LASF44
	.byte	0x4
	.value	0x125
	.long	0x108
	.byte	0x4c
	.uleb128 0xb
	.long	.LASF45
	.byte	0x4
	.value	0x12e
	.long	0xcd
	.byte	0x54
	.uleb128 0xb
	.long	.LASF46
	.byte	0x4
	.value	0x12f
	.long	0xcd
	.byte	0x58
	.uleb128 0xb
	.long	.LASF47
	.byte	0x4
	.value	0x130
	.long	0xcd
	.byte	0x5c
	.uleb128 0xb
	.long	.LASF48
	.byte	0x4
	.value	0x131
	.long	0xcd
	.byte	0x60
	.uleb128 0xb
	.long	.LASF49
	.byte	0x4
	.value	0x132
	.long	0x91
	.byte	0x64
	.uleb128 0xb
	.long	.LASF50
	.byte	0x4
	.value	0x134
	.long	0x8a
	.byte	0x68
	.uleb128 0xb
	.long	.LASF51
	.byte	0x4
	.value	0x136
	.long	0x2ed
	.byte	0x6c
	.byte	0
	.uleb128 0xc
	.long	.LASF104
	.byte	0x4
	.byte	0x9a
	.uleb128 0x9
	.long	.LASF53
	.byte	0xc
	.byte	0x4
	.byte	0xa0
	.long	0x2cb
	.uleb128 0xa
	.long	.LASF54
	.byte	0x4
	.byte	0xa1
	.long	0x2cb
	.byte	0
	.uleb128 0xa
	.long	.LASF55
	.byte	0x4
	.byte	0xa2
	.long	0x2d1
	.byte	0x4
	.uleb128 0xa
	.long	.LASF56
	.byte	0x4
	.byte	0xa6
	.long	0x8a
	.byte	0x8
	.byte	0
	.uleb128 0x6
	.byte	0x4
	.long	0x29a
	.uleb128 0x6
	.byte	0x4
	.long	0x113
	.uleb128 0xd
	.long	0x71
	.long	0x2e7
	.uleb128 0xe
	.long	0xcf
	.byte	0
	.byte	0
	.uleb128 0x6
	.byte	0x4
	.long	0x293
	.uleb128 0xd
	.long	0x71
	.long	0x2fd
	.uleb128 0xe
	.long	0xcf
	.byte	0x27
	.byte	0
	.uleb128 0x9
	.long	.LASF57
	.byte	0x10
	.byte	0x1
	.byte	0x1b
	.long	0x352
	.uleb128 0xa
	.long	.LASF58
	.byte	0x1
	.byte	0x1d
	.long	0x352
	.byte	0
	.uleb128 0xa
	.long	.LASF59
	.byte	0x1
	.byte	0x1f
	.long	0x37
	.byte	0x4
	.uleb128 0xa
	.long	.LASF60
	.byte	0x1
	.byte	0x20
	.long	0x37
	.byte	0x6
	.uleb128 0xa
	.long	.LASF61
	.byte	0x1
	.byte	0x21
	.long	0x37
	.byte	0x8
	.uleb128 0xa
	.long	.LASF62
	.byte	0x1
	.byte	0x23
	.long	0x37
	.byte	0xa
	.uleb128 0xa
	.long	.LASF63
	.byte	0x1
	.byte	0x24
	.long	0x37
	.byte	0xc
	.byte	0
	.uleb128 0x6
	.byte	0x4
	.long	0x25
	.uleb128 0x2
	.long	.LASF64
	.byte	0x1
	.byte	0x26
	.long	0x2fd
	.uleb128 0xf
	.long	.LASF105
	.byte	0x1
	.byte	0x2a
	.long	0x393
	.byte	0x1
	.long	0x393
	.uleb128 0x10
	.string	"nX"
	.byte	0x1
	.byte	0x2a
	.long	0x37
	.uleb128 0x10
	.string	"nY"
	.byte	0x1
	.byte	0x2a
	.long	0x37
	.uleb128 0x11
	.long	.LASF65
	.byte	0x1
	.byte	0x2c
	.long	0x7f
	.byte	0
	.uleb128 0x6
	.byte	0x4
	.long	0x37
	.uleb128 0x12
	.long	0x363
	.long	.LFB11
	.long	.LFE11-.LFB11
	.uleb128 0x1
	.byte	0x9c
	.long	0x3c6
	.uleb128 0x13
	.long	0x373
	.uleb128 0x2
	.byte	0x91
	.sleb128 0
	.uleb128 0x13
	.long	0x37d
	.uleb128 0x2
	.byte	0x91
	.sleb128 4
	.uleb128 0x14
	.long	0x387
	.long	.LLST0
	.byte	0
	.uleb128 0x15
	.long	.LASF74
	.byte	0x1
	.byte	0x38
	.long	.LFB12
	.long	.LFE12-.LFB12
	.uleb128 0x1
	.byte	0x9c
	.long	0x46a
	.uleb128 0x16
	.long	.LASF70
	.byte	0x1
	.byte	0x38
	.long	0x5b
	.uleb128 0x2
	.byte	0x91
	.sleb128 0
	.uleb128 0x11
	.long	.LASF66
	.byte	0x1
	.byte	0x3a
	.long	0x393
	.uleb128 0x17
	.long	.LASF67
	.byte	0x1
	.byte	0x3b
	.long	0x393
	.long	.LLST1
	.uleb128 0x17
	.long	.LASF68
	.byte	0x1
	.byte	0x3c
	.long	0x393
	.long	.LLST2
	.uleb128 0x18
	.long	0x363
	.long	.LBB17
	.long	.Ldebug_ranges0+0
	.byte	0x1
	.byte	0x3a
	.long	0x43e
	.uleb128 0x19
	.long	0x37d
	.byte	0
	.uleb128 0x19
	.long	0x373
	.byte	0
	.uleb128 0x1a
	.long	.Ldebug_ranges0+0
	.uleb128 0x1b
	.long	0x387
	.byte	0
	.byte	0
	.byte	0
	.uleb128 0x1a
	.long	.Ldebug_ranges0+0x18
	.uleb128 0x17
	.long	.LASF69
	.byte	0x1
	.byte	0x40
	.long	0x7f
	.long	.LLST3
	.uleb128 0x1c
	.string	"i"
	.byte	0x1
	.byte	0x41
	.long	0x7f
	.long	.LLST4
	.uleb128 0x1d
	.long	.LVL14
	.long	0x971
	.byte	0
	.byte	0
	.uleb128 0x1e
	.long	.LASF106
	.byte	0x1
	.byte	0x4f
	.long	0x49
	.long	.LFB13
	.long	.LFE13-.LFB13
	.uleb128 0x1
	.byte	0x9c
	.uleb128 0x1f
	.long	.LASF76
	.byte	0x1
	.byte	0x54
	.long	0x5b
	.long	.LFB14
	.long	.LFE14-.LFB14
	.uleb128 0x1
	.byte	0x9c
	.long	0x4ea
	.uleb128 0x16
	.long	.LASF62
	.byte	0x1
	.byte	0x54
	.long	0x37
	.uleb128 0x2
	.byte	0x91
	.sleb128 0
	.uleb128 0x16
	.long	.LASF63
	.byte	0x1
	.byte	0x54
	.long	0x37
	.uleb128 0x2
	.byte	0x91
	.sleb128 4
	.uleb128 0x17
	.long	.LASF71
	.byte	0x1
	.byte	0x59
	.long	0x37
	.long	.LLST5
	.uleb128 0x17
	.long	.LASF72
	.byte	0x1
	.byte	0x5a
	.long	0x25
	.long	.LLST6
	.uleb128 0x20
	.long	.LASF73
	.byte	0x1
	.byte	0x5b
	.long	0x25
	.uleb128 0x2
	.byte	0x91
	.sleb128 -17
	.uleb128 0x1d
	.long	.LVL23
	.long	0x3c6
	.byte	0
	.uleb128 0x21
	.long	.LASF75
	.byte	0x1
	.byte	0x98
	.long	0x5b
	.long	.LFB15
	.long	.LFE15-.LFB15
	.uleb128 0x1
	.byte	0x9c
	.long	0x50d
	.uleb128 0x1d
	.long	.LVL24
	.long	0x47f
	.byte	0
	.uleb128 0x1f
	.long	.LASF77
	.byte	0x1
	.byte	0xa3
	.long	0x5b
	.long	.LFB16
	.long	.LFE16-.LFB16
	.uleb128 0x1
	.byte	0x9c
	.long	0x543
	.uleb128 0x16
	.long	.LASF78
	.byte	0x1
	.byte	0xa3
	.long	0x543
	.uleb128 0x2
	.byte	0x91
	.sleb128 0
	.uleb128 0x16
	.long	.LASF79
	.byte	0x1
	.byte	0xa3
	.long	0x543
	.uleb128 0x2
	.byte	0x91
	.sleb128 4
	.byte	0
	.uleb128 0x6
	.byte	0x4
	.long	0x7f
	.uleb128 0x1f
	.long	.LASF80
	.byte	0x1
	.byte	0xb2
	.long	0x5b
	.long	.LFB17
	.long	.LFE17-.LFB17
	.uleb128 0x1
	.byte	0x9c
	.long	0x5d7
	.uleb128 0x22
	.string	"ch"
	.byte	0x1
	.byte	0xb2
	.long	0x66
	.uleb128 0x2
	.byte	0x91
	.sleb128 0
	.uleb128 0x17
	.long	.LASF62
	.byte	0x1
	.byte	0xb4
	.long	0x37
	.long	.LLST7
	.uleb128 0x17
	.long	.LASF63
	.byte	0x1
	.byte	0xb5
	.long	0x37
	.long	.LLST8
	.uleb128 0x11
	.long	.LASF81
	.byte	0x1
	.byte	0xb6
	.long	0x393
	.uleb128 0x18
	.long	0x363
	.long	.LBB24
	.long	.Ldebug_ranges0+0x38
	.byte	0x1
	.byte	0xb6
	.long	0x5cd
	.uleb128 0x23
	.long	0x37d
	.long	.LLST9
	.uleb128 0x23
	.long	0x373
	.long	.LLST10
	.uleb128 0x1a
	.long	.Ldebug_ranges0+0x38
	.uleb128 0x14
	.long	0x387
	.long	.LLST11
	.byte	0
	.byte	0
	.uleb128 0x1d
	.long	.LVL32
	.long	0x47f
	.byte	0
	.uleb128 0x1f
	.long	.LASF82
	.byte	0x1
	.byte	0xca
	.long	0x5b
	.long	.LFB18
	.long	.LFE18-.LFB18
	.uleb128 0x1
	.byte	0x9c
	.long	0x6b1
	.uleb128 0x16
	.long	.LASF83
	.byte	0x1
	.byte	0xca
	.long	0xc2
	.uleb128 0x2
	.byte	0x91
	.sleb128 0
	.uleb128 0x22
	.string	"cl"
	.byte	0x1
	.byte	0xca
	.long	0x5b
	.uleb128 0x2
	.byte	0x91
	.sleb128 4
	.uleb128 0x17
	.long	.LASF81
	.byte	0x1
	.byte	0xcc
	.long	0x393
	.long	.LLST12
	.uleb128 0x1c
	.string	"pos"
	.byte	0x1
	.byte	0xcd
	.long	0xa6
	.long	.LLST13
	.uleb128 0x17
	.long	.LASF62
	.byte	0x1
	.byte	0xce
	.long	0x37
	.long	.LLST14
	.uleb128 0x17
	.long	.LASF63
	.byte	0x1
	.byte	0xcf
	.long	0x37
	.long	.LLST15
	.uleb128 0x18
	.long	0x363
	.long	.LBB28
	.long	.Ldebug_ranges0+0x50
	.byte	0x1
	.byte	0xcc
	.long	0x67c
	.uleb128 0x23
	.long	0x37d
	.long	.LLST16
	.uleb128 0x23
	.long	0x373
	.long	.LLST17
	.uleb128 0x1a
	.long	.Ldebug_ranges0+0x50
	.uleb128 0x14
	.long	0x387
	.long	.LLST18
	.byte	0
	.byte	0
	.uleb128 0x24
	.long	.Ldebug_ranges0+0x68
	.long	0x69e
	.uleb128 0x1c
	.string	"wch"
	.byte	0x1
	.byte	0xd9
	.long	0x37
	.long	.LLST19
	.uleb128 0x1d
	.long	.LVL54
	.long	0x3c6
	.byte	0
	.uleb128 0x1d
	.long	.LVL58
	.long	0x47f
	.uleb128 0x1d
	.long	.LVL61
	.long	0x4ea
	.byte	0
	.uleb128 0x25
	.long	.LASF84
	.byte	0x1
	.value	0x102
	.long	0x5b
	.long	.LFB19
	.long	.LFE19-.LFB19
	.uleb128 0x1
	.byte	0x9c
	.long	0x6ea
	.uleb128 0x26
	.long	.LASF85
	.byte	0x1
	.value	0x102
	.long	0x393
	.uleb128 0x2
	.byte	0x91
	.sleb128 0
	.uleb128 0x26
	.long	.LASF86
	.byte	0x1
	.value	0x102
	.long	0x393
	.uleb128 0x2
	.byte	0x91
	.sleb128 4
	.byte	0
	.uleb128 0x25
	.long	.LASF87
	.byte	0x1
	.value	0x111
	.long	0x5b
	.long	.LFB20
	.long	.LFE20-.LFB20
	.uleb128 0x1
	.byte	0x9c
	.long	0x7ce
	.uleb128 0x26
	.long	.LASF62
	.byte	0x1
	.value	0x111
	.long	0x37
	.uleb128 0x2
	.byte	0x91
	.sleb128 0
	.uleb128 0x26
	.long	.LASF63
	.byte	0x1
	.value	0x111
	.long	0x37
	.uleb128 0x2
	.byte	0x91
	.sleb128 4
	.uleb128 0x26
	.long	.LASF83
	.byte	0x1
	.value	0x111
	.long	0xa6
	.uleb128 0x2
	.byte	0x91
	.sleb128 8
	.uleb128 0x26
	.long	.LASF88
	.byte	0x1
	.value	0x111
	.long	0x7f
	.uleb128 0x2
	.byte	0x91
	.sleb128 12
	.uleb128 0x27
	.long	.LASF81
	.byte	0x1
	.value	0x113
	.long	0x393
	.long	.LLST20
	.uleb128 0x27
	.long	.LASF89
	.byte	0x1
	.value	0x114
	.long	0x37
	.long	.LLST21
	.uleb128 0x27
	.long	.LASF90
	.byte	0x1
	.value	0x115
	.long	0x37
	.long	.LLST22
	.uleb128 0x28
	.string	"i"
	.byte	0x1
	.value	0x116
	.long	0x7f
	.long	.LLST23
	.uleb128 0x29
	.long	0x363
	.long	.LBB34
	.long	.Ldebug_ranges0+0x80
	.byte	0x1
	.value	0x113
	.long	0x7b4
	.uleb128 0x23
	.long	0x37d
	.long	.LLST24
	.uleb128 0x23
	.long	0x373
	.long	.LLST25
	.uleb128 0x1a
	.long	.Ldebug_ranges0+0x80
	.uleb128 0x14
	.long	0x387
	.long	.LLST26
	.byte	0
	.byte	0
	.uleb128 0x2a
	.long	.LBB41
	.long	.LBE41-.LBB41
	.uleb128 0x28
	.string	"ch"
	.byte	0x1
	.value	0x11a
	.long	0x25
	.long	.LLST27
	.byte	0
	.byte	0
	.uleb128 0x25
	.long	.LASF91
	.byte	0x1
	.value	0x133
	.long	0x5b
	.long	.LFB21
	.long	.LFE21-.LFB21
	.uleb128 0x1
	.byte	0x9c
	.long	0x866
	.uleb128 0x26
	.long	.LASF62
	.byte	0x1
	.value	0x133
	.long	0x37
	.uleb128 0x2
	.byte	0x91
	.sleb128 0
	.uleb128 0x26
	.long	.LASF63
	.byte	0x1
	.value	0x133
	.long	0x37
	.uleb128 0x2
	.byte	0x91
	.sleb128 4
	.uleb128 0x26
	.long	.LASF92
	.byte	0x1
	.value	0x133
	.long	0x7f
	.uleb128 0x2
	.byte	0x91
	.sleb128 8
	.uleb128 0x27
	.long	.LASF81
	.byte	0x1
	.value	0x135
	.long	0x393
	.long	.LLST28
	.uleb128 0x28
	.string	"i"
	.byte	0x1
	.value	0x136
	.long	0x7f
	.long	.LLST29
	.uleb128 0x2b
	.long	0x363
	.long	.LBB43
	.long	.Ldebug_ranges0+0xa8
	.byte	0x1
	.value	0x135
	.uleb128 0x23
	.long	0x37d
	.long	.LLST30
	.uleb128 0x23
	.long	0x373
	.long	.LLST31
	.uleb128 0x1a
	.long	.Ldebug_ranges0+0xa8
	.uleb128 0x14
	.long	0x387
	.long	.LLST32
	.byte	0
	.byte	0
	.byte	0
	.uleb128 0x25
	.long	.LASF93
	.byte	0x1
	.value	0x14b
	.long	0x5b
	.long	.LFB22
	.long	.LFE22-.LFB22
	.uleb128 0x1
	.byte	0x9c
	.long	0x8ff
	.uleb128 0x26
	.long	.LASF94
	.byte	0x1
	.value	0x14b
	.long	0x7f
	.uleb128 0x2
	.byte	0x91
	.sleb128 0
	.uleb128 0x27
	.long	.LASF81
	.byte	0x1
	.value	0x14d
	.long	0x393
	.long	.LLST33
	.uleb128 0x27
	.long	.LASF95
	.byte	0x1
	.value	0x14e
	.long	0x7f
	.long	.LLST34
	.uleb128 0x27
	.long	.LASF96
	.byte	0x1
	.value	0x14f
	.long	0x7f
	.long	.LLST35
	.uleb128 0x29
	.long	0x363
	.long	.LBB47
	.long	.Ldebug_ranges0+0xc0
	.byte	0x1
	.value	0x16d
	.long	0x8f5
	.uleb128 0x23
	.long	0x37d
	.long	.LLST36
	.uleb128 0x23
	.long	0x373
	.long	.LLST37
	.uleb128 0x1a
	.long	.Ldebug_ranges0+0xc0
	.uleb128 0x14
	.long	0x387
	.long	.LLST38
	.byte	0
	.byte	0
	.uleb128 0x1d
	.long	.LVL110
	.long	0x47f
	.byte	0
	.uleb128 0x25
	.long	.LASF97
	.byte	0x1
	.value	0x17f
	.long	0x5b
	.long	.LFB23
	.long	.LFE23-.LFB23
	.uleb128 0x1
	.byte	0x9c
	.long	0x934
	.uleb128 0x2c
	.long	.LASF81
	.byte	0x1
	.value	0x181
	.long	0x393
	.uleb128 0x28
	.string	"i"
	.byte	0x1
	.value	0x182
	.long	0x7f
	.long	.LLST39
	.byte	0
	.uleb128 0x2d
	.long	.LASF107
	.byte	0x1
	.value	0x18c
	.long	0x5b
	.long	.LFB24
	.long	.LFE24-.LFB24
	.uleb128 0x1
	.byte	0x9c
	.uleb128 0x20
	.long	.LASF98
	.byte	0x1
	.byte	0x28
	.long	0x358
	.uleb128 0x5
	.byte	0x3
	.long	s_szVgaInfo
	.uleb128 0x2e
	.long	.LASF99
	.byte	0x5
	.byte	0xa8
	.long	0x2d1
	.uleb128 0x2e
	.long	.LASF100
	.byte	0x5
	.byte	0xa9
	.long	0x2d1
	.uleb128 0x2f
	.long	.LASF108
	.long	0xcd
	.long	0x98e
	.uleb128 0x30
	.long	0xcd
	.uleb128 0x30
	.long	0x98e
	.uleb128 0x30
	.long	0xcf
	.byte	0
	.uleb128 0x6
	.byte	0x4
	.long	0x994
	.uleb128 0x31
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
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x7
	.uleb128 0x26
	.byte	0
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x8
	.uleb128 0xf
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
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
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x11
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
	.byte	0
	.byte	0
	.uleb128 0x12
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
	.uleb128 0x13
	.uleb128 0x5
	.byte	0
	.uleb128 0x31
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x18
	.byte	0
	.byte	0
	.uleb128 0x14
	.uleb128 0x34
	.byte	0
	.uleb128 0x31
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x17
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
	.uleb128 0x16
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
	.uleb128 0x17
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
	.uleb128 0xb
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x19
	.uleb128 0x5
	.byte	0
	.uleb128 0x31
	.uleb128 0x13
	.uleb128 0x1c
	.uleb128 0xb
	.byte	0
	.byte	0
	.uleb128 0x1a
	.uleb128 0xb
	.byte	0x1
	.uleb128 0x55
	.uleb128 0x17
	.byte	0
	.byte	0
	.uleb128 0x1b
	.uleb128 0x34
	.byte	0
	.uleb128 0x31
	.uleb128 0x13
	.uleb128 0x1c
	.uleb128 0xb
	.byte	0
	.byte	0
	.uleb128 0x1c
	.uleb128 0x34
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
	.uleb128 0x17
	.byte	0
	.byte	0
	.uleb128 0x1d
	.uleb128 0x4109
	.byte	0
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x31
	.uleb128 0x13
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
	.uleb128 0x1f
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
	.uleb128 0x20
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
	.uleb128 0x21
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
	.uleb128 0x22
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
	.uleb128 0x23
	.uleb128 0x5
	.byte	0
	.uleb128 0x31
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x17
	.byte	0
	.byte	0
	.uleb128 0x24
	.uleb128 0xb
	.byte	0x1
	.uleb128 0x55
	.uleb128 0x17
	.uleb128 0x1
	.uleb128 0x13
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
	.uleb128 0x26
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
	.uleb128 0x27
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
	.uleb128 0x28
	.uleb128 0x34
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
	.uleb128 0x29
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
	.uleb128 0x2a
	.uleb128 0xb
	.byte	0x1
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x6
	.byte	0
	.byte	0
	.uleb128 0x2b
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
	.byte	0
	.byte	0
	.uleb128 0x2c
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
	.uleb128 0x2d
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
	.uleb128 0x2e
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
	.uleb128 0x2f
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3f
	.uleb128 0x19
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x27
	.uleb128 0x19
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x34
	.uleb128 0x19
	.uleb128 0x3c
	.uleb128 0x19
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x30
	.uleb128 0x5
	.byte	0
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x31
	.uleb128 0x26
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.section	.debug_loc,"",@progbits
.Ldebug_loc0:
.LLST0:
	.long	.LVL0-.Ltext0
	.long	.LVL1-.Ltext0
	.value	0x2
	.byte	0x30
	.byte	0x9f
	.long	.LVL1-.Ltext0
	.long	.LVL2-.Ltext0
	.value	0x17
	.byte	0x71
	.sleb128 0
	.byte	0xa
	.value	0xffff
	.byte	0x1a
	.byte	0x72
	.sleb128 0
	.byte	0xa
	.value	0xffff
	.byte	0x1a
	.byte	0x1e
	.byte	0x70
	.sleb128 0
	.byte	0xa
	.value	0xffff
	.byte	0x1a
	.byte	0x22
	.byte	0x31
	.byte	0x24
	.byte	0x9f
	.long	.LVL2-.Ltext0
	.long	.LVL3-.Ltext0
	.value	0x1c
	.byte	0x71
	.sleb128 0
	.byte	0xa
	.value	0xffff
	.byte	0x1a
	.byte	0x3
	.long	s_szVgaInfo+6
	.byte	0x94
	.byte	0x2
	.byte	0xa
	.value	0xffff
	.byte	0x1a
	.byte	0x1e
	.byte	0x70
	.sleb128 0
	.byte	0xa
	.value	0xffff
	.byte	0x1a
	.byte	0x22
	.byte	0x31
	.byte	0x24
	.byte	0x9f
	.long	.LVL3-.Ltext0
	.long	.LVL4-.Ltext0
	.value	0x1e
	.byte	0x71
	.sleb128 0
	.byte	0xa
	.value	0xffff
	.byte	0x1a
	.byte	0x3
	.long	s_szVgaInfo+6
	.byte	0x94
	.byte	0x2
	.byte	0xa
	.value	0xffff
	.byte	0x1a
	.byte	0x1e
	.byte	0x91
	.sleb128 0
	.byte	0x94
	.byte	0x2
	.byte	0xa
	.value	0xffff
	.byte	0x1a
	.byte	0x22
	.byte	0x31
	.byte	0x24
	.byte	0x9f
	.long	.LVL4-.Ltext0
	.long	.LFE11-.Ltext0
	.value	0x2
	.byte	0x30
	.byte	0x9f
	.long	0
	.long	0
.LLST1:
	.long	.LVL6-.Ltext0
	.long	.LVL9-.Ltext0
	.value	0xc
	.byte	0x73
	.sleb128 0
	.byte	0xa
	.value	0xffff
	.byte	0x1a
	.byte	0x31
	.byte	0x24
	.byte	0x77
	.sleb128 0
	.byte	0x22
	.byte	0x9f
	.long	.LVL10-.Ltext0
	.long	.LVL17-.Ltext0
	.value	0xc
	.byte	0x73
	.sleb128 0
	.byte	0xa
	.value	0xffff
	.byte	0x1a
	.byte	0x31
	.byte	0x24
	.byte	0x77
	.sleb128 0
	.byte	0x22
	.byte	0x9f
	.long	0
	.long	0
.LLST2:
	.long	.LVL7-.Ltext0
	.long	.LVL8-.Ltext0
	.value	0x15
	.byte	0x70
	.sleb128 0
	.byte	0xa
	.value	0xffff
	.byte	0x1a
	.byte	0x31
	.byte	0x1c
	.byte	0x73
	.sleb128 0
	.byte	0xa
	.value	0xffff
	.byte	0x1a
	.byte	0x1e
	.byte	0x31
	.byte	0x24
	.byte	0x77
	.sleb128 0
	.byte	0x22
	.byte	0x9f
	.long	.LVL10-.Ltext0
	.long	.LVL11-.Ltext0
	.value	0x15
	.byte	0x70
	.sleb128 0
	.byte	0xa
	.value	0xffff
	.byte	0x1a
	.byte	0x31
	.byte	0x1c
	.byte	0x73
	.sleb128 0
	.byte	0xa
	.value	0xffff
	.byte	0x1a
	.byte	0x1e
	.byte	0x31
	.byte	0x24
	.byte	0x77
	.sleb128 0
	.byte	0x22
	.byte	0x9f
	.long	.LVL11-.Ltext0
	.long	.LVL14-1-.Ltext0
	.value	0x1a
	.byte	0x3
	.long	s_szVgaInfo+8
	.byte	0x94
	.byte	0x2
	.byte	0xa
	.value	0xffff
	.byte	0x1a
	.byte	0x31
	.byte	0x1c
	.byte	0x73
	.sleb128 0
	.byte	0xa
	.value	0xffff
	.byte	0x1a
	.byte	0x1e
	.byte	0x31
	.byte	0x24
	.byte	0x77
	.sleb128 0
	.byte	0x22
	.byte	0x9f
	.long	0
	.long	0
.LLST3:
	.long	.LVL12-.Ltext0
	.long	.LVL13-.Ltext0
	.value	0x5
	.byte	0x76
	.sleb128 0
	.byte	0x31
	.byte	0x24
	.byte	0x9f
	.long	.LVL13-.Ltext0
	.long	.LVL18-.Ltext0
	.value	0x1
	.byte	0x56
	.long	0
	.long	0
.LLST4:
	.long	.LVL12-.Ltext0
	.long	.LVL15-.Ltext0
	.value	0x2
	.byte	0x30
	.byte	0x9f
	.long	.LVL15-.Ltext0
	.long	.LFE12-.Ltext0
	.value	0x1
	.byte	0x50
	.long	0
	.long	0
.LLST5:
	.long	.LVL19-.Ltext0
	.long	.LVL20-.Ltext0
	.value	0x2
	.byte	0x30
	.byte	0x9f
	.long	.LVL20-.Ltext0
	.long	.LVL21-.Ltext0
	.value	0x1
	.byte	0x50
	.long	.LVL22-.Ltext0
	.long	.LFE14-.Ltext0
	.value	0x2
	.byte	0x30
	.byte	0x9f
	.long	0
	.long	0
.LLST6:
	.long	.LVL19-.Ltext0
	.long	.LVL20-.Ltext0
	.value	0x2
	.byte	0x30
	.byte	0x9f
	.long	.LVL20-.Ltext0
	.long	.LVL21-.Ltext0
	.value	0x5
	.byte	0x70
	.sleb128 0
	.byte	0x38
	.byte	0x25
	.byte	0x9f
	.long	.LVL22-.Ltext0
	.long	.LFE14-.Ltext0
	.value	0x2
	.byte	0x30
	.byte	0x9f
	.long	0
	.long	0
.LLST7:
	.long	.LVL27-.Ltext0
	.long	.LVL31-.Ltext0
	.value	0x1
	.byte	0x52
	.long	.LVL34-.Ltext0
	.long	.LVL36-.Ltext0
	.value	0x2
	.byte	0x30
	.byte	0x9f
	.long	.LVL36-.Ltext0
	.long	.LFE17-.Ltext0
	.value	0x1
	.byte	0x52
	.long	0
	.long	0
.LLST8:
	.long	.LVL28-.Ltext0
	.long	.LVL31-.Ltext0
	.value	0x1
	.byte	0x56
	.long	.LVL34-.Ltext0
	.long	.LVL36-.Ltext0
	.value	0x3
	.byte	0x76
	.sleb128 1
	.byte	0x9f
	.long	.LVL36-.Ltext0
	.long	.LFE17-.Ltext0
	.value	0x1
	.byte	0x56
	.long	0
	.long	0
.LLST9:
	.long	.LVL28-.Ltext0
	.long	.LVL33-.Ltext0
	.value	0x1
	.byte	0x56
	.long	.LVL34-.Ltext0
	.long	.LFE17-.Ltext0
	.value	0x1
	.byte	0x56
	.long	0
	.long	0
.LLST10:
	.long	.LVL28-.Ltext0
	.long	.LVL30-.Ltext0
	.value	0x1
	.byte	0x52
	.long	.LVL30-.Ltext0
	.long	.LVL32-1-.Ltext0
	.value	0x5
	.byte	0x3
	.long	s_szVgaInfo+10
	.long	.LVL34-.Ltext0
	.long	.LVL36-.Ltext0
	.value	0x5
	.byte	0x3
	.long	s_szVgaInfo+10
	.long	.LVL36-.Ltext0
	.long	.LFE17-.Ltext0
	.value	0x1
	.byte	0x52
	.long	0
	.long	0
.LLST11:
	.long	.LVL28-.Ltext0
	.long	.LVL29-.Ltext0
	.value	0x2
	.byte	0x30
	.byte	0x9f
	.long	.LVL29-.Ltext0
	.long	.LVL30-.Ltext0
	.value	0x13
	.byte	0x73
	.sleb128 0
	.byte	0xa
	.value	0xffff
	.byte	0x1a
	.byte	0x70
	.sleb128 0
	.byte	0x1e
	.byte	0x72
	.sleb128 0
	.byte	0xa
	.value	0xffff
	.byte	0x1a
	.byte	0x22
	.byte	0x31
	.byte	0x24
	.byte	0x9f
	.long	.LVL30-.Ltext0
	.long	.LVL31-.Ltext0
	.value	0x18
	.byte	0x73
	.sleb128 0
	.byte	0xa
	.value	0xffff
	.byte	0x1a
	.byte	0x70
	.sleb128 0
	.byte	0x1e
	.byte	0x3
	.long	s_szVgaInfo+10
	.byte	0x94
	.byte	0x2
	.byte	0xa
	.value	0xffff
	.byte	0x1a
	.byte	0x22
	.byte	0x31
	.byte	0x24
	.byte	0x9f
	.long	.LVL31-.Ltext0
	.long	.LVL32-1-.Ltext0
	.value	0x1c
	.byte	0x73
	.sleb128 0
	.byte	0xa
	.value	0xffff
	.byte	0x1a
	.byte	0x76
	.sleb128 0
	.byte	0xa
	.value	0xffff
	.byte	0x1a
	.byte	0x1e
	.byte	0x3
	.long	s_szVgaInfo+10
	.byte	0x94
	.byte	0x2
	.byte	0xa
	.value	0xffff
	.byte	0x1a
	.byte	0x22
	.byte	0x31
	.byte	0x24
	.byte	0x9f
	.long	.LVL34-.Ltext0
	.long	.LVL35-.Ltext0
	.value	0x18
	.byte	0x73
	.sleb128 0
	.byte	0xa
	.value	0xffff
	.byte	0x1a
	.byte	0x70
	.sleb128 0
	.byte	0x1e
	.byte	0x3
	.long	s_szVgaInfo+10
	.byte	0x94
	.byte	0x2
	.byte	0xa
	.value	0xffff
	.byte	0x1a
	.byte	0x22
	.byte	0x31
	.byte	0x24
	.byte	0x9f
	.long	.LVL35-.Ltext0
	.long	.LVL36-.Ltext0
	.value	0x1c
	.byte	0x73
	.sleb128 0
	.byte	0xa
	.value	0xffff
	.byte	0x1a
	.byte	0x76
	.sleb128 0
	.byte	0xa
	.value	0xffff
	.byte	0x1a
	.byte	0x1e
	.byte	0x3
	.long	s_szVgaInfo+10
	.byte	0x94
	.byte	0x2
	.byte	0xa
	.value	0xffff
	.byte	0x1a
	.byte	0x22
	.byte	0x31
	.byte	0x24
	.byte	0x9f
	.long	.LVL36-.Ltext0
	.long	.LVL37-.Ltext0
	.value	0x13
	.byte	0x73
	.sleb128 0
	.byte	0xa
	.value	0xffff
	.byte	0x1a
	.byte	0x70
	.sleb128 0
	.byte	0x1e
	.byte	0x72
	.sleb128 0
	.byte	0xa
	.value	0xffff
	.byte	0x1a
	.byte	0x22
	.byte	0x31
	.byte	0x24
	.byte	0x9f
	.long	.LVL37-.Ltext0
	.long	.LFE17-.Ltext0
	.value	0x17
	.byte	0x73
	.sleb128 0
	.byte	0xa
	.value	0xffff
	.byte	0x1a
	.byte	0x76
	.sleb128 0
	.byte	0xa
	.value	0xffff
	.byte	0x1a
	.byte	0x1e
	.byte	0x72
	.sleb128 0
	.byte	0xa
	.value	0xffff
	.byte	0x1a
	.byte	0x22
	.byte	0x31
	.byte	0x24
	.byte	0x9f
	.long	0
	.long	0
.LLST12:
	.long	.LVL43-.Ltext0
	.long	.LVL47-.Ltext0
	.value	0x1
	.byte	0x53
	.long	.LVL47-.Ltext0
	.long	.LVL48-.Ltext0
	.value	0x3
	.byte	0x73
	.sleb128 -2
	.byte	0x9f
	.long	.LVL48-.Ltext0
	.long	.LVL57-.Ltext0
	.value	0x1
	.byte	0x53
	.long	0
	.long	0
.LLST13:
	.long	.LVL42-.Ltext0
	.long	.LVL59-.Ltext0
	.value	0x1
	.byte	0x55
	.long	.LVL60-.Ltext0
	.long	.LVL64-.Ltext0
	.value	0x1
	.byte	0x55
	.long	.LVL65-.Ltext0
	.long	.LFE18-.Ltext0
	.value	0x1
	.byte	0x55
	.long	0
	.long	0
.LLST14:
	.long	.LVL42-.Ltext0
	.long	.LVL46-.Ltext0
	.value	0x1
	.byte	0x56
	.long	.LVL49-.Ltext0
	.long	.LVL50-.Ltext0
	.value	0x1
	.byte	0x56
	.long	.LVL50-.Ltext0
	.long	.LVL56-.Ltext0
	.value	0x2
	.byte	0x30
	.byte	0x9f
	.long	.LVL56-.Ltext0
	.long	.LVL59-.Ltext0
	.value	0x1
	.byte	0x56
	.long	.LVL60-.Ltext0
	.long	.LVL62-.Ltext0
	.value	0x1
	.byte	0x56
	.long	.LVL65-.Ltext0
	.long	.LFE18-.Ltext0
	.value	0x1
	.byte	0x56
	.long	0
	.long	0
.LLST15:
	.long	.LVL42-.Ltext0
	.long	.LVL59-.Ltext0
	.value	0x1
	.byte	0x57
	.long	.LVL60-.Ltext0
	.long	.LVL63-.Ltext0
	.value	0x1
	.byte	0x57
	.long	.LVL65-.Ltext0
	.long	.LFE18-.Ltext0
	.value	0x1
	.byte	0x57
	.long	0
	.long	0
.LLST16:
	.long	.LVL40-.Ltext0
	.long	.LVL43-.Ltext0
	.value	0x1
	.byte	0x57
	.long	.LVL65-.Ltext0
	.long	.LFE18-.Ltext0
	.value	0x1
	.byte	0x57
	.long	0
	.long	0
.LLST17:
	.long	.LVL39-.Ltext0
	.long	.LVL43-.Ltext0
	.value	0x1
	.byte	0x56
	.long	.LVL65-.Ltext0
	.long	.LFE18-.Ltext0
	.value	0x1
	.byte	0x56
	.long	0
	.long	0
.LLST18:
	.long	.LVL40-.Ltext0
	.long	.LVL41-.Ltext0
	.value	0x2
	.byte	0x30
	.byte	0x9f
	.long	.LVL41-.Ltext0
	.long	.LVL43-.Ltext0
	.value	0x17
	.byte	0x77
	.sleb128 0
	.byte	0xa
	.value	0xffff
	.byte	0x1a
	.byte	0x71
	.sleb128 0
	.byte	0xa
	.value	0xffff
	.byte	0x1a
	.byte	0x1e
	.byte	0x76
	.sleb128 0
	.byte	0xa
	.value	0xffff
	.byte	0x1a
	.byte	0x22
	.byte	0x31
	.byte	0x24
	.byte	0x9f
	.long	.LVL65-.Ltext0
	.long	.LFE18-.Ltext0
	.value	0x17
	.byte	0x77
	.sleb128 0
	.byte	0xa
	.value	0xffff
	.byte	0x1a
	.byte	0x71
	.sleb128 0
	.byte	0xa
	.value	0xffff
	.byte	0x1a
	.byte	0x1e
	.byte	0x76
	.sleb128 0
	.byte	0xa
	.value	0xffff
	.byte	0x1a
	.byte	0x22
	.byte	0x31
	.byte	0x24
	.byte	0x9f
	.long	0
	.long	0
.LLST19:
	.long	.LVL43-.Ltext0
	.long	.LVL44-.Ltext0
	.value	0xb
	.byte	0x73
	.sleb128 -2
	.byte	0x94
	.byte	0x1
	.byte	0x8
	.byte	0xff
	.byte	0x1a
	.byte	0x23
	.uleb128 0x700
	.byte	0x9f
	.long	.LVL45-.Ltext0
	.long	.LVL51-.Ltext0
	.value	0x9
	.byte	0x70
	.sleb128 0
	.byte	0x8
	.byte	0xff
	.byte	0x1a
	.byte	0x23
	.uleb128 0x700
	.byte	0x9f
	.long	.LVL51-.Ltext0
	.long	.LVL54-1-.Ltext0
	.value	0xb
	.byte	0x73
	.sleb128 -2
	.byte	0x94
	.byte	0x1
	.byte	0x8
	.byte	0xff
	.byte	0x1a
	.byte	0x23
	.uleb128 0x700
	.byte	0x9f
	.long	0
	.long	0
.LLST20:
	.long	.LVL71-.Ltext0
	.long	.LVL85-.Ltext0
	.value	0x1
	.byte	0x50
	.long	0
	.long	0
.LLST21:
	.long	.LVL71-.Ltext0
	.long	.LVL77-.Ltext0
	.value	0x1
	.byte	0x52
	.long	0
	.long	0
.LLST22:
	.long	.LVL71-.Ltext0
	.long	.LVL72-.Ltext0
	.value	0x6
	.byte	0x73
	.sleb128 0
	.byte	0x76
	.sleb128 0
	.byte	0x22
	.byte	0x9f
	.long	.LVL72-.Ltext0
	.long	.LVL75-.Ltext0
	.value	0x8
	.byte	0x73
	.sleb128 0
	.byte	0x91
	.sleb128 0
	.byte	0x94
	.byte	0x2
	.byte	0x22
	.byte	0x9f
	.long	.LVL76-.Ltext0
	.long	.LVL78-.Ltext0
	.value	0x8
	.byte	0x91
	.sleb128 -22
	.byte	0x94
	.byte	0x2
	.byte	0x73
	.sleb128 0
	.byte	0x22
	.byte	0x9f
	.long	0
	.long	0
.LLST23:
	.long	.LVL71-.Ltext0
	.long	.LVL79-.Ltext0
	.value	0x2
	.byte	0x30
	.byte	0x9f
	.long	.LVL79-.Ltext0
	.long	.LVL82-.Ltext0
	.value	0x6
	.byte	0x76
	.sleb128 0
	.byte	0x77
	.sleb128 0
	.byte	0x1c
	.byte	0x9f
	.long	.LVL82-.Ltext0
	.long	.LVL83-.Ltext0
	.value	0x7
	.byte	0x77
	.sleb128 0
	.byte	0x20
	.byte	0x76
	.sleb128 0
	.byte	0x22
	.byte	0x9f
	.long	.LVL83-.Ltext0
	.long	.LVL84-.Ltext0
	.value	0x6
	.byte	0x76
	.sleb128 0
	.byte	0x77
	.sleb128 0
	.byte	0x1c
	.byte	0x9f
	.long	0
	.long	0
.LLST24:
	.long	.LVL68-.Ltext0
	.long	.LVL70-.Ltext0
	.value	0x1
	.byte	0x53
	.long	.LVL70-.Ltext0
	.long	.LVL75-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 4
	.long	.LVL86-.Ltext0
	.long	.LFE20-.Ltext0
	.value	0x1
	.byte	0x53
	.long	0
	.long	0
.LLST25:
	.long	.LVL68-.Ltext0
	.long	.LVL72-.Ltext0
	.value	0x1
	.byte	0x56
	.long	.LVL72-.Ltext0
	.long	.LVL75-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 0
	.long	.LVL86-.Ltext0
	.long	.LFE20-.Ltext0
	.value	0x1
	.byte	0x56
	.long	0
	.long	0
.LLST26:
	.long	.LVL68-.Ltext0
	.long	.LVL69-.Ltext0
	.value	0x2
	.byte	0x30
	.byte	0x9f
	.long	.LVL86-.Ltext0
	.long	.LVL87-.Ltext0
	.value	0x17
	.byte	0x73
	.sleb128 0
	.byte	0xa
	.value	0xffff
	.byte	0x1a
	.byte	0x72
	.sleb128 0
	.byte	0xa
	.value	0xffff
	.byte	0x1a
	.byte	0x1e
	.byte	0x76
	.sleb128 0
	.byte	0xa
	.value	0xffff
	.byte	0x1a
	.byte	0x22
	.byte	0x31
	.byte	0x24
	.byte	0x9f
	.long	.LVL87-.Ltext0
	.long	.LFE20-.Ltext0
	.value	0x1c
	.byte	0x73
	.sleb128 0
	.byte	0xa
	.value	0xffff
	.byte	0x1a
	.byte	0x3
	.long	s_szVgaInfo+6
	.byte	0x94
	.byte	0x2
	.byte	0xa
	.value	0xffff
	.byte	0x1a
	.byte	0x1e
	.byte	0x76
	.sleb128 0
	.byte	0xa
	.value	0xffff
	.byte	0x1a
	.byte	0x22
	.byte	0x31
	.byte	0x24
	.byte	0x9f
	.long	0
	.long	0
.LLST27:
	.long	.LVL73-.Ltext0
	.long	.LVL74-.Ltext0
	.value	0x8
	.byte	0x70
	.sleb128 0
	.byte	0x94
	.byte	0x1
	.byte	0x8
	.byte	0x7f
	.byte	0x1a
	.byte	0x9f
	.long	.LVL74-.Ltext0
	.long	.LVL75-.Ltext0
	.value	0x8
	.byte	0x70
	.sleb128 -2
	.byte	0x94
	.byte	0x1
	.byte	0x8
	.byte	0x7f
	.byte	0x1a
	.byte	0x9f
	.long	.LVL80-.Ltext0
	.long	.LVL83-.Ltext0
	.value	0x1
	.byte	0x52
	.long	0
	.long	0
.LLST28:
	.long	.LVL95-.Ltext0
	.long	.LVL96-.Ltext0
	.value	0x1
	.byte	0x52
	.long	.LVL96-.Ltext0
	.long	.LVL100-.Ltext0
	.value	0x1
	.byte	0x50
	.long	.LVL100-.Ltext0
	.long	.LVL101-.Ltext0
	.value	0x3
	.byte	0x70
	.sleb128 -2
	.byte	0x9f
	.long	.LVL101-.Ltext0
	.long	.LVL102-.Ltext0
	.value	0x1
	.byte	0x50
	.long	0
	.long	0
.LLST29:
	.long	.LVL94-.Ltext0
	.long	.LVL99-.Ltext0
	.value	0x2
	.byte	0x30
	.byte	0x9f
	.long	0
	.long	0
.LLST30:
	.long	.LVL90-.Ltext0
	.long	.LVL97-.Ltext0
	.value	0x1
	.byte	0x56
	.long	.LVL97-.Ltext0
	.long	.LVL98-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 4
	.long	0
	.long	0
.LLST31:
	.long	.LVL89-.Ltext0
	.long	.LVL93-.Ltext0
	.value	0x1
	.byte	0x52
	.long	.LVL93-.Ltext0
	.long	.LVL98-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 0
	.long	0
	.long	0
.LLST32:
	.long	.LVL90-.Ltext0
	.long	.LVL91-.Ltext0
	.value	0x2
	.byte	0x30
	.byte	0x9f
	.long	.LVL91-.Ltext0
	.long	.LVL92-.Ltext0
	.value	0x17
	.byte	0x76
	.sleb128 0
	.byte	0xa
	.value	0xffff
	.byte	0x1a
	.byte	0x71
	.sleb128 0
	.byte	0xa
	.value	0xffff
	.byte	0x1a
	.byte	0x1e
	.byte	0x72
	.sleb128 0
	.byte	0xa
	.value	0xffff
	.byte	0x1a
	.byte	0x22
	.byte	0x31
	.byte	0x24
	.byte	0x9f
	.long	.LVL92-.Ltext0
	.long	.LVL93-.Ltext0
	.value	0x1c
	.byte	0x76
	.sleb128 0
	.byte	0xa
	.value	0xffff
	.byte	0x1a
	.byte	0x3
	.long	s_szVgaInfo+6
	.byte	0x94
	.byte	0x2
	.byte	0xa
	.value	0xffff
	.byte	0x1a
	.byte	0x1e
	.byte	0x72
	.sleb128 0
	.byte	0xa
	.value	0xffff
	.byte	0x1a
	.byte	0x22
	.byte	0x31
	.byte	0x24
	.byte	0x9f
	.long	.LVL93-.Ltext0
	.long	.LVL97-.Ltext0
	.value	0x1e
	.byte	0x76
	.sleb128 0
	.byte	0xa
	.value	0xffff
	.byte	0x1a
	.byte	0x3
	.long	s_szVgaInfo+6
	.byte	0x94
	.byte	0x2
	.byte	0xa
	.value	0xffff
	.byte	0x1a
	.byte	0x1e
	.byte	0x91
	.sleb128 0
	.byte	0x94
	.byte	0x2
	.byte	0xa
	.value	0xffff
	.byte	0x1a
	.byte	0x22
	.byte	0x31
	.byte	0x24
	.byte	0x9f
	.long	.LVL97-.Ltext0
	.long	.LVL98-.Ltext0
	.value	0x20
	.byte	0x91
	.sleb128 4
	.byte	0x94
	.byte	0x2
	.byte	0xa
	.value	0xffff
	.byte	0x1a
	.byte	0x3
	.long	s_szVgaInfo+6
	.byte	0x94
	.byte	0x2
	.byte	0xa
	.value	0xffff
	.byte	0x1a
	.byte	0x1e
	.byte	0x91
	.sleb128 0
	.byte	0x94
	.byte	0x2
	.byte	0xa
	.value	0xffff
	.byte	0x1a
	.byte	0x22
	.byte	0x31
	.byte	0x24
	.byte	0x9f
	.long	0
	.long	0
.LLST33:
	.long	.LVL103-.Ltext0
	.long	.LVL107-.Ltext0
	.value	0x2
	.byte	0x30
	.byte	0x9f
	.long	.LVL107-.Ltext0
	.long	.LVL108-.Ltext0
	.value	0x1
	.byte	0x50
	.long	.LVL108-.Ltext0
	.long	.LVL109-.Ltext0
	.value	0x3
	.byte	0x70
	.sleb128 -2
	.byte	0x9f
	.long	.LVL109-.Ltext0
	.long	.LVL110-1-.Ltext0
	.value	0x1
	.byte	0x50
	.long	.LVL111-.Ltext0
	.long	.LFE22-.Ltext0
	.value	0x2
	.byte	0x30
	.byte	0x9f
	.long	0
	.long	0
.LLST34:
	.long	.LVL103-.Ltext0
	.long	.LVL105-.Ltext0
	.value	0x2
	.byte	0x30
	.byte	0x9f
	.long	.LVL105-.Ltext0
	.long	.LVL107-.Ltext0
	.value	0x1
	.byte	0x50
	.long	.LVL111-.Ltext0
	.long	.LVL112-.Ltext0
	.value	0x2
	.byte	0x30
	.byte	0x9f
	.long	.LVL112-.Ltext0
	.long	.LVL115-.Ltext0
	.value	0x1
	.byte	0x50
	.long	.LVL116-.Ltext0
	.long	.LVL118-.Ltext0
	.value	0x2
	.byte	0x30
	.byte	0x9f
	.long	.LVL118-.Ltext0
	.long	.LVL120-.Ltext0
	.value	0x1
	.byte	0x50
	.long	0
	.long	0
.LLST35:
	.long	.LVL104-.Ltext0
	.long	.LVL106-.Ltext0
	.value	0x1
	.byte	0x53
	.long	.LVL111-.Ltext0
	.long	.LVL114-.Ltext0
	.value	0x1
	.byte	0x53
	.long	.LVL116-.Ltext0
	.long	.LVL117-.Ltext0
	.value	0x1
	.byte	0x53
	.long	.LVL117-.Ltext0
	.long	.LVL118-.Ltext0
	.value	0x3
	.byte	0x73
	.sleb128 1
	.byte	0x9f
	.long	.LVL118-.Ltext0
	.long	.LVL119-.Ltext0
	.value	0x1
	.byte	0x53
	.long	0
	.long	0
.LLST36:
	.long	.LVL105-.Ltext0
	.long	.LVL107-.Ltext0
	.value	0x1
	.byte	0x53
	.long	.LVL113-.Ltext0
	.long	.LVL116-.Ltext0
	.value	0x1
	.byte	0x53
	.long	.LVL119-.Ltext0
	.long	.LFE22-.Ltext0
	.value	0x1
	.byte	0x53
	.long	0
	.long	0
.LLST37:
	.long	.LVL105-.Ltext0
	.long	.LVL107-.Ltext0
	.value	0x1
	.byte	0x50
	.long	.LVL113-.Ltext0
	.long	.LVL115-.Ltext0
	.value	0x1
	.byte	0x50
	.long	.LVL115-.Ltext0
	.long	.LVL116-.Ltext0
	.value	0x1
	.byte	0x51
	.long	.LVL119-.Ltext0
	.long	.LVL120-.Ltext0
	.value	0x1
	.byte	0x50
	.long	.LVL120-.Ltext0
	.long	.LFE22-.Ltext0
	.value	0x1
	.byte	0x51
	.long	0
	.long	0
.LLST38:
	.long	.LVL105-.Ltext0
	.long	.LVL107-.Ltext0
	.value	0x2
	.byte	0x30
	.byte	0x9f
	.long	.LVL113-.Ltext0
	.long	.LVL116-.Ltext0
	.value	0x2
	.byte	0x30
	.byte	0x9f
	.long	.LVL119-.Ltext0
	.long	.LVL121-.Ltext0
	.value	0xf
	.byte	0x72
	.sleb128 0
	.byte	0xa
	.value	0xffff
	.byte	0x1a
	.byte	0x73
	.sleb128 0
	.byte	0x1e
	.byte	0x71
	.sleb128 0
	.byte	0x22
	.byte	0x31
	.byte	0x24
	.byte	0x9f
	.long	.LVL121-.Ltext0
	.long	.LVL122-.Ltext0
	.value	0xf
	.byte	0x70
	.sleb128 0
	.byte	0xa
	.value	0xffff
	.byte	0x1a
	.byte	0x73
	.sleb128 0
	.byte	0x1e
	.byte	0x71
	.sleb128 0
	.byte	0x22
	.byte	0x31
	.byte	0x24
	.byte	0x9f
	.long	0
	.long	0
.LLST39:
	.long	.LVL123-.Ltext0
	.long	.LVL124-.Ltext0
	.value	0x2
	.byte	0x30
	.byte	0x9f
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
	.long	.LBB17-.Ltext0
	.long	.LBE17-.Ltext0
	.long	.LBB20-.Ltext0
	.long	.LBE20-.Ltext0
	.long	0
	.long	0
	.long	.LBB21-.Ltext0
	.long	.LBE21-.Ltext0
	.long	.LBB22-.Ltext0
	.long	.LBE22-.Ltext0
	.long	.LBB23-.Ltext0
	.long	.LBE23-.Ltext0
	.long	0
	.long	0
	.long	.LBB24-.Ltext0
	.long	.LBE24-.Ltext0
	.long	.LBB27-.Ltext0
	.long	.LBE27-.Ltext0
	.long	0
	.long	0
	.long	.LBB28-.Ltext0
	.long	.LBE28-.Ltext0
	.long	.LBB31-.Ltext0
	.long	.LBE31-.Ltext0
	.long	0
	.long	0
	.long	.LBB32-.Ltext0
	.long	.LBE32-.Ltext0
	.long	.LBB33-.Ltext0
	.long	.LBE33-.Ltext0
	.long	0
	.long	0
	.long	.LBB34-.Ltext0
	.long	.LBE34-.Ltext0
	.long	.LBB39-.Ltext0
	.long	.LBE39-.Ltext0
	.long	.LBB40-.Ltext0
	.long	.LBE40-.Ltext0
	.long	.LBB42-.Ltext0
	.long	.LBE42-.Ltext0
	.long	0
	.long	0
	.long	.LBB43-.Ltext0
	.long	.LBE43-.Ltext0
	.long	.LBB46-.Ltext0
	.long	.LBE46-.Ltext0
	.long	0
	.long	0
	.long	.LBB47-.Ltext0
	.long	.LBE47-.Ltext0
	.long	.LBB51-.Ltext0
	.long	.LBE51-.Ltext0
	.long	.LBB52-.Ltext0
	.long	.LBE52-.Ltext0
	.long	0
	.long	0
	.section	.debug_line,"",@progbits
.Ldebug_line0:
	.section	.debug_str,"MS",@progbits,1
.LASF66:
	.string	"pBaseAddr"
.LASF19:
	.string	"__quad_t"
.LASF39:
	.string	"_old_offset"
.LASF6:
	.string	"BOOL"
.LASF94:
	.string	"nDelMode"
.LASF80:
	.string	"VGA_PrintChar"
.LASF83:
	.string	"pString"
.LASF72:
	.string	"cursor_h"
.LASF84:
	.string	"VGA_GetCursorPos"
.LASF95:
	.string	"nCharDelX"
.LASF34:
	.string	"_IO_save_end"
.LASF29:
	.string	"_IO_write_end"
.LASF9:
	.string	"short int"
.LASF78:
	.string	"pLines"
.LASF15:
	.string	"sizetype"
.LASF44:
	.string	"_offset"
.LASF65:
	.string	"nAddrOffset"
.LASF89:
	.string	"InputEnd"
.LASF73:
	.string	"cursor_l"
.LASF69:
	.string	"nMoveLen"
.LASF23:
	.string	"_flags"
.LASF108:
	.string	"memcpy"
.LASF0:
	.string	"BYTE"
.LASF102:
	.string	"biosvga.c"
.LASF101:
	.string	"GNU C 4.9.2 20150212 (Red Hat 4.9.2-6) -m32 -mtune=generic -march=i686 -g -O2"
.LASF35:
	.string	"_markers"
.LASF25:
	.string	"_IO_read_end"
.LASF1:
	.string	"WORD"
.LASF74:
	.string	"_VGA_ScrollLine"
.LASF68:
	.string	"pEndLine"
.LASF64:
	.string	"VGA_DISPLAY_INFO"
.LASF98:
	.string	"s_szVgaInfo"
.LASF105:
	.string	"_VGA_GetDisplayAddr"
.LASF62:
	.string	"CursorX"
.LASF63:
	.string	"CursorY"
.LASF60:
	.string	"Colums"
.LASF88:
	.string	"nBufLen"
.LASF12:
	.string	"float"
.LASF82:
	.string	"VGA_PrintString"
.LASF17:
	.string	"long long int"
.LASF71:
	.string	"cursor_index"
.LASF43:
	.string	"_lock"
.LASF21:
	.string	"long int"
.LASF97:
	.string	"VGA_Clear"
.LASF87:
	.string	"VGA_GetString"
.LASF7:
	.string	"CHAR"
.LASF40:
	.string	"_cur_column"
.LASF56:
	.string	"_pos"
.LASF103:
	.string	"/media/gaojie/Dev/hellox/HelloX_OS/kernel/arch/x86"
.LASF55:
	.string	"_sbuf"
.LASF52:
	.string	"_IO_FILE"
.LASF106:
	.string	"VGA_GetDisplayID"
.LASF4:
	.string	"DWORD"
.LASF76:
	.string	"VGA_SetCursorPos"
.LASF2:
	.string	"unsigned char"
.LASF16:
	.string	"signed char"
.LASF107:
	.string	"InitializeVGA"
.LASF18:
	.string	"long long unsigned int"
.LASF10:
	.string	"unsigned int"
.LASF53:
	.string	"_IO_marker"
.LASF42:
	.string	"_shortbuf"
.LASF13:
	.string	"LPSTR"
.LASF27:
	.string	"_IO_write_base"
.LASF51:
	.string	"_unused2"
.LASF24:
	.string	"_IO_read_ptr"
.LASF59:
	.string	"Lines"
.LASF90:
	.string	"StartIndex"
.LASF93:
	.string	"VGA_DelChar"
.LASF31:
	.string	"_IO_buf_end"
.LASF61:
	.string	"LastLine"
.LASF8:
	.string	"char"
.LASF70:
	.string	"bScrollUp"
.LASF81:
	.string	"pVideoBuf"
.LASF54:
	.string	"_next"
.LASF45:
	.string	"__pad1"
.LASF46:
	.string	"__pad2"
.LASF47:
	.string	"__pad3"
.LASF48:
	.string	"__pad4"
.LASF49:
	.string	"__pad5"
.LASF3:
	.string	"short unsigned int"
.LASF79:
	.string	"pColums"
.LASF5:
	.string	"long unsigned int"
.LASF28:
	.string	"_IO_write_ptr"
.LASF11:
	.string	"double"
.LASF22:
	.string	"__off64_t"
.LASF75:
	.string	"VGA_ChangeLine"
.LASF20:
	.string	"__off_t"
.LASF36:
	.string	"_chain"
.LASF58:
	.string	"pVideoAddr"
.LASF33:
	.string	"_IO_backup_base"
.LASF99:
	.string	"stdin"
.LASF30:
	.string	"_IO_buf_base"
.LASF92:
	.string	"nDelLen"
.LASF38:
	.string	"_flags2"
.LASF50:
	.string	"_mode"
.LASF26:
	.string	"_IO_read_base"
.LASF14:
	.string	"LPCSTR"
.LASF41:
	.string	"_vtable_offset"
.LASF57:
	.string	"tag__VGA_DISPLAY_INFO"
.LASF77:
	.string	"VGA_GetDisplayRange"
.LASF32:
	.string	"_IO_save_base"
.LASF67:
	.string	"pNextLine"
.LASF37:
	.string	"_fileno"
.LASF91:
	.string	"VGA_DelString"
.LASF85:
	.string	"pCursorX"
.LASF86:
	.string	"pCursorY"
.LASF96:
	.string	"nCharDelY"
.LASF100:
	.string	"stdout"
.LASF104:
	.string	"_IO_lock_t"
	.ident	"GCC: (GNU) 4.9.2 20150212 (Red Hat 4.9.2-6)"
	.section	.note.GNU-stack,"",@progbits
