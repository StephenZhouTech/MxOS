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

#include "os_time.h"
#include "os_task.h"
#include "os_types.h"
#include "os_trace.h"
#include "os_configs.h"
#include "os_critical.h"
#include "os_sw_timer.h"
#include "os_scheduler.h"
#include "os_error_code.h"

#if CONFIG_USE_SW_TIMER

#define OS_SW_TIMER_TASK_PRIO                       31

#define OS_SW_TIMER_LOCK()                          OS_API_EnterCritical()
#define OS_SW_TIMER_UNLOCK()                        OS_API_ExitCritical()

OS_SwTimerManager_t SwTimerManager;
OS_SwTimerNode_t SwTimerNode[CONFIG_MAX_TIMER_DEFINE];
static OS_Uint32_t OS_SwTimerTaskHandle = 0;

extern void OS_TaskSuspendToReady(OS_TCB_t * TaskCB);
extern void OS_TaskReadyToSuspend(OS_TCB_t * TaskCB);
extern void OS_Schedule(void);

#define OS_SW_TIMER_CHECK_HANDLE_VALID(HANDLE)          \
{                                                       \
    if (HANDLE >= CONFIG_MAX_TIMER_DEFINE)              \
    {                                                   \
        return OS_SW_TIMER_HANDLE_INVALID;              \
    }                                                   \
}

#define OS_SW_TIMER_CHECK_BEEN_CREATED(HANDLE)          \
{                                                       \
    if (SwTimerNode[HANDLE].Used == OS_SW_TIMER_UNUSED) \
    {                                                   \
        return OS_SW_TIMER_NOT_BEEN_CREATED;            \
    }                                                   \
}

#define OS_SW_TIMER_HANDLE_TO_POINTER(HANDLE)            &SwTimerNode[HANDLE]

void OS_SwTimerInit(void)
{
    OS_Uint32_t i = 0;

    for (i = 0; i < CONFIG_MAX_TIMER_DEFINE; i++)
    {
        SwTimerNode[i].Used = OS_SW_TIMER_UNUSED;
    }

    SwTimerManager.NextWakeupTime = 0;
    ListHeadInit(&SwTimerManager.TimerNodesList);
}

OS_Uint32_t OS_GetSwTimerResource(OS_Uint32_t *SwTimerHandle)
{
    OS_Uint32_t i = 0;

    for (i = 0; i < CONFIG_MAX_TIMER_DEFINE; i++)
    {
        if (SwTimerNode[i].Used == OS_SW_TIMER_UNUSED)
            break;
    }

    if (i == CONFIG_MAX_TIMER_DEFINE)
    {
        return OS_NOT_ENOUGH_SW_TMR_RESOURCE;
    }
    else
    {
        *SwTimerHandle = i;
    }

    return OS_SUCCESS;
}

void OS_UpdateSwTimerNextWakeup(OS_Uint32_t NextWakeupTime)
{
    SwTimerManager.NextWakeupTime = NextWakeupTime;
}

OS_Uint32_t OS_SwTimerNextWakeupTime(void)
{
    return SwTimerManager.NextWakeupTime;
}

OS_Uint32_t OS_API_SwTimerCreate(OS_Uint32_t *SwTimerHandle,
                                 OS_Uint8_t WorkMode,
                                 OS_Uint32_t Interval,
                                 OS_SwTimerHandler_t TimeoutHandler,
                                 void *FuncParam)
{
    OS_Uint32_t Ret = OS_SUCCESS;

    OS_CHECK_NULL_POINTER(SwTimerHandle);
    OS_CHECK_NULL_POINTER(TimeoutHandler);

    if (WorkMode != OS_SW_TIMER_ONESHOT && WorkMode != OS_SW_TIMER_AUTO_RELOAD)
    {
        return OS_SW_TMR_INVALID_MODE;
    }

    if (Interval >= OS_TSK_DLY_MAX)
    {
        return OS_SW_TMR_INVALID_INTERVAL;
    }

    OS_SW_TIMER_LOCK();

    Ret = OS_GetSwTimerResource(SwTimerHandle);
    if (Ret != OS_SUCCESS)
        goto OS_API_SwTimerCreate_Exit;

    SwTimerNode[*SwTimerHandle].Mode = WorkMode;
    SwTimerNode[*SwTimerHandle].Interval = Interval;
    SwTimerNode[*SwTimerHandle].Handler = TimeoutHandler;
    SwTimerNode[*SwTimerHandle].WakeupTime = 0;
    SwTimerNode[*SwTimerHandle].Used = OS_SW_TIMER_USED;
    SwTimerNode[*SwTimerHandle].Param = FuncParam;
    SwTimerNode[*SwTimerHandle].Status = OS_SW_TIMER_STOP;
    ListHeadInit(&SwTimerNode[*SwTimerHandle].Node);

    TRACE_SwTimerCreate(SwTimerHandle, WorkMode, Interval);

OS_API_SwTimerCreate_Exit:
    OS_SW_TIMER_UNLOCK();

    return Ret;
}

static void OS_AddSwTimerToManager(OS_SwTimerNode_t *SwTimer)
{
    ListHead_t *TargetListHead = &SwTimerManager.TimerNodesList;
    ListHead_t *ListIterator = OS_NULL;
    OS_SwTimerNode_t *SwTimerIterator = OS_NULL;

    /* The list should be sort as wakeup time */
    if (ListEmpty(TargetListHead))
    {
        ListAdd(&SwTimer->Node, TargetListHead);
    }
    else
    {
        // Insert in the list with sort
        ListForEach(ListIterator, TargetListHead)
        {
            SwTimerIterator = ListEntry(ListIterator, OS_SwTimerNode_t, Node);
            // If SwTimerIterator->WakeupTime after SwTimer->WakeupTime
            if (OS_TIME_AFTER_EQ(SwTimerIterator->WakeupTime, SwTimer->WakeupTime))
                break;
        }

        // Do not find position
        if (ListIterator == TargetListHead)
        {
            ListAddTail(&SwTimer->Node, TargetListHead);
        }
        else
        {
            ListAdd(&SwTimer->Node, ListIterator->prev);
        }
    }

    /* Update the next NextWakeupTime */
    SwTimerIterator = ListFirstEntry(TargetListHead, OS_SwTimerNode_t, Node);

    OS_UpdateSwTimerNextWakeup(SwTimerIterator->WakeupTime);
}

OS_Uint32_t OS_API_SwTimerStart(OS_Uint32_t SwTimerHandle)
{
    OS_Uint32_t Ret = OS_SUCCESS;
    OS_SwTimerNode_t *SwTimer = OS_NULL;

    OS_SW_TIMER_CHECK_HANDLE_VALID(SwTimerHandle);
    OS_SW_TIMER_CHECK_BEEN_CREATED(SwTimerHandle);

    OS_SW_TIMER_LOCK();

    SwTimer = OS_SW_TIMER_HANDLE_TO_POINTER(SwTimerHandle);

    if (SwTimer->Status == OS_SW_TIMER_RUNNING)
    {
        Ret = OS_SW_TIMER_ALREADY_RUNNING;
        goto OS_API_SwTimerStart_Exit;
    }

    SwTimer->WakeupTime = SwTimer->Interval + OS_GetCurrentTime();
    SwTimer->Status = OS_SW_TIMER_RUNNING;

    TRACE_SwTimerStart(SwTimer);

    /* Add a started timer to timer manager */
    OS_AddSwTimerToManager(SwTimer);

OS_API_SwTimerStart_Exit:
    OS_SW_TIMER_UNLOCK();

    return Ret;
}

/* Every tick will check timer timeout */
void OS_SwTimerCheck(OS_Uint32_t CurrentTime)
{
    OS_TCB_t *TaskCB = OS_TSK_HANDLE_TO_TCB(OS_SwTimerTaskHandle);

    /* Check if there are timer on this list */
    if (!ListEmpty(&SwTimerManager.TimerNodesList))
    {
        /* Should wake up timer task ? */
        if (OS_TIME_AFTER_EQ(CurrentTime, SwTimerManager.NextWakeupTime))
        {
            /* Move the Timer task to ready list */
            if (TaskCB->State == OS_TASK_SUSPEND)
            {
                OS_TaskSuspendToReady(TaskCB);
            }
        }
    }
}

void OS_SwTimerTaskEntry(void *Parameter)
{
    OS_Uint32_t CurrentTime = 0;
    OS_SwTimerNode_t *SwTimer = OS_NULL;
    OS_TCB_t *TaskCB = OS_TSK_HANDLE_TO_TCB(OS_SwTimerTaskHandle);

    while (1)
    {
        OS_SW_TIMER_LOCK();

        while (!ListEmpty(&SwTimerManager.TimerNodesList))
        {
            CurrentTime = OS_GetCurrentTime();

            SwTimer = ListFirstEntry(&SwTimerManager.TimerNodesList, OS_SwTimerNode_t, Node);

            if (OS_TIME_AFTER_EQ(CurrentTime, SwTimer->WakeupTime))
            {
                SwTimer->Handler(SwTimer->Param);
                ListDel(&SwTimer->Node);

                if (SwTimer->Mode == OS_SW_TIMER_AUTO_RELOAD)
                {
                    SwTimer->WakeupTime = SwTimer->Interval + CurrentTime;
                    OS_AddSwTimerToManager(SwTimer);
                }
                else
                {
                    SwTimer->Status = OS_SW_TIMER_STOP;
                }
            }
            else
            {
                break;
            }
        }

        if (!ListEmpty(&SwTimerManager.TimerNodesList))
        {
            /* Update for next wake up condition */
            SwTimer = ListFirstEntry(&SwTimerManager.TimerNodesList, OS_SwTimerNode_t, Node);
            OS_UpdateSwTimerNextWakeup(SwTimer->WakeupTime);
        }

        OS_SW_TIMER_UNLOCK();

        /* Suspend my self */
        OS_TaskReadyToSuspend(TaskCB);

        OS_Schedule();
    }
}

OS_Uint32_t OS_API_SwTimerStop(OS_Uint32_t SwTimerHandle)
{
    OS_Uint32_t Ret = OS_SUCCESS;
    OS_SwTimerNode_t *SwTimer = OS_NULL;

    OS_SW_TIMER_CHECK_HANDLE_VALID(SwTimerHandle);
    OS_SW_TIMER_CHECK_BEEN_CREATED(SwTimerHandle);

    OS_SW_TIMER_LOCK();

    SwTimer = OS_SW_TIMER_HANDLE_TO_POINTER(SwTimerHandle);

    if (SwTimer->Status != OS_SW_TIMER_RUNNING)
    {
        Ret = OS_SW_TIMER_NOT_RUNNING;
        goto OS_API_SwTimerStop_Exit;
    }

    TRACE_SwTimerStop(SwTimer);

    SwTimer->Status = OS_SW_TIMER_STOP;

    ListDel(&SwTimer->Node);

OS_API_SwTimerStop_Exit:
    OS_SW_TIMER_UNLOCK();

    return Ret;
}

OS_Uint32_t OS_API_SwTimerDelete(OS_Uint32_t SwTimerHandle)
{
    OS_Uint32_t Ret = OS_SUCCESS;
    OS_SwTimerNode_t *SwTimer = OS_NULL;

    OS_SW_TIMER_CHECK_HANDLE_VALID(SwTimerHandle);
    OS_SW_TIMER_CHECK_BEEN_CREATED(SwTimerHandle);

    OS_SW_TIMER_LOCK();

    SwTimer = OS_SW_TIMER_HANDLE_TO_POINTER(SwTimerHandle);

    if (SwTimer->Status == OS_SW_TIMER_RUNNING)
    {
        ListDel(&SwTimer->Node);
    }

    TRACE_SwTimerDelete(SwTimer);

    SwTimer->Used = OS_SW_TIMER_UNUSED;

    OS_SW_TIMER_UNLOCK();

    return Ret;
}

void OS_SwTimerTaskCreate(void)
{
    TaskInitParameter Param;

    Param.Name[0] = 'S';
    Param.Name[1] = 'W';
    Param.Name[2] = '_';
    Param.Name[3] = 'T';
    Param.Name[4] = 'I';
    Param.Name[5] = 'M';
    Param.Name[6] = 'E';
    Param.Name[7] = 'R';
    Param.Name[8] = 0x00;
    Param.Priority = OS_SW_TIMER_TASK_PRIO;
    Param.PrivateData = OS_NULL;
    Param.StackSize = CONFIG_SW_TMR_TASK_STACK_SIZE;
    Param.TaskEntry = OS_SwTimerTaskEntry;

    OS_API_TaskCreate(Param, (void *)&OS_SwTimerTaskHandle);
}

#endif // CONFIG_USE_SW_TIMER

