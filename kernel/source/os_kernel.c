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
#include "os_kernel.h"
#include "os_critical.h"

extern void OS_MemInit(void);
extern void OS_SchedulerInit(void);
extern void OS_CriticalInit(void);
extern void OS_IdleTaskCreate(void);
extern void OS_FirstTaskStartup(void);

void OS_API_KernelInit(void)
{
    /* Initial the critical */
    OS_CriticalInit();

    /* Initial the memory manager */
    OS_MemInit();

    /* Initial the task scheduler */
    OS_SchedulerInit();

    /* Initial the Timestamp value */
    OS_TimeInit();
}

void OS_API_KernelStart(void)
{
    /* Lock */
    ARCH_InterruptDisable();

    /* Create Idle task */
    OS_IdleTaskCreate();

    /* Configure the IRQ just like NVIC priority */
    ARCH_InterruptInit();

    /* Configure the misc just like FPU feature */
    ARCH_MiscInit();

    /* Configure System Tick for OS heart beat */
    ARCH_SystemTickInit();

    /* Startup The first task */
    OS_FirstTaskStartup();

    /* Enable interrupt before start the first task */
}

