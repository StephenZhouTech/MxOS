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
#include "os_types.h"
#include "os_list.h"

typedef struct _OS_Sem {
    ListHead_t List;
    OS_Uint32_t Count;
    OS_Uint8_t Used;
} OS_Sem_t;

typedef enum _OS_SemUsed {
    OS_SEM_UNUSED = 0,
    OS_SEM_USED
} OS_SemUsed_e;

OS_Uint32_t OS_API_SemCreate(OS_Uint32_t *SemHandle, OS_Uint32_t Count);
OS_Uint32_t OS_API_BinarySemCreate(OS_Uint32_t *SemHandle, OS_Uint32_t Count);

OS_Uint32_t OS_API_SemWait(OS_Uint32_t SemHandle);
OS_Uint32_t OS_API_BinarySemWait(OS_Uint32_t SemHandle);

OS_Uint32_t OS_API_SemWaitTimeout(OS_Uint32_t SemHandle, OS_Uint32_t Timeout);
OS_Uint32_t OS_API_BinarySemWaitTimeout(OS_Uint32_t SemHandle, OS_Uint32_t Timeout);

OS_Uint32_t OS_API_SemTryWait(OS_Uint32_t SemHandle);
OS_Uint32_t OS_API_BinarySemTryWait(OS_Uint32_t SemHandle);

OS_Uint32_t OS_API_SemPost(OS_Uint32_t SemHandle);
OS_Uint32_t OS_API_BinarySemPost(OS_Uint32_t SemHandle);

OS_Uint32_t OS_API_SemDestory(OS_Uint32_t SemHandle);
