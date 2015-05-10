//***********************************************************************/
//    Author                    : zhangbing(mail:550695@qq.com)
//    Original Date             : Mar 28,2009
//    Module Name               : gdi.h
//    Module Funciton           : 
//                                Declares gdi object and related structures,
//                                constants and global routines.
//                                
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __GDI_H__
#define __GDI_H__

typedef DWORD   COLORREF;
typedef HANDLE  HDC; 
typedef HANDLE  HRGN;
typedef HANDLE HWND;

typedef struct tagRECT
{
    INT    left;
    INT    top;
    INT    right;
    INT    bottom;
} RECT, *PRECT;

typedef struct tagPOINT
{
    INT  x;
    INT  y;
} POINT, *PPOINT;


#define DCT_SCREEN    0x00000001L
#define DCT_PRINT      0x00000002L
#define DCT_MEMORY   0x00000004L

/*
* DC.iBkMode 属性
*/
#define TRANSPARENT  1  /* 针对文字输出时,文字是以透明的背景方式还是以DC中设定的背景颜色 */
#define OPAQUE       2  /* 以透明背景颜色输出 */



/*
 * 设备类型 GetDCEx函数中参数dcx_flags常量
 */
#define DCX_WINDOW              0x00000001L
#define DCX_CLIPCHILDREN        0x00000002L
#define DCX_CLIPSIBLINGS        0x00000004L
#define DCX_EXCLUDERGN          0x00000008L
#define DCX_INTERSECTRGN        0x00000010L


/*
* DC.uTextAlign 文字对齐方式(标志)
*/
#define TA_NOUPDATECP   0
#define TA_UPDATECP     1

#define TA_LEFT         0
#define TA_RIGHT        2
#define TA_CENTER       6

#define TA_TOP          0
#define TA_BOTTOM       8
#define TA_BASELINE     24
#define TA_RTLREADING   256
#define TA_MASK         (TA_BASELINE+TA_CENTER+TA_UPDATECP+TA_RTLREADING)


/* 库存逻辑对象 */
#define WHITE_BRUSH         0
#define LTGRAY_BRUSH        1
#define GRAY_BRUSH          2
#define DKGRAY_BRUSH        3
#define BLACK_BRUSH         4
#define NULL_BRUSH          5
#define HOLLOW_BRUSH        NULL_BRUSH

#define WHITE_PEN           6
#define BLACK_PEN           7
#define NULL_PEN            8


typedef struct tagDC {
    INT         iType;      /* the DC's type, screen DC | memory DC | print DC*/
    HANDLE   hDevice;       /* 物理输出设备句柄 */
    HWND     hWnd;          /* DC所属的窗口 */

    DWORD    dwFlags;       /* 窗口设备裁剪标记 */

	
    INT      iBkMode;       /* 背景模式 */
    UINT     uTextAlign;    /* 文字对齐方式 */
    COLORREF bkColor;       /* 文字输出时的背景颜色 */
    COLORREF textColor;     /* 文字的颜色 */
    INT      iDrawMode;     /* rop2 覆盖绘图模式 */
    POINT    pt;            /* 当前的画笔坐标*/


    HANDLE   pPen;          /* 当前的画笔句柄 */
    HANDLE   pBrush;        /* 当前的画刷句柄 */
    HANDLE   pFont;         /* 当前的字体句柄 */
    HANDLE   pBitmap;       /* 当前的位图 */
    HRGN     hrgn;          /* 用户指定的DC裁剪域 */
}DC, *PDC;




#endif


