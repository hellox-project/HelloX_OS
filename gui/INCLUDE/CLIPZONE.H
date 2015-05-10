//***********************************************************************/
//    Author                    : Garry
//    Original Date             : May 01, 2009
//    Module Name               : CLIPZONE.H
//    Module Funciton           : 
//                                Clip zone of WINDOW is defined in this file.
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __CLIPZONE_H__
#define __CLIPZONE_H__
#endif

//The definition of CLIPZONE object.
struct __CLIPZONE{
	int x;
	int y;
	int height;
	int width;
	__CLIPZONE* pNext;
	__CLIPZONE* pPrev;
};

//Definition of region object.This object is used to represent window's clipzone.
struct __REGION{
	__CLIPZONE ClipZoneHdr;
};

//Routines used to manipulate region object.
//Create and initialize one region object.
__REGION* CreateRegion();

//Destroy one region object.
VOID DestroyRegion(__REGION* pRegion);

//Check if one point is in a region.
BOOL PtInRegion(__REGION* pRegion,int x,int y);

//Add one clip zone rectangle into region.
VOID AddClipZone(__REGION* pRegion,__CLIPZONE* pClipZone);


