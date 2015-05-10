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

    org 0x0000                    ;;The start address of one segment.
    ;org 0x0100                   ;;------------- ** debug ** -------------



    %define DEF_BOOT_START 0x7c00  ;;The start segment address of the boot sector.
                                   

    %define DEF_ORG_START  0x07c0  ;;The orginal address of the boot sector
                                   ;;when loaded into memory by BIOS.

    
    %define DEF_RINIT_START 0x1000 ;;The real mode initialize code start offset address.
                                   
    %define DEF_RINIT_LEN   8      ;;The real mode initialize code's length,unit is 512 byte,one sector.
                                   								      

    %define DEF_MAST_LEN   840    ;;The master's length,in sector,thus,the Master's max length is 300K.
                                   


gl_start:                        ;;Start label of the boot code.

    cli                          ;;Mask all maskable interrupts.
    mov ax,DEF_ORG_START         ;;First,the boot code move itself to DEF_-
                                 ;;BOOT_START from DEF_ORG_START.
    mov ds,ax
    mov ss,ax
    mov sp,0xfff0
	cld  	
	
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
    jmp DEF_BOOT_START : gl_bootbgn  ;;Jump to the DEF_BOOT_START to execute.

gl_bootbgn:
    call np_printmsg             ;;Print out the process message.

    call np_load
	
	jmp   0:DEF_RINIT_START     ;;Jump to the real mode initialize code.
    	
    call np_deadloop             ;;If correct,this line code can not be rea-
                                 ;;ched forever.


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
.ll_inc_es
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

np_deadloop:                     ;;This procedure is a dead loop,when the
                                 ;;boot sector code detect some exception,
                                 ;;it calls this procedure.
                                 ;;If this procedure is called,it means the
                                 ;;system can not loading continue,the only
                                 ;;method is to restart the computer.
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
    ret                          ;;End of the procedure,in this procedure,
                                 ;;this position can not reach forever.


np_printmsg:                     ;;This procedure print out the loading msg.
    mov ah,0x03
    mov bh,0x00
    int 0x010                    ;;Read the position of cursor.
    mov cx,34
    mov bx,0x0007
    mov bp,loadmsg
    mov ax,0x1301
    int 0x010                    ;;Print out the loading message.
    ret                          ;;End of the procedure.



gl_datasection:
    loadmsg db 0x0d
            db 0x0a
            db 'Hx Loading now,waiting please.'
            db 0x0d
            db 0x0a
    errmsg  db 0x0d
            db 0x0a
            db 'Fault Error:please power off your computer,and restart again.'            
            db 0x0d
            db 0x0a
    device_index
            db 0x80
  _DAP_DATA:
   dap_size
	        db 0x10
    dap_reserved
	        db  0x0
    dap_count
	        dw  0x2
	dap_bufaddr_lw
	        dw  0x0	
	dap_bufaddr_hi
	        dw  0x0	
	dap_sector_lw		 
	        dd  0x02  ;; hx start form 2 sector
	dap_sector_hi
			dd  0x0
    total_sector
            dw  DEF_RINIT_LEN+DEF_MAST_LEN
	 
    times 510 - ($ - $$) db 0x00
            dw 0xaa55
