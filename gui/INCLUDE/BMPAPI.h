//***********************************************************************/
//    Author                    : sh
//    Original Date             : 14,04 2009
//    Module Name               : BMPAPI.h
//    Module Funciton           : Declare the BMP struct  
//								  Declare 5 functions define in BMPAPI.cpp
//
//    Last modified Author      : sh
//    Last modified Date        : 18,04 2009
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              : 
//***********************************************************************/

#ifndef __BMPAPI_H_
#define __BMPAPI_H_

#ifndef __VESA_H__
#include "..\INCLUDE\VESA.H"
#endif

//Modified by garry at 2009.04.22.
#ifndef __VIDEO_H__
#include "..\INCLUDE\VIDEO.H"
#endif


//#define DATA_TYPE_INT


#ifdef DATA_TYPE_INT
typedef int IMGDATATYPE; 
#else
typedef BYTE IMGDATATYPE; 
#endif

//Revised by garry in 2009.04.22,replace the float point number 0.5 by
//multiplying 2 and then divided by 2.
#define ROUND(a)		(((a) < 0) ? (int)(((a) * 2 - 1)/2) : (int)(((a) * 2 + 1)/2)) 
#define BOUND(a, b, c)	((a) < (b) ? (b) : ((a) > (c) ? (c) : (a)))
#define  BytesPerLine(BmWidth, BmBitCnt)	(((BmBitCnt) * (BmWidth) + 31) / 32 * 4)
//#define RGB(r,g,b) ((((DWORD)r) << 16) + (((DWORD)g) << 8) + ((DWORD)b))


typedef struct 
{
	unsigned short	type;
	unsigned long	size;
	unsigned short	reserved1;
	unsigned short	reserved2;
	unsigned long	OffBits;
} BMPFILEHEADER; // 14 bytes   note: sizeof(BMPFILEHEADER) == 16  (padding 2 bytes) 

/************************************************************************************************
   
   BMP文件头说明(一共14个字节)：

   type：			数据地址为0，它用于标志文件格式，其值为0x4d42(十进制19778)(即字符串“BM”)，
					表示该图像文件为BMP文件。
   size：			数据地址为2，它指定这个BMP文件大小，以字节为单位。
   reservedl：		数据地址为6，是BMP文件的保留字，其值必须为0。 
   reserved2：		数据地址为8，是BMP文件的保留字，其值必须为0。 
   OffBits：		数据地址为10，它为从文件头到实际的位图数据的偏移字节数，即前三部分(BMP文
					件头、BMP信息头、调色板)的长度之和，以字节为单位。

 ************************************************************************************************/


typedef struct 
{
	unsigned long	size;
	long			width;
	long			height;
	unsigned short	planes;
	unsigned short	BitCount;
	unsigned long	compression;
	unsigned long	SizeImage;
	long			XPelsPerMeter;
	long			YPelsPerMeter;
	unsigned long	ClrUsed;
	unsigned long	ClrImportant;
} BMPINFOHEADER; // 40 bytes

/************************************************************************************************
 
   BMP信息头说明(一共40个字节)：

   size：			数据地址为14，它指定位图信息头的长度，其值为40。
   width：			数据地址为18，它指定图象的宽度，单位是像素。  
　 height：			数据地址为22，它指定图象的高度，单位是像素。若height的取值为正数， 则表明
					位图为bottom—up类型的DIB位图，而且位图原点为左下角。若height的取值为 负数，
					则表明位图为top—down类型的DIB位图，而且位图原点为左上角。在一般位图定义中，
					height字段的取值为正数。
   planes：			数据地址为26，它代表目标设备的级别，必须为1。
   BitCount：		数据地址为28，它确定每个像素所需要的位数。值为1表示黑白二色图，值4为表示16
					色图，值8为表示256色图，值为24表示真彩色图。
   compression：	数据地址为30，它代表bottom—up类型位图的压缩类型(注意：bottom—down类型位图
					不能进行压缩处理)，其可能取值及其含义分别为：若该字段的取值为BI—RGB，则表示
					文件内的图像数据没有经过压缩处理；若该字段的取值为BI—RLE8，则表示所压缩的图
					像数据是256色，采用的压缩方法是RLE8；若该字段的取值为BI—RLE4，则表示所压缩
					的图像数据是16色，采用的压缩方法是RLE4；若该字段的取值为BI—BITFIELDS，则表
					明图像文件内的数据没有经过压缩处理，而且颜色表由分别表示每个像素点的红、绿、
					蓝三原色的双字组成。BMP文件格式在处理单色或者真彩色图像时，不论图像数据多么
					庞大，都不对图像数据进行任何压缩处理。
   Sizelmage：		数据地址为34，它给出该BMP内图像数据占用的空间大小。若图像文件描述BI—RGB位图，
					则该字段的值必须设置为0。
　 XPelsPerMeter：	数据地址为38，以每米像素数为单位，给出位图目的设备水平以及垂直方向的分辨率。
					应用程序可以根据XPelsPerMeter字段的值，从源位图组中选择与当前设备特点最匹配
					的位图。
　 YPelsPerMeter：	数据地址为42，以每米像素数为单位，给出位图目的设备水平以及垂直方向的分辨率。
					应用程序可以根据YPelsPerMeter字段的值，从源位图组中选择与当前设备特点最匹配
					的位图。
   ClrUsed：		数据地址为46，它给出位图实际使用的颜色表中的颜色变址数。如果该字段的取值为0，
					则代表本位图用到的颜色为2的BitCount次幂，其中BitCount字段的取值与compression
					所指定的压缩方法相关。例如：如果图像为16色，而该字段的取值为10，则代表本位图
					共使用了12种颜色；如果该字段的取值非零，而且BitCount字段的取值小于16，则该字
					段指定图像或者设备驱动器存取的实际颜色数。若biBitCount字段的取值大于或者等于
					16，则该字段指定使Window 系统调色板达到最优性能的颜色表大小。
　 Clrlmportant：	数据地址为50，它给出位图显示过程中重要颜色的变址数。若该字段的取值为0，则表示
					所有使用的颜色都是重要颜色。 

 ************************************************************************************************/


typedef struct 
{
	BMPFILEHEADER BmpFileHeader;
	BMPINFOHEADER BmpInfoHeader;
} BMPHEADER;

/************************************************************************************************
   
	 BMP头包括文件头和信息头，一共54个字节。 

 ************************************************************************************************/


typedef struct 
{
	unsigned char	blue;
	unsigned char	green;
	unsigned char	red;
	unsigned char	reserved;
} hcRGBQUAD;

/************************************************************************/
/*	 在此说明，BMP头有些信息并没多大用，以上的结构体只为作标准BMP读写的扩展之用.
	 实际的有用的信息存储在下面的BMPIMAGE中，这样便于操作                                                                      */
/************************************************************************/


typedef struct 
{
	//int CompNum;
	int height;
	int width;
	int BitCount;//位深度
	BYTE *ColorMap;//调色板
	BYTE *ColorIndex;//位深度<=8的BMP的数据
	int IndexNum;//因为位深度<8的BMP索引值不足1byte，所以要保存图像数据字节数
	IMGDATATYPE *DataB, *DataG, *DataR;//用于显示的颜色分量值，bpp<=8时可为NULL
} BMPIMAGE;


//#ifdef __cplusplus
//#if __cplusplus
//extern "C"{
//#endif
//#endif /* __cplusplus */ 
	//remember fopen and fclose to operator FILE *

	/*extern BMPIMAGE *BmpRead(FILE* InBmp);
	extern void BmpWrite(FILE* OutBmp, BMPIMAGE *image);
	extern BMPIMAGE* ImageAlloc(int height, int width);//, int CompNum);
	extern void ImageDealloc(BMPIMAGE* image);	
	extern void BmpShow(__VIDEO *pVideo, int x, int y, BMPIMAGE *image);
	
	//other functions
	extern void Bmp2Txt(BMPIMAGE *image, FILE *OutTxt);
	extern void Get16Num(unsigned char *orin, char *str);*/
	void BmpShowArray(__VIDEO* pVideo, int x, int y, int height, int width, BYTE *DataB, BYTE *DataG, BYTE *DataR);
	//data declare
	extern char NUM16[16];
	extern BYTE DataBlue[196];
	extern BYTE DataGreen[196];
	extern BYTE DataRed[196];

	//extern __VIDEO Video;
//#ifdef __cplusplus
//#if __cplusplus
//}
//#endif
//#endif /* __cplusplus */ 



#endif