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

#ifndef __MXOS_MM_H__
#define __MXOS_MM_H__

#include "os_types.h"
#include "os_list.h"
/*
 * Memory Manager support multi-zone with different priority
 * In default, the lower index in zone list, the higher priority to be allocated 
 */

/* MemZone_t is a struct to mamnage the whole memory */
typedef struct _MemZone {
    ListHead_t  FreeListHead;       /* The free list head of the memory         */
    ListHead_t  UsedListHead;       /* The used list head of the memory         */
    OS_Uint32_t StartAddr;          /* The start address of the memory          */
    OS_Uint32_t TotalSize;          /* The total size of the memory(aligned)    */
    OS_Uint32_t RemainingSize;      /* The remaining size of the memory         */
} MemZone_t;

/* MemBlockDesc_t is a struct to present every memory block */
typedef struct _MemBlockDesc
{
    ListHead_t  List;               /* The list node in free list               */
    OS_Uint32_t Size;               /* The memory block size                    */
} MemBlockDesc_t;

void *OS_API_Malloc(OS_Uint32_t sz);
void OS_API_Free(void *addr);

#endif // !__MXOS_MM_H__
