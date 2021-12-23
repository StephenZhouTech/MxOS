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

#ifndef __MXOS_QUEUE_H__
#define __MXOS_QUEUE_H__

#include "os_types.h"
#include "os_list.h"

typedef struct _OS_Queue {
    /* This field hold the buffer of the queue */
    void           *DataBuffer;
    /* This list pend all of the reader when the queue is empty */
    ListHead_t      ReaderSleepList;
    /* This list pend all of the writer when the queue is full */
    ListHead_t      WriterSleepList;
    /* This position means the next message will be read out */
    OS_Uint32_t     ReadPos;
    /* This position means the next message will be write in */
    OS_Uint32_t     WritePos;
    /* This means one element size */
    OS_Uint32_t     ElementSize;
    /* This means how many element will be managed in this queue */
    OS_Uint32_t     ElementNr;
    /* This stand the status of this queue */
    OS_Uint8_t      Used;
} OS_Queue_t;

typedef enum _OS_QueueUsed {
    OS_QUEUE_UNUSED = 0,
    OS_QUEUE_USED
} OS_QueueUsed_e;

OS_Uint32_t OS_API_QueueCreate(OS_Uint32_t *QueueHandle,OS_Uint32_t ElementSize,OS_Uint32_t ElementNr);

OS_Uint32_t OS_API_QueueWrite(OS_Uint32_t QueueHandle, const void * buffer, OS_Uint32_t size);

OS_Uint32_t OS_API_QueueTryWrite(OS_Uint32_t QueueHandle, const void * buffer, OS_Uint32_t size);

OS_Uint32_t OS_API_QueueWriteTimeout(OS_Uint32_t QueueHandle, const void * buffer, OS_Uint32_t  size, OS_Uint32_t Timeout);

OS_Uint32_t OS_API_QueueRead(OS_Uint32_t QueueHandle, void * buffer, OS_Uint32_t size);

OS_Uint32_t OS_API_QueueTryRead(OS_Uint32_t QueueHandle, void * buffer, OS_Uint32_t size);

OS_Uint32_t OS_API_QueueReadTimeout(OS_Uint32_t QueueHandle, void * buffer, OS_Uint32_t size, OS_Uint32_t Timeout);

OS_Uint32_t OS_API_QueueDestory(OS_Uint32_t QueueHandle);

OS_Uint32_t OS_API_QueueRemainingSpace(OS_Uint32_t QueueHandle);

#endif // __MXOS_QUEUE_H__
