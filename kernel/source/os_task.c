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
#include "os_lib.h"
#include "os_mem.h"
#include "os_list.h"
#include "os_time.h"
#include "os_task.h"
#include "os_trace.h"
#include "os_printk.h"
#include "os_configs.h"
#include "os_critical.h"
#include "os_scheduler.h"
#include "os_error_code.h"

#define OS_IDEL_TASK_PRIO               0x00

#define OS_TASK_LOCK()               OS_API_EnterCritical()
#define OS_TASK_UNLOCK()             OS_API_ExitCritical()

OS_TCB_t * volatile CurrentTCB = OS_NULL;
OS_TCB_t * volatile SwitchNextTCB = OS_NULL;
static OS_Uint32_t OS_IdleTaskHandle = 0;

extern void OS_Schedule(void);
extern OS_TCB_t * OS_HighestPrioTaskGet(void);
extern OS_Int16_t OS_IsSchedulerSuspending(void);
extern void OS_RemoveTaskFromReadyList(OS_TCB_t * TaskCB);
extern void OS_AddTaskToReadyList(OS_TCB_t * TaskCB);
extern void OS_TaskReadyToDelay(OS_TCB_t * TaskCB);
extern void OS_TaskUnknowToSuspend(OS_TCB_t * TaskCB);
extern void OS_TaskSuspendToReady(OS_TCB_t * TaskCB);

OS_Uint32_t OS_API_TaskCreate(TaskInitParameter Param, OS_Uint32_t *TaskHandle)
{
    OS_Uint32_t Ret = OS_SUCCESS;
    OS_TCB_t *TaskCB = OS_NULL;

    if (TaskHandle == OS_NULL)
    {
        return OS_NULL_POINTER;
    }

    if (Param.Priority >= OS_MAX_TASK_PRIORITY)
    {
        return OS_TASK_PRIO_OUT_OF_RANGE;
    }

    OS_TASK_LOCK();

    /* allocte memory for task struct */
    TaskCB = (OS_TCB_t *)OS_API_Malloc(sizeof(OS_TCB_t));
    if (TaskCB == OS_NULL)
    {
        Ret = OS_NOT_ENOUGH_MEM_FOR_TASK_CREATE;
        goto TaskCreateMemNotEnough;
    }

    /* allocte memory for task stack */
    TaskCB->Stack = OS_API_Malloc(Param.StackSize);
    if (TaskCB->Stack == OS_NULL)
    {
        OS_API_Free((void *)TaskCB);
        Ret = OS_NOT_ENOUGH_MEM_FOR_TASK_CREATE;
        goto TaskCreateMemNotEnough;
    }

#if CONFIG_STACK_OVERFLOW_CHECK
    TaskCB->StartOfStack = TaskCB->Stack;
#endif

    // Fill the task stack with magic number in order to check stack overflow
    OS_Memset((void *) TaskCB->Stack, OS_TASK_MAGIC_NUMBER, Param.StackSize);

    TaskCB->Priority = Param.Priority;

    OS_Memset((void *) TaskCB->TaskName, 0x00, CONFIG_TASK_NAME_LEN);
    OS_Memcpy((void *) TaskCB->TaskName,(void *)Param.Name, CONFIG_TASK_NAME_LEN);
    // Set the last one to zero to make sure the end of the char *
    TaskCB->TaskName[CONFIG_TASK_NAME_LEN - 1] = 0x00;

    // Prepare the stack for task
    TaskCB->Stack = ARCH_PrepareStack((void *) TaskCB->Stack, (void *)&Param);

    TaskCB->State = OS_TASK_UNKNOWN;
    OS_AddTaskToReadyList(TaskCB);

    TaskCB->WakeUpTime = OS_TIME_MAX;

    *TaskHandle = (OS_Uint32_t)TaskCB;

    TRACE_TaskCreate(TaskCB);

    OS_PRINTK_INFO("Create Task Name:[%s], Priority:[%d]", TaskCB->TaskName, TaskCB->Priority);

    OS_TASK_UNLOCK();

    return OS_SUCCESS;

TaskCreateMemNotEnough:
    OS_TASK_UNLOCK();
    return Ret;
}

/*
 * Analysis Context:
 *-----------------------|--In ISR --------------- [Not Allowed]
 * -- Yield Current  ----|--Scheduler Suspending - [Not Allowed]
 *-----------------------|--In Thread ------------ [Allowed](Schedule right now)
 */
OS_Uint32_t OS_API_TaskYield(void)
{
    OS_Uint32_t Ret = OS_SUCCESS;

    OS_TASK_LOCK();

    if (ARCH_IsInterruptContext())
    {
        Ret = OS_YIELD_IN_INTR_CONTEXT;
        goto OS_API_TaskYield_Exit;
    }

    if (OS_IsSchedulerSuspending())
    {
        Ret = OS_YIELD_IN_SCH_SUSPEND;
        goto OS_API_TaskYield_Exit;
    }

    TRACE_TaskYield(CurrentTCB);

    OS_Schedule();

OS_API_TaskYield_Exit:
    OS_TASK_UNLOCK();

    return Ret;
}

/*
 * Analysis Context:
 *-----------------------|--In ISR --------------- [Not Allowed]
 * -- Delay Current  ----|--Scheduler Suspending - [Not Allowed]
 *-----------------------|--In Thread ------------ [Allowed](Schedule right now)
 */

OS_Uint32_t OS_API_TaskDelay(OS_Uint32_t TickCnt)
{
    OS_Uint32_t Ret = OS_SUCCESS;

    OS_TASK_LOCK();

    if (ARCH_IsInterruptContext())
    {
        Ret = OS_TSK_DLY_IN_INTR_CONTEXT;
        goto OS_API_TaskDelay_Exit;
    }

    if (OS_IsSchedulerSuspending())
    {
        Ret = OS_TSK_DLY_IN_SCH_SUSPEND;
        goto OS_API_TaskDelay_Exit;
    }

    if ( (TickCnt == 0) || (TickCnt>=OS_TSK_DLY_MAX) )
    {
        Ret = OS_TSK_DLY_TICK_INVALID;
        goto OS_API_TaskDelay_Exit;
    }

    CurrentTCB->WakeUpTime = OS_GetCurrentTime() + TickCnt;

    TRACE_TaskDelay(CurrentTCB, TickCnt);

    OS_TaskReadyToDelay(CurrentTCB);

    OS_Schedule();

OS_API_TaskDelay_Exit:
    OS_TASK_UNLOCK();

    return Ret;
}

/*
 * Analysis Context:
 *------------------------------------------------
 * 1. Suspend which already suspending ----------- [Not Allowed]
 *------------------------------------------------
 *
 *-----------------------|--In ISR --------------- [Not Allowed]
 * 2. Suspend Current  --|--Scheduler Suspending - [Not Allowed]
 *-----------------------|--In Thread ------------ [Allowed](Schedule right now)
 *
 *-----------------------|--In ISR --------------- [Allowed]
 * 3. Suspend Other  ----|--Scheduler Suspending - [Allowed]
 *-----------------------|--In Thread -------------[Allowed]
 */
OS_Uint32_t OS_API_TaskSuspend(OS_Uint32_t TaskHandle)
{
    OS_Uint32_t Ret = OS_SUCCESS;
    OS_TCB_t *TaskCB = OS_TSK_HANDLE_TO_TCB(TaskHandle);

    OS_TASK_LOCK();

    if (TaskCB == OS_NULL)
    {
        Ret = OS_NULL_POINTER;
        goto OS_API_TaskSuspend_Exit;
    }

    if (TaskCB->State == OS_TASK_SUSPEND)
    {
        Ret = OS_TSK_SUSPEND_IN_SUSPENDING;
        goto OS_API_TaskSuspend_Exit;
    }

    TRACE_TaskSuspend(TaskCB, CurrentTCB);

    /* Check if want to suspend Current itself */
    if (TaskCB == CurrentTCB)
    {
        /* Suspend Current in interrupt */
        if (ARCH_IsInterruptContext())
        {
            Ret = OS_SUSPEND_CUR_TSK_IN_INTR;
            goto OS_API_TaskSuspend_Exit;
        }
        /* Suspend Current when scheduler suspending */
        if (OS_IsSchedulerSuspending())
        {
            Ret = OS_SUSPEND_CUR_TSK_IN_SCH_SUSPEND;
            goto OS_API_TaskSuspend_Exit;
        }
    }

    OS_TaskUnknowToSuspend(TaskCB);

    /* If suspend current task in thread, then pick next task to run */
    if (TaskCB == CurrentTCB)
    {
        OS_Schedule();
    }

OS_API_TaskSuspend_Exit:
    OS_TASK_UNLOCK();

    return Ret;
}

/*
 * Analysis Context:
 *------------------------------------------------
 * 1. Resume which not in suspending ------------- [Not Allowed]
 *------------------------------------------------
 *
 *-----------------------|--In ISR --------------- [Not Allowed]
 * 2. Resume Current  ---|--Scheduler Suspending - [Not Allowed]
 *-----------------------|--In Thread -------------[Not Allowed]
 *
 *-----------------------|--In ISR --------------- [Allowed]--|
 * 3. Resume Other  -----|--Scheduler Suspending - [Allowed]--|--Check highest priority--
 *-----------------------|--In Thread -------------[Allowed]--|
 */
OS_Uint32_t OS_API_TaskResume(OS_Uint32_t TaskHandle)
{
    OS_Uint32_t Ret = OS_SUCCESS;
    OS_TCB_t *TaskCB = OS_TSK_HANDLE_TO_TCB(TaskHandle);

    OS_TASK_LOCK();

    if (TaskCB == OS_NULL)
    {
        Ret = OS_NULL_POINTER;
        goto OS_API_TaskResume_Exit;
    }

    if (TaskCB == CurrentTCB)
    {
        Ret =  OS_RESUME_CUR_TSK;
        goto OS_API_TaskResume_Exit;
    }

    if (TaskCB->State != OS_TASK_SUSPEND)
    {
        Ret = OS_RESUME_TSK_NOT_IN_SUSPEND;
        goto OS_API_TaskResume_Exit;
    }

    TRACE_TaskResume(TaskCB);

    /* Move task from suspend to ready list */
    OS_TaskSuspendToReady(TaskCB);

    /* If resumed task's priority higher than current task */
    if (TaskCB->Priority >= CurrentTCB->Priority)
    {
        OS_Schedule();
    }

OS_API_TaskResume_Exit:
    OS_TASK_UNLOCK();

    return Ret;
}

/*
 * Analysis Context:
 *------------------|--In ISR --------- [Allowed]-|-higher?
 * 1. Set Current  -|--Sch Suspending - [Allowed]-|
 *------------------|--In Thread -------[Allowed]-|-lower?---[re-schedule]
 *
 *------------------|--In ISR ----------[Allowed]-|-higher and also higher than current ?---[re-schedule]
 * 2. Set In Other -|--Sch Suspending - [Allowed]-|-
 *------------------|--In Thread -------[Allowed]-|-lower?
 *
 *------------------|
 * 3. Task In Ready-|- Update Ready list
 *------------------|
 */
OS_Uint32_t OS_API_TaskPrioritySet(OS_Uint32_t TaskHandle, OS_Uint8_t NewPriority)
{
    OS_Uint32_t Ret = OS_SUCCESS;
    OS_Uint8_t  NeedReSch = 0;
    OS_TCB_t *TaskCB = OS_TSK_HANDLE_TO_TCB(TaskHandle);

    if (TaskCB == OS_NULL)
    {
        return OS_NULL_POINTER;
    }

    if (NewPriority == TaskCB->Priority)
    {
        return OS_SET_SAME_PRIO;
    }

    if (NewPriority >= OS_MAX_TASK_PRIORITY)
    {
        return OS_TASK_PRIO_OUT_OF_RANGE;
    }

    OS_TASK_LOCK();

    TARCE_TaskPrioritySet(TaskCB, CurrentTCB, NewPriority);

    /* If set current priority */
    if (TaskCB == CurrentTCB)
    {
        /* Set current priority smaller than before */
        if (NewPriority < CurrentTCB->Priority)
        {
            NeedReSch = 1;
        }
    }
    /* Set other task priority */
    else
    {
        /*
         * Re-Schedule condition:
         * 1. Priority adjusted higher than before
         * 2. After priority adjusted, also higher than current
         * 3. This task should in ready list
         */
        if ( (NewPriority > TaskCB->Priority    ) &&
             (NewPriority > CurrentTCB->Priority) &&
             (TaskCB->State == OS_TASK_READY    ) )
        {
            NeedReSch = 1;
        }
    }

    /* Because the priority of ready task stand head list, so update it */
    if (TaskCB->State == OS_TASK_READY)
    {
        OS_RemoveTaskFromReadyList(TaskCB);
        /* Update priority */
        TaskCB->Priority = NewPriority;
        OS_AddTaskToReadyList(TaskCB);
    }
    else
    {
        /* Update priority */
        TaskCB->Priority = NewPriority;
    }

    if (NeedReSch)
    {
        OS_Schedule();
    }

    OS_TASK_UNLOCK();

    return Ret;
}

OS_Uint8_t OS_API_TaskPriorityGet(OS_Uint32_t TaskHandle)
{
    OS_TCB_t *TaskCB = OS_TSK_HANDLE_TO_TCB(TaskHandle);
    OS_Uint8_t Ret = 0;

    OS_TASK_LOCK();
    Ret = TaskCB->Priority;
    OS_TASK_UNLOCK();

    return Ret;
}

void OS_IdleTask(void *Parameter)
{
    while (1);
}

void OS_IdleTaskCreate(void)
{
    TaskInitParameter Param;
    Param.Name[0] = 'I';
    Param.Name[1] = 'D';
    Param.Name[2] = 'L';
    Param.Name[3] = 'E';
    Param.Name[4] = 0x00;
    Param.Priority = OS_IDEL_TASK_PRIO;
    Param.PrivateData = OS_NULL;
    Param.StackSize = CONFIG_IDLE_TASK_STACK_SIZE;
    Param.TaskEntry = OS_IdleTask;
    OS_API_TaskCreate(Param, (void *)&OS_IdleTaskHandle);
}

void OS_FirstTaskStartup(void)
{
    /* Find the highest priority task in ready list */
    CurrentTCB = OS_HighestPrioTaskGet();

    /* Start scheduler */
    ARCH_StartScheduler((void *)CurrentTCB);
}
