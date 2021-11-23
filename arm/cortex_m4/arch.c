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
//#include "core_cm4.h"

#if ((defined(__CC_ARM) && defined(__TARGET_FPU_VFP))                         \
     || (defined(__CLANG_ARM) && defined(__VFP_FP__) && !defined(__SOFTFP__)) \
     || (defined(__ICCARM__) && defined(__ARMVFP__ ))                          \
     || (defined(__GNUC__) && defined(__VFP_FP__) && !defined(__SOFTFP__)))
#define ARCH_FPU_USED   1
#else
#define ARCH_FPU_USED   0
#endif

/* Constants required to set up the initial stack. */
#define THUMB_CODE_BIT              ( 0x01 << 24 )
#define ARCH_XPSR_INIT              THUMB_CODE_BIT

/*
 * 0xFFFF_FFF1 means return Handler mode
 * 0xFFFF_FFF9 means return Thread mode using MSP
 * 0xFFFF_FFFD means return Thread mode using psp
*/
#define ARCH_EXEC_RETURN            ( 0xFFFFFFFD )

/*
 * NVIC Priority,set pendsv and systick as lowset priority
 */
#define ARCH_NVIC_LOWEST_PRIO       255
#define ARCH_PENDSV_PRIORITY        ARCH_NVIC_LOWEST_PRIO
#define ARCH_SYSTICK_PRIORITY       ARCH_NVIC_LOWEST_PRIO

/* The System NVIC priority register can be access by Byte */
#define ARCH_PENDSV_PRIO_REG        0xE000ED22
#define ARCH_SYSTICK_PRIO_REG       0xE000ED23

/* SystemTick configure for MxOS heart beat */
#define ARCH_SYSTICK_CTL            0xE000E010
#define ARCH_SYSTICK_RELOAD         0xE000E014
#define ARCH_SYSTICK_CURRENT        0xE000E018
#define ARCH_SYSTICK_CABLI          0xE000E01C

#define ARCH_SYSTICK_CLK_SRC        (0x01 << 2)
#define ARCH_SYSTICK_INT            (0x01 << 1)
#define ARCH_SYSTICK_EN             (0x01 << 0)

#define ARCH_COPROCESSOR_ACCESS_CTL 0xE000ED88

#define ARCH_FPU_CONTEX_CTL         0xE000EF34
#define ARCH_FPU_ASPEN              (0x01UL << 31)
#define ARCH_FPU_LSPEN              (0x01UL << 30)

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
#if ARCH_FPU_USED
    OS_Uint32_t excReturn;
#endif
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
    ARCH_InterruptDisable();
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
#if ARCH_FPU_USED
    taskContext->excReturn = ARCH_EXEC_RETURN;
#endif
    taskContext->R0  = (OS_Uint32_t)TaskParam->PrivateData;
    taskContext->LR  = (OS_Uint32_t)TaskExitErrorEntry;
    taskContext->PC  = (OS_Uint32_t)TaskParam->TaskEntry;
    taskContext->xPSR = ARCH_XPSR_INIT;
    taskContext->R4 =  0x04040404;
}

void ARCH_InterruptDisable(void)
{
    __disable_irq();
}

void ARCH_InterruptEnable(void)
{
    __enable_irq();
}

void ARCH_InterruptInit(void)
{
    OS_REG8(ARCH_PENDSV_PRIO_REG) = ARCH_PENDSV_PRIORITY;
    OS_REG8(ARCH_SYSTICK_PRIO_REG) = ARCH_SYSTICK_PRIORITY;
}

void ARCH_SystemTickInit(void)
{
    OS_REG32(ARCH_SYSTICK_CTL) &= (~ARCH_SYSTICK_EN);
    OS_REG32(ARCH_SYSTICK_RELOAD) = (CONFIG_SYS_CLOCK_RATE / CONFIG_SYS_TICK_RATE_HZ) - 1;
    OS_REG32(ARCH_SYSTICK_CTL) |= (ARCH_SYSTICK_CLK_SRC | ARCH_SYSTICK_INT | ARCH_SYSTICK_EN);
}

void ARCH_MiscInit(void)
{
#if ARCH_FPU_USED
    /* Set CP10 and CP11 full access */
    OS_REG32(ARCH_COPROCESSOR_ACCESS_CTL) |= ((3UL << 10*2) | (3UL << 11*2));
    /* Enable the FPU lazy stacking feature to optimise the RTOS performance */
    OS_REG32(ARCH_FPU_CONTEX_CTL) |= (ARCH_FPU_ASPEN | ARCH_FPU_LSPEN);
#endif
}

__asm void ARCH_StartScheduler(void *TargetTCB)
{
    PRESERVE8

    // Set the 0xE000ED08 to R0(VTOR)
    ldr r0, =0xE000ED08
    // Get the interrupt vector table address
    ldr r0, [r0]
    // Get the first element of the interrupt vector -- MSP
    ldr r0, [r0]

    // Recover the MSP to real MSP because we will never go back ^_^
    msr msp, r0
    // Enable global interrupt(PRIMASK)
    cpsie i
    // Enable fault interrupt(FAULTMASK)
    cpsie f
    dsb
    isb

    nop
}
