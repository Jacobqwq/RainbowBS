/*
----------------------------------------------------------------------
                RainbowBS File : MemOP.c
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
#define USE_CLIB 0
#if USE_CLIB
  #include <string.h>
#endif

/*
********************************************************************
*                      RBS_MemSet8
********************************************************************
*/

void RBS_MemSet8(U8 *pDes,U8 fill,USIZE num) {
#if USE_CLIB
  memset(pDes,fill,num);
#else
  U8 dif;
  USIZE num_ints;
  RBS_DEBUG_ERROR((NULL == pDes) || (0 == num),"invalid para.",return);
  /* Write bytes until reaching an int boundary */
  dif = (sizeof(unsigned) - 1) & (IPTRDIFF)pDes;
  if (0 != dif) {
    dif = sizeof(unsigned) - dif;
    dif = MIN(dif,num);
    num -= dif;
    while (0 != dif--)
      *pDes++ = fill;
  }
  /* Write integers */
  num_ints = num>>(sizeof(unsigned)>>1);
  if (0 != num_ints) {
    U8 i;
    unsigned fill_int = 0;
    unsigned *pInt = (unsigned*)pDes;
    num &= (sizeof(unsigned) - 1);
    for (i = 0;i < sizeof(unsigned);i++)
      fill_int |= (unsigned)fill<<(i<<3);
    while (num_ints >= 4) {
      *pInt++ = fill_int;
      *pInt++ = fill_int;
      *pInt++ = fill_int;
      *pInt++ = fill_int;
      num_ints -= 4;
    }
    while (0 != num_ints) {
      *pInt++ = fill_int;
      num_ints--;
    }
    pDes = (U8*)pInt;
  }
  /* Fill the remainder byte wise */
  while (0 != num) {
    *pDes++ = fill;
    num--;
  }
#endif
}

/*
********************************************************************
*                      RBS_MemSet16
********************************************************************
*/

void RBS_MemSet16(U16 *pDes,U16 fill,USIZE num) {
  RBS_DEBUG_ERROR((NULL == pDes) || (0 == num),"invalid para.",return);
  RBS_DEBUG_ERROR(((IPTRDIFF)pDes & 0x1) != 0,"not 2-bytes alignment.",return);
  if (sizeof(unsigned) <= 2) {
    while (num >= 4) {
      *pDes++ = fill;
      *pDes++ = fill;
      *pDes++ = fill;
      *pDes++ = fill;
      num -= 4;
    }
    while (0 != num) {
      *pDes++ = fill;
      num--;
    }
  } else {
    USIZE num_ints;
    /* Write bytes until reaching an int boundary */
    U8 dif = (sizeof(unsigned) - 1) & (IPTRDIFF)pDes;
  if (0 != dif) {
      dif >>= 1;
      dif = (sizeof(unsigned)>>1) - dif;
      dif = MIN(dif,num);
      num -= dif;
      while (0 != dif--)
        *pDes++ = fill;
    }
    /* Write integers */
    num_ints = num>>(sizeof(unsigned)>>2);
    if (0 != num_ints) {
      U8 i;
      unsigned fill_int = 0;
      unsigned *pInt = (unsigned*)pDes;
      num &= ((sizeof(unsigned)>>1) - 1);
      for (i = 0;i < (sizeof(unsigned)>>1);i++)
        fill_int |= (unsigned)fill<<(i<<4);
      while (num_ints >= 4) {
        *pInt++ = fill_int;
        *pInt++ = fill_int;
        *pInt++ = fill_int;
        *pInt++ = fill_int;
        num_ints -= 4;
      }
      while (0 != num_ints) {
        *pInt++ = fill_int;
        num_ints--;
      }
      pDes = (U16*)pInt;
    }
    /* Fill the remainder byte wise */
    while (0 != num) {
      *pDes++ = fill;
      num--;
    }
  }
}

/*
********************************************************************
*                      RBS_MemSet32
********************************************************************
*/

void RBS_MemSet32(U32 *pDes,U32 fill,USIZE num) {
  RBS_DEBUG_ERROR((NULL == pDes) || (0 == num),"invalid para.",return);
  RBS_DEBUG_ERROR(((IPTRDIFF)pDes & 0x3) != 0,"not 4-bytes alignment.",return);
  if (sizeof(unsigned) <= 4) {
    while (num >= 4) {
      *pDes++ = fill;
      *pDes++ = fill;
      *pDes++ = fill;
      *pDes++ = fill;
      num -= 4;
    }
    while (0 != num) {
      *pDes++ = fill;
      num--;
    }
  } else {
    USIZE num_ints;
    /* Write bytes until reaching an int boundary */
    U8 dif = (sizeof(unsigned) - 1) & (IPTRDIFF)pDes;
    if (0 != dif) {
      dif >>= 2;
      dif = (sizeof(unsigned)>>2) - dif;
      dif = MIN(dif,num);
      num -= dif;
      while (0 != dif--)
        *pDes++ = fill;
    }
    /* Write integers */
    num_ints = num>>(sizeof(unsigned)>>3);
    if (0 != num_ints) {
      U8 i;
      unsigned fill_int = 0;
      unsigned *pInt = (unsigned*)pDes;
      num &= ((sizeof(unsigned)>>2) - 1);
      for (i = 0;i < (sizeof(unsigned)>>2);i++)
        fill_int |= (unsigned)fill<<(i<<5);
      while (num_ints >= 4) {
        *pInt++ = fill_int;
        *pInt++ = fill_int;
        *pInt++ = fill_int;
        *pInt++ = fill_int;
        num_ints -= 4;
      }
      while (0 != num_ints) {
        *pInt++ = fill_int;
        num_ints--;
      }
      pDes = (U32*)pInt;
    }
    /* Fill the remainder byte wise */
    while (0 != num) {
      *pDes++ = fill;
      num--;
    }
  }
}

/*
********************************************************************
*                      RBS_MemCpy8
********************************************************************
*/

void RBS_MemCpy8(U8 *pDes,const U8 *pSrc,USIZE num) {
#if USE_CLIB
  memcpy(pDes,pSrc,num);
#else
  USIZE num_ints;
  U8 dif1,dif2;
  RBS_DEBUG_ERROR((NULL == pDes) || (NULL == pSrc) || (0 == num),"invalid para.",return);
  dif1 = (sizeof(unsigned) - 1) & (IPTRDIFF)pDes;
  dif2 = (sizeof(unsigned) - 1) & (IPTRDIFF)pSrc;
  if (dif1 != dif2) {
    while (num >= 4) {
      *pDes++ = *pSrc++;
      *pDes++ = *pSrc++;
      *pDes++ = *pSrc++;
      *pDes++ = *pSrc++;
      num -= 4;
    }
    while (0 != num) {
      *pDes++ = *pSrc++;
      num--;
    }
    return;
  } else if (0 != dif1) {
    /* Write bytes until reaching an int boundary */
    do {
      *pDes++ = *pSrc++;
      num--;
    } while ((0 != num) && (0 != ((sizeof(unsigned) - 1) & (IPTRDIFF)pDes)));
  }
  /* Write integers */
  num_ints = num>>(sizeof(unsigned)>>1);
  if (0 != num_ints) {
    num &= (sizeof(unsigned) - 1);
    while (num_ints >= 4) {
      *(unsigned*)pDes = *(unsigned*)pSrc;
      pDes += sizeof(unsigned);
      pSrc += sizeof(unsigned);
      *(unsigned*)pDes = *(unsigned*)pSrc;
      pDes += sizeof(unsigned);
      pSrc += sizeof(unsigned);
      *(unsigned*)pDes = *(unsigned*)pSrc;
      pDes += sizeof(unsigned);
      pSrc += sizeof(unsigned);
      *(unsigned*)pDes = *(unsigned*)pSrc;
      pDes += sizeof(unsigned);
      pSrc += sizeof(unsigned);
      num_ints -= 4;
    }
    while (0 != num_ints) {
      *(unsigned*)pDes = *(unsigned*)pSrc;
      pDes += sizeof(unsigned);
      pSrc += sizeof(unsigned);
      num_ints--;
    }
  }
  /* Fill the remainder byte wise */
  while (0 != num) {
    *pDes++ = *pSrc++;
    num--;
  }
#endif
}

/*
********************************************************************
*                     RBS_Read16L
********************************************************************
*/

U16 RBS_Read16L(const U8* *ppData) {
  U16 data = *(const U16*)(*ppData);
#if !RBS_CFG_CPU_BYTE_ORDER_L
  data = (data<<8) | (data>>8);
#endif
  *ppData += 2;
  return data;
}

/*
********************************************************************
*                    RBS_Read32L
********************************************************************
*/

U32 RBS_Read32L(const U8* *ppData) {
  U32 data = *(const U32*)(*ppData);
#if !RBS_CFG_CPU_BYTE_ORDER_L
  data = (data<<24) | ((data<<8) & 0xFF0000u) | ((data>>8) & 0xFF00u) | (data>>24);
#endif
  *ppData += 4;
  return data;
}

/*
********************************************************************
*                    RBS_Write16L
********************************************************************
*/

void RBS_Write16L(U8* *ppData,U16 data) {
#if RBS_CFG_CPU_BYTE_ORDER_L
  *(U16*)(*ppData) = data;
#else
  U8 *pData = *ppData;
  *pData++ = (U8)data;
  *pData = data>>8;
#endif
  *ppData += 2;
  return;
}

/*
********************************************************************
*                       RBS_Write32L
********************************************************************
*/

void RBS_Write32L(U8* *ppData,U32 data) {
#if RBS_CFG_CPU_BYTE_ORDER_L
  *(U32*)(*ppData) = data;
#else
  U8 *pData = *ppData;
  *pData++ = (U8)data;
  *pData++ = (U8)(data>>8);
  *pData++ = (U8)(data>>16);
  *pData = (U8)(data>>24);
#endif
  *ppData += 4;
  return;
}

/*
********************************************************************
*                      RBS_Read16B
********************************************************************
*/

U16 RBS_Read16B(const U8* *ppData) {
  U16 data = *(const U16*)(*ppData);
#if RBS_CFG_CPU_BYTE_ORDER_L
  data = (data<<8) | (data>>8);
#endif
  *ppData += 2;
  return data;
}

/*
********************************************************************
*                      RBS_Read32B
********************************************************************
*/

U32 RBS_Read32B(const U8* *ppData) {
  U32 data = *(const U32*)(*ppData);
#if RBS_CFG_CPU_BYTE_ORDER_L
  data = (data<<24) | ((data<<8) & 0xFF0000u) | ((data>>8) & 0xFF00u) | (data>>24);
#endif
  *ppData += 4;
  return data;
}

/*
********************************************************************
*                      RBS_Write16B
********************************************************************
*/

void RBS_Write16B(U8* *ppData,U16 data) {
#if RBS_CFG_CPU_BYTE_ORDER_L
  U8 *pData = *ppData;
  *pData++ = data>>8;
  *pData = (U8)data;
#else
  *(U16*)(*ppData) = data;
#endif
  *ppData += 2;
  return;
}

/*
********************************************************************
*                     RBS_Write32B
********************************************************************
*/

void RBS_Write32B(U8* *ppData,U32 data) {
#if RBS_CFG_CPU_BYTE_ORDER_L
  U8 *pData = *ppData;
  *pData++ = (U8)(data>>24);
  *pData++ = (U8)(data>>16);
  *pData++ = (U8)(data>>8);
  *pData = (U8)data;
#else
  *(U32*)(*ppData) = data;
#endif
  *ppData += 4;
  return;
}

/*************************** End of file ****************************/
