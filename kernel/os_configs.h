#ifndef __MXOS_CONFIG_H__
#define __MXOS_CONFIG_H__

#include "os_types.h"

#define OS_KERNEL_MAJOR_VERSION                     1
#define OS_KERNEL_MINOR_VERSION                     0

#define OS_ASSERT(x)                                if((x) == 0) {ARCH_InterruptDisable(); while(1);}

/* Memory Mamanger */
#define CONFIG_TOTAL_HEAP_SIZE                      (32 * OS_SIZE_KB)

/* Task and Scheduler */
#define CONFIG_TASK_NAME_LEN                        (16 * OS_SIZE_BYTE)
#define CONFIG_IDLE_TASK_STACK_SIZE                 (512 * OS_SIZE_BYTE)
#define CONFIG_TICK_COUNT_INIT_VALUE                (0x00000000)

#define CONFIG_STACK_OVERFLOW_CHECK                 1

/* Configure System Tick Rate */
#define CONFIG_SYS_CLOCK_RATE                       (168 * OS_FREQ_MHZ)
#define CONFIG_SYS_TICK_RATE_HZ                     (1 * OS_FREQ_KHZ)

/* Architecture */
#define CONFIG_ARM_ARCH                             1

#define ARCH_SystemTickHander                       SysTick_Handler
#define ARCH_PendSVHandler                          PendSV_Handler

/* OS Debug */
#define OS_DBG_SCHEDULER                            1
#define OS_DBG_MEMORY                               1

/* OS Printk level */
#define OS_PRINTK_ERROR_LEVEL                       3
#define OS_PRINTK_WARNING_LEVEL                     2
#define OS_PRINTK_INFO_LEVEL                        1
#define OS_PRINTK_DEBUG_LEVEL                       0

#define OS_USE_PRINTK                               1
#define OS_PRINTK_LEVEL                             (OS_PRINTK_INFO_LEVEL)

#endif // !__MXOS_CONFIG_H__
