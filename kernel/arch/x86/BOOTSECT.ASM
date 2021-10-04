;;-----------------------------------------------------------------------
;;  Original Author           : Garry
;;  Original Date             : May,14,2004
;;  FileName                  : bootsect.asm
;;  Finished Date             : May 14,2004
;;  Last Modified Date        : May 14,2004
;;  Last Modified Author      :
;;  Procedure defined         :
;;                              1.np_load
;;                              2.np_deadloop
;;                              3.np_printmsg
;;                              4.np_printprocess
;;  Lines number              : 254
;;  Module function           :
;;                              This module countains the boot sector code,
;;                              these code is fit to HD flopy,1.44M.
;;
;;------------------------------------------------------------------------

    bits 16                      ;;This boot sector's code is running under
                                 ;;real address mode.

    org 0x0000                   ;;Start from very begining.

	;;The start segment address of the boot sector after relocation.
    ;%define DEF_BOOT_START 0x9f00
	%define DEF_BOOT_START 0x9EC0
	;;Original address of the boot sector after loaded into memory.
    %define DEF_ORG_START  0x07c0
    
	;;Start address of realinit module.
    %define DEF_RINIT_START 0x1000
    ;;The real mode initialize code's length,unit is 512 byte,one sector.                               
    %define DEF_RINIT_LEN   8

    %define DEF_MAST_LEN   1120   ;The HelloX kernel's length,in sector unit.
	                              ;Please be noted that the kernel includes miniker and master.The
								  ;upper boundary of the kernel size is 560k.
    
	;Start from here.                                   
gl_start:
    cli                          ;;Mask all maskable interrupts.
    mov ax,DEF_ORG_START         ;;First,the boot code move itself to DEF_-
                                 ;;BOOT_START from DEF_ORG_START.
    mov ds,ax
    mov ss,ax
    mov sp,0xfff0
	cld  	
	
	;Relocate the bootsector.
    mov si,0x0000    
    mov ax,DEF_BOOT_START
    mov es,ax
    mov di,0x0000    
    mov cx,0x0200                ;;The boot sector's size is 512B
    rep movsb

    mov ax,DEF_BOOT_START        ;;Prepare the execute context.
    mov ds,ax
    mov es,ax
    mov ss,ax
    mov sp,0x0ffe
    jmp DEF_BOOT_START : gl_bootbgn

gl_bootbgn:
    ;Save boot drive number.
	mov byte [device_index],dl
	;Show out a message.
    call np_printmsg
	;Load kernel into memory.
    call np_load
	
	;Jump to the real mode initialize code.
	jmp 0 : DEF_RINIT_START
	;jmp 0x100 : 0

	;Enter deadloop in case of exception.
    call np_deadloop

;;------------------------------------------------------------------------
;;  The following is some helper functions and data section.
;;------------------------------------------------------------------------

np_load:                         ;;This procedure use the int 13 interrupt
                                 ;;call,load the operating system kernal
                                 ;;into memory.
    push es
    mov ax,0
    mov es,ax	
    mov bx,DEF_RINIT_START
    xor cx,cx
			
.ll_start:    
    mov ah,0x42		   
	mov dl,byte [device_index]
	lea si,[dap_size]
	mov word [dap_bufaddr_lw],bx 
	mov word [dap_bufaddr_hi],es
    int 0x13	
	jc .ll_error

.ll_count:	
    inc dword [dap_sector_lw]
	inc dword [dap_sector_lw]
    dec word [total_sector]
    dec word [total_sector]
    jz .ll_end

    cmp bx,63*1024               ;;If the buffer reachs 64k boundry,we must
                                 ;;reinitialize it.
    je .ll_inc_es
    add bx,1024
    jmp .ll_start
.ll_inc_es:
    mov bx,es
    add bx,4*1024                
    mov es,bx                    ;;Update the es register to another 64k b-
                                 ;;oundry.
    xor bx,bx
	jmp .ll_start
.ll_error:                       ;;If there is an error,enter a dead loop.
    dec byte [device_index]
    jmp .ll_start
    mov dx,0x03f2
    mov al,0x00
    out dx,al
    pop es
    call np_deadloop
.ll_end:
    mov dx,0x03f2                ;;The following code shut off the FDC.
    mov al,0x00
    out dx,al
	mov si,0x0000
    pop es
    ret                          ;;End of the procedure.

np_deadloop:
    mov ah,0x03
    mov bh,0x00
    int 0x010
    mov cx,65
    mov bx,0x0007
    mov bp,errmsg
    mov ax,0x1301
    int 0x010
    mov cx,0x01
.ll_begin:
    inc cx
    loop .ll_begin
    ret

np_printmsg:
    mov ah,0x03
    mov bh,0x00
    int 0x010
    mov cx,40
    mov bx,0x0007
    mov bp,loadmsg
    mov ax,0x1301
    int 0x010
    ret

;Data section of this module.
gl_datasection:
    loadmsg db 0x0d
            db 0x0a
            db 'HelloX is loading now,please wait...'
            db 0x0d
            db 0x0a
    errmsg  db 0x0d
            db 0x0a
            db 'Fault Error:please power off and restart again.'            
            db 0x0d
            db 0x0a
    device_index db 0x80

  int13_dap:
    dap_size db 0x10
    dap_reserved db 0x0
    dap_count dw 0x2
	dap_bufaddr_lw dw 0x0	
	dap_bufaddr_hi dw 0x0
	;Kernel resides in disk from 2nd sector.
	dap_sector_lw dd 0x02
	dap_sector_hi dd 0x0
    total_sector dw DEF_RINIT_LEN + DEF_MAST_LEN

    times 510 - ($ - $$) db 0x00
	;Last word must be 0xAA55.
    dw 0xAA55
