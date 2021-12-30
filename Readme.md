# MxOS #
A Lite Real Time Operation System for 32bit MCU(CPU)

![](https://gitee.com/stephenzhou-tech/mx-os/raw/master/documents/MxOS_V1_0_Architecture.png)

## Introductions ##
### 1. Memory Manager ###
- 1.1 Support allocating and free memory API.
- 1.2 Bestfit memory allocate algorithm
- 1.3 Support memory merge with two free memory
- 1.4 Trace functions

### 2. A Real Time task scheduler ###
- 2.0 Preemptive scheduling strategy
- 2.1 Support max task priority 32 levels
- 2.2 Bigger priority number means higher priority
- 2.3 Tasks of the same priority can be executed on a rotational basis
- 2.4 Idle task priority equal 0
- 2.5 Support suspend/resume scheduler
- 2.6 Support suspend/resume task
- 2.7 Support task blocked in timestamp(delay)
- 2.8 Support yeild CPU
- 2.9 Support stack overflow check
- 2.10 Support change task priority dynamic
- 2.11 Support get task priority
- 2.12 Trace functions

### 3. IPC ###
- 3.1 Support semaphore to synchronous tasks
- 3.2 Support mutex lock to protect critical zone with priority rise algorithm to prevent priority reversal
- 3.3 Support queue to make tasks transfer data more easier
- 3.4 Trace function for IPC

### 4. Critical protection ###
- 4.1 Support suspend task scheduler to protect critical zone
- 4.2 Support Disable/Enable global interrupt to to protect critical zone

### 5. Printk ###
- 5.1 Support debug print level from DEBUG to ERROR

### 6. Software Timer ###
- 6.1 Support software timer at highest priority(31), execute in task context
- 6.2 Trace function for IPC

### 7. Shell ###
- 7.1 Support register external shell to operation system
- 7.2 Already adapt a letter shell for MxOS
- 7.3 Support **task** command to show task informations
- 7.4 Support **mem** command to show memory informations

### 8. CPU architecture ###
- 8.1 Adapt Cortex-M4 with FPU architecture

### 9. Demo ###
- 9.1 Add a demo by using STM32F407ZGT6(Cortex-M4 Core with FPU)
- 9.2 Add some test case in demo to verify the MxOS

##How to use##
### Configure ###
Firstly there is a kernel configure file called **os_configs.h**, you can choose the feature which you want, and configure it in this file.

### Source code ###
All of the kernel source code and header file is defined in 

> **kernel\source** 
 
> **kernel\include** 

these to your projects(If you will use it), and compile these with your applications.

### Demo application ###
In your application, firstly make sure the **os_configs.h** have been configured, and then you can use MxOS like below:

	OS_Uint32_t task1_handle = 0;
	OS_Uint32_t task2_handle = 0;
	OS_Uint32_t task3_handle = 0;
	
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
I have finished some demo including Multi-Task run, Suspend/Delay task, usage of IPC(sem/mutex/queue), shell, and software timer demo, in the path:
> **demo\stm32f407zgt6\applications\mxosDemo\src**

### Task ###
There are some APIs for control task:

	OS_Uint32_t OS_API_TaskCreate(TaskInitParameter Param, OS_Uint32_t *TaskHandle);
	OS_Uint32_t OS_API_TaskDelay(OS_Uint32_t TickCnt);
	OS_Uint32_t OS_API_TaskSuspend(OS_Uint32_t TaskHandle);
	OS_Uint32_t OS_API_TaskResume(OS_Uint32_t TaskHandle);

	OS_Uint32_t OS_API_TaskPrioritySet(OS_Uint32_t TaskHandle, OS_Uint8_t NewPriority);
	OS_Uint8_t OS_API_TaskPriorityGet(OS_Uint32_t TaskHandle);

Firstly, you should create your task, and a valid task handle will return to you, and then you can use your task handle to control your task.

### Memory ###
There are some APIs for memory:
	
	void *OS_API_Malloc(OS_Uint32_t WantSize);
	void OS_API_Free(void *addr);
If memory allocte successfully, it will return the start address of your memory, otherwise, return OS_NULL;

When you free memory, system will auto merge the two adjacent blocks of memory.

### Sempaphore ###
Support counting sempaphore and binary sempaphore;

The sempaphore API defined as below:

    OS_Uint32_t OS_API_SemCreate(OS_Uint32_t *SemHandle, OS_Uint32_t Count);
    OS_Uint32_t OS_API_BinarySemCreate(OS_Uint32_t *SemHandle, OS_Uint32_t Count);
    
    OS_Uint32_t OS_API_SemWait(OS_Uint32_t SemHandle);
    OS_Uint32_t OS_API_BinarySemWait(OS_Uint32_t SemHandle);
    
    OS_Uint32_t OS_API_SemWaitTimeout(OS_Uint32_t SemHandle, OS_Uint32_t Timeout);
    OS_Uint32_t OS_API_BinarySemWaitTimeout(OS_Uint32_t SemHandle, OS_Uint32_t Timeout);
    
    OS_Uint32_t OS_API_SemTryWait(OS_Uint32_t SemHandle);
    OS_Uint32_t OS_API_BinarySemTryWait(OS_Uint32_t SemHandle);
    
    OS_Uint32_t OS_API_SemPost(OS_Uint32_t SemHandle);
    OS_Uint32_t OS_API_BinarySemPost(OS_Uint32_t SemHandle);
    
    OS_Uint32_t OS_API_SemDestory(OS_Uint32_t SemHandle);

The **Wait** API suffix as **Try** means it will never cause task sleep.

The **Wait** API suffix as **Timeout** means once cause task sleep, when timeout or wake up condition comes, it will return.

The **Wait** API without any suffix, mean if the task does not meet the wakeup condition, it will never wake up.

### Mutex Lock ###
Task can use mutex lock to protect critical zone.
The API is defined as below:

    OS_Uint32_t OS_API_MutexCreate(OS_Uint32_t *MutexHandle);

	OS_Uint32_t OS_API_MutexLock(OS_Uint32_t MutexHandle);

	OS_Uint32_t OS_API_MutexLockTimeout(OS_Uint32_t MutexHandle, OS_Uint32_t Timeout);

	OS_Uint32_t OS_API_MutexTryLock(OS_Uint32_t MutexHandle);

	OS_Uint32_t OS_API_MutexUnlock(OS_Uint32_t MutexHandle);

	OS_Uint32_t OS_API_MutexDestory(OS_Uint32_t MutexHandle);

The rule of usage is the same as Sempaphore.

### Queue ###
The Queue is used for task/task, irq/task transfer data, the APIs is defined as below:

    OS_Uint32_t OS_API_QueueCreate(OS_Uint32_t *QueueHandle,OS_Uint32_t ElementSize,OS_Uint32_t ElementNr);

	OS_Uint32_t OS_API_QueueWrite(OS_Uint32_t QueueHandle, const void * buffer, OS_Uint32_t size);

	OS_Uint32_t OS_API_QueueTryWrite(OS_Uint32_t QueueHandle, const void * buffer, OS_Uint32_t size);

	OS_Uint32_t OS_API_QueueWriteTimeout(OS_Uint32_t QueueHandle, const void * buffer, OS_Uint32_t  size, OS_Uint32_t Timeout);

	OS_Uint32_t OS_API_QueueRead(OS_Uint32_t QueueHandle, void * buffer, OS_Uint32_t size);

	OS_Uint32_t OS_API_QueueTryRead(OS_Uint32_t QueueHandle, void * buffer, OS_Uint32_t size);

	OS_Uint32_t OS_API_QueueReadTimeout(OS_Uint32_t QueueHandle, void * buffer, OS_Uint32_t size, OS_Uint32_t Timeout);

	OS_Uint32_t OS_API_QueueDestory(OS_Uint32_t QueueHandle);

	OS_Uint32_t OS_API_QueueRemainingSpace(OS_Uint32_t QueueHandle);

### Software Timer ###
Of course, you can use software time instead of hardware time in MxOS:

    OS_Uint32_t OS_API_SwTimerCreate(OS_Uint32_t *SwTimerHandle,
                                 OS_Uint8_t WorkMode,
                                 OS_Uint32_t Interval,
                                 OS_SwTimerHandler_t TimeoutHandler,
                                 void *FuncParam);

	OS_Uint32_t OS_API_SwTimerStart(OS_Uint32_t SwTimerHandle);

	OS_Uint32_t OS_API_SwTimerStop(OS_Uint32_t SwTimerHandle);

	OS_Uint32_t OS_API_SwTimerDelete(OS_Uint32_t SwTimerHandle);

Support two mode of software timer:

> Auto Reload

> OneShot

### Critical zone protection ###
There are 3 ways to protect critical zone:

	void OS_API_EnterCritical(void);
	void OS_API_ExitCritical(void);

    void OS_API_SchedulerSuspend(void);
	void OS_API_SchedulerResume(void);
	
	OS_Uint32_t OS_API_MutexLock(OS_Uint32_t MutexHandle);
	OS_Uint32_t OS_API_MutexUnlock(OS_Uint32_t MutexHandle);

The first group will disable global interrupt, be carefull to use it, it will cause the real time kernel can not handle exceptions.

The second group means suspend task scheduler, it will cause the task switch stop.

The third group means use mutex, if the resource is not ready, task will sleep.

### Trace ###
All of the kernel trace functions have been added in the **os_trace.h**, if needed, just implement the functions which you want.


### About print log ###
There is a lite logger system split log with different level:
> ERROR_LEVEL

> WARNING_LEVEL

> INFO_LEVEL

> DEBUG_LEVEL

If you want to use it, firstly you should initialize your serial port(UART), and make sure the **printf** can output log.

> Strongly recommended, initialize your serial port in the function **PlatformInit**, and call the **PlatformInit** before use MxOS(just like the demo).

### About Shell ###
I have adapt a shell(letter Shell) for MxOS, thanks for the author of the letter Shell.
If you want to use shell function, you should implement two interface of your platform:

	/* 
	 * This function should implement as write char data,
	 * using UART in polling mode
	 */
    void PlatformUartSendDataPolling(const char ch)

	/* 
	 * This function should implement as read char data,
	 * using UART in polling mode,
	 * if read succussfully return 0, otherwise return -1
	 */
	signed char PlatformUartRecvDataPolling(char *ch)

I have port stm32f407 uart1 as console in demo:
> platform\platform.c

There are two command in system:

> mem ------ Check memory informations

> task ------ Check task informations

See more detail in source code.

Contact me by: *StephenZhou_Tech@163.com*
