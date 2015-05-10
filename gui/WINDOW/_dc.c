//***********************************************************************/
//    Author                    : zhangbing(mail:550695@qq.com)
//    Original Date             : April,21 2009
//    Module Name               : dc.c
//    Module Funciton           : 
//                                The implementation of dc object.
//                                This is the kernel object in GUI module of
//                                Hello China.
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __KAPI_H__
#include "..\INCLUDE\KAPI.H"
#endif

#ifndef __VESA_H__
#include "..\INCLUDE\VESA.H"
#endif

#ifndef __VIDEO_H__
#include "..\INCLUDE\VIDEO.H"
#endif

#ifndef __GLOBAL_H__
#include "..\INCLUDE\GLOBAL.H"
#endif

#ifndef __WNDMGR_H__
#include "..\INCLUDE\WNDMGR.H"
#endif

#include "..\include\gdi.h"

BOOL CheckWndStyle(HWND hWnd,DWORD dwWndStyle)
{
    __WINDOW *pwnd = (__WINDOW *)hWnd;

    return ((pwnd->dwWndStyle & dwWndStyle) == dwWndStyle);
}

BOOL IsVisible(HWND hWnd)
{
#if 0
    if(CheckStyle(HWND, WS_VISIBLE))  // now no WS_VISIBLE style!
        return TRUE;
    return FALSE;
#endif
}

static void InitDC(HWND hWnd,  PDC pDc, HRGN hrgnClip, DWORD dcxFlags, INT iDcType)
{
    __WINDOW *pwnd = (__WINDOW *)hWnd;

    pDc->iType = iDcType;
    /* 建立DC和实际输出(也可能时内存)的联系 */

    if(iDcType == DCT_SCREEN)
         pDc->hDevice = &Video;
    else
         pDc->hDevice =  NULL;
    pDc->hWnd = hWnd;
    pwnd->hDC = (HDC)pDc;
    pDc->dwFlags = dcxFlags;

    /* 初始化DC的却省值 */
    pDc->iBkMode = OPAQUE;
    pDc->uTextAlign = TA_LEFT | TA_TOP | TA_NOUPDATECP;
    pDc->bkColor = RGB(255, 255, 255);  /* 却省时文字的输出是白色背景色*/
    pDc->textColor = RGB(0, 0, 0);      /* 却省时文字的输出是黑色文字色*/

#ifdef ROP
    pDc->iDrawMode = R2_COPYPEN;
#endif
    pDc->pt.x = 0;
    pDc->pt.y = 0;

#if 0

    pDc->pPen = (PPENOBJ)GetStockObject(BLACK_PEN);
    pDc->pBrush = (PBRUSHOBJ)GetStockObject(WHITE_BRUSH);
    pDc->pFont = (FONTOBJ *)GetStockObject(SYSTEM_FONT);


    pDc->hrgn = CreateEmptyRgn();
    if(!hrgn)
        return NULL;

    if(CalcWindowVisRgn(hWnd, hrgn, dcxFlags) == FALSE)
    {
        DeleteObject((HRGN)hrgn);
        pDc->pPen = NULL;
        pDc->pBrush = NULL;
        GdItemFree(pDc);
        return NULL;
    }

    /* 如果参数hrgnClip不为空并且窗口存在可视区域则对计算后的visrgn做加入/减去操作 */

    /* 执行求交操作,hrgnClip必须是一个region而不是一个HRGN_FULL标志 */
    if (hrgnClip != NULL && hrgnClip > HRGN_FULL)
    {
        if(dcxFlags & DCX_INTERSECTRGN)
        {
            IntersectRgn(hrgn, hrgn, hrgnClip);
        }
        /* 执行排除操作 */
        else if(dcxFlags & DCX_EXCLUDERGN)
        {
            SubtractRgn(hrgn, hrgn, hrgnClip);
        }
    }
#endif
}

/*==========================================================================
* HDC GetDCEx(HWND hWnd, HRGN hrgnClip, DWORD flags)
* 功能:返回某个窗口的DC，该函数提供了对窗口可视区域的附加控制，可以根据参数提供
*      的一些标志信息来返回需要的裁剪控制。
* 参数:
*      hWnd:     待求窗口DC的窗口句柄。
*      hrgnClip  指定可能和设备描述表返回的窗口可视区域相运算的裁剪域,这种运算可
*                能是在可视区域中排除这个hrgnClip也可能是求可视区域中和hrgnClip
*                的交集。
*      dcxFlags 指定设备描述表如何被创建的一些标记。
* ---------------------------------------------------------------------------
* DCX_WINDOW         返回DC包含窗口的非客户区。
* DCX_CLIPSIBLINGS   在窗口hWnd的Z轴之上的所有可见兄弟窗口都要从DC的剪裁区中排除
*
* DCX_CLIPCHILREN    所有可见的子窗口区都要从DC的剪裁区中排除
* DCX_EXCLUDERGN     从窗口的可视区域中排除由hrgnclip指定的区域(VisRgn - hrgnClip)
*
* DCX_INTERSECTRGN   由hrgnclip指定的区域与窗口的可视区域相交(VisRgn ^ hrgnClip)
*
* 返回值：如果成功返回指定窗口的设备描述表句柄,失败返回NULL如果参数hWnd为空则返
*         回桌面的DC.
*
* 历史记录:
* 2009/04/21  创建
===========================================================================*/
HDC GetDCEx(HWND hWnd, HRGN hrgnClip, DWORD dcxFlags)
{
    HRGN  hrgn = NULL;
    PDC pDc = NULL;

    if(hWnd == NULL)
    {
#if 0 // unimplemented
        hWnd = GetDesktopWindow();
#endif
        if(hWnd == NULL)
            return NULL;
    }

    /* 分配内存 */
    pDc = KMemAlloc(sizeof(DC), KMEM_SIZE_TYPE_ANY);
    if(!pDc)
        return NULL;


    /* 首先判断该窗口是否可视,是具备显示 */
    if(!IsVisible(hWnd))
        return NULL;

        dcxFlags &= ~(DCX_CLIPSIBLINGS | DCX_CLIPCHILDREN);

#if 0  // now in wndmgr.h has no WS_CLIPCHILDREN and WS_CLIPSIBLINGS Description
	if (CheckWndStyle(hWnd, WS_CLIPCHILDREN))
		dcxFlags |= DCX_CLIPCHILDREN;

	if (CheckWndStyle(hWnd, WS_CLIPSIBLINGS))
		dcxFlags |= DCX_CLIPSIBLINGS;

        /* 取客户区DC */
        if (!(dcxFlags & DCX_WINDOW))
        {

            /*
            * 窗口最小化则不需要裁剪子窗口.
            */
            if (CheckWndStyle(hWnd, WS_MINIMIZE))
            {
                dcxFlags &= ~DCX_CLIPCHILDREN;

            }

        }
#endif
    InitDC(hWnd, pDc, hrgnClip, dcxFlags,  DCT_SCREEN);

    return (HANDLE)pDc;
}


HDC GetWindowDC(HWND hWnd)
{
    return GetDCEx(hWnd, NULL, DCX_WINDOW);
}


HDC GetDC(HWND hWnd)
{
    return GetDCEx(hWnd, NULL, 0);
}


HDC CreateCompatibleDC(HDC hdc)
{
    PDC pDc = (PDC)hdc;

    return (HANDLE)pDc;
}

int ReleaseDC(HWND hWnd, HDC hdc)
{
    __WINDOW *pwnd = (__WINDOW *)hWnd;
    PDC pDc = (PDC)hdc;
	
    /* 不用ReleaseDC释放内存DC而用DeleteDc来释放 */
    if(!pDc || (pDc->iType == DCT_MEMORY))
        return 0;

#if 0 // not support now
    DeleteObject((HPEN)pDc->pPen);
    DeleteObject((HBRUSH)pDc->pBrush);
    DeleteObject((HBRUSH)pDc->pFont);
    DeleteObject((HRGN)pDc->hrgn);
    DeleteObject((HRGN)pDc->pBitmap);
#endif
    // process Device context of this window's field.
    pwnd->hDC = NULL;
    KMemFree(pDc, KMEM_SIZE_TYPE_ANY, sizeof(DC));
    return 1;
}


BOOL DeleteDC(HDC hdc)
{
    PDC pDc = (PDC)hdc;
    /* DeleteDC只用来释放内存DC */
    if(!pDc || !(pDc->iType & DCT_MEMORY))
        return FALSE;

    /* 释放硬件申请的内存 */

    pDc->iType  = DCT_SCREEN;
    return ReleaseDC(NULL, hdc);
}


