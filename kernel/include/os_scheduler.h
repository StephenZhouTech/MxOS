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

#ifndef __MXOS_SCHEDULER_H__
#define __MXOS_SCHEDULER_H__

#include "os_task.h"
#include "os_configs.h"

typedef struct _OS_TaskScheduler {
    ListHead_t      ReadyListHead[OS_MAX_TASK_PRIORITY];
    ListHead_t      BlockTimeoutListHead;
    ListHead_t      SuspendListHead;
    ListHead_t      DelayListHead;
#if CONFIG_USE_SHELL
    ListHead_t      AllTasksListHead;
#endif
    OS_Uint32_t     PriorityActive;
    OS_Int16_t      SchedulerSuspendNesting;
    OS_Uint8_t      ReSchedulePending;
} OS_TaskScheduler_t;

typedef enum _OS_SchedulerStateList {
    OS_READY_LIST = 0,
    OS_DELAY_LIST,
    OS_SUSPEND_LIST,
    OS_BLOCKED_TIMEOUT_LIST
} OS_SchedulerStateList_e;

typedef enum _OS_BlockType {
    OS_BLOCK_TYPE_ENDLESS = 0,
    OS_BLOCK_TYPE_TIMEOUT
} OS_BlockType_e;

typedef enum _OS_BlockSortType {
    OS_BLOCK_SORT_FIFO = 0,
    OS_BLOCK_SORT_TASK_PRIO
} OS_BlockSortType_e;

typedef enum _OS_SchedulerReschPending {
    NO_RESCH_PENDING = 0,
    RESCH_PENDING
} OS_SchedulerReschPending_e;

void OS_API_SchedulerSuspend(void);
void OS_API_SchedulerResume(void);
void OS_SystemTickHander(void);

#endif // __MXOS_SCHEDULER_H__
