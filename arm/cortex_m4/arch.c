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
#include "os_task_sch.h"

/* Constants required to set up the initial stack. */
#define THUMB_CODE_BIT              ( 0x01 << 24 )
#define ARCH_XPSR_INIT              THUMB_CODE_BIT

/*
 * 0xFFFF_FFF1 means return Handler mode
 * 0xFFFF_FFF9 means return Thread mode using MSP
 * 0xFFFF_FFFD means return Thread mode using psp
*/
#define ARCH_EXEC_RETURN            ( 0xFFFFFFFD )

/* Note: Do not modify this struct sequence, this definiation is sort by hardware arch */
typedef struct _TaskContext {
    OS_Uint32_t R4;
    OS_Uint32_t R5;
    OS_Uint32_t R6;
    OS_Uint32_t R7;
    OS_Uint32_t R8;
    OS_Uint32_t R9;
    OS_Uint32_t R10;
    OS_Uint32_t R11;
    OS_Uint32_t excReturn;
    OS_Uint32_t R0;
    OS_Uint32_t R1;
    OS_Uint32_t R2;
    OS_Uint32_t R3;
    OS_Uint32_t R12;
    OS_Uint32_t LR;
    OS_Uint32_t PC;
    OS_Uint32_t xPSR;
} TaskContext;

static void TaskExitErrorEntry( void )
{
    __disable_irq();
    while(1);
}

void ARCH_PrepareStack(void *StartOfStack, void *Param)
{
    TaskContext *taskContext = OS_NULL;
    TaskInitParameter *TaskParam = (TaskInitParameter *)Param;

    OS_Uint32_t TopOfStack = (OS_Uint32_t)StartOfStack + TaskParam->StackSize - sizeof(OS_Uint32_t);

    taskContext = (TaskContext *)( TopOfStack - sizeof(TaskContext) );
    /* 
     * According the Cortex-M4 datasheet, we switch contex in PendSV handler
     * and the exception stack frame struct layout in storage sequence is just like:
     * -------------------- Hardware auto stack in/out --------------------
     * 1.  |xPSR |
     * 2.  |PC   |
     * 3.  |LR   |
     * 4.  |R12  |
     * 5.  |R3   |
     * 6.  |R2   |
     * 7.  |R1   |
     * 8.  |R0   |
     * -------------------- Stack in/out by manual --------------------
     * 9.  |R11  |
     * 10. |R10  |
     * 12. |R9   |
     * 13. |R8   |
     * 14. |R7   |
     * 15. |R6   |
     * 16. |R5   |
     * 17. |R4   |
     */
    taskContext->excReturn = ARCH_EXEC_RETURN;
    taskContext->R0  = (OS_Uint32_t)TaskParam->PrivateData;
    taskContext->LR  = (OS_Uint32_t)TaskExitErrorEntry;
    taskContext->PC  = (OS_Uint32_t)TaskParam->TaskEntry;
    taskContext->xPSR = ARCH_XPSR_INIT;
    taskContext->R4 =  0x04040404;
}

void ARCH_DisableIRQ(void)
{

}

void ARCH_EnableIRQ(void)
{

}

void ARCH_IRQInit(void)
{

}

void ARCH_SystemTickInit(OS_Uint32_t TickRateHZ)
{

}

void ARCH_StartScheduler(void)
{

}
