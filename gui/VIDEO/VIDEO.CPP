//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Mar 28,2009
//    Module Name               : VIDEO.CPP
//    Module Funciton           : 
//                                Basic video functions are implemented in 
//                                this file.
//                                
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __KAPI_H__
#include "..\INCLUDE\\KAPI.H"
#endif

#ifndef __VESA_H__
#include "..\INCLUDE\VESA.H"
#endif

#ifndef __VIDEO_H__
#include "..\INCLUDE\VIDEO.H"
#endif

#ifndef __GUISHELL_H__
#include "..\INCLUDE\GUISHELL.H"
#endif

//Local array used to determine the VBE mode's information
//according to the mode number.
static struct __VBE_MODE{
	WORD ModeNum;
	WORD Width;
	WORD Height;
	WORD ClrBit;
}ModeArray[] = {
	{0x4113,800,600,16},
	{0x4116,1024,768,16},
	{0x4118,1024,768,32},
	{0x4110,640,480,16},
	{0x4105,1024,768,8},
	{0x4103,800,600,8},
	{0x4101,640,480,8},
	{0,0,0,0}    //Indicate the end of this array.
};

//clear the screen.
static VOID ClearScreen(__VIDEO* pVideo)
{
	DWORD  dwScreenSize = pVideo->dwScreenWidth * pVideo->dwScreenHeight;
	DWORD* pVideoBase   = (DWORD*)pVideo->pBaseAddress;
	for(DWORD i = 0;i < dwScreenSize;i ++)
	{
		*pVideoBase = 0x00C0C0C0;
		pVideoBase ++;
	}
}

//Initialize routine of video object.
static BOOL Initialize(__VIDEO* pVideo)
{
	__VBE_INFO* pVbeInfo        = (__VBE_INFO*)VBE_INFO_START;
	LPVOID      pBaseAddr       = NULL;
	LPVOID      pPhysAddr       = NULL;
	WORD        ModeNum         = 0;
	DWORD*      pDisplayMem     = NULL;
	BOOL        bResult         = FALSE;
	int         i               = 0;

	if(!pVbeInfo->IfInGraphic)  //Not enter graphic mode.
	{
		goto __TERMINAL;
	}

	//Try to reserve the virtual address space for video's display memory.
	pPhysAddr = (LPVOID)pVbeInfo->ModeInfo.physbaseptr;
	pBaseAddr = VirtualAlloc(pPhysAddr,
		DISPLAY_MEMORY_LENGTH,
		VIRTUAL_AREA_ALLOCATE_IO,  //Allocate flags.
		VIRTUAL_AREA_ACCESS_RW,  //Access flats.
		"VIDEO");
	if(pBaseAddr != (LPVOID)pVbeInfo->ModeInfo.physbaseptr)  //In normal case,this can not occur.
	{
		goto __TERMINAL;
	}
	ModeNum = pVbeInfo->GraphicNum;
	while(ModeArray[i].ModeNum)  //Search the mode array to get mode related information.
	{
		if(ModeNum == ModeArray[i].ModeNum)  //Found,break.
		{
			break;
		}
		i += 1;
	}
	if(ModeArray[i].ModeNum == 0) //Can not find a match one,it is impossible in normal case.
	{
		goto __TERMINAL;
	}
	if(ModeArray[i].ModeNum != 0x4118)  //Only 1024 * 768 * 32 is supported.
	{
		goto __TERMINAL;
	}
	pVideo->dwScreenHeight = ModeArray[i].Height;
	pVideo->dwScreenWidth  = ModeArray[i].Width;
	pVideo->BitsPerPixel   = ModeArray[i].ClrBit;
	pVideo->pBaseAddress   = pBaseAddr;
	pVideo->dwMemLength    = DISPLAY_MEMORY_LENGTH;
	//Now clear the screen.
	ClearScreen(pVideo);
	bResult = TRUE;

__TERMINAL:
	if(!bResult)  //Failed to initialize video object,release all resources.
	{
		if(pBaseAddr)
		{
			VirtualFree(pBaseAddr);
		}
	}
	return bResult;
}

//Uninitialize routine of Video.
static VOID Uninitialize(__VIDEO* pVideo)
{
	if(NULL == pVideo)
	{
		return;
	}
	//Release all sources allocated for Video object.
	if(pVideo->pBaseAddress)
	{
		VirtualFree(pVideo->pBaseAddress);
	}
	return;
}

//--------------------- Interface offered by Video object ----------------

//Converts the mouse dimension into screen dimension.
VOID MouseToScreen(__VIDEO* pVideo,int x,int y,int* px,int* py)
{
	if((NULL == pVideo) || (NULL == px) || (NULL == py))
	{
		return;
	}
	*px = (x * pVideo->dwScreenWidth)  / MOUSE_DIM_X;
	*py = (y * pVideo->dwScreenHeight) / MOUSE_DIM_Y;
}

//Draw one pixel in screen.
VOID DrawPixel(__VIDEO* pVideo,int x,int y,__COLOR color)
{
	CHAR*     pAddress = NULL;

	if(NULL == pVideo)
	{
		return;
	}
	pAddress =  (CHAR*)pVideo->pBaseAddress;
	pAddress += ((y * pVideo->dwScreenWidth) + x) * pVideo->BitsPerPixel / 8;
	*(DWORD*)pAddress = color;
}

//Get one pixel's color.
__COLOR GetPixel(__VIDEO* pVideo,int x,int y)
{
	CHAR* pAddress = NULL;

	if(NULL == pVideo)
	{
		return 0;
	}
	pAddress =  (CHAR*)pVideo->pBaseAddress;
	pAddress += ((y * pVideo->dwScreenWidth) + x) * pVideo->BitsPerPixel / 8;
	return *(__COLOR*)pAddress;
}
 
//Swap 2 integer's value.
inline void swap_int(int *a, int *b) {
    *a ^= *b;
    *b ^= *a;
    *a ^= *b;
}
 
#define abs(x) ((x) < 0 ? -(x) : (x))
//Draw line.
//This routine is copied from the book "Theory of computer graphic",written
//by Donald Hearn and M.Pauline Baker.
VOID DrawLine(__VIDEO* pVideo,int x1,int y1,int x2,int y2,__COLOR c)
{
    int dx = abs(x2 - x1),
        dy = abs(y2 - y1),
        yy = 0;
	//int dx,dy,yy;
	//dx = x2 > x1 ? (x2 - x1 + 1) : (x1 - x2 + 1);
	//dy = y2 > y1 ? (y2 - y1 + 1) : (y1 - y2 + 1);
	//yy = 0;
 
    if (dx < dy) {
        yy = 1;
        swap_int(&x1, &y1);
        swap_int(&x2, &y2);
        swap_int(&dx, &dy);
    }
 
    int ix = (x2 - x1) > 0 ? 1 : -1,
         iy = (y2 - y1) > 0 ? 1 : -1,
         cx = x1,
         cy = y1,
         n2dy = dy * 2,
         n2dydx = (dy - dx) * 2,
         d = dy * 2 - dx;
 
	DrawPixel(pVideo,x1,y1,c);  //Draw first dot.
    if (yy) { // 如果直线与 x 轴的夹角大于 45 度
        //while (cx != x2) {
		do{
            if (d < 0) {
                d += n2dy;
            } else {
                cy += iy;
                d += n2dydx;
            }
            //pDC->SetPixel(cy,cx,c);
            DrawPixel(pVideo,cy,cx,c);
            cx += ix;
        //}
		}while(cx != x2);
    } else { // 如果直线与 x 轴的夹角小于 45 度
        //while (cx != x2) {
		do{
            if (d < 0) {
                d += n2dy;
            } else {
                cy += iy;
                d += n2dydx;
            }
            //pDC->SetPixel(cx,cy,c);
            DrawPixel(pVideo,cx,cy,c);
            cx += ix;
        }while(cx != x2);
    }
	DrawPixel(pVideo,x2,y2,c);  //Draw last dot.
}

//Draw Rectangle.
VOID DrawRectangle(__VIDEO* pVideo,int x1,int y1,int x2,int y2,__COLOR lineClr,
                                        BOOL bFill,__COLOR fillClr)
{
         /*
         DrawLine(pVideo,x1,y1,x2,y1,lineclr);
         DrawLine(pVideo,x2,y1,x2,y2,lineclr);
         DrawLine(pVideo,x2,y2,x1,y2,lineclr);
         DrawLine(pVideo,x1,y2,x1,y1,lineclr);
 
         if(bFill)  //Should fill this rectangle.
         {
                   int ystart = (y1 < y2) ? y1 : y2;
                   int yend   = (y1 < y2) ? y2 : y1;
                   //if(ystart == yend)
                   if(yend - ystart < 2)  //
                   {
                            return;
                   }
                   ystart += 1;  //Skip the uper border line.
                   //yend   -= 1;  //Skip the down border line.
                   for(ystart;ystart < yend;ystart ++)
                   {
                            DrawLine(pVideo,x1 + 1,ystart,x2 - 1,ystart,fillclr);
                   }
         }*/
         //The following code is for debugging.
         int xstart,xend;
         int ystart,yend;
         int i,j;
 
         xstart = (x1 < x2) ? x1 : x2;
         xend   = (x1 < x2) ? x2 : x1;
         ystart = (y1 < y2) ? y1 : y2;
         yend   = (y1 < y2) ? y2 : y1;
 
         //Draw edge lines first.
         for(i = xstart;i <= xend;i ++)
         {
                   DrawPixel(pVideo,i,ystart,lineClr);
                   DrawPixel(pVideo,i,yend,lineClr);
         }
         for(i = ystart;i <= yend;i ++)
         {
                   DrawPixel(pVideo,xstart,i,lineClr);
                   DrawPixel(pVideo,xend,i,lineClr);
         }
         if(bFill)
         {
                   for(i = ystart + 1;i < yend;i ++)
                   {
                            for(j = xstart + 1;j < xend;j ++)
                            {
                                     DrawPixel(pVideo,j,i,fillClr);
                            }
                   }
         }
}

//A local helper used to auxiliate the circling routine.
static void plot_circle_point(__VIDEO* pVideo,int xc,int yc,int x,int y,__COLOR color)
{
	//pDC->SetPixel(xc + x,yc + y,color);
	DrawPixel(pVideo,xc + x,yc + y,color);
 	DrawPixel(pVideo,xc - x,yc + y,color);
 	DrawPixel(pVideo,xc + x,yc - y,color);
 	DrawPixel(pVideo,xc - x,yc - y,color);
 	DrawPixel(pVideo,xc + y,yc + x,color);
 	DrawPixel(pVideo,xc - y,yc + x,color);
 	DrawPixel(pVideo,xc + y,yc - x,color);
 	DrawPixel(pVideo,xc - y,yc - x,color);
}

static void BresenhamCircle(__VIDEO* pVideo,int xc,int yc,int r,__COLOR color)
{
  int x,y,d;
  x=0;y=r;
  d=3-2*r;
  while(x<y)
  {
     plot_circle_point(pVideo,xc,yc,x,y,color);
     if(d<0)
	 {
		 d=d+4*x+6;
	 }
     else
     {
		 d=d+4*(x-y)+10;
		 y--;
     }
     x++;
  }
  if(x==y)
  {
	  plot_circle_point(pVideo,xc,yc,x,y,color);
  }
}

/*下一点的走向 */
#define D_RIGHT 1
#define D_DOWN  2
#define D_RIGHTDOWN 3

//rect为控制矩形
void Bresenham_Ellipse(__VIDEO* pVideo,int x1,int y1,int x2,int y2,__COLOR linecolor)
{
	int xo,yo,a,b;//椭圆 圆点(xo,yo) 长轴:a  短轴b
	int x,y;
	long Delta_D,Delta;
	char Direction;

	//左闭右开，上闭下开
	if((x2-x1)%2==0) x2-=2;
	else x2--;
	if((y2-y1)%2==0) y2-=2;
	else y2--;
	xo=(x2-x1)/2;
	yo=(y2-y1)/2;
	a=xo-x1;
	b=yo-y1;
	if( (a==0)||(b==0) )return;
	if(a<0) a=-a;
	if(b<0) b=-b;
	x=0;y=b;
	Delta_D=(long)b*b*(x+1)*(x+1)+(long)a*a*(y-1)*(y-1)-(long)a*a*b*b;
	while(y>=0)
	{
		DrawPixel(pVideo,x+xo,yo-y,linecolor);
		DrawPixel(pVideo,-x+xo,yo-y,linecolor);
		DrawPixel(pVideo,x+xo,yo+y,linecolor);
		DrawPixel(pVideo,-x+xo,yo+y,linecolor);
		if(Delta_D<0)
		{
			Delta=2L*Delta_D+(long)a*a*(2*y-1);
			if(Delta>0) Direction=D_RIGHTDOWN;
			else	     Direction=D_RIGHT;
		}
		else
		if(Delta_D>0)
		{
			Delta=(-2L*Delta_D)+(long)b*b*(2*x+1);
			if(Delta>=0) Direction=D_RIGHTDOWN;
			else  Direction=D_DOWN;
		}
		else  /*Delta_D==0*/
		Direction=D_RIGHTDOWN;

		switch(Direction)
		{
		case D_RIGHTDOWN:
			x++;
			y--;
			Delta_D+=(long)b*b*(2*x+1)+(long)a*a*(-2*y+1);
			break;
		case D_RIGHT:
			x++;
			Delta_D+=(long)b*b*(2*x+1);
			break;
		case D_DOWN:
			y--;
			Delta_D+=(long)a*a*(-2*y+1);
			break;
		default: break;
		}
	}
}

//Draw circle using Bresenham algorithm.
static inline void _draw_circle_8(__VIDEO* img, int xc, int yc, int x, int y,__COLOR c)
{    
    // 参数 c 为颜色值    
    DrawPixel(img, xc + x, yc + y, c);    
    DrawPixel(img, xc - x, yc + y, c);    
    DrawPixel(img, xc + x, yc - y, c);   
    DrawPixel(img, xc - x, yc - y, c);    
    DrawPixel(img, xc + y, yc + x, c);   
    DrawPixel(img, xc - y, yc + x, c);    
    DrawPixel(img, xc + y, yc - x, c);    
    DrawPixel(img, xc - y, yc - x, c);    
}       
  
//Bresenham's circle algorithm    
static void draw_circle(__VIDEO* img, int xc, int yc, int r, int fill,__COLOR c)
{    
    // (xc, yc) 为圆心，r 为半径    
    // fill 为是否填充    
    // c 为颜色值       
    // 如果圆在图片可见区域外，直接退出    
    
    int x = 0, y = r, yi, d;    
        d = 3 - 2 * r;    
  
    if(fill)  
    {    
        // 如果填充（画实心圆）    
        while(x <= y)   
        {    
            for(yi = x; yi <= y; yi ++)    
            {  
                _draw_circle_8(img, xc, yc, x, yi, c);    
            }  
            if(d < 0)   
            {    
                d = d + 4 * x + 6;    
            }   
            else   
            {    
                d = d + 4 * (x - y) + 10;    
                y --;    
            }    
  
            x++;    
        }    
    }   
    else   
    {    
        // 如果不填充（画空心圆）    
        while (x <= y)   
        {    
            _draw_circle_8(img, xc, yc, x, y, c);    
  
            if(d < 0)   
            {    
                d = d + 4 * x + 6;    
            }   
            else   
            {    
                d = d + 4 * (x - y) + 10;    
                y --;    
            }    
  
            x ++;    
        }    
    }    
}

//Draw a circle according to the given position and radius,fill it using
//the specified color if necessary.It calls Bresenham circle drawing algorithm
//directly.
void DrawCircle(__VIDEO* pVideo,int xc,int yc,int r,__COLOR color,BOOL bFill)
{
	draw_circle(pVideo,xc,yc,r,bFill,color);
}

//------------------------------------------------------------------------
//Declaration of global video object.
__VIDEO Video = {
	1024,       //dwScreenWidth.
	768,        //dwScreenHeight.
	32,         //BitsPerPixel.
	NULL,       //pBaseAddress.
	0,          //dwMemLength.
	Initialize, //Initialize routine.
	Uninitialize,  //Uninitialize.
	DrawPixel,     //DrawPixel.
	GetPixel,      //GetPixel.
	DrawLine,      //DrawLine.
	DrawRectangle, //DrawRectangle.
	NULL,   //DrawEllipse.
	DrawCircle,    //DrawCircle.
	MouseToScreen, //MouseToScreen.
};
