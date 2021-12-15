#include "stm32f4xx.h"
#include "log.h"
#include "led.h"
#include "os_mem.h"
#include "os_list.h"
#include "os_lib.h"
#include "os_task.h"
#include "os_kernel.h"
#include "os_scheduler.h"
#include "os_critical.h"
#include "os_time.h"
#include "os_printk.h"
#include "platform.h"
#include "os_error_code.h"

/* Test Case For OS scheduler */
#define TC_SCHEDULER_IN_MAIN        0
/* Test Case For OS memory manager */
#define TC_MEM_IN_MAIN              0

#define DEMO_SHOW                   1

void _delay(uint32_t cnt)
{
    while(cnt-- != 0);
}

OS_Uint32_t task1_handle = 0;
OS_Uint32_t task2_handle = 0;
OS_Uint32_t task3_handle = 0;

OS_Uint32_t t1_cnt = 0;
OS_Uint32_t t2_cnt = 0;
OS_Uint32_t t3_cnt = 0;

#define TASK_1_DELAY        1000
#define TASK_2_DELAY        2000
#define TASK_3_DELAY        3000

void TASK1_FUNC(void *param)
{
    OS_Uint32_t Ret = OS_SUCCESS;

    while(1)
    {
        t1_cnt++;
        OS_API_SchedulerSuspend();
        printf("Task 1 CurTime = %d\r\n", OS_GetCurrentTime());
        OS_API_SchedulerResume();

        /* Suspend task 2 and task 3 */
        if (t1_cnt == 5)
        {
            OS_API_TaskSuspend(task2_handle);
            OS_API_TaskSuspend(task3_handle);
            printf("T1 Suspend 2/3\r\n");
        }

        /* Resume task 2 and task 3 */
        if (t1_cnt == 10)
        {
            printf("T1 Resume 2/3\r\n");
            OS_API_TaskResume(task2_handle);
            OS_API_TaskResume(task3_handle);
        }

        /* Suspend task 2 and task 3 with scheduler suspend */
        if (t1_cnt == 15)
        {
            OS_API_SchedulerSuspend();
            OS_API_TaskSuspend(task2_handle);
            OS_API_TaskSuspend(task3_handle);
            printf("T1 Suspend 2/3(SchPend)\r\n");
            OS_API_SchedulerResume();
        }

        /* Resume task 2 and task 3 with scheduler suspend */
        if (t1_cnt == 20)
        {
            OS_API_SchedulerSuspend();
            printf("T1 Resume 2/3(SchPend)\r\n");
            OS_API_TaskResume(task2_handle);
            OS_API_TaskResume(task3_handle);
            OS_API_SchedulerResume();
        }

        /* Resume task 1 itself */
        if (t1_cnt == 25)
        {
            Ret = OS_API_TaskResume(task1_handle);
            printf("T1 Resume T1:[%d]\r\n", Ret);
        }

        /* Resume task 1 itself with scheduler suspend */
        if (t1_cnt == 30)
        {
            OS_API_SchedulerSuspend();
            Ret = OS_API_TaskResume(task1_handle);
            printf("T1 Resume T1(SchPend):[%d]\r\n", Ret);
            OS_API_SchedulerResume();
        }

        /* Suspend task 1 itself with scheduler suspend */
        if (t1_cnt == 35)
        {
            OS_API_SchedulerSuspend();
            Ret = OS_API_TaskSuspend(task1_handle);
            printf("T1 Suspend T1(SchPend):[%d]\r\n", Ret);
            OS_API_SchedulerResume();
        }

        /* Suspend task 1 itself */
        if (t1_cnt == 40)
        {
            printf("T1 Suspend T1\r\n");
            OS_API_TaskSuspend(task1_handle);
        }

        /* T1 Get priorty */
        if (t1_cnt == 42)
        {
            printf("T1 Get T1/T2/T3 Prio:[%d/%d/%d]\r\n",
                            OS_API_TaskPriorityGet(task1_handle),
                            OS_API_TaskPriorityGet(task2_handle),
                            OS_API_TaskPriorityGet(task3_handle));
        }

        /* Change T1 priorty to the same as T1 */
        if (t1_cnt == 45)
        {
            Ret = OS_API_TaskPrioritySet(task1_handle, 1);
            printf("T1 Set T1 Prio to same :[%d]\r\n", Ret);
        }

        /* Raise T1 priorty to highest */
        if (t1_cnt == 50)
        {
            printf("T1 Set T1 Prio to highest\r\n");
            Ret = OS_API_TaskPrioritySet(task1_handle, 4);
        }

        /* T1 Get priorty */
        if (t1_cnt == 55)
        {
            printf("T1 Get T1/T2/T3 Prio:[%d/%d/%d]\r\n",
                            OS_API_TaskPriorityGet(task1_handle),
                            OS_API_TaskPriorityGet(task2_handle),
                            OS_API_TaskPriorityGet(task3_handle));
        }

        /* Down T1 priorty to highest */
        if (t1_cnt == 60)
        {
            printf("T1 Set T1 Prio to lowset\r\n");
            Ret = OS_API_TaskPrioritySet(task1_handle, 1);
        }

        /* T1 Get priorty */
        if (t1_cnt == 65)
        {
            printf("T1 Get T1/T2/T3 Prio:[%d/%d/%d]\r\n",
                            OS_API_TaskPriorityGet(task1_handle),
                            OS_API_TaskPriorityGet(task2_handle),
                            OS_API_TaskPriorityGet(task3_handle));
        }

        /* Raise T2 priorty to highest */
        if (t1_cnt == 70)
        {
            printf("T1 Set T2 Prio to highest\r\n");
            OS_API_TaskPrioritySet(task2_handle, 4);
        }

        /* T1 Get priorty */
        if (t1_cnt == 75)
        {
            printf("T1 Get T1/T2/T3 Prio:[%d/%d/%d]\r\n",
                            OS_API_TaskPriorityGet(task1_handle),
                            OS_API_TaskPriorityGet(task2_handle),
                            OS_API_TaskPriorityGet(task3_handle));
        }

        /* Down T2 priorty to highest */
        if (t1_cnt == 80)
        {
            printf("T1 Set T2 Prio to before\r\n");
            Ret = OS_API_TaskPrioritySet(task2_handle, 2);
        }

        /* T1 Get priorty */
        if (t1_cnt == 85)
        {
            printf("T1 Get T1/T2/T3 Prio:[%d/%d/%d]\r\n",
                            OS_API_TaskPriorityGet(task1_handle),
                            OS_API_TaskPriorityGet(task2_handle),
                            OS_API_TaskPriorityGet(task3_handle));
        }

        /* Suspend T3 */
        if (t1_cnt == 87)
        {
            printf("T1 Suspend T3\r\n");
            OS_API_TaskSuspend(task3_handle);
        }

        /* Set T3 priorty */
        if (t1_cnt == 90)
        {
            printf("T1 Set T3 Prio to 15\r\n");
            OS_API_TaskPrioritySet(task3_handle, 15);
        }

        /* T1 Get priorty */
        if (t1_cnt == 95)
        {
            printf("T1 Get T1/T2/T3 Prio:[%d/%d/%d]\r\n",
                            OS_API_TaskPriorityGet(task1_handle),
                            OS_API_TaskPriorityGet(task2_handle),
                            OS_API_TaskPriorityGet(task3_handle));
        }

        if (t1_cnt == 96)
        {
            printf("############## ^_^ ##############\r\n");
        }

        OS_API_TaskDelay(TASK_1_DELAY);
        BspLEDOff(LED_2);
        BspLEDOn(LED_2);
    }
}

void TASK2_FUNC(void *param)
{
    while(1)
    {
        OS_API_SchedulerSuspend();
        printf("Task 2 CurTime = %d\r\n", OS_GetCurrentTime());
        OS_API_SchedulerResume();

        if (t1_cnt >= 40)
        {
            t2_cnt++;
        }

        if (t2_cnt == 5)
        {
            printf("T2 Resume T1\r\n");
            OS_API_TaskResume(task1_handle);
        }

        OS_API_TaskDelay(TASK_2_DELAY);
        BspLEDOff(LED_3);
        BspLEDOn(LED_3);
    }
}

void TASK3_FUNC(void *param)
{
    while(1)
    {
        OS_API_SchedulerSuspend();
        printf("Task 3 CurTime = %d\r\n", OS_GetCurrentTime());
        OS_API_SchedulerResume();

        OS_API_TaskDelay(TASK_3_DELAY);
        BspLEDOff(LED_4);
        BspLEDOn(LED_4);
    }
}


extern void SCH_TC_Entry(void);
extern void MEM_TC_Entry(void);

int main(void)
{
    PlatformInit();

#if TC_MEM_IN_MAIN
    MEM_TC_Entry();
#endif

#if TC_SCHEDULER_IN_MAIN
    SCH_TC_Entry();
#endif

#if DEMO_SHOW
    OS_API_KernelInit();

    OS_Uint32_t TaskInputParam = 1;
    TaskInitParameter Param;
    OS_Memset(Param.Name, 0x00, CONFIG_TASK_NAME_LEN);
    Param.Name[0] ='z';
    Param.Name[1] ='t';
    Param.Priority = 1;
    Param.PrivateData = &TaskInputParam;
    Param.StackSize = 1024;
    Param.TaskEntry = TASK1_FUNC;
    OS_API_TaskCreate(Param, (void *)&task1_handle);

    Param.Name[0] ='y';
    Param.Name[1] ='c';
    Param.Priority = 2;
    Param.TaskEntry = TASK2_FUNC;
    OS_API_TaskCreate(Param, (void *)&task2_handle);

    Param.Name[0] ='m';
    Param.Name[1] ='x';
    Param.Priority = 3;
    Param.TaskEntry = TASK3_FUNC;
    OS_API_TaskCreate(Param, (void *)&task3_handle);

    OS_API_KernelStart();
#endif

    while(1)
    {
        _delay(10000000);
        BspLEDOn(LED_2 | LED_3 | LED_4);
        _delay(10000000);
        BspLEDOff(LED_2 | LED_3 | LED_4);
    }
}
