//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Mar 21,2009
//    Module Name               : MODMGR.H
//    Module Funciton           : 
//                                This module countains module manager
//                                and it's related pro-type definition.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __MODMGR_H__
#define __MODMGR_H__

#include "types.h"

#include "ktmgr.h"

#ifdef __cplusplus
extern "C" {
#endif

//Hello China's root directory name.
#define OS_ROOT_DIR     "C:\\PTHOUSE\\"
#define OS_MOD_CFGFILE  "MODCFG.INI"   //Module's configure file name.
#define MAX_MODCF_LEN   4096           //Maximal module configure file's size.

//Maximal external module file name's length.
#define MAX_MOD_FILENAME 32
#define MAX_LINE_LENGTH  256   //Maximal line's length in MODCFG.INI file.

//External module types.
#define MOD_ATTR_RES        0x00000000
#define MOD_ATTR_BIN        0x00000001
#define MOD_ATTR_EXE        0x00000002
#define MOD_ATTR_DLL        0x00000004

//Module's initialization routine.
typedef BOOL  (*__MODULE_INIT)(void);

//Module manager's definition.
typedef struct tag__MODULE_MGR{
	__KERNEL_THREAD_ROUTINE ShellEntry;     //Shell entry,can be replaced by other kernel module.
	BOOL                    (*Initialize)(struct tag__MODULE_MGR* lpThis);
	BOOL                    (*InitModule)(__MODULE_INIT pInitEntry);
	BOOL                    (*ReplaceShell)(__KERNEL_THREAD_ROUTINE shell);
	BOOL                    (*LoadExternalMod)(LPSTR strCfgFileName);  //Load external modules from external storage.
} __MODULE_MGR;

//Object to manager kernel module.
typedef struct tag__KERNEL_MODULE{
	DWORD         dwLoadAddress;    //Base address of this kernel module.
	__MODULE_INIT InitRoutine;      //Module's initialize routine address,in must case,it is same as above.
}__KERNEL_MODULE;

//External module description object,used to describe an external module.
BEGIN_DEFINE_OBJECT(__EXTERNAL_MOD_DESC)
    CHAR    ModFileName[MAX_MOD_FILENAME + 1];  //Module file's name.
    DWORD   StartAddress;                   //Target loading address.
	DWORD   dwModAttribute;                 //Module attribute.
END_DEFINE_OBJECT(__EXTERNAL_MOD_DESC)

//A global array is defined to manager all kernel module.
//These modules are static linked into kernel.
extern __KERNEL_MODULE KernelModule[];

//Module manager object,it is a global object.
extern __MODULE_MGR ModuleMgr;

#ifdef __cplusplus
}
#endif

#endif  //__MODMGR_H__
