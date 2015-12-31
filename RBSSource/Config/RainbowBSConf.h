/*
----------------------------------------------------------------------
                RainbowBS File : RainbowBSConf.h
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

/**
 * \file
 * RainbowBS Configuration.
 * \author QWQ <jacobqwq@icloud.com>
 */

/**
 * \defgroup RBS_config RBS configration
 * @{
 */

#ifndef RAINBOWBSCONF_H
#define RAINBOWBSCONF_H

#if defined(__cplusplus)
extern "C" {
#endif

/*
********************************************************************
*                      Sys/CPU Info.
********************************************************************
*/

#define RBS_CFG_SYS_INFO  "RainbowSys"
#define RBS_CFG_TICK_RATE  100

#define RBS_CFG_CPU_WORD_SIZE    32 /* CPU word size */
#define RBS_CFG_CPU_BYTE_ORDER_L 1  /* CPU byte order */
#define RBS_CFG_CPU_STACK_DOWN   1  /* stack growth direction */

/*
********************************************************************
*                    Application model
********************************************************************
*/

#define RBS_APP_NONE 0
#define RBS_APP_PTP  1
#define RBS_APP_OS   2

#ifndef RBS_CFG_APP_MODEL
  #define RBS_CFG_APP_MODEL  RBS_APP_PTP
#endif

#if (RBS_CFG_APP_MODEL == RBS_APP_OS)
  #ifdef WIN32
    #include <windows.h>
    typedef HANDLE HMUTEX;
  #else
    #include "cmsis_os.h"
    typedef osMutexId HMUTEX;
  #endif
#elif (RBS_CFG_APP_MODEL == RBS_APP_PTP)
  #define RBS_CFG_PTP_NO_PROCESS_NAME 0
  #define RBS_CFG_PTP_PROCESS_STATS 0
  #define RBS_CFG_PTP_NUMEVENTS 32
#endif

/*
********************************************************************
*                   Dynamic memory management
********************************************************************
*/

/* 3 means 8 bytes,2 means 4 bytes,1 means 2 bytes,0 means 1 byte */
#define RBS_CFG_DMM_ALIGN 2

/*
********************************************************************
*                          Debug
********************************************************************
*/

#define RBS_DEBUG_LEVEL_NOCHECK   0  /* No running time checks are performed */
#define RBS_DEBUG_LEVEL_ERRORS    1  /* Errors are recorded */
#define RBS_DEBUG_LEVEL_WARNINGS  2  /* Errors and Warnings are recorded */
#define RBS_DEBUG_LEVEL_LOG       3  /* Errors,Warnings and logs are recorded */

#define RBS_CFG_DEBUG_BUFSIZE 300
#define RBS_CFG_DEBUG_LEVEL RBS_DEBUG_LEVEL_LOG

#if defined(__cplusplus)
}
#endif

#endif

/** @} */

/*************************** End of file ****************************/
