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

#include "arch.h"
#include "os_time.h"
#include "os_list.h"
#include "os_types.h"
#include "os_trace.h"
#include "os_configs.h"
#include "os_critical.h"
#include "os_scheduler.h"
#include "os_error_code.h"

#define OS_SCHEDULER_LOCK()               OS_API_EnterCritical()
#define OS_SCHEDULER_UNLOCK()             OS_API_ExitCritical()

#if OS_DBG_SCHEDULER
    #define SCH_FUNCTION_SPACE
#else
    #define SCH_FUNCTION_SPACE static
#endif

SCH_FUNCTION_SPACE OS_TaskScheduler_t Scheduler;

extern OS_TCB_t * volatile CurrentTCB;
extern OS_TCB_t * volatile SwitchNextTCB;

SCH_FUNCTION_SPACE void SetPriorityActive(OS_Uint8_t ActiveBit)
{
    Scheduler.PriorityActive |= (0x01 << ActiveBit);
}

SCH_FUNCTION_SPACE void ClearPriorityActive(OS_Uint8_t ActiveBit)
{
    Scheduler.PriorityActive &= (~(0x01 << ActiveBit));
}

OS_TCB_t * OS_HighestPrioTaskGet(void)
{
    OS_Uint8_t TargetPri = 0;
    OS_TCB_t * TargetTCB = OS_NULL;

#if CONFIG_ARM_ARCH
    TargetPri = (31 - __clz(Scheduler.PriorityActive));
#else
    {
        OS_Uint8_t TryBit = 31;
        for (; TryBit >=0; TryBit--)
        {
            if (Scheduler.PriorityActive & (0x01 << TryBit))
                break;
        }
        TargetPri = TryBit;
    }
#endif

    OS_ASSERT(TargetPri <= 31);
    OS_ASSERT(!ListEmpty(&Scheduler.ReadyListHead[TargetPri]));

    TargetTCB = ListFirstEntry(&Scheduler.ReadyListHead[TargetPri], OS_TCB_t, StateList);
    return TargetTCB;
}

OS_Int16_t OS_IsSchedulerSuspending(void)
{
    OS_Int16_t IsSuspending = 0;

    OS_SCHEDULER_LOCK();

    IsSuspending = Scheduler.SchedulerSuspendNesting;
    OS_ASSERT(IsSuspending >= 0);

    OS_SCHEDULER_UNLOCK();

    return IsSuspending;
}

void OS_API_SchedulerSuspend(void)
{
    OS_SCHEDULER_LOCK();
    Scheduler.SchedulerSuspendNesting++;

    TRACE_SchedulerSuspend(Scheduler.SchedulerSuspendNesting);

    OS_SCHEDULER_UNLOCK();
}

void OS_API_SchedulerResume(void)
{
    OS_SCHEDULER_LOCK();
    Scheduler.SchedulerSuspendNesting--;

    TRACE_SchedulerResume(Scheduler.SchedulerSuspendNesting);

    OS_ASSERT(Scheduler.SchedulerSuspendNesting >= 0);
    OS_SCHEDULER_UNLOCK();
}

OS_Uint8_t OS_CheckTaskInTargetList(OS_TCB_t *TargetTCB, OS_Uint8_t TargetList)
{
    OS_Uint8_t Ret = 0;
    ListHead_t *StateListHead = OS_NULL;
    ListHead_t *ListIterator  = OS_NULL;
    OS_TCB_t   *TCB_Iterator  = OS_NULL;

    switch (TargetList)
    {
        case OS_READY_LIST:
        {
            StateListHead = &Scheduler.ReadyListHead[TargetTCB->Priority];
        }
        break;

        case OS_DELAY_LIST:
        {
            StateListHead = &Scheduler.DelayListHead;
        }
        break;

        case OS_SUSPEND_LIST:
        {
            StateListHead = &Scheduler.SuspendListHead;
        }
        break;

        case OS_BLOCKED_LIST:
        {
            StateListHead = &Scheduler.BlockListHead;
        }
        break;

        default : break;
    }

    OS_ASSERT(StateListHead != OS_NULL);

    ListForEach(ListIterator, StateListHead)
    {
        TCB_Iterator = ListEntry(ListIterator, OS_TCB_t, StateList);
        if (TargetTCB == TCB_Iterator)
        {
            Ret = 1;
            break;
        }
    }

    return Ret;
}

void OS_AddTaskToReadyList(OS_TCB_t * TaskCB)
{
    OS_ASSERT(TaskCB->State != OS_TASK_READY);

    ListAdd(&TaskCB->StateList, &Scheduler.ReadyListHead[TaskCB->Priority]);
    SetPriorityActive(TaskCB->Priority);
    TaskCB->State = OS_TASK_READY;

    TRACE_AddToTargetList(TP_READY_LIST, TaskCB);
}

void OS_AddTaskToDelayList(OS_TCB_t *TaskCB)
{
    ListHead_t *ListIterator = OS_NULL;
    OS_TCB_t   *TCB_Iterator = OS_NULL;

    OS_ASSERT(TaskCB->State != OS_TASK_DELAY);

    if (ListEmpty(&Scheduler.DelayListHead))
    {
        ListAdd(&TaskCB->StateList, &Scheduler.DelayListHead);
    }
    else
    {
        // Insert in the delay list with sort
        ListForEach(ListIterator, &Scheduler.DelayListHead)
        {
            TCB_Iterator = ListEntry(ListIterator, OS_TCB_t, StateList);
            // If TCB_Iterator->WakeUpTime after TargetTCB->WakeUpTime
            if (OS_TIME_AFTER_EQ(TCB_Iterator->WakeUpTime, TaskCB->WakeUpTime))
                break;
        }

        // Do not find position
        if (ListIterator == &Scheduler.DelayListHead)
        {
            ListAddTail(&TaskCB->StateList, &Scheduler.DelayListHead);
        }
        else
        {
            ListAdd(&TaskCB->StateList, TCB_Iterator->StateList.prev);
        }
    }
    TaskCB->State = OS_TASK_DELAY;

    TRACE_AddToTargetList(TP_DELAY_LIST, TaskCB);
}

void OS_AddTaskToSuspendList(OS_TCB_t * TaskCB)
{
    OS_ASSERT(TaskCB->State != OS_TASK_SUSPEND);

    ListAdd(&TaskCB->StateList, &Scheduler.SuspendListHead);
    TaskCB->State = OS_TASK_SUSPEND;

    TRACE_AddToTargetList(TP_SUSPEND_LIST, TaskCB);
}

void OS_AddTaskToBlockedList(OS_TCB_t * TaskCB)
{
    OS_ASSERT(TaskCB->State != OS_TASK_BLOCKED);

    ListAdd(&TaskCB->StateList, &Scheduler.BlockListHead);
    TaskCB->State = OS_TASK_BLOCKED;

    TRACE_AddToTargetList(TP_BLOCKED_LIST, TaskCB);
}

void OS_RemoveTaskFromReadyList(OS_TCB_t * TaskCB)
{
    OS_ASSERT(TaskCB->State == OS_TASK_READY);

    ListDel(&TaskCB->StateList);
    if ( ListEmpty(&Scheduler.ReadyListHead[TaskCB->Priority]) )
    {
        ClearPriorityActive(TaskCB->Priority);
    }
    TaskCB->State = OS_TASK_UNKNOWN;

    TRACE_RemoveFromTargetList(TP_READY_LIST, TaskCB);
}

void OS_RemoveTaskFromDelayList(OS_TCB_t * TaskCB)
{
    OS_ASSERT(TaskCB->State == OS_TASK_DELAY);

    ListDel(&TaskCB->StateList);
    TaskCB->State = OS_TASK_UNKNOWN;

    TRACE_RemoveFromTargetList(TP_DELAY_LIST, TaskCB);
}

void OS_RemoveTaskFromSuspendList(OS_TCB_t * TaskCB)
{
    OS_ASSERT(TaskCB->State == OS_TASK_SUSPEND);

    ListDel(&TaskCB->StateList);
    TaskCB->State = OS_TASK_UNKNOWN;

    TRACE_RemoveFromTargetList(TP_SUSPEND_LIST, TaskCB);
}

void OS_RemoveTaskFromBlockedList(OS_TCB_t * TaskCB)
{
    OS_ASSERT(TaskCB->State == OS_TASK_BLOCKED);

    ListDel(&TaskCB->StateList);
    TaskCB->State = OS_TASK_UNKNOWN;

    TRACE_RemoveFromTargetList(TP_BLOCKED_LIST, TaskCB);
}

void OS_TaskReadyToDelay(OS_TCB_t * TaskCB)
{
    /* Remove from ready list firstly */
    OS_RemoveTaskFromReadyList(TaskCB);
    /* Add it in delay list */
    OS_AddTaskToDelayList(TaskCB);
}

void OS_TaskDelayToReady(OS_TCB_t * TaskCB)
{
    /* Remove from delay list firstly */
    OS_RemoveTaskFromDelayList(TaskCB);
    /* Add it in ready list */
    OS_AddTaskToReadyList(TaskCB);
}

void OS_TaskCheckWakeUp(OS_Uint32_t time)
{
    ListHead_t *ListIterator = OS_NULL;
    ListHead_t *ListIterator_prev = OS_NULL;
    OS_TCB_t   *TCB_Iterator = OS_NULL;

    if (ListEmpty(&Scheduler.DelayListHead))
    {
        return;
    }

    // Find out the task which should be wake up
    ListForEach(ListIterator, &Scheduler.DelayListHead)
    {
        TCB_Iterator = ListEntry(ListIterator, OS_TCB_t, StateList);
        // Check if the task is timeout
        if (OS_TIME_AFTER_EQ(time, TCB_Iterator->WakeUpTime))
        {
            TRACE_TaskDelayTimeout(TCB_Iterator);

            ListIterator_prev = ListIterator->prev;
            OS_TaskDelayToReady(TCB_Iterator);
            // Current ListIterator will link to ready list, relocate the pointer
            ListIterator = ListIterator_prev;
        }
        else
        {
            break;
        }
    }
}

void OS_Schedule(void)
{
    OS_Uint8_t NeedResch = 0;

    /* Check if the scheduler is suspend */
    if (OS_IsSchedulerSuspending() != 0)
        return ;

    /* Find the highest priority task now */
    SwitchNextTCB = OS_HighestPrioTaskGet();

    /* Check CurrentTCB is in the ready list */
    if (CurrentTCB->State != OS_TASK_READY)
    {
        NeedResch = 1;
        ListMoveTail(&SwitchNextTCB->StateList, &Scheduler.ReadyListHead[SwitchNextTCB->Priority]);
        goto _OS_ScheduleRightNow;
    }

    /* Check wich task have the highset prioirty */
    if (SwitchNextTCB->Priority > CurrentTCB->Priority)
    {
        NeedResch = 1;
        ListMoveTail(&SwitchNextTCB->StateList, &Scheduler.ReadyListHead[SwitchNextTCB->Priority]);
    }

    // The next task have the same prioirty as before
    if (SwitchNextTCB->Priority == CurrentTCB->Priority)
    {
        // Check the next is the same as before
        if (SwitchNextTCB != CurrentTCB)
        {
            NeedResch = 1;
            // Move the next to the list of highest priority list
            ListMoveTail(&SwitchNextTCB->StateList, &Scheduler.ReadyListHead[SwitchNextTCB->Priority]);
        }
        else
        {
            // Only one task in the highest prioirty list
            if (ListIsLast(&SwitchNextTCB->StateList, &Scheduler.ReadyListHead[SwitchNextTCB->Priority]))
            {
                NeedResch = 0;
            }
            else
            {
                // more than one task in highest priority task list
                NeedResch = 1;

                SwitchNextTCB = ListEntry(CurrentTCB->StateList.next, OS_TCB_t, StateList);
                ListMoveTail(&CurrentTCB->StateList, &Scheduler.ReadyListHead[SwitchNextTCB->Priority]);
                ListMoveTail(&SwitchNextTCB->StateList, &Scheduler.ReadyListHead[SwitchNextTCB->Priority]);
            }
        }
    }

_OS_ScheduleRightNow:
    if (NeedResch)
    {
        TRACE_ContextSwitch(CurrentTCB, SwitchNextTCB, OS_GetCurrentTime());

        ARCH_TriggerContextSwitch((void *)CurrentTCB, (void *)SwitchNextTCB);
    }
}

void OS_SchedulerInit(void)
{
    OS_Uint32_t i = 0;

    /* Initial the task ReadyList */
    for (i = 0; i < OS_MAX_TASK_PRIORITY; i++)
    {
        ListHeadInit(&Scheduler.ReadyListHead[i]);
    }
    ListHeadInit(&Scheduler.BlockListHead);
    ListHeadInit(&Scheduler.SuspendListHead);
    ListHeadInit(&Scheduler.DelayListHead);

    Scheduler.SchedulerSuspendNesting = 0;
}

void OS_SystemTickHander(void)
{
    OS_SCHEDULER_LOCK();

    /* Check if the scheduler is active */
    if (Scheduler.SchedulerSuspendNesting != 0)
        goto SystemTickHanderExit;

    /* Increment of System Tick */
    OS_IncrementTime();

    TRACE_IncrementTick(OS_GetCurrentTime());

    OS_TaskCheckWakeUp(OS_GetCurrentTime());

    /* Schedule */
    OS_Schedule();

SystemTickHanderExit:
    OS_SCHEDULER_UNLOCK();
}
