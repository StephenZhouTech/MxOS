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
#include "os_lib.h"
#include "os_mem.h"
#include "arch.h"
#include "os_task_sch.h"
#include "os_error_code.h"
#include "os_configs.h"

#define OS_TASK_MAGIC_NUMBER            0xA5
#define OS_IDEL_TASK_PRIO               0x00

typedef enum _OS_ScheduerState {
    SCHEDULER_SUSPEND = 0,
    SCHEDULER_RUNNING
} OS_ScheduerState_e;

typedef struct _OS_TaskControlBlock {
    void            *Stack;
    OS_Uint8_t      Priority;
    ListHead_t      StateList;
    OS_Int8_t       TaskName[CONFIG_TASK_NAME_LEN];
} OS_TCB_t;

typedef struct _OS_TaskScheduler {
    ListHead_t      ReadyListHead[OS_MAX_TASK_PRIORITY];
    ListHead_t      BlockListHead;
    ListHead_t      SuspendListHead;
    OS_Uint8_t      PriorityActive;
    OS_Uint32_t     CurrentTime;
    OS_Uint8_t      ScheduerState;
} OS_TaskScheduler_t;

static OS_TCB_t * volatile CurrentTCB = OS_NULL;
static OS_TaskScheduler_t Scheduler;
static OS_Uint32_t OS_IdleTaskHandle = 0;

void SetPriorityActive(OS_Uint8_t ActiveBit)
{
    Scheduler.PriorityActive |= (0x01 << ActiveBit);
}

void ClearPriorityActive(OS_Uint8_t ActiveBit)
{
    Scheduler.PriorityActive &= (~(0x01 << ActiveBit));
}

OS_Uint32_t OS_API_TaskCreate(TaskInitParameter Param, OS_Uint32_t *TaskHandle)
{
    OS_TCB_t *TaskCB = OS_NULL;

    if (TaskHandle == OS_NULL)
    {
        return OS_NULL_POINTER;
    }

    if (Param.Priority >= OS_MAX_TASK_PRIORITY)
    {
        return OS_TASK_PRIO_OUT_OF_RANGE;
    }

    /* allocte memory for task struct */
    TaskCB = (OS_TCB_t *)OS_API_Malloc(sizeof(OS_TCB_t));
    if (TaskCB == OS_NULL)
    {
        return OS_NOT_ENOUGH_MEM_FOR_TASK_CREATE;
    }

    /* allocte memory for task stack */
    TaskCB->Stack = OS_API_Malloc(Param.StackSize);
    if (TaskCB->Stack == OS_NULL)
    {
        OS_API_Free((void *)TaskCB);
        return OS_NOT_ENOUGH_MEM_FOR_TASK_CREATE;
    }
    // Fill the task stack with magic number in order to check stack overflow
    OS_Memset((void *) TaskCB->Stack, OS_TASK_MAGIC_NUMBER, Param.StackSize);

    TaskCB->Priority = Param.Priority;

    OS_Memset((void *) TaskCB->TaskName, 0x00, CONFIG_TASK_NAME_LEN);
    OS_Memcpy((void *) TaskCB->TaskName,(void *)Param.Name, CONFIG_TASK_NAME_LEN);
    // Set the last one to zero to make sure the end of the char *
    TaskCB->TaskName[CONFIG_TASK_NAME_LEN - 1] = 0x00;

    // Prepare the stack for task
    TaskCB->Stack = ARCH_PrepareStack((void *) TaskCB->Stack, (void *)&Param);

    ListAdd(&TaskCB->StateList, &Scheduler.ReadyListHead[TaskCB->Priority]);

    SetPriorityActive(TaskCB->Priority);

    *TaskHandle = (OS_Uint32_t)TaskCB;

    return OS_SUCCESS;
}

void OS_IdleTask(void *Parameter)
{
    /*
     *******************************************************************
     * Because we configure and enabled the systick earlier
     * Once we enable global irq, maybe the systick handler will be called
     * Now we using PSP and run as thread mode
     * So, the highest priority task will be execute in next context switch
     ********************************************************************
     */
    ARCH_InterruptEnable();
    ARCH_ChangeToUserMode();
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

void OS_API_KernelInit(void)
{
    OS_Uint32_t i = 0;

    /* Initial the memory manager */
    OS_MemInit();

    /* Initial the task ReadyList */
    for (i = 0; i < OS_MAX_TASK_PRIORITY; i++)
    {
        ListHeadInit(&Scheduler.ReadyListHead[i]);
    }
    ListHeadInit(&Scheduler.BlockListHead);
    ListHeadInit(&Scheduler.SuspendListHead);
    Scheduler.CurrentTime = CONFIG_TICK_COUNT_INIT_VALUE;
    Scheduler.ScheduerState = SCHEDULER_SUSPEND;
}

OS_TCB_t * OS_TargetTaskSearch(void)
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

void OS_API_KernelStart(void)
{
    /* Close IRQ */
    ARCH_InterruptDisable();

    /* Set the scheduler state to running */
    Scheduler.ScheduerState = SCHEDULER_RUNNING;

    /* Create Idle task */
    OS_IdleTaskCreate();

    /* Configure the IRQ just like NVIC priority */
    ARCH_InterruptInit();

    /* Configure the misc like FPU feature */
    ARCH_MiscInit();

    /* Configure System Tick for OS heart beat */
    ARCH_SystemTickInit();

    /* The first task will be execute is Idle task */
    CurrentTCB = (OS_TCB_t *)OS_IdleTaskHandle;

    /* Start scheduler */
    ARCH_StartScheduler((void *)OS_IdleTaskHandle);
}

void OS_TickTimeWrapHandle(void)
{
    // TODO : Handle time wraps back
}

void OS_SystemTickHander(void)
{
    OS_Uint8_t NeedSwitchCtx = 0;
    OS_TCB_t * NextTCB = OS_NULL;

    ARCH_InterruptDisable();

    Scheduler.CurrentTime++;
    if (Scheduler.CurrentTime == 0)
    {
        OS_TickTimeWrapHandle();
    }

    NextTCB = OS_TargetTaskSearch();

    if (NextTCB->Priority >= CurrentTCB->Priority)
    {
        NeedSwitchCtx = 1;
    }

    if (NextTCB->Priority == CurrentTCB->Priority)
    {
        ListMoveTail(&CurrentTCB->StateList, &Scheduler.ReadyListHead[CurrentTCB->Priority]);
    }

    if (NeedSwitchCtx)
    {
        ARCH_TriggerContextSwitch((void *)CurrentTCB, (void *)NextTCB);
    }

    ARCH_InterruptEnable();
}
