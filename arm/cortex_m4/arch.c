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

#define ARCH_NVIC_INT_CTL           0xE000ED04
#define ARCH_PENDSV_SET             (0x01UL << 28)

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

void *ARCH_PrepareStack(void *StartOfStack, void *Param)
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
    taskContext->R4 =  0x44444444;

    return (void *)taskContext;
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

void ARCH_TriggerContextSwitch(void *_CurrentTCB, void *_NextTCB)
{
    OS_REG32(ARCH_NVIC_INT_CTL) |= (ARCH_PENDSV_SET);
}

void ARCH_SystemTickHander(void)
{
    OS_SystemTickHander();
}

// __asm void ARCH_PendSVHandler(void)
// {
//     extern CurrentTCB;
//     extern SwitchNextTCB;

//     PRESERVE8

//     MRS     R0, PSP
//     ISB

//     /* Check if the current task is using VFP. */
//     TST     R14, #0x10
//     IT      EQ
//     /* Using the lazy stacking, just store S16-S31 */
//     VSTMDBEQ    R0!, {S16-S31}
//     /* Store core registers */
//     STMDB   R0!, {R4-R11, R14}
//     /* Get the location of the current TCB. */
//     LDR     R3, =CurrentTCB
//     /* Now the R2 store the stack pointer of TCB */
//     LDR     R2, [R3]
//     /* Update the Current TCB stack pointer */
//     STR     R0, [R2]

//     /* Get the next TCB stack pointer */
//     LDR     R3, =SwitchNextTCB
//     LDR     R3, [R3]
//     LDR     R0, [R3]

//     /* Pop the core registers. */
//     LDMIA   R0!, {R4-R11, R14}
//     /* Check if the next task is using VFP. before */
//     TST     R14, #0x10
//     IT      EQ
//     VLDMIAEQ R0!, {S16-S31}

//     /* Set the next PSP */
//     MSR     PSP, R0

//     ISB

//     /* Update CurrentTCB */
//     LDR     R0, =CurrentTCB
//     STR     R3, [R0]

//     BX      R14
// }

__asm void ARCH_PendSVHandler(void)
{
    extern CurrentTCB;
    extern OS_TaskSwitchContext;

    PRESERVE8

    MRS     R0, PSP
    ISB

    /* Check if the current task is using VFP. */
    TST     R14, #0x10
    IT      EQ
    /* Using the lazy stacking, just store S16-S31 */
    VSTMDBEQ    R0!, {S16-S31}
    /* Store core registers */
    STMDB   R0!, {R4-R11, R14}
    /* Get the location of the current TCB. */
    LDR     R3, =CurrentTCB
    /* Now the R2 store the stack pointer of TCB */
    LDR     R2, [R3]
    /* Update the Current TCB stack pointer */
    STR     R0, [R2]


    STMDB   SP!, {R0, R3}
    DSB
    ISB
    BL      OS_TaskSwitchContext
    LDMIA   SP!, {R0, R3}

    /* Get the next TCB stack pointer */
    LDR     R1, [R3]
    LDR     R0, [R1]

    /* Pop the core registers. */
    LDMIA   R0!, {R4-R11, R14}
    /* Check if the next task is using VFP. before */
    TST     R14, #0x10
    IT      EQ
    VLDMIAEQ R0!, {S16-S31}

    /* Set the next PSP */
    MSR     PSP, R0

    ISB

    BX      R14
}

__asm void ARCH_ChangeToUserMode(void)
{
    PRESERVE8
    PUSH    {R4,LR}
    MOV     R4, #3
    MSR     CONTROL, R4
    POP     {R4,PC}
}

__asm void ARCH_StartScheduler(void *TargetTCB)
{
    PRESERVE8

    // Set the 0xE000ED08 to R0(VTOR)
    LDR     R1, =0xE000ED08
    // Get the interrupt vector table address
    LDR     R1, [R1]
    // Get the first element of the interrupt vector -- MSP
    LDR     R1, [R1]
    // Recover the MSP to real MSP top because we will never go back ^_^
    MSR     MSP, R1

    // Get the task stack pointer address of target TCB
    LDR     R0, [R0]
    // Set the PC relative location and the top of the stack
#if ARCH_FPU_USED
    MOV     R1, #60
    MOV     R2, #64
#else
    MOV     R1, #56
    MOV     R2, #60
#endif

    // Reset highest task's PSP at the top of the task stack
    ADDS    R2, R0, R2
    MSR     PSP, R2

    // Now the R1 is the first task entry
    ADDS    R1, R0, R1
    LDR     R1, [R1]

    DSB
    ISB

    // Set CONTROL As 0x03 to use PSP and run in thread mode
    MOV     R2, #2
    MSR     CONTROL, R2

    BX      R1
    NOP
    NOP
}
