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
#include "os_mutex.h"
#include "os_trace.h"
#include "os_printk.h"
#include "os_configs.h"
#include "os_critical.h"
#include "os_scheduler.h"
#include "os_error_code.h"

#if CONFIG_USE_MUTEX

#define OS_MUTEX_MAX_COUNT                              1

#define OS_MUTEX_LOCK()                                 OS_API_EnterCritical()
#define OS_MUTEX_UNLOCK()                               OS_API_ExitCritical()

OS_Mutex_t OS_MutexPool[CONFIG_MAX_MUTEX_DEFINE];

#define OS_MUTEX_CHECK_HANDLE_VALID(HANDLE)             \
{                                                       \
    if (HANDLE >= CONFIG_MAX_MUTEX_DEFINE)                  \
    {                                                   \
        return OS_MUTEX_HANDLE_INVALID;                 \
    }                                                   \
}

#define OS_MUTEX_CHECK_BEEN_CREATED(HANDLE)             \
{                                                       \
    if (OS_MutexPool[HANDLE].Used == OS_MUTEX_UNUSED)   \
    {                                                   \
        return OS_MUTEX_NOT_BEEN_CREATED;               \
    }                                                   \
}

#define OS_MUTEX_HANDLE_TO_POINTER(HANDLE)              &OS_MutexPool[HANDLE]

extern OS_TCB_t * volatile CurrentTCB;

extern void OS_TaskReadyToBlock(OS_TCB_t * TaskCB, ListHead_t *SleepHead, OS_Uint8_t BlockType, OS_Uint8_t SortType);
extern OS_Int16_t OS_IsSchedulerSuspending(void);
extern void OS_Schedule(void);
extern void OS_TaskBlockToReady(OS_TCB_t * TaskCB);
extern void OS_TaskChangePriority(OS_TCB_t * TaskCB, OS_Uint8_t NewPriority);

void OS_MutexInit(void)
{
    OS_Uint32_t i = 0;

    for (i = 0; i < CONFIG_MAX_MUTEX_DEFINE; i++)
    {
        OS_MutexPool[i].Used = OS_MUTEX_UNUSED;
        OS_MutexPool[i].Owner = OS_NULL;
        OS_MutexPool[i].OwnerHoldCount = 0;
        OS_MutexPool[i].OwnerPriority = 0;
        ListHeadInit(&OS_MutexPool[i].SleepList);
    }
}

OS_Uint32_t OS_GetMutexResource(OS_Uint32_t *MutexHandle)
{
    OS_Uint32_t i = 0;

    for (i = 0; i < CONFIG_MAX_MUTEX_DEFINE; i++)
    {
        if (OS_MutexPool[i].Used == OS_MUTEX_UNUSED)
            break;
    }

    if (i == CONFIG_MAX_MUTEX_DEFINE)
    {
        return OS_NOT_ENOUGH_MUTEX_RESOURCE;
    }
    else
    {
        *MutexHandle = i;
    }

    return OS_SUCCESS;
}

OS_Uint32_t OS_API_MutexCreate(OS_Uint32_t *MutexHandle)
{
    OS_Uint32_t Ret = OS_SUCCESS;

    OS_CHECK_NULL_POINTER(MutexHandle);

    OS_MUTEX_LOCK();

    Ret = OS_GetMutexResource(MutexHandle);
    if (Ret != OS_SUCCESS)
        goto OS_API_MutexCreate_Exit;

    OS_MutexPool[*MutexHandle].Used = OS_MUTEX_USED;
    OS_MutexPool[*MutexHandle].Owner = OS_NULL;
    OS_MutexPool[*MutexHandle].OwnerHoldCount = 0;
    OS_MutexPool[*MutexHandle].OwnerPriority = 0;
    ListHeadInit(&OS_MutexPool[*MutexHandle].SleepList);

    TARCE_MutexCreate(MutexHandle);

OS_API_MutexCreate_Exit:
    OS_MUTEX_UNLOCK();

    return Ret;
}

void OS_MutexSleep(OS_TCB_t *TaskCB, OS_Mutex_t *Mutex, OS_Uint8_t BlockType)
{
    TARCE_MutexSleep(TaskCB, Mutex, BlockType);

    /* Check to priority to prevent task priority inversion */
    if (TaskCB->Priority > Mutex->Owner->Priority)
    {
        /* Increase the owner task priority temporarily */
        OS_TaskChangePriority(Mutex->Owner, TaskCB->Priority);
    }

    // Insert to the sleep list with priority sort
    OS_TaskReadyToBlock(TaskCB, &Mutex->SleepList, BlockType, OS_BLOCK_SORT_TASK_PRIO);
}

OS_Uint32_t OS_MutexLock(OS_Uint32_t MutexHandle,
                         OS_Uint8_t  BlockType,
                         OS_Uint32_t Timeout)
{
    OS_Uint32_t Ret = OS_SUCCESS;
    OS_Mutex_t *Mutex = OS_NULL;
    OS_TCB_t *TaskCB = CurrentTCB;

    OS_MUTEX_CHECK_HANDLE_VALID(MutexHandle);
    OS_MUTEX_CHECK_BEEN_CREATED(MutexHandle);

    OS_MUTEX_LOCK();

    if (ARCH_IsInterruptContext())
    {
        Ret = OS_USE_MUTEX_IN_INTR_CONTEXT;
        OS_PRINTK_ERROR("MutexLock In ISR");
        goto OS_MutexLock_Exit;
    }

    if (OS_IsSchedulerSuspending())
    {
        Ret = OS_USE_MUTEX_IN_SCH_SUSPEND;
        OS_PRINTK_ERROR("MutexLock in scheduler suspend");
        goto OS_MutexLock_Exit;
    }

    Mutex = OS_MUTEX_HANDLE_TO_POINTER(MutexHandle);

    /* If no task get this lock before */
    if (Mutex->OwnerHoldCount == 0)
    {
        /* Record mutex owner */
        Mutex->Owner = TaskCB;
        /* Record mutex lock owner priority */
        Mutex->OwnerPriority = TaskCB->Priority;
        /* Increment the hold count */
        Mutex->OwnerHoldCount++;

        TARCE_MutexLock(TaskCB);

        /* return OK */
        goto OS_MutexLock_Exit;
    }

    /* The mutexlock have been got, check if the owner is current */
    if (Mutex->Owner == TaskCB)
    {
        /* Support owner nesting lock */
        Mutex->OwnerHoldCount++;
        goto OS_MutexLock_Exit;
    }

    /* Other task require this mutexlock */
    if (Timeout == 0)
    {
        /* Means try mutex lock failed */
        Ret = OS_TRY_MUTEX_LOCK_FAILED;
        goto OS_MutexLock_Exit;
    }

    /* Get wake up timestamp if using timeout strategy */
    TaskCB->WakeUpTime = OS_GetCurrentTime() + Timeout;

    /* Before sleep, clear the wake up flag */
    TaskCB->IpcTimeoutWakeup = OS_IPC_NO_TIMEOUT;

    /* Here means other task require this mutexlock, and will in sleep */
    OS_MutexSleep(TaskCB, Mutex, BlockType);

    OS_Schedule();

    OS_MUTEX_UNLOCK();
    OS_MUTEX_LOCK();

    /* Wake up here */
    if (TaskCB->IpcTimeoutWakeup == OS_IPC_WAIT_TIMEOUT)
    {
        Ret = OS_MUTEX_WAIT_TIMEOUT;
    }

OS_MutexLock_Exit:
    OS_MUTEX_UNLOCK();

    return Ret;
}

OS_Uint32_t OS_API_MutexLock(OS_Uint32_t MutexHandle)
{
    return OS_MutexLock(MutexHandle, OS_BLOCK_TYPE_ENDLESS, 0xFF);
}

OS_Uint32_t OS_API_MutexLockTimeout(OS_Uint32_t MutexHandle, OS_Uint32_t Timeout)
{
    if (Timeout >= OS_TSK_DLY_MAX)
    {
        return OS_MUTEX_INVALID_TIMEOUT;
    }

    return OS_MutexLock(MutexHandle, OS_BLOCK_TYPE_TIMEOUT, Timeout);
}

OS_Uint32_t OS_API_MutexTryLock(OS_Uint32_t MutexHandle)
{
    return OS_MutexLock(MutexHandle, OS_BLOCK_TYPE_TIMEOUT, 0x00);
}

static OS_Uint8_t OS_MutexWakeup(OS_TCB_t *TaskCB, OS_Mutex_t *Mutex)
{
    OS_Uint8_t NeedResch = 0;
    OS_TCB_t *WakeupTaskCB = OS_NULL;

    /* Check if the owner priority have been modified, just set it back */
    if (TaskCB->Priority != Mutex->OwnerPriority)
    {
        OS_TaskChangePriority(Mutex->Owner, Mutex->OwnerPriority);
    }

    /* If there are tasks blocking on this mutex lock */
    if (!ListEmpty(&Mutex->SleepList))
    {
        /* Pick the next highset blocked task to wakeup */
        WakeupTaskCB = ListEntry(Mutex->SleepList.next, OS_TCB_t, IpcSleepList);
        OS_TaskBlockToReady(WakeupTaskCB);
        /* Update the mutex lock owner */
        Mutex->Owner = WakeupTaskCB;
        Mutex->OwnerHoldCount = 1;
        Mutex->OwnerPriority = WakeupTaskCB->Priority;

        TARCE_MutexWakeup(WakeupTaskCB, Mutex);

        NeedResch = 1;
    }
    else
    {
        /* No other task blocked on this mutex lock */
        Mutex->Owner = OS_NULL;
        Mutex->OwnerPriority = 0;
    }

    return NeedResch;
}

OS_Uint32_t OS_API_MutexUnlock(OS_Uint32_t MutexHandle)
{
    OS_Uint32_t Ret = OS_SUCCESS;
    OS_Mutex_t *Mutex = OS_NULL;
    OS_TCB_t *TaskCB = CurrentTCB;
    OS_Uint8_t NeedResch = 0;

    OS_MUTEX_CHECK_HANDLE_VALID(MutexHandle);
    OS_MUTEX_CHECK_BEEN_CREATED(MutexHandle);

    OS_MUTEX_LOCK();

    if (ARCH_IsInterruptContext())
    {
        Ret = OS_USE_MUTEX_IN_INTR_CONTEXT;
        goto OS_API_MutexUnlock_Exit;
    }

    if (OS_IsSchedulerSuspending())
    {
        Ret = OS_USE_MUTEX_IN_SCH_SUSPEND;
        goto OS_API_MutexUnlock_Exit;
    }

    Mutex = OS_MUTEX_HANDLE_TO_POINTER(MutexHandle);

    /* Unlock without lock */
    if (Mutex->OwnerHoldCount == 0)
    {
        Ret = OS_MUTEX_UNLOCK_INVALID;
        OS_PRINTK_ERROR("MutexUnlock without lock");
        goto OS_API_MutexUnlock_Exit;
    }

    /* Unlock by other task */
    if (Mutex->Owner != TaskCB)
    {
        Ret = OS_MUTEX_UNLOCK_NOT_OWNER;
        OS_PRINTK_ERROR("MutexUnlock Owner error");
        goto OS_API_MutexUnlock_Exit;
    }

    Mutex->OwnerHoldCount--;

    /* Here should be nesting call by owner */
    if (Mutex->OwnerHoldCount != 0)
    {
        Ret = OS_SUCCESS;
        goto OS_API_MutexUnlock_Exit;
    }

    TARCE_MutexUnLock(TaskCB);

    /* Here should release mutex lock, and wake up other task */
    NeedResch = OS_MutexWakeup(TaskCB, Mutex);

    if (NeedResch)
    {
        OS_Schedule();
    }

OS_API_MutexUnlock_Exit:
    OS_MUTEX_UNLOCK();

    return Ret;
}

OS_Uint32_t OS_API_MutexDestory(OS_Uint32_t MutexHandle)
{
    OS_Uint32_t Ret = OS_SUCCESS;
    OS_Mutex_t *Mutex = OS_NULL;

    OS_MUTEX_CHECK_HANDLE_VALID(MutexHandle);
    OS_MUTEX_CHECK_BEEN_CREATED(MutexHandle);

    OS_MUTEX_LOCK();

    Mutex = OS_MUTEX_HANDLE_TO_POINTER(MutexHandle);

    if (!ListEmpty(&Mutex->SleepList))
    {
        Ret = OS_MUTEX_DESTORY_IN_NO_EMPTY;
        goto OS_API_MutexDestory_Exit;
    }

    if (Mutex->OwnerHoldCount != 0)
    {
        Ret = OS_MUTEX_DESTORY_IN_OWNER_USING;
        goto OS_API_MutexDestory_Exit;
    }

    Mutex->Owner = OS_NULL;
    Mutex->OwnerPriority = 0;
    Mutex->Used = OS_MUTEX_UNUSED;

OS_API_MutexDestory_Exit:
    OS_MUTEX_UNLOCK();

    return Ret;
}

#endif // CONFIG_USE_MUTEX
