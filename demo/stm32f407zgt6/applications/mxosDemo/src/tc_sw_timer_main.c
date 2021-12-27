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
#include "os_sw_timer.h"

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

OS_Uint32_t SwTimerHandle1 = 0;
OS_Uint32_t SwTimerHandle2 = 0;

#define TASK_1_DELAY        1000
#define TASK_2_DELAY        2000
#define TASK_3_DELAY        3000

#define SW_TIMER_INTERVAL_1 1500
#define SW_TIMER_INTERVAL_2 4000

void TASK1_FUNC(void *param)
{
    while(1)
    {
        t1_cnt++;

        OS_API_SchedulerSuspend();
        printf("T1[%04d] CurTime = %d\r\n",t1_cnt, OS_GetCurrentTime());
        OS_API_SchedulerResume();

        OS_API_TaskDelay(TASK_1_DELAY);
    }
}

void TASK2_FUNC(void *param)
{
    while(1)
    {
        t2_cnt++;

        OS_API_SchedulerSuspend();
        printf("T2[%04d] CurTime = %d\r\n",t2_cnt, OS_GetCurrentTime());
        OS_API_SchedulerResume();

        OS_API_TaskDelay(TASK_2_DELAY);
    }
}

void TASK3_FUNC(void *param)
{
    OS_Uint32_t Ret = OS_SUCCESS;

    while(1)
    {
        t3_cnt++;

        OS_API_SchedulerSuspend();
        printf("T3[%04d] CurTime = %d\r\n",t3_cnt, OS_GetCurrentTime());
        OS_API_SchedulerResume();

        if (t3_cnt == 5)
        {
            printf("T3 Start Timer\r\n");
            Ret = OS_API_SwTimerStart(SwTimerHandle1);
            if (Ret == OS_SUCCESS)
            {
                printf("[OK] : T3 Start Timer 1 Interval = [%d]\r\n", SW_TIMER_INTERVAL_1);
            }
            else
            {
                printf("[ERROR] : T3 Start Timer 1 Failed\r\n");
            }

            Ret = OS_API_SwTimerStart(SwTimerHandle2);
            if (Ret == OS_SUCCESS)
            {
                printf("[OK] : T3 Start Timer 2 Interval = [%d]\r\n", SW_TIMER_INTERVAL_2);
            }
            else
            {
                printf("[ERROR] : T3 Start Timer 2 Failed\r\n");
            }
        }

        if (t3_cnt == 10)
        {
            Ret = OS_API_SwTimerStop(SwTimerHandle1);
            if (Ret == OS_SUCCESS)
            {
                printf("[OK] : T3 Stop Timer 1 OK !\r\n");
            }
            else
            {
                printf("[ERROR] : T3 Stop Timer 1 Failed\r\n");
            }

            Ret = OS_API_SwTimerStop(SwTimerHandle2);
            if (Ret == OS_SW_TIMER_NOT_RUNNING)
            {
                printf("[OK] : T3 Stop Timer 2 already stopped !\r\n");
            }
            else
            {
                printf("[ERROR] : T3 Stop Timer 2 Failed\r\n");
            }
        }

        if (t3_cnt == 15)
        {
            Ret = OS_API_SwTimerStart(SwTimerHandle1);
            if (Ret == OS_SUCCESS)
            {
                printf("[OK] : T3 Restart Timer 1 OK !\r\n");
            }
            else
            {
                printf("[ERROR] : T3 Restart Timer 1 Failed\r\n");
            }

            Ret = OS_API_SwTimerDelete(SwTimerHandle2);
            if (Ret == OS_SUCCESS)
            {
                printf("[OK] : T3 Delete Timer 2 OK !\r\n");
            }
            else
            {
                printf("[ERROR] : T3 Delete Timer 2 Failed\r\n");
            }
        }

        if (t3_cnt == 20)
        {
            Ret = OS_API_SwTimerDelete(SwTimerHandle1);
            if (Ret == OS_SUCCESS)
            {
                printf("[OK] : T3 Delete Timer 1 OK !\r\n");
            }
            else
            {
                printf("[ERROR] : T3 Delete Timer 1 Failed\r\n");
            }
        }

        OS_API_TaskDelay(TASK_3_DELAY);

    }
}

void AutoReloadTimerHandler(void *Param)
{
    printf("[Auto Reload] : [%d]\r\n", OS_GetCurrentTime());
}

void OneShotTimerHandler(void *Param)
{
    printf("[One Shot] : [%d]\r\n", OS_GetCurrentTime());
}

int main(void)
{
    PlatformInit();

    OS_API_KernelInit();

    OS_Uint32_t TaskInputParam = 1;
    TaskInitParameter Param;
    OS_Memset(Param.Name, 0x00, CONFIG_TASK_NAME_LEN);
    Param.Name[0] ='T';
    Param.Name[1] ='1';
    Param.Priority = 1;
    Param.PrivateData = &TaskInputParam;
    Param.StackSize = 1024;
    Param.TaskEntry = TASK1_FUNC;
    OS_API_TaskCreate(Param, (void *)&task1_handle);

    Param.Name[0] ='T';
    Param.Name[1] ='2';
    Param.Priority = 2;
    Param.TaskEntry = TASK2_FUNC;
    OS_API_TaskCreate(Param, (void *)&task2_handle);

    Param.Name[0] ='T';
    Param.Name[1] ='3';
    Param.Priority = 3;
    Param.TaskEntry = TASK3_FUNC;
    OS_API_TaskCreate(Param, (void *)&task3_handle);

    OS_API_SwTimerCreate(&SwTimerHandle1,
                        OS_SW_TIMER_AUTO_RELOAD,
                        1500,
                        AutoReloadTimerHandler,
                        OS_NULL);

    OS_API_SwTimerCreate(&SwTimerHandle2,
                        OS_SW_TIMER_ONESHOT,
                        4000,
                        OneShotTimerHandler,
                        OS_NULL);

    OS_API_KernelStart();

    while(1);
}
