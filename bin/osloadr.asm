bits 16
org 0x00

mov al, 'L'
call ShowRoutine
mov al, 'O'
call ShowRoutine
mov al, 'A'
call ShowRoutine
mov al, 'D'
call ShowRoutine
mov al, ' '
call ShowRoutine
mov al, 'O'
call ShowRoutine
mov al, 'K'
call ShowRoutine

jmp __END

ShowRoutine:
mov ah, 0x0E
int 0x10
ret

__END:
cli
jmp $

times 564*1024 - ($ - $$) db 0x00

