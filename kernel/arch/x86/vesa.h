//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Mar 28,2009
//    Module Name               : VESA.H
//    Module Funciton           : 
//                                Declares VESA standard related structures
//                                and contants,which maybe used by all modules
//                                in GUI module.
//                                
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __VESA_H__
#define __VESA_H__

#include "TYPES.H"

#pragma pack(push,1)

//VBE controller information.
typedef struct tagVBE_CONTROLLER_INFO {
	char  vbesignature[4];   // VBE Signature
	WORD  vbeversion;        // VBE Version
	DWORD oemstringptr;      // Pointer to OEM String
	BYTE  capabilities[4];   // Capabilities of graphics cont.
	DWORD videomodeptr;      // Pointer to Video Mode List
	WORD  totalmemory;       // number of 64Kb memory blocks
	WORD  oemsoftwarerev;    // VBE implementation Software revision
	DWORD oemvendornameptr;  // Pointer to Vendor Name String
	DWORD oemproductnameptr; // Pointer to Product Name String
	DWORD oemproductrevptr;  // Pointer to Product Revision String
	char  reserved[222];     // Reserved for VBE implementation scratch area
	char  oemdata[256];      // Data Area for OEM Strings
}__VBE_CONTROLLER_INFO;

//VBE mode specific information.
typedef struct tagVBE_MODE_INFO {
	// Mandatory information for all VBE revision
	WORD  modeattributes;     // Mode attributes
	BYTE  winaattributes;     // Window A attributes
	BYTE  winbattributes;     // Window B attributes
	WORD  wingranularity;     // Window granularity
	WORD  winsize;    	     // Window size
	WORD  winasegment;        // Window A start segment
	WORD  winbsegment;        // Window B start segment
	DWORD winfuncptr;         // pointer to window function
	WORD  bytesperscanline;   // Bytes per scan line

	// Mandatory information for VBE 1.2 and above
	WORD  xresolution;         // Horizontal resolution in pixel or chars
	WORD  yresolution;         // Vertical resolution in pixel or chars
	BYTE  xcharsize;           // Character cell width in pixel
	BYTE  ycharsize;           // Character cell height in pixel
	BYTE  numberofplanes;      // Number of memory planes
	BYTE  bitsperpixel;        // Bits per pixel
	BYTE  numberofbanks;       // Number of banks
	BYTE  memorymodel;         // Memory model type
	BYTE  banksize;            // Bank size in KB
	BYTE  numberofimagepages;  // Number of images
	BYTE  reserved1;           // Reserved for page function

	// Direct Color fields (required for direct/6 and YUV/7 memory models)
	BYTE  redmasksize;         // Size of direct color red mask in bits
	BYTE  redfieldposition;    // Bit position of lsb of red bask
	BYTE  greenmasksize;       // Size of direct color green mask in bits
	BYTE  greenfieldposition;  // Bit position of lsb of green bask
	BYTE  bluemasksize;        // Size of direct color blue mask in bits
	BYTE  bluefieldposition;   // Bit position of lsb of blue bask
	BYTE  rsvdmasksize;        // Size of direct color reserved mask in bits
	BYTE  rsvdfieldposition;   // Bit position of lsb of reserved bask
	BYTE  directcolormodeinfo; // Direct color mode attributes

	// Mandatory information for VBE 2.0 and above
	DWORD physbaseptr;         // Physical address for flat frame buffer
	DWORD offscreenmemoffset;  // Pointer to start of off screen memory
	WORD  offscreenmemsize;    // Amount of off screen memory in 1Kb units
	char  reserved2[206];      // Remainder of ModeInfoBlock
}__VBE_MODE_INFO;

/*
 * A block of memory describes the VBE related 
 * information:
 *   IfInGraphic: set to 1 when enter graphic mode;
 *   GraphicNum: Graphic mode's number;
 *   ModeInfo: VBE mode info, loaded by realinit.bin;
 *   ControllerInfo: Video controller's info.
 * This block of memory is fixed at VBE_INFO_START,
 * which is defined as 0xE000 under x86.
 */
typedef struct tagVBE_INFO {
	WORD                  IfInGraphic;
	WORD                  GraphicNum;
	__VBE_MODE_INFO       ModeInfo;
	__VBE_CONTROLLER_INFO ControllerInfo;
}__VBE_INFO;

/* 
 * Memory address of VBE info structure.
 * realinit.bin initializes the memory
 * block's content when switch_to_graphic
 * system call is invoked.
 */
#define VBE_INFO_START 0x0000E000

#pragma pack(pop)

/* Switch display mode to text. */
void SwitchToText();

/* Switch display mode into Graphic. */
LPVOID SwitchToGraphic(__VBE_INFO* pVbeInfo);

#endif //__VESA_H__
