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

#ifndef __MXOS_TRACE_H__
#define __MXOS_TRACE_H__

/************************** Trace For Memory manger **************************/
typedef enum _OS_TracePointMem {
    TP_MALLOC_SUCCESS,
    TP_MALLOC_SUCCESS_SPLIT,
    TP_MALLOC_FAILED_NOT_ENOUGH,
    TP_MALLOC_FAILED_WANT_TOO_LARGE,
    TP_FREE_MERGE_PREV,
    TP_FREE_MERGE_POST,
    TP_FREE_DONE
} OS_TracePointMem_e;

#ifndef TRACE_MemoryInit
    #define TRACE_MemoryInit(MemZone)
#endif

#ifndef TRACE_Malloc
    #define TRACE_Malloc(TracePoint, MemBlockDesc, MemZone)
#endif

#ifndef TRACE_Free
    #define TRACE_Free(TracePoint, WaitForFree, MergePostion, MemZone)
#endif

/**************************** Trace For Scheduler ****************************/
typedef enum _OS_TracePointScheduler {
    TP_READY_LIST,
    TP_DELAY_LIST,
    TP_SUSPEND_LIST,
    TP_BLOCKED_LIST
} OS_TracePointScheduler_e;

#ifndef TRACE_IncrementTick
    #define TRACE_IncrementTick(Tick)
#endif

#ifndef TRACE_AddToTargetList
    #define TRACE_AddToTargetList(List, TaskCB)
#endif

#ifndef TRACE_RemoveFromTargetList
    #define TRACE_RemoveFromTargetList(List, TaskCB)
#endif

#ifndef TRACE_TaskDelayTimeout
    #define TRACE_TaskDelayTimeout(TaskCB)
#endif

#ifndef TRACE_TaskBlockTimeout
    #define TRACE_TaskBlockTimeout(TaskCB)
#endif

#ifndef TRACE_ContextSwitch
    #define TRACE_ContextSwitch(_Current, _Next, Timestamp)
#endif

#ifndef TRACE_SchedulerSuspend
    #define TRACE_SchedulerSuspend(NestingCnt)
#endif

#ifndef TRACE_SchedulerResume
    #define TRACE_SchedulerResume(NestingCnt)
#endif

/**************************** Trace For Task ****************************/
#ifndef TRACE_TaskCreate
    #define TRACE_TaskCreate(TaskCB)
#endif

#ifndef TRACE_TaskSuspend
    #define TRACE_TaskSuspend(TaskCB, _Current)
#endif

#ifndef TRACE_TaskResume
    #define TRACE_TaskResume(TaskCB)
#endif

#ifndef TRACE_TaskDelay
    #define TRACE_TaskDelay(TaskCB, DelayTick)
#endif

#ifndef TRACE_TaskYield
    #define TRACE_TaskYield(TaskCB)
#endif

#ifndef TARCE_TaskPrioritySet
    #define TARCE_TaskPrioritySet(TaskCB, _Current, Priority)
#endif

/**************************** Trace For Semaphore ****************************/
#ifndef TARCE_SemCreate
    #define TARCE_SemCreate(SemHandle, Count)
#endif

#ifndef TARCE_SemWaitEntry
    #define TARCE_SemWaitEntry(Sem)
#endif

#ifndef TARCE_SemWaitSleep
    #define TARCE_SemWaitSleep(TaskCB, Sem, BlockType)
#endif

#ifndef TARCE_SemWakeup
    #define TARCE_SemWakeup(TaskCB)
#endif

#ifndef TARCE_SemDestory
    #define TARCE_SemDestory(TaskCB)
#endif

/**************************** Trace For Mutex ****************************/
#ifndef TARCE_MutexCreate
    #define TARCE_MutexCreate(MutexHandle)
#endif

#ifndef TARCE_MutexSleep
    #define TARCE_MutexSleep(TaskCB, Mutex, BlockType)
#endif

#ifndef TARCE_MutexLock
    #define TARCE_MutexLock(TaskCB)
#endif

#ifndef TARCE_MutexUnLock
    #define TARCE_MutexUnLock(TaskCB)
#endif

#ifndef TARCE_MutexWakeup
    #define TARCE_MutexWakeup(TaskCB, Mutex)
#endif

/**************************** Trace For Queue ****************************/
#ifndef TARCE_QueueCreate
    #define TARCE_QueueCreate(QueueHandle)
#endif

#ifndef TARCE_QueueWriterSleep
    #define TARCE_QueueWriterSleep(TaskCB, Queue, BlockType)
#endif

#ifndef TARCE_QueueReaderSleep
    #define TARCE_QueueReaderSleep(TaskCB, Queue, BlockType)
#endif

#ifndef TARCE_QueueWriterWakeup
    #define TARCE_QueueWriterWakeup(TaskCB, Queue)
#endif

#ifndef TARCE_QueueReaderWakeup
    #define TARCE_QueueReaderWakeup(TaskCB, Queue)
#endif

#ifndef TARCE_QueueReadWakeupWriter
    #define TARCE_QueueReadWakeupWriter(TaskCB, Queue)
#endif

#ifndef TARCE_QueueWriteWakeupReader
    #define TARCE_QueueWriteWakeupReader(TaskCB, Queue)
#endif

#ifndef TARCE_QueueReadOut
    #define TARCE_QueueReadOut(TaskCB, Queue)
#endif

#ifndef TARCE_QueueWriteIn
    #define TARCE_QueueWriteIn(TaskCB, Queue)
#endif


#endif // __MXOS_TRACE_H__

