;;------------------------------------------------------------------------
;;  Original Author           : Garry
;;  Original Finished date    :
;;  FileName                  : realinit.asm
;;  Procedure defined         :
;;                              1.np_printmsg
;;                              2.
;;                              3.
;;                              4.
;;                              5.
;;  Last modified date        :
;;  Last modified author      :
;;  Lines Number              :
;;  Function                  :
;;                              This module countains the real mode initia-
;;                              lize code.
;;                              These code initialize the CPU context,CRT
;;                              dis play,Flopy disk controller,8259 chip,etc.
;;------------------------------------------------------------------------
 
    bits 16                      ;;The real mode code.
 
    org 0x0000
    ;org 0x0100                   ;;------------ ** debug ** --------------
 
    %define DEF_RINIT_START 0x01000  ;;Start address of this module.
                                     ;;This code is loaded into memory by
                                     ;;boot sector,and resides at 0x01000.
 
    %define DEF_MINI_START  0x02000  ;;The start address of the mini-kernal
                                     ;;when it be loaded into memory.
   
    %define DEF_VBE_INFO    0x0E000  ;;Start address of VBE information block.
 
gl_initstart:
    mov ax,DEF_RINIT_START       ;;The following code prepare the execute
                                 ;;context.
    shr ax,4
    mov ds,ax
    mov es,ax
    mov ss,ax
    mov sp,0x0ff0
 
    mov ax,okmsg
    call np_strlen
    mov ax,okmsg
    call np_printmsg
    mov ax,initmsg
    call np_strlen
    mov ax,initmsg
    call np_printmsg
 
    ;;The following code initializes the system hardware.
 
    call np_init_crt             ;;Initialize the crt display.
    call np_init_keybrd          ;;Initialize the key board.
    call np_init_dmac            ;;Initialize the DMA controller.
    call np_init_8259            ;;Initialize the interrupt controller.
    call np_init_clk             ;;Initialize the clock chip.
    call np_get_syspara          ;;Gather the system parameters.
 
    ;;Now,we have finished intialize the system hardware,it's time to enter
    ;;protected mode.
 
    call np_act20addr            ;;First,activate the above 12 address lines.
 
    xor eax,eax
    mov ax,ds
    shl eax,0x04
    add eax,gl_gdt_content       ;;Form the line address of the gdt content.
    mov dword [gl_gdt_base],eax
    lgdt [gl_gdt_ldr]            ;;Now,load the gdt register.
 
    mov ax,okmsg                 ;;--------- ** debug ** -----------------
    call np_strlen
    mov ax,okmsg
    call np_printmsg
    ;call np_write_mem
    ;mov ax,gl_mem_content
    ;mov cx,0x08
    ;call np_printmsg
 
    mov eax,cr0
    or eax,0x01                  ;;Set the PE bit of CR0 register.
    mov cr0,eax                  ;;Enter the protected mode.
 
    ;mov ax,0x010                 ;;The following code reinitialize the segme-
                                 ;;nt registers.
    ;mov ds,ax
    ;mov es,ax
    ;mov fs,ax
    ;mov gs,ax
 
    jmp dword 0x08 : DEF_MINI_START  ;;Transant the control to Mini Kernal.
 
    ;mov ax,0x4c00                ;;------------- ** debug ** -------------
    ;int 0x21                     ;;------------- ** debug ** -------------
 
;;------------------------------------------------------------------------
;;  Initialize procedures,including:
;;  1.np_act20addr       : Activate the above 12 address lines of the PC.
;;  2.np_init8259        : Initialize the 8259 chip.
;;------------------------------------------------------------------------
 
np_deadloop:                     ;;A dead loop procedure,is called by init-
                                 ;;ializing procedures,when these initialing
                                 ;;procedure are failed.
                                 ;;This procedure print out a message,and
                                 ;;enter into a dead loop.
                                 ;;The only way to recover from this dead
                                 ;;loop is cool restart your computer,because
                                 ;;of the shutting off of the interrupt.
    mov ax,errormsg
    call np_strlen
    mov ax,errormsg,
    call np_printmsg
 
    mov cx,0x01
.ll_dead:
    inc cx
    loop .ll_dead
    ret                          ;;The end of the procedure.
 
;;CRT display initialization routine,only switch to text mode.
np_init_crt:
    push es
    mov ax,0x00
    mov es,ax
    mov word [es : DEF_VBE_INFO],0x00
    mov ax,0x03
    int 0x10
    mov eax,0x01
    pop es
    ret
 
__np_init_crt:
    push ax
    push bx
    push cx
    push dx
    push es
    push si
    push di
    mov ax,0x00
    mov es,ax
    mov word [es : DEF_VBE_INFO],0x00
    mov si,VBE_MODE_ARRAY
.ll_testvbe:       ;Test if the present video controller can support VBE.
    mov di,DEF_VBE_INFO
    mov ax,0x4f00
    int 0x10
    cmp al,0x4f
    jnz .ll_terminal  ;Can not support VBE mode.
    ;jmp .ll_terminal  ;Ignore the subsequent int 10 call for testing.
.ll_setmodebgn:
    cmp word [si],0x00
    jz .ll_terminal
    mov bx,word [si]
    mov ax,0x4f02
    int 0x10
    add si,0x02                  ;;Point to next mode.
    cmp ah,0x00
    jnz .ll_setmodebgn           ;;Set display mode failed.
    ;;Set mode successfully,then retrive mode information.
    sub si,0x02
    mov di,DEF_VBE_INFO
    mov word [es : di],0x01      ;;Indicate in graphic mode.
    add di,0x02
    mov ax,word [si]
    mov word [es : di],ax        ;;Store mode number.
    add di,0x02
    mov cx,ax
    sub cx,0x4000                ;;Mask the flat memory bit.Very important,will be failed otherwise.
    mov ax,0x4f01
    int 0x10
    cmp ah,0x00
    jz .ll_terminal
    ;;If can not retrive mode information,return to text mode.
    mov word [es : DEF_VBE_INFO],0x00
    mov ax,0x03
    int 0x10
.ll_terminal:
    pop di
    pop si
    pop es
    pop dx
    pop cx
    pop bx
    pop ax
    ret
   
.ll_cannotsupport:
    ret                          ;;The end of the procedure.
 
np_init_keybrd:
    ret                          ;;The end of the procedure.
 
np_init_dmac:
    ret                          ;;The end of the procedure.
 
np_init_8259:
    ret                          ;;The end of the procedure.
 
np_init_clk:
    ret                          ;;The end of the procedure.
np_get_syspara:
    ret                          ;;The end of the procedure.
 
 
np_act20addr:                    ;;A procedure to activate the 20th address line.
 
                                 ;;In IBM-PC hardware,the above 12 address l-
                                 ;;ine,are firmed when computer restart,in o-
                                 ;;rder to access the whole 32 bit address s-
                                 ;;pace,we must activate the above 12 address
                                 ;;lines,which is begin at 20th line.
    call np_wait_8042free
    cmp cx,0x0000
    je .ll_error
 
    mov al,0x0D1
    mov dx,0x064
    out dx,al
    call np_wait_8042free
    cmp cx,0x0000
    je .ll_error
    mov al,0x0DF
    mov dx,0x060
    out dx,al
    jmp .ll_end
.ll_error:                       ;;An error occured,the 8042 input buffer is
                                 ;;always full,maybe hardware error.
    call np_deadloop             ;;End dead loop.
.ll_end:
    in al,0xee                   ;;Fuck code!!!!!
    in al,0xee
    in al,0xee
    in al,0xee
 
    in al,0x92
    or al,0x02
    out 0x92,al
    ret                          ;;The end of the procedure,np_act20addr
 
np_wait_8042free:                ;;This procedure test the 8042 input buffer
                                 ;;state,if full,return a failure value in cx,
                                 ;;otherwise,return non-zero in cx.
    mov cx,0xffff
.ll_begin:
    in al,0x64
    test al,0x02
    jnz .ll_begin
    ret                          ;;End of the procedure.
 
 
;;------------------------------------------------------------------------
;;  The following are some helper procedures.
;;  Including:
;;  1.np_printmsg       : print out some message,use ax and cx to trans pa-
;;                        rameters.
;;  2.np_strlen         : get the string's length,a zero indicates the end
;;                        of a string.Use ax as string's base address,and cx
;;                        as return value.
;;  3.
;;
;;------------------------------------------------------------------------
 
np_printmsg:                     ;;This procedure print out some message,
                                 ;;the ax register indicates the base address
                                 ;;of the string,and cx is the length of the
                                 ;;string.
                                 ;;CAUTION: After return,the ax register's
                                 ;;value may be changed.
 
    push bp                      ;;Save the registers used by this proc.
    push bx
    push ax                      ;;Some registers not used in this procedu-
                                 ;;re maybe used in the int 0x010,so we must
                                 ;;save them too.Fuck BIOS!!!!!
    push cx
    mov ah,0x03
    mov bh,0x00
    int 0x010                    ;;Read the position of cursor.
    mov bx,0x0007
    mov ax,0x1301
    pop cx
    pop bp
    int 0x010                    ;;Print out the the message.
    pop bx
    pop bp
    ret                          ;;End of the procedure.
 
np_strlen:                       ;;This procedure get a string's length.
                                 ;;A zero indicates the string's end,the
                                 ;;same as C string.
                                 ;;The ax countains the base address of
                                 ;;the string,and after return,the cx coun-
                                 ;;tains the result.
                                 ;;CAUTION: After return,the ax's value may
                                 ;;be changed.
    push bp
    push bx
    mov bx,ax
    mov al,0x00
    xor cx,cx
.ll_begin:
    cmp byte [bx],al
    je .ll_end
    inc bx
    inc cx
    jmp .ll_begin
.ll_end:
    pop bx
    pop bp
    ret                          ;;End of the string.
 
gl_datasection:
    okmsg      db 0x0d
               db 0x0a
               db 'OK!'
               db 0x00
    rebootmsg  db 0x0d
               db 0x0a
               db 'Reboot now...'
               db 0x00
    poffmsg    db 0x0d
               db 0x0a
               db 'Power off now...'
               db 0x00
    initmsg    db 0x0d
               db 0x0a
               db 'Initializing the hardware...'
               db 0x00
    errormsg   db 0x0d
               db 0x0a
               db 'Error: please restart your computer by manual(reset button)'
               db ',or power off it and power on again.'
               db 0x0d
               db 0x0a
               db 0x00
gl_gdt_content:                  ;;The GDT table content
               dd 0x00000000     ;;NULL GDT entry.
               dd 0x00000000
               dd 0x0000ffff     ;;Code segment GDT entry.
               dd 0x00cf9b00
               dd 0x0000ffff     ;;Data segment GDT entry.
               dd 0x00cf9300
gl_gdt_ldr:
               dw 8*3 - 1        ;;The GDT table's limit.
gl_gdt_base:
               dd 0x00000000     ;;The GDT table's line address,must initial-
                                 ;;ize it before lgdt instruction.
gl_mem_content:                  ;;--------------- ** debug ** -----------
               dd 0x00000000
               dd 0x00000000
 
np_write_mem:                    ;;--------------- ** debug ** -----------
    push ax
    push bx
    push cx
    push dx
    push si
    push di
    mov eax,dword [ds : 0x4668]
    mov di,gl_mem_content
    add di,0x07
    mov cx,0x08
    mov bl,0x0a
.ll_begin:
    mov bh,al
    and bh,0x0f
    cmp bh,bl
    jb .ll_isn
    sub bh,bl
    add bh,'a'
    mov byte [di],bh
    jmp .ll_continue
.ll_isn:
    add bh,'0'
    mov byte [di],bh
.ll_continue:
    shr eax,0x04
    dec di
    loop .ll_begin
    pop di
    pop si
    pop dx
    pop cx
    pop bx
    pop ax
    ret                          ;;End of the procedure.
 
 
    VBE_MODE_ARRAY       dw 0x4118   ;; 1024*768*32 mode,flat display memory.
                         dw 0x4116   ;; 1024*768*16 mode,flat display memory.
                         dw 0x4113   ;; 800*600*16 mode,flat display memory.
                         dw 0x4110   ;; 640*480*16 mode,flat display memory.
                         dw 0x4105   ;; 1024*768*8 mode,flat display memory.
                         dw 0x4103   ;; 800*600*8 mode,flat display memory.
                         dw 0x4101   ;; 640*480*8 mode,flat display memory.
                         dw 0x00     ;; Terminal indicator of VBE_MODE_ARRAY.
 
gl_endof_module:
    times 2*1024 - ($ - $$) db 0x00
 
;--------------------------------------------------------------------------
; The following code segment is the implementation of BIOS service call.
;--------------------------------------------------------------------------
 
    align 4
    bits 32 ;32 bits code since in protected mode present.
    jmp __32CODE_BEGIN
    ;Following is data area used to save registers temporary.
    align 4
__P_IDTR:
    dw 00     ;Limitation of IDTR
    dd 00     ;Base of IDTR
__CR3 dd 0x00 ;Used to save CR3
__32CODE_BEGIN:
    push ebx
    push ecx
    push edx
    push esi
    push edi
    push ebp
    cli  ;Disable interrupt
    sidt [__P_IDTR + 4096] ;Save IDTR
    push eax
    mov eax,cr3
    mov dword [__CR3 + 4096],eax ;Save CR3.
    mov eax,cr0
    and eax,0x7FFFFFFF ;Clear PG bit
    mov cr0,eax  ;Disable paging.
    xor eax,eax
    mov cr3,eax  ;Flush TLB.
    pop eax
    jmp 0x38 : 4096 + __16BIT_ENTRY ;Jump to 16 bits code.
 
    align 4
    bits 16  ;16 bits code.
__16BIT_ENTRY:
    jmp __16CODE_BEGIN
    align 4
__R_IDTR:
    dw 1024
    dd 0x00
__16CODE_BEGIN:
    mov bx,0x30
    mov ds,bx
    mov ss,bx
    mov es,bx
    mov fs,bx
    mov gs,bx
    lidt [__R_IDTR + 4096] ;Load interrupt vector table,the lidt use physical address.
    mov ebx,cr0
    and bl,0xFE ;Clear PE bit
    mov cr0,ebx
    jmp 0x100 : __REAL_MODE_ENTRY  ;Jump to real mode.
 
    align 4
    bits 16
__REINIT_8259:           ;Re-initialize the 8259 chip to comply real mode,
                         ;since it has been configured into different mode
                         ;for protected mode.
    push ax
    mov al,0x13
    out 0x20,al
    mov al,0x08
    out 0x21,al
    mov al,0x09
    out 0x21,al
    pop ax
    ret
 
__REINIT_8259_EX:
    push ax
    mov al,0x11
    out 0x20,al
    out 0xa0,al
    mov al,0x08
    out 0x21,al
    mov al,0x70
    out 0xa1,al
    mov al,0x04
    out 0x21,al
    mov al,0x02
    out 0xa1,al
    mov al,0x01
    out 0x21,al
    out 0xa1,al
    mov al,0xb8
    out 0x21,al
    mov al,0x8f
    out 0xa1,al
 
    mov al,0x20                  ;;Indicate the interrupt chip we have fin-
                                 ;;ished handle the interrupt.
                                 ;;:-)
                                 ;;It is mandatory when switch back real
                                 ;;mode from protect mode.In the switching
                                 ;;process,interrupt may occur but is disabled.
                                 ;;When re-initialize the 8259 chip and enable
                                 ;;the interrupt,pending interrupt may cause
                                 ;;system crash,since the interrupt vector is
                                 ;;different between protect mode and real
                                 ;;mode.
                                 ;;So this code segment cancels all pending
                                 ;;interrupt(s) in real mode.
    out 0x20,al
    out 0xa0,al
 
    pop ax
    ret
 
__REAL_MODE_ENTRY:
    jmp __REALCODE_BEGIN
    align 4
__ESP dd 0x00  ;Save ESP
__REALCODE_BEGIN:
    mov bx,cs
    mov ds,bx
    mov ss,bx
    mov es,bx
    mov fs,bx
    mov gs,bx
    mov dword [__ESP],esp
    mov sp,0xff0
    call __REINIT_8259_EX         ;Set 8259 to BIOS mode
    sti ;Enable interrupt.
    ;OK,can run BIOS code now.
__BIOS_BEGIN:
    mov bx,ax
    shl bx,0x01              ;Multiply 2 since the label is 16 bits.
    add bx,__BIOS_JMP_TABLE
    mov ax,word [bx]
    call ax                  ;Call the appropriate BIOS service routine.
    jmp __BACK_TO_PROTECT
 
    ;The following is a jumping table used to locate proper BIOS service
    ;routine.
    align 4
    bits 16
__BIOS_JMP_TABLE:
    dw __REBOOT           ;Entry point for reboot.
    dw __POWEROFF         ;Entry point for poweroff.
    dw __READSECT         ;Entry point for read sector.
    dw __WRITESECT        ;Entry point for Write sector // old Entry point for delay test.
    dw __TO_GRAPHIC       ;Entry point for switching to graphic mode.
    dw __TO_TEXT          ;Entry point for switching to text mode.
    dw 0x00
 
    ;All BIOS service routine should be placed below,and should add
    ;a entry point in above jump table.
    align 4
 
;;Switch the current display to graphic mode.
__TO_GRAPHIC:
    ;push ax
    push bx
    push cx
    push dx
    push es
    push si
    push di
    mov ax,0x00
    mov es,ax
    mov word [es : DEF_VBE_INFO],0x00
    mov si,VBE_MODE_ARRAY
.ll_testvbe:           ;Test if the present video controller can support VBE.
    mov di,DEF_VBE_INFO
    mov ax,0x4f00
    int 0x10
    cmp al,0x4f
    jnz .ll_failed     ;Can not support VBE mode.
.ll_setmodebgn:
    cmp word [si],0x00
    jz .ll_failed      ;;Can not find a available graphic mode.
    mov bx,word [si]
    mov ax,0x4f02
    int 0x10
    add si,0x02                  ;;Point to next mode.
    cmp ah,0x00
    jnz .ll_setmodebgn           ;;Set display mode failed.
    ;;Set mode successfully,then retrive mode information.
    sub si,0x02
    mov di,DEF_VBE_INFO
    mov word [es : di],0x01      ;;Indicate in graphic mode.
    add di,0x02
    mov ax,word [si]
    mov word [es : di],ax        ;;Store mode number.
    add di,0x02
    mov cx,ax
    sub cx,0x4000                ;;Mask the flat memory bit.Very important,will be failed otherwise.
    mov ax,0x4f01
    int 0x10
    cmp ah,0x00
    jz .ll_success               ;;Set graphic mode successfully.
    ;;If can not retrive mode information,return to text mode.
    mov word [es : DEF_VBE_INFO],0x00
    mov ax,0x03
    int 0x10
.ll_failed:                      ;;Set graphic mode failed.
    mov eax,0x00
    jmp .ll_terminal
.ll_success:                     ;;Set mode successfully.
    mov eax,0x01
.ll_terminal:
    pop di
    pop si
    pop es
    pop dx
    pop cx
    pop bx
    ;pop ax
    ret
 
;;Switch to text mode.
__TO_TEXT:
    push es
    mov ax,0x00
    mov es,ax
    mov word [es : DEF_VBE_INFO],0x00
    mov ax,0x03
    int 0x10
    mov eax,0x01
.ll_terminal:
    pop es
    ret
 
__REBOOT:
    jmp 0xffff : 0x0000 ;Jump to the CPU's first instruction after reset.
    mov eax,0x01   ;Indicate the service is success,though this instruction
                   ;will never execute.
    ret
 
__POWEROFF:
    mov ax,0x5301
    xor bx,bx
    int 0x15
    mov ax,0x530e
    mov cx,0x102
    int 0x15
    mov ax,5307
    mov bl,0x01
    mov cx,0x03
    int 0x15
    mov eax,0x01   ;Indicate the calling is success.
    ret
 
 
    ;Data address packet for int 0x13 extension.
    align 4
__DAP_BEGIN:
    dapPacketSize   db 0x10
    dapReserved1    db 0x00
    dapSectorsNum   db 0x00
    dapReserved2    db 0x00
    dapBuffOff      dw 0x00
    dapBuffSeg      dw 0xf00  ;The buffer of int 13h is start from F000h.
    dapSectorLow    dd 0x00
    dapSectorHigh   dd 0x00
 
__READSECT:
 
    ;pusha           ;Save all registers.
    ;Initialize the DAP
    mov dword [dapSectorLow],edx  ;EDX contains the start sector number.
    mov eax,esi   ;ESI contains the sector number to read.
    mov byte [dapSectorsNum],al
    mov ah,0x42
    lea si,[__DAP_BEGIN] ;Load DAP's address.
 
    ;Only read the first harddisk now.
    mov  dl ,0x80 ;Only handle the first HD.
    int  0x13
    jc   .__ERROR        ;Read failed.
    ;popa
    mov eax,0x01          ;Indicate the successful of this operation.
    ret
.__ERROR:
    ;popa
    mov eax,0x00
    ret
 
__WRITESECT:
     ;pusha           ;Save all registers.
    ;Initialize the DAP
    mov dword [dapSectorLow],edx  ;EDX contains the start sector number.
    mov eax,esi   ;ESI contains the sector number to read.
    mov byte [dapSectorsNum],al
    mov ah,0x43
    lea si,[__DAP_BEGIN] ;Load DAP's address.
	 
    ;Only read the first harddisk now.
    mov  dl ,0x80 ;Only handle the first HD.
    int  0x13
    jc   .__ERROR        ;Read failed.
    ;popa
		
    mov eax,0x01          ;Indicate the successful of this operation.
    ret
.__ERROR:
    ;popa
    mov eax,0x00
    ret
 
 
    ;After BIOS process,should switch back to protect mode.
    align 4
    bits 16
__BACK_TO_PROTECT:
    cli
    mov esp,dword [__ESP] ;Restore ESP in protect mode
    ;cli
    mov ebx,cr0
    or bl,0x01  ;Set PE bit.
    mov cr0,ebx
    jmp 0x08 : __PM_BEGIN + 4096
 
    ;The following code will be running in protect mode.
    align 4
    bits 32
__PM_BEGIN:
    mov bx,0x10
    mov ds,bx
    mov bx,0x18
    mov ss,bx
    mov bx,0x20
    mov es,bx
    mov fs,bx
    mov gs,bx
    lidt [__P_IDTR + 4096] ;Re-load protect mode IDTR.
    mov ebx,dword [__CR3 + 4096]
    mov cr3,ebx
    mov ebx,cr0
    or ebx,0x80000000  ;Enable paging.
    mov cr0,ebx
    mov ebx,np_init8259
    add ebx,4096
    push eax           ;np_init8259 will use al so save eax first.
    call ebx           ;Change 8259's working mode to protect mode.
    pop eax
    sti
    ;Restore registers in protect mode.
    pop ebp
    pop edi
    pop esi
    pop edx
    pop ecx
    pop ebx
    ret  ;Return to master.
 
;-----------------------------------------------------------------
 
    align 4
    bits 32     ;---------- CAUTION ------------------------------
np_init8259:                     ;;This procedure initializes the int-
                                 ;;errupt controller,8259 chip.
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
 
    mov al,0x20                  ;;Indicate the interrupt chip we have fin-
                                 ;;ished handle the interrupt.
                                 ;;:-)
                                 ;;It is mandatory when switch back to protect
                                 ;;mode from real mode.In the switching
                                 ;;process,interrupt may occur but is disabled.
                                 ;;When re-initialize the 8259 chip and enable
                                 ;;the interrupt,pending interrupt may cause
                                 ;;system crash,since the interrupt vector is
                                 ;;different in protect mode.
                                 ;;So this code segment cancels all pending
                                 ;;interrupt(s) in real mode.
    out 0x20,al
    out 0xa0,al
 
    ret                         ;;End of the procedure.
 
    align 4
    bits 16   ;Restore the default setting.
 
 
;-----------------------------------------------------------------
; END OF REALINIT.ASM
;-----------------------------------------------------------------
times 4*1024 - ($ - $$) db 0x00
