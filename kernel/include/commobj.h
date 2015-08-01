//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Sep,03 2004
//    Module Name               : commobj.cpp
//    Module Function           : 
//                                This module countains common object definition
//                                and macros.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __COMMOBJ_H__
#define __COMMOBJ_H__


#include "types.h"
#ifdef __cplusplus
extern "C" {
#endif

//Root object for all system level objects.
typedef struct tag__COMMON_OBJECT{
	DWORD              dwObjectType;         //Object type.
	DWORD              dwObjectID;           //Object ID.
	DWORD              dwObjectSize;         //This object's size.
	struct tag__COMMON_OBJECT*   lpPrevObject;         //Point to previous object.
	struct tag__COMMON_OBJECT*   lpNextObject;         //Point to next object.
	struct tag__COMMON_OBJECT*   lpObjectOwner;        //This object's owner.
	//__COMMON_OBJECT*   lpLeft;               //Used in the future,AVL tree's left branch.
	//__COMMON_OBJECT*   lpRight;              //Used in the ruture,AVL tree's right branch.
	BOOL               (*Initialize)(struct tag__COMMON_OBJECT*); //Object's initialize routine.
	VOID               (*Uninitialize)(struct tag__COMMON_OBJECT*);
} __COMMON_OBJECT;


//
//The following macro is used to implementation inherit.
//
#define INHERIT_FROM_COMMON_OBJECT \
	DWORD              dwObjectType;        \
	DWORD              dwObjectID;          \
	DWORD              dwObjectSize;        \
	__COMMON_OBJECT*   lpPrevObject;        \
	__COMMON_OBJECT*   lpNextObject;        \
	__COMMON_OBJECT*   lpObjectOwner;       \
	BOOL               (*Initialize)(__COMMON_OBJECT*);    \
	VOID               (*Uninitialize)(__COMMON_OBJECT*);  

//
//Begin to define a object type,i.e,a class.
//
#define BEGIN_DEFINE_OBJECT(objectname) \
	typedef struct tag##objectname{

//
//End of the definition of object.
//
#define END_DEFINE_OBJECT(objectname) \
	}objectname;

//
//Translate the source type to destination type.
//
#define OBJECT_TYPE_TRANSLATE(src_type,des_type) \
	(des_type)(src_type)

#define DECLARE_PREDEFINED_OBJECT(objname) \
	struct objname;

//
//The following object is used to create object by object manager.
//
BEGIN_DEFINE_OBJECT(__OBJECT_INIT_DATA)
	DWORD               dwObjectType;
	DWORD               dwObjectSize;
	BOOL                (*Initialize)(__COMMON_OBJECT*);
	VOID                (*Uninitialize)(__COMMON_OBJECT*);
END_DEFINE_OBJECT(__OBJECT_INIT_DATA)

//
//The following macros are used to form the initialization data used by object
//manager.
//When you define a new object type,please add one line in objmgr.cpp,using the
//macro OBJECT_INIT_DATA,the parameters are:
// objtype :  Object type value.
// objsize :  Object size.
// init    :  Initialization routine.
// uninit  :  Uninitialization routine.
//

#define BEGIN_DECLARE_INIT_DATA(name)            \
	static __OBJECT_INIT_DATA name[] = \
	{

#define OBJECT_INIT_DATA(objtype,objsize,init,uninit)    \
	{objtype,objsize,init,uninit},

#define END_DECLARE_INIT_DATA() \
	{0,0,0,0}                   \
	};


//
//The following object is used by Object Manager to manage objects.
//

BEGIN_DEFINE_OBJECT(__OBJECT_LIST_HEADER)
    DWORD              dwObjectNum;
    DWORD              dwMaxObjectID;
	__COMMON_OBJECT*   lpFirstObject;
END_DEFINE_OBJECT(__OBJECT_LIST_HEADER)


//
//The following object are Object Manager.
//

#define MAX_OBJECT_TYPE    64         //The maximal types in this version.

BEGIN_DEFINE_OBJECT(__OBJECT_MANAGER)
    DWORD                        dwCurrentObjectID;
	__OBJECT_LIST_HEADER         ObjectListHeader[MAX_OBJECT_TYPE];
	__COMMON_OBJECT*             (*CreateObject)(struct tag__OBJECT_MANAGER*,__COMMON_OBJECT*,DWORD);
	__COMMON_OBJECT*             (*GetObjectByID)(struct tag__OBJECT_MANAGER*,DWORD);
	__COMMON_OBJECT*             (*GetFirstObjectByType)(struct tag__OBJECT_MANAGER*,DWORD);
	VOID                         (*DestroyObject)(struct tag__OBJECT_MANAGER*,__COMMON_OBJECT*);
END_DEFINE_OBJECT(__OBJECT_MANAGER)

//
//************************************************************************
//************************************************************************
//************************************************************************
//The declare of ObjectManager,the first global object of Hello China!
//

extern __OBJECT_MANAGER ObjectManager;


//
//The following are object types's definition.
//For each object in this system,it must have a type value assocated with it.
//

#define OBJECT_TYPE_DRIVER                  0x00000001
#define OBJECT_TYPE_FILE                    0x00000002
#define OBJECT_TYPE_VOLUME                  0x00000003
#define OBJECT_TYPE_FAT32                   0x00000004
#define OBJECT_TYPE_NTFS                    0x00000005
#define OBJECT_TYPE_KERNAL_THREAD           0x00000006
#define OBJECT_TYPE_PROCESS                 0x00000007
#define OBJECT_TYPE_THREAD                  0x00000008
#define OBJECT_TYPE_MUTEX                   0x00000009
#define OBJECT_TYPE_EVENT                   0x0000000A
#define OBJECT_TYPE_CRITICAL_SECTION        0x0000000B
#define OBJECT_TYPE_VMA                     0x0000000C
#define OBJECT_TYPE_DLL                     0x0000000D
#define OBJECT_TYPE_IMAGE                   0x0000000E
#define OBJECT_TYPE_PRIORITY_QUEUE          0x0000000F
#define OBJECT_TYPE_FIFO_QUEUE              0x00000010
#define OBJECT_TYPE_KERNEL_THREAD           0x00000011
#define OBJECT_TYPE_TIMER                   0x00000012
#define OBJECT_TYPE_INTERRUPT               0x00000013
#define OBJECT_TYPE_SEMAPHORE               0x00000014
#define OBJECT_TYPE_DEVICE                  0x00000015
#define OBJECT_TYPE_DRCB                    0x00000016
#define OBJECT_TYPE_MAILBOX                 0x00000017
#define OBJECT_TYPE_PAGE_INDEX_MANAGER      0x00000018
#define OBJECT_TYPE_VIRTUAL_MEMORY_MANAGER  0x00000019
#define OBJECT_TYPE_COMMON_QUEUE            0x0000001A
#define OBJECT_TYPE_RING_BUFFER             0x0000001B
#define OBJECT_TYPE_CONDITION               0x0000001C

// Please define your object type here,and increment the type value.
//The maximal object type value should not exceed 63.

#ifdef __cplusplus
}
#endif

#endif //End of commobj.h
