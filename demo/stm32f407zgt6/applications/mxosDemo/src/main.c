#include "stm32f4xx.h"
#include "delay.h"
#include "log.h"
#include "led.h"
#include "os_mem.h"
#include "os_list.h"
#include "os_lib.h"
#include "os_task_sch.h"

void _delay(uint32_t cnt)
{
    while(cnt-- != 0);
}

OS_Uint32_t task1_handle = 0;
OS_Uint32_t task2_handle = 0;

OS_Uint32_t t1_cnt = 0;
OS_Uint32_t t2_cnt = 0;

void TASK1_FUNC(void *param)
{
    while(1)
    {
        t1_cnt++;
    }
}

void TASK2_FUNC(void *param)
{
    while(1)
    {
        t2_cnt++;
    }
}
int main(void)
{
    LogInit();

    BspLedInit();

    LOG_DEBUG("**********************************************************");
    LOG_DEBUG("************** MxOS Project Running ....****************");
    LOG_DEBUG("**********************************************************");

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

    Param.Name[0] ='m';
    Param.Name[1] ='x';
    Param.TaskEntry = TASK2_FUNC;
    OS_API_TaskCreate(Param, (void *)&task2_handle);

    OS_API_KernelStart();

    while(1)
    {
        _delay(100000000);
        BspLEDOn(LED_2 | LED_3 | LED_4);
        _delay(100000000);
        BspLEDOff(LED_2 | LED_3 | LED_4);
    }
}
