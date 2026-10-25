;;------------------------------------------------------------------------
;;    Original Author                   : Garry
;;    Original Date                     : Mar,27,2004
;;    Original Finished date            :
;;    Module Name                       : miniker.asm
;;    Usage                             : countains the mini-kernal code.
;;    Defined procedure                 :
;;                                        1.
;;                                        2.
;;                                        3.
;;    Last modified author              : Garry
;;    Last modified date                :
;;    Last modified content             :
;;------------------------------------------------------------------------

;;------------------------------------------------------------------------
;;    MEMORY LAYOUT OF THIS MODULE:
;;        0x0000(0x100000) - 0x1FFF [8K]: Initialization code and sys
;;                                        tables,such as IDT,GDT...
;;        0x2000(0x102000) - 0x2FFF [4K]: Trampoline code for AP in SMP
;;        0x3000(0x103000) - 0x3FFF [4k]: Memory layout table.
;;        0x4000(0x104000) - 0x7FFF [16k]: System stack for system init.
;;        0x8000(0x108000) - 0xBFFF [16k]: Reserved.
;;------------------------------------------------------------------------

;;----------------------- ** Predefines and macros ** --------------------
;;  The following section defines some definations and macros,which can
;;  be used by the ASM code.
;;  NOTICE: In the following code section,may define other macros to use
;;  by local procedure,but these macros only used by the procedure define
;;  it,not 'GLOBAL'.
;;------------------------------------------------------------------------

	;Initial value of ESP in process of system initialization.
	%define DEF_INIT_ESP   0x108000

    con_org_start_addr equ 0x00002000  ;;When this module,mini-kernal is l-
                                       ;;oaded into memory,it resides at
                                       ;;con_org_start_addr,but we must re-
                                       ;;serve the uper 1M memory,so when
                                       ;;the control of CPU transfer to
                                       ;;the mini-kernal,it first moves i-
                                       ;;tself to con_start_addr(see bellow)
                                       ;;from con_org_start_addr,and then
                                       ;;jump to the proper address to exe-
                                       ;;cute.

    con_mini_start equ 0x00100000      ;;Start linear address of the mini-kernal.
    con_mast_start equ 0x00110000      ;;The master's start address when running.

	;;The end address of AP processor's stack pool.
	;;Each AP processor has one 16K bytes stack in this pool,
	;;use processor's ID as index.Processor with ID as N's stack
	;;top pointer is calculated as:
	;;stk_ptr = con_ap_stack_top - 16K * N
	;;This space of memory is free after the BSP is boot over,
	;;all loaded kernel modules in this space will be relocated
	;;to memory space high than 1M.So it can be used as AP
	;;stack safety.
	con_ap_stack_top equ 0xA0000
	con_ap_stack_size equ 0x4000

;;--------------------------- ** Module header ** ------------------------
;;  The following section is the module's header.
;;------------------------------------------------------------------------


    bits 32                      ;;The mini-kernal is a pure 32 bits OS ker-
                                 ;;nal.

    org 0x00100000               ;;The mini-kernal is loaded at the start
                                 ;;address 0x100000 of the linear address sp-
                                 ;;ace,so the uper 1M memory is reserved.
                                 ;;The uper 4K of this reserved area is co-
                                 ;;untains the system hardware information
                                 ;;filled by BIOS,and the DISPLAY memory a-
                                 ;;lso resides this area.
                                 ;;In additional,we can make other use of
                                 ;;this reserved memory.

	jmp gl_reinitgdt             ;;The first part of the mini-kernal image is
                                 ;;data section,so the first instruction must
                                 ;;to jump to the actualy code section,which
                                 ;;start at gl_sysredirect.

;;--------------------------- ** Data section ** -------------------------
;;  The following section defines the system's kernal data structures,such
;;  as the GDT,IDT,and other system variables.
;;------------------------------------------------------------------------

align 8
gl_sysdata_section:              ;;System data section,where countains the s-
                                 ;;ystem tables,such as GDT,IDT,and some ope-
                                 ;;rating system variables.

gl_sysgdt:                       ;;The start address of GDT.
                                 ;;In order to load the mini-kernal,the sys-
                                 ;;tem loader program,such as sysldrd.com(f-
                                 ;;or DOS) or sysldrb(for DISK),have initia-
                                 ;;lized the GDT,and make the code segment
                                 ;;and data segment can address the whole 32
                                 ;;bits linear address.
                                 ;;After the mini-kernal loaded,the control
                                 ;;transform to the OS kernal,so the kernal
                                 ;;will initialize the GDT again,this initi-
                                 ;;alization will make the GDT much proper.
    gl_gdt_null:
							dd 0 ;;The first entry of GDT must be NULL.
                            dd 0
    gl_gdt_syscode:
                            dw 0xFFFF
                            dw 0x0000
                            db 0x00
                            dw 0xCF9B
                            db 0x00
    gl_gdt_sysdata:
                            dw 0xFFFF
                            dw 0x0000
                            db 0x00
                            dw 0xCF93
                            db 0x00
    gl_gdt_sysstack:
                            dw 0xFFFF
                            dw 0x0000
                            db 0x00
                            dw 0xCF93
                            db 0x00
    gl_gdt_sysext:
                            dw 0xFFFF
                            dw 0x0000
                            db 0x00
                            dw 0xCF93
                            db 0x00
    gl_gdt_sysvga:               ;;Vga text mode base address,which can be
                                 ;;used by display driver.
                                 ;;This segment is loaded to gs segment.
                            dw 0x0048    ;;The segment's limit is 0x48 * 4K
                                         ;;= 288K,countains the vag text m-
                                         ;;ode buffer and graphic mode buf-
                                         ;;fer.
                            dw 0x8000
                            db 0x0b      ;;This segment's base address is
                                         ;;0x0b8000,which is the display's
                                         ;;text mode buffer address.
                            dw 0xc093
                            db 0x00
    gl_gdt_normal:
							dw 0xFFFF
                            dw 0x0000
                            db 0x00
                            dw 0x0092
                            db 0x00
    gl_gdt_code16:
							dw 0xFFFF
                            dw 0x0000
                            db 0x00
                            dw 0x0098
                            db 0x00
	gl_gdt_usrcode:
	                        dw 0xFFFF
							dw 0x0000
							db 0x00
							dw 0xCFFB
							db 0x00
	gl_gdt_usrdata:
	                        dw 0xFFFF
							dw 0x0000
							db 0x00
							dw 0xCFF3
							db 0x00
	;;Space reserved for TSS,each processor has one,64 elements
	;;at most.
	gl_gdt_tss_space:
	                        times 64 dd 0
							times 64 dd 0

align 4
    gl_gdtr_ldr:                 ;;The following variables countains info
                                 ;;about GDT table,which is used by lgdt
                                 ;;instruction.
		dw (74 * 8 - 1)
        dd gl_sysgdt             ;;The gdt base linear address.

align 8
    gl_sysidt:                   ;;The following area is used by system IDT
                                 ;;(Interrupt Descriptor Table).
                                 ;;The hardware(CPU) can only support 256
                                 ;;interrupts,for each interrupt,there is a
                                 ;;entry,so there should be reserved 2K sp-
                                 ;;ace for the IDT,but in PC archiecture,128
                                 ;;interrupts is enough,so we reserved 128
                                 ;;IDT entries.
        times 128 dd 0x00
        times 128 dd 0x00

    gl_trap_int_handler:         ;;The following array countains the inter-
                                 ;;rupt handlers offset,this array is used
                                 ;;to fill the idt.
                                 ;;We implement the interrupt handlers and
                                 ;;system call procedures in another code
                                 ;;segment,when Hello China initialized,it
                                 ;;load that segment into memory,and then
                                 ;;fill this array using that segment's data.
                                 ;;Hello China declares 128 interrupt handle-
                                 ;;rs,it's enough.
                                 ;;The first 32 IDT entries are trap gate,and
                                 ;;the following entries are interrupt gate,
                                 ;;so,we deal them separately.
                                 ;;When the Hello China's kernal,Master isn't
                                 ;;loaded,the mini-kernal initialize these
                                 ;;handlers using np_inth_tmp and np_traph_tmp.
        dd    gl_traph_tmp_00
        dd    gl_traph_tmp_01
        dd    gl_traph_tmp_02
        dd    gl_traph_tmp_03
        dd    gl_traph_tmp_04
        dd    gl_traph_tmp_05
        dd    gl_traph_tmp_06
        dd    gl_traph_tmp_07
        dd    gl_traph_tmp_08
        dd    gl_traph_tmp_09
        dd    gl_traph_tmp_0a
        dd    gl_traph_tmp_0b
        dd    gl_traph_tmp_0c
        dd    gl_traph_tmp_0d
        dd    gl_traph_tmp_0e
        dd    gl_traph_tmp_0f

        times 16 dd gl_traph_tmp

        dd    np_int20
        dd    np_int21
        dd    np_int22
        dd    np_int23
        dd    np_int24
        dd    np_int25
        dd    np_int26
        dd    np_int27
        dd    np_int28
        dd    np_int29
        dd    np_int2a
        dd    np_int2b
        dd    np_int2c
        dd    np_int2d
        dd    np_int2e
        dd    np_int2f

        times 79 dd np_inth_tmp

        dd    gl_syscall
		dd    gl_reschedule
        
    gl_idtr_ldr:
        dw 8*128 - 1             ;;The IDT table's limit
        dd gl_sysidt             ;;The IDT table's linear address.
                                 ;;This label,gl_idt_ldr,is used by the in-
                                 ;;struction lidt.

;;---------------- ** System initialize code section ** ------------------
;;  The following section countains the system initialize code,these code
;;  Initialize the CPU running context,such as GDT,IDT,and some system le-
;;  vel arrays.
;;------------------------------------------------------------------------

align 4
gl_reinitgdt:                    ;;The following code initializes the GDT
                                 ;;and all of the segment registers.
	lgdt [gl_gdtr_ldr]           ;;Load the new gdt content into gdt register.
    mov ax,0x010
    mov ds,ax
    mov ss,ax
    mov esp,DEF_INIT_ESP
    mov es,ax
    mov fs,ax
    mov gs,ax
    jmp dword 0x08 : gl_sysinit  ;;Far jump operation to renew the cs register.

gl_sysinit:                      ;;The start position of the init process.
    mov eax,gl_trap_int_handler
    push eax
    call np_fill_idt             ;;Initialize the IDT table.
    pop eax
	call np_init_syscall         ;;Initializes the system call function.
    lidt [gl_idtr_ldr]           ;;Load the idtr.
    call np_init8259             ;;Reinitialize the interrupt controller.
    nop
    nop
    nop
    mov eax,con_mast_start
    jmp eax

;;A helper routine to set a TSS into GDT table.
;;Parameters:
;;  %1 parameter: index of the TSS in GDT
;;  %2 parameter: first 4 bytes of GDT entry
;;  %3 parameter: second 4 bytes.
np_set_tss:
    push ebp
	mov ebp,esp
	xor eax,eax
	push esi
	push edi
	mov eax,dword [ebp + 0x08]
	shl eax,0x03   ;Locate the indexed GDT entry.
	mov edi,gl_gdt_tss_space
	add edi,eax
	mov esi,dword [ebp + 0x0C]
	mov dword [edi],esi
	add edi,0x04
	mov esi,dword [ebp + 0x10]
	mov dword [edi],esi
	mov eax,0x01
    pop edi
	pop esi
	leave
    ret

;;Helper routine to initializes the system call mechanism.
;;It just set up the IDT entry for system call(0x7F),the
;;difference between this entry and other interrupt entries
;;is,the DPL of this entry is 3,not 0,so the user code
;;can invoke it as the entry of system call.
np_init_syscall:
    push ebp
	mov ebp,esp
	push ebx
	push ecx
	push edi
	push esi
	mov edi,gl_sysidt
	mov eax,0x7F
	shl eax,0x03   ;8 bytes length for each IDT entry.
	add edi,eax
	;Now [edi] points to the system call's entry.
	mov ebx,gl_syscall
	mov eax,0x08   ;Code segment selector.
	shl eax,0x10
	mov ax,bx      ;Low 16 bits of handler.
	mov dword [edi],eax
	mov eax,ebx
	mov ax,0xEE00  ;Attribute byte of the entry.
	add edi,0x04
	mov dword [edi],eax
	pop esi
	pop edi
	pop ecx
	pop ebx
	leave
	ret

;;This procedure fills the IDT table.
np_fill_idt:                     
    push ebp
    mov ebp,esp

    %define DEF_TRAP_WORD_0 0x0000
    %define DEF_TRAP_WORD_1 0x0008
    %define DEF_TRAP_WORD_2 0x8f00
    %define DEF_TRAP_WORD_3 0x0000

    %define DEF_INT_WORD_0  0x0000
    %define DEF_INT_WORD_1  0x0008
    %define DEF_INT_WORD_2  0x8e00
    %define DEF_INT_WORD_3  0x0000

    push ecx
    push esi
    push edi
    push ebx
    mov esi,dword [ebp + 0x08]
    mov edi,gl_sysidt
    cld
    mov ecx,32                   ;;Init the first 32 entries of IDT table.
.ll_bgn1:
    mov eax,DEF_TRAP_WORD_1
    shl eax,0x010
    mov ebx,dword [esi]
    mov ax,bx
    stosd
    shr ebx,0x010
    mov ax,bx
    shl eax,0x010
    mov ax,DEF_TRAP_WORD_2
    stosd
    add esi,0x04
    loop .ll_bgn1

    mov ecx,96                   ;;Initialize the rest 96 entries of IDT.
.ll_bgn2:
    mov eax,DEF_INT_WORD_1
    shl eax,0x010
    mov ebx,dword [esi]
    mov ax,bx
    stosd
    shr ebx,0x010
    mov ax,bx
    shl eax,0x010
    mov ax,DEF_INT_WORD_2
    stosd
    add esi,0x04
    loop .ll_bgn2
    pop ebx
    pop edi
    pop esi
    pop ecx
    leave
    ret

;;Initializes interrupt controller chip.
np_init8259:
    mov al,0x11
    out 0x20,al
    nop
    nop
    out 0xa0,al

    mov al,0x20
    out 0x21,al
    mov al,0x28
    out 0xa1,al

    mov al,4
    out 0x21,al
    mov al,2
    out 0xa1,al

    mov al,1
    out 0x21,al
    out 0xa1,al

    mov al,0x00
    out 0x21,al
    mov al,0x00
    out 0xa1,al
    ret

;;------------------------- ** Temp procedures ** ------------------------
;;  The following defines some temp procedures,are used by the mini-kernal
;;  to fill the system tables.

np_traph_tmp:                    ;;This procedure is used to fill the first
                                 ;;32 entries of IDT table.
                                 ;;After the Master kernal is loaded,it wou-
                                 ;;ld replaced by others.
    push eax
    ;call np_formatdbgstr         ;;This procedure only print out somethings
                                 ;;then returned.
    ;call np_dbg_output
    mov al,0x20                  ;;Indicate the interrupt chip we have fin-
                                 ;;ished handle the interrupt.
                                 ;;:-)
    out 0x20,al
    out 0xa0,al
    pop eax
    pop eax
    iret                         ;;End of the procedure.

gl_traph_tmp:
    push eax
    cmp dword [gl_general_int_handler],0x00000000
    jz .ll_continue
    push ebx                     ;;The following code saves the general
                                 ;;registers.
    push ecx
    push edx
    push esi
    push edi
    push ebp
    mov eax,esp
    push eax
    mov eax,0x1F
    push eax
    call dword [gl_general_int_handler]
    pop eax                      ;;Restore the general registers.
    pop eax
    mov esp,eax
    pop ebp
    pop edi
    pop esi
    pop edx
    pop ecx
    pop ebx
.ll_continue:

    ;mov al,0x20                  ;;Indicate the interrupt chip we have fin-
                                 ;;ished handle the interrupt.
                                 ;;:-)
    ;out 0x20,al
    ;out 0xa0,al
    pop eax
    iret

gl_traph_tmp_00:
    push eax
    cmp dword [gl_general_int_handler],0x00000000
    jz .ll_continue
    push ebx                     ;;The following code saves the general
                                 ;;registers.
    push ecx
    push edx
    push esi
    push edi
    push ebp
    mov eax,esp
    push eax
    mov eax,0x00
    push eax
    call dword [gl_general_int_handler]
    pop eax                      ;;Restore the general registers.
    pop eax
    mov esp,eax
    pop ebp
    pop edi
    pop esi
    pop edx
    pop ecx
    pop ebx
.ll_continue:

    ;mov al,0x20                  ;;Indicate the interrupt chip we have fin-
                                 ;;ished handle the interrupt.
                                 ;;:-)
    ;out 0x20,al
    ;out 0xa0,al
    pop eax
    iret


gl_traph_tmp_01:
    push eax
    cmp dword [gl_general_int_handler],0x00000000
    jz .ll_continue
    push ebx                     ;;The following code saves the general
                                 ;;registers.
    push ecx
    push edx
    push esi
    push edi
    push ebp
    mov eax,esp
    push eax
    mov eax,0x01
    push eax
    call dword [gl_general_int_handler]
    pop eax                      ;;Restore the general registers.
    pop eax
    mov esp,eax
    pop ebp
    pop edi
    pop esi
    pop edx
    pop ecx
    pop ebx
.ll_continue:

    ;mov al,0x20                  ;;Indicate the interrupt chip we have fin-
                                 ;;ished handle the interrupt.
                                 ;;:-)
    ;out 0x20,al
    ;out 0xa0,al
    pop eax
    iret

gl_traph_tmp_02:
    push eax
    cmp dword [gl_general_int_handler],0x00000000
    jz .ll_continue
    push ebx                     ;;The following code saves the general
                                 ;;registers.
    push ecx
    push edx
    push esi
    push edi
    push ebp
    mov eax,esp
    push eax
    mov eax,0x02
    push eax
    call dword [gl_general_int_handler]
    pop eax                      ;;Restore the general registers.
    pop eax
    mov esp,eax
    pop ebp
    pop edi
    pop esi
    pop edx
    pop ecx
    pop ebx
.ll_continue:

    ;mov al,0x20                  ;;Indicate the interrupt chip we have fin-
                                 ;;ished handle the interrupt.
                                 ;;:-)
    ;out 0x20,al
    ;out 0xa0,al
    pop eax
    iret

gl_traph_tmp_03:
    push eax
    cmp dword [gl_general_int_handler],0x00000000
    jz .ll_continue
    push ebx                     ;;The following code saves the general
                                 ;;registers.
    push ecx
    push edx
    push esi
    push edi
    push ebp
    mov eax,esp
    push eax
    mov eax,0x03
    push eax
    call dword [gl_general_int_handler]
    pop eax                      ;;Restore the general registers.
    pop eax
    mov esp,eax
    pop ebp
    pop edi
    pop esi
    pop edx
    pop ecx
    pop ebx
.ll_continue:

    ;mov al,0x20                  ;;Indicate the interrupt chip we have fin-
                                 ;;ished handle the interrupt.
                                 ;;:-)
    ;out 0x20,al
    ;out 0xa0,al
    pop eax
    iret

gl_traph_tmp_04:
    push eax
    cmp dword [gl_general_int_handler],0x00000000
    jz .ll_continue
    push ebx                     ;;The following code saves the general
                                 ;;registers.
    push ecx
    push edx
    push esi
    push edi
    push ebp
    mov eax,esp
    push eax
    mov eax,0x04
    push eax
    call dword [gl_general_int_handler]
    pop eax                      ;;Restore the general registers.
    pop eax
    mov esp,eax
    pop ebp
    pop edi
    pop esi
    pop edx
    pop ecx
    pop ebx
.ll_continue:

    ;mov al,0x20                  ;;Indicate the interrupt chip we have fin-
                                 ;;ished handle the interrupt.
                                 ;;:-)
    ;out 0x20,al
    ;out 0xa0,al
    pop eax
    iret

gl_traph_tmp_05:
    push eax
    cmp dword [gl_general_int_handler],0x00000000
    jz .ll_continue
    push ebx                     ;;The following code saves the general
                                 ;;registers.
    push ecx
    push edx
    push esi
    push edi
    push ebp
    mov eax,esp
    push eax
    mov eax,0x05
    push eax
    call dword [gl_general_int_handler]
    pop eax                      ;;Restore the general registers.
    pop eax
    mov esp,eax
    pop ebp
    pop edi
    pop esi
    pop edx
    pop ecx
    pop ebx
.ll_continue:

    ;mov al,0x20                  ;;Indicate the interrupt chip we have fin-
                                 ;;ished handle the interrupt.
                                 ;;:-)
    ;out 0x20,al
    ;out 0xa0,al
    pop eax
    iret

gl_traph_tmp_06:
    push eax
    cmp dword [gl_general_int_handler],0x00000000
    jz .ll_continue
    push ebx                     ;;The following code saves the general
                                 ;;registers.
    push ecx
    push edx
    push esi
    push edi
    push ebp
    mov eax,esp
    push eax
    mov eax,0x06
    push eax
    call dword [gl_general_int_handler]
    pop eax                      ;;Restore the general registers.
    pop eax
    mov esp,eax
    pop ebp
    pop edi
    pop esi
    pop edx
    pop ecx
    pop ebx
.ll_continue:

    ;mov al,0x20                  ;;Indicate the interrupt chip we have fin-
                                 ;;ished handle the interrupt.
                                 ;;:-)
    ;out 0x20,al
    ;out 0xa0,al
    pop eax
    iret

gl_traph_tmp_07:
    push eax
    cmp dword [gl_general_int_handler],0x00000000
    jz .ll_continue
    push ebx                     ;;The following code saves the general
                                 ;;registers.
    push ecx
    push edx
    push esi
    push edi
    push ebp
    mov eax,esp
    push eax
    mov eax,0x07
    push eax
    call dword [gl_general_int_handler]
    pop eax                      ;;Restore the general registers.
    pop eax
    mov esp,eax
    pop ebp
    pop edi
    pop esi
    pop edx
    pop ecx
    pop ebx
.ll_continue:

    ;mov al,0x20                  ;;Indicate the interrupt chip we have fin-
                                 ;;ished handle the interrupt.
                                 ;;:-)
    ;out 0x20,al
    ;out 0xa0,al
    pop eax
    iret

gl_traph_tmp_08:
    push eax
    cmp dword [gl_general_int_handler],0x00000000
    jz .ll_continue
    push ebx                     ;;The following code saves the general
                                 ;;registers.
    push ecx
    push edx
    push esi
    push edi
    push ebp
    mov eax,esp
    push eax
    mov eax,0x08
    push eax
    call dword [gl_general_int_handler]
    pop eax                      ;;Restore the general registers.
    pop eax
    mov esp,eax
    pop ebp
    pop edi
    pop esi
    pop edx
    pop ecx
    pop ebx
.ll_continue:

    ;mov al,0x20                  ;;Indicate the interrupt chip we have fin-
                                 ;;ished handle the interrupt.
                                 ;;:-)
    ;out 0x20,al
    ;out 0xa0,al
    pop eax
    iret

gl_traph_tmp_09:
    push eax
    cmp dword [gl_general_int_handler],0x00000000
    jz .ll_continue
    push ebx                     ;;The following code saves the general
                                 ;;registers.
    push ecx
    push edx
    push esi
    push edi
    push ebp
    mov eax,esp
    push eax
    mov eax,0x09
    push eax
    call dword [gl_general_int_handler]
    pop eax                      ;;Restore the general registers.
    pop eax
    mov esp,eax
    pop ebp
    pop edi
    pop esi
    pop edx
    pop ecx
    pop ebx
.ll_continue:

    ;mov al,0x20                  ;;Indicate the interrupt chip we have fin-
                                 ;;ished handle the interrupt.
                                 ;;:-)
    ;out 0x20,al
    ;out 0xa0,al
    pop eax
    iret

gl_traph_tmp_0a:
    push eax
    cmp dword [gl_general_int_handler],0x00000000
    jz .ll_continue
    push ebx                     ;;The following code saves the general
                                 ;;registers.
    push ecx
    push edx
    push esi
    push edi
    push ebp
    mov eax,esp
    push eax
    mov eax,0x0a
    push eax
    call dword [gl_general_int_handler]
    pop eax                      ;;Restore the general registers.
    pop eax
    mov esp,eax
    pop ebp
    pop edi
    pop esi
    pop edx
    pop ecx
    pop ebx
.ll_continue:

    ;mov al,0x20                  ;;Indicate the interrupt chip we have fin-
                                 ;;ished handle the interrupt.
                                 ;;:-)
    ;out 0x20,al
    ;out 0xa0,al
    pop eax
    iret

gl_traph_tmp_0b:
    push eax
    cmp dword [gl_general_int_handler],0x00000000
    jz .ll_continue
    push ebx                     ;;The following code saves the general
                                 ;;registers.
    push ecx
    push edx
    push esi
    push edi
    push ebp
    mov eax,esp
    push eax
    mov eax,0x0b
    push eax
    call dword [gl_general_int_handler]
    pop eax                      ;;Restore the general registers.
    pop eax
    mov esp,eax
    pop ebp
    pop edi
    pop esi
    pop edx
    pop ecx
    pop ebx
.ll_continue:

    ;mov al,0x20                  ;;Indicate the interrupt chip we have fin-
                                 ;;ished handle the interrupt.
                                 ;;:-)
    ;out 0x20,al
    ;out 0xa0,al
    pop eax
    iret

gl_traph_tmp_0c:
    push eax
    cmp dword [gl_general_int_handler],0x00000000
    jz .ll_continue
    push ebx                     ;;The following code saves the general
                                 ;;registers.
    push ecx
    push edx
    push esi
    push edi
    push ebp
    mov eax,esp
    push eax
    mov eax,0x0c
    push eax
    call dword [gl_general_int_handler]
    pop eax                      ;;Restore the general registers.
    pop eax
    mov esp,eax
    pop ebp
    pop edi
    pop esi
    pop edx
    pop ecx
    pop ebx
.ll_continue:

    ;mov al,0x20                  ;;Indicate the interrupt chip we have fin-
                                 ;;ished handle the interrupt.
                                 ;;:-)
    ;out 0x20,al
    ;out 0xa0,al
    pop eax
    iret

gl_traph_tmp_0d:
    push eax
    cmp dword [gl_general_int_handler],0x00000000
    jz .ll_continue
    push ebx                     ;;The following code saves the general
                                 ;;registers.
    push ecx
    push edx
    push esi
    push edi
    push ebp
    mov eax,esp
    push eax
    mov eax,0x0d
    push eax
    call dword [gl_general_int_handler]
    pop eax                      ;;Restore the general registers.
    pop eax
    mov esp,eax
    pop ebp
    pop edi
    pop esi
    pop edx
    pop ecx
    pop ebx
.ll_continue:

    ;mov al,0x20                  ;;Indicate the interrupt chip we have fin-
                                 ;;ished handle the interrupt.
                                 ;;:-)
    ;out 0x20,al
    ;out 0xa0,al
    pop eax
    iret

gl_traph_tmp_0e:
    push eax
    cmp dword [gl_general_int_handler],0x00000000
    jz .ll_continue
    push ebx                     ;;The following code saves the general
                                 ;;registers.
    push ecx
    push edx
    push esi
    push edi
    push ebp
    mov eax,esp
    push eax
    mov eax,0x0e
    push eax
    call dword [gl_general_int_handler]
    pop eax                      ;;Restore the general registers.
    pop eax
    mov esp,eax
    pop ebp
    pop edi
    pop esi
    pop edx
    pop ecx
    pop ebx
.ll_continue:

    ;mov al,0x20                  ;;Indicate the interrupt chip we have fin-
                                 ;;ished handle the interrupt.
                                 ;;:-)
    ;out 0x20,al
    ;out 0xa0,al
    pop eax
    iret

gl_traph_tmp_0f:
    push eax
    cmp dword [gl_general_int_handler],0x00000000
    jz .ll_continue
    push ebx                     ;;The following code saves the general
                                 ;;registers.
    push ecx
    push edx
    push esi
    push edi
    push ebp
    mov eax,esp
    push eax
    mov eax,0x0f
    push eax
    call dword [gl_general_int_handler]
    pop eax                      ;;Restore the general registers.
    pop eax
    mov esp,eax
    pop ebp
    pop edi
    pop esi
    pop edx
    pop ecx
    pop ebx
.ll_continue:

    ;mov al,0x20                  ;;Indicate the interrupt chip we have fin-
                                 ;;ished handle the interrupt.
                                 ;;:-)
    ;out 0x20,al
    ;out 0xa0,al
    pop eax
    iret

np_inth_tmp:                     ;;This procedure is used to fill the reset
                                 ;;entries of IDT.
    push eax
    mov al,0x20                  ;;Indicate the interrupt chip we have fin-
                                 ;;ished handle the interrupt.
                                 ;;:-)
    out 0x20,al
    out 0xa0,al
    pop eax
    iret                         ;;End of the procedure.

np_int20:
    push eax
    cmp dword [gl_general_int_handler],0x00000000
    jz .ll_continue
    push ebx                     ;;The following code saves the general
                                 ;;registers.
    push ecx
    push edx
    push esi
    push edi
    push ebp
    mov eax,esp
    push eax
    mov eax,0x20
    push eax
    call dword [gl_general_int_handler]
    pop eax                      ;;Restore the general registers.
    pop eax
    mov esp,eax
    pop ebp
    pop edi
    pop esi
    pop edx
    pop ecx
    pop ebx
.ll_continue:

    mov al,0x20                  ;;Indicate the interrupt chip we have fin-
                                 ;;ished handle the interrupt.
                                 ;;:-)
    out 0x20,al
    out 0xa0,al
    pop eax
    iret

np_int21:                        ;;Changed in 12 DEC,2008
    push eax
    cmp dword [gl_general_int_handler],0x00000000
    jz .ll_continue
    push ebx                     ;;The following code saves the general
                                 ;;registers.
    push ecx
    push edx
    push esi
    push edi
    push ebp
    mov eax,esp
    push eax
    mov eax,0x21
    push eax
    call dword [gl_general_int_handler]
    pop eax                      ;;Restore the general registers.
    pop eax
    mov esp,eax
    pop ebp
    pop edi
    pop esi
    pop edx
    pop ecx
    pop ebx
.ll_continue:

    mov al,0x20                  ;;Indicate the interrupt chip we have fin-
                                 ;;ished handle the interrupt.
                                 ;;:-)
    out 0x20,al
    out 0xa0,al
    pop eax
    iret

np_int22:
    push eax
    cmp dword [gl_general_int_handler],0x00000000
    jz .ll_end
    push ebx
    push ecx
    push edx
    push esi
    push edi
    push ebp
    mov eax,esp
    push eax
    mov eax,0x22
    push eax
    call dword [gl_general_int_handler]
    pop eax
    pop eax
    mov esp,eax
    pop ebp
    pop edi
    pop esi
    pop edx
    pop ecx
    pop ebx
.ll_end:
    mov al,0x20                  ;;Indicate the interrupt chip we have fin-
                                 ;;ished handle the interrupt.
                                 ;;:-)
    out 0x20,al
    out 0xa0,al
    pop eax
    iret

np_int23:
    push eax
    cmp dword [gl_general_int_handler],0x00000000
    jz .ll_end
    push ebx
    push ecx
    push edx
    push esi
    push edi
    push ebp
    mov eax,esp
    push eax
    mov eax,0x23
    push eax
    call dword [gl_general_int_handler]
    pop eax
    pop eax
    mov esp,eax
    pop ebp
    pop edi
    pop esi
    pop edx
    pop ecx
    pop ebx
.ll_end:
    mov al,0x20                  ;;Indicate the interrupt chip we have fin-
                                 ;;ished handle the interrupt.
                                 ;;:-)
    out 0x20,al
    out 0xa0,al
    pop eax
    iret

np_int24:
    push eax
    cmp dword [gl_general_int_handler],0x00000000
    jz .ll_end
    push ebx
    push ecx
    push edx
    push esi
    push edi
    push ebp
    mov eax,esp
    push eax
    mov eax,0x24
    push eax
    call dword [gl_general_int_handler]
    pop eax
    pop eax
    mov esp,eax
    pop ebp
    pop edi
    pop esi
    pop edx
    pop ecx
    pop ebx
.ll_end:
    mov al,0x20                  ;;Indicate the interrupt chip we have fin-
                                 ;;ished handle the interrupt.
                                 ;;:-)
    out 0x20,al
    out 0xa0,al
    pop eax
    iret

np_int25:
    push eax
    cmp dword [gl_general_int_handler],0x00000000
    jz .ll_end
    push ebx
    push ecx
    push edx
    push esi
    push edi
    push ebp
    mov eax,esp
    push eax
    mov eax,0x25
    push eax
    call dword [gl_general_int_handler]
    pop eax
    pop eax
    mov esp,eax
    pop ebp
    pop edi
    pop esi
    pop edx
    pop ecx
    pop ebx
.ll_end:
    mov al,0x20                  ;;Indicate the interrupt chip we have fin-
                                 ;;ished handle the interrupt.
                                 ;;:-)
    out 0x20,al
    out 0xa0,al
    pop eax
    iret

np_int26:
    push eax
    cmp dword [gl_general_int_handler],0x00000000
    jz .ll_end
    push ebx
    push ecx
    push edx
    push esi
    push edi
    push ebp
    mov eax,esp
    push eax
    mov eax,0x26
    push eax
    call dword [gl_general_int_handler]
    pop eax
    pop eax
    mov esp,eax
    pop ebp
    pop edi
    pop esi
    pop edx
    pop ecx
    pop ebx
.ll_end:
    mov al,0x20                  ;;Indicate the interrupt chip we have fin-
                                 ;;ished handle the interrupt.
                                 ;;:-)
    out 0x20,al
    out 0xa0,al
    pop eax
    iret

np_int27:
    push eax
    cmp dword [gl_general_int_handler],0x00000000
    jz .ll_end
    push ebx
    push ecx
    push edx
    push esi
    push edi
    push ebp
    mov eax,esp
    push eax
    mov eax,0x27
    push eax
    call dword [gl_general_int_handler]
    pop eax
    pop eax
    mov esp,eax
    pop ebp
    pop edi
    pop esi
    pop edx
    pop ecx
    pop ebx
.ll_end:
    mov al,0x20                  ;;Indicate the interrupt chip we have fin-
                                 ;;ished handle the interrupt.
                                 ;;:-)
    out 0x20,al
    out 0xa0,al
    pop eax
    iret

np_int28:
    push eax
    cmp dword [gl_general_int_handler],0x00000000
    jz .ll_end
    push ebx
    push ecx
    push edx
    push esi
    push edi
    push ebp
    mov eax,esp
    push eax
    mov eax,0x28
    push eax
    call dword [gl_general_int_handler]
    pop eax
    pop eax
    mov esp,eax
    pop ebp
    pop edi
    pop esi
    pop edx
    pop ecx
    pop ebx
.ll_end:
    mov al,0x20                  ;;Indicate the interrupt chip we have fin-
                                 ;;ished handle the interrupt.
                                 ;;:-)
    out 0x20,al
    out 0xa0,al
    pop eax
    iret

np_int29:
    push eax
    cmp dword [gl_general_int_handler],0x00000000
    jz .ll_end
    push ebx
    push ecx
    push edx
    push esi
    push edi
    push ebp
    mov eax,esp
    push eax
    mov eax,0x29
    push eax
    call dword [gl_general_int_handler]
    pop eax
    pop eax
    mov esp,eax
    pop ebp
    pop edi
    pop esi
    pop edx
    pop ecx
    pop ebx
.ll_end:
    mov al,0x20                  ;;Indicate the interrupt chip we have fin-
                                 ;;ished handle the interrupt.
                                 ;;:-)
    out 0x20,al
    out 0xa0,al
    pop eax
    iret

np_int2a:
    push eax
    cmp dword [gl_general_int_handler],0x00000000
    jz .ll_end
    push ebx
    push ecx
    push edx
    push esi
    push edi
    push ebp
    mov eax,esp
    push eax
    mov eax,0x2a
    push eax
    call dword [gl_general_int_handler]
    pop eax
    pop eax
    mov esp,eax
    pop ebp
    pop edi
    pop esi
    pop edx
    pop ecx
    pop ebx
.ll_end:
    mov al,0x20                  ;;Indicate the interrupt chip we have fin-
                                 ;;ished handle the interrupt.
                                 ;;:-)
    out 0x20,al
    out 0xa0,al
    pop eax
    iret

np_int2b:
    push eax
    cmp dword [gl_general_int_handler],0x00000000
    jz .ll_end
    push ebx
    push ecx
    push edx
    push esi
    push edi
    push ebp
    mov eax,esp
    push eax
    mov eax,0x2b
    push eax
    call dword [gl_general_int_handler]
    pop eax
    pop eax
    mov esp,eax
    pop ebp
    pop edi
    pop esi
    pop edx
    pop ecx
    pop ebx
.ll_end:
    mov al,0x20                  ;;Indicate the interrupt chip we have fin-
                                 ;;ished handle the interrupt.
                                 ;;:-)
    out 0x20,al
    out 0xa0,al
    pop eax
    iret

np_int2c:
    push eax
    cmp dword [gl_general_int_handler],0x00000000
    jz .ll_end
    push ebx
    push ecx
    push edx
    push esi
    push edi
    push ebp
    mov eax,esp
    push eax
    mov eax,0x2c
    push eax
    call dword [gl_general_int_handler]
    pop eax
    pop eax
    mov esp,eax
    pop ebp
    pop edi
    pop esi
    pop edx
    pop ecx
    pop ebx
.ll_end:
    mov al,0x20                  ;;Indicate the interrupt chip we have fin-
                                 ;;ished handle the interrupt.
                                 ;;:-)
    out 0x20,al
    out 0xa0,al
    pop eax
    iret

np_int2d:
    push eax
    cmp dword [gl_general_int_handler],0x00000000
    jz .ll_end
    push ebx
    push ecx
    push edx
    push esi
    push edi
    push ebp
    mov eax,esp
    push eax
    mov eax,0x2d
    push eax
    call dword [gl_general_int_handler]
    pop eax
    pop eax
    mov esp,eax
    pop ebp
    pop edi
    pop esi
    pop edx
    pop ecx
    pop ebx
.ll_end:
    mov al,0x20                  ;;Indicate the interrupt chip we have fin-
                                 ;;ished handle the interrupt.
                                 ;;:-)
    out 0x20,al
    out 0xa0,al
    pop eax
    iret

np_int2e:
    push eax
    cmp dword [gl_general_int_handler],0x00000000
    jz .ll_end
    push ebx
    push ecx
    push edx
    push esi
    push edi
    push ebp
    mov eax,esp
    push eax
    mov eax,0x2e
    push eax
    call dword [gl_general_int_handler]
    pop eax
    pop eax
    mov esp,eax
    pop ebp
    pop edi
    pop esi
    pop edx
    pop ecx
    pop ebx
.ll_end:
    ;mov al,0x20                  ;;Indicate the interrupt chip we have fin-
                                 ;;ished handle the interrupt.
                                 ;;:-)
    ;out 0x20,al
    ;out 0xa0,al
    pop eax
    iret

np_int2f:
    push eax
    cmp dword [gl_general_int_handler],0x00000000
    jz .ll_end
    push ebx
    push ecx
    push edx
    push esi
    push edi
    push ebp
    mov eax,esp
    push eax
    mov eax,0x2f
    push eax
    call dword [gl_general_int_handler]
    pop eax
    pop eax
    mov esp,eax
    pop ebp
    pop edi
    pop esi
    pop edx
    pop ecx
    pop ebx
.ll_end:
    mov al,0x20                  ;;Indicate the interrupt chip we have fin-
                                 ;;ished handle the interrupt.
                                 ;;:-)
    out 0x20,al
    out 0xa0,al
    pop eax
    iret

gl_syscall:                      ;;System service call entry point.
    push eax
    cmp dword [gl_general_int_handler],0x00000000
    jz .ll_end
    push ebx
    push ecx
    push edx
    push esi
    push edi
    push ebp
    mov eax,esp
    push eax
    mov eax,0x7F                 ;;System call entry gate,modified in 2009.03.14 by Garry.
    push eax
    call dword [gl_general_int_handler]
    pop eax
    pop eax
    mov esp,eax
	;Restore user mode segment selectors.
	mov ax,0x4b
	mov ds,ax
	mov es,ax
	mov fs,ax
	mov gs,ax
    pop ebp
    pop edi
    pop esi
    pop edx
    pop ecx
    pop ebx
.ll_end:
    pop eax
    iret

;;Simulates a interrupt to trigger the rescheduling
;;of thread/process.
gl_reschedule:
    sti
    push eax
    cmp dword [gl_general_int_handler],0x00000000
    jz .ll_end
    push ebx
    push ecx
    push edx
    push esi
    push edi
    push ebp
    mov eax,esp
    push eax
    mov eax,0x80                 ;;Rescheduling entry gate.
    push eax
    call dword [gl_general_int_handler]
    pop eax
    pop eax
    mov esp,eax
    pop ebp
    pop edi
    pop esi
    pop edx
    pop ecx
    pop ebx
.ll_end:
    pop eax
    iret

np_set_gdt_entry:                ;;This procedure finds the first empty gdt
                                 ;;slot,and initializes this entry according
                                 ;;the parameter,returns the index value.
    push ebp
    mov ebp,esp
    push ebx
    push ecx
    xor eax,eax
    mov ebx,gl_sysgdt
    mov ecx,48                   ;;The first 6 GDT entries are occupied by
                                 ;;system.
.ll_begin:
    cmp dword [ebx + ecx],eax
    jnz .ll_continue
    cmp dword [ebx + ecx + 0x04],eax
    jnz .ll_continue
    mov eax,dword [ebp + 0x08]   ;;Get the first part(4 bytes) of the entry.
    mov dword [ebx + ecx],eax
    mov eax,dword [ebp + 0x0c]   ;;Get the second part(4 bytes) of the entry.
    mov dword [ebx + ecx + 0x04],eax
    shr ecx,0x03                 ;;Get the entry's index.
    mov eax,ecx
    jmp .ll_end
.ll_continue:
    add ecx,0x08
    cmp ecx,511*8                ;;If we have reached the end of the GDT.
    jbe .ll_begin
    mov eax,0x00000000           ;;Indicate the false of this operation.
    jmp .ll_end
.ll_end:
    pop ecx
    pop ebx
    leave
    retn                         ;;End of the procedure.

	;;Reserves some space,to make sure the minikernal's 
	;;size is exactly 8K.
	;;The 8 bytes substracted is the space for import/export
	;;handlers in following section.
    times 8*1024 - 8 - ($ - $$) db 0x00

;;------------------------------------------------------------------------
;;    Import handlers,countains the procedures imported into this module.
;;    These procedures are implemented by Master,filled into this array in
;;    process of system initialization, and will be invoked in this module.
;;------------------------------------------------------------------------
gl_import_procedures:
    gl_general_int_handler   dd 0x00000000

;;------------------------------------------------------------------------
;;    Export routines table.
;;    All routines in this table are implemented in this module by ASM
;;    language,and would be called in HelloX kernel.
;;------------------------------------------------------------------------
gl_export_procedures:
      gl_set_tss dd np_set_tss

;;------------------------------------------------------------------------
;;    The following code is trampoline for Application Processor in SMP
;;    environment.
;;    It's start address is realinit.bin plus 8K of miniker.bin,the fixed
;;    address makes it simple to jump here.
;;    It's the master that lead the AP jump here in process of initializsation,
;;    please refer the master's implementation code for detail.
;;------------------------------------------------------------------------

    ;;Code is 16 bits since the AP runs in real mode when start
	;;very first.
	;;Just load gdtr and switch to protected mode.
    bits 16
gl_trampoline_16:
	cli
	;;Can not reach gl_gdtr_ldr since it's location
	;;is above 1M memory,beyond the scope of real
	;;mode CPU's memory accessing,so we calculate the
	;;corresponding address after it is loaded into
	;;memory very first,where is 8K from start of
	;;physical memory.
	;;It's original content keep unchange before and
	;;after it's relocation.
	mov eax,gl_gdtr_ldr
	sub eax,con_mini_start
	add eax,con_org_start_addr
	lgdt [eax]
	
	;;Initializes all segment registers.
	mov ax,0x010
    mov ds,ax
	mov ss,ax
	mov esp,DEF_INIT_ESP
    mov es,ax
    mov fs,ax
    mov gs,ax

	;;Switch to protected mode.
	mov eax,cr0
	or eax,0x01
	mov cr0,eax
	jmp dword 0x08 : gl_trampoline_32

	;;Start of trampoline's 32 bit code.
	;;Initializes the stack pointer register,
	;;and jump to master.
	align 16
	bits 32
gl_trampoline_32:
	;;Initializes all segment registers.
	mov ax,0x010
    mov ds,ax
	mov ss,ax
	mov esp,DEF_INIT_ESP
    mov es,ax
    mov fs,ax
    mov gs,ax
	;;Initialize IDT.
	lidt [gl_idtr_ldr]
	;;Get current processor's ID.
	xor ecx,ecx
	mov eax,0x01
	cpuid
	shr ebx,24
	and ebx,0xff
	;;Now ebx contains the processor's ID,
	;;it's same as initial APIC ID.
	;;Calculate the current processor's stack
	;;top pointer.
	mov ecx,ebx
	xor eax,eax
.ll_add_16k:
    add eax,con_ap_stack_size
	loop .ll_add_16k
	mov ebx,con_ap_stack_top
	sub ebx,eax
	;;Now ebx is the stack top pointer,initializes
	;;the stack pointer.
	mov ax,0x018
	mov ss,ax
	mov esp,ebx
	;;Jump to the start of master.
	mov eax,con_mast_start
	jmp eax

	;;Slide to next 12K boundary.
	times 12*1024 - ($ - $$) db 0x00

;;------------------------------------------------------------------------
;;    The following 4K space is reserved for memory layout table.
;;    It will be filled up by OS laoder and refered by OS kernel.
;;------------------------------------------------------------------------

gl_mem_layout_table:
    ;;Each element of the table as follows:
	;;start_address(64bits), length(64bits), type
	;;the last element must be all zere,and each element's
	;;length is 20bytes.
	;;This table is initialized by system loader,
	;;and will be used in process of system 
	;;initialization.
	;;The maximal length of the table is 4K,which
	;;can contain 200 elements at most.

	times 16*1024 - ($ - $$) db 0x00

;;------------------------------------------------------------------------
;;    The following 16K space is reserved for system stack in process of
;;    OS initialization.The __OS_Entry routine in master,will use this
;;    block of space as stack,and all routines in miniker,are all use this
;;    space as stack.
;;    Kernel thread's stack space and interrupt stack space will be used
;;    after OS initialization process,this space is left unused after ward.
;;------------------------------------------------------------------------

gl_sys_init_stack:
    times 32*1024 - ($ - $$) db 0x00

;;------------------------------------------------------------------------
;;    END OF MINIKER.ASM
;;    The total size of miniker.bin must be 48K,to fit the requirement of
;;    boot sector(HelloX's constraints...:-)
;;    So there is 16K space left unused now,and may be used in the future.
;;------------------------------------------------------------------------

	;;Make the miniker.bin's size is exactly 48K.
    times 48*1024 - ($ - $$) db 0x00
