/*
----------------------------------------------------------------------
                RainbowBS File : Convert.c
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

/*
********************************************************************
*                       RBS_Number2String
********************************************************************
*/

static const char gHex[16] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
static const U32 gPow10[10] = {1,10,100,1000,10000,100000,1000000,10000000,100000000,1000000000};

U8 RBS_Number2String(U32 value,ePOW ePow,U8 length,char *pText) {
  U8 bits = 0;
  RBS_DEBUG_ERROR(NULL == pText,"NULL para.",return 0);
  switch (ePow) {
    case SHEX:
         if (0 == length) {
           U8 i;
           for (i = 0,length = 8;length > 1;length--,i += 4) {
             if (0 != (value & (0xF0000000u>>i)))
               break;
           }
         } else if (length > 8u) {
           length = 8u;
         }
         RBS_DEBUG_ASSERT((length > 0) && (length < 9u));
         bits = length;
         while(length--) {
           pText[length] = gHex[value & 0xF];
           value >>= 4;
         }
         pText[bits] = 0;
         break;
    case SBINARY:
         if (0 == length) {
           U8 i;
           for (i = 0,length = 32;length > 1;length--,i++) {
             if (0 != (value & (0x80000000u>>i)))
               break;
           }
         } else if (length > 32u) {
           length = 32u;
         }
         RBS_DEBUG_ASSERT((length > 0) && (length < 33u));
         bits = length;
         while(length--) {
           pText[length] = (value & 1u) ? '1' : '0';
           value >>= 1;
         }
         pText[bits] = 0;
         break;
    case SDECIMAL:
         if (0 == length) {
           length = 1;
           while ((length <= 9) && (value >= gPow10[length]))
             length++;
         } else  if (length > 10u) {
           length = 10u;
         }
         RBS_DEBUG_ASSERT((length > 0) && (length < 11u));
         bits = length;
         {
           U8 i = 1;
           while (length--) {
             U8 c = (U8)(value % gPow10[i]);
             value -= c;
             pText[length] = '0' + (U8)(c / gPow10[i-1]);
             i++;
           }
         }
         pText[bits] = 0;
         break;
  }
  return bits;
}

/*
********************************************************************
*                        RBS_sin
********************************************************************
*/

static const U16 gSin[33] = {
  0,       /* sin(0)*1024 */
  50,      /* sin(1/32*90degrees)*1024 */
  100,     /* sin(2/32*90degrees)*1024 */
  150,     /* sin(3/32*90degrees)*1024 */
  200,     /* sin(4/32*90degrees)*1024 */
  249,     /* sin(5/32*90degrees)*1024 */
  297,     /* sin(6/32*90degrees)*1024 */
  345,     /* sin(7/32*90degrees)*1024 */
  392,     /* sin(8/32*90degrees)*1024 */
  438,     /* sin(9/32*90degrees)*1024 */
  483,     /* sin(10/32*90degrees)*1024 */
  526,     /* sin(11/32*90degrees)*1024 */
  569,     /* sin(12/32*90degrees)*1024 */
  610,     /* sin(13/32*90degrees)*1024 */
  650,     /* sin(14/32*90degrees)*1024 */
  688,     /* sin(15/32*90degrees)*1024 */
  724,     /* sin(16/32*90degrees)*1024 */
  759,     /* sin(17/32*90degrees)*1024 */
  792,     /* sin(18/32*90degrees)*1024 */
  822,     /* sin(19/32*90degrees)*1024 */
  851,     /* sin(20/32*90degrees)*1024 */
  878,     /* sin(21/32*90degrees)*1024 */
  903,     /* sin(22/32*90degrees)*1024 */
  926,     /* sin(23/32*90degrees)*1024 */
  946,     /* sin(24/32*90degrees)*1024 */
  964,     /* sin(25/32*90degrees)*1024 */
  980,     /* sin(26/32*90degrees)*1024 */
  993,     /* sin(27/32*90degrees)*1024 */
  1004,    /* sin(28/32*90degrees)*1024 */
  1013,    /* sin(29/32*90degrees)*1024 */
  1019,    /* sin(30/32*90degrees)*1024 */
  1023,    /* sin(31/32*90degrees)*1024 */
  1024     /* sin(90degrees)*1024 */
};

int RBS_sin(int angle) {
  BOOL bNegative = FALSE;
  int i;
  U32 r;
  U16 rest;
  if (angle < 0) {
    angle = -angle;
    bNegative = TRUE;
  }
  angle &= ANG_360DEG - 1;  /* reduce to [0,360) degrees */
  if (angle > ANG_180DEG) { /* reduce to [0,180] degrees */
    angle -= ANG_180DEG;
    bNegative = bNegative ? FALSE : TRUE;
  }
  if (angle > ANG_90DEG) /* reduce to [0,90] degrees */
    angle = ANG_180DEG - angle;
  /* Now angle is reduced to [0,90] degrees */
  i = angle>>(11 - 5);//ANG_90DEG = 2^11,sizeof(aSin) = 2^5 + 1
  rest = angle & ((1u<<(11 - 5)) - 1);
  if (0 != rest) {
    r = gSin[i] * ((1u<<(11 - 5)) - rest) + gSin[i + 1] * rest;
    r = (r + (1u<<5))>>(11 - 5);
  } else {
    r = gSin[i];
  }
  return bNegative ? 0 - r : r;
}

/*************************** End of file ****************************/
