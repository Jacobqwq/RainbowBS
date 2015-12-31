/*
----------------------------------------------------------------------
                  RainbowBS File : Port.h
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
 * Implementation of porting.
 * \author QWQ <jacobqwq@icloud.com>
 */

#ifndef PORT_H
#define PORT_H

#ifdef _REL_PATH
  #include "../RainbowBS.h"
#else
  #include "RainbowBS.h"
#endif

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * \brief Initialize hardware.
 * \retval TRUE successful.
 * \retval FALSE failed.
 */
BOOL Port_Init(void);
/**
 * \brief Get system tick.
 * \return system tick.
 */
TICK Port_GetTickCount(void);
/**
 * \brief Get system local time.
 * \param[out] pTime time structure.
 */
void Port_GetLocalTime(tTime *pTime);
/**
 * \brief Delay some millisecond.
 * \param[in] millisec millisecond count.
 */
void Port_Delay(U32 millisec);
#if ((RBS_CFG_DEBUG_LEVEL >= RBS_DEBUG_LEVEL_ERRORS) || defined(USE_DOXYGEN))
/**
 * \brief Output error information.
 * \param[in] s string.
 */
  void Port_Printf_Error(const char *s);
#endif
#if ((RBS_CFG_DEBUG_LEVEL >= RBS_DEBUG_LEVEL_WARNINGS) || defined(USE_DOXYGEN))
/**
 * \brief Output warning information.
 * \param[in] s string.
 */
  void Port_Printf_Warn(const char *s);
#endif
#if ((RBS_CFG_DEBUG_LEVEL >= RBS_DEBUG_LEVEL_LOG) || defined(USE_DOXYGEN))
/**
 * \brief Output log information.
 * \param[in] s string.
 */
  void Port_Printf_Log(const char *s);
#endif

#if ((RBS_CFG_APP_MODEL == RBS_APP_OS) || defined(USE_DOXYGEN))
  /**
   * \brief Wait for a mutex.
   * \param[in] hMutex handle of mutex.
   */
  BOOL Port_GetMutex(HMUTEX hMutex);
  /**
   * \brief Release a mutex.
   * \param[in] hMutex handle of mutex.
   */
  BOOL Port_FreeMutex(HMUTEX hMutex);
#endif

#if defined(__cplusplus)
}
#endif

#endif

/*************************** End of file ****************************/
