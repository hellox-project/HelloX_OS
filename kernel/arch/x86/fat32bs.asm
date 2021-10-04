;-------------------------------------------------------------------------
; Boot loader of HelloX OS for storages formatted as FAT32.
; Author: Garry.Xin
; Version: V1.0
; Last modified date: 2021/06/05
; 
; This loader is the new version of HelloX's loader.
; It is written to the first sector of storage, include
; USB stick, harddisk with IDE/AHCI interface, and others.
; BIOS loads the loader into memory at 0x7C00, and handovers
; CPU to it, then HelloX university starts.
;
; Boot loader: This module, just one sector's size;
; OS Loader: A more complicated loader to load the whole HelloX
;            image into memory. OS loader is loaded by boot loader.
;
; MEM layout before relocation:
;    0x7C00 ~ 0x7E00: boot loader in memory loaded by BIOS.
; MEM layout after relocation:
;    0x00000 ~ 0x00FFF: reserved, unchanged BIOS area, 4K;
;    0x01000 ~ 0x8FFFF: HelloX OS loader, loaded by boot loader, 572K;
;    0x90000 ~ 0x901FF: Reserved, 512 bytes;
;    0x90200 ~ 0x903FF: FAT BPB loaded by boot loader, 512 bytes;
;    0x90400 ~ 0x905FF: Temporary space for useful FAT32 variables, 512 bytes;
;    0x97C00 ~ 0x97DFF: Boot loader after relocation, 512 bytes;
;    0x97E00 ~ 0x97FFF: Stack of boot loader, 512 bytes;
;    0x98000 ~ 0x9FFFF: Root cluster's buffer;
;    0xA0000 ~ 1M: Unused, 640K~, high end memory,...;
;-------------------------------------------------------------------------

; All of these are defining offsets in memory to store data later, 
; or to load data into.

; Segment address of the boot loader after relocation.
; The boot loader is loaded into 0x7C00 by BIOS, it will
; relocate itself to 0x90000(576K) to yeild space for
; HelloX's OS loader, which is start from 0x1000(4K).
%define __BOOT_LOADER_SEGMENT 0x9000

; This defines offset to first MBR entry-0x10
%define MBR 0x7DAE

; Defines address to a buffer for loading root directory clusters.
; Please be noted this is the segment address of buffer, so 0x10
; will be multiplied to get the actual address in this code file.
%define __ROOT_BUFFER_SEG 0x9000
%define __ROOT_BUFFER_OFF 0x8000
; Offset of this bootloader in memory, in segment.
%define __BOOT_LOADER_OFFSET 0x7C00
; Segment of the OS loader after loaded into memory.
%define __OS_LOADER_START 0x100 ;(0x1000->0x100)

; Define fixed memory locations, where useful data will be saved into.
%define BPB_LBA 0x0400    ; <--Dword - Points to LBA of first sector of loaded partition
%define FAT_LBA 0x0404    ; <--Dword - Points to LBA of first FAT sector
%define DATA_LBA 0x0408   ; <--Dword - Points to LBA of first DATA sector
%define CLUSTER BPB+RootCluster    ; <--Dword - Points to Temporary number of cluster to be loaded
%define DEV 0x0414        ; <--Byte  - Points to Current storage device index (for bios interrupts)

; Memory address to FAT BPB which is loaded later.
%define BPB 0x0200 ; BPB offset address.
; And some offsets in loaded BPB for calculating FAT values.
  %define BytesPerSec   0x0B ; <--Word  - Points to Bytes Per Sector
  %define SecPerCluster 0x0D ; <--Byte  - Points to Sectors Per Cluster
  %define ResvdSecs     0x0E ; <--Word  - Points to Reserved Sectors
  %define FatCount      0x10 ; <--Byte  - Points to Number of FATs
  %define FatSize       0x24 ; <--Dword - Points to Size of one FAT
  %define RootCluster   0x2C ; <--Dword - Points to Root Cluster Number

; Fixed bytes per sector as 512.
%define __BYTES_PER_SECTOR 0x200

; Error codes and their meanings.
%define ERR_NO_EXT      0x30 ; <-- Extended BIOS interrupts not supported
%define ERR_NO_ACTIVE   0x31 ; <-- No active partition found
%define ERR_NOT_FAT32   0x32 ; <-- Active partition is not FAT32
%define ERR_NOT_FOUND   0x33 ; <-- File is not present in root directory
%define ERR_HARDWARE    0x34 ; <-- Loading sector error(probably hardware error)

; Fat32 partition types.
%define FAT32 0x0B
%define FAT32LBA 0x0C
%define HIDFAT32 0x1B
%define HIDFAT32LBA 0x1C

; Relocate the boot loader from 0x00/0x7C00(segment/offset)
; to 0x9000/0x7C00.
__BOOT_LOADER_RELOCATION:
  xor ax, ax
  mov ds, ax
  mov ax, __BOOT_LOADER_SEGMENT
  mov es, ax
  mov si, __BOOT_LOADER_OFFSET
  mov di, __BOOT_LOADER_OFFSET
  mov cx, 0x0200
  cld
  rep movsb
  jmp __BOOT_LOADER_SEGMENT : (__BOOT_LOADER_OFFSET + __BOOT_LOADER_BEGIN)

__BOOT_LOADER_BEGIN:
; Now init the segment registers accordingly.
  mov ds, ax
  mov ss, ax
  mov sp, 0x7FFF

; Checking if BIOS supports extended int13.
;  mov ah, 0x41
;  mov bx, 0x55AA
;  int 0x13
;    mov al, ERR_NO_EXT ;if not - save error code and jump to error routine
;    jc ErrorRoutine

; Try to find active partition.
  mov bx, MBR
findActivePart:
  add bl, 0x10
  cmp bl, 0xFE
    mov al, ERR_NO_ACTIVE ;if no bootable partition was found - save error code and jump to error routine
    je ErrorRoutine
  cmp byte [bx], 0x80
  jne findActivePart;

; Check active partition type, only FAT32 is supported up to now.
  cmp byte[bx+0x04], FAT32
  je loadBPB
  cmp byte[bx+0x04], FAT32LBA
  je loadBPB
;  cmp byte[bx+0x04], HIDFAT32
;  je loadBPB
;  cmp byte[bx+0x04], HIDFAT32LBA
;  je loadBPB
  mov al, ERR_NOT_FAT32
  jmp ErrorRoutine

; When found active partition, load it's first sector to get FAT information.
; TODO: Review the initialization of DAP, since buffer's base address may changed
;       after relocation of boot loader.
loadBPB:
  mov ecx, dword [bx+0x08]
  mov dword [DAP.address+__BOOT_LOADER_OFFSET], ecx
  mov byte [DEV], dl
  call dapLoad ; BPB will be loaded into [BPB].

; Then calculate and save FAT information.
; LBA of first FAT sector.
  mov ecx, dword [DAP.address+__BOOT_LOADER_OFFSET]
  mov dword [BPB_LBA], ecx
  ;add cx, word [BPB+ResvdSecs] ; May overflow here.
  xor eax, eax
  mov ax, word[BPB+ResvdSecs]
  add ecx, eax
  mov dword [FAT_LBA], ecx

; LBA of first DATA sector
  xor eax, eax
  mov al, byte [BPB+FatCount]
  mul dword [BPB+FatSize]
  add ecx, eax
  xor eax, eax
  mov al, byte [BPB+SecPerCluster]
  sub ecx, eax  ; First 2 cluster number is reserved.
  sub ecx, eax
  mov dword[DATA_LBA], ecx

; Then search root directory for OS loader.
searchCluster:
  cmp dword [CLUSTER], 0x0FFFFFF8
  mov al, ERR_NOT_FOUND ; if file wasn't found(searched whole root directory)-save error code
  jae ErrorRoutine      ; and jump to error routine.
  ; Load one cluster of root directory into root cluster buffer.
  ; Init the DAP's segment as root cluster buffer's segment, and
  ; set the DAP's offset as the offset of root cluster buffer.
  mov word [DAP.segment+__BOOT_LOADER_OFFSET], __ROOT_BUFFER_SEG
  mov word [DAP.offset+__BOOT_LOADER_OFFSET], __ROOT_BUFFER_OFF
  call lnc
  ; Prepare registers for searching cluster.
  ; In lnc routine, [DAP.segent + __BOOT_LOADER_OFFSET] is increased
  ; one cluster << 4bits, the upper boundary of cluster buffer.
  ; So use this value as loop's end condition.
  xor edx, edx
  mov dx, word [DAP.segment+__BOOT_LOADER_OFFSET]
  shl edx, 0x04
  mov ebx, __ROOT_BUFFER_OFF - 0x20 ; <-- Offset to Root Buffer-0x20.
  ; Prepare registers for each filename comparison.
  searchPrep:
  add bx, 0x0020
  cmp edx, ebx ; <--- if whole cluster was checked - load another one.
    je searchCluster
  ; Check if entry has attribute VOLUME_ID or DIRECTORY.
  test byte[es:bx+0x0b], 0x18
    jnz searchPrep
  mov di, bx
  mov cx, 0x000B ; 11 bytes file name.
  mov si, __BOOT_LOADER_OFFSET + __OS_LOADER
  ; Compare single characters of filenames from root directory with OS loader.
  repe cmpsb
    jne searchPrep ;<--- if filenames don't match, try another one.

; If filename matches get this file's cluster number.
match:
  mov ax, word[es:bx+0x14]
  mov dx, word[es:bx+0x1A]
  mov word [CLUSTER], dx
  mov word [CLUSTER+2], ax
  mov word [DAP.segment+__BOOT_LOADER_OFFSET], __OS_LOADER_START
  mov word [DAP.offset+__BOOT_LOADER_OFFSET], 0
; and load it into memory.
loadfile:
  call lnc
  ; Show out a dot to indicate progress: al = 0x2E('.')
  ;mov ax, 0x0E2E
  ;int 0x10
  cmp dword [CLUSTER], 0x0FFFFFF8
  jb loadfile

; This is where function jumps when whole file is loaded
fileLoaded:
; Jump to loaded Code, all registers except CS are not inited.
  mov dl, byte[DEV]
  jmp __OS_LOADER_START : 0x0000 ; THE END.

; Routines used.
; lnc - Load Next Cluster:
; loads cluster into memory, and saves next cluster's number.
lnc:
; this part saves next cluster's number
  push dword [CLUSTER]
  push word [DAP.segment+__BOOT_LOADER_OFFSET]
  push word [DAP.offset+__BOOT_LOADER_OFFSET]
  xor eax, eax
  mov al, 0x04
  mul dword[CLUSTER] ; eax contains the offset of the cluster in fat.
  xor ebx, ebx
  mov bx, __BYTES_PER_SECTOR ;word [BPB+BytesPerSec]
  div ebx ; now eax contains the sector index of next cluster, edx contains
          ; the offset of next cluster number in sector.
  add eax, dword[FAT_LBA] ; eax then contains the obsolute sector index of cluster.
  ; Load the sector that contains next cluster's index into memory.
  mov dword[DAP.address+__BOOT_LOADER_OFFSET], eax
  mov word [DAP.segment+__BOOT_LOADER_OFFSET], __ROOT_BUFFER_SEG
  mov word [DAP.offset+__BOOT_LOADER_OFFSET], __ROOT_BUFFER_OFF
  mov word [DAP.count+__BOOT_LOADER_OFFSET], 0x0001
  xchg bx, dx ; bx contains the offset of next cluster number in sector.
  call dapLoad
  mov eax, dword [bx+__ROOT_BUFFER_OFF] ; eax contains the next cluster number.
  and eax, 0x0FFFFFFF ; clear the upper 4 bits.
  mov dword [CLUSTER], eax ; Now [CLUSTER] contains the next cluster number.

; This part loads current cluster into memory.
  pop word [DAP.offset+__BOOT_LOADER_OFFSET]
  pop word [DAP.segment+__BOOT_LOADER_OFFSET]
  xor eax, eax
  mov al, byte[BPB+SecPerCluster]
  mov byte[DAP.count+__BOOT_LOADER_OFFSET], al
  pop dword ebx ; ebx contains the cluster number to load.
  mul ebx
  add eax, dword[DATA_LBA]
  mov dword[DAP.address+__BOOT_LOADER_OFFSET], eax
  call dapLoad
; Also sets buffer for next cluster right after loaded one
  xor eax, eax
  mov al, byte[BPB+SecPerCluster]
  ;mul word [BPB+BytesPerSec] ;
  ;shl eax, 9 ; eax * __BYTES_PER_SECTOR
  ;shr ax, 0x04
  shl eax, 5 ; combine uper 2 instructions.
  add word [DAP.segment+__BOOT_LOADER_OFFSET], ax
  ret

; Simple routine loading sectors according to DAP.
dapLoad:
  mov ah, 0x42
  mov si, DAP+__BOOT_LOADER_OFFSET
  mov dl, byte [DEV]
  int 0x13
  jnc .end
  mov al, ERR_HARDWARE
  jmp ErrorRoutine
.end:
  ret

; Simple Error routine(only prints one character - error code)
ErrorRoutine:
mov ah, 0x0E
int 0x10
jmp $

; File name of OS loader in 8/3 format.
__OS_LOADER : db 'OSLOADR BIN' 

DAP:
.size:    db 0x10
.null:    db 0x00
.count:   dw 0x0001
.offset:  dw 0x0000
.segment  dw 0x9020 ; (__BOOT_LOADER_SEGMENT + BPB) >> 4, segment address of BPB.
.address: dq 0x0000000000000000

; Pad to 512 bytes.
times 510 - ($ - $$) db 0x00
; Last word must be 0xAA55.
dw 0xAA55
