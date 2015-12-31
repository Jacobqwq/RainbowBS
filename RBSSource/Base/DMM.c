/*
----------------------------------------------------------------------
                  RainbowBS File : DMM.c
----------------------------------------------------------------------
*/

/*
 * Copyright (C) 2015 QWQ(jacobqwq@icloud.com).
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * - The name of the author may not be used to endorse or promote
 *   products derived from this software without specific prior
 *   written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDERS AND CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "../RainbowBS.h"
#include "../Port/Port.h"

typedef enum {
  DM_NONE,
  DM_BLOCK,   /**< fixed size blocks */
  DM_POOL,    /**< memory pool */
  DM_POOL_AUTO/**< memory pool with anti-fragmentation */
} eDMTYPE;

typedef struct {
  eDMTYPE eType;/**< DMM type */
  char *pName;/**< DMM name */
  void *pObjectArrayStart;/**< pointer of the first array(tBLOCK or tMEM) object */
  void *pObjectArrayLast;/**< pointer of the last array(tBLOCK or tMEM) object */
  void *pFreeObject;/**< pointer for accelerating allocation */
  USIZE free_object_count;/**< count of free array(tBLOCK or tMEM) objects */
  USIZE free_bytes;/**< count of free bytes if eType is DM_POOL or DM_POOL_AUTO */
  U8 *pDataStart;/**< pointer of data head */
  U8 *pDataEnd;/**< pointer of data end */
  USIZE block_size;/**< block size if eType is DM_BLOCK */
#if (RBS_CFG_APP_MODEL == RBS_APP_OS)
  HMUTEX hMutex;/**< lock */
#endif
} tDMMHEAD;

typedef struct {
  hDMM hDmm;  /**< DMM handle */
  U8 *pData;  /**< pointer of data area */
  U8 lock_count; /**< 0:not allocated,1:allocated,[2,255]:being used lock_count-1 times */
} tBLOCK;

typedef struct MEM_struct{
  hDMM hDmm;  /**< DMM handle */
  U8 *pData;  /**< pointer of memory area */
  USIZE size; /**< usable size of allocated block */
  struct MEM_struct *pNext;/**< next handle in linked list */
  struct MEM_struct *pPrev;/**< prev handle in linked list */
  U8 lock_count;/**< 0:not allocated,1:allocated,[2,255]:being used lock_count-1 times.*/
} tMEM;

#define DMM_VALID(ptDmmHead,type,object_type) ((NULL != (ptDmmHead)) && (type == (ptDmmHead)->eType) \
                                             && (NULL != (ptDmmHead)->pObjectArrayStart) && (NULL != (ptDmmHead)->pObjectArrayLast) \
                                             && (NULL != (ptDmmHead)->pDataStart) && (NULL != (ptDmmHead)->pDataEnd) \
                                             && ((U8*)((object_type*)(ptDmmHead)->pObjectArrayLast + 1) <= (ptDmmHead)->pDataStart) \
                                             && ((ptDmmHead)->pDataStart <= (ptDmmHead)->pDataEnd) && (NULL != (ptDmmHead)->pFreeObject))
#define DMM_OBJECT_COUNT(ptDmmHead,object_type) ((object_type*)(ptDmmHead)->pObjectArrayLast - (object_type*)(ptDmmHead)->pObjectArrayStart + 1)
#define BLOCK_VALID(ptBlock)  ((NULL != ptBlock) && (NULL != ptBlock->hDmm) && (NULL != ptBlock->pData) \
                               && (((U8*)ptBlock - (U8*)((tDMMHEAD*)ptBlock->hDmm)->pObjectArrayStart) % sizeof(tBLOCK) == 0) \
                               && ((U8*)ptBlock >= (U8*)((tDMMHEAD*)ptBlock->hDmm)->pObjectArrayStart) \
                               && ((U8*)ptBlock <= (U8*)((tDMMHEAD*)ptBlock->hDmm)->pObjectArrayLast))
#define BLOCK_ALLOC(ptBlock)  (0 != ptBlock->lock_count)
#define MEM_VALID(ptMem)  ((NULL != ptMem) && (NULL != ptMem->hDmm) \
                           && (((U8*)ptMem - (U8*)((tDMMHEAD*)ptMem->hDmm)->pObjectArrayStart) % sizeof(tMEM) == 0) \
                           && ((U8*)ptMem >= (U8*)((tDMMHEAD*)ptMem->hDmm)->pObjectArrayStart + sizeof(tMEM)) \
                           && ((U8*)ptMem <= (U8*)((tDMMHEAD*)ptMem->hDmm)->pObjectArrayLast))
#define MEM_ALLOC(ptMem)  ((0 != ptMem->lock_count) && (0 != ptMem->size) && (NULL != ptMem->pData) && (NULL != ptMem->pPrev))

/*
********************************************************************
*                    RBS_DMM_RegisterBlock
********************************************************************
*/

#if (RBS_CFG_APP_MODEL == RBS_APP_OS)
hDMM RBS_DMM_RegisterBlock(char *pName,void *pDM,USIZE size,USIZE block_size,HMUTEX hMutex) {
#else
hDMM RBS_DMM_RegisterBlock(char *pName,void *pDM,USIZE size,USIZE block_size) {
#endif
  tDMMHEAD *ptDmmHead;
  U16 i;
  tBLOCK *ptBlock;
  RBS_DEBUG_ERROR((NULL == pDM) || (size <= sizeof(tDMMHEAD)) || (0 == block_size),"invalid para.",return HDMM_NULL);
  block_size = (block_size + ((1u<<RBS_CFG_DMM_ALIGN) - 1u)) & (~((1u<<RBS_CFG_DMM_ALIGN) - 1u));
  /* adjust to alignment boundary */
  ptDmmHead = (tDMMHEAD*)ALIGN_F(pDM,sizeof(USIZE));
  RBS_DEBUG_ERROR((U8*)pDM + size <= (U8*)ptDmmHead + sizeof(tDMMHEAD),"size is not enough.",return HDMM_NULL);
  ptDmmHead->eType = DM_BLOCK;
  ptDmmHead->pFreeObject = ptDmmHead->pObjectArrayStart = (void*)ALIGN_F((U8*)ptDmmHead + sizeof(tDMMHEAD),sizeof(USIZE));
  ptDmmHead->pDataEnd = (U8*)ALIGN_B((U8*)pDM + size - 1,1u<<RBS_CFG_DMM_ALIGN);
  ptDmmHead->free_object_count = (ptDmmHead->pDataEnd + 1 - (U8*)ptDmmHead->pObjectArrayStart) / (sizeof(tBLOCK) + block_size);
  ptDmmHead->pDataStart = (U8*)ALIGN_F((tBLOCK*)ptDmmHead->pObjectArrayStart + ptDmmHead->free_object_count,1u<<RBS_CFG_DMM_ALIGN);
  ptDmmHead->pObjectArrayLast = (tBLOCK*)ptDmmHead->pObjectArrayStart + (ptDmmHead->free_object_count - 1);
  RBS_DEBUG_ERROR(ptDmmHead->pDataStart >= ptDmmHead->pDataEnd,"size is not enough.",return HDMM_NULL);
#if (RBS_CFG_APP_MODEL == RBS_APP_OS)
  ptDmmHead->hMutex = hMutex;
#endif
  ptDmmHead->block_size = block_size;
  /* initilize tBLOCK array */
  ptBlock = (tBLOCK*)ptDmmHead->pObjectArrayStart;
  for (i = 0;i < DMM_OBJECT_COUNT(ptDmmHead,tBLOCK);i++,ptBlock++) {
    ptBlock->hDmm = (hDMM)ptDmmHead;
    ptBlock->pData = ptDmmHead->pDataStart + i * block_size;
    ptBlock->lock_count = 0;
  }
  if ((NULL == pName) || (0 == *pName))
    ptDmmHead->pName = "Unnamed Block";
  else
    ptDmmHead->pName = pName;
  RBS_DEBUG_LOG_FORMAT("Registe Block %s OK!(free block count:%d,block bytes:%d).",ptDmmHead->pName,ptDmmHead->free_object_count,ptDmmHead->block_size);
  return (hDMM)ptDmmHead;
}

/*
********************************************************************
*                     RBS_DMM_AllocZeroBlock
********************************************************************
*/

hBLOCK RBS_DMM_AllocZeroBlock(hDMM hDmm) {
  hBLOCK r = RBS_DMM_AllocBlock(hDmm);
  if (HBLOCK_NULL != r) {
    void *pBlock = RBS_DMM_UseHBlock(r);
    tDMMHEAD *ptDmmHead = (tDMMHEAD*)hDmm;
    RBS_MemSet8(pBlock,0,ptDmmHead->block_size);
    RBS_DMM_UnuseHBlock(r);
  }
  return r;
}

/*
********************************************************************
*                     RBS_DMM_AllocBlock
********************************************************************
*/

hBLOCK RBS_DMM_AllocBlock(hDMM hDmm) {
  tBLOCK *ptBlock = HBLOCK_NULL;
  tDMMHEAD *ptDmmHead = (tDMMHEAD*)hDmm;
  RBS_DEBUG_ERROR(!DMM_VALID(ptDmmHead,DM_BLOCK,tBLOCK),"invalid handle.",return HBLOCK_NULL);
#if (RBS_CFG_APP_MODEL == RBS_APP_OS)
  if (ptDmmHead->hMutex != NULL) {
    RBS_DEBUG_STA(BOOL r);
    RBS_DEBUG_STA(r =) Port_GetMutex(ptDmmHead->hMutex);
    RBS_DEBUG_ASSERT(r);
  }
#endif
  if (0 != ptDmmHead->free_object_count) {
    ptBlock = (tBLOCK*)ptDmmHead->pFreeObject;
    while (0 != ptBlock->lock_count) {
      if (ptBlock == (tBLOCK*)ptDmmHead->pObjectArrayLast)
        ptBlock = (tBLOCK*)ptDmmHead->pObjectArrayStart;
      else
        ptBlock += 1;
    }
    RBS_DEBUG_ASSERT(0 == ptBlock->lock_count);
    ptBlock->lock_count = 1;
    ptDmmHead->free_object_count--;
    ptDmmHead->pFreeObject = (void*)((ptBlock == (tBLOCK*)ptDmmHead->pObjectArrayLast) ? ptDmmHead->pObjectArrayStart : ptBlock + 1);
  }
  RBS_DEBUG_IF(HBLOCK_NULL != ptBlock,;);
  RBS_DEBUG_LOG_FORMAT("%s:Alloc Block OK!(free block count:%d,block bytes:%d).",ptDmmHead->pName,ptDmmHead->free_object_count,ptDmmHead->block_size);
  RBS_DEBUG_ELSIF(;,TRUE,;);
  RBS_DEBUG_LOG_FORMAT("%s:Alloc Block Failed!(free block count:%d,block bytes:%d).",ptDmmHead->pName,ptDmmHead->free_object_count,ptDmmHead->block_size);
  RBS_DEBUG_ENDIF(;);
#if (RBS_CFG_APP_MODEL == RBS_APP_OS)
  if (ptDmmHead->hMutex != NULL) {
    RBS_DEBUG_STA(BOOL r);
    RBS_DEBUG_STA(r =) Port_FreeMutex(ptDmmHead->hMutex);
    RBS_DEBUG_ASSERT(r);
  }
#endif
  return ptBlock;
}

/*
********************************************************************
*                     RBS_DMM_FreeBlock
********************************************************************
*/

BOOL RBS_DMM_FreeBlock(hBLOCK hBlock) {
  BOOL r = TRUE;
  tBLOCK *ptBlock = (tBLOCK*)hBlock;
  tDMMHEAD *ptDmmHead;
  RBS_DEBUG_ERROR(NULL == ptBlock,"invalid handle.",return FALSE);
  ptDmmHead = (tDMMHEAD*)(ptBlock->hDmm);
  RBS_DEBUG_ERROR(!DMM_VALID(ptDmmHead,DM_BLOCK,tBLOCK),"invalid handle.",return FALSE);
#if (RBS_CFG_APP_MODEL == RBS_APP_OS)
  if (ptDmmHead->hMutex != NULL) {
    RBS_DEBUG_STA(r =) Port_GetMutex(ptDmmHead->hMutex);
    RBS_DEBUG_ASSERT(r);
  }
#endif
  RBS_DEBUG_ERROR(!BLOCK_VALID(ptBlock) || !BLOCK_ALLOC(ptBlock),"invalid handle.",r = FALSE);
  RBS_DEBUG_ERROR(r && (1 != ptBlock->lock_count),"handle is using.",r = FALSE);
  RBS_DEBUG_IF(r,;);
  ptBlock->lock_count = 0;
  ptDmmHead->pFreeObject = (void*)ptBlock;
  ptDmmHead->free_object_count++;
  RBS_DEBUG_LOG_FORMAT("%s:Free Block OK!(free block count:%d,block bytes:%d).",ptDmmHead->pName,ptDmmHead->free_object_count,ptDmmHead->block_size);
  RBS_DEBUG_ELSIF(;,TRUE,;);
  RBS_DEBUG_LOG_FORMAT("%s:Free Block Failed!(free block count:%d,block bytes:%d).",ptDmmHead->pName,ptDmmHead->free_object_count,ptDmmHead->block_size);
  RBS_DEBUG_ENDIF(;);
#if (RBS_CFG_APP_MODEL == RBS_APP_OS)
  if (ptDmmHead->hMutex != NULL) {
    RBS_DEBUG_STA(r =) Port_FreeMutex(ptDmmHead->hMutex);
    RBS_DEBUG_ASSERT(r);
  }
#endif
  return r;
}

/*
********************************************************************
*                     RBS_DMM_UseHBlock
********************************************************************
*/

void* RBS_DMM_UseHBlock(hBLOCK hBlock) {
  void *r = (void*)0xFFFFFFFF;
  tBLOCK *ptBlock = (tBLOCK*)hBlock;
#if (RBS_CFG_APP_MODEL == RBS_APP_OS)
  tDMMHEAD *ptDmmHead;
#else
  RBS_DEBUG_STA(tDMMHEAD *ptDmmHead);
#endif
  RBS_DEBUG_ERROR(NULL == ptBlock,"invalid handle.",return NULL);
#if (RBS_CFG_APP_MODEL == RBS_APP_OS)
  ptDmmHead = (tDMMHEAD*)(ptBlock->hDmm);
#else
  RBS_DEBUG_STA(ptDmmHead = (tDMMHEAD*)(ptBlock->hDmm));
#endif
  RBS_DEBUG_ERROR(!DMM_VALID(ptDmmHead,DM_BLOCK,tBLOCK),"invalid handle.",return NULL);
#if (RBS_CFG_APP_MODEL == RBS_APP_OS)
  if (ptDmmHead->hMutex != NULL) {
    RBS_DEBUG_STA(BOOL b);
    RBS_DEBUG_STA(b =) Port_GetMutex(ptDmmHead->hMutex);
    RBS_DEBUG_ASSERT(b);
  }
#endif
  RBS_DEBUG_ERROR(!BLOCK_VALID(ptBlock) || !BLOCK_ALLOC(ptBlock),"invalid handle.",r = NULL);
  RBS_DEBUG_ERROR((NULL != r) && (255 == ptBlock->lock_count),"handle referance too more.",r = NULL);
  RBS_DEBUG_IF(NULL != r,;);
  ptBlock->lock_count++;
  r = (void*)(ptBlock->pData);
  RBS_DEBUG_ENDIF(;);
#if (RBS_CFG_APP_MODEL == RBS_APP_OS)
  if (ptDmmHead->hMutex != NULL) {
    RBS_DEBUG_STA(BOOL b);
    RBS_DEBUG_STA(b =) Port_FreeMutex(ptDmmHead->hMutex);
    RBS_DEBUG_ASSERT(b);
  }
#endif
  return r;
}

/*
********************************************************************
*                     RBS_DMM_UnuseHBlock
********************************************************************
*/

BOOL RBS_DMM_UnuseHBlock(hBLOCK hBlock) {
  BOOL r = TRUE;
  tBLOCK *ptBlock = (tBLOCK*)hBlock;
#if (RBS_CFG_APP_MODEL == RBS_APP_OS)
  tDMMHEAD *ptDmmHead;
#else
  RBS_DEBUG_STA(tDMMHEAD *ptDmmHead);
#endif
  RBS_DEBUG_ERROR(NULL == ptBlock,"invalid handle.",return FALSE);
#if (RBS_CFG_APP_MODEL == RBS_APP_OS)
  ptDmmHead = (tDMMHEAD*)(ptBlock->hDmm);
#else
  RBS_DEBUG_STA(ptDmmHead = (tDMMHEAD*)(ptBlock->hDmm));
#endif
  RBS_DEBUG_ERROR(!DMM_VALID(ptDmmHead,DM_BLOCK,tBLOCK),"invalid handle.",return FALSE);
#if (RBS_CFG_APP_MODEL == RBS_APP_OS)
  if (ptDmmHead->hMutex != NULL) {
    RBS_DEBUG_STA(r =) Port_GetMutex(ptDmmHead->hMutex);
    RBS_DEBUG_ASSERT(r);
  }
#endif
  RBS_DEBUG_ERROR(!BLOCK_VALID(ptBlock) || !BLOCK_ALLOC(ptBlock),"invalid handle.",r = FALSE);
  RBS_DEBUG_ERROR(r && (1 == ptBlock->lock_count),"handle is not using.",r = FALSE);
  RBS_DEBUG_IF(r,;);
  ptBlock->lock_count--;
  RBS_DEBUG_ENDIF(;);
#if (RBS_CFG_APP_MODEL == RBS_APP_OS)
  if (ptDmmHead->hMutex != NULL) {
    RBS_DEBUG_STA(r =) Port_FreeMutex(ptDmmHead->hMutex);
    RBS_DEBUG_ASSERT(r);
  }
#endif
  return r;
}

/*
********************************************************************
*                     RBS_DMM_RegisterPool
********************************************************************
*/

#if (RBS_CFG_APP_MODEL == RBS_APP_OS)
hDMM RBS_DMM_RegisterPool(char *pName,void *pDM,USIZE size,BOOL bAntiFrag,U16 handle_count,HMUTEX hMutex) {
#else
hDMM RBS_DMM_RegisterPool(char *pName,void *pDM,USIZE size,BOOL bAntiFrag,U16 handle_count) {
#endif
  tDMMHEAD *ptDmmHead;
  U16 i;
  tMEM *ptMEM;
  RBS_DEBUG_ERROR((NULL == pDM) || (size <= (sizeof(tDMMHEAD) + handle_count * sizeof(tMEM)) || (0 == handle_count)),"invalid para.",return HDMM_NULL);
  handle_count++;
  /* adjust to alignment boundary */
  ptDmmHead = (tDMMHEAD*)ALIGN_F(pDM,sizeof(USIZE));
  RBS_DEBUG_ERROR((U8*)pDM + size <= (U8*)ptDmmHead + sizeof(tDMMHEAD),"size is not enough.",return HDMM_NULL);
  ptDmmHead->eType = bAntiFrag ? DM_POOL_AUTO : DM_POOL;
  ptDmmHead->pObjectArrayStart = (void*)ALIGN_F((U8*)ptDmmHead + sizeof(tDMMHEAD),sizeof(USIZE));
  ptDmmHead->pFreeObject = (tMEM*)ptDmmHead->pObjectArrayStart + 1;
  ptDmmHead->pObjectArrayLast = (tMEM*)ptDmmHead->pObjectArrayStart + (handle_count - 1);
  ptDmmHead->pDataStart = (U8*)ALIGN_F((tMEM*)ptDmmHead->pObjectArrayLast + 1,1u<<RBS_CFG_DMM_ALIGN);
  ptDmmHead->pDataEnd = (U8*)ALIGN_B((U8*)pDM + size - 1,1u<<RBS_CFG_DMM_ALIGN);
  RBS_DEBUG_ERROR(ptDmmHead->pDataStart > ptDmmHead->pDataEnd,"size is not enough.",return HDMM_NULL);
#if (RBS_CFG_APP_MODEL == RBS_APP_OS)
  ptDmmHead->hMutex = hMutex;
#endif
  ptDmmHead->free_object_count = handle_count - 1;
  ptDmmHead->free_bytes = ptDmmHead->pDataEnd - ptDmmHead->pDataStart + 1;
  /* initialize tMEM array */
  ptMEM = (tMEM*)ptDmmHead->pObjectArrayStart;
  for (i = 0;i < handle_count;i++,ptMEM++) {
    ptMEM->hDmm = (hDMM)ptDmmHead;
    ptMEM->lock_count = 0;
    ptMEM->pNext = ptMEM->pPrev = HMEM_NULL;
    ptMEM->pData = NULL;
    ptMEM->size = 0;
  }
  /* first handle used as head */
  ((tMEM*)ptDmmHead->pObjectArrayStart)->pData = ptDmmHead->pDataStart;
  if ((NULL == pName) || (0 == *pName))
    ptDmmHead->pName = "Unnamed Pool";
  else
    ptDmmHead->pName = pName;
  RBS_DEBUG_LOG_FORMAT("%s:Registe Pool OK!(free handle count:%d,free bytes:%d).",ptDmmHead->pName,ptDmmHead->free_object_count,ptDmmHead->free_bytes);
  return (hDMM)ptDmmHead;
}

/*
********************************************************************
*                     RBS_DMM_AllocZeroMem
********************************************************************
*/

hMEM RBS_DMM_AllocZeroMem(hDMM hDmm,USIZE size) {
  hMEM r = RBS_DMM_AllocMem(hDmm,size);
  if (HMEM_NULL != r) {
    void *pMem = RBS_DMM_UseHMem(r);
    RBS_MemSet8(pMem,0,size);
    RBS_DMM_UnuseHMem(r);
  }
  return r;
}

/*
********************************************************************
*                     RBS_DMM_AllocMem
********************************************************************
*/

static tMEM *_Defragmentation(USIZE size) {

  return HMEM_NULL;
}

static void _Alloc(tDMMHEAD *ptDmmHead,USIZE size,tMEM **pptMem) {
  tMEM *ptMem = HMEM_NULL;
  if ((0 != ptDmmHead->free_object_count) && (size <= ptDmmHead->free_bytes)) {
    tMEM *pPrev = HMEM_NULL,*pNext;
    ptMem = (tMEM*)ptDmmHead->pFreeObject;
    while (0 != ptMem->lock_count) {
      if (ptMem == ptDmmHead->pObjectArrayLast)
        ptMem = (tMEM*)ptDmmHead->pObjectArrayStart + 1;
      else
        ptMem += 1;
    }
    RBS_DEBUG_ASSERT(0 == ptMem->lock_count);
    /* locate hole of sufficient size */
    for (pPrev = ptDmmHead->pObjectArrayStart;(pNext = pPrev->pNext) != HMEM_NULL;pPrev = pNext) {
      if (pNext->pData >= (pPrev->pData + pPrev->size) + size)
        break;
    }
    if (HMEM_NULL == pNext) {
    if ((ptDmmHead->pDataEnd + 1) < (pPrev->pData + pPrev->size) + size)
      pPrev = HMEM_NULL;
    }
    /* automatic defragmentation to create hole of sufficient size */
    if ((HMEM_NULL == pPrev) && (DM_POOL_AUTO == ptDmmHead->eType))
      pPrev = _Defragmentation(size);
    /* occupy hole */
    if (HMEM_NULL != pPrev) {
      ptMem->pNext = pPrev->pNext;
      pPrev->pNext = ptMem;
      if (ptMem->pNext != HMEM_NULL)
        ptMem->pNext->pPrev = ptMem;
      ptMem->pPrev = pPrev;
      ptMem->pData = pPrev->pData + pPrev->size;
      ptMem->size = size;
      ptMem->lock_count = 1;
      ptDmmHead->free_object_count--;
      ptDmmHead->free_bytes -= size;
      ptDmmHead->pFreeObject = (void*)((ptMem == (tMEM*)ptDmmHead->pObjectArrayLast) ? ptDmmHead->pObjectArrayStart : ptMem + 1);
    } else {
      ptMem = HMEM_NULL;
    }
  }
  *pptMem = ptMem;
}

hMEM RBS_DMM_AllocMem(hDMM hDmm,USIZE size) {
  tMEM *ptMem = HMEM_NULL;
  tDMMHEAD *ptDmmHead = (tDMMHEAD*)hDmm;
  RBS_DEBUG_ERROR((0 == size) || (!DMM_VALID(ptDmmHead,DM_POOL_AUTO,tMEM) && !DMM_VALID(ptDmmHead,DM_POOL,tMEM)),"invalid para.",return HMEM_NULL);
  size = (size + ((1u<<RBS_CFG_DMM_ALIGN) - 1u)) & (~((1u<<RBS_CFG_DMM_ALIGN) - 1u));
#if (RBS_CFG_APP_MODEL == RBS_APP_OS)
  if (ptDmmHead->hMutex != NULL) {
    RBS_DEBUG_STA(BOOL r);
    RBS_DEBUG_STA(r =) Port_GetMutex(ptDmmHead->hMutex);
    RBS_DEBUG_ASSERT(r);
  }
#endif
  _Alloc(ptDmmHead,size,&ptMem);
  RBS_DEBUG_IF(HMEM_NULL != ptMem,;);
  RBS_DEBUG_LOG_FORMAT("%s:Alloc Mem(%d bytes) OK!(free handle count:%d,free bytes:%d).",ptDmmHead->pName,size,ptDmmHead->free_object_count,ptDmmHead->free_bytes);
  RBS_DEBUG_ELSIF(;,TRUE,;);
  RBS_DEBUG_LOG_FORMAT("%s:Alloc Mem(%d bytes) Failed!(free handle count:%d,free bytes:%d).",ptDmmHead->pName,size,ptDmmHead->free_object_count,ptDmmHead->free_bytes);
  RBS_DEBUG_ENDIF(;);
#if (RBS_CFG_APP_MODEL == RBS_APP_OS)
  if (ptDmmHead->hMutex != NULL) {
    RBS_DEBUG_STA(BOOL r);
    RBS_DEBUG_STA(r =) Port_FreeMutex(ptDmmHead->hMutex);
    RBS_DEBUG_ASSERT(r);
  }
#endif
  return ptMem;
}

/*
********************************************************************
*                      RBS_DMM_ReallocMem
********************************************************************
*/

hMEM RBS_DMM_ReallocMem(hMEM hMem,USIZE size) {
  tMEM *r = HMEM_NULL;
  tMEM *ptMem = (tMEM*)hMem;
  tDMMHEAD *ptDmmHead;
  RBS_DEBUG_STA(BOOL b = TRUE);
  RBS_DEBUG_ERROR((NULL == ptMem) || (0 == size),"invalid para.",return HMEM_NULL);
  size = (size + ((1u<<RBS_CFG_DMM_ALIGN) - 1u)) & (~((1u<<RBS_CFG_DMM_ALIGN) - 1u));
  ptDmmHead = (tDMMHEAD*)(ptMem->hDmm);
  RBS_DEBUG_ERROR(!DMM_VALID(ptDmmHead,DM_POOL,tMEM) && !DMM_VALID(ptDmmHead,DM_POOL_AUTO,tMEM),"invalid handle.",return HMEM_NULL);
#if (RBS_CFG_APP_MODEL == RBS_APP_OS)
  if (ptDmmHead->hMutex != NULL) {
    RBS_DEBUG_STA(b =) Port_GetMutex(ptDmmHead->hMutex);
    RBS_DEBUG_ASSERT(b);
  }
#endif
  RBS_DEBUG_ERROR(!MEM_VALID(ptMem) || !MEM_ALLOC(ptMem),"invalid handle.",b = FALSE);
  RBS_DEBUG_ERROR(b && (1 != ptMem->lock_count),"handle is using.",b = FALSE);
  RBS_DEBUG_IF(b,;);
  if (size == ptMem->size)
    r = hMem;
  else {
    _Alloc(ptDmmHead,size,&r);
    if (HMEM_NULL != r) {
      RBS_MemCpy8(r->pData,ptMem->pData,MIN(ptMem->size,size));
      /*free old handle*/
      ptMem->lock_count = 0;
    ptDmmHead->free_bytes += ptMem->size;
      ptMem->size =0;
      ptMem->pData = NULL;
      ptDmmHead->pFreeObject = (void*)ptMem;
      ptDmmHead->free_object_count++;
      ptMem->pPrev->pNext = ptMem->pNext;
    if (NULL != ptMem->pNext)
        ptMem->pNext->pPrev = ptMem->pPrev;
    }
  }
  RBS_DEBUG_ENDIF(;);
  RBS_DEBUG_IF(HMEM_NULL != r,;);
  RBS_DEBUG_LOG_FORMAT("%s:Realloc Mem(%d bytes) OK!(free handle count:%d,free bytes:%d).",ptDmmHead->pName,size,ptDmmHead->free_object_count,ptDmmHead->free_bytes);
  RBS_DEBUG_ELSIF(;,TRUE,;);
  RBS_DEBUG_LOG_FORMAT("%s:Realloc Mem(%d bytes) Failed!(free handle count:%d,free bytes:%d).",ptDmmHead->pName,size,ptDmmHead->free_object_count,ptDmmHead->free_bytes);
  RBS_DEBUG_ENDIF(;);
#if (RBS_CFG_APP_MODEL == RBS_APP_OS)
  if (ptDmmHead->hMutex != NULL) {
    RBS_DEBUG_STA(b =) Port_FreeMutex(ptDmmHead->hMutex);
    RBS_DEBUG_ASSERT(b);
  }
#endif
  return r;
}

/*
********************************************************************
*                      RBS_DMM_FreeMem
********************************************************************
*/

BOOL RBS_DMM_FreeMem(hMEM hMem) {
  BOOL r = TRUE;
  tMEM *ptMem = (tMEM*)hMem;
  tDMMHEAD *ptDmmHead;
  RBS_DEBUG_ERROR(NULL == ptMem,"invalid handle.",return FALSE);
  ptDmmHead = (tDMMHEAD*)(ptMem->hDmm);
  RBS_DEBUG_ERROR(!DMM_VALID(ptDmmHead,DM_POOL,tMEM) && !DMM_VALID(ptDmmHead,DM_POOL_AUTO,tMEM),"invalid handle.",return FALSE);
#if (RBS_CFG_APP_MODEL == RBS_APP_OS)
  if (ptDmmHead->hMutex != NULL) {
    RBS_DEBUG_STA(r =) Port_GetMutex(ptDmmHead->hMutex);
    RBS_DEBUG_ASSERT(r);
  }
#endif
  RBS_DEBUG_ERROR(!MEM_VALID(ptMem) || !MEM_ALLOC(ptMem),"invalid handle.",r = FALSE);
  RBS_DEBUG_ERROR(r && (1 != ptMem->lock_count),"handle is using.",r = FALSE);
  RBS_DEBUG_IF(r,;);
  ptMem->lock_count = 0;
  ptDmmHead->free_bytes += ptMem->size;
  ptMem->pData = NULL;
  ptDmmHead->pFreeObject = (void*)ptMem;
  ptDmmHead->free_object_count++;
  ptMem->pPrev->pNext = ptMem->pNext;
  if (NULL != ptMem->pNext)
    ptMem->pNext->pPrev = ptMem->pPrev;
  RBS_DEBUG_LOG_FORMAT("%s:Free Mem(%d bytes) OK!(free handle count:%d,free bytes:%d).",ptDmmHead->pName,ptMem->size,ptDmmHead->free_object_count,ptDmmHead->free_bytes);
  ptMem->size = 0;
  RBS_DEBUG_ELSIF(;,TRUE,;);
  RBS_DEBUG_LOG_FORMAT("%s:Free Mem(%d bytes) Failed!(free handle count:%d,free bytes:%d).",ptDmmHead->pName,ptMem->size,ptDmmHead->free_object_count,ptDmmHead->free_bytes);
  RBS_DEBUG_ENDIF(;);
#if (RBS_CFG_APP_MODEL == RBS_APP_OS)
  if (ptDmmHead->hMutex != NULL) {
    RBS_DEBUG_STA(r =) Port_FreeMutex(ptDmmHead->hMutex);
    RBS_DEBUG_ASSERT(r);
  }
#endif
  return r;
}

/*
********************************************************************
*                      RBS_DMM_UseHMem
********************************************************************
*/

void* RBS_DMM_UseHMem(hMEM hMem) {
  void *r = (void*)0xFFFFFFFF;
  tMEM *ptMem = (tMEM*)hMem;
#if (RBS_CFG_APP_MODEL == RBS_APP_OS)
  tDMMHEAD *ptDmmHead;
#else
  RBS_DEBUG_STA(tDMMHEAD *ptDmmHead);
#endif
  RBS_DEBUG_ERROR(NULL == ptMem,"invalid handle.",return NULL);
#if (RBS_CFG_APP_MODEL == RBS_APP_OS)
  ptDmmHead = (tDMMHEAD*)(ptMem->hDmm);
#else
  RBS_DEBUG_STA(ptDmmHead = (tDMMHEAD*)(ptMem->hDmm));
#endif
  RBS_DEBUG_ERROR(!DMM_VALID(ptDmmHead,DM_POOL,tMEM) && !DMM_VALID(ptDmmHead,DM_POOL_AUTO,tMEM),"invalid handle.",return NULL);
#if (RBS_CFG_APP_MODEL == RBS_APP_OS)
  if (ptDmmHead->hMutex != NULL) {
    RBS_DEBUG_STA(BOOL b);
    RBS_DEBUG_STA(b =) Port_GetMutex(ptDmmHead->hMutex);
    RBS_DEBUG_ASSERT(b);
  }
#endif
  RBS_DEBUG_ERROR(!MEM_VALID(ptMem) || !MEM_ALLOC(ptMem),"invalid handle.",r = NULL);
  RBS_DEBUG_ERROR((NULL != r) && (255 == ptMem->lock_count),"handle referance too more.",r = NULL);
  RBS_DEBUG_IF(NULL != r,;);
  ptMem->lock_count++;
  r = (void*)(ptMem->pData);
  RBS_DEBUG_ENDIF(;);
#if (RBS_CFG_APP_MODEL == RBS_APP_OS)
  if (ptDmmHead->hMutex != NULL) {
    RBS_DEBUG_STA(BOOL b);
    RBS_DEBUG_STA(b =) Port_FreeMutex(ptDmmHead->hMutex);
    RBS_DEBUG_ASSERT(b);
  }
#endif
  return r;
}

/*
********************************************************************
*                      RBS_DMM_UnuseHMem
********************************************************************
*/

BOOL RBS_DMM_UnuseHMem(hMEM hMem) {
  BOOL r = TRUE;
  tMEM *ptMem = (tMEM*)hMem;
#if (RBS_CFG_APP_MODEL == RBS_APP_OS)
  tDMMHEAD *ptDmmHead;
#else
  RBS_DEBUG_STA(tDMMHEAD *ptDmmHead);
#endif
  RBS_DEBUG_ERROR(NULL == ptMem,"invalid handle.",return FALSE);
#if (RBS_CFG_APP_MODEL == RBS_APP_OS)
  ptDmmHead = (tDMMHEAD*)(ptMem->hDmm);
#else
  RBS_DEBUG_STA(ptDmmHead = (tDMMHEAD*)(ptMem->hDmm));
#endif
  RBS_DEBUG_ERROR(!DMM_VALID(ptDmmHead,DM_POOL,tMEM) && !DMM_VALID(ptDmmHead,DM_POOL_AUTO,tMEM),"invalid handle.",return FALSE);
#if (RBS_CFG_APP_MODEL == RBS_APP_OS)
  if (ptDmmHead->hMutex != NULL) {
    RBS_DEBUG_STA(r =) Port_GetMutex(ptDmmHead->hMutex);
    RBS_DEBUG_ASSERT(r);
  }
#endif
  RBS_DEBUG_ERROR(!MEM_VALID(ptMem) || !MEM_ALLOC(ptMem),"invalid handle.",r = FALSE);
  RBS_DEBUG_ERROR(r && (1 == ptMem->lock_count),"handle is not using.",r = FALSE);
  RBS_DEBUG_IF(r,;);
  ptMem->lock_count--;
  RBS_DEBUG_ENDIF(;);
#if (RBS_CFG_APP_MODEL == RBS_APP_OS)
  if (ptDmmHead->hMutex != NULL) {
    RBS_DEBUG_STA(r =) Port_FreeMutex(ptDmmHead->hMutex);
    RBS_DEBUG_ASSERT(r);
  }
#endif
  return r;
}

/*
********************************************************************
*                      RBS_DMM_GetHMemSize
********************************************************************
*/

USIZE RBS_DMM_GetHMemSize(hMEM hMem) {
  USIZE r = 1;
  tMEM *ptMem = (tMEM*)hMem;
#if (RBS_CFG_APP_MODEL == RBS_APP_OS)
  tDMMHEAD *ptDmmHead;
#else
  RBS_DEBUG_STA(tDMMHEAD *ptDmmHead);
#endif
  RBS_DEBUG_ERROR(NULL == ptMem,"invalid handle.",return 0);
#if (RBS_CFG_APP_MODEL == RBS_APP_OS)
  ptDmmHead = (tDMMHEAD*)(ptMem->hDmm);
#else
  RBS_DEBUG_STA(ptDmmHead = (tDMMHEAD*)(ptMem->hDmm));
#endif
  RBS_DEBUG_ERROR(!DMM_VALID(ptDmmHead,DM_POOL,tMEM) && !DMM_VALID(ptDmmHead,DM_POOL_AUTO,tMEM),"invalid handle.",return 0);
#if (RBS_CFG_APP_MODEL == RBS_APP_OS)
  if (ptDmmHead->hMutex != NULL) {
    RBS_DEBUG_STA(BOOL b);
    RBS_DEBUG_STA(b =) Port_GetMutex(ptDmmHead->hMutex);
    RBS_DEBUG_ASSERT(b);
  }
#endif
  RBS_DEBUG_ERROR(!MEM_VALID(ptMem) || !MEM_ALLOC(ptMem),"invalid handle.",r = 0);
  RBS_DEBUG_IF(0 != r,;);
  r = ptMem->size;
  RBS_DEBUG_ENDIF(;);
#if (RBS_CFG_APP_MODEL == RBS_APP_OS)
  if (ptDmmHead->hMutex != NULL) {
    RBS_DEBUG_STA(BOOL b);
    RBS_DEBUG_STA(b =) Port_FreeMutex(ptDmmHead->hMutex);
    RBS_DEBUG_ASSERT(b);
  }
#endif
  return r;
}

/*************************** End of file ****************************/
