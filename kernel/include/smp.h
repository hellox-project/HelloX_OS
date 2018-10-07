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

#include <TYPES.H>
#include <stdint.h> /* For common data types. */

/* Get local processor's ID,it must be implemented in arch specific source code. */
unsigned int __GetProcessorID();

/* 
 * Architecture specific spin lock operations,must be implemented in
 * arch specific source code.
 * These routines must not be called directly except and only except the
 * macros follows.
 */
BOOL __raw_spin_lock(__SPIN_LOCK* sl);
void __raw_spin_unlock(__SPIN_LOCK* sl);

/* 
 * Critical section operations under SMP,these routines must be 
 * implemented in arch specific source code.
 * These routines also must not be called directly.
 */
unsigned long __smp_enter_critical_section(__SPIN_LOCK* sl);
unsigned long __smp_leave_critical_section(__SPIN_LOCK* sl, unsigned long dwFlags);

/* User should use these macros to manipulate the critical section. */
#if defined(__CFG_SYS_SMP)
#define __ENTER_CRITICAL_SECTION_SMP(spin_lock,dwFlags) \
    dwFlags = __smp_enter_critical_section(&(spin_lock)); \
	BUG_ON(0xFFFFFFFF == dwFlags)
#define __LEAVE_CRITICAL_SECTION_SMP(spin_lock,dwFlags) \
    dwFlags = __smp_leave_critical_section(&(spin_lock),dwFlags)
#else /
/* UP environment,just skip the spin lock. */
#define __ENTER_CRITICAL_SECTION_SMP(spin_lock,dwFlags) \
	__ENTER_CRITICAL_SECTION(NULL,dwFlags)
#define __LEAVE_CRITICAL_SECTION_SMP(spin_lock,dwFlags) \
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags)
#endif

/* 
 * Macros to operate the spin lock.
 * Declare and initialize a spin lock.
 */
#define __DECLARE_SPIN_LOCK(sl_name) __SPIN_LOCK sl_name = SPIN_LOCK_INIT_VALUE
/* Initialize a spin lock,name is used to debugging. */
#define __INIT_SPIN_LOCK(sl, name) sl = SPIN_LOCK_INIT_VALUE

/* Acquire and release the specified spin lock. */
#define __ACQUIRE_SPIN_LOCK(sl_name) __raw_spin_lock(&sl_name)
#define __RELEASE_SPIN_LOCK(sl_name) __raw_spin_unlock(&sl_name)

/* General memory barriers. */
#if defined(__CFG_SYS_SMP)
#define __MEMORY_BARRIER() {__asm lock add dword ptr [esp],0}
#else
#define __MEMORY_BARRIER()
#endif

/* 
 * Write and read barriers,just bounce to general memory barrier now. 
 * Maybe optimized farther.
 */
#define __READ_BARRIER() __MEMORY_BARRIER()
#define __WRITE_BARRIER() __MEMORY_BARRIER()

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
	/* How many logical CPU(Processor) in system. */
	volatile int nProcessorNum;
	/* BSP's processor ID in SMP environment. */
	unsigned int bspProcessorID;

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
	/* Get current processor ID. */
	unsigned int (*GetCurrentProcessorID)();
	/* Return current processor's specific information. */
	void* (*GetCurrentProcessorSpecific)();
	/* Get the processor number. */
	int (*GetProcessorNum)();
	/* Get a CPU id that new created kernel thread will be put on. */
	unsigned int (*GetScheduleCPU)();
	/* Show out CPU information. */
	VOID (*ShowCPU)(struct tag__PROCESSOR_MANAGER* pMgr);
}__PROCESSOR_MANAGER;

/* Check if current processor is BSP. */
#if defined(__CFG_SYS_SMP)
#define __CURRENT_PROCESSOR_IS_BSP() \
    (__CURRENT_PROCESSOR_ID == ProcessorManager.bspProcessorID)
#else
#define __CURRENT_PROCESSOR_IS_BSP() (TRUE)
#endif

/* Interrupt controller object,one corresponding to each physical controller. */
typedef struct tag__INTERRUPT_CONTROLLER {
	unsigned long objectSignature;
	uint8_t* pBase; /* Base address. */
	unsigned int id; /* Controller ID. */
	char* typeName; /* Controller type. */
	/* Supported start vector value. */
	unsigned char vectorStart;
	/* Supported end vector value. */
	unsigned char vectorEnd;
	/* Corresponding interrupt object. */
	__COMMON_OBJECT* pIntObj;

	/* Initializer of the controller object. */
	BOOL (*Initialize)(struct tag__INTERRUPT_CONTROLLER* pThis);
	/* Acknowledge interrupt. */
	BOOL (*AckInterrupt)(struct tag__INTERRUPT_CONTROLLER* pThis, unsigned int intVector);
	/* Send initial IPI. */
	BOOL (*Send_Init_IPI)(struct tag__INTERRUPT_CONTROLLER* pThis, unsigned int destination);
	/* Send Startup IPI. */
	BOOL (*Send_Start_IPI)(struct tag__INTERRUPT_CONTROLLER* pThis, unsigned int destination);
	/* Send general IPI. */
	BOOL (*Send_IPI)(struct tag__INTERRUPT_CONTROLLER* pThis, int destination, unsigned int ipiType);
}__INTERRUPT_CONTROLLER;

/* Broadcast to all interrupt controllers if destination is set to this value. */
#define INTERRUPT_DESTINATION_ALL 0xFFFFFFFF

/* Inter-Processor-Interrupt types. */
#define IPI_TYPE_NEWTHREAD 0x01
#define IPI_TYPE_TLBFLUSH  0x02

/*
* Creates and initializes an interrupt controller object,
* given it's base address,id,type,and supported vector range.
* This like a class factory of interrupt controller.
*/
__INTERRUPT_CONTROLLER* CreateInterruptController(uint8_t* pBase, unsigned int id, 
	unsigned int type,
	unsigned char rangStart,
	unsigned char rangEnd);

/* Destroy the given interrupt controller object. */
void DestroyInterruptController(__INTERRUPT_CONTROLLER* pIntCtrl);

/* 
 * Logical CPU specific information,associated with 
 * a logical CPU(processor) together to hold the processor's
 * specific information.
 */
typedef struct tag__LOGICALCPU_SPECIFIC {
	__INTERRUPT_CONTROLLER* pIntCtrl;
	/* CPU status. */
	volatile unsigned long cpuStatus;
	void* vendorSpec; /* Points to CPU vendor's specific information. */
}__LOGICALCPU_SPECIFIC;

/* CPU status value. */
#define CPU_STATUS_NORMAL  0x00
#define CPU_STATUS_HALTED  0x01

/* Get the current processor's ID,no matter in SMP or UP environment. */
#if defined(__CFG_SYS_SMP)
#define __CURRENT_PROCESSOR_ID (ProcessorManager.GetCurrentProcessorID())
#else /* UP environment. */
#define __CURRENT_PROCESSOR_ID (0)
#endif

/* Get chip id giving the processor ID. */
uint8_t __GetChipID(unsigned int processor_id);

/* Get the core ID giving the processor ID. */
uint8_t __GetCoreID(unsigned int processor_id);

/* Get the logical CPU ID giving the processor ID. */
uint8_t __GetLogicalCPUID(unsigned int processor_id);

/* Global Processor Manager Instance,only available when SMP is enabled. */
#if defined(__CFG_SYS_SMP)
extern __PROCESSOR_MANAGER ProcessorManager;
#endif

#endif //__SMP_H__
