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
#include "os_mem.h"
#include "os_lib.h"
#include "os_time.h"
#include "os_list.h"
#include "os_queue.h"
#include "os_trace.h"
#include "os_printk.h"
#include "os_configs.h"
#include "os_critical.h"
#include "os_scheduler.h"
#include "os_error_code.h"

#if CONFIG_USE_QUEUE

#define OS_QUEUE_LOCK()                                 OS_API_EnterCritical()
#define OS_QUEUE_UNLOCK()                               OS_API_ExitCritical()

OS_Queue_t OS_QueuePool[CONFIG_MAX_QUEUE_DEFINE];

#define OS_QUEUE_CHECK_HANDLE_VALID(HANDLE)             \
{                                                       \
    if (HANDLE >= CONFIG_MAX_QUEUE_DEFINE)                  \
    {                                                   \
        return OS_QUEUE_HANDLE_INVALID;                 \
    }                                                   \
}

#define OS_QUEUE_CHECK_BEEN_CREATED(HANDLE)             \
{                                                       \
    if (OS_QueuePool[HANDLE].Used == OS_QUEUE_UNUSED)   \
    {                                                   \
        return OS_QUEUE_NOT_BEEN_CREATED;               \
    }                                                   \
}

#define OS_QUEUE_HANDLE_TO_POINTER(HANDLE)              &OS_QueuePool[HANDLE]

#define OS_QUEUE_POS_TO_INDEX(QUEUE_P, POS)             ( (POS) % (QUEUE_P->ElementNr) )

#define OS_QUEUE_INDEX_TO_BUF_ADDR(QUEUE_P, INDEX)      ( ((OS_Uint8_t *)QUEUE_P->DataBuffer) + (QUEUE_P->ElementSize * INDEX) )

extern OS_TCB_t * volatile CurrentTCB;

extern void OS_TaskReadyToBlock(OS_TCB_t * TaskCB, ListHead_t *SleepHead, OS_Uint8_t BlockType, OS_Uint8_t SortType);
extern OS_Int16_t OS_IsSchedulerSuspending(void);
extern void OS_Schedule(void);
extern void OS_TaskBlockToReady(OS_TCB_t * TaskCB);
extern void OS_TaskChangePriority(OS_TCB_t * TaskCB, OS_Uint8_t NewPriority);

void OS_QueueInit(void)
{
    OS_Uint32_t i = 0;

    for (i = 0; i < CONFIG_MAX_QUEUE_DEFINE; i++)
    {
        OS_QueuePool[i].DataBuffer = OS_NULL;
        OS_QueuePool[i].ReadPos = 0;
        OS_QueuePool[i].WritePos = 0;
        OS_QueuePool[i].ElementSize = 0;
        OS_QueuePool[i].ElementNr = 0;
        OS_QueuePool[i].Used = OS_QUEUE_UNUSED;

        ListHeadInit(&OS_QueuePool[i].ReaderSleepList);
        ListHeadInit(&OS_QueuePool[i].WriterSleepList);
    }
}

OS_Uint32_t OS_GetQueueResource(OS_Uint32_t *QueueHandle)
{
    OS_Uint32_t i = 0;

    for (i = 0; i < CONFIG_MAX_QUEUE_DEFINE; i++)
    {
        if (OS_QueuePool[i].Used == OS_QUEUE_UNUSED)
            break;
    }

    if (i == CONFIG_MAX_QUEUE_DEFINE)
    {
        return OS_NOT_ENOUGH_QUEUE_RESOURCE;
    }
    else
    {
        *QueueHandle = i;
    }

    return OS_SUCCESS;
}

OS_Uint32_t OS_API_QueueCreate(OS_Uint32_t *QueueHandle,
                               OS_Uint32_t ElementSize,
                               OS_Uint32_t ElementNr)
{
    OS_Uint32_t Ret = OS_SUCCESS;

    OS_CHECK_NULL_POINTER(QueueHandle);

    if (ElementSize == 0 || ElementNr == 0)
    {
        return OS_QUEUE_CREATE_INVALID_PARAM;
    }

    OS_QUEUE_LOCK();

    Ret = OS_GetQueueResource(QueueHandle);
    if (Ret != OS_SUCCESS)
        goto OS_API_QueueCreate_Exit;

    /* Alloc buffer for the queue data */
    OS_QueuePool[*QueueHandle].DataBuffer = OS_API_Malloc(ElementSize * ElementNr);

    if (OS_QueuePool[*QueueHandle].DataBuffer == OS_NULL)
    {
        Ret = OS_NOT_ENOUGH_MEM_FOR_QUEUE_CREATE;
        goto OS_API_QueueCreate_Exit;
    }

    /* Record the queue element informations */
    OS_QueuePool[*QueueHandle].ElementNr = ElementNr;
    OS_QueuePool[*QueueHandle].ElementSize = ElementSize;

    /* Initial the read/write position to 0 */
    OS_QueuePool[*QueueHandle].ReadPos = 0;
    OS_QueuePool[*QueueHandle].WritePos = 0;

    /* Initial the read/write sleep list */
    ListHeadInit(&OS_QueuePool[*QueueHandle].ReaderSleepList);
    ListHeadInit(&OS_QueuePool[*QueueHandle].WriterSleepList);

    /* Mark this resource is used */
    OS_QueuePool[*QueueHandle].Used = OS_QUEUE_USED;

    TARCE_QueueCreate(QueueHandle);

OS_API_QueueCreate_Exit:
    OS_QUEUE_UNLOCK();

    return Ret;
}

void OS_QueueSleep(OS_TCB_t *TaskCB, ListHead_t *SleepListHead, OS_Uint8_t BlockType)
{
    // Insert to the sleep list with priority sort
    OS_TaskReadyToBlock(TaskCB, SleepListHead, BlockType, OS_BLOCK_SORT_TASK_PRIO);
}

void OS_QueueWakeup(ListHead_t *SleepListHead)
{
    OS_TCB_t *WakeupTaskCB = OS_NULL;

    WakeupTaskCB = ListFirstEntry(SleepListHead, OS_TCB_t, IpcSleepList);

    OS_TaskBlockToReady(WakeupTaskCB);
}

OS_Uint32_t OS_QueueRemainingSpace(OS_Queue_t *Queue)
{
   return (Queue->ElementNr - (Queue->WritePos - Queue->ReadPos) );
}

OS_Uint8_t OS_QueueEmpty(OS_Queue_t *Queue)
{
   return (Queue->WritePos == Queue->ReadPos);
}

OS_Uint8_t OS_QueueFull(OS_Queue_t *Queue)
{
   return ( OS_QueueRemainingSpace(Queue) == 0 );
}

static OS_Uint32_t OS_QueueWrite(OS_Uint32_t QueueHandle, const void * buffer,
                                 OS_Uint32_t size, OS_Uint8_t BlockType, OS_Uint32_t Timeout)
{
    OS_Uint32_t Ret = OS_SUCCESS;
    OS_Queue_t *Queue = OS_NULL;
    OS_TCB_t *TaskCB = CurrentTCB;
    OS_Uint32_t index = 0;
    OS_Uint8_t *BufferAddr = OS_NULL;

    OS_CHECK_NULL_POINTER(buffer);

    OS_QUEUE_CHECK_HANDLE_VALID(QueueHandle);
    OS_QUEUE_CHECK_BEEN_CREATED(QueueHandle);

    OS_QUEUE_LOCK();

    Queue = OS_QUEUE_HANDLE_TO_POINTER(QueueHandle);

    /* Check if the buffer size is greater than one element size */
    if (size > Queue->ElementSize)
    {
        Ret = OS_QUEUE_WR_DATA_TOO_BIG;
        goto OS_QueueWrite_Exit;
    }

    /*
     *********************************************************************
     * NOTE : Because when the queue is full the writer will be blocked
     * it will cause task sleep, and context switch immediately, but the
     * RTOS always pick the highest priority task to run, we can not make
     * sure the queue have space for writer when writer is woken up, so
     * here I use 'while' to make sure the queue have at least one space
     * for write in, in OS_QUEUE_LOCK environment.
     *********************************************************************
     */
    /* Check if the queue is full */
    while (OS_QueueFull(Queue))
    {
        /* Check if this is try behavior */
        if (Timeout == 0)
        {
            Ret = OS_QUEUE_TRY_WR_FAILED;
            goto OS_QueueWrite_Exit;
        }

        /* If current context is in ISR */
        if (ARCH_IsInterruptContext())
        {
            Ret = OS_QUEUE_WR_FULL_IN_INTR_CONTEXT;
            OS_PRINTK_ERROR("Queue Write Full In ISR");
            goto OS_QueueWrite_Exit;
        }

        /* If current scheduler is suspending */
        if (OS_IsSchedulerSuspending())
        {
            Ret = OS_QUEUE_WR_FULL_IN_SCH_SUSPEND;
            OS_PRINTK_ERROR("Queue Write Full in scheduler suspend");
            goto OS_QueueWrite_Exit;
        }

        /* Get wake up timestamp if using timeout strategy */
        TaskCB->WakeUpTime = OS_GetCurrentTime() + Timeout;

        /* Before sleep, clear the wake up flag */
        TaskCB->IpcTimeoutWakeup = OS_IPC_NO_TIMEOUT;

        /* Writer task go sleep */
        OS_QueueSleep(TaskCB, &Queue->WriterSleepList, BlockType);

        TARCE_QueueWriterSleep(TaskCB, Queue, BlockType);

        OS_Schedule();

        OS_QUEUE_UNLOCK();
        OS_QUEUE_LOCK();

        /* Wake up here */
        TARCE_QueueWriterWakeup(TaskCB, Queue);

        if (TaskCB->IpcTimeoutWakeup == OS_IPC_WAIT_TIMEOUT)
        {
            Ret = OS_QUEUE_WR_WAIT_TIMEOUT;
            goto OS_QueueWrite_Exit;
        }
    }

    TARCE_QueueWriteIn(TaskCB, Queue);

    /* The queue still have unused space, copy data in */
    /* According to the write postion, calculate the buffer index */
    index = OS_QUEUE_POS_TO_INDEX(Queue, Queue->WritePos);
    /* Accroding to the index, find out the buffer address */
    BufferAddr = OS_QUEUE_INDEX_TO_BUF_ADDR(Queue, index);
    /* Copy data into target queue buffer */
    OS_Memcpy((void *) BufferAddr, buffer, size);
    /* Update write position to next */
    Queue->WritePos++;

    /* Check if any task sleeping on the reader list */
    if (!ListEmpty(&Queue->ReaderSleepList))
    {
        TARCE_QueueWriteWakeupReader(TaskCB, Queue);
        /* Because new data arrived just wake up reader */
        OS_QueueWakeup(&Queue->ReaderSleepList);

        OS_Schedule();
    }

OS_QueueWrite_Exit:
    OS_QUEUE_UNLOCK();

    return Ret;
}

OS_Uint32_t OS_API_QueueWrite(OS_Uint32_t QueueHandle, const void * buffer, OS_Uint32_t size)
{
    return OS_QueueWrite(QueueHandle, buffer, size, OS_BLOCK_TYPE_ENDLESS, 0xFF);
}

OS_Uint32_t OS_API_QueueTryWrite(OS_Uint32_t QueueHandle, const void * buffer, OS_Uint32_t size)
{
    return OS_QueueWrite(QueueHandle, buffer, size, OS_BLOCK_TYPE_ENDLESS, 0x00);
}

OS_Uint32_t OS_API_QueueWriteTimeout(OS_Uint32_t QueueHandle, const void * buffer,
                                     OS_Uint32_t  size, OS_Uint32_t Timeout)
{
    if (Timeout >= OS_TSK_DLY_MAX)
    {
        return OS_MUTEX_INVALID_TIMEOUT;
    }

    return OS_QueueWrite(QueueHandle, buffer, size, OS_BLOCK_TYPE_TIMEOUT, Timeout);
}

 OS_Uint32_t OS_QueueRead(OS_Uint32_t QueueHandle, void * buffer, OS_Uint32_t size,
                          OS_Uint8_t BlockType, OS_Uint32_t Timeout)
{
    OS_Uint32_t Ret = OS_SUCCESS;
    OS_Queue_t *Queue = OS_NULL;
    OS_TCB_t *TaskCB = CurrentTCB;
    OS_Uint32_t index = 0;
    OS_Uint8_t *BufferAddr = OS_NULL;

    OS_CHECK_NULL_POINTER(buffer);

    OS_QUEUE_CHECK_HANDLE_VALID(QueueHandle);
    OS_QUEUE_CHECK_BEEN_CREATED(QueueHandle);

    OS_QUEUE_LOCK();

    Queue = OS_QUEUE_HANDLE_TO_POINTER(QueueHandle);

    /* Check if the buffer size is greater than one element size */
    if (size > Queue->ElementSize)
    {
        Ret = OS_QUEUE_RD_DATA_TOO_BIG;
        goto OS_QueueRead_Exit;
    }

    /*
     *********************************************************************
     * NOTE : Because when the queue is empty the reader will be blocked,
     * and it will cause task sleep, and context switch immediately, but the
     * RTOS always pick the highest priority task to run, we can not make
     * sure the queue have data for reader when reader is woken up, so
     * here I use 'while' to make sure the queue have at least one data
     * for read out, in OS_QUEUE_LOCK environment.
     *********************************************************************
     */
    /* Check if queue is empty */
    while (OS_QueueEmpty(Queue))
    {
        /* Check if this is try behavior */
        if (Timeout == 0)
        {
            Ret = OS_QUEUE_TRY_RD_FAILED;
            goto OS_QueueRead_Exit;
        }

        /* If current context is in ISR */
        if (ARCH_IsInterruptContext())
        {
            Ret = OS_QUEUE_RD_EMPTY_IN_INTR_CONTEXT;
            OS_PRINTK_ERROR("Queue Read empty In ISR");
            goto OS_QueueRead_Exit;
        }

        /* If current scheduler is suspending */
        if (OS_IsSchedulerSuspending())
        {
            Ret = OS_QUEUE_RD_EMPTY_IN_SCH_SUSPEND;
            OS_PRINTK_ERROR("Queue Read empty in scheduler suspend");
            goto OS_QueueRead_Exit;
        }

        /* Get wake up timestamp if using timeout strategy */
        TaskCB->WakeUpTime = OS_GetCurrentTime() + Timeout;

        /* Before sleep, clear the wake up flag */
        TaskCB->IpcTimeoutWakeup = OS_IPC_NO_TIMEOUT;

        /* Reader task go sleep */
        OS_QueueSleep(TaskCB, &Queue->ReaderSleepList, BlockType);

        TARCE_QueueReaderSleep(TaskCB, Queue, BlockType);

        OS_Schedule();

        OS_QUEUE_UNLOCK();
        OS_QUEUE_LOCK();

        /* Wake up here */
        TARCE_QueueReaderWakeup(TaskCB, Queue);

        if (TaskCB->IpcTimeoutWakeup == OS_IPC_WAIT_TIMEOUT)
        {
            Ret = OS_QUEUE_RD_WAIT_TIMEOUT;
            goto OS_QueueRead_Exit;
        }
    }

    TARCE_QueueReadOut(TaskCB, Queue);

    /* The queue have valid data, copy data out */
    /* According to the read postion, calculate the buffer index */
    index = OS_QUEUE_POS_TO_INDEX(Queue, Queue->ReadPos);
    /* Accroding to the index, find out the buffer address */
    BufferAddr = OS_QUEUE_INDEX_TO_BUF_ADDR(Queue, index);

    /* Copy queue data into target buffer */
    OS_Memcpy(buffer, (void *)BufferAddr, size);

    /* Update read position to next */
    Queue->ReadPos++;

    /* Check if any task sleeping on the writer list */
    if (!ListEmpty(&Queue->WriterSleepList))
    {
        TARCE_QueueReadWakeupWriter(TaskCB, Queue);
        /* Because new data read out, just wake up writer */
        OS_QueueWakeup(&Queue->WriterSleepList);

        OS_Schedule();
    }

OS_QueueRead_Exit:
    OS_QUEUE_UNLOCK();

    return Ret;
}

OS_Uint32_t OS_API_QueueRead(OS_Uint32_t QueueHandle, void * buffer, OS_Uint32_t size)
{
    return OS_QueueRead(QueueHandle, buffer, size, OS_BLOCK_TYPE_ENDLESS, 0xFF);
}

OS_Uint32_t OS_API_QueueTryRead(OS_Uint32_t QueueHandle, void * buffer, OS_Uint32_t size)
{
    return OS_QueueRead(QueueHandle, buffer, size, OS_BLOCK_TYPE_ENDLESS, 0x00);
}

OS_Uint32_t OS_API_QueueReadTimeout(OS_Uint32_t QueueHandle, void * buffer,
                                       OS_Uint32_t size, OS_Uint32_t Timeout)
{
    if (Timeout >= OS_TSK_DLY_MAX)
    {
        return OS_MUTEX_INVALID_TIMEOUT;
    }

    return OS_QueueRead(QueueHandle, buffer, size, OS_BLOCK_TYPE_TIMEOUT, Timeout);
}

OS_Uint32_t OS_API_QueueRemainingSpace(OS_Uint32_t QueueHandle)
{
    OS_Queue_t *Queue = OS_NULL;
    OS_Uint32_t RemainingSpace = 0;

    OS_QUEUE_CHECK_HANDLE_VALID(QueueHandle);
    OS_QUEUE_CHECK_BEEN_CREATED(QueueHandle);

    OS_QUEUE_LOCK();

    Queue = OS_QUEUE_HANDLE_TO_POINTER(QueueHandle);

    RemainingSpace = OS_QueueRemainingSpace(Queue);

    OS_QUEUE_UNLOCK();

    return RemainingSpace;
}

OS_Uint32_t OS_API_QueueDestory(OS_Uint32_t QueueHandle)
{
    OS_Uint32_t Ret = OS_SUCCESS;
    OS_Queue_t *Queue = OS_NULL;

    OS_QUEUE_CHECK_HANDLE_VALID(QueueHandle);
    OS_QUEUE_CHECK_BEEN_CREATED(QueueHandle);

    OS_QUEUE_LOCK();

    Queue = OS_QUEUE_HANDLE_TO_POINTER(QueueHandle);

    /* Check if any task sleeping on the writer list */
    if (!ListEmpty(&Queue->WriterSleepList))
    {
        Ret = OS_QUEUE_DESTORY_WR_SLP;
        goto OS_API_QueueDestory_Exit;
    }

    /* Check if any task sleeping on the reader list */
    if (!ListEmpty(&Queue->ReaderSleepList))
    {
        Ret = OS_QUEUE_DESTORY_RD_SLP;
        goto OS_API_QueueDestory_Exit;
    }

    /* Check if the queue is empty */
    if (!OS_QueueEmpty(Queue))
    {
        Ret = OS_QUEUE_DESTORY_QUEUE_NOT_EMPTY;
        goto OS_API_QueueDestory_Exit;
    }

    OS_API_Free(Queue->DataBuffer);

    Queue->Used = OS_QUEUE_UNUSED;

OS_API_QueueDestory_Exit:
    OS_QUEUE_UNLOCK();

    return Ret;
}

#endif // CONFIG_USE_QUEUE
