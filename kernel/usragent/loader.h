//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Mar 29, 2024
//    Module Name               : loader.h
//    Module Funciton           : 
//                                HelloX's loader is defined in this
//                                file. The loader is a common purpuse that
//                                can load many types of module, such as PE,
//                                COFF, ELF, ...
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __LOADER_H__
#define __LOADER_H__

#include "hellox.h"
#include "pteb.h"

/* Loader errors. */
#define LOADER_ERROR_OK           0
#define LOADER_ERROR_IMAGEHEADER  1
#define LOADER_ERROR_NORELOC      2
#define LOADER_ERROR_LOAD         3
#define LOADER_ERROR_BIND         4

/* Load all modules of a application into memory. */
unsigned long LoadAllModules(__PEB* peb);

/* Unload all modules. */
unsigned long UnloadModules(__PEB* peb);

/* Bind all module's import symbol. */
unsigned long BindModulesSymbol(__PEB* peb);

/* Windows API SET name prefix. */
#define WIN_API_SET_PREFIX_API "api-ms-win-"
#define WIN_API_SET_PREFIX_EXT "ext-ms-win-"

/* Loader version string. */
#define LOADER_VER "Loader 1.0.0"

/* The the container DLL's name given API SET name. */
const char* GetContainerDllName(const char* api_name);

#endif  //__LOADER_H__
