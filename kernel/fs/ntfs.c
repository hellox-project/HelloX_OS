//------------------------------------------------------------------------
//  NTFS file system implementation code.
//
//  Author                 : Garry.Xin
//  Initial date           : Nov 05,2011
//  Last updated           : Nov 05,2011
//  Last updated author    : Garry.Xin
//  Last udpated content   : 
//------------------------------------------------------------------------

#ifndef __STDAFX_H__
#include <StdAfx.h>
#endif
#ifndef __NTFS_H__
#include "ntfs.h"
#endif

#ifndef __FSSTR_H__
#include "fsstr.h"
#endif

#include "../lib/stdio.h"
#include "../lib/string.h"



//This module will be available if and only if the DDF function is enabled.
#ifdef __CFG_SYS_DDF
 
//A local helper routine used to decode data run.
/* This routine can not be port to other hardware platforms directly since it assume the
target machine is little endian,i.e,for a integer length larger than 1 byte,a pointer to
this integer is pointing to it's lowest byte actually.
   All bit related operations should be consider carefully when migrate the source code
to big endian machine.
*/
static __NTFS_DATA_RUN* DecodeDataRun(BYTE* pDataRun,UINT_32* pTotalClus)
{
         BOOL                  bResult    = FALSE;
         __NTFS_DATA_RUN*      pRunList   = NULL;
         __NTFS_DATA_RUN*      pCurrNode  = NULL;
         __NTFS_DATA_RUN*      pNewNode   = NULL;
         UINT_32               offsetLength = 0;
         UINT_32               lengthLength = 0;
         UINT_32               offset = 0;
         UINT_32               length = 0;
         UINT_8                lenoff = 0;  //Length and offset byte in data run,i.e,the first byte.
         UINT_8                offup  = 0;
         UINT_32               prevOffset = 0;
         UINT_32               currOffset = 0;
                    //BYTE*                 pDebugRun  = NULL; //For debugging.
         
         if(NULL == pDataRun)
         {
                   goto __TERMINAL;
         }
                    *pTotalClus = 0;
 
         while(*pDataRun)
         {
                   lenoff = (UINT_8)(*pDataRun);
                   lengthLength = lenoff & 0x0F;  //Get data run length's length.
                   offsetLength = (lenoff >> 4) & 0x0F; //Get offset's length.
                   //Decode length's value.
                   pDataRun += 1;  //Skip the length and offset byte.
                   switch(lengthLength)
                   {
                   case 1:
                            length = *(UINT_8*)pDataRun;
                            break;
                   case 2:
                            length = *(UINT_16*)pDataRun;
                            break;
                   case 3:
                            length = *(UINT_32*)pDataRun;
                            //length <<= 8;  //Skip the upper byte.
                            //length >>= 8;  //Keep the lower 3 bytes.
                                                                 length &= 0x00FFFFFF;  //Skip the 4th byte of a UINT_32.
                            break;
                   case 4:
                            length = *(UINT_32*)pDataRun;
                            break;
                   default:
                            break;
                   }
                   //printf("Length: %d,length length: %d,",length,lengthLength); //---- debug ----
                   pDataRun += lengthLength;  //Locate to offset now.
                   offup = *(UINT_8*)(pDataRun + offsetLength - 1);  //Get the upper byte.
                   switch(offsetLength)
                   {
                   case 1:
                            if(offup >= 128)
                            {
                                     offset  = 0xFFFFFF00;
                                     offset += *(UINT_8*)pDataRun;
                            }
                            else
                            {
                                     offset = *(UINT_8*)pDataRun;
                            }
                            break;
                   case 2:
                            if(offup >= 128)
                            {
                                     offset  = 0xFFFF0000;
                                     offset += *(UINT_16*)pDataRun;
                            }
                            else
                            {
                                     offset = *(UINT_16*)pDataRun;
                            }
                            break;
                   case 3:
                            if(offup >= 128)
                            {
                                     offset = *(UINT_32*)pDataRun;
                                     offset <<= 8;  //Skip the upper byte.
                                     offset >>= 8;  //Keep the lower 3 bytes.
                                     offset |= 0xFF000000;
                            }
                            else
                            {
                                     offset = *(UINT_32*)pDataRun;
                                     offset <<= 8;  //Skip the upper tyte.
                                     offset >>= 8;  //Only keep the lower 3 bytes,pad 0 in uper byte.
                            }
                            break;
                   case 4:
                            offset = *(UINT_32*)pDataRun;
                            break;
                   default:
                            break;
                   }
                   //printf("Offset: %d-0x%0X,offset length: %d\r\n",offset,offset,offsetLength);  //---- debug ----
                   pDataRun += offsetLength;  //Point to next data run.
                   currOffset += offset;      //Please remember that the offset in data run is the relative offset
                                              //from previous data run.
 
                   pNewNode = (__NTFS_DATA_RUN*)__MEM_ALLOC(sizeof(__NTFS_DATA_RUN));
                   if(NULL == pNewNode)
                   {
                            goto __TERMINAL;
                   }
                   //Initialize the run list node.
                   pNewNode->pNext    = NULL;
                   if(NULL == pRunList)  //The first node.
                   {
                            pRunList  = pNewNode;
                            pCurrNode = pNewNode;
                            pNewNode->clusNum = 0;
                            pNewNode->startClusLow  = 0;
                            pNewNode->startClusHigh = 0;
                   }
                   else //Should link it to run list.
                   {
                            pCurrNode->pNext = pNewNode;
                   }
                   //Update new node's data members.
                   pNewNode->clusNum       = length;
                                        *pTotalClus             += length;
                   pNewNode->startClusLow  = currOffset;
                   pCurrNode = pNewNode;
         }
         bResult = TRUE;
 
__TERMINAL:
         if(!bResult)
         {
                   pCurrNode = pRunList;
                   while(pCurrNode)  //Should release memories.
                   {
                            pNewNode  = pCurrNode;
                            pCurrNode = pCurrNode->pNext;
                            __MEM_FREE(pNewNode);
                   }
                   pRunList = NULL;
         }
         return pRunList;
}
 
//A local helper routine to restore 2 bytes content in the end of a sector by
//replace it using data in file record header,since these 2 bytes is written
//an update serial number by NTFS driver.
static BOOL RestoreSectContent(__NTFS_FILE_SYSTEM* pFileSystem,BYTE* pFileRecord)
{
         UINT_16            usn;
         UINT_16*           pUSA          = NULL;  //Update serial array.
         BYTE*              pUSNOff       = NULL;
         UINT_32            sectnum;
         UINT_32            i;
 
         if((NULL == pFileSystem) || (NULL == pFileRecord))
         {
                   return FALSE;
         }
         pUSNOff =  pFileRecord;
         pUSNOff += FR_USN_OFFSET(pFileRecord);
         usn     =  *(UINT_16*)pUSNOff;
         pUSNOff += sizeof(UINT_16);  //pUSNOff now pointing to the actual content of sectors.
         pUSA    = (UINT_16*)pUSNOff;
         pUSNOff = pFileRecord;
         //Calculate how many sectors one file record occupies.
         //sectnum = FR_SIZE_ALLOC(pFileRecord) / pFileSystem->bytesPerSector;
         sectnum = FR_USN_SIZE(pFileRecord);
         //Update all sector's last 2 bytes.But the verification should be done to check if
         //the sector's content is valid before update.
         for(i = 0;i < sectnum;i ++)
         {
                   pUSNOff = pFileRecord + pFileSystem->bytesPerSector * (i + 1);
                   pUSNOff -= sizeof(UINT_16);
                   if(usn != *(UINT_16*)pUSNOff)  //Invalid sector.
                   {
                            return FALSE;
                   }
                   //OK,sector's content is correct,so update it.
                   *(UINT_16*)pUSNOff = pUSA[i];
         }
         return TRUE;
}
 
//A helper routine,to locate a specific file attribute from file record,
//and return the offset.
static BYTE* GetFileAttribute(__NTFS_FILE_SYSTEM* pFileSystem,BYTE* pFileRecord,UINT_32 attr)
{
         BYTE*    pAttrOffset      = pFileRecord;
         UINT_32  FrSize           = 0;
         UINT_32  visitedSize      = 0;
         BYTE*    pFoundAttr       = NULL;
         int      i;
         static   BYTE ntfsSig[]   = {'F','I','L','E'};
 
         if((NULL == pFileRecord) || (0 == attr) || (NULL == pFileSystem))
         {
                   return NULL;
         }
         //Check if the file record is a valid one.
         for(i = 0;i < 4;i ++)
         {
                   if(*(pFileRecord + i) != ntfsSig[i])
                   {
                            return NULL;
                   }
         }
         //printf("GetFileAttribute: attr = 0x%02X\r\n",attr);
         pAttrOffset += FR_ATTR_OFFSET(pFileRecord);
         FrSize += FR_ATTR_OFFSET(pFileRecord);
         while(FrSize < FR_SIZE(pFileRecord))
         {
                   if(RA_TYPE(pAttrOffset) == attr)  //Find the desired attribute.
                   {
                            pFoundAttr = pAttrOffset;
                            //return pAttrOffset;
                   }
                   //Check if we reach the end of FR.
                   if(*(UINT_32*)pAttrOffset == 0xFFFFFFFF)
                   {
                            break;
                   }
                   FrSize      += RA_SIZE(pAttrOffset);
                   pAttrOffset += RA_SIZE(pAttrOffset);
         }
         return pFoundAttr;
}
 
//A helper routine that retrieve non-residential attribute's data,i.e,skip
//the file attribute's header and return the start address of attribute data.
static BYTE* GetDataRunDesc(BYTE* pDataAttr)
{
         if(NULL == pDataAttr)
         {
                   return NULL;
         }
 
         if(NRA_NO_RA_FLAG(pDataAttr) == 1)  //No residential attribute.
         {
                   return (pDataAttr + NRA_DRUN_OFFSET(pDataAttr));
         }
         else
         {
                   return NULL;
         }
}
 
//A helper routine to obtain residential file attribute's data information from file
//attribute,i.e,skip the attribute's header and return the meaningful data.
static BYTE* GetAttributeData(BYTE* pFileAttr)
{
         if(NULL == pFileAttr)
         {
                   return NULL;
         }
         return (pFileAttr + RA_ADATA_OFFSET(pFileAttr));
}
 
//A helper routine to decode file's data attribute.
static BOOL ParseDataAttr(__NTFS_FILE_OBJECT* pFileObject,BYTE* pDataAttr)
{
         __NTFS_DATA_RUN*      pRunList   = NULL;
         UINT_32               raDataSize = 0;
         BYTE*                 pAttrOffset = NULL;
         
         if((NULL == pFileObject) || (NULL == pDataAttr))
         {
                   return FALSE;
         }
         //Check if the data attribute is residential attribute.
         if(0 == RA_NO_RA_FLAG(pDataAttr))
         {
                   pAttrOffset = GetAttributeData(pDataAttr);
                   raDataSize  = RA_ADATA_SIZE(pDataAttr);
                   memcpy(pFileObject->pFileBuffer,pAttrOffset,raDataSize);  //Read the file's content directly.
                   pFileObject->buffStartVCN  = 0;
                   pFileObject->fileBuffContentSize = raDataSize;
 
                   //Initialize file size variables.
                   pFileObject->fileSizeHigh   = 0;
                   pFileObject->fileSizeLow    = raDataSize;
                   pFileObject->fileAllocSize  = raDataSize;
                   pFileObject->totalCluster   = 0;
                   return TRUE;
         }
         else //No-residential,should decode data run.
         {
                   pAttrOffset = GetDataRunDesc(pDataAttr);
                   if(!pAttrOffset)  //Exception.
                   {
                            return FALSE;
                   }
                   //Decode data run.
                   pRunList = DecodeDataRun(pAttrOffset,&pFileObject->totalCluster);
                   if(NULL == pRunList)  //Exception.
                   {
                            return FALSE;
                   }
                   //OK,set the data run list of file object.
                   pFileObject->pRunList = pRunList;
                   //Update the file's size attribute.The common operation to set the file's size
                   //is analyze the FILE_NAME attribute,but this attribute's file size value may
                   //invalid in some special case,so we assume one file only has one DATA attribute
                   //and set the file's size as DATA attribute's length.
                   //This implementation may not cover all scenarios,but in most case it should be 
                   //OK.
                   //Since lack standard information about NTFS,hereby glitches in very special case
                   //is tolerable.
                   pFileObject->fileAllocSize = NRA_ALLOC_SIZE(pDataAttr);
                   pFileObject->fileSizeLow   = NRA_TRUE_SIZE(pDataAttr);
                   pFileObject->fileSizeHigh  = 0;  //Does not support yet.
                   return TRUE;
         }
}
 
//A local helper routine to process FILE_NAME attribute.
static BOOL ParseFileNameAttr(__NTFS_FILE_SYSTEM* pFileSystem,__NTFS_FILE_OBJECT* pFileObject,BYTE* pFnAttr)
{
         BYTE*   pAttrData = NULL;
         WCHAR*  pNameOff  = NULL;
		 int     i         = 0;
 
         if((NULL == pFileSystem) || (NULL == pFileObject) || (NULL == pFnAttr))
         {
                   return FALSE;
         }
         pAttrData = GetAttributeData(pFnAttr);
         if(NULL == pAttrData)
         {
                   return FALSE;
         }
         pFileObject->fileAttr = FN_DOS_ATTR(pAttrData);
         pFileObject->fileAllocSize = FN_ALLOC_SIZE(pAttrData);
         pFileObject->fileSizeLow = FN_VALID_SIZE(pAttrData);
         pFileObject->fileSizeHigh = 0;  //Not supported yew.
 
         //Process file name.
         pFileObject->fileNameSize = FN_NAME_SIZE(pAttrData);
         pNameOff = (WCHAR*)(pAttrData + FN_NAME_OFFSET);
         for(i = 0;i < FN_NAME_SIZE(pAttrData);i ++)
         {
                   pFileObject->FileName[i] = pNameOff[i];
         }
         pFileObject->FileName[i] = 0;
         return TRUE;
}
 
//Parse INDEX ROOT and INDEX ALLOCATION attributes,specific for directory.
//This routine parses INDEX ROOT attribute first,FALSE will be returned if no IR attribute in
//file record.The INDEX ALLOCATION will be parsed.
static BOOL ParseIndexAttr(__NTFS_FILE_SYSTEM* pFileSystem,__NTFS_FILE_OBJECT* pFileObject,BYTE* pFileRecord)
{
         BOOL    bResult        = FALSE;
         BYTE*   pIndexRoot     = NULL;
         BYTE*   pIndexAlloc    = NULL;
         BYTE*   pDataRunOff    = NULL;    //Data run descriptor's offset in file record.
         __NTFS_DATA_RUN* pRunList = NULL;
 
         if((NULL == pFileSystem) || (NULL == pFileObject) || (NULL == pFileRecord))
         {
                   goto __TERMINAL;
         }
         pIndexRoot = GetFileAttribute(pFileSystem,pFileRecord,NTFS_ATTR_INDEXROOT);
         if(NULL == pIndexRoot)
         {
                   goto __TERMINAL;
         }
         //Now process the INDEX ROOT attribute now.
 
         //Process INDEX ALLOCATION attribute.
         pIndexAlloc = GetFileAttribute(pFileSystem,pFileRecord,NTFS_ATTR_INDEXALLOC);
         if(NULL != pIndexAlloc)  //Process it.
         {
                   //printf("Process IA attribute,type = %d,flags = %X\r\n",NRA_TYPE(pIndexAlloc),NRA_FLAGS(pIndexAlloc));
                   pDataRunOff = GetDataRunDesc(pIndexAlloc);
                   if(NULL == pDataRunOff)  //Exception case.
                   {
                            goto __TERMINAL;
                   }
                   //Decode data run descriptor.
                   pRunList = DecodeDataRun(pDataRunOff,&pFileObject->totalCluster);
                   if(NULL == pRunList)  //Exception.
                   {
                            return FALSE;
                   }
                   //OK,set the data run list of file object.
                   pFileObject->pRunList = pRunList;
                   //Update the file's size attribute.The common operation to set the file's size
                   //is analyze the FILE_NAME attribute,but this attribute's file size value may
                   //invalid in some special case,so we assume one file only has one DATA attribute
                   //and set the file's size as DATA attribute's length.
                   //This implementation may not cover all scenarios,but in most case it should be 
                   //OK.
                   //Since lack standard information about NTFS,hereby glitches in very special case
                   //is tolerable.
                   pFileObject->fileAllocSize = NRA_ALLOC_SIZE(pIndexAlloc);
                   pFileObject->fileSizeLow   = NRA_TRUE_SIZE(pIndexAlloc);
                   pFileObject->fileSizeHigh  = 0;  //Does not support yet.
         }
         bResult = TRUE;
 
__TERMINAL:
         return bResult;
}
 
//A critical routine to create a file object by giving the file's record.
static __NTFS_FILE_OBJECT* NtfsCreateFileByFR(__NTFS_FILE_SYSTEM* pFileSystem,BYTE* pFileRecord)
{
         __NTFS_FILE_OBJECT*          pNewFile       = NULL;
         BYTE*                        pFileAttr      = NULL;
         static BYTE                  ntfsSig[]      = {'F','I','L','E'};
         int                          i;
         BOOL                         bResult        = FALSE;
		 UINT_32                      dwFlags;
 
         if((NULL == pFileSystem) || (NULL == pFileRecord))
         {
                   goto __TERMINAL;
         }
         //Check if the file record is a valid one.
         for(i = 0;i < 4;i ++)
         {
                   if(*(pFileRecord + i) != ntfsSig[i])
                   {
                            goto __TERMINAL;
                   }
         }
         //OK,create file object first.
         pNewFile = (__NTFS_FILE_OBJECT*)__MEM_ALLOC(sizeof(__NTFS_FILE_OBJECT));
         if(NULL == pNewFile)
         {
                   goto __TERMINAL;
         }
         pNewFile->pNext = pNewFile;
         pNewFile->pPrev = pNewFile;
         //Initialize the new file's file record buffer.
         pNewFile->pFileRecord = (BYTE*)__MEM_ALLOC(pFileSystem->FileRecordSize);
         if(NULL == pNewFile->pFileRecord)
         {
                   goto __TERMINAL;
         }
         memcpy(pNewFile->pFileRecord,pFileRecord,pFileSystem->FileRecordSize);
         //Initialize the new file's temporary buffer.
		 pNewFile->fileBuffSize = pFileSystem->clusSize;
		 if(pNewFile->fileBuffSize < MIN_FILE_BUFFER_SIZE)
		 {
			 pNewFile->fileBuffSize = MIN_FILE_BUFFER_SIZE;
		 }
         pNewFile->pFileBuffer = (BYTE*)__MEM_ALLOC(pNewFile->fileBuffSize);
         if(NULL == pNewFile->pFileBuffer)
         {
                   goto __TERMINAL;
         }
         //pNewFile->fileBuffSize = pFileSystem->sectorPerClus * pFileSystem->bytesPerSector;
         pNewFile->fileBuffContentSize = 0;  //Has not any valid data yet.
         pNewFile->buffStartVCN = 0;
 
         //Initialize file's common attributes.
         pFileAttr = GetFileAttribute(pFileSystem,pFileRecord,NTFS_ATTR_FILENAME);
         if(NULL == pFileAttr)  //Have not file name attribute,invalid case.
         {
                   goto __TERMINAL;
         }
         //Decode the file attribute now.
         if(!ParseFileNameAttr(pFileSystem,pNewFile,pFileAttr))
         {
                   goto __TERMINAL;
         }
 
         pNewFile->currPtrClusOff = 0;
         pNewFile->currPtrLCN   = 0;
         pNewFile->currPtrVCN   = 0;
         pNewFile->pFileSystem  = pFileSystem;
         pNewFile->pRunList     = NULL;
 
         /*//Initialize file's size attribute.
         pNewFile->totalCluster   = 0;  //DATA attribute's total cluster number.
         pNewFile->fileAllocSize  = 0;
         pNewFile->fileSizeHigh   = 0;
         pNewFile->fileSizeLow    = 0;*/
 
         //Now decode file's data attribute if exist(for common file).
         pFileAttr = GetFileAttribute(pFileSystem,pFileRecord,NTFS_ATTR_DATA);
         if(pFileAttr)
         {
                   if(!ParseDataAttr(pNewFile,pFileAttr))
                   {
                            goto __TERMINAL;
                   }
         }
         else  //It's a directory file,decode INDEX_ROOT and INDEX_ALLOCATION attribute.
         {
                   if(!ParseIndexAttr(pFileSystem,pNewFile,pFileRecord))
                   {
                            goto __TERMINAL;
                   }
         }
         //Insert the file into file system's list.
         __ENTER_CRITICAL_SECTION(NULL,dwFlags);
         pNewFile->pNext  = pFileSystem->fileRoot.pNext;
         pNewFile->pPrev  = &(pFileSystem->fileRoot);
         pFileSystem->fileRoot.pNext->pPrev = pNewFile;
         pFileSystem->fileRoot.pNext = pNewFile;
         __LEAVE_CRITICAL_SECTION(NULL,dwFlags);
 
         bResult = TRUE;
 
__TERMINAL:
         if(!bResult)
         {
                   if(pNewFile)
                   {
                            if(pNewFile->pFileRecord)
                            {
                                     __MEM_FREE(pNewFile->pFileRecord);
                            }
                            if(pNewFile->pFileBuffer)
                            {
                                     __MEM_FREE(pNewFile->pFileBuffer);
                            }
                            __MEM_FREE(pNewFile);
                            pNewFile = NULL;
                   }
         }
         return pNewFile;
}
 
//A helper routine,calculate a file record's VCN number and offset in MFT.
static UINT_32 GetFRPosition(__NTFS_FILE_SYSTEM* pFileSystem,UINT_32 frIndex,UINT_32* pOffset)
{
         UINT_32    frPerClus = 0;
         UINT_32    vcn       = 0;
 
         if(NULL == pFileSystem)
         {
                   return 0;
         }
		 //The following code is revised in 2011/12/10.
		 if(pFileSystem->clusSize >= pFileSystem->FileRecordSize)
		 {
			 frPerClus = pFileSystem->clusSize / pFileSystem->FileRecordSize;
			 vcn = frIndex / frPerClus;
			 if(pOffset)
			 {
				 *pOffset = (frIndex % frPerClus) * pFileSystem->FileRecordSize;
			 }
		 }
		 else
		 {
			 frPerClus = pFileSystem->FileRecordSize / pFileSystem->clusSize;
			 vcn = frIndex * frPerClus;
			 if(pOffset)
			 {
				 *pOffset = 0;
			 }
		 }
         //frPerClus = (pFileSystem->bytesPerSector * pFileSystem->sectorPerClus) / pFileSystem->FileRecordSize;
         //vcn = frIndex / frPerClus;
         //if(pOffset)
         //{
         //         *pOffset = (frIndex % frPerClus) * pFileSystem->FileRecordSize;
         //}
         return vcn;
}
 
//A helper routine to retrieve a file's file record given it's file record index.
static BOOL GetFileRecord(__NTFS_FILE_SYSTEM* pFileSystem,__NTFS_FILE_OBJECT* pMFT,UINT_32 frIndex,BYTE* pFileRecord)
{
         UINT_32    frVCN      = 0;
         UINT_32    frOffset   = 0;
         UINT_32    frLCN      = 0;
         BYTE*      pClusterBuff = NULL;
		 UINT_32    clusNum    = 0;  //How many clusters should be read to retrieve one FR.
 
         if((NULL == pMFT) || (NULL == pFileRecord))
         {
                   return FALSE;
         }
         //Get the FR's VCN and cluster offset first.
         frVCN = GetFRPosition(pFileSystem,frIndex,&frOffset);
         //Convert the VCN to LCN.
         frLCN = VCN2LCN(pMFT,frVCN);
         if(0 == frLCN)  //Invalid LCN number.
         {
			 return FALSE;
         }
		 //printf("GetFileRecord : LCN = %d\r\n",frLCN);
         //OK,retrieve the file record now.revised in 2011/12/10.
		 clusNum = pFileSystem->FileRecordSize / pFileSystem->clusSize;
		 if(0 == clusNum)  //If cluster size is less than FR size.
		 {
			 clusNum = 1;
		 }
         pClusterBuff = (BYTE*)__MEM_ALLOC(pFileSystem->clusSize * clusNum);
         if(NULL == pClusterBuff)
         {
                   return FALSE;
         }
         if(!ReadCluster(pFileSystem,frLCN,clusNum,pClusterBuff))
         {
                   __MEM_FREE(pClusterBuff);
                   return FALSE;
         }
         memcpy(pFileRecord,pClusterBuff + frOffset,pFileSystem->FileRecordSize);
         __MEM_FREE(pClusterBuff);
         return RestoreSectContent(pFileSystem,pFileRecord);
}
 
//A local routine used to create one NTFS file object by given it's file record number
//in $MFT.
__NTFS_FILE_OBJECT* NtfsCreateFileByFRN(__NTFS_FILE_SYSTEM* pFileSystem,UINT_32 frIndex)
{
         __NTFS_FILE_OBJECT*    pFileObject   = NULL;
         BYTE*                  pFileRecord   = NULL;
 
         if(NULL == pFileSystem)
         {
                   goto __TERMINAL;
         }
         if(NULL == pFileSystem->pMFT)  //Exception case,maybe caused by uncomplete initialization of FS obj.
         {
                   goto __TERMINAL;
         }
         pFileRecord = (BYTE*)__MEM_ALLOC(pFileSystem->FileRecordSize);
         if(NULL == pFileRecord)
         {
                   goto __TERMINAL;
         }
         //Get the file record's content from $MFT.
         if(!GetFileRecord(pFileSystem,pFileSystem->pMFT,frIndex,pFileRecord))
         {
                   goto __TERMINAL;
         }
         pFileObject = NtfsCreateFileByFR(pFileSystem,pFileRecord);
 
__TERMINAL:
         if(NULL != pFileRecord)
         {
                   __MEM_FREE(pFileRecord);
         }
         return pFileObject;
}
 
//Destroy a file object.
VOID NtfsDestroyFile(__NTFS_FILE_OBJECT* pFileObject)
{
         __NTFS_DATA_RUN*    pRunList    = NULL;
		 UINT_32             dwFlags; 
 
         if(NULL == pFileObject)
         {
                   return;
         }
         //Delete it from file system's file list.
         __ENTER_CRITICAL_SECTION(NULL,dwFlags);
         pFileObject->pPrev->pNext = pFileObject->pNext;
         pFileObject->pNext->pPrev = pFileObject->pPrev;
         __LEAVE_CRITICAL_SECTION(NULL,dwFlags);
         //Release file's buffer.
         if(pFileObject->pFileBuffer)
         {
                   __MEM_FREE(pFileObject->pFileBuffer);
         }
         if(pFileObject->pFileRecord)
         {
                   __MEM_FREE(pFileObject->pFileRecord);
         }
         pRunList = pFileObject->pRunList;
         while(pRunList)
         {
                   pFileObject->pRunList = pRunList->pNext;
                   __MEM_FREE(pRunList);
                   pRunList = pFileObject->pRunList;
         }
}
 
//A core routine used to create NTFS file system given a boot sector's 
//content.
__NTFS_FILE_SYSTEM* CreateNtfsFileSystem(__COMMON_OBJECT* pPartition,BYTE* pSector)
{
         static BYTE           ntfsOEMName[] = {'N','T','F','S',' ',' ',' ',' '};
         __NTFS_FILE_SYSTEM*   pFileSystem   = NULL;
         int                   i;
         BOOL                  bResult       = FALSE;
         BYTE*                 pFrBuffer     = NULL;
         __NTFS_FILE_OBJECT*   pMFT          = NULL;
         __NTFS_FILE_OBJECT*   pRoot         = NULL;
         UINT_32               rootLCN       = 0;    //LCN of ROOT DIR's file record in MFT.
		 UINT_32               clusNum       = 0;    //How many cluster one FR occupying.
 
         if((NULL == pSector) || (NULL == pPartition))
         {
                   goto __TERMINAL;
         }
         for(i = 0;i < 8;i ++)  //Check if the partition is a valid NTFS partition.
         {
                   if(*(pSector + BPB_OEMNAME_START + i) != ntfsOEMName[i])
                   {
                            goto __TERMINAL;
                   }
         }
         //This partition like a NTFS partiton,try to mount it.
         pFileSystem = (__NTFS_FILE_SYSTEM*)__MEM_ALLOC(sizeof(__NTFS_FILE_SYSTEM));
         if(NULL == pFileSystem)
         {
                   goto __TERMINAL;
         }
         pFileSystem->bytesPerSector = BPB_BYTES_PER_SEC(pSector);
         if(pFileSystem->bytesPerSector > 4096)  //Assume the sector's size is not larger than 4K.
         {
                   goto __TERMINAL;
         }
         pFileSystem->sectorPerClus  = BPB_SEC_PER_CLUS(pSector);
         pFileSystem->clusSize       = pFileSystem->sectorPerClus * pFileSystem->bytesPerSector;
         if(pFileSystem->clusSize > 4096)  //Cluster size should not exceed 4k.
         {
                   goto __TERMINAL;
         }
         pFileSystem->hiddenSector     = BPB_HIDDEN_SEC(pSector);
         pFileSystem->totalSectorLow   = BPB_TOTAL_SEC_LOW(pSector);
         pFileSystem->totalSectorHigh  = BPB_TOTAL_SEC_HIGH(pSector);
         pFileSystem->mftStartClusLow  = BPB_MFT_START_LOW(pSector);
         pFileSystem->mftStartClusHigh = BPB_MFT_START_HIGH(pSector);
         if(pFileSystem->mftStartClusLow == 0)  //Invalid value for MFT.
         {
                   goto __TERMINAL;
         }
         pFileSystem->FileRecordSize   = 1024;  //Assuem it is 1024,should be OK.
         pFileSystem->DirBlockSize     = BPB_CLUS_PER_DR(pSector);//4096;  //Assume it is 4k,should be OK.
         pFileSystem->DirBlockSize    *= pFileSystem->clusSize;
         for(i = 0;i < 8;i ++)
         {
                   pFileSystem->serialNumber[i] = *(pSector + BPB_SerialNum + i);
         }
		 //Initialize volume label and partition object.
		 strcpy(pFileSystem->volLabel,NTFS_DEFAULT_LABEL);
		 pFileSystem->pNtfsPartition = pPartition;

         //Initialize file object list.
         pFileSystem->fileRoot.pPrev = &(pFileSystem->fileRoot);
         pFileSystem->fileRoot.pNext = &(pFileSystem->fileRoot);
 
         //OK,process MFT file and root directory now.Revised in 2011/12/10.
		 clusNum = pFileSystem->FileRecordSize / pFileSystem->clusSize;
		 if(0 == clusNum)  //Cluster size less than FR size will lead this case.
		 {
			 clusNum = 1;
		 }
         pFrBuffer = (BYTE*)__MEM_ALLOC(pFileSystem->clusSize * clusNum);
         if(NULL == pFrBuffer)
         {
                   goto __TERMINAL;
         }
         //Read the first cluster of MFT.
         if(!ReadCluster(pFileSystem,pFileSystem->mftStartClusLow,clusNum,pFrBuffer))
         {
                   goto __TERMINAL;
         }
         //OK,try to create MFT file object.
         pMFT = NtfsCreateFileByFR(pFileSystem,pFrBuffer);
         if(!pMFT)
         {
                   PrintLine("NTFS CreateNtfsFileSystem : Can not create MFT file object.");
                   goto __TERMINAL;
         }
         pFileSystem->pMFT = pMFT;
         //Try to create ROOT DIRECTORY file object.But we should get the VCN number of MFT where
         //root directory's file record resides.
         if(!GetFileRecord(pFileSystem,pMFT,0x05,pFrBuffer))
         {
                   PrintLine("NTFS NtfsCreateFileSystem : Can not get ROOT DIRECTORY's file record.");
                   goto __TERMINAL;
         }
         pRoot = NtfsCreateFileByFR(pFileSystem,pFrBuffer);
         if(!pRoot)
         {
                   PrintLine("NTFS NtfsCreateFileSystem : Can not create ROOT DIRECTORY file object.");
                   goto __TERMINAL;
         }
         pFileSystem->pRootDir = pRoot;
 
         bResult = TRUE;
 
__TERMINAL:
         if(!bResult)
         {
                   if(pMFT)
                   {
                            NtfsDestroyFile(pMFT);
                   }
                   if(pRoot)
                   {
                            NtfsDestroyFile(pRoot);
                   }
                   if(pFileSystem != NULL)  //Should release it.
                   {
                            __MEM_FREE(pFileSystem);
                            pFileSystem = NULL;
                   }
                   if(pFrBuffer)
                   {
                            __MEM_FREE(pFrBuffer);
                   }
         }
         return pFileSystem;
}

//Destroy an NTFS file system object.
void DestroyNtfsFileSystem(__NTFS_FILE_SYSTEM* pFileSystem)
{
	if(NULL == pFileSystem)
	{
		return;
	}
	//Release all resources or objects in file system object.
	if(pFileSystem->pMFT)
	{
		NtfsDestroyFile(pFileSystem->pMFT);
	}
	if(pFileSystem->pRootDir)
	{
		NtfsDestroyFile(pFileSystem->pRootDir);
	}
	//Should release ALL file objects in file list object,but this may
	//cause conflict since one FILE OBJECT corresponds one device object,
	//only release FILE OBJECT is not enough since device object remained.
	//In future version this problem should be solved but currently it should
	//not be a problem since DestroyNtfsFileSystem only called in the initialization
	//process,no file object will be created in this process.
	KMemFree(pFileSystem,KMEM_SIZE_TYPE_ANY,0);
}
 
//Create a NTFS object and return it by spcifying it's full name.
//This routine searchs all directory entries level by level until reach
//the outest level,then search the target file and try to open it.
__NTFS_FILE_OBJECT* NtfsCreateFile(__NTFS_FILE_SYSTEM* pFileSystem,const WCHAR* pFullName)
{
         __NTFS_FILE_OBJECT*       pNewFile        = NULL;
         __NTFS_FILE_OBJECT*       pCurrDir        = NULL;      //Current directory to search.
         BOOL                      bResult         = FALSE;
         WCHAR                     DirName[MAX_FILE_NAME_LEN];
         WCHAR                     FileName[MAX_FILE_NAME_LEN];
         UINT_32                   level           = 0;
         UINT_32                   i;
         UINT_32                   frIndex         = 0;
		 BOOL                      onlyDir         = FALSE;  //Indicate if the given name is only contains directory name.
 
         if((NULL == pFileSystem) || (NULL == pFullName))
         {
                   goto __TERMINAL;
         }
         //Parse the full name,get it's directory part and file name part correspondingly.
         if(!wGetPathName((WCHAR*)pFullName,DirName,FileName))  //Exception,maybe full name is in illegal format.
         {
                   goto __TERMINAL;
         }
		 if(FileName[0] == 0)
		 {
			 onlyDir = TRUE;
		 }
         //Get directory levels.
         if(!wGetFullNameLevel((WCHAR*)pFullName,&level))
         {
                   goto __TERMINAL;
         }
         //OK,search directory level by level.
         pCurrDir = pFileSystem->pRootDir;
         for(i = 0;i < level;i ++)
         {
                   if(!wGetSubDirectory((WCHAR*)pFullName,i + 1,DirName))
                   {
                            goto __TERMINAL;
                   }
                   //printf("NtfsCreateFile : subdir = ");
                   //wprintf(DirName);
                   //printf("\r\n");
                   //Find the sub directory in current directory.
                   if(!FindFile(pCurrDir,DirName,&frIndex)) //Can not find.
                   {
                            //printf("Can not find the subdirectory when round = %d\r\n",i);
                            goto __TERMINAL;
                   }
                   //Find the subdirectory,open it,destroy the current directory's
                   //file object if it is not the root directory.
                   if(pCurrDir != pFileSystem->pRootDir)
                   {
                            NtfsDestroyFile(pCurrDir);
                   }
                   pCurrDir = NtfsCreateFileByFRN(pFileSystem,frIndex);
                   if(NULL == pCurrDir)  //Can not create the file object.
                   {
                            goto __TERMINAL;
                   }
                   //OK,set it as the current directory and continue to search.
         }
         //When reach here,it means that the lowest level of directory has been reached.If only directory
		 //name specified,then return pCurrDir is OK.
		 if(onlyDir)
		 {
			 bResult = TRUE;
			 goto __TERMINAL;
		 }
         //Actual file name is given,then try to search the target file in this directory.
         if(!FindFile(pCurrDir,FileName,&frIndex))
         {
                   goto __TERMINAL;
         }
         //Found the file,open it.
         pNewFile = NtfsCreateFileByFRN(pFileSystem,frIndex);
		 bResult  = TRUE;
 
__TERMINAL:
		 if(onlyDir)
		 {
			 if(bResult)
			 {
				 return pCurrDir;
			 }
		 }
		 //A data file,then should release the directory object and return the file.
         if(NULL != pCurrDir)
         {
                   if(pCurrDir != pFileSystem->pRootDir)
                   {
                            NtfsDestroyFile(pCurrDir);
                   }
         }
         return pNewFile;
}
 
//------------------------------------------------------------------------
//  **********************************************************
//  The following code only for debugging in Windows platform.
//  **********************************************************
//------------------------------------------------------------------------
/* 
//A helper routine to dumpout one cluster's content.
VOID DumpCluster(BYTE* pCluster)
{
         int i = 4096 / 16;
         int j;
         BYTE ch;
 
         for(i;i > 0;i --)
         {
                   for(j = 0;j < 16;j ++)
                   {
                            printf("%02X ",*(pCluster + j));
                                                                 if(0 == (j + 1) % 8)
                                                                 {
                                                                           printf("  ");
                                                                 }
                   }
                   printf("    ");
                   for(j = 0;j < 16;j ++)
                   {
                            ch = *(pCluster + j);
                            if(((ch >= 'A') && (ch <= 'Z')) || ((ch >= 'a') && (ch <= 'z')) || ((ch >= '1') && (ch <= '9')))
                            {
                                     printf("%c",ch);
                            }
                            else
                            {
                                     printf(".");
                            }
                   }
                   printf("\r\n");
                   pCluster += 16;
         }
}
 
//A local helper routine used to dumpout file object's information.
void DumpFileObject(__NTFS_FILE_OBJECT* pFileObject)
{
         __NTFS_DATA_RUN*  pRunList = NULL;
 
         printf("  NTFS file object information:\r\n");
         printf("  -----------------------------\r\n");
         printf("  File name:");
         for(int i = 0;i < pFileObject->fileNameSize;i ++)
         {
                   printf("%c",pFileObject->FileName[i]);
         }
         printf("\r\n");
         printf("  File size : %d\r\n",pFileObject->fileSizeLow);
         printf("  File alloc size : %d\r\n",pFileObject->fileAllocSize);
         printf("  File attr : %X\r\n",pFileObject->fileAttr);
         printf("  File total clus : %d\r\n",pFileObject->totalCluster);
         //Dumpout run list.
         printf("  NTFS run list begin:\r\n");
         pRunList = pFileObject->pRunList;
         while(pRunList)
         {
                   printf("  Clus num : %d,start LCN: %d\r\n",pRunList->clusNum,
                            pRunList->startClusLow);
                   pRunList = pRunList->pNext;
         }
         printf("  NTFS run list end.\r\n");
         printf("\r\n");
}
 
static BOOL ReadSector0(BYTE* pBuffer)
{
         BOOL bResult = FALSE;
         char* szDeviceName = PARTITION_DEVICE_NAME;
         HANDLE hDevice = NULL;
 
         hDevice = CreateFile(
                   szDeviceName,
                   GENERIC_READ,
                   FILE_SHARE_READ | FILE_SHARE_WRITE,
                   NULL,
                   OPEN_EXISTING,
                   0,
                   NULL);
         if(INVALID_HANDLE_VALUE == hDevice)  //Can not open device.
         {
                   return bResult;
         }
         //Read device.
         DWORD dwRead = 0;
         bResult = ReadFile(hDevice,pBuffer,512,&dwRead,NULL);
         CloseHandle(hDevice);
         return bResult;
}
 
void main()
{
         __NTFS_FILE_SYSTEM*   pFileSystem = NULL;
         BYTE                  Sector0[512];
         BYTE                  buffer[4096];
         __NTFS_FILE_OBJECT*   pSysFile = NULL;
         __NTFS_FILE_OBJECT*   pExampFile = NULL;
         UINT_32               readSize = 0;
         int                   i = 0;
         WCHAR                 fileName[] = {'h','c','n','i','m','g','e','.','b','i','n',0};
         UINT_32               frIndex = 0;
         UINT_32               frStartSect = 0;  //OS kernel's FR's start sector number.
         UINT_32               drOffset = 0;     //Data run offset in FR of OS kernel.
         char*                 fullname = "C:\\Program Files\\Cisco\\hellocn.txt";
         WCHAR                 FullName[128];
 
         if(!ReadSector0(Sector0))
         {
                   printf("  Can not read sector 0.\r\n");
                   return;
         }
         pFileSystem = CreateNtfsFileSystem(Sector0);
         if(NULL == pFileSystem)
         {
                   printf("  Can not create NTFS file system object.\r\n");
                   return;
         }
         printf("  Mount NTFS file system successfully.\r\n");
         DumpFileSystem(pFileSystem);
         printf("\r\n");
         //DumpFileObject(pFileSystem->pMFT);
         //DumpFileObject(pFileSystem->pRootDir);
         //ListDirFile(pFileSystem->pRootDir);
         //ShowLoaderInfo(pFileSystem);
         byte2unicode(FullName,fullname);
         //ListDirFile(pFileSystem,FullName);
         pExampFile = NtfsCreateFile(pFileSystem,FullName);
         if(NULL == pExampFile)
         {
                   printf("Can not find the desired file.\r\n");
         }
         else
         {
                   printf("  Find and open the desired file successfully.\r\n");
                   printf("  ----------------------------- Target file's content ----------------------------- \r\n");
                   while(NtfsReadFile(pExampFile,buffer,4096,&readSize,NULL))
                   {
                            if(0 == readSize)  //End of file.
                            {
                                     break;
                            }
                            DumpCluster(buffer);
							memset(buffer,0,4096);
                            printf("  -------------------------------------------------------------------------------\r\n");
                   }
         }
 
         NtfsDestroyFile(pExampFile);
         NtfsDestroyFile(pFileSystem->pMFT);
         NtfsDestroyFile(pFileSystem->pRootDir);
         __MEM_FREE(pFileSystem);
}
*/

#endif
