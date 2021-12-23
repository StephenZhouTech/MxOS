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
#include "os_sem.h"
#include "os_task.h"
#include "os_time.h"
#include "os_list.h"
#include "os_trace.h"
#include "os_printk.h"
#include "os_critical.h"
#include "os_configs.h"
#include "os_scheduler.h"
#include "os_error_code.h"

#define OS_SEM_MAX_COUNT                            0xFFFE
#define OS_BINARY_SEM_MAX_COUNT                     1

#define OS_SEM_LOCK()                               OS_API_EnterCritical()
#define OS_SEM_UNLOCK()                             OS_API_ExitCritical()

OS_Sem_t OS_SemPool[OS_MAX_SEM_DEFINE];

#define OS_SEM_CHECK_HANDLE_VALID(HANDLE)           \
{                                                   \
    if (HANDLE >= OS_MAX_SEM_DEFINE)                \
    {                                               \
        return OS_SEM_HANDLE_INVALID;               \
    }                                               \
}

#define OS_SEM_CHECK_BEEN_CREATED(HANDLE)           \
{                                                   \
    if (OS_SemPool[HANDLE].Used == OS_SEM_UNUSED)   \
    {                                               \
        return OS_SEM_NOT_BEEN_CREATED;             \
    }                                               \
}

#define OS_SEM_HANDLE_TO_POINTER(HANDLE)            &OS_SemPool[HANDLE]

extern OS_TCB_t * volatile CurrentTCB;

extern void OS_TaskReadyToBlock(OS_TCB_t * TaskCB, ListHead_t *SleepHead, OS_Uint8_t BlockType, OS_Uint8_t SortType);
extern OS_Int16_t OS_IsSchedulerSuspending(void);
extern void OS_Schedule(void);
extern void OS_TaskBlockToReady(OS_TCB_t * TaskCB);

void OS_SemaphoreInit(void)
{
    OS_Uint32_t i = 0;

    for (i = 0; i < OS_MAX_SEM_DEFINE; i++)
    {
        OS_SemPool[i].Count = 0;
        OS_SemPool[i].Used = OS_SEM_UNUSED;
        ListHeadInit(&OS_SemPool[i].List);
    }
}

OS_Uint32_t OS_GetSemResource(OS_Uint32_t *SemHandle)
{
    OS_Uint32_t i = 0;

    for (i = 0; i < OS_MAX_SEM_DEFINE; i++)
    {
        if (OS_SemPool[i].Used == OS_SEM_UNUSED)
            break;
    }

    if (i == OS_MAX_SEM_DEFINE)
    {
        return OS_NOT_ENOUGH_SEM_RESOURCE;
    }
    else
    {
        *SemHandle = i;
    }

    return OS_SUCCESS;
}

static OS_Uint32_t OS_SemCreate(OS_Uint32_t *SemHandle, OS_Uint32_t Count)
{
    OS_Uint32_t Ret = OS_SUCCESS;

    if (SemHandle == OS_NULL)
    {
        return OS_NULL_POINTER;
    }

    OS_SEM_LOCK();

    Ret = OS_GetSemResource(SemHandle);
    if (Ret != OS_SUCCESS)
        goto OS_SemCreate_Exit;

    OS_SemPool[*SemHandle].Used = OS_SEM_USED;
    OS_SemPool[*SemHandle].Count = Count;
    ListHeadInit(&OS_SemPool[*SemHandle].List);

    TARCE_SemCreate(SemHandle, Count);

OS_SemCreate_Exit:
    OS_SEM_UNLOCK();

    return Ret;
}

OS_Uint32_t OS_API_SemCreate(OS_Uint32_t *SemHandle, OS_Uint32_t Count)
{
    if (Count > OS_SEM_MAX_COUNT)
        return OS_SEM_OVERFLOW;

    return OS_SemCreate(SemHandle, Count);
}

OS_Uint32_t OS_API_BinarySemCreate(OS_Uint32_t *SemHandle, OS_Uint32_t Count)
{
    if (Count > OS_BINARY_SEM_MAX_COUNT)
        return OS_SEM_OVERFLOW;

    return OS_SemCreate(SemHandle, Count);
}

static OS_Uint32_t OS_SemWait(OS_Uint32_t SemHandle, OS_Uint8_t BlockType,
                              OS_Uint32_t Timeout)
{
    OS_Uint32_t Ret = OS_SUCCESS;
    OS_Sem_t *Sem = OS_NULL;
    OS_TCB_t *TaskCB = CurrentTCB;

    OS_SEM_CHECK_HANDLE_VALID(SemHandle);
    OS_SEM_CHECK_BEEN_CREATED(SemHandle);

    OS_SEM_LOCK();

    Sem = OS_SEM_HANDLE_TO_POINTER(SemHandle);

    if (ARCH_IsInterruptContext())
    {
        Ret = OS_SEM_WAIT_IN_INTR_CONTEXT;
        goto OS_SemWait_Exit;
    }

    if (OS_IsSchedulerSuspending())
    {
        Ret = OS_SEM_WAIT_IN_SCH_SUSPEND;
        goto OS_SemWait_Exit;
    }

    TARCE_SemWaitEntry(Sem);

    if (Sem->Count > 0)
    {
        Sem->Count--;
        goto OS_SemWait_Exit;
    }

    if (Timeout == 0)
    {
        Ret = OS_SEM_TRY_WAIT_FAILED;
        goto OS_SemWait_Exit;
    }

    /* Get wake up timestamp if using timeout strategy */
    TaskCB->WakeUpTime = OS_GetCurrentTime() + Timeout;

    /* Before sleep, clear the wake up flag */
    TaskCB->IpcTimeoutWakeup = OS_IPC_NO_TIMEOUT;

    TARCE_SemWaitSleep(TaskCB, Sem, BlockType);

    OS_TaskReadyToBlock(TaskCB, &Sem->List, BlockType, OS_BLOCK_SORT_FIFO);

    OS_Schedule();

    OS_SEM_UNLOCK();
    OS_SEM_LOCK();

    /* Wake up here */
    if (TaskCB->IpcTimeoutWakeup == OS_IPC_WAIT_TIMEOUT)
    {
        Ret = OS_SEM_WAIT_TIMEOUT;
    }

OS_SemWait_Exit:
    OS_SEM_UNLOCK();

    return Ret;
}

OS_Uint32_t OS_API_SemWait(OS_Uint32_t SemHandle)
{
    return OS_SemWait(SemHandle, OS_BLOCK_TYPE_ENDLESS, 0xFF);
}

OS_Uint32_t OS_API_BinarySemWait(OS_Uint32_t SemHandle)
{
    return OS_API_SemWait(SemHandle);
}

OS_Uint32_t OS_API_SemWaitTimeout(OS_Uint32_t SemHandle, OS_Uint32_t Timeout)
{
    if (Timeout >= OS_TSK_DLY_MAX)
    {
        return OS_SEM_INVALID_TIMEOUT;
    }

    return OS_SemWait(SemHandle, OS_BLOCK_TYPE_TIMEOUT, Timeout);
}

OS_Uint32_t OS_API_BinarySemWaitTimeout(OS_Uint32_t SemHandle, OS_Uint32_t Timeout)
{
    return OS_API_SemWaitTimeout(SemHandle, Timeout);
}

OS_Uint32_t OS_API_SemTryWait(OS_Uint32_t SemHandle)
{
    return OS_SemWait(SemHandle, OS_BLOCK_TYPE_TIMEOUT, 0x00);
}

OS_Uint32_t OS_API_BinarySemTryWait(OS_Uint32_t SemHandle)
{
    return OS_API_SemTryWait(SemHandle);
}

static OS_Uint32_t OS_SemPost(OS_Uint32_t SemHandle, OS_Uint32_t MaxCount)
{
    OS_Uint32_t Ret = OS_SUCCESS;
    OS_Sem_t *Sem = OS_NULL;
    ListHead_t *IpcSleepList = OS_NULL;
    OS_TCB_t *TaskCB = OS_NULL;

    OS_SEM_CHECK_HANDLE_VALID(SemHandle);
    OS_SEM_CHECK_BEEN_CREATED(SemHandle);

    OS_SEM_LOCK();

    Sem = OS_SEM_HANDLE_TO_POINTER(SemHandle);

    if (Sem->Count >= MaxCount)
    {
        Ret = OS_SEM_OVERFLOW;
        goto OS_API_SemPost_Exit;
    }

    if (!ListEmpty(&Sem->List))
    {
        // Pick the last one of sleep list because we use FIFO algorithm
        IpcSleepList = PickListLast(&Sem->List);
        TaskCB = ListEntry(IpcSleepList, OS_TCB_t, IpcSleepList);

        TARCE_SemWakeup(TaskCB);

        OS_TaskBlockToReady(TaskCB);

        OS_Schedule();
    }
    else
    {
        Sem->Count++;
    }

OS_API_SemPost_Exit:
    OS_SEM_UNLOCK();

    return Ret;
}

OS_Uint32_t OS_API_SemPost(OS_Uint32_t SemHandle)
{
    return OS_SemPost(SemHandle, OS_SEM_MAX_COUNT);
}

OS_Uint32_t OS_API_BinarySemPost(OS_Uint32_t SemHandle)
{
    return OS_SemPost(SemHandle, OS_BINARY_SEM_MAX_COUNT);
}

OS_Uint32_t OS_API_SemDestory(OS_Uint32_t SemHandle)
{
    OS_Uint32_t Ret = OS_SUCCESS;
    OS_Sem_t    *Sem = OS_NULL;
    ListHead_t  *ListIterator  = OS_NULL;
    OS_TCB_t    *TCB_Iterator  = OS_NULL;

    OS_SEM_CHECK_HANDLE_VALID(SemHandle);
    OS_SEM_CHECK_BEEN_CREATED(SemHandle);

    OS_SEM_LOCK();

    Sem = OS_SEM_HANDLE_TO_POINTER(SemHandle);

    // Wake up all of the task
    while (!ListEmpty(&Sem->List))
    {
        ListIterator = Sem->List.next;
        TCB_Iterator = ListEntry(ListIterator, OS_TCB_t, IpcSleepList);

        TARCE_SemDestory(TCB_Iterator);

        OS_TaskBlockToReady(TCB_Iterator);
    }

    Sem->Count = 0;
    Sem->Used = OS_SEM_UNUSED;

    // Pick all the task which block on this sem
    OS_Schedule();

    OS_SEM_UNLOCK();

    return Ret;
}
