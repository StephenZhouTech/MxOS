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

#include "os_time.h"
#include "os_types.h"
#include "os_configs.h"

OS_Uint32_t volatile OS_CurrentTime = 0;

/* 
 * Initial the kernel timestamp
 * Note : This function called by OS_API_KernelInit()
 */
void OS_TimeInit(void)
{
    OS_CurrentTime = CONFIG_TICK_COUNT_INIT_VALUE;
}

/* 
 * Initial Increment kernel timestamp
 * Note : This function called when system tick handle
 * This should be protected with lock
 */
void OS_IncrementTime(void)
{
    OS_CurrentTime++;
}

/* 
 * Get current kernel timestamp
 * Note : This should be protected with lock
 */
OS_Uint32_t OS_GetCurrentTime(void)
{
    return OS_CurrentTime;
}
