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
#include "arch.h"
#include "os_types.h"
#include "os_task.h"
#include "os_error_code.h"
#include "os_configs.h"
#include "os_critical.h"
#include "os_time.h"
#include "os_scheduler.h"
#include "os_mem.h"
#include "os_lib.h"
#include "os_kernel.h"

#include <stdio.h>

extern OS_TaskScheduler_t Scheduler;

OS_Uint32_t task_handle_A0 = 0;
OS_Uint32_t task_handle_A1 = 0;
OS_Uint32_t task_handle_A2 = 0;

OS_Uint32_t task_handle_B0 = 0;
OS_Uint32_t task_handle_B1 = 0;

OS_Uint32_t task_handle_C0 = 0;
OS_Uint32_t task_handle_C1 = 0;

OS_Uint32_t cnt_A0 = 0;
OS_Uint32_t cnt_A1 = 0;
OS_Uint32_t cnt_A2 = 0;

OS_Uint32_t cnt_B0 = 0;
OS_Uint32_t cnt_B1 = 0;

OS_Uint32_t cnt_C0 = 0;
OS_Uint32_t cnt_C1 = 0;

void TC_TASK_FUNC_A0(void *param)
{
    while(1)
    {
        cnt_A0++;
    }
}

void TC_TASK_FUNC_A1(void *param)
{
    while(1)
    {
        cnt_A1++;
    }
}

void TC_TASK_FUNC_A2(void *param)
{
    while(1)
    {
        cnt_A2++;
    }
}

void TC_TASK_FUNC_B0(void *param)
{
    while(1)
    {
        cnt_B0++;
    }
}

void TC_TASK_FUNC_B1(void *param)
{
    while(1)
    {
        cnt_B1++;
    }
}

void TC_TASK_FUNC_C0(void *param)
{
    while(1)
    {
        cnt_C0++;
    }
}

void TC_TASK_FUNC_C1(void *param)
{
    while(1)
    {
        cnt_C1++;
    }
}

extern OS_Uint32_t volatile OS_CurrentTime;

extern void OS_DBG_SCH_DumpReadyList(void);
extern void OS_DBG_SCH_DumpDelayList(void);
extern void OS_DBG_SCH_DumpSuspendList(void);
extern void OS_DBG_SCH_DumpBlockList(void);
extern void OS_DBG_SCH_DumpAllStateList(void);

extern void OS_DBG_TASK_DumpTaskInfo(OS_TCB_t *TaskCB);
extern OS_TCB_t * OS_HighestPrioTaskGet(void);
extern OS_Int16_t OS_IsSchedulerSuspending(void);
extern void OS_API_SchedulerSuspend(void);
extern void OS_API_SchedulerResume(void);
extern OS_Uint8_t OS_CheckTaskInTargetList(OS_TCB_t *TargetTCB, OS_Uint8_t TargetList);

extern void OS_TaskAddToDelayList(OS_TCB_t *TaskCB);
extern void OS_TaskAddToReadyList(OS_TCB_t * TaskCB);
extern void OS_TaskAddToSuspendList(OS_TCB_t * TaskCB);
extern void OS_TaskAddToBlockedList(OS_TCB_t * TaskCB);

extern void OS_RemoveTaskFromReadyList(OS_TCB_t * TaskCB);
extern void OS_RemoveTaskFromDelayList(OS_TCB_t * TaskCB);
extern void OS_RemoveTaskFromSuspendList(OS_TCB_t * TaskCB);
extern void OS_RemoveTaskFromBlockedList(OS_TCB_t * TaskCB);

extern void OS_TaskReadyToDelay(OS_TCB_t * TaskCB);
extern void OS_TaskDelayToReady(OS_TCB_t * TaskCB);

extern void OS_TaskCheckWakeUp(OS_Uint32_t time);
extern void OS_Schedule(void);

typedef enum _DBG_ScheduleResults {
    SCHEDULER_SUSPENDING = 1,
    SCHEDULER_CURRENT_NOT_IN_READY,
    SCHEDULER_NEXT_HIGHER_THAN_CURRENT,
    SCHEDULER_PRIO_EQ_NOT_SAME,
    SCHEDULER_PRIO_EQ_SAME_ONLY_ONE,
    SCHEDULER_PRIO_EQ_SAME_NOT_ONLY_ONE,
} DBG_ScheduleResults_e;

OS_TCB_t * volatile DBG_CurrentTCB = OS_NULL;
OS_TCB_t * volatile DBG_SwitchNextTCB = OS_NULL;

OS_Uint8_t OS_DBG_Schedule(void)
{
    OS_Uint8_t NeedResch = SCHEDULER_SUSPENDING;

    /* Check if the scheduler is suspend */
    if (OS_IsSchedulerSuspending() != 0)
        return NeedResch;

    /* Find the highest priority task now */
    DBG_SwitchNextTCB = OS_HighestPrioTaskGet();

    /* Check DBG_CurrentTCB is in the ready list */
    if (!OS_CheckTaskInTargetList(DBG_CurrentTCB, OS_READY_LIST))
    {
        NeedResch = SCHEDULER_CURRENT_NOT_IN_READY;
        ListMoveTail(&DBG_SwitchNextTCB->StateList, &Scheduler.ReadyListHead[DBG_SwitchNextTCB->Priority]);
        goto _OS_ScheduleRightNow;
    }

    /* Check wich task have the highset prioirty */
    if (DBG_SwitchNextTCB->Priority > DBG_CurrentTCB->Priority)
    {
        NeedResch = SCHEDULER_NEXT_HIGHER_THAN_CURRENT;
        ListMoveTail(&DBG_SwitchNextTCB->StateList, &Scheduler.ReadyListHead[DBG_SwitchNextTCB->Priority]);
    }

    // The next task have the same prioirty as before
    if (DBG_SwitchNextTCB->Priority == DBG_CurrentTCB->Priority)
    {
        // Check the next is the same as before
        if (DBG_SwitchNextTCB != DBG_CurrentTCB)
        {
            NeedResch = SCHEDULER_PRIO_EQ_NOT_SAME;
            // Move the next to the list of highest priority list
            ListMoveTail(&DBG_SwitchNextTCB->StateList, &Scheduler.ReadyListHead[DBG_SwitchNextTCB->Priority]);
        }
        else
        {
            // Only one task in the highest prioirty list
            if (ListIsLast(&DBG_SwitchNextTCB->StateList, &Scheduler.ReadyListHead[DBG_SwitchNextTCB->Priority]))
            {
                NeedResch = SCHEDULER_PRIO_EQ_SAME_ONLY_ONE;
            }
            else
            {
                // more than one task in highest priority task list
                NeedResch = SCHEDULER_PRIO_EQ_SAME_NOT_ONLY_ONE;

                DBG_SwitchNextTCB = ListEntry(DBG_CurrentTCB->StateList.next, OS_TCB_t, StateList);
                ListMoveTail(&DBG_CurrentTCB->StateList, &Scheduler.ReadyListHead[DBG_SwitchNextTCB->Priority]);
                ListMoveTail(&DBG_SwitchNextTCB->StateList, &Scheduler.ReadyListHead[DBG_SwitchNextTCB->Priority]);
            }
        }
    }

_OS_ScheduleRightNow:
    return (NeedResch);
}


OS_Uint32_t TC_Step = 0;

void TC_PASS(void)
{
    printf("*************************************************\r\n");
    printf("[PASS]\r\n");
    printf("*************************************************\r\n");
}

OS_Uint32_t TC_CreateTasks(void)
{
    OS_Uint32_t Ret = OS_SUCCESS;

    OS_Uint32_t TaskInputParam = 1;
    TaskInitParameter Param;
    printf("*************************************************\r\n");
    printf("[TC_%d] : %s\r\n",TC_Step++, __func__);
    printf("*************************************************\r\n");
    OS_Memset(Param.Name, 0x00, CONFIG_TASK_NAME_LEN);
    Param.Name[0] ='A';
    Param.Name[1] ='0';
    Param.Priority = 1;
    Param.PrivateData = &TaskInputParam;
    Param.StackSize = 1024;
    Param.TaskEntry = TC_TASK_FUNC_A0;
    Ret = OS_API_TaskCreate(Param, (void *)&task_handle_A0);
    if (Ret == OS_SUCCESS)
    {
        printf("TaskCreate %s Successful\r\n", Param.Name);
    }
    else
    {
        printf("TaskCreate %s Failed\r\n", Param.Name);
        return Ret;
    }

    Param.Name[0] ='A';
    Param.Name[1] ='1';
    Param.Priority = 1;
    Param.TaskEntry = TC_TASK_FUNC_A1;
    Ret = OS_API_TaskCreate(Param, (void *)&task_handle_A1);
    if (Ret == OS_SUCCESS)
    {
        printf("TaskCreate %s Successful\r\n", Param.Name);
    }
    else
    {
        printf("TaskCreate %s Failed\r\n", Param.Name);
        return Ret;
    }

    Param.Name[0] ='A';
    Param.Name[1] ='2';
    Param.Priority = 1;
    Param.TaskEntry = TC_TASK_FUNC_A2;
    Ret = OS_API_TaskCreate(Param, (void *)&task_handle_A2);
    if (Ret == OS_SUCCESS)
    {
        printf("TaskCreate %s Successful\r\n", Param.Name);
    }
    else
    {
        printf("TaskCreate %s Failed\r\n", Param.Name);
        return Ret;
    }

    Param.Name[0] ='B';
    Param.Name[1] ='0';
    Param.Priority = 15;
    Param.TaskEntry = TC_TASK_FUNC_B0;
    Ret = OS_API_TaskCreate(Param, (void *)&task_handle_B0);
    if (Ret == OS_SUCCESS)
    {
        printf("TaskCreate %s Successful\r\n", Param.Name);
    }
    else
    {
        printf("TaskCreate %s Failed\r\n", Param.Name);
        return Ret;
    }

    Param.Name[0] ='B';
    Param.Name[1] ='1';
    Param.Priority = 15;
    Param.TaskEntry = TC_TASK_FUNC_B1;
    Ret = OS_API_TaskCreate(Param, (void *)&task_handle_B1);
    if (Ret == OS_SUCCESS)
    {
        printf("TaskCreate %s Successful\r\n", Param.Name);
    }
    else
    {
        printf("TaskCreate %s Failed\r\n", Param.Name);
        return Ret;
    }

    Param.Name[0] ='C';
    Param.Name[1] ='0';
    Param.Priority = 31;
    Param.TaskEntry = TC_TASK_FUNC_C0;
    Ret = OS_API_TaskCreate(Param, (void *)&task_handle_C0);
    if (Ret == OS_SUCCESS)
    {
        printf("TaskCreate %s Successful\r\n", Param.Name);
    }
    else
    {
        printf("TaskCreate %s Failed\r\n", Param.Name);
        return Ret;
    }

    Param.Name[0] ='C';
    Param.Name[1] ='1';
    Param.Priority = 31;
    Param.TaskEntry = TC_TASK_FUNC_C1;
    Ret = OS_API_TaskCreate(Param, (void *)&task_handle_C1);
    if (Ret == OS_SUCCESS)
    {
        printf("TaskCreate %s Successful\r\n", Param.Name);
    }
    else
    {
        printf("TaskCreate %s Failed\r\n", Param.Name);
        return Ret;
    }

    Param.Name[0] ='D';
    Param.Name[1] ='1';
    Param.Priority = 32;
    Param.TaskEntry = TC_TASK_FUNC_C1;
    Ret = OS_API_TaskCreate(Param, (void *)&task_handle_C1);
    if (Ret == OS_SUCCESS)
    {
        Ret = 0xFF;
        printf("TaskCreate %s Failed \r\n", Param.Name);
    }
    else
    {
        Ret = OS_SUCCESS;
        printf("TaskCreate %s Successful\r\n", Param.Name);
    }

    OS_DBG_SCH_DumpAllStateList();

    return Ret;
}

OS_TCB_t * TC_HighestPriorityTaskPick(void)
{
    OS_TCB_t *TaskCB = OS_NULL;

    printf("\r\n");
    printf("\r\n");
    printf("*************************************************\r\n");
    printf("[TC_%d] : %s\r\n",TC_Step++, __func__);
    printf("*************************************************\r\n");

    TaskCB = OS_HighestPrioTaskGet();
    OS_DBG_TASK_DumpTaskInfo(TaskCB);

    return TaskCB;
}

void TC_ReadyToReady(void)
{
    // should be stuck here
    printf("[TC_%d] : %s\r\n",TC_Step++, __func__);
    printf("Should stuck here\r\n");
    OS_TaskAddToReadyList((OS_TCB_t *)task_handle_A0);
    printf("[Error]:%d\r\n", __LINE__);
}

void TC_RemoveFromReady(void)
{
    printf("\r\n");
    printf("\r\n");
    printf("*************************************************\r\n");
    printf("[TC_%d] : %s\r\n",TC_Step++, __func__);
    printf("*************************************************\r\n");

    OS_RemoveTaskFromReadyList((OS_TCB_t *) task_handle_A0);
    OS_RemoveTaskFromReadyList((OS_TCB_t *) task_handle_B0);
    OS_RemoveTaskFromReadyList((OS_TCB_t *) task_handle_B1);
    OS_RemoveTaskFromReadyList((OS_TCB_t *) task_handle_C1);

    OS_DBG_SCH_DumpAllStateList();
}

void TC_RemoveAlreadyRemoved(void)
{
    printf("[TC_%d] : %s\r\n",TC_Step++, __func__);
    printf("Should stuck here\r\n");
    OS_RemoveTaskFromReadyList((OS_TCB_t *) task_handle_A0);
    printf("[Error]:%d\r\n", __LINE__);
}

void TC_AddToDelay(void)
{
    printf("\r\n");
    printf("\r\n");
    printf("*************************************************\r\n");
    printf("[TC_%d] : %s\r\n",TC_Step++, __func__);
    printf("*************************************************\r\n");

    OS_TCB_t *TaskCB = (OS_TCB_t *)task_handle_A0;
    TaskCB->WakeUpTime = 0x00F0;
    OS_TaskAddToDelayList(TaskCB);

    TaskCB = (OS_TCB_t *)task_handle_B0;
    TaskCB->WakeUpTime = 0x0100;
    OS_TaskAddToDelayList(TaskCB);

    TaskCB = (OS_TCB_t *)task_handle_B1;
    TaskCB->WakeUpTime = 0x0030;
    OS_TaskAddToDelayList(TaskCB);

    TaskCB = (OS_TCB_t *)task_handle_C1;
    TaskCB->WakeUpTime = 0x00F0;
    OS_TaskAddToDelayList(TaskCB);

    OS_DBG_SCH_DumpAllStateList();
}

void TC_DelayToDelay(void)
{
    printf("[TC_%d] : %s\r\n",TC_Step++, __func__);
    printf("Should stuck here\r\n");
    OS_TaskAddToDelayList((OS_TCB_t *)task_handle_B0);
    printf("[Error]:%d\r\n", __LINE__);
}

void TC_WakeUpFromDelay(void)
{
    printf("\r\n");
    printf("\r\n");
    printf("*************************************************\r\n");
    printf("[TC_%d] : %s\r\n",TC_Step++, __func__);
    printf("*************************************************\r\n");

    printf("Set Current Time = 0x29\r\n");
    OS_TaskCheckWakeUp(0x29);
    OS_DBG_SCH_DumpAllStateList();

    printf("Set Current Time = 0x30\r\n");
    OS_TaskCheckWakeUp(0x30);
    OS_DBG_SCH_DumpAllStateList();

    printf("Set Current Time = 0xFF\r\n");
    OS_TaskCheckWakeUp(0xFF);
    OS_DBG_SCH_DumpAllStateList();

    printf("Set Current Time = 0x200\r\n");
    OS_TaskCheckWakeUp(0x200);
    OS_DBG_SCH_DumpAllStateList();
}

void SCH_TC_Entry(void)
{
    OS_Uint8_t Sch_Ret = 0;
    OS_Uint32_t Ret = OS_SUCCESS;
    OS_TCB_t * TaskCB = OS_NULL;

    printf("**********************************************************\r\n");
    printf("********************* SCH TC Entry ***********************\r\n");
    printf("**********************************************************\r\n");

    OS_API_KernelInit();

    /*
     * ------------------------------
     * A0/A1/A2 Priority=1      Ready
     * B0/B1    Priority=15     Ready
     * C0/C1    Priority=31     Ready
     * ------------------------------
     */
    Ret = TC_CreateTasks();
    if (Ret != OS_SUCCESS)
    {
        printf("[Error] : Create Tasks Failed\r\n");
        while(1);
    }
    TC_PASS();

    TaskCB = TC_HighestPriorityTaskPick();
    if ((void *)TaskCB != (void *)task_handle_C1)
    {
        printf("[Error] : Get highest priority Failed\r\n");
        while(1);
    }
    TC_PASS();
    /* NOTE : Should Stuck here */
    //TC_ReadyToReady();

    /* Remove from Ready list*/
    /*
     * ------------------------------
     * A1/A2    Priority=1      Ready
     * C0       Priority=31     Ready
     * ------------------------------
     * A0       Priority=1      Unkonw
     * B0       Priority=15     Unkonw
     * B1       Priority=15     Unkonw
     * C1       Priority=31     Unkonw
     * ------------------------------
     */
    TC_RemoveFromReady();
    if (OS_CheckTaskInTargetList((OS_TCB_t *)task_handle_A0, OS_READY_LIST))
    {
        printf("[Error] : A0 still in ready list\r\n");
        while(1);
    }
    if (OS_CheckTaskInTargetList((OS_TCB_t *)task_handle_B0, OS_READY_LIST))
    {
        printf("[Error] : B0 still in ready list\r\n");
        while(1);
    }
    if (OS_CheckTaskInTargetList((OS_TCB_t *)task_handle_B1, OS_READY_LIST))
    {
        printf("[Error] : B1 still in ready list\r\n");
        while(1);
    }
    if (OS_CheckTaskInTargetList((OS_TCB_t *)task_handle_C1, OS_READY_LIST))
    {
        printf("[Error] : C1 still in ready list\r\n");
        while(1);
    }
    TC_PASS();

    TaskCB = TC_HighestPriorityTaskPick();
    if ((void *)TaskCB != (void *)task_handle_C0)
    {
        printf("[Error] : Get highest priority Failed\r\n");
        while(1);
    }
    TC_PASS();

    /* NOTE : Should Stuck here */
    //TC_RemoveAlreadyRemoved();

    /*
     * ------------------------------
     * A1/A2    Priority=1      Ready
     * C0       Priority=31     Ready
     * ------------------------------
     * A0       Priority=1      Delay
     * B0       Priority=15     Delay
     * B1       Priority=15     Delay
     * C1       Priority=31     Delay
     * ------------------------------
     */
    TC_AddToDelay();
    if (!OS_CheckTaskInTargetList((OS_TCB_t *)task_handle_A0, OS_DELAY_LIST))
    {
        printf("[Error] : A0 not in delay list\r\n");
        while(1);
    }
    if (!OS_CheckTaskInTargetList((OS_TCB_t *)task_handle_B0, OS_DELAY_LIST))
    {
        printf("[Error] : B0 not in delay list\r\n");
        while(1);
    }
    if (!OS_CheckTaskInTargetList((OS_TCB_t *)task_handle_B1, OS_DELAY_LIST))
    {
        printf("[Error] : B1 not in delay list\r\n");
        while(1);
    }
    if (!OS_CheckTaskInTargetList((OS_TCB_t *)task_handle_C1, OS_DELAY_LIST))
    {
        printf("[Error] : C1 not in delay list\r\n");
        while(1);
    }
    TC_PASS();

    /* NOTE : Should Stuck here */
    //TC_DelayToDelay();

    /*
     * ------------------------------
     * A0/A1/A2 Priority=1      Ready
     * B0/B1    Priority=15     Ready
     * C0/C1    Priority=31     Ready
     * ------------------------------
     */
    TC_WakeUpFromDelay();
    if (!OS_CheckTaskInTargetList((OS_TCB_t *)task_handle_A0, OS_READY_LIST))
    {
        printf("[Error] : A0 not in ready list\r\n");
        while(1);
    }
    if (!OS_CheckTaskInTargetList((OS_TCB_t *)task_handle_B0, OS_READY_LIST))
    {
        printf("[Error] : B0 not in ready list\r\n");
        while(1);
    }
    if (!OS_CheckTaskInTargetList((OS_TCB_t *)task_handle_B1, OS_READY_LIST))
    {
        printf("[Error] : B1 not in ready list\r\n");
        while(1);
    }
    if (!OS_CheckTaskInTargetList((OS_TCB_t *)task_handle_C1, OS_READY_LIST))
    {
        printf("[Error] : C1 not in ready list\r\n");
        while(1);
    }
    TC_PASS();

    /* Scheduler Test Case */
    DBG_CurrentTCB = (OS_TCB_t *)task_handle_C1;
    Sch_Ret = OS_DBG_Schedule();
    if (Sch_Ret == SCHEDULER_PRIO_EQ_SAME_NOT_ONLY_ONE && ((OS_Uint32_t)DBG_SwitchNextTCB == task_handle_C0) )
    {
        printf("[1] : Schedule OK\r\n");
    }
    else
    {
        printf("[ERROR] : Schedule[1]\r\n");
        while(1);
    }
    DBG_CurrentTCB = DBG_SwitchNextTCB;

    Sch_Ret = OS_DBG_Schedule();
    if (Sch_Ret == SCHEDULER_PRIO_EQ_NOT_SAME && ((OS_Uint32_t)DBG_SwitchNextTCB == task_handle_C1))
    {
        printf("[2] : Schedule OK\r\n");
    }
    else
    {
        printf("[ERROR] : Schedule[2]\r\n");
        while(1);
    }
    DBG_CurrentTCB = DBG_SwitchNextTCB;

    OS_RemoveTaskFromReadyList((OS_TCB_t *) task_handle_C1);
    Sch_Ret = OS_DBG_Schedule();
    if (Sch_Ret == SCHEDULER_CURRENT_NOT_IN_READY && ((OS_Uint32_t)DBG_SwitchNextTCB == task_handle_C0))
    {
        printf("[3] : Schedule OK\r\n");
    }
    else
    {
        printf("[ERROR] : Schedule[3]\r\n");
        while(1);
    }
    DBG_CurrentTCB = DBG_SwitchNextTCB;

    OS_API_SchedulerSuspend();
    Sch_Ret = OS_DBG_Schedule();
    if (Sch_Ret == SCHEDULER_SUSPENDING)
    {
        printf("[4] : Schedule OK\r\n");
    }
    else
    {
        printf("[ERROR] : Schedule[4]\r\n");
        while(1);
    }
    OS_API_SchedulerResume();

    OS_RemoveTaskFromReadyList((OS_TCB_t *) task_handle_B0);
    OS_RemoveTaskFromReadyList((OS_TCB_t *) task_handle_B1);
    OS_RemoveTaskFromReadyList((OS_TCB_t *) task_handle_C0);

    Sch_Ret = OS_DBG_Schedule();
    if (Sch_Ret == SCHEDULER_CURRENT_NOT_IN_READY && ((OS_Uint32_t)DBG_SwitchNextTCB == task_handle_A0))
    {
        printf("[5] : Schedule OK\r\n");
    }
    else
    {
        printf("[ERROR] : Schedule[5]\r\n");
        while(1);
    }
    DBG_CurrentTCB = DBG_SwitchNextTCB;

    OS_TaskAddToReadyList((OS_TCB_t *) task_handle_C0);
    Sch_Ret = OS_DBG_Schedule();
    if (Sch_Ret == SCHEDULER_NEXT_HIGHER_THAN_CURRENT && ((OS_Uint32_t)DBG_SwitchNextTCB == task_handle_C0))
    {
        printf("[6] : Schedule OK\r\n");
    }
    else
    {
        printf("[ERROR] : Schedule[6]\r\n");
        while(1);
    }
    DBG_CurrentTCB = DBG_SwitchNextTCB;

    Sch_Ret = OS_DBG_Schedule();
    if (Sch_Ret == SCHEDULER_PRIO_EQ_SAME_ONLY_ONE && ((OS_Uint32_t)DBG_SwitchNextTCB == task_handle_C0))
    {
        printf("[7] : Schedule OK\r\n");
    }
    else
    {
        printf("[ERROR] : Schedule[7]\r\n");
        while(1);
    }
    DBG_CurrentTCB = DBG_SwitchNextTCB;

    // OS_API_KernelStart();

    printf("********************* SCH TC Finished ***********************\r\n");
}

