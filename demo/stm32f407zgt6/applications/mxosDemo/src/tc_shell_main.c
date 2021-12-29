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

#define TASK_1_DELAY        1000
#define TASK_2_DELAY        2000
#define TASK_3_DELAY        3000

void TASK1_FUNC(void *param)
{
    while(1)
    {
        t1_cnt++;

        OS_API_TaskDelay(TASK_1_DELAY);
    }
}

void TASK2_FUNC(void *param)
{
    while(1)
    {
        t2_cnt++;

        OS_API_TaskDelay(TASK_2_DELAY);
    }
}

void TASK3_FUNC(void *param)
{
    while(1)
    {
        t3_cnt++;

        OS_API_TaskDelay(TASK_3_DELAY);

    }
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

    OS_API_KernelStart();

    while(1);
}
