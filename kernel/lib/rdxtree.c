//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Oct 22,2017
//    Module Name               : rdxtree.c
//    Module Funciton           : 
//                                Implementation code of radix tree,which
//                                is a basic data structure that can be used
//                                to accelerate searching,by given a index(key)
//                                value.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#include <StdAfx.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "rdxtree.h"

/* 
 * Calculate the maximal index value that a tree can contain 
 * given a tree height. 
 */
static unsigned long GetMaxIndex(int height)
{
	unsigned long delta = 0;
	unsigned long index = 0;

	while (height)
	{
		height--;
		delta = RADIX_TREE_SLOT_MASK;
		delta <<= (RADIX_TREE_SLOT_SHIFT * height);
		index += delta;
	}
	return index;
}

/* Insert a new node and it's associated item into radix tree. */
static int __InsertIntoTree(__RADIX_TREE* pTree, unsigned long index, void* item)
{
	__RADIX_TREE_NODE* pNode = NULL;
	__RADIX_TREE_NODE* slot = NULL;
	unsigned int height, shift;
	int offset, error;

	/* Tree object and node item must be specified. */
	BUG_ON(NULL == pTree);
	BUG_ON(NULL == item);
	
	if (index > GetMaxIndex(pTree->tree_height))
	{
		//Should extend the tree height.
		__RT_DEBUG("%s:try to extend tree height.\r\n", __FUNCTION__);
		error = pTree->ExtendHeight(pTree, index);
		if (error)
		{
			return error;
		}
	}

	__RT_DEBUG("%s:tree height = %d.\r\n", __FUNCTION__, pTree->tree_height);
	/* Set default indicator. */
	if (0 == index)
	{
		pTree->hasDefault = 1;
		pTree->defaultItem = item;
	}

	slot = pTree->pChild;
	height = pTree->tree_height;
	shift = (height - 1) * RADIX_TREE_SLOT_SHIFT;
	offset = 0;
	while (height > 0)
	{
		if (NULL == slot)
		{
			slot = (__RADIX_TREE_NODE*)__ALLOCATE_MEMORY(sizeof(__RADIX_TREE_NODE));
			if (NULL == slot)
			{
				return -ENOMEM;
			}
			memset(slot, 0, sizeof(__RADIX_TREE_NODE));
			__RT_DEBUG("%s:a new tree node is created.\r\n", __FUNCTION__);
			/* Save the new node to node list. */
			slot->pNext = pTree->NodeList.pNext;
			slot->pPrev = &pTree->NodeList;
			pTree->NodeList.pNext->pPrev = slot;
			pTree->NodeList.pNext = slot;

			slot->height = height;
			if (NULL != pNode)
			{
				pNode->count++;
				pNode->slot[offset] = slot;
				pNode->slot_map |= (1 << offset);
				slot->pParent = pNode;
			}
			else //slot should be tree root.
			{
				pTree->pChild = slot;
				slot->pParent = NULL;
				pTree->childIsNode = 1;
			}
		}
		offset = (index >> shift) & RADIX_TREE_SLOT_MASK;
		pNode = slot;
		slot = pNode->slot[offset];
		shift -= RADIX_TREE_SLOT_SHIFT;
		height--;
	}

	/* The index already exist. */
	if (NULL != slot)
	{
		return -EEXIST;
	}

	if (pNode) /* Leaf node. */
	{
		pNode->count++;
		pNode->slot[offset] = item;
		pNode->slot_map &= ~(1 << offset); /* The slot contains an item. */
	}
	else /* Tree root. */
	{
		pTree->pChild = item;
		pTree->childIsNode = 0; /* First child is a leaf. */
	}
	return 0;
}

/* Protected version. */
static int InsertIntoTree(__RADIX_TREE* pTree, unsigned long index, void* item)
{
	int ret = -1;

	BUG_ON(NULL == pTree);
	BUG_ON(NULL == pTree->lock);
	WaitForThisObject(pTree->lock);
	ret = __InsertIntoTree(pTree, index, item);
	ReleaseMutex(pTree->lock);
	return ret;
}

/* Extend radix tree's height,to cover the index value. */
static int __ExtendHeight(__RADIX_TREE* pTree, unsigned long index)
{
	__RADIX_TREE_NODE* pNode = NULL;
	unsigned int height = 0;
	unsigned int new_height = 0;

	BUG_ON(NULL == pTree);

	height = pTree->tree_height;
	while (index > GetMaxIndex(height))
	{
		height++;
	}

	if (NULL == pTree->pChild) /* Tree is empty. */
	{
		pTree->tree_height = height;
		return 0;
	}

	/* Extend the tree height accordingly. */
	do{
		pNode = (__RADIX_TREE_NODE*)__ALLOCATE_MEMORY(sizeof(__RADIX_TREE_NODE));
		if (NULL == pNode)
		{
			return -ENOMEM;
		}
		memset(pNode, 0, sizeof(__RADIX_TREE_NODE));
		/* Link to node list. */
		pNode->pNext = pTree->NodeList.pNext;
		pNode->pPrev = &pTree->NodeList;
		pTree->NodeList.pNext->pPrev = pNode;
		pTree->NodeList.pNext = pNode;

		pNode->slot[0] = pTree->pChild;
		if (pTree->childIsNode)
		{
			pNode->slot_map |= (1 << 0);
		}

		/* Update Node member accordingly. */
		new_height = pTree->tree_height + 1;
		pNode->height = new_height;
		pNode->count = 1;
		pTree->pChild = pNode;
		pTree->childIsNode = 1;
		pTree->tree_height = new_height;
	} while (height > pTree->tree_height);

	return 0;
}

/* Protected version. */
static int ExtendHeight(__RADIX_TREE* pTree, unsigned long index)
{
	int ret = -1;

	BUG_ON(NULL == pTree);
	BUG_ON(NULL == pTree->lock);
	WaitForThisObject(pTree->lock);
	ret = __ExtendHeight(pTree, index);
	ReleaseMutex(pTree->lock);
	return ret;
}

/* Lookup radix tree by given an index value,return the matched item. */
static void* __LookupRadixTree(__RADIX_TREE* pTree, unsigned long index)
{
	unsigned int height = 0;
	unsigned int shift = 0;
	__RADIX_TREE_NODE* pNode = NULL;
	__RADIX_TREE_NODE* slot = NULL;
	int offset = 0;

	BUG_ON(NULL == pTree);
	pNode = pTree->pChild;
	if (NULL == pNode)
	{
		return pNode;
	}

	if (!pTree->childIsNode) /* Only one leaf. */
	{
		if (0 == index)
		{
			return pNode;
		}
		return NULL;
	}

	/* Search the whole tree. */
	height = pNode->height;
	if (index > GetMaxIndex(height))
	{
		return NULL;
	}
	shift = (height - 1) * RADIX_TREE_SLOT_SHIFT;
	do{
		offset = (index >> shift) & RADIX_TREE_SLOT_MASK;
		slot = pNode->slot[offset];
		if (NULL == slot)
		{
			return NULL;
		}
		pNode = slot;
		shift -= RADIX_TREE_SLOT_SHIFT;
		height -= 1;
	} while (height > 0);

	return pNode;
}

/* Protectd version. */
static void* LookupRadixTree(__RADIX_TREE* pTree, unsigned long index)
{
	void* ret = NULL;

	BUG_ON(NULL == pTree);
	BUG_ON(NULL == pTree->lock);
	WaitForThisObject(pTree->lock);
	ret = __LookupRadixTree(pTree, index);
	ReleaseMutex(pTree->lock);
	return ret;
}

/* Return the longest matched item given an index value. */
static void* __Search_Best_Local(__RADIX_TREE* pTree, unsigned long _index, int* pmatched_pf)
{
	unsigned int height = 0, shift = 0;
	__RADIX_TREE_NODE* pNode = NULL, *slot = NULL;
	int offset = 0;
	unsigned long index = _index;
	int matched_pf = 0; /* Prefix length that has matched. */
	/* The last matched prefix length,consider after back tracking. */
	int last_matched_pf = RADIX_TREE_INDEX_BITS; 
	int try_again = 0;

	BUG_ON(NULL == pTree);
	BUG_ON(NULL == pmatched_pf);

	pNode = pTree->pChild;
	if (NULL == pNode)
	{
		*pmatched_pf = 0;
		return NULL;
	}
	if (!pTree->childIsNode) /* Only one leaf item. */
	{
		if (index != 0)
		{
			*pmatched_pf = 0;
			return NULL;
		}
		*pmatched_pf = last_matched_pf;
		return pNode;
	}

	height = pNode->height;
	if (index > GetMaxIndex(height))
	{
		*pmatched_pf = 0;
		return NULL;
	}

	/* Search the whole tree. */
	shift = (height - 1) * RADIX_TREE_SLOT_SHIFT;
	do{
		offset = (index >> shift) & RADIX_TREE_SLOT_MASK;
		slot = pNode->slot[offset];
		if (NULL == slot)
		{
			/* Try to backtrack. */
			if (height < pTree->pChild->height)
			{
				while (0 == ((index >> shift) & RADIX_TREE_SLOT_MASK))
				{
					height++;
					shift += RADIX_TREE_SLOT_SHIFT;
					if (NULL == pNode->pParent) /* Reach the top of tree. */
					{
						__RT_DEBUG("%s:reach the top of tree,height = %d.\r\n", __FUNCTION__, height);
						break;
					}
					pNode = pNode->pParent;
				}
				if (0 == matched_pf)
				{
					index = 0;
				}
				else
				{
					index &= ~((1 << (RADIX_TREE_INDEX_BITS - matched_pf)) - 1);
				}
				last_matched_pf = matched_pf;
				__RT_DEBUG("%s:back track,height = %d,matched_pf = %d,index = 0x%X,pNode = 0x%X.\r\n",
					__FUNCTION__,
					height,
					matched_pf,
					index,
					pNode);
				continue;
			}
			else  /* Can not find item at top node. */
			{
				__RT_DEBUG("%s:last matched pf_len = 0.\r\n", __FUNCTION__);
				*pmatched_pf = 0;
				if (pTree->hasDefault) /* Use default route. */
				{
					return pTree->defaultItem;
				}
				else
				{
					return NULL;
				}
			}
		}
		pNode = slot;
		shift -= RADIX_TREE_SLOT_SHIFT;
		height -= 1;
		matched_pf += RADIX_TREE_SLOT_SHIFT;
	} while (height > 0);

	__RT_DEBUG("%s:last matched pfx_len = %d.\r\n",
		__FUNCTION__,
		last_matched_pf);
	*pmatched_pf = last_matched_pf;
	return pNode;
}

/* Protected version. */
static void* Search_Best_Local(__RADIX_TREE* pTree, unsigned long _index, int* pmatched_pf)
{
	void* ret = NULL;

	BUG_ON(NULL == pTree);
	BUG_ON(NULL == pTree->lock);
	WaitForThisObject(pTree->lock);
	ret = __Search_Best_Local(pTree, _index, pmatched_pf);
	ReleaseMutex(pTree->lock);
	return ret;
}

/* Shrink radix tree's height. */
static void __RadixTreeShrink(__RADIX_TREE* pTree)
{
	__RADIX_TREE_NODE* pToFree = NULL;
	/* Point to the first item of the node to be freed. */
	void* pNewItem = NULL;

	BUG_ON(NULL == pTree);
	while (pTree->tree_height > 0)
	{
		pToFree = pTree->pChild;
		/* Can not shrink if the node has more than one item. */
		if (pToFree->count > 1)
		{
			break;
		}
		/* Can not shrink if the only item is not in left most position. */
		if (NULL == pToFree->slot[0])
		{
			break;
		}

		/* Save the only left most item. */
		pNewItem = pToFree->slot[0];
		if (pTree->tree_height > 1)
		{
			pTree->pChild = pNewItem;
		}
		else
		{
			pTree->pChild = pNewItem;
			pTree->childIsNode = 0;
		}
		pTree->tree_height--;
		/* Unlink it from node list,and release the node. */
		pToFree->pPrev->pNext = pToFree->pNext;
		pToFree->pNext->pPrev = pToFree->pPrev;
		__FREE_MEMORY(pToFree);
	}
}

/* Protected version. */
static void RadixTreeShrink(__RADIX_TREE* pTree)
{
	BUG_ON(NULL == pTree);
	BUG_ON(NULL == pTree->lock);
	WaitForThisObject(pTree->lock);
	__RadixTreeShrink(pTree);
	ReleaseMutex(pTree->lock);
}

/* Local helper structure to save tree path. */
typedef struct tag__RADIX_TREE_PATH{
	__RADIX_TREE_NODE* pNode;
	int offset;
}__RADIX_TREE_PATH;

/* Delete a given index from radix tree,and return the associated item. */
static void* __RadixTreeDelete(__RADIX_TREE* pTree, unsigned long index)
{
	__RADIX_TREE_NODE* slot = NULL;
	__RADIX_TREE_NODE* pToFree = NULL;
	__RADIX_TREE_PATH node_path[RADIX_TREE_MAX_HEIGHT + 1];
	__RADIX_TREE_PATH* pPath = node_path;
	int shift = 0, height = 0, offset = 0;

	BUG_ON(NULL == pTree);
	height = pTree->tree_height;
	if (index > GetMaxIndex(height))
	{
		goto __TERMINAL;
	}
	slot = pTree->pChild;
	if (0 == height) /* Root node is item. */
	{
		BUG_ON(pTree->childIsNode);
		pTree->pChild = NULL;
		goto __TERMINAL;
	}

	shift = (height - 1) * RADIX_TREE_SLOT_SHIFT;
	pPath->pNode = NULL;
	pPath->offset = 0;

	/* 
	 * Save the path from tree root to the target node
	 * into node_path array.
	 */
	do{
		if (NULL == slot) /* Index is not in tree. */
		{
			goto __TERMINAL;
		}
		pPath++;
		offset = (index >> shift) & RADIX_TREE_SLOT_MASK;
		pPath->offset = offset;
		pPath->pNode = slot;
		slot = slot->slot[offset];
		shift -= RADIX_TREE_SLOT_SHIFT;
		height--;
	} while (height > 0);

	if (NULL == slot)
	{
		goto __TERMINAL;
	}

	/*
	 * Release the tree node(s) without any child
	 * alongside the tree path.
	 */
	pToFree = NULL;
	while (pPath->pNode)
	{
		pPath->pNode->slot[pPath->offset] = NULL;
		pPath->pNode->count--;
		pPath->pNode->slot_map &= ~(1 << offset);
		/*
		 * The parent link to the node that will be freed
		 * is reset when reach here,so just release it.
		 * Unlink it from node list before free.
		 */
		if (pToFree)
		{
			pToFree->pPrev->pNext = pToFree->pNext;
			pToFree->pNext->pPrev = pToFree->pPrev;
			__FREE_MEMORY(pToFree);
		}
		if (pPath->pNode->count > 0)
		{
			if (pPath->pNode == pTree->pChild)
			{
				//Shrink the tree.
				RadixTreeShrink(pTree);
			}
			goto __TERMINAL;
		}
		/* Node child in this node,release it. */
		pToFree = pPath->pNode;
		pPath--;
	}

	/* Reach the root of tree. */
	pTree->tree_height = 0;
	pTree->pChild = NULL;
	pTree->childIsNode = 0;
	if (pToFree)
	{
		pToFree->pPrev->pNext = pToFree->pNext;
		pToFree->pNext->pPrev = pToFree->pPrev;
		__FREE_MEMORY(pToFree);
	}

__TERMINAL:
	return slot;
}

/* Protected version. */
static void* RadixTreeDelete(__RADIX_TREE* pTree, unsigned long index)
{
	void* ret = NULL;

	BUG_ON(NULL == pTree);
	BUG_ON(NULL == pTree->lock);
	WaitForThisObject(pTree->lock);
	ret = __RadixTreeDelete(pTree, index);
	ReleaseMutex(pTree->lock);
	return ret;
}

/* Create a new radix tree. */
__RADIX_TREE* CreateRadixTree()
{
	__RADIX_TREE* pTree = NULL;
	BOOL bResult = FALSE;

	pTree = (__RADIX_TREE*)__ALLOCATE_MEMORY(sizeof(__RADIX_TREE));
	if (NULL == pTree)
	{
		goto __TERMINAL;
	}
	memset(pTree, 0, sizeof(__RADIX_TREE));

	/* Create access lock. */
	pTree->lock = CreateMutex();
	if (NULL == pTree->lock)
	{
		goto __TERMINAL;
	}

	pTree->pChild = NULL;
	pTree->childIsNode = 0;
	pTree->hasDefault = 0;
	pTree->defaultItem = NULL;
	pTree->signature = KERNEL_OBJECT_SIGNATURE;
	pTree->tree_height = 0;
	/* Set operation functions. */
	pTree->Insert = InsertIntoTree;
	pTree->ExtendHeight = ExtendHeight;
	pTree->Lookup = LookupRadixTree;
	pTree->Search_Best = Search_Best_Local;
	pTree->Delete = RadixTreeDelete;

	/* Initializes the tree node list. */
	pTree->NodeList.pPrev = pTree->NodeList.pNext = &pTree->NodeList;

	/* Mark as success. */
	bResult = TRUE;

__TERMINAL:
	if (!bResult)
	{
		if (pTree)
		{
			if (pTree->lock)
			{
				DestroyMutex(pTree->lock);
			}
			__FREE_MEMORY(pTree);
			pTree = NULL;
		}
	}
	return pTree;
}

/* Destroy a radix tree.Use a simplified method. */
void DestroyRadixTree(__RADIX_TREE* pTree)
{
	__RADIX_TREE_NODE* pNode = NULL;
	__RADIX_TREE_NODE* pNodeTmp = NULL;

	if (NULL == pTree)
	{
		return;
	}
	BUG_ON(KERNEL_OBJECT_SIGNATURE != pTree->signature);

	/* Delete all node list in tree. */
	pNode = pTree->NodeList.pNext;
	while (pNode != &pTree->NodeList)
	{
		pNodeTmp = pNode;
		pNode = pNode->pNext;
		/* 
		* No need do unlink operation,since the whole 
		* node list will be destroyed.
		*/
		__FREE_MEMORY(pNodeTmp);
	}

	BUG_ON(NULL == pTree->lock);
	DestroyMutex(pTree->lock);
	__FREE_MEMORY(pTree);
	return;
}

//------------------------------------------------------------------------
/* Test and helper routines. */
static size_t totalMemory = 0;
void* __rt_alloc(size_t sz)
{
	char* ptr = (char*)_hx_malloc(sz + sizeof(size_t));
	if (ptr)
	{
		*(size_t*)ptr = sz;
		ptr += sizeof(size_t);
		totalMemory += sz;
	}
	return ptr;
}

void __rt_free(void* ptr)
{
	char* memptr = ptr;
	if (memptr)
	{
		memptr -= sizeof(size_t);
		totalMemory -= *(size_t*)memptr;
	}
	_hx_free(memptr);
}

void ShowMemory()
{
	_hx_printf("MEM: Total allocated mem size = %d.\r\n", totalMemory);
}
