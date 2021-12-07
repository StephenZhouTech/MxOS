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
#include "arch.h"
#include "os_types.h"
#include "os_task.h"
#include "os_error_code.h"
#include "os_configs.h"
#include "os_critical.h"
#include "os_time.h"
#include "os_scheduler.h"
#include "os_mem.h"
#include "os_lib.h"

#include <stdio.h>

extern OS_TaskScheduler_t Scheduler;

char *TaskStateString[] =
{
    "READY_STATE",
    "DELAY_STATE",
    "SUSPEND_STATE",
    "BLOCKED_STATE",
    "UNKNOWN_STATE"
};

void OS_DBG_TASK_DumpTaskInfo(OS_TCB_t *TaskCB)
{
    printf("Dump Task Info:\r\n");
    printf("--------------------------\r\n");
    printf("Task Name: %s\r\n", TaskCB->TaskName);
    printf("Task Priority: %d\r\n", TaskCB->Priority);
    printf("Task State: %s\r\n", TaskStateString[TaskCB->State]);
    printf("Task WakeUpTime: 0x%08X\r\n", TaskCB->WakeUpTime);
    printf("--------------------------\r\n");
}

void OS_DBG_SCH_DumpReadyList(void)
{
    OS_Uint32_t i = 0;
    ListHead_t *StateListHead = OS_NULL;
    ListHead_t *ListIterator  = OS_NULL;
    OS_TCB_t   *TCB_Iterator  = OS_NULL;

    printf("[Dump Ready List]:\r\n");
    printf("[PriorityActive = 0x%08X]\r\n", Scheduler.PriorityActive);

    for (i = 0; i < OS_MAX_TASK_PRIORITY; i++)
    {
        if (Scheduler.PriorityActive & (0x01 << i))
        {
            printf("--<Priority=%d>:\r\n", i);
            StateListHead = &Scheduler.ReadyListHead[i];
            ListForEach(ListIterator, StateListHead)
            {
                TCB_Iterator = ListEntry(ListIterator, OS_TCB_t, StateList);
                printf("    ## Task Name: %s\r\n", TCB_Iterator->TaskName);
            }
        }
    }
}

void OS_DBG_SCH_DumpDelayList(void)
{
    ListHead_t *StateListHead = OS_NULL;
    ListHead_t *ListIterator  = OS_NULL;
    OS_TCB_t   *TCB_Iterator  = OS_NULL;

    printf("[Dump Delay List]:\r\n");

    StateListHead = &Scheduler.DelayListHead;

    if (ListEmpty(StateListHead))
    {
        printf("    ## Empty...\r\n");
    }
    else
    {
        ListForEach(ListIterator, StateListHead)
        {
            TCB_Iterator = ListEntry(ListIterator, OS_TCB_t, StateList);
            printf("    ## Task Name: %s\r\n", TCB_Iterator->TaskName);
            printf("    ## Task Wakeup time: 0x%08X\r\n", TCB_Iterator->WakeUpTime);
        }
    }
}

void OS_DBG_SCH_DumpSuspendList(void)
{
    ListHead_t *StateListHead = OS_NULL;
    ListHead_t *ListIterator  = OS_NULL;
    OS_TCB_t   *TCB_Iterator  = OS_NULL;

    printf("[Dump Suspend List]:\r\n");

    StateListHead = &Scheduler.SuspendListHead;

    if (ListEmpty(StateListHead))
    {
        printf("    ## Empty...\r\n");
    }
    else
    {
        ListForEach(ListIterator, StateListHead)
        {
            TCB_Iterator = ListEntry(ListIterator, OS_TCB_t, StateList);
            printf("    ## Task Name: %s\r\n", TCB_Iterator->TaskName);
        }
    }
}

void OS_DBG_SCH_DumpBlockList(void)
{
    ListHead_t *StateListHead = OS_NULL;
    ListHead_t *ListIterator  = OS_NULL;
    OS_TCB_t   *TCB_Iterator  = OS_NULL;

    printf("[Dump Block List]:\r\n");

    StateListHead = &Scheduler.BlockListHead;

    if (ListEmpty(StateListHead))
    {
        printf("    ## Empty...\r\n");
    }
    else
    {
        ListForEach(ListIterator, StateListHead)
        {
            TCB_Iterator = ListEntry(ListIterator, OS_TCB_t, StateList);
            printf("    ## Task Name: %s\r\n", TCB_Iterator->TaskName);
        }
    }
}

void OS_DBG_SCH_DumpAllStateList(void)
{
    OS_DBG_SCH_DumpReadyList();
    OS_DBG_SCH_DumpDelayList();
    OS_DBG_SCH_DumpSuspendList();
    OS_DBG_SCH_DumpBlockList();
}

