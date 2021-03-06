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

#ifndef __MXOS_TASK_H__
#define __MXOS_TASK_H__

#include "os_types.h"
#include "os_configs.h"
#include "os_list.h"

#define OS_MAX_TASK_PRIORITY                    32
#define OS_TASK_MAGIC_NUMBER                    0xA5
#define OS_TASK_STACK_BOUNDARY                  0xA5A5A5A5

typedef void (*TaskFunction_t)(void *PrivateData);

typedef struct _OS_TaskControlBlock {
    void            *Stack;
#if CONFIG_STACK_OVERFLOW_CHECK
    void            *StartOfStack;
#endif
    OS_Uint8_t      Priority;
    ListHead_t      StateList;
    ListHead_t      IpcSleepList;
#if CONFIG_USE_SHELL
    ListHead_t      TasksList;
#endif
    OS_Uint8_t      IpcTimeoutWakeup;
    OS_Uint8_t      State;
    OS_Int8_t       TaskName[CONFIG_TASK_NAME_LEN];
    OS_Uint32_t     WakeUpTime;
} OS_TCB_t;

typedef struct _TaskInitParameter {
    TaskFunction_t  TaskEntry;
    OS_Uint8_t      Priority;
    OS_Int8_t       Name[CONFIG_TASK_NAME_LEN];
    OS_Uint32_t     StackSize;
    void            *PrivateData;
} TaskInitParameter;

typedef enum _OS_TaskState {
    OS_TASK_READY = 0,
    OS_TASK_DELAY,
    OS_TASK_SUSPEND,
    OS_TASK_ENDLESS_BLOCKED,
    OS_TASK_TIMEOUT_BLOCKED,
    OS_TASK_UNKNOWN
} OS_TaskState_e;

typedef enum _OS_IpcTimeoutWakeup {
    OS_IPC_NO_TIMEOUT = 0,
    OS_IPC_WAIT_TIMEOUT
} OS_IpcTimeoutWakeup_e;

#define OS_TSK_HANDLE_TO_TCB(Handle)            ((OS_TCB_t *)Handle)

OS_Uint32_t OS_API_TaskCreate(TaskInitParameter Param, OS_Uint32_t *TaskHandle);
OS_Uint32_t OS_API_TaskDelay(OS_Uint32_t TickCnt);
OS_Uint32_t OS_API_TaskSuspend(OS_Uint32_t TaskHandle);
OS_Uint32_t OS_API_TaskResume(OS_Uint32_t TaskHandle);

OS_Uint32_t OS_API_TaskPrioritySet(OS_Uint32_t TaskHandle, OS_Uint8_t NewPriority);
OS_Uint8_t OS_API_TaskPriorityGet(OS_Uint32_t TaskHandle);

#endif // __MXOS_TASK_H__
