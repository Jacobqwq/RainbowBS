/*
----------------------------------------------------------------------
                  RainbowBS File : Debug.c
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

#if (RBS_CFG_DEBUG_LEVEL >= RBS_DEBUG_LEVEL_ERRORS)

char gDebugBuf[RBS_CFG_DEBUG_BUFSIZE+1] = {0};

void RBS_Debug_Error(const char *s) {
#if (RBS_CFG_APP_MODEL == RBS_APP_OS)
  extern HMUTEX Debug_Mutex;
  Port_GetMutex(Debug_Mutex);
#endif
  Port_Printf_Error(s);
#if (RBS_CFG_APP_MODEL == RBS_APP_OS)
  Port_FreeMutex(Debug_Mutex);
#endif
}

#if (RBS_CFG_DEBUG_LEVEL >= RBS_DEBUG_LEVEL_WARNINGS)
void RBS_Debug_Warn(const char *s) {
#if (RBS_CFG_APP_MODEL == RBS_APP_OS)
  extern HMUTEX Debug_Mutex;
  Port_GetMutex(Debug_Mutex);
#endif
  Port_Printf_Warn(s);
#if (RBS_CFG_APP_MODEL == RBS_APP_OS)
  Port_FreeMutex(Debug_Mutex);
#endif
}
#endif

#if (RBS_CFG_DEBUG_LEVEL >= RBS_DEBUG_LEVEL_LOG)
void RBS_Debug_Log(const char *s) {
#if (RBS_CFG_APP_MODEL == RBS_APP_OS)
  extern HMUTEX Debug_Mutex;
  Port_GetMutex(Debug_Mutex);
#endif
  Port_Printf_Log(s);
#if (RBS_CFG_APP_MODEL == RBS_APP_OS)
  Port_FreeMutex(Debug_Mutex);
#endif
}
#endif

#endif

/*************************** End of file ****************************/
