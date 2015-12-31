/*
----------------------------------------------------------------------
                   RainbowBS File : RainbowBS.h
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
 * RainbowBS Interface.
 * \author QWQ <jacobqwq@icloud.com>
 */

#ifndef RAINBOWBS_H
#define RAINBOWBS_H

#include <stddef.h> //ptrdiff_t,size_t,NULL,offsetof() are needed
#ifdef WIN32
  #include <windows.h> //BOOL needed
  #pragma warning(disable:4996)
#else
  #include <stdint.h> //bits-limited integer types are needed
#endif

#ifdef _REL_PATH
  #include "./Config/RainbowBSConf.h"
#else
  #include "RainbowBSConf.h"
#endif

#if defined(__cplusplus)
extern "C" {
#endif

#define RBS_VERSION         "0.1.0"
#define RBS_VERNUM          0x0100
#define RBS_VER_MAJOR       0
#define RBS_VER_MINOR       1
#define RBS_VER_REVISION    0
#define RBS_VER_SUBREVISION 0

/*
********************************************************************
*                    Configuration Check
********************************************************************
*/

#if ((RBS_CFG_TICK_RATE < 1) || (RBS_CFG_TICK_RATE > 1000) || (1000u%RBS_CFG_TICK_RATE != 0))
  #error Wrong RBS_CFG_TICK_RATE in RainbowBSConf,h!
#endif

#if ((RBS_CFG_APP_MODEL != RBS_APP_PTP) && (RBS_CFG_APP_MODEL != RBS_APP_NONE) && (RBS_CFG_APP_MODEL != RBS_APP_OS))
  #error Wrong RBS_CFG_APP_MODEL in RainbowBSConf,h!
#endif

#if ((RBS_CFG_DMM_ALIGN != 0) && (RBS_CFG_DMM_ALIGN != 1) && (RBS_CFG_DMM_ALIGN != 2) && (RBS_CFG_DMM_ALIGN != 3))
  #error Wrong RBS_CFG_DMM_ALIGN in RainbowBSConf,h!
#endif

#if ((RBS_CFG_DEBUG_LEVEL != RBS_DEBUG_LEVEL_NOCHECK) && (RBS_CFG_DEBUG_LEVEL != RBS_DEBUG_LEVEL_ERRORS) \
     && (RBS_CFG_DEBUG_LEVEL != RBS_DEBUG_LEVEL_WARNINGS) && (RBS_CFG_DEBUG_LEVEL != RBS_DEBUG_LEVEL_LOG))
  #error Wrong RBS_CFG_DEBUG_LEVEL in RainbowBSConf,h!
#elif (RBS_CFG_DEBUG_LEVEL != RBS_DEBUG_LEVEL_NOCHECK)
  #if (RBS_CFG_DEBUG_BUFSIZE < 200)
    #error Wrong RBS_CFG_DEBUG_BUFSIZE in RainbowBSConf,h!
  #endif
#endif

/*
********************************************************************
*                      Basic Types
********************************************************************
*/

/**
 * \defgroup type basic types
 * basic types defination.
 * @{
 */
#if (defined(WIN32) && !defined(USE_DOXYGEN))
  typedef signed char       I8;
  typedef unsigned char     U8;
  typedef signed short      I16;
  typedef unsigned short    U16;
  typedef signed long       I32;
  typedef unsigned long     U32;
  typedef signed __int64    I64;
  typedef unsigned __int64  U64;
#else
  typedef uint8_t     BOOL;
  typedef int8_t      I8;
  typedef uint8_t     U8;
  typedef int16_t     I16;
  typedef uint16_t    U16;
  typedef int32_t     I32;
  typedef uint32_t    U32;
  typedef int64_t     I64;
  typedef uint64_t    U64;
#endif
typedef size_t USIZE;
typedef ptrdiff_t IPTRDIFF;
typedef U64 TICK;
/** @} */

/*
********************************************************************
*                        Basic Macros
********************************************************************
*/

/**
 * \defgroup macros basic macros
 * basic macros defination.
 * @{
 */
#ifndef FALSE
  #define FALSE  0u
#endif

#ifndef TRUE
  #define TRUE   1u
#endif

#define USE_PARA(para) (para = (para))
#define MIN(v0,v1)     (((v0) > (v1)) ? (v1) : (v0))
#define MAX(v0,v1)     (((v0) > (v1)) ? (v0) : (v1))
#define ABS(v)         (((v) >= 0) ? (v) : (-(v)))
#define SWAP(a,b)      (a = (a) + (b),b = (a) - (b),a = (a) - (b))
#define COUNT_OF(a)    (sizeof(a)/sizeof(a[0]))
#define _STR(a)    #a
#define STR(a)     _STR(a)
#define _CONS(a,b) a##b
#define CONS(a,b)  _CONS(a,b)
#define ALIGN_F(pointer,power2) (((IPTRDIFF)(pointer) + ((IPTRDIFF)((power2) - 1))) & (~((IPTRDIFF)((power2) - 1))))
#define ALIGN_B(pointer,power2) ((IPTRDIFF)(pointer) & (~((IPTRDIFF)((power2) - 1))))
/** @} */

/*
********************************************************************
*                           System
********************************************************************
*/

/**
 * \defgroup sys system releted
 * Implementation of system releted.
 * @{
 */
/**
 * \brief Initialize RainbowBS.
 * \retval TRUE successfully.
 * \retval FALSE failed.
 */
BOOL RBS_Init(void);
/**
 * \name system information
 * @{
 */
/**
 * \brief Get RainbowBS version string.
 * \return version string.
 * \sa RBS_GetSysInfo()
 */
const char* RBS_GetVersionString(void);
/**
 * \brief Get system description string.
 * \return system description string.
 * \sa RBS_GetVersionString()
 */
#ifdef USE_DOXYGEN
  const char* RBS_GetSysInfo(void);
#else
  #define RBS_GetSysInfo() RBS_CFG_SYS_INFO
#endif
/** @} */
/**
 * \name time releated
 * @{
 */
#define RBS_TICK_MS    (1000u/RBS_CFG_TICK_RATE)
/**
 * \brief Get system tick.
 * \return system tick.
 */
TICK RBS_GetTickCount(void);
/**
 * \brief Get system running time(ms).
 * \return system running time in millisecond.
 * \sa RBS_GetLocalTime()
 */
U64 RBS_GetRunTime(void);
/**
 * \brief Time structure.
 */
typedef struct {
  U16 year;/**< year[1601,30827] */
  U8 month;/**< month[1,12] */
  U8 day;/**< day of month[1,31] */
  U8 day_of_week;/**< day of week[0,6] */
  U8 hour;/**< hour[0,23] */
  U8 minute;/**< minute[0,59] */
  U8 second;/**< second[0,59] */
  U16 milliseconds;/**< milliseconds[0,999] */
} tTime;
/**
 * \brief Get system local time.
 * \param[out] ptTime local time pointer.
 * \sa RBS_GetRunTime()
 */
void RBS_GetLocalTime(tTime *ptTime);
/**
 * \brief Delay some milliseconds.
 * \param[in] milliseconds millisecond count .
 */
void RBS_Delay(U32 milliseconds);
/** @} */
/** @} */

/*
********************************************************************
*                         CPU Info.
********************************************************************
*/

/**
 * \defgroup cpuinfo CPU information
 * Information about CPU.
 * @{
 */
/**
 * \brief CPU word-width.
 * \return CPU word-width(8/16/32/64).
 */
#ifdef USE_DOXYGEN
  U8 RBS_GetCPUBits(void);
#else
  #define RBS_GetCPUBits() RBS_CFG_CPU_WORD_SIZE
#endif
/**
 * \brief Check if CPU is little-endian.
 * \retval TRUE little-endian.
 * \retval FALSE big-endian.
 */
#ifdef USE_DOXYGEN
  BOOL RBS_IsCPULittleEndian(void);
#else
  #define RBS_IsCPULittleEndian() RBS_CFG_CPU_BYTE_ORDER_L
#endif
/**
 * \brief Check if stack grows down.
 * \retval TRUE growth down.
 * \retval FALSE growth up.
 */
#ifdef USE_DOXYGEN
  BOOL RBS_IsStackGrowDown(void);
#else
  #define RBS_IsStackGrowDown() RBS_CFG_CPU_STACK_DOWN
#endif
/** @} */

/*
********************************************************************
*                       Memory functions
********************************************************************
*/

/**
 * \defgroup memop memory operation
 * Memory set and copy operation.
 * @{
 */
/**
 * \brief Set memory by byte unit.
 * \param[out] pDes destination pointer.
 * \param[in] fill byte filled.
 * \param[in] num total units.
 */
void RBS_MemSet8(U8 *pDes,U8 fill,USIZE num);
/**
  * \brief Set memory by two-bytes unit.
  * \param[out] pDes destination pointer(two-bytes alignment).
  * \param[in] fill two-bytes filled.
  * \param[in] num total units.
  */
void RBS_MemSet16(U16 *pDes,U16 fill,USIZE num);
/**
  * \brief Set memory by four-bytes unit.
  * \param[out] pDes destination pointer(four-bytes alignment).
  * \param[in] fill four-bytes filled.
  * \param[in] num total units.
  */
void RBS_MemSet32(U32 *pDes,U32 fill,USIZE num);
/**
  * \brief Copy memory by byte unit.
  * \param[out] pDes destination pointer.
  * \param[in] pSrc source pointer.
  * \param[in] num total units.
  */
void RBS_MemCpy8(U8 *pDes,const U8 *pSrc,USIZE num);
/**
 * \brief Read a 16-bits entity in little-endian.
 * \param[inout] ppData data pointer,added 2 bytes after calling.
 * \return data entity
 */
U16 RBS_Read16L(const U8* *ppData);
/**
 * \brief Read a 32-bits entity in little-endian.
 * \param[inout] ppData data pointer,added 4 bytes after calling.
 * \return data entity
 */
U32 RBS_Read32L(const U8* *ppData);
/**
 * \brief Write a 16-bits entity in little-endian.
 * \param[inout] ppData data pointer,added 2 bytes after calling.
 * \param[in] data data to be wtitten.
 */
void RBS_Write16L(U8* *ppData,U16 data);
/**
 * \brief Write a 32-bits entity in little-endian.
 * \param[inout] ppData data pointer,added 4 bytes after calling.
 * \param[in] data data to be wtitten.
 */
void RBS_Write32L(U8* *ppData,U32 data);
/**
 * \brief Read a 16-bits entity in big-endian.
 * \param[inout] ppData data pointer,added 2 bytes after calling.
 * \return data entity
 */
U16 RBS_Read16B(const U8* *ppData);
/**
 * \brief Read a 32-bits entity in big-endian.
 * \param[inout] ppData data pointer,added 4 bytes after calling.
 * \return data entity
 */
U32 RBS_Read32B(const U8* *ppData);
/**
 * \brief Write a 16-bits entity in big-endian.
 * \param[inout] ppData data pointer,added 2 bytes after calling.
 * \param[in] data data to be wtitten.
 */
void RBS_Write16B(U8* *ppData,U16 data);
/**
 * \brief Write a 32-bits entity in big-endian.
 * \param[inout] ppData data pointer,added 4 bytes after calling.
 * \param[in] data data to be wtitten.
 */
void RBS_Write32B(U8* *ppData,U32 data);
/** @} */

/*
********************************************************************
*                    Conversion functions
********************************************************************
*/

/**
 * \defgroup conversion conversion releted
 * @{
 */
/**
 * \name convert digit to string
 * @{
 */
/**
 * \brief Specific hex,binary or decimal
 */
typedef enum {
  SHEX,/**< hex */
  SBINARY,/**< binary */
  SDECIMAL/**< decimal */
} ePOW;
/**
 * \brief Convert unsigned digit to ASCII string.
 * \param[in] value unsigned digit.
 * \param[in] ePow choose hex,binary or decimal.
 * \param[in] length Max.bits from lowest bit.If Len is 0,choosing Min.characters automatically.
 * \param[out] pText output string buffer.
 * \return character number.
 * \sa ePOW
 */
U8 RBS_Number2String(U32 value,ePOW ePow,U8 length,char *pText);
/** @} */

/**
 * \name trigonometric releted
 * @{
 */
#define ANG_45DEG  1024
#define ANG_90DEG  (2*ANG_45DEG)
#define ANG_135DEG (3*ANG_45DEG)
#define ANG_180DEG (4*ANG_45DEG)
#define ANG_225DEG (5*ANG_45DEG)
#define ANG_270DEG (6*ANG_45DEG)
#define ANG_315DEG (7*ANG_45DEG)
#define ANG_360DEG (8*ANG_45DEG)
/**
 * \brief Calculate sine.
 *
 * This function calculates sine without using float-point numbers.
 * It use a constant table to look up for the approximate value.
 *
 * \param[in] angle angle=degrees*ANG_45DEG/45,degrees=angle*45/ANG_45DEG.
 * \return sin(degrees)*1024.
 *
 * Example:
 * \code
 * //calculate sine of 30 degrees,the return value is 511 which
 * //is approximately equal to sin30*1024 which is 512.
 * int value = RBS_sin(30*ANG_45DEG/45);
 * \endcode
 */
int RBS_sin(int angle);
#ifdef USE_DOXYGEN
/**
 * \brief Calculate cosine.
 *
 * This function calculates cosine without using float-point numbers.It use a constant
 * table to look up for the appropriate value.
 *
 * \param[in] angle angle=degrees*ANG_45DEG/45,degrees=angle*45/ANG_45DEG.
 * \return cos(degrees)*1024.
 */
  int RBS_cos(int angle);
/**
 * \brief Calculate tangent.
 *
 * This function calculates tangent without using float-point numbers.It use a constant
 * table to look up for the appropriate value.
 *
 * \param[in] angle angle=degrees*ANG_45DEG/45,degrees=angle*45/ANG_45DEG.
 * \return tan(degrees)*1024.
 */
  int RBS_tan(int angle);
/**
 * \brief Calculate cotangent.
 *
 * This function calculates cotangent without using float-point numbers.It use a constant
 * table to look up for the appropriate value.
 *
 * \param[in] angle angle=degrees*ANG_45DEG/45,degrees=angle*45/ANG_45DEG.
 * \return cotan(degrees)*1024.
 */
  int RBS_ctan(int angle);
#else
  #define RBS_cos(angle)  RBS_sin(angle + ANG_90DEG)
  #define RBS_tan(angle)  (RBS_sin(angle)/RBS_cos(angle))
  #define RBS_ctan(angle) (RBS_cos(angle)/RBS_sin(angle))
#endif
/** @} */
/** @} */

/*
********************************************************************
*                  Dynamic memory management
********************************************************************
*/

/**
 * \defgroup dmm dynamic memory management
 * Dynamic memory management for both memory poll and block.
 * @{
 */
#define HDMM_NULL NULL
typedef void *hDMM;

#define HBLOCK_NULL NULL
typedef void *hBLOCK;

#define HMEM_NULL NULL
typedef void *hMEM;
/**
 * \brief Register the memory area as a dynamic block memory area.
 * \param[in] pName block memory name.
 * \param[in] pDM pointer to the block memory area.
 * \param[in] size block memory area size(bytes).
 * \param[in] block_size each block size.
 * \param[in] hMutex memory block mutex for thread-safe.If NULL,no thread-safe support.
 * \retval HDMM_NULL failed.
 * \retval other handle of dynamic block memory area.
 */
#if (RBS_CFG_APP_MODEL == RBS_APP_OS)
  hDMM RBS_DMM_RegisterBlock(char *pName,void *pDM,USIZE size,USIZE block_size,HMUTEX hMutex);
#else
  hDMM RBS_DMM_RegisterBlock(char *pName,void *pDM,USIZE size,USIZE block_size);
#endif
/**
 * \brief Allocate a free block with zero initialization from the dynamic block memory area.
 * \param[in] hDmm handle of dynamic block memory area.
 * \retval HBLOCK_NULL failed.
 * \retval others handle of dynamic memory area.
 */
hBLOCK RBS_DMM_AllocZeroBlock(hDMM hDmm);
/**
 * \brief Allocate a free block from the dynamic block memory area.
 * \param[in] hDmm handle of dynamic block memory area.
 * \retval HBLOCK_NULL failed.
 * \retval others handle of dynamic memory area.
 */
hBLOCK RBS_DMM_AllocBlock(hDMM hDmm);
/**
 * \brief Free the allocated block.
 * \param[in] hBlock handle of a allocated block.
 * \retval TRUE successful.
 * \retval FALSE failed.
 */
BOOL RBS_DMM_FreeBlock(hBLOCK hBlock);
/**
 * \brief Get the allocated block pointer.
 * \param[in] hBlock handle of a allocated block.
 * \return block area pointer.
 * \sa RBS_DMM_UnuseHBlock()
 * \note used in pair with RBS_DMM_UnuseHBlock()
 */
void* RBS_DMM_UseHBlock(hBLOCK hBlock);
/**
 * \brief Unuse the allocated block area.
 * \param[in] hBlock handle of a allocated block.
 * \return block area pointer.
 * \sa RBS_DMM_UseHBlock()
 * \note used in pair with RBS_DMM_UseHBlock()
 */
BOOL RBS_DMM_UnuseHBlock(hBLOCK hBlock);
/**
 * \brief Register the memory area as a dynamic pool memory area.
 * \param[in] pName memory pool name.
 * \param[in] pDM pointer to the memory area.
 * \param[in] size memory area size(bytes).
 * \param[in] bAntiFrag enable defragmentation.
 * \param[in] handle_count count of memory handles.
 * \param[in] hMutex memory pool mutex for thread-safety.If NULL,no thread-safe support.
 * \retval Not HDMM_NULL successful.
 * \retval HDMM_NULL failed.
 */
#if (RBS_CFG_APP_MODEL == RBS_APP_OS)
  hDMM RBS_DMM_RegisterPool(char *pName,void *pDM,USIZE size,BOOL bAntiFrag,U16 handle_count,HMUTEX hMutex);
#else
  hDMM RBS_DMM_RegisterPool(char *pName,void *pDM,USIZE size,BOOL bAntiFrag,U16 handle_count);
#endif
/**
 * \brief Allocate a free memory with zero initialization from the dynamic memory area.
 * \param[in] hDmm handle of dynamic memory area.
 * \param[in] size memory area bytes.
 * \retval HMEM_NULL failed.
 * \retval others handle of dynamic memory area.
 */
hMEM RBS_DMM_AllocZeroMem(hDMM hDmm,USIZE size);
/**
 * \brief Allocate a free memory from the dynamic memory area.
 * \param[in] hDmm handle of dynamic memory area.
 * \param[in] size memory area bytes.
 * \retval HMEM_NULL failed.
 * \retval others handle of dynamic memory area.
 */
hMEM RBS_DMM_AllocMem(hDMM hDmm,USIZE size);
/**
 * \brief Reallocate a free memory from the same dynamic memory area.
 * \param[in] hMem handle of an allocated memory.
 * \param[in] size memory area bytes.
 * \retval HMEM_NULL failed.
 * \retval others handle of dynamic memory area.
 */
hMEM RBS_DMM_ReallocMem(hMEM hMem,USIZE size);
/**
 * \brief Free the allocated memory.
 * \param[in] hMem handle of a allocated memory.
 * \retval TRUE successful.
 * \retval FALSE failed.
 */
BOOL RBS_DMM_FreeMem(hMEM hMem);
/**
 * \brief Get the allocated memory pointer.
 * \param[in] hMem handle of a allocated memory.
 * \return memory area pointer.
 * \sa RBS_DMM_UnuseHMem()
 * \note used in pair with RBS_DMM_UnuseHMem()
 */
void* RBS_DMM_UseHMem(hMEM hMem);
/**
 * \brief Unuse the allocated memory area.
 * \param[in] hMem handle of a allocated memory.
 * \return memory area pointer.
 * \sa RBS_DMM_UseHMem()
 * \note used in pair with RBS_DMM_UseHMem()
 */
BOOL RBS_DMM_UnuseHMem(hMEM hMem);
/**
 * \brief Get the allocated memory size.
 * \param[in] hMem handle of the allocated memory.
 * \return memory area size.
 */
USIZE RBS_DMM_GetHMemSize(hMEM hMem);
/** @} */

/*
********************************************************************
*                   Protothread process
********************************************************************
*/

#if ((RBS_CFG_APP_MODEL == RBS_APP_PTP) || defined(USE_DOXYGEN))
#ifdef _REL_PATH
  #include "Base/Protothreads/pt.h"
  #include "Base/Protothreads/pt-sem.h"
#else
  #include "pt.h"
  #include "pt-sem.h"
#endif
/**
 * \defgroup ptprocess protothread process
 * A process in consists of a single \ref pt "protothread".
 *
 * @{
 */
typedef U8 process_event_t;
typedef void *process_data_t;

#define PROCESS_NONE      NULL
#define PROCESS_BROADCAST NULL

/**
 * \name predefined event type
 * @{
 */
#define PROCESS_EVENT_NONE            0x80
#define PROCESS_EVENT_INIT            0x81
#define PROCESS_EVENT_POLL            0x82
#define PROCESS_EVENT_EXIT            0x83
#define PROCESS_EVENT_SERVICE_REMOVED 0x84
#define PROCESS_EVENT_CONTINUE        0x85
#define PROCESS_EVENT_MSG             0x86
#define PROCESS_EVENT_EXITED          0x87
#define PROCESS_EVENT_TIMER           0x88
#define PROCESS_EVENT_COM             0x89
#define PROCESS_EVENT_MAX             0x8a
/** @} */

/**
 * \brief Process structure(one process consists one protothread).
 * \note application code should not change any member of the process structure
 *       for they are maintained by system inside.
 */
typedef struct process_struct {
  struct process_struct *next;/**< pointer to the next process,all processes are in a process list */
#if RBS_CFG_PTP_NO_PROCESS_NAME
  #define PROCESS_NAME_STRING(process) ""
#else
  const char *name;/**< process name string */
  #define PROCESS_NAME_STRING(process) ((NULL == (process)) ? "No Name" : (process)->name)
#endif
  PT_THREAD((*thread)(struct pt *,process_event_t,process_data_t));
  struct pt pt;
  U8 state;/**< indicate process state */
  BOOL needspoll;/**< indicate if a process has high privilege to be called,\sa process_poll() */
} tPROCESS;

/**
 * \name process declaration and definition
 * @{
 */
/**
 * \brief Define the body of a process.
 *
 * This macro is used to define the body (protothread) of a
 * process. The process is called whenever an event occurs in the
 * system, A process always start with the PROCESS_BEGIN() macro and
 * end with the PROCESS_END() macro.
 *
 * \hideinitializer
 */
#define PROCESS_THREAD(name,ev,data) \
        static PT_THREAD(process_thread_##name(struct pt *process_pt, \
        process_event_t ev,process_data_t data))
/**
 * \brief Declare the name of a process.
 *
 * This macro is typically used in header files to declare the name of
 * a process that is implemented in the C file.
 */
#define PROCESS_NAME(name) extern tPROCESS name
/**
 * \brief Define a process.
 *
 * This macro defines a process. The process has two names: the
 * variable of the process structure, which is used by the C program,
 * and a human readable string name, which is used when debugging.
 * A configuration option allows removal of the readable name to save RAM.
 *
 * \param[in] name The variable name of the process structure.
 * \param[in] strname The string representation of the process' name.
 * \note definition must be global.
 * \hideinitializer
 */
#if RBS_CFG_PTP_NO_PROCESS_NAME
  #define PROCESS(name,strname) PROCESS_THREAD(name,ev,data); \
                                tPROCESS name = { NULL,process_thread_##name }
#else
  #define PROCESS(name,strname) PROCESS_THREAD(name,ev,data); \
                                tPROCESS name = { NULL,strname,process_thread_##name }
#endif
/** @} */

/**
 * \name semaphore declaration and definition
 * @{
 */
/**
 * \brief Declare the name of a semaphore.
 *
 * This macro is typically used in header files to declare the name of
 * a semaphore that is implemented in the C file.
 */
#define SEM_NAME(name) extern struct pt_sem sem_##name
/**
 * \brief Define a semaphore.
 * \param name The variable name of the semaphore structure.
 * \param count The count value of the semaphore.
 * \note definition must be global.
 * \hideinitializer
 */
#define SEM(name,count) struct pt_sem sem_##name = {count}
/** @} */

/**
 * \name process protothread functions
 * @{
 */
/**
 * \brief Define the beginning of a process.
 *
 * This macro defines the beginning of a process, and must always
 * appear in a PROCESS_THREAD() definition. The PROCESS_END() macro
 * must come at the end of the process.
 *
 * \hideinitializer
 */
#define PROCESS_BEGIN()             PT_BEGIN(process_pt)
/**
 * \brief Define the end of a process.
 *
 * This macro defines the end of a process. It must appear in a
 * PROCESS_THREAD() definition and must always be included. The
 * process exits when the PROCESS_END() macro is reached.
 *
 * \hideinitializer
 */
#define PROCESS_END()               PT_END(process_pt)
/**
 * \brief Wait for an event to be posted to the process.
 *
 * This macro blocks the currently running process until the process
 * receives an event.
 *
 * \hideinitializer
 */
#define PROCESS_WAIT_EVENT()        PROCESS_YIELD()
/**
 * \brief Wait for an event to be posted to the process, with an extra
 * condition.
 *
 * This macro is similar to PROCESS_WAIT_EVENT() in that it blocks the
 * currently running process until the process receives an event. But
 * PROCESS_WAIT_EVENT_UNTIL() takes an extra condition which must be
 * true for the process to continue.
 *
 * \param[in] c The condition that must be true for the process to continue.
 * \sa PT_WAIT_UNTIL()
 * \hideinitializer
 */
#define PROCESS_WAIT_EVENT_UNTIL(c) PROCESS_YIELD_UNTIL(c)
/**
 * \brief Yield the currently running process.
 * \hideinitializer
 */
#define PROCESS_YIELD()             PT_YIELD(process_pt)
/**
 * \brief Yield the currently running process until a condition occurs.
 *
 * This macro is different from PROCESS_WAIT_UNTIL() in that
 * PROCESS_YIELD_UNTIL() is guaranteed to always yield at least
 * once. This ensures that the process does not end up in an infinite
 * loop and monopolizing the CPU.
 *
 * \param[in] c The condition to wait for.
 * \hideinitializer
 */
#define PROCESS_YIELD_UNTIL(c)      PT_YIELD_UNTIL(process_pt,c)
/**
 * \brief Wait for a condition to occur.
 *
 * This macro does not guarantee that the process yields, and should
 * therefore be used with care. In most cases, PROCESS_WAIT_EVENT(),
 * PROCESS_WAIT_EVENT_UNTIL(), PROCESS_YIELD() or
 * PROCESS_YIELD_UNTIL() should be used instead.
 *
 * \param[in] c The condition that must be true for the process to continue
 * \hideinitializer
 */
#define PROCESS_WAIT_UNTIL(c)       PT_WAIT_UNTIL(process_pt,c)
/**
 * \brief Wait for a condition not occur.
 *
 * This macro does not guarantee that the process yields, and should
 * therefore be used with care. In most cases, PROCESS_WAIT_EVENT(),
 * PROCESS_WAIT_EVENT_UNTIL(), PROCESS_YIELD() or
 * PROCESS_YIELD_UNTIL() should be used instead.
 *
 * \param[in] c The condition that must be false for the process to continue.
 * \hideinitializer
 */
#define PROCESS_WAIT_WHILE(c)       PT_WAIT_WHILE(process_pt,c)
/**
 * \brief Exit the currently running process.
 * \hideinitializer
 */
#define PROCESS_EXIT()              PT_EXIT(process_pt)
/**
 * \brief Spawn a protothread from the process.
 * \param pt[inout] The protothread state (struct pt) for the new protothread
 * \param thread[in] The call to the protothread function.
 * \sa PT_SPAWN()
 * \hideinitializer
 */
#define PROCESS_PT_SPAWN(pt,thread)   PT_SPAWN(process_pt,pt,thread)
/**
 * \brief Yield the process for a short while.
 *
 * This macro yields the currently running process for a short while,
 * thus letting other processes run before the process continues.
 *
 * \hideinitializer
 */
#define PROCESS_PAUSE()  do { \
                          process_post(PROCESS_CURRENT(), PROCESS_EVENT_CONTINUE, NULL); \
                          PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_CONTINUE); \
                         } while(0)
/**
 * Wait for a semaphore
 *
 * This macro carries out the "wait" operation on the semaphore.
 *
 * \param[in] name semaphore name
 * \note It's the application's responsibility to notify the waiting process if semaphore is avilable.
 * \sa PROCESS_SIGNAL_SEM()
 * \hideinitializer
 */
#define PROCESS_WAIT_SEM(name)   PT_SEM_WAIT(process_pt,&sem_##name)
/**
 * Signal a semaphore
 *
 * This macro carries out the "signal" operation on the semaphore.
 *
 * \param[in] name semaphore name
 * \note It's the application's responsibility to notify the waiting process if semaphore is avilable.
 * \sa PROCESS_WAIT_SEM()
 * \hideinitializer
 */
#define PROCESS_SIGNAL_SEM(name) PT_SEM_SIGNAL(process_pt,&sem_##name)
/** @} */

/**
 * \name poll and exit handlers
 * @{
 */
/**
 * \brief Specify an action when a process is polled.
 * \note This declaration must come immediately before the PROCESS_BEGIN() macro.
 * \param handler The action to be performed.
 */
#define PROCESS_POLLHANDLER(handler) if(ev == PROCESS_EVENT_POLL) { handler; }

/**
 * \brief Specify an action when a process exits.
 * \note This declaration must come immediately before the PROCESS_BEGIN() macro.
 * \param handler The action to be performed.
 */
#define PROCESS_EXITHANDLER(handler) if(ev == PROCESS_EVENT_EXIT) { handler; }
/** @} */

/**
 * \name process functions called from application programs
 * @{
 */
/**
 * \brief Start a process.
 * \param[inout] ptProcess A pointer to a process structure.
 * \param[in] arg An argument pointer that can be passed to the new process
 * \retval TRUE successful.
 * \retval FALSE Try to start a process that is already running.
 */
BOOL process_start(tPROCESS *ptProcess,process_data_t arg);
/**
 * \brief Post an asynchronous event to one or all processes.
 *
 * This function posts an asynchronous event to one or all
 * processes. The handing of the event is deferred until the target
 * process is scheduled by the kernel. An event can be broadcast to
 * all processes, in which case all processes in the system will be
 * scheduled to handle the event.
 *
 * \param[inout] ptProcess The process to which the event should be posted,or
 *               PROCESS_BROADCAST if the event should be posted to all processes.
 * \param[in] ev The event to be posted.
 * \param[inout] data The auxiliary data to be sent with the event
 * \retval TRUE The event could be posted.
 * \retval FALSE The event queue was full and the event could not be posted.
 * \sa process_send()
 */
BOOL process_post(tPROCESS *ptProcess,process_event_t ev,void* data);
/**
 * \brief Send a synchronous event to a process.
 *
 * This function sends a synchronous event to one processes.
 * On opposite of process_post(),the handing of the event is completed immediately after calling.
 *
 * \param[inout] ptProcess A pointer to the process' process structure.
 * \param[in] ev The event to be posted.
 * \param[inout] data A pointer to additional data that is posted together with the event.
 * \sa process_post()
 */
void process_send(tPROCESS *ptProcess,process_event_t ev,void* data);
/**
 * \brief Cause a process to exit
 *
 * This function causes a process to exit. The process can
 * either be the currently executing process, or another
 * process that is currently running.
 *
 * \param[inout] ptProcess The process that is to be exited
 * \sa PROCESS_CURRENT()
 */
void process_exit(tPROCESS *ptProcess);
/**
 * \brief Get a pointer to the currently running process.
 *
 * This macro get a pointer to the currently running
 * process. Typically, this macro is used to post an event to the
 * current process with process_post().
 */
#define PROCESS_CURRENT() process_current
extern tPROCESS *process_current;
/**
 * \brief Switch context to another process
 *
 * This function switch context to the specified process and executes
 * the code as if run by that process. Typical use of this function is
 * to switch context in services, called by other processes. Each
 * PROCESS_CONTEXT_BEGIN() must be followed by the
 * PROCESS_CONTEXT_END() macro to end the context switch.
 *
 * \param[in] ptProcess The process to use as context
 * \sa PROCESS_CONTEXT_END()
 * \sa PROCESS_CURRENT()
 *
 * Example:
 * \code
 * PROCESS_CONTEXT_BEGIN(&test_process);
 * etimer_set(&timer, CLOCK_SECOND);
 * PROCESS_CONTEXT_END(&test_process);
 * \endcode
 */
#define PROCESS_CONTEXT_BEGIN(ptProcess) { tPROCESS *tmp_current = PROCESS_CURRENT();process_current = ptProcess
/**
 * \brief End a context switch
 *
 * This function ends a context switch and changes back to the
 * previous process.
 *
 * \param[in] ptProcess The process used in the context switch
 * \sa PROCESS_CONTEXT_START()
 */
#define PROCESS_CONTEXT_END(ptProcess) process_current = tmp_current; }
/**
 * \brief Allocate a global event number.
 *
 *  event numbers above 128 are global and may
 *  be posted from one process to another. This function
 *  allocates one such event number.
 *
 * \note There currently is no way to deallocate an allocated event number.
 * \return The allocated event number
 */
process_event_t process_alloc_event(void);
/** @} */

/**
 * \name process functions called from device drivers.
 * @{
 */
/**
 * \brief Request a process to be polled.
 *
 * This function typically is called from an interrupt handler to
 * cause a process to be polled.
 *
 * \param[inout] ptProcess A pointer to the process' process structure.
 * \retval TRUE poll successfully.
 * \retval FALSE if the process is not running.
 */
BOOL process_poll(tPROCESS *ptProcess);
/** @} */

/**
 * \name functions called by the system and boot-up code
 * @{
 */
/**
 * \brief Run the system once - call poll handlers and process one event.
 *
 * This function should be called repeatedly from the main() program
 * to actually run the system. It calls the necessary poll
 * handlers, and processes one event. The function returns the number
 * of events that are waiting in the event queue so that the caller
 * may choose to put the CPU to sleep when there are no pending
 * events.
 *
 * Example about how to use protothread process model for event-driven system:
 * \code
 * int main(void) {
 *   RBS_Init();//RBS initialization
 *   process_start();//start process
 *   ...
 *   while (1) {
 *     do {
 *       ...//do some optional user code
 *     } while(process_run() > 0);
 *     sleep();//go to sleep
 *   }
 * }
 * \endcode
 *
 * \return The number of events that are currently waiting to be processed.
 */
U16 process_run(void);
/**
 * \brief Check if a process is running.
 *
 * This function checks if a specific process is running.A process can be on running state
 * after calling process_start() normally.
 *
 * \param[in] ptProcess The process pointer.
 * \retval TRUE if the process is running.
 * \retval FALSE if the process is not running.
 */
BOOL process_is_running(tPROCESS *ptProcess);
/**
 * \brief Number of events waiting to be processed.
 * \return Number of events that are currently waiting to be processed.
 */
 U16 process_nevents(void);
/** @} */
/** @} */

/**
 * \defgroup etimer Event timers
 * Event timers provides a way to generate timed events. An event
 * timer will post an event to the process that set the timer when the
 * event timer expires.
 *
 * An event timer is declared as a \c struct \c etimer and all access
 * to the event timer is made by a pointer to the declared event timer.
 *
 * @{
 */
/**
 * \brief Event timer structure.
 *
 * This structure is used for declaring a timer.The timer must be set
 * with etimer_set() before it can be used.
 */
typedef struct etime_struct {
  TICK start;
  TICK interval;
  struct etime_struct *next;
  tPROCESS *p;
} tETIME;

/**
 * \name etimer functions called from application programs
 * @{
 */
/**
 * \brief Set an event timer.
 *
 * This function is used to set an event timer for a time
 * sometime in the future. When the event timer expires,
 * the event PROCESS_EVENT_TIMER will be posted to the
 * process that called the etimer_set() function.
 *
 * \param[inout] ptEtime A pointer to the event timer
 * \param[in] interval The interval before the timer expires.
 */
void etimer_set(tETIME *ptEtime,TICK interval);
/**
 * \brief Reset an event timer with the same interval as was previously set.
 *
 * This function resets the event timer with the same
 * interval that was given to the event timer with the
 * etimer_set() function. The start point of the interval
 * is the exact time that the event timer last
 * expired. Therefore, this function will cause the timer
 * to be stable over time, unlike the etimer_restart() function.
 *
 * \param[inout] ptEtime A pointer to the event timer.
 * \sa etimer_restart()
 */
void etimer_reset(tETIME *ptEtime);
/**
 * \brief Restart an event timer from the current point in time.
 *
 * This function restarts the event timer with the same
 * interval that was given to the etimer_set()
 * function. The event timer will start at the current time.
 *
 * \param[inout] ptEtime A pointer to the event timer.
 * \note A periodic timer will drift if this function is
 *       used to reset it. For periodic timers, use the
 *       etimer_reset() function instead.
 * \sa etimer_reset()
 */
void etimer_restart(tETIME *ptEtime);
/**
 * \brief Adjust the expiration time for an event timer.
 *
 * This function is used to adjust the time the event
 * timer will expire. It can be used to synchronize
 * periodic timers without the need to restart the timer
 * or change the timer interval.
 *
 * \param[inout] ptEtime A pointer to the event timer.
 * \param[in] timediff The time difference to adjust the expiration time with.
 * \note This function should only be used for small
 *       adjustments. For large adjustments use etimer_set() instead.
 * \note A periodic timer will drift unless the etimer_reset() function is used.
 * \sa etimer_set()
 * \sa etimer_reset()
 */
void etimer_adjust(tETIME *ptEtime,int timediff);
/**
 * \brief Get the expiration time for the event timer.
 *
 * This function returns the expiration time for an event timer.
 *
 * \param[inout] ptEtime A pointer to the event timer.
 * \return The expiration time for the event timer.
 */
TICK etimer_expiration_time(tETIME *ptEtime);
/**
 * \brief Get the start time for the event timer.
 *
 * This function returns the start time (when the timer
 * was last set) for an event timer.
 *
 * \param[inout] ptEtime A pointer to the event timer
 * \return The start time for the event timer.
 */
TICK etimer_start_time(tETIME *ptEtime);
/**
 * \brief Check if an event timer has expired.
 *
 * This function tests if an event timer has expired and
 * returns true or false depending on its status.
 *
 * \param[in] ptEtime A pointer to the event timer.
 * \retval TRUE if the timer has expired.
 * \retval FALSE if the timer has not expired.
 */
BOOL etimer_expired(tETIME *ptEtime);
/**
 * \brief Stop a pending event timer.
 *
 * This function stops an event timer that has previously
 * been set with etimer_set() or etimer_reset(). After
 * this function has been called, the event timer will not
 * emit any event when it expires.
 *
 * \param[inout] ptEtime A pointer to the pending event timer.
 */
void etimer_stop(tETIME *ptEtime);
/** @} */

/**
 * \name etimer functions called from timer interrupts,by the system
 * @{
 */
/**
 * \brief Make the event timer aware that the clock has changed.
 *
 * This function is used to inform the event timer module
 * that the system clock has been updated.Typically,this
 * function would be called from the timer interrupt
 * handler when the clock has ticked.
 */
void etimer_request_poll(void);
/**
 * \brief Check if there are any non-expired event timers.
 *
 * This function checks if there are any active event
 * timers that have not expired.
 *
 * \retval TRUE if there are active event timers.
 * \retval FALSE if there are no active timers.
 */
BOOL etimer_pending(void);
/** @} */
/** @} */
#endif

/*
********************************************************************
*                          Debug
********************************************************************
*/

/**
 * \defgroup debug debug
 * used for system or application debug.
 * @{
 */
/**
 * \name statement macros
 * statement macros for debug
 * @{
 */
#if ((RBS_CFG_DEBUG_LEVEL >= RBS_DEBUG_LEVEL_ERRORS) || defined(USE_DOXYGEN))
  /**
   * \brief C statement `sta` is compiled if enable debug.
   * \param sta C statement
   */
  #define RBS_DEBUG_STA(sta)       sta
  /**
   * \brief C statement `if` is compiled if enable debug.
   *
   * This macro equals to `if (exp) { sta`
   *
   * \param exp C expression.
   * \param sta C statement.
   * \note this should used with `RBS_DEBUG_ELSIF()` and `RBS_DEBUG_ENDIF()`.
   * \sa RBS_DEBUG_ELSIF()
   * \sa RBS_DEBUG_ENDIF()
   */
  #define RBS_DEBUG_IF(exp,sta)    if (exp) { sta
  /**
   * \brief C statement `else if` is compiled if enable debug.
   *
   * This macro equals to `sta1;} else if (exp) { sta2`
   *
   * \param sta1 C statement.
   * \param exp C expression.
   * \param sta2 C statement.
   * \note this should used with `RBS_DEBUG_IF()` and `RBS_DEBUG_ENDIF()`.
   * \sa RBS_DEBUG_IF()
   * \sa RBS_DEBUG_ENDIF()
   */
  #define RBS_DEBUG_ELSIF(sta1,exp,sta2)  sta1;} else if (exp) { sta2
  /**
   * \brief C statement `}` for `if` is compiled if enable debug.
   *
   * This macro equals to `sta1;}`
   *
   * \param sta C statement.
   * \note this should used with `RBS_DEBUG_IF()` and `RBS_DEBUG_ELSIF()`.
   * \sa RBS_DEBUG_IF()
   * \sa RBS_DEBUG_ELSIF()
   */
  #define RBS_DEBUG_ENDIF(sta)     sta;}
#else
  #define RBS_DEBUG_STA(sta)
  #define RBS_DEBUG_IF(exp,sta)
  #define RBS_DEBUG_ELSIF(sta1,exp,sta2)
  #define RBS_DEBUG_ENDIF(sta)
#endif
/** @} */
/**
 * \name error macro function
 * error macros for debug output
 * @{
 */
#if ((RBS_CFG_DEBUG_LEVEL >= RBS_DEBUG_LEVEL_ERRORS) || defined(USE_DOXYGEN))
  #include <stdio.h>
  #include <string.h>
  #ifndef USE_DOXYGEN
    void RBS_Debug_Error(const char *s);
    extern char gDebugBuf[RBS_CFG_DEBUG_BUFSIZE+1];
    #define ASSERT_POS "ASSERT:"__FILE__"-"/*__FUNCTION__*/"()-Line:"STR(__LINE__)
    #define RBS_DEBUG_ASSERT(exp)  do{if(!(exp)){RBS_Debug_Error(ASSERT_POS);while(1);}}while(0)
    #define ERROR_POS "Error:"__FILE__"-"/*__FUNCTION__*/"()-Line:"STR(__LINE__)":"
  #endif
  /**
   * \brief Output error information.
   * \param[in] exp C language logical expression.
   * \param[in] s information string.
   * \param[in] sta C statement.
   * \note length of information string should not exceed `RBS_CFG_DEBUG_BUFSIZE`
   * \hideinitializer
   */
  #define RBS_DEBUG_ERROR(exp,s,sta) do{if(exp){RBS_Debug_Error(ERROR_POS s "\r\n");sta;}}while(0)
  /**
   * \brief Output format error information conditionally
   * \param[in] exp C language logical expression.
   * \param[in] format format information string.
   * \param[in] sta C statement.
   * \param[in] ... parameters.
   * \note length of information string should not exceed `RBS_CFG_DEBUG_BUFSIZE`
   * \hideinitializer
   */
  #define RBS_DEBUG_ERROR_FORMAT(exp,format,sta,...)  do{if(exp){sprintf(gDebugBuf,ERROR_POS format "\r\n",__VA_ARGS__);\
                                                         RBS_Debug_Error(gDebugBuf);sta;}}while(0)
#else
  #define RBS_DEBUG_ASSERT(exp)
  #define RBS_DEBUG_ERROR(exp,s,sta)
  #define RBS_DEBUG_ERROR_FORMAT(exp,format,...)
#endif
/** @} */
/**
 * \name warning macro function
 * warning macros for debug output
 * @{
 */
#if ((RBS_CFG_DEBUG_LEVEL >= RBS_DEBUG_LEVEL_WARNINGS) || defined(USE_DOXYGEN))
  #ifndef USE_DOXYGEN
    void RBS_Debug_Warn(const char *s);
    #define WARN_POS "WARN:"__FILE__"-"/*__FUNCTION__*/"()-"STR(__LINE__)":"
  #endif
  /**
   * \brief Output warning information conditionally
   * \param[in] exp C language logical expression.
   * \param[in] s information string.
   * \note length of information string should not exceed `RBS_CFG_DEBUG_BUFSIZE`
   * \hideinitializer
   */
  #define RBS_DEBUG_WARN(exp,s) do{if(exp)RBS_Debug_Warn(WARN_POS s "\r\n");}while(0)
  /**
   * \brief Output format warning information conditionally
   * \param[in] exp C language logical expression.
   * \param[in] format format information string.
   * \param[in] ... parameters.
   * \note length of information string should not exceed `RBS_CFG_DEBUG_BUFSIZE`
   * \hideinitializer
   */
  #define RBS_DEBUG_WARN_FORMAT(exp,format,...)  do{if(exp){sprintf(gDebugBuf,WARN_POS format "\r\n",__VA_ARGS__);\
                                                    RBS_Debug_Warn(gDebugBuf);}}while(0)
#else
  #define RBS_DEBUG_WARN(exp,s)
  #define RBS_DEBUG_WARN_FORMAT(exp,format,...)
#endif
/** @} */
/**
 * \name log macro function
 * log macros for debug output
 * @{
 */
#if ((RBS_CFG_DEBUG_LEVEL >= RBS_DEBUG_LEVEL_LOG) || defined(USE_DOXYGEN))
  #ifndef USE_DOXYGEN
    void RBS_Debug_Log(const char *s);
  #endif
 /**
  * \brief Output format log information.
  * \param[in] s information string.
  * \note length of information string should not exceed `RBS_CFG_DEBUG_BUFSIZE`
  * \hideinitializer
  */
  #define RBS_DEBUG_LOG(s)  RBS_Debug_Log(s "\r\n")
 /**
  * \brief Output format log information.
  * \param[in] format format information string.
  * \param[in] ... parameters.
  * \note length of information string should not exceed `RBS_CFG_DEBUG_BUFSIZE`
  * \hideinitializer
  */
  #define RBS_DEBUG_LOG_FORMAT(format,...) do{sprintf(gDebugBuf,format "\r\n",__VA_ARGS__);\
                                              RBS_Debug_Log(gDebugBuf);}while(0)
#else
  #define RBS_DEBUG_LOG(s)
  #define RBS_DEBUG_LOG_FORMAT(format,...)
#endif
/** @} */
/** @} */

#if defined(__cplusplus)
}
#endif

#endif

/*************************** End of file ****************************/
