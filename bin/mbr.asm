;mbr.asm - Master Boot Record V 2.1
;By pANZERNOOb 2015

;The Master Boot Record(MBR) is a type of bootrecord that contains a table defining
;the partitions on that physical disk and a program that loads the first volume boot record(VBR) from
;any bootable partitions found. A MBR is only needed for fixed hard drives or disks that have
;been partitioned, it is not recommended for use by removable disks such as USB drives however 
;this version should work on any boot device you write it to and will NEVER conflict with the filsystem
;that the VBR resides on.

;This is most definitly the last revision I will make to this code as not much changed
;this time and I'm certain that all major(and some minor) bugs have been worked out,
;One specific bug to point out, some BIOSes act diffrently in how they treat removable disks,
;it drove me insane trying to figure out why the code ran fine from a USB drive on every machine
;except the one I wrote it with, turns out that the BIOS reported the INT13h extensions were
;present for my disk but that I never checked the Bitmask to see if it would actually let me use them,
;this led to the read function assuming that it could use the extensions when it wasn't allowed.

;Revision: A short CPUID algorithm was added to the MBR, VBR, and OSLDR.SYS to prevent the bootloader
;from crashing legacy machines in the event that one finds it a good idea to run a modern bootloader
;on their old IBM 5150. I plan on making updates to almost all my loader components and possibly even
;come up with an extension to the multiboot specification to include various additional information
;that would be difficult to detect in PMODE such as CPU speed and things like that. This MBR will NOT
;boot on anything older than a 80486, 8086/80186 machines will not display an error message rather
;they will always boot from next device or ROM Basic. I do not have an original IBM PC lying around
;however this loader was emulated for a legacy PC and works in it's entirety(except the error message)

;Layout of MBR:
;	440 BYTES - Boot code
;	1   DWORD - Disk Signature
;	1   WORD  - Reserved
;   	64  BYTES - Partition table
;	1   WORD  - Boot signature, 0x55AA
;Total size:512 bytes, one sector

;Assemble using NASM: nasm mbr.asm -f bin -o mbr.bin
;MBR.asm--------------------------------------------------------------------------------------------------------------------------;
bits 16										;Use 16-bit opcodes
org 0x0600									;MBR is loaded to 0000:7C00 by BIOS, then relocated to 0000:0600

;Copy MBR from 0000:7C00 to 0000:0600 to make space for the Volume Boot Record
start:
	xor ax,ax								;Zero AX
	mov ds,ax								;Data Segment	:0000
	mov es,ax								;Extra Segment	:0000
	mov ss,ax								;Stack Segment	:0000
	mov sp,0x7C00							;Stack Pointer	:7C00
	push sp									;Push stack pointer
	pop cx									;Pop Stack pointer into CX
	cmp cx,sp								;On an 8086/80186 CPU SP gets decremented before being pushed to the stack
	je Not_8086							    ;so if the SP on the stack is diffrent than the original we can identify an 8086/80186
	int 0x18								;Boot from next device or ROM BASIC, MBR will work on a legacy 8086/80186 machine but the VBR wont
Not_8086:									;I don't have an IBM 5150 lying around and my normal error handler doesn't work in an emulator, so abort
	cld										;Increment SI|DI,Decrement CX
	mov si,0x7C00							;SI=>Base of MBR
	mov di,0x0600							;DI=>0600
	mov cx,0x0200							;512 bytes
	rep movsb								;Copy 512 bytes from 0000:7C00 to 0000:0600
	push ax									;Code Segment	:0000
	push MBR_Main							;Offset			:MBR_Main
	retf		   							;Far return to relocated code
;After being moved the code checks the CPU again and then proceeds to look through the partition entries to find a bootable one.
MBR_Main:	
	lea bx,[CPUID_Fail]						;CS:BX => CPUID failure ISR
	mov WORD [0x18],bx						;Change offset
	mov WORD [0x1A],ax						;and code segement which should be 0000
	xadd bl,bl							    ;This instruction only will cause an Invalid Opcode Exception(6h) on CPUs older than the 80486
	sti								        ;Re-enable interrupts
	mov cx,0x04  							;Check 4 Partition entries
	lea bp,[PT1_Status]						;BP=>Start of Partition Table
Check_Table:
	cmp [bp],BYTE 0x80						;Is partition active?
	je  Found_Entry							;Yes, jump over error handler
	cmp [bp],BYTE 0x00 						;Is flag Valid?
	jne Invalid_Table						;Nope, table is invalid
	add bp,0x10								;Add 16 Bytes
	loop Check_Table						;Check all 4 entries
	int 0x18								;Load next boot device or ROM BASIC
;A bootable partition was found, try to load it. First however check INT13h support
Found_Entry:								
	mov BYTE [bp],dl						;Overwrite Bootable flag with Drive number given in DL
	mov ah,0x41							    ;INT 13h AH=41h: Check Extensions Present	
	mov bx,0x55AA							;Must be 55AA
	int 0x13							    ;Check extensions
	jc  No_Ext							    ;Carry flag set? No extensions
;NOTE: A really irritating bug/design flaw in some BIOSes is that they report that a removable disk(such as a USB drive)
;has INT13h extensions but will not allow you to use them on these types of drives.
	test cx,1							    ;It is nescisarry to test Bit 0 of CX to verify that the BIOS
	jz No_Ext							    ;will actually allow Extended Read/Write calls on drive.
;Read from the disk using INT13h Extensions
	mov cx,0x05							    ;Attempt to read disk 5 times
	mov eax,DWORD [bp+0x08]					 ;Get the LBA of our VBR
	mov DWORD [DAP_LBA_LSD],eax				 ;and load it into the DAP
Ext_Read:
	mov ah,0x42								;INT 13h AH=42h: Extended Read Sectors From Drive
	mov dl,BYTE [bp]						;DL = Drive Number
	lea si,[DAP_Size]						;DS:SI=>DAP
	int 0x13								;Read disk
	jnc Verify_VBR							;Carry flag clear? Proceed to load VBR
	or cx,cx								;Count = 0?
	jz Disk_Error							;Error
	dec cx									;Count--
	xor ah,ah								;INT 13h AH=00h: Reset Disk Drive
	int 0x13								;Reset Disk
	jmp Ext_Read							;Try Again
;Read from disk using CHS addressing
No_Ext:
	mov di,0x05								;Attempt to read drive 5 times
Read_Disk:
	mov ax,0x0201 	     					;AH = 02h Read Sector AL = 01h One sector
	mov bx,0x7C00	     					;Read to 0000:7C00
	mov dl,[bp]     						;DL = Drive number
	mov dh,[bp+0x01]     					;DH = Head Number
	mov cl,[bp+0x02]						;CL = Bits 0-5 make up Sector Number 
	mov ch,[bp+0x03]						;CH = Along with bits 6-7 of CL makes up the Cylinder
	int 0x13								;INT13h AH=02h: Read sectors
	jnc Verify_VBR							;Carry flag clear? Continue to load VBR
	or di,di								;Count = 0?
	jz Disk_Error							;Error
	dec di									;Count--
	xor ah,ah								;INT 13h AH=00h: Reset Disk Drive
	int 0x13								;Reset Disk
	jmp Read_Disk							;Try Again
	
;All that's left is to check the signature and pass control to the VBR
Verify_VBR:
	mov si,0x7DFE							;DS:SI=>Last WORD of VBR
	cmp [si],WORD 0xAA55					;Is it 55 AA?(remember,Little-Endian)
	jne Missing_OS							;Nope, "Missing operating system!"
	mov dl,BYTE [bp]						;DL = Boot disk number
	push cs									;Push Code Segment
	push 0x7C00								;Push VBR entry point
	retf									;Pass control over to the VBR
	
;Error Handler
CPUID_Fail:
	lea si,[CPUID_FAIL]						;"Incompatible CPU, 80486+ required!"
	jmp Print_Error	
Missing_OS:									
	lea si,[MISSING_OS]						;"Missing operating system!"
	jmp Print_Error	
Invalid_Table:
	lea si,[INVALID_TABLE]					;"Invalid partition table!"
	jmp Print_Error
Disk_Error:
	lea si,[DISK_ERROR]						;"Error reading disk!"
Print_Error:
	call PrintS								;Print error string
	lea si,[ERROR_MSG]						;"Halting machine."
	call PrintS								;Print other message						
	cli										;Clear interrupts
	hlt										;Halt CPU
	
;PrintS - Prints a null terninated string
;In : DS:SI => string
PrintS:
	lodsb									;AL = [SI], SI++
	or al,al								;Zero Byte?
	jz PrintS_Return						;Done
	mov ah,0x0E								;INT10h AH=0Eh - Teletype output	
	mov bx,0x0007							;BH = Page BL = Color
	int 0x10								;Print Character
	jmp PrintS								;Loop again	
PrintS_Return:
	ret										;Return to caller	
;***********************************************Data Area************************************************
CPUID_FAIL					db "Incompatible CPU, 80486+ required!",0
MISSING_OS	  				db "Missing operating system!",0
INVALID_TABLE 				db "Invalid partition table!",0		
DISK_ERROR  				db "Error reading disk!",0
ERROR_MSG					db 0x0D,0x0A,"Halting machine",0
;DAP - Disk Address Packet
DAP_Size					db 0x10			;Size of packet, always 16 BYTES
DAP_Reserved				db 0x00			;Reserved
DAP_Sectors					dw 0x0001		;Number of sectors to read, one
DAP_Offset					dw 0x7C00		;Read to 0000:7C00
DAP_Segment 				dw 0x0000		;Segment		
DAP_LBA_LSD					dd 0x00000000	;Low DWORD of LBA	
DAP_LBA_MSD					dd 0x00000000	;High DWORD
times 440-($-$$)			db 0x00			;Make sure code section is 440 bytes in length
Disk_Sig 					dd 0x00000000	;Disk signature, used to track drives 
Reserved 					dw 0x0000		;Reserved	
;********************************************Partition Table*********************************************
;Partition table - Four 16-Byte entries describing the disk partitioning  
PT1_Status					db 0x00			;Drive number/Bootable flag
PT1_First_Head  			db 0x00			;First Head
PT1_First_Sector			db 0x00			;Bits 0-5:First Sector|Bits 6-7 High bits of First Cylinder
PT1_First_Cylinder			db 0x00			;Bits 0-7 Low bits of First Cylinder
PT1_Part_Type				db 0x00			;Partition Type
PT1_Last_Head	  			db 0x00			;Last Head 
PT1_Last_Sector				db 0x00			;Bits 0-5:Last Sector|Bits 6-7 High bits of Last Cylinder
PT1_Last_Cylinder			db 0x00			;Bits 0-7 Low bits of Last Cylinder
PT1_First_LBA				dd 0x00000000	;Starting LBA of Partition
PT1_Total_Sectors			dd 0x00000000	;Total Sectors in Partition
PT2_Status					db 0x00
PT2_First_Head  			db 0x00
PT2_First_Sector			db 0x00
PT2_First_Cylinder			db 0x00
PT2_Part_Type				db 0x00
PT2_Last_Head	  			db 0x00
PT2_Last_Sector				db 0x00
PT2_Last_Cylinder			db 0x00
PT2_First_LBA				dd 0x00000000
PT2_Total_Sectors			dd 0x00000000
PT3_Status					db 0x00
PT3_First_Head  			db 0x00
PT3_First_Sector			db 0x00
PT3_First_Cylinder			db 0x00
PT3_Part_Type				db 0x00
PT3_Last_Head	  			db 0x00
PT3_Last_Sector				db 0x00
PT3_Last_Cylinder			db 0x00
PT3_First_LBA				dd 0x00000000
PT3_Total_Sectors			dd 0x00000000
PT4_Status					db 0x00
PT4_First_Head  			db 0x00
PT4_First_Sector			db 0x00
PT4_First_Cylinder			db 0x00
PT4_Part_Type				db 0x00
PT4_Last_Head	  			db 0x00
PT4_Last_Sector				db 0x00
PT4_Last_Cylinder			db 0x00
PT4_First_LBA				dd 0x00000000
PT4_Total_Sectors			dd 0x00000000
							db 0x55			;Indicates that this is a Boot sector
							db 0xAA
;---------------------------------------------------------------------------------------------------------------------------------;
