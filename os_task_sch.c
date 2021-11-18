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
#include "os_list.h"
#include "os_mem.h"
#include "arch.h"
#include "os_configs.h"
#include "os_task_sch.h"
#include "os_error_code.h"

typedef enum _OS_ScheduerState {
    SCHEDULER_SUSPEND = 0,
    SCHEDULER_RUNNING
} OS_ScheduerState_e;

typedef struct _OS_TaskControlBlock {
    ListHead_t      StateList;
    OS_Int8_t       TaskName[CONFIG_TASK_NAME_LEN];
    OS_Uint8_t      Priority;
    OS_Uint32_t     *StackPointer;
} OS_TCB_t;

typedef struct _OS_TaskScheduler {
    ListHead_t      ReadyListHead[OS_MAX_TASK_PRIORITY];
    ListHead_t      BlockListHead;
    ListHead_t      SuspendListHead;
    OS_Uint32_t     PriorityActive;
    OS_Uint32_t     CurrentTime;
    OS_Uint8_t      ScheduerState;
} OS_TaskScheduler_t;

OS_Uint32_t OS_API_TaskCreate(  TaskFunction_t Func,
                                const char * Name,
                                OS_Uint32_t StackDepth,
                                OS_Uint32_t Priority,
                                OS_Uint32_t *Handler,
                                void * const Parameters )
{
    if (Name == OS_NULL || Handler == OS_NULL)
    {
        return OS_NULL_POINTER;
    }

    return OS_SUCCESS;
}
