//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Sep,03 2004
//    Module Name               : commobj.h
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

/* 
 * Ancester object for all kernel objects. 
 * Any kernel object is inherit from this object,
 * it's contains some common attributes apply
 * to all kernel objects.
 */
typedef struct tag__COMMON_OBJECT{
	DWORD dwObjectType;
	DWORD dwObjectID;
	DWORD dwObjectSize;
	/* Refer counter of this object. */
	__ATOMIC_T refCount;
	/* Points to previous one in object list. */
	struct tag__COMMON_OBJECT* lpPrevObject;
	/* Points to next one in object list. */
	struct tag__COMMON_OBJECT* lpNextObject;
	/* This object's owner object,maybe thread,process,... */
	struct tag__COMMON_OBJECT* lpObjectOwner;

	/* Initialization routine of the object,like constructor in C++. */
	BOOL (*Initialize)(struct tag__COMMON_OBJECT*);
	/* Destructor routine. */
	BOOL (*Uninitialize)(struct tag__COMMON_OBJECT*);
} __COMMON_OBJECT;

/*
 * Simulates inherit mechanism in C++, inherits all
 * attributes of common object by using this macro.
 */
#define INHERIT_FROM_COMMON_OBJECT \
	DWORD              dwObjectType;        \
	DWORD              dwObjectID;          \
	DWORD              dwObjectSize;        \
	__ATOMIC_T         refCount;            \
	__COMMON_OBJECT*   lpPrevObject;        \
	__COMMON_OBJECT*   lpNextObject;        \
	__COMMON_OBJECT*   lpObjectOwner;       \
	BOOL               (*Initialize)(__COMMON_OBJECT*);    \
	BOOL               (*Uninitialize)(__COMMON_OBJECT*);  

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

/* 
 * Describes kernel object's attributes that are 
 * used by Object Manager,when create kernel object.
 * Each kernel object corresponding to one entry,in
 * a global array defined in objmgr.c file.
 * Object Manager will create kernel object by using
 * these attributes.
 */
BEGIN_DEFINE_OBJECT(__OBJECT_INIT_DATA)
	DWORD dwObjectType;
	DWORD dwObjectSize;
	BOOL (*Initialize)(__COMMON_OBJECT*);
	BOOL (*Uninitialize)(__COMMON_OBJECT*);
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

/* 
 * Header element of kernel object list. 
 * Each kernel object created by Object Manager is
 * linked into it's list,according to it's type.
 */
BEGIN_DEFINE_OBJECT(__OBJECT_LIST_HEADER)
    DWORD dwObjectNum;
    DWORD dwMaxObjectID;
	__COMMON_OBJECT* lpFirstObject;
END_DEFINE_OBJECT(__OBJECT_LIST_HEADER)

/* Kernel object type's maximal value. */
#define MAX_KERNEL_OBJECT_TYPE 64

/* Object Manager. */
BEGIN_DEFINE_OBJECT(__OBJECT_MANAGER)
    DWORD dwCurrentObjectID;
#if defined(__CFG_SYS_SMP)
	__SPIN_LOCK spin_lock;
#endif
	/* 
	 * Kernel object's list array,each type of kernel object 
	 * corresponding one entry of this array,indexed by it's
	 * object type value.
	 */
	__OBJECT_LIST_HEADER ObjectListHeader[MAX_KERNEL_OBJECT_TYPE];

	/* Initializer of object manager. */
	BOOL (*Initialize)(struct tag__OBJECT_MANAGER*);
	/* Create a new object. */
	__COMMON_OBJECT* (*CreateObject)(struct tag__OBJECT_MANAGER*,__COMMON_OBJECT*,unsigned long);
	/* Get an object by specifying it's type and ID. */
	__COMMON_OBJECT* (*GetObject)(struct tag__OBJECT_MANAGER*, unsigned long type, unsigned long _id);
	/* Get an object by specifying it's ID. */
	__COMMON_OBJECT* (*GetObjectByID)(struct tag__OBJECT_MANAGER*, unsigned long);
	/* Get the specified type's object list header. */
	__COMMON_OBJECT* (*GetFirstObjectByType)(struct tag__OBJECT_MANAGER*, unsigned long);
	/* Destroy one kernel object. */
	BOOL (*DestroyObject)(struct tag__OBJECT_MANAGER*, __COMMON_OBJECT*);
	/* Increase kernel object's reference counter. */
	BOOL (*AddRefCount)(struct tag__OBJECT_MANAGER*, __COMMON_OBJECT*);
	/* Validates a kernel object. */
	BOOL (*ValidateObject)(struct tag__OBJECT_MANAGER*, __COMMON_OBJECT*);
	/* Show out a specified kind of kernel object's information. */
	VOID (*ShowKernelObject)(unsigned long kobj_type);
END_DEFINE_OBJECT(__OBJECT_MANAGER)

/* 
 * Object Manager object,all kernel objects are 
 * managed by this one. 
 */
extern __OBJECT_MANAGER ObjectManager;

/* Kernel objects HelloX has defined up to now. */
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

/*
 * Please define your object type here,and increment 
 * the type value.
 * The maximal object type value should not exceed 63.
 */

#endif //__COMMOBJ_H__
