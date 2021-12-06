#ifndef __MXOS_CONFIG_H__
#define __MXOS_CONFIG_H__

#include "os_types.h"

#define OS_ASSERT(x)        if((x) == 0) {ARCH_InterruptDisable(); while(1);}

/* Memory Mamanger*/
#define CONFIG_TOTAL_HEAP_SIZE                      (32 * OS_SIZE_KB)

/* Task Scheduler*/
#define CONFIG_TASK_NAME_LEN                        (16 * OS_SIZE_BYTE)
#define CONFIG_IDLE_TASK_STACK_SIZE                 (512 * OS_SIZE_BYTE)
#define CONFIG_TICK_COUNT_INIT_VALUE                (0x00000000)

#define CONFIG_SYS_CLOCK_RATE                       (168 * OS_FREQ_MHZ)
#define CONFIG_SYS_TICK_RATE_HZ                     (1 * OS_FREQ_KHZ)

#define CONFIG_ARM_ARCH                             1

#define ARCH_SystemTickHander       SysTick_Handler
#define ARCH_PendSVHandler          PendSV_Handler

/* OS Debug */
#define OS_DBG_SCHEDULER                            1
#define OS_DBG_MEMORY                               1

#endif // !__MXOS_CONFIG_H__
