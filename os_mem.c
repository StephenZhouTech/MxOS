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
#include "os_configs.h"
#include "os_lib.h"
#include "arch.h"
#include "os_mem.h"

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

/* The size of the structure placed at the beginning of each allocated memory
block must by correctly byte aligned. */
static const OS_Uint32_t MmBlkDescAlignSize = ( sizeof( MemBlockDesc_t ) + ( ( OS_Uint32_t ) ( ARCH_BYTE_ALIGNMENT - 1 ) ) ) & ~( ( OS_Uint32_t ) ARCH_BYTE_ALIGNMENT_MASK );

#define OS_MM_MIN_BLOCK_SZ          (MmBlkDescAlignSize << 1)

static MemZone_t MemZone;
static OS_Uint8_t _Heap[ CONFIG_TOTAL_HEAP_SIZE ];
static OS_Uint8_t MemInitialFinished = 0;

#if MM_DEBUG
void *DBG_GetMemZone(void)
{
    return ((void *)&MemZone);
}
#endif // MM_DEBUG

static void _OS_MemInit(void)
{
    MemBlockDesc_t *MmBlockDesc = OS_NULL;

    /* Start Address must be aligned firstly */
    MemZone.StartAddr = OS_DataAlign((OS_Uint32_t)_Heap, ARCH_BYTE_ALIGNMENT, ARCH_BYTE_ALIGNMENT_MASK);
    /*
     * Calculate the total size of the memory zone.
     * Note: the total size include the block descriptor struct : MemBlockDesc_t
     */
    MemZone.TotalSize = CONFIG_TOTAL_HEAP_SIZE - (MemZone.StartAddr - (OS_Uint32_t)_Heap);

    /*
     * Calculate the remaining size of the memory zone.
     * Note: This size not include the block descriptor struct : MemBlockDesc_t
     * So, we sub a MmBlkDescAlignSize because the first of the memory will put the MemBlockDesc_t
     */
    MemZone.RemainingSize    = MemZone.TotalSize - MmBlkDescAlignSize;
    /* Init the list head of the free list */
    ListHeadInit(&MemZone.FreeListHead);
    ListHeadInit(&MemZone.UsedListHead);

    /* Add a free memory block at the begining of the aligned heap */
    MmBlockDesc         = (MemBlockDesc_t *)MemZone.StartAddr;
    MmBlockDesc->Size   = MemZone.TotalSize;
    ListAdd(&MmBlockDesc->List, &MemZone.FreeListHead);
}

static void _InsertMmBlockDescToFreeList(ListHead_t *InsertList)
{
    ListHead_t     *ListIterator = OS_NULL;
    MemBlockDesc_t *MmBlkDescIterator = OS_NULL;
    MemBlockDesc_t *WaitForInsert = (MemBlockDesc_t *)InsertList;

    // Sort the memory block size from small to big
    ListForEach(ListIterator, &MemZone.FreeListHead)
    {
        MmBlkDescIterator = (MemBlockDesc_t *)ListIterator;
        if (MmBlkDescIterator->Size > WaitForInsert->Size)
            break;
    }

    // The insert one is the biggset one
    if (ListIterator == &MemZone.FreeListHead)
    {
        ListAddTail(InsertList, &MemZone.FreeListHead);
    }
    else
    {
        ListAdd(InsertList, MmBlkDescIterator->List.prev);
    }
}

void *OS_API_Malloc(OS_Uint32_t WantSize)
{
    void *pReturnAddr = OS_NULL;

    OS_Uint32_t OS_RequstSize = 0;
    ListHead_t *ListPos = OS_NULL;

    MemBlockDesc_t *AllocteMmBlkDesc = OS_NULL;
    MemBlockDesc_t *NewMmBlkDesc     = OS_NULL;

    // If the memory manager have not been init, just call this first
    if (!MemInitialFinished)
    {
        _OS_MemInit();
        MemInitialFinished = 1;
    }

    if (WantSize > 0)
    {
        // Calculate real os size by adding the aligned MemBlockDesc_t
        OS_RequstSize = WantSize + MmBlkDescAlignSize;
        OS_RequstSize = OS_DataAlign(OS_RequstSize, ARCH_BYTE_ALIGNMENT, ARCH_BYTE_ALIGNMENT_MASK);
    }

    // Check if the max free block memory size is enough
    if (OS_RequstSize <= MemZone.RemainingSize)
    {
        // Iterate the memory block descriptor to find the bestfit free block
        ListForEach(ListPos, &MemZone.FreeListHead)
        {
            AllocteMmBlkDesc = (MemBlockDesc_t *)ListPos;
            // Find bestfit one
            if (AllocteMmBlkDesc->Size >= OS_RequstSize)
            {
                pReturnAddr = (void *)( ((OS_Uint8_t *)AllocteMmBlkDesc) + MmBlkDescAlignSize);
                // Delete from the free list
                ListDel(&AllocteMmBlkDesc->List);
                // Add the allocted memory block to used list
                ListAdd(&AllocteMmBlkDesc->List, &MemZone.UsedListHead);

                // Check if the size of this block can be split into two part
                if ((AllocteMmBlkDesc->Size - OS_RequstSize) > OS_MM_MIN_BLOCK_SZ)
                {
                    NewMmBlkDesc = (MemBlockDesc_t *)( (OS_Uint8_t *)AllocteMmBlkDesc + OS_RequstSize);
                    NewMmBlkDesc->Size   = AllocteMmBlkDesc->Size - OS_RequstSize;

                    AllocteMmBlkDesc->Size = OS_RequstSize;

                    _InsertMmBlockDescToFreeList(&NewMmBlkDesc->List);
                }
                MemZone.RemainingSize -= AllocteMmBlkDesc->Size;
                break;
            }
        }

        if (ListPos == &MemZone.FreeListHead)
        {
            // can not find memory to be allocted
        }
    }
    else
    {
        // not enough memory
    }

    return pReturnAddr;
}

static void _MergeMmBlock(ListHead_t *MergetList)
{
    ListHead_t     *ListIterator      = OS_NULL;
    MemBlockDesc_t *MmBlkDescIterator = OS_NULL;
    MemBlockDesc_t *MmBlkDescAddrPrev = OS_NULL;
    MemBlockDesc_t *MmBlkDescAddrPost = OS_NULL;
    MemBlockDesc_t *MergeMmBlkDesc    = (MemBlockDesc_t *)MergetList;
    ListHead_t     *InsertList        = MergetList;

    // find the prev and post address
    ListForEach(ListIterator, &MemZone.FreeListHead)
    {
        MmBlkDescIterator = (MemBlockDesc_t *)ListIterator;
        if ((OS_Uint8_t *)MmBlkDescIterator + MmBlkDescIterator->Size == (OS_Uint8_t *)MergeMmBlkDesc)
        {
            MmBlkDescAddrPrev = MmBlkDescIterator;
        }
        if ((OS_Uint8_t *)MergeMmBlkDesc + MergeMmBlkDesc->Size == (OS_Uint8_t *)MmBlkDescIterator)
        {
            MmBlkDescAddrPost = MmBlkDescIterator;
        }
    }

    if (MmBlkDescAddrPost)
    {
        ListDel(&MmBlkDescAddrPost->List);
        MergeMmBlkDesc->Size   += MmBlkDescAddrPost->Size;
    }

    if (MmBlkDescAddrPrev)
    {
        ListDel(&MmBlkDescAddrPrev->List);
        MmBlkDescAddrPrev->Size   += MergeMmBlkDesc->Size;
        InsertList = &MmBlkDescAddrPrev->List;
    }
    _InsertMmBlockDescToFreeList(InsertList);
}

void OS_API_Free(void *pAddr)
{
    ListHead_t     *ListIterator = OS_NULL;
    MemBlockDesc_t *UsedMmBlkDesc = OS_NULL;
    MemBlockDesc_t *MmBlkDescIterator = OS_NULL;

    if (!MemInitialFinished)
        return;

    if (pAddr != OS_NULL)
    {
        // find the memory block descriptor first
        UsedMmBlkDesc = (MemBlockDesc_t *)( (OS_Uint8_t *)pAddr - MmBlkDescAlignSize );
        // check if this is in used list
        ListForEach(ListIterator, &MemZone.UsedListHead)
        {
            MmBlkDescIterator = (MemBlockDesc_t *)ListIterator;
            if (MmBlkDescIterator == UsedMmBlkDesc)
                break;
        }

        OS_ASSERT(ListIterator != &MemZone.UsedListHead);
        // Delete from used list
        ListDel(&UsedMmBlkDesc->List);
        // Set status to free
        // Check if it can be merged with other memory block
        _MergeMmBlock(&UsedMmBlkDesc->List);
    }
}
