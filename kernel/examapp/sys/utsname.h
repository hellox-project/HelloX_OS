//***********************************************************************/
//    Author                    : Garry
//    Original Date             : April 17,2015
//    Module Name               : utsname.h
//    Module Funciton           : 
//                                Simulates POSIX utsname.h.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __UTSNAME_H__
#define __UTSNAME_H__

//Information length.
#define _UTSNAME_SYSNAME_LENGTH   16
#define _UTSNAME_NODENAME_LENGTH  32
#define _UTSNAME_RELEASE_LENGTH   32
#define _UTSNAME_VERSION_LENGTH   32
#define _UTSNAME_MACHINE_LENGTH   32

//struct utsname.
struct utsname{
	char sysname[_UTSNAME_SYSNAME_LENGTH];   //OS Name.
	char nodename[_UTSNAME_NODENAME_LENGTH]; //Name on network.
	char release[_UTSNAME_RELEASE_LENGTH];   //Release level.
	char version[_UTSNAME_VERSION_LENGTH];   //Current version.
	char machine[_UTSNAME_MACHINE_LENGTH];   //Hardware platform type.
};

//uname routine.
extern int uname(struct utsname* __name);

#endif //__UTSNAME_H__
