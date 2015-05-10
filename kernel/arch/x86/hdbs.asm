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
;DATA_BUF_SEG EQU  0200H ; 用于读取根目录或文件内容的缓冲区(8K) 段地址
;DATA_BUF_OFF EQU  2000H
DATA_BUF_OFF EQU  3000H  ; Modified by Garry.
;STACK_ADDR  EQU  7BD0H ; 堆栈栈顶(注意：堆栈大小约为20K) 
OSLOADER_ADDR EQU  8000H ; FDOSLDR.BIN放入内存中的起始位置，这就意味着
        ; 装载程序及相关资源的尺寸不能超过608K
        ; 8000H - A000H (32K - 640K )
;OSLOADER_SEG EQU  0800H ; 起始段地址     
OSLOADER_SEG EQU 0100H  ; Start address of OS IMAGE,modified by Garry.
SECOND_SECTOR EQU  03H  ; 第二个引导扇区的扇区号(第四个扇区)
SECOND_ADDR  EQU  7E00H ; 第二个引导扇区的装载位置

;====================================================================
; 用堆栈保存若干中间变量( SS = 0 BP = 7C00H )
;====================================================================
FAT_START_SECTOR EQU  4  ; FAT表的起始扇区号  DWORD
ROOT_START_SECTOR EQU  8  ; 根目录的起始扇区号 DWORD
DATA_START_SECTOR EQU  12  ; 数据区起始扇区号  DWORD
FAT_ENTRY_SECTORS EQU  14  ; FAT表所占的扇区数  WORD
ROOT_ENTRY_SECTORS EQU  16  ; 根目录所占的扇区数 WORD
DIR_PER_SECTOR  EQU  17  ; 每个扇区所容纳的目录 BYTE
DISK_EXT_SUPPORT EQU  18     ; 磁盘是否支持扩展BIOS BYTE
CURRENT_CLUSTER  EQU  40  ; 当前正在处理的簇号 DWORD


;====================================================================  
; 扩展磁盘服务所使用的地址包
;====================================================================
DAP_SECTOR_HIGH  EQU  24  ; 起始扇区号的高32位 ( 每次调用需要重置 ) DWORD
DAP_SECTOR_LOW  EQU  28  ; 起始扇区号的低32位 ( 每次调用需要重置 ) DWORD
DAP_BUFFER_SEG  EQU  30  ; 缓冲区段地址   ( 每次调用需要重置 ) WORD
DAP_BUFFER_OFF  EQU  32  ; 缓冲区偏移   ( 每次调用需要重置 ) WORD  
DAP_RESERVED2  EQU  33  ; 保留字节
DAP_READ_SECTORS EQU  34  ; 要处理的扇区数(1 - 127 )
DAP_RESERVED1  EQU  35  ; 保留字节
DAP_PACKET_SIZE  EQU  36  ; 包的大小为16字节

;====================================================================
; 
; 目录项结构(每个结构为32字节)
; 
;====================================================================
OFF_DIR_NAME    EQU  0  ; 目录项的偏移  BYTE[11]
OFF_DIR_ATTRIBUTE   EQU  11  ; 目录属性   BYTE
OFF_NT_RESERVED    EQU  12  ; 保留属性   BYTE
OFF_CREATE_TIME_HUNDREDTH EQU  13  ; 创建时间   BYTE
OFF_CREATE_TIME    EQU  14  ; 创建时间   WORD
OFF_CREATE_DATE    EQU  16  ; 创建时间   WORD
OFF_LAST_ACCESS_DATE  EQU  18  ; 上次访问时间  WORD
OFF_START_CLUSTER_HIGH  EQU  20  ; 起始簇号高位  WORD
OFF_LAST_UPDATE_TIME  EQU  22  ; 上次更新时间  WORD
OFF_LAST_UPDATE_DATE  EQU  24  ; 上次更新时间  WORD
OFF_START_CLUSTER_LOW  EQU  26  ; 起始簇号低位  WORD
OFF_FILE_SIZE    EQU  28  ; 文件尺寸   DWORD

; 相关常量
DIR_NAME_DELETED   EQU  0E5H ; 该项已经被删除
DIR_NAME_FREE    EQU  00H  ; 该项是空闲的(其后也是空闲的)
DIR_NAME_DOT    EQU  2EH  ; 特殊目录 . 或 ..
DIR_NAME_SPACE    EQU  20H  ; 不允许的字符
DIR_ENTRY_SIZE    EQU  32  ; 每个目录项的尺寸，其结构如上所示 

;文件属性
DIR_ATTR_READONLY   EQU  01H  ; 只读文件
DIR_ATTR_HIDDEN    EQU  02H  ; 隐藏文件
DIR_ATTR_SYSTEM    EQU  04H  ; 系统文件
DIR_ATTR_VOLUME    EQU  08H  ; 卷标号(只可能出现在根目录中)
DIR_ATTR_SUBDIR    EQU  10H  ; 子目录
DIR_ATTR_ARCHIVE   EQU  20H  ; 归档属性
DIR_ATTR_LONGNAME   EQU  0FH  ; 长文件名
DIR_ATTR_LONGNAME_MASK  EQU  3FH  ; 长文件名掩码

; 簇属性
CLUSTER_MASK    EQU  0FFFFFFFH ; 簇号掩码(FAT32=>FAT28)
CLUSTER_FREE    EQU  00000000H ; 簇是空闲的
CLUSTER_RESERVED   EQU  00000001H ; 簇是保留的
CLUSTER_MIN_VALID   EQU  00000002H ; 最小有效簇号
CLUSTER_MAX_VALID   EQU  0FFFFFF6H ; 最大有效簇号
CLUSTER_BAD     EQU  0FFFFFF7H ; 坏簇
CLUSTER_LAST    EQU  0FFFFFF8H   ;0xFFFFFFF8-0xFFFFFFFF表示文件的最后一个簇

;Start segment address of bootsect after loaded into memory.
DEF_ORG_START EQU 7C0H

;Base segment address of the bootsect after relocation
;DEF_BOOT_START EQU 9F00H
;DEF_BOOT_START EQU 9B00H  --Modified by Garry in 28.May
DEF_BOOT_START EQU 9000H

;Start address of OS image.
DEF_RINIT_START EQU 1000H

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
SectorsPerCluster  DB 32 ; 每个簇的扇区数 ( 1 2 4 8 16 32 64 128 )
        ; 两者相乘不能超过32K(簇最大大小)
ReservedSectors   DW 36 ; 从卷的第一个扇区开始的保留扇区数目；
        ; 该值不能为0，对于FAT12/FAT16，该值通常为1；
        ; 对于FAT32，典型值为32；
NumberOfFATs   DB 2 ; 卷上FAT数据结构的数目，该值通常应为2
HiddenSectors   DD 38 ; 包含该FAT卷的分区之前的隐藏扇区数

;====================================================================
; 
; EBPB ( Extended BIOS Parameter Block )
; 
;====================================================================
SectorsPerFAT32   DD 14994   ; 对于FAT32，该字段包含一个FAT的大小，而SectorsPerFAT16
          ; 字段必须为0;
RootDirectoryStart  DD 2   ; 根目录的起始簇号，通常为2；
DriveNumber    DB ?   ; 用于INT 0x13的驱动器号，0x00为软盘，0x80为硬盘

;====================================================================
;
; 真正的启动代码从这开始( 偏移：0x3E ) 
; 其功能是搜索磁盘的根目录，查找FDOSLDR.BIN文件，将其读入内存并运行。
;
;====================================================================
;====================================================================
;
; Memory layout:
;     9F000H - A0000H : Bootsector code,buffer,BP and SP
;     9F000H - 9F3FFH : Boot sector code
;     9F400H - 9F7FFH : Base address space.
;     9F800H - 9FFFEH : Stack area of boot sector code.
;     A0000H - A2000H : Temporary buffer for int 13h.
;     Actual start address is 90000H
;     
_BOOT_CODE:

;The following code is added by Garry.
    cli                          ;;Mask all maskable interrupts.
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
    mov es,ax
    mov ss,ax
    mov bp,0x7f0                 ;!!!!!!! CAUTION !!!!!!!!!!!!!
    mov sp,0xffe
    jmp DEF_BOOT_START : gl_bootbgn  ;;Jump to the DEF_BOOT_START to execute.

gl_bootbgn:
 ; 初始化相关寄存器及标志位
 ;CLI      ; 先关掉中断
 ;CLD      ; 方向为向前递增
 ;XOR  AX,AX   ; AX = 0
 ;MOV  DS,AX   ; 设置数据段寄存器 DS:SI
 ;MOV  ES,AX   ; 设置附加段寄存器 ES:DI
 ;MOV  SS,AX   ; 设置堆栈段寄存器
 ;MOV  BP,7C00H  ; 设置基址寄存器
 ;MOV  SP,STACK_ADDR ; 设置堆栈栈顶
 STI      ; 允许中断  --Modified by Garry.xin--

 ;====================================================================
 ; 保存启动的磁盘编号
 ;====================================================================
 MOV  [DriveNumber],DL; 该值由BIOS设置，如果是从USB启动，该值为0x80
       ; 即为第一个硬盘的编号，该值将用于后续的磁盘
       ; 读取调用


 ;====================================================================  
 ; 准备FAT32文件系统常用的常数，以便后面的操作
 ;====================================================================
 ;
 ; [隐藏扇区][保留扇区][FAT][DATA]
 ;
 ;====================================================================
 
 MOV  BYTE [BP - DIR_PER_SECTOR],16 ; AL    = DirEntriesPerSector
 
 ; FAT起始扇区
 ; FAT起始扇区 = Hidden+Reserved
 MOV  AX ,WORD [ReservedSectors]
 CWD          ; AX => DX : AX
 ADD  AX, WORD [HiddenSectors]
 ADC   DX, WORD [HiddenSectors+2]  
 MOV  WORD[ BP - FAT_START_SECTOR  ],AX 
 MOV   WORD[ BP - FAT_START_SECTOR+2],DX 
 
 
 ; FAT表所占的扇区数
 ; FAT_SECTORS = NumberOfFAT * SectorsPerFAT
 XOR  EAX,EAX
 MOV  AL, BYTE [NumberOfFATs]  ; FAT的数目
 MOV  EBX,DWORD [SectorsPerFAT32]
 MUL  EBX        ; 乘积放入 EDX:EAX
 MOV  DWORD [ BP - FAT_ENTRY_SECTORS  ] , EAX
 
 ; 计算数据区起始扇区
 ADD  EAX ,DWORD[ BP - FAT_START_SECTOR  ]
 MOV  DWORD [ BP - DATA_START_SECTOR ],EAX 
 
 
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
 MOV  BYTE  [BP - DAP_READ_SECTORS],01H
 ;MOV  WORD  [BP - DAP_BUFFER_SEG  ],00H
 MOV  WORD  [BP - DAP_BUFFER_SEG  ],DEF_BOOT_START
 MOV  BYTE  [BP - DAP_READ_SECTORS],01H  ; 每次只读取一个扇区 
 
 ; 下面开始查找根目录并且装载FDOSLDR.BIN
 JMP  _SEARCH_LOADER
 
;====================================================================
; 错误处理
;====================================================================
 
__ERROR:
 ; 调用键盘中断，等待用户按键
 MOV  AH,00H
 INT  16H

__ERROR_1: 
 ; 重启计算机
 INT  19H 

;====================================================================
; 
; 子过程
; 
;====================================================================

;====================================================================
; 
; 读取一个磁盘扇区
; 输入： 已经设置了DAP中相应的字段
; 限制： 不能读取超过一个簇的内容   
; 
;====================================================================
ReadSector:

 PUSHA  ; 保存寄存器
 
;====================================================================
; INT 13H  AH = 42H 扩展磁盘调用
;====================================================================
 ; 每次读取一个扇区
 MOV  AH,42H         ; 功能号 
 LEA  SI ,[BP - DAP_PACKET_SIZE]    ; 地址包地址

 ; 驱动器号
 MOV  DL ,[DriveNumber]      ; 驱动器号
 INT  13H
 JC   __ERROR        ; 读取失败 ------- DEBUG --------
 POPA       ; 恢复寄存器 
 RET

;====================================================================
; 数据区
;====================================================================
LoaderName     db "HCNIMGE BIN"       ; 第二阶段启动程序 FDOSLDR.BIN
ProcessDot     db "."

;====================================================================
; 查找根目录，检查是否有 FDOSLDR.BIN文件
;====================================================================
_SEARCH_LOADER: 


 ; 设置缓冲区
 MOV  WORD [ BP - DAP_BUFFER_OFF  ], DATA_BUF_OFF ; 0000:1000H
 
 ; 根目录起始扇区号
 MOV  EAX,DWORD[RootDirectoryStart]
 MOV  DWORD[ BP - CURRENT_CLUSTER ],EAX

; 检查下一个簇
_NEXT_ROOT_CLUSTER:

 ; 根据簇号计算扇区号
 DEC  EAX
 DEC  EAX  ; EAX = EAX - 2
 XOR  EBX,EBX 
 MOV  BL, BYTE[ SectorsPerCluster]
 MUL  EBX 
 ADD  EAX,DWORD[ BP- DATA_START_SECTOR]
 MOV  DWORD[ BP - DAP_SECTOR_LOW  ], EAX
 MOV  DL,[SectorsPerCluster]

; 检查下一个扇区
_NEXT_ROOT_SECTOR:
  
 ; 依次读取每个根目录扇区，检查是否存在FDOSLDR.BIN文件
 CALL ReadSector
 
 ; 检查该扇区内容
 MOV  DI,DATA_BUF_OFF
 MOV  BL,BYTE [ BP - DIR_PER_SECTOR]

; 检查每一个目录项
_NEXT_ROOT_ENTRY:
 CMP  BYTE [DI],DIR_NAME_FREE
 JZ  __ERROR    ; NO MORE DIR ENTRY
 
 ; 检查是否装载程序
 PUSH  DI       ; 保存DI
 MOV  SI,LoaderName
 MOV  CX,11
 REPE  CMPSB 
 JCXZ  _FOUND_LOADER    ; 装载Loader并运行
  
 ; 是否还有下一个目录项(内层循环)
 POP  DI
 ADD   DI,DIR_ENTRY_SIZE
 DEC  BL 
 JNZ   _NEXT_ROOT_ENTRY
 
 ; 检查是否还有下一个扇区可读(外层循环)
 DEC  DL
 JZ  _CHECK_NEXT_ROOT_CLUSTER
 INC  DWORD [ BP - DAP_SECTOR_LOW ] ; 增加扇区号
 JMP  _NEXT_ROOT_SECTOR 
 
; 检查下一个簇
_CHECK_NEXT_ROOT_CLUSTER:

 ; 计算FAT所在的簇号和偏移 
 ; FatOffset = ClusterNum*4
 XOR  EDX,EDX
 MOV  EAX,DWORD[BP - CURRENT_CLUSTER]
 SHL  EAX,2
 XOR  ECX,ECX
 MOV  CX,512
 DIV  ECX  ; EAX = Sector EDX = OFFSET
 ADD  EAX,DWORD [BP - FAT_START_SECTOR  ]
 MOV  DWORD [ BP - DAP_SECTOR_LOW ], EAX 
   
 ; 读取扇区
 CALL  ReadSector
  
 ; 检查下一个簇
 MOV  DI,DX
 ADD  DI,DATA_BUF_OFF
 MOV  EAX,DWORD[DI]  ; EAX = 下一个要读的簇号
 AND  EAX,CLUSTER_MASK
 MOV  DWORD[ BP - CURRENT_CLUSTER ],EAX
 CMP  EAX,CLUSTER_LAST  ; CX >= 0FFFFFF8H，则意味着没有更多的簇了
 JB  _NEXT_ROOT_CLUSTER
 JMP  __ERROR

;====================================================================
; 装载FDOSLDR.BIN文件
;====================================================================
_FOUND_LOADER:
 ; 目录结构地址放在DI中
 POP  DI
 XOR  EAX,EAX
 MOV  AX,[DI+OFF_START_CLUSTER_HIGH] ; 起始簇号高16位
 ;SHL  AX,16
 SHL  EAX,16 ;----- Modified by Garry.xin----
 MOV  AX,[DI+OFF_START_CLUSTER_LOW]  ; 起始簇号低16位
 MOV  DWORD[ BP - CURRENT_CLUSTER ],EAX
 MOV  CX, OSLOADER_SEG      ; CX  = 缓冲区段地址 
 
  
_NEXT_DATA_CLUSTER:
 
 ; 根据簇号计算扇区号
 DEC  EAX
 DEC  EAX  ; EAX = EAX - 2
 XOR  EBX,EBX 
 MOV  BL, BYTE[ SectorsPerCluster]
 MUL  EBX 
 ADD  EAX,DWORD[ BP- DATA_START_SECTOR]
 MOV  DWORD[ BP - DAP_SECTOR_LOW  ], EAX
 MOV  DL,[SectorsPerCluster]

 ; 设置缓冲区
 MOV  WORD [ BP - DAP_BUFFER_SEG   ],CX
 MOV  WORD [ BP - DAP_BUFFER_OFF   ],00H
   
 ; 每个簇需要读取的扇区数
 MOV  BL , BYTE [SectorsPerCluster]

_NEXT_DATA_SECTOR:
 ; 读取簇中的每个扇区(内层循环)
 ; 注意 : 通过检查文件大小，可以避免读取最后一个不满簇的所有大小
 ; 读取数据扇区
 CALL  ReadSector

 ;Show loading process,modified by Garry.Xin
 PUSH BX
 MOV  AL,'.'
 MOV  AH,0EH
 MOV  BX,07H
 INT  10H
 POP  BX
 
 ; 更新地址，继续读取
 MOV  AX, 512
 ADD  WORD  [BP - DAP_BUFFER_OFF],AX 
 INC  DWORD [BP - DAP_SECTOR_LOW]  ; 递增扇区号
 DEC  BL        ; 内层循环计数
 JNZ  _NEXT_DATA_SECTOR
  
 
 ; 检查下一个簇
  
 ; 更新读取下一个簇的缓冲区地址
 MOV  CL,BYTE [ SectorsPerCluster ]
 MOV  AX ,512
 SHR  AX ,4
 MUL  CL
 ADD  AX ,WORD [ BP - DAP_BUFFER_SEG ] 
 MOV  CX,AX ; 保存下一个簇的缓冲区段地址
 
 ;====================================================================
 ;
 ; 检查是否还有下一个簇(读取FAT表的相关信息)
 ;  LET   N = 数据簇号
 ;  THUS FAT_BYTES  = N*4  (FAT32)
 ;      FAT_SECTOR = FAT_BYTES / BytesPerSector
 ;    FAT_OFFSET = FAT_BYTES % BytesPerSector
 ;
 ;====================================================================
 
 ; 计算FAT所在的簇号和偏移 
 MOV  EAX,DWORD [BP - CURRENT_CLUSTER]
 XOR  EDX,EDX
 SHL  EAX,2
 XOR  EBX,EBX
 MOV  BX,512
 DIV  EBX   ; EAX = Sector  EDX = Offset
 
 ; 设置缓冲区地址
 ADD  EAX,DWORD [BP - FAT_START_SECTOR  ]
 MOV  DWORD [ BP - DAP_SECTOR_LOW ], EAX 
 MOV   WORD [BP - DAP_BUFFER_SEG  ], DEF_BOOT_START 
 MOV  WORD [BP - DAP_BUFFER_OFF  ], DATA_BUF_OFF ; 0000:1000H

 ; 读取扇区
 CALL  ReadSector
  
 ; 检查下一个簇
 MOV  DI,DX
 ADD  DI,DATA_BUF_OFF
 MOV  EAX,DWORD[DI]  ; EAX = 下一个要读的簇号
 AND  EAX,CLUSTER_MASK
 MOV  DWORD[ BP - CURRENT_CLUSTER ],EAX
 CMP  EAX,CLUSTER_LAST  ; CX >= 0FFFFFF8H，则意味着没有更多的簇了
 JB  _NEXT_DATA_CLUSTER

;读取完毕
_RUN_LOADER: 

 ; 运行FDOSLDR.BIN
 ;MOV  DL,[DriveNumber]
 JMP  DEF_RINIT_START / 16 : 0

Padding  TIMES 510-($-$$) db  00H
Signature  DW 0AA55H
