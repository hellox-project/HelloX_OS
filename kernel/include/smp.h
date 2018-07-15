//***********************************************************************/
//    Author                    : Garry
//    Original Date             : July 12,2018
//    Module Name               : smp.h
//    Module Funciton           : 
//                                Symmentric Multiple Processor related constants,
//                                structures and routines.
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __SMP_H__
#define __SMP_H__

#include <stdint.h> /* For common data types. */

/* spin lock and it's operations. */
typedef volatile unsigned int __SPIN_LOCK;

#define SPIN_LOCK_VALUE_UNLOCK 0
#define SPIN_LOCK_VALUE_LOCK 1

/* Initial value is unlocked. */
#define SPIN_LOCK_INIT_VALUE SPIN_LOCK_VALUE_UNLOCK

/* 
 * Architecture specific spin lock operations,must be implemented in
 * arch specific source code.
 * These routines must not be called directly except and only except the
 * macros follows.
 */
void __raw_spin_lock(__SPIN_LOCK* sl);
void __raw_spin_unlock(__SPIN_LOCK* sl);

/* 
 * Critical section operations under SMP,these routines must be 
 * implemented in arch specific source code.
 * These routines also must not be called directly.
 */
unsigned long __smp_enter_critical_section(__SPIN_LOCK* sl);
unsigned long __smp_leave_critical_section(__SPIN_LOCK* sl, unsigned long dwFlags);

/* User should use these macros to manipulate the critical section. */
#define __ENTER_CRITICAL_SECTION_SMP(spin_lock,dwFlags) \
    dwFlags = __smp_enter_critical_section(&(spin_lock))
#define __LEAVE_CRITICAL_SECTION_SMP(spin_lock,dwFlags) \
    dwFlags = __smp_leave_critical_section(&(spin_lock),dwFlags)

/* 
 * Macros to operate the spin lock.
 * Declare and initialize a spin lock.
 */
#define __DECLARE_SPIN_LOCK(sl_name) __SPIN_LOCK sl_name = SPIN_LOCK_INIT_VALUE
#define __INIT_SPIN_LOCK(sl_name) sl_name = SPIN_LOCK_INIT_VALUE

/* Acquire and release the specified spin lock. */
#define __ACQUIRE_SPIN_LOCK(sl_name) __raw_spin_lock(&sl_name)
#define __RELEASE_SPIN_LOCK(sl_name) __raw_spin_unlock(&sl_name)

/*
 * All processors in system are organized as follows :
 * 1. At least one domain in system, and there may be several domains in NUMA architecture;
 * 2. At least one chip in each domain, chip is the physical package of CPU;
 * 3. At least one core in each chip, there maybe several cores in each chip, in case of multiple core;
 * 4. At least one logical CPU in each core.The logical CPU corresponding the hyper thread.
 * Each kernel thread will be scheduled to one logical CPU, or core directly in case of no hyper thread,
 * so the logical CPU is the mininum unit of scheduling, we call this unit as PROCESSOR.
 * One logical CPU is used to delegate the core,in case of no hyper thread is supported by chip.
 * This processor hierarchy is managed by a tree like structure, that there is one domain list to link
 * all domains together, and there is a child list in each domain, in each chip under a domain, in each
 * core under a chip.
 * A dedicated structure,processor node,is defined to manage each node in domain list and it's sub level
 * nodes,as follows:
 */
typedef struct tag__PROCESSOR_NODE {
	uint8_t nodeType;
	uint8_t nodeID;
	/* A level specific pointer,contains level and arch specific info. */
	void* pLevelSpecificPtr;
	/* Child list's head and tail. */
	struct tag__PROCESSOR_NODE* pChildHead;
	struct tag__PROCESSOR_NODE* pChildTail;
	/* Next on in same level. */
	struct tag__PROCESSOR_NODE* pNext;
}__PROCESSOR_NODE;

/* Processor node types. */
#define PROCESSOR_NODE_TYPE_DOMAIN 0  /* NUMA domain. */
#define PROCESSOR_NODE_TYPE_CHIP   1  /* Physical chip. */
#define PROCESSOR_NODE_TYPE_CORE   2  /* Core in one chip. */
#define PROCESSOR_NODE_TYPE_LCPU   3  /* Logical CPU. */

/* 
 * Processor Manager object,all processors in system,under 
 * SMP configuration,are managed by this object.
 */
typedef struct tag__PROCESSOR_MANAGER {
	/* The domain list header. */
	__PROCESSOR_NODE* pDomainList;
	/* Spin lock used to protect the domain list. */
	__SPIN_LOCK spin_lock;

	/* Process Manager offered operations. */
	BOOL (*Initialize)(struct tag__PROCESSOR_MANAGER* pMgr);
	BOOL (*AddProcessor)(uint8_t domainid, uint8_t chipid, uint8_t coreid, uint8_t lcpuid);
	/* Set chip specific information. */
	BOOL (*SetChipSpecific)(uint8_t domainid, uint8_t chipid, void* pSpec);
	/* Get chip specific information. */
	void* (*GetChipSpecific)(uint8_t domainid, uint8_t chipid);
	/* Set logical CPU specific information. */
	BOOL (*SetLogicalCPUSpecific)(uint8_t domainid, uint8_t chipid, uint8_t coreid, uint8_t lcpuid, void* pSpec);
	/* Get Logical CPU specific information. */
	void* (*GetLogicalCPUSpecific)(uint8_t domainid, uint8_t chipid, uint8_t coreid, uint8_t lcpuid);
	/* Show out CPU information. */
	VOID (*ShowCPU)(struct tag__PROCESSOR_MANAGER* pMgr);
}__PROCESSOR_MANAGER;

/* Global Processor Manager Instance,only available when SMP is enabled. */
#if defined(__CFG_SYS_SMP)
extern __PROCESSOR_MANAGER ProcessorManager;
#endif

#endif //__SMP_H__
