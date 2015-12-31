/*
----------------------------------------------------------------------
                   RainbowBS File : Sysinfo.c
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

#include "../Port/Port.h"

/*
********************************************************************
*                  RBS_GetVersionString
********************************************************************
*/

#define RBS_VERSION_STR "RainbowBS V"RBS_VERSION" Build:"__DATE__" "__TIME__"\r\n"
const char* RBS_GetVersionString(void) {
  return RBS_VERSION_STR;
}

/*
********************************************************************
*                          RBS_Init
********************************************************************
*/

#if (RBS_CFG_APP_MODEL == RBS_APP_PTP)
  PROCESS_NAME(etimer_process);
  extern void process_init(void);
#endif

BOOL RBS_Init(void) {
  BOOL r = TRUE;
  if (!Port_Init())
    r = FALSE;
#if (RBS_CFG_APP_MODEL == RBS_APP_PTP)
  if (r) {
    process_init();
    if (!process_start(&etimer_process,NULL))
      r = FALSE;
  }
#endif
  if (r)
    RBS_DEBUG_LOG("RBS Initialization OK!");
  else
    RBS_DEBUG_LOG("RBS Initialization Failed!");
  return r;
}

/*
********************************************************************
*                       RBS_GetTickCount
********************************************************************
*/

TICK RBS_GetTickCount(void) {
  return Port_GetTickCount();
}

/*
********************************************************************
*                       RBS_GetRunTime
********************************************************************
*/

U64 RBS_GetRunTime(void) {
  return (U64)Port_GetTickCount() * RBS_TICK_MS;
}

/*
********************************************************************
*                       RBS_GetLocalTime
********************************************************************
*/

void RBS_GetLocalTime(tTime *ptTime) {
  RBS_DEBUG_ERROR(NULL == ptTime,"NULL para.",return);
  Port_GetLocalTime(ptTime);
}

/*
********************************************************************
*                        RBS_Delay
********************************************************************
*/

void RBS_Delay(U32 milliseconds) {
  Port_Delay(milliseconds);
}

/*************************** End of file ****************************/
