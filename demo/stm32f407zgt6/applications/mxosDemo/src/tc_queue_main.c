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
#include "os_queue.h"

void _delay(uint32_t cnt)
{
    while(cnt-- != 0);
}

/* Only choose one */
#define TC_QUEUE_READ_SLEEP                 0
#define TC_QUEUE_WRITE_SLEEP                0
#define TC_QUEUE_READ_SLEEP_TIMEOUT         0
#define TC_QUEUE_WRITE_SLEEP_TIMEOUT        1

OS_Uint32_t task1_handle = 0;
OS_Uint32_t task2_handle = 0;
OS_Uint32_t task3_handle = 0;

OS_Uint32_t t1_cnt = 0;
OS_Uint32_t t2_cnt = 0;
OS_Uint32_t t3_cnt = 0;

OS_Uint32_t QueueHandle = 0;

#define QUEUE_ELEMENT_SZ        32
#define QUEUE_ELEMENT_NR        3

#define TASK_1_DELAY        1000
#define TASK_2_DELAY        5000
#define TASK_3_DELAY        3000

OS_Uint8_t ReadBuf[QUEUE_ELEMENT_SZ];
OS_Uint8_t WriteBuf[QUEUE_ELEMENT_SZ];

OS_Uint8_t TC_MemorySame(OS_Uint8_t *Mem_1, OS_Uint8_t *Mem_2, OS_Uint32_t Size)
{
    OS_Uint32_t i = 0;
    OS_Uint8_t  Ret = 1;

    for (i = 0; i < Size; i++)
    {
        if (Mem_1[i] != Mem_2[i])
        {
            Ret = 0;
            break;
        }
    }

    return Ret;
}

void TASK1_FUNC(void *param)
{
    OS_Uint32_t Ret = 0;

    BspLEDOn(LED_2);
    while(1)
    {
        t1_cnt++;

#if TC_QUEUE_READ_SLEEP
        Ret = OS_API_QueueRead(QueueHandle, ReadBuf, QUEUE_ELEMENT_SZ);

        if (Ret == OS_SUCCESS)
        {
            OS_Uint8_t ReadSuccess = 0;
            ReadSuccess = TC_MemorySame(ReadBuf, WriteBuf, QUEUE_ELEMENT_SZ);

            if (ReadSuccess)
            {
                printf("T1 Read Success...\r\n");
            }
            else
            {
                printf("T1 Read ERROR\r\n");
            }
        }
#endif

#if TC_QUEUE_READ_SLEEP_TIMEOUT
        Ret = OS_API_QueueReadTimeout(QueueHandle, ReadBuf, QUEUE_ELEMENT_SZ, 1000);

        if (Ret == OS_SUCCESS)
        {
            OS_Uint8_t ReadSuccess = 0;

            ReadSuccess = TC_MemorySame(ReadBuf, WriteBuf, QUEUE_ELEMENT_SZ);

            if (ReadSuccess)
            {
                printf("T1 Read Success...\r\n");
            }
            else
            {
                printf("T1 Read ERROR\r\n");
            }
        }
        else if (Ret == OS_QUEUE_RD_WAIT_TIMEOUT)
        {
            printf("T1 Read Timeout\r\n");
        }
        else
        {
            printf("T1 Read Failed:%d\r\n", Ret);
        }
#endif

#if TC_QUEUE_WRITE_SLEEP
        OS_Memset((void *)WriteBuf, t1_cnt, QUEUE_ELEMENT_SZ);
        Ret = OS_API_QueueWrite(QueueHandle, WriteBuf, QUEUE_ELEMENT_SZ);
        if (Ret == OS_SUCCESS)
        {
            printf("T1 Write [%d] Success\r\n", WriteBuf[0]);
        }
#endif

#if TC_QUEUE_WRITE_SLEEP_TIMEOUT
        OS_Memset((void *)WriteBuf, t1_cnt, QUEUE_ELEMENT_SZ);
        Ret = OS_API_QueueWriteTimeout(QueueHandle, WriteBuf, QUEUE_ELEMENT_SZ, 1000);
        if (Ret == OS_SUCCESS)
        {
            printf("T1 Write [%d] Success\r\n", WriteBuf[0]);
        }
        else if (Ret == OS_QUEUE_WR_WAIT_TIMEOUT)
        {
            printf("T1 Write Timeout\r\n");
        }
        else
        {
            printf("T1 Write Failed:%d\r\n", Ret);
        }
#endif

    }
}

void TASK2_FUNC(void *param)
{
    OS_Uint32_t Ret = 0;
    BspLEDOn(LED_3);
    while(1)
    {
        t2_cnt++;

#if TC_QUEUE_READ_SLEEP || TC_QUEUE_READ_SLEEP_TIMEOUT
        OS_Memset((void *)WriteBuf, t2_cnt, QUEUE_ELEMENT_SZ);
        Ret = OS_API_QueueWrite(QueueHandle, WriteBuf, QUEUE_ELEMENT_SZ);
        if (Ret == OS_SUCCESS)
        {
            printf("T2 Write Success [%d]\r\n", WriteBuf[0]);
        }
#endif

#if TC_QUEUE_WRITE_SLEEP || TC_QUEUE_WRITE_SLEEP_TIMEOUT

        Ret = OS_API_QueueRead(QueueHandle, ReadBuf, QUEUE_ELEMENT_SZ);
        if (Ret == OS_SUCCESS)
        {
            printf("T2 Read Success [%d]\r\n", ReadBuf[0]);
        }
#endif
        OS_API_TaskDelay(TASK_2_DELAY);
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

    OS_API_QueueCreate(&QueueHandle, QUEUE_ELEMENT_SZ, QUEUE_ELEMENT_NR);

    OS_API_KernelStart();

    while(1);
}
