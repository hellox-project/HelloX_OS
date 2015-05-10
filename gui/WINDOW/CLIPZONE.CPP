//***********************************************************************/
//    Author                    : Garry
//    Original Date             : May 01, 2009
//    Module Name               : CLIPZONE.CPP
//    Module Funciton           : 
//                                Clip zone of WINDOW is implementated in this file.
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#include "..\INCLUDE\KAPI.H"
#include "..\INCLUDE\CLIPZONE.H"

//Create one region object and initialize it.
__REGION* CreateRegion()
{
	__REGION* pRegion = (__REGION*)KMemAlloc(sizeof(__REGION),
		KMEM_SIZE_TYPE_ANY);

	if(NULL == pRegion)  //Can allocate memory.
	{
		return NULL;
	}
	//Initialize the region object.
	pRegion->ClipZoneHdr.height = 0;
	pRegion->ClipZoneHdr.width  = 0;
	pRegion->ClipZoneHdr.x      = 0;
	pRegion->ClipZoneHdr.y      = 0;
	pRegion->ClipZoneHdr.pNext = &pRegion->ClipZoneHdr;
	pRegion->ClipZoneHdr.pPrev = &pRegion->ClipZoneHdr;

	return pRegion;
}

//Destroy one region object.
VOID DestroyRegion(__REGION* pRegion)
{
	__CLIPZONE* pNext = NULL;
	__CLIPZONE* pCurr = NULL;

	if(!pRegion)
	{
		return;
	}
	pNext = pCurr = pRegion->ClipZoneHdr.pNext;
	while(pNext != &pRegion->ClipZoneHdr)
	{
		pCurr = pNext;
		pNext = pNext->pNext;
		KMemFree(pCurr,KMEM_SIZE_TYPE_ANY,0);
	}
	KMemFree(pRegion,KMEM_SIZE_TYPE_ANY,0);
}

//Check if a given point is in region.
BOOL PtInRegion(__REGION* pRegion,int x,int y)
{
	__CLIPZONE* pClipZone = NULL;

	if(NULL == pRegion)
	{
		return FALSE;
	}
	pClipZone = pRegion->ClipZoneHdr.pNext;
	while(pClipZone != &pRegion->ClipZoneHdr)
	{
		if((x >= pClipZone->x) && (x <= pClipZone->x + pClipZone->width) &&
		   (y >= pClipZone->y) && (y <= pClipZone->y + pClipZone->height))
		{
			return TRUE;
		}
		pClipZone = pClipZone->pNext;
	}
	//If reach here,means does not fall in region.
	return FALSE;
}

//A local helper routine,check if two rectangle can be merged together.
//If can,then merge them and return TRUE,else return FALSE.
static BOOL Merge(__CLIPZONE* p1,__CLIPZONE* p2)
{
	//p2 is in p1's right.
	if((p1->x + p1->width == p2->x) && (p1->height == p2->height))
	{
		p1->width += p2->width;
		return TRUE;
	}
	//p2 is in p1's left.
	if((p2->x + p2->width == p1->x) && (p2->height == p1->height))
	{
		p1->width += p2->width;
		p1->x = p2->x;
		return TRUE;
	}
	//p2 is in p1's top.
	if((p2->y + p2->height == p1->y) && (p2->width == p1->width))
	{
		p1->y = p2->y;
		p1->height += p2->height;
		return TRUE;
	}
	//p2 is in p1's bottom.
	if((p1->y + p1->height == p2->y) && (p1->width == p2->width))
	{
		p1->height += p2->height;
		return TRUE;
	}
	return FALSE;
}

//Add one clip zone rectangle to region.This routine will check the
//added rect if it can merge with existing.If so,merge them.
VOID AddClipZone(__REGION* pRegion,__CLIPZONE* pClipZone)
{
	__CLIPZONE* pStart = NULL;

	if((NULL == pRegion) || (NULL == pClipZone))
	{
		return;
	}
	pStart = pRegion->ClipZoneHdr.pNext;
	while(pStart != &pRegion->ClipZoneHdr)
	{
		if(Merge(pClipZone,pStart))
		{
			//Delete pStart from list.
			pStart->pPrev->pNext = pStart->pNext;
			pStart->pNext->pPrev = pStart->pPrev;
			KMemFree(pStart,KMEM_SIZE_TYPE_ANY,0);  //Release the merged one.
			AddClipZone(pRegion,pClipZone);
			return;
		}
	}
	//Can not merge,insert into list directly.
	pClipZone->pPrev = &pRegion->ClipZoneHdr;
	pClipZone->pNext = pRegion->ClipZoneHdr.pNext;
	pRegion->ClipZoneHdr.pNext->pPrev = pClipZone;
	pRegion->ClipZoneHdr.pNext = pClipZone;
	return;
}

//Local macro,to check if p2 include p1.
#define CLIPZONE_INCLUDE(p1,p2) ((p2->x <= p1->x) && (p2->y <= p1->y) \
	&& (p2->x + p2->width  >= p1->x + p1->width) \
	&& (p2->y + p2->height >= p1->y + p1->height))

//A local helper routine,minus rectange p2 from p1,the result(maybe several
//rectangles) will be put into pResult list.
