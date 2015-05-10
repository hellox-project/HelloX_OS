//***********************************************************************/
//    Author                    : zato
//    Original Date             : Apr 22,2009
//    Module Name               : WordLib.CPP
//    Module Funciton           : 
//                                Display Chinese word and Westen char in
//                                this file.
//                                
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                
//                                
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

#include "..\INCLUDE\WordLib.H"

VOID DispHZK16(int x, int y, unsigned char *pHZ  , __COLOR color)
{   
	int QM=0;//区码
	int WM=0;//位码
	unsigned char * p;//汉字库的起始地址
	unsigned char * pm=0;//将要显示的字模的其实地址
	unsigned char mat[16][2];//字模数组
	int i,j,k=0;
	p=(unsigned char *)CHNCHAR_LIB_BASE;
	QM=*pHZ-0xA0;
	WM=*(pHZ+1)-0xA0;
	pm=p+(94*(QM-1)+(WM-1))*32;
	for(i=0;i<16;i++)
		for(j=0;j<2;j++)
			mat[i][j]=*(pm+i*2+j);
		
		for(i=0;i<16;i++)
			for(j=0;j<2;j++)
				for(k=0;k<8;k++)
					if(mat[i][j]&(0x80>>k))
						DrawPixel(&Video,x+j*8+k,y+i,color);
					
					
}
VOID DispASC16(int x, int y, unsigned char *pXZ  , __COLOR color)
{   
	
	unsigned char * p;//英文字库的起始地址
	unsigned char * pm=0;//将要显示的字模的起始地址
	unsigned char mat[16];//字模数组
	int i,k=0;
	p=(unsigned char *)ASCII_LIB_BASE;
	//if((*pXZ)>0 && (*pXZ)<0x80)
	{
		pm=(*pXZ)*16+p;
	}
	
	for(i=0;i<16;i++)
		mat[i]=*(pm+i);
	
	for(i=0;i<16;i++)
		for(k=0;k<8;k++)
			if(mat[i]&(0x80>>k))
				DrawPixel(&Video,x+k,y+i,color);
			
			
}

VOID TextOut(int x,int y,char *pStr,__COLOR color)
{
#define WITH_HZK16 16
#define WITH_ASC16 8
	unsigned char *p;
	int x0,y0;
	x0=x;
	y0=y;
	p=(unsigned char*)pStr;
	while(*p!='\0')
	{
		if((*p>=0x80)&&(*(p+1)>=0x80))
		{	
			DispHZK16(x0, y0, (unsigned char *)p , color);
			p+=2;
			x0+=WITH_HZK16;
		}
		else if((*p<0x80)&&(*p>0))
		{
			DispASC16(x0, y0, (unsigned char *)p  , color);
			p++;
			x0+=WITH_ASC16;
		}
		else if(*p>=80 && *(p+1)<0x80)
		{
			p++;
		}
	}
}
