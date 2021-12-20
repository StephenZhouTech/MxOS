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
#include "os_trace.h"
#include "os_printk.h"
#include "platform.h"
#include "os_error_code.h"
#include "os_mutex.h"

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

OS_Uint32_t MutexHandle = 0;

#define TASK_1_DELAY        1000
#define TASK_2_DELAY        2000
#define TASK_3_DELAY        3000

/*-----------------------------------------------------
 * Task 1 Priority = 1
 * Task 2 Priority = 2
 * Task 3 Priority = 3
 * ----------------------------------------------------
 * Task 1 get mutex lock for 10s
 * Task 2/3 get mutex lock and sleep
 * Task 1 priority rise to 2/3
 * Task 1 unlock recovery priority to 1
 * Task 3 get mutex lock for 10s
 * Task 1 get mutex lock and sleep
 * Task 3 unlock mutex and wake up task 2
 * Task 2 get mutex lock for 10s
 * Task 2 unlock mutex and wake up task 1
*/

void TASK1_FUNC(void *param)
{
    while(1)
    {
        t1_cnt++;

        OS_API_SchedulerSuspend();
        OS_PRINTK_DEBUG("T1[%04d] CurTime = %d",t1_cnt, OS_GetCurrentTime());
        OS_API_SchedulerResume();

        if (t1_cnt <= 2)
        {
            OS_API_MutexLock(MutexHandle);
            OS_PRINTK_DEBUG("T1[%04d] lock to 5s", t1_cnt);
            OS_API_TaskDelay(5000);
            OS_PRINTK_DEBUG("T1[%04d] to Unlock", t1_cnt);
            OS_API_MutexUnlock(MutexHandle);
        }
        else
        {
            OS_API_TaskDelay(TASK_1_DELAY);
        }
    }
}

void TASK2_FUNC(void *param)
{
    while(1)
    {
        t2_cnt++;

        OS_API_SchedulerSuspend();
        OS_PRINTK_DEBUG("T2[%04d] CurTime = %d",t2_cnt, OS_GetCurrentTime());
        OS_API_SchedulerResume();

        if (t2_cnt == 1)
        {
            OS_API_TaskDelay(1000);

            OS_API_MutexLock(MutexHandle);
            OS_PRINTK_DEBUG("T2[%04d] lock to 5s", t2_cnt);
            OS_API_TaskDelay(5000);
            OS_PRINTK_DEBUG("T2[%04d] to Unlock", t2_cnt);
            OS_API_MutexUnlock(MutexHandle);
        }
        else
        {
            OS_API_TaskDelay(TASK_2_DELAY);
        }
    }
}

void TASK3_FUNC(void *param)
{
    while(1)
    {
        t3_cnt++;

        OS_API_SchedulerSuspend();
        OS_PRINTK_DEBUG("T3[%04d] CurTime = %d",t3_cnt, OS_GetCurrentTime());
        OS_API_SchedulerResume();

        if (t3_cnt == 1)
        {
            OS_API_TaskDelay(1200);

            OS_API_MutexLock(MutexHandle);
            OS_PRINTK_DEBUG("T3[%04d] lock to 5s", t3_cnt);
            OS_API_TaskDelay(5000);
            OS_PRINTK_DEBUG("T3[%04d] to Unlock", t3_cnt);
            OS_API_MutexUnlock(MutexHandle);
        }
        else
        {
            OS_API_TaskDelay(TASK_3_DELAY);
        }
    }
}

int main(void)
{
    PlatformInit();

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

    OS_API_MutexCreate(&MutexHandle);

    OS_API_KernelStart();

    while(1);
}
