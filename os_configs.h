#ifndef __MXOS_CONFIG_H__
#define __MXOS_CONFIG_H__

#include "os_types.h"

#define CONFIG_TOTAL_HEAP_SIZE                      (32 * OS_SIZE_KB)

#define OS_ASSERT(x)        if((x) == 0) {while(1);}

#endif // !__MXOS_CONFIG_H__
