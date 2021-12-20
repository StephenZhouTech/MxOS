/*
 * MxOS Kernel V0.1
 * Copyright (C) 2020 StephenZhou.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * 1 tab == 4 spaces!
 */

#ifndef __MXOS_MUTEX_H__
#define __MXOS_MUTEX_H__

#include "os_types.h"
#include "os_list.h"
#include "os_task.h"

typedef struct _OS_Mutex {
    ListHead_t  SleepList;
    OS_TCB_t    *Owner;
    OS_Uint32_t OwnerHoldCount;
    OS_Uint8_t  OwnerPriority;
    OS_Uint8_t  Used;
} OS_Mutex_t;

typedef enum _OS_MutexUsed {
    OS_MUTEX_UNUSED = 0,
    OS_MUTEX_USED
} OS_MutexUsed_e;

OS_Uint32_t OS_API_MutexCreate(OS_Uint32_t *MutexHandle);

OS_Uint32_t OS_API_MutexLock(OS_Uint32_t MutexHandle);

OS_Uint32_t OS_API_MutexLockTimeout(OS_Uint32_t MutexHandle, OS_Uint32_t Timeout);

OS_Uint32_t OS_API_MutexTryLock(OS_Uint32_t MutexHandle);

OS_Uint32_t OS_API_MutexUnlock(OS_Uint32_t MutexHandle);

OS_Uint32_t OS_API_MutexDestory(OS_Uint32_t MutexHandle);

#endif // __MXOS_MUTEX_H__
