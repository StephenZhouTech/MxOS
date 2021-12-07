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

#define TASK_1_DELAY        1000
#define TASK_2_DELAY        2000
#define TASK_3_DELAY        3000

void TASK1_FUNC(void *param)
{
    while(1)
    {
        OS_API_SchedulerSuspend();
        printf("Task 1 CurTime = %d\r\n", OS_GetCurrentTime());
        OS_API_SchedulerResume();

        OS_API_TaskDelay(TASK_1_DELAY/2);
        BspLEDOff(LED_2);
        OS_API_TaskDelay(TASK_1_DELAY/2);
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

        OS_API_TaskDelay(TASK_2_DELAY/2);
        BspLEDOn(LED_3);
        OS_API_TaskDelay(TASK_2_DELAY/2);
        BspLEDOff(LED_3);
    }
}

void TASK3_FUNC(void *param)
{
    while(1)
    {
        OS_API_SchedulerSuspend();
        printf("Task 3 CurTime = %d\r\n", OS_GetCurrentTime());
        OS_API_SchedulerResume();

        OS_API_TaskDelay(TASK_3_DELAY/2);
        BspLEDOn(LED_4);
        OS_API_TaskDelay(TASK_3_DELAY/2);
        BspLEDOff(LED_4);
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
