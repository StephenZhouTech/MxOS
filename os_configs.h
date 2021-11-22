#ifndef __MXOS_CONFIG_H__
#define __MXOS_CONFIG_H__

#include "os_types.h"

#define OS_ASSERT(x)        if((x) == 0) {while(1);}

/* Memory Mamanger*/
#define CONFIG_TOTAL_HEAP_SIZE                      (32 * OS_SIZE_KB)

/* Task Scheduler*/
#define CONFIG_TASK_NAME_LEN                        (16 * OS_SIZE_BYTE)
#define CONFIG_IDLE_TASK_STACK_SIZE                 (512 * OS_SIZE_BYTE)
#define CONFIG_TICK_COUNT_INIT_VALUE                (0x00000000)
#define CONFIG_SYS_TICK_RATE_HZ                     (1000)

#endif // !__MXOS_CONFIG_H__
