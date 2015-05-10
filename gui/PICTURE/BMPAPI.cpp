//***********************************************************************/
//    Author                    : sh
//    Original Date             : 14,04 2009
//    Module Name               : BMPAPI.cpp
//    Module Funciton           : Read BMP from file, Write BMP from memory to 
//								  file; ImageAlloc allocate mem for BMPIMAGE
//
//    Last modified Author      : sh
//    Last modified Date        : 18,04 2009
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

#include "..\include\BMPAPI.h"


char NUM16[16] = {
	'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};

#if 0
BMPIMAGE *BmpRead(FILE* InBmp)
{
	int n, x, y;
	int count, mask, type, compression;
	int width, height, BitCount, ColorNum;
	int LineBytes, BytesRead, ColorIndex;
	unsigned char r, g, b, byte;
	unsigned char ColorMap[3 * 256];
	unsigned char PadZeros[4], BmpHeader[54];
	BMPIMAGE *image;
	
	//Read 54 bytes to BmpHeader
	fread(BmpHeader, sizeof(unsigned char), 54, InBmp); 
	//little-endian the high bit in high byte
	//type 0 1
	type = (BmpHeader[1] << 8) + BmpHeader[0];
	//width 18 19 20 21 
	width = (BmpHeader[21] << 24) + (BmpHeader[20] << 16) + (BmpHeader[19] << 8) + BmpHeader[18]; 
	//height 22 23 24 25
	height = (BmpHeader[25] << 24) + (BmpHeader[24] << 16) + (BmpHeader[23] << 8) + BmpHeader[22];
	//bitcount 28 29
	BitCount = (BmpHeader[29] << 8) + BmpHeader[28];
	//compression 30 31 32 33
	compression = (BmpHeader[33] << 24) + (BmpHeader[32] << 16) + (BmpHeader[31] << 8) + BmpHeader[30];
	LineBytes = (width * BitCount + 31) / 32 * 4;

	
	/************************************************************************/
	/* assert the image is bmp                                                                     */
	/************************************************************************/
	if (type != ((unsigned short)('M' << 8) | 'B'))
	{
		printf("the file maybe is not a bmp image.\n");
		return NULL;
	}

	if (compression != 0)
	{
		printf("can not read a compressed bmp.\n");
		return NULL;
	}

	if (BitCount > 8 && BitCount != 24)
	{
		printf("can not read a %d bit bmp.\n", BitCount);
		return NULL;
	}
	
	//If the bmp has 24 bpp
	if (BitCount == 24)
	{
		if ((image = ImageAlloc(height, width)) == NULL)
		{
			printf("fail to allocate image.\n");
			return NULL;
		}

		image->ColorIndex = NULL;

		for (y = height - 1; y >= 0; y--) 
		{
			for (x = 0; x < width; x++)    
			{
				//first is blue, then is green, third is red
				//InBmp auto point to next
				fread(&b, sizeof(unsigned char), 1, InBmp);
				fread(&g, sizeof(unsigned char), 1, InBmp);
				fread(&r, sizeof(unsigned char), 1, InBmp);

				image->DataB[y * width + x] = b; 
				image->DataG[y * width + x] = g;
				image->DataR[y * width + x] = r;
			}
			//there are less than 4 bytes to align, read it to PadZeros
			fread(PadZeros, sizeof(unsigned char), (LineBytes - width * 3), InBmp);
		}
	}
	else if (BitCount <= 8)  
	{
		ColorNum = 1 << BitCount;//palate size

		if ((image = ImageAlloc(height, width)) == NULL)
		{
			printf("fail to allocate image.\n");
			return NULL;
		}
		/************************************************************************/
		/* malloc order can't be reverse                                                                     */
		/************************************************************************/
		if((image->ColorMap = (BYTE *)malloc(3 * (1 << BitCount) * sizeof(BYTE))) == NULL){
			return NULL;
		}
		
		//read palate first, sizeof color is 4bytes
		for (n = 0; n < ColorNum; n++) 
		{ 
			//the order is blue, green, red, reserve
			fread(&b, sizeof(unsigned char), 1, InBmp);
			fread(&g, sizeof(unsigned char), 1, InBmp);
			fread(&r, sizeof(unsigned char), 1, InBmp);	
			fread(PadZeros, sizeof(unsigned char), 1, InBmp);	
			
			//palate is small, allocate it stack memory
			ColorMap[n * 3 + 0] = b;
			ColorMap[n * 3 + 1] = g;
			ColorMap[n * 3 + 2] = r;
			image->ColorMap[n * 3 + 0] = b;
			image->ColorMap[n * 3 + 1] = g;
			image->ColorMap[n * 3 + 2] = r;
			
	    }
		
		if((image->ColorIndex = (BYTE*)malloc(height * width * sizeof(BYTE))) == NULL){
			return NULL;
		}

		count = 0;
		BytesRead = 0;
		mask = (1 << BitCount) - 1;

		int ele = 0;

		//BMP的图像存储是倒立的，与正常图像沿y轴对称

		for (y = height - 1; y >= 0; y--) 
		{
			for (x = 0; x < width; x++)    
			{
	//	for	(y = 0; y < height; y++){
	//		for (x = 0; x < width; x++){
				//it is for 1 && 4 bpp, 1bpp's index only 1bit, 4bpp's index are 4bits
				//in 4bpp, the high bits color is for first pixel, low bits for second 
				if (count <= 0)
				{
					count = 8;
					BytesRead += 1;
					fread(&byte, sizeof(unsigned char), 1, InBmp);	
					image->ColorIndex[ele++] = byte;
				}
				
				count -= BitCount;
				ColorIndex = (byte >> count) & mask;

				image->DataB[y * width + x] = ColorMap[ColorIndex * 3 + 0];
				image->DataG[y * width + x] = ColorMap[ColorIndex * 3 + 1];
				image->DataR[y * width + x] = ColorMap[ColorIndex * 3 + 2];
				
			}

			if ((4 - BytesRead % 4) != 4)//BytesRead % 4 != 0
			{
				fread(PadZeros, sizeof(unsigned char), (4 - BytesRead % 4), InBmp);
				for (int r = 0; r < (4 - BytesRead % 4); r++){
					image->ColorIndex[ele++] = PadZeros[r];
				}
			}
		
			count = 0;
			BytesRead = 0;
		}
		image->IndexNum = ele;
	}
	image->BitCount = BitCount;
	return image;
}


void BmpWrite(FILE* OutBmp, BMPIMAGE *image)
{
	int x, y;
	int height, width, BitCount, OffBits;
	int LineBytes, PadBytes, BmpBytes, ImageBytes;
	IMGDATATYPE TempB, TempG, TempR;
	unsigned char b, g, r;
	unsigned char BmpHeader[54], PadZeros[4] = {0, 0, 0, 0};

	height = image->height;
	width = image->width;
	BitCount = image->BitCount;
	
	LineBytes = (width * BitCount + 31) / 32 * 4;
	ImageBytes = LineBytes * height;
	OffBits = 54 + ((BitCount == 24) ? 0 : ((1 << BitCount) * 4));
	BmpBytes = OffBits + ImageBytes; 
	PadBytes  = LineBytes - width * 3;
	
	_hx_memset(BmpHeader, 0 , 54);

	*(BmpHeader +  0) = 'B';
	*(BmpHeader +  1) = 'M';
	//whole file size(bytes), low byte-address store low-bits
	*(BmpHeader +  2) = (unsigned char)(BmpBytes);  
	*(BmpHeader +  3) = (unsigned char)(BmpBytes >> 8);
	*(BmpHeader +  4) = (unsigned char)(BmpBytes >> 16);
	*(BmpHeader +  5) = (unsigned char)(BmpBytes >> 24);
	//6,7,8,9 reserve
	//0 offsets to imagedata 
	*(BmpHeader + 10) = (unsigned char)(OffBits);
	*(BmpHeader + 11) = (unsigned char)(OffBits >> 8);
	*(BmpHeader + 12) = (unsigned char)(OffBits >> 16);
	*(BmpHeader + 13) = (unsigned char)(OffBits >> 24);
	//sizeof BmpInfoHeader
	*(BmpHeader + 14) = 0x28;
	//width
	*(BmpHeader + 18) = (unsigned char)(width);
	*(BmpHeader + 19) = (unsigned char)(width >> 8);
	*(BmpHeader + 20) = (unsigned char)(width >> 16);
	*(BmpHeader + 21) = (unsigned char)(width >> 24);
	//height
	*(BmpHeader + 22) = (unsigned char)(height);
	*(BmpHeader + 23) = (unsigned char)(height >> 8);
	*(BmpHeader + 24) = (unsigned char)(height >> 16);
	*(BmpHeader + 25) = (unsigned char)(height >> 24);
	//planes 
	*(BmpHeader + 26) = 1;
	//bitcount 28,29
	*(BmpHeader + 28) = (unsigned char)(BitCount); 
	//compression == 0(BI_RGB), 30,31,32,33
	*(BmpHeader + 30) = 0;//In general, no compression
	*(BmpHeader + 34) = (unsigned char)(ImageBytes);
	*(BmpHeader + 35) = (unsigned char)(ImageBytes >> 8);
	*(BmpHeader + 36) = (unsigned char)(ImageBytes >> 16);
	*(BmpHeader + 37) = (unsigned char)(ImageBytes >> 24);
	/************************************************************************/
	/* Hresolution, Vresolution, color numbers, important color nums can be 0;                                                                      */
	/************************************************************************/
	
	//write BMPheader
	fwrite(BmpHeader, sizeof(char), 54, OutBmp); 

	if(BitCount <= 8){
		int ColNum = 1 << BitCount;
		BYTE zero = 0;
		BYTE bb,gg,rr;
		//write the palate after bmpheader
		for (int q = 0; q < ColNum; q++){
			bb = image->ColorMap[q * 3 + 0];
			gg = image->ColorMap[q * 3 + 1];
			rr = image->ColorMap[q * 3 + 2];
			fwrite(&bb,sizeof(BYTE),1,OutBmp);
			fwrite(&gg,sizeof(BYTE),1,OutBmp);
			fwrite(&rr,sizeof(BYTE),1,OutBmp);
			//reserve = 0
			fwrite(&zero, sizeof(BYTE), 1, OutBmp);
		}
		//write the image data
		fwrite(image->ColorIndex, sizeof(BYTE), image->IndexNum, OutBmp);
	}
	
	else if (BitCount == 24){
		for (y = height - 1; y >= 0; y--)  
		{
			for (x = 0; x <width ; x++)    
			{
				TempB = image->DataB[y * width + x];
				TempG = image->DataG[y * width + x];
				TempR = image->DataR[y * width + x];

				TempB = (IMGDATATYPE)ROUND(TempB);
				TempG = (IMGDATATYPE)ROUND(TempG);
				TempR = (IMGDATATYPE)ROUND(TempR);

				b = (unsigned char)BOUND(TempB, 0, 255);
				g = (unsigned char)BOUND(TempG, 0, 255);
				r = (unsigned char)BOUND(TempR, 0, 255);

				fwrite(&b, sizeof(unsigned char), 1, OutBmp);
				fwrite(&g, sizeof(unsigned char), 1, OutBmp);
				fwrite(&r, sizeof(unsigned char), 1, OutBmp);
			}

			fwrite(PadZeros, sizeof(unsigned char), PadBytes, OutBmp);
		}
	}
	//free BMPIMAGE
	//ImageDealloc(image);
}


BMPIMAGE* ImageAlloc(int height, int width)
{
	BMPIMAGE* image;
	//allocate the BMPIAMGE struct
	if ((image = (BMPIMAGE *) malloc (sizeof(BMPIMAGE))) == NULL)
	{
		printf("fail to allocate memory image.\n");
		return NULL;
	}
	//allocate the image display data
	if ((image->DataB = (IMGDATATYPE *) calloc (height * width, sizeof(IMGDATATYPE))) == NULL)
	{
		printf("fail to allocate image->DataA.\n");
		return NULL;
	}

	if ((image->DataG = (IMGDATATYPE *) calloc (height * width, sizeof(IMGDATATYPE))) == NULL)
	{
		printf("fail to allocate image->DataB.\n");
		return NULL;
	}

	if ((image->DataR = (IMGDATATYPE *) calloc (height * width, sizeof(IMGDATATYPE))) == NULL)
	{
		printf("fail to allocate image->DataR.\n");
		return NULL;
	}
	image->ColorIndex = NULL;
	image->ColorMap = NULL;
	image->height = height;
	image->width = width;

	return image;
}


void ImageDealloc(BMPIMAGE* image)
{

	free(image->DataB);
	free(image->DataG);
	free(image->DataR);

	if(image->BitCount <= 8){
		free(image->ColorIndex);
		free(image->ColorMap);
	}

	free(image);
}

void BmpShow(__VIDEO *pVideo, int x, int y, BMPIMAGE *image){
	if(image == NULL)
		return;
	int i, j;
	int height, width;
	IMGDATATYPE TempB, TempG, TempR;
	unsigned char b, g, r;
	height = image->height;
	width = image->width;
	
	for (j = height - 1; j >= 0; j--){
		if(height - j >= y)
			break;
		for (i = 0; i <width ; i++){
			if(i + x >= width)
				break;
			
			TempB = image->DataB[j * width + i];
			TempG = image->DataG[j * width + i];
			TempR = image->DataR[j * width + i];
			
			TempB = (IMGDATATYPE)ROUND(TempB);
			TempG = (IMGDATATYPE)ROUND(TempG);
			TempR = (IMGDATATYPE)ROUND(TempR);
			
			b = (unsigned char)BOUND(TempB, 0, 255);
			g = (unsigned char)BOUND(TempG, 0, 255);
			r = (unsigned char)BOUND(TempR, 0, 255);
			putpixel(pVideo, x + i, y + j - height - 1, RGB(r,g,b));
		}
	}

}

void Bmp2Txt(BMPIMAGE *image, FILE *OutTxt){
	/************************************************************************/
	/* now, this function just transform image DataB(G or R) to text                                                                     */
	/************************************************************************/
	int i;
	char *strin = (char *)malloc(sizeof(char) * 6);
	char le = '{';
	char ri = '}';
	char k = '\n';
	char fen = ';';
	char *ArrayNameB = "BYTE DataBlue[]=";
	char *ArrayNameG = "BYTE DataGreen[]=";
	char *ArrayNameR = "BYTE DataRed[]=";

	fwrite(ArrayNameB, sizeof(char), 16, OutTxt);
	fwrite(&le, sizeof(char), 1, OutTxt);
	for (i=0; i<(image->height)*(image->width); i++){
		Get16Num(image->DataB + i, strin);
		if(i == (image->height)*(image->width)-1){
			fwrite(strin, sizeof(char), 4, OutTxt);
			break;
		}
		fwrite(strin, sizeof(char), 5, OutTxt);
		if (i % 10 == 0 && i){
			fwrite(&k, sizeof(char), 1, OutTxt);
		}
	}
	fwrite(&ri, sizeof(char), 1, OutTxt);
	fwrite(&fen, sizeof(char), 1, OutTxt);
	fwrite(&k, sizeof(char), 1, OutTxt);
	fwrite(&k, sizeof(char), 1, OutTxt);
	fwrite(&k, sizeof(char), 1, OutTxt);

	fwrite(ArrayNameG, sizeof(char), 17, OutTxt);
	fwrite(&le, sizeof(char), 1, OutTxt);
	for (i=0; i<(image->height)*(image->width); i++){
		Get16Num(image->DataG + i, strin);
		if(i == (image->height)*(image->width)-1){
			fwrite(strin, sizeof(char), 4, OutTxt);
			break;
		}
		fwrite(strin, sizeof(char), 5, OutTxt);
		if (i % 10 == 0 && i){
			fwrite(&k, sizeof(char), 1, OutTxt);
		}
	}
	fwrite(&ri, sizeof(char), 1, OutTxt);
	fwrite(&fen, sizeof(char), 1, OutTxt);
	fwrite(&k, sizeof(char), 1, OutTxt);
	fwrite(&k, sizeof(char), 1, OutTxt);
	fwrite(&k, sizeof(char), 1, OutTxt);

	fwrite(ArrayNameR, sizeof(char), 15, OutTxt);
	fwrite(&le, sizeof(char), 1, OutTxt);
	for (i=0; i<(image->height)*(image->width); i++){
		Get16Num(image->DataR + i, strin);
		if(i == (image->height)*(image->width)-1){
			fwrite(strin, sizeof(char), 4, OutTxt);
			break;
		}
		fwrite(strin, sizeof(char), 5, OutTxt);
		if (i % 10 ==0 && i){
			fwrite(&k, sizeof(char), 1, OutTxt);
		}
	}
	fwrite(&ri, sizeof(char), 1, OutTxt);
	fwrite(&fen, sizeof(char), 1, OutTxt);
	fwrite(&k, sizeof(char), 1, OutTxt);
	fwrite(&k, sizeof(char), 1, OutTxt);
	free(strin);
	
}

void Get16Num(unsigned char *orin, char *str){
	//str must malloc 6 bytes
	int j =0;
	str[j++] = '0';
	str[j++] = 'x';
	str[j++] = NUM16[(*orin) >> 4];
	str[j++] = NUM16[(*orin) & ((1 << 4) - 1)];
	str[j++] = ',';
	str[j] = '\0';
}


#endif

void BmpShowArray(
				  __VIDEO* pVideo,
				  int x, 
				  int y, 
				  int height, 
				  int width, 
				  BYTE *DataB, 
				  BYTE *DataG, 
				  BYTE *DataR
				  )
{
	int i, j;
	BYTE r, g, b;
	IMGDATATYPE TempB, TempG, TempR;
	for (j = height - 1; j >= 0; j--){
		//Modified by garry in 2009.04.22:
		//I don't know the usage of the following statements.But once you specify a height value
		//greater than y,it can not display correctly.
		//if(height - j >= y)
		//	break;
		for (i = 0; i <width ; i++){
		//	if(i + x >= width)
		//		break;
			
			TempB = DataB[j * width + i];
			TempG = DataG[j * width + i];
			TempR = DataR[j * width + i];
			
			TempB = (IMGDATATYPE)ROUND(TempB);
			TempG = (IMGDATATYPE)ROUND(TempG);
			TempR = (IMGDATATYPE)ROUND(TempR);
			
			b = (unsigned char)BOUND(TempB, 0, 255);
			g = (unsigned char)BOUND(TempG, 0, 255);
			r = (unsigned char)BOUND(TempR, 0, 255);
			DrawPixel(pVideo, x + i, y + j - height - 1, RGB(r,g,b));
		}
	}
}

