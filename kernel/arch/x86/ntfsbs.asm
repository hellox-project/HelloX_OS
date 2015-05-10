BITS   16   ; 生成16位代码而不是32位代码
SECTION  .TEXT   ; 代码段
;ORG   7C00H  ; 指定程序被装入内存的起始位置
ORG 0000H
 
;====================================================================
; 
; 宏和常量定义
; 
;====================================================================
?     EQU  0  ; NASM不支持DW ?这样的语法，可以使用这样的定义
        ; 模拟，以使代码的可读性更强     
 
;====================================================================  
; 扩展磁盘服务所使用的地址包
;====================================================================
DAP_SECTOR_HIGH            EQU  24  ; 起始扇区号的高32位 ( 每次调用需要重置 ) DWORD
DAP_SECTOR_LOW             EQU  28  ; 起始扇区号的低32位 ( 每次调用需要重置 ) DWORD
DAP_BUFFER_SEG             EQU  30  ; 缓冲区段地址   ( 每次调用需要重置 ) WORD
DAP_BUFFER_OFF             EQU  32  ; 缓冲区偏移   ( 每次调用需要重置 ) WORD  
DAP_RESERVED2              EQU  33  ; 保留字节
DAP_READ_SECTORS           EQU  34  ; 要处理的扇区数(1 - 127 )
DAP_RESERVED1              EQU  35  ; 保留字节
DAP_PACKET_SIZE            EQU  36  ; 包的大小为16字节
 
;Start segment address of bootsect after loaded into memory.
DEF_ORG_START              EQU 7C0H
 
;Base segment address of the bootsect after relocation
DEF_BOOT_START             EQU 9000H
 
;Start segment address of OS image.
DEF_RINIT_START            EQU 1000H
DEF_INT13B_START           EQU 9200H
 
;====================================================================
;
; 启动扇区(512字节)
;
;====================================================================
_ENTRY_POINT:
 
; 3字节的跳转指令
 JMP SHORT _BOOT_CODE ; 跳转到真正的引导代码
 NOP       ; 空指令以保证字节数为3
 
;====================================================================
; 
; BPB( BIOS Parameter Block ) 
; 
;====================================================================
SectorsPerCluster          DB 8          ; 每个簇的扇区数 ( 1 2 4 8 16 32 64 128 )
                                         ; 两者相乘不能超过32K(簇最大大小)
HiddenSectors              DD 63         ; 包含该FAT卷的分区之前的隐藏扇区数
frStartSector              DD 6398128    ;File record's start sector number of OS kernel.
drOffset                   DW 328        ;Data run list's offset relat to file record.
 
;====================================================================
; 
; EBPB ( Extended BIOS Parameter Block )
; 
;====================================================================
DriveNumber                DB ?   ; 用于INT 0x13的驱动器号，0x00为软盘，0x80为硬盘
 
;====================================================================
;
; Memory layout:
;     90000H - A0000H : Bootsector code,buffer,BP and SP
;     90000H - 90FFFH : Bootsector code,total 4K for NTFS file system.
;     91000H - 913FFH : Base pointer(BP) address space,used to keep some 
;                       local variables.1K bytes space totally.
;     91400H - 91FFFH : Stack area of boot sector code,3K.
;     92000H - 92FFFH : Temporary buffer for int 13h,4K space.
;     Actual start address is 90000H
;     
_BOOT_CODE:
 
;The following code is added by Garry.
    mov ax,DEF_ORG_START         ;;First,the boot code move itself to DEF_-
                                 ;;BOOT_START from DEF_ORG_START.
    mov ds,ax
 
    cld
    mov si,0x0000
    mov ax,DEF_BOOT_START
    mov es,ax
    mov di,0x0000
    mov cx,0x0200                ;;The boot sector's size is 512B
    rep movsb
 
    mov ax,DEF_BOOT_START        ;;Prepare the execute context.
    mov ds,ax
    mov ss,ax
    mov bp,0x13FF                 ;!!!!!!! CAUTION !!!!!!!!!!!!!
    mov sp,0x1FFF
    jmp DEF_BOOT_START : gl_bootbgn  ;;Jump to the DEF_BOOT_START to execute.
 
    ;***********************************************************************
    ;The following are some helper routines called by the loading process.
    ;***********************************************************************
 
    ;The first one is the disk sector reading routine,it reads one sector
    ;per time by calling interrupt 13H of BIOS.
    ;The DAP(Disk Accessing Packet) is in BP space,the DAP_XXXX_XXX style macros
    ;are used to access specific data member of DAP.
    ;You should prepare the DAP correctly before calling this routine since it
    ;assumes that all DAP data members are normally.
__READ_SECTOR:
    PUSHA ;Save all registers.
    mov eax,dword [HiddenSectors]
    add dword [BP - DAP_SECTOR_LOW],eax ;Add the hidden sector before current partition,adjust to
                                        ;physical sector number.
    MOV  AH,42H                         ; 功能号 
    LEA  SI ,[BP - DAP_PACKET_SIZE]     ; 地址包地址
    MOV  DL ,[DriveNumber]              ; 驱动器号
    INT  13H
    JC   __ERROR                        ; 读取失败 ------- DEBUG --------
    POPA                                ; 恢复寄存器 
    RET
 
    ;Error handling routines,any error or exception will cause the whole loading
    ;process fail.
__ERROR:                                ; 调用键盘中断，等待用户按键
    MOV  AH,00H
    INT  16H
 
__ERROR_1: 
                                        ; 重启计算机
    INT  19H
 
    
    ;Print out a dot to indicate the loading process.
__SHOW_PROGRESS:
    PUSHA
    MOV  AL,'.'
    MOV  AH,0EH
    MOV  BX,07H
    INT  10H
    POPA
    RET
 
    ;Read one cluster into memory address specified by bx(segment) and edx(offset).
    ;The cluster number to read is in EAX register.
    ;This routine calls __READ_SECTOR to conduct the reading,how many sectors will
    ;be calculated before reading.
__READ_CLUSTER:
    ;pusha
    push eax
    push ecx
    xor ecx,ecx
    mov cl,byte [SectorsPerCluster]
    mul ecx ;Now eax contains the start sector number.
.BEGIN_READ:
    ;Initialize the DAP now.
    MOV  DWORD [ BP - DAP_SECTOR_LOW ], EAX 
    MOV  WORD [BP - DAP_BUFFER_SEG  ], bx 
    ;MOV  WORD [BP - DAP_BUFFER_OFF ],  dx
    MOV  WORD [BP - DAP_BUFFER_OFF ], 0x00
    MOV  BYTE [BP - DAP_READ_SECTORS], cl
    call __READ_SECTOR
    ;add dx,512 / 16
    ;inc eax
    ;loop .BEGIN_READ
    call __SHOW_PROGRESS  ;Print out a dot to indicate the loading progress.
    ;popa
    pop ecx
    pop eax
    ret
 
    ;Read several clusters from disk into memory where is specified by bx and edx as
    ;__READ_CLUSTER routine.
    ;EAX contains the start cluster number and ECX contains how many clusters will be
    ;read.
__READ_CLUSTERS:
    pusha
.BEGIN_READ:
    call __READ_CLUSTER
    ;Update memory's address and cluster number.
    push eax
    push ecx
    xor ecx,ecx
    mov ax,512
    shr ax,0x04
    mov cl,byte [SectorsPerCluster]
    mul cl
    add bx,ax ;Now bx contains next cluster's segment address.
    pop ecx
    pop eax
    inc eax   ;Read next cluster.
    loop .BEGIN_READ
    popa
    ret
 
    ;Decode OS image's data run and load it into memory.
    ;EBX contains datarun's base address.
    ;__length,__offset and lcn are global variables to store the current data run.
    __length DD 00
    __offset DD 00
    lcn      DD 00
    __load_seg dw DEF_RINIT_START / 16 ;OS kernel's starting segment address.
    ;__load_off dd 0x00                 ;OS kernel's starting address offset.
__LOAD_DATARUN:
    pusha
 
.__BGN_DECODE:
    cmp byte [ebx],0
    jz .__JMP_TO_END  ;Can not jump to .__END_DECODE since out of range,
                      ;jump to a middle springboard first.
    jmp .__DECODE_CONT
.__JMP_TO_END:
    jmp .__END_DECODE
.__DECODE_CONT:       ;Continue to decode.
 
    ;Decode the length and offset's size.
    ;DH = length's size
    ;DL = offset's size
    mov dl,byte [ebx]
    mov dh,dl
    and dh,0x0f
    shr dl,0x04
    and dl,0x0f
 
    inc ebx
    xor ecx,ecx
 
    ;Decode length.
.__NEXT_LENGTH_BYTE:
    xor eax,eax
    mov al,byte [ebx]
    shl eax,cl
    add dword [__length],eax
    inc ebx
    add cl,0x08
    dec dh
    jnz .__NEXT_LENGTH_BYTE
 
    ;Decode offset.
    xor ecx,ecx
.__NEXT_OFFSET_BYTE:
    xor eax,eax
    mov al,byte [ebx]
    shl eax,cl
 
    cmp dl,0x01  ;If is the last byte.
    ja .__NOT_LAST_BYTE_OR_POSTIVE
 
    cmp byte [ebx],0x00
    jge .__NOT_LAST_BYTE_OR_POSTIVE
 
    ;Last byte is negative,a little complicated.
    add eax,dword [__offset]
    cmp cl,0x00
    ja .__OFFSET_SIZE_2
    or eax,0xFFFFFF00
    jmp .__OFFSET_CONT
.__OFFSET_SIZE_2:
    cmp cl,0x08
    ja .__OFFSET_SIZE_3
    or eax,0xFFFF0000
    jmp .__OFFSET_CONT
.__OFFSET_SIZE_3:
    cmp cl,0x10
    ja .__OFFSET_CONT
    or eax,0xFF000000
 
.__OFFSET_CONT:
    mov dword [__offset],eax
    neg eax
    sub dword [lcn],eax
    inc ebx
    jmp .__DECODE_OFFSET_OVER
 
.__NOT_LAST_BYTE_OR_POSTIVE:
    add dword [__offset],eax
    inc ebx
    add cl,0x08
    dec dl
    jnz .__NEXT_OFFSET_BYTE
 
    mov eax,dword [lcn]
    add eax,dword [__offset]
    mov dword [lcn],eax
 
.__DECODE_OFFSET_OVER:
    ;Should process this data run,i.e,load it into memory.
    push eax
    push ebx
    push ecx
    push edx
    mov bx,word [__load_seg]
    ;mov edx,dword [__load_off]
    mov eax,dword [lcn]
    mov ecx,dword [__length]
    call __READ_CLUSTERS     ;Read the data run into memory.
    ;Should update next data run's residential address.
    mov eax,dword [__length]
    mov bx,512
    mul bx
    mov bl,byte [SectorsPerCluster]
    mul bl
    shr eax,4   ;eax / 4,equals the segment address.
    add word [__load_seg],ax
    pop edx
    pop ecx
    pop ebx
    pop eax
 
    mov dword [__length],0x00
    mov dword [__offset],0x00
    jmp .__BGN_DECODE ;Decode another datarun.
 
.__END_DECODE:
    popa
    ret
 
 
gl_bootbgn:
    ;====================================================================
    ; 保存启动的磁盘编号
    ;====================================================================
    MOV  [DriveNumber],DL         ; 该值由BIOS设置，如果是从USB启动，该值为0x80
                                  ; 即为第一个硬盘的编号，该值将用于后续的磁盘
                                  ; 读取调用
 
    ;====================================================================
    ;
    ; 初始化DiskAddressPacket
    ; 使用时只需要修改字段：DATA_BUFFER_OFF DATA_BUFFER_SEG 
    ;       DAP_SECTOR_LOW  DAP_SECTOR_HIGH
    ;
    ;====================================================================
    MOV  DWORD [BP - DAP_SECTOR_HIGH ],00H
    MOV  BYTE  [BP - DAP_RESERVED1   ],00H
    MOV  BYTE  [BP - DAP_RESERVED2   ],00H
    MOV  BYTE  [BP - DAP_PACKET_SIZE ],10H
    MOV  WORD  [BP - DAP_BUFFER_SEG  ],DEF_INT13B_START
    MOV  WORD  [BP - DAP_BUFFER_OFF  ],00H
    MOV  EAX,  DWORD [frStartSector]
    MOV  DWORD [BP - DAP_SECTOR_LOW  ],EAX
    MOV  BYTE  [BP - DAP_READ_SECTORS],02H  ;Assume FR's size is 1024 bytes,2 sectors.
    call __READ_SECTOR
    mov ebx,0x2000    ;This is the offset of the loader's data segment,please caution.
                      ;Please refer the memory layout in front of this document,which
                      ;assumes 92000H(in liner address space) as the int13's data buffer,
                      ;the segment part of 92000H is 9000H,and offset part is 2000H,so
                      ;2000H must be added to ebx to locate to the actual position of data run.
    add bx,word [drOffset]
    call __LOAD_DATARUN
 
    ;Load over,run it.
_RUN_LOADER:                       ; 运行FDOSLDR.BIN
    MOV  DL,[DriveNumber]
    JMP  DEF_RINIT_START / 16 : 0
 
    Padding  TIMES 510-($-$$) db  00H
    Signature  DW 0AA55H
