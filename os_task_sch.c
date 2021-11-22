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

#define OS_TASK_MAGIC_NUMBER            0xA5
#define OS_IDEL_TASK_PRIO               0x00

typedef enum _OS_ScheduerState {
    SCHEDULER_SUSPEND = 0,
    SCHEDULER_RUNNING
} OS_ScheduerState_e;

typedef struct _OS_TaskControlBlock {
    ListHead_t      StateList;
    OS_Int8_t       TaskName[CONFIG_TASK_NAME_LEN];
    OS_Uint8_t      Priority;
    void            *Stack;
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

    ARCH_PrepareStack((void *) TaskCB->Stack, (void *)&Param);

    ListAdd(&TaskCB->StateList, &Scheduler.ReadyListHead[TaskCB->Priority]);

    SetPriorityActive(TaskCB->Priority);

    *TaskHandle = (OS_Uint32_t)TaskCB;

    return OS_SUCCESS;
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

void OS_API_SchedulerInit(void)
{
    /* Initial the ReadList */
    OS_Uint32_t i = 0;
    for (i = 0; i < OS_MAX_TASK_PRIORITY; i++)
    {
        ListHeadInit(&Scheduler.ReadyListHead[i]);
    }
    ListHeadInit(&Scheduler.BlockListHead);
    ListHeadInit(&Scheduler.SuspendListHead);
    Scheduler.CurrentTime = CONFIG_TICK_COUNT_INIT_VALUE;
    Scheduler.ScheduerState = SCHEDULER_SUSPEND;
}

void OS_API_StartKernel(void)
{
    OS_IdleTaskCreate();

    /* Close IRQ */
    ARCH_DisableIRQ();

    /* Configure the IRQ just like NVIC priority */
    ARCH_IRQInit();

    /* Configure System Tick for OS heart beat */
    ARCH_SystemTickInit(CONFIG_SYS_TICK_RATE_HZ);

    /* Set the scheduler state to running */
    Scheduler.ScheduerState = SCHEDULER_RUNNING;
    /* Enable IRQ */
    ARCH_StartScheduler();
}
