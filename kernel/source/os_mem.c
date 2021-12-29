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
#include "os_lib.h"
#include "os_mem.h"
#include "os_list.h"
#include "os_trace.h"
#include "os_printk.h"
#include "os_configs.h"
#include "os_critical.h"

#if CONFIG_USE_SHELL
#include "os_shell.h"
#endif

#define OS_MEM_LOCK()                   OS_API_EnterCritical()
#define OS_MEM_UNLOCK()                 OS_API_ExitCritical()

#if OS_DBG_MEMORY
    #define MEM_FUNCTION_SPACE
#else
    #define MEM_FUNCTION_SPACE static
#endif


/* The size of the structure placed at the beginning of each allocated memory
block must by correctly byte aligned. */
static const OS_Uint32_t MmBlkDescAlignSize = ( sizeof( MemBlockDesc_t ) + ( ( OS_Uint32_t ) ( ARCH_BYTE_ALIGNMENT - 1 ) ) ) & ~( ( OS_Uint32_t ) ARCH_BYTE_ALIGNMENT_MASK );

#define OS_MM_MIN_BLOCK_SZ          (MmBlkDescAlignSize << 1)

MEM_FUNCTION_SPACE MemZone_t MemZone;
MEM_FUNCTION_SPACE OS_Uint8_t _Heap[ CONFIG_TOTAL_HEAP_SIZE ];

void OS_MemInit(void)
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

    OS_PRINTK_INFO("Total memory : 0x%08X Bytes, Address at 0x%08X", MemZone.TotalSize, MemZone.StartAddr);
    OS_PRINTK_INFO("Memory Mamanger Init finished...");

    TRACE_MemoryInit(MemZone);
}

static void OS_InsertMemBlockDescToFreeList(ListHead_t *InsertList)
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

    OS_MEM_LOCK();

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

                TRACE_Malloc(TP_MALLOC_SUCCESS, AllocteMmBlkDesc, MemZone);

                // Check if the size of this block can be split into two part
                if ((AllocteMmBlkDesc->Size - OS_RequstSize) > OS_MM_MIN_BLOCK_SZ)
                {
                    NewMmBlkDesc = (MemBlockDesc_t *)( (OS_Uint8_t *)AllocteMmBlkDesc + OS_RequstSize);
                    NewMmBlkDesc->Size   = AllocteMmBlkDesc->Size - OS_RequstSize;

                    AllocteMmBlkDesc->Size = OS_RequstSize;

                    OS_InsertMemBlockDescToFreeList(&NewMmBlkDesc->List);

                    TRACE_Malloc(TP_MALLOC_SUCCESS_SPLIT, NewMmBlkDesc, MemZone);
                }
                MemZone.RemainingSize -= AllocteMmBlkDesc->Size;
                break;
            }
        }

        if (ListPos == &MemZone.FreeListHead)
        {
            // can not find memory to be allocted
            TRACE_Malloc(TP_MALLOC_FAILED_NOT_ENOUGH, OS_NULL, MemZone);
            OS_PRINTK_WARNING("Not enough memory in free list for Malloc");
        }
    }
    else
    {
        // not enough memory
        TRACE_Malloc(TP_MALLOC_FAILED_WANT_TOO_LARGE, OS_NULL, MemZone);
        OS_PRINTK_WARNING("Total memory not enough for Malloc");
    }

    OS_MEM_UNLOCK();

    return pReturnAddr;
}

static void OS_MergeMemBlock(ListHead_t *MergetList)
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
        TRACE_Free(TP_FREE_MERGE_POST, MergeMmBlkDesc, MmBlkDescAddrPost, MemZone);

        ListDel(&MmBlkDescAddrPost->List);
        MergeMmBlkDesc->Size   += MmBlkDescAddrPost->Size;
    }

    if (MmBlkDescAddrPrev)
    {
        TRACE_Free(TP_FREE_MERGE_PREV, MergeMmBlkDesc, MmBlkDescAddrPost, MemZone);

        ListDel(&MmBlkDescAddrPrev->List);
        MmBlkDescAddrPrev->Size   += MergeMmBlkDesc->Size;
        InsertList = &MmBlkDescAddrPrev->List;
    }
    OS_InsertMemBlockDescToFreeList(InsertList);
}

void OS_API_Free(void *pAddr)
{
    ListHead_t     *ListIterator = OS_NULL;
    MemBlockDesc_t *UsedMmBlkDesc = OS_NULL;
    MemBlockDesc_t *MmBlkDescIterator = OS_NULL;

    OS_MEM_LOCK();

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
        OS_MergeMemBlock(&UsedMmBlkDesc->List);
    }

    TRACE_Free(TP_FREE_DONE, OS_NULL, OS_NULL, MemZone);

    OS_MEM_UNLOCK();
}

#if CONFIG_USE_SHELL

void ShellMem(void)
{
    ListHead_t     *ListIterator = OS_NULL;
    MemBlockDesc_t *MmBlkDescIterator = OS_NULL;

    printf("----------------------- Total Memory ----------------------\r\n");
    printf("|--- Address ---|--- Size(Bytes) ---|\r\n");
    printf("|   0x%08X      0x%08X     |\r\n", MemZone.StartAddr, MemZone.TotalSize);


    printf("----------------------- Free Memory -----------------------\r\n");
    printf("|--- Address ---|--- Size(Bytes) ---|\r\n");
    ListForEach(ListIterator, &MemZone.FreeListHead)
    {
        MmBlkDescIterator = (MemBlockDesc_t *)ListIterator;
        printf("|   0x%08X      0x%08X     |\r\n", (OS_Uint32_t)MmBlkDescIterator, MmBlkDescIterator->Size);
    }

    printf("----------------------- Used Memory -----------------------\r\n");
    printf("|--- Address ---|--- Size(Bytes) ---|\r\n");
    ListForEach(ListIterator, &MemZone.UsedListHead)
    {
        MmBlkDescIterator = (MemBlockDesc_t *)ListIterator;
        printf("|   0x%08X      0x%08X     |\r\n", (OS_Uint32_t)MmBlkDescIterator, MmBlkDescIterator->Size);
    }
}
SHELL_EXPORT_CMD(mem, ShellMem, Show memory info);

#endif // CONFIG_USE_SHELL

