//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Jan 06,2012
//    Module Name               : launch.cpp
//    Module Funciton           : 
//                                Application launcher's implementation code.
//                                The launcher is one of most important part of GUI shell,
//                                which is used to list all GUI applications and launch it
//                                according to user request.
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/
 
#include "..\INCLUDE\KAPI.H"
#include "..\INCLUDE\STDIO.H"
#include "..\INCLUDE\STRING.H"
#include "..\INCLUDE\VESA.H"
#include "..\INCLUDE\VIDEO.H"
#include "..\INCLUDE\CLIPZONE.H"
#include "..\INCLUDE\GDI.H"
#include "..\INCLUDE\WNDMGR.H"
#include "..\INCLUDE\GLOBAL.H"
#include "..\INCLUDE\GUISHELL.H"
 
#include "..\INCLUDE\MSGBOX.H"
#include "..\include\bmpbtn.h"
#include "..\include\launch.h"
 
//Application profile's list.
static __APP_PROFILE*  pAppProfileList = NULL;
 
//A helper routine to verify if a file is a Hello China eXecutable(HCX) file.
static BOOL IsHCX(FS_FIND_DATA* pFindData)
{
         int nfnlen = 0;
 
         if(NULL == pFindData)
         {
                   return FALSE;
         }
         //Get file name's length.
         nfnlen = strlen(pFindData->cAlternateFileName);
         if(nfnlen <= 4)  //HCX file's name must larger than 4.
         {
                   return FALSE;
         }
         if(0 == strcmp(&pFindData->cAlternateFileName[nfnlen - 4],".HCX"))  //Post-fix is not '.HCX'.
         {
                   return TRUE;
         }
         if(0 == strcmp(&pFindData->cAlternateFileName[nfnlen - 4],".hcx"))  //Post-fix is not '.hcx'.
         {
                   return TRUE;
         }
         return FALSE;
}
 
//A local helper routine to load application's bitmap from HCX file.
static BOOL LoadBitmap(FS_FIND_DATA* pFindData,__APP_PROFILE* pApp)
{
         HANDLE  hHcxFile     = NULL;
         CHAR    FullName[MAX_FILE_NAME_LEN];
         BOOL    bResult      = FALSE;
         LPVOID  pBuffer      = NULL;
         DWORD   dwReadSize   = 0;
         __HCX_HEADER* pHdr   = NULL;
         __COLOR clr          = 0;
         __COLOR* pClrArray   = (__COLOR*)pApp->pBitmap;
         char*   pR           = NULL;
         char*   pB           = NULL;
         char*   pG           = NULL;
         char*   pRGBStart    = NULL;
         DWORD   i            = 0;
 
         strcpy(FullName,GUI_APP_DIRECTORY);
         strcat(FullName,pFindData->cAlternateFileName);
         //Open the HCX file.
         hHcxFile = CreateFile(
                   FullName,
                   FILE_ACCESS_READ,
                   0,
                   NULL);
         if(NULL == hHcxFile)
         {
                   goto __TERMINAL;
         }
         pBuffer = KMemAlloc(pFindData->nFileSizeLow,KMEM_SIZE_TYPE_ANY);
         if(NULL == pBuffer)
         {
                   goto __TERMINAL;
         }
         //Read the whole content.
         if(!ReadFile(hHcxFile,pFindData->nFileSizeLow,pBuffer,&dwReadSize))
         {
                   goto __TERMINAL;
         }
         if(dwReadSize != pFindData->nFileSizeLow)
         {
                   goto __TERMINAL;
         }
         pHdr = (__HCX_HEADER*)pBuffer;
         if((pHdr->dwBmpWidth != PROFILE_BMP_DIMX) || (pHdr->dwBmpHeight != PROFILE_BMP_DIMY))
         {
                   goto __TERMINAL;
         }
         if(pHdr->dwColorBits < 24)  //Only 24 bits color are supported.
         {
                   goto __TERMINAL;
         }
         //Locate the color array.
         pRGBStart = (char*)pBuffer + pHdr->dwBmpOffset;
         pR        = pRGBStart;
         pB        = pRGBStart + pHdr->dwBmpHeight * pHdr->dwBmpWidth;
         pG        = pRGBStart + pHdr->dwBmpHeight * pHdr->dwBmpWidth * 2;
         for(i = 0;i < pHdr->dwBmpWidth * pHdr->dwBmpHeight;i ++)
         {
                   clr = RGB(*pR,*pG,*pB);
                   *pClrArray = clr;
                   pR ++;
                   pB ++;
                   pG ++;
                   pClrArray ++;
         }
         //Set application's name as user specified.
         for(i = 0;i < MAX_SHOWNAME_LENGTH;i ++)
		 {
			 if(0 == pHdr->AppName[i])
			 {
				 break;
			 }
			 pApp->ShowName[i] = pHdr->AppName[i];
		 }
		 pApp->ShowName[i] = 0;  //Set the end of show name.

         bResult = TRUE;
 
__TERMINAL:
         //Release all resources.
         if(NULL != hHcxFile)
         {
                   CloseFile(hHcxFile);
         }
         if(NULL != pBuffer)
         {
                   KMemFree(pBuffer,KMEM_SIZE_TYPE_ANY,0);
         }
         return bResult;
}
 
//Initializes application profile list by travel the GUI application directory.
static __APP_PROFILE* InitProfileList(HANDLE hMainFrame)
{
         HANDLE          hFindHandle = NULL;
         FS_FIND_DATA    FindData;
         __APP_PROFILE*  pNewApp     = NULL;
         BOOL            bResult     = FALSE;
         BOOL            bFindResult = FALSE;
         static DWORD    AppBtnID    = 1024;
         //char            buff[32];
 
         //Try to search the application residential directory.
         hFindHandle = FindFirstFile(
                   GUI_APP_DIRECTORY,
                   &FindData);
         if(NULL == hFindHandle)
         {
                   //MessageBox(hMainFrame,"FindFirstFile failed.","ERROR",MB_OKCANCEL);
                   goto __TERMINAL;
         }
         do{
                   //Create a new application profile object and link it to list,if the file is a GUI app.
                   if(IsHCX(&FindData))
                   {
                            pNewApp = (__APP_PROFILE*)KMemAlloc(sizeof(__APP_PROFILE),KMEM_SIZE_TYPE_ANY);
                            if(NULL == pNewApp)
                            {
                                     goto __TERMINAL;
                            }
                            //Allocate memory for application's man ICON(bitmap).
                            pNewApp->pBitmap = KMemAlloc(PROFILE_BMP_DIMX * PROFILE_BMP_DIMY * 4,KMEM_SIZE_TYPE_ANY);
                            if(NULL == pNewApp->pBitmap)
                            {
                                     goto __TERMINAL;
                            }
                            //Initialize the application profile object.
                            strcpy(pNewApp->AppName,FindData.cAlternateFileName);
                            pNewApp->dwButtonID = AppBtnID ++;
                            pNewApp->hAppMainThread = NULL;
                            pNewApp->hButton    = NULL;
                            pNewApp->dwFileSize = FindData.nFileSizeLow;  //Store the file's size.
                            //Load the HCX's bitmap information,to display it in launcher's bitmap button.
                            if(!LoadBitmap(&FindData,pNewApp))
                            {
                                     KMemFree(pNewApp->pBitmap,KMEM_SIZE_TYPE_ANY,0);
                                     pNewApp->pBitmap = NULL;
                            }
                            //Link to profile list.
                            pNewApp->pNext = pAppProfileList;
                            pAppProfileList = pNewApp;
                   }
                   //Find next one.
                   bFindResult = FindNextFile(
                            GUI_APP_DIRECTORY,
                            hFindHandle,
                            &FindData);
         }while(bFindResult);
 
         //_hx_sprintf(buff,"Button ID = %d",AppBtnID);
         //MessageBox(hMainFrame,buff,"Info",MB_OK);
 
         bResult = TRUE;
 
__TERMINAL:
         if(hFindHandle)
         {
                   FindClose(GUI_APP_DIRECTORY,hFindHandle);
         }
         if(!bResult)  //Should release the resource allocated.
         {
                   KMemFree(pNewApp,KMEM_SIZE_TYPE_ANY,0L);
         }
         return pAppProfileList;
}
 
//Load application profiles in the main frame window.
void LoadAppProfile(HANDLE hMainFrame)
{
         HANDLE hBmpBtn = NULL;
         __RECT rect;
         __APP_PROFILE* pAppProfile = NULL;
         int row,col;  //How many rows and columns in main frame.
         int i,j;
 
         //Initialize application profile list first.
         pAppProfile = InitProfileList(hMainFrame);
         if(NULL == pAppProfile)
         {
                   return;
         }
 
         if(!GetWindowRect(hMainFrame,&rect,GWR_INDICATOR_CLIENT))
         {
                   goto __TERMINAL;
         }
 
         //Calculate how many rows and how many columns application profiles in
         //main frame.
         row = (rect.right - rect.left - 40) / (PROFILE_BMP_DIMX + PROFILE_MARGIN);
         col = (rect.bottom - rect.top - 40) / (PROFILE_BMP_DIMY + PROFILE_MARGIN * 4);  //Reserve text's space of bitmap button.
 
         //Create app profiles in main frame by using bitmap button.
         for(i = 0;i < col;i ++)
         {
                   for(j = 0;j < row;j ++)
                   {
                            if(NULL == pAppProfile)
                            {
                                     break;
                            }
                            pAppProfile->hButton = CreateBitmapButton(
                                     hMainFrame,
                                     pAppProfile->ShowName,
                                     pAppProfile->dwButtonID,
                                     35 + PROFILE_MARGIN + j * (PROFILE_BMP_DIMX + PROFILE_MARGIN),
                                     35 + PROFILE_MARGIN + i * (PROFILE_BMP_DIMY + PROFILE_MARGIN * 4),
                                     PROFILE_BMP_DIMX,
                                     PROFILE_BMP_DIMY,
                                     pAppProfile->pBitmap,
                                     NULL);
                            if(NULL == pAppProfile->hButton)  //Failed to create button.
                            {
                                     goto __TERMINAL;
                            }
                            pAppProfile = pAppProfile->pNext;
                   }
                   if(NULL == pAppProfile)
                   {
                            break;
                   }
         }
 
__TERMINAL:
         return;
}
 
//Fetch executable part from HCX file,return the start address and it's size.
static LPVOID FetchExe(LPVOID pFileBuff,DWORD* pExeSize)
{
         __HCX_HEADER*  pHcxHdr = (__HCX_HEADER*)pFileBuff;
 
         *pExeSize = pHcxHdr->dwBmpOffset;
         return pFileBuff;
}
 
//A local helper routine to launch a HCX executable file,given it's loaded
//address in memory.
static BOOL ExecuteHCX(LPVOID pStartAddr,__APP_PROFILE* pAppProfile)
{
         if((NULL == pStartAddr) || (NULL == pAppProfile))
         {
                   return FALSE;
         }
         if(0xE9909090 != *(DWORD*)pStartAddr)  //Not is a valid BIN file.
         {
                   return FALSE;
         }
         pAppProfile->hAppMainThread = CreateKernelThread(
                   0,
                   KERNEL_THREAD_STATUS_READY,
                   PRIORITY_LEVEL_NORMAL,
                   (__KERNEL_THREAD_ROUTINE)pStartAddr,
                   NULL,
                   NULL,
                   pAppProfile->AppName);
         if(NULL == pAppProfile->hAppMainThread)
         {
                   return FALSE;
         }
         return TRUE;
}
 
//Load a HCX file,distill it's executable part and load it into HCX_LOAD_ADDRESS,then
//execute it.
static BOOL LoadHCX(__APP_PROFILE* pAppProfile)
{
         BOOL            bResult        = FALSE;
         HANDLE          hFile          = NULL;
         LPVOID          pFileBuff      = NULL;
         CHAR            FullName[MAX_FILE_NAME_LEN];
         DWORD           dwReadSize     = 0;
         LPVOID          pExecut        = NULL;
 
         if(NULL == pAppProfile)
         {
                   goto __TERMINAL;
         }
         //Allocate memory to hold the HCX file.
         if(0 == pAppProfile->dwFileSize)
         {
                   goto __TERMINAL;
         }
         pFileBuff = KMemAlloc(pAppProfile->dwFileSize,KMEM_SIZE_TYPE_ANY);
         if(NULL == pFileBuff)
         {
                   goto __TERMINAL;
         }
 
         //Open the HCX file,construct file's full path and name first.
         strcpy(FullName,GUI_APP_DIRECTORY);
         strcat(FullName,pAppProfile->AppName);
         hFile = CreateFile(
                   FullName,
                   FILE_ACCESS_READ,
                   0,
                   NULL);
         if(NULL == hFile)
         {
                   goto __TERMINAL;
         }
         //Load the HCX file.
         if(!ReadFile(hFile,pAppProfile->dwFileSize,pFileBuff,&dwReadSize))
         {
                   goto __TERMINAL;
         }
         if(dwReadSize != pAppProfile->dwFileSize)  //File's size is not same as it first checked.
         {
                   goto __TERMINAL;
         }
         //OK,fetch it's executable part and load it to HCX_LOAD_ADDRESS.
         pExecut = FetchExe(pFileBuff,&dwReadSize);  //dwReadSize contains the executable part's size.
         if(NULL == pExecut)  //Maybe not a valid HCX file format.
         {
                   goto __TERMINAL;
         }
         _hx_memcpy((LPVOID)HCX_LOAD_ADDRESS,pExecut,dwReadSize);
         //Execute it.
         if(!ExecuteHCX((LPVOID)HCX_LOAD_ADDRESS,pAppProfile))
         {
                   goto __TERMINAL;
         }
         bResult = TRUE;
 
__TERMINAL:
         if(pFileBuff)
         {
                   KMemFree(pFileBuff,KMEM_SIZE_TYPE_ANY,0L);
         }
         if(hFile)
         {
                   CloseFile(hFile);
         }
         return bResult;
}
 
//Launch a application,the button ID that user clicked is a key to locate
//the appropriate application profile.
void LaunchApplication(DWORD dwButtonID)
{
         __APP_PROFILE* pProfile = pAppProfileList;
         
         if(NULL == pProfile)
         {
                   return;
         }
         while(pProfile->dwButtonID != dwButtonID)
         {
                   pProfile = pProfile->pNext;
                   if(NULL == pProfile)  //End of list.
                   {
                            break;
                   }
         }
         if(NULL == pProfile)  //Can not find a matched profile.
         {
                   return;
         }
		 if(pProfile->hAppMainThread != NULL)  //The application has been launched before.
		 {
			 DestroyKernelThread(pProfile->hAppMainThread);
			 pProfile->hAppMainThread = NULL;
		 }
         //Find the appropriate application profile,launch it.
         LoadHCX(pProfile);
         return;
}
