/*
----------------------------------------------------------------------
                  RainbowBS File : Port.c
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

#include "Port.h"

static TICK _TickCount;
#if (RBS_CFG_APP_MODEL == RBS_APP_OS) && (RBS_CFG_DEBUG_LEVEL >= RBS_DEBUG_LEVEL_ERRORS)
  HMUTEX Debug_Mutex;
  #ifndef WIN32
    osMutexDef(DEBUG_Mutex);
  #endif
#endif

/*
********************************************************************
*                         Port_Init
********************************************************************
*/

#ifdef WIN32
  void CALLBACK TimeProc(HWND hwnd,UINT message,UINT idTimer,DWORD dwTime) {
    _TickCount++;
    #if (RBS_CFG_APP_MODEL == RBS_APP_PTP)
      etimer_request_poll();
    #endif
  }
#else
  #if (RBS_CFG_APP_MODEL == RBS_APP_OS)
    void TickHandler(void const *arg) {
      USE_PARA(arg);
      _TickCount++;
    }
    osTimerDef(Tick,TickHandler);
    static osTimerId _TickId;
  #else
    #include "stm32f4xx.h"
    void SysTick_Handler(void) {
      _TickCount++;
      #if (RBS_CFG_APP_MODEL == RBS_APP_PTP)
        etimer_request_poll();
      #endif
    }
  #endif
#endif

BOOL Port_Init(void) {
  _TickCount = 0;
#ifdef WIN32
  SetTimer(NULL,1,RBS_TICK_MS,TimeProc);
  #if (RBS_CFG_APP_MODEL == RBS_APP_OS) && (RBS_CFG_DEBUG_LEVEL >= RBS_DEBUG_LEVEL_ERRORS)
    if (NULL == (Debug_Mutex = CreateMutex(NULL,FALSE,"DEBUG_Mutex")))
      return FALSE;
  #endif
#else
  #if (RBS_CFG_APP_MODEL == RBS_APP_OS)
    if (0 == osKernelRunning()) {
      if (osOK != osKernelInitialize())
        return FALSE;
    }
    _TickId = osTimerCreate(osTimer(Tick),osTimerPeriodic,NULL);
    if (osOK != osTimerStart(_TickId,RBS_TICK_MS))
      return FALSE;
    #if (RBS_CFG_DEBUG_LEVEL >= RBS_DEBUG_LEVEL_ERRORS)
      if (NULL == (Debug_Mutex = osMutexCreate(osMutex(DEBUG_Mutex))))
        return FALSE;
    #endif
  #else
    NVIC_SetPriorityGrouping(3);
    SystemCoreClockUpdate();//updata SystemCoreClock
    SysTick_Config(SystemCoreClock/RBS_CFG_TICK_RATE);//start SysTick interrupt
  #endif
#endif
  return TRUE;
}

/*
********************************************************************
*                       Port_GetTickCount
********************************************************************
*/

TICK Port_GetTickCount(void) {
  return _TickCount;
}

/*
********************************************************************
*                     Port_GetLocalTime
********************************************************************
*/

void Port_GetLocalTime(tTime *pTime) {
#ifdef WIN32
  SYSTEMTIME tm;
  GetLocalTime(&tm);
  pTime->year = tm.wYear;
  pTime->month = (U8)tm.wMonth;
  pTime->day = (U8)tm.wDay;
  pTime->day_of_week = (U8)tm.wDayOfWeek;
  pTime->hour = (U8)tm.wHour;
  pTime->minute = (U8)tm.wMinute;
  pTime->second = (U8)tm.wSecond;
  pTime->milliseconds = tm.wMilliseconds;
#else
  return;
#endif
}

/*
********************************************************************
*                        Port_Delay
********************************************************************
*/

void Port_Delay(U32 millisec) {
#ifdef WIN32
  Sleep(millisec);
#else
  #if (RBS_CFG_APP_MODEL == RBS_APP_OS)
    osDelay(millisec);
  #else
    TICK CurTick = _TickCount;
    while(_TickCount < CurTick + millisec/RBS_TICK_MS);
  #endif
#endif
}

/*
********************************************************************
*                     Port_Printf_Error
********************************************************************
*/

#if (RBS_CFG_DEBUG_LEVEL >= RBS_DEBUG_LEVEL_ERRORS)
void Port_Printf_Error(const char *s) {
  printf(s);
}
#endif

/*
********************************************************************
*                    Port_Printf_Warn
********************************************************************
*/

#if (RBS_CFG_DEBUG_LEVEL >= RBS_DEBUG_LEVEL_WARNINGS)
void Port_Printf_Warn(const char *s) {
  printf(s);
}
#endif

/*
********************************************************************
*                       Port_Printf_Log
********************************************************************
*/

#if (RBS_CFG_DEBUG_LEVEL >= RBS_DEBUG_LEVEL_LOG)
void Port_Printf_Log(const char *s) {
  printf(s);
}
#endif

/*
********************************************************************
*                       Port_GetMutex
********************************************************************
*/

#if (RBS_CFG_APP_MODEL == RBS_APP_OS)
BOOL Port_GetMutex(HMUTEX hMutex) {
#ifdef WIN32
  DWORD r;
  RBS_DEBUG_ASSERT(NULL != hMutex);
  r = WaitForSingleObject(hMutex,INFINITE);
  return (WAIT_OBJECT_0 == r) ? TRUE : FALSE;
#else
  osStatus status;
  RBS_DEBUG_ASSERT(NULL != hMutex);
  status = osMutexWait(hMutex,osWaitForever);
  return (osOK == status) ? TRUE : FALSE;
#endif
}
#endif

/*
********************************************************************
*                       Port_FreeMutex
********************************************************************
*/

#if (RBS_CFG_APP_MODEL == RBS_APP_OS)
BOOL Port_FreeMutex(HMUTEX hMutex) {
#ifdef WIN32
  BOOL r;
  RBS_DEBUG_ASSERT(NULL != hMutex);
  r = ReleaseMutex(hMutex);
  return r;
#else
  osStatus status;
  RBS_DEBUG_ASSERT(NULL != hMutex);
  status = osMutexRelease(hMutex);
  return (osOK == status) ? TRUE : FALSE;
#endif
}
#endif

/*************************** End of file ****************************/
