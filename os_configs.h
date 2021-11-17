#ifndef __MXOS_CONFIG_H__
#define __MXOS_CONFIG_H__

#include "os_types.h"

#define OS_ASSERT(x)        if((x) == 0) {while(1);}

/* Memory Mamanger*/
#define CONFIG_TOTAL_HEAP_SIZE                      (32 * OS_SIZE_KB)

/* Task Scheduler*/
#define CONFIG_TASK_NAME_LEN                        (16 * OS_SIZE_BYTE)



#endif // !__MXOS_CONFIG_H__
