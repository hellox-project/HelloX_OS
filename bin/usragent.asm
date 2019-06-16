bits 32
align 4
org 0x40000000

gl_mainloop:
    push ebp
    mov ebp,esp
    mov ecx,dword [ebp + 0x08]
    mov ebx,dword [ebp + 0x0C]
.ll_printloop:
    push ebx
    push ecx
    mov edi,dword [ebx]
    mov eax, 0x2D
    int 0x7F
    pop ecx
    pop ebx
    add ebx,0x04
    loop .ll_printloop
    call gl_exit
    leave
    ret

gl_exit:
    mov eax,0x40
    int 0x7F
    ret
