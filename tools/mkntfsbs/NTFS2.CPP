//------------------------------------------------------------------------
//  NTFS file system implementation code,the second part,since it's code line
//  number is too large that can not be hold in one file.
//
//  Author                 : Garry.Xin
//  Initial date           : Nov 06,2011
//  Last updated           : Nov 06,2011
//  Last updated author    : Garry.Xin
//  Last udpated content   : 
//------------------------------------------------------------------------
 
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
 
#ifndef __NTFS_H__
#include "NTFS.H"
#endif
 
//A helper routine to read one or several sectors from disk partiton.
BOOL ReadSector(__NTFS_FILE_SYSTEM* pFileSystem,UINT_32 startSector,UINT_32 sectorNum,BYTE* pBuffer)
{
         BOOL bResult = FALSE;
         char* szDeviceName = PARTITION_DEVICE_NAME;
         HANDLE hDevice = NULL;
         long offsetHigh = 0;
         long offsetLow  = 0;
         __int64 offset  = 0;
 
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
         offset = (__int64)startSector * 512;
         offsetLow  = (long)offset;
         offsetHigh = (long)(offset >> 32);
         //printf("ReadSector : offsetHigh = %d,offsetLow = %d.\r\n",offsetHigh,offsetLow);
         SetFilePointer(hDevice,offsetLow,&offsetHigh,FILE_BEGIN);
         //Read device.
         DWORD dwRead   = 0;
         bResult = ReadFile(hDevice,pBuffer,sectorNum * 512,&dwRead,NULL);
         if(dwRead < sectorNum * 512)
         {
                   printf("ReadSector error : Actual read size is less than required reading size.\r\n");
         }
         CloseHandle(hDevice);
         return bResult;
}
 
//A helper local routine convert the cluster number to partition's sector number.
static UINT_32 Cluster2Sector(__NTFS_FILE_SYSTEM* pFileSystem,UINT_32 clusNum)
{
         if(NULL == pFileSystem)
         {
                   return 0;
         }
         return clusNum * pFileSystem->sectorPerClus;
}
 
//A helper routine to read one or several clusters from disk partition.
BOOL ReadCluster(__NTFS_FILE_SYSTEM* pFileSystem,UINT_32 startLCN,UINT_32 clusNum,BYTE* pBuffer)
{
         UINT_32 startSector = Cluster2Sector(pFileSystem,startLCN);
         UINT_32 sectorNum   = pFileSystem->sectorPerClus * clusNum;
         //printf("Start LCN : %d,start sector : %d,sector number : %d\r\n",startLCN,startSector,sectorNum);
         return ReadSector(pFileSystem,startSector,sectorNum,pBuffer);
}
 
//A helper routine convert a given file's VCN to LCN.
//If this function fail it will return 0,which is not a valid LCN for file.
UINT_32 VCN2LCN(__NTFS_FILE_OBJECT* pFileObject,UINT_32 vcn)
{
         __NTFS_DATA_RUN*  pRunList = NULL;
 
         if(NULL == pFileObject)
         {
                   return 0;
         }
         if(NULL == pFileObject->pRunList)
         {
                   return 0;
         }
         if(vcn >= pFileObject->totalCluster)  //Exceed file's scope.
         {
                   return 0;
         }
         //OK,try to convert it.
         pRunList = pFileObject->pRunList;
         while(pRunList)
         {
                   if(vcn < pRunList->clusNum)
                   {
                            return (vcn + pRunList->startClusLow);
                   }
                   else
                   {
                            vcn -= pRunList->clusNum;
                            pRunList = pRunList->pNext;
                   }
         }
         return 0;
}
 
//A local helper routine to load some data from disk into file's local buffer,and
//update the buffer pointer variables of file object.
static BOOL LoadFileBuffer(__NTFS_FILE_OBJECT* pFileObject)
{
         BOOL    bResult        = FALSE;
         UINT_32 lcn            = 0;
         UINT_32 vcn            = 0;
         UINT_32 clusNum        = 0;  //How many cluster(s) to read.
         UINT_32 clusSize       = 0;
         BYTE*   pFileBuffer    = NULL;
         UINT_32 maxClus        = 0;  //How many cluster(s) between current pointer and the end of file.
                    UINT_32 validSize      = 0;  //How many bytes is valid on content buffer.
 
         if(NULL == pFileObject)
         {
                   goto __TERMINAL;
         }
         clusSize = pFileObject->pFileSystem->bytesPerSector * pFileObject->pFileSystem->sectorPerClus;
         clusNum  = pFileObject->fileBuffSize / clusSize;
         maxClus  = pFileObject->fileSizeLow;
         maxClus -= pFileObject->currPtrVCN * clusSize;
         //maxClus /= clusSize;
                    maxClus = (0 == maxClus % clusSize) ? maxClus / clusSize : (maxClus / clusSize + 1);
 
         if(clusNum > maxClus)
         {
                   clusNum = maxClus;
         }
         vcn     = pFileObject->currPtrVCN;
         pFileBuffer = pFileObject->pFileBuffer;
 
         while(clusNum)
         {
                   lcn = VCN2LCN(pFileObject,vcn);
                   if(0 == lcn)  //Exception case,should not occur.
                   {
                            goto __TERMINAL;
                   }
                   //Read cluster now.
                   if(!ReadCluster(pFileObject->pFileSystem,lcn,1,pFileBuffer))
                   {
                            goto __TERMINAL;
                   }
                   pFileBuffer += clusSize;
                   clusNum --;
                   vcn ++;
         }
         pFileObject->buffStartVCN = pFileObject->currPtrVCN;
                    validSize = pFileBuffer - pFileObject->pFileBuffer;
                    if(validSize > pFileObject->fileSizeLow - pFileObject->buffStartVCN * clusSize)
                    {
                             validSize = pFileObject->fileSizeLow - pFileObject->buffStartVCN * clusSize;
                    }
         //pFileObject->fileBuffContentSize = pFileBuffer - pFileObject->pFileBuffer;
                   pFileObject->fileBuffContentSize = validSize;
         bResult = TRUE;
 
__TERMINAL:
         return bResult;
}
 
//Set a file's current pointer to a specific position.Please note the only supported move method is from file's
//start position to the desired position located by toMoveLow and toMoveHigh.
UINT_32 NtfsSetFilePointer(__NTFS_FILE_OBJECT* pFileObject,UINT_32 toMoveLow,UINT_32* ptoMoveHigh,UINT_32 moveMethod)
{
         UINT_32        hasMove       = 0;     //Actually moved length.
         UINT_32        clusSize      = 0;
 
         if(NULL == pFileObject)
         {
                   return hasMove;
         }
         hasMove = (toMoveLow < pFileObject->fileSizeLow) ? toMoveLow : pFileObject->fileSizeLow;
         clusSize = pFileObject->pFileSystem->bytesPerSector * pFileObject->pFileSystem->sectorPerClus;
         pFileObject->currPtrVCN = hasMove / clusSize;
         pFileObject->currPtrClusOff = hasMove % clusSize;
         pFileObject->currPtrLCN = VCN2LCN(pFileObject,hasMove / clusSize);
         //Load current position's content into temporary buffer.
         if(!LoadFileBuffer(pFileObject))
         {
                   printf("NtfsSetFilePointer: Can not load file's content.\r\n");
         }
         return hasMove;
}
 
//Fetch a given file's size.
UINT_32 NtfsGetFileSize(__NTFS_FILE_OBJECT* pFileObject,UINT_32* pSizeHigh)
{
         if(NULL == pFileObject)
         {
                   return 0;
         }
         return pFileObject->fileSizeLow;
}
 
//Fetch a file object's attributes.
UINT_32 NtfsGetFileAttr(__NTFS_FILE_OBJECT* pFileObject)
{
         if(NULL == pFileObject)
         {
                   return 0;
         }
         return pFileObject->fileAttr;
}
 
//Read file content,this is one of the most important routine in NTFS's
//implementation code.Here is the routine's processing procedure:
// 1. Calculate how many bytes to read,since the requested size may larger than
//    file's actual size;
// 2. Check if file's content buffer contains the desired file content,and copy
//    from it directly if so;
// 3. Read the rest desired file content from file;
// 4. Update file's buffer content,only last part of this read transaction will
//    be copied to file buffer;
// 5. Update the file's current pointer to the position where last read byte resides.
//
BOOL NtfsReadFile(__NTFS_FILE_OBJECT* pFileObject,BYTE* pBuffer,UINT_32 toReadSize,UINT_32*pReadSize,void* pExt)
{
         BOOL                  bResult      = FALSE;
         __NTFS_FILE_SYSTEM*   pFileSystem  = NULL;
         UINT_32               toRead       = 0;
         UINT_32               fileOffset   = 0;     //File current pointer's offset in the whole file.
         UINT_32               buffValidEnd = 0;
         UINT_32               buffValidStart = 0;
         UINT_32               clusSize       = 0;
         UINT_32               totalRead      = 0;
 
         if((NULL == pFileObject) || (NULL == pBuffer) || (0 == toReadSize) || (NULL == pReadSize))
         {
                   goto __TERMINAL;
         }
         pFileSystem = pFileObject->pFileSystem;
         clusSize = pFileSystem->bytesPerSector * pFileSystem->sectorPerClus;
 
         //Calculate fileOffset.
         fileOffset = pFileObject->currPtrVCN * clusSize;
         fileOffset += pFileObject->currPtrClusOff;
         if(fileOffset > pFileObject->fileSizeLow)  //Exception case.
         {
                   goto __TERMINAL;
         }
         //Calculate how many bytes should be read.
         toRead = (toReadSize <= (pFileObject->fileSizeLow - fileOffset)) ? toReadSize : (pFileObject->fileSizeLow - fileOffset);
         //Launch the reading process.
                    //printf("Begin to read %d bytes,file offset = %d\r\n",toRead,fileOffset);
         while(toRead)
         {
                   //Check if the desired content is in file's local buffer,and return it directly if so.
                   buffValidStart = pFileObject->buffStartVCN * clusSize;
                   buffValidEnd =  buffValidStart;
                   buffValidEnd += pFileObject->fileBuffContentSize;
                   if((buffValidEnd > fileOffset) && (buffValidStart <= fileOffset)) //Can fetch some content from buffer directly.
                   {
                            //Calculate the start position to read of file's local buffer
                            buffValidStart = fileOffset - pFileObject->buffStartVCN * clusSize;
                            if(toRead <= buffValidEnd - fileOffset)  //Buffer's content is enough.
                            {
                                     memcpy(pBuffer,(pFileObject->pFileBuffer + buffValidStart),toRead);
                                     fileOffset += toRead;  //Update file's current pointer.
                                     pFileObject->currPtrVCN     = fileOffset / clusSize;
                                     pFileObject->currPtrClusOff = fileOffset % clusSize;
                                     totalRead += toRead;
                                     if(pReadSize)
                                     {
                                               *pReadSize = totalRead;
                                     }
                                     bResult = TRUE;
                                     goto __TERMINAL;
                            }
                            //Read the whole content in file's buffer.
                            memcpy(pBuffer,(pFileObject->pFileBuffer + buffValidStart),buffValidEnd -fileOffset);
                            totalRead += buffValidEnd - fileOffset;
                            pBuffer   += buffValidEnd - fileOffset;
                            toRead    -= buffValidEnd - fileOffset;
                            fileOffset = buffValidEnd;  //The subsequent reading should start from here.
                            pFileObject->currPtrVCN = fileOffset / clusSize;
                            pFileObject->currPtrClusOff = fileOffset % clusSize;
                   }
                   else  //File buffer's content is invalid,update it.
                   {
                            if(!LoadFileBuffer(pFileObject))
                            {
                                     printf("Can not load file buffer.\r\n");
                                     goto __TERMINAL;
                            }
                   }
         }
         if(pReadSize)
         {
                   *pReadSize = totalRead;
         }
         bResult = TRUE;
 
__TERMINAL:
         return bResult;
}
 
//A local helper routine to restore 2 bytes content in the end of a sector by
//replace it using data in Index Block header,since these 2 bytes is written
//an update serial number by NTFS driver.
static BOOL RestoreIBSectContent(__NTFS_FILE_SYSTEM* pFileSystem,BYTE* pIndexBlock)
{
         UINT_16            usn;
         UINT_16*           pUSA          = NULL;  //Update serial array.
         BYTE*              pUSNOff       = NULL;
         UINT_32            sectnum;
         static BYTE        indexSig[] = {'I','N','D','X'};  //Index block's signature.
         UINT_32            i;
 
         if((NULL == pFileSystem) || (NULL == pIndexBlock))
         {
                   return FALSE;
         }
         //Check if the given block is a valid index block.It is very important since the
         //allocated cluster for Index Allocation attribute may not be used at all.
         for(i = 0;i < 4;i ++)
         {
                   //printf("pIndexBlock[%d] = %d,indexSig[%d] = %d\r\n",i,pIndexBlock[i],i,indexSig[i]);
                   if(pIndexBlock[i] != indexSig[i])
                   {
                            return FALSE;
                   }
         }
 
         pUSNOff =  pIndexBlock;
         pUSNOff += IB_USN_OFFSET(pIndexBlock);
         usn     =  *(UINT_16*)pUSNOff;
         pUSNOff += sizeof(UINT_16);  //pUSNOff now pointing to the actual content of sectors.
         pUSA    = (UINT_16*)pUSNOff;
         pUSNOff = pIndexBlock;
         //Calculate how many sectors one file record occupies.
         //sectnum = FR_SIZE_ALLOC(pFileRecord) / pFileSystem->bytesPerSector;
         sectnum = IB_USN_SIZE(pIndexBlock);
         //Update all sector's last 2 bytes.But the verification should be done to check if
         //the sector's content is valid before update.
         for(i = 0;i < sectnum;i ++)
         {
                   pUSNOff = pIndexBlock + pFileSystem->bytesPerSector * (i + 1);
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
 
//Get an Index Entry from a specific index block,a start searching position is specified
//by startPos variable.Return the base address of the Index Entry if find,else NULL is
//returned to indicate the failure.
static BYTE* GetIEFromBlock(__NTFS_FILE_SYSTEM* pFileSystem,BYTE* pIndexBlock,UINT_32* pOffset)
{
         BOOL        bResult    = FALSE;
         static BYTE indexSig[] = {'I','N','D','X'};  //Index block's signature.
         BYTE*       pIndexEntry = NULL;
         UINT_32     IEOffset    = 0;     //Offset of the desired Index Entry in current index block.
         int         i;
 
         if((NULL == pFileSystem) || (NULL == pIndexBlock) || (NULL == pOffset))
         {
                   goto __TERMINAL;
         }
         //Check if the given block is a valid one.
         for(i = 0;i < 4;i ++)
         {
                   //printf("pIndexBlock[%d] = %d,indexSig[%d] = %d\r\n",i,pIndexBlock[i],i,indexSig[i]);
                   if(pIndexBlock[i] != indexSig[i])
                   {
                            goto __TERMINAL;
                   }
         }
         //printf("GetIEFromBlock : index offset = %d\r\n",*pOffset);
         //Check the validate of offset.
         if(*pOffset >= pFileSystem->DirBlockSize)
         {
                   goto __TERMINAL;
         }
         //If this is the first time to search this block.
         if(0 == *pOffset)
         {
                   //Restore the Index Block's sector rear 2 bytes.
                   //if(!RestoreIBSectContent(pFileSystem,pIndexBlock))
                   //{
                   //      goto __TERMINAL;
                   //}
                   IEOffset = IB_ENTRY_OFFSET(pIndexBlock);
                   if(0 == IEOffset)  //Exception case.
                   {
                            goto __TERMINAL;
                   }
                   pIndexEntry = pIndexBlock + IEOffset;  //Now pIndexEntry pointing to the desired IE.
                   //Check if this is the last entry.
                   if(IE_FLAGS_LAST == IE_ENTRY_FLAGS(pIndexEntry))
                   {
                            pIndexEntry = NULL;
                            goto __TERMINAL;
                   }
                   else  //Not the last one.
                   {
                            if(0 == IE_ENTRY_SIZE(pIndexEntry))  //Invalid index entry.
                            {
                                     pIndexEntry = NULL;
                                     goto __TERMINAL;
                            }
                            *pOffset = IE_ENTRY_SIZE(pIndexEntry) + IEOffset; //Update the offset.
                            goto __TERMINAL;
                   }
         }
         else  //Not from begining.
         {
                   pIndexEntry = pIndexBlock + (*pOffset);
                   if(IE_FLAGS_LAST == IE_ENTRY_FLAGS(pIndexEntry))  //Last index entry.
                   {
                            pIndexEntry = NULL;
                            goto __TERMINAL;
                   }
                   else
                   {
                            if(0 == IE_ENTRY_SIZE(pIndexEntry))  //Invalid index entry.
                            {
                                     pIndexEntry = NULL;
                                     goto __TERMINAL;
                            }
                            *pOffset += IE_ENTRY_SIZE(pIndexEntry);  //Update offset to point to next one.
                            goto __TERMINAL;
                   }
         }
 
__TERMINAL:
         return pIndexEntry;
}
 
//Get file's name from an Index Entry object.
static BOOL GetFileName(BYTE* pIndexEntry,WCHAR* pFileName)
{
         int     i           = 0;
         BYTE*   pStream     = NULL;  //Pointing to file name attribute in Index Entry.
         UINT_16 streamSize  = 0;
         UINT_16 nameLength  = 0;
 
         if((NULL == pIndexEntry) || (NULL == pFileName))
         {
                   return FALSE;
         }
         pStream = pIndexEntry + IE_STREAM_OFFSET;
         streamSize = IE_ENTRYDATA_SIZE(pIndexEntry);
         nameLength = FN_NAME_SIZE(pStream);
         pStream += FN_NAME_OFFSET;
         for(i = 0;i < nameLength;i ++)
         {
                   pFileName[i] = ((WCHAR*)pStream)[i];
         }
         pFileName[i] = 0;  //Set the end of file name.
         return TRUE;
}
 
//A local helper routine used to search a given index block to find a specified file.
//FALSE will be returned if can not find.
static BOOL FindFileInBlock(__NTFS_FILE_SYSTEM* pFileSystem,BYTE* pIndexBlock,WCHAR* pszFileName,
                                                                 UINT_32* pFRIndex)
{
         UINT_32         offset = 0;
         BYTE*           pIndexEntry = NULL;
         WCHAR           fileName[255];
 
         if((NULL == pFileSystem) || (NULL == pIndexBlock) || (NULL == pszFileName) || (NULL == pFRIndex))
         {
                   return FALSE;
         }
         while((pIndexEntry = GetIEFromBlock(pFileSystem,pIndexBlock,&offset)) != NULL)
         {
                   if(GetFileName(pIndexEntry,fileName))
                   {
                            //printf("FindFileInBlock:srcfn = ");
                            //wprintf(fileName);
                            //printf(",dstfn = ");
                            //wprintf(pszFileName);
                            //printf("\r\n");
                            if(0 == wstrcmp(pszFileName,fileName))
                            {
                                     *pFRIndex = IE_FR_NUMBER(pIndexEntry);
                                     return TRUE;
                            }
                   }  //Exception case.
                   else
                   {
                            return FALSE;
                   }
         }
         return FALSE;
}
 
//Search a specified file in a given directory and return it's file record index in MFT.
//FALSE will be returned if failed.
BOOL FindFile(__NTFS_FILE_OBJECT* pDirectory,WCHAR* pwszFileName,UINT_32* pFRIndex)
{
         BOOL             bResult        = FALSE;
         UINT_32          readSize       = 0;
         BYTE*            pBuffer        = NULL;
 
         if((NULL == pDirectory) || (NULL == pwszFileName) || (NULL == pFRIndex))
         {
                   goto __TERMINAL;
         }
         //Allocate buffer for temporary usage.
         pBuffer = (BYTE*)__MEM_ALLOC(pDirectory->pFileSystem->DirBlockSize);
         if(NULL == pBuffer)
         {
                   goto __TERMINAL;
         }
         //Now read the directory's content and search it.
         NtfsSetFilePointer(pDirectory,0,NULL,0);  //Set current position to begining.
         while(NtfsReadFile(pDirectory,pBuffer,pDirectory->pFileSystem->DirBlockSize,&readSize,NULL))
         {
                   if(0 == readSize)
                   {
                            goto __TERMINAL;
                   }
                   if(!RestoreIBSectContent(pDirectory->pFileSystem,pBuffer))
                   {
                            printf("Failed to reset the index block's original content.\r\n");
                            goto __TERMINAL;
                   }
                   if(FindFileInBlock(pDirectory->pFileSystem,pBuffer,pwszFileName,pFRIndex))
                   {
                            bResult = TRUE;
                            goto __TERMINAL;
                   }
         }
__TERMINAL:
         if(NULL != pBuffer)
         {
                   __MEM_FREE(pBuffer);
         }
         return bResult;
}
 
//A local helper routine to store an Index Entry's essential information to FS_FIND_DATA object.
static void StoreStream(BYTE* pIndexEntry,FS_FIND_DATA* pFindData)
{
         BYTE*        pStream     = NULL;
         UINT_32      index       = 0;
         UINT_32      nameSize    = 0;
 
         if((NULL == pIndexEntry) || (NULL == pFindData))
         {
                   return;
         }
         pStream = pIndexEntry + IE_STREAM_OFFSET;
         nameSize = FN_NAME_SIZE(pStream);
         //Store file times information.
         memcpy((void*)&pFindData->ftCreationTime,(pStream + FN_CTIME_OFFSET),8);
         memcpy((void*)&pFindData->ftLastWriteTime,(pStream + FN_ATIME_OFFSET),8);
         memcpy((void*)&pFindData->ftLastAccessTime,(pStream + FN_LATIME_OFFSET),8);
 
         pFindData->nFileSizeLow = FN_VALID_SIZE(pStream);
         pFindData->dwFileAttribute = FN_DOS_ATTR(pStream);
         //Process FILE NAME now.
         for(index = 0;(index < nameSize) && (index < MAX_FILE_NAME_LEN);index ++)
         {
                   pFindData->cFileName[index] = (CHAR)((WCHAR*)(pStream + FN_NAME_OFFSET))[index]; //CAUTION!May cause UNICODE problems.
         }
         pFindData->cFileName[index] = 0;  //Set the terminator of file name.
         return;
}
 
//Find the first valid file from a given directory and return it's general information.
//A NTFS_FIND_HANDLE object also is returned to launch the FindNextFile process.
//It's a internal function used only in this module,the external version is NtfsFindFirstFile,without
//the leading under line.
static __NTFS_FIND_HANDLE* __NtfsFindFirstFile(__NTFS_FILE_OBJECT* pDirectory,FS_FIND_DATA* pFindData)
{
         __NTFS_FIND_HANDLE*    pFindHandle  = NULL;
         BOOL                   bResult      = FALSE;
         BYTE*                  pIndexEntry  = NULL;
         BYTE*                  pBuffer      = NULL;  //Cluster buffer.
         UINT_32                readSize     = 0;
 
         if((NULL == pDirectory) || (NULL == pFindData))
         {
                   goto __TERMINAL;
         }
         //Allocate memory for find handle.
         pFindHandle = (__NTFS_FIND_HANDLE*)__MEM_ALLOC(sizeof(__NTFS_FIND_HANDLE));
         if(NULL == pFindHandle)
         {
                   goto __TERMINAL;
         }
         pFindHandle->pDirectory     = pDirectory;
         pFindHandle->nextIndexBlock = 0;
         pFindHandle->ieOffset       = 0;
 
         pBuffer = (BYTE*)__MEM_ALLOC(pDirectory->pFileSystem->DirBlockSize);
         if(NULL == pBuffer)
         {
                   goto __TERMINAL;
         }
         //Try to read the directory's INDEX_ALLOCATION attribute and locate a valid Index Entry.
         NtfsSetFilePointer(pDirectory,0,NULL,0);  //Locate to file's begining.
         while(NtfsReadFile(pDirectory,pBuffer,pDirectory->pFileSystem->DirBlockSize,&readSize,NULL))
         {
                   if(0 == readSize)  //Reach the end of the directory.
                   {
                            printf("NtfsFindFirstFile : readSize = 0\r\n");
                            goto __TERMINAL;
                   }
                   if(!RestoreIBSectContent(pFindHandle->pDirectory->pFileSystem,pBuffer))
                   {
                            goto __TERMINAL;
                   }
                   //Try to fetch a valid Index Entry.
                   if((pIndexEntry = GetIEFromBlock(pDirectory->pFileSystem,pBuffer,&pFindHandle->ieOffset)) != NULL)
                   {
                            //Store the file's information to pFindData from Index Entry's stream.
                            StoreStream(pIndexEntry,pFindData);
                            bResult = TRUE;
                            goto __TERMINAL;
                   }
                   else
                   {
                            //printf("NtfsFindFirstFile : Can not get any valid index entry from block.\r\n");
                   }
                   //Should check the next Index Block.
                   pFindHandle->nextIndexBlock ++;
                   pFindHandle->ieOffset = 0;
         }
__TERMINAL:
         if(NULL != pBuffer)
         {
                   __MEM_FREE(pBuffer);
         }
         if(!bResult)
         {
                   if(NULL != pFindHandle)
                   {
                            __MEM_FREE(pFindHandle);
                   }
                   pFindHandle = NULL;
         }
         return pFindHandle;
}
 
//Find first file in a given directory,return the find handle and fill the pFindData by using
//the first entry in target directory.
__NTFS_FIND_HANDLE* NtfsFindFirstFile(__NTFS_FILE_SYSTEM* pFileSystem,const WCHAR* pDirName,FS_FIND_DATA* pFindData)
{
         __NTFS_FILE_OBJECT*    pCurrDir     = NULL;
         __NTFS_FIND_HANDLE*    pFindHandle  = NULL;
         WCHAR                  DirName[MAX_FILE_NAME_LEN];
         UINT_32                level        = 0;
         UINT_32                frIndex      = 0;
         UINT_32                i;
 
         if((NULL == pFileSystem) || (NULL == pDirName) || (NULL == pFindData))
         {
                   goto __TERMINAL;
         }
         //Get directory's level number.
         if(!GetFullNameLevel((WCHAR*)pDirName,&level))  //Maybe caused by illeagal file name format.
         {
                   goto __TERMINAL;
         }
         printf("  NtfsFindFirstFile : Target directory name's level is %d\r\n",level);
         //Try to open the target directory level by level.
         pCurrDir = pFileSystem->pRootDir;
         for(i = 0;i < level;i ++)
         {
                   if(!GetSubDirectory((WCHAR*)pDirName,i + 1,DirName))
                   {
                            goto __TERMINAL;
                   }
                   //Try to open it in current directory.
                   if(!FindFile(pCurrDir,DirName,&frIndex))
                   {
                            goto __TERMINAL;
                   }
                   //Find the subdirectory,try to open it,but we should release
                   //current directory object since it will be replaced by the new created one.
                   //One exception is that the current directory is root,then we should not
                   //release it.
                   if(pCurrDir != pFileSystem->pRootDir)
                   {
                            NtfsDestroyFile(pCurrDir);
                   }
                   pCurrDir = NtfsCreateFileByFRN(pFileSystem,frIndex);
                   if(NULL == pCurrDir)
                   {
                            goto __TERMINAL;
                   }
                   //OK,continue to search next level subdirectory.
				   DumpFileObject(pCurrDir);
         }
         //If reach here it means that the target subdirectory has been found and opened,
         //so we call __NtfsFindFirstFile to search it.
         pFindHandle = __NtfsFindFirstFile(pCurrDir,pFindData);
 
__TERMINAL:
         return pFindHandle;
}
 
//Find next file in a given directory.The directory's information is stored in __NTFS_FIND_HANDLE object.
BOOL NtfsFindNextFile(__NTFS_FIND_HANDLE* pFindHandle,FS_FIND_DATA* pFindData)
{
         BYTE*   pBuffer     = NULL;
         BYTE*   pIndexEntry = NULL;
         UINT_32 readSize    = 0;
         BOOL    bResult     = FALSE;
         UINT_32 blockOff    = 0;     //Next Index Entry's offset in the whole directory file.
 
         if((NULL == pFindHandle) || (NULL == pFindData))
         {
                   goto __TERMINAL;
         }
         pBuffer = (BYTE*)__MEM_ALLOC(pFindHandle->pDirectory->pFileSystem->DirBlockSize);
         if(NULL == pBuffer)
         {
                   goto __TERMINAL;
         }
         blockOff = pFindHandle->pDirectory->pFileSystem->DirBlockSize * pFindHandle->nextIndexBlock;
         NtfsSetFilePointer(pFindHandle->pDirectory,blockOff,NULL,0);
         while(NtfsReadFile(pFindHandle->pDirectory,pBuffer,pFindHandle->pDirectory->pFileSystem->DirBlockSize,
                   &readSize,NULL))
         {
                   if(0 == readSize)  //Reach the end of the file.
                   {
                            goto __TERMINAL;
                   }
                   if(!RestoreIBSectContent(pFindHandle->pDirectory->pFileSystem,pBuffer))
                   {
                            goto __TERMINAL;
                   }
                   //Try to fetch a valid Index Entry.
                   if((pIndexEntry = GetIEFromBlock(pFindHandle->pDirectory->pFileSystem,pBuffer,&pFindHandle->ieOffset)) != NULL)
                   {
                            //Store the file's information to pFindData from Index Entry's stream.
                            StoreStream(pIndexEntry,pFindData);
                            bResult = TRUE;
                            goto __TERMINAL;
                   }
                   //Check next Index Block.
                   pFindHandle->nextIndexBlock ++;
                   pFindHandle->ieOffset = 0;
         }
__TERMINAL:
         if(NULL != pBuffer)
         {
                   __MEM_FREE(pBuffer);
         }
         return bResult;
}
 
//Close find handle object.
void NtfsCloseFind(__NTFS_FIND_HANDLE* pFindHandle)
{
         if(NULL == pFindHandle)
         {
                   return;
         }
         if(NULL != pFindHandle->pDirectory)  //Should close it if it is not the root directory object.
         {
                   if(pFindHandle->pDirectory->pFileSystem->pRootDir != pFindHandle->pDirectory)
                   {
                            NtfsDestroyFile(pFindHandle->pDirectory);
                   }
         }
         __MEM_FREE(pFindHandle);
}
 
//A debugging routine to list all files in a given directory object.
void ListDirFile(__NTFS_FILE_SYSTEM* pFileSystem,const WCHAR* pDirName)
{
         __NTFS_FIND_HANDLE*     pFindHandle  = NULL;
         FS_FIND_DATA            FindData;
         BOOL                    bNextFind    = TRUE;
 
         pFindHandle = NtfsFindFirstFile(pFileSystem,pDirName,&FindData);
         if(NULL == pFindHandle)
         {
                   printf("Can not find any file in the given directory.\r\n");
                   return;
         }
         //OK,launch the iterate finding process.
         printf("  \r\n--------------------------------------------------------- \r\n");
         while(bNextFind)
         {
                   printf("   %32s  x%08X    %8d\r\n",
                            FindData.cFileName,
                            FindData.dwFileAttribute,
                            FindData.nFileSizeLow);
                   bNextFind = NtfsFindNextFile(pFindHandle,&FindData);
         }
         NtfsCloseFind(pFindHandle);
}
 
//A debugging routine used to enumerate all file's in a index block.
void EnumIndexBlock(__NTFS_FILE_SYSTEM* pFileSystem,BYTE* pIndexBlock)
{
         UINT_32         offset   = 0;
         BYTE*           pIndexEntry = NULL;
         WCHAR           fileName[255];
 
         if(NULL == pIndexBlock)
         {
                   return;
         }
         printf("\r\n");
         while((pIndexEntry = GetIEFromBlock(pFileSystem,pIndexBlock,&offset)) != NULL)
         {
                   printf("----------- Index Block's information -------------\r\n");
                   if(GetFileName(pIndexEntry,fileName))
                   {
                            wprintf(fileName);
                            printf("\r\n");
                            /*printf("\r\n");
                            for(i = 0;i < 255;i ++)
                            {
                                     if(0 == fileName[i])
                                     {
                                               printf("\r\n");
                                               break;
                                     }
                                     printf("%c",fileName[i]);
                            }*/
                   }
                   else
                   {
                            printf("Error:Get Index Entry successfully but can not get file name.\r\n");
                   }
         }
         printf("\r\n");
}
