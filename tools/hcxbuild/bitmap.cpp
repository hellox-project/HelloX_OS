//***********************************************************************/
//    Author                    : sh
//    Original Date             : 14,04 2009
//    Module Name               : BMPAPI.cpp
//    Module Funciton           : Read BMP from file, Write BMP from memory to 
//                                                                         file; ImageAlloc allocate mem for BMPIMAGE
//
//    Last modified Author      : sh
//    Last modified Date        : 18,04 2009
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              : 
//***********************************************************************/
 
//#ifndef __KAPI_H__
//#include "..\INCLUDE\\KAPI.H"
//#endif
 
//#ifndef __VESA_H__
//#include "..\INCLUDE\VESA.H"
//#endif
 
//#ifndef __VIDEO_H__
//#include "..\INCLUDE\VIDEO.H"
//#endif
 
//#ifndef __GUISHELL_H__
//#include "..\INCLUDE\GUISHELL.H"
//#endif
 
//#include "..\include\BMPAPI.h"
#include "StdAfx.h"
#include "bitmap.h"
 
#define __MEM_ALLOC(size) malloc(size)
#define __MEM_FREE(p)     free(p)
 
char NUM16[16] = {
         '0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};
 
BMPIMAGE *LoadBitmap(HANDLE InBmp)
{
         int n, x, y;
         int count, mask, type, compression,bmpsig;
         int width, height, BitCount, ColorNum;
         int LineBytes, BytesRead, ColorIndex;
         unsigned char r, g, b, byte;
         unsigned char PadZeros[4];
         unsigned char BmpHeader[54];
         unsigned long readSize = 0;
         BMPIMAGE *image = NULL;      //Image object which will be returned by this routine if successful.
         char*    pBuffer = NULL;     //Used to buffer file content.
         char*    pRGB    = NULL;     //Used to travel pBuffer.
         int      buffSize = 0;
         
         //Read 54 bytes to BmpHeader
         //fread(BmpHeader, sizeof(unsigned char), 54, InBmp); 
         if(!ReadFile(InBmp,BmpHeader,54,&readSize,NULL))
         {
                   goto __TERMINAL;
         }
         //little-endian the high bit in high byte
         //type 0 1
         //type = BmpHeader.BmpFileHeader.type;
         type= BmpHeader[1];
         type <<= 8;
         type += BmpHeader[0];
         //width 18 19 20 21 
         //width = BmpHeader.BmpInfoHeader.width;
 
         width = ((DWORD)BmpHeader[21] << 24)
                     + ((DWORD)BmpHeader[20] << 16)
                     + ((DWORD)BmpHeader[19] << 8)
                     + BmpHeader[18]; 
         //Height 22 23 24 25
         //height = BmpHeader.BmpInfoHeader.height;
         height = ((DWORD)BmpHeader[25] << 24)
                      + ((DWORD)BmpHeader[24] << 16)
                      + ((DWORD)BmpHeader[23] << 8)
                      + BmpHeader[22];
         //bitcount 28 29
         //BitCount = BmpHeader.BmpInfoHeader.BitCount;
         BitCount = ((DWORD)BmpHeader[29] << 8) + BmpHeader[28];
         //compression 30 31 32 33
         //compression = BmpHeader.BmpInfoHeader.compression;
         compression = ((DWORD)BmpHeader[33] << 24)
                           + ((DWORD)BmpHeader[32] << 16)
                                     + ((DWORD)BmpHeader[31] << 8)
                                     + BmpHeader[30];
         LineBytes = (width * BitCount + 31) / 32 * 4;
 
         
         /************************************************************************/
         /* Validate the file's type,only BMP supported currently.
         /************************************************************************/
         bmpsig = 'M';
         bmpsig <<= 8;
         bmpsig += 'B';
         if (type != bmpsig)
         {
                   //printf("the file maybe is not a bmp image.\n");
                   goto __TERMINAL;
         }
 
         if (compression != 0)
         {
                   printf("can not read a compressed bmp.\n");
                   goto __TERMINAL;
         }
 
         if (BitCount > 8 && BitCount != 24)
         {
                   printf("can not read a %d bit bmp.\n", BitCount);
                   goto __TERMINAL;
         }
         
         //If the bmp has 24 bpp
         if (BitCount == 24)
         {
                   if ((image = ImageAlloc(height, width)) == NULL)
                   {
                            //printf("fail to allocate image.\n");
                            return NULL;
                   }
                   buffSize = height * (width + 3);  //At most 3 bytes for line padding.
                   buffSize *= 3;  //3 bytes per pixel.
                   pBuffer = (char*)__MEM_ALLOC(buffSize);  //4 bytes for padding.
                   if(NULL == pBuffer)
                   {
                            goto __TERMINAL;
                   }
                   //Read the whole file's content into memory.
                   if(!ReadFile(InBmp,pBuffer,buffSize,&readSize,NULL))
                   {
                            goto __TERMINAL;
                   }
                   pRGB = pBuffer;
                   for (y = height - 1; y >= 0; y--) 
                   {
                            for (x = 0; x < width; x++)    
                            {
                                     //first is blue, then is green, third is red
                                     b = *pRGB ++;
                                     g = *pRGB ++;
                                     r = *pRGB ++;
 
                                     image->DataB[y * width + x] = b; 
                                     image->DataG[y * width + x] = g;
                                     image->DataR[y * width + x] = r;
                            }
                            //there are less than 4 bytes to align, read it to PadZeros
                            //fread(PadZeros, sizeof(unsigned char), (LineBytes - width * 3), InBmp);
                            //ReadFile(InBmp,PadZeros,(LineBytes - width * 3),&readSize,NULL);
                            pRGB += (LineBytes - width * 3);
                   }
         }
         else if (BitCount <= 8)  
         {
                   ColorNum = 1 << BitCount;//palate size
                   if ((image = ImageAlloc(height, width)) == NULL)
                   {
                            //printf("fail to allocate image.\n");
                            goto __TERMINAL;
                   }
                   //Allocate color map for this bitmap.
                   if((image->ColorMap = (BYTE *)__MEM_ALLOC(3 * (1 << BitCount) * sizeof(BYTE))) == NULL)
                   {
                            goto __TERMINAL;
                   }
                   //Allocate BMP file buffer,used to contain palate and color index.
                   buffSize = height * (width + 3);
                   pBuffer  = (char*)__MEM_ALLOC(buffSize);
                   if(NULL == pBuffer)
                   {
                            goto __TERMINAL;
                   }
                   //Read palate first.
                   if(!ReadFile(InBmp,pBuffer,ColorNum * 4,&readSize,NULL))
                   {
                            goto __TERMINAL;
                   }
                   pRGB = pBuffer;  //Use pRGB to travel pBuffer since we should not change pBuffer's value.
                   for (n = 0; n < ColorNum; n++) 
                   { 
                            b = *pRGB ++;
                            g = *pRGB ++;
                            r = *pRGB ++;
                            pRGB ++;  //Skip the padding byte.
                            image->ColorMap[n * 3 + 0] = b;
                            image->ColorMap[n * 3 + 1] = g;
                            image->ColorMap[n * 3 + 2] = r;
             }
                   //Allocate color index array,please note that since zero maybe padded into line to align 4 bytes,
                   //extra memory space is reserved here to fit this situation.
                   if((image->ColorIndex = (BYTE*)__MEM_ALLOC(height * (width + 3) * sizeof(BYTE))) == NULL)
                  {
                            goto __TERMINAL;
                   }
                   //Now read the BMP's index data.
                   if(!ReadFile(InBmp,pBuffer,buffSize,&readSize,NULL))
                   {
                            goto __TERMINAL;
                   }
                   pRGB = pBuffer;  //Use pRGB to travel the pBuffer.
                   count = 0;
                   BytesRead = 0;
                   mask = (1 << BitCount) - 1;
                   int ele = 0;
                   //BMP的图像存储是倒立的，与正常图像沿y轴对称
                   for (y = height - 1; y >= 0; y--) 
                   {
                            for (x = 0; x < width; x++)    
                            {
                                     if (count <= 0)
                                     {
                                               count = 8;
                                               BytesRead += 1;
                                               //fread(&byte, sizeof(unsigned char), 1, InBmp);   
                                               //ReadFile(InBmp,&byte,1,&readSize,NULL);
                                               byte = *pRGB ++;
                                               image->ColorIndex[ele++] = byte;
                                     }
                                     count -= BitCount;
                                     ColorIndex = (byte >> count) & mask;
 
                                     image->DataB[y * width + x] = image->ColorMap[ColorIndex * 3 + 0];
                                     image->DataG[y * width + x] = image->ColorMap[ColorIndex * 3 + 1];
                                     image->DataR[y * width + x] = image->ColorMap[ColorIndex * 3 + 2];
                            }
 
                            if ((4 - BytesRead % 4) != 4)//BytesRead % 4 != 0
                            {
                                     //fread(PadZeros, sizeof(unsigned char), (4 - BytesRead % 4), InBmp);
                                     pRGB += (4 - BytesRead % 4);
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
__TERMINAL:
         if(NULL != pBuffer)
         {
                   __MEM_FREE(pBuffer);
         }
         return image;
}
 
 
BMPIMAGE* ImageAlloc(int height, int width)
{
         BMPIMAGE* image   = NULL;
         BOOL      bResult = FALSE;
 
         //allocate the BMPIAMGE struct
         if ((image = (BMPIMAGE *)__MEM_ALLOC(sizeof(BMPIMAGE))) == NULL)
         {
                   goto __TERMINAL;
         }
         //allocate the image display data
         if ((image->DataB = (IMGDATATYPE *)__MEM_ALLOC(height * width * sizeof(IMGDATATYPE))) == NULL)
         {
                   goto __TERMINAL;
         }
 
         if ((image->DataG = (IMGDATATYPE *)__MEM_ALLOC(height * width * sizeof(IMGDATATYPE))) == NULL)
         {
                   goto __TERMINAL;
         }
 
         if ((image->DataR = (IMGDATATYPE *)__MEM_ALLOC(height * width * sizeof(IMGDATATYPE))) == NULL)
         {
                   goto __TERMINAL;
         }
         image->BitCount   = 24;   //Only allocate 24 bits true color image object.
         image->ColorIndex = NULL;
         image->ColorMap = NULL;
         image->height = height;
         image->width = width;
         bResult = TRUE;
 
__TERMINAL:
         if(!bResult)
         {
                   ImageDealloc(image);
                   image = NULL;
         }
         return image;
}
 
 
void ImageDealloc(BMPIMAGE* image)
{
 
         if(NULL == image)
         {
                   return;
         }
         if(image->DataB)
         {
                   __MEM_FREE(image->DataB);
         }
         if(image->DataG)
         {
                   __MEM_FREE(image->DataG);
         }
         if(image->DataR)
         {
                   __MEM_FREE(image->DataR);
         }
 
         if(image->BitCount <= 8){
                   if(image->ColorIndex)
                   {
                            __MEM_FREE(image->ColorIndex);
                   }
                   if(image->ColorMap)
                   {
                            __MEM_FREE(image->ColorMap);
                   }
         }
         __MEM_FREE(image);
}
 
//Show a bitmap in a given rectangle,x and y specify the left top corner of
//the target rectangle and cx/cy specify the demension of the rect.
void ShowBitmap(CDC* pDC, int x, int y,int cx,int cy,BMPIMAGE *image){
         int i, j;
         int height, width;
         IMGDATATYPE TempB, TempG, TempR;
         unsigned char b, g, r;
 
         height = image->height;
         width = image->width;
         
         if(image == NULL)
         {
                   return;
         }
         for(j = 0;j < height;j ++)
         {
                   if(j >= cy)
                   {
                            break;
                   }
                   for (i = 0; i <width ; i++)
                   {
                            if(i >= cx)
                            {
                                     break;
                            }
                            
                            TempB = image->DataB[j * width + i];
                            TempG = image->DataG[j * width + i];
                            TempR = image->DataR[j * width + i];
                            
                            TempB = (IMGDATATYPE)ROUND(TempB);
                            TempG = (IMGDATATYPE)ROUND(TempG);
                            TempR = (IMGDATATYPE)ROUND(TempR);
                            
                            b = (unsigned char)BOUND(TempB, 0, 255);
                            g = (unsigned char)BOUND(TempG, 0, 255);
                            r = (unsigned char)BOUND(TempR, 0, 255);
                            //putpixel(pVideo, x + i, y + j - height - 1, RGB(r,g,b));
                            pDC->SetPixel(x + i,y + j,RGB(r,g,b));
                   }
         }
}
 
/*
void Bmp2Txt(BMPIMAGE *image, FILE *OutTxt){
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
         
}*/
 
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
 
/*
void ShowBitmap(LPSTR fileName,CDC* pDC)
{
         BMPIMAGE* pImage = NULL;
 
         HANDLE hFile = CreateFile(
                   fileName,
                   GENERIC_READ,
                   FILE_SHARE_DELETE | FILE_SHARE_WRITE | FILE_SHARE_READ,
                   NULL,
                   OPEN_EXISTING,
                   0,
                   NULL);
         if(INVALID_HANDLE_VALUE == hFile)
         {
                   pDC->TextOut(0,0,"Can not open the BMP file to read.");
                   return;
         }
         pImage = LoadBitmap(hFile,pDC);
         if(NULL == pImage)
         {
                   pDC->TextOut(0,50,"Can not read BMP content from file.");
                   return;
         }
         //Now display it on screen.
         ShowBitmap(pDC,0,0,600,500,pImage);
         pDC->TextOut(0,50,"Read BMP file successfully.");
         //Release all objects and resources.
         CloseHandle(hFile);
         ImageDealloc(pImage);
}
*/
