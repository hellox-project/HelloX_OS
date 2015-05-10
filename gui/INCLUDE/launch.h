//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Jan 06,2012
//    Module Name               : launch.h
//    Module Funciton           : 
//                                Application launcher related definitions and constants.
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

#ifndef __LAUNCH_H__
#define __LAUNCH_H__

//Use 128 * 128 dimension for application profile.
#define PROFILE_BMP_DIMX   128
#define PROFILE_BMP_DIMY   128

//Margin between 2 app profiles,both for vertical and horizontal.
#define PROFILE_MARGIN     10

//Default application's load address.HCX's executable part will be loaded
//into this address to execute by launcher.
//The user application's size should not large than 128K bytes,since only 128K
//byte space,which start from HCX_LOAD_ADDRESS to 2M,is reserved for it.
#define HCX_LOAD_ADDRESS   0x001E0000
#define MAX_HCXFILE_SIZE   0x00020000

//GUI application's position.
#define GUI_APP_DIRECTORY "C:\\HCGUIAPP\\"

//Display name's maximal length.
#define MAX_SHOWNAME_LENGTH  16

//Application profile,describes each application in GUI shell.
struct __APP_PROFILE{
	__APP_PROFILE*  pNext;                       //Next in link.
	TCHAR           AppName[MAX_FILE_NAME_LEN];  //Application's file name.
	TCHAR           ShowName[MAX_SHOWNAME_LENGTH + 1];  //Application's name.
	HANDLE          hAppMainThread;              //Main thread handle of the application.
	DWORD           dwButtonID;                  //Bitmap button ID.
	HANDLE          hButton;                     //Bitmap button's handle.
	DWORD           dwFileSize;                  //HCX file's size.
	LPVOID          pBitmap;                     //Bitmap data,128 * 128 dimension,32 bits color.
};

//Load all application's profile.
void LoadAppProfile(HANDLE hMainFrame);

//Launch a application,a button ID is given to locate the appropriate application.
void LaunchApplication(DWORD dwButtonId);

//HCX header,will be put in each HCX file's begining by hcxbuild application.
struct __HCX_HEADER{
         DWORD          dwHcxSignature;   //Signature,HCX_SIGNATURE.
         DWORD          dwEntryOffset;    //Entry point's offset in HCX file.
         DWORD          dwBmpOffset;      //HCX application bitmap's offset in HCX file.
         DWORD          dwBmpWidth;       //Bitmap's width.
         DWORD          dwBmpHeight;      //Bitmap's height.
         DWORD          dwColorBits;      //How many bits per color.
         CHAR           AppName[16];      //Application's display name.
         CHAR           MajorVersion;     //Major version of this app.
         CHAR           MinorVersion;     //Minor version of this app.
         CHAR           OsMajorVersion;   //OS's major version this app require.
         CHAR           OsMinorVersion;   //OS's minor version this app require.
};
 
#define HCX_SIGNATURE 0xE9909090  //HCX file's signature,it's a sign of a valid HCX file.
 
//Round a number to 16's boundary.
#define ROUND_TO_16(x) ((x) % 16 == 0 ? (x) : ((x) + 16 - ((x) % 16)))

#endif