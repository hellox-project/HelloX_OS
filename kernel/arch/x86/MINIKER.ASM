;;------------------------------------------------------------------------
;;    Original Author                   : Garry
;;    Original Date                     : Mar,27,2004
;;    Original Finished date            :
;;    Module Name                       : miniker.asm
;;    Usage                             : countains the mini-kernal code
;;    Defined procedure                 :
;;                                        1.
;;                                        2.
;;                                        3.
;;    Last modified author              : Garry
;;    Last modified date                :
;;    Last modified content             :
;;------------------------------------------------------------------------

;;----------------------- ** Predefines and macros ** --------------------
;;  The following section defines some definations and macros,which can
;;  be used by the ASM code.
;;  NOTICE: In the following code section,may define other macros to use
;;  by local procedure,but these macros only used by the procedure define
;;  it,not 'GLOBAL'.


    %define DEF_PARA_01    dword [ebp + 0x08]         ;;In order to make the
                                                      ;;local procedures can
                                                      ;;access the parameters
                                                      ;;passed,it can use ebp
                                                      ;;register.
                                                      ;;This definations can
                                                      ;;make the code easy to
                                                      ;;organize.
    %define DEF_PARA_02    dword [ebp + 0x0c]

    %define DEF_FALSE      0x00000000    ;;These two definations indicate
                                         ;;the procedure's executation result.
                                         ;;It simulates the bool data-type.
    %define DEF_TRUE       0xffffffff

    %define DEF_INIT_ESP   0x013ffff0    ;;The initializing value of the ESP
                                         ;;register.
    ;%define DEF_INIT_ESP  0x00001000     ;;---------- ** debug ** --------


    con_org_start_addr equ 0x00002000  ;;When this module,mini-kernal is l-
                                       ;;oaded into memory,it resides at
                                       ;;con_org_start_addr,but we must re-
                                       ;;serve the uper 1M memory,so when
                                       ;;the control of CPU transparent to
                                       ;;the mini-kernal,it first moves i-
                                       ;;tself to con_start_addr(see bellow)
                                       ;;from con_org_start_addr,and then
                                       ;;jump to the proper address to exe-
                                       ;;cute.

    con_start_addr equ 0x00100000  ;;Start linear address of the mini-kernal.

    ;con_start_addr equ 0x00002000  ;;-------------- ** debug ** ----------

    con_mast_start equ 0x00110000  ;;The kermal,master's start address.
    ;con_mast_start equ 0x00010000  ;;------------- ** debug **------------

    con_mini_size  equ 0x00010000  ;;We assume the mini-kernal's size is 64K.
    con_mast_size  equ 0x0008c000  ;;The master's length,not acceed 560k.

;;--------------------------- ** Module header ** ------------------------
;;  The following section is the module's header.
;;------------------------------------------------------------------------


    bits 32                      ;;The mini-kernal is a pure 32 bits OS ker-
                                 ;;nal.
    ;org 0x02000                  ;;-------------- ** debug ** ------------

    org 0x00100000               ;;The mini-kernal is loaded at the start
                                 ;;address 0x100000 of the linear address sp-
                                 ;;ace,so the uper 1M memory is reserved.
                                 ;;The uper 4K of this reserved area is co-
                                 ;;untains the system hardware information
                                 ;;filled by BIOS,and the DISPLAY memory a-
                                 ;;lso resides this area.
                                 ;;In additional,we can make other use of
                                 ;;this reserved memory.


    mov ax,0x010                 
    mov ds,ax
    mov es,ax
    jmp gl_sysredirect           ;;The first part of the mini-kernal image is
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
    gl_gdt_null             dd 0 ;;The first entry of GDT must be NULL.
                            dd 0

                                 ;;In this operating system,Hello China,we
                                 ;;arrange the system memory as following:
                                 ;;
                                 ;;  start addr    end addr  size   usage
                                 ;;  ----------------------- ----- --------
                                 ;;  0x00000000 - 0x000fffff  1M   reserved
                                 ;;  0x00100000 - 0x013fffff  20M  os code
                                 ;;  0x00100000 - 0x013fffff  20M  os data
                                 ;;  0x00100000 - 0x013fffff  20M  os stack
                                 ;;  0x01400000 - 0xffffffff       program
                                 ;;
                                 ;;Please note that the OS code area and the
                                 ;;OS data area are overlapped.

    gl_gdt_syscode               ;;The system code segment's GDT entry.
                            ;dw 0x1400    ;;The segment's limit is 20M
                            dw 0xFFFF
                                         ;;Please note that the OS code and
                                         ;;OS data segments include the re-
                                         ;;served uper 1M memory.
                            dw 0x0000
                            db 0x00
                            ;dw 0xc09b    ;;Readable,executeable,and acces-
                            dw 0xCF9B
                                         ;;sable,unit is 4k,instruction a-
                                         ;;ddress attribute is 32 bits.
                            db 0x00

    gl_gdt_sysdata               ;;The system data segment's GDT entry.
                            ;dw 0x1400    ;;The segment's limit is 20M
                            dw 0xFFFF
                            dw 0x0000
                            db 0x00
                            ;dw 0xc093    ;;Readable,writeable,and access-
                            dw 0xCF93
                                         ;;able,unit is 4k.
                            db 0x00

    gl_gdt_sysstack              ;;The system stack segment's GDT entry.
                            ;dw 0x1400    ;;Segment's limit is 20M
                            dw 0xFFFF
                            dw 0x0000    ;;The stack's base address is
                                         ;;0x01000000
                            db 0x00
                            ;dw 0xc093    ;;Readable,writeable,and access-
                            dw 0xCF93
                                         ;;able,unit is 4K,and the default
                                         ;;operand's size is 32 bits.
                            db 0x00


    gl_gdt_sysext                ;;The system extent segment's GDT entry,
                                 ;;this segment will be loaded into es re-
                                 ;;gister.
                            ;dw 0x1400    ;;The segment's limit is 20M
                            dw 0xFFFF
                            dw 0x0000
                            db 0x00
                            ;dw 0xc093    ;;Readable,writeable,and access-
                            dw 0xCF93
                                         ;;able,unit is 4k.
                            db 0x00

    gl_gdt_sysvga                ;;Vga text mode base address,which can be
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
    gl_gdt_normal           dw 0xFFFF
                            dw 0x0000
                            db 0x00
                            dw 0x0092
                            db 0x00
    gl_gdt_code16           dw 0xFFFF
                            dw 0x0000
                            db 0x00
                            dw 0x0098
                            db 0x00

        times 256 dd 0x00        ;;The following is reserved gdt entry spa-
                                 ;;ce,where can be resided by TSS,LDT and
                                 ;;other system tables.
                                 ;;We reserved 4K memory for this usage,so,
                                 ;;it can countains max 512 GDT entry,if all
                                 ;;these entries is used by user applicatio-
                                 ;;n,this operating system can support max
                                 ;;216 user applications(task,or process),
                                 ;;each application use a LDT and a TSS.
        times 256 dd 0x00
        times 256 dd 0x00
        times 256 dd 0x00

align 4
    gl_gdtr_ldr:                 ;;The following variables countains info
                                 ;;about GDT table,which is used by lgdt
                                 ;;instruction.
        dw 256*4*4 + 6*8 - 1     ;;The gdt limit.
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
        

    gl_idtr_ldr:
        dw 8*128 - 1             ;;The IDT table's limit
        dd gl_sysidt             ;;The IDT table's linear address.
                                 ;;This label,gl_idt_ldr,is used by the in-
                                 ;;struction lidt.


;;------------------------------------------------------------------------
;;    The following code is a display(console) driver.
;;------------------------------------------------------------------------
    ;;The following definations are used for console driver.
    ;;We assume that the driver card is VGA compatiable.

    %define DEF_INDEX_PORT 0x3d4 ;;The index port of the display card.
    %define DEF_VALUE_PORT 0x3d5 ;;The data port of the display card.

    %define DEF_DISPMEM_START 0x000b8000  ;;The display memory's start addr.
    %define DEF_DISPMEM_END   0x000bc000  ;;The display memory's end addr.
                                          ;;The display memory's size is 16k.
    %define DEF_REG_CUR_H  0x0e        ;;Cursor's current position's high by-
                                       ;;te register.
    %define DEF_REG_CUR_L  0x0f        ;;Low byte register.

    %define DEF_REG_MEM_H     0x0c     ;;The display memory's original addre-
                                       ;;ss,high byte.
    %define DEF_REG_MEM_L     0x0d     ;;The display memory's original addre-
                                       ;;ss,low byte.

    %define DEF_NUM_COLUMN    80       ;;The columns number of the display.
    %define DEF_NUM_LINE      25       ;;Lines number.

    %define DEF_MAX_STR_LEN   512      ;;The max string's length.

align 8

gl_condrv_data_section:          ;;The following is the data section of con-
                                 ;;sole driver.

    gl_x              dw 0x00    ;;Current columns.
    gl_y              dw 0x00    ;;Current lines.

    gl_dispmem_start  dd DEF_DISPMEM_START
                                 ;;Display memory's start address.

    gl_dispmem_end    dd DEF_DISPMEM_START + 16000
                                 ;;Display memory's end address.
                                 ;;The display memory's size can be deduced
                                 ;;by sub gl_dispmem_start from gl_dispmem_end.
    gl_curr_start     dd 0x000b8000
                                 ;;The current address of the display memory.

    gl_onescrn_size   dd 4000    ;;The one screen's display memory size,in 80
                                 ;;*25 mode,it's 4000 byte.

    gl_default_attr   db 0x07    ;;The default character attribute,white for-
                                 ;;eground,black background,not flash.
                                 ;;This attribute can be changed by applicat-
                                 ;;ion.


np_setcursor:                    ;;The procedure moves the cursor to x,y.
                                 ;;SetCursor(),the cursor's position is
                                 ;;stored at gl_x,gl_y.
    cli                          ;;First,mask all maskable interrupts.
    push ebp
    mov ebp,esp
    push edx
    cmp word [gl_x],DEF_NUM_COLUMN
    jae .ll_error
    cmp word [gl_y],DEF_NUM_LINE
    jae .ll_error
    xor eax,eax
    mov ax,word [gl_y]
    mov dl,DEF_NUM_COLUMN
    mul dl
    add ax,word [gl_x]
    shl eax,0x01
    add eax,dword [gl_curr_start]
    sub eax,DEF_DISPMEM_START         ;;Now,we have get the offset of the cu-
                                      ;;rsor's position releative the start
                                      ;;address of display memory.
    push eax
    push eax
    mov dx,DEF_INDEX_PORT
    mov al,DEF_REG_CUR_L
    out dx,al
    mov dx,DEF_VALUE_PORT
    pop eax
    shr ax,0x01
    out dx,al

    mov dx,DEF_INDEX_PORT
    mov al,DEF_REG_CUR_H
    out dx,al
    mov dx,DEF_VALUE_PORT
    pop eax
    shr ax,0x09
    out dx,al                    ;;Now,we have set the cursor's positon regi-
                                 ;;sters.
    jmp .ll_ok

.ll_error:
    mov eax,DEF_FALSE
    jmp .ll_end
.ll_ok:
    mov eax,DEF_TRUE
.ll_end:
    pop edx
    leave
    sti
    ret                          ;;End of the procedure.

np_gotoxy:                       ;;The procedure goes to x,y
                                 ;;GotoXY(word x,word y)
    cli
    push ebp
    mov ebp,esp
    push ebx
    mov ax,word [ebp + 0x08]
    cmp ax,DEF_NUM_COLUMN
    jae .ll_error
    mov bx,word [ebp + 0x0c]
    cmp bx,DEF_NUM_LINE
    jae .ll_error
    mov word [gl_x],ax
    mov word [gl_y],bx
    call np_setcursor            ;;Now reset cursor's position.
    jmp .ll_end
    
.ll_error:
    mov eax,DEF_FALSE
    pop ebx
    leave
    sti
    ret
.ll_end:
    mov eax,DEF_TRUE
    pop ebx
    leave
    sti
    ret                          ;;End of the procedure.

np_setorgaddr:                   ;;Modify the start address of the display
                                 ;;memory.
                                 ;;SetOrgAddr(dword dwOrgAddr)
    cli
    push ebp
    mov ebp,esp
    mov eax,dword [ebp + 0x08]
    cmp eax,dword [gl_dispmem_start]
    jb .ll_error
    cmp eax,dword [gl_dispmem_end]
    ja .ll_error
    mov dword [gl_curr_start],eax

    push edx                     ;;Now,change the display card's original
                                 ;;display memory.
    sub eax,dword [gl_dispmem_start]
    push eax
    push eax
    mov dx,DEF_INDEX_PORT
    mov al,DEF_REG_MEM_H
    out dx,al
    mov dx,DEF_VALUE_PORT
    pop eax
    shr ax,0x09                  ;;The original's address is fucking ???
    out dx,al

    mov dx,DEF_INDEX_PORT
    mov al,DEF_REG_MEM_L
    out dx,al
    mov dx,DEF_VALUE_PORT
    pop eax
    shr ax,0x01
    out dx,al
    pop edx
    jmp .ll_end
.ll_error:
    mov eax,DEF_FALSE
    leave
    sti
    ret
.ll_end:
    mov eax,DEF_TRUE
    leave
    sti
    ret                          ;;End of the procedure.

np_movup:                        ;;This procedure moves the end part of the
                                 ;;display memory to up,used by scrool****
                                 ;;procedure.
                                 ;;MovUp(dword currAddr).
    cli
    push ebp
    mov ebp,esp
    push ecx
    push esi
    push edi
    mov eax,dword [ebp + 0x08]
    mov ecx,dword [gl_dispmem_end]
    sub ecx,eax
    mov esi,eax
    mov edi,dword [gl_dispmem_start]
    cld
    rep movsb                    ;;Move the end part of the display memory
                                 ;;to up part.
    mov ah,byte [gl_default_attr]
    mov al,' '
    mov ecx,dword [gl_dispmem_end]
    ;add ecx,dword [gl_dispmem_start]
    sub ecx,edi
    shr ecx,0x01
    rep stosw                   ;;Fill the rest part as space.
    pop edi
    pop esi
    pop ecx
    leave
    sti
    ret                          ;;End of the procedure.

np_scrollupl:                    ;;Scroll up one line.
                                 ;;ScrollUpl().
    mov eax,DEF_NUM_COLUMN * 2
    add eax,dword [gl_curr_start]
    add eax,dword [gl_onescrn_size]
    cmp eax,dword [gl_dispmem_end]
    ja .ll_needscroll
    sub eax,dword [gl_onescrn_size]
    mov dword [gl_curr_start],eax
    push eax
    call np_setorgaddr
    pop eax
    jmp .ll_end
.ll_needscroll:
    sub eax,dword [gl_onescrn_size]
    push eax
    call np_movup
    pop eax
    mov eax,dword [gl_dispmem_start]
    push eax
    call np_setorgaddr
    pop eax
.ll_end:
    mov eax,DEF_TRUE
    ret

np_changeline:                   ;;Change the current line to next.
                                 ;;ChangeLine().
    mov ax,word [gl_y]
    inc ax
    cmp ax,DEF_NUM_LINE
    jae .ll_scrollup             ;;If the current line exceed the max lines,
                                 ;;then scrolls up one line.
    mov word [gl_y],ax
    jmp .ll_end
.ll_scrollup:
    call np_scrollupl            ;;Scroll up one line,the current line keep
                                 ;;no change.
.ll_end:
    call np_setcursor            ;;Reset the cursor's position.
    ret                          ;;End of the procedure.

np_gotohome:                     ;;This procedure goto one line's head.
    mov word [gl_x],0x00
    call np_setcursor
    ret                          ;;End of the procedure.

np_gotonext:                     ;;This procedure moves the current position
                                 ;;to next.
    mov ax,word [gl_x]
    inc ax
    cmp ax,DEF_NUM_COLUMN
    jae .ll_changeline
    mov word [gl_x],ax
    jmp .ll_end

.ll_changeline:
    mov word [gl_x],0x00
    call np_changeline
.ll_end:
    call np_setcursor
    ret                          ;;End of the procedure.

np_gotoprev:                     ;;This procedure changes the current positi-
                                 ;;on to the previous,which can be used by
                                 ;;DEL,BackSpace key handler.
    push ebp
    mov ebp,esp
    push edx
    mov ax,word [gl_y]
    add ax,word [gl_x]
    cmp ax,0x0000
    jz .ll_end                   ;;If the current position is locating the
                                 ;;begin of one screen,this procedure only
                                 ;;returns.
    cmp word [gl_x],0x00
    jz .ll_needscroll
    dec word [gl_x]
    jmp .ll_adjust
.ll_needscroll:
    mov word [gl_x],DEF_NUM_COLUMN - 1
    dec word [gl_y]              ;;Adjust to the up lines.
.ll_adjust:
    xor eax,eax
    mov ax,word [gl_y]
    mov dl,DEF_NUM_COLUMN
    mul dl
    add ax,word [gl_x]
    shl eax,0x01
    mov edx,dword [gl_curr_start]
    add edx,eax
    mov al,' '
    mov ah,byte [gl_default_attr]
    cli
    mov word [edx],ax
    call np_setcursor            ;;Update the cursor's position to the curr-
                                 ;;ent position.
    sti
.ll_end:
    pop edx
    leave
    ret                          ;;End of the procedure.


np_printstr:                     ;;Print one string at current position.
                                 ;;PrintStr(string str).
    push ebp
    mov ebp,esp
    push ecx
    push esi
    mov esi,dword [ebp + 0x08]
    push esi
    call np_hlp_strlen
    pop esi
    cmp eax,DEF_MAX_STR_LEN
    ja .ll_adjust
    jmp .ll_continue
.ll_adjust:
    mov eax,DEF_MAX_STR_LEN
.ll_continue:
    mov ecx,eax
    mov ah,byte [gl_default_attr]
.ll_begin:
    mov al,byte [esi]
    push eax
    call np_printch
    pop eax
    inc esi
    loop .ll_begin
    pop esi
    pop ecx
    leave
    ret                          ;;End of the procedure.

np_changeattr:                   ;;Change the default character attribute.
                                 ;;ChangeAttr(byte attr).
    push ebp
    mov ebp,esp
    mov al,byte [ebp + 0x08]
    mov byte [gl_default_attr],al
    leave
    ret                          ;;End of the procedure.

np_printch:                      ;;Print out a character.
                                 ;;PrintCh(word ch),the variable ch countains
                                 ;;the character's ascii code and it's attri-
                                 ;;bute.
    push ebp
    mov ebp,esp
    push ebx
    xor eax,eax
    mov ax,word [gl_y]
    mov bl,DEF_NUM_COLUMN
    mul bl
    add ax,word [gl_x]
    shl ax,0x01                  ;;Form the offset address of the current pos-
                                 ;;ition.
    add eax,dword [gl_curr_start]  ;;Now,eax countains the address where to be
                                   ;;print the character.

    ;mov eax,dword [gl_curr_start]  ;;------------ ** debug ** -----------

    mov ebx,dword [ebp + 0x08]
    mov word [eax],bx
    call np_gotonext
    pop ebx
    leave
    ret                          ;;End of the procedure.

np_clearscreen:                  ;;This procedure clear the whole display
                                 ;;memory,so the whole is cleared.
    push ebp
    mov ebp,esp
    push ecx
    push edi
    mov edi,dword [gl_dispmem_start]
    mov dword [gl_curr_start],edi
    mov word [gl_x],0x00
    mov word [gl_y],0x00
    mov al,' '
    mov ah,byte [gl_default_attr]
    mov ecx,dword [gl_dispmem_end]
    sub ecx,dword [gl_dispmem_start]
    shr ecx,0x01
    cli
    cld
    rep stosw                    ;;Fill the whole display memory as space.
    push dword [gl_dispmem_start]
    call np_setorgaddr           ;;Adjust the current display start address.
    pop dword [gl_dispmem_start]
    call np_setcursor            ;;Adjust the position of the cursor.
    sti
    pop ecx
    pop edi
    leave
    ret                          ;;End of the procedure.

;;------------------------------------------------------------------------
;;    The following code is a key board driver.
;;------------------------------------------------------------------------
    
    %define VK_ESC          0x01
    %define VK_MINIS_SIGN   '-'
    %define VK_EQUAL_SIGN   '='
    %define VK_BACKSPACE    0x0e
    %define VK_TAB          0x0f
    %define VK_RETURN       0x0a
    %define VK_LEFT_CTRL    0x1d
    %define VK_LEFT_SHIFT   0x2a
    %define VK_DOT          '.'
    %define VK_RIGHT_SHIFT  0x2a
    %define VK_LEFT_ALT     0x38
    %define VK_SPACE        ' '
    %define VK_CAPS_LOCK    0x3a
    %define VK_F1           0x3b
    %define VK_F2           0x3c
    %define VK_F3           0x3d
    %define VK_F4           0x3e
    %define VK_F5           0x3f
    %define VK_F6           0x40
    %define VK_F7           0x41
    %define VK_F8           0x42
    %define VK_F9           0x43
    %define VK_F10          0x44
    %define VK_F11          0x57
    %define VK_F12          0x58

    %define VK_KP_NUM        0x45
    %define VK_KP_DOT        '.'
    %define VK_KP_RETURN     0x0a
    %define VK_KP_ADD_SIGN   '+'
    %define VK_KP_MINIS_SIGN '-'
    %define VK_KP_0          '0'
    %define VK_KP_1          '1'
    %define VK_KP_2          '2'
    %define VK_KP_3          '3'
    %define VK_KP_4          '4'
    %define VK_KP_5          '5'
    %define VK_KP_6          '6'
    %define VK_KP_7          '7'
    %define VK_KP_8          '8'
    %define VK_KP_9          '9'
    %define VK_KP_SCROLL     0x46

align 8

gl_kdrv_datasection:             ;;The following section defines some variab-
                                 ;;les and global storage.

    gl_normalqueue:              ;;This queue is used for normal key buffer.
                    dd 0x00000000 ;;These 64 byte space is reserved for key-
                    dd 0x00000000 ;;board buffer.
                    dd 0x00000000
                    dd 0x00000000
                    dd 0x00000000
                    dd 0x00000000
                    dd 0x00000000
                    dd 0x00000000
                    dd 0x00000000
                    dd 0x00000000
                    dd 0x00000000
                    dd 0x00000000
                    dd 0x00000000
                    dd 0x00000000
                    dd 0x00000000
                    dd 0x00000000

                    db 0x00       ;;This byte is used for queue head.
                    db 0x00       ;;This byte is used for queue trail.
                    db 0x00       ;;This byte record the elements' counter.

    gl_extendqueue:              ;;This queue is used for extend key buffer.
                    dd 0x00000000
                    dd 0x00000000
                    dd 0x00000000
                    dd 0x00000000
                    dd 0x00000000
                    dd 0x00000000
                    dd 0x00000000
                    dd 0x00000000
                    dd 0x00000000
                    dd 0x00000000
                    dd 0x00000000
                    dd 0x00000000
                    dd 0x00000000
                    dd 0x00000000
                    dd 0x00000000
                    dd 0x00000000

                    db 0x00      ;;Queue head.
                    db 0x00      ;;Queue trial.
                    db 0x00      ;;Elements' counter.

    %define DEF_QUEUE_LEN  0x40  ;;The queue's length,in this implementation,
                                 ;;we assume the key-board buffer queue's le-
                                 ;;ngth is 64 byte.

    gl_notify_os:                ;;This double word countains a procedure,wh-
                                 ;;ich is called by the key-board driver to
                                 ;;notify the os kernal(here,is the master)
                                 ;;this event.
                                 ;;It can be set by a system call,if this
                                 ;;handler is null(0),the driver will not
                                 ;;call it.
                                 ;;NotifyOS(dword control_and_keycode).
                                 ;;Parameter means is following:
                                 ;;bit 0 - 7 : key code.
                                 ;;bit 8 - 13 : the same as bellow.
                                 ;;bit 14 : 0 for key down,1 for key up.
                                 ;;bit 15 : 0 for normal key,1 for extend key.
                                 ;;bit 16 - 31: reserved.

                    ;dd 0x00000000
                    dd np_notify_os  ;;---------- ** debug ** -----------
    gl_control_bits:
                    db 0x00      ;;Control byte,following are it's means:
                                 ;;bit 0 : Shift key status,1 = shift down.
                                 ;;bit 1 : alt key status,1 = alt key down.
                                 ;;bit 2 : ctrl key status,1 = ctrl key down.
                                 ;;bit 3 : caps lock key status,1 = down.
                                 ;;bit 4 : e0 byte status,if the first scan-
                                 ;;        code is e0,then this bit set.
                                 ;;bit 5 : e1 byte status.
                                 ;;bit 6 : reserved.
                                 ;;bit 7 : reserved.


    gl_scanasc_map:              ;;The following is a scancode to ascii code
                                 ;;map table,used by the key board driver
                                 ;;to find the ascii code according the scan-
                                 ;;code.
    db 0x00
    db VK_ESC,           '1',            '2',            '3',            '4'
    db '5',              '6',            '7',            '8',            '9'
    db '0',              '-',            '=',            VK_BACKSPACE,   VK_TAB
    db 'q',              'w',            'e',            'r',            't'
    db 'y',              'u',            'i',            'o',            'p'
    db '[',              ']',            VK_RETURN,      VK_LEFT_CTRL,   'a'
    db 's',              'd',            'f',            'g',            'h'
    db 'j',              'k',            'l',            ';',            39  ;;'''
    db '`',              VK_LEFT_SHIFT,  '\',            'z',            'x'
    db 'c',              'v',            'b',            'n',            'm'
    db ',',              VK_DOT,         '/',            VK_RIGHT_SHIFT, '*'
    db VK_LEFT_ALT,      VK_SPACE,       VK_CAPS_LOCK,   VK_F1,          VK_F2
    db VK_F3,            VK_F4,          VK_F5,          VK_F6,          VK_F7
    db VK_F8,            VK_F9,          VK_F10,         VK_KP_NUM,      VK_KP_SCROLL
    db VK_KP_7,          VK_KP_8,        VK_KP_9,        VK_KP_MINIS_SIGN, VK_KP_4
    db VK_KP_5,          VK_KP_6,        VK_KP_ADD_SIGN, VK_KP_1,        VK_KP_2
    db VK_KP_3,          VK_KP_0,        VK_KP_DOT,      0x00,           0x00
    db 0x00,             VK_F11,         VK_F12,         0x00,           0x00

    gl_scanasc_map_up:           ;;The following is a scancode to ascii code
                                 ;;map table,used by the key board driver
                                 ;;to find the ascii code according the scan-
                                 ;;code.
    db 0x00
    db VK_ESC,           '!',            '@',            '#',            '$'
    db '%',              '^',            '&',            '*',            '('
    db ')',              '_',            '+',            VK_BACKSPACE,   VK_TAB
    db 'Q',              'W',            'E',            'R',            'T'
    db 'Y',              'U',            'I',            'O',            'P'
    db '{',              '}',            VK_RETURN,      VK_LEFT_CTRL,   'A'
    db 'S',              'D',            'F',            'G',            'H'
    db 'J',              'K',            'L',            ':',            '"'
    db '~',              VK_LEFT_SHIFT,  '|',            'Z',            'X'
    db 'C',              'V',            'B',            'N',            'M'
    db '<',              '>',            '?',            VK_RIGHT_SHIFT, '*'
    db VK_LEFT_ALT,      VK_SPACE,       VK_CAPS_LOCK,   VK_F1,          VK_F2
    db VK_F3,            VK_F4,          VK_F5,          VK_F6,          VK_F7
    db VK_F8,            VK_F9,          VK_F10,         VK_KP_NUM,      VK_KP_SCROLL
    db VK_KP_7,          VK_KP_8,        VK_KP_9,        VK_KP_MINIS_SIGN, VK_KP_4
    db VK_KP_5,          VK_KP_6,        VK_KP_ADD_SIGN, VK_KP_1,        VK_KP_2
    db VK_KP_3,          VK_KP_0,        VK_KP_DOT,      0x00,           0x00
    db 0x00,             VK_F11,         VK_F12,         0x00,           0x00

    gl_handler_table:            ;;The following table countains the key pre-
                                 ;;ss event handlers.
    dd np_null_handler,           np_null_handler   ;;VK_ESC
    dd np_asckey_handler,         np_asckey_handler
    dd np_asckey_handler,         np_asckey_handler
    dd np_asckey_handler,         np_asckey_handler
    dd np_asckey_handler,         np_asckey_handler
    dd np_asckey_handler,         np_asckey_handler
    dd np_asckey_handler,         np_asckey_handler
    dd np_asckey_handler,         np_null_handler   ;;VK_TAB
    dd np_asckey_handler,         np_asckey_handler
    dd np_asckey_handler,         np_asckey_handler
    dd np_asckey_handler,         np_asckey_handler
    dd np_asckey_handler,         np_asckey_handler
    dd np_asckey_handler,         np_asckey_handler
    dd np_asckey_handler,         np_asckey_handler
    dd np_asckey_handler,         np_null_handler   ;;VK_LEFT_CTRL
    dd np_asckey_handler,         np_asckey_handler
    dd np_asckey_handler,         np_asckey_handler
    dd np_asckey_handler,         np_asckey_handler
    dd np_asckey_handler,         np_asckey_handler
    dd np_asckey_handler,         np_asckey_handler
    dd np_asckey_handler,         np_asckey_handler
    dd np_shift_handler
    dd np_asckey_handler
    dd np_asckey_handler,         np_asckey_handler
    dd np_asckey_handler,         np_asckey_handler
    dd np_asckey_handler,         np_asckey_handler
    dd np_asckey_handler,         np_asckey_handler
    dd np_asckey_handler,         np_asckey_handler
    dd np_shift_handler
    dd np_asckey_handler
    dd np_null_handler                              ;;VK_LEFT_ALT
    dd np_asckey_handler
    dd np_capslock_handler
    times 64 dd np_null_handler  ;;--------------- ** debug ** -----------
    dd VK_F1
    dd VK_F2,                     VK_F3
    dd VK_F4,                     VK_F5
    dd VK_F6,                     VK_F7
    dd VK_F8,                     VK_F9
    dd VK_F10,                    VK_KP_NUM
    dd VK_KP_SCROLL,              VK_KP_7
    dd VK_KP_8,                   VK_KP_9
    dd VK_KP_MINIS_SIGN,          VK_KP_4
    dd VK_KP_5,                   VK_KP_6
    dd VK_KP_ADD_SIGN,            VK_KP_1
    dd VK_KP_2,                   VK_KP_3
    dd VK_KP_0,                   VK_KP_DOT
    dd 0x00,                      0x00
    dd 0x00,                      VK_F11
    dd VK_F12,                    0x00
    dd 0x00,                      0x00

np_notify_os:                    ;;------------ ** debug ** --------------
    push ebp
    mov ebp,esp
    push ebx
    mov eax,dword [ebp + 0x08]
    test ah,01000000b
    jnz .ll_end                  ;;Key up,do nothing.
    test ah,10000000b
    jnz .ll_end                  ;;Extend key,do nothing.
    mov ebx,gl_normalqueue
    push ebx
    call np_dequeue
    pop ebx
    cmp eax,DEF_FALSE
    je .ll_end
    cmp al,VK_RETURN
    jne .ll_continue
    call np_changeline
    call np_gotohome
    jmp .ll_end
.ll_continue:
    cmp al,VK_BACKSPACE
    je .ll_continue_1
    jmp .ll_printkey
.ll_continue_1:
    call np_gotoprev
    jmp .ll_end
.ll_printkey:
    mov ah,0x07
    push eax
    call np_printch
    pop eax
.ll_end:
    pop ebx
    leave
    retn                         ;;End of the procedure.

np_null_handler:                 ;;This handler is a null handler,to fill up
                                 ;;the handler table,and may be updated in the
                                 ;;future.
    push ebp
    mov ebp,esp
    leave
    retn                         ;;End of the procedure.

np_capslock_handler:             ;;This procedure handles the caps-lock key
                                 ;;up/down events.
    push ebp
    mov ebp,esp
    push ebx
    mov eax,dword [ebp + 0x08]
    cmp al,0x80
    jae .ll_keyup
    mov ah,byte [gl_control_bits]
    test ah,00001000b
    jz .ll_set1
    and ah,11110111b             ;;Clear the caps-lock key bit.
    mov byte [gl_control_bits],ah  ;;Update the control byte.
    jmp .ll_continue
.ll_set1:
    or ah,00001000b              ;;Set the caps-lock key down bit.
    mov byte [gl_control_bits],ah
.ll_continue:
    or ah,10000000b              ;;Set the extend key bit.
    mov ebx,dword [gl_notify_os]
    cmp ebx,0x00000000
    je .ll_end
    push eax
    call ebx
    pop eax
    jmp .ll_end
.ll_keyup                        ;;If this is a key down event,only notify
                                 ;;os kernal this event,do not reset the
                                 ;;caps-lock control bit.
    mov ah,byte [gl_control_bits]
    or ah,11000000b              ;;Set the extend key bit and the key up bit.
    sub al,0x80
    mov ebx,dword [gl_notify_os]
    cmp ebx,0x00000000
    je .ll_end
    push eax
    call ebx
    pop eax
.ll_end:
    pop ebx
    leave
    retn                         ;;End of the procedure.

np_shift_handler:                ;;This procedure handles the shift key up/down
                                 ;;event.
    push ebp
    mov ebp,esp
    push ebx
    xor eax,eax
    mov al,byte [ebp + 0x08]     ;;Get the key's scan code.
    cmp al,0x80
    jae .ll_keyup
    mov ah,byte [gl_control_bits]
    or ah,0x01                   ;;Set the shift down bit.
    mov byte [gl_control_bits],ah
    or ah,10000000b              ;;Set the extend key bit.
    mov ebx,dword [gl_notify_os]
    cmp ebx,0x00000000
    je .ll_end
    push eax
    call ebx                     ;;Notify os kernal this event.
    pop eax
    jmp .ll_end
.ll_keyup:
    mov ah,byte [gl_control_bits]
    and ah,11111110b             ;;Clear the shift down bit.
    mov byte [gl_control_bits],ah
    or ah,11000000b              ;;Set the key up bit and the extend key bit.
    mov ebx,dword [gl_notify_os]
    cmp ebx,0x00000000
    je .ll_end
    sub al,0x80                  ;;Adjust the key's scan code.
    push eax
    call ebx
    pop eax
    jmp .ll_end
.ll_end:
    pop ebx
    leave
    retn                         ;;End of the procedure.

np_general_handler:              ;;This procedure handles all key press events,
                                 ;;and call the correct handler according the
                                 ;;scan code.
                                 ;;The parameter is key's scan code.
    push ebp
    mov ebp,esp
    push ebx
    push esi
    xor ebx,ebx
    xor esi,esi
    mov al,byte [ebp + 0x08]     ;;Get the key's scan code.
    cmp al,0xe0
    je .ll_e0
    cmp al,0xe1
    je .ll_e1
    cmp al,0x80
    jae .ll_keyup                ;;This is a key up event.
    cmp al,0x58                  ;;------------ ** debug ** --------------
    jae .ll_end                  ;;------------ ** debug ** --------------
    mov bl,al
    jmp .ll_continue
.ll_keyup:
    mov bl,al
    sub bl,0x80
.ll_continue:
    mov esi,ebx
    mov ebx,gl_handler_table
    shl esi,0x02
    add ebx,esi
    push eax
    call [ebx]     
    pop eax
    jmp .ll_end
.ll_e0:
    mov ah,byte [gl_control_bits]
    or ah,00010000b
    mov byte [gl_control_bits],ah  ;;Set the e0 bit in control byte.
    jmp .ll_end
.ll_e1:
    mov ah,byte [gl_control_bits]
    or ah,00100000b
    mov byte [gl_control_bits],ah  ;;Set the e1 bit in control byte.
.ll_end:
    pop esi
    pop ebx
    leave
    retn                         ;;The end of the procedure.
    


np_asckey_handler:               ;;The procedure handle the ascii key press
                                 ;;event.
                                 ;;It's parameter is the scan code.
    push ebp
    mov ebp,esp
    push ebx
    xor eax,eax
    mov al,byte [ebp + 0x08]
    cmp al,0x80
    jae .ll_keyup                ;;This event is a key up event.
    mov ah,byte [gl_control_bits]
    test ah,00000001b            ;;If shift key down.
    jnz .ll_shiftdown
    test ah,00001000b            ;;If capslock key down.
    jnz .ll_capslockdown
    mov ebx,gl_scanasc_map
    jmp .ll_continue
.ll_shiftdown:
.ll_capslockdown:
    mov ebx,gl_scanasc_map_up
.ll_continue:
    xor eax,eax
    mov al,byte [ebp + 0x08]
    add ebx,eax
    mov al,byte [ebx]
    push eax
    push gl_normalqueue
    call np_inqueue              ;;Add the key into normal key queue.
    pop eax
    pop eax
    mov ebx,dword [gl_notify_os]
    cmp ebx,0x00000000
    jz .ll_end
    mov ah,byte [gl_control_bits]
    push eax
    call ebx                     ;;Notify the os kernal this event.
    pop eax
    jmp .ll_end
.ll_keyup:
    mov ebx,dword [gl_notify_os]
    cmp ebx,0x00000000
    jz .ll_end
    mov ah,byte [gl_control_bits]
    or ah,01000000b              ;;Set the key up bit.
    push eax
    call ebx                     ;;Notify the os kernal this event.
    pop eax
.ll_end:
    pop ebx
    leave
    retn



np_inqueue:                      ;;This procedure add a element into the qu-
                                 ;;eue,and update the current queue control
                                 ;;value.
                                 ;;The parameter is queue's base address,and
                                 ;;the element to be added to the queue.
                                 ;;InQueue(dword dwQueueBase,byte ele).
    push ebp
    mov ebp,esp
    push ebx
    push ecx
    mov ebx,dword [ebp + 0x08]   ;;Get the current queue's base address.
    push ebx
    call np_queuefull
    pop ebx
    cmp eax,DEF_TRUE
    je .ll_queuefull             ;;If the current queue is full,only set the
                                 ;;return result to false,and return.
    xor eax,eax
    mov al,byte [ebx + DEF_QUEUE_LEN + 1]  ;;Get the current queue's trial.
    mov cl,byte [ebp + 0x0c]     ;;Get the element to be added to the queue.
    mov byte [ebx + eax],cl      ;;Add the element to the queue.
    inc al
    cmp al,DEF_QUEUE_LEN
    je .ll_adjust
    jmp .ll_continue
.ll_adjust:
    mov al,0x00
.ll_continue:
    inc byte [ebx + DEF_QUEUE_LEN + 2]  ;;Update the queue element's counter.
    mov byte [ebx + DEF_QUEUE_LEN + 1],al ;;Update the current queue's trial.
    mov eax,DEF_TRUE
    jmp .ll_end
.ll_queuefull:
    mov eax,DEF_FALSE
.ll_end:
    pop ecx
    pop ebx
    leave
    ret                          ;;End of the procedure.


np_dequeue:                      ;;This procedure delete a element from the
                                 ;;queue,and update the current queue control
                                 ;;value.
                                 ;;The parameter is queue's base address,and
                                 ;;the return value is countained in eax re-
                                 ;;gister,which al countains the element get
                                 ;;from the queue.
    push ebp
    mov ebp,esp
    push ebx
    push ecx
    mov ebx,dword [ebp + 0x08]   ;;Get the current queue's base address.
    push ebx
    call np_queueempty
    pop ebx
    cmp eax,DEF_TRUE
    je .ll_queueempty
    xor eax,eax
    mov al,byte [ebx + DEF_QUEUE_LEN]  ;;Get the queue's head.
    mov cl,byte [ebx + eax]      ;;Get the current element.
    inc al
    cmp al,DEF_QUEUE_LEN
    je .ll_adjust
    jmp .ll_continue
.ll_adjust:
    mov al,0x00
.ll_continue:
    mov byte [ebx + DEF_QUEUE_LEN],al  ;;Update the queue's head value.
    dec byte [ebx + DEF_QUEUE_LEN + 2] ;;Update the queue's length.
    xor eax,eax
    mov al,cl
    jmp .ll_end
.ll_queueempty:                  ;;If the current queue is full,this proced-
                                 ;;ure only set the return value to false,and
                                 ;;return.
    mov eax,DEF_FALSE
.ll_end:
    pop ecx
    pop ebx
    leave
    ret                          ;;End of the procedure.

np_queuefull:                    ;;Determine if the current queue is full.
                                 ;;If the queue's trail + 1 = queue's head,
                                 ;;the queue is full.
                                 ;;The parameter is queue's base address.
    push ebp
    mov ebp,esp
    mov eax,dword [ebp + 0x08]   ;;Get the queue's base address.
    add eax,DEF_QUEUE_LEN
    add eax,0x02                 ;;Adjust the eax register,so this register
                                 ;;countains the queue element counter's of-
                                 ;;fset.
    cmp byte [eax],DEF_QUEUE_LEN
    je .ll_full
    mov eax,DEF_FALSE
    jmp .ll_end
.ll_full:
    mov eax,DEF_TRUE
.ll_end:
    leave
    ret                          ;;End of the procedure.

np_queueempty:                   ;;Determine if the current queue is empty.
                                 ;;If the queue's head = queue's trial,then
                                 ;;the queue is empty.
                                 ;;The parameter is queue's base address.
    push ebp
    mov ebp,esp
    mov eax,dword [ebp + 0x08]
    add eax,DEF_QUEUE_LEN
    add eax,0x02
    cmp byte [eax],0x00
    je .ll_empty
    mov eax,DEF_FALSE
    jmp .ll_end
.ll_empty:
    mov eax,DEF_TRUE
.ll_end:
    leave
    ret                          ;;End of the procedure.

np_set_notifyos_handler:         ;;This procedure set the notify os handler.
                                 ;;It's parameter is the handler's base ad-
                                 ;;dress.This procedure returns the previous
                                 ;;handler's base address.
                                 ;;dword SetNotifyOsHandler(dword dwBaseAddr).
    push ebp
    mov ebp,esp
    push ebx
    mov eax,dword [ebp + 0x08]   ;;Get the handler's base address.
    cmp eax,0x00000000
    jz .ll_error
    mov ebx,dword [gl_notify_os]
    mov dword [gl_notify_os],eax
    mov eax,ebx                  ;;Set the return value to the previous hand-
                                 ;;ler's value.
    jmp .ll_end
.ll_error:                       ;;If the parameter is null,then return a
                                 ;;false.
    mov eax,DEF_FALSE
.ll_end:
    pop ebx
    leave
    retn                         ;;End of the procedure.

;;---------------- ** System initialize code section ** ------------------
;;  The following section countains the system initialize code,these code
;;  Initialize the CPU running context,such as GDT,IDT,and some system le-
;;  vel arrays.
;;------------------------------------------------------------------------

align 4
gl_sysredirect:                  ;;Redirect code of mini-kenal,this code
                                 ;;moves the mini-kernal from con_org_st-
                                 ;;art_addr to con_start_addr.
    mov ecx,con_mini_size + con_mast_size
    shr ecx,0x02
    mov esi,con_org_start_addr   ;;Original address.
    mov edi,con_start_addr       ;;Target address.
    cld
    rep movsd

    mov eax,gl_initgdt
    jmp eax                      ;;After moved mini-kernal to the start
                                 ;;address,the mini-kernal then jump to
                                 ;;the start entry,labeled by gl_initgdt.

gl_initgdt:                      ;;The following code initializes the GDT
                                 ;;and all of the segment registers.
    lgdt [gl_gdtr_ldr]           ;;Load the new gdt content into gdt regis-
                                 ;;ter.
    mov ax,0x010
    mov ds,ax
    mov ax,0x018
    mov ss,ax
    mov esp,DEF_INIT_ESP         ;;The two instructions,mov ss and mov esp
                                 ;;must resides together.
    mov ax,0x020
    mov es,ax
    mov fs,ax                    ;;Initialize the fs as the same content as
                                 ;;es.If need,we can change the fs's value.
    mov ax,0x020
    mov gs,ax
    jmp dword 0x08 : gl_sysinit  ;;A far jump,to renew the cs register's v-
                                 ;;alue,and clear the CPU's prefetched que-
                                 ;;ue,trans the control to the new squence.

gl_sysinit:                      ;;The start position of the init process.
    mov eax,gl_trap_int_handler
    push eax
    call np_fill_idt             ;;Initialize the IDT table.
    pop eax
    lidt [gl_idtr_ldr]           ;;Load the idtr.
    call np_init8259             ;;Reinitialize the interrupt controller.
    sti
    nop
    nop
    nop
    mov eax,con_mast_start     
    jmp eax

;; The following defines some helper procedures,to help the system initial-
;; ize process.

np_fill_idt:                     ;;This procedure fills the IDT table.
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
    mov esi,DEF_PARA_01
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
    ret                          ;;End of the procedure.

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
    ret                         ;;End of the procedure.


;;------------------------- ** Temp procedures ** ------------------------
;;  The following defines some temp procedures,are used by the mini-kernal
;;  to fill the system tables.

np_delay:
    push ebp
    mov ebp,esp
    sub esp,0x04
    mov word [ebp - 2],0x00ff
.ll_begin:
    mov word [ebp - 4],0xffff 
.ll_loop:
    nop
    nop
    nop
    nop
    dec word [ebp - 4]
    jnz .ll_loop
    dec word [ebp - 2]
    jnz .ll_begin

    add esp,0x04
    leave
    ret                          ;;End of the procedure.


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
    mov al,0x20                  ;;Indicate the interrupt chip we have fin-
                                 ;;ished handle the interrupt.
                                 ;;:-)
    out 0x20,al
    out 0xa0,al
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
    sti                          ;;Enable interrupt for system call,modified in 2009.03.14 by Garry.
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
    pop ebp
    pop edi
    pop esi
    pop edx
    pop ecx
    pop ebx
.ll_end:
    pop eax
    iret

;;----------------------- ** Helper procedures ** ------------------------
;;  The following section defines some helper procedures,including string
;;  operation procedures,data transform procedures,and list operations.
;;  The mini-ker use these procedures to implement it's functions.
;;  These procedures are named as np_hlp_xxxx.

np_hlp_strlen:                   ;;This procedure can get the string's leng-
                                 ;;th.In Hello China's kernal,we use a zero
                                 ;;to indicate the end of a string,like C.
    push ebp
    mov ebp,esp
    push esi
    mov esi,DEF_PARA_01          ;;This procedure takes one parameter,the
                                 ;;string's start address,and returns the
                                 ;;string's length in eax register.
    xor eax,eax
.ll_bgn:
    cmp byte [esi],0x0
    jz .ll_end
    inc eax
    inc esi
    jmp .ll_bgn
.ll_end:
    pop esi
    leave
    ret                          ;;End of the procedure.

np_hlp_strcpy:                   ;;This procedure can copy the source string
                                 ;;(passed as the second parameter) to the
                                 ;;destination string(passed as the first p-
                                 ;;arameter.

                                 ;;CAUTION: This procedure do not check the
                                 ;;destination's size,so,please make sure
                                 ;;the source string's size is not beyond the
                                 ;;destination buffer's size,otherwise,some
                                 ;;errors be occur.
    push ebp
    mov ebp,esp
    push edi
    push esi
    push ecx
    mov edi,DEF_PARA_01          ;;The first parameter is the destination's
                                 ;;buffer address.
    mov esi,DEF_PARA_02          ;;The second parameter is the source stri-
                                 ;;ing's start address.
    push esi
    call np_hlp_strlen           ;;Call the strlen procedure calculate the
                                 ;;source string's length.
    pop esi
    mov ecx,eax
    cld
    rep movsb                    ;;Copy the source string to the destination.
    pop ecx
    pop esi
    pop edi
    leave
    ret                          ;;End of the procedure.

np_hlp_hex2str:                  ;;This procedure transform the hex number to
                                 ;;ascii string format,so that the hex value
                                 ;;can be displayed on the screen.
                                 ;;This procedure use the __syscall parameter
                                 ;;pass rule(please refer the specific),and 2
                                 ;;parameters are passed to this procedure:
                                 ;;
                                 ;;  1. the hex number to be transformed;
                                 ;;  2. the start address(offset) of the
                                 ;;     result string.
                                 ;;
                                 ;;we can refer these parameters use the ebp
                                 ;;register.
    push ebp
    mov ebp,esp
    push esi                     ;;Save the registers used by this procedure.
    push ecx
    mov eax,DEF_PARA_01          ;;The first parameter,the number to be tran-
                                 ;;sformed.
    mov esi,DEF_PARA_02          ;;The second parameter,the offset of the re-
                                 ;;sult string.
    add esi,0x07
    mov ch,0x08
.ll_begin:
    mov cl,al
    and cl,0x0f
    shr eax,0x04
    cmp cl,0x0a
    jb .ll_isn
    sub cl,0x0a
    add cl,'A'
    mov byte [esi],cl
    dec esi
    dec ch
    jnz .ll_begin
    jmp .ll_end
.ll_isn:
    add cl,'0'
    mov byte [esi],cl
    dec esi
    dec ch
    jnz .ll_begin
.ll_end:
    pop ecx
    pop esi
    leave
    ret                          ;;End of the procedure.

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


    times 47*1024 - ($ - $$) db 0x00    ;;The following two lines reserve
                                        ;;some space,to make sure the mini-
                                        ;;kernal's size is exactly 48k.
    times 1024 - 108         db 0x00

;;------------------------------------------------------------------------
;;    The following is a double word array,it countains the procedures
;;    import this module.
;;    This procedures are implemented by Master,and are called in this
;;    module.
;;------------------------------------------------------------------------

gl_import_procedures:
    gl_int22_handler         dd 0x00000000
    gl_int23_handler         dd 0x00000000
    gl_int24_handler         dd 0x00000000
    gl_int25_handler         dd 0x00000000
    gl_int26_handler         dd 0x00000000
    gl_int27_handler         dd 0x00000000
    gl_int28_handler         dd 0x00000000
    gl_int29_handler         dd 0x00000000
    gl_int2a_handler         dd 0x00000000
    gl_int2b_handler         dd 0x00000000
    gl_int2c_handler         dd 0x00000000
    gl_int2d_handler         dd 0x00000000
    gl_int2e_handler         dd 0x00000000
    gl_int2f_handler         dd 0x00000000

    gl_set_gdt_entry         dd np_set_gdt_entry  ;;Set the GDT entry,
                                                  ;;called by other modules.
    gl_general_int_handler   dd 0x00000000

;;------------------------------------------------------------------------
;;    The following is a double word array,it countains the offset of
;;    some main procedures in this module(mini-kernal),so,the other part
;;    of the operating system,such as master,can access this procedure
;;    directly.
;;    A nother way to call these procedures is system call.
;;------------------------------------------------------------------------

gl_export_procedures:

    dd  np_gotoprev
    dd  np_set_notifyos_handler

    dd  np_hlp_strlen
    dd  np_hlp_strcpy
    dd  np_hlp_hex2str

    dd  np_changeline
    dd  np_gotohome
    dd  np_printch
    dd  np_clearscreen
    dd  np_changeattr
    dd  np_printstr

;;----------------------- ** End of the module ** ------------------------
