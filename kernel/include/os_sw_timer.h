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

#ifndef __MXOS_SW_TIMER_H__
#define __MXOS_SW_TIMER_H__

#include "os_types.h"
#include "os_list.h"

typedef void (*OS_SwTimerHandler_t)(void *Param);

typedef struct _OS_SwTimerManager {
    ListHead_t      TimerNodesList;
    OS_Uint32_t     NextWakeupTime;
} OS_SwTimerManager_t;

typedef struct _OS_SwTimerNode {
    ListHead_t              Node;
    OS_Uint8_t              Status;
    OS_Uint8_t              Mode;
    OS_Uint32_t             Interval;
    OS_SwTimerHandler_t     Handler;
    OS_Uint32_t             WakeupTime;
    OS_Uint8_t              Used;
    void                    *Param;
} OS_SwTimerNode_t;

typedef enum _OS_SwTimerMode {
    OS_SW_TIMER_ONESHOT = 0,
    OS_SW_TIMER_AUTO_RELOAD
} OS_SwTimerMode_e;

typedef enum _OS_SwTimerUsed {
    OS_SW_TIMER_UNUSED = 0,
    OS_SW_TIMER_USED
} OS_SwTimerUsed_e;

typedef enum _OS_SwTimerStatus {
    OS_SW_TIMER_STOP = 0,
    OS_SW_TIMER_RUNNING
} OS_SwTimerStatus_e;

OS_Uint32_t OS_API_SwTimerCreate(OS_Uint32_t *SwTimerHandle,
                                 OS_Uint8_t WorkMode,
                                 OS_Uint32_t Interval,
                                 OS_SwTimerHandler_t TimeoutHandler,
                                 void *FuncParam);

OS_Uint32_t OS_API_SwTimerStart(OS_Uint32_t SwTimerHandle);

OS_Uint32_t OS_API_SwTimerStop(OS_Uint32_t SwTimerHandle);

OS_Uint32_t OS_API_SwTimerDelete(OS_Uint32_t SwTimerHandle);

#endif // __MXOS_SW_TIMER_H__
