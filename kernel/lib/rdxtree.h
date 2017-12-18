//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Oct 22,2017
//    Module Name               : rdxtree.h
//    Module Funciton           : 
//                                Data structures,constants,and other definitions of
//                                radix tree,which is a basic data structure that can be used
//                                to accelerate searching,by given a index(key)
//                                value.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __RDXTREE_H__
#define __RDXTREE_H__

/* Memory allocator for radix tree,to simplify debugging. */
void* __rt_alloc(size_t sz);
void __rt_free(void* ptr);

//-------------------------------------------------------------------------
/* OS simulator. */
#define __ALLOCATE_MEMORY(sz) __rt_alloc(sz)
#define __FREE_MEMORY(ptr) __rt_free(ptr)

#define __RT_DEBUG

/* 
 * Slot number of a radix tree node,it's must be square of 2,so we use
 * bit shift to get it.
 */
#define RADIX_TREE_SLOT_SHIFT 4
#define RADIX_TREE_SLOT_SIZE (1 << RADIX_TREE_SLOT_SHIFT)
#define RADIX_TREE_SLOT_MASK (RADIX_TREE_SLOT_SIZE - 1)

/* Set the upper bound of tree node's slot number. */
#if (RADIX_TREE_SLOT_SHIFT > 7)
#error "Radix tree slot shift too large,must less than or equal to 6."
#endif

/* Tree index's bit number. */
#define RADIX_TREE_INDEX_BITS 32

/* Maximal height of a radix tree,given it's index bits and node slot shift. */
#if (0 == (RADIX_TREE_INDEX_BITS % RADIX_TREE_SLOT_SHIFT))
#define RADIX_TREE_MAX_HEIGHT (RADIX_TREE_INDEX_BITS / RADIX_TREE_SLOT_SHIFT)
#else
#define RADIX_TREE_MAX_HEIGHT (RADIX_TREE_INDEX_BITS / RADIX_TREE_SLOT_SHIFT + 1)
#endif

/* Radix tree node,each slot points to a low level node or item. */
typedef struct tag__RADIX_TREE_NODE{
	unsigned int height; /* Node's height in tree. */
	int count; /* How many slot in the node. */
	void* slot[RADIX_TREE_SLOT_SIZE];
	/* 
	 * Slot map,each bit corresponds to one slot,0 means the slot
	 * is not used or point to an item,1 means the slot is point to
	 * a tree node.
	 */
#if (RADIX_TREE_SLOT_SIZE <= 32)
	unsigned long slot_map : RADIX_TREE_SLOT_SIZE;
#else /* Use 64 bit integer. */
	unsigned long long slot_map : RADIX_TREE_SLOT_SIZE;
#endif

	/* Point to parent node,used in backtrack process. */
	struct tag__RADIX_TREE_NODE* pParent;

	/*
	 * All nodes are linked into a bi-direction link list, 
	 * make it easy to destroy the whole tree.
	 */
	struct tag__RADIX_TREE_NODE* pPrev;
	struct tag__RADIX_TREE_NODE* pNext;
}__RADIX_TREE_NODE;

/* Radix tree object. */
typedef struct tag__RADIX_TREE{
	__RADIX_TREE_NODE* pChild; /* Tree root. */
	__RADIX_TREE_NODE  NodeList; /* All tree node(s). */
	unsigned long childIsNode : 1; /* First child is a tree node or an item. */
	unsigned long hasDefault : 1; /* If has default route. */
	void* defaultItem; /* Contains the item if hasDefault is 1. */
	unsigned int tree_height;

	/* Lock of this tree,synchronize accessing. */
	HANDLE lock;

	/* Object signature. */
	unsigned long signature;

	/* Radix tree operations. */
	int (*Insert)(struct tag__RADIX_TREE* pTree, unsigned long index, void* item);
	int (*ExtendHeight)(struct tag__RADIX_TREE* pTree, unsigned long index);
	void* (*Lookup)(struct tag__RADIX_TREE* pTree, unsigned long index);
	void* (*Search_Best)(struct tag__RADIX_TREE* pTree, unsigned long index, int* pMatched_pf);
	void* (*Delete)(struct tag__RADIX_TREE* pTree, unsigned long index);
}__RADIX_TREE;

/* Create or destroy a radix tree. */
__RADIX_TREE* CreateRadixTree();
void DestroyRadixTree(__RADIX_TREE* pTree);
void ShowRadixTree(__RADIX_TREE* pTree);

#endif //__RDXTREE_H__
