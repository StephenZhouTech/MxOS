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
#include "os_mem.h"
#include "os_list.h"
#include "os_types.h"

#include <stdio.h>

void *p1 = OS_NULL;
void *p2 = OS_NULL;
void *p3 = OS_NULL;
void *p4 = OS_NULL;
void *p5 = OS_NULL;

#define P1_SIZE             512
#define P2_SIZE             1024
#define P3_SIZE             2048
#define P4_SIZE             256
#define P5_SIZE             128

extern void OS_MemInit(void);
extern MemZone_t MemZone;
extern void OS_DBG_DumpFreeListInfo(void);
extern void OS_DBG_DumpUsedListInfo(void);

void MEM_TC_Entry(void)
{
    printf("**********************************************************\r\n");
    printf("********************* MEM_TC_Entry ***********************\r\n");
    printf("**********************************************************\r\n");

    OS_MemInit();

    printf("[Step 0] : Initial\r\n");
    OS_DBG_DumpFreeListInfo();
    OS_DBG_DumpUsedListInfo();

    /* Step 1 : malloc  */
    printf("[Step 1] : Malloc \r\n");
    p1 = OS_API_Malloc(P1_SIZE);
    if(p1 != OS_NULL)
    {
        printf("OS_API_Malloc %d Successful...\r\n", P1_SIZE);
    }
    else
    {
        printf("OS_API_Malloc %d Failed\r\n", P1_SIZE);
        while (1);
    }

    p2 = OS_API_Malloc(P2_SIZE);
    if(p2 != OS_NULL)
    {
        printf("OS_API_Malloc %d Successful...\r\n", P2_SIZE);
    }
    else
    {
        printf("OS_API_Malloc %d Failed\r\n", P2_SIZE);
        while (1);
    }

    p3 = OS_API_Malloc(P3_SIZE);
    if(p3 != OS_NULL)
    {
        printf("OS_API_Malloc %d Successful...\r\n", P3_SIZE);
    }
    else
    {
        printf("OS_API_Malloc %d Failed\r\n", P3_SIZE);
        while (1);
    }

    p4 = OS_API_Malloc(P4_SIZE);
    if(p4 != OS_NULL)
    {
        printf("OS_API_Malloc %d Successful...\r\n", P4_SIZE);
    }
    else
    {
        printf("OS_API_Malloc %d Failed\r\n", P4_SIZE);
        while (1);
    }
    OS_DBG_DumpFreeListInfo();
    OS_DBG_DumpUsedListInfo();

    /* Step 2 : free p3 */
    printf("[Step 2] : Free p3=0x%08X \r\n", p3);
    OS_API_Free(p3);
    OS_DBG_DumpFreeListInfo();
    OS_DBG_DumpUsedListInfo();

    /* Step 3 : malloc p5 */
    printf("[Step 3] : Malloc p5 \r\n");
    p5 = OS_API_Malloc(P5_SIZE);
    if(p5 != OS_NULL)
    {
        printf("OS_API_Malloc %d Successful...\r\n", P4_SIZE);
    }
    else
    {
        printf("OS_API_Malloc %d Failed\r\n", P4_SIZE);
        while (1);
    }
    OS_DBG_DumpFreeListInfo();
    OS_DBG_DumpUsedListInfo();

    /* Step 4 : free p4 */
    printf("[Step 4] : Free p4=0x%08X \r\n", p4);
    OS_API_Free(p4);
    OS_DBG_DumpFreeListInfo();
    OS_DBG_DumpUsedListInfo();

    /* Step 5 : free p2 */
    printf("[Step 5] : Free p2=0x%08X \r\n", p2);
    OS_API_Free(p2);
    OS_DBG_DumpFreeListInfo();
    OS_DBG_DumpUsedListInfo();

    /* Step 6 : free p1 */
    printf("[Step 6] : Free p1=0x%08X \r\n", p1);
    OS_API_Free(p1);
    OS_DBG_DumpFreeListInfo();
    OS_DBG_DumpUsedListInfo();

    /* Step 7 : free p5 */
    printf("[Step 7] : Free p5=0x%08X \r\n", p5);
    OS_API_Free(p5);
    OS_DBG_DumpFreeListInfo();
    OS_DBG_DumpUsedListInfo();

    printf("********************* MEM TC Finished ***********************\r\n");
}
