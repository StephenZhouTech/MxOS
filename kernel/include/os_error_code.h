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
#ifndef __MXOS_ERROR_CODE_H__
#define __MXOS_ERROR_CODE_H__

typedef enum _OS_ErrorCode {
    OS_SUCCESS = 0,
    OS_NULL_POINTER,
    OS_TASK_PRIO_OUT_OF_RANGE,
    OS_NOT_ENOUGH_MEM_FOR_TASK_CREATE,
    OS_YIELD_IN_INTR_CONTEXT,
    OS_YIELD_IN_SCH_SUSPEND,
    OS_TSK_DLY_IN_INTR_CONTEXT,
    OS_TSK_DLY_IN_SCH_SUSPEND,
    OS_TSK_DLY_TICK_INVALID,
    OS_TSK_SUSPEND_IN_SUSPENDING,
    OS_SUSPEND_CUR_TSK_IN_INTR,
    OS_SUSPEND_CUR_TSK_IN_SCH_SUSPEND,
    OS_RESUME_CUR_TSK,
    OS_RESUME_TSK_NOT_IN_SUSPEND,
    OS_SET_SAME_PRIO,
} OS_ErrorCode_e;

#define OS_CHECK_RETURN(Ret)                \
{                                           \
    if(Ret != OS_SUCCESS)                   \
        return Ret;                         \
}

#endif // __MXOS_ERROR_CODE_H__



